<?php

function resize_array() {
  return array(
  'org' =>    [0, 'org'], 
  'tiny'   => [40, 'tiny', ['crop' => 1]],                        
  'thumb' =>  [100, 'thumbs'],
  'square' => [200, 'square', ['crop' => 1]],                        
  'small' =>  [200, 'small'], 
  'medium' => [640, 'medium'], 
  'ig' =>     [640, 'ig', ['crop' => 1]], // instagram square
  'big'   =>  [1280, 'big'], 
  'full'  =>  [1920, 'full']
  );
}
/*  

 Media db
  
 name - not in use 
 filename - basefile of org. file (on disk in org folder for file upload - not needed if url is set)
 src      - basefile of big file in image folder
 url      - location of original if online

*/ 


/** unsused - automatic comment added on  */
function media_process_batch($options=[]) {
  global $__SITE;
  
  $search = isset($options['search']) ? $options['search']: ['processed' => 0];
  $sort   = isset($options['sort']) ? $options['sort'] : 'id desc';
  $search['processed'] = 0; /** should we make this optional */
                     
  $mediaObjs = find_objects('media', $search, ['sort' => $sort]);
  $errors = $messages = [];
  $debug = 0;
  
  $count = count($mediaObjs);

  $hard_limit = 1000;
  $limit = isset($options['limit']) ? $options['limit'] : $hard_limit;
  
  $messages[] = "Found $count mediaObjs, proccessing max $limit<br>";
  
  $rows = [];
  $rows[] = ['id', 'client', 'site', 'parent', 'pid', 'src', 'h', 'w', 'size', 'errors', 'messages'];    
  $i = $processed = 0;
  
  foreach($mediaObjs as $mediaObj) {
    $data = ['processed' => 1];
    $res = media_process($mediaObj, $options);
    $errors = array_merge_smart($errors, $res->error);
    $messages = array_merge_smart($messages, $res->message);    
    $rows[] = $res->row;
    $processed += $res->processed;
    $i++;
    if($limit > 0 && $i >= $limit) break; 
  }
 
  $result = new stdClass;
  $result->success = 1;
  $result->error = $errors;
  $result->message = $messages;
  $result->count = $count;
  $result->processed = $i;
  $result->rows = $rows;

  $now = now();
  print_log("{$now}: media-process: count=$count ", 'media-process', LOG_LEVEL_TEST);
  return $result;
  
}


/** 
  Process uploaded media:
  - checks if valid (exists, correct mime, not corrupt)
  - checks if parent object exists  
  - checks/sets size/h/w/bitrate etc
  - resizes if required (now generally handled by cloud)
  - copies original to /web/media/
  - uploads to private cloud server (volume)
  - uploads to public cloud (cloudinary) if required (user profile images, some sites/parent types)
*/  
 
function media_process($mediaObj, $options=[]) {
  global $__SITE;
  $errors = $messages = [];
  $debug = 0;                       
  
  $errors = $messages = [];

  $delete = $stop_processing = false;
  $data = ['processed' => 1];
  
  /** check required fields */
  //$required = ['client_id', 'site_id', 'parent_id', 'parent_type', 'src', 'path', 'filename', 'type'];
  $required = ['client_id', 'site_id', 'parent_id', 'parent_type', 'src', 'path'];
  foreach($required as $k) {
    if(!$mediaObj->$k) {
      $errors[] = "{$mediaObj->id}: missing {$k} - delete";
      $stop_processing = $delete = true;
      break;
    }
  }

  
  /** check that parent exists */
  if(!$stop_processing) {
    $parent_type = $mediaObj->parent_type;
    $parent_id   = $mediaObj->parent_id;  
    $parentObj = get_object($mediaObj->parent_type, $mediaObj->parent_id);
    if(!$parentObj) {
      $errors[] = "{$mediaObj->id}: missing parent {$parent_type} {$parent_id} - delete";
      $stop_processing = $delete = true;
    }
  }
                         
  $site_id = $mediaObj->site_id;
  $shortname = get_object('site', $site_id, 'shortname');
  $src_dir = "/web/data/{$shortname}";
  $media_dir = "/web/media/{$shortname}";
  $path = $mediaObj->path;
  $src = $mediaObj->src;
  
  /** check that source exists on disk */
  if(!$stop_processing) {  
    $rel_src = print_media($mediaObj, ['size' => 'org', 'source_only' => 1]);
    $full_src = "{$src_dir}{$rel_src}";
    if(!file_exists($full_src)) {
      $errors[] = "{$mediaObj->id}: missing src {$full_src} - delete";
      $stop_processing = $delete = true;
    }
  }

  /** check actual file mime/type */
  if(!$stop_processing) {
    
    $mime_type = mime_content_type($full_src); // actual mime according to PHP
    $typeObj = $mime_type ? find_object('mime_type', ['mime' => $mime_type]) : new stdClass;
    $type = $typeObj->type;
    
    $allowed_types = ['image', 'audio', 'video']; // what about 
    if(!$typeObj || !in_array($type, $allowed_types)) {
      $errors[] = "{$mediaObj->id}: illegal or missing mime:{$mime_type} type={$type} - delete";
      $stop_processing = $delete = true;
    } else {
      
      $extension = file_extension($full_src);
      $mime_ext = mime_extension($mime_type) ?: $typeObj->extension;
      if(!$extension) { 
        $src .= ".{$mime_ext}";
        $mediaObj->src = $src; // needed below
        
        echo("Missing extension for $full_src Mime: $mime_type Type: {$type} Should be: {$mime_ext}<br>Src: {$src}<br>");
        
        //$data['src'] = $new_src;
        
      }
        
      /** check file size, extension, reported mime/type according */
      $s = filesize($full_src);
      if($s < 1) {
        $errors[] = "{$mediaObj->id}: empty file  - delete";
        $stop_processing = $delete = true;
      } else {
        $img_type = mime_type($full_src);
        $fields = ['filesize' => $s, 'mime' => $mime_type, 'type' => $type, 'src' => $src];      
        foreach($fields as $k => $v) {
          if($v && $mediaObj->$k != $v) {        
            $data[$k] = $v;
            $errors[] = "{$mediaObj->id}: wrong $k {$mediaObj->$k} => {$v} - update";
          }
        }
      }
          
    }
  }
                         

  /** check image info for images */
  if(!$stop_processing && $type == 'image') {
    if($debug) $messages[] = "Check image";
    $info = getimagesize($full_src); // size of original
    $w = $info[0];
    $h = $info[1];
    $bits = $info['bits'];
    $channels = $info['channels'];
    $mime = $info['mime'];
    
    $fields = ['height' => $h, 'width' => $w, 'depth' => $bits, 'channels' => $channels, 'mime' => $mime];      
    foreach($fields as $k => $v) {
      if($v && $mediaObj->$k != $v) {        
        $data[$k] = $v;
        $errors[] = "{$mediaObj->id}: wrong $k {$mediaObj->$k} => {$v} - update";
      }
    }
  }

  /** copy original from /web/data/ to /web/media/ */    
  if(!$stop_processing) {
    if($debug) $messages[] = "Copy original";
    if (!file_exists($media_dir.$path) && !mkdir($media_dir.$path, 0775, true)) {
      $errors[] = "Failed to create {$dst_dir}";
      unset($data['processed']);
      $stop_processing = 1;
    } else {      
      $full_dst = "{$media_dir}{$path}{$mediaObj->src}";
      echo("copy: $full_src to: $full_dst media src={$mediaObj->src}<br>");
      if(!copy($full_src, $full_dst)) {
        $errors[] = "Failed to copy {$full_src} to {$full_dst}";
        unset($data['processed']);
        $stop_processing = 1;
      }
    }
  }                                       

  /** upload to media server */
  $media_server_ip = MEDIA_SERVER_IP;
  if(!$stop_processing && $media_server_ip) {
    if($debug) $messages[] = "Upload to media server";
    $res = media_upload_mediaserver($media_server_ip, $mediaObj);
    $uerrors = $res->error;
    $success = $res->success;
    if($success && !$uerrors) {
      $remote_size = bytes2human($res->filesize);
      if($umessages) $messages = array_merge_smart($messages, $umessages);
      else $messages[] = "Uploaded {$remote_size_h} to media server ({$media_server_ip})";
    } else {
      if($uerrors) $errors = array_merge_smart($errors, $uerrors);
      else $errors[] = "Failed to upload {$mediaObj->id} to media server ({$media_server_ip})";
      unset($data['processed']);
      $stop_processing = 1;
    }
  }
  
  /** upload to cloudinary */
  
  $site_config = get_object('site', $site_id, 'cloudinary');
  $upload_types = ['user_profile']; /** always upload these for all sites */
  $siteConfigObj = $site_config ? json_decode($site_config) : new stdClass;
  $site_types = $siteConfigObj->obj_type; 

  $upload_cloud = in_array($parent_type, $upload_types) || in_array($parent_type, $site_types) || in_array('all', $site_types);      
  $ci_config = $upload_cloud ? get_config('ci', $site_id) : '';
  
  if(!$stop_processing && $upload_cloud && $ci_config) {
    if($debug) $messages[] = "Upload to cloud";
    $res = media_upload_cloud($mediaObj);
    $uerrors = $res->error;
    $success = $res->success;
    if($success && !$uerrors) {
      $remote_size = bytes2human($res->filesize);
      if($umessages) $messages = array_merge_smart($messages, $umessages);
      else $messages[] = "Uploaded {$remote_size_h} to cloud server (Cloudinary/{$cloud_name})";
    } else {
      if($uerrors) $errors = array_merge_smart($errors, $uerrors);
      else $errors[] = "Failed to upload {$mediaObj->id} to cloud server (Cloudinary/{$cloud_name})";      
      /** it's ok if not uploaded to cloudinary */
      //unset($data['processed']);
      //$stop_processing = 1;
    }
  }
  
  
  if($delete) { /** delete */
    list($dcount, $derrors) = delete_object('media', $mediaObj->id);
    if($derrors) $errors = array_merge_smart($errors, $derrors);
    else $messages[] = "Deleted {$mediaObj->id} Count: {$dcount}";
  } elseif($data) { /** update */
    list($mid, $merrors) = update_object('media', $data, $mediaObj->id);
    if($merrors) $errors = array_merge_smart($errors, $merrors);
    else $messages[] = "Done with {$mediaObj->id} Processed={$data['processed']}";
  } else {
    $messages[] = "Done with {$mediaObj->id} - nothing to do.";
  }
  
  $row = [$mediaObj->id, $mediaObj->client_id, $mediaObj->site_id, $mediaObj->parent_type, $mediaObj->parent_id, 
            $rel_src, "{$mediaObj->height}/$h", "{$mediaObj->width}/$w", $mediaObj->filesize."/{$s}", 
            html_span(implode('<br>', $errors), 'red'), html_span(implode('<br>', $messages), 'green')];

  $result = new stdClass;
  $result->success = 1;
  $result->error = $errors;
  $result->message = $messages;
  $result->processed = $data['processed'];
  $result->row = $row;
     
  dump($errors);
  $now = now();
  print_log("{$now}: media-process {$mediaObj->id}: count=$count ", 'media-process', LOG_LEVEL_TEST);
  return $result;
}

function media_upload_mediaserver($ip, $mediaObj_or_id, $options= []) {
  $errors = $messages = [];
  $mediaObj = any2obj('media', $mediaObj_or_id);
  
  $site_id = $mediaObj->site_id;
  $shortname = get_object('site', $site_id, 'shortname');
  $src_dir = "/web/data/{$shortname}";
  $media_dir = "/web/media/{$shortname}";
  $path = $mediaObj->path;
  $src = $mediaObj->src;

  $full_src = "{$media_dir}{$path}{$src}";
  if(!file_exists($full_src)) return json_error_object("Cannot upload, {$full_src} not found");
  $src_size = filesize($full_src);
  if(!$src_size) return json_error_object("Cannot upload, {$full_src} is empty");

  /** get and check keys */
  $key1 = '/web/.priv/.ssh/id_rsa.pub';
  $key2 = '/web/.priv/.ssh/id_rsa';
  $size1 = filesize($key1);
  $size2 = filesize($key2);

  if(!$size1) return json_error_object("Failed to read public key");
  if(!$size2) return json_error_object("Failed to read private key");
  
  /** create connection */
  $connection = ssh2_connect($ip, 22, array('hostkey'=>'ssh-rsa'));
  if(!$connection) json_error("No connection to {$ip}");
  if(!ssh2_auth_pubkey_file($connection, 'media', $key1, $key2)) {
    ssh2_exec($connection, 'exit');
    return json_error_object("Public Key Authentication Failed");
  }
  
  $sftp = ssh2_sftp($connection);
  if(!$sftp) {
    ssh2_exec($connection, 'exit');
    return json_error_object("SFTP Failed");
  }
  
  $path = $mediaObj->path;
  $shortname = $site_id == $__SITE->id ? $__SITE->shortname : get_object('site', $site_id, 'shortname');
  
  $remote_media_dir = "/volume/media/{$shortname}{$path}";
  ssh2_sftp_mkdir($sftp, $remote_media_dir, 0775, true); // create, even if exists
  
  /** all good, send to media server */
  if(!ssh2_scp_send($connection, $full_src, $remote_media_dir.$src)) {
    ssh2_exec($connection, 'exit');
    return json_error_object("Failed to upload {$full_src} to {$ip}:{$remote_media_dir}{$src}");
  }    

  $domain = $site_id == $__SITE->id ? $__SITE->domain : get_object('site', $site_id, 'domain');
  $site_url = "http://media.{$domain}";    
  $url = "{$site_url}{$path}{$src}?live=1&ts=".time();
  
  $remote_size = remote_filesize($url) ?: 0; /** get file directly, no glide, cache bust */
  if($remote_size != $src_size) {
    ssh2_exec($connection, 'exit');
    return json_error_object("Uploaded file wrong size of {$url}: {$remote_size} != {$src_size}");
  }

  ssh2_exec($connection, 'exit');

  $messages[] = "OK: Uploaded {$mediaObj->id} to {$ip}:{$remote_media_dir}";
  
  $result = new stdClass;
  $result->success = 1;
  $result->error = $errors;
  $result->message = $messages;
  $result->filesize = $remote_size;
  return $result;
}                    
    
/** Upload to cloud (cloudinary now, could be others) */
function media_upload_cloud($mediaObj_or_id, $options= []) {
  $errors = $messages = [];
  $mediaObj = any2obj('media', $mediaObj_or_id);
  if(!$mediaObj) return json_error_object("Media Upload Cloud: missing media");
  
  $path = $mediaObj->path;
  $src = $mediaObj->src;

  $site_id = $mediaObj->site_id;
  $shortname = get_object('site', $site_id, 'shortname');
  $src_dir = "/web/data/{$shortname}";
  $media_dir = "/web/media/{$shortname}";

  $full_src = "{$media_dir}{$path}{$src}";

  if(!file_exists($full_src)) return json_error_object("Cannot upload, {$full_src} not found");
  $src_size = filesize($full_src);
  if(!$src_size) return json_error_object("Cannot upload, {$full_src} is empty");

  /** all good, send to cloudinary */
  require_once 'xml_cloudinary.inc';
  $res = cloudinary_upload_media($mediaObj);
  if(!$res->success || $res->error) return $res;
  
  $messages[] = "OK: Uploaded {$mediaObj->id} to {$ip}:{$remote_media_dir}";
  
  $result = new stdClass;
  $result->success = 1;
  $result->error = $errors;
  $result->message = $messages;
  $result->filesize = $remote_size;
  return $result;
}
  
// takes a hex color (3 or 6 chars). Returns value from 0 (black) to 1 (white) for brightness of color
/** unsused - automatic comment added on  */
function get_brightness($hexcolor) {
  // HEX > RGB
  if(!$hexcolor) return 1;
  list($R, $G, $B) = color_rgb($hexcolor);
  $brightness = ($R+$G+$B) / 765;
  //echo("brightness of $hexcolor = $brightness<br> $R + $G + $B = ".($R+$G+$B));
  return($brightness);
}

// returns decimal RGB values from hex color (FFF or FFFFFF)
function color_rgb($hexcolor) {
    if(strlen($hexcolor) == 3) {
    $R = hexdec(substr($hexcolor, 0,1)) ^ 2;
    $G = hexdec(substr($hexcolor, 1,1)) ^ 2;
    $B = hexdec(substr($hexcolor, 2,1)) ^ 2;
  } elseif(strlen($hexcolor) == 6) {
    $R = hexdec(substr($hexcolor, 0,2));
    $G = hexdec(substr($hexcolor, 2,2));
    $B = hexdec(substr($hexcolor, 4,2));
  }
  return array($R,$G,$B);
}


function is_color($str) {
  $str = ltrim($str, '#');
  $regex = '^[a-fA-F0-9]+$/i';
  return ((strlen($str)==6 || strlen($str)==3) && preg_match($regex, $str)) ? true : false;

}

// takes and image (relative path), replaces all colors with $color (preserving transparency)
// returns new image
/** unsused - automatic comment added on  */
function image_color_replace($image, $color, $default='', $full=false) {
  $url = $full ? add_host($image) : $image;
  if($color && $default && $color==$default) return $url;
  $save_to = $_SERVER['DOCUMENT_ROOT'].$image;
  return ColorReplace($url, $color, $save_to);
}



// returns full system file path for media 
// including filename of image if $size is set (org, big, small, thumb)
function get_media_file($media, $size='') {
  return $_SESSION['media_dir'].get_media_tail($media, $size);
}

// returns relative web path for media
// including filename of image if $size is set (org, big, small, thumb)
function get_media_src($media, $size='') {
  $src = $_SESSION['media_path'].get_media_tail($media, $size);
  return $src;
  
  //if($size) {
  //  $mediaObj = any2obj('media', $media);
  //  $src .=  '?'.$mediaObj->id;
  //}
  //return $src;
}


// including filename of image if $size is set (org, big, small, thumb)
// if size is not specified, returns base directory
function get_media_tail($media, $size='') {
  $mediaObj = any2obj('media', $media);
  if(!$mediaObj->property_type || !$mediaObj->parent_id) return ''; // this is no a property media 
  $type_dir = image_type_dir($mediaObj->property_type);
  $pid  = $mediaObj->parent_id ? $mediaObj->parent_id : 0;
  $sid = $mediaObj->source_id ? $mediaObj->source_id : 0;
  $cid = $mediaObj->city_id ? $mediaObj->city_id : 0;  
  $aid = $mediaObj->estate_agent_id;
  $oid = $mediaObj->owner_id;
  if(!$aid && !$oid) $aoid=0; else $aoid = $aid ? $aid : $oid; 
  $middle = "$sid/$cid/$aoid/$pid/"; // use estate_agent_id

  $dir = $type_dir.$middle;

  if($size) {
    $size_dir = image_size_dir($size);
    $dir.=$size_dir;  
    $src = $size == 'org' ? $mediaObj->filename : $mediaObj->src;
    if($src) $dir.=$src;  
  }

  return $dir;
}

// return html_image for first picture for a property
//function get_first_pic($apt_or_id, $size='small', $show_coming_soon=true, $class='thumb_pic') {
function property_images($apt_or_id, $options = array()) {
  return object_images(property_types(), $apt_or_id, $options);
}

function object_images($obj_type, $obj_or_id, $options = array()) {
  $id  = $obj_or_id->id ?: $obj_or_id;
  if(!$id) return null;
  $size = $options['size'];
  $full = $options['full'];
  $first = $options['first']; // just get first
  if($size == 'thumb') $size = 'thumbs';
  if(!$size) $size = 'medium';
  
  //$field = $options['source_only'] ? 'src' : ""; // ""; // just source or full path
  if($options['source_only']) $field = 'src';
  else if($options['full_source']) $field = "concat (path, '$size/', src)";
  else $field = '';
  
  $search = array('parent_type' => $obj_type, 'parent_id' => $id, 'active' => 1);
  $search_options = array('sort' => 'rank', 'field' => $field);  
  $result = $first ? find_object('media', $search, $field) : find_objects('media', $search, $search_options);
  if(!$full) return $result;
  
  if(is_array($obj_type)) $obj_type = $obj_type[0]; // property images sends array
  // prepend site url
  $obj = get_object($obj_type, $id);
  $site_id = $obj->site_id ?: $_SESSION['site_id'];
  
  $site_url = site_full_url($site_id);
  if($first) return $site_url.$result;
  foreach($result as &$img) $img = $site_url.$img;
  return $result;
}


function get_first_pic($apt_or_id, $options = array()) {
  $size = $options['size'] ?: 'small';
  $class = $options['class'];
  $default = $options['show_default'];
  
  $apt = any2obj('property', $apt_or_id);
  $id  = $apt->id;
  if(!$size) $size = 'small';
  $img_size = $_SESSION['media_'.$size.'_size'];
  if(!$img_size) {
    dump($_SESSION);
  }
  if($image = find_object('media', array('parent_id' => $id, 'active' => 1))) {
    $image_src  = get_media_src($image, $size); // hack for now: small images from AFR are square and ugly
    if($options['return'] == 'source') return $image_src;
    $image_id = "title_photo_".$id;
    return html_image($image_src, $apt->name, array('id' => $image_id, 'class' => $class, 'width' => $img_size));
  } else {
    return ($default ? html_div(phrase('no_photo_available', CAPITALIZE), 'no_photo') : "");
  }
}

function image_size_dir($size, $options=NULL) {
  if(!$size) return '';
  switch($size) {
    //case 'big':
    //  return '';
    case 'thumb':
      return 'thumbs/';
    default:
      return "$size/";
  }
}

function image_type_dir($type) {
  switch($type) {
    case "hotel":
      return "hotels/";
    case "boat":
      return "boats/";
    case "office":
      return "offices/";
    case "shortstay_office":
      return "shortstay-offices/";
    case "shortstay":
      return "serviced-apartments/";
    case "longstay":
      return "rental-apartments/";
    case "realestate":
      return "real-estate/";
    case "property":
      return "properties/";
    default:
      die("image_type_dir: missing or incorrect type '$type'");
    break;
  }
}


// returns file system directory for images of given property or room
// takes apt od and does DB lookup to find city, source, agent, owner etc
/** unsused - automatic comment added on  */
function get_image_dir($id, $type, $room_id=0) {
  global $db;
  if(!$type) die("get_image_dir: missing type");
  $subdir = image_type_dir($type);
  if(!$subdir) die("get_image_dir: no subdir");
  $apt = db_getRow("SELECT id, city_id, source_id, estate_agent_id, owner_id FROM property WHERE id=$id");
  $tail = get_image_path_tail($apt, $room_id);
  return $_SESSION['media_dir']."$subdir/$tail";
}

// returns file system directory for images of given property or room
// takes apt object and array or media to find city, source, agent, owner etc
/** unsused - automatic comment added on  */
function build_image_dir($id, $type, $media, $room_id=0) {
  global $db;
  if(!$type) die("get_image_dir: missing type");
  $subdir = image_type_dir($type);
  if(!$subdir) die("get_image_dir: no subdir");
  $apt = $media;
  $apt['id'] = $id;
  $tail = get_image_path_tail($apt, $room_id);
  return $_SESSION['media_dir']."$subdir/$tail";
}

// returns web URL for images of given property or room
/** unsused - automatic comment added on  */
function get_image_path($id, $type, $room_id=0) {
  global $db;
  if(!$id) return ''; //die("get_image_path: missing id");
  if(!$type) return ''; // die("get_image_path: missing type");
  $subdir = image_type_dir($type);
  if(!$subdir) die("get_image_path: no subdir");
  $table = 'property';
  $qs = "SELECT id, city_id, source_id, estate_agent_id, owner_id FROM $table WHERE id=$id";
  $apt = db_getRow($qs);
  //print_debug("get_image_path query:<br>$qs;<br>");
  $tail = get_image_path_tail($apt, $room_id);
  //if(!$tail) die("get_image_path: no tail");
  return str_replace('//', '/', $_SESSION['media_path']."$subdir/$tail");
}

// returns 'the last bit' of the image path for properties
function get_image_path_tail($apt, $room_id=0) {
  if(is_array($apt)) $apt = array2obj($apt);
  $sid = $apt->source_id;
  $cid = $apt->city_id;
  $aid = $apt->estate_agent_id;
  $oid = $apt->owner_id;
  $id = $apt->id;
  if($aid) {
    $tail = "$sid/$cid/$aid/$id/";
  } elseif($oid) {
    $tail = "$sid/$cid/$oid/$id/";
  } else {
    //echo("get_image_path: neither estate_agent_id or owner_id defined<br>");
    $tail = "";
  }
  if($room_id) $tail .= "$room_id/";
  return $tail;
}

// downloads all photo from $url_array
// adds to table media
// resizes and builds gallery
// apt is assoc. array
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_property_photos($type, $apt, $imageArray, $room_id=0) {
  foreach($imageArray as $media) add_object('media', $media);
}


// same as built-in getimagesize(), but removes query string first
/** unsused - automatic comment added on  */
function imagesize($src) {
  $stripped_src = ($pos = strpos($src, '?')) ? substr($src, 0, $pos) : $src;
  return getimagesize($stripped_src);
}


// cover Rahul's cases
// Maximizes screen realestate according to specified aspect ratio (default is 16/9)
// only used for large images where aspect ratio is less than 16/9
/** unsused - automatic comment added on  */
function smart_resize($file, $dst, $long=1920, $options=array()) {
  if($long < 1920) return  resize_image($file, $dst, $long, $options);  // only used for HD images
  
  $inv_aspect = $options['inv_aspect'] ?: 0.5625; // for a 16/9, use 9/16
  
  $image = new Imagick($file);
  if(!$image) return false;
  $ih = $image->getImageHeight();
  $iw = $image->getImageWidth();

  $short = $inv_aspect * $long; // the desired short side according to 16/9 aspect ratio

  $short_side = min($ih,$iw); // short side of input image
  $long_side = max($ih,$iw);  // long side of input image
  
  $image_aspect = $long_side / $short_side; // the aspect ratio of the input image we want to resize
  $aspect = 1/$inv_aspect; // = 1.777777, the HD 16/9 aspect ratio we optimize for

  if($image_aspect < $aspect) { // set short side to 1080 if aspect is less than 16/9 (ie, more square than wide screen)
    // $calc_long = ($short / $short_side) * $long_side;
    $calc_long = $short * $image_aspect;
    $calc_short = round($calc_long / $image_aspect);
    print_log("smart-resize: original h/w = ($ih/$iw) size=$long calc-long=$calc_long short=$calc_short aspect=$image_aspect", 'resize-image', LOG_LEVEL_TEST);
    return resize_image($file, $dst, $calc_long, $options);
  } else { // set long side to 1920
    $calc_short = round($long / $image_aspect);
    print_log("smart-resize: original h/w = ($ih/$iw) long=$long short=$calc_short aspect=$image_aspect", 'resize-image', LOG_LEVEL_TEST);
    return resize_image($file, $dst, $long, $options);
  }
}

// todo: test non-square resize
function resize_image($file, $dst, $size, $options='') {
  if(!$size) return copy($file, $dst); // just copy it
  if(is_array($size)) {
    if(count($size) == 2) list($w, $h) = $size;
    else if(count($size) == 4) list($w, $h, $x, $y) = $size;
  } else {    
    $w = $h = $size;
  }
  
  $bestfit  = isset($options['bestfit']) ? $options['bestfit'] : true;
  $crop     = $options['crop'];
  $blur     = isset($options['blur']) ? $options['blur'] : 1;
  $strip    = isset($options['strip']) ? $options['strip'] : 1;
  $quality  = $options['quality'] ?: 80;
  $filter   = $options['filter'] ?: Imagick::FILTER_LANCZOS;

  $image = new Imagick($file);
  if(!$image) return false;
  $ih = $image->getImageHeight();
  $iw = $image->getImageWidth();

  
     
  if($crop) { // resizes image and returns the cropped selection (aspect ratio is maintained)
    $long_side = max($w, $h); 
    $result = $ih <= $iw ? $image->resizeImage(0, $long_side, $filter, $blur) : $image->resizeImage($long_side, 0, $filter, $blur); // Resize image algorithm based on shortest side (bestfit off)
    $rih = $image->getImageHeight(); // result height
    $riw = $image->getImageWidth();  // result width

    if(!isset($x)) $x = $riw > $w ? ($riw - $w) /2 : 0; // default crop to center of image
    if(!isset($y)) $y = $rih > $h ? ($rih - $h) /2 : 0; // default crop to center of image
    if($result) $result = $image->cropImage($w, $h, $x, $y);

    // resize so that the shortest side of the result is the longest side of the input
    //if($ih <= $iw) {  // landscape
    //} else {  // portrait
    //  $result = $image->resizeImage($long_side, 0, $filter, $blur, $bestfit); // Resize image algorithm based on width
    //}
    
  } else { // if bestfit is set (default), aspect ratio is maintained, the long side will be set to $w or $h 
    $result = $image->resizeImage($w, $h, $filter, $blur, $bestfit); // Resize image based on width and height    
  }
  
  if($result) {
    $image->setImageCompression(Imagick::COMPRESSION_JPEG); // Set to use jpeg compression
    $image->setImageCompressionQuality($quality); // Set compression level (1 lowest quality, 100 highest quality)
    if($strip) $image->stripImage(); // Strip out unneeded meta data
    $result = $image->writeImage($dst); // Writes resultant image to output directory  
  }
  $image->destroy(); // Destroys Imagick object, freeing allocated resources in the process
  if($result) print_log("Resized $file to $dst (size=$size)", 'resize', LOG_LEVEL_TEST);
  return $result;
}

/** unsused - automatic comment added on  */
function getGDversion() {
  $gd_version_number = $_SESSION['gd_version_number'];
  if ($gd_version_number === null) { // look up once per session
    ob_start();
    phpinfo(8);
    $module_info = ob_get_contents();
    ob_end_clean();
    if (preg_match("/\bgd\s+version\b[^\d\n\r]+?([\d\.]+)/i", $module_info,$matches)) {
      $gd_version_number = $matches[1];
    } else {
      $gd_version_number = 0;
    }
    //echo("Looked up GD version number = $gd_version_number<br>");
    $_SESSION['gd_version_number'] = $gd_version_number;
  }
  return $gd_version_number;
} 

function ColorReplace($url, $color, $save_to='')
{
    if($save_to && file_exists($save_to)) return $url; // already exists
    # The new color and its channels
    $i=$color; 
    $color = pad_color($color); // convert FFF to FFFFFF
    
    //die("color=$color, was $i<br>");
    $r = 0+("0x".substr( $color, 0, 2));
    $g = 0+("0x".substr( $color, 2, 2));
    $b = 0+("0x".substr( $color, 4, 2));
    # I read the kind of file from the extension
    $tmp = pathinfo($url);
    $extension =$tmp['extension'];
    if( $extension == 'png' ) $oldIm = @imagecreatefrompng( $url );
    elseif( $extension == 'gif' ) $oldIm = @imagecreatefromgif( $url );
   
    if(!$oldIm) return;
    //echo("url=$url color=$color ext=$extension<br>");
    //dump($oldIm);
    # I replace EACH color in the palette with the new one
    for( $i=0; $i < imagecolorstotal( $oldIm ); $i++ )
        imagecolorset( $oldIm, $i, $r,$g,$b );

    if($save_to) { # Save the result
      if( $extension == 'gif' ) $result = @imagegif($oldIm, $save_to);
      elseif( $extension == 'png' ) $result = @imagepng($oldIm, $save_to);
      //echo("ColorReplace; converted $url of type=$extension to color='$color'. Saving to '$save_to' <br>");
      return $result;
    } else {
      # Output the file in png format : each color is now NEW COLOR
      # but transparency is preserved !!
      header("Content-type: image/png");
      imagepng($oldIm);
      imagedestroy($oldIm);
    }
}
// takes 3 or 6  - almost same as fix_color in helpers.inc
function pad_color($color) {
  //if(!is_color($color)) return $color;
  if(strlen($color) == 6) return $color;
  $R = substr($color, 0,1);
  $G = substr($color, 1,1);
  $B = substr($color, 2,1);
  return "$R$R$G$G$B$B";
}

/** unsused - automatic comment added on  */
function get_gallery_media($apt) {
  $searchArray['active'] = 1;
  $searchArray['parent_id'] = $apt->id;
  $searchArray['property_type'] = $apt->type;
  $searchArray['descriptiontype_id'] = DESC_TYPE_ALBUM; // photo album;
  return find_objects('media', $searchArray, array('sort' => 'rank'));
}

function build_gallery($type, $id) {
  //include_once("classes/gallery.class.inc");
  if(!is_property_type($type)) return; 
  
  $apt = get_object($type, $id);
  $output = "Building gallery for $apt->name<br>";
  
  // Options used by flash player
  $options['maxImageWidth'] = $_SESSION['media_big_size'];
  $options['maxImageHeight'] = $_SESSION['media_big_size'];
  $options['textColor'] = "0xffffff";
  $options['frameColor'] = "0xffffff";
  $options['frameWidth'] = "4";
  $options['stagePadding'] = "20";
  $options['thumbnailColumns'] = "4";
  $options['thumbnailRows'] = "1";
  $options['navPosition'] = "bottom";
  $options['title'] = "";
  $options['enableRightClickOpen'] = "false";
  $options['backgroundImagePath'] = "";
  $options['showDownloadLinks'] = false;

  $searchArray['parent_id'] = $id;
  $searchArray['property_type'] = $type;
  $searchArray['descriptiontype_id'] = DESC_TYPE_ALBUM; // photo album;
  $searchArray['active'] = 1;
  if($images = find_objects('media', $searchArray, array('sort' => 'rank'))) {
    $first = $images[0];
    $imagedir = get_media_file($first);
    $imagepath  = get_media_src($first);
    $count = count($images);    
    $options['imagePath'] = $imagepath;
    $options['thumbPath'] = $imagepath.image_size_dir('thumb');
    $output .= "$count images found for $apt->name, writing to $imagedir<br>";
  
    $files = array();
    foreach($images as $image) {
      $file['name'] = $image->src;
      $file['caption'] = $image->comment;
      $files[]= $file;
    }    
    $output .= write_gallery_xml($imagedir, $files, $options);

  } else {
    $output .= "No images found for $apt->name, deleting gallery<br>";
    unlink($imagedir.'gallery.xml'); // delete if there
  }
  return $output; 
}

// class GalleryXml creates gallery.xml file
function write_gallery_xml($path, $files, $options) {
  $xml = '<?xml version="1.0" encoding="UTF-8"'." ?>\n";
  $xml .= getXmlOptions($options)."\n";
  foreach ($files as  $file)
  {
    $xml .= "<image>\n";
    $xml .= "\t<filename>".$file['name']."</filename>\n";
    if ($file['caption']) $xml .= "\t<caption><![CDATA[".$file['caption']."]]></caption>\n";
    if ($options['showDownloadLinks']) $xml .= "\t".'<caption><![CDATA[<a href="'.$options['imagePath'].'/'.$fileName.'" target="_blank"><u>Open image in new window</u></a>]]></caption>'."\n";
  	$xml .= "</image>\n";
  }
  $xml .= "</simpleviewerGallery>\n";

  $file = $path."/gallery.xml";
  $output .= form_text('foo', $xml);
  if (file_exists($file)) @chmod($file, 0777);
  if($b = write_file($file, $xml)) {
    $output .= "Wrote $b bytes to $file<br>";
  } else {
    $output .= "Couldn't write to $file<br>";
  }
  @fclose($file_handle);
  @chmod($file, 0777);
  return $output;
}
  
  //if (!$file_handle = @fopen($file,"w")) {
  //  $output .= '<p class="error">Cannot open XML document: '.$file.'. Change permissions to 0777 for $file and parent directory.</p>';
  //} elseif (!@fwrite($file_handle, $xml)) {
  //  $output .= '<p class="error">Cannot write to XML document: '.$file.'. Change permissions to 0777 for file and parent directory.<p>';
  //} else {
  //  $output .= '<p>Successfully created XML document: <em>'.$file.'</em></p>';
  //}

// helper function for write_gallery_xml
function getXmlOptions($options)
{
  $xmlOptions = '<simpleviewerGallery';
  foreach ($options as $optName => $optValue) $xmlOptions .= ' '.$optName.' = "'.$optValue.'"';
  $xmlOptions .= '>';
  return $xmlOptions;
}

function  media_directory($siteObj, $mediaObj, $options=NULL) {
  $imagepath = trim($options['path']) ?: $mediaObj->path; // path - must be set 
  if($imagepath) return ''; // array(false, array('resize_media: No path found.'));
  $site_dir = site_directory($siteObj);
  return clean_path($site_dir.$imagepath);
}

/** unsused - automatic comment added on  */
function media_info($mediaObj_or_id, $options=NULL) {
  if(!$mediaObj_or_id) return '';
  if(is_object($mediaObj_or_id) || is_array($mediaObj_or_id) || isint($mediaObj_or_id)) { // media object or ID
    $mediaObj = any2obj('media', $mediaObj_or_id);
  } else if(strlen($mediaObj_or_id)) { // URL or list of IDs or URLs
    $url_or_id = array_shift(explode(',', $mediaObj_or_id));
    if(isint($url_or_id)) {
      $mediaObj = get_object('media', $url_or_id);
    } else { // must be URL
      return $url_or_id;
    }
  }
  
  if(!$mediaObj)  return ''; 
  $type = $mediaObj->type;            
                      
  $lines[phrase('type', CAPITALIZE)] = $type; 
  $lines[phrase('mime', CAPITALIZE)] = $mediaObj->mime;
  if($name = $mediaObj->name) $lines[phrase('title', CAPITALIZE)] = $name;
  
  if($user_id = $mediaObj->user_id) $lines[phrase('uploaded_by', CAPITALIZE)] = get_user_name($user_id).' '.sql2human($mediaObj->created); 
  if($mediaObj->parent_type) $lines[phrase('parent', CAPITALIZE)] = implode(' ', array_filter(array($mediaObj->parent_type, $mediaObj->parent_id, $mediaObj->parent_field))); 
  if($size = $mediaObj->filesize) $lines[phrase('file_size', CAPITALIZE)] = bytes2human($size);
  switch($type) {
    case 'image':
      if($mediaObj->height) $lines[phrase('height', CAPITALIZE)] = $mediaObj->height."px"; 
      if($mediaObj->width) $lines[phrase('width', CAPITALIZE)] = $mediaObj->width."px"; 
      if($mediaObj->depth) $lines[phrase('depth', CAPITALIZE)] = $mediaObj->depth." bits"; 
      if($mediaObj->channels) $lines[phrase('channels', CAPITALIZE)] = $mediaObj->channels; 
      break; 
    case 'audio':
      if($mediaObj->length) $lines[phrase('length', CAPITALIZE)] = $mediaObj->length."s"; 
      if($mediaObj->sample_rate) $lines[phrase('sample_rate', CAPITALIZE)] = $mediaObj->sample_rate." Hz"; 
      if($mediaObj->bit_rate) $lines[phrase('bit_rate', CAPITALIZE)] = $mediaObj->bit_rate." bps"; 
      if($mediaObj->audio_codec) $lines[phrase('audio_codec', CAPITALIZE)] = $mediaObj->audio_codec; 
      if($mediaObj->length) $lines[phrase('length', CAPITALIZE)] = $mediaObj->length." s"; 
      if($mediaObj->channels) $lines[phrase('channels', CAPITALIZE)] = $mediaObj->channels; 
      break; 
    case 'video':
      if($mediaObj->height) $lines[phrase('height', CAPITALIZE)] = $mediaObj->height."px"; 
      if($mediaObj->width) $lines[phrase('width', CAPITALIZE)] = $mediaObj->width."px"; 
      if($mediaObj->frame_rate) $lines[phrase('frame_rate', CAPITALIZE)] = $mediaObj->frame_rate." fps"; 
      if($mediaObj->bit_rate) $lines[phrase('bit_rate', CAPITALIZE)] = $mediaObj->bit_rate." bps"; 
      if($mediaObj->video_codec) $lines[phrase('video_codec', CAPITALIZE)] = $mediaObj->video_codec; 
      if($mediaObj->audio_codec) $lines[phrase('audio_codec', CAPITALIZE)] = $mediaObj->audio_codec; 
      if($mediaObj->channels) $lines[phrase('channels', CAPITALIZE)] = $mediaObj->channels;
      break;                 
    case 'application':
      break;
    case 'text':
      break;
  }
  foreach($lines as $title => $val) $items[] = html_b($title).": ".$val;      
  return html_ul($items);
  
}

// moves media to another path (relative path, not full system dir)
// called from pre_edit (assumes mediaObj still has current path)
function move_media($mediaObj_or_id, $new_path) {
  if(!$mediaObj_or_id) return array(0, array("move_media: no obj or id received"));
  $log = array();  
  $errors = array();

  $obj = any2obj('media', $mediaObj_or_id);
  if(!$obj) return array(0, array("move_media: no object found"));
  $id = $obj->id;

  $site_id = $obj->site_id ?: $_SESSION['site_id'];
  $type = $obj->type;
  
  global $resizeArray;
	//$obj_type = 'media';
	//$desc_type = $obj->descriptiontype_id;
	//$ptype = pick_first($obj->parent_type, $obj->property_type); 


  // move image on disk if path changes

  $old_path = $obj->path;
  $siteObj = get_object('site', $site_id);
  
  $doc_root = site_directory($siteObj) ?: $_SERVER['DOCUMENT_ROOT'];    
            

  $sizes = $obj->resize ? array_keys($resizeArray) : array('');
  $log[]= "move_media $id to $new_path docroot=$doc_root, sizes=".implode(',', $sizes)."\n"; // .print_r($changes, true), 'a');
  
  foreach($sizes as $size) {
    $dir = image_size_dir($size);
    $old_dir = clean_path($doc_root.$old_path.$dir);
    $new_dir = clean_path($doc_root.$new_path.$dir);
    $log[] = "old=$old_dir\nnew=$new_dir";
    if(!is_dir($new_dir) && !mkdir_recursive($new_dir)) $errors[] = "Error: couldn't make $new_dir";
    if(is_dir($old_dir) && is_dir($new_dir)) {
      $src = $size == 'org' ? $obj->filename : $obj->src;
      $old_src = $old_dir.$src;                
      $new_src = $new_dir.$src;                

      if($src && is_file($old_src)) {
        if(rename($old_src, $new_src)) {
          $log[]= "OK: renamed src ($old_src, $new_src)";
        } else {
          $errors[]= "Error: could not rename src ($old_src, $new_src)";
        }
      } else {
        $errors[] = "Error: couldn't find $old_src";
      }

      // this shouldn't be needed
      $file = $obj->filename;
      $old_file = $old_dir.$file;
      $new_file = $new_dir.$file;
      if($file && $src != $file && is_file($old_file)) {
        $log []= "rename file ($old_file, $new_file)";
        if(rename($old_file, $new_file)) {
          $log[]= "Ok: renamed file ($old_file, $new_file)";
        } else {
          $errors[]= "Error: could not rename file ($old_file, $new_file)";
        }
      }
    } else {
      $errors[] = "Error: Missing $old_dir or $new_dir";
    }
  }    
   
  print_log("Done running move_media Details:".print_r($log, true));
  if($errors) print_log("move_media errors: ".print_r($errors, true));
  
  $result = $errors ? false : true;
  return array($result, $errors);
  
  // echo("pre_edit_media: path=$path<br>");
  //$dirs = array('', 'thumbs', 'small', 'big', 'full', 'org');
  //foreach($dirs as $dir) {
  //  $new_dir = clean_path($_SERVER['DOCUMENT_ROOT'].$path.($dir ? "$dir/" : ''));
  //  if(!is_dir($new_dir) && mkdir_recursive($new_dir)) return array(false, array("Couldn't make directory $new_dir"));
  //  //echo("made dir $new_dir<br>");
  //}

}

// takes comma separated of urls, returns json for media_json field
function media_list2json($list, $options = []) {
  $mediaAr = [];
  $urls = explode(',', $list);
  foreach($urls as $url)  $mediaAr[] = ['type' => 'image', 'filename' => $url];
  return json_encode($mediaAr);
}


// returns json encoded array of media for an object
function media2json($parent_type, $parentObj_or_id, $options = []) {
  $parentObj = any2obj($parent_type, $parentObj_or_id);
  $parent_id = $parentObj->id;
  $parent_field = isset($options['parent_field']) ? $options['parent_field'] : 'media';
  $size = isset($options['size']) ? $options['size'] : 'square';
  
  $mediaAr = [];
  
  $val = $parentObj->$parent_field;
  
  if(strpos($val, '//') !== false) { // string of urls
    return media_list2json($parentObj->media);
  } elseif(is_whole_number($val)) { // just one image
    $mediaAr[] = media2json_single($val, $options);
    print_log("media2json pt=$parent_type single val={$val}", 'media', LOG_LEVEL_TEST);
    
  } else {
    $search = ['parent_type' => $parent_type,'parent_id' => $parent_id, 'parent_field' => $parent_field, 'active' => 1];
    $mediaObjs = find_objects('media', $search, ['sort' => 'rank']);
    if(!$mediaObjs) return '[]';
    foreach($mediaObjs as $mediaObj) {          
      $mediaAr[] = media2json_single($mediaObj, $options);
    }    
  }
  return json_encode($mediaAr);
}

// helper for media2json
function media2json_single($mediaObj_or_id, $options=[]) {
  $mediaObj = any2obj('media', $mediaObj_or_id);
  $mObj = new stdClass;
  if($mediaObj->cloud_url) $mObj->cloud = 1;
  if($mediaObj->processed) $mObj->ms = 1;
  
  $print_options = ['source_only' => 1];
  if($mediaObj->resize && $size = $options['size']) $print_options['size'] = $size;        
  $mObj->filename = $filename = print_media($mediaObj, $print_options); // variable name for historical reasons: as in golf_club from leading courses
  
  foreach(['id', 'name', 'type'] as $f) if(strlen($mediaObj->$f)) $mObj->$f = $mediaObj->$f;
  
  print_log("media2json id={$mediaObj->id} fn={$filename} mObj:".dump($mObj,true), 'media', LOG_LEVEL_TEST);
  
  return $mObj;
}
// returns an array of the src of the medias in list.
function media_json_sources($json, $size='square') {
  $sources = [];
  $mediaObjs = json_decode($json);
  foreach($mediaObjs as $mediaObj) {
    $src = $size == 'square' ? $mediaObj->filename : str_replace("/square/", "/{$size}/", $mediaObj->filename);
    $sources[] = $src;
  }
  timer('property-search', "media json");
  return $sources;
}

// returns an array of the src of the medias in list.
/** slow - use above media_json_sources instead */
function get_media_sources($list, $size='small') {
  //return '';
  if(!$list) return array();
  $ids = explode(",", $list) ?: array();
  $options['size'] = $size;
  $options['source_only'] = true;
  $sources = array();
  foreach($ids as $id) {
    $sources[] = print_media($id, $options);
  }  
  timer('property-search', "media sources");  
  return $sources;
}

// returns the src of the mediaObj or id. If it's a list (comma separated), uses the first  
function get_media_source($mediaObj_or_id, $size='small', $options=[]) {
  $options['size'] = $size;
  $options['source_only'] = true;
  return print_media($mediaObj_or_id, $options);
}

/** unsused - automatic comment added on  */
function first_media_source($media, $size='small') {
  $options['size'] = $size;
  $options['source_only'] = true;
  $images = $media ? array_filter(explode(',', $media)) : array();
  return count($images) ? print_media($images[0], $options) : '';
}

/** Print list of media: photos, audio, video, documents */
/** unsused - automatic comment added on  */
function print_media_list($media, $options=NULL) { 
  if(!$media) return '';
  $output = "";
  $ids = explode(',', $media);
  foreach($ids as $id) {
    $mediaObj = get_object('media', $id);
    $mediaObj->type = 'application'; // force list view
    $output .= print_media($mediaObj).html_break();
  }
  return $output;
}

/** query string used by Glide to automatically served scaled image on Media Server */     
function image_size_query($size) {
  switch($size) {
    case 'org'   : return ''; // array(0, 'org'), 
    case 'tiny'  : return '?w=40&w=40&fit=crop'; // array($_SESSION['tiny_pic_width'] ?: 40, 'tiny', array('crop' => 1)),                        
    case 'thumb' : return '?w=100'; //  array($_SESSION['thumb_pic_width'] ?: 100, 'thumbs'),
    case 'square': return '?w=200&h=200&fit=crop'; // array($_SESSION['square_pic_width'] ?: 200, 'square', array('crop' => 1)),                        
    case 'small' : return '?w=200'; //  array($_SESSION['small_pic_width'] ?: 200, 'small'), 
    case 'medium': return '?w=640'; // array($_SESSION['medium_pic_width'] ?: 640, 'medium'), 
    case 'ig'    : return '?w=640&h=640&fit=crop'; //     array($_SESSION['ig_pic_width'] ?: 640, 'ig', array('crop' => 1)), // instagram square
    case 'big'   : return '?w=1280'; // array($_SESSION['big_pic_width']   ?: 1280, 'big'), 
    case 'full'  : return '?w=1920'; // array($_SESSION['full_pic_width']  ?: 1920, 'full'));
    default: return '';
  }
}

// http://res.cloudinary.com/demo/image/upload/w_400,h_400,c_crop,g_face,r_max/w_200/lady.jpg
/** transformation used by cloudinary to automatically served scaled image  */
http://res.cloudinary.com/yah/image/upload/w_400,h_400,c_crop,g_face,r_max/w_200/1120008.jpg

function cloudinary_transform_url($url, $params=[]) {
  if(!$params) return $url;
  return $url;  
}

function cloudinary_size_transformation($size) {
  switch($size) {
    case 'org'   : return []; 
    case 'tiny'  : return ['width' => 40, 'width' => 40, 'crop' => 1];                        
    case 'thumb' : return ['width' => 100];
    case 'square': return ['width' => 200, 'width' => 200, 'crop' => 1];                        
    case 'small' : return ['width' => 200]; 
    case 'medium': return ['width' => 640]; 
    case 'ig'    : return ['width' => 640, 'width' => 640, 'crop' => 1];
    case 'big'   : return ['width' => 1280]; 
    case 'full'  : return ['width' => 1920];
    default: return '';
  }
}

// returns either the path (src) or image for given mediaObject (or ID)
// if neither object nor id, assume a string containing the path
function print_media($mediaObj_or_id, $options=NULL) {
  global $__SITE;
  $full = $options['full'] ? 1 : 0;
  //$full = true; 

  if(!$mediaObj_or_id) return '';

  $url = 0;
  
  if(is_object($mediaObj_or_id) || is_array($mediaObj_or_id) || isint($mediaObj_or_id)) { // media object or ID
    $mediaObj = any2obj('media', $mediaObj_or_id);
  } else if(strlen($mediaObj_or_id)) { // URL or list of IDs or URLs or JSON
    if(isJson($mediaObj_or_id)) {
      $images = json_decode($mediaObj_or_id);
      if(!count($images)) return '';
      $first = $images[0];
      $filename = is_object($first) ? $first->filename : $first;
      return print_media($filename, $options);
    }
    
    $url_or_id = array_shift(explode(',', $mediaObj_or_id));
    if(isint($url_or_id)) {
      $mediaObj = get_object('media', $url_or_id);
    } else { // must be URL
      $url = 1; 
      $src = str_replace(array('http://', 'https://'), '//', $url_or_id);    
      return $options['source_only'] ? $src : html_image($src, basename($src), $options);
    }
  }
  
  $debug = "?mid={$mediaObj->id}&p={$mediaObj->processed}";
  
  if($mediaObj) { 
    $size = isset($options['size']) ? $options['size'] : '';
    $media_server = $mediaObj->processed && $_SESSION['media_server'] ? 1 : 0;
    $debug .= "&ms={$media_server}";

    if($mediaObj->resize == 1 && !$size & $size !== false) $size = 'medium'; // default
    if(!$mediaObj->resize || $mediaObj->resize > 1) $size = '';

    $cloud_url = $mediaObj->cloud_url;    
    if($_SESSION['cloudinary'] && $cloud_url) {
      $src = cloudinary_transform_url($cloud_url, cloudinary_size_transformation($size));
      $url = 1;
    } elseif($media_server) {
      $full = 1;      
      $src = "{$mediaObj->path}{$mediaObj->src}";
    } else {
      $size_dir = $size ? image_size_dir($size) : '';    
      $src = $mediaObj->path.$size_dir.($size == 'org' ? $mediaObj->filename : $mediaObj->src);
    }

    switch($mediaObj->type) {
      case 'image':
        $default = '/images/icons/media.png';
        $html_media = html_image($src ?: $default, $name, $options);
        break;
      case 'video':
        $default = '/images/icons/video.png';
        $html_media = html5_video($src ?: $default, $name, $options);
        break;
      case 'audio':
        $default = '/images/icons/audio.png';
        $html_media = html5_audio($src ?: $default, $name, $options);
        break;
      case 'application':
        //$icon_class = find_object('mime_type', array('mime' => $mediaObj->mime), 'icon') ?: 'file-o';
        $icon_class = file_awesome_class($mediaObj->src);
        //$debug = "class of {$mediaObj->src} = '{$icon_class}' ---- ";
        $icon = html_i('', "fa fa-file-{$icon_class}-o");
        $name = $mediaObj->name ?: $mediaObj->filename;
        $html_media = html_link($src, "$icon $name", array("download" => "download"))." ".bytes2human($mediaObj->filesize);
      default:
        break;
    }
  }
  
  $debug .= "&full=$full&url=$url";
  if($full && !$url) { /** we have a relative path and want a fully qualified URL with host */
    $site_id = $mediaObj->site_id ?: $__SITE->id;
    if($media_server) {
      $domain = $site_id == $__SITE->id ? $__SITE->domain : get_object('site', $site_id, 'domain');
      $site_url = "//media.{$domain}";    
    } else {
      $site_url = site_full_url($site_id);
    }
    $src = $site_url.$src;
    if($size && $media_server) $src .= image_size_query($size);
  }
  
  
  $src = str_replace(array('http://', 'https://'), '//', $src);
  //$src .= $debug;
  $output = $options['source_only'] ? $src : $html_media;
  return $output;
}


function rotate_media($mediaObj_or_id, $degrees, $options=NULL) {
  $outputAr = array();
  if(is_object($mediaObj_or_id) || is_array($mediaObj_or_id) || isint($mediaObj_or_id)) { // media object or ID
    $mediaObj = any2obj('media', $mediaObj_or_id);
  } else if(strlen($mediaObj_or_id)) { // URL or list of IDs or URLs
    $url_or_ids = explode(',', $mediaObj_or_id);
    if(count($url_or_ids)) {
      foreach($url_or_ids as $url_or_id) {
        $output .= rotate_media($url_or_id, $degrees, $options);
      }
    } else { // src/url, not handled yet
      $src = $mediaObj_or_id;
      $output = "Did not rotate $src $degrees degrees<br>";
    }
  }
  if($mediaObj) {
    $src = $mediaObj->src;
    $path = $mediaObj->path;
    $title = $mediaObj->name ?: basename($mediaObj->src);
    if($mediaObj->resize) {
      global $resizeArray;
      foreach($resizeArray as $size => $resizeOptions) {
        if($size == 'org') continue; // don't mess with orginal
        $dir = $resizeOptions[1];
        $full_path = clean_path($_SERVER['DOCUMENT_ROOT'].$path.$dir.'/'.$src);
        if(is_file($full_path)) {
          $result = rotate_image($full_path, $degrees);
          //$outputAr[]= "Rotated $full_path $degrees degrees";
        } else {
          $outputAr[]= html_b("$full_path not found");
        }
  
      }
    } else {
      $src = clean_path($path.$dir.$src);
      //$outputAr[]= "Rotated $title $degrees degrees";
    }    
  }
  $outputAr[]= "Rotated $title $degrees degrees";
  return implode(html_break(), $outputAr);
}

function rotate_image($full_path, $degrees) {
  $imagick = new Imagick();
  if(!is_file($full_path)) return false;
  
  $imagick->readImage($full_path); 
  $imagick->rotateImage(new ImagickPixel('none'), $degrees); 
  $imagick->writeImage($full_path); 
  $imagick->clear(); 
  $imagick->destroy(); 
  
  return true;
}
  
// resizes a media object according to specified (gallery) options
// also optimizes image
function resize_media($siteObj, $mediaObj, $options=NULL) {    
  $debug = true;
  $resized = false;
  global $resizeArray;   
  global $log_dir;
  $filename	=	"resize_log.txt";
  $log = "$log_dir/$filename";
  $mid = $mediaObj->id;
  
  $max_size = 0;
  $errors = array();
  $messages = array();
  $resizeAr = $options['resize']; // array where key->value is directory->size
  
  if(!$resizeAr) return array(false, array("resize_media: no resize array"), $messages);
  $optimize = isset($options['optimize']) ? $options['optimize'] : true; // optimize by default

  print_log("resize_media: options=".dump($options, true), 'resize', LOG_LEVEL_TEST);

  if($resizeAr === 1 || $resizeAr === true) $resizeAr = $resizeArray; // use global standard
  else if (is_object($resizeAr)) $resizeAr = (array) $resizeAr;
  else if (!is_whole_number($resizeAr) && !is_array($resizeAr)) return array(false, array("resize_media: resize is neither whole number nor array"), $messages);
  
  $skip_small = isset($options['skip_small']) ? $options['skip_small'] : false;
  $update = isset($options['update']) ? $options['update'] : true;
  $imagedir  = trim($options['directory']) ?: media_directory($siteObj, $mediaObj, $options); // file system full path - must be set 
  if(!$imagedir) return array(false, array('resize_media: No dir found.'));
  
  $src = trim($mediaObj->src);
  if(!$src) return array(false, array('resize_media: No src for media.'));

  if(is_whole_number($resizeAr)) { /** resize the original, leave in base directory */
    $max_size = $resizeAr;
    $org_src = $imagedir.$src;
    $resizeAr = array('' => $max_size); // make array for use below
    if(!is_file($imagedir.$src)) return array(false, array("resize_media: resize is whole number ($max_size) but org_src $org_src does not exist"), $messages);
    print_log("$org_src exists, resize in place to $max_size", 'resize', LOG_LEVEL_TEST);

  } else { /** resize array: create multiple directories */

    print_log("Resize array", 'resize', LOG_LEVEL_TEST);

    // first move uploaded file to org directory with our name
    // move orginal to org folder if needed
    $uploaded_file = $mediaObj->uploaded_file ?: $imagedir.$mediaObj->filename;
  
    $org_dir = $imagedir.'org/';
    $org_src = $org_dir.$mediaObj->filename;
  
    //return array(false, array("uf=".$mediaObj->uploaded_file." file=".$mediaObj->filename." dir=$org_dir"));
    
    if(is_file($uploaded_file)) { // org file is in base directory, move it to org
      if(!is_dir($org_dir) && !mkdir_recursive($org_dir)) return array(false, array("Couldn't make directory $org_dir"));
      $messages[] = "<span class='red'>Couldn't find $org_dir, created it</span>";
  
      if(!rename($uploaded_file, $org_src)) return array(false, array("Couldn't move $uploaded_file to $org_dir"));
      $messages[] = "<span class='red'>Moved original $uploaded_file to $org_src</span>";
  
    } else if(is_file($org_src)) { // org file is in base directory, move it to org
      $messages[] = "<span class='green'>".$mediaObj->filename." is already in $org_dir</span>";
    } else {
      return array(false, array("resize_media: could not find original in $uploaded_file or $org_src"));
    }
  
    //print_log(dump($messages, true));
    
    // find original, otherwise use full or big
    $keys = array('org', 'full', 'big');
    print_log("Starting resize", 'resize', LOG_LEVEL_TEST);
    foreach($keys as $org_key) {
      list($size, $dir, $resize_options) = $resizeAr[$org_key];
      $org_src = $imagedir."$dir/".($org_key == 'org' ? $mediaObj->filename : $src);
      if(is_file($org_src)) break; //found it
      $org_src = NULL;
    }
  
    if(!$org_src) return array(false, array("resize_media: No original found for $src ($mediaObj->id), nor full or big."));
  
    print_log("OK: org_src=$org_src", 'resize', LOG_LEVEL_TEST);

    // warn if original is too low res?
    if($options['preview']) {
      $preview_src = $mediaObj->path.($dir ? $dir.'/' : '').$mediaObj->filename;
      $messages[] = "original preview rank: $mediaObj->rank active: $mediaObj->active ($preview_src):".html_image($preview_src, $mediaObj->filename, array('width' => 100));
    }
  
    print_log("full resizeArray=".print_r($resizeAr, true), 'resize', LOG_LEVEL_TEST);
    
  }
  
  $img_type = mime_type($org_src);
  $more_info = [];
  $img_info_array = getimagesize($org_src, $more_info); // size of original
  $w = $img_info_array[0];
  $h = $img_info_array[1];
  $max = max($h, $w);
  if($mediaObj->type != 'image') return array(false, array("resize_media: $src ($mediaObj->id) is not an image."));

  print_log("OK: h=$h w=$w", 'resize', LOG_LEVEL_TEST);
  
  foreach($resizeAr as $key => $dirAr) {
    list($size, $dir, $resize_options) = $dirAr;
    if($max_size) {
      print_log("fixed resize: max=$max size=$max_size", 'resize', LOG_LEVEL_TEST);
      $overwrite = true;
      $size = $max_size; /** replace in place */
      if($max <= $max_size) {
        print_log("fixed resize, but max $max <= $max_size, skipping resize", 'resize', LOG_LEVEL_TEST);
        $resized = 1; /** this allows update_object below */
        $max_size = $max;
        continue; // do not upscale
      }
    } else {
      $overwrite = $options['overwrite'][$key] || $options['overwrite_all']; 
    }      
    
    if($debug) print_log("traversing resizeArray for key=$key: size=$size dir=$dir resize_options=".print_r($resize_options, true), 'resize', LOG_LEVEL_TEST);
    $dest_dir = $imagedir.($dir ? $dir.'/' : '');
    $dest_src = $dest_dir.$src;
    if(!is_dir($dest_dir)) {
      print_log("making $dest_dir...", 'resize', LOG_LEVEL_TEST);
      if(!mkdir_recursive($dest_dir)) {
        if($debug) print_log("Couldn't make $dest_dir<br>");
        return array(false, array("Couldn't make directory $dest_dir"));
      } else {
        if($debug) print_log("OK, made $dest_dir", 'resize', LOG_LEVEL_TEST);
      }
      $messages[] = "<span class='red'>Couldn't find $dest_dir, created it</span>";
    } else {
      print_log("OK, $dest_dir exists", 'resize', LOG_LEVEL_TEST);
    }

    if($options['preview']) {
      $preview_src = $mediaObj->path.($dir ? $dir.'/' : '').$src;
      $messages[] = "$key preview rank: $mediaObj->rank active: $mediaObj->active ($preview_src):".html_image($preview_src, $src, array('width' => 100));
    }
    
    if(!$max_size && ($key == 'org' || $key == $org_key)) {
      print_log("Skipping $key, don't resize original", 'resize', LOG_LEVEL_TEST);
      continue;  // don't resize the original
    }
    
    if(is_file($dest_src)) {   
      print_log("Destination image $dest_src exists", 'resize', LOG_LEVEL_TEST);
      $img_info_array = getimagesize($dest_src); // size of this imagee
      $iw = $img_info_array[0];
      $ih = $img_info_array[1];
      $imax = max($ih, $iw);

      if($key == 'full' && $max < $size) {
        if($update && $delete) {
          //unlink($dest_src);
          print_log("deleted full image of $dest_src ($w x $h) => $size", 'resize', LOG_LEVEL_TEST);
        } else {
          print_log("<span class='red'>warning: full image of $dest_src ($w x $h) is less than $size</span>", 'resize', LOG_LEVEL_TEST);
        }
      }
      if($overwrite && $imax != $size) {
        print_log("$key: $dest_src ($iw x $ih) already exists, overwriting with (long=$size)", 'resize', LOG_LEVEL_TEST);
      } else {
        if($options['verbose']) print_log("$key: $dest_src ($iw x $ih) already exists, skipping", 'resize', LOG_LEVEL_TEST);
        else if($imax != $size) print_log("$key: $dest_src ($iw x $ih) already exists (wrong size: $imax < $size), skipping", 'resize', LOG_LEVEL_TEST);
        continue; //found it
      }
    } else {
      print_log("Destination image $dest_src does not exists update=$update", 'resize', LOG_LEVEL_TEST);
    }

    
    if($skip_small && $max < $size) {
      print_log("Original too small: skipping $key resize of $org_src ($w x $h) => $size: $dest_src", 'resize', LOG_LEVEL_TEST);
      continue;
    }
    
    // if($key == 'full' && $max < $size) $messages[] = "skipping full resize of $org_src ($w x $h) => $size: $dest_src";
    
    
    if($update) {
      if($debug) print_log("Calling: resize_image($org_src, $dest_src, $size, options=)".print_r($resize_options, true));
      if(smart_resize($org_src, $dest_src, $size, (array) $resize_options)) {
        $messages[] = "<span class='green'>resized $org_src => $dest_src (size=$size)</span>";
        print_log("OK: resized image $src to $dest_src: size=$size in $dir", 'resize', LOG_LEVEL_TEST);
        $resized = true;
        if($optimize) {
          list($optim_success, $optim_error, $optim_message) = optimize_image($img_type, $dest_src);
          if($optim_message) $messages[] = $optim_message;
          // if($optim_error) $errors[] = $optim_error; Let's ignore this error
        }
      } else {
        print_log("Couldn't resize image $src to $dest_src: size=$size in $dir", 'resize', LOG_LEVEL_TEST);
        return array(false, array("Couldn't resize image $src to size $size in $dir"));
      }
    }
  }
  $data = array();

  $data['resize'] = $max_size ?: 1;
  if($max_size || !$mediaObj->width)  $data['width']  = $img_info_array[0];
  if($max_size || !$mediaObj->height) $data['height'] = $img_info_array[1];
  if($max_size || !$mediaObj->depth)  $data['depth']  = $img_info_array['bits'];
  if($max_size || !$mediaObj->channels) $data['channels'] = $img_info_array['channels'];
  if(!$mediaObj->mime) $data['mime'] = $img_type;
  if(!$mediaObj->filesize) $data['filesize'] = filesize($org_src);
  if($optimize && $success)  $data['optimized']  = 1;

  print_log("after resize, data:".dump($data, true), 'resize', LOG_LEVEL_TEST);    
  if($resized && $mid && $data) {
    print_log("after resize, updating media $mid", 'resize', LOG_LEVEL_TEST);    
    list($id, $errors) = update_object('media', $data, $mid);
  } else {
    print_log("after resize, no updating, resized=$resized id=$mid", 'resize', LOG_LEVEL_TEST);
  }

  $result = $data;
  return array($result, $errors, $messages);
}

function optimize_image($mime, $file) {
  $result = 0;
  $error = $message = '';
  switch($mime) {
    case 'image/jpeg':
      $command = "jpegoptim --strip-all $file";
      break;
    case 'image/png':
      $command = "optipng $file";
      break;
    case 'image/gif':
      break;
    default:
      $error = "Extension $ext not supported";
      break;              
  }
  if($command) {
    $cmd_output = array();
    $len = $new_len = filesize($file);
    //$len_h = bytes2human($len);
    $time_start = microtime(true);
    execCmd($command, $cmd_output, $cmd_result, 'www-data', true, $_SESSION['sqlgarbage']);    
    $time_end = microtime(true);
    $new_len = filesize($file);
    $perc = (($len-$new_len)/$len) * 100;
    //$new_len_h = bytes2human($len);
    $time = microtime2human($time_end - $time_start);
    if($cmd_result) {
      $error = "<b> [".$time."] FAILED to optimize $file: ".implode(' - ', $cmd_output)."</b>";
      $result = false;
    } else {
      $result = true;
      $result_str = $cmd_output[0];
      // below only works for jpeg
      //$pattern = "/(\S+) ([0-9]+)x([0-9]+) ([0-9]+)bit ([A-Z]+)[^0-9]+\s*([0-9]+)[^0-9]+([0-9]+) bytes \(([0-9\.]+)%\).*/";
      //if(preg_match($pattern, $result_str, $m)) {
      //  //dump($m);
      //  $fn = $m[1];
      //  $w = $m[2];
      //  $h = $m[3];
      //  $bits = $m[4];
      //  $itype = $m[5];
      //  $ilen = $m[6];
      //  $new_len = $m[6];
      //  $new_len_h = bytes2human($new_len);
      //}

      $message = $result_str;    
      $message .= " [".$time."] Optimized $file (reduction $perc%)";
    }
    
  }
  
  return array($result, $error, $message);
}


// new function (April 2013) for renaming/resizing according to new global resizeArray
/** Tested for rental images in 2017, works  */
function resize_multiple_media($mediaSearch, $options=NULL) {
  global $resizeArray;
  global $__SITE_ID;
  $site_id = $mediaSearch['site_id'] ?: $__SITE_ID;
  if(!$site_id) die("not site ID");
  $mediaSearch['site_id'] = $site_id;
  
  $siteObj = get_object('site', $site_id);
  $site_dir = site_directory($siteObj);
  if(!$site_dir) die("not site dir");

  $mediaObjs = find_objects('media', $mediaSearch);
  $count = count($mediaObjs);
  $output = "Resizing $count media<br>";
  
  foreach($mediaObjs as $mediaObj) {
    $id = $mediaObj->id;
    $site_id = $mediaObj->site_id;
    $parent_id = $mediaObj->parent_id;
    $parent_type = $mediaObj->parent_type;
    $parent_field = $mediaObj->parent_field;
    $path = $mediaObj->path;
    $src = $mediaObj->src;
    $filename = $mediaObj->filename;
    $type = $mediaObj->type;
    
    if(!$siteObj) $siteObj = get_object('site', $site_id);
    
    $parentObj = get_object($parent_type, $parent_id);
    if(!$parentObj) {
      $output .= html_b("$parent_type $parent_id does not exist<br>");
      //delete_object('media', $id);
      continue;
    }          
    if($parent_path = $parentObj->path) {
      $correct_path = clean_path($parent_path."$parent_field/$type/");
      if($path != $correct_path) {
        echo(html_b("-$path<br>=$correct_path<br>"));
      }
    }
    
    $output .= "$parent_type $parent_id: type=$type path=$path src=$src filename=$filename<br>";
    
    
    $options['path'] = $path; // array where key->value is directory->size
    $dir = $options['directory'] = clean_path($site_dir.$path);
    
    if(!$dir) die("path={$path} dir={$dir}");
    
    $org_src = clean_path($dir.'org/'.$mediaObj->filename);
    if(is_file($org_src)) {
      $output .= "$org_src exists!<br>";
      $options['resize'] = $options['resize'] ?: $resizeArray;
      $options['overwrite_all'] = true;
      //dump($resize_options);die();
      list($result, $errors, $messages) = resize_media($siteObj, $mediaObj, $options);
    } else {
      $output .= html_b("$org_src not found! parent=$parent_type $parent_id<br>");
    }
    
    if($errors) {dump($errors);die();}         
    dump($messages); 
  }
  return $output;
}


// helpers to fix / clean / update media table and filesystem
// March 2012
// resizes missing images according to resize settings 
/** unsused - automatic comment added on  */
function resize_property_images($siteObj, $aptObj, $options=NULL) {
  $pid = $aptObj->id;
  $apt_name = $aptObj->name;
  $gallery_options = property_gallery_options($siteObj, $aptObj);
  //dump($gallery_options);die();
  $output = array();  
  $search = $options['search'] ?: $gallery_options['search'];
  $options['path']      = $options['path'] ?: $gallery_options['path'];
  $options['directory'] = $options['directory'] ?: $gallery_options['directory'];
  $options['resize']    = $options['resize'] ?: $gallery_options['resize'];
  //dump($options);die();
  if($mediaObjs = find_objects('media', $search, array('sort' => 'rank'))) {    
    $mcount = count($mediaObjs);
    $output[]= "<h2>$apt_name ($pid) has $mcount pictures</h2>";
    foreach($mediaObjs as $mediaObj) {
      list($result, $errors, $messages) = resize_media($siteObj, $mediaObj, $options);
      if(count($messages)) $output = array_merge($output, $messages);      
      if(!$result && count($errors)) $output = array_merge($output, $errors);      
      //if(!$result || $errors) $output .= html_span(implode(html_break(), $errors).html_break(), 'red');
    }
  } else {
    $output[]= "<b>$apt_name ($pid) has no pictures</b>";
  }
  return $output;
}

// function property_media_name($siteObj, $aptObj, $mediaObj)

function property_media_name($aptObj, $filename, $rank) {
  $ts = time();
  if(!$aptObj->shortname || !$filename) return '';
  if($rank < 0) $rank = "inactive".$rank;
  list($first, $suffix) = split_filename($filename);
  return implode('-', array($aptObj->shortname, $rank, $ts)).".".strtolower($suffix);
}


// renames a media object according to new naming convention
function rename_media($siteObj, $aptObj, $mediaObj, $rank, $options=NULL) {    
  $resizeAr = (array) $options['resize']; // array where key->value is directory->size
  if(!$resizeAr) return;
  $update = isset($options['update']) ? $options['update'] : true;
  
  $imagedir  = trim($options['directory']) ?: media_directory($siteObj, $mediaObj, $options); // file system full path - must be set 
  if(!$imagedir) return array(false, array('rename_media: No dir found.'));

  $src = trim($mediaObj->src);
  if(!$src) return array(false, array('rename_media: No src for media.'));

  // list($first, $suffix) = split_filename($src);

  foreach($resizeAr as $key => $dirAr) {
    if($key == 'org') continue; // don't rename the original
    list($size, $dir, $resize_options) = $dirAr;
    $dest_dir = $imagedir.($dir ? $dir.'/' : '');
    
    $new_filename = property_media_name($aptObj, $src, $rank); 

    $from = $dest_dir.$src;
    $to   = $dest_dir.$new_filename;      
    if(!is_file($from)) {
      $messages[] = "<span class='red'>rename_media: couldn't find source file $from, skipping</span>";
      continue;
      //return array(false, array("rename_media: couldn't find source file $from")); 
    }
    if($from == $to) {
      $messages[] = "$aptObj->shortname: $from already has the correct name";
    } else {
      if($update) {
        $result = rename($from, $to);
        if(!$result) return array(false, array("rename_media: Rename $from => $to failed."));
      }
      $messages[] = "Moved $from => $to";
    }
  }
    
  return array($new_filename, $errors, $messages);
}

// organizes all property images by rank, renames to new convention (type_city_aptname)
function rename_property_images($siteObj, $aptObj, $options=NULL) {
  $pid = $aptObj->id;
  $apt_name = $aptObj->name;
  $update = isset($options['update']) ? $options['update'] : true;
  $gallery_options = property_gallery_options($siteObj, $aptObj);
  
  $search = $options['search'] ?: $gallery_options['search'];
  if(!isset($search['active'])) $search['active'] = 1; // force active media unless set
  $options['path']      = $options['path'] ?: $gallery_options['path'];
  $options['directory'] = $options['directory'] ?: $gallery_options['directory'];
  $options['resize']    = $options['resize'] ?: $gallery_options['resize'];
  
  if($mediaObjs = find_objects('media', $search, array('sort' => 'rank'))) {    
    $mcount = count($mediaObjs);
    $output .= "<b>$apt_name ($pid) has $mcount pictures</b><br>";
    $i = 1;
    foreach($mediaObjs as $mediaObj) {
      $id = $mediaObj->id;
      $rank = $mediaObj->active ? $i : $i * -1; // negative rank for in active images
      list($new_filename, $errors, $messages) = rename_media($siteObj, $aptObj, $mediaObj, $rank, $options);
      if($messages) $output .= implode(html_break(), $messages).html_break();      
      if(!$new_filename || $errors) $output .= html_span(implode(html_break(), $errors).html_break(), 'red');
      //echo("new_name for $aptObj->name $mediaObj->src = $new_filename");die();
      if($update) db_query("UPDATE media SET src='$new_filename', rank=$rank WHERE id=$id");
      //else $output .= "$apt_name $rank = $new_filename<br>";
      $i++;
    }
  } else {
    $output .= "<span class='red bold'>$apt_name ($pid) has no pictures</span><br>";
  }
  
  if($options['search']['active']) {
    // also process the inactive images
    $options['search'] = $search;
    $options['search']['active'] = 0;      
    $output .= rename_property_images($siteObj, $aptObj, $options);
  }
  return $output;

}

// March 2012
// renames images for given properties to new system in database, and moves files correspondingly
/** unsused - automatic comment added on  */
function old_rename_property_images($aptObjs, $update=false, $limit=0) {
  
  $count = count($aptObjs);
  //dump($aptObjs);die();
  $row = array();
  $row[] = 'id';
  $row[] = 'type';
  $row[] = 'parent_type';
  $row[] = 'parent_id';
  $row[] = 'apt';
  $row[] = 'active';
  $row[] = 'exists?';
  $row[] = 'rank';
  $row[] = 'new type';
  $row[] = 'old name';
  $row[] = 'new name';
  $row[] = 'new filename';
  $row[] = 'path';
  $row[] = 'db path';
  $rows[] = $row;
  
  foreach($aptObjs as $apt) {
    if($limit && $ic >= $limit) {
      $output .= "i=$ic, limit=$limit, exiting<br>";
      //dump($rows);
      break;
    }
    $pid = $apt->id;
    $apt_name = $apt->name;
    if($boat_type = $apt->boat_type) {
      $type_name = get_object('boat_type', $boat_type, 'name');
    } else {
      $type_name = $pt;
    }
    $type_name_str = get_dictionary_entry($type_name, $lang, LOWERCASE) ?: $type_name;
  
    if($mediaObjs = find_objects('media', array('parent_id' => $pid), array('sort' => 'rank'))) {
      $pcount++;
    } else {
      $output .= "<b>$apt_name has no pictures</b><br>";
      continue;
    }
    
    $lang = 'nl';
    $rank = 1;
    foreach($mediaObjs as $mediaObj) {
      $id=$mediaObj->id;
      $new_filename = implode('-', array($apt->shortname, $rank));
      $new_name = implode('-', array($type_name_str,$city_name,$apt->shortname, $rank));
      $img_type = mime_type($mediaObj->src);     
      $src = $mediaObj->src;
      $path = get_media_tail($mediaObj);
      $full_file = $base.$path.$src;
      $exists = is_file($full_file);
      if($src == $new_filename) {
        $output .= "$apt->name is already processed<br>";
        continue;
      }
  
      if(!$exists) {
        $output .= "<b>$full_file for $apt->name doesn't exist</b><br>";
        continue;
      }
  
      $row = array();
      $row[] = $mediaObj->id;
      $row[] = $mediaObj->type;
      $row[] = $mediaObj->parent_type;
      $row[] = $mediaObj->parent_id;
      $row[] = $apt->shortname;
      $row[] = $mediaObj->active;
      $row[] = $exists;
      $row[] = $mediaObj->rank;
      $row[] = $img_type;
      $row[] = $src;
      $row[] = $new_name;
      $row[] = $new_filename;
      $row[] = $path;
      $row[] = $mediaObj->path;
      
      list($first, $suffix) = split_filename($mediaObj->src);
      $rows[] = $row;
      //if(++$i > 100) break; 
      if($update) {
        // move
        $dirs = array('', 'small/', 'full/', 'thumbs/');
        foreach($dirs as $dir) {
          $full_path = $base.$path.$dir;
          if(!is_dir($full_path) && !mkdir_recursive($full_path)) {
            $output .= "Couldn't make path $full_path<br>";
            continue;
          }
  
          $from = $full_path.$src;
          $to   = $full_path.$new_filename;      
          $result = rename($from, $to);
          $output .= $result ? "Moved $from -> $to<br>" : "<b>Failed to move $from -> $to</b><br>";
        }
        
        db_query("UPDATE media SET src='$new_filename', rank=$rank, type='$img_type' WHERE id=$id");
      }
      
      $rank++;
    }
    db_query("UPDATE property SET processed=1 WHERE id=$pid");
    $ic++;
  }
  $output = "<h2>Found $count $ptype - $pcount has photos</h2>". $output;
  $output .= html_table($rows);
  return $output;
}

/*
'retina-sprite retina32', array('top' => 28, 'left' => 114, 'rows' => 16, 'cols' => 3, 'height'=> 32, 'width' => 32)
'retina-sprite retina48', array('top' => 28, 'left' => 114, 'rows' => 16, 'cols' => 3, 'height'=> 48, 'width' => 48)
'retina-sprite retina64', array('top' => 12, 'left' => 35, 'rows' => 16, 'cols' => 3, 'height'=> 64, 'width' => 64)
*/
                         
/** unsused - automatic comment added on  */
function sprite_print($sprite, $rows, $cols, $options=NULL) {

  //$rows = $options['rows'] ?: 13; // rows in sprite
  //$cols = $options['cols'] ?: 14; // columns in sprite
                    
  $yo = $options['width'] ?: 49; // grid width
  $xo = $options['height'] ?: 49; // grid height
  
  $yi = $options['top'] ?: 4; // y offset
  $xi= $options['left'] ?: 10; // x offset

  $output .= html_div('', 'clear_float').html_break(2).html_h1("$sprite Size: $xo x $yo Offset: $xi x $yi").html_break(2);
  $count =1;                                                      
  if($bg_color = $options['bg_color'])   $bg = "background-color: #$bg_color; ";  
  for($i=0;$i < $rows; $i++) {
    for($j=0;$j < $cols; $j++) {
      $y=$yi + $i*$yo;
      $x=$xi + $j*$xo;
      //echo(($xi + $i*$x).html_break());
      $style = $bg."background-position:-".$x."px -".$y."px";
      $output .= html_div($count, "$sprite s".$i, "style='$style'");
      $css_output .= ".$sprite.s".$count.' {'.$style.'}'.html_break();
      $count++;
    }
  }
  
  //$output .= html_h2("CSS").html_div($css_output, 'clear_float');
  $output .= html_h2("CSS").html_div($css_output, 'clear_float');
  return $output;
}

function media_options($options=array()) {
  $mediaOptions = array(
    'minimum_size' => 100, 
    'minimum_file_size' => '1K', 
    'maximum_file_size' => '20M', 
    'allow_multiple' => true, 
    'allowed_types' => array('image','audio','video'), 
    'resize' => 1);
  if($options) $mediaOptions = array_merge($mediaOptions, $options);
  return $mediaOptions;
}


// set media field of shortstay
/** unsused - automatic comment added on  */
function update_property_media($ptype = 'shortstay') {  
  $mediaObjs = find_objects('media', array('parent_type' => $ptype), array('sort' => 'parent_id'));
  foreach($mediaObjs as $mediaObj) {
    $pid = $mediaObj->parent_id;
    if($done[$pid]) continue;
    $output .= "Processing $pid<br>";
    update_parent_media_list($mediaObj);
    $done[$pid] = true;
  }
  return $output;
}

/** new 2017 */  
function fix_resize_images() {  
  global $resizeArray;
  //$output .= dump($resizeArray, true);
  /** create missing resized images */
  $limit = 1500;
  $rows = [];
  $rows[] = ['ID', 'Created', 'Title', 'Path', 'Src', 'Org', 'Tiny', 'Thumb', 'Square', 'Small', 'Medium', 'IG', 'Big', 'Full', 'Dim', 'Biggest', 'Notes'];

  $search = ['site_id' => $__SITE_ID, 'resize' => 1, 'resized' => 1, 'active' => 1];
  $mediaObjs = find_objects('media', $search, ['sort' => 'id desc', 'limit' => $limit]);
  $base = str_replace('data-dev', 'data', rtrim($_SESSION['data_dir'], '/'));

  $count = count($mediaObjs);
  $missing = 0;
  foreach($mediaObjs as $mediaObj) {
    $path = $mediaObj->path;
    $src= $mediaObj->src;
    
    $org_src = "{$base}/{$path}/org/{$mediaObj->filename}";
    $org = file_exists($org_src); $org_str = $org ? html_span('Y', 'green') : html_span('N', 'red');
    if(!$org) {
      echo("Missing: {$mediaObj->name} created {$mediaObj->created} org={$org_src}<br>");
      continue;
    }
    
    $tiny = file_exists("{$base}/{$path}/tiny/{$src}"); $tiny_str = $tiny ? html_span('Y', 'green') : html_span('N', 'red');
    $thumb = file_exists("{$base}/{$path}/thumbs/{$src}"); $thumb_str = $thumb ? html_span('Y', 'green') : html_span('N', 'red');
    $square = file_exists("{$base}/{$path}/square/{$src}"); $square_str = $square ? html_span('Y', 'green') : html_span('N', 'red');
    $small = file_exists("{$base}/{$path}/small/{$src}"); $small_str = $small ? html_span('Y', 'green') : html_span('N', 'red');
    $medium = file_exists("{$base}/{$path}/medium/{$src}"); $medium_str = $medium ? html_span('Y', 'green') : html_span('N', 'red');
    $ig = file_exists("{$base}/{$path}/ig/{$src}"); $ig_str = $ig ? html_span('Y', 'green') : html_span('N', 'red');
    $big = file_exists("{$base}/{$path}/big/{$src}"); $big_str = $big ? html_span('Y', 'green') : html_span('N', 'red');
    $full = file_exists("{$base}/{$path}/full/{$src}"); $full_str = $full ? html_span('Y', 'green') : html_span('N', 'red');
    
    //if($small && $medium && $big && $full) continue;
    //if($small && $medium && $ig && $big && $full) continue;
    $ar = ['tiny' => $tiny *1, 'thumb' => $thumb *2, 'square' => $square * 3, 
           'small' => $small * 4, 'medium' => $big * 5, 'ig' => $ig * 6, 'big' => $big * 7, 'full' => $full * 8];
    $ar2 = array_filter($ar);
    if(count($ar2) == 8) continue;
    $missing++;
    
    end($ar2);
    $biggest = key($ar2); 
    $notes = '';
    foreach($ar as $k => $v) {
      if(!$v) {
        
        list($size, $dir, $resize_options) = $resizeArray[$k];
        //echo("k=$k size=$size dir=$dir options=".dump($resize_options,true));
        //resize_image($file, $dst, $long, $options);
        $file = $org_src;
        $dst = "{$base}/{$path}/{$dir}/{$src}";
        $long = '';
        $dst_dir = "{$base}/{$path}/{$dir}";
        
        if(!is_dir($dst_dir) && !mkdir($dst_dir, 0777, true)) die("Couldn't make $dst_dir");
        
        $result = resize_image($file, $dst, $size, $resize_options);
        $output .= dump($result, true);
        
        $img = str_replace('/web/data/vpatina/', '', $dst);
        //echo("Img: {$img} <img src='{$img}'><br>");
        //die("resize {$dst}: result=$resize");
        
        $notes .= "{$k}, ";
        //symlink("{$base}/{$path}{$biggest}/{$src}", "{$base}/{$path}{$k}/{$src}");
      }
    }
    $rows[] = [$mediaObj->id, $mediaObj->created, $mediaObj->name, $mediaObj->path, $mediaObj->src, $org_str, $tiny_str, $thumb_str, $square_str, $small_str, $medium_str, $ig_str, $big_str, $full_str, "{$mediaObj->height}/{$mediaObj->width}", $biggest, $notes];
  }
  $output .= html_h2("Found $count Media. $missing needs fixing");
  
  $output .= html_table($rows, ['class' => 'web20']);
  return $output;
}

function fix_media_json($obj_type, $field) {
  $json_field = "{$field}_json";
  $objs = find_objects($obj_type, [$field => ['<>', ['', 0]]]);
  $count = count($objs);
  $output .= html_h2("Found {$count} {$obj_type} with {$field} and no json");
  $rows = [];
  $rows[] = ['id', $field, $json_field];
  foreach($objs as $obj) {
    if($mediaJson = media2json($obj_type, $obj, ['parent_field' => $field])) {
      list($id, $errors) = update_object($obj_type, [$json_field => $mediaJson], $obj->id, ['debug_query' => 0]);
      if($errors) {
        dump($errors);
        die();
      }
      $rows[] = [$id, $obj->$field, $mediaJson];
    }
  }
  $output .= html_table($rows, ['class' => 'web20']);
  return $output;
}
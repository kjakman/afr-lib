<?php

// return link to front end for exhibition,media,artwork,map
function gallery_link($obj_type, $obj_id, $options=[]) {
  global $__APP;
  $types = ['artwork', 'media_collection', 'media_collection_map', 'media', 'user_profile'];
  if(!$obj_id || !in_array($obj_type, $types)) return '';
  $obj = get_object($obj_type, $obj_id);
  switch($obj_type) {
    case 'artwork';
      return reverse_route($__APP, 'artwork', ['id' => $obj_id, 'name' => slugify($obj->title)]);
      break;
    case 'media_collection';
      return reverse_route($__APP, 'exhibition:id', ['id' => $obj_id, 'name' => slugify($obj->title)]);
      break;
    case 'media_collection_map';
      return reverse_route($__APP, 'item:id', ['id' => $obj_id, 'name' => slugify($obj->name)]);
      break;
    case 'user_profile';
      return reverse_route($__APP, 'profile:id', ['id' => $obj->user_id, 'name' => slugify($obj->display_name)]);
      break;
    case 'media';
      return reverse_route($__APP, 'media:id', ['id' => $obj_id, 'name' => slugify($obj->name)]);
      break;
    default:
      return '';
  }
}

/** geocode galleries 
    todo: make automatic 
*/
function geocode_galleries() {  
  $userObjs = find_objects('user', ["user.site_id" => 247, "user_level" => 26]);
  foreach($userObjs as $userObj) {
    $user = get_user($userObj);
    $data = [];
    $country_name = get_country_name($user->country);
    if($user->address1 && $user->city && $user->country) {
      $address = implode(",", array_filter([$user->address1, $user->address2, trim("{$user->zip} {$user->city}"), $user->state, $country_name]));
      $data['address_formatted'] = $address;
      
      list($status, $acc, $lat, $lng, $bounds) = google_geocode($address);
      
      if($status==200) {
        $scope->http_code = "200"; //override error set in set_env.inc due to unknown slug
        $center = [$lat, $lng];
        $locObj = center2loc($center);
        $boundsObj = $bounds ? json_decode($bounds) : radius2bounds($locObj, DEFAULT_RADIUS);
        $bounds = any2bounds($boundsObj, true);
        
        $data['location_lat'] = $lat;
        $data['location_long'] = $lng;
        $data['location_bounds'] = json_encode($bounds);
        
        list($id, $errors) = update_object('user_profile', $data, $user->profile_id);
        echo("Updated $id with ".dump($data,true));
      }
    }
  }
}

/** return list of artist names for an exhibition */
function collection_artist_list($collectionObj_or_id, $options=[]) {
  $collectionObj = any2obj('media_collection', $collectionObj_or_id);
  //$artist_names = find_objects('media_collection_map', ['media_collection_id' => $collection_id, 'active' => 1], ['fields' => 'distinct artist_name']);
  $artist_id = $collectionObj->artist_id;
  $artist_ids_json = $collectionObj->artist_ids;
  $artist_ids = json_decode($artist_ids_json);

  if(!is_array($artist_ids) || !count($artist_ids)) $artist_ids = [$artist_id];
  
  
  if(!is_array($artist_ids) || !count($artist_ids)) return '';

  print_log("id: {$collectionObj->id}  ids:".dump($artist_ids, true), 'backend', LOG_LEVEL_TEST);

  if($options['link']) { /** not yet implemented */
    foreach($artist_ids as $artist_id) {
    }
  } else {
    $artist_names = find_objects('user_profile', ['user_id' => $artist_ids], ['fields' => 'distinct display_name']); 
    return implode(', ', $artist_names);
  }
}

/** not yet implemented */
function collection_artist_links($collectionObj_or_id) {
  return collection_artist_list($collectionObj_or_id, ['link' => true]);
}

/** 
  Data for plugin 
*/

function gallery_get_json($collection_id, $site_id=247) {
  global $__SITE, $u_id;  
  $siteObj = $site_id == $__SITE->id ? $__SITE : get_object('site', $site_id);

  $obj_type = 'media_collection';
  $obj_id = $collection_id;
      
  $output = '';
  $debug = '';   
  
  $strip = false;
  
  $events = find_objects('event', ['parent_type' => $obj_type, 'parent_id' => $obj_id, 'end_time' => ['>', now()]]);
  $jsonObj->events = $events;
  
  
  $collectionObj = get_object('media_collection', $collection_id);
  $jsonObj->curator_id = $collectionObj->user_id;
  $jsonObj->collection_name = $collectionObj->title;

  $collectionObj->description = safe_text($collectionObj->description);
  
  $jsonObj->collection = $collectionObj;
  $jsonObj->collection_id = $collectionObj->id;
  $jsonObj->collection_image = $collectionObj->image ? print_media($collectionObj->image, ['full' => 1, 'size' => 'big']) : '';

  $address_fields = ['address1' , 'address2' , 'city' , 'state' , 'zip' , 'country' , 'email_address', 'phone', 'mobile', 
                     'location_lat','location_long', 'location_bounds', 'address_formatted'];
  $user_fields = array_merge(['user_id', 'profile_id', 'display_name', 'description'], $address_fields);            

  
  $curator_id = $collectionObj->user_id;
  $curatorObj = $curator_id ? get_user($curator_id) : null;

  
  if($curatorObj) { // used for chat
    $jsonObj->curator_name = $curator_name = $curatorObj->display_name;
    $jsonObj->curator_email = $curator_email = $curatorObj->email_address;
    $jsonObj->curator_greeting = "Hello, I'm {$curator_name}!";
    $jsonObj->curator_image = $curator_image = user_photo($curatorObj, 'big', ['full' => 1]);

    $curator = [];
    foreach($user_fields as $f) $curator[$f] = $curatorObj->$f;
    $curator['image'] = $curator_image;
    
    $curator['description'] = safe_text($curator['description']);
    
    $jsonObj->curator = $curator;
  }
  
  if($u_id) { // used for chat
    $userObj = get_user($u_id);
    foreach($user_fields as $f) $user[$f] = $userObj->$f;    
  }
  
  
  $jsonObj->base_link = page_link(PAGE_GALLERY, $obj_type, $collectionObj->type);
  
  print_log("cid: $curator_id name: $curator_name email: $curator_email".dump($curatorObj,true), 'gallery', LOG_LEVEL_ALWAYS);

  $media_ids = [];
  $mapObjs = find_objects('media_collection_map', array('media_collection_id' => $collection_id, 'active' => 1), array('fields' => ['id', 'media_id', 'type', 'description', 'artwork_id', 'media_id', 'artist_id'], 'sort' => 'rank'));;
  if($mapObjs){
    foreach($mapObjs as $mapObj) {
     if($mapObj->media_id != '0' && $mapObj->artist_id != '0'){
      $mapArray[$mapObj->media_id] = $mapObj->id; 
      $mapArtworkArray[$mapObj->media_id] = $mapObj->artwork_id; 
      $get_media = get_object('media', $mapObj->media_id);
      //dump($mapObj);die;
      if(!$mapObj->artwork_id){
        $artwork_link = "{$vpatinaSiteUrl}/item/{$mapObj->media_id}/";
        $title = $collectionObj->title; 
        $type_val = related_images_type($mapObj->type);
        if($type_val)
          $type = ' ('.$type_val.') ';
          $name = "$title $type $mapObj->description";
      }else{
        $artwork_link = "${vpatinaSiteUrl}/artwork/{$mapObj->artwork_id}/";

        if($get_media->original_year)
          $year = '('.$get_media->original_year.'),';
          $artist = user_display_name($get_media->artist_id);
          if($artist)
            $name = "$get_media->name $year $get_media->original_technique, by $artist";
          else
            $name = "$get_media->name $year $get_media->original_technique";
      }
      $get_media->comment = $name;
      list($id, $errors) = update_object('media', $get_media, $get_media->id);
      
      if($errors) json_error($errors);
     }
    }
    //$media_ids = find_objects('media_collection_map', array('media_collection_id' => $collection_id, 'active' => 1), array('fields' => 'distinct media_id', 'sort' => 'rank'));
    $media_ids = array_keys($mapArray);
    
    
    $obj_id_list = implode(',', $media_ids);
    $base_url = $_SERVER['HTTP_HOST'];  
    $protocol = "https";
    $fields = array(
      'id as media_id', 
      'src', 
      'path', 
      'comment as title', 
      'comment',
      "concat ('$protocol', '://', '$base_url', path, 'full/', src) as href",
      "concat ('$protocol', '://', '$base_url', path, 'small/', src) as href_small",
      'original_currency' ,
      'original_price'    ,
      'original_height'   ,
      'original_width'    ,
      'original_url'      ,
      'original_count'    ,
      'original_available',
      'original_year'     ,
      'original_technique',
      'digital_price'     ,
      'digital_height'    ,
      'digital_width'     ,
      'height as h'    ,
      'width as w'     ,
      'artist_id');
    
    $items = find_objects('media', array('id' => $media_ids), array('sort' => "FIELD(id, $obj_id_list)", 'fields' => $fields));
    foreach($items as &$item) {
      $item->map_id = $map_id = $mapArray[$item->media_id];
      $item->artwork_id = $artwork_id = $mapArtworkArray[$item->media_id];

      $slug = slugify($item->title);
      //$item->link = "{$artwork_link}{$slug}~{$map_id}";
      $item->link =  $artwork_id  ? "https://www.vpatina.com/artwork/{$obj_id}/{$slug}-{$artwork_id}?plugin" : "{$artwork_link}{$slug}~{$map_id}";

      $item->comment = safe_text($item->comment);
    }
  }
  
  //dump($media_ids);
  $artist_ids = find_objects('media', array('id' => $media_ids), array('fields' => "distinct (artist_id)"));
  
  $artist_names = $artists = [];
  foreach($artist_ids as $artist_id) {
    $artistObj = get_user($artist_id);
    $artist_names[$artist_id] = $artist_name = $artistObj->display_name;
    $artist_image = $artistObj->photo ? user_photo($artistObj, 'big', ['full' => 1]) : '';
    
    $artist = [];
    foreach($user_fields as $f) $artist[$f] = $artistObj->$f;
    $artist['image'] = $artist_image;
    $artist['description'] = safe_text($artist['description']);
    
    $artists[$artist_id] = $artist;
  }
  
  $jsonObj->artists = $artists;
  $jsonObj->artist_names = $artist_names;
  $jsonObj->target = $target;
  

  $shareObj = $share_id ? get_object('sharing', $share_id) : 0;
  $pluginOptions = $shareObj ? ['infobar_show' => $shareObj->infobar_show, 'infobar_control' => $shareObj->infobar_control, 'border_width' => $shareObj->border_width, 'border_radius' => $shareObj->border_radius, 'border_color' => $shareObj->border_color] : [];    
  
  $jsonObj->obj_type = $obj_type;
  $jsonObj->obj_id = $obj_id;
  $jsonObj->site_id = $site_id;
  if($jsonObj->user_id = $u_id) {
    $jsonObj->user = $user;
  }
  $jsonObj->user_name = $u_id ? user_display_name($u_id) : '';
        
  $jsonObj->options = $pluginOptions;
  
  $jsonObj->items = $items;
  
  if($collection_id) {
    $jsonObj->collection_id = $collection_id;
    $jsonObj->user_collection_id = 0;
    $jsonObj->follow = [];      
    $user_collection_id = 0;
    if($u_id) {
      $user_collection_id = gallery_find_user_collection($siteObj, $u_id, array('return' => 'id'));
      if($user_collection_id) { 
        $jsonObj->user_collection_id = $user_collection_id;
        $jsonObj->follow = $follow_ids = find_objects('media_collection_map', array('media_collection_id' => $user_collection_id, 'active' => 1), array('fields' => 'distinct (media_id)'));
      }
    }
  }
    
  return $jsonObj;
}

function gallery_getrelated_json($collection_id, $site_id=247) {
  global $__SITE, $u_id;  
  $siteObj = $site_id == $__SITE->id ? $__SITE : get_object('site', $site_id);

  $get_artids = find_objects('media_collection_map', ['media_collection_id'=> $collection_id],array('fields' => ['artwork_id']) );

  $artids = [];
  foreach($get_artids as $get_artid){
    $artids[] = $get_artid->artwork_id;
  }
  $json_array = [];
  foreach($artids as $obj_id){
    $obj_type = 'artwork_media';
    //$obj_id = $artwork_id;
        
    $output = '';
    $debug = '';   
    
    $strip = false;
    $artworkObj = get_object('artwork', $obj_id);
    //dump($artworkObj);
    $jsonObj->curator_id = $artworkObj->user_id;
    $jsonObj->collection_name = $artworkObj->title;

    $artworkObj->description = safe_text($artworkObj->description);
    
    $jsonObj->collection = $artworkObj;
    $jsonObj->collection_id = $artworkObj->id;
    $jsonObj->collection_image = $artworkObj->title_image ? print_media($artworkObj->title_image, ['full' => 1, 'size' => 'big']) : '';

    $address_fields = ['address1' , 'address2' , 'city' , 'state' , 'zip' , 'country' , 'email_address', 'phone', 'mobile', 'location_lat','location_long', 'location_bounds', 'address_formatted'];
    $user_fields = array_merge(['user_id', 'profile_id', 'display_name', 'description'], $address_fields);            

    
    $curator_id = $artworkObj->user_id;
    $curatorObj = $curator_id ? get_user($curator_id) : null;
    //dump($curatorObj);
    
    if($curatorObj) { // used for chat
      $jsonObj->curator_name = $curator_name = $curatorObj->display_name;
      $jsonObj->curator_email = $curator_email = $curatorObj->email_address;
      $jsonObj->curator_greeting = "Hello, I'm {$curator_name}!";
      $jsonObj->curator_image = $curator_image = user_photo($curatorObj, 'big', ['full' => 1]);

      $curator = [];
      foreach($user_fields as $f) $curator[$f] = $curatorObj->$f;
      $curator['image'] = $curator_image;
      
      $curator['description'] = safe_text($curator['description']);
      
      $jsonObj->curator = $curator;
    }
    
    if($u_id) { // used for chat
      $userObj = get_user($u_id);
      foreach($user_fields as $f) $user[$f] = $userObj->$f;    
    }
    
    $jsonObj->base_link = page_link(PAGE_GALLERY, $obj_type, $collectionObj->type);
    
    print_log("cid: $curator_id name: $curator_name email: $curator_email".dump($curatorObj,true), 'gallery', LOG_LEVEL_ALWAYS);

    $media_ids = [];
    $mapObjs = find_objects('artwork_media', array('artwork_id' => $artwork_id), array('fields' => ['id', 'media', 'artwork_id', 'description', 'type'], 'sort' => 'row_order'));
    if($mapObjs){
      foreach($mapObjs as $mapObj) {
        $mapArray[$mapObj->media] = $mapObj->id; 
        $get_media = get_object('media', $mapObj->media);

        $title = $artworkObj->title;
        $type_val = related_images_type($mapObj->type);
        $artist_name = user_display_name($artworkObj->artist_id);

        $name = $title." by ".$artist_name." (".$type_val."), ".$mapObj->description;
        $get_media->name = $name;
        list($id, $errors) = update_object('media', $get_media, $get_media->id);
        if($errors) json_error($errors);
      }
      $media_ids = array_keys($mapArray);

      if($artworkObj->title_image)
        array_unshift($media_ids, $artworkObj->title_image);

      $artwork_link = site_full_url($siteObj)."/item/";
      $obj_id_list = implode(',', $media_ids);
      $base_url = $_SERVER['HTTP_HOST'];  
      $protocol = "https";
      $fields = array(
        'id as media', 
        'src', 
        'path', 
        'name as title', 
        'comment',
        "concat ('$protocol', '://', '$base_url', path, 'full/', src) as href",
        "concat ('$protocol', '://', '$base_url', path, 'small/', src) as href_small",
        'original_currency' ,
        'original_price'    ,
        'original_height'   ,
        'original_width'    ,
        'original_url'      ,
        'original_count'    ,
        'original_available',
        'original_year'     ,
        'original_technique',
        'digital_price'     ,
        'digital_height'    ,
        'digital_width'     ,
        'height as h'    ,
        'width as w'     ,
        'artist_id');
      
      $items = find_objects('media', array('id' => $media_ids), array('sort' => "FIELD(id, $obj_id_list)", 'fields' => $fields));

      foreach($items as &$item) {
        $item->map_id = $map_id = $mapArray[$item->media];
        $slug = slugify($item->title);
        $item->link = "{$artwork_link}{$slug}~{$map_id}";
        $item->comment = safe_text($item->comment);
      }

      $artist_ids = find_objects('media', array('id' => $media_ids), array('fields' => "distinct (artist_id)"));
      $artist_names = $artists = [];
      foreach($artist_ids as $artist_id) {
        $artistObj = get_user($artist_id);
        $artist_names[$artist_id] = $artist_name = $artistObj->display_name;
        $artist_image = $artistObj->photo ? user_photo($artistObj, 'big', ['full' => 1]) : '';
        
        $artist = [];
        foreach($user_fields as $f) $artist[$f] = $artistObj->$f;
        $artist['image'] = $artist_image;
        $artist['description'] = safe_text($artist['description']);
        
        $artists[$artist_id] = $artist;
      }
      
      $jsonObj->artists = $artists;
      $jsonObj->artist_names = $artist_names;
      $jsonObj->target = $target;   
      
      $jsonObj->obj_type = $obj_type;
      $jsonObj->obj_id = $obj_id;
      $jsonObj->site_id = $site_id;
      if($jsonObj->user_id = $u_id) {
        $jsonObj->user = $user;
      }
      $jsonObj->user_name = $u_id ? user_display_name($u_id) : '';
            
      $jsonObj->options = $pluginOptions;
      
      $jsonObj->items = $items;
      
      if($artwork_id) {
        $jsonObj->collection_id = $artwork_id;
        $jsonObj->user_collection_id = 0;
        $jsonObj->follow = [];      
        $user_collection_id = 0;
        if($u_id) {
          $user_collection_id = gallery_find_user_collection($siteObj, $u_id, array('return' => 'id'));
          if($user_collection_id) { 
            $jsonObj->user_collection_id = $user_collection_id;
            $jsonObj->follow = $follow_ids = find_objects('artwork_media', array('artwork_id' => $user_collection_id), array('fields' => 'distinct (media)'));
          }
        }
      }
    }
    
    $json_array[] = $jsonObj;
  }
  //dump($json_array);
  //exit;
  return $json_array;
}

function gallery_getartwork_json($artwork_id, $site_id=247) {
  global $__SITE, $u_id;  
  $siteObj = $site_id == $__SITE->id ? $__SITE : get_object('site', $site_id);

  $obj_type = 'artwork_media';
  $obj_id = $artwork_id;
      
  $output = '';
  $debug = '';   
  
  $strip = false;
  
  $artworkObj = get_object('artwork', $obj_id);

  $jsonObj->curator_id = $artworkObj->user_id;
  $jsonObj->collection_name = $artworkObj->title;

  $artworkObj->description = safe_text($artworkObj->description);
  
  $jsonObj->collection = $artworkObj;
  $jsonObj->collection_id = $artworkObj->id;
  $jsonObj->collection_image = $artworkObj->title_image ? print_media($artworkObj->title_image, ['full' => 1, 'size' => 'big']) : '';

  $address_fields = ['address1' , 'address2' , 'city' , 'state' , 'zip' , 'country' , 'email_address', 'phone', 'mobile', 'location_lat','location_long', 'location_bounds', 'address_formatted'];
  $user_fields = array_merge(['user_id', 'profile_id', 'display_name', 'description'], $address_fields);            

  
  $curator_id = $artworkObj->user_id;
  $curatorObj = $curator_id ? get_user($curator_id) : null;

  
  if($curatorObj) { // used for chat
    $jsonObj->curator_name = $curator_name = $curatorObj->display_name;
    $jsonObj->curator_email = $curator_email = $curatorObj->email_address;
    $jsonObj->curator_greeting = "Hello, I'm {$curator_name}!";
    $jsonObj->curator_image = $curator_image = user_photo($curatorObj, 'big', ['full' => 1]);

    $curator = [];
    foreach($user_fields as $f) $curator[$f] = $curatorObj->$f;
    $curator['image'] = $curator_image;
    
    $curator['description'] = safe_text($curator['description']);
    
    $jsonObj->curator = $curator;
  }
  
  if($u_id) { // used for chat
    $userObj = get_user($u_id);
    foreach($user_fields as $f) $user[$f] = $userObj->$f;    
  }
  
  $jsonObj->base_link = page_link(PAGE_GALLERY, $obj_type, $collectionObj->type);
  
  print_log("cid: $curator_id name: $curator_name email: $curator_email".dump($curatorObj,true), 'gallery', LOG_LEVEL_ALWAYS);

  $media_ids = [];
  $mapObjs = find_objects('artwork_media', array('artwork_id' => $artwork_id), array('fields' => ['id', 'media', 'artwork_id', 'description', 'type'], 'sort' => 'row_order'));
  foreach($mapObjs as $mapObj) {
    $mapArray[$mapObj->media] = $mapObj->id; 
    $get_media = get_object('media', $mapObj->media);

    $title = $artworkObj->title;
    $type_val = related_images_type($mapObj->type);
    $artist_name = user_display_name($artworkObj->artist_id);

    $name = $title." by ".$artist_name." (".$type_val."), ".$mapObj->description;
    $get_media->name = $name;
    list($id, $errors) = update_object('media', $get_media, $get_media->id);
    if($errors) json_error($errors);
  }
  $media_ids = array_keys($mapArray);

  if($artworkObj->title_image)
    array_unshift($media_ids, $artworkObj->title_image);

  $artwork_link = site_full_url($siteObj)."/item/";


  $obj_id_list = implode(',', $media_ids);
  $base_url = $_SERVER['HTTP_HOST'];  
  $protocol = "https";
  $fields = array(
    'id as media', 
    'src', 
    'path', 
    'name as title', 
    'comment',
    "concat ('$protocol', '://', '$base_url', path, 'full/', src) as href",
    "concat ('$protocol', '://', '$base_url', path, 'small/', src) as href_small",
    'original_currency' ,
    'original_price'    ,
    'original_height'   ,
    'original_width'    ,
    'original_url'      ,
    'original_count'    ,
    'original_available',
    'original_year'     ,
    'original_technique',
    'digital_price'     ,
    'digital_height'    ,
    'digital_width'     ,
    'height as h'    ,
    'width as w'     ,
    'artist_id');
  
  $items = find_objects('media', array('id' => $media_ids), array('sort' => "FIELD(id, $obj_id_list)", 'fields' => $fields));

  foreach($items as &$item) {
    $item->map_id = $map_id = $mapArray[$item->media];
    $slug = slugify($item->title);
    $item->link = "{$artwork_link}{$slug}~{$map_id}";
    $item->comment = safe_text($item->comment);
  }

  $artist_ids = find_objects('media', array('id' => $media_ids), array('fields' => "distinct (artist_id)"));
  $artist_names = $artists = [];
  foreach($artist_ids as $artist_id) {
    $artistObj = get_user($artist_id);
    $artist_names[$artist_id] = $artist_name = $artistObj->display_name;
    $artist_image = $artistObj->photo ? user_photo($artistObj, 'big', ['full' => 1]) : '';
    
    $artist = [];
    foreach($user_fields as $f) $artist[$f] = $artistObj->$f;
    $artist['image'] = $artist_image;
    $artist['description'] = safe_text($artist['description']);
    
    $artists[$artist_id] = $artist;
  }
  
  $jsonObj->artists = $artists;
  $jsonObj->artist_names = $artist_names;
  $jsonObj->target = $target;
  

  $shareObj = $share_id ? get_object('sharing', $share_id) : 0;
  $pluginOptions = $shareObj ? ['infobar_show' => $shareObj->infobar_show, 'infobar_control' => $shareObj->infobar_control, 'border_width' => $shareObj->border_width, 'border_radius' => $shareObj->border_radius, 'border_color' => $shareObj->border_color] : [];    
  
  $jsonObj->obj_type = $obj_type;
  $jsonObj->obj_id = $obj_id;
  $jsonObj->site_id = $site_id;
  if($jsonObj->user_id = $u_id) {
    $jsonObj->user = $user;
  }
  $jsonObj->user_name = $u_id ? user_display_name($u_id) : '';
        
  $jsonObj->options = $pluginOptions;
  
  $jsonObj->items = $items;
  
  if($artwork_id) {
    $jsonObj->collection_id = $artwork_id;
    $jsonObj->user_collection_id = 0;
    $jsonObj->follow = [];      
    $user_collection_id = 0;
    if($u_id) {
      $user_collection_id = gallery_find_user_collection($siteObj, $u_id, array('return' => 'id'));
      if($user_collection_id) { 
        $jsonObj->user_collection_id = $user_collection_id;
        $jsonObj->follow = $follow_ids = find_objects('artwork_media', array('artwork_id' => $user_collection_id), array('fields' => 'distinct (media)'));
      }
    }
  }
  //dump($jsonObj);
  return $jsonObj;
}


/** new functions for template based layout */

/** hack for now to update maps with media data */
function update_media_collection_maps() {
  db_query("update media,gallery set media.artist_id=gallery.user_id where media.parent_type='gallery' and media.parent_id=gallery.id");
  db_query("update media_collection_map,media set media_collection_map.name=media.name,media_collection_map.artist_id=media.artist_id,media_collection_map.original_year=media.original_year,media_collection_map.original_technique=media.original_technique where media_collection_map.media_id = media.id");
  db_query("update media_collection_map,user_profile set media_collection_map.artist_name=user_profile.display_name where media_collection_map.artist_id = user_profile.user_id");
  db_query("update media_collection_map,user_profile set media_collection_map.curator_name=user_profile.display_name where media_collection_map.curator_id = user_profile.user_id");
  return "OK, updated gallery data";
}

// returns src of collection
function collection_title_image($collectionObjOrId, $options=NULL) {
  $collectionObj = any2obj('media_collection', $collectionObjOrId);
  if(!$collectionObj) return '';
  if($media_id = $collectionObj->title_image) return get_media_source($media_id, $options);
  $media_id = find_object('media_collection_map', array('media_collection_id' => $collectionObj->id, 'active' => 1), 'media_id', array('sort' => 'rank'));
  if($media_id) return get_media_source($media_id, $options);
  return '';  
}

// returns src of folder
function folder_title_image($folderObjOrId, $options=NULL) {
  $folderObj = any2obj('gallery', $folderObjOrId);
  if(!$folderObj) return '';
  if($media_id = $folderObj->title_image) return get_media_source($media_id, $options);
  $media_id = find_object('media', array('parent_type' => 'gallery', 'parent_id' => $folderObj->id, 'active' => 1), 'id', array('sort' => 'rank'));
  if($media_id) return get_media_source($media_id, $options);
  return '';  
}

/** gets media object from mapObj and merges the two */
function get_artwork($mapObj_or_id) {
  $mapObj = any2obj('media_collection_map', $mapObj_or_id);
  $artObj = get_object('media', $mapObj->media_id);
  $media_id = $artObj->id;
  $artObj->media_id = $media_id;
  $artObj->map_id = $artObj->id = $mapObj->id;
  $keys = array('media_collection_id', 'user_id', 'rank');
  foreach($keys as $fld) $artObj->$fld = $mapObj->$fld;
  return $artObj;

}

function gallery_find_user_collection($siteObj_or_id, $userObj_or_id, $options='') {
  $siteObj = any2obj('site', $siteObj_or_id);
  $site_id = $siteObj->id;
  $userObj = any2obj('user', $userObj_or_id);
  print_log("gallery_find_user_collection user={$userObj->id}", 'gallery', LOG_LEVEL_ALWAYS);
  if(!$userObj) {
    print_log("Could not find user for obj/id:".dump($userObj_or_id, true), 'gallery', LOG_LEVEL_ALWAYS);
    print_log("Stack: ".caller_stack(), 'gallery', LOG_LEVEL_ALWAYS);
    return false;    
  }
  
  $user_id = $userObj->id;
  $return = $options['return'];
  $collection_type = $options['collection_type'] ?: MEDIA_COLLECTION_TYPE_COLLECTION;
  $myCollectionObj = find_object('media_collection', array('site_id' => $site_id, 'user_id' => $user_id, 'type' => $collection_type, 'active' => 1));

    
  if(!$myCollectionObj) {
    print_log("Could not find collection for user $user_id", 'gallery', LOG_LEVEL_ALWAYS);
    $my_collection_id = gallery_add_user_collection($siteObj, $userObj); // else add collection
    $myCollectionObj = get_object('media_collection', $my_collection_id);
  }
  return $return == 'id' ? $myCollectionObj->id : $myCollectionObj;
}

function gallery_add_user_collection($siteObj_or_id, $userObj_or_id, $options=array()) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $site_id = $siteObj->id;
  $userObj = any2obj('user', $userObj_or_id);
  $user_id = $userObj->id;
  $collection_type = $options['collection_type'] ?: MEDIA_COLLECTION_TYPE_COLLECTION;
  $collection_title = $options['collection_titlre'] ?: phrase('my_collection', CAPITALIZE);

  $colData['active'] = 1;
  $colData['site_id'] = $site_id;
  $colData['user_id'] = $user_id;
  $colData['type'] = $collection_type;
  $colData['title'] = $collection_title;
  list($collection_id, $errors) = add_object('media_collection', $colData);
  print_log("Adding personal collection for user $user_id", 'gallery', LOG_LEVEL_ALWAYS);
  
  if($errors) {
    //dump($errors);
  } else {
    $messages[] = "Added collection id=$collection_id<br>";
  }
  return $collection_id;
}

/** return list of mapObjs representing unique artwork for given collection ids */

function gallery_unique_artworks($collection_ids, $options) {
  //dump($collection_ids);
  $mapObjs = find_objects('media_collection_map', array('media_collection_id' => $collection_ids, 'active' => 1), $options);
  $media_ids = array();
  $objs = array();
  foreach($mapObjs as $mapObj) {
    $media_id = $mapObj->media_id;
    if($media_ids[$media_id]) continue;
    $media_ids[$media_id] = 1;
    $objs[] = $mapObj;
  }
  
  return $objs;  
}


/** return list of selected art*/

function gallery_selected_artworks($user_id, $options) {
  $get_id = find_object('media_collection', array('user_id' => $user_id, 'type' =>150, 'active' =>1));
  $mapObjs = find_objects('media_collection_map', array('media_collection_id' => $get_id->id));
  $objs = array();
  foreach($mapObjs as $mapObj) {
    $objs[] = $mapObj;
  }  
  return $objs;  
}

/** finds first curator/exhibition/project for media_map_object */
function gallery_item_related($siteObj_or_id, $mapObj_or_id, $options = array()) {  
  $siteObj = any2obj('site', $siteObj_or_id);
  $mapObj = any2obj('media_collection_map', $mapObj_or_id);
  $media_id = $mapObj->media_id;   
  //$media_id = "1111111111111";
  $result = new stdClass;

  if(!$media_id) return $result;

  if($collection_ids = find_objects('media_collection_map', array('media_id' => $media_id), array('fields' => 'distinct media_collection_id'))) {
    //$project     = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'id'));
    $exhibition  = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'id'));    
    $curator     = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'user_id'));
    //$artists     = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'user_id'));
  
    //if($project) {
    //  $result->project_id = $project->id;
    //}     
    if($curator) {
      $result->curator_id = $curator->user_id;
      $result->curator = get_user($curator->user_id);
    }     
    if($exhibition) {
      $result->exhibition_id = $exhibition->id;
      $result->exhibition = get_object('media_collection', $exhibition->id);
    }     
  }  
  return $result;
}

/** finds first curator/exhibition/project for artwork */
function gallery_artwork_related($siteObj_or_id, $artworkObj_or_id, $options = array()) {  
  $siteObj = any2obj('site', $siteObj_or_id);
  $artworkObj = any2obj('artwork', $artworkObj_or_id);
  $artwork_id = $artworkObj->id;   
  //$media_id = "1111111111111";
  $result = new stdClass;

  if(!$artwork_id) return $result;

  if($collection_ids = find_objects('media_collection_map', array('active' => 1, 'artwork_id' => $artwork_id), array('fields' => 'distinct media_collection_id'))) {
    //$project     = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'id'));
    $exhibition  = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'id'));    
    $curator     = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'user_id'));
    //$artists     = find_object('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'user_id'));
  
    //if($project) {
    //  $result->project_id = $project->id;
    //}     
    if($curator) {
      $result->curator_id = $curator->user_id;
      $result->curator = get_user($curator->user_id);
    }     
    if($exhibition) {
      $result->exhibition_id = $exhibition->id;
      $result->exhibition = get_object('media_collection', $exhibition->id);
    }     
  }  
  return $result;
}

     

      //dump($related);
      /**
      if($collection_id = $obj->media_collection_id) {
        $collectionObj = get_object('media_collection', $collection_id);
        $collection_type = $collectionObj->type;
        echo("collection_id = $collection_id type=$collectionObj->type<br>");
        if($collection_type == MEDIA_COLLECTION_TYPE_EXHIBITION) {
          echo("type = Exhibition<br>");
          if($curator_id = $collectionObj->user_id) {
            echo("curator_id=$curator_id<br>");
            $scope->curator_id = $curator_id;
            $scope->curator = get_user($curator_id);
          }
        }
      }
      */
    

function gallery_explore($siteObj_or_id, $obj_type, $id, $options = array()) {
  global $u_id, $u_level;
  
  $obj = $obj_type && $id ? get_object($obj_type, $id) : null;
  $auth_edit = $obj ? gallery_auth_media($obj_type, $obj, $u_id) : false; 
  $edit = $auth_edit && $options['edit'];
  $embed = $auth_edit && $options['embed'];
  $sharing_id = $options['sharing_id'];
  $preview = $options['preview'];
  $tutorial_button = "";
    
  $siteObj = any2obj('site', $siteObj_or_id);  
  $site_id = $siteObj->id;
  $obj_id = $id;
  //if($obj_type == 'media_collection_map') {
  //  $obj_id = get_object('media_collection_map', $id, 'media_id');    
  //  $obj_type = 'media';
  //}

  $scope = new stdClass;
  $scope->obj_type = $obj_type;
  $scope->obj_id = $obj_id;
  $scope->vault = 1;
  foreach($options as $k => $v) $scope->$k = $v;
  
  $edit_obj_type = $obj_type;
  $edit_obj_id = $id;
  switch($obj_type) {
  case 'media_collection_map':
    $template = $edit ? '_gallery-artwork-form' : '_gallery-artwork-detail';
    $controller = 'gallery_artworks';
    $tutorial_button = 
    " <a class=\"btn btn-primary popup-iframe\" target=\"vimeo\" href=\"https://vimeo.com/145297358\">
      <i class=\"fa fa-play-circle margin-right\"></i>
      Editing Art
    </a>";
    
    break;
  case 'media_collection':
    if($id) {
      if($embed) {
        if($sharing_id) $template = $preview ? '_gallery-collection-embed-preview' : '_gallery-collection-embed-edit'; 
        else $template = '_gallery-collection-embed';      
      } else {
        $template = $edit ? '_gallery-collection-form' : '_gallery-collection-detail';
        //if($edit) $scope->uploader = object_media_uploader($obj_type, $obj_id, 'title_image');
      }
    } else {
      $template = '_gallery-collections.html';
    }
    $controller = 'gallery_collections';
    break;
  case 'user_profile':
    $userObj = get_object('user', $obj->user_id);
    if(user_has_role($userObj, USER_ROLE_CURATOR)) $template = '_user-profile-curator.html';
    elseif(user_has_role($userObj, USER_ROLE_ARTIST))  $template = '_user-profile-artist.html';
    else $template = '_user-profile-collector.html';
    
    $controller = 'artist_profile';
    $scope->obj_type = 'user';
    $scope->obj_id = $userObj->id;     
    //return "user: {$userObj->id} profile:{$userObj->display_name} ctrl: {$controller} tpl:{$template}";
    break;
  case 'user':
    $template = '_gallery-artwork-detail';
    $controller = 'gallery_artworks';
    break;
  case 'gallery':
    //die("GH: auth=$auth_edit options={$options['edit']} edit=$edit".dump($options, true));
    $template = $edit ? '_gallery-folder-form' : '_gallery-folder-detail';
    $controller = 'gallery_folder';
    $tutorial_button = 
    " <a class=\"btn btn-primary popup-iframe\" target=\"vimeo\" href=\"https://vimeo.com/128292540\">
      <i class=\"fa fa-play-circle margin-right\"></i>
      Tutorial - Adding and Removing Art
    </a>";
    
    break;
  case 'media':
    //$edit = $auth_edit;
    $template = $edit ? '_gallery-artwork-form' : '_gallery-artwork-detail';
    $controller = 'gallery_artworks';
    $tutorial_button = 
    " <a class=\"btn btn-primary popup-iframe\" target=\"vimeo\" href=\"https://vimeo.com/145297358\">
      <i class=\"fa fa-play-circle margin-right\"></i>
      Editing Art
    </a>";
    break;
  default:
    return "Gallery Explore: Invalid type $obj_type";
    break;
  }
  //if($sharing_id) {
  //  $scope->sharing_id = $sharing_id;
  //  $scope->preview = $preview;
  //}
  $pageObj = new stdClass;
  $pageObj->template = $template;
  $pageObj->controller = $controller;
  
  $output = '';
  $load_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=$obj_type&id=$id";      
  $preview_button = html_link("#", html_i('', "fa fa-arrow-left")." ".phrase('back_to_preview', CAPITALIZE), array('class' => 'btn btn-success ajax-loader', 'data-url' => $load_url, 'data-target' => '#cms_viewer'))." ";
  if($edit) {
    //$output .= $preview_button;    
  } else if($auth_edit) {
    if(!$embed) {
      $load_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=$edit_obj_type&id=$edit_obj_id&edit=1"; 
      $edit_button = html_link("#", html_i('', "fa fa-pencil")." ".phrase('edit', CAPITALIZE), array('class' => 'btn btn-success ajax-loader', 'data-url' => $load_url, 'data-target' => '#cms_viewer'));
      $output .= $edit_button;
    }
    if($obj_type == 'media_collection') {
      $class = $embed ? "visuallyhidden" : ""; /** we still need this link for reloading, but don't show it */
      $load_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=$edit_obj_type&id=$edit_obj_id&embed=1"; 
      //$embed_button = html_link("#", html_i('', "fa fa-link")." ".phrase('embed', CAPITALIZE), array('id' => 'embed_link', 'class' => "btn btn-success ajax-loader $class", 'data-url' => $load_url, 'data-target' => '#cms_viewer'));
      $embed_button = html_link(site_page_link($siteObj, PAGE_PLUGIN)."?pt={$edit_obj_type}&pid={$edit_obj_id}", html_i('', "fa fa-link")." ".phrase('embed', CAPITALIZE), array('id' => 'embed_link', 'class' => "btn btn-success", 'target' => 'embed'));
      $output .= " ".$embed_button;
    }
  }

  
  if($edit || $embed) $output .= $preview_button;
  $output .= $tutorial_button;

  $this_app = new stdClass;
  $app->page = $pageObj;
  $app->site = $siteObj;
  
  $output .= fetch_template($app, ['scope' => $scope]);
  return $output;
} 

 

function gallery_title_text($siteObj, $search, $options=NULL) {
  $obj_type = $search['obj_type'];
  $id       = $search['id'];
  $text = '';
  switch($obj_type) {
    case 'media': // single image from database      
      $obj = $imageObj = get_object($obj_type, $id);


      $year = $imageObj->original_year;
      $h = $imageObj->original_height;
      $w = $imageObj->original_width;
      $currency = $imageObj->original_currency ?: 'EUR';
      $price = $imageObj->original_price;
      
      if($imageObj->name) $text .= html_h3($imageObj->name.($year ? " ($year)" : ''));
      
      $orgArray = array();
      
      if($technique = $imageObj->original_technique) $text .= html_h4($technique);
      if($h > 0 && $w > 0) {
        $h_in = round($h / 2.54);
        $w_in = round($w / 2.54);
        $text .= phrase('height', CAPITALIZE).": ".$h."cm ($h_in\")"." x ".phrase('width', CAPITALIZE).": ".$w."cm ($w_in\")";
      }
      
      $count_str = $price_str = $available_str = $count_str = $dot = '';
      if($currency && $price > 0) $orgArray[]= get_currency_symbol($currency, true).formatFloat($price);
      $count = $imageObj->original_count;
      if($count > 0) $orgArray[]= $count > 1 ? "Series of $count" : "Original of one";    
      $available = $imageObj->original_available;
      $original_help_text = '';
      if($available == DB_YES) {
        $original_help_text = "Interested in the original? Please contact the curator and/or the artist for the latest information about price and availability, both of which may be subject to change.";
        $orgArray[]= 'Original available';
        $dot = html_span('&#x25cf;', 'dot_outside green');
      } elseif($available == DB_NO) {
        $dot = html_span('&#x25cf;', 'dot_outside red');
      } else {
        $dot = '';
      }
  
      $text .= html_div($dot.implode(', ', $orgArray), 'relative');
      
      //if($sd = $imageObj->original_technique ?: $imageObj->short_description) $text .= html_h4($sd);
      if($artist_name) {
        $artistAr = array_filter(array($artist_name,$artist_link,$artist_email));      
        $text .= html_div(phrase('artist', CAPITALIZE).': '.implode(', ', $artistAr));
      }
      if($curator_name) {
        $curatorAr = array_filter(array($curator_name,$curator_link,$curator_email));      
        $text .= html_div(phrase('curator', CAPITALIZE).': '.implode(', ', $curatorAr));
      }
      if($curatorObj) {
        //$text .= html_h4($imageObj->short_description);
      }
      
      if($desc = $imageObj->comment) $text .= html_p($desc);
      if($original_help_text) $text .= html_div($original_help_text, 'margin-top margin-bottom small-text'); 

      break;
    case 'gallery': // single image from database
      $obj = get_object($obj_type, $id);
      $text = html_h2($obj->title);
      $text .= html_p($obj->description);      
      break;
    case 'artist': // single image from database
      $text = user_profile_text($siteObj, $id, $options);
      break;
    default:
      break;
  }
  return $text;      
}

// uploader for new dynatree cms
function gallery_uploader($galleryObj_or_id, $options = array()) {
  global $u_id, $site_id,$u_level;
  $galleryObj = any2obj('gallery', $galleryObj_or_id);
  $gallery_id = $galleryObj->id;
  
  $class_attrs = get_object_attributes('gallery', false);
  $upload_options = $class_attrs['media'][ATT_LENGTH];
  $upload_options['minimum_size'] = array(1080, 1920); // either short side >= 1080 or long side >= 1920
  $fld = 'media';
  $val = $galleryObj->media;
     
  $mediaData['user_id'] = $u_id ?: $galleryObj->user_id;
  $mediaData['artist_id'] = $galleryObj->user_id;
  $mediaData['parent_id'] = $gallery_id;
  $mediaData['parent_type'] = 'gallery';
  $mediaData['parent_field'] = 'media';

  //$handler_options = array('handler' => 'cms_media_handler', 'handler_operation' => 'new-gallery-folder', 'target' => 'ajax_messages');
  
  $upload_options['download_template'] = $options['download_template'] ?: "";
  
  $upload_options['media_data'] = $mediaData;
  $upload_options['data-value_list'] = $val;
  $upload_options['show_file_list'] = false;
  $upload_options['title'] = "Click or drag to upload digital content";
  //$upload_options['handler'] = 'cms_media_handler';
  
  $handler =isset($options['handler']) ? $options['handler'] : 'refreshActiveNode';
  if($upload_options['handler'] = $handler) {
    $handler_options = array();
    $upload_options['handler_options'] = $handler_options;
  }


  // add edit button to uploader
  $edit_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=media&edit=1"; // id will be appended
  $upload_options['edit_url'] = $edit_url;
  $upload_options['edit_target'] = '#cms_viewer';

  //dump($upload_options);
  $uploader = jquery_file_uploader("gallery-$gallery_id-$fld", $val, $upload_options);
  //$uploader = fine_uploader("gallery-$gallery_id-$fld", $val, $upload_options);
  return $uploader;
}

function gallery_auth_media($obj_type, $obj_or_id, $user_id, $auth_ids = []) {  
  global $u_level;
  $obj = any2obj($obj_type, $obj_or_id);
  if($obj->user_id == $user_id) return true;
  if(is_array($auth_ids) && in_array($obj->user_id, $auth_ids)) return true;
  if(is_array($auth_ids) && in_array($obj->artist_id, $auth_ids)) return true;
  //die("id: {$obj->id} user_id={$user_id} obj_user_id={$obj->user_id } not in ".dump($auth_ids,true));
  
  $auth_edit = false;
  $user_level = $u_level ?: get_object('user', $user_id, 'level');
  $is_staff   = $user_level >= USER_LEVEL_STAFF;
  $is_admin   = $user_level >= USER_LEVEL_ADMIN;
  if($is_staff || $is_admin) return true;
  $is_curator = $user_id && user_has_role($user_id, USER_ROLE_CURATOR);
  $is_artist = $user_id && user_has_role($user_id, USER_ROLE_ARTIST);

  switch($obj_type) {
  case 'media':
      
    // temp: todo - mayday, hack for now to let all edit
    return true;
    
    $owner_id = $obj->user_id;
    $artist_id = $obj->artist_id;
    $is_owner   = $user_id && $owner_id && $owner_id == $user_id;
    $is_artist  = $user_id && $artist_id && $artist_id == $user_id;              
    $auth_edit = $is_staff || $is_owner || $is_artist ? 1 : 0;
    
    break;
  case 'gallery':
    $auth_edit = $obj->user_id == $user_id;
    break;
  case 'media_collection':
    $auth_edit = $obj->user_id == $user_id;
    break;
  case 'media_collection_map':
    $collection_user_id = get_object('media_collection', $obj->media_collection_id, 'user_id');
    $auth_edit = $collection_user_id == $user_id;
    break;
  default:
    break;
  }

  return $auth_edit;
}

function gallery_create_button($siteObj_or_id, $userObj_or_id, $options=NULL) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $userObj = any2obj('user', $userObj_or_id);
  $data = array('site_id' => $siteObj->id, 'user_id' => $userObj->id);
  $folder_tip = "You must create folders in your studio in order to upload and organize your art into vPatina";
  $shared_tip = "You must create shared folders in order to share art with the galleries you collaborate with.";

  /**
  Share your digital editions with curators, or choose to display them<br>
                 on your profile page, by creating a shared folder and dragging your<br>
                 digital editions into it."; // phrase("text_cms_shared_folder_help")
                 */
                 
  $exhibit_tip = "You must create exhibition folders in order to create exhibitions that can be embedded on any website of your choice";
  /**
  Curate exhibitions, featuring a range of artists, by dragging<br>
                  digital editions to an Exhibition folder. These exhibitions can be<br>
                  imbedded on any website of your choice. "; // phrase("text_cms_exhibition_help")
                  */
                  
  if(user_has_role($userObj, USER_ROLE_ARTIST))  $items[] = html_link('#', html_span(' ', 'fontello icon-folder').phrase('folder', CAPITALIZE), array('class' => 'cms-create', 'id' => 'cms_create_folder', 'data-obj_type' => 'gallery', 'data-site_id' => $siteObj->id, 'data-user_id' => $userObj->id))." ".html_tooltip($folder_tip, "help");
  if(user_has_role($userObj, USER_ROLE_ARTIST))  $items[] = html_link('#', html_span(' ', 'fontello icon-export-2').phrase('shared_folder', CAPITALIZE), array('class' => 'cms-create', 'id' => 'cms_create_project', 'data-obj_type' => 'media_collection', 'data-type' => MEDIA_COLLECTION_TYPE_PROJECT, 'data-site_id' => $siteObj->id, 'data-user_id' => $userObj->id))." ".html_tooltip($shared_tip, "help");
  if(user_has_role($userObj, USER_ROLE_CURATOR)) $items[] = html_link('#', html_span(' ', 'fontello icon-picture').phrase('exhibition', CAPITALIZE), array('class' => 'cms-create', 'id' => 'cms_create_exhibition', 'data-obj_type' => 'media_collection', 'data-type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'data-site_id' => $siteObj->id, 'data-user_id' => $userObj->id))." ".html_tooltip($exhibit_tip, "help");
                                                                                                                                                                                                                                                                                
  //  $items[] = html_link($link, phrase('my_vault', CAPITALIZE));


  $menu = html_ul($items);
  

  $output = html_link('#', html_i('', 'fa fa-upload margin-right').phrase('create', CAPITALIZE), array('id' => 'cms-create-button', 'class' => 'btn btn-success toggle-div dropdown', 'data-toggle' => 'gallery-create-menu', 'title' => 'foo bar'));

  $output .= html_div($menu, 'user-menu ztop click_hide', "id='gallery-create-menu'");
  return html_div($output, 'user-info');
}

/** end new functions for template based layout */


// custom object_form() version for artwork
function gallery_media_form($siteObj, $obj=null, $options = array()) {
  $obj_type = 'media';
  
  // fields
  $fields = array(
    'name' => 'Title *', 
    'original_technique' => 'Technique *', 
    'digital_price' => 'Price of digital edition', 
    'original_height' => 'Height of original (cm)', 
    'original_width' => 'Width of original (cm)', 
    'original_year' => 'Year of creation', 
    'original_count' => 'Number in series', 
    'original_available' => 'Available',
    'original_price' => 'Price of original work', 
    // 'original_url' => 'URL of original work', 
    'comment' => 'Description of artwork', 
    );
  //$attr_fields = attribute_fields($fields);

  $tip = "Add information to the artwork such as availability and the price of the original. 
          To release High Definition digital editions to collectors, set a price for the digital edition above 0.00.
          If the digital edition price is set to 0.00, the High Definition digital edition is restricted (not released to collectors).<br>";
  
  $editOptions['title'] = $obj ? phrase('edit', CAPITALIZE)." $obj->name" : phrase('add_artwork', CAPITALIZE);
  $editOptions['tip'] = $tip;
  $editOptions['fields'] = $fields;
  $editOptions['autosave'] = true;

  $help_text = html_p("Please note fields marked with an * are mandatory");
  $tc_link = html_link("/terms-and-conditions/", "Terms and Conditions");
  $cl_link = html_link("/digital-collector-license/", "Digital Collector License");
  $fp_link = html_link("/fee-payment-schedule/", "Fee & Payment Schedule");
  
  $help_text .= html_p("The sale of digital editions is governed by the $tc_link, the $cl_link and the $fp_link. High Definition digital editions will are a maximum of 1920 x 1080 pixels and Medium Definition digital editions are a maximum of  640 x 640 pixels.");

  global $u_level,$u_id;
  $site_id = $siteObj->id;
  $media_id = $obj->id;
  $load_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=media&id=$media_id";      
  $preview_button = html_div(html_link("#", html_i('', "fa fa-check")." ".phrase('save', CAPITALIZE), array('class' => 'ajax-loader', 'data-url' => $load_url, 'data-target' => '#cms_viewer')));

  return html_div($preview_button, 'margin-top').object_form($obj_type, $obj, $editOptions).html_div($help_text);
}

/** old code below */

// custom object_form() version for artwork
function gallery_collection_form($siteObj, $obj=null, $options = array()) {
  $obj_type = 'media_collection';
  
  // fields
  $fields = array(
    'title' => 'Title *', 
    'description' => 'Description', 
    'title_image' => 'Title Image',
    );
  //$attr_fields = attribute_fields($fields);

  $editOptions['title'] = $obj ? phrase('edit', CAPITALIZE)." $obj->name" : phrase('add_artwork', CAPITALIZE);
  $editOptions['fields'] = $fields;
  $editOptions['autosave'] = true;

  $help_text = html_p("Please note fields marked with an * are mandatory");
  global $u_level,$u_id;
  $site_id = $siteObj->id;
  $id = $obj->id;
  $load_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=$obj_type&id=$id";      
  $preview_button = html_div(html_link("#", html_i('', "fa fa-eye")." ".phrase('preview', CAPITALIZE), array('class' => 'ajax-loader', 'data-url' => $load_url, 'data-target' => '#cms_viewer')));

  return html_div($preview_button, 'margin-top').object_form($obj_type, $obj, $editOptions).html_div($help_text);
}
  
function gallery_share_data($siteObj, $share_link, $options = array()) {
  global $u_id;
  if(!$share_link) return array();
  $site_id = $siteObj->id;
  
  if(!is_url($share_link)) $share_link = site_full_url($siteObj, array('url' => $share_link));
  $share_link = str_replace('//test.', '//www.', $share_link);

  $spid = $_GET['_sid'];  // this was already shared
  $share_id = uniqid();
  $shareArray = array_filter(array('uid' => $u_id, 'sid' => $site_id, 'pid' => $_SESSION['page_id'], 'cid' => $options['collection_id'], 'obt' => $options['obj_type'], 'oid' => $options['object_id'], 'spid' => $options['spid'], 'title' => $options['title']));
  $share_json = json_encode($shareArray);
  $qs = array2qs(array('_sid' => $share_id, 'jsn' => $share_json), true); // this shows one image only
  $share_link .= strpos($share_link, '?') ? "&$qs" : "?$qs";
  $full_link = $share_link;
  $share_link = url_shorten(strip_affiliate($share_link));

  $tweet = ($collection_name ? "$collection_name: " : '' )."$title";
  if($user_name) $tweet .= " ($user_name)";

  $shareData['full_link'] = $full_link;
  $shareData['data'] = $shareArray;
  $shareData['link'] = $shareData['short_link'] = $share_link;
  $shareData['count'] = 'none';
  $shareData['counturl'] = $share_link;
  $shareData['title'] = $title;
  $shareData['tweet'] = $tweet;
  $shareData['size'] = 'small';
  $shareData['hashtags'] = 'vpatina';
  $shareData['class'] = 'fb-loader-popup';
  $shareData['image'] = $options['image'];
  return $shareData;
}

// list of related artists, curators, exhibitions
function gallery_related($siteObj_or_id, $obj_type, $id, $options = array()) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $site_id = $siteObj->id;
  $debug .= "ot=$obj_type id = $id<br>";
  $obj = $options['object'] ?: get_object($obj_type, $id);
  $limit = 2;
  $start = 0;
  
  //die("related obj_type = $obj_type");
  switch($obj_type) {
    
  case 'media_collection_map': // return artist, curator(s), exhibition(s), and project(s)?
    if($mapObj = get_object('media_collection_map', $id)) {
      $media_id = $mapObj->media_id;
      //$obj_type = 'media';
    }

    $collection_ids = find_objects('media_collection_map', array('media_id' => $media_id), array('fields' => 'distinct media_collection_id', 'start' => $start, 'limit' => $limit, 'sort' => 'rank'));
    $projects    = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'distinct id', 'start' => $start, 'limit' => $limit, 'sort' => 'rank'));
    $exhibitions = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'distinct id', 'start' => $start, 'limit' => $limit, 'sort' => 'rank'));    
    $curators    = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'distinct user_id', 'start' => $start, 'limit' => $limit, 'sort' => 'rank'));
    $artists     = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'distinct user_id', 'start' => $start, 'limit' => $limit, 'sort' => 'rank'));
    $debug = "Collection ids: (".count($collection_ids).") ".implode(',', $collection_ids);
    $debug .= "Exhibition ids: (".count($exhibitions).") ".implode(',', $exhibitions);
    break;

  case 'media_collection': // return artist, curator(s)
    $collection_ids = $id;
    $image_options['base_link'] = gallery_collection_link($siteObj, $obj);  
    $curators = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'distinct user_id'));
    $mapObjs   = find_objects('media_collection_map', array('media_collection_id' => $collection_ids, 'active' => 1), ['sort' => 'rank']);
    $media_ids = $mapObjs ? array_find_key($mapObjs, 'media_id') : array();
    
    $artists  = find_objects('media', array('id' => $media_ids, 'active' => 1),  array('fields' => 'distinct user_id'));
    $debug .= "Collection: Artists=".count($artists)." Curators=".count($curators); // $userObj->username." is artist with projects: ".implode(',', $projects);
    $debug = count($media_ids) ? "Media ids: (".count($media_ids).") ".implode(',', $media_ids).html_break() : '';
    foreach($media_ids as $dmedia_id) {
      $dmediaObj = get_object('media', $dmedia_id);
      $duser_id = $dmediaObj->user_id;
      $duserObj = get_object('user', $duser_id);
      $dmapObjs = find_objects('media_collection_map', array('media_id' => $dmedia_id));
      $map_count = count($mapObjs);
      if($map_count == 1) {
        $dmapObjs = $mapObj[0];
      }
      $debug .= "Media $dmedia_id owner=$duserObj->username: $dmediaObj->name count=$map_count".html_break();
    }
    break;

  case 'user': // return curator(s), exhibition(s), and project(s)?
    $userObj = get_object('user', $id);
    if(user_has_role($userObj, USER_ROLE_ARTIST)) {
      $projects = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'user_id' => $userObj->id, 'active' => 1),  array('fields' => 'distinct id')); // find all exhibitions by this curator
      $media_ids = find_objects('media_collection_map', array('media_collection_id' => $projects, 'active' => 1),  array('fields' => 'distinct media_id'));
      $collection_ids = find_objects('media_collection_map', array('media_id' => $media_ids, 'active' => 1),  array('fields' => 'distinct media_collection_id'));
      $exhibitions = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'distinct id')); // find all exhibitions by this curator
      $curators   = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $exhibitions, 'active' => 1),  array('fields' => 'distinct user_id'));
      $artists  = array();
      $debug .= $userObj->username." is artist with projects: ".implode(',', $projects);
    } else if(user_has_role($userObj, USER_ROLE_CURATOR)) {
      $exhibitions = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'user_id' => $userObj->id, 'active' => 1),  array('fields' => 'distinct id')); // find all exhibitions by this curator
      $media_ids = find_objects('media_collection_map', array('media_collection_id' => $exhibitions, 'active' => 1),  array('fields' => 'distinct media_id'));
      $collection_ids = find_objects('media_collection_map', array('media_id' => $media_ids, 'active' => 1),  array('fields' => 'distinct media_collection_id'));
      $curators  = array();
      $artists   = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $collection_ids, 'active' => 1),  array('fields' => 'distinct user_id'));
      $debug .= $userObj->username." is curator with exhibition: ".implode(',', $exhibitions);
    }
    break;
    
  default:
    echo("Explore: Missing or unsupported type $obj_type");exit;
    break;
  }                                                        

  //$debug = dump($image_options, true);
  $result->curator_ids     = $curators;
  $result->artist_ids      = $artists;
  $result->project_ids     = $projects;
  $result->exhibition_ids  = $exhibitions;
  $result->image_ids       = $media_ids;

  $result->curators     = gallery_related_list($siteObj, 'user', $curators, array('title' => 'curators', 'sprite' => 'mini-sprite-32 s213', 'list-class' => 'related-list'));
  $result->artists      = gallery_related_list($siteObj, 'user', $artists, array('title' => 'artists', 'sprite' => 'mini-sprite-32 s34', 'list-class' => 'related-list'));
  $result->projects     = gallery_related_list($siteObj, 'media_collection', $projects, array('title' => 'projects', 'sprite' => 'mini-sprite-32 s41', 'list-class' => 'related-list-inline'));
  $result->exhibitions  = gallery_related_list($siteObj, 'media_collection', $exhibitions, array('title' => 'exhibitions', 'sprite' => 'mini-sprite-32 s41', 'list-class' => 'related-list-inline'));
  $result->images       = gallery_related_list($siteObj, 'media', $mapObjs, array('image_options' => $image_options, 'title' => 'digital_editions', 'sprite' => 'mini-sprite-32 s41', 'list-class' => 'related-list-inline'));
  
  //$debug = "ot=$obj_type id=$id";
  $debug = '';
  if(!$_SESSION['live']) $result->debug = $debug;
  
  
  return $result;
}
// wrappers for gallery_object_list

// helper function  for gallery_related to print list of iamges, project, artist, etc
function gallery_related_list($siteObj, $obj_type, $ids, $options = array()) {
  if(!count($ids)) return '';
  $site_id = $siteObj->id;
  $imgOptions = $options['image_options'];
  $imgOptions['size'] = 'square';
  $link_class = 'new-tab';
  $sprite = $options['sprite'];
  $list_class = $options['list-class'];
  $title = $options['title'];
  $nav_src = "explore-data-$title"; // hidden div where we store JSON data for navigation 

  $count = count($ids);
  $items = array();
  $i=0;
  $nav_ids = array();
  foreach($ids as $id) {
    if(is_object($id)) { // media_collection_map
      $obj_type = 'media_collection_map';
      $image_id = $id->media_id; // map object
      $image_obj_type = 'media';
      $id = $id->id;
    } else {
      $image_id = $id;
      $image_obj_type = $obj_type;
    }
    $imageObj = gallery_title_image($siteObj, array('obj_type' => $image_obj_type, 'id' => $image_id), $imgOptions);
    $link = $imageObj->link;
    $link = strpos($link, '?') ? "$link&c=$count&i=$i" : "$link?c=$count&i=$i"; 
    $link_data = array('site_id' => $site_id, 'obj_type' => $obj_type, 'id' => $id, 'nav_source' => $nav_src);
    $link_options = array('class' => $link_class, 'data-data' => json_encode($link_data));
    $items[] = html_div(html_link($link, html_image($imageObj->square_src, $imageObj->title).html_div($imageObj->title, 'related-image-title'), $link_options), 'img_cont relative');
    $i++;
    $nav_ids[] = $id;
  }
  $icon = $sprite ? html_div('', "$sprite related-title-icon") : '';

  // store result for navigation (prev/next)
  $nav_data = array();
  $nav_data['site_id'] = $siteObj->id;
  $nav_data['obj_type'] = $obj_type;
  if($obj_subtype) $nav_data['obj_subtype'] = $obj_subtype;
  $nav_data['id_list'] = $nav_ids;
  $data_json = json_encode($nav_data);
  
  //$nav_data_str = html_div('', 'hidden', array("id" => $nav_src, "data-result" => $data_json));
  //$nav_data_str = "$nav_src=".$data_json;
  $nav_data_script = html_script_inline("$('body').data('$nav_src', $data_json);");  
  $output = $nav_data_script.html_div($icon.html_h3(phrase($title, CAPITALIZE), 'related-title'), 'related-title-cont').html_ul($items, $list_class);
  //$output .= $nav_data_str;
  return $output;
}

// show list of exhibitions (use gallery list)
function gallery_collection_list($siteObj_or_id, $collection_type, $userObj_or_id = NULL, $options='') {
  //return "s-$siteObj_or_id";   

  $siteObj = any2obj('site', $siteObj_or_id);
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php
  $user_id = is_object($userObj_or_id) ? $userObj_or_id->id : $userObj_or_id;
  //return dump($options, true);  

  //dump($search);
  
  $auth_edit = $options['auth_edit'];
  $search = $options['search'] ?: array(); 
  $search['site_id'] = $siteObj->id;
  $search['type'] = $collection_type;
  if(!$auth_edit && !isset($search['active'])) $search['active'] = 1;
  if($user_id) $search['user_id'] = $user_id;
  if($collection_ids = $options['collection_id']) $search['id'] = $collection_ids;
  $options['search'] = $search;

  $objs = $options['objects'] ?: find_objects('media_collection', $search);
  //return dump($search, true).dump($objs, true);  
  if($options['return'] == 'object') return $objs;
  
  $options['objects'] = $objs;
  $options['obj_type'] = 'media_collection';
  return gallery_object_list($siteObj, $options);
}

function gallery_trash($obj_type, $search, $trash=0, $title='') {
  if($obj_type == 'gallery') {
    $item_name = 'folder';
    $parent_name = 'vault';
  } else { // media
    $item_name = 'image';
    $parent_name = 'folder';
  }
  $class = 'gallery_trash';
  
  if($trash) {
    $folder_link = strip_query_string();
    $title = $title ?: phrase($parent_name);
    $trash_output = html_link($folder_link, html_image('/images/icons/back.png', phrase('back', CAPITALIZE))." ".phrase('back_to', CAPITALIZE)." $title");
  } else {
    $search['active'] = 0;
    $trash_count = count_objects($obj_type, $search);
    $trash_link = '?vr';
    $trash_icon = html_image('/images/icons/trash-full-32.png', phrase('recycle_bin', CAPITALIZE));      
    //$trash_output = html_link($trash_link, $trash_icon)." ".html_span($trash_count, 'count')." ".($trash_count==1 ? phrase($item_name) : phrase($item_name.'s'))." ".phrase('in')." ".html_link($trash_link, phrase('recycle_bin'));
    $trash_output = html_link($trash_link, $trash_icon)." ".html_link($trash_link, phrase('recycle_bin', CAPITALIZE))." [".html_span($trash_count, 'count')."]";
    if(!$trash_count) $class .= " invisible";
  }
  
  $output = html_div($trash_output, $class, array('id' => 'gallery_trash'));
  return $output;
}

// show list of galleries (folders)
function gallery_folder_list($siteObj_or_id, $userObj_or_id=NULL, $options=NULL) {  
  $siteObj = any2obj('site', $siteObj_or_id);
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php
  
  $user_id = is_object($userObj_or_id) ? $userObj_or_id->id : $userObj_or_id;

  $options['obj_type'] = 'gallery';

  //echo(dump($options, true));die();  
  $trash = $options['trash']; // show recycle bin ?

  if($user_id) $search['user_id'] = $user_id;
  $search['site_id'] = $siteObj->id;
  $search['active'] = $trash ? 0 : 1;
  $options['search'] = $search;
  

  $search = $options['search'];
  $auth_edit = $options['auth_edit'];
  
  //dump($search);
  if($auth_edit && $options['format'] != 'json') $trash_div = gallery_trash('gallery', $search, $trash, $title);    
  
  //$title = html_h3($title, 'content_subtitle');
  $output = gallery_object_list($siteObj, $options);
  if($trash_div) $output = $trash_div.$output;
  return $output;
}

// show a folder
function gallery_folder($siteObj_or_id, $galleryObj_or_id=NULL, $options='') {
  $siteObj = any2obj('site', $siteObj_or_id);  
  if($galleryObj_or_id) $galleryObj = any2object('gallery', $galleryObj_or_id);
  if(!$galleryObj) return "Folder not found";
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php

  $auth_edit = $options['auth_edit'];

  $trash = $options['trash'];  
  $search['parent_type'] = 'gallery';
  $search['parent_id'] = $gallery_id = $galleryObj->id;    
  $search['active'] = $trash ? 0 : 1;

  $options['search'] = $search;
  $options['obj_type'] = 'media';
  
  //$options['base_link'] = strip_query_string($_SERVER['REQUEST_URI']);
  $options['base_link'] = gallery_folder_link($siteObj, $galleryObj, array('base_link' => $options['base_link']));

  $trash = $options['trash']; // show recycle bin ?
  if($auth_edit && $options['format'] != 'json') $trash_div = gallery_trash('media', $search, $trash, $galleryObj->title);    
  $output = gallery_object_list($siteObj, $options);
  if($trash_div) $output = $trash_div.$output;
  return $output;
}  

// show all media matching search - e.g. property images
function gallery_media($siteObj_or_id, $search, $options='') {
  $siteObj = any2obj('site', $siteObj_or_id);  
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php
  $auth_edit = $options['auth_edit'];
  $trash = $options['trash']; // show recycle bin ?
  if($auth_edit && $options['format'] != 'json') $trash_div = gallery_trash('media', $search, $trash, $galleryObj->title);    
    
  //$search['parent_type'] = 'gallery';
  //$search['parent_id'] = $gallery_id = $galleryObj->id;    
  //$search['active'] = $trash ? 0 : 1;

  $options['search'] = $search;
  $options['obj_type'] = 'media';
  
  //$options['base_link'] = strip_query_string($_SERVER['REQUEST_URI']);
  $options['base_link'] = gallery_folder_link($siteObj, $galleryObj, array('base_link' => $options['base_link']));

  $output = gallery_object_list($siteObj, $options);
  if($trash_div) $output = $trash_div.$output;
  return $output;
}  

  // show list of users
function gallery_user_list($siteObj_or_id, $user_ids = array(), $options='') {  

  $siteObj = any2obj('site', $siteObj_or_id);
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php
  $auth_edit = $options['auth_edit'];
  $role = $options['role'];
  $site_id = $siteObj->id;

  //return "site_id=$siteObj_or_id options=".dump($options,true);
  //dump($options);

  $options['obj_type'] = 'user';
  if($base_link = gallery_user_link($siteObj, 0, array('role' => $role))) $options['base_link'] = $base_link;
    
  if(!$user_ids) { // find users
    if($role = $options['role']) {
      $role_name = get_object('user_role', $role, 'name');
      if($auth_edit || $options['show_all']) { // show all
        $aSearch[DB_LOGICAL_OR] = array('site_id' => $site_id, 'auth_sites' => "*$site_id*", 'user_level' => USER_LEVEL_SYSTEM);
        $aSearch['active'] = 1;
        $aSearch['roles'] = array('&', $role);
        $tbl = get_object_table('user');
        $user_ids = find_objects('user', $aSearch, array('fields' => "{$tbl}.id"));  
        //dump($user_ids);
      } elseif($collection_type = $options['collection_type']) {  // show only those with content
        $user_ids = find_objects('media_collection', array('type' => $collection_type, 'active' => 1, 'site_id' => $siteObj->id), array('fields' => 'distinct user_id'));
      }
      
      if($role == USER_ROLE_ARTIST && ($auth_edit || $options['show_all'])) { // also show artists who didn't publish yet
        $user_ids2 = find_objects('gallery', array('active' => 1, 'site_id' => $siteObj->id), array('fields' => 'distinct user_id'));
        //dump($user_ids2);
        $user_ids = array_unique(array_merge($user_ids, $user_ids2)); 
        //dump($user_ids);
      }
    }
  }
  
  if($user_ids) {
    $profileObjs = find_objects('user_profile', array('user_id' => $user_ids, 'site_id' => $site_id), array('sort' => 'rank'));
    if($options['return'] == 'object') return $profileObjs;
  
    $user_count = count($user_ids);
    $profile_count = count($profileObjs);
    //echo("Found $profile_count profiles for $user_count $role_name<br>");                                     
    $options['obj_type'] = 'user';
    $options['search_obj_type'] = 'gallery';
    $options['objects'] = $profileObjs;
    //dump($options);
  }

  return gallery_object_list($siteObj, $options);
}


// show list of curators
function gallery_curator_list($siteObj_or_id, $user_ids = array(), $options='') {
  $siteObj = any2obj('site', $siteObj_or_id);
  $options['size'] = $options['size'] ?: 'square';
  $options['role'] = USER_ROLE_CURATOR;
  $options['collection_type'] = MEDIA_COLLECTION_TYPE_EXHIBITION;
  return gallery_user_list($siteObj, $user_ids, $options);
}

// show list of curators
function gallery_artist_list($siteObj_or_id, $user_ids = array(), $options='') {
  $siteObj = any2obj('site', $siteObj_or_id);
  $options['size'] = $options['size'] ?: 'square';
  $options['role'] = USER_ROLE_ARTIST;
  $options['collection_type'] = MEDIA_COLLECTION_TYPE_PROJECT;
  return gallery_user_list($siteObj, $user_ids, $options);
}

// returns "product_id-variant_id" if this image is in the cart, false otherwise
function gallery_image_in_cart($siteObj_or_id, $mediaObj_or_id) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $mediaObj = any2obj('media', $mediaObj_or_id);
  if($productObj = find_object('product', array('source_type' => 'media', 'fid' => $mediaObj->id))) {
    $product_id = $productObj->id;
  } else {
    return false;
  }
  $cartObj = get_cart($siteObj);
  $items = $cartObj->items ?: array();
  foreach($items as $item) {
    if($item->product_id == $product_id) return ($item->product_id ?: 0)."-".($item->variant_id ?: 0);
  }
  return false;
}

// returns "product_id-variant_id" if this user owns this image, false otherwise
function gallery_user_owns_image($siteObj_or_id, $userObj_or_id, $mediaObj_or_id) {
  if(!$userObj_or_id) return false;
  $debug = '';
  $siteObj = any2obj('site', $siteObj_or_id);
  $userObj = any2obj('user', $userObj_or_id);
  $mediaObj = any2obj('media', $mediaObj_or_id);
  $userCollectionObj = gallery_find_user_collection($siteObj, $userObj);
  $debug = "gallery_user_owns_image 1: site:$siteObj->id user:$userObj->username media:$mediaObj->id ($mediaObj->name) collection:$userCollectionObj->id".PHP_EOL;
  
  //if($userCollectionObj) echo("Found collection $userCollectionObj->id for $userObj->username (media=$mediaObj->id)<br>");
  if($mediaObj && $userCollectionObj) {
    if($productObj = find_object('product', array('source_type' => 'media', 'fid' => $mediaObj->id))) {
      $product_id = $productObj->id;
      $debug .= "gallery_user_owns_image 2: product:$productObj->id ($productObj->name)".PHP_EOL;
      if($mapObj = find_object('media_collection_map', array('media_collection_id' => $userCollectionObj->id, 'product_id' => $product_id))) {
        if($variant_id = $mapObj->variant_id) {
          $variantObj = get_object('product_variant', $variant_id);
          $debug .= "gallery_user_owns_image 3: map:$mapObj->id variant = $variantObj->id ($variantObj->resolution)".PHP_EOL;
        } else {
          $debug .= "gallery_user_owns_image 3: map:$mapObj->id no product variant".PHP_EOL;
        }
        print_log($debug, 'gallery', LOG_LEVEL_DEBUG);
        return ($mapObj->product_id ?: 0)."-".($mapObj->variant_id ?: 0);
      }
    }
  }
  print_log($debug, 'gallery', LOG_LEVEL_DEBUG);
  return false;
}

// returns product id for given media
function media_product_id($media_id, $options = array()) {
  $product_id = find_object('product', array('source_type' => 'media', 'fid' => $media_id), 'id');
  return $product_id;
}

function gallery_cart_button($siteObj, $mediaObj_or_id, $options = array()) {
  global $u_id;
  
  $disable_cart_buttons = $options['disable_cart_buttons']; 

  $cart_button = $resolution = '';
  $user_owns_image = false;
  $mediaObj = any2obj('media', $mediaObj_or_id);
  $cartObj = get_cart($siteObj);
  $cart_items = $cartObj->items ?: array();
  
  $cartData['site_id'] = $siteObj->id;
  $cartData['user_id'] = $u_id ?: 0;
  $debug = '';
  //if(!$_SESSION['live']) $debug .= dump($cartObj, true);

  $product_id = media_product_id($mediaObj->id);
  
  //if($cart_items[$product_id]) return "(Already in your cart)";
  
  if($productVariantId = gallery_user_owns_image($siteObj, $u_id, $mediaObj)) { // logged in user owns this, show download button
    $variantObj = new stdClass;
    $user_owns_image = true;
    list($product_id, $variant_id) = explode('-', $productVariantId);
    $cartData['parent_type'] = 'media';
    $cartData['parent_id'] = $mediaObj->id;
    $cartData['product_id'] = $product_id;
    $cartData['variant_id'] = $variant_id;

    $download_title = phrase('download', CAPITALIZE);
    $variantObj = $variant_id ? get_object('product_variant', $variant_id) : NULL;
    
    $resolution = $variant_id ? get_object('product_variant', $variant_id, 'option_value') : '';
    //if(!$_SESSION['live']) $debug = "name: $variantObj->name res=$variantObj->option_value id=$variantObj->id<br>"; 
    switch($resolution) {
    case 'low':
      $cartData['size'] = 'small';
      break;
    case 'medium':
      $cartData['size'] = 'medium';
      break;
    case 'high':
      $cartData['size'] = 'full';
      break;
    default:
      $cartData = array();
      break;
    }
    if($cartData) {
      $variant_value = $resolution."_resolution";
      $download_class = $disable_cart_buttons ? "" : "download_media"; 

      $cart_button = html_div(html_link('#', html_i("fa fa-download")." ".phrase('download', CAPITALIZE).' '.phrase($variant_value), array('class' => "btn btn-success $download_class", 'data-data' => json_encode($cartData))), 'margin_bottom');
      $cart_button .= file_downloader();
    }
    if($resolution == 'high') return $cart_button;
  } 
  
  $cart_button = '';
  if(0) { /** cart buttons disabled for now */
    // only get here if user doesn't own high-res image aleady 
    if($productObj = find_object('product', array('source_type' => 'media', 'fid' => $mediaObj->id))) { // product, but not owned by this user
      $cur = get_currency_symbol($productObj->currency, true); 
      //dump($productObj);
      $product_id = $productObj->id;
      if($variantObjs = find_objects('product_variant', array('product_id' => $product_id, 'active' => 1, 'option_value' => array('!=', $resolution)))) {
        $variant_count = count($variantObjs);
        $cartData['product_id'] = $product_id;
        foreach($variantObjs as $variantObj) {          
          $resolution = $variantObj->option_name == 'resolution' ? $variantObj->option_value : '';      
          if($resolution != 'medium' && !($variantObj->price > 0)) {
            // $cart_button .= html_div("High Resolution Not Available", 'inline-block margin-right');
          } else {          
            $cartData['variant_id'] = $variantObj->id;
            $add_class = $disable_cart_buttons ? "" : "add_cart";
            
            //$option_name = get_object('product_option', $variantObj->option_id, 'name');
            $option_name = $variantObj->option_name;
            $button_text = html_i('', "fa fa-cart-plus")." ".phrase('add_to_cart', CAPITALIZE);          
            $button_text = phrase($variantObj->option_value, CAPITALIZE)." $option_name: $cur $variantObj->price";          
            $cart_button .= html_div(html_link('#', $button_text, array('class' => "btn btn-success cart $add_class", 'data-data' => json_encode($cartData))), 'inline-block margin-right');
          }
        }
      } else {      
        $cartData['parent_type'] = 'media';
        $cartData['parent_id'] = $mediaObj->id;
        $cart_button = html_div(html_link('#', $button_text, array('class' => "btn btn-success cart $add_class", 'data-data' => json_encode($cartData))), 'inline-block');
      }
    }
  }
  
  return $cart_button.$debug;
}


//function gallery_user_collection($siteObj_or_id, $userObj_or_id, $mediaObj_or_id) {



function gallery_user_collection($siteObj_or_id, $userObj_or_id, $options='') {
  $siteObj = any2obj('site', $siteObj_or_id);
  $site_id = $siteObj->id;
  $userObj = any2obj('user', $userObj_or_id);
  $user_id = $userObj->id;

  //$options['size'] = $options['size'] ?: 'square';
  //$options['role'] = USER_ROLE_ARTIST;
  //$options['collection_type'] = MEDIA_COLLECTION_TYPE_PROJECT;
  $errors = $messages = array();

  $collectionObj = gallery_find_user_collection($siteObj, $userObj);
  $collection_id = $collectionObj->id;     
  if(!$collectionObj) {  // user has no collection
    //$collection_product_ids = array();
    //$messages[] = "Could not find My collection<br>";
    return '';
  }
  
  // return assoc array of product-variant ids
  if($options['return'] == 'products') {
    $ar = array();
    $product_ids = find_objects('media_collection_map', array('media_collection_id' => $collection_id), array('fields' => "distinct product_id"));
    foreach($product_ids as $product_id) {
      if(!$product_id) continue;
      $ar[$product_id] = get_object('product', $product_id);  
    }
    //echo("map objects in collection $collection_id: ".dump($pv_ids, true));
    return $ar;
  }

  if($options['return'] == 'variants') {
    $ar = array();
    //echo("col_id=$collection_id<br>");
    $pv_ids = find_objects('media_collection_map', array('media_collection_id' => $collection_id), array('fields' => "concat(product_id,'-',variant_id) as pv_id"));
    foreach($pv_ids as $pv_id) {
      list($product_id, $variant_id) = explode('-', $pv_id);
      if(!$product_id || !$variant_id) continue;
      $ar[$product_id] = get_object('product_variant', $variant_id);  
    }
    //echo("map objects in collection $collection_id: ".dump($pv_ids, true));
    return $ar;
  }
  //$output .= gallery_object_list();
  if($errors) $output .= print_errors($errors);
  if($messages) $output .= print_messages($messages);
  $output = html_h2($collectionObj->title);
  if($collection_id) {
    $output .= gallery_collection($siteObj, $collection_id, $options);
  } else {
    $errors[] = "No collection found";
  }

  return $output;
}

// removes images from cart or order that this user already owns 
function gallery_remove_owned_items($cart_or_orderObj, $pvArray) {
  global $u_id;
  $messages = array();
  $id = $cart_or_orderObj->id;
  $type = isset($cart_or_orderObj->order_id) ? 'cart' : 'payment_order';
  if(!$id || !$u_id || !$pvArray || !$cart_or_orderObj) return array($cart_or_orderObj, $messages);

  //dump($pvArray);
  //dump($cart_or_orderObj);
  $items = $cart_or_orderObj->items ?: array();
  $debug = "type=$type id=$id u_id=$u_id<br>gallery_remove_owned_items before: count=$cart_or_orderObj->count total=$cart_or_orderObj->total<br>";//.dump($items, true));
  $delete_ids = array();
  foreach($items as $item) {
    $product_id = ($item->product_id ?: 0);
    $variant_id = ($item->variant_id ?: 0);
    if(!$product_id) continue;
    if($varObj = $pvArray[$product_id]) {
      //dump($varObj);
      $debug .= "You own product $product_id variant $varObj->id basket: $variant_id<br>";
      // varObj = is the variant the user already owns
      if($varObj->id == $variant_id) {
        $messages[] = "$item->name was removed as you already own this.";
        $delete_ids[] = $item->id;
      } elseif($varObj->option_value == 'high') {
        $messages[] = "$item->name was removed as you already own the high resolution.";
        $delete_ids[] = $item->id;
      } else {
        $messages[] = "Upgrading $item->name to high resolution.";
      }
    }
  }
  if($delete_ids) {
    $sub_type = $type == 'cart' ? 'cart_row' : 'order_row';
    //foreach($delete_ids as $del_id) delete_object($sub_type, $del_id);     
    $debug .= "gallery_remove_owned_items deleting $sub_type ".implode(',', $delete_ids)."<br>";
    $cart_or_orderObj = get_object($type, $id); // reload (class_handlers handle the rest
    $debug .= "reloaded $type $id".dump($cart_or_orderObj, true);
  }
  $debug .= dump($messages, true);
  $debug .= "gallery_remove_owned_items after: count=$cart_or_orderObj->count total=$cart_or_orderObj->total<br>";//.dump($items, true));
  if(!$_SESSION['live']) echo($debug);
  return array($cart_or_orderObj, $messages);
}


function add_paid_images_to_collection($userObj_or_id, $collectionObj_or_id=NULL, $collection_product_ids = array()) {
  $userObj = any2obj('user', $userObj_or_id);
  $user_id = $userObj->id;
  if(!$user_id) return;
  $debug = !$_SESSION['live'];
  
  if($collectionObj_or_id) {
    $collectionObj = any2obj('media_collection', $collectionObj_or_id);
    $collection_id = $collectionObj->id;     
  } elseif($collectionObj = gallery_find_user_collection($siteObj, $userObj)) {
    $collection_id = $collectionObj->id;     
  } else { // add user collection
    $collection_id = gallery_add_user_collection($siteObj, $userObj);
  }
  if(!$collection_id) return;

  // find product ids already in user collection  
  if(!$collection_product_ids) $collection_product_ids = array_filter(find_objects('media_collection_map', array('media_collection_id' => $collection_id), array('fields' => "concat(product_id,'-',variant_id)"))) ?: array();  
  $add_product_ids = array();
  if($debug) $messages[] = "In collection: ".implode(',', $collection_product_ids);
  // check for paid work not yet added to collection
  if($paid_inv_ids = find_objects('invoice', array('user_id' => $user_id, 'status' => INV_STATUS_COMPLETE), array('fields' => 'invoice.id'))) { // paid invoices
    if($paid_product_ids = find_objects('inv_lineitem', array('inv_id' => $paid_inv_ids), array('fields' => "concat(product_id,'-',variant_id) as pv_id"))) { // paid products
      if($debug) $messages[] = "Paid: ".implode(',', $paid_product_ids);
      //$messages[] = "My product count=: ".count($paid_product_ids);
      $add_product_ids = array_diff($paid_product_ids, $collection_product_ids);
      if($debug) $messages[] = "Add: ".implode(',', $add_product_ids). " to collection $collection_id";
      if($collection_id && $add_product_ids) {
        $messages[] = "My collection id=$collection_id, adding ".count($add_product_ids)." images";
        foreach($add_product_ids as $pv_id) { // add images to collection
          list($product_id, $variant_id) = explode('-', $pv_id);
          $messages[] = "Adding $name to My collection id=$id product_id=$product_id variant_id=$variant_id";
          $productObj = get_object('product', $product_id);          
          $variantObj = $variant_id ? get_object('product_variant', $variant_id) : NULL;
          if($productObj->source_type != 'media') continue; // not an images
          $name = get_variant_name($variantObj, $productObj);
          $media_id = $productObj->fid;          
          $mediaObj = $media_id ? get_object('media', $media_id) : null;
          $media_name = $mediaObj->name;
          $mapData['user_id'] = $user_id;
          $mapData['site_id'] = $site_id;
          $mapData['media_collection_id'] = $collection_id;
          $mapData['collection_type'] = $collectionObj->type;
          
          $mapData['media_id'] = $media_id;
          $mapData['product_id'] = $productObj->id;
          $mapData['variant_id'] = $variant_id;
          $mapData['artist_id'] = $artist_id = $mediaObj->artist_id ?: $mediaObj->user_id;
          $mapData['artist_name'] = user_display_name($mediaObj->artist_id);
          $mapData['original_technique'] = $mediaObj->original_technique;
          $mapData['original_year'] = $mediaObj->original_year;
          $mapData['name'] = $mediaObj->name;
          
          $mapSearch = array('media_collection_id' =>$collection_id, 'media_id' => $media_id);
          if($mapObj = find_object('media_collection_map', $mapSearch)) {
            $v_id = $mapObj->variant_id;
            $vObj = $v_id ? get_object('product_variant', $v_id) : null;
            $messages[] = "$media_name is already in collection with product_id=$mapObj->product_id variant_id=$v_id res=$vObj->option_value";
            $upgrade = $variant_id != $v_id && $variantObj->option_value == 'high';
            if($upgrade) {
              $messages[] = "Upgrading $media_name to high";
              list($map_id, $errors) = update_object('media_collection_map', $mapData, $mapObj->id);
              if($errors) {
                $messages[] = "Upgrading failed errors=".dump($errors, true);
              } else {
                $messages[] = "Upgrade ok: id=$map_id";
              }
            } else {
              $messages[] = "Not an upgrade (res=$vObj->option_value, existing=$variantObj->option_value), ignoring $media_name";
            }
          } else {
            list($id, $errors) = add_object('media_collection_map', $mapData);
            if($id) {
              $messages[] = "Added $name to My collection id=$id";
            } else {
              $messages[] = "Failed to add $name to My collection id=$id errors=".dump($errors, true);
              //dump($errors);
            }
          }
        }
      }      
    }    
  }
  $debug_str = $debug ? implode(PHP_EOL, $messages).PHP_EOL : '';
  print_log($debug_str, 'gallery', LOG_LEVEL_DEBUG);
  return array_merge($collection_product_ids, $add_product_ids); // all product ids owned by this user, including those that were already in collection
}
  
function site_gallery_profile($siteObj, $galleryObj_or_id, $options=NULL) {
  $galleryObj = any2obj('gallery', $galleryObj_or_id);
  $imageObj = gallery_title_image($siteObj, array('obj_type' => 'gallery', 'id' => $galleryObj->id), array('object' => $galleryObj));
  $pic = html_image($imageObj->small_src, $imageObj->title);
  $title = $galleryObj->title ?: phrase('no_title', CAPITALIZE);
  $text = $galleryObj->description;
  return html_div(html_div($pic, 'profile_pic').html_h3($title, 'pad profile_name').html_div($text, 'profile_text').html_div('', 'profile_footer'), $options['class']);  
}

function site_collection_profile($siteObj, $collectionObj_or_id, $options=NULL) {
  $collectionObj = any2obj('media_collection', $collectionObj_or_id);
  $imageObj = gallery_title_image($siteObj, array('obj_type' => 'media_collection', 'id' => $collectionObj->id), array('object' => $collectionObj));
  $pic = html_image($imageObj->small_src, $imageObj->title);
  $title = $collectionObj->title ?: phrase('no_title', CAPITALIZE);
  $text = $collectionObj->description;
  return html_div(html_div($pic, 'profile_pic').html_h3($title, 'pad profile_name').html_div($text, 'profile_text').html_div('', 'profile_footer'), $options['class']);  
}

// print the extra information for artist/project/image
function gallery_info($siteObj, $search, $options=NULL) {  
  extract($search);
  $options['size'] = 'small';
  $imageObj = gallery_title_image($siteObj, $search, $options);
  $title_pic = $imageObj->photo;
  $gallery_info = gallery_title_text($siteObj, $search, $options);
  return html_div(html_div($title_pic, 'float_left margin').html_div($gallery_info, 'pad').html_div('', 'profile_footer float_clear'));
}

// returns the media objects belonging to a gallery
// inactive images included if auth edit is true
function gallery_collection_maps($siteObj, $collectionObj_or_id, $options=NULL) {
  $collection_id = is_object($collectionObj_or_id) ? $collectionObj_or_id->id : $collectionObj_or_id;
  $auth_edit = $options['auth_edit'];
  $mapSearch['media_collection_id'] = $collection_id;
  if(!$auth_edit) $mapSearch['active'] = 1;
  $mapObjs = find_objects('media_collection_map', $mapSearch);
  return $mapObjs;  
}

// returns all images belonging to user
function gallery_user_images($siteObj_or_id, $userObj_or_id, $options=NULL) {
  $user_id = is_object($userObj_or_id) ? $userObj_or_id->id : $userObj_or_id;
  $siteObj = any2obj('site', $siteObj_or_id);
}

// returns all folders (galleries) for this user; top level by default
function gallery_user_folder($siteObj_or_id, $userObj_or_id, $options=NULL) {
  $user_id = is_object($userObj_or_id) ? $userObj_or_id->id : $userObj_or_id;
  $site_id = is_object($siteObj_or_id) ? $siteObj_or_id->id : $siteObj_or_id;
  $search['site_id'] = $site_id;    
  $search['user_id'] = $user_id;    
  $search['parent_id'] = $options['parent_id'] ?: 0;    
  $search['active'] = 1;
  return find_objects('gallery', $search);
}

function gallery_folder_images($siteObj_or_id, $galleryObj_or_id, $options=array()) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $gallery_id = is_object($galleryObj_or_id) ? $galleryObj_or_id->id : $galleryObj_or_id;
  $search['parent_type'] = 'gallery';
  $search['parent_id'] = $gallery_id;    
  $search['active'] = 1;
  return find_objects('media', $search);
}

function gallery_collection_images($siteObj_or_id, $collectionObj_or_id=NULL, $options=array()) {
  $mapObjs = gallery_collection($site_id, $collectionObj_or_id, array('return' => 'object'));
  $mapArray = array();
  foreach($mapObjs as $mapObj) $mapArray[$mapObj->media_id] = $mapObj->id;
  $ids = array_keys($mapArray);
  $search['id'] = $ids;
  $imageObjs = find_objects('media', $search);
  foreach($imageObjs as $imageObj) $imageObj->map_id = $mapArray[$imageObj->id]; // store the map_id with the media
  return $imageObjs;
}

// show a collection of images
function gallery_collection($siteObj_or_id, $collectionObj_or_id=NULL, $options=array()) {  
  $siteObj = any2obj('site', $siteObj_or_id);
  if($collectionObj_or_id) $collectionObj = any2object('media_collection', $collectionObj_or_id);
  if(!$collectionObj) return "Collection not found";
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php

  $userObj = get_object('user', $collectionObj->user_id);
  $display_name = user_display_name($userObj);
  
  $mapObjs = gallery_collection_maps($siteObj, $collectionObj, $options);
  if($options['return'] == 'object') return $mapObjs;
  
  //$gallery_options = user_gallery_options($siteObj, $collectionObj, $userObj, $options);
  $options['objects'] = $mapObjs;
  $options['parent_obj_type'] = $options['obj_type'];
  $options['parent_obj_subtype'] = $options['obj_subtype'];
  $options['parent_obj'] = $collectionObj;
  $options['base_link'] = gallery_collection_link($siteObj, $collectionObj);  
  $options['obj_type'] = 'media_collection_map';

  if($options['view'] == 'simple') return gallery_simple_list($siteObj, $options);

  return gallery_object_list($siteObj, $options);
}

function gallery_view_menu($siteObj, $options=NULL) {
  //dump($options);
  
  $viewer    = pick_first($_GET['viewer'], $options['viewer'], $_SESSION['gallery_viewer'], 'html');
  $format    = pick_first($_GET['format'], $options['format'], $_SESSION['gallery_format'], 'list');

  //unset($_SESSION['gallery_format']);
  //unset($_SESSION['gallery_viewer']);

  //dump($options);
  //echo("gvm: viewer=$viewer format=$format<br>");
  
  $list_class = 'inline';
  $link_class = 'blue';
  $list_item_class = 'inline';
  $active_class = 'active black bold';
  $link_list_options = array('class' => $list_class, 'item_class' => $list_item_class, 'active_class' => $active_class, 'link_class' => $link_class, 'clean' => true);
  
  //dump($options);
  $viewer_links = array('html' => 'HTML', 'fancybox' => 'Fancybox', 'galleria' => 'Galleria');
  $output .= html_div(html_b(phrase('viewer', CAPITALIZE).': ').html_link_list($viewer_links, 'viewer', $viewer, $link_list_options), 'inline');

  if($viewer == 'html') {
    $format_links = $viewer == 'html' ? 
      array('compact' => phrase('compact', CAPITALIZE), 'list'  => phrase('list', CAPITALIZE), 'icons' =>  phrase('icons', CAPITALIZE)) : 
      array('image' => 'HTML', 'json' => 'JSON');
    $output .= html_div(html_b(phrase('view', CAPITALIZE).': ').html_link_list($format_links, 'format', $format, $link_list_options), 'inline');
  }

  if($theme_links) $output .= html_div(html_b(phrase('theme', CAPITALIZE).': ').html_link_list($theme_links, 'theme', $sel_theme, $link_list_options));

  return $output;
}

// the simple list is used in the My vPatina view
function gallery_simple_list($siteObj, $options='') {
  global $__header;
  $galleria_version = '1.4.2'; // 1.2.5
  $__header['scripts'][] = "/afr/bower/galleria/src/galleria.js";    
  $__header['onload'] .= "fancybox_loader();\n";
  //dump($options);
  $obj_type = $options['obj_type'];
  $obj_subtype = $options['obj_subtype'];
  $objects = $options['objects'];
  $dataAr = array();
  $target_id = $options['target_id']; // id of tabs
  $count = count($objects);
  $nav_src = $options['store_result'];
  $i = 0;
  foreach($objects as $obj) {
    //dump($obj);
    $id = $obj->id;
    $imageObj = gallery_title_image($siteObj, array('obj_type' => $obj_type, 'id' => $id), array('object' => $obj));
    $medium_src = $imageObj->medium_src;
    $thumb_src = $imageObj->square_src;
    $big_src = $imageObj->big_src;

    // items for screen + fancybox
    $link_data = array('site_id' => $siteObj->id, 'obj_type' => $obj_type, 'id' => $id, 'count' => $count, 'index' => $i);
    if($nav_src) {
      $link_data['nav_source'] = $nav_src;
    }
    $info_icon = html_link('#', html_div('', 'mini-sprite-16 s212'), array('title' => phrase('image_information', CAPITALIZE), 'class' => 'info-link new-tab', 'data-data' => json_encode($link_data)));
    $full_icon = html_link('#', html_div('', 'mini-sprite-16 s2'), array('title' => phrase('full_screen', CAPITALIZE), 'class' => 'full-link', 'data-data' => json_encode($link_data)));
    $title = html_div($info_icon.$full_icon.$imageObj->title, 'image-title relative');
    $image = html_image($thumb_src, $imageObj->title, array('class' => 'relative', 'data-info-link' => json_encode($link_data), 'data-info-target' => $target_id));
    $items[] = html_link($medium_src, $image, array('class' => 'fancybox', 'rel' => 'group')).$title;

    // data for galleria
    $data = new stdClass();
    $data->image = $medium_src;
    $data->thumb = $thumb_src;
    //$data->big = $big_src;
    $data->title = $imageObj->title;
    $data->description = $imageObj->short_description;
    $dataAr[]= $data;
    $i++;
  }
  $data_json = json_encode($dataAr);

  $close_me = html_div('', 'close_parent abs_top_right_outside mini-sprite-32 s211');
  $output = html_div($close_me.html_div('', 'gallery_cont'), 'gallery_cont_wrapper relative');
  $output .= html_ul($items, 'simple-list').html_div('', 'clear_float');
  $output .= gallery_store_result($siteObj->id, 'user_collection', $objects, $obj_type, $obj_subtype);  

  $galleria_script = "
    Galleria.loadTheme('/afr/bower/galleria/themes/azur/galleria.azur.min.js');
    $('.full-link').click(function() {
      $('.gallery_cont_wrapper').show();
      Galleria.run('.gallery_cont', {
        dataSource: data,
      });
    });
  ";         
    //$('.gallery_cont').data('galleria').enterFullscreen();
  $output .= html_script_inline("var data=$data_json;".$galleria_script); // for galleria
  return $output;
}

function gallery_store_result($site_id, $result_id, $objects, $obj_type, $obj_subtype=NULL) {    
  $data['site_id'] = $siteObj->id;
  $data['obj_type'] = $obj_type;
  if($obj_subtype) $data['obj_subtype'] = $obj_subtype;
  $data_key = $obj_type == 'user' ? 'user_id' : get_object_key($obj_type);
  $data['id_list'] = $id_list = array_find_key($objects, $data_key);
  $data_json = json_encode($data);
  $output = html_div('', 'hidden', array("id" => $result_id, "data-result" => $data_json));
  //$output .= "list for $result_id=".implode(',', $id_list)."<br>";
  return $output;
  //$output .= dump($objects, true);
}

// unified function for displaying HTML of users, collections, or images
// replaces (for HTML) site_gallery, gallery_user_list, site_gallery_list, gallery_collection_list
function gallery_object_list($siteObj, $options='') {
  global $__header;
  $obj_type  = $options['obj_type'];  
  $search    = $options['search'];
  $base_link = $options['base_link'];
  $auth_edit = $options['auth_edit'];
  $no_list   = $options['no_list'];  
  $obj_subtype = $options['obj_subtype'];
  $link_only = $options['link_only'];
  $object_only = $options['object_only'];
  $show_link  = isset($options['show_link']) ? $options['show_link'] : true;  
  
  $viewer    = pick_first($_GET['viewer'], $options['viewer'], $_SESSION['gallery_viewer'], 'html');
  $format    = pick_first($_GET['format'], $options['format'], $_SESSION['gallery_format'], 'list');
  if($link_only || $object_only) {
    $format = 'json';
    // echo("format = $format");    
    // dump($options);
  }
  
  //dump($options);
  if(in_array($viewer, array('galleria', 'fancybox'))) {
    //return "gh: viewer=$viewer".dump($options, true);
    return site_gallery($siteObj, $options);
  } 
  if(!in_array($format, array('icons', 'compact', 'list', 'json', 'image'))) $format = $options['format'] = 'list';

  if($viewer== 'html' && in_array($format, array('icons', 'compact', 'list')) && !$options['no_session']) { // don't mess with session variables (format/viewer) when called from site_gallery
    $_SESSION['gallery_format'] = $format;
    $_SESSION['gallery_viewer'] = $viewer;
  }

  //echo("<br>gol: viewer=$viewer format=$format<br>");
  
  // the context of this list
  if($parent_obj_type = $options['parent_obj_type'] ?: $search['parent_type']) {
    $parent_id = $options['parent_id'] ?: $options['id'];
    $parent_obj_subtype = $options['parent_obj_subtype'];
    $parentObj = $options['parent_obj'];
    if(!$parentObj && $parent_id) $parentObj = get_object($parent_obj_type, $parent_id);
  }
  
  if(!$obj_type) return "gallery_object_list: no object type";
  if(isset($options['objects'])) { // list of objects
    $objects = $options['objects'];
    //echo("There are ".count($objects)." $obj_type".html_break());
  } elseif($search) {
    //dump($search);
    $objects = find_objects($obj_type, $search, $options);
    //dump($objects);
  } else {
    $errors[] =  "gallery_object_list: no objects or search";
  }

  $result_id = $options['store_result'];
  if($obj_type && $objects && $result_id) { // store in data tag for retrieval later (used for paging in explorer)
    $output .= gallery_store_result($site_id, $result_id, $objects, $obj_type, $obj_subtype);
  }
  
  /*
  $rating = feedback_stars($imageObj->title, $imageObj->rating ?: 10, $imageObj->rating_count ?: 10, array('stars_only' => true));
  $rating = '';
  $rating = feedback_stars($imageObj->title, $imageObj->rating ?: 10, $imageObj->rating_count ?: 10, array('stars_only' => true));
  $rating = '';
  $rating = feedback_stars($imageObj->title, $imageObj->rating ?: 10, $imageObj->rating_count ?: 10);
  $rating = '';
  */
  //die("gh ot=$obj_type".dump($options, true));

  switch($obj_type) {
  case 'user':
    $role = $options['role'];
    if(!$role) $errors[] = "gallery_object_list: no user role";
    $count_type = $role == USER_ROLE_ARTIST ? 'project' : 'exhibition';

    //dump($imageObjSearch);
    $pageSearch = array('object_type' => 'user', 'object_subtype' => $role);
    $edit_fields = '"first_name", "last_name", "display_name", "city", "country", "photo"';
    $edit_obj_type = 'user_profile';
    break;
  case 'gallery':
    $pageSearch = array('object_type' => 'gallery');
    $count_type = 'image';
    $edit_fields = '"title", "description", "image", "media"';
    if($auth_edit && !$options['trash'] && $parent_obj_type == 'user' && $parentObj) {
      $artist_id = $parentObj->id;
      $handler_options = array('handler' => 'new_media_handler', 'handler_operation' => 'new-gallery-folder', 'target' => 'ajax_messages');
      $buttons[] = html_link("#", html_i('', "fa fa-folder")." ".phrase('new', CAPITALIZE).' '.phrase('folder'), array('data-options' => json_encode($handler_options), 'class' => 'btn btn-success add_object', 'data-form_class' => 'fieldset_left_wide', 'data-obj_type' => "gallery", 'data-fields' => '["site_id", "user_id", "type", "title", "description", "title_image"]', 'data-data' => '{"site_id": "'.$siteObj->id.'", "user_id": "'.$artist_id.'"}'));
    }
    break;
  case 'media_collection':
    $pageSearch = array('object_type' => 'media_collection', 'object_subtype' => $obj->type ?: $options['type']);
    $count_type = 'image';
    if($auth_edit && $obj_subtype) {
      $collection_type = $obj_subtype;
      $collection_type_name = get_object('media_collection_type', $obj_subtype, 'name');

      $artist_id = $parentObj->id;
      $art_link = html_link(gallery_vault_link($siteObj,  $parentObj->id), "your vault");
      $help_text = "To add images to your $collection_type_name, go to $art_link, add a folder with some images, and publish it to a project for others to see.";

      $handler_options = array('handler' => 'new_media_handler', 'handler_operation' => 'new-media-collection', 'target' => 'ajax_messages', 'dialog_title' => phrase('new', CAPITALIZE).' '.$collection_type_name, 'obj_type' => 'media_collection');
      $buttons[] = html_link("#", html_i('', "fontello icon-picture")." ".phrase('new', CAPITALIZE).' '.phrase($collection_type_name), array('data-options' => json_encode($handler_options), 'class' => 'btn btn-success margin_bottom add_object', 'data-form_class' => 'fieldset_left_wide', 'data-obj_type' => "media_collection", 'data-fields' => '["site_id", "user_id", "type", "title", "description", "title_image"]', 'data-data' => '{"site_id": "'.$siteObj->id.'", "type": "'.$collection_type.'", "user_id": "'.$artist_id.'"}'));
    }
    
    break;
  case 'media_collection_map':
    $pageSearch = array('object_type' => 'media_collection', 'object_subtype' => $obj->type ?: $options['type']);
    $count_type = 'image';    
    switch($parent_obj_type) {
      case 'media_collection':
        if($parent_obj_subtype) {
          $collection_type = get_object('media_collection_type', $parent_obj_subtype, 'name');
          switch($parent_obj_subtype) {
            case MEDIA_COLLECTION_TYPE_PROJECT: 
              $art_link = html_link(gallery_vault_link($siteObj,  $parentObj->user_id), "browse your vault");
              break;
            default:
              $art_link = html_link(gallery_artist_link($siteObj), "browse the artists");
              break;
          }
          $help_text = "To add images to your $collection_type, $art_link, check the checkbox next to images you want to add, then use the menu on top and choose 'Add to'";
        }
        break;
      default:
        break;      
    }
    break;
  case 'media':
    $pageSearch = array('object_type' => 'media');
    $count_type = '';
    $edit_fields = '"name", "short_description", "comment", "user_filename"';    
    break;
  default:                
    $errors[] = "gallery_object_list: unsuported object type $obj_type";    
  }

  // add quick upload button
  //echo("auth=$auth_edit pot=$parent_obj_type pid=$parent_id<br>");
  if($auth_edit && $parent_obj_type == 'gallery' && $parent_id && !$options['trash']) {
    $galleryObj = $parentObj;
    $gallery_id = $parent_id;
    
    $class_attrs = get_object_attributes('gallery', false);
    $upload_options = $class_attrs['media'][ATT_LENGTH];
    
    $fld = 'media';
    $val = $galleryObj->$fld;
    
    $mediaData['user_id'] = $galleryObj->user_id; // means that images are stored in separate directory per user
    $mediaData['parent_id'] = $gallery_id;
    $mediaData['parent_type'] = 'gallery';
    $mediaData['parent_field'] = $fld;

    $upload_options['media_data'] = $mediaData;
    $upload_options['data-value_list'] = $val;
    $upload_options['show_file_list'] = false;
    $upload_options['title'] = "Click or drag to upload media";
    $upload_options['handler'] = 'new_media_handler';
    $upload_options['handler_options'] = $options;

    //dump($upload_options);
    $uploader = jquery_file_uploader("gallery-$gallery_id-$fld", $val, $upload_options);
  }
    
  if(!$objects) {
    if($format == 'json' || $object_only || $link_only) return array();
    $output .= $title.html_h3('['.phrase('empty', CAPITALIZE).']', 'content_subtitle');
    if($auth_edit && $help_text) $output .= $help_text;
    //$output .= $uploader;
    //return $output;
  }
        
  $base_link = $base_link ?: $options['base_link'] ?: site_page_link($siteObj, PAGE_GALLERY, array('search' => $pageSearch)); // e.g. '/art/project/'

  if($errors) {
    echo(implode(html_break(), $errors));
    dump($options);
    return;
    
  }

  $html = true;
  switch($format) {
    case 'json':
      $html = false;
      break;
    case 'icons':
      $container_class = 'icon_list';
      break;
    case 'compact':
      $container_class = 'compact_list box-drop-shadow';
      break;
    case 'list':
      $item_class = 'border-bottom-light relative';
      $container_class = 'gallery_list box-drop-shadow';
      break;
    default:
      break;
  }      
  if($auth_edit) {
    $container_class .= ' sortable';
    $containerOptions['data-obj_type'] = $obj_type == 'user' ? 'user_profile' : $obj_type;
  }
  
  $jsonObjs = array();
  $artistJson = NULL;
  $profileObjs = array();
  $items = array();
  
  //dump($options);
  
  $i=0;
  $options['count'] = count($objects);
  $options['edit_fields'] = $edit_fields;
  $link = array();
  //dump($options);
  foreach($objects as $obj) {
    $id = $obj->id;
    $item_id = $id;
    $prefix = 'media-item-';

    $options['object'] = $obj;
    $options['index'] = $i;
    
    // parent info is used by navigation (back button) in fancybox/galleria
    if($obj_type == 'media_collection_map') {
      $parent['obj_type'] = 'media_collection';
      $parent['id'] = $obj->media_collection_id;
    } else {
      $parent['obj_type'] = $obj_type;
      $parent['id'] = $id;
    }
    
    $options['parent'] = $parent;
    $imageObj_search_field = $obj_type == 'user' ? 'user_id' : 'id'; // get rid of this hack
    $iid = $obj->$imageObj_search_field;

    // $titleSearch = $obj_type == 'media_collection_map' ? array('obj_type' => 'media', 'id' => $obj->media_id) : array('obj_type' => $obj_type, 'id' => $iid); 
    $titleSearch = array('obj_type' => $obj_type, 'id' => $iid); 
    
    //dump($titleSearch);dump($options);
    //echo("gti: base_link = $options[base_link]<br>");    
    $imageObj = $options['imageObj'] = gallery_title_image($siteObj, $titleSearch, $options); 
    //dump($imageObj);echo("format=$format");die();

    //$links[] = $imageObj->link;
    if(!$show_link) unset($imageObj->link);
    if($format == 'json') $jsonObjs[] = $imageObj;
    else $items[$item_id] = gallery_object_list_item($siteObj, $obj_type, $iid, $options);
    
    $i++;
  }
  //dump($links);
  
  if($jsonObjs) {
    //dump($jsonObjs);
    //mydie("returning ".count($jsonObjs)." json objects");
    return $jsonObjs;
  }
  
  // begin output
  $output .= $title;
  
  // $edit_button = $auth_edit ? html_div(inline_edit_link_old('media', 'gallery', 0), 'ie_edit_link') : '';
  // if($edit_button) $output = html_div($edit_button.$output, 'relative');

  switch($format) {
    case 'json':
      if($jsonObjs) $script = "var data=".json_encode($jsonObjs)."\n";
      if($artistJson) $script .= "var artists=".json_encode($artistJson)."\n";
        $output .= html_div('', $container_class, array('id' => $container_id, 'width' => $w, 'height' => $h)); // place holder for viewer
      break;

    default: // html, list
      if($container_id) $containerOptions['id'] = $container_id;
      if(!$no_list) {
        $containerOptions['item_class'] = $item_class;
        $containerOptions['item_prefix'] = $prefix;
      }
      if(count($buttons)) $output .= html_div(implode("\n", $buttons), 'margin_bottom');      
      //echo("cc=$container_class");
      $output .= $no_list ? html_div(implode("\n", $items), $container_class, $containerOptions) : html_ul($items, $options['list_class'] ?: $container_class, $containerOptions);
      $output .= $uploader;
      //$output .= html_ul($items, $container_class, array('id' => $container_id, 'item_class' => $item_class));
      //$output .= html_div('', 'clear_float');
      break;
  }

  //$debug = "format=$format auth=$auth_edit uploader=$uploader pot=$parent_obj_type pid=$parent_id";
  $debug = '';
  return $debug.$output;
  
  return $output;
}

function gallery_object_list_item($siteObj_or_id, $obj_type, $id, $options=NULL) {
  global $u_id, $u_level;
  if(is_object($siteObj_or_id)) $siteObj = $siteObj_or_id;
  else $siteObj = $_SESSION['siteObj'] ?: any2obj('site', $siteObj_or_id);
  $site_id = $siteObj->id;
  
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php
  //$debug = dump($options, true);
  $options = (array) $options; // from app_data.php  
  $search = (array) $options['search'];
  $imageObj = $options['imageObj'];
  $auth_edit = $options['auth_edit'];
  $obj = $options['object'] ?: get_object($obj_type, $id);
  $parent_obj_type = $options['parent_obj_type'] ?: $search['parent_type'];
  $edit_fields = $options['edit_fields'];
  $size     = $options['size'] ?: 'thumb';
  
  $link_class = trim($options['link_class']." cms-link"); 
  $link_target = $options['link_target']; 
  
  if(!$imageObj) {
    $titleSearch = $obj_type == 'media_collection_map' ? array('obj_type' => 'media', 'id' => $obj->media_id) : array('obj_type' => $obj_type, 'id' => $id); 
    if($obj_type == 'media_collection_map') unset($options['object']);
    $imageObj = gallery_title_image($siteObj, $titleSearch, $options);
  }
    
  if($auth_edit) {

    // checkbox         
    //$checkbox = form_checkbox_simple('media_'.$obj->id, false, array('class' => 'gshow')); 
    
    // slider
    if($parent_obj_type != 'gallery' && $obj_type != 'gallery') { // we use recycle bin then
      $slider_data['obj_type'] = $obj_type;
      $slider_data['id'] = $id;
      $slider_data_json = json_encode($slider_data);
      $slider = on_off_slider('media-switch-'.$obj->id, $obj->active, array('handler' => 'edit', 'target_field' => 'active', 'data' => $slider_data_json, 'disabled' => is_file($_SERVER['DOCUMENT_ROOT'].$imageObj->path.$imageObj->filename)));
    }            
    // edit button
    $edit_obj_type = $edit_obj_type ?: $obj_type; 
    $edit_obj_id = $id;
    $load_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=$edit_obj_type&id=$edit_obj_id&edit=1"; //&obj_type=media&id=$obj->id";
    $edit_button = html_link("#", html_i('', "fa fa-pencil")." ".phrase('edit', CAPITALIZE), array('class' => 'ajax-loader', 'data-url' => $load_url, 'data-target' => '#cms_viewer'));
  }

  $link = $imageObj->link;
  $image_title = $imageObj->title ?: phrase('no_title', CAPITALIZE);
  $format = $options['format'] ?: 'list';
  $item_rel = $options['item_rel'];
  
  if(isset($options['count']) && isset($options['index'])) {
    $c = $options['count'];
    $i = $options['index'];
    $link = strpos($link, '?') ? "$link&c=$c&i=$i" : "$link?c=$c&i=$i";
  }

  $link_js = $options['show_snippets'] ? 'itemprop="image"' : '';
  $link_data = array('site_id' => $site_id, 'obj_type' => $obj_type, 'id' => $id);
  if($nav_src = $options['store_result']) $link_data['nav_source'] = $nav_src;

  $link_options = array('js' => $link_js, 'rel' => $item_rel, 'class' => $link_class, 'data-data' => json_encode($link_data));
  
  switch($format) {
    case 'image': // plain image with link, no text (used by Galleria, Flexslider)
      $src_key = $size ? $size."_src" : 'thumb_src';      
      $src = $imageObj->$src_key;
      $image = "\n  ".html_link($imageObj->medium_src, html_image($src, $imageObj->description, array('class' => 'copyright', 'js' => $js)), $link_options);
      $output .= $image;
      break;

    case 'icons': // Images with title
      $image = "\n  ".html_div(html_link($link, html_image($imageObj->thumb_src, $imageObj->description, array('js' => $js)), $link_options).($checkbox ? html_div($checkbox, 'abs_top_left') : ''), 'img_cont relative');
      if($image_title) $image .= html_div(html_link($link, $image_title, $link_options), 'icon_list_title').html_div($imageObj->count_string, 'item_count center');
      $item_class = 'icon_list_item';      
      $output .= $image;
      break;
      
    case 'compact': // Compact list          
      $cols = array();
      $thumb_width = 30;
      $cols[] = html_div(html_link($link, html_image($imageObj->thumb_src, $image_title, array('class' => 'thumb')), $link_options), 'img_cont'); // first col
      $cols[] = html_h4(html_link($link, $image_title, $link_options), 'inline').html_div($imageObj->count_string, 'item_count inline');
      $cols[] = html_div($checkbox, '');
      $output .= css_columns($cols);
      break;
      
    case 'list': // Pretty list
      $cols = array();
      $cols[] = html_div(html_link($link, html_image($imageObj->small_src, $image_title, array('class' => 'thumb')), $link_options), 'img_cont'); // first col
      $cols[] = html_h4(html_link($link, $image_title, $link_options)).$imageObj->text.html_div($imageObj->count_string, 'item_count').html_div($edit_button, 'pad_top');
      $cols[] = ($checkbox ? html_div($checkbox, 'right') : '').$slider;

      $output .= css_columns($cols);
      break;

    default: // html
      $output .= "unknown format $format";
      break;
  }

  if($error = $imageObj->error) $output .= html_div($error, 'red error');
  //$output .= dump($options, true);
  $output .= $debug;
  return $output;
}


function gallery_user_link($siteObj, $userObj_or_id=NULL, $options=NULL) {
  $role = $options['role'];
  $user_type = $role ? get_object('user_role', $role, 'name') : 'user';
  //echo("role=$role ut=$user_type<br>");
  $base_link = $options['base_link'] ?: site_page_link($siteObj, PAGE_GALLERY, array('search' => array('object_type' => 'user', 'object_subtype' => $role))); // e.g '/art/artists/'
  $profileObj = $userObj_or_id ? find_user_profile($userObj_or_id) : NULL;
  if(!$profileObj) return $base_link;    
  return $base_link.url_safe($profileObj->display_name ?: $user_type)."-$profileObj->user_id/";
}

function gallery_vault_link($siteObj, $userObj_or_id, $options=NULL) {
  $vaultPage = site_find_pagetype($siteObj, 'Vault', $_SESSION['lang']);
  if($options['base_link'] = $vaultPage->path) return gallery_user_link($siteObj, $userObj_or_id, $options);
  return '';
}

// link to an artist (gallery) or all artists
function gallery_artist_link($siteObj, $userObj_or_id=NULL, $options=NULL) {
  $options['role'] = USER_ROLE_ARTIST;
  return gallery_user_link($siteObj, $userObj_or_id, $options);
}

// link to an curator (gallery) or all curators
function gallery_curator_link($siteObj, $userObj_or_id=NULL, $options=NULL) {
  $options['role'] = USER_ROLE_CURATOR;
  return gallery_user_link($siteObj, $userObj_or_id, $options);
}

// link to a media_collection (set) or all sets
function gallery_collection_link($siteObj, $collectionObj_or_id=NULL, $options=NULL) {
  $collectionObj = $collectionObj_or_id ? any2obj('media_collection', $collectionObj_or_id) : NULL;
  $base_link = $options['base_link'] ?: site_page_link($siteObj, PAGE_GALLERY, array('search' => array('object_type' => 'media_collection', 'object_subtype' => $collectionObj->type ?: $options['type']))); // e.g. '/art/project/'
  if(!$collectionObj) return $base_link;  
  return $base_link.($collectionObj->title ? url_safe($collectionObj->title) : 'exhibition')."-$collectionObj->id/";
}

// link to a folder (gallery)                                     
function gallery_folder_link($siteObj, $galleryObj_or_id, $options=NULL) {
  $base_link = $options['base_link'] ?: site_page_link($siteObj, PAGE_GALLERY, array('search' => array('object_type' => 'gallery'))); // e.g. '/art/project/'
  $galleryObj = any2obj('gallery', $galleryObj_or_id);
  return $base_link.($galleryObj->title ? url_safe($galleryObj->title) : 'project')."-$galleryObj->id/";
}


// link to an image (media)
function gallery_media_link($siteObj, $mediaObj_or_id, $options=NULL) {
  $base_link = $options['base_link'] ?: site_page_link($siteObj, PAGE_GALLERY, array('search' => array('object_type' => 'media'))); // e.g. '/art/project/'
  $mediaObj = any2obj('media', $mediaObj_or_id);
  return $base_link.($mediaObj->title ? url_safe($mediaObj->title) : 'image')."-$mediaObj->id/";
}

function site_gallery_image($siteObj, $mediaObj, $options=NULL) {
  $imageObj = gallery_title_image($siteObj, array('obj_type' => 'media', 'id' => $mediaObj->id), $options); 
  //dump($imageObj);
  $output = html_div(html_image($imageObj->medium_src, $imageObj->title, array('class' => 'copyright')), 'gallery_image_container img_cont', array('id' => 'media-item-'.$mediaObj->id));  
  $output .= html_div(html_h2($imageObj->title ?: phrase('no_title', CAPITALIZE)), 'gallery_image_title');  
  //if($imageObj->description) $output .= html_div($imageObj->description, 'gallery_image_description');  
  if($mediaObj->short_description) $output .= html_div($mediaObj->short_description, 'gallery_image_description');  
  if($mediaObj->comment) $output .= html_div($mediaObj->comment, 'gallery_image_description');  
  return $output;
}

function site_gallery_menu($siteObj, $pageObj, $data=NULL, $options=NULL) {  
  global $u_id;
  global $u_level;
  //dump($options);
  $auth_edit = $options['auth_edit'];    
  $items = array();
  $item_classes = array();
  $obj_type = $data['obj_type'];
  $obj_subtype = $data['obj_subtype'];
  $parent_type = $data['parent_type'];
  $title = $options['title'];
  
  $obj = $data['object'];
  $id = $data['id'];

  if($auth_edit) {  
    if($search = $data['search']) {
      $child_obj_type = $search['obj_type'];
      $child_obj_subtype = $search['obj_subtype'];
    }
    $trash = $options['trash'];           
    
    if($target = $options['target']) $data['target'] = $target;
    $recycle = false;
    
    //dump($data);
    //dump($options);
    //echo("obj_type=$obj_type");
    switch($obj_type) {
      case 'gallery':
        $galleryObj = $obj;
      //dump($galleryObj);
        break;
      case 'user':
        break;
      case 'media_collection':
        break;
      case 'media':
        $single_image = true;  // show one image in gallery
        //echo("pt=$parent_type");
        if($parent_type == 'gallery') $show_image_buttons = true; // show one image in gallery
        //echo("si=$show_image_buttons");      
        break;
      default:
        break;
    }
      
    $show_check = $target && !$single_image;  
    $hide_class = $show_check ? 'ghide ' : '';
  
    if($show_check) { // add check menu
      $sub_menu = html_link('#', form_checkbox_simple('gcheck', false, array('js' => "data-target='$target'", 'class' => 'check_on_off')).html_span('', 'garrow'), array('class' => 'gbutton gcheck gdropdown_narrow'));
      $sub_items = array();
      $sub_items[html_link('#', phrase('all', CAPITALIZE))] = array('class' => 'check_all');
      $sub_items[html_link('#', phrase('none', CAPITALIZE))] = array('class' => 'check_none');
      //$sub_items[html_link('#', phrase('starred', CAPITALIZE))] = array();
      //$sub_items[html_link('#', phrase('unstarred', CAPITALIZE))] = array();
      $sub_menu .= html_ul($sub_items, 'width-1', array("data-target" => $target));    
      $items[] = $sub_menu; 
      $item_classes[] = '';
    }
  
    // sharing  
    //dump($data);
    $can_share = $obj_type != 'gallery' && $child_obj_type != 'gallery';
    if($can_share) {
      $share_options = array('dialog_title' =>phrase('share', CAPITALIZE)." $obj_type $id", 'action' => 'share');
      $share_data = array('title' => $title, 'site_id' => $siteObj->id, 'obj_type' => $obj_type, 'id' => $id, 'user_id' => $u_id, 'page_id' => $pageObj->id);
      $items[] = html_link('#', '&nbsp;', array('title' => phrase('share', CAPITALIZE)." $obj_type $id", 'class' => 'gbutton sharing left gshare', 'data-options' => json_encode($share_options), 'data-data' => json_encode($share_data)));
      $item_classes[] = '';
    }
  
    // folder/project/exhibition menus for user
    if($user_id = $data['user_id']) {
      if($objs = gallery_folder_list($siteObj, $user_id, array('object_only' => true))) {
        //dump($objs);
        $sub_menu = html_link('#', phrase('folders', CAPITALIZE).html_span('', 'garrow'), array('class' => 'gbutton left gdropdown'));
        $sub_items = array();
        $all_link = gallery_vault_link($siteObj, $user_id);
        $sub_items[] = html_link($all_link, html_em(phrase('all', CAPITALIZE)));
        foreach($objs as $obj) {
          $sub_items[] = html_link($obj->link, $obj->title);
        }
        $sub_menu .= html_ul($sub_items, 'width-3');
        $items[] = $sub_menu;
        $item_classes[] = '';
      }
  
      if($objs = gallery_collection_list($siteObj, MEDIA_COLLECTION_TYPE_PROJECT, $user_id, array('object_only' => true))) {
        //dump($objs);
        $sub_menu = html_link('#', phrase('projects', CAPITALIZE).html_span('', 'garrow'), array('class' => 'gbutton left gdropdown'));
        $sub_items = array();
        $all_link = gallery_artist_link($siteObj, $user_id);
        $sub_items[] = html_link($all_link, html_em(phrase('all', CAPITALIZE)));
        foreach($objs as $obj) {
          $sub_items[] = html_link($obj->link, $obj->title);
        }
        $sub_menu .= html_ul($sub_items, 'width-3');
        $items[] = $sub_menu;
        $item_classes[] = '';
      }
  
      if($objs = gallery_collection_list($siteObj, MEDIA_COLLECTION_TYPE_EXHIBITION, $user_id, array('object_only' => true))) {
        $sub_menu = html_link('#', phrase('exhibitions', CAPITALIZE).html_span('', 'garrow'), array('class' => 'gbutton left gdropdown'));
        $sub_items = array();
        $all_link = gallery_curator_link($siteObj, $user_id);
        $sub_items[] = html_link($all_link, html_em(phrase('all', CAPITALIZE)));
        foreach($objs as $obj) {
          $sub_items[] = html_link($obj->link, $obj->title);
        }
        $sub_menu .= html_ul($sub_items, 'width-3');
        $items[] = $sub_menu;
        $item_classes[] = '';
      }
  
    }
    
  
    // move to menu 
    //dump($data);
    if(!$trash && $child_obj_type == 'media') {
      $recycle = true;
      if($galleryObj) {
        $gallerySearch['active'] = 1;
        $gallerySearch['site_id'] = $siteObj->id;
        $gallerySearch['user_id'] = $galleryObj->user_id;
        $gallerySearch['id'] = array('!=', $galleryObj->id);  
        $galleryObjs = find_objects('gallery', $gallerySearch);
        if($galleryObjs) { 
          $sub_menu = html_link('#', html_span('', 'garrow'), array('title' => phrase('move_to', CAPITALIZE)." ".phrase('folder'), 'class' => 'gbutton folder left gdropdown_narrow'));
          $sub_items = array();
          foreach($galleryObjs as $galleryObj) {
            $sub_items[html_link('#', $galleryObj->title)] = array('data-gallery_id' => $galleryObj->id);
          }
          $sub_menu .= html_ul($sub_items, 'width-2 move_to', array('data-target' => $target, 'data-data' => json_encode(array('obj_type' => 'gallery', 'site_id' => $siteObj->id, 'user_id' => $u_id)), 'data-operation' => 'gallery-move', 'data-handler' => 'delete_div_handler'));    
          $items[] = $sub_menu;
          $item_classes[] = $hide_class;
        }
        $show_image_buttons = true;
      }
  
      // add to project (Publish)    
      $collection_type = MEDIA_COLLECTION_TYPE_PROJECT;
      $collection_type_name = get_object('media_collection_type', $collection_type, 'name');
      $collectionSearch['site_id'] = $siteObj->id;
      $collectionSearch['active'] = 1;
      $collectionSearch['type'] = $collection_type;
      $collectionSearch['user_id'] = $galleryObj->user_id;    
      $collectionObjs = find_objects('media_collection', $collectionSearch);
      if($collectionObjs) { // add to menu
        $sub_menu = html_link('#', html_span('', 'garrow'), array('title' => phrase('add_to', CAPITALIZE)." $collection_type_name", 'class' => 'gbutton publish left gdropdown_narrow'));
        $sub_items = array();
        foreach($collectionObjs as $collectionObj) {
          $sub_items[html_link('#', $collectionObj->title)] = array('data-collection_id' => $collectionObj->id);
        }
        $sub_menu .= html_ul($sub_items, 'width-2 add_to', array('data-target' => $target, 'data-data' => json_encode(array('obj_type' => $child_obj_type, 'site_id' => $siteObj->id, 'user_id' => $u_id)), 'data-operation' => 'collection-add', 'data-handler' => 'collection-add'));    
        $items[] = $sub_menu;
        $item_classes[] = $hide_class;
      }
    }
  
    
    if($obj_type == 'media_collection' && $obj_subtype == MEDIA_COLLECTION_TYPE_PROJECT) { // add to exhibition
      $collection_type = MEDIA_COLLECTION_TYPE_EXHIBITION;
      $collection_type_name = get_object('media_collection_type', $collection_type, 'name');
      $collectionSearch['site_id'] = $siteObj->id;
      $collectionSearch['active'] = 1;
      $collectionSearch['type'] = $collection_type;
      $collectionSearch['user_id'] = $u_id;    
      $collectionObjs = find_objects('media_collection', $collectionSearch);
      if($collectionObjs) { // add to menu
        $sub_menu = html_link('#', html_span('', 'garrow'), array('title' => phrase('add_to', CAPITALIZE)." $collection_type_name", 'class' => 'gbutton exhibit left gdropdown_narrow'));
        $sub_items = array();
        foreach($collectionObjs as $collectionObj) {
          $sub_items[html_link('#', $collectionObj->title)] = array('data-collection_id' => $collectionObj->id);
        }
        $sub_menu .= html_ul($sub_items, 'width-2 add_to', array('data-target' => $target, 'data-data' => json_encode(array('obj_type' => $child_obj_type, 'site_id' => $siteObj->id, 'user_id' => $u_id)), 'data-operation' => 'collection-add', 'data-handler' => 'collection-add'));    
        $items[] = $sub_menu;
        $item_classes[] = $hide_class;
      }
    }
  
    //dump($data);
    // exhibit entire project
    if($child_obj_type == 'media_collection' && $child_obj_subtype == MEDIA_COLLECTION_TYPE_PROJECT) { 
      $collection_type = MEDIA_COLLECTION_TYPE_EXHIBITION;
      //$collection_type_name = get_object('media_collection_type', $collection_type, 'name');
      $exhibitOptions['target'] = $target; 
      $exhibitOptions['obj_type'] = $child_obj_type; 
      $exhibitOptions['operation'] = 'collection-new'; 
      $exhibitOptions['action'] = 'publish'; 
      
      $items[] = html_link('#', html_span('', 'garrow'), array('title' => phrase('exhibit_entire_project', CAPITALIZE), 'class' => 'gbutton exhibit left', 'data-data' => json_encode(array('obj_type' => 'media_collection', 'site_id' => $siteObj->id, 'user_id' => $u_id, 'type' => $collection_type)), 'data-options' => json_encode($exhibitOptions)));
      $item_classes[] = $hide_class;
    }
    
    
    // publish entire folder
    if($child_obj_type == 'gallery' && !$trash) { 
      $recycle = true;
      $collection_type = MEDIA_COLLECTION_TYPE_PROJECT;
      //$collection_type_name = get_object('media_collection_type', $collection_type, 'name');
      $publishOptions['target'] = $target; 
      $publishOptions['obj_type'] = $child_obj_type; 
      $publishOptions['operation'] = 'collection-new'; 
      $publishOptions['action'] = 'publish'; 
      $items[] = html_link('#', html_span('', 'garrow'), array('title' => phrase('publish_entire_folder', CAPITALIZE), 'class' => 'gbutton publish left gpublish', 'data-data' => json_encode(array('obj_type' => 'media_collection', 'site_id' => $siteObj->id, 'user_id' => $u_id, 'type' => $collection_type)), 'data-options' => json_encode($publishOptions)));
      $item_classes[] = $hide_class;
    }
  
    if(0) { // more menu
      $sub_menu = html_link('#', phrase('more', CAPITALIZE).html_span('', 'garrow'), array('class' => 'gbutton left gdropdown'));
      $sub_items = array();
      $folders = array(1 => 'Do something', 2 => 'Do something else');
      foreach($folders as $key => $name) {
        $sub_items[html_link('#', $name)] = array('class' => '');
      }
      $sub_menu .= html_ul($sub_items, 'width-2');    
      $items[] = $sub_menu;
      $item_classes[] = $hide_class;
    }
  
    
    // recycle / delete
    $delOptions['target'] = $target; 
    $delOptions['obj_type'] = $child_obj_type; 
    $delOptions['handler'] = 'delete_div_handler,update_gallery_trash'; 
    $delOptions['prefix'] = $options['prefix'] ?: 'media-item-';   
    $delOptions['action'] = $recycle ? 'recycle' : 'delete';
    
    //dump($options);
    $delOptions['handler_target'] = 'gallery_trash'; 
    
    $trash_text = $recycle ? phrase('recycle', CAPITALIZE) : phrase('delete', CAPITALIZE); 
    $undelOptions = $delOptions;
    $undelOptions['recycle'] = 1; 
    $undelOptions['action'] = 'restore'; 
    
    $del_class = $recycle ? 'trash' : 'delete';
    $items[] = html_link('#', '&nbsp;', array('title' => $trash_text, 'class' => "$del_class gbutton left gdelete", 'data-options' => json_encode($delOptions))); 
    $item_classes[] = $hide_class;
    if($trash) {
      $items[] = html_link('#', '&nbsp;', array('title' => phrase('undelete', CAPITALIZE), 'class' => 'gbutton undelete left gundelete', 'data-options' => json_encode($undelOptions)));
      $item_classes[] = $hide_class;
    }
  
  
    //$items[] = html_link('#', phrase('archive', CAPITALIZE), array('class' => 'left')); $item_classes[] = 'ghide garchive';
    //$items[] = html_link('#', phrase('spam', CAPITALIZE), array('class' => 'left')); $item_classes[] = 'ghide gspam';
    
    // single image buttons (recycle, rotate, publish ?)
    if($show_image_buttons) {
      $rotateOptions['target'] = $target; 
      $rotateOptions['obj_type'] = 'media'; 
      $rotateOptions['handler'] = 'rotate_media_handler'; 
      $rotateOptions['prefix'] = $options['prefix'] ?: 'media-item-'; 
      $rrOptions=$rlOptions=$rotateOptions;
      $rlOptions['degrees'] = 270; 
      $rrOptions['degrees'] = 90; 
      $items[] = html_link('#', '&nbsp;', array('class' => 'gbutton left rotate_right grotate', 'title' => phrase('rotate', CAPITALIZE).' '.phrase('right'), 'data-options' => json_encode($rrOptions))); 
      $item_classes[] = $hide_class;
      $items[] = html_link('#', '&nbsp;', array('class' => 'gbutton left rotate_left grotate', 'title' => phrase('rotate', CAPITALIZE).' '.phrase('left'), 'data-options' => json_encode($rlOptions))); 
      $item_classes[] = $hide_class;
    
    }

  }
  
  // previous/next
  // foo
  //dump($data);      
  $navData['obj_type'] = $data['obj_type'];
  $navData['obj_subtype'] = $data['obj_subtype'];

  if($child_obj_type) $navData['child_obj_type'] = $child_obj_type;
  if($child_obj_subtype) $navData['child_obj_subtype'] = $child_obj_subtype;
  
  if($data['id']) $navData['id'] = $data['id'];
  if($data['parent_id']) $navData['parent_id'] = $data['parent_id'];
  if($data['parent_type']) $navData['parent_type'] = $data['parent_type'];
  $navData['site_id']  = $siteObj->id;

  $navOptions['link_only'] = 1;
  if($options['auth_edit']) $navOptions['auth_edit'] = $options['auth_edit'];
  if($options['trash']) $navOptions['trash'] = $options['trash'];
  
  $index = $_GET['i'];
  if($count = $_GET['c']) {
    $navData['count'] = $count;
    $navData['index'] = $index;
    $item_nav = html_b($index + 1)." ".phrase('of')." ".html_b($count);
    $obj_name = '';  
    if($index) {
      $prev_link = replace_query_var('', 'i', $index-1); 
      $items[] = html_link('#', '&nbsp;',  array('data-nav' => json_encode($navData), 'data-options' => json_encode($navOptions), 'title' => phrase('previous', CAPITALIZE).' '.$obj_name, 'class' => 'gbutton previous')); 
      $item_classes[] = '';
    }
    $items[] = html_link('#', $item_nav, array('class' => 'num')); 
    $item_classes[] = '';
    
    if($index < $count-1) {
      $next_link = replace_query_var('', 'i', $index+1); 
      $items[] = html_link('#', '&nbsp;',  array('data-nav' => json_encode($navData), 'data-options' => json_encode($navOptions), 'title' => phrase('next', CAPITALIZE).' '.$obj_name, 'class' => 'gbutton next')); 
      $item_classes[] = '';
    }
  }
  
  $gmenu_options = array('item_classes' => $item_classes, 'data' => $data);
  
  return site_gmenu($siteObj, $items, $gmenu_options);
}

function site_gmenu_nav($siteObj, $navData, $options = array()) {
  $nav_source = $navData['nav_source'];
  $index = $navData['index'];
  $navData['site_id'] = $siteObj->id;
  $ref = $navData['ref'] ?: '';
  $base = $navData['base'] ?: '';
  if($count = $navData['count']) {
    //$item_nav = html_b($index + 1)." ".phrase('of')." ".html_b($count);
    $obj_name = '';  
    if($index) {
      $prev_link = replace_query_var('', 'i', $index-1);
      $navData['index'] = $index - 1;
      $items[] = html_link('', '&nbsp;',  array('data-nav' => json_encode($navData), 'data-options' => json_encode($navOptions), 'title' => phrase('previous', CAPITALIZE).' '.$obj_name, 'class' => 'gbutton new-tab gprevious')); 
    }
    //$items[] = html_link('#', $item_nav, array('class' => 'num')); 
    //$item_classes[] = '';
    
    if($index < $count-1) {
      $next_link = replace_query_var('', 'i', $index+1); 
      $navData['index'] = $index + 1;
      $items[] = html_link('', '&nbsp;',  array('data-nav' => json_encode($navData), 'data-options' => json_encode($navOptions), 'title' => phrase('next', CAPITALIZE).' '.$obj_name, 'class' => 'gbutton new-tab gnext')); 
    }
  }
  //$output = "source=$nav_source nav c=$count i=$index<br>";
  $output .= site_gmenu($siteObj, $items, $gmenu_options);
  return $output;
}

// returns a (virtual) image objects with information needed to display the title image for artist/project/collection/image
function gallery_title_image($siteObj_or_id, $search, $options=NULL) {
  global $u_id;
  $siteObj = any2obj('site', $siteObj_or_id);
  $site_id = $siteObj->id;
  
  $class    = $options['class'] ?: "vp-loader-$viewer";
  $size     = $options['size'] ?: 'thumb';
  $data     = $options['data'];
  $format   = $options['format'];
  $index    = $options['index'] ?: 0;
  $obj_type = $search['obj_type'] ?: $options['obj_type'];
  $obj      = $options['object'];                             
  $id       = $search['id'] ?: $obj->id;
  $auth_edit = $options['auth_edit']; 
  $base_link = $options['base_link'];
  $link_only = $options['link_only'];
  $full = $options['full'];

  $parent_obj_type = $options['parent']['obj_type'];
  $parent_id       = $options['parent']['id'];
  $parent_index    = $options['parent']['index'] ?: 0;

  $available_dot = '';

  // special case: media_collection_map
  if($obj_type == 'media_collection_map') {
    $obj_type = 'media';
    $id = $obj->media_id;
    $collectionObj = get_object('media_collection', $obj->media_collection_id);
    $collection_id = $collectionObj->id;    
    if($curator_id = $collectionObj->user_id) {
      $curatorObj = get_object('user', $curator_id);
      $curator_email = get_user_email($curatorObj);
      $curator_name = user_display_name($curatorObj);
    }
    $obj = get_object('media', $id);
  }
  
  $search['obj_type'] = $obj_type;  
  $debug = false;               
  $count = 0;
  
  //echo("gti: ot=$obj_type id=$id<br>".dump($search, true));
  
  switch($obj_type) {
    case 'media': // single image from database
      $base_link = $options['base_link'];
      //echo("bl=$base_link<br>");
      if(!$base_link) $artwork_link = site_page_link($siteObj, PAGE_GALLERY, array('search' => array('object_type' => 'media'))); // e.g. '/art/project/'
      $link = $base_link ? "$base_link?mid=$id" : $artwork_link.($imageObj->title ? url_safe($imageObj->title) : 'image')."-$id/";
      //echo("gh-link=$link<br>");
      //if($auth_edit) $edit_link = $link;
      
      if($options['link_only']) return $link;
      $media_id = $id;
      $mediaObj = $obj ?: get_object('media', $id);
      
      //if($options['obj_type'] == 'media_collection_map') {
      //  $map_id = $options['id'];
      //  $mapObj = $obj ?: get_object('media_collection_map', $map_id);
      //  $media_id = $mapObj->media_id;
      //  $mediaObj = get_object('media', $media_id);
      //} else {      
      //}
      
      $db_key = $options['db_key'] ?: 'src';
      $photo_url = $src = $mediaObj->$db_key;
      
      //$photo_url = array_pop(explode('/', $path));
      //$directory = $path;

      $available = $mediaObj->original_available;
      if($available == DB_YES) {
        $available_dot = html_span('&#x25cf;', 'green').' ';
      } elseif($available == DB_NO) {
        $available_dot = html_span('&#x25cf;', 'red').' ';
      } else {
        $available_dot = '';
      }

      $directory = $mediaObj->path;
      $title     = $mediaObj->name; // ?: phrase('no_title');
      $short_desc = $mediaObj->short_description;
      
      if($short_desc) {
        //$title .= ": $short_desc"; 
      } 
      
      $description  = gallery_photo_info($mediaObj, $options['info_fields']);
      $gallery_id   = $options['gallery_id'];  
      $gallery_name = $options['gallery_name'];       
      $artist_id    = $options['artist_id']; 
      $artist_name  = $options['artist_name'];      
      $hide_count = true;
      $full_src = $_SERVER['DOCUMENT_ROOT'].$directory.($mediaObj->resize ? "full/" : "").$src;
      //dump($options);dump($mediaObj);echo("full=$full_src<br>");die();
      if(!is_file($full_src)) {
        $use_base = true; // hack for now for properties
        $error = "Media $id not found".html_break()."Dir=$full_src";
        //dump($options);
        //dump($mediaObj);
      }
      $obj = $mediaObj;
    
      break;

    case 'media_collection':
      $collection_id = $id;
      if(!$id) {
        mydie("collection_title_image: type=collection no id");
      }
      $collectionObj = $obj ?: get_object('media_collection', $id);
      $title = $collection_name = $collectionObj->title; // ?: phrase('no_title', CAPITALIZE);
      if(!$collectionObj) mydie("Collection $collection_id not found");       
      $alt = $collectionObj->title ?: "Collection $collection_id";
      $artist_id = $user_id = $collectionObj->user_id;
      if(!$user_id) mydie("Gallery has no owner");
      $userObj = get_object('user', $collectionObj->user_id);
      $link = gallery_collection_link($siteObj, $collectionObj, array('base_link' => $options['base_link']));
      if($options['link_only']) return $link;

      $count_type_singular = 'image';
      $count_type_plural = 'images';

      $media_search['media_collection_id'] = $collectionObj->id;
      if(!$auth_edit) $media_search['active'] = 1;
      $mediaMapObjs = find_objects('media_collection_map', $media_search);
      $count = count($mediaMapObjs);

      if($title_image = $collectionObj->title_image) {
        $photo_url = print_media($title_image, array('source_only' => true, 'size' => $options['size']));
        $directory = '';  
      } elseif($count) {
        $first_media_id = $mediaMapObjs[0]->media_id;
        $mediaObj = get_object('media', $first_media_id);
        //dump($mediaMapObjs);
        //dump($mediaObj);
        $directory = $mediaObj->path;
        $photo_url = $mediaObj->src;
        //$directory = "/user/images/$artist_user_id/$galleryObj->id/";
        //$photo_url = find_object('media', $media_search, 'src');
      } else {
        $directory = "";
        $photo_url = '/images/icons/gallery-64.png';
      }

      break;
      
    case 'gallery':                           
      $gallery_id = $id;
      if(!$id) {
        dump($options);
        mydie("gallery_title_image: type=gallery no id");
      }
      $galleryObj = $obj ?: get_object('gallery', $gallery_id);
      $title = $gallery_name = $galleryObj->title; // ?: phrase('no_title', CAPITALIZE);
      if(!$galleryObj) mydie("Gallery $gallery_id not found");       
      $alt = $galleryObj->title ?: "Gallery $gallery_id";
      $artist_id = $user_id = $galleryObj->user_id;
      if(!$user_id) mydie("Gallery has no owner");
      $userObj = get_object('user', $galleryObj->user_id);
      $link = gallery_folder_link($siteObj, $galleryObj, array('base_link' => $options['base_link']));
      if($options['link_only']) return $link;

      $count_type_singular = 'image';
      $count_type_plural = 'images';

      $media_search['parent_type'] = 'gallery';
      $media_search['parent_id'] =  $gallery_id;
      $media_search['active'] = 1;
      $count = count_objects('media', $media_search);
      //echo(dump($media_search, true)." count = $count");
      if($title_image = $galleryObj->image) { // does this gallery have a custom image
        $photo_url = print_media($title_image, array('source_only' => true, 'size' => $options['size']));
        $directory = '';  
      } else if($mObj = find_object('media', $media_search)) { // use first image of gallery (as returned by site_gallery_item)
        $mediaObj = $mObj;
      }
      $title_link = html_link("#", html_div('', 'mini-sprite-32 s3'), array('title' => "View project $title", 'js' => "data-search='".json_encode(array('obj_type' => 'gallery', 'id' => $id, 'parent_index' => $index))."'", 'class' => $class)).' ';  

      if($artist_id) {
        if(!$artist_name) {
          $profileObj = $obj ?: find_object('user_profile', array('user_id' => $user_id));
          $artist_name = $profileObj->display_name ?: get_name_string($profileObj);
        }  
      }

      // catchall
      if(!$photo_url || !file_exists($_SERVER['DOCUMENT_ROOT'].$directory.$photo_url)) {
        //echo("couldn't find ".$_SERVER['DOCUMENT_ROOT'].$directory.$photo_url.html_break());
        $photo_url = '/images/icons/gallery-64.png';
        $directory = '';          
      }
      
      break;

    case 'user':
      $profileObj = $obj ?: find_object('user_profile', array('user_id' => $id));
      //dump($search);die("Gh user_id = $user_id");
      //dump($profileObj);
      $count_type = 'media_collection';

      $gallerySearch['user_id'] = $artist_id = $user_id = $id;  // todo: add artist object ?
      if(!$auth_edit) $gallerySearch['active'] = 1;
      $collectionSearch = $gallerySearch;
      
      $alt = $profileObj->display_name;
      $title = $artist_name = $profileObj->display_name ?: get_name_string($profileObj);
      $role = $options['role']; // ?: get_object('user', $user_id, 'roles');
      $link = gallery_user_link($siteObj, $user_id, array('base_link' => $base_link)); 
      if($options['link_only']) return $link;

      //$link = "?obj_type=artist&id=$artist_id";
      //echo("role=$role<br>");
      switch($role) {
        case USER_ROLE_ARTIST:
          $count_type_singular = 'project';
          $count_type_plural = 'projects';
          $collectionSearch['type'] = MEDIA_COLLECTION_TYPE_PROJECT;
          break;
        case USER_ROLE_CURATOR:
          $count_type_singular = 'exhibition';
          $count_type_plural = 'exhibitions';
          $collectionSearch['type'] = MEDIA_COLLECTION_TYPE_EXHIBITION;
          break;
        default:
          break;
      }
      $count_search = search2filters($collectionSearch);
        
      if($photo_url = print_media($profileObj->photo, array('source_only' => true, 'size' => $options['size']))) { // use artist profile photo
        $directory = ''; // "";
        //echo("purl=$photo_url");
        //echo("profile=$profileObj->display_name pp=$profileObj->photo size=$options[size] url=$photo_url<br>");
        //die("artist has profile pic: dir=$directory url=$photo_url");
      } else if($galleryObj = find_object('gallery', $gallerySearch)) { // find first gallery
        //dump($galleryObj);die();
        $titleImageSearch = array('obj_type' => 'gallery', 'id' => $galleryObj->id);
        //dump($titleImageSearch);die();
        if($photo_url = gallery_title_image($siteObj, $titleImageSearch, array('object' => $galleryObj, 'source_only' => true))) {
          $directory = "/user/images/$user_id/$galleryObj->id/";      
        }
      }
      // catchall
      if(!$photo_url || (!is_url($photo_url) && !file_exists($_SERVER['DOCUMENT_ROOT'].$directory.$photo_url))) {
        //echo("couldn't find ".$_SERVER['DOCUMENT_ROOT'].$directory.$photo_url.html_break());
        $photo_url = '/images/png/user_profile_pic_225.png';
        $directory = '';          
      }
      $title_link = html_link("#", html_div('', 'mini-sprite-32 s3'), array('title' => "View projects by $artist_name", 'js' => "data-search='".json_encode(array('obj_type' => 'artist', 'id' => $artist_id, 'parent_index' => $index))."'", 'class' => $class)).' ';  
      $country = $profileObj->country ? get_object('countries', $profileObj->country, 'name') : '';
      $text = implode(', ', array_filter(array($profileObj->city, $profileObj->region, $country)));
      break;

    default: 
      mydie($obj_type ? "gallery_title_image: unsupported object type $obj_type" : "gallery_title_image: no object type");
      break;
  }
  if($mediaObj && !$artist_id) {
    if($mediaObj->parent_type == 'gallery' && $mediaObj->parent_id) $artist_id = get_object('gallery', $mediaObj->parent_id, 'user_id');
    // $artist_id = $mediaObj->user_id;
  }
  
  if($artist_id && !$artist_name) {
    $profileObj = find_object('user_profile', array('user_id' => $artist_id));
    $artist_name = $profileObj->display_name ?: get_name_string($profileObj);
  }
  
  if($options['source_only']) return $photo_url;

  if(!$count) $count = $count_type && $count_search ? get_objects_count($count_type, $count_search) : 0;
  if(!isset($options['use_path'])) $options['use_path'] = true; 
  
  if(!$mediaObj) {
    $mediaObj->path = $directory;
    $mediaObj->src = $photo_url;
  }
  $imageObj = gallery_image_object($siteObj, $mediaObj, $options);

  //$imageObj->small_src = 'http://www.suntrek.org/images/image-solar-surface-below05.gif';
  //echo("Title=$title bl=$base_link link='$link' <br>");      
  //dump($options);
  //dump($imageObj);die();
  
  if($mediaObj) {
    $copy_fields = array('original_year','original_height','original_height','original_url','original_technique',
      'original_width','original_count','original_available',
      'original_currency','original_price',
      'digital_currency', 'digital_price');
    foreach($copy_fields as $k) $imageObj->$k = $mediaObj->$k;
  }
  
  $imageObj->obj_type     = $obj_type; 
  $imageObj->id           = $id;
  $imageObj->image        = $imageObj->big_src; 
  $imageObj->href         = $use_base ? $imageObj->base_src : $imageObj->medium_src;
  $imageObj->artist_name  = $artist_name;
  $imageObj->artist_id    = $artist_id;
  $imageObj->gallery_id   = $artist_id;
  $imageObj->gallery_name = $gallery_name;
  $imageObj->available    = $available_dot;
  $imageObj->title        = $title;
  $imageObj->short_description  = $short_desc;
  $imageObj->description  = $description;
  $imageObj->text         = $text;
  $imageObj->link         = $link;
  $imageObj->error        = $error;
  if(!$hide_count) {
    $imageObj->count        = $count;
    $imageObj->count_string = $count.' '.($count == 1 ? phrase($count_type_singular ?: $count_type) : phrase($count_type_plural ?: $count_type.'s'));
  }
  //dump($imageObj);
  switch($parent_obj_type) {
    case 'gallery':
      $back_link = html_link("#", html_i('', 'fa fa-arrow-circle-left fa-2x'),  array('title' => "Back to $artist_name", 'js' => "data-search='".json_encode(array('obj_type' => 'artist', 'id' => $artist_id, 'index' => $parent_index))."'", 'class' => $class)).' ';  
      break;
    case 'artist':
      $back_link = html_link("#", html_i('', 'fa fa-arrow-circle-left fa-2x'),  array('title' => phrase('all_artist', CAPITALIZE), 'js' => "data-search='".json_encode(array('site_id' => $siteObj->id, 'index' => $parent_index))."'", 'class' => $class)).' ';
      break;
    case 'media_collection':
      $collection_id = $parent_id;
      $collectionObj = get_object('media_collection', $collection_id);
      if($collectionObj->type == MEDIA_COLLECTION_TYPE_EXHIBITION) $curator_id = $collectionObj->user_id;
      break;
      
    default: // all artists
      $back_link = '';
      break;
  }

  if($options['show_navigation']) {
    $title = $imageObj->available.$title;
    $org_currency = $mediaObj->original_currency;
    $org_price = $mediaObj->original_price;            
    $org_technique = $imageObj->original_technique;
    if($org_currency && $org_price > 0) {
      $price_str = get_currency_symbol($org_currency, true).formatFloat($org_price);
      $title .= " ($price_str)";
    }
    if($org_technique) $title .= " - $org_technique";
    if($artist_name) $title .= " - $artist_name";
    if($curator_name && $curator_email) $title .= "<br>Curator: <a href='mailto:$curator_email'>$curator_name</a>";


    if($title) $nav_text = html_div($title, 'gallery-nav-title');
    if($short_desc) $nav_text .= html_div($short_desc, 'gallery-nav-desc');   
    if($back_link) $nav_buttons .= $back_link;
    if($title_link) $nav_buttons .= $title_link;
    if($_SESSION['dev']) $nav_text .= "<br>session=".session_id(); 

    $loader_class_iframe = 'vp-loader-iframe';
    $loader_class = 'vp-loader-overlay';
    $loader_class_popup = 'vp-loader-popup';

    if($media_id)  {
      if($curator_id) {
        $aff_id = $curator_id;
        $aff_type = 'u';
      } else if($artist_id) {
        $aff_id = $artist_id;
        $aff_type = 'u';
      }
      //$buy_link = site_page_link($siteObj, PAGE_CART, array('full' => true, 'aff_id' => $aff_id, 'aff_type' => $aff_type));
      $about_link = site_page_link($siteObj, PAGE_ABOUT, array('full' => true, 'aff_id' => $aff_id, 'aff_type' => $aff_type));
      if($u_id) {

        $base_link = $options['base_link'];
        if(!$base_link) $artwork_link = site_page_link($siteObj, PAGE_GALLERY, array('search' => array('object_type' => 'media'))); // e.g. '/art/project/'
        $art_link = $base_link ? "$base_link?mid=$id" : $artwork_link.($imageObj->title ? url_safe($imageObj->title) : 'image')."-$obj->id/";
        $art_link = site_full_url($siteObj).$edit_link;
        $edit_link = $art_link;
        
        $edit_debug = " link=$edit_link u_id=$u_id auth_edit=".($auth_edit ? "yes" : "no");
      }

      //$nav_buttons .= html_link("#", html_div('', 'mini-sprite-32 s214', array('style' => 'margin-top:10px;margin-right:10px')), array('title' => phrase('about', CAPITALIZE)." vPatina", 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('obj_type' => 'site', 'id' => 247))."'", 'class' => $loader_class)).' ';

      $logo = html_image("https://www.vpatina.com/img/vpatina-logo-32.png", 'vPatina', array('height' => 24, 'width' => 24, 'style' => 'margin-top: -10px'));
      $nav_buttons .= html_link("#", $logo, array('title' => phrase('about', CAPITALIZE)." vPatina", 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('obj_type' => 'site', 'id' => 247))."'", 'class' => $loader_class)).' ';
      if($media_id)      $nav_buttons .= html_link("#", '<i class="fa fa-picture-o fa-2x"></i>', array('title' => "Artwork information", 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('obj_type' => 'media', 'id' => $media_id))."'", 'class' => $loader_class)).' '; //mini-sprite-32 s41
      if($collection_id) $nav_buttons .= html_link("#", '<i class="fa fa-bank fa-2x"></i>', array('title' => "Exhibition information", 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('obj_type' => 'media_collection', 'id' => $collection_id))."'", 'class' => $loader_class)).' '; //mini-sprite-32 s41
      if($artist_id)     $nav_buttons .= html_link("#", '<i class="fa fa-paint-brush fa-2x"></i>', array('title' => "Artist information", 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('obj_type' => 'user', 'id' => $artist_id))."'", 'class' => $loader_class)).' '; // 'mini-sprite-32 s34'
      if($curator_id)    $nav_buttons .= html_link("#", '<i class="fa fa-eye fa-2x"></i>', array('title' => "Curator information", 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('obj_type' => 'user', 'id' => $curator_id))."'", 'class' => $loader_class)).' '; // 'mini-sprite-32 s213'

      $buy_link = "http://www.vpatina.com/login/?_aid=$aff_id&ref=%2Fmyvpatina%2F%3F_aif%3D".$media_id."%26_aid%3D".$aff_id."&".time()."-".rand(1,1000);; /** todo: fix this */
      if($buy_link) {
        $download_title = phrase('follow', CAPITALIZE);
        //$nav_buttons .= html_link('#', '<i class="fa fa-star-o fa-2x"></i>', array('data-url' => $buy_link."?".time()."-".rand(1,1000), 'title' => $download_title, 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('parent_type' => 'media', 'parent_id' => $media_id))."'", 'class' => 'vp-loader-iframe')).' ';
        $alreadyFollow = 0;
        if($u_id) {
          $userCollectionObj = gallery_find_user_collection($siteObj, $u_id);    
          $collection_id = $userCollectionObj->id;
          $alreadyFollow = find_object('media_collection_map', array('media_id' => $media_id, 'media_collection_id' => $collection_id), 'id');
        }
        $imageObj->follow = $alreadyFollow;
        
        $fa_icon = $alreadyFollow ? "fa-star" : "fa-star-o";

        $class = $u_id ? "favorite-link" : "vp-loader-iframe";
        $followSearch = array('parent_type' => 'media', 'parent_id' => $media_id, 'site_id' => $site_id, 'user_id' => $u_id);
        $followParent = array('obj_type' => $parent_obj_type, 'id' => $parent_id);
        
        $followOptions = $alredyFollow ? array("title" => "You already follow this") : array('data-url' => $buy_link, 
                'title' => $download_title, 
                'data-parent' => $followParent, 
                'data-search' => $followSearch, 
                'data-viewer' => $viewer, 
                'class' => $class);
                
        $nav_buttons .= html_link('#', '<i class="fa fa '.$fa_icon.' fa-2x"></i>', $followOptions).' ';
        /**
        if($pv_id = gallery_user_owns_image($siteObj, $u_id, $media_id)) { // user already owns image
          list($product_id, $variant_id) = explode('-', $pv_id);
          $resolution = $variant_id ? get_object('product_variant', $variant_id, 'option_value') : '';
          if($resolution == 'high') {
            $nav_buttons .= html_div('<i class="fa fa-star fa-2x"></i>', '', array('title' => phrase('you_own_this', CAPITALIZE))); // 'mini-sprite-32 s154'
            $download_title = '';
          } else {
            $download_title = phrase('upgrade_to_high_resolution', CAPITALIZE);
          }       
          */
        //} else if($pv_id = gallery_image_in_cart($siteObj_or_id, $media_id)) {
        //  list($product_id, $variant_id) = explode('-', $pv_id);
        //  $resolution = $variant_id ? get_object('product_variant', $variant_id, 'option_value') : '';
        //  $cart_title = phrase('already_in_your_cart', CAPITALIZE).($resolution ? " ($resolution resolution)" : '');
        //  $nav_buttons .= html_div('', 'mini-sprite-32 s154', array('title' => $cart_title));
        //  $download_title = '';
        //}
        /**
        if($download_title) {
          $download_title .= " ($media_id)";
          $nav_buttons .= html_link('#', '<i class="fa fa-star-o fa-2x"></i>', array('data-url' => $buy_link."?".time()."-".rand(1,1000), 'title' => $download_title, 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('parent_type' => 'media', 'parent_id' => $media_id))."'", 'class' => 'vp-loader-iframe')).' ';
        }
        */
        
      }
      //if($edit_link)   $nav_buttons .= html_link('#', html_div('', 'mini-sprite-32 s8'), array('data-url' => $edit_link, 'title' => phrase('edit', CAPITALIZE).$edit_debug, 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('parent_type' => 'media', 'parent_id' => $media_id))."'", 'class' => $loader_class_iframe)).' ';

      if(1) { // social
        
        if($base_link) {
          $share_link = site_full_url($siteObj).$base_link."?mid=$media_id";
          $share_link = str_replace('//test.', '//www.', $share_link);
          $share_link = str_replace('http://www.', 'https://www.', $share_link); // dirty hack
          if($share_link) {
            $tiny_url = url_shorten($share_link);
            if(!$collection_name && $collection_id) $collection_name = get_object('media_collection', $collection_id, 'title');
            if(!$artist_name && $artist_id) $artist_name = user_display_name($artist_id);

            $tweet = ($collection_name ? "$collection_name: " : '' )."$title";
            
            $data = [];
            $nav_buttons .= html_link("#", '<i class="fa fa-share-alt fa-2x"></i>', ['class' => 'vp-loader-overlay', 'title' => "Share", 'data-data' => $data]);
            // $nav_buttons .= dump($data,true);
                // 'js' => "data-viewer='$viewer' 
                // data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' 
                // data-search='".json_encode(array('obj_type' => 'share', 'foo' => 'bar', 'share_link' => $share_link, 'parent_obj_type' => $parent_obj_type, 'parent_id' => $parent_id))."'", 
                // 'class' => $loader_class)).' ';
            
            // <a title="Share" class="vp-loader-overlay" href="#" data-viewer="" 
            // data-parent="{&quot;obj_type&quot;:&quot;media_collection&quot;,&quot;id&quot;:&quot;1743&quot;}" 
            // data-search="{&quot;obj_type&quot;:&quot;share&quot;,&quot;foo&quot;:&quot;bar&quot;,&quot;share_link&quot;:&quot;http://test.vpatina.com/exhibition/1743/?mid=1113654&quot;,&quot;parent_obj_type&quot;:&quot;media_collection&quot;,&quot;parent_id&quot;:&quot;1743&quot;}"><i class="fa fa-share-alt fa-2x"></i></a>
            
          }
        }
      }
    }
    

        
    if($u_id) $nav_buttons .= user_button($siteObj, $u_id);
    $close_button = html_link("#", html_div('', 'mini-sprite-32 s163'), array('title' => "Hide", 'class' => 'gallery-close-button'));
    //$show_button  = html_link("#", html_div('', 'mini-sprite-32 s212'), array('title' => "Show image information", 'class' => 'gallery-info-button'));
    $nav_left = $close_button.html_div($nav_text, 'gallery-nav-text');
    $nav_right = html_div($nav_buttons, 'gallery-nav-buttons');
    $imageObj->navigation = $nav_left.$nav_right.html_div('', 'clear_float');
    //$imageObj->navigation_off = "foo".$show_button;
    
  } else {
    $imageObj->navigation = html_h2($title);//.$description;
  }
  //$imageObj->navigation = 'got here';
  //unset($imageObj->navigation);
  
  //$size_path = image_size_dir($size);
  $src_key = $size ? $size."_src" : 'thumb_src';
  

  $src = is_url($photo_url) ? $photo_url : $imageObj->$src_key;
  $imageObj->photo = html_image($src, $alt, array('js' => "data-search='".json_encode($search)."'", 'class' => $class));
  //echo("size=$size key=$src_key src=$src<br>").dump($imageObj, true);
  return $imageObj;
}


// populates imageObj with relevant fields from mediaObj 
function gallery_image_object($siteObj, $mediaObj, $options=NULL) {
  if(is_object($mediaObj)) {
    //$path = $mediaObj->path && !is_url($mediaObj->path) ? site_full_url($siteObj).$mediaObj->path : $mediaObj->path;

    if(is_url($mediaObj->path)) {
      $path = $mediaObj->path; // && !is_url($mediaObj->path) ? site_full_url($siteObj).$mediaObj->path : $mediaObj->path;
    } else {
      $path = site_full_url($siteObj).$mediaObj->path;
    }
    //echo("m: $mediaObj->path p: $path".dump($mediaObj, true));die();
    $src = $mediaObj->src;
    $filename = $mediaObj->filename;

    //$path = "path=".$mediaObj->path;
    //$path .= " url? ".(is_url($mediaObj->path) ? "yes" : "no");
    //$path = "http://www.google.com/";
    
    //if(is_url($src)) return array($src, $src, $src, $src, $src); 
    if($mediaObj->resize) {
      $imageObj = new stdClass;
      $imageObj->thumb_src  = $path.image_size_dir('thumb', $options).$src; 
      $imageObj->square_src = $path.image_size_dir('square', $options).$src; 
      $imageObj->small_src  = $path.image_size_dir('small', $options).$src; 
      $imageObj->medium_src = $path.image_size_dir('medium', $options).$src; 
      $imageObj->big_src    = $path.image_size_dir('big', $options).$src;
      $imageObj->full_src   = $path.image_size_dir('full', $options).$src; 
      $imageObj->org_src    = $path.image_size_dir('org', $options).$filename;
      $imageObj->base_src   = $path.$src;
      
    } else {
      $imageObj->thumb_src = $imageObj->square_src = $imageObj->small_src = $imageObj->medium_src = $imageObj->big_src = $imageObj->full_src = $imageObj->org_src = $path.$src;
    }                                                                                             

  } elseif(strlen($mediaObj)) {
    $imageObj->thumb_src = $imageObj->square_src = $imageObj->small_src = $imageObj->medium_src = $imageObj->big_src = $imageObj->full_src = $imageObj->org_src = $mediaObj; 
  } else {
    return $mediaObj; 
    dump($mediaObj);die("gallery_image_object: no input");
  }
  $imageObj->image = $imageObj->big_src;
  
  return $imageObj;  
}



// extends the info fields array (fld => name);
// replaces the array value (name) with this array('name', 'length', 'readonly', 'type') 
function extend_info_fields($info_fields) {
  if(is_array($info_fields[0])) return $info_fields; // already extended

  $attributes = get_object_attributes('media', false);
  foreach($info_fields as $fld => $name) {
    $attr = $attributes[$fld];
    $flags = $attr[ATT_FLAGS];
    $max = $attr[ATT_LENGTH];
    $type = $attr[ATT_TYPE];
    $readonly = $flags & AF_READONLY_EDIT;
    if(!trim($name)) $name = $attr[ATT_NAME];    
    $extended_info_fields[$fld] = array($name, $type, $max, $readonly); 
  }
  return $extended_info_fields;
}


// returns the text used for the information lightbox
function gallery_photo_info($mediaObj, $info_fields) {
  if(!$mediaObj || !$info_fields) return ''; 
  foreach($info_fields as $fld => $array) {
    $val = $mediaObj->$fld;
    list($name, $type, $max, $readonly) = $array;
    if(!$readonly && $val) $items[] = str_replace("'", "&apos;", $val);
  }
  //$items = array_filter($items);
  if(count($items)) $desc = implode(html_break(), $items);
  return html_break().$desc;
}


function galleria_fullscreen() {
  Galleria.loadTheme('/afr/bower/galleria/themes/fullscreen/galleria.fullscreen.min.js');
}

// displays frame for editing gallery
function gallery_frame($photos, $trash) {
  if(is_array($photos)) $photos = implode("\n", $photos);
  if(is_array($trash)) $trash = implode("\n", $trash);
  $show_empty_link = $trash ? '' : 'hidden';
  $output = '
	<div id="gallery_frame" class="demo ui-widget ui-helper-clearfix">
		<ul id="gallery" class="gallery ui-helper-reset ui-helper-clearfix">
      '.$photos.'
		</ul>
		<div id="trash" class="ui-widget-content ui-state-default">
      <h4 class="ui-widget-header">
        <span class="ui-icon ui-icon-trash">Trash</span>
        Trash
				<a href="link/to/empty/trash/script/when/we/have/js/off" id="gallery_empty_trash" class="'.$show_empty_link.'" title="Empty Trash"> [ Empty Trash ]</a>
      </h4>
      <ul class="gallery ui-helper-reset">
			'.$trash.'
  		</ul>
		</div>
	</div>
	';
  return $output;
}

function gallery_photos($media_search, $options, $show_trash=false) {
  global $__header;
  $media_search['active'] = $show_trash ? 0 : 1;

  //dump($media_search);die();
  if($pics = find_objects('media', $media_search)) {
    foreach($pics as $row) {
      $photos .= gallery_photo($row, $options, $show_trash);
    }
  }
  $__header['onload'] .= $data_js;
  return $photos;
}

// function gallery_photo_id($id) {
//   if($mediaObj = get_object('media', $id)) {
//     return gallery_photo($mediaObj
//   }
// }

function gallery_photo($row, $options, $trash=false) {
  if($image = $row->image) { // a fully qualified path or URL
    $thumb = $big = $image;
  } else {
    $path = $options['path'];
    $big_path = $options['big_path'] ?: $path;
    $thumb_path = $options['thumb_path'] ?: $path;
    $fld = $options['db_key'] ?: 'src';
    $thumb = $thumb_path.$row->$fld;
    $big = $big_path.$row->$fld;
  }    
    //$filename = $desc = $row->filename;
    
    $class = $options['class'];
    
    $title = pick_first($options['title'], $row->name, $row->title, $row->src, $row->filename);
    //echo("title=$title");

    $info_fields = $options['info_fields'] ?: array();  	  
    foreach($info_fields as $fld => $array) {
      list($name, $type, $max, $readonly) = $array;
      $data[$fld] = $row->$fld;
    }
    $data_json = str_replace("'", "&apos;", json_encode($data));
    $data_str = "data-info='$data_json'";
    
    if($options['show_info'] && $options['info_field']) $title .= html_div(gallery_photo_info($row, $options['info_field']), 'gallery_photo_info');
    $li_style =  $trash ? ' style="display: list-item; width: 48px;"' : '';
    $img_style =  $trash ? ' style="height: 36px; display: inline;"' : '';
    $trash_link = $trash ? '<a href="#" title="Recycle this image" class="ui-icon ui-icon-refresh">Recycle image</a>' : 
                           '<a href="#" title="Delete this image" class="ui-icon ui-icon-trash">Delete image</a>';
    return 
'
    	<li id="li_'.$row->id.'" class="ui-widget-content ui-corner-tr float_left '.$class.'"'.$li_style.'>
    		<h5 id="lih_'.$row->id.'" class="ui-widget-header overflow_hidden">'.$title.'</h5>
    		<img id="img_'.$row->id.'" src="'.$thumb.'" alt="'.$filename.'" width="96" height="72"'.$img_style.' '.$data_str.'/>
    		<a id="mag_'.$row->id.'" href="'.$big.'" target="pic" title="'.$title.'" class="preview ui-icon ui-icon-zoomin">View larger</a>
    		'.$trash_link.'
    	</li>
';
}

// returns a (sorted) array of all images belonging to a gallery 
function site_gallery_items($siteObj, $galleryObj_or_id, $options=NULL) {
  $galleryObj = any2obj('gallery', $galleryObj_or_id);
  $gallery_id = $galleryObj->id;
  
  $search = $options['search'];
  $search['parent_type'] = 'gallery';
  $search['parent_id'] = $gallery_id;
  $search['parent_field'] = 'media';
  if(!$options['show_all']) $search['active'] = 1;
  return find_objects('media', $search, $options);
}

function user_gallery_options($siteObj, $galleryObj, $userObj=NULL, $options=NULL) {
  $gallery_options = $options;
  if(!$userObj && $galleryObj->user_id) $userObj = get_object('user', $galleryObj->user_id);
  if(!$userObj) {
    dump($galleryObj);
    die("user_gallery_options: no user found");
  }
  $parent_id = $galleryObj->parent_id;
  $default_path = $parent_id ? get_object('gallery', $parent_id, 'path')."$galleryObj->id/" : "/user/$userObj->id/vault/$galleryObj->id/";

  //$default_path = "/user/images/object/gallery/$userObj->id/$galleryObj->id/";

  //$default_resize = array('org' => array(0, 'org'), 'thumb' => array(65, 'thumbs'), 'small' => array(200, 'small', array('crop' => 1)), 'big' => array(1280, ''), 'full' => array(1920, 'full'));

  $default_resize = array('org' => array(0, 'org'), 
                        'thumb' => array($_SESSION['thumb_pic_width'] ?: 100, 'thumbs'),
                        'square' => array($_SESSION['square_pic_width'] ?: 200, 'square', array('crop' => 1)),                        
                        'small' => array($_SESSION['small_pic_width'] ?: 200, 'small'), 
                        'medium' => array($_SESSION['medium_pic_width'] ?: 640, 'medium'), 
                        'big'   => array($_SESSION['big_pic_width']   ?: 1280, 'big'), 
                        'full'  => array($_SESSION['full_pic_width']  ?: 1920, 'full'));

  $default_info = array('name' => 'Title',
                        'var1' => 'Material',
                        'var4' => 'Size',
                        'var3' => 'Year',
                        'var2' => 'Sold?',
                        'short_description' => 'Short Description',
                        'comment' => 'Description');

  $info_fields = $options['info_fields'] ?: $default_info;

  $gallery_options['path'] = $path = $options['path'] ?: $default_path;

  $gallery_options['directory'] = $options['directory'] ?: clean_path($_SESSION['docroot'].$path);

  $gallery_options['search'] = array('active' => 1, 'parent_id' => $galleryObj->id, 'parent_type' => 'gallery', 'parent_field' => 'media'); //, 'descriptiontype_id' => DESC_TYPE_ALBUM);
  $gallery_options['show_info'] = true;
  $gallery_options['h'] = $options['h'] ?: 400;
  $gallery_options['w'] = $options['w'] ?: $_SESSION['inner_width'] - 40;

  $gallery_options['info_fields'] = extend_info_fields($info_fields); 
  
  $gallery_options['min_size'] = $options['min_size'] ?: 1920;
  $gallery_options['resize'] = $resizeAr = $options['resize'] ?: $default_resize; 
  
  $resizeAr = $gallery_options['resize'];
  foreach($resizeAr as $key => $ar) {
    $size_path = $gallery_options[$key.'_path'] = clean_path($path.$resizeAr[$key][1].'/'); 
  }

  //$gallery_options['thumb_path'] = clean_path($path.$resizeAr['thumb'][1].'/'); 
  //$gallery_options['small_path'] = clean_path($path.$resizeAr['small'][1].'/'); 
  //$gallery_options['big_path']   = clean_path($path.$resizeAr['big'][1].'/'); 
  //$gallery_options['full_path']  = clean_path($path.$resizeAr['full'][1].'/'); 
  //$gallery_options['org_path'] = clean_path($path.$resizeAr['org'][1].'/'); 
  $gallery_options['use_path'] = isset($options['use_path']) ? $options['use_path'] : 'yes';

  return $gallery_options;
}

function default_gallery_options($siteObj, $options = array()) {
  $gallery_options['obj_type'] = 'media';
  $gallery_options['use_path'] = false;
  $gallery_options['show_info'] = true;
  $gallery_options['h'] = 400;
  $gallery_options['w'] = $_SESSION['inner_width_narrow'] - 40;

  $gallery_options['resize'] = array('org' => array(0, 'org'), 
                                     'thumb' => array($_SESSION['thumb_pic_width'] ?: 100, 'thumbs'), 
                                     'square' => array($_SESSION['square_pic_width'] ?: 200, 'square', array('crop' => 1)), 
                                     'small' => array($_SESSION['small_pic_width'] ?: 200, 'small'), 
                                     'medium' => array($_SESSION['medium_pic_width'] ?: 320, 'medium'), 
                                     'big'   => array($_SESSION['big_pic_width']   ?: 640, 'big'), 
                                     'full'  => array($_SESSION['full_pic_width']  ?: 1280, 'full'));

  $info_fields = array('name' => 'Title',
                       'short_description' => 'Short Description',
                       'comment' => 'Description',
                       );
   
  $gallery_options['info_fields'] = extend_info_fields($info_fields); 
  return $gallery_options;
}

function property_gallery_options($siteObj, $apt) {
  $gallery_options = default_gallery_options($siteObj);
  $gallery_options['path'] = $path = $_SESSION['media_path'].image_type_dir($apt->type).get_image_path_tail($apt);
  $gallery_options['directory'] = $dir = clean_path($_SESSION['docroot'].$path);
  $gallery_options['search'] = $search = array('active' => 1, 'parent_id' => $apt->id, 'parent_type' => $apt->type, 'descriptiontype_id' => DESC_TYPE_ALBUM);  
  $gallery_options['media_data'] = array_filter(array('site_id' => $siteObj->id, 'directory' => $dir, 'path' => $path, 'parent_field' => 'media', 'parent_type' => $search['parent_type'], 'parent_id' => $search['parent_id'], 'descriptiontype_id' => $search['descriptiontype_id'])); // todo: add property type/id if needed

  $resizeAr = $gallery_options['resize'];
  foreach($resizeAr as $key => $ar) {
    $size_path = $gallery_options[$key.'_path'] = clean_path($path.$resizeAr[$key][1].'/'); 
  }
  // hack to use big instead of full if full apth doesn't exist
  if($full_path = $gallery_options['full_path']) {
    if(!is_dir($_SESSION['docroot'].$full_path)) {                                  
      $gallery_options['full_path'] = $gallery_options['big_path'];
    }
  }

  return $gallery_options;
}

function gallery_options_js($options) {
  $info_fields = $options['info_fields'] ?: array();
  $resize = $options['resize'] ?: array();
  
  $js = "var gallery_info_fields = ".json_encode($info_fields).";\n";
  $js .= "var gallery_resize = ".json_encode($resize).";\n";
  return $js;  
}

//   
function site_gallery_edit($siteObj, $options='') {
  global $__header;
  global $upload_dir;

  $__header['onload'] .= "edit_gallery_init('media');";
  $site_dir = site_directory($siteObj);
  $upload_path = $options['path']; // relative path
  $upload_directory = $upload_path ? clean_path($site_dir.$upload_path) : $upload_dir;
  
  $size_limit = ini_get('upload_max_filesize');
  $resize = $options['resize'] ?: array();
  $search = $options['search'];
  $info_fields = $options['info_fields'] ?: array();
  
  //$size_limit = "2k";
  //dump($options);
  // dump($mediaObjs);die();
  
  $h = $options['h'];
  $w = $options['w'];
  
  $media_search = $options['search'];
  $media_data = $options['data'] ?: array();
  $photos  = gallery_photos($media_search, $options);
  $trash  = gallery_photos($media_search, $options, true);

  //$head = button_add_media('media', $obj_type, $id).html_break();

  //$resize = $options['resize'] = array('big' => 640, 'thumbs' => 96);

  $upload_options['allow_multiple'] = true;
  $upload_options['size_limit'] = $size_limit;
  $upload_options['allowed_types'] = array('image', 'video');
  $upload_options['path'] = $upload_path;
  $upload_options['directory'] = $upload_directory;
  $upload_options['media_data'] = array_filter(array('site_id' => $siteObj->id, 'directory' => $upload_directory, 'path' => $upload_path, 'parent_type' => $search['parent_type'], 'parent_id' => $search['parent_id'], 'descriptiontype_id' => $search['descriptiontype_id'])); // todo: add property type/id if needed
  
  foreach($media_data as $k => $v) $upload_options['media_data'][$k] = $v; 
  
  $upload_options['resize'] = $resize;
  $upload_options['handler'] = 'gallery_upload_handler';

  // dump($upload_options);
  $head .= jquery_file_uploader('gallery_upload', $val, $upload_options);
  $form .= gallery_frame($photos, $trash);

  $instructions = array(
    'Click "Upload a file" to upload one or more images from your computer.',
    'Firefox/Chrome users can also drag and drop images from their computer onto the button.',
    "Images will be uploaded to $upload_path ($upload_directory)",
    "The maximum image size is $size_limit",
    "The picture will automatically be resized for web use, but upload will be faster if you resize it before uploading",
    'To add an image from your computer, click "Browse", select a file, then click "Add".',
    'To add an image from a URL, type or paste the URL into the URL text box, then click "Add".',
    'You can reorder the gallery by simply dragging the images.',
    'You can remove an image from the gallery by either by dragging it to the Trash or by clicking the trash icon.',
    'You can restore an image from trash by dragging it back to the gallery or by clicking the recycle icon.',
    'You can view larger image by hovering over the zoom icon.',
    'Click on the zoom icon or double-click the image to edit image title, descriptions, etc.',
    'To permanently delete images in the Trash, press the "empty trash" button. This can not be undone.',
    'No need to save - all changes are written to the server in the background.');

  //$output = 'Edit Gallery';

  if(count($instructions)) $output .= show_hide_div(html_ul($instructions), 'Show instructions').html_break();

  if($head) $output .= $head.html_break();
  if($buttons) $form .= implode(' ', $buttons);
  $form .= array2form($hidden_fields);
  $output .= html_form($form, 'save_property');
  //$prefix = 'image_info_';

  // the form used for the info dialog                                              
  //$attributes = get_object_attributes('media', false);
  //dump($info_fields);
  foreach($info_fields as $fld => $array) {            
    list($name, $type, $max, $readonly) = $array;
    $class = $readonly ? 'uneditable-input' : '';
    switch($type) {
    case 'html':
      $fields[$name]= form_ckeditor($fld, ''); 
      break;
    case 'text':
      $fields[$name]= form_text($prefix.$fld, '', array('rows' => 6, 'cols' => 40));
      break;                                                                        
    default:                                            
      $fields[$name]= form_input($prefix.$fld, 'text', '', array('size' => 40, 'max' => 40, 'class' => $class, 'readonly' => $readonly)); // 40, $max, $readonly, $class);
      break;
    }
    
  }
    
  $form_name = 'image_info_form';
  
  $form2 = html_div('', 'messages', "id='".$form_name."_messages'");    
  $form2 .= html_div(html_form(html_fieldset($fields, 'web20 fieldset_left_wide').form_input($prefix.'id', 'hidden', ''), $form_name), 'float_left');    
  $handler = 'gallery_update_handler';
  $on_save = "onclick=\"return save_object('$form_name', 'media', 'edit', {'handler' :'$handler'});\"";
  $on_cancel = "onclick=\"close_parent_dialog('$form_name');return false;\"";    

  $buttons[] = form_input('edit_object', 'submit', ' Save ', array('js' => $on_save)); 
  $buttons[]= form_input('cancel_object', 'submit', ' Cancel ', array('js' => $on_cancel)); 
  $form2 .= implode(' ', $buttons);

  $preview = html_image('', '', array('id' =>' image_info_preview'));
  $output .= html_div($form2.$preview, 'visuallyhidden', "id='image_info_dialog'");
  $output .= html_script_inline(gallery_options_js($options));
  if($tail) $output .= html_break().$tail;

  return html_div($output, 'gallery_editor');
}

function site_user_galleries($siteObj, $user_id, $options=NULL) {             
  $search = $options['search'];
  $search['user_id'] = $user_id; // required 
  $search['site_id'] = $siteObj->id; // required 
  if(!$options['auth_edit']) $search['active'] = 1; // show only active unless auth-edit
  return $options['count'] ? count_objects('gallery', $search) : find_objects('gallery', $search); 
}

function site_user_collections($siteObj, $user_id, $options=NULL) {             
  $search = $options['search'];
  $search['user_id'] = $user_id; // required 
  $search['site_id'] = $siteObj->id; // required 
  if(!$options['auth_edit']) $search['active'] = 1; // show only active unless auth-edit
  return $options['count'] ? count_objects('media_collection', $search) : find_objects('media_collection', $search); 
}

// show one gallery (formats: json, galleria, fancybox, list, or edit)
function site_gallery($siteObj, $options='') {
  global $__header;
  global $edit;
  if($options['edit'] || $edit == 'media:gallery:0') return site_gallery_edit($siteObj, $options);

  $path        = $options['path'];  
  $viewer      = pick_first($_GET['viewer'], $options['viewer'], 'galleria'); // format of item (json, various HTML formats   
  $format      = pick_first($_GET['format'], $options['format'], 'json'); // 3rd party viewer (galleria, fancybox)
  $search      = $options['search'];  
  $auth_edit   = $options['auth_edit'];
  $info_fields = $options['info_fields'] = $options['info_fields'] ?: extend_info_fields(array('name' => 'Title', 'short_description' => 'Short Description','comment' => 'Description'));
  $resize      = $options['resize'] = $options['resize'] ?: array('org' => array(0, 'org'), 'thumb' => array(96, 'thumbs'), 'big' => array(640, 'big'), 'full' => array(1280, 'full'));

  if(is_array($options['media'])) $mediaObjs = $options['media'];
  else if(is_array($options['mediaObjs'])) $mediaObjs = $options['mediaObjs'];
  //$mediaObjs   = $options['media'] ?: $options['mediaObjs'];

  // default class/id for container, list items
  $container_class = '';
  $container_id = '';
  $item_class = '';
  $html = false;

  if($title = $options['title']) $title = html_h2($title, 'content_title');
  
  foreach($resize as $key => $size_ar) {
    list($size, $dir) = $size_ar;
    if(!$options[$key.'_path']) $options[$key.'_path'] = $path.($dir ? "$dir/" : '');
  }
                     
  if($jsonObjs = $options['json_data']) $format = 'json'; // we already have the data in json format
  
  //if($jsonObjs) dump($jsonObjs);    
  //dump($options);
  $gallery_id = pick_first($options['gallery_id'], $options['id'], $options['search']['parent_id'], 1);

  switch($viewer) {
    case 'galleria':
      $show_info  = $options['show_info']; 
      $show_counter = $options['show_counter']; 
      $h = ($options['h'] ?: $options['height']) ?: 400;
      $w = ($options['w'] ?: $options['width'])  ?: ($_SESSION['inner_width'] ? $_SESSION['inner_width'] - 40 : 600);
      $showCarousel = isset($options['show_carousel']) ? $options['show_carousel'] : 'true'; 
      $galleria_version = '1.4.2'; // 1.2.5
      $__header['scripts'][] = "/afr/bower/galleria/src/galleria.js";    
      $theme  = pick_first(array($options['theme'], $_SESSION['galleria_theme'] ,'azur'));

      //$themes = array('classic', 'white', 'dots', 'fullscreen', 'azur');
      $themes = array('classic', 'fullscreen', 'azur');
      $sel_theme = $_GET['theme'] ?: $theme ;
      if(!in_array($sel_theme, $themes)) $sel_theme = $themes[0]; // default to first theme if not set or invalid
      $__header['css'][] = "/afr/bower/galleria/themes/$sel_theme/galleria.$sel_theme.css";
      $theme_links = array('azur' => 'Azur', 'classic' => 'Classic', 'fullscreen' => 'Fullscreen');
     
      $container_id = 'galleria_'.$gallery_id;
      $container_class = 'loading-big';

      $showInfo = $show_info ? 'true' : 'false'; 
      $showCounter = $show_counter ? 'true' : 'false'; 
      $transition = $options['transition'] ?: 'slide'; 
      $transition_speed = $options['transition_speed'] ?: '700'; 
      if($format == 'json') {
        $dataSource = "dataSource: gallery_data,";
      } else {
        $format = 'image';
      }

      $viewer_script = "         
        Galleria.loadTheme('/afr/bower/galleria/themes/$sel_theme/galleria.$sel_theme.min.js');
        $('#".$container_id."').galleria({
            $dataSource
            width: $w,
            height: $h,
            transition: '$transition',
            transition_speed: $transition_speed,
            showInfo: $showInfo,
            carousel: $showCarousel,
            clicknext: false,
            showImagenav: true,
            showCounter: $showCounter
        });
        $('#galleria').removeClass('loading-big');
        ";
      
      break;
    case 'fancybox': 
      $container_id = 'fancybox_'.$gallery_id;
      $item_rel = $options['item_rel'] = "gallery_".$gallery_id;
      if($format == 'json') {
        $viewer_script = '$.fancybox.open(gallery_data);';
      } else {
        $format = 'image';
        $viewer_script = '
        $(".fancybox").fancybox({
          openEffect	: "none",
          closeEffect	: "none"
        });
        //$.fancybox.open($(".fancybox"));
        var $items = $("DIV #'.$container_id.' [rel='.$item_rel.']");
        //alert("rel count=" + $items.length);
        $.fancybox.open($items);
        ';
      }
      
      $container_class = 'loading-big';
      $link_class = 'fancybox';
      $item_class = 'inline';
      $no_list = $options['no_list'] = true;
      break;                                 

    case 'flexslider':
      $__header['scripts'][] = '/afr/bower/flexslider/jquery.flexslider-min.js';
      $__header['css'][] = '/afr/bower/flexslider/flexslider.css';
      $options['size'] = 'medium';
      $container_class = 'flexslider';
      $list_class = 'slides';
      $viewer_script = '
      $(".flexslider").flexslider({
        animation: "slide"
      });';  
      break;
      
    default:
      die("site_gallery: Unknown viewer $viewer");
  }
    

  // begin output
  $options['container_id']    = $container_id;
  $options['container_class'] = $container_class;
  $options['link_class']      = $link_class;
  $options['item_class']      = $item_class;
  $options['list_class']      = $list_class;
  
  unset($options['viewer']); // or we get an endless loop
  unset($_GET['viewer']);
  
  //$output = "sg: viewer=$viewer format=$format<br>";
  $options['format'] = 'image';
  $options['auth_edit'] = false;
  $options['no_session'] = true; // don't mess with session variables (format/viewer)
  //echo("viewer=$viewer format=$format");
  switch($format) {
    case 'json':
      $json_options = $options;
      $json_options['format'] = 'json';
      
      //echo(caller_stack().dump($json_options,true));
      $jsonObjs = $jsonObjs ?: gallery_object_list($siteObj, $json_options);
      if($jsonObjs) $script = "var gallery_data=".json_encode($jsonObjs)."\n";
      //if($artistJson) $script .= "var gallery_artists=".json_encode($artistJson)."\n";
      $output .= html_div('', $container_class, array('id' => $container_id, 'width' => $w, 'height' => $h)); // place holder for viewer
      break;

    default: // html
      //dump($options);
      $object_list = gallery_object_list($siteObj, $options);
      if($viewer == 'flexslider') {
        //echo("ol");
        //dump($object_list);
      }
      $output .= html_div($object_list, $container_class, array('id' => $container_id));
      //dump($output);
      //$output .= $no_list ? html_div(implode("\n", $items), $container_class, array('id' => $container_id)) : html_ul($items, $container_class, array('id' => $container_id, 'item_class' => $item_class, 'show_id' => true));
      break;
  }
  

  // add script
  if($script) {
    if($options['noscript']) $_SESSION['galleria_script'] = $script; // don't run script now (e.g. when tabbed or hidden), but store for later
    else $output .= html_script_inline($script);
  }
  if($viewer_script) $__header['onload'] .= $viewer_script;
  
  return $output;

}

// find all media in exhibition or project, make into product
function collections2products($site_id, $collection_id = 0) {
  $collection_ids = $collection_id ? array($collection_id) : find_objects('media_collection', array('site_id' => $site_id, 'type' => array(MEDIA_COLLECTION_TYPE_EXHIBITION, MEDIA_COLLECTION_TYPE_PROJECT), 'active' => 1),  array('fields' => 'distinct id'));
  $media_ids = find_objects('media_collection_map', array('media_collection_id' => $collection_ids, 'active' => 1), array('fields' => 'distinct media_id'));
  foreach($media_ids as $media_id) { // add products
    $mediaObj = get_object('media', $media_id); 
    if(!$mediaObj->active) {
      $output .= html_b("media $media_id is not active - skipping<br>");
      continue;
    }
    list($productObj, $message) = media2product($site_id, $media_id);
    $output .= $message;
    $product_id = $productObj->id; 
    if($product_id && !$mediaObj->product_id) {
      $output .= "Setting missing Product id of $mediaObj->id ($mediaObj->name) to $product_id<br>";
      db_query("UPDATE media set product_id=$product_id where id=$media_id"); 
    }
  }  
  return $output;
}

function media2product($site_id, $media_id) {
  $price = get_object('media', $media_id, 'digital_price');
  $siteObj = get_object('site', $site_id);
  $client_id = $siteObj->client_id;
  $output = '';
  $productData = obj2product($site_id, 'media', $media_id);

  if($productObj = find_object('product', array('source_type' => 'media', 'fid' => $media_id))) {
    $product_id = $productObj->id;
    $output .= "Found existing product $product_id ($productObj->name) for media $media_id<br>";
    list($product_id, $errors) = update_object('product', $productData, $product_id); // update product
    $action = "Update";
  } else {
    $productData['client_id'] = $client_id;
    list($product_id, $errors) = add_object('product', $productData); // make new product
    $action = "ADD";
  }

  if($errors) {dump($errors);die("Failed to $action product");}
  if($product_id) {
    $productObj = get_object('product', $product_id);
    $product_name = $productObj->name;
    $output .= "$action product $product_id ($product_name)<br>";
    if($price > 0) { /** 1001 = product_option: resolution */
      $output .= add_product_variants($product_id, 1001, array('values' => array('medium', 'high'), 'prices' => array(0, $price)));
    } else {
      $output .= add_product_variants($product_id, 1001, array('values' => array('medium'), 'prices' => array(0)));
    }
  }
  
  return array($productObj, $output);
}

// function to exhibit an image from a project (media_collection_map)
function gallery_exhibit_image($siteObj, $mapObj_or_id, $collection_id, $options=array()) {
  $mapObj = any2obj('media_collection_map', $mapObj_or_id);
  if(!$mapObj) return json_error_object("Map not found");
  $media_id = $mapObj->media_id;  

  return gallery_publish_image($siteObj, $media_id, $collection_id, $options);
}

// function to add art to collection
function gallery_publish_artwork($siteObj, $artworkObj_or_id, $collection_id, $options=array()) {
  $artworkObj = any2obj('artwork', $artworkObj_or_id);
  if(!$artworkObj) return json_error_object("Artwork {$artworkObj_or_id} not found");
  $errors = $messages = array();
  
  $artwork_id = $artworkObj->id;
  if(!$artwork_id) return json_error_object("Cannot publish: Missing id");
  $collection_user_id = get_object('media_collection', $collection_id, 'user_id');
  if(find_object('media_collection_map', array('artwork_id' => $artwork_id, 'media_collection_id' => $collection_id))) {
    $artwork_name = $artworkObj->title ?: phrase('this_image', CAPITALIZE);
    return json_error_object("$artwork_name is already in this collection.");
  }
  $mapData['site_id'] = $siteObj->id;
  $mapData['user_id'] = $collection_user_id ?: $artworkObj->user_id;
  $mapData['media_collection_id'] = $collection_id;
  $mapData['collection_type'] = get_object('media_collection', $collection_id, 'type');
  $mapData['media_id'] = $artworkObj->title_image;
  $mapData['artwork_id'] = $artwork_id;
  $mapData['rank'] = $artworkObj->rank;
  $mapData['artist_id'] = $artworkObj->artist_id;
  $mapData['artist_name'] = user_display_name($artworkObj->artist_id);
  $mapData['curator_name'] = user_display_name($artworkObj->user_id);
  $mapData['original_technique'] = $artworkObj->technique;
  $mapData['original_year'] = $artworkObj->primary_year;
  $mapData['name'] = $artworkObj->title;
  $mapData['type'] = 30;
  
  $image_title = $artworkObj->title ?: $artwork_id;

  print_log("gallery_publish_artwork: publish artwork {$artwork_id} to collection {$collection_id} stack:".caller_stack(), 'gallery', LOG_LEVEL_TEST);
  list($id, $errors) = add_object('media_collection_map', $mapData);

  if(!$id) {
    $errors = array_merge_smart("Failed to add artwork $artwork_id to collection $collection_id", $errors);
    return json_error_object($errors); // Tried to add $image_title to $collection_type_name $collectionObj->title, but no id received");
  }

  $result->data = $id;
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

//function to add selected art from drive
function gallery_publish_folderimgart($siteObj, $this_id, $user_id, $options=array()) {
  $get_media = find_object('media', ['id'=>$this_id]);
  $duplicate = find_object('media_collection_map', array('user_id'=>$user_id, 'collection_type'=>150, 'active'=>1, 'media_id'=>$this_id, 'artwork_id' => 0, 'media_collection_id' => 0 ));
 
  if($duplicate) {
    return json_error_object("$get_media->filename is already in this profile.");
  }
  $mapData['site_id'] = $siteObj->id;
  $mapData['user_id'] = $user_id;
  $mapData['media_collection_id'] = 1;
  $mapData['collection_type'] = 150;
  $mapData['media_id'] = $get_media->id;
  $mapData['artwork_id'] = 0;
  $mapData['name'] = $get_media->name;
  
  $image_title = $get_media->filename;
  list($id, $errors) = add_object('media_collection_map', $mapData);
 
  if(!$id) {
    $errors = array_merge_smart("Failed to add medias $get_media->filename to profile $user_id", $errors);
    return json_error_object($errors); 
  }

  $result->data = $id;
  
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

//function to Add profile logo from media
function gallery_publish_logo($siteObj, $this_id, $user_id, $options=array()) {
  $get_media = find_object('media', ['id'=> $this_id]);
  $get_user = find_object('user_profile', ['user_id'=>$user_id]);

  if($get_media){
    $photo_arr['filename'] = $get_media->path.$get_media->src;
    $photo_arr['id'] = $get_media->id;
    $photo_arr['type'] = $get_media->type;

    $get_user->photo_json = json_encode($photo_arr);
    $get_user->photo = $this_id;
    list($id, $errors) = update_object('user_profile', $get_user, $get_user->id);
    if($errors) return json_error_object($errors);  
  }
  if(!$id) {
    $errors = array_merge_smart("Failed to add media $get_media->filename to Logo image $user_id", $errors);
    return json_error_object($errors); 
  }

  $result->data = $id;
  
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

//function to Add title image from media
function gallery_publish_artworkimg($siteObj, $this_id, $artwork_id, $options=array()) {
  $get_media = find_object('media', ['id'=> $this_id]);
  $artwork = find_object('artwork', ['id'=> $artwork_id, 'active'=>1]);
  if($get_media){
    $title_image_arr = [];
    $title_image_arr[]['filename'] = $get_media->path.'medium/'.$get_media->src;
    $title_image_arr[]['id'] = $get_media->id;
    $title_image_arr[]['type'] = $get_media->type;

    $artwork->title_image_json = json_encode($title_image_arr);
    $artwork->title_image = $get_media->id;
    list($id, $errors) = update_object('artwork', $artwork, $artwork->id);
    if($errors) return json_error_object($errors); 
  }
  if(!$id) {
    $errors = array_merge_smart("Failed to add media $get_media->filename to Logo image $user_id", $errors);
    return json_error_object($errors); 
  }
  $result->data = $id;
  
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

//function to Add related images to artwork
function gallery_publish_relatedimg($siteObj, $this_id, $artwork_media_id,$options=array()) {
  $get_media = find_object('artwork_media', ['id'=> $artwork_media_id]);
  $media = find_object('media', ['id'=> $get_media->media, 'parent_type'=> 'artwork_media', 'parent_field'=>'media']);
  
  if($media){
    list($del_id, $errors) = delete_object('media', $media->id);
  }
  if($get_media){
    $get_media->media = $this_id;
    list($id, $errors) = update_object('artwork_media', $get_media, $get_media->id);
    if($errors) return json_error_object($errors); 
  }
  if(!$id) {
    $errors = array_merge_smart("Failed to add media $get_media->filename to related artwork $user_id", $errors);
    return json_error_object($errors); 
  }
  $result->data = $id;
  
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

//function to Add website logo from media
function gallery_publish_weblogo($siteObj, $this_id, $minisite_id, $options=array()) {
  $get_media = find_object('media', ['id'=> $this_id]);
  $minisite = find_object('minisite', ['id'=> $minisite_id, 'active'=>1]);
  if($minisite){
    $existing = $minisite->logo;
    if($existing){
      list($del_id, $errors) = delete_object('media', $existing);
      if($errors) return json_error_object($errors); 
    }
    $minisite->logo = $get_media->id;
    list($id, $errors) = update_object('minisite', $minisite, $minisite->id);
    if($errors) return json_error_object($errors);
  }
  if(!$id) {
    $errors = array_merge_smart("Failed to add media $get_media->filename to Logo image $minisite_id", $errors);
    return json_error_object($errors); 
  }
  $result->data = $id;
  
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

//function to Add Exhibition title image logo from media
function gallery_publish_titleimg($siteObj, $this_id, $collection_id, $options=array()) {
  $get_media = find_object('media', ['id'=> $this_id]);
  $collection = find_object('media_collection', ['id'=> $collection_id]);
  $title_image_arr = [];
  if($collection){
    $existing = $collection->title_image;
    $title_img = find_object('media', ['id'=> $existing]);
    
    if($title_img->parent_id == $collection->id){
      //list($del_id, $errors) = delete_object('media', $existing);
      //if($errors) return json_error_object($errors); 
    }
    $title_image_arr[]['filename'] = $get_media->path.'medium/'.$get_media->src;
    $title_image_arr[]['id'] = $get_media->id;
    $title_image_arr[]['type'] = $get_media->type;
    $collection->title_image_json = json_encode($title_image_arr);
    
    $collection->title_image = $get_media->id;
    list($id, $errors) = update_object('media_collection', $collection, $collection->id);
    if($errors) return json_error_object($errors);
  }
  if(!$id) {
    $errors = array_merge_smart("Failed to add media $get_media->filename to Logo image $minisite_id", $errors);
    return json_error_object($errors); 
  }
  $result->data = $id;
  
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}


// function to add media to collection 
function gallery_publish_image($siteObj, $mediaObj_or_id, $collection_id, $options=array()) {
  $mediaObj = any2obj('media', $mediaObj_or_id);
  if(!$mediaObj) return json_error_object("Media not found");
  $errors = $messages = array();
  
  $media_id = $mediaObj->id;
  if(!$media_id) return json_error_object("Cannot publish: Missing id");
  $collection_user_id = get_object('media_collection', $collection_id, 'user_id');
  
  print_log("gallery_publish_image: publish image {$media_id} to collection {$collection_id} stack:".caller_stack(), 'gallery', LOG_LEVEL_TEST);
  
  if(find_object('media_collection_map', array('media_id' => $media_id, 'media_collection_id' => $collection_id))) {
    $media_name = $mediaObj->name ?: phrase('this_image', CAPITALIZE);
    //$collectionObj = get_object('media_collection', $collection_id);
    //$collection_name = $collectionObj->name ?: phrase('this_collection', CAPITALIZE);
    return json_error_object("$media_name is already in this collection.");
  }
  $mapData['site_id'] = $siteObj->id;
  $mapData['user_id'] = $collection_user_id ?: $mediaObj->user_id;
  $mapData['media_collection_id'] = $collection_id;
  $mapData['collection_type'] = get_object('media_collection', $collection_id, 'type');
  $mapData['media_id'] = $media_id;
  $mapData['rank'] = $mediaObj->rank;
  $mapData['artist_id'] = $mediaObj->artist_id;
  $mapData['artist_name'] = user_display_name($mediaObj->artist_id);
  $mapData['original_technique'] = $mediaObj->original_technique;
  $mapData['original_year'] = $mediaObj->original_year;
  $mapData['name'] = $mediaObj->name;
  
  $image_title = $mediaObj->name ?: $media_id;
  list($id, $errors) = add_object('media_collection_map', $mapData);

  if(!$id) {
    $errors = array_merge_smart("Failed to add media $media_id to collection $collection_id", $errors);
    return json_error_object($errors); // Tried to add $image_title to $collection_type_name $collectionObj->title, but no id received");
  }

  $result->data = $id;
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

// function to add media to collection custom approach
function gallery_publish_images($siteObj, $mediaObj_or_id, $map_id, $options=array()) {
  $mediaObj = any2obj('media', $mediaObj_or_id);
  if(!$mediaObj) return json_error_object("Media not found");
  $errors = $messages = array();

  $media_id = $mediaObj->id;
  if(!$media_id) return json_error_object("Cannot publish: Missing id");
  $collection_map = find_object('media_collection_map', array('id' => $map_id) );

  $collection_id = $collection_map->media_collection_id;
  $collection_user_id = get_object('media_collection', $collection_id, 'user_id');
  
  print_log("gallery_publish_image: publish image {$media_id} to collection {$collection_id} stack:".caller_stack(), 'gallery', LOG_LEVEL_TEST);
  
  if(find_object('media_collection_map', array('id' => $map_id, 'media_collection_id' => $collection_id, 'media_id'=> $media_id))) {
    $media_name = $mediaObj->name ?: phrase('this_image', CAPITALIZE);
    //$collectionObj = get_object('media_collection', $collection_id);
    //$collection_name = $collectionObj->name ?: phrase('this_collection', CAPITALIZE);
    return json_error_object("$media_name is already in this collection.");
  }
  $collection_map->site_id = $siteObj->id;
  $collection_map->media_id = $media_id;
  $collection_map->artist_id = $mediaObj->artist_id;
  $collection_map->artist_name = user_display_name($mediaObj->artist_id);
  $collection_map->original_technique = $mediaObj->original_technique;
  $collection_map->original_year = $mediaObj->original_year;
  $collection_map->name = $mediaObj->name;
  
  $image_title = $mediaObj->name ?: $media_id;
  list($id, $errors) = update_object('media_collection_map', $collection_map, $collection_map->id);

  if(!$id) {
    $errors = array_merge_smart("Failed to add media $media_id to collection $collection_id", $errors);
    return json_error_object($errors); // Tried to add $image_title to $collection_type_name $collectionObj->title, but no id received");
  }

  $result->data = $id;
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
}

// function to add images to folder
function gallery_publish_folderimg($siteObj, $mediaObj_or_id, $folder_id, $options=array()) {
  $mediaObj = any2obj('media', $mediaObj_or_id);
  $check_folder = get_object('media_folder', $folder_id);
  if($check_folder->rank == 1)
    $folder_id = 0;
  if(!$mediaObj) return json_error_object("Media not found");
  else{
    $media_id = $mediaObj->id;
    if(!$media_id) return json_error_object("Cannot publish: Missing id");
    $mediaObj->folder_id = $folder_id;

    list($id, $errors) = update_object('media', $mediaObj, $media_id);
    if(!$id) {
      $errors = array_merge_smart("Failed to add media", $errors);
      return json_error_object($errors);
    }
    $result->data = $id;

  }
  $errors = $messages = array();
  if($messages) $result->messages = $messages;
  if($errors) {                 
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
    $result->id = $id;
  }
  return $result;                         
} 

// function to exhibit a collection 
function gallery_exhibit_collection($siteObj, $collectionObj_or_id, $user_id, $options=array()) {
  $collection_id = $collectionObj_or_id->id ?: get_object('media_collection', $collectionObj_or_id, 'id');
  if(!$collection_id) return json_error_object("Missing collection id");
  $options['user_id'] = $user_id;
  $options['collection_type'] = MEDIA_COLLECTION_TYPE_EXHIBITION;
  return gallery_publish_folder($siteObj, 'media_collection', $collection_id, $options);
}

// publish and exhibition
function gallery_publish_collection($siteObj, $collectionObj_or_id, $user_id, $options=array()) {
  $collection_id = $collectionObj_or_id->id ?: get_object('media_collection', $collectionObj_or_id, 'id');
  if(!$collection_id) return json_error_object("Missing collection id");
  $options['user_id'] = $user_id;
  $options['collection_type'] = MEDIA_COLLECTION_TYPE_PROJECT;
  return gallery_publish_folder($siteObj, 'media_collection', $collection_id, $options);
}

// function to publish a folder, or project 
function gallery_publish_folder($siteObj, $obj_type, $id, $options=array()) {
  $add_obj_type = 'media_collection';
  if($obj_type != 'gallery' && $obj_type != 'media_collection') return json_error_object("Unsupported type $obj_type");
  if(!$id) return json_error_object("Cannot publish: Missing id");
  $collection_type = $options['collection_type'];
  if(!$collection_type) $collection_type = $obj_type == 'gallery' ? MEDIA_COLLECTION_TYPE_PROJECT : MEDIA_COLLECTION_TYPE_EXHIBITION;
  $collection_type_name = get_object('media_collection_type', $collection_type, 'name');
  $parent_type_name = $obj_type == 'gallery' ? "Folder" : "Project"; 
  $added_ids = $errors = $messages = array();
  

  $ids = is_array($id) ? $id : array($id); // allow publish multiple
  foreach($ids as $source_id) {
    $obj = get_object($obj_type, $source_id);
    if(!$obj) return json_error_object("$parent_type_name $source_id not found");

    if($obj->type == MEDIA_COLLECTION_TYPE_PROJECT) $options['artist_id'] = $obj->user_id; // Set artist ID of exhibition to user ID of project
    
    $colData = (array) $obj;
    $colData['type'] = $collection_type;
    if($user_id = $options['user_id']) $colData['user_id'] = $user_id;
    list($id, $errors) = add_object($add_obj_type, $colData);
    if($errors) $messages[] = implode(html_break(), $errors);
    elseif(!$id) return json_error_object("Tried to add $collection_type_name $obj->title, but no id received");
    else {
      $added_ids[] = $id;
      $collectionObj = get_object($add_obj_type, $id);
      $collector_id = $collectionObj->user_id;
      $collection_id = $collectionObj->id; 
      $col_url = gallery_collection_link($siteObj, $collectionObj);
      $collection_link = html_link($col_url, "$collection_type_name $collectionObj->title");

      if($obj_type == 'gallery') {
        $mediaObjs = gallery_folder_images($siteObj, $source_id);
      } else {               
        $mapObjs = gallery_collection_maps($siteObj, $source_id);
        $mediaObjs = array();
        foreach($mapObjs as $mapObj) $mediaObjs[] = get_object('media', $mapObj->media_id);  
        $media_count = count($mediaObjs);
        $map_count = count($mapObjs);
        //$messages[] = "Found $map_count maps and $media_count images found for $obj_type $source_id";
        //json_message($messages);exit;
      }

      $count = count($mediaObjs);
      
      foreach($mediaObjs as $mediaObj) {
        $mapResult = gallery_publish_image($siteObj, $mediaObj, $collection_id);
        if($mapResult->errors) $errors = array_merge($errors, $mapResult->errors);
      }
      $messages[] = "Added $collection_link with $count images.";
      if($collection_type == MEDIA_COLLECTION_TYPE_EXHIBITION && !user_has_role($collector_id, USER_ROLE_CURATOR)) user_add_role($collector_id, USER_ROLE_CURATOR);
    }  
  }
  if($added_ids) $result->data = $added_ids;
  if($messages) $result->messages = $messages;
  if($errors) {
    $result->errors = $errors;
    $result->success = 0;
  } else {
    $result->success = 1;
  }
  return $result;
}



function media2artwork($mediaObj, $gallery_id = 0) {
  $artData = obj2array($mediaObj);
  
  $artData['title'] = trim($mediaObj->name);
  $artData['description'] = trim($mediaObj->comment);
  $artData['title_image'] = $mediaObj->id;
  $artData['comment'] = ''; 
  if($gallery_id) $artData['gallery_id'] = $gallery_id;
  
  unset($artData['id']);
  $artData['primary_year'] = $year = trim($mediaObj->original_year);
  if($year) $artData['primary_year_type'] = 'Created';
  
  $flds = ['original_technique','original_currency','original_price','original_height','original_width','original_count'];
  foreach($flds as $fld) {
    $new_fld = str_replace('original_', '', $fld);
    $artData[$new_fld] = trim($mediaObj->$fld);
    unset($artData[$fld]);
  }
  unset($artData['depth']); 
  
  if($mediaObj->original_height > 0 && $mediaObj->original_width > 0) $artData['unit'] = 'cm';
  if($tech =  $artData['technique']) {
    $type = 0; 
    if(strstr($tech, 'mixed')) $type = 22;
    else if(stristr($tech, 'draw')) $type = 21;
    else if(stristr($tech, 'print')) $type = 11;
    else if(stristr($tech, 'installation')) $type = 60;
    else if(stristr($tech, 'video') || strstr($tech, 'film')) $type = 50;
    else if(stristr($tech, 'cast')) $type = 40;
    else if(stristr($tech, 'sculpture')) $type = 30;
    else if(stristr($tech, 'photo')) $type = 20;
    else if(stristr($tech, 'performance')) $type = 70;
    
    if(!$type) $type = 10; // fallback, painting
    $artData['type'] = $type;
    
    //echo("Tech: {$tech} => $type<br>");
  } else {
    unset($artData['type']);
  }
  if($mediaObj->original_available == 1) $artData['status'] = 10;
  elseif($mediaObj->original_available == -1)  $artData['status'] = 30;

  $artFields = object_fields('artwork');
  foreach($artData as $fld => $val) {
    if(!in_array($fld, $artFields)) unset($artData[$fld]);
  }
  $artist_id = $artData['artist_id'];
  $artData['artist_ids'] = "[{$artist_id}]"; 
  $artData['artist_name'] = user_display_name($artist_id); 

  return $artData;
}  

function media2artworks() {
  $limit = 0;
  $i = 0;
  $reset = true;
  if($reset) { /** start over */
    db_query("DELETE FROM artwork");
    db_query("UPDATE db_sequence SET nextid=1000 WHERE seq_name='artwork'");
    db_query("UPDATE media_collection_map SET artwork_id = NULL");    
  }
  $rows = [];
  $rows[] = ['created', 'title', 'user', 'artist', 'count'];
  $existing_ids = db_getCol("SELECT distinct title_image FROM artwork") ?: [];
  $collectionObjs = find_objects('media_collection', ['type' => 120, 'active' => 1], ['sort' => 'created']);
  $cCount = count($collectionObjs);
  
  foreach($collectionObjs as $collectionObj) {
    if($limit && $i >= $limit) break;
    $collection_id = $collectionObj->id;
    $map_ids = db_getCol("SELECT distinct media_id FROM media_collection_map WHERE artwork_id is NULL AND media_collection_id={$collection_id} ORDER by rank");
    $count = count($map_ids);
    if(!$count) continue;
    $rows[] = [$collectionObj->created, $collectionObj->title, user_display_name($collectionObj->user_id),user_display_name($collectionObj->artist_id), $count];
    foreach($map_ids as $id) {
      if(!$id) continue;
      $mediaObj = get_object('media', $id);
      if(!$mediaObj || !$mediaObj->artist_id) continue;
      if(in_array($mediaObj->name, ['Installation Shot','Installation View'])) continue;
      $media_id = $mediaObj->id;
      
      if(in_array($media_id, $existing_ids)) continue;
      if(!trim($mediaObj->name) && !trim($mediaObj->origial_technique)) continue;
      
      $i++;
      if($limit && $i > $limit) break;
      
      $artData = media2artwork($mediaObj, $collectionObj->user_id);
      //$output .= dump($artData, true);
      
      //continue;
      
      list($aid, $errors) = add_object('artwork', $artData);
      $existing_ids[] = $media_id;
      
      if($errors) {
        $output .= "Error adding {$mediaObj->id} {$mediaObj->name}:".dump($errors, true).dump($mediaObj,true);
        print_log("Error adding {$mediaObj->id} {$mediaObj->name}:".dump($errors, true), 'artwork', LOG_LEVEL_TEST);
      } else {
        $output .= "Added artwork {$aid} {$artData['title']}<br>";
        print_log("Added artwork {$aid} {$artData['title']} created: {$artData['created']} collection: {$collectionObj->title} {$collectionObj->created} ", 'artwork', LOG_LEVEL_TEST);
        db_query("UPDATE media SET parent_type='artwork', parent_id={$aid}, parent_field='title_image' WHERE id={$media_id}");
        $artworkObj = get_object('artwork', $aid);
        if($mediaJson = media2json('artwork', $artworkObj, ['parent_field' => 'title_image'])) {
          update_object('artwork', ['title_image_json' => $mediaJson], $aid);
        }
        
      }
      $added++;
    }
  }
  $output .= html_table($rows, ['class' => 'web20']);
  print_log("Done: Added {$i} artworks in {$cCount} collections", 'artwork', LOG_LEVEL_TEST);
  db_query("UPDATE media_collection_map,artwork SET artwork_id = artwork.id WHERE media_collection_map.media_id=artwork.title_image");
  
  
  $added = 0;
  $output .= html_h2("Added {$i} artworks in {$cCount} collections");
  return $output;
}

  
/** temporary fix to be deprectated when plugin is updated - get artwork data from media */ 
function artwork2media($artworkObj_or_id) {
  if(!$artworkObj_or_id) return json_error_object("Missing Artwork Object or ID");
  $artworkObj = any2obj('artwork', $artworkObj_or_id);
  if(!$artworkObj) return json_error_object("Artwork Object not found");

  $media_id = $artworkObj->title_image;
  if(!$media_id) return json_error_object("Missing title image");
  
  $data = [
    'name' => $artworkObj->title,
    'comment' => $artworkObj->description,
    'artist_id' => $artworkObj->artist_id,
    'original_currency'  => $artworkObj->currency,
    'original_price'     => $artworkObj->price,
    'original_height'    => $artworkObj->height,
    'original_width'     => $artworkObj->width,
    //'original_depth'     => $artworkObj->depth,
    'original_year'      => $artworkObj->primary_year,
    'original_technique' => $artworkObj->technique,
    'original_count'     => $artworkObj->editions,    
  ];

  if($artworkObj->status == 10) $data['original_available'] = 1;
  else if($artworkObj->status > 10) $data['original_available'] = -1;
  
  // 'original_available' => $artworkObj->title,

  list($id, $errors) = update_object('media', $data, $media_id);
  if($errors) return json_error_object($errors);
  $result = new stdClass;
  $result->success = 1;
  $result->artwork_id = $artworkObj->id;
  $result->media_id = $media_id;
  $result->data = $data;
  return $result;  
}

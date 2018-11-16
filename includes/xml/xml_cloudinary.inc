<?php

function cloudinary_signature($siteObj_or_id, $str) {
  require_once 'vendor/cloudinary/cloudinary_php/src/Cloudinary.php';
  require_once 'vendor/cloudinary/cloudinary_php/src/Uploader.php';
  require_once 'vendor/cloudinary/cloudinary_php/src/Api.php';
  
  $site_id = any2id('site', $siteObj_or_id);
  $ci_config = get_config('ci', $site_id);
  if(!$ci_config) return json_error_object("Cannot upload, missing Cloudinary config for site {$site_id}");
  
  $config = \Cloudinary::config((array) $ci_config);
  if(!$config) return json_error_object("Cannot upload, did not receive Cloudinary config for site {$site_id}");

  $ts = time();
  $params_to_sign = "{$str}timestamp={$ts}{$ci_config->api_secret}";
  $signature = \Cloudinary::api_sign_request($params_to_sign, $ci_config->api_secret);
  return $signature;
}  

/** api call - params not yet implementd */
/** unsused - automatic comment added on  */
function cloudinary_api($siteObj_or_id, $method, $params = []) {
  require_once 'vendor/cloudinary/cloudinary_php/src/Cloudinary.php';
  require_once 'vendor/cloudinary/cloudinary_php/src/Uploader.php';
  require_once 'vendor/cloudinary/cloudinary_php/src/Api.php';
  
  $site_id = any2id('site', $siteObj_or_id);
  $ci_config = get_config('ci', $site_id);
  if(!$ci_config) return json_error_object("Cannot upload, missing Cloudinary config for site {$site_id}");
  $config = \Cloudinary::config((array) $ci_config);
  if(!$config) return json_error_object("Cannot upload, did not receive Cloudinary config for site {$site_id}");
  
  $api = new \Cloudinary\Api($config);
  $result = $api->$method();
  return $result; 
}

/** Upload individual media object to cloudinary */
function cloudinary_upload_media($mediaObj_or_id, $options=[]) {
  require_once 'vendor/cloudinary/cloudinary_php/src/Cloudinary.php';
  require_once 'vendor/cloudinary/cloudinary_php/src/Uploader.php';
  require_once 'vendor/cloudinary/cloudinary_php/src/Api.php';

  $time_start = microtime(true);

  $mediaObj = any2obj('media', $mediaObj_or_id);
  if(!$mediaObj) return json_error_object("Cloudinary: missing media");

  $errors = $messages = [];
  $tagsAr = [];
  
  $max_file_size = 10485760;
  $site_id = $mediaObj->site_id;
  $parent_id = $mediaObj->parent_id;
  $parent_type = $mediaObj->parent_type;
  $mid = $mediaObj->id;
  $success = $remote_size = 0;
  
  $ci_config = get_config('ci', $site_id);
  if(!$ci_config) return json_error_object("Cannot upload, missing Cloudinary config for site {$site_id}");

  $config = \Cloudinary::config((array) $ci_config);
  if(!$config) return json_error_object("Cannot upload, did not receive Cloudinary config for site {$site_id}");
  
  if(!$parent_id || !$parent_type) return json_error_object("Missing parent");
  $parentObj = get_object($parent_type, $parent_id);
  if(!$parentObj) return json_error_object("Missing parent");
  
  $parent_name = $parentOb->name ?: $parentObj->title;
  $tagsAr[] = $parent_type;
  $tagsAr[] = $parent_id;
  $tagsAr[] = get_object_name($parent_type);
    
  if(isset($parentObj->city_id)) $tagsAr[] = get_object('geo_city', $parentObj->city_id, 'name'); 
  if($parentObj->address_city) $tagsAr[] = $parentObj->address_city; 
  if($parentObj->address_country) {
    $tagsAr[] = cc_name($parentObj->address_country); 
    $tagsAr[] = $parentObj->address_country;
  }
    
  if($parentObj->host_id) $tagsAr[] = get_user_name($parentObj->host_id); 
  if($cloud_url = $mediaObj->cloud_url) return json_error_object("Already uploaded");

  $siteObj = get_object('site', $site_id);
  $shortname = $siteObj->shortname;
  
  $media_dir = "/web/media/{$shortname}";
  $site_dir = rtrim($siteObj->directory, '/');
  
  $path = $mediaObj->path;
  $src = $mediaObj->src;
  if($mediaObj->resized) $path = "{$path}org/";
  $org_src = file_exists("{$media_dir}{$path}{$src}") ? "{$media_dir}{$path}{$src}" : "{$site_dir}{$path}{$src}";

  if(!file_exists($org_src)) return json_error_object("Cannot upload, {$org_src} not found");
  $src_size = filesize($org_src);
  if(!$src_size) return json_error_object("Cannot upload, {$org_src} is empty");


  if($src_size > $max_file_size) return json_error_object("File too large. {$org_s} > {$max_file_size}");
  
  $tags = array_filter([$mediaObj->name, $parent_name, $parent_owner]);
  $tags = array_merge($tags, array_unique(array_filter($tagsAr)));
  
  $ciOptions = ["public_id" => $mediaObj->id, "tags" => $tags];
  
  $org_s_h = human2bytes($src_size);

  //echo("Uploading {$org_src} ($org_s_h) with options:".dump($ciOptions,true)." config:".dump($config,true));
  //die();
  
  /** all good, send to cloudinary */
  try {
    $result = \Cloudinary\Uploader::upload($org_src, $ciOptions);
  } catch(Exception $e) {
    $errors[] = 'Cloudinary Error: ' .$e->getMessage();
  }
  
  if($result) {
    $time = round(microtime(true) - $time_start,2);
    
    $remote_size = $result['bytes'];    
    $success = 1;
    $messages[] = "OK. Uploaded {$mediaObj->id} ($org_s_h) to {$url} in {$time}s";
    $url = $result['url'];
    $data = ['cloud_url' => $url, 'cloud_data' => json_encode($result)];
    list($id, $errors) = update_object('media', $data, $mediaObj->id);
  } else {
    $errors[] = "Cloudinary returned nil";
  }
  
  $result = new stdClass;
  $result->success = $success;
  $result->filesize = $remote_size;
  $result->error = $errors;
  $result->message = $messages;
  return $result;
}

/** Upload all media matching search to cloudinary */
/** unsused - automatic comment added on  */
function cloudinary_upload($search, $options=[]) {
  $time_start = microtime(true);
  set_time_limit(900); // increase max allowed execution time.
  $errors = $messages = [];

  // why is autoloader not working ?
  
  $max_file_size = 10485760;
  $hard_limit = 40;
  $limit = isset($options['limit']) ? $options['limit'] : $hard_limit;  
  $options['sort'] = 'id desc';
  
  $search['cloud_url'] = '';
  $search['missing_file'] = 0;
  $search['missing_parent'] = 0;
  $search['parent_type'] = ['!=', ''];
  $search['parent_id'] = ['>', 0];
  $search['filesize'] = ['<=', $max_file_size];
  
  $mediaObjs = find_objects('media', $search, $options);
  $count = count($mediaObjs);
  $i = $uploaded = 0;
  foreach($mediaObjs as $mediaObj) {
    $res = cloudinary_upload_media($mediaObj);
    if($res->success) $uploaded++;
    $i++;
    if($limit && $i >= $limit) break;
  }

  $time = round(microtime(true) - $time_start,2);
  $messages[] = "Uploaded {$count} media in {$time}s";
  
  $result = new stdClass;
  $result->success = $count ? 1 : 0;
  $result->count = $i;
  $result->error = $errors;
  $result->message = $messages;
  return $result;
}
<?php

/** mostly old - get rid of, use db_rental */

function property_geoname($aptObj) {
  if((!isset($aptObj->address_country) || !isset($aptObj->address_city)) && $aptObj->id) $aptObj = get_object('property', $aptObj->id);
  $cname = cc_name($aptObj->address_country);
  $name = implode(', ',array_filter([$aptObj->name, $aptObj->address_city, $cname]));
  return $name;
}

function property_geolink($aptObj_or_id, $options=[]) {
  global $__LANGUAGE, $__SITE;
  
  $aptObj = any2obj('property', $aptObj_or_id);
  if(!isset($aptObj->site_id)) $aptObj = get_object('property', $aptObj->id); // incomplete data
  $siteObj = $aptObj->site_id == $__SITE->id ? $__SITE : get_object('site', $aptObj->site_id);
  $path = isset($options['path']) ? $options['path'] : '';
  if(!$path && is_property_type($aptObj->type)) {
    $path = site_page_link($siteObj, PAGE_RENTAL_DETAIL, $options);
  }
  
  $name = property_geoname($aptObj);
  $slugged_name = slugify($name);
  $slug = $slugged_name ? "{$slugged_name}~{$aptObj->id}" : $aptObj->id;
  $link = "{$path}{$slug}";
  $already_full = strpos($path, 'http') !== false ? 1 : 0;
  if(!$already_full && ($aptObj->site_id != $__SITE->id || $options['full'])) {
    $site_url = site_full_url($aptObj->site_id);
    $link = $site_url.$link;
    print_log("full={$already_full}  apt: {$aptObj->site_id} site:{$__SITE->id} adding site_url:{$site_url} link={$link}", 'property', LOG_LEVEL_TEST);
  }

  print_log("path={$path} id={$aptObj->id} name={$name} slug={$slug} link={$link}", 'property', LOG_LEVEL_TEST);

  return $link;  
}


// Checks if a property is ready to be published
function property_remaining_steps($siteObj, $type, $apt_id) {
  $reasons = array();
  $words_required = 15;
  $images_required = 1;  

  $aptObj = get_object($type, $apt_id);
  

  $short_desc = get_object('property', $apt_id, 'short_description');
  $descObj = property_description($siteObj, $apt_id);
  $word_count = count(explode(' ', strip_tags($descObj->description." ".$short_desc)));
  //if($word_count < $words_required) $reasons[] = "Please <a href='#' class='linkref' data-target='#rental-description'>describe</a> your rental using at least $words_required words.";
  if($word_count < $words_required) $reasons[] = "Description: write a short description - at least 15 words.";

  switch($type) {
  case 'shortstay':
    //if(!($aptObj->currency && $aptObj->rate_day)) $reasons[] = "Please <a href='#' class='linkref' data-target='#rental-pricing'>set</a> your the currency and basic rate per night.";
    if(!($aptObj->currency && $aptObj->rate_day)) $reasons[] = "Pricing: set your currency and the basic rate per night.";
    break;
  }
  
  $media = property_images($apt_id) ?: array_filter(explode(',', $aptObj->media));
  $media_count = count($media);
  if($media_count < $images_required) $reasons[] = "Photos: upload at least $images_required photo.";

  //echo("mc=$media_count wc=$word_count<br>".dump($media_count, true).dump($word_count, true)); 
  return $reasons;
}

function hourly_rate($ptype) {
  return find_object('property_type', array('name' => $ptype), 'hourly');
}

/** unsused - automatic comment added on  */
function print_exception_input_fields($ptype, $search, $exceptionObj=NULL, $options=NULL) {
  $form_name = 'add_exception_form';
  $apt_id = $search['apt_id'];
  $host_id = $search['host_id'];
  $aptObj = $search['aptObj'];
  $city_id = $search['city_id'] ?: $aptObj->city_id;
    
  $calparams['minDate'] = "'+0d'";
  $calparams['maxDate'] = "'+3y'";
  $calparams['dependent_field'] = "end";
  $calparams['dependent_field_add_days'] = 1;
  
  $city_name = $aptObj->city_id ? get_object('geo_city', $city_id, 'name') : '';
  $propAr = array($aptObj->id => $aptObj->name, '' => phrase('all_properties', CAPITALIZE));
  $cityAr = array($aptObj->city_id => $city_name, '' => phrase('all_cities', CAPITALIZE));   

  $host_properties = find_objects($ptype, array('host_id' => $host_id), array('fields' => 'id'));
  if(count($host_properties) > 1) {
    $fields['which_property'] = form_radio_buttons('apt_id', array('rows' => $propAr, 'value' => $exceptionObj ? $exceptionObj->apt_id : $aptObj->id));
  } else {
    $fields['property'] = $aptObj->name.form_input('apt_id', 'hidden', $apt_id);
  }
  
  $host_cities = find_objects($ptype, array('host_id' => $host_id), array('fields' => 'distinct city_id'));
  if(count($host_cities) > 1) {
    $fields['which_city'] = form_radio_buttons('city_id', array('rows' => $cityAr, 'value' => $exceptionObj ? $exceptionObj->city_id : $aptObj->city_id));
  } else {
    $fields['city'] = $city_name.form_input('city_id', 'hidden', $city_id);
  }
  
  $fields[phrase('start_date', CAPITALIZE)] = datepicker('start', $calparams, $exceptionObj->start);       

  unset($calparams['dependent_field']);
  unset($options['dependent_field_add']);
  $fields[phrase('end_date', CAPITALIZE)] = datepicker('end', $calparams, $exceptionObj->end);

  if($hourly = hourly_rate($ptype)) {
    $fields[phrase('length_of_stay', CAPITALIZE).' >='] = form_input('num_hours', 'text', $exceptionObj->num_hours, array('size' => 2, 'max' => 4)).' '.phrase('hours');
  } else {
    $fields[phrase('length_of_stay', CAPITALIZE).' >='] = form_input('num_days', 'text', $exceptionObj->num_days, array('size' => 2, 'max' => 4)).' '.phrase('days'); 
    //$fields[phrase('length_of_stay ', CAPITALIZE).' >='] = form_input('num_months', 'text', $exceptionObj->num_months, 2, 4).' '.phrase('months');
  }
   
  $fields[phrase('number_of_guests', CAPITALIZE).' >='] = form_input('num_guests', 'text', $exceptionObj->num_guests, array('size' => 2, 'max' => 4)); 
  $fields[phrase('checkin', CAPITALIZE).' <='] = form_input('days_from_now', 'text', $exceptionObj->days_from_now, array('size' => 2, 'max' => 4))." ".phrase('days_from_now');
  $fields[phrase('checkin', CAPITALIZE).' >='] = form_input('days_from_now_gt', 'text', $exceptionObj->days_from_now_gt, array('size' => 2, 'max' => 4))." ".phrase('days_from_now');

  //$fields[phrase('fit_factor', CAPITALIZE).' >='] = form_input('fit_factor', 'text', $exceptionObj->fit_factor, array('size' => 2, 'max' => 4))." %"; 
  $fields[phrase('fit', CAPITALIZE).' '.phrase('between').' >='] = form_input('fit_between', 'text', $exceptionObj->fit_between, array('size' => 2, 'max' => 4))." %"; 
  $fields[phrase('fit', CAPITALIZE).' '.phrase('next_to').' >='] = form_input('fit_next_to', 'text', $exceptionObj->fit_next_to, array('size' => 2, 'max' => 4))." %"; 

  // hourly
  // $fields[phrase('start_time', CAPITALIZE)] = timepicker('start_time', array(),  $exceptionObj->start_time ?: $aptObj->checkin);
  // $fields[phrase('end_time', CAPITALIZE)] = timepicker('end_time', array(), $exceptionObj->end_time ?: $aptObj->checkout);
  
  return $fields;  
}

/** unsused - automatic comment added on  */
function print_exception_output_fields($ptype, $search, $exceptionObj=NULL, $options=NULL) {
  $form_name = 'add_exception_form';
  $apt_id = $search['apt_id'];
  $host_id = $search['host_id'];
  $aptObj = $search['aptObj'];

  $hourly = hourly_rate($ptype);
  $rate_unit = $hourly ? phrase('hour') : phrase('day'); // todo: handle months ? 
  $rate_unit_plural = $hourly ? phrase('hours') : phrase('days'); // todo: handle months ? 
  $rate_field = $hourly ? 'rate_hour' : 'rate_day';
  
  // priority         | int(6)                           | NO   |     | 0                 |                             |

  $fields[phrase('description', CAPITALIZE)] = form_input('description', 'text', $exceptionObj->description, array('size' => 30, 'max' => 255)); 
  $fields[phrase('can_book_directly', CAPITALIZE)] = yes_no_chooser('book_direct', $exceptionObj->book_direct); // form_checkbox('book_direct', $exceptionObj->book_direct); 
  $fields[phrase('minimum_stay', CAPITALIZE)]      = form_input('min_stay', 'text', $exceptionObj->min_stay, array('size' => 2, 'max' => 4))." ".phrase($rate_unit_plural); 
  $fields[phrase('maximum_stay', CAPITALIZE)]      = form_input('max_stay', 'text', $exceptionObj->max_stay, array('size' => 2, 'max' => 4))." ".phrase($rate_unit_plural); 
  $fields[phrase('maximum_guests', CAPITALIZE)]    = form_input('max_guests', 'text', $exceptionObj->max_guests, array('size' => 2, 'max' => 4)); 
  $fields[phrase('price', CAPITALIZE)]             = form_input($rate_field, 'text', $exceptionObj->rate_day, array('size' => 4, 'max' => 10))." $aptObj->currency / ".phrase($rate_unit); 
  $fields[phrase('weekend_rate', CAPITALIZE)]      = form_input('rate_weekend', 'text', $exceptionObj->rate_weekend, array('size' => 4, 'max' => 10))." $aptObj->currency / ".phrase($rate_unit); 
  $fields[phrase('price_extra_guest', CAPITALIZE)] = form_input('rate_extraperson', 'text', $exceptionObj->rate_extraperson, array('size' => 4, 'max' => 10))." $aptObj->currency / ".phrase($rate_unit); 
  $fields[phrase('price_change', CAPITALIZE)]      = form_input('rate_change', 'text', $exceptionObj->rate_change, array('size' => 4, 'max' => 10))."+/- $aptObj->currency ".phrase('or')." %"; 

  $fields[phrase('last_rule', CAPITALIZE)]      = form_checkbox('last', $exceptionObj->last); 
  //$fields[phrase('discount', CAPITALIZE)]          = form_input('discount', 'text', $exceptionObj->discount, 4, 10)." $aptObj->currency ".phrase('or')." %"; 
  //$fields[phrase('surcharge', CAPITALIZE)]         = form_input('surcharge', 'text', $exceptionObj->surcharge, 4, 10)." $aptObj->currency ".phrase('or')." %"; 


  return $fields;  
}

/** unsused - automatic comment added on  */
function property_gallery_count($aptObj_or_id) {
  $aptObj = any2obj('property', $aptObj_or_id);
  return count_objects('media', array('parent_type' => $aptObj->type, 'parent_id' => $aptObj->id, 'active' => 1, 'descriptiontype_id' => DESC_TYPE_ALBUM));
}

// returns array('shortstay', 'longstay', etc
/** unsused - automatic comment added on  */
function property_types() {
  global $pTypeArray;
  if(is_array($pTypeArray)) return array_values($pTypeArray);  
  return find_objects('property_type', array(), array('field' => 'name'));  
}

function is_property_type($obj_type) {
  if($obj_type == 'property') return true; // do we want this ?

  global $pTypeArray;
  if(is_array($pTypeArray)) return in_array($obj_type, $pTypeArray); // Array lookup
  return find_object('property_type', array('name' => $obj_type)) ? true : false; // DB lookup  
}

// returns default property filters
function property_filters($siteObj_or_id, $options=NULL) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $filters_json = $siteObj->property_filters;
  $ptype = $siteObj->property_type ?: 'shortstay';
  
  $search = ['client_id' => $siteObj->client_id, 'type' => $ptype, 'active' => 1, 'hide' => 0];
  
  if($filters_json) {
    $filters = json_decode($filters_json) ?: [];
    foreach($filters as $k => $v) $search[$k] = $v;
  }
  return $search;  
}
 

// set host id = parent_id of owner, add field to table if needed
/** unsused - automatic comment added on  */
function set_host_id() {
  $execute = $options['execute'] ?: $_GET['execute'];
  $execute = 1;
  $alter = 1;
  $classes = get_classes();
  $count = count($classes);
  echo("There are $count classes. Execute=$execute<br>");
  //dump($classes);die();
  $classAr = array();
  foreach($classes as $class) {
    $obj_type = $class->type;
    $table = get_object_table($obj_type);
    
    if($classAr[$table]) {
      //echo("$table is set: ".implode(',', array_keys($classAr)).html_break());
      continue;  
    }
    $has_owner = $has_host = false;
    $fields = class_fields($obj_type, false);
    if(in_array('owner_id', $fields)) {
      $has_owner = true;
      if(in_array('host_id', $fields)) {
        $has_host = true;
      }
      $classAr[$table] = array('obj_type' => $obj_type, 'has_owner' => $has_owner, 'has_host' => $has_host);
    }
  }
  
  $count = count($classAr);  
  echo("There are $count classes with owner_id<br>");
  $rows = array();
  $rows[] = array("obj_type", "table", "owner", "host");
  foreach($classAr as $table => $ar) {
    $obj_type = $ar['obj_type'];
    $has_owner = $ar['has_owner'];
    $has_host = $ar['has_host'];
    
    //$count = db_getOne("SELECT count(*) from $table WHERE owner_id=$from_site");
    $owner_str = $has_owner ? html_span("Yes", 'green') : html_span("No", 'red');
    $host_str = $has_host ? html_span("Yes", 'green') : html_span("No", 'red');
    if($count) {
      $rows[] = array($obj_type, $table, $owner_str, $host_str);
      if($alter) {
        db_query("ALTER table $table ADD column host_id int(11)");        
      }
      echo("UPDATE $table,owner SET $table.host_id=owner.parent_id WHERE $table.owner_id=owner.id<br>");
      db_query("UPDATE $table,owner SET $table.host_id=owner.parent_id WHERE $table.owner_id=owner.id");
    }
  }
  $output = html_table($rows, array('class' => 'web20'));
  return $output;
}

function get_random_user() {
  $q = "select address_parent_id as id,first_name,last_name,city,state,country from address where address_parent_type = 'user' and first_name != '' and last_name != '' and city != '' and country != '' order by rand() limit 1";
  return db_getRow($q);  
}

function lorem_ipsum($paragraphs = 3,$min=10,$max=30,  $lorem=false) {
  if(!$lorem) {
    $url = "http://www.randomtext.me/api/gibberish/p-$paragraphs/$min-$max";
    // $url = "http://baconipsum.com/api/?type=meat-and-filler&paras=3&start-with-lorem=0";
    $json = file_get_contents($url);
    $obj = json_decode($json);
    return $obj->text_out;

  } else { 
    $content = file_get_contents("http://loripsum.net/api/$paragraphs/short/");
  }
  
  return $content;
}


// add a random review to a property 
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_random_review($siteObj, $aptObj, $options = array()) {
  if(!$aptObj) die("add_random_reviews: no apt");

  $host_id = $aptObj->host_id;
  if(!$host_id) die("No host for $aptObj->name");
  
  $guest = get_random_user();      
  //dump($guest);die();

  $bias = rand(0,4); // 0 = unhappy guest, 4 = perfectly happy
  
  $data = (array) $guest;
  
  $data['language'] = 'en';
  $data['site_id'] = $siteObj->id;
  $data['guest_id'] = $guest->id;
  $data['host_id'] = $host_id;
  $data['apt_id'] = $aptObj->id;
  $data['property_type'] = $aptObj->type;

  $data['parent_type'] = 'dummy';
  $data['parent_id'] = rand(100000,10000000);

  $data['can_publish'] = $data['published'] = 1;

  $data['feedback_comfort']  = $f1 = min(10, $bias + rand(5,10));
  $data['feedback_location'] = $f2 = min(10, $bias + rand(5,10));
  $data['feedback_cleaning'] = $f3 = min(10, $bias + rand(5,10));
  $data['feedback_value']    = $f4 = min(10, $bias + rand(5,10));
  $f = ($f1 + $f2 + $f3 + $f4) / 4;
  $data['feedback_overall']  = $f;
  $data['feedback_recommend']  = $f > 7 ? 1 : 0;
  
  $data['feedback_comments'] = lorem_ipsum(3);

  //dump($data);die();  
  
  return add_object("feedback", $data);
}

// update chats with random user ids, reservation ids, datest
/** unsused - automatic comment added on  */
function random_chats($user_id) {
  $time = time();
  $chatObjs = find_objects("chat", array(), array("sort" => "id desc", "limit" => 0));
  $user_ids = db_getCol("select distinct(user_id) from user_profile where photo > 0 and photo < 1200000 and user_id != $user_id;");
  $res_ids = db_getCol("select distinct(res_id) from reservation where guest_id=$user_id;");  
  $res_count = count($res_ids);
  $user_count = count($user_ids);
  $output .= "count=$user_count".dump($user_ids, true);
  $threads = array();
  
  $factor = 600;
  foreach($chatObjs as $chatObj) {
    $created = date("Y-m-d H:i:s", $time);
    $rand = round(rand(0, $user_count-1));
    $other_user_id = $user_ids[$rand];
    $send = round(rand(0,1));
    if($send) {
      $sender_id = $other_user_id;
      $receiver_id = $user_id;      
    } else {
      $sender_id = $user_id;
      $receiver_id = $other_user_id;
    }
    $thread = min($sender_id, $receiver_id).'-'.max($sender_id, $receiver_id);
    $threads[$thread] = 1;
    $output .= "$chatObj->id: $created rand=$rand send=$send sender=$sender_id receiver=$receiver_id<br>";
    $time -= $factor * rand(0,10) + rand(0, $factor);
    update_object("chat", array("created" => $created, "sender_id" => $sender_id, "receiver_id" => $receiver_id), $chatObj->id);
  }
  
  $q = "update chat set thread = concat(least(sender_id,receiver_id),'-',greatest(sender_id,receiver_id))";
  db_query($q);
  $output .= dump($threads, true);
  foreach($threads as $thread => $val) {   
    $rand = round(rand(0, $res_count-1));
    $res_id = $res_ids[$rand];
    $output .= "Setting $thread reservation to $res_id<br>";
    db_query("UPDATE chat SET parent_type='reservation', parent_id=$res_id WHERE thread='$thread'");
  }
  update_reservation_chat_count();
  return $output;
}

// loops through reservations, add chat objects if extra_info or message is set
/** unsused - automatic comment added on  */
function chat_add_reservation_threads() {
  
  $resObjs = db_getAll("SELECT res_id,guest_id,host_id,created,message,extra_info FROM reservation where property_type='shortstay' and apt_id != 0 and status in (10,15,30,40) and (extra_info != '' or message != '') order by res_id desc");
  $count = count($resObjs);
  if(!$count) return "No reservation found:<br>$qs";  
  $output = html_h3("Adding threads for $count res objects");
  
  foreach($resObjs as $resObj) {
    $res_id = $resObj->res_id;
    $host_id = $resObj->host_id;
    $guest_id = $resObj->guest_id ?: 0;
    if(!$host_id) die("reservation $res_id has no host");
    
    $data = array("parent_id" => $res_id, "parent_type" => "reservation", "created" => $resObj->created);
    //dump($resObj);die();
    if($message = $resObj->message) {
      $data['sender_id'] = $host_id;
      $data['receiver_id'] = $guest_id;
      $data['message'] = $message;
      if($host_id == $guest_id) die("host=guest=$host_id".dump($resObj,true));
      //dump($data);die();
      list($id, $errors) = add_object("chat", $data);      
      if($errors) die(dump($errors, true));
      $output .= "Added chat $id<br>";
    }
    
    if($message = $resObj->extra_info) {
      $data['sender_id'] = $guest_id;
      $data['receiver_id'] = $host_id;
      $data['message'] = $message;
      
      if($host_id == $guest_id) die("host=guest=$host_id".dump($resObj,true));
      //dump($data);die();
      list($id, $errors) = add_object("chat", $data);      
      if($errors) die(dump($errors, true));
      $output .= "Added chat $id<br>";
    }
    
  }
  return $output;
}

// loops through all chat objects, set correct thread
/** unsused - automatic comment added on  */
function chat_set_thread() {
  $chatObjs = find_objects("chat", array("thread" => ''));
  $count = count($chatObjs);
  if(!$count) return "All chats have threads set";  
  $output = html_h3("$count chat objects need a thread");
  foreach($chatObjs as $chatObj) {
    $thread_id = chat_thread_id((array) $chatObj);
    list($id, $errors) = update_object("chat", array('thread' => $thread_id), $chatObj->id);
    if($errors) die(dump($errors, true));
    $output .= "Set thread of chat $id to $thread_id<br>";
    
  }
  return $output;
}

// return a random quote from this host
function host_quote($userObj_or_id) {
  
  $userObj = any2obj('user', $userObj_or_id);
  $host_id = $userObj->id;
  $search = array('active' => 1, 'host_id' => $host_id, "length(quote)" => array('>', 20));
  $searchOptions = array('fields' => array("id", "name", "type", "city_name", "address_city", "city_id", "quote"), 'sort' => 'rand()', 'limit' => 1, 'debug_query' => false);
  $quotes =  find_objects('property', $search, $searchOptions);
  //dump($quotes);
  return count($quotes) ? $quotes[0] : '';
  
}

// return a random quote from any rental
/** unsused - automatic comment added on  */
function rental_quote($count=1) {  
  $host_id = $userObj->id;
  $search = array('active' => 1, 'client_id' => $_SESSION['client_id'], "length(quote)" => array('>', 10), "priority" => array('>', 0));
  $searchOptions = array('fields' => array("id as apt_id", "host_id", "name", "type", "city_name", "address_city", "city_id", "quote"), 'sort' => 'rand()', 'limit' => $count, 'debug_query' => false);
  $quotes =  find_objects('property', $search, $searchOptions);
  
  // require summary, rental description, profile picture
  return count($quotes) == 1 ? $quotes[0] : $quotes;
}

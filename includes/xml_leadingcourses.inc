<?php

/** Begin helpers */
//lc_get_region($obj);

// calls lc_get_region for each continent
function lc_get_all($options = array()) {
  $continents = array(369,370,371,372,373,374,377,378,379);
  $objs = array();
  foreach($continents as $region_id) {
    $options['level'] = 1; // root
    $obj = lc_get_region($region_id, 'root', $options);
    //$output .= html_h4("Regions $i").dump($obj, true);
    $objs[] = $obj;
  }
  return $objs;
}


function lc_get_region($id, $parent_id, $options = array()) {
  global $clubAr;
  extract($options);
  
  if(!$id) die("lc_get_region: missing id");  

  $output = '';
  $clubs = array();
  $db = $options['update'] === 'region' || $options['update'] === 1;
  
  $method = 'regions';
  $params['region_id'] = $id;
  $url = lc_get_url($method, $params);

  $json_url = lc_cache_file("region", $id);  
  //print_log("url: {$url} json:{$json_url}", 'leadingcourses', LOG_LEVEL_ALWAYS);

  $obj = xmlurl2obj($url, array("log_file" => 'leadingcourses', "cache" => $json_url, "debug" => $options['debug'], "live" => $options['live']));

  print_log("\n\nREGION: {$obj->name} ($id)", 'leadingcourses', LOG_LEVEL_ALWAYS);

  $region = $obj->region;
  $id = $region->id;
  if(!$id) die("Region has no ID");
  
  if($level) $region->level = $level;
  
  $output .= html_h2("output from ".$obj->_input_source);
  $name = $region->name;
  if($update && $parent_id) {
    list($db_id, $cc) = lc_add_region($parent_id, $region, $options);
    $region->db_id = $db_id;
    $cc = $db_id ? get_object('golf_region', $db_id, 'country') : '';
    if($cc) $options['cc'] = $cc; // for children
    //die("gh: cc=$cc".dump($region,true));
  } else {
    //die("no add update=$update pid=$parent_id");
    $db_id = $region->db_id = lc_find_region($id);
  }

  $output .= "update=$update parent=$parent_id db_id=$db_id<br>";
  
  //$clubs = $region->nr_clubs;
  $subs = $region->sub_regions ? $region->sub_regions->region : null;      
  if($subs && !is_array($subs)) $subs = array($subs); // just one, then object
  $sub_count = is_array($subs) ? count($subs): 0;

  print_log("Processing Region $db_id ($id). Children: $sub_count", 'leadingcourses', LOG_LEVEL_ALWAYS);
  
  if($recurse && $sub_count) { // not a leaf node
    $children = array();
    foreach($subs as $sub) {
      if($level) $options['level'] = $level + 1;
      $children[] = lc_get_region($sub->id, $db_id, $options); 
    }
    $region->children = $children;
  }
  
  if($club && !$sub_count) { // leaf node, get clubs
    $clubs = $region->club_ids ? $region->club_ids->club_id : null;      
    $club_count = is_array($clubs) ? count($clubs): 0;
    if($clubs && !is_array($clubs)) $clubs = array($clubs); 
    $region->clubs = implode(',', $clubs);
    
    $output .= "Getting clubs: club=$club club_count=$club_count db_id=$db_id<br>"; 

    
    // process clubs - only at leaf node when parent region ID is set
    if($clubs && $db_id) {
      foreach($clubs as $club_id) {
        $output .= "Processing club $club_id in region $db_id<br>";
        print_log("Calling lc_get_club $club_id in region $db_id", 'leadingcourses', LOG_LEVEL_ALWAYS);
        $clubObj = lc_get_club($club_id, $db_id, $options);
      }
    } 
  } else {
    $output .= "No clubs: club=$club subcount=$sub_count<br>"; 
  }
  
  if($echo) echo($output);
  unset($region->club_ids); // for easier printing
  unset($region->sub_regions); // for easier printing
  
  return $region;
}
                              
function lc_find_region($id) {
  if(!$id) die("lc_find_region: no id");
  return find_object('golf_region', array('source_id' => SOURCE_LEADING_COURSES, 'fid' => $id), 'id');
}
 
function lc_add_region($parent_id, $regionObj, $options = array()) {
  $id = $regionObj->id;
  $db_id = lc_find_region($id);
  extract($options);
  
  if($db_id && !$overwrite) return array($db_id, ''); 

  $name = $regionObj->name;
  
  //echo("pid=$parent_id".dump($regionObj,true));die();
  $search = array("source_id" => SOURCE_LEADING_COURSES, "fid" => $id);

  $data = array();
  $data['name'] = $name;
  $data['fid'] = $id;
  $data['source_id'] = SOURCE_LEADING_COURSES;
  $data['parent_id'] = $parent_id == 'root' ? 0 : $parent_id;
  $data['nr_clubs'] = $regionObj->nr_clubs;
  if($level = $regionObj->level) $data['level'] = $level;
  
  if($level == 2) { // country
    $country_map = lc_country_map();
    $cc = $data['country'] = $country_map[$name] ?: find_object('country', array('name' => $name), 'code');;
    if(!$cc) echo("No cc for $name<br>");
    
  } else if($cc = $options['cc']) {
    $data['country'] = $cc; // from parent
  }
  //dump($data);die();
  
  if($db_id) {
    list($db_id, $errors) = update_object("golf_region", $data, $db_id);
    $action = "update";
  } else {
    list($db_id, $errors) = add_object("golf_region", $data);
    $action = "add";
  }
  if($errors) {dump($errors); die();}
  $output .= "$action region $db_id ($id)<br>";

  if($options['echo']) echo($output);
  
  return array($db_id, $cc);
}




function lc_get_club($club_id, $region_id, $options = array()) {
  if(!$club_id) die("lc_get_club: missing id");  

  global $attributeAr;

  $output = "";
  $club_url = lc_get_url("club", array('club_id' => $club_id));
  $json_url = lc_cache_file("club", $club_id);
  if(!$json_url) die("No cache file for club $club_id");
  $clubObj = xmlurl2obj($club_url, array("log_file" => 'leadingcourses', "cache" => $json_url, "debug" => $options['debug'], "live" => $options['live']));
  if(!$clubObj->id) {
    echo("xmlurl2obj returned malformed club for club $club_id:".dump($clubObj,true));
    return null;
  }
  if(!$clubObj) {
    return null;
    die("Couldn't get club $club_id from $club_url: cache=$json_url");
  }
  
  $db = $options['update'] == 'club' || $options['update'] == 1;
  //echo("db=$db options:".dump($options,true));die();
  print_log(" Processing Club $club_id in region $region_id db=$db", 'leadingcourses', LOG_LEVEL_ALWAYS);
  if($db && $region_id) {
    $db_id = lc_add_club($clubObj, $region_id, $options);
    $clubObj->db_id = $db_id;
  }
  return $clubObj;  
}


// http://php.net/manual/en/function.floatval.php
// This function takes the last comma or dot (if any) to make a clean float, 
// ignoring thousand separator, currency or any other letter :
function lc_add_club($clubObj, $region_id, $options = array()) {
  global $courseAr;
  global $courseAttAr;
  print_log(" - Add/Edit Club $club_id in region $region_id", 'leadingcourses', LOG_LEVEL_ALWAYS);

  $id = $clubObj->id; 
  if(!$id) {
    die("lc_add_club: No club id:".dump($clubObj, true).caller_stack());
  }

  // lc_remove_duplicates($clubObj->id);
  
  // figure out whether to return or process
  $field = 'address_country'; // custom check for missing fields
  $dbObj = lc_find_club($clubObj->id);
  
  $db_id = $dbObj->id;
  $db_cc = $dbObj->address_country;
  
  if($db_id && !$dbObj->active) {
    print_log(" - Skipping club $db_id as it has been de-activated", 'leadingcourses', LOG_LEVEL_ALWAYS);
    return $db_id;
  }
  
  $attributes = $clubObj->attributes ? $clubObj->attributes->attribute : array();
  if($attributes && !is_array($attributes)) $attributes = array($attributes); // just one, then object
  $data = lc_club_attribute_map($attributes);

  $cc = $options['cc'];
  $region_cc = $cc ? null : get_object('golf_region', $region_id, 'country');
  if(!$cc) $cc = $region_cc;
  
  if(!$cc) {
    
    echo("data: ".dump($data,true));
    echo("region cc: $region_cc");
    die("no cc for $id in region $region_id options:".dump($options, true));
  }
  $data['currency'] = $cur_code = country2currency($cc) ?: '';
  
  //echo("processing $id: db_id=$db_id field=$field cc=$cc db_cc=$db_cc<br>");
  if($fee = $data['fee_weekend'] ?: $data['fee_weekday']) {
    $currency = preg_replace('/[0-9\.\,]+/','',$fee); // strip away price 
    $currency = trim($currency, chr(0xC2).chr(0xA0)); // strip nbsp \u00a0
    
    $data['currency_utf'] = $currency; 
    $data['currency_html'] = $cur_html = unicode2html($currency); 
    if(!$currency) {
      dump($data);
      die("no currency in $fee");
    }
    //lc_currency_map($currency);
    //if($db_id) {
    //  db_query("UPDATE golf_club SET currency='$cur_code', currency_utf='$currency', currency_html='$cur_html' WHERE id=$db_id");
    //  return $db_id;
    //}
  }
  
  $mapAr = lc_club_map();
  //dump($data);
  
  $data = array();
  $data['address_country'] = $cc;
  
  if($source_ts = $clubObj->_timestamp) $data['source_updated'] = $source_ts;
  else die("No ts".dump($clubObj,true));
  
  // add data from object
  foreach($clubObj as $k => $v) {
    $key = $mapAr[$k];
    if($key && strlen($v)) $data[$key] = $v;
  }
  
  // child objects: attributes, courses, reviews, media
  $courses = $clubObj->courses ? $clubObj->courses->course : array();
  if($courses && !is_array($courses)) $courses = array($courses); // just one, then object

  // add course names to club object; copied to location (alt_names) and used to detect changes  
  if(count($courses)) {
    $courseAr = array();
    foreach($courses as $course) {
      if($course->name) $courseAr[] = $course->name;
    }
    if($course_names = implode(",", array_unique($courseAr))) {
      $data['golf_courses'] = $course_names; 
    }    
  }

  
  $images  = $clubObj->clubimages ? $clubObj->clubimages->clubimage : array();
  if($images && !is_array($images)) $images = array($images); // just one, then object
  $image_count = $data['image_count'] = count($images);

  /**
  if($image_count && $db_id && !$clubObj->image_count) {
    //db_query("UPDATE golf_club SET image_count=$image_count where id=$db_id");
    //echo("UPDATE golf_club SET image_count=$image_count where id=$db_id<br>");
  } else {
    //echo("ic=$image_count db=$db_id club=".$clubObj->image_count.html_break());
    //dump($clubObj);
    //dump($images);
    //die();
  }
  */
   
  $image_json = json_encode($images);
  $data['media_json'] = str_replace('200x150', '800x600', $image_json); // dirty little trick to get usable images
  $data['feedback_count'] = $data['review_count'];
  
  if($options['download'] && $image_count) {
    if(!$dbObj->media) $overwrite = $options['overwrite'] = 'yes';    
    //echo("Download: $image_count overwrite=$overwrite<br>");
  } else {
    $imageAr = array();
    foreach($images as $image) $imageAr[] = $image->filename;
    
    $data['media'] = implode(",", $imageAr);
    //echo("No Download: ".dump($options, true));
  }
  
  $reviews = $clubObj->reviews ? $clubObj->reviews->review : array();
  if($reviews && !is_array($reviews)) $reviews = array($reviews); // just one, then object


  $data['golf_region_id'] = $region_id;
  $data['description'] = $clubObj->attributes->description;
    
  if($attributes) { // just for dump readability
    unset($clubObj->attributes->attribute);
    unset($clubObj->attributes->description);
  }

  
  //$data = lc_club_map($clubObj); // map attributes
  $data['source_id'] = SOURCE_LEADING_COURSES;
  if($data['fee_weekday']) $data['fee_weekday'] = toFloat($data['fee_weekday']);
  if($data['fee_weekend'])  $data['fee_weekend'] = toFloat($data['fee_weekend']);
  if($data['review_score'])  $data['review_score'] = toFloat($data['review_score']) ?: 0;

  if($data['season_start'])  $data['season_start'] = human2sql($data['season_start']);
  if($data['season_end'])  $data['season_end'] = human2sql($data['season_end']);
  
  //die("add club id=$clubObj->id, region=$region_id".dump($data,true).dump($clubObj,true));

  if($db_id && $db_cc && !$options['overwrite']) {
    //echo("not processing $id: db_id=$db_id field=$field cc=$cc db_cc=$db_cc<br>");
    return $db_id;
  }
  
  //echo("DB ID: {$db_id}".dump($data,true));die();
  
  if($db_id) { // exists
    list($db_id, $errors) = update_object("golf_club", $data, $db_id);
    $action = "update";
  } else { // new
    list($db_id, $errors) = add_object("golf_club", $data);
    $action = "add";
  }
  if($errors) {
    echo("Failed to $action club $id:".dump($errors,true).dump($data, true).dump($clubObj, true)); 
    die();
  }
   
  print_log(" - $action club $db_id ($id)", 'leadingcourses', LOG_LEVEL_ALWAYS);
  $output = "$action club $db_id ($id)<br>";
  //die($output);
  
  if($db_id && count($courses)) {
    $courseMap = lc_course_map();
    // Add courses
    foreach($courses as $course) {
      $courseData = array();
      //$dummy = array();
      $courseData['fid'] = $fid = $course->id;
      $courseData['name'] = $course->name ?: $clubObj->name;
      $courseData['golf_club_id'] = $db_id;
      $courseData['source_id'] = SOURCE_LEADING_COURSES;
      $courseData['currency'] = $data['currency'];
      $courseData['source_updated'] = $clubObj->_timestamp;
      
  
      foreach($course as $k => $v) {
        if($k == 'attributes') {
          $attributes = $course->attributes->attribute;
          foreach($attributes as $attribute) {
            $attName = $attribute->name;
            $attValue = $attribute->value;
            if(!strlen($attValue)) continue;            
            $fixed_key = lc_fix_key($attName);

            //$dummy[$fixed_key] = $attValue; // for debugging only
            if($key = $courseMap[$fixed_key]) {            
              if(!in_array($key, array('architect', 'description'))) $attValue = unicode2html($attValue);
              
              if(strpos($key, 'fee_') === 0) $attValue = toFloat($attValue);
              else if(strpos($key, 'length_') === 0 || strpos($key, 'number_') === 0) $attValue = intval($attValue);
              
              $courseData[$key] = $attValue; 
            }
          }
          $courseAttAr = lc_add_attributes($courseAttAr, $attributes);
        } else {
          $courseAr[$k]++;
        }
      }
      $db_course_id = find_object('golf_course', array('source_id' => SOURCE_LEADING_COURSES, 'fid' => $fid), 'id');
      if($db_course_id) { // exists
        list($db_course_id, $errors) = update_object("golf_course", $courseData, $db_course_id);
        $action = "update";
      } else { // new
        list($db_course_id, $errors) = add_object("golf_course", $courseData);
        $action = "add";
      }
      $output .= "$action course $id ($db_course_id)<br>";
      if($errors) {
        echo("Failed to $action course $id:".dump($errors,true).dump($courseData, true).dump($clubObj, true)); 
        die();
      }
    }
    
  }
  
  if($db_id && count($reviews)) {
    foreach($reviews as $review) {
      $review_id = find_object('feedback', array('source_id' => SOURCE_LEADING_COURSES, 'fid' => $review->review_id), 'id');
      
      if($review_id) {
        $output .= "found review ".$review->review_id.html_break();
        //echo("found review ".$review->review_id.html_break());
        // do nothing for now
      } else {
        //echo("did not found review ".$review->review_id).html_break();
        //die("did not find found review ".$review->review_id);
        $feedbackMap = lc_review_map();
        $feedbackData = map_array((array) $review, $feedbackMap);
        $feedbackData['source_id'] = SOURCE_LEADING_COURSES;
        $feedbackData['parent_type'] = "golf_club";
        $feedbackData['parent_id'] = $db_id;
        $feedbackData['can_publish'] = $feedbackData['published'] = 1;
        
        list($review_id, $review_errors) = add_object('feedback', $feedbackData);
        if($review_errors) {echo("Add feedback error: ".dump($review_errors,true).dump($feedbackData, true)); die();}
        else $output .= "Added review $review_id<br>";
      }
    }
  }

  if($options['echo']) echo($output);

  return $db_id;  
}


function lc_get_url($method, $params, $options = array()) {
  $lang = $options['language'] ?: 'en';
  $user = 'hitabrick';
  $pwd = '8YWLMLDZBT';
  $params_str = array2qs($params);
  return "https://$user:$pwd@xml.leadingcourses.com/$lang/$method.xml?$params_str";
}

function lc_cache_file($method, $id, $options = array()) {  
  $lang = $options['language'] ?: 'en';
  if(!$_SESSION['cache_dir']) die("lc_cache_file: No cache dir specified");
  $cache_file = $_SESSION['cache_dir']."leadingcourses/$method-$id-$lang.json";
  return $cache_file;  
}

function lc_find_club($club_id, $options = array()) {
  $search = array("source_id" => SOURCE_LEADING_COURSES, "fid" => $club_id);
  return find_object('golf_club', $search);
}

function lc_remove_duplicates($club_id, $options = array()) {
  $search = array("source_id" => SOURCE_LEADING_COURSES, "fid" => $club_id);
  $club_ids = find_objects('golf_club', $search, array("field" => "id"));
  $count = count($club_ids);
  asort($club_ids);
  $id_str = implode(",", $club_ids);
  if($count > 1) {
    array_shift($club_ids);
    echo("found duplicates of fid=$club_id ids=$id_str<br>");
    db_query("DELETE FROM golf_club WHERE id IN ($id_str)");
    db_query("DELETE FROM golf_course WHERE golf_club_id IN ($id_str)");
    db_query("DELETE FROM location WHERE parent_id='golf_club' AND parent_id IN ($id_str)");
    //die();
  }
    
}


function lc_review_map() {
  return array(
    'review_id' => 'fid',
    'review_rating' => 'feedback_overall',
    'review_motivation' => 'feedback_comments',
    'review_insert_time' => 'created',
    'review_url' => 'source_url',
    'review_insert_time' => '',
    //'user_username' => '',
    'user_name' => 'first_name',
  );    
}

     
function lc_country_map() {
  return array(
  'United States of America' => 'US',  
  'The Gambia' => 'GM',  
  'Kazakhstan' => 'KZ',  
  'Bonaire' => 'AN',  
  'Curaçao' => 'AN',  
  'Sint Maarten' => 'AN',  
  'The Bahamas' => 'BS',  
  'United States Virgin Islands' => 'VI',  
  'Republic of Ireland' => 'IE'
  );
}

function lc_course_map() {
  return array(
    'green_fee_weekdays_high_season'=> 'fee_weekday',
    'green_fee_weekends_high_season'=> 'fee_weekend',
    'green_fee_weekdays_low_season'=> 'fee_weekday_low',
    'green_fee_weekends_low_season'=> 'fee_weekend_low',
    'course_type'=> 'course_type',
    'par_men_yellow'=> 'par_men',
    'par_ladies_red'=> 'par_women',
    'course_rating_men_yellow'=> 'rating_men',
    'course_rating_ladies_red'=> 'rating_women',
    'slope_rating_men_yellow'=> 'slope_men',
    'slope_rating_ladies_red'=> 'slope_women',
    'architect'=> 'architect',
    'number_of_holes'=> 'number_holes',
    'length_ladies_red'=> 'length_men',
    'length_men_yellow'=> 'length_women',  
    'golf_course_description'=> 'description',  
    'signature_hole'=> 'signature_hole',  
  );
}

function lc_currency_map($currency) {
  switch($currency) {
  case '&euro;':
    return 'EUR';
  default:
    return $currency;
  }
}

function lc_club_attribute_map($attributes) {
  $facs = $data = $unused = array();
  $mapAr = lc_club_map();
  $facAr = lc_club_facs();
  foreach($attributes as $attribute) {
    $attName = $attribute->name;
    $attValue = $attribute->value;
    if($fac_id = $facAr[$attName]) { // this is a facility
      if(strtolower($attValue) == 'yes') $facs[] = $fac_id; 
    } else if($key = $mapAr[$attName]) { // this is an object member
      $utf8 = in_array($key, array('address_formatted', 'name', 'description', 'fee_weekday', 'fee_weekend'));
      if(strlen($attValue)) $data[$key] = $utf8 ? $attValue : unicode2html($attValue);
    } else {
      $unused[$attName] = $attValue;
    }
  }
  //echo("gh");
  //echo("facs".dump($facs,true));
  
  asort($facs);
  $data['facilities'] = implode(',', $facs);
  return $data;
  
}

function lc_club_map() {
  $mapAr = array(
    /** in attributes */
    'from' => 'season_start', // 17 (21 May 2015...) 
    'to' => 'season_end', // 17 (24 May 2015...) 
    'green fee weekdays' => 'fee_weekday', // 17 (€ 20.00...) 
    'green fee weekends' => 'fee_weekend', // 17 (€ 20.00...) 
    'book a tee time' => 'booking_url', // 187 (https://en.leadingcourses.com/clickout/?origin=hitabrick&type=club&subtype=book_...) 

    'founded' => 'founded', // 10 (1995...)
    'payment options' => 'payment_options', // 72 (cash, Maestro, Mastercard, Visa...) 
    'formerly known as' => 'formerly_known_as', // 78 (Limburg Golf Houthalen...) 

    'handicap (men)' => 'handicap_men', // 5 (45...) 
    'handicap (ladies)' => 'handicap_women', // 5 (45...) 

    'visiting address' => 'address_formatted', // 119 (Carretera General numero 3, La Cortinada - Ordino, Andorra...) 
    'website' => 'address_web', // 573 (https://en.leadingcourses.com/clickout/?origin=hitabrick&type=club&subtype=websi...) 
    'telephone number' => 'phone', // 36 (+376 850 400...) 
    'fax number' => 'fax', // 73 (+376 850 400...) 
    'email address' => 'email_address', // 49 (info@ordinogolfclub.com...) 
    'facebook page' => 'facebook', // 244 (https://en.leadingcourses.com/clickout/?origin=hitabrick&type=club&subtype=faceb...) 
    'twitter' => 'twitter', // 305 (https://en.leadingcourses.com/clickout/?origin=hitabrick&type=club&subtype=twitt...) 

    /** END in attributes */    
    /** In object */
    'id' => 'fid',
    'name' => 'name',
    'latitude' => 'location_lat',
    'longitude' => 'location_long',
    'avg_rating' => 'review_score',
    'nr_reviews' => 'review_count',
    'nr_courses' => 'nr_courses',
    'url' => 'source_url',    
    /** End In object */ 

  );
  return $mapAr;
}

function lc_club_facs() {
  $facAr = array(
    'bar'                                    => 920,
    'buggy (power cart) available'           => 921,
    'caddie'                                 => 922,
    'charging system for trolley'            => 923,
    'child care'                             => 924,
    'children\'s playground'                 => 925,
    'chipping area'                          => 926,
    'compact course'                         => 927,
    'conference rooms'                       => 928,
    'driving range'                          => 929,
    'electric trolley for hire'              => 930,
    'golf clubs for hire'                    => 931,
    'handicap required'                      => 932,
    'locker room'                            => 933,
    'pro shop'                               => 934,
    'putting green'                          => 935,
    'restaurant'                             => 936,
    'swimming pool'                          => 937,
    'teaching pros'                          => 938,
    'trolley (pull cart/trundlers) for hire' => 939,
    'wi-fi'                                  => 940
  );
                                                                            
  return $facAr;
}

//$output .= html_h4(count($clubAr)." clubs"); //.dump($clubAr, true);
if(count($attributeAr)) {
  $output .= html_h4(count($attributeAr)." club attributes");
  $output .= lc_analyze_attributes($attributeAr);
}

if(count($courseAttAr)) {
  $output .= html_h4(count($courseAttAr)." course attributes");
  $output .= lc_analyze_attributes($courseAttAr);
}

if(count($courseAr)) {
  $output .= html_h4(count($courseAttAr)." course members");
  $output .= dump($courseAr,true);
}


/** Below used for analyzing data, not needed after live */


//$output .= dump($attributeAr, true);


function lc_add_attributes($attAr, $attributes) {
  foreach($attributes as $attribute) {
    $attName = $attribute->name;
    $attValue = $attribute->value;
    if(strlen($attValue)) {
      $attAr[$attName][$attValue]++;
      $attAr[$attName]['_count']++;
      //echo("Adding $attName $attValue<br>");
    }
  }
  return $attAr;
}

function lc_analyze_attributes($attributeAr) {
  $facAr = array();
  $attAr = array();
  foreach($attributeAr as $name => $values) {
    $keys = '';
    $count = $values['_count'];
    unset($values['_count']);
    $vcount = count($values);
    if($vcount <= 2) {
      $keys = array_keys($values);
      asort($keys);
      $keys = implode(",", $keys);
      if(in_array($keys, array('yes', 'no', 'no,yes'))) {
        $facAr[] = strtolower($name);
        unset($attributeAr[$name]);
      }
      //$output .= dump($values, true);
    } else {
      $example = array_keys($values)[0];
      $lengths = array_map('strlen', array_keys($values));
      $key = lc_fix_key($name);
      $attAr[$key] = max($lengths)." (".substr($example,0,80)."...) " ;
      //$output .= "$name: count=$count values=$vcount".html_break();
      //if($vcount < 100) $output .= dump($values, true);
    }
  }
  asort($facAr);
  $output .= "Facs:".dump($facAr, true);
  
  asort($attAr);
  $output .= "Other attributes:".dump($attAr, true);
  return $output;
}

// replace spaces, remove parentheses from key
function lc_fix_key($name) {
  return  str_replace(array(' ','(',')'), array('_','',''), strtolower($name));
}


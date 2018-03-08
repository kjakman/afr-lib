<?php

function rental_link($aptObj_or_id, $options=[]) {
  return property_geolink($aptObj_or_id, $options);
}

function rental_fees($aptObj_or_id) {
  $aptObj = any2obj('shortstay', $aptObj_or_id);
  $types = ['cleaning' => ['type' => 150, 'description' => "Cleaning Fee"]];
  $rows = [];
  $total = 0;
  foreach($types as $k => $row) {
    $key = "fee_{$k}";
    $val = isset($aptObj->$key) ? $aptObj->$key : 0;
    if($val > 0) {
      $row['quantity'] = 1;
      $row['amount'] = $row['total'] = $val;
      if($aptObj->charge_tax) $row['tax'] = tax_category($row['type']);
      $rows[] = $row;
      $total += $val;
    }
  }  
  if(!count($rows)) $rows='';
  return [$total, $rows];
}

// returns id of nearest city in database given country and coordinates
function rental_find_city($country_code, $aptObj) {
  
  $lat = $aptObj->location_lat;
  $long = $aptObj->location_long;
  $cc = $aptObj->address_country;
  $add_city = $aptObj->address_city;
  
  if(!($lat && $long && $cc)) return 0;
  
  $cityObjs = location_find_nearest('geo_city', $lat, $long, array("radius" => 1000, "limit" => 4, "search" => array("country_code" => $cc)));
  $cityAr = [];
  foreach($cityObjs as $cityObj) $cityAr[$cityObj->id] = $cityObj->name;
  $city_str = implode(',', array_values($cityAr));
  $count = count($cityObjs);
    
  if(!$count) return 0;
    
  $city = null;
  if($add_city) {
    foreach($cityObjs as $cityObj) { // look for city matching name of entered address
      if($add_city == $cityObj->name || $add_city == $cityObj->asciiname || strpos($cityObj->alt_names, $add_city) !== false) {
        $city = $cityObj;
        break;
      }
    }
  }
  if(!$city) $city = $cityObjs[0]; // not found or no city in address, use nearest
  return $city->id;
}


/** tmp for conversion */
/** unsused - automatic comment added on  */
function fees2json($client_id, $limit=0) {
  $search = ['client_id' => $client_id, 'fee_json' => '', 'status' => [STATUS_PROPOSED,STATUS_RESERVED,STATUS_COMPLETE], 'fees' => ['>', 0]];
  $search['res_id'] = ['>', 43000];
  $options = ['sort' => 'res_id desc'];
  
  $resObjs = find_objects('reservation', $search, $options);
  $count = count($resObjs);
  $output .= html_h2("found $count reservations");
  $i=0;
  foreach($resObjs as $resObj) {
    $json = fee2json($resObj);
    if($json) update_object('reservation', ['fee_json' => $json], $resObj->res_id);
    $fees = $resObj->fees;
    $output .= "{$resObj->res_id}: Fees={$fees} json={$json}<br>";
    if($limit && ++$i >= $limit) break;
  }
  return $output;
}

/** tmp for conversion */
function fee2json($resObj) {
  $low_rate = [10,20,30,50,60,70,80,90,100,170,181,182];
  $hi_rate = [110,120,140,150];
  $inv_id = booking_invoice($resObj, 'invoice.id');
  
  $keys = ['service' => 110,'cleaning' => 150,'local' => 160 ,'booking' => 140, 'other' => 180,'pickup'=>170];
  $feeAr = [];
  foreach($keys as $k => $id) {   
    $key = "fee_{$k}";
    $fee = $resObj->$key;
    $desc = '';
    if($fee > 0) {
      $tax = 'H';
      if(in_array($id, $low_rate)) $tax = 'L';
      if(in_array($id, $hi_rate)) $tax = 'H';
      
      if($inv_id) {
        if($liObj = find_object('inv_lineitem', ['inv_id' => $inv_id, 'type' => $id])) {
          $tax = $liObj->tax_category;
          $desc = $liObj->description;
          $quantity = $liObj->quantity; // use invoice
          $amount = $liObj->amount; // use invoice
          $total = $liObj->total; // use invoice
          
          if($liObj->total != $fee) {           
            echo("{$key} mismatch li:{$liObj->id} res_id: {$resObj->res_id} inv_id:$inv_id mismatch: li={$liObj->amount}  res={$fee}<br>");
          }
        } else {
          echo("Could not find lineitem for type {$key} ($id) for res_id: {$resObj->res_id} inv_id = $inv_id using tax={$tax}<br>");
          $quantity = 1;
          $amount = $total = $fee;
          //return '';
          
        }        
      } else {
        echo("Could not invoice for res_id: {$resObj->res_id} inv_id = $inv_id<br>");
        return '';
      }
      $feeAr[] = ['type' => $id, 'amount' => $amount, 'quantity' => $quantity, 'total' => $total, 'tax' => $tax, 'description' => $desc];
    }
  }
  return json_encode($feeAr);
}


/** used by cronscript to import rental calendars */
function calendar_import() {
  require_once("xml_airbnb.inc");
  $today = today();
  $result = new stdClass;
  db_query("DELETE FROM reservation WHERE source_id > 0 AND res_id < 0 AND checkout <= '$today'");
  //db_query("DELETE FROM reservation,property WHERE reservation.source_id > 0 AND reservation.res_id < 0 AND property.active=0 AND reservation.apt_id=property.id");
  
  $importObjs = find_objects('calendar_import', []);
  
  foreach($importObjs as $importObj) {
    $source_id = $importObj->id;
    $apt_id = $importObj->apt_id; 
    if(!$apt_id) {
      $errors[]= "{$op}: Source has no rental ID";
      continue;
    }
    
    $url = $importObj->url;
    if(!is_url($url)) {
      $errors[]= "{$op}: {$url} is not valid";
      continue;
    }
    
    $fname = $importObj->fname;
    $map = $fname ? array($fname => $apt_id) : array(); 
    $res = airbnb_import($apt_id, $url, $map);    
    if($res->success) {
      update_object("calendar_import", array("last_sync" => now()), $source_id);
      $messages[] = "OK: Imported calendar for {$apt_id}";
    } else {
      $error = $res->error ?: "Error: import failed for {$apt_id}";
      $errors = array_merge_smart($errors, $error); 
    }
  }
  $result->success = 1;
  $result->message = $messages;
  $result->error = $errors;
  return $result;
}


// returns array of dummy resObjs with blocked dates
function rental_blocked_dates($aptObj_or_id, $in, $out) {
  $apt_id = any2id('property', $aptObj_or_id);
  $slice = calendar_availability_slice($aptObj_or_id, $in, $out);
  if($slice === false) return [];

  $blocked = [];
  $chars = str_split($slice);

  $cin = 0;
  $date = $in;
  foreach($chars as $char) {
    //echo("{$date}: {$char}<br>");
    if(!$cin && $char == 'N') {
      $cin = $date;
    } elseif($cin && $char != 'N') {
      $blocked[] = array2obj(['checkin' => $cin, 'checkout' => $date, 'status' => STATUS_BLOCKED, 'apt_id' => $apt_id]);
      $cin = 0;
    } 
    $date = next_day($date);
  }

  if($cin) $blocked[] = array2obj(['checkin' => $cin, 'checkout' => $date, 'status' => STATUS_BLOCKED, 'apt_id' => $apt_id]);  // terminate if open
  
  return $blocked;
}

/** returns 1 for available, 0 for not available, false if not known */
function calendar_availability($aptObj_or_id, $in, $out) {
  $slice = calendar_availability_slice($aptObj_or_id, $in, $out);
  if(0) echo("slice from {$in} - {$out}: {$slice} for ".any2id('property', $aptObj_or_id)."<br>");
  if($slice === false) return false;
  return strpos($slice, "N") === false ? 1 : 0;  
}

/** returns slice of availability for given dates, or false if not found for given range, or range is not valid */
function calendar_availability_slice($aptObj_or_id, $in, $out) {
  $aptObj = any2obj('property', $aptObj_or_id);
  $start = $aptObj->calendar_startdate;
  $avail = $aptObj->calendar_availability;
  $len = strlen($avail);
  if(!$start || !$len) return false; // missing
  
  $today = today();
  $last_month = add_day($today, -30);
  $in = date2sql($in);
  $out = date2sql($out);
  $offset = day_diff($start, $in); // how many days ago
  $nd =  day_diff($in, $out);
  $sum = $offset + $nd;
  
  if($start < $last_month) return false; // updated too long ago
  if($len < $sum) return false; // avail. string not long enough
  if(!$in || !$out || $in >= $out) return false; // invalid in/out
 
  if($in < $start) {
    $prefix = $in < $start ? str_repeat("X", -1 * $offset) : '';
    $avail = $prefix.$avail;
    $slice = substr($avail, 0, $nd);
  } else {
    $slice = substr($avail, $offset, $nd);
  }
  
  if(0) { /** debug */
    echo("in:$in out:$out start:$start len:$len sum:$sum<br>prefix:{$prefix} last_month:{$last_month}<br>"); 
    echo("offset:$offset nd:$nd<br>");
    echo("avail:$avail<br>");
    echo("slice:$slice<br>");
    echo("len:".strlen($slice)." nd=$nd<br><br>");
  }
  
  return $slice;
}


      
/** new version */
function search2res($searchObj) {
  global $__SITE;
  $siteObj = $__SITE;
  if(is_array($searchObj)) $searchObj = array2obj($searchObj);
    
  $apt_id = $searchObj->apt_id;
  $ptype = pick_first($searchObj->property_type, $searchObj->t, $siteObj->property_type, 'shortstay');
  $aptObj = get_object($ptype, $apt_id);

  $resData = [
    "client_id" => $siteObj->client_id, 
    "site_id" => $siteObj->id, 
    "city_id" => $aptObj->city_id,
    "host_id" => $aptObj->host_id, 
    "property_type" => $ptype, 
    "apt_id" => $apt_id,
    "apartment_name" => $aptObj->name,
    "num_guests" => $searchObj->ng,
    ];
  
  $priceObj = price_breakdown($aptObj, $searchObj);
  if($priceObj->error) $resData['error'] = $priceObj->error;
  
  $flds = ['average' => 'rate_base', 'service_fee' => 'fee_service'];
  foreach($flds as $k => $v) $resData[$v] = $priceObj->$k;
  if($priceObj->average_discounted > 0) $resData['rate_base'] = $priceObj->average_discounted;
  
  $flds = ['checkin', 'checkout', 'num_days', 'num_months', 'num_guests', 'currency', 'rate_type', 'discount', 'city_tax', 'fee_cleaning', 'city_tax', 'grand_total'];
  foreach($flds as $k) {
    if(isset($priceObj->$k)) $resData[$k] = $priceObj->$k;
  }
  
  $res_type = $aptObj->book_direct || $priceObj->book_direct ? RES_TYPE_DIRECT : RES_TYPE_REQUEST; 
  $resData["type"] = $res_type;

  return $resData;
}


function rental_search($search, $options = []) {
  global $__SITE, $__CLIENT_ID, $__LANGUAGE;
  $cutoff = 760; /** max checkout date for search, corresponding to length of property->calendar_availability - property->calendar_startdate */ 
  $hard_limit = 200; /** max rentals fetched in search (total count) */
  $page_limit = 18;  /** rentals returned for page */
  $max_price = 1000;
  
  $errors = $messages = [];
  $data = new stdClass;  
  
  $site_id = $__SITE->id;

  print_log("\n\nrental_search...", 'rental-search', LOG_LEVEL_TEST);
  //timer('property-search', 'property_search array');
  
  //$price_fld = property_price_field($ptype);
  //if($searchObj->$price_fld) $searchObj->$price_fld = str_replace(';', '-', $searchObj->$price_fld); // new slider uses ; instead of -

  
  $searchObj = is_object($search) ? $search : array2obj($search);
  //dump($searchObj);
  $property_type = pick_first($searchObj->t, $__SITE->property_type, 'shortstay');
  
  $searchArray = [];
  $searchArray['hide'] = 0; 
  $searchArray['type'] = $property_type; 
  $searchArray['active'] = 1; 
  //$searchArray['client_id'] = $__CLIENT_ID; 
  
  if($site_filters_json = get_object('site', $site_id, 'property_filters')) {
    $siteFilters = json_decode($site_filters_json, true);
    if(is_array($siteFilters)) $searchArray = array_merge($searchArray, $siteFilters);
  }
    
  $fields = array('id', 'site_id', 'type', 'name', 'priority', 'currency', 'address_city', 'address_country',
                  'rate_day', 'rate_weekend', 'rate_week', 'rate_2_weeks', 'rate_3_weeks', 'rate_month', 
                  'discount_week','discount_2_weeks','discount_3_weeks','discount_month', 'min_stay',
                  'min_stay', 'rate_change', 'changeover', 'max_guests', 'location_lat', 'location_long', 'review_score', 'review_count', 
                  'media', 'media_json','guid', 'source_id', 'book_direct', 'calendar_startdate', 'short_description');

  
  $searchOptions['sort'] = $sort = $searchObj->sort = property_sort($property_type);
  $searchOptions['fields'] = $fields; 
  $searchOptions['limit'] = $hard_limit; 

  /** guests */
  $searchObj->ng = $ng = $searchObj->ng ?: 1;
  if($ng > 1) $searchArray['max_guests'] = ['>=', $ng];
    
  /** location - we only care about bounds - preprocessing of center/location/loc_id/geocoding must take place in controller */
  if($bounds = $searchObj->bounds) {
    $bounds = any2bounds($bounds, true);
    print_log("rental loc-search bounds=".dump($bounds,true), 'property-search', LOG_LEVEL_TEST);
    $searchArray = bounds2search($bounds, $searchArray); // accepts any bounds format
    //print_log("rental loc-search search=".dump($searchArray, true), 'property-search', LOG_LEVEL_TEST);
  }
  
  /** price:  should perhaps do nothing, we need to look at actual rates after exceptions ? */
  if($p = $searchObj->p) {
    list($min, $max) = explode(';', $p);
    if($max >= $max_price) $max = 0;
    if($min && $max) $searchArray['rate_day'] = ['><', $min, $max];
    elseif($min) $searchArray['rate_day'] = ['>=', $min];
    elseif($max) $searchArray['rate_day'] = ['<=', $max];
  }
  
  /** bedrooms, rooms, property subtype */
  if($pt = $searchObj->pt) $searchArray['hosting_property_type'] = $pt;
  if($nr = $searchObj->nr) $searchArray['num_bedrooms'] = ['>=', $nr];
  if($nb = $searchObj->nb) $searchArray['num_bathrooms'] = ['>=', $nb];  
 
  /** amenitities */
  if($facs = $searchObj->fac) $searchArray['facilities'] = '*'.str_replace(['|', ','], '*', $facs).'*';
  
  /** dates - availability search */
  $in  = $searchObj->in ? date2sql($searchObj->in) : '';
  $out = $searchObj->out ? date2sql($searchObj->out) : '';

  if($in && $out) { 
    $today  = today();
    $year_ago = add_day($date, -360);
    
    $wd     = get_sql_weekday($in); // day of week
    $nd     = day_diff($in, $out); // how many days
    $offset = day_diff($today, $in); // how many days from today

    if($nd <= 0) return json_error_object("Checkout must be after checkin");
    if($offset < 0) return json_error_object("Checkin cannot be in the past");
    if($offset + $nd > $cutoff) {
      $cutoff_date = add_day($today, $cutoff);
      return json_error_object("Checkout cannot be after $cutoff_date");
    }
    
    /** changeover */
    $searchArray['changeover'] = ['0', $wd];

    $sql_start = $offset + 1; // mysql is 1 indexed
    $avail_fld = "substr(calendar_availability, {$sql_start} + datediff('{$today}', calendar_startdate), {$nd}) as avail";
    $fields[] = $avail_fld;
    $searchOptions['fields'] = $fields; 
    
    $searchArray['min_stay'] = ['<=', $nd];
    $searchArray['max_stay'] = ['$or', [0, ['>=', $nd]]];
    
    $searchArray["length(calendar_availability) - datediff('{$today}', calendar_startdate)"] = ['>=', $nd]; // $cutoff;
    
    $searchArray['calendar_startdate'] = ['>=', $year_ago];
    $searchOptions['having'] = "avail not like '%N%'";
    
    $data->in  = $in;
    $data->out = $out;
    $data->nd  = $nd;
    $data->days_from_now = $offset;
     
    print_log("Search Avail: '{$avail_fld}'", 'property-search', LOG_LEVEL_TEST);
    
  }
  
  
  /** do the database search */
  $rentals = find_objects($property_type, $searchArray, $searchOptions) ?: [];
  $total_count = count($rentals);
                     
  /** reduce to page size */
  $start = $searchObj->start ?: 0;
  $limit = pick_first($options['limit'], $searchObj->limit, $page_limit); /** rentals returned for page */  
  
  if($limit > 0 && count($rentals) > $limit) $rentals = array_slice($rentals, $start, $limit);
  
  $radius = 1000; // radius around result if no bounds
  
  if(!$bounds && count($rentals)) {
    list($center, $bounds) = location_object_bounds($rentals, $radius);
    print_log("rental_search calc bounds:".dump($bounds,true), 'rental-search', LOG_LEVEL_TEST);
  }
  
  $query = '';
  if($searchAr = $searchObj ? array_filter(obj2array($searchObj)) : []) {
    //print_log('searchAr:'.dump($searchAr, true), 'property-search', LOG_LEVEL_TEST);
    foreach(['__sl', 'sort'] as $fld) unset($searchAr[$fld]);
    //print_log('searchAr2:'.dump($searchAr, true), 'property-search', LOG_LEVEL_TEST);
    //if($bounds = $searchAr['bounds']) 
    $query = $searchAr ? http_build_query($searchAr) : '';
    //print_log('query:'.$query, 'property-search', LOG_LEVEL_TEST);
  }

  /** we're only looping through the 18 shown on-screen */
  foreach($rentals as $aptObj) {
    $currency = strtoupper($aptObj->currency);
    $rate = $aptObj->rate_day;
    if($currency != SYSTEM_CURRENCY) {
      $aptObj->rate_day = round(get_exchange_rate($currency, SYSTEM_CURRENCY, $rate));
      $aptObj->currency = SYSTEM_CURRENCY;
      print_log("{$aptObj->id} {$currency}{$rate} => {$aptObj->currency}{$aptObj->rate_day}", 'rental-rate', LOG_LEVEL_TEST);
    }
  }
  
  $data->start = $start;
  $data->limit = $limit;
  $data->total_count = $total_count;
  $data->count = count($rentals);
  $data->center = $center ?: [];
  $data->bounds = $bounds ?: [];
  $data->query = $query;
  //$data->zoom = $zoom ?: 1;
  $data->language = isset($options['language']) ? $options['language'] : $__LANGUAGE;
  $data->search = $searchArray;
  $data->ng  = $ng;
  
  
  $data->rentals = $rentals;
  
  $result = new stdClass;  
  $result->success = count($errors) ? 0 : 1;
  $result->error = $errors;
  $result->message = $messages;  
  $result->data = $data;  

  if($searchObj->debug) {
    echo("Input: ".dump($searchObj,true));      
    echo("Search: ".dump($searchArray,true));      
    echo("Result: ".dump($result,true));      
  }
                 
  return $result;
}                      


/** new availability function, looks at all factors like min_stay, changeover, max guests etc */
function rental_availability($aptObj_or_id, $search = array(), $options = array()) {
  $ptype = isset($options['property_type']) ? $options['property_type'] : 'shortstay'; 
  $aptObj = any2obj($ptype, $aptObj_or_id);
  if(!$aptObj) return [0, "Missing rental"];
  
  if(is_object($search)) $search = (array) $search;
  $ng  = $search['ng'] ?: 1;
  $in  = $search['in'] ?: '';
  $out = $search['out'] ?: '';
  
  if($ng > 0 && $aptObj->max_guests > 0 && $ng > $aptObj->max_guests) return [0, phrase('maximum number of guests for %s is %s', CAPITALIZE, $aptObj->name, $aptObj->max_guests)];
  
  
  if($in && $out) {
    if(!is_date($in)) return [0, "Invalid checkin"];
    if(!is_date($out)) return [0, "Invalid checkout"];
    if($in < today()) return [0, "Checkin is in the past"];
    if($in >= $out) return [0, "Checkin must be before checkout"];

    
    $fid = $aptObj->guid;
    $source_id = $aptObj->source_id;
    print_log("Fid: {$fid} Source: {$source_id} In: $in Out: $out", 'interhome-search', LOG_LEVEL_TEST);
    
    $nd = period_diff($in, $out, 'd');
    
    // check min.stay.
    if($aptObj->min_stay > 0 && $aptObj->min_stay > $nd) return [0, "Minimum stay is {$aptObj->min_stay} days."];
    
    // check changeover
    if($co = $aptObj->changeover) {
      $wd = get_sql_weekday($in);
      $in_wd = WEEKDAYS[$co];
      if($co != $wd) return [0, "Checkin only allowed on $in_wd"];      
    }   
        
    // check availability
    if(isset($options['booked_ids'])) {
      $booked_ids = $options['booked_ids'] ?: [];
      if(in_array($id, $booked_ids)) return [0, "The rental is booked."];
    } else { /** DB function, expensive in loop - replace by calendar_availability */
      //$avail = get_apartment_availability($aptObj->id, $in, $out);
      $avail = calendar_availability($aptObj, $in, $out);
      if(!$avail) return [0, "The rental is already booked."];
    }    
  }  
  return [1, ""];  
}

// prints rate with currency and optionally, overstrike rate + discount rate, or (if rate change > 0), just the higher rate
/** deprecated: called from db_reservation.inc on line 898: print_price_breakdown */
/** unsused - automatic comment added on  */
function print_rate($cur_str, $rate, $discount=0, $unit='', $format=FORMAT_ALWAYS) {
  $new_rate = apply_discount($rate, $discount);
  $rate = $new_rate < $rate ? 
    html_strike(formatFloat($rate, $format, $cur_str)).' '.formatFloat($new_rate, $format, $cur_str) : 
    formatFloat($rate, $format, $cur_str);
  return $unit ? "$rate / ".phrase($unit) : $rate;
}


/** called by cronscript to update calendar_availability */
/** unsused - automatic comment added on  */
function rental_process_calendar_availability($options=[]) {
  $hard_limit = 1000;
  $result = new stdClass;
  $messages = $errors = [];
  $processed = 0;
  $search = ['calendar_availability' => '', 'active' => 1];
  if(isset($options['search'])) {
    foreach($options['search'] as $k => $v) $search[$k] = $v;
  }
  
  $obj_type = 'shortstay';
  $limit = isset($options['limit']) ? $options['limit'] : $hard_limit;
  $aptObjs = find_objects($obj_type, 
                          $search, 
                          ['sort' => 'id desc', 'limit' => $limit, 'fields' => ['id', 'type', 'media', 'name', 'source_id', 'guid']]);
  $count = count($aptObjs);
  $output .= html_h2("Found $count");
  foreach($aptObjs as $aptObj) {
    $id = $aptObj->id;
    $res = rental_update_calendar($aptObj);
    if($res->success) {
      $processed++;
      $messages = array_merge_smart($messages, $res->message);
      //$output .= "Update {$aptObj->id} len=".strlen($str)."<br>";
    } else {
      $errors = array_merge_smart($errors, $res->error);
    }
  }
  
  
  $result->success = $errors ? 0 : 1;
  $result->error = $errors;
  $result->message = $messages;  
  $result->count = $count;
  $result->processed = $processed;
  return $result;
}

/** called by cronscript to update media_json */
/** unsused - automatic comment added on  */
function rental_process_media_json($options=[]) {
  $result = new stdClass;
  $messages = $errors = [];
  $processed = 0;
  $obj_type = 'shortstay';

  $search = ['media_json' => '', 'active' => 1];
  if(isset($options['search'])) {
    foreach($options['search'] as $k => $v) $search[$k] = $v;
  }
  
  $aptObjs = find_objects($obj_type, 
                          $search, 
                          ['sort' => 'id desc', 'limit' => 3000, 'fields' => ['id', 'media', 'name']]);
  
  $count = count($aptObjs);
  foreach($aptObjs as $aptObj) {
    if($mediaJson = media2json($obj_type, $aptObj)) {
      list($pid, $perrors) = update_object('shortstay', ['media_json' => $mediaJson], $aptObj->id);
      if($perrors) $errors = array_merge_smart($errors, $perrors);
      else $processed++;
    } else {
      $errors[] = "No media json for {$aptObj->id}";
    }
  }
  
  $result->success = $errors ? 0 : 1;
  $result->error = $errors;
  $result->message = $messages;  
  $result->count = $count;
  $result->processed = $processed;
  return $result;
}


/** Update property->calendar_availability and property->calendar_startdate */
function rental_update_calendar($aptObj_or_id) {

  $aptObj = any2obj('shortstay', $aptObj_or_id);
  $id = $aptObj->id;
  
  $cutoff = 760; 
  $min_status = STATUS_RESERVED;
  $today = today();
  
  $avail_str = str_repeat("Y", $cutoff);
  $search = ['apt_id' => $id, 'status' => ['>=', $min_status]];
  $searchOptions = ['sort' => 'checkin', 'fields' => ['checkin','checkout', 'num_days']];
  
  if($aptObj->source_id == SOURCE_ID_INTERHOME) {
    return json_success_object("rental_update_calendar: Interhome - do nothing");
  } else {    
    $resObjs = find_objects('reservation', $search, $searchOptions) ?: [];  
  
    foreach($resObjs as $resObj) {
          
      $in = strip_time($resObj->checkin);
      $out = strip_time($resObj->checkout);
      $nd = period_diff($in, $out, 'd'); // how many days
      $start = day_diff($today, $in); // how many days
      
      $len = $start < 0 ? $nd + $start : $nd;
      $insert_str = str_repeat("N", $len);
      $insert_len = strlen($insert_str);    
  
      $pos = $start < 0 ? 0 : $start;
      
      $head = $start > 0 ? substr($avail_str, 0, $start) : '';
  
      if($start < 0) {
        $head = '';
        $tail = substr($avail_str, $insert_len);;
      } else {
        $head = $start ? substr($avail_str, 0, $start) : '';
        $tail = substr($avail_str, $start + $insert_len);
      }
      
      
      $avail_str = $head.$insert_str.$tail;
      
      $head_len = strlen($head);    
      $tail_len = strlen($tail);    
      $avail_len = strlen($avail_str);    
      
      $strlen = strlen($avail_str);
      if($strlen > $cutoff) $avail_str = substr($avail_str, 0, $cutoff);
    }
  }
  
  $data = [];
  $data['calendar_startdate'] = $today;
  $data['calendar_availability'] = $avail_str;
  
  list($pid, $errors) = update_object('property', $data, $id);
  if($errors) return json_error_object($errors);
  
  $result = new stdClass;
  $result->success = 1;
  $result->message = "Updated availability object for {$aptObj->id} len=".strlen($avail_str);
  $result->data = $data;
  return $result;
}


function property_edit_link($type, $id, $link_only=false, $ref_name='', $ref_link='') {
  global $__SITE;
  $siteObj = $__SITE;
  if(!auth_action($type, 'edit', $id)) return '';  
  if($link = site_pagetype_link($siteObj, PAGE_EDIT_PROPERTY, $_SESSION['language'], true)) $link = rtrim($link, '/')."/$id"; 
  else return '';
  return $link_only ? $link : arrow_link(phrase('edit', CAPITALIZE), $link, '', false, array('target' => 'edit_property'));
}


//function property_description($siteObj, $id, $lang='', $desctype_id = DESC_TYPE_PROPERTY) {
function property_description($siteObj_or_id, $id, $lang='', $options=NULL) {
  if($siteObj_or_id) $siteObj = any2obj('site', $siteObj_or_id);
  $site_id = $options['site_id'] ?: ($siteObj->source_site_id ?: $siteObj->id);

  $searchArray['type'] = $options['type'] ?: DESC_TYPE_PROPERTY; // property information
  $searchArray['property_id'] = $id;
  $searchArray['language'] = $lang ?: $_SESSION['language'];
  $searchArray['site_id'] = $site_id ? array($site_id, 0, 10) : 0;
  
  $descObjs = find_objects('property_description', $searchArray, array('sort' => 'site_id desc'));
  $descObj = $descObjs[0];
  if(!$descObj && !$options['force_language']) { // not found, try input language
    $input_lang = get_object('property', $id, 'input_lang');
    if($input_lang && $input_lang != $lang) {
      $searchArray['language'] = $input_lang;
      $descObj = find_object('property_description', $searchArray);
    }
  }
  return $descObj;
}

// uploader for jquery file upload
function property_uploader($ptype, $aptObj_or_id, $options = array()) {
  global $u_id, $site_id,$u_level;
  $aptObj = any2obj($ptype, $aptObj_or_id);
  $apt_id = $aptObj->id;
  
  $class_attrs = get_object_attributes($ptype, false);
  $upload_options = $class_attrs['media'][ATT_OPTIONS];
  if($upload_options && !is_array($upload_options)) $upload_options = json_decode($upload_options, true); /** we now use json in attribute definition */ 
    
  $upload_options = array_extend($upload_options, $options);
  
  //$upload_options['minimum_size'] = array(1080, 1920); // either short side >= 1080 or long side >= 1920

  $upload_options['minimum_size'] = 640; // either short side >= 1080 or long side >= 1920

  $fld = 'media';
  $val = $aptObj->media;
     
  
  $mediaData['user_id'] = $u_id ?: $aptObj->user_id;
  $mediaData['parent_id'] = $apt_id;
  $mediaData['parent_type'] = $ptype;
  $mediaData['parent_field'] = 'media';
  if($aptObj->source_id == SOURCE_ID_INTERHOME) { /** todo: check if media is list of external IDs */
    $mediaData['inline'] = 1;
  }
  
  $upload_options['download_template'] = $options['download_template'] ?: "";
  
  $upload_options['media_data'] = $mediaData;
  $upload_options['data-value_list'] = $val;
  $upload_options['show_file_list'] = false;
  $upload_options['title'] = "Click or drag to upload property images";
  
  $edit_url = "/admin/app_data.php?oper=save&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=media"; // id will be appended
  $upload_options['edit_url'] = $edit_url;
  //$upload_options['edit_target'] = '#edit-rental-image';
  
  $handler = isset($options['handler']) ? $options['handler'] : '';
  if($upload_options['handler'] = $handler) {
    $handler_options = array();
    $upload_options['handler_options'] = $handler_options;
  }

  $uploader = jquery_file_uploader("apt-$apt_id-$fld", $val, $upload_options);

  //$edit_dialog = html_div("<br>Edit dialog<br>", "border", array('id' => 'edit-rental-image'));
  //$uploader .= $edit_dialog;
  
  //$uploader .= "Hello:".dump($mediaData,true);
  return $uploader;
}


/* New functions rental_ */

// return array assoc. of property facilities order by group 
function rental_amenities($siteObj, $aptObj=null, $options = array()) {
  $search = array();
  
  $search['active'] = 1; 
  if($siteObj->golf_site) $search['golf_rental'] = 1;
  else $search['serviced'] = 1;
  if($options['searchable']) $search['searchable'] = 1;
  
  //array("serviced" => 1, "searchable" =>1)
  $facs = find_objects("facility", $search, array("sort" => 'group_name, name'));
  $facArray = array();
  $pfacs = $aptObj ? explode(',', $aptObj->facilities) : array();
  foreach($facs as &$fac) {
    $fac->value = in_array($fac->id, $pfacs) ? 1 : 0;
    $facArray[$fac->group_name][] = $fac;
  }
  return $facArray;
}

function rental_facility_icon($id) {
  /* im = icomoon 
     fa = font awesome 
     fi = foundation icons: http://test.hitabrick.com/afr/bower/foundation-icon-fonts/index.html
     icon = our own custom font (afr)
   
    afr custom:
    
                                
     */
  
  $default = "fa fa-check-square-o"; /* return checkbox */
  $icon_map = array(
    100 =>'im im-climate-control',
    140 =>'im im-terrace',
    150 =>'im im-bathtub',
    190 =>'im im-lines',
    134 =>'im im-children',
    230 =>'im im-restaurant',
    133 =>'im im-elder',
    
    
    260 =>'fi fi-elevator', /* foundation */
    310 =>'fa fa-fire', /* new */

    311 =>'fa fa-fire-extinguisher', /* new */
    
    300 =>'im im-fitness',
    360 =>'im im-patio',
    
    // 390 =>'afr icon-hair-dryer', /* hair dryer */
    
    170 =>'im im-wi-fi',  /* new */
    380 =>'afr icon-iron',
    
    420 =>'im im-spa',
    905 =>'im im-kitchen',  
    
    470 =>'afr icon-microwave', /* microwave oven */
    800 =>'fi fi-mountains', /* mountain view */
    480 =>'fi fi-no-smoking', /* no smoking */
    490 =>'', /* oven */
    
    500 =>'im im-parking',
    130 =>'im im-dog',
    
    510 =>'im im-phone', /* new */
    // 600 =>'afr icon-sauna', /* sauna */
    
    621 =>'im im-bed',
    790 =>'im im-sun-lines',
    132 =>'im im-smoking',
    690 =>'im im-pool',
    710 =>'im im-tv',
    730 =>'im im-washing-machine',
    760 =>'im im-wheel-chair',  
    780 =>'im im-wi-fi',
    
    // not active
    590 =>'im im-satellite',
    400 =>'im im-thermometer',
    650 =>'im im-stereo',
    151 =>'im im-electric',
    110 =>'im im-bus',
    460 =>'im im-air',
    
    /* new custom AFR icons */
    470 => 'af icon-microwave            ',
    690 => 'af icon-swimming-pool-indoor ',
    600 => 'af icon-sauna                ',
    180 => 'af icon-satellite-tv         ',
    942 => 'af icon-walking              ',            
    909 => 'af icon-central-heating      ',
    400 => 'af icon-central-heating      ',
    941 => 'af icon-golf-flag            ',
    380 => 'af icon-ironing              ',
    // 000 => 'af icon-city                 ',
    // 000 => 'af icon-town                 ',
    // 000 => 'af icon-room-sign            ',
    // 000 => 'af icon-cancel               ',
    240 => 'af icon-dryer                ',
    937 => 'af icon-swimming-pool        ',
    000 => 'af icon-toilet-paper         ',
    191 => 'af icon-golf-clubs           ',
    390 => 'af icon-hair-dryer           ',
    470 => 'af icon-microwave            ',
    490 => 'af icon-microwave            ',
        
    /* icomoon not in use
    000 =>'im im-fm',
    000 =>'im im-shift-auto',
    000 =>'im im-lock',
    000 =>'im im-casino',
    000 =>'im im-diesel',                     
    000 =>'im im-car-doors',
    000 =>'im im-soundproof',
    000 =>'im im-meet',
    000 =>'im im-width',
    000 =>'im im-shift',
    000 =>'im im-bed',
    000 =>'im im-car-window',
    000 =>'im im-plane',
    000 =>'im im-bar',
    000 =>'im im-car-wheel',
    000 =>'im im-business-person',
    000 =>'im im-driver',
    000 =>'im im-icon_1041',
    000 =>'im im-shower',
    000 =>'im im-sunrise',
    000 =>'im im-sun',
    000 =>'im im-moon',
    000 =>'im im-sun3',
    000 =>'im im-windy',
    000 =>'im im-wind',
    000 =>'im im-snowflake',
    000 =>'im im-cloudy',
    000 =>'im im-cloudy-moon',
    000 =>'im im-sun-lines',
    000 =>'im im-moon-lines',
    000 =>'im im-cloud-lines',
    000 =>'im im-cloud',
    000 =>'im im-cloud-lightning',
    000 =>'im im-lightning',
    000 =>'im im-rainy',
    000 =>'im im-rain',
    000 =>'im im-windy-cloud',
    000 =>'im im-windy-cloud-rain',
    000 =>'im im-snowy',
    000 =>'im im-snowy-cloud',
    000 =>'im im-snow-cloud',
    000 =>'im im-snow-cloud-2',
    000 =>'im im-cloud-2',
    000 =>'im im-cloud-lightning-2',
    000 =>'im im-lightning-2',
    000 =>'im im-sun-fill',
    000 =>'im im-moon-fill',
    000 =>'im im-cloudy-fill',
    000 =>'im im-cloudy-moon-fill',
    000 =>'im im-cloud-fill',
    000 =>'im im-cloud-lightning-fill',
    000 =>'im im-rainy-fill',
    000 =>'im im-rain-fill',
    000 =>'im im-windy-cloud-fill',
    000 =>'im im-windy-cloud-rain-fill',
    000 =>'im im-snowy-cloud-fill',
    000 =>'im im-snow-cloud-fill-2',
    000 =>'im im-cloud-fill-2',
    000 =>'im im-cloud-lightning-fill-2',
    000 =>'im im-compass',
    000 =>'im im-none',
    000 =>'im im-celsius',
    000 =>'im im-fahrenheit'
    */
    );
  return $icon_map[$id];
}

// boxes with filters that can be clicked away  
function rental_search_filters($vars, $options = array()) {
  $filters = array();
  $vars = (array) $vars;
  print_log("rental_search_filters: vars:".dump($vars, true), 'app', LOG_LEVEL_TEST);
  if($vars['in'] && $vars['out']) {
    $date_format = DATE_FORMAT;
    $data = array("in" => '', "in_disp" => '', "out" => '', "out_disp" => '');
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('dates', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

  // guests
  if($vars['ng'] > 1) {        
    $data = array("ng" => 0);
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('guests', CAPITALIZE), 'data' => $data, 'link' => $link);
  }  

  // rental type
  if($vars['pt']) {        
    $data = array("pt" => '');
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('type', CAPITALIZE), 'data' => $data, 'link' => $link);
  }  

  // faciliites
  if($facs = $vars['fac']) {
    if(1) { // show each fac
      $facAr = explode(',', $facs);
      foreach($facAr as $facId) {
        $facCopy = $facAr;
        if(($key = array_search($facId, $facCopy)) !== false) {
          unset($facCopy[$key]);
        }        
        $rest = implode(',', $facCopy);
        $data = array("fac" => $rest);
        $link = "?".array2qs(array_merge($vars, $data));
        $facName = get_object("facility", $facId, 'name');
        $filters[] = ['name' => phrase($facName, CAPITALIZE), 'data' => $data, 'link' => $link, 'class' => 'set-fac'];
      }
    } else { // show all facs together    
      $data = array("fac" => '');
      $link = "?".array2qs(array_merge($vars, $data));
      $filters[] = array('name' => phrase('amenities', CAPITALIZE), 'data' => $data, 'link' => $link);
    }
    
  }

    // bedrooms
  if($vars['nr'] > 1) {        
    $data = array("nr" => 1);
    $link = "?".array2qs(array_merge($vars, $data));
    //$filters[] = html_link($link, "Bedrooms", array("class" => "form-resubmit", "data-data" => json_encode($data), "data-target" => "#rental_search_form"));
    $filters[] = array('name' => phrase('bedrooms', CAPITALIZE), 'data' => $data, 'link' => $link);
    
  }  

  // bathrooms
  if($vars['nb'] > 1) {        
    $data = array("nb" => 1);
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('bathrooms', CAPITALIZE), 'data' => $data, 'link' => $link);
  }  

  // price
  //if($vars['p'] || $vars['pp']) {
  $p = $vars['p'];
  list($min, $max) = explode(';', $p);
  if($max >= $max_price) $max = 0;
  if($min > 0 || $max > 0) {        
    
    $data = array("p" => '', "pp" => '');
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('price', CAPITALIZE), 'data' => $data, 'link' => $link, 'class' => 'green-border');
  }  
  

  // size
  if($vars['sz'] > 1) {
    $data = array("sz" => 1);
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('size', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }  
  
  return $filters; 
}

// tips when search result is empty  
function rental_search_tips($vars, $optons = array()) {
  $vars = (array) $vars;
  $center = $vars['center'];
  $bounds = $vars['bounds'];
  
  $unset = array('location_lat','location_long','location_bounds'); // not in use
  foreach($unset as $f) unset($vars[$f]); 
  $tips = array();
  
  // widen geographical area (increase radius)
  if($bounds) {
    $boundsObj = bounds_json2obj($bounds);
    $radius = bounds2radius($boundsObj);
    
    if($radius < 200*1000) { // 100 km  
      $locObj = bounds2loc($boundsObj);
      $newBoundsObj = radius2bounds($locObj, $radius * 2); // four times radius
      
      $newBounds = bounds_obj2json($newBoundsObj);
      $data = array("bounds" => $newBounds);
      $link = "?".array2qs(array_merge($vars, $data));
      $tips[] = array('name' => phrase('try_a_bigger_area', CAPITALIZE), 'data' => $data, 'link' => $link, 'class' => 'map-zoom-out');
    }
  }

  // remove faciliites
  if($vars['fac']) {        
    $data = array("fac" => '');
    $link = "?".array2qs(array_merge($vars, $data));

    $tips[] = array('name' => phrase('try_without_amenities', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }

  // reduce number of guests
  if($vars['ng'] > 4) {        
    $data = array("ng" => 4);
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_fewer_people', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }

  if(strlen($vars['p'])) {        
    $data = array("p" => '');
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_all_rates', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }

  if($vars['pt']) {        
    $data = array("pt" => '');
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_all_rental_types', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }

  if($vars['nr'] > 2) {        
    $data = array("nr" => 2);
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_fewer_number_of_rooms', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }

  if($vars['nb'] > 2) {        
    $data = array("nb" => 2);
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_fewer_number_of_bathrooms', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }

  // $tips[] = ['name' => dump($vars,true)];
  // lengthen stay
  $in = $vars['in'];
  $out = $vars['out'];
  $nd = $in && $out ? period_diff($in, $out, 'd') : 0;  
  if($nd >= 1 && $nd < 3) {
    $date_format = DATE_FORMAT;
    $out = sql_add_days($in, 3);
    $date = date($date_format, strtotime($out));
    $data = array("out" => $date);
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_longer_stay', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }
  
  return $tips;  
}

/** function to print fac icons */
/** valid categories: golf (course), golf_rental, serviced, non_serviced, boat */
/** unsused - automatic comment added on  */
function test_facs($category) {  
  $facAr = $aptObj->facilities ? explode(',', $aptObj->facilities) : array();
  $facs = find_objects('facility', array($category => 1));
  $output .= '<ul class="booking-item-features booking-item-features-expand-inline  mb30 clearfix">';
  foreach($facs as $fac) {
    $icon = rental_facility_icon($fac->id) ?: "fa fa-check";
    $output .= '<li class="col-xs-6 col-sm-4 col-md-6 col-lg-4"><i class="'.$icon.'"></i><span class="booking-item-feature-title">'.$fac->name.'('.$icon.') '.'</span></li>';
  }

  $output .= '</ul>';
  return $output;
}

/** new confirmed bookings made in this period */
function calendar_booked($start, $end, $search, $options = array()) {
  $search['status'] = [STATUS_RESERVED, STATUS_COMPLETED];
  $search['booked_timestamp'] = array('><', $start, $end); 
  $fields = ['res_id', 'apt_id', 'booked_timestamp', 'checkin', 'checkout', 'status', 'total', 'grand_total'];
  $objs = find_objects('reservation', $search,  array('fields' => $fields, 'sort' => $sort, 'debug_query' => $debug));
  return $objs;
}

function calendar_invoiced($start, $end, $search, $options = array()) {
  $search['status'] = ['>=', INVOICE_STATUS_SENT];
  $search['invoice_date'] = array('><', $start, $end); 
  $fields = ['id', 'res_id', 'apt_id', 'invoice_date', 'status', 'currency', 'total'];
  $objs = find_objects('invoice', $search,  array('fields' => $fields, 'sort' => $sort, 'debug_query' => $debug));
  return $objs;
}

function calendar_paid($start, $end, $search, $options = array()) {
  $search['payment_status'] = PAYMENT_STATUS_COMPLETED;
  $search['payment_date'] = array('><', $start, $end); 
  $fields = ['id', 'res_id', 'apt_id', 'payment_date', 'payment_status', 'currency', 'amount'];
  $objs = find_objects('payment', $search,  array('fields' => $fields, 'sort' => $sort, 'debug_query' => $debug));
  return $objs;
}

/** from helpers_calendar.inc */
// generic function to get all objects in range with at least status $status
function calendar_entries($obj_type, $start, $end, $search, $options = array()) {
  $tbl = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  if($obj_type == 'res_exception') { /** make nice */
    $exObjs  = date_exceptions($start, $end, $search['host_id'], $search['apt_id'], $options);
    $objs = [];
    foreach($exObjs as $exObj) {
      $exObj->obj_type = $obj_type;
      $exObj->obj_id = $exObj->id;      
      $exObj->title = $exObj->description;      
      $exObj->status = 40;      
      $objs[] = $exObj;
    }
    return $objs;
    //list($start_field, $end_field) = object_timefields($obj_type);
    //$fields = ["'{$obj_type}' as obj_type", "{$tbl}.{$key} as obj_id",'site_id', 'host_id', 'user_id','apt_id', 'description as title', "substr({$start_field},1,10) as start", "substr({$end_field},1,10) as end"];  
  } else {
    list($start_field, $end_field) = object_timefields($obj_type);
    $fields = ["'{$obj_type}' as obj_type", "{$tbl}.{$key} as obj_id",'site_id', 'host_id', 'user_id','apt_id', 'COALESCE(NULLIF(last_name,\'\'), NULLIF(first_name,\'\'), organization) as title', 'status', 'status_change_timestamp', "substr({$start_field},1,10) as start", "substr({$end_field},1,10) as end"];  
  }
  
  
  if($obj_type == 'reservation') {
    $fields = array_merge($fields, ['charge_tax', 'tax_included', 'charge_city_tax', 'city_tax_rate', 'city_tax']);
    $fields = array_merge($fields, ['num_days', 'num_guests', 'total', 'commission', 'fees', 'grand_total', 'tax', 'tax_of_which', 'alert']);
  }
  
  //$search['property_type'] = $ptype = isset($options['property_type']) ? $options['property_type'] : 'shortstay';  
  $search['client_id'] = $_SESSION['client_id'];
  $search[$start_field]  = array('<', $end); /** <= if end_date = last day show, < if end_date is day after last day shown */
  $search[$end_field]    = array('>=', $start); /** >= because we want to show somebody is checkout out on the first day */
  
  $sort = $start_field;    
  //print_log("$obj_type search:".dump($search,true), 'calendar', LOG_LEVEL_TEST);
  $objs = find_objects($obj_type, $search,  array('fields' => $fields, 'sort' => $sort, 'debug_query' => $debug));

  return $objs;
}


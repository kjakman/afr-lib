<?php

function property_sort($type='') {
  global $__SITE;
  $siteObj = $__SITE;
  $const = defined(PROPERTY_SORT) ? PROPERTY_SORT : '';
  return pick_first($siteObj->property_sort, $const, 'priority DESC, source_id, book_direct DESC, created DESC');
}

function service_fee_matrix() {
  
  return array(500 => 0.10, 1000 => 0.09, 1500 => 0.08, 2000 => 0.07, 0 => 0.06); 
  
  /** old code below */
  // variables
  $min = 0.06;
  $max = 0.12;
  $factor = 1.4;
  $min_rent = 1000;

  $fee = $max;
  $rent = $min_rent;
  $matrix = array();
  while($fee >= $min) {
    $commission = $rent * $fee;
    $matrix[$rent] = $fee;
    $fee = $fee - 0.01;
    $rent = $rent * $factor;
  }
  return $matrix;
}

function service_fee_percentage($rent) {
  $matrix = service_fee_matrix();
  foreach($matrix as $amount => $percentage) {
    if($rent < $amount) break;
    //echo(" - rent ($rent) > $amount: p=$percentage<br>");
  }
  return $percentage;
}

function service_fee($rent) {
  $percentage = service_fee_percentage($rent);
  $fee = ceil($rent * $percentage);
  return $fee;
  //echo("Rent=$rent Percentage = $percentage fee=$fee<br>");
}



function rate_exception_keys() {
  return ['rate_day', 'rate_hour', 'rate_extraperson', 'rate_change', 'rate_weekend', 'rate_week', 'rate_2_weeks', 'rate_3_weeks', 'rate_month', 'currency'];
}

// returns true if exception affects the rate
/** unsused - automatic comment added on  */
function rate_exception($ex) {
  $rate_keys = rate_exception_keys();
  $exAr = remove_empty((array) $ex);
  return array_intersect($rate_keys, array_keys($exAr)) ? true : false;
}

// returns true if exception depends on the date
/** unsused - automatic comment added on  */
function date_exception($ex) {
  $rate_keys = array('start', 'end');
  $exAr = remove_empty((array) $ex);
  return array_intersect($rate_keys, array_keys($exAr)) ? true : false;
}


// returns exceptions for a host in given period, optionally limited to apt_ids (array or single value)
// that affect the rate
// if field is set, this must be not-null
function rate_exceptions($start, $end, $host_id, $apt_ids, $field = '') {
  $exceptions = date_exceptions($start, $end, $host_id, $apt_ids, $field);
  return array_filter($exceptions, "rate_exception");  
}

/** new */
// returns exceptions for a host in given period, optionally limited to apt_ids (array or single value)
// if field is set, this must be not-null
function date_exceptions($start, $end, $host_id, $apt_ids, $field = '') {
  $source_id = isint($apt_ids) ? get_object('shortstay', $apt_ids, 'source_id') : 0;
  if($source_id == SOURCE_ID_INTERHOME) {
    require_once('xml_interhome.inc');
    $aptObj = get_object('property', $apt_ids);    
    $priceObjs = interhome_pricing($aptObj->guid, $start, $end, ['average' => false]);
    $exObjs = [];
    //print_log("date_exceptions: $start-$end $apt_ids source:$source_id code:{$aptObj->guid}".dump($priceObjs, true), 'calendar', LOG_LEVEL_TEST);
    foreach($priceObjs as $priceObj) {
      $exObj = new stdClass;
      $exObj->apt_id = $aptObj->id;
      $exObj->host_id = $aptObj->host_id;
      $exObj->property_type = $aptObj->type;

      $exObj->start = $priceObj->startdate;
      $exObj->end = get_next_day($priceObj->enddate);
      $exObj->rate_day = $priceObj->rentalprice;
      $exObjs[] = $exObj;
    }
    return $exObjs;
  }
  
  $exSearch = array();
  if($host_id) { // if only one host is shown, show for this apt or all apts
    //$exSearch[DB_LOGICAL_OR] = array('apt_id' => $apt_ids, DB_LOGICAL_AND => array('host_id' => $host_id, 'apt_id' => ATT_DEFAULT_NULL)); 
    $exSearch[DB_LOGICAL_OR] = array('apt_id' => $apt_ids, DB_LOGICAL_AND => array('host_id' => $host_id, 'apt_id' => [0, ATT_DEFAULT_NULL])); 
  } else {
    $exSearch['apt_id'] = $apt_ids; // don't show host exceptions if multiple host_ids in calendar (then host_id = 0)
  }
  $exSearch['active'] = 1;
  if($field) $exSearch[$field] = array("!=", ATT_DEFAULT_NULL);
  $exSearch['start'] = array("<", $end);;
  $exSearch['end'] = array(">", $start);  
  //dump($exSearch);
  return find_objects('res_exception', $exSearch, array('debug_query' => 0));
  
}

// returns true if exception does not depend on the date
/** unsused - automatic comment added on  */
function non_date_exception($ex) {
  return !date_exception($ex);
}

// splits exceptions into those that must be applied in a date loop, and those that can be applied before the loop
// if multiple exceptions with days_from_now/days_from_now_gt, length of stay, removes all but the most relevant
function split_exceptions($type, $exceptions, $searchObj, $apt_id=0, $options=NULL) {   
  $in = $searchObj->in;
  $out = $searchObj->out;
  $hourly = hourly_rate($type);
  
  // lamdba function using closure which returns true if array or object $v has key $key and corresponding and value is not null
  $has_key_not_null = function($key) {
    return function ($v) use ($key) {$v = remove_empty((array) $v); return in_array($key, array_keys((array) $v)) ? true : false ; };
  };
  
  // lamdba function using closure which returns false if array or object $v has key $key and corresponding and value is not null
  $not_has_key_not_null = function($key) {
    return function ($v) use ($key) {$v = remove_empty((array) $v); return in_array($key, array_keys((array) $v)) ? false : true ; };
  };

  $debug = $options['debug'] && $apt_id;
  if($debug) echo(count($exceptions)." exceptions:". print_exceptions($type, $apt_id, $exceptions, $options).html_break());

  // removes all but the biggest num_days / num_hours
  if($hourly) {
    if($max_hours_exception = array_find_max($exceptions, 'num_hours')) {
      $exceptions = array_filter($exceptions, $not_has_key_not_null('num_hours'));
      $exceptions[] = $max_hours_exception; 
    }
  } else {
    if($max_days_exception = array_find_max($exceptions, 'num_days')) {
      $exceptions = array_filter($exceptions, $not_has_key_not_null('num_days'));
      $exceptions[] = $max_days_exception; 
    }
  }

  // removes all but the biggest days_from_now_gt
  if($max_dfn_exception = array_find_max($exceptions, 'days_from_now_gt')) {
    $exceptions = array_filter($exceptions, $not_has_key_not_null('days_from_now_gt'));
    $exceptions[] = $max_dfn_exception; 
  }
  
  // removes all but the smallest days_from_now
  if($min_dfn_exception = array_find_min($exceptions, 'days_from_now')) {
    $exceptions = array_filter($exceptions, $not_has_key_not_null('days_from_now'));
    $exceptions[] = $min_dfn_exception; 
  }
  
  $non_date_exceptions = array_filter($exceptions, "non_date_exception"); // rate affecting exceptions where date does not matter, can be applied now 
  $date_exceptions = array_filter($exceptions, "date_exception");
  
  $date_intersect_exceptions = array(); // these must be checked in loop
  $date_non_intersect_exceptions = array(); // these can be applied now
  
  if($date_exceptions) { // exceptions where date matters
    foreach($date_exceptions as $date_exception) {
      $f = $date_exception->overlap_fraction = dates_intersect_fraction($in, $out, $date_exception->start, $date_exception->end);         
      if($f == 1) {
        $date_non_intersect_exceptions[] = $date_exception;
      } else {
        $date_intersect_exceptions[] = $date_exception;    
      }
    }
  } 
  
  if($debug) echo(count($exceptions)." rate exceptions:".print_exceptions($type, $apt_id, $exceptions, $options).html_break());
  if($debug) echo(count($non_date_exceptions)." non-date:". print_exceptions($type, $apt_id, $non_date_exceptions, $options).html_break());
  if($debug) echo(count($date_intersect_exceptions)." intersect:". print_exceptions($type, $apt_id, $date_intersect_exceptions, $options).html_break());
  if($debug) echo(count($date_non_intersect_exceptions)." non-intersect:". print_exceptions($type, $apt_id, $date_non_intersect_exceptions, $options).html_break());

  $loop_exceptions = $date_intersect_exceptions;
  $non_loop_exceptions = array_merge($non_date_exceptions, $date_non_intersect_exceptions);
  return array($non_loop_exceptions, $loop_exceptions);
}


// gets a price breakdown object with rows from database
function get_price_breakdown($priceObj_or_id) {
  $priceObj = any2obj('price_breakdown', $priceObj_or_id);
  if(!$priceObj) return null;
  $priceObj->prices = array();
  $prices = find_objects('price_breakdown_row', array('price_breakdown_id' => $priceObj->id),  array('sort' => 'date'));
  foreach($prices as $dayPriceObj) {
    $priceObj->prices[$dayPriceObj->date] = $dayPriceObj;
  }
    
  return $priceObj;  
}

/** unsused - automatic comment added on  */
function delete_price_breakdown($pb_id) {
  list($id, $errors) = delete_object('price_breakdown', $pb_id);
}
  
// add or update price breakdown
/** unsused - automatic comment added on  */
function save_price_breakdown($priceObj, $id=0) {
  $debug = false;
  $errors = array();
  $prices = $priceObj->prices ?: array();  
  $dates  = count($prices) ? array_keys($prices) : array();    
  if($id) {
    $exPriceObj = get_price_breakdown($id);
    if($debug) echo("update id=$id");
    if(!$exPriceObj) die("couldn't find existing");
  }
  if($exPriceObj) {
    if($debug) echo("Updating price $id<br>");    
    $exPrices = $exPriceObj->prices ?: array(); // existing prices    
    $exDates  = count($exPrices) ? array_keys($exPrices) : array();
    $delDates = array_diff($exDates, $dates);
    if($debug) echo("Dates<br> was: ".implode(',',$exDates)."<br> now: ".implode(',',$dates)."<br> diff: ".implode(',',$delDates)."<br>");
    if(count($delDates)) { // dates that should be deleted
      $del_ids = array();
      foreach($delDates as $delDate) $del_ids[] = $exPrices[$delDate]->id;
      if(count($del_ids)) {
        list($del_count, $errors) = delete_objects('price_breakdown_row', $del_ids);        
        if($debug) echo("Deleting Dates: ".implode(',',$del_ids)."<br>");
      }
    }
    list($id, $errors) = update_object('price_breakdown', $priceObj, $id);     
    if($debug) echo("Updated price $id<br>");
    // delete rows no longer in range
  } else {
    list($id, $errors) = add_object('price_breakdown', $priceObj);
    if($debug) echo("Added price $id<br>");
  }

  if($id && $prices) {  
    foreach($prices as $dayPriceObj) {
      $dayPriceObj->price_breakdown_id = $id;
      $date = $dayPriceObj->date;
      $oldPrice = $exPrices[$date];
      if($row_id = $oldPrice ? $oldPrice->id : 0) {
        list($rid, $rerrors) = update_object('price_breakdown_row', $dayPriceObj, $row_id); 
        if($debug) echo("$date: Updated existing price $row_id<br>");
      } else {
        list($pid, $rerrors) = add_object('price_breakdown_row', $dayPriceObj); 
        if($debug) echo("$date: Added price $pid<br>");
      }
            
    }
  }
  return array($id, $errors);
}
  

// returns the average rate per day/hour/shift for given period and number of guests
// including exceptions and excluding discounts (unless apply_discount is set)
// fees exluded
// todo: return num_hours for hourly reservations
function get_average_rate($aptObj_or_id, $in, $out, $num_guests=1, $apply_discount=false) {
  $aptObj = any2obj('shortstay', $aptObj_or_id);
  $apt_id = $aptObj->id;
  $host_id = $aptObj->host_id;
  
  $searchObj->in = $in;    
  $searchObj->out = $out;
  $searchObj->ng = $num_guests;
  $unit = 'day';
  $searchObj->nd = $num_days =  period_diff($in, $out, 'd');
  $searchObj->nm = $num_months =  period_diff($in, $out, 'm');

  $priceObj = price_breakdown($aptObj, $searchObj, null, array('total' => false));
  $priceObj->unit = $unit;
  return $priceObj;

}

function price_breakdown($aptObj, $searchObj, $exceptions=NULL, $options=NULL) {
  $priceObj = clone($searchObj);
  $searchObj->apt_id = $aptObj->id;
  $priceObj->client_id = $aptObj->client_id;
  $priceObj->legal_entity_id = $aptObj->legal_entity_id;
  $priceObj->search = $searchObj;
  $discount = '';            
  $total_only = $options['total'];
  $debug = $options['debug'];
  $options['price_breakdown'] = true;
  //dump($aptObj);
  $type = $aptObj->type;
  $apt_id = $aptObj->id;
  $in = $searchObj->in;
  $out = $searchObj->out;
  $nd = $searchObj->nd ?: period_diff($in, $out, 'd');
  $nm = $searchObj->nm ?: period_diff($in, $out, 'm');
  $ng = $num_guests = intval($searchObj->ng ?: 1);

  $fees = $tax = 0;

  if(isset($options['fees']) || isset($searchObj->fees)) $fees = $options['fees'] ?: $searchObj->fees; // explicitly set (from reservation?)
  if(isset($options['city_tax']) || isset($searchObj->city_tax)) $tax = $options['city_tax'] ?: $searchObj->city_tax; // explicitly set (from reservation)
  
  $prices = [];
  
  if(!($in && $out && $nd > 0) || hourly_rate($type)) {
    if($debug) echo("returning because !(in && out && nd > 0) !($in && $out && $nd > 0)<br>".dump($searchObj, true));
    return array();
  }

  
  $currency = strtoupper($aptObj->currency);
  if($currency != SYSTEM_CURRENCY) {
    print_log("{$aptObj->id} rate={$aptObj->currency}{$aptObj->rate_day}", 'rental-rate', LOG_LEVEL_TEST);
    $aptObj->rate_day = get_exchange_rate($currency, SYSTEM_CURRENCY, $aptObj->rate_day);
    $aptObj->currency = SYSTEM_CURRENCY;
    print_log("{$aptObj->id} now={$aptObj->currency} {$aptObj->rate_day}", 'rental-rate', LOG_LEVEL_TEST);
  }
  
  $weekly_pricing = $monthly_pricing = false;  
  $non_loop_exceptions = $loop_exceptions = array();
  if($nd >= 28 && !$aptObj->discount_month && $aptObj->rate_month > 0) { // monthly pricing, ignore daily rates and exceptions
    $month_exceptions = $loop_exceptions = rate_exceptions($in, $out, 0, $aptObj->id, 'rate_month'); // only rate_month exceptions for this specific property
    $unit = 'day';
    $weekend_pricing = false;
    $monthly_pricing = true;  
    $rate_day = $aptObj->rate_day = round(($aptObj->rate_month * $nm) / $nd, 6); /** hmm, need to think about this */
    
  } elseif($nd >= 21 && !$aptObj->discount_3_weeks && $aptObj->rate_3_weeks > 0) { // 3-weekly pricing, ignore daily rates and exceptions
    $week_field = 'rate_3_weeks';
    $week_count = 3;
    $week_exceptions = $loop_exceptions = rate_exceptions($in, $out, 0, $aptObj->id, 'rate_3_weeks'); 
    $unit = 'day';
    $weekend_pricing = false;
    $weekly_pricing = true;  
    $rate_day = $aptObj->rate_day = round($aptObj->rate_3_weeks / 14, 6);
    $aptObj->rate_week = round($aptObj->rate_3_weeks / 3, 6);
    //print_log("3-week pricing: rate_3_week = {$aptObj->rate_3_weeks} rate_week: {$aptObj->rate_week}", 'calendar', LOG_LEVEL_TEST); 
    //foreach($week_exceptions as $exObj) {
    //  print_log("{$exObj->id}: start={$exObj->start} end={$exObj->end} rate_2_week = {$exObj->rate_2_weeks} rate_3_week = {$exObj->rate_3_weeks}", 'calendar', LOG_LEVEL_TEST); 
    //}

  } elseif($nd >= 14 && !$aptObj->discount_2_weeks && $aptObj->rate_2_weeks > 0) { // 2-weekly pricing, ignore daily rates and exceptions
    $week_field = 'rate_2_weeks';
    $week_count = 2;
    $week_exceptions = $loop_exceptions = rate_exceptions($in, $out, 0, $aptObj->id, 'rate_2_weeks') ?: array();
    $unit = 'day';
    $weekend_pricing = false;
    $weekly_pricing = true;  
    $rate_day = $aptObj->rate_day = round($aptObj->rate_2_weeks / 21, 6);
    $aptObj->rate_week = round($aptObj->rate_2_weeks / 2, 6);
    //print_log("2-week pricing: rate_2_week = {$aptObj->rate_2_weeks} rate_week: {$aptObj->rate_week}", 'calendar', LOG_LEVEL_TEST); 
    //foreach($week_exceptions as $exObj) {
    //  print_log("{$exObj->id}: start={$exObj->start} end={$exObj->end} rate_2_week = {$exObj->rate_2_weeks} rate_3_week = {$exObj->rate_3_weeks}", 'calendar', LOG_LEVEL_TEST); 
    //}
    
  } elseif($nd >= 7 && !$aptObj->discount_week && $aptObj->rate_week > 0) { // weekly pricing, ignore daily rates and exceptions
    $week_field = 'rate_week';
    $week_count = 1;
    $week_exceptions = $loop_exceptions = rate_exceptions($in, $out, 0, $aptObj->id, 'rate_week'); // only rate_month exceptions for this specific property
    $unit = 'day';
    $weekend_pricing = false;
    $weekly_pricing = true;  
    $rate_day = $aptObj->rate_day = round($aptObj->rate_week / 7, 6);
    
  } else { // daily pricing (or hourly)
    
    if(!$exceptions) $exceptions = rate_exceptions($in, $out, $aptObj->host_id, $aptObj->id);
                        
    if($aptObj->rate_weekend <= 0) $aptObj->rate_weekend = $aptObj->rate_day;
    $unit = hourly_rate($type) ? 'hour' : 'day';
      
    $weekend_rate_cutoff = $aptObj->weekend_rate_cutoff;  /** not in use */ 
    $weekend_rate_cutoff = 0; /** turn this off for now. always use weekend rates unless weekly rate is set */
    $weekend_pricing = (!$weekend_rate_cutoff || $nd < $weekend_rate_cutoff) && $aptObj->rate_weekend > 0 && $aptObj->rate_weekend != $aptObj->rate_day; // will weekday affect the price
                
    $rate_exceptions = array_filter($exceptions, "rate_exception"); // filter out non-relevant exceptions (that don't affect the price)
    if($debug) echo(count($exceptions)." exceptions after rate filter:". print_exceptions($type, $apt_id, $exceptions, $options).html_break());
    
    list($non_loop_exceptions, $loop_exceptions) = split_exceptions($type, $rate_exceptions, $searchObj, $apt_id, $options);

    print_log("{$aptObj->id} before loop={$aptObj->currency} {$aptObj->rate_day}", 'rental-rate', LOG_LEVEL_TEST);
    if($non_loop_exceptions) { // these we can apply now
      if($debug) echo(count($non_loop_exceptions)." exceptions applied before loop:".print_exceptions($type, $apt_id, $non_loop_exceptions, $options).html_break());
      $aptObj = apply_exceptions($type, $aptObj, $non_loop_exceptions, $searchObj, $options);
    } 
    print_log("{$aptObj->id} after loop={$aptObj->currency} {$aptObj->rate_day}", 'rental-rate', LOG_LEVEL_TEST);
    
    $rate_day = $aptObj->rate_day;
    $rate_weekend = $aptObj->rate_weekend > 0 && $aptObj->rate_weekend != $aptObj->rate_day ? $aptObj->rate_weekend : 0;
  }

  
  $rate_base_ng = get_rate_num_guests($rate_day, $aptObj, $ng);
  $weekend_rate_ng = $weekend_pricing ? get_rate_num_guests($rate_weekend, $aptObj, $ng) : $rate_base_ng;

  /**
  if(isset($_GET['foo'])) {
    echo("in=$in out=$out ng=$ng day=$aptObj->rate_day base=$rate_base_ng we=$weekend_rate_ng<br>");
    echo("total_only = $total_only unit=$unit<br>");
    $debug = $options['debug'] = 1;
  }
  */
  
  if($debug) echo(count($non_loop_exceptions)." non-loop exceptions:". print_exceptions($type, $apt_id, $non_loop_exceptions, $options).html_break());
  if($debug) echo(count($loop_exceptions)." loop exceptions:". print_exceptions($type, $apt_id, $loop_exceptions, $options).html_break());
  
                                                                    
  //if($debug) echo("Rate exceptions: ".dump($rate_exceptions, true));
  if($debug) echo("Weekend pricing ? = ".($weekend_pricing ? 'true' : 'false')."<br>");
  if($debug) echo("Total ? = ".($total_only ? 'true' : 'false')."<br>");  

  // the global rate
  $priceObj->checkin = $in;
  $priceObj->checkout = $out;
  $priceObj->num_months = $nm;
  $priceObj->num_days = $nd;
  $priceObj->num_guests = $ng;
  $priceObj->weekly_pricing = $weekly_pricing;
  $priceObj->monthly_pricing = $monthly_pricing;

  $priceObj->rate_change = $aptObj->rate_change; // discount / surcharge applied before loop (but after non-loop-exceptions)
  //if(!$discount && $priceObj->rate_change) $priceObj->discount = $discount = percent2discount($priceObj->rate_change);
  $priceObj->discount = $discount = percent2discount($priceObj->rate_change);
  $priceObj->base_rate = $rate_base_ng;
  $priceObj->base_rate_weekend = $weekend_rate_ng;
  $priceObj->currency = $aptObj->currency;
  $priceObj->type = $aptObj->type;
  $priceObj->apt_id = $aptObj->id;
  $priceObj->rate_type = $unit;
  $priceObj->currency = $aptObj->currency;
  
  $weekendAr = $hourly ? array(WEEKDAY_SATURDAY, WEEKDAY_SUNDAY) : array(WEEKDAY_FRIDAY, WEEKDAY_SATURDAY);
  $priceObj->rate_class = RATE_CLASS_STANDARD; 
  $priceObj->book_direct = $aptObj->book_direct;
    
  if($total_only && !$loop_exceptions) {  // we just want the total and average - no need to iterate  
    if($weekend_pricing) { // no need to iterate since we can quickly calculate how many weekend days there are
      $weekend_count = weekday_count($in, $out, $weekendAr);
      if(!$weekend_count) $weekend_pricing = false;
      $midweek_count = $nd - $weekend_count;       
      $priceObj->total = $total = $midweek_count * $rate_base_ng +  $weekend_count * $weekend_rate_ng;
      $priceObj->average = $total / $nd;
      if($debug) echo("Total = $midweek_count * $rate_base_ng + $weekend_count * $weekend_rate_ng = $total<br>");
      $priceObj->special_rate = $weekend_count ? "Weekend" : ""; // yes, weekend pricing; 
      $priceObj->rate_class = RATE_CLASS_SPECIAL; // weekend
    } else { 
      if($debug) echo("gh: day=$rate_day base=$rate_base_ng ng=$ng<br>");
      $priceObj->total = $rate_base_ng * $nd;
      $priceObj->average = $rate_base_ng;
    }
    $priceObj->weekend_pricing = $weekend_pricing;
    
    //$priceObj->total = round($priceObj->total); /** round total */
    //if(!$priceObj->average) $priceObj->average = $priceObj->total / $nd;
    return $priceObj;
  }

  $prices = $exAr = $defaultCommentAr = array();
  $date = $in;
  $weekday = get_sql_weekday($date);
  if($non_loop_exceptions) { // store ID and comment of exceptions already applied
    //dump($non_loop_exceptions);
    if($non_loop_date_exceptions = array_filter($non_loop_exceptions, "date_exception")) {
      foreach($non_loop_date_exceptions as $exObj) $exAr[$exObj->id] = $exObj->description; 
    }
  }
  $defaultCommentAr = array_filter(array_values($exAr));
  //echo(count($defaultCommentAr)." default comments=".dump($defaultCommentAr, true));

  
  if($aptObj->source_id == SOURCE_ID_INTERHOME) {
    require_once('xml_interhome.inc');
    
    $fid = $aptObj->guid;
    $rate = $aptObj->rate_day;
    $ih_rates = interhome_pricing($fid, $in, $out, ['average' => false]);
    $count = count($ih_rates);
    $date = $in;
    $total = 0;
    
    /** find last fixrate */
    $fixed = $fixday = 0;
    while($count--) {
      $rateObj = $ih_rates[$count];
      
      if($rateObj->fixprice > 0) {
        $fixed = $rateObj->fixprice;
        break;
      }
    }
    $fixday = $fixed > 0 ? $fixed/$nd : 0;
    print_log("fixrate: $fixed per/day: $fixday", "interhome", LOG_LEVEL_TEST);
    
    for($i=0;$i < $nd; $i++) {
      $dateObj->in = $date;
      $dateObj->out = get_next_day($date);
      $weekday = get_sql_weekday($date);

      $day_rate = interhome_dayrate($ih_rates, strip_time($date));
      if(!$day_rate) {
        print_log("Oops: no ih rate for day $date".dump($ih_rates,true), "interhome", LOG_LEVEL_TEST);
        $error = "Missing rate for {$date}";
        //echo($error.dump($ih_rates,true));
        return json_error_object($error); /** Houston, we have a problem */
      }
      $day_rate += $fixday;
      
      $dayPriceObj = new stdClass;
      $dayPriceObj->date = $date;
      $dayPriceObj->weekday = $weekday;
      $dayPriceObj->currency = $aptObj->currency;
      $dayPriceObj->base_rate = $dayPriceObj->rate = $dayPriceObj->base_rate_weekend = $day_rate;
      $dayPriceObj->comment = $dayPriceObj->rate_change = '';
      $prices[$date] = $dayPriceObj;

      $total += $day_rate;
      $date = $dateObj->out;
    }
    
    
    $priceObj->prices   = $prices;

    $average = $total / $nd;
    $priceObj->average = $average = $priceObj->average_discounted = round($average, 6);    
    $priceObj->total = round($total, 2);
  
    /** city tax */
    $priceObj->city_tax = $tax;

    /** fees */
    $priceObj->fees     = $fees;

    /** service fee */
    $priceObj->service_fee = $service_fee = service_fee($total);   

    $priceObj->grand_total = round($total + $tax + $fees + $service_fee, 2);

    print_log("ih days $nd: total:$total average:$average service fee: $service_fee", "interhome", LOG_LEVEL_TEST);
    
    return $priceObj;
  } 
    
  // loop through all dates, either because we want to print (total is not set), and/or because there are loop exceptions
  $total = $week_total = $month_total = $total_actual = 0;
  for($i=0;$i < $nd; $i++) {
    $dayPriceObj = NULL;
    $thisAptObj = clone $aptObj; // reset
    if($discount) $thisAptObj->rate_change = '0'; // reset, as we don't want this in the loop. This is applied at the end

    $dateObj = clone $searchObj;
    $dateObj->in = $date;
    $dateObj->out = get_next_day($date);
    
    $commentAr = $defaultCommentAr;
    if($loop_exceptions) { // we only need to check exceptions that depend on the date and intersect the in/out range while iterating
      foreach($loop_exceptions as $exception) {
        if(exception_applies($thisAptObj, $exception, $dateObj, $options)) {
          if($debug) echo("$date: $exception->description <b>does apply</b><br>");
          $commentAr[]= $exception->description; // for this day
          $exAr[$exception->id] = $exception->description; // for all days
          $thisAptObj = exception2apt($exception, $thisAptObj, $dateObj, $options);
        } else {
          if($debug) echo("$date: $exception->description does not apply ($exception->start - $exception->end)<br>");
        }
      }  
    }
    
    
    //$debug_str = "day=$thisAptObj->rate_day we=$thisAptObj->rate_weekend";
    
    if($weekly_pricing) {
      
      $extra_guest_rate = get_rate_num_guests(0, $thisAptObj, $ng); // how much extra per day
      $week_rate = ($thisAptObj->$week_field)/$week_count + $extra_guest_rate * 7;
      $rate_base_ng = $weekend_rate_ng = round($week_rate/7, 6);
      $day_rate = $rate_base_ng;
      $commentAr[]=  round($week_rate,2)."/week";
      $week_total += $week_rate;    

      //print_log("date= rate_2_week = {$exObj->rate_2_weeks}", 'calendar', LOG_LEVEL_TEST); 
      
    } elseif($monthly_pricing) {
      $days_in_month = get_daysofmonth($date);
      $extra_guest_rate = get_rate_num_guests(0, $thisAptObj, $ng); // how much extra per day      
      $month_rate = $thisAptObj->rate_month + $extra_guest_rate * $days_in_month;
      $rate_base_ng = $month_rate_ng = round($month_rate / $days_in_month,6);
      $day_rate = $rate_base_ng;
      $commentAr[]=  round($month_rate, 2)."/month";
      $month_total += $month_rate;
    } else { // daily pricing
      $rate_base_ng = get_rate_num_guests($thisAptObj->rate_day, $thisAptObj, $ng);
      $weekend_rate_ng = get_rate_num_guests($thisAptObj->rate_weekend, $thisAptObj, $ng);
      
      if($weekend_pricing && in_array($weekday, $weekendAr)) { // weekend pricing (no exceptions apply)
        $day_rate = $weekend_rate_ng;
        $dayPriceObj->weekend_rate = 1;
        $priceObj->weekend_pricing = 1;
        $commentAr[]=  phrase('weekend', CAPITALIZE);
        //dump($commentAr);
        $priceObj->rate_class = RATE_CLASS_WEEKEND; // weekend
      } else { 
        $day_rate = $rate_base_ng;
        $dayPriceObj->weekend_rate = 0;
        $dayPriceObj->orgd = $aptObj->rate_day;
        $dayPriceObj->orgw = $aptObj->rate_weekend;
        $dayPriceObj->newd = $thisAptObj->rate_day;
        $dayPriceObj->neww = $thisAptObj->rate_weekend;
        $dayPriceObj->exception_overlap = $thisAptObj->exception_overlap;
      }
    }
    //$debug_str = " c=".count($commentAr);
    $comment = implode(', ', array_filter($commentAr)).$debug_str;

    $dayPriceObj->date = $date;
    $dayPriceObj->weekday = $weekday;
    $dayPriceObj->currency = $aptObj->currency;
    $dayPriceObj->base_rate = $rate_base_ng;
    $dayPriceObj->base_rate_weekend = $weekend_rate_ng;
    $dayPriceObj->rate_change = $rate_change = $thisAptObj->rate_change;
    $dayPriceObj->rate = $actual_rate = apply_surcharge($day_rate, $rate_change);  // actual rate (e.g. after discount / surcharge)  
    //$dayPriceObj->difference = $diff ?: '';
    $dayPriceObj->comment = $comment;

    $different_from_base = $comment || $dayPriceObj->base_rate != $priceObj->base_rate || $dayPriceObj->base_rate_weekend != $priceObj->base_rate_weekend || $dayPriceObj->rate_change;
    if($different_from_base) { // only store if different from base
      $prices[$date] = $dayPriceObj;
    }
    $date = $dateObj->out; // next day
    $weekday = next_weekday($weekday);
    
    $total += $day_rate;
    $total_actual += $actual_rate;
    print_log("day: $day_rate actual: $actual_rate total:$total_actual", "debug", LOG_LEVEL_DEBUG);
  }

  //$priceObj->discount = 0;
  //if($priceObj->rate_change) {
  //  list($abs, $perc) = split_percentage($priceObj->rate_change);
  //  if($perc < 0) $priceObj->discount = (-1*$perc).'%';
  //  //elseif($perc > 0) $priceObj->average = apply_surcharge($priceObj->average, $priceObj->rate_change); // apply surcharge quietly
  //}
  
  if(count($exAr)) {
    //echo("There are ".count($exAr)." rate affecting exceptions");
    //dump($exAr);
    $priceObj->exceptions =  implode(',', array_keys($exAr)); // reason for special price
    $priceObj->rate_class = RATE_CLASS_SPECIAL; // exception    
  }
  if($priceObj->weekend_pricing) $exAr[0] = phrase('weekend', CAPITALIZE); 
  $priceObj->reason = implode(', ', array_values($exAr));
  
  if($weekly_pricing) {
    $priceObj->average_weekly = round($week_total / $nd, 6);
  }
  if($monthly_pricing) {
    $priceObj->average_monthly = round($month_total / $nd, 6);
  }                   
  
  /** todo: don't override existing discount */
  if($nd >= 28 && $aptObj->discount_month > 0) { // monthly discount, ignore monthly rates and exceptions
    //$priceObj->rate_change = -1*$aptObj->discount_month."%";
    $discount = $aptObj->discount_month."%";  
  } elseif($nd >= 21 && $aptObj->discount_3_weeks > 0) { // weekly discount, ignore weekly rates and exceptions
    $discount = $aptObj->discount_3_weeks."%";
  } elseif($nd >= 14 && $aptObj->discount_2_weeks > 0) { // weekly discount, ignore weekly rates and exceptions
    $discount = $aptObj->discount_2_weeks."%";
  } elseif($nd >= 7 && $aptObj->discount_week > 0) { // weekly discount, ignore weekly rates and exceptions
    $discount = $aptObj->discount_week."%";
  }
  
  if($discount) { // global discount
    $average = $total_actual / $nd;
    $priceObj->discount = $discount;
    $priceObj->average = round($average, 6);
    $priceObj->total = $total_actual; //round($average * $nd, 2); // get rid of rounding errors so it'll match res    
    $priceObj->total_discounted = $total_actual_discounted = round(apply_discount($total_actual, $discount), 2);  // after discount
    $priceObj->average_discounted = round(apply_discount($average, $discount), 2);  // after discount
  } else {
    $average = $total_actual / $nd;
    $priceObj->average = $average = round($average, 6);    
    $priceObj->total = $total_actual_discounted = round($total_actual, 2);
    unset($priceObj->total_discounted);
    unset($priceObj->average_discounted);
  }

  //$priceObj->total = round($priceObj->total); /** round total */
  //$priceObj->total_discounted = round($priceObj->total_discounted); /** round total */

  /**
  if(isset($options['fees']) || isset($searchObj->fees)) { // explicitly set (from reservation?)
    $fees = $options['fees'] ?: $searchObj->fees;
  } else {
    $feeArray = get_property_fees($aptObj) ?: array(); // calculate
    foreach($feeArray as $k => $v) $priceObj->$k = $v;
    $fees = $feeArray['fees'] ?: 0;
  }

  $charge_city_tax = $aptObj->charge_city_tax;
  if(isset($options['city_tax']) || isset($searchObj->city_tax)) { // explicitly set (from reservation)
    $tax = $options['city_tax'] ?: $searchObj->city_tax;
  } elseif($aptObj->charge_city_tax) {
    $city_tax_rate = get_city_tax_rate($aptObj->city_id);
    $tax = $city_tax_rate ? apply_surcharge($total_actual_discounted, $city_tax_rate, true) : 0; 
    print_log("charge:{$aptObj->charge_city_tax} rate:{$city_tax_rate} tax:$tax", "rental-rate", LOG_LEVEL_TEST);
  } else {
    $tax = 0; // calculate
  }

  */

  // apt2res
  $flds = ['client_id', 'site_id', 'legal_entity_id', 'corporate'];
  foreach($flds as $fld) {
    $priceObj->$fld = $aptObj->$fld;
  }
  
  /** bill monthly */
  $priceObj->bill_monthly = 1;
  
  /** rental fees */
  list($fees, $fee_rows) = rental_fees($aptObj);
  $priceObj->fee_rows = $fee_rows;
  $priceObj->fees = $fees;
  
  /** city tax */
  $city_tax_rate = get_city_tax_rate($aptObj->city_id) ?: ''; /** store with res in case turned on later */
  $city_tax = 0;
  if($charge_city_tax = $aptObj->charge_city_tax) {
    $tax_rate_hi = 1.21; /** todo: not static */
    $tax_rate_lo = 1.06; /** todo: not static */
    $total_actual_discounted_ex_tax = $aptObj->charge_tax && $aptObj->tax_included ? $total_actual_discounted / $tax_rate_lo : $total_actual_discounted;
    $city_tax = $city_tax_rate ? apply_surcharge($total_actual_discounted_ex_tax, $city_tax_rate, true) : 0; 
  }
  
  /** service fee */
  global $__SITE;
  $charge_service_fee = $__SITE->charge_service_fee ?: 0;
  $service_fee = $charge_service_fee ? service_fee($total_actual_discounted) : 0;
  
  $priceObj->fees     = $fees;
  $priceObj->prices   = $prices;
  $priceObj->grand_total = $grand_total = round($total_actual_discounted + $city_tax + $fees + $service_fee, 2);  
  $priceObj->city_tax = $city_tax;
  $priceObj->charge_service_fee = $charge_service_fee;
  $priceObj->service_fee = $service_fee;

  $priceObj->city_tax_rate = $city_tax_rate;
  $priceObj->charge_city_tax = $charge_city_tax;
  $priceObj->charge_tax = $aptObj->charge_tax;
  $priceObj->tax_included = $aptObj->tax_included;
  
  
  print_log("total:$total_actual ($total_actual_discounted) + tax:$tax + fees:$fees service:$service_fee gt=$grand_total", "rental-rate", LOG_LEVEL_TEST);
  
  //$priceObj->base_rate = $checkin;
  //$priceObj->rate_change = $r;
  //dump($priceObj);
  return $priceObj;
}
                                                                         
function print_price_breakdown($aptObj, $priceObj, $options=NULL) {
  
  print_log(dump($priceObj,true), 'calendar', LOG_LEVEL_TEST);
  $type = $priceObj->type;
  $searchObj = $priceObj;
  $interhome = $aptObj->source_id == SOURCE_ID_INTERHOME;
  
  $currency = $priceObj->currency ?: $aptObj->currency;
  $cur = get_currency_symbol($currency, true);
  $discount = $priceObj->discount;

  $prices = $priceObj->prices ?: array();  
  if(!$prices) $prices = array(); //return 'Price breakdown: No prices received';

  $ng = $num_guests = $priceObj->num_guests ?: 1;
  $nd = $num_days = $priceObj->num_days;
  $nm = $num_months = $priceObj->num_months;
  
  $base_rate = $priceObj->base_rate;
  $base_rate_weekend = $priceObj->base_rate_weekend;
  $weekend_pricing = $priceObj->weekend_pricing;
  $weekendAr = $hourly ? array(WEEKDAY_SATURDAY, WEEKDAY_SUNDAY) : array(WEEKDAY_FRIDAY, WEEKDAY_SATURDAY);

  if($priceObj->weekly_pricing) {
    $rate_type_str = phrase('average_weekly_rate', CAPITALIZE);
    $rate_string = formatFloat($priceObj->average_weekly, FORMAT_ALWAYS, $cur);
  } elseif($priceObj->monthly_pricing) {
    $rate_type_str = phrase('average_monthly_rate', CAPITALIZE);
    $rate_string = formatFloat($priceObj->average_monthly, FORMAT_ALWAYS, $cur);
  } else {
    $rate_type_str = phrase('average_daily_rate', CAPITALIZE);
    //$rate_string = formatFloat($base_rate, FORMAT_ALWAYS, $cur);
    $rate_string = formatFloat($priceObj->average, FORMAT_ALWAYS, $cur);
    if($weekend_pricing) {
      $weekend_rate_string = formatFloat($base_rate_weekend, FORMAT_ALWAYS, $cur);
      $weekend_price_str = phrase('weekend_rate', CAPITALIZE).": ".html_span($weekend_rate_string, 'weekend_rate').html_break();
    }
  }
  
  
  if($prices) {
    $has_change = array_filter(array_find_key($prices, 'rate_change'));
    //$has_reason = array_filter(array_find_key($prices, 'reason'));
    $has_notes = array_filter(array_find_key($prices, 'comment'));
  }
    
  $date = $priceObj->checkin;
  $num_days = $priceObj->num_days;
  $weekday = get_sql_weekday($date);  
  for($i = 0; $i < $num_days; $i++) {
    if($dayPriceObj = $prices[$date]) {
      $dayPriceObj->stored = 1;
    } else { // standard rates
      $dayPriceObj = clone $priceObj;
      $dayPriceObj->stored = 0;
      unset($dayPriceObj->comment);
      $dayPriceObj->date = $date;
      $dayPriceObj->weekday = $weekday;
      if($weekend_pricing && in_array($weekday, $weekendAr)) {
        $dayPriceObj->rate = $dayPriceObj->base_rate_weekend; 
        $dayPriceObj->weekend_rate = 1;
        $dayPriceObj->comment = phrase('weekend', CAPITALIZE);
        $has_notes = true;
      } else {
        $dayPriceObj->weekend_rate = 0;
        $dayPriceObj->rate = $dayPriceObj->base_rate;
      }
      if($discount) unset($dayPriceObj->rate_change);
      //unset($priceObj->rate_change); // don't apply this here
    }
   
    $sign = floatval($dayPriceObj->rate_change) > 0 ? '+' : '';  
    $rate_change = $dayPriceObj->rate_change ? $sign.$dayPriceObj->rate_change : '';
    
    $row = array(sql2human($date, array('show_weekday' => false)), $_SESSION['weekday_names'][$weekday]);    
    if($has_change) {
      $std_rate = $dayPriceObj->weekend_rate ? html_span(formatFloat($dayPriceObj->base_rate_weekend, FORMAT_ALWAYS, $cur), 'weekend_rate') : formatFloat($dayPriceObj->base_rate, FORMAT_ALWAYS, $cur);
      $row[] = $std_rate;
      $row[] = $rate_change;
    }
    
    $day_rate = formatFloat($dayPriceObj->rate, FORMAT_ALWAYS, $cur); /** printed rate */
    
    $stored = $dayPriceObj->stored;
    //$day_rate .= "stored: $stored";
    $row[] = $dayPriceObj->weekend_rate ? html_span($day_rate, 'weekend_rate') : $day_rate;

    if($has_notes) $row[] = $dayPriceObj->comment;
    
    $rows[] = $row;
    $date = add_day($date, 1);
    $weekday = next_weekday($weekday);
    //$total_std += $dayPriceObj->base_rate;
    //$total += $dayPriceObj->rate;
  }

  //$average_std = round($total_std/count($prices), 2);
  //$average = round($total/count($prices), 2);
  //$total = round($average * count($prices), 2); // makes it identical to reservation, in case of 1 cent rounding error

  // begin rows
  $first_row = array(phrase('date', CAPITALIZE), phrase('weekday', CAPITALIZE));
  if($has_change) {
    $first_row[] = phrase('standard', CAPITALIZE);
    $first_row[] = phrase('change', CAPITALIZE);
  }
  $first_row[] = phrase('price', CAPITALIZE);
    
  if($has_notes) $first_row[] = phrase('notes', CAPITALIZE);
  
  array_unshift($rows, $first_row);
  $caption = phrase('price_breakdown', CAPITALIZE)." $aptObj->name".html_break().
                    print_time_range($searchObj->in, $searchObj->out). " ($nd ".phrase('days').')'.html_break().
                    "$rate_type_str: $rate_string".html_break().$weekend_price_str;
  if($discount) $caption .= " - $discount = ".formatFloat(apply_discount($priceObj->average, $discount), FORMAT_ALWAYS, $cur);
  
  $pad_before = $has_change ? 3 : 1;
  $pad_after = count($first_row) - ($pad_before + 2);
  $ar_before = array_fill(0, $pad_before, '');
  $ar_after = $pad_after >0  ? array_fill(0, $pad_after, '') : array();

  if($discount) {
    $rows[] = array_merge($ar_before, array(html_b(phrase('discount',  CAPITALIZE)), html_b('-'.$discount)), $ar_after);
    //$rows[] = array_merge($ar_before, array(html_b(phrase('discount_rate',  CAPITALIZE)), html_b(formatFloat(apply_discount(, FORMAT_ALWAYS, $cur))), $ar_after);
  }

  //$total_row   = array(html_b(phrase('total',  CAPITALIZE)), html_b(formatFloat($total, FORMAT_ALWAYS, $cur)));
  //$average_row = array(html_b(phrase('average',  CAPITALIZE)), html_b(formatFloat($average, FORMAT_ALWAYS, $cur)));  
  $average_row = array(html_b(phrase('average',  CAPITALIZE)." (*)"), html_b(print_rate($cur, $priceObj->average)));  

  // total, tax, fees, grand total
  $tax = $priceObj->city_tax;
  $fee = $priceObj->fees;
  $total = $priceObj->total;
  
  $total_row   = array(html_b(phrase('total',  CAPITALIZE)), html_b(print_rate($cur, $total, $discount)));
  if($tax > 0 || $fee > 0) {
    $gtotal = apply_discount($total, $discount) + $fee + $tax;
    if($fee) $fee_row = array(html_b('+ '.phrase('fees',  CAPITALIZE)), html_b(print_rate($cur, $fee)));  
    if($tax) $tax_row = array(html_b('+ '.phrase('city_tax',  CAPITALIZE)), html_b(print_rate($cur, $tax)));  
    $gt_row   = array(html_b('= '.phrase('grand_total',  CAPITALIZE)), html_b(print_rate($cur, $gtotal)));
  }
  $rows[] = array_merge($ar_before, $total_row, $ar_after);
  if($fee >0) $rows[] = array_merge($ar_before, $fee_row, $ar_after);
  if($tax >0) $rows[] = array_merge($ar_before, $tax_row, $ar_after);
  if($gtotal) $rows[] = array_merge($ar_before, $gt_row, $ar_after);
  $rows[] = array_merge($ar_before, $average_row, $ar_after);


  if($interhome) {
    $note = '(*) The rates are approximate. Actual live rates will be shown at time of booking.';
  } else {
    $note = '(*) Note that the actual average day rate uses 6 decimal places<br>for better rounding, so the total may appear to be a few cents off.';
  }
  $rows[] = array($note);
  $output = html_table($rows, array('title' => $caption, 'class' => 'web20', 'footer' => 1));
  
  return $output;
}



// returns just the fit factor given a search and apt_id
function fit_factor($searchObj, $apt_id, $options=NULL) {
  $fit_type = $options['fit_type'] ?: 'best';
  $ffObj = $options['fit_factor_object'] ?: fit_factor_object($searchObj, $apt_id, $options);
  $key = 'fit_'.$fit_type;
  return $ffObj->$key;
}


// returns URL of image with graphical presentation of fit factor
// function print_fit_factor($searchObj, $apt_id, $options=NULL) {
function fit_factor_image($ffObj, $options=NULL) {
  $ffObj2 = clone $ffObj;
  $fit_type = $options['fit_type'] ?: 'best';
  switch($fit_type) {
    case 'before':
      $ffObj2->gap_before = 0;
      if(!$ffObj2->fit_before) return '';
      $ffObj2->fit_factor = $ffObj->fit_before;
      break;
    case 'after':
      $ffObj2->gap_after = 0;
      if(!$ffObj2->fit_after) return '';
      $ffObj2->fit_factor = $ffObj->fit_after;
      break;
    case 'between':
      if(!$ffObj2->fit_between) return '';
      $ffObj2->fit_factor = $ffObj->fit_between;
      $ffObj2->fit_type = 'between';
      break;
    case 'next_to':
      if(!$ffObj->fit_before && !$ffObj->fit_after) return '';
      
      if($ffObj2->fit_before > $ffObj2->fit_after) {
        $ffObj2->gap_before = 0;
        $ffObj2->fit_factor = $ffObj->fit_before;
        $ffObj2->fit_type = 'before';
      } else {
        $ffObj2->fit_factor = $ffObj->fit_after;
        $ffObj2->gap_after = 0;
        $ffObj2->fit_type = 'after';
      }
      break;
    case 'best':
      $ffObj2->fit_factor = $ffObj->fit_best;
      break;
    default:
      return '';
      break;
  }
  //echo("type=$fit_type".dump($ffObj2, true)); return '';
  $ffObj2->gap_count = $ffObj2->gap_before + $ffObj2->unit_count + $ffObj2->gap_after; 
  return "/afr/home/fit_factor.php?".array2qs(obj2array($ffObj2));

  //$ffObj = fit_factor_object($searchObj, $apt_id, $options);
  $fit_type = $ffObj->fit_type; 
  $gap_count = $ffObj->gap_count;
  $gap_before = $ffObj->gap_before;
  $gap_after = $ffObj->gap_after;
  $fit_factor = $ffObj->fit_factor;
  $unit_count = $ffObj->unit_count;
  $unit = $ffObj->unit;           
  $unit_str = phrase($unit.'s', CAPITALIZE);
  if(!$gap_count || !$fit_factor || !in_array($fit_type, array('before', 'after', 'between'))) return '';  
}
                                                                                             
// calculate fit factor, return ffObj (see comments in code below for details)
// %age of gap a request fills between 2 other bookings
// OR if first or last booking, %age filled by booking in an imaginary gap created by adding an equal number of open days to other side of booking
function fit_factor_object($searchObj, $apt_id, $options=NULL) {
  $status = $options['status'] ?: STATUS_RESERVED; 
  $ptype = $searchObj->pt ?: get_object('property', $apt_id, 'type');
  //$fit_type = $options['fit_type'] ?: 'best'; // valid values are between / before / after / next_to (before or after), best
  
  $in = $searchObj->in;
  $out = $searchObj->out;
  $hourly = hourly_rate($ptype);

  if($hourly) {
    $start = strip_date($in);
    $end = strip_date($out);
    if(!is_time($start)) $invalid = "invalid time $start";
    if(!is_time($end)) $invalid = "invalid time $end";
  } else {
    $in = strip_time($in);
    $out = strip_time($out);   
  }         

  if(!$invalid && $in >= $out) $invalid = "in >= out"; // invalid
  if(!$invalid && !calendar_availability($apt_id, $in, $out)) $invalid = "not available";  // not available, invalid
            
  if($invalid) {
    $searchObj->fit_type = $invalid;
    $searchObj->fit_factor = 0;    
    return $searchObj;
  }
    
                   
  $unit = $hourly ? 'hour' : 'day';
  
  $resSearch['apt_id'] = $apt_id;
  $resSearch['status'] = array('>=', $status);
  $unit_count = period_diff($in, $out, $unit); // whole days, 2 decimal hours
  if($hourly) {
    $aptObj = get_object($ptype, $apt_id);
    $booking_gap = $aptObj->booking_gap;
    if($start_hour = $aptObj->start_hour) {
      $start_out = strip_time($in).' '.clean_time($start_hour);
      //echo("start out = $start_hour => $start_out<br>");
    }
    
    if($end_hour = $aptObj->end_hour) {
      $end_in = strip_time($out).' '.clean_time($end_hour);
      //echo("end hour = $end_hour => $end_in<br>");
    }
    
    $resSearchLast['checkout'] = array('<=', $in);
    $resSearchFirst['checkin'] = array('>=', $out);
    
  } else {
    $resSearchLast['checkout'] = array('<=', $in);
    $resSearchFirst['checkin'] = array('>=', $out);
  }
  
  $last_out = find_object('reservation', array_merge($resSearch, $resSearchLast), "max(checkout)");
  $first_in = find_object('reservation', array_merge($resSearch, $resSearchFirst), "min(checkin)");
  if($booking_gap > 0) { // number of hours required between bookings (hourly reservations)
    if($first_in) $first_in = add_interval($first_in, dectime2hm(-1 * $booking_gap), 'HOUR_MINUTE'); // only add gap if there is preceding booking
    if($last_out) $last_out = add_interval($last_out, dectime2hm($booking_gap), 'HOUR_MINUTE'); // only add gap if there is following booking
  }    
    
  if($hourly) {
    if($start_out) $last_out = max($last_out, $start_out);
    if($end_in) $first_in = min($first_in, $end_in);
  } else {
    $last_out = strip_time($last_out);
    $first_in = strip_time($first_in);
  }

  $gap_after = $first_in ? period_diff($out, $first_in, $unit) : 0;
  $gap_before = $last_out ? period_diff($last_out, $in, $unit) : 0; // gap 
  $gap_count = $first_in && $last_out ? period_diff($last_out, $first_in, $unit) : 0; // gap between two bookins
  
  // should be same as gap_before + unit_count + gap_after
  //if($last_out) echo("Gap between $last_out and $in = ".period_diff($last_out, $in, $unit)." $unit".html_break());
  
  $fit_factor = 0;
  $not_between_penalty = 1;
  //$not_between_penalty = 0.75;
  if($unit_count > 0) {                                         
    if($gap_count > 0)  $fit_between = $fit_factors['between'] = round(($unit_count / $gap_count) * 100);
    if($first_in)  $fit_before = $fit_factors['before']  = round(($unit_count / ($unit_count + $gap_after))  * 100 * $not_between_penalty);
    if($last_out) $fit_after = $fit_factors['after']   = round(($unit_count / ($unit_count + $gap_before)) * 100 * $not_between_penalty);
    $fit_factors['next_to'] = $fit_next_to = max($fit_before, $fit_after);
    $fit_factor = $fit_factors['best'] = $fit_best = max($fit_factors); // pick the best of the three fit factors
    $fit_type = array_search($fit_factor, $fit_factors);
    //dump($fit_factors);
  }
  //if($fit_type == 'before') $gap_before = 0;
  //if($fit_type == 'after') $gap_after = 0;
  $ffObj->gap_before = $gap_before;  // the number of hours/days in the gap between last booking and this booking
  $ffObj->gap_after = $gap_after;    // the number of hours/days in the gap between this booking and next booking
  $ffObj->in = $in;                  // the checkin of the search
  $ffObj->out = $out;                // the checkout of the search
  $ffObj->first_in = $first_in;      // the first checkin after search
  $ffObj->last_out = $last_out;      // the last checkout before search
  $ffObj->unit = $unit;              // unit: 'hour' or 'day'
  $ffObj->unit_count = $unit_count;  // the number of hours/days of the search
  $ffObj->gap_count = $fit_type == 'between' ? $gap_count : $gap_before + $unit_count + $gap_after;    // the number of hours/days in the gap
  $ffObj->fit_type = $fit_type;      // the type of fit ('between', 'before', 'after')

  //$ffObj->fit_factor = $fit_factor > 100 ? 0 : round($fit_factor,1);  // the fit factor (% of gap filled by booking)
  $ffObj->fit_before =  $fit_before;
  $ffObj->fit_after =   $fit_after;
  $ffObj->fit_next_to = $fit_next_to;
  $ffObj->fit_between = $fit_between;
  $ffObj->fit_best =    $fit_best;

  //dump($fit_factors);
  //dump($ffObj);
  return $ffObj;  
}

  
//fit factor: helper to 
//$ar = array(
//  array('in' => '2012-11-06 08:00:00', 'out' => '2012-11-08 12:00:00'), 
//}
/** unsused - automatic comment added on  */
function test_fit_factor($ar, $type, $apt_id, $options=NULL) {
  $rows = array();
  $first_row = array('in', 'out', 'last_out', 'first_in', 'unit', 'unit_count', 'gap_count', 'gap_before', 'gap_after', 'fit_type', 'fit_before', 'fit_after', 'fit_next_to', 'fit_between', 'fit_best');
  $rows[] = $first_row;
  $apt_name = get_object($type, $apt_id, 'name'); // D
  foreach ($ar as $a) {
    $row = array();
    $time = strip_date($a['in']);
    $is_time = is_time($time);
    $ffObj = fit_factor_object(array2obj($a), $apt_id, $options);
    //dump($ffObj);
    foreach($first_row as $var) $row[] = $ffObj->$var;
    //$row[] = "$time is time? = ".($is_time ? 'yes' : 'no');
    $fit_between_img = $ffObj->fit_between && ($img = fit_factor_image($ffObj, array('fit_type' => 'between'))) ? html_image($img, $ffObj->fit_between.'%') : '';
    $fit_next_to_img = $ffObj->fit_next_to && ($img = fit_factor_image($ffObj, array('fit_type' => 'next_to'))) ? html_image($img, $ffObj->fit_next_to.'%') : '';
    $row[] = $fit_between_img;
    $row[] = $fit_next_to_img;    
    $rows[] = $row;                                                                                
  }
  return html_break(2).html_table($rows, array('title' => $apt_name, 'class' => 'web20'));
}

function update_reservation_chat_count() {

  $countObjs = db_getAll("select parent_id as res_id,count(*) as count from chat where parent_type = 'reservation' group by parent_id order by count desc");
  foreach($countObjs as $countObj) {
    $count = $countObj->count;
    $res_id = $countObj->res_id;
    $qs = "update reservation set chat_count = $count where res_id=$res_id";
    //echo("$qs<br>");
    db_query($qs);
  }
}

// thread is a string user1-user2 (or user1-object_id)
// this helper returns the user that is NOT $user_id (if it's an int)
function chat_thread_user($thread, $user_id) {
  list($u1,$u2) = explode('-', $thread);
  $other_user_id = $u1 == $user_id ? $u2 : $u1;
  return isint($other_user_id) ? $other_user_id : 0;
}

function booking_type_name($resObj_or_id) {
  $status = is_object($resObj_or_id) ?  $resObj->status : get_object('reservation', $resObj_or_id, 'status');
  switch($status) {
    case STATUS_DRAFT:
    case STATUS_INQUIRY:
      return "Inquiry";
    case STATUS_REQUEST:
      return "Request";
    default:
      return "Booking";
  }
}


function add_special_offer($resObj, $total) {
  global $u_id;
  $res_id = $resObj->res_id;
  
  $offerData = ['rate_type' => 'total', 'rate_base' => $total, 'user_id' => $u_id];
  foreach(['res_id', 'site_id', 'guest_id', 'host_id', 'apt_id', 'property_type', 'checkin', 'checkout', 'num_guests', 'num_days', 'currency'] as $key) $offerData[$key] = $resObj->$key;

  if($offerObj = find_object('special_offer', ['res_id' => $res_id, 'active' => 1])) {
    $offer_id = $offerObj->id;
    update_object('special_offer', ['active' => 0], $offer_id);
  } 
  
  //else {
  $offerData['org_currency'] = $resObj->currency;
  $offerData['org_rate_type'] = $resObj->rate_type;
  $offerData['org_rate_base'] = $resObj->rate_base;
  $offerData['org_fee_cleaning'] = $resObj->fee_cleaning;
  //json_error("add offer".dump($offerData, true));
  return add_object('special_offer', $offerData);
  //}
}




// hack for now: 
//if($resObj->property_type == 'boat')  && $city_id==1000) $tt_rate = '2%';


// Returns exception for a given host/type:
// - For one apartment
// - For all apartments for this host in this city
// - For all apartments for this host
// - For a given organization (contract)

function print_host_exceptions($type, $host_id, $city_id, $apt_id) {
  global $u_hostid;
  $aptObj = get_object($type, $apt_id);  
  $city_name = get_object('geo_city', $city_id, 'name');  
   
  $host_id = pick_first($host_id, $aptObj->host_id, $u_hostid);
  if($host_id && ($hostObj = get_object('host', $host_id))) $host_name  = get_name_string($hostObj);

  $search['property_type'] = $type;
  $search['apt_id'] = $apt_id;
  $search['host_id'] = $host_id;
  $search['city_id'] = $city_id;
    
  // For this apartment
  if($apt_exceptions = host_exceptions($type, $host_id, $city_id, $apt_id)) {
    $host_exceptions .= html_subheading("For $aptObj->name only");
    $host_exceptions .= print_exceptions($type, $apt_id, $apt_exceptions, array('can_edit' => true, 'search' => $search));
  }
  
  $search['apt_id'] = ATT_DEFAULT_NULL;
  
  // For all apartments for this host in this city
  $type_str = property_type_name($type, 2);
  $host_exceptions .= html_subheading("For all $type_str in $city_name (for $host_name)");
  $host_exceptions .= print_exceptions($type, $apt_id, host_exceptions($type, $host_id, $city_id), array('can_edit' => true, 'search' => $search));

  // For all apartments for this host
  $host_cities = find_objects($aptObj->type, array('host_id' => $aptObj->host_id), array('fields' => 'distinct city_id'));
  if(count($host_cities) > 1) {
    $search['city_id'] = ATT_DEFAULT_NULL;
    $host_exceptions .= html_subheading("All $type_str (for $host_name)");
    $host_exceptions .= print_exceptions($type, $apt_id, host_exceptions($type, $host_id), array('can_edit' => true, 'search' => $search));
  }
  
  return $host_exceptions;
}
     
function unit_string_map($var, $val, $hourly, $currency) {

  //if($var == 'num_months') return unit_string('month', $val);
  if($var == 'num_hours') return unit_string('hour', $val);
  if($var == 'num_days') return unit_string('day', $val);
  
  if(in_array($var, array('days_from_now', 'days_from_now_gt'))) return unit_string('day', $val).' '.phrase('from_now');

  $rate_unit = $hourly ? 'hour' : 'day'; // todo: handle months ? 

  if(in_array($var, array('rate_day', 'rate_extraperson', 'rate_hour'))) return "$currency / ".unit_string($rate_unit, 1);
  if(in_array($var, array('min_stay', 'max_stay'))) return unit_string($rate_unit, $val);
  if(in_array($var, array('discount', 'surcharge'))) return strpos($val, '%') ? '' : $currency." / ".unit_string($rate_unit, 1);


  return '';
  
}

// echo("$aptObj->name FF: $search_ff >= $exception->fit_factor<br>");  // check fit factor                                                        
// echo("$aptObj->name DFN: $days_from_now <= $exception->days_from_now<br>"); // check days from now (lte)                                        
// echo("$aptObj->name DFN_gt: $days_from_now > $exception->days_from_now_gt<br>"); // check days from now (gt)                                    
// echo("$aptObj->name ND: $searchObj->nd >= $exception->num_days<br>"); // check length of stay                                                   
// echo("$aptObj->name DI: $exception->start:$exception->end X $searchObj->in:$searchObj->out=$days_overlap days overlap<br>"); // check date range

// handle 'org_id' (contracts)

function print_exceptions($type, $apt_id, $exceptions, $options=NULL) {
  if(!$exceptions) return '';
  if(!is_array($exceptions)) $exceptions = array($exceptions); 
  $debug = $options['debug'];
  $can_edit = $options['can_edit'];
  
  // priority
  $count = count($exceptions);
  $i = 0;  

  if($can_edit) {
    $data = array('type' => $type, 'apt_id' => $apt_id);
    $data_json = json_encode($data);  
    
    $data['search'] = $options['search'];      
    $search_json = json_encode($data);
  }

  //$currency = get_object($type, $apt_id, 'currency'); // todo: get from city ? 
  
  foreach($exceptions as $ex) {
    $items[] = print_exception($ex, $options);
    //$items[] = $debug ? "$ex->id: $item" : $item;
    $i++;
  }
  return $options['plaintext'] ? implode(PHP_EOL, $items) : html_ul($items);
}

function print_exception($ex, $options=NULL) {
  $in_vars = array('start' => 'From', 'end' => 'to', 'num_guests' => 'Number of guests >=', 'num_days' => 'Length of stay >=', 'num_hours' => 'Length of stay >=', /*'num_months' => 'Length of stay >=',*/ 'days_from_now' => 'Checkin <=', 'days_from_now_gt' => 'Checkin >=', 'fit_between' => 'Fit between >=', 'fit_next_to' => 'Fit next to >=');
  $out_vars = array('book_direct' => 'Can book direct', 'min_stay' => 'Minimum stay', 'max_stay' => 'Maximum stay', 'max_guests' => 'Maximum guests', 'rate_day' => 'Day rate', 'rate_extraperson' => 'Extra person rate', 'rate_change' => 'Rate Change', 'rate_week' => 'Weekly rate', 'rate_2_weeks' => "Rate for 2 weeks", 'rate_3_weeks' => "Rate for 3 weeks", 'rate_month' => 'Monthly rate', 'changeover' => "Changeover", 'last' => 'Last rule');

  $ptype = $ex->property_type;
  $hourly = hourly_rate($ptype);
  $in = $out = $links = array();
  foreach($in_vars as $in_var => $in_name) {
    $val = $ex->$in_var;
    if(!strlen($val)) continue;
    //if($in_var == 'num_months') {
    //  $rate_unit = phrase('month'); // todo: handle months ? 
    //  $rate_unit_plural = phrase('months'); // todo: handle months ? 
    //}
    $val = in_array($in_var, array('start', 'end')) ? sql2human($val) : $val;
    $unit = unit_string_map($in_var, $val, $hourly, $currency);
    $in[] = "$in_name $val $unit";
  }
  foreach($out_vars as $out_var => $out_name) {
    $val = $ex->$out_var;
    if(!strlen($val)) continue;
    $unit = unit_string_map($out_var, $val, $hourly, $currency);
    if($out_var == 'last') {
      if(!$val) continue;
      $val = "Yes";
    } elseif($out_var == 'changeover') {
      $val = WEEKDAYS[$val];
    }
    if($out_var == 'book_direct') $val = $val ? "Yes" : "No";
    if($out_var == 'rate_change') {
      list($abs_change, $perc_change) = split_discount($val);
      if(!$abs_change && !$perc_change) continue;
      if($abs_change > 0 || $perc_change > 0) {
        $out_name = phrase('surcharge', CAPITALIZE);
      } else {
        $out_name = phrase('discount', CAPITALIZE);
        $val = ltrim($val, '-');
      }
    }
    if(strlen($val)) $out[] = "$out_name: $val $unit";
  }
  if($can_edit) { // we're in edit-property and can edit exceptions

    $data['id'] = $ex->id;
    $ex_json = json_encode($data);      

    $type = get_object('property', $apt_id, 'type');
    $link = array();                                              
    $links[] = html_link('#', '<i class="fa fa-pencil-square"></i>', array('js' => "onclick='load_div(\"load_exception_form\", \"add_exception_div\", $ex_json);return false'"));
    $links[] = html_link('#', '<i class="fa fa-times"></i>', array('js' => "onclick='if(!confirm(\"Are you sure you want to permanently delete this exception?\")) return false; delete_object(\"res_exception\", $ex->id, {\"handler\": \"print_host_exceptions\", \"handler_data\": $data_json});return false'"));

    if($count > 1 && $i > 0) $links[] = html_link('#', '<i class="fa fa-arrow-up"></i>', array('js' => "onclick='operation_object(\"res_exception\", $ex->id, \"rank_up\", {\"handler\": \"print_host_exceptions\", \"handler_data\": $data_json}, $search_json);return false'"));
    if($count > 1 && $i < $count-1) $links[] = html_link('#', '<i class="fa fa-arrow-down"></i>', array('js' => "onclick='operation_object(\"res_exception\", $ex->id, \"rank_down\", {\"handler\": \"print_host_exceptions\", \"handler_data\": $data_json}, $search_json);return false'"));
    $links_str = implode(' ', $links);                                         
  }
  $reason = $ex->description ? " ($ex->description) " : "";
  $item = implode(' - ', $in).' => '.implode(' - ', $out)." $reason";
  if($apt_id) $item .= " $links_str";
  return $item;
}

function find_surcharge_exceptions($aptObj, $searchObj=NULL) {
  global $db;
  if(!$aptObj) return null;
  if(!$aptObj->id) die("find_surcharge_exceptions: ".caller_stack().dump($aptObj, true));
  $today = today();
  $qs = "SELECT * FROM res_exception 
          WHERE 
                active=1 AND ((host_id = $aptObj->host_id AND apt_id IS NULL) OR apt_id = $aptObj->id) AND 
                rate_change > 0 AND end > '$today' AND start <= end AND start IS NOT NULL
          ORDER BY start";
  //if(isset($_GET['foo'])) echo($qs);
  return db_getAll($qs);
}  



// mini version of price_breakdown - calculates new base_rate (average rate using only weekend pricing, nothing else)
/** unsused - automatic comment added on  */
function apply_weekend_pricing($type, $aptObjs_or_ids, $searchObj) {
  $in  = $searchObj->in;
  $out = $searchObj->out;
  $ng  = $searchObj->ng;
  $nd  = $searchObj->nd;
  $result = array();
  if(!is_array($aptObjs_or_ids)) $aptObjs_or_ids = array($aptObjs_or_ids); // single apt

  $hourly = hourly_rate($type);
  $weekendAr = $hourly ? array(WEEKDAY_SATURDAY, WEEKDAY_SUNDAY) : array(WEEKDAY_FRIDAY, WEEKDAY_SATURDAY);
  $weekend_count = weekday_count($in, $out, $weekendAr);
  $midweek_count = $nd - $weekend_count;       

  foreach($aptObjs_or_ids as $aptObjs_or_id) {
    $aptObj = any2obj($type, $aptObjs_or_id);
    if($aptObj->exception_overlap > 0) {
      $result[] = $aptObj;
      //echo("$aptObj->name has exception overlap, skipping weekend pricing, will be done in loop<br>");
      continue;      
    }
    $weekend_rate_cutoff = $aptObj->weekend_rate_cutoff;  /** not in use */ 
    $weekend_rate_cutoff = 0; /** turn this off for now. always use weekend rates unless weekly rate is set */

    $weekend_pricing = $weekend_count && (!$weekend_rate_cutoff || $nd < $weekend_rate_cutoff) && $aptObj->rate_weekend > 0 && $aptObj->rate_weekend != $aptObj->rate_day; // will weekday affect the price
    $base_rate = $aptObj->rate_day;
    $weekend_rate = $aptObj->rate_weekend;
    if($weekend_pricing) {
      $base_rate_ng = get_rate_num_guests($aptObj->rate_day, $aptObj, $ng);
      $weekend_rate_ng = get_rate_num_guests($aptObj->rate_weekend, $aptObj, $ng);
      
      $total = $midweek_count * $base_rate +  $weekend_count * $weekend_rate;
      $average = round($total / $nd, 6);
      $aptObj->rate_day = $average;
      //echo("$aptObj->name has weekend pricing $nd days: br=$base_rate wr=$weekend_rate $weekend_count x $weekend_rate $midweek_count x $base_rate = $average<br>");
    } else {
      //echo("$aptObj->name has no weekend pricing $nd days: weekend days: $weekend_count cutoff=$weekend_rate_cutoff we-rate=$weekend_rate<br>");
    }      
    $result[] = $aptObj;
  }
  return $result;
}

// applies list of exceptions to each applicable apt_id in list
// returns array of (possibly modified) aptObjs. Removes aptObjs no longer available.
// if single apt_id is passed, returns single aptObj, otherwise array of aptObjs
function apply_exceptions($type, $aptObjs_ids, $exceptions, $searchObj, $options=NULL) {
  $debug = $options['debug'];
  $debug_str = "";
  $unset = array();
  $apts = $options['apts'] ?: array();

  //$debug = 1 && !$_SESSION['live'];
  $options['debug'] = $debug;
  
  if(is_array($searchObj)) $searchObj = array2obj($searchObj);
  
  if(!is_array($aptObjs_ids)) { // single apt
    $single = true;
    $aptObj = any2obj($type, $aptObjs_ids);
    $apts = [$aptObj];
    $apt_ids = [$aptObj->id];
  }
  if(!$apt_ids) return $single ? NULL : array();
  
  $count = count($apt_ids);
  $ex_count = count($exceptions);
  if($debug) $debug_str .= "<br><br>Applying $ex_count exceptions to $count properties<br>";  
  if($debug) $debug_str .= "Search: in:{$searchObj->in} out:{$searchObj->in} ng: {$searchObj->ng}<br>";  
  $output_fields = exception_output_fields($type);
  $book_direct = 0;

  $apt_count = count($apts);
  if($debug) $debug_str .= html_b("There are $apt_count apts in options<br>");
  if(!$apt_count) foreach($apt_ids as $apt_id) $apts[] = get_object($type, $apt_id); // exepensive
  
  if(!$ex_count) return $single ? $apts[0] : $apts;

  $hourly = hourly_rate($type);
  $rate_unit = $hourly ? 'hour' : 'day';
  //if($hourly) 
  $num_units = period_diff($searchObj->in, $searchObj->out, $rate_unit); 
  $searchObj->rate_unit = $rate_unit;
  $searchObj->num_units = $num_units;
  
  if($debug) $debug_str .= "$count apts: ".implode(',', $apt_ids).html_break();
  
  $resultApts = array();
  
  for($i=0;$i < $count; $i++) {
    $aptObj = $apts[$i]; // the input property (before exceptions)
    if($debug) $debug_str .= html_h3(" Before Ex: $aptObj->id, $aptObj->name ($aptObj->min_stay): $aptObj->rate_day + $aptObj->rate_change book_direct=$aptObj->book_direct<br>");
    foreach($exceptions as $exception) {
      if($debug) $debug_str .= "$aptObj->name min=$aptObj->min_stay".html_break();
      if(exception_applies($aptObj, $exception, $searchObj, $options)) {
        if($debug) $debug_str .= "applying exception $exception->id to $aptObj->name ($aptObj->id)<br>";
        $aptObj = exception2apt($exception, $aptObj, $searchObj, $options); // apply exception         
        if($single && $debug) {
          $aptAr = (array) $aptObj;      
          $debug_str .= "apply_exceptions after exception $exception->id =".dump(array_intersect_key($aptAr, array_flip($output_fields)), true);
        }
      } else {
        if($debug) $debug_str .= "exception {$exception->id} does not apply<br>";
      }
      //if($aptObj->host_id == $exception->host_id || $aptObj->id == $exception->apt_id) apply_exception($aptObj, $exception, $searchObj, $debug);
      // if($debug) $debug_str .= "$aptObj->name min is now=$aptObj->min_stay".html_break());
      // $eff = $exception->fit_factor;
      // if($eff > 0) {
      //   $fit_factor = fit_factor($searchObj, $aptObj->id);
      //   $good_fit = $fit_factor >= $eff ? true : false; //$debug_str .= "eff: $aptObj->name = $fit_factor<br>");
      // }
      // todo: fix below w/o affecting other processing
      //if(!$single && !$good_fit && (($aptObj->max_stay && $days > $aptObj->max_stay) || $days < $aptObj->min_stay || $num_guests > $aptObj->max_guests)) $unset[$aptObj->id] = true;  

      if(($aptObj->max_stay && $num_units > $aptObj->max_stay) || ($aptObj->min_stay && $num_units < $aptObj->min_stay) || ($aptObj->max_guests && $num_guests > $aptObj->max_guests)) { // resasons for removal
        if($single) { // don't remove when single - only for internet searches
          $unavailable_reasons = array();
          $aptObj->unavailable = true;
          if(($aptObj->max_stay && $num_units > $aptObj->max_stay)) $unavailable_reasons[]= phrase('maximum_stay', CAPITALIZE).": $aptObj->max_stay ".unit_string($rate_unit, $aptObj->max_stay);
          if($num_units < $aptObj->min_stay) $unavailable_reasons[]= phrase('minimum_stay', CAPITALIZE).": $aptObj->min_stay ".unit_string($rate_unit, $aptObj->min_stay);
          if($num_guests > $aptObj->max_guests) $unavailable_reasons[]= phrase('maximum_guests', CAPITALIZE).": $aptObj->max_guests";
          $aptObj->unavailable_reason = implode(',', $unavailable_reasons);
        } else {
          $unset[$aptObj->id] = true;  // don't remove when single - only for internet searches
        }
      }
      if($exception->last) break; // last rule, no more processing.      
    }
    // $apt->discount = $apt->discount ?: percent2discount($apt->rate_change); // set discount is negative %, add to rate is +%, if absolute - already happened in exception2apt, if %, unset      
    if($debug) $debug_str .= html_h3(" After Ex: $aptObj->id, $aptObj->name ($aptObj->min_stay): $aptObj->rate_day + $aptObj->rate_change<br><br>");
    if(!$unset[$aptObj->id]) $resultApts[] = $aptObj; // add to result unless removed
    $aptObj->exception_applied = 1;
    //$debug_str .= "$aptObj->name bd=$aptObj->book_direct<br>";
    if($aptObj->book_direct) $book_direct++;  
  }
  
  if($debug && $unset) $debug_str .= "removing the following: ".implode(',', array_keys($unset));

  // remove properties with offending min or max stay
  // if($remove = array_keys($unset)) {
  //   //dump($unset);
  // } else {
  //   $result_apts = $apts;
  // }
  // if($debug) $debug_str .= "Now ".count($result_apts)." properties".html_break();
  // unset($apts[$i])

  if($single && $debug) {
    $aptAr = (array) $aptObj;      
    $debug_str .= "apply_exceptions after=".dump(array_intersect_key($aptAr, array_flip($output_fields)), true);
  }
  //$debug_str .= "$book_direct apts can be directly booked";
  if(!$single && $book_direct) { // put directly bookable first
    $sortedResult = array();
    foreach($resultApts as $resultApt) {
      if($resultApt->book_direct) array_unshift($sortedResult, $resultApt); else array_push($sortedResult, $resultApt);      
    }
    return $sortedResult;
  }
  
  
  if($debug) print_log($debug_str, 'exception', LOG_LEVEL_ALWAYS);
  
  return $single ? $aptObj : $resultApts;
}

function exception_output_fields($type) {
  $hourly = hourly_rate($ptype);
  $rate_fields = $hourly ? array('rate_hour') : array('rate_day', 'rate_weekend', 'rate_week', 'rate_2_weeks', 'rate_3_weeks', 'rate_month');
  return array_merge($rate_fields, array('book_direct', 'min_stay', 'max_stay', 'max_guests', 'rate_extraperson', 'rate_change'));
}

/** unsused - automatic comment added on  */
function exception_input_fields($type) {
  $hourly = hourly_rate($ptype);
  $length_field = $hourly ? 'num_hours' : 'num_days';
  //$num_months = $hourly ? '' : 'num_months';
  return array('host_id', 'city_id', 'apt_id', 'start', 'end', 'num_guests', $length_field, 'days_from_now', 'days_from_now_gt', 'fit_between', 'fit_next_to');
}

// checks if an exception applies
/** unsused - automatic comment added on  */
function exception_applies($aptObj, $exception, $searchObj, $options = NULL) {
  $debug = $options['debug'];
  $exception->apt_id = intval($exception->apt_id);                  
    
  if(intval($exception->host_id) > 0 && $aptObj->host_id != $exception->host_id) {if($debug) print_log("wrong host: $aptObj->host_id != $exception->host_id", 'exception', LOG_LEVEL_ALWAYS);return false;} // wrong host
  if(intval($exception->host_id) > 0 && $aptObj->host_id != $exception->host_id) {if($debug) print_log("wrong host: $aptObj->host_id != $exception->host_id", 'exception', LOG_LEVEL_ALWAYS);return false;} // wrong host
  if(intval($exception->city_id) > 0 && $aptObj->city_id != $exception->city_id) {if($debug) print_log("wrong city: $aptObj->city_id != $exception->city_id", 'exception', LOG_LEVEL_ALWAYS);return false;} // wrong city
  if(intval($exception->apt_id) > 0 && $aptObj->id != $exception->apt_id)    {if($debug) print_log("wrong property: '$aptObj->id' != $exception->apt_id", 'exception', LOG_LEVEL_ALWAYS);return false;} // wrong apt  
  if(is_date($exception->start) && $searchObj->out <= $exception->start)     {if($debug) print_log("checkout before start: $searchObj->out <= $exception->start", 'exception', LOG_LEVEL_ALWAYS);return false;} // checking out on or before start of exception
  if(is_date($exception->end)   && $searchObj->in >= $exception->end)        {if($debug) print_log("checkin after end: $searchObj->in >= $exception->end", 'exception', LOG_LEVEL_ALWAYS);return false;} // checking in on or after end of exception
  if(intval($exception->num_days) > 0 && $searchObj->nd &&   $searchObj->nd < $exception->num_days)   {if($debug) print_log("too short (days): $searchObj->nd < $exception->num_days", 'exception', LOG_LEVEL_ALWAYS);return false;} // too short stay 
  if(intval($exception->num_hours) > 0 && $searchObj->nh &&  $searchObj->nh < $exception->num_hours)  {if($debug) print_log("too short (hours): $searchObj->nh < $exception->num_hours", 'exception', LOG_LEVEL_ALWAYS);return false;} // too short stay 
  //if($exception->num_months && $searchObj->nm && $searchObj->nm < $exception->num_months) {if($debug) print_log("too short (months): $searchObj->nm < $exception->num_months", 'exception', LOG_LEVEL_ALWAYS);return false;} // too short stay 
  if($exception->num_guests && $searchObj->ng && $searchObj->ng < $exception->num_guests) {if($debug) print_log("too few guests: $searchObj->ng < $exception->num_guests", 'exception', LOG_LEVEL_ALWAYS);return false;} // too few guests 

  $from_date = $searchObj->booking_date ? $searchObj->booking_date : today(); // preserve last minute discount if updated later
  $days_from_now = period_diff($from_date, $searchObj->in, 'd');
  if($exception->days_from_now && $days_from_now > $exception->days_from_now) {if($debug) print_log("too distant: $days_from_now > $exception->days_from_now", 'exception', LOG_LEVEL_ALWAYS);return false;} // too distant 
  if($exception->days_from_now_gt && $days_from_now < $exception->days_from_now_gt) {if($debug) print_log("too close: $days_from_now < $exception->days_from_now_gt", 'exception', LOG_LEVEL_ALWAYS);return false;} // too close 
  if($exception->fit_between || $exception->fit_next_to) {
    $ffObj = fit_factor_object($searchObj, $aptObj->id);
    if($exception->fit_between && $exception->fit_between > $ffObj->fit_between) {if($debug) print_log("not good enough fit between: $exception->fit_between > $ffObj->fit_between", 'exception', LOG_LEVEL_ALWAYS);return false;} // not good enough fit   
    if($exception->fit_next_to && $exception->fit_next_to > $ffObj->fit_next_to) {if($debug) print_log("not good enough fit next_to: $exception->fit_next_to > $ffObj->fit_next_to", 'exception', LOG_LEVEL_ALWAYS);return false;} // not good enough fit   
  }      

  return true;
}


// copies output vars from exception to aptObj
// do not use this for date intersecting date-exceptions: test outside the function
function exception2apt($exception, $aptObj, $searchObj, $options=NULL) {  
  $debug = $options['debug'];
  $debug_str = '';
  
  //$debug = true;
  $id = $exception->id;
  
  /** if we set a daily rate, but not weekend rate, use day rate */
  //if($exception->rate_day > 0 && !$exception->rate_weekend) $exception->rate_weekend = $exception->rate_day; 
  
  /** for rate_week and rate_month exceptions (if they apply), we can process here and return
      because no other fields are allowed 
      We will assume here that it does apply, since exceptions_applies() must be called first
      */
  if($exception->rate_week || $exception->rate_2_weeks || $exception->rate_3_weeks) {
    $aptObj->rate_week = $exception->rate_week;
    $aptObj->rate_2_weeks = $exception->rate_2_weeks;
    $aptObj->rate_3_weeks = $exception->rate_3_weeks;
    return $aptObj;
  }
  
  if($exception->rate_month) {
    $aptObj->rate_month = $exception->rate_month;
    return $aptObj;
  }
      
  //if(1) print_log("applying exception $exception->id ($exception->description) to $aptObj->name<br>");
  $days_overlap_fraction = 1;
  $hourly = hourly_rate($aptObj->type);
  if(!$hourly && $exception->start && $exception->end) { //todo: fix for hourly
    $days_overlap = dates_intersect($searchObj->in, $searchObj->out, $exception->start, $exception->end);
    $num_days = period_diff($searchObj->in, $searchObj->out, 'd');
    $days_overlap_fraction = dates_intersect_fraction($searchObj->in, $searchObj->out, $exception->start, $exception->end);
    if($debug) print_log("In=$searchObj->in Out=$searchObj->out Start: $exception->start End: $exception->end Overlap: $days_overlap Days=$num_days Fraction: $days_overlap_fraction<br>", 'exception', LOG_LEVEL_ALWAYS);
  }
  
  $outputVars = exception_output_fields($aptObj->type); // array('book_direct', 'min_stay', 'max_stay', 'max_guests', 'rate_day/rate_hour', 'rate_extraperson', 'discount', 'surcharge')
  $orgAptObj = $aptObj;
  $changes = array();
  $rate_field = hourly_rate($aptObj->type) ? 'rate_hour' : 'rate_day'; 
  if($rate_change = $aptObj->rate_change) {
    list($abs_value, $percentage) = split_percentage($rate_change);
    if($abs_value) {
      $aptObj->$rate_field += $abs_value; // for absolute value, change the rate field
      $aptObj->rate_change = ''; // remove, as it can't be added below.
    }
  }

  //if($debug) die(dump($exception,true));
  
  $vars = dump($outputVars, true);
  if($debug) print_log("<span class='green'>Processing Vars=$vars</span><br>", 'exception', LOG_LEVEL_ALWAYS);
  foreach($outputVars as $var) {
    if($debug) print_log("<span class='green'>Processing Var=$var</span><br>", 'exception', LOG_LEVEL_ALWAYS);
    $val = $exception->$var;
    if(strlen($val)) {      
      if($debug) print_log("$id: var=$var val=$val overlap=$days_overlap_fraction<br>", 'exception', LOG_LEVEL_ALWAYS);
      if($days_overlap_fraction < 1 && in_array($var, array('rate_day', 'rate_hour', 'rate_extraperson', 'rate_weekend', 'rate_change'))) {        
        $aptObj->exception_overlap = $days_overlap_fraction; // this tells us to call get_average_rate for this property
        if($debug) print_log("<span class='red'>Var=$var: Returning because days_overlap_fraction=$days_overlap_fraction</span><br>", 'exception', LOG_LEVEL_ALWAYS);
        continue; // don't apply if dates intersect, let price breakdown handle it
      }
      if($var == 'rate_change') {
        list($abs_value, $percentage) = split_percentage($val);
        if($debug) print_log("rate change: abs=$abs_value perc=$percentage<br>", 'exception', LOG_LEVEL_ALWAYS);

        if($abs_value) {                    
          $aptObj->$rate_field += $abs_value; // for absolute value, change the rate field
          if($aptObj->rate_weekend > 0) $aptObj->rate_weekend += $abs_value; // for absolute value, change the rate field
          if($debug) print_log("changing $rate_field: adding $abs_value".html_break(), 'exception', LOG_LEVEL_ALWAYS);
        } elseif($percentage > 0 && !$options['price_breakdown']) { // for price breakdown, keep the % increase for visibility
          $aptObj->$rate_field = apply_surcharge($aptObj->$rate_field, $val); // for positive percentage, add to the rate field
          if($aptObj->rate_weekend > 0) $aptObj->rate_weekend = apply_surcharge($aptObj->rate_weekend, $val); // for positive percentage, add to the rate field
          if($aptObj->rate_extraperson > 0) $aptObj->rate_extraperson = apply_surcharge($aptObj->rate_extraperson, $val); // for positive percentage, add to the rate field
          if($debug) print_log("changing $rate_field: adding $percentage %".html_break(), 'exception', LOG_LEVEL_ALWAYS);
        } else { // discount
          $val = add_percentages($aptObj->rate_change, $val); // won't work if $aptObj->rate_change is absolute
          if($debug) print_log("changing $var from ".$aptObj->$var. " to (add_perc ".$exception->$var.") ".$val.html_break(), 'exception', LOG_LEVEL_ALWAYS);
          $aptObj->$var = $val;
        }
      } else {
        if($debug) print_log("changing $var from ".$aptObj->$var. " to ".$val.html_break(), 'exception', LOG_LEVEL_ALWAYS);
        $aptObj->$var = $val;          
      }
      $changes[] = $var;
    }
  }

  $in = $searchObj->in;
  $co = $exception->changeover; 
  $start = $exception->start;
  $end = $exception->end;
  if($co && $in && $start && $end && $in >= $start && $in < $end) $aptObj->changeover = $co;
  
  return $aptObj;
}


/** unsused - automatic comment added on  */
function direct_bookable($apt, $searchObj=NULL) {
  // echo("$apt->name applied=$apt->exception_applied book direct=".$apt->book_direct."<br>");
  if($apt->type == 'hotel') return true; // hotels are always directly bookable
  elseif(in_array($apt->type, array('shortstay', 'boat', 'shortstay_office'))) { // shortstay/boat
    if($apt->book_direct) return true; // always directly bookable

    if($searchObj) { // onrequest, but maybe last minute / just fit / long booking
      $searchArray = array();
    }

    return false; // no exceptions
  }
  return false; // real-estate / long-stay, other types, not bookable
}

// adds rate_extraperson to rate_day for each num_guest > rate_num_guests
// if rate_day is zero, just returns the surcharge
function get_rate_num_guests($rate_base, $aptObj, $num_guests) {
  //if(!$rate_base) return 0;
  $rate_num_guests  = $aptObj->rate_num_guests;
  $rate_extraperson = $aptObj->rate_extraperson;
  $extra_guests = $aptObj->rate_num_guests > 0 ? $num_guests - $rate_num_guests : 0;
  if($extra_guests > 0) $rate_base += $extra_guests * $rate_extraperson; // add rate for extra guests
  //if($aptObj->type == 'shortstay_office') $rate_base = round($rate_base) - 0.50 ; // round to nearest whole and subtract 50 cents
  return $rate_base;
}

// update status based on payments
function update_reservation_status($resObj_or_id) {
  $resObj = any2obj('reservation', $resObj_or_id);
  $res_id = $resObj->res_id;
  
  $status = $resObj->status;
  
  $grand_total = $resObj->grand_total;
  $paid = $resObj->paid;
  $due = $resObj->deposit > 0 ? $resObj->deposit : $grand_total;
  $balance = $resObj->balance;
  $authorized = $resObj->authorized;
  
  $new_status = 0;
  
  if($status > 0 && $status <= STATUS_RESERVED && $grand_total > 0) {

    if($balance < INV_TOLERANCE) $new_status = STATUS_COMPLETE;
    elseif($due - $paid < INV_TOLERANCE) $new_status = STATUS_RESERVED;
    elseif($due - $authorized < INV_TOLERANCE) $new_status = STATUS_PENDING;    

  } elseif($status == STATUS_COMPLETE && $balance > INV_TOLERANCE) { // demote to reserved
    $new_status = STATUS_RESERVED;
  }
  
  print_log("update_reservation_status: id=$res_id gt=$grand_total due=$due paid=$paid auth=$authorized balance=$balance status=$status new=$new_status", 'payment', LOG_LEVEL_TEST);
    
  if($new_status && $new_status != $status) {
    $data = array("status" => $new_status);
    update_object("reservation", $data, $res_id);
  }
}



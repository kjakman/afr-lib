<?php

/** unsused - automatic comment added on  */
function public_link($obj, $obj_type) {
  switch($obj_type) {
    case 'reservation':
    case 'booking':
      return booking_link($obj, true);
    case 'invoice':
      return invoice_link($obj, true);
    case 'payment':
      return payment_link($obj, true);
    default:
      return '';
    }
}

/** from db_reservation, print_property, db_apartment, helpers_request - those still in use */
function invoice_link($invObj_or_id, $public = false) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $link = site_page_link($invObj->site_id, PAGE_INVOICE, ['full' => 1]).$invObj->id;
  if($public && $invObj->booking_pin) $link .= "?pin=".myhash($invObj->booking_pin);
  return $link;
}

function payment_link($invObj_or_id, $public = false) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $link = site_page_link($invObj->site_id, PAGE_PAYMENT, ['full' => 1]).$invObj->id;
  if($public && $invObj->booking_pin) $link .= "?pin=".myhash($invObj->booking_pin);
  return $link;
}

function booking_link($resObj_or_id, $public = false) {
  $resObj = any2obj('reservation', $resObj_or_id);
  $link = site_page_link($resObj->site_id, PAGE_STATUS, ['full' => 1]).$resObj->res_id;
  if($public && $resObj->booking_pin) $link .= "?pin=".myhash($resObj->booking_pin);
  return $link;
}

// returns associate array of arrival information (view)
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function arrival_details($resObj_or_id, $user_level=0) {
  $resObj = any2obj('reservation', $resObj_or_id); 
  $fields=array();  
  //if($resObj->property_type == 'shortstay') list($airport_pickup_ok, $airport_pickup_fee) = pickup_ok_fee($resObj->apt_id);  
  $arrival_str = implode(' - ', array_filter(array(phrase($resObj->arrival_method, CAPITALIZE), $resObj->arrival_number, strip_sec($resObj->arrival_time), $resObj->arrival_notes)));
  if($arrival_str) $fields[phrase('arrival_information', CAPITALIZE)] = $arrival_str;
  $fields[phrase('checkin_time', CAPITALIZE)] = $resObj->arrival_property ? strip_sec($resObj->arrival_property) : "???";
  
  // if($airport_pickup_ok) $fields[phrase('airport_pickup', CAPITALIZE)]= $resObj->airport_pickup ? phrase('yes', CAPITALIZE) : phrase('no', CAPITALIZE);
  return $fields;
}

/**
function booking_link($resObj_or_id, $options=array()) {
  global $__SITE;
  $resObj = any2object('reservation', $resObj_or_id);
  return site_page_link($site_id, PAGE_STATUS).$resObj->res_id; 
}

function payment_link($paymentObj_or_id, $options=array()) {
  global $__SITE;
  $paymentObj = any2object('payment', $paymentObj_or_id);
  $site_id = $paymentObj->site_id ?: $__SITE->id;
  return site_page_link($site_id, PAGE_PAYMENT).$paymentObj->id; 
}

function invoice_link($invoiceObj_or_id, $options=array()) {
  global $__SITE;
  $invoiceObj = any2object('invoice', $invoiceObj_or_id);
  $site_id = $invoiceObj->site_id ?: $__SITE->id;
  return site_page_link($site_id, PAGE_INVOICE).$invoiceObj->id; 
}
*/

/** is this still in use ? */
function apt2res($data, $aptObj, $new=true) { // copies data from apartment to reservation
  if($aptObj) {
    $data['apt_id'] = $aptObj->id;
    $data['city_id'] = $city_id = $aptObj->city_id;
    $data['currency'] = $aptObj->currency;
    $data['host_id'] = $aptObj->host_id;
    $data['apartment_name'] = $aptObj->parent_id ? get_object('property', $aptObj->parent_id, 'name') : $aptObj->name;
    $data['corporate'] = $aptObj->corporate;
    $data['property_type'] = $aptObj->type;

    $data['city_name'] = $city_id ? get_object('geo_city', $city_id, 'name') : $aptObj->address_city;
    
    if($new) { // only for new reservations
      if($data['deposit_percentage'] == '') $data['deposit_percentage'] = $aptObj->deposit;
      
      /** for unchecked checkboxes, set to 0 */
      $checkbox_fields = ['charge_tax', 'tax_included', 'charge_city_tax'];
      foreach($checkbox_fields as $fld) {
        if(!isset($data[$fld])) $data[$fld] = $aptObj->$fld; 
      }
    }
  } else {
    $data['apartment_name'] = "?";
  }
  //$errors = $data;
  return $data;
}

function booking_fee_total($fee_json) {
  if(!$fee_json || !isJson($fee_json)) return 0;
  $feeArray = json_decode($fee_json);
  $fees = 0;
  foreach($feeArray as $feeObj) $fees += $feeObj->total;
  return $fees;
}

function booking_fees($resObj) {
  return $resObj->fee_json ? json_decode($resObj->fee_json) : [];
}

function booking_update_rate($resObj_or_id) {
  $resObj = any2obj('reservation', $resObj_or_id);
  if(!$resObj || $resObj->rate_locked) return false;
  if(!$resObj->property_type || !$resObj->apt_id) return false;
  
  $aptObj = get_object($resObj->property_type, $resObj->apt_id);
  if(!$aptObj) return false;
  
  $priceObj = get_average_rate($aptObj, $resObj->checkin, $resObj->checkin, $resObj->num_guests);
  print_log("booking_update_rate: priceObj:".dump($priceObj,true), 'booking', LOG_LEVEL_TEST);
  
  /** don't trigger handlers - this is called from class_handlers */
  $resData = [];
  $flds = ['rate_type', 'rate_base', 'discount'];
  foreach($flds as $fld) {
    if(isset($priceObj->$fld) && $priceObj->$fld != $resObj->$fld) $resData[$fld] = $priceObj->$fld;
  }
  print_log("booking_update_rate: data:".dump($resData,true), 'booking', LOG_LEVEL_TEST);
  if($resData) {
    $db_terms = array_tuplets($resData);
    $qs = "UPDATE reservation SET $db_terms WHERE res_id=$id";
    db_query($qs);
  }
    
  return $resData;  
}

/** (discounted) total for stay, no fees */
function booking_subtotal($resObj_or_id) {
  $resObj = any2obj('reservation', $resObj_or_id);
  $rate_base = $resObj->rate_base;
  $discount_base = apply_discount($rate_base, $resObj->discount, 4); // four decimal places to reduce rounding errors
  switch( $resObj->rate_type) {
    case 'day':
      return $resObj->num_days*$discount_base;
    case 'month':
      $resObj->num_months*$discount_base;
    case 'total':
    case 'special':
      return $discount_base;
    default:
      return 0;
  }
}


function booking_city_tax($resObj) {
  $res_id = $resObj->res_id;
 
  $total = $resObj->total;
  $charge_tax = $resObj->charge_tax;
  $tax_included = $resObj->tax_included;
  $charge_city_tax = $resObj->charge_city_tax;
  if(!$total) return 0;
  
  $vat_rate = 1.06; // todo: not hard coded
  $total_ex = $charge_tax && $tax_included ? $total / $vat_rate : $total;

  $city_tax = 0;
  if($charge_city_tax = $resObj->charge_city_tax) {
    $city_tax_rate = get_city_tax_rate($resObj->city_id) ?: ''; /** store with res in case turned on later */
    $city_tax = $city_tax_rate ? apply_surcharge($total_ex, $city_tax_rate, true) : 0; 
  }
  print_log("booking_city_tax id=$res_id total=$total total_ex=$total_ex charge_tax=$charge_tax tax_included=$tax_included city_id={$resObj->city_id} charge_city_tax=$charge_city_tax city_tax_rate=$city_tax_rate city_tax=$city_tax", 'booking', LOG_LEVEL_TEST);  
  return $city_tax;
}


// new total rate based on num_days/hours + base_rate + dicount
function booking_update_total($resObj_or_id) {
  $resObj = any2obj('reservation', $resObj_or_id);
	
	$id = $res_id = $resObj->res_id;
	$site_id = $resObj->site_id;

  $status = $resObj->status;
  
  $charge_tax = $resObj->charge_tax;
  $tax_included = $resObj->tax_included;
  $charge_city_tax = $resObj->charge_city_tax;
	$charge_service_fee = $site_id && $resObj->type != RES_TYPE_ADMIN ? get_object('site', $site_id, 'charge_service_fee') : 0;

  $total = $resObj->total = booking_subtotal($resObj); /** already discounted */
  $paid  = get_reservation_paid($resObj);
  $authorized = get_reservation_authorized($resObj);
  $fees = booking_fee_total($resObj->fee_json);

	$service_fee = $charge_service_fee ? service_fee($total) : 0;
	$city_tax = $resObj->charge_city_tax ? booking_city_tax($resObj) : 0;
  $deposit = $total * $resObj->deposit_percentage / 100;

  $grand_total = $total + $service_fee + $fees + $city_tax + $resObj->other_charges;
  $balance = $grand_total - $paid;
  
  $resData = [];
  $resData['paid']        = $paid;
  $resData['authorized']  = $authorized;
  $resData['total']       = $total;
  $resData['deposit']     = $deposit;
  $resData['fees']        = $fees;
  $resData['fee_service'] = $service_fee;
  $resData['city_tax']    = $city_tax;  
  $resData['balance']     = $balance;
  
  if($charge_tax) {
    $tax_of_which = $tax = 0;
    /** get tax for stay */
    $cat = 'L'; // todo, not hardcode for NL
    if($tax_included) $tax_of_which += get_tax_included($total, get_tax_rate($cat));
    else $tax += get_tax_added($total, get_tax_rate($cat));
    
    print_log("booking_update_total: rent tax: tax_included=$tax_included tax=$tax tow=$tax_of_which", 'booking', LOG_LEVEL_TEST);
    
    /** get tax for service fee */
    
    /** get tax for fees */
    $fee_json = $resObj->fee_json ?: [];
    $fees = json_decode($fee_json);
    if(is_array($fees) && count($fees)) {
      foreach($fees as $feeObj) {
        $cat = $feeObj->tax;
        $fee_total = $feeObj->total;
        
        if($tax_included) $tax_of_which += get_tax_included($fee_total, get_tax_rate($cat));
        else $tax += get_tax_added($fee_total, get_tax_rate($cat));
      }
    }
    
    if($tax_included) {
      $resData['tax_of_which'] = $tax_of_which;
      $resData['tax'] = 0;
    } else {
      $resData['tax'] = $tax;      
      $resData['tax_of_which'] = 0;
      if($tax > 0 || $tax < 0) $grand_total += $tax;
      print_log("booking_update_total: Adding $tax  to gt: {$grand_total}", 'booking', LOG_LEVEL_TEST);      
    }
  }
  
  if($resObj->status == STATUS_COMPLETE && $balance > INV_TOLERANCE) $resData['status'] = STATUS_RESERVED; // demote if there's a balance
  
  $resData['grand_total'] = $grand_total;
  
  /** don't trigger handlers - this is called from class_handlers */
  $db_terms = array_tuplets($resData);
  $qs = "UPDATE reservation SET $db_terms WHERE res_id=$id";
  db_query($qs);

  print_log("booking_update_total: Total Tax: tax_included=$tax_included tax=$tax tow=$tax_of_which", 'booking', LOG_LEVEL_TEST);
  //print_log("booking_update_total: id=$res_id total=$total charge_tax=$charge_tax tax_included=$tax_included charge_city_tax=$charge_city_tax city_tax=$city_tax charge_service_fee=$charge_service_fee service_fee=$service_fee paid=$paid auth=$authorized grand_total=$grand_total balance=$balance", 'booking', LOG_LEVEL_TEST);
    
  return $resData;
}


/** new */
// returns exceptions for a host, optionally limited to apt_ids (array or single value)
// if field is set, this must be not-null
function host_exceptions($start, $end, $host_id, $apt_ids, $field = '') {
  $exSearch = array();
  if($host_id) { // if only one host is shown, show for this apt or all apts
    $exSearch[DB_LOGICAL_OR] = array('apt_id' => $apt_ids, DB_LOGICAL_AND => array('host_id' => $host_id, 'apt_id' => ATT_DEFAULT_NULL)); 
  } else {
    $exSearch['apt_id'] = $apt_ids; // don't show host exceptions if multiple host_ids in calendar (then host_id = 0)
  }
    
  $exSearch['active'] = 1;
  if($field) $exSearch[$field] = array("!=", ATT_DEFAULT_NULL);
  return find_objects('res_exception', $exSearch, array('debug_query' => 0));
  
}

/** unsused - automatic comment added on  */
function reservation_guest_name($resObj_or_id, $options = array()) {
  $resObj = any2obj('reservation', $resObj_or_id);
  if($guest_id = $resObj->guest_id) return user_display_name($guest_id);
  return pick_first($resObj->first_name, $resObj->last_name, phrase('anonymous', CAPITALIZE));
}



/** cron related functions */

/** base function for mail reminders */
function booking_reminder($type, $search, $field) {
  $result = new stdClass;
  $errors = $messages = [];
  $resObjs = find_objects('reservation', $search);
  $count = count($resObjs);
  $i = $sent_count = 0;
  foreach($resObjs as $resObj) {
    switch($type) {
      case 'reminder':
        $result = booking_reminder_mail($resObj);
        break;
      case 'review':
        $result = booking_review_mail($resObj);
        break;
      default: 
        return json_error_object("Booking reminder $type not implemented");
    }
    if($result->success) {
      $sent = 1;
      $sent_count++;
    } else {
      $errors = array_merge_smart($errors, $result->error ?: "Failed to send {$type} reminder for booking {$resObj->res_id}");
      $sent = -1;
    }
    list($id, $errors) = update_object('reservation', [$field => $sent], $resObj->res_id);
    $i++;
  }
  
  if($i) {
    $now = now();
    $message = "{$sent_count}/$i {$type} reminders sent";  
    print_log("{$now}: {$message}", 'booking-cron', LOG_LEVEL_ALWAYS);
    $messages[] = $message;
  }
  
  $result->success = 1;
  $result->sent = $sent_count;
  $result->messages = $messages;
  return $result;
}

/** used by cronscript to send reminder to host/guest */
function booking_checkin_reminder() {
  $today = today();
  $in = add_day($today, 1);
  $out = add_day($today, BOOKING_REMINDER_PERIOD);  
  $search = ['status' => [STATUS_RESERVED, STATUS_COMPLETE], 'checkout' => ['><', $in, $out], 'reminder_sent' => 0, 'send_email' => 1, 'type' => ['!=', RES_TYPE_ADMIN]]; 
  return booking_reminder('checkin', $search, 'reminder_sent');
}

/** used by cronscript to send request for review to host/guest */
function booking_review_reminder() {
  $today = today();
  $in = add_day($today, -1 * BOOKING_REVIEW_PERIOD);
  $out = add_day($today, -1);  
  $search = ['status' => [STATUS_RESERVED, STATUS_COMPLETE], 'checkout' => ['><', $in, $out], 'review_sent' => 0, 'send_email' => 1, 'type' => ['!=', RES_TYPE_ADMIN]];
  return booking_reminder('review', $search, 'review_sent');
}

/** used by cronscript to expire bookings */
/** unsused - automatic comment added on  */
function booking_expire() {
  
  $result = new stdClass;
  $now = now();
  $errors = $messages = [];
  $search = ['type' => ['!=', RES_TYPE_ADMIN], 'status' => [STATUS_INQUIRY, STATUS_REQUEST, STATUS_PROPOSED, STATUS_PENDING], 'expires' => ['><', 0, $now]];
  $updated = $count = 0;
  if($resObjs = find_objects('reservation', $search)) {
    $count = count($resObjs);
    foreach($resObjs as $resObj) {    
      list($id, $uerrors) = update_object('reservation', array('status' => STATUS_EXPIRED), $resObj->res_id);
      $messages[] = "{$resObj->res_id} created  {$resObj->created} expired: {$resObj->site_id} t={$resObj->type} st={$resObj->status} {$resObj->first_name} {$resObj->last_name} @ {$resObj->apartment_name} {$resObj->checkin} {$resObj->checkout}";
      if($id) $updated++;
      else if($uerrors) $errors = array_merge_smart($errors, $uerrors ?: "Failed to expire booking {$resObj->res_id}");
    }
  }
  if($count) {
    $message = "{$count} bookings expired, updated {$updated}";  
    print_log("{$now}: {$message}", 'booking-cron', LOG_LEVEL_ALWAYS);
    $messages[] = $message;
  }
    
  $result->success = 1;
  $result->messages = $messages;
  return $result;
}

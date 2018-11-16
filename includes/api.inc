<?php

function api_calendar($obj_data) {
  global $__USER;
  if(!$__USER) return json_error_object("Please log in");
  if($__USER->user_level < USER_LEVEL_HOST) return json_error_object("Not authorized");
  if($error = check_required($obj_data, array('host_id' => "Host ID", 'start' => "Start", 'end' => "End"))) return json_error_object("Missing: ".implode(', ', $error));
        
  print_log("api_calendar data:".dump($data,true), 'calendar', LOG_LEVEL_TEST);
  
  $pretty = isset($obj_data['pretty']) ? JSON_PRETTY_PRINT : 0;
  $apt_id = list2array($obj_data['apt_id']);
  $host_id = list2array($obj_data['host_id']);

  $start = $obj_data['start'];
  $end = $obj_data['end'];      
  $edit = $obj_data['edit'] ? true : false; // set edit=1 to allow editing
  
  $debug = isset($_GET['debug']) && $_GET['debug']; 
  $auth_host_ids = user_host_list($__USER->id);  
  
  if($diff = array_diff($host_id, $auth_host_ids)) return json_error_object("Host not authorized:".dump($diff,true));
  
  $search = [];
  $search['host_id'] = $host_id; /** mandatory */
  if($apt_id) $search['apt_id'] = $apt_id;

  
  $exObjs = $resObjs = $invObjs = $eventObjs = [];
  //$exObjs  = date_exceptions($start, $end, $search, $options);

  $options = [];
  $status = $obj_data['status'] ?: STATUS_PROPOSED;
  $search['status'] = ['>=', $status];

  $resObjs = calendar_entries('reservation', $start, $end, $search, $options);
  $invObjs = calendar_entries('invoice', $start, $end, $search, $options);
  $exObjs = calendar_entries('res_exception', $start, $end, $search, $options);
  
  $events = array_merge($resObjs,$exObjs,$invObjs);
  $result = new stdClass;
  $result->count = count($events);
  $result->event_count = count($eventObjs);
  $result->reservation_count = count($resObjs);
  $result->invoice_count = count($invObjs);
  $result->exception_count = count($exObjs);

  /** new expensive stats functions */
  unset($search['status']);
  $result->booked = $result->invoiced = $result->paid = [];
  
  if(1) { /** adds significant time */
    $result->booked = calendar_booked($start, $end,$search, $options);
    $result->invoiced = calendar_invoiced($start, $end,$search, $options);
    $result->paid = calendar_paid($start, $end,$search, $options);
  }
  $result->success = 1;
  $result->events = $events;
  return $result;
}

function api_rentals($obj_data) {
  global $__USER;
  list($valid, $errors) = check_api_credentials($obj_data);
  if(!$valid) return json_error_object($errors ?: "Invalid credentials");

  if($error = check_required($obj_data, array('host_id' => "Host ID"))) return json_error_object("Missing: ".implode(', ', $error));
          
  $apt_id = list2array($obj_data['apt_id']);
  $host_id = list2array($obj_data['host_id']);  

  $start = $obj_data['start'];
  $end = $obj_data['end'];      
      
  $search = [];
  $search['host_id'] = $host_id; /** mandatory */
  $search['active'] = 1; /** mandatory */
  if($city_id) $search['city_id'] = $city_id; /** mandatory */
  if($apt_id) $search['apt_id'] = $apt_id;
  if(isset($obj_data['licensed'])) $search['licensed'] = $obj_data['licensed'];

  $fields = ['id','type','city_id','source_id','licensed','name','location_lat','location_long',
             'currency', 'rate_day', 'min_guests', 'max_guests', 'min_stay', 'max_stay', 
             'address_street','address_number','address_suffix', 'address_postcode', 'address_city','address_country'
             ];
  $rentals = find_objects('shortstay', $search, ['fields' => $fields]);
  $result = new stdClass;
  $result->success = 1;
  $result->rentals = $rentals;
  return $result;
}

function api_bookings($obj_data) {
  $result = api_rentals($obj_data);
  if(!$result->success) return $result;
  $rentals = $result->rentals;
  if(!$rentals) return json_error_object("No rentals found");
  $cal = [];
  $today = today();
  foreach($rentals as $aptObj) {
    $result = rental_update_calendar($aptObj);
    if($result->error) return $result;
    $data = $result->data;
    if($data['calendar_startdate'] != $today) return json_error_object("Startdate mismatch");    
    $cal[$aptObj->id] = $data['calendar_availability'];
  }
  $result = new stdClass;
  $result->success = 1;
  $result->start_date = $today;
  $result->calendar = $cal;
  return $result;
}

function api_booking($obj_data) {
  if(1 && $_SESSION['dev']) {
    rental_update_calendar($obj_data['apt_id']);
    db_query("delete from reservation where res_id >= 47986");
  }
  
  $result = api_availability($obj_data);
  if(!$result->available || !$result->success) {
    $result->success = 0;
    return $result;
  }
  
  $rate = $result->rate;
  $rental_currency = $rate->currency;
  
  $required = ['first_name' => "First Name", 'last_name' => "Last Name", 
               'currency' => "Currency", 'rate' => "Rate", 'paid' => "Paid", 'commission' => "Commission"];
  if($error = check_required($obj_data, $required)) return json_error_object("Missing: ".implode(', ', $error));

  $resData = $obj_data;
  $resData['type'] = RES_TYPE_DIRECT;
  $resData['status'] = STATUS_RESERVED;
  //$resData['extra_info'] = $extra_info;
  $resData['send_email'] = 0;

  $resData['num_guests'] = $obj_data['ng'];
  $resData['checkin'] = $obj_data['in'];
  $resData['checkout'] = $obj_data['out'];
  $resData['site_id'] = 10;
  $resData['rate_base'] = $rate_day = $obj_data['rate'];  
  
  $source_id = $resData['source_id'];
  $paid = $obj_data['paid'];
  $nd = $rate->num_days;
  $total = $rate_day * $nd;
  $deposit = 0.1 * $total;
  
  $booking_currency = $obj_data['currency'];
  
  if($booking_currency != $rental_currency) return json_error_object("Expected currency {$rental_currency}");
  if(!$paid || $paid < ($deposit - 1)) return json_error_object("Paid must be at least {$rental_currency} {$deposit} (10% of {$total})"); 
  if($paid > ($total + 1)) return json_error_object("Paid ({$paid}) must be less than the total ({$rental_currency} {$total})"); 
  
  unset($resData['source_id']); /** or invoice won't be created */
  list($res_id, $errors) = add_object('reservation', $resData);
  if($errors) return json_error_object($errors);
  if($res_id && $source_id) update_object('reservation', ['source_id' => $source_id], $res_id);  
  
  $invObj = find_booking_invoice($res_id);
  if(!$invObj) return json_error_object("Booking invoice for booking {$res_id} not found");
  $inv_id = $invObj->id;
  
  $payData = ['inv_id' => $inv_id, 'currency' => $rental_currency, 'amount' => $paid, 
              'payment_type' => PAYMENT_TYPE_AGENT, 'payment_status' => PAYMENT_STATUS_COMPLETED, 'paid_date' => $today];

  print_log("adding payment for inv $inv_id", 'api', LOG_LEVEL_TEST);

  list($payment_id, $pay_errors) = add_object('payment', $payData);

  print_log("added payment {$payment_id} for inv $inv_id errors:".dump($pay_errors,true), 'api', LOG_LEVEL_TEST);
  
  if($pay_errors) return json_error_object($pay_errors);
  
  $resObj = get_object('reservation', $res_id);
  
  $result = new stdClass;
  $result->success = 1;
  $result->booking_id = $res_id;
  $result->booking_link = booking_link($resObj, true);
  $result->invoice_id = $inv_id;
  $result->booking = $resObj;
  dump($result);die();
  return $result;
}
  
function api_availability($obj_data) {
  global $__USER;
  list($valid, $errors) = check_api_credentials($obj_data);
  if(!$valid) return json_error_object($errors ?: "Invalid credentials");

  if($error = check_required($obj_data, array('apt_id' => "Rental ID", 'in' => "Checkin", 'out' => "Checkout", 'ng' => "# Guests"))) return json_error_object("Missing: ".implode(', ', $error));

  extract($obj_data);
  list($avail, $reason) = rental_availability($apt_id, ['in' => $in, 'out' => $out, 'ng' => $ng]);
    
  $result = new stdClass;
  $result->success = 1;
  $result->available = $avail ? 1 : 0;
  if($avail) $result->rate = get_average_rate($apt_id, $in, $out, $ng);

  if($reason) $result->reason = $reason;
  return $result;
}

function check_api_credentials($obj_data) {
  global $__USER;
  if($source_id = $obj_data['source_id']) {
    $sourceObj = get_object('source', $source_id);
    if(!$sourceObj) return [0, "Source {$source_id} not found"];
    
    $api_key = $obj_data['api_key'];
    if(!$api_key) return [0, "Missing api key"];
    $source_api_key = $sourceObj->api_key;
    if(!$source_api_key) return [0, "Source has no api key"];
    if($source_api_key != $api_key) return [0, "Wrong API key"];
    return [1, ""];
  }
    
  if(!$__USER) return [0, "Please log in"];
  if($__USER->user_level < USER_LEVEL_HOST) return [0, "Not authorized"];
  return [1, ""];  
}
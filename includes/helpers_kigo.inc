<?php

/** unsused - automatic comment added on  */
function kigo_sync_reservations($options = array()) {  
  $limit = $options['limit'] ?: 0;
  $dryrun = $options['dryrun'] ?: false;
    
  $resObjs = find_objects('reservation', array('owner_id' => 1010, 'apt_id' => array('>', 0), 'status' => array('>=', STATUS_RESERVED), 'checkout' => array('>',today())));
  $count = count($resObjs);
  $output = html_h2("Found $count reservations");
  //$resObjs = array();
  $i = 0;
  foreach($resObjs as $resObj) {
    $res_id = $resObj->res_id;
    if($limit && $i++ >= $limit) {
      $output .= "i=$i limit=$limit, exiting<br>";
      break;
    }
    $in = strip_time($resObj->checkin);
    $out = strip_time($resObj->checkout);
    $kigo_res_id = kigo_reservation_id($resObj);
    $apt_id = $resObj->apt_id;
    if(!$apt_id) {
      $output .= "Reservation $res_id has no apt_id ($resObj->notes)<br>";
      continue;
      //dump($resObj);
    }
    $kigo_apt_id = kigo_property_id($apt_id);
    if(!$kigo_apt_id) {
      $res_status = get_object('res_status', $resObj->status, 'name');
      $output .= "Did not find kigo reservation ID or property ID for $resObj->res_id (apt_id=$resObj->apt_id name=$resObj->apartment_name status=$res_status notes=$resObj->notes)<br>";
      continue;
    }
    if($dryrun) {
      $action = $kigo_res_id ? "Updating" : "Adding";
      $output .= "DRYRUN: $action reservation $res_id ($apt_id) <b>Kigo: $fid Apt: $kigo_apt_id</b><br>";
      continue;
    }
    if($kigo_res_id) {
      list($fid, $errors) = kigo_update_reservation($resObj, $kigo_res_id);
      $result = kigo_parse_result($fid, $errors);
      $output .= "Updated reservation $res_id ($resObj->apartment_name: $in-$out) <b>kigo_res_id=$kigo_res_id fid=$fid</b>: $result<br>";
    } else if($kigo_apt_id) {
      list($fid, $errors) = kigo_create_reservation($resObj);
      $result = kigo_parse_result($fid, $errors);
      $output .= "Added reservation $res_id ($resObj->apartment_name: $in-$out) <b>fid=$fid</b>: $result<br>";
    }
  }
  return $output;
}

function kigo_parse_result($fid, $errors=array()) {
  $error_str = '';  
  if($errors) {
    if(count($errors) == 1) {
      $error = $errors[0];
      if(strpos($error, 'conflict')) {            
        $error_str = "Conflict"; 
      } else {
        $error_str = $error;
      }
    } else {
      $error_str = dump($errors, true);
    }
  }
  $result = $error_str ? html_span($error_str, 'red') : html_span('OK', 'green');
  return $result;
}

function kigo_reservation_id($resObj_or_id) {
  $fids = object_foreign_key('reservation', $resObj_or_id);
  if(!$fids) return 0;
  return json_get_value($fids, SOURCE_ID_KIGO) ?: 0;
}

function kigo_property_id($aptObj_or_id) {
  if($_SESSION['dev']) return 42383; // test property
  $fids = object_foreign_key('property', $aptObj_or_id);
  if(!$fids) return 0;
  return json_get_value($fids, SOURCE_ID_KIGO) ?: 0;
}

// returns our property corresponding to Kigo's ID
/** unsused - automatic comment added on  */
function kigo_find_property($kigo_id) {
  $fids_json = '"'.SOURCE_ID_KIGO.'":"'.$kigo_id.'"';
  return find_object('property', array('fids' => "*".$fids_json."*"));
}

// returns our reservation corresponding to Kigo's ID
/** unsused - automatic comment added on  */
function kigo_find_reservation($kigo_id) {
  $fids_json = '"'.SOURCE_ID_KIGO.'":"'.$kigo_id.'"';
  return find_object('reservation', array('fids' => "*".$fids_json."*"));
}

function kigo_guest_object($resObj) {
  //if(!array_filter(array($resObj->first_name, $resObj->last_name, $resObj->email_address))) return null;
  $email = strtolower($resObj->email_address) ?: "mail@domain.com";
  $first_name = $resObj->first_name ?: "[FIRST]";
  $last_name =  $resObj->last_name ?: "[LAST]";
  if(!is_email($email)) return null;
  $country = strtoupper($resObj->country) ?: null;
  //else if(!is_country($country)) return null;
  $guestObj->RES_GUEST_FIRSTNAME = $first_name;
  $guestObj->RES_GUEST_LASTNAME = $last_name;
  $guestObj->RES_GUEST_EMAIL = $email;  
  $guestObj->RES_GUEST_COUNTRY = $country;  
  $guestObj->RES_GUEST_PHONE = pick_first($resObj->phone1, $resObj->phone2, $resObj->phone3, 'n/a');
  return $guestObj;
}


/** unsused - automatic comment added on  */
function kigo_get_reservations($diff_id = NULL, $options = array()) {
  $data = array("DIFF_ID" => $diff_id);
  $api_method = "diffPropertyCalendarReservations";
  $result = kigo_api_call($api_method, $data, $options);
  return $result;  
}

/** unsused - automatic comment added on  */
function kigo_get_reservation($kigo_id, $options = array()) {
  $data = array("RES_ID" => (int) $kigo_id);
  $api_method = "readReservation";
  $result = kigo_api_call($api_method, $data, $options);
  $result_code = $result->API_RESULT_CODE;
  if($result_code == "E_OK") {
    return $result->API_REPLY;
  } else {
    return null;
    //if($kigo_error = kigo_parse_response($result)) $errors[] = $kigo_error;
  }
  //return $result;  
}

function kigo_cancel_reservation($resObj_or_id, $kigo_res_id=0, $options = array()) {
  $errors = array();
  if($resObj_or_id) { 
    $resObj = any2obj('reservation', $resObj_or_id);
    $kigo_res_id = kigo_reservation_id($resObj);
    if(!$kigo_res_id) $errors[] = "No Kigo res_id found for $resObj->res_id";
  } elseif(!$kigo_res_id) {
    $errors[] = "No reservation or kigo reservation id passed";
  }

  if($errors) return array(0, $errors);
  
  $api_method = "cancelReservation";
  $kigoObj->RES_ID = (int) $kigo_res_id;
  $data_json = json_encode($kigoObj);
  $result = kigo_api_call($api_method, $data_json, $options);
  $result_code = $result->API_RESULT_CODE;

  print_log("kigo_cancel_reservation: res_id=$res_id kigo_res_id=$kigo_res_id result=$result_code", 'kigo', LOG_LEVEL_ALWAYS);

  return array($kigo_res_id, $errors);
}

function kigo_res_object($resObj_or_id, $kigo_prop_id=0, $kigo_res_id=0, $options = array()) {
  $resObj = any2obj('reservation', $resObj_or_id);
  if($_SESSION['dev']) $kigo_prop_id = 42383; // test property
  $kigo_res_id = $kigo_res_id ?: kigo_reservation_id($resObj);
  
  if($kigo_res_id) { // update 
    $kigoResObj->RES_ID = (int) $kigo_res_id;
  } else {  // create
    $kigoResObj->PROP_ID = (int) ($kigo_prop_id ?: kigo_property_id($resObj->apt_id));
  }    
  $kigoResObj->RES_CHECK_IN = strip_time($resObj->checkin);
  $kigoResObj->RES_CHECK_OUT = strip_time($resObj->checkout);
  $kigoResObj->RES_N_ADULTS = (int) ($resObj->num_guests ?: 0);
  $kigoResObj->RES_N_CHILDREN = (int) ($resObj->num_children ?: 0);
  $kigoResObj->RES_N_BABIES = (int) ($resObj->num_babies ?: 0);
  if($guestObj = kigo_guest_object($resObj)) $kigoResObj->RES_GUEST = $guestObj;
  
  if(trim($resObj->alert.$resObj->notes)) {
    $kigoResObj->RES_COMMENT = ($resObj->alert ? "NB: $resObj->alert. " : "").$resObj->notes;
  } else {
    $kigoResObj->RES_COMMENT = get_reservation_description($resObj);
  }
  if($resObj->extra_info) $kigoResObj->RES_COMMENT_GUEST = $resObj->extra_info;

  if($user_defined_attributes) $kigoResObj->RES_UDRA = $user_defined_attributes;
  return $kigoResObj;
}

function kigo_update_reservation($resObj_or_id, $kigo_res_id, $options = array()) {
  $resObj = any2obj('reservation', $resObj_or_id);
  if(!$kigo_res_id) return kigo_create_reservation($resObj, $options);
  return kigo_create_or_update_reservation($resObj, $kigo_res_id, $options);
}

function kigo_create_reservation($resObj_or_id, $options = array()) {
  $resObj = any2obj('reservation', $resObj_or_id);
  if($kigo_res_id = kigo_reservation_id($resObj)) return kigo_update_reservation($resObj, $kigo_res_id, $options);
  return kigo_create_or_update_reservation($resObj, 0, $options);
}

function kigo_create_or_update_reservation($resObj_or_id, $kigo_res_id = 0, $options = array()) {
  $resObj = any2obj('reservation', $resObj_or_id);

  $res_id = $resObj->res_id;
  
  $errors = array();
  $kigo_id = (int) SOURCE_ID_KIGO;
  $kigo_prop_id = kigo_property_id($resObj->apt_id);
  $kigo_res_id = $kigo_res_id ?: kigo_reservation_id($resObj);
  
  // error checking
  if(!$kigo_prop_id) $errors[] = "$resObj->apartment_name has no Kigo ID";
  if(!$resObj) $errors[] = "Reservation not found";
  if($resObj->status < STATUS_RESERVED) $errors[] = "Reservation not confirmed (status=$resObj->status)";
  //if($resObj->checkin < today()) $errors[] = "Reservation in the past";
  if($errors) return array(0, $errors);
  $kigoResObj = kigo_res_object($resObj, $kigo_prop_id, $kigo_res_id);  
  
  $api_method = $kigo_res_id ? "updateReservation" : "createConfirmedReservation";
  $debug = false;
  
  if($debug) dump($resData);

  $result = kigo_api_call($api_method, $kigoResObj, $options);
  $result_code = $result->API_RESULT_CODE;
  print_log("create_or_update_reservation: method=$api_method res_id=$res_id kigo_res_id=$kigo_res_id result=$result_code fid=$result->RES_ID", 'kigo', LOG_LEVEL_ALWAYS);
  print_log("result=".dump($result, true), 'kigo', LOG_LEVEL_ALWAYS);
  if(!$kigo_res_id && $result_code == "E_OK") {
    $reply = $result->API_REPLY;
    if($fid = $reply->RES_ID) {
      $fids_json = json_encode(array((string) $kigo_id => (string) $fid));
      print_log("Updating reservation $res_id setting fids='$fids_json'", 'kigo', LOG_LEVEL_ALWAYS);
      list($id, $errors) = update_object('reservation', array('fids' => $fids_json), $res_id);
    }
  }
  if($kigo_error = kigo_parse_response($result)) $errors[] = $kigo_error;
  if($debug) dump($result);
  //echo("code=$result_code kigo error=$kigo_error<br>");
  return array($fid, $errors);
}

function kigo_parse_response($result) {
  $result_code = $result->API_RESULT_CODE;
  if($result_code != "E_OK" && $result->API_RESULT_TEXT) return $result->API_RESULT_TEXT;
  return '';
}

function kigo_api_call($api_method, $data = array(), $options = array()) {
  global $__SITE;
  $siteObj = $__SITE;
  $url = $options['url'] ?: "https://app.kigo.net/api/ra/v1/";
  $options['content_type'] = 'application/json';
  $options['encode_data'] = true; // send data as json string
  $username = $options['username'] = pick_first($options['username'], $siteObj->kigo_username,"youramsterdamhome");
  $password = $options['password'] = pick_first($options['password'], $siteObj->kigo_password,"dEfvHk4N");
  $http_method = $options['http_method'] = isset($options['http_method']) ? $options['http_method'] : "POST";
  $result_json = rest_api_call($url, $api_method, $data, $options);
  $result = json_decode($result_json);
  $result_code = $result->API_RESULT_CODE;
  print_log("Called kigo $api_method. Result code=$result_code", 'kigo', LOG_LEVEL_ALWAYS);
  
  print_log("Kigo data=".dump($data, true)." result=".dump($result, true), 'kigo', LOG_LEVEL_DEBUG);

  return $result;
}

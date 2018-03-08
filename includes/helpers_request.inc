<?php

// prints name: $nn per unit
function product_print($productObj) {
  $unit = uom_name($productObj->unit);      
  return "$productObj->name (".formatFloat($productObj->price, FORMAT_IF_NEEDED, get_currency_symbol($productObj->currency, true))." $unit)";
}

function get_correspondence($parent_type, $parent_id, $user_level) {
  $search['parent_type'] = $parent_type;
  $search['parent_id'] = $parent_id;

  // fetch chat and log
  if($chatObjs = find_objects('chat', $search)) {    
    foreach($chatObjs as $chatObj) {
      if(!auth_action('chat', 'view', $chatObj->id, $user_level)) continue; // not allowed      
      $from        = $chatObj->sender_id ? "" : "";
      $to          = $chatObj->recipient_id ? "" : "";
      //$from_level  = $chatObj->sender_level ? phrase(get_user_level_name($chatObj->sender_level), CAPITALIZE) : phrase('everybody', CAPITALIZE);
      //$to_level    = $chatObj->receiver_level ? phrase(get_user_level_name($chatObj->receiver_level), CAPITALIZE) : phrase('everybody', CAPITALIZE);
      $title       = sql2human($chatObj->timestamp).': '; // .$from_level.' -> '.$to_level;
      $chat_rows[] = html_b($title).': '.$chatObj->message;
    }
  }

  return count($chat_rows) ? implode(html_break(), $chat_rows) : '';
}

// authorize action variables, return authObject
/**
function auth_action_vars($inputVars, $options=NULL) {
  global $u_level;
  global $u_id;
  $errors = array();
  
  $page_type = $options['page_type'] ?: PAGE_STATUS;

  // phase 1: required info  
  $pin = trim($inputVars['pin']);
  $created = $inputVars['ts'];
  $action = $inputVars['action'] ?: $inputVars['act'];
  
  if($id = trim($inputVars['id'])) {
    $enq_type = trim($inputVars['t']) ?: 'reservation';
  } else if($id = trim($inputVars['rid'])) {
    $enq_type = 'reservation';
  }
  if($enq_type == 'order') $enq_type = 'payment_order'; // shorthand

  $authObj->obj_type =   $enq_type; 
  $authObj->created =    $created; 
  $authObj->pin =        $pin; 
  $authObj->id =         $id;

  $obj = get_object($enq_type, $id);

  $valid_types = array('reservation', 'enquiry', 'invoice', 'event', 'block', 'payment_order');

  if(!$enq_type) $errors[] = "auth_action_vars: Missing enquiry type".($_SESSION['live'] ? '' : "<br>enq_type=$enq_type<br>".caller_stack().dump($inputVars,true));
  elseif(!$id) $errors[] = "Missing ID";
  else if(!in_array($enq_type, $valid_types)) $errors[] = phrase('invalid', CAPITALIZE)." ".phrase('type', LOWERCASE).' '.$enq_type;
  else if(!$pin && !$created) {
    if($u_id && $u_id == $obj->user_id) $object_host = true; // object belongs to this user 
    else if($u_level < USER_LEVEL_STAFF) $errors[] = phrase('login', CAPITALIZE)." ".phrase('or').' '.phrase('pin', UPPERCASE).' '.phrase('is_required', LOWERCASE);
  }

  if($errors) {$authObj->errors = $errors;return $authObj;}
  
  // phase 2: object exists and is valid 
  if($enq_type == 'reservation') {      
    $enq_name = $resObj->status >= STATUS_RESERVED ? phrase('reservation', CAPITALIZE) : phrase('request', CAPITALIZE);
  } else {
    $enq_name = phrase(get_object_name($enq_type), CAPITALIZE);
  }

  if(!$obj) $errors[] = "$enq_name $id: ".phrase('not_found', LOWERCASE);
  if($obj && !$obj->created) $errors[] = "$enq_name $id: Missing TS";
  if($obj && !$obj->booking_pin) $errors[] = "$enq_name $id: Missing PIN";
  if($errors) {$authObj->errors = $errors;return $authObj;}

  // phase 3: correct TS/Pin 
  if($created && $created != myhash(stripdate($obj->created))) $errors[] = phrase('invalid', CAPITALIZE)." ".phrase('ts', LOWERCASE);
  if($pin && $pin != myhash($obj->booking_pin)) $errors[] = phrase('invalid', CAPITALIZE)." ".phrase('pin', LOWERCASE);
  if($errors) {$authObj->errors = $errors;return $authObj;}

  // phase 4: all good, populate auth object 
  if($inputVars['search']) { // search from form
    if(!$id) $errors[] = phrase('id', UPPERCASE)." ".phrase('is_required', LOWERCASE);
    if($u_level < USER_LEVEL_STAFF && !$pin) $errors[] = phrase('pin', UPPERCASE)." ".phrase('is_required', LOWERCASE);
    if(!$errors) {
      $pin = myhash($pin); // hash the pin
      $user_level = USER_LEVEL_GUEST;
      $user_type = 'guest';  
    }
  } elseif($created && $pin) { // link with hashed PIN and timestamp (staff)
    $user_level = USER_LEVEL_STAFF;
    $user_type = 'staff';  
  } elseif($created) {         // link with hashed timestamp (host)
    $user_level = USER_LEVEL_OWNER;
    $user_type = 'host';  
  } elseif($pin) {        // link with hashed pin - do not submit (guest)
    $user_type = 'guest';  
    $user_level = USER_LEVEL_GUEST;
  } else {
    if($object_host || $u_level >= USER_LEVEL_STAFF) { // logged in, no need for vars, redirect with vars
      $status_link = action_link($enq_type, $id, array('user_level' => $u_level, 'action' => $action));
      //echo("Redirecting to $status_link");
      redirect($status_link);
    } else {
      $errors[] = "Secret key missing";
    }
  }
  
  $auth_edit = $u_level && auth_action($enq_type, 'edit', $id);
  
  $key = get_object_key($enq_type);  

  $authObj->obj =        $obj;      
  $authObj->obj_key  =   $key; 
  $authObj->obj_name =   $enq_name;
  $authObj->auth_edit =  $auth_edit;
  
  $authObj->user_level = $user_level;
  $authObj->user_type =  $user_type;
  $authObj->errors =     $errors;      
  
  if($options['fetch']) {
    $enq_type       = $authObj->obj_type;
    $enq_name       = $authObj->obj_name;
    $enq_key = $key = $authObj->obj_key;
    $auth_edit      = $authObj->auth_edit;
    $id             = $authObj->id;
    $user_level     = $authObj->user_level;
    $user_type      = $authObj->user_type;
    $enq_obj = $obj = $authObj->obj;
    $errors         = $authObj->errors;
    $created        = $authObj->created;
    $pin            = $authObj->pin;
    switch($enq_type) {
      case 'invoice':
        $invObj = $obj;
        $inv_id = $id;
        $inv_parent_type = $invObj->parent_type;
        $inv_parent_id = $invObj->parent_id;
        if($inv_parent_type && $inv_parent_id) {
          $invParentObj = get_object($inv_parent_type, $inv_parent_id);
          switch($inv_parent_type) {
          case 'reservation':
            $resObj = $invParentObj;
            $res_id = $inv_parent_id;
            break;
          case 'block':
            $blockObj = $invParentObj; 
            $block_id = $inv_parent_id;
            break;
          case 'payment_order':
            $orderObj = $invParentObj;
            $order_id = $inv_parent_id;
            break;            
          }
          //if(!$live) echo("Invoice; pt=$inv_parent_type pid=$inv_parent_id res_id=$res_id resObj=$resObj->res_id<br>");
        } else {
          if($res_id = $invObj->res_id) {
            $inv_parent_id = $res_id;
            $inv_parent_type = 'reservation';
            $resObj = $invParentObj = get_object($inv_parent_type, $inv_parent_id);
          } elseif($order_id = $invObj->order_id) {
            $inv_parent_id = $order_id;
            $inv_parent_type = 'payment_order';
            $orderObj = $invParentObj = get_object($inv_parent_type, $inv_parent_id);
          } elseif($block_id = $invObj->block_id) {
            $inv_parent_id = $block_id;
            $inv_parent_type = 'block';
            $blockObj = $invParentObj = get_object($inv_parent_type, $inv_parent_id);
          }
          //if(!$live) echo("Invoice no pt/pid; pt=$inv_parent_type pid=$inv_parent_id res_id=$res_id resObj=$resObj->res_id<br>");
        }
        break;
      case 'reservation':
        $res_id = $id;
        $resObj = $obj;
        //dump($inputVars);echo("t=$enq_type id=$id");
        // dump($resObj);die("rid=$resObj->res_id");
        if($invObj = find_booking_invoice($resObj->res_id)) $res_inv_id = $invObj->id;
        if($mismatch = $auth_edit ? reservation_invoice_mismatch($resObj, array('values' => true)) : false) {
          $errors[] = "Invoice mismatch:<br>Reservation: $mismatch[reservation]<br>Invoice: $mismatch[invoice]<br>Difference: $mismatch[difference]";
        }
        
        break;
      case 'enquiry':
        $enq_id = $id;
        $enqObj = $obj;
        break;
      case 'event':
        $event_id = $id;
        $eventObj = $obj;
        break;
      case 'payment_order':
        $order_id = $id;
        $orderObj = $obj;
        if($invObj = find_order_invoice($orderObj->id)) $inv_id = $invObj->id;
        break;
      case 'block':
        $block_id = $id;
        $blockObj = $obj;      
        if($res_id = $obj->res_id) $resObj = get_object('reservation', $res_id);
        break;
      default:
        die("Invalid enq_type $enq_type");
    }
    $authObj->resObj = $resObj;
    $authObj->apt_id = $resObj->apt_id;
    $authObj->property_type = $resObj->property_type;
  }
  
  return $authObj;
}
*/

// button with link to status page
function enq_status_button($obj_type, $id, $user_level=USER_LEVEL_GUEST, $ref_link='', $ref_name='') {
  $link = action_link($obj_type, $id, array('user_level' => $user_level, 'ref_link' => $ref_link, 'ref_name' => $ref_name));
  return html_image_link($link, '/images/icons/web_page_tiny.png', phrase('status', CAPITALIZE));
}

/*
// link to status page (linkonly) - deprecated, use action_link
function enq_status_link($obj_type, $id, $user_level=USER_LEVEL_GUEST, $ref_link='', $ref_name='') {
  return action_link($obj_type, $id, array('user_level' => $user_level, 'ref_link' => $ref_link, 'ref_name' => $ref_name));
    $ts  = $user_level >= USER_LEVEL_OWNER ? myhash(stripdate($obj->created)) : '';
    $pin = $user_level == USER_LEVEL_GUEST || $user_level >= USER_LEVEL_STAFF ? myhash($obj->booking_pin) : ''; // hide booking PIN unless staff or higher
    $hidden_fields = array_filter(array('t' => $obj_type, 'id' => $id, 'ref_link' => $ref_link, 'ref_name' => $ref_name, 'ts' => $ts, 'pin' => $pin));
    $link = get_page_link(PAGE_STATUS).'?'.array2qs($hidden_fields);
  }
  return $link;
}
*/

// returns vars needed for process.php
function public_action_vars($obj_or_id, $obj_type='reservation', $user_level=USER_LEVEL_USER, $action='') {
  $obj = any2obj($obj_type, $obj_or_id);
  $pin = $obj->booking_pin;
  if(!strlen($pin)) update_object($obj_type, array('booking_pin' => generate_booking_pin()), $obj->id ?: $obj->res_id);
  return array('pin' => myhash($pin));

  /** Not in use anymore, except PIN 
  global $u_level;
  //if(!$user_level) $user_level = $u_level ? $u_level : USER_LEVEL_GUEST; // default to logged in user if any, otherwise guest  
  if(!$obj) {
    //dump($obj);
    //echo("public_action_vars: obj_type=$obj_type ul=$user_level: missing object<br>");
    return array();
  }
  $key = get_object_key($obj_type);
  $id = $obj->$key;  
  if(!$id) {
    return array();
    die("public_action_vars: obj_type=$obj_type action=$action, missing id on object<br>");;
  }
  
  
  $vars['t'] = $obj_type;
  $vars['id'] = $id;
  if($user_level >= USER_LEVEL_USER && $user_level <= USER_LEVEL_GUEST) {
    $vars['pin'] = myhash($pin);
  } elseif($user_level == USER_LEVEL_OWNER) {
    $vars['ts'] = myhash(stripdate($obj->created));
  } elseif($user_level > USER_LEVEL_OWNER) {
    $vars['pin'] = myhash($pin);
    $vars['ts'] = myhash(stripdate($obj->created));
  } 
  if($action) $vars['act'] = $action;
  //if($obj_type == 'block') {echo("pin=$pin ul=$user_level");dump($vars);}
  return $vars;
  */ 
}

// secure links used on screen to accept/decline/correspond about reservation, enquiry, or invoice

function action_link($obj_type, $obj_or_id, $options = NULL) {
  $full       = $options['full'];
  $pin        = $options['pin'];
  $page_type  = $options['page_type'] ?: PAGE_STATUS;
  $action     = $options['action'] ?: $options['act'];
  $user_level = $options['user_level'] ?: USER_LEVEL_USER;
  $obj = any2obj($obj_type, $obj_or_id);
  $key = get_object_key($obj_type);
  $id = $obj->$key;
  
  if(!$obj) {
    //die("no obj for $obj_type".dump($obj_or_id, true));
    return '';
   }
   
  switch($obj_type) {
  case 'invoice':
    $page_type = PAGE_INVOICE;    
    break;
  case 'payment':
    $page_type = PAGE_PAYMENT;    
    break;
  case 'feedback':
    $page_type = PAGE_FEEDBACK;    
    break;
  default:
    break;
  }
  
  if(in_array($action, array('pay','paying'))) $page_type = PAGE_PAYMENT;
  else if(in_array($action, array('feedback'))) $page_type = PAGE_FEEDBACK;
  
  //if(!$vars) return '';
  if($full) { // public, for emails
    $site_id = $obj->site_id ?: $_SESSSION['site_id'];
    if(!$site_id) {
      //die("no site id".dump($obj, true));
      return '';
    }
    $base_link = site_pagetype_link($obj->site_id, $page_type, $obj->language, true, true);
  } else {
    $base_link = get_page_link($page_type);
  }
  $base_link = str_replace('//test.', '//www.', $base_link); /** this allows us to use test to send real mail */
  $encode = isset($options['encode']) ? $options['encode'] : true;  
  //echo("bl for $obj_type pt=$page_type: $base_link");
  if($obj->booking_pin && $pin || ($user_level < USER_LEVEL_GUEST || ($user_level == USER_LEVEL_GUEST && !$obj->guest_id))) {
    $vars = public_action_vars($obj, $obj_type, $user_level, $action);
    return $base_link."$id/?".array2qs($vars, $encode);
  }
  
  return $base_link.$id.'/';
}

// public secure links used in emails to accept/decline/correspond about reservation, enquiry, or invoice
function public_action_link($obj_or_id, $obj_type, $options = NULL) {  
  $options['full'] = true; // return full link using the site of the object
  $options['pin'] = true; // force pin 
  return action_link($obj_type, $obj_or_id, $options);
}

// returns link to calendar for given type/object
function calendar_link($type, $obj) {
  if(!$obj) return '';
  if($type == 'enquiry') return '';
  $key = get_object_key($type);
  //$status_field = object_status_field($type);
  $status_field = 'status';

  list($start_field, $end_field) = object_timefields($type);
  if(!$start_field) return '';
  $in = $obj->$start_field;
  $out = $obj->$end_field;
  $now = now();
  
  if($in <= $now && $out >= $now) { // go to this month if reservation spans today
    //echo("in: $in <= $now && out: $out >= $now<br>");
    list($d, $m, $y) = sql2date($now);  
  } else {
    //echo("NOT: in: $in <= $now && out: $out >= $now<br>");
    list($d, $m, $y) = sql2date($in);
  }
  if($obj->$status_field < 0) $qsAr['st'] = -100;
  $qsAr['t'] = $obj->property_type;
  $qsAr['show'] = $type;
  $qsAr['cid'] = $obj->city_id;
  $qsAr['mode'] = SHOW_CALENDAR;
  $qsAr['ownr'] = $obj->host_id;
  $qsAr['m'] = $m;
  $qsAr['y'] = $y;
  if($obj->num_hours > 0) $qsAr['d'] = $d;
  
  $qsAr['id'] = $obj->$key;
  if($obj->rate_type == 'hour') $qsAr['d'] = $d; // hourly reservations (todo: make work for invoices)
  if($type == 'reservation') $qsAr['t'] = $obj->property_type;  
  return get_page_link(PAGE_CALENDAR).'?'.array2qs($qsAr);
}


function  print_block_form($block_id, $cur, $rate, $max, $name, $smoking_policy, $index, $subindex=0) {
  $index_str = $index ? " # $index" : "";
  $sub_index_str = $subindex ? " # $subindex" : "";
  $output  = "<div class='block_roomname'><b>".phrase('room', CAPITALIZE)." $index_str:</b> $name $sub_index_str</div><!--roomname-->";

  $prefix = "room:".$index;

  $smoking_array = array(0 => phrase('no_preference', CAPITALIZE), 1 => phrase('non_smoking_preferred', CAPITALIZE), 2 => phrase('smoking_preferred', CAPITALIZE));

  $block_fields[phrase('guest_last_name', CAPITALIZE)]  = form_input("$prefix:guest_name", 'text', $_POST["$prefix:guest_name"], array('size' => 20, 'max' => 40));
  $block_fields[phrase('guests', CAPITALIZE)]           = select_from_array("$prefix:guest_qty", num_array(1, $max, 1), ($_POST["$prefix:guest_qty"] ? $_POST["$prefix:guest_qty"] : $max));
  if($smoking_policy == 1) {
    $block_fields[phrase('smoking', CAPITALIZE)]          = phrase('non_smoking_only', CAPITALIZE).form_input("$prefix:smoking", 'hidden', 0);
  } else {
    $block_fields[phrase('smoking', CAPITALIZE)]          = select_from_array("$prefix:smoking", $smoking_array, $_POST["$prefix:smoking"]);
  }

  //$block_form .= print_array_assoc($block_fields, 'line');
  $block_form .= html_fieldset($block_fields, 'fieldset_top');

  $output  .= "<div class='block_form'>$block_form</div><!--block_form-->";
  $output  .= form_input("_".$prefix, 'hidden', "$block_id:$cur:$rate:$max:$name");

  return $output ;
}

// login form used in process.php
function login_form_process($inputVars) {
  $req_type_array = array('reservation' => phrase('reservation', CAPITALIZE), 'enquiry' => phrase('enquiry', CAPITALIZE), 'invoice' => phrase('invoice', CAPITALIZE));
  $fields[phrase('type', CAPITALIZE)] = select_from_array('t', $req_type_array, $inputVars['t']);
  $fields[phrase('id', CAPITALIZE)]           = form_input('id', 'text', $inputVars['id'], array('size' => 6, 'max' => 8));
  $fields[phrase('pin', CAPITALIZE)]          = form_input('pin', 'text', $inputVars['pin'], array('size' => 6, 'max' => 6)).' '.form_input('search', 'submit', phrase('search', CAPITALIZE));
  return html_form(html_fieldset($fields, 'fieldset_left_wide'), 'process_login', get_page_link(PAGE_STATUS));  
}

// returns either [ edit ] or [ cancel ] link for editing a section in process.php
function edit_section_link($base_link, $edit, $section) {
  $edit_link = $edit==$section ? html_link($base_link, '[ '.phrase('cancel', CAPITALIZE).' ]') : html_link(replace_uri_var($base_link,"act", 'edit').'&amp;section='.$section.'#'.$section, '[ '.phrase('edit', CAPITALIZE).' ]');
  return $edit_link;
}

// returns array of lines due for a direct internet reservation where no invoice yet exists
function get_reservation_due($resObj) {
  if($invObj = find_booking_invoice($resObj->res_id)) return get_due_now($invObj, $resObj);
  if($resObj->status >= STATUS_RESERVED) return $resObj->balance;
  if($resObj->deposit_percentage >= 100 || $resObj->deposit_percentage <= 0) return $resObj->balance;  
  $due_now = ($resObj->deposit_percentage/100) * $resObj->grand_total - $resObj->paid;
  return $due_now > INV_TOLERANCE ? number_format($due_now, 2, '.', '') : 0;  
}


function payment_details($inv_id_or_obj, $user_level, $context='screen') {  
  if(!$inv_id_or_obj) return '';
  $invObj = any2obj('invoice', $inv_id_or_obj);
  if(!$invObj) return '';
  //if($invObj)  = find_booking_invoice($resObj->res_id);
  require_once('db_invoice.inc');
  $due_now = get_due_now($invObj);   // how much is due now (either full amount or deposit + fees);
  return print_payment_due($invObj, $due_now, $user_level, $context);
}


// takes a draft invoice object and prints the lineitems
function print_payment_due($invObj, $due_now, $user_level=USER_LEVEL_GUEST, $context='screen') {
  return print_invoice($invObj, $user_level, array('due_now' => $due_now, 'context' => $context)); // draft invoice
}

// takes an array of invoice object and prints them
function print_receipts($invObjs, $user_level=USER_LEVEL_GUEST, $context='screen') {
  foreach($invObjs as $invObj) $output .= print_invoice($invObj, $user_level, array('due_now' => $due_now, 'context' => $context));        
  return $output;    
}

// prints list of payments 
function print_withdrawals($withdrawals, $user_level=USER_LEVEL_GUEST) {
  $rows = array();
  $rows[]=  array(html_b(phrase('date', CAPITALIZE)), html_b(phrase('method', CAPITALIZE)), html_b(phrase('amount', CAPITALIZE)), html_b(phrase('fee', CAPITALIZE)), html_b(phrase('net', CAPITALIZE)), html_b(phrase('recipient', CAPITALIZE)), '');
  foreach($withdrawals as $withdrawalObj) {    
    $actions = class_actions_withdrawal('withdrawal', $withdrawalObj, 'recordset', $ref_name, $ref_link, $user_level);
    // if($user_level >= USER_LEVEL_STAFF) $actions = array_merge($actions, default_class_actions('payment', $withdrawalObj->id, 'payment', '',0, 0, 'Status', urlencode($_SERVER['REQUEST_URI']), 'recordset')); // add standard buttons or links
    $actions_str = $actions ? implode('', $actions) : '';

    if(!$cur) $cur = get_currency_symbol($withdrawalObj->currency, true); 
    $rows[]=  array(sql2human($withdrawalObj->created, array('show_time' => true)), get_object('payment_type', $withdrawalObj->payment_type, 'name'), $cur.$withdrawalObj->gross, $cur.$withdrawalObj->fee, $cur.$withdrawalObj->net, $withdrawalObj->recipient, $actions_str);
  }
  return html_table($rows);
  //return css_table($rows);
}

function print_booking_payments($resObj_or_id, $user_level=USER_LEVEL_GUEST) {
  $resObj = any2obj('resoice', $resObj_or_id);
  $paymentObjs = find_objects('payment', array('res_id' => $resObj->res_id, 'payment_status' => array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PENDING, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED)));
  return print_payments($paymentObjs, $user_level);
}

// prints list of payments 
function print_payments($payments, $user_level=USER_LEVEL_GUEST) {
  // BEGIN OUTPUT
  //$output = html_subheader(phrase('payments', CAPITALIZE));
  if(!count($payments)) return '';
  $rows = array();
  $rows[]=  array(html_b(phrase('date', CAPITALIZE)), html_b(phrase('method', CAPITALIZE)), html_b(phrase('amount', CAPITALIZE)), html_b(phrase('status', CAPITALIZE)), html_b(phrase('notes', CAPITALIZE)));
  $refundable_types = find_objects('payment_type', array('refundable' => 1), array('fields' => 'id'));
  $refundable_statuses = array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PARTIALLY_REFUNDED);
  //dump($refundable_types); 
  foreach($payments as $paymentObj) {
    $actions = array();
    $ignore_payment_types = array(PAYMENT_TYPE_TRANSFER, PAYMENT_TYPE_AGENT_OWES);
    if($user_level == USER_LEVEL_GUEST && in_array($paymentObj->payment_type, $ignore_payment_types)) continue; // don't show for guests
    if($user_level == USER_LEVEL_GUEST && $paymentObj->payment_source != PAYMENT_SOURCE_GUEST) continue; // don't show for guests
    /** turned off for now
    if($user_level >= USER_LEVEL_GUEST) $actions = class_actions_payment('payment', $paymentObj, 'recordset', $ref_name, $ref_link, $user_level);
    if($user_level >= USER_LEVEL_STAFF) $actions = array_merge($actions, default_class_actions('payment', $paymentObj->id, 'payment', '',0, 0, 'Status', urlencode($_SERVER['REQUEST_URI']), 'recordset')); // add standard buttons or links
    */
    if($user_level >= USER_LEVEL_STAFF) {
      global $u_id;
      $site_id = $_SESSION['site_id'];
      $verified = $paymentObj->verified;
      $can_refund = $verified && $paymentObj->amount > 0 && in_array($paymentObj->payment_type, $refundable_types) && in_array($paymentObj->payment_status, $refundable_statuses);
      if($verified) $actions[] = html_link("/admin/app_data.php?oper=verify-payment&site_id=$site_id&user_id=$u_id&payment_id=$paymentObj->id", html_i('', 'fa fa-check text-success'), array('class' => '', 'title' => phrase('verified', CAPITALIZE), 'data-data' => array('payment_id' => $paymentObj->id, 'gateway_id' => $paymentObj->gateway_id), 'data-toggle' => "modal", 'data-target' => "#verify_modal"));
      if($can_refund) {
        $icon = '<span class="fa-stack fa-lg"><i class="fa fa-money fa-stack-1x text-success"></i><i class="fa fa-long-arrow-left fa-stack-1x "></i></span>';
        $actions[] = html_link('#', $icon, array('class' => 'fill-form', 'data-form' => '#refund_form', 'data-data' => array('payment_id' => $paymentObj->id, 'amount' => $paymentObj->amount, 'currency' => $paymentObj->currency), 'data-toggle' => "modal", 'data-target' => "#refund_modal", 'title' => phrase('refund', CAPITALIZE)));
      }
      //$actions = array_merge($actions, default_class_actions('payment', $paymentObj->id, 'payment', '',0, 0, 'Status', urlencode($_SERVER['REQUEST_URI']), 'recordset')); // add standard buttons or links
    }
    
    $actions_str = $actions ? implode('', $actions) : '';

    $payment_status = get_object('payment_status', $paymentObj->payment_status, 'name');

    // payment amount
    $cur = get_currency_symbol($paymentObj->currency, true);
    $pay_amount  = formatFloat($paymentObj->amount, FORMAT_ALWAYS, $cur);
    // echo("pay amount for payment $paymentObj->id = $pay_amount<br>");
    if($paymentObj->payment_currency && $paymentObj->payment_gross && $paymentObj->currency != $paymentObj->payment_currency) {
      $rcv_cur    = get_currency_symbol($paymentObj->payment_currency, true);
      $rcv_amount  = $paymentObj->payment_currency == 'BTC' ? "$rcv_cur ".clean_num($paymentObj->payment_gross) : formatFloat($paymentObj->payment_gross, FORMAT_ALWAYS, $rcv_cur);
      $pay_amount_str = $rcv_amount.' => '.$pay_amount;
    } else {
      $pay_amount_str = $pay_amount;
    }

    $row = array(sql2human($paymentObj->created, array('show_time' => true)), get_object('payment_type', $paymentObj->payment_type, 'name'), $pay_amount_str);
    if($payment_status) $row[] = $payment_status;
    $row[] = $paymentObj->description;
    if($actions_str) $row[] = $actions_str;    
    $rows[]= $row;
  }
  return html_table($rows, array('class' => 'accounting_table'));
  // return css_columns(rows2cols($rows), '', '', array('', 'right', '', ''));  
}

// takes a reserved reservation and prints balance due to host
function print_balance($resObj) {
  $res_id = $resObj->res_id;
  if(!$res_id) return;

  // BEGIN OUTPUT
  $output = html_subheader(phrase('balance', CAPITALIZE)." (".phrase('invoice', CAPITALIZE)." # $inv_id)");
  return $output;    
}

// prints bitcoin button for express checkout (or 2, if amount due != total)
//function bitcoin_button($currency, $amount, $cur_per_btc = 0, $hidden_fields='', $options = NULL) {
//  $cur = get_currency_symbol($currency, true);                            
//  if(!$cur_per_btc) $cur_per_btc = get_btc_rate('BTC', $currency);
//  //echo("1 $cur = $cur_per_btc BTC");
//  $cur_btc = get_currency_symbol('BTC', true);
//  $hidden_fields['pay'] = 1; // used only in request.php to process
//  $hidden_fields['currency'] = $currency;
//
//  if($options['custom']) {
//    $form .= phrase('pay', CAPITALIZE).' '." $cur ".form_input('custom_amount', 'text', $inputVars['custom_amount'], array('size' => 8, 'max' => 20, 'id' => 'btc_custom_amount')).html_break(2);
//    $hidden_fields['pay_custom'] = 1;
//    $hidden_fields['cur_per_btc'] = $cur_per_btc;
//    $form .= form_input('amount_btc', 'hidden', $inputVars['custom_amount'], array('size' => 8, 'max' => 20, 'id' => 'custom_amount_btc'));
//    $amount_btc = 0;  
//    $form .= html_social_button("#", phrase('pay', CAPITALIZE)." $cur_btc ".html_span($amount_btc, '', 'id="custom_amount_btc_text"')." BTC", array('class' => 'bitcoin submit_parent_form')); 
//  } else {
//    $amount_btc = clean_num(number_format($amount / $cur_per_btc, 8));
//    $form .= phrase('pay', CAPITALIZE).' '.formatFloat($amount, FORMAT_ALWAYS, $cur).html_break(2);
//    $hidden_fields['amount'] = $amount;
//    $hidden_fields['amount_btc'] = $amount_btc;
//    $form .= html_social_button("#", phrase('pay', CAPITALIZE)." $cur_btc $amount_btc BTC", array('class' => 'bitcoin submit_parent_form')); 
//  }
//
//  $hidden_fields['payment_type'] = PAYMENT_TYPE_BITCOIN;
//  $form .= array2form($hidden_fields);
//  return html_div(html_form($form, 'bitcoin_express_form'), 'center bold');
//}

// returns comma separated list of cards accepted by property of type $ptype and id = $id
// + always accept Amex, Visa, MC
function accepted_cc_cards($ptype, $pid) {
  $cc_list = array(1,2,3); // always accept Amex, Visa, MC
  if($ptype && $pid && $cc_property_list = get_object_field($ptype, $pid, 'cards_accepted')) $cc_list = array_merge(explode(',', $cc_property_list), $cc_list); 
  return implode(',', array_unique($cc_list)); // merge with cards accepted by hotel  
}

// Amex message displayed when using Amex for non-USD transactions (PayPal only allows Amex transactions in USD)
// Todo: Maybe update button-text ?
function currency_message($payment_type, $supported_currency, $currency, $total_due=0, $due_now=0, $class='') {
  $cur = get_currency_symbol($currency, true);
  $sup_cur = get_currency_symbol($supported_currency, true);
  $class = $class ? "message $class" : 'message';    
  if($due_now) $due_now_sc = get_exchange_rate($currency, $supported_currency, $due_now, true);
  
  $message .= "Our gateway only supports $supported_currency for $payment_type.".html_break();
  if($due_now) $message .= "Amount due in $supported_currency: ".$sup_cur.$due_now_sc." ( = ".formatFloat($due_now, FORMAT_ALWAYS, $cur).").".html_break();
  if($total_due && $due_now != $total_due) {
    $total_sc = get_exchange_rate($currency, $supported_currency, $total_due, true);
    $message .= "Total due in $supported_currency: ".$sup_cur.$total_sc." ( = ".formatFloat($total_due, FORMAT_ALWAYS, $cur).").".html_break();
  }
  //echo("supcur = $supported_currency cur=$currency total=$total_due ($total_sc - $total_due_usd) due_now=$due_now ($due_now_sc - $amount_due_usd) <br>");
  
  return html_div($message, $class, "id='amex_info'");
}

function print_simple_address_form($obj_or_vars, $options=NULL) {
  $fields = address_fields($obj_or_vars, $options);
  return html_fieldset($fields, 'fieldset_top');
}

// return assoc. array of simple address fields (edit)
function address_fields($obj_or_vars, $options=NULL) {
  $len = 15;
  $input_options = array('size' => 15, 'max' => 50, 'class' => 'request_form_text_field'); 
  $obj = is_object($obj_or_vars) ? $obj_or_vars : (object) $obj_or_vars;
  $cc = $options['no_country_default'] ? '' :  $_SESSION['country_code'];
  $fields = array();
  if($corporate = $options['corporate']) $fields[phrase('organization', CAPITALIZE)] = form_input('organization', 'text', $obj->organization, $input_options); 
  
  $fields2 =  array(phrase('address', CAPITALIZE)       => form_input('address1', 'text',  $obj->address1 ?: $obj->address, $input_options),
                   phrase('address', CAPITALIZE).'...' => form_input('address2', 'text',  $obj->address2, $input_options),
                   phrase('city', CAPITALIZE)          => form_input('city', 'text',  $obj->city, $input_options),
                   phrase('state_province', CAPITALIZE)=> form_input('state', 'text',  $obj->state ?: $obj->region, $input_options),
                   phrase('postal_code', CAPITALIZE)   => form_input('zip', 'text',  $obj->zip ?: $obj->postal_code, $input_options));
  $fields = array_merge($fields, $fields2);
  if(!$options['hide_phone']) $fields[phrase('phone', CAPITALIZE)] = form_input('cell_phone', 'text',  pick_first($obj->cell_phone, $obj->phone, $obj->home_phone, $obj->work_phone) , $input_options);
   $fields[phrase('country', CAPITALIZE)] = html_country_select('country', array('value' => $obj->country, 'class' => 'request_form_text_field'));

  return $fields;                 
}

function name_fields($obj_or_vars, $options = NULL) {  
  global $salutationArray;
  $input_options = array('size' => 15, 'max' => 50, 'class' => 'request_form_text_field'); 
  $confirm_email = $options['confirm_email'];
  $password = $options['password'];
  $template = $options['template'];

  $len = 15;
  $obj = is_object($obj_or_vars) ? $obj_or_vars : (object) $obj_or_vars;
  $agent_id = $obj->agent_id;
  $agent_type = $agent_id ? get_object('agent', $obj->agent_id, 'type') : 0;
  $title = $agent_type == AGENT_TYPE_HOTEL ? phrase('room', CAPITALIZE) : phrase('title', CAPITALIZE); // the agent is a hotel, show room field

  if($password) {
    //$pw_tip =
    $pw_key = phrase('password', CAPITALIZE)." (".phrase('new_account', CAPITALIZE).")";
    $password_input = form_input('password', 'password',     '', $input_options).html_div(html_password_meter(), 'inline');    
    $fields[$pw_key] = $password_input;
  }

  switch($template) {
  case 'corporate': // first*/[infix]/last*/salutation/email*/email2/organization/title/[password]/
    $fields =  array(html_b(phrase('first_name', CAPITALIZE))    => form_input('first_name', 'text',    $obj->first_name, $input_options),
                   phrase('title', CAPITALIZE)    => form_input('title', 'text',    $obj->title, $input_options),
                   html_b(phrase('last_name', CAPITALIZE))     => form_input('last_name', 'text',     $obj->last_name, $input_options),
                   phrase('salutation', CAPITALIZE)            => select_from_array('salutation',     $salutationArray, $obj->salutation),
                   html_b(phrase('email_address', CAPITALIZE)) => form_input('email_address', 'text', $obj->email_address, $input_options),
                   html_b(phrase('confirm', CAPITALIZE)." ".phrase('email_address', LOWERCASE)) => form_input('email_address2', 'text', pick_first($obj->email_address2, $confirm_email), $input_options),
                   phrase('organization', CAPITALIZE)          => form_input('organization', 'text',  $obj->organization, $input_options),
                   $title                                      => form_input('title', 'text',         $obj->title, $input_options));
    break;
  case 'name_only': // first*/[infix]/last*/email*/[password]/
    $fields =  array(html_b(phrase('first_name', CAPITALIZE))    => form_input('first_name', 'text',    $obj->first_name, $input_options),
                     html_b(phrase('last_name', CAPITALIZE))     => form_input('last_name', 'text',     $obj->last_name, $input_options));
    break;
  case 'name': // first*/[infix]/last*/email*/[password]/
    $fields =  array(html_b(phrase('first_name', CAPITALIZE))    => form_input('first_name', 'text',    $obj->first_name, $input_options),
                     html_b(phrase('last_name', CAPITALIZE))     => form_input('last_name', 'text',     $obj->last_name, $input_options),
                     html_b(phrase('email_address', CAPITALIZE)) => form_input('email_address', 'text', $obj->email_address, $input_options));
    break;
  default: // email*/[password]/first/[infix]/last    
    $fields =  array(html_b(phrase('email_address', CAPITALIZE)) => form_input('email_address', 'text', $obj->email_address, $input_options),
                     $pw_key => $password_input,
                     phrase('first_name', CAPITALIZE)    => form_input('first_name', 'text',    $obj->first_name, $input_options),
                     phrase('last_name', CAPITALIZE)     => form_input('last_name', 'text',     $obj->last_name, $input_options));
                     
    break;
  }
  
  if($agent_id) {
    if($agent_contact = $obj->agent_contact) {
      $contact = get_object('contact', $agent_contact);
      $fields[phrase('booked_by', CAPITALIZE)] = get_name_string($contact);
    } elseif($agent_contacts = find_object('contact', array('agent_id' => $agent_id))) {
      //$fields[phrase('booked_by', CAPITALIZE)] = get_select_box_off('agent_contact', 'contact', $obj->agent_contact, '', array('agent_id' => $agent_id));
      $fields[phrase('booked_by', CAPITALIZE)] = object_select('agent_contact', 'contact', array('value' => $obj->agent_contact, 'filters' => array('agent_id' => $agent_id)));
    }        
  }
  
  return $fields;
}

// prints list of events for this reservation
function reservation_catering($resObj, $options=NULL) {  
  if(!$resObj) return '';
  $can_edit = $options['can_edit'];
  $can_edit = 1; // todo, remove
  $form_name = 'add_catering_form';   

  if($inv_ids = find_reservation_invoices($resObj->res_id, array('field' => 'invoice.id'))) {
    $search['inv_id'] = $inv_ids;
    $search['product_id'] = array('>', 0);
    $search['type'] = array(LI_TYPE_PRODUCT, LI_TYPE_CATERING, LI_TYPE_PACKAGE);
  
    if($liObjs = find_objects('inv_lineitem', $search, array('sort' => 'inv_id'))) {
      $rows[] = phrase(array("description", "quantity", "amount", "total", "supplier", "status"), CAPITALIZE);
      foreach($liObjs as $liObj) {
        $li_id = $liObj->id;
        $product_id = $liObj->product_id;
        if($supplier_ids = find_objects('product_supplier_map', array('product_id' => $product_id), array('fields' => 'supplier_id'))) {
          if(count($supplier_ids) > 1) {
            $supplier_chooser = object_select('supplier_id_'.$li_id, 'supplier',  array('search' => array('supplier.id' => $supplier_ids)));
          } else {
            $supplier_chooser = form_input('supplier_id_'.$li_id, 'hidden', $supplier_ids[0]).html_b(get_object('supplier', $supplier_ids[0], 'address.organization')); 
          }
          //dump($supplier_ids);
          if($eventObj = find_object('event', array('lineitem_id' => $liObj->id))) {
            $st = get_object('event_status', $eventObj->status);
            $status_str = html_span($st->name, 'bold', "style='color:#$st->color'");
          } else {
            $status_str = html_link("#", "[ ".phrase('plan', CAPITALIZE)." ]", array('js' => 'onclick="load_catering(\''.$resObj->res_id.'\', \''.$li_id.'\');return false"'));
          }
          $product_name = get_object('product', $product_id, 'name');
          $rows[] = array($product_name, print_price($liObj, 'amount', FORMAT_ALWAYS), clean_num($liObj->quantity), print_price($liObj, 'total', FORMAT_ALWAYS), $supplier_chooser, $status_str); 
        }
      }
    }
    $table = count($rows) ? html_table($rows, array('class' => 'accounting_table')) : '';
  }

  if(!$table) return '';
  
  // add event button
  $submit = form_input('add_event', 'submit', phrase('save', CAPITALIZE));

  $add_catering_div = html_div('', 'relative', "id='add_catering_div'");

  // $catering_form = $catering_form
  
  $output =  $table.$add_catering_button.$add_catering_div;
  return $output;
  
  // dump($inv_ids);
  // return $output;
}
  

// prints list of events for this reservation
function reservation_events($resObj, $options=NULL) {
  global $u_level;
  $res_id = $resObj->res_id;
  $add_event_button = ($u_level >= USER_LEVEL_STAFF) ? html_social_button($add_event_link, phrase('add', CAPITALIZE), array('class' => 'green_back', 'js' => 'id="add_event_button" onclick="load_event_form(\''.$res_id.'\');return false"')) : '';

  if(!$resObj) return '';
  $can_edit = $options['can_edit'];
  $can_edit = 1; // todo, remove

  $eventObjs = find_objects('event', array('res_id' => $resObj->res_id));
  //if(!$eventObjs) return $add_event_button;
  
  if($eventObjs) {
    $rows[] = phrase(array("what", "who", "when", "from", "to", "notes", "status", "actions"), CAPITALIZE);
    
    $status_str = $type_name = $contractor_name = '';
    $event_fields = array('res_id', 'contractor_id', 'start_time', 'end_time', 'start_address', 'end_address', 'notes', 'actions');
    
    foreach($eventObjs as $eventObj) {
      $event_id = $eventObj->id;
      $buttons = $can_edit ? html_div(recordset_buttons_inline('event', $eventObj->id, $event_fields, array('reload' => 1)), 'ie_edit_link') : '';
      if($contractor_id = $eventObj->contractor_id) {
        $contractorObj = get_object('user', $eventObj->contractor_id);
        $contractor_name = $eventObj->contractor_name ?: get_name_string($contractorObj);
        $contractor_phone = $contractorObj->cell_phone;
        $contractor_email = $contractorObj->email_address;
      } else {
        $contractor_name = $eventObj->contractor_name;
      }
      if($status = $eventObj->status) {
        $st = get_object('event_status', $status);
        $status_str = html_span($st->name, 'bold', "style='color:#$st->color'");
      }
      
      $type = $eventObj->type;
      //$time_string = $resObj->hourly ? print_time_range($eventObj->start_time, $eventObj->end_time) : $eventObj->start_time;
      $time_string = print_time_range($eventObj->start_time, $eventObj->end_time);
      $actions = $can_edit ? html_link('#', phrase('edit', CAPITALIZE), array('js' => 'onclick="load_event_form(\''.$res_id.'\', \''.$event_id.'\');return false"')) : '';
      $type_name = $type ? get_object('event_type', $type, 'name') : "";
      $rows[] = array($type_name ?: $type, $contractor_name, $time_string, $eventObj->start_address, $eventObj->end_address, $eventObj->notes, $status_str, $actions);      
    }
    $table = html_table($rows, array('class' => 'web20'));
  }
  

  //if($errors)   $add_event_form .= print_errors($errors, false);
  //if($messages) $add_event_form .= print_messages($messages, false);
  $event_form_div = html_div('', 'relative', "id='add_event_div'");

  return html_div($table.$add_event_button.$event_form_div, '', "id='reservation_events_div'");
}

function reservation_event_form($resObj, $eventObj=NULL) {
  $event_id = $eventObj->id;
  $form_name = 'add_event_form';
  $ajax_url = '/afr/home/app_data.php?oper=add_event';
  $ajax_handler = 'load_events';

  $hidden_fields = array('res_id' => $resObj->res_id, 'event_id' => $eventObj->id);

  $add_event_fields = event_fields($resObj, $eventObj);  
  $add_event_form = html_fieldset($add_event_fields, 'fieldset_top', array('translate' => true));
  $add_event_form .= array2form($hidden_fields);
  $submit = form_input('add_event', 'submit', phrase('save', CAPITALIZE));
  $errors = html_div('', '', "id='".$form_name."_errors'");
  $messages = html_div('', '', "id='".$form_name."_messages'");

  $action = $event_id ? 'edit' : 'add';

  $title = html_h4(phrase($action, CAPITALIZE)." ".phrase('event')." $event_id");
  $output .= html_form($title.$errors.$messages.$add_event_form.$submit, $form_name, '', 'post', 'border pad round margin_top').html_close_icon();
  $output .= html_script_inline('
    var form_name = "'.$form_name.'";
    var ajax_url  =  "'.$ajax_url.'";
    var handler   =  "'.$ajax_handler.'";
    var referer   = "'.$referer.'";
    var $form = $("#" + form_name); 
    $(".input").keypress(function(e) {
       if(e.which == 13) {
           jQuery(this).blur();
           jQuery("#submit_form").focus().click();
       }
    });
     $("#submit_form").click(function() {
       $form.submit();
       return false;
     });
     $form.validate({ 
      ignore: ":hidden",
      rules: { 
        pickup_time: { 
          required: true, 
        }, 
        delivery_time: { 
          required: true, 
        }, 
      }, 
      submitHandler: function(form) {
        //alert("submit");
        ajaxSubmit({form: form_name, url: ajax_url, type: "post", "handler": handler});
      }
    });
  ');  

  return $output;
}

function event_fields($resObj, $eventObj=NULL, $options=NULL) {
  $form_name = 'add_event_form';
  if($ptype = $eventObj->property_type ?: $resObj->property_type) {
    global $propertyTypeId;
    $ptype_id = $propertyTypeId[$ptype]; 
  }
  
  // $fields[phrase('type', CAPITALIZE)] = object_select('contractor_type', 'contractor_type');  

  $fields[phrase('what', CAPITALIZE)] = form_dynamic_select('/admin/form_dynamic_select.php', 'contractor_id', 'event_type', array('value' => $eventObj->type, 'type' => 'event_type', 'search' => array('property_types' => "*$ptype_id*"), 'sort' => 'id', 'form_id' => $form_name, 'target_first' => ' ', 'hide_empty_target' => true, 'first' => phrase('select', CAPITALIZE).' '.phrase('staff')));

  $fields[phrase('who', CAPITALIZE)] = object_select('contractor_id', 'user', array('search' => user_role_search(USER_ROLE_CONTRACTOR), 'value' => $eventObj->contractor_id));
  $fields[phrase('status', CAPITALIZE)] = object_select('status', 'event_status', array('id' => array('<', EVENT_STATUS_COMPLETE), 'value' => $eventObj->status ?: EVENT_STATUS_PROPOSED));

  $fields[phrase('start_address', CAPITALIZE)] = object_select('start_landmark_id', 'landmark', array('value' => $eventObj->start_landmark_id ?: $resObj->pickup_location, 'search' => array('type' => LANDMARK_TYPE_BOARDING)));
  $fields[phrase('start_time', CAPITALIZE)] = timepicker('start_time', array(),  $eventObj->start_time ?: $resObj->checkin);
  $fields[phrase('end_address', CAPITALIZE)] = object_select('end_landmark_id', 'landmark', array('value' => $eventObj->end_landmark_id ?: $resObj->dropoff_location, 'search' => array('type' => LANDMARK_TYPE_BOARDING)));
  $fields[phrase('end_time', CAPITALIZE)] = timepicker('end_time', array(), $eventObj->end_time ?: $resObj->checkout);
  
  $fields[phrase('notes', CAPITALIZE)] = form_input('event_notes', 'text', $eventObj->notes, array('size' => 60, 'max' => 255)); 

  return $fields;  
}

function catering_form($res_id, $lineitem_id, $supplier_id) {          
  $form_name = 'add_catering_form';   
  $referer = public_action_link($res_id, 'reservation');
  $ajax_url = '/afr/home/app_data.php?oper=add_catering';
  $ajax_handler = 'load_events';

  $resObj = get_object('reservation', $res_id);
  $liObj = get_object('inv_lineitem', $lineitem_id);
  $supplierObj = get_object('supplier', $supplier_id);
  
  $hidden_fields = array('catering_res_id' => $res_id, 'catering_lineitem_id' => $lineitem_id, 'catering_supplier_id' => $supplier_id);

  $errors = html_div('', '', "id='".$form_name."_errors'");
  $messages = html_div('', '', "id='".$form_name."_messages'");
  //$errors = html_error_box('', array('id' => $form_name."_errors"));
  //$messages = html_message_box('', array('id' => $form_name."_messages"));
  
  $add_catering_fields = catering_fields($resObj, $liObj, $supplierObj);  

  //if($errors)   $add_catering_form .= print_errors($errors, false);
  //if($messages) $add_catering_form .= print_messages($messages, false);
  $add_catering_form .= $errors.$messages;
  $add_catering_form .= html_fieldset($add_catering_fields, 'fieldset_top', array('translate' => true));
  $add_catering_form .= array2form($hidden_fields);
  $add_catering_form .= form_input('add_event', 'submit', phrase('save', CAPITALIZE));

  $output = html_form($add_catering_form.$submit, $form_name, '', 'post', 'border pad round margin_top').html_close_icon();

  $output .= html_script_inline('
    var form_name = "'.$form_name.'";
    var ajax_url  =  "'.$ajax_url.'";
    var handler   =  "'.$ajax_handler.'";
    var referer   = "'.$referer.'";
    var $form = $("#" + form_name); 
    $(".input").keypress(function(e) {
       if(e.which == 13) {
           jQuery(this).blur();
           jQuery("#submit_form").focus().click();
       }
    });
     $("#submit_form").click(function() {
       $form.submit();
       return false;
     });
     $form.validate({ 
      ignore: ":hidden",
      rules: { 
        pickup_time: { 
          required: true, 
        }, 
        delivery_time: { 
          required: true, 
        }, 
      }, 
      submitHandler: function(form) {
        ajaxSubmit({form: form_name, url: ajax_url, type: "post", "handler": handler});
      }
    });
  ');  

  return $output;  
}


function catering_fields($resObj, $liObj, $supplierObj, $options=NULL) {
  $form_name = 'add_event_form';
  if($ptype = $eventObj->property_type ?: $resObj->property_type) {
    global $propertyTypeId;
    $ptype_id = $propertyTypeId[$ptype]; 
  }
  
  $product_id = $liObj->product_id;
  $product_name = get_object('product', $product_id, 'name');
  
  $contractor_type = CONTRACTOR_TYPE_CAPTAIN;

  // $fields[phrase('type', CAPITALIZE)] = object_select('contractor_type', 'contractor_type');  

  $fields[phrase('what', CAPITALIZE)] = $product_name;
  $fields[phrase('supplier', CAPITALIZE)] = $supplierObj->organization;

  $fields[phrase('status', CAPITALIZE)] = object_select('catering_status', 'event_status', array('id' => array('<', EVENT_STATUS_COMPLETE), 'value' => $obj->status ?: EVENT_STATUS_PROPOSED));
                       
  if($delivery = $supplierObj->delivery) {    
    $fields[phrase('delivery', CAPITALIZE).'?'] = yes_no_chooser('catering_delivery', "", "", "", "catering_delivery_update();");           
  } else {
    $fields[phrase('delivery', CAPITALIZE).'?'] = phrase('no');
  }

  $supplier_address = implode(', ', array_filter(array($supplierObj->address1, $supplierObj->address2)));
  unset($supplierObj->country);
  $supplier_address = get_address_string($supplierObj, ',');
  $fields[phrase('pickup_address', CAPITALIZE)] = form_input('catering_pickup_address', 'text', $supplier_address);
  $fields[phrase('pickup_time', CAPITALIZE)] = timepicker('catering_pickup_time', array(),  $obj->start_time ?: $resObj->checkin);
  if($delivery) $fields[phrase('delivery_address', CAPITALIZE)] = object_select('catering_delivery_landmark_id', 'landmark', array('value' => $obj->start_landmark_id ?: $resObj->pickup_location, 'search' => array('type' => LANDMARK_TYPE_BOARDING)));
  $fields[phrase('delivery_time', CAPITALIZE)] = timepicker('catering_delivery_time', array(),  $obj->start_time ?: $resObj->checkin);
  $fields[phrase('who', CAPITALIZE)] = object_select('catering_contractor_id', 'user', array('search' => user_role_search(USER_ROLE_CONTRACTOR), 'value' => $obj->contractor_id));
            
  //if($delivery) {
  //  // delivery form  
  //  $delivery_fields[phrase('delivery_address', CAPITALIZE)] = object_select('start_landmark_id', 'landmark', array('value' => $obj->start_landmark_id ?: $resObj->pickup_location, 'search' => array('type' => LANDMARK_TYPE_BOARDING)));
  //  $delivery_fields[phrase('delivery_time', CAPITALIZE)] = timepicker('start_time', array(),  $obj->start_time ?: $resObj->checkin);
  //} else {  
  //  // pickup form  
  //}
  
  //$fields[phrase('end_address', CAPITALIZE)] = object_select('end_landmark_id', 'landmark', array('value' => $obj->end_landmark_id ?: $resObj->dropoff_location, 'search' => array('type' => LANDMARK_TYPE_BOARDING)));
  //$fields[phrase('end_time', CAPITALIZE)] = timepicker('end_time', array(), $obj->start_time ?: $resObj->checkout);
  
  $fields[phrase('notes', CAPITALIZE)] = form_input('catering_notes', 'text', $obj->contractor_notes, array('size' => 60, 'max' => 255)); 

  return $fields;  
}

function boarding_details($obj_or_vars, $locs=NULL) {
  $resObj = is_object($obj_or_vars) ? $obj_or_vars : (object) $obj_or_vars;
  $fields=array();  
  if($resObj->property_type != 'boat') return array();  
  $fields[phrase('boarding_location', CAPITALIZE)] = $resObj->pickup_location ? get_object('landmark', $resObj->pickup_location, 'name') : 'Home Dock';
  $fields[phrase('unboarding_location', CAPITALIZE)] = $resObj->dropoff_location ? get_object('landmark', $resObj->dropoff_location, 'name') : 'Home Dock';
  return $fields;
}


function boarding_fields($obj_or_vars, $locs=NULL, $popup_map=true) {
  global $__header;
  
  $resObj = is_object($obj_or_vars) ? $obj_or_vars : (object) $obj_or_vars;
  $apt_id = $resObj->apt_id;
  $ptype  = $resObj->property_type;
  if($apt_id && $ptype) $loc_id = get_object($ptype, $apt_id, 'location_id'); // home location

  foreach($locs as $loc) $rows[$loc->id] = $loc->name;  
  $boarding_chooser = select_from_array('pickup_location', $rows, $resObj->pickup_location ?: $loc_id);
  $unboarding_chooser = select_from_array('dropoff_location', $rows, $resObj->dropoff_location ?: $loc_id);

  if($popup_map) {
    $boarding_map_link = html_link('#', phrase('map', CAPITALIZE), phrase('view_map', CAPITALIZE), array('id' => 'show_boarding_map'));  
    $unboarding_map_link = html_link('#', phrase('map', CAPITALIZE), phrase('view_map', CAPITALIZE), array('id' => 'show_unboarding_map'));  
  } else {
    //$map_link = site_pagetype_link($_SESSION['siteObj'], PAGE_MAP); // link to status page
    $map_link = get_page_link(PAGE_MAP);
    $search['type'] = LANDMARK_TYPE_BOARDING;    
    $search['active'] = 1;
    $map_link .= '?search='.urlencode(json_encode($search));
    $boarding_map_link = html_link($map_link, phrase('map', CAPITALIZE), array('target' => '_new'));
    $unboarding_map_link = '';
  }
  
  $boarding_fields[phrase('boarding_location', CAPITALIZE)] = $boarding_chooser.' '.$boarding_map_link;  
  $boarding_fields[phrase('unboarding_location', CAPITALIZE)] = $unboarding_chooser.' '.$unboarding_map_link;;

  return $boarding_fields;
}

function boarding_maps($locs) {
  global $__header;
  $map_id = 'boarding_map';
  $boarding_map = gmap3(array('markers' => $locs, 'obj_type' => 'landmark', 'id' => $map_id, 'init' => 0));
  $map_cont = html_div($boarding_map, 'hidden', "id='map_container'");
  $__header['onload'] .= '$("#show_boarding_map, #show_unboarding_map").click(function() {
    boarding_map_init($(this).attr("id"));return false;
  });
';
  return $map_cont.$map2_cont;
}

function boarding_locs($resObj) {
  $apt_id = $resObj->apt_id;
  $ptype  = $resObj->property_type;
  if($apt_id && $ptype) $aptObj = get_object($ptype, $apt_id);
  if(!$aptObj->location_id && $aptObj->location_accuracy) { // no home location set for this boat, add address manually    
    $homeLoc = obj2loc($ptype, $aptObj);                  
    $homeLoc->name = "$aptObj->address_street $aptObj->address_number".' ('.phrase('home_dock', CAPITALIZE).')';
    $locs[] = $homeLoc;
  }

  $city_id = $resObj->city_id;
  if(!$city_id) return $locs;
  $landmarks = find_objects('landmark', array('active' => 1, 'city_id' => $city_id, 'type' => LANDMARK_TYPE_BOARDING));
  foreach($landmarks as $landmark) {
    if($landmark->id == $aptObj->location_id) $landmark->name .= ' ('.phrase('home_dock', CAPITALIZE).')';
    $locs[] = obj2loc('landmark', $landmark);  
  }
  return $locs;
}


function print_boarding_form($obj_or_vars) {
  $resObj = is_object($obj_or_vars) ? $obj_or_vars : (object) $obj_or_vars;
  $locs = boarding_locs($resObj);
  $maps = boarding_maps($locs);
  $boarding_fields = boarding_fields($resObj, $locs);
  return $maps.html_fieldset($boarding_fields, 'fieldset_top');  
}

function print_arrival_details_form($obj_or_vars) {
  $resObj = is_object($obj_or_vars) ? $obj_or_vars : (object) $obj_or_vars;
  $arrival_fields = arrival_fields($resObj);
  return html_fieldset($arrival_fields, 'fieldset_top');
}

function pickup_ok_fee($apt_id) {
  $apt = get_object('shortstay', $apt_id);
  $airport_pickup_ok = strpos($apt->facilities, '110') !== FALSE;
  $airport_pickup_fee = $apt->fee_pickup;
  return array($airport_pickup_ok, $airport_pickup_fee);
}

// return assoc. array of arrival fields (edit)
function arrival_fields($obj_or_vars) {

  $resObj = is_object($obj_or_vars) ? $obj_or_vars : (object) $obj_or_vars;
  
  $show_tooltip = true;   
  $airport_pickup_ok = false;
  
  $apt_id = $resObj->apt_id;
  $ptype  = $resObj->property_type;
  
  
  if($resObj->admincal) {
    $airport_pickup_ok = true;
    $show_tooltip = false;   
  } else if($apt_id) { // todo: assuming shortstay here
    list($airport_pickup_ok, $airport_pickup_fee) = pickup_ok_fee($apt_id);
    $debug = "pickup ok=".$airport_pickup_ok." fee=".$airport_pickup_fee.html_break();
    $onchange = 'update_pickup("'.$resObj->checkin.'");';
  }

  if($airport_pickup_ok) {
    $pickup_text = phrase('text_airport_pickup').html_break().phrase('pickup_fee', CAPITALIZE).': '.formatFloat($airport_pickup_fee, 1, get_currency_symbol($resObj->currency, true));
    if($airport_pickup_fee > 0) $pickup_text .= ' ('.phrase('will_be_added_to_the_total', CAPITALIZE).')';
    if($resObj->checkin) {
      $deadline = sql_add_days($resObj->checkin, -1); // 24 hours before checkin is last chance to set pickup
      if($too_late = now() > $deadline) $pickup_text = phrase('too_late_for_airport_pickup_changes', CAPITALIZE);
    }
  }
  
  $tip1 = html_tooltip(phrase('text_flight'));
  $tip2 = html_tooltip(phrase('text_arrival_time'));
  $tip3 = html_tooltip(phrase('text_checkin_time'));

  $arrival_method_array = array('air' => phrase('air', CAPITALIZE), 'train' => phrase('train', CAPITALIZE), 'car' => phrase('car', CAPITALIZE), 'bus' => phrase('bus', CAPITALIZE), 'other' => phrase('other', CAPITALIZE));
  $arrival_fields[phrase('arrive_by', CAPITALIZE)] = select_from_array('arrival_method', $arrival_method_array, $resObj->arrival_method, array('first' => '??'));
  $arrival_fields[phrase('flight', CAPITALIZE)." ".$tip1] = form_input('arrival_number', 'text', $resObj->arrival_number, array('size' => 10, 'max' => 10));
  $arrival_fields[phrase('arrival', CAPITALIZE)." ".$tip2] = timepicker('arrival_time', '', $resObj->arrival_time);
  $arrival_fields[phrase('checkin', CAPITALIZE)." ".$tip3] = timepicker('arrival_property', '', $resObj->arrival_property);
  
  $arrival_fields[phrase('arrival_notes', CAPITALIZE)] = form_input('arrival_notes', 'text', $resObj->arrival_notes, array('size' => 50, 'max' => 100));
  if($airport_pickup_ok) {
    $pickup_check = $resObj->airport_pickup ? true : false;
    //ajax_url = "/home/ajax/calfilter.php?ac=load_dialog&obj_type=" + g_obj_type + "&apt_id=" + apt.id + "&property_type=" + g_property_type + "&checkin=" + escape(checkin) + "&checkout=" + escape(checkout)
    //$dialog.load(ajax_url, function (responseText) {
    // if($airport_pickup_fee > 0) $js = 'onchange=\'$("#rate_box").load("/home/ajax/request_helpers.php");\''; // update fees and total
    $js = "onchange='update_pickup_fee();'";
    $arrival_fields[html_tooltip($pickup_text, '', array('anchor' => phrase('airport_pickup', CAPITALIZE)))] = form_checkbox('airport_pickup', $pickup_check, array('js' => $js));
  }
  return $arrival_fields;                                                 
}


function print_property_watchlist($plist, $show_estate_agents=false, $show_handling_agent=false) {
  $rows=array();
  $header_row = array(phrase('date', CAPITALIZE), phrase('city', CAPITALIZE), phrase('property_type', CAPITALIZE), phrase('property', CAPITALIZE), phrase('price', CAPITALIZE));
  if($show_estate_agents)   $header_row[] = phrase('estate_agent', CAPITALIZE);
  if($show_handling_agents) $header_row[] = phrase('handling_agent', CAPITALIZE);
  $rows[]=  $header_row;
  
  foreach($plist as $listObj) {
    $date   = sql2human($listObj->timestamp);
    $city   = geoname($listObj->city_id);
    $type   = phrase($listObj->property_type, CAPITALIZE);
    $link   = get_link($listObj->property_type, $listObj->apt_id, false);
    $name   = html_link($link, $listObj->apartment_name);
    $rate   = formatFloat($listObj->apt_price, 1, get_currency_symbol($listObj->apt_currency, true));
    $rooms  = $listObj->num_rooms;
    //$row    = array($date, $city, $type, $name, $rate, $rooms);
    $row    = array($date, $city, $type, $name, $rate);
    $estate_agent_id = $listObj->estate_agent_id;
    if($show_estate_agents && $estate_agent_id) {
      $agentObj = get_object('estate_agent', $estate_agent_id);
      $agent_name = $agentObj->address_url ? html_link($agentObj->address_url , $agentObj->name) : $agentObj->name;
      $agent_text = print_estate_agent_details($agentObj);
      //$agentObj->logo ? 500 : 200
      $agent_help = print_info_popup('estate_agent_info_'.$estate_agent_id, $agent_text, 0, 0, false);
      
      $row[] = "$agent_name $agent_help";
    }
    if($show_handling_agents) {
      $row[] = $handling_agent;
    }
    $rows[] = $row;
  }
  return html_table($rows);
}

// returns details about a request (either enquiry or reservation)
// used on-screen and in emails  (view)
function request_details($request_type, $obj, $user_level=0) {
 switch($request_type) {
  case 'reservation':
    return reservation_details($obj, $user_level);
    break;
  case 'enquiry':
    return enquiry_details($obj, $user_level);
    break;
  case 'payment_order':
    return order_details($obj, $user_level);
    break;
  default:
    return "";
    die("request_details: missing or unknown request type '$requst_type'");
    break;
  }
}

// returns associate array of enquiry information (view)
function invoice_details($invObj_or_id, $user_level=0) {
  $invObj = any2obj("invoice", $invObj_or_id);  
  if($invObj->legal_entity_id) $fields[phrase('billed_by', CAPITALIZE)]     = get_object('legal_entity', $invObj->legal_entity_id, 'name');
  if($invObj->invoice_id) {
    $fields[phrase('invoice', CAPITALIZE).' '.phrase('id', UPPERCASE)]     = $invObj->invoice_id;
    $fields[phrase('voucher', CAPITALIZE).' '.phrase('id', UPPERCASE)]     = $invObj->id;
    $type_str = 'invoice';
  } else {
    $type_str = $invObj->invoice_id_required ? 'voucher' : 'invoice';
    $fields[phrase($type_str, CAPITALIZE).' '.phrase('id', UPPERCASE)]     = $invObj->id;
  }
  $fields[phrase('type', CAPITALIZE)]     = get_object('inv_type', $invObj->type, 'name').' '.phrase($type_str);
  $fields[phrase('status', CAPITALIZE)]     = get_object('inv_status', $invObj->status, 'name').' '.phrase('since').' '.sql2human($invObj->status_change_timestamp);
  if($invObj->invoice_date) $fields[phrase('invoice_date', CAPITALIZE)] = sql2human($invObj->invoice_date);
  if($invObj->due_date) $fields[phrase('due_date', CAPITALIZE)] = sql2human($invObj->due_date);
  if($invObj->sent_date) $fields[phrase('sent_date', CAPITALIZE)] = sql2human($invObj->sent_date);
  $fields[phrase('start_date', CAPITALIZE)] = sql2human($invObj->start_date);
  $fields[phrase('end_date', CAPITALIZE)]   = sql2human($invObj->end_date);
  if($invObj->concerning) $fields[phrase('concerning', CAPITALIZE)]   = $invObj->concerning;
  if($invObj->notes) $fields[phrase('comments', CAPITALIZE)]   = $invObj->notes;
  if($user_level >= USER_LEVEL_STAFF) {
    if($invObj->export_timestamp) $fields[phrase('export_timestamp', CAPITALIZE)] = sql2human($invObj->export_timestamp).' to '.$invObj->export_gateway;
  }
  return $fields;
} 

// returns associate array of enquiry information (view)
function enquiry_details($enqObj, $user_level=0) {
  $fields=array();
  $type = $enqObj->property_type;
  $price = formatFloat($enqObj->apt_price, 1, get_currency_symbol($enqObj->apt_currency, true));
  if($type == 'longstay') $price .= "/".phrase('month', CAPITALIZE);
  $fields[phrase('enquiry_id', CAPITALIZE)]     = $enqObj->id;
  if($user_level == USER_LEVEL_GUEST)
    $fields[phrase('booking_pin', CAPITALIZE)]  = $enqObj->booking_pin;
  $fields[phrase('property_type', CAPITALIZE)]  = phrase($type, CAPITALIZE);
  $fields[phrase('property', CAPITALIZE)]       = $enqObj->apartment_name;
  $fields[phrase('price', CAPITALIZE)]          = $price;
  return $fields;
}

// returns associate array of order information (view)
function order_details($orderObj, $user_level=0) {
  $currency = $orderObj->currency;
  $cur = get_currency_symbol($currency, true);

  if($org_id = $orderObj->org_id) $fields[phrase('company', CAPITALIZE)] = get_object('org', $org_id, 'org_name');
    $fields[phrase('reservation', CAPITALIZE).' ID'] = $orderObj->res_id;
  if($user_level == USER_LEVEL_GUEST) $fields[phrase('booking_pin', CAPITALIZE)]  = $orderObj->booking_pin;

  $fields[phrase('status', CAPITALIZE)]  = get_object('order_status', $orderObj->status, 'name');
  $fields[phrase('total', CAPITALIZE)]   = formatFloat($orderObj->total, 1, $cur);
  $fields[phrase('paid', CAPITALIZE)]    = formatFloat($orderObj->paid, 1, $cur);
  $fields[phrase('balance', CAPITALIZE)] = formatFloat($orderObj->balance, 1, $cur);
  return $fields;

}

// returns associate array of reservation information (view)
function reservation_details($resObj, $user_level=0) {
  $show_weekday = false;
  //dump($resObj);
  $show_time = $resObj->num_hours > 0 || ($resObj->start_hour && $resObj->end_hour);
  $checkin = sql2human($resObj->checkin, array('show_weekday' => $show_weekday, 'show_time' => $show_time));
  $checkout = sql2human($resObj->checkout, array('show_weekday' => $show_weekday, 'show_time' => $show_time));

  $fields=array();
  if($org_id = $resObj->org_id) $fields[phrase('company', CAPITALIZE)] = get_object('org', $org_id, 'org_name');
  $fields[phrase('reservation', CAPITALIZE).' ID'] = $resObj->res_id;
  if($user_level == USER_LEVEL_GUEST) 
    $fields[phrase('booking_pin', CAPITALIZE)]  = $resObj->booking_pin;
  $ptype_name = property_type_name($resObj->property_type, 1, CAPITALIZE);  
  $fields[$ptype_name] = $resObj->apartment_name;
  $fields[phrase('check-in', CAPITALIZE)]       = $checkin;
  $fields[phrase('check-out', CAPITALIZE)]      = $checkout;

  if($resObj->pickup_location) $fields[phrase('pickup_location', CAPITALIZE)] = get_object('landmark', $resObj->pickup_location, 'name');
  if($resObj->dropoff_location) $fields[phrase('dropoff_location', CAPITALIZE)] = get_object('landmark', $resObj->dropoff_location, 'name');

  if($resObj->num_hours > 0) {
    $fields[phrase('num_hours', CAPITALIZE)]       = $resObj->num_hours;
  } else {
    $fields[phrase('num_days', CAPITALIZE)]       = $resObj->num_days;
  }
  if($resObj->num_guests > 0) $fields[phrase('num_guests', CAPITALIZE)]     = $resObj->num_guests;
  if($user_level >= USER_LEVEL_OWNER && trim($resObj->alert)) $fields[phrase('NB!', CAPITALIZE)]     = $resObj->alert;

  return $fields;
}

// returns fields used for printing/mailing event details
function event_details($obj) {
 
  // $fields[phrase('id', CAPITALIZE)]         = $obj->id;
  $fields[phrase('type', CAPITALIZE)]       = get_object('event_type', $obj->type, 'name');
  // $fields[phrase('status', CAPITALIZE)]     = get_object('event_status', $obj->status, 'name'). " (".sql2human($obj->status_change_timestamp).")";
  // if($obj->status_change_timestamp > 1) $status_str .= " (".sql2human($enqObj->status_change_timestamp).")";

  $fields[phrase('start_time', CAPITALIZE)] = sql2human($obj->start_time, array('show_weekday' => true, 'show_time' => true));
  // $fields[phrase('end_time', CAPITALIZE)]   = sql2human($obj->end_time, true, true);

  $fields[phrase('address', CAPITALIZE)]     = $obj->start_address;
  $fields[phrase('to_address', CAPITALIZE)] = $obj->end_address;

  $fields[phrase('reservation', CAPITALIZE)]  = $obj->res_id;
  if($obj->apt_id) $fields[phrase('property', CAPITALIZE)] = get_object('property', $obj->apt_id, 'name');

  $fields[phrase('customer_name', CAPITALIZE)]   = $obj->customer_name;
  $fields[phrase('customer_count', CAPITALIZE)]  = $obj->customer_count;
  $fields[phrase('customer_notes', CAPITALIZE)]  = $obj->customer_notes;

  $fields[phrase('contractor_name', CAPITALIZE)]   = $obj->contractor_name;
  $fields[phrase('contractor_notes', CAPITALIZE)]  = $obj->contractor_notes;

  $fields[phrase('alert', CAPITALIZE)]   = $obj->alert;
  $fields[phrase('notes', CAPITALIZE)]   = $obj->notes;
  
  return array_filter($fields);
}

// returns associate array of arrival information (view)
function arrival_details($resObj_or_id, $user_level=0) {
  $resObj = any2obj('reservation', $resObj_or_id); 
  $fields=array();  
  if($resObj->property_type == 'shortstay') list($airport_pickup_ok, $airport_pickup_fee) = pickup_ok_fee($resObj->apt_id);  
  $arrival_str = implode(' - ', array_filter(array(phrase($resObj->arrival_method, CAPITALIZE), $resObj->arrival_number, strip_sec($resObj->arrival_time), $resObj->arrival_notes)));
  if($arrival_str) $fields[phrase('arrival_information', CAPITALIZE)] = $arrival_str;
  $fields[phrase('checkin_time', CAPITALIZE)] = $resObj->arrival_property ? strip_sec($resObj->arrival_property) : "???";
  
  // if($airport_pickup_ok) $fields[phrase('airport_pickup', CAPITALIZE)]= $resObj->airport_pickup ? phrase('yes', CAPITALIZE) : phrase('no', CAPITALIZE);
  return $fields;
}

// hotel agents use this field for "room" 
function address_title_field($resObj) {
  $agent_type = $resObj->agent_id ? get_object('agent', $resObj->agent_id, 'type') : '';
  return $agent_type == AGENT_TYPE_HOTEL ? phrase('room', CAPITALIZE) : phrase('title', CAPITALIZE);
}

// returns associate array of address information (view)
function address_details($addObj, $show_contact=false) {
  $fields=array();
  if(is_array($addObj)) $addObj = array2obj($addObj);
  
  if($name = get_name_string($addObj)) $fields[phrase('name', CAPITALIZE)] = $name;
  if($title = $addObj->title) $fields[address_title_field($addObj)] = $title;
  if($show_contact) {
    if($address = get_address_string($addObj, ', ')) $fields[phrase('address', CAPITALIZE)] = $address;
    if($addObj->email_address) $fields[phrase('email', CAPITALIZE)] = html_link('mailto:'.$addObj->email_address, $addObj->email_address);
    if($addObj->home_phone)    $fields[phrase('home_phone', CAPITALIZE)] = $addObj->home_phone;
    if($addObj->work_phone)    $fields[phrase('work_phone', CAPITALIZE)] = $addObj->work_phone;
    if($addObj->cell_phone)    $fields[phrase('cell_phone', CAPITALIZE)] = $addObj->cell_phone;
  }
  return $fields;
}

function property_address($param1, $street_only=false) {
  $address=array();
  $street="";
  if($param1->address_street  ) $street.=$param1->address_street;
  if($param1->address_number  ) $street.=" $param1->address_number";
  if($param1->address_suffix  ) $street.="-$param1->address_suffix";
  if($street) $address[]=$street;
  if(!$street_only) {
    if($param1->address_postcode) $address[]=$param1->address_postcode;
    if($param1->address_city    ) $address[]=$param1->address_city;
    if($param1->address_country ) $address[]=$param1->address_country;
  }
  $address = implode(', ', array_filter($address));
  return $address;
}

function get_apartment_address($apt, $show_name=1, $show_country=1, $glue=", ") {
  $obj->name = $apt->address_name;
  $obj->attn = $apt->address_attention;

  $obj->street = $apt->address_street;
  $obj->number = $apt->address_number;
  $obj->suffix = $apt->address_suffix;
  $obj->line2 = $apt->address_line2;

  $obj->city = $apt->address_city;
  $obj->state = $apt->address_state;
  $obj->postcode = $apt->address_postcode;

  $obj->country = $apt->address_country;

  return get_address_string($obj, $glue);
}

// prints left round box with enquiry or reservation details
function request_sidebar($request_type, $obj, $user_level=USER_LEVEL_GUEST, $class='column_width') {
 if(!$width) $width = $_SESSION['left_column_width'];
 if(!$obj) return ''; //return "sidebar: no object";
 switch($request_type) {
    case 'reservation':
      return reservation_sidebar($obj, $user_level, $class);
      break;
    case 'enquiry':
      return enquiry_sidebar($obj, $user_level, $class);
      break;
    case 'block':
      return block_sidebar($obj, $user_level, $class);
      break;
    case 'payment_order':
      return order_sidebar($obj, $user_level, $class);
      break;
    case 'order':
      return order_sidebar($obj, $user_level, $class);
      break;
    case 'cart':
      return cart_sidebar($obj, $user_level, $class);
      break;
    case 'invoice':
      $parent_type = $obj->parent_type;
      $parent_id = $obj->parent_id;
      if($parent_type && $parent_id && ($obj = get_object($parent_type, $parent_id))) return  request_sidebar($parent_type, $obj, $user_level, $class);
      break;
  default:
    if($_SESSION['dev']) echo("request_sidebar: missing or unknown request type '$request_type'");
    return '';
    break;
  }    
}
                  
// prints left round box with enquiry details
function enquiry_sidebar($enqObj, $user_level=USER_LEVEL_GUEST, $class='column_width') {
  if(!$width) $width = $_SESSION['left_column_width'];
  $enq_id         = $enqObj->id;
  $property       = get_object($enqObj->property_type, $enqObj->apt_id);
  $property_name  = $property->name;
  $num_rooms      = $property->num_rooms;
  $ptype          = $property->type;

  $city_id        = $property->city_id;
  $city_name      = geoname($city_id);
  
  if($ptype && $property) {
    $property_text = property_print($_SESSION['siteObj'], $_SESSION['pageObj'], $property, array('show_links' => false));
  }
  
  $type = phrase('enquiry', CAPITALIZE);
  //$title = $enq_id ? "$type $enq_id" : phrase('new', CAPITALIZE). ' '.$type;
  $title = $enq_id ? "$type $enq_id" : phrase('your_request', CAPITALIZE);
  $request .= html_tooltip("<span class='heading2'>$property_name $stars </span>", $property_text).html_break()."$city_name".html_break(2);

  if($enqObj->status) {
    $statusObj = get_object('enq_status', $enqObj->status);
    $status_str = phrase($statusObj->name, CAPITALIZE);
    $status_help = tooltip('status_info', $statusObj->description);
    if($enqObj->status_change_timestamp > 1) $status_str .= " (".sql2human($enqObj->status_change_timestamp).")";
    $request .= phrase('status', CAPITALIZE).":".html_break()."<b>$status_str</b>$status_help".html_break(2);
  }

  list($searchObj, $filters) = get_filter_string(obj2array($enqObj));
  if($filters) $request .= phrase('your_search', CAPITALIZE).":".html_break()."<b>$filters</b>".html_break(2);
 
  return title_box($title, $request, $class);
}

function order_sidebar($orderObj, $user_level=USER_LEVEL_GUEST, $class='column_width') {
  $title = phrase('your_order', CAPITALIZE)." $orderObj->id";
  //$request .= html_tooltip("<span class='heading2'>$property_name $stars </span>", $property_text).html_break()."$city_name".html_break(2);
  //$orderObj = get_object('payment_order', $orderObj->id);
  //dump($orderObj);
  
  if($orderObj->status) {
    $statusObj = get_object('order_status', $orderObj->status);
    $status_str = phrase($statusObj->name, CAPITALIZE);
    $status_help = tooltip('status_info', $statusObj->description);
    if($orderObj->status_change_timestamp > 1) $status_str .= " (".sql2human($orderObj->status_change_timestamp).")";
    $output .= phrase('status', CAPITALIZE).":".html_break()."<b>$status_str</b>$status_help".html_break(2);
  }


  $cur_str    = get_currency_symbol($orderObj->currency, true); // HTML currency symbol
  //dump($orderObj);

  
  $rows = order_rows($siteObj, $orderObj);
  $output = html_table($rows, array('class' => 'accounting_table', 'header' => 1)).html_break();  
  
  // begin output 
  $lines = array();
  if($rate_type != 'total') { // show day rate
    $lines[] = array(phrase('total', CAPITALIZE).': ', print_rate($cur_str, $orderObj->total, $orderObj->discount));    // base rate + discounted rate if any
    $lines[] = array(phrase('paid', CAPITALIZE).': ', print_rate($cur_str, $orderObj->paid));    // base rate + discounted rate if any
    $lines[] = array(phrase('balance', CAPITALIZE).': ', print_rate($cur_str, $orderObj->balance));    // base rate + discounted rate if any
  }
  $table = html_table($lines, array('class' => 'accounting_table', 'header' => 0));  
  $rate_box = html_div($table, 'bold', "id='rate_box'");
  $output .= round_box($rate_box, 'box_dark_color');

  if($orderObj->parent_Type == 'cart') {
    $cart_link = site_page_link($_SESSION['siteObj'], PAGE_CART);
    $cart_button = html_social_button($cart_link, "&#x25C0 ".phrase('cart', CAPITALIZE), array('class' => 'green_back white caret-previous')); 
    $output .= html_break().$cart_button;
  }
  return title_box($title, $output, $class);
}

function cart_sidebar($cartObj, $user_level=USER_LEVEL_GUEST, $class='column_width') {
  $title = phrase('your_cart', CAPITALIZE)." $cartObj->id";
  $siteObj = $_SESSION['siteObj'];
  if(!$cartObj->items) $cartObj = get_cart($siteObj, $cartObj->id); // loads the cart rows
  $cur_str    = get_currency_symbol($cartObj->currency, true); // HTML currency symbol
  $items = $cartObj->items;
  $output = print_cart($siteObj, $cartObj, array('compact' => true));

  $cart_link = site_page_link($_SESSION['siteObj'], PAGE_CART);
  $cart_button = html_social_button($cart_link, "&#x25C0 ".phrase('cart', CAPITALIZE), array('class' => 'green_back white caret-previous')); 
  $output .= html_break().$cart_button;

  return title_box($title, $output, $class);
}

// prints left round box with enquiry details - links + choosers to change guests/dates/property
// todo: move round_box to caller, return array ?
function block_sidebar($blockObj, $user_level=USER_LEVEL_GUEST, $class='column_width') {
  $block_id = $blockObj->id;
  $resObjs = block_children($blockObj);

  //dump($resObjs);
  foreach($resObjs as $resObj) {
    $rid = $resObj->res_id;
    $aid = $resObj->apt_id;    
    $ar['rt'][$aid] = $resObj->rate_type;
    $ar['rb'][$aid] = $resObj->rate_base;
    $ar['in'][$aid] = $resObj->checkin;
    $ar['ou'][$aid] = $resObj->checkout;
    $ar['nd'][$aid] = $nd = $resObj->num_days;
    $ar['nh'][$aid] = $nh = $resObj->num_hours;
    $nd_total += $nd;
    $nh_total += $nh;
  }
  //dump($ar['rt']);
  $same_rate = array_same($ar['rt']) && array_same($ar['rb']);
  $same_count = array_same($ar['nd']) && array_same($ar['nh']);
  
  $type = "Block";
  $title = $block_id ? "$type $block_id" : phrase('your_request', CAPITALIZE);

  $apt_ids = explode(',', $blockObj->apt_list);
  $ptype = $blockObj->property_type;
  if(!$ptype) die("block_sidebar: missing property type on block");
  foreach($apt_ids as $apt_id) {
    $property = get_object($ptype, $apt_id);
    if($property->parent_id) {
      $apt_id = $property->parent_id;    
      $property = get_object($ptype, $property->parent_id);
    }      
    $apt_count[$apt_id] += 1;
    $apts[$apt_id] = $property;
  }
  foreach($apt_count as $apt_id => $apt_count) {
    $property = $apts[$apt_id];
    $property_name = $property->name;
    $property_text = property_print($_SESSION['siteObj'],  $_SESSION['pageObj'], $property, array('show_links' => false));
    $count_str = $apt_count > 1 ? "$apt_count X " : '';
    $length_str = $same_count ? "" : ", ".sidebar_length($ar['nd'][$apt_id], $ar['nh'][$apt_id]);
    $request .= html_tooltip(html_b($count_str.$property_name).$length_str." $stars  ", $property_text).html_break();
  }
  if($city_id = $blockObj->city_id) $city_name = geoname($city_id);
  $request .= $city_name.html_break(2);

  if($blockObj->status) {
    $statusObj = get_object('res_status', $blockObj->status);
    $status_str = $statusObj->name;
    $status_tip = tooltip('status_info', $statusObj->description);
    $status_str = phrase($statusObj->name, CAPITALIZE);
    if($blockObj->status_change_timestamp > 1) $status_str .= " (".sql2human($blockObj->status_change_timestamp).")";
    $request .= phrase('status', CAPITALIZE).":".html_break().html_b($status_str).$status_tip.html_break(2);
  }
  if($same_count) {
    $show_time = $nh > 0 ? true : false;
    $checkin_str = sql2human($resObj->checkin, array('show_weekday' => true, 'show_time' => $show_time));
    $checkout_str = sql2human($resObj->checkout, array('show_weekday' => true, 'show_time' => $show_time));
    $request .= phrase('checkin', CAPITALIZE).": ".html_break().html_b($checkin_str).html_break(2);
    $request .= phrase('checkout', CAPITALIZE).": ".html_break().html_b($checkout_str).html_break(2);
  }
  
  $num_apts = count($apt_ids);
  if($num_apts > 0) {
    $type_str = property_type_name($blockObj->property_type, $num_apts);
    $request .= html_b($num_apts).' '.$type_str.html_break();
  }
  $request .= $same_count ? sidebar_length($nd, $nh).html_break() : sidebar_length($nd_total, $nh_total)." ".phrase('total').html_break();
  
  $request .= html_break();
  $request .= print_rate_box($blockObj);
  
  return title_box($title, $request, $class);
}


function reservation_sidebar($resObj, $user_level=USER_LEVEL_GUEST, $class='column_width') {
  if(!$width) $width = $_SESSION['left_column_width'];
  //dump($resObj);
  $res_id = $resObj->res_id;
  $ptype  = $resObj->property_type;
  $apt_id = $resObj->apt_id;
  $num_hours = $resObj->num_hours;
  $num_days  = $resObj->num_days;
  $num_apts  = $resObj->block_count;

  $hourly = hourly_rate($ptype);
  $aptObj = get_object($resObj->property_type, $resObj->apt_id);
  $min_stay = $aptObj->min_stay;  

  $can_edit = true;
  if($event_id = $resObj->event_id) {
    $eventObj = get_object('event', $event_id);
    $is_ticket = $eventObj->type == EVENT_TYPE_TICKET;
    if($eventObj->name) $event_name = html_div($eventObj->name);
    //$can_edit = false;
  }    
  $change = $_GET['change'];
  if($num_apts || $resObj->dest_id) $can_edit = false; // todo: change
  
  //$direct_booking = $resObj->type == RES_TYPE_DIRECT;
  if($city_id = $resObj->city_id) $city_name = geoname($city_id);
  
  if($ptype && $apt_id) {
    $property = get_object($ptype, $apt_id);
    if($property->parent_id) $property = get_object($ptype, $property->parent_id);
    $property_name = $property->name;
    
    //$property_text = property_print($_SESSION['siteObj'],  $_SESSION['pageObj'], $property, array('show_links' => false, 'hide_description' => true));
    
  } else if($ptype) { // unknown apartment
    $type_str = property_type_name($resObj->property_type, 1);
    $property_name = "$type_str - $city_name";
    $property_name = $resObj->package_id ? get_object('product', $resObj->package_id, 'name') :  "???";
  }
  
  $type = $resObj->status >= STATUS_RESERVED ? phrase('reservation', CAPITALIZE) : phrase('request', CAPITALIZE);
  //$title = $res_id ? "$type $res_id" : phrase('new', CAPITALIZE). ' '.$type;
  $title = $res_id ? "$type $res_id" : phrase('your_request', CAPITALIZE);
  
  $pageObj = $_SESSION['pageObj'];
  
  if($change) {
    $hidden_fields = $pageObj->page_type == PAGE_STATUS ? 
      public_action_vars($resObj, 'reservation', $user_level, 'change') : 
      array('rid' => $resObj->res_id, 'pin' => myhash($resObj->booking_pin));
    $form = array2form($hidden_fields); // hidden fields
    $submit = form_input('change', 'submit', phrase('ok', UPPERCASE));
  }

  // property
  $property_count = $_SESSION['object_count'][$type];
  if($can_edit && $property_count > 1) $edit_link = res_change_link($resObj, 'property', $user_level);
  if($change == 'property') {
    $all_apt_list = find_objects($ptype, $_SESSION['property_filters'][$ptype], array('fields' => 'id'));
    if($apt_ids = available_apartments($resObj->checkin, $resObj->checkout, $ptype, $resObj->city_id, $all_apt_list, STATUS_PROPOSED, $resObj->res_id, $resObj->num_guests)) { // show apartments that ARE available
      //foreach($apt_ids as $apt_id) $chooser_rows[$apt_id] = get_object('property', $apt_id, 'name');
      list($chooser_rows, $tip) = print_available_apartments($resObj, $apt_ids, $changes, 'inline');
      $property_info = modaltip('property_info', $tip, phrase('available_properties', CAPITALIZE)); // , array('width' => 500 )
      $apt_chooser = select_from_array('apt_id', $chooser_rows, $resObj->apt_id, "");
      $request .= html_form($form.$apt_chooser.$property_info.html_break().$submit.' '.$edit_link).html_break(2);
    } else {
      $request .= "Oops: no avail. apts.".html_break(2);
    }
  } else {
    $siteObj = $_SESSION['siteObj'];
    $property_link = property_link($siteObj, $apt_id);
    //$request .= $event_name.html_tooltip(html_b($property_name)." $stars  ", $property_text).$edit_link.html_break().$city_name.html_break(2);
    $request .= $event_name.html_link($property_link, $property_name)." $stars  ".$edit_link.html_break().$city_name.html_break(2);
  }
      
  if($resObj->status && $resObj->status != STATUS_DRAFT) {
    $statusObj = get_object('res_status', $resObj->status);
    $status_str = $statusObj->name;
    $status_str = phrase($statusObj->name, CAPITALIZE);
    if($resObj->status_change_timestamp > 1) $status_str .= " (".sql2human($resObj->status_change_timestamp).")";
    $request .= phrase('status', CAPITALIZE).":".html_break().html_tooltip(html_b($status_str), $statusObj->description).html_break(2);
  }

  // checkin - checkout
  if($can_edit) $edit_link = $hourly ? res_change_link($resObj, 'time', $user_level) : res_change_link($resObj, 'dates', $user_level);
  
  $show_time = $num_hours > 0 ? true : false;
  $checkin_str = sql2human($resObj->checkin, array('show_weekday' => true, 'show_time' => $show_time));
  $checkout_str = sql2human($resObj->checkout, array('show_weekday' => true, 'show_time' => $show_time));
  if($change == 'dates') { // show choosers
  	
  	// checkin
  	$calparams['minDate'] = "'+0d'";
  	$calparams['maxDate'] = "'+1y'";
  	$calparams['dependent_field'] = "out"; // update checkout
  	$calparams['dependent_field_add_days'] = $resObj->num_days;  // with checkin + num_days days
    $in_chooser = phrase('checkin', CAPITALIZE).": ".$edit_link.html_break().datepicker('in', $calparams, $resObj->checkin).html_break(2);

    // checkout
    if($can_edit) $edit_link = res_change_link($resObj, 'dates', $user_level);
    $checkout_str = sql2human($resObj->checkout, array('show_weekday' => true));
  	unset($calparams['dependent_field']);
  	unset($options['dependent_field_add']);
    $out_chooser = phrase('checkout', CAPITALIZE).": ".$edit_link.html_break().datepicker('out', $calparams, $resObj->checkout);
    $request .= html_form($form.$in_chooser.$out_chooser.$submit.html_break(2));
    
  } elseif($change == 'time') { // show choosers

  	// checkin date
  	$calparams['minDate'] = "'+0d'";
  	$calparams['maxDate'] = "'+1y'";
  	$calparams['dependent_field'] = "out"; // update checkout
  	$calparams['dependent_field_add_days'] = $resObj->num_days;  // with checkin + num_days days
    $in_chooser = phrase('checkin', CAPITALIZE).": ".$edit_link.html_break().datepicker('in', $calparams, $resObj->checkin);

    // checkin time
    $first_hour = ($start_hour = hm2dectime($aptObj->start_hour)) ? $start_hour : 8;
    $last_hour = ($end_hour = hm2dectime($aptObj->end_hour)) ? $last_hour = $end_hour - $min_stay : 23;
    $timeArray = time_array($first_hour, $last_hour, 0.5);
    $time = hm2dectime(strip_date($resObj->checkin));
    $time_chooser  = select_from_array('time', $timeArray, $time ?: 9);

    // num_hours
    $hoursArray = num_array($min_stay, 12, 0.5); // half hour
    $nh_chooser = select_from_array('nh', $hoursArray, clean_num($resObj->num_hours));

    $request .= html_form($form."$in_chooser $time_chooser".html_break().$nh_chooser.' '.phrase('hours').' '.$submit.html_break(2));

  } else {
    $request .= phrase('checkin', CAPITALIZE).": ".$edit_link.html_break().html_b($checkin_str).html_break(2);
    $request .= phrase('checkout', CAPITALIZE).": ".$edit_link.html_break().html_b($checkout_str).html_break(2);
  }

  $boat_type = $ptype == 'boat' ? get_object($ptype, $apt_id, 'boat_type') : '';
  if($ptype == 'boat' && $boat_type != BOAT_TYPE_SELF_OPERATED) {
    $package_id = $resObj->package_id;
    $catering_id = $resObj->catering_id;
    if($can_edit) {
      $package_edit_link = res_change_link($resObj, 'package', $user_level);
      $catering_edit_link = res_change_link($resObj, 'catering', $user_level);
    }

    //$package_tip = tooltip('package_tip', phrase('text_package'));
    //$request .= phrase('package', CAPITALIZE).": $package_edit_link $package_tip".html_break();
    //if($change == 'package') {
    //  $package_chooser = package_chooser($ptype, $apt_id, array('notip' => true, 'value' => $resObj->package_id));
    //  $request .= html_form($form.$package_chooser.' '.$guest_str.' '.$submit.html_break());
    //} else {
    //  $product_name = $package_id ? get_object('product', $package_id, 'name') : phrase('none', CAPITALIZE);
    //  $request .= html_b($product_name).html_break(2);  
    //}

    $request .= html_tooltip(phrase('catering', CAPITALIZE).": ", phrase('text_catering')).$catering_edit_link.html_break();
    if($change == 'catering') {
      $catering_chooser = catering_chooser($ptype, $apt_id, array('notip' => true, 'value' => $resObj->catering_id));
      $request .= html_form($form.$catering_chooser.' '.$guest_str.' '.$submit.html_break());
    } else {
      $product_name = $catering_id ? get_object('product', $catering_id, 'name') : phrase('none', CAPITALIZE);
      $request .= html_b($product_name).html_break(2);  
    }

  }
  
  $edit_link = $can_edit && $hourly && !$is_ticket? res_change_link($resObj, 'time', $user_level) : '';
  $request .= sidebar_length($num_days, $num_hours)." ".$edit_link.html_break();

  if($num_guests = $resObj->num_guests) {
    if($can_edit) $edit_link = res_change_link($resObj, 'guests', $user_level);
    $guest_str = ($num_guests == 1 ? phrase('guest', LOWERCASE) : phrase('guests', LOWERCASE)).' '.$edit_link;
    if($change == 'guests') { // show choosers
      $ng_chooser = select_from_array('ng', num_array(1, $property->max_guests, 1), $resObj->num_guests);
      $request .= html_form($form.$ng_chooser.' '.$guest_str.' '.$submit.html_break());
    } else {
      $request .= html_b($num_guests).' '.$guest_str.html_break();
    }
  }
  //dump($resObj);
  if($num_apts > 0) {
    $type_str = property_type_name($resObj->property_type, $num_apts);
    $request .= html_b($num_apts).' '.$type_str.html_break();
  }
  
  if($num_rooms = $resObj->num_rooms) {
    $rooms_str = $num_rooms == 1 ? phrase('room', LOWERCASE) : phrase('rooms', LOWERCASE);
    $request .= html_b($num_rooms). ' '.$rooms_str.html_break();
  }
    
  $request .= html_break();
  if($apt_id) $request .= print_rate_box($resObj);
  if(!$hourly) {
    $searchObj = null;
    if($pb_id = $resObj->price_breakdown_id) {
      $searchObj->id = $pb_id;
    } elseif(!$resObj->res_id) {
      $searchObj = res2searchObj($resObj);
      $searchObj->apt_id = $apt_id;
      $searchObj->fees = $resObj->fees;
      $searchObj->city_tax = $resObj->city_tax;
    }
    if($searchObj) {
      $fancy_breakdown_link = html_fancybox_link(ajax_operation_link('price-breakdown', $searchObj, array('public' => true)),  phrase('price_breakdown', CAPITALIZE));
      $request .= $fancy_breakdown_link.html_break();
    }
  }
  return title_box($title, $request, $class);
}


function sidebar_length($num_days, $num_hours, $bold=true) {  // length of stay
  $unit = $num_hours > 0 ? 'hour' : ($resObj->start_hour && $resObj->end_hour ? 'shift' : 'day');
  $num_units = $num_hours > 0 ? $num_hours : $num_days;
  return unit_string($unit, $num_units, array('bold' => $bold, 'show_count' => true));
}

// links shown in sidebar for changing reservation (property, num guests, dates)
function res_change_link($resObj, $var, $user_level=USER_LEVEL_GUEST) {
  $page = $_SESSION['page'];

  // permissions
  if($resObj->status < 0) return ''; // cannot edit cancelled reservations
  else if($resObj->checkout < today()) return ''; // cannot edit past reservations
  else if($var == 'checkin' && $resObj->checkin < today()) return ''; // cannot edit checkin if in the past
  else if($resObj->status >= STATUS_PROPOSED) { 
    if($var=='property' && $user_level < USER_LEVEL_STAFF) return ''; // only staff can change property after it is reserved
    if($page != 'Status') return ''; // can only change in process page if reserved
  }

  if($change = $_GET['change']) {
    if($change != $var) return ''; // no change link, we're already changing another section
    $cancel = true; // are we changing this section - show cancel link
  }
  $text = $cancel ? '[ '.phrase('cancel', CAPITALIZE).' ]' : '[ '.phrase('change', CAPITALIZE).' ]';

  // base link
  $city_dir = city_link($resObj->city_id);
  $base_link = $page == 'Status' ? 
    action_link('reservation', $resObj, array('user_level' => $user_level)) : // process
    get_page_link($page, true, '', '', $city_dir)."?rid=$resObj->res_id&pin=".myhash($resObj->booking_pin); // request/book
    
  if($cancel) $ch_link = $base_link;
  elseif(!$resObj->res_id) $ch_link = res2search($resObj); // no res id, go back to search (property page)
  else $ch_link = $base_link."&change=$var"; // cancel or edit link  
  $ch_link = htmlspecialchars($ch_link, ENT_COMPAT, 'UTF-8', false); // convert to &amp; w/o double encoding;

  //echo("bl=$base_link cl=$ch_link<br>");
  //echo("var=$var city_id = $resObj->city_id cd=$city_dir bl=$base_link cl=$ch_link<br>");
  //dump(res2search($resObj));
  
  return $ch_link ? html_link($ch_link, $text) : '';

//  switch($var) {
//    case "property":
//      break;
//    case "dates":
//      break;
//    case "guests":
//      break;
//    default:
//      die("res_change_link: missing or unknown var $var");
//      break;
//  }
//  return $link;
}

// returns boat/boats, apartment/apartments etc
// todo: case
function property_type_name($type, $count=1, $case='', $lang='') {
  if($lang) $options['language'] = $lang;
  switch($type) {
  case 'boat':
    return $count==1 ? phrase('boat', $case, $options) : phrase('boats', $case, $options);
  case 'shortstay':
    return $count==1 ? phrase('apartment', $case, $options) : phrase('apartments', $case, $options);
  case 'longstay':
    return $count==1 ? phrase('apartment', $case, $options) : phrase('apartments', $case, $options);
  case 'realestate':
    return $count==1 ? phrase('house', $case, $options) : phrase('houses', $case, $options);
  default:
    return $count==1 ? phrase($type, $case, $options) : phrase($type.'s', $case, $options);
  }    
}

// returns boat_type for boat, apartment_type for apartment etc
function property_subtype($apt) {
  $ptype = $apt->type;
  if($ptype == 'boat') return 'boat_type';
  if(in_array($ptype, array('shortstay', 'longstay', 'realestate'))) {
    if($apt->apartment_type) return 'apartment_type';
    else if($apt->house_type) return 'house_type';
  }    
  return '';
}

// returns apartment type, house type, boat type, etc
function property_subtype_name($apt, $case='') {
  $ptype = $apt->type;
  if($ptype == 'boat') {
    return $apt->boat_type ? phrase(get_object('boat_type', $apt->boat_type, 'name'), $case) : '';
  } else if(in_array($ptype, array('shortstay', 'longstay', 'realestate'))) {
    if($apt->apartment_type) return phrase(get_object('property_apartment_type', $apt->apartment_type, 'name'), $case);
    else if($apt->house_type) return phrase(get_object('property_house_type', $apt->house_type, 'name'), $case);
  }    
  return '';
}

function print_rate_type($rate_type) {
  switch($rate_type) {
    case 'person':
      return phrase('per_person', CAPITALIZE);
      break;
    case 'pp_hour':
      return phrase('per_person_per_hour', CAPITALIZE);
      break;
    default:
      return phrase($rate_type, CAPITALIZE).' '.phrase('rate', CAPITALIZE);
  }
}

// prints rate with currency and optionally, overstrike rate + discount rate, or (if rate change > 0), just the higher rate
function print_rate($cur_str, $rate, $discount=0, $unit='', $format=FORMAT_ALWAYS) {
  //$new_rate = apply_surcharge($rate, $rate_change);
  $new_rate = apply_discount($rate, $discount);
  $rate = $new_rate < $rate ? 
    html_strike(formatFloat($rate, $format, $cur_str)).' '.formatFloat($new_rate, $format, $cur_str) : 
    formatFloat($rate, $format, $cur_str);
  return $unit ? "$rate / ".phrase($unit) : $rate;
}

// prints total, paid, fees, balance
/** Todo: move to smarty template */
function print_rate_box($resObj, $user_level = 0) {
  //dump($resObj);
  $cur_str    = get_currency_symbol($resObj->currency, true); // HTML currency symbol

  // begin output 
  $lines = array();
  if($rate_type != 'total') { // show day rate
    $lines[] = array(print_rate_type($resObj->rate_type).': ', print_rate($cur_str, $resObj->rate_base, $resObj->discount));    // base rate + discounted rate if any
  }
  if($resObj->fees > 0) {
    $feeArray = get_reservation_fees($resObj);
    //echo("fees=$resObj->fees");dump($feeArray);//dump($resObj);
    $tip = print_fees($resObj->currency, $feeArray);
    $lines[] = array(phrase('fees', CAPITALIZE).": ".html_tooltip($tip), formatFloat($resObj->fees, FORMAT_ALWAYS, $cur_str));
  }

  // all other invoice lineitems
  $extras = $resObj->other_charges;
  if($extras > 0) {
    $extra_desc = "All other products and services.";
    $tip = $extra_desc;
    $lines[] = array(phrase('extras', CAPITALIZE).': '.html_tooltip($tip), formatFloat($extras, FORMAT_ALWAYS, $cur_str));  // total
  }
    
  if($resObj->city_tax > 0) {
    list($tt_tax_amount, $tt_desc, $tt_quant) = get_city_tax($resObj);
    $tip = $tt_desc;
    $lines[] = array(phrase('tax', CAPITALIZE).': '.html_tooltip($tip), formatFloat($resObj->city_tax, FORMAT_ALWAYS, $cur_str));  // total
  }
  
  /** show service fee */
  $service_fee = $resObj->fee_service;
  if($user_level != USER_LEVEL_OWNER && $service_fee > 0) {
    $siteObj = object_site($resObj);
    $tip = "Fee charged by {$siteObj->name} for our services.";
    $lines[] = array(phrase('service_fee', CAPITALIZE).': '.html_tooltip($tip), formatFloat($resObj->fee_service, FORMAT_ALWAYS, $cur_str));  // total
    $grand_total = $resObj->grand_total;
  } else {
    $grand_total = $resObj->grand_total - $service_fee; // don't show this to host
  }
  
  $lines[] = array(phrase('total', CAPITALIZE).': ', formatFloat($grand_total, FORMAT_ALWAYS, $cur_str));  // total

  if($resObj->paid > 0) {
    $lines[] = array(phrase('paid', CAPITALIZE).": ", formatFloat($resObj->paid, FORMAT_ALWAYS, $cur_str));
    $lines[] = array(phrase('balance', CAPITALIZE).": ", formatFloat($resObj->balance, FORMAT_ALWAYS, $cur_str));
  }
  
  $table = html_table($lines, array('class' => 'accounting_table', 'header' => 0));
  //return $table;
  
  $rate_box = html_div($table, 'bold', "id='rate_box'");
  return $rate_box;
  //return round_box($rate_box, 'box_dark_color');
}


// returns a description like "Reservation 11233: Guest Name @ Apartment: Dates
function get_reservation_description($resObj) {
  $guest_name = get_name_string($resObj);
  return phrase('reservation', CAPITALIZE)." $resObj->res_id: $guest_name @ $resObj->apartment_name";
}

function get_order_description($orderObj) {
  $first_product_name = find_object('order_row', array('order_id' => $orderObj->id), 'description', array('sort' => 'id'));
  return phrase('payment_order', CAPITALIZE)." $orderObj->id: $first_product_name";
}

function capture_form($paymentObj) {
  $payment_id = $paymentObj->id;
  list($auth_currency, $auth_amount) = payment_currency_amount($paymentObj); // capture in same currency / amount as authorization

  $form .= form_input('amount', 'text', $_POST['amount'] ? $_POST['amount'] : $auth_amount, array('size' => 6)).' '.$auth_currency." (Max. $auth_amount $auth_currency)".html_break();    
  $form .= form_input('currency', 'hidden', $auth_currency);    
  $form .= form_input('payment_id', 'hidden', $payment_id);    
  $form .= "Note to customer:".html_break().form_text('capture_note', $_POST['capture_note'], array('rows' => 3, 'cols' => 40, 'max' => 255)).html_break();    
  $form .= form_input('close_auth', 'hidden', 1);
  $form .= html_break();
  return $form;
}

function refund_form($paymentObj) {
  $payment_id = $paymentObj->id;
  list($payment_currency, $payment_amount) = payment_currency_amount($paymentObj);
  $currency = $paymentObj->currency;
  $amount   = $paymentObj->amount;
  
  $sum_refund = sum_refund($paymentObj->id); // in invoice currency, 
  $max_refund = $amount - $sum_refund;

  if($currency != $payment_currency) $form .= html_b("The actual refund will be issued in the payment currency ($payment_currency).").html_break();
  
  $form .= form_input('amount', 'text', $_POST['amount'] ? $_POST['amount'] : number_format($max_refund,2,'.', ''), array('size' => 6)).' '.$currency." (Max. $max_refund $currency)".html_break();    
  $form .= form_input('currency', 'hidden', $currency);    
  $form .= form_input('payment_id', 'hidden', $payment_id);    
  $form .= "Note to customer:".html_break().form_text('refund_note', $_POST['refund_note'], array('rows' => 3, 'cols' => 40, 'max' => 255)).html_break();    
  $form .= html_break();
  return $form;
}

function withdrawal_form($paymentObj) {
  $payment_id = $paymentObj->id;

  list($withdraw_currency, $max_withdraw) = payment_net_outstanding($paymentObj);  
  
  $withdraw_gross = $max_withdraw;
  $withdraw_fee = withdrawal_fee($withdraw_currency, $max_withdraw);
  $withdraw_net = $max_withdraw - $withdraw_fee;
  
  if(0 && $_SESSION['dev']) {
    $form .= form_input('amount', 'text', $_POST['amount'] ? $_POST['amount'] : number_format($withdraw_net,2), array('size' => 6)).' '.$payment_currency." (Max. $withdraw_net $payment_currency)".html_break();    
  } else {
    $form .= form_input('amount', 'hidden', $_POST['amount'] ? $_POST['amount'] : number_format($withdraw_net,2), array('size' => 6));    
  }
  $form .= form_input('currency', 'hidden', $withdraw_currency);    
  $form .= form_input('gross', 'hidden', $withdraw_gross);    
  $form .= form_input('fee', 'hidden', $withdraw_fee);    
  $form .= form_input('net', 'hidden', $withdraw_net);    
  $form .= form_input('payment_id', 'hidden', $payment_id);    
  $form .= "Note to host:".html_break().form_text('withdraw_note', $_POST['withdraw_note'], array('rows' => 3, 'cols' => 40, 'max' => 255)).html_break();    
  $form .= html_break();
  return $form;
}

// returns agents that handles given combination of type/city/language
function  find_agents($property_type, $city_id, $language='') {
  global $db;
  if(!$property_type) die("find_agents: missing property_type");
  if(!$city_id)      die("find_agents: missing city_id");

  $pt_id = db_getOne("SELECT id FROM property_type WHERE name='$property_type'");
  $qs = "SELECT id FROM agent WHERE cities like '%$city_id%' AND types like '%$pt_id%'";
  if($language) $qs .= " AND (languages = '' OR languages like '%$language%')";
 
  $agents = db_getCol($qs);
  return $agents;
}

function correspondence_recipients($obj, $user_level) {
  // find email addresses

  // staff mail
  $s = $_SESSION['site'];
  $staff_name   = $s->name;
  $staff_email  = $s->email_address;

  // guest mail
  $guest_name   = get_name_string($obj);
  $guest_email  = $obj->email_address;

  // host mail
  if($hostObj   = get_object('user', $obj->host_id))  {
    $host_name  = get_name_string($hostObj);
    $host_uid   = $hostObj->id;
    $host_email = $hostObj->email_address;
  }

  // agent mail
  if($agentObj   = get_object('agent', $obj->agent_id))  {
    $agent_name  = get_name_string($agentObj);
    $agent_email = $agentObj->email_address;
  }
        
  if($guest_name && $guest_email) $recipients[USER_LEVEL_GUEST] = phrase('guest', CAPITALIZE)." (".$guest_name.")";
  if($host_name && $host_email) $recipients[USER_LEVEL_OWNER] = phrase('host', CAPITALIZE)." (".$host_name.")";
  if($agent_name && $agent_email) $recipients[USER_LEVEL_AGENT] = phrase('agent', CAPITALIZE)." (".$agent_name.")";
  if($staff_name && $staff_email) $recipients[USER_LEVEL_STAFF] = phrase('staff', CAPITALIZE)." (".$staff_name.")";
  if(count($recipients) > 1) $recipients[0] = phrase('everybody', CAPITALIZE);
      
  // can't send message to ourselves
  unset($recipients[$user_level]);
  return $recipients;
}

function correspondence_form($obj, $user_level, $recipients) {    
  // previous correspondence           
  $chat_table = get_correspondence($enq_type, $id, $user_level);
  
  // the form
  if($chat_table) $correspondence_form .= $chat_table.html_break(3);  
  $correspondence_form .= html_b(phrase('to', CAPITALIZE).": ").select_from_array('receiver_level', $recipients).html_break(2);
  $correspondence_form .= form_text('message', $inputVars['message'], array('rows' => 6, 'cols' => 52));
  return $correspondence_form;  
}

function get_min_stay($type) {  
  switch($type) {
    case 'shortstay': 
      return 3;
    case 'hotel':
      return 1;
    default:
      return 1;
    }
}

function res2searchObj($resObj) {
  if($resObj->res_id) $searchObj->rid = $resObj->res_id; 
  $searchObj->t = $searchObj->pt = $type = $resObj->property_type; 
  $searchObj->eid = $resObj->event_id;
  $searchObj->sh = $resObj->shift;
  $searchObj->ng = $resObj->num_guests;
  $searchObj->in = $in = $resObj->num_days > 0 ? strip_time($resObj->checkin)  : $resObj->checkin;
  $searchObj->out = $out = $resObj->num_days > 0 ? strip_time($resObj->checkout) : $resObj->checkout;
  $hourly = hourly_rate($type);
  if($hourly || $resObj->num_hours > 0) {
    $searchObj->unit = 'hour';
    $searchObj->nh = $searchObj->num_units = clean_num($resObj->num_hours) ?: period_diff($in, $out, 'h');
  } else {
    $searchObj->unit = 'day';
    if($resObj->num_days > 0)  $searchObj->nd = $searchObj->num_units = $resObj->num_days ?: period_diff($in, $out, 'd');
  }
  return $searchObj;
}

// takes a resObj, returns link to property search page (used for modifying request)
function res2search($resObj, $linkonly=true, $title='') {
  if(!$resObj) return '';  
  //$in  = escape_uri($resObj->num_days ? strip_time($resObj->checkin) : $resObj->checkin);
  //$out = escape_uri($resObj->num_days ? strip_time($resObj->checkout) : $resObj->checkout);
  $in  = escape_uri(strip_time($resObj->checkin));
  $out = escape_uri(strip_time($resObj->checkout));


  if($resObj->event_type == EVENT_TYPE_TICKET) { // this is a ticket, show ticket page
    $qs = "t=$resObj->property_type&id=$resObj->apt_id&start=$in&ng=$resObj->num_guests";
    if($agent_id = $resObj->agent_id) $qs .= "&agid=$agent_id";

    $link = get_page_link(PAGE_TICKET).'?'.$qs;
        
  } else {
    $qs = "t=$resObj->property_type&in=$in&ng=$resObj->num_guests";
    if($agent_id = $resObj->agent_id) $qs .= "&agid=$agent_id";
  
    if($catering_id = $resObj->catering_id) $qs .= "&catering=$catering_id";
    
    if($sn = $resObj->sn) $qs .= "&sn=$sn"; // search was for one particular property
    else $qs .= "&search=Search"; // search all
    
    if($resObj->num_days > 0) {
      $nd = $resObj->num_days;
      $qs .= "&nd=$resObj->num_days&out=$out";
    } else if($resObj->num_hours > 0) {
      $in_time = strip_date($resObj->checkin);
      $nh = clean_num($resObj->num_hours);
      $qs .= "&nh=$nh&time=".hm2dectime($in_time);
    }
    $city_dir = city_dir($resObj->city_id);

    $property_link = get_page_link(property_page_type($resObj->property_type), true, '', '', $city_dir);
    //echo("city_dir=$city_dir<br>");
    //$property_link = site_page_link($_SESSION['siteObj'], property_page_type($resObj->property_type)); // property_page_type($resObj->property_type), true, '', '', $city_dir);

    if(!$property_link) return '';
    $link = $property_link.'?'.$qs;
  }
  
  if($linkonly) return $link; // link only - no html
  
  // return full link
  if(!$title) $title = phrase('search', CAPITALIZE);
  return html_link($link, $title);
}

function res2search2($resObj, $title='', $linkonly=false) {
  //$qs = 
  $city_dir = city_link($resObj->city_id);
  $link = get_page_link(property_page_type($resObj->property_type), true, '', '', $city_dir).'?'.$qs;
  return $linkonly ? $link : html_link($link, $title);

}

// takes a property type (string), returns corresponding page_type ID
function property_page_type($type) {
  $type = strtolower($type);
  switch($type) {
    case 'shortstay':
      return PAGE_SHORTSTAY;
    case 'longstay':
      return PAGE_LONGSTAY;
    case 'realestate':
      return PAGE_REALESTATE;
    case 'boat':
      return PAGE_BOAT;
    case 'office':
      return PAGE_OFFICE;
    case 'hotel':
      return PAGE_HOTEL;
    case 'shortstay-office':
      return PAGE_SHORTSTAY-OFFICE;
    default:
      return PAGE_PROPERTY;      
  }
}
      
      
function get_shift_hours($shift) {
  switch($shift) {
    case DAY_SHIFT:
      $start_time = '09:00:00';
      $end_time   = '21:00:00';
      break;
    case MORNING_SHIFT:
      $start_time = '09:00:00';
      $end_time   = '15:00:00';
      break;
    case AFTERNOON_SHIFT:
      $start_time = '15:00:00';
      $end_time   = '21:00:00';
      break;
    default: 
      $start_time = $end_time   = '';
  }
  return array($start_time, $end_time);
}

function apply_shift_hours($checkin, $checkout, $start_time, $end_time) {
  if($start_time && $end_time) {
    $checkin  = strip_time($checkin)." $start_time";
    $checkout = strip_time($checkout)." $end_time";
  }
  return array($checkin, $checkout);
}

// a property can have a rate array like this: 3=145;4=180;5=215;8=330 (rate_array_unit must also be set);
function rate_array_rate($aptObj, $searchObj) {
  $unit = $aptObj->rate_array_unit;
  if(!$aptObj->rate_array || !$unit) return array('', '');
  $rate_array = filters2array($aptObj->rate_array);
  
  if($searchObj->nh > 0) $count = $searchObj->nh;
  else if ($num_days > 0) $count = $searchObj->nd;
  else return array('', '');
  
  if($rate_array[$count]) return array($unit, $rate_array[$count] / $count); // count is in array
  
  $keys = array_keys($rate_array);
  $max = max($keys);
  $min = min($keys);
  if($count > $max) return array($unit, $rate_array[$max] / $max); // count is more than max, use max
  if($count < $min) return array('', ''); // count is less than than min, does not qualify
  
  // step through array to check if count lies between 2 values
  sort($keys); 
  for($i=0;$i < count($keys) -1; $i++ ) {
    $rate_key = $keys[$i];
    $next_rate_key = $keys[$i + 1];
    //echo("is $count between $rate_key and $next_rate_key ?<br>");
    if($count > $rate_key && $count < $next_rate_key) {
      //echo("$count is between $rate_key and $next_rate_key, using $rate_key (".$rate_array[$rate_key] / $rate_key.")<br>");
      return array($unit, $rate_array[$rate_key] / $rate_key);
    }
  }
  //echo("$count is not in array (".implode(',', $keys)."), not bigger than max ($max), not less than min ($min) and nowhere in between<br>");
  return array('', ''); // should never get here  
}


// returns rate per day, hour, or shift
function get_base_rate($apt, $searchObj=NULL) {
  if($apt->rate_array) list($array_unit, $array_rate) = rate_array_rate($apt, $searchObj);
  $hourly = hourly_rate($apt->type);
  if($hourly) {
    if($array_unit == 'hour') return array($array_unit, $array_rate);
    return array('hour', $apt->rate_hour);
  } else {
    switch($searchObj->sh) {
      case MORNING_SHIFT:
        return array('shift', $apt->rate_morning);
      case AFTERNOON_SHIFT:
        return array('shift', $apt->rate_afternoon);
      default:
        if($array_unit == 'day') return array($array_unit, $array_rate);
        return array('day', $apt->rate_day);
    }    
  }
}

/* given various possible in/out combinations in search, caculates the following:
   checkin (in)
   checkout (out)
   start_hour (sh)  
   end_hour   (eh)

   // num_days (nd)
   // num_hours (nh)
   // num_months (nm)
   returns and object with the variable names listed first (for resFormat=true), or in parentheses (if $resFormat=false);
   
*/
   
// do not use this for existing reservations, only new ones
// todo: allow time for daily reservations (how do we know?)
// if $time is set, time of checkin is ignored
function search2in_out($searchObj, $resFormat = false) {
  $checkin  = $in = date2sql($searchObj->in);
  $checkout = $out = date2sql($searchObj->out);
  $hourly = 0;
  if($checkin && !$checkout && $searchObj->nh && $searchObj->time) { // calculate in/out time from in_date + time + num_hours    
    list($checkin, $checkout) = date_time_hours2in_out($checkin, $searchObj->time, $searchObj->nh);
    $resObj->num_hours = $searchObj->nh;
    $resObj->num_days = $searchObj->nd = $resObj->num_months = $searchObj->nm = 0; // no days/months for hourly
    $hourly = 1;
  } else if($shift = $searchObj->sh) { // day shift
    list($start_hour, $end_hour) = get_shift_hours($shift);
    list($checkin, $checkout) = apply_shift_hours($checkin, $checkout, $start_hour, $end_hour);  
    $resObj->start_hour = $start_hour;
    $resObj->end_hour   = $end_hour;
    //$num_days++;
  } else { // could be hourly or daily
    if($searchObj->nd > 0 || !strip_date($checkin)) { // no time, must be daily
      $resObj->num_hours = 0;
      $resObj->num_days = $searchObj->nd = period_diff($in, $out, 'd'); 
      $resObj->num_months = period_diff($in, $out, 'm'); // no days/months for hourly
    } else { // hourly ?
      $resObj->num_hours = $searchObj->nh = $nh = period_diff($in, $out, 'h');
      //echo("$in to $out = $nh hours<br>");
      $resObj->num_days = $resObj->num_months = 0; // no days/months for hourly
      $hourly = 1;
    }
  }
  
  $resObj->checkin  = $searchObj->in = $checkin;
  $resObj->checkout = $searchObj->out = $checkout;

  if($resObj->hourly = $hourly) {
    $resObj->unit = $searchObj->unit = 'hour';
    $resObj->num_units = $searchObj->num_units = $searchObj->nh;
  } else {
    $resObj->unit = $searchObj->unit = 'day';
    $resObj->num_units = $searchObj->num_units = $searchObj->nd;
  }
  
  if($resFormat) return $resObj;
  else return $searchObj; 
}
            

// do not use this for existing reservations, only new ones
// takes a searchObj (typically from $_GET) or inputVars($_GET or $_POST) and a propertyObj
// returns a dummy resObj (not from DB) with rates and fees and any reservation value in the searchObj
function search2res($searchObj) {
  global $u_guestid;
  if(is_array($searchObj)) $searchObj = array2obj($searchObj);
  //if($property && !is_object($property) && $searchObj->type && $searchObj->id) $property = get_object($searchObj->type, $searchObj->id); 
  $searchObj = search2in_out($searchObj);
  $resObj =  search2in_out($searchObj, true);
  
  //dump($resObj);die();
  if($searchObj->apt_id) $searchObj->id = $searchObj->apt_id;
    
  // copy from posted vars
  $class_def  = get_class_def('reservation', true);
  $res_fields = $class_def->fields;
  foreach($res_fields as $key=>$fld) { 
    $key = array_pop(explode('.', $key)); // the field
    if($key != 'id' && isset($searchObj->$key)) $resObj->$key = $searchObj->$key;
  }
  //dump($resObj);die();
  
  // fetch property
  if($apt_ids = explode(',', $searchObj->id)) { // multiple properties
    $resObj->block_count = $num_apts = $searchObj->nu;
    if($num_apts < count($apt_ids)) $apt_ids = array_slice($apt_ids, 0, $num_apts); // only need num units
    $resObj->apt_list   = implode(',', $apt_ids);
    $resObj->apt_id     = $apt_id = $apt_ids[0];
  } else {
    $resObj->apt_id     = $apt_id = $searchObj->id;
  }
  $resObj->event_id       = $event_id = $searchObj->eid;
  $resObj->agent_id       = $agent_id = $searchObj->agid;
  $resObj->property_type  = $ptype  = pick_first($searchObj->t, $searchObj->property_type, $searchObj->obj_type);
  $resObj->catering_id    = $catering_id = $searchObj->catering;
  if($sn = $searchObj->sn) $resObj->sn = $sn;

  if($agent_id) {    
    $agentObj = get_object('agent', $agent_id);
    $resObj->email_address = $agentObj->email_address;
    $resObj->organization = $agentObj->name;
    $resObj->hide_address = true;
  }
  //dump($searchObj);

  if(!$apt_id && !$ptype) {
    die("search2res: missing property type and id");
  }
  
  if($apt_id) {
    $aptObj = $property = get_object($ptype, $apt_id); 
    if(!$property) die("search2res: couldn't find $ptype $apt_id");
  }
  
  $resObj->client_id      = $property->client_id ?: $_SESSION['client_id'];
  $resObj->num_guests     = $searchObj->ng ?: 0;
  $resObj->apartment_name = $property->name ?: '???';
  $resObj->city_id        = $city_id = $property->city_id ?: $searchObj->city_id;
  $resObj->host_id       = $host_id = $property->host_id ?: $searchObj->host_id;
  
  $checkin   = $resObj->checkin;  
  $checkout  = $resObj->checkout;  
  $num_days  = $resObj->num_days;
  $num_hours = $resObj->num_hours;

  // copy address from guest
  if($u_guestid) {
    $guestObj = get_object('user', $u_guestid);
    copy_object_vars('address', $guestObj, $resObj);
    $resObj->guest_id = $u_guestid;      // works without this line (set in request.php)
    $resObj->org_id = $guestObj->org_id; // works without this line (set in request.php)    
  }
  
  // calculate rate, fees, and total: todo: implement boats
  if($apt_id && in_array($ptype, array('shortstay', 'shortstay_office', 'boat'))) { // calculate rate, used in left box (print_request())    
  

    if($event_id = $resObj->event_id) {
      $eventObj = get_object('event', $event_id);
      $resObj->event_type = $eventObj->type;
    }

    if($eventObj->ticket_price > 0) {
      $currency = $eventObj->ticket_currency ?: $property->currency;
      $num_units = $resObj->num_guests;
      $unit = 'ticket';
      $discount = $eventObj->ticket_discount;
      $rate_per_unit = $eventObj->ticket_price;
      $resObj->deposit_percentage  = 100;
    } else {
      $currency = $property->currency;
      
      $priceObj = price_breakdown($aptObj, $searchObj);

      $unit = $priceObj->unit;
      $num_days = $priceObj->num_days;
      $rate_per_unit = round($priceObj->average, 2);
      $discount = $priceObj->discount;
      $resObj->rate_class = $priceObj->rate_class ?: RATE_CLASS_STANDARD;
      //$resObj->search = dump($searchObj, true);                       
      //$resObj->price = "For $aptObj->name $searchObj->in - $searchObj->out = ".dump($priceObj, true);                       
      //$resObj->rate_type = 'day'; // todo: add shift to valid rate types. for now use day
      switch($unit) {
        case 'hour':
          $num_units = $num_hours;
          break;
        case 'day':
          $num_units = $num_days;
          break;
        case 'month':
          $num_units = $num_months;
          break;
        case 'hour':
          $num_units = 1;
          break;
      }
      $discount = $property->discount ?: percent2discount($property->rate_change); // discount not used any more, kept for now for backwards compatability
    
      $feeAr = get_property_fees($property, $searchObj);
      foreach($feeAr as $fee_key => $fee_val) $resObj->$fee_key = $fee_val; // copy fees from property to reservation    
    }
    $resObj->currency  = $currency;
    $resObj->cur = get_currency_symbol($currency, true);    
    $resObj->rate_base = $rate_per_unit;
    $resObj->rate_type = $unit; // todo: add shift to valid rate types. for now use day
    $resObj->discount  = $discount;
    $resObj->rate_discount = $discount_rate = $discount ? apply_discount($rate_per_unit, $discount) : $rate_per_unit;
    $resObj->total  = $total = $discount_rate * $num_units;
    //echo("full rate=$rate_ng discount=$discount discount rate=$discount_rate num_units=$num_units");
       
  
    // city tax, VAT
    //if($resObj->charge_tax = $aptObj->charge_tax) $resObj->tax_included = $aptObj->tax_included; 

    if($resObj->charge_tax = $aptObj->charge_tax) $resObj->tax_included = 1; // tax_included hack 
    $resObj->charge_city_tax =  $aptObj->charge_city_tax;
  
    // special for Sicco
    if($aptObj->host_id = 1284 && $num_days > 0 && $num_days < 6) {
      $resObj->charge_city_tax = $resObj->charge_tax = $resObj->tax_included = 0;
    }
    
    if($resObj->charge_city_tax) {
      list($tt_amount, $resObj->city_tax_desc, $tt_quant) = get_city_tax($resObj, $total);
      $resObj->city_tax = $tt_amount * $tt_quant;
    } else {
      $resObj->city_tax = 0;
    }

    $resObj->grand_total = $grand_total = $total + $resObj->fees + $resObj->city_tax; 
    //echo("total=$total ct=$resObj->city_tax gt = $resObj->grand_total<br>");


    // reservation block
    if($num_apts > 1) {
      $resObj->block_total = $block_total = $total * $num_apts;
      $resObj->block_grand_total  = $block_total + $fees;
    }


  }
  //dump($resObj);
  return $resObj;
}

// used in request.php and process.php to process changes from the sidebar
function process_reservation_changes($change_vars, $resObj, $aptObj, $errors, $messages) {
  //dump($change_vars);
  $type_str = property_type_name($resObj->property_type, 1, CAPITALIZE);
  
  $res_id = $resObj->res_id; 
  $property_type = $resObj->property_type;
  $change_msgs = array();
  if($new_apt_id = $change_vars['apt_id']) { // change property
    $changes['apt_id'] = $new_apt_id;
    $aptObj = get_object($resObj->property_type, $new_apt_id);
    $new_apt_name = $aptObj->name;
    $change_msgs[] = phrase('property', CAPITALIZE).' '.phrase('changed_to', LOWERCASE).' '.$new_apt_name;
  }

  if(isset($change_vars['package_id']) && $change_vars['package_id'] != $resObj->package_id) { // change package
    $new_package_id = $change_vars['package_id'];
    $changes['package_id'] = $new_package_id;
    if($new_package_id) {
      $productObj = get_object('product', $new_package_id);
      $change_msgs[] = phrase('package', CAPITALIZE).' '.phrase('changed_to')." ".product_print($productObj);
    } else {
      $change_msgs[] = phrase('package', CAPITALIZE).' '.phrase('removed');
    }
  }
  
  if(isset($change_vars['catering_id']) && $change_vars['catering_id'] != $resObj->catering_id) { // change catering
    $new_catering_id = $change_vars['catering_id'];
    $changes['catering_id'] = $new_catering_id;
    if($new_catering_id) {
      $productObj = get_object('product', $new_catering_id);
      $change_msgs[] = phrase('catering', CAPITALIZE).' '.phrase('changed_to').product_print($productObj);
    } else {
      $change_msgs[] = phrase('catering', CAPITALIZE).' '.phrase('removed');
    }
  }

  if($new_ng = $change_vars['ng']) { // change number of guests
    if($new_ng != $resObj->num_guests) {
      $changes['num_guests'] = $new_ng;
      $change_msgs[] = phrase('num_guests', CAPITALIZE).' '.phrase('changed_to', LOWERCASE).' '.$new_ng;
    }

  } elseif($new_in = $change_vars['in']) { // change checkin/out
    $new_nh = $change_vars['nh'];
    $new_time = $change_vars['time']; 
    if($new_nh || $new_time) { // change date/time (hourly)
      $hourly = true;
      list($new_in, $new_out) = date_time_hours2in_out($new_in, $new_time, $new_nh);
    } else {    
      $new_out = $change_vars['out'];
    }
    $new_in_str   = sql2human($new_in, array('show_time' => $hourly));
    $new_out_str  = sql2human($new_out, array('show_time' => $hourly));

    $date_changes = array();

    $in_changed = $hourly ? $new_in != $resObj->checkin : strip_time($new_in) != strip_time($resObj->checkin);
    $out_changed = $hourly ? $new_out != $resObj->checkout : strip_time($new_out) != strip_time($resObj->checkout);
    
    if($in_changed) { // change checkin
      $changes['checkin'] = $newResObj->checkin = $new_in; // preserve checkin time?
      $date_changes[] = phrase('checkin', CAPITALIZE).' '.phrase('changed_to', LOWERCASE).' '.$new_in_str;
    }      

    if($out_changed) { // change checkout
      $changes['checkout'] = $newResObj->checkout = $new_out; // preserve checkout time?
      $date_changes[] = phrase('checkout', CAPITALIZE).' '.phrase('changed_to', LOWERCASE).' '.$new_out_str;
    }

    if($hourly) {
      $new_period = $changes['num_hours'] = $new_nh ?: period_diff($new_in, $new_out, 'h');;
    } else {
      $new_period = $changes['num_days'] = period_diff($new_in, $new_out, 'd');
    }

    // todo: allow change for under min_stay if reserved, but add fee, or inform they have to pay for full stay
    if($aptObj->min_stay && $new_period < $aptObj->min_stay) {
      $errors[] = phrase('minimum_stay', CAPITALIZE).' = '.$aptObj->min_stay.' '.($hourly ? phrase('hours') : phrase('days'));
    } elseif($aptObj->max_stay && $new_days > $aptObj->max_stay) {
      $errors[] = phrase('maximum_stay', CAPITALIZE).' = '.$aptObj->max_stay.' '.($hourly ? phrase('hours') : phrase('days'));
    } elseif(!get_apartment_availability($aptObj->id, $new_in, $new_out, $res_id)) { // check availability
      $errors[] = phrase('sorry', CAPITALIZE).": $aptObj->name ".phrase('is_not_available', LOWERCASE).' '.$new_in_str.' - '.$new_out_str;
      $cancel_url = get_page_link($_SESSION['page'])."?rid=$resObj->res_id&pin=".myhash($resObj->booking_pin); // assuming request/book: todo, allow process
      $messages[] = html_link($cancel_url, phrase('cancel', CAPITALIZE));
      if($avail_apt_ids = available_apartments($new_in, $new_out, $property_type, $resObj->city_id, '', STATUS_PROPOSED, $resObj->res_id, $resObj->num_guests)) { // show apartments that ARE available
        $avail_str  = html_subheader(phrase('available', CAPITALIZE).' '.phrase('properties', LOWERCASE).':');
        $avail_str .= print_available_apartments($resObj, $avail_apt_ids, $changes);
        $messages[] = $avail_str;
        $pick_property = true;        
      } else { // no apts available
        $errors[] = "No available $type_str $in_str - $out_str";
      }
    } else { // apartment is available for the new dates
      if(count($date_changes)) $change_msgs[] = implode(html_break(), $date_changes);    
    }
    
  } 
  
  // update and output message    
  if(!$errors && $change_msgs) {
    list($res_id, $errors) = update_object('reservation', $changes, $res_id);
    if(!$errors) {
      $messages = $messages ? array_merge($messages, $change_msgs) : $change_msgs;
      $resObj = get_object('reservation', $res_id); // get fresh object
    }
  } else {
    $messages[] = $errors ? '' : phrase('no_change', CAPITALIZE);
  }
  return array($resObj, $errors, $messages, $pick_property);
}

// form used in site_page_status
function print_process_form($form, $hidden_fields, $action='', $submit='') {
  if($action) {
    $hidden_fields['act'] = $action;
    if(!$submit) $form .= form_input(url_safe($action), 'submit', phrase($action, CAPITALIZE)); // add default submit button
  }
  if($submit) $form .= $submit; // add custom submit button 
  return html_form(array2form($hidden_fields).$form);
}


function message_form($guest_name, $resObj) {
  $message_form = html_subheader(phrase('message_for', CAPITALIZE).' '.$guest_name.' ('.phrase('optional').')');
  $message_form .= html_div("A mail will be automatically generated and sent to the guest. If you would like to add text to this email, fill in the text box below and click 'Send'. Otherwise, click 'Yes' above." ,'message'); // todo: i18n
  $message_form .= form_text('message', $resObj->message, array('rows' => 5, 'cols' => 52));
  // $message_form .= form_input('submit', 'submit', ' Send ');
  return $message_form;
}

function print_confirmation($parent_type, $parent_id_or_obj, $options=NULL) {
  if(!$parent_type) return "print_confirmation: missing type";
  $obj = any2obj($parent_type, $parent_id_or_obj);
  if(!$obj) return "print_confirmation: missing object";
  $inv_id = $options['inv_id'];

  $name = get_name_string($obj) ?: phrase('customer', CAPITALIZE);
  $booking_pin = $obj->booking_pin;
  $email       = $obj->email_address;
  
  $status_url  = public_action_link($obj, $parent_type);
  $status_page = html_link($status_url, phrase('status', CAPITALIZE));

  if(!$booking_id) $booking_id = $res_id;

  switch($parent_type) {
  case 'reservation':
    $reserved = $obj->status >= STATUS_RESERVED;    
    $type = $reserved ? 'reservation' : 'request';
    $last_sentence = $reserved ? phrase('your_reservation_is_confirmed', CAPITALIZE) : phrase('we_will_get_back_to_you_shortly', CAPITALIZE); // phrase('We will get back to you within 24 hours.');
    $details = print_array_assoc(reservation_details($obj, USER_LEVEL_GUEST), 'left').html_break();
    $invObj = $inv_id ? get_object('invoice', $inv_id) : find_booking_invoice($obj->res_id); 
    
    break;
  case 'payment_order':
    $paid = $obj->status >= ORDER_STATUS_PAID;    
    $type = 'order';
    $action = $paid ? phrase('reservation', LOWERCASE) : phrase('request', LOWERCASE);  
    $last_sentence = phrase('thank_you_for_your_order', CAPITALIZE);
    $details = print_array_assoc(order_details($obj, USER_LEVEL_GUEST), 'left').html_break();
    $invObj = $inv_id ? get_object('invoice', $inv_id) : find_order_invoice($order_id); 
    // $details .= print_array_assoc(order_details($obj, USER_LEVEL_GUEST), 'left').html_break(); // print items
    break;
  default:
    return "print_confirmation: unsupported type $parent_type";
  }


  // begin output  
  $output  = html_header(phrase('dear', CAPITALIZE)." $name,", 2).html_break();
  $output .= phrase('thank_you_for_your', CAPITALIZE).' '.phrase($type).'. ';
  $output .= phrase('an_email_has_been_sent_to', CAPITALIZE).' '.$email.'. '.html_break();
  $output .= $last_sentence.'.'.html_break(2);
  $output .= phrase('click_here_for', CAPITALIZE).' '.$status_page.'. '.html_break(2);
  
  $output .= html_subheader(phrase($type, CAPITALIZE));
  $output .= $details;
  
  //$output .= "<b>invid=$inv_id</b><br>";
  if($invObj) {    
    $payment_details = payment_details($invObj, USER_LEVEL_GUEST);
    $output .= html_subheader(phrase('payment_details', CAPITALIZE));
    $output .= $payment_details.html_break();    
  }
  return $output;  
}


function order_confirmation($orderObj_or_id, $options=NULL) {
  $orderObj = any2obj('payment_order', $orderObj_or_id);
  $order_id = $orderObj->id;
  return print_confirmation('payment_order', $orderObj, $options);
}

// prints confirmation (thank you screen)
function reservation_confirmation($resObj_or_id, $options=NULL) {
  $resObj = any2obj('reservation', $resObj_or_id);
  $res_id = $resObj->res_id;
  if(!$resObj) return "Could not find reservation $res_id";
  
  $_SESSION['new_booking_rid'] = 0; // clear
  
  return print_confirmation('reservation', $resObj, $options);  
}

// prints form used for open enquiries (longstay/realestate)
function print_enquiry_form($aptObj, $inputVars, $enqObj='', $message='') {

  global $u_id;  
	$formname	      =	'searchbar';
	$apt_name	      =	$aptObj->name	?	$aptObj->name	:	$aptObj->shortname;
  $city_id        = $aptObj->city_id;
  $property_type  = $aptObj->type;
	$button_name    = phrase('send_request',	CAPITALIZE);

  list($searchObj, $filters) = get_filter_string($inputVars);
    
  // begin output
  $form = $message;

  $form .= form_input('referer',      'hidden', $referer);
  $form .= form_input('apt_id',       'hidden', $aptObj->id);
  $form .= form_input('aff_id',       'hidden', $_SESSION['aid']);
  $form .= form_input('process_type', 'hidden', $property_type);
  $form .= form_input('start',        'hidden', $inputVars['start']);
  $form .= form_input('limit',        'hidden', $inputVars['limit']);
		
	$form .= obj2form($searchObj);	// form	vars

	if($enqObj)	{
	  $addrObj = $enqObj;
    $form .= form_input('enq_id',     'text', $enqObj->id);
    $form .= form_input('address_id', 'text', $enqObj->address_id);
    $form .= form_input('search_id',  'text', $enqObj->property_search_id);
  } elseif($ses_enq_id = $_SESSION['enquiry_id'])	{
    $enqObj = get_object('enquiry', $ses_enq_id);    
    if(!$inputVars['extra_info'])     $inputVars['extra_info']      = $enqObj->extra_info;
    $addrObj = $enqObj;
	} elseif($u_id) {
	  $addrObj	=	get_object('user', $u_id);
	} else {
	  $addrObj = array2obj($inputVars); // use input vars
	}  
	$form .= html_subheader(phrase("your_details", CAPITALIZE));  	
  $fieldset_name = name_fields($addrObj, array('confirm_email' => $addrObj->email_address));
  $form .= html_fieldset($fieldset_name, 'fieldset_top');

	$form .= print_simple_address_form($addrObj);
	
	$form .= "<div id='calendar'></div>";
  
  list($searchObj, $filters) = get_filter_string(obj2array($enqObj));
	$form .= html_subheader(phrase("your_search", CAPITALIZE));  
	if($filters) $form .= "<ul>".$filters."</ul>";


  // todo: fix	
	//$form .= print_searchbar($property_type, $city_id, $formname,	$_SESSION['narrow_design'],	false, false);	



	// if(show_property_alert(array2obj($inputVars))) {
	// 	echo("<p><b>".phrase('property_alert', CAPITALIZE).":</b><br>");
	// 	echo("<input type='checkbox' name='alert'	>");
	// 	echo("&nbsp;&nbsp;".phrase('property_alert_helptext',	CAPITALIZE));
	// }

	$form .= html_subheader(phrase("extra_information", CAPITALIZE));  

	//$form .= print_ref_form($enqObj);
	$form .= '<b>'.phrase('questions_or_comments', CAPITALIZE).'</b><br>';
	$form .= form_text('extra_info', $inputVars['extra_info']);
  $form .= '<p>'.form_input('enquire', 'submit', $button_name).'</p>';

  return html_secure_form($form, $formname);  

}

function invoice_list($objs, $ref_name='', $ref_link='', $show_actions=false, $show_payment_info=false, $show_export=false) {
  $header_row = array('Created', 'Modified', 'Invoice Date', 'Due date', 'Start date', 'End date', 'Invoice#', 'Status', 'Apartment', 'Guest', 'Total', 'Paid', 'Balance');
  if($show_payment_info) $header_row = array_merge($header_row, array('Payments', 'Fees', 'Net', 'Withdrawn', 'Outstanding', 'Notes', ''));
  if($show_actions) array_unshift($header_row, '');
  if($show_export) {
    $header_row[] = '';
    $qs = array2qs($_GET ?: $_POST);
  }

  $currencies = array();
  foreach($objs as $obj) {
    if(!$cur) $cur = get_currency_symbol($obj->currency, true);
    if($show_actions) $actions = recordset_buttons($obj_type, $obj, '', '', $cal_name, $cal_link);
    $resObj = get_object('reservation', $obj->res_id);
    $guest_name = get_name_string($resObj);
    $row = array(sql2human($obj->created), sql2human($obj->timestamp), sql2human($obj->invoice_date), sql2human($obj->duedate), sql2human($obj->start_date), sql2human($obj->end_date), $obj->invoice_id ?: $obj->id, get_object('inv_status', $obj->status, 'name'), get_object('property', $obj->apt_id, 'name'), $guest_name,
                    $obj->total, $obj->paid, $obj->balance);

    if($show_payment_info) {
      $paymentObjs = find_objects('payment', array('inv_id' => $obj->id, 'payment_status' => array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED)));
      $pcount = count($paymentObjs);
      $payment_list = print_payments($paymentObjs, 0);
      $tip = $pcount ? tooltip('ptip_'.$obj->id, $payment_list) : '';
      $p_total = $p_fees = $p_net = $p_sent = $p_outstanding = $sum_pcurcount = array();
      foreach($paymentObjs as $paymentObj) {
        list($p_currency, $p_amount) = payment_currency_amount($paymentObj);
        $currencies[$p_currency] = 1;
        $pay_cur = get_currency_symbol($p_currency, true);
        list($w_currency, $gross) = payment_net_outstanding($paymentObj);
        if($gross && $w_currency != $p_currency) die("payment currency ($p_currency) != withdrawal currency ($w_currency): payment $paymentObj->id. Contact the administrator.");
        // echo("Net outstanding for $paymentObj->id = $gross<br>");
        $p_total[$p_currency] += $p_amount; // always in invoice currency
        $p_fees[$p_currency]  += $paymentObj->payment_fee;
        $p_net[$p_currency]   += $paymentObj->payment_net;
        $p_sent[$p_currency]  += $paymentObj->withdrawal_amount;
        $p_outstanding[$p_currency] += $gross;
        $pcurcount[$p_currency] ++;
        if($gross > 0) $p_ids[$p_currency][] = $paymentObj->id;
      }
      $row = array_merge($row, array(print_mulitple_currencies($p_total).($pcount ? " ($pcount)" : '').$tip, print_mulitple_currencies($p_fees), print_mulitple_currencies($p_net), print_mulitple_currencies($p_sent), print_mulitple_currencies($p_outstanding), $obj->notes, ''));
    }
    if($show_actions) array_unshift($row, $actions);
    if($show_export) $row[] = html_link("/admin/scripts/reeleezee.php?$qs&id=$obj->id&send=1", 'Send'); 
    $rows[] = $row;
    
    $sum_total   += $obj->total;
    $sum_paid    += $obj->paid;
    $sum_balance += $obj->balance;
    $sum_pcount  += $pcount;
    // dump($currencies);
    foreach(array_keys($currencies) as $thiscur) {
      $sum_payments[$thiscur]     += $p_total[$thiscur];
      $sum_fees[$thiscur]         += $p_fees[$thiscur];
      $sum_net[$thiscur]          += $p_net[$thiscur];
      $sum_withdrawn[$thiscur]    += $p_sent[$thiscur];
      $sum_outstanding[$thiscur]  += $p_outstanding[$thiscur];
      $sum_pcurcount[$thiscur]    += $pcurcount[$thiscur];
    }
  }

  // withdraw links for each currency
  foreach(array_keys($currencies) as $thiscur) {
    $sum_pcurcount[$thiscur] = " ($sum_pcurcount[$thiscur])";
    if(count($p_ids[$thiscur])) $w_links[$thiscur] = ' '.html_image_link("/admin/withdraw.php?currency=$thiscur&pids=".implode(',', $p_ids[$thiscur]), '/images/icons/withdraw_tiny.png',
                                   phrase('withdraw', CAPITALIZE).' '.formatFloat($sum_outstanding[$thiscur], FORMAT_IF_NEEDED, get_currency_symbol($cur, true)));
  }

  if(count($rows)) {
    $row = array(html_b(count($objs).' invoices'), '', '', '', '', '', '', '',html_b(formatFloat($sum_total, FORMAT_NEVER, $cur)), html_b(formatFloat($sum_paid, FORMAT_NEVER, $cur)), html_b(formatFloat($sum_balance, FORMAT_NEVER, $cur)));
    if($show_payment_info) $row = array_merge($row, array(print_mulitple_currencies($sum_payments, true, FORMAT_NEVER, $sum_pcurcount).html_break().html_b("$sum_pcount payments"), print_mulitple_currencies($sum_fees, true, FORMAT_NEVER), print_mulitple_currencies($sum_net, true, FORMAT_NEVER), print_mulitple_currencies($sum_withdrawn, true, FORMAT_NEVER), print_mulitple_currencies($sum_outstanding, true, FORMAT_NEVER, $w_links)));
    $rows[] = $row; 
    array_unshift($rows, $header_row);
  }
  return $rows;
}
          
// returns printable rows from list of events
// todo: if all event types are the same, hide type, otherwise show
function event_list($events, $ref_name='', $ref_link='', $show_actions=false) {
  // $header_row = array('Date', 'Apartment', 'Guest', 'Type', 'Status', 'Address', '# Guests', 'Alert', 'Notes', ''); 
  $header_row = array('Date', 'Address', 'Guest', '# Guests', 'Alert', 'Notes', ''); 
  if(isset($_GET['print'])) $show_actions = false;
  if($show_actions) $header_row[] = '';
  $rows = array();
  foreach($events as $obj) {
    $apt_name = $obj->res_id ? get_object('property', $obj->apt_id, 'name') : '?';
    $num_guests = $obj->res_id ? get_object('reservation', $obj->res_id, 'num_guests') : '?';
    $row = array(sql2human($obj->start_time, array('show_weekday' => true, 'show_time' => true)), $obj->start_address, $obj->customer_name, $num_guests, $obj->alert, $obj->notes);
    if($show_actions) array_unshift($row, recordset_buttons('event', $obj, '', '', $ref_name, $ref_link)); // recordset_buttons_inline($enq_type, $id, $fields, $options)
    $rows[] = $row;
  }
  if(count($rows)) array_unshift($rows, $header_row);
  return $rows;
}

// action links
function enq_actions($enq_type, $id, $base_link, $user_level=USER_LEVEL_GUEST, $compact=false) {
  $actions = array(); 
  $obj = get_object($enq_type, $id);
  
  // which action links do we show
  $ptype = $obj->property_type;
  $apt_id = $obj->apt_id;
  if($apt_id && $ptype) $aptObj = get_object($ptype, $apt_id);

  switch($enq_type) {  
    case 'reservation':
      $resObj = $obj;      
      $show_cancel = $obj->checkin >= today();
      switch($resObj->status) {
        case STATUS_UNCONFIRMED:
          $req_name = phrase('request');
          if($user_level == USER_LEVEL_GUEST) {
            // $actions['cancel'] = phrase('cancel', CAPITALIZE)." ".phrase('request', CAPITALIZE);      
            $searchObj = res2searchObj($resObj);
            $req_link = print_request_link($aptObj, $searchObj, array('link_only' => true));
            $action_str .= html_link($req_link."&pin=".myhash($resObj->booking_pin), phrase('edit', CAPITALIZE)." $req_name").html_break();
            
            if(direct_bookable($aptObj, $resObj)) $actions['book_now'] = phrase('book_now', CAPITALIZE); // we can book this property directly
          } elseif($user_level >= USER_LEVEL_OWNER && $resObj->apt_id) {
            $actions['decline'] = phrase('decline', CAPITALIZE)." $req_name";      
            $actions['accept']  = phrase('accept', CAPITALIZE)." $req_name";      
            $actions['resend']  = phrase('send', CAPITALIZE)." ".phrase('mail', CAPITALIZE);      
          }      
          break;
        case STATUS_PROPOSED:
          $req_name = phrase('proposal', CAPITALIZE);
          if($user_level == USER_LEVEL_GUEST) {
            //$actions['cancel'] = phrase('cancel', CAPITALIZE)." ".phrase('reservation', CAPITALIZE);      
            $actions['pay'] = phrase('accept', CAPITALIZE)." ".phrase('offer', CAPITALIZE);      
            $actions['reject']  = phrase('decline', CAPITALIZE)." ".phrase('offer', CAPITALIZE);
            $show_cancel = false;
          } elseif($user_level >= USER_LEVEL_OWNER) {
            $actions['resend']  = phrase('send', CAPITALIZE)." ".phrase('proposal', CAPITALIZE);      
          }      
          break;
        case STATUS_RESERVED:
          $req_name = phrase('reservation', CAPITALIZE);
          if($user_level == USER_LEVEL_GUEST) {
          } elseif($user_level >= USER_LEVEL_OWNER) {
            $actions['resend']  = phrase('send', CAPITALIZE)." ".phrase('confirmation', CAPITALIZE);      
          }          
          break;
        case STATUS_COMPLETE:
          $req_name = phrase('reservation', CAPITALIZE);
          if($user_level == USER_LEVEL_GUEST) {
          } elseif($user_level >= USER_LEVEL_OWNER) {
            $actions['resend']  = phrase('send', CAPITALIZE)." ".phrase('confirmation', CAPITALIZE);      
          }          
          break;
      }
      if($user_level >= USER_LEVEL_OWNER) {
        if($paid = get_reservation_paid($resObj)) {
          if($paid != $resObj->paid) $actions['update_payments']  = phrase('update', CAPITALIZE)." ".phrase('payments', CAPITALIZE);
        }
        $invObjs = find_reservation_invoices($resObj->res_id);
        if($invObjs) { // && !invoices_locked($invObjs)) {
          $actions['recalculate_invoice'] = phrase('recalculate', CAPITALIZE)." ".phrase('invoices', CAPITALIZE);
        }
      }
      //$actions['correspond']  = phrase('send', CAPITALIZE)." ".phrase('email', CAPITALIZE);          
      if($show_cancel && $resObj->status >= STATUS_UNCONFIRMED) $actions['cancel'] = phrase('cancel', CAPITALIZE)." ".$req_name;      
      if($user_level >= USER_LEVEL_STAFF) $actions['add_invoice'] = phrase('add', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
      if(!$compact) $actions['view'] = phrase('view', CAPITALIZE)." $req_name";      
      break;

    case 'invoice': 
      $invObj = $obj;
      $resObj = $obj->res_id ? get_object('reservation', $obj->res_id) : NULL;
      // if($invObj->status == INV_STATUS_DRAFT) 
      if($user_level >= USER_LEVEL_STAFF) {
        if($le_id = $invObj->legal_entity_id) $reeleezee_username = get_object('legal_entity', $le_id, 'reeleezee_username');
        $actions['add_invoice'] = phrase('add', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
        $actions['copy_invoice'] = phrase('copy', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
        if(!$invObj->invoice_id && $invObj->legal_entity_id) $actions['set_invoice_id'] = phrase('set', CAPITALIZE)." ".phrase('invoice', CAPITALIZE)." ".phrase('id', CAPITALIZE);
        if($reeleezee_username) $actions['export_invoice'] = phrase('export', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
        if($invObj->res_id && $invObj->type == INV_TYPE_BOOKING && !invoice_locked($invObj)) $actions['recalculate_invoice'] = phrase('recalculate', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
        if($invObj->status == INV_STATUS_DRAFT) $actions['finalize_invoice'] = phrase('finalize', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
        $actions['send_inv'] = phrase('send', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
        $actions['recalculate_invoice'] = phrase('recalculate', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
      }
      if($invObj->balance > 0) {
        if($user_level >= USER_LEVEL_OWNER) {
          $actions['receive'] = phrase('receive', CAPITALIZE)." ".phrase('payment', CAPITALIZE);      
          $actions['pay']     = phrase('charge', CAPITALIZE)." ".phrase('card', CAPITALIZE);
        } else {
          $actions['pay']     = phrase('pay', CAPITALIZE)." ".phrase('invoice', CAPITALIZE);
        }
      }
      break;    
      if(!$compact) $actions['view'] = phrase('view', CAPITALIZE)." ".phrase('invoice');      
  }
  unset($actions[$action]); // don't show current action
  foreach($actions as $act_key => $act_name) {
    $action_str .= html_link(replace_uri_var($base_link,"act", $act_key), $act_name).html_break();
  }
  if(!$compact) {
    // hard coded custom links
    $expires_res = 90;  //  no reviews allowed more than 90 days after checkout
    $expires_enq = 180;  //  no reviews allowed after 180 days of enquiry creation

    switch($enq_type) {  
      case 'reservation': 
        if($resObj->email_address) $action_str .= html_link(replace_uri_var($base_link,"act", 'edit').'&amp;section=chat#chat', phrase('send', CAPITALIZE).' '.phrase('correspondence')).html_break(); // cheesy hack for chat link
        if($user_level >= USER_LEVEL_OWNER)  $action_str .= html_link(public_action_link($resObj, 'reservation', array('user_level' => USER_LEVEL_GUEST)), phrase('guest', CAPITALIZE).' '.phrase('view', CAPITALIZE, array('class' => 'non-html')).html_break()); // cheesy hack for chat link
        if($user_level >= USER_LEVEL_STAFF)  $action_str .= html_link(action_link('reservation', $resObj,  array('user_level' => USER_LEVEL_OWNER)), phrase('host', CAPITALIZE).' '.phrase('view', CAPITALIZE).html_break()); // cheesy hack for chat link
        
        // feedback link
        $feedbackObj = find_object('feedback', array('parent_type' => $enq_type, 'parent_id' => $id));
        //dump($feedbackObj);
        if($user_level == USER_LEVEL_GUEST) {
          if($feedbackObj) {
            $feedback_action = 'view';
          } else {
            $today = today();            
            $feedback_allowed = (($obj->checkout > $today) || 
                                 (add_day($obj->checkout, $expires_res) < $today) ||
                                 ($obj->status == STATUS_BLOCKED) ||
                                 ($obj->status < STATUS_RESERVED)) ? false : true;
            // echo("allowed = $feedback_allowed");
            $feedback_action = $feedback_allowed ? 'send' : '';
          }
        } else {
          $feedback_action = $feedbackObj ? 'view' : '';
        }        
        if($feedback_action) {
          $feedback_link = public_action_link($obj, $enq_type, array('user_level' => $user_level, 'page_type' => PAGE_FEEDBACK));        
          $action_str .= html_link($feedback_link, phrase($feedback_action, CAPITALIZE).' '.phrase('feedback', LOWERCASE)).html_break();
        }
        break;
      case 'invoice': 
        if($rid = $invObj->res_id) {
          $resObj = get_object('reservation', $rid);
          $action_str .= html_link(action_link('reservation', $resObj, array('user_level' => $user_level)), phrase('view', CAPITALIZE).' '.phrase('reservation', CAPITALIZE).html_break()); 
        }
        if($invObj->type == INV_TYPE_BLOCK && ($block_id = $invObj->block_id)) {
          $blockObj = get_object('block', $block_id);
          $action_str .= html_link(action_link('block', $blockObj, array('user_level' => $user_level)), phrase('view', CAPITALIZE).' '.phrase('block', CAPITALIZE).html_break()); 
        }
        break;
    }
    global $u_hostid;
    if($user_level >= USER_LEVEL_STAFF || $user_level == USER_LEVEL_OWNER && $obj->host_id == $u_hostid) { 
      if($cal_link = calendar_link('reservation', $resObj)) $action_str .= html_link($cal_link, phrase('calendar', CAPITALIZE)).html_break(); // add calendar link
      //if(auth_action($enq_type, 'edit')) $action_str .= html_link(get_object_link($enq_type, $id, 'edit', '', '','', urlencode($page_title), urlencode($base_link)), phrase('back-end', CAPITALIZE).' '.phrase('edit')).html_break(); // add back-end link
      //if(auth_action($enq_type, 'view')) $action_str .= html_link(get_object_link($enq_type, $id, 'view', '', '','', urlencode($page_title), urlencode($base_link)), phrase('back-end', CAPITALIZE).' '.phrase('view')).html_break(); // add back-end link
      $action_str .= recordset_buttons_inline($enq_type, $id, $fields, $options);
    }
  }  
  //$action_str .= "ul=$user_level";  
  return $action_str;
}

function ticket_guest_list($id, $user_level=USER_LEVEL_GUEST) {
  $width        = $_SESSION['outer_width']; // total screen availalble
  $eventObj = get_object('event', $id) ?: die("Event not found");
  
  $resSearch['event_id'] = $id;
  $resSearch['status'] = array('>=', STATUS_RESERVED);
  
  $resObjs = find_objects('reservation', $resSearch);
  if(!count($resObjs)) {
    $errors[] = "No tickets sold for this event";
  } else {
    $header = array('Actions', 'Name', 'Count', 'Paid', 'Notes');
    $rows = array();
    $rows[] = $header;
    foreach($resObjs as $resObj) {
      $row = array();
      $name = get_name_string($resObj);
      $status_link = action_link('reservation', $resObj, array('user_level' => $user_level));
      if($invObj = find_booking_invoice($resObj->res_id)) {
        $link_id = 'res_'.$resObj->res_id.';inv_'.$invObj->id;
        $row[] = html_image_link('#', '/images/icons/shopping-cart_tiny.png', 'Add to cart', array('class' => 'cart', 'id' => $link_id));
      } else {
        $row[] = html_span('X', 'red');
      }
      $row[] = html_link($status_link, $name);
      $row[] = $resObj->num_guests;
      $row[] = $resObj->paid;
      $row[] = $resObj->extra_info;
      $rows[] = $row;
    }
    $list .= html_table($rows);
  }
  //echo($output);
  $start_str = sql2human($eventObj->start_time, array('show_weekday' => true, 'show_time' => true));
  $event_name = $eventObj->name;
  $event_str = "$aptObj->name $event_name: $start_str";
  $agent_str = $agent_id ? get_object('agent', $agent_id, 'name').': ' : '';
  
  $output = html_subheader($event_str);
  $output .= $list;
  $output .= product_dialog();

  return $output; 
  
}

// place holder for request form dialog (
function request_form_dialog($options) {
  global $__header;
  $__header['scripts'][] = '/afr/bower/jquery-validate/dist/jquery.validate.min.js';
  if($_SESSION['lang'] != 'en') $__header['scripts'][] = '/afr/bower/jquery-validate/localization/messages_'.$_SESSION['lang'].'.js';
  if($options['show_map']) {
    //$__header['scripts'][] = "//maps.google.com/maps/api/js?sensor=false";
    $__header['scripts'][] = GMAP_API; 
    $__header['scripts'][] = "/jquery/gmap/jquery.ui.map.full.min.js";
    //$__header['scripts'][] = "/jquery/gmap/gmap3.min.js";
    $__header['scripts'][] = "/afr/js/afr_gmap.js";
  }
  return html_div('', '', "id='request_form_dialog'");
}

// place holder for request form dialog (
function package_form_dialog($options) {
  global $__header;
  //$__header['scripts'][] = 'http://ajax.aspnetcdn.com/ajax/jquery.validate/1.9/jquery.validate.min.js';
  $__header['scripts'][] = '/afr/bower/jquery-validate/dist/jquery.validate.min.js';
  if($_SESSION['lang'] != 'en') $__header['scripts'][] = '/afr/bower/jquery-validate/localization/messages_'.$_SESSION['lang'].'.js';
  if($options['show_map']) {
    //$__header['scripts'][] = "//maps.google.com/maps/api/js?sensor=false";
    $__header['scripts'][] = GMAP_API; 
    $__header['scripts'][] = "/jquery/gmap/jquery.ui.map.full.min.js";
    //$__header['scripts'][] = "/jquery/gmap/gmap3.min.js";
    $__header['scripts'][] = "/afr/js/afr_gmap.js";
  }
  return html_div('', '', "id='request_form_dialog'");
}

// max_guests, id, type
function request_form($site_id, $type, $id, $options = NULL) {
  
  if(!$site_id) return "request_form: missing site_id";
  if(!$type) return "request_form: missing type";
  if(!$id) return "request_form: missing id";
  $obj_type = $type;

  if(is_property_type($type)) {
    $ptype = $obj_type;
    $apt_id = $id;
    $boat_type = $ptype == 'boat' ? get_object($ptype, $apt_id, 'boat_type') : '';
    $min_guests  = 1;
    $max_guests =  get_object($ptype, $apt_id, 'max_guests');
    $rate_type = 'hour';    
    $handler = 'reservation-request';

  } elseif ($type == 'product') {
    $ptype = 'boat'; // hard coded for now
    $apt_id = 0;
    $productObj = get_object('product', $id);
    if($productObj->category_id == PRODUCT_CATEGORY_PACKAGE) {
      $package_id = $id;
      $min_guests  = get_object($type, $id, 'minimum_quantity') ?: 1;
      $max_guests  = 200; // or more? 
      $num_hours = $productObj->num_hours;
      $package_name = $productObj->name;
      $rate_type = uom2rate_type($productObj->unit);    
//      $handler = 'package-request';
      $handler = 'reservation-request';
    } else {
      return "request_form: $type $id is not a package ($productObj->name cat = $productObj->category_id) ";
    }
  }

  $obj->city_id = $city_id = $_SESSION['city_id'] ?: 1000; // todo: allow others
  $obj->apt_id  = $apt_id;
  $obj->property_type = $ptype;    
  
  //$siteObj = get_object('site', $site_id);
  //$aptObj = get_object($ptype, $apt_id);
  $len = 15;
  

  $fieldset = searchbar($ptype, $city_id, array('fields_only' => true, 'apt_id' => $apt_id, 'min_guests' => $min_guests, 'max_guests' => $max_guests));
  
  //$calparams['minDate'] = "'+0d'";
  //$calparams['maxDate'] = "'+12m'";
  switch($ptype) {
  case 'boat':
    // $fieldset[phrase('date', CAPITALIZE)] = datepicker('date', $calparams);  
    // $timeparams['startTime'] = "'08:00'";
    // $fieldset[phrase('time', CAPITALIZE)] = timepicker('time', $timeparams);
    // $fieldset[phrase('hours', CAPITALIZE)] = $num_hours ? $num_hours.form_input('num_hours', 'hidden', $num_hours) : form_select('num_hours', array('rows' => num_array(1.5, 12, 0.5)));
    // $fieldset[phrase('num_guests', CAPITALIZE)] = form_select('num_guests', array('rows' => num_array($min_guests, $max_guests)));
  
    if($boat_type != BOAT_TYPE_SELF_OPERATED) {
      //$fieldset[phrase('package', CAPITALIZE)] = $package_name ? $package_name.form_input('package_id', 'hidden', $package_id) : package_chooser($ptype, $apt_id, array('value' => $package_id));  
      $fieldset[phrase('catering', CAPITALIZE)] = catering_chooser($ptype, $apt_id, array('notip' => true));  
    }
    $locs = boarding_locs($obj);  
    $fieldset_boar = boarding_fields($obj, $locs, $options['show_map']);
    $fieldset = array_merge($fieldset, $fieldset_boar);
    break;
  case 'shortstay':    
    break;
  default:
    break;
  }

  $form .= html_fieldset($fieldset, 'fieldset_top').html_break();

  $input_options = array('size' => 15, 'max' => 50, 'class' => 'request_form_text_field'); 
  
  $fieldset = name_fields($addrObj, array('confirm_email' => $addrObj->email_address));
  $fieldset[phrase('phone', CAPITALIZE)] = form_input('cell_phone', 'text', '', $input_options);  
 
  $fieldset[phrase('message', CAPITALIZE)] = form_text('extra_info', '', array('rows' => 3, 'cols' => 60));

  //$form .= form_input('submit', 'submit', ' OK ');
  
  // $fieldset[''] = form_input('', 'text', 10);

  $form_name = 'request_form';
  $action = "/home/app_data.php?oper=$handler";

  // $on_save = "onclick=\"var result =  alert('result=' + result); return false;\"";
  $on_cancel = "onclick=\"close_parent_dialog('$form_name');return false;\"";    

  $form .= html_fieldset($fieldset, 'fieldset_top');

  // cancel / submit                                  
  $buttons[] = form_input('submit_request', 'submit', ' '.phrase('send', CAPITALIZE).' ',  array('js' => $on_save)); 
  $buttons[]= form_input('cancel_request', 'submit', ' '.phrase('cancel', CAPITALIZE).' ', array('js' => $on_cancel)); 
  $form .= implode(' ', $buttons);

  $errors = html_div('', 'error', "id='".$form_name."_errors'");
  $messages = html_div('', 'form_message', "id='".$form_name."_messages'");  

  $hidden_fields = array('site_id' => $site_id, 'apt_id' => $apt_id, 'obj_type' => $type, 'property_type' => $ptype, 'rate_type' => $rate_type);
  if($data = $options['data']) {
    //dump($data);                                 
    $hidden_fields = array_merge($hidden_fields, (array) $data);
  }
  
  $form = html_form($form.array2form($hidden_fields), $form_name, $action);
  $output = $debug.$errors.$messages.$form;
  $output .= boarding_maps($locs);

  $markers = gmap3_markers(array('markers' => $locs));

  // $output .= print_r($hidden_fields, true);

  $output .= html_script_inline('
   $("#request_form").validate({ 
	    rules: { 
	      first_name: "required", 
	      last_name:  "required", 
	      date_input: "required", 
	      time_input: "required", 
	      num_hours:  "required", 
	      cell_phone: {
          required: true, 
//          phone: true 
	      }, 
	      email_address: { 
          required: true, 
          email: true 
	      }, 
        email_address2: {
          email: true,
          equalTo: "#email_address"
        }
	    }, 
	    messages: { 
	      code: "Optional code" 
	    }, 
	    submitHandler: function(form) {
        submit_form("'.$form_name.'", "'.$action.'", {"reload": "0", "keepalive":1, "overwrite": 1 });
      }	    
	  });
    $("#show_boarding_map, #show_unboarding_map").click(function() {
      boarding_map_init($(this).attr("id"));
      return false;
    });
	  
	  '.$markers);  
  //echo($form);    
  
  return $output;
  //return html_div($output, 'visuallyhidden', "id='request_form_dialog'");
}

function package_chooser($ptype='', $apt_id='', $options='') {
  $productObjs = find_objects('product', array('category_id' => PRODUCT_CATEGORY_PACKAGE), array('sort' => 'name'));
  foreach($productObjs as $productObj) {
    $rows[$productObj->id] = product_print($productObj);
  }
  if(!$options['notip']) $tip = tooltip('package_tip', phrase('text_package'));
  return select_from_array('package_id', $rows, $options['value'], array('first' => phrase('none', CAPITALIZE), 'onchange' => $options['onchange']))." ".$tip;
}

function catering_chooser($ptype='', $apt_id='', $options='') {
  $productObjs = find_objects('product', array('category_id' => PRODUCT_CATEGORY_CATERING), array('sort' => 'name'));
  foreach($productObjs as $productObj) {
    $rows[$productObj->id] = product_print($productObj);
  }
  if(!$options['notip']) $tip = tooltip('catering_tip', phrase('text_catering'));
  return select_from_array('catering_id', $rows, $options['value'], array('first' => phrase('none', CAPITALIZE), 'onchange' => $options['onchange']))." ".$tip;
}

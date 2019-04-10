<?php


/**
todo:

way to edit texts online easily

deal with templates:
updated_account
account_invitation
account_upgrade_request
account_upgrade_confirmation
account_activation
user_register
forgot_password

reservation_expiration
not_available

*/

/** unsused - automatic comment added on  */
function account_mail($userObj_or_id, $options=[]) {
  global $__SITE, $__LANG;  
  $result = json_error_object("account mail: no mail sent");
  $userObj = get_user($userObj_or_id);  
  if(!$userObj) return json_error_object("Could not find message:".dump($userObj_or_id,true));

  $site_id = $userObj->site_id ?: $__SITE->id;
  $siteObj = get_object('site', $site_id);
  $template = 'mail/mail-account';
  $subject = mail_subject('account', $userObj);
  
  $user_name = get_user_name($userObj);
  $user_email = user_email($userObj, $user_name);
  
  $user_text = "Just testing...";
  
  if($user_text && $user_email) {
    $lang = pick_first(get_user_language($userObj->id), $__LANG, $siteObj->default_language, 'en');
    $mail_scope = ['user' => $userObj, 'receiver_role' => 'user', 'receiver_name' => $user_name ?: phrase('user', CAPITALIZE), 'receiver_id' => $userObj->id, 'main_message' => $message];  
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope]);      

    if($options['debug']) return ['to' => $user_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
    
    $mail_options = ['receiver_role' => 'user', 'parent_type' => 'user', 'parent_id' => $userObj->id, 'language' => $lang, 'debug' => $options['debug']];
    $result = send_mail($siteObj, $subject, $body, $user_email, $mail_options);
    if($result->error) print_log("Error sending account mail to staff: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }

  if($staff_text) {
    $lang = pick_first($siteObj->default_language, 'en');
    $mail_scope = ['user' => $userObj, 'receiver_role' => 'staff', 'receiver_name' => "{$siteObj->name} ".phrase('staff'), 'main_message' => $message];  
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope]);      

    if($options['debug']) return ['to' => $host_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
    
    $mail_options = ['receiver_role' => 'staff', 'parent_type' => 'user', 'parent_id' => $userObj->id, 'language' => $lang, 'debug' => $options['debug']];
    $result = send_mail($siteObj, $subject, $body, $site_email, $mail_options);
    if($result->error) print_log("Error sending account mail to staff: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }
  
  return $result;
}


/** coming soon...
function review_mail($resObj_or_id, $options=[]) {
  global $__SITE, $__LANG;  
  return $result;
}
*/

/** not yet impl. (not needed - can send manually from /invoice/ page ?) */
function invoice_mail($resObj_or_id, $options=[]) {
  global $__SITE;
  $guest_text = phrase('you have a new invoice from %s (attached)', CAPS, $__SITE->name);
}


/** not longer in use 
function message_mail($messageObj_or_id, $options=[]) {
  global $__SITE, $__LANG;  
  $result = json_error_object("message mail: no mail sent");
  $messageObj = any2obj('message', $messageObj_or_id);  
  if(!$messageObj) return json_error_object("Could not find message:".dump($messageObj_or_id,true));

  $site_id = $messageObj->site_id ?: $__SITE->id;
  $siteObj = get_object('site', $site_id);
  $site_email = $messageObj->site_id ? site_email($messageObj->site_id, true) : "info@{$siteObj->domain}";
  $sender = $replyto = $messageObj->email_address ?: $site_email;
  $lang = pick_first($__LANG, $siteObj->default_language, 'en');
  $template = 'mail/mail-messsage';
  $subject = mail_subject('message', $messageObj);
  $staff_name = "{$siteObj->name} ".phrase('staff');      
  $mail_scope = ['receiver_role' => 'staff', 'message' => $messageObj, 'receiver_name' => $staff_name,  'main_message' => $message];
  $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope]);      
  $mail_options = ['receiver_role' => 'site', 'parent_type' => 'message', 'replyto' => $replyto, 'parent_id' => $messageObj->id, 'language' => $lang, 'debug' => $options['debug']];
  $result = send_mail($siteObj, $subject, $body, $site_email, $mail_options);
  if($result->error) print_log("Error sending message mail to staff: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);  
  return $result;
}
*/

/** not finished ... */

function chat_mail($chatObj_or_id, $options=[]) {
  global $__SITE, $__LANG, $__USER;  
  $result = json_error_object("correspondence mail: no mail sent");
  
  $chatObj = any2obj('chat', $chatObj_or_id);  
  if(!$chatObj) return json_error_object("Could not find chat");

  $ts = date('D j F Y H:i', strtotime($chatObj->created));
  
  $receiver_id = $chatObj->receiver_id;
  $sender_id = $chatObj->sender_id;
  if($receiver_id == $__USER->id) return $result; // no need to send to self
  if(!$chatObj->notify) return $result;
  
  $chat_id = $chatObj->id;  
  $site_id = $chatObj->site_id ?: $__SITE->id;
  $siteObj = get_object('site', $site_id);
  $site_email = site_email($siteObj);
  
  $template = 'mail/mail-chat';
  
  $who = $chatObj->sender_id ? user_display_name($chatObj->sender_id) : phrase('an unknown user');
  $subject = "[{$siteObj->shortname}] ".phrase('you have a message from %s', CAPITALIZE, $who);

  // associate mail with parent of chatObj
  if($chatObj->parent_type && $chatObj->parent_id) {
    $parent_id = $chatObj->parent_id;
    $parent_type = $chatObj->parent_type;
  } else {
    $parent_id = $chatObj->id;
    $parent_type = 'chat';
  }    
  
  $sender_name = user_display_name($sender_id);
  $receiver_name = user_display_name($receiver_id);  
  $main_message = phrase('you have a message from %s', CAPITALIZE, $sender_name);
  $receiver_mail = get_user_email($receiver_id);

  $action = phrase('correspondence', CAPITALIZE);
  if($receiver_mail  && $__USER->id != $chatObj->receiver_id) {   
    $subject = "[{$siteObj->shortname}] {$action}: {$main_message}";
    $lang = pick_first(get_user_language($chatObj->receiver_id), $chatObj->language, $__LANG, $siteObj->default_language, 'en');
    $mail_options = ['parent_type' => $parent_type, 'parent_id' => $parent_id, 'language' => $lang, 'debug' => $options['debug']];  
    $mail_scope = ['receiver_name' => $receiver_name, 'sender_id' => $sender_id,  'receiver_id' => $receiver_id, 'chat_id' => $chatObj->id, 'main_message' => "{$main_message} {$ts}:"]; 
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'chat']);      
    $result = send_mail($siteObj, $subject, $body, $receiver_mail, $mail_options);
    if($result->error) print_log("Error sending chat mail to receiver: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);    
  }

  if($site_email) {
    $main_message = phrase('%s sent a message to %s', CAPITALIZE, $sender_name, $receiver_name);
    $subject = "[{$siteObj->shortname}] {$action}: {$main_message}";
    $staff_name = "{$siteObj->name} ".phrase('staff');    
    $mail_options = ['parent_type' => $parent_type, 'parent_id' => $parent_id, 'language' => $lang, 'debug' => $options['debug']];  
    $mail_scope = ['receiver_name' => $staff_name, 'sender_id' => $sender_id,  'receiver_id' => $receiver_id, 'chat_id' => $chatObj->id, 'main_message' => "{$main_message} {$ts}:"]; 
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'chat']);      
    $result = send_mail($siteObj, $subject, $body, $site_mail, $mail_options);
    if($result->error) print_log("Error sending chat mail to receiver: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);        
  }
  
  return $result;
}


function payment_mail($paymentObj_or_id, $options=[]) {
  global $__SITE, $__LANG;  
  $result = json_error_object("payment mail: no mail sent");
  
  $paymentObj = any2obj('payment', $paymentObj_or_id);  
  if(!$paymentObj) return json_error_object("Could not find payment:".dump($paymentObj_or_id,true));

  $site_id = $paymentObj->site_id ?: $__SITE->id;
  $siteObj = get_object('site', $site_id);

  $res_id = $paymentObj->res_id;
  $inv_id = $paymentObj->inv_id;
  $guest_id =  $paymentObj->guest_id;
  $host_id =  $paymentObj->host_id;

  $changes = isset($options['changes']) ? $options['changes'] : [];

  $template = $guest_text = $host_text = $staff_text = $guest_email = $host_email = $staff_email = $cc = $action = "";
  $head = $inv_text = $res_text = '';
      
  $status = $paymentObj->payment_status;  
  $type = $paymentObj->payment_type;  
  $verified = $paymentObj->verified;  
  $gateway_id = $paymentObj->gateway_id;
  
  $status_name = get_object('payment_status', $status, 'name');
  $type_name = get_object('payment_type', $paymentObj->payment_type, 'name');
  $v_str = $verified ? "verified" : "unverified";

  //$receipt = payment_receipt($paymentObj, $feeObj);
  $cur = currency2html($paymentObj->currency);
  $amount_str = "{$cur}$paymentObj->amount on ".sql2human($paymentObj->created, array('show_time' => true));

  $payment_str = phrase('payment', CAPITALIZE);
  
  $subject = "[{$siteObj->shortname}] $payment_str {$paymentObj->id}";
  $dear = phrase('dear', CAPITALIZE);
            
  $site_email = $paymentObj->site_id ? site_email($paymentObj->site_id, true) : "info@{$siteObj->domain}";
  $staff_email = $site_email;
  
  //$host_email = $paymentObj->host_id ? get_user_email($paymentObj->host_id) : '';
  //$guest_email = $paymentObj->guest_id ? get_user_email($paymentObj->guest_id) : '';

  $host_email = host_email($paymentObj);
  $guest_email = guest_email($paymentObj);

  $guest_name = $paymentObj->guest_id ? get_user_name($paymentObj->guest_id) : '';

  if($res_id) { /** payment is for a booking */
    $resObj = get_object('reservation', $res_id);
    $guest_id =  $resObj->guest_id;
    $host_id  =  $resObj->host_id;
    $guest_name =  $resObj->guest_id ? get_user_name($resObj->guest_id) : get_name_string($resObj);
    
    $apt_id = $resObj->apt_id;
    $res_link = html_link(booking_link($resObj), phrase('booking')." {$res_id}");
    $apt_name = get_object('property', $apt_id, 'name') ?: phrase('rental')." {$apt_id}";
    $apt_link = html_link(property_geolink($apt_id), $apt_name);
    $period_str = print_time_range($resObj->checkin, $resObj->checkout);
    $res_text = phrase('the payment is for %s @ %s for %s', CAPITALIZE, $res_link, $apt_link, html_b($period_str));
    
    if(!$resObj->send_email) {
      $head = phrase('this mail was not sent to the guest (%s) because send email is turned off for the booking', CAPITALIZE, $guest_name);
      $guest_email = '';
    } elseif(!$guest_email && is_email($resObj->email_address)) {
      $guest_email = $resObj->email_address;
    }
    
    $booking_str = phrase('booking', CAPITALIZE);
    $subject .=  ": $booking_str $res_id ({$guest_name} @ {$apt_name} - {$period_str})";
  } elseif($inv_id) {
    
    $invObj = get_object('invoice', $inv_id);    
    $invoice_str = phrase('invoice', CAPITALIZE);
    $period_str = print_time_range($invObj->start_date, $invObj->end_date);
    $guest_name =  $invObj->guest_id ? get_user_name($invObj->guest_id) : get_name_string($invObj);
    $subject .=  ": $invoice_str $inv_id ({$inv_name} - {$period_str})";
  }

  
  if($inv_id = $paymentObj->inv_id) {
    $invoice_id = get_object('invoice', $inv_id, 'invoice_id') ?: $inv_id;
    $inv_link = html_link(invoice_link($inv_id), phrase('invoice')." {$invoice_id}");
    //$inv_text = phrase('click here to see the invoice for this payment: %s', CAPITALIZE, $inv_link);    
  }
  
  
  $template = 'mail/mail-invoice';
  $mail_options = ['parent_type' => 'payment', 'parent_id' => $paymentObj->id, 'language' => $lang, 'debug' => $options['debug']];
    
  if($guest_email) {
    $guest_text = phrase('thank you for your %s payment of %s.',CAPITALIZE, $type_name, $amount_str);
    $guest_text .= phrase('the status of your payment is %s', CAPITALIZE, html_b($status_name));  
    if($res_text) $guest_text = html_p($guest_text).html_p($res_text);
    if($inv_text) $guest_text = html_p($guest_text).html_p($inv_text);
    
    if(!$guest_name) $guest_name = phrase('guest', CAPITALIZE);

    $mail_scope = ['profile_id' => $paymentObj->host_id, 'receiver_role' => 'guest', 'receiver_name' => $guest_name, 'receiver_id' => $guest_id, 'payment_id' => $paymentObj->id, 'main_message' => $guest_text]; 
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'payment']);      
    
    $mail_options['language'] = get_user_language($paymentObj->guest_id) ?: $lang;
    $mail_options['receiver_id'] = $paymentObj->guest_id;
    $mail_options['receiver_role'] = 'guest';
    
    $result = send_mail($siteObj, $subject, $body, $guest_email, $mail_options);
    if($result->error) print_log("Error sending payment mail to guest: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }

  
  if($host_email && (!$resObj || !$resObj->corporate)) {
    if($head) $host_text .= html_p(html_b($head));
    $host_text .= html_p(phrase('you received a %s %s payment of %s.',CAPITALIZE, html_b($status_name), "{$v_str} {$type_name}", $amount_str));
    if($res_text) $host_text .= html_p($res_text);
    if($inv_text) $host_text = html_p($host_text).html_p($inv_text);
    if($pay_desc = $paymentObj->description) $host_text .= html_p(html_b(phrase('notes', CAPITALIZE)).": {$pay_desc}");
    
    $host_name = $paymentObj->host_id ? get_user_name($paymentObj->host_id) : phrase('host', CAPITALIZE);
    $mail_scope = ['profile_id' => $paymentObj->guest_id, 'receiver_role' => 'host', 'receiver_name' => $host_name, 'receiver_id' => $host_id, 'payment_id' => $paymentObj->id, 'main_message' => $host_text]; 
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'payment']);      
    
    $mail_options['language'] = get_user_language($paymentObj->host_id) ?: $lang;
    $mail_options['receiver_id'] = $paymentObj->host_id;
    $mail_options['receiver_role'] = 'host';
    
    $result = send_mail($siteObj, $subject, $body, $host_email, $mail_options);    
    if($result->error) print_log("Error sending payment mail to host: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }

  if($staff_email) {

    if($head) $host_text .= html_p(html_b($head));
    $staff_text .= html_p(phrase('a %s %s payment was made to %s for %s.',CAPITALIZE, html_b($status_name), "{$v_str} {$type_name}", $host_name, $amount_str));
    if($res_text) $staff_text .= html_p($res_text);
    if($inv_text) $staff_text = html_p($staff_text).html_p($inv_text);
    if($pay_desc = $paymentObj->description) $staff_text .= html_p(html_b(phrase('notes', CAPITALIZE)).": {$pay_desc}");  

    $staff_name = "{$siteObj->name} ".phrase('staff');    
    $mail_scope = ['profile_id' => $paymentObj->guest_id, 'receiver_role' => 'staff', 'receiver_name' => $staff_name, 'receiver_id' => 0, 'payment_id' => $paymentObj->id, 'main_message' => $staff_text]; 
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'payment']);      

    $mail_options['receiver_id'] = 0;
    $mail_options['receiver_role'] = 'staff';
    $mail_options['language'] = $lang;

    $result = send_mail($siteObj, $subject, $body, $staff_email, $mail_options);    
    if($result->error) print_log("Error sending payment mail to staff: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }

  return $result;
}
        
function rental_mail($aptObj_or_id, $options=[]) {
  global $__SITE, $__LANG;  
  $result = json_error_object("rental mail: no mail sent");
  
  $aptObj = any2obj('property', $aptObj_or_id);
  
  if(!$aptObj) return json_error_object("Could not find booking:".dump($aptObj_or_id,true));
  if($aptObj->source_id > 0) return $result;  /** no mails for imports */

  $site_id = $aptObj->site_id ?: $__SITE->id;
  $apt_id = $id = $aptObj->id;
  $siteObj = get_object('site', $site_id);

  $changes = isset($options['changes']) ? $options['changes'] : [];

  $template = $guest_text = $host_text = $staff_text = $guest_email = $host_email = $staff_email = $cc = $action = "";

  $rental_link = ''; 
  $loc_link = ''; 
  
  $new = isset($options['new']) && $options['new']; 
  $activated = $changes['active'] && $changes['active']['new'];
  
  if($new || $activated) {
    $action = $new ? 'added': 'activated';
    if($new) update_object('property', ['activation_timestamp' => now()], $apt_id);
    if(!$new) $host_text = phrase("your rental %s in %s was {$action}.", CAPITALIZE, html_link($rental_link, $aptObj->apartment), html_link($loc_link, $aptObj->address_city));
    $staff_text = phrase("the rental %s in %s was {$action}.", CAPITALIZE, html_link($rental_link, $aptObj->apartment), html_link($loc_link, $aptObj->address_city));
  }
  
  $template = 'mail/mail-rental';
  if($action) {
    
    $from = $siteObj->email_address ?: "info@{{$siteObj->domain}}";
    
    $rental_str = phrase('rental');
    $action_str = phrase($action);
    $subject = "[{$siteObj->shortname}] $rental_str {$aptObj->id} $action_str ({$aptObj->name} @ {$aptObj->address_city})";
    $dear = phrase('dear', CAPITALIZE);
              
    $site_email = $aptObj->site_id ? site_email($aptObj->site_id, true) : "info@{$siteObj->domain}";
    $staff_email = $site_email;
    $host_email = $aptObj->host_id ? get_user_email($aptObj->host_id, true) : '';

    $lang = pick_first($__LANG, $siteObj->default_language, 'en'); /** todo: language per user */
    
    $mail_options = ['parent_type' => 'property', 'parent_id' => $aptObj->id, 'language' => $lang, 'debug' => $options['debug']];
    if($host_email && $host_text) {
      
      $host_name = $aptObj->host_id ? get_user_name($aptObj->host_id, true) : phrase('host', CAPITALIZE);      
      $mail_scope = ['profile_id' => $aptObj->host_id, 'receiver_role' => 'host', 'receiver_name' => $host_name, 'receiver_id' => $host_id, 'apt_id' => $aptObj->id, 'main_message' => $host_text]; 
      $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'rental']);      

      if($options['debug']) return ['to' => $host_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
      
      $mail_options['language'] = get_user_language($aptObj->host_id) ?: $lang;
      $mail_options['receiver_id'] = $aptObj->host_id;
      $mail_options['receiver_role'] = 'host';
      
      $result = send_mail($siteObj, $subject, $body, $host_email, $mail_options);    
      if($result->error) print_log("Error sending rental mail to host: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
    }
  
    if($staff_email && $staff_text) {
      
      $staff_name = "{$siteObj->name} ".phrase('staff');    
      $mail_scope = ['profile_id' => $aptObj->host_id, 'receiver_role' => 'staff', 'receiver_name' => $staff_name, 'receiver_id' => 0, 'apt_id' => $aptObj->id, 'main_message' => $staff_text]; 
      $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'rental']);      

      $mail_options['receiver_id'] = 0;
      $mail_options['receiver_role'] = 'staff';
      $mail_options['language'] = $lang;

      if($options['debug']) return ['to' => $staff_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
      
      $result = send_mail($siteObj, $subject, $body, $staff_email, $mail_options);    
      if($result->error) print_log("Error sending rental mail to staff: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
    }
 
  }
  
  return $result;
}

function booking_review_mail($resObj_or_id, $options=[]) {
  return booking_mail($resObj_or_id, ['review' => 1]);
}

function booking_reminder_mail($resObj_or_id, $options=[]) {
  return booking_mail($resObj_or_id, ['reminder' => 1]);
}

function booking_mail($resObj_or_id, $options=[]) {
  global $__SITE, $__LANG;
  $resObj = any2obj('reservation', $resObj_or_id);
  if(!$resObj) return json_error_object("Could not find booking:".dump($resObj_or_id,true));
  $changes = isset($options['changes']) ? $options['changes'] : [];
  $review  = isset($options['review']) ? $options['review'] : 0;
  $reminder = isset($options['reminder']) ? $options['reminder'] : 0;
  $today = today();
  
  $result = json_error_object("booking mail: no mail sent");
  
  
  $site_id = $resObj->site_id ?: $__SITE->id;
  $siteObj = get_object('site', $site_id);
  
  $apt_id = $resObj->apt_id;
  if(!$apt_id) return json_error_object("booking_mail: missing rental ID");
  $apt_name = get_object('property', $apt_id, 'name');
  
  $res_id = $resObj->res_id;
  $status = $resObj->status;
  $template = $guest_text = $host_text = $staff_text = $guest_email = $host_email = $staff_email = $cc = $action = "";
  $guest_message = $host_message = $staff_message = '';
  
  $period = print_time_range($resObj->checkin, $resObj->checkout);
  $template = 'mail/mail-booking';
    
  $personal_message = '';
  
  /** booking changed */
  if($changes) {
    
    if(!in_array($status, [STATUS_RESERVED,STATUS_COMPLETE]) && $changes) return $result;
      
    $notify_on = array_flip(['num_guests', 'checkin', 'checkout', 'arrival_method', 'arrival_time']); /** duplicated in post_edit_reservation */
    $res_changes = array_intersect_key($changes, $notify_on);

    if(!$res_changes) return $result;
      
    /** move this to template mail/booking-changed */ 
    $who = changed_by($changes);      
    $mail_text = phrase('%s updated the booking', CAPITALIZE, $who).html_break();
    $action = 'changed';
    $items = [];
    foreach($changes as $change) {
      $items[] = $change['str']; // only show if loggable?
    }        
    $mail_text .= html_ul($items);
    $guest_text = $host_text = $staff_text = $mail_text;
    
  } elseif($review) { /** send reminder to review the booking */
    $exires = add_days($today, -1 * BOOKING_REVIEW_PERIOD);
    if($resObj->review_sent > 0) return json_error_object("Review reminder: already sent"); 
    if(!in_array($resObj->status, [STATUS_RESERVED, STATUS_COMPLETE])) return json_error_object("Review reminder: Not a confirmed booking"); 
    if($resObj->checkout > $today) return json_error_object("Review reminder: not checked out yet");
    if($resObj->checkout < $expires) return json_error_object("Review reminder: review period expired on ".sql2human($expires));
    $template = 'mail/mail-booking-review';
    $guest_text = phrase('how was your stay at %s?', CAPS, $apt_name);  
    
    //die("ok, sending review mail result=".dump($result,true));
  } elseif($reminder) { /** send reminder of upcoming checkin */
    if($resObj->reminder_sent > 0) return json_error_object("Booking reminder already sent");
    if(!in_array($resObj->status, [STATUS_RESERVED, STATUS_COMPLETE])) return json_error_object("Booking reminder: Not a confirmed booking"); 
    if($resObj->checkin < $today) return json_error_object("Booking reminder: already checked in");
    if($resObj->checkin > add_days($today, BOOKING_REMINDER_PERIOD)) return json_error_object("Booking reminder: too early to notify");
    //$template = 'mail/mail-booking-reminder';
    $guest_text = phrase('reminder', CAPS).": ".phrase('you have an upcoming booking at %s', CAPS, $apt_name);  
    $host_text = phrase('reminder', CAPS).": ".phrase('you have an upcoming booking for %s', CAPS, $apt_name);  
    
  } else { /** send standard mail based on status */
    
    list($guest_text, $host_text, $staff_text) = booking_mail_message($resObj);
    
    if(strip_blank($resObj->extra_info)) $guest_message = $resObj->extra_info;
    if(strip_blank($resObj->message)) $host_message = $resObj->message;
    if(strip_blank($resObj->notes)) $staff_message = $resObj->notes;
    
  }

  /** send mail */

  $from = $siteObj->email_address ?: "info@{{$siteObj->domain}}";
  
  $subject = booking_mail_subject($resObj, ['action' => $action]);
  $dear = phrase('dear', CAPITALIZE);
  
  $guest_name = $resObj->guest_id ? user_display_name($resObj->guest_id) : get_name_string($resObj);
  $host_name = $resObj->host_id ? user_display_name($resObj->host_id) : phrase('host', CAPITALIZE);
  
  $host_email = host_email($resObj);
  $guest_email = guest_email($resObj) ?: email_recipient($resObj->email_address, $guest_name);

  $site_email = $resObj->site_id ? site_email($resObj->site_id, true) : "info@{$siteObj->domain}";
  $staff_email = $site_email;
    
  $lang = pick_first($resObj->language, $__LANG, $siteObj->default_language, 'en'); /** todo: language per user */
  $mail_options = ['parent_type' => 'reservation', 'parent_id' => $resObj->res_id, 'language' => $lang, 'debug' => $options['debug']];
  
  /** send mail to guest */
  if($resObj->send_email && $guest_email && $guest_text) {

    $mail_scope = ['profile_id' => $resObj->host_id, 'receiver_role' => 'guest', 'receiver_name' => $guest_name ?: phrase('guest', CAPITALIZE), 'receiver_id' => $resObj->guest_id, 'res_id' => $resObj->res_id, 'main_message' => $guest_text];
    if($personal_message = $host_message ?: $guest_message) $mail_scope['personal_message'] = $personal_message;
    
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'booking']);      
    
    if($options['debug']) return ['to' => $guest_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
    
    $mail_options['language'] = get_user_language($resObj->guest_id) ?: $lang;
    $mail_options['receiver_id'] = $resObj->guest_id;
    $mail_options['receiver_role'] = 'guest';
    
    $result = send_mail($siteObj, $subject, $body, $guest_email, $mail_options);
    if($result->error) print_log("Error sending booking mail to guest: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }

  
  /** send mail to host */
  if(!$resObj->corporate && $host_email && $host_text) {
    
    $mail_scope = ['profile_id' => $resObj->guest_id, 'receiver_role' => 'host', 'receiver_name' => $host_name ?: phrase('host', CAPITALIZE), 'receiver_id' => $resObj->host_id, 'res_id' => $resObj->res_id, 'main_message' => $host_text]; 
    if($personal_message = $guest_message ?: $host_message) $mail_scope['personal_message'] = $personal_message;
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'booking']);      
    
    if($options['debug']) return ['to' => $host_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
    
    $mail_options['language'] = get_user_language($resObj->host_id) ?: $lang;
    $mail_options['receiver_id'] = $resObj->host_id;
    $mail_options['receiver_role'] = 'host';
    
    $result = send_mail($siteObj, $subject, $body, $host_email, $mail_options);    
    if($result->error) print_log("Error sending booking mail to host: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }

  /** send mail to staff */
  if($staff_email && $staff_text) {
    $staff_name = "{$siteObj->name} ".phrase('staff');    
    $mail_scope = ['profile_id' => $resObj->guest_id, 'receiver_role' => 'staff', 'receiver_name' => $staff_name, 'receiver_id' => 0, 'res_id' => $resObj->res_id, 'main_message' => $staff_text]; 
    $personal_message = $host_message ?: $guest_message;
    if($staff_message) $personal_message .= "<br><b>Notes:</b> {$staff_message}";
    if($personal_message) $mail_scope['personal_message'] = $personal_message;
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'booking']);      

    $mail_options['receiver_id'] = 0;
    $mail_options['receiver_role'] = 'staff';
    $mail_options['language'] = $lang;

    if($options['debug']) return ['to' => $staff_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
    
    $result = send_mail($siteObj, $subject, $body, $staff_email, $mail_options);    
    if($result->error) print_log("Error sending booking mail to staff: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }
  
  /** send mail to site (copy of guest message, just HAB for now) */
  if($resObj->site_id == 263 && $guest_text && $site_email) {
    $site_text = "<p><b>Copy of guest message to {$siteObj->name}</b></p><br>".$guest_text;

    $staff_name = "{$siteObj->name} ".phrase('staff');    
    $mail_scope = ['profile_id' => $resObj->guest_id, 'receiver_role' => 'staff', 'receiver_name' => $staff_name, 'receiver_id' => 0, 'res_id' => $resObj->res_id, 'main_message' => $site_text]; 
    $personal_message = $host_message ?: $guest_message;
    if($staff_message) $personal_message .= "<br><b>Notes:</b> {$staff_message}";
    if($personal_message) $mail_scope['personal_message'] = $personal_message;
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => 'booking']);      

    if($options['debug']) return ['to' => $site_email, 'from' => $from, 'subject' => $subject, 'body' => $body];
    $mail_options['receiver_id'] = 0;
    $mail_options['receiver_role'] = 'site';
    
    $result = send_mail($siteObj, $subject, $body, $site_email, $mail_options);    
    if($result->error) print_log("Error sending booking mail to site: ".$result->error[0], 'mail', LOG_LEVEL_ERROR);
  }
  
  if($options['debug']) return ['message' => 'no mail sent', 'id' => $resObj->res_id, 'status' => $status, 'template' => $template, 'guest' => $guest_email, 'guest_len' => strlen($guest_text), 'host' => $host_email, 'host_len' => strlen($host_text), 'staff' => $staff_email, 'staff_len' => strlen($staff_text)];
  
  return $result;
}


function booking_mail_message($resObj) {
  $guest_text = $host_text = $staff_text = '';
  $period = print_time_range($resObj->checkin, $resObj->checkout);
  
  switch($resObj->status) {
    case STATUS_EXPIRED:
      $guest_text = $host_text = $staff_text = phrase("the request for %s in the period %s has expired.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));     
      break;
    case STATUS_CANCELLED:
      $guest_text = $host_text = $staff_text = phrase("the booking for %s in the period %s has been cancelled.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));     
      break;
    case STATUS_DECLINED:
      $guest_text = phrase("%s is unfortunately not available in the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));     
      break;
    case STATUS_REJECTED:
      $host_text = phrase("the guest has unfortunately decided not to book %s in the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));     
      $guest_text = phrase("you declined to book %s in the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));     
      break;
    case STATUS_INQUIRY:
      $host_text = $staff_text = phrase("you have an inquiry for %s for the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      $guest_text = phrase("you sent an inquiry for %s for the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      break;
    case STATUS_UNCONFIRMED:
      $host_text = $staff_text = phrase("%s has been requested for the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      $guest_text = phrase("you requested %s for the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      break;
    case STATUS_PROPOSED:
      $host_text = phrase("you confirmed that %s is available in the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      $guest_text = phrase("%s is available in the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      break;
    case STATUS_RESERVED:
    case STATUS_COMPLETE:
      $host_text = $staff_text = phrase("%s has been booked for the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      $guest_text = phrase("you are confirmed to stay in %s in the period %s.", CAPITALIZE, html_b($resObj->apartment_name), html_b($period));
      
      $checkin_instructions = '';
      if($desc = find_object('property_description', ['host_id' => $resObj->host_id, 'active' => 1, 'property_id' => 0], 'description')) $checkin_instructions .= $desc."\n"; 
      if($desc = find_object('property_description', ['host_id' => $resObj->host_id, 'active' => 1, 'property_id' => $resObj->apt_id], 'description')) $checkin_instructions .= $desc."\n"; 
      if($checkin_instructions) $guest_text .= html_h2(phrase('arrival instructions', CAPS)).nl2br($checkin_instructions);
      
      break;      
    default:
      break;
  }
  return [$guest_text, $host_text, $staff_text];
}

function mail_subject($obj_type, $obj, $options=[]) {
  global $__SITE; 
  $obj_name = $request_name = $tail = '';
  
  switch($obj_type) {
    case 'reservation': 
      return booking_mail_subject($obj, $options);
      break;
    case 'message':
      $request_name = phrase('message from', CAPS);
      $tail = "{$obj->name}: {$obj->subject}";
      break;
    default:
      $obj_name = get_object_name($obj_type);
      break;      
  }
  
  $obj_id = any2id($obj_type, $obj);  
  if(!$request_name) $request_name = phrase($obj_name, CAPS)." {$obj_id}";
    
  $site_id = $obj->site_id ?: $__SITE->id;
  $siteObj = get_object('site', $site_id);

  $action = isset($options['action']) ? $options['action'] : '';
  $action_str = $action ? " ".phrase($action) : '';

  //$guest_name = guest_name($obj) ?: get_name_string($resObj);

  $subject = "[{$siteObj->shortname}] {$request_name}{$action_str}";
  if($tail) $subject .= " {$tail}";  
  return $subject;
}

function booking_mail_subject($resObj_or_id, $options=[]) {
  $action = isset($options['action']) ? $options['action'] : '';
  $action_str = $action ? " ".phrase($action) : '';
  
  $resObj = any2obj('reservation', $resObj_or_id);

  switch($resObj->status) {
    case STATUS_INQUIRY:
      $request_name = phrase('booking_inquiry', CAPITALIZE);
      break;
    case STATUS_REQUEST:
      $request_name = phrase('booking_request', CAPITALIZE);
      break;
    case STATUS_PROPOSED:
      $request_name = phrase('accepted_booking', CAPITALIZE);
      break;
    case STATUS_PENDING:
      $request_name = phrase('pending_booking', CAPITALIZE);
      break;
    case STATUS_RESERVED:
    case STATUS_COMPLETE:
      $request_name = phrase('confirmed_booking', CAPITALIZE);
      break;
    case STATUS_REJECTED:
    case STATUS_DECLINED:
      $request_name = phrase('declined_booking', CAPITALIZE);
      break;
    case STATUS_CANCELED:
      $request_name = phrase('cancelled_booking', CAPITALIZE);
      break;
    case STATUS_EXPIRED:
      $request_name = phrase('expired_booking', CAPITALIZE);
      break;
    default:
      $request_name = phrase('booking', CAPITALIZE);
      break;    
  }
      
  $site_id = $resObj->site_id ?: $__SITE->id;
  $siteObj = get_object('site', $site_id);
  $length_str = '';
    
  $count = $resObj->num_days;
  if($count > 0) {
    $units = $count == 1 ? phrase('day') : phrase('days'); // todo, allow hours
    $length_str = " - $count $units ";
  }
  
  $guest_name = $resObj->guest_id ? get_user_name($resObj->guest_id) : get_name_string($resObj);

  $subject = "[{$siteObj->shortname}] {$request_name}{$action_str} {$resObj->res_id} ({$guest_name}{$length_str} @ {$resObj->apartment_name})";
  return $subject;
}

// takes Some Guy <foo@bar.com> or comma separated list thereof and returns an array('foo@bar.com', 'Some Guy') usable by SwiftMailer;
function parse_email($email) {
  if(is_array($email)) return $email; // assume correct
  if(!trim($email)) return [];
  
  $mail_object = Mail_RFC822::parseAddressList($email, null,null,false); /** false to not stumble on non-ascii characters */
  if (PEAR::isError($mail_object)) {
    print_log($mail_object->getMessage(), 'mail', LOG_LEVEL_ERROR);
    return [];
  }
  
  $addresses = [];
  foreach($mail_object as $result) {
    $name = $result->personal;
    $host = $result->host;    
    $email_address = "{$result->mailbox}@{$host}";
    if($host != 'localhost') {
      if($name) $addresses[$email_address] = $name;
      else if($email_address) $addresses[] = $email_address;
    }
  }
  return $addresses;
}

// takes either a string 'Tobias Brox <tobix@cpan.org>' or a parsed array (from parse_email) and returns the name portion if there is only one adderss 
function email_receiver_name($recipient) {
  list($email, $name) = parse_recipient($recipient);
  return $name;
}

function email_receiver_email($recipient) {
  list($email, $name) = parse_recipient($recipient);
  return $email;
}

// parse single recipient 'Tobias Brox <tobix@cpan.org>' into [email, name]
function parse_recipient($email) {
  $parsed_email = is_array($email) ? $email : parse_email($email);
  if(count($parsed_email) != 1) return [];
  foreach($parsed_email as $k => $v) {
    if($k == 0 && is_email($v)) return [$v, '']; // no name
    if(is_email($k)) return [$k, $v];
  }
  return [];
}

// return 'Tobias Brox <tobix@cpan.org>'
function email_recipient($email, $name='') {
  $email = trim($email);
  if(!$email || !is_email($email)) return '';
  $name = str_replace(['<','>'], '', trim($name)); /** no lt/gt in name! */
  return $name ? "{$name} <$email>" : $email;
}

/** because I'm lazy */
function send_mail_old($subject, $body, $from, $to, $replyto="", $cc="", $parent_type=NULL, $parent_id=NULL, $language='', $delay=0, $headers="") {
  $site_id = $_SESSION['site_id'];
  return send_mail($site_id, $subject, $body, $to, 
    array('sender' => $from, 'replyto' => $replyto, 'parent_type' => $parent_type, 'parent_id' => $parent_id, 'language' => $language));
}

function send_campaign($campaignObj_or_id, $options = array()) {
  $campaignObj = any2obj('mail_campaign', $campaignObj_or_id);
  if(!$campaignObj) return json_error_object('campaign not found');
  $time_to_send = $campaignObj->time_to_send;
  if(!$time_to_send) return json_error_object("Campaign time to send not set"); 
  $now = now();
  if($time_to_send > $now) return json_error_object("Campaign not due to send until $time_to_send UTC"); 

  // ok send  
  $options['batch'] = 1;
  $options['parent_type'] = 'mail_campaign';
  $options['parent_id'] = $campaignObj->id;
  foreach(['sender','language', 'attachment', 'attachment_name', 'cc', 'bcc','replyto','sender_id'] as $f) $options[$f] = $campaignObj->$f;
  
  $recipients = json_decode($campaignObj->recipients);
  $count = count($recipients);
  if(!$count) return json_error_object("No recipients found: {$campaignObj->recipients}");
  $recipientsList = array2list($recipients);
  
  print_log("rlist:".dump($recipientsList, true), 'campaign', LOG_LEVEL_TEST);
  //return json_error_object("gh count=$count r:{$campaignObj->recipients}");
  
  return send_mail($campaignObj->site_id, $campaignObj->subject, $campaignObj->body, $recipientsList, $options);
}

/** new function using SwiftMailer */
function send_mail($siteObj_or_id, $subject, $body, $recipient, $options = array()) { 
  global $__SITE;
  global $__USER;
  $errors = [];  
  $now = now();

  $siteObj = any2obj('site', $siteObj_or_id);
  if(!$siteObj) return json_error_object("send_mail: Missing site"); 
  //$site_id = isset($options['site_id']) ? $options['site_id'] : $__SITE->id;
  //$siteObj = $site_id == $__SITE->id ? $__SITE : get_object('site', $site_id);
  
  $user_id = $__USER ? $__USER->id : 0;  
  
  // options
  $user = isset($options['smtp_user']) ? $options['smtp_user'] : get_config("gu", $siteObj->id); 
  $pass = isset($options['smtp_pass']) ? $options['smtp_pass'] : get_config("gp", $siteObj->id);
  $host = isset($options['smtp_host']) ? $options['smtp_host'] : get_config("gh", $siteObj->id);
  $port = isset($options['smtp_port']) ? $options['smtp_port'] : get_config("gpt", $siteObj->id);
  $send = isset($options['send_mail']) ? $options['send_mail'] : true; // turn off for debugging
  $echo = isset($options['echo']) ? $options['echo'] : false; // turn on for debugging

  $debug = isset($options['debug']) ? $options['debug'] : false; // turn on for debugging
  $role  = isset($options['role']) ? $options['role'] : ''; // staff, guest, host, site
  $batch = isset($options['batch']) ? $options['batch'] : false; // set true to send separate mail for each recipient
  
  $sender  = isset($options['sender']) ? $options['sender'] : site_email($siteObj);
  $replyto = isset($options['replyto']) ? $options['replyto'] : site_replyto($siteObj);
  $cc      = isset($options['cc']) ? $options['cc'] : '';
  $bcc     = isset($options['bcc']) ? $options['bcc'] : '';

  $priority   = isset($options['priority']) ? $options['priority'] : 0;
  $attachment = isset($options['attachment']) ? $options['attachment'] : '';
  $attachment_name = isset($options['attachment_name']) ? $options['attachment_name'] : '';
  
  $from = parse_email($sender); // split into array ['receiver@domain.org', 'other@domain.org' => 'A name']
  $to = parse_email($recipient);  
  if($cc) $cc = parse_email($cc);
  if($bcc) $bcc = parse_email($bcc);
  if($replyto) $replyto = parse_email($replyto);
  $receiver_name = $options['receiver_name'] = isset($options['receiver_name']) ? $options['receiver_name'] : email_receiver_name($to); // get 'A name'
  $receiver_email = $options['receiver_email'] = isset($options['receiver_email']) ? $options['receiver_email'] : email_receiver_email($to); // get 'foo@bar.com'
  $receiver_username = $options['receiver_username'] = $options['receiver_id'] ? username($options['receiver_id']) : $receiver_email; // login username
  
  $log_level = $debug_mail || (isset($options['log']) && $options['log'])  ? LOG_LEVEL_ALWAYS : LOG_LEVEL_TEST;

  $debug_mail = isset($_SESSION['debug_mail']) ? $_SESSION['debug_mail'] : false;
  /** extensive debug logging
  if($debug_mail) { 
    print_log("\n\n------------", 'mail', $log_level);
    print_log("send_mail: {$now}", 'mail', $log_level);
    print_log("send_mail: subject: $subject", 'mail', $log_level);
    print_log("send_mail: sender: ".(is_array($sender) ? dump($sender,true) : $sender), 'mail', $log_level);
    print_log("send_mail: receiver: ".(is_array($recipient) ? dump($recipient,true) : $recipient), 'mail', $log_level);
    //if($cc) print_log("send_mail: cc: ".(is_array($cc) ? dump($cc,true) : $cc), 'mail', $log_level);
    print_log("send_mail: receiver: strlen:".strlen($recipient), 'mail', $log_level);
    print_log("send_mail: receiver: array?:".(is_array($recipient) ? " yes " : " no"), 'mail', $log_level);
    print_log("send_mail: receiver: obj?:".(is_object($recipient) ? " yes " : " no"), 'mail', $log_level);
    if($options) print_log("send_mail: options:".dump($options,true), 'mail', $log_level);
  } */
  
  if(!$host) $host = "ssl://smtp.gmail.com"; // fallback
  if(!$port) $port = "465"; // fallback
  if(!$user) $errors[] = "Cannot send email: missing username for {$siteObj->id}"; 
  if(!$recipient || !$to) $errors[] = "Cannot send email: missing receiver email"; 
  if(!$sender || !$from) $errors[] = "Cannot send email: missing sender email"; 
  if(!$subject) $errors[] = "Cannot send email: missing subject"; 
  if(!$body) $errors[] = "Cannot send email: missing body"; 
  //if(!$pass) $errors[] = "Cannot send email: missing password");
  
  if($errors) {
    print_log("send_mail: returning with errors:".dump($errors,true), 'mail', $log_level);
    print_log("send_mail: stack:".caller_stack(), 'mail', $log_level);    
    return json_error_object($errors);
  }



  $deployment = $_SESSION['deployment'];  
  $staging = $_SESSION['staging'] ||  $deployment == 'staging' ? 1 : 0;
  $dev = $_SESSION['dev'] ||  $deployment == 'dev' ? 1 : 0;
  
  $body_len = human2bytes(strlen($body));
  print_log("send_mail role: {$role} to: {$recipient} from: {$sender} subject: {$subject} body: {$body_len}", 'mail', LOG_LEVEL_TEST);  
  if($debug) return array2obj(['to' => $recipient, 'from' => $sender, 'subject' => $subject, 'body' => $body]); // do nothing, just return
    
  $send_mail = $_SESSION['live'] ? 1 : 0; // by default, send mail if live
  $send_mail_ar = $_SESSION['send_mail'];
  if(is_array($send_mail_ar) && $deployment) { // override in index.php of site
    $send_mail = $send_mail_ar[$deployment];
    if(is_email($send_mail)) {
      $body = "<b>Recipient: ".htmlspecialchars($recipient)."</b><br>{$body}";
      $recipient = $to = $send_mail;
    }
  }
  if(!$_SESSION['live'] && $deployment) $subject = "<{$deployment}> {$subject}";
   
  // for staging, only send mail to logged in user
  
  if($staging) {
    $subject = "[STAGING] {$subject}";
    $user_email = $__USER ? get_user_email($__USER) : ''; 
    $subject .= " To: {$recipient}";
    if($user_email && is_email($user_email)) {
      print_log("{$now}: staging: sending to all mails to logged in user {$user_email}", 'mail', LOG_LEVEL_TEST);
      $to = email_recipient($user_email, "Recipient: {$recipient}");
      $recipient = $user_email;
      $cc = '';
      $send_mail = 1; // or only if set in send_mail_ar
    } else {
      $send_mail = 0;
    }
  } elseif(0 && $dev) {
    switch($siteObj->id) {
    case '263':
      break;
      $to = array('hitabrickbv@gmail.com' => "Test Mail ({$recipient})");
    default:      
      $to = array('dev@kjakman.com' => "Test Mail ({$siteObj->domain})");
      break;
    }
    $cc = $bcc = '';
    $subject = "[DEV] {$subject}";
  }
    
  if(!$from) return json_error_object("Sender $sender not valid email");
  if(!$to) return json_error_object("Receiver $recipient not valid email");
  if(!$subject) return json_error_object("Missing email subject");
  if(!strlen(strip_blank($body))) return json_error_object("Missing email body");

  
  
  /** Google does not allow from outside domain */
  $domain = $siteObj->domain;
  if(!strpos($sender, $domain)) {
    $org_sender = $sender;
    $site_email = $siteObj->email_address;
    if(is_email($user)) $sender = $user;
    else if(is_email($site_email)) $sender = $site_email;
    else json_error("{$now}: Error. Non-registered domain sender $org_sender - none found for site.");

    $replyto = $from;
    $from = [$sender => $siteObj->name];
  }
  
  $message = Swift_Message::newInstance()
  ->setSubject($subject)
  ->setFrom($from)
  ->setTo($to);
  
  if($cc) $message->setCc($cc);
  if($bcc) $message->setCc($bcc);
  if($replyto) $message->setReplyTo($replyto);
  if($priority && $priority >= 1 && $priority <=5) $message->setPriority($priority);   
  if($attachment && is_file($attachment) && filesize($attachment) > 0) {
    $file = Swift_Attachment::fromPath($attachment);
    if($attachment_name) $file->setFilename($attachment_name);
    if($file) $message->attach($file);    
  }
    
  try {                           
    if(strpos($host, "://")) $transport = Swift_SmtpTransport::newInstance($host, $port);
    else if($port == 465) $transport = Swift_SmtpTransport::newInstance($host, $port, "ssl");
    else if($port == 587) $transport = Swift_SmtpTransport::newInstance($host, $port, "tls");
    else $transport = Swift_SmtpTransport::newInstance($host, $port); 
    //$transport->setSourceIp('188.166.113.34'); // force IPv4
  } catch (Swift_TransportException $e) {
    json_error("Swift Transport Error: ".$e->getMessage());  
  }
  
  if($user) $transport->setUsername($user);
  if($pass) $transport->setPassword($pass);

  $mailer = Swift_Mailer::newInstance($transport);
  

  
  $result = new stdClass;
  $log_mail = false;
  if(!$send) $send_mail = false;
  $options['send_mail'] = $send_mail;
  
  if($batch) { /** send one email for each recipient */
        
    $result = send_mail_batch($mailer, $message, $body, $to, $options);
    $count = $options['count'] = $batchResult->count;

  } else { /** send one email for all recipients */
  
    $options['guid'] = unique_id();
    $body = mail_body($body, $to, $options);
    $message->setBody($body, 'text/html');
    if($send_mail) { /** send is from options and overrides all others */
      $log_mail = true;
      $result->success = $options['count'] = $mailer->send($message);    
      $result->message = $message = "OK: Sent mail";    
    } else {
      $log_mail = $_SESSION['db_dev'] ? 1 : 0; // don't log unsent mail on live database
      $result->success = 1;
      $result->message = $message = "NOT SENDING: simulated sent mail";
    }
  }
  
  $message .= " from $sender to $recipient with subject $subject. deployment=$deployment mail_queue: {$log_mail}";
  
  //$message .= " mailAr:".dump($send_mail_ar, true);
  
  print_log("{$now}: {$message}", 'mail', LOG_LEVEL_ALWAYS);
  
  $result->sender = $sender;
  $result->recipient = $recipient;
  $result->subject = $subject;
  $result->body = $body;
  
  if($log_mail) {
    $mail_id = log_mail($siteObj, $subject, $body, $sender, $recipient, $options); 
    $result->id = $mail_id;
    $result->guid = $mail_id ? get_object('mail_queue', $mail_id, 'guid') : '';
  }
  
  if($echo) {
    $body_len = bytes2human(strlen($body));
    echo("<pre>From: {$sender}<br>To: {$recipient}<br>Subject: {$subject} Body ($body_len): <hr>{$body}<br></pre>".dump($result,true));
  }
  
  
  return $result;
    
}

/** replace constants in mail body */
function mail_body($body, $to, $options) {

  $replaceAr = array(
      '__MAIL_UUID' => $options['guid'], 
      '__MAIL_CAMPAIGN_ID' => $options['campaign_id'], 
      '__MAIL_CAMPAIGN_UUID' => $options['campaign_guid'], 
      '__MAIL_RECIPIENT' => $options['receiver_name'] ?: phrase('user'), // todo: look up role (guest/host/etc). Translate according to mail language
      '__MAIL_RECEIVER_ID' => $options['receiver_id'], 
      '__MAIL_RECEIVER_USERNAME' => $options['receiver_username']);
  $body = str_replace(array_keys($replaceAr), array_values($replaceAr), $body);
  
  /**  
  // used to view mail online
  if(strpos($body, '__MAIL_UUID')) {
    $body = str_replace('__MAIL_UUID', $options['guid'], $body);
  }
  if(strpos($body, '__MAIL_CAMPAIGN_ID')) {
    $body = str_replace('__MAIL_CAMPAIGN_ID', $options['campaign_id'], $body);
  }
  if(strpos($body, '__MAIL_CAMPAIGN_UUID')) {
    $body = str_replace('__MAIL_CAMPAIGN_UUID', $options['campaign_guid'], $body);
  }
  if(strpos($body, '__MAIL_RECEIVER_ID')) {
    $body = str_replace('__MAIL_CAMPAIGN_ID', $options['campaign_id'], $body);
  }
  if(strpos($body, '__MAIL_RECEIVER_USERNAME')) {
    $body = str_replace('__MAIL_CAMPAIGN_ID', $options['campaign_id'], $body);
  }
  if(strpos($body, '__MAIL_RECEIVER_USERNAME')) {
    $body = str_replace('__MAIL_CAMPAIGN_ID', $options['campaign_id'], $body);
  }
  if(strpos($body, '__MAIL_RECIPIENT')) {
    $receiver_name = $options['receiver_name']; 
    if(!$receiver_name) $receiver_name = count($to) > 1 ? 'all' : 'user'; 
    $body = str_replace('__MAIL_RECIPIENT', $receiver_name, $body); 
  }
  */
  
  return $body;
}

// Send the message to each individual recipient
function send_mail_batch($mailer, $message, $org_body, $to, $options = []) {
  $send_mail = $options['send_mail'];
  $failed = $errors = [];
  $count = $i = 0;
  $campaign_id = $options['parent_type'] == 'mail_campaign' ? $options['parent_id'] : 0;
  $campaign_guid = unique_id(); // once per campaign
  
  $uuidAr = [];
  foreach ($to as $address => $name) {
    $body = $org_body;
    $sent = 0;
    if (is_int($address)) {
      $address = $name;
      $message->setTo($address);
      $name = '';
    } else {
      $message->setTo(array($address => $name));
    }  
    print_log("send_mail_batch i:$i address: {$address} name: {$name}", 'mail', LOG_LEVEL_TEST);
    $i ++;


    $guid = unique_id(); // once per message     
    $body = mail_body($body, $to, ['receiver_name' => $name, 'receiver_username' => $address, 'guid' => $guid, 'campaign_id' => $campaign_id, 'campaign_guid' => $campaign_guid]);
    $message->setBody($body, 'text/html');
    
    print_log("send_mail_batch to={$address} name={$name} guid=$guid campaign={$campaign_id} body len=".strlen($body), 'campaign', LOG_LEVEL_TEST);
    if(strlen($body)) {
    
      if($send_mail) {
        $sent = $mailer->send($message, $failed);
        $count += $sent;
      } else {
        $sent =  is_email($address) ? 1 : 0; // simulate
        $count += $sent;
      }
    } else {
      $errors[] = "Cannot send mail - empty body";
    }
    
    if($sent) $uuidAr[$guid] = email_recipient($address, $name); 
  }
  print_log("send_mail_batch send:$count send_mail=$send_mail failed array:".dump($failed, true), 'mail', LOG_LEVEL_TEST);
  
  if($campaign_id) {
    $data = ['sent_time' => now(), 'sent_count' => $count, 'guid' => $campaign_guid, 'guids' => json_encode($uuidAr)];
    if(count($failed)) $data['failed'] = json_encode($failed);
    list($id, $errors) = update_object('mail_campaign', $data, $campaign_id);
    if(!$errors) $errors=[]; // shouldn't be needed, but just in case
  }
  if(!$count) $errors[] = "No mails sent";
    
  $result = new stdClass;  
  $result->success = $count && !$errors ? 1 : 0;
  $result->sent = $count;
  $result->failed = count($failed);  
  $result->error = $errors;
  if($count) $result->message = "OK: Sent $count mails";    
  return $result;
}

function campaign_recipients($data) {
  global $__SITE;
  $filter_str = '';
  $receiver_roles = $data['receiver_roles'];
  if(!$receiver_roles) return [0, "No receiver roles selected"];
  $search = ['user.site_id' => $__SITE->id, 'active' => 1];

  $selected_roles = explode(',', $receiver_roles);  
  $selected_roles_str = [];
  if($active_since = $data['active_since']) {
    $active_str = "active since $active_since";
    $filter_str .= " {$active_str}";
    $search['last_login'] = ['>=', date2sql($active_since)];    
  }
    
  if(count($selected_roles) > 1) {
    foreach($selected_roles as $role_id) {
      $role_terms["roles & {$role_id}"] = $role_id;
      $selected_roles_str[] = get_object('user_role', $role_id, 'name');
    }
    $roleSearch = array_merge($search, [DB_LOGICAL_OR => $role_terms]);
  } else {
    $roleSearch = $search;
    $role_id = $selected_roles[0];
    $roleSearch["roles & {$role_id}"] = $role_id;
  }
  $selected_users = find_objects('user', $roleSearch);
  $count = count($selected_users);
  $roles_str = implode(',', $selected_roles_str);
  if(!$count) {
    //dump($roleSearch);
    return [[], "No recipients found for roles {$roles_str} $filter_str"];
  }
  
  $recipients = [];
  $data['total_count'] = $count;
  foreach($selected_users as $user) {
    $recipients[]= user_email($user, -1); // use display name
  }
  
  return [$recipients, $errors];
}

// save sent mail to mail_queue
function log_mail($siteObj, $subject, $body, $sender_email, $receiver_email, $options = array()) {
  $site_id = $siteObj->id;
  $obj = new stdClass;
  
  if($parent_type == 'mail_campaign' && $parent_id) { /** update campaign */
    
    return $parent_id;
    
  } else {
    $mail_data = [];
    foreach(['language', 'parent_type', 'parent_id', 'attachment', 'cc', 'bcc', 'replyto', 'guid','sender_id', 'receiver_id', 'receiver_role', 'receiver_name'] as $k) {
      if(isset($options[$k])) $mail_data[$k] = $options[$k];
    }
    extract($mail_data);
    
    if(!$language && $parent_type && $parent_id) {
      $obj = get_object($parent_type, $parent_id);
      if($obj && isset($obj->language)) $language = $obj->language;
    }
      
    $mail_data['language']   = $language ?:$_SESSION['language'];
    $mail_data['subject']    = $subject;
    $mail_data['site_id']    = $site_id ?: $_SESSION['site_id'];
    $mail_data['recipient']  = $receiver_email;
    $mail_data['try_sent']   = 1;
    $mail_data['sent_time']  = now();
    $mail_data['sender']     = $sender_email;
    $mail_data['headers']    = isset($options['header']) ? json_encode($options['header']) : '';
      
    $mail_data['body']       = $body;
    list($id, $errors) = add_object('mail_queue', $mail_data);
    return $id;
  }
  
}
     

// SMS helpers

// event reminder
/** unsused - automatic comment added on  */
function sms_reminder($obj, $field) {
  if($obj && $obj->reminder_phone) {
    list($body, $from) = sms_template("event_reminder", 'event', $obj, $field);
    return send_sms($obj->reminder_phone, $body, $from, 'event', $obj->id); // live version
  }
  return false;
}


// send an SMS using clickatel gateway with $delay seconds delay
function send_sms($to, $body, $from='', $parent_type=NULL, $parent_id=NULL, $delay=0, $echo=false, $verbose=false, $dryrun=false) {  
  $registered_senders = array('31642200934' /* kjetil */, '31624220928' /* alec */, '31657124869' /* afr */); //hard coded for now
  $message = $error = '';
  if(!phone_valid_idd($to)) {
    if($echo) echo("send_sms: $to is probably missing country code<br>");
    return false;
  }

  $to = clean_phonenumber($to);
  $from = clean_phonenumber($from);
  if($_SESSION['dev'] || $_SESSION['test']) {
    $dryrun = true; // simulation; don't use up our credits for testing
    //$echo = true;    
  }

  
  $body = trim($body);
  if(!strlen($body)) {
    if($echo) echo("send_sms: no body<br>");
    return false;
  }
  
  require_once ("classes/sms_api.class.inc");
  global $u_id;
  $mysms = new sms();
  if(!$to) return json_error_object("send_sms: missing recipient."); //die("send_sms: missing recipient.");  
  if(!$from || !in_array($from, $registered_senders)) $from = '31642200934';

  $data = array('recipient' => $to, 'sender' => $from, 'body' => $body);
  if($delay) {
    $data['time_to_send'] = date('Y-m-d H:i:s',  time() + $delay*1000);
  } else {
    $result = $data['result'] = $dryrun ? 'SIM' : $mysms->send($to, $from, $body); 
    $balance = $data['balance'] = $mysms->getbalance();
    $data['session_id']   = $mysms->session;
    $data['parent_type']  = $parent_type;
    $data['parent_id']    = $parent_id;
    if($result == 'OK') {
      $data['sent_time'] = now(); 
      $data['try_sent'] = 1; // failed attempt, tried once
    } else {
      //dump($result);
      $data['try_sent'] = -1; // failed attempt, tried once
    }
  }  
  list($id, $errors) = add_object('sms_queue', $data);
  if($errors) dump($errors);

  switch($result) {
    case "OK":
      $message = "Sent SMS from $from to $to. Balance=$balance";
      if($echo) echo("$message<br>");
      $success = SMS_OK; // 1
      break;
    case "SIM":
      $message = "Simlulated SMS from $from to $to. Balance=$balance";
      if($echo) echo("$message<br>");
      $success = SMS_SIM; // 2
      break;
    default: 
      $error = "Failed to sent SMS from $from to $to. Balance=$balance Result=$result";
      if($echo) echo("$error<br>");
      $success = SMS_ERROR; // 0
      break;
  }
  if($echo && $verbose) echo($body);
  
  
  if($success) return json_success_object($message, array('success' => $success));
  else return json_error_object($error);
  return $success;
  //return $result == 'OK'; // returns true on success
  
}

// send sms to owner (guest, staff not implemented yet);
function booking_sms($rid, $old_status=0) {
  $resObj = get_object('reservation', $rid);
  if($oid = $resObj->host_id) $ownerObj = get_user($oid);
  if(!$ownerObj->sms_alert) return; // sms_alert is off
  if(!strlen(trim($ownerObj->phone)))return; // no phone number
  $to = $ownerObj->phone;
  
  $body = ''; // turn off for now
  switch($resObj->status) {
    case STATUS_UNCONFIRMED:
      if(!($ownerObj->sms_alert | SMS_FLAG_REQUEST)) return;
      if($resObj->type == RES_TYPE_REQUEST) {
        list($body, $from) = sms_template("reservation_request", 'reservation', $resObj, USER_LEVEL_OWNER, $ownerObj->language);
        $body = ''; // turn off for now
      }
      break;
    case STATUS_PROPOSED:
      if(!$resObj->send_email) return;
      if(!trim($resObj->phone)) return;
      if(!($ownerObj->sms_alert | SMS_FLAG_PROPOSED)) return;
      
      list($body, $from) = sms_template("reservation_proposed", 'reservation', $resObj, USER_LEVEL_GUEST, $resObj->language);
      $to = $resObj->phone; // send to guest
      
      if(!phone_valid_idd($to)) { // try adding +31 for dutch cell phone numbers
        if($resObj->country == 'NL' && strpos($to, '06') === 0) {
          $to = "+31".ltrim($to, '0'); // if still not valid, send_sms will catcht it
          echo("Added +31 to $to");
        }
      }
      
      break;

    case STATUS_RESERVED: // confirmed reservation
      if(!($ownerObj->sms_alert | SMS_FLAG_CONFIRMED)) return;
      $two_days_from_now = add_interval(today(), 2);      
      if($old_status < STATUS_RESERVED && ($resObj->status == STATUS_RESERVED  || $resObj->status == STATUS_COMPLETE) && $resObj->checkin >= today() && $resObj->checkin < $two_days_from_now ) {
        list($body, $from) = sms_template("reservation_confirmation", 'reservation', $resObj, USER_LEVEL_OWNER, $ownerObj->language);
      }
      break;

    case STATUS_CANCELLED:
      if(!($ownerObj->sms_alert | SMS_FLAG_CANCELLED)) return;
      if($old_status >= STATUS_RESERVED) {
        list($body, $from) = sms_template("reservation_cancellation", 'reservation', $resObj, USER_LEVEL_OWNER, $ownerObj->language);
      }
      break;

    default:
      break;

  }      
  
  if($body) {
    send_sms($to, $body, $from, 'reservation', $resObj->id);
  }
}

/** old - still needs replacing with new template based mail */
/** unsused - automatic comment added on  */
function send_updated_account_mail($obj_type, $personObj, $changes) {
  if($personObj->user_terms == 1) { // they registered on the site themselves
    //$userObj = get_object('user', $personObj->parent_id);
    list($subject, $body, $from, $to, $replyto) = mail_template("updated_account", $obj_type, $personObj, 'accounts', 0, $personObj->language, $changes);
    send_mail_old($subject, $body, $from, $to, $replyto, $cc, $obj_type, $personObj->id, $personObj->language);   
  }
}

function send_account_invitation_mail($site_id, $userObj, $code, $email_address) {
  $userObj->site_id = $site_id;  
  list($subject, $body, $from, $to, $replyto) = mail_template("account_invitation", 'user', $userObj, 'accounts', 0, $personObj->language, $code, $email_address);
  send_mail_old($subject, $body, $from, $email_address, $replyto, $cc, 'user', $hostObj->id, $hostObj->language);
  return '';
}

function send_account_upgrade_request($mapObj) {  
  $user_id = $mapObj->user_id;
  $host_id = $mapObj->host_user_id;
  $userObj = get_object('user', $user_id);
  $hostObj = get_object('user', $host_id);
  $user_email = get_user_email($userObj);
  $host_email = get_user_email($hostObj);
  
  list($subject, $body, $from, $to, $replyto) = mail_template("account_upgrade_request", 'user', $hostObj, 'accounts', 0, $personObj->language, $userObj);
  send_mail_old($subject, $body, $from, $to, $replyto, $cc, 'user', $hostObj->id, $hostObj->language);   
}

function send_account_upgrade_confirmation($mapObj) {
  $user_id = $mapObj->user_id;
  $host_id = $mapObj->host_user_id;
  $userObj = get_object('user', $user_id);
  $hostObj = get_object('user', $host_id);
  $user_email = get_user_email($userObj);
  $host_email = get_user_email($hostObj);
  
  list($subject, $body, $from, $to, $replyto) = mail_template("account_upgrade_confirmation", 'user', $userObj, 'accounts', 0, $personObj->language, $hostObj);
  send_mail_old($subject, $body, $from, $to, $replyto, $cc, 'user', $userObj->id, $userObj->language);   
}

/** unsused - automatic comment added on  */
function send_activation_mail($obj_type, $personObj) {
  list($subject, $body, $from, $to, $replyto) = mail_template("account_activation", $obj_type, $personObj, 'accounts', 0, $personObj->language);
  send_mail_old($subject, $body, $from, $to, $replyto, $cc, $obj_type, $personObj->id, $personObj->language);   
}

/** unsused - automatic comment added on  */
function send_new_account_mail($userObj) {
  global $__SITE;
  $siteObj = $__SITE;
  list($subject, $body, $from, $to, $replyto) = mail_template("user_register", 'user', $userObj);
  $subject = "Welcome to {$siteObj->name}";
  return send_mail_old($subject, $body, $from, $to, $replyto, '', $obj_type, $userObj->id);

}

/** unsused - automatic comment added on  */
function send_password_mail($userObj, $guid) {
  if(!$guid) die("send_password_mail: missing guid");
  $userObj->guid = $guid;
  list($subject, $body, $from, $to, $replyto) = mail_template("forgot_password", 'user', $userObj, 'accounts', 0, $userObj->language);
  //echo("subject = $subject<br>from=$from<br>to=$to<br>,replyto=$replyto<br>$body");
  $result = send_mail_old($subject, $body, $from, $to, $replyto, $cc, 'user', $userObj->id, $userObj->language);
  return $result;
}

function sms_template($template, $obj_type, $obj, $param1=0, $param2=0) {
  global $siteObj;
  $from = $siteObj->phone;
  if(!$from && $siteObj->legal_entity_id) $from = get_object('legal_entity', $siteObj->legal_entity_id, 'phone'); 

  $body = '';
  switch($obj_type) {
    case 'reservation': 
      $resObj = $obj;
      $user_level = $param1;
      $langugage = $param2;    
      $url = booking_link($resObj);      
      if($short_url = url_shorten($url)) $details = "Link: $short_url\n";
      $details .= print_array_assoc(reservation_details($resObj, $user_level), 'plaintext');
      if($site_id = $resObj->site_id) {
        $siteObj = get_object('site', $resObj->site_id);
        $from = $siteObj->phone;
      }
      break;
    default:
      break;
  }
  
  switch ($template) {
    case "event_reminder":
      if($obj->type > 1) $event_type = get_object('event_type', $obj->type, 'name');
      else $event_type = "Event";
  
      $field = 'reminder_period'.$param1;
      list($unit, $count) = parse_reminder_time($obj->field);
      $body .= "Reminder: $event_type in $count $unit:\n";
      $body .= print_array_assoc(event_details($obj), 'plaintext');
      break;

    case "reservation_proposed":
      $body .= "Your reservation request with $siteObj->domain was accepted\n";
      $body .= "An email was sent to $resObj->email_address, please check your spam folder if you did not receive it.\n";
      $body .= "Link: $short_url.\n";
      break;      

    case "reservation_confirmation":
      $body .= "New confirmed reservation:\n".$details;
      break;
  
    case "reservation_cancellation":
      $body .= "This reservation was cancelled:\n".$details;
      break;
  
    case "reservation_request":
      $body .= "New reservation request:\n".$details;
      break;
  
    default:
      die("Unknown SMS template $template");
      break;
  }
  return array($body, $from);
}

/** used by record in db_object */

function parse_mail_body($body) {
  $pos = strpos($body, '"');
  $start = $pos ? substr($body, $pos+1) : $body;
  return quoted_printable_decode(rtrim($start, '";')); // strip last 2 (";)
}


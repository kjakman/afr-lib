<?php


/** todo: function mail_template($template, $obj_type, $obj, $options = array()) { */ 

function mail_template($template, $obj_type, $obj, $action, $user_level=USER_LEVEL_GUEST, $lang='', $param1=0, $param2=0) {
  global $reqTypeArray;

  
   
  //echo("t=$template, ot=$obj_type, ac=$action<br>");
  // todo: implement options
  if($obj_type == 'boat') $options['show_details'] = false;
  
  $action_str = phrase($action, CAPITALIZE);
  $session_lang = $_SESSION['language'];  // store session (screen) language

  if(!$lang) $lang = pick_first(array($obj->language, $_SESSION['language'], 'en'));
  $_SESSION['language'] = $lang; // override session language

  // quite kludgy, but this pre-processing will fetch the chat object, then proceed with processing as if reservation or enquiry
  switch($obj_type) {
    case 'chat':
      $chatObj = $obj;
      $parent_type = $chatObj->parent_type;
      $reqObj = in_array($parent_type, array('reservation', 'enquiry')) ? get_object($parent_type, $chatObj->parent_id) : null;

      //$message = $chatObj->message;

      switch($parent_type) {
        case 'reservation':
          $obj = $resObj = $reqObj;
          $obj_type = 'reservation';
        break;
        case 'enquiry':
          $obj = $enqObj = $reqObj;
          $obj_type = 'enquiry';
        break;
        default:
          // die("Unknown or missing parent type of chat obj: $parent_type");
      }    
    break;
    case 'user':
    if(!$obj->email_address && is_email($obj->username)) $obj->email_address = $obj->username;
    break;
  }

  // get default url,name,email from client (AFR, AB)
  if($client_id = $obj->client_id ?: $_SESSION['client_id']) {
    $clientObj = get_object('client', $client_id);
    $our_site_url  = $clientObj->web_address;
    $our_site_name = $clientObj->name;
    $our_site_name_short = $clientObj->shortname;
    $our_email = $clientObj->email_address;
  }
  
  // catchall
  $our_site_url  = $our_site_url ?: "http://www.apartments-for-rent.com";
  $our_site_name = $our_site_name ?: "Apartments-for-rent";
  $our_site_name_short = $our_site_name_short ?: "AFR";  
  $our_sub = $action ?: 'info';
  $our_email = $our_email ?: $our_sub."@apartments-for-rent.com";

  $obj_name = get_object_name($obj_type);

  // get recipient / owner / agent / affiliate name + email
  if($obj) {
    list($guest_name, $guest_email) = mail_recipient($obj);
    //echo("guest_name = $guest_name, guest_email = $guest_email<br>");

    if(($host_id = $obj->host_id)    && ($ownerObj = get_user($host_id)))    list($owner_name, $owner_email) = mail_recipient($ownerObj);
    if(($org_id   = $obj->org_id   ) && ($orgObj   = get_object('org',   $org_id)))      list($org_name, $org_email)     = mail_recipient($orgObj);
    if(($agent_id = $obj->agent_id ) && ($agentObj = get_object('agent', $agent_id)))    list($agent_name, $agent_email) = mail_recipient($agentObj);
    //if(($aff_id   = $obj->aff_id   ) && ($affObj   = get_object('affiliate', $aff_id)))  list($aff_name, $aff_email)     = mail_recipient($affObj);
    if(($site_id  = $obj->site_id  ) && ($siteObj  = get_object('site', $site_id)))      list($site_name, $site_email)   = mail_recipient($siteObj);

    if($city_id = $obj->city_id) $city_name = geoname($city_id);

    if($siteObj) {
      $site_url         = $siteObj->url;
      $site_name_short  = strtoupper($siteObj->shortname);
      $site_name        = $siteObj->name;
      $site_email       = $siteObj->email_address;
    } elseif($affObj) {
      $site_name        = $affObj->affiliate_name;
      $site_email       = $affObj->email_address;
      $site_name_short  = $affObj->affiliate_name;
      $site_url         = $affObj->url;
    } else {
      $site_name        = $our_site_name;
      $site_name_short  = $our_site_name_short;
      $site_id          = $_SESSION['site_id'] ?: 10; // AFR
      $siteObj          = get_object('site', $site_id);
    }

    $domain = $siteObj->domain;    
    $do_not_reply       = "Do Not Reply <donotreply@$domain>";
    $site_replyto       = $do_not_reply;

    // catch all
    if(!$site_name)       $site_name        = $our_site_name;
    if(!$site_name_short) $site_name_short  = $our_site_name_short;
    if(!$site_url)        $site_url         = $our_site_url;
    if(!$site_email)      $site_email       = $our_email;
    $site_email_long = "$site_name <".$site_email.">";

    // standard subject for requests (reservations + enquiries)
    if($request_name = $reqTypeArray[$obj_type]) {
      $subject  = request_mail_subject($obj_type, $obj, $site_name_short, $action);
      $reqObj   = $obj; // alias of object
      $apt_id   = $reqObj->apt_id;
      $property_type = $reqObj->property_type;
      if($apt_id && $property_type) {
        $aptObj = get_object($property_type, $apt_id);
        $price_field = property_price_field($property_type);
        $apt_price = $aptObj->$price_field;
      }
      $apt_name = $reqObj->apartment_name;

      if($obj_type == 'reservation') {
        $resObj = $obj;
        $show_time = $resObj->num_hours > 0 ? true : false;
        $checkin_str = sql2human($resObj->checkin, array('show_weekday' => true, 'show_time' => $show_time));
        $checkout_str = sql2human($resObj->checkout, array('show_weekday' => true, 'show_time' => $show_time));
      } elseif($obj_type == 'enquiry') {
        $enqObj = $obj;
      }
    } else {
      // default subject
      //echo("ot=$obj_type");dump($reqTypeArray);
      if(!$subject) $subject =  "[".$site_name_short." $action_str"."] $obj_name: $name";
    }
  }

  if($action == 'accounts') {
    if($obj_type == 'user') {
      $userObj = $obj;
    } else {
      $personObj = $obj;
      $userObj = get_object('user', $personObj->parent_id);
    }
    $user_type = get_user_level_name($userObj->user_level);
  }

  // pick recipient, name based on user_level
  $from = $site_email_long;
  $replyto = $site_replyto; // by default, cannot reply to any email
  if(!$user_type) $user_type = get_user_level_name($user_level);
  switch($user_level) {
    case USER_LEVEL_GUEST:
      $to = $guest_email;
      $name = $guest_name;
      break;
    case USER_LEVEL_OWNER:
      $to = $owner_email;
      $name = $owner_name;
      break;
    case USER_LEVEL_AGENT:
      $to = $agent_email;
      $name = $agent_name;
      break;
    case USER_LEVEL_STAFF:
      $to = $our_email;
      $from = $replyto = $guest_email;
      $name = phrase('staff', CAPITALIZE);
      break;
    default:
      $to = $guest_email;
      $name = $guest_name ? $guest_name : phrase($user_type, CAPITALIZE);
      break;
  }

  if(!$from) $from = $our_email; // default

// here the mail templates begin....
  $mail_body = '';

  
  
//  message
//  alert
//  notes
//  extra_info
  
  if(!$siteObj) $siteObj = $_SESSION['siteObj'];
  $scope = ['object' => $object, 'object_type' => $obj_type, 'receiver_name' => $name, 'receiver_email' => $to]; 
  $options = ['scope' => $scope];
  $mail_body = smarty_mail_template($siteObj, $template, $options);
  $mail_footer = '';
  if(!$mail_body) {    
    switch ($template) {
  
    case "contact_form":
      $subject .= $siteObj->name." ".phrase('contact_form', CAPITALIZE);
      $mail_body = html_p("You have received a new message from your website's contact form.").html_break();
  
      if($obj->subject) $mail_body .= html_b("Subject: ").$obj->subject.html_break();
      if($obj->name) $mail_body .= html_b("Sender: ").$obj->name.html_break();
      if($obj->email_address) $mail_body .= html_b("Email: ").$obj->email_address.html_break();
      if($obj->phone) $mail_body .= html_b("Phone: ").$obj->phone.html_break();
      if($obj->remote_ip) $mail_body .= html_b("IP Address: ").$obj->remote_ip.html_break();    
      if($obj->message) $mail_body .=  html_b("Message: ").html_p(txt2html(strip_tags($obj->message)));
      
      //."Here are the details:\n\nName: $name\n\nPhone: $phone\n\nEmail: $email_address\n\nMessage:\n$message";
      break;
  
    case "order_confirmation":
      $subject .= phrase('purchase_confirmation', CAPITALIZE);
      $mail_body .= vpatina_order_confirmation($siteObj, $obj);
      break;
      
    case "forgot_password":
      $subject .= phrase('password_reset_request', CAPITALIZE);
      $mail_body .= "<p>We received a request to reset the password for the following account:</p>";
      $mail_body .= login_details($obj);
      //dump($siteObj);
      if($reset_link = reset_link($siteObj, $obj->guid)) $mail_body .= "<p>Click $reset_link to reset your password. The link expires in one hour.</p>";
      $mail_body .= "<p>If this action was not initiated by you, you may safely ignore this message.</p>";
      break;
  
    case "updated_account":
      $changes = $param1;
      $who = changed_by($changes);
      $subject .= " - account updated by $who";
      $mail_body.= html_p("$who made the following changes to this account:");
      foreach($changes as $change) $mail_body.= $change['str'].html_break();
      break;
  
    case "account_activation":
      $to = $guest_email;
      $link.= activation_link($userObj);
      $mail_body .= "Thank you for registering with $site_name.";
      // $mail_body .= "Click $link to activate your account.".html_break(2);
      // $mail_body .= address_details($obj, true);
      $mail_body .= login_details($obj);
      break;
      
    case "account_invitation":
      $user_name_str = '';
      if($param1 && $param2) {
        $code = $param1;
        $to = $name = $email_address = $param2;
        $userObj = $obj;
        $profileObj = find_user_profile($userObj);
        $user_name = get_display_name($userObj);
        $user_name_str = "($user_name)";
        if(!$siteObj && $userObj->site_id) $siteObj = get_object('site', $userObj->site_id);
        $siteObj = $siteObj ?: $_SESSION['siteObj'];
        if(!$siteObj) die("mail_template: Missing site");
        if($account_link = site_page_link($siteObj, PAGE_ACCOUNT, array('full' => true))) {
          $account_link .= "?ua&c=$code";
          $this_link = html_link($account_link, 'this link');
          $mail_body .= html_div("A $site_name user $user_name_str has invited you to link your accounts so you can share content. Use $this_link to accept the invitation (after registering and/or logging in.)");
          $subject = phrase('account_linking_invitation', CAPITALIZE).": ".$user_name;
        }
      } else {
        die("Missing params: 1=$param1 2=$param2");
        $mail_body = '';
      }
      
      //$mail_body .= address_details($profileObj ?: $param1, true);
      //$mail_body .= login_details($obj);
      break;
  
    case "account_upgrade_confirmation":
      $to = $guest_email;
      //$link.= activation_link($userObj);
      if($hostObj = $param1) {
        $user_name = get_display_name($hostObj);
        $user_name_str = "($user_name)";
        $mail_body .= html_div("Your $site_name account was approved by $user_name_str and successfully linked.");
      }
      // $mail_body .= "Click $link to activate your account.".html_break(2);
      // $mail_body .= address_details($obj, true);
      //$mail_body .= login_details($obj);
      break;
  
    case "account_upgrade_request":
      $to = $guest_email;
      $user_name_str = '';
      if($userObj = $param1) {
        $profileObj = find_user_profile($userObj);
        $user_name = get_display_name($userObj);
        $user_name_str = "($user_name)";
        if(!$siteObj && $userObj->site_id) $siteObj = get_object('site', $userObj->site_id);
        $siteObj = $siteObj ?: $_SESSION['siteObj'];
        if(!$siteObj) die("mail_template: Missing site");
        
        if($account_link = site_page_link($siteObj, PAGE_ACCOUNT, array('full' => true))) {
          $this_link = html_link($account_link, 'this link');
          $mail_body .= html_div("A $site_name user $user_name_str has used your code to request account linking for content sharing. Use $this_link to verify or deny the request (after logging in.)");
          $subject = phrase('account_linking_request', CAPITALIZE).": ".$user_name;
        }
      }
      
      //$mail_body .= address_details($profileObj ?: $param1, true);
      //$mail_body .= login_details($obj);
      break;
  
    case "new_account":
      $subject = phrase('welcome_to', CAPITALIZE)." ".$site_name;
      $userObj = get_object('user', $obj->parent_id);
      $user_type = get_user_level_name($userObj->user_level);
  
      if($address = address_details($obj, true)) $address_str = print_array_assoc($address, 'left').html_break(2);
  
      if($user_level == USER_LEVEL_STAFF) { // copy to staff
        $mail_body .= "A new $user_type has registered with $site_name.<br>";
      } else {
        $link.= activation_link($userObj);
        $mail_body .= "Thank you for registering with $site_name. Your account is active.<br>";
        $mail_body .= login_details($obj);
      }
      $mail_body .= $address_str;
  
      $owner_message = "
      <p>We have added an account for you on our system so that you can log in to our system as an owner. Very shortly, you will be able to add properties yourself.</p>
      <p>At this moment, you can send us an email at info@apartments-for-rent.com with pictures and description, and we will add it for you.</p>
      <p>You will still be able to view the details of your property online and see reservations of enquiries.</p>
      ";
      break;
  
    case "new_rental":
      $aptObj = $obj;
      //$host_name = get_name_string($aptObj->host_id);
      $city_name = $aptObj->address_city;
      $siteObj = $obj->site_id ? get_object('site', $obj->site_id) : $_SESSION['siteObj'];
      $site_url = site_full_url($siteObj);
      $property_link = $site_url.smarty_property_link($aptObj);
      $subject = "[$site_name_short Rental] A rental was activated in $city_name";
  
      $mail_body .= "
      <p>A rental was activated: <a href='$property_link'>$aptObj->name</a></p>
      ";
      break;
  
    case "rental_added":
      $subject = phrase('your_rental_was_added_on', CAPITALIZE)." ".$site_name;
      
      $edit_link = site_page_link($siteObj, PAGE_EDIT_PROPERTY, array('full' => true)).$obj->id;
      
      $profile_link = site_page_link($siteObj, PAGE_PROFILE, array('full' => true));
      $user_profile = html_link($profile_link, "user profile");
      
      $mail_body .= "
        <p>Congratulations, you've successfully created a new listing on your $site_name account!</p>
        <p>You can publish your listing as soon as you have completed all of the necessary steps. Click <a href='$edit_link'>here</a> to visit your $site_name account and complete your listing.</p>
        <p>If you have any questions, please do not hesitate to <a href='mailto:$our_email'>contact</a> our Customer Support Team</p>
        <p>Don't forget to complete your {$user_profile}.</p>
        ";
      break;
  
    case "rental_activated":
      $subject = phrase('your_rental_was_activated_on', CAPITALIZE)." ".$site_name;
      $edit_link = site_page_link($siteObj, PAGE_EDIT_PROPERTY, array('full' => true)).$obj->id;
      $here = html_link($edit_link, "here");
      $mail_body .= "
        <p>Congratulations, your new listing is now published. Providing more information and pictures will greatly increase your odds of succesfully renting out your $site_name home! Click {$here} to visit your Hitabrick account and manage your listing.</p>
        <p>If you have any questions, please do not hesitate to <a href='mailto:$our_email'>contact</a> our Customer Support Team</p>
        ";
      break;
  
      
    case "reservation_expiration":
      $subject .= " has expired.";
      $mail_body .= "
      <p>Thank you for your reservation request with $site_name.</p>
      <p>Unfortunately the owner has not yet replied to this request, so your request has now expired. </p>
      <p>Although it is still possible for the owner to respond, he or she did not do so within the time we require.</p>
      <p>We therefore send you this message as we do not want our customers to wait indefinetely.</p>
      ";
      break;
  
    case "reservation_cancellation":
      $subject .= " has been cancelled.";
      $mail_body .= "
      <p>The reservation with $site_name was cancelled.</p>
      <p>Please find the details below.</p>
      ";
      $reason = trim($resObj->cancel_reason);
      if(strlen($reason)) $mail_body .= "<p>Cancellation reason: $reason.</p>";
      break;
  
    case "reservation_request":
      if($user_level >= USER_LEVEL_STAFF) { // STAFF, ADMIN
        $mail_body .= phrase('text_reservation_request_owner', '', array('search' => '__apt_name', 'replace' => $apt_name));
        $action_links['accept'] = phrase('accept', CAPITALIZE);
        $action_links['decline'] = phrase('decline', CAPITALIZE);
      } elseif($user_level >= USER_LEVEL_OWNER) { // OWNER, AGENT
        $mail_body .= phrase('text_reservation_request_owner', '', array('search' => '__apt_name', 'replace' => $apt_name));
        $action_links['accept'] = phrase('accept', CAPITALIZE);
        $action_links['decline'] = phrase('decline', CAPITALIZE);
      } else { // GUEST
        //$mail_body .= phrase('text_reservation_request_guest', 0, array('search' => array('__apt_name', '__site_name'), 'replace' =>array($apt_name, $site_name)));
        $mail_body .= phrase('text_reservation_request_guest', '', array('search' => array('__apt_name', '__site_name'), 'replace' => array($apt_name, $site_name)));
      }
  
      if($property_type == 'boat') {  
        $options['hide_accounting'] = true;
      }
      break;
  
    case "available_apartment":
      if($property_type == 'boat') {  
        $mail_body = html_break().phrase('text_available_boat', 0, array('search' => '__site_name', 'replace' => $siteObj->name));
        if($msg = $resObj->message) $mail_body.=html_p(txt2html($msg)); // optional message to guest
        $options['hide_details'] = true;
        $options['hide_footer'] = true;
  
      } else {
        $subject .= " ".phrase('is_available');
        $mail_body .= phrase('text_available_apartment', '', array('search' => array('__apt_name', '__checkin_str', '__checkout_str'), 'replace' => array($apt_name, $checkin_str, $checkout_str)));
        if($msg = $resObj->message) $mail_body.=html_p(txt2html($msg)); // optional message to guest
      }
  
      $action_links['confirm'] = phrase('confirm', CAPITALIZE);
      $action_links['reject'] = phrase('cancel', CAPITALIZE);
      break;
  
    case "not_available":
      $subject .= phrase('is_not_available');;
      $mail_body .= phrase('text_not_available', '', array('search' => array('__apt_name', '__checkin_str', '__checkout_str', '__site_url'), 'replace' => array($apt_name, $checkin_str, $checkout_str, $site_url)));
      if($msg = $resObj->message) $mail_body.=html_p(txt2html($msg)); // optional message to guest
      break;
  
    case "reservation_changed":
      $changes = $param1;
      $who = changed_by($changes);
      $subject .= ' '.phrase('was_changed_by', '', array('search'  => '__who', 'replace' => $who));
      $mail_body.= phrase('text_reservation_changed', '', array('search'=> '__who', 'replace' => $who));
      foreach($changes as $change) $mail_body.= $change['str'].html_break(); // only show if loggable?
      $mail_body.= html_break(2);
      if($msg = $resObj->message) $mail_body.=html_p(txt2html($msg)); // optional message to guest
    break;
  
    case "payment_trouble":
      $subject .= " - problems with payment.";
      if($user_level >= USER_LEVEL_OWNER) { // OWNER, STAFF, ADMIN
        $mail_body.= "
        <p>The guest tried to pay by providing a valid credit card, but was unable to pay. Please use the link below to contact the guest within 24 hours to resolve the payment problem.</p>
        ";
      } else { // GUEST
        $mail_body.= html_p("We're sorry you were having problems with the payment. Your reservation will be held for 24 hours while you find another payment option.").
                     html_p("To attempt paying again, use the 'Request Status' link in this email and then 'Confirm Request' to return to the payment page.");
        //$mail_body.= "
        //<p>We are sorry you had problems paying. The staff and/or owner should contact you shortly to find an alternative payment method.
        //   To attempt paying again, click the 'Request Status' link below and then 'Cofirm Request' to return to the payment page.</p>
        //";
      }
      break;
  
    case "reservation_confirmation":
      $subject .= phrase('is_confirmed').".";
  
      
      if($user_level >= USER_LEVEL_OWNER) { // OWNER, STAFF, ADMIN
        $mail_body.= "
        <p>You have a new confirmed reservation from $site_name.</p>
        ";
        
        if($resObj->balance > 1) $action_links['pay'] = phrase('collect_balance', CAPITALIZE);
      } else { // GUEST
        $agent_string = $agentObj ? " (via $agentObj->name)" : '';
  
        $term = $property_type == 'boat' ? 'text_reservation_confirmation_boat' : 'text_reservation_confirmation';
        $mail_body .= phrase($term, 0, array('search' => '__site_name', 'replace' => $siteObj->name));


        // $mail_body .= "
        // <p>Thank you for your reservation with $site_name".$agent_string.". Your reservation is confirmed.</p>
        // <p>Please find the booking details below. This also serves as your receipt.</p>
        // ";
        if($resObj->balance > 1) $action_links['pay'] = phrase('pay_balance', CAPITALIZE);
        $mail_body .= checkin_instructions($siteObj, $resObj, $aptObj);
        $mail_footer = final_checkin_instructions();

      }
      break;
  
    case "account_hacked":
      //$subject .= phrase('is_confirmed').".";
  
      $subject = "$subject IMPORTANT: Your credit card information may have been compromised";
  
      $date_range = print_time_range($obj->checkin, $obj->checkout);
      $mail_body .= "
      
        <p>Today we were informed by a guest that the credit card used in paying for our services was skimmed. There is reason to believe that we are
        a victim of cyber crime and that payment details were stolen in the transactions that took place on one of our websites:
        <ul>
          <li>www.apartments-for-rent.com
          <li>www.youramsterdamhousing.com
          <li>www.jordaanapartmentsamsterdam.com
        </ul>
        <p>We take this attack serious and want to avoid additional damage for you our much valued guest. 
        We deeply regret the inconvenience this causes and want to assure that we 
        take this and the protection of your details serious. In making the payment on our website we have sent the following details to our payment provider:</p>
        <p>Name, card number, expiration date and the cvc code on selected transactions.</p>      
        <p>Currently we  are further investigating what happened and we will report the attack to local police force.  We have taken primary measures to minimise further damage to you, our client and will continue doing so. We recommend that you inform your card issuing company about this attack and block your card. Please also review all credit card transactions carefully (sofar it seems skimmed cards have been used with EAT24.COM, SALAXYCELL and UBER, MFT and GRUBHUB although these companies are in no way involved in the skim) and report them to your credit card company.</p>      
        <p>Please be aware we are further investigating and need all resources we  can use. If you do however want to contact us we can be reached at:</p>
        <p>
        Sicco Behrens<br>
        Sicco@YourAmsterdamHousing.com<br>
        +31 6 52068281<br>
        
        <p>We are prioritising your security and information now. Once this has been solved we are happy to offer you a complimentary night during your next stay with us.</p>
        <br>
      <p>Your card was used to pay or attempt to pay for a stay in $obj->apartment_name from $date_range.</p>
      <p>The charge or attempted charge took place on $obj->timestamp.</p>
      
      ";
      
      $mail_body .= request_mail_subject($obj_type, $obj, $site_name_short, $action);
      //$mail_body .= dump($obj, true);
      $mail_body .= html_break(2);
      break;
      
    case "event_reminder":
      if($obj->type > 1) $event_type = get_object('event_type', $obj->type, 'name');
      else $event_type = "Event";
  
      $field = 'reminder_period'.$param1;
      list($unit, $count) = parse_reminder_time($obj->field);
      $subject .= " in $count $unit";
  
      $mail_body .= html_p("Reminder: $event_type in $count $unit:\n");
      $mail_body .= print_array_assoc(event_details($obj), 'left').html_break(2);
      break;
  
    case "event_summary":
      global $site_url;
      $header_row = array('', 'Date', 'Type', 'Status', 'Apartment', 'Address', 'Guest', '# Guests', 'Alert', 'Notes', ''); 
      $events = $param1;
      $vars = $param2;
      $start = $vars['from'];
      $end   = $vars['to'];
      $type = $vars['type'] ? get_object('event_type', $vars['type'], 'name') : '';
      $period = $start == $end ? sql2human($start, array('show_weekday' => true)) : sql2human($start). " - ".sql2human($end);
      $subject .= "$type summary for $period";
      
      $rows = event_list($events);
      array_unshift($rows, $header_row);
      $mail_body .= html_h3($subject).html_p(html_link("http://$site_url/admin/events.php?from=$start&to=$end&type=$vars[type]", phrase('view_online', CAPITALIZE)));
      $mail_body .= html_table($rows);
      if($obj->type > 1) $event_type = get_object('event_type', $obj->type, 'name');
      else $event_type = "Event";
  
      break;
  
    case "correspondence":
      if($chatObj->reply) $mail_body .= "<p><b>".stripslashes($param1->reply)."</b></p>";
      if($correspondence = get_correspondence($chatObj->parent_type, $chatObj->parent_id, $user_level)) $mail_body .= "<p>$correspondence</p>";
      break;
  
  
    case "send_invoice":
      $invObj = $obj;
      $inv_link = html_link(public_action_link($invObj, 'invoice', array('user_level' => $user_level)), phrase('view', CAPITALIZE)." ".phrase('invoice', LOWERCASE));
      $pay_link = html_link(public_action_link($invObj, 'invoice', array('user_level' => $user_level, 'action' => 'pay')), phrase('pay', CAPITALIZE)." ".phrase('invoice', LOWERCASE));
      $inv_id = $invObj->invoice_id ?: $invObj->id;
      $subject = "[AFR Invoice] Invoice $inv_id ($name)";
  
      if($recipients = $invObj->sent_to) $to = $recipients;
  
      if($message = $invObj->message) $mail_body .= html_p(txt2html($message));
  
      $mail_body .= "
      <p>You have a new invoice from $site_name (attached).</p>
      <p>Click the link below to view this invoice online.</p>
      <p>$inv_link</p>
      <p>Click the link below to pay the invoice.</p>
      <p>$pay_link</p>
      <p>We appreciate your business.</p>
      ";
      break;
  
  
    case "property_enquiry":
      if($user_level >= USER_LEVEL_STAFF) { // STAFF, ADMIN
        $mail_body.= "<p>You have a new $type_str enquiry from $site_name.</p>";
      } elseif($user_level == USER_LEVEL_AGENT) { // AGENT
        $agentObj = get_object('agent', $param1->agent_id);
        $name = $agentObj->name;
        $to = $agentObj->email_address;
        $mail_body.= "<p>You have a new $type_str enquiry. Please use the links below to get the details of the customer.</p>";
      } elseif($user_level == USER_LEVEL_OWNER) { // OWNER
        list($name, $to) = mail_recipient(get_user($param1->host_id));
        $mail_body.= "<p>You have a new $type_str enquiry. Please use the links below to get the details of the customer.</p>";
      } else { // GUEST
        $mail_body .= "<p>Thank you for your $type_str enquiry. $text</p>";
      }
      break;
  
  //  case "send_feedback_request":
  //    $subject = "[AFR feedback] Your feedback is kindly requested, $name.";
  //    $link_yes = html_link($link, 'Yes', true);
  //    $link_no  = html_link($link, 'No', true);
  //    $mail_body .= "
  //    <p>Thank you for choosing $site_name. We appreciate your business.</p>
  //    <p>Please help us improve our service by filling in our feedback form. It only takes a minute.</p>
  //    <p>Use one of the links below to take you to the feedback form.</p>
  //    <p>$link_yes $link_no</p>
  //    ";
  //   break;
  
    case "message_received":
      $to = $site_email;
      $replyto = $obj->email_address;
      $mail_body .= "
      <p>You have received a message via the contact form on your website $site_url:</p>
      <p><b>From:    </b> $obj->name</p>
      <p><b>Email:   </b> $obj->email_address</p>
      <p><b>Subject: </b> $obj->subject</p>
      <p><b>Message: </b><br>$obj->message</p>
      ";
      break;
  
    case "feedback_received":
      $feedback_link = public_action_link($obj, $obj_type, array('user_level' => $user_level, 'act' => 'feedback'));
      $feedback_id = $param1;
      $feedbackObj = get_object('feedback', $feedback_id);
      $feedback = print_feedback($feedbackObj);
      $apt_name = $feedbackObj->apt_id ? ' @ '.get_object('property', $feedbackObj->apt_id, 'name') : '';
      $reviewer_name = get_name_string($feedbackObj);
      $publish_string = $user_level == USER_LEVEL_STAFF ? 
        "To publish the feedback, use the link below." :
        "You can view the feedback by clicking the link below. It will be published as soon as it has been reviewed by our staff.";
      $mail_body .= "
      <p>You have received a customer feedback from $reviewer_name for $obj->parent_type $obj->id $apt_name:</p>
      <p>$publish_string</p>
      <p>$feedback</p>
      <p>$feedback_link</p>
      ";
      break;
  
    case "enquiry_feedback":
      $subject .= " - Your feedback is kindly requested, $name.";
      $link_yes = html_link($link, 'Yes', array('full' => true));
      $link_no  = html_link($link, 'No', array('full' => true));
      $mail_body .= "
      <p>Thank you for choosing $site_name. We appreciate your business.</p>
      <p>Please help us improve our service by filling in our feedback form. It only takes a minute.</p>
      <p>Use one of the links below to take you to the feedback form.</p>
      <p>$link_yes $link_no</p>
      ";
      break;
  
    case "reservation_feedback":
      $subject .= " - Your feedback is kindly requested, $name.";
      // $subject = "[AFR feedback] Your reservation at $apt_name in $city_name.";
      // $vars = public_action_vars($obj, $obj_type, $user_level);
      // $link = get_page_link('feedback')."?".array2qs($vars, true); 
      $link = public_action_link($obj, $obj_type, array('user_level' => $user_level, 'act' => 'feedback'));
      
      $links[] = html_link($link."&r=f", "I stayed at $apt_name and want to answer 4 questions");
      $links[] = html_link($link."&r=y", "I stayed at $apt_name, but do not have time to answer the questions");
      $links[] = html_link($link."&r=n", "I did not stay at $apt_name");
      $agent_string = $agentObj ? "<p>We appreciate that you may receive a similar request from the booking agent ($agentObj->name). As property operators, it is still very useful for us to receive direct customer feedback. Thank you for your understanding.</p>" : '';       
      $mail_body .= "
      <p>Thank you for choosing $site_name. We appreciate your business.</p>
      <p>Please help us improve our service by filling in our feedback form. It only takes a minute.</p>
      ".$agent_string.html_ul($links);
      break;
  
    case "payment_received":
      $paymentObj = $param1;
  
      $status = $paymentObj->payment_status;  
      $type = $paymentObj->payment_type;  
      $verified = $paymentObj->verified;  
      $gateway_id = $paymentObj->gateway_id;
      
      $status_name = get_object('payment_status', $status, 'name');
      $type_name = get_object('payment_type', $paymentObj->payment_type, 'name');
      $v_str = $verified ? "verified" : "unverified";
  
      //$receipt = payment_receipt($paymentObj, $feeObj);
      $amount_str = "$paymentObj->amount $paymentObj->currency on ".sql2human($paymentObj->created, array('show_time' => true));
  
      switch($user_level) {
        case USER_LEVEL_GUEST;
          $first_line = "Thank you for your $pay_type payment of {$amount_str}.";
          $first_line .= "<br>The status of your payment is <b>$status_name</b>.";
          break;
        case USER_LEVEL_OWNER;
          $first_line = "You received a <b>$status_name</b> $v_str $type_name payment of {$amount_str}.";
          break;
        case USER_LEVEL_STAFF;
          $first_line = "<p>A <b>$status_name</b> $v_str $type_name payment was made to $owner_name for {$amount_str}.";
          break;
      }
      $mail_body .= html_p($first_line);
  
      if($user_level == USER_LEVEL_GUEST &&
         $gateway_id == GATEWAY_PAYPAL && 
         in_array($paymentObj->payment_type, array(PAYMENT_TYPE_CREDIT_CARD, PAYMENT_TYPE_CREDIT_CARD_AUTH, PAYMENT_TYPE_CREDIT_CARD_CAPTURE))
         ) {
        $cc_text = payment_cc_text();
        $mail_body .= html_p(html_b($cc_text));
      }
  
      if($pay_desc = $paymentObj->description) $mail_body .= html_p("Notes: $pay_desc");
      break;
  
    case "transaction_received":
      $txObj = $obj;
      $subject = "[AFR Transaction] $txObj->res_id $txObj->mc_gross $txObj->mc_currency";
      $mail_body .= print_transaction_details($txObj);
      if($res_id = $txObj->res_id) $mail_body .= print_reservation(get_object('reservation', $res_id));
      break;
  
    case "relocation_advice":
      $city_id = ucfirst($obj->city_id);
      $city_name = get_city_name($city_id);
      $section = $obj->section;
      $question = $obj->question;
      $address = address_details($obj, true);
      $subject = "[AFR Relocation Advice] $city_name $section advice ($name)";
      $type = $obj->obj_type;
      $id = $obj->obj_id;
      if($type && $id) {
        $apt = get_object($type, $id);
        $property_str = print_property($type, $apt);
      }
  
      if($user_level >= USER_LEVEL_STAFF) { // STAFF, ADMIN
        $mail_body.= "
        <p>You have received a request for $city $section advice.</p>
        ";
      } else { // guest
        $mail_body.= "
        <p>Thank you for your interest in $city_name $section advice.</p>
        <p>One of our advisors will contact you within the next 48 hours.</p>
        ";
      }
  
      $mail_body.= "<p>".phrase('question', CAPITALIZE).": $question</p>";
      $mail_body.= "<p>".phrase('customer', CAPITALIZE).": $address</p>";
      $mail_body.= "<p>$property_str</p>";
      break;
  
    case "tell_a_friend":
      $type = $param1->type;
      $public_link = get_link($param1->type, $param1->apt_id, false, false);
      $public_link_url = replace_uri_var($public_link, 'ta', $param1->id);
      $public_link = html_link($public_link_url, $public_link);
      $apt = get_object($type, $param1->apt_id);
      $property_str = print_property($param1->type, $apt);
  
      $subject = $param1->apt_name;
      $prop_link = html_link($public_link_url, $param1->apt_name);
      $site = html_link($_SERVER['HTTP_HOST'], $_SERVER['HTTP_HOST']);
      $mail_body .= "
      <p>$param1->from is sending you this message from $site regarding the property $prop_link</p>
      <p>$property_str</p>
      <p>$public_link</p>
      <p>$param1->message</p>
      <p>$mail_str</p>
      ";
  
      break;
  
    case "property_alert":
      $paObj = $obj;
      $to = $paObj->email;
      $apt_id_list = $param1;
  
      $city_id = $paObj->city_id;
      $city_name = $paObj->city_name;
      $type = $paObj->type;
      $type_name = $type=='realestate' ? "real estate" : "rental";
      $now = $paObj->now;
      $filters = $paObj->filters;
      $guest_name = $paObj->first_name || $paObj->last_name ? "$paObj->first_name $paObj->last_name" : "Guest";
      $email = $paObj->email;
      $unsubscribe_link = property_alert_unsubscribe_link($paObj->id, $paObj->created);
      $apts_str = print_property_list($type, $param2, false, false);
  
      $subject = "[AFR Property Alert] $now: $city_name ($type)";
      $hits = $paObj->hits;
      if($limit = $paObj->limit) {
        // $all_link = "<a href=''></a>";
        // Click $all_link to see all $hits.
        $max_hits_str = "Showing only the first $limit. To refine your property alert, simply create a new one using the same email address [$email] and the old one will be overwritten.";
      }
  
      $mail_body .= "
      <p>$hits new $type_name properties were imported for $city_name that matched your search:</p>
      <p>$filters</p>
      $max_hits_str
      <hr />
      $apts_str
      <p>$unsubscribe_link</p>
      ";
      break;
  
    case "new_property_alert":
      $city_id = $param1->city_id;
      $city_name = geoname($city_id);
      $type = $param1->type;
      $type_name = $type=='realestate' ? "real estate" : "rental";
      $now = $param1->now;
      $filters = $param1->filters;
      $guest_name = $param1->first_name || $param1->last_name ? "$param1->first_name $param1->last_name" : "Guest";
      $unsubscribe_link = property_alert_unsubscribe_link($param1->id, $param1->created);
  
      $subject = "[AFR Property Alert] $now: $city_name ($type)";
  
      $mail_body .= "
      <p>Thank you for registering with Apartments-for-rent Property Alerts.</p>
      <p>We will send a mail when new $type_name properties are added for $city_name that matched your search:</p>
      <p>$filters</p>
      <hr>
      <p>$unsubscribe_link</p>
      ";
      break;
  
    default:
      return array(false, '', '', '', '');
      break;
    }
  }
  if(!$mail_body) return array();

  if($msg = $resObj->message ?: $obj->message) $mail_body.= html_p(txt2html($msg)); // optional message to guest
  
  if($user_level >= USER_LEVEL_OWNER) {
    if($msg = txt2html($obj->alert)) $mail_body .= html_p(html_b(phrase('NB!', CAPITALIZE)).html_break().html_b($msg)); 
    if($msg = txt2html($obj->notes)) $mail_body .= html_p(html_b(phrase('guest_notes', CAPITALIZE)).html_break().$msg); 
  }

  if($user_level >= USER_LEVEL_GUEST) {
    if($msg = txt2html($obj->extra_info)) $mail_body .= html_p(html_b(phrase('guest_notes', CAPITALIZE)).html_break().$msg); 
  } 
  
  
  // begin email output
  $greeting = "<h2>".phrase('dear', CAPITALIZE)." $name,</h2>";

  $body = mail_header($siteObj); // todo: customize for agents + affiliates
  if($obj_type == 'reservation' && $resObj && $user_level > USER_LEVEL_GUEST && !$resObj->send_email) $body .= html_p(html_b("This update was not sent to the guest, because 'send email' is not set."));
  $body .= $greeting;


  // if($message = $obj->message) $body .= $message;
  $body .= $mail_body;  // from the big switch statement

  // add request details
  if($reqObj) $body .= reservation_reply($reqObj, $obj_type, $user_level, $action_links, $options);

  // add footer
  if($mail_footer) $body .= $mail_footer;
  $body .= mail_footer($siteObj, $lang);

  $_SESSION['language'] = $session_lang; // restore session (screen) language

  return array($subject, $body, $from, $to, $replyto);
}

function sms_template($template, $obj_type, $obj, $param1=0, $param2=0) {
  global $siteObj;
  $from = $siteObj->cell_phone;
  if(!$from && $siteObj->legal_entity_id) $from = get_object('legal_entity', $siteObj->legal_entity_id, 'cell_phone'); 

  $body = '';
  switch($obj_type) {
    case 'reservation': 
      $resObj = $obj;
      $user_level = $param1;
      $langugage = $param2;    
      $url = public_action_link($resObj, $obj_type, array('user_level' => $user_level));      
      if($short_url = url_shorten($url)) $details = "Link: $short_url\n";
      $details .= print_array_assoc(reservation_details($resObj, $user_level), 'plaintext');
      if($site_id = $resObj->site_id) {
        $siteObj = get_object('site', $resObj->site_id);
        $from = $siteObj->cell_phone;
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


function property_alert_unsubscribe_link($id, $ts) {
  $u_s = phrase('unsubscribe', LOWERCASE);
  $host = $_SERVER['HTTP_HOST'];
  $ts = stripdate($ts);
  if(!$id) die("property_alert_unsubscribe_link: no id");
  if(!$ts) die("property_alert_unsubscribe_link: no ts");
  $link = "<a href='http://$host/unsubscribe/?cookie=".$ts.$id."'>$u_s</a>";
  return $link;
}

function mail_recipient($obj) {
  //echo("mail recipient: ");dump($obj);
  $name = get_name_string($obj);
  $email = $obj->email_address;

  if(!$email && ($obj->user_id || $obj->username)) {
    $userObj = $obj->username ? $obj : get_object('user', $obj->user_id);
    $name = get_user_name($userObj);
    $email = get_user_email($userObj);           
  }
  return array($name, $email);
}

function request_mail_subject($request_type, $reqObj, $shortname, $action='') {
  global $propertyTypeArray;
  global $reqTypeArray;

  $property_type = $reqObj->property_type;
  $property_type_name = $propertyTypeArray[$property_type];

  if($property_type == 'boat') { // special case
    $request= phrase('request', CAPITALIZE);
    $name = get_name_string($reqObj);
    $len = sidebar_length($reqObj->num_days, $reqObj->num_hours, false);
    $date = sql2human($reqObj->checkin);
    $subject = "$request $name - $date, $len @ $reqObj->apartment_name ";
    return $subject;
  }

  $request_type_name = $reqTypeArray[$request_type];

  //$action_str = phrase($action, CAPITALIZE); // later
  $action_str = $action ? ucfirst($action) : $property_type_name;
  //echo("ptype = $property_type, name=$property_type_name action=$action str=$action_str");dump($propertyTypeArray);

  $guest_name = get_name_string($reqObj);
  $key = get_object_key($request_type);
  $id = $reqObj->$key;
  $city_name = geoname($reqObj->city_id);

  if($count = $reqObj->num_days) {
    $units = $count == 1 ? 'day' : 'days'; // todo, allow hours
    $length_str = "- $count $units ";
  } elseif($reqObj->num_hours > 0) {
    $count = $reqObj->num_hours;
    $units = $count == 1 ? 'hour' : 'hours'; // todo, allow hours
    $length_str = "- $count $units ";
  }

  $subject = "[$shortname $action_str] $city_name $request_type_name $id ($guest_name ".$length_str."@ $reqObj->apartment_name)";

  if($_SESSION['aid'] && $shortname == 'AFR') { // add affiliate name only for AFR for now
    $aff_name = $_SESSION['aff_name'];
    $subject = "<$aff_name> ".$subject;
  }

  return $subject;
}

function reservation_reply($reqObj, $enq_type, $user_level, $action_links, $options=NULL) {
  $siteObj  = ($site_id  = $reqObj->site_id) ? get_object('site', $site_id) : $_SESSION['siteObj']; 
  //$action_links['correspond'] = phrase('correspond', CAPITALIZE);
  if(!$options['hide_details']) {
    // should we show the customer address ?
    if($user_level == USER_LEVEL_GUEST || $user_level >= USER_LEVEL_STAFF) { // guest or staff (or higher) - YES
      $show_address = true;
    } else { // owner, agent, affiliate, estate_agent, depends on status
      if($enq_type == 'reservation') {
        $show_address = $reqObj->status >= STATUS_RESERVED;
      } elseif($enq_type == 'enquiry') {
        $show_address = $reqObj->status >= ENQ_STATUS_CONFIRMED;
      } else { // shouldn't happen
        $show_address = false;
      }
    }
  
    if($remote_cc = $enqObj->remote_cc) {
      $cc_name = get_object('countries', $remote_cc, 'name');
    }
    if($remote_cc && $user_level >= USER_LEVEL_OWNER) $body .= print_array_assoc(array('Guest Location: ' => $cc_name), 'left').html_break(2);
  
    if($address = address_details($reqObj, $show_address)) $body .= print_array_assoc($address, 'left').html_break(2);
    $body .= print_array_assoc(request_details($enq_type, $reqObj, $user_level), 'left');

    /** print invoices */
    if(!$options['hide_accounting']) {
      if($enq_type == 'reservation' && ($reqObj->status == STATUS_PROPOSED || $reqObj->status == STATUS_RESERVED)) {
        if($invObjs = find_reservation_invoices($reqObj->res_id)) {
          $body .= html_break().html_h3(phrase('open_invoices', CAPITALIZE));
          foreach($invObjs as $invObj) {
            if($invObj->balance < INV_TOLERANCE) continue;
            $body .= payment_details($invObj, 0, array('context' => 'mail'));
          }
        }
      }
    }
  }

  // status link
  $link_options = array('user_level' => $user_level, 'full' => true);
  if(!$siteObj->require_user_login) $link_options['pin'] = true; // public link with pin (no login required)   
  $link =  action_link($enq_type, $reqObj, $link_options);
  
  $action_links_str = html_link($link, phrase('status', CAPITALIZE)).html_break(3);
  /**
  if(count($action_links)) {
    foreach($action_links as $action => $name) {
      $link = public_action_link($reqObj, $enq_type, array('user_level' => $user_level, 'action' => $action));
      $action_links_str .= html_link($link, $name).html_break();
    }
  }
  */
  if($options['hide_footer']) { // just show links
    $body .= html_break(2).$action_links_str;
  } else {    
    $link = public_action_link($reqObj, $enq_type, array('user_level' => $user_level, 'action' => 'correspond'));
    $body .= html_break(2).phrase('text_do_not_reply',0, array('search' => '__site_name', 'replace' => $siteObj->name));
    $body .= $action_links_str;

    /**
    if($user_level == USER_LEVEL_GUEST) {
      $url = site_pagetype_link($reqObj->site_id, PAGE_STATUS, $reqObj->language, true, true);
      if($_SESSION['test']) $url = str_replace('//test.', '//www.', $url);        
      $body .= html_break(2).phrase('text_mail_trouble', 0, array('search' => array('__url', '__enq_type'), 'replace' => array($url, ucfirst($enq_type)))).html_break();
    }
    */
  }
  return $body;
}

function google_map_link($lat, $long, $text='', $lang='en') {
  return html_link("http://maps.google.com/maps?q=".urlencode($lat).",".urlencode($long)."+(".urlencode($text).")&iwloc=A&hl=$lang", $text);
}

function afr_checkin_instructions($resObj) {
  if($site_id  = $resObj->site_id) $siteObj  = get_object('site', $site_id); 
  if($siteObj) {
    $site_name = $siteObj->name;
    $site_email = $siteObj->email_address;
    $terms_url = site_page_link($siteObj, PAGE_TERMS, array('full' => 1));
    if(!url_exists($terms_url)) $terms_url = "http://".$siteObj->url."/terms.html";
  } else {
    $site_name = 'Apartments-for-rent';
    $site_email = 'info@apartments-for-rent.com';
    $terms_url = 'http://www.apartments-for-rent.com/terms-and-conditions/';
  }
  if($terms_url && url_exists($terms_url)) {
    $terms_string = "
    <p>For our cancellation policy please look at: </p>
    <p><a href='$terms_url' target='_blank'>'$terms_url</a></p>";
  }


  $output = "
<p>
<h2>Arrival &amp; check in procedure</h2>

We will welcome you at your apartment to hand-over the keys and to answer any questions you might have.<br />
Therefore it is crucial for us to know at what time you will arrive so we can make our planning and nobody has to wait.<br />
<b>You can inform us by phone or email:</b><br />
<b>Phone:</b> +31 (0)6-57124869<br />
<b>Email:</b> <a href='mailto:$site_email' target='_blank'>$site_email</a><br />
<br />
<b>Check-in time:</b> From 15.00hrs. to 19.00hrs. (3 pm - 7 pm)<br />
Note: A late check-in fee of &euro;35 applies for arrivals after 19.00hrs (7 pm)<br />
<br />
<b>Check out time:</b> 11.00hrs (11 am)<br />
<br />
<b>Early arrival:</b><br />
If you want to be sure of an early check-in then we advise you to pre-book the apartment the night before arrival. If you arrive early but you do not wish to pre-book the apartment you can store your luggage at our office until the apartment is ready.<br />
<br />
<b>Late check-out:</b><br />
Check-out until 15.00hrs (3pm): Half the day price<br />
Check-out after 15.00hrs (3pm): Full day price<br />
<em>Note: A late check-out is always depending on availability.</em><br />
<br />
<h2>Pick up and arrival</h2>
<br />
<b>Airport</b><br />
We are happy to arrange an airport pick up for you, to do so we need to have your flight details and mobile phone number.<br />
The driver will meet you at the arrival gate with a sign with you name on it and bring straight from the airport to the apartment.<br />
If for any reason you cannot find him then you can call him on: +31651155060.<br />
Price: &euro;50,-<br />
<br />
<b>Train station</b><br />
Unfortunately cars cannot stop at central station. If you do wish to be picked-up you can take the train to Amsterdam-Schiphol station.<br />
Besides central station most international trains stop at this station.<br />
Our driver can pick you up from the meeting point and bring you straight to your apartment.<br />
We will need an arrival time and mobile number from you to book a pick-up.<br />
The number of the driver: +31651155060.<br />
Price: &euro;50,-<br />
Note: If for any reason you are delayed or missed your flight and you haven&#39;t informed us in time by phone then we are obliged to charge you for the total drivers fee of &euro;50,-.<br />
<br />
<b>Car</b><br />
When you arrive by car we need to have an estimated arrival time. If during the day your arrival time changes we ask you to contact us as soon as possible.<br />
We need you to call us 30minutes before you arrive at the apartment.<br />
Our number: +31 (0)6 57 12 48 69<br />
Please also give us a number where we can reach you on!<br />
<br />
We hope we have given you all the information you need. Should you still have any questions or remarks regarding your reservation then please do not hesitate to contact us.<br />
<br />
$terms_string
Have a lovely day.</p>
<p>The $site_name Team</p>
";
  return $output;
}

function airport_pickup_instructions($resObj) {
  $output = "
  <p>Upon arrival at the airport, you will be met by one of our chauffeurs and taken straight to the apartment
     you have rented, where your will be checked in. Make sure you provide us with your correct flight arrival
     details, otherwise we can not pick you up from the airport. <b>THIS IS VERY IMPORTANT!</b>
  <p>Also, please make sure you have provided us with a working cell phone number, and switch it on as soon as you
     get off the plane. If something happens to you or your luggage at the airport, the chauffeur can at least
     get hold of you and assist you with any problem that may arise.</p>
  <p>The standard procedure is that the driver waits for you (holding a sign with your name)
     after customs. Please use the exit closest to your luggage conveyor belt.</p>
  <p>If you do not find him there call him +31 651155060.</p>
  <p>The driver will wait for 45 minutes after actual landing time. If he has no
     contact with you he will go to the meeting point (there is only one and it has a
     red/white cube) after which he leaves the airport. Thus in case something goes
     wrong (lost luggage) please call within 45 minutes otherwise the driver is
     gone.</p>
  <p><b>IF YOU DO NOT ARRIVE BY PLANE PLEASE INFORM US IN ORDER TO MAKE ARANGEMENTS FOR CHECK IN.</b></p>
  ";
  return $output;
}

function missing_arrival_info_instructions($resObj) {
  $output = "
  <p>You have not yet indicated how you arrive (by plane, train, etc.). If you arrive by air and provide us with the
     details at least 48 hours prior to your arrival, we will pick you up at the airport <b>free of charge.</b></p>
  <p>If at this time you do not know your flight number, but still want airport pickup after your flight number
     is known, you can update your arrival information on the reservation status page (use the link at the bottom
     of this email.</p>
  <p>Below you will find the instructions for aiport pickup, <b>in case you do provide us with the necessary information on
     time - at least 48 hours prior to your arrival.</b></p>
  <p>&nbsp;</p>

  ";
  return $output;
}

function no_airport_pickup_instructions($resObj) {
  $output = "
  <p>If at this time you do not know your flight number, but still want airport pickup after your flight number
     is known, you can update your arrival information on the reservation status page (use the link at the bottom
     of this email.</p>
  ";
  return $output;
}

//  <p>You have chosen for airport pickup.</p>
//  <p>You have NOT chosen for airport pickup or it is not available for the apartment you have chosen.</p>
//  <p>The driver will be waiting for you outside customs with a sign with the name $name.</p>
//  <p>He will be expecting you to arrive on flight $resObj->arrival_number arriving $resObj->arrival_time.</p>


function generic_checkin_instructions($resObj) {
  switch($resObj->property_type) {
    case 'boat':
    $output = "
    <p>The boat operator will contact you with more specific instructions for where and when to meet.</p>
    <p>For self-operated boats, you must bring a valid photo ID, such as passport or driver's license. 
       A &euro; 100 Euro cash damange deposit is also required which will be refunded when you return the boat.</p>
    <p>Life jackets are provided for children on boats where this is applicable.</p>
    <p>If your arrival is cancelled or delayed, or in case of any emergencies, please make sure you contact the owner by phone and/or email.</p>
    <p>All of our Terms and Conditions are explained on our website.</p>
    ";
    $output = '';
    break;
    
    default:
    $output = "
    <p>The owner or somebody representing the owner will meet you at the apartment to check you in and give you keys.</p>
    <p>Please call the owner an hour before arrival at the apartment to let them know you are coming, even if you are on schedule.</p>
    <p>Since there is usually no reception at the apartment, the owner or check-in staff will often have to wait around for your arrival, so calling ahead will make their lives much easier.</p>
    <p>Check in time is from 3 PM onwards. If the apartment you have rented was not occupied the night before you arrived, you may request to check in early. If needed, please arrange this directly with the owner.</p>
    <p>Check out time is 11.00 AM. You can request to check out late and if the apartment you are staying in is not rented the next day, your request will be granted.</p>
    <p>On checking out, the apartment will need to be prepared for the next guests - if you do not leave on time the preparation team cannot do their work, and you may be charged for delaying them.</p>
    <p>If your arrival is cancelled or delayed, or in case of any emergencies, please make sure you contact the owner by phone and/or email.</p>
    <p>As a security bond, the owner may ask you to sign a one page check in agreement that permits your credit card to be charged reasonable costs for any damages or extra cleaning charges should they be required.</p>
    <p>All of our Terms and Conditions are explained on our website.</p>
    ";
    break;
  }
  return $output;
}

function final_checkin_instructions() {
  $output = "
  <p><b>PLEASE PRINT THIS EMAIL AND BRING IT WITH YOU!</b></p>
  <p>Have a safe trip and thank you for booking with us!</p>
  ";
  return $output;
}

function checkin_instructions($siteObj, $resObj, $aptObj) {
  if($resObj->property_type == 'boat') {
    $output = property_checkin_instructions($siteObj, $aptObj, $resObj);

    $contact_info = html_b(phrase('contact_information', CAPITALIZE)).': '.html_break();
    $contact_info .= phrase('name', CAPITALIZE). ': '.$siteObj->name.html_break();
    $contact_info .= phrase('phone', CAPITALIZE).': '.$siteObj->email_address.html_break();
    $contact_info .= phrase('email', CAPITALIZE).': '.$siteObj->work_phone.html_break();    
    $output .= html_p($contact_info);
    
    return $output;                                                        
  }

  $street_num = "$aptObj->address_street $aptObj->address_number";
  $lang = $resObj->language;
  $ownerObj = get_user($aptObj->host_id);

  // map
  $map_link = html_b(phrase('map', CAPITALIZE).": ").google_map_link($aptObj->location_lat, $aptObj->location_long, $aptObj->name." - $street_num", $resObj->language);

  // address
  $aptObj->address_suffix = '';
  $address = html_b(phrase('address', CAPITALIZE).": ").property_address($aptObj);

  // contact info
  $contact = html_b(phrase('host', CAPITALIZE)." ".phrase('contact_information', CAPITALIZE).":<br />");
  $contact .= phrase('name', CAPITALIZE).": ".get_name_string($ownerObj)."<br />";
  if($email = $ownerObj->email_address) $contact .= phrase('email', CAPITALIZE).": ".html_link('mailto:'.$email, $email)."<br/ >";
  if($phone = $ownerObj->cell_phone) $contact .= phrase('phone', CAPITALIZE).": ".$phone."<br/ >";

  // Find checkin instructions for this apartment
  $instructions = html_header(phrase('checkin_instructions', CAPITALIZE), 3);
  $instrSearch['property_id']        = $resObj->apt_id;
  // $instrSearch['property_type']      = $resObj->property_type;
  $instrSearch['type'] = DESC_TYPE_CHECKIN;
  $instrSearch['language']           = $lang;

  $instructions = $aptObj->host_id == 1284 ? afr_checkin_instructions($resObj) : generic_checkin_instructions($resObj);

  if($lang && ($insObj = find_object('property_description', $instrSearch))) { // Reservation language property checkin instructions
    $instructions .= $insObj->description;
  } else {
    unset($instrSearch['language']);
    if($insObjs = find_objects('property_description', $instrSearch)) $instructions .= $insObj->description; // Try w/o language
  }

  // begin output
  $output .= html_p($instructions);
  $output .= html_p($address);
  $output .= html_p($map_link);
  $output .= html_p($contact);
  //$output .= html_p($thank_you);

  return $output;

}

function property_checkin_instructions($siteObj, $aptObj, $resObj) {
  return property_description($siteObj, $aptObj->id, $resObj->language, array('type' => DESC_TYPE_CHECKIN));
}

function agent_address($param1) {
  $address=array();
  if($param1->address_street  ) $address[]=$param1->address_street;
  if($param1->address_postcode) $address[]=$param1->address_postcode;
  if($param1->address_city    ) $address[]=$param1->address_city;
  if($param1->address_country ) $address[]=$param1->address_country;
  $address = implode(', ', $address);

  if($param1->address_phone   ) $phone = "<tr><td><b>Phone:</b></td><td>$param1->address_phone</td></tr>";
  if($param1->address_fax     ) $fax   = "<tr><td><b>Fax:  </b></td><td>$param1->address_fax  </td></tr>";
  if($param1->address_url     ) $url   = "<tr><td><b>URL:  </b></td><td>$param1->address_url  </td></tr>";
  if($param1->address_email   ) $email = "<tr><td><b>Email:</b></td><td>$param1->address_email</td></tr>";

  $body = "
<table border='0' cellspacing='1' cellpadding='1'>
<tr><td><b>Name:</b></td><td>$param1->name</td></tr>
<tr><td><b>Address:   </b></td><td>$address           </td></tr>
$email
$phone
$fax
$url
</table>
";
  return $body;
}


function print_transaction_details($txObj) {
  $fields = array('txn_id','item_name','item_number','payment_status','payment_currency','payment_amount','receiver_email','payer_email');
  $rows = array();
  foreach($fields as $field) {
    $rows[] = array(phrase($field, CAPITALIZE), $txObj->$field);
  }
  return html_table($rows, array('title' => 'Transaction Details', 'header' => 0, 'header_col'=>  1));
}

function login_details($userObj) {
  $body = phrase('username', CAPITALIZE).": ".$userObj->username."<br>";

  //$body .= phrase('password', CAPITALIZE).":".$userObj->password."<br>";
  if($userObj->email_address) $body .= phrase('email', CAPITALIZE).": ".$userObj->email_address."<br>";
  
  if($providerProfileObjs = find_objects('provider_profile', array('user_id' => $userObj->id))) {
    $body .= phrase('or_login_using', CAPITALIZE); // todo: translate    
    foreach($providerProfileObjs as $providerProfileObj) {
      $providers[] = get_object('provider', $providerProfileObj->provider_id, 'name');
    }
    $body .= implode(',', $providers);
  }
  //$body .= phrase('password', CAPITALIZE).":".$userObj->password."<br>";
  return $body;
}

function mail_header($siteObj) {
  $url        = $siteObj->url;
  $site_name  = $siteObj->name;

  $html_start_afr = "
    <?xml version='1.0' encoding='utf-8'?><!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'>
    <html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>
    <head>
      <meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>
      <title>$site_name</title>
      <style type='text/css'>
        BODY { background-color: #fff; color: #111;
             font-family: Verdana, Arial, Helvetica, sans-serif;
             font-size:12px; font-style: normal; font-weight: normal; font-variant: normal;
             line-height: normal; text-align: left }
          A { font-family:arial;font-size:12px;font-weight:bold;font-style: normal; text-decoration: none; }
          A:link { color: #f7bf01; text-decoration: none; }
          A:visited { color: #f7bf01; text-decoration: none; }
          A:hover { color: #f7bf01; text-decoration: underline; }
          A:active { color: #f7bf01; text-decoration: underline; }
        table {font-size: 12px; margin-left:2px;margin-top:5px;margin-bottom:5px}
        th {font-weight: bold;}
      </style>
      <link REL='StyleSheet' HREF='http://$url/css/main.css' TYPE='text/css' MEDIA='screen'>
    </head>
    <body>
  ";
  return $html_start_afr;
}

function mail_footer($siteObj, $lang='') {
  if(!$lang) $lang = $_SESSION['language'];
  $url        = $siteObj->url;
  $site_name  = $siteObj->name;
  $logo       = $siteObj->logo;
  
  $protocol = $_SESSION['live'] && $siteObj->secure ? 'https' : 'http';
  if(strpos($logo, '//') === 0) $logo = "$protocol:$logo";
  
  //$show_logo  = $siteObj->show_mail_logo;
  $show_contact  = $siteObj->show_mail_contact;
  $call_us     = $siteObj->show_mail_call_us;
  $contact = get_name_string($siteObj);
  
  $mail_footer = '';
  if($contentObj  = find_object('site_content', array('site_id' => $siteObj->id, 'language' => $lang))) {
    $mail_footer = html_p($contentObj->mail_footer);
  }
  
  // dump($siteObj);
  //if($mail_footer) { // custom greeting
  //  $html_end_afr = $mail_footer.html_break();

  if(!$mail_footer) { // standard greeting
    $html_end_afr = html_p(phrase('text_please_contact_us', CAPITALIZE)).
                    html_p(phrase('sincerely_yours', CAPITALIZE).',');
    if($contact && $show_contact) $html_end_afr .= $contact.html_break(); // personal name from siteObj
  }  
  
  $html_end_afr .= $site_name.html_break();
  $html_end_afr .= html_link(add_http($url), $url).html_break();
  if($email = $siteObj->email_address) $html_end_afr .= "E: ".html_link("mailto:$email", $email).html_break();
  if($phone = $siteObj->work_phone) $html_end_afr .= "T: $phone".html_break();

  if($logo) {
    $logo_link = html_link(add_http($url), html_image($logo, $site_name, array("style" => "max-width: 200px")));    
    $html_end_afr .= html_break(2).html_p($logo_link);
  }
  //html_image_link(add_http($url), $logo, $site_name, array("style" => "max-width: 200px"))); // logo

  $html_end_afr .= "
    <p>
    <font size='1' face='Verdana'>
    $mail_footer
    =================================================================<br>
    The information contained in this message may be confidential and is intended to be<br>
    exclusively for the addressee. Should you receive this message unintentionally, please do<br>
    not use the contents herein and notify the sender immediately by returning this e-mail.<br>
    </font>
    </p>
    </body>
  </html>
  ";
  return $html_end_afr;
}

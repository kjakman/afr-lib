<?php
function iReserve_send($legal_entity_id, $method, $content, $send=true, $echo=false) {
  if(!$send) {
    echo("not sending. echo = $echo");
    $output .= html_h2("Not sending - Content").form_text('xmlDocument', print_r($content, true), array('rows' => 30, 'cols' => 150));
    if($echo) echo($output);
    return NULL;
  }
  
  //echo("reel $obj_type echo=$echo<br>");
  global $xml_dir;
  if(!$legal_entity_id) die("iReserve send: Legal entity ID required");
  if(!$content) die("iReserve send: Content missing");
  $legalObj = get_object('legal_entity', $legal_entity_id) ?: die("iReserve export: Legal entity not found for id=$legal_entity_id");
  $client_id = $legalObj->client_id ?: die("iReserve export: No client ID for $legalObj->name");
  // $username = $legalObj->iReserve_username ?: die("iReserve export: No iReserve username for $legalObj->name");
  // $password = $legalObj->iReserve_password ?: die("iReserve export: No iReserve password for $legalObj->name");
    
  if($_SESSION['dev']) {
    $username = 'beheerder';
    $password = 'mokumbootderden123!';
  }                                
  $md5_password = md5($password);
  $source_id = 25; // Teqa iReserve
  $url = "https://mokumboot.i-reserve.nl/api/soap";
  $wsdl = "https://mokumboot.i-reserve.nl/api/soap?wsdl";
                                        
  $output = "Starting iReserve $obj_type Import for Legal Entity $legal_entity_id ($legalObj->name)".html_break(); 
  $ts = stripdate(now());
  $base_dir = "$xml_dir/iReserve/$client_id/$legal_entity_id/";
  if(!is_dir($base_dir)) mkdir($base_dir, 0777, true) ?: die("Failed to make $base_dir");
    
  $options = array(
              'soap_version'=>SOAP_1_2,
              'exceptions'=>true,
              'trace'=>1,
              'cache_wsdl'=>WSDL_CACHE_NONE
          ); 
  
  $client  = new SoapClient($wsdl, $options);
  $client->__setLocation($url);

  $signature = "
  <Signature>
      <Username>$username</Username>
      <SignatureValue>$md5_password</SignatureValue>
      <Method>MD5</Method>
  </Signature>";

  //echo($output);           
  $result = $client->__soapCall($method, array(utf8_encode($content), $signature)); 
  $sent = $client->__getLastRequest();

  // $rh = $client->__getLastResponseHeaders());
  // $r = $client->__getLastResponse());
  
  $output .= html_h2("$method: Sent").form_text('xmlDocument', print_r($sent, true), array('rows' => 20, 'cols' => 150));
  $output .= html_h2("$method: Result").form_text('xmlDocument', print_r($result, true), array('rows' => 20, 'cols' => 150));

  $filename = "$base_dir/log.txt";
  $log_content = "$method: Sent on $ts\n $content\n $method: Result on $ts\n".print_r($result, true)."\n-----------------\n\n";
  $b = write_file($filename, $content.print_r($result, true), 'a');
  
  // $filename = "$base_dir/$method-$ts.xml";
  // $b = write_file($filename, $content.print_r($result, true));
  $output .= "Wrote $b bytes to $filename".html_break();
                                                                                                             
  if($echo) echo($output);                        
  return $result; // return success = true in result (or number of updates?)  
    
}

function iReserve_content($method, $resObj=NULL) {
  switch($method) {    
    case 'get_bookings':
      $today = today();
      // $today = add_day(today(), -365);
      $next_year = add_day(today(), 365);
      $content = "
      <Content>
          <FromDate>$today</FromDate>
          <TilDate>$next_year</TilDate>
      </Content>
      ";
      break;
  case 'update_customer':
    if(!$resObj->iReserve_customer_id) die("update_customer: no customer ID");
    return iReserve_content('add_customer', $resObj);
    break;
  case 'update_booking':
    if(!$resObj->iReserve_booking_id) die("update_booking: no booking ID"); 
    return iReserve_content('add_booking', $resObj);
    break;
  case 'booking_status_update':
    $booking_id = $resObj->fid ? : die("iReserve_content: $method: no booking ID");
    list($fstatus, $fstatus_name) = iReserve_get_status($resObj->status);
    $content = "
    <Content>
      <Booking ID=\"$booking_id\">
        <Status code=\"$fstatus\">$fstatus_name</Status>
      </Booking>
    </Content>";      
    break;
  case 'add_payment':
    $paymentObj = $resObj;
    list($pcode, $pcode_name) = iReserve_get_payment_code($paymentObj->payment_type);
    $booking_id = $resObj->iReserve_booking_id ? : die("iReserve_content: $method: no booking ID");
    if($customer_id = $resObj->iReserve_customer_id) $customer = "<Customer ID=\"$customer_id\"></Customer>";
    $status = $paymentObj->payment_status == PAYMENT_STATUS_COMPLETED ? "PAID" : "";
    $content = "
    <Content>
      <Payment>
        <Booking ID=\"$booking_id\"></Booking>
        $customer
        <Amount currency=\"$paymentObj->currency\">$paymentObj->amount</Amount>
        <Method code=\"$pcode\">$pcode_name</Method>
        <Transaction ID=\"$paymentObj->id\"></Transaction>
        <Remarks>$paymentObj->description</Remarks>
        <Status code=\"$status\"></Status>
      </Payment>
    </Content>";      
    break;

  case 'add_booking':
      //dump($resObj);die();
      if($customer_id = $resObj->iReserve_customer_id) {
        $booking_id = $resObj->iReserve_booking_id ? " ID=\"$resObj->iReserve_booking_id\"" : "";
        $customer = "<Customer ID=\"$customer_id\"></Customer>";
        $aptObj = get_object('property', $resObj->apt_id);
        $in_date = strip_time($resObj->checkin);
        $out_date = strip_time($resObj->checkout);
        $in_time = strip_date($resObj->checkin);
        $out_time = strip_date($resObj->checkout);
        
        $total = $resObj->grand_total_new_res ?: $resObj->grand_total;
        list($fstatus, $fstatus_name) = iReserve_get_status($resObj->status);
        $valid_status = $resObj->status >= STATUS_UNCONFIRMED ? "Y" : "N";                      
        $content = "           
      <Content>          
        <Booking$booking_id>
          <ExternalReference>$resObj->res_id</ExternalReference>
          <FromDate>".$in_date."</FromDate>
          <TilDate>".$out_date."</TilDate>
          <FromTime>".$in_time."</FromTime>
          <TilTime>".$out_time."</TilTime>
          <Product ID=\"$aptObj->guid\">$resObj->apartment_name</Product>
          $customer   
          <Status code=\"$fstatus\" valid=\"$valid_status\">$fstatus_name</Status>
          <Location></Location>
          <Number>1</Number>
          <Price currency=\"$resObj->currency\">$total</Price>
          <Remarks>$resObj->extra_info</Remarks>
          <RemarksIntern>$resObj->notes</RemarksIntern>
          <Additional></Additional>
          <Custom01>Test</Custom01>
        </Booking>
      </Content>
      ";
      } else {
        die("add_booking: Did not receive iReserve customer_id");
      }
  break;

  case 'add_customer':
      $sex = salutation2sex($resObj->salutation);
      $full_name = get_name_string($resObj);
      $house_num = '';
      $customer_id = $resObj->iReserve_customer_id ? " ID=\"$resObj->iReserve_customer_id\"" : "";
      // $first_name = str_pos$resObj->first_name;
      $first_name = strpos($resObj->first_name, '[AB]') === 0 ? $resObj->first_name : "[AB] $resObj->first_name"; 
      $content = "
      <Content>          
        <Customer$customer_id>
          <ExternalReference>$resObj->address_id</ExternalReference>
          <Initials>$first_name</Initials>
          <Prefix></Prefix>
          <Lastname>$resObj->last_name</Lastname>
          <Sex>$sex</Sex>
          <Fullname>$full_name</Fullname>
          <Email>$resObj->email_address</Email>
          <Postal>$resObj->zip</Postal>
          <Housenum>$house_num</Housenum>
          <Housenumext></Housenumext>
          <Address1>$resObj->address1</Address1>
          <Address2>$resObj->address2</Address2>
          <Address3></Address3>
          <City>$resObj->city</City>
          <Country>$resObj->country</Country>
          <TelHome>$resObj->cell_phone</TelHome>
          <TelMob>$resObj->cell_phone</TelMob>
          <TelWork>$resObj->cell_phone</TelWork>
          <TelFax>$resObj->fax</TelFax>
          <CompName>".($resObj->org_name ?: $resObj->organization)."</CompName>
        </Customer>
      </Content>
";
  break;

//          <CompTel></CompTel>
//          <CompFax></CompFax>
//          <MemberId></MemberId>
//          <SignupDate></SignupDate>
//          <ExpireDate></ExpireDate>
//          <BirthDate></BirthDate>
//          <IdNumber></IdNumber>
//          <IdType></IdType>
//          <Custom01></Custom01>
//          <Custom02></Custom02>
//          <Custom03></Custom03>
//          <Custom04></Custom04>
//          <Custom05></Custom05>
//          <Custom06></Custom06>
//          <Custom07></Custom07>
//          <Custom08></Custom08>
//          <Custom09></Custom09>
//          <Custom10></Custom10>
                                                
  default:
      die("Method '$method' not implemented");
      break;
   }
   return $content;
}

function iReserve_import($legal_entity_id, $method,  $send=true, $echo=false) {
  $content =  iReserve_content($method);
  return iReserve_send($legal_entity_id, $method, $content, $send, $echo);    
}

function iReserve_export($legal_entity_id, $method, $resObj, $send=true, $echo=false) {
  if(!$resObj) die("iReserve_export: Nothing to export");
  $content =  iReserve_content($method, $resObj);
  return iReserve_send($legal_entity_id, $method, $content, $send, $echo);    
}

// parses Vr Mei 07, 2010 into 2010-05-07
function iReserve_dutch2sql($date) {
  $months = array('Jan', 'Feb', 'Mrt', 'Apr', 'Mei', 'Jun', 'Jul', 'Aug', 'Sep', 'Okt', 'Nov', 'Dec');
  list($md, $y) = explode(', ', $date);
  list($wd, $mn, $d) = explode(' ', $md);
  $m = zero_pad(array_search($mn, $months) + 1); 
  return "$y-$m-$d";
}
                      
function iReserve_map_status($fstatus) {
  switch($fstatus) {
    case 10: return STATUS_UNCONFIRMED; //  => Nieuw
    case 20: return STATUS_PROPOSED;    //  => Open
    case 30: return STATUS_RESERVED;    //  => Open, betaald
    case 70: return STATUS_CANCELLED;   //  => Geannuleerd door gebruiker
    case 80: return STATUS_REJECTED;    //  => Afgewezen
    case 90: return STATUS_COMPLETE;    //  => Gesloten
    default: return 0;
  }
}                             

function iReserve_get_status($status) {
  switch($status) {
    case STATUS_EXPIRED:     return array(70, 'Geannuleerd door gebruiker');
    case STATUS_REJECTED:    return array(80, 'Afgewezen');
    case STATUS_CANCELLED:   return array(70, 'Geannuleerd door gebruiker');
    case STATUS_DECLINED:    return array(80, 'Afgewezen');
    case STATUS_UNCONFIRMED: return array(10, 'Nieuw');
    case STATUS_PROPOSED:    return array(20, 'Open');
    case STATUS_RESERVED:    return array(30, 'Open, betaald');
    case STATUS_BLOCKED:     return array(30, 'Gesloten');
    case STATUS_COMPLETE:    return array(30, 'Gesloten');
//    case STATUS_BLOCKED:     return array(90, 'Gesloten');
//    case STATUS_COMPLETE:    return array(90, 'Gesloten');
    default: return array(0, '');
  }
}

function iReserve_get_payment_code($payment_method) {
  switch($payment_method) {
    case PAYMENT_TYPE_CREDIT_CARD:         return array("CARDGATE", "Credit card");
    case PAYMENT_TYPE_CREDIT_CARD_AUTH:    return array("CARDGATE", "Credit card authorization");
    case PAYMENT_TYPE_CASH:                return array("CASH", "Paid in cash");
    case PAYMENT_TYPE_BANK:                return array("ABNAMRO", "Bank payment");
    case PAYMENT_TYPE_PAYPAL:              return array("PAYPAL", "PayPal");
    case PAYMENT_TYPE_ECHECK:              return array("PAYPAL", "PayPal eCheck");
    case PAYMENT_TYPE_IDEAL:               return array("IDEAL", "iDeal");
    case PAYMENT_TYPE_AGENT:               return array("CASH", "Paid to agent");
    case PAYMENT_TYPE_PROMISSORY:          return array("CASH", "Promissory payment");
    case PAYMENT_TYPE_PIN:                 return array("PIN", "PIN");
    
    // case PAYMENT_TYPE_CREDIT_CARD_CAPTURE: return array("CARDGATE", "Credit card capture");
    // case PAYMENT_TYPE_AGENT_OWES:          return array("", "");
    // case PAYMENT_TYPE_BLOCK:               return array("", "");
    // case PAYMENT_TYPE_REFUND:              return array("", "");
    // case PAYMENT_TYPE_TRANSFER:            return array("", "");
    // case PAYMENT_TYPE_CORRECTION:          return array("", "");
  }
}

function iReserve_xml($method, $resObj=NULL, $send=true, $echo=false, $test=false) {
  
	$obj = $resObj;
  $legal_entity_id = 1006;
  $source_id = SOURCE_ID_TEQA;
  $res_id = $resObj->res_id;
  $messages = array();
  $errors = array();
  if($test) {
    $result = read_file('/web/data/afr-dev/XML/iReserve/1002/1006/get_bookings-20110322120325.xml');
  } else {
  
    switch($method) {
      case 'add_customer':
        $result = iReserve_export($legal_entity_id, $method,  $resObj, $send, $echo);
        $xml = new SimpleXMLElement($result);
        if($xml->Response->Status != 'OK') {
          $errors[] = "iReserve_xml $method: Result Not OK"; 
          $errors[] = (string) $xml->Response->Message; 
          return array($messages, $errors, $resObj);
        }  
        $customer_xml = $xml->Content->Customer;
        $customer_id = (string) $customer_xml['ID']; 
        if(!$customer_id) {
          $errors[] = "iReserve_xml: $method: $cust_meth returned no ID"; 
          return array($messages, $errors, $resObj);
        }      
        break;              

      case 'update_customer':
        $customer_id = $resObj->iReserve_customer_id = $resObj->fgid;
        $booking_id = $resObj->iReserve_booking_id = $resObj->fid;
        if(!$customer_id) {
          $errors[] = "iReserve_xml $method: No customer_id on reservation"; 
          return array($messages, $errors, $resObj);
        }
        $result = iReserve_export($legal_entity_id, $method,  $resObj, $send, $echo);
        break;
        

      case 'booking_status_update':
        if(!$resObj->fid) {
          $errors[] = "iReserve_xml $method: No booking_id on reservation"; 
          return array($messages, $errors, $resObj);
        }
        $result = iReserve_export($legal_entity_id, $method, $resObj, $send, $echo);
        break;
        
      case 'update_booking':
        $customer_id = $resObj->iReserve_customer_id = $resObj->fgid;
        $booking_id = $resObj->iReserve_booking_id = $resObj->fid;
        if(!$booking_id) {
          $errors[] = "iReserve_xml $method: No booking_id on reservation"; 
          return array($messages, $errors, $resObj);
        }
        $result = iReserve_export($legal_entity_id, $method,  $resObj, $send, $echo);
        break;

      case 'get_booking_payments':
        $customer_id = $resObj->iReserve_customer_id = $resObj->fgid;
        $booking_id = $resObj->iReserve_booking_id = $resObj->fid;
        if(!$booking_id) {
          $errors[] = "iReserve_xml $method: No booking_id on reservation"; 
          return array($messages, $errors, $resObj);
        }
        $result = iReserve_export($legal_entity_id, $method,  $resObj, $send, $echo);
        break;

      case 'add_payment':
        $resObj = get_object('reservation', $res_id);
        $customer_id = $obj->iReserve_customer_id = $resObj->fgid;
        $booking_id = $obj->iReserve_booking_id = $resObj->fid;
        if(!$booking_id) {
          $errors[] = "iReserve_xml $method: No booking_id on reservation"; 
          return array($messages, $errors, $resObj);
        }
        $result = iReserve_export($legal_entity_id, $method,  $obj, $send, $echo);
        break;

      case 'add_booking':
        if($_SESSION['dev'] || $_SESSION['test']) die("No bookings allowed in test or dev mode");
        if($resObj->res_id && $resObj->dest_id == $source_id && $resObj->fid) { // this booking has already been added, use update
          $resObj->iReserve_customer_id = $resObj->fgid;
          $resObj->iReserve_booking_id = $resObj->fid;
          echo("add_booking: res_id=$resObj->res_id, dest_id=$resObj->dest_id, updating booking<br>");
          $cust_meth = $resObj->fgid ? 'update_customer' : 'add_customer';
          $method = 'update_booking';
        } else {
          $cust_meth = 'add_customer';     
        }
        $result = iReserve_export($legal_entity_id, $cust_meth,  $resObj, $send, $echo);
        $xml = new SimpleXMLElement($result);
        if($xml->Response->Status != 'OK') {
          $errors[] = "iReserve_xml: Method $method: Result Not OK"; 
          $errors[] = (string) $xml->Response->Message; 
          return array($messages, $errors, $resObj);
        }    
        $customer_id = $resObj->iReserve_customer_id = (string) $xml->Content->Customer['ID'] ?: $xml->Content->Customers->Customer['ID']; 
        if(!$customer_id) {
          $errors[] = "iReserve_xml: $method: $cust_meth returned no ID"; 
          return array($messages, $errors, $resObj);
        }        
        $result = iReserve_export($legal_entity_id, $method,  $resObj, $send, $echo);
        break;
      case 'get_bookings':
        $result = iReserve_import($legal_entity_id, $method,  $send, $echo);
        break;
      default:
        $errors[] = "Method $method not implented"; 
        return array($messages, $errors, $resObj);        
    }
  }
  
  if($send) {
    if(!$result) {
      $errors[] = "iReserve_xml: Send is on but no result";
      return array($messages, $errors, $resObj);
    }
    $result = str_replace('&', '&amp;', $result); // Teqa should have done this, not valid XML 
    $xml = new SimpleXMLElement($result);
    if($xml->Response->Status != 'OK') {
      $errors[] = "iReserve_xml: Method $method: Result Not OK"; 
      $errors[] = (string) $xml->Response->Message; 
      return array($messages, $errors, $resObj);
    }
    switch($method) {
      case 'get_bookings':
        //dump($xml);    
        $log_date = '2012-08-05';
        $date_log = array();
        foreach ($xml->Content->Bookings->Booking as $booking) {
          $apt_id=0;
          $data = array();
          $data['send_email'] = 0;
          $data['num_guests'] = 1;
          $data['site_id'] = SITE_ID_AFR;

          
          $data['fid'] = $fid = (string) $booking['ID'];
          $in = iReserve_dutch2sql($booking->FromDate);
          $out = iReserve_dutch2sql($booking->TilDate);
          $in_time = strlen($booking->FromTime) == 5 ? $booking->FromTime : '0'.$booking->FromTime;
          $out_time = strlen($booking->TilTime) == 5 ? $booking->TilTime : '0'.$booking->TilTime;
      
          $guid = (string) $booking->Product['ID'];
          if(!$guid) continue;
          
          $data['apt_id'] = $apt_id = find_object('property', array('source_id' => SOURCE_ID_TEQA, 'guid' => $guid), 'id');
          $data['apartment_name'] = $apt_name = (string) $booking->Product;
          $all_apts["$guid: ($apt_name)"]++;
          $aptAr[$guid] = "$guid: ($apt_name) = $apt_id";

          if(!$apt_id) {
            //die("get_bookings sid=$source_id guid=$guid");
            $messages[] = html_span("Couldn't find property $apt_name (source=$source_id guid=$guid)", 'red');
            //continue;
          }
          $fstatus = (string) $booking->Status['code'];
          $fstatus_name = (string) $booking->Status;
      
          $data['status'] = $status = iReserve_map_status($fstatus);    
          if(!$status) {
            $messages[] = "Couldn't find status $fstatus_name ($fstatus)";
            continue;
          }
          
          $checkin = $data['checkin'] = $in.' '.$in_time.':00';
          $checkout = $data['checkout'] = $out.' '.$out_time.':00';

          if($customer_id = (string) $booking->Customer['ID']) {
            $customer = $booking->Customer;
            $first_name = $data['first_name'] = (string) $customer->Initials;
            $last_name = $data['last_name'] = implode(' ', array_filter(array($customer->Prefix, $customer->Lastname)));
            $email = (string) $customer->Email;
            $data['email_address'] = is_email($email) ? $email : '';
            $data['address1'] = (string) $customer->Address1;
            $data['city'] = (string) $customer->City;
          } else {
            $data['first_name'] = '??';
          }

          if($log_date && $in == $log_date) $date_log[] = "$guid: ($apt_name) $in_time - $out_time status: $fstatus ($first_name $last_name)";

          $price = $booking->Price;
          $data['rate_type'] = 'total';
          $data['rate_base'] = (string) $price;
          $data['currency'] = (string) $price['currency'];
          //$statusAr[$status] = 
          $status_name = get_object('res_status', $status, 'name'); 
          $all[] = $data;

          if(!$apt_id) {
            //dump($booking);
            //dump($data);
          } else {

            //die("teqa=".SOURCE_ID_TEQA);
            if($resObj = find_object('reservation', array('fid' => $fid))) {              
              $res_id     = $resObj->res_id;
              $source_id  = $resObj->source_id;
              $dest_id    = $resObj->dest_id;
              //echo("Found $fid=$res_id<br>");
            } else {
              $res_id = $source_id = $dest_id = 0;
              echo("Could not find $fid<br>");
            }
              
            if($res_id) { 
              if($dest_id == SOURCE_ID_TEQA) { // update - our booking
                unset($data['rate_type']);
                unset($data['rate_base']);
                unset($data['currency']);
                list($rid, $errors) = update_object('reservation', $data, $res_id);
                $action = "Updated (our: $res_id)";
              } elseif($source_id == SOURCE_ID_TEQA) { // update - their booking                
                $data['source_id'] = $source_id;
                list($rid, $errors) = update_object('reservation', $data, $res_id);
                $action = "Updated (their: $res_id)";
              }
            } elseif($status >= STATUS_UNCONFIRMED) { // new booking (theirs)
              $data['source_id'] = SOURCE_ID_TEQA;
              //dump($data);die();
              list($rid, $errors) = add_object('reservation', $data);
              $action = "Added (their: $res_id)";
            } else { // skip new (cancelled) booking (theirs)
              $action = "Skipping (their new)";
            }
            
            if($errors) {
              return array($messages, $errors, $resObj);
            } else {
              $messages[] = "$action $status_name reservation $fid @ $apt_name ($apt_id):";
              $messages[] = "&nbsp; $checkin - $checkout ($first_name $last_name)";
              //$messages[] = "Their Status: $fstatus_name ($fstatus)";
            }
          }
        }
        asort($date_log);
        dump($date_log);
        dump($all_apts);
        ksort($aptAr);
        ksort($all_apts);
        dump($aptAr);
        //dump($statusAr);
        //dump($all);  
        break;
      
      case 'add_booking':
        $booking = $xml->Content->Booking;
        $booking_id = (string) $booking['ID'];
        $customer_id = (string) $booking->Customer['ID'];
        if($resObj->res_id) {
          if($fid = $booking_id) {            
            list($rid, $errors) = update_object('reservation', array('fid' => $fid, 'fgid' => $customer_id, 'dest_id' => $source_id), $resObj->res_id);
            if($errors) {
              dump($errors);
            } else {
              $messages[] = "Added booking $fid (reservation $rid)";
            }
          } else {
            $errors[] = "iReserve_xml: No ID Booking received from iReserve";        
            return array($messages, $errors, $resObj);
          }
        } else {
          $messages[] = "Added booking $fid (new reservation)";
        }
        break;
  
      case 'update_booking':
        $fid = (string) $xml->Content->Booking['ID'];
        $messages[] = "Updated booking $fid (reservation $res_id)";
        break;

      case 'update_customer':
        $messages[] = "Updated customer $customer_id";
        break;

      case 'add_customer':
        $messages[] = "Added customer $customer_id";
        break;
      
    }
  }
  $resObj->customer_id = $customer_id;
  $resObj->booking_id = $booking_id;  

  if($messages) print_log("iReserve $action for reservation $rid", 'ireserve');
  if($messages) print_log($messages, 'ireserve', LOG_INFO);
  if($errors) print_log($errors, 'ireserve', LOG_ERROR);
    
  return array($messages, $errors, $resObj);
}

?>

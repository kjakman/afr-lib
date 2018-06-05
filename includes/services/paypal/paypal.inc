<?php

// PayPal functions 
/**
 * Send HTTP POST Request
 *
 * @param	string	The API method name
 * @param	string	The POST Message fields in &name=value pair format
 * @return	array	Parsed HTTP Response body
 */
function PPHttpPost($methodName_, $nvpStr_) {
  global $__SITE;
  $siteObj = $__SITE;

	$version = '60.0';
	$live = $_SESSION['live'] || $_SESSION['live_payments_test'];
	//$live = 1;
  $API_UserName =  get_config(($live ? "ppu" : "pput"), $siteObj->id);
  $API_Password =  get_config(($live ? "ppp" : "pppt"), $siteObj->id);
  $API_Signature = get_config(($live ? "pps" : "ppst"), $siteObj->id);
  $API_Endpoint =  get_config(($live ? "ppe" : "ppet"), $siteObj->id);

  //echo("sl=".$_SESSION['live'].html_break());
  //echo("tp=".$_SESSION['live_payments_test'].html_break());
  //die("live=$live user=$API_UserName Endpoint=$API_Endpoint");
  
	// Set the API operation, version, and API signature in the request.
	$nvpreq = "METHOD=$methodName_&VERSION=".urlencode($version)."&PWD=".
	  urlencode($API_Password)."&USER=".urlencode($API_UserName)."&SIGNATURE=".urlencode($API_Signature)."$nvpStr_";
  //echo($nvpreq);

  $httpResponse = http_post($API_Endpoint, $nvpreq);
  $now = now();
  print_log("http_post: $now: $API_Endpoint", "paypal", LOG_LEVEL_ALWAYS);
  //print_log($nvpreq, "paypal", LOG_LEVEL_ALWAYS);
  print_log($httpResponse, "paypal", LOG_LEVEL_ALWAYS);
  
	// Extract the response details.
	$httpResponseAr = explode("&", $httpResponse);

	$httpParsedResponseAr = array();
	foreach ($httpResponseAr as $i => $value) {
		$tmpAr = explode("=", $value);
		if(sizeof($tmpAr) > 1) {
			$httpParsedResponseAr[$tmpAr[0]] = urldecode($tmpAr[1]);
		}
	}

	if((0 == sizeof($httpParsedResponseAr)) || !array_key_exists('ACK', $httpParsedResponseAr)) {
		exit("Invalid HTTP Response for POST request($nvpreq) to $API_Endpoint.");
	}

  if($dev && $methodName_ != 'SetExpressCheckout') {
     //echo("Sent method $methodName_ to $API_Endpoint:<br />$nvpreq<br />Result = $httpParsedResponseAr[ACK] <hr />");
     //dump($httpParsedResponseAr);
  }
	return $httpParsedResponseAr;
}

// makes an name-value pair string from lineitems, used by PayPal
function lineitems2nvp($lineitems) {
  //dump($lineitems);
  // add lineitems
  if($n_lineitems = sizeof($lineitems)) {
    for($i=0;$i < $n_lineitems;$i++) {
      if($desc = $lineitems[$i]->description) {
        $li_desc = $desc;
      } else {
        $type = $lineitems[$i]->type;    
        $typename = get_lineitem_type_name($type);
        $li_desc = phrase($typename);
      }
      //echo("Lineitem $i type is $type - $typename - $typename_localized<br>");
      $lineitem_str.="&L_NAME$i=".urlencode($li_desc);
      $lineitem_str.="&L_NUMBER$i=$i";
      $lineitem_str.="&L_QTY$i=".urlencode($lineitems[$i]->quantity);
      $lineitem_str.="&L_TAXAMT$i=".urlencode($lineitems[$i]->tax);
      $li_total = apply_discount($lineitems[$i]->amount, $lineitems[$i]->discount);
      $lineitem_str.="&L_AMT$i=".urlencode(number_format($li_total, 2));
      //$lineitem_str.="&L_CURRENCYCODE$i=".urlencode($lineitems[$i]->currency);
      $total += $lineitems[$i]->quantity * $li_total;
    }
    $lineitem_str .= "&ITEMAMT=".number_format(urlencode($total), 2);   
  }
  //echo("li total=$total<br>");
  return $lineitem_str;
}


/*************************************************
From PayPal's APIError.php

Displays error parameters.

Called by DoDirectPaymentReceipt.php, TransactionDetails.php,
GetExpressCheckoutDetails.php and DoExpressCheckoutPayment.php.

*************************************************/

function get_payment_error($resArray) {
  
  //$resArray=$_SESSION['reshash']; 
  
  $payment_error_str ="";
  
  if(isset($_SESSION['curl_error_no'])) { 
    //echo("curl error<br>");
  	$errorCode= $_SESSION['curl_error_no'];
  	$errorMessage= $_SESSION['curl_error_msg'];	
  
    $payment_error_str .="cURL Error: $errorCode ($errorMessage)";
  
  } else {
    //echo("pp error<br>");
  
  /* If there is no URL Errors, Construct the HTML page with 
     Response Error parameters.   
     */
  
    //$payment_error_str .="Gateway Error: Ack:$resArray[ACK] CorrelationID:$resArray[CORRELATIONID] Version:$resArray[VERSION] AVS:$resArray[AVSCODE] CVV2:$resArray[CVV2MATCH]";
    $payment_errors = array();
    $count=0;
    while (isset($resArray["L_SHORTMESSAGE".$count])) {
      $errorCode    = $resArray["L_ERRORCODE".$count];
      $shortMessage = $resArray["L_SHORTMESSAGE".$count];
      $longMessage  = $resArray["L_LONGMESSAGE".$count]; 
      $payment_errors[]= "Error $errorCode: $shortMessage ($longMessage)";	
      $count++;
    } 
  } 
  unset($_SESSION['curl_error_no']);
  unset($_SESSION['curl_error_msg']);
  return array($errorCode, implode(html_break(), $payment_errors));
}

// helpers to check validity and return data needed for set_express_checkout, do_express_checkout, and do_direct_payment
function check_pp_checkout($currency, $amount, $invObj, $options=NULL) {
  $inv_id = $invObj->id;
  $data = array();
  $error = '';
  
  if(!$inv_id) return array($data, "gateway check: no invoice id");
  $concerning = $invObj->concerning;
  
  $lineitems = $options['lineitems'] ?: get_lineitems($inv_id); 
  if($lineitems) $fractional = fractional_lineitems($lineitems) ? 1 : 0;

  $inv_cur = $invObj->currency;
  $inv_amount = $invObj->balance;
  $li_amount = number_format(sum_lineitems($lineitems), 2, '.', ',');  
  $li_count = count($lineitems);

  $parent_type = $invObj->parent_type;
  $parent_id = $invObj->parent_id;
  if(!$parent_type) return array($data, "gateway check: no parent type");
  if(!$parent_id) return array($data, "gateway check: no parent id");
  $parentObj = get_object($parent_type, $parent_id);
  if(!$parentObj) return array($data, "gateway check: could not find $parent_type $parent_id");
  if($currency != $inv_cur) return array($data, "gateway check: currency $currency != invoice currency $inv_cur");
  if($amount > $inv_amount + INV_TOLERANCE) return array($data, "gateway check: amount $amount > invoice amount $inv_amount");

  $site_id = $invObj->site_id ?: $parentObj->site_id;

  // all ok, get data
  switch($parent_type) {
    case 'reservation':
      $resObj = $parentObj;
      $res_id = $parent_id = $resObj->res_id;
      $custom = "site_id=$site_id,res_id=$res_id".($inv_id ? ",inv_id=$inv_id" : "");
      $description = $concerning ?: get_reservation_description($resObj);
      $phone = $resObj->phone1;
      $email  = $resObj->email_address;
      break;
    case 'payment_order':
      $orderObj = $parentObj;
      $order_id = $parent_id = $orderObj->id;      
      $custom = "site_id=$site_id,order_id=$order_id".($inv_id ? ",inv_id=$inv_id" : "");
      $description = $concerning ?: get_order_description($orderObj);
      $phone = $orderObj->phone1;
      $email  = $orderObj->email_address;
      break;
    case 'invoice':
      $custom = "site_id=$site_id,inv_id=$inv_id";
      $description = $concerning;
      $phone = $invObj->phone1;
      $email  = $invObj->email_address;
      break;
    default:
     return "gateway check: invalid parent type $parent_type";
  }

  $amount_due = get_due_now($invObj, $resObj);
  if($amount_due < $inv_amount && $amount == $amount_due) $data['invoice_suffix'] = '-deposit'; // guest paying deposit
  else if($amount < $inv_amount) $data['invoice_suffix'] = '-partial-'.now(); // paying custom amount
  
  if($amount == $li_amount && !$fractional) $data['lineitems'] = $lineitems;

  $data['li_amount'] = $li_amount;
  $data['inv_amount'] = $inv_amount;
  $data['parentObj'] = $parentObj;
  $data['custom'] = $custom;
  $data['description'] = $description;
  $data['phone'] = $phone;
  $data['email'] = $email;
  return array($data, $error);
}

/** verify txn id from paypal (for IPN or when returning from PayPal using standard payments */

function paypal_verify($data) {
  
  define("USE_SANDBOX", $_SESSION['dev'] ? 1 : 0); // Set to 0 once you're ready to go live
  $date = now();
  
  if(!$data) {
    print_log("$date paypal_verify: no data", 'ipn', LOG_LEVEL_ALWAYS); 
    return "NODATA";
  }
  
  $success = false;
  $errors = array();
  $test = USE_SANDBOX ? 1 : 0;
  $date = date('[Y-m-d H:i e]');
  
  // read the post from PayPal system and add 'cmd'
  $req = 'cmd=_notify-validate';
  if(function_exists('get_magic_quotes_gpc')) {
    $get_magic_quotes_exists = true;
  }
  foreach ($data as $key => $value) {
    if($get_magic_quotes_exists == true && get_magic_quotes_gpc() == 1) {
      $value = urlencode(stripslashes($value));
    } else {
      $value = urlencode($value);
    }
    $req .= "&$key=$value";
  }
  
  // Post IPN data back to PayPal to validate the IPN data is genuine
  // Without this step anyone can fake IPN data
  if(USE_SANDBOX) {
    $paypal_url = "https://www.sandbox.paypal.com/cgi-bin/webscr";
  } else {
    $paypal_url = "https://www.paypal.com/cgi-bin/webscr";
  }
  
  $ch = curl_init($paypal_url);
  if ($ch == FALSE) {
    print_log("Curl to $paypal_url failed!", 'ipn', LOG_LEVEL_ERROR);
    return FALSE;
    exit;
  }
  
  curl_setopt($ch, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER,1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $req);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 1);
  curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
  curl_setopt($ch, CURLOPT_FORBID_REUSE, 1);
  
  if(DEBUG == true) {
    curl_setopt($ch, CURLOPT_HEADER, 1);
    curl_setopt($ch, CURLINFO_HEADER_OUT, 1);
  }
  
  // Set TCP timeout to 30 seconds
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 30);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array('Connection: Close'));
  
  // CONFIG: Please download 'cacert.pem' from "http://curl.haxx.se/docs/caextract.html" and set the directory path
  // of the certificate as shown below. Ensure the file is readable by the webserver.
  // This is mandatory for some environments.
  
  //$cert = __DIR__ . "./cacert.pem";
  //curl_setopt($ch, CURLOPT_CAINFO, $cert);
  
  $txn_res = curl_exec($ch);
  if (curl_errno($ch) != 0) { // cURL error
    print_log("$date: Can't connect to PayPal to validate IPN message: " . curl_error($ch) . PHP_EOL, 'ipn', LOG_LEVEL_ERROR);
    curl_close($ch);
    exit;
  
  } else {
     // Log the entire HTTP response if debug is switched on.
     print_log("$date: HTTP request of validation request:". curl_getinfo($ch, CURLINFO_HEADER_OUT) ." for IPN payload: $req" . PHP_EOL,'ipn', LOG_LEVEL_TEST);
  
     // Split response headers and payload
     list($headers, $txn_res) = explode("\r\n\r\n", $txn_res, 2);
     print_log("$date: HTTP response of validation request txn_res='$txn_res'" . PHP_EOL,'ipn', LOG_LEVEL_ALWAYS);
  
     curl_close($ch);
  }
  return $txn_res;
}

/** old 
  $host_domains = explode(".", $_SERVER['HTTP_HOST']);
  $host_sub = $host_domains[0];  
  if($host_sub=='dev' || $_SESSION['test_payments']) { // sandbox or beta-sandbox, AFR's user
  	// Set up your API credentials, PayPal end point, and API version.
  	$API_UserName = urlencode('test_api1.kjakman.com');
  	$API_Password = urlencode('9DNL4THE3DJGVPZR');
  	$API_Signature = urlencode('AHM0ES8KQkLNTLUGu6Mn9YheM3e2Aw9sGf1N844wADix2PbdeRYYwn1x');
  	$API_Endpoint = "https://api-3t.sandbox.paypal.com/nvp";
    $dev = true;
  } else { // live, AFR's user
  	$API_UserName = $siteObj->paypal_api_username ? urlencode($siteObj->paypal_api_username) : urlencode('paypal_api1.kjakman.com');
  	$API_Password = get_config("pp", $siteObj->id);
  	$API_Signature = $siteObj->paypal_api_signature ? urlencode($siteObj->paypal_api_signature) :urlencode('A3z6y6uf0DXYqqNpbJWSQONEMSSOAkIoX47gmVQq7-5PdmJ7UhIhZ7W7');
  	$API_Endpoint = "https://api-3t.paypal.com/nvp";
    $dev = false;
  }

  */
?>
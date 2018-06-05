<?php

/**
 * This example assumes that a token was obtained from the SetExpressCheckout API call.
 * This example also assumes that a payerID was obtained from the SetExpressCheckout API call
 * or from the GetExpressCheckoutDetails API call.
 */

require_once 'paypal.inc';

//function set_express_checkout($paymentType, $currency, $amount, $return_url, $parentObj, $inputVars, $lineitems=0, $partial_payment=false) {

function set_express_checkout($currency, $amount, $return_url, $invObj, $options=NULL) {
  $inv_id = $invObj->id;

  list($data, $error) = check_pp_checkout($currency, $amount, $invObj, $options);
  if($error) return array("", $error);
  $amount = round($amount, 2);
  
  $li_amount = $data['li_amount'];
  $inv_amount = $data['inv_amount'];
  
  // options
  $partial_payment = $options['partial_payment'] ?: $data['partial_payment'];    
  $inputVars = $options['data'];
  $paymentType = $options['pp_action'] ?: "SALE";
  
  $lineitems = $data['lineitems'];
      
  print_log("set_express_checkout valid so far: cur=$currency inv_cur=$inv_cur amount=$amount inv_amount=$inv_amount li_amount=$li_amount li_count=$li_count return=$return_url pt=$parent_type pid=$parent_id inv=$inv_id", "payment");
  
  $host = $_SERVER['HTTP_HOST'];
  $host_domains = explode(".", $host);
  $host_sub = $host_domains[0];

  $environment = $host_sub == 'dev' ? 'sandbox' : 'live';
  switch($_SESSION['language']) {
  case 'nl':
    $localecode = 'NL';
  default:
    $localecode = 'US';
  }
  
  // required
  $nvpAr['Amt']           = $amount;
  $nvpAr['ReturnURL']     = $return_url."&act=paying";
  $nvpAr['CancelURL']     = $return_url."&act=pay&error_str=PayPal payment cancelled";
  $nvpAr['PAYMENTACTION'] = $paymentType;
  $nvpAr['CURRENCYCODE']  = $currency;

  // optional
  $nvpAr['DESC']          = $data['description'];
  $nvpAr['CUSTOM']        = $data['custom'];
  $nvpAr['INVNUM']        = $inv_id.$data['invoice_suffix'];
  $nvpAr['NOSHIPPING']    = 1;
  $nvpAr['ALLOWNOTE']     = 1;
  $nvpAr['LOCALECODE']    = $localecode ?: 'US'; // will support ES, DE, FR, IT - otherwise defaults to US

  // appearance
  $nvpAr['HDRIMG']        = $environment == 'live' ? $_SESSION['logo_url_small_secure'] : $_SESSION['logo_url_small'];
  
  //die($nvpAr['HDRIMG']);
  // $nvpAr['HDRBORDERCOLOR']= $_SESSION['theme_color'];
  $nvpAr['HDRBACKCOLOR']  = $_SESSION['box_medium_color'];
  $nvpAr['PAYFLOWCOLOR']  = $_SESSION['box_medium_color'];

  // others
  $nvpAr['EMAIL']         = $data['email'];
  
  $nvp_str = "&".array2qs($nvpAr, false);
  if($lineitems) $nvp_str .= lineitems2nvp($lineitems);

  //if($lineitems && $amount == $li_amount) $nvp_str .= lineitems2nvp($lineitems); // add lineitems if found and sum adds up

  //return array("", "inv=$inv_amount amount=$amount li=$li_amount li count=".count($lineitems));
  
  // Execute the API operation; see the PPHttpPost function above.
  $resArray = PPHttpPost('SetExpressCheckout', $nvp_str);
  if("Success" == $resArray["ACK"]) {
  	// Redirect to paypal.com.
  	$token = urldecode($resArray["TOKEN"]);
  	$payPalURL = "https://www.paypal.com/webscr&cmd=_express-checkout&token=$token";
  	if("sandbox" === $environment || "beta-sandbox" === $environment) {
  		$payPalURL = "https://www.$environment.paypal.com/webscr&cmd=_express-checkout&token=$token";
  	}
  	print_log("set_express_checkout: success! token=$token, redirecting to $payPalURL", "payment");
  	return array($payPalURL, "");
  } else  {
    list($error_code, $error) = get_payment_error($resArray);
  }
  print_log("set_express_checkout error:  $error");
  return array("", $error);
                            
}

function get_express_checkout($token) {
  if(!$token) die ("get_express_checkout: missing token");  
  return PPHttpPost('GetExpressCheckoutDetails', "&TOKEN=$token");
}


function do_express_checkout($currency, $amount, $invObj, $inputVars, $options=NULL) {
  $resArray = array();
  $paymentType = $options['pp_action'] ?: "SALE";
  $payerID = $inputVars['PayerID'];
  $amount = round($amount, 2);
  
  if(!$payerID) return array("do_express_checkout: no payerID", '', $resArray);

  list($data, $error) = check_pp_checkout($currency, $amount, $invObj, $options);
  if($error) return array($error, '', $resArray);
  $lineitems = $data['lineitems'];
      
  $nvpAr['TOKEN']         = htmlspecialchars($inputVars['token']);
  $nvpAr['PAYERID']       = $payerID;
  $nvpAr['PAYMENTACTION'] = $paymentType;

  $nvpAr['CURRENCYCODE']  = $currency;
  $nvpAr['AMT']           = $amount;

  // optional
  $nvpAr['DESC']          = $invObj->concerning;
  $nvpAr['CUSTOM']        = $data['custom']; // "res_id=$res_id,inv_id=$inv_id";
  $nvp_str = "&".array2qs($nvpAr, false);

  if($lineitems) $nvp_str .= lineitems2nvp($lineitems);
  
  // Execute the API operation; see the PPHttpPost function above.
  $resArray = PPHttpPost('DoExpressCheckoutPayment', $nvp_str);
  //dump($resArray);
  if("Success" == $resArray["ACK"]) {
    $error = 0;
    //$dp_msg = "PayPal Express Checkout Payment Completed Successfully";
    $dp_msg = "Payment: OK $resArray[TRANSACTIONID] Received: $resArray[CURRENCYCODE] $resArray[AMT]";
  } else  {
  	//echo('DoExpressCheckoutPayment failed:<br>');
    list($error, $dp_msg) = get_payment_error($resArray);
  }

  return array($error, $dp_msg, $resArray);      

}

//    [TOKEN] => EC-1Y596427DN357741T
//    [TIMESTAMP] => 2009-04-29T08:42:48Z
//    [CORRELATIONID] => ce2992c02cb4a
//    [ACK] => Success
//    [VERSION] => 51.0
//    [BUILD] => 892334
//    [TRANSACTIONID] => 0RW38864UV295390M
//    [TRANSACTIONTYPE] => expresscheckout
//    [PAYMENTTYPE] => instant
//    [ORDERTIME] => 2009-04-29T08:42:44Z
//    [AMT] => 323.80
//    [FEEAMT] => 9.74
//    [SETTLEAMT] => 417.32
//    [TAXAMT] => 0.00
//    [CURRENCYCODE] => EUR
//    [EXCHANGERATE] => 1.32879
//    [PAYMENTSTATUS] => Completed
//    [PENDINGREASON] => None
//    [REASONCODE] => None

?>
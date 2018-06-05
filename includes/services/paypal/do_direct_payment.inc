<?php
/***********************************************************
DoDirectPaymentReceipt.php

Submits a credit card transaction to PayPal using a
DoDirectPayment request.

The code collects transaction parameters from the form
displayed by DoDirectPayment.php then constructs and sends
the DoDirectPayment request string to the PayPal server.
The paymentAction variable becomes the PAYMENTACTION parameter
of the request string.

After the PayPal server returns the response, the code
displays the API request and response in the browser.
If the response from PayPal was a success, it displays the
response parameters. If the response was an error, it
displays the errors.

Called by DoDirectPayment.php.

Calls CallerService.php and APIError.php.

***********************************************************/

require_once 'paypal.inc';

function do_direct_payment($currency, $amount, $invObj, $cc_id, $options=NULL) {
                   
  global $u_id;
  $debug = false;
  if($_SESSION['dev']) $debug = true;
  
  $resArray = array();
  $dp_msg = $dp_error = '';
  
  $amount = round($amount, 2);
  $now = $ts = now();
  
  $paymentAction = $options['payment_action'] ?: 'Sale';
  $inv_id = $invObj->id ? $invObj->id : die("do_direct_payment: no inv id");;
  if($block_id = $invObj->block_id) $blockObj = get_object('block', $block_id);
  $res_id = $invObj->res_id ? $invObj->res_id : $blockObj->res_id; // die("do_direct_payment: no res id");;

  print_log("$ts: do_direct_payment $paymentAction $currency $amount on inv_id=$inv_id res_id=$res_id", 'paypal', LOG_LEVEL_ALWAYS);
  
  if(!$cc_id) return array("do_direct_payment: no cc id", $dp_msg, $resArray);
  $ccObj = get_object('cc', $cc_id);
  if(!$ccObj) return array("do_direct_payment: cc not found", $dp_msg, $resArray);
    
  list($data, $error) = check_pp_checkout($currency, $amount, $invObj, $options);
  if($error) return array(1, $error, $resArray);


  $cc_type = get_object('cc_type', $ccObj->cc_type, 'name');
  if($cc_type=="American Express") $cc_type="Amex";  // Paypal wants 'Amex'

  // same code in request.php - verify
  if($cc_type == 'Amex' && $currency != "USD") { /* Hack to use USD instead of EUR, GBP, etc for AMEX */
    $payment_currency = "USD";
    $payment_amount = get_exchange_rate($currency, 'USD', $amount, true);
    if(!$payment_amount) return array("Couldn't get USD exchange rate for $currency $amount", '', array());
  } else { // pay currency == invoice currency
    $payment_currency = $currency;
    $payment_amount   = $amount;
  }

  // from cardholder to first + last 
  $nameAr     = explode(' ', $ccObj->cc_cardholder);
  $last_name  = array_pop($nameAr);
  $first_name = implode(' ', $nameAr);

  $nvpAr['PAYMENTACTION']   = $paymentAction;
  $nvpAr['CREDITCARDTYPE']  = $cc_type;
  $nvpAr['ACCT']            = $ccObj->cc_number;
  $nvpAr['EXPDATE']         = str_pad($ccObj->cc_exp_month, 2, '0', STR_PAD_LEFT).$ccObj->cc_exp_year; // Month must be padded with leading zero
  $nvpAr['CVV2']            = $ccObj->cc_cvv2;
  $nvpAr['FIRSTNAME']       = $first_name;
  $nvpAr['LASTNAME']        = $last_name;
  $nvpAr['STREET']          = $ccObj->address1;
  $nvpAr['STREET2']         = $ccObj->address2;
  $nvpAr['CITY']            = $ccObj->city;
  $nvpAr['STATE']           = $ccObj->state;
  $nvpAr['ZIP']             = $ccObj->zip;
  $nvpAr['COUNTRYCODE']     = $ccObj->country;
  $nvpAr['PHONENUM']        = $ccObj->phone;
  $nvpAr['EMAIL']           = $ccObj->email_address;

  $nvpAr['CURRENCYCODE']    = $payment_currency;
  $nvpAr['AMT']             = $payment_amount;
  $nvpAr['IPADDRESS']       = $_SERVER['REMOTE_ADDR'];
  $nvpAr['DESC']            = $description = $invObj->concerning;
  $nvpAr['CUSTOM']          = "inv_id=$inv_id,cc_id=$cc_id";
  if($res_id) $nvpAr['CUSTOM'] .= ",res_id=$res_id";
  if($order_id) $nvpAr['CUSTOM'] .= ",order_id=$order_id";
  if($block_id) $nvpAr['CUSTOM'] .= ",block_id=$block_id";
  
  $nvpAr['INVNUM'] = $inv_id.$data['invoice_suffix'];

  $nvp_str = "&".array2qs($nvpAr, false);

  //return array(1, dump($nvpAr, true), $resArray);

  $lineitems = $data['lineitems'];
  
  /** turned off for now due to rounding errors */
  // if($lineitems && $payment_currency == $invObj->currency) $nvp_str .= lineitems2nvp($lineitems);
  
  //echo("do_direct_payment.inc: nvp_str='".str_replace("&", "<br>", $nvp_str)."'");
  if($debug)  echo("$nvp_str");
  print_log("$ts: do_direct_payment: nvp=$nvp_str", 'paypal', LOG_LEVEL_ALWAYS);
  
  $resArray = PPHttpPost('DoDirectPayment', $nvp_str);
  $ack = strtoupper($resArray["ACK"]);

  if($debug) dump($resArray);

  print_log("DoDirectPayment: $now:", "paypal", LOG_LEVEL_ALWAYS);
  print_log($nvpAr, "paypal", LOG_LEVEL_ALWAYS);
  print_log($resArray, "paypal", LOG_LEVEL_ALWAYS);
  
  if(strpos($ack, "SUCCESS")===0) // Success or SuccessWithWarning
  {                
    $charge_status = $invObj->charge_status = 0;
    $dp_msg = "Payment: OK $resArray[TRANSACTIONID] Amt: $resArray[CURRENCYCODE] $resArray[AMT] AVS: $resArray[AVSCODE] CVV2: $resArray[CVV2MATCH]";
    $error = 0;
  } else {
    $_SESSION['reshash']=$resArray;
    list($error, $dp_msg) = get_payment_error($resArray);
    $charge_status = $invObj->charge_status = 30; // TRANSACTION_NOT_COMPLETE
  }

  print_log("$ts: msg=$dp_msg error=$error", 'paypal', LOG_LEVEL_ALWAYS);
  
  // copy result from PayPal
  $map = array(
    'charge_status' =>       'status',
    'res_id' =>              'res_id',
    'paymentType' =>         'paymentaction',
    'currency' =>            'currencycode',
    'amount' =>              'amount',
    'description' =>         'description',
    'errorStr'    =>         'payment_error',
    'TIMESTAMP' =>           'remote_timestamp',
    'CORRELATIONID' =>       'correlationid',
    'ACK' =>                 'ack',
    'VERSION' =>             'version',
    'BUILD' =>               'build',
    'AVSCODE' =>             'avscode',
    'CVV2MATCH' =>           'cvv2match',
    'TRANSACTIONID' =>       'transactionid',
    'AMT' =>                 'amt'
  );
  
  $chargeData = map_array($resArray, $map, false, true); // passthru=false, addslashes = true

  // and add own
  $chargeData['cc_id']          = $cc_id;
  $chargeData['res_id']         = $res_id;
  $chargeData['inv_id']         = $inv_id;
  $chargeData['paymentaction']  = $paymentAction;
  $chargeData['currency']       = $payment_currency;
  $chargeData['amount']         = $payment_amount;
  $chargeData['description']    = addslashes($description);
  $chargeData['charge_status']  = $charge_status;
  $chargeData['payment_error']  = $error ? addslashes("$error: $dp_msg") : "";
  
  print_log("$ts: do_direct_payment: adding charge:".dump($chargeData, true), 'paypal', LOG_LEVEL_ALWAYS);
  
  list($charge_id, $charge_errors) = add_object('charge', $chargeData); // charges are always added, not updated

  if($charge_errors) {
    $error = implode(html_break(), $charge_errors);
  } else {
    $resArray['charge_id'] = $charge_id;
  }

    
  //die($nvp_str);
  //$li_amount = number_format(sum_lineitems($lineitems), 2, '.', ',');  
  //$error = "amount=$currency $payment_amount. sum li=$li_amount";

  return array($error, $dp_msg, $resArray);
}

?>
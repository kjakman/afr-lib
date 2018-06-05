<?php
/***********************************************************

***********************************************************/

require_once 'paypal.inc';

function do_capture($currency, $amount, $completeType, $note, $paymentObj, $partial_payment=false) {
  $inv_id = $paymentObj->inv_id;
  
  // Set request-specific fields.
  $nvpAr['AUTHORIZATIONID'] = $paymentObj->txn_id;
  $nvpAr['AMT']             = number_format($amount, 2);
  $nvpAr['CURRENCYCODE']    = $currency;
  $nvpAr['COMPLETETYPE']    = $completeType ;				
  if($note) $nvpAr['NOTE']  = $note;
  
  // $nvpAr['INVNUM']          = $partial_payment ? $inv_id.'-deposit' : $inv_id;

  $nvpStr = "&".array2qs($nvpAr, false);
  // echo("<hr>$nvpStr<hr>");
  $resArray = PPHttpPost('DoCapture', $nvpStr);

  $ack = strtoupper($resArray["ACK"]);
  
  if(strpos($ack, "SUCCESS")===0) // Success or SuccessWithWarning
  {
    // $invObj->charge_status = 0;
    // add_charge($invObj, $resArray, $u_id);
    $dp_msg = "DoCapture: OK $resArray[TRANSACTIONID] Amt: $currencyCode $resArray[AMT]";
    return array(0, $dp_msg, $resArray);  
  } else {
    list($error, $dp_msg) = get_payment_error($resArray);
    return array($error, $dp_msg, $resArray);      
  }
  
}

?>
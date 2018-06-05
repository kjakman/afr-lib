<?php
/***********************************************************

***********************************************************/

require_once 'paypal.inc';

function do_refund($siteObj, $currency, $amount, $refundType, $note, $paymentObj) {

  // Set request-specific fields.
  $nvpAr['TRANSACTIONID']   = $paymentObj->txn_id;
  $nvpAr['REFUNDTYPE']      = $refundType ;		 // 'Full' or 'Partial'		
  $nvpAr['AMT']             = number_format($amount, 2);
  $nvpAr['CURRENCYCODE']    = $currency;
  if($note) $nvpAr['NOTE']  = $note;
  if($refundType ==  'Partial') {
  	if(!isset($amount)) return array("Amount required for partial refunds", $dp_msg, $resArray);
  	if(!isset($note))   return array("Customer notes required for partial refunds", $dp_msg, $resArray);
  }

  $nvpStr = "&".array2qs($nvpAr, false);
  $resArray = PPHttpPost('RefundTransaction', $nvpStr);
  $ack = strtoupper($resArray["ACK"]);
  // dump($nvpAr);
  // dump($resArray);
  if(strpos($ack, "SUCCESS")===0) // Success or SuccessWithWarning
  {        
    $dp_msg = "OK. Refunded $currency $amount";
    return array(0, $dp_msg, $resArray);  
  } else {
    list($error, $dp_msg) = get_payment_error($resArray);
    return array($error, $dp_msg, $resArray);      
  }
  
}

// Array
// (
//     [REFUNDTRANSACTIONID] => 1G1594168D3515928
//     [FEEREFUNDAMT] => 0.39
//     [GROSSREFUNDAMT] => 10.00
//     [NETREFUNDAMT] => 9.61
//     [CURRENCYCODE] => EUR
//     [TIMESTAMP] => 2010-03-04T10:49:30Z
//     [CORRELATIONID] => fdb8ddbec9813
//     [ACK] => Success
//     [VERSION] => 60.0
//     [BUILD] => 1212010
// )

?>
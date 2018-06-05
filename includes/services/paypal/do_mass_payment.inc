<?php
/***********************************************************

***********************************************************/

require_once 'paypal.inc';

function do_single_payment($currency, $amount, $receiverEmail, $subject, $note='', $uniqueID='') {
  print_log("Single Payment $now cur=$currency amount=$amount recever=$receiverEmail subject=$subject note=$note", 'withdraw', LOG_LEVEL_ALWAYS);
  $receiverData['receiverEmail'] = $receiverEmail;
  $receiverData['amount'] = $amount;
  if($note) $receiverData['note'] = $note;
  if($uniqueID) $receiverData['uniqueID'] = $uniqueID;
  $receiversArray[] = $receiverData;

  return do_mass_payment($currency, $receiversArray, $subject);
}

function do_mass_payment($currency, $receiversArray, $subject) {
  global $u_id;
  // dump($u);
  // echo("u-id = $u[id]");
  $test = false;
  if($u_id != 501) return array(1, "This feature is disabled for now...", $resArray);
  
  // Set request-specific fields.
  $emailSubject =urlencode('example_email_subject');
  $receiverType = urlencode('EmailAddress');

  $nvpAr['EMAILSUBJECT']   = $subject;  
  $nvpAr['RECEIVERTYPE']   = 'EmailAddress';
  $nvpAr['CURRENCYCODE']   = $currency;
  
  // Add request-specific fields to the request string.
  $nvpStr="&EMAILSUBJECT=$emailSubject&RECEIVERTYPE=$receiverType&CURRENCYCODE=$currency";
      
  foreach($receiversArray as $i => $receiverData) {
    $nvpAr['L_EMAIL'.$i]      = $receiverData['receiverEmail'];
    $nvpAr['L_AMT'.$i]        = $receiverData['amount'];
    if($receiverData['uniqueID']) $nvpAr['L_UNIQUEID'.$i]   = $receiverData['uniqueID'];
    if($receiverData['note'])     $nvpAr['L_NOTE'.$i]       = $receiverData['note'];
  }

  // dump($nvpAr);
  $nvpStr = "&".array2qs($nvpAr, false);
  
  if($test) {
    dump($nvpStr);
    return array("Simulated withdraw $currency $receiverData[amount] to $receiverData[receiverEmail]", $dp_msg, $resArray);        
  } else {
    $resArray = PPHttpPost('MassPay', $nvpStr);
    $ack = strtoupper($resArray["ACK"]);
  }
  
  $now = now();
  print_log("Mass Payment $now cur=$currency: data=".dump($receiversArray, true), 'withdraw', LOG_LEVEL_ALWAYS);
  print_log("Mass Payment response=".dump($resArray, true), 'withdraw', LOG_LEVEL_ALWAYS);
  //print_log("NVP=".dump($nvpAr, true), 'withdraw', LOG_LEVEL_ALWAYS);
  
  //dump($resArray);
  if(strpos($ack, "SUCCESS")===0) // Success or SuccessWithWarning
  {        
    foreach($receiversArray as $i => $receiverData) {
      $dp_msgs[] = "OK. Sent $currency $receiverData[amount] to $receiverData[receiverEmail].";
    }
    $dp_msg = implode(html_break(), $dp_msgs);
    return array(0, $dp_msg, $resArray);  
  } else {
    list($error, $dp_msg) = get_payment_error($resArray);
    return array($error, $dp_msg, $resArray);      
  }
  
}
?>
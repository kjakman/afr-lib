<?php
require_once('set_env.inc');
require_once('paypal.inc');
  
//echo("gh");die();
// CONFIG: Enable debug mode. This means we'll log requests into 'ipn.log' in the same directory.
// Especially useful if you encounter network errors or other intermittent problems with IPN (validation).
// Set this to 0 once you go live or don't require logging.
//define("DEBUG", 1);
//define("LOG_FILE", $_SESSION['log_dir']."ipn.log");

$date = now();

// Read POST data
// reading posted data directly from $_POST causes serialization
// issues with array data in POST. Reading raw POST data from input stream instead.
$raw_post_data = file_get_contents('php://input');
$raw_post_array = explode('&', $raw_post_data);
$myPost = array();
foreach ($raw_post_array as $keyval) {
  $keyval = explode ('=', $keyval);
  if (count($keyval) == 2) $myPost[$keyval[0]] = urldecode($keyval[1]);
}

print_log("$date: IPN received\n raw data=".dump($raw_post_array, true), 'ipn', LOG_LEVEL_ALWAYS);
print_log("$date: IPN received\n data=".dump($myPost, true), 'ipn', LOG_LEVEL_ALWAYS);

$txn_res = paypal_verify($myPost);
print_log("$date: IPN txn_res=$txn_res", 'ipn', LOG_LEVEL_ALWAYS);
  

// Inspect IPN validation result and act accordingly

if (strcmp ($txn_res, "VERIFIED") == 0) {
  // check whether the payment_status is Completed
  // check that txn_id has not been previously processed
  // check that receiver_email is your PayPal email
  // check that payment_amount/payment_currency are correct
  // process payment and mark item as paid.

  // assign posted variables to local variables
  $txArray = $_POST;
  $item_name = $_POST['item_name'];
  $item_number = $inv_id = $_POST['item_number'];
  
  /** hack to look up real invoice ID from invoice number */
  if($inv_id) {
    $invObj = get_object('invoice', $inv_id);
    if($invObj) {
      print_log("Found invoice id=$inv_id from real ID $item_number", 'ipn', LOG_LEVEL_ALWAYS);
    } else {
      if($invObj = find_object('invoice', ['invoice_id' => $inv_id])) {
        $inv_id = $invObj->id;
        print_log("Found invoice id=$inv_id from invoice number $item_number", 'ipn', LOG_LEVEL_ALWAYS);        
      } else {
        print_log("Did not find invoice number $item_number", 'ipn', LOG_LEVEL_ALWAYS);        
      }
    }
  }
  
  
  $payment_status = $_POST['payment_status'];
  $payment_amount = $_POST['mc_gross'];
  $payment_currency = $_POST['mc_currency'];
  $txn_id = $_POST['txn_id'];
  $receiver_email = $_POST['receiver_email'];
  $payer_email = $_POST['payer_email'];

  if(!$errors) $success = true;  
  if(DEBUG == true) {
    print_log("$date: Verified IPN: $req ". PHP_EOL,'ipn', LOG_LEVEL_ALWAYS);
  }
} else if (strcmp ($txn_res, "INVALID") == 0) {
  // log for manual investigation
  // Add business logic here which deals with invalid IPN messages
  print_log("$date: Invalid IPN: $req" . PHP_EOL,'ipn', LOG_LEVEL_ERROR);
}


// save	locals to	tx object
$txArray['tx_gateway'] = "PAYPAL";
$txArray['txn_res']	=	$txn_res;

// $txArray['tx_notes'] .= "host=$_SERVER[HTTP_HOST] db=$db_db order=$txArray[invoice] inv_id=$inv_id";

// insert	transaction	if HTTP	succes
if($success) {
                        
  $tx_data = $txArray; // txArray = incoming. tx_data is saved
	$tx_type = $txn_type = $tx_data['tx_type'] = $txArray['txn_type']; // PayPal uses txn_type, we store as tx_type
  $tx_data['ipn_test'] = $test ? 1 : 0;
  
  // for masspay, add one transaction for each recipient
  if($tx_type == 'masspay') {
    $tx_str .= "Type = 'masspay' (One or more transactions)\n";
    //echo($tx_str.html_break());write_file($log, $tx_str,	'a');
    
    $count=1;
    $tx_data['sender_email'] = $txArray['payer_email'];
    while (isset($txArray["masspay_txn_id_".$count])) {
      $tx_data['txn_id']         = $txn_id = $txArray["masspay_txn_id_".$count];
      $tx_data['receiver_email'] = $txArray["receiver_email_".$count];
    	$tx_data['payment_status'] = find_object('payment_status', array('name' => $txArray["status_".$count]), 'id'); // Map to internal object

      if(isset($txArray["mc_currency_".$count])) {
        $tx_data['mc_currency'] = $txArray["mc_currency_".$count];
        $tx_data['mc_fee']      = $txArray["mc_fee_".$count];
        $tx_data['mc_gross']    = $txArray["mc_gross_".$count];
      } else { // no mc_ for USD - todo: test this
        $tx_data['mc_currency'] = 'USD';
        $tx_data['mc_fee']      = $txArray["payment_fee_".$count];
        $tx_data['mc_gross']    = $txArray["payment_gross_".$count];
      }
      
      $uniqueID = $txArray["unique_id_".$count];
      list($foo, $payment_id) = explode('=', $uniqueID);
      $customData = qs2array($uniqueID);
      $withdrawal_id = $customData['wid'];
      $payment_id = $customData['pid'];
      //$site_id = $customData['pid'];
      
      if($withdrawal_id) { // multiple payments
      } elseif($payment_id) { // single payment
        // $tx_data['parent_txn_id'] = $payment_id;
        $paymentObj = get_object('payment', $payment_id);
        $inv_id = $txArray['inv_id'] = $paymentObj->inv_id;
        $txn_res_id = $txArray['res_id'] = $paymentObj->res_id;
        $order_id = $txArray['order_id'] = $paymentObj->order_id;
      }

      $txArray['txn_id'] = $txn_id; // needed for check transaction
    	list($tx_st, $notes) = check_transaction($txArray);			
    	$tx_data['tx_st']	=	$tx_st;
    	$tx_data['tx_notes'] = $notes;
    	$tx_st_string = get_object('tx_status', $tx_st, 'name');
      $tx_str .= "Transaction $count: txn: $txn_id  status=$tx_st_string\n";
      if($notes) $tx_str .= "Notes=$notes\n";
      $tx_str .= "res_id=$txn_res_id, inv_id=$inv_id, order_id=$order_id, payment_id=$payment_id\n";
      //echo($tx_str.html_break());write_file($log, $tx_str,	'a');
       
      $tx_data['withdrawal_id'] = $withdrawal_id;
      $tx_data['payment_id'] = $payment_id;
      if($site_id) $tx_data['site_id'] = $site_id;
      $tx_data['inv_id'] = $inv_id;
      $tx_data['res_id'] = $txn_res_id;
      $tx_data['order_id'] = $order_id;
      $tx_data['cc_id']  = $cc_id;
      $tx_data['owner_id'] = get_object('invoice', $inv_id, 'owner_id');
      $tx_data['host_id'] = get_object('invoice', $inv_id, 'host_id');
    	list($tx_id, $errors)	=	add_object('transaction',	$tx_data);
    	if($errors)	{
        $tx_str .= "Failed to add transaction ($txn_id)\n";        
        $tx_str .= print_r($errors, true);
        //echo($tx_str.html_break());write_file($log, $tx_str,	'a');
    	  //dump($errors);
      } else {
        $tx_str .= "Added transaction $tx_id  ($txn_id)\n";
        //echo($tx_str.html_break());write_file($log, $tx_str,	'a');
      }
      $count++;
    }
  } else { // just one transaction
  	
    $txn_id = $txArray["txn_id"];     
  	$tx_str .= "One transaction: Type = '$tx_type'\n";
    //echo($tx_str.html_break());write_file($log, $tx_str,	'a');

    $custom = trim($txArray['custom']);
    if($customArray = explode(',',	$custom)) { // split custom	field (site_id, inv_id, res_id, order_id)
      foreach($customArray as	$ca) {
      	list($key, $val) = explode('=',	$ca);
      	$key = trim($key);
      	$val = trim($val);
      	if(strlen($val)) $txArray[$key]	=	$val;
      }
    } elseif($custom) { // look for number in string, assume it's invoice ID
      $txArray['inv_id'] = find_first_int($txArray['invoice']);
    }

    if(!$txn_type && $txArray['reason_code'] == 'refund') $tx_data['tx_type'] = 'refund'; // PayPal doesn't send txn_type for refunds
    
    if(!$txArray['inv_id'] && $txArray['invoice']) { // invoice ID is explicitly set (e.g. in Virtual Termincal "Order Number", extract
      $txArray['invoice_id'] = $txArray['invoice']; // store full textual invoice id
      $txArray['inv_id'] = find_first_int($txArray['invoice']);
    }
    
    $site_id = $txArray['site_id'];
    $inv_id = $txArray['inv_id'];
    $txn_res_id = $txArray['res_id'];
    $order_id = $txArray['order_id'];
    $cc_id  = $txArray['cc_id'];
    
    // no res_id, get from invoice
    if($inv_id) {
      if($invObj = get_object('invoice', $inv_id)) {
        $txArray['inv_id'] = $inv_id; // store valid internal invoice id
        if(!$txn_res_id) $txArray['res_id'] = $txn_res_id = $invObj->res_id;
        if(!$cc_id)  $txArray['cc_id']  = $cc_id = $invObj->cc_id;
      } else {
        // unset($txArray['inv_id']);
      }
    }
  	
  	list($tx_st, $notes) = check_transaction($txArray, $pp_email);			
  	$tx_data['tx_st']	=	$tx_st;
  	$tx_data['tx_notes'] = $notes;
  	$tx_data['payment_status'] = find_object('payment_status', array('name' => $txArray['payment_status']), 'id'); // Map to internal object

  	$tx_st_string = get_object('tx_status', $tx_st, 'name');
    $tx_str .= "Transaction details: txn: $txn_id  status=$tx_st_string\n";
    if($notes) $tx_str .= "Notes=$notes\n";
    $tx_str .= "res_id=$txn_res_id, inv_id=$inv_id, payment_id=$payment_id\n";
    //$tx_str .= "Transaction details: status=$tx_st notes=$notes\nres_id=$txn_res_id, inv_id=$inv_id\n";

    $tx_data['site_id'] = $site_id;
    $tx_data['inv_id'] = $inv_id;
    $tx_data['res_id'] = $txn_res_id;
    $tx_data['order_id'] = $order_id;
    $tx_data['cc_id']  = $cc_id;
    $tx_data['owner_id'] = get_object('invoice', $inv_id, 'owner_id');

  	list($tx_id, $errors)	=	add_object('transaction',	$tx_data);
  	if($errors)	{
      $tx_str .= "Failed to add transaction ($txn_id)\n";
      $tx_str .= print_r($errors, true);
  	  //dump($errors);
    } else {
      $tx_str .= "Added transaction $tx_id ($txn_id)\n";
    }
  }
} else {
  $tx_str .= "Failure - Did not add transaction.\n";
}

$tx_str ."\n\n-------------------------------------\n\n";
$size	=	strlen($tx_str);
print_log($tx_str, 'ipn', LOG_LEVEL_ALWAYS);

exit;        

?>


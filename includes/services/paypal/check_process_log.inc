<?


// check transaction
if($txn_res==$success) {
  list($tx_st, $notes) = check_transaction($txArray, $pp_email);      
} else {         
  $tx_st = get_tx_status_id('TXN_NOT_VERIFIED');
  $notes = "Transaction not verified by PayPal Request: $req ";
}  

// process payment
if($tx_st == get_tx_status_id('OK')) {
  // $status = receive_payment($txArray['item_number'], $txArray['mc_gross']); // process payment     
  // todo: send mail to us and client
}
   
// logging

$txArray['tx_st'] = $tx_st;
$txArray['tx_notes'] = $notes;
$txArray['item_name'] = urldecode($txArray['item_name']);

insert_transaction($txArray); // log transaction to db

//update_inv_payment($txArray); // update invoice 

$txArrayDump = print_r($txArray, true);
print_debug("<h3>Transaction Object</h3> <pre>$txArrayDump</pre>");

// send email to self with transaction
$to_address = 'payments@apartments-for-rent.com';
$from_address = 'admin@apartments-for-rent.com';
if($rid=$txArray['item_number']) {
  $resObj = get_object('reservation', $rid);
}

tx_writeToFile($txArray);
$doc_root = $_SERVER['DOCUMENT_ROOT'];
$logfile = $doc_root."/logs/txdump.log";
write_file($logfile, $txArrayDump, 'a');
?>
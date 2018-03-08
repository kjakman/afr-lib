<?php

/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_charge($order, $resArray, $user=0, $errorStr="") {
   $description = $order->description ? $order->description  : $order->item_name;
   $query_str = "INSERT INTO charge
                (status, user, res_id,
                 paymentaction, creditcardtype, acct, expdate, cvv2,
                 firstname, lastname, street, street2, city, state, zip, countrycode,
                 currencycode, amount, description,
                 remote_timestamp, correlationid, ack, version, build, avscode, cvv2match, transactionid, amt, payment_error)
         VALUES ('$order->charge_status', $user, '$order->res_id',
                 '$order->paymentType', '$order->cc_type', '$order->cc_number', '$order->exp_date', '$order->cc_cvv2',
                 '$order->first_name', '$order->last_name', '$order->address1', '$order->address2', '$order->city', '$order->state', '$order->zip', '$order->country',
                 '$order->currency', '$order->balance', '$description',
                 '$resArray[TIMESTAMP]', '$resArray[CORRELATIONID]', '$resArray[ACK]', '$resArray[VERSION]',
                 '$resArray[BUILD]', '$resArray[AVSCODE]', '$resArray[CVV2MATCH]','$resArray[TRANSACTIONID]', '$resArray[AMT]', '$errorStr'
                  )";
   db_query($query_str);
}

// return number of days between date1 and date2
/** unsused - automatic comment added on  */
function get_charges($res_id=0) {
  global $db;
  $query = "SELECT * from charge";
  if($res_id) $query .= " WHERE res_id=$res_id";
  return db_getAll($query);
}

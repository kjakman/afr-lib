<?php

// returns all lineitems for invoice if currency and amount match
// returns lineitems due now for invoice if currency and amount match
function payment_lineitems($invObj, $currency, $amount) {
  // try all lineitems (for description)
  if($currency != $invObj->currency) return array();

  // try all lineitems
  $lineitems = get_lineitems($invObj->id);
  $sum_li    = sum_lineitems($lineitems);
  if(abs($sum_li - $amount) <= INV_TOLERANCE) return $lineitems;
    
  // try due lineitems
  //$lineitems = get_due_lineitems($invObj);
  //$sum_li    = sum_lineitems($lineitems);
  //if(abs($sum_li - $amount) <= INV_TOLERANCE) return $lineitems;

  return array();
}

// returns sum of completed payment minus refuns for give object
// if authorized = true, returns sum of authorizations
function get_object_paid($obj_type, $obj_or_id, $authorized=false) {
  global $db;
  switch($obj_type) {
  case 'reservation':
    $key = $parent_key = 'res_id';
    break;
  case 'payment_order':
    $key = 'id';
    $parent_key = 'order_id';
    break;
  case 'invoice':
    $key = 'id';
    $parent_key = 'inv_id';
    break;
  default:
    die("get_object_paid: unsupported obj_type $obj_type");
  }
  
  $obj = any2obj($obj_type, $obj_or_id);
  if(!$obj) return;
  $id = $obj->$key;  

  // old: $paid_statuses = implode(',', array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PENDING, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED));

  $paid_statuses = $authorized ? PAYMENT_STATUS_PENDING : implode(',', array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED));
  $qs = "SELECT sum(amount) FROM payment WHERE $parent_key=$id AND payment_status IN ($paid_statuses)";
  $paid = db_getOne($qs);                                          
  return $paid > 0 ? $paid : '0.00';
}

function get_reservation_authorized($resObj_or_id) {
  return get_object_paid('reservation', $resObj_or_id, true);
}

function get_reservation_paid($resObj_or_id) {
  return get_object_paid('reservation', $resObj_or_id);
}


function get_paid($orderObj_or_id) {                 
  return get_object_paid('payment_order', $orderObj_or_id);
}

function update_reservation_paid($resObj_or_id) {
  $ts = now();
  $resObj = any2obj('reservation', $resObj_or_id);
  if(!$resObj) return;

  $res_id = $resObj->res_id;
  $paid = $resObj->paid = get_reservation_paid($resObj);
  $authorized = $resObj->authorized = get_reservation_authorized($resObj);
  $balance = $resObj->balance = $resObj->grand_total - $paid;
  print_log("$ts: update_reservation_paid: rid=$res_id authorized=$authorized paid=$paid balance=$balance", 'payment', LOG_LEVEL_ALWAYS);
  
  //db_query("UPDATE reservation SET paid='$paid', authorized='$authorized', balance='$balance' WHERE res_id=$res_id");
  update_object("reservation", array("paid" => $paid, "authorized" => $authorized, "balance" => $balance), $res_id);
  
  update_reservation_status($resObj); 
  
  return $paid;
}

function update_paid($order_id_or_obj) {
  $orderObj = any2obj('payment_order', $order_id_or_obj);
  $order_id = $orderObj->id;
  if(!$order_id) return false;
  $amount = $orderObj->total;
  
  $orderData['paid'] = $paid = get_paid($order_id);
  $orderData['balance'] = $balance = $amount - $paid;
      
  // echo("uop id=$order_id ($orderObj->status): paid=$paid balance=$balance");
  $status = 0;
  if($balance < INV_TOLERANCE) {
    $status = ORDER_STATUS_COMPLETE;
  } else if($paid > 0) { 
    $status = ORDER_STATUS_PARTIAL_PAYMENT;
  }
  if(!$status || $status == $orderObj->status) {
    $orderObj->paid = $paid;
    $orderObj->balance = $balance;
    return $orderObj; // no changes
  }
  
  $orderData['status'] = $status;  
  list($id, $errors) = update_object('payment_order', $orderData, $order_id);
  if($errors) print_log("update_payment_order: updating order errors:".print_r($errors, true), 'payment_order');
  else print_log("update_payment_order: updated order $id with total=$amount paid=$paid  balance=$balance", 'payment_order');

  update_order_parent($orderObj); // updates cart with order iD

  return get_object('payment_order', $order_id);                         
  //} elseif($cart_id = $orderObj->cart_id) { // redundant, get rid of ?
  //  update_object('cart', array('order_id' => $order_id), $cart_id); // means cart will be saved, new created on next login
  //}
}

// update corresponding parent cart
function update_order_parent($orderObj) {  
  $parent_type = $orderObj->parent_type;
  $parent_id = $orderObj->parent_id;
  if($parent_type && $parent_id) {
    switch($parent_type) {
    case 'cart':
      update_object('cart', array('order_id' => $orderObj->id), $parent_id); // means cart will be saved, new created on next login
      break;
    default:
      break;
    }
  }
}

// updates a payment order after a payment is added or updated
/** unsused - automatic comment added on  */
function update_payment_order($paymentObj) {  
  $order_id = $paymentObj->order_id;
  $completed = $paymentObj->payment_status == PAYMENT_STATUS_COMPLETED ? 1 : 0;
  print_log("update_payment_order: payment_id=$paymentObj->id order_id=$order_id completed=$completed", 'payment_order');
  if($order_id && $completed) {
    $orderObj = get_object('payment_order', $order_id);
    if($orderObj->currency != $paymentObj->payment_currency) {
      print_log("update_payment_order: order currency ($orderObj->currency) <> payment currency ($paymentObj->payment_currency) ", 'payment_order', LOG_LEVEL_ERROR);
    }
  }
}

// For PayPal express: this checks payment and performs (get_express/do_express)
// For refund: issue the refund (if PayPal)
// For the others: payment was OK, add to payment objects, update invoice, etc
// amount_usd field is hack to allow receiving USD amount different than invoice amount/currency
function add_payment($currency, $amount, $invObj, $payment_type, $inputVars, $paymentObj=NULL) {
  $ts = now();
  $inv_id = $invObj->id;
  $res_id = $invObj->res_id;
  
  print_log("$ts: add_payment $currency $amount type=$payment_type inv_id=$inv_id res_id=$res_id", 'payment', LOG_LEVEL_ALWAYS);
  
  $paymentData = array();
  $draft = $invObj->status == INV_STATUS_DRAFT;
  if($order_id = pick_first($inputVars['order_id'], $invObj->order_id, $paymentObj->order_id)) $paymentData['order_id'] = $order_id;
  
  $lineitems = payment_lineitems($invObj, $currency, $amount);
  
  $cur = get_currency_symbol($currency, true);
  if($payment_type != PAYMENT_TYPE_CREDIT_CARD_CAPTURE && $amount <= 0) return array("1", "Balance = $cur $amount, nothing to collect."); // nothing to be collected
  $paymentData['host_id'] = $invObj->host_id;

  $copy_vars = array('res_id', 'host_id', 'site_id', 'client_id', 'legal_entity_id');
  foreach($copy_vars as $var) $paymentData[$var] = $invObj->$var;

  if(isset($inputVars['payment_type'])) $payment_type = $inputVars['payment_type']; // ovverridden in process_payment for promissory payments
    
  $gateway_id = payment_type2gateway_id($payment_type);
  
  // Collect payment   
  switch($payment_type) {
    case PAYMENT_TYPE_PROMISSORY:
      $paymentData['payment_status'] = PAYMENT_STATUS_PENDING;
      $paymentData['payment_currency'] =  $currency;
      $paymentData['payment_gross'] =     $amount;
      $paymentData['payment_fee'] =       0;
      $paymentData['payment_net'] =       $amount;
      $paymentData['payment_notes'] =     '';
      break;

    case PAYMENT_TYPE_AGENT:
      $paymentData['payment_status'] = PAYMENT_STATUS_COMPLETED;
      $paymentData['payment_currency'] =  $currency;
      $paymentData['payment_gross'] =     $amount;
      $paymentData['payment_fee'] =       0;
      $paymentData['payment_net'] =       $amount;
      $paymentData['payment_notes'] =     '';
      break;

    case PAYMENT_TYPE_BITCOIN: // if we get here, we're already done (processed in process_payment)
      if($order_id) $orderObj = get_object('payment_order', $order_id);
      if(!$orderObj) {
        $payment_error = "Could not find order $order_id";
      } else {
        $paymentObj = find_object('payment', array('order_id' => $order_id));
        if(!$paymentObj) {
          $payment_error = "Could not find a payment for order $order_id";
        } else {
          if($paymentObj->payment_status != PAYMENT_STATUS_COMPLETED) {
            $payment_error = "Payment status for order $order_id is $paymentObj->payment_status (not complete)";
          } else if($paymentObj->payment_gross >= $orderObj->total) {
            $payment_message = "Thank you for your payment of ".clean_num($paymentObj->payment_gross)." BTC. Paid in full.";
          } else {
            $payment_error = "Insufficient payment: received $paymentObj->payment_gross vs order $orderObj->total";
          }
        }
      }

      // log
      if($payment_error) print_log("get_payment (bitcoin): ".$payment_error, 'payment', LOG_LEVEL_ERROR);
      else print_log("get_payment (bitcoin): ".$payment_message, 'payment', LOG_LEVEL_INFO);

      return array($payment_error, $payment_message); // payment is already added, so return
      break;

    case PAYMENT_TYPE_CREDIT_CARD:
      // echo("get_payment: method='$payment_type' type=$paymentType<br>");
      $paymentAction = $inputVars['payment_action'];
      $paymentData['payment_status'] = strtolower($paymentAction) == 'sale' ? PAYMENT_STATUS_COMPLETED : PAYMENT_STATUS_PENDING; // never pending for cc "Sale" payments. Will only be inserted if success (below)
      if($paymentAction == 'Authorization') $payment_type = PAYMENT_TYPE_CREDIT_CARD_AUTH; // set to authorization for internal payment object
      $paymentData['cc_id']         = $inputVars['cc_id'];
      $paymentData['charge_id']     = $inputVars['charge_id'];
      $paymentData['txn_id']        = $inputVars['txn_id'];
      break;

    case PAYMENT_TYPE_CREDIT_CARD_CAPTURE:
      require_once("paypal/do_capture.inc");
      $completeType = $inputVars['close_auth'] ? "Complete" : "NotComplete";
      list($charge_error, $dp_msg, $resArray) = do_capture($currency, $amount, $completeType, $inputVars['capture_note'], $paymentObj);
      if($charge_error) {
        $payment_error = $dp_msg;
      } else { // successfully captured payment
        $payment_id = $paymentObj->id;
        $payment_message = $dp_msg;
        $paymentData['payment_status'] = PAYMENT_STATUS_COMPLETED;
        // $paymentData['payment_status'] =    $resArray['PAYMENTSTATUS'];
        $paymentData['payment_currency'] =  $resArray['CURRENCYCODE'];
        $paymentData['payment_gross'] =     $resArray['AMT'];
        $paymentData['payment_fee'] =       $resArray['FEEAMT'];
        $paymentData['payment_net'] =       $resArray['AMT'] - $resArray['FEEAMT'];
        $paymentData['payment_notes'] =     $inputVars['capture_note'];
        //$paymentData['settle_currency'] =   $resArray[''];
        $paymentData['settle_amount'] =     $resArray['SETTLEAMT'];
        $paymentData['exchange_rate'] =     $resArray['EXCHANGERATE'];
        $paymentData['charge_id']     = $resArray['charge_id'];
        $paymentData['cc_id']         = $inputVars['cc_id'];
        $paymentData['txn_id']        = $resArray['TRANSACTIONID'];
        // $paymentData['parent_id']      = $paymentObj->id; // todo: later allow multiple capture on one parent payment
      }
      break;

    case PAYMENT_TYPE_IDEAL:
      // echo("get payment : ideal");
      // dump($inputVars);
      $errors = $messages = array();
      $txn_id = $inputVars['transaction_id'];
      $partner_id = $inputVars['partner_id'];
      if(!$txn_id) return array(0, "iDeal: missing transaction ID", '');
      if(!$partner_id) return array(0, "iDeal: missing partner ID", '');

      $messages[] = "iDeal: OK, returned with transaction id $txn_id";
      
      // processing sometimes happens first in the background (/ideal/return.php) - check for this
      if($txObj = find_object('transaction', array('tx_gateway' => 'IDEAL', 'txn_id' => $txn_id))) {
        $tx_id = $txObj->tx_id; // return value: our tx id - check in get_payment
        $messages[] = "Found transaction $tx_id with txn = $txn_id";
        if($txObj->tx_st == TX_ST_OK) {
          $messages[] = "Transaction OK: ".$txObj->tx_notes;
        } else {
          $errors[] = "Transaction not verified: ".$txObj->tx_notes;
        }
      }
      if($paymentObj = find_object('payment', array('payment_type' => PAYMENT_TYPE_IDEAL, 'txn_id' => $txn_id))) {
        $payment_id = $paymenObj->id;
        $messages[] = "Found payment $payment_id with txn = $txn_id";
        //return array($payment_id, "iDeal: a payment already exists with transaction ID $txn_id", '');
      } 

      if(!$paymentObj && !$txObj) { // wasn't entered yet, check with mollie
        $messages[] = "Checking payment with bank...";
        list($ideal_error, $ideal_message) = ideal_return($partner_id, $txn_id, $amount_eur, $test); // verify if payment is OK
                                        
        //if($payment_id = find_object('payment', array('txn_id' => $txn_id), 'id')) return array($payment_id, "iDeal: a payment already exists with transaction ID $txn_id", '');
        //$amount_eur = $currency == 'EUR' ? $amount : $inputVars['amount_eur'];
        if($ideal_error)   $errors[]   = "iDeal:".$ideal_error ;
        if($ideal_message) $messages[] = "iDeal:".$ideal_message;
        // echo("Couldn't find our tx with $txn_id ideal returneded:<br>");dump($errors);dump($messages);          
      }
      // we're done, payment is added above, so just return
      return array($payment_id, implode(html_break(), $errors), implode(html_break(), $messages));
      break;
            
    case PAYMENT_TYPE_PAYPAL:
      $token = $inputVars['token'];                              
      if(!$token) return array(0, "PayPal: missing token", '');
      print_log("Returned from PayPal with token $token - calling get_express_checkout\n", 'paypal', LOG_LEVEL_INFO);    
      require_once("/paypal/do_express_checkout.inc");
      $payerID = $inputVars['PayerID'];
      if($details = get_express_checkout($token)) { // first, get the payment details (address, fees, etc.)
        list($error_code, $pp_error) = get_payment_error($details);
        if($pp_error) {
          $errors[] = $pp_error;
        } else { // own error checking
          if($details['PAYERID']      != $payerID)  {$errors[]= "Payer ID mismatch."; break;} 
          if($details['AMT']          != $amount)   {$errors[]= "Amount mismatch."; break;} 
          if($details['CURRENCYCODE'] != $currency) {$errors[]= "Currency mismatch."; break;}
        }
        if($errors) return array(0, implode(html_break(), $errors), ''); // "get_express_checkout failed.";
        $resArray['PayerID'] = $payerID; // return value
      } else {
        print_log("get_express_checkout failure".PHP_EOL, 'paypal', LOG_LEVEL_ERROR);    
        return array(0, "PayPal: checkout failure", ''); // "get_express_checkout failed.";        
      }                                               
      print_log("Returned from get_express_checkout with PayerID $payerID - calling do_express_checkout".PHP_EOL, 'paypal', LOG_LEVEL_INFO);    

      $paymentType = "Sale";
      print_log("Calling do_express_checkout\n", 'paypal', LOG_LEVEL_INFO);    
      list($charge_error, $charge_message, $resArray) = do_express_checkout($currency, $amount, $invObj, $inputVars, array('lineitems' => $lineitems));
      print_log("Returned from do_express_checkout with resArray".dump($resArray,true).PHP_EOL, 'paypal', LOG_LEVEL_INFO);    
      if($charge_error) {
        print_log("Returned from do_express_checkout with error $charge_error".PHP_EOL, 'paypal', LOG_LEVEL_INFO);    
        return array(0, $charge_error, ''); // prevent adding payment below
      } else {
        $payment_message = $charge_message;
        //echo("got here");
        // dump($resArray);
        $paymentData['payment_status'] =    $resArray['PAYMENTSTATUS']=='Completed' ? PAYMENT_STATUS_COMPLETED :  PAYMENT_STATUS_PENDING;
        $paymentData['payment_currency'] =  $resArray['CURRENCYCODE'];
        $paymentData['payment_gross'] =     $resArray['AMT'];
        $paymentData['payment_fee'] =       $resArray['FEEAMT'];
        $paymentData['payment_net'] =       $resArray['AMT'] - $resArray['FEEAMT'];
        $paymentData['payment_notes'] =     $resArray['NOTE'];
        //$paymentData['settle_currency'] =   $resArray[''];
        $paymentData['settle_amount'] =     $resArray['SETTLEAMT'];
        $paymentData['exchange_rate'] =     $resArray['EXCHANGERATE'];
        $paymentData['txn_id']        = $resArray['TRANSACTIONID'];
        print_log("Returned from do_express_checkout with status $resArray[PAYMENTSTATUS] for $resArray[CURRENCYCODE] $resArray[AMT]\n", 'paypal', LOG_LEVEL_INFO);    

      }
      break;

    case PAYMENT_TYPE_REFUND:
      list($payment_currency, $payment_amount) = payment_currency_amount($paymentObj);

      $refund_currency = $payment_currency; // refund in same currency as payment
      $refund_amount = $currency == $payment_currency ? $amount : round(($payment_amount / $paymentObj->amount) * $amount, 2);

      $tx_id = $paymentObj->tx_id;
      $txObj = $tx_id ? get_object('transaction', $tx_id) : null;
      $notes    = $inputVars['refund_note'];
      $refundType = $amount < $paymentObj->amount ? 'Partial' : 'Full';
      
      if($txObj->tx_gateway == 'PAYPAL') { // is this PayPal - if so, send message to PayPal
        // echo("tx_id=$tx_id gateway=$txObj->tx_gateway<br>");
        $gateway_id = GATEWAY_PAYPAL;
        // issue refund
        require_once("paypal/do_refund.inc");
        list($charge_error, $dp_msg, $resArray) = do_refund($refund_currency, $refund_amount, $refundType, $notes, $paymentObj);
        if($charge_error) {
          $payment_error = $dp_msg;
        } else { // successfully captured payment
          $paymentData['txn_id'] = $resArray['REFUNDTRANSACTIONID'];
          $paymentData['payment_fee']   = -1 * $resArray['FEEREFUNDAMT'];
          $paymentData['payment_gross'] = -1 * $resArray['GROSSREFUNDAMT'];
          $paymentData['payment_net']   = -1 * $resArray['NETREFUNDAMT'];
        }
      }
    
      if(!$payment_error) {
        $paymentData['parent_id']      = $paymentObj->id;
        $paymentData['description']    = $paymentObj->description;
        $paymentData['cc_id']          = $paymentObj->cc_id;
        $paymentData['payment_notes']  = $notes;
        $paymentData['payment_status'] = PAYMENT_STATUS_COMPLETED;
        
        // dump($paymentData);
        
        // update original payment as refunded to prevent further refunds; todo - allow partial refund
        $sum_refund = sum_refund($paymentObj->id);
        $orgpay_status = $sum_refund + $amount >= $paymentObj->amount ? PAYMENT_STATUS_REFUNDED : PAYMENT_STATUS_PARTIALLY_REFUNDED;
  
        $amount = -1 * $amount;      
        
        // todo: remove comment
        list($pay_id, $pay_errors) = update_object('payment', array('payment_status' => $orgpay_status), $paymentObj->id); 
      }
      break;
      
    default:
      die("get_payment: unknown paymentType: '$payment_type'");      
  }
        
  
  if($payment_error) {
    print_log("add_payment error: ".$payment_error.PHP_EOL, 'payment', LOG_LEVEL_ERROR);
    return array(0, $payment_error, $payment_message);
  }
  
  if($draft) {
    list($inv_id, $send_errors) = finalize_invoice($invObj); // moves draft_invoice to open invoice
  } else {
    $inv_id = $invObj->id;
  }
  
  // Success? Then add payment  
  if($send_errors) {
    $payment_error = "Send Invoice:".implode(' - ', $send_errors);
    print_log("finalize_invoice error: ".$payment_error.PHP_EOL, 'payment', LOG_LEVEL_ERROR);
  } else {
    //echo("sent invoice - new id = $new_inv_id<br>");                         
    // dump($paymentData);
  
    $paymentData['payment_type']  = $payment_type;
    $payment_errors = array();
    if($payment_id) { // update payment (Capture)
      list($payment_id, $payment_errors) = update_object('payment', $paymentData, $payment_id);
    } else { // add new payment
      if($gateway_id) $paymentData['gateway_id']    = $gateway_id;  
      $paymentData['site_id']       = $site_id;
      $paymentData['inv_id']        = $inv_id;
      $paymentData['res_id']        = $invObj->res_id;
      $paymentData['currency']      = $currency;
      $paymentData['amount']        = $amount;

      $paymentData['description']   = addslashes($invObj->concerning);

      if($resArray['CURRENCYCODE'] && ($resArray['CURRENCYCODE'] != $currency)) { // store actual charged currency with payment object (PayPal or CC)
        $paymentData['payment_currency'] = $resArray['CURRENCYCODE'];
        $paymentData['payment_gross']    = $resArray['AMT'];        
      }

      if($invObj->type == INV_TYPE_BLOCK) { // this is a block invoice, add dummy payments for all children        
        $childTotal = array();
        foreach($lineitems as $lineitem) $childTotal[$lineitem->inv_id] += $lineitem->total;
        //dump($childTotal);
        $keys = array('payment_status', 'description', 'currency', 'concerning', 'payment_notes', 'gateway_id', 'block_id');
        $childPaymentData = array_copy($paymentData, $keys);

        $childPaymentData['description'] .= " - partial";
        $childPaymentData['payment_type'] .= PAYMENT_TYPE_BLOCK;
        foreach($childTotal as $child_inv_id => $child_amount) {
          $childPaymentData['inv_id'] = $child_inv_id;
          $childPaymentData['res_id'] = get_object('invoice', $child_inv_id, 'res_id');
          $childPaymentData['amount'] = $child_amount;
          //dump($childPaymentData);
          list($child_payment_id, $child_payment_errors) = add_object('payment', $childPaymentData);
          if($child_payment_errors) $payment_errors = array_merge($payment_errors, $child_payment_errors); 
        }      
        unset($paymentData['res_id']); // don't count block payment towards individual reservation
      }

      //dump($paymentData);
      //echo("get_payment: adding payment<br>");return array($payment_error, $payment_message);
      //print_log("All good, adding payment".PHP_EOL, 'payment', LOG_LEVEL_INFO);
      print_log("$ts: add_payment All good, adding payment with data:".dump($paymentData, true), 'payment', LOG_LEVEL_ALWAYS);
      
      list($payment_id, $payment_errors) = add_object('payment', $paymentData);
    }      
    if($payment_errors) {
      $payment_error = implode(html_break(), $payment_errors);
      //dump($payment_errors);
    } else {
      print_log("Added payment $payment_id".PHP_EOL, 'payment', LOG_LEVEL_INFO);
      //echo("added payment $payment_id<br>$output");
      if($txObj && !$txObj->payment_id) { // update transaction with payment id
        list($tx_id, $tx_errors) = update_object('transaction', array('payment_id' => $payment_id), $txObj->tx_id);
      }
    }
  }

  if($payment_error) print_log("get_payment error: ".$payment_error.PHP_EOL, 'payment', LOG_LEVEL_ERROR);
  else print_log("get_payment message: ".$payment_message.PHP_EOL, 'payment', LOG_LEVEL_INFO);

  return array($payment_id, $payment_error, $payment_message);
}

  
function payment_supported_currencies() {
  return array("EUR", "USD"); // only USD and EUR for now
}

/** unsused - automatic comment added on  */
function payment_supported_currency($currency) {
  // $supported_currencies = array("EUR", "USD", "GBP", "CHF");
  $supported_currencies = payment_supported_currencies();
  return in_array($currency, $supported_currencies);
}



function verifed_paypal_transaction($tx_id) {
  if(!$tx_id) return false;
  $txObj = get_object('transaction', $tx_id);
  if(!$txObj) return false;
  if($txObj->tx_gateway != "PAYPAL") return false;
  if($txObj->txn_res != "VERIFIED") return false;
  // if(!$_SESSION['dev'] && $txObj->ipn_test) return false;
  // if($txObj->payment_status != PAYMENT_STATUS_COMPLETED) return false;
  return true;
}

// returns the PayPal ID of the owner for the given paymentObj (withdrawal recipient.)
function payment_paypal_recipient($paymentObj) {
  $inv_id = $paymentObj->inv_id;
  $host_id = $paymentObj->host_id;
  if(!$host_id && $inv_id) $host_id = get_object('invoice', $inv_id, 'host_id'); // get host_id from invoice
  $userObj = $host_id ? get_object('user', $host_id) : NULL;
  return pick_first($userObj->paypal_id, $userObj->paypal_id_private, $userObj->paypal_id_business);
}


// sum of all refunds to a paymentf
function sum_refund($id, $use_payment_currency=false) {
  if(!$id) return 0;
  global $db;
  $field = $use_payment_currency ? 'payment_gross' : 'amount'; 
  return db_getOne("SELECT -1*sum($field) FROM payment WHERE payment_type=".PAYMENT_TYPE_REFUND." AND parent_id=$id");
}

// sum of all withdrawals for payment
function sum_withdrawals($id) {
  if(!$id) return 0;
  global $db;
  return db_getOne("SELECT sum(gross) FROM withdrawal WHERE payment_id=$id");
}

function check_transaction($txArray, $receiver_id='')	{
	$res_id	=	$txArray['res_id'];
	$inv_id	= $txArray['inv_id'];
	$order_id	=	$txArray['order_id'];
  $parent_id   = $txArray['parent_id'];
  $parent_type = $txArray['parent_type'];

	$txn_id	=	$txArray['txn_id'];
	$txn_res =$txArray['txn_res'];
	$receiver =	$txArray['receiver_email'];
	$payment_status	=	$txArray['payment_status'];

  //$payment_status_id = find_object('payment_status', array('name' => $payment_status), 'id'); // look up if valid status

	if($txn_res	!= "VERIFIED") return	array(TX_ST_TXN_NOT_VERIFIED,	"Txn Res=$txn_res");
	// else if($txArray['test_ipn'])	return array(TX_ST_TEST, "");

	if(!$txn_id) array(TX_ST_INCOMPLETE,	"Missing TXN ID");
	if(!$res_id && !$parent_id && !$order_id) return	array(TX_ST_MISSING_RESERVATION_ID,	"");
	if(!$inv_id) return	array(TX_ST_MISSING_INVOICE_ID,	"");

	if($res_id && !get_object('reservation',	$res_id))	return array(TX_ST_UNKNOWN_RESERVATION_ID, "");
	else if($order_id && !get_object('payment_order',	$order_id))	return array(TX_ST_UNKNOWN_RESERVATION_ID, "");
	//if($parent_type && $parent_id && !get_object($parent_type,	$parent_id))	return array(TX_ST_UNKNOWN_RESERVATION_ID, "");
	if(!get_object('invoice',	$inv_id))	return array(TX_ST_UNKNOWN_INVOICE_ID, "");
	if($receiver_id && $receiver_id !=	$receiver) return	array(TX_ST_WRONG_RECEIVER_EMAIL,	"Wrong receiver	ID: $receiver:	POSSIBLE FRAUD");	
	
	//if(!$payment_status_id) return	array(TX_ST_INCOMPLETE,	"Status=$payment_status");

	$gateway = $txArray['tx_gateway'];
	if($gateway != "BITPAY") {
	  if($tx_id	=	find_object('transaction', array('txn_id'	=> $txn_id), 'tx_id')) return	array(TX_ST_EXISTING_TXN_ID, "Transaction ID	(txn_id=$txn_id)	already	exists ($tx_id)	POSSIBLE FRAUD)");
	}
	
	return array(TX_ST_OK, "");
}
  
function payment_type2gateway_id($payment_type) {  
  switch($payment_type) {
  case PAYMENT_TYPE_BITCOIN:
    return GATEWAY_PAYSIUS;
  case PAYMENT_TYPE_CREDIT_CARD:
    return GATEWAY_PAYPAL;
  case PAYMENT_TYPE_PAYPAL:
    return GATEWAY_PAYPAL;
  case PAYMENT_TYPE_IDEAL:
    return GATEWAY_MOLLIE;
  default:
    return NULL;
  }
}    

function paypal_get_balance($all_currencies = true) {
  require_once('paypal/paypal.inc');
  $nvpAr = array();
  $nvpAr['RETURNALLCURRENCIES'] = $all_currencies ? 1 : 0;
  $nvp_str = "&".array2qs($nvpAr, false);
  $resArray = PPHttpPost('GetBalance', $nvp_str);
  $result = array();
  if($resArray['ACK'] == 'Success') {
    foreach($resArray as $k => $v) {
      if(strpos($k, 'L_CURRENCYCODE') === 0) {
        $index = filter_var($k, FILTER_SANITIZE_NUMBER_INT);
        $result[$v] = $resArray['L_AMT'.$index];
      }
    }
  }
  return $result;
}

function paypal_payment($siteObj, $id) {
  require_once('paypal/paypal.inc');
  $nvpAr = array();
  $nvpAr['TRANSACTIONID'] = $id;
  $nvp_str = "&".array2qs($nvpAr, false);
  $resArray = PPHttpPost('GetTransactionDetails', $nvp_str);
  return paypal_result($resArray);
}

function paypal_map() {
  return array(
    'TRANSACTIONID' => 'id',
    'AMT' => 'amount',
    'FEEAMT' => 'fee',
    'TAXAMT' => 'tax',
    'CURRENCYCODE' => 'currency',
    'TRANSACTIONTYPE' => 'method',
    'PAYMENTSTATUS' => 'status',
    'ORDERTIME' => 'created',
    'RECEIVERBUSINESS' => 'receiver',
    'RECEIVEREMAIL' => 'receiver_email',
    'L_NAME0' => 'description',
  );
}

function paypal_result($resArray) {
  $result = new stdClass;
  foreach($resArray as $k => $v) $resArray[$k] = urldecode($v);
  if($resArray['ACK'] == 'Success') {
    $result->success = 1;
    $data = new stdClass;
    $map = paypal_map();
    foreach($map as $k => $v) {
      if($val = $resArray[$k]) $data->$v = $val;
    }
    $meta = new stdClass;    
    if($resArray['INVNUM']) $meta->inv_id = $resArray['INVNUM'];
    $data->metadata = $meta;

    $details = new stdClass;
    if($resArray['FIRSTNAME'] || $resArray['LASTNAME']) $details->consumerName = $resArray['FIRSTNAME']. " ".$resArray['LASTNAME'];
    $data->details = $details;
    
    //$data = array2obj($resArray);
    $result->data = $data;
  } else {
    $result->success = 0;
    $errors = array();
    if($str = $resArray['L_LONGMESSAGE0']) $errors[] = $str;
    if($str = $resArray['L_SHORTMESSAGE0']) $errors[] = $str;
    if($str = $resArray['L_ERRORCODE0']) $errors[] = "(Error code: $str)";
    $error = count($errors) ? implode(html_break(), $errors) : "Unkwnown error";
    $result->error = $error;
  }
  return $result;
}

/** return array of payment methods available for this mollie key */
function mollie_methods($mollie_key) {
  if(!$mollie_key) return [];
  $mollie = new Mollie_API_Client;
  $mollie->setApiKey($mollie_key);
  
  $methodsAr = array();
  $methods = $mollie->methods->all();
  foreach($methods as $method) {
    $methodsAr[$method->id] = $method;
  }
  //if($pp = $methodsAr['paypal']) { // paypal first
  //  unset($methodsAr['paypal']);
  //  $pp->description = "PayPal or Credit Card";
  //  $methodsAr = array('paypal' => $pp) + $methodsAr;
  //}
  //dump($methodsAr);
  return $methodsAr;
}


/** return Mollie Payment object from txn_id */
function mollie_payment($siteObj, $id) {
  $result = new stdClass;
  $result->success = 0;
  
  if(!$id) return null;

  $site_id = $siteObj->id;
  $key = $_SESSION['live'] || $_SESSION['live_payments_test'] ? "ml" : "mt"; 
  $mollie_key =  get_config($key, $site_id);
  
  if(!$mollie_key) {
    $result->error = "Missing API key for $site_id ($key)";
    return $result;
  }
  
  try {
    $mollie = new Mollie_API_Client;
    $mollie->setApiKey($mollie_key);
    if($paymentObj = $mollie->payments->get($id)) {
      $result->success = 1;
      $result->data = $paymentObj;
    }
  } catch (Mollie_API_Exception $e) {
    $result->error =  "API call failed: " . htmlspecialchars($e->getMessage());
  }
  return $result;
}

/** return refunds for Mollie Payment object */
function mollie_refunds($siteObj, $id) {
  $result = new stdClass;
  
  //$result = mollie_payment($id);
  //if(!$result->sucess) return $result;
  //$result->success = 0;
  $site_id = $siteObj->id;

  $mollie_key =  $_SESSION['live'] || $_SESSION['live_payments_test'] ? get_config('ml', $site_id) : get_config('mt', $site_id);  
  $opts = array(
    'http'=>array(
      'method'=>"GET",
      'header'=> "Authorization: Bearer $mollie_key\r\n"
    )
  );
  
  $context = stream_context_create($opts);
  $url = "https://api.mollie.nl/v1/payments/$id/refunds";
  if($dataJson = file_get_contents($url, false, $context)) {
    $result->success = 1;
    $data = json_decode($dataJson);
    foreach($data as $k => $v) $result->$k = $v;      
  } else {
    $result->success = 0;
    $result->error = "Failed to read from $url";
  }
  return $result;
  //return json_encode($result);
}

/** return our transaction data from Mollie Payment object */
function mollie_transaction($paymentObj) { 
  $result = new stdClass;
  $error = '';
  
  //$paymentObj = $response->data;
  //print_log("payment object:".dump($paymentObj, true), 'mollie', LOG_LEVEL_ALWAYS);
  
  $test     = $paymentObj->test;
  $site_id  = $paymentObj->metadata->site_id;
  $inv_id   = $paymentObj->metadata->inv_id;
  $res_id   = $paymentObj->metadata->res_id;
  if(!$res_id) $order_id = $paymentObj->metadata->order_id;
  
  $name     = $paymentObj->details->consumerName;
  $account  = $paymentObj->details->consumerAccount;
       
  $detail_keys = array(
    'bankName', 'bankAccount', 'transferReference', 
    'consumerName', 'consumerAccount', 'consumerBic', 
    'bitcoinAddress', 'bitcoinAmount', 'bitcoinRate', 'bitcoinUri',
    'paypalReference');
  foreach($detail_keys as $key) {
    $tx_key = camel2underscore($key);    
    $txArray[$tx_key] = $paymentObj->details->$key;
  }

  if($txArray['bitcoin_amount']) {
    $txArray['settle_currency'] = "BTC";
    $txArray['settle_amount'] = $txArray['bitcoin_amount'];
    $txArray['exchange_rate'] = $txArray['bitcoin_rate'];
  }
  
  $amount   = $paymentObj->amount;

  $txn_type   = $paymentObj->method;
  if($txn_type == 'ideal') {
    $fee = 1.18;  
  } elseif($txn_type == 'creditcard') {
    $fee = ('0.25' + 0.028 * $amount) * 1.21;
  } else {
    $fee = 1; // todo: implement
  }
  
  /** only for refunds */
  $refund_id = $paymentObj->refund_id;
  $refund_amount = $paymentObj->refund_amount;
  $refund_status = $paymentObj->refund_status;

  print_log("mollie_transaction: refund_id=$refund_id amount=$amount refund_amount=$refund_amount", 'mollie', LOG_LEVEL_TEST);
  $txn_id = $paymentObj->id;
  if($refund_id && $refund_amount > 0) {
    //$txn_type = PAYMENT_TYPE_REFUND;
    $txArray['parent_txn_id'] = $txn_id;
    $txArray['refund_amount'] = $refund_amount;
    $txn_id = $refund_id;
    $amount = -1 * $refund_amount;
    $fee = 0;
    print_log("mollie_transaction: REFUND: txn=$refund_id parent=".$txArray['parent_txn_id'], 'mollie', LOG_LEVEL_TEST);
  } 
  
  /** figure out status */
  $payment_status = 0;
  switch($paymentObj->status) {
  case 'open': // The payment has just been created, no action has happened on it yet.
    $payment_status = PAYMENT_STATUS_OPEN;
    break;
  case 'pending': // The payment has just been started, no final confirmation yet.
    $payment_status = PAYMENT_STATUS_PENDING;
    break;
  case 'cancelled': // The customer has cancelled the payment.
    $payment_status = PAYMENT_STATUS_CANCELLED;
    break;
  case 'expired': // The payment has expired due to inaction of the customer.
    $payment_status = PAYMENT_STATUS_EXPIRED;
    break;
  case 'paid': // The payment has been paid.
    $payment_status = PAYMENT_STATUS_COMPLETED;
    break;
  case 'paidout': // The payment has been paidout and the money has been transferred to the bank account of the merchant.
    $payment_status = PAYMENT_STATUS_COMPLETED;
    break;
  case 'refunded': // The payment has been refunded, either through Mollie or through the payment provider (in the case of PayPal).
    switch($refund_status) {
    case 'pending':
      $payment_status = PAYMENT_STATUS_PENDING; 
    break;
    case 'processing':
      $payment_status = PAYMENT_STATUS_PROCESSING; 
    break;
    case 'refunded':
      $payment_status = PAYMENT_STATUS_REFUNDED; 
    break;
    default:
      $error = "Unknown or missing refund status $refund_status"; 
    break;
    }
    /** the parent payment will be set to PAYMENT_STATUS_REFUNDED, and this payment changed COMPLETED by post_add transaction */
    break;
  default:
    $error = "Error: Unknown payment_status $paymentObj->status";
    break;    
  }
  
  /** calculate fees: todo update */

  $txArray['amount_remaining'] = $paymentObj->amountRemaining;

  $txArray['tx_gateway']  = "MOLLIE";              
  
  $txArray['mc_currency'] = 'EUR';
  $txArray['mc_gross']    = $amount;
  $txArray['mc_fee']      = $fee; // TODO; not hard code?
                           
  if($res_id) $txArray['res_id'] = $res_id;
  elseif($order_id) $txArray['order_id'] = $order_id;
  $txArray['inv_id']      = $inv_id;
  $txArray['site_id']     = $site_id;

  if($inv_id) {
    $parent_type = 'invoice';
    $parent_id = $inv_id;
  } elseif($res_id) {
    $parent_type = 'reservation';
    $parent_id = $res_id;
  } elseif($order_id) {
    $parent_type = 'payment_order';
    $parent_id = $order_id;
  }
  
  $txArray['parent_id']   = $parent_id;
  $txArray['parent_type'] = $parent_type;
    
  $txArray['txn_id']      = $txn_id;
  
  $txArray['tx_st']	      =	$tx_st;
  $txArray['tx_notes']    = $notes;
  $txArray['tx_type']     = $txn_type;
  //$txArray['receiver_email'] = $partner_id;
  $txArray['txn_res']	= "VERIFIED";
  
  $txArray['payment_date'] = date("H:i:s M d, Y T"); // 05:46:09 Jan 18, 2010 PST
  $txArray['payment_status'] = $payment_status;
  
  $txArray['first_name']     = $name;
  $txArray['payer_email']    = $account;
  
  //if($uhuru) write_file('/web/data/uhuru/log/ideal-test.txt', print_r($txArray, true),	'a');
  
  if ($txn_id) {      
    list($tx_st, $tx_notes) = check_transaction($txArray);			
    $txArray['tx_st']	=	$tx_st;
    $tx_notes = array_filter(array($error,$message,$tx_notes));
    $txArray['tx_notes'] = implode("\n", $tx_notes);    
  } else {
    $txArray['tx_notes'] = $error = "Missing tx_id";
    $txArray['tx_st']	=	$tx_st = TX_ST_INCOMPLETE;
  }
  
  //$log_result[] = "Status: ".get_object('tx_status', $tx_st, 'name');
  //$log_result[] = "Notes: ".$txArray['tx_notes'];
  //$log_result[] = dump($txArray, true);
  
  $result->success = $error ? 0 : 1;
  $result->data = $txArray;
  if($error) $result->error = $error;

  return $result;
}


function payment_currency_amount($paymentObj) {
  if($paymentObj->payment_currency) return array($paymentObj->payment_currency, $paymentObj->payment_gross);
  return array($paymentObj->currency, $paymentObj->amount);
}

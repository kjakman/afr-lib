<?php

function pay_row($paymentObj, $currency, $owner_name, $owner_pp, $w_gross, $p_withdrawn, $verified) {        
  $cur = get_currency_symbol($currency, true);
  $gateway = $paymentObj->gateway_id ? get_object('gateway', $paymentObj->gateway_id, 'name') : '';    
  $inv_link = $paymentObj->inv_id ? html_link(get_object_link('invoice', $paymentObj->inv_id), $paymentObj->inv_id) : '';    
  $payment_status = get_object('payment_status', $paymentObj->payment_status, 'name');
  $payment_type = get_object('payment_type', $paymentObj->payment_type, 'name');
  if($tx_id = $paymentObj->tx_id) {
    $tx_link = html_link(get_object_link('transaction', $paymentObj->tx_id), $paymentObj->tx_id);
    $txObj = get_object('transaction', $tx_id);
    $tx_name = $txObj->first_name.' '.$txObj->last_name;
  } else {
    $tx_link = $tx_name = '';
  }


  $payRow = array(html_link(get_object_link('payment', $paymentObj->id), $paymentObj->id), sql2human($paymentObj->created, array('show_time' => true)), $inv_link, $gateway, $payment_type, $payment_status, 
                     formatFloat($paymentObj->payment_gross, FORMAT_ALWAYS, $cur), formatFloat($paymentObj->payment_fee, FORMAT_ALWAYS, $cur), formatFloat($paymentObj->payment_net, FORMAT_ALWAYS, $cur), 
                     formatFloat($p_withdrawn, $cur), formatFloat($w_gross, FORMAT_ALWAYS, $cur), 
                     $owner_name, $owner_pp, $tx_link, $tx_name, $verified);

  global $u_id;
  if($u_id == SYSTEM_USER_ID) { 
    if($owner_pp) {
      $gross  = number_format($paymentObj->payment_gross, 2, '.', '');
      $w_link = html_link("?currency=$paymentObj->payment_currency&host_id=$paymentObj->host_id&send=1&amount=$gross&pids=$paymentObj->id", "Send: $cur$gross to $owner_pp.");
    } else {
      $w_link = "[no PPID]";      
    }
    $payRow[] = $w_link;
  }

  return $payRow;
}  

/** unsused - automatic comment added on  */
function print_oustanding_payments($paymentObjs, $currency) {  
  global $u_id;
  $errors = array();
  $messages = array();
  if(!count($paymentObjs)) return '';

  $pids = array_find_key($paymentObjs, 'id');
  $pid_list = implode(',', $pids);
  
  $cur = get_currency_symbol($currency, true);
  $headerRow = array('ID', 'Date', 'Invoice', 'Gateway', 'Method', 'Status', 'Gross', 'Fee', 'Net', 'Withdrawn', 'Outstanding', 'Owner', 'PayPal ID', 'Tx ID', 'From', 'Verified');
  $payRows[] = $headerRow;
  $incomplete_transfers = array(); 
  
  foreach($paymentObjs as $paymentObj) {
    $id = $paymentObj->id;
    $host_id = $paymentObj->host_id;

    $inv_id = $paymentObj->inv_id;
    if(!$host_id && $inv_id) $host_id = get_object('invoice', $inv_id, 'host_id'); // get host_id from invoice
    $ownerObj = $host_id ? get_user($host_id) : NULL;
    $owner_name = get_name_string($ownerObj);
    $owner_pp = get_owner_pp_id($ownerObj, $currency);
    $pp_verified = verifed_paypal_transaction($paymentObj->tx_id); // check if verified paypal payment
  
    //dump($paymentObj);  
    list($p_currency, $p_amount) = payment_currency_amount($paymentObj);
    
    list($w_currency, $w_gross) = payment_net_outstanding($paymentObj);
    
    if($w_currency != $currency) die("Currency mismatch for $paymentObj->id: received '$currency', withdrawal currency = '$w_currency'");
    if($u_level == USER_LEVEL_OWNER && $host_id != $u_hostid) die("Payment $id belongs to $owner_name, not to logged in owner ($u_name)");
    if(!$ownerObj) die("No owner found for payment $id");  
    // if(!$owner_pp) die("No PayPal ID found for owner $owner_name");  
    if($owner_pp && !is_email($owner_pp)) die("$owner_pp is not a valid email address.");  
    if($paymentObj->gateway_id == GATEWAY_PAYPAL) {
      if($pp_verified) {
        $verified = 'Yes';
      } else {
        // echo(html_div("Payment $id is not verified by PayPal", 'errors'));
        $verified = html_span('No', 'red');
      }
    } else {
      $verified = '';
    }

    $p_withdrawn   = sum_withdrawals($id);
    if(! $w_gross * 0) {   
      $sum_withdraw = sum_withdrawals($paymentObj->id); // subtract what has already been withdrawn, if any
      $errors[] = "Payment $id net = $w_currency $w_gross. ";
      if($sum_withdraw) $errors[] = "$w_currency $sum_withdraw has already been withdrawn.";
    } else {
      $w_total[$host_id] += $w_gross;
    }
    // $p_total

    // for now, ignore transfers
    // todo: count + in total if not withdrawn
    // count - in total if not deducted.
    if($paymentObj->payment_type != PAYMENT_TYPE_TRANSFER) {
      $total_paid += $p_amount;
      $total_fees += $paymentObj->payment_fee;
      $total_net  += $paymentObj->payment_net;
      $total_out  += $p_withdrawn;
      $total_owed += $w_gross;
    }
  
    $total_owner_paid[$host_id] += $p_amount;
    $total_owner_fees[$host_id] += $paymentObj->payment_fee;
    $total_owner_net[$host_id]  += $paymentObj->payment_net;
    $total_owner_out[$host_id]  += $p_withdrawn;
    $total_owner_owed[$host_id] += $w_gross;

    $payRow = pay_row($paymentObj, $currency, $owner_name, $owner_pp, $w_gross, $p_withdrawn, $verified); 
    $payRows[] = $payRow;
    $payRowsOwner[$host_id][] = $payRow;
    $paymentObjsOwner[$host_id][] = $paymentObj;
    $pidsOwner[$host_id][] = $paymentObj->id;

    if($paymentObj->payment_type == PAYMENT_TYPE_TRANSFER && $paymentObj->payment_status == PAYMENT_STATUS_CREATED) { // add twice, in and out (unless already deducted)
      $paymentObj->amount *= -1;
      $paymentObj->payment_net *= -1;
      $paymentObj->payment_gross *= -1;
      $w_gross *= -1;
      if($p_withdrawn) $p_withdrawn *= -1;
      $p_amount *= -1;

      
      $from_host_id = $paymentObj->from_host_id;
      $incomplete_transfers[$host_id] = $from_host_id;
      $total_owner_paid[$from_host_id] += $p_amount;
      $total_owner_fees[$from_host_id] += $paymentObj->payment_fee;
      $total_owner_net [$from_host_id] += $paymentObj->payment_net;
      $total_owner_out [$from_host_id] += $p_withdrawn;
      $total_owner_owed[$from_host_id] += $w_gross;
      $w_total[$from_host_id] += $w_gross;      

      $payRow = pay_row($paymentObj, $currency, $owner_name, $owner_pp, $w_gross, $p_withdrawn, $verified); 
      $payRows[] = $payRow;
      $payRowsOwner[$from_host_id][] = $payRow;
      $paymentObjsOwner[$from_host_id][] = $paymentObj;
      $pidsOwner[$from_host_id][] = $paymentObj->id;

    }

  }
  
  $payRows[] = array('Total', '', '', '', '', '',
                     formatFloat($total_paid, FORMAT_ALWAYS, $cur), formatFloat($total_fees, FORMAT_ALWAYS, $cur), formatFloat($total_net, FORMAT_ALWAYS, $cur), 
                     formatFloat($total_out, FORMAT_ALWAYS, $cur), formatFloat($total_owed, FORMAT_ALWAYS, $cur), '', '');

  $title = "$currency: All Owners ($cur$total_owed)";
  $output .= show_hide_div(html_table($payRows, array('title' => $title, 'class' => 'web20', 'header' => 1, 'footer' => 1)), $title);

  $owners = array_keys($w_total);
  $o_count = count($owners);

  // print table per owner (if multiple) and issue withdrawal
  foreach ($owners as $o_id) {
    $oObj = ($o_count == 1) ? $ownerObj : get_user($o_id);
    $o_name = get_name_string($oObj);
    $o_pp = get_owner_pp_id($oObj, $currency);
    $gross  = number_format($w_total[$o_id], 2, '.', '');
    $fee    = withdrawal_fee($currency, $gross, $o_pp == $oObj->paypal_id_usd);
    $net    = $gross - $fee;
    $o_pids = $pidsOwner[$o_id];
    $o_pid_list = implode(',', $o_pids);
    
    $rows = array();
    $rows[] = $headerRow;
    $total_paid = $total_out = $total_fees = $total_net = $total_owed = 0;
    foreach($payRowsOwner[$o_id] as $o_row) $rows[] = $o_row;
    $total_paid = $total_owner_paid[$o_id];
    $total_fees = $total_owner_fees[$o_id];
    $total_net  = $total_owner_net[$o_id];
    $total_out  = $total_owner_out[$o_id];
    $total_owed = $total_owner_owed[$o_id];
    $rows[] = array("Total for $o_name", '', '', '', '', '', 
                       formatFloat($total_paid, FORMAT_ALWAYS, $cur), formatFloat($total_fees, FORMAT_ALWAYS, $cur), formatFloat($total_net, FORMAT_ALWAYS, $cur), 
                       formatFloat($total_out, FORMAT_ALWAYS, $cur), formatFloat($total_owed, FORMAT_ALWAYS, $cur), '', '');  
    
    if($u_id == SYSTEM_USER_ID) {
      if($pt_host_id = $incomplete_transfers[$o_id]) {
        $pt_owner = get_user($pt_host_id);
        $pt_owner_name = get_name_string($pt_owner);
        $link = "Cannot send $cur$gross to $o_name due to pending transfer from $pt_owner_name.";      
      } elseif($o_pp) {
        $link = html_link("?currency=$currency&host_id=$o_id&send=1&amount=$gross&pids=$o_pid_list", "Send: $cur$gross to $o_pp.");
      } else {
        $link = "Cannot send $cur$gross to $o_name; missing PayPal ID";      
      }
    }
    $title = "$currency: $o_name: ($cur$gross)";
    $output .= show_hide_div(html_table($rows, array('title' => $title, 'class' => 'web20', 'header' => 1, 'footer' => 1)), $title);
    $output .= $link.html_break(2);
  }

  if($errors)   $output = print_errors($errors, false).$output;
  if($messages) $output = print_messages($messages, false).$output;
  return $output;
}

function withdraw_multiple_payments($host_id, $currency, $pids) {
  require_once("paypal/do_mass_payment.inc");

  $test = $_SESSION['live'] ? false : true;
  
  $errors = array();
  $messages = array();

  $paymentObjs = find_objects('payment', array('id' => $pids));
  $now = now();
  
  $ownerObj = get_user($host_id);
  $owner_name = get_name_string($ownerObj);
  $owner_pp = $ownerObj->paypal_id; // get_owner_pp_id($ownerObj, $currency);
  if(!$owner_pp) {
    $errors[] = "Host $host_id has no paypal_id";
    return array($errors, $messages);
  }
    
  if(!is_email($owner_pp)) {
    $errors[] = "Host $host_id has invalid paypal_id ($owner_pp)";
    return array($errors, $messages);
  }

  $pid_list = implode(',', $pids);

  //die("opp of $host_id = $owner_pp");  
  // double check validity and calculate total
  foreach($paymentObjs as $paymentObj) {
    $id = $paymentObj->id;
    $p_oid = $paymentObj->host_id;
    if(!$p_oid && $paymentObj->inv_id) $p_oid = get_object('invoice', $paymentObj->inv_id, 'host_id'); // get host_id from invoice

    //list($w_currency, $w_gross) = payment_net_outstanding($paymentObj);
    $w_gross = $paymentObj->payment_net;
    //die("todo: check for refunds");
    
    $w_currency = $paymentObj->payment_currency;
    
    if($test) $messages[] = "$id: outstanding = $w_currency $w_gross";
    
    $pp_verified = verifed_paypal_transaction($paymentObj->tx_id); // check if verified paypal payment
    if(0 && $paymentObj->gateway_id == GATEWAY_PAYPAL && !$pp_verified) {
      $errors[] = "Payment $id is not verified";
      return array($errors, $messages);
    }
    
    if($w_currency != $currency) {
      $errors[] = "Payment $id is in '$w_currency' - only '$currency' allowed.";
      return array($errors, $messages);
    }
    
    if($p_oid != $host_id) {
      $errors[] = "Host ID mismatch on payment $id";
      return array($errors, $messages);
    }
    
    $gross += $w_gross;

    /**
    if($p_oid != $host_id) {
      if($paymentObj->payment_type == PAYMENT_TYPE_TRANSFER) {
        if($paymentObj->from_host_id == $host_id) {
          $gross -= $w_gross;          
          echo("Owner=$host_id: Payment $p_oid = transfer from $paymentObj->from_host_id to $paymentObj->host_id, deducting<br>");
          $paymentObj->amount *= -1;
          $paymentObj->payment_gross *= -1;
          $paymentObj->payment_fee   *= -1;
          $paymentObj->payment_net   *= -1;

        } else {
          echo("Owner=$host_id: Payment $p_oid = transfer from $paymentObj->from_host_id to $paymentObj->host_id, skipping<br>");
        }
      } else {
        die("Payment $id is owned by $p_oid: only $host_id allowed.");
      }
    } else {
      $gross += $w_gross;
    }
    */
    
  }
  
  
  // echo("OK. Withdrawing Total = $w_currency $gross to $owner_pp<br>");
  // return;
  
  $fee    = withdrawal_fee($currency, $gross, $owner_pp == $ownerObj->paypal_id_usd);
  $net    = $gross - $fee;

  $messages[] = "Paying Out $gross - $fee = $w_currency $net to $owner_name ($owner_pp)";
  
  $notes = implode(',', $pids);
  // first add withdrawal to get ID
  // $w_data['payment_id']   = $paymentObj->id;
  $w_data['gateway_id']   = GATEWAY_PAYPAL;
  $w_data['payment_type'] = PAYMENT_TYPE_PAYPAL;
  $w_data['currency']     = $currency;
  $w_data['gross']        = $gross;
  $w_data['fee'] = $w_data['gateway_fee'] = $fee;
  $w_data['net']          = $net;
  $w_data['notes']        = $notes;
  $w_data['recipient']    = $owner_pp;
  $w_data['host_id']     = $host_id;

  if($test) {
    $messages[] = "Adding withdrawal:".dump($w_data,true);
  } else {
    list($wid, $werrors) = add_object('withdrawal', $w_data);
  }
  
  if($werrors) {
    $errors = array_merge($errors, $werrors);  
  } else {  // withdraw funds
    $firstPaymentObj = $paymentObjs[0];
    $first_id = $firstPaymentObj->id;
    
    $s = $_SESSION['site'];
    $uniqueID = "pid=$first_id&wid=".$wid;
    $subject = "[$s->shortname accounting] Withdrawal of $currency $net for host $host_id payments: $pid_list";

    if($test) { 
      $messages[] = "test mode: do_single_payment($currency, $net, $owner_pp, $subject, '', $uniqueID)";
      // update payments      
      $i=1;
      $total = 0;
      foreach($paymentObjs as $paymentObj) {
        list($pay_currency, $pay_amount) = payment_currency_amount($paymentObj);
        $w_fee = $i == 1 ? $fee : 0;
        $w_amount = $paymentObj->payment_net;
        
        /**
        if($paymentObj->payment_type == PAYMENT_TYPE_TRANSFER && $paymentObj->from_host_id == $host_id) {
          $status = $paymentObj->withdrawal_id ? PAYMENT_STATUS_COMPLETED : PAYMENT_STATUS_DEDUCTED;
          echo("update payment $paymentObj->id: transfer out $currency $w_amount from $owner_pp to $paymentObj->host_id. Status=$status<br>");
        } else
        */
        
        if($paymentObj->payment_type == PAYMENT_TYPE_CORRECTION) {
          $status = PAYMENT_STATUS_COMPLETED;
          $messages[] = "payment $paymentObj->id: (correction) $currency $w_amount Fee=$w_fee host=$paymentObj->host_id. Status=$status";
        } else {
          $messages[] = "payment $paymentObj->id: $currency $w_amount pp=$owner_pp Fee=$w_fee ";
        }
        $total += $w_amount;
        
        $i++;
      }
      $messages[] = "Total: $total";
      

    } else {      
      list($charge_error, $charge_message, $resArray) = do_single_payment($currency, $net, $owner_pp, $subject, '', $uniqueID);
    
      if($charge_error) {
        $errors[] = "Failed to withdraw $currency $net to $owner_pp<br>".$charge_error.$charge_message;
        delete_object('withdrawal', $wid);
      } else { // successfully sent money
        $messages[] = $charge_message;
  
        // update payments      
        $i=1;
        foreach($paymentObjs as $paymentObj) {
          $payData = array();
          list($pay_currency, $pay_amount) = payment_currency_amount($paymentObj);
          $w_fee = $i == 1 ? $fee : 0;
          $w_amount = $paymentObj->payment_net;
          
          if($paymentObj->payment_type == PAYMENT_TYPE_TRANSFER && $paymentObj->from_host_id == $host_id) {
            $payData['payment_status'] = $paymentObj->withdrawal_id ? PAYMENT_STATUS_COMPLETED : PAYMENT_STATUS_DEDUCTED; // if not withdrawn by recipient, mark as deducted. Otherwise completed.            
          } else {
            if($paymentObj->payment_type == PAYMENT_TYPE_CORRECTION) $payData['payment_status'] = PAYMENT_STATUS_COMPLETED;
            if($paymentObj->payment_type == PAYMENT_TYPE_TRANSFER && $paymentObj->host_id == $host_id && $paymentObj->payment_status == PAYMENT_STATUS_DEDUCTED) $payData['payment_status'] = PAYMENT_STATUS_COMPLETED; // was already deducted, now withdrawn so complete
            
            $payData['withdrawal_id']         = $wid;
            $payData['withdrawal_currency']   = $currency;
            $payData['withdrawal_amount']     = $w_amount;
            $payData['withdrawal_type']       = PAYMENT_TYPE_PAYPAL;
            $payData['withdrawal_fee']        = $w_fee; // add fee to first
            $payData['withdrawal_date']       = $now;
            $payData['withdrawal_recipient']  = $owner_pp;
          }          
          update_object('payment', $payData, $paymentObj->id);
          $i++;
        }
        // update_object('withdrawal', $w_data, $wid);      
      }
    }
  }
  return array($errors, $messages);
}

/** unsused - automatic comment added on  */
function print_past_withdrawals($siteObj, $currencies=NULL, $host_ids = NULL) {
  global $db;
  global $u_id;
  $currencies = $currencies ?: db_getCol("SELECT distinct(currency) FROM withdrawal");
  if(!$host_ids && $u_id == SYSTEM_USER_ID) $host_ids = db_getCol("SELECT distinct(host_id) FROM withdrawal");
  if(!$host_ids) return '';
  $output .= html_subheading("Past Withdrawals");
  
  foreach($host_ids as $host_id) {
    foreach($currencies as $currency) {
      $output .= past_withdrawals($siteObj, $currency, $host_id);
    }
    //$output .= html_break();
  }
  return $output;
}

function past_withdrawals($siteObj, $currency, $host_id) {
  //$owner_name = get_select_value2('owner', $wObj->host_id);
  //$title = "Past withdrawals in $currency for $owner_name";
  $o_name = get_select_value2('owner', $host_id);
  $cur = get_currency_symbol($currency, true);
  $wSearch['currency'] = $currency;
  $wSearch['host_id'] = $host_id;
  
  $wObjs = find_objects('withdrawal', $wSearch);
  if(!$wObjs) return '';
  $rows[] = array('', 'Date', 'Currency', 'Total', 'Correction', /*'Transfer',*/ 'Gross', 'Fee', 'Net', 'Recipient', 'Invoices', 'Notes');
  foreach($wObjs as $wObj) {
    $w_id = $wObj->id;
    $preview_url = site_page_link($siteObj, PAGE_PAYMENTS)."?withdrawal_id=".$wObj->id;
    //$preview_url = "/admin/payment.php?withdrawal_id=".$wObj->id;
    $preview_js = "onclick='preview_dialog(\"$wObj->id\", \"$preview_url\"); return false;'";
    $details_link = html_link($preview_url, "Details", array('full' => true, 'js' => $preview_js, 'class' => 'arrow_link non-html'));
    $total = round(db_getOne("SELECT sum(payment_net) FROM payment WHERE withdrawal_id = $w_id AND payment_type < 100 AND payment_status=20"),2);
    $correction = round(db_getOne("SELECT sum(amount) FROM payment WHERE withdrawal_id = $w_id AND payment_type=110"),2);
    $transfer = round($wObj->gross - ($total+$correction),2);
    $inv_ids = db_getCol("SELECT distinct(inv_id) FROM payment WHERE withdrawal_id = $w_id AND payment_type < 100 AND payment_status=20");
    $invoiceAr=array();
    $invoices = $wObj->invoices;
    if(!$invoices) {
      foreach($inv_ids as $inv_id) {
        $invObj = get_object('invoice', $inv_id);
        $invoice_id = $invObj->invoice_id ?: $invObj->id;
        if($invoice_id) $invoiceAr[] = $invoice_id;
      }
      $invoices = implode(',',$invoiceAr);
      db_query("UPDATE withdrawal SET invoices='$invoices' WHERE id=$w_id");
    }
    $rows[] = array($details_link, sql2human($wObj->created), $wObj->currency, $total, $correction, /*$transfer,*/ $wObj->gross, $wObj->fee, $wObj->net, $wObj->recipient, $invoices, $wObj->notes);
    $sum['gross'] += $wObj->gross;
    $sum['fee'] += $wObj->fee;
    $sum['correction'] += $wObj->correction;
    $sum['net'] += $wObj->net;
  }
  $rows[] = array('', '', $sum['gross'], $sum['fee'], $sum['correction'], $sum['net'],  '', '', '');
  //dump($wObjs);
  $table =html_table($rows, array('title' => $title, 'class' => 'web20', 'header' => 1, 'footer' => 1));
  $total = formatFloat($sum['net'], FORMAT_ALWAYS, $cur);
  $w_title = "$currency: $o_name ($total)";
  return show_hide_div($table, $w_title);
}

function get_owner_pp_id($oObj, $currency='') {
  return $currency == 'USD' ? pick_first($oObj->paypal_id_usd, $oObj->paypal_id_private, $oObj->paypal_id_business) : 
                              pick_first($oObj->paypal_id_private, $oObj->paypal_id_business);
}


// Withdraw payment to owner.
/** unsused - automatic comment added on  */
function withdraw_payment($paymentObj, $notes='') {
  require_once("paypal/do_mass_payment.inc");
  $payment_id = $paymentObj->id;
  $owner_pp_id = payment_paypal_recipient($paymentObj);
  $ownerObj = get_user($paymentObj->host__id);

  list($currency, $gross) = payment_net_outstanding($paymentObj);
  $fee = withdrawal_fee($currency, $amount, $owner_pp_id == $ownerObj->paypal_id_usd);
  $net = $gross - $fee; // subtract the fee
  
  if(!is_email($owner_pp_id)) return array(1, "Cannot withdraw payment: $owner_pp_id is not a valid email address.");
  if(!payment_supported_currency($currency)) return array(1, "Cannot withdraw payment: '$currency' is not a supported currency.");
  if($net <= 0) return array(1, "Cannot withdraw payment: Amount ($gross) - Fee ($fee) = Net ($net). Must be greater than zero.");
  if(!verifed_paypal_transaction($paymentObj->tx_id)) return array(1, "Cannot withdraw payment: transaction is not verified.");
  if($paymentObj->gateway_id != GATEWAY_PAYPAL) return array(1, "Cannot withdraw payment: Not a PayPal payment.");
  if(!in_array($paymentObj->payment_status, array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PARTIALLY_REFUNDED))) return array(1, "Cannot withdraw payment: Payment status is ".get_object('payment_status', $paymentObj->payment_status, 'name'));
  if(!in_array($paymentObj->payment_type, array(PAYMENT_TYPE_CREDIT_CARD, PAYMENT_TYPE_PAYPAL, PAYMENT_TYPE_CREDIT_CARD_CAPTURE, PAYMENT_TYPE_ECHECK))) return array(1, "Cannot withdraw payment: Payment type is ".get_object('payment_type', $paymentObj->payment_type, 'name'));

  $wObj = find_object('withdrawal', array('payment_id' => $payment_id));
  if($wObj) return array(1, "This payment has already been withdrawn.".html_break()."$wObj->currency $wObj->net was sent to $wObj->recipient on ".sql2human($wObj->created, array('show_weekday' => true, 'show_time' => true)));
  if($w_date = $paymentObj->withdrawal_date) return array(1, "This payment has already been withdrawn.".html_break()."$paymentObj->withdrawal_currency $paymentObj->withdrawal_amount was sent to $paymentObj->withdrawal_recipient on ".sql2human($w_date, array('show_weekday' => true, 'show_time' => true)));

  //if($_SESSION['dev']) $owner_pp_id = 'guest@kjakman.com'; // for testing
  
  $s = $_SESSION['site'];
  $uniqueID = "pid=".$paymentObj->id;
  $subject = "[$s->shortname accounting] Instant withdrawal of $currency $amount for payment $paymentObj->id";
  list($charge_error, $charge_message, $resArray) = do_single_payment($currency, $net, $owner_pp_id, $subject, '', $uniqueID);
  if($charge_error) {
    return array($charge_error, "Failed to withdraw $currency $amount to $owner_pp_id.".$charge_message);
  } else { // successfully captured payment
    $w_data['payment_id']   = $paymentObj->id;
    $w_data['gateway_id']   = GATEWAY_PAYPAL;
    $w_data['payment_type'] = PAYMENT_TYPE_PAYPAL;
    $w_data['currency']     = $currency;
    $w_data['gross']        = $gross;
    $w_data['fee']          = $fee;
    $w_data['net']          = $net;
    $w_data['notes']        = $notes;
    $w_data['recipient']    = $owner_pp_id;
    
    list($wid, $werrors)    = add_object('withdrawal', $w_data);
    return $werrors ? array(0, implode(html_break, $werrors).html_break().$charge_message) : array(0, $charge_message);
  }
}

// returns the amount the owner can withdraw from a payment (fee is not subtracted yet...)
function payment_net_outstanding($paymentObj) {
  $currency = $paymentObj->payment_currency;
  $payment_net = $paymentObj->payment_net; // in payment currency
  $ownerObj = get_user($paymentObj->host_id);
  $owner_pp_id = payment_paypal_recipient($paymentObj);
  
  // For valid transfers and correction, just return currency + net
  if($paymentObj->payment_type == PAYMENT_TYPE_CORRECTION && !$paymentObj->withdrawal_id) return array($currency, $paymentObj->payment_net); 

  //if($paymentObj->user_id == SYSTEM_USER_ID && 
  //   in_array($paymentObj->payment_type, array(PAYMENT_TYPE_TRANSFER, PAYMENT_TYPE_CORRECTION)) && 
  //   ( ($paymentObj->payment_status == PAYMENT_STATUS_CREATED && !$paymentObj->withdrawal_id) || $paymentObj->payment_status == PAYMENT_STATUS_DEDUCTED)) {
  //  //echo("amount=$paymentObj->amount Net=$payment_net out=$sum_withdraw refund=$sum_refund result=$result<br>");
  //  return array($currency, $payment_net);
  //}
       
  if(!verifed_paypal_transaction($paymentObj->tx_id)) return array($currency, 0);
  if(!$payment_net) return array($currency, 0);
  
  //$payment_net = $currency == $payment_currency ?

  // $payment_net = $currency == $paymentObj->currency ? $paymentObj->payment_net : round(($payment_amount / $paymentObj->amount) * $amount, 2);
  $sum_withdrawals = sum_withdrawals($paymentObj->id);
  $sum_withdraw = max($paymentObj->withdrawal_amount, $sum_withdrawals); // subtract what has already been withdrawn, if any

  $sum_refund = sum_refund($paymentObj->id, true); // subtract refunds if any

  $max_withdraw = $payment_net - ($sum_withdraw + $sum_refund);
  $fee = withdrawal_fee($currency, $max_withdraw, $owner_pp_id == $ownerObj->paypal_id_usd);
  $net = $max_withdraw - $fee;
  $result = $net > 0 ? number_format($max_withdraw,2, '.', '') : 0;  
  //echo("amount=$paymentObj->amount Net=$payment_net out=$sum_withdraw refund=$sum_refund result=$result<br>");
  return array($currency, $result); 
}

function withdrawal_fee($currency, $amount, $us2us=false) {
  $fee_percentage = 2;
  $fee_factor = $fee_percentage / 100;
  
  switch($currency) {
    case 'USD':
      $fee_max = 20;
      $fee = $us2us ? 1 : min($amount*$fee_factor, $fee_max); // fee is $1 when paid to a US account
      break;
    case 'EUR':
      $fee_max = 16;
      $fee = min($amount*$fee_factor, $fee_max);
      break;
    default:
      $fee_max = 20;
      $fee = min($amount*$fee_factor, get_exchange_rate($currency, 'USD', $fee_max, false)); // $20 in whatever currency
      break;    
  }
  return number_format($fee, 2, '.', '');
  //if(in_array($currency, array('USD', 'EUR', 'GBP', 'CHF'))) return 1; // return 1 USD, 1 EUR, 1 GBP, 1 CHF
  //return get_exchange_rate($currency, 'EUR', $amount, false); // return 1 EUR in whatever currency
}


/** unsused - automatic comment added on  */
function check_withdrawals() {
  global $db;
  $wObjs = find_objects('withdrawal');
  $rows[] = array('id', 'date', 'wAmount', 'pAmount', 'diff', 'transfers', 'wCur', 'pCur', 'wOwner', 'pOwner', 'notes');
  foreach($wObjs as $wObj) {
    $error = '';
    $id = $wObj->id;
    $host_id = $wObj->host_id;
    $currency = $wObj->currency;

    if($pids = db_getCol("select id from payment where withdrawal_id = $id")) { 
      $pid_list = implode(",", $pids);
      $o_ids = db_getCol("select distinct(host_id) from payment where id IN ($pid_list)");
      $p_curs = db_getCol("select distinct(payment_currency) from payment where id IN ($pid_list) AND payment_status=20 and payment_type IN (10,30,60,90) AND gateway_id=10");
      $cur_list = implode(",", $p_curs);
      $p_amount = db_getOne("select sum(payment_net) from payment where id IN ($pid_list)");
     
      $xfer = db_getOne("select sum(amount) from payment where withdrawal_id=$id AND payment_status=20 and payment_type IN (100) and host_id=$host_id");
      //echo("select sum(amount) from payment where withdrawal_id=$id AND payment_status=20 and payment_type IN (100) and host_id=$host_id = $xfer<br>");

      // check owner id
      if(count($o_ids) == 1) {
        if($o_ids[0] == $host_id) {
        } else {
          $error = html_b("$id: Error: withdrawal host_id $host_id != payment host_id $o_ids[0]<br>");
        }
      } else if(count($o_ids) > 1) {
        $error = html_b("$id: Error: withdrawal host_id $host_id != multiple payment host_id<br>");
      } else {
        $error = html_b("$id: Error: withdrawal host_id $host_id != multiple payment host_id (missing)<br>");
      }

      // check currency
      if(count($p_curs) == 1) {
        $pCur = $p_curs[0];
        if($pCur != $currency) {
          $error = html_b("$id: Error: withdrawal currency $currency != payment currency $pCur<br>");
        }
      } else if(count($p_curs) > 1) {
        $error = html_b("$id: Error: withdrawal currency $currency != multiple payment currency<br>");
      } else {
        $error = html_b("$id: Error: withdrawal currency $currency != multiple payment currency (missing)<br>");
      }

      //$output .= "$id: Str: $str<br>";
    } else {
      //$output .= dump($wObj, true);
      $error = html_b("$id: No payments found with this withdrawal id<br>");
      $p_amount = $pCur = $pid_list = '';
    }
    $notes = $error ?: "$currency $wObj->gross => Owner $wObj->host_id ($wObj->recipient) Payments: $pid_list";
    if($update && !$error) db_query("UPDATE withdrawal SET notes='$notes' WHERE id=$id");
    $diff = $p_amount - $wObj->gross;
    $rows[] = array($wObj->id, $wObj->created, round($wObj->gross), round($p_amount), round($diff), round($xfer), $currency, $cur_list, $wObj->host_id, $pid_list, $notes);  
    $total_gross += $wObj->gross;
    $total_diff += $diff;
    $total_xfer += $xfer;
    $total_paid += $p_amount;
  }
  $rows[] = array('', '', $total_gross, $total_paid, $total_diff, $total_xfer, '', '', '', '');  
  return html_table($rows, array('title' => 'Withdrawals', 'class' => 'web20', 'header' => 1, 'footer' => 1));
}

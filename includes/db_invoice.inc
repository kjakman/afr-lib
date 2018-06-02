<?php

function get_currencies($options = array()) {
  global $currencyArray;
  return is_array($currencyArray) && count($currencyArray) ? 
    $currencyArray : 
    array('EUR'=>'Euro', 'USD'=> 'US Dollar', 'GBP'=>'British Pound', 'BTC' => 'Bitcoin');
}

function reservation_invoice_mismatch($resObj_or_id, $options = array()) {
  $resObj = any2obj('reservation', $resObj_or_id);
  if($resObj->status < STATUS_PROPOSED) return 0;
  $res_total = $resObj->grand_total;  
  $options['field'] = 'sum(total) as grand_total';
  $totals = find_reservation_invoices($resObj->res_id, $options);
  $inv_total = is_array($totals) ? array_pop($totals) : $total;
  $mismatch = abs($res_total - $inv_total) > INV_TOLERANCE ? round($inv_total - $res_total) : 0;
  if($options['values'] && $mismatch) return array("reservation" => $res_total, "invoice" => $inv_total, "difference" => $mismatch);
  return $mismatch;
}

function recalculate_invoice($inv_id) {
  if($liObjs = get_lineitems($inv_id)) {
    foreach($liObjs as $liObj) update_lineitem_total($liObj->id);
  }
  //echo("update_invoice_total $inv_id");          
  update_invoice_total($inv_id);
  return true;
}

// new invoice functions

// if monthly = false: creates single invoice for a new reservation and returns the ID
// if monthly = true and period > 1 month: splits into several invoices, returns first ID 
// TODO: if invoice already exists for this reservation, update it

/** no longer in use */
function get_pdf_viewer_url($id, $draft=true, $auth_key='') {
	$d=$draft ? 1 : 0;
	$url = "/invoices/index.php?inv_id=$id&amp;draft=$d";
	if($auth_key) $url.="&amp;".$auth_key; // used for verifictation if user is not logged in
	return $url;
}

function get_pdf_url($obj, $obj_type='invoice') {
  $key = get_object_key($obj_type);
  $id = $obj->$key;
  $ts = stripdate($obj->timestamp);
  $hash = hash('ripemd160', $ts);
  $now = date('YmdHis');
  return "/invoices/{$obj_type}_{$id}-{$hash}-{$now}.pdf";
}


/** unsused - automatic comment added on  */
function invoice_button($invObj, $user_level=0, $small=false) {
  $auth_key = 'ts='.stripdate($invObj->created)."&amp;ul=$user_level";
  $url = get_pdf_viewer_url($invObj->id, $draft, $auth_key);
  $icon = $small ? '/images/icons/acrobat_tiny.png' : '/images/icons/acrobat.png';
  return html_image_link($url, $icon, phrase('view', CAPITALIZE).' '.phrase('pdf', UPPERCASE), array('full' => true, 'class' => 'non-html'));
}

function get_pdf_path($invObj) {
	return docroot(false).get_pdf_url($invObj);
}

// delete old versions of invoice on disk
function purge_invoices($inv_id) {
  $path = docroot()."/invoices/";
  //echo("path=$path<br>");
  foreach (glob("$path/invoice_".$inv_id."-*.pdf") as $filename) {
    unlink($filename);
    //echo("rm $filename<br>");
  }
}


/** unsused - automatic comment added on  */
function add_inv_lineitem($inv_id, $currency, $amount, $type, $notes="", $user=0) {
  global $db;
  $id = nextId("inv_lineitem");
  $now=date("Y-m-d H:i:s");

	$draft = true;

	$inv_table = get_object_table('invoice');
  $inv_currency = db_getOne("SELECT currency from $inv_table WHERE id=$inv_id");
  if($currency != $inv_currency) die("add_inv_lineitem: Line item currency ($currency) <> inv currency ($inv_currency) of invoice $inv_id: amount=$amount type=$type");

  $query_str = "INSERT INTO inv_lineitem
                (id, inv_id, type, currency, amount, description, user_id, created)
         VALUES ($id, $inv_id, '$type', '$currency', '$amount', '$notes', '$user', '$now')";
  db_query($query_str);

	$data['inv_id'] = $inv_id;
	$data['quantity'] = 1;
	$data['amount'] = $amount;
	$data['tax_percentage'] = $data['shipping'] = $data['handling'] = 0;
	post_edit_inv_lineitem($id, $data);
		
  return $id;
}

// returns first open invoice for given res. id
// returns the booking invoice for given res. id
function find_booking_authorization($invObj_or_id) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $inv_id = $invObj->id;
  return find_object('payment', array('inv_id' => $inv_id, 'payment_type' => PAYMENT_TYPE_CREDIT_CARD_AUTH, 'payment_status' => PAYMENT_STATUS_PENDING));
}

// returns first open invoice for given res. id
// returns the booking invoice for given res. id
function find_booking_invoice($resObj_or_id) {
  $resObj = any2obj('reservation', $resObj_or_id);
  $res_id = $resObj->res_id;
  $options = array('sort' => "start_date");
  $invs = find_objects('invoice', array('res_id' => $res_id, 'type' => array(INV_TYPE_BOOKING, INV_TYPE_COMPLETE)), $options);
  return $invs[0];
}

// returns first open invoice for given res. id
// returns the booking invoice for given res. id
function find_order_invoice($orderObj_or_id) {
  $orderObj = any2obj('payment_order', $orderObj_or_id);
  return find_object('invoice', array('order_id' => $orderObj->id));
}


/** unsused - automatic comment added on  */
function find_first_open_invoice($res_id) {
  $invObj = find_booking_invoice($res_id); // first try booking invoice
  if($invObj->balance >= 1) return $invObj;
  
  $invs = find_objects('invoice',  array('res_id' => $res_id, 'balance' => array('>', INV_TOLERANCE)), array('sort' => "start_date"));
  return $invs[0];  
}

// returns array of reservation invoice(s) for given res. id (not correction or supplemental)
// should be exactly 1 invoice, unless monthly billing
// case 1: booking
// case 2: complete
// case 3: booking [1], partial [2], partial [3], ....

/** unsused - automatic comment added on  */
function find_invoices($key, $id, $skip_invoice_id=0) {
  if(!$skip_invoice_id) return find_objects('invoice', array($key => $id), array('sort' => 'start_date'));
  return find_objects('invoice', array($key => $rid, 'invoice.id' => array('!=', $skip_invoice_id)), array('sort' => "sequence, start_date"));  
}

function booking_invoice($resObj_or_id, $fields='') {
  $res_id = any2id('reservation', $resObj_or_id);
  $invSearch = array();
  $invSearch['parent_type'] = 'reservation';
  $invSearch['parent_id'] = $res_id;  
  $invSearch['status'] = array('>=', INV_STATUS_OPEN);
  $invSearch['head_invoice'] = 1;  
  return find_object('invoice', $invSearch, $fields);  
}

function find_reservation_invoices($res_id, $options=array()) {
  if(!$res_id) return null;
  $field = $options['field'];
  $sort = $options['sort'] = $options['sort'] ?: 'start_date';
  $search = $options['search'];
  $invSearch['res_id'] = $res_id;
  if(!$search['status']) $invSearch['status'] = array('>=', INV_STATUS_DRAFT);
  if(is_array($search)) $invSearch = array_merge($invSearch, $search);
  
  return find_objects('invoice', $invSearch, $options);
}


/** unsused - automatic comment added on  */
function find_block_invoice($block_id) {
  return find_object('invoice', array('block_id' => $block_id, 'type' => INV_TYPE_BLOCK));
}

// takes amount (excl. tax), calculates tax to be added 
function get_tax_added($amount, $tax_rate) {
  return round($amount * ($tax_rate / 100), 2);
}

// takes acmount (including tax), returns the tax included
function get_tax_included($amount, $tax_rate) {              
  return $amount - get_ex_tax($amount, $tax_rate);
}

// takes an amount (including tax), returns amount excluding tax
function get_ex_tax($amount, $tax_rate) {
  return round($amount / (($tax_rate / 100) + 1), 2);
}

// called by post_edit_inv_lineitem to update total etc 
// should we use li_tax() ?
function update_lineitem_total($id) {
  if(!isint($id)) {
    
    print_log("running update_lineitem_total for missing or incorrect id=$id<br>", "invoice", LOG_LEVEL_TEST);
    return;
  }
	$liObj = get_object('inv_lineitem', $id);
	$inv_id = $liObj->inv_id;
  $invObj = get_object('invoice', $inv_id);

  print_log("running update_lineitem_total id=$id inv_id=$inv_id", "invoice", LOG_LEVEL_TEST);
  
	// subtotal
	$li_subtotal = lineitem_subtotal($liObj);
	
  // calculate tax	
  //$tax_cat = get_tax_category($li);
  $tax_cat = $liObj->tax_category;
  $tax_rate = get_tax_rate($tax_cat, $invObj->start_date); // todo: remove (BTW October 1 2012)
  //echo("tax rate for tax cat $tax_cat on $invObj->start_date = $tax_rate<br>invoice tax=$invObj->charge_tax<br>included=$invObj->tax_included");
  //$tax_rate = $liObj->tax_percentage;

	if($invObj->charge_tax && $tax_rate > 0) {
	  //$invObj->tax_included = 1; // tax_included hack
    if($invObj->tax_included) {
      $li_tax = 0;
      $total_ex = $li_subtotal / (1 + $tax_rate / 100);
      $li_tax_of_which =  $li_subtotal - $total_ex;
    } else {
      $li_tax_of_which = 0;
      $li_tax = $li_subtotal * ($tax_rate/100);
    }
  } else {
    $li_tax = $li_tax_of_which = 0;
  }

	// add Tax, S&H
	$li_total = $li_subtotal + $li_tax + $liObj->shipping + $liObj->handling;
  //dump($id);
  $account_number = lineitem_account_number($liObj);
  db_query("UPDATE inv_lineitem SET tax='$li_tax', tax_of_which='$li_tax_of_which', tax_percentage='$tax_rate', subtotal='$li_subtotal', total='$li_total', account_number='$account_number' WHERE id=$id");

  //print_log(" update_lineitem_total id=$id<br>", "invoice", LOG_LEVEL_TEST);

  // update invoice total
  print_debug("calling update_invoice_total for $inv_id from update_lineitem_total (li: $id)<br>");
	update_invoice_total($inv_id);

}                                                         

function lineitem_fee_types() {
  return [140,150,160,180,190,181,182,191,192,193,194,135,183]; // faster
  return db_getCol("select id from lineitem_type where name like '%_fee'"); // future proof 
}

//function lineitem_fee_types() {
//  return array(LI_TYPE_BOOKING_FEE, LI_TYPE_CLEANING_FEE,LI_TYPE_LOCAL_FEE,LI_TYPE_PICKUP_FEE,LI_TYPE_OTHER_FEE,LI_TYPE_CALLOUT_FEE,LI_TYPE_WAITING_FEE,LI_TYPE_CC_FEE);
//}

function lineitem_rent_types() {
  return array(LI_TYPE_DEPOSIT, LI_TYPE_BALANCE, LI_TYPE_RENT, LI_TYPE_CHANGE_LENGTH, LI_TYPE_CHANGE_GUESTS, LI_TYPE_CHANGE_PROPERTY, LI_TYPE_CHANGE_RATE, LI_TYPE_CHANGE_DISCOUNT, LI_TYPE_CHANGE_TOTAL);
}

/** unsused - automatic comment added on  */
function lineitem_is_rent($li_type) {
  $rent_types = lineitem_rent_types();
  return in_array($li_type, $rent_types) ? 1 : 0;
}

function lineitem_account_number($liObj_or_id) {
  $liObj = any2obj('inv_lineitem', $liObj_or_id);
  if($inv_id = $liObj->inv_id) {    
    if($apt_id = get_object('invoice', $inv_id, 'apt_id')) {
      $aptObj = get_object('property', $apt_id);
      if(lineitem_is_rent($liObj->type)) return $aptObj->account_rent; 
      if($liObj->type == LI_TYPE_CITY_TAX && $aptObj->account_city_tax) return $aptObj->account_city_tax;
      if($liObj->type == LI_TYPE_BOOKING_FEE && $aptObj->account_booking_fee) return $aptObj->account_booking_fee;
      switch($liObj->tax_category) {
        case 'N':
          return $aptObj->account_no_tax;
        case 'L':
          return $aptObj->account_lo_tax;
        case 'H':
          return $aptObj->account_hi_tax;
        default:
          return '';          
      }
    }
  }
  return '';            
}

// this is called only *once* by pre_add_inv_lineitem
// calling it more times would screw up price if tax is included on product but not on invoice or vice versa
function li_tax($liObj_or_id) {
  $errors = array();
  $liObj = any2obj('inv_lineitem', $liObj_or_id); 
  if(!$liObj) return array(0,0,0,0, array("li_tax: Missing lineitem"));
  $inv_id = $liObj->inv_id;
  if(!$inv_id) return array(0,0,0,0, array("li_tax: Missing invoice ID"));
  $invObj = get_object('invoice', $inv_id); 
  $amount = $liObj->amount;
	$subtotal = lineitem_subtotal($liObj);
  $tax_rate  = $liObj->tax_percentage;
  
  if($product_id = $liObj->product_id) {
    $productObj  = get_object('product', $product_id);
    $tax_rate = $productObj->tax_category ? get_object('tax_category', $productObj->tax_category, 'percentage') : 0; 
    $tax_included  = $productObj->tax_included;

    //echo("tax rate of $productObj->name = $tax_rate<br>");
    print_log("li_tax product_id=$product_id tax_rate=$tax_rate tax_include=$tax_included", 'invoice', LOG_LEVEL_DEBUG);    

    if(!($tax_rate > 0)) return array($amount, $subtotal, 0, 0, $errors); // no tax

	  $tax_included = 1; // tax_included hack
	  $invObj->tax_included = 1; // tax_included hack
    
    if($tax_included) { // tax is included in this product
      $tow = get_tax_included($subtotal, $tax_rate);

      if($invObj->charge_tax && $invObj->tax_included) return array($amount, $subtotal, 0, $tow, $errors);  // tax is included in product and on invoice 
      
      $amount_excl = get_ex_tax($amount, $tax_rate);
      $subtotal_excl = $subtotal - $tow;
      if(!$invObj->charge_tax) return array($amount_excl, $subtotal_excl, 0, 0, $errors); // don't charge tax, so remove from amount

      $tax = $subtotal - $subtotal_excl;
      if(!$invObj->tax_included) return array($amount_excl, $subtotal_excl, $tax, 0, $errors); // tax is not included on invoice (but it is on product) recalculate  
      
    } else { // tax is not included in this product
      $tax = get_tax_added($subtotal, $tax_rate);

      if($invObj->charge_tax && !$invObj->tax_included) return array($amount, $subtotal, $tax, 0, $errors);  // tax is not included in product and also not on invoice 

      if(!$invObj->charge_tax) return array($amount, $subtotal, 0, 0, $errors); // don't charge tax, not included in product, so just return


      $subtotal_incl = $subtotal + $tax; 
      $amount_incl = $amount + get_tax_added($amount, $tax_rate);

      if($invObj->tax_included) return array($amount_incl, $subtotal_incl, 0, $tax, $errors); // tax is not included on product (but it is on invoice) recalculate  

    }
        
  } else { // this is not a product, but (presumably) property rental. If tax is included on property object, it also will be on the invoice, so assume this    
    
    if(!$invObj->charge_tax) return array($amount, $subtotal, 0, 0, $errors);
	  // $invObj->tax_included = 1; // tax_included hack
    if($invObj->tax_included) {  
      $tax = 0;
      $tow = get_tax_included($subtotal, $tax_rate);
    } else {
      $tax = get_tax_added($subtotal, $tax_rate);
      $tow = 0;
    }
    //echo("Id=$invObj->id Tax included=$invObj->tax_included Tax rate for ".get_object('lineitem_type', $li->type, 'name')." = $tax_rate %. tax=$tax tax_of_which = $tow<br>");
    //echo("ct=$invObj->charge_tax ti=$invObj->tax_included ");dump($li);
    return array($amount, $subtotal, $tax, $tow, $errors);
  }
}


  // if(is_array($liObj)) $liObj = array2obj($liObj);
  // $tax_rate = $liObj->tax_percentage;
  // if(!$liObj || !($tax_rate > 0) || $invObj->tax_included) return 0;
  // $subtotal = lineitem_subtotal($liObj);
  // //$total = $liObj->quantity * $liObj->amount;
  // //$total = apply_discount($total, $liObj->discount);
  // return tax_added($subtotal, $tax_rate); // round(lineitem_subtotal($liObj) * ($tax_rate / 100), 2);

  // if(is_array($liObj)) $liObj = array2obj($liObj);
  // $tax_rate = $liObj->tax_percentage;
  // if(!$liObj || !($tax_rate > 0) || !$invObj->tax_included) return 0;
  // $subtotal = lineitem_subtotal($liObj);
  // $total_ex =  ex_tax($subtotal, $tax_rate); // $subtotal / (1 + $tax_rate / 100);
  // return 
  // return round($subtotal - $total_ex, 2);



// returns assoc array with [tax_rate] => total for a set of lineitems
function get_of_which_tax($invObj, $liObjs) {  
	//if($invObj->charge_tax) $invObj->tax_included = 1; // tax_included hack
  if(!count($liObjs) || !$invObj->tax_included) return array();
  $result = array();
  foreach($liObjs as $liObj) {
    if($liObj->tax_percentage > 0) {
      $result[$liObj->tax_percentage] += $liObj->tax_of_which;
    }
  } 
  return $result;
}

/** unsused - automatic comment added on  */
function fractional_lineitems($lineitems) {
  foreach($lineitems as $li) {    
    if(floor($li->quantity) != $li->quantity) return true;
  }
  return false;
}

function sum_lineitems($lineitems) {
  if(!$lineitems) return 0;
  foreach($lineitems as $li) {    
    $total += $li->total;
    //$li_total = apply_discount($li->amount, $li->discount);
    //$total += $li->quantity * $li_total;
  }
  return round($total, 2); 
}

function get_due_now($invObj_or_id, $resObj_or_id=NULL) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $resObj = $resObj_or_id ? any2obj('reservation', $resObj_or_id) : NULL;
  $deposit_percentage = $resObj->deposit_percentage;
  
  
  $total = $invObj->total;
  $paid = $invObj->paid;
  $balance = $invObj->balance;
  $deposit = round($total * $deposit_percentage / 100, 2);
  $head = $invObj->head_invoice;
  $sequence = $invObj->sequence;
  $type = $invObj->type;
  $inv_id = $invObj->invoice_number ?: $invObj->id;
  
  print_log("get_due_now: id=$inv_id perc: $deposit_percentage total=$total paid=$paid balance=$balance deposit=$deposit head=$head seq=$sequence type=$type", 'invoice', LOG_LEVEL_TEST);

  if(!$resObj && $invObj->res_id) $resObj = get_object('reservation', $invObj->res_id);
  if(!$resObj) return $balance; // all due
  if($deposit_percentage <=0 || $deposit_percentage >= 100) return $balance; // all due  
  if($invObj->type != INV_TYPE_BOOKING || !$invObj->head_invoice) return $balance; // all due
  if($invObj->sequence > 1) return $balance;
  
  
    
  $due = $deposit - $paid;


  
  //if($resObj->status > STATUS_PROPOSED) return $balance; // all due unless proposed

  // ok, so it's the head booking invoice and there is a deposit percentage on the reservation
  //if(!$_SESSION['live']) echo("Get due now total=$total paid=$paid balance=$balance perc=$deposit_percentage deposit=$deposit due=$due type=$invObj->type head=$invObj->head_invoice<br>");
  if($due >= INV_TOLERANCE) {
    //if(!$_SESSION['live']) echo("Returning due now = $due_now<br>");
    print_log("get_due_now: OK due=$due", 'invoice', LOG_LEVEL_TEST);
    return $due;
  }
  //if(!$_SESSION['live']) echo("Returning balance = $balance<br>");
  return $balance;
}
  

// returns the lineitems for given inv. id
function get_lineitems($inv_id, $sort='type', $order=0, $field='') {
  return find_objects('inv_lineitem', ['inv_id' => $inv_id], array('sort' => $sort, 'order' => $order, 'fields' => $field));  
}


// for booking invoice, returns deposit + certain fees or (if already paid) all other lineitems
// all others, return all lineitems
function get_due_lineitems($invObj) {

  $inv_id = $invObj->id;
  $total = $invObj->total;  
  $balance = $invObj->balance;
  $li_due = array();
                
  if(!$inv_id || $balance <= 1) return $li_due;
  $liObjs = get_lineitems($inv_id);
  return $liObjs;
  
}                             


function authorized_payments($invObj_or_id) {
  $inv_id = any2id('invoice', $invObj_or_id);
  if(!$inv_id) return false;
  $authorized = 0;
  return $authorized; /** todo: implement */
}

// before:returns true if there are any pending payments for given invoice id
// now: returns sum of pending payments for given invoice id
function pending_payments($invObj_or_id) {
  $inv_id = any2id('invoice', $invObj_or_id);
  if(!$inv_id) return false;
  $qs = "SELECT sum(amount) FROM payment WHERE payment_status=".PAYMENT_STATUS_PENDING." AND inv_id=$inv_id";
  $pending = db_getOne($qs) ?: 0;
  print_log("pending_payments ($inv_id): pending=$pending qs=$qs", 'invoice', LOG_LEVEL_TEST);
  return $pending; // check if there are pending payments            
}      

function get_invoice_paid($invObj_or_id) {                 
  $inv_id = any2id('invoice', $invObj_or_id);
  if(!$inv_id) return false;
  $paid_statuses = implode(',', array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED));
  $qs = "SELECT sum(amount) FROM payment WHERE inv_id=$inv_id AND payment_status IN ($paid_statuses)";
  $paid = db_getOne($qs);                                          
  return $paid > 0 ? $paid : '0.00';
}

// runs whenever a lineitem is added or edited
function update_invoice_total($id) {
  print_log("running update_invoice_total for $id", 'invoice', LOG_LEVEL_TEST);
	if(!$id) {
	  //echo("running update_invoice_total: missing ID");
	  return;
	}
	
	global $db;
	$obj_type = 'invoice';
	$obj = $invObj = get_object($obj_type, $id);
	$key = get_object_key($obj_type);
	$table = get_object_table($obj_type);
	$res_id = $obj->res_id;
	$now = now();

  // $changed_fields = array_keys($changes);
  // foreach($changes as $field => $change) {
  //   $paid_fields = array('paid', 'paid_date', 'status');
  //   if(in_array($field, $paid_fields)) $paid_now = true;
  // }

  // update invoice - todo: use stored procedure
  $parent_field = $obj->type == INV_TYPE_BLOCK ? 'block_parent' : 'inv_id';

  $query = "SELECT sum(tax) as tax,
                                   sum(tax_of_which) as tax_of_which,
                                   sum(shipping) as shipping, 
                                   sum(handling) as handling, 
                                   sum(amount) as amount, 
                                   sum(subtotal) as subtotal, 
                                   sum(total) as total, 
                                   count(*) as lineitems 
                              FROM inv_lineitem WHERE $parent_field=$id";
  $inv_total = db_getRow($query);
  //print_log("update_invoce_total: query1=$query", "invoice", LOG_LEVEL_TEST);
  //print_log("update_invoce_total: total=".dump($inv_total, true), "invoice", LOG_LEVEL_TEST);
  $paid = get_invoice_paid($obj);
  $pending = pending_payments($obj);
  $authorized = authorized_payments($obj);

  //echo("paid for $obj->id = $paid<br>");
  $balance = $inv_total->total - $paid;
  //dump($inv_total);
  if($balance < INV_TOLERANCE) $status_term = "status = ".INV_STATUS_COMPLETE.", locked=1, ";
  $query = "UPDATE $table SET lineitems=$inv_total->lineitems, 
                              tax='$inv_total->tax',
                              tax_of_which='$inv_total->tax_of_which',
                              shipping='$inv_total->shipping',
                              handling='$inv_total->handling',
                              amount='$inv_total->amount', 
                              subtotal='$inv_total->subtotal', 
                              total='$inv_total->total', 
                              authorized='$authorized',
                              pending='$pending',
                              paid='$paid',
                              $status_term
                              balance='$balance'
                       WHERE $key=$id";
  db_query($query);
  //print_log("update_invoce_total: query2=$query", "invoice", LOG_LEVEL_TEST);
  // update reservation other_charges
  $debug = true;
  if($debug) print_log("update_invoice_total: res_id=$res_id", "invoice", LOG_LEVEL_ALWAYS);
  
  if($res_id) {
    if($inv_ids = find_reservation_invoices($res_id, array('field' => 'invoice.id'))) {
      //dump($inv_ids);
      $inv_id_list = implode(',', $inv_ids);
      $extra_li_types = implode(',', array(LI_TYPE_PRODUCT, LI_TYPE_SERVICES, LI_TYPE_EXTRAS, LI_TYPE_CATERING));
      $extras = db_getOne("SELECT sum(total) FROM inv_lineitem WHERE inv_id IN ($inv_id_list) AND type IN ($extra_li_types)"); 
      if($debug) print_log("update_invoice_total: other charges=$extras", "invoice", LOG_LEVEL_ALWAYS);
      if($extras > 0 || $extras < 0) {
        db_query("UPDATE reservation SET other_charges = '$extras' WHERE res_id=$res_id");
        booking_update_total($res_id);
      }
    }
  }

  if($order_id = $invObj->order_id) {
    update_paid($order_id);
  }
}

// runs whenever a payment is added or edited
function update_invoice_payments($id) {

  print_log("update_invoice_payments: $id", 'invoice', LOG_LEVEL_TEST);
  
  if(!$id) {
    $rerrors[] = "update_invoice_payments: missing id";
    return $errors;
    // mydie("update_invoice_payments: missing id");
  }
  // echo("Running post_edit_invoice id=$id<br>");
  //dump($changes);
	global $db;
	$res_changes = array();
	$obj_type = 'invoice';
	$obj = $invObj = get_object($obj_type, $id);
	$key = get_object_key($obj_type);
	$table = get_object_table($obj_type);
	$res_id = $obj->res_id;
	$now = now();
  $parent_field = $obj->type == INV_TYPE_BLOCK ? 'block_parent' : 'inv_id';
  $parent_id = $obj->parent_id;
  $parent_type = $obj->parent_type;

  /**
	$ignore_payment_types = implode(',', array(PAYMENT_TYPE_TRANSFER, PAYMENT_TYPE_AGENT_OWES));
	$valid_statuses = implode(',', array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED)); // don't count pending payments
  $paid = db_getOne("SELECT sum(amount) FROM payment WHERE $parent_field=$id AND payment_status in (".$valid_statuses.") AND payment_type NOT IN (".$ignore_payment_types.")") ?: 0;
  */
  
  $paid = get_invoice_paid($id);
  
  $total = $obj->total;
  $balance = $obj->total - $paid;
  $pending_payments = $pending = pending_payments($id) ?: 0;
  $pending_balance = $pending_payments > 0 ? $balance - $pending_payments : $balance; // balance when counting pending payments 

  print_log("update_invoice_payments: $id paid=$paid pending=$pending  pt=$parent_type pid=$parent_id", 'invoice', LOG_LEVEL_TEST);
  
  if($total > 0 && $balance < INV_TOLERANCE) { // paid in full
    $status = INV_STATUS_COMPLETE; // complete if balance < 1 (to allow for rounding errors) 
  } elseif($pending_balance < INV_TOLERANCE) { // paid in full if counting pending payments
    $status = INV_STATUS_PENDING; // complete if balance < 1 (to allow for rounding errors) 
  } elseif($paid > 0) { // received in part
    $status = INV_STATUS_PARTIAL;
  } 
  
  $status_change = $status != $obj->status;  // todo: use post_edit_invoice class handler for this
  $changes = $paid != $obj->paid || $pending != $obj->pending || $status_change;
  
  if($changes) {
    $locked = $status == INV_STATUS_COMPLETE;
    $status_change_term = $status_change ? "status_change_timestamp = '".now()."'," : '';
    db_query("UPDATE $table SET status='$status',
                                locked='$locked',
                                paid='$paid',
                                pending='$pending_payments',
                                $status_change_term 
                                balance='$balance'
                          WHERE $key=$id"); // set status changed timestamp  
  
  }
  /** catchall to update parent paid/balance.
      used for uhuru_registration 
      post_add_payment handles reservation paid/order paid
  */
  
  
  if($parent_type && $parent_id) {
    $pObj = get_object($parent_type, $parent_id);
    print_log("updating parent (total={$pObj->total}) with paid=$paid", 'invoice', LOG_LEVEL_TEST);
    if(isset($pObj->total) && isset($pObj->paid) && isset($pObj->balance) && $pObj->total == $invObj->total && $pObj->paid != $paid) {
      $total = $pObj->total;
      update_object($parent_type, ['paid' => $paid, 'balance' => $total - $paid], $parent_id); 
    }
  }
  

  // todo: don't count payments for invoices like drinks etc for the reservation
  /**
  This is handled by update_reservation_paid
 
  if($res_id) {
    $resObj = get_object('reservation', $res_id);
    $paid_field = $obj->type == INV_TYPE_BLOCK ? 'block_paid' : 'paid';
    $resData[$paid_field] = $paid;
    if($paid != clean_num($resObj->paid)) {
      list($rid, $rerrors)  = update_object('reservation', $resData, $res_id); // trigger res obj handlers if paid 
    } 
    
    if($pending_payments > 0) { // we have pending payments
      if($resObj->status < STATUS_RESERVED) update_object('reservation', array('status' => STATUS_RESERVED), $resObj->res_id); // Still set it to reserved, though payments are pending (promise or auth)
    }
  }  
  */
  

  $block_id = $obj->block_id;
  $block_inv_id = $obj->block_parent;
  //echo("Update invoice payments: inv_id=$id res_id=$res_id block_id=$block_id block_parent=$block_inv_id<br>");
  if($block_id && $block_inv_id && $block_inv_id != $id) {
    //echo("calling update_invoice_payments with inv_id=$block_inv_id<br>");
    update_invoice_payments($block_inv_id);
  }
  //$blockInvObj = get_object('invoice', $block_inv_id);

  return $rerrors;
}

function booking_payments($resObj_or_id) {
  $res_id = any2id('reservation', $resObj_or_id);
  $paymentObjs = find_objects('payment', array('res_id' => $res_id));
  return $paymentObjs;
}

function invoice_payments($invObj_or_id) {
  $inv_id = any2id('invoice', $invObj_or_id);
  // 'payment_status' => array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PENDING, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED)
  $paymentObjs = find_objects('payment', array('inv_id' => $inv_id));
  return $paymentObjs;
}

/** returns rows to print payments in table */
function payment_rows($payments, $user_level=USER_LEVEL_GUEST) {
  if(!count($payments)) return [];
  $rows = array();
  $rows[]=  array(html_b(phrase('date', CAPITALIZE)), html_b(phrase('method', CAPITALIZE)), html_b(phrase('amount', CAPITALIZE)), html_b(phrase('status', CAPITALIZE)), html_b(phrase('invoice', CAPITALIZE)), html_b(phrase('notes', CAPITALIZE)));
  $refundable_types = find_objects('payment_type', array('refundable' => 1), array('fields' => 'id'));
  $refundable_statuses = array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PARTIALLY_REFUNDED);
  foreach($payments as $paymentObj) {
    $actions = array();
    $ignore_payment_types = array(PAYMENT_TYPE_TRANSFER, PAYMENT_TYPE_AGENT_OWES);
    if($user_level == USER_LEVEL_GUEST && in_array($paymentObj->payment_type, $ignore_payment_types)) continue; // don't show for guests
    if($user_level == USER_LEVEL_GUEST && $paymentObj->payment_source != PAYMENT_SOURCE_GUEST) continue; // don't show for guests
    /** turned off for now
    if($user_level >= USER_LEVEL_GUEST) $actions = class_actions_payment('payment', $paymentObj, 'recordset', $ref_name, $ref_link, $user_level);
    if($user_level >= USER_LEVEL_STAFF) $actions = array_merge($actions, default_class_actions('payment', $paymentObj->id, 'payment', '',0, 0, 'Status', urlencode($_SERVER['REQUEST_URI']), 'recordset')); // add standard buttons or links
    */
    if($user_level >= USER_LEVEL_STAFF) {
      global $u_id;
      $site_id = $_SESSION['site_id'];
      $verified = $paymentObj->verified;
      $can_refund = $verified && $paymentObj->amount > 0 && in_array($paymentObj->payment_type, $refundable_types) && in_array($paymentObj->payment_status, $refundable_statuses);
      if($verified) $actions[] = html_link("/ajax.php?oper=verify-payment&site_id=$site_id&user_id=$u_id&payment_id=$paymentObj->id", html_i('', 'fa fa-check text-success'), array('class' => '', 'title' => phrase('verified', CAPITALIZE), 'data-data' => array('payment_id' => $paymentObj->id, 'gateway_id' => $paymentObj->gateway_id), 'data-toggle' => "modal", 'data-target' => "#verify_modal"));
      if($can_refund) {
        $icon = '<span class="fa-stack fa-lg"><i class="fa fa-money fa-stack-1x text-success"></i><i class="fa fa-long-arrow-left fa-stack-1x "></i></span>';
        $actions[] = html_link('#', $icon, array('class' => 'fill-form', 'data-form' => '#refund_form', 'data-data' => array('payment_id' => $paymentObj->id, 'amount' => $paymentObj->amount, 'currency' => $paymentObj->currency), 'data-toggle' => "modal", 'data-target' => "#refund_modal", 'title' => phrase('refund', CAPITALIZE)));
      }
    }
    
    $actions_str = $actions ? implode('', $actions) : '';

    $payment_status = get_object('payment_status', $paymentObj->payment_status, 'name');

    // payment amount
    $cur = get_currency_symbol($paymentObj->currency, true);
    $pay_amount  = formatFloat($paymentObj->amount, FORMAT_ALWAYS, $cur);
    if($paymentObj->payment_currency && $paymentObj->payment_gross && $paymentObj->currency != $paymentObj->payment_currency) {
      $rcv_cur    = get_currency_symbol($paymentObj->payment_currency, true);
      $rcv_amount  = $paymentObj->payment_currency == 'BTC' ? "$rcv_cur ".clean_num($paymentObj->payment_gross) : formatFloat($paymentObj->payment_gross, FORMAT_ALWAYS, $rcv_cur);
      $pay_amount_str = $rcv_amount.' => '.$pay_amount;
    } else {
      $pay_amount_str = $pay_amount;
    }

    $row = array(sql2human($paymentObj->payment_date ?: $paymentObj->created, array('show_time' => false)), get_object('payment_type', $paymentObj->payment_type, 'name'), $pay_amount_str);
    if($payment_status) $row[] = $payment_status;
    $invoice_id = $paymentObj->invoice_id ?: $paymentObj->inv_id;
    $row[] = html_link("#invoice_{$paymentObj->inv_id}",$invoice_id);
    $row[] = $paymentObj->description;
    if($actions_str) $row[] = $actions_str;    
    $rows[]= $row;
  }
  return $rows;
}

/** returns rows to print invoice as table */
function invoice_rows($invObj_or_id, $auth_level=0, $options = []) {
  $auth_edit = $auth_level >= USER_LEVEL_STAFF;
  $due_now = $options['due_now'];
  $context = $options['context'];

  $context = $options['context'] ?: 'screen';
  $invObj = any2obj('invoice', $invObj_or_id);
  $inv_id = $invObj->id;
  if(!$inv_id) return;
  $draft = $invObj->status == INV_STATUS_DRAFT; 
  $currency = $invObj->currency;
  $cur = get_currency_symbol($currency, true);

  $parent_field      = $invObj->type == INV_TYPE_BLOCK ? 'block_parent' : 'inv_id'; 
  $show_quantity = true;
  $show_discount     = db_getOne("SELECT abs(sum(discount)) FROM inv_lineitem WHERE $parent_field=$inv_id") > 0;
  $show_tax          = db_getOne("SELECT abs(sum(tax)) FROM inv_lineitem WHERE $parent_field=$inv_id") > 0;
  $show_tax_of_which = db_getOne("SELECT abs(sum(tax_of_which)) FROM inv_lineitem WHERE $parent_field=$inv_id") > 0;

  $num_cols = 1;  // total
  if($show_quantity) $num_cols++; // quantity
  if($show_quantity || $show_discount) $num_cols++; // unit price
  if($show_discount) $num_cols++; // discount
  if($show_tax || $show_tax_of_which) $num_cols++; // tax
  
  $pad_cols = $show_tax_of_which ? $num_cols - 1 : $num_cols;

  // header row
  $header_row[] = html_b(phrase('description', CAPITALIZE));
  if($show_quantity) {
    $header_row[] = html_b(phrase('quantity', CAPITALIZE));
  }
  if($show_quantity || $show_tax || $show_discount) {
    $header_row[] = html_b(phrase('amount', CAPITALIZE));
    if($show_discount) $header_row[] = html_b(phrase('discount', CAPITALIZE));
    if($show_tax) $header_row[] = html_b(phrase('V.A.T.', UPPERCASE));  
    $header_row[] = html_b(phrase('total', CAPITALIZE));
    if($show_tax_of_which) $header_row[] = html_b(phrase('Of_which_VAT'));  
  } else {
    $header_row[] = html_b(phrase('amount', CAPITALIZE));  
    if($show_tax_of_which) $header_row[] = html_b(phrase('Of_which_VAT'));  
  }
  if($auth_edit) $header_row[] = html_b(phrase('actions', CAPITALIZE));  
  
  $rows[]= $header_row;  
  $liObjs = get_lineitems($inv_id);

  $booking_fee = false;

  // line items
  $qs = get_query_string();
  $q_str = $qs ? "?".urlencode($qs) : '';
  foreach($liObjs as $liObj) {
    $id = $liObj->id;
    $li_fields = array('inv_id', 'currency', 'type', 'quantity', 'amount', 'discount');
    if($invObj->charge_tax) $li_fields[] = 'tax_category';
    $li_fields[] = 'description';

    $row = array();    
    $li_desc = $liObj->description ?: phrase(get_object('lineitem_type', $liObj->type, 'name'), CAPITALIZE);
    $row[] = html_div($li_desc.$buttons, 'relative');
    
    if($show_quantity) {
      $row[] = formatFloat($liObj->quantity);
    }
    if($show_quantity || $show_tax || $show_discount) {
      $row[] = formatFloat($liObj->amount, FORMAT_ALWAYS, $cur);
      if($show_discount) $row[] = $liObj->discount ?: '';
      if($show_tax) $row[] = formatFloat($liObj->tax, FORMAT_ALWAYS, $cur);
      $row[] = formatFloat($liObj->total, FORMAT_ALWAYS, $cur);
      if($show_tax_of_which) $header_row[] = $row[] = formatFloat($liObj->tax_of_which, FORMAT_ALWAYS, $cur);
    } else {
      $row[] = formatFloat($liObj->total, FORMAT_ALWAYS, $cur);
      if($show_tax_of_which) $header_row[] = $row[] = formatFloat($liObj->tax_of_which, FORMAT_ALWAYS, $cur);
    }

    if($auth_edit) {
      $edit_link = html_link('#', html_i('', "fa fa-pencil-square"), 
        array(
          "class" => "text-success edit-object",
          "data-fields" => array("inv_id", "user_id","type","tax_category","currency", "quantity","amount","description"), 
          "data-obj_type" => "inv_lineitem", 
          "data-obj_id" => $liObj->id, 
          "data-reload_link" => "#invoice-overview"));
      $delete_link = html_link('#', html_i('', "fa fa-times"), 
        array(
          "class" => "text-danger delete-object",
          "id" => "delete-inv_lineitem-$id",
          "data-toggle" => "confirmation", 
          "data-id" => $liObj->id, 
          "data-obj_type" => "inv_lineitem", 
          "data-reload_link" => "#invoice-overview"));
      $row[] = $edit_link.' '.$delete_link;  
    }
    $rows[]=  $row;
    if($liObj->type == LI_TYPE_BOOKING_FEE) $booking_fee = true;
  }

  // paid, due, total, balance
  if($show_quantity || $show_tax || $show_discount) {
    $total_row[]= html_b(phrase('total', CAPITALIZE)); 
    if($show_quantity) $total_row[] = '';
    $total_row[] = '';
    if($show_discount) $total_row[] = '';
    if($show_tax) $total_row[]= html_b(formatFloat($invObj->tax, FORMAT_ALWAYS, $cur));
    $total_row[]= html_b(formatFloat($invObj->total, FORMAT_ALWAYS, $cur));
  } else {      
    $total_row = array(html_b(phrase('total', CAPITALIZE)), html_b(formatFloat($invObj->total, FORMAT_ALWAYS, $cur))); 
  }

  if($rid = $invObj->res_id) {
    $resObj = get_object('reservation', $rid);
    $res_status = get_object('reservation', $rid, 'status');
    $deposit_percentage = $resObj->deposit_percentage;
    $due_now = get_due_now($invObj, $resObj);   // how much is due now (either full amount or percentage of total
    $status_ok = $res_status >= STATUS_PROPOSED && $res_status <= STATUS_COMPLETE && $res_status != STATUS_BLOCKED; 
  } else {
    $status_ok = $invObj->status >= INV_STATUS_DRAFT;
  }

  if($status_ok && $invObj->balance > INV_TOLERANCE && $_GET['act'] != 'pay') {
    $pay_url = payment_link($invObj_or_id, true);
    $pay_link = html_break().html_link($pay_url, phrase('pay_now', CAPITALIZE), ['target' => 'payment']);
  }

  if($invObj->paid > 0 || $invObj->paid < 0) {
    $paid_term = $invObj->paid > 0 ? 'paid' : 'refunded'; 
    $paid_row = array(html_b(phrase($paid_term, CAPITALIZE)));
    $balance_row = array(html_b(phrase('balance', CAPITALIZE)));
    if($show_quantity || $show_tax || $show_discount) {
      $paid_row = array_pad($paid_row, $pad_cols , '');
      $balance_row = array_pad($balance_row, $pad_cols, '');
    }
    $paid_row []= html_b(formatFloat($invObj->paid, FORMAT_ALWAYS, $cur));
    $balance_row []= html_b(formatFloat($invObj->balance, FORMAT_ALWAYS, $cur)).$pay_link;  

  } else if($due_now && $due_now < $invObj->balance) {
    $due_now_text = phrase('due_now', CAPITALIZE).": (".$deposit_percentage.'% '.phrase('of_total').")";
    $due_row = array(html_b($due_now_text));
    $due_row = array_pad($due_row, $pad_cols, '');
    $due_row[]= html_b(formatFloat($due_now, FORMAT_ALWAYS, $cur)).$pay_link;
  } else if($pay_link && $invObj->balance >= 1) { // nothing paid, nothing due, still balance. show pay link in total row    
    $due_row = array_fill(0, $pad_cols, '');
    $due_row[] = $pay_link;
  }

  if($total_row) $rows []= $total_row;

  // show tax already included - after total
  if($invObj->charge_tax) {
    $tax_of_which_ar = get_of_which_tax($invObj, $liObjs);
    foreach($tax_of_which_ar as $tax_rate => $tax_amount) {
      $of_which_tax_row = array(html_b(phrase('Of_which_VAT').' '.$tax_rate.'%'));
      $of_which_tax_row = array_pad($of_which_tax_row, $num_cols , '');
      $of_which_tax_row[] = html_b(formatFloat($tax_amount, FORMAT_ALWAYS, $cur));
      $rows[] = $of_which_tax_row;
    }
  }

  if($paid_row) $rows []= $paid_row;  
  if($balance_row) $rows []= $balance_row;
  else if($due_row) $rows []= $due_row;

  return $rows;
}

function add_city_tax($invObj_or_id) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $inv_id = $invObj->id;
  $res_id = $invObj->res_id;
  if(!$inv_id || !$res_id) return false;
  $resObj = get_object('reservation', $res_id);
  if(!$resObj) return false;
  print_log("running add_city_tax for $inv_id", 'invoice', LOG_LEVEL_TEST);

  $li_type = LI_TYPE_CITY_TAX;
  $li_id = find_lineitem_id($inv_id, $li_type); 
  
  $city_tax_rate = $resObj->city_tax_rate ?: get_city_tax_rate($resObj->city_id);
  $charge_city_tax = $invObj->charge_city_tax && $resObj->charge_city_tax && $city_tax_rate;

  if(!$charge_city_tax) {
    if($li_id) list($lid, $li_errors) = delete_object('inv_lineitem', $li_id);
    return 0;
  }

      
  $tt_tax_types = implode(",", array(LI_TYPE_DEPOSIT,LI_TYPE_BALANCE,LI_TYPE_RENT,LI_TYPE_CHANGE_LENGTH,LI_TYPE_CHANGE_GUESTS,LI_TYPE_CHANGE_PROPERTY,LI_TYPE_CHANGE_RATE,LI_TYPE_CHANGE_DISCOUNT,LI_TYPE_CHANGE_TOTAL));
  $qs = "SELECT SUM(subtotal-tax_of_which) FROM inv_lineitem WHERE inv_id=$inv_id AND type IN ($tt_tax_types)";
  $total = db_getOne($qs);

  $total_discounted = apply_discount($total, $invObj->discount); 
  
  /** next three not needed, as we use SUM(subtotal) above, which already is incl/excl. tax */
  // $tax_rate_hi = 1.21; /** todo: not static */
  // $tax_rate_lo = 1.06; /** todo: not static */
  // $total_discounted_ex_tax = $invObj->charge_tax && $invObj->tax_included ? $total_discounted / $tax_rate_lo : $total_discounted;
  
  $city_tax = apply_surcharge($total_discounted, $city_tax_rate, true);
  $city_tax_desc = "{$city_tax_rate} city tax";
  
  //list($tt_tax_amount, $tt_desc, $tt_quant) = get_city_tax($resObj, apply_discount($total, $discount));

  print_log("city_tax for $inv_id: total=$total total_ex=$total_discounted_ex_tax discount=$discount tax=$tt_tax_amount, desc=$tt_desc quant=$tt_quant", 'invoice', LOG_LEVEL_TEST);
    
  $li = [];
  $li['currency']     = $invObj->currency;
  $li['amount']       = $city_tax;
  $li['description']  = $city_tax_desc;
  $li['quantity']     = 1;

  //dump($li_id);die();
  if($li_id) { // update manually
    $action = "UPDATE";
    list($lid, $li_errors) = update_object('inv_lineitem', $li, $li_id);
  } else { // add - trigger handlers
    $li['inv_id']       = $inv_id;
    $li['discount']     = 0;
    $li['type']         = $li_type;  
    $li['tax_category'] = "N";
    $action = "ADD";
    list($lid, $li_errors) = add_object('inv_lineitem', $li);
  }

  print_log("Ran add_city_tax for $inv_id: action=$action $li_id total=$total $city_tax_desc = $city_tax", "invoice", LOG_LEVEL_TEST);

  return $city_tax;
}


// cancels all invoices (unless payment has been made)
function cancel_reservation_invoices($res_id) {
  db_query("UPDATE invoice SET status=".INV_STATUS_CANCELLED." WHERE status >= ".INV_STATUS_DRAFT." AND status <= ".INV_STATUS_SENT." AND res_id=$res_id");  
}

function delete_reservation_invoices($res_id) {
  global $db;
  if(!$res_id) return;
  if($ids = db_getCol("SELECT id FROM invoice WHERE status <= ".INV_STATUS_SENT." AND res_id=$res_id")) {
    $id_list = implode(',', $ids);
    db_query("DELETE FROM invoice WHERE id IN ($id_list)");
    db_query("DELETE FROM inv_lineitem WHERE inv_id IN ($id_list)");
  }
}

// creates reservation invoice(s) for given res. id
// cancels old invoices no longer needed
function add_reservation_invoices($resObj_or_id, $options=NULL) {
	$i = 1;
  $resObj = any2obj('reservation', $resObj_or_id);
  print_log("add_reservation_invoices: source={$resObj->source_id} fid={$resObj->fid} ", 'invoice', LOG_LEVEL_TEST);
  
  if($resObj->source_id || $resObj->fid) return false; // don't add invoices if it's not our reservation
  
  $res_id = $resObj->res_id;
  $inv_ids = array();
  $dryrun = $options['dryrun']; 
  $debug = $options['debug']; 
  $hourly = $resObj->hourly;

  $debug = false;
  $debug = !$_SESSION['live'];

  $debug = true;

  
	// get date array and add an invoice for each date range (just one, unless monthly)
	$monthly = $resObj->monthly_billing;


  if($invObjs = find_reservation_invoices($res_id, array('search' => array('type' => INV_TYPE_BOOKING, 'status' => array('>=', STATUS_ALL))))) { // shouldn't happen
    foreach($invObjs as $iObj) $invArray[$invObj->id] = $iObj; // store assoc. for retrieval in cancel code below
    $old_inv_ids = array_find_key($invObjs, 'id'); // existing reservation booking invoice objects (ignore others)
    $inv_count = count($invObjs);
  } else {
    $new = true; // no invoices for this reservation, adding invoices for first time
  }

	$dates =  invoice_date_array($resObj, $invObjs);
  print_log("add_reservation_invoices: res_id=$res_id monthly=$monthly dates=".dump($dates, true), 'invoice', LOG_LEVEL_TEST);
	
	if($debug) $log .= html_break(3)."$inv_count existing invoices new=$new in=$resObj->checkin out=$resObj->checkout".dump($dates, true);
  if($debug) $log .= "Running add_reservation_invoices: monthly=$monthly";

  // dump($dates);     
  //dump($invArray);
  //dump($dateArray);
	foreach($dates as $dateArray) {
    $date_change = $tax_change = $locked = 0;
	  if($inv_id = $dateArray['id']) { // existing invoice ?
	    //echo("existing invoice $inv_id<br>");
	    $invObj = get_object('invoice', $inv_id); // load new or updated invoice     
	    $locked = $dateArray['locked'] || invoice_locked($invObj);
      if($locked) {
        $il = invoice_locked($invObj);
        if($debug) print_log("Skipping Locked: il=$il id=$inv_id date: {$dateArray['locked']} Obj: {$invObj->locked}", 'invoice', LOG_LEVEL_TEST);     
        $invObj = NULL;

        //echo("locked invoice $inv_id<br>");
        //echo(html_b("add_reservation_invoices: Attempt to edit locked invoice $inv_id"));
        $i++;
        continue;
      }
      $date_change = date_or_datetime($invObj->start_date, $hourly) != $dateArray['start_date'] || date_or_datetime($invObj->end_date, $hourly) != $dateArray['end_date'] ? 1 : 0; // did the date/times change ?
	  } else {
	    $invObj = NULL;
	  }
	  $first_new_invoice =  $new && $i==1;
    $invData = reservation2invoice($resObj, $dateArray);
    
    if($debug) $log .= "Date array:".dump($dateArray, true);     
    if($debug) $log .= "Inv data:".dump($invData, true);     

    $invData['sequence'] = $monthly ? $i : 0;
    if($first_new_invoice) $invData['head_invoice'] = 1; // the first invoice is the head invoice, set only when first created
    $head_invoice = $invObj->head_invoice || $invData['head_invoice'];
    
    $tax_change = $invObj && ($invData['charge_city_tax'] != $invObj->charge_city_tax || $invData['charge_tax'] != $invObj->charge_tax || $invData['tax_included'] != $invObj->tax_included) ? 1 : 0;
    if($debug) $log .= html_b("Date changed = $date_change Tax changed = $tax_change".PHP_EOL);
    
    // add invoice            
    list($new_inv_id, $inv_errors) = add_or_update_object('invoice', $invData);	  
    if($inv_errors) mydie("add_or_update_invoices: failed to add/update invoice $inv_id".dump($inv_errors, true));
    $inv_ids[]= $new_inv_id;                       

    $old_start = $invObj->start_date;
    $old_end = $invObj->end_date;

    $invObj = get_object('invoice', $new_inv_id);

    $new_start = $invObj->start_date;
    $new_end = $invObj->end_date;
                       
    $date_start = $dateArray['start_date'];
    $date_end = $dateArray['end_date'];
    
    if($debug) $log .= "Datecheck: id=$inv_id: Start: date=$date_start old=$old_start new=$new_start<br>End: date=$date_end old=$old_end new=$new_end";
                                                                                                     
    $liArray = array();
    if($head_invoice && !$locked) {  // add fees and catering to head invoice only
      //echo("addding/updating fees head_invoice=$head_invoice ($invObj->id) locked=$locked first_new = $first_new_invoice new=$new i=$i to $new_inv_id<br>");
      
      $liArray = res2li_fees($new_inv_id, $resObj);
      print_log("add_res_inv fees ".dump($liArray, true), 'invoice', LOG_LEVEL_TEST);
      
      delete_unused_fees($new_inv_id, $liArray);
      //delete_unused_fees($invObj, $fee_array);
      
      //if($resObj->catering_id) $liArray[]= res2li_catering($new_inv_id, $resObj);
    }
    
    if(!$date_change && !$tax_change) { // if date/tax change, handled by post_edit_invoice
      $liArray[] = res2li_rent($new_inv_id, $resObj, $dateArray); // add rent (or package)
      if($debug) $log .= "Rent lineitems for invoice $new_inv_id=".dump($liArray, true);
      
      //dump($liArray);
      $li_errors = add_or_update_invoice_lineitems($new_inv_id, $liArray);
      if($inv_errors) mydie("add_reservation_invoices: failed to add lineitems".dump($li_errors, true));
      $delete_li_types = array(LI_TYPE_DEPOSIT, LI_TYPE_BALANCE);
      if($invObj->charge_city_tax || $invData['charge_city_tax']) add_city_tax($new_inv_id); // add city tax
      else $delete_li_types[] =  LI_TYPE_CITY_TAX; // remove city tax
      delete_lineitem_types($new_inv_id, $delete_li_types); //we don't use these any more
      update_invoice_total($new_inv_id);
    }

    $i++;
	}

  // cancel any invoice left over
  if($new) {
    $log .= "Invoice is new, not cancelling others<br>";
  } else {
    $cancel_ids = $old_inv_ids ? array_diff($old_inv_ids, $inv_ids) : array();  
    $output .= "Old:".implode(',', $old_inv_ids)."<br>";
    $output .= "New:".implode(',', $inv_ids)."<br>";
    $output .= "Diff:".implode(',', $cancel_ids)."<br>";
    foreach($cancel_ids as $iid) {
      if(invoice_locked($iid)) continue;
      if($invArray[$iid]->status != INV_STATUS_CANCELLED) {
        list($id, $errors) = update_object('invoice', array('status' => INV_STATUS_CANCELLED), $iid);
        if($errors) die("add_or_update_invoices: cancel invoice failed: ".dump($errors, true));
        $log .= "Cancelled invoice $iid (not locked) <br>";
      }
    }
  }
  if($debug) print_log(str_replace("<br>", "\n", $log), 'invoice', LOG_LEVEL_TEST);
	return $inv_ids;
}


function delete_unused_fees($inv_id, $liArray) {
  $invObj = get_object('invoice', $inv_id);
  $property_type = $invObj->property_type;
  if(!$property_type) return;
  $fee_types = lineitem_fee_types($property_type);
  $all_fee_types = array();
  $inv_fee_types = array();

  foreach($fee_types as $fee_type) $all_fee_types[] = $fee_type;
  foreach($liArray as $li) $inv_fee_types[] = $li['type'];
  $del_fee_types = array_diff($all_fee_types, $inv_fee_types);
  
  if($del_fee_types) {
    $qs = "DELETE FROM inv_lineitem WHERE inv_id=$inv_id AND type IN (".implode(',', $del_fee_types).")";
    db_query($qs);
  }
}

function res2li_catering($invObj_or_id, $resObj) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $inv_id = $invObj->id;
     
  $productObj = get_object('product', $catering_id);
  $li_type = LI_TYPE_CATERING;
  $li_amount = $productObj->price;
  $li_quant = uom_pphr_quantity($productObj->unit, $resObj->num_guests, $resObj->num_hours);
  $li_desc   = product_print($productObj);      
  
  $li = array();
  $li['id']           = find_lineitem_id($inv_id, $li_type);
  $li['inv_id']       = $inv_id;
  $li['product_id']   = $catering_id;
  $li['block_id']     = $resObj->block_id;
  $li['quantity']     = $li_quant;
  $li['currency']     = $currency;
  $li['amount']       = $li_amount;
  //$li['discount']     = $discount;
  $li['description']  = $li_desc;
  $li['type']         = $li_type;

  return $li;
}


// returns lineitem data for the rent
function res2li_rent($invObj_or_id, $resObj, $dateArray) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $inv_id = $invObj->id;
  $hourly = $resObj->hourly;
  $corporate = $resObj->corporate;
  $discount = $resObj->discount;
  $num_guests = $resObj->num_guests;
  $ptype = $resObj->property_type;
  $apt_id = $resObj->apt_id;
  list($abs_discount, $perc_discount) = split_discount($discount);

  $currency = $resObj->currency;
  $rate_type = $resObj->rate_type;
  $rate_base = $resObj->rate_base;
    
  if(!$dateArray) die("res2li_rent: missing date array");
  $in = date_or_datetime($dateArray['start_date'], $hourly);
  $out = date_or_datetime($dateArray['end_date'], $hourly); 

  /** Let reservation handle its own rate
  if(!$resObj->rate_locked && $apt_id && $ptype) {
    $aptObj = get_object($ptype, $apt_id);
    $priceObj = get_average_rate($aptObj, $in, $out, $num_guests);
    $rate_base = $priceObj->average;    
    if($_SESSION['dev']) print_log("res2li_rent: priceObj=".dump($priceObj, true), 'invoice', LOG_LEVEL_DEBUG);
  } else {
    $rate_base = $resObj->rate_base;
  }    
  */
  
  
  $num_days = $dateArray['num_days'] ?: period_diff($in, $out, 'd');
  $days_in_month = $dateArray['days_in_month'] ?: get_daysofmonth($in);
  $total_days = $resObj->num_days;

  if($_SESSION['dev']) print_log("res2li_rent: date in months ($in) = $days_in_month ratelock=$resObj->rate_locked apt_id=$apt_id ptype=$ptype dateArray=".dump($dateArray, true)." in=$in out=$out nd=$num_days total=$total_days", 'invoice', LOG_LEVEL_DEBUG);
  /* 
  // use date array instead of reservation
  {  
  if($dateArray) { 
    $searchObj->in   = $in = date_or_datetime($dateArray['start_date'], $hourly);
    $searchObj->out  = $out = date_or_datetime($dateArray['end_date'], $hourly); 
    $searchObj = search2in_out($searchObj); // calculates num_days, num_months, num_hours
    
    $resObj->checkin    = $searchObj->in;
    $resObj->checkout   = $searchObj->out;
    $resObj->num_months = $searchObj->nm;
    $resObj->num_days   = $searchObj->nd;
    $resObj->num_hours  = $searchObj->nh;
    //dump($dateArray);
    //dump($searchObj);
  }
  */
    
  //dump($dateArray);
  //dump($invObj);
	$rate_text = formatFloat($rate_base, FORMAT_IF_NEEDED)." ".$currency;
  if($abs_discount) { // apply discount now
    $discount_rate = apply_discount($rate_base, $discount);
	  //$rate_text = "($rate_text)=> ".number_format($discount_rate, 2)." ".$currency;
	  $rate_text = number_format($discount_rate, 2)." ".$currency;
	  $rate_base = $discount_rate;
	  $discount = '';
  } elseif($perc_discount) {
  	$rate_text .= " - $discount";
  } else {
    $discount = '';
  }

  if($resObj->monthly_billing) { // monthly pricing
    $li_type = LI_TYPE_RENT;
    //echo("nd=$num_days dim=$days_in_month td=$total_days");
    //dump($dateArray);
    
    //if(!$num_days) $num_days = $days_in_month; 
    $fraction = $num_days/$days_in_month;    
  	if($rate_type == 'month') {
  	  //echo("nd=$num_days dim=$day_in_month fraction=$fraction<br>");
   	  $li_quant = $fraction;
      $li_desc = $fraction == 1 ? "One month: $rate_text" : 
                                  "Fraction ($num_days / $days_in_month) of month: $rate_text";
                                  
    } elseif($rate_type == 'day') {
   	  $li_quant = $num_days;
      $li_desc = "$num_days days at $rate_text";
    } elseif($rate_type == 'total') {
      $days_total = $resObj->num_days;
   	  $li_quant = $num_days / $days_total;
      $li_desc = "Fraction ($num_days / $days_total ) of fixed rate of $rate_text";
    } elseif($rate_type == 'special') {
      $days_total = $resObj->num_days;
   	  $li_quant = $num_days / $days_total;
      $li_desc = "Fraction ($num_days / $days_total ) of fixed special rate of $rate_text";
    } elseif($rate_type == 'hour') {
      die("create_reservation_invoice: hourly rate not supported for reservation > 1 month");
    }   			

  } else {  // daily/hourly/package pricing

    $unit = $rate_type;
    switch($rate_type) {
      case 'total':                                                                
        $li_quant = 1;
        $rate_desc = "Fixed rate of $rate_text";
        break;
      case 'special':                                                                
        $li_quant = 1;
        $rate_desc = "Special offer: $rate_text";
        break;
      case 'month':
        $li_quant = $resObj->num_months;
        break;
      case 'day':
        $li_quant = $num_days;
        //if($resObj->start_hour && $resObj->end_hour) $unit = 'shift'; // todo: let shift be it's own type ?
        break;
      case 'hour':
        $li_quant = $resObj->num_hours;
        break;
      case 'person':
        $li_quant = $resObj->num_guests;
        break;
      case 'pp_hour':
        $li_quant = $resObj->num_hours * $resObj->num_guests;
        break;
      case 'ticket':
        $li_quant = $resObj->num_guests;
        break;
      default:
        mydie("res2li_rent: missing or unknown rate_type '$rate_type'");
        break;        
    }
    $q_str = formatFloat($li_quant, FORMAT_IF_NEEDED);
    if(!$rate_desc) $rate_desc = $q_str.' '.phrase($unit.($li_quant == 1? '': 's')).' @ '.$rate_text;
    $li_desc   = $rate_desc;      
  }
  
  $li_type = LI_TYPE_RENT;
  if($package_id = $resObj->package_id) {
    $productObj = get_object('product', $package_id);
    $li_type = LI_TYPE_PACKAGE;
    $li_desc   = product_print($productObj);      
  }

  $liData['id']           = find_lineitem_id($inv_id, $li_type);
  $liData['product_id']   = $package_id;
  $liData['inv_id']       = $inv_id;
  $liData['block_id']     = $resObj->block_id;
  $liData['quantity']     = $li_quant;
  $liData['currency']     = $currency;
  $liData['amount']       = $rate_base;
  $liData['discount']     = $discount;
  $liData['description']  = $li_desc;
  $liData['type']         = $li_type;
  if($package_id) {
    $liData['tax_category'] = $tax_cat = $productObj->tax_category;
    $start_date = get_object('invoice', $inv_id, 'start_date'); // todo: remove (BTW October 1 2012) 
    $liData['tax_percentage'] = get_tax_rate($tax_cat, $start_date);
  }
  if($_SESSION['dev']) print_log("res2li_rent: amount=$rate_base line=".dump($liData, true), 'invoice', LOG_LEVEL_DEBUG);
  
  return $liData;
}
  
function find_lineitem_id($inv_id, $li_type) {
  return find_object('inv_lineitem', array('inv_id' => $inv_id, 'type' => $li_type), 'id');
}

function delete_lineitem_types($inv_id, $li_types) {
  if($li_ids = find_objects('inv_lineitem', array('inv_id' => $inv_id, 'type' => $li_types), array('fields' => 'id'))) {
    $id_list = implode(',', $li_ids);
    db_query("DELETE FROM inv_lineitem WHERE id in ($id_list)");
  }
}
  
// add or update payment order from cart data
/** unsused - automatic comment added on  */
function cart2order($cartObj_or_id, $data=NULL, $options=NULL) {
  $payment_type = $data['payment_type'];
  $errors = array();
  $cartObj = any2obj('cart', $cartObj_or_id);
  $cart_id = $cartObj->id;
  $order_id = $cartObj->order_id ?: $data['order_id'];
  if(!$cartObj) return array(0, array("cart2order: missing cart."));

  if($cartObj->source_id || $cartObj->fid) return array(0, array("cart2order: not our cart")); // don't add invoice if it's not our cart

  $itemObjs = $cartObj->items;
  if(!$itemObjs && $cart_id) $itemObjs = find_objects('cart_row', array('cart_id' => $cart_id));
  if(!$itemObjs) return array(0, array("cart2order: missing lines.")); // don't add invoice if empty

  // $orderSearch['cart_id'] = $cart_id; // look for existing (not for cart)
  // if($order_id = find_object('order', $orderSearch, 'id')) return array($order_id, $errors);
  
  $orderData = (array) $cartObj;
  if($data) $orderData = array_merge($orderData, $data);
  
  $orderData['cart_id'] = $cart_id;
  if($orderData['payment_type'] = $payment_type) $orderData['gateway_id'] = payment_type2gateway_id($payment_type);

  //$copyAr = array('site_id', 'client_id', 'legal_entity_id', 'user_id');
  //foreach($copyAr as $key) $orderData[$key] = $cartObj->$key;

  $order_product_ids = array();
  if($order_id) { // existing
    $order_product_ids = find_objects('order_row', array('order_id' => $order_id), array('fields' => 'product_id'));
  } else {
    $orderData['status'] = ORDER_STATUS_OPEN; // new order
    //$orderData['uuid']       = $uuid;
    $orderData['inv_id']     = $inv_id;
    $orderData['user_id']     = $cartObj->user_id; // should have been copied above
    if($gateway_id) $orderData['gateway_id'] = $gateway_id;
    $orderData['currency'] = $orderData['invoice_currency']  = $cartObj->currency;
    $orderData['total'] = $orderData['balance'] = $orderData['invoice_amount'] =  $cartObj->total;
    $orderData['paid'] = 0;
    $orderData['parent_type'] = 'cart';
    $orderData['parent_id'] = $cart_id;
    $orderData['description'] = "Cart $cart_id";
    list($order_id, $errors) = add_object('payment_order', $orderData);
    if($errors || !$order_id) return array ($order_id, $errors);
  }

  $errors = array(); // shouldn't be needed

  // add/update lines
  $cart_product_ids = array();
  foreach($itemObjs as $itemObj) {    
    list($cart_item_id, $item_errors) = cart2order_item($order_id, $itemObj);
    if($item_errors) $errors = array_merge($errors, $item_errors);
    $cart_product_ids[] = $itemObj->product_id;
  }
  // delete lines on order not in cart
  $delete_product_ids = array_diff($order_product_ids, $cart_product_ids);
  //echo("Cart products: ".implode(',', $product_ids).html_break());
  //echo("Order products: ".implode(',', $order_product_ids).html_break());
  //echo("Delete products: ".implode(',', $delete_product_ids).html_break());
  if(count($delete_product_ids)) db_query("DELETE FROM order_row WHERE order_id=$order_id AND product_id in (".implode(',', $delete_product_ids).")");
  update_order_total($order_id);
  return array ($order_id, $errors); 
}


// assume cart is valid (no duplicate product ids)
function cart2order_item($order_id, $itemObj, $options=NULL) {
  if(!$order_id) return array(0, array("cart2order_item: Missing order_id"));
  if(!$itemObj) return array(0, array("cart2order_item: Blank cart row"));
  
  $orderItemData = (array) $itemObj;
  $orderItemData['description'] = $itemObj->name;
  $orderItemData['order_id'] = $order_id;
  $product_id = $orderItemData['product_id'];
  $orderRowId = find_object('order_row', array('order_id' => $order_id, 'product_id' => $product_id), 'id');
  //  echo("row-id=$orderRowId oid=$order_id pid=$product_id".dump($orderItemData, true));
  if($orderRowId) {
    list($row_id, $errors) =  update_object('order_row', $orderItemData, $orderRowId);
  } else {
    list($row_id, $errors) =  add_object('order_row', $orderItemData);
  }
  //echo("add order_row returned id=$row_id errors=".dump($errors,true));die();
  
  return array($row_id, $errors);
}

function order2invoice($orderObj_or_id, $options=NULL) {
  global $__SITE;
  $errors = array();
  $orderObj = any2obj('payment_order', $orderObj_or_id);
  $siteObj = $site_id = $orderObj->site_id ? get_object('site', $site_id) : $__SITE;
  $order_id = $orderObj->id;
  if(!$order_id) return array(0, array("order2invoice: missing order id."));
                     
  $itemObjs = $orderObj->items ?: find_objects('order_row', array('order_id' => $order_id));

  if($orderObj->source_id || $orderObj->fid) return  array(0, array("order2invoice: not out order.")); // don't add invoice if it's not our order
  //if(!$orderObj->total) return array(0, array("order2invoice: missing total.")); // don't add invoice if empty
  if(!$itemObjs) return array(0, array("order2invoice: missing lines.")); // don't add invoice if empty

  $invSearch['order_id'] = $order_id; // look for existing open invoice
  $invSearch['status'] = INV_STATUS_OPEN; // look for existing open invoice
  if($inv_id = find_object('invoice', $invSearch, 'invoice.id')) return array($inv_id, $errors);
  
  $invData = array();
  $invData['order_id'] = $order_id;
  $invData['parent_type'] = 'payment_order';
  $invData['parent_id'] = $invData['order_id'] = $order_id;
  $invData['currency'] = $orderObj->invoice_currency ?: $orderObj->currency;
  $invData['legal_entity_id'] = $orderObj->legal_entity_id ?: $siteObj->legal_entity_id;
  list($inv_id, $errors) = add_object('invoice', $invData);
  if($errors || !$inv_id) return array ($inv_id, $errors);
  $errors = array(); // should be so from add_object
  // copy lines
  foreach($itemObjs as $itemObj) {    
    list($cartItem_id, $item_errors) = order2invoice_item($inv_id, $itemObj);
    if($item_errors) $errors = array_merge($errors, $item_errors);
  }
  return array($inv_id, $errors);
}

// assume cart is valid (no duplicate product ids)
function order2invoice_item($inv_id, $itemObj, $options=NULL) {
  if(!$inv_id) return array(0, array("order2invoice_item: Missing invoice_id"));
  if(!$itemObj) return array(0, array("order2invoice_item: Blank order row"));
  if(!$itemObj->product_id) return array(0, array("order2invoice_item: No product ID on item"));
  
  $liData = (array) $itemObj;
  unset($liData['id']);
  
  $liData['type'] = LI_TYPE_PRODUCT;
  $liData['inv_id'] = $inv_id;
  if(!$liData['tax_category']) $liData['tax_category'] = "N"; // default for now; todo: copy from product to cart row to order row
  list($li_id, $errors) =  add_object('inv_lineitem', $liData); 
  return array($li_id, $errors);
}
                       
// returns array with invoice data used to create or update 
// invoice based on reservation data
//function reservation2invoice($resObj_or_id, $invoice_type=INV_TYPE_CORRECTION, $billing_in_date='', $billing_out_date='', $invObj=0) {
function reservation2invoice($resObj_or_id, $invData) {
  $resObj = any2obj('reservation', $resObj_or_id);
  if(!$resObj) mydie("reservation2invoice: no reservation received");
  
  $res_id = $resObj->res_id;  
  $hourly = $resObj->hourly;
  
  $in  = date_or_datetime($resObj->checkin, $hourly); 
  $out = date_or_datetime($resObj->checkout, $hourly);

  $inv_id = $invData['inv_id'] ?: 0;
  if($finv_id) $invObj = get_object('invoice', $inv_id);
  $copy_fields = array('client_id', 'legal_entity_id', 'res_id', 'block_id', 'agent_id', 'apt_id', 'org_id', 'contact_id', 'bill_to', 'host_id', 'city_id', 'site_id', 'property_type', 'charge_city_tax', 'charge_tax', 'tax_included', 'currency', 'corporate');
  foreach($copy_fields as $field) if(!isset($invData[$field])) $invData[$field] = $resObj->$field;	
  $invData['start_date'] = $start = $invData['start_date'] ?: $in;
  $invData['end_date']   = $end = $invData['end_date'] ?: $out;
  $invData['type']       = $invData['type'] ?: INV_TYPE_BOOKING;
  if(!$inv_id) $invData['status'] = $invData['status'] ?: INV_STATUS_OPEN;
  
  print_log("\n\nid=$inv_id start=$start end=$end".dump($invData, true), 'invoice', LOG_LEVEL_TEST);
  //if($city_id = $invData['city_id']) $invData['country_id'] = get_object('city', $city_id, 'grandparent_id');

  // figure out "concerning" invoice text
  if(!$invData['concerning']) {
    // number of guests
    $num_guests = $resObj->num_guests;
    $ng_string = $num_guests <= 1 ? '' : " ($num_guests guests)";
  
    // which apartment
    $guest_name = get_name_string($resObj);
    if($apt_id = $resObj->apt_id) {
      $aptObj   = get_object('property', $resObj->apt_id);
      $apt_addr = $aptObj->address_street.' '.$aptObj->address_number;
      $apt_name = $resObj->apartment_name;
    }
    if($resObj->client_id == 1001 || !$apt_name) $apt_name_str = " @ $apt_addr"; // for Sicco, show address 
    else if($apt_name) $apt_name_str = " @ $apt_name ";
  
    // concerning...
    $concerning .= $guest_name.$ng_string.$apt_name_str;
    $invData['concerning'] = $concerning;
  }

  $invData['parent_type'] = 'reservation';
  $invData['parent_id'] = $invData['order_id'] = $res_id;
  
  $invData['customer_notes'] = $resObj->invoice_notes;

  
  // figure out address: for new invoices only, preserve address of existing invoices
  $addObj = null;
	if(!$inv_id) { 
    if($org_id = $resObj->org_id) { // corporate invoice
      $contact_id = $resObj->contact_id;
      if($contact_id && $resObj->invoice_contact) {
        $cc_id = $resObj->cc_id; // or add cc_id to contact ?
        $addObj = get_object('contact', $contact_id);
      } else {
        $addObj = get_object('org', $org_id);
      }
  
    } elseif($guest_id = $resObj->guest_id)  { // registered guest
      $addObj = get_user($guest_id);      
    } else { // personal invoice - address info from credit card (or reservation)
      $addObj = $resObj;            
    }

    if($addObj) {          
      if($addData = copy_address($addObj)) {
        foreach($addData as $k => $v) $invData[$k] = $v;
      }
    }
  }

  return $invData;
}

// returns an array of array('start_date' => $in, 'end_date' => $out, id=>$existing_invoice_id) based on checkin/out of reservation 
// if there are existing locked invoices, the code will work around those, and included the locked ranges with and id
// for hourly reservations, result has just one element including time (unless there are existing locked invoices);
// for daily reservations, result has just one element unless monthly (or existing locked invoices)
function invoice_date_array($resObj_or_id, $invObjs) {
  $resObj = any2obj('reservation', $resObj_or_id);
  $res_id = $resObj->res_id;
  $hourly = $resObj->hourly;
  $monthly = $resObj->monthly_billing; 
  
  $inv_count = count($invObjs);

  // figure out missing dates (for daily, monthly, not hourly)
  $dryrun = $options['dryrun']; 
  $debug = $options['debug']; 
  $lockAr = array();
  $updateAr = array();
  $dateAr = array();
  $added = array();
  //echo("add_or_update_invoices: dryrun=$dryrun debug=$debug<br>");
  
  $in  = date_or_datetime($resObj->checkin, $hourly); 
  $out = date_or_datetime($resObj->checkout, $hourly);
  $nd = period_diff($in, $out, 'd');
  if($inv_count > 0) { // there are existing booking invoices, figure out which are locked and which can be overwritten
    foreach($invObjs as $iObj) {        
      if($locked = $iObj->locked = invoice_locked($iObj)) {
        $lockAr[] = array('start' => date_or_datetime($iObj->start_date, $hourly), 'end' => date_or_datetime($iObj->end_date, $hourly), 'id' => $iObj->id, 'locked' => 1); // don't mess with these
      } else {
        $updateAr[] = $iObj->id; // array('id' => $iObj->id, 'start' => strip_time($iObj->start_date), 'end' => strip_time($iObj->end_date)); // don't mess with these $iObj; // these can be overwritten
      }
    }
  }
  //dump($lockAr);
  if($count = count($lockAr)) {
    print_log("inv_date_array: $count locked, montly in=$in out=$out", 'invoice', LOG_LEVEL_TEST);
    
    $last = $count - 1;
    
    // add invoice(s) before the first locked invoice
    if($in < $lockAr[0]['start']) {
      if($monthly) { 
        $dateAr = monthly_date_array($in, $lockAr[0]['start']);
      } else {
        $dateAr[] = array('start_date' => $in, 'end_date' => $lockAr[0]['start']);
      }
      $dateAr[] = $lockAr[0];
      $added[0] = true;
    }

    // add invoice(s) between locked invoices if 2 or more are locked
    for($i=0;$i < $last;$i++) {
      $prevAr = $lockAr[$i];
      $nextAr = $lockAr[$i+1];
      if($prevAr['end'] < $nextAr['start']) { // gap between locked invoices 
        if(!$added[$i]) {
          $dateAr[] = $prevAr;
          $added[$i] = true;
        }
        if($monthly) { 
          $dateAr = array_merge($dateAr, monthly_date_array($prevAr['end'], $nextAr['start']));
        } else {
          $dateAr[] = array('start_date' => $prevAr['end'], 'end_date' => $nextAr['start']);
        }          
      }
    }

    // add invoice(s) after lasted locked invoice
    if($out > $lockAr[$last]['end']) {
      if(!$added[$last]) $dateAr[] = $lockAr[$last];
      if($monthly) { 
        $dateAr = array_merge($dateAr, monthly_date_array($lockAr[$last]['end'], $out));
      } else {
        $dateAr[] = array('start_date' => $lockAr[$last]['end'], 'end_date' => $out);
      }
    }

  } else { // none are locked, just add or overwrite existing
    if($monthly) { 
      $dateAr = monthly_date_array($in, $out);
      print_log("inv_date_array: none locked, montly in=$in out=$out dates=".dump($dateAr,true), 'invoice', LOG_LEVEL_TEST);
    } else {
      $dateAr[] = array('start_date' => $in, 'end_date' => $out);
    }
  }
  
  if($debug) dump($dateAr);
  
  foreach($dateAr as &$iAr) {
    if($iAr['locked']) continue; // don't overwrite locked invoices 
    $iAr['id'] = count($updateAr) ? array_shift($updateAr) : 0; // add existing ID causing overwrite
  }
  return $dateAr;
}

// returns true if an invoice cannot be changed (locked)
// will update invoice and set locked=1 if locked and not set
function invoice_locked($invObj_or_id) {
  $locked = 0;
  $invObj = any2obj('invoice', $invObj_or_id);
  if($invObj->locked) return 1;
  
  $locked = $invObj->status == INV_STATUS_COMPLETE || ($invObj->total > INV_TOLERANCE && $invObj->balance < INV_TOLERANCE);
  if($locked) update_object('invoice', array('locked' => 1), $invObj->id);
  return $locked;
}

// returns true if all invoices are locked
/** unsused - automatic comment added on  */
function invoices_locked($invObjs) {
  foreach($invObjs as $invObj) {
    if(!invoice_locked($invObj)) return false;
  }
  return true;
}

function add_or_update_invoice_lineitems($inv_id, $li_array) {
  if(!$li_array) return;
  foreach($li_array as $li) {  
    $li['inv_id'] = $inv_id; // set parent
    // $li['block_id'] = $inv_block_id; // set block id 
    if($id=$li['id']) {
      //echo("updating li $id with");dump($li);
      
      list($li_id, $li_errors) = update_object('inv_lineitem', $li, $id);
    } else {
      
      $start_date = get_object('invoice', $inv_id, 'start_date'); // todo: remove (BTW October 1 2012) 
      
      // get the tax rate for this lineitem when added
      $li['tax_category']   = $tax_cat = $li['tax_category'] ?: get_tax_category($li);
      $li['tax_percentage'] = get_tax_rate($tax_cat, $start_date);
      
      list($li_id, $li_errors) = add_object('inv_lineitem', $li);
      // if($li_errors) dump($li_errors);
      // else echo("Added lineitem $li_id");
    }
    //if($li_errors) return $li_errors;
    if(count($li_errors)) {
      json_error($li_errors);
      $invObj=get_object('invoice', $inv_id);
      dump($li_errors);dump($li);dump($invObj);
      die("add_or_update_invoice_lineitems: add_invoice_lineitems error");
    }
  }
}


// takes a draft invoice - copiues to invoice, deletes draft
function finalize_invoice($invObj) {
  $inv_id = $invObj->id;
  if($invObj->status != INV_STATUS_DRAFT) return array($inv_id, array("Only draft invoices can be finalized."));
  if(!$invObj->currency) return array($inv_id, array("Invoice $inv_id has no currency."));
  //if($invObj->total < INV_TOLERANCE) return array($inv_id, array('Invoice $inv_id has no total (less than 1 $invObj->currency).'));
  //if($invObj->balance < INV_TOLERANCE) return array($inv_id, array('Invoice $inv_id has no balance (less than 1 $invObj->currency).'));
  
  $inv_data['status'] = INV_STATUS_OPEN;
	$inv_data['duedate'] = sql_add_days(today(), 30); // due in 30 days unless otherwise set		

	//$inv_data['sent_date'] = now();
	$inv_data['invoice_date'] = now(); 		
  // move pdf to sent folder

  $src = get_pdf_path($invObj, true);
  $dst = get_pdf_path($invObj, false);

  if(file_exists($src) && !mv($src, $dst)) die("Move invoice failed: couldn't mv '$src' to '$dst'");
  return update_object('invoice', $inv_data, $inv_id);

}

function copy_invoice($invObj_or_id, $options = array()) {
  $invObj = any2object('invoice', $invObj_or_id);
  if(!$invObj) return json_error_object("Missing invoice");
  
  $inv_data = (array) $invObj;
  $inv_data['locked'] = 0;
  $clear_fields = array('invoice_id', 'invoice_date', 'total', 'paid', 'balance', 'paid_date', 'payment_date', 'tax', 'tax_of_which', 'export_timestamp', 'status_change_timestamp', 'sent_to', 'sent_date');
  foreach($clear_fields as $fld) unset($inv_data[$fld]);
    
  $inv_data['type'] = INV_TYPE_SUPPLEMENTAL;
  $inv_data['status'] = INV_STATUS_OPEN;
  list($new_inv_id, $inv_errors) = add_object('invoice', $inv_data);
  if($inv_errors) return json_error_object($inv_errors);

  $link = invoice_link($new_inv_id);
  $inv_link .= html_link($link, $new_inv_id);
  $message = "Created new invoice $inv_link";

  $response = new stdClass;
  $response->success = true;
  $response->message = $message;
  $response->link = $link;
  $response->id = $new_inv_id;
  
  if(!$options['copy_lineitem']) return $response;
  if($liObjs = get_lineitems($invObj->id)) {
    foreach($liObjs as $liObj) {
      $liData = (array) $liObj;
      $liData['inv_id'] = $new_inv_id;
      list($li_id, $li_errors) = add_object('inv_lineitem', $liData);
      if($li_errors) return json_error_object($inv_errors);
    }
  }
  return $response; 
}
  

/** returns number of percent given a tax category */
function get_tax_rate($tax_category, $date=NULL) {
  $tax_category = strtoupper($tax_category);
  if(!$tax_category) return 0;
  switch($tax_category) {
    case 'H':
      return 21;
    case 'L':
      return 6;
    case 'N':
      return 0;
    default:
      break;
  }
  
  $catObj = get_object('tax_category', $tax_category);    
  return $catObj ? $catObj->percentage : 0;
}

/** deal with other countries later, when relevant */
function get_tax_category($liObj) {
  if(is_array($liObj)) $liObj = array2obj($liObj);
  return tax_category($liObj->type);
  
}

function tax_category($li_type) {
  return get_object('lineitem_type', $li_type, 'tax_category');
}

function get_city_tax_rate($city_id) {  
  return get_object('geo_city', $city_id, 'city_tax');
}


function lineitem_subtotal($liObj) {
  if(is_array($liObj)) $liObj = array2obj($liObj);
  return round(apply_discount($liObj->quantity * $liObj->amount, $liObj->discount), 2);
}

// find fees for apt.
function res2li_fees($invObj_or_id, $resObj) {
  $invObj = any2obj('invoice', $invObj_or_id);
  $inv_id = $invObj->id;
  $res_fees = booking_fees($resObj) ?: [];
  
	/** add service fee */
	if($resObj->fee_service > 0) {
	  $res_fees[] = array2obj(['quantity' => 1, 'currency' => $invObj->currency, 'amount' => $resObj->fee_service, 'desciption' => 'Service Fee', 'type' => LI_TYPE_SERVICE_FEE]);
	}

  if(!$res_fees) return array();
  $fee_array = [];

	
  
  foreach($res_fees as $feeObj) {
    $li = [];    
    $li['inv_id']       = $inv_id;
    $li['quantity']     = $q = $feeObj->quantity;
    $li['currency']     = $invObj->currency;
    $li['amount']       = $amount = $feeObj->amount;
    $li['type']         = $li_type = $feeObj->type;
    $li['description']  = $feeObj->description ?: phrase(get_object('lineitem_type', $li_type, 'name'), CAPS);
    $li['id']           = find_lineitem_id($inv_id, $li_type);
    $li['tax_category'] = $feeObj->tax;
    $fee_array[] = $li;
  }

  return $fee_array;
}

// sets sequential invoice ID unique for this legal entity
function set_invoice_id($id, $legalObj_or_id) {
  $legalObj = any2obj('legal_entity', $legalObj_or_id);  
  $legal_entity_id = $legalObj->id;
  if($prefix = $legalObj->invoice_prefix) {
    $next_id = $legalObj->invoice_nextid;
    $invoice_id = $prefix.$next_id;
    update_object('legal_entity', ['invoice_nextid' => $next_id + 1], $legal_entity_id);
  } else {
    $custom_id = 'A'.$id;
  }
  $data['invoice_id'] = $invoice_id;      
  list($id, $errors) = update_object('invoice', $data, $id);      
  if($errors) dump($errors);
  return $invoice_id;
}

function reeleezee_export_invoice($id, $send=true, $echo=false) {
  include_once('xml_reeleezee.inc');
  
  if(!$id) return json_error_object("reeleezee_export_invoice: Missing invoice ID");
  $invObj = get_object('invoice', $id);
  $legal_entity_id = $invObj->legal_entity_id;
  if(!$legal_entity_id) return json_error_object("reeleezee_export_invoice: Missing legal ID");
  $legalObj = get_object('legal_entity', $legal_entity_id);
  $re_user = $legalObj->reeleezee_username;
  $re_pass = $legalObj->reeleezee_password;
  if(!$legalObj)  return json_error_object("Reeleezee export: Legal entity not found for id=$legal_entity_id");
  if(!$re_user)   return json_error_object("Reeleezee export: No reeleezee username for $legalObj->name");
  if(!$re_pass)   return json_error_object("Reeleezee export: No reeleezee password for $legalObj->name");
  if(!$legalObj->client_id)  return json_error_object("Reeleezee export: No client ID for $legalObj->name");
    
  if($org_id = $invObj->org_id) {
    $orgObj = get_object('org', $org_id);
    if(1 || !$orgObj->export_timestamp) { // export always for now          
      $result =  reeleezee_export_org($org_id, $legalObj, true, false);
      if($result->error) return $result;
    }
  }
  if($legalObj->invoice_id_required && !$invObj->invoice_id) set_invoice_id($id, $legalObj); // set sequential ID
  $result = reeleezee_export($legal_entity_id, 'invoice', array('invoice.id' => $id), false, 0, $send, $echo); //, $check_validity=0, $limit=0, $send=true, $echo=false)
  if($result->error) return $result;
  if(!$result->xml) return json_error_object("reeleezee_export_invoice: No xml".dump($result,true));
  $xml = $result->xml;
  
  $errors = $message = [];
  if($result->error) return $result;
  
  if($result->message) $messages = array_merge_smart($messages, $result->message);
  
  if($result->success) {
    $error = '';
    if($iso_datetime = $xml->ImportResult->ExportInfo->CreateDateTime) {
      $sql_datetime = date('Y-m-d H:i:s', strtotime($iso_datetime));
      //$result->ProcessedDateTime = $sql_datetime;
      $data = array('locked' => 1, 'export_timestamp' => $sql_datetime, 'export_gateway' => 'Reeleezee');
      if(!$invObj->invoice_date) $data['invoice_date'] = today();          
      list($iid, $errors) = update_object('invoice', $data, $id);
    }
    
    $ts = $xml->ProcessedDateTime;
    
    $messages[] = "Reeleeze export success: ";
    $action = $count = '';
          
    if($count = $xml->ImportResult->SalesInvoice->TotalCreated) {
      $messages[] = "Created $count invoice(s)";
    }
    
    if($count = $xml->ImportResult->SalesInvoice->TotalUpdated) {
      $messages[] = "Updated $count invoice(s)";
    }
    
    $export_result = (array) $xml->ImportResult->ExportInfo;
    if(is_array($export_result) && count($export_result)) {
      foreach($export_result as $k => $v) {
        $messages[] = "$k: $v";
      }
    }
    
  } else {    
    $result->error = "Unknown Reeleeze export error";        
  }
  
  $result->message = $messages;
  return $result;
}

function reeleezee_export_org($id, $legalObj, $send=true, $echo=false) {
  include_once('xml_reeleezee.inc');
  if(!$id) die("reeleezee_export_org: missing ID");

  $re_user = $legalObj->reeleezee_username;
  $re_pass = $legalObj->reeleezee_password;
  if(!$legalObj)  return json_error_object("Reeleezee export org: Legal entity not found for id=$legal_entity_id");
  if(!$re_user)   return json_error_object("Reeleezee export org: No reeleezee username for $legalObj->name");
  if(!$re_pass)   return json_error_object("Reeleezee export org: No reeleezee password for $legalObj->name");

  if(!$legalObj->client_id)  return json_error_object("Reeleezee export org: No client ID for $legalObj->name");
  $legal_entity_id = $legalObj->id;

  
  $orgObj = get_object('org', $id);
  if(!$orgObj)  return json_error_object("Reeleezee export org: Org {$id} not found");
    
  $result = reeleezee_export($legal_entity_id, 'org', array('org.id' => $id), false, 0, $send, $echo); //, $check_validity=0, $limit=0, $send=true, $echo=false)
  if(!$result->success || $result->error) return $result;
  $xml = $result->xml;
  if(!$xml || !$xml->ImportResult) return json_error_object("reeleezee_export_org: missing XML");
  $iso_datetime = $xml->ImportResult->ExportInfo->CreateDateTime;
  $sql_datetime = date('Y-m-d H:i:s', strtotime($iso_datetime));;
  list($iid, $ierrors) = update_object('org', array('export_timestamp' => $sql_datetime, 'export_gateway' => 'Reeleezee'), $id);  
  if($ierrors) $result->error = $ierrors;
  print_log("export_org: xml_result: iso:{$iso_datetime} sql:{$sql_datetime}".dump($xml,true), 'reeleezee', LOG_LEVEL_TEST);        

  return $result;
}

/** unsused - automatic comment added on  */
function add_agent_invoices($options=NULL) {
  
  if($apt_id = $options['apt_id']) $search['apt_id']  = $apt_id;
  if($host_id = $options['host_id']) $search['host_id']  = $host_id;
  if($client_id = $options['client_id'] ?: $_SESSION['client_id']) $search['client_id'] = $client_id;

  $search['agent_id'] = array('>', 0);
  $search['status'] = array(STATUS_RESERVED, STATUS_COMPLETE);
  
  if($resObj) $resObjs = array($resObj);
  else $resObjs = find_objects('reservation', $search);
  $count = count($resObjs);
  echo("Found $count reservations<br>");

  foreach($resObjs as $resObj) {
    $res_id = $resObj->res_id;
    $agent_id = $resObj->agent_id;
    $agentObj = get_object('agent', $agent_id);
    if(!$agentObj->balance_charged) continue; // the agent doesn't charge the client
    $inv_search['res_id'] = $res_id;
    $inv_search['type'] = INV_TYPE_AGENT;
    if(find_object('invoice', $inv_search)) {
      echo("Found invoice for $agentObj->name for reservation $resObj->res_id<br>");
      continue; // invoice already made
    }
    $new_id = add_agent_invoice($resObj, $agentObj);
    die("die");
  }
}

// determine correct client id and legal id for invoice
function get_invoice_client_legal_id($invObj) {

  
  $ptype = $invObj->property_type;
  if($res_id = $invObj->res_id) {
    $resObj = get_object('reservation', $res_id);
    if(!$resObj) echo("<span class='red'>Warning: Reservation not found for invoice ($invObj->id)</span><br>");
    //if($resObj->property_type != $ptype) die("get_invoice_client_legal_id: property type mismatch: $ptype (invoice $invObj->id) vs $resObj->property_type ($res_id)");
  }
  
  $p_type = $resObj->property_type ?: $invObj->property_type;
  $apt_id = $resObj->apt_id ?: $invObj->apt_id;
  if($apt_id && $ptype) { // apt has precedence
    $aptObj = get_object($ptype, $apt_id);
    $client_id  =  $aptObj->client_id; 
    $legal_id =  $aptObj->legal_entity_id;
    //echo("$ptype $apt_id: client=$client_id, legal=$legal_id<br>");
  } else if($site_id = $invObj->site_id) { // try the site
    $siteObj = get_object('site', $site_id);
    $client_id  =  $siteObj->client_id; 
    $legal_id =  $siteObj->legal_entity_id;
    //echo("site $site_id: client=$client_id, legal=$legal_id<br>");
  } else {
    echo(html_span("get_invoice_client_legal_id: no property or site_id on $invObj->id<br>", 'red'));
    $legal_id = $invObj->legal_entity_id;
    $client_id = '?';
  }
  
  

  //if(!$client_id) $invObj->client_id'] = $aptObj->client_id ?: $_SESSION['client_id']; 
  
  // special for Sicco
  if($aptObj->host_id == 1284) {
    $client_id = 1001;
    $num_days   = $resObj->num_days;
    if($num_days > 0 && $num_days < 6) {
      $legal_id = LEGAL_ENT_OPAO; // OPAO, no tax
    } else { // more than 6 days
      $legal_id = $aptObj->licensed ? LEGAL_ENT_YAH : LEGAL_ENT_HOUSIST; // YAH or Housist
    }
  } 
  
  return array($client_id, $legal_id);
}

// fixes account numbers, client, legal_id on old invoices
/** unsused - automatic comment added on  */
function fix_invoices($options) {
  //dump($options);
  
  if(!$options['search']) $options['search'] = array('status', array('>', 0));
  $search = $options['search'];
  if(!$options['sort']) $options['sort'] = 'created DESC';

  $limit = $options['limit'];
  $start = $options['start'] ?: 0;
  $first = $start + 1;
  $order = $options['order'];

  //dump($search);
  $invoices = find_objects('invoice', $search, $options);

  $total_count = count_objects('invoice', $search);
  $count = count($invoices);
  $last = $limit ? min($count, $start+$limit) :  $count;   

  if($start > 0) {    
    $prev_link = replace_query_var('', 'start', $start - $limit); 
    $prev_str = html_link($prev_link, "Previous");
  }
  if($total_count > $count) {    
    $next_link = replace_query_var('', 'start', $start + $limit); 
    $next_str = html_link($next_link, "Next");
  }
  $title = "$prev_str Showing $first-$last of $total_count invoices $next_str";

  $first_row = array('id' => 'inv_id', 'number' => 'invoice_id', 'exported' => 'export_timestamp', 'export', 'legal' => 'legal_entity_id', 'site' => 'site_id', 'type' => 'property_type', 'apt', 'stay', 'created' => 'created', 'date' => 'invoice_date', 'status' => 'status', 'tax?', 'incl?', 'total', 'paid' => 'paid', 'balance' => 'balance', 'org' => 'org_id', 'concerning / payments / lineitems'); 
  foreach($first_row as $k => $v) {
    if(isint($k)) $row[] = ucfirst($v);
    else $row[] = html_link(replace_query_var('', 'srt', $v), ucfirst($k));
  }
  
  if(!$search['client_id']) array_unshift($row, 'client');
  $rows[] = $row;
  
  foreach($invoices as $invObj) {
    $inv_id = $invObj->id;
    $inv_status = get_object('inv_status', $invObj->status, 'name');
      
    $invLiObjs = find_objects('inv_lineitem', array("inv_id" => $invObj->id));

    if($client_id = $invObj->client_id) $client = get_object('client', $client_id, 'name');
    else $client = '<span class="red">None</span>';

    if($legal_id = $invObj->legal_entity_id) {
      $legalObj = get_object('legal_entity', $legal_id);
      $charge_tax = $legalObj->charge_tax;    
      $tax_included = $legalObj->tax_included;    
      $legal = $legalObj->name;
    } else {
      $legalObj = NULL;
      $charge_tax = '?';    
      $tax_included = '?';    
      $legal = '<span class="red">None</span>';
    }                                                                                                 
    
    list($c_client_id, $c_legal_id) = get_invoice_client_legal_id($invObj);

    if($c_client_id) $c_client = get_object('client', $c_client_id, 'name');
    else $c_client = '<span class="red">None</span>';

    if($c_legal_id) $c_legal = get_object('legal_entity', $c_legal_id, 'name');
    else $c_legal = '<span class="red">None</span>';

     if($legal_id != $c_legal_id)  {
       $legal .=  html_span(" should be $c_legal", 'red');    
       $fix_legal_link = replace_query_var('', 'fix_legal', $invObj->id)."&fix_legal_id=".$c_legal_id; 
       $fix_legal_str = html_link($fix_legal_link, ' (Fix)');
       $legal .= " ".$fix_legal_str;
     }
     if($client_id != $c_client_id) $client .=  html_span(" should be $c_client", 'red');    
    
    $res_id = $invObj->res_id;
    $resObj = $res_id ? get_object('reservation', $res_id) : NULL;    
    $stay = $ptype == 'boat' ? $resObj->num_hours : $resObj->num_days;
    if(!$stay) $stay = '<span class="red">0</span>';
    $ptype = $invObj->property_type ?: $resObj->property_type;
    
    if($apt_id = $invObj->apt_id) {
      $aptObj = get_object($ptype, $apt_id);
      $licensed = $aptObj->licensed ? html_span(" (lic.)", 'green') : '' ;
      $apt = $aptObj->name.$licensed;
    } else {
      $apt = '<span class="red">None</span>';
    }
    
    if($site_id = $invObj->site_id) $site = get_object('site', $site_id, 'shortname');
    else $site = '<span class="red">None</span>';
                        
    $li_total = 0;
    $li_rows = array();
    foreach($invLiObjs as $liObj) {
      $li_id = $liObj->id;
      $account_number = lineitem_account_number($liObj);
      if($type_id = $liObj->type) $li_type = get_object('lineitem_type', $type_id, 'name');
      else $li_type = '<span class="red">None</span>';
      $li_total += $liObj->total;
        
      $li_rows[] = array('', '', $li_type, $liObj->account_number, $account_number, $liObj->total, '', '', '',$liObj->tax_of_which, $liObj->tax, $liObj->description);
      
      if($options['update']) {
        if($liObj->account_number != $account_number) {
          echo("UPDATE inv_lineitem SET account_number='$account_number' WHERE id=$li_id<br>");
          db_query("UPDATE inv_lineitem SET account_number='$account_number' WHERE id=$li_id");
        }
      }
    }

    $count = count($li_rows);
    if($count) array_unshift($li_rows, array('', '<b>Lines:</b>', 'type', 'old account', 'new account', 'amount', '', '', '',  'tax_of_which', 'tax', 'desc')); 
    if($options['show_all'] && $count) {
      $li_table = html_table($li_rows, array('caption' => $title, 'class' => 'web20'));
      $li_str = show_hide_div($li_table, "$count lineitems");
    } else {
      $li_str = '';
    }
    //if($fix_id = $options['update']['legal_entity_id']) {
    //  db_query("UPDATE invoice SET client_id='$c_client_id', legal_entity_id='$c_legal_id' WHERE id=$inv_id");
    //}
    
    //if($options['update']) db_query("UPDATE invoice SET client_id='$c_client_id', legal_entity_id='$c_legal_id', processed=1 WHERE id=$inv_id");
    // if($options['update']) db_query("UPDATE invoice SET processed=1 WHERE id=$inv_id");

    // payments
    $paymentObjs = find_objects('payment', array('inv_id' => $inv_id));
    $paid = 0;
    $pay_rows = array();
    foreach($paymentObjs as $paymentObj) {
      $paid += $paymentObj->amount;
      $pay_type = get_object('payment_type', $paymentObj->payment_type, 'name');
      $pay_status = get_object('payment_status', $paymentObj->payment_status, 'name');
      $pay_rows[] = array('', '', $pay_type, $pay_status, strip_time($paymentObj->created), $paymentObj->currency, $paymentObj->amount, $paymentObj->description);
    }
    $count = count($pay_rows);
    if($count) array_unshift($pay_rows, array('', '<b>Payments:</b>', 'type', 'status', 'date', 'currency', 'amount', 'desc')); 
    if($options['show_all'] && $count) {
      $pay_table = html_table($pay_rows, array('caption' => $title, 'class' => 'web20'));
      $pay_str = show_hide_div($pay_table, "$count payments");
    } else {
      $pay_str = '';
    }

    $balance = round($invObj->total - $paid, 2);
    $total_str   = (abs($invObj->total - $li_total) < 1) ? "<span class='green'>Total:</span>   $invObj->total "  : "<span class='bold red'>Total:</span> Invoice: $invObj->total vs Lines: $li_total"; 
    $paid_str    = abs($invObj->paid - $paid) < 1        ? "<span class='green'>Paid:</span>    $invObj->paid"    : "<span class='bold red'>Paid:</span> Invoice: $invObj->paid vs Payments: $paid"; 
    $balance_str = abs($invObj->balance - $balance) < 1  ? "<span class='green'>Balance:</span> $invObj->balance" : "<span class='bold red'>Balance:</span> Invoice: $invObj->balance vs Payments: $balance"; 
    
    $charge_tax_str = $invObj->charge_tax == $charge_tax ? $invObj->charge_tax : html_span("$invObj->charge_tax => $charge_tax", 'red');  
    $tax_included_str = $invObj->tax_included == $tax_included ? $invObj->tax_included : html_span("$invObj->tax_included => $tax_included", 'red');  
    $org_str = $invObj->org_id ? get_object('org', $invObj->org_id, 'name') : '';

    if($legalObj->reeleezee_username && $legalObj->reeleezee_password) {
      $export_link = replace_query_var('', 'export', $invObj->id); 
      $export_str = html_link($export_link, 'Send');
    } else {
      $export_str = html_span('X', 'red');
    }
    
    $last = implode(html_break(), array_filter(array($invObj->concerning, $li_str, $pay_str)));
    $row = array($invObj->invoice_id, strip_time($invObj->export_timestamp), $export_str, $legal, $site, $ptype, $apt, $stay, strip_time($invObj->created), strip_time($invObj->invoice_date), $inv_status, $charge_tax_str, $tax_included_str, $total_str, $paid_str, $balance_str, $org_str, $last);
    if(!$search['client_id']) array_unshift($row, $client);                                                                   
    $inv_link = html_link(invoice_link($inv_id), $inv_id, false, '', 'target="_new"', '') ; 
    array_unshift($row, $inv_link);
    $rows[] = $row;
    //if($options['show_all']) {
    //  $rows = array_merge($rows, $li_rows);
    //  $rows = array_merge($rows, $pay_rows);      
    //}

  }
  $output .= html_table($rows, array('caption' => $title, 'class' => 'web20'));
  return $output;
}
  

function add_agent_invoice($resObj, $agentObj) {
  
  $commissionable_types = array(LI_TYPE_DEPOSIT, LI_TYPE_RENT, LI_TYPE_BALANCE); // lineitem types that are commissionable
  
  $invObj = find_booking_invoice($resObj->res_id);
  if(!$invObj)  {                              
    echo("add_agent_invoice: Could not find booking invoice for reservation $resObj->res_id<br>");
    return;
  }
  $inv_id = $invObj->id;
  $copy_fields = array('res_id', 'org_id', 'agent_id', 'host_id', 'city_id', 'site_id', 'client_id', 'legal_entity_id', 'apt_id', 'property_type', 
                       'charge_city_tax', 'tax_included', 'currency', 'start_date', 'end_date', 'guest_name', 'concerning'); 
  
  $invData = array_intersect_key(obj2array($invObj), array_flip($copy_fields));
  $invData['type'] = INV_TYPE_AGENT;

  $addData = copy_address($agentObj);
  foreach($addData as $k => $v) $invData[$k] = $v;
  
  list($new_inv_id, $errors) = add_object('invoice', $invData);
  if($errors) {dump($errors); die("add_agent_invoice: add invoice");}

  $liObjs = get_lineitems($inv_id);
  $unset = array('id', 'created', 'timestamp', 'user_id');
  foreach($liObjs as $liObj) {    
    foreach($unset as $fld) unset($liObj->$fld);
    $liObj->inv_id = $new_inv_id;
    list($lid, $lerrors) = add_object('inv_lineitem', $liObj);
    if($lerrors) {dump($lerrors); die("add_agent_invoice: add lineitem");}
    if(in_array($liObj->type, $commissionable_types))$total += $liObj->total;
  }

  // add commission
  if($commission = $agentObj->commission) {
    list($amount, $perc) = split_discount($commission);
    $liData['description'] = "Your commission:";
    if($perc) {
      $liData['amount']   = round(($perc*$total)/100, 2) * -1;
      $liData['description'] .= " $perc% of $total";
    } else {
      $liData['amount']   = $amount  * -1;
    }
    $liData['inv_id']   = $new_inv_id;
    $liData['type']     = LI_TYPE_COMMISSION;
    $liData['currency'] = $invObj->currency;

    list($lid, $lerrors) = add_object('inv_lineitem', $liData);
    if($lerrors) {dump($lerrors); die("add_agent_invoice: add commission lineitem");}
  }
  $link = invoice_link($new_inv_id);
  echo("Added agent invoice ".html_link($link, $new_inv_id)." for $agentObj->name: Reservation $resObj->_resid<br>");  
  return $id;
}

// end new invoice functions

// converts legal object to org array for inserting
function legal2org($legalObj) {
  $orgData = array_copy($legalObj, array('account_number' => 'bank_account_number', 
                                       'account_name' => 'bank_account_number', 
                                       'tax_id' => 'tax_number', 
                                       'commerce_id' => 'coc_number'), true);
  unset($orgData['id']);
  return $orgData;
}

function generate_software_invoice($host_id, $start, $options) {
  $debug  = $options['debug'];
  $dryrun = $options['dryrun'];
  $fee    = $options['fee'] ?: '5'; // The percentage fee
  $my_client_id = 1005; // Syspensa
  $my_legal_id = 1005; // Syspensa
  $my_site_id = 213;
  $currency = 'EUR'; // hard coded for now
  
  list($year, $month, $day) = explode('-', $start);
  $month_name = month_name($month);
  
  $client_id = 1001;  

  // hack for now to always use YAHsele for Sicco
  if($client_id == 1001) {
    $legalObj = get_object('legal_entity', 1008);
  } else {
    $legalObjs = find_objects('legal_entity', array('client_id' => $client_id));
    //dump($legalObjs);
    $legal_count = count($legalObjs);
    $legalObj = $legalObjs[0];
    $legal_name = $legalObj->name;
    if($legal_count > 1) {
      if($debug) $debug_txt .= "Found $legal_count legal entities for $client_id, using $legal_name<br>";
    }
  }
  $org_id = find_object('org', array('name' => $legalObj->name, 'client_id' => $my_client_id), 'org.id');
  if(!$org_id) { //  no org for this client, add it 
    $orgData = legal2org($legalObj);  
    $orgData['client_id'] = $my_client_id;
    list($org_id, $errors) = add_object('org', $orgData);
    if($org_id) {
      if($debug) $debug_txt .= "Added org for legal entity $client->name $org_id<br>";
    } else {
      dump($errors);
      die("Could not add org for $client->name");
    }
  }
  //dump($orgObj);die();
  $orgObj = get_object('org', $org_id);

  if($debug) echo(html_b("generate_software_invoice: host: $host_id client: $client_id org: $orgObj->name start:$start").html_break().$debug_txt);

  
  $invSearch['type'] = INV_TYPE_CONSULTING;
  $invSearch['status'] = array('>=', INV_STATUS_OPEN);
  $invSearch['client_id'] = $my_client_id;
  $invSearch['legal_entity_id'] = $my_legal_id;
  $invSearch['start_date'] = $start;
  //$invSearch['end_date'] = $end_date;
  $invSearch['host_id'] = $host_id;

  if($invObj = find_object('invoice', $invSearch)) {
    $inv_id = $invObj->id;
    if($debug) echo("Found existing invoice $inv_id for {$orgObj->name}: $invObj->concerning.<br>Total=$invObj->total Fee: $fee<br>");

  } else {  

    // copy address from org 
    $address_fields = object_fields('address', false, true);
    $invData = array_copy($orgObj, $address_fields);
    //else $invData = array_copy($legalObj, $address_fields);
  
    $stats = get_host_stats($host_id, $start, $options);
    $total = $stats['total'];                                                           
  
    $start_date = $total['start_date'];
    $end_date   = $total['end_date'];
    if(!$start_date || !$end_date) die("No start or end date");
    
    $invData['type']         = INV_TYPE_CONSULTING;
    $invData['status']       = INV_STATUS_SENT;
    $invData['start_date']   = $start_date;
    $invData['end_date']     = $end_date;
    $invData['invoice_date'] = $end_date;
    $invData['org_id']       = $org_id;
    $invData['client_id']    = $my_client_id;
    $invData['legal_entity_id'] = $my_legal_id;
    $invData['concerning']   = "Syspensa Consulting $month_name $year";
    $invData['currency']     = $currency;
    $invData['city_id']      = $total['city_id'];
    $invData['charge_tax']   = $legalObj->charge_tax;
    $invData['tax_included'] = $legalObj->tax_included;
    $invData['site_id']      = $my_site_id;
    $invData['host_id']     = $host_id;
    
    
    $fee = $total['software_fee'];
    if($debug) {
      echo("Fee for $start_date - $end_date = $fee<br>");
      dump($invData);
      dump($stats); 
      die();
    }
    
    $li_array = array();      
    // add lineitems
    $li = array();
    $li['inv_id']       = $inv_id;
    $li['quantity']     = 1;
    $li['currency']     = $currency;
    if($fee > 0) {
      $li['tax_category'] = 'H';
      $li['amount']       = $fee;
      $li['description']  = "Software usage fee $month_name $year";
      $li['type']         = LI_TYPE_SOFTWARE_USAGE_FEE;
      $li_array[] = $li; // add second line item
    }
    $total = $fee;
    $wSearch['host_id'] = $host_id;
    $wSearch['created'] = array('><', $start_date, "$end_date 23:59:59");
    $wObjs = find_objects('withdrawal', $wSearch);
    $w_count = count($wObjs);
    
    /**
    if($w_count) {
      foreach($wObjs as $wObj) {
        $withdrawal_currency = $wObj->currency;
        $withdrawal_fee = $wObj->gateway_fee;
        $w_date = sql2human($wObj->created);
        $desc = "Gateway withdrawal fee on $w_date";
        if($withdrawal_fee > 0) {
          $total += $withdrawal_fee; 
          if($withdrawal_currency != $currency) {
            $org_withdrawal_fee = $withdrawal_fee;
            $withdrawal_fee = get_exchange_rate($withdrawal_currency, $currency, $withdrawal_fee, false); // convert to invoice currency
            $desc .= " ($withdrawal_currency $org_withdrawal_fee)";
          }
          $li['tax_category'] = 'N';
          $li['amount']       = $withdrawal_fee;
          $li['description']  = $desc;
          $li['type']         = LI_TYPE_GATEWAY_FEE;
          $li_array[] = $li; // add second line item
        }  
      }
    }
    */
    
    $li_count = count($li_array);
    if($dryrun) {
      if($debug) echo("Dryrun, not adding invoice with $li_count lineitems, $w_count withdrawals - Fee: $fee Total: $total<br>");      
    } elseif($li_count) {
      list($inv_id, $errors) = add_object('invoice', $invData);
      if($inv_id) {
        $invoice_id = set_invoice_id($inv_id, $my_client_id);
        foreach($li_array as &$li) $li['inv_id'] = $inv_id;
        //dump($li_array);die();
        $li_errors = add_or_update_invoice_lineitems($inv_id, $li_array);
        $invObj = get_object('invoice', $inv_id); // reload
        if($debug) echo("Added invoice $inv_id with invoice_id $invoice_id with $li_count lineitems, $w_count withdrawals - Fee: $fee Total: $invObj->total Tax: $invObj->tax_of_which <br>");
      } else {
        dump($errors);
      }
    } else {
      if($debug) echo("Not adding invoice $inv_id - $li_count lineitems, $w_count withdrawals - Fee: $fee Total: $total<br>");
    }
  }
  if($debug) echo("---------<br>");
  return $invObj;
}

// start is an array (first_month, first_year) or mysql date
// end is an array (last_month, last_year) or mysql date
/** unsused - automatic comment added on  */
function generate_software_invoices($start, $end, $host_ids, $options=NULL) {
  $dryrun = $options['dryrun'];

  if(is_array($start)) list($month, $year) = $start;
  elseif(is_date($start)) list($day, $month, $year) = sql2date($start);
  else die("invalid start date $start");
  
  if(is_array($end)) list($last_month, $last_year) = $end;
  elseif(is_date($end)) list($last_day, $last_month, $last_year) = sql2date($end);
  else die("invalid end date $end");

  if(!is_array($host_ids)) $host_ids = array($host_ids);
  $invObjs = array();
  while(!($month > $last_month && $year >= $last_year)) {
    foreach($host_ids as $host_id) {
      $start = "$year-".zero_pad($month)."-01";
      $invObj = generate_software_invoice($host_id, $start, $options);
      if($invObj) $invObjs[] = $invObj;
    }  
    list($month, $year) = get_next_month($month, $year);
  }
  return $invObjs;
}

// helper function for debugging invoices
/** unsused - automatic comment added on  */
function invoice_debug_print($res_id) {
  global $db;
  $resObj = get_object('reservation', $res_id);
  $invObjs = find_reservation_invoices($res_id, array('search' => array('type' => INV_TYPE_BOOKING, 'status' => array('>', -100)))); // existing reservation booking invoice objects, cancelled or otherwise (ignore others)
  if(!$invObjs) return "No invoices";
  
  $rows[] = array("id", "invoice_id", "main?", "type", "status", "locked?", "start", "end", "days", "tt", "total", "paid", "balance");
  if($resObj) $rows[] = array("", "", "", "Res:", "", "", "$in", "$out", "$nd", "$resObj->city_tax", "$resObj->grand_total", "$resObj->paid", "$resObj->balance");
  $ind = $ind_total = $balance = $total = $paid = $tt_total = 0;
  foreach($invObjs as $iObj) {
    $locked = invoice_locked($iObj);
    $type = get_object('inv_type', $iObj->type, 'name');
    $status = get_object('inv_status', $iObj->status, 'name');
    $locked_str = $locked ? html_span("Yes", 'red') : html_span("No", 'green');
    $start = strip_time($iObj->start_date);
    $end = strip_time($iObj->end_date);
    $ind = period_diff($start, $end, 'd');
    $itt = db_getOne("SELECT sum(total) FROM inv_lineitem WHERE inv_id=$iObj->id AND type=".LI_TYPE_CITY_TAX);
    $rows[] = array($iObj->id, $iObj->invoice_id, $iObj->head_invoice, $type, $status, $locked_str, $start, $end, $ind, $itt, $iObj->total, $iObj->paid, $iObj->balance);
    if($iObj->status >= INV_STATUS_DRAFT) {
      $tt_total += $itt;
      $ind_total += $ind;
      $balance += $iObj->balance;
      $total += $iObj->total;
      $paid += $iObj->paid;
    }
  }
  $rows[] = array("", "", "Total:", "","", "", "", "", "$ind_total", $tt_total, "$total", "$paid", "$balance");
  return html_table($rows, array('class' => 'web20'));
}

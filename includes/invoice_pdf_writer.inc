<?php
$ul=$_SESSION['loggedin']['user_level'];


require('tfpdf/tfpdf.php');

/** Create blank PDF with header / footer */
function create_pdf($siteObj_or_id, $options=[]) {
  $obj_type = $options['obj_type'];
  $obj = $options['obj'];
  
  $siteObj = any2obj('site', $siteObj_or_id);
  if(!$siteObj) die("create_pdf: Missing site");
  
  if($legal_id = pick_first($obj->legal_entity_id, $siteObj->legal_entity_id)) $legalObj = get_object('legal_entity', $legal_id);
  $client_id = $obj->org_id ? get_object('org', $invObj->org_id, 'name') : $invObj->guest_id;

  // header
  $company_text = '';
  if($legalObj) {
    $coArray = get_address_array($legalObj);
    $company_text = implode("\n", array_unique($coArray));
    print_log("text: {$company_text} Ar:".dump($coArray, true)."Obj:".dump($legalObj, true), 'invoice', LOG_LEVEL_TEST);
  }
    
  // footer
	if($tax_no  = $legalObj->tax_id) $footer_text = "Registration number / Tax ID: $tax_no\n"; 
  
	
	extract($options);
	
  // colors  
	$theme_color = strlen($siteObj->theme_color) == 6 || strlen($siteObj->theme_color) == 3 ? $siteObj->theme_color : "ffffff";	
	$text_color = ($theme_color == "fff" || $theme_color == "ffffff") ? "000" : "fff";
	$text_color_fg = "000";
		
  //$pdf = new tFPDF();
  $pdf = new PDF();
  
  // Add a Unicode font (uses UTF-8)
  $pdf->AddFont('DejaVu','','DejaVuSans.ttf',true);
  $pdf->AddFont('DejaVu','B','DejaVuSans-Bold.ttf',true);
  $pdf->AddFont('DejaVu','I','DejaVuSans-Oblique.ttf',true);

  
  $pdf->SetFont('DejaVu','',14);
  //dump($pdf);die();;
  
  $pdf->logo = $siteObj->logo_print ?: $siteObj->logo;
  $pdf->title = strtoupper($type);
  $pdf->company = $company_text;
  $pdf->type = $type;
	$pdf->footer_text = $footer_text; 
	$pdf->theme_color = $theme_color;
	$pdf->text_color = $text_color;
	$pdf->text_color_fg = $text_color_fg;
  $pdf->AliasNbPages();
  $pdf->AddPage();
  $pdf->SetFont('Times','',12);
	$pdf->width = 180;	

  $i=1;
  $w = 20; // cell width
  $left_x = 15; 
  

  // divider
  //$pdf->Ln(5);
	//$pdf->Line($pdf->getX(), $pdf->getY(), $pdf->getX()+$pdf->width, $pdf->getY());
  //$pdf->Ln(5);
  

  foreach($headers as $k=>$v) {
    if($v) {
		  $pdf->SetFont('DejaVu','B',10);
      $pdf->Cell(40,6,"$k:",0,0);
		  $pdf->SetFont('DejaVu','',10);
      $pdf->Cell(0,6,"$v",0,1);
    }
  }

  $pdf->Ln(5);

  return $pdf;
}


function create_pdf_booking($reservation) {
  
// Optionally define the filesystem path to your system fonts
// otherwise tFPDF will use [path to tFPDF]/font/unifont/ directory
// define("_SYSTEM_TTFONTS", "C:/Windows/Fonts/");

  
  $obj_type = 'reservation';
  $resObj = any2obj('reservation', $reservation);
  $res_id = $resObj->res_id;
  $org_id = $resObj->org_id;
  
  if(!$res_id) mydie("create_pdf_confirmation: Booking not found");

  $site_id = pick_first(array($invObj->site_id, $resObj->site_id, $_SESSION['site_id']));  
  if(!$site_id) die("no site id on invoice, res or in session..");
  $siteObj = get_object('site', $site_id);
  if(!$siteObj) die("could not find site '$site_id'");

  
  $path = get_pdf_url($resObj, 'reservation');
  $filename = docroot(false).$path; // get_pdf_path($invObj);				

  $pdf = create_pdf($siteObj, ['obj_type' => 'reservation', 'obj' => $resObj, 'type' => 'itinerary']);

  $ptype = $resObj->property_type ?: 'shortstay';
  $aptObj = get_object($ptype, $resObj->apt_id);

  $checkin_instructions = '';
  if($desc = find_object('property_description', ['host_id' => $resObj->host_id, 'active' => 1, 'property_id' => 0], 'description')) $checkin_instructions .= $desc."\n"; 
  if($desc = find_object('property_description', ['host_id' => $resObj->host_id, 'active' => 1, 'property_id' => $resObj->apt_id], 'description')) $checkin_instructions .= $desc."\n"; 

  // guest 
  $guestObj = $resObj->guest_id ? get_user($resObj->guest_id) : $resObj;
	$name = get_name_string($guestObj);
	
  $y0 = $pdf->getY();
  $pdf->SetCol(0);
  $pdf->SetFont('DejaVu','B',10);
  $pdf->Cell(0,5,"Guest:",0,1);
  $pdf->SetFont('DejaVu','',10);
  $pdf->Cell(0,5,$name,0,1);
  //foreach($address as $v) if(trim($v)) $pdf->Cell(0,5,trim($v),0,1);
  if($guestObj->phone) $pdf->Cell(0,5,"Tel: {$guestObj->phone}",0,1);
  $pdf->Cell(0,5,"{$guestObj->email_address}",0,1);

  // host
  $hostObj = get_user($resObj->host_id);
  $name = get_user_name($hostObj);
    
  $pdf->SetCol(1);
  $pdf->SetY($y0);
  $pdf->SetFont('DejaVu','B',10);
  $pdf->Cell(0,5,"Host:",0,1);
  $pdf->SetFont('DejaVu','',10);
  $pdf->Cell(0,5,$name,0,1);
  if($hostObj->phone) $pdf->Cell(0,5,"Tel: {$hostObj->phone}",0,1);
  $pdf->Cell(0,5,"{$hostObj->email_address}",0,1);
  

  // rental
  $pdf->SetCol(2);
  $pdf->SetY($y0);
 
  $pdf->SetFont('DejaVu','B',10);
  $pdf->Cell(0,5,"Rental:",0,1);
  $pdf->SetFont('DejaVu','',10);
  $pdf->Cell(0,5,"{$aptObj->name}",0,1);  
  $address  = get_address_array($aptObj);
  foreach($address as $v) if(trim($v)) $pdf->Cell(0,5,trim($v),0,1);

  $pdf->SetCol(0);

  // Line break
  $pdf->Ln(5);
	$pdf->Line($pdf->getX(), $pdf->getY(), $pdf->getX()+$pdf->width, $pdf->getY());
  $pdf->Ln(5);

  $y0 = $pdf->getY();
  
  // Booking details
  $rows = array('Booking Date' => sql2human($resObj->created),
                 'Booking ID' => $resObj->res_id,
                 'Check-in' =>  sql2human($resObj->checkin),
                 'Check-out' =>  sql2human($resObj->checkout),
                 //'Period' =>  print_time_range($resObj->checkin, $resObj->checkout),
                 'Concerning' => "{$resObj->num_guests} ".($resObj->num_guests == 1 ? "guest" : "guests").", {$resObj->num_days} ".($resObj->num_days == 1 ? "day" : "days"));

  foreach($rows as $k=>$v) $pdf->PrintRow($k,$v);

  $pdf->SetCol(1.5);
  $pdf->SetY($y0);
  $cur = get_currency_symbol($resObj->currency);
  $rows = array('Rate' => $cur.round($resObj->rate_base,2)."/".$resObj->rate_type,
                 'Fees' => $cur.($resObj->fees + $resObj->fee_service),
                 'Total' => $cur.$resObj->grand_total,
                 'Paid' =>  $cur.$resObj->paid,
                 'Due' =>  $cur.$resObj->balance
                 );

  foreach($rows as $k=>$v) $pdf->PrintRow($k,$v);

  $pdf->Ln(5);
  $pdf->SetCol(0);
  $pdf->Ln(5);

  $pdf->SetX(15);
  $pdf->SetFont('DejaVu','B',14);
  $pdf->SetTextColor(0,0,0);
  $pdf->SetLineWidth(0.2);
  $pdf->MultiCell(180,4,"Arrival Instructions",0,'C');

  $pdf->Ln(5);
  $pdf->SetFont('DejaVu','',10);
  $pdf->MultiCell(180,4,$checkin_instructions,0,'L');
  $pdf->Ln(3);
  
  $pdf->Output($filename, "F");
  
  return $path;
}


function create_pdf_invoice($invObj_or_id) {
  	
  
  
  $obj_type = 'invoice';

  //$invObj=get_object($obj_type, $inv_id);
  $invObj = any2obj('invoice', $invObj_or_id);
  $inv_id = $invObj->id;
  if(!$inv_id) mydie("create_pdf_invoice: Invoice not found");
  
  if($rid = $invObj->res_id) $resObj = get_object('reservation', $rid); // for backwards compat.

  $site_id = pick_first(array($invObj->site_id, $resObj->site_id, $_SESSION['site_id']));  
  if(!$site_id) die("no site id on invoice, res or in session..");
  $siteObj = get_object('site', $site_id);
  if(!$siteObj) die("could not find site '$site_id'");
  //if($aff_id = $siteObj->affiliate_id) $affObj = get_object('affiliate', $aff_id);
  
  if($legal_id = pick_first($invObj->legal_entity_id, $siteObj->legal_entity_id)) $legalObj = get_object('legal_entity', $legal_id);
	$invoice_id = $invObj->invoice_id;

	/**
	$coArray = get_address_array($legalObj);
	//array_unshift($coArray, $legalObj->name);	
	$company_text = implode("\n", array_unique($coArray));
	*/
	
  $parent_field = $invObj->type == INV_TYPE_BLOCK ? 'block_parent' : 'inv_id';
  $lineitems = get_lineitems($inv_id);
  $payments  = find_objects('payment', array('show_invoice' => 1, $parent_field => $inv_id, 'payment_status' => array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_REFUNDED,PAYMENT_STATUS_PARTIALLY_REFUNDED))); 

  $rid = $invObj->res_id;
  if($org_id = $invObj->org_id) $orgObj = get_object('org', $org_id);
  
  $pending_payments = pending_payments($inv_id);
    
  if($legalObj->invoice_id_required && !$invoice_id) { // for those using reeleezee, it's not a real invoice until it has been exported and assigned a sequential invoice ID
    $type = 'voucher';
    $voucher_id = $inv_id;
  } else {
    if(!$invoice_id) $invoice_id = $inv_id;
    if($invObj->total < 0) {
      $type = 'credit slip';
    } elseif($invObj->paid + INV_TOLERANCE >= $invObj->total && !$pending_payments) {
      $type = 'receipt';
    } else {
      $type = 'invoice';
    }
  }

  // address
	$billing_name = get_name_string($invObj);
	$company_name = $org_id ? pick_first($invObj->organization, $orgObj->organization, $orgObj->name) : $invObj->organization;
	unset($invObj->organization); // otherwise double
  $name = $company_name && $billing_name ? array($company_name, $invObj->title, "Attn: $billing_name") : array_filter(array($company_name, $invObj->title, $billing_name));
	$address = get_address_array($invObj);
  $full_address = array_filter(array_unique(array_merge($name, $address)));

  if(0) {
    $coArray[] = "site_id: $site_id";
    $coArray[] = "inv sid: $invObj->site_id";
    $coArray[] = "res sid: $resObj->site_id";
    $coArray[] = "name: $siteObj->name";
    $coArray[] = "id: $siteObj->id";
    $coArray[] = "lid: $siteObj->legal_entity_id";
    $coArray[] = "logo: $siteObj->logo";
  	$company_text = implode("\n", $coArray);
  	$company_text = implode("\n", $invObj);
  }
  
  // description
  $month_array = array('Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct', 'Nov', 'Dec');
  $date = sql2human($invObj->invoice_date ?: $invObj->created);
	
  if($res_id = $invObj->res_id) {
  	$resObj = get_object('reservation', $res_id);
	  $checkin = sql2human($resObj->checkin);
	  $checkout = sql2human($resObj->checkout);
	  // $title .= " - Apartment $resObj->apartment";
	  $title .= ", $checkin - $checkout";
	}
	
	$start_date = $invObj->start_date;
	$end_date = $invObj->end_date;
	
	$show_time = $resObj->num_hours > 0 || strip_time($start_date) == strip_time($end_date);
  if($show_time) {
    $start_time = strip_date_sec($start_date);
    $end_time   = strip_date_sec($end_date);
    $period_str = sql2human($start_date)." $start_time - $end_time";    
  } else {
    $start_date = sql2human($start_date); // , false, $show_time);
    $end_date = sql2human($end_date); // false, $show_time);
    $period_str = "$start_date - $end_date";
  }
  
  $client_id = $invObj->org_id ? get_object('org', $invObj->org_id, 'name') : $invObj->guest_id;
  //if(!$client) $client = "$invObj->first_name $invObj->last_name $checkin - $checkout";

  //$period_count = $resObj->num_hours > 0 ? "($resObj->num_hours hours)" : ($resObj->num_days > 0 ? "($resObj->num_days days)" : '');
  $headers = array('Date' => $date,
                 'Invoice ID' => $invoice_id,
                 'Voucher ID' => $voucher_id,
                 'Client' => $client_id,
                 'Reservation ID' => $res_id,
                 'Period' => "$period_str $period_count",
                 'Concerning' => $invObj->concerning,
                 'Notes' => $invObj->customer_notes,
                 'Location' => $invObj->location,
                 'Comments' => $invObj->notes);


    // accounting
  $total = $invObj->total;
  $paid = $invObj->paid;
  $balance = $invObj->balance;
  $tax = $invObj->tax;
  $tax_of_which = $invObj->tax_of_which;
//  $discount = $invObj->discount;
  
  $total_excl = $total - $tax;
	$currency = $invObj->currency;
  $currency_symbol = get_currency_symbol($invObj->currency);
	$vat_text = $tax ?  "VAT" : "VAT (n/a)";
  $paid_text = $pending_payments ? 'Paid (Payment pending)' : 'Paid';

  $tax_of_which_ar = get_of_which_tax($invObj, $lineitems);
            
  // if($invObj->charge_tax || $tax > 0) {
  //   if($invObj->tax_included) { // show tax already included
  //   } elseif($tax > 0) { // add tax
  //   }
  // } else { // no tax
  // }
                               
      $price['__line'] = '';
      $price['Total'] = $currency_symbol." ".number_format($total, 2);
      foreach($tax_of_which_ar as $tax_rate => $tax_amount)  {
        if($tax_amount > 0) $price["Of which VAT $tax_rate %"] = $currency_symbol." ".number_format($tax_amount, 2);
      }

  //  if($tax > 0 ) {
//  $price = array('Total excl. VAT' => $currency_symbol." ".number_format($total_excl, 2),
////                 'Discount' => print_discount($discount, $currency_symbol),  
//                 $vat_text => $currency_symbol." ".number_format($tax, 2),
//                 'Total incl. VAT' => $currency_symbol." ".number_format($total, 2),  
//                 $paid_text => $currency_symbol." ".number_format($paid, 2),
//                 '__line' => '',
//                 'Total due' => $currency_symbol." ".number_format($balance, 2) 
//                 );
//  } else {
//  $price = array('Total ' => $currency_symbol." ".number_format($total, 2),
//                 $paid_text => $currency_symbol." ".number_format($paid, 2),
//                 '__line' => '',
//                 'Total due' => $currency_symbol." ".number_format($balance, 2) 
//                 );
//  }

  $path = get_pdf_url($invObj);
  $filename = docroot(false).$path; // get_pdf_path($invObj);				

  switch($type) {
    case 'invoice':
      $end_text = "Payment due within 14 days, stating invoice number and client name\n";
      if($legalObj->bank_name && $legalObj->account_name && $legalObj->account_number) $end_text .= "Bank: $legalObj->bank_name - Account Name: $legalObj->account_name\nAccount #: $legalObj->account_number\n";
      if($ba_no  = $legalObj->commerce_id)    $end_text .= "Chamber of commerce: $ba_no\n";
      break;
    case'receipt':
      $end_text = "Paid in full.";
      break;
    case'voucher':
      $end_text = "This voucher is not a formal invoice, but serves as a receipt of your payment and/or indication of amount due until the final invoice is created by our accounting department.\n";
      if($legalObj->bank_name && $legalObj->account_name && $legalObj->account_number) $end_text .= "Bank: $legalObj->bank_name - Account Name: $legalObj->account_name\nAccount #: $legalObj->account_number\n";
      if($ba_no  = $legalObj->commerce_id)    $end_text .= "Chamber of commerce: $ba_no\n";
      break;
    case 'credit slip':
      $end_text = "";
      break;
    default:
      //$end_text = "type='$type'";
      break;
	 }
	// print PDF

  $pdf = create_pdf($siteObj, ['obj_type' => 'invoice', 'obj' => $invObj, 'type' => $type]);
	
  $pdf->end_text = $end_text;
  
  $i=1;
  $w = 20; // cell width
  $left_x = 15; 
  
  // let's first figure out how many columns we need for the lineitems (excluding the description, which is always there)
  // minimum is 1 (just the total)
  // at present, max is 5: quantity, unit price, discount, tax, total
  // later maybe shipping + handling

  $show_discount = db_getOne("SELECT sum(discount) FROM inv_lineitem WHERE inv_id=$inv_id") > 0;
  $show_tax = db_getOne("SELECT sum(tax) FROM inv_lineitem WHERE inv_id=$inv_id") > 0;
  $show_tax_of_which = db_getOne("SELECT sum(tax_of_which) FROM inv_lineitem WHERE inv_id=$inv_id") > 0;
  $show_quantity = count(db_getCol("SELECT max(quantity) FROM inv_lineitem WHERE inv_id=$inv_id")) > 1;
  $show_quantity = true;

  $num_cols = 1;  // total
  if($show_quantity) $num_cols++; // quantity
  if($show_quantity || $show_discount || $show_tax) $num_cols++; // unit price
  if($show_discount) $num_cols++; // discount
  if($show_tax || $show_tax_of_which) $num_cols++; // tax

  $first_col = 100 + (5 - $num_cols) * $w; 
  $first_pay_col = 100; 
  
  foreach($full_address as $v) {
    if($v) {
      $pdf->Cell(0,5,$v,0,1);
    }
  }

  $pdf->Ln(5);
	$pdf->Line($pdf->getX(), $pdf->getY(), $pdf->getX()+$pdf->width, $pdf->getY());
  $pdf->Ln(5);

  foreach($headers as $k=>$v) {
    if($v) {
		  $pdf->SetFont('DejaVu','B',10);
      $pdf->Cell(40,6,"$k:",0,0);
		  $pdf->SetFont('DejaVu','',10);
      $pdf->Cell(0,6,"$v",0,1);
    }
  }

  $pdf->Ln(5);


  $debug = false;

  $x = $first_col;
  // Lineitem Header
  $pdf->SetFont('DejaVu','B',10);
  $pdf->SetX($left_x);
  $pdf->Cell(0,6,"Description",0,0);

  //$pdf->SetX($x);
  //$x+=$w;

  if($show_quantity) {  
    $pdf->SetX($x);
    $pdf->Cell($w,6,"Quantity",0,0, 'R');
    $x+=$w;
  }
  if($show_quantity || $show_tax || $show_discount) {
    $pdf->SetX($x);
    $pdf->Cell($w,6,"Unit Price",0,0, 'R');
    $x+=$w;
  }
  
  //$x = 140;

  if($show_discount) {
    $pdf->SetX($x);
    $pdf->Cell($w,6,"Discount",0,0, 'R');
    $x+=$w;
  }
  if($show_tax) {
    $pdf->SetX($x);
    $pdf->Cell($w,6,"+ VAT",0,0, 'R');
    $x+=$w;
  } else if($show_tax_of_which) {
    $pdf->SetX($x);
    $pdf->Cell($w,6,"Incl. VAT",0,0, 'R');
    $x+=$w;
  }

  $pdf->SetX($x);
  $pdf->Cell($w,6,"Total",0,1, 'R');

  $max_x = $x;  
  
  // $pdf->SetX(160);
  // $pdf->Cell(0,6,"Subtotal",0,1);

	// Line items
  foreach($lineitems as $line) {
    if($line->total) {

      $li_quant =  formatFloat($line->quantity);  // quantity, format as needed
      $li_amount = number_format($line->amount, 2);  // amount without tax
      $li_discount = print_discount($line->discount, $currency_symbol);  // discount % or total
      $li_tax    = number_format($line->tax, 2);  // tax
      $li_total  = number_format($line->total, 2);  // amount with tax, shipping, and handling      
      $li_desc = $line->description ?: phrase(get_object('lineitem_type', $line->type, 'name'), CAPITALIZE);
      if(!$li_desc) { // no description - use type
	      $type_desc = get_lineitem_type_name($line->type);
	      $li_desc = phrase($type_desc, CAPITALIZE);
      }
      $li_desc = add_breaks($li_desc, 100-15, 10);

		  $pdf->SetX($left_x);
		  $pdf->SetFont('DejaVu','',10);
      
      //$pdf->Cell(100,6, $li_desc,0,0);
      
      //$pdf->SetLeftMargin(15);
      //$pdf->SetRigthMargin(100);
      $pdf->Write(5, $li_desc);
      $x = $first_col;

      
      if($show_quantity) {  
        $pdf->SetFont('DejaVu','',10);
        $pdf->SetX($x);
        $pdf->Cell($w,6,"$li_quant",0,0, 'R');
        $x+=$w;
      }

      if($show_quantity || $show_discount || $show_tax) {
        $pdf->SetX($x);
        $pdf->Cell($w,6,"$currency_symbol $li_amount",0,0, 'R');
        $x+=$w;
      }
      
      if($show_discount) {
  		  $pdf->SetX($x);
  		  $pdf->Cell($w,6,"$li_discount",0,0, 'R');
        $x+=$w;
      }

      if($show_tax) {
  		  $pdf->SetX($x);
  		  $pdf->Cell($w,6,"$currency_symbol $li_tax",0,0, 'R');
        $x+=$w;
      } else if($show_tax_of_which) {
  		  $pdf->SetX($x);
  		  $tow = $line->tax_percentage > 0 ? formatFloat($line->tax_percentage).'%' : '';
  		  $pdf->Cell($w,6,$tow,0,0, 'R');
        $x+=$w;
      }

		  $pdf->SetX($x);
		  $pdf->Cell($w,6,"$currency_symbol $li_total",0,1, 'R');
      //$x+=$w;

      
      $n++;
    }
  }

  $pdf->Ln(5);
                 
	// Total
	$start_x = $max_x - 3*$w;
  foreach($price as $k => $v) {
    if($k=='__line') {
			$pdf->Line($start_x, $pdf->getY(), $start_x + 4*$w, $pdf->getY());
			$pdf->Ln(5);
			
    } else if($v) {
		  $pdf->SetX($start_x);    
		  $pdf->SetFont('DejaVu','B',10);
      $pdf->Cell(3*$w,6,"$k:",0,0);
		  $pdf->SetFont('DejaVu','',10);
		  $pdf->SetX($max_x);    
      $pdf->Cell($w,6,$v,0,1, 'R');
    }
  }
  
  if(count($payments)) {
    $pdf->Ln(5);
  
    // Payment Header
    $pdf->SetFont('DejaVu','B',10);
    $pdf->SetX($left_x);
    $pdf->Cell($first_pay_col - $left_x,6,"Payments",0,0);

    $x = $first_pay_col;
    $pdf->SetX($x);
    $pdf->Cell($w,6,"Date",0,0);
    $x+=2*$w;

    $pdf->SetX($x);
    $pdf->Cell($w,6,"Method",0,0);
    $x+=$w;

    //$pdf->SetX($x);
    //$pdf->Cell($w,6,"Status",0,0);
    $x+=$w;

    $pdf->SetX($x);
    $pdf->Cell($w,6,"Amount",0,1, 'R');
  
    $p_desc_max = 45;
    
  	// Payments
    foreach($payments as $payment) {
      
    	$pay_date         = sql2human($payment->created);
      $currency_symbol  = get_currency_symbol($payment->currency);    
      $pay_amount       = number_format($payment->amount, 2);  // amount without tax    
      $pay_type         = get_select_value2('payment_type', $payment->payment_type);
      // $pay_status       = get_select_value2('payment_status', $payment->payment_status);
      $pay_status       = get_select_value2('payment_status', $payment->payment_status);
      
      $rcv_currency     = $payment->payment_currency;
      $rcv_amount       = number_format($payment->payment_gross, 2);      
      $pay_amount_str = ($rcv_currency && $rcv_amount && $rcv_currency != $payment->currency) ? get_currency_symbol($rcv_currency)." $rcv_amount => $currency_symbol $pay_amount" : "$currency_symbol $pay_amount";
      
      $p_desc = $payment->description;
      if(strlen($p_desc) > $p_desc_max) $p_desc = substr($p_desc, 0, $p_desc_max).'...';
      //$pay_desc = $line->description;
      //if(!$pay_desc) { // no description - use type
      //  $type_desc = get_lineitem_type_name($line->type);
      //  $pay_desc = phrase($type_desc, CAPTIALIZE);
      //}
      $x = $left_x;
      
      $pdf->SetFont('DejaVu','',10);
      $pdf->SetX($x);
      $pdf->Cell($first_pay_col - $left_x,6,$p_desc,0,0);

      $x = $first_pay_col;
      $pdf->SetX($x);
      $pdf->Cell($w,6,$pay_date,0,0);
      $x+=2*$w;

      $pdf->SetX($x);
      $pdf->Cell($w,6,$pay_type,0,0);
      $x+=$w;

      //$pdf->SetX($x);
      //$pdf->Cell($w,6,$pay_status,0,0);
      $x+=$w;

      $pdf->SetX($x);
      $pdf->Cell($w,6,$pay_amount_str,0,1, 'R');
      	 
      $n++;
      
    }

    $pdf->Ln(5);
    $paid_ar['__line'] = '';
    //$paid_ar['Total'] = $currency_symbol." ".number_format($total, 2);
    $paid_ar[$paid_text]  = $currency_symbol." ".number_format($paid, 2);
    $paid_ar['Balance due'] = $currency_symbol." ".number_format($balance, 2); 

    $start_x = $max_x - 3*$w;
    foreach($paid_ar as $k => $v) {
      if($k=='__line') {
        $pdf->Line($start_x, $pdf->getY(), $start_x + 4*$w, $pdf->getY());
        $pdf->Ln(5);
        
      } else if($v) {
        $pdf->SetX($start_x);    
        $pdf->SetFont('DejaVu','B',10);
        $pdf->Cell(3*$w,6,"$k:",0,0);
        $pdf->SetFont('DejaVu','',10);
        $pdf->SetX($max_x);    
        $pdf->Cell($w,6,$v,0,1, 'R');
      }
    }


  }


  $pdf->Output($filename, "F");
  
	return $path;
}

function hex2rgb($hex) {
  $color = str_replace('#','',$hex);
  $rgb = array('r' => hexdec(substr($color,0,2)),
               'g' => hexdec(substr($color,2,2)),
               'b' => hexdec(substr($color,4,2)));
  return $rgb;
}

function rgb2cmyk($var1,$g=0,$b=0) {
   if(is_array($var1)) {
     $r = $var1['r'];
     $g = $var1['g'];
     $b = $var1['b'];
   }
   else $r=$var1;
   $cyan    = 255 - $r;
   $magenta = 255 - $g;
   $yellow  = 255 - $b;
   $black  = min($cyan, $magenta, $yellow);
   $cyan    = @(($cyan    - $black) / (255 - $black)) * 255;
   $magenta = @(($magenta - $black) / (255 - $black)) * 255;
   $yellow  = @(($yellow  - $black) / (255 - $black)) * 255;
   return array('c' => $cyan / 255,
               'm' => $magenta / 255,
               'y' => $yellow / 255,
               'k' => $black / 255);
}

// adds page break to text to fit given $width
function add_breaks($text, $width, $font_size=10) {
  // font size 10 Arial: 50 chars on 85 pixels
  $cpp = 50/85;  
  $len = strlen($text);
  $max = $cpp * $width;
  $result = $text;
  $start = 0;
  //echo("max chars for $width = $max  <br>");
  while($len > $max) {
    $head = substr($text, $start, $max);
    $tail = substr($text, $max);
    $result = $head."\n".$tail;
    $text = $tail;
    $len = strlen($text);
  }
  
  return $result;
}



class PDF extends tFPDF
{
  
  function SetCol($col)   
  {
    // Set position at a given column
    $this->col = $col;
    $x = 15 + $col*65;
    $this->SetLeftMargin($x);
    $this->SetX($x);
  }  
  
  //Page header
  function Header()
  {

    
		// margins
		$this->setMargins(15,15);

		// title
    $this->SetFont('DejaVu','B',22);
    $this->SetTextColor(0,0,0);
    $this->SetLineWidth(0.2);
    $this->Cell(190,5,$this->title,0,2,'C');
    $this->Ln(4);
	 
	  if($this->logo) $this->Image($this->logo,16,19, 0, 24);


    // "for"
    $this->SetFont('DejaVu','B',16);
    $this->SetX(110);
    //$this->Cell(20,24,"By:",0,0,'C');
    
    		
		// box with name/address of legal entity
    $this->SetFont('DejaVu','B',10);
    $this->SetX(135);
    list($r,$g,$b) = color_hex2dec($this->theme_color);
    $this->SetFillColor($r,$g,$b); // 9c6b00
    list($r,$g,$b) = color_hex2dec($this->text_color);
    $this->SetTextColor($r,$g,$b);
    $this->MultiCell($w,6,$this->company,0,'R',true);

		// logo    384 / 145
    $this->SetDrawColor(0,0,0);
		$this->Rect(16,19,179,24);

    $this->Ln(10);
    
    //Save ordinate
    $this->y0=$this->GetY();

    $this->SetAutoPageBreak(true, 40);
  }

  //Page footer
  function Footer()
  {

		// end text
    $this->SetY(-40);
    $this->SetX(0);
    $this->SetFont('DejaVu','B',8);
    $this->SetTextColor(0,0,0);
    $this->SetLineWidth(0.2);
    $this->MultiCell(210,4,$this->end_text,0,'C');
    $this->Ln(3);

		// footer box 
    $this->SetFont('DejaVu','B',10);
    $this->SetX(15);
    list($r,$g,$b) = color_hex2dec($this->theme_color);
    $this->SetFillColor(hexdec('9c'),hexdec('6b'),hexdec('00')); // 9c6b00
    $this->SetFillColor($r,$g,$b); // 9c6b00

		// footer text
    $this->SetFont('DejaVu','B',10);
    list($r,$g,$b) = color_hex2dec($this->text_color);
    $this->SetTextColor($r,$g,$b);
    $this->Cell($w,7,$this->footer_text,1,1,'C',true);

	  // page number 
    list($r,$g,$b) = color_hex2dec($this->text_color_fg);
    $this->SetTextColor($r,$g,$b);
    $this->SetY(-10);
    $this->SetFont('DejaVu','I',8);
    $this->Cell(0,10,'Page '.$this->PageNo().'/{nb}',0,0,'C');


  }
  
  
  // Print cells: bold k + v
  function PrintRow($k, $v)
  {
    if(!trim($v)) return;
    $this->SetFont('DejaVu','B',10);
    $this->Cell(40,6,"$k:",0,0);
    $this->SetFont('DejaVu','',10);
    $this->Cell(0,6,"$v",0,1);
  }
}	


<?php
function reeleezee_export($legal_entity_id, $obj_type, $search, $check_validity=0, $limit=0, $send=true, $echo=false) {
  global $xml_dir;
  if(!$legal_entity_id) return json_error_object("Reeleezee export: Legal entity ID required");
  $legalObj = get_object('legal_entity', $legal_entity_id);
  $client_id = $legalObj->client_id;
  $username = $legalObj->reeleezee_username;
  $password = $legalObj->reeleezee_password;
  $messages = $errors = [];
  
  if(!$legalObj)  return json_error_object("Reeleezee export: Legal entity not found for id=$legal_entity_id");
  if(!$username) return json_error_object("Reeleezee export: No reeleezee username for $legalObj->name");
  if(!$password)  return json_error_object("Reeleezee export: No reeleezee password for $legalObj->name");
  if(!$client_id)  return json_error_object("Reeleezee export: No client ID for $legalObj->name");
   
  if($_SESSION['dev']) {
    $echo = true;  
    $username = 'amsterdam';
    $password = 'stront91!';
  }
  if(isset($_GET['echo'])) $echo=true;
  
  //$echo = true;
  //$check_validity = true;

  $now = now();
  print_log("{$now}: export {$obj_type} ".json_encode($search), 'reeleezee', LOG_LEVEL_ALWAYS);
  
  $url = "https://portal.reeleezee.nl/ezVersionService/externalServices/TaxonomyService/TaxonomyWebService.asmx";
  $wsdl = "https://portal.reeleezee.nl/ezVersionService/externalServices/TaxonomyService/taxonomywebservice.asmx?WSDL";

  $output = "Starting Reelezee $obj_type Export for Legal Entity $legal_entity_id ($legalObj->name)".html_break(); 

  $table = get_object_table($obj_type);
  
  //return json_error_object("OK: obj-type={$obj_type}");
  
  //$search['client_id'] = $client_id; 
  if($obj_type == 'org') {
    //$search[$table.'.client_id'] = $client_id;
    $search[DB_LOGICAL_OR] = array('client_id' => $client_id, $table.'.id' => 100); // 100 is dummy org for particulier 
  } else {
    $search['client_id'] = $client_id;
  }
  if($obj_type == 'invoice') {
    $search[$table.'.legal_entity_id'] = $legal_entity_id; 
    unset($search['client_id']); // for now
  }
    //dump($search);
  
  $objs = find_objects($obj_type, $search, array('sort' => $sort, 'limit' => $limit));
  $count = count($objs);
  if(!$count) {
    $output .= "Reeleezee export: No objects matched search for $obj_type.".html_break();                                       
    dump($search);
  } else {  
    $output .= "Reeleezee export: $count $obj_type objects matched search.".html_break();                                       
    
    switch($obj_type) {
      case 'invoice':
        list($inv_xml, $error) = reeleezee_invoices($objs);
        if($error) return json_error_object($error);
        $data .= $inv_xml;  
        break;
      case 'org':
        list($cust_xml, $error) = reeleezee_customer_list($objs);
        
        //die("inv objs:".dump($objs,true)." xml:".form_text('xmlDocument', print_r($cust_xml, true), array('rows' => 30, 'cols' => 150)));
        if($error) return json_error_object($error);
        $data .= $cust_xml;
        break;
      default:
        return json_error_object("Reeleezee export: Unsupported object type $obj_type");
        break;
    }
    $xml = reeleezee_xml($data);
    $ts = stripdate(now());
    $base_dir = "$xml_dir/Reeleezee/$client_id/$legal_entity_id/";
    if(!is_dir($base_dir) && !mkdir($base_dir, 0777, true))  return json_error_object("Failed to make $base_dir");
    
    $filename = "$base_dir/$obj_type-$ts.xml";
    $output .= form_text('xmlDocument', $xml, array('rows' => 10, 'cols' => 150));
    $len = strlen($xml);
    if($send) {
      if($len) {
        $fields['userName'] = $username;
        $fields['password'] = $password;
        $fields['xmlDocument'] = $xml;
        $fields['checkValidityOnly'] = $check_validity ? 1 : 0;

        $output .= "Sending $len bytes to $url with username $username".html_break();
        $client = new SoapClient($wsdl, array("trace" => 1, "exception" => 0)); 
        $client->__setLocation($url);

        try {
          $result = $client->Import($fields);
        } catch (Exception $e) {
          $error = $e->getMessage();
          $output .= $error;
          $xml_result = new stdClass;
          $error = $xml_result->ImportResult->MaxMessage;
          
          return json_error_object("Soap error: {$error} (Could be too many invalid attempts)");
        }
        
        //$client = new SoapClient($wsdl, array("trace" => 1, "exception" => 0)); 
        //$client->__setLocation($url);
        //$result = $client->Import($fields);

        $output .= html_h2("Result").form_text('xmlDocument', print_r($result, true), array('rows' => 30, 'cols' => 150));
      } else {
        $output .= "Document is empty - not sending.".html_break();                                       
      }
    } else {
      $output .= "Send is false - not sending.".html_break();                                       
    }
  }
  
  $b = write_file($filename, $xml.print_r($result, true));
  $output .= "Wrote $b bytes to $filename".html_break();
  if($echo) echo($output);
  $xml_result = new SimpleXMLElement($result->ImportResult);  
  if(!$xml_result) return json_error_object("reeleezee_export_invoice: Could not parse xml");

  print_log("xml_result: ".dump($xml_result,true), 'reeleezee', LOG_LEVEL_TEST);

  
  $success = $xml_result->ImportResult['Succeeded'] == 'true' ? 1 : 0;
  
  $xml_result = xml2obj($xml_result); /**  basically a deep array to object */  
  if(!$xml_result || !is_object($xml_result)) return json_error_object("reeleezee_export_invoice: Could not parse xml (2)");

  if(!$success) {
    $errors[] = "Reeleeze export error:";
    $more_errors = (array) $xml_result->ImportResult->MessageList->Message;
    if(is_array($more_errors) && count($more_errors)) $errors = array_merge($errors, $more_errors);
    if($error = $xml_result->ImportResult->MaxMessage) $errors[] = "MaxMessage: {$error}";
  }  

  $now = now();
  $result_str = $success ? "OK" : "ERROR";
  print_log("{$now}: export {$obj_type} result: $result_str".($errors ? dump($errors,true) : ''), 'reeleezee', LOG_LEVEL_ALWAYS);
  print_log("xml_result: ".dump($xml_result,true), 'reeleezee', LOG_LEVEL_TEST);
  
  $result = new stdClass;
  $result->success = $success;
  $result->xml = $xml_result;
  $result->message = $messages; // not in use
  $result->error = $errors; // not in use
  
  /** delete after testing */
  $result->xml_sent = $xml; // not in use
  $result->output = $output; // not in use
  
  
  return $result;
  //return array($success, $xml_result); // return success = true in result (or number of updates?)  
}

function reeleezee_xml($data) {
  if(!$data) return '';
  $ts = date('c');
  $xml = '<?xml version="1.0" encoding="utf-8"?>
<Reeleezee version="1.17" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.reeleezee.nl/taxonomy/1.17 taxonomy\1.17\Reeleezee.xsd" xmlns="http://www.reeleezee.nl/taxonomy/1.17">
	<Import>
		<ExportInfo>
			<Name>Reeleezee export</Name>
			<Source>Apartments-for-rent</Source>
			<CreateDateTime>'.$ts.'</CreateDateTime>
		</ExportInfo>
'.$data.'
	</Import>
</Reeleezee>
';
  return $xml;
}


function reeleezee_invoices($invObjs) {
  if(!count($invObjs)) return '';
  foreach($invObjs as $invObj) {
    list($xml, $error) = reeleezee_invoice($invObj);
    if($error) return ['', $error];
    $list .= $xml;
  }
  $xml = "
    <SalesInvoiceList>
$list
		</SalesInvoiceList>
";
  return [$xml, ''];
}

function reeleezee_invoice($invObj) {
  $nil = 'xsi:nil="true"';
  $tax_incl = $invObj->tax_included ? 'true' : 'false';
  $discount = reeleezee_discount($invObj->discount);  
  $invoice_id = $invObj->invoice_id;
  if(!$invoice_id) return [0, "Skipping invoice $invObj->id with missing invoice_id"]; 
  $inv_id = numerify($invoice_id);
  $contact_id = $nil;
  $customer_id = $invObj->org_id;
  if($customer_id && !get_object('org', $customer_id)) return [0, "Skipping invoice $invObj->id with unknown customer $customer_id"];
  
  if(!$customer_id) $customer_id = DUMMY_CUSTOMER_ID; // Reeleezee requires one
  
  $customer_id_str = $customer_id ? "ID=\"$customer_id\"" : $nil;
  $address = reeleezee_address($invObj, 'RecipientAddress', true);
  
  $date = strip_time($invObj->invoice_date) ?: today();
  $duedate = strip_time($invObj->duedate) ?: add_day(today(), 14);
  $bookdate = strip_time($invObj->start_date) ?: $date;
  
  //$date = "8378--348";
  
  //$status = $invObj->status == INV_STATUS_DRAFT ? 'tentative' : 'open';
  $status = $invObj->status == 'open';
  $status = 'open';
  
  $footer = htmlspecialchars("Payment due within 14 days, stating invoice number and client name\n");
  $bottom = $nil;
  $lineitems = reeleezee_lineitems($invObj, $bookdate);
  $header_xml = reeleezee_tag_or_nil('Header', $invObj->concerning);
  $xml = "
			<SalesInvoice ReferenceNumber=\"$inv_id\">
				<ReferenceNumber>$inv_id</ReferenceNumber>
				<ContactPersonReference $contact_id />
				<CustomerReference $customer_id_str />
$address
				<LanguageCode xsi:nil=\"true\" />
				<DocumentDate>$date</DocumentDate>
				<BookDate>$bookdate</BookDate>
				<PaymentDueDate>$duedate</PaymentDueDate>
				<IsVatIncludedInPrice>$tax_incl</IsVatIncludedInPrice>
				<Status>$status</Status>
				$header_xml
				<Footer>$footer</Footer>
				<BottomText $bottom />
				<PaymentReference xsi:nil=\"true\" />
$discount
$lineitems
			</SalesInvoice>

  ";
  return [$xml, ''];
}

function reeleezee_lineitems($invObj, $date='') {
  $liObjs = find_objects('inv_lineitem', array('inv_id' => $invObj->id));
  if(!count($liObjs)) return '';
  foreach($liObjs as $liObj) $list .= reeleezee_lineitem($liObj, $date); 
  $xml = "
				<LineList>
$list
				</LineList>
  ";
  return $xml;
}

function reeleezee_lineitem($liObj, $date='') {
  $nil = 'xsi:nil="true"';
  
  if(!$date) $date = strip_time($liObj->created);
  $vat_code = reeleezee_tax_code($liObj);

  $unit = 'ST';
  $discount = reeleezee_discount($liObj->discount);
  $account = $nil;
  $product = 'Code="1"';
  $product = $nil;
  $li_desc = $liObj->description ?: phrase(get_object('lineitem_type', $liObj->type, 'name'), CAPITALIZE);
  $desc_xml = reeleezee_tag_or_nil('Description', $li_desc);
  $account_no = $liObj->account_number;
  $account = $account_no > 0 ? "<AccountReference Number=\"$account_no\" />" : '';                                                                                                                                                                      
  //          <AccountReference $account />
  //						<ProductReference $product />
  $amount = round($liObj->amount, 2);
    
  $xml = "
					<Line>
						<Date>$date</Date>
						$desc_xml
						<Quantity>".clean_num($liObj->quantity)."</Quantity>
						<Unit>$unit</Unit>
						<Price>{$amount}</Price>
						<VatCode>$vat_code</VatCode>
						$discount
						$account
						<EmployeeReference xsi:nil=\"true\" />
						<DepartmentReference xsi:nil=\"true\" />
						<ProjectReference xsi:nil=\"true\" />
						<PageBreak>false</PageBreak>
					</Line>
  ";
  return $xml;
}

// todo: use tax_category
function reeleezee_tax_code($liObj) {
  $tax_int = round($liObj->tax_percentage) ?: 0;
  switch($tax_int) {
    case '19':
      return 'H';
    case '6':
      return 'L';
    case '0':
      return 'N';
    default:
      return 'N';    
  }
}

function reeleezee_discount($discount) {
  $nil = "<Discount xsi:nil=\"true\" />";

  if(!$discount) return $nil;
  list($abs, $perc) = split_discount($discount);
  if($abs) {
    return "
        <Discount>
				  <Amount>$abs</Amount>
        </Discount>
";
  } elseif ($perc) {
    return "
        <Discount>
				  <Percentage>$perc</Percentage>
        </Discount>
";
  } else {
    return $nil;
  }
}

function reeleezee_customer_list($objs) {
  if(!count($objs)) return '';
  foreach($objs as $obj) {
    list($cust_xml, $error) = reeleezee_customer($obj);
    //die("gh2 obj=".dump($obj,true)." xml=".form_text('foo', $cust_xml));
    
    if($error) return ['', $error];
    $list .= $cust_xml;
  }
  $xml = "
    <CustomerList>
$list
		</CustomerList>
";

  return [$xml, ''];
}

//				<TaxDepositLHNumber xsi:nil=\"true\" />
//				<TaxDepositOBNumber xsi:nil=\"true\" />
//				<TaxDepositICLNumber xsi:nil=\"true\" />
//				<BranchReference xsi:nil=\"true\" />

function reeleezee_customer($obj) {
  $addObj1 = find_object('address', array('address_parent_type' => 'org', 'address_parent_id' => $obj->id, 'address_type' => ADDRESS_TYPE_PHYSICAL));
  $addObj2 = find_object('address', array('address_parent_type' => 'org', 'address_parent_id' => $obj->id, 'address_type' => ADDRESS_TYPE_DELIVERY));
  $addObjs = array_filter(array($addObj1, $addObj2));
  
  $address_list = reeleezee_address_list($addObjs);  

  $full_name_xml = reeleezee_tag('FullName', trim($obj->name), 50);
  $search_name_xml = reeleezee_tag('SearchName', trim($obj->name), 50);
  //$language_xml = reeleezee_tag_or_nil('LanguageCode'              , $obj->language);
  $phone_xml    = reeleezee_tag_or_nil('PhoneNumber'               , clean_phonenumber($obj->phone));
  $fax_xml      = reeleezee_tag_or_nil('FaxNumber'                 , clean_phonenumber($obj->fax));
  $email_xml    = reeleezee_tag_or_nil('EmailAddress'              , $obj->email_address);
  $web_xml      = reeleezee_tag_or_nil('WebsiteAddress'            , $obj->web_address);
  $comment_xml  = reeleezee_tag_or_nil('Comment'                   , $obj->comment);
  $cocn_xml     = reeleezee_tag_or_nil('ChamberOfCommerceNumber'   , $obj->coc_number);
  $cocc_xml     = reeleezee_tag_or_nil('ChamberOfCommerceCity'     , $obj->coc_city);
  $tax_xml      = reeleezee_tag_or_nil('FiscalIdentificationNumber', $obj->tax_number);
  $bank_xml     = reeleezee_tag_or_nil('BankAccountNumber'         , $obj->bank_account_number);
  $default_address = $obj->default_address ==  ADDRESS_TYPE_DELIVERY ? 'Delivery' : 'Office';
  $xml = "
			<Customer ID=\"$obj->id\">
        $full_name_xml
        $search_name_xml
				<Code xsi:nil=\"true\" />
				<DefaultAddress>$default_address</DefaultAddress>
        $language_xml
        $phone_xml
        $fax_xml
        $web_xml
        $comment_xml
        $cocn_xml
        $cocc_xml
        $bank_xml
$address_list				  
			</Customer>
";

  return [$xml, ''];
}

function reeleezee_tag($tag, $val, $max=0) {
  if($max && strlen($val) > $max) $val = substr($val, 0, $max);
  return "<$tag>".utf8_encode(htmlspecialchars($val))."</$tag>";
}

function reeleezee_tag_or_blank($tag, $val, $max=0) {
  return strlen($val) ? reeleezee_tag($tag, $val, $max) : "";
}

function reeleezee_tag_or_nil($tag, $val, $max=0) {
  $val = trim($val);
  return strlen($val) ? reeleezee_tag($tag, $val, $max) : "<$tag xsi:nil=\"true\" />";
}
  
function reeleezee_address_list($objs) {
  if(!count($objs)) return '';
  foreach($objs as $obj) $list .= reeleezee_address($obj, 'Address', false, true); 
  $xml = "
    <AddressList>
$list
		</AddressList>
";
  return $xml;
}

function reeleezee_address($addObj, $tag='Address', $show_name=true, $show_type=false) {
  $nil = "xsi:nil=\"true\"";
  $name_str = get_name_string($addObj);
  // $street = implode(', ', array_filter(array($addObj->address1, $addObj->address2)));

  $name_xml    = $show_name ? reeleezee_tag_or_nil('Name', $name_str, 50) : '';
  $street_xml  = reeleezee_tag_or_nil('Street', $addObj->address1, 30);
  $number_xml  = reeleezee_tag_or_nil('Number', $addObj->address2, 50);
  $zipcode_xml = reeleezee_tag_or_nil('Zipcode', $addObj->zip, 10);
  $city_xml    = reeleezee_tag_or_nil('City', $addObj->city, 30);
  if($addObj->country == 'EU') $addObj->country = '';
  $country_xml = reeleezee_tag_or_blank('CountryCode', $addObj->country);

  if($show_type) {
    $type = $addObj->address_type == ADDRESS_TYPE_DELIVERY ? 'Type="Delivery"' : 'Type="Office"'; 
  } 
  $xml = "
        <$tag $type>
					$name_xml
					$street_xml 
					$number_xml 
					$zipcode_xml
					$city_xml
					$country_xml
				</$tag>
  ";
  return $xml;
}

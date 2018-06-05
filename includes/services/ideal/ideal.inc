<?php

require_once('iDeal/ideal.class.php');

// returned from Bank, check payment is ok 
// also used by ideal/report.php 

/** unsused - automatic comment added on  */
function ideal_validate($inputVars) {
  $errors = array();  
  if(!($inputVars['transaction_id'] || $inputVars['bank_id'])) $errors[] = phrase("please_select_your_bank", CAPITALIZE);
  return $errors;  
}


// Euros only
function ideal_return($partner_id, $tx_id, $amount, $test=-1) {
  if($test == -1) $test == $_SESSION['dev'] || $_SESSION['test'] ? true : false; // default test mode to true if dev site

  //$ideal_error = "Dummy error";
  
  if($tx_id) {
  	$iDEAL = new iDEAL_Payment($partner_id);
	  if($test) $iDEAL->setTestMode();	
  	$iDEAL->checkPayment($tx_id);

  	// echo("checking tx $tx_id for partner $partner_id test=$test");
  	// dump($iDEAL);

	  //dump($iDEAL);
  	
  	if ($iDEAL->getPaidStatus() == true) {
  	  $guest = $iDEAL->getConsumerInfo();
      $guest_name = $guest['consumerName'];
      $guest_acct = $guest['consumerAccount'];
      $guest_city = $guest['consumerCity'];
      
  	  $actual_amount = $iDEAL->getAmount();
      $amount_cents = str_replace(array(',', '.'), '', $amount*100); // convert to cents, remove separators

      if($actual_amount != $amount_cents) {
        $ideal_error = "Amount mismatch. Paid: ".formatFloat($actual_amount/100, FORMAT_ALWAYS, '&euro;')." Due: ".formatFloat($amount_cents/100, FORMAT_ALWAYS, '&euro;');
      } else {          	      
        $ideal_message = "Transaction OK!".html_break();
        $ideal_message .= phrase("received", CAPITALIZE).' '.formatFloat($actual_amount/100, FORMAT_ALWAYS, '&euro;').': '." $guest_name ($guest_city). ".phrase('account_number', CAPITALIZE).": $guest_acct";
      }
  	} else {                           
      if($error_msg = trim($iDEAL->getErrorMessage())) $ideal_error = "Payment failed: ".$error_msg;
      else $ideal_error = $iDEAL->status;
  	}
  } else {
    $ideal_error = "Missing transaction ID";
  }
  
  return array($ideal_error, $ideal_message);
}

// creates iDeal payment - redirects to bank
/** unsused - automatic comment added on  */
function ideal_create($partner_id, $profile_key, $bank_id, $amount, $description, $return_url, $report_url, $test = -1) {
  $return_url .= "&partner_id=$partner_id&profile_key=$profile_key";
  $iDEAL = new iDEAL_Payment ($partner_id);
  if($test == -1) $test == $_SESSION['dev'] ? true : false; // default test mode to true if dev site
  if($test) $iDEAL->setTestMode();
  $iDEAL->setProfileKey($profile_key);

  $amount_cents = str_replace(array(',', '.'), '', $amount * 100); // convert to cents, remove separators
  
	if($iDEAL->createPayment($bank_id, $amount_cents, $description, $return_url, $report_url)) {
	  $tx_id = $iDEAL->getTransactionId();
		$result->transaction_id = $tx_id;
    //dump($iDEAL);
  
    //$tx_data[]  
    //list($id, $error) = add_object('transaction', $tx_array);
		/* Hier kunt u de aangemaakte betaling opslaan in uw database, bijv. met het unieke transactie_id
		   Het transactie_id kunt u aanvragen door $iDEAL->getTransactionId() te gebruiken. Hierna wordt 
		   de consument automatisch doorgestuurd naar de gekozen bank. */
		$resArray['bank_url'] = $resArray['redirect'] = $iDEAL->getBankURL();
		// echo("redirecting to $bank_url");
		// redirect($bank_url);
	} else {
		$resArray['error'] = $iDEAL->getErrorMessage();
	}
	return $resArray;
}

// returns list of banks
/** unsused - automatic comment added on  */
function ideal_prepare($partner_id, $test = -1) {
  if($test == -1) $test == $_SESSION['dev'] ? true : false; // default test mode to true if dev site
  //echo("Running ideal_prepare. test mode = $test<br>");      
  if (!in_array('ssl', stream_get_transports())) {
  	echo "<h1>Foutmelding</h1>";
  	echo "<p>Uw PHP installatie heeft geen SSL ondersteuning. SSL is nodig voor de communicatie met de Mollie iDEAL API.</p>";
  	exit;	
  }
  
  $iDEAL = new iDEAL_Payment ($partner_id);
  if($test) $iDEAL->setTestMode();
  $bank_array = $iDEAL->getBanks();
  if ($bank_array == false) echo '<p>Er is een fout opgetreden bij het ophalen van de banklijst: ', $iDEAL->getErrorMessage(), '</p>';
  return $bank_array;  
}

// /** unsused - automatic comment added on  */
function ideal_report($partner_id, $tx_id, $amount, $test=-1) {
//   if($test == -1) $test = $_SESSION['dev'] ? true : false; // default test mode to true if dev site
// 
// 	$iDEAL = new iDEAL_Payment ($partner_id);
// 	if($test) $iDEAL->setTestMode();	
// 	
// 	$iDEAL->checkPayment($tx_id);
// 
// 	if ($iDEAL->getPaidStatus() == true) 
// 	{
// 		/* De betaling is betaald, deze informatie kan opgeslagen worden (bijv. in de database).
// 	   	   Met behulp van $iDEAL->getConsumerInfo(); kunt u de consument gegevens ophalen (de 
// 		   functie returned een array). Met behulp van $iDEAL->getAmount(); kunt u het betaalde
// 		   bedrag vergelijken met het bedrag dat afgerekend zou moeten worden. */
// 	  $guest = $iDEAL->getConsumerInfo();
//     $guest_name = $guest['consumerName'];
//     $guest_acct = $guest['consumerAccount'];
//     $guest_city = $guest['consumerCity'];
// 
//   	$actual_amount = $iDEAL->getAmount();
//     $amount_cents = str_replace(array(',', '.'), '', $amount*100); // convert to cents, remove separators
// 
//     if($actual_amount != $amount_cents) {
//       $ideal_error = "Amount mismatch. Paid: ".formatFloat($actual_amount/100, FORMAT_ALWAYS, '&euro;')." Due: ".formatFloat($amount_cents/100, FORMAT_ALWAYS, '&euro;');
//     } else {          	  
//       $ideal_message = "Transaction OK!".html_break();
//       $ideal_message .= phrase("received", CAPITALIZE).' '.formatFloat($actual_amount/100, FORMAT_ALWAYS, '&euro;').': '." $guest_name ($guest_city). ".phrase('account_number', CAPITALIZE).": $guest_acct";
//     }
// 	} else {
//     if($error_msg = trim($iDEAL->getErrorMessage())) $ideal_error = $error_msg;
//     else $ideal_error = "Unknown error";
// 	}
//   return array($ideal_error, $ideal_message);
// 
// }

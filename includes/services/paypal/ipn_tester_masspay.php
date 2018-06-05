<? include_once('../include/set_env.inc');?>
<?

$data_str = 
"[txn_type] => masspay
[payment_gross_1] => 233.00
[payment_date] => 06:02:59 Apr 14, 2010 PDT
[last_name] => Larsen
[mc_fee_1] => 1.00
[masspay_txn_id_1] => 4XR1459403269552D
[receiver_email_1] => guest@kjakman.com
[residence_country] => US
[verify_sign] => AZbd2mEyNAqTPxX31JNEDGkUVd.lAIKkjBR9DruFgQBok58oqTslcR3J
[payer_status] => verified
[test_ipn] => 1
[payer_email] => test@kjakman.com
[first_name] => Kjetil
[payment_fee_1] => 1.00
[payer_id] => 7UXA4LZABE5GN
[payer_business_name] => Art Apartments - test
[payment_status] => Processed
[status_1] => Completed
[mc_gross_1] => 233.00
[charset] => windows-1252
[notify_version] => 2.9
[mc_currency_1] => USD
[unique_id_1] => Res:35114,Pay:1103
[ipn_test] => 1
[tx_gateway] => PAYPAL
[txn_res] => VERIFIED";

$dataAr = split("\n", $data_str);
foreach($dataAr as $line) {
  $line = trim($line);
  list($k, $v) = explode(' => ', $line);
  $k = ltrim($k, '[');
  $k = rtrim($k, ']');
  $data[$k] = trim($v);
}
//dump($data);

$qs = array2qs($data);
echo($qs);
?>
  
  
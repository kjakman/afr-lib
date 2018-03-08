<?php
function ReverseDNSLookup($ip)
{
  $host = gethostbyaddr($ip);
  $tld = substr(strrchr(strtoupper($host), "."), 1);
  return array($host, $tld);
}

function MaxMindIPcountryLookup($ip, $license_key) {
  $query = "http://67.15.94.80:8010/a?l=" . $license_key . "&i=" . $ip;
  $url = parse_url($query);
  $host = $url["host"];
  $path = $url["path"] . "?" . $url["query"];
  $timeout = 1;
  $fp = fsockopen ($host, 8010, $errno, $errstr, $timeout);
  if ($fp) {
    fputs ($fp, "GET $path HTTP/1.0\nHost: " . $host . "\n\n");
    while (!feof($fp)) {
      $buf .= fgets($fp, 128);
    }
    $lines = split("\n", $buf);
    $country = $lines[count($lines)-1];
    fclose($fp);
  } else {
    return "NL";
  }
return $country;
}

$disallowed_cc = array("NL", "NOT_FOUND", "INVALID_LICENCE_KEY"); // list of banned countries
$remote_addr = $_SERVER['REMOTE_ADDR'];

list($host, $tld) = ReverseDNSLookup($remote_addr);
$_SESSION['remote_dns']=$host;
$_SESSION['remote_tld']=$tld;


if($_SESSION['country_code']=="" || isset($_GET['iplookup'])) { // only do this if country code is not already set in session
  //$license_key="kIwID1rw9syq";
  $license_key="DqTBU0aYl1gC";

  $_SESSION['remote_addr']= $remote_addr = $_SERVER['REMOTE_ADDR'];
  //$allowed_ips = array('84.41.133.205' /*kj */, '82.215.30.167' /* office */, '80.56.122.2' /* office */, , '83.117.136.186' /* richard */, '62.163.47.23' /* escape */, '217.149.211.214' /*ALEC*/, '212.96.76.86' /* charda*/,  '146.50.209.248' /* thomas wi-fi */);
  //$allowed_ips = array('83.117.136.186' /* richard */, '62.163.47.23' /* escape */, '217.149.211.214' /*ALEC*/, '212.96.76.86' /* charda*/,  '146.50.209.248' /* thomas wi-fi */);
  //$allowed_ips = array();
  $allowed_ips = array('82.170.177.86' /* office new */, '82.170.178.109' /* kj new */, '213.10.8.77' /* new escape 2*/, '80.56.214.14' /* new escape */, '84.41.133.205' /*kj */, '83.117.136.186' /* richard */, '62.163.47.23' /* escape */, '217.149.211.214' /*ALEC*/, '212.96.76.86' /* charda*/,  '146.50.209.248' /* thomas wi-fi */);
  $spider_ips = array('66.249.', '64.68.8', '64.233.1', '216.239.', '209.85.238.', '209.185.', '216.239.', '66.102' /* google */,
                       /* yahoo */
                       'xx82.170.' /* telfort */);

// http://www.webmasterworld.com/forum39/1502.htm
// 216.239.*
// 64.68.8*
// 66.102.
// 64.233.1
// 216.200.251.*


// Google Inc. EC12-1-GOOGLE (NET-64-68-80-0-1) 64.68.80.0 - 64.68.87.255
// Google Inc. GOOGLE-2 (NET-66-102-0-0-1) 66.102.0.0 - 66.102.15.255
// Google Inc. GOOGLE (NET-64-233-160-0-1) 64.233.160.0 - 64.233.175.255
// Google Inc. GOOGLE (NET-216-239-32-0-1) 216.239.32.0 - 216.239.63.255
// Google Inc. MFN-T324-216-200-251-112-29 (NET-216-200-251-112-1) 216.200.251.112 - 216.200.251.119 


  if(check_ip($remote_addr, $spider_ips)) { // don't do lookup for known IP
    //echo("found $remote_addr in spider IPs: ".implode(',', $spider_ips));
    $country_code = $_SESSION['country_code'] = "XX";
  } elseif(in_array($remote_addr, $allowed_ips)) { // don't do lookup for known IP
    //echo("found $remote_addr in known IPs:".implode(',', $allowed_ips));
    $country_code = $_SESSION['country_code'] = "NP";
//  } elseif($cc=db_getOne("SELECT country_code FROM weblog WHERE remote_ip='$remote_addr'")) { // don't look up if this user has been here before
//    //echo("found cc in DB - is $cc");
//    $country_code = $_SESSION['country_code'] = $cc;


  } else {
    $country_code = $_SESSION['country_code'] = MaxMindIPcountryLookup($remote_addr, $license_key); // lookup using web service
    //echo("rad: $remote_addr; tld: $tld mmcc: $country_code");
    if(strstr($country_code, "INVALID_LICENSE_KEY")) {
      //echo("got here:ilc");
      $country_code = $_SESSION['country_code'] = "NL";
    } elseif(strstr($country_code, "IP_NOT_FOUND")) {
      $country_code = $_SESSION['country_code'] = "NL";
      //$country_code = $_SESSION['country_code'] = $tld;
    }
  }
}

//if(in_array($_SESSION['country_code'], $disallowed_cc)) { // redirect to junk if not allowed
//  header("Location:foo.bar");
//}

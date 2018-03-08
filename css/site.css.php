<?php                                     
include_once('../include/set_env.inc');
if($_GET) {
  $page_id = $_GET['p'];
  $css_id  = $_GET['t'];
  $aff_id  = $_GET['aid'];
                                                           
  $siteObj = $_GET['s'] ? get_object('site', $_GET['s']) : null;
}          

if(!$siteObj) $siteObj = $_SESSION['siteObj'];         
if($siteObj) {                  
  $output = site_stylesheet($siteObj, $pageObj, array('aid' => $aff_id));
  $output .= site_stylesheet_overrides($siteObj);
  
  header("Content-type: text/css; charset: utf-8");   
  if($output) echo($output);  
  //if($db_output) echo($db_output);

  // add affiliate styles
  if($affObj= $_SESSION['aff_object']) {
    echo(read_file($siteObj->test_directory."afr/css/aff.css")); // generic affiliate css
    if($aff_css = $affObj->affiliate_css) echo(read_file($siteObj->test_directory."afr/css/aff/$aff_css")); // generic affiliate css
  }
}
   
?>


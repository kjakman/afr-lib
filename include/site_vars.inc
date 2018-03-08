<?php


function set_site_vars($siteObj, $affObjType=NULL, $aff_id=NULL, $use_partners_filters=false) {
  //global $pTypeArray;
  //echo("start: site_id=$site_id aff_id=$aff_id<br/>");
  //$afr = $siteObj->domain == 'apartments-for-rent.com';   

  if(!$siteObj) return;
  if($affObjType && $aff_id) {
    $affObj = get_object($affObjType, $aff_id);
    if($aff_site_id = $_SESSION['aff_site_id'] = $affObj->site_id) $siteObj = get_object('site', $aff_site_id); // old-style auto-build sub-site. Infer site from affiliate object
    $_SESSION['show_login']=$_SESSION['show_search']=$_SESSION['show_header']=$_SESSION['show_footer']=false;
  }
  //dump($siteObj->show_cities);
  // what to show
  $_SESSION['show_types']   = $types  = site_property_types($siteObj); //$siteObj->show_types ? explode(',', $siteObj->show_types) : array();
  $_SESSION['show_cities']  = $cities = site_cities($siteObj);
  $_SESSION['show_langs']   = $langs  = site_languages($siteObj); // $siteObj->languages ? explode(',', $siteObj->languages) : array($siteObj->default_language);

  //dump($cities);
  //dump($siteObj->property_filters);
  $_SESSION['property_filters'] = NULL;
  foreach($types as $type_id) { // todo: filters are not yet per type
    $type = get_object('property_type', $type_id, 'name');
    $_SESSION['property_filters'][$type] = $use_partners_filters && $siteObj->partner_filters ? filters2search($siteObj->partner_filters) : filters2search($siteObj->property_filters);
    $_SESSION['property_sort'][$type] = $siteObj->property_sort ?: 'priority DESC, book_direct DESC, created DESC';
  }
  
  
  if($id_filters = $_SESSION['property_filters'][$type]['id']) {
    if(!$siteObj->property_sort && !is_array($id_filters[1])) $psort = $_SESSION['property_sort'][$type] = 'field(id, '.implode(',', $id_filters).')'; // sort by id list
  }
  //dump($_SESSION['property_filters'][$type]);
  //echo("aid=$aff_id type=$type");dump($types);dump($_SESSION['property_filters']);dump($siteObj->property_filters);
  //if(isset($_GET['foo'])) {dump($_SESSION['property_filters']['boat']);dump($siteObj);dump(search2filters($_SESSION['property_filters']['boat']));}
  
  // colors, images
  $site_url = $siteObj->url; // e.g. www.afr.com

  $_SESSION['logo_url']               = "http://".$site_url.$siteObj->logo;
  $_SESSION['logo_url_small']         = "http://".$site_url.$siteObj->logo_small;

  $_SESSION['logo_url_secure']        = str_replace("http", 'https', $_SESSION['logo_url']);
  $_SESSION['logo_url_small_secure']  = str_replace("http", 'https', $_SESSION['logo_url_small']);


  //echo("sc=".dump($_SESSION['show_cities']));

  // affiliate
  $_SESSION['aff_id'] = $_SESSION['aid'] = $aff_id;

  $_SESSION['aff_object'] = $affObj;
  $_SESSION['aff_name']   = $affObj->affiliate_name;
  $_SESSION['aff_css']    = $affObj->affiliate_css;
  $_SESSION['aff_email']  = $affObj->email_address;

  $_SESSION['site_languages'] = $languages = explode(',', $siteObj->languages);
  $_SESSION['site_default_language'] = $siteObj->default_language ?: $languages[0];

}
?>
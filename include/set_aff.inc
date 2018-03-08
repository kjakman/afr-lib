<?php
//if(strlen($script_vars[0])==7 && strpos($script_vars[0], 'aid')===0) {

  $_SESSION['show_login']=$_SESSION['show_search']=$_SESSION['show_header']=$_SESSION['show_footer']=false;
  $_SESSION['show_langs']=true;
  $_SESSION['narrow_design']=1;

  $script_base .= "/$aid";
  $affObj = get_object('affiliate', $aid);
  if(!$affObj) die("Aff $aid not found");
  //dump($affObj);
  $aff_lang = $affObj->default_language ? $affObj->default_language : 'en'; 
  //$aff_lang_name = db_getOne("SELECT code FROM language WHERE id=$aff_lang");
  $aff_city_name = db_getOne("SELECT directory FROM city WHERE id=$aff_city");
  $aff_city = $affObj->default_city ? $affObj->default_city : 1000; // 1000 is Amsterdam
  $aff_city_name = db_getOne("SELECT directory FROM city WHERE id=$aff_city");
  $aff_script = $affObj->affiliate_script;
  $aff_css = $affObj->affiliate_css ? $affObj->affiliate_css : "main.css";
  
  
  // todo: store affObj in session instead of all these vars (unset uname+password?)
  //$_SESSION['affilite'] = $affObj;
  $_SESSION['aff_object'] = $affObj;
  
  $_SESSION['aff_name'] = $affObj->affiliate_name;
  $_SESSION['aff_email'] = $affObj->email;
  $_SESSION['aff_live'] = $affObj->live;
  $_SESSION['aff_hide_local'] = $aff_hide_local = $affObj->hide_local;;
  $_SESSION['aff_show_city_menu'] = $aff_show_city_menu = $affObj->show_city_menu;
  $_SESSION['aff_home'] = $aff_home = "/aid$aid/$aff_lang/$aff_city_name/$aff_script"; // todo - look up in DB depending on aff_id
  $_SESSION['aff_root'] = $aff_root = "/aid$aid"; // todo - look up in DB depending on aff_id
  $_SESSION['aff_agent_filter'] = $aff_agent_filter = $affObj->estate_agent_filter;
  $_SESSION['aff_property_filter'] = $aff_property_filter = $affObj->property_filter;

  // params
  $_SESSION['property_filters']['shortstay']  = $affObj->shortstay_minprice; // aff_shortstay_minprice'] =  
  $_SESSION['property_filters']['shortstay']  = $affObj->shortstay_maxprice; // aff_shortstay_maxprice'] =  
  $_SESSION['property_filters']['shortstay']  = $affObj->shortstay_minstay;  // aff_shortstay_minstay'] =   
  $_SESSION['property_filters']['shortstay']  = $affObj->shortstay_maxstay;  // aff_shortstay_maxstay'] =   
  $_SESSION['property_filters']['realestate'] = $affObj->realestate_minprice;// aff_realestate_minprice'] = 
  $_SESSION['property_filters']['realestate'] = $affObj->realestate_maxprice;// aff_realestate_maxprice'] = 
  $_SESSION['property_filters']['longstay']   = $affObj->longstay_minprice;  // aff_longstay_minprice'] =   
  $_SESSION['property_filters']['longstay']   = $affObj->longstay_maxprice;  // aff_longstay_maxprice'] =   

  //dump($affObj);

  // dimensions
  $_SESSION['outer_width']        = $affObj->screen_width ? $affObj->screen_width : 800;
  $_SESSION['inner_width']        = $_SESSION['outer_width'] - 40; // only one col for affs for now

  //$_SESSION['inner_width']        = $affObj->inner_width ? $affObj->inner_width : 500;
  //$_SESSION['right_column_width'] = "235";

  $_SESSION['theme_color']   = $affObj->theme_color; // change this
  $_SESSION['theme_color2'] =  $affObj->theme_color2; // todo change
  // round box with title
  $_SESSION['title_box_top_bg']  = $affObj->title_box_top_color;
  $_SESSION['title_box_bottom_bg']  = $affObj->title_box_top_image;
  $_SESSION['title_box_border']     = $affObj->title_box_border;
  $_SESSION['title_box_color']      = $affObj->title_box_color;

  // colors
  $_SESSION['background_color']   = $affObj->background_color;
  $_SESSION['text_color']         = $affObj->text_color;
  $_SESSION['link_color']         = $affObj->link_color;
  $_SESSION['link_hilite_color']  = $affObj->link_hilite_color;
  $_SESSION['box_top_color']      = $affObj->box_top_color;
  $_SESSION['box_top_image']      = $affObj->box_top_image;
  $_SESSION['box_lite_color']     = $affObj->box_lite_color;
  $_SESSION['box_medium_color']   = $affObj->box_medium_color;
  $_SESSION['box_dark_color']     = $affObj->box_dark_color;

  if($aff_show_city_menu) {
    $cities = $aff_cities = get_aff_cities($affObj);
    $aff_city_count = count($aff_cities);
    if($aff_city_count>1) $_SESSION['show_menu']=1;
  } else {
    $_SESSION['show_menu']=0;
  }

function get_aff_cities($affObj) {

  $aff_cities = array();
  $aff_regions = array();
  $aff_countries  = array();
  if($affObj->show_cities) $aff_cities = explode(',', $affObj->show_cities);
  if($affObj->show_countries) { // add regions of this country to regions list
    $aff_countries = explode(',', $affObj->show_countries);
    foreach($aff_countries as $aff_country) {
      $countryregions = db_getCol("SELECT id FROM region WHERE parent_id=$aff_country");
      $aff_regions = array_merge($aff_regions, $countryregions);
    }
  }
  if($affObj->show_regions) { // add regions in this list to regions list
    $affdbregions = explode(',', $affObj->show_regions);
    $aff_regions = array_merge($aff_regions, $affdbregions);
  }
  if(count($aff_regions)) { // add cities of region list to city list
    foreach($aff_regions as $aff_region) {
      $affdbcities = db_getCol("SELECT id FROM msCity WHERE active=1 AND parent_id=$aff_region");
      $aff_cities = array_merge($aff_cities, $affdbcities);
    }
  }
  return array_unique($aff_cities);
}
  // sort by country/region (see header.inc)
  //$trail[] = array(phrase('home', CAPITALIZE), $aff_home);
?>
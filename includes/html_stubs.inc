<?php

// async loader: http://www.yterium.net/jQl-an-asynchronous-jQuery-Loader?lang=fr
function jQl($load) {
 return '<script type="text/javascript">var jQl={q:[],dq:[],ready:function(a){typeof a=="function"&&jQl.q.push(a);return jQl},unq:function(){for(var a=0;a<jQl.q.length;a++)jQl.q[a]();jQl.q=[]},bId:null,boot:function(a){if(typeof window.jQuery.fn=="undefined"){if(!jQl.bId)jQl.bId=setInterval(function(){jQl.boot(a)},25)}else{jQl.bId&&clearInterval(jQl.bId);jQl.bId=0;jQl.unqjQdep();$(jQl.unq());typeof a=="function"&&a()}},booted:function(){return jQl.bId===0},loadjQ:function(a,c){setTimeout(function(){var b=document.createElement("script"); b.src=a;document.getElementsByTagName("head")[0].appendChild(b)},1);jQl.boot(c)},loadjQdep:function(a){jQl.loadxhr(a,jQl.qdep)},qdep:function(a){if(a)typeof window.jQuery.fn!=="undefined"&&!jQl.dq.length?jQl.rs(a):jQl.dq.push(a)},unqjQdep:function(){if(typeof window.jQuery.fn=="undefined")setTimeout(jQl.unqjQdep,50);else{for(var a=0;a<jQl.dq.length;a++)jQl.rs(jQl.dq[a]);jQl.dq=[]}},rs:function(a){var c=document.createElement("script");document.getElementsByTagName("head")[0].appendChild(c);c.text= a},loadxhr:function(a,c){var b;b=jQl.getxo();b.onreadystatechange=function(){b.readyState!=4||200!=b.status||c(b.responseText,a)};try{b.open("GET",a,true);b.send("")}catch(d){}},getxo:function(){var a=false;try{a=new XMLHttpRequest}catch(c){for(var b=["MSXML2.XMLHTTP.5.0","MSXML2.XMLHTTP.4.0","MSXML2.XMLHTTP.3.0","MSXML2.XMLHTTP","Microsoft.XMLHTTP"],d=0;d<b.length;++d){try{a=new ActiveXObject(b[d])}catch(e){continue}break}}finally{return a}}}; if(typeof window.jQuery=="undefined")var $=jQl.ready,jQuery=$;'.$load.'</script>';
}

function rpx_link($token_url='') {
  $token_url = urlencode('http://'.$_SERVER['HTTP_HOST'].'/rpx/RPX.php');
  return "
<a class=\"rpxnow\" onclick=\"return false;\"
   href=\"https://resprox.rpxnow.com/openid/v2/signin?token_url=$token_url\">
  Sign In
</a>
";
}

function rpx_footer($site) {
  return "
  <script src=\"https://rpxnow.com/openid/v2/widget\"
          type=\"text/javascript\"></script>
  <script type=\"text/javascript\">
    RPXNOW.overlay = true;
    RPXNOW.language_preference = 'en';
  </script>
";
}


function banner_buttons($siteObj, $pageObj) {
  global $script_city;
  if($script_city) {  
    //$sections = array('home' => PAGE_HOME, 'shortstay' => PAGE_SHORTSTAY, 'longstay' => PAGE_LONGSTAY, 'realestate' => PAGE_REALESTATE, 'boat'  => PAGE_BOAT, 'hotel'  => PAGE_HOTEL,  'office' => PAGE_OFFICE, 'shortstay_office'  => PAGE_SHORTSTAY-OFFICE);
    $sections = array('home' => PAGE_HOME, 'shortstay' => PAGE_SHORTSTAY, 'longstay' => PAGE_LONGSTAY);
    foreach($sections as $section => $page_type) {
      $offset=$page_type == PAGE_HOME ? 79 : 155;
      $width = $offset - 11;
      $url = $page_type == PAGE_HOME ? city_link($_SESSION['city_id']) : get_page_link($page_type);
      $anchor = phrase('page_'.$section, UPPERCASE);
      $class = $pageObj->page_type == $page_type ? " yellow_hilite" : "";
      if($page_type == PAGE_HOME || $_SESSION['object_count'][$section])  {
        $tabs++;
        $output .= "<span onclick='document.location=\"$url\";' class='banner_button$class' style='left: ".$left."px;width:".$width."px'>$anchor</span>";
        $left+=$offset;
      }
      if($tabs==4) break; // maximum 4 buttons incl. home
    }
  } else {
    $url = get_page_link(PAGE_HOME);
    $text = phrase('page_home', UPPERCASE);
    $output = "<span onclick='document.location=\"$url\";' class='banner_button yellow_hilite' style='left: 0;width:68px'>$text</span>";
  }
  return $output;
}

// returns title/keywords/description used in meta tags
// side effect: goes to not found (404) for illegal page
function print_suggest_form($lang='en') {
  return '';
  if(!$lang) $lang='en';
  $self = $_SERVER['PHP_SELF'];

  // suggestion form
  $js = 'onsubmit="var geoUrl = $(\'#geourl\').val(); if(!geoUrl) return false; $(\'#suggest_form\').attr(\'action\', \'/\' + geoUrl + \'/\'); "';  
  $form = object_ajax_chooser('geourl', 'geoname', array('filters' => array('active' => 1)));
  $form .= form_input('go', 'submit', phrase('ok', CAPITALIZE));
  return html_form($form, 'suggest_form', '', 'post', '', $js);
}

function print_suggest_form_new($lang='en') {
  if(!$lang) $lang='en';
  $self = $_SERVER['PHP_SELF'];

  // suggestion form
  $js = 'onsubmit="var geoUrl = $(\'#geourl\').val(); if(!geoUrl) return false;alert(geoUrl);$(\'#suggest_form\').attr(\'action\', \'/\' + geoUrl + \'/\'); "';
  $form = form_autocomplete('geourl', '', 'geoname', 'name', 'directory', "active=1 AND lang='$lang'");
  $form .= form_input('go', 'submit', phrase('ok', CAPITALIZE));

  return html_form($form, 'suggest_form', '', 'post', '', $js);

}

// quotes
function quotes($city_id, $lang) {
  $num_quotes = 9;
  $max_length = 500;

  // get quotes entered in this language
  $quotes = find_objects('quote', array('can_publish' => 1, 'published' => 1, 'language' => $lang, 'city_id' => array('0', $city_id)), array('sort' => 'rand()', 'limit' => $num_quotes));
 
  // not enough so pad with quotes translated from English 
  if(count($quotes) < $num_quotes) {
    $tquotes = find_objects('quote', array('can_publish' => 1, 'published' => 1, 'language' => 'en', 'city_id' => array('0', $city_id)), array('sort' => 'rand()', 'limit' => $num_quotes));
    foreach($tquotes as $tquote) {
      if($translation = get_dictionary_entry('quote_'.$tquote->id, $lang)) {
        $tquote->quote = $translation;
        $quotes[] = $tquote;
      }
    }
  }

  // output
  foreach($quotes as $quoteObj) {
    $quote = $quoteObj->quote;
    if(($max_length) && strlen($quote) > $max_length) $quote = substr($quote, 0, $max_length)."...";
    $name = implode(", ", array_filter(array($quoteObj->name, $quoteObj->title, $quoteObj->company)));
    $output .= html_p(html_b($quote).html_break().$name);
  }
  return $output;
}

// category boxes on front page
function categories($city_id, $class='center_column_width_narrow') {
  global $db;
  global $city_name_localized;
  global $propertyTypeObjects;
  //$types = $propertyTypeObjects;
  
  $typeSearch['active'] = 1;
  $typeSearch['hide'] = 0;  // not hidden (for main site)
  $types = find_objects('property_type', $typeSearch, array('sort' => 'id'));    

  $aff_id       = $_SESSION['aff_id'];
  $city_dir     = $_SESSION['city_dir'];
  $bullet_color = $_SESSION['theme_color2'];
  if($_SESSION['aid']) {
    $aff_text = "<p>".phrase('text_explanation_affiliate', CAPITALIZE, array('search' => '__affname', 'replace' => $_SESSION['aff_name']))."</p>";
    if($aff_email = $_SESSION['aff_email']) $aff_text  .= "<p>".phrase('text_explanation_affiliate_contact', CAPITALIZE, array('search' => '__email', 'replace' => "<a mailto='$aff_email'>$aff_email</a>"))."</p>";
    $output .= html_div($aff_text, "$class bold").html_break(); //"<div style='font-weight:bold;width:".$width."px'>".$aff_text."</div>".html_break();
  }

  foreach($types as $typeObj) {
    $links = array();
    $type = $typeObj->name;
    $name = $typeObj->description;
    $min_price= $max_price = 0;
    $count = $_SESSION['object_count'][$type];
    if(!$count) continue;

    $url = get_page_link($type);
    if($_SESSION['aid']) {
      $min_price = number_format($_SESSION['aff_'.$type.'_minprice']);
      $max_price = number_format($_SESSION['aff_'.$type.'_maxprice']);
      $min_stay = $_SESSION['aff_shortstay_minstay'];
    }
    $currency = get_currency_symbol(get_city_currency($city_id), true);
    $price_field = property_price_field($type);
    if(!$min_price) $min_price = number_format(db_getOne("SELECT min($price_field) FROM property WHERE type='$type' AND active=1 AND $price_field > 0 AND city_id=$city_id"));
    if(!$max_price) $max_price = number_format(db_getOne("SELECT max($price_field) FROM property WHERE type='$type' AND active=1 AND $price_field > 0 AND city_id=$city_id"));

    $js2 = "onclick='document.location=\"$url\"'";

    $search = html_link($url, phrase('search', CAPITALIZE), array('class' => 'arrow_link abs_bottom_right'));
    
    $title = phrase("tab_".$type, UPPERCASE);
    if($add_link = property_add_link($type, $city_id, true)) { // only returns link if authorized;
      $title .= html_span(arrow_link(phrase('add', CAPITALIZE), $add_link), 'abs_top_right tiny whitelink');
    } else {
      $explanation = phrase('text_explanation_'.$type, CAPITALIZE, array('search' => '__city', 'replace' => $city_name_localized));    
      if($explanation != 'Text explanation' .$type) $title .= html_span(tooltip('tip_'.$type, $explanation, '', 'abs_right', '/images/new/information_icon_24_2.png', 24, 24));
    }

    $desc = phrase("desc_".$type, CAPITALIZE, array('search' => array('__count', '__currency', '__minstay', '__minprice', '__maxprice'), 'replace' => array($count, $currency, $min_stay, $min_price, $max_price)));
    $list = html_div($desc, 'title_box_list');
    $output .= title_box($title, $search.$add.$expl_div.$list, $class, array('content_class' => 'categories_content'));    
  }
  return $output;                                                              

}

// todo: remove hard coded
function afr_address($siteObj=NULL) {
  if(!$siteObj) $siteObj = $_SESSION['site'];
  $output = phrase('web', CAPITALIZE).": ".html_link(add_http($siteObj->url), $siteObj->name).html_break();
  $output .= phrase('email', CAPITALIZE).": ".html_link('mailto:'.$siteObj->email_address, $siteObj->email_address).html_break(2);
  $address_lines = array('Euro Business Center Amsterdam','Keizersgracht 62-64','1015 CS Amsterdam','The Netherlands','Tel: +31 20 8005077');
  $output .= implode(html_break(), $address_lines);
  return $output;
}

function rss_feed($url) {
  return '';
  require_once 'magpierss/rss_fetch.inc';
  $rss = &fetch_rss($url);
  $title = html_header(strtoupper($rss->channel['title']), 3, 'dkgrey').html_break();
  $items = $rss->items ?: array();
  if(!$items) return ''; 
  foreach ($items as $item ) $links[] = html_link($item['link'], utf8_encode($item['title']));
  return $title.implode('', $links);
}

// links to apartment pages in all cities except city_id (for footer)
function internal_property_links($city_id) {
  $links =array();
  $cities = find_objects('city', array('major' => 1, 'active' => 1, 'id' => array('!=', $city_id)));
  foreach($cities as $city) $links[] = html_link(city_link($city->id), phrase('apartment', CAPITALIZE)." ".$city->name, array('class' => 'greylink'));    
  return implode(" &#8226; ", $links).html_break();
}

// links to all partners with show_in_footer = 1 for this city (or all cities)
function global_partner_links($city_id) {
  $links =array();
  // 'url, name'
  $dblinks=find_objects('link', array('show_in_footer' => 1, 'active' => 1, 'city' => array('0', $city_id)), array('fields' => array('name', 'url'))); 
  foreach($dblinks as $link) $links[] = html_link(add_http($link->url), $link->name);
  return implode(" &#8226; ", $links);
}

function city_menu($siteObj, $pageObj, $options=NULL) {
  global $script_city;
  $cities = $_SESSION['show_cities'];
  $max_cities = $options['max_cities'] ?: 16;
  
  // if(!$_SESSION['show_cities'] || (is_array($_SESSION['show_cities']) && count($_SESSION['show_cities'])) <= 1) return '';
    
  $script = array_pop(explode("/", $_SERVER['SCRIPT_FILENAME']));  
  $this_city = $_SESSION['city_id'];

  $i = 1;
  foreach($cities as $city_id => $name) {
    $class = $city_id == $_SESSION['city_id'] ? 'active' : '';
    $links[] = html_link(city_link($city_id), $name, array('class' => $class));    
    if($i++ > $max_cities) break;
  }
  $home = get_page_link('home');
  $class = ($script=='index.php' && !$script_city) ? " active" : "";
  if(count($cities) > $max_cities) $links[] = html_link($home, phrase('other_cities', CAPITALIZE), array('class' => $class));
  $city_menu = implode(' ', $links);
  return $city_menu;
}

function relocation_links($city_id=0) {
  $city_dir = $city_id ? city_dir($city_id, $_SESSION['language']) : '';  
  $objs = find_objects('relocation_service', array('active' => 1), array('sort' => 'id'));
  foreach($objs as $obj) $links[]= array(get_page_link($obj->section, true, '', '', $city_dir), phrase('menu_'.$obj->section, UPPERCASE));
  return $links;
}

function guide_links($city_id=0) {
  $city_dir = $city_id ? city_dir($city_id, $_SESSION['language']) : '';  
  $objs = find_objects('city_guide', array('active' => 1), array('sort' => 'id'));
  foreach($objs as $obj) $links[] = array(get_page_link('guide_'.$obj->section, true, '', '', $city_dir), phrase('menu_guide_'.$obj->section, UPPERCASE));
  return $links;
}

function relocation_menu() {
  $title = html_header(phrase('relocation_information', UPPERCASE).":", 3, 'pad');
  $parentObj = get_object('site_page', PAGE_RELOCATION);
  return $title.site_menu($_SESSION['siteObj'], $_SESSION['pageObj'], $parentObj);
}

function guide_menu() {
  $title = html_header($city_name." ".phrase('city_guide', UPPERCASE).":", 3, 'pad');
  $parentObj = get_object('site_page', PAGE_GUIDE);
  return $title.site_menu($_SESSION['siteObj'], $_SESSION['pageObj'], $parentObj);
}

function footer_menu($siteObj) {
  $links = array();
  $pgs = find_objects('site_page', array('parent_id' => 0, 'active' => 1, 'show_menu' => 1, 'site_id' => $siteObj->id), array('sort' => 'id'));
  foreach ($pgs as $page) $links[] = get_page_link($page, false, $_SESSION['lang']);
  $output .=  html_break().implode(" | ", $links);

  return $output;
}                                                                                                                                                        
                              
function language_menu($pageObj) {
  global $page;
  global $geoObj;
  global $this_apt; 
  $siteObj = $_SESSION['siteObj'];
  if(!$_SESSION['show_langs']) return '';
  $langs = get_langs();
  foreach($langs as $lang) {
    $name = phrase($lang->name, 'UPPERCASE');
    $flag_src = "/images/flags/".$lang->code.".png";
    if($this_apt) $url = property_link($siteObj, $this_apt, $lang->code);
    else $url = $pageObj->name == 'Home' && $geoObj ? loc_link($geoObj->level, $geoObj->fid, $lang->code) : get_page_link($pageObj, true, $lang->code);
    $url .= ($qs = get_query_string()) ? "?$qs" : '';
    $menu[]= $lang->code==$_SESSION['language'] ? html_image($flag_src, $name, array('height' => 16, 'width' => 25)) : html_link($url, html_image($flag_src, $name, array('height' => 12, 'width' => 18)));
  }
  return implode(' ', $menu);  
}

function partner_links($city_id, $lang='') {
  if(!$lang) $lang = $_SESSION['language'];
  $site_id = $_SESSION['site_id'];
  // if($site_id > 100) return; // hack to show live links on dev site
  $searchArray['active'] = 1;                     
  $searchArray['site'] = array('0', $site_id); // all sites or this site
  $searchArray['language'] = array($lang, '', 'en'); // all langs, english, or this lang
  if($city_id) $searchArray['city'] = array('0', $city_id); // all cities or this city
  $searchArray['relocation_service'] = '0'; 
  $searchArray['city_guide'] = '0'; 
  //dump($searchArray);die();
  $links = find_objects('link', $searchArray, array('sort' => 'category'));

  //$partners = html_header($title, 2);
  foreach($links as $linkObj) {
    $name = $linkObj->name;
    $url=add_http($linkObj->url);                                        
    $desc = $linkObj->description;
    if($linkObj->category != $category) {
      $category = $linkObj->category;
      $output .= html_subheader($category);
    }
    $output .= html_link($url, $name);
    if($desc) $output .= " - ".$desc;
    $output .= html_break()."\n";
  }
  return $output;
}

function relocation_advice_form($section, $city_id) {
  return $output;
}

// prints content of a page in a title box
function afr_content($siteObj, $pageObj, $class='inner_width') {
  global $can_edit;
  global $edit;

  $content_button = $can_edit ? html_div(inline_edit_link('page_content', 'content', $pageObj->content_id), 'ie_edit_link') : '';
  $title_button   = $can_edit ? html_div(inline_edit_link('page_content', 'title', $pageObj->content_id), 'ie_edit_link') : '';
  $subtitle_button   = $can_edit ? html_div(inline_edit_link('page_content', 'subtitle', $pageObj->content_id), 'ie_edit_link') : '';

  //echo("ce=$can_edit e=$edit tb='$title_button' ci=$pageObj->content_id<br>");
  
  if($ref_link = $_GET['ref_link']) {
    $back_name = $_GET['ref_name'] ? phrase('back_to', CAPITALIZE)." ".$_GET['ref_name']: phrase('back', CAPITALIZE);
    $back_link = html_div(html_link($ref_link, $back_name));
  }
  
  $content_image  = $pageObj->title_image ? html_image($img, $pageObj->title, array('class' => 'floatLeft')) : '';

  if($edit == 'page_content:title:'.$pageObj->content_id) {        
    $section_title = inline_edit_form('page_content', 'title', $pageObj->content_id, $pageObj->title, 'text', '', $pageObj->path);
  } else {
    $section_title = $can_edit ? html_div(parse_header_vars($pageObj->title).$title_button, 'relative') : $pageObj->title;
    if($pageObj->subtitle) $section_subtitle = $can_edit ? html_div(html_header($pageObj->subtitle, 2).$subtitle_button, 'uppercase relative') : html_header($pageObj->subtitle, 2);
  }

  if($edit == 'page_content:content:'.$pageObj->content_id) {        
    $section_title = '';
    $section_content = html_div(inline_edit_form('page_content', 'content', $pageObj->content_id, $pageObj->content, 'html', '', $pageObj->path), 'top-z');
  } elseif(strlen(trim($pageObj->content))) {
    $section_content = $can_edit ? html_div($pageObj->content.$content_button, 'relative') : $pageObj->content;
  }                        

  $content =  $pageObj->pre_content.$section_subtitle.$section_image.$section_content.$pageObj->post_content;

  return title_box($section_title, $back_link.$content.$back_link, $class);
}

function afr_home_page($city_id, $aff_id, $can_edit=false) {
  global $db;
  global $__header; 
  global $map_markers;
  global $edit;
  $siteObj = $_SESSION['siteObj'];
  
  if($aff_id) return categories($city_id); // affiliate - blue boxes (categories) only.
  if($city_id) {
    $cityObj = get_object('city', $city_id);
    $desc_field = 'description_'.$_SESSION['language'];
    if($edit == "city:$desc_field:$city_id") return html_div(inline_edit_form('city', $desc_field, $city_id, $cityObj->$desc_field, 'html', '', $_SERVER['PHP_SELF']), 'top-z');
  }
  
  if($cityObj) { // city map - cityObj is set in set_env.inc
     $map_options['center'] = array($cityObj->location_lat,$cityObj->location_long);
     $map_options['accuracy'] = $cityObj->location_accuracy;
     $map_options['height'] = 200;
     $map_options['width'] = $_SESSION['column_width_wide'];
     $map_options['class'] = 'round border';
   } else { // world map
     $map_options['height'] = 480;
     $map_options['width'] = 680;
     $geoObj = $_SESSION['geoObj'];
     $level = $geoObj->level;
     $name = $geoObj->name;
     $id = $geoObj->fid;
     //echo("level of $name ($id) = $level<br>");
     list($level, $geoObjs, $map_menu) = map_data($level, $id);
     foreach($geoObjs as $gObj) {
       $gObj->name = geoname($gObj->id, $level + 1);
       $gObj->link = loc_link($level + 1, $gObj->id);
     }
     $map_options['markers'] = $geoObjs;
  }

  //dump($map_options);
  $map_html = gmap3($map_options);
    
  if($city_id) { // show city page
    //global $propertyTypeArray;
    global $city_name_localized;
    $property_types = site_property_types($siteObj);
    
    if($_SESSION['object_count']['total']) {       // top left box
      $object_list = array();
      foreach($property_types as $key => $name) if($_SESSION['object_count'][$key]) $object_list[]=phrase("title_".$key, LOWERCASE);
      if(count($object_list) > 1) {
        $last = array_pop($object_list);
        $object_list_string = implode(", ", $object_list)." ".phrase('and', LOWERCASE)." ".$last;
      } else {
        $object_list_string = implode(", ", $object_list);
      }

      $title_main = phrase('title_main', CAPITALIZE, array('search' => array('__city', '__objectlist'), 'replace' => array($city_name_localized, $object_list_string)));
      $title_sub = phrase('title_sub', CAPITALIZE, array('search' => array('__city', '__objectlist'), 'replace' => array($city_name_localized, $object_list_string)));
      if(strlen($title_sub)<100) $title_sub = html_break(4).$title_sub;

      $top_left_content = html_div($title_main.html_break(3), '', "id='box_top_left_title_main'");
      $top_left_content .= html_div($title_sub, '', "id='box_top_left_title_sub'").html_image('/images/new/big_arrow_left.png', 'big arrow', array('js' => "style='position:absolute;right:5px;bottom:5px;'", 'height' => 118, 'width' => 88));
      $top_left_box = round_box($top_left_content, 'column_width_wide box_lite_color border');
    }

    $city_map_box = html_div($map_html, 'margin_bottom');
    
    // quotes
    $quotes = quotes($_SESSION['city_id'], $_SESSION['language']);
    $quote_box = round_box($quotes, 'column_width_wide box_medium_color border');

    
    //die("gh1");  


    // city welcome text
    $welcome_text = html_tidy(db_getOne("SELECT $desc_field FROM msCity WHERE id=$city_id")); // look for city-specific welcome text
    if(!$welcome_text) { // use generic welcome text
      $landmarks = $_SESSION['landmarks'] ? "and close to famous landmarks, such as $_SESSION[landmarks]" : "";
      $welcome_text = html_tidy(phrase('welcometext_city', CAPITALIZE, array('search' => array('__city', '__landmarks', '__numtotal'), 'replace' => array($city_name_localized, $landmarks, $_SESSION['object_count']['total']))));
    }
    $content_button = $can_edit ? html_div(inline_edit_link('city', $desc_field, $city_id), 'ie_edit_link') : '';
    //dump($_SERVER);
    $welcome_text_box = round_box($welcome_text.$content_button, 'center_column_width_narrow box_medium_color border');
    
    if($_SESSION['city_country_code']=='NL') $news_box = round_box(rss_feed('http://www.dutchnews.nl/news/index.xml'), 'center_column_width_narrow box_medium_color orange_block_links small');

    $categories = categories($city_id);

    $left_column = $top_left_box.$city_map_box.$quote_box; // left column
    $middle_column = $categories.$welcome_text_box.$news_box; //  middle column
  //$right_column = $relocation_box.$guide_box.$news_box; // right column

  } else { // global - show world map

    $welcome_text = html_tidy(phrase('welcometext')); // global welcome text
    $welcome_text_box = round_box($welcome_text, 'inner_width box_medium_color');

    $big_map = html_header(phrase('destination', CAPITALIZE), 2);
    $big_map .= print_suggest_form($_SESSION['language']);
    $big_map .= html_div($map_html, 'margin_top margin_bottom');
    $big_map .= $map_menu;
    $big_map_box = round_box($big_map, 'inner_width box_lite_color');
    // $ad = display_ad_slot('6928533474'); // long thin

    $left_column = $big_map_box.html_break().$welcome_text_box;

    $right_class = "class='afr_menu'";
  }

  $eura = html_image("/images/new/eura_logo.png", "Eura member", array('height' => 46, 'width' => 100));
  //$amex = html_image("/images/new/amex_80px.png", "Official Amex Partner", '', '', 80, 80);
  $bitcoin = html_image("/images/png/weAcceptBitcoin.png", "We accept Bitcoins", array('height' => 46, 'width' => 100));
//  $star = html_script("https://seal.starfieldtech.com/getSeal?sealID=1026502066036ad5ba21271076bf93d2217f9901875260922639503713");
  //$paypal = html_image_link("https://www.paypal.com/us/verified/pal=paypal%40apartments-for-rent%2ecom", "https://www.paypal.com/en_US/i/icon/verification_seal.gif", "Official PayPal Seal");

  $footer_row = $eura.$amex.$star.$paypal.$bitcoin;

  $output .= "
      <div id=\"header_row\">$header_row</div>
      <div id=\"left_column\">$left_column</div>
      <div id=\"right_column\" $right_class>$right_column</div>
      <div id=\"middle_column\">$middle_column</div>
      <div id=\"footer_row\">$footer_row</div>\n";
  
  return $output;
}

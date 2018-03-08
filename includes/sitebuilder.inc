<?php

function sb_make_all($siteObj, $upload=false) {
  global $db;                                   
  $org_lang = $_SESSION['lang'];
  $_SESSION['lang'] = $_SESSION['language'] = $siteObj->default_language; // temp set session lang to simplify
  $org_size = $_SESSION['media_small_size'];
  $_SESSION['media_small_size'] = 120;

  $test_url = add_http($siteObj->test_url);
  $local_dir = $siteObj->test_directory;
  $image_dir = $local_dir.$siteObj->image_directory;
  $siteObj->property_type = $property_type = get_object('property_type', $siteObj->show_types, 'name');
  $property_type_name = $property_type == 'shortstay' ? 'apartment' : $property_type; 
  
  
  $siteObj->city_id = $_SESSION['show_cities'][0];
  
  echo(html_h1("Running sb_make_all for $siteObj->name ($siteObj->id)"));
  recursiveDelete("$local_dir/j/"); // delete old JS
  shell_exec("cp -ax /websites/autobuild.skel/j/ $local_dir"); // copy javascript
  shell_exec("cp -ax /websites/autobuild.skel/i/ $local_dir"); // copy images - keep old ones.. 
  //shell_exec("cp -ax /websites/autobuild.skel/c/ $local_dir"); // copy css - keep old ones..
  

  $siteObj->type_name_plural = phrase($property_type_name.'s', LOWERCASE, array('lang' =>$siteObj->default_language));
  $siteObj->type_name = phrase($property_type_name, LOWERCASE);

  $siteObj->type_name_plural_cap = phrase($property_type_name.'s', CAPITALIZE, array('lang' => $siteObj->default_language));
  $siteObj->type_name_cap = phrase($property_type_name, CAPITALIZE);

  $siteObj->hourly = $property_type == 'boat' ? true : false;
  $siteObj->show_map = $property_type == 'boat' ? false : true;
  
  echo("show_types=$siteObj->show_types type=$siteObj->property_type name=$property_type_name name(dict)= $siteObj->type_name plural = $siteObj->type_name_plural");

  $filters = explode(';', $siteObj->property_filters);
  foreach($filters as $filter) {
    list($k, $v) = explode('=', $filter);
    if($k == 'city_id') $siteObj->city_id = $v;
  }
  
  //$filters[] = "parent_id IS NULL"; // don't show child properties
                  
  $siteObj->property_filters = $filters;
  //dump($filters);  die();
  //dump($siteObj);
  $siteObj = sb_make_css($siteObj);
  //dump($siteObj);

  // generate corner and border images
  if($color = $siteObj->itemlist_background_color) generate_corners($color, $image_dir);
  if($border_color = $siteObj->itemlist_border) generate_borders($color, $image_dir);

  if($color = $siteObj->itemdetail_background_color) generate_corners($color, $image_dir);
  if($border_color = $siteObj->itemdetail_border) generate_borders($color, $image_dir);

  if($color = $siteObj->right_box_background_color) generate_corners($color, $image_dir);
  if($border_color = $siteObj->right_box_border) generate_borders($color, $image_dir);

  $pages = find_objects('page_content', array('site_id' => $siteObj->id, 'active' => 1), array('sort' => 'priority DESC'));
  $count = count($pages);
  echo("found $count pages<br>"); 
  //die();
  foreach($pages as $pageObj) {
    $site_page = get_object('site_page', $pageObj->page_id);
    $pageObj = site_merge_page_object($site_page, $pageObj);
    sb_make_page($siteObj, $pageObj);
  }
  // make request.php page
  $dir = $siteObj->test_directory;
  $pageObj = sb_request_page($siteObj, $pageObj);
  sb_write($dir.$pageObj->script, $pageObj->html);

  if($siteObj->affiliate_id) {
    // make status.php page
    if($pageObj = sb_status_page($siteObj, $pageObj)) sb_write($dir.$pageObj->script, $pageObj->html);
  
    // make book.php page
    if($pageObj = sb_book_page($siteObj, $pageObj)) sb_write($dir.$pageObj->script, $pageObj->html);
  
    // make search.php page
    if($pageObj = sb_search_page($siteObj, $pageObj)) sb_write($dir.$pageObj->script, $pageObj->html);

    // make feedback.php page
    if($pageObj = sb_feedback_page($siteObj, $pageObj)) sb_write($dir.$pageObj->script, $pageObj->html);

    // make view.php page
    if($pageObj = sb_any_page($siteObj, $pageObj)) sb_write($dir.$pageObj->script, $pageObj->html);

  }
    
  // make apartments
  $shortname = $siteObj->shortname;
  if(sb_make_apartments($siteObj, $filters))
    echo ("<h3>Successfully built <a href='$test_url' target='_new'>$siteObj->name test site </a>.</h3>");
  else
    echo ("<h3>Failed to build <a href='$test_url' target='_new'>$siteObj->name test site </a>.</h3>");

  $page_id = $siteObj->property_type == 'shortstay' ? 1301 : 1304;
  $pageObj = find_object('page_content', array('site_id' => $siteObj->id, 'language' => $siteObj->default_language, 'page_id' => $page_id));

  //$base = $siteObj->property_type; // == 'shortstay' ? 'apartments' : 'boats';
  //if($base == 'shortstay') $base = 'apartments';
  $base = $siteObj->type_name_plural;
  
  $ext = "html";
  
  $script = $siteObj->property_type == 'shortstay' ? 'apartments.html' : 'boats.html';
  //$script = $pageObj->script;
  echo("Script = $script<br>");
  shell_exec("rm $local_dir/$script"); // remove apartments.html

  $local_dir = $siteObj->test_directory;
  //list($base, $ext) = explode('.', $pageObj->script);
  $cmd = "cp -a $local_dir".$base."_1."."$ext $local_dir/$script";
  execRootCmd($cmd, $output); // link apartments.html to apartments_1.html 
  echo("$cmd => ".dump($output));
  
  if(!isset($_GET['upload'])) echo('<a href="/admin/scripts/sitebuilder.php?site='.$shortname.'&upload" target="_new">Click here to upload to live server</a>');
  if($upload) sb_upload($siteObj);
  $_SESSION['lang'] = $_SESSION['language'] = $org_lang; // restore session lang
  $_SESSION['media_small_size'] = $org_size;

}

function sb_make_css($siteObj) {
  // add layout info to siteObj
  $css = get_default_layout($siteObj);
  
  // hard coded now for YAH
  switch($siteObj->domain) {
  case 'youramsterdamhousing.com':
    $css['banner_height']  = 275;
    $css['banner_width']   = 850;
    $css['banner_src']     = '/f/yah.swf';
    $css['content_background_color'] = '232323';
    $css['itemlist_background_color'] = '232323';
    $css['itemdetail_background_color'] = '232323';

    break;
  default:
    break;
  }
                                    
  if($layoutObjs =   find_objects('site_layout', array('site_id' => $siteObj->id))) {
    foreach($layoutObjs as $layoutObj) {
      //$name = get_object_field('site_layout_div', $layoutObj->layout_div, 'name');
      $name = $layoutObj->div_select;
      $laArray = obj2array($layoutObj);
      foreach($laArray as $key => $val) {
        $full_key = $name.'_'.$key;
        //echo($full_key.html_break());
        if($val && isset($css[$full_key]) ) $css[$full_key] = $val; // override default with DB value (if already in default)
      }
    }
  }

  // calculated values
  $css['apTitle_width'] = $css['containerApLeft_width'] -  22;
  $css['itemlist_width'] = $css['containerApRight_width'] -  4;
  $css['detailItem_width'] = $css['containerApRight_width'] -  8;

  $css['containerApartments_width'] = $css['containerApLeft_width'] + $css['containerApRight_width'] + 12;

// containerApRight_width'     => 682,  = pageNav_w

// 'itemlist_width'                   => 678,

// 'itemdetail_width'                 => 700,


  $siteObj->layout = $css;
  foreach($css as $k => $v) $siteObj->$k = $v; // copy to siteObj; todo: use ->layout instead

  $colors = $siteObj->layout;
  foreach($colors as $k => $v) {
    $keys[] = 'CSS_'.$k;
    $vals[] = $v;
  }
  // generate css file
  if($siteObj->autogenerate_css) {
    $skel_file = "/websites/autobuild.skel/c/housing.css";
    $css_file = "$siteObj->test_directory/c/housing.css";
    $css = read_file($skel_file);
    $css = str_replace($keys, $vals, $css);
    // write_file($css_file, $css); // turn off, use new CMS
  }

  return $siteObj;
}

function sb_make_page($siteObj, $pageObj) {
  $dir = $siteObj->test_directory;
  $pageObj = sb_page($siteObj, $pageObj);
  sb_write($dir.$pageObj->script, $pageObj->html);
}

function sb_write($filename, $html) {
  if(write_file($filename, $html)) {
    echo("wrote to $filename<br />");
    return true;
  } else {
    echo("failed to write to $filename<br />");
    return false;
  }
}

// update site_page,page_content set page_content.script = 'contact.html',show_top_menu=1,page_content.priority=4 where page_content.page_id=site_page.id and page_type=1010 and site_page.site_id in (108,109,202,216,219);
// update site_page,page_content set page_content.menu_title = 'Apartments', page_content.priority=8, page_content.script = 'apartments.html',show_top_menu=1 where page_content.page_id=site_page.id and page_type=1301 and site_page.site_id in (108,109,202,216,219) and page_content.language='en';
// update site_page,page_content set page_content.menu_title = 'Appartementen', page_content.priority=8, page_content.script = 'apartments.html',show_top_menu=1 where page_content.page_id=site_page.id and page_type=1301 and site_page.site_id in (108,109,202,216,219) and page_content.language='nl';
// update site_page,page_content set page_content.script = 'about.html',show_top_menu=1,page_content.priority=6 where page_content.page_id=site_page.id and page_type=1001 and site_page.site_id in (108,109,202,216,219);
// update site_page,page_content set page_content.script = '/',page_content.menu_title = 'Home', page_content.priority=10, show_top_menu=1 where page_content.page_id=site_page.id and (page_type=1000 or path='/') and site_page.site_id in (108,109,202,216,219);
//update site_page,page_content set site_page.script = page_content.script where show_top_menu=1 and page_content.page_id=site_page.id and site_page.site_id in (108,109,202,216,219);

function sb_top_nav($siteObj, $active_page) {

  $pages = find_objects('page_content', array('site_id' => $siteObj->id, 'show_top_menu' => 1, 'active' => 1), array('sort' => 'priority DESC'));
  $nav = '
      <div id="nav">
        <ul>
  ';

  foreach($pages as $this_page) {
    $site_page = get_object('site_page', $this_page->page_id);
    $pageObj = site_merge_page_object($site_page, $this_page);
      $url = clean_path('/'.ltrim($pageObj->script, '/'));
    $name = $pageObj->menu_title;
    if($url=='index.html') $url = ''; // special case for home
    //if(strtolower($pageObj->name) == $siteObj->property_type) {
    //  list($fname, $ext) = split_filename($url);
    //  $url = $fname.'_1.'.$ext;
    //}

    $class = $pageObj->name == $active_page ? "active floatLeft" : "floatLeft";
    $nav .= "<li class=\"".$class."\"><a href=\"".$url."\"><span>".$name."</span></a></li>\n";
  }
  $nav .= '
        </ul>
      </div><!-- nav -->
  ';
  return $nav;
}

function sb_sub_nav($siteObj, $active_page) {

  $pages = find_objects('page_content', array('site_id' => $siteObj->id, 'show_sub_menu' => 1, 'active' => 1), array('sort' => 'priority DESC'));
  $nav = "
    <div id=\"subNav\">
      <ul>
  ";
  foreach($pages as $this_page) {
    $site_page = get_object('site_page', $this_page->page_id);
    $pageObj = site_merge_page_object($site_page, $this_page);
    $url = $pageObj->script;
    $name = $pageObj->menu_title;

    if($url=='index.html') $url = ''; // special case for home
    //if(strtolower($pageObj->name) == $siteObj->property_type) {
    //  list($fname, $ext) = split_filename($url);
    //  $url = $fname.'_1.'.$ext;
    //}
    $class = $pageObj->name == $active_page ? "active floatLeft" : "floatLeft";
    $nav .= "<li class='$class'><a href=\"/".$url."\"><span>".$name."</span></a></li>\n";
  }
  $nav .= "
      </ul>
    </div><!-- subNav -->
  ";
  return $nav;

}


function sb_right_box($siteObj, $pageObj) {
  if($siteObj->shortname == 'yah') { // special case
    $output .= html_div(html_image_link('/pdf/YourAmsterdamHousing.pdf', '/i/download_small.png', 'Download PDF', array('target' => "_blank")), 'rightBox');     
  }  

//                   <div id="downloadBtn"><img src="/i/transparent.gif" height="47" width="230" alt="Download PDF"  /></div>
//    ' <a href="/pdf/YourAmsterdamHousing.pdf" target="_blank">
//                 </a><!-- downloadBtn -->';

  // if($siteObj->shortname == 'yah') { // special case
  //   $output = '
  //     <div id="rightBox">';
  //   $output .= sb_search($siteObj, $pageObj->language).'<br />';
  //   $output .= ' <a href="/pdf/YourAmsterdamHousing.pdf" target="_blank">
  //                  <div id="downloadBtn"><img src="/i/transparent.gif" height="47" width="230" alt="Download PDF"  /></div>
  //                </a><!-- downloadBtn -->';
  //   $output .= '
  //     </div><!-- rightBox  -->';
  // } else {
  //   $width = $siteObj->right_col_w;
  //   $output = '
  //     <div id="rightBox">';
  //   $output .= sb_search($siteObj, $pageObj->language);
  //   //$output .= sb_search_result($width, $pageObj->language);
  //   $output .= '
  //     </div><!-- rightBox  -->';
  //   return custom_box($output, $siteObj->image_directory, $siteObj->right_box_background_color, $siteObj->right_box_width, $siteObj->right_box_height, $siteObj->right_box_border, $siteObj->right_box_class);
  // 
  // }
  return $output;
}

function sb_search_box($siteObj, $lang='en') {
  
  $action = '/search.php';
  return searchbar($siteObj->property_type, $siteObj->city_id, array('action' => $action));
  
  if($city_id = $siteObj->city_id) {
    $hidden_fields = form_input('city_id', 'hidden', $city_id);
  } else {
    // todo: show city chooser
  }
  

	// checkin
	$calparams['minDate'] = "'+0d'";
	$calparams['maxDate'] = "'+1y'";
	//$calparams['defaultDate'] = "'+1d'";
  
  $server = $siteObj->test_url;
  $action = '/search.php';
  if($siteObj->hourly) {
    $in_chooser = datepicker('in', $calparams);
    // $onchange = " onchange='update_pickup(\"$resObj->checkin\");'";
    $time_chooser = timepicker('arrival_time', '', $resObj->arrival_time, $onchange);
    $fields = array('Date'   => $in_chooser,
                    'Time'   => $time_chooser,
                    '# Hours'   => select_from_array('nh', num_array(2, 10, 0.5), $obj->num_hours),
                    '# Guests'  => select_from_array('ng', num_array(1, 10, 1), $obj->customer_count),
                    '&nbsp;'    => ' '.form_input('search', 'submit', 'Search'));
  } else {
    $calparams['dependent_field'] = "out"; // update checkout
    $calparams['dependent_field_add_days'] = 3;  // with checkin + num_days days
    $in_chooser = datepicker('in', $calparams);
  
    // checkout
    //$calparams['defaultDate'] = "'+4d'";
    unset($calparams['dependent_field']);
    unset($options['dependent_field_add']);
    $out_chooser = datepicker('out', $calparams);

    $fields = array('Checkin'   => $in_chooser,
                    'Checkout'  => $out_chooser,
                    '# Guests'  => select_from_array('ng', num_array(1, 10, 1), $obj->customer_count),
                    '&nbsp;'    => ' '.form_input('search', 'submit', 'Search'));
  }

  $fieldset = html_fieldset($fields, '',  'fieldset_top', '', false);
  $form     = html_form($fieldset.$hidden_fields , 'search', $action, 'GET');
  return html_div($form, 'search');
  
  

  return ("
  <form action='' method='GET'>
  Number of guests: <br />
  <input type=\"text\" name=\"ng\" size=\"2\" maxlength=\"2\" value=\"$_GET[ng]\"><br />

  Checkin: <br />
  <input type=\"text\" name=\"in\" size=\"8\" value=\"$_GET[in]\" maxlength=\"10\" readonly=\"readonly\">
  <img src=\"/i/calendar_button.gif\" onclick=\"displayDatePicker('in', false, 'ymd', '-');\">
  <br />

  Checkout: <br />
  <input type=\"text\" name=\"out\" size=\"8\" value=\"$_GET[out]\" maxlength=\"10\" readonly=\"readonly\">
  <img src=\"/i/calendar_button.gif\" onclick=\"displayDatePicker('out', false, 'ymd', '-', 'in');\">
  <br />
  </form>
  ");

}


function sb_make_apartments($siteObj, $filters) {

  $start = 0;
  $limit = 200;
  $apts_pp = $siteObj->items_per_page;
  $type = $siteObj->property_type;
  $sort= 'priority DESC, scraped DESC, id';
  //dump($filters);die();
  
  $apts = get_objects($type, $filters, $sort, $order, $start, $limit);
  $count = count($apts);
  if(!$count) {
    dump($filters);
    die("No properties");
  }
  echo("<b>Found $count properties, $apts_pp properties per page. Limit = $limit</b><br />");
  $num_pages = ceil($count / $apts_pp);
  $shortname = $siteObj->shortname;
  
  switch($siteObj->property_type) {
    case 'shortstay':
      $page_type = PAGE_SHORTSTAY;
      break;
    case 'boat':
      $page_type = PAGE_BOAT;
      break;
    case 'realestate':
      $page_type = PAGE_REALESTATE;
      break;
    case 'longstay':
      $page_type = PAGE_LONGSTAY;
      break;
    default:
      die("sb_make_apartments: property type $property_type not implemented.");
  }
  //$page_id = $siteObj->property_type == 'shortstay' ? 1301 : 1304;

  if($pageObj = site_find_pagetype($siteObj, $page_type)) {
    $page_id = $pageObj->id;
    $search = array('site_id' => $siteObj->id, 'language' => $siteObj->default_language, 'page_id' => $page_id);
    $pageObj = find_object('page_content', $search);
    if(!$pageObj) {dump($search);die("sb_make_apartments: couldn't find page with parent $page_id in $lang");}
  } else {
    die("sb_make_apartments: couldn't find page with type=$page_type for $siteObj->name");
  }
  // generate apt detail html pages
  $j=1;
  //dump($pageObj);die();
  $base = $siteObj->type_name;
  $ext = "html";

  // first create all links
  foreach($apts as $apt) {
    $apt->sb_index = $j;
    $basename = $base."-".$apt->shortname.".html";       // use shortname of property
    $apt->sb_link = "/$basename";
    //echo("apt link = $apt->sb_link<br>"); 
    $dir = $siteObj->test_directory;
    $apt->sb_filename = $dir."apts/$basename";
    $j++;
  }

  // then generate the pages  
  foreach($apts as $apt) {
    $pageObj = sb_apartment($siteObj, $pageObj, $apts, $apt);
    sb_write($apt->sb_filename, $pageObj->html);
    if($limit && $j >= $limit) break;
  }

  // generate list html pages
  // $name = 'apartments';
  // $pageObj = find_object('page_content', array('site_id' => $siteObj->id, 'language' => $siteObj->default_language, 'page' => $name));
  // if(!$pageObj) die("sb_make_apartments: couldn't find page $name");
  $base_pl = $siteObj->type_name_plural;
  for($i=0; $i < $num_pages; $i++) {

    //list($base, $ext) = explode('.', $pageObj->script);
    $basename = $base_pl."_".($i+1).".$ext";
    $filename = $dir."$basename";

    echo("base=$base ext=$ext dir=$dir basename=$basename file=$filename<br>");

    $apts_subset   = array_slice($apts, $i * $apts_pp, $apts_pp);
    $page_num = $i+1;
    $pageObj = sb_apartment_list($siteObj, $pageObj, $apts, $apts_subset, $num_pages, $page_num);
    sb_write($filename, $pageObj->html);

  }

  return true;
}


// HTML output functions
// TODO; generate divs and elements with code?

// apartments.html
function sb_page($siteObj, $pageObj, $content='') {
  $property_type = $siteObj->property_type;
  $head = sb_head($siteObj, $pageObj);
  $body = sb_body($siteObj, $pageObj, $content);
  $html = '
  <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
  <html xmlns="http://www.w3.org/1999/xhtml">
'. $head.'
'. $body.'
  </html>

  ';
  if(!is_object($pageObj)) dump($pageObj);
  $pageObj->html = $html;
  return $pageObj;
}

function sb_header_var($siteObj, $pageObj, $var) {
  if($apt = $pageObj->apt) {
    $apt_type = property_subtype_name($apt);
    $apt_name = $apt->name;
  }

  if($ptype = $aptObj->type ?: $siteObj->property_type) {
    $ptype_name = property_type_name($ptype);
  }
  
  if($city_id = $aptObj->city_id ?: $siteObj->city_id) {
    $cityObj = get_object('city', $city_id);
    $city_name = $cityObj->name;    
    $district_name = get_object('district', $cityObj->parent_id, 'name');
    $country_name = get_object('district', $cityObj->grandparent_id, 'name');
  }
  return str_replace(
    array('__city', '__district', '__country', '__propertytype', '__aptname', '__apttype', '__sitename', '__pagename'), 
    array($city_name, $district_name, $country_name, $ptype_name, $apt_name, $apt_type, $siteObj->name, $pageObj->screen_title ?: $pageObj->name), 
    $pageObj->$var);
}

function sb_head($siteObj, $pageObj) {
  $se_desc = sb_header_var($siteObj, $pageObj, 'se_description');
  $se_keyw  = sb_header_var($siteObj, $pageObj, 'se_keywords');
  $se_title = sb_header_var($siteObj, $pageObj, 'se_title');

  $html = '
  <head>
  <title>'.$se_title.'</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <meta name="description" content="'.$se_desc.'" />
  <meta name="keywords" content="'.$se_keyw.'" />
  <link rel="stylesheet" href="/c/housing.css" type="text/css" media="screen" />
  <link rel="stylesheet" href="/c/jquery-ui-1.7.2.custom.css" type="text/css" media="screen" />
  ';
  
  $terms = js_terms($siteObj->default_language);
  $html.= html_script_inline($terms);
//  <script type=\"text/javascript\">
//    var error_not_valid_email   = '".phrase('not_valid_email', LOWERCASE)."';
//    var error_invalid_character = '".phrase('invalid_character', CAPITALIZE)."';
//    var error_in_email_address  = '".phrase('in_email_address', LOWERCASE)."';
//    var text_new_window         = '".phrase('opens_in_new_window')."';
//    var dayNames                = ['".implode("','", phrase('weekday_names'))."'];
//    var dayNamesShort           = ['".implode("','", phrase('weekday_shortnames'))."'];
//    var dayNamesMin             = ['".implode("','", phrase('weekday_shortestnames'))."'];
//    var monthNames              = ['".implode("','", phrase('month_names'))."'];
//    var monthNamesShort         = ['".implode("','", phrase('month_shortnames'))."'];
//  </script>
//  ";

  $jquery_version = '1.4.1';
  $jquery_ui_version = '1.7.2';
  $scripts = array('/j/datetime.js',
                   '/j/jquery/'.$jquery_version.'/jquery.min.js',
                   '/j/jquery/ui/'.$jquery_ui_version.'/jquery-ui.min.js'); // all in one                   
  foreach($scripts as $script) $html .= "  ".html_script($script);
  
//  <script src="/j/jquery/-1.2.3.js" type="text/javascript"></script>
//  <script src="/j/jquery/-1.2.3.js" type="text/javascript"></script>
//  <link rel="stylesheet" href="/c/datepicker.css" type="text/css" media="screen" />
//  <script src="/j/datepicker.js" type="text/javascript" ></script>


  // optional javascript + css
  if(strtolower($pageObj->name) == $property_type) {
    $html .= '
      <script src="/j/swfobject.js" type="text/javascript" ></script>
      <script src="/j/jquery.idTabs.min.js" type="text/javascript"></script>
   ';
    if($siteObj->show_map) {
      $html .= '
        <script src="http://maps.google.com/maps?file=api&amp;v=2&amp;key='.$siteObj->googlemap_live_key.'" type="text/javascript"></script>
        <script src="/j/googlemap.js" type="text/javascript" ></script>
     ';
    } 
  }
  
  if($pageObj->name == 'Request') {
    $html .= '
      <script src="/j/request.js" type="text/javascript"></script>
    ';
  }
  if($gaid = $siteObj->google_analytics_id) $html .= google_tracker($gaid);
  $html .= '
  </head>
  ';
  return $html;
}


function sb_body($siteObj, $pageObj, $content='') {
  $header  = sb_header($siteObj);
  $top_nav = sb_top_nav($siteObj, $pageObj->name);
  if(!$content) $content = sb_content($siteObj, $pageObj);
  $body =  '
<body onload="" onunload="">
  <div id="container">
    '.$header.'
    '.$top_nav.'
    '.$content.'
  </div><!-- container -->
'.$scripts.'
</body>';

  return $body;
}

function sb_content($siteObj, $pageObj) {
  $page_content = sb_page_content($siteObj, $pageObj);
  $sub_nav      = sb_sub_nav($siteObj, $pageObj->name);
  $search_box   = sb_search_box($siteObj, $pageObj);
  $right_box    = sb_right_box($siteObj, $pageObj);
  $footer       = sb_footer($siteObj);
  $html =  '
    <div id="left">
    '.$page_content.'
    </div><!-- left -->
    <div id="right">
    '.$search_box.'
    '.$right_box.'
    '.$sub_nav.'
    '.$footer.'
    </div><!-- right -->';
  return $html;
}

function sb_any_page($siteObj) {
  $skel_file = "/websites/autobuild.skel/p/view.inc";  
  $pageObj->script = 'view.php';
  $pageObj->language = $siteObj->default_language ? $siteObj->default_language : 'en';
  $content = "";
  if($aff_id=$siteObj->affiliate_id) {
    $affObj = get_object('affiliate', $aff_id);
    $content .= '<? $aff_id='.$aff_id.';?>'."\n";
  } else {
    echo("Site $siteObj->name has no affiliate ID - cannot make feedback page<br>");
    return NULL;
  }
  
  // $content .= '<$w='.($affObj->narrow_col_width + $affObj->center_col_width + 20).';>'."\n";  
  $content .= '<? $site_id='.$siteObj->id.';?>'."\n";
  $content .= '<? $lang ='.$pageObj->language.';?>'."\n";
  $content .= read_file($skel_file);
  return sb_page($siteObj, $pageObj, $content);
}

function sb_request_page($siteObj) {

  $skel_file = "/websites/autobuild.skel/p/request.inc";
  //$pageObj = find_object('page_content', array('site_id' => $siteObj->id, 'language' => $siteObj->default_language, 'page' => 'request'));
  $pageObj->script = 'request.php';
  $pageObj->language = $siteObj->default_language ? $siteObj->default_language : 'en';

  $content = "";
  if($aff_id=$siteObj->affiliate_id) $content .= '<? $aff_id='.$aff_id.';?>'."\n";
  $content .= '<? $site_id='.$siteObj->id.';?>'."\n";
  $content .= read_file($skel_file);
  return sb_page($siteObj, $pageObj, $content);
}

function sb_feedback_page($siteObj) {

  $skel_file = "/websites/autobuild.skel/p/feedback.inc";  
  $pageObj->script = 'feedback.php';
  $pageObj->language = $siteObj->default_language ? $siteObj->default_language : 'en';
  $content = "";
  if($aff_id=$siteObj->affiliate_id) {
    $affObj = get_object('affiliate', $aff_id);
    $content .= '<? $aff_id='.$aff_id.';?>'."\n";
  } else {
    echo("Site $siteObj->name has no affiliate ID - cannot make feedback page<br>");
    return NULL;
  }
  
  // $content .= '<$w='.($affObj->narrow_col_width + $affObj->center_col_width + 20).';>'."\n";
  
  $content .= '<? $site_id='.$siteObj->id.';?>'."\n";
  $content .= '<? $lang ='.$pageObj->language.';?>'."\n";
  $content .= read_file($skel_file);
  return sb_page($siteObj, $pageObj, $content);
}

function sb_status_page($siteObj) {

  $skel_file = "/websites/autobuild.skel/p/status.inc";
  
  //$pageObj = find_object('page_content', array('site_id' => $siteObj->id, 'language' => $siteObj->default_language, 'page' => 'status'));
  $pageObj->script = 'status.php';
  $pageObj->language = $siteObj->default_language ? $siteObj->default_language : 'en';
  $content = "";
  if($aff_id=$siteObj->affiliate_id) {
    $affObj = get_object('affiliate', $aff_id);
    $content .= '<? $aff_id='.$aff_id.';?>'."\n";
  } else {
    echo("Site $siteObj->name has no affiliate ID - cannot make status page<br>");
    return NULL;
  }
  
  // $content .= '<$w='.($affObj->narrow_col_width + $affObj->center_col_width + 20).';>'."\n";
  
  $content .= '<? $site_id='.$siteObj->id.';?>'."\n";
  $content .= '<? $lang ='.$pageObj->language.';?>'."\n";
  $content .= read_file($skel_file);
  return sb_page($siteObj, $pageObj, $content);
}

function sb_book_page($siteObj) {

  $skel_file = "/websites/autobuild.skel/p/book.inc";
  
  $pageObj->script = 'book.php';
  $pageObj->language = $siteObj->default_language ? $siteObj->default_language : 'en';
  $content = "";
  if($aff_id=$siteObj->affiliate_id) {
    $affObj = get_object('affiliate', $aff_id);
    $content .= '<? $aff_id='.$aff_id.';?>'."\n";
  } else {
    echo("Site $siteObj->name has no affiliate ID - cannot make booking page<br>");
    return NULL;
  }
  
  // $content .= '<$w='.($affObj->narrow_col_width + $affObj->center_col_width + 20).';>'."\n";
  
  $content .= '<? $site_id='.$siteObj->id.';?>'."\n";
  $content .= '<? $lang ='.$pageObj->language.';?>'."\n";
  $content .= read_file($skel_file);
  return sb_page($siteObj, $pageObj, $content);
}

function sb_search_page($siteObj) {

  $skel_file = "/websites/autobuild.skel/p/search.inc";
  
  $pageObj->script = 'search.php';
  $pageObj->language = $siteObj->default_language ? $siteObj->default_language : 'en';
  $content = "";
  if($aff_id=$siteObj->affiliate_id) {
    $affObj = get_object('affiliate', $aff_id);
    $content .= '<? $aff_id='.$aff_id.';?>'."\n";
  } else {
    echo("Site $siteObj->name has no affiliate ID - cannot make search page<br>");
    return NULL;
  }
  
  // $content .= '<$w='.($affObj->narrow_col_width + $affObj->center_col_width + 20).';>'."\n";
  
  $content .= '<? $site_id='.$siteObj->id.';?>'."\n";
  $content .= '<? $lang ='.$pageObj->language.';?>'."\n";
  $content .= read_file($skel_file);
  return sb_page($siteObj, $pageObj, $content);
}

function sb_apartment($siteObj, $pageObj, $apts, $apt) {

  if(!$siteObj->hide_apt_menu) $left = sb_list($siteObj, $apts, $apt->name);

  $descObj = property_description($siteObj, $apt->id, $siteObj->default_language);
  $len = strlen($descObj->description);
  //echo($descObj ? "Using description from site $descObj->site_id in lang $descObj->language for $apt->name with len=$len<br>" : "<b>No description found for $apt->name</b><br>");
  $apt->description = $descObj->description;
  $right = sb_item_details($siteObj, $apt);
  $content .= sb_apt_container($left, $right);
  $pageObj->apt = $apt;
  return sb_page($siteObj, $pageObj, $content);
}


function sb_apartment_list($siteObj, $pageObj, $apts, $apts_subset, $num_pages, $page_num) {

  // left side
  $left = sb_list($siteObj, $apts);

  // right side (apt item list)
  $item_list= sb_item_list($siteObj, $apts_subset);
  $paging =  sb_paging($siteObj, $pageObj, $num_pages, $page_num);
  $right = '
        <div class="pageNav">
        '.$select_items.'
        '.$paging.'
        </div><!-- pageNav -->

        '.$item_list.'

    <div class="pageNavBottom">
        '.$paging.'
    </div>  <!-- pageNavBottom -->
  ';


  $content .= sb_apt_container($left, $right);
  return sb_page($siteObj, $pageObj, $content);
}

function sb_header($siteObj) {
  $banner = html_media($siteObj->banner_src, $siteObj->banner_height, $siteObj->banner_width);
  $output = '
    <div id="header">
'. $banner .'
    </div><!-- header -->
';
  return $output;
}

function sb_footer($siteObj) {
  $email = $siteObj->email_address;
  if(!$email) {
    $domain = url2domain($siteObj->url);
    $email = "info@".$domain;
  }
  $name  = $siteObj->name;
  $output = '
    <div id="footer">
    <p><span class="blue">'.$name.'</span> <br/ >&copy; 2011 - <a href="mailto:'.$email.'">'.$email.'</a></p>
    </div><!-- footer -->
';
  return $output;
}

function sb_page_content($siteObj, $pageObj) {
  $content = '
    <div id="content">
    <span class="title">'.$pageObj->title.'</span>
       <div id="contentTxt">
    '.$pageObj->content.'
       </div><!-- contentTxt -->
    </div><!-- content -->
  ';
  return custom_box($content, $siteObj->image_directory, $siteObj->content_background_color, $siteObj->content_width, $siteObj->content_height, $siteObj->content_border, $siteObj->content_class);
}

function sb_list($siteObj, $apts, $selected=0) {
  $i=1;
  //dump($apts);die();
  foreach($apts as $apt) {
    $list_items .= $selected && $apt->name == $selected ?
      ('<li class="activeItem">'.$apt->name."</li>\n") :
      ('<li>'.html_link($apt->sb_link, $apt->name).'</li>');
    $i++;
  }
  $list = '<ul>'.$list_items.'</ul>';

  //html_unordered_list($links, false);
  return '<div class="apTitle">'.html_link($pageObj->script, $siteObj->type_name_plural).'</div><!-- apTitle -->'.$list;
}


function sb_apt_selectitems() {
  return ''; // turned off
  $html = '
              <div id="selectItems">
                <select>
                <option value="">Pricing</option>
                <option value="&euro;250 - &euro;500">&euro;250 - &euro;500</option>
                <option value="&euro;500 - &euro;750">&euro;500 - &euro;750</option>
                </select>
                <select>
                <option value="">Location</option>
                <option value="Centrum">Centrum</option>
                </select>
              </div> <!-- selectItems -->
  ';
  return $html;
}


function sb_apt_container($left, $right) {
  $html = '

        <div id="containerApartments">
          <div id="containerApLeft">
  '.$left.'
          </div><!-- containerApLeft -->

          <div id="containerApRight">
  '.$right.'
          </div><!-- containerApRight -->
        </div><!-- containerApartments -->
  ';
  return $html;
}

function sb_paging($siteObj, $pageObj, $pages, $page_num) {
  //$script = $pageObj->script;
  //list($base, $ext) = explode('.', $pageObj->script);
  //$base = $siteObj->property_type;
  $base = $siteObj->type_name_plural;
  $ext = 'html';

  $prev_url = $page_num==1 ? $script : $base."_".($page_num-1).".".$ext;
  if($page_num > 1) $paging = '<a href="'.$prev_url.'"><img class="mar" src="/i/arrowPrev.gif" alt="" /></a>';
  //echo("there are $pages pages - this is number $page_num<br>");
  for($p=1;$p <= $pages;$p++) {
    $url = $base."_".$p.".".$ext;
    $link = $p == $page_num ? html_link("#", "<span class=\"pageActive\">$p</span>") : html_link($url, "$p");
    $paging .= " $link ";
  }

  if($page_num < $pages) $paging .= '<a href="'.$base.'_'.($page_num+1).'.'.$ext.'"><img class="mar" src="/i/arrowNext.gif" alt="" /></a>';

  $html = '
            <div class="paging">'.$paging.'</div><!-- paging -->
  ';
  //echo($html);
  return $html;
}

function sb_item_details($siteObj, $apt) {
  $type = $siteObj->property_type;
  //$image_server = "http://www2.".$siteObj->domain;
  if($siteObj->show_map) {
    $map_html = "<div id=\"big_map_container\">
                  <div id=\"big_map\"></div><!-- big_map -->
                 </div><!-- big_map_container -->";
    $js_event = "showEl('big_map_container');load('big_map', '$apt->location_lat', '$apt->location_long', '13');";
    $js_event2 = "hideEl('big_map_container');";
  }
  
  $first_pic = $image_server.get_first_pic($apt->id, array('size' => 'small', 'show_default' => true, 'class' => 'detailPic'));
  $gallery = $image_server.get_image_path($apt->id, $type)."gallery.xml";
  //echo("gallery = '$gallery', fp='$first_pic'<br>");
  
  $gallery_color = '101010';
  $preload_color = '00a0af';
  $h=400;
  $w=620;
  if($siteObj->hourly) {
    $price_str  = $apt->rate_hour_min > 0 ? "From &euro; $apt->rate_hour_min - &euro; $apt->rate_hour per hour " : "&euro; $apt->rate_hour per hour";
  } else {
    $price_str  = $apt->rate_day_min > 0 ? "From &euro; $apt->rate_day_min - &euro; $apt->rate_day per day " : "&euro; $apt->rate_day per day";
  }
//      <div id="containerDetail">
//        <div id="containerDetailMid">
//        </div><!-- containerDetailMid -->
//        <div id="containerDetailBottom"></div><!-- containerDetailBottom -->
//      </div><!-- containerDetail -->

  $details = details($apt, false);

  // todo: not just boats
  $request_link = $apt->book_direct == DB_REQUIRED ? '/boats.php?sn='.$apt->id : '/request.php?apt_id='.$apt->id.'&apt_name='.$apt->name.'&pt='.$apt->type;
  
  // http://dev.apartments-for-rent.com/aid1035/amsterdam/boat/?sn=101183
  
  $html = '
          <div id="detailItem">
            '.$first_pic.'
            <p class="detailName">
              <span class="houseTitleDetail">'.$apt->name.'</span><br />
              '. $price_str .'
            </p>
            <p class="links">
              <a class="more" href="'.$request_link.'">Request this '.$siteObj->type_name.'</a>
            </p>

          </div><!-- detailItem -->

          <div id="usual1" class="usual">
            <ul>
            <li><a class="selected" href="#tab1"  onclick="'.$js_event2.'">Description</a></li>
            <li><a href="#tab2" onclick="'.$js_event2.'">Details</a></li>
            <li><a href="#tab3" onclick="'.$js_event2.'">Photos</a></li>
';
  if($siteObj->show_map) $html .= '
            <li><a href="#tab4" onclick="'.$js_event.'">Location</a></li>
';
  $html .= '
            </ul>

            <div id="tab1">
              '.$apt->description.'
            </div> <!-- tab 1 -->

            <div id="tab2">
              '.$details.'
            </div> <!-- tab 2 -->

            <div id="tab3">
            	<div id="flashcontent" style="background-color:#'.$gallery_color.'">
            	  SimpleViewer requires Macromedia Flash. <a href="http://www.macromedia.com/go/getflashplayer/">Get Macromedia Flash.</a>
            	</div>
            	<script type="text/javascript">
            		var fo = new SWFObject("/f/viewer.swf", "viewer", "'.$w.'", "'.$h.'", "7", "#'.$gallery_color.'");
            		fo.addVariable("preloaderColor", "0x'.$preload_color.'");
            		fo.addVariable("xmlDataPath", "'.$gallery.'");
            		fo.write("flashcontent");
            	</script>
            </div> <!-- tab 3 -->

            <div id="tab4">
            </div> <!-- tab 4 -->

          </div><!-- usual -->
          <script type="text/javascript">
            $("#usual1 ul").idTabs();
          </script>

          '.$map_html.'
  ';

  return custom_box($html, $siteObj->image_directory, $siteObj->itemdetail_background_color, $siteObj->itemdetail_width, $siteObj->itemdetail_height, $siteObj->itemdetail_border, $siteObj->itemdetail_class);

}

function sb_item_list($siteObj, $items) {
  foreach($items as $item) $list .= "<br />".sb_item($siteObj, $item);
  return $list;
}

function sb_item($siteObj, $apt) {

  $type = $siteObj->property_type;
  $cutoff_length = 120;
  $apt_description = smart_truncate($apt->description, $cutoff_length);
  $apt_details= details($apt, true);
  //$price_str  = $apt->rate_day_min > 0 ? "From &euro; $apt->rate_day_min per day" : "&euro; $apt->rate_day per day";

  if($siteObj->hourly) {
    $price_str  = $apt->rate_hour_min > 0 ? "From &euro; $apt->rate_hour_min - &euro; $apt->rate_hour per hour " : "&euro; $apt->rate_hour per hour";
  } else {
    $price_str  = $apt->rate_day_min > 0 ? "From &euro; $apt->rate_day_min - &euro; $apt->rate_day per day " : "&euro; $apt->rate_day per day";
  }
  
  //$image_server = "http://www2.".$siteObj->domain;

  $first_pic = $image_server.get_first_pic($apt->id, array('size' => 'small'));
  //echo("First pick for $apt->name = '$first_pic' type='$type'<br>");
  if($siteObj->show_map) {
    $map_id = "map".$apt->sb_index;
    $map_html = '<span class="map">
                    <a name="'.$map_id.'"></a>
                    <a href="#'.$map_id.'"
                       onclick="showEl(\''.$map_id.'_container\');load(\''.$map_id.'\', \''.$apt->location_lat.'\', \''.$apt->location_long.'\'); return false;">Show Map</a>
                   </span>
                   <div class="smallMapContainer" id="'.$map_id.'_container">
                    <a href="#'.$map_id.'" onclick="hideEl(\''.$map_id.'_container\');return false;" style="color:#fff;padding:2px">X</a><br>
                    <div class="smallMap" id="'.$map_id.'"></div><!-- smallMap -->
                   </div><!-- smallMapContainer -->
                 ';
  }
  $request_link = $apt->book_direct == DB_REQUIRED ? '/boats.php?sn='.$apt->id : '/request.php?apt_id='.$apt->id.'&apt_name='.$apt->name.'&pt='.$apt->type;

  $item = '
          <div class="whiteContent">
            <div class="whiteLeft">
              <a href="'.$apt->sb_link.'">'.$first_pic.'</a>
            </div><!-- whiteLeft -->
            <div class="whiteCen">
              <span class="houseTitle"><a href="'.$apt->sb_link.'">'.$apt->name.'</a></span>
              '.$map_html.'
              <br><br>
              '.$apt_description.'
              <p class="links">
              <a class="more" href="'.$request_link.'">Request this '.$siteObj->type_name.'</a>
              <a class="more" href="'.$apt->sb_link.'" style="margin-right:5px">More Information</a></p>
            </div><!-- whiteCen -->
            <div class="whiteRight">
              <p style="font-weight:bold">
              '.$price_str.'<br />
              '.$apt_details.'
              </p>
            </div><!-- whiteRight -->
          </div><!-- whiteContent -->

  ';
  return custom_box($item, $siteObj->image_directory, $siteObj->itemlist_background_color, $siteObj->itemlist_width, $siteObj->itemlist_height, $siteObj->itemlist_border, $siteObj->itemlist_class);
}

function sb_upload($siteObj) {

  $echo = false;
  $dryrun = false;
  $testServerObj = $siteObj->dev_server ? get_object('server', $siteObj->dev_server) : die("No dev server for $siteObj->domain");
  $liveServerObj = $siteObj->live_server ?get_object('server', $siteObj->live_server) : die("No live server for $siteObj->domain");
  $local = same_network($testServerObj->ip, $liveServerObj->ip);
  
  $live_server = $liveServerObj->hostname; // 'diz.knxx.net';
  $url = add_http($siteObj->url);

  $local_dir = $siteObj->test_directory;
  $remote_dir = $siteObj->directory;
  $ldir = rtrim($local_dir, '/'); // strip last slash
  $rdir= rtrim($remote_dir, '/'); // strip last slash

  $live_key = $siteObj->googlemap_live_key;
  $test_key = $siteObj->googlemap_test_key;

  execRootCmd("rpl -R '$test_key' '$live_key' $ldir", $output); // replace test key with live key
  if($local)  execRootCmd("rm -rf $rdir", $output);
  $command = $local ? "cp -ax $ldir $rdir" : "rsync -az $ldir/ $live_server:$rdir";
  $action = $local ? "Copied" : "Uploaded";
  echo(execRootCmd($command, $output));//dump($output);
  echo ("<hr>Command=$command<h3>$action to <a href='$url'>live site</a></h3>".dump($output));  
  execRootCmd("rpl -R '$live_key' '$test_key' $ldir", $output); // put test key back

  if(!$local) { // sync photos for amsterdam
    echo("<b>Not Local</b><br>"."sudo rsync -az /web/images/afr/apts/1/1000/ $live_server:/web/images/afr/apts/1/1000/<br>");
    shell_command("sudo rsync -az /web/images/afr/apts/1/1000/ $live_server:/web/images/afr/apts/1/1000/", $echo, $dryrun);
  }

  //dump($siteObj);

  // junk ?
  // $dev_server = $siteObj->test_url;
  // $live_server = $siteObj->url;
  // 
  // $test_engine_server = "test2.".$siteObj->domain;
  // $live_engine_server = "www2.".$siteObj->domain;
  // 
  // //$dev_server = "dev.apartments-for-rent.com";
  // $afr_server = "www.apartments-for-rent.com";


  // execRootCmd("rpl -R '$test_engine_server' '$live_engine_server' $ldir", $output);//dump($output);
  // replace dev server with live server
  //shell_command("rpl -R '$dev_server' '$live_server' $dir", $echo, $dryrun);

  // replace URLs in images
//  shell_command("sudo rpl -R '$test_engine_server' '$live_engine_server' $ldir", $echo, $dryrun);
//  echo("sudo rpl -R '$test_engine_server' '$live_engine_server' $ldir<br>");
//  die();
  //execRootCmd("rpl -R '$afr_server' '$live_server' $ldir", $output);//dump($output);
  //execRootCmd("rpl -R '$dev_server' '$live_server' $ldir", $output);//dump($output);

  // execRootCmd("rpl -R '$live_engine_server' '$test_engine_server' $ldir", $output);//dump($output);


}

// *** server setup code *** //

function server_setup($siteObj, $echo=false, $execute=false) {
  die("gh - dev");
  set_time_limit(9000); // increase max allowed execution time.

  if($_SESSION['dev'] || $_SESSION['test']) {
    $execute = false;
    $echo = true;
  }

  echo("Running server setup for $siteObj->name ($siteObj->id)<br>");  
  $nameServerObj = get_object('server', NAMESERVER_ID) ?: die("server_setup: Nameserver not found");
  $appServerObj  = get_object('server', $siteObj->app_server) ?: die("server_setup: Application server not found");
  $devServerObj  = get_object('server', $siteObj->dev_server) ?: die("server_setup: Dev. server not found");;
  $liveServerObj = get_object('server', $siteObj->live_server) ?: die("server_setup: Live server not found");;
  $dev_server_ip = $devServerObj->ip; //'195.66.135.149';
  $app_server_ip = $appServerObj->ip; //'195.66.135.147';
  $live_server_ip = $liveServerObj->ip; // 69.59.146.254
  
  $timestamp = date("Ymd")."00";
  
  // site variables
  $domain = url2domain($siteObj->url);
  $shortname = $siteObj->shortname;
  list($dom, $tld) = explode('.', $domain);
                           
  // local server settings  
  $local_server = $devServerObj->hostname;
  $local_log_dir = $devServerObj->logdir;            
  $local_web_root = $devServerObj->webroot;
  $local_web_dir = $local_web_root.'test.'.$domain.'/'; //$siteObj->test_directory;
  $local_script_dir = $local_web_dir."sh/";
  $local_apachename = $devServerObj->apachename ?: 'apache2';;
  $local_apache_dir = "/etc/$local_apachename/";
  $local_apache_file = $local_apache_dir."sites-available/test.$domain";
  $local_apache_tmp_file = $local_script_dir."apache_dev_$dom";
  $local_script_file = $local_script_dir."local.sh";  
  $local_master_file = $local_script_dir."master_$dom";
  $local_remotescript_file = $local_script_dir."remote.sh";
  $local_nameserverscript_file = $local_script_dir."nameserver.sh";
  $local_remoteapache_tmp_file = $local_script_dir."apache_live_$dom";
  $local_bind_file = $local_script_dir."bind_$dom"  ;
  $local_web_dir_noslash = rtrim($local_web_dir, '/');
    
  // remote server
  $remote_server = $liveServerObj->hostname;
  $remote_log_dir = $liveServerObj->logdir;            
  $remote_web_root = $liveServerObj->webroot; // "/websites/";   
  $remote_web_dir = $remote_web_root.'www.'.$domain.'/'; //$siteObj->test_directory;
  $remote_script_dir = $remote_web_dir."sh/";
  $remote_apachename = $liveServerObj->apachename ?: 'apache2';
  $remote_apache_dir = "/etc/$remote_apachename/";
  $remote_apache_file = $remote_apache_dir."sites-available/www.$domain";
  $remote_apache_tmp_file = $remote_script_dir."apache_live_$dom";
  $remote_script_file = $remote_script_dir."remote.sh";  
  
  // nameserver setup
  // hard coded for now (we use only one nameserver)

  $nameserver = $nameServerObj->hostname;
  $rcpthosts  = '/var/qmail/control/rcpthosts';
  $named_conf = '/etc/bind/named.conf.local';
  $virtual_domains ='/var/qmail/control/virtualdomains';
  $nameserver_bind_file = "/etc/bind/zones/db.".$domain;
  $nameserver_tmp_dir = "/home/phpexec/autoconfig/";
  $nameserver_master_tmp_file = $nameserver_tmp_dir."master_$dom";
  $nameserver_bind_tmp_file = $nameserver_tmp_dir."bind_$dom";
  $nameserver_script_file = $nameserver_tmp_dir."nameserverscript_$dom.sh";   
  
  // rewrite rules
  //   $htaccess = '
  // RewriteEngine On                                                             
  // RewriteRule ^shortstay-(.*)$ /apts/shortstay-$1                              
  // RewriteRule ^longstay-(.*)$ /apts/longstay-$1
  // RewriteRule ^realestate-(.*)$ /apts/realestate-$1                                                                                                                                                                                      
  //   ';
  //   $htaccess_file = $local_web_dir.'.htaccess';
  
  $master = '
zone "'.$domain.'" {
    type master;
    file "db.'.$domain.'";
    allow-transfer { knxxns; };
};

';

  $bind = '; BIND db file for '.$domain.'
$TTL    900
@       IN  SOA ns1.knxx.net.      ns.knxx.net. (
                '.$timestamp.'      ; Serial Number: YYYYMMDDNN
                10800           ; Refresh every 3 hours
                1800            ; Retry every 30 minutes
                604800          ; Expire in 7 days
;                86400 )         ; Default/Minimum TTL: 1 day
                900 )           ; TTL 15 min.

; Name Servers
                IN NS   ns1.knxx.net.
                IN NS   ns1.servepath.com.

; Master A record and MX records
                IN A    '.$live_server_ip.'
                IN MX   10 mail.knxx.net.
                IN MX   50 mail2.knxx.net.

$ORIGIN '.$domain.'.

; Machines in domain
www             IN A    '.$live_server_ip.'
test            IN A    '.$dev_server_ip.'
admin           IN A    '.$dev_server_ip.'
dev             IN A    '.$dev_server_ip.'
staging         IN A    '.$dev_server_ip.'
';


  $apache_remote = '
#'.$domain.'
<VirtualHost '.$live_server_ip.':80>
  ServerName www.'.$domain.'
  ServerAlias '.$domain.'
  DocumentRoot '.$remote_web_dir.'
  DirectoryIndex index.php index.html index.htm
  ErrorLog '.$remote_log_dir.'error_'.$domain.'
  TransferLog '.$remote_log_dir.'access_'.$domain.'
  AddDefaultCharset utf-8
  Options ExecCGI FollowSymLinks Includes MultiViews
</VirtualHost>
';

  $apache_local = '
#'.$domain.'
<VirtualHost '.$dev_server_ip.':80>
  ServerName test.'.$domain.'
  ServerAlias test2.'.$domain.' staging.'.$domain.' dev.'.$domain.' admin.'.$domain.' www2.'.$domain.'
  DocumentRoot '.$local_web_dir.'
  DirectoryIndex index.php index.html index.htm
  ErrorLog '.$local_log_dir.'error_'.$domain.'_php
  TransferLog '.$local_log_dir.'access_'.$domain.'_php
  AddDefaultCharset utf-8
  Options ExecCGI FollowSymLinks Includes MultiViews
</VirtualHost>
';                  

  // shell scripts
  $local_commands = array("### Dev. server scripts for $domain ### ");
  $remote_commands = array("### Live server scripts for $domain ### ");
  $nameserver_commands = array("### Nameserver server scripts for $domain ### ");

  // cp skel directory on local (dev) server
  if(!is_dir($local_web_dir)) execCmd("cp -ax /websites/autobuild.skel $local_web_dir_noslash");

  // local apache setup
  $local_commands[]= "cp $local_apache_tmp_file $local_apache_file";
  $local_commands[]= "cd ".$local_apache_dir."sites-enabled/";
  $local_commands[]= "ls test.$domain || ln -s ../sites-available/test.$domain";
  // $local_commands[]= "/etc/init.d/apache2 reload";  

  // sync webdir to remote (live) server
  $same_server = same_network($live_server_ip, $dev_server_ip);
  $local_commands[]= $same_server ? "cp -ax $local_web_dir $remote_web_dir" : "rsync -az $local_web_dir $remote_server:$remote_web_dir"; 

  // copy local tmp config files to nameserver
  $local_commands[]= "rsync -a $local_nameserverscript_file $nameserver:$nameserver_script_file"; 
  $local_commands[]= "rsync -a $local_bind_file $nameserver:$nameserver_bind_tmp_file"; 
  $local_commands[]= "rsync -a $local_master_file $nameserver:$nameserver_master_tmp_file";

  $remote_commands[]= "cp $remote_apache_tmp_file $remote_apache_file";
  $remote_commands[]= "cd ".$remote_apache_dir."sites-enabled/";
  $remote_commands[]= "ls www.$domain || ln -s ../sites-available/www.$domain";
  $remote_commands[]= "/etc/init.d/$remote_apachename reload";

  $nameserver_commands[] = "cp $named_conf $named_conf.bak";
  $nameserver_commands[] = "grep -c $domain $named_conf || cat $nameserver_master_tmp_file >> $named_conf";
  $nameserver_commands[] = "cp $rcpthosts $rcpthosts.bak";
  $nameserver_commands[] = "cp $nameserver_bind_tmp_file $nameserver_bind_file";
  $nameserver_commands[] = "grep -c $domain $rcpthosts || echo $domain >> $rcpthosts";
  $nameserver_commands[] = "cp $virtual_domains $virtual_domains.bak";
  $nameserver_commands[] = "grep -c $domain $virtual_domains || echo $domain:alias-$domain >> $virtual_domains";
  $nameserver_commands[] = "echo \"&".$siteObj->email_forward."\" > /var/qmail/alias/.qmail-$dom:$tld-default";
  $nameserver_commands[] = "chown root $nameserver_bind_file";
  $nameserver_commands[] = "chgrp bind $nameserver_bind_file";
  $nameserver_commands[] = "/etc/init.d/bind9 reload";
  $nameserver_commands[] = "/etc/init.d/qmail reload";
  $nameserver_commands[] = "grep $domain /var/log/daemon.log";

  $result['bind']                = $bind;
  $result['master']              = $master;
  $result['remote_apache']       = $apache_remote;
  $result['local_apache']        = $apache_local;
  $result['remote_commands']     = $remote_commands;
  $result['local_commands']      = $local_commands;
  $result['nameserver_commands'] = $nameserver_commands;


  // write .htaccess file
  // write_file($htaccess_file, $htaccess, 'w', $echo);
             
  $local_cmd_str      = implode(newline(), $local_commands).newline();     
  $remote_cmd_str     = implode(newline(), $remote_commands).newline();    
  $nameserver_cmd_str = implode(newline(), $nameserver_commands).newline();
                 
  // $execute = false;
  if($execute) {
    // write local (temporary) config files 
    write_file($local_master_file, $master, 'w', $echo);
    write_file($local_bind_file,   $bind, 'w', $echo);
    write_file($local_apache_tmp_file, $apache_local, 'w', $echo);
    write_file($local_remoteapache_tmp_file, $apache_remote, 'w', $echo);
  
    // write shell scripts
    write_file($local_script_file          , $local_cmd_str,      'w', $echo);
    write_file($local_remotescript_file    , $remote_cmd_str,     'w', $echo);
    write_file($local_nameserverscript_file, $nameserver_cmd_str, 'w', $echo);

    // command execution
    execRootCmd("chown phpexec $local_script_dir/*");
    execRootCmd("chgrp web $local_script_dir/*");
    execRootCmd("chmod u+x $local_script_dir/*.sh");
    execRootCmd("chmod g+w $local_script_dir/*");
                            
    // run local script
    execRootCmd($local_script_file, $local_output);
    echo("Local output contains ".count($local_output). " lines".html_break());
    $local_output_str      = implode(newline(), $local_output).newline();     
  
    // run script on live server
    if($same_server) {
      execRootCmd($remote_script_file, $remote_output);
    } else {
      execRemoteRootCmd($remote_server, $remote_script_file, $remote_output);
    }
    echo("Remote output contains ".count($remote_output). " lines".html_break());
  
    // run script on nameserver
    execRemoteRootCmd($nameserver, $nameserver_script_file, $nameserver_output);
    echo("Nameserver output contains ".count($nameserver_output). " lines".html_break());
  }
  
  if($echo) {
    $cols = 160;                                                                                     
    echo(html_subheader("Local commands: $local_server ($local_script_file)").             form_text('', $local_cmd_str,            array('rows' => count($local_commands),      'cols' => $cols)));
    echo(html_subheader("Remote commands: $remote_server ($local_remotescript_file)").     form_text('', $remote_cmd_str,           array('rows' => count($remote_commands),     'cols' => $cols)));
    echo(html_subheader("Nameserver commands: $nameserver ($local_nameserverscript_file)").form_text('', $nameserver_cmd_str,       array('rows' => count($nameserver_commands), 'cols' => $cols)));
    if($execute) {                                                                                                                  
      echo(html_subheader("Local output: $local_server ($local_script_file)").               form_text('', $local_output_str,       array('rows' => count($local_output),        'cols' => $cols)));
      echo(html_subheader("Remote output: $remote_server ($remote_script_file)").            form_text('', $remote_output_str,      array('rows' => count($remote_output),       'cols' => $cols)));
      echo(html_subheader("Nameserver output: $nameserver_server ($nameserver_script_file)").form_text('', $nameserver_output_str,  array('rows' => count($nameserver_output),   'cols' => $cols)));
    }
  }

  // show shell scripts  
  
  //foreach($local_commands as $cmd)  {
  //  if($cmd && strpos($cmd, '#') !== 0) {
  //    if($execute) $result['local_command_output'][] = shell_exec("sudo ".$cmd);
  //    //echo("$cmd<br>");
  //  }
  //}
  //
  //foreach($remote_commands as $cmd) {
  //  if($cmd && strpos($cmd, '#') !== 0) {
  //    $rcmd = "sudo ssh root@$remote_server '".str_replace("'", "''", $cmd)."'";
  //    if($execute) $result['remote_command_output'][] = shell_exec($rcmd);
  //    //echo("$rcmd<br>");
  //  }
  //}
  
  return $result;
}


function sb_google_search($siteObj, $lang='en') {
  $server = $siteObj->test_url;
  return ('
<!-- SiteSearch Google -->
<form method="get" action="http://'.$server.'/">
<table border="0" bgcolor="#'.$siteObj->right_box_background_color.'">
<tr><td style="white-space: nowrap;" valign="top" align="left" height="32">

<br/>
<input type="hidden" name="domains" value="'.$server.'"></input>
<label for="sbi" style="display: none">Enter your search terms</label>
<input type="text" name="q" size="31" maxlength="255" value="" id="sbi"></input>
<label for="sbb" style="display: none">Submit search form</label>
<input type="submit" name="sa" value="Google Search" id="sbb"></input>
</td></tr>
<tr>
<td style="white-space: nowrap;" >
<table>
<tr>
<td>
<input type="radio" name="sitesearch" value="" checked id="ss0"></input>
<label for="ss0" title="Search the Web"><font size="-1" color="#000000">Web</font></label></td>
<td>
<input type="radio" name="sitesearch" value="'.$server.'" id="ss1" CHECKED></input>
<label for="ss1" title="Search '.$server.'"><font size="-1" color="#000000">'.$server.'</font></label></td>
</tr>
</table>
<input type="hidden" name="client" value="pub-6293044799390826"></input>
<input type="hidden" name="forid" value="1"></input>
<input type="hidden" name="channel" value="4784782340"></input>
<input type="hidden" name="ie" value="UTF-8"></input>
<input type="hidden" name="oe" value="UTF-8"></input>
<input type="hidden" name="safe" value="active"></input>
<input type="hidden" name="flav" value="0000"></input>
<input type="hidden" name="sig" value="8FzbI7Kl9ubTtibG"></input>
<input type="hidden" name="cof" value="GALT:#F7BF01;GL:1;DIV:#AEAEAE;VLC:F7BF01;AH:center;BGC:F4F4F4;LBGC:F3F3F3;ALC:3E78A6;LC:3E78A6;T:000000;GFNT:0000FF;GIMP:0000FF;LH:50;LW:132;L:http://www.apartments-for-rent.com/images/logo_384.gif;S:http://www.apartments-for-rent.com/;FORID:11"></input>
<input type="hidden" name="hl" value="'.$lang.'"></input>
</td></tr></table>
</form>
<!-- SiteSearch Google -->
');
}

function sb_search_result($width='680', $lang='en') {
  return ("
<!-- Google Search Result Snippet Begins -->
<div id=\"googleSearchUnitIframe\" background=\"#f00\"></div>

<script type=\"text/javascript\">
   var googleSearchIframeName = 'googleSearchUnitIframe';
   var googleSearchFrameWidth = ".$width.";
   var googleSearchFrameborder = 2 ;
   var googleSearchDomain = 'www.google.com';
</script>
<script type=\"text/javascript\"
         src=\"http://www.google.com/afsonline/show_afs_search.js\">
</script>
<!-- Google Search Result Snippet Ends -->
");
}


function get_default_layout($siteObj) {
  $css = array(

  /* -------------------- MAIN --------------------  */

    'body_background_color'      => '338',
    'title_background_color'     => '0094a7',
    'header_color'    => '00a0af',
    'content_color'   => 'fff'   ,
    'content_background_color'   => '44f',
    'nav_color'       => 'fff'   ,
    'nav_background_color'       => '000'   ,
    'subNav_color'    => 'fff'   ,
    'subNav_background_color'    => '232349',
    'subNav_hi'    => '0093a8',
    'subNav_link'  => 'fff'   ,
    'footer_color'    => 'fff'   ,
    'footer_link'  => 'fff'   ,

    'banner_height'     => 275,
    'banner_width'     => 850,
    'banner_src'   => '/i/banner.jpg',

//    'right_box_src'       => '../i/download.jpg',
    'right_box_background_color'       => 'ccc',
    'right_box_height'       => 311   ,
    'right_box_width'       => 357   ,
    'right_col_width'       => 357   ,
    'right_box_src'   => '',
    'left_col_width'       => 480   ,

  /* -------------------- APARTMENTS --------------------  */

    'apTitle_background_color'              => '0094a7',

    'containerApLeft_color'      => 'fff',
    'containerApLeft_background_color'      => '262626',
    'containerApLeft_hi_color'   => '0094a7',
    'containerApLeft_hi_background_color'   => 'fff',
    'containerApLeft_width'       => 122,

    'containerApLeft_sel_color'  => '262626',
    'containerApLeft_sel_background_color'  => '0094a7',
    'containerApRight_color'     => 'fff',
    'containerApRight_width'     => 682,

    'pageNav_background_color'              => '000',
    'pageNav_color'              => 'fff',
    'pageNav_sel_background_color'          => '00b0d8',

  /* -------------------- ROUND BOXES --------------------  */

    'content_color'                   => 'fff',
    'content_background_color'                   => '232349',
    'content_border'               => '',
    'content_width'                    => 480,
    'content_height'                    => 0,

    'itemlist_color'                  => 'fff',
    'itemlist_background_color'                  => '232349',
    'itemlist_border'              => '',
    'itemlist_width'                   => 678,
    'itemlist_height'                   => 'fff',

    'itemdetail_color'                => 'fff',
    'itemdetail_background_color'                => '232349',
    'itemdetail_border'            => '',
    'itemdetail_width'                 => 700,
    'itemdetail_height'                 => '0094a7',

  );

  return $css;
}

function custom_box($content, $image_dir, $color, $width, $height='', $border='', $class="") {
  if(!$color) $color = "232349";
  $str = custom_top($image_dir, $color, $width, $height, $border, $class);

  if($height) $height='height:'.$height.'px;';
  //$iw = $width - 24;
  $iw = $width -2;
  if($border) $border_s = "border-left: 1px solid #$border;border-right: 1px solid #$border";
  $str.="<div class='round_container' style='background:#$color;width:".$iw."px;border:1px solid #$color;$border_s;$height;'>\n
          $content\n
          </div><!-- round_container -->\n";

  $str .= custom_bottom($image_dir, $color, $width, $border, $class);
  return $str;          
}

function custom_top($image_dir, $color, $width, $height="", $border="", $class="") {
  $w=$width-12;
  $l=$width-6;
  $tl=$image_dir."TL_".$color.".gif";
  $tr=$image_dir."TR_".$color.".gif";
  if($border) {
    $border_t = "border-top: 1px solid #$border";
    $tlb = $image_dir."TLB_".$color.".gif";
    $trb = $image_dir."TRB_".$color.".gif";
  }
  $str = "<div class='round_box' style='position:relative;width:".$width."px;'>\n";
  $str.=" <div class='round_top' style='width:100%;position:relative;height:6px;vertical-align:top;'>\n
          <span class='round_corner' style='left:0px;z-index: 10;'><img src='$tl' alt='corner'></span>\n";
  if($border) {
    $str.="<span class='round_corner_tl' style='left:0px;z-index: 20;'><img src='$tlb' alt='corner'></span>\n";
    $str.="<span class='round_corner_tr' style='left:".$l."px;z-index: 20;'><img src='$trb' alt='corner'></span>\n";
  }
  $str.="  <span class='round_bar'    style='width:".$w."px;background: #$color;$border_t'>&nbsp;</span>\n";
  $str.="  <span class='round_corner' style='left:".$l."px;z-index: 10;'><img src='$tr' alt='corner'>&nbsp;</span>\n";
  $str.="    </div><!-- round_top -->\n";
  return $str;
}

function custom_bottom($image_dir, $color, $width, $border="", $class="") {
  $w=$width-12;
  $l=$width-6;
  $width = "style='width:".$width."px'";
  $bl=$image_dir."BL_".$color.".gif";
  $br=$image_dir."BR_".$color.".gif";
  if($border) {
    $border_t = "border-top: 1px solid #$border";
    $blb = $image_dir."BLB_".$color.".gif";
    $brb = $image_dir."BRB_".$color.".gif";
  }
  $str.="  <div class='round_bottom' style='width:100%;position:relative;height:6px;vertical-align:top'>\n
          <span class='round_corner' style='left:0px;z-index: 10;'><img src='$bl' alt='corner'></span>\n";
  if($border) {
    $str.="<span class='round_corner_bl' style='left:0px;z-index: 20;'><img src='$blb' alt='corner'></span>\n
          <span class='round_corner_br' style='left:".$l."px;z-index: 20;'><img src='$brb' alt='corner'></span>\n";
  }

  $str.="  <span class='round_bar'    style='width:".$w."px;background: #$color;$border_b'>&nbsp;</span>\n
          <span class='round_corner' style='left:".$l."px;z-index: 10;'><img src='$br' alt='corner'>&nbsp;</span>\n";
  $str.="  </div><!-- round_bottom -->\n";
  $str.="</div><!-- title_box or round_box -->\n";
  return $str;
}

<?php

function property_list_simple($siteObj_or_id, $ptype, $objs_or_ids, $options=NULL) {

  if(is_array($objs_or_ids)) $objs_or_ids = array_filter($objs_or_ids);
  if(!$objs_or_ids || !is_array($objs_or_ids)) {
    $output = "property_list_simple: on result";
    return $output;
  }
  
  $siteObj = any2obj('site', $siteObj_or_id);
  $options = property_config($siteObj, $pageObj, $options);
  $options["hide_qs"] = true; // don't show query params in link 
  
  print_log(dump($options, true), 'ajax-property', LOG_LEVEL_TEST);  
  $class     = $options['class'] ?: 'inner_width_narrow';
  $i=0;
  $properties = array();
  foreach($objs_or_ids as $objs_or_id) {

    $apt = is_object($objs_or_id) ? $objs_or_id : get_object($ptype, $objs_or_id);
    if(!$apt->id) die("property_list_simple ptype=$ptype id=$id: ".caller_stack().dump($apt, true)."ids=".dump($ids, true));
    
    $bg_class = $i%2 ? 'box_medium_color' : 'box_lite_color';
    $can_edit = $options['can_edit'] = auth_rental($apt);
    $options['index'] = $i;
    $properties[] = html_div(property_print($siteObj, $pageObj, $apt, $options), 'col-md-6 col-lg-4');
    
    //$js = 'id="property-'.$apt->id.'"';    
    //$output .= $options['box_list_item'] ? 
    //  round_box($property, "border $class property-list-box $bg_class", array('js' => $js)) :
    //  html_div($debug.$property, '', $js);
    $i++;
  }
  
  return html_div(implode(PHP_EOL, $properties), 'row', array("id" => "property-simple-list", "data-facs" => $options['facs'], "data-max_guests" => $options['max_guests'], "data-min_price" => $options['min_price'], "data-max_price" => $options['max_price'], "data-count" => count($objs_or_ids)));
}

function property_list($siteObj, $pageObj, $options=NULL) {
  global $__header;
  $debug = $_GET['debug'] == 'print' && !$_SESSION['live'];

  $options = property_config($siteObj, $pageObj, $options);
  //dump($options);
  $search    = $options['search'] ?: array();
  $searchObj = $options['searchObj'] ?: array(); // query param styloe
  $can_edit  = $options['can_edit'];  
  $class     = $options['class'] ?: 'inner_width_narrow';
  $type      = $options['property_type'];  
  $apts      = $options['apts'] ?: array();  
  $sort      = $options['sort'] = pick_first($options['sort'], $_SESSION['property_sort'][$type], 'priority DESC, book_direct DESC, created DESC');
  $start     = $options['start'] ?: 0;
  $limit     = $options['limit'] ?: 0;
  
  if(count($apts)) {
    if($debug) {$apt_ids2 = array_find_key($apts, 'id');echo(html_div("sort = $sort order=$order<br>pp result1=".implode(",", $apt_ids2).html_break(), 'inner_width_narrow'));}
  } else { // not passed, do search/sort/exceptions here (when called from app_data.php for ajax_load)
    $searchObj = $options['searchObj'];
    if($debug) $output .= html_b("No properties, searching").html_break();// .dump($options, true);
    
    if($searchObj) {
      //echo("object search");dump($searchObj);
      list($apts, $errors, $message, $searchArray) = property_search($searchObj, array('sort' => $sort, 'order' => $order));
    } else {
      //echo("array search");dump($search);
      $apts = find_objects($type, $search, $options);
    }

    if($debug) {$apt_ids2 = array_find_key($apts, 'id');echo(html_div("sort = $sort order=$order<br>pp result2=".implode(",", $apt_ids2).html_break(), 'inner_width_narrow'));}
    
  }
  $all_apts = $apts;

  $all_count = count($all_apts);
  $count = $options['count'] = count($all_apts); 
    
  //if($start || $limit) 
  $apts = property_sort_slice($all_apts, $sort, $order, $start, $limit);  
  $apt_ids3 = array_find_key($apts, 'id');
  if($debug) echo("pp result3=".implode(",", $apt_ids3).html_break());


  if($options['ajax_scroll']) { // automatically load next 
    //unset($options['searchObj']);
    //unset($search['id']);
    //unset($search['min_stay']);
    //unset($search['max_guests']);
              
    $js_options = array('property_type' => $type, 'count' => $count, 'start' => $start, 'limit' => $limit); // clear
    //$js_options['search'] = $search;
    if($searchObj && !$searchObj->city_id) $searchObj->city_id = $city_id ?: $_SESSION['city_id']; // why do we need this hack
    $js_options['searchObj'] = $searchObj;
    $js_options['search'] = $search;
    
    //unset($js_options['apts']);
    $load_more_json = json_encode($js_options, true);
    $load_more_json_div = html_div($load_more_json, 'hidden load_more_json');
  
    // debug
    if($debug) {
      foreach($all_apts as $apt) $all_apt_names[] = $apt->id;
      foreach($apts as $apt)  $slice_apt_names[] = $apt->id;
      $output .= html_div("start=$start count=$count limit=$limit", 'inner_width_narrow');//.dump($js_options, true).$output;
      $output .= html_div("All ($all_count): ".implode(',', $all_apt_names), 'inner_width_narrow');//.dump($js_options, true).$output;
      $output .= html_div("Slice $start + $limit: ".implode(',', $slice_apt_names), 'inner_width_narrow');//.dump($js_options, true).$output;
    }
    
    $output .= $load_more_json_div;

    
  //$options['ajax_scroll'] = true;
    $__header['scripts'][] = '/jquery/appear/jquery.appear-1.1.1.min.js';
    //$output = "start=$start count=$count limit=$limit <br><pre>".print_r($search, true)."</pre>";
    //$output = "start=$start count=$count limit=$limit <br>";
    $load_more = '$(".load_more").remove();';
    if($limit && $count > $limit) { // limit output until scrolled
      $fresh_page = $start==0 ? 1 : 0;
      $start += $limit;
      $options['start'] = $js_options['start'] = $start;
      //$output .= "Json<pre>".print_r($load_more_json, true)."</pre>";
      
      $load_more .= '
        var start = '.$start.';
        if('.$fresh_page.') last_loaded = 0; // new page, reset
        if(start >= last_loaded) {
          //alert("appending");
          $("<div class=\"load_more\" id=\"load_more_'.$start.'\"></div>").insertAfter($("#property-list"));
          $("#load_more_'.$start.'").appear(function() {debug("appear");load_more("property-list", start);},{"one" : false});
        }
      ';
    }
  } else {
    if($limit && $count > $limit) { // show next button
      $onclick = 
      $next_button = html_div(phrase('next', CAPITALIZE), 'zocial primay');
    }
    if($start > 0) { // show prev button
      $prev_button;
    }
  }
  
  $i=0;
  //if($debug) {
  //  foreach($apts as $apt) echo("$apt->id: $apt->name<br>");
  //  die();
  //}
  foreach($apts as $apt) {
    $bg_class = $i%2 ? 'box_medium_color' : 'box_lite_color';
    $property = property_print($siteObj, $pageObj, $apt, $options);
    $ptype = $apt->type;
    $js = 'id="'.$ptype.'-'.$apt->id.'"';
    $output .= $options['box_list_item'] ? 
      round_box($property, "border $class property-list-box $bg_class", array('js' => $js)) :
      html_div($property, '', $js);
    //$output .= $property;;
    $i++;
  }

  if($options['show_popup_map']) {
    $output .= html_div('The map', 'gmap border', "id='gmap_popup'");   // div for small pop-up map
    $__header['scripts'][] = "/afr/js/afr_gmap.js";
    $map_options['id'] = 'gmap_popup';
    $map_options['zoom'] = '3';
    if($cityObj = get_object('geo_city', $apts[0]->city_id)) {
      $map_options['center'] = "$cityObj->location_lat,$cityObj->location_long";
      //$map_options['accuracy'] = $cityObj->location_accuracy;
    }
  
    $__header['onload'] .= '
    gmap3_maplink_init('.json_encode($map_options).');
  ';
  }
  //if($debug) {echo($output); die("done");}
  $output = $options['nodiv'] ? $output : html_div($output, 'property-list', 'id="property-list" rel="property_list"'.$style);
  if($load_more) $output .= html_script_inline($load_more);
  return $output;
}

// returns title for property
function property_title($apt, $link='') {
  $type = $apt->type;
  $name = $apt->name;
  if(in_array($type, array('longstay', 'realestate', 'office', 'shortstay_office'))) { // add postcode, city
    if($add_str = implode(' ', array_filter(array($apt->address_postcode, $apt->address_city)))) $name .= html_break().$add_str;
  }  
  return $link ? html_link($link, $name) : $name; 
}

function property_template($siteObj, $pageObj, $apt, $options=NULL) {
  //dump($options);
  // options
  
  if(!$apt) return "No apt";
  if(!$apt->id) die("property_template: ".caller_stack().dump($apt, true));
  $debug      = $options['debug'];
  $exceptions = $options['loop_exceptions']; // rate affecting date-intersect exceptions
  
  $template   = $options['template']  ?: $siteObj->template; 
  
  $details    = $options['details'];
  $searchObj  = $options['searchObj'];
  $can_edit   = $options['can_edit'];  
  $nav        = $options['nav'];  
  $total_w    = $options['width'] ?: $_SESSION['inner_width']; 
  $thumb_size = $options['thumb_size'] ?: 'small';
  $show_links = isset($options['show_links']) ? $options['show_links'] : true;
  $show_map   = isset($options['show_map']) ? $options['show_map'] : true;
  $site_id    = $siteObj->id;
  $id         = $apt->id;
  $type       = $apt->type;

  $gallery_count = property_gallery_count($apt);
  
  if($can_edit) $edit_link = property_edit_link($apt->type, $apt->id, false, phrase('page_'.$apt->type, CAPITALIZE), $_SERVER['REQUEST_URI']);
  //dump($apt);
  
  if($_SESSION['debug']) echo("Template=$template<br>");
  
  // picture and url
  $pic        = get_first_pic($apt, array('size' => $thumb_size));
  
  //if(!$options['hide_qs']) $query_string = ($qs = get_query_string() ? "?$qs" : '');
  $qs = $options['hide_qs'] ? '' : get_query_string();
  $url        = $apt->url = property_link($siteObj, $apt, array('qs' => $qs));
  $photo      = $apt->photo = $pic && $url ? html_link($url, $pic) : $pic;

  // HTML elements like price, description, feedback
  $name         = $details ? property_title($apt) : property_title($apt, $url);
  $description  = $options['hide_description'] ? '' : html_tidy(print_property_description($siteObj, $pageObj, $apt, $options));
  $descriptor   = property_descriptor($apt->type, $apt);
  $cur          = get_currency_symbol($apt->currency, 1);
  $price_fld    = property_price_field($apt->type);

  $rate = clean_num($apt->$price_fld);
  if($apt->exception_overlap) { // one or more (rate affecting) exceptions apply to some of the dates, call get_average_rate
    $debug = false;
    //list($unit, $units, $cur, $rate, $discount) = get_average_rate($apt, $searchObj->in, $searchObj->out, $searchObj->ng);
    $priceObj = get_average_rate($apt, $searchObj->in, $searchObj->out, $searchObj->ng);
    $unit = $priceObj->unit;
    $units = $priceObj->num_days;
    $rate = round($priceObj->average, 2);
    $discount = $priceObj->discount;      

    $apt->$price_fld = number_format($rate, 2);
    $apt->rate_extraperson = 0; // already calculated in get_average_rate
    if($debug) {
      echo("$apt->name overlap = $apt->exception_overlap - rate from $searchObj->in - $searchObj->out for $searchObj->ng guests = $rate<br>");
      dump($priceObj);
    }

  }
  $price_pp        = $rate > 0 ? property_price($apt, $searchObjClone ?: $searchObj, array('pp' => true)) : phrase('on_request', CAPITALIZE);
  $price_pp_only   = $rate > 0 ? property_price($apt, $searchObjClone ?: $searchObj, array('pp_only' => true)) : phrase('on_request', CAPITALIZE);
  $price           = $rate > 0 ? property_price($apt, $searchObjClone ?: $searchObj, array('pp' => false)) : phrase('on_request', CAPITALIZE);

  $rate = clean_num($apt->$price_fld);
  
  //echo("pf = $price_fld rate = $raw_price<br>");
  $min_guests   = $apt->min_guests ?: 1;
  $feedback_bar = property_feedback_bar($apt, '', array('url' => $url));
  if($facilities   = property_facilities($siteObj, $pageObj, $apt, $options)) $fac_str = "<div id=\"facilities_block\">$facilities</div>";
    
  $hreview_score = round($apt->review_score / 2, 1); // review out of 5

  // rate: todo, finish
  // $rate         = $apt->rate_hour; // todo: allow day
    
  if($show_links) {
    $links = array();
    //if($show_map) {      
    //  $loc = obj2loc($apt->type, $apt); // this will add noise if needed
    //  $apt->map_link = property_map($loc, $map_options);
    //}
    $links[]   = $edit_link; //show on top
    list($request_text, $request_link) = print_request_link($apt, $searchObj); 
    $links[]=  html_link($request_link, $request_text, array('class' => 'arrow_link'));
    if(!$details && !$options['hide_more_info']) $links[] = html_link($url, phrase('more_info', CAPITALIZE), array('class' => 'arrow_link'));
    $links_str     = $apt->links = count($links) ? implode(html_break(), array_filter($links)) : '';
    
    //dump($links);
  }

  $buttons = array();
  if(!$apt->links) { // use buttons
    // book/request buttons
    $button_class = $details ? 'big_red_button' : 'small_red_button';
    if($apt->book_direct == DB_REQUIRED) { 
      $book_uri = replace_uri_var($url, 'sn', $apt->id);
      $book_js = 'onclick = "window.location.href = \''.$book_uri.'\';return false;"';        
      $buttons[] = form_input('request', 'button', phrase('book_now', UPPERCASE), array('js' => $book_js, 'class' => $button_class));
    } else {
      if($searchObj->in) {
        list($request_text, $request_link) = print_request_link($apt, $searchObj);
        $request_js = 'onclick = "window.location.href = \''.$request_link.'\';return false;"';
      } else {
        $request_text = phrase('request_offer', UPPERCASE);
        $request_js = 'onclick = "request_form(\''.$site_id.'\', \''.$type.'\', \''.$id.'\', {\'title\': \''.phrase('request', CAPITALIZE).' '.$apt->name.'\'}); return false;"';
      }        
      $buttons[] = form_input('request', 'button', $request_text, array('js' => $request_js, 'class' => $button_class));
    }
    if($details && $siteObj->work_phone) $buttons[] = form_input('call', 'button', phrase('or_call', UPPERCASE).' '.$siteObj->work_phone, array('class' => 'big_red_button'));
  }

  $buttons_str = count($buttons) ? implode("\n", $buttons) : $apt->links;
  $unit = $type == 'boat' ? 'hour' : 'day';

  $review_count = feedback_count($apt);
  $review_score = feedback_score($apt->id);
  //echo("Template=$template");


  if($details) {
    switch($template) {
    case 'air': // air details
    case 'bookme':
 
        if($gallery_count) {
          $gallery_options = property_gallery_options($siteObj, $apt);
          
          //dump($gallery_options);
          $gallery_options['viewer'] = 'flexslider';
          $gallery_options['format'] = 'html';
          $gallery_options['container_class'] = 'flexslider';
          //dump($gallery_options);
          $flexslider = site_gallery($siteObj, $gallery_options);
        }
        $_SESSION['details_columns'] = 1;
        $bootstrap = $_SESSION['bootstrap'];

        $detailsAr = array();
        if($apt->hosting_room_type) $detailsAr[] = html_span(' ', 'font-size-large air-blue icon-home').phrase(get_object('hosting_room_type', $apt->hosting_room_type, 'name'), CAPITALIZE);
        if($apt->max_guests) $detailsAr[] = html_span(' ', 'air-blue icon-users').phrase('sleeps', CAPITALIZE).' '.$apt->max_guests;        
        if($apt->num_bedrooms) $detailsAr[] = html_span(' ', 'air-blue icon-lodging').$apt->num_bedrooms.' '.($apt->num_bedrooms == 1 ? phrase('bedroom', CAPITALIZE) : phrase('bedrooms', CAPITALIZE));
        if($apt->num_bathrooms) $detailsAr[] = html_span(' ', 'air-blue icon-toilet').$apt->num_bathrooms.' '.($apt->num_bathrooms == 1 ? phrase('bathroom', CAPITALIZE) : phrase('bathrooms', CAPITALIZE));
        //$detailsAr[] = html_span(' ', 'green icon-lodging').$apt->rating_men;                        
        $detailsAr[] = html_span(' ', 'air-blue icon-credit-card').print_price($apt, 'rate_day').($apt->rate_weekend > 0 && $apt->rate_weekend != $apt->rate_day ? '-'.html_span(print_price($apt, 'rate_weekend'), 'dk-red') : '')."/".phrase('day');
        $apt_details = implode(html_break(), $detailsAr);

        //dump($searchObj);
        
        $req_link = print_request_link($apt, $searchObj, array('link_only' => true));
        $edit_link = property_edit_link($apt->type, $apt->id, true);
                             
        //$rl = site_pagetype_link($siteObj, PAGE_REQUEST, $_SESSION['language'], true);
        //$debug = html_break()."req=$rl".html_break().$req_link;

        $buttons = array();
        $buttons[] = html_link($req_link, html_span(phrase('book_now', CAPITALIZE), 'zocial fontello icon-key air-blue-back air-button'));        
        if($edit_link) $buttons[] = html_link($edit_link, html_span(phrase('edit', CAPITALIZE), 'zocial fontello icon-key air-blue-back air-button'));
        $apt_details .= html_div(implode(PHP_EOL, $buttons), 'air-buttons');

        if($bootstrap) {

          // http://www.helloerik.com/the-subtle-magic-behind-why-the-bootstrap-3-grid-works
          $back_link = '';                                      
          if(isset($_GET['sr'])) {
            $page_link = site_page_link($siteObj, $_SESSION['pageObj']);
            $qs = "?".get_query_string(); // remove uri var needs ? or & in front
            $back_qs = remove_uri_var($qs, 'sr');            
            
            $search_link = html_link($page_link.$back_qs."&search=1", html_span(phrase('search', CAPITALIZE), 'zocial fontello icon-search air-blue-back'));
            if($apt_list = $_GET['apt_list']) {
              $aptIdArray = explode(',', $apt_list);
              $this_index = array_search($apt->id, $aptIdArray);
              $prev_index = $this_index > 0 ? $this_index - 1 : -1;
              $next_index = $this_index < count($aptIdArray) - 1 ? $this_index + 1 : -1;
              $prev_class = $prev_index > -1 ? "previous" : "previous disabled";
              $next_class = $next_index > -1 ? "next" : "next disabled";
              $prev_class = $prev_index > -1 ? "" : "disabled";
              $next_class = $next_index > -1 ? "" : "disabled";
              $prev_link = html_link($prev_index > -1 ? property_link($siteObj, $aptIdArray[$prev_index], array('qs' => $qs)) : '#', "&larr; ".phrase('previous', CAPITALIZE));
              $next_link = html_link($next_index > -1 ? property_link($siteObj, $aptIdArray[$next_index], array('qs' => $qs)) : '#', phrase('next', CAPITALIZE)." &rarr;");
            }
          }
          //dump($_SERVER);
          // tabs
          
          $tabs['photos'] = $flexslider.$description; //html_div($description, 'property-description');
          $tabs['map']    = property_map($apt, array('big' => true, 'type' => 'satellite'));
          $tabs['reviews'] = feedback_tab($apt);
          //$tabs['details'] = $details;
          $tabs = tabs($tabs, array('id' => 'property_tabs'));
          //$tabs = implode(html_break(), $tabs);

          
          $output = "
        <div class=\"container air-details\">
          <ul class=\"pager\">
            <li class=\"\">$search_link</li>
            <li class=\"$prev_class\">$prev_link</li>
            <li class=\"$next_class\">$next_link</li>
          </ul>
          <h2>$name</h2>
          <div class=\"row\">
            <div class=\"col-xs-12 col-sm-4 col-sm-push-8\">
              <div class=\"row\">                                          
                <div class=\"col-12 col-lg-6 air-property-details\">$apt_details</div>
                <div class=\"col-12 col-lg-6\">$golf_str</div>
              </div>
            </div>
            <div class=\"air-box col-xs-12 col-sm-8 col-sm-pull-4\">
            $tabs
            </div>
          </div>     
        </div>
          ";
          if(!$_SESSION['live']) $output .= $debug;
        } else {
          $output = "
          <div class='content-row'><h2>$name</h2></div>
          <div class='col half'>$flexslider</div>
          <div class='col half pad_left'>$description</div>
          <div class='content-row'>bottom</div>
          ";
        }
        break;
        
      case 'baris':
        $rating_w = round(64/10 * $review_score->score) + 1;         
        $feedback = feedback_tab($apt, $options['feedback']);
 
        if($gallery_count) {
          $gallery_options = property_gallery_options($siteObj, $apt);
          $gallery_options['carousel'] = false;
          $gallery_options['w'] = 610;
          $gallery_options['can_edit'] = $can_edit;
          $gallery_options['show_info'] = false;
          $gallery_options['show_snippets'] = true;
          $gallery = site_gallery($siteObj, $gallery_options);
        }

        $title =  phrase("request", CAPITALIZE)." $apt->name";

        $label_pp_u    = phrase('price',CAPITALIZE).' '.phrase('per_'.$unit);
        $label_pppp_u  = phrase('price',CAPITALIZE).' '.phrase('per_person_per_'.$unit);
        $label_persons = phrase('persons', CAPITALIZE);
        //
        //if($details) {
        //  $rate_str     = $rate > 0 ? "
        //                    <span itemprop=\"offerDetails\" itemscope itemtype=\"http://data-vocabulary.org/Offer\">
        //                     <meta itemprop=\"currency\" content=\"".$apt->currency."\" />$cur 
        //                     <span itemprop=\"price\">$rate</span>
        //                    </span>\n" : phrase('on_request', CAPITALIZE);
        //} else {
        //  $rate_str     = $rate > 0 ? "$cur $rate" : phrase('on_request', CAPITALIZE);
        //}
        //
        //
        //$pp_u    = $rate_from.$rate_str;
        //$pppp_u  = $rate > 0 ? $rate_from.$rate_pp : $rate_str; 
        //$persons = "$min_guests - $apt->max_guests";
                   
        $output = "
        <div itemscope itemtype=\"http://data-vocabulary.org/Product\" class=\"property-details-block\">
          <h2 itemprop=\"name\">$apt->name</h2>
          <span itemscope itemtype=\"http://data-vocabulary.org/Review-aggregate\">
            <span title=\"Rating $apt->name = $hreview_score / 5\" style=\"width:".$rating_w."px !important\" class=\"rating\">
              <meta itemprop=\"rating\" content=\"$hreview_score\" />
            </span>             
            <a href=\"#tab_reviews\" title=\"Reviews $apt->name\" class=\"reviewtitlebig\">(<span itemprop=\"count\">$review_count</span> ".phrase('reviews').")</a>
            $edit_link
          </span>     
          <div class=\"imageframe\">$gallery</div>
          <div itemprop=\"description\" class=\"property-details-description\">$description</div>

          <div id=\"details-block\">
            <div class=\"details-labels left_column\">
              $label_pp_u   <br />
              $label_pppp_u <br />
              $label_persons<br />
            </div>
            <div class=\"details-values middle_column \">
              $pp_u        <br />
              $pppp_u      <br />
              $persons     <br />       
            </div>
            <div class=\"details-buttons right_column\">
              $buttons_str
            </div>
            <div class=\"footer_row\"></div>
          </div>
          $fac_str
        </div>
        <div id=\"reviewsblock\">                                                                                                                                                                      
        <a name=\"tab_reviews\"><h2>".phrase('reviews', CAPITALIZE)."</h2></a><br>        
        $feedback                                          
        </div>
        ";
        break;  

      default:
                 
      // tabbed view      
      $tabs = array();
      if($gallery_count) {
        //$gallery_options = property_gallery_options($siteObj, $apt);
        //dump($gallery_options);
        $gallery_options['can_edit'] = $can_edit;
        //$gallery_options['w'] = 200;
        $search['parent_type'] = $apt->type;
        $search['parent_id'] = $apt->id;
        $search['active'] = 1 ;
        $gallery_options['viewer'] = 'galleria';
        $gallery_options['format'] = 'json';
        $gallery_options['obj_type'] = 'media';
        $gallery_options['search'] = $search;
        $gallery_options['width'] = 600;
        $gallery_options['show_link'] = false;
        
        $tabs['photos'] = site_gallery($siteObj, $gallery_options);
      }
      if($description) $tabs['description']  = $description;
      $tabs['details'] = details($apt);
        
      //if($apt->location_accuracy) {
      //  $map_options['big'] = true;
      //  $map_options['zoom'] = 13;
      //  if($apt->type == 'shortstay') $map_options['radius'] = 200; // show circle with 200 meter radius     
      //  $tabs['location'] = property_map($apt, $map_options);
      //}
      if($device_id = $apt->gps_tracking_id) $tabs['tracking'] = googlemap_tracker($device_id);
      if($apt->review_nr >= 4 && $apt->review_score > 0) $tabs['reviews'] = feedback_tab($apt, $options['feedback']);      
      // if($apt->type=='realestate') $tabs['financing'] = mortgage_tab($apt);
      $tabs = tabs($tabs, array('init' => false)); // print the tabs
                   
      $output .= html_div("
       $nav
       <div id=\"detailItem\">
          <div class='detailPic'>$pic</div>
          <div class=\"detailName\">
            <div class=\"houseTitleDetail\">$name</div>
            <div class=\"blue_text\">$descriptor</div>
            <div>$feedback_bar</div>
            <div class=\"blue_text\">$price_pp</div>      
          </div>
          <p class=\"links\">$links_str</p>
        </div><!-- detailItem -->
        $tabs
        $map
        $nav    
      ", '');    
      break;
    }
  } else { // list view
    
    if($gallery_count > 1) {
      $js = "data-search='{\"obj_type\":\"$type\",\"id\":\"$id\"}'"; 
      $gallery_link = html_link("#", phrase("more_pictures", CAPITALIZE), array('class' => 'vp-loader-fancybox', 'js' => $js));
    }
    switch($template) {
    case 'air': // air list
      $image_size = 'medium';
      $images = property_images($apt->id, array('size' => $image_size));
      $image_urls = quoted_list($images, '"');

      $pic_src = $images[0];
      //$pic_src = get_first_pic($apt, array('size' => 'medium', 'return' => 'source'));
      $review_count = 0;
      $room_type = $apt->hosting_room_type ? get_object('hosting_room_type', $apt->hosting_room_type, 'name') : "None: $apt->hosting_room_type";
      $city_id = $apt->city_id;
      $city_name = $apt->address_city;
      if($country_code = $apt->address_country) $country_name = get_object('countries', $country_code, 'name');
      if(!$country_name && $city_id) {
        $cityObj = get_object('geo_city', $city_id);
        $country_name = $cityObj->country_code;
      }
      if(!$city_name && $cityObj) $city_name = $cityObj->name; 
      $area = "$city_name, $country_name";
      $debug = dump($images, true);


      $host_id = $apt->host_id;
      $user_id = $apt->user_id;
      if($profile_id = $apt->user_profile_id) {
        $profileObj = get_object('user_profile', $profile_id);
        $profile_link = user_profile_link($siteObj, $profileObj);
        $profile_pic = print_media($profileObj->photo, array('size' => 'square', 'full' => true, 'source_only' => true));
      } else {
        $profile_link = '#';
        $profile_pic = '';
      }
      $pet_profile = '';

      if($user_id && $siteObj->id == 249) { // show pet
        if($profile_id = $apt->user_profile_id) {
          if($petProfileObj = find_object('pet_profile', array('user_id' => $user_id))) {
            $pet_profile_link = pet_profile_link($siteObj, $petProfileObj);
            $pet_profile_pic = print_media($petProfileObj->photo, array('size' => 'square', 'full' => true, 'source_only' => true));
            $pet_profile = "
              <a class=\"listing-host-img media-link media-photo\" href=\"$pet_profile_link\">
                <img src=\"$pet_profile_pic\">
              </a>
            ";
          }
        }
      }
      if(!$profile_pic) $profile_pic = '/images/user_profile_pic_225.png';
      //$output = $debug;
      $lat = $apt->location_lat;
      $long = $apt->location_long;
      $host_id = $apt->host_id;
      $facs = $apt->facilities;
      $name = htmlspecialchars($apt->name);
      if($edit_link) $edit_link = html_div($edit_link, 'action-links');
      $output .= "       
     <div id=\"$type-$id\" class=\"air-item listing hilite_target\" data-facs=\"$facs\" data-lat=\"$lat\" data-lng=\"$long\" data-name=\"$name\" data-url=\"$url\" data-user=\"$host_id\" data-id=\"$id\">
        <div class=\"listing-img media-photo\"'>
          $edit_link
          <div class=\"listing-img-container media-photo image-rotation\" data-current=\"0\" data-images='[$image_urls]'>
            <img src=\"$pic_src\">
          </div>
          <div class=\"image-rotation-control block-link image-rotation-prev\">
            <i class=\"icon icon-chevron-left listing-slideshow-chevron\">&lt;</i>
          </div>
          <div class=\"image-rotation-control block-link image-rotation-next\">
            <i class=\"icon icon-chevron-right listing-slideshow-chevron\">&gt;</i>
          </div>
        </div>
        <div class=\"listing-footer clearfix\">
          <a class=\"listing-host-img media-link media-photo\" href=\"$profile_link\">
            <img src=\"$profile_pic\">
          </a>
          <a class=\"listing-quick-info property-detail-link\" href=\"$url\" title=\"$room_type | $area\">
            <span class=\"listing-room-type\">
              $name<br>
              $room_type<br>
              $area
            </span>
          </a>
          $pet_profile
          <a class=\"listing-price property-detail-link\" href=\"$url\">
            <span class=\"price\">
              <span class=\"currency\">$cur</span>
              <span class=\"price-amount\">$rate</span>
            </span>
            <span class=\"price-sub\">
                Per night
            </span>
          </a>
        </div>
      </div>\n";
        //return html_div($item, 'property-content air');
        break;
        
      case 'bookme':

        $pic    = get_first_pic($apt, array('size' => 'small'));
        $photo  = html_link($url, $pic);
 
        $output = "
        <div class='bookme-item'>
          <div class='content-row'>
            <h2>$name - ".property_descriptor($apt->type, $apt)."</h2>
          </div>
          <div class='col one-quarter'>
            <div class='imgContainer110'>$photo</div>
            $gallery_link
          </div>
          <div class='col three-quarters'>
            <b>$price_pp</b><br>
            $apt->short_description
            <div>$apt->links</div>
          </div>
          <div class='content-row'></div>
        </div>
        ";
        break;

      case 'owner':
        $edit_link = property_edit_link($apt->type, $apt->id, true);
        $view_link = property_link($siteObj, $apt);
        $buttons_str = '';
        $buttons_str .= html_span(html_link($edit_link, phrase('edit', CAPITALIZE), array('class' => 'icon edit')), 'action_button');
        $buttons_str .= html_span(html_link($view_link, phrase('view', CAPITALIZE), array('class' => 'icon view')), 'action_button');
        $buttons_str .= html_span(html_link($cal_link, phrase('calendar', CAPITALIZE), array('class' => 'icon calendar')), 'action_button');
                
        $data['obj_type'] = $type;
        $data['id'] = $id;
        $data_json = json_encode($data);
        
        $steps_remaining = property_remaining_steps($siteObj, $apt->type, $apt->id);
        //dump($steps_remaining);
        if($count = count($steps_remaining)) {
          $steps_remaining_title = html_break().html_h3(phrase('you_are_almost_there', CAPITALIZE).'...');
          $steps_remaining_title .= $count." ".($count == 1 ? phrase('step_remaining') : phrase('steps_remaining')).":".html_break();
          $steps_remaining_string = $steps_remaining_title.html_ul($steps_remaining);
        }
          
        //$steps_remaining[] = 'foo';
        //echo("photo=$photo");
        $output = "
        <div class=\"pad owner_listing\">
          <div class='pad float_left'>$photo</div>
          <div class='margin_top pad float_left'>
            <h2 class='houseTitle'><a href=\"$url\">$name</a></h2>
            <div class='actions'>$buttons_str</div>
          </div>
          <div class='pad float_right'>                                                                                                                  
          ".on_off_slider('ajax-switch-'.$id, $apt->active, array('handler' => 'edit', 'target_field' => 'active', 'data' => $data_json, 'disabled' => count($steps_remaining) ? true : false)).$steps_remaining_string."
          </div>
          <div class='float_clear'></div>
        </div>        
        ";        
        break;
      case 'baris':
        //$apt->review_score = 9;
        $rating_w = round(64/10 * $review_score->score) + 1; 
        $icon = $type == 'boat' ? 'sprite-boat-icon' : 'sprite-bed-icon';
        $output = "
        <div class=\"property-content baris\">
            $photo
            <div class=\"property-details\">
              <a class=\"property-title\" href=\"$url\">$apt->name</a> 
              <span title=\"Rating $apt->name = $hreview_score / 5\" style=\"width:".$rating_w."px !important\" class=\"rating\"></span>
              <a href=\"$url#tab_reviews\" title=\"Reviews $apt->name\" class=\"reviewtitle\">($review_count ".phrase('reviews').")</a> $edit_link
                <div class=\"property-description\">$description</div>
                <div class=\"property-icons\">
                  <div class=\"left_column\">                                           
                    <div class=\"property-icon sprites sprite-2persons\"><span class=\"icon-margin\">$price</span></div>
                    <div class=\"property-icon sprites sprite-1person\"><span class=\"icon-margin\">$price_pp_only</span></div>
                  </div>
                  <div class=\"right_column\">
                    <div class=\"property-icon sprites $icon\"><span class=\"icon-margin\">$min_guests - $apt->max_guests ".html_span(phrase('persons'), 'detail-label')."</span></div>
                    <div class=\"property-icon book-now\">$buttons_str</div>
                  </div>
                  <div class=\"footer_row\"></div>
                </div>
            </div>
        </div>
        ";
        
        break;
      case 'narrow':
        $output = "
        <div class=\"propertyContent\">
          <div class=\"propertyLeft\">
            $photo
          </div>
          <div class=\"propertyCenter\">                                                                                  
            <span class=\"houseTitle blue_text\">".property_title($apt)."</span> - <span class=\"blue_text\">".property_descriptor($apt->type, $apt)." - ".property_price($apt, $searchObj)."</span><br>
            <span class=\"map\">$apt->map_link</span>
            $feedback_bar
            <div class = \"propertyDescription clear_float\">$description</div>
            <p class=\"links\">$apt->links</p>
          </div>
        </div>\n";
        break;
        
      default:
        $details    = details($apt, true);
        //dump($apt->links);
        $output = "
        <div class=\"propertyContent\">
          <div class=\"propertyLeft\">
            $photo
            $gallery_link
          </div>
          <div class=\"propertyCenter\">
            <div class=\"houseTitle blue_text\">$name</div>
            <span class=\"map\">$apt->map_link</span>
            $feedback_bar
            <div class = \"propertyDescription clear_float\">$description</div>
          </div>
          <div class=\"propertyRight\">
            <span class=\"blue_text\">".property_descriptor($apt->type, $apt)."</span>
            <span class=\"blue_text\">".$price_pp."</span><br>
            $details
            <p class=\"links abs_bottom_right\">$apt->links</p>
          </div>
        </div>\n";
        break;      
    }
  }
  return $output;
}

// detail view
//function property-details($siteObj, $pageObj, $apt, $searchObj=NULL, $nav='', $can_edit=false) {
function property_details($siteObj, $pageObj, $apt, $options=NULL) {
  $options['details'] = true;
  $output = property_template($siteObj, $pageObj, $apt, $options);
  //dump($options);
  return $options['nodiv'] ? $output : html_div($output, '', 'id="property-list"');
  //return $output;  
}

  
// list view                      
function property_print($siteObj, $pageObj, $apt, $options=NULL) {
  $options['details'] = false;
  $apt->discount = $apt->discount ?: percent2discount($apt->rate_change);
  if($apt->parent_id) $apt = get_object($apt->type, $apt->parent_id);
  if(!$apt->id) die("property_print: ".caller_stack().dump($apt, true));
  $output = property_template($siteObj, $pageObj, $apt, $options);
  return $output;
}

// returns prev/next links used in property_print_details
function property_details_nav($apt, $apt_ids, $glue=' - ') {
  $id = $apt->id;
  $type = $apt->type;
  $size = sizeof($apt_ids);
  $si = array_search($id, $apt_ids); // index of this apt in the result
  $apts_url = get_page_link($apt->type, true, $_SESSION['language'], '', city_dir($apt->city_id));
  if($qs  = get_query_string()) $qs_s = "?$qs";

  if($si) {
    $apt = get_object($type, $apt_ids[$si-1]);
    $nav[]= html_link($apts_url.$apt->shortname."/".$qs_s, phrase('previous', CAPITALIZE));
  }

  if($si < $size-1) {
    $apt = get_object($type, $apt_ids[$si+1]);
    $nav[]= html_link($apts_url.$apt->shortname."/".$qs_s, phrase('next', CAPITALIZE));
  }

  if($qs) $nav[]= html_link($apts_url.$qs_s, phrase('back', CAPITALIZE));
  return implode($glue, $nav);
}

// tab content helpers


// small map or big map
//function property_map($apt, $big=false, $add_noise=true) {
function property_map($apt, $map_options=NULL) {
  if(!$apt->location_accuracy) return '';

  global $__header;
  $big = $map_options['big'];
 
  //$loc = obj2loc($apt->type, $apt, $map_options['ignore_noise']);
  $loc = obj2loc($apt->type, $apt, true);
  if(!$loc) return '';
  if($bounds = $map_options['bounds'] ?: $apt->location_bounds) $map_options['bounds'] = $bounds;
  if($bounds_json = $bounds) $tail = $bounds_json;  

  $map_options['markers'] = $loc;
  $tail = '';
  if($big) { // show big map for property details
    $id = 'big_map';
    if($cityObj = get_object('geo_city', $apt->city_id)) {
      if(!isset($map_options['center'])) $map_options['center'] = "$cityObj->location_lat,$cityObj->location_long";
      //if(!isset($map_options['accuracy'])) $map_options['accuracy'] = $cityObj->location_accuracy;
    }
    $map_options['zoom'] = 14;
    $map_options['height'] = '360';
    $map_options['id'] = $id;    
    $map_options['site_id'] = $_SESSION['site_id'];    
    //$map_options['init'] = false;
    $map = gmap3($map_options);
  } else { // just show a link for popup map
    $id = 'map-'.$apt->id;
    $map = html_link("#property-$apt->id", phrase('show_map', CAPITALIZE), array('id' => $id, 'js' => "onclick='return false;'", 'class' => 'map_link'));
  }        
  return $map.$tail;
}

function has_map($apts) {
  if(!count($apts)) return false;
  foreach($apts as $apt) if($apt->location_accuracy && $apt->location_lat && $apt->location_long) return true;
  return false;
}

function mortgage_tab($apt) {
  $price_fld = property_price_field($apt->type);
  $topic = geoname($apt->city_id)." ".phrase($apt->type, LOWERCASE)." ".phrase('mortgage', LOWERCASE)." ".phrase('advice', LOWERCASE);
  $apt_str = "$apt->name - $apt->currency ".number_format($apt->$price_fld);
  $str = html_header($topic, 2);
  $str .= html_header($apt_str, 2);
  $str .= phrase('text_mortgage');

  global $ao_errors;
  if(sizeof($ao_errors)) $str .= print_errors($ao_errors, false);

  return relocation_advice_form('mortgage', $apt->city_id, $apt->type, $apt->id);
}

// box on left which shows search + advanced filters
function searchbox($type, $city_id, $apt_ids, $messages='') {
  $count = count($apt_ids);
  if($count) $output .=  property_search_hits($type, $count);
  if(count($apt_ids) && !$_SESSION['narrow_design']) {
    if($filters = searchfilters($type, $city_id, $apt_ids)) $output .= html_fieldset($filters, 'inset', array('legend' => phrase('refine_your_search', CAPITALIZE)));
  }
  return $output;
}

function search_result_title($result) {
  $currency = $result->currency ?: 'EUR';
  $cur = get_currency_symbol($currency, 1);
  
  $min_price = $result->min_price ?: 0;
  $max_price = $result->max_price ?: 0;
  $max_guests = $result->max_guests ?: 0;
  $count = $result->count ?: 0;
  
  $property_text1 = html_span(phrase('property'), 'property_text_1');
  $property_text2 = html_span(phrase('properties'), 'property_text_2');
  $title = html_div(html_span($count, 'count').' '.$property_text1.$property_text2.' '.phrase('found'), 'margin-right inline');

  $title .= html_div($cur.html_span($min_price, 'min_price').'-'.html_span($max_price, 'max_price')." (".phrase('Max.').' '.html_span($max_guests, 'max_guests').' '.phrase('guests').")", 'search-result-details');
  
  $output = html_div($title, 'air-title', array('id' => 'search_result_title'));
  return $output;
  
}

function property_filters_air($siteObj, $search, $options = NULL) {
  $template = $options['template']  ?: $siteObj->template; 

  
  $client_id = $siteObj->client_id;
  $property_type = $options['property_type'] ?: site_property_type($siteObj); 
  $ng = $search['max_guests'][1] ?: 2;
  $price_field = property_price_field($property_type);
  $qs_ng = "SELECT round(min($price_field + ($ng - rate_num_guests) * rate_extraperson)) as min_price, round(max($price_field + ($ng - rate_num_guests) * rate_extraperson)) as max_price, min(max_guests) as min_guests, max(max_guests) as max_guests FROM property WHERE type='$property_type' AND client_id=$client_id";
  $qs_1g = "SELECT round(min($price_field)) as min_price, round(max($price_field)) as max_price, min(max_guests) as min_guests, max(max_guests) as max_guests, min(num_rooms) as min_rooms, max(num_rooms) as max_rooms, min(living_size) as min_size, max(living_size) as max_size FROM property WHERE type='$property_type' AND client_id=$client_id";
  $qs = $ng > 1 ? $qs_ng : $qs_1g;
  $min_max = db_getRow($qs);
  
  // dates
  $today = today();
  $calparams['maxDate'] = "'+1y'";
  $calparams['init_date'] = $today;
  $calparams['onSelect'] = "in_out_change_handler"; //"function(dateText) {in_out_change_handler();}";
  $calparams['class'] = "date_autoloader";
  
  $fields['checkin']  = datepicker('in', $calparams);
  $fields['checkout'] = datepicker('out', $calparams);
  $fieldset_class = $options['fieldset_class'] ?: 'fieldset_top';

  $output = search_result_title($options['result']);
  $output .= html_fieldset($fields, $fieldset_class, array('legend' => $options['fieldset_title'], 'translate' => true));
  
  // num guests
  $output .= searchfilter_slider("ng", 'searchfilter', array('template' => $template, 'all_link' => $all_link, 'title' => phrase('guests', CAPITALIZE), 'all_link_text' => phrase('all_numbers', CAPITALIZE), 'min' => 1, 'max' => $min_max->max_guests, 'val1' => $min_max->min_guests, 'val2' => $min_max->max_guests));  

  // price
  $price_slider_title = phrase('price', CAPITALIZE).' '.phrase('per_day');
  $output .= searchfilter_slider("price", 'searchfilter', array('template' => $template, 'all_link' => $all_link, 'title' => $price_slider_title, 'all_link_text' => phrase('all_prices', CAPITALIZE), 'prefix' => '&euro;', 'min' => $min_max->min_price, 'max' => $min_max->max_price, 'val1' => $min_max->min_price, 'val2' => $min_max->max_price));

  // facilities
  $facs = property_facilities_filters($property_type);
  $output .= show_hide_div($facs, html_span(phrase('more_filters', CAPITALIZE), 'zocial fontello icon-down-open air-blue-back'), array('class' => 'margin-top', 'show_icon' => false));
  return html_div($output, 'searchbox', "id='searchbox'");
}

// new searchbox with search / filters
function property_searchbox($siteObj, $pageObj, $options=NULL) {  
  global $db;
  $template   = $options['template']  ?: $siteObj->template; 

  $all_apt_ids   = array();

  $property_type = $options['property_type'] ?: site_property_type($siteObj); 
  if(!$property_type) mydie("property_searchbox: missing property_type");
  $unit = hourly_rate($property_type) ? 'hour' : 'day';
  if($template == 'air' && $unit == 'day') return property_filters_air($siteObj, $search, $optiosn);

  $city_id       = $options['city_id'] ?: $search['city_id'];
  $client_id = $siteObj->client_id;
  
  $apt_ids= $options['apt_ids'] ?: array(); // the search result
  $search = $options['search'];
  $debug = $options['debug'];

  //if($template == 'air') return property_filters_air($siteObj, $search, $options);
  
  //dump($options);
  //dump($search);
  $ng = $search['max_guests'][1];
  //$debug_str = "Ng=$ng";
  if($search) { // do the basis search here (w/o dynamic filters, to get all apts w/o filters)
    unset($search['rate_day']);
    unset($search['rate_hour']);
    unset($search['max_guests']);
    $aptObjs =  find_objects($property_type, $search);
    $all_apt_ids = array_find_key($aptObjs, 'id');
  }
  if(!$all_apt_ids) $all_apt_ids = $apt_ids;
  $all_id_list = implode(',', $all_apt_ids);
  
  //$debug = true;  
  if($debug) {
    $debug_str = "All: ".count($all_apt_ids).html_break();
    $debug_str .= "Search: ".count($apt_ids).html_break();
    $debug_str .= "$all_id_list<br>".dump($search,true);
    $debug_str .= dump($search, true);
    $debug_str .= "Price: Min = $min_max->min_price Max=$min_max->max_price<br>";
    $debug_str .= "Guest: Min = $min_max->min_guests Max=$min_max->max_guests<br>";
  }
  $count = count($all_apt_ids);


  if($count) {
    $price_field = property_price_field($property_type);

    // these look at min/max price + num guests in search result
    $qs_ng = "SELECT round(min($price_field + ($ng - rate_num_guests) * rate_extraperson)) as min_price, round(max($price_field + ($ng - rate_num_guests) * rate_extraperson)) as max_price, min(max_guests) as min_guests, max(max_guests) as max_guests FROM property WHERE id IN ($all_id_list)";
    $qs_1g = "SELECT round(min($price_field)) as min_price, round(max($price_field)) as max_price, min(max_guests) as min_guests, max(max_guests) as max_guests, min(num_rooms) as min_rooms, max(num_rooms) as max_rooms, min(living_size) as min_size, max(living_size) as max_size  FROM property WHERE id IN ($all_id_list)";

    $qs = $ng > 1 ? $qs_ng : $qs_1g;
    $min_max = db_getRow($qs);
  
    $all_link = $options[$property_type]['all_link'] ?: $_SERVER['REQUEST_URI'];
    if(in_array($property_type, array('boat', 'shortstay'))) {  
      $price_slider_title = phrase('price', CAPITALIZE).' '.phrase('per_'.$unit);
      $sliders .= searchfilter_slider("ng", 'searchfilter', array('template' => $template, 'all_link' => $all_link, 'title' => phrase('guests', CAPITALIZE), 'all_link_text' => phrase('all_numbers', CAPITALIZE), 'min' => 1, 'max' => $min_max->max_guests, 'val1' => $min_max->min_guests, 'val2' => $min_max->max_guests));  
    } elseif(in_array($property_type, array('longstay', 'realestate'))) {
      $price_slider_title = phrase('price', CAPITALIZE);
      if($property_type == 'longstay') $price_slider_title .= ' '.phrase('per_month');
      $sliders .= searchfilter_slider("nr", 'searchfilter', array('template' => $template, 'all_link' => $all_link, 'title' => phrase('rooms', CAPITALIZE), 'all_link_text' => phrase('all_numbers', CAPITALIZE), 'min' => 1, 'max' => $min_max->max_rooms, 'val1' => $min_max->min_rooms, 'val2' => $min_max->max_rooms));  
      $sliders .= searchfilter_slider("sz", 'searchfilter', array('template' => $template, 'all_link' => $all_link, 'title' => phrase('size', CAPITALIZE), 'all_link_text' => phrase('all_sizes', CAPITALIZE), 'suffix' => 'm&#178;', 'min' => 1, 'max' => $min_max->max_size, 'val1' => $min_max->min_size, 'val2' => $min_max->max_size));  
    }
  
    // show price slider for all types
    $price_slider = searchfilter_slider("price", 'searchfilter', array('template' => $template, 'all_link' => $all_link, 'title' => $price_slider_title, 'all_link_text' => phrase('all_prices', CAPITALIZE), 'prefix' => '&euro;', 'min' => $min_max->min_price, 'max' => $min_max->max_price, 'val1' => $min_max->min_price, 'val2' => $min_max->max_price));
  
    $output = $price_slider.$sliders;

    if($filters = searchfilters($property_type, $city_id, $apt_ids ?: $all_apt_ids, $options)) $output .= html_fieldset($filters); //, phrase('refine_your_search', CAPITALIZE), 'inset');
  } else {
    //$output .= html_fieldset(searchfilters($property_type, $city_id, $apt_ids ?: $all_apt_ids, $options)); //, phrase('refine_your_search', CAPITALIZE), 'inset');
  }
  
  return $output ? html_div($debug_str.$output, 'searchbox', "id='searchbox'") : '';
}


// returns a slider for searchfilters with from to values
function searchfilter_slider($name, $class, $options=NULL) {    
  global $__header;

  $id      = $options['id'] ?: $name;
  $min     = $options['min'] ?: 0;
  $max     = $options['max'] ?: 0;
  $title   = $options['title'] ?: $name; // link for removing all in on go

  $slider_id    = $id.'_slider';
  $slider_from  = $id.'_from';
  $slider_to    = $id.'_to';       
  $slider_title = $id.'_slider_title';    
  $slider_span = $id.'_slider_span';    
  $template   = $options['template']  ?: $siteObj->template; 

  if($val = $_GET[$id]) {
    list($val1, $val2) = explode('-', $val);
  } else {
    $val1    = $options['val1'] ?: ($_REQUEST[$id.'_from'] ?: 0); 
    $val2    = $options['val2'] ?: ($_REQUEST[$id.'_to'] ?: 0);
  }
  if(!$val1) $val1 = $min;
  if(!$val2) $val2 = $max;

  if($request_uri = $options['all_link']) {
    //if($qs = get_query_string()) $request_uri .= "?$qs";
  } else {
    $request_uri = $_SERVER['REQUEST_URI'];
  }
  $uri = replace_uri_var($request_uri, 'search', 1);
  $uri = replace_uri_var($uri, 'nocache', 1);
  $url = remove_uri_var($uri, $id);
  $url = remove_uri_property($url);
  
  if($val1 != $min || $val2 != $max) { // we are filtering, so show all link
    $js = "onclick='loadProperties(\"".$url."\", \"".$template."\");return false;'";
    $url_txt = $options['all_link_text'] ?: phrase('all', CAPITALIZE)." $name"; // text for link for removing filter
    $all_link = html_link('#', $url_txt, array('js' => $js, 'class' => 'delete_link'));
  }
  
  $filter_url .= $template == 'air' ? "$id=" : $url."&$id="; // for air, we just want the variable, not the full URL

  
  $range_text = $options['prefix'].$val1.$options['suffix'].' - ' .$options['prefix'].$val2.$options['suffix'];
  
  $output .= html_span("$title: ", 'search_filter_title').$all_link;
  $output .= html_span($range_text, 'slider_range', "id='$slider_span'");
  $output .= form_input($slider_from, 'hidden', $val1);
  $output .= form_input($slider_to, 'hidden', $val2);
  $output .= html_div('', 'search_filter_slider', "id='$slider_id'");
  
  $output .= html_script_inline('    
     $( "#'.$slider_id.'" ).slider({
      range: true,
      min: '.$min.',
      max: '.$max.',
      values: [ '.$val1.', '.$val2.' ],
      slide: function( event, ui ) {
        $( "#'.$slider_span.'" ).html( "'.$options['prefix'].'" + ui.values[ 0 ] + "'.$options['suffix'].' - '.$options['prefix'].'" + ui.values[ 1 ] + "'.$options['suffix'].'");      
      },
      stop: function(event, ui) {
        if($("#'.$slider_from.'").val() == ui.values[0] && $("#'.$slider_to.'").val() == ui.values[1]) {
          //alert("no change");
        } else {
          var template = "'.$template.'";
          var url = "'.$filter_url.'" + ui.values[0] + "-" + ui.values[1];
          loadProperties(url, template);
        }
        $( "#'.$slider_from.'" ).val(ui.values[ 0 ]);
        $( "#'.$slider_to.'" ).val(ui.values[ 1 ] );

      }
    });
  ');
  return html_div($output, 'search_filters search_filter_slider_container');
}
  


function property_search_hits($type, $count) {
  $object_name = property_type_name($type, $count);
  return html_div("$count $object_name ".phrase('matched your request'), 'bold', "id='search_hits'");
}
  
function property_facilities($siteObj, $pageObj, $apt, $options=NULL) {
  $col_count = 3;
  if(!$apt->facilities) return ''; // not yet set
  $facObjs = property_facilitity_objects($apt->type);
  $count = count($facObjs);
  $apt_facs = explode(',', $apt->facilities);
  $row_count = ceil($count / $col_count);
  $col = array();
  $i = 0;
  foreach($facObjs as $facObj) {
    $class = 'inline-block sprites  '.(in_array($facObj->id, $apt_facs) ? 'sprite-yes' : 'sprite-no');
    $fac_name = html_div(html_div('&nbsp;', $class).' '.html_div(phrase($facObj->name, CAPITALIZE), 'inline-block facility_name'), 'facility');
    $col[] = $fac_name;
    if(++$i >= $row_count) {
      $i = 0;
      $cols[] = $col;
      $col = array();
    }
  }
  if($i < $row_count) $cols[] = $col;
  
  $cols[] = array();
  //echo("there are $count facilities, $col_count columns, $row_count rows");dump($cols);
  return css_columns($cols);    
}

function property_facilitity_objects($type, $show_searchable=true) {
  global $db;

  if(in_array($type, array('boat', 'hotel', 'office'))) {
    $search = "$type = 1";
  } elseif(in_array($type, array('realestate', 'longstay'))) {
    $search = 'non_serviced = 1';
  } elseif($type == 'shortstay') {
    $search = 'serviced = 1'; 
  } else {
    return array();
  }
  //echo("type=$type SELECT * FROM property_facilities WHERE $search ORDER BY name<br>");
                             
  if($show_searchable)  $search .= " AND searchable=1";
  $facObjs = db_getAll("SELECT * FROM property_facilities WHERE $search ORDER BY name");
  return $facObjs;
}

// advanced filters to reduce search result
function searchfilters($type, $city_id, $apt_ids, $options=NULL) {
  global $db;
  $uri = remove_uri_property($_SERVER['REQUEST_URI']);
  $template   = $options['template']  ?: $siteObj->template; 

  if($apt = $_SESSION['property']) {
    $shortname = $apt->shortname;   
    $uri = str_replace("$shortname/", '', $uri); // filter links always take you back to list view
  }
  $filters = array();
  if($type == 'shortstay' && $city_id) {
    if($borough_col  = db_getCol("SELECT id FROM msBorough WHERE parent_id=$city_id AND active=1 AND shortstay=1 ORDER BY name")) {
      $hoodObjs = db_getAll("SELECT id,name FROM msNeighborhood WHERE parent_id IN (".implode(',', $borough_col).") AND active=1 AND shortstay=1 ORDER BY parent_id, name");      
      $filters = add_neighborhood_filters($filters, $hoodObjs, $apt_ids);
    }
    // $facObjs = db_getAll("SELECT * FROM property_facilities WHERE searchable=1 AND serviced=1 ORDER BY name");
    // $filters = add_facility_filters($filters, $facObjs, $apt_ids);    
  } else if($type == 'hotel') {
  } else if($type == 'boat') {
    $typeObjs = db_getAll("SELECT * FROM boat_type WHERE active=1 ORDER BY name");
    $filters = add_boat_type_filters($filters, $typeObjs, $apt_ids);    
    //dump($filters);
    //$facObjs = db_getAll("SELECT * FROM property_facilities WHERE searchable=1 AND boat=1 ORDER BY name");
    //$filters = add_facility_filters($filters, $facObjs, $apt_ids);    
  } else if($type == 'office') {
  } else if($type == 'realestate' || $type == 'longstay') {
    //$facObjs = db_getAll("SELECT * FROM property_facilities WHERE searchable=1 AND serviced=1 ORDER BY name");
    // $filters = add_facility_filters($filters, $facObjs, $apt_ids);    
    $boroughObjs = db_getAll("SELECT * FROM msBorough WHERE parent_id=$city_id AND active=1 ORDER BY name");
    //dump($boroughObjs);
    $filters = add_borough_filters($filters, $boroughObjs, $apt_ids);
    //dump($filters);
  }

  $facObjs = property_facilitity_objects($type);
  $filters = $facObjs && $apt_ids ? add_facility_filters($filters, $facObjs, $apt_ids) : $filters;    
  
  // the variables            MULTITPLE   TERM
  $vars['bt']  = array(false, 'boat_type'); // todo: allow multiple ?
  $vars['fac'] = array(true, 'facilities');
  $vars['dis']  = array(false, 'district'); // todo: allow multiple ?
  $vars['ar']  = array(true, 'area');
  //dump($filters);
  // print the links
  foreach($filters as $var => $array) {
    $ajax = true;
    if($request_uri = $options[$var]['all_link']) {
      if($qs = get_query_string()) $request_uri .= "?$qs";
      //if($var == 'bt') $debug = "r=$request_uri<br>";
      $ajax = false; // go to another custom page when clearing this filter
    } else {
      $request_uri = $uri;
    }
    $uri = replace_uri_var($request_uri, 'search', 1);
    $uri = replace_uri_var($uri, 'nocache', 1);
  
    if($var == 'dis') $uri = remove_uri_var($uri, 'ar'); // non-generic hack; remove neigborhood variable for districts 
    $title = phrase($vars[$var][1], CAPITALIZE);
    $multiple = $vars[$var][0]; // does this variable allow us to select multiple ?
    $links = array();
    $selected = 0;

    //dump($array);
    // if(count($array) <= 1) continue; // no point showing filter with only one choice
    foreach($array as $id => $value) {
      if($hits = $value[1]) { // not selected
        $new_val = $multiple ? var_list_add($_GET[$var], $id) : $id;
        $url = replace_uri_var($uri, $var, $new_val);
        $add = phrase('add', CAPITALIZE)." ".$value[0];
        $js = 'onclick="loadProperties(\''.$url.'\',\''.$template.'\');return false;" title="'.$add.'"';
        $links[] = html_span($value[0].html_span($hits, 'float_right pointer'), 'search_filter toggle_off pointer', $js);
      } else {
        $selected++;
        if($multiple) { // selected - toggle on
          $new_val = var_list_remove($_GET[$var], $id);
          $url = $new_val ? replace_uri_var($uri, $var, $new_val) : remove_uri_var($uri, $var); // allow var=0
        } else {
          $url = remove_uri_var($uri, $var);
        }          
        $url = htmlspecialchars($url, ENT_COMPAT, 'UTF-8', false); // replace ampersands
        $remove = phrase('remove', CAPITALIZE)." ".$value[0];
        $js = 'onclick="loadProperties(\''.$url.'\',\''.$template.'\');return false;" title="'.$remove.'"';
        $links[] = html_span($value[0].html_span('x', 'float_right pointer'), 'search_filter toggle_on pointer', $js);
      }      
    }
    $output .= html_span("$title:", 'search_filter_title');
    if($selected > 0) {
      $url = htmlspecialchars(remove_uri_var($uri, $var), ENT_COMPAT, 'UTF-8', false);
      $js = $ajax ? 'onclick="loadProperties(\''.$url.'\',\''.$template.'\');return false;" title="'.$remove.'"' : '';
      //if($var == 'bt') $debug .= "js=$js<br>";
      $output .= " ".html_link($url, phrase('all', CAPITALIZE)." $title", array('full' => true, 'js' => $js, 'class' => 'delete_link')); // link for removing all in on go
    }
    $output .= html_break();
    $output .= implode('', $links).html_break(2);
  }
  return html_div($debug.$output, 'search_filters');
}

function property_facilities_filters($type) {
  $filters = array();
  $facObjs = property_facilitity_objects($type, false);
  $var = 'fac';
  foreach($facObjs as $fac) {
    $name = phrase($fac->name, CAPITALIZE);
    $id= $fac->id;
    $facArray[$id] = $name;
  }
  $checkedArray = $_GET['fac'] ? explode(',', $_GET['fac']) : array();
  $output = form_input('prop_facs', 'hidden', '', array('class' => 'cb_target'));
  foreach($facArray as $id => $name) {
    $cb_id = 'fac_'.$id;
    //$hits = 0;
    //$hits_str = " (".html_span($hits, 'pointer', array('id' => 'fac_hits_'.$id)).")";
    //$hits_str = '';
    $checked = in_array($id, $checkedArray);    
    $checkbox = form_input($cb_id, 'checkbox', $id, array('checked' => $checked, 'class' => 'cb_group prop_fac'));
    
    $links[] = html_div($checkbox.html_label($name, $cb_id, 'margin-left').$hits_str, 'fac-container pointer', array('data-id' => $id, 'id' => 'fac_cont_'.$id));
    // search_filter toggle_off 
  }
  $output .= implode("\n", $links);
  return html_div($output, 'checkbox_group clearfix', array('id' => 'facility_filters'));
}

function add_borough_filters($filters, $boroughObjs, $apt_ids) {
  global $db;
  if(!$apt_ids) return $filters;
  $nfilters = array();
  $apts_clause =  "AND id IN (".implode(',',$apt_ids).")";
  $var = 'dis';
  foreach($boroughObjs as $borough) {
    $id= $borough->id;
    $name = geoname($id, GEOLEVEL_BOROUGH);
    $query = $_GET[$var];
    if(strlen($query) && strpos($query, $id) !== FALSE) { // already on - show red and show boroughs
      $nfilters[$var][$id] = array($name, 0);
      $hoodObjs = db_getAll("SELECT id,name FROM msNeighborhood WHERE parent_id = $id AND active=1 ORDER BY name");
      $nfilters = add_neighborhood_filters($nfilters, $hoodObjs, $apt_ids);
    } elseif($hits = db_getOne("SELECT COUNT(*) FROM property WHERE borough_id = '$id' $apts_clause")) {
      $nfilters[$var][$id] = array($name, $hits);
      $total_hits += $hits;
    }
  }

  $num_unknown = db_getOne("SELECT COUNT(*) FROM property WHERE borough_id = '0' $apts_clause");
  if(($total_hits && $num_unknown && !$_GET[$var]) || $_GET[$var] == '0000') {// add for those not yet marked with borough
    $hits = $_GET[$var] == '0000' ? 0 : $num_unknown;
    $nfilters[$var]['0000'] = array(phrase('unknown', CAPITALIZE).' '.phrase('district', CAPITALIZE), $hits);
  }
  return array_merge($filters, $nfilters);
}

function add_neighborhood_filters($filters, $hoodObjs, $apt_ids) {
  global $db;
  if(!$apt_ids) return $filters;
  $nfilters = array();
  $apts_clause =  "AND id IN (".implode(',',$apt_ids).")";
  $var = 'ar';
  foreach($hoodObjs as $hood) {
    $id= $hood->id;
    $name = geoname($id, GEOLEVEL_NEIGHBORHOOD);
    $query = $_GET[$var];       
    if(strlen($query) && strpos($query, $id) !== FALSE) { // already on
      $nfilters[$var][$id] = array($name, 0);
    } elseif($hits = db_getOne("SELECT COUNT(*) FROM property WHERE neighborhood_id = '$id' $apts_clause")) {
      //echo("SELECT COUNT(*) FROM property WHERE neighborhood_id = '$id' $apts_clause<br>");
      $nfilters[$var][$id] = array($name, $hits);
      $total_hits += $hits;
    }
  }
  //dump($nfilters);
  //echo("total hits=$total_hits");
  /*  
  $num_unknown = db_getOne("SELECT COUNT(*) FROM property WHERE neighborhood_id = '0' $apts_clause");
  //echo("$num_unknown properties have unknown hood<br>"."SELECT COUNT(*) FROM property WHERE neighborhood_id = '0' $apts_clause");
  if(($total_hits && $num_unknown && !$_GET[$var]) || $_GET[$var] == '0000') {  // add for those not yet marked with neighborhood    
    $hits = $_GET[$var] == '0000' ? 0 : $num_unknown;
    $nfilters[$var]['0000'] = array(phrase('unknown', CAPITALIZE).' '.phrase('area', CAPITALIZE), $hits);
  }
  */
  
  return array_merge($filters, $nfilters);
}

function add_boat_type_filters($filters, $facObjs, $apt_ids) {
  global $db;
  if(!$apt_ids) return $filters;
  $nfilters = array();
  $var = 'bt';
  $apts_clause =  "AND id IN (".implode(',',$apt_ids).")";
  foreach($facObjs as $fac) {
    $name = phrase($fac->name, CAPITALIZE);
    $id= $fac->id;
    $query = $_GET[$var];       
    if(strlen($query) && strpos($query, $id) !== FALSE) { // already on
      $nfilters[$var][$id] = array($name, 0);
    } elseif($hits = db_getOne("SELECT COUNT(*) FROM property WHERE boat_type like '%$id%' $apts_clause")) {
      $nfilters[$var][$id] = array($name, $hits);
    }
  }
  return array_merge($filters, $nfilters);

}

function add_facility_filters($filters, $facObjs, $apt_ids) {
  global $db;
  if(!$apt_ids) return $filters;
  $nfilters = array();
  $var = 'fac';
  $apts_clause =  "AND id IN (".implode(',',$apt_ids).")";
  foreach($facObjs as $fac) {
    $name = phrase($fac->name, CAPITALIZE);
    $id= $fac->id;
    $query = $_GET[$var];       
    if(strlen($query) && strpos($query, $id) !== FALSE) { // already on
      $nfilters[$var][$id] = array($name, 0);
    } elseif($hits = db_getOne("SELECT COUNT(*) FROM property WHERE facilities like '%$id%' $apts_clause")) {
      $nfilters[$var][$id] = array($name, $hits);
    }
  }
  return array_merge($filters, $nfilters);
}

function bootstrap_autocomplete($autocomplete_options) {
  $placeholder = 'Where are you going?';
  $autocomplete = form_autocomplete("location_id", $autocomplete_options, array('image' => false, 'placeholder' => $placeholder));
  return $autocomplete.'<span class="input-icon"><i class="glyphicon glyphicon-map-marker"></i></span>';
}


// box on top with basic search fields
function bootstrap_searchbar($siteObj, $type, $options=array()) {  
  die("bootstrap_searchbar: No longer in use. Use include template");
  
  // location text box "Where are you going"
  if($_GET) extract($_GET);
  
  if($autocomplete_options = $options['location_options']) {
    $autocomplete = "
    <div class=\"col-sm-4 relative nogutter margin-top margin-bottom\">
      ".bootstrap_autocomplete($autocomplete_options)."
    </div>\n";
  }
  
    $output = "

    ";
    return $output;
}

// box on top with basic search fields
function searchbar($type, $search_options=array(), $options=array()) {  
  global $db;
  global $pTypeAr;                                            
  global $u_level;                 
  global $can_edit;
  $typeObj = $pTypeAr[$type];
  $city_id = $search_options['city_id'];
  
  $messages      = $options['messsages'];
  $show_submit   = isset($options['show_submit']) ? $options['show_submit'] : DB_YES;
  $fieldset_class = $options['fieldset_class'] ?: 'fieldset_top';
  $show_headers  = isset($options['show_headers']) ? $options['show_headers'] : true;
  $init_date     = isset($options['init_date']) ? $options['init_date'] : true;
  $action        = pick_first($options['action'], site_pagetype_link($_SESSION['siteObj'], property_type2page($type)), $_SERVER['REDIRECT_URL'], $_SERVER['PHP_SELF']);

  $fields_only   = isset($options['fields_only']) ? $options['fields_only'] : false;
  
  $filter_vars = array('ar', 'dis', 'fac', 'srt', 'rev');

  $map_max_count = 200;
  $check_apt_id = $_GET['sn'] ?: $options['apt_id'];
  if($check_apt_id) $aptObj = get_object($type, $check_apt_id);
  $bullet = html_bullet();
  $arrow = html_arrow();
  $formname = 'searchbar';   
  //$formname = 'rental_search_form';   
  $show_adv = true;
  
  //dump($messages);
  if($typeObj->bookable) {
    $min_stay = $aptObj->min_stay ?: get_min_stay($type); // default period for given type (in days)
    if($city_id) $city_clause = " AND city_id=$city_id"; 
    $max_guests =  $aptObj->max_guests ?: db_getOne("SELECT max(max_guests) from property WHERE type='$type' $city_clause AND active=1 AND hide=0");
    if(!$max_guests) $max_guests = 2;
  }
  
  // errors and messages
  $output = $messages;

	// $calparams->onchange = $onchange;
	// $calparams->calyoffset = $calyoffset;
	// $calparams->add = 0;
	// $caldata = $_GET['ci_d'] ? $_GET : $_POST;

	if($init_date) {
    $today = today();
    if($u_level <= USER_LEVEL_GUEST) $calparams['minDate'] = "'+0d'";
    $calparams['maxDate'] = "'+1y'";
    $calparams['init_date'] = $today;
  }
         
  $show_type = $options['show_type'] ?: $type;
  $rules = array();
  
  switch($show_type) {
  case 'hotel':
    //$facObjs = db_getAll("SELECT * FROM hotel_facility WHERE facilitytype_id IN (1,3) ORDER BY facilitytype_id, name");
    $fields['checkin']  = datepicker('in', $calparams);

    $fields['checkout'] = datepicker('out', $calparams);
    break;
   
  case 'shortstay':
    //$facObjs      = db_getAll("SELECT * FROM property_facilities WHERE searchable AND serviced ORDER BY name");
    // if(!$city_id) $fields['where_are_you_going'] = form_input('text', 'city_id');
    if($options['autocomplete']) {
      $placeholder = $options['placeholder'] ?: 'where_are_you_going?';
      $fields[$placeholder] = form_autocomplete("location_id", $search_options, $options);
      $rules1 = array(
        'location_id' => array('required' => 'true'),
        //'in' => array('required' => 'true', 'date' => 'true'),
        //'out' => array('required' => 'true', 'date' => 'true'),
      );

    } elseif(!$city_id || $options['show_city_chooser']) {
      
      $city_filters['id'] = db_getCol("select distinct(property.city_id) from property,msCity where property.type='$type' and property.active=1 and property.city_id=msCity.id and msCity.active =1;"); // todo: add site specific filters
      if(!$_SESSION['script_city']) $fields['where_are_you_going?'] = object_select('city_id', 'geo_city', array('filters' => $city_filters, 'value' => $city_id));
      $city_set = true;
    }
    
    // checkin chooser
  	$calparams['dependent_field'] = "out"; // update checkout
  	$calparams['dependent_field_add_days'] = $min_stay;  // with checkin + min_stay days
    $fields['checkin']  = datepicker('in', $calparams);

    // checkout chooser
  	unset($calparams['dependent_field']);
  	unset($options['dependent_field_add']);
  	if($init_date) $calparams['init_date'] = "'".add_day($today, $min_stay)."'";
    $fields['checkout'] = datepicker('out', $calparams);

    // num guest chooser
    $guestsArray  = num_array(1, $max_guests, 1);
    $fields['guests']   = select_from_array('ng', $guestsArray, $_GET['ng'], array('first' => $show_headers ? '' : phrase('guests')));
    break;
  case 'shortstay_office':
    //$facObjs      = db_getAll("SELECT * FROM property_facilities WHERE searchable AND office ORDER BY name");

    // checkin chooser
  	$calparams['dependent_field'] = "out"; // update checkout
  	$calparams['dependent_field_add_days'] = $min_stay;  // with checkin + min_stay days
    $fields['checkin']  = datepicker('in', $calparams);

    // checkout chooser
  	unset($calparams['dependent_field']);
  	unset($options['dependent_field_add']);
  	if($init_date) $calparams['init_date'] = "'".add_day($today, $min_stay)."'";
    $fields['checkout'] = datepicker('out', $calparams);

    // shift chooser
    $shiftArray  = array(DAY_SHIFT => phrase('day', CAPITALIZE).' (9.00-21.00)', MORNING_SHIFT => phrase('morning', CAPITALIZE).' (9.00-15.00)', AFTERNOON_SHIFT => phrase('afternoon', CAPITALIZE).' (15.00-21.00)');
    $fields['shift']   = select_from_array('sh', $shiftArray, $_GET['sh']);

    // num guest chooser
    $max_guests   =  $check_apt_id ? get_object($type, $check_apt_id, 'max_guests') : db_getOne("SELECT max(max_guests) from property WHERE type='$type' AND city_id=$city_id AND active=1 AND hide=0 AND host_id >= 1000");
    $guestsArray  = num_array(1, $max_guests, 1);
    $fields['persons']   = select_from_array('ng', $guestsArray, $_GET['ng'], array('first' => $show_headers ? '' : phrase('guests')));

    // num chairs chooser
    //$max_chairs   = $max_guests;
    //$chairsArray  = num_array(1, $max_chairs*2, 1);
    //$fields['chairs']   = select_from_array('nc', $chairsArray, $_GET['nc']);
    break;
  case 'boat':
    $first_hour = ($start_hour = hm2dectime($aptObj->start_hour)) ? $start_hour : 8;
    $last_hour = ($end_hour = hm2dectime($aptObj->end_hour)) ? $last_hour = $end_hour - $min_stay : 23;
    
    //$facObjs = db_getAll("SELECT * FROM property_facilities WHERE searchable AND boat ORDER BY name");

    global $cateringArray;
    
    if($aptObj) {
      foreach($cateringArray as $k=>$v) {
        if(!$k) continue;
        $key = 'boat_max_'.$k;
        //if(!$aptObj->$key) unset($cateringArray[$k]);
      }
    }
    //$timeArray = num_array($first_hour, $last_hour, 1, ".00");
    $timeArray = time_array($first_hour, $last_hour, 0.5);
    $hoursArray = num_array($min_stay, 12, 0.5); // half hour
    //$hoursArray = num_array($min_stay, 12); // full hour periods only
    $passengersArray = num_array(1, $max_guests, 1);

    $fields['date']       = datepicker('in', $calparams);
    $fields['time']       = select_from_array('time', $timeArray, $_GET['time'] ?: 9);
    $fields['hours']      = select_from_array('nh', $hoursArray, $_GET['nh'] ?: 3, array('first' => $show_headers ? '' : phrase('hours')));
    $fields['passengers'] = select_from_array('ng', $passengersArray, $_GET['ng'], array('first' => $show_headers ? '' : phrase('guests')));
    //$fields['catering']   = select_from_array('catering', $cateringArray, $_GET['catering']);
    break;
  case 'office':

    //$facObjs = db_getAll("SELECT * FROM property_facilities WHERE searchable AND office ORDER BY name");
    $empArray = num_array(1, 200, 1);
    $monthsArray = num_array(1, 50, 1);
    $districtObjs = db_getAll("SELECT id,name FROM msBorough WHERE parent_id=$city_id AND active=1 AND realestate=1 ORDER BY name");

    $fields['start_date'] = datepicker('in', $calparams);
    $fields['months']     = select_from_array('num_months', $monthsArray, $_GET['num_months']);
    $fields['employees']  = select_from_array('num_employees', $empArray, $_GET['num_employees']);
    break;
  case 'shortstay_office':   
    break;
  case 'realestate':
  case 'longstay':

    // search filters
    $searchArray = property_filters($_SESSION['site_id'], NULL, array('type' => $type));
    if($city_id) $searchArray['city_id'] = $city_id;    

    $c_max_living = find_object($type, $searchArray, 'max(living_size)');
    $c_max_rooms = 8;

    // figure out min/max prices
    $price_field = property_price_field($type);
    //$c_min_price = db_getOne("SELECT min($price_field) FROM property WHERE $price_field>0 AND type='$type' AND active=1 AND city_id=$city_id");
    $field = $type == 'longstay' ? 'rent_min_price' : 'sale_min_price';
    $c_min_price = db_getOne("SELECT $field FROM msCity WHERE id=$city_id");
    $c_max_price = db_getOne("SELECT max($price_field) FROM property WHERE type='$type' AND active=1 AND city_id=$city_id");

    $c_min_step = $_SESSION['min_step'][$type];
    $c_max_step = $c_min_step * 2;
    if($aid = $_SESSION['aid']) {
      if($_SESSION['aff_'.$type.'_minprice'] && $_SESSION['aff_'.$type.'_minprice'] > $c_min_price) $c_min_price = $_SESSION['aff_'.$type.'_minprice'];
      if($_SESSION['aff_'.$type.'_maxprice'] && $_SESSION['aff_'.$type.'_maxprice'] < $c_max_price) $c_max_price = $_SESSION['aff_'.$type.'_maxprice'];
    }
    $c_min_price_high = $c_max_price - $c_max_step;
    $c_max_price_low = max($_GET['min'], $c_min_price + $c_max_step);

    

    $fields['min_price']   = select_from_array('min', num_array($c_min_price, $c_min_price_high, $c_min_step, " &euro;", true, false, 30), $_GET['min'], array('first' => phrase('no_min', CAPITALIZE)));
    $fields['max_price']   = select_from_array('max', num_array($c_max_price_low, $c_max_price, $c_max_step, " &euro;", true, false, 30), $_GET['max'], array('first' => phrase('no_max', CAPITALIZE)));
    $fields['rooms']       = select_from_array('nr', num_array(0, $c_max_rooms, 1, "+", true, false, 20), $_GET['nr']);
    $fields['living_size'] = select_from_array('sz', num_array(0, $c_max_living, 10, "+", true, false, 20), $_GET['sz']);
    //$fields['bathrooms']   = select_from_array('nb', num_array(0, $c_max_rooms, 1, "+", true, false, 20), $_GET['nb']);
    break;              
  default: // real estate, longstay
    die("Searchbar - unknown type $type");
    break;
  }

  if($fields_only) return $fields; // we don't want the whole form, just the fields
  
  $output .= form_input('t', 'hidden', $type);   
  //$output .= form_input('center', 'text', $_GET['center'], array('class' => 'location-center'));   
  //$output .= form_input('bounds', 'text', $_GET['bounds'], array('class' => 'location-bounds'));   
  
  global $u_agentid;
  if($agent_id = $_GET['agent_id'] ?: $u_agentid) $output .= form_input('agid', 'hidden', $agent_id); 

  // submit button
  if($check_apt_id && $show_submit == DB_REQUIRED) {
    $fields['&nbsp;'] = form_input('search', 'submit', phrase('next', CAPITALIZE). ' => ', array('class' => 'button'));
    //$output .= form_input('nc', 'hidden', 1); // no availability check required
  } else  {
    $fields['&nbsp;'] = form_input('search', 'submit', phrase('search', CAPITALIZE), array('class' => 'button'));
  }

  //dump($fields);
  
  // print the inputs
  if($fieldset_class) {
    $output .= html_fieldset($fields, $fieldset_class, array('legend' => $options['fieldset_title'], 'translate' => true));
  } elseif($show_headers) {
    foreach($fields as $title => $input) $output .= html_float_field(($title ? phrase($title, CAPITALIZE).':' : '&nbsp;'), $input);
    // foreach($fields as $title => $input) if($title) $output .= html_float_field(phrase($title, CAPITALIZE).':', $input);  
    $output .= html_div('', 'clear_float'); // clear the float
  } else {
    foreach($fields as $title => $input) $output .= $input;
  }
  if($apt_id = $_GET['sn']) $output .= form_input('sn', 'hidden', $apt_id); // limit search to this apt (for availability)
  
  // add sort, limit and filter vars
  if(!$city_set) $filter_vars[] = 'city_id'; // don't add if we show city_chooser
  foreach($filter_vars as $var) if($val = $_GET[$var]) $output .= form_input($var, 'hidden', $val);

  // begin output
//  return $show_submit ? html_form($output, $formname, $_SERVER['PHP_SELF'], 'GET', 'searchbarform', "onsubmit='return searchbarValidate(document.$formname);'") : $output;
  //return $show_submit ? html_form($output, $formname, $action ?: $_SERVER['PHP_SELF'], 'get', 'searchbarform', "onsubmit='return cleanEmptyFields(this);'") : $output;
  //echo("Action=$action<br>");
  if($this_apt = $_SESSION['this_apt']) { // remove property URL from action
    $action = str_replace(array($this_apt->shortname."/", $this_apt->id."/"), '', $action);
  }
  
  
  if($can_edit)   $add_button = $can_edit ? html_div(property_add_link($type), 'ie_edit_link') : '';
  //echo("action=$action");
  
  $message_box = html_message_box('', array('id' => $formname."_messages"));  
  if($options['autocomplete']) {
    $handler_options['validate'] = $rules ?: array();
    $handler_options['ignore_hidden'] = false;
    $output .= html_form_handler($formname, 'property-searchbar', $handler_options);
  }
    
  $output = $show_submit ? html_form($output, $formname, $action, 'get', 'searchbarform') : $output;
  $output = html_div($message_box.$output.$add_button, 'inline relative', "id='searchbar_container'");
   
  return $output;
}


/* still to clean */
function print_facs($facs) {
  return db_id2name('property_facilities', $facs, true, true);
}
//  $facs = explode(',', $facs);
//  foreach($facs as $fac) {
//    $fac_str .= phrase(
//}

function details($apt, $compact=false) {
  $type = $apt->type;
  $cutoff = 8; // number of lines to show
  $details_array = array();
  if($type=='shortstay') {
    $details_array =  details_short($apt, $compact);
  } elseif($type=='shortstay_office') {
    $details_array =  details_shortstay_office($apt, $compact);
  } else if($type=='boat') {
    $details_array = details_boat($apt, $compact);
  } else if($type=='hotel') {
    if($num_rooms = $apt->num_rooms) $details_array[]= phrase('rooms', CAPITALIZE).": $num_rooms";
    $details_array = array_merge($details_array, details_lr($type, $apt, $compact));
  } else {
    $details_array = details_lr($type, $apt, $compact);
  }

  $details_array = array_filter(array_slice($details_array, 0, $cutoff));
  $details_str = implode(html_break(), $details_array);
  
  if(!$compact && $apt->facilities) { // show facilities
    $details_str .= html_break(2).html_header(phrase('facilities', CAPITALIZE).':', 2).print_facs($apt->facilities);  
  }  

  return $details_str;

}

function details_short($apt, $compact=false, $cutoff=10) {
  $details_array = array();

  // check affiliate min/max stay
  if($aid = $_SESSION['aid']) {
    if($apt->min_stay && $_SESSION['aff_shortstay_minstay'] && $_SESSION['aff_shortstay_minstay'] > $apt->min_stay) $apt->min_stay = $_SESSION['aff_shortstay_minstay'];
    if($apt->max_stay && $_SESSION['aff_shortstay_maxstay'] && $_SESSION['aff_shortstay_maxstay'] < $apt->max_stay) $apt->max_stay = $_SESSION['aff_shortstay_maxstay'];
  }

  $minstay = $apt->min_stay ? $apt->min_stay." ".($apt->min_stay==1 ? phrase('day', LOWERCASE) : phrase('days', LOWERCASE)) : "";
  $maxstay = $apt->max_stay ? $apt->max_stay." ".($apt->max_stay==1 ? phrase('day', LOWERCASE) : phrase('days', LOWERCASE)) : "";
  $maxguests = $apt->max_guests;

  //$details_array[]= $minstay;
  $details_array[]= $rate_html;
  $details_array[]= "$type";
  if($maxguests) $details_array[]= phrase('sleeps', CAPITALIZE).": $maxguests";
  if($minstay) $details_array[]= phrase('minimum_stay', CAPITALIZE).": $minstay";
  if($maxstay) $details_array[]= phrase('maximum_stay', CAPITALIZE).": $maxstay";


  if($districts = get_apt_borough($apt, $_SESSION['lang']))
    $details_array[]= phrase('district', CAPITALIZE).": $districts";
  if($hood = get_apt_hood($apt, $_SESSION['lang']))
    $details_array[]= phrase('area', CAPITALIZE).": $hood";

  return $details_array;
}

function details_shortstay_office($apt, $compact=false, $cutoff=10) {
  $details_array = array();

  // check affiliate min/max stay
  if($aid = $_SESSION['aid']) {
    if($apt->min_stay && $_SESSION['aff_shortstay_minstay'] && $_SESSION['aff_shortstay_minstay'] > $apt->min_stay) $apt->min_stay = $_SESSION['aff_shortstay_minstay'];
    if($apt->max_stay && $_SESSION['aff_shortstay_maxstay'] && $_SESSION['aff_shortstay_maxstay'] < $apt->max_stay) $apt->max_stay = $_SESSION['aff_shortstay_maxstay'];
  }

  $minstay = $apt->min_stay ? $apt->min_stay." ".($apt->min_stay==1 ? phrase('day', LOWERCASE) : phrase('days', LOWERCASE)) : "";
  $maxstay = $apt->max_stay ? $apt->max_stay." ".($apt->max_stay==1 ? phrase('day', LOWERCASE) : phrase('days', LOWERCASE)) : "";
  $maxguests = $apt->max_guests;

  $details_array[]= $rate_html;
  $details_array[]= "$type";
  if($maxguests) $details_array[]= phrase('desks', CAPITALIZE).": $maxguests";
  if($maxstay) $details_array[]= phrase('maximum_stay', CAPITALIZE).": $maxstay";


  if($districts = get_apt_borough($apt, $_SESSION['lang']))
    $details_array[]= phrase('district', CAPITALIZE).": $districts";
  if($hood = get_apt_hood($apt, $_SESSION['lang']))
    $details_array[]= phrase('area', CAPITALIZE).": $hood";

  return $details_array;
}

function details_boat($apt, $compact=false, $cutoff=10) {
  $details_array = array();
  if($max=$apt->max_guests) $details_array[]= phrase('max', CAPITALIZE).". ".phrase('passengers', LOWERCASE).": $max";
  if($max=$apt->boat_max_drinks) $details_array[]= phrase('max', CAPITALIZE).". ".phrase('drinks_and_snacks', LOWERCASE).": $max";
  if($max=$apt->boat_max_buffet) $details_array[]= phrase('max', CAPITALIZE).". ".phrase('buffet', LOWERCASE).": $max";
  if($max=$apt->boat_max_dinner) $details_array[]= phrase('max', CAPITALIZE).". ".phrase('dinner', LOWERCASE).": $max";
  if($min=$apt->min_stay) $details_array[]= phrase('min', CAPITALIZE).". ".phrase('hours', LOWERCASE).": $min";
  //if($apt->facilities) $details_array[]= phrase('extras',CAPITALIZE).": ".db_id2name('property_facilities', $apt->facilities, true, true);
  if($apt->start_hour && $apt->end_hour) $details_array[]= phrase('hours', CAPITALIZE).": $apt->start_hour - $apt->end_hour";
  if(!$compact) {
    if($apt->extras) $details_array[]= phrase('facilities',CAPITALIZE).": ".db_id2name('property_facilities', $apt->facilities, true, true);
    if($apt->building_year) $details_array[]= phrase('building_year',CAPITALIZE).": ".$apt->building_year;
    if(!$apt->estate_agent_id || $apt->estate_agent_id==1800) { // c&k
      //$details_array[]= "<b>".phrase('listed_by_owner', CAPITALIZE)."</b>";
    } else {
      $agentObj = get_object('estate_agent', $apt->estate_agent_id);
      $details_array[]= $agentObj->name;
    }
  }
  //if($apt->facilities) $details_array[]= phrase('facilities',CAPITALIZE).": ".db_id2name('property_facilities', $apt->facilities, true, true);

  return $details_array;
}

function details_lr($type, $apt, $compact=false, $cutoff=10) {
  $fac_table = $type=='hotel' ? 'hotel_facility' : 'property_facilities';
  $today = date("Y-m-d");
  $details_array = array();

  if($districts = get_apt_borough($apt, $_SESSION['lang'])) {
    $details_array[]= phrase('borough', CAPITALIZE).": $districts";
  }
  if($hood = get_apt_hood($apt, $_SESSION['lang'])) {
    $details_array[]= phrase('neighborhood', CAPITALIZE).": $hood";
  }

  if($apt->living_size) $details_array[]= phrase('living_size',CAPITALIZE).": ".$apt->living_size."&nbsp;m&sup2;";
  if($apt->available_from > $today) $details_array[]= phrase('available_from',CAPITALIZE).": ".sql2human($apt->available_from);
  if($apt->min_stay > 0) $details_array[]= phrase('minimum_stay',CAPITALIZE).": $min_stay ".phrase('months');
  if($apt->available_to > "0000-00-00") $details_array[]= phrase('available_to',CAPITALIZE).": ".sql2human($apt->available_to);

  if($apt->building_period) $details_array[]= phrase('building_period',CAPITALIZE).": ".db_id2name('property_building_period', $apt->building_period, true, false);
  elseif($apt->building_year) $details_array[]= phrase('building_year',CAPITALIZE).": ".$apt->building_year;

  //if($apt->num_rooms) $details_array[]= phrase('rooms',CAPITALIZE).": ".$apt->num_rooms;
  if($apt->num_bathrooms) $details_array[]= phrase('bathrooms',CAPITALIZE).": ".$apt->num_bathrooms;
  if($apt->num_bedrooms) $details_array[]= phrase('bedrooms',CAPITALIZE).": ".$apt->num_bedrooms;
  if($apt->floor) $details_array[]= phrase('floor',CAPITALIZE).": ".$apt->floor;
  if($apt->parking) $details_array[]= phrase('parking',CAPITALIZE).": ".db_id2name('property_parking', $apt->parking, true, true, true).($apt->parking_capacity>1 ? " (".$apt->parking_capacity ." ".phrase('cars',LOWERCASE).")" : "");

    if($apt->storage) $details_array[]= phrase('storage',CAPITALIZE).": ".db_id2name('property_storage', $apt->storage, true, true, true);
    if($apt->placement) $details_array[]= phrase('placement',CAPITALIZE).": ".db_id2name('property_placement', $apt->placement, true, true);
    if($apt->balcony) $details_array[]= phrase('balcony',CAPITALIZE).": ".$balcony;
    if($apt->garden) {
      $dir_str = $apt->garden_dir ? " (".phrase($apt->garden_dir).")" : "";
      $details_array[]= phrase('garden',CAPITALIZE).": ".db_id2name('property_garden', $apt->garden, true, true).$dir_str;
    }
    if($apt->notes) {
      $details_array[]= phrase('notes',CAPITALIZE).": ".db_id2name('property_notes', $apt->notes, true, true);
      //dump($apt->notes);
    }
    if($apt->facilities) $details_array[]= phrase('extras',CAPITALIZE).": ".db_id2name($fac_table, $apt->facilities, true, true);
    if($apt->outside) $details_array[]= phrase('outside',CAPITALIZE).": ".db_id2name('property_outside', $apt->outside, true, true);
    if($apt->extras) $details_array[]= phrase('extras',CAPITALIZE).": ".db_id2name($fac_table, $apt->facilities, true, true);

  if(!$compact) {

    if($size = $apt->backyard_size) {
      $d = $apt->backyard_deep;
      $w = $apt->backyard_wide;
      if($d && $w) $dim = "(".$d."m x ".$w."m)";
      if($dir = $apt->backyard_dir) $facing = phrase('facing',LOWERCASE)." ".phrase($dir, LOWERCASE);
      $details_array[]= phrase('backyard',CAPITALIZE).": $size m&#178; $dim &nbsp;$facing";
    }

    if($apt->timestamp!="0000-00-00 00:00:00" && $updated != $created) $details_array[]= phrase('last_updated',CAPITALIZE).": $updated";
    if($apt->lot_size) $details_array[]= phrase('lot_size',CAPITALIZE).": ".$apt->lot_size."&nbsp;m&sup2;";
    if($apt->volume) $details_array[]= phrase('volume',CAPITALIZE).": ".$apt->volume."&nbsp;m&sup3;";
    if($apt->floors) $details_array[]= phrase('floors',CAPITALIZE).": ".$apt->floors;
    if($apt->heating) $details_array[]= phrase('heating',CAPITALIZE).": ".db_id2name('property_heating', $apt->heating, true, true);
    if($el = $apt->energy_label) {
      preg_match('/([A-G])\s+\(([0-9])[\.\,]([0-9]{2})\)/', $el, $m);
      if($m) $details_array[]= phrase('energy_label',CAPITALIZE).": ".$apt->energy_label;
    }
    if(!$apt->estate_agent_id || $apt->estate_agent_id==1800) { // c&k
      $details_array[]= "<b>".phrase('listed_by_owner', CAPITALIZE)."</b>";
    } else {
      $agentObj = get_object('estate_agent', $apt->estate_agent_id);
      $details_array[]= $agentObj->name;
    }
    if($backyard) $details_array[]= phrase('backyard',CAPITALIZE).": ".$backyard;
    if($apt->status) $details_array[]= phrase('status',CAPITALIZE).": ".db_id2name('property_status', $apt->status, true, true);
    if($apt->insulation) $details_array[]= phrase('insulation',CAPITALIZE).": ".db_id2name('property_insulation', $apt->insulation, true, true);

    $created = sql2human(substr($apt->created, 0, 10));
    if($apt->created) $details_array[]= phrase('listed_since',CAPITALIZE).": $created";
    $updated = substr($apt->timestamp, 0, 10);
  }
  return $details_array;
}

  // type (e.g. 1 BR house, Furnished aparment
function property_descriptor($type, $apt) {

  if($type=='longstay' || $type=='shortstay' || $type=='realestate') {
    if($type=='longstay') { // furnished or not
      if($apt->furnishing) {
        //$output .= db_id2name('property_furnishing', $apt->furnishing, true, false);
        $outputArray[]= phrase(get_name('property_furnishing', $apt->furnishing), LOWERCASE);
      }
    } elseif($type=='shortstay') { // furnished or not
      $outputArray[]= phrase('serviced', LOWERCASE);
    }

    if($apt->num_rooms > 1) {
      $outputArray[]= $apt->num_rooms. " ".phrase('room', LOWERCASE);
    } elseif($num = $apt->num_bedrooms) {
      $outputArray[]= $num. " ".phrase('bedroom', LOWERCASE);
    }

    if($apt->house_type) {
      $outputArray[]= phrase(get_name('property_house_type', $apt->house_type), LOWERCASE);
    } else if($apt->apartment_type) {
      $outputArray[]= phrase(get_name('property_apartment_type', $apt->apartment_type), LOWERCASE);
    } else if($apt->object_type) {
      $outputArray[]= phrase(get_name('property_object_type', $apt->object_type), LOWERCASE);
    } else {
      $outputArray[]= (($apt->num_rooms==1 || $apt->num_bedrooms==0) ? phrase('studio', LOWERCASE) : phrase('apartment', LOWERCASE));
    }

  } elseif($type=='boat') {
    $outputArray[]= property_subtype_name($apt, CAPITALIZE);
    // $outputArray[]= 'foo';
    //global $boatTypeArray;
    //$outputArray[]= phrase($boatTypeArray[$apt->boat_type], CAPITALIZE);
  } elseif($type=='office') {
    $outputArray[]= "office descriptor";
  } elseif($type=='shortstay_office') {
    $outputArray[]= phrase($type, CAPITALIZE);
  } else { // not implemented
    return phrase($type, CAPITALIZE);
  }
  return is_array($outputArray) ? ucfirst(implode(' ', $outputArray)) : "";
}


function property_price($apt, $searchObj=NULL, $options=NULL) {
  $html = isset($options['html']) ? $options['html'] : true;  
  $cur = $html ? get_currency_symbol($apt->currency, 1) : $apt->currency;
  $type = $apt->type;
  if($p=$apt->sales_price)  return "$cur ".number_format($p); // real estate
  if($p=$apt->rental_price) return "$cur ".number_format($p)." ".html_span(phrase('per_month'), 'detail-label'); // long stay

  // rate per day  - todo: add per hour
  if($searchObj->in && $searchObj->out && $searchObj->ng) { // search, get total rate for given period (days) + number of guests
    $lines = array();
    
    list($unit, $base_rate) = get_base_rate($apt, $searchObj); // returns rate per day, hour, or shift      
    $rate_total = $apt->rate_total ? $apt->rate_total : get_rate_num_guests($base_rate, $apt, $searchObj->ng);

    // discount rate
    $rate_discount = $apt->discount ? apply_discount($rate_total, $apt->discount) : $rate_total;
    $lines [] = print_rate($cur, $rate_total, $apt->discount)." ".html_span(phrase('per_'.$unit), 'detail-label');
       
    if($searchObj->ng > 1) {
      $rate_pp    = $rate_discount/$searchObj->ng;    
      $rate_pp_str = formatFloat($rate_pp, 1, $cur)." ".($searchObj->nh > 0 ? html_span(phrase('per_person_per_hour'), 'detail-label') : html_span(phrase('per_person_per_day'), 'detail-label'));
      if($options['pp_only']) return $rate_pp_str; 
      else if($options['pp']) $lines []= $rate_pp_str;  
    }        
    
    return implode(html_break(), $lines);
    
  } elseif(hourly_rate($type)) { // shortstay or hotel - no search
    $p=$apt->rate_hour;
    return formatFloat($p, 1, $cur)." ".html_span(phrase('per_hour'), 'detail-label');
  } elseif($apt->rate_day > 0) { // shortstay or hotel - no search
    $p=$apt->rate_day;
    $unit_str = $type=='hotel' ? "" : "/".html_span(phrase('day'), 'detail-label');
    //$unit_str =  ? phrase('hour') : phrase('day');
    if($apt->rate_day_min > 0) { // from - to rates
      if($type=='shortstay') {
        $price_str = $cur.number_format($apt->rate_day_min, 2)." - $cur".number_format($apt->rate_day).$unit_str;
        //$price_str .= phrase('from', CAPITALIZE)." $cur ".number_format($apt->rate_day_min, 2)." ".html_break(1).phrase('to')." $cur ".number_format($apt->rate_day)." ".$unit_str;
      } else {
        $price_str  = html_span(phrase('from', CAPITALIZE), 'detail-label')." $cur ".number_format($apt->rate_day_min, 2)." ".$unit_str;
      }
      return $price_str;
    } else { // base rate per day - flat discount if any
      $rate_total     = get_rate_num_guests($apt->rate_day, $apt, $num_guests);
      //$rate_total_str = formatFloat($rate_total, 1, $cur)." ".$unit_str;
      //$rate_discount  = apply_discount($rate_total, get_discount_num_days($apt, 0)); // flat discount only 
      $rate_discount = $apt->discount ? apply_discount($rate_total, $apt->discount) : $rate_total;
      return $rate_discount < $rate_total ? html_span(formatFloat($rate_total, 1, $cur), 'strike').' &nbsp; '.formatFloat($rate_discount, 1, $cur)." ".html_span(phrase('per_day'), 'detail-label') : formatFloat($rate_discount, 1, $cur)." ".html_span(phrase('per_day'), 'detail-label');
    }
  }
}

function print_rate_table($aptObj, $searchObj=NULL, $options = NULL) {
  list($unit, $base_rate) = get_base_rate($aptObj, $searchObj);                                                                                         
  $debug = "base: $aptObj->rate_day (calc:$base_rate) weekend=$aptObj->rate_weekend";
  $show_weekend_rate = isset($options['weekend_pricing']) ? $options['weekend_pricing'] : true;
  if($show_weekend_rate) {
    $weekend_rate = $aptObj->rate_weekend > 0 ? $aptObj->rate_weekend : $base_rate;
    $show_weekend_rate = $weekend_rate != $base_rate;
  }
  $ng = $searchObj->ng;
  $cur = get_currency_symbol($aptObj->currency, true);

  $rate_num_guests  = $aptObj->rate_num_guests;
  $extra_guests = $aptObj->rate_num_guests > 0 ? $ng - $rate_num_guests : 0;

  // first row: header
  $row = array('#'.phrase('guests', CAPITALIZE), phrase($unit.'_rate', CAPITALIZE));
  if($show_weekend_rate) $row[] = phrase('weekend_rate', CAPITALIZE);  
  $rows[] = $row;

  // base rate (not including extra guests);
  $title = $aptObj->rate_num_guests > 0 ? "1 - $aptObj->rate_num_guests" : "1 ".phrase('or_more');
  $row  = array($title); 
  $row[] = formatFloat($base_rate, FORMAT_ALWAYS, $cur); 
  if($show_weekend_rate) $row[] = html_span(formatFloat($weekend_rate, FORMAT_ALWAYS, $cur), 'weekend_rate'); 
  $rows[] = $row;
  
  // rate for N guests (+ weekend rate for N guests if applicable)
  if($extra_guests > 0) {
    $row  = array($ng); 
    $base_rate = get_rate_num_guests($base_rate, $aptObj, $ng);
    $row[] = formatFloat($base_rate, FORMAT_ALWAYS, $cur); 
    if($show_weekend_rate) {
      $weekend_rate = get_rate_num_guests($weekend_rate, $aptObj, $ng);
      $row[] = html_span(formatFloat($weekend_rate, FORMAT_ALWAYS, $cur), 'weekend_rate'); 
    }  
    $rows[] = $row;
  }
  
  // discount rate, if applicable
  $discount = $aptObj->discount ?: percent2discount($aptObj->rate_change);
  if($discount = percent_discount($discount)) {
    $row  = array("-$discount = "); 
    $row[] = formatFloat(apply_discount($base_rate, $discount), FORMAT_ALWAYS, $cur); 
    if($show_weekend_rate) $row[] = html_span(formatFloat(apply_discount($weekend_rate, $discount), FORMAT_ALWAYS, $cur), 'weekend_rate'); 
    $rows[] = $row;
  }
  //$caption = "rc: ".$aptObj->rate_change." d=".$aptObj->discount. " dis=$discount";
  
  return $debug.html_table($rows, array('title' => $caption, 'class' => 'web20')); 
}
                   
function print_request_link($apt_or_type, $searchObj=NULL, $options=NULL) {
  $form = $options['form'] ?: false;
  $linkonly = $options['link_only'] ?: false;
  $escape = isset($options['escape']) ? $options['escape'] : true;
  $res_id = $searchObj->res_id ?: $searchObj->rid;
  if($res_id) $resObj = get_object('reservation', $res_id);
  $direct = $options['direct'] || $resObj->type == RES_TYPE_DIRECT;
    
  $siteObj = $_SESSION['siteObj'];
  if(is_array($searchObj)) $searchObj = array2obj($searchObj);

  if(is_object($apt_or_type)) {
    $apt = $apt_or_type;
    $type = $apt->type;
  } else {
    $apt->id = 0;
    $apt->type = $type = $apt_or_type;
    $searchObj->nc = 1; // don't check availability
  }
  $timestamp = stripdate($_SESSION['now']);
  $searchObj = new stdClass;
  $searchObj->id = $apt->id;

  $button_text = phrase('request', CAPITALIZE); // .' '.phrase($type, LOWERCASE);

  //dump($searchObj);die();
  
  if(in_array($type, array('shortstay', 'boat', 'shortstay_office'))) { // reservation
    $double_links = false;
    if($searchObj->in && $searchObj->out) { // this is a search, so presuambly it's available, go to request page
      $unset = array('in_disp', 'out_disp', 'search');
      foreach($unset as $var) unset($searchObj->$var);
      $qs = obj2qs($searchObj);

      if($direct_bookable = $direct ?: direct_bookable($apt, $searchObj)) { // book now link
        $booking_link = site_pagetype_link($siteObj, PAGE_BOOK, $_SESSION['language'], true).'?'.$qs;
        return $linkonly ? $booking_link : array(phrase('book_now', CAPITALIZE), $booking_link);
      }
      
      $booking_link = site_pagetype_link($siteObj, PAGE_REQUEST, $_SESSION['language'], true).'?'.$qs;
      return $linkonly ? $booking_link : array($button_text, $booking_link);
        
    } else { // requesting without having search, show search
      $booking_link = get_page_link($apt->type)."?sn=$apt->id";
      return $linkonly ? $booking_link : array($button_text, $booking_link);
    }
  } elseif($type=='hotel') { // book now
    $button_text = phrase('book_now', CAPITALIZE);
    $action_url = get_page_link('book');
  } else { // contact agent
    $button_text = phrase('contact_agent', CAPITALIZE);
    $action_url = get_page_link('enquire');
  }
  
  // below doesn't apply to reservations which return above
  // set vars
  if($aid=$_SESSION['aid']) $searchObj->aff_id = $aid;
  $searchObj->timestamp = $timestamp;
  $searchObj->request = 1;
  $searchObj->apt_id = $apt->id;
  $searchObj->search_type = $apt->type;

  // generate form or link
  if($form) {
    $form = "<form name='request_form' method='GET' action='$action_url'>";
    $form .= obj2form($searchObj);
    $form .= arrow_link($button_text, 'javascript:document.request_form.submit()');
    $form .= "</form>";
    return $form;
  }

  $booking_link = $action_url."?".obj2qs($searchObj);
  return $linkonly ? $booking_link : array($button_text, $booking_link);
  //return $linkonly ? $booking_link : arrow_link($button_text, $booking_link);
}

function print_property_alert($pa_errors, $searchObj, $inner_width, $filters, $echo=true) {
  return $output;
}

function get_tell_a_friend_form($type, $apt, $request_query="", $taf) {
  return $output;
}

function print_estate_agent_details($agentObj) {
  if(!$agentObj) return "";

  $left_rows[] = array(phrase('phone', CAPITALIZE), $agentObj->address_phone);
  $left_rows[] = array(phrase('fax', CAPITALIZE), $agentObj->address_fax);
  $left_rows[] = array(phrase('url', CAPITALIZE), html_link($agentObj->address_url, $agentObj->address_url));
  $left_rows[] = array(phrase('email', CAPITALIZE), html_link('mailto:'.$agentObj->address_email, $agentObj->address_email));
  $left = html_table($left_rows, array('title' => html_header($agentObj->name, 2), 'header' => 0, 'header_col' => 1));

  if($logo = $agentObj->logo) {$right = html_image_link($agentObj->address_url, "/photos/estate-agents/$logo");
    $rows = array();
    $rows[] = array($left, $right);
    return html_table($rows);
    //return css_table($rows);
  } else {
    return $left;
  }
}

function get_distinctions($apt, $oneapt=false, $plist=0) {

  $distinctions= array();

  if($rating = $apt->rating) {
    $star = $oneapt ? "star" : "stars";
    $distinctions[] = "<img src='/images/stars/$star$rating.png'>";
  }

  $now_ts = time();
  if($apt->status==110) {
    $distinctions[]= "<span style='color:#800;font-weight:bold;font-size:12px'>".phrase('sold', CAPITALIZE)."</span>";  ;
  } elseif(!$oneapt && $plist && in_array($apt->id, $plist)) {
    $distinctions[]= "<span style='color:#800;font-weight:bold;font-size:12px'>".phrase('already_requested', CAPITALIZE)."</span>";
  } elseif($apt->status==120) {
    $distinctions[]= "<span style='color:#800;font-weight:bold;font-size:12px'>".phrase('rented_or_sold', CAPITALIZE)."</span>";  ;
  } elseif($apt->status==130) {
    $distinctions[]= "<span style='color:#800;font-weight:bold;font-size:12px'>".phrase('rented', CAPITALIZE)."</span>";  ;
  } elseif($apt->status==100) {
    $create_age = (($now_ts - mysqldate_to_unix_timestamp($apt->created)) / 3600);
    $change_age = (($now_ts - mysqldate_to_unix_timestamp($apt->timestamp)) / 3600);

    if($apt->priority) $distinctions[]= "<span style='color:#080;font-weight:bold;font-size:12px'>".phrase('top_pick', CAPITALIZE)."</span>";  ;
    if($create_age<72) { // new
      $distinctions[]= "<span style='color:#F00;font-weight:bold;font-size:12px'>".phrase('new', CAPITALIZE)."</span>";
    } elseif($change_age<72) { // updated
      $distinctions[]= "<span style='color:#008;font-weight:bold;font-size:12px'>".phrase('updated', CAPITALIZE)."</span>";
    }
    if($apt->reactivated) $distinctions[]= "<span style='color:#0FF;font-weight:bold;font-size:12px'>".phrase('reactivated', CAPITALIZE)."</span>";  ;
  }
  return $distinctions;
}

/* hotel code */

function print_available_block($hotel_id, $full_block, $selected_block_id=false, $compact=false) {
  $fields[] = '';

  if($compact) {
    $limit = 3;
  }

  if($selected_block_id) {
    $fields[] = phrase('available_rooms', CAPITALIZE);
    $fields[] = phrase('persons', CAPITALIZE);
    $fields[] = phrase('rate_per_room', CAPITALIZE);
    $fields[] = 'Number of rooms';
  } else {
    $fields[] = phrase('available_rooms', CAPITALIZE);
    $fields[] = phrase('persons', CAPITALIZE);
    $fields[] = phrase('availability', CAPITALIZE);
    $fields[] = phrase('rate_per_room', CAPITALIZE);
    $fields[] = 'Book';
  }

  $rows[] = $fields;
  $checkin = $full_block['arrival_date'];
  $checkout = $full_block['departure_date'];
  $booking_id = $full_block['hotel_id'];
  $blocks = $full_block['block'];

  $i=0;
  foreach($blocks as $block) {
    if($limit && ++$i > $limit) break; // only show limited blocks on listing page
    $block_id = $block['block_id'];
    $booking_room_id = array_shift(explode('_', $block_id));

    $name = $block['name'];

    //if($room_id = find_object('hotel', array('source_id' => 23, 'guid' => $booking_room_id))) {
    if($roomObj = find_object('room', array('guid' => $booking_room_id))) {
      //dump($roomObj);
      $smoking_policy = $roomObj->smoking_policy;
      $room_id = $roomObj->id;
      $mi = phrase('more_information', CAPITALIZE);

      if($compact) { // link to hotel page with room details
        $apt_link = get_link('hotel', $hotel_id, false);
        $room_link = $apt_link."?blid=$block_id&room_id=$room_id";
        if($request_query=$_SESSION['request_query']) $room_link.="&".str_replace('&', '&amp;', $request_query);
        $name_str = html_link($room_link, $name);
      } else { // print room details inline in show/hide div

        // option 1 - get from our DB
        $room_details = print_room($room_id);

        // option 2 - get from block text
        $block_text = find_delimited_str($block['block_text'], '<![CDATA[', ']]>');
        $room_details = $block_text;

        $display_room = $_GET['room_id'] == $room_id; // expand div if set in query string
        $room_div = show_hide_div($room_details, $mi, $display_room);
        $name_str = "<b>$name</b> ".$room_div;
      }

    } else {
      //echo("NOT FOUND room $booking_room_id<br>");
      $name_str = "<b>$name</b>";
      $smoking_policy = 0; // let guest choose
    }

    $text = $block['block_text'];

    $incremental_price = $block['incremental_price'];
    $blocks_available = count($incremental_price); // todo: get real max

    //$room = find_object('room', array('parent_id' => $hotel_id, );

    $image_src = '';
    $block_photos  = $block['photos'];
    if($block_photos) {
      $booking_photo_id = $block_photos[0]['photo_id'];
      $image_src = $block_photos[0]['url_square60'];
    }

    $max = $block['max_occupancy'];
    $max_price_cur = $block['rack_rate']['currency'];
    $max_price = get_currency_symbol($max_price_cur, true). " ". $block['rack_rate']['price'];

    $min_price = $block['min_price']['price'];
    $min_price_cur = $block['min_price']['currency'];
    $min_price_cur_sym = get_currency_symbol($max_price_cur, true);
    $min_price_str = $min_price_cur_sym. " ". $min_price;

    $image = $image_src ? html_image($image_src) : "";

    $rate = html_strike($max_price, $min_price_str);
    $person = html_image('/images/old/head.png');
    $persons = str_repeat($person, $max);
    // dump($block);

    $row = array($image, $name_str, $persons);
    $roomArray = array();

    if($selected_block_id) {

      $row[] = $rate;
      // handle incremental pricing

      $roomArray[] = array(0, 0);

      if($blocks_available) {
        $i=1;
        foreach($incremental_price as $price) {
          $cur = $price['currency'];
          $rate = $price['price'];
          $cur_sym = get_currency_symbol($cur, true);
          $roomArray[] = array("$i:$cur:$rate:$max:$name:$smoking_policy", "$i ($cur_sym $rate)");
          $i++;
        }
      }

      $selected = $selected_block_id == $block_id ? $roomArray[1][0] : 0;
      $num_rooms_select = select_from_array("block_id:$block_id", $roomArray, $selected);
      $row[] = $num_rooms_select;
    } else {

      $row[] = $blocks_available;
      $row[] = $rate;
      $room_link = "/book.php?t=hotel&id=$hotel_id&bid=$booking_id&in=$checkin&out=$checkout&blid=$block_id";
      $booking_link = "/book.php?t=hotel&id=$hotel_id&bid=$booking_id&in=$checkin&out=$checkout&blid=$block_id";
      $button = arrow_link(phrase('book_now', CAPITALIZE), $booking_link);
      $row[] = $button;
    }

    $rows[] = $row;

  }
  //$caption = phrase('availability', CAPITALIZE).": "; // add checkin checkout

  return html_table($rows, array('title' => $caption, 'class' => 'availability_block'));
}

function print_description_type($id, $description, $lang='') {
  $title = get_object_field('description_type', $id,'name');
  $title = phrase($title);
  return print_sub_section($title, $description);
}

function print_sub_section($title, $description) {
  $output = '
  <p class="subheading">'.$title.'</p>
  <div class="descriptionsContainer">
    <div class="description">
    '.$description.'
    </div>
  </div>
  ';
  return $output;
}
/* end hotel code */

// saves HTML text from CKEditor - either checkin instructions or property descriptions
function save_property_descriptions($property_type, $property_id, $inputVars, $desc_type=DESC_TYPE_PROPERTY, $field_prefix='description') {
  $site_id = $inputVars['site_id'];
  if($site_id == 10) $site_id = 0; // hack (for now), since AFR is default 
  
  $pd_search['property_type'] = $property_type;
  $pd_search['type']          = $desc_type; //array(DESC_TYPE_PROPERTY, DESC_TYPE_HOTEL);
  $pd_search['property_id']   = $property_id;
  $pd_search['site_id']       = $site_id; // todo: update all descriptions, set site_id to 30 for AFR

  $errors = $ids = array();
  $id = $inputVars['id'];
  //dump($inputVars);die();
  foreach($inputVars as $k => $val) {
    list($fld, $lang) = explode('_', $k);
    if($fld == $field_prefix) {
      $pd_search['language']           = $lang;
      $did = find_object('property_description', $pd_search, 'id');
      //echo("did=$did");dump($pd_search);
      $pd_data = $pd_search;
      $pd_data['description'] = $val;
      $pd_data['type'] = $desc_type;
      list($nid, $nerrors) = save_object('property_description', $pd_data, $did);
      $ids[] = $nid;
      if($nerrors) $errors = array_merge($errors, $nerrors);
    }
  }
  return array($ids, $errors);
}

function add_property_description($description, $lang, $pid, $ptype, $source_id='', $action='') {
  $aptObj = get_object($ptype, $pid);
  
  if(!strlen(trim($description))) return;
  $descSearch['language'] = $lang;                                                                                                                                                                                        
  $descSearch['type'] = DESC_TYPE_PROPERTY; // property information
  $descSearch['property_id'] = $pid;
  $descSearch['property_type'] = $ptype;
  $descObj = find_object('property_description', $descSearch);
  
  $desc = $descSearch;  
  if($source_id) $desc['source_id'] = $source_id;
  $desc['description'] = $description;
  $desc['city_id'] = $aptObj->city_id;
  $desc['host_id'] = $aptObj->host_id;
  $desc['source_id'] = $aptObj->source_id;
  $desc['action'] = $action;
  $desc['type'] = DESC_TYPE_PROPERTY; // property information
      
  if($descObj) {
    //echo("Updating property description in $lang for $pid<br />");
    list($desc_id, $errors) = update_object('property_description', $desc, $descObj->id);
  } else {        
    //echo("Adding property description in $lang for $pid<br />");
    list($desc_id, $errors) = add_object('property_description', $desc);
  }
  return array($desc_id, $errors);
}

//function property_description($siteObj, $id, $lang='', $desctype_id = DESC_TYPE_PROPERTY) {
function property_description($siteObj_or_id, $id, $lang='', $options=NULL) {
  if($siteObj_or_id) $siteObj = any2obj('site', $siteObj_or_id);
  $site_id = $options['site_id'] ?: ($siteObj->source_site_id ?: $siteObj->id);

  $searchArray['type'] = $options['type'] ?: DESC_TYPE_PROPERTY; // property information
  $searchArray['property_id'] = $id;
  $searchArray['language'] = $lang ?: $_SESSION['language'];
  $searchArray['site_id'] = $site_id ? array($site_id, 0, 10) : 0;
  
  $descObjs = find_objects('property_description', $searchArray, array('sort' => 'site_id desc'));
  $descObj = $descObjs[0];
  if(!$descObj && !$options['force_language']) { // not found, try input language
    $input_lang = get_object('property', $id, 'input_lang');
    if($input_lang && $input_lang != $lang) {
      $searchArray['language'] = $input_lang;
      $descObj = find_object('property_description', $searchArray);
    }
  }
  return $descObj;
}

// function print_property_description($siteObj, $aptObj, $lang='', $options=NULL, $can_edit=false) {
function print_property_description($siteObj, $pageObj, $aptObj, $options=NULL) {
  global $edit;
  $can_edit = $options['can_edit'];
  $lang = $options['language'] ?: $_SESSION['language'];
  
  $id = $aptObj->id;
  $descObj = property_description($siteObj, $aptObj->id, $lang, $options);

  // what happends if no descObj ? test
  if($edit == 'property_description:description:'.$descObj->id) {
    $path = $_SERVER['REDIRECT_URL'] ?: $_SERVER['PHP_SELF'];
    $lang = $options['language'] ?: $_SESSION['language'];
    global $inputLangArray;    
    if($lang_name = $inputLangArray[$lang]) {
      $instructions = "Please write a description in $lang_name".html_break();
      $site_id = $options['site_id'] ?: $siteObj->id;
      //echo("site_id = $site_id");
      if($site_id > 10) {
        $site_name = get_object('site', $site_id, 'domain');
        $instructions .= "This will only apply to the site $site_name".html_break();
      }
      //$instructions .= "path=$path";
      $extra_fields = array('site_id' => $site_id, 'language' => $lang, 'property_type' => $aptObj->type, 'property_id' => $aptObj->id);
      $desc = html_div(html_div($instructions, 'message').inline_edit_form('property_description', 'description', $descObj->id, $descObj->description, 'html', '', $path, $extra_fields), 'top-z');
    } else {
      $desc = html_div("Unsupported input language $lang_name ($lang)", 'error');
    }
  } else {
    $desc = html_tidy($descObj->description);
    //dump($aptObj);die();
    if($peak_exceptions = find_surcharge_exceptions($aptObj)) {    
      $desc .= html_h2(phrase('exceptions', CAPITALIZE).':');
      //$desc .= html_h2("Peak season surcharge");
      $desc .= print_exceptions($aptObj->type, $id, $peak_exceptions);
    }
  
    if($descObj->machine_translated) {
      $desc .= html_break().html_div(' [ '.phrase('machine_translated', CAPITALIZE).' ] ', 'tiny italics');
      //$desc .= google_branding('google_branding_'.$id);
    }
  }

  if($can_edit) {
    $field = 'description';
    $edit_options['fields'] = array('site_id', 'description');
    $edit_options['inline'] = true; 
    $data = array('site_id' => $siteObj->id); 
    if($edit_options['inline']) {
      $id = $edit_options['inline'] || $descObj->site_id == $siteObj->id ? $descObj->id : 0; // force add if different site
      if(!$id) $data['description'] = $descObj->description;
    } 
    $edit_options['data'] = $data;
    $edit_button = html_div(inline_edit_link('property_description', $field, $id, $edit_options), 'ie_edit_link');
    $desc .= $edit_button;
  }
  
  return html_div($desc, 'property_description relative');
}

function property_feedback_bar($obj, $class='', $options=NULL) {
  $siteObj = $_SESSION['siteObj'];
  $score = round($obj->review_score, 1);
  $count = $obj->review_nr;  
  if(!($score > 0) || $count < 3) return '';

  $bar = feedback_bar($score);
  $count_str = " ($count ".($count == 1 ? phrase('review') : phrase('reviews')).')';
  
  $url = $options['url'] ?: property_link($siteObj, $obj); 
  $count_link = "$url#tabs_tab_reviews";
  $output = ($count >= 1) ? $bar.html_link($count_link, $count_str) : '';
  //return $output;
  return html_div($output, $class);  
}

function print_feedback($obj_or_id, $options=NULL) {
  $options = (array) $options;
  $full = $options['full_name'];
  // dump($options);
  $obj = is_object($obj_or_id) ? $obj_or_id : get_object('feedback', $obj_or_id);
  if($options['hide_city']) $obj->city = NULL; 
  $country  = $obj->country ? phrase(get_object('countries', $obj->country, 'name'), CAPITALIZE) : '';
  $flag     = $obj->country ? html_flag($obj->country).' ' : '';
  $name     = $options['full_name'] ? get_name_string($obj) : pick_first($obj->first_name, $obj->last_name);
  $name     = ($obj->anonymous || !$name) ? html_h2(phrase('anonymous', CAPITALIZE)) : html_h2($name);
  $profile  = $obj->profile ? html_h3(phrase(get_object('profile', $obj->profile, 'name')), CAPITALIZE) : '';
  $address  = $obj->city || $country ? html_div($flag.implode(', ', array_filter(array($obj->city, $country)))) : '';
  $date     = $options['hide_date'] ? '' : sql2human($obj->created);
  $pos      = $obj->feedback_comments ? html_div(html_div('&nbsp;', 'inline-block sprites sprite-positive').' '.$obj->feedback_comments, 'feedback_comment') : '';
  $neg      = $obj->feedback_comments_neg ? html_div(html_div('&nbsp;', 'inline-block sprites sprite-negative').' '.$obj->feedback_comments_neg, 'feedback_comment') : '';
  $score    = $obj->feedback_overall == 10 ? 10 : number_format($obj->feedback_overall, 1);
  $score    = $score > 0 ? html_div($score, 'sprites sprite-speech-bubble score') : '';  

  if($can_edit) {
    //echo("gh");
    $feedback_fields = array('created', 'first_name', 'last_name', 'city', 'country', 'feedback_comments', 'feedback_comments_neg'); 
    $edit_button = html_div(inline_edit_link('feedback', '', $obj->id, array('fields' => $feedback_fields)), 'ie_edit_link');
    //$score .= $edit_button;
  }

  $cols[]= $name.$profile.$address.$date.$edit_button;;
  $cols[]= $pos.$neg;
  $cols[]= $score;

  if($full) { // print individual scores
    $apt = get_object('property', $obj->apt_id);
    $cats = feedback_categories($apt->type);
    foreach($cats as $cat) {
      $key = 'feedback_'.$cat;
      $scores[$cat] = $obj->$key;
      $rows[] = print_feedback_cat_row($cat);
    }
    $cat_scores = json_encode($scores);
    $ind_scores = html_table($rows, array('header' => 0));
    //$total_score = html_div($total_score, 'score', 'id="feedback_total"');  
    $ind_scores.= html_script_inline("feedback_update($cat_scores);");    
    
  }
    
  return html_div(css_columns($cols, '', '', array('col1', 'col2', 'col3')), 'feedback').$ind_scores;
}

function print_feedback_cat_row($cat) {
  return array(html_span(phrase($cat, CAPITALIZE).': ', 'bold', "id='feedback_label_".$cat."'").'&nbsp; ', feedback_bar(0, 'feedback_'.$cat));
}

function feedback_score($apt_id) {         
  global $db;
  $qs = "SELECT avg(feedback_overall) as score, count(*) as count FROM feedback WHERE apt_id=$apt_id AND feedback_overall > 0 AND published=1";
  $feedback = db_getRow($qs);
  if(!$feedback->count) $feedback->score=0;
  $feedback->recommend = db_getOne("SELECT count(*) FROM feedback WHERE apt_id=$apt_id AND feedback_recommend=1 AND feedback_overall > 0 AND published=1");
  return $feedback;
}

function update_property_feedback($apt_id) {
  $feedback = feedback_score($apt_id);
  if($feedback) list($id, $errors) = update_object('property', array('review_nr' => $feedback->count, 'review_score' => $feedback->score, 'review_recommend' => $feedback->recommend), $apt_id);
}

function feedback_count($apt, $search=NULL) {
  $reviews = find_feedback($apt, $search);
  return count($reviews);
}


//function find_feedback($apt, $lang='', $profile=-1) {
function find_feedback($apt, $search=NULL, $all_langs=false, $options = NULL) {
  $options = (array) $options;
  $limit = 0; // later, cut off
  $sort = 'created';
  $order = 1;
  $lang = $search['language'];
  $search['published'] = 1;
  $search['apt_id'] = $apt->id;   
  $reviews = find_objects('feedback', $search, array('sort' => $sort, 'order' => $order)); // this language
  if($lang && $all_langs) {
    $search['language'] = array('<>', $lang);
    $reviews = array_merge($reviews, find_objects('feedback', $search, array('sort' => $sort, 'order' => $order))); // all other language
  }
  return $reviews;
}
  
function feedback_parent_field($obj_type) {
  switch($obj_type) {
    case 'reservation':
      return 'parent_id';
    case 'site_page':
      return 'page_id';
    case 'product':
      return 'product_id';
    default:
      return '';
  }
}

function feedback_average($search) {
  global $db;
  $search['feedback_overall'] = array('>', 0); // only count feedback with overall score  
  $filters = search2where($search);
  $qs = "select count(id) as count, round(avg(feedback_overall), 1) as score from feedback WHERE $filters";
  //echo($qs);  dump($feedbackRow);
  $feedbackRow = db_getRow($qs);
  return array($feedbackRow->score, $feedbackRow->count);
}

function feedback_stars($name, $score, $count, $options=NULL) {
  $url = $options['url'];
  $rating_w = round(64/10 * $score) + 1; // score is out of 10
  $stars_score = round($score / 2, 1); // stars = review out of 5
  $output = "<span title=\"Rating $name = $stars_score / 5\" style=\"width:".$rating_w."px !important\" class=\"rating\"></span>\n";
  if($options['stars_only']) return $output;
  $output .= $url ? "<a href=\"$url#tab_reviews\" title=\"Reviews $name\" class=\"reviewtitle\">($count reviews)</a>\n" :
                     "<br><span class=\"reviewtitle\">$count reviews</span>\n";
  return $output;
}

function feedback_bar($score, $id='') {
  $w = 11 * $score - 1;
  $stripe = html_div('', 'feedback_stripe', "style='width:".$w."px'".($id ? " id='$id'" : ''));
  $score_str = $score;
  // $score_str = $id ? $score : "$score";
  return html_div($stripe, 'sprites feedback-bar sprite-feedback_bar float_left').' &nbsp;'.html_span($score_str, 'bold float_clear', 'id="'.$id.'_score"');
  //return html_div($stripe, 'feedback_bar float_left').html_span($score_str, 'bold', 'id="'.$id.'_score"');
}

function feedback_tab($apt, $options = array()) {
  $siteObj = $_SESSION['siteObj'];  
  $search['site_id'] = $siteObj->source_site_id ?: $siteObj->id;
  $search['language'] = $_SESSION['language'];
  //dump($options);die();
  $reviews = find_feedback($apt, $search);
  $lang = $_SESSION['lang'];  

  // summary                                    
  $cats = feedback_categories($apt->type);  
  foreach($cats as $cat) $rows[] = print_feedback_cat_row($cat);
  $total_score = html_div($total_score, 'score sprites sprite-speech-bubble', 'id="feedback_total"');  

  $cat_scores = feedback_category_scores($apt, $cats);
  $script = html_script_inline("feedback_update($cat_scores);");
  $cols[] = html_table($rows, array('header' => 0));
  $cols[] = feedback_choosers($apt, $cats, $options);
  $cols[] = html_h2(phrase('total', CAPITALIZE).':').html_break().$total_score;

  $summary = html_div(css_columns($cols, '', '', array('col1', 'col2', 'col3')), 'feedback_summary');

  // individual reviews
  foreach($reviews as $review) $individual .= print_feedback($review, ($options['feedback'] ?: $options));

  return $summary.html_div($individual, '', 'id="feedback_ind"').$script;
}

// returns the feedback categories to display
// if input is true, the 'overall' category is ommited, as this is calculated
function feedback_categories($type, $input=false) {
  switch($type) {
    case 'shortstay':
      $result = array('location', 'comfort', 'cleaning', 'value');
      break;
    case 'boat':
      $result = array('catering', 'service', 'value');
      break;
    default: 
      die("feedback_categories: Review for $type not yet implemented.");
  }
  if(!$input) $result[] = 'overall';
  return $result;
}

function feedback_choosers($apt, $cats, $options) {
  global $db;
  $profiles = db_getCol("SELECT DISTINCT profile FROM feedback WHERE apt_id=$apt->id AND published=1 ORDER BY profile DESC");
  $apt_id = $apt->id;
  if(count($profiles) < 2) return '';
  $lang = $_SESSION['language'];
  $options_json = json_encode($options);

  //echo($options_json);
  //$siteObj = $_SESSION['siteObj'];  
  //$lang_clause = ($lang = $_SESSION['language']) ? "language = '$lang' AND " : '';  
  //$site_clause = ($site_id = $siteObj->source_site_id ?: $siteObj->id) ? "site_id = $site_id AND " : "";

  // all revies
  $count = db_getOne("SELECT count(id) FROM feedback WHERE apt_id=$apt->id AND published=1");
  $profile_name = phrase('all',CAPITALIZE).' '.phrase('reviews');
  $cat_scores = feedback_category_scores($apt, $cats);
  $profile = -1;
  $js = "onmouseover='feedback_update($cat_scores);' onclick='feedback_load($profile, $apt_id, \"$lang\", $options_json);\$(\"ul#feedback_choosers a\").removeClass(\"selected\");\$(this).addClass(\"selected\");'";
  $links[] = html_link('#tab_reviews', $profile_name." ($count)", array('title' => $profile_name, 'js' => $js));
  //dump($profiles);                                                     
  // per category (profile)
  foreach($profiles as $profile) {
    $count = db_getOne("SELECT count(id) FROM feedback WHERE apt_id=$apt->id AND published=1 AND profile=$profile");
    if(!$count) continue;
    $profile_name = $profile ? phrase(get_object('profile', $profile, 'name'), CAPITALIZE) : phrase('unknown', CAPITALIZE);
    $cat_scores = feedback_category_scores($apt, $cats, $profile);
    $js = "onmouseover='feedback_update($cat_scores);' onclick='feedback_load($profile, $apt_id, \"$lang\", $options_json);\$(\"ul#feedback_choosers a\").removeClass(\"selected\");\$(this).addClass(\"selected\");'";
    $links[] = html_link('#tab_reviews', $profile_name." ($count)", array('title' => $profile_name.' '.phrase('reviews'), 'js' => $js));
  }
  return html_ul($links, 'link_list', array('js' => "id='feedback_choosers'"));
}

function feedback_category_scores($apt, $cats, $profile=0) {
  global $db;                                                                                       
  //$siteObj = $_SESSION['siteObj'];
  //$lang_clause = ($lang = $_SESSION['language']) ? "language = '$lang' AND " : '';  
  //$site_clause = ($site_id = $siteObj->source_site_id ?: $siteObj->id) ? "site_id = $site_id AND " : "";

  $cat_scores = array();                                                                          
  foreach($cats as $cat) {                                                                        
    $field = 'feedback_'.$cat;                                                                    
    $qs = "SELECT avg($field) FROM feedback WHERE apt_id=$apt->id AND $field > 0 AND published=1";    
    if($profile) $qs .= " AND profile=$profile";
    $cat_scores[$cat] = round(db_getOne($qs), 1);                                               
  }
  //echo("scores for profile=$profile and apt=$apt->name");
  //dump($cat_scores);
  return json_encode($cat_scores);                                                                
} 



function print_date_time_range($checkin, $checkout) {
  $in_date = strip_time($checkin);
  $out_date = strip_time($checkout);

  $in_time = strip_date_sec($checkin);
  $out_time = strip_date_sec($checkout);

  if($in_date == $out_date && $out_time > $in_time) { // same day, different times
    return sql2human($checkin)." ".sql2human_time($checkin)." - ".sql2human_time($checkout);
  } else {
    return sql2human($checkin)." - ".sql2human($checkout);
  }
}

// returns a sorted subset of apts
function property_sort_slice($apts, $sort, $order, $first, $count) { 
  //return $apts;
  if($sort == 'rate_day') { // hack to manually sort by actual day rate
    foreach($apts as $tmpapt) {
      list($unit, $base_rate) = get_base_rate($tmpapt, $searchObj); // returns rate per day, hour, or shift      
      $tmpapt->rate_base     = $rate_base = get_rate_num_guests($base_rate, $tmpapt, $searchObj->ng);
      $tmpapt->rate_discount = $tmpapt->discount ? apply_discount($rate_base, $tmpapt->discount) : $rate_base;
      // echo("<b>During sort:</b> base='$unit $base_rate' $tmpapt->name base: $tmpapt->rate_base $tmpapt->rate_discount<br>");

    }
    //echo("<b>Before sort:</b><br>");foreach($apts as $tmpapt) echo("$tmpapt->name base: $tmpapt->rate_base $tmpapt->rate_discount<br>");
    usort($apts, ($order ? 'rate_sort_rev' : 'rate_sort')); // sort apts
    //echo("<br><b>After sort:</b><br>");foreach($apts as $tmpapt) echo("$tmpapt->name base: $tmpapt->rate_base $tmpapt->rate_discount<br>");
  }
  if(!$count) $count = count($apts);
  $apts  = count($apts) ? array_slice($apts, $first, $count) : array(); // slice the apts we want to display
  
  return $apts;
}
  
//function property_slider($type, $searchArray, $width=0, $first=0, $aid=0) {
function property_slider($type, $searchArray, $options=NULL) {
  $siteObj = $_SESSION['siteObj'];
  if(!is_array($options) && strlen($options)) $options = json_decode($options, true); // from app_data.php
  $width = $options['width'];
  $limit = $options['limit'] ?: $options['count']; // number of images to show
  $first = $options['first'] ?: $options['start']; // which to show first
  $aid   = $options['aid'];
  $ajax  = $options['ajax']; // means this is called from app_data.php, so don't show container
  
  if(!$limit) { // calculate
    $inner_width = $width ?: $_SESSION['inner_width'];
    $small_pic_width = $_SESSION['small_pic_width'] ?: 200;
    $limit = round($inner_width / $small_pic_width);
  }
  
  $prev = $first-$limit >= 0 ? $first-$limit : 0;
  $next = $first+$limit;
  if($aid)  $_SESSION['aff_id'] = $_SESSION['aid'] = $aid;
  $base_link = get_page_link($type);
  $prev_link = $base_link."?start=$prev";           
  $next_link = $base_link."?start=$next";
  $sort = ($options['sort'] ?: $_SESSION['property_sort'][$type]) ?: 'priority DESC, book_direct DESC, created DESC';
  $apts = find_objects($type, $searchArray, array('sort' => $sort, 'order' => $order, 'start' => $first, 'limit' => $limit));
  //dump($searchArray);
  $count = get_objects_count('property', search2filters($searchArray));

  $json_search = json_encode($searchArray);
  $ajax_url = "/home/app_data.php?ac=property-slider&type=$type&limit=$limit&search=".urlencode($json_search)."&aid=".$_SESSION['aid'];  
  $effect1 = '$(\'#property-slider\').animate({marginLeft: \'+=1000\',opacity: 0.25}, 500);';
  $effect2 = '$(\'#property-slider\').animate({marginLeft: \'-=1000\',opacity: 0.25}, 500);';
  $effect1 = $effect2 = '$(\'#property-slider\').animate({opacity: 0.25}, 200);';
  $js1 = 'onclick="'.$effect1.'$(\'#property-slider-cont\').load(\''.$ajax_url.'&start='.$prev.'\');return false;"';
  $js2 = 'onclick="'.$effect2.'$(\'#property-slider-cont\').load(\''.$ajax_url.'&start='.$next.'\');return false;"';
  foreach($apts as $apt) {
    $purl         = property_link($siteObj, $apt);
    $pic          = html_link($purl, get_first_pic($apt, array('size' => 'small', 'show_default' => true)));
    $link         = html_link($purl, $apt->name);
    $price        = property_price($apt);
    $distinctions = implode(html_break(), get_distinctions($apt, false));
    //$distinctions = 'foo<br>bar';
    $property_str = html_div(implode(html_break(), array($link, $price, $distinctions)), '');
    $properties   .= html_div($pic.$property_str, 'property_box');
  }
  $clear = html_div('', 'clear_float'); // clear the float

  $prev_arrow = $first ? html_div(html_image_link($prev_link, "/images/new/grey_prev.png", phrase('previous', CAPITALIZE), array('js' => $js1)), 'property-slider-arrow') : "";
  $next_arrow = $first+$limit < $count ? html_div(html_image_link($next_link, "/images/new/grey_next.png", phrase('next', CAPITALIZE), array('js' => $js2)), 'property-slider-arrow') : "";

  $pslider = html_div($properties, $class, array("id" => 'property-slider')); // properties only
  $pslider2 = $prev_arrow.$pslider.$next_arrow.$clear; // with arrows

  $output = $ajax ? $pslider2 : html_div($pslider2, 'content_width', 'id="property-slider-cont"');
  //$output .= "foo limit=$limit";
  return $output;
}

function property_calendar($aptObj, $options = array()) {
  require_once('helpers_calendar.inc');
  
  global $__header;
  global $u_level;
  $__header['scripts'][] = "/afr/js/admincal.js";
  $__header['css'][] = "/afr/css/cal.css";
  $__header['css'][] = "/afr/css/afr_gmenu.css";

  $type = $aptObj->type;                   
  $id = $aptObj->id;  
  $step = $_GET['st'] ?: $step;
  if(!$date) $date = $_GET['in'];
  $type_name = property_type_name($type, 1); // 'boat'; 
  $type_name_plural = property_type_name($type, 2); // e.g. 'boats'
  
  $ajax_url = $_GET['ac'] == 'property-calendar' ? $_SERVER['REQUEST_URI'] : "/home/app_data.php?ac=property-calendar&type=".$aptObj->type."&id=".$aptObj->id."&aid=".$_SESSION['aid'];
  $ajax_target = 'calendar_picker';       
  
  $org_params = $_SESSION['cal_params'];
  
  $cal_params = $_SESSION['cal_defaults'];
  if(!$num_hours) $num_hours = $aptObj->min_stay;
  $ajax_url = replace_uri_var($ajax_url, 'nh', $num_hours);
  if($date) $ajax_url = remove_uri_var($ajax_url, 'target_date');

  $cal_params->standalone = 1;
  $cal_params->hide_past = 1;
  $cal_params->show = 'reservation';
  $cal_params->user_level    = $u_level;
  $cal_params->show_filters = 0;
  $cal_params->hide_apts = 1;
  $cal_params->hide_res = 0; 
  $cal_params->hide_weeks = 1;
  $cal_params->ajax_target = $ajax_target;             
  $cal_params->period['d'] = 1;
  $cal_params->ajax_success_handler = "
    //alert(dump(urlParams));
    init_property_calendar(urlParams);
  ";
  $cal_params->simple_menu = 1; 
  $cal_params->max_months = 12;
  $cal_params->allow_day_view = false;
  $cal_params->allow_week_view = false;
  //$cal_params->zoom = 0.5; 

  
  if($date) {
    list($td, $tm, $ty) = sql2date($date); 
    $cal_params->target_day   = $cal_filters['d'] = $td;
    $cal_params->target_month = $cal_filters['m'] = $tm;
    $cal_params->target_year  = $cal_filters['y'] = $ty;
    //$debug .= $date.html_break();
    //$ajax_url .= "&target_date=$date";
  } else {
    $cal_filters['d'] = 0;
  }
  //$debug .= $ajax_url.html_break();
  $cal_params->base_url = $ajax_url;
  $cal_params->summary_view = $_GET['d'] || $td ? false : true;

  $cal_filters['p'] = 1;
  $cal_filters['pp']['d'] = 1;
  $cal_filters['z'] = 0.8;
  $cal_filters['f'] = 0;
  $cal_filters['t'] = $type;
  $cal_filters['apt_list'] = $id;

  $cal_filters['sh'] = $aptObj->start_hour ? hm2dectime($aptObj->start_hour) : 8;
  $cal_filters['eh'] = $aptObj->end_hour ? hm2dectime($aptObj->end_hour) - $num_hours + 0.5 : 24;
  $cal_filters['nh'] = $num_hours;
  $cal_params->filters = $cal_filters;

  if(is_array($options)) foreach($options as $k=>$v) $cal_params->$k = $v;
  //dump($cal_params);
  $_SESSION['cal_params'] = $cal_params;
  $calendar = calendar($cal_title, $cal_params);
  return $calendar;  


  // below not used anymore 
  
  $content = html_div($calendar, 'cal_picker', $style);


  //$output = html_h2($title).html_break();
  $output .= $debug;  
  $output .= $content;  
  //$style = " style='height:260px;border:1px solid white'";

  return html_div($output, '', 'id="calendar_picker"'.$style);

  if(count($messages)) $output .= print_messages($messages);
  if(count($errors)) $output .= print_errors($errors);

  if($step > 1) {
    $prev_step = $step - 1;
    $ajax_url = replace_uri_var($ajax_url, 'st', $prev_step);
    $js = ajax_load_onclick($ajax_target, $ajax_url, 'init_property_calendar();');                       
    $back_link = html_link('#', phrase('back', CAPITALIZE), array('js' => $js));
    $content .= html_break().$back_link;
    //$debug .= "step=$step";
  }

  // $urlParams = $date ? "{'ac':'property-calendar','type':'boat','id':'101404','aid':'','nh':'1','y':'2011','m':'12','d':'1','w':'0'}" : '""';
  $urlParams = $date ? "{'y':'".$ty."','m':'".$tm."','d':'".$td."','w':'0'}" : '""';
  
  $__header['onload'] .= '
    //alert("onload");
    var urlParams = '.$urlParams.';
    init_property_calendar(urlParams);
  '.$js.'
    $("#in_disp").change(function() { 
      var date = $("#in").val();
      var ymd = sql2ymd(date);
      //alert(date + "Y=" + ymd.y + " m=" + ymd.m + " d=" + ymd.d);
      $(\'#calendar_picker\').addClass(\'spinning_wheel\');
      $(\'#calendar_picker\').load(\'/home/app_data.php?ac=property-calendar&type='.$aptObj->type.'&id='.$aptObj->id.'&aid='.$_SESSION['aid'].'&target_date=\' + date, function(response, status, xhr) {
        init_property_calendar(ymd);
        $(\'#calendar_picker\').removeClass(\'spinning_wheel\');
      }); 
    });
    

    $("#nh").change(function() {
      if($("#in_disp").data("day_mode")) return;
      var nh = $("#nh").val();
      var ym = fixDate(g_target_year + "-" + g_target_month + "-00");
      //alert(ym);
      $(\'#calendar_picker\').addClass(\'spinning_wheel\');
      $(\'#calendar_picker\').load(\'/home/app_data.php?ac=property-calendar&type='.$aptObj->type.'&id='.$aptObj->id.'&aid='.$_SESSION['aid'].'&nh=\' + nh + \'&target_date=\' + ym, function(response, status, xhr) {
        init_property_calendar();
        $(\'#calendar_picker\').removeClass(\'spinning_wheel\');
      }); 
      return false;
    });
  ';      
  
    
}


// lets user pick date/time/property for available properties using the calendar
function calendar_picker($aptObj, $step=1) {
  global $__header;
  $__header['scripts'][] = "/afr/js/admincal.js";
  $__header['css'][] = "/afr/css/cal.css";

  $type = $aptObj->type;                   
  $id = $aptObj->id;  
  $step = $_GET['st'] ?: $step;

  $type_name = property_type_name($type, 1); // 'boat'; 
  $type_name_plural = property_type_name($type, 2); // e.g. 'boats'
  
  $ajax_url = $_GET['ac'] == 'cal-picker' ? $_SERVER['REQUEST_URI'] : "/home/app_data.php?ac=cal-picker&st=$step&type=".$aptObj->type."&id=".$aptObj->id."&aid=".$_SESSION['aid'];
  $ajax_target = 'calendar_picker';       

  if($pid = $aptObj->parent_id) $aptObj = get_object($type, $pid);
  $title = phrase('step', CAPITALIZE).' '.$step.': ';

  switch($step) {
  case 1:
    $title .= "How long would you like to rent the $type_name for?";
    $content = rate_array_chooser($aptObj); 
    break;
  case 2:    
    require_once('helpers_calendar.inc');
    $title .= phrase('pick_a_date_and_time', CAPITALIZE);   
    //$messages[] = html_span('X', 'red'). ' = '.phrase('unavailable');
    //$messages[] = html_span('Y', 'green'). ' = '.phrase('available');
    $messages[] = "Explanation:";
    //$messages[] = html_span('X', 'red'). " = No more $type_name_plural available.";
    //$messages[] = html_span('Y', 'green'). " = Still some $type_name_plural available.";
    $siteObj = $_SESSION['siteObj'];
    $phone = $siteObj->work_phone;
    $phone_str = (strlen($phone) > 5 ? ": $phone" : ".");
    $messages[] = html_image('/images/icons/check_off_tiny.png'). " = No more $type_name_plural available.";
    $messages[] = html_image('/images/icons/check_on_tiny.png'). " = Still some $type_name_plural available.";
    $messages[] = "A number refers to the amount of $type_name_plural still available for the selected time and period.";   
    $messages[] = "Please note that online bookings are only available starting tomorrow. For bookings for today, please call us".$phone_str;
    
    $org_params = $_SESSION['cal_params'];
    $cal_params = $_SESSION['cal_defaults'];
    $num_hours = $_GET['nh'];
    $ajax_url = replace_uri_var($ajax_url, 'nh', $num_hours);
    $cal_params->user_level    = 0;
    $cal_params->hide_controls = 1;
    $cal_params->hide_apts = 1;
    $cal_params->hide_res = 1;
    $cal_params->hide_weeks = 1;
    $cal_params->base_url = $ajax_url;
    $cal_params->ajax_target = $ajax_target;             
    $cal_params->max_months = 12;
    $cal_params->allow_day_view = false;
    $cal_params->allow_week_view = false;
    $cal_params->summary_view = true;

    //$cal_params->show_day_controls = true;

    // $cal_params->hide_past = 1;
    //dump($_SESSION['cal_filters']);
    $cal_filters['d'] = 0;
    $cal_filters['p'] = 1;
    $cal_filters['z'] = 0.9;
    $cal_filters['f'] = 0;
    $cal_filters['t'] = $type;
    $cal_filters['apt_list'] = $id;

    $cal_filters['sh'] = $aptObj->start_hour ? hm2dectime($aptObj->start_hour) : 8;
    $cal_filters['eh'] = $aptObj->end_hour ? hm2dectime($aptObj->end_hour) - $num_hours + 0.5 : 24;
    $cal_filters['nh'] = $num_hours;
    // echo("eh=" . hm2dectime($aptObj->end_hour). ' - '.$num_hours."<br>");
    // dump($cal_filters);
    // $cal_title = "Click on an available date";
    $cal_params->filters = $cal_filters;

    //dump($cal_params);
    $cal_params->cell_width = 21; // for property_calendar( for LCC, this is set elsewhere, but where ? 
    
    $calendar = calendar($cal_title, $cal_params);
    $content = html_div($calendar, 'cal_picker', $style);
    //dump($_SESSION['cal_params']);

    
    $_SESSION['cal_params'] = $org_params;

    break;

  case 3:    
    $start_time = $_GET['bt']; // begin time
    $num_hours = $_GET['nh'];
    $num_avail = $_GET['na'];
    $avail_apts = $_GET['apt_id'];
    $date = get_sql_date($_GET['d'], $_GET['m'], $_GET['y']);
    $date_str = sql2human($date);
    $time_str = dectime2hm($start_time).' - '.dectime2hm($start_time + $num_hours); 

    $in = "$date ".dectime2hm($start_time, true, true);
    $out = "$date ".dectime2hm($start_time + $num_hours, true, true);
    //echo("in=$in out=$out<br>");
    //$text = "Please choose the number of $type_name you want 
    //$title .= phrase('final_step', CAPITALIZE);

    $max_guests = $aptObj->max_guests;
    $messages[] = "The maximum number of people per $type_name is $max_guests.";
    if($num_avail == 1) $messages[] = "Only one $type_name is available for the chosen time and date. If you need more, please go back and choose another date/time.";
    $title .= "Please choose the number of $type_name_plural you would like to book on $date_str from $time_str.";
    $form = html_b("Number of $type_name_plural: "). select_from_array('nu', num_array(1, $num_avail, 1), $_GET['nu']);
    $submit_text = phrase('next', CAPITALIZE).' => ';
    $form .= form_input('submit', 'submit', $submit_text);
    $aid = $_SESSION['aid'] = $_SESSION['aff_id'] = $_GET['aid'];    
    $action_url = site_pagetype_link($_SESSION['siteObj'], PAGE_BOOK);
    //$from .= "action=$action_url";
    // $messages[] = "Available apts: $avail_apts";
    $hidden_fields = array('t' => $aptObj->type, 'id' => $avail_apts, 'in' => $in, 'out' => $out);
    $form .= array2form($hidden_fields);

    $content = html_form($form, 'cal_picker_form', $action_url, 'POST');
    // dump($_GET);
    //$title .= phrase('', CAPITALIZE);;
    //$output = rate_array_chooser($aptObj); 
    break;
  default:
    die("calendar_picker: unknown step $step");
    break;                                                    
  }
  //$output .= "Step = $step";
  if($step > 1) {
    $prev_step = $step - 1;
    $ajax_url = replace_uri_var($ajax_url, 'st', $prev_step);
    $js = ajax_load_onclick($ajax_target, $ajax_url, 'init_property_calendar();');                       
    $back_link = html_link('#', phrase('back', CAPITALIZE), array('js' => $js));
    $content .= html_break().$back_link;
  }
  $output = html_h2($title).html_break();
  if(count($messages)) $output .= print_messages($messages);
  if(count($errors)) $output .= print_errors($errors);
  $output .= $debug;  
  $output .= $content;  
  //$style = " style='height:260px;border:1px solid white'";

  return html_div($output, '', 'id="calendar_picker"'.$style);
}

// displays possible rental periods/prices for a property with a rate_array
function rate_array_chooser($aptObj) {
  $type = $aptObj->type;
  $currency = $aptObj->currency;
  $unit = $aptObj->rate_array_unit;
  $arrayStr = $aptObj->rate_array;
  $currency_symbol = get_currency_symbol($currency, true);
  $array = filters2array($arrayStr); 
  $next_step = 2;
  foreach($array as $period => $rate) {
    $unit_str_single = phrase($unit);
    $unit_str = $period == 1 ? $unit_str_single : phrase($unit.'s');
    $per_unit = formatFloat($rate / $period, FORMAT_ALWAYS, $currency_symbol);
    $str = "$period $unit_str: ".formatFloat($rate, FORMAT_IF_NEEDED, $currency_symbol)." ($per_unit / $unit_str_single)";
    //$url = replace_uri_var('', 'st', $next_step);
    //$url = replace_uri_var($url, 'nh', $period);
    $ajax_url = "/home/app_data.php?ac=cal-picker&st=$next_step&type=".$aptObj->type."&id=".$aptObj->id."&aid=".$_SESSION['aid']."&nh=".$period;  
    $ajax_target = 'calendar_picker';    
    $js = ajax_load_onclick($ajax_target, $ajax_url, 'init_property_calendar();');                       
    $li[] = html_link('#', $str, array('js' => $js));
  }
  return html_ul($li);
}

function property_sort($siteObj) {  
  return $siteObj->property_sort ?: 'priority DESC, book_direct DESC, created DESC';
}

function property_edit_link($type, $id, $link_only=false, $ref_name='', $ref_link='') {
  $siteObj = $_SESSION['siteObj'];
  if(!auth_action($type, 'edit', $id)) return '';  
  if($link = site_pagetype_link($siteObj, PAGE_EDIT_PROPERTY, $_SESSION['language'], true)) $link = rtrim($link, '/')."/$id/"; //"/?id=$id&amp;type=$type";
  else return '';
  return $link_only ? $link : arrow_link(phrase('edit', CAPITALIZE), $link, '', false, array('target' => 'edit_property'));
}
                  
function property_add_link($type, $city_id='', $link_only=false) {
  $siteObj = $_SESSION['siteObj'];
  if(!auth_action($type, 'add')) return ''; 
  if($link = get_page_link(PAGE_ADD_PROPERTY)) $link .= "?type=$type";
  else return '';
  if($city_id) $link .= "&amp;city_id=$city_id";
  return $link_only ? $link : arrow_link(phrase('add', CAPITALIZE).' '.property_type_name($type), $link);
}

function price_from_to($obj) {
  $price = '';
  if($obj->price_from > 0 || $obj->price_to > 0) {
    if($obj->price_from > 0 && $obj->price_to > 0 && $obj->price_from != $obj->price_to) { // show range
      $price = print_price($obj, 'price_from')." - ".print_price($obj, 'price_to');
    } else if($obj->price_from > 0) { // show from
      $price = print_price($obj, 'price_from');
    } else if($obj->price_to > 0) {  // show to
      $price = print_price($obj, 'price_to');
    }
  }
  return $price;
}

// prev/next/search buttons on property details page when part of search result
/** deprecated */
function property_pager($apt, $siteObj) {
  if(!isset($_GET['sr'])) return ''; // not a search
  $back_link = '';                                      
  $page_link = site_page_link($siteObj, $_SESSION['pageObj']);
  $qs = "?".get_query_string(); // remove uri var needs ? or & in front
  $back_qs = remove_uri_var($qs, 'sr');            
  
  $search_link = html_link($page_link.$back_qs."&search=1", html_span(phrase('search', CAPITALIZE), 'zocial fontello icon-search air-blue-back'));
  if($apt_list = $_GET['id_list']) {
    $id = $apt->id;
    $aptIdArray = explode(',', $apt_list);
    $this_index = array_search($apt->id, $aptIdArray);
    $prev_index = $this_index > 0 ? $this_index - 1 : -1;
    $next_index = $this_index < count($aptIdArray) - 1 ? $this_index + 1 : -1;
    $prev_class = $prev_index > -1 ? "previous" : "previous disabled";
    $next_class = $next_index > -1 ? "next" : "next disabled";
    $prev_class = $prev_index > -1 ? "" : "disabled";
    $next_class = $next_index > -1 ? "" : "disabled";
    $prev_link = html_link($prev_index > -1 ? property_link($siteObj, $aptIdArray[$prev_index], array('qs' => $qs)) : '#', "&larr; ".phrase('previous', CAPITALIZE));
    $next_link = html_link($next_index > -1 ? property_link($siteObj, $aptIdArray[$next_index], array('qs' => $qs)) : '#', phrase('next', CAPITALIZE)." &rarr;");
  }
  
  $output = "
  <ul class=\"pager\">
    <li class=\"\">$search_link</li>
    <li class=\"$prev_class\">$prev_link</li>
    <li class=\"$next_class\">$next_link</li>
  </ul>
  ";

  if(0) {
    $prev_id = $prev_index > -1 ? $aptIdArray[$prev_index] : 'N/A';
    $next_id = $next_index > -1 ? $aptIdArray[$next_index] : 'N/A';
    $debug = "list=$apt_list id=$id index=$this_index prev=$prev_index ($prev_id) next=$next_index ($next_id)<br>";
    $debug .= "qs=$qs<br>";    
    $debug .= "pl=$page_link<br>";    
    $output .= $debug;
  }
  
  return $output;

}


// uploader for jquery file upload
function property_uploader($ptype, $aptObj_or_id, $options = array()) {
  global $u_id, $site_id,$u_level;
  $aptObj = any2obj($ptype, $aptObj_or_id);
  $apt_id = $aptObj->id;
  
  $class_attrs = get_object_attributes($ptype, false);
  $upload_options = $class_attrs['media'][ATT_OPTIONS];
  if($upload_options && !is_array($upload_options)) $upload_options = json_decode($upload_options, true); /** we now use json in attribute definition */ 
    
  $upload_options = array_extend($upload_options, $options);
  
  //$upload_options['minimum_size'] = array(1080, 1920); // either short side >= 1080 or long side >= 1920

  $upload_options['minimum_size'] = 640; // either short side >= 1080 or long side >= 1920

  $fld = 'media';
  $val = $aptObj->media;
     
  $mediaData['user_id'] = $u_id ?: $aptObj->user_id;
  $mediaData['parent_id'] = $apt_id;
  $mediaData['parent_type'] = $ptype;
  $mediaData['parent_field'] = 'media';
  
  $upload_options['download_template'] = $options['download_template'] ?: "";
  
  $upload_options['media_data'] = $mediaData;
  $upload_options['data-value_list'] = $val;
  $upload_options['show_file_list'] = false;
  $upload_options['title'] = "Click or drag to upload property images";
  
  $edit_url = "/admin/app_data.php?oper=save&site_id=$site_id&user_level=$u_level&user_id=$u_id&obj_type=media"; // id will be appended
  $upload_options['edit_url'] = $edit_url;
  //$upload_options['edit_target'] = '#edit-rental-image';
  
  $handler = isset($options['handler']) ? $options['handler'] : '';
  if($upload_options['handler'] = $handler) {
    $handler_options = array();
    $upload_options['handler_options'] = $handler_options;
  }

  $uploader = jquery_file_uploader("apt-$apt_id-$fld", $val, $upload_options);

  //$edit_dialog = html_div("<br>Edit dialog<br>", "border", array('id' => 'edit-rental-image'));
  //$uploader .= $edit_dialog;
  
  return $uploader;
}


/* New functions rental_ */

// return array assoc. of property facilities order by group 
function rental_amenities($siteObj, $aptObj=null, $options = array()) {
  $search = array();
  
  $search['active'] = 1; 
  if($siteObj->golf_site) $search['golf_rental'] = 1;
  else $search['serviced'] = 1;
  if($options['searchable']) $search['searchable'] = 1;
  
  //array("serviced" => 1, "searchable" =>1)
  $facs = find_objects("facility", $search, array("sort" => 'group_name, name'));
  $facArray = array();
  $pfacs = $aptObj ? explode(',', $aptObj->facilities) : array();
  foreach($facs as &$fac) {
    $fac->value = in_array($fac->id, $pfacs) ? 1 : 0;
    $facArray[$fac->group_name][] = $fac;
  }
  return $facArray;
}

function rental_facility_icon($id) {
  /* im = icomoon 
     fa = font awesome 
     fi = foundation icons: http://test.hitabrick.com/afr/bower/foundation-icon-fonts/index.html
  */
  
  $default = "fa fa-check-square-o"; /* return checkbox */
  $icon_map = array(
    100 =>'im im-climate-control',
    140 =>'im im-terrace',
    150 =>'im im-bathtub',
    190 =>'im im-lines',
    134 =>'im im-children',
    230 =>'im im-restaurant',
    133 =>'im im-elder',
    
    
    260 =>'fi fi-elevator', /* foundation */
    310 =>'fa fa-fire', /* new */

    311 =>'fa fa-fire-extinguisher', /* new */
    
    300 =>'im im-fitness',
    360 =>'im im-patio',
    
    390 =>'', /* hair dryer */
    
    170 =>'im im-wi-fi',  /* new */
    380 =>'',
    
    420 =>'im im-spa',
    905 =>'im im-kitchen',  
    
    470 =>'', /* microwave oven */
    800 =>'fi fi-mountains', /* mountain view */
    480 =>'fi fi-no-smoking', /* no smoking */
    490 =>'', /* oven */
    
    500 =>'im im-parking',
    130 =>'im im-dog',
    
    510 =>'im im-phone', /* new */
    600 =>'', /* sauna */
    
    621 =>'im im-bed',
    790 =>'im im-sun-lines',
    132 =>'im im-smoking',
    690 =>'im im-pool',
    710 =>'im im-tv',
    730 =>'im im-washing-machine',
    760 =>'im im-wheel-chair',  
    780 =>'im im-wi-fi',
    
    // not active
    590 =>'im im-satellite',
    400 =>'im im-thermometer',
    650 =>'im im-stereo',
    151 =>'im im-electric',
    110 =>'im im-bus',
    460 =>'im im-air',
    
    /* icomoon not in use
    000 =>'im im-fm',
    000 =>'im im-shift-auto',
    000 =>'im im-lock',
    000 =>'im im-casino',
    000 =>'im im-diesel',                     
    000 =>'im im-car-doors',
    000 =>'im im-soundproof',
    000 =>'im im-meet',
    000 =>'im im-width',
    000 =>'im im-shift',
    000 =>'im im-bed',
    000 =>'im im-car-window',
    000 =>'im im-plane',
    000 =>'im im-bar',
    000 =>'im im-car-wheel',
    000 =>'im im-business-person',
    000 =>'im im-driver',
    000 =>'im im-icon_1041',
    000 =>'im im-shower',
    000 =>'im im-sunrise',
    000 =>'im im-sun',
    000 =>'im im-moon',
    000 =>'im im-sun3',
    000 =>'im im-windy',
    000 =>'im im-wind',
    000 =>'im im-snowflake',
    000 =>'im im-cloudy',
    000 =>'im im-cloudy-moon',
    000 =>'im im-sun-lines',
    000 =>'im im-moon-lines',
    000 =>'im im-cloud-lines',
    000 =>'im im-cloud',
    000 =>'im im-cloud-lightning',
    000 =>'im im-lightning',
    000 =>'im im-rainy',
    000 =>'im im-rain',
    000 =>'im im-windy-cloud',
    000 =>'im im-windy-cloud-rain',
    000 =>'im im-snowy',
    000 =>'im im-snowy-cloud',
    000 =>'im im-snow-cloud',
    000 =>'im im-snow-cloud-2',
    000 =>'im im-cloud-2',
    000 =>'im im-cloud-lightning-2',
    000 =>'im im-lightning-2',
    000 =>'im im-sun-fill',
    000 =>'im im-moon-fill',
    000 =>'im im-cloudy-fill',
    000 =>'im im-cloudy-moon-fill',
    000 =>'im im-cloud-fill',
    000 =>'im im-cloud-lightning-fill',
    000 =>'im im-rainy-fill',
    000 =>'im im-rain-fill',
    000 =>'im im-windy-cloud-fill',
    000 =>'im im-windy-cloud-rain-fill',
    000 =>'im im-snowy-cloud-fill',
    000 =>'im im-snow-cloud-fill-2',
    000 =>'im im-cloud-fill-2',
    000 =>'im im-cloud-lightning-fill-2',
    000 =>'im im-compass',
    000 =>'im im-none',
    000 =>'im im-celsius',
    000 =>'im im-fahrenheit'
    */
    );
  return $icon_map[$id];
}

// boxes with filters that can be clicked away  
function rental_search_filters($vars, $optons = array()) {
  //$boxOptions = array("class" => "");
  $filters = array();
  $vars = (array) $vars;
  //dump($vars);  
  // dates
  if($vars['in'] && $vars['out']) {
    $date_format = DATE_FORMAT;
    $data = array("in" => '', "in_disp" => '', "out" => '', "out_disp" => '');
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('dates', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

  // guests
  if($vars['ng'] > 1) {        
    $data = array("ng" => 1);
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('guests', CAPITALIZE), 'data' => $data, 'link' => $link);
  }  

  // rental type
  if($vars['pt']) {        
    $data = array("pt" => '');
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('type', CAPITALIZE), 'data' => $data, 'link' => $link);
  }  

  // faciliites
  if($vars['fac']) {        
    $data = array("fac" => '');
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('amenities', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

    // bedrooms
  if($vars['nr'] > 1) {        
    $data = array("nr" => 1);
    $link = "?".array2qs(array_merge($vars, $data));
    //$filters[] = html_link($link, "Bedrooms", array("class" => "form-resubmit", "data-data" => json_encode($data), "data-target" => "#rental_search_form"));
    $filters[] = array('name' => phrase('bedrooms', CAPITALIZE), 'data' => $data, 'link' => $link);
    
  }  

  // bathrooms
  if($vars['nb'] > 1) {        
    $data = array("nb" => 1);
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('bathrooms', CAPITALIZE), 'data' => $data, 'link' => $link);
  }  

  // price
  if($vars['rate_day']) {        
    $data = array("rate_day" => '');
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('price', CAPITALIZE), 'data' => $data, 'link' => $link, 'class' => 'green-border');
  }  

  // size
  if($vars['sz'] > 1) {
    $data = array("sz" => 1);
    $link = "?".array2qs(array_merge($vars, $data));
    $filters[] = array('name' => phrase('size', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }  
  
  return $filters; 
  
  // begin output
  $output = '';
  if(count($filters)) $output .= implode(html_break(), $filters);        
  //$output .= dump($_GET, true);
  return $output;
}

// tips when search result is empty  
function rental_search_tips($vars, $optons = array()) {
  $vars = (array) $vars;
  $center = $vars['center'];
  $bounds = $vars['bounds'];
  
  $unset = array('location_lat','location_long','location_bounds'); // not in use
  foreach($unset as $f) unset($vars[$f]); 
  $tips = array();
  
  // widen geographical area (increase radius)
  if($bounds) {
    $boundsObj = bounds_json2obj($bounds);
    $radius = bounds2radius($boundsObj);
    
    if($radius < 200*1000) { // 100 km  
      $locObj = bounds2loc($boundsObj);
      $newBoundsObj = radius2bounds($locObj, $radius * 2); // four times radius
      
      $newBounds = bounds_obj2json($newBoundsObj);
      $data = array("bounds" => $newBounds);
      $link = "?".array2qs(array_merge($vars, $data));
      $tips[] = array('name' => phrase('try_a_bigger_area', CAPITALIZE), 'data' => $data, 'link' => $link, 'class' => 'map-zoom-out');
    }
  }

  // remove faciliites
  if($vars['fac']) {        
    $data = array("fac" => '');
    $link = "?".array2qs(array_merge($vars, $data));

    $tips[] = array('name' => phrase('try_without_amenities', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }

  // reduce number of guests
  if($vars['ng'] > 4) {        
    $data = array("ng" => 4);
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_fewer_people', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }
  
  // lengthen stay
  $in = $vars['in'];
  $out = $vars['out'];
  $nd = $in && $out ? period_diff($in, $out, 'd') : 0;  
  if($nd >= 1 && $nd < 3) {
    $date_format = DATE_FORMAT;
    $out = sql_add_days($in, 3);
    $data = array("out" => $out, "out_disp" => date($date_format, strtotime($out)));
    $link = "?".array2qs(array_merge($vars, $data));
    $tips[] = array('name' => phrase('try_longer_stay', CAPITALIZE), 'data' => $data, 'link' => $link);   
  }
  
  return $tips;  
}

/** function to print fac icons */
/** valid categories: golf (course), golf_rental, serviced, non_serviced, boat */
function test_facs($category) {  
  $facAr = $aptObj->facilities ? explode(',', $aptObj->facilities) : array();
  $facs = find_objects('facility', array($category => 1));
  $output .= '<ul class="booking-item-features booking-item-features-expand-inline  mb30 clearfix">';
  foreach($facs as $fac) {
    $icon = rental_facility_icon($fac->id) ?: "fa fa-check";
    $output .= '<li class="col-xs-6 col-sm-4 col-md-6 col-lg-4"><i class="'.$icon.'"></i><span class="booking-item-feature-title">'.$fac->name.'('.$icon.') '.'</span></li>';
  }

  $output .= '</ul>';
  return $output;
}

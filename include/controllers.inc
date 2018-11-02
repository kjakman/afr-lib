<?php

function controller_mytest($app, $options = []) {
  $scope = $app->page->scope;
  $scope->template = 'test-page.html';
  $scope->foo = "bar";
  return;
}


/** not used by any site atm */
function controller_home($app, $options = []) {
  return;
}

function controller_minisite($app, $options = []) {
  $__header = $app->header;
  $u_id = $app->user_id;
  $pageObj = $app->page;
  $scope = $app->page->scope;
  $msObj = $scope->msObj = $app->minisite;
  if($css = $msObj->css) $__header->styles[] = "/css/minisites/{$msObj->url}/{$css}?v={$app->version}";
  
  $se_title = $se_description = '';
  
  $scope->minisite = $minisite = $msObj->url;
  $scope->basepath = $msObj->internal ? "/{$minisite}" : '';    
  $scope->minisite_name = $msObj->name;
  $scope->minisite_type = $type = $msObj->type;
  
  $user_id = $msObj->user_id;
  if(!$user_id) die("Minisite: missing user");
  
  //die("controller_minisite: user: {$user_id}");
  /** which page */
  $path = $pageObj->path;
  $parts = array_filter(explode('/', trim($path,'/')));
  $page = $tpl = pick_first(strtolower($parts[0]), 'home');
  $slug = count($parts) > 1 ? strtolower($parts[1]) : '';
  $obj_id = 0;
  if(strstr($slug, '~')) {
    list($name, $obj_id) = explode('~', $slug);
  }
  $subview = count($parts) > 2 ? strtolower($parts[2]) : '';
  
  if($page == 'exhibitions' && count($parts) > 1) $subview = strtolower($parts[1]);
  
  $scope->artworks = [];
  $scope->user_id = $user_id;
  $scope->user = $user = get_user($user_id);
  $scope->user_display_name = $display_name = user_display_name($user);
  $title = '';
  
  if(count($parts) > 3) return controller_error($app, "{$path} not found");
   
  if($page == 'home') { /** for now, use first exhibition */
    $gallery = '0';
    $artfair = '0';
    $others = '0';

    $scope->mObj = $mObj = find_object('minisite', ['user_id' => $user_id, 'active' => 1]);
    $scope->homeObj = $homeObj = find_object('minisite_home_page', ['user_id' => $user_id, 'active' => 1]);
    $scope->artist_Objs = $artist_Obj = find_object('minisite_artist_page', ['user_id' => $user_id, 'active' => 1]);
    $scope->exhibition_Obj = $exhibitionpage = find_object('minisite_exhibition_page', ['user_id' => $user_id, 'active' => 1]);

    $scope->timings = $timings = find_objects('opening_hours', ['user_id' => $user_id], ['sort' => 'id asc']);
    $scope->profileObj = $profileObj = find_objects('user_profile', ['user_id' => $user_id]);

    if($homeObj->show_gallery)
      $gallery = '10';
    if($homeObj->show_artfair)
      $artfair = '20';
    if($homeObj->show_others)
      $others = '30'; 
    $today = today();
    
    $search = ['user_id' => $user_id, 'type' => COLLECTION_TYPE_EXHIBITION, 'active' => 1, 'subtype' => [$gallery,$artfair,$others] ];
    $search['start_date'] = ['<=', $today];
    $search['end_date'] = ['>=', $today];

    $exhibitionObjs = find_objects('media_collection', $search, ['sort' => 'start_date desc']) ?: [];
    $count  = count($exhibitionObjs);

    if ($homeObj->layout == '1'){
        
      $tpl = $page = 'exhibition';

      if($count) {
        $title = 'Current Exhibition';
      } else { // no current exhibition
        
        // try future
        unset($search['start_date']);
        unset($search['end_date']);
        $search['start_date'] = ['>', $today];
        $exhibitionObjs = find_objects('media_collection', $search, ['sort' => 'start_date desc', 'limit' => 1]) ?: [];
        if($count  = count($exhibitionObjs)) {
          $title = 'Next Exhibition';
        } else {          
          // try past
          unset($search['start_date']);
          unset($search['end_date']);
          $search['end_date'] = ['<', $today];
          $exhibitionObjs = find_objects('media_collection', $search, ['sort' => 'start_date desc', 'limit' => 1]) ?: [];            
          $count  = count($exhibitionObjs);
          if($count) {
            $title = 'Latest Exhibition';
          } else {
            if(!$count) die("no exhibitions");
          }
        }
        $exhibitionObj = $exhibitionObjs[0];
      }

      $exhibitionObj = $exhibitionObjs[0];
      $slug = slugify($exhibitionObj->title);        
    }

    if ($homeObj->layout == '2'){ //Timeline View

      if($homeObj->show_recent == '1'){
        unset($search['start_date']);
        unset($search['end_date']);
        $exhibitionhome = find_objects('media_collection', $search, ['sort' => 'start_date desc']) ?: []; 
      }
      else{
        unset($search['start_date']);
        unset($search['end_date']);
        $exhibitionhome = find_objects('media_collection', $search, ['sort' => 'start_date asc']) ?: []; 
      }

        foreach($exhibitionhome as $exhibitionObj) {
          $current = "";

          if($exhibitionObj->start_date && $exhibitionObj->end_date) {
            if($exhibitionObj->start_date <= $today && $exhibitionObj->end_date >= $today){
              $current = "current";
              $exhibitionObj->current = $current; 
              $exhibitionObj->slug = slugify($exhibitionObj->title);
            }
            elseif($exhibitionObj->end_date < $today){
              $current = "past";
              $exhibitionObj->current = $current; 
              $exhibitionObj->slug = slugify($exhibitionObj->title);
              $exhibitionObj->past_exists = "1";
            }
            elseif($exhibitionObj->start_date > $today){
              $current = "future";
              $exhibitionObj->current = $current; 
              $exhibitionObj->slug = slugify($exhibitionObj->title);
              $exhibitionObj->future_exists = "1";
            } 

            $filter_exhibitions[] = $exhibitionObj;
          }
        }
        $scope->filter_exhibitions = $filter_exhibitions;

        /* To view latest exhibition */
        $latestexhibition = find_objects('media_collection', $search, ['sort' => 'start_date desc', 'limit' => 1]) ?: []; 
        $scope->latest_exhibitions = $latestexhibition;
      }
      else{
        print_log("No templates chosen", 'minisite', LOG_LEVEL_TEST);
      }
    
  }
  
  
  //dump($options);
  $artist = null;
  $artists = $artistAr = [];
  $data = [];
  $today = today();
  
  $exhibitions = $artists = [];
  
  print_log("type={$type} path: {$path} page:{$page} slug:{$slug} id: {$obj_id} name: {$name} sub:{$subview} options".dump($options,true), 'minisite', LOG_LEVEL_TEST);  
  
  $se_title = $minisite;
  $se_description = $minisite;
  
  switch($type) {
  case 'artist':
    $search = ['user_id' => $user_id, 'active' => 1, 'type' => COLLECTION_TYPE_EXHIBITION, 'subtype' => 40, 'show_website' => 1];
    $scope->artist = $artist = get_user($user_id);
    if(!$artist) return scope_error($app, "Artist {$slug} not found", '404');

    $artist->slug = slugify($uObj->display_name);
    $artist->collections = $exhibitionObjs = find_objects('media_collection', $search);
    //if($page == 'home') $tpl = $page = 'artist';
    
    
    /**
    $collectionObjs = find_objects('media_collection', ['user_id' => $user_id, 'active' => 1, 'type' => COLLECTION_TYPE_PROJECT]);    
    $collectionObj = $collectionObjs[0];
    $collection_count = count($collectionObjs);
    $exhibition_count = count($exhibitionObjs);
    print_log("user: {$user_id} $collection_count collections $exhibition_count id={$collectionObj->id} type={$collectionObj->type} title={$collectionObj->title}", 'minisite', LOG_LEVEL_TEST);

    $artist->collection = $collectionObj;
    $artist->collection_id = $collectionObj->id;
    $artist->title_image = $uObj->photo ?: collection_title_image($collectionObj, 'ig');
    $artist->collections = $exhibitionObjs;
    */
    
    
    //die("type={$type} path: {$path} page:{$page} slug:{$slug} sub:{$subview} options".dump($options,true));
    
    break;
    
  case 'gallery':
  default:
    
    if($page == 'artists' || $page == 'artist') {
      /** Artists - only do when needed */  
      $scope->artist_ids = $artist_ids = user_artist_list_display($user_id) ?: [];
      
      foreach($artist_ids as $artist_id) {
        $uObj = get_user($artist_id);    
        
        $colSearch = ['user_id' => $user_id, 'artist_id' => $uObj->id, 'active' => 1, 'type' => COLLECTION_TYPE_EXHIBITION, 'start_date' => ''];
        $collectionObj = find_object('media_collection', $colSearch);
        //dump($collectionObj);
        print_log("artist {$uObj->display_name} ({$artist_id}) id={$collectionObj->id} type={$collectionObj->type} title={$collectionObj->title}", 'debug', LOG_LEVEL_TEST);
        
        if(!$collectionObj) {
          //echo("{$uObj->display_name} has no projects<br>");
          continue; /** no work to dipslay */
        }
        $uObj->collection = $collectionObj;
        $uObj->collection_id = $collectionObj->id;
        $uObj->slug = $this_slug = slugify($uObj->display_name);
        
        $uObj->title_image = $title_image = $uObj->photo ?: collection_title_image($collectionObj, 'ig');        
        //$uObj->title_image = collection_title_image($collectionObj, 'ig') ?: $uObj->photo; /** (David asked for this */

        print_log("{$this_slug} ({$artist_id}) u_photo: {$uObj->photo} title: {$title_image} collection: {$collectionObj->title} ({$collectionObj->id})", "minisite", LOG_LEVEL_TEST);
        
        $artistAr[] = $uObj;
        //$data[$uObj->slug] = ['id' => $uObj->id, 'name' => $uObj->display_name];
      }
      
      $artistAr = array_sort($artistAr, 'last_name');
      foreach($artistAr as $this_artist) $artists[$this_artist->slug] = $this_artist;        
      $scope->artists = $artists;
      //dump($artistAr);

      $scope->artist_Objs = $artist_Obj = find_object('minisite_artist_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->exhibition_Obj = $exhibitionpage = find_object('minisite_exhibition_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->mObj = $mObj = find_object('minisite', ['user_id' => $user_id, 'active' => 1]);
      $scope->timings = $timings = find_objects('opening_hours', ['user_id' => $user_id], ['sort' => 'id asc']);
    }
    
    if($page == 'exhibitions' || $page == 'exhibition' ) {
      if($page == 'exhibitions') {
        $when = $subview ?: 'currentandfuture';
      } else { // single exhibition, load all
        $when = 'all';
      }
      
      $today = today();
      
      /** Exhibitions - only do when needed  */

      /*Filter exhibition by type */
      $gallery = '0';
      $artfair = '0';
      $others = '0';
      $scope->homeObj = $homeObj = find_object('minisite_home_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->exhibition_Obj = $exhibitionpage = find_object('minisite_exhibition_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->artist_Objs = $artist_Obj = find_object('minisite_artist_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->mObj = $mObj = find_object('minisite', ['user_id' => $user_id, 'active' => 1]);
      $scope->timings = $timings = find_objects('opening_hours', ['user_id' => $user_id], ['sort' => 'id asc']);

      if($exhibitionpage->show_gallery)
        $gallery = '10';
      if($exhibitionpage->show_artfair)
        $artfair = '20';
      if($exhibitionpage->show_others)
        $others = '30'; 
  
      $search = ['user_id' => $user_id, 'type' => COLLECTION_TYPE_EXHIBITION, 'active' => 1, 'subtype' => [$gallery,$artfair,$others] ];
      switch($when) {
        case 'past':
          $search['end_date'] = ['<', $today];
          break;
        case 'current':

          $search['start_date'] = ['<=', $today];
          $search['end_date'] = ['>=', $today];      
          break;
        case 'future':
          
          $search['start_date'] = ['>', $today];
          break;
        case 'currentandfuture':
          
          $search['end_date'] = ['>=', $today];
          break;
        case 'all':
          
          break;
        default: // all or single
          break;
      }

      /** Filter by gallery, Art, Other events */
      if($exhibitionpage->show_recent == 1 )
        $exhibitionObjs = find_objects('media_collection', $search, ['sort' => 'start_date desc']) ?: [];
      else
        $exhibitionObjs = find_objects('media_collection', $search, ['sort' => 'start_date asc']) ?: [];

        $exhibition_ids = $exhibitions = [];
        foreach($exhibitionObjs as $exhibitionObj) {
          $exhibitionObj->slug = slugify($exhibitionObj->title);
          $current = "";
          if($exhibitionObj->start_date && $exhibitionObj->end_date) {
            if($exhibitionObj->end_date < $today) {
              $current = "past";
            } elseif($exhibitionObj->start_date > $today) {
              $current = "future";
            } else {
              $current = "current";
            }
          } else {
            continue; /** skip exhibitions w/o dates */
          }
          $exhibitionObj->current = $current;    
          $exhibitions[$exhibitionObj->slug] = $exhibitionObj;
          $exhibition_ids[] = $exhibitionObj->id;
        }

        $scope->exhibitions = $exhibitions; 
    }
         
    break;
  }
  
  /** todo: 404 if slug/subview not allowed or not found */
  
  switch($page) {
    case 'artist':
      //dump($artist);
      $artist = $artists[$slug];
      $scope->artist = $artist;
      if($collection_id = $artist->collection_id) {
        $jsonObj = gallery_get_json($collection_id);
        $scope->data = $jsonObj;
        $scope->collection_id = $collection_id;
        $search = ['media_collection_id' => $collection_id, 'active' => 1];
        $scope->artworks = $mapObjs = find_objects('media_collection_map', $search, ['sort' => 'rank']);
      }        
      
      $se_title = $artist->display_name;
      $se_description = $artist->description;
                          
      
      $scope->show_images = 1;
      $scope->show_solo = strlen(strip_blank($artist->gallery_solo_shows)) > 0 ? 1 : 0;
      $scope->show_group = strlen(strip_blank($artist->gallery_group_shows)) > 0 ? 1 : 0;
      $scope->show_press = strlen(strip_blank($artist->press)) > 0 ? 1 : 0;
      $scope->show_bio = $scope->show_solo || $scope->show_group || $scope->show_press ? 1 : 0;
      $scope->show_pub = 0; // strlen(strip_blank($artist->publications)) > 0;
      break;
    case 'artists':
      break;
    
    case 'exhibition':
      if(!$slug) return scope_error($app, "Missing slug", '404');
      $collectionObj = $obj_id ? get_object('media_collection', $obj_id) : $exhibitions[$slug];
      
      if(!$collectionObj) {
        print_log("no $slug in ".dump($exhibitions,true), 'minisite', LOG_LEVEL_TEST);
        return scope_error($app, "Exhibition {$slug} not found", '404');
      }
      
      $artist_names = collection_artist_list($collectionObj);
      //dump($artist_names);
      
      $se_title = "{$collectionObj->title}, {$artist_names}";
      $se_description = $collectionObj->description;
      
      
      
      $scope->collection = $collectionObj;
      $scope->collection_id = $collection_id = $collectionObj->id;
      if($loc_id = $collectionObj->location_id) {
        $locObj = get_object('location', $loc_id);
        $scope->collection->location = $locObj;
      }
  
      if($event_id = $collectionObj->event_id) {
        $eventObj = get_object('event', $event_id);
        if(!$eventObj) die("event $event_id not found");
        $scope->collection->event = $eventObj;
      }      
      $search = ['media_collection_id' => $collection_id, 'active' => 1];
      $scope->artworks = $mapObjs = find_objects('media_collection_map', $search, ['sort' => 'rank']);
      break;
    case 'work':
      
      break;
    case 'exhibitions':
      break;
    case 'contact':
      $scope->receiver_name = $display_name;
      $scope->receiver_email = $user->email_address;
      $scope->mObj = $mObj = find_object('minisite', ['user_id' => $user_id, 'active' => 1]); 
      $scope->homeObj = $homeObj = find_object('minisite_home_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->exhibition_Obj = $exhibitionpage = find_object('minisite_exhibition_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->artist_Objs = $artist_Obj = find_object('minisite_artist_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->contact_Obj = $contactObj = find_object('minisite_home_page', ['user_id' => $user_id, 'active' => 1]);
      $scope->timings = $timings = find_objects('opening_hours', ['user_id' => $user_id], ['sort' => 'id asc']);
      
      break;
    case 'about':
      break;
    case 'news':
      break;
    case 'private':
      break;
    case 'home': /** not fully implemented yet - assumes existence of custom page for this artist */
      //$tpl = "{$minisite}/{$tpl}";
      //die("page={$page} tpl={$tpl}");
      break;
    default:
      return scope_error($app, "Page {$page} not found", '404');
      break;
  }
  
  
  //dump($_SERVER);
  $docroot = docroot(false);
  $custom_filename = "{$docroot}/user/minisite/{$minisite}/pages/{$page}/default.html";
  if(is_file($custom_filename)) {
    //print_log("page=$page path={$path} slug={$slug} subview={$subview} tpl={$pageObj->template}".dump($parts,true), 'minisite', LOG_LEVEL_TEST);
    list($custom_header, $custom_content) = site_parse_pagefile($custom_filename);
    $scope->headers = $custom_header;
    $scope->custom_content = $custom_content;

    print_log("Found custom file {$custom_filename} content={$custom_content}", 'minisite', LOG_LEVEL_TEST);

  }
  
  $pageObj->template = "minisites/{$tpl}";
  $pageObj->name = $page;
  $scope->page = $page;
  $scope->slug = $slug;
  $scope->subview = $subview;
  $scope->title = $title;

  $keywords = implode(',', array_filter([$minisite, $display_name, $page, $slug, $subview, $site->name, $page->name, $type, $se_title]));
  
  $app->page->se_title = htmlspecialchars($se_title);
  $app->page->se_keywords = htmlspecialchars($keywords);
  $app->page->se_description = htmlspecialchars($se_description);
    
  print_log("page=$page path={$path} slug={$slug} subview={$subview} tpl={$pageObj->template}".dump($parts,true), 'minisite', LOG_LEVEL_TEST);
  return;
}


function controller_login($app, $options = []) {
  global $u_id;
  $scope = $app->page->scope;
  $scope->provider_keys = $pk = $app->scope->provider_keys; // find_objects('provider_key', array('site_id' => $siteObj->id, 'active' => 1), array("field" => "provider_id"));
  $ref = $_GET['ref'];
  if($u_id && $ref) {
    $scope->redirect = $ref; 
  }
  return;
}

function controller_register($app, $options = []) {
  $scope = $app->page->scope;
  return;
}

function controller_generic($app, $options = []) {
  $scope = $app->page->scope;
  return;
}

function controller_calendar($app, $options = []) {
  global $u_id, $__CLIENT_ID, $__USER;
  $__header = $app->header; 
  $scope = $app->page->scope;
  $siteObj = $app->site;
  $app->page->hide_edit_menu = 1;
  $scope->hide_menu = 1;
  
  $ver = $app->version;
  
  $__header->scripts[] = "/afr/js/admincal.js?v=$ver";
  $__header->scripts[] = "/afr/js/admincal-dialog.js?v=$ver";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js";    
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";
  $__header->styles[] = "/afr/css/admincal.css?v=$ver";
  $__header->styles[] = "/afr/css/afr_gmenu.css?v=$ver";
  
  $cp = $options['cal_params'] ?: new stdClass;
  
  $ptype = $siteObj->property_type ?: 'shortstay';
  $host_ids = user_host_list($u_id);  
  $psearch = array('host_id' => $host_ids);

  
  $scope->show_inactive = $show_inactive = isset($_GET['si']) ? $_GET['si'] : 0;
  if(!$show_inactive) $psearch['active'] = 1;
          
  //$scope->rentals = find_objects($ptype, $psearch);
  $rentals = find_objects($ptype, $psearch, ['fields' => ['id', 'name', 'type', 'site_id', 'active'], 'sort' => 'address_street,address_number,address_suffix']);
  foreach($rentals as $rental) {
    $rental->link = rental_link($rental, ['full' => 1, 'live' => 1]);
    //print_log("link for {$rental->name}: {$rental->link}", 'calendar', LOG_LEVEL_TEST);
  }
  array_unshift($rentals, ['id' => 0, 'name' => "?"]);
  $scope->rentals = $rentals;
  $scope->sites = find_objects('site', ['client_id' => $__CLIENT_ID, 'property_type' => $ptype, 'active' => 1], ['fields' => ['site.id', 'site.name', 'site.shortname']]);
  $scope->legal_entities = find_objects('legal_entity', ['client_id' => $__CLIENT_ID, 'active' => 1], ['fields' => ['legal_entity.id', 'legal_entity.name', 'legal_entity.shortname']]);
  $scope->agents = find_objects('agent', ['client_id' => $__CLIENT_ID, 'active' => 1], ['fields' => ['agent.id', 'agent.name'], 'sort' => 'agent.name']);
  $scope->clients = find_objects('org', ['client_id' => $__CLIENT_ID, 'active' => 1], ['fields' => ['org.id', 'org.name'], 'sort' => 'org.name']);
  $scope->host_ids = $host_ids;
                     
  $start = date('Y-m-01');
  $scope->data = ['start' => $_GET['start'] ?: $start, 'mode' => 'calendar', 'period' => $_GET['p'] ?: 1, 
                  'rentals' => $rentals, 'view' => 'month', 'host_ids' => $host_ids, 'show_inactive' => 0,
                  'user_level' => $__USER->user_level, 'user_id' => $__USER->id]; /** new cal params */
  return;                                              
}

function controller_search($app, $options = []) {
  $siteObj = $app->site;
  $scope = $app->page->scope;
  $results = array();
  if($term = $_GET['s']) {
        
    // page content
    if($result = search_result($siteObj, 'page_content', $term, array('title', 'content'), 'id', $options))
      $results = array_merge($results, $result);

    // articles / blogs
    if($result = search_result($siteObj, 'article', $term, array('title', 'content'), 'id', $options))
      $results = array_merge($results, $result);

    // comments
    if($result = search_result($siteObj, 'comment', $term, array('title', 'content'), 'id', $options))
      $results = array_merge($results, $result);

    // events
    if($result = search_result($siteObj, 'event', $term, array('name', 'notes'), 'id', $options))
      $results = array_merge($results, $result);
    
  }
  $scope->results = $results;
  //dump($scope->result);
  return;
}

function controller_site_edit($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  
  $scope = $app->page->scope;
  $ver = $app->version ?: date('md');

  $__header->scripts[] = "/afr/js/afr_cms_jstree.js?v=$ver";
  $__header->styles[] = "/afr/bower/jstree/dist/themes/default/style.min.css";
  $__header->scripts[] = "/afr/bower/jstree/dist/jstree.min.js";
    
  return;
}

/** generic controller for back-end, e.g site_edit. Called from ajax.php: cms-explore */
function controller_cms_explore($app, $options = []) {
  $siteObj = $app->site;
  $scope = $app->page->scope;
  $language = pick_first($scope->language, $app->language, $siteObj->default_language);
  $obj_type = $scope->obj_type;
  $obj_id = $scope->obj_id;
  $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : new stdClass;
  switch($obj_type) {
    case 'site_page': // add page_content for chosen language
      if($obj_id) {
        $obj = site_find_content($siteObj, $obj_id, $language); 
      } else { /** add page */
        $obj->id = $obj->obj_id = 0;
        $obj->name = $name = $scope->name;
        $obj->active = 1;
        $obj->show_menu = 1;
        $obj->parent_id = $parent_id = $scope->parent_id;
        $obj->user_level = 0;
        $safe_name = url_safe($name);
        $parent_path = $parent_id ? find_object('page_content', ['page_id' => $parent_id, 'language' => $language], 'path') : '';
        $obj->path = str_replace("//", "/", clean_path("{$parent_path}/{$safe_name}/"));
      }
      $obj->foo = 'bar';
      break;
    case 'site': // add site_content for chosen language
      break;
    default:
      break;
  }
  $scope->obj = $obj;
  return;
}


/** uses rental_search, object type is location */
function controller_location($app, $options = []) {
  return controller_geo_location($app, $options);
}

/** uses rental_search, object type is geo_city */
function controller_geo_location($app, $options = []) {
  global $u_id, $__LANG;
  $__header = $app->header; 
  $siteObj = $app->site;
  $min_radius = 2250;
  
  $scope = $app->page->scope; // $scope = new stdClass;
  $scope->property_type = $ptype = pick_first($siteObj->property_type, $app->property_type, 'shortstay');
  
  /** preprocessing - find bounds */
  $__header->styles[] = "/afr/css/afr-air.css";

  $slug = $app->slug;
  $obj_type = $scope->obj_type = $app->object_type;
  $obj_id = $scope->obj_id = $app->object_id;
  $obj = $locObj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : null;
  $scope->slug = $slug;
  $scope->obj = $obj;
  
  
  if($obj) {
    $center = loc2center($obj);
    $obj_center = json_encode($center);
    $obj_radius = $obj->location_radius;
    $bounds = any2bounds($obj->location_bounds, true) ?: [];  
    if($center && !$bounds) {
      if($obj->population > 0) {
        $radius = min(round($locObj->population / 10), 10000); // bigger radius for big cities, max 10km
      } else {
        $radius = DEFAULT_RADIUS;
      }
      $radius = max($radius, $min_radius); // bigger radius for big cities, min 3km
      
      $bounds = radius2bounds($obj, $radius, true);      
      print_log("calculated bounds {$bounds} radius={$radius}", 'property-search', LOG_LEVEL_TEST);
      
    }
    
    
    print_log("location set: slug={$slug} type={$obj_type} id={$obj_id} obj-center {$obj_center} radius={$obj_radius} obj-bounds={$obj->location_bounds} ", 'property-search', LOG_LEVEL_TEST);
    
    $pageObj = $app->page;
    $parent_type = $pageObj->page_type == PAGE_LOCATION ? 'location': 'geo_city';
    
    // get location description    
    $search = ['parent_type' => $parent_type, 'parent_id' => $obj->id, 'language' => $__LANG];
    if($descObj = find_object('object_description', $search)) {
      $scope->location->title = $descObj->title;
      $scope->location->description = $descObj->description;
      $scope->location->description_id = $descObj->id;
    } elseif($descObj = find_object('object_description', ['parent_type' => $parent_type, 'parent_id' => $obj->id])) {
      $descData = ['language' => $__LANG];
      foreach(['site_id', 'parent_type', 'parent_id', 'type', 'description'] as $f) $descData[$f] = $descObj->$f;
      list($desc_id, $errors) = add_object('object_description', $descData);
      if($desc_id) {
        $scope->location->title = $descObj->title;
        $scope->location->description = $descObj->description;
        $scope->location->description_id = $desc_id;
      }
      if($errors) $scope->error = $errors;
    }
    
  } elseif($slug) {
    list($status, $acc, $lat, $lng, $bounds) = google_geocode($slug);
    print_log("geocoding: slug={$slug} status={$status} lat={$lat} lng={$lng}", 'property-search', LOG_LEVEL_TEST);
    
    if($status==200) {
      $scope->http_code = "200"; //override error set in set_env.inc due to unknown slug
      $center = [$lat, $lng];
      $locObj = center2loc($center);
      $boundsObj = $bounds ? json_decode($bounds) : radius2bounds($locObj, DEFAULT_RADIUS);
      $bounds = any2bounds($boundsObj, true);
    }
  } else {
    return scope_error($app, "Neither location object nor slug", '404');
  }
  
  if($bounds) {
    $center = bounds2center($bounds);
    $loc = center2loc($center);
    $radius = bounds2radius($bounds);
    
    if($obj_radius > $min_radius) {
      print_log("Loc radius > min ($min_radius): $locObj->location_radius", 'property-search', LOG_LEVEL_TEST);
      $min_radius = $locObj->location_radius;
    }
      
    print_log("\n\nradius=$radius", 'property-search', LOG_LEVEL_TEST);
    if($radius < $min_radius) {
      //print_log("zooming out".dump($bounds,true), 'property-search', LOG_LEVEL_TEST);
      $bounds = radius2bounds($loc, $min_radius, true);
      $radius = bounds2radius($bounds);
      //print_log("new radius=$radius".dump($bounds,true), 'property-search', LOG_LEVEL_TEST);
      
    }
    
    $center_str = $scope->center = json_encode($center);
    $bounds_str = $scope->bounds = json_encode($bounds);
  } else {
    return scope_error($app, "Could not find location slug={$slug} name={$locObj->name}", '404');
  }
  
  print_log("OK: Found bounds: center={$center_str} bounds={$bounds_str} radius={$radius}", 'property-search', LOG_LEVEL_TEST);

  $options['bounds'] = $bounds_str;
  controller_rental_search($app, $options);  
  return;
}
  
function controller_rental_search($app, $options = []) {
  global $u_id, $__DEFAULT_BOUNDS;
  $__header = $app->header; 
  $siteObj = $app->site;

  $ver = $app->version ?: date('md');
  $__header->styles[] = "/afr/css/afr-air.css";

  $scope = $app->page->scope; // $scope = $options['scope'] ?: new stdClass;
  
  $scope->property_type = $ptype = pick_first($siteObj->property_type, $app->property_type, 'shortstay');
  $scope->amenities = rental_amenities($siteObj, null, array('searchable' => 1));

  print_log("controller_rental_search: stack:".caller_stack(), 'app', LOG_LEVEL_TEST);
  
  /** todo: */
  $scope->property_link = site_page_link($siteObj, PAGE_RENTAL_DETAIL);

  print_log("controller_rental_search: stack:".caller_stack(), 'app', LOG_LEVEL_TEST);

  $rs = $scope->rental_search;

  $isset = isset($scope->rentals) ? "yes" : "no";
  $rentals = (array) $scope->rentals;
  $count = count($rentals);
  
  print_log("controller_rental_search: rs={$rs} isset={$isset} count={$count}", 'rental-search', LOG_LEVEL_TEST);
  
  if($rs || $count) { /** new - search now happens in ajax.php: rental-search, returns json */
    $scope->loaded = 1;
    $preset = 1;
    $search = null;

    $sc = clone $scope;
    unset($sc->rentals);
    unset($sc->amenities);
    print_log("controller_rental_search: scope".dump($sc, true), 'app', LOG_LEVEL_TEST);
    
  } else {
    //print_log("no rentals: controller_rental_search: scope".dump($scope, true), 'rental-search', LOG_LEVEL_TEST);
    $scope->loaded = 1;
    $preset = 0;
    $search = $_GET;
    
    if($bounds = $options['bounds']) {
      $search['bounds'] = $bounds; // from controller_geo_location or controller_location
    } elseif($bounds = $search['bounds']) {
      //print_log("rental_search: expl. bounds:".dump($bounds,true), 'property-search', LOG_LEVEL_TEST);            
    } elseif($loc = $search['loc']) {
      $loc = urldecode($loc);
      list($status, $acc, $lat, $lng, $bounds) = google_geocode($loc);
      if($status==200) {
        $scope->http_code = "200"; //override error set in set_env.inc due to unknown slug
        $center = [$lat, $lng];
        $locObj = center2loc($center);
        $boundsObj = $bounds ? json_decode($bounds) : radius2bounds($locObj, DEFAULT_RADIUS);
        $bounds = any2bounds($boundsObj, true);        
        $search['bounds'] = json_encode($bounds);
      }
    }
    
    if(!$search['bounds']) $search['bounds'] = $siteObj->default_bounds ?: $__DEFAULT_BOUNDS;
    
    /** widen search by 100%, as map will show bounds + a little extra... */
    if(1) {
      $search_bounds_json = $search['bounds'];
      $search_bounds = json_decode($search['bounds']);
      $search_radius = bounds2radius($search_bounds);
  
      $search_center = bounds2center($search_bounds);
      $search_loc = center2loc($search_center);
      
      $factor = 2;
      $wide_radius = $search_radius * $factor;
      $wide_bounds = radius2bounds($search_loc, $wide_radius, true);
    
      $search['bounds'] = $wide_bounds_json = json_encode($wide_bounds);
      
      //print_log("Widened search from radius = $search_radius to $wide_radius", 'property-search', LOG_LEVEL_TEST);
      //print_log("Was: $search_bounds_json", 'property-search', LOG_LEVEL_TEST);
      //print_log("Now: $wide_bounds_json", 'property-search', LOG_LEVEL_TEST);
    }
    
    $result = rental_search($search);
    if($error = $result->error) $scope->error = $result->error;  
    
    $data = $result->data ?: [];
    foreach($data as $k=>$v) $scope->$k = $v;
    $rentals = $data->rentals;
    
    $scope->bounds = $search_bounds; /** reset to original */
  }

  $count = count($rentals);  
  if(count($rentals)) {
    if($search) $scope->filters = $filters = rental_search_filters($search);
  } else {
    $scope->tips = $tips = rental_search_tips($search);
  }
  
  $scope->s2 = $search;
  
  $center_str = $scope->center = json_encode($scope->center);
  $bounds_str = $scope->bounds = json_encode($scope->bounds);
  
  print_log("controller_rental_search v2: count=".count($rentals)." preset={$preset} center={$center} bounds={$bounds}", 'app', LOG_LEVEL_TEST);
  
  return;
}


function controller_rental_feedback($app, $options = []) {
  $scope = $app->page->scope;
  $obj_type = $scope->obj_type = $app->object_type;
  $obj_id = $scope->obj_id = $app->object_id;
  $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : null;

  $scope->feedback = $obj;
  return;  
}

function controller_sitemap($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $scope = $app->page->scope;
  $siteObj = $app->site;
  $tree = site_sitemap($siteObj, []);
  //$search = ['address_country' => 'PT']; // for testing
  if($ptype = $siteObj->property_type) {    
    $ptree = site_sitemap_property_tree($siteObj, ['type' => $ptype, 'search' => $search]);
    $tree = array_merge($tree, $ptree);
  }
  if($siteObj->golf_site) {
    $gtree = site_sitemap_golf_tree($siteObj, ['search' => $search]);
    $tree = array_merge($tree, $gtree);
  }
  $scope->sitemap_xml = $xml = site_sitemap_xml($siteObj, ["tree" => $tree]);
  //echo(form_text('foo', $xml));die();
  $xmlfile = docroot().'sitemap.xml';
  $len = write_file($xmlfile, $xml);
  
  $scope->sitemap = $tree;
  return;  
}

function controller_event_calendar($app, $options = []) {
  global $u_id, $u_level,$__USER_LEVELS;
  $__header = $app->header; 
  
  $ver = $app->version ?: date('md');
  $siteObj = $app->site;
  
  $scope = $app->page->scope;
  $__header->scripts[] = "/afr/bower/fullcalendar/dist/fullcalendar.js";
  $__header->scripts[] = "/afr/bower/geocomplete/jquery.geocomplete.min.js";
  $__header->styles[] = "/afr/bower/fullcalendar/dist/fullcalendar.css";

  $__header->scripts[] = "/afr/js/afr_gmap.js";
  $__header->scripts[] = "/afr/bower/bootstrap-confirmation2/bootstrap-confirmation.js";    
  $__header->scripts[] = "/afr/js/event-form.js?v={$ver}";    

  $__header->styles[] = "/afr/jquery/periodpicker/build/jquery.periodpicker.min.css";
  $__header->scripts[] = "/afr/jquery/periodpicker/build/jquery.periodpicker.full.min.js";
  
  $calOptions = array();
  $site_id = $scope->site_id = $siteObj->id;
  
  $scope->calendar_edit = $auth_edit = $u_level >= USER_LEVEL_EDITOR;
  $site_id = $siteObj->id;
  $scope->calendar_url = $url = ajax_operation_link('agenda-calendar-events', array('site_id' => $site_id, 'edit' => $auth_edit), array('public' => true));
  $calOptions = array("selectable" => true, "editable" => $auth_edit, "url" => $url, "site_id" => $site_id, "timeFormat" => "H:mm");
    
  if($in) $calOptions['defaultDate'] = $in;
  $scope->calendar_options = $calOptions;
  $scope->user_levels = $__USER_LEVELS;
  return;
}


/** add rental */
function controller_rental_add($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  
  $ver = $app->version;  
  $__header->scripts[] = "/afr/bower/geocomplete/jquery.geocomplete.min.js";
  $scope = $app->page->scope;
  $scope->currency = "EUR";
  $scope->host_ids = $host_ids = user_host_list($u_id) ?: array();
  
  $scope->rental_types = find_objects("hosting_property_type", array('active' => 1));
  $scope->room_types = find_objects("hosting_room_type", array('active' => 1));
    
  return;  
}

/** /rentals/ page = table view */
function controller_rentals($app, $options = []) {
  global $u_id, $u_level;
  $__header = $app->header; 
  $siteObj = $app->site;
  
  $__header->scripts[] = "/afr/bower/jquery-file-download/src/Scripts/jquery.fileDownload.js";
  $__header->scripts[] = "/afr/bower/table-export/jquery.base64.js";
  $__header->scripts[] = "/afr/bower/table-export/tableExport.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js"; /** todo: support other languages, use $app->locale  */
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/export/bootstrap-table-export.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/editable/bootstrap-table-editable.min.js";
  $__header->scripts[] = "/afr/bower/x-editable/dist/bootstrap3-editable/js/bootstrap-editable.js";
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";  
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/extensions/editable/bootstrap-editable.css";

  $scope = $app->page->scope;
  $errors = $messages = array();
  $client_id = $scope->client_id = $app->client_id;
  $client_name = $client_id ? get_object('client', $client_id, 'name') : '';
  $ptype = $scope->property_type = pick_first($siteObj->property_type, $app->property_type, "shortstay");
  $filters = array();
  $filters['client_id'] = $client_id;
  $system_user = 0;
  $host_ids = user_host_list($u_id) ?: array();

  if($u_level >= USER_LEVEL_STAFF) {
    $system_user = $scope->system_user = 1;
    $host_ids = array();
  } elseif(count($host_ids)) { 
    $filters['host_id'] = $host_ids;
  } else {
    return scope_error($app, "You are not a host, and not authorized to manage any hosts", '404');
  }
  
  $obj_type = $ptype;

    
  /** handle filters */
  if(!isset($_GET['active'])) $_GET['active'] = 1;
  foreach(array('active','host_id','address_country') as $fld) {
    if(isset($_GET[$fld]) && strlen($_GET[$fld])) {
      $val = trim($_GET[$fld]);
      switch($fld) {        
        default:
          $filters[$fld] = $val;
          break;
      }
    }
  }  

  $host_id = count($host_ids) == 1 ? $host_ids[0] : $filters['host_id'];
  if($host_id) $client_name = user_display_name($host_id);
  if($cc = $filters['address_country']) $client_name .= " in ".find_object('countries', array('id' => $cc), 'name');

  //dump($filters);
  $scope->count = $count = count_objects($obj_type, $filters);
  
  if(!$host_ids) {
    $host_ids = find_objects($obj_type, $filters, array("field" => "distinct host_id"));
  }
  
  $scope->countries = $countries = find_objects($obj_type, $filters, array("field" => "distinct address_country"));
  $scope->user_level = $u_level;
  $scope->host_ids = $host_ids;
  $scope->filters = $filters;
  $filter_json = count($filters) ? json_encode($filters) : '';
  $scope->obj_type = $obj_type;
  $obj_name = get_object_name($obj_type);
  $scope->data_url = "/ajax.php?oper=get-rows&obj_type=$obj_type".($filter_json ? "&filters=".urlencode($filter_json) : '');
  $scope->edit_url = "/ajax.php?oper=save&obj_type=$obj_type";
  $scope->title = $obj_name."s ".($client_id ? " for $client_name" : "")." ({$count})";
  
  return;
}

/** host rental list */
/** /my-rentals/ page = list view */
function controller_rental_list($app, $options=[]) {
  global $__USER, $u_id;
  $siteObj = $app->site;
  $scope = $app->page->scope;

  /** rental list page - and rental list tab (for single rental edit */    
  $ptype = $scope->property_type = $siteObj->property_type ?: 'shortstay';
  
  $host_ids = user_host_list($u_id);
  $pSearch = array("host_id" => $host_ids);
  $pSearch = array_merge_json($pSearch, $siteObj->property_filters);
  
  $limit = 10;
  $start = $scope->start = $_GET['start'] ?: 0;        
  $pOptions = array('limit' => $limit, 'start' => $start, 'sort' => 'name');
  
  $scope->rental_count = $count = count_objects($ptype, $pSearch);
  $scope->rental_list = $rentals = find_objects($ptype, $pSearch, $pOptions);
  
  foreach($rentals as $aptObj) { /** check if we can list */
    if($aptObj->active) continue;
    $steps = property_remaining_steps($siteObj, $aptObj->type, $aptObj->id);
    $aptObj->steps_remaining = count($steps); 
  }

  /** only on rental list page */    
  $descObjs = find_objects('description_type', array('generic' => 1), array("sort" => "id desc"));
  $generic_types = [];
  foreach($descObjs as $descObj) {
    $key = "text_".url_safe($descObj->name)."-example";
    $descObj->example = phrase($key);
    $generic_types[] = $descObj->id;
  }
    
  $scope->description_types_generic = $descObjs;

  $host_id = 0;
  if($__USER->user_level == USER_LEVEL_HOST) {
    $host_id = $u_id;
  } else {
    $host_ids = user_host_list($u_id);
    if(count($host_ids) > 1) {
      $rental_host_ids = find_objects($ptype, ['host_id' => $host_ids], ['fields' => 'distinct host_id']);
      if(count($rental_host_ids) == 1) {
        $host_id = $rental_host_ids[0];
      } else {
        $host_id = $rental_host_ids[0]; // pick the first
        $host_name = user_display_name($host_id);
        $scope->warning = "Note: You are managing rentals for multiple hosts. Generic descriptions apply to {$host_name}.";
        //$scope->error = "You are managing rentals for multiple hosts. Generic descriptions for multiple hosts is not yet implemented. Please contact support";
      }
    }
  }
      
  if($host_id) {
    // look for existing common/generic descriptions
    $descObjs = find_objects('property_description', ["host_id" => $host_id, "property_type" => $property_type, "property_id" => 0, "type" => $generic_types, "active" => 1]);
    $descriptions = [];
    foreach($descObjs as $descObj) {
      $type = $descObj->type;
      $descTypeObj = get_object('description_type', $type);
      if(!$descTypeObj->private) $description_more .= html_h4(phrase($descTypeObj->name, CAPITALIZE)).nl2br($descObj->description);        
      $descriptions[$descObj->type] = $descObj;
    }
  }
  $scope->host_id = $host_id;
  $scope->guides_generic = $descriptions;
  
  $scope->template = 'rental-list.html'; 

  return;
  
}
  
/** edit rental */
function controller_rental_edit($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $scope = $app->page->scope;
  $siteObj = $app->site;
  $pageObj = $app->page;
  $ver = $app->version;

  if(!$u_id) {
    redirect_login();
    return;
  }
  
  $id = $obj_id = $apt_id = $scope->obj_id = $scope->apt_id = pick_first($scope->rental_id, $scope->apt_id, $scope->obj_id, $_GET['apt_id'] ?: $_GET['obj_id'], $options['apt_id'] ?: $options['obj_id'], $app->object_id);
  $t = $ptype = $scope->property_type = $siteObj->property_type ?: 'shortstay';
  
  /** host rental list use same URL - /edit-rental/ */
  if(!$id) return  controller_error($app, "Missing rental ID");

  /** OK: edit single rental */  
  $__header->scripts[] = "/afr/bower/geocomplete/jquery.geocomplete.min.js";
  $__header->scripts[] = "/afr/js/admincal-dialog.js?v=$ver";
  $__header->styles[] = "/afr/css/afr_tooltip.css";
      
  jfu_dependencies($app);
  
  $__header->scripts[] = "/afr/js/admincal.js?v=$ver";
  $__header->scripts[] = "/afr/js/admincal-dialog.js?v=$ver";
  $__header->styles[] = "/afr/css/admincal.css?v=$ver";
  $__header->styles[] = "/afr/css/afr_gmenu.css?v=$ver";

  
  $aptObj = get_object($t, $id);
  print_log("controller_rental_edit t=$t id=$id apt=".dump($aptObj,true), 'app', LOG_LEVEL_TEST);
  
  $scope->auth_edit = $auth_edit = auth_rental($aptObj);
  if(!auth_rental($aptObj)) return controller_error($app, "Edit Rental \"{$aptObj->name}\" not authorized");
  //$scope = new stdClass;
  controller_rental_detail($app, $options);

  /** moved from rental_detail */
  $steps_remaining = 0;
  if($source_id = $aptObj->source_id) {
    $scope->import_url = ajax_operation_link('rental-import', array('site_id' => $siteObj->id, 'property_type' => $aptObj->type, 'apt_id' => $id, 'source_id' => $source_id), array('public' => false));
  } else {
    $scope->steps_remaining = property_remaining_steps($siteObj, $aptObj->type, $aptObj->id);
    //if(count($steps_remaining) && $aptObj->active) update_object('property', array('active' => 0), $aptObj->id); // deactivate;
  }
  
  $scope->uploader = property_uploader($ptype, $id, array("view" => "grid", "handler" => "property_photo_handler")); // doesn't work in switch, as dependencies are not loaded
  
  
  //$scope->subview = $template = pick_first($pageObj->subview, $_GET['view'], $_GET['template'], $options['template'], "_rental-edit-location.html");  
  //if($template && strpos($template, '_rental-edit-') === false) $template = $scope->subview = $app->page->subtemplate = "_rental-edit-{$template}.html"; // allow shortform
  $subview = $app->page->subview ?: 'location';  
  $app->page->subtemplate = "_rental-edit-{$subview}.html";
  switch($subview) {
    case "location":
      break;
    case "photos":
      $mediaData = array("user_id" => $u_id, "site_id" => $siteObj->id, "parent_type" => $ptype, "parent_id" => $id, "parent_field" => "media");
      if($aptObj->source_id == SOURCE_ID_INTERHOME) { /** todo: check if media is list of external IDs */
        $mediaData['inline'] = 1;
      }
      $scope->media_data = $mediaData; 
      break;
    case "calendar":
      $scope->single_calendar_edit = true;

      //$site_url = site_full_url($siteObj);      
      
      /** doesn't work on Hitabrick for some reason, deirects to http */
      //$scope->single_calendar_url = $url = "/api/calendar/?host_id={$aptObj->host_id}&amp;apt_id={$aptObj->id}&amp;status=1";
      
      $scope->single_calendar_url = $url = ajax_operation_link('rental-calendar-events', array('apt_id' => $id, 'host_id' => $aptObj->host_id, 'single' => 1));        
      print_log("api-url: {$url}", 'calendar', LOG_LEVEL_TEST);
      
      $scope->single_calendar_options = array("selectable" => true, "editable" => true, "weekNumbers" => true);
      controller_calendar($app); /** loads rentals */
      
        // "events" => "/afr/XML/HV/property-calendar.php?format=event&id=$id&edit=1");
      break;
    case "description":
      $scope->short_desc_example = $phrase = phrase("text_short-description-example");
      //dump($phrase);
      break;
    case "guide":
      $desc_types = array(DESC_TYPE_PROPERTY,DESC_TYPE_AREA);
      if($siteObj->golf_site) $desc_types[] = DESC_TYPE_GOLF;
      $desc_types[] = DESC_TYPE_FOOD; // private 
      $desc_types[] = DESC_TYPE_RULES;
      $desc_types[] = DESC_TYPE_INTERACTION;
      $desc_types[] = DESC_TYPE_DIRECTIONS; // private
      $desc_types[] = DESC_TYPE_CHECKIN; // private 
      //dump($desc_types);
      $descObjs = find_objects('description_type', array('id' => $desc_types), array("sort" => "private, id desc"));
      foreach($descObjs as &$descObj) {
        $key = "text_".url_safe($descObj->name)."-example";
        $descObj->example = phrase($key);
      }
      $scope->description_types = $descObjs;
      // not in use DESC_TYPE_TRAVEL,DESC_TYPE_QUICK,DESC_TYPE_ALBUM,DESC_TYPE_STRAP,

      break;
    case "amenities":
      $scope->amenities = rental_amenities($siteObj, $aptObj, array('searchable' => 0));
      //$scope->debug = dump($scope->amenities,true);
      //dump($facArray);
      break;
    case "import":
      $scope->sources = find_objects('calendar_import', array('property_type' => $ptype, 'apt_id' => $id, 'active' => 1));
      break;
      
    case "reviews":
      $start = $scope->start = $_GET['start'] ?: 0;
      $limit = $scope->limit = 10;
      $searchOptions = array("start" => $start, "limit" => $limit);        
      $search = ["client_id" => $app->client_id, "parent_id" => $id, "parent_type" => $aptObj->type, "active" => 1, "published" => 1];
      $scope->reviews = $reviews = find_objects("review", $search, $searchOptions);
      $scope->review_count = count_objects("review", $search);
      $scope->items = $items = find_objects('review_item', ['site_id' => $siteObj->id, 'parent_type' => $aptObj->type]);
     break;
    case "terms":
      break;
    case "pricing":
      break;
    case "list":
      $host_ids = user_host_list($u_id);
      $pSearch = array("host_id" => $host_ids);
      $pSearch = array_merge_json($pSearch, $siteObj->property_filters);
      $start = $scope->start = $_GET['start'] ?: 0;        
      
      $pOptions = array('limit' => 10, 'start' => $start);
      
      $scope->rental_list = $rentals = find_objects($ptype, $pSearch, $pOptions);
      $scope->rental_count = $count = count_objects($ptype, $pSearch);
      
      foreach($rentals as $aptObj) { /** check if we can list */
        if($aptObj->active) continue;
        $steps = property_remaining_steps($siteObj, $aptObj->type, $aptObj->id);
        $aptObj->steps_remaining = count($steps); 
      }
      break;
      
    default:
      return controller_error($app, "Unknown subview {$subview}");
      break;
  }
  

  return;
  
}
  
/** alias */
function controller_rental($app, $options = []) {
  return controller_rental_detail($app, $options);
}

function controller_rental_detail($app, $options = []) {  
  global $u_id, $__USER, $__LANGUAGE;
  $__header = $app->header; 
  $siteObj = $app->site;
  $scope = $app->page->scope;
  $scope->stack = caller_stack();
  $pageObj = $app->page;
      
  print_log("controller_rental_detail: scope=".dump($scope,true), 'app', LOG_LEVEL_TEST);
  extract($_GET);
  
  $__header->scripts[] = "/afr/bower/fullcalendar/dist/fullcalendar.js";
  $__header->styles[] = "/afr/bower/fullcalendar/dist/fullcalendar.css";
  $__header->styles[] = "/afr/bower/foundation-icon-fonts/foundation-icons.css";
  $__header->styles[] = "/afr/css/afr-air.css";

  $ver = $app->version ?: date('md');
    
  $imageAr = array();
  if($data = $options['data']) { // from fetch-template
    extract($data);    
  }
  
  $t = $ptype = $scope->property_type = $siteObj->property_type ?: 'shortstay';
  $id = $obj_id = $apt_id = $scope->obj_id = $scope->apt_id = pick_first($scope->rental_id, $scope->apt_id, $scope->obj_id, $_GET['apt_id'] ?: $_GET['obj_id'], $options['apt_id'] ?: $options['obj_id'], $app->object_id);  
  
  if(!$id) return controller_error($app, "Missing rental ID");

  
  $aptObj = get_object($t, $id);
  if(!$aptObj) return controller_error($app, "Rental not found");
      
  /** This controller is also used by rental edit */
  $edit = $pageObj->page_type == PAGE_EDIT_PROPERTY;
  $scope->auth_edit = $auth_edit = auth_rental($aptObj);  
  if($edit && !$auth_edit) return controller_error($app, "None of your rentals found");
  
  /** viewing deactivated property */ 
  if(!$aptObj->active && !$edit) {
    if($auth_edit) {
      $scope->message = "Preview: Rental not activated";
    } else {
      return controller_error($app, "No active rental found", '403');
    }
  }
  
  
  /** viewing active property - not edit - update page views */ 
  if($aptObj->active && !$edit) {
    $pv = ($aptObj->pageviews ?: 0) + 1;
    update_object('property', ['pageviews' => $pv], $aptObj->id);
  }
  
  $qs = get_query_string();
  $scope->search_url = $app->search_url = site_page_link($siteObj->id, PAGE_RENTAL_SEARCH).($qs ? "?{$qs}" : "");
  
  $in = $_GET['in'] ? date2sql($in) : '';
  $out = $_GET['out'] ? date2sql($out) : '';
  $ng = $_GET['ng'] ?: 1; 
  $nd = $in && $out && $in > $out ? datediff('days', $in, $out) : 0;

  $url = property_geolink($aptObj);
  $this_url = this_path();
  $script = $_SERVER['SCRIPT_NAME'];
  $method = $_SERVER['REQUEST_METHOD'];
  $ref    = $_SERVER['HTTP_REFERER'];
  
  $redirect = $url != $this_url && !$edit && !$scope->ajax && $method == 'GET' && !strpos($script, 'ajax.php') ? 1 : 0;
  if(0 && $redirect) {
    print_log("rental_controller: redirecting: script: {$script} method:{$method} ref:{$ref}", 'rental', LOG_LEVEL_TEST);
    $qs = get_query_string();
    if($qs) $url .= "?{$qs}";
    redirect($url);
  }
  
  $scope->id = $aptObj->id; /** important or template won't work */
  $scope->rental = $orgAptObj = clone $aptObj; /** something (price breakdown) changes $aptObj */
  $scope->property_link = property_geolink($aptObj);

  if($host_id = $aptObj->host_id) {    
    $hostObj = get_user($host_id);
    $scope->host = $hostObj;
  }

  unset($aptObj->address_number);
  unset($aptObj->address_suffix);
  unset($aptObj->address_postcode);
  $scope->address = get_address_string($aptObj, ', ');

  /** Property Guide */    
  $descObjs = find_objects('property_description', ["property_id" => $aptObj->id, "active" => 1]);
  //dump($descObjs);
  $descriptions = array();
  $description = '';
  $description_more = '';
  
  
  foreach($descObjs as $descObj) {
    $type = $descObj->type;
    if($type == DESC_TYPE_PROPERTY) {
      if($descObj->site_id == $siteObj->id && $descObj->language == $__LANGUAGE) {
        $description = nl2br($descObj->description);
      } else {
        unset($descObj->id); // so we can save per site and language
        if(!$description) { // will be ignored or overwritten by above if correct site+language is found
          $description = nl2br($descObj->description);
          if($descObj->language != $__LANGUAGE) {
            $debug = "(Lang={$descObj->language} this={$__LANGUAGE})<br>";
            $debug =  '';
            $description = $debug.$description; 
          }
        }
      }
    } else {
      $descTypeObj = get_object('description_type', $type);
      if(!$descTypeObj->private) $description_more .= html_h4(phrase($descTypeObj->name, CAPITALIZE)).nl2br($descObj->description);
    }
    
    $descriptions[$descObj->type] = $descObj;
          
  }

  /** check avaialbility, including num_guests, min/max stay, changeover, etc */
  $scope->rate_average = 0;  
  $scope->discount_rate_average = 0;
  $scope->discount = '';
  $scope->description = $description.$description_more; /** used for public site */
  $scope->guides = $descriptions;
  
  if($in && $out) { /** search - get average rate */
    
    list($avail, $reason) = rental_availability($aptObj, ['in' => $in, 'out' => $out, 'ng' => $ng]);
    $scope->available = $avail;
    $scope->reason = $reason;
    $scope->rate_title = $aptObj->source_id == SOURCE_ID_INTERHOME ? "Approx." : "Average"; 

    
    $searchObj = array2obj($_GET, array('map' => 'urldecode'));
    $searchObj->in = $in;
    $searchObj->out = $out;
    $searchObj->t = $aptObj->type; 
    $searchObj->apt_id = $aptObj->id; 
    
    $city_id = $aptObj->city_id;
    $priceObj = get_average_rate($aptObj, $in, $out, $ng, true);
    
    print_log("Rate object for {$aptObj->name} in details:".dump($priceObj, true), 'property-search', LOG_LEVEL_TEST);
    
    $scope->rate = $priceObj;
    $scope->rate_average = $aptObj->rate_average = $avg = round($priceObj->average);
    $scope->rate_currency = $priceObj->currency;

    $scope->rate_unit = $rate_unit = 'night';
    if($priceObj->average_weekly) {
      $scope->rate_unit = $rate_unit = 'week';
      $scope->rate_average = round($priceObj->average_weekly);
    } elseif($priceObj->average_monthly) {
      $scope->rate_unit = $rate_unit = 'month';
      $scope->rate_average = round($priceObj->average_monthly);
    }
    
    if($priceObj->average_discounted) {
      $scope->discount = $aptObj->discount = $priceObj->discount;
      $scope->discount_rate_average = $aptObj->discount_rate_average = $priceObj->average_discounted;
    }
    
    /**
    
    $scope->price_breakdown = print_price_breakdown($aptObj, $priceObj);

    $scope->price_breakdown_link = 
      html_fancybox_link(
        ajax_operation_link('price-breakdown', $searchObj, array('public' => true)),  
        phrase('price_breakdown', CAPITALIZE), 
        array('class' => 'price-breakdown-link', 'data-url' => ajax_operation_link('price-breakdown', null, array('public' => true)), 'data-data' => array('t' => $t, 'apt_id' => $aptObj->id, 'in' => $in, 'out' => $out, 'ng' => $ng)));
      
    if(isset($_GET['foo'])) {
      echo("Average for $ng guests from $in to $out = $avg<br>");
      dump($priceObj);
    }
    */
    
  } else {
    
    $currency = strtoupper($aptObj->currency);
    $rate = $aptObj->rate_day;
    if($currency != SYSTEM_CURRENCY) {
      $aptObj->rate_day = round(get_exchange_rate($currency, SYSTEM_CURRENCY, $rate));
      $aptObj->currency = SYSTEM_CURRENCY;
      print_log("{$aptObj->id} {$currency}{$rate} => {$aptObj->currency}{$aptObj->rate_day}", 'rental-rate', LOG_LEVEL_TEST);
      $scope->rental = $aptObj;
    }
    
    $scope->rate_currency = $aptObj->currency;
    $scope->rate_average = $aptObj->rate_average = $aptObj->rate_day;
  }
  
  /** used for single calendar */
  if($orgAptObj->discount_week) $orgAptObj->rate_week = 0; /** discount takes precedence over fixed rate */
  if($orgAptObj->discount_month) $orgAptObj->rate_month = 0; /** discount takes precedence over fixed rate */
  if($orgAptObj->discount_2_weeks) $orgAptObj->rate_2_weeks = 0; /** discount takes precedence over fixed rate */
  if($orgAptObj->discount_3_weeks) $orgAptObj->rate_3_weeks = 0; /** discount takes precedence over fixed rate */
  
  $rate_weekend = $aptObj->rate_weekend > 0 ? $aptObj->rate_weekend : $aptObj->rate_day;
  $scope->single_calendar_rental = 
    array("id" => $aptObj->id, 
          "name" => htmlspecialchars($aptObj->name, ENT_QUOTES), 
          "type" => $aptObj->type, 
          "host_id" => $aptObj->host_id,
          "owner_id" => $aptObj->owner_id, /** for historical reasons */
          "currency" => $aptObj->currency, "cur" => get_currency_symbol($aptObj->currency,true), 
          "rate_unit" => $rate_unit,
          "rate_day" => $orgAptObj->rate_day,            
          "rate_week" => $orgAptObj->rate_week, 
          "rate_2_weeks" => $orgAptObj->rate_2_weeks, 
          "rate_3_weeks" => $orgAptObj->rate_3_weeks, 
          "rate_month" => $orgAptObj->rate_month, 
          "rate_weekend" => $rate_weekend, 
          "rate_extraperson" => $aptObj->rate_extraperson ?: 0, 
          "rate_average" => $aptObj->rate_average ?: $aptObj->rate_day, 
          "discount_rate_average" => $aptObj->discount_rate_average ?: 0, 
          "rate_num_guests" => $aptObj->rate_num_guests ?: 2, 
          "discount_week" => $orgAptObj->discount_week, 
          "discount_2_weeks" => $orgAptObj->discount_2_weeks, 
          "discount_3_weeks" => $orgAptObj->discount_3_weeks, 
          "discount_month" => $orgAptObj->discount_month,
          "ng" => $ng, "in" => $in, "out" => $out, "nd" => $nd, "changeover" => $aptObj->changeover);    
  
  if(!$edit) { /** public details page */
    $mediaObjs = property_images($id) ?: explode(',', $aptObj->media);
    if($mediaObjs) {
      foreach($mediaObjs as $mediaObj) {
        $image = new stdClass;
        $image->src = print_media($mediaObj, array('size' => 'big', 'source_only' => true));
        $image->title = $mediaObj->name;
        $imageAr[] = $image;
      }
    }
    $scope->images = $imageAr;

    $scope->calendar = 1; // show availability calendar 
    
    $nearbyApts = location_find_nearest($t, $aptObj->location_lat, $aptObj->location_long, array('limit' => 4, 'exclude' => $aptObj->id));
    foreach($nearbyApts as $nearbyApt) {
      if($nearbyApt->currency != SYSTEM_CURRENCY) {
        $currency = $nearbyApt->currency; // just for logging
        $rate = $nearbyApt->rate_day; // just for logging
        $nearbyApt->rate_day = round(get_exchange_rate($nearbyApt->currency, SYSTEM_CURRENCY, $nearbyApt->rate_day));
        $nearbyApt->currency = SYSTEM_CURRENCY;
        print_log("nearby: {$nearbyApt->id} {$currency}{$rate} => {$nearbyApt->currency}{$nearbyApt->rate_day}", 'rental-rate', LOG_LEVEL_TEST);
      }
    }
    $scope->nearby_apts = $nearbyApts;

    if($siteObj->golf_site) {
      //echo("Got here ptype=$t nearbyapts=".count($scope->nearby_apts));
      $scope->nearby_golf = location_find_nearest('golf_club', $aptObj->location_lat, $aptObj->location_long, array('limit' => 4));
    }
    $scope->edit_link = $auth_edit ? property_edit_link($t, $id, true) : "";

    //$start = $scope->start = $_GET['start'] ?: 0;
    //$limit = $scope->limit = 10;
    $review_search = ["client_id" => $app->client_id, "parent_id" => $id, "parent_type" => $aptObj->type, "active" => 1, "published" => 1];
    $scope->reviews = find_objects("review", $review_search, ["limit" => 10]);
    $scope->review_count = count_objects("review", $review_search);
    
    if(strip_blank($aptObj->quote)) {        
      $scope->quote = $aptObj->quote;
    } else {
      $quote = host_quote($aptObj->host_id, $id);
      $scope->quote = $quote ? $quote->quote : '';        
    }
    
    if($peak_exceptions = find_surcharge_exceptions($aptObj)) {    
      $exceptions = html_h4(phrase('exceptions', CAPITALIZE).':');
      $exceptions .= print_exceptions($aptObj->type, $id, $peak_exceptions);;
      $scope->description .= $exceptions; 
    }
    
  }
  
  
  $facAr = $aptObj->facilities ? explode(',', $aptObj->facilities) : array();
  $facs = array();
  foreach($facAr as $fac_id) {
    $facObj = new stdClass;
    $facObj->id = $fac_id;
    $facObj->name = get_object("facility", $fac_id, "name");
    $facObj->icon = rental_facility_icon($fac_id) ?: "fa fa-check";
    $facs[] = $facObj;
  }

  $scope->facs = $facs; // $scope->facilities = aptObj->facilities, already in use
  $scope->single_calendar_edit = $cal_edit = false;
  $event = 'rental-calendar-availability';
  $scope->single_calendar_url = ajax_operation_link($event, array('id' => $id, 'edit' => 0), array('public' => true));
  $calOptions = array("selectable" => true, "editable" => false);
  
  if($in) $calOptions['defaultDate'] = $in;
  $scope->single_calendar_options = $calOptions;
  
  $today = today();      
  $yesterday = yesterday();      
  if($aptObj->calendar_startdate || $aptObj->calendar_startdate < $yesterday) {
    $res = rental_update_calendar($aptObj);
    if($res->success && is_array($res->data)) {
      $aptObj->calendar_availability = $res->data['calendar_availability'];
      $aptObj->calendar_startdate = $res->data['calendar_startdate'];
    }
  }

  
  $availabilty = [];
  if($aptObj->calendar_availability && $aptObj->calendar_startdate) {
    $offset = day_diff($aptObj->calendar_startdate, $today); // how many days ago
    $avail = substr($aptObj->calendar_availability, $offset);
    $length = strlen($avail);
    $blockedDates = [];
    if(strpos($avail, 'N') !== false) {
      //die("got here avail=$avail");
      $arr = str_split($avail);
      $d = $today;
      foreach($arr as $a) {
        if($a == 'N') $blockedDates[] = date(DATEPICKER_FORMAT_PHP, strtotime($d));
        $d = next_day($d);
      }
    }
    
    $availabilty = ["blocked_dates" => $blockedDates, "first" => $today, "last" => add_day($today, $length),  "length" => $length];
  } 
  
  if($changeover = $aptObj->changeover) {

    /** note - convert to 0=sun in JavaScript due to https://bootstrap-datepicker.readthedocs.io/en/latest/options.html#daysofweekdisabled */
    $wd = [0 =>'sun',1=>'mon',2=>'tue',3=>'wed',4=>'thu',5=>'fri',6=>'sat'];
    if($changeover == 7) $changeover = 0; 
    unset($wd[$changeover]);
    $availabilty["blocked_weekdays"] = array_keys($wd); // block these days for check-in 
    
  }
  
  $scope->availability = count($availabilty) ? $availabilty : '';

  $scope->data = array("t" => $t, "id" => $id, "reset"  => 1); // send to ajax fetch-template method and back to this controller

  //die("ok, got here end id={$id} tpl:{$app->page->template} scope".dump($scope,true));
  
  return;
}

function controller_rental_inquiry($app, $options = []) {
  $options['request_type'] = 'inquiry';
  rental_booking_handler($app, $options);
  $scope = $app->page->scope;
  
  $resObj = $scope->booking;
  switch($resObj->status) {
  case STATUS_DRAFT:
    break;
  case STATUS_INQUIRY:
    $scope->template = "rental-inquiry-success.html";
    break;
  default:
    break;
  }
  
  return;
}

function controller_rental_request($app, $options = []) {
  $options['request_type'] = 'request';
  rental_booking_handler($app, $options);
  $scope = $app->page->scope;

  $resObj = $scope->booking;
  switch($resObj->status) {
  case STATUS_DRAFT:
    break;
  case STATUS_REQUEST:
    $scope->template = "rental-request-success.html";
    break;
  default:
    break;
  }

  return;
}

function controller_rental_booking($app, $options = []) {
  $options['request_type'] = 'booking';
  rental_booking_handler($app, $options);
  return;
}


/** common handler for inquiry/request/booking above*/
function rental_booking_handler($app, $options = array()) {
  $siteObj = $app->site;
  $pageObj = $app->page;
  $scope = $app->page->scope;
  $obj_id = $slug = $scope->obj_id = $app->slug ?: $app->object_id;
  if(!$obj_id) return scope_error($app, "Missing rental ID", '404');
  $obj_type = pick_first($scope->property_type, $app->object_type, $pageObj->property_type, $siteObj->property_type, 'shortstay');  
  $obj = $aptObj = $scope->rental = get_object($obj_type, $obj_id);
  if(!$obj) return scope_error($app, "Rental {$slug} not found", '404');
  
  extract($_GET);
  $in  = $_GET['in'] ? date2sql($in) : '';
  $out = $_GET['out'] ? date2sql($out) : '';
  $ng  = $_GET['ng'] ?: 1;
  $res_id  = $_GET['rid'];
  
  if($res_id) {
    $resObj = get_object('reservation', $rid);
    // security
    $auth_res = auth_res($resObj);
    if($auth_res < USER_LEVEL_GUEST) return controller_error($app, "Not authorized");    
  } elseif($in && $out && $ng) {
    $resData = search2res(['in' => $in, 'out' => $out, 'ng' => $ng, 'property_type' => $obj_type, 'apt_id' => $obj_id]);
    //echo("gh in=$in out=$out ng=$ng t=$obj_type id=$obj_id".dump($resData,true));
    if($error = $resData['error']) {
      $scope->error = $error;
    }

    $resObj = array2obj($resData);
  } else {
    return controller_error($app, "Missing search parameters");
  }
  
  
  if($aptObj->source_id == SOURCE_ID_INTERHOME && $aptObj->guid) {
    
    $result = intherome_live_availability($aptObj->guid, strip_time($resObj->checkin), strip_time($resObj->checkout), $resObj->num_guests);
    print_log("Before Interhome Rate: {$resObj->currency} {$resObj->rate_base} Total: {$resObj->total} Fee: {$resObj->fee_service} GT: {$resObj->grand_total}", 'interhome', LOG_LEVEL_TEST);
    
    if($result->success) {
      $total = $result->total;
      $currency = $result->currency;
      $rate_day = $total / $resObj->num_days;
      $service_fee = service_fee($total);
      $dest_id = SOURCE_ID_INTERHOME;
      
      if($res_id) { /** no longer in use ? */
        if($total) update_object('reservation', array('rate_base' => $rate_day, 'currency' => $currency, 'fee_service' => $service_fee, 'dest_id' => $dest_id), $res_id);
        $resObj = get_object('reservation', $res_id); // reload
      } else {
        $resObj->currency = $currency;
        $resObj->rate_base = $rate_day;
        $resObj->total = $total;
        $resObj->fee_service = $service_fee;
        $resObj->grand_total = $total + $service_fee;
        $resObj->dest_id = $dest_id;
      }
      
      if(!$app->live) $scope->messages = $result->message;
      $scope->source_services = $result->services;
      print_log("Ok, Interhome Rate: {$rate_day} Total: {$total} Fee: {$service_fee} GT: {$resObj->grand_total}", 'interhome', LOG_LEVEL_TEST);
    } else {
      $scope->source_error = $result->error ?: "Unknown error from Booking Partner";
      $ts = now();
      print_log("$ts: Interhome request failed: id:{$resObj->res_id} property:{$aptObj->id} in:".strip_time($resObj->checkin)." out:".strip_time($resObj->checkout)." guest:{$resObj->guest_id} Errors:".dump($result, true), 'interhome', LOG_LEVEL_ALWAYS);
    }
  }
  
 
  $scope->booking = $resObj;  
  $scope->search_url = $app->search_url ?: site_page_link($siteObj->id, PAGE_RENTAL_SEARCH);
  $scope->feeArray = array('fee_service' => "Service Fee", 'fee_cleaning' => "Cleaning Fee", 'fee_booking' => "Booking Fee");
  
  return;
}

function controller_dictionary($app, $options = []) {
  global $u_id, $__USER;
  $__header = $app->header; 
  $siteObj = $app->site;
  $__header->scripts[] = "/afr/bower/jquery-file-download/src/Scripts/jquery.fileDownload.js";
  $__header->scripts[] = "/afr/bower/table-export/jquery.base64.js";
  $__header->scripts[] = "/afr/bower/table-export/tableExport.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js"; /** todo: support other languages, use $app->locale  */
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/export/bootstrap-table-export.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/editable/bootstrap-table-editable.min.js";
  $__header->scripts[] = "/afr/bower/x-editable/dist/bootstrap3-editable/js/bootstrap-editable.js";

  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/extensions/editable/bootstrap-editable.css";


  $scope = $app->page->scope;
  $errors = $messages = array();
  
  $obj_type = 'dictionary';

  $filters = array();
  $tbl = get_object_table($obj_type);

  //$filters["{$tbl}.site_id"] = [ATT_DEFAULT_NULL, $siteObj->id];  
  //$filters["{$tbl}.site_id"] = array('term' => ['!=', 'text_*'], 'site_id' => ['$or', [$siteObj->id, '$NULL']]);
  if($site_id = $_GET['site_id']) $filters["{$tbl}.site_id"] = $site_id;
  else $filters["{$tbl}.site_id"] = ['$or', [$siteObj->id, '$NULL']];
  //'language' => $language, 
   
  foreach(array('language','site_id') as $fld) {
    if($val = trim($_GET[$fld])) $filters[$fld] = $val;
  }
  
  if($__USER->user_level < USER_LEVEL_SYSTEM && $siteObj->id == 265)  $filters['site_id'] = $siteObj->id;
  
  $scope->filters = $filters;
  $filter_json = count($filters) ? json_encode($filters) : '';
  $scope->obj_type = $obj_type;
  $scope->data_url = "/ajax.php?oper=get-rows&obj_type=$obj_type".($filter_json ? "&filters=".urlencode($filter_json) : '');
  $scope->edit_url = "/ajax.php?oper=save&obj_type=$obj_type";
  $scope->title = "Dictionary";
    
  return;
}

function controller_users($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->scripts[] = "/afr/bower/jquery-file-download/src/Scripts/jquery.fileDownload.js";
  $__header->scripts[] = "/afr/bower/table-export/jquery.base64.js";
  $__header->scripts[] = "/afr/bower/table-export/tableExport.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js"; /** todo: support other languages, use $app->locale  */
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/export/bootstrap-table-export.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/editable/bootstrap-table-editable.min.js";
  $__header->scripts[] = "/afr/bower/x-editable/dist/bootstrap3-editable/js/bootstrap-editable.js";  
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/extensions/editable/bootstrap-editable.css";

  $scope = $app->page->scope;
  $errors = $messages = array();
  $client_id = $scope->client_id = $app->client_id;
  $client_name = $client_id ? get_object('client', $client_id, 'name') : '';
  
  $obj_type = $options['obj_type'] ?: 'user';

  $filters = array();
  $tbl = get_object_table($obj_type);
  $filters["{$tbl}.client_id"] = $client_id;
  foreach(array('user_level','org_id','start','end') as $fld) {
    if($val = trim($_GET[$fld])) {
      switch($fld) {
      case 'start':
        $filters['created'] = array(">=", date2sql($val));
        break;
      case 'end':
        $filters['created'] = array("<=", date2sql($val));
        break;
      case 'user_level':
        $filters['user_level'] = array("=", $val);
        break;
      default:
        $filters[$fld] = $val;
        break;
      }
    }
  }
  
  $scope->filters = $filters;
  $filter_json = count($filters) ? json_encode($filters) : '';
  $scope->obj_type = $obj_type;
  $scope->data_url = "/ajax.php?oper=get-rows&obj_type=$obj_type".($filter_json ? "&filters=".urlencode($filter_json) : '');
  $scope->edit_url = "/ajax.php?oper=edit-user";
  $scope->title = ucfirst($obj_type)."s ".($client_id ? " for $client_name" : "");
    
  return;
}

function controller_backend($app, $options = []) {
  global $u_id, $u_level,$__CLASS_FLAGS;
  $__header = $app->header; 
  $siteObj = $app->site;
  $__header->scripts[] = "/afr/bower/jquery-file-download/src/Scripts/jquery.fileDownload.js";
  $__header->scripts[] = "/afr/bower/table-export/jquery.base64.js";
  $__header->scripts[] = "/afr/bower/table-export/tableExport.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js"; /** todo: support other languages, use $app->locale  */
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/export/bootstrap-table-export.js";
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";

  $limit = 50;

  $scope = $app->page->scope;  
  $scope->class_flags = $__CLASS_FLAGS;
  $obj_type = $_GET['obj_type'];
  
  $classes = array();
  if($u_level < USER_LEVEL_ADMIN) return controller_error($app, "Not authorized", 401);
  $scope->classes = $classes = admin_menu_items($siteObj, $options);  
  
  if($obj_type) {
    $scope->obj_type = $obj_type;
    $scope->obj_name = get_object_name($obj_type);
    $scope->obj_key = get_object_key($obj_type);
    
    /** this is if classes are stored in DB */
    if($classObj = get_object('class', $obj_type, '', array('ignore_filters' => 1))) { /** read from db */     
      $scope->class_def = $classObj;
      $scope->class_attributes = find_objects('attribute', array('class' => $obj_type));
      $scope->class_security = find_objects('security', array('node' => $obj_type));
    }
    
    //echo("obj-type={$obj_type} class=".dump($classObj,true));
    /** read from file, disabled 
    $class_found = false;
    if($class_header = get_class_header($obj_type)) { 
      $class_found = true;
      $class_user_level = $class_header[CLASS_USER_LEVEL];
    }
    */
   
    $actions = array('view', 'list', 'add', 'edit', 'del', 'copy');
    
    foreach($actions as $ac) {
      $key = 'auth_'.$ac;
      $scope->$key = auth_action($obj_type, $ac);
    }
    if(!$classObj) {
      //return scope_error($app, "$obj_type not found", '404');
    } elseif($classObj->user_level > $u_level) {
      $scope->error = "Not authorized (User level)";
      return;
    } elseif(!$scope->auth_list) {
      return scope_error($app, "Not authorized (List)", '404');
    }
    $class_def = get_class_def($obj_type);
    $class_fields = $class_def->fields;
    $fields = array();
    foreach($class_fields as $field => $attributes) {
      if(!($attributes[ATT_FLAGS] & AF_HIDE_LIST)) $fields[$field] = $attributes[ATT_NAME];      
    }
    $scope->fields = $fields;
    $scope->data_url = "/ajax.php?oper=list&obj_type=$obj_type&limit=$limit";
        
    //$scope->auth_add = auth_action($obj_type, 'add');
  } else {
    //$scope->classes = get_classes();
  }
  //dump($items);
  
  return;
}


function controller_payments($app, $options = []) {
  // redirect_secure();
  $options['obj_type'] = 'payment';
  controller_invoices($app, $options);
  return;
}

function controller_payouts($app, $options = []) {
  // redirect_secure();
  $options['obj_type'] = 'withdrawal';
  controller_invoices($app, $options);
  return;
}

function controller_invoices($app, $options = []) {
  // redirect_secure();
  global $u_id, $u_level;
  $__header = $app->header; 
  $siteObj = $app->site;
  
  $__header->scripts[] = "/afr/bower/jquery-file-download/src/Scripts/jquery.fileDownload.js";
  $__header->scripts[] = "/afr/bower/table-export/jquery.base64.js";
  $__header->scripts[] = "/afr/bower/table-export/tableExport.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js"; /** todo: support other languages, use $app->locale  */
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/export/bootstrap-table-export.js";
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";

  $scope = $app->page->scope;
  $errors = $messages = array();
  $client_id = $scope->client_id = $app->client_id;
  $client_name = $client_id ? get_object('client', $client_id, 'name') : '';
  $host_ids = [];
  
  if($siteObj->property_site) {
    $host_ids = user_host_list($u_id) ?: array();
  }
  $ptype = pick_first($siteObj->property_type, $app->property_type, "shortstay");
  $filters = array();
  $filters['client_id'] = $client_id;
  $system_user = 0;
  if($u_level == USER_LEVEL_SYSTEM) {
    $system_user = $scope->system_user = 1;
  } elseif(count($host_ids)) { 
    $filters['host_id'] = $host_ids;
  } elseif($siteObj->property_site) {
    $scope->error = "You are not a host, and not authorized to manage any hosts";
    return;
  }
  
  
  $scope->booking_link = site_page_link($siteObj, PAGE_BOOKING);
  
  $obj_type = $options['obj_type'] ?: 'invoice';
  if(!in_array($obj_type, array('invoice', 'payment', 'withdrawal'))) {
    $scope->error = "Invalid class $obj_type";
    return;
  }
  
  switch($obj_types) {
    case "invoice";
    break;
    case "payment";
    break;
    case "withdrawal";
    break;    
  }
  /** handle filters */
  foreach(array('status','org_id','start','end', 'payment_type', 'payment_status', 'sender_id', 'recipient_id', 'currency', 'host_id') as $fld) {
    if($val = trim($_GET[$fld])) {
      switch($fld) {        
        case 'start':
          $filters['sent_date'] = array(">=", date2sql($val));
          break;
        case 'end':
          $filters['sent_date'] = array("<=", date2sql($val));
          break;
        case 'status':
          $filters['status'] = array("=", $val);
          break;
        case 'payment_status':
          $filters['payment_status'] = array(">=", $val);
          break;
        default:
          $filters[$fld] = $val;
          break;
      }
    }
  }
  
  if($_GET['start'] && $_GET['end']) $filters['sent_date'] = array("><", date2sql($_GET['start']), date2sql($_GET['end']));  
  //echo("type=$obj_type".dump($filters, true));
  
  $scope->filters = $filters;
  $filter_json = count($filters) ? json_encode($filters) : '';
  $scope->obj_type = $obj_type;
  $obj_name = get_object_name($obj_type);
  $scope->data_url = "/ajax.php?oper=get-rows&obj_type=$obj_type".($filter_json ? "&filters=".urlencode($filter_json) : '');
  $scope->title = $obj_name."s ".($client_id ? " for $client_name" : "");
  
  if(in_array($obj_type, array('payment', 'invoice'))) {
    $org_ids = find_objects($obj_type, array('client_id' => $client_id), array('fields' => 'distinct org_id'));
    $scope->org_ids = array_filter($org_ids);
  } else { // Payouts
    
    /** Filters from form */
    $scope->host_id = $host_id = $_GET['host_id'] ?: 0;
    $scope->currency = $currency = $_GET['currency'] ?: $currencies[0];
    if($host_id) {
      $scope->host = $hostObj = get_user($host_id);
      $scope->host_name = $host_name = get_user_name($hostObj);
    }

    $scope->title = "Past Payouts";
    //if($filters['recipient_id']) $search['recipient_id'] = $recipient_id;
    //if($filters['currency']) $search['payment_currency'] = $currency;  // todo chooser  


    /** Outstanding Payments */
    $search = array();
    if(!$system_user) $search['host_id'] = $host_ids;  
    
    if($start = $_GET['ostart']) $search['created'] = array(">=", date2sql($start));
    if($end = $_GET['oend']) $search['created'] = array("<=", date2sql($end));

    $search['client_id'] = $client_id;
    $search['gateway_id'] = GATEWAY_PAYPAL;
    $search['payment_status'] = array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PARTIALLY_REFUNDED);  
    $search['payment_type'] = array(PAYMENT_TYPE_CREDIT_CARD, PAYMENT_TYPE_PAYPAL, PAYMENT_TYPE_CREDIT_CARD_CAPTURE, PAYMENT_TYPE_ECHECK, PAYMENT_TYPE_REFUND);  
    //$search['payment_net'] = array('>=', INV_TOLERANCE);  
    $search['tx_id'] = array('>', 0);    
    $search['withdrawal_id'] = array('=', ATT_DEFAULT_NULL);  // todo: this excludes partial withdrawals

    //$scope->currencies = $currencies = find_objects("payment", $search, array("fields" => "distinct(payment_currency) as currency"));
    $scope->currencies = $currencies = array("" => "Select Currency...", "EUR" => "Euro", "USD" => "US Dollars");

    $hostsAr = array();
    $hostsAr[0] = "Select...";
    

    $tbl = get_object_table('user');
    $user_fields = array("{$tbl}.id", 'concat(user_profile.first_name, " ", user_profile.last_name) as name'); 

    if($host_ids = find_objects('payment', $search, array('fields' => "distinct host_id"))) {
      $hosts = find_objects('user', array("{$tbl}.id" => $host_ids), array('fields' => $user_fields));
      foreach($hosts as $host) $hostsAr[$host->id] = $host->name;
      $scope->outstanding_hosts = $hostsAr;
    }

    if($host_id) $search['host_id'] = $host_id;    
    if($currency) $search['payment_currency'] = $currency;
    
    $search_json = json_encode($search);    
    $scope->outstanding_url = "/ajax.php?oper=get-rows&obj_type=payment&filters=".urlencode($search_json);
    
    /** Outstanding Corrections and Transfers */
    $search = array();
    if(!$system_user) $search['host_id'] = $host_ids;    
    $search['client_id'] = $client_id;
    // $search['payment_status'] = array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_PARTIALLY_REFUNDED);  
    //$search['payment_type'] = array(PAYMENT_TYPE_TRANSFER, PAYMENT_TYPE_CORRECTION);  
    $search['payment_type'] = PAYMENT_TYPE_CORRECTION;  
    //$search['payment_net'] = array('>=', INV_TOLERANCE);  
    $search['withdrawal_id'] = array('=', ATT_DEFAULT_NULL);  // todo: this excludes partial withdrawals
    if($host_id) $search['host_id'] = $host_id;
    if($currency) $search['payment_currency'] = $currency;

    $search_json = json_encode($search);    
    $scope->correction_url = "/ajax.php?oper=get-rows&obj_type=payment&filters=".urlencode($search_json);
    
    $hostsAr = array();
    $hostsAr[0] = "Select Host...";
    if($scope->host_ids = $host_ids = find_objects('property', array('client_id' => $client_id, 'type' => $ptype, 'active' => 1), array('fields' => "distinct host_id"))) {
      $hosts = find_objects('user', array("{$tbl}.id" => $host_ids), array('fields' => $user_fields));
      foreach($hosts as $host) $hostsAr[$host->id] = $host->name;
      $scope->hosts = $hostsAr;
    }
    if($system_user) $scope->paypal_balance = $balance = paypal_get_balance();
    
  }
  
  return;
}

function controller_dashboard($app, $options = []) {
  $scope = $app->page->scope;
  $siteObj = $app->site;

  $inbox_scope = controller_inbox($app, $options);
  $scope->threads = $inbox_scope->threads;
  
  if($siteObj->property_site) {
    $bookings_scope = controller_bookings($app, $options);
    foreach($bookings_scope as $k => $v) $scope->$k = $v;
  }
  
  //$scope->debug = "Dashboard: Coming soon!";
  return;
}

function controller_bookings($app, $options = []) {
  global $u_id, $u_level;
  $pageObj = $app->page;
  $__header = $app->header;   
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js"; /** todo: support other languages, use $app->locale  */    
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";
    
  $scope = $app->page->scope;
  // redirect_secure();

  $errors = $messages = array();

  $slug = $app->slug;  
  $obj_type = $scope->obj_type = "reservation"; 
  
  $all_hosts = 0;
  if($u_level >= USER_LEVEL_STAFF) {
    $host_id = $slug ?: 0;
    if($host_id && !get_user($host_id)) return controller_error($app, "User {$slug} not found");    
  } elseif($u_level >= USER_LEVEL_HOST) {
    $host_id = $slug ?: $u_id;
    $host_ids = user_host_list($u_id);
    if($host_id != $u_id && in_array($host_id, $host_ids)) return controller_error($app, "User {$slug} not authorized");
  } else {
    $host_id = $u_id;
  }
  
  $inline = $pageObj->page_type == PAGE_DASHBOARD;

  // my bookings - or that of user in slug - or all bookings (if staff and no slug)
  $search = array("res_id" => ['>', 0], "type" => ['>', RES_TYPE_ADMIN], "status" => ['!=', [STATUS_DRAFT, STATUS_BLOCKED]]);
  //$search['paid'] = ['>', 0];
  
  if($host_id) $search['host_id'] = $host_id; 

  if($inline) {
    $start = $scope->start = $_GET['start'] ?: 0;
    $limit = $scope->limit = 50;
    $searchOptions = array("fields" => $fields, "start" => $start, "limit" => $limit);
    
    $bookings = find_objects("reservation", $search, $searchOptions);
    $scope->booking_count = count_objects("reservation", $search);
    
    foreach($bookings as $k => $booking) {
      $booking->chat_count = count_objects('chat', ['parent_type' => "reservation", 'parent_id' => $booking->res_id]);;
      $bookings[$k] = $booking;
    }  
    $scope->bookings = $bookings;
  } else {
    $scope->bookings_url = "/ajax.php?oper=get-rows&obj_type=reservation&filters=".urlencode(json_encode($search));
  }
  
  //$scope->booking_count = count($bookings);

  // my trips
  $search = array("guest_id" => $u_id, "status" => ['!=', STATUS_DRAFT]);
  if($inline) {
    $fields = array('res_id','created','status','city_id','city_name','checkin','checkout','apartment_name','currency','grand_total', 'chat_count');
    $start = $scope->start = $_GET['start'] ?: 0;
    $limit = $scope->limit = 50;
    $searchOptions = array("fields" => $fields, "start" => $start, "limit" => $limit);
    $trips = find_objects("reservation", $search, $searchOptions);
    foreach($trips as $k => $trip) {
      $trip->chat_count = count_objects('chat', ['parent_type' => "reservation", 'parent_id' => $trip->res_id]);;
      $trips[$k] = $trip;
    }
    $scope->trips = $trips;
    $scope->trip_count = count_objects("reservation", $search);
  } else {
    $scope->trips_url = "/ajax.php?oper=get-rows&obj_type=reservation&filters=".json_encode($search);
  }
  
  return;
}

/** no security (used in mail templates called from crontab) - copy to booking_status 
    fetch booking, rental, guest, host */
function controller_booking($app, $options = []) {
  global $u_id, $u_level;
  $scope = $app->page->scope;
  $errors = $messages = array();
  print_log("controller_booking:".dump($scope,true), 'app', LOG_LEVEL_TEST);

  $obj_type = $scope->obj_type = "reservation"; 
  $obj_id = $res_id = $scope->obj_id = $scope->res_id = pick_first($scope->res_id ?: $scope->obj_id, $_GET['res_id'] ?: $_GET['obj_id'], $options['res_id'] ?: $options['obj_id'], $app->object_id);
  $obj = $resObj = $res_id ? get_object($obj_type, $obj_id) : ''; // force reload     
  // print_log("controller_booking: res_id=$obj_id".dump($obj,true), 'app', LOG_LEVEL_TEST);

  if(!$resObj) {
    $errors[] = "Booking {$obj_id} not found"; 
    $scope->error = $errors;
    return;
  }
  
  
  $status = $resObj->status;
  $apt_id = $resObj->apt_id;
  $property_type = $resObj->property_type;
  $aptObj = ($apt_id && is_property_type($property_type)) ? get_object($property_type, $apt_id) : null;    
  if(!$aptObj) return controller_error($app, "Rental not found");
  
  $pin = $scope->pin_code = pick_first($_GET['pin'], $options['pin'], $scope->pin, $scope->pin_code);

  $scope->host_id = $host_id = $obj->host_id;
  $scope->is_host = $is_host = $u_id && $host_id && $u_id == $host_id;
  $scope->guest_id = $guest_id = $obj->guest_id;
  $scope->is_guest = $is_guest = $u_id && $guest_id && $u_id == $guest_id;
  $hostObj = $host_id ? get_user($host_id) : null;
  
  if($guest_id) {
    $guestObj = get_user($guest_id);
  } else {
    $guestObj = new stdClass;
    $keys = array('salutation', 'first_name', 'last_name', 'email_address', 'phone', 'city', 'country');
    foreach($keys as $key) $guestObj->$key = $resObj->$key;
    $guestObj->display_name = get_name_string($guestObj);
  }
  
  $scope->id = $res_id = $resObj->res_id;
  $scope->rental = $aptObj;
  $scope->booking = $resObj;
  $scope->guest = $guestObj;
  $scope->host = $hostObj;
  $scope->public_link = booking_link($resObj, true);
  $scope->pin = $pin;

  $scope->type_name = booking_type_name($resObj); 
  // print_log("controller_booking done:".dump($scope,true), 'app', LOG_LEVEL_TEST);
      
  return;
}
  
function controller_booking_status($app, $options = []) {
  global $u_id, $u_level;
  $__header = $app->header; 
  $siteObj = $app->site;
  $ver = $app->version ?: date('md');
  if($u_level >= USER_LEVEL_STAFF) $__header->scripts[] = "/afr/js/admincal-dialog.js?v=$ver"; 
  controller_booking($app, $options);
  $scope = $app->page->scope;  
  if($scope->error) return;
  
  $resObj   = $scope->booking;
  $is_host  = $scope->is_host;
  $is_guest = $scope->is_guest;
  $pin      = $scope->pin_code;
  $res_id   = $resObj->res_id;
  
  if($u_id) { // we're logged in 
    $user_id = $u_id;
    $auth_level = auth_res($resObj);
  } elseif($pin) { // check pin
    $scope->pin_code = $pin;
    $auth_level = myhash($resObj->booking_pin) == $pin ? USER_LEVEL_GUEST : 0;
    if(!$auth_level) $auth_level = myhash_old($resObj->booking_pin) == $pin ? USER_LEVEL_GUEST : 0; // grandfather clause
    if(!$auth_level) {
      $scope->error = "Invalid PIN";
      $scope->template = '403';
      $pin = '';
      return;
    }
  } else { // redirect to login
    redirect_login();
    return;
  }
  
  print_log("res_id:{$res_id} pin:{$pin} auth:{$auth_level}", 'booking', LOG_LEVEL_TEST);
  
  $scope->auth_level = $auth_level;
  $scope->auth_edit = $auth_edit = $auth_level > 0; // can edit?
  if(!$auth_level) {
    $scope->error = "Not authorized";
    $scope->template = '403';
    return;
  }  
  
  $scope->data = array_filter(array("obj_type" => 'reservation', "res_id" => $res_id, "pin"  => $pin)); // send to ajax fetch-template method and back to this controller (in options)
  $scope->is_admin = $is_admin = $u_level >= USER_LEVEL_ADMIN;
  $scope->can_cancel = ($is_guest && in_array($status, [STATUS_INQUIRY, STATUS_REQUEST, STATUS_PROPOSED, STATUS_PENDING, STATUS_RESERVED, STATUS_COMPLETE])) || 
                       ($is_host && in_array($status, [STATUS_PROPOSED, STATUS_PENDING, STATUS_RESERVED, STATUS_COMPLETE])) || 
                       $is_admin;  
  
  $scope->can_alter = 0; /** todo: implement */
  
    
  if($auth_edit) {
    $misArray = reservation_invoice_mismatch($resObj, array('values' => 1));
    if($difference = $misArray['difference']) {
      $scope->mismatch = $misArray;
    }      
  }    
  
  if($errors) {
    $scope->error = $errors;
    $scope->template = "403";    
    return;
  }
    
  //$scope->rate_box = print_rate_box($resObj, $is_host ? USER_LEVEL_OWNER : USER_LEVEL_GUEST); /** get rid of */
  $scope->uploader = object_media_uploader("reservation", $res_id, 'upload');
  
  /** template specific */
  //$scope->view = $template = pick_first($_GET['view'], $_GET['template'], $options['template'], "_booking-status-overview.html");
  //if($template && strpos($template, '_booking-status-') === false) $template = $scope->view = "_booking-status-{$template}.html"; // allow shortform
  
  $obj_type = 'reservation';
  $obj_id = $res_id;
  
  $subview = $app->page->subview ?: 'overview';    
  $app->page->subtemplate = "_booking-status-{$subview}.html";
  switch($subview) {
    case "overview":
      // find invoices and payments
      $invSearch = array();
      $invSearch['parent_type'] = 'reservation';
      $invSearch['parent_id'] = $res_id;  
      $invSearch['status'] = array('>=', INV_STATUS_DRAFT);
      //$fields = array('invoice.id', 'invoice_id', 'type', 'status', 'start_date', 'end_date');
      $scope->invoices = $invObjs = find_objects('invoice', $invSearch, array('sort' => 'type, start_date'));
      $scope->inv_id = $inv_id = count($invObjs) ? $invObjs[0]->id : 0;
      $scope->invoice = $inv_id ? get_object('invoice', $inv_id) : null;
      
      if($auth_level >= USER_LEVEL_STAFF) {
        $scope->change_log = $log = object_change_log($obj_type, $obj_id);
        $scope->mail_log = object_mail_log($obj_type, $obj_id);
      }

      $payments = booking_payments($res_id);
      $resObj->payments = $payments ? payment_rows($payments, $auth_level) : [];
      
      //dump($invObjs);
      break;
    case "edit":
      break;
      
    case "share":
      break;

    case "upload":
      break;
      
    case "payment":
      break;
    
    /** fix */
    case "feedback":
      if($feedback = find_object("feedback", array("parent_type" => "reservation", "parent_id" => $resObj->res_id))) {
        // show existing feedback
        $scope->feedback = $feedback;
        $scope->foo = 'found';

      } elseif($auth_edit) { // show form
        $scope->foo = 'yes';
        $scope->purpose = get_objects_assoc("feedback_purpose", "", array('first' => phrase('select', CAPITALIZE)));
        $scope->profile = get_objects_assoc("feedback_profile", "", array('first' => phrase('select', CAPITALIZE)));
      } else {
        $scope->foo = 'not';
      }
      break;
      
    case "chat":
      $host_id = $resObj->host_id;
      $guest_id = $resObj->guest_id ?: "reservation_".$resObj->res_id;
      
      $scope->sender_id = $user_id;
      $scope->receiver_id = $user_id == $resObj->host_id ? $guest_id : $host_id;
      $scope->parent_type = "reservation";
      $scope->parent_id = $resObj->res_id;
      $scope->user_ids = [$guest_id, $host_id];
      $scope->thread = find_objects("chat", array("parent_type" => "reservation", "parent_id" => $resObj->res_id));
      //dump($scope->chat);
      break;
          
    case "itinerary": /** itinerary */
      $scope->pdf_url = create_pdf_booking($resObj); // always create fresh PDF on view
      break;
      
    case "send": /** proposal */
      if(!in_array($resObj->status, array(STATUS_PROPOSED, STATUS_RESERVED, STATUS_COMPLETE))) {
        $scope->error = "Invalid reservation status";
        $scope->template = '403';
        return;
      }

      list($guest_text, $host_text, $staff_text) = booking_mail_message($resObj);
      
      
      //$mail_scope = ['profile_id' => $resObj->host_id,
      //               'booking' => $resObj, 
      //               'rental' => $aptObj, 
      //               'receiver_name' => $guest_name, 
      //               'receiver_role' => 'guest', 
      //               'receiver_id' => $resObj->guest_id, 
      //               'main_message' => $guest_text];
      //$scope->mail_scope = $mail_scope;            
      
      
      //$body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => $controller]);
      //echo("body = $body");
      
      $mail = new stdClass;
      
      $mail->template = $template = "mail/mail-booking";    
      $mail->controller = $controller = "booking";            
      $mail->from = site_email($resObj->site_id, true) ?: site_email($siteObj);    
      $mail->subject = mail_subject('reservation', $resObj);
      $mail->main_message = $guest_text;

      $guest_name =  $resObj->guest_id ? get_user_name($resObj->guest_id) : get_name_string($resObj);
      $mail->recipient = $mail->to = guest_email($resObj, true) ?: email_recipient($resObj->email_address, $guest_name);    
      $mail->receiver_role = 'guest';
      $mail->receiver_id = $resObj->guest_id;

      $mail->obj_name = 'booking';
      $mail->obj_type = 'reservation';
      $mail->obj_id = $resObj->res_id;
      $mail->action = '';
      $scope->mail  = $mail; 

      break;

    case "recalculate":
      print_log("recalc:", 'invoice', LOG_LEVEL_TEST);
      
      $search = array('res_id' => $res_id, 'status' => array('>=', INV_STATUS_DRAFT));
      $fields = array('invoice.id', 'invoice_id', 'start_date', 'end_date', 'total', 'paid', 'balance', 'sequence', 'locked', 'concerning');
      $options = array('fields' => $fields, 'sort' => 'sequence', 'debug_query' => 0);
      $scope->before = $before = find_objects('invoice', $search, $options);
      $all_ids = [];
      foreach($before as $invObj) $all_ids[] = $invObj->id;
      //foreach($before as &$invObj) $invObj->concerning = "foo";
      //$message = html_h2('before').invoice_debug_print($res_id);
      
      
      $inv_ids = add_reservation_invoices($res_id);
      //dump($inv_ids);
      if($inv_ids === false) {
        $scope->error = "Failed to recalcuate invoices - is this a booking from a remote source? ";
      } else {
        foreach($all_ids as $inv_id) recalculate_invoice($inv_id);
        //$invObjs = find_reservation_invoices($res_id, array('search' => array('type' => INV_TYPE_BOOKING, 'status' => array('>=', STATUS_ALL))));
        //$inv_ids = array_find_key($invObs, 'id');
        
        $scope->after = $after = find_objects('invoice', $search, $options);     
        
        /** calc extras - from update_invoice_total */
        $count = count($all_ids);
        $inv_id_list = $count ? implode(',', $all_ids) : '';
        print_log("recalc: count=$count list=$inv_id_list for $res_id", "invoice", LOG_LEVEL_ALWAYS);  
        if($count) {
          $extra_li_types = implode(',', array(LI_TYPE_PRODUCT, LI_TYPE_SERVICES, LI_TYPE_EXTRAS, LI_TYPE_CATERING));
          $qs = "SELECT sum(total) FROM inv_lineitem WHERE inv_id IN ($inv_id_list) AND type IN ($extra_li_types)";
          $extras = db_getOne($qs); 
          print_log("recalc: update_invoice_total: other charges=$extras from $inv_id_list for $res_id", "invoice", LOG_LEVEL_ALWAYS);  
          print_log("qs: $qs", "invoice", LOG_LEVEL_ALWAYS);  
          db_query("UPDATE reservation SET other_charges = '$extras' WHERE res_id=$res_id");
          booking_update_total($res_id);
        }
        
        $resObj = get_object('reservation', $res_id);
        
      }
       
    case "cancel":
      if(!$is_host && !$is_guest && !$is_admin) {
        $scope->error = "Only guest and host can cancel a booking";
        $scope->template = "404";
      }
      break;

    default:
      return controller_error($app, "Unknown subview {$subview}");
      break;
  }
  
  $scope->booking = $resObj;
  return;
}

function controller_payment_status($app, $options = []) {
  $scope = $app->page->scope;
  $options['prefix'] = "_payment";
 
  /** normally we use invoice ID, but for mails, we pass payment ID */
  if($payment_id = $scope->payment_id) {
    $paymentObj = $scope->payment = get_object('payment', $payment_id);
    $scope->inv_id = $paymentObj->inv_id;
  }
  
  controller_invoice_status($app, $options); // much the same
  return;
}

/** base controller, also used by mail */ 
function controller_payment($app, $options = []) {
  $scope = $app->page->scope;
  $options['prefix'] = "_payment";
 
  /** normally we use invoice ID, but for mails, we pass payment ID */
  if($payment_id = $scope->payment_id) {
    $paymentObj = $scope->payment = get_object('payment', $payment_id);
    $scope->inv_id = $paymentObj->inv_id;
  }
  controller_invoice($app, $options);
  return;
}

/** base controller, also used by mail */ 
function controller_invoice($app, $options = []) {
  $scope = $app->page->scope;  
  
  $errors = $messages = array();
  $obj_type = $scope->obj_type = "invoice"; 
  $obj_id = $inv_id = $scope->obj_id = $scope->inv_id = pick_first($scope->inv_id ?: $scope->obj_id, $_GET['inv_id'] ?: $_GET['obj_id'], $options['inv_id'] ?: $options['obj_id'], $app->object_id);
  $pin = pick_first($_GET['pin'], $options['pin'], $scope->pin, $scope->pin_code);
  $obj = $invObj = $inv_id ? get_object($obj_type, $obj_id) : '';  

  print_log("controller_invoice: inv_id={$inv_id} pin={$pin} id: {$invObj->id}".dump($scope,true), 'app', LOG_LEVEL_TEST);
  
  if(!$invObj) return controller_error("Invoice not found", '403');

  $scope->obj_id = $obj_id;
  $scope->obj_type = $obj_type;
  $scope->obj = $scope->invoice = $obj;
  $scope->pin = $pin;
  $scope->host_id = $host_id = $obj->host_id;
  $scope->is_host = $is_host = $u_id && $host_d && $u_id == $host_id;
  $scope->guest_id = $guest_id = $obj->guest_id;
  $scope->is_guest = $is_guest = $u_id && $guest_id && $u_id == $guest_id;
  return;
}

function controller_invoice_status($app, $options = []) {
  global $u_id, $u_level, $__LIVE;
  $__header = $app->header; 
  $__header->scripts[] = "/afr/bower/jquery-file-download/src/Scripts/jquery.fileDownload.js";
  $__header->styles[] = "/afr/css/print.css";
  
  $errors = $messages = array();

  $siteObj = $app->site;
  
  controller_invoice($app, $options);
  $scope = $app->page->scope;  
  if($scope->error) return;
  
  $obj_type = $scope->obj_type; 
  $obj_id = $inv_id = $scope->inv_id = $scope->obj_id;
  $obj = $invObj = $scope->obj;  
  $pin = $scope->pin;
  $host_id = $scope->host_id;
  $is_host = $scope->is_host;
  $guest_id = $scope->guest_id;
  $is_guest = $scope->is_guest;
  $site_id = $siteObj->id;


  /** security */
  if($u_id) { // we're logged in 
    $user_id = $u_id;
    $auth_level = auth_invoice($invObj);
  } elseif($pin) { // check pin
    $scope->pin_code = $pin;
    $auth_level = myhash($invObj->booking_pin) == $pin ? USER_LEVEL_GUEST : 0;
    print_log("gh pin={$pin} auth:{$auth_level} id: {$invObj->id}", 'invoice', LOG_LEVEL_TEST);
    if(!$auth_level) $auth_level = myhash_old($invObj->booking_pin) == $pin ? USER_LEVEL_GUEST : 0; // grandfather clause
    if(!$auth_level) return controller_error("Invalid PIN", '403');
  } else { // redirect to login
    redirect_login();
    return;
  }

  print_log("ok pin={$pin} auth:{$auth_level} id: {$invObj->id}", 'invoice', LOG_LEVEL_TEST);

  $scope->auth_level = $auth_level;
  $scope->data = array_filter(array("obj_type" => 'invoice', "inv_id" => $inv_id, "pin"  => $pin)); // send to ajax fetch-template method and back to this controller (in options)
  
  if(!$auth_level) {
    $scope->error = "Unauthorized";
    $scope->template = '403';
    return;
  }
  
  
  $scope->auth_edit_invoice = $scope->auth_invoice = $auth_edit = $auth_level >= USER_LEVEL_STAFF; // can edit?
  $scope->show_contact = $auth_edit || $is_guest || $pin; 
  
  if($auth_edit) {
    $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js";
    $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js"; /** todo: support other languages, use $app->locale  */
    $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/editable/bootstrap-table-editable.js";
    $__header->scripts[] = "/afr/bower/x-editable/dist/bootstrap3-editable/js/bootstrap-editable.js";
    
    $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css";
    $scope->data_url = "/ajax.php?oper=get-rows&obj_type=inv_lineitem&search=$inv_id"; //&return=rows";
    $scope->edit_url = "/ajax.php?oper=edit&obj_type=inv_lineitem";
    $scope->title = "Edit invoice ".$invObj->invoice_number ?: $inv_id;
    
    
    $scope->inv_ids = $invObj->sequence ? find_objects('invoice', ['parent_type' => $invObj->parent_type, 'parent_id' => $invObj->parent_id], ['field' => 'invoice.id']) : [];
    $org_id = $invObj->org_id;
    $contact_id = $invObj->contact_id;
    $scope->contact = $contact_id ? get_object('contact', $contact_id) : null;
    $scope->contacts = $org_id ? find_objects('contact', ['org_id' => $org_id, 'active' => 1], ['sort' => 'organization, first_name']) : [];
    
  }

  $scope->public_link = $public_link = invoice_link($invObj, true);

  $parent_type = $invObj->parent_type;
  $parent_id = $invObj->parent_id;
  $guest_id = 0;
  $le_id = $invObj->legal_entity_id;
  $scope->legal_id = $le_id;
  $scope->reeleezee_username = $le_id ? get_object('legal_entity', $le_id, 'reeleezee_username') : '';
  
  $resObj = null;
  if($parent_type && $parent_id) {
    $parentObj = get_object($parent_type, $parent_id);
    if($parent_type == 'reservation') $resObj =  $parentObj;
    $scope->type_name = booking_type_name($resObj);     
  }
  $aptObj = $resObj = null;
  switch($parent_type) {
  case 'reservation':
    $scope->booking = $resObj = $parentObj;  
    $scope->due_now = $invObj->due_now = get_due_now($invObj, $resObj);   // how much is due now (either full amount or percentage of total
    //echo("Due = $due_now");
    $apt_id = $resObj->apt_id;
    $ptype = $resObj->property_type;
    $scope->rental = $aptObj = $apt_id && $ptype ? get_object($ptype, $apt_id) : null;
    $guest_id = $resObj->guest_id; 
    break;
  default:
    break;
  }

  //$scope->view = $template = pick_first($_GET['view'], $_GET['template'], $options['template'], "_invoice-overview.html");
  //if($template && strpos($template, '_invoice-') === false) $template = $scope->view = "_invoice-{$template}.html"; // allow shortform
  $ctrl = $app->page->controller;
  if($ctrl == 'payment_status') { /** this controller is also used for payments */
    $prefix = 'payment';
    $default_subview = 'payment';
  } else {
    $prefix = 'invoice';
    $default_subview = 'overview';
  }

  $subview = $app->page->subview ?: $default_subview; 
  $app->page->subtemplate = "_{$prefix}-{$subview}.html";
  
  print_log("controller_invoice_status: inv_id={$inv_id} pin={$pin} id={$invObj->id} ctrl={$ctrl} subview={$subview} prefix={$prefix}", 'app', LOG_LEVEL_TEST);
  
  switch($subview) {
  case "overview":
    if($auth_level >= USER_LEVEL_STAFF) {
      $scope->change_log = object_change_log($obj_type, $obj_id);
      $scope->mail_log = $mail_log = object_mail_log($obj_type, $obj_id);
    }
  
    break;
  case "receive":
    $paymentTypes = array(PAYMENT_TYPE_CREDIT_CARD, PAYMENT_TYPE_CASH, PAYMENT_TYPE_BANK, PAYMENT_TYPE_PAYPAL, PAYMENT_TYPE_AGENT);
    $scope->payment_types = find_objects_assoc('payment_type', array('id' => $paymentTypes), array('first' => phrase('select', CAPITALIZE).'...'));
    $scope->payment_statuses = array(PAYMENT_STATUS_COMPLETED => 'Completed', PAYMENT_STATUS_PENDING => 'Pending');
    $scope->currencies = get_currencies();
    break;
  case "export":
    break;
  case "recalculate":
    $res_id = $invObj->res_id;
    $scope->invoice_before = invoice_rows($inv_id);
    recalculate_invoice($inv_id);
    $scope->invoice_after = invoice_rows($inv_id);
    break;
    
  case "payment":
    
    /** set up Mollie, PayPal */
    $live = $app->live || $app->live_payments_test;
    $scope->paypal_key = $pp_key = get_config(($live ? "ppu" : "pput"), $siteObj->id);
    $scope->mollie_key = $mollie_key = get_config(($live ? "ml" : "mt"), $siteObj->id);
    $scope->mollie_methods = $mollie_methods = mollie_methods($mollie_key);    

    /** look for PayPal return */
    if($scope->txn_id = $_GET['tx']) {
      $scope->tx_st = $_GET['st'];
      $scope->tx_currency = $_GET['cc'];
      $scope->tx_amount = $_GET['amt'];
    }
    
    if($resObj && ($resObj->status == STATUS_RESERVED || $resObj->status == STATUS_COMPLETE) && $invObj->head_invoice && $invObj->type == INV_TYPE_BOOKING) { 
      $template = "mail/booking-mail";
      $guest_text = "this is a test";
      $mail_scope = ['receiver' => 'guest', 'profile_id' => $resObj->host_id, 'res_id' => $resObj->res_id, 'title' => "{$dear} {$guest_name},", 'personal_message' => $guest_text];
      $mail_template_options = [
        'scope' => $mail_scope, 
        'controller' => 'booking_status', 'replyto' => "noreply@{$siteObj->domain}"];
      //$body = smarty_mail_template($siteObj, $template, $mail_template_options);      
      
      $scope->mail_template = $template;    
      $scope->mail_controller = 'booking_status';    
      $scope->sender = $from;    
      $scope->receiver = $to;    
      $scope->subject = booking_mail_subject($resObj);    
      $scope->preview = $body;
    }
    
    /**
    $ccObjs = array();    
    if($u_id == $guest_id) { // user is paying
      $ccObjs = cc_user_cards($guest_id);  
    } elseif($auth_edit) { // owner, staff
      $ccObjs = cards_on_file(array('parent_type' => $parent_type, 'parent_id' => $parent_id));
    }
    if(count($ccObjs)) $scope->cards = cc_cards_printable($ccObjs);
    */
    break;
  case "print":
    
    purge_invoices($inv_id);
    //dump($invObj);
    $scope->pdf_url = $pdf_url = create_pdf_invoice($invObj); // always create fresh PDF on view
    print_log("url of {$invObj->id}= {$pdf_url}", 'invoice', LOG_LEVEL_ALWAYS);
    break;
    
  case "send":
    $invoice_id_required = get_object('legal_entity', $invObj->legal_entity_id, 'invoice_id_required') ? 1 : 0;
    if($invoice_id_required && !trim($invObj->invoice_id)) {
      $errors[] = "An Invoice ID is required before sending this invoice";
      if(1) {
        $export_link = html_link(invoice_link($invObj)."/export", "export", ['class' => 'click-target', 'data-target' => '#invoice_export_link']);
        $errors[] = "Perhaps you need to $export_link the invoice first?";
      }
      $scope->error = $errors;
      return;
    }

    $scope->mail_log = $mail_log = object_mail_log($obj_type, $obj_id);
    
    $pdf_url = create_pdf_invoice($invObj); // always create fresh PDF on view   

    $guest_text = phrase('you have a new invoice from %s (attached)', CAPS, $siteObj->name);
    $guest_name =  $invObj->guest_id ? get_user_name($invObj->guest_id) : get_name_string($invObj);
    $recipient = guest_email($invObj, true) ?: email_recipient($invObj->email_address, $guest_name);
    $guest_name = "People who pay";    
    $site_name = str_replace(' ', '-', $siteObj->name);
    $invoice_id = $invObj->invoice_id ?: $invObj->id;
    
    
    $mail = new stdClass;
    
    $mail->template = $template = "mail/mail-invoice";    
    $mail->controller = $controller = "invoice";            
    $mail->sender = site_email($invObj->site_id, true) ?: site_email($siteObj);    
    $mail->recipient = $recipient;
    $mail->receiver_name = $guest_name;
    $mail->subject = mail_subject('invoice', $invObj);
    $mail->body = $body;
    $mail->main_message = phrase('you have a new invoice from %s (attached)', CAPS, $siteObj->name);
    $mail->personal_message = ""; 
    $mail->attachment = docroot().$pdf_url;
    $mail->attachment_name = str_replace(' ', '_', "$site_name invoice $invoice_id").'.pdf';
    $mail->obj_name = 'invoice';
    $mail->obj_type = 'invoice';
    $mail->obj_id = $invObj->id;

    $scope->mail  = $mail; 
    
    break;
  case "share":
    break;
    
  default:
    return controller_error($app, "Unknown subview {$subview}");
    break;
  }
  
  $payments = invoice_payments($inv_id);
  $invObj->payments = $payments ? payment_rows($payments, $auth_level) : [];

  $scope->invoice_rows = invoice_rows($invObj, $auth_level);  
  $scope->invoice = $invObj;  
  return;
  
}

function controller_forgot_password($app, $options = []) {
  global $u_id;
  $scope = $app->page->scope;
  $errors = array();
  
  if($guid = $_GET['guid']) {

    /** new: using phpauth */
    $phpauth = user_phpauth();    
    $result = $phpauth->verifyKey($guid);
    if($result['error']) {
      $errors[] = $result['message'];
    } else {
      $prObj = new stdClass();
      $prObj->id = $result['id'];
      $prObj->user_id = $result['user_id'];
      $prObj->guid = $guid;
      $scope->reset = $prObj;
    }

    if($errors) {
      $scope->error= $errors;
      $scope->template = '403';      
      return;
    }
    $scope->message = $messages;  
    $scope->template = "reset-password.html";
  }

  $scope->user_id = $u_id;
  return;
}


function controller_user_account($app, $options = []) {
  global $u_id, $u_level;
  $__header = $app->header; 
  $__header->scripts[] = "/afr/bower/iban/iban.js";
  $siteObj = $app->site;
  
  //print_log("controller_user_account 1 u_id=$u_id", 'smarty', LOG_LEVEL_TEST);
  $scope = $app->page->scope;
  if(!$u_id) {
    redirect_login();
    //$scope->redirect = site_pagetype_link($siteObj, PAGE_LOGIN)."?ref=".site_pagetype_link($siteObj, PAGE_PROFILE);
    return;
  }
  
  $obj_type = $scope->obj_type = $app->object_type;
  $obj_id = $scope->obj_id = $app->object_id;
  $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : null;

  if($obj_type == 'user' && $obj_id && $obj) { // from slug
    $user_id = $obj_id;
    $userObj = $obj;
  } else {
    $user_id = $u_id;
    $userObj = $app->user ?: get_object('user', $user_id); // logged in user, already merged with profile
  }
  // for now
  // later (allow admins to edit other profiles): $scope->auth_edit = $auth_edit = auth_action('user', 'edit', $u_id);

  $scope->profile = $scope->user = $userObj;
  $scope->user_id = $userObj->id;
  $scope->auth_edit = $scope->auth_edit_account = $auth_edit = auth_user_account($user_id);
    
  if(!$auth_edit) {
    $scope->error = "Unauthorized";
    $scope->template = "404";
    return;
  }
  
  //$scope->view = $template = pick_first($_GET['view'], $_GET['template'], $options['template'], "_user-account-notifications.html");
  //if($template && strpos($template, '_user-account-') === false) $template = $scope->view = "_user-account-{$template}.html"; // allow shortform

  // public
  // ["notifications","security","privacy","password","linking","payout","transactions"]
  $subview = $app->page->subview ?: 'notifications';    
  $app->page->subtemplate = "_user-account-{$subview}.html";
  switch($subview) {
    case "notifications":
      break;
    case "security":
      break;
    case "privacy":
      break;
    case "password":
      break;
    /**
    case "cards":
      $ccObjs = cc_user_cards($user_id);  
      $scope->cards = cc_cards_printable($ccObjs);      
      break;
      */
    case "linking":
      $scope->link_requests = find_objects('user_map', ['host_user_id' => $user_id, 'verified' => 0]);
      $scope->linked_users  = find_objects('user_map', ['user_id' => $user_id]);
      $scope->auth_users    = find_objects('user_map', ['host_user_id' => $user_id, 'verified' => 1]);
      break;
    case "payout":
      break;
    case "transactions":
      $start = $scope->start = $_GET['start'] ?: 0;
      $limit = $scope->limit = 10;
      $scope->transactions = find_objects('payment', array("host_id" => $u_id));

      //$fields = array('res_id','created','status','city_id','city_name','checkin','checkout','apartment_name','currency','grand_total', 'chat_count');
      $fields = '';
      $searchOptions = array("fields" => $fields, "start" => $start, "limit" => $limit);
      $search = array("host_id" => $user_id, "status" => array(STATUS_COMPLETEED));
      $scope->transactions = find_objects("payment", $search, $searchOptions);
      $scope->transaction_count = count_objects("payment", $search);
      break;
    default:
      return controller_error($app, "Unknown subview {$subview}");
      break;
  }
  //dump($scope);
  return;  
}

/** new public: view user profile */
function controller_user_profile($app, $options = []) {
  //global $u_id, $u_level;
  $u_id = $app->user->id;
  //dump($app);
  
  $__header = $app->header; 
  $__header->scripts[] = "/afr/bower/lightslider/dist/js/lightslider.min.js";
  $__header->styles[] = "/afr/bower/lightslider/dist/css/lightslider.min.css";
  $siteObj = $app->site;
    
  if($providers = $app->scope->provider_keys) {
    foreach($providers as $provider_id) { 
      if(in_array($provider_id, $providers)) $scope->provider_profiles[$provider_id] =  find_object('provider_profile', array('site_id' => $siteObj->id, 'user_id' => $user_id, 'provider_id' => $provider_id));
    }
  }

  $scope = $app->page->scope;
  
  $obj_type = $scope->obj_type = $app->page->object_type ?: 'user';
  $obj_id = $scope->obj_id = $app->page->object_id;
                      
  //die("t={$obj_type} id={$obj_id}");
  $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : null;
  
  if($obj_type == 'user' && $obj_id && $obj) { // from slug
    $user_id = $obj_id;
    $userObj = $obj;
    if(!$userObj->active) {
      if($u_level < USER_LEVEL_STAFF) return controller_error($app, "User not found", 404);
      $scope->message = "User not active";
    }
  } else {
    //if(!$u_id) {          
    //  $scope->error = "Missing User ID and not logged in";
    //  $scope->template = "404";
    //  return;
    //}
    $user_id = $u_id;
    $userObj = $app->user ?: get_object('user', $user_id); // logged in user, already merged with profile
  }
  
  //dump($userObj);
  // for now     
  // later (allow admins to edit other profiles): $scope->auth_edit = $auth_edit = auth_action('user', 'edit', $u_id);

  $scope->profile = $scope->user = $userObj;
  $scope->user_id = $userObj->id;
 
  $scope->auth_edit = $auth_edit = $scope->auth_edit_user = auth_user_profile($user_id);
  
  $scope->auth_login = auth_login($user_id) && $u_id != $user_id;
  
  //die("ctrl: {$app->page->controller} tpl {$app->page->template}");
  //die("u_id={$u_id} user_id={$user_id} edit={$auth_edit} al=".$scope->auth_login. " e=".$scope->auth_edit_user);
  
  /** property sites */
  if($ptype = $siteObj->property_type) {
    $scope->guest_reviews = find_objects("feedback", array("parent_type" => "reservation", "host_id" => $user_id, "published" => 1));
    $scope->host_reviews  = find_objects("feedback", array("parent_type" => "user", "parent_id" => $user_id, "published" => 1));
    $scope->host_listings = $host_listings = find_objects($ptype, array("host_id" => $user_id, "active" => 1), ["limit" => 25, "fields" => ['id', 'active', 'guid', 'type','name', 'media', 'address_city', 'address_country']]);
    //dump($host_listings);
    $scope->host_listings_count = count_objects($ptype, array("host_id" => $user_id, "active" => 1));
  }
  
  /** todo: art website or override */
  
  return;
}

/** new: edit user profile */
function controller_edit_profile($app, $options = []) {
  global $u_id, $u_level;
  $__header = $app->header; 
  $siteObj = $app->site;
  
  $__header->scripts[] = "/afr/bower/bootstrap-tags/dist/js/bootstrap-tags.min.js";
  $__header->scripts[] = "/afr/bower/geocomplete/jquery.geocomplete.min.js";
  $__header->styles[] = "/afr/bower/bootstrap-tags/dist/css/bootstrap-tags.css";

  $__header->scripts[] = "/afr/js/afr_gmap.js";

  $ptype = $siteObj->property_type ?: "";
  $scope = $app->page->scope;
  $scope->path = this_path();
  
  $obj_type = $scope->obj_type = $app->object_type;
  $obj_id = $scope->obj_id = $app->object_id;
  $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : null;

  if($obj_type == 'user' && $obj_id && $obj) { // from slug
    $user_id = $obj_id;
    $userObj = get_user($obj_id);
  } else {
    $userObj = get_user($u_id); 
    $user_id = $u_id;
  }

  $scope->profile = $scope->user = $userObj;
  $scope->user_id = $userObj->id;
  $scope->auth_edit = $auth_edit = $scope->auth_edit_user = auth_user_profile($user_id);

  //die("edit $user_id = $auth_edit");

  if(!$auth_edit) {
    $scope->error = "Unauthorized $user_id";
    $scope->template = "404";
    return;
  }
  
  //$scope->view = $template = pick_first($_GET['view'], $_GET['template'], $options['template'], "_user-profile-overview.html");
  //if($template && strpos($template, '_user-profile-') === false) $template = $scope->view = "_user-profile-{$template}.html"; // allow shortform
  
  if($provider_id = $_GET["provider_id"]) {
    $result = user_provider_profile($siteObj, $provider_id, $u_id);
    if($result->success) {
      $profileObj = $result->data;
      $scope->debug = dump($profileObj, true);
    } else {
      $scope->error = $result->error;
    }
    $subview = 'verification';
  } else {
    $subview = $app->page->subview ?: 'overview';    
  }

  $scope->uploader = user_profile_uploader($user_id, array("view" => "list", "handler" => "user_photo_handler")); // doesn't work in switch, as dependencies are not loaded      
  $scope->passport_uploader = user_profile_uploader($user_id, array("view" => "list", "field" => "passport", "resize" => 0)); // doesn't work in switch, as dependencies are not loaded      

  $app->page->subtemplate = "_user-profile-edit-{$subview}.html";
  
  switch($subview) {
    case "artist":
    case "gallery":
    case "collector":
    case "overview":
    case "settings":
    case "photos":
      break;
    case "message":
      $inbox_scope = controller_inbox($app, $options);
      
      if($user_id == $u_id) {
        $scope->error = phrase('you cannot send a message to yourself', CAPITALIZE);
      }
      $scope->parent_type = $_GET['parent_type'];
      $scope->parent_id = $_GET['parent_id'];
      $scope->sender_id = $u_id;
      $scope->receiver_id = $user_id;
      $scope->form_redirect = $scope->thread_id = $thread_id = chat_thread_id((array) $scope);
      break;
    case "history":
      $fields = array('res_id','created','status','city_id','city_name','checkin','checkout','apartment_name','currency','grand_total', 'chat_count');
      $start = $scope->start = $_GET['start'] ?: 0;
      $limit = $scope->limit = 10;
      $search = array("guest_id" => $user_id, "status" => ['!=', STATUS_DRAFT]);
      $searchOptions = array("fields" => $fields, "start" => $start, "limit" => $limit);
      $trips = find_objects("reservation", $search, $searchOptions);
      foreach($trips as $k => $trip) {
        $trip->chat_count = count_objects('chat', ['parent_type' => "reservation", 'parent_id' => $trip->res_id]);;
        $trips[$k] = $trip;
      }
      
      $scope->trips = $trips;
      $scope->trip_count = count_objects("reservation", $search);
      break;
    case "bookings":
      $fields = array('res_id','created','status','city_id','city_name','checkin','checkout','apartment_name','currency','grand_total', 'chat_count');
      $start = $scope->start = $_GET['start'] ?: 0;
      $limit = $scope->limit = 10;
      $searchOptions = array("fields" => $fields, "start" => $start, "limit" => $limit);
      $host_ids = user_host_list($user_id) ?: $user_id;
      $search = array("host_id" => $host_ids, "status" => array('<>', array(STATUS_DRAFT, STATUS_BLOCKED)));
      $scope->bookings = find_objects("reservation", $search, $searchOptions);
      $scope->booking_count = count_objects("reservation", $search);
      break;
    case "wishlist":
      $search = array("site_id" => $siteObj->id, "user_id" => $user_id);
      $wishlistObjs = find_objects("wishlist", $search) ?: array();
      foreach($wishlistObjs as &$wishlistObj) {
        $wishlistObj->obj = get_object($wishlistObj->obj_type, $wishlistObj->obj_id);
      }
      $scope->wishlist = $wishlistObjs;
      break;
    case "inbox":
      $inbox_scope = controller_inbox($app, $options);
      $scope->threads = $inbox_scope->threads;
      $scope->inv_id = $inbox_scope->inv_id;
      break;
    case "verification":
      if($providers = $app->scope->provider_keys) {
        foreach($providers as $provider_id) { 
          if(in_array($provider_id, $providers)) $scope->provider_profiles[$provider_id] =  find_object('provider_profile', array('site_id' => $siteObj->id, 'user_id' => $u_id, 'provider_id' => $provider_id));
        }
      }
      break;

    default:
      return controller_error($app, "Unknown subview {$subview}");
      break;
  }

  
   //$scope->message = "u_id=$u_id user_id=$user_id auth=$auth_edit se={$scope->auth_edit} t2=$template";
   //$scope->auth_edit = 111;
   //$scope->auth_edit_user = 112;
   //return;
   //die("gh3");

  //echo("o=$obj_id t=$template".dump($scope,true)); 
  return;    
}

function controller_artist_profile($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";

  $scope = $app->page->scope;
  
  controller_user_profile($app,$options);
  //dump($scope);
  $obj_type = $scope->obj_type = $scope->obj_type ?: $app->object_type;
  $obj_id = $scope->obj_id =  $scope->obj_id ?: $app->object_id;
  $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : null;
  
  if(!$obj_id) $obj_id = $u_id;
  
  //echo("ot=$obj_type id=$obj_id");
  //die('gh');
  if($obj_type == 'user' && $obj_id) { // from slug
    $user_id = $obj_id;
    $userObj = get_user($obj_id);
  } else {
    $userObj = $app->user; // logged in user, already merged with profile
    $user_id = $u_id;
  }
  
  $scope->profile = $userObj;
  $scope->auth_edit = $auth_edit = auth_user_profile($user_id);
  $scope->auth_login = auth_login($user_id);
  
  $scope->auth_edit = $scope->auth_edit_user = $auth_edit; // scope auth_edit overrridden somewhere
  //die("AL=".$scope->auth_login." edit=".$auth_edit);

  // $al = auth_login($user_id);
  // echo("<br><br><br><br>user_id=$user_id u_id=$u_id auth_edit: $auth_edit al: $al");  
  // if($auth_edit) return controller_traveler_profile($app, $options);
  
  $is_curator = $user_id && user_has_role($user_id, USER_ROLE_CURATOR);
  $is_artist = $user_id && user_has_role($user_id, USER_ROLE_ARTIST);

  // find curators  
  $scope->curator_ids = find_objects('media_collection_map', array('artist_id' => $user_id, 'curator_id' => array('>', 0)), array('fields' => 'distinct curator_id', 'debug_query' => 0));
  //dump($curator_ids);

  $start = 0;
  $limit = 12;
  
  $url = $scope->url ?: this_url();
  if($is_curator) {
    $scope->template = "user-profile-curator.html";
    $scope->curator_id = $curator_id = $user_id;
    $scope->curator_exhibitions = $curator_exhibitions = find_objects('media_collection', 
      array('user_id' => $curator_id, 'type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'active' => 1), ['start' => $start, 'limit' => $limit]);
    $scope->curator_artworks = gallery_unique_artworks(array_find_key($curator_exhibitions, 'id'), ['start' => $start, 'limit' => $limit]);
  } elseif($is_artist) {
    $scope->template = "user-profile-artist.html";    

    $scope->artist_id = $artist_id = $user_id;
    $scope->artist_projects = $artist_projects = find_objects('media_collection', 
      array('user_id' => $artist_id, 'type' => MEDIA_COLLECTION_TYPE_PROJECT, 'active' => 1), ['start' => $start, 'limit' => $limit]);;
    if($project_ids = array_find_key($artist_projects, 'id')) {
      $scope->artist_artworks = gallery_unique_artworks($project_ids, ['start' => $start, 'limit' => $limit]);          
      //$scope->artist_artworks = $artist_artworks = find_objects('media_collection_map', array('media_collection_id' => $project_ids, 'active' => 1));
    }
    $artist_exhibitions_ids = find_objects('media_collection_map', 
      array('artist_id' => $artist_id, 'collection_type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'active' => 1), 
      array('fields' => 'distinct media_collection_id', ['start' => $start, 'limit' => $limit]));

    $scope->artist_exhibitions = find_objects('media_collection', 
      array('id' => $artist_exhibitions_ids, 'active' => 1), ['start' => $start, 'limit' => $limit]);
  } else {
    $scope->template = "user-profile-collector.html";
  }
  $display_name = $title = $userObj->display_name;
  
  $img_src = print_media($userObj->photo, array('source_only' => true, 'full' => true));
  /** Add facebook Open Graph tags */
  $__header->meta[] = array('property' => 'og:type', 'content' => 'article');
  $__header->meta[] = array('property' => 'og:image', 'content' => add_https($img_src));
  $__header->meta[] = array('property' => 'og:url', 'content' => $url);
  $__header->meta[] = array('property' => 'og:title', 'content' => $display_name);
  $__header->meta[] = array('property' => 'og:description', 'content' =>  strip_tags($userObj->description));    
  $__header->meta[] = array('property' => 'article:author', 'content' =>  "https://www.facebook.com/vpatinabv");    

  $__header->link[] = array('property' => 'image_src', 'content' =>  add_https($img_src));    
  $app->page->author = $display_name;    
  $app->page->description = strip_tags($userObj->description);   
  $app->page->title = $title;    
  
  $app->page->template = $scope->template;
  //die("ctrl: {$app->page->controller} tpl {$app->page->template}");
  
  
  return;  
}                           



function controller_inbox_item($app, $options=[]) {
   $scope->foo = "bar";
   return;
}

/** only used for mail */
function controller_chat($app, $options=[]) {
  global $u_id;
  if(!$u_id) return;
  $user_id = $u_id;  
  
  $scope = $app->page->scope;
  $chat_id = pick_first($scope->chat_id, $option['chat_id'], $scope->obj_id, $option['obj_id']);
  $receiver_id = pick_first($scope->receiver_id, $option['receiver_id']); // to whom this mail is sent
  
  $scope->messages = [];
  if($chat_id) { /** todo: security */
    $chatObj = $scope->chat = get_object('chat', $chat_id);
    $sender_id = $chatObj->sender_id;
    $scope->sender = get_user($sender_id);
    $scope->parent_type = $parent_type = $chatObj->parent_type;
    $scope->parent_id   = $parent_id = $chatObj->parent_id;
    if($parent_type && $parent_id) {
      $parentObj = get_object($parent_type, $parent_id);
      
      switch($parent_type) {
      case 'reservation':
        $scope->booking = $parentObj;
        $scope->rental = get_object('property', $parentObj->apt_id);
        $scope->profile_id = $receiver_id == $parentObj->host_id ?  $parentObj->guest_id : $parentObj->host_id;
        
        break;
      default:
        break;
      }
    }
    
    $thread_id = $chatObj->thread;
    $scope->limit = $limit = 10;
    $scope->count = count_objects("chat", array("thread" => $thread_id));
    $chatObjs = find_objects("chat", array("thread" => $thread_id), array("sort" => "created DESC", "limit" => $limit));
    $scope->thread_id = $thread_id;
    $scope->thread = $chatObjs;
  }
  return;
}

/** used for inbox page */
function controller_inbox($app, $options=[]) {
  global $u_id, $u_level;
  $scope = $app->page->scope;
  if(!$u_id) return;
  $user_id = $u_id;  

  $thread_id = pick_first($scope->thread, $option['thread'], $app->slug);

  if($thread_id) { // get one thread
    $user_ids = explode('-', $thread_id);
    if(count($user_ids) < 2 || ($u_level < USER_LEVEL_STAFF && !in_array($u_id, $user_ids))) {
      $scope->error = "Invalid thread";      
      return;      
    }

    
    $chatObjs = find_objects("chat", array("thread" => $thread_id), array("sort" => "created DESC", "limit" => 0));

    
    if($scope->count = $count = count($chatObjs)) {
      $chatObj = $chatObjs[0]; // get first

      $scope->chat = $chatObj;
      
      $scope->parent_type = $parent_type = $chatObj->parent_type;
      $scope->parent_id   = $parent_id = $chatObj->parent_id;
            
      if($parent_type != 'reservation') { // look for any chat with reservation in this thread
        if($res_id = find_object("chat", array("thread" => $thread_id, "parent_type" => "reservation"), "parent_id")) {
          $scope->parent_type = $parent_type = 'reservation';
          $scope->parent_id   = $parent_id = $res_id;
        }
      }
          
      if($parent_type && $parent_id) $scope->parent = $parentObj = get_object($parent_type, $parent_id);
      
      switch($parent_type) {
      case 'reservation':
        $scope->booking = $resObj = $parentObj; // get_object("reservation", $parent_id);
        $scope->invoice = $invObj = booking_invoice($resObj);
        $scope->inv_id = $invObj->id;
        if($resObj->property_type && $resObj->apt_id) $scope->rental = get_object($resObj->property_type, $resObj->apt_id);
        break;
      case 'shortstay':
        $scope->rental = $parentObj; // get_object($parent_type, $parent_id);
        break;
      default:
        break;
      }
      
      // mark as read
      $unreadSearch = ["thread" => $thread_id, "read_on" => ATT_DEFAULT_NULL, "receiver_id" => $user_id];
      list($count, $errors) = update_objects("chat", ["read_on" => now()], $unreadSearch);
      
    }
    
    //$scope->this_sender_id = $user_ids[0];
    //$receiver_id = $user_ids[1];
    $scope->user_id1 = $user_ids[0];
    $scope->user_id2 = $user_ids[1];
    $scope->user_ids = $user_ids;
    
    if(in_array($u_id, $user_ids)) { // we're in this chat
      $scope->sender_id = $u_id;
      if($other_user_id = chat_thread_user($thread_id, $user_id)) { // get other user
        $scope->receiver_id = $other_user_id;
        $scope->other_user = get_user($other_user_id);
      }
    } else { // we're not in this chat (staff)
      $scope->sender_id = $sender_id = $user_ids[0];
      $scope->receiver_id = $receiver_id = isint($user_ids[1]) ? $user_ids[1] : 0;
      $scope->other_user = get_user($sender_id); 
      $scope->first_user = $receiver_id ? get_user($receiver_id) : null; 
    }
              
    $scope->thread_id = $thread_id;
    $scope->thread = $chatObjs;
    //die("count=$count".dump($scope,true));
    
  } else { // find all threads

    $chatFields = array("thread", "max(id) as id, thread, max(created) as created", 
                "max(distinct(parent_type)) as parent_type", "max(distinct(parent_id)) as parent_id",
                "count(*) as count");
    $chatOptions = array('group' => 'thread', 'fields' => $chatFields, 'sort' => 'created desc', "debug_query" => false);
    $chatSearch[DB_LOGICAL_OR] = array("sender_id" => $user_id, "receiver_id" => $user_id);    
    $scope->threads = $threads = find_objects("chat", $chatSearch, $chatOptions);
    foreach($threads as &$thread) {
      $thread->other_user_id = chat_thread_user($thread->thread, $user_id); // get other user
      
      $alertSearch = ["thread" => $thread->thread, "read_on" => ATT_DEFAULT_NULL, "receiver_id" => $user_id, "status_change" => [">", 0]];
      $unreadSearch = ["thread" => $thread->thread, "read_on" => ATT_DEFAULT_NULL, "receiver_id" => $user_id];
      
      $thread->alert = count_objects("chat", $alertSearch);
      $thread->unread_count = $unread = count_objects("chat", $unreadSearch);
    }
  }
  return;
}



function controller_event($app, $options = []) {
  global $u_level, $auth_edit;
  $__header = $app->header; 
  $ver = $app->version ?: date('md');
  $siteObj = $app->site;
  
  
  $__header->scripts[] = "/afr/bower/geocomplete/jquery.geocomplete.min.js";
  $__header->scripts[] = "/afr/js/afr_gmap.js";
  $__header->scripts[] = "/afr/jquery/periodpicker/build/jquery.periodpicker.full.min.js";

  $__header->scripts[] = "/afr/bower/bootstrap-confirmation2/bootstrap-confirmation.js";    
  $__header->scripts[] = "/afr/js/event-form.js?v=$ver";    
  $__header->styles[] = "/afr/jquery/periodpicker/build/jquery.periodpicker.min.css";
  
  $scope = $app->page->scope;
  $obj_type = 'event';
  
  $city = $options['city'] ?: 'Amsterdam';
  $country = $options['country'] ?: 'NL';
  $fields = array("id", "name", "notes", "start_address", "start_time", "end_time", "startdatetime", "enddatetime", "startdate", "enddate", "location_lat", "location_long","location_bounds","address_street","address_number","address_city","address_postcode","address_state","address_country","address_formatted");
  $id = pick_first($_GET['id'], $options['id'], $_GET['article_id'], $app->object_id, $app->slug, $scope->event_id, $options['event_id']);

  $scope->auth_add_event = $u_level >= USER_LEVEL_EDITOR ? 1 : 0;
  
  if($id) {
    $scope->event = $obj = get_object($obj_type, $id);
    if($obj->user_level > $u_level) return controller_error($app, "Not authorized", 401);
    $scope->id = $obj->id;
    if($media = $obj->media) $scope->media_list = explode(',', $media);
    $scope->uploader = object_media_uploader($obj_type, $id);
    $scope->auth_edit_event = $u_level >= USER_LEVEL_EDITOR || auth_event($obj);
    object_increment($obj_type, $id, 'views');
    
    return;
  }

  // could return here if array not needed in single view
  // return;
  
  // below only used if Javascript is disabled
  $where = $_GET['e'];  
  $eventSearch = array('site_id' => $siteObj->id, 'active' => 1, 'end_time' => ['>=', today()], 'user_level' => array('<=', $u_level));
  switch($where) {
  case 'l': // local
    $eventSearch['address_city'] = 'Amsterdam';
    break;
  case 'n': // local
    $eventSearch['address_country'] = 'NL';
    break;
  case 'i': // local
    $eventSearch['address_country'] = array('!=', 'NL');
    break;
  default:
    break;
  }
    
  if(is_array($search)) $eventSearch = array_merge($search, $eventSearch);
  $events = find_objects('event', $eventSearch, array('sort' => 'start_time'));
  foreach($events as &$event) { // assign classes for sorting
    if(strtolower($event->address_city) == strtolower($city)) $class = "local";    
    elseif(strtolower($event->address_country) == strtolower($country)) $class = "national";
    else $class = $event->address_country ? "international" : "";
    $event->loc_class = $class;
    $event->auth_edit_event = $u_level >= USER_LEVEL_EDITOR || auth_event($event);
  }
  
  
  $scope->events = $events;
  return;  
}

function controller_blog($app, $options = []) {
  global $u_level;
  $scope = $app->page->scope;
  
  $options['object_subtype'] = ARTICLE_TYPE_BLOG;  
  controller_article($app, $options);
  
  $scope->auth_add_blog = $u_level >= USER_LEVEL_EDITOR ? 1 : 0;
  return;  
}

function controller_news($app, $options = []) {
  global $u_level;
  $scope = $app->page->scope;
  
  $options['object_subtype'] = ARTICLE_TYPE_NEWS;
  controller_article($app, $options);
  
  $scope->auth_add_news = $u_level >= USER_LEVEL_EDITOR ? 1 : 0;
  return;  
}

// generic controller for articles (news, blogs, etc). Requires that article type is set 

function controller_article($app, $options = []) {
  global $u_level, $__LANG;
  $pageObj = $app->page;
  $siteObj = $app->site;
  $scope = $app->page->scope;
  $obj_type = 'article';
  $fields = array("id", "type", "title", "content");

  $scope->auth_add_article = $auth_add = $auth_edit = $u_level >= USER_LEVEL_EDITOR ? 1 : 0;
  
  $id = $scope->obj_id = $app->object_id;
  $options['article_id'] = $obj_id;

  $scope->type = $type = pick_first($options['object_subtype'], $scope->object_subtype, $pageObj->object_subtype);

  $lang = $options['language'] ?: $__LANG;
  $user_level = $u_level ?: 0;

  $page_size = isset($options['page_size']) ? $options['page_size'] : 5; // for pagination
  $hard_limit = 20;
  
  $scope->start = $start = isset($_GET['start']) ? $_GET['start'] : 0;
  $scope->limit = $limit = $page_size ?: $hard_limit;
  
  $add = $scope->add = isset($_GET['add']) && $auth_add;

  $search = ['site_id' => $siteObj->id, 'type' => $type, 'language' => $lang, 'active' => 1, 'user_level' => array('<=', $u_level ?: 0)];
  if(!$auth_add) $search['published'] = 1;
  
  $searchOptions = ["sort" => "post_date DESC", "start" => $start, "limit" => $limit];
  $articles = find_objects($obj_type, $search, $searchOptions);  
  
  $sidebar_limit = 5;
  $scope->recent  = find_objects($obj_type, $search, array('sort' => 'created desc', 'limit' => $sidebar_limit));  
  $scope->popular = find_objects($obj_type, $search, array('sort' => 'views desc', 'limit' => $sidebar_limit));  
  
  if($id) {    
    $obj = get_object($obj_type, $id);
    $scope->article_id = $obj->id;
    $scope->auth_edit_article = $obj->auth_edit_article = $auth_edit = $u_level >= USER_LEVEL_EDITOR || auth_article($obj);
    $scope->comments = find_objects('comment', array("article_id" => $id, 'active' => 1));
    if($auth_edit) {
      jfu_dependencies($app); // load scripts + css for new file uploader
      if(isset($_GET['publish'])) {
        $publish = $_GET['publish'] ? 1 : 0;        
        if($publish != $obj->published) {
          list($id, $errors) = update_object('article', ['published' => $publish], $id);
          if($errors) return controller_error($app, $errors);          
          $obj->published = $published;
        }
      }

      //$scope->article->json_data = object_json_data($obj_type, $obj, $fields);
      $scope->comments = $comments = find_objects('comment', ['article_id' => $id, 'active' => 1], ['sort' => 'created']);
      $scope->comment_count = count($comments); 
    } elseif(!$obj->published) {
      return controller_error($app, "Not authorized");
    }
    
    $scope->edit = $edit = isset($_GET['edit']) && $auth_edit;
    if(!$edit) object_increment($obj_type, $id, 'views');
    $scope->article = $obj;
    
  } elseif($add) {
    // do nothing
  } elseif($type) {
    
  
    if($auth_edit) {
      foreach($articles as &$article) {
        //$article->json_data = object_json_data($obj_type, $article, $fields);
        $article->auth_edit_article = $u_level >= USER_LEVEL_EDITOR || auth_article($article);
      }
    }
    
    $scope->count = $count = count_objects('article', $search);
    //echo("Found $count articles");dump($search);
    $scope->articles = $articles;
    //dump($articles);
    
  } else {
    $scope->error = "No article ID or type set";
  }

  
  return;
}

// todo: add limit, sort, paging
function controller_faq($app, $options = []) {
  global $__LANG;
  global $u_level, $auth_edit;
  $siteObj = $app->site;
  
  $scope = $app->page->scope;
  $obj_type = 'article';
  $facs = [];
  
  $search = ['site_id' => $siteObj->id, 'type' => ARTICLE_TYPE_FAQ, 'language' => $siteObj->default_language, 'active' => 1];
  $articleObjs = find_objects('article', $search, []);
  //dump($articleObjs);die("gh");
  foreach($articleObjs as $articleObj) {
    if($__LANG != $siteObj->default_language) {
      $descSearch = ['parent_type' => 'article', 'parent_id' => $articleObj->id];
      if($descObj = find_object('object_description', $descSearch)) {
        $articleObj->title = $descObj->title;
        $articleObj->content = $descObj->description;
        $desc_id = $descObj->id;
      } else {
        $descData = array_merge($descSearch, ['title' => $articleObj->title, 'description' => $articleObj->content, 'language' => $__LANG]);
        list($desc_id, $errors) = add_object('object_description', $descData);        
        //$articleObj->content = $descObj->content;
      }
      $articleObj->obj_type = 'object_description';    
      $articleObj->obj_id = $desc_id;    
      $articleObj->content_field = 'description';
    } else {
      $articleObj->obj_type = 'article';
      $articleObj->obj_id = $articleObj->id;    
      $articleObj->content_field = 'content';
    }
    $facs[] = $articleObj;
  }
  //dump($facs);
  $scope->articles = $facs;
  $scope->auth_edit_article = $u_level >= USER_LEVEL_STAFF;
  return;
}



function controller_golf_club($app, $options = []) {
  global $u_id, $__LANG;
  $__header = $app->header; 
  $scope = $app->page->scope;
  $ver = $app->version ?: date('md');
  $__header->styles[] = "/afr/css/afr-air.css";
  $obj_type = 'golf_club';
  $slug = $app->slug;
  $id = $obj_id = $golf_club_id = $scope->obj_id = $scope->golf_club_id = pick_first($scope->golf_club_id, $scope->obj_id, $_GET['obj_id'], $options['obj_id'], $app->object_id);
  
  if(!$id) { 
    $error = $slug ? "$slug not found" : "No golf club specified";
    return scope_error($app, $error, '403');
  }

  $obj = get_object($obj_type, $id);    
   
  if(!$obj) return scope_error($app, "Golf club {$slug} not found", '404');
  $scope->golf_club = $obj;
  
  // $mediaObjs = object_images($obj_type, $id);
  $images = $obj->images ? json_decode($obj->images) : [];
  foreach($images as $mediaObj) {
    $image = new stdClass;
    $image->src = $mediaObj->filename;
    $image->title = $mediaObj->name;
    if($copyright = $mediaObj->copyright_name) {
      $copyright = html_i('', 'fa fa-copyright')." $copyright";
      $copyright_link = $mediaObj->copyright_url;
      $copyright_text = $copyright_link ? html_link($copyright_link, $copyright, array('target' => 'ext')) : $copyright;
      $image->copyright = html_div($copyright_text, 'copyright');        
    }
    $imageAr[] = $image;
  }
  

  $search = ['parent_type' => 'golf_club', 'parent_id' => $obj->id, 'language' => $__LANG];
  if($descObj = find_object('object_description', $search)) {
    $scope->description = $descObj;
  } elseif($descObj = find_object('object_description', ['parent_type' => 'golf_club', 'parent_id' => $obj->id])) { // any language
    $descData = ['language' => $__LANG];
    foreach(['site_id', 'parent_type', 'parent_id', 'type', 'description'] as $f) $descData[$f] = $descObj->$f;
    list($desc_id, $errors) = add_object('object_description', $descData);
    if($desc_id) $scope->description = get_object('object_description', $desc_id);
    if($errors) $scope->error = $errors;
  }
  
  $scope->golf_courses = $golf_courses = find_objects('golf_course', array('golf_club_id' => $id)) ?: array();
  //dump($golf_courses);
  
  //dump($imageAr);
  $scope->images = $imageAr;
  $scope->nearby_golf = location_find_nearest('golf_club', $obj->location_lat, $obj->location_long, array('limit' => 4, 'exclude' => $obj->id));
  $scope->nearby_apts = location_find_nearest('shortstay', $obj->location_lat, $obj->location_long, array('limit' => 4));      
  $scope->reviews = find_objects('review', array("client_id" => $app->client_id, "parent_type" => $obj_type, "parent_id" => $id, "published" => 1, "active" => 1));

  $facAr = $obj->facilities ? explode(',', $obj->facilities) : array();
  $facs = array();
  foreach($facAr as $fac_id) {
    $facObj = new stdClass;
    $facObj->id = $fac_id;
    $facObj->name = get_object("facility", $fac_id, "name");
    $facObj->icon = rental_facility_icon($fac_id) ?: "fa fa-check";
    $facs[] = $facObj;
  }
  $scope->facs = $facs; // $scope->facilities = aptObj->facilities, already in use
  
  return;
}

/** mail preview function - for staff and higher only */
function controller_review($app, $options = []) {
  global $u_id, $u_level, $__CLIENT_ID, $__SITE;
  $siteObj = $app->site;
  $scope = $app->page->scope;

  $obj_id = $scope->obj_id = $app->object_id;
  $obj_type = $scope->obj_type = $app->object_type;
  $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : null;
  

  $parent_type = pick_first($options['parent_type'], $_GET['_pt'], $siteObj->review_type);
  $parent_id = pick_first($options['parent_id'], $_GET['_pid']);
  $pin = $_GET['_pin'];

  if($obj) { /** View existing review */
    $scope->review = $obj;
    $parent_type = $obj->parent_type;
    $parent_id = $obj->parent_id;
    
  } elseif($parent_type) { /** add new review */
    $allow_multiple_reviews = 0;
    
    $scope->parent_type = $parent_type;
    if($parent_id) {
      $scope->parent_id = $parent_id;
      $scope->parent_object = $parentObj = get_object($parent_type, $parent_id);
      
      
      switch($parent_type) {          
        case 'reservation':
          $allow_multiple_reviews = 1;
          
          $resObj = $parentObj;
          if(!$resObj) return controller_error($app, phrase("booking not found", CAPS));
          
          $scope->reviewed_obj_type = $parent_type;
          $scope->reviewed_obj_id = $parent_id;
          $scope->reviewed_object = $parentObj;

          $scope->parent_id = $parent_id = $resObj->apt_id; /** we actually want to review the rental */ 
          $scope->parent_type = $parent_type = $resObj->property_type;
          $scope->parent_object = $parentObj = get_object($resObj->property_type, $resObj->apt_id);
          
          
          $today = today();
          $review_cutoff = BOOKING_REVIEW_PERIOD;
          $last_review_date = add_day(strip_time($resObj->checkout), $review_cutoff);
          if($u_id) {
            if($u_level < USER_LEVEL_STAFF && $resObj->guest_id && $u_id != $resObj->guest_id) return controller_error($app, phrase("only the guest can rate bookings", CAPS));
          } else {
            if($resObj->guest_id) redirect_login();
            if(myhash($resObj->booking_pin) != $pin) return controller_error($app, phrase("missing or invalid code", CAPS));           
          }
          if($reviewObj = find_object('review', ['reviewed_obj_type' => 'reservation', 'reviewed_obj_id' => $resObj->res_id])) {
            return controller_error($app, phrase("you already rated this %s on %s", CAPS, get_object_name('reservation'), sql2human($reviewObj->created)));
          }                              
          if(!in_array($resObj->status, [STATUS_RESERVED, STATUS_COMPLETE])) return controller_error($app, phrase("only confirmed bookings can be rated", CAPS));
          if($today > $last_review_date)  return controller_error($app, phrase("the review period expired on %s", CAPS, sql2human($last_review_date)));
          if(strip_time($resObj->checkout) > $today)  return controller_error($app, phrase("you cannot rate the booking until you check out on %s", CAPS, sql2human($resObj->checkout)));
          break;
        default:
          break;      
      }
    } 
  
    /** only allow user to review same item once ? Doesn't make sense for bookings, since they can stay in same flat twice... */
    if($u_id && !$allow_multiple_reviews) {
      if($reviewObj = find_object('review', ['user_id' => $u_id, 'parent_type' => $parent_type, 'parent_id' => $parent_id])) {
        return controller_error($app, phrase("you already rated this %s on %s", CAPS, get_object_name($parent_type), sql2human($reviewObj->created)));
        //$scope->review = $reviewObj;
      }
    }
  
  }
  
  $scope->items = $items = find_objects('review_item', ['site_id' => $siteObj->id, 'parent_type' => $parent_type]);
  if(!count($items)) return controller_error($app, "No review items found for $parent_type");
    
  if(!$obj && !$parent_type) return controller_error($app, "No review or parent");
  
  return;
}

/** mail preview function - for staff and higher only */
function controller_mail_campaign($app, $options = []) {
  global $u_id, $u_level, $__CLIENT_ID, $__SITE, $__USER;
  global $__SITE_ROLES;
  $siteObj = $app->site;
  $scope = $app->page->scope;
  $campaign_id = pick_first($_GET['campaign_id'], $app->object_id, $app->slug, $scope->campaign_id, $options['campaign_id']);

  $step = 0;
  if($campaign_id) {
    $scope->campaign = $campaignObj = get_object('mail_campaign', $campaign_id);
    $scope->campaign_id = $campaignObj->id;
    $step = isset($_GET['preview']) ? 3 : 2;
  }
  
  $scope->step = $step;
  
  $search = ['user.site_id' => $siteObj->id, 'active' => 1];
  
  $receiver_roles = $_GET['receiver_roles'];
  $selected_roles = $receiver_roles ? explode(',', $receiver_roles) : [];
  $selected_roles_str = [];
  
  if(!$step) {
    $receiver_count = $_GET['receiver_count'];
    $active_since = $_GET['active_since'];
    $name = trim($_GET['name']);  
    
    if($active_since) $search['last_login'] = ['>=', date2sql($active_since)];  
    $scope->users  = $users = find_objects('user', $search);
    $scope->user_count  = $count = count($users);
    
    $roleObjs = find_objects('user_role', [], ['sort' => 'id']);
    $roles = $role_terms = [];
    foreach($roleObjs as $roleObj) {
      if(is_array($__SITE_ROLES) && !in_array($roleObj->id, $__SITE_ROLES)) continue;
      $roleSearch = array_merge($search, ["roles & {$roleObj->id}" => $roleObj->id]);
      $users = find_objects('user', $roleSearch, ['debug_query' => 0]);
      $count = $roleObj->count = count($users);
      if($count || in_array($roleObj->id, $selected_roles)) $roles[] = $roleObj;
      if(in_array($roleObj->id, $selected_roles)) $selected_roles_str[] = $roleObj->name;
    }
    //echo("gh roles=".dump($roles,true));
    if($receiver_roles) {
      list($recipients, $errors) = campaign_recipients($_GET);
      if($errors) {
        $scope->error = $errors;
      } else {
        $scope->receiver_count = $receiver_count = count($recipients);
        $scope->message = "{$receiver_count} unique recipients found (some users have multiple roles)";
      }
      
      // dump($recipients);
      /**
      if(count($selected_roles) > 1) {
        foreach($selected_roles as $role_id) $role_terms["roles & {$role_id}"] = $role_id;
        $roleSearch = array_merge($search, [DB_LOGICAL_OR => $role_terms]);
      } else {
        $roleSearch = $search;
        $role_id = $selected_roles[0];
        $roleSearch["roles & {$role_id}"] = $role_id;
      }
      $scope->selected_users  = $selected_users = find_objects('user', $roleSearch);      
      $scope->receiver_count  = $receiver_count = count($selected_users);
      $active_str = $active_since ? "active since $active_since" : "";
      $roles_str = implode(',', $selected_roles_str);
      if($receiver_count) {
        $scope->message = "{$receiver_count} active users with roles $roles_str $active_str";
      } else {
        $scope->error = "No active users found with roles $roles_str $active_str";
      }
      */

      if($_GET['submit']) { // show step 2
        $campaign = array2obj($_GET);
        $campaign->sender = site_email($siteObj, $siteObj->name); 
        $campaign->subject = ""; // "[{$siteObj->shortname}]"; 
        $campaign->replyto = email_recipient(get_user_email($__USER),get_user_name($__USER));
        $campaign->recipients = json_encode($recipients); 
        $campaign->receiver_count = $receiver_count;
        if($active_since) $campaign->active_since = date2sql($active_since);
        
        // printable list of roles
        $roleObjs = find_objects('user_role', ['id' => $selected_roles]);
        $roles = [];
        foreach($roleObjs as $roleObj) $roles[] = $roleObj->name;

        $scope->receiver_roles_str = implode(", ", $roles);
        $scope->step = 2; // passed to template, not switch statement below
        $scope->receiver_count = $receiver_count;        
        $scope->campaign = $campaign;
      }
    }
    
    $scope->selected_roles = $selected_roles;
    $scope->roles = $roles;
    
  }
      
  return;
}

/** contact form */
function controller_contact($app, $options = []) {
  global $__USER;
  $scope = $app->page->scope;
  $siteObj = $app->site;

  $scope->replyto_sender = 1;
  $__header = $app->header; 
  $__header->base_scripts[] = "//www.google.com/recaptcha/api.js";

  //die("contact ctrl".dump($scope,true));
  print_log("controller_contact: scope=".dump($scope,true), 'mail', LOG_LEVEL_TEST);
  
  /** all of the below currently only used for vPatina plugin */
  
  if($sender = trim(urldecode($_GET['sender']))) {
    list($email, $name) = parse_recipient($sender);
    $scope->sender_name = $name;
    $scope->sender_email = $email;
    //echo("s=$sender n=$name e=$email"); 
  } elseif($__USER) {
    $scope->sender_name = $__USER->display_name;
    $scope->sender_email = $__USER->email_address;
    $scope->sender_phone = $__USER->phone;
  }
  if($recipient = trim(urldecode($_GET['recipient']))) {
    list($email, $name) = parse_recipient($recipient);
    $scope->receiver_name = $name;
    $scope->receiver_email = $email;
  }
  if($subject = trim(urldecode($_GET['subject']))) $scope->subject = $subject;

  if($msg = trim(urldecode($_GET['msg']))) $scope->main_message = $msg;
  //else $scope->main_message = "You have a message from the contact form.";
  else $scope->main_message = "An inquiry via {$siteObj->name}:";

  extract($_GET);
    
  $scope->provider_keys = $pk = $app->scope->provider_keys; // find_objects('provider_key', array('site_id' => $siteObj->id, 'active' => 1), array("field" => "provider_id"));
          
  return;
}

/** mail preview function - for staff and higher only */
function controller_mail($app, $options = []) {
  global $u_id, $u_level, $__CLIENT_ID, $__SITE, $__USER;
  $scope = $app->page->scope;
  $siteObj = $app->site;

  $mid = pick_first($scope->obj_id, $scope->mail_id, $option['obj_id'], $scope->obj_id, $_GET['mid'], $app->object_id);
  $uid = $_GET['uid'];

  $mailObj = null;
  if($mid) {
    $mailObj = get_object('mail_queue', $mid);
    $auth_mail = auth_mail($mailObj);
  } else if($uid) { // UID is almost impossible to guess, so no log-in/auth required other than correct client ID
    $mailObj = find_object('mail_queue', ['guid' => $uid]);
    $auth_mail = $mailObj->client_id == $__CLIENT_ID;
  }
  
  if($mailObj && !$auth_mail) {
    return scope_error($app, "Unauthorized $mid", '403');
  }
  
  $scope->auth_mail = $auth_mail;
  $scope->show_address_book = 0;
  
  if($mailObj) {
    $scope->mail = $mailObj;
   
  } elseif($u_id && $u_level >= USER_LEVEL_STAFF) { /** blank mail template */
    $scope->mail_template = $template = "mail/mail-generic";
    
    $sender_name = user_display_name($__USER->id);
    
    $guest_name =  $resObj->guest_id ? get_user_name($resObj->guest_id) : get_name_string($resObj);
    $mail_scope = ['receiver_name' => "", 
                   'receiver_role' => 'user', 
                   'receiver_id' => 0, 
                   'main_message' => phrase('you have a message from %s', CAPS, $sender_name)];
    
    $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope]);
    
    $mail = new stdClass;
    $mail->template = $template = "mail/mail-booking";    
    $mail->controller = $controller = "booking";            
    $mail->scope = $mail_scope;
    $mail->from = user_email($__USER, true) ?: site_email($siteObj);    
    $mail->to = "";    
    $mail->subject = mail_subject('mail');
    $mail->body = $body; 
    $mail->action = '';
    $scope->mail  = $mail;
    
    $scope->show_address_book = $u_id ? 1 : 0;
    
  } elseif($u_id) {
    return controller_error($app, phrase("nothing to see here", CAPS));    
  } else {
    return controller_error($app, phrase("please log in", CAPS));    
  }
     
  return;
}

function controller_share($app, $options = []) {
  $user_id = $app->user_id;
  $user = $app->user;
  
  $__header = $app->header; 
  $siteObj = $app->site;  
  $scope = $app->page->scope;
  
  /** new */

  $obj_type = $_GET['obj_type'] ?: '';
  $obj_id = $_GET['obj_id'] ?: 0;
  $template = $_GET['template'] ?: '';
  $controller = $_GET['controller'] ?: '';

  //echo("ot={$obj_type} id:{$obj_id}");
  
  $public = ['artwork', 'media_collection_map', 'collection', 'media_collection'];
  if($obj_type && $obj_id && in_array($obj_type, $public)) {
    if(!in_array($obj_type, $public)) return controller_error($app, "Unsupported object type $obj_type");
    $obj = get_object($obj_type, $obj_id);
    if(!$obj) return controller_error($app, "{$obj_type} {$obj_id} not found");
    
    if(!$template) { /** figure out template/controller based on object type */
      switch($obj_type) {
      case 'media_collection_map':
      case 'artwork':
        $template = 'mail-artwork.html';
        $controller = 'gallery_artworks';
        $obj_type = 'media_collection_map';
        break;
      case 'media_collection':
      case 'collection':
        $template = 'mail-collection.html';
        $controller = 'gallery_collections';
        $obj_type = 'media_collection';
        break;
      default:
        return controller_error($app, "Unsupported object type $obj_type");      
      }
      //echo("tpl={$template}");
    }
  }
  
  // https://test.vpatina.com/share/?url=https%3A%2F%2Ftest.vpatina.com%2Fexhibition%2Fclementine~1743&obj_type=media_collection&obj_id=1743&subject=foobar&_aid=17425&recipient=Thea+VG+<theo%40vpatina.com>
  
  if($subject = $_GET['subject']) $scope->hide_subject = 1; 
  $mail = new stdClass;
  
  $sender = $_GET['sender'] ?: '';
  if(!$sender && $user) $sender = user_email($user);
  
  $mail->obj_type = $obj_type;
  $mail->obj_name = $obj_name = $obj_type ? get_object_name($_GET['obj_type'], LOWERCASE) : '';
  $mail->obj_id = $obj_id;
  
  $mail->template = $template;    
  $mail->controller = $controller;       
  $mail->sender = $sender;
  $mail->recipient = $recipient = $_GET['recipient'] ?: '';
  //$mail->subject = $user ? user_display_name($user)." shared this with you from {$app->site->name}" : "Shared with you from {$app->site->name}";
  $mail->subject = $subject ?: "I thought you might be interested in this {$obj_name}";
  $mail->main_message = '';

  $scope->mail  = $mail; 
  
  return;
}

/** vPatina + Art controllers */
/** using old code: todo: replace with new controllers and templates */
     

function controller_gallery_artists($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  $__header->scripts[] = '/afr/js/afr_gallery.js?ver='.$app->version;  
  $scope = $app->page->scope;
  $scope->items = $scope->objects;  
  $siteObj = $app->site;
  // unset($scope->objects);

  if($obj_id = $artist_id = $scope->obj_id = $app->object_id) {
    return  controller_artist_profile($app, $options);

  } else { /** browse artists: todo */
    $myCollectionObj = gallery_find_user_collection($siteObj, $u_id);
    $scope->my_collection_id = $collection_id = $myCollectionObj->id;
    $scope->items = $scope->objects ?: gallery_collection_images($siteObj, $collection_id, array('return' => 'object'));
    // unset($scope->objects);
  }

  return;
}

function controller_gallery_curators($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  $__header->scripts[] = '/afr/js/afr_gallery.js?ver='.$app->version;
  $siteObj = $app->site;
  
  $scope = $app->page->scope;
  $scope->items = $scope->objects;

  if($obj_id = $artist_id = $scope->obj_id = $app->object_id) {
    return  controller_artist_profile($app, $options);

  } else { /** browse curators: todo */
    $myCollectionObj = gallery_find_user_collection($siteObj, $u_id);
    $scope->my_collection_id = $collection_id = $myCollectionObj->id;
    $scope->items = $scope->objects ?: gallery_collection_images($siteObj, $collection_id, array('return' => 'object'));
    // unset($scope->objects);
  }
  
  return;
}


// my-vpatina (view, explore art)
function controller_art_viewer($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";
  $siteObj = $app->site;
  
  $scope = $app->page->scope;
  $aif = $_GET['_aif'];
  $scope->my_collection_id = $collection_id = gallery_find_user_collection($siteObj, $u_id, array('return' => 'id'));    

  /** follow media from GET var (plugin) */
  if(isint($aif)) {
    if(!$collection_id) return controller_error($app, "Missing user collection");
    $mediaObj = get_object('media', $aif);
    if(!$mediaObj) return controller_error($app, "Artwork not found");
    $media_name = $mediaObj->name ?: "this artwork";
    if($mapObj = find_object('media_collection_map', array('media_id' => $aif, 'media_collection_id' => $collection_id))) {
      $scope->warning = "You already follow $media_name";
    } else {
      $result = gallery_publish_image($siteObj, $aif, $collection_id);
      if($result->success) {
        $scope->message = "You now follow $media_name";
      } else {
        $scope->error = $result->error;
      }
    }
    $scope->follow_id = $aif; 
  }
  
  //dump($myCollectionObj);
  
  return;
}  

/** new backend for vPatina */
function controller_gallery_backend($app, $options = []) {
  global $u_id, $u_level;
  $scope = $app->page->scope;
  $errors = $messages = [];  
  if(!$u_id) {
    redirect_login();
    return;
  }
  if($u_level < USER_LEVEL_ARTIST) return scope_error($app, "Unauthorized", '403');
  $userObj = $app->user;
  $siteObj = $app->site;
  
  $__header = $app->header; 
  $__header->styles[] = "/afr/css/afr_sidebar.css?ver={$app->version}";
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";
  $__header->styles[] = "/afr/bower/tag-it/css/jquery.tagit.css";
  $__header->styles[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.css?v=1.11";
  $__header->styles[] = "https://cdnjs.cloudflare.com/ajax/libs/bootstrap-timepicker/0.5.2/css/bootstrap-timepicker.min.css";
  $__header->styles[] = "/afr/bower/bootstrap-tags/dist/css/bootstrap-tags.css";
  $__header->styles[] = "/afr/bower/bootstrap-toggle/css/bootstrap-toggle.min.css";

  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";

  $__header->scripts[] = "/afr/bower/geocomplete/jquery.geocomplete.min.js";
  $__header->scripts[] = "/afr/bower/confirm-bootstrap/confirm-bootstrap.js";
  $__header->scripts[] = "/afr/bower/bootstrap-toggle/js/bootstrap-toggle.min.js";

  $__header->scripts[] = "/afr/js/afr_gmap.js";
  $__header->scripts[] = "/afr/bower/bootstrap-confirmation2/bootstrap-confirmation.js";    
  $__header->scripts[] = "/afr/js/event-form.js?ver={$app->version}";    
  
  $__header->scripts[] = "/afr/bower/jquery.are-you-sure/jquery.are-you-sure.js";

  $__header->scripts[] = "/afr/js/afr_gallery_backend.js?ver={$app->version}";

  $__header->scripts[] = "/afr/bower/bootstrap-tags/dist/js/bootstrap-tags.min.js";

  $__header->scripts[] = "/afr/bower/tag-it/js/tag-it.min.js";
  $__header->scripts[] = "/afr/bower/jquery-file-download/src/Scripts/jquery.fileDownload.js";
  $__header->scripts[] = "/afr/bower/table-export/jquery.base64.js?v=1.11";
  $__header->scripts[] = "/afr/bower/table-export/tableExport.js?v=1.11";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/bootstrap-table.min.js?v=1.11";
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/locale/bootstrap-table-en-US.js?v=1.11"; /** todo: support other languages, use $app->locale  */
  $__header->scripts[] = "/afr/bower/bootstrap-table/dist/extensions/export/bootstrap-table-export.js?v=1.11";
  $__header->scripts[] = "https://cdnjs.cloudflare.com/ajax/libs/bootstrap-timepicker/0.5.2/js/bootstrap-timepicker.min.js";
  
  if(0) { // cloudinary direct upload
    require_once("xml/xml_cloudinary.inc");
    $ci_config = get_config('ci', $siteObj->id);
    if(!$ci_config) return scope_error($app, "Missing Cloudinary Configuration");
    if(!$ci_config->preset_name) return scope_error($app, "Missing Cloudinary Preset");
    
    $signature = cloudinary_signature($app->site);
    $cloudinaryData = ["timestamp" => time(), "callback" => rtrim($app->site_url,'/')."/cloudinary_cors.html", "signature" => $signature, "api_key" => $ci_config->api_key];
    $scope->cloudinary_data = $cloudinaryData;
    $scope->cloudinary_config = $ci_config;

    $__header->scripts[] =  "/afr/bower/blueimp-file-upload/js/vendor/jquery.ui.widget.js";             
    $__header->scripts[] =  "/afr/bower/blueimp-file-upload/js/jquery.iframe-transport.js";              
    $__header->scripts[] =  "/afr/bower/blueimp-file-upload/js/jquery.fileupload.js";                                                                   
    $__header->scripts[] =  "/afr/bower/cloudinary-jquery-file-upload/cloudinary-jquery-file-upload.js"; 
    $__header->scripts[] =  "script:$.cloudinary.config({\"api_key\":\"{$ci_config->api_key}\",\"cloud_name\":\"{$ci_config->cloud_name}\"});"; 
  } else { // local server upload (to cloudinary from server)
    jfu_dependencies($app); // load scripts + css for new file uploader
  }
  

  

  // artwork
  $artist_ids = ($u_level < USER_LEVEL_ADMIN) ? user_artist_list($app->user_id) : find_objects('user', ['user.site_id' => $app->site->id, 'roles & '.USER_ROLE_ARTIST => ['>', 0]], ['fields' => 'user.id']);
  
  $artist_count = count($artist_ids);
  $user_ids = array_unique(array_merge($artist_ids, [$u_id]));
  
  
  $subview = $app->page->subview = $app->page->subview ?: 'artworks';
  //$subview = $app->page->subview ?: 'artworks';
  $partial = isset($_GET['_pv']) && $_GET['_pv'];
  $app->page->subtemplate = "_gallery-backend-{$subview}.html";
  
  if(!$partial) { /** don't need to do this in partial view */
    $artists = [];
    foreach($artist_ids as $artist_id) {
      $artists[$artist_id] = user_display_name($artist_id);
    }
    $scope->artists = json_encode($artists);
    $scope->user_ids = json_encode(array_keys($artists));
  }
    
  $scope->obj_id = $id = $obj_id = $app->page->object_id;
  $scope->add_to = $add_to = $_GET['add_to'] ?: 0;
  $scope->hide_related = 1; /** don't show related images in preview */
  $scope->artist_id = user_has_role($userObj, USER_ROLE_ARTIST) ? $userObj->id : 0;
  $scope->curator_id = user_has_role($userObj, USER_ROLE_CURATOR) ? $userObj->id : 0;
  $scope->backend = 1;
  
  $action = $app->page->action ?: '';
  
  $addCollectionObj = $add_to ? get_object('media_collection', $add_to) : null;
  
  
  //$obj_type = $app->page->object_type;
  //$obj = $obj_id && $obj_type ? get_object($obj_type, $obj_id) : null;
  
  $auth_edit = 0;
  $edit = $action == 'edit' ? 1 : 0;
  $add = $action == 'add' ? 1 : 0;
  
  switch($subview) {
    case "settings":
      $scope->section = 'settings';
      $app->page->subtemplate = $sub = '_gallery-backend-settings.html';
      break;
      
    case "artwork":
      $scope->section = 'artwork';
      $scope->obj_type = $obj_type = "artwork";
      if(!$add) {
        if(!$id) return controller_error($app, "Missing ID");
        controller_gallery_artworks($app);
        $obj = $scope->artwork;
        $scope->auth_edit_artwork = $obj ? ($auth_artist || gallery_auth_media($obj_type, $obj, $u_id, $artist_ids)) : false;
        
      }
      //die("u_id={$app->user_id } Auth_artist: {$auth_artist} auth_artwork={$scope->auth_edit_artwork}".dump($artist_ids,true));
      
      $app->page->subtemplate = $sub = '_gallery-backend-artwork.html';
      $app->page->itemview = 'artwork';
      $app->page->listview = 'artworks';
      print_log("action={$action} type={$obj_type} id={$obj_id} title={$obj->title} edit={$edit} auth={$auth_edit} sub={$sub}", 'backend', LOG_LEVEL_TEST);
      
      break;
    case "artworks":
      $scope->section = 'artwork';
      $scope->obj_type = $obj_type = "artwork";
      $app->page->itemview = 'artwork';
      $app->page->listview = 'artworks';

      $filters = [];
      if($add_to) $filters = ['active' => 1];
      if($addCollectionObj && $addCollectionObj->artist_ids && strlen($addCollectionObj->artist_ids) > 2) {
        $filters['artist_id'] = json_decode($addCollectionObj->artist_ids);
      } else {
        $filters[DB_LOGICAL_OR2] = ['user_id' => $app->user_id, 'artist_id' => $artist_ids];
      }
      $filter_json = count($filters) ? json_encode($filters) : '';
      $scope->list_url = "/ajax.php?oper=get-rows&obj_type=artwork".($filter_json ? "&filters=".urlencode($filter_json) : '');
      break;
    case "plugin":
      $scope->section = 'plugin';
      $scope->obj_type = $obj_type = "plugin";
      if($add) {
        $pt = $_GET['_pt'];
        $pid = $_GET['_pid'];
        if(!$pt) return controller_error($app, "Missing Parent Type");
        if(!$pid) return controller_error($app, "Missing Parent ID");
        $scope->parent_type = $pt;
        $scope->parent_id = $pid;
        $scope->parent_object = $pObj = get_object($pt, $pid);
      } else {
        if(!$id) return controller_error($app, "Missing ID");
        $scope->plugin = $obj = get_object('plugin', $id);
        $scope->preview = plugin_javascript(['shid' => $id, 'pid' => $obj->parent_id]);
        $pt = $obj->parent_type;
        $pid = $obj->parent_id;

        $scope->auth_edit_plugin = $u_level >= USER_LEVEL_STAFF || auth_object('plugin', $obj);
        $auth_obj = auth_object('plugin', $obj);
        
        /** fetch collection info */
        if($pt == 'media_collection' && $pid) {
          $scope->collection = get_object($pt, $pid);
        }
            
        //$app->object_type = $pt;
        //$app->object_id = $pid;
        //controller_gallery_collections($app);        
        //$app->object_type = 'plugin';
        //$app->object_id = $id;
      }
      
      $app->page->itemview = 'plugin';
      $app->page->listview = 'plugins';
            
      $scope->images = $images = find_objects('media_collection_map', ['media_collection_id' => $pid, 'active' => 1], ['sort' => 'rank']);
      break;
    case "plugins":
      $scope->section = 'plugin';
      $scope->obj_type = $obj_type = "plugin";
      $app->page->itemview = 'plugin';
      $app->page->listview = 'plugins';

      $filters = $u_level < USER_LEVEL_ADMIN ? ['user_id' => $app->user_id] : [];
      $pt = $_GET['_pt'];
      $pid = $_GET['_pid'];
      if($pt && $pid) {
        $scope->parent_type = $pt;
        $scope->parent_id = $pid;
        $filters['parent_type'] = $pt;
        $filters['parent_id'] = $pid;
      }
      
      $filter_json = count($filters) ? json_encode($filters) : '';
      $scope->list_url = "/ajax.php?oper=get-rows&obj_type=plugin&amp;sort=id%20desc&amp;".($filter_json ? "&filters=".urlencode($filter_json) : '');
      break;

    case "website":
      $scope->section = 'website';
      $scope->obj_type = $obj_type = "minisite";

      if(!$add) {      
        if(!$id) return controller_error($app, "Missing ID");
        $scope->minisite = $obj = get_object($obj_type, $id);
        $scope->artist_ids = $artist_ids = user_artist_list_display($obj->user_id) ?: [];
        foreach($artist_ids as $artist_id) {
          $uObj = get_user($artist_id);

          $colSearch = ['user_id' => $obj->user_id, artist_id=> $uObj->id, 'active' => 1, 'type' => COLLECTION_TYPE_EXHIBITION, 'start_date' => ''];
          $collectionobj = find_object('media_collection', $colSearch);
          $artist_id = $collectionobj->artist_id;
          
          if(!$collectionobj) {
            //echo("{$uObj->display_name} has no projects<br>");  
            continue;
          }
          $scope->viewartist[]=$viewartist[] = get_user($artist_id);
        }
             
          $scope->auth_edit_minisite = $u_level >= USER_LEVEL_STAFF || auth_object('minisite', $obj);   
      }

      $page_id = $_GET['page_id'];
      $scope->page_id = $page_id;
      //dump($scope->page_id);

      $colSearch = ['user_id' => $obj->user_id, 'active' => 1];
      $scope->minisiteobj = $minisiteobj = find_object('minisite', $colSearch);
      $scope->pagelist = $pagelist = find_objects('minisite_pages', $colSearch);

      $col = ['user_id' => $obj->user_id, 'page_id' => $page_id,'active' => 1];
      $scope->mhome = $mhome = find_object('minisite_home_page', $col);
      $scope->martist = $martist = find_object('minisite_artist_page', $col);
      $scope->mExhibition = $mExhibition = find_object('minisite_exhibition_page', $col);
     
      $scope->userprofile = $userprofile = get_user($obj->user_id);

      $profileSearch = ['user_id' => $obj->user_id];
      $scope->baseprofile = $profile = find_objects('user_profile', $profileSearch);
      
      $time = db_getAll("select * from opening_hours where user_id='$obj->user_id'");
      
      $scope->timing = $time;
      //dump($time);

      $sub = $app->host_sub;
      $app->page->itemview = 'website';
      $app->page->listview = 'websites';
      if($_GET['domain'] && is_url("https://{$sub}.{$obj->domain}")) {
        $scope->url = $url = "https://{$sub}.{$obj->domain}"; 
      } else {
        $scope->url = $url = "https://{$_SERVER['SERVER_NAME']}/{$obj->url}"; 
      }
      $scope->title = "URL: <a href='{$url}' target='website'>{$url}</a>";

      break;
    case "websites":
      $scope->section = 'website';
      $scope->obj_type = $obj_type = "minisite";
      $app->page->itemview = 'website';
      $app->page->listview = 'websites';

      $filters = $u_level < USER_LEVEL_ADMIN ? ['user_id' => $app->user_id] : [];
      $filter_json = count($filters) ? json_encode($filters) : '';
      $scope->list_url = "/ajax.php?oper=get-rows&obj_type=minisite&amp;sort=id%20desc&amp;".($filter_json ? "&filters=".urlencode($filter_json) : '');
      break;
      
    case "medium":
      $scope->section = 'media';
      $scope->obj_type = $obj_type = "media";
      if(!$add) {      
        if(!$id) return controller_error($app, "Missing ID");
        $scope->media = $obj = get_object('media', $id);
        $scope->owner = get_user($obj->user_id);
        $scope->auth_edit_media = $u_level >= USER_LEVEL_STAFF || gallery_auth_media($obj_type, $obj);
      }
      $app->page->itemview = 'medium';
      $app->page->listview = 'media';
      break;
      
    case "media":
      $scope->section = 'media';
      $scope->obj_type = $obj_type = "media";
      $app->page->itemview = 'medium';
      $app->page->listview = 'media';

      $filters = ['active' => 1];
      $filters[DB_LOGICAL_OR2] = ['user_id' => $app->user_id, 'artist_id' => $artist_ids]; 
      $filter_json = count($filters) ? json_encode($filters) : '';
      $scope->list_url = "/ajax.php?oper=get-rows&obj_type=media".($filter_json ? "&filters=".urlencode($filter_json) : '');
      break;
    case "vault":
      $scope->section = 'vault';
      $scope->obj_type = $obj_type = "gallery";
      if($id) {
        $scope->folder = $folder = get_object($obj_type, $id);
        $title = $folder->title ?: "Untitled folder";
        $up_link = $folder->parent_id ? 
          reverse_route($app, 'backend:subview:id', ['subview' => 'vault', 'id' => $folder->parent_id]) : 
          reverse_route($app, 'backend:subview', ['subview' => 'vault']);
        $title .= " <a href='{$up_link}'><i class='fa fa-level-up'></i></a>";
        $scope->title = "<i class='fa fa-folder'></i> ".$title;
      }
      $app->page->itemview = 'vault';
      $app->page->listview = 'vault';

      $filters = ['active' => 1, 'user_id' => $u_id];
      $filters['parent_id'] = $id ?: 0;
      //$filters[DB_LOGICAL_OR2] = ['user_id' => $app->user_id, 'artist_id' => $artist_ids]; 
      $filter_json = count($filters) ? json_encode($filters) : '';
      $scope->list_url = "/ajax.php?oper=get-rows&obj_type=gallery".($filter_json ? "&filters=".urlencode($filter_json) : '');
      break;

    case "network":
      $scope->section = 'network';
      $app->page->itemview = 'profile';
      $app->page->listview = 'network';
	    $scope->obj_type = $obj_type = "artist"; //Bala
      
      $filters = ['active' => 1, 'user.site_id' => $app->site->id];
      if($u_level < USER_LEVEL_ADMIN) $filters['user_id'] = $user_ids;
      $filter_json = count($filters) ? json_encode($filters) : '';
      $scope->list_url = "/ajax.php?oper=get-rows&obj_type=user&amp;sort=id%20desc&amp;".($filter_json ? "&filters=".urlencode($filter_json) : '');
      break;
    case "settings":
      $scope->section = 'settings';
      break;
    case "profile":
      $scope->section = 'network';
      $scope->obj_type = $obj_type = "artist";
      if(!$add) {   
        if(!$id) return controller_error($app, "Missing ID");
        if($_GET['profile_id']) $id = $scope->id = $scope->obj_id = get_object('user_profile', $id, 'user_id');
        $scope->profile = get_user($id);
        controller_artist_profile($app);
        $profile = $scope->profile;
      }

      $app->page->subtemplate = $sub = '_gallery-backend-profile.html'; // $edit ? '_gallery-backend-artwork-form.html' : '_gallery-artwork-detail.html';
      $scope->profile_template = $utpl = "_{$scope->template}";
      print_log("type={$obj_type} id={$obj_id} title={$profile->display_name} edit={$edit} auth={$auth_edit} sub={$sub} usertpl={$utpl}".dump($scope,true), 'backend', LOG_LEVEL_TEST);
      $scope->auth_edit_profile = in_array($profile->user_id, $artist_ids) || $scope->auth_edit_user; /** from controller_artist_profile */
      $app->page->itemview = 'profile';
      $app->page->listview = 'network';
      
      unset($scope->template);
      
      /**
      if($app->page->action == "edit") {
        die("edit profile $id");
      } else {
        $app->page->subtemplate = "_{$scope->template}";
        unset($scope->template);
      }
      */

      $data = db_getAll("select * from opening_hours where user_id='$id'");
      $scope->timing = $data;
      break;
      
    case "exhibitions":
      $scope->section = 'exhibition';
      $scope->obj_type = $obj_type = "media_collection";
      $app->page->itemview = 'exhibition';
      $app->page->listview = 'exhibitions';

      $filters = ['type' => COLLECTION_TYPE_EXHIBITION];
      if($u_level < USER_LEVEL_ADMIN) $filters['user_id'] = $app->user_id;
      $filter_json = count($filters) ? json_encode($filters) : '';
      $scope->list_url = "/ajax.php?oper=get-rows&obj_type=media_collection&amp;sort=id%20desc&amp;".($filter_json ? "&filters=".urlencode($filter_json) : '');
      
      break;
      
    case "exhibition":
      $scope->section = 'exhibition';
      $scope->obj_type = $obj_type = "media_collection";
      
      if(!$add) {                  
        if(!$id) return controller_error($app, "Missing ID");
        controller_gallery_collections($app);
        $obj = $scope->collection;
        $scope->auth_edit_collection = $auth_edit = $obj ? gallery_auth_media($obj_type, $obj, $u_id) : false;
        
        $curator = $scope->curator;
        
        // locations
        $locations = [];
        if($loc_id = $curator->location_id) {
          $locObj = get_object('location', $loc_id);
          $locations[$loc_id] = "{$locObj->address_street} {$locObj->address_number}";
        }

        $locObjs = find_objects('location', ['user_id' => $curator->user_id, 'active' => 1, 'parent_type' => ['media_collection', 'event']], ['debug_query' => 0]);
        foreach($locObjs as $locObj) {
          $loc_id = $locObj->id;
          $locations[$loc_id] = "{$locObj->address_street} {$locObj->address_number}";
        }
        $locations[0] = "Another location...";          
        $scope->locations = $locations;
        
        $scope->events = $events = find_objects('event', ['active' => 1, 'parent_type' => 'media_collection', 'parent_id' => $id]); //pradeepa
        
        $collection = get_object('media_collection', $id);
        $collection_id = $collection->id;
        
        $art = find_objects('media_collection_map', ['media_collection_id' => $collection_id, 'artwork_id' => ['>', 0]]);
        $media = find_objects('media_collection_map', ['media_collection_id' => $collection_id, 'artwork_id' => ATT_DEFAULT_NULL]);
        
        $art_ids = array_find_key($art, 'artwork_id');
        $media_ids = array_find_key($media, 'media_id');
        $artist_ids = array_unique(array_find_key(array_merge($art, $media), 'artist_id'));
  
        //print_log("art: ".dump($art,true), 'gallery', LOG_LEVEL_TEST);
        //print_log("media ".dump($media,true), 'gallery', LOG_LEVEL_TEST);
        print_log("art id: ".implode(',', $art_ids), 'gallery', LOG_LEVEL_TEST);
        print_log("media ".implode(',', $media_ids), 'gallery', LOG_LEVEL_TEST);
        print_log("map: artist id: ".implode(',', $artist_ids), 'gallery', LOG_LEVEL_TEST);
        
        if(!$collection->artist_ids) {
          $collection_artist_ids = find_objects('media_collection_map', ['active' => 1, 'media_collection_id' => $collection_id], ['fields' => 'distinct artist_id']);
          if($collection_artist_ids) {          
            $artists_json = $collection->artist_ids = json_encode($collection_artist_ids);
            update_object('media_collection', ['artist_id' => $artist_ids[0], 'artist_ids' => $artists_json], $collection->id);
            print_log("setting collection {$collection_id} artist_ids to {$artists_json}", 'backend', LOG_LEVEL_TEST);
            //$collection = get_object('media_collection', $collection_id); // reload
          }
        }
        
        $artwork_ids = $_GET['add_art'];
        $media_ids = $_GET['add_media'];
        
        if($artwork_ids || $media_ids) { // cannot be both
          $ids = $artwork_ids ?: $media_ids; 
          $what = $artwork_ids ? 'artwork' : 'media'; 
          $ids = explode(',', $ids);
          $changed = '';
          foreach($ids as $this_id) {
            $existing = $what == 'artwork' ? $art_ids : $media_ids;
            if(in_array($this_id, $existing)) continue; // already exists
            print_log("Adding {$what} {$this_id} to collection {$collection_id}", 'gallery', LOG_LEVEL_TEST);
            
            $result = $what == 'artwork' ? gallery_publish_artwork($siteObj, $this_id, $collection_id) : gallery_publish_image($siteObj, $this_id, $collection_id);
            if($error = $result->error ?: $result->errors) $errors = array_merge_smart($errors, $error);
            if($message = $result->message ?: $result->messages) $messages = array_merge_smart($messages, $message);
            
            $changed = $what;
            $mapObj = $result->data;
            $artist_id = $mapObj->artist_id;
            if(!in_array($artist_id, $artist_ids)) $artist_ids[] = $artist_id;
          }
          
          /** update artist list: keep existing artists NOT in the list that were explicitly added */
          $artist_ids = find_objects('media_collection_map', ['active' => 1, 'media_collection_id' => $collection_id, 'artwork_id' => ['>', 0]], ['fields' => 'distinct artist_id']);
          if($artist_ids) {   
            $existing_artist_ids = json_decode($collection->artist_ids) ?: [];
            
            print_log("map: artist ids: ".implode(',', $artist_ids), 'gallery', LOG_LEVEL_TEST);
            print_log("existing artist ids: ".implode(',', $existing_artist_ids), 'gallery', LOG_LEVEL_TEST);
            
            $artist_ids = array_unique(array_merge($existing_artist_ids, $artist_ids)); // keep existing
  
            print_log("combined artist ids: ".implode(',', $artist_ids), 'gallery', LOG_LEVEL_TEST);
            
            $artists_json = $collection->artist_ids = json_encode($artist_ids);
            print_log("combined artist json: {$artists_json}", 'gallery', LOG_LEVEL_TEST);
            
            update_object('media_collection', ['artist_id' => $artist_ids[0], 'artist_ids' => $artists_json], $collection->id);
          }
                    
          switch($changed) { // reload
            case 'artwork':
              $art = find_objects('media_collection_map', ['media_collection_id' => $collection_id, 'artwork_id' => ['>', 0]]);
              break;
            case 'media':
              $media = find_objects('media_collection_map', ['media_collection_id' => $collection_id, 'artwork_id' => ATT_DEFAULT_NULL]);
              break;
            default:
              break;
          }
          
        }
        
        /**
        $art_ids = array_find_key($art, 'artwork_id');
        $media_ids = array_find_key($media, 'media_id');
        print_log("2:art id: ".implode(',', $art_ids), 'gallery', LOG_LEVEL_TEST);
        print_log("2:media ".implode(',', $media_ids), 'gallery', LOG_LEVEL_TEST);
        
        $collection->artworks = $art;
        $collection->media = $media;
        */
                
        $scope->collection = $collection;
        
      }
      
      
      $app->page->subtemplate = $sub = '_gallery-backend-exhibition.html'; // $edit ? '_gallery-backend-artwork-form.html' : '_gallery-artwork-detail.html';
      $app->page->itemview = 'exhibition';
      $app->page->listview = 'exhibitions';

      print_log("type={$obj_type} id={$obj_id} title={$obj->title} edit={$edit} auth={$auth_edit} sub={$sub}", 'backend', LOG_LEVEL_TEST);
      
      $scope->error = $errors;
      $scope->message = $messages;
      break;
     
    default:
      return controller_error($app, "Unknown subview {$subview}");
      break;
  }
    
  print_log("controller_gallery_backend: sub={$subview} partial={$partial}", 'app', LOG_LEVEL_TEST);
  return;
}

 
// my-vault (CMS)           
function controller_art_explorer($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $ver = $app->version;
  $siteObj = $app->site;
  $scope = $app->page->scope;
  
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";
  $__header->styles[] = '/afr/css/gallery/afr_gmenu.css';
  $__header->styles[] = '/afr/bower/dynatree/dist/skin/ui.dynatree.css';

  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  if($app->language != 'en') $__header->base_scripts[] = '/afr/bower/jquery-validate/localization/messages_'.$app->language.'.js';
  $__header->base_scripts[] = "/afr/js/afr_cms.js?v=$ver";
  $__header->base_scripts[] = "/afr/js/afr_cms_dynatree.js?v=$ver";
  $__header->base_scripts[] = '/afr/bower/dynatree/dist/jquery.dynatree.min.js';
  $__header->scripts[] = '/afr/bower/jquery-zclip/jquery.zclip.js';
  $__header->scripts[] = '/afr/bower/jquery-validate/dist/jquery.validate.min.js';


  jfu_dependencies($app); // load scripts + css for new file uploader

  
  $scope = $app->page->scope;

  $scope->create_button = gallery_create_button($siteObj, $u_id);  
  $scope->vault = 1;  
  //$scope->explorer = site_page_edit_gallery($app, $options);
  return;
}

function controller_gallery_media($app, $options = []) {
  //die("controller_gallery_item id={$app->object_id}");
  $scope = $app->page->scope;
  $scope->obj_type = 'media';
  $app->page->template = 'gallery-media.html';
  die("Not implemented...");

  //controller_gallery_artworks($app, $options);
  return;
}

function controller_gallery_item($app, $options = []) {
  //die("controller_gallery_item id={$app->object_id}");
  $scope = $app->page->scope;
  $scope->obj_type = 'media_collection_map';
  $app->page->template = 'gallery-artworks.html';
  controller_gallery_artworks($app, $options);
  return;
}

function controller_gallery_artworks($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  $siteObj = $app->site;
  
  $scope = $app->page->scope;
  $obj_id = $scope->obj_id = $scope->obj_id ?: $app->object_id;
  $obj_type = $scope->obj_type = $scope->obj_type ?: "artwork"; // $app->object_type;
  
  $parent_obj_type = $scope->parent_obj_type;
  $parent_obj_id = $scope->parent_obj_id;
  
  
  $tpl = $app->page->template;
  //die("tpl=$tpl");
  print_log("controller_gallery_artworks: $obj_type/$obj_id parent: $parent_obj_type/$parent_obj_id", 'gallery', LOG_LEVEL_TEST);
  
  
  // When coming from plug-in, we don't have map id. Find it
  if($obj_type == 'media' && $obj_id && $parent_obj_type == 'media_collection' && $parent_obj_id) {
    if($mapObj = find_object('media_collection_map', ['media_collection_id' => $parent_obj_id, 'media_id' => $obj_id])) {
      print_log("controller_gallery_artworks: found map:".dump($mapObj, true), 'gallery', LOG_LEVEL_TEST);
      $obj_id = $scope->obj_id = $mapObj->id;
      $obj_type = $scope->obj_type = 'media_collection_map';      
    }
  }
  
  $scope->my_collection_id = $scope->user_collection_id = $user_collection_id = $u_id ? gallery_find_user_collection($siteObj, $u_id, array('return' => 'id')) : 0;
  $scope->follow_type = "media";

    
  if($obj_id) {
    $obj = get_object($obj_type, $obj_id);
    
    $scope->mail_controller = 'gallery_artworks';
    $scope->mail_template = 'mail/mail-artwork.html';
    
    $scope->obj_id = $obj_id;

    if(!in_array($obj_type, ['media_collection_map', 'artwork'])) return scope_error($app, "Illegal type $obj_type", '404');
    
    $collection_id = $curator_id = $mapObj = $collectionObj = null;
    
    /** map - gives us collection to work with */
    if($obj_type == 'media_collection_map') {
      $mapObj = get_object('media_collection_map', $obj_id);
      $obj_id = $artwork_id = $mapObj->artwork_id;
      if($collection_id = $mapObj->media_collection_id) $collectionObj = get_object('media_collection', $collection_id);
      
      $curator_id = $mapObj->curator_id;
      if($artwork_id) {
        $obj_type = 'artwork';
        $related = gallery_item_related($siteObj, $mapObj);
      } else {
        return; /** no more processing needed */
      }
      //if(!$artwork_id) return scope_error($app, "Missing Artwork ID in map", '404');
    } else {
      $related = gallery_artwork_related($siteObj, $obj);
    }
    
    /** must be artwork */
    $artwork_id = $obj_id;
    
    $scope->artwork = $artworkObj = get_object('artwork', $artwork_id);
    $scope->artist_id = $artist_id = $artworkObj->artist_id;
    $scope->media_id = $media_id = $scope->artwork->title_image = $artworkObj->title_image; 

    
    foreach($related as $k => $v) $scope->$k = $v;
    if(!$collectionObj) $collectionObj = $related->exhibition;
    if(!$curator_id) $curator_id = $related->curator_id;
    
    $start = 0;
    $limit = 12;
    if($curator_id) {
      $scope->curator_exhibitions = $curator_exhibitions = find_objects('media_collection', 
          array('user_id' => $curator_id, 'type' => MEDIA_COLLECTION_TYPE_EXHIBITION), 
          array('active' => 1, 'start' => $start, 'limit' => $limit));
      $scope->curator_artworks = gallery_unique_artworks(array_find_key($curator_exhibitions, 'id'), ['start' => $start, 'limit' => $limit]);
    }

    
    if($artist_id) {
      $scope->artist_name = $artist_name = user_display_name($artist_id);
      $scope->artist_projects = $artist_projects = find_objects('media_collection', 
        array('user_id' => $artist_id, 'type' => MEDIA_COLLECTION_TYPE_PROJECT), 
        array('active' => 1, 'active' => 1, 'start' => $start, 'limit' => $limit));
      //$scope->artist_exhibitions = $artist_exhibitions = find_objects('media_collection', array('user_id' => $curator_id, 'type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'active' => 1));;

      if($project_ids = array_find_key($artist_projects, 'id')) {
        $scope->artist_artworks = gallery_unique_artworks($project_ids, ['start' => $start, 'limit' => $limit]);          
        //$scope->artist_artworks = $artist_artworks = find_objects('media_collection_map', array('media_collection_id' => $project_ids, 'active' => 1));
      }
      
      $artist_exhibitions_ids = find_objects('media_collection_map', 
        array('artist_id' => $artist_id, 'collection_type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'active' => 1), 
        array('fields' => 'distinct media_collection_id', 'active' => 1), array('start' => $start, 'limit' => $limit));
      
      $scope->artist_exhibitions = find_objects('media_collection', 
        array('id' => $artist_exhibitions_ids, 'active' => 1), array('start' => $start, 'limit' => $limit));
    }
    
    if($collectionObj) {
      if($collectionObj && $collectionObj->active && in_array($collectionObj->type, [COLLECTION_TYPE_EXHIBITION,COLLECTION_TYPE_PROJECT])) {
        
        if($loc_id = $collectionObj->location_id) {
          $locObj = get_object('location', $loc_id);
          $scope->exhibition->location = $locObj;
        }
    
        if($event_id = $collectionObj->event_id) {
          $eventObj = get_object('event', $event_id);
          $scope->exhibition->event = $eventObj;
        }
      }
      $scope->collection = $collectionObj; /** why are we using collection and exhibition for scope? */
    }
    
    //$artwork = $scope->artwork;
    $title = $artworkObj->name ?: "Untitled";
    $title .= " by $artist_name";
    $description = implode('. ', array_filter(array($artworkObj->original_technique, $artworkObj->short_description, $artworkObj->comment)));
    $scope->follow_id = $artworkObj->media_id;

    if($u_id && isset($_GET['follow']) && $user_collection_id && $media_id) { /** follow this artwork */
      $follow = $_GET['follow'] ? 1 : 0;
      if($mapObj = find_object('media_collection_map', array('media_id' => $media_id, 'media_collection_id' => $user_collection_id))) {
        if(!$follow) delete_object('media_collection_map', $mapObj->id);
        /** do nothing, already following */
      } else {
        if($follow) $result = gallery_publish_image($siteObj, $media_id, $user_collection_id);
      }    
    }
    
    $img_src = $scope->main_image = print_media($artworkObj->media_id, array('source_only' => true, 'full' => true));
    
    /** Add facebook Open Graph tags */
    $__header->meta[] = array('property' => 'og:type', 'content' => 'article');
    $__header->meta[] = array('property' => 'og:image', 'content' => add_https($img_src));
    $__header->meta[] = array('property' => 'og:url', 'content' => $scope->url ?: this_url());
    $__header->meta[] = array('property' => 'og:title', 'content' => $title);
    $__header->meta[] = array('property' => 'og:description', 'content' =>  strip_tags($description));    
    //$__header->meta[] = array('property' => 'article:author', 'content' =>  "Some Dude");    
    $__header->meta[] = array('property' => 'article:author', 'content' =>  "https://www.facebook.com/vpatinabv");    

    $__header->link[] = array('property' => 'image_src', 'content' =>  add_https($img_src));
    
    $app->page->author = $artist_name;    
    $app->page->description = strip_tags($description);   
    $app->page->title = $title;    
    
    /** check if this artwork is in user's collection */
    $scope->collected = $scope->my_collection_id = 0;
    $media_id = $scope->artworkObj->media_id;    
    if($u_id && $media_id) {
      $scope->collected = $collected = find_object('media_collection_map', array('media_collection_id' => $user_collection_id, 'active' => 1, 'media_id' => $media_id), array('fields' => 'id')) ? 1 : 0;
    }
    
    $scope->artist = $artistObj = $artist_id ? get_user($artist_id) : null;
  } elseif($objects = $scope->objects) { /** from ajax.php?oper=gallery-get-collection */
    $scope->items = $items = $scope->objects;
    /** all collected images */
    $scope->user_collection = $user_collection_id ? find_objects('media_collection_map', array('media_collection_id' => $user_collection_id, 'active' => 1), array('fields' => 'distinct (media_id)', 'sort' => 'id DESC')) : array();      
    
  } else { /** browse artworks: todo */
  }

  
  return;
}
                               
function controller_plugin($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";

  
  $scope = $app->page->scope;
  $obj_id = $media_id = $scope->obj_id ?: $app->object_id;
  $obj_type = $scope->obj_type ?: "plugin";
  if($obj_type != 'plugin') return controller_error($app, "Missing ID or Illegal type $obj_type");

  if($obj_id) {
    $obj = $plugin =$scope->plugin = get_object($obj_type, $obj_id);
    if(!$obj) return controller_error($app, "Could not find $obj_type $obj_id");
  }

  if($obj) {
    $parent_type = $plugin->parent_type;
    $parent_id = $plugin->parent_id;
  } else {
    $parent_type = pick_first($_GET['pt'], $scope->parent_type, $options['parent_type']);
    $parent_id = pick_first($_GET['pid'], $scope->parent_id, $options['parent_id']);
  }
  
  if($parent_type && $parent_id) {
    $scope->parent_object = $parent_object = get_object($parent_type, $parent_id);
    if(!$parent_object) return controller_error($app, "Could not find $parent_type $parent_id");

    $scope->parent_type = $parent_type;
    $scope->parent_id = $parent_id;
    
    $scope->plugins = $plugins = find_objects('plugin', array('parent_id' => $parent_id, 'parent_type' => $parent_type));
    $scope->plugin_count = count($plugins);

    
    if($parent_type == 'media_collection') {
      $scope->collection = $parent_object;
      $scope->preview = plugin_javascript(['shid' => $plugin->id]);
      $scope->preview_html = sharing_output(['shid' => $plugin->id]);

      $scope->images = $images = find_objects('media_collection_map', ['media_collection_id' => $parent_id, 'active' => 1], ['sort' => 'rank']);
    }
  }
  
  return;
  
}


function controller_gallery_folder($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  
  $scope = $app->page->scope;
  $obj_id = $media_id = $scope->obj_id ?: $app->object_id;
  $obj_type = $scope->obj_type ?: "gallery";

  if(!$obj_id || $obj_type != 'gallery') return scope_error($app, "Missing ID or  Illegal type $obj_type", '404');

  $obj = get_object($obj_type, $obj_id); 
  if(!$obj) {
    $scope->error = "Could not find $obj_type $obj_id";
    $scope->template = "404";
  }
  $auth_edit = $scope->auth_edit = gallery_auth_media($obj_type, $obj, $u_id); 
  
  $scope->artist_id = $artist_id = $obj->user_id;
  $scope->artist_name = $artist_name = user_display_name($artist_id);
  $scope->artist = $artistObj = $artist_id ? get_user($artist_id) : null;
  $scope->obj_id = $scope->folder_id = $obj_id;
  $scope->folder = $obj;
  $scope->artworks = find_objects('media', array('parent_type' => $obj_type, 'parent_id' => $obj_id, 'active' => 1));
  
  return;
}

function controller_gallery_collections($app, $options = []) {
  global $u_id;
  $__header = $app->header; 
  $__header->styles[] = "/afr/css/gallery/afr_gallery.css?ver={$app->version}";
  $__header->base_scripts[] = "/afr/js/afr_gallery.js?ver={$app->version}";
  $siteObj = $app->site;
  

  //die("tpl={$app->page->template}");
  
  $scope = $app->page->scope;
  $scope->items = $scope->objects;  
  // unset($scope->objects);
  $obj_id = $media_id = $scope->obj_id = $scope->obj_id ?: $app->object_id;
  $obj_type = $scope->obj_type = $scope->obj_type ?: $app->object_type;
  
  $start = 0;
  $limit = 12;
  
  print_log("sot: {$scope->obj_type} tos {$app->object_type} => {$obj_type}", 'gallery', LOG_LEVEL_TEST);
  $scope->my_collection_id = $scope->user_collection_id = $user_collection_id = $u_id ? gallery_find_user_collection($siteObj, $u_id, array('return' => 'id')) : 0;
  $scope->follow_type = $obj_type;
  if($obj_id) {
    $scope->follow_id = $obj_id;
    $scope->user_collection = $user_collection_id ? find_objects('media_collection_map', array('media_collection_id' => $user_collection_id, 'active' => 1), array('fields' => 'distinct (media_id)')) : array();
    $scope->collection_id = $scope->obj_id = $collection_id = $obj_id;
    $scope->collection = $collectionObj = get_object('media_collection', $obj_id);
    $scope->mail_template = 'mail/mail-collection.html';
    $scope->mail_controller = 'gallery_collections';

    /** find artwork in collection */
    $scope->artworks = $artworks = find_objects('media_collection_map', array('media_collection_id' => $collection_id, 'active' => 1), array('sort' => 'rank'));

    if($collectionObj->type == MEDIA_COLLECTION_TYPE_EXHIBITION) {
      if($scope->curator_id = $curator_id = $collectionObj->user_id) $scope->curator = get_user($collectionObj->user_id);
      $scope->curator_exhibitions = $curator_exhibitions = find_objects('media_collection', 
          array('id' => array('!=', $collection_id), 'user_id' => $curator_id, 'type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'active' => 1),
          array('start' => $start, 'limit' => $limit));
      $scope->curator_artworks = gallery_unique_artworks(array_find_key($curator_exhibitions, 'id'),['start' => $start, 'limit' => $limit]);
    }

    $scope->url = site_full_url($siteObj).page_link(PAGE_GALLERY, 'media_collection', $collectionObj->type).slugify($collectionObj->title)."~".$obj_id;  

    $curator_id = $collectionObj->user_id;
    
    $mid = isset($_GET['mid']) ? $_GET['mid'] : 0;
    if($mid) {
      $artwork = get_object('media', $mid);
      $author_name = user_display_name($artwork->artist_id);
      $title = $artwork->name ?: "Untitled Artwork";
      $title .= " by $author_name";
      $description = implode('. ', array_filter(array($artwork->original_technique, $artwork->short_description, $artwork->comment)));

    } else {
      $author_name = user_display_name($curator_id);
      $title = $collectionObj->title ?: "Untitled Exhibition";
      $title .= " by $author_name";
      $description = $collectionObj->description;
    }

    if($loc_id = $collectionObj->location_id) {
      $locObj = get_object('location', $loc_id);
      $scope->collection->location = $locObj;
    }

    if($event_id = $collectionObj->event_id) {
      $eventObj = get_object('event', $event_id);
      $scope->collection->event = $eventObj;
    }
    
    $img_src = $mid ? print_media($mid, array('source_only' => true, 'full' => true)) : collection_title_image($collectionObj);
  
    /** Add facebook Open Graph tags */
    $__header->meta[] = array('property' => 'og:type', 'content' => 'article');
    $__header->meta[] = array('property' => 'og:image', 'content' => add_https($img_src));
    $__header->meta[] = array('property' => 'og:url', 'content' => $scope->url ?: this_url());
    $__header->meta[] = array('property' => 'og:title', 'content' => $title);
    $__header->meta[] = array('property' => 'og:description', 'content' =>  strip_tags($description));    
    //$__header->meta[] = array('property' => 'article:author', 'content' =>  "Some Dude");    
    $__header->meta[] = array('property' => 'article:author', 'content' =>  "https://www.facebook.com/vpatinabv");    
  
    $__header->link[] = array('property' => 'image_src', 'content' =>  add_https($img_src));    
    
    $app->page->author = $author_name;    
    
    $app->page->description = strip_tags($description);
    $app->page->title = $title;    
    
  } else { /** find collections */
    //$scope->error = "got here type=$type artist_id=$artist_id".dump($options, true);
    //return;
    
    if($items = $scope->items) { // my-vpatina
      //$myCollectionObj = gallery_find_user_collection($siteObj, $u_id);
      //$scope->my_collection_id = $collection_id = $myCollectionObj->id;
      //$scope->items = $scope->objects ?: gallery_collection_images($siteObj, $collection_id, array('return' => 'object'));
      //$scope->error = "got here id=$obj_id count=".count($items);
    } else { // my-vault
      $collection_type = $scope->type ?: MEDIA_COLLECTION_TYPE_EXHIBITION;
      if(!$collection_type) {
        $scope->error = "Missing collection type";
        $scope->template = "404";
        return;    
      }
      $search = array('type' => $collection_type, 'active' => 1);
      $user_id = pick_first($scope->artist_id, $scope->user_id, $u_id);    
      $search['user_id'] = $user_id;
      $scope->items = $items = find_objects('media_collection', $search);
      $scope->vault = 1;
    }
    
  }

  $scope->auth_edit = $auth_edit = $obj_type && $obj_id ? gallery_auth_media($obj_type, $obj_id, $u_id) : false; 
 //  update location set location_bounds ='[["36.6504","-6.1641"],["36.6684","-6.1461"]]' where id=8746;
  if($auth_edit && $obj_type == 'media_collection') {
    if($sharing_id = $scope->sharing_id) { // edit single
      $scope->sharing = get_object('sharing', $sharing_id);
      if($scope->preview) {
        $scope->sharing_preview = sharing_output(array('shid' => $sharing_id), array('preview' => 1));
      }
    } else { // add
      $scope->sharing = $sharing =  find_objects('sharing', array('parent_id' => $obj_id, 'parent_type' => $obj_type));
      $scope->sharing_count = count($sharing);
    }
  }
  
  print_log("eot: {$scope->obj_type}", 'gallery', LOG_LEVEL_TEST);
  
  // unset($scope->objects);
  return;
}

function controller_test($app, $options = []) {
  $scope = $app->page->scope;
  $scope->debug = 'some debug message';  
  $scope->error = 'some random error';  
  $scope->message = ['message1', 'message2'];  
  //$scope->redirect = 'http://www.google.com';
  //$scope->template = 'test2';
  return;
}

/** End vPatina + Art controllers */

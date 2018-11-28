<?php

/** still in use 2017, but probably shouldn't be */

function site_data_directory($siteObj_or_id) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $site_shortname = url_safe($siteObj->shortname);
  return $_SESSION['live'] ? "/web/data/$site_shortname/" : "/web/data-dev/$site_shortname/";
}


// takes path parts (from set_env), return page if found
function path2page($siteObj, $path_parts, $options = array()) {
  $debug = $options['debug'];
  $pageObj = null;
  $path_var = '';
  $this_path = count($path_parts) ? '/'.implode('/', $path_parts).'/' : '/';
  $full_path = this_path();
  if(count($path_parts) > 1 ) {
    $last_part = array_pop($path_parts);
    $this_path_min1 = '/'.implode('/', $path_parts).'/';
  } else {
    $last_part = array_pop($path_parts);
    $this_path_min1 = '';
  }
  if($debug) {
    print_log("Looking for ".dump($path_parts, true), 'set_env', LOG_LEVEL_TEST);
    print_log("This path= $this_path", 'set_env', LOG_LEVEL_TEST);
  }

  if($this_path == '/' && $script_lang && ($pageObj = site_home_page($siteObj, $script_lang))) { // special case for home page    
    if($debug) print_log("Ok 1", 'set_env', LOG_LEVEL_TEST);
  } elseif($pageObj = site_find_page($siteObj, $this_path)) {    
    if($debug) print_log("Ok 2", 'set_env', LOG_LEVEL_TEST);
  } elseif($this_path_min1 && ($pageObj = site_find_page($siteObj, $this_path_min1))) { // maybe last path part is a property name or other variable?
    $path_var = $_SESSION['path_var'] = $last_part;
    if($debug) print_log("Ok 3", 'set_env', LOG_LEVEL_TEST);
  }
  return array($pageObj, $path_var);
}


function site_find($domain) {
  $siteObj =  find_object('site', array('domain' => $domain));
  return $siteObj;
}

// finds the home page for given language
function site_home_page($siteObj_or_id, $lang='') {
  if(!$lang) $lang = $_SESSION['lang'];
  $siteObj = any2obj('site', $siteObj_or_id);
  $homePageObj = find_object('site_page', array('site_id' => ($siteObj->parent_site_id ?: $siteObj->id), 'page_type' => PAGE_HOME)); // home page
  return site_find_content($siteObj, $homePageObj, $lang);
}


// returns site-page merged with site-content for given path and site  
function site_find_page($siteObj, $path) {
  $contentSearch['path'] = $path;
  $contentSearch['page_content.site_id'] = $siteObj->id;
  $sort = "page_content.site_id DESC";
  //dump($contentSearch);
  if($pageContentObj = find_object('page_content', $contentSearch, '', array('sort' => $sort))) {
    //dump($pageContentObj);
    return site_page_object($pageContentObj);
  }
  return NULL;
}

// finds content for this page (for correct site and language)
function site_find_content($siteObj_or_id, $pageObj_or_id, $lang='') {

  $site_id = any2id('site', $siteObj_or_id);
  $page_id = any2id('site_page', $pageObj_or_id);  
  if($pageContentObj = find_object('page_content', ['site_id' => $site_id, 'page_id' => $page_id, 'language' => $lang ?: $_SESSION['language']])) return site_page_object($pageContentObj, $pageObj);
  return any2obj('site_page', $pageObj_or_id);
}


// full URL of any site
// without trailing slash
function site_full_url($siteObj_or_id, $trailing_slash = false) {    
  $siteObj = any2obj('site', $siteObj_or_id);
  $url = rtrim(site_url($siteObj), '/');
  $url = $trailing_slash ? "{$url}/" : $url; // add trailing if set  
  if(strpos($url, 'http') === false) {
    $protocol = $siteObj->secure ? 'https' : 'http'; 
    $url = "{$protocol}://{$url}";
  }
  return $url;
}

function site_url($siteObj_or_id, $options = []) {  
  $siteObj = any2obj('site', $siteObj_or_id);
  if($siteObj->id == $_SESSION['site_id']) return $_SERVER['HTTP_HOST'];
  if(!$options['live']) {
    if($_SESSION['dev']) return 'dev.'.$siteObj->domain;
    if($_SESSION['test']) return 'test.'.$siteObj->domain;
  }
  return $siteObj->url;
}

/** no longer in use */
function site_docroot($siteObj_or_id) {  
  $siteObj = any2obj('site', $siteObj_or_id);
  $domain = $_SESSION['live'] ? 'www.'.$siteObj->domain : 'test.'.$siteObj->domain;
  return "/websites/{$domain}";
}

// searches for page by type (or name) for given site (obj or id)
// returns first found if multiple hits (by default sort)
// if page_type is a string instead of numeric, it searches by name instead of page type 
function site_find_pagetype($siteObj, $page_type, $lang='', $options=NULL) {
  
  $pageSearch = $options['search'];
  $pageSearch['site_id'] = $siteObj->parent_site_id ?: $siteObj->id;
  $pageSearch['active'] = 1;

  if(isint($page_type)) $pageSearch['page_type'] = $page_type;
  else $pageSearch['name'] = $page_type;


  $pageObj = find_object('site_page', $pageSearch);
  $result = $pageObj;
  
  if($pageObj && $lang) { // return content with corresponding language
    $contentSearch['page_id'] = $pageObj->id;
    $contentSearch['page_content.site_id'] = $siteObj->id;
    $contentSearch['language'] = $lang;
    if($pageContentObj = find_object('page_content', $contentSearch)) $result =  site_page_object($pageContentObj, $pageObj);
  }
  if($field = $options['field']) return $result->$field;
  return $result;
}

/** smarty alias for get_page, returns array instead of object */
function smarty_get_page($pageObj_or_id, $lang='') {
  return (array) get_page($pageObj_or_id, $lang);
}

function smarty_page_content($page_type, $options=[]) {
  return (array) page_content($page_type, $lang);
}

/** alias for site_find_content */
function get_page($pageObj_or_id, $lang='') {
  global $__SITE;
  return site_find_content($__SITE, $pageObj_or_id, $lang);
}

/** new: used by Smarty; find by page_type, optionally language/object_type/object_subtype */
function page_content($page_type, $options=[]) {
  global $__SITE, $__LANG;
  $pSearch['site_id'] = $__SITE->id;
  $pSearch['page_type'] = $page_type;
  $pSearch['active'] = 1;
  if($obj_type = $options['object_type']) $pSearch['object_type'] = $obj_type;
  if($obj_subtype = $options['object_subtype']) $pSearch['object_subtype'] = $obj_subtype;
  $pageObj = find_object('site_page', $pSearch);

  if(!$pageObj) return null; /** throw exception */
  
  $lang = pick_first($options['language'], $options['lang'], $__LANG, $__SITE->default_language);
  $cSearch = array('page_id' => $pageObj->id, 'language' => $lang);
  
  $pcObj = find_object('page_content', $cSearch, $options['field'] ?: $options['fields']);
  return $pcObj;
}

/** new: used by Smarty; find by page_type/object_type/object_subtype */
// todo: put obj_type/subtype in options
function page_link($page_type, $obj_type='', $obj_subtype='', $options=[]) {
  if($obj_type) $options['object_type'] = $obj_type;
  if($obj_subtype) $options['object_subtype'] = $obj_subtype;  
  $pageObj = page_content($page_type, $options);
  return $pageObj->path;
}


/** new: used by Smarty; print html_link with name/path given page_id */
/** optional: full,  */
function print_page_link($page_id, $options = array()) {
  $pageObj = get_page($page_id);
  $qs = $options['qs'] ? "?{$qs}" : "";
  return html_link($pageObj->path, pick_first($pageObj->menu_title, $pageObj->title, $pageObj->name)).$qs;
}

// return links for given site/page_type/language if found
function site_pagetype_link($site, $page_type, $lang='', $linkonly=true, $full=false) {
  $siteObj = any2obj('site', $site);
  $pSiteObj =  ($pid = $siteObj->parent_site_id) ? get_object('site', $pid) : $siteObj;
  $pageObj = site_find_pagetype($pSiteObj, $page_type);
  return $pageObj ? site_page_link($siteObj, $pageObj, array('linkonly' => $linkonly, 'full' => $full, 'lang' => $lang)) : '';  
}

// return links for given site/page_object/language if found
function site_page_link($siteObj_or_id, $pageObj_or_type, $options=NULL) {
  global $__LANGUAGES, $__LANG;
  $siteObj = any2obj('site', $siteObj_or_id);
  
  $lang = pick_first($options['lang'], $options['language'], $__LANG);
  $geoObj = $_SESSION['geoObj'];
  $linkonly = isset($options['linkonly']) ? $options['linkonly'] : true;
  $full = $options['full'];
  $pageObj = isint($pageObj_or_type) ? site_find_pagetype($siteObj, $pageObj_or_type, $lang, $options) : $pageObj_or_type; 

  $site_langs = $__LANGUAGES;
  if(!$lang || !in_array($lang, $site_langs)) $lang = $_SESSION['lang'] ?: $siteObj->default_language;
  
  if($gdir = ($options['city_dir'] ?: $options['geo_dir'])) {
    $geodir = '/'.trim($gdir, '/').'/';
    //echo("spl: cd=$geodir<br>");
  } elseif($pageObj->relative && $geoObj) {
    $geodir = loc_link($geoObj->level, $geoObj->fid, $lang, true, true);    
  }

  //dump($geoObj);echo("geodir = $geodir<br>");
  $content = isset($pageObj->lang) && $lang == $pageObj->lang ? $pageObj : site_find_content($siteObj, $pageObj, $lang);
  if($content->site_id != $siteObj->id) { // alternate site    
    $siteObj = get_object('site', $content->site_id);
    $full = true;
  }
  
  if(!$content) {
    return '';
  }
  
  $link = $content->path;
  if($pageObj->relative && $geodir) {
    if(strpos($link, "/$lang/") === 0) $link = '/'.$lang.$geodir.substr($link, 4); // put lang first like /nl/rotterdam/path/
    else $link = $geodir.ltrim($link, '/');
  }
  if($link[0] != '/') $link = "/$link"; // always start with slash
  $aid = $options['aff_id'] ?: $_SESSION['aff_id'];
  $aff_type = $options['aff_type'] ?: $_SESSION['aff_type'];
  if($aid) { // add affiliate prefix if needed
    //$prefix = $_SESSION['showing_partners'] ? 'aidp' : 'aid';
    if(strpos($link, "aid$aff_type$aid") === FALSE) $link = "/aid$aff_type$aid".$link;                                
  }
  $link = clean_path($link);
  if($full) {
    $link = site_url($siteObj, $options).$link;
    $link = add_http($link, $siteObj->secure);
  }
  return $linkonly ? $link : html_link($link, $options['name'] ?: $content->menu_title);
}

// $p can be a page_type (int), pageObj (object or array), or pageObj->name (string)
function get_page_link($p, $linkonly=true, $lang="", $name="", $city_dir='', $full=false) {
  global $__SITE;
  $siteObj = $__SITE;
  if($pid = $siteObj->parent_site_id) $siteObj = get_object('site', $pid);
  if(!$lang) $lang= $_SESSION['language'] ?: $siteObj->default_language;
  //dump($p);
  if(isint($p)) {
    $pageObj = site_find_pagetype($siteObj, $p); // received page_type
  } else {
    if(is_array($p)) $p = array2obj($p);
    $pageObj = is_object($p) ? $p : find_object('site_page', array('site_id' => $siteObj->id, 'name' => $p));
  }
  if(!$pageObj) return '';
  $options['linkonly'] = $linkonly;
  $options['full'] = $full;
  $options['lang'] = $lang;
  $options['name'] = $name;
  $options['city_dir'] = $city_dir;
  //dump($options);
  return site_page_link($__SITE, $pageObj, $options);
}

// returns parent, or parent of parent etc until given level (0 is root)                                                      
function site_get_ancestor($siteObj, $pageObj, $level=0) {
  if($level == '') $level = 0;
  $ancestorObj = $pageObj;
  while($ancestorObj->level > $level) {
    //dump($ancestorObj);    
    // die("al=$ancestorObj->level level=$level");
    $ancestorObj = get_object('site_page', $ancestorObj->parent_id);
  }
  return site_find_content($siteObj, $ancestorObj, $_SESSION['lang']) ?: $ancestorObj;
}


// returns the id for an editable element
function site_edit_id($siteObj_or_id, $pageObj_or_id, $obj_type, $fields, $id, $options=NULL) {
  global $__AUTH_EDIT;
  $site_id = is_object($siteObj_or_id) ? $siteObj_or_id->id : $siteObj_or_id;
  $page_id = is_object($pageObj_or_id) ? $pageObj_or_id->id : $pageObj_or_id;
  
  if(is_array($fields)) $fields = implode('|', $fields);
  if(!$__AUTH_EDIT) return $options['id'] ?: "";
  

  $action = $options['action'] ?: 'edit';
  $site_id = $options['site_id'] ?: $site_id;
  $page_id = $options['page_id'] ?: $page_id;
  $lang    = $options['language'] ?: $_SESSION['language'];
  $parent_id = ($options['parent_id'] ?: $options['data']['parent_id']) ?: 0;
  $type = $options['type'] ?: 0;

  $id = implode('-', array($action,$obj_type,$fields,$id,$site_id,$page_id,$lang, $parent_id, $type));
  return $id;
  
}

function redirect_login($ref='') {
  global $__SITE;  
  if($_GET['_new'] == "register") {
    unset($_GET['_new']);
    $login_page = site_page_link($__SITE, PAGE_REGISTER);      
  } else {
    $login_page = site_page_link($__SITE, PAGE_LOGIN);
  }
  
  if(!$ref) $ref = $_SESSION['referring_page'] = ($_SERVER['REDIRECT_URL  '] ?:$_SERVER['REQUEST_URI']);
  $ref = str_replace('act=logout', '', $ref);
  
  print_log("Redirect Login stack=".caller_stack(), 'api', LOG_LEVEL_TEST);
  
  redirect("{$login_page}?ref=".urlencode($ref));    
  die("<a href='/admin/login.php'>Please log in to access this page</a>");                                                        
}


// returns page_content merged with parent page
function site_page_object($content, $pageObj=NULL) {
  $pageContentObj = is_object($content) ? clone $content : get_object('page_content', $content);      
  if(!$pageObj) $pageObj = get_object('site_page', $pageContentObj->page_id); 
  return site_merge_page_object($pageObj, $pageContentObj);
}                            

// merges content fields from contentPageObj with pageObj
function site_merge_page_object($pageObj, $pageContentObj) {
  $mergedPageObj = is_object($pageObj) ? clone $pageObj : $pageObj;
  if(!$pageContentObj) return $mergedPageObj;
  foreach($pageContentObj as $k => $v) {
    if($k == 'id') $k = 'content_id';
    $mergedPageObj->$k = $v; // merge as one
  }
  return $mergedPageObj;
}

/** new edit menu */
function site_edit_menu($siteObj, $pageObj, $options = array()) {
  if(!$siteObj || !is_object($siteObj)) return '';
  if(!$pageObj || !is_object($pageObj)) return '';
  
  $url = this_url();
  
  $buttons = array();
  if($_SESSION['__edit_mode']) {
    $buttons[] = html_link(replace_query_var($url, '__edit_mode', 0), "<i class='fa fa-times-circle'></i> Exit Edit Mode", array('class' => 'btn btn-danger __edit_site __edit_on'));
  } else {
    $buttons[] = html_link(replace_query_var($url, '__edit_mode', 1), "<i class='fa fa-pencil-square-o'></i> Edit Content", array('class' => 'btn btn-success __edit_page'));
    $buttons[] = html_link("#page-form-modal", "<i class='fa fa-plus-square'></i> Add Page", array('class' => 'btn btn-success __add_page reset-form', 'data-toggle' => "modal", 'data-backdrop' => "static", 'data-form' => '#page-form'));
    $clone = clone $pageObj;
    unset($clone->content);    
    $buttons[] = html_link("#page-edit-form-modal", "<i class='fa fa-file-text-o'></i> Edit Page", array('class' => 'btn btn-success __edit_page fill-form', 'data-toggle' => "modal", 'data-backdrop' => "static", 'data-form' => '#page-edit-form', 'data-data' => $clone));
    
    $children = find_objects('site_page', array('parent_id' => $pageObj->id));
    if($pageObj->page_type != PAGE_HOME && !$children) 
      $buttons[] = html_link("#", "<i class='fa fa-times'></i> Delete Page", 
         array('class' => 'btn btn-danger __delete_page', 
           'data-obj_type' => 'site_page', 'data-obj_id' => $pageObj->id,
           'data-toggle' => 'confirmation', 'data-placement' => 'bottom', 'data-on-confirm' => 'delete_object', 'data-redirect' => '/'
           ));

    $edit_site_url = site_page_link($siteObj, PAGE_EDIT_SITE);
    $buttons[] = html_link($edit_site_url, "<i class='fa fa-sitemap'></i> Edit Site", array('class' => 'btn btn-success __edit_site'));
    $is_writable = $is_file = false;
    if($template = $pageObj->template) {
      //list($filename, $ext) = split_filename($template);
      $filename = str_replace(".html", "", $template);
      $file = clean_path(__DIR__."/pages/$filename".'.html');    
      $is_writable = is_writable($file);
      $is_file = is_file($file);
      if($is_file && $is_writable) {
        $buttons[] = html_link("#", "<i class='fa fa-code'></i> Edit Template", array('class' => 'btn btn-success __edit_template', 'data-template' => $file));
      }
    }
    $text = "";    
    if($pageObj->template) {
      $text .= "Template: $pageObj->template (";
      $text .= $is_file ? "File exists" : html_span("$file does not exist template=$template", 'red');
      if($is_file) $text .= $is_writable ? " and is writable" : " but is not writable";
      $text .= ") ";
    }
    //if($count = count($children)) $text .= " Page has $count children (cannot be deleted)";
    if($pageObj->controller) $text .= " Controller: $pageObj->controller";
    $text = html_span($text, "margin-left text-success", array("style" => ""));

  }
  

  $output = implode(PHP_EOL, $buttons);
  if($text) $output .= html_div($text);
  //$output = "Foo";
  return html_div($output, 'hidden abs_top_left-off', array('id' => "site_edit_panel"));
}

/** new forms for editing page, later site */
function site_edit_forms($app, $options = array()) {
  global $u_level, $u_id;
  $auth_edit = $u_id && $u_level >= USER_LEVEL_STAFF;
  if(!$auth_edit) return '';
    
  $page_templates = array();
  if($template = $siteObj->template) {
    $template_dir = "/web/templates/{$template}/stripped/";
    $page_templates = file_array($template_dir);
  }

  $forms = '';
  $forms .= smarty_parse_template($app, 'afr/pages/page-form.html', array('page_templates' => $page_templates));    
  $forms .= smarty_parse_template($app, 'afr/pages/page-edit-form.html', array('page_templates' => $page_templates));    
  
  //list($filename, $ext) = split_filename($pageObj->template);
  $template = $pageObj->template;
  $filename = rtrim($template, '.html').'.html';
  $template_file = clean_path(docroot()."/pages/$filename");    
  $forms .= smarty_parse_template($app, 'afr/pages/components/_template-edit-form.html', array('filename' => $template_file));
  
  //die("ok uid = $u_id l=$u_level auth=$auth_edit forms=$forms");
 
  return $forms;
}


// add page with given type to site
function site_add_page_type($siteObj_or_id, $pageTypeObj_or_id, $options=NULL) {
  $echo = $options['echo'];
  $update = isset($options['update']) ? $options['update'] : true;
  $pid = 0;
  
  $siteObj = any2obj('site', $siteObj_or_id);
  $pageTypeObj = any2obj('page_type', $pageTypeObj_or_id);
  
  $errors = $messages = array();
  
  $page_search = array();
  $page_search['site_id'] = $siteObj->id;
  if($pageTypeObj->name =='Home' ) {
    $page_search['page_type'] = PAGE_HOME;
  } else {
    $page_search['page_type'] = $pageTypeObj->id;
  }
  //echo($pageTypeObj->name);dump($page_search);
  if($pObj = find_object('site_page', $page_search)) {
    //dump($pObj);die();    
    $messages []= "Found page $pageTypeObj->name for site $siteObj->name";
    list($cids, $cerrors, $cmessages) = site_add_content_pages($siteObj, $pObj);
    if($cids) {
      $messages[] = "Added content IDs:".implode(',', $cids);
    } else {      
      $messages[] = "No content added for {$pObj->name} ({$pObj->id})";
    }
    if($cmessages) $messages = array_merge($messages, $cmessages);
    if($cerrors) $errors = array_merge($errors, $cerrors);
    
  } else {
    $pageData = (array) $pageTypeObj;
    unset($pageData['site_id']); 
    //$pageData['home_page']    = $pageTypeObj->id == PAGE_HOME ? 1 : 0;
    $pageData['site_id']      = $siteObj->id;
    //$pageData['level']        = 1; 
    $pageData['page_type']    = $pageTypeObj->id; 
    $pageData['user_id']      = 0;
    
    $messages []= "Adding page $pageTypeObj->name for site $siteObj->domain";
    // dump($page_data);    
    if($update) list($pid, $errors) = add_object('site_page', obj2array($pageData));
                                     
    if($errors) {
      dump($errors);
      dump($page_data);
    } else {
      $messages []= "Added page $pageTypeObj->name ($pid) for site $siteObj->domain ($siteObj->id)";
    }
  }
  if($echo) echo(print_errors($errors).print_messages($messages));
  
  return array($pid, $errors, $messages);
}


// adds page_content for a given site_page for each language
function site_add_content_pages($siteObj_or_id, $pageObj_or_id, $options=NULL) {
  
  $siteObj = any2obj('site', $siteObj_or_id);
  $pageObj = any2obj('site_page', $pageObj_or_id);

  $languages = json_decode($siteObj->languages);
  if(!$siteObj) die("site_add_content_pages: no site found for $siteObj_or_id");
  if(!$pageObj) die("site_add_content_pages: no page found for $pageObj_or_id");
  
  $echo   = $options['echo'];
  $update = isset($options['update']) ? $options['update'] : true;  

  //$add    = isset($options['add']) ? $options['add'] : true;
  
  $errors = array();
  $messages = array();

  $site_id = $siteObj->id;

  if($languages[0] != $siteObj->default_language) { // make sure default language is first
    array_unshift($languages, $siteObj->default_language);
    $languages = array_unique($languages);
  }
  //dump($languages);die();
    
  //dump($languages);
  foreach($languages as $lang) {
    $contentObj = site_find_content($siteObj, $pageObj, $lang); // if found, don't add again
    if($contentObj->page_id && !$options['overwrite']) {
      $messages[] = "Found content for page $pageObj->name in $lang: title={$contentObj->title} path={$contentObj->path}";
      //dump($contentObj);
      continue; // already exists, continue unless overwrite is set
    } else {
      $messages[] = "Did not find content for page $pageObj->name in $lang";
    }
    
    $alt_site_id = find_object('site_content', array('site_id' => $site_id, 'language' => $lang), 'alt_site_id');
    $contentData->path = $path = site_page_path($siteObj, $pageObj, $lang);
    if(!$contentObj->page_id) { // new content
      $contentData->title = $contentData->menu_title = $title = site_page_name($pageObj, $lang);
      $contentData->site_id = $alt_site_id ?: $site_id;
      $contentData->language = $lang;
      $contentData->page_id = $pageObj->id;
      $messages[] = "Adding content for page $pageObj->name in $lang: title=$title path=$path";
      if($update) list($cid, $cerrors) = add_object('page_content', $contentData);
      if($cerrors) $errors = array_merge($errors, $cerrors);
      else $ids[] = $cid;
      //dump($contentData);

    } else { // update existing content (path only)
      $content_id = $contentObj->id;
      $messages[] = "Updating content for page $pageObj->name in $lang path=$path";
      if($update) list($cid, $cerrors) = update_object('page_content', $contentData, $content_id);
      if($cerrors) $errors = array_merge($errors, $cerrors);
      else $ids[] = $cid;
    }
  }
  if($echo) echo(print_errors($errors).print_messages($messages));
  return array($ids, $errors, $messages);
}


// calculates the default path for a given page in a given language
function site_page_path($siteObj, $pageObj, $lang) {
  $page_type = $pageObj->page_type;
  $site_id = $pageObj->site_id  ?: $siteObj->id;
  if($page_type == PAGE_HOME) {
    $alt_site_id = find_object('site_content', array('site_id' => $siteObj->id, 'language' => $lang), 'alt_site_id');
    if($alt_site_id && $alt_site_id = $pageObj->site_id) return '/'; 
    return $lang == $siteObj->default_language ? '/' : "/$lang/"; // Home is special case
  }

  // Not home page
  if($page_type) $pageTypeObj = get_object('page_type', $page_type);    
  $dic_key = 'url_'.str_replace(' ', '_', strtolower($pageObj->name));
  $dic_search = array('term' => $dic_key, 'language' => $lang);
  if($dicObj = find_object('dictionary', $dic_search)) { // try specific URL entry
    //dump($dic_search);dump($dicObj);
    $page_path =  $dicObj->phrase;
  } elseif($dicObj = find_object('dictionary', array('term' => $pageObj->name, 'language' => $lang))) { // try generic entry
    $page_path =  $dicObj->phrase;
  } else {
    $page_path =  $pageObj->name;
  }
  $page_path = url_safe($page_path);
  
  if($parent_id = $pageObj->parent_id) { // prepend parent path
    $parentObj = get_object('site_page', $parent_id);
    $parentContentObj = find_object('page_content', array('page_id' => $parentObj->id, 'language' => $lang));
    $parent_path = $parentContentObj->path;
    //$parent_path_nolang = strpos($parent_path, "/$lang/") === 0 ? substr($parent_path, 3) : $parent_path; // '/nl/foo/' => '/foo/'
    $path = $parent_path."$page_path/";
    //echo("site_page_path: parent_path='$parent_path' parent_path_nolang='$parent_path_nolang' path='$path'<br>");
  } else {
    $parent_id = 0;
    $parent_path = '';
    $path = "/$page_path/";
    //echo("site_page_path: no parent path='$path'<br>");
  }
  if(!site_path_exists($site_id, $path)) return $path; // unique, we're done
  
  // exists, try to prepend language if not already present (but don't add /$lang/ if not in parent path)
  if(!$parent_id && $lang != $siteObj->default_language && strpos($path, "/$lang/") !== 0) $path = "/$lang".$path;
  if(!site_path_exists($site_id, $path)) return $path; // unique, we're done
  
  // if path exits, add -N until unique
  $i = 1;
  $path = rtrim($page_path,'/')."-$i/";
  while(site_path_exists($site_id, $path)) {
    $path = rtrim($page_path,'/')."-$i/";
    $i++;
  }
  
  return clean_path("/$path/");

  // old - delete
  
  // path already exists
  // cases:  
  //  1: There are more than 1 of this page_type Fix: postfix number to page_path
  $pageSearch['site_id'] = $site_id;
  $pageSearch['page_type'] = $page_type;
  $pageSearch['parent_id'] = $parent_id;
  if($pages = find_objects('site_page', $pageSearch)) {
    $i = $page_count = count($pages);
    //$page_path = "$page_path-$i";
    $new_page_path = rtrim($page_path,'/')."-$i/";
    $path = $parent_path_nolang.$new_page_path;
    while(site_path_exists($site_id, $path)) {
      $i++;
      $new_page_path = rtrim($page_path,'/')."-$i/";
      $path = $parent_path_nolang.$new_page_path;
    }
    return url_safe($path);
  }

  //  2: There is no unique dictionary entry for this type, Fix: prefix /$lang/
  
  return "/$lang".$path; // should we first check if this is really unique?
}

// checks if given path already exists for this site
function site_path_exists($site_id, $path) {

  //$contentObj = site_find_content($siteObj, $pageObj, $lang);
  $contentSearch['page_content.site_id'] = $site_id;
  $contentSearch['page_content.path'] = $path;
  //if($contentObj) $contentSearch['id'] = array('!=', $contentObj->id);
  
  $pageContent = find_object('page_content', $contentSearch); 
  if($pageContent) {
    //echo("Site: $site_id Found content with path='$path' ($pageContent->path id=$pageContent->id)<br>");
  } else {
    //echo("Site: $site_id Did not find content with path='$path'<br>");
  }
  return $pageContent ? true : false;
  
}


// generates default name for (new) page based on page_type and dictionary
function site_page_name($pageObj, $lang) {
  $dic_key = 'page_'.strtolower($pageObj->name);
  if($dicObj = find_object('dictionary', array('term' => $dic_key, 'language' => $lang))) {
    $title = ucfirst($dicObj->phrase);
  } else {
    $title = $pageObj->name;
  }
  return $title;
}
  
// automatically adds all pages for a site for which cms_add is set in page_type
// currently: home, about, contact
function site_add_default_pages($siteObj_or_id, $options=NULL) {

  $echo = $options['echo'];
  $update = isset($options['update']) ? $options['update'] : true;

  $page_types = array();
  $siteObj = any2obj('site', $siteObj_or_id);
  if($siteObj->parent_site_id) return; // don't add any pages, only parent site has pages
  if($echo) echo(html_h3($siteObj->name));
  $pages_to_add = $siteObj->add_pages ?: 0; 
  $pages_to_add |= CMS_ADD_ALWAYS;
  
  dump($siteObj);
  if($siteObj->property_site) {
    $pages_to_add |= CMS_ADD_PROPERTY; // this is a property site, add property related pages
    //$page_types = find_objects('page_type', array('property' => explode(',', $show_types))); // add shortstay, boat, etc
  }
  
  if($siteObj->allow_user_login) $pages_to_add |= CMS_ADD_USER_LOGIN;
  if($siteObj->allow_user_register) $pages_to_add |= CMS_ADD_USER_REGISTER;
  if($siteObj->gallery_site) $pages_to_add |= CMS_ADD_GALLERY;
  if($siteObj->payment_site) $pages_to_add |= CMS_ADD_PAYMENTS;

  //dump($siteObj);die();
  $pageTypeObjs = find_objects('page_type');
  foreach($pageTypeObjs as $pageTypeObj) {
    //echo("$pageTypeObj->name = $pageTypeObj->cms_add   $pageTypeObj->cms_add & $pages_to_add = ").($pageTypeObj->cms_add & $pages_to_add).html_break();
    if($pageTypeObj->cms_add & $pages_to_add) {
      //if($echo) echo("  $pageTypeObj->name: yes".html_break());
      $page_types[] = $pageTypeObj;
    }
  }
  
  $messages = array();
  $errors = array();
  $ids = array();
  //$update = false;$echo=true;  
  //dump($page_types);
  foreach($page_types as $page_type) {
    list($page_id, $page_errors, $page_messages) = site_add_page_type($siteObj, $page_type, $options);
    $ids[] = $page_id;
    $messages = array_merge($messages, $page_messags);
    $errors = array_merge($errors, $page_messags);
  }
  //if($echo) echo(print_errors($errors).print_messages($messages));
  return array($errors, $messages);
}

function site_update_content($siteObj) {
  return site_add_content($siteObj, false);
}


// adds site_content for a given site for each language
function site_add_content($siteObj, $add=true) {
  global $__LANGUAGES;
  $errors = array();
  $site_id = $siteObj->id;
  $languages = json_decode($siteObj->languages);
  if($languages[0] != $siteObj->default_language) { // make sure default language is first
    array_unshift($languages, $siteObj->default_language);
    $languages = array_unique($languages);
  }
  
  //dump($languages);
  foreach($languages as $lang) {
    $contentData->site_id = $site_id;
    $contentData->language = $lang;
    $contentObj = find_object('site_content', $contentData);  
    if($contentObj->page_id) {
      $content_id = $contentObj->id;
      echo("Found site_content for $siteObj->domain and language = $lang<br>");
      // list($cid, $cerrors) = update_object('site_content', $contentData, $content_id);
      // if($cerrors) $errors = array_merge($errors, $cerrors);
    } else {
      // $errors[] = "Could not find $lang content for page $pageObj->id ($pageObj->name)";
      //$contentData->title = $contentData->se_title = $contentData->menu_title = $title = site_page_name($pageObj, $lang);
      echo("Adding site content for $siteObj->name in $lang<br>");
      $contentData->title = $siteObj->name;
      list($cid, $cerrors) = add_object('site_content', $contentData);
      if($cerrors) $errors = array_merge($errors, $cerrors);
      else $ids[] = $cid;
    }

  }
  if($errors) dump($errors);
  return array($ids, $errors);
}

function site_update_content_pages($siteObj, $pageObj) {
  return site_add_content_pages($siteObj, $pageObj);
}



// sitemap

// returns array of all pages in same language as $pageObj for given site
// used to display HTML sitemap and to generate XML sitemap
function site_sitemap($siteObj_or_id, $options) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $tree = site_sitemap_tree($siteObj, $options);
  return $tree;    
}

// recursively returns tree of pages in given language for given site 
function site_sitemap_tree($siteObj_or_id, $options=NULL) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $lang = $options['language'] ?: $_SESSION['language'];
  $parent_id = isset($options['parent_id']) ? $options['parent_id'] : -1;
  if(!$lang) $lang = $siteObj->default_language; // todo: all languages    
  if(!$lang) die("site_sitemap_tree: language was not specified and $siteObj->domain has no default language");
  $tree = array();
  $parent_id = $options['parent_id'] ?: 0;
  $search = array('site_id' => $siteObj->parent_site_id ?: $siteObj->id, 'active' => 1, 'parent_id' => $parent_id, 'user_level' => 0, 'path_var' => '');
  $pageObjs = find_objects('site_page', $search, ['sort' => 'rank']);
  $count = count($pageObjs);
  foreach($pageObjs as $pageObj) {
    $node = site_sitemap_node($siteObj, $pageObj, $lang);
    if(!$node) continue;
    
    $options['parent_id'] = $pageObj->id;
    $children = site_sitemap_tree($siteObj, $options);
    $node->child_count = count($node->children);
    $node->children = $children;
    $tree[$node->path] = $node;
  }
  return $tree;
}



function site_write_sitemap($siteObj_or_id, $lang='') {
  $siteObj = any2obj('site', $siteObj_or_id);
  $doc_dir = site_directory($siteObj);
  if(!$doc_dir) return;
                      
  
  $xml = site_sitemap_xml($siteObj_or_id);
  
  $base_dir = $_SERVER['DOCUMENT_ROOT']."sites/$siteObj->domain/";
  $xmlfile = $base_dir.'sitemap.xml';
  write_file($xmlfile, $xml);
  $size = strlen($xml);
  echo("Wrote $size bytes to $xmlfile<br>");
  return $size;
}


function site_sitemap_xml($siteObj_or_id, $options = []) {  
  $siteObj = any2obj('site', $siteObj_or_id);
  $tree = isset($options['tree']) ? $options['tree'] : site_sitemap_tree($siteObj, $options);
  $urlset = site_sitemap_xml_nodes($siteObj, $tree);
  
  $xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>
  <urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\" 
   xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"
   xsi:schemaLocation=\"http://www.sitemaps.org/schemas/sitemap/0.9 http://www.sitemaps.org/schemas/sitemap/0.9/sitemap.xsd\">;
{$urlset}
</urlset>";
      
  return $xml;
}

// print sitemap tree as XML
function site_sitemap_xml_nodes($siteObj_or_id, $nodes) {
  $site_url = rtrim(site_full_url($siteObj_or_id), '/');
  
  foreach($nodes as $path => $node) {
    if(strpos($path, 'http') !== 0) {
      $path = "{$site_url}{$path}";
      //die("no http, now $path");
    }
    $xml .= "
  <url>
    <loc>".$path."</loc>
  </url>";
    if($node->children) $xml .= site_sitemap_xml_nodes($siteObj_or_id, $node->children);
  }
  return $xml;
//    $priority = $node->priority ?: '0.5';
//    $change_freq = $node->change_freq ?: 'daily';
//    <priority>".$priority."</priority>
//    <changefreq>".$change_freq."</changefreq>

}


    
// recursively returns tree of pages in given language for given site
// called from site_menu_tree
function site_page_tree($siteObj_or_id, $options=NULL) {
  $search = $options['search'] ?: array('site_id' => $siteObj->parent_site_id ?: $siteObj->id, 'active' => 1, 'show_menu' => 1);
  $siteObj = any2obj('site', $siteObj_or_id);
  $lang = $options['language'];
  $parent_id = isset($options['parent_id']) ? $options['parent_id'] : -1;
  if(!$lang) $lang = $siteObj->default_language; // todo: all languages    
  if(!$lang) die("site_page_tree: language was not specified and $siteObj->domain has no default language");
  $tree = array();
  $root = 0;
  if($parent_id == -1) { // root
    $root = 1;
    $homePage = site_home_page($siteObj, $lang);
    if(!$homePage) echo("Warning: could not find home page for $siteObj->domain");
    $options['parent_id'] = 0;
    $root_node = site_sitemap_node($siteObj, $homePage, $lang);
    $children = site_page_tree($siteObj, $options);
    $root_node->child_count = count($children);
    $root_node->children = $children; // recurse the rest
    $tree[$root_node->path] = $root_node;
  }
  $parent_id = $options['parent_id'];
  
  //$search = array('site_id' => $siteObj->parent_site_id ?: $siteObj->id, 'active' => 1, 'parent_id' => $parent_id, 'admin' => 0);
  
  $search['parent_id'] = $parent_id;
  if(!$parent_id) $search['page_type'] = array('!=', PAGE_HOME); // this is only needed for top level 
  $pageObjs = find_objects('site_page', $search, array('sort' => 'rank'));
  $count = count($pageObjs);
  foreach($pageObjs as $pageObj) {
    $node = site_sitemap_node($siteObj, $pageObj, $lang);
    if(!$node) continue;
    
    $options['parent_id'] = $pageObj->id;
    $children = site_page_tree($siteObj, $options);
    $node->child_count = count($children);
    $node->children = $children;
    //dump($node);
    $tree[$node->path] = $node;
  }
  
  //dump($tree);die();
  return $tree;
}
  
// a node is just the fields of pageContent we care about (+ its children, set in site_sitemap_tree)
function site_sitemap_node($siteObj, $pageObj, $lang='') {
  $node = new stdClass;
  $node->page_id = $pageObj->id;
  if($pageObj->path) {
    $pageContent = $pageObj;
    $lang = $pageContent->language;
  } else if($lang) {
    $pageContent = site_find_content($siteObj, $pageObj, $lang);
  } else {
    die("site_sitemap_node: neither page content nor language passed");
  }
  
  if(!$pageContent) return NULL;
  $node->content_id = $pageContent->content_id;
  $copy_flds = array('title', 'menu_title', 'user_level', 'page_type', 'is_folder'); 
  foreach($copy_flds as $fld) $node->$fld = $pageContent->$fld;

  if($node->is_folder) { 
    $node->name = $pageObj->name;
    $node->path = $pageContent->path;
  } else {  
    //dump($pageContent); 
    $node->se_title = site_header_var($siteObj, $pageObj, 'title');
    $node->path = $pageContent->path;
    $node->url = site_page_link($siteObj, $pageContent, array('full' => true, 'lang' => $lang));
    $node->name = pick_first(array($pageContent->menu_title, $pageContent->title, $pageContent->se_title, $pageObj->name));
    $node->lang = $lang;
    $node->show_menu = $pageObj->show_menu;  
    $node->user_level = $pageObj->user_level;
    
    if($page_type = $node->page_type) {
      $pageTypeObj = get_object('page_type', $page_type);
    }
  } 
  //dump($node);                    
  //echo("$node->name<br>");
  return $node;
}

function site_header_var($siteObj, $pageObj, $var) {
  $fld = 'se_'.$var;
  $val = $pageObj->$fld ?: $siteObj->$fld;
  return parse_header_vars($siteObj, $pageObj, $val);
}

function parse_header_vars($siteObj, $pageObj, $var) {
 global $city_name;   
 global $site_name;   
 global $apt_name; 
 global $geo_name;
 $country_name  = $_SESSION['geo_country_name'];
 $district_name = $_SESSION['geo_district_name'];
 $path_var = ucfirst(str_replace('-', ' ', $_SESSION['path_var']));
 return ucfirst(trim(str_replace(array('__pagename', '__sitename', '__city', '__district', '__country', '__aptname', '__pathvar'), array($pageObj->title ?: $pageObj->menu_title, $site_name, $city_name, $district_name, $country_name, $apt_name, $path_var), $var))); 
}  


/** new 2016, used by jstree in edit-site */
function site_tree($siteObj_or_id, $options=NULL, $parent_id=0) {
  global $u_id, $u_level, $__LANG;
  
  $lang = isset($options['language']) ? $options['language'] : $__LANG; 
  $site_id = any2id('site', $siteObj_or_id);
  $search = ['site_id' => $site_id, 'parent_id' => $parent_id];
  if(isset($options['active']) && $options['active']) $search['active'] = 1; 
  if(isset($options['show_menu']) && $options['show_menu']) $search['show_menu'] = 1;
  
  $searchOptions['fields'] = ['id', 'parent_id', 'name as text', 'active', 'show_menu'];
  $tree = find_objects('site_page', $search, $searchOptions) ?: [];
  foreach($tree as $node) {
    $children = site_tree($site_id, $options, $node->id);
    $node->obj_type = $obj_type = 'site_page';
    $node->obj_id = $obj_id = $node->id;
    //$node->id = "{$obj_type}-{$obj_id}";
    $node->id = $obj_id;
    $node->child_count = $count = count($children);
    $node->icon = $count ? "" : "fa fa-file-o";
    $node->language = $lang;
    
    $auth_edit = 1;
    $auth_delete = $u_level == USER_LEVEL_SYSTEM || $node->user_id == $u_id ? 1 : 0;
    $node->auth = ["view" => 1, "edit" => $auth_edit, "delete" => $auth_delete];
    $classes = [];
    $classes[] = $node->active ? "bold" : "italic disabled";
    $classes[] = $node->show_menu ? "text-success" : "text-default";
    $node->li_attr = ["class" => implode(' ', $classes)];
    $node->children = $children;
  }
  return $tree;
  

  /** dummy root node 
  if($parent_id) return $tree; // not root
  $root = new stdClass;
  $root->text = get_object('site', $site_id, 'name');
  $root->child_count = count($tree);
  $root->icon = "fa fa-sitemap";
  $root->root = 1;
  $root->obj_type = $obj_type = 'site';
  $root->obj_id = $obj_id = $site_id;
  // $node->id = "{$obj_type}-{$obj_id}";
  $root->id = -1; 
  $root->active = 1;
  $root->children = $tree;
  */
  
  return $root;
}


// returns cache directory for site, page if given
function site_cache_dir($siteObj_or_id, $pageObj=NULL) {
  return clean_path($_SERVER['DOCUMENT_ROOT'].site_cache_path($siteObj_or_id));
}

// returns relative path to cache directory for site, page if given
function site_cache_path($siteObj_or_id, $pageObj=NULL) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $result = "/sites/$siteObj->domain/cache/";
  return $pageObj ? clean_path($result.$pageObj->path) : $result; 
}

// helper function for spidering, used by site_spider_page and site_sitemap_html
function site_spider_node($siteObj_or_id, $node, $options=NULL) {
  //dump($tree);die();
  $siteObj = any2obj('site', $siteObj_or_id);
  $base_dir = site_cache_dir($siteObj);
  $path = $node->path;  
  $target_dir = clean_path($base_dir.$path);

  $start_time = getTime(); 
  $url = replace_query_var($node->url, '__cam', 1);
  print_log("Spidering node $name with url=$url");
  if($options['overwrite'] )$url = replace_query_var($url, '__overwrite', 1);
  list($html, $status) = get_html_and_status($url);      
  $end_time = getTime();
  $time = number_format(($end_time - $start_time),2);
  $len = strlen($html);
  print_log("Spidered node $name with status=$status len=$len");
  $message = $status == 200 ? 'OK' : "Error";
  $item = array('path' => $path, 'length' => bytes2human($len), 'time' => $time.'s', 'status' => $status, 'message' => $message);
  if($status == 200) {
    if(is_dir($target_dir) || mkdir($target_dir, 0775, true)) {
      $target_file = $target_dir.'index.html';
      if($dryrun) {
        echo("site_spider_node: (dryrun) Wrote $target_file ($len) to $target_dir<br>");
      } elseif(!write_file($target_file, $html)) {
        die("site_spider_node: Couldn't write $target_file to $target_dir");
      } else {
        //echo("Wrote $target_file ($len) to $target_dir<br>");
        //die();
      }
    } else {
      die("site_spider_node: Couldn't make $target_dir");
    }
  }   

  if($pb_id = $options['progress_id']) progressbar_increase_progress($pb_id);

  return $item;
}

function site_template_prepare($site_id, $source, $dest, $prepared = false) {
  
  if(!is_file($source)) return array(0, "$source does not exist");
  //if(!copy($source, $dest)) return array(0, "Could not copy template $source to $dest");
    
  $content = read_file($source);
  if(!$content) return array(0, "$source is an empty file or could not be read");

  $prepare = strpos($content, "{* Source:") === false;
  if(!$prepared && $prepare) {  /** don't replace if processed before */
    /** multiple passes needed as it will otherwise replace itself: { => {ldelim{rdelim} */
    $content = str_replace('}', '__§__', $content); // replace right curly bracket with tmp
    $content = str_replace('{', '{ldelim}', $content); // replace left curly bracket
    $content = str_replace('__§__', '{rdelim}', $content); // replace right curly bracket using tmp
    $content = str_replace('"assets/', '"/assets/', $content); // replace asset path
    $content = template_replace_images($site_id, $content);
  }
  
  /** Add comment */
  $now = now();
  $content = "{* Source: $source Target: $dest Created: $now *}\n".$content;

  
  //echo("content = $content");
  if(!write_file($dest, $content)) return array(0, "Could not write template to $dest");
  return array(1, "");
  
}

function template_replace_images($site_id, $content) {
  /** Replace image tags with content tags */
  $pattern = '|\<img\s(.*)\>|i'; // case insensitive

  $output = preg_replace_callback (
    $pattern, 
    function($matches) {
      //dump($matches);
      $match = $matches[1];
      if(!$match) return '';

      $match = trim(trim($match, "/"));
      $attrs = explode(' ', $match);
      
      $tagObj = new stdClass;
      foreach($attrs as $attr) {
        $attr = trim($attr);
        //$value = strpos($value, '"') === 0 ? trim($value, '"') : trim($value, "'"); /** strip single or double quotes */          
        list($key,$value) = explode('=', $attr);
        $key = trim($key);
        $value = trim($value);
        $value = trim($value, "\"'");
        // $value = str_replace('"', '\"', $value); // escape quotes (json_encode does this)
        //echo("Key=$key value=$value<br>");
        if($key == 'src') { /** check that image exists and get dimensions */
          $src = $value;
        } else {
          $tagObj->$key = $value;
        }
      }
      if($src) {
        $tagObj->name = "image-".url_safe($src)."-".rand(1,1000);
        $tagObj->type = "image";
        
        $h = $tagObj->height ?: 0;
        $w = $tagObj->width ?: 0;
        $max_explicit = ($h || $w) ? max($h, $w) : 0; 

        $filename = is_url($src) ? $src : siteUrl(false).$src;
        
        $tagObj->value = $filename;
        
        $imageInfo = getimagesize($filename);
        if(!$imageInfo) return $match." {* $filename is not an image *}\n";
        $width = $imageInfo[0];
        $height = $imageInfo[0];
        $max = max($width, $height);
        
        $max = $max_explicit ?: $max;
        if($max) $tagObj->resize = $max;
        
        $tag = "{".json_encode($tagObj)."}";

        //echo("Tag = $tag<br>");
      } else {
        //echo("No src, no tag".dump($tagObj, true));
        return $match." {* Failed to parse image tag *}\n";
      }
      //echo(html_break());
      return $tag;
    }, 
    $content);
  return $output;
}

 
// apply controller if any and return scope in pageObj
function apply_controller($app, $options = array()) {
  $global_scope = $app->scope = $app->scope ?: new StdClass; /** global scope */
  if(!isset($app->page->scope)) $app->page->scope = new stdClass; /** local scope - set by controller */
  
  $siteObj = $app->site;  
  $pageObj = $app->page;
  
  $tpl = $app->page->template;
  $ctrl = $app->page->controller ?: 'generic';
  $name = $app->page->name;

  // global controller
  $controller_func = "controller_{$ctrl}";
  print_log("Looking for global controller {$controller_func}", 'app', LOG_LEVEL_TEST);
  if(function_exists($controller_func)) {
    print_log("Found global controller {$controller_func}", 'app', LOG_LEVEL_TEST);
    call_user_func($controller_func, $app, $options);  
    $global_scope->global_controller = str_replace("controller_", '', $controller_func);
  }
  
  // site controller
  if($lib_prefix = $siteObj->library_prefix ?: $siteObj->shortname) {
    $controller_func = "{$lib_prefix}_controller_{$ctrl}";
    print_log("Looking for local controller {$controller_func}", 'app', LOG_LEVEL_TEST);
    //die("Looking for local controller {$controller_func}");
    if(function_exists($controller_func)) {
      //mydie("'{$controller_func}' does  exist");
      print_log("Found local controller {$controller_func}", 'app', LOG_LEVEL_TEST);
      call_user_func($controller_func, $app, $options);
      $global_scope->site_controller = str_replace($lib_prefix."_controller_", '', $controller_func);    
    } else {
      //mydie("'{$controller_func}' does not exist");
    }
  }
    
  $scope = $app->page->scope; /** local scope - set by controller */

  if(!isset($global_scope->controller)) $global_scope->controller = $global_scope->site_controller ?: $global_scope->global_controller;
  if(!isset($global_scope->template)) $global_scope->template = $app->page->template;
  
  return;
}
  

// apply controller, parse template, return output
// template is set in pageObj->template
// controller is set in pageObj->controller
function fetch_template($app, $options = array()) {
  $siteObj = $app->site;
  $pageObj = $app->page;

  if(!$pageObj->controller && $options['controller']) $pageObj->controller = $options['controller'];
  if(!$pageObj->template && $options['template']) $pageObj->template = $options['template'];
  
  $scope = isset($options['scope']) ? $options['scope'] : $pageObj->scope;
  $pageObj->scope = $scope;
  $template = $pageObj->template;

  if(!$template) return "fetch_template: Missing template"; 
  if(!$pageObj->controller) $pageObj->controller = 'generic';

  $controller = $pageObj->controller;
  $partial_view = isset($options['partial_view']) ? $options['partial_view'] : 0; 
  print_log("fetch_template: tpl={$template} ctrl={$controller} partial:{$partial_view} scope:", 'app', LOG_LEVEL_TEST);
  apply_controller($app, []);
  
  $pageObj->page_elements = $pageElementArray = smarty_page_elements($app);    

  $filename = smarty_preparse_template($app, $template);  
  $output = smarty_parse_template($app, $filename);
  
  return $output;
}


/** Look for page elements for this page / path */
function smarty_page_elements($app) {  
  $siteObj = $app->site;
  $pageObj = $app->page;
  $pageElementArray = [];
  if($ms = $pageObj->minisite) {
    $path = "/{$ms}".$pageObj->path;
    $search = ['site_id' => $siteObj->id, 'path' => $path];
  } else {
    $search = ['site_id' => $siteObj->id, 'page_id' => $pageObj->id, 'language' =>  [$pageObj->language, '*']];
  }  
  $pageElementObjs = find_objects('page_element', $search) ?: [];
  //die("smarty_page_elements ".count($pageElementObjs)." page elements search:".dump($search,true));
  
  foreach($pageElementObjs as $pageElementObj) $pageElementArray[$pageElementObj->name] = $pageElementObj;
  return $pageElementArray;
}

/** persist page element to disk */
function smarty_save_page_element($pageElement) {
  global $__SITE_ID, $__SITE;
  $site_id = $pageElement->site_id;
  $siteObj = $site_id == $__SITE_ID ? $__SITE : get_object('site', $site_id);

  $domain = $siteObj->domain;
  
  $approot = approot(false);
  $base_path = "{$approot}/content/{$domain}";
  
  $path = $pageElement->path;
  if(!$path) die("smarty_save_page_element: Missing path");
  if(!$site_id) die("smarty_save_page_element: Missing site id");
  
  if(in_array($pageElement->type, ['text', 'html'])) {
    if($pageElement->options) {
      die("options:".dump($pageElement,true));
    }
    $ext = $pageElement->type == 'text' ? 'txt' : strtolower($pageElement->type); 
    $dir = $base_path.rtrim($path, '/');
    if(!is_dir($dir) && !mkdir($dir, 0775, true)) die("smarty_save_page_element: Failed to create $dir");
    $file = $dir."/_".$pageElement->name.".".$ext;
    if(!$pageElement->content) {
      return false;
      die("no content".dump($pageElement,true));              
    }    
    //if(is_file($file)) continue;
    //die("element path = {$element_path} file={$file}");
    $len = write_file($file, $pageElement->content.PHP_EOL);
    if(!$len) die("smarty_save_page_element: Failed to write to {$file}");
    
    return true;
  }
  return false;
}
  
// options is used to pass extra data to scope
function smarty_parse_template($app, $filename, $options = array()) {
  global $__LANGUAGE, $__LANGUAGES, $__USER, $__USER_LEVEL, $__USER_ID, $__PATH_VAR,$__AUTH_EDIT,$__EDIT_MODE,$__EDIT_MENU, $__CLIENT_ID,$__GLOBAL_SCOPE;
  global $u_id, $u_level;
  $siteObj = $app->site;
  $pageObj = $app->page;
  
  if (!class_exists('Smarty')) {
    require_once('Smarty/Smarty.class.php');    
  }
  $root = $options['root']; // top level page template? - not in use
  $main = $options['main']; // main template (content)
  
  $approot = approot(false);
  $userObj = $app->user;

  $query_string = get_query_string();
  $this_path = this_path();
  
  if($query_string) $query_string = "?$query_string"; // so we can append it to URLs in template

  $auth_edit = $_SESSION['auth_edit'];
  if($scope = $options['scope'] ?: $pageObj->scope) {
    unset($options['scope']);
  } else {
    $scope = new stdClass;
  }
  
  if(is_array($options)) foreach($options as $k => $v) $scope->$k = $v;
  //dump($scope);die();
  $scopeAr = is_array($scope) ? $scope : obj2array($scope, true); // recursive
  $globalAr =  $app->scope ? obj2array($app->scope, true) : array();

  //print_log("\n\n\nsmarty_parse_template ($filename) scope=".dump($scope,true), 'app', LOG_LEVEL_TEST);
  //$globalAr = obj2array($global);

  $template = $_SESSION['page_template'];
  $template_dir = $approot.'pages';
  
  $smarty = new Smarty();

  if(isset($_GET['cache']) && !$_GET['cache']) $smarty->clearAllCache();

  $globals = array('u_id' => $u_id, 'u_level' => $u_level);
  
  $smarty->setTemplateDir($approot.'/smarty/templates');
  $smarty->setCompileDir($approot.'/smarty/templates_c');
  $smarty->setCacheDir($approot.'/smarty/cache');
  $smarty->setConfigDir($approot.'/smarty/configs');

  // custom modifiers                       
  $smarty->registerPlugin('modifier', 'currency', 'currency2html');
  $smarty->registerPlugin('modifier', 'media', 'print_media');
  $smarty->registerPlugin('modifier', 'photo', 'get_media_source');
  $smarty->registerPlugin('modifier', 'photos', 'get_media_sources');
  $smarty->registerPlugin('modifier', 'default', 'return_if_empty');
  $smarty->registerPlugin('modifier', 'date', 'sql2human');
  $smarty->registerPlugin('modifier', 'datetime', 'sql2human_datetime');
  $smarty->registerPlugin('modifier', 'daterange', 'print_time_range');
  $smarty->registerPlugin('modifier', 'add_day', 'add_day');
  $smarty->registerPlugin('modifier', 'strip_time', 'strip_time');
  $smarty->registerPlugin('modifier', 'price', 'formatFloat');
  $smarty->registerPlugin('modifier', 'apply_discount', 'apply_discount');

  $smarty->registerPlugin('modifier', 'name', 'get_name_string');
  $smarty->registerPlugin('modifier', 'address', 'get_address_string');
  $smarty->registerPlugin('modifier', 'display_name', 'user_display_name');
  $smarty->registerPlugin('modifier', 'displayname', 'user_display_name');
  $smarty->registerPlugin('modifier', 'username', 'user_display_name');
  $smarty->registerPlugin('modifier', 'userphoto', 'user_photo');
  $smarty->registerPlugin('modifier', 'media_list', 'print_media_list');
  $smarty->registerPlugin('modifier', 'fullname', 'get_user_name');
  $smarty->registerPlugin('modifier', 'full_name', 'get_user_name');
  $smarty->registerPlugin('modifier', 'get_user', 'get_user');
  $smarty->registerPlugin('modifier', 'markup', 'txt2html');
  $smarty->registerPlugin('modifier', 'has_role', 'user_has_role');

  $smarty->registerPlugin('modifier', 'object_key', 'get_object_key');
  $smarty->registerPlugin('modifier', 'object_table', 'get_object_table');

  $smarty->registerPlugin('modifier', 'get', 'smarty_get_object');
  $smarty->registerPlugin('modifier', 'find', 'smarty_find_object');
  $smarty->registerPlugin('modifier', 'find_all', 'smarty_find_objects');
  $smarty->registerPlugin('modifier', 'count_objects', 'count_objects');
  
  $smarty->registerPlugin('modifier', 'period_diff', 'period_diff');
  $smarty->registerPlugin('modifier', 'elapsed', 'time_elapsed_human');
  $smarty->registerPlugin('modifier', 'print_array', 'print_array_assoc');
  $smarty->registerPlugin('modifier', 'flag', 'html_flag');
  $smarty->registerPlugin('modifier', 'tooltip', 'html_tooltip');
  $smarty->registerPlugin('modifier', 'checkmark', 'html_checkmark');
  $smarty->registerPlugin('modifier', 'pagination', 'html_pagination');
  $smarty->registerPlugin('modifier', 'object_select', 'object_select');
  $smarty->registerPlugin('modifier', 'myhash', 'myhash');
  $smarty->registerPlugin('modifier', 'alert', 'print_alert');
  $smarty->registerPlugin('modifier', 'translate', 'smarty_translate');
  $smarty->registerPlugin('modifier', 'transplace', 'smarty_transplace');
  $smarty->registerPlugin('modifier', 'permissions', 'permissions_print');
  $smarty->registerPlugin('modifier', 'permissions_edit', 'permissions_edit');
  
  $smarty->registerPlugin('modifier', 'slugify', 'slugify');
  
  if(1 || $siteObj->property_site) {
    $smarty->registerPlugin('modifier', 'property_geolink', 'smarty_property_geolink');
    $smarty->registerPlugin('modifier', 'property_link', 'smarty_property_link');
    $smarty->registerPlugin('modifier', 'public_link', 'public_link');
    $smarty->registerPlugin('modifier', 'city', 'cityname');
    $smarty->registerPlugin('modifier', 'guest_name', 'reservation_guest_name');  
  }

  if(1 || $siteObj->payment_site) {
    $smarty->registerPlugin('modifier', 'lineitems', 'get_lineitems');
    $smarty->registerPlugin('modifier', 'invoice_rows', 'invoice_rows');
  }
  
  if(1 || $siteObj->gallery_site) { /** Gallery / Art */  
    $smarty->registerPlugin('modifier', 'collection_photo', 'collection_title_image');
    $smarty->registerPlugin('modifier', 'folder_photo', 'folder_title_image');
    $smarty->registerPlugin('modifier', 'artists', 'collection_artist_list');
    $smarty->registerPlugin('modifier', 'gallery_link', 'gallery_link');
  }
  
  $smarty->registerPlugin('modifier', 'plural', 'plural');
  $smarty->registerPlugin('modifier', 'trans', 'phrase');

  
  /** Array Utilities */
  $smarty->registerPlugin('modifier', 'array_remove', 'array_remove');
  $smarty->registerPlugin('modifier', 'array_remove_key', 'array_remove_key');
  $smarty->registerPlugin('modifier', 'obj2array', 'obj2array');
  $smarty->registerPlugin('modifier', 'any2array', 'any2array');
  $smarty->registerPlugin('modifier', 'array2obj', 'array2obj');
  $smarty->registerPlugin('modifier', 'array2list', 'array2list');

  /** HTML, TEXT */  
  $smarty->registerPlugin('modifier', 'table', 'html_table');
  $smarty->registerPlugin('modifier', 'anchor', 'html_link');
  $smarty->registerPlugin('modifier', 'nl2br', 'nl2br');
  $smarty->registerPlugin('modifier', 'hide_false', 'hide_false');
  $smarty->registerPlugin('modifier', 'yes_no', 'html_yes_no');
  $smarty->registerPlugin('modifier', 'truncate_html', 'truncate_html');
  $smarty->registerPlugin('modifier', 'strip_style', 'strip_style');
  $smarty->registerPlugin('modifier', 'strip_attribute', 'strip_attribute');

  /** header tags */
  $smarty->registerPlugin('modifier', 'script_tag', 'html_script_tag');
  $smarty->registerPlugin('modifier', 'style_tag', 'html_style_tag');
  $smarty->registerPlugin('modifier', 'meta_tag', 'html_meta_tag');
  $smarty->registerPlugin('modifier', 'header_link', 'html_header_link');
  
  /** query, path */  
  
  $smarty->registerPlugin('modifier', 'rroute', 'smarty_reverse_route');
  $smarty->registerPlugin('modifier', 'query', 'build_query_string');
  $smarty->registerPlugin('modifier', 'strip_query', 'strip_query_string');
  $smarty->registerPlugin('modifier', 'link', 'page_link');
  $smarty->registerPlugin('modifier', 'page', 'smarty_page_content');
  $smarty->registerPlugin('modifier', 'get_page', 'smarty_get_page');
  $smarty->registerPlugin('modifier', 'site_url', 'site_url');
  $smarty->registerPlugin('modifier', 'print_link', 'print_page_link');
  $smarty->registerPlugin('modifier', 'error', 'print_errors');
  $smarty->registerPlugin('modifier', 'message', 'print_messages');
  $smarty->registerPlugin('modifier', 'find_page', 'print_page_link');
  $smarty->registerPlugin('modifier', 'parent_path', 'parent_path');
  $smarty->registerPlugin('modifier', 'slug', 'slug');

  $smarty->registerPlugin('modifier', 'add_query_var', 'add_query_var');
  $smarty->registerPlugin('modifier', 'remove_query_var', 'remove_query_var');
  $smarty->registerPlugin('modifier', 'replace_query_var', 'replace_query_var');

  $smarty->registerPlugin('modifier', 'address_data', 'get_address_data');
  
  /** CMS / Page edit */
  $smarty->registerPlugin('modifier', 'uploader', 'smarty_uploader');
  $smarty->registerPlugin('modifier', 'media_uploader', 'smarty_media_uploader');
  $smarty->registerPlugin('modifier', 'object_uploader', 'object_media_uploader');
  $smarty->registerPlugin('modifier', 'editor', 'smarty_editor');
  $smarty->registerPlugin('modifier', 'editable', 'smarty_editable');
  $smarty->registerPlugin('function', 'editable', 'smarty_edit');
  
  /** File */
  $smarty->registerPlugin('modifier', 'file_icon', 'file_icon');
  $smarty->registerPlugin('modifier', 'file_size', 'filesize');
  $smarty->registerPlugin('modifier', 'file_extension', 'file_extension');
  $smarty->registerPlugin('modifier', 'file_name', 'file_name');
  $smarty->registerPlugin('modifier', 'file_path', 'file_path');
  $smarty->registerPlugin('modifier', 'file_loc', 'file_loc');
  $smarty->registerPlugin('modifier', 'bytes2human', 'bytes2human');
  
  
  /** Rating */
  $smarty->registerPlugin('modifier', 'stars', 'star_rating');
  $smarty->registerPlugin('modifier', 'rating', 'star_rating');

  // smarty functions
  
  $smarty->assign('template_dir', $template_dir);
  $smarty->assign('config', $_SESSION['config'] ?: []);
  
  $smarty->assign('get', $_GET);  

  parse_str(ltrim($query_string,'?'), $qvars);
  
  $smarty->assign('query_string', $query_string);
  $smarty->assign('qs', ltrim($query_string, '?')); // $_SERVER['QUERY_STRING'] doesn't work as expected due to mod rewrite
  $smarty->assign('query_vars', $qvars); // $_SERVER['QUERY_STRING'] doesn't work as expected due to mod rewrite
  
  //  $_GET includes vars from mod_rewrite
  
  foreach($globals as $key => $val) $smarty->assign($key, $value);
  $smarty->assign('today', today());
  $smarty->assign('now', now());
  $smarty->assign('time', time());
  $smarty->assign('live', $_SESSION['live']);
  $smarty->assign('deployment', $_SESSION['deployment']);
  if($userObj->timezone) $smarty->assign('usertime', date());
  
  //$smarty->assign('sites', $_SESSION['client_sites']);
  //$smarty->assign('legal_entities', $_SESSION['client_legal_entities']);
  
  $site_url = rtrim(site_full_url($siteObj), '/');
  $this_url = remove_query_var(this_url(), 'act');
  $this_full_url = strpos($this_url, 'http') === 0 ? $this_url : $site_url.$this_url;
  $smarty->assign('client_ip', client_ip());
  $smarty->assign('auth_edit_site', $__AUTH_EDIT);
  $smarty->assign('edit_mode', $__EDIT_MODE);
  $smarty->assign('edit_menu', $__EDIT_MENU);
  $smarty->assign('client_id', $__CLIENT_ID);
  $smarty->assign('site', (array) $siteObj);
  $smarty->assign('user', (array) $userObj);
  $smarty->assign('page', (array) $pageObj);
  $smarty->assign('app', obj2array($app,true));
  
  $smarty->assign('page_template', $pageObj->template);
  $smarty->assign('page_controller', $pageObj->controller);
  $smarty->assign('lang', $__LANGUAGE);
  $smarty->assign('language', $__LANGUAGE);
  $smarty->assign('languages', $__LANGUAGES);
  $smarty->assign('root_user', $_SESSION['root_user']);
  $smarty->assign('scope', $scopeAr);
  $smarty->assign('global', $globalAr);
  $smarty->assign('template', $template);   
  
  //$smarty->assign('doc_root', docroot(false)); // no trailing slash
  //$smarty->assign('app_root', approot(false)); // no trailing slash
  
  $smarty->assign('app_root', __DIR__); // no trailing slash
  $smarty->assign('doc_root', __DIR__."/www"); // no trailing slash
  $smarty->assign('lib_root', __DIR__."/afr"); // no trailing slash
  
  $smarty->assign('this_path', this_path());
  $smarty->assign('path_var', $__PATH_VAR);
  $smarty->assign('full_path', full_path());
  $smarty->assign('site_url', $site_url); // no trailing slash
  $smarty->assign('this_url', $this_url);
  $smarty->assign('this_full_url', $this_full_url);
  $smarty->assign('aff_id', $_SESSION['_aid']);
  $smarty->assign('server', $_SERVER);

  $smarty->assign('dump', smarty_dump);

  try {                
    $template = $smarty->fetch($filename);
  }
    catch (Exception $e) {
    $smarty_error = $e->getMessage();
    $scope->error = htmlentities($smarty_error);
    $template = "Cannot parse $filename:<br>{$smarty_error}<br><br>";
  }
  
  $output =  '';
  
  if($_SESSION['__edit_mode']) {
    $full_filename = __DIR__.'/../pages/'.$filename;
    if(is_file($full_filename) && is_writeable($full_filename)) {
      $output .= '<a class="btn btn-success __edit_template __edit_link" data-template="'.$full_filename.'" href="#"><i class="fa fa-code"></i> Edit Template '.$filename.'</a>';
    } else {
      //$output .= '<a class="btn btn-success __edit_template __edit_link" data-template="'.$full_filename.'" href="#"><i class="fa fa-code"></i> NOT Edit Template '.$filename." full=$full_filename exists=".is_file($full_filename)." writable=".is_writeable($full_filename).'</a>';
    }
  }

  //$output .= print_alert($filename, "message");
  
  if(0 && $main) { // only do this once
    if(!$_SESSION['live'] && $scope->debug) $output .= print_alert("Debug: ".$scope->debug, "info");
    if($message = $scope->message ?: $scope->messages) $output .= print_alert($message, "message");
    if($warning = $scope->warning ?: $scope->warnings) $output .= print_alert($warning, "warning");
    if($error = $scope->error ?: $scope->errors) {
      if(1) { /** todo: test if printed in template */
        //$output .= form_text('foo', $template);
        $output .= print_alert($error, "error");
      }
    }
  }
  
  return $output.$template;
}

function smarty_reverse_route($routeName, $params=[]) {
  global $__APP;  
  return reverse_route($__APP, $routeName, $params);
}

function smarty_case($scase) {
  switch(strtolower($scase)) {
    case 'u': 
      $case = UPPERCASE;
      break;
    case 'l': 
      $case = LOWERCASE;
      break;
    case 'c': 
      $case = CAPITALIZE;
      break;
    case 't': 
      $case = TITLECASE;
      break;
    default:  
      $case = -1;
      break;
  }
  return $case;
}

// this translates and replaces: accepts a varying number of arguents after $term and $scase, which are passed to phrase. No options accepted
function smarty_transplace($term, $scase='') {
  $args = func_get_args();
  $args[1] = smarty_case($scase);
  $phrase = call_user_func_array('phrase', $args);
  return $phrase;
}

// set case to ne or editable to 0 to prevent in-line editable 
function smarty_translate($term, $scase = '', $editable=1) {
  global $__LANG, $__SITE, $__LOCAL_TEMPLATE, $__AUTH_EDIT, $__EDIT_MODE;
  print_log("smarty_translate: $term ($__LANG) $case", 'dictionary', LOG_LEVEL_TEST);
  $case = smarty_case($scase);
  $options = [];
  if($__LOCAL_TEMPLATE) $options['site_id'] = $__SITE->id;
  $phrase = phrase($term, $case, $options);

  if($scase == 'ne') $editable = 0;
  if($__AUTH_EDIT && $__EDIT_MODE && $editable) { 
    global $__PAGE;
    $term = strtolower(trim(str_replace(' ', '_', $term)));
    $dic_id = find_object('dictionary', ['term' => $term, 'language' => $__LANG], 'id');
    print_log("smarty_translate: edit $term ($__LANG) id: $dic_id", 'dictionary', LOG_LEVEL_TEST);
    
    if($dic_id) {
      $id = site_edit_id($__SITE, $__PAGE, 'dictionary', 'phrase', $dic_id, array('type' => 'text'));
      $edit_options = array('id' => $id);
      $phrase = html_span($phrase, '__edit __dic', $edit_options); // allow edit
    }
  }
  return $phrase;
}

function smarty_dump($obj) {
  return dump($obj, true);
}

function smarty_find_object($obj_type, $search, $field='') {
  $search = json_decode($search, true);
  $result = find_object($obj_type, $search, $field);
  return is_object($result) ? (array) $result : $result;
}

function smarty_find_objects($obj_type, $search, $options=[]) {
  if(!is_array($search)) $search = json_decode($search, true);
  $result = find_objects($obj_type, $search, $options);
  return $result;
}
function smarty_get_object($id, $obj_type, $field='') {
  $result = get_object($obj_type, $id, $field);
  return is_object($result) ? (array) $result : $result;
}

function smarty_property_geolink($obj_or_id, $path = '') {
  global $__SITE;
  $obj = any2obj('property', $obj_or_id);
  //if(!$path && is_property_type($obj->type)) $path = site_page_link($__SITE, property_type2page($obj->type));
  $options = [];
  if($path) $options['path'] = $path;
  return property_geolink($obj, $options);
}
  
// set page to make non-db lookup
function smarty_property_link($obj_or_id, $path = '') {
  return smarty_property_geolink($obj_or_id, $path);  
}

function smarty_editor($id, $options = array()) {
  $pageElement = get_object('page_element', $id);
  $val = $pageElement->content;
  $fld = "page_element-$id";
  
  
  $editor = form_text('content', $val, array("class" => "summernote form-control special", "data-height" => $fck_h, "data-full" => 1));
  $hidden = form_input("obj_type", "hidden", "page_element");
  $hidden .= form_input("id", "hidden", $id);
  $submit = form_input("submit", "submit", "Save", array("class" => "btn btn-success"));
  $form = html_form($hidden.$editor.$submit, "form-".$fld, '', 'post', 'ajax-submit', array('data-handler' => 'save'));
  return $form;
}

function smarty_uploader($obj_type, $id, $options = array()) {
  global $u_id;
  $field = isset($options['field']) ? $options['field'] : 'media'; /** todo: look at class def, pick fist media type */
  
  $output = "Uploader for media $obj_type $id";
  
  if($options && !is_array($options)) $options = json_decode($options, true); /** we now use json in attribute definition */ 

  //print_log("smarty_uploader options=".dump($options, true), 'upload', LOG_LEVEL_TEST);
  
  // $output .= dump($options, true);
  //die("smarty_uploader obj_type=$obj_type id=$id".caller_stack());
  $obj = get_object($obj_type, $id);
  if(!$obj) return print_errors("$obj_type $id not found");
  
  //return "Uploader media $id".dump($options, true);
  $resize = $options['resize'] ?: 0;
  
  $upload_options = array("resize" => 1);  
  $upload_options = array_extend($upload_options, $options);

  $upload_options['allow_multiple'] = 1; /** disable for now */
  
  //$upload_options['minimum_size'] = array(1080, 1920); // either short side >= 1080 or long side >= 1920
  $val = $id;
     
  $mediaData['user_id'] = $u_id;
  $mediaData['parent_id'] = $parent_id = $id;
  $mediaData['parent_type'] = $parent_type = $obj_type;
  $mediaData['parent_field'] = $fld = $field;
  //$mediaData['name'] = $mediaObj->name;
  
  $upload_options['download_template'] = $options['download_template'] ?: "";
  
  $upload_options['media_data'] = $mediaData;
  $upload_options['data-value_list'] = $val;
  $upload_options['show_file_list'] = false;
  $upload_options['title'] = "Click or drag to upload media";
  $upload_options['resize'] = $resize; /** resize to max px longest side ? */
  
  $handler = isset($options['handler']) ? $options['handler'] : '';
  if($upload_options['handler'] = $handler) {
    $handler_options = array();
    $upload_options['handler_options'] = $handler_options;
  }

  $uploader = jquery_file_uploader("$parent_type-$parent_id-$fld", $val, $upload_options);
  
  $output = $uploader;
  return $output;
}

// media uploader
function smarty_media_uploader($id, $options = array()) {
  global $u_id;
  $output = "Media Uploader for media $id";
  if($options && !is_array($options)) $options = json_decode($options, true); /** we now use json in attribute definition */ 

  $output .= dump($options, true);
  
  $mediaObj = get_object('media', $id);
  if(!$mediaObj) return print_errors("Media $id not found");
  
  //return "Uploader media $id".dump($options, true);
  $resize = $options['resize'] ?: 0;
  
  $upload_options = array("resize" => 1);  
  $upload_options = array_extend($upload_options, $options);
  $upload_options['allow_multiple'] = 0; /** disable for now */
  
  //$upload_options['minimum_size'] = array(1080, 1920); // either short side >= 1080 or long side >= 1920
  $val = $id;
     
  $mediaData['user_id'] = $u_id;
  $mediaData['parent_id'] = $parent_id = $mediaObj->parent_id;
  $mediaData['parent_type'] = $parent_type = $mediaObj->parent_type;
  $mediaData['parent_field'] = $fld = $mediaObj->parent_field;
  $mediaData['name'] = $mediaObj->name;
  
  $upload_options['download_template'] = $options['download_template'] ?: "";
  
  $upload_options['media_data'] = $mediaData;
  $upload_options['data-value_list'] = $val;
  $upload_options['show_file_list'] = false;
  $upload_options['title'] = "Click or drag to upload profile image";
  $upload_options['resize'] = $resize; /** resize to max px longest side ? */
  
  $handler = isset($options['handler']) ? $options['handler'] : '';
  if($upload_options['handler'] = $handler) {
    $handler_options = array();
    $upload_options['handler_options'] = $handler_options;
  }

  $uploader = jquery_file_uploader("$parent_type-$parent_id-$fld", $val, $upload_options);
  
  $output = $uploader;
  return $output;
}


function smarty_editable($val, $obj_type, $obj_id, $field = '', $options = []) {
  global $__SITE, $__PAGE, $__AUTH_EDIT, $__EDIT_MODE;
  
  if(!$__AUTH_EDIT || !$__EDIT_MODE) return $val;
  
  $auth_edit = auth_action($obj_type, 'edit', $obj_id);
  if(!$auth_edit) return $val;
  
  $id = site_edit_id($__SITE, $__PAGE, $obj_type, $field, $obj_id);
  return html_section($val, '__edit', ['id' => $id]);
}
  
// implements function in smarty template that allows editing of variables:
// {editable var="page.title"}
function smarty_edit($attrs) {
  global $__PAGE, $__SITE, $__AUTH_EDIT, $__EDIT_MODE;
  $pageObj = $__PAGE;
  $siteObj = $__SITE;
  //$pageElementArray = $_SESSION['page_elements'];  

  list($obj_type, $field) = explode(".", $attrs['var']);

  if(!in_array($obj_type, array("page", "page_content", "site"/*, "page_element"*/))) return "smarty_edit error: unsupported object type $obj_type";
  if($obj_type == 'page') $obj_type = 'page_content';

  $class_def = get_class_def($obj_type, false);  
  $attr = $class_def->fields[$field];
  if(!$attr) return "smarty_edit error: $obj_type has no field $field.";//.dump($attrs, true);

  $type = $attr[ATT_TYPE];
  if($type == 'str') $type = 'text';

  switch($obj_type) {
  case "page_content":
    $value = $pageObj->$field;
    $edit_id = $pageObj->content_id;
    break;
  case "site":
    $value = $siteObj->$field;
    $edit_id = $siteObj->id;
    break;
  }
  if(!$__AUTH_EDIT || !$__EDIT_MODE) return $value;      

  $edit_class = '__edit '; 
  $media_button = '';
  if(in_array($type, array('image','audio','video', 'media'))) {
    $media_button = html_div(html_image("/images/icons/".$type.".png", 'edit'), 'ie_edit_link');
    $safe_name = url_safe($name);
    $page_id = $pageObj->id; // name might change, so use ID
    $media_options['path'] = "/user/page/$page_id/$safe_name/"; 
    $media_options['allow_multiple'] = false;
    $media_button = "smarty_edit media: Edit $type";
    return $media_buttons;
  }
  $id = site_edit_id($siteObj, $pageObj, $obj_type, $field, $edit_id, array('type' => $type));
  $edit_options = array('id' => $id);
  
  if($media_options) {
    $class_options->$field = array(ATT_OPTIONS => $media_options); // override options of field content
  }
  $output = html_section($value.$media_button, '__edit', $edit_options);
  return $output;
  
  //echo("Smarty edit ot=$obj_type fld=$field");
}

    
function site_application($app, $options = []) {
  apply_controller($app, []);  
  if(isset($app->page->scope->template) && $app->page->scope->template) 
    $app->page->template = $app->page->scope->template; /** soft redirect from controller */

  //dump($app->header);
  $request = $_SERVER['REQUEST_URI'];
  $redirect = $app->page->scope->redirect;
  $sub = $app->page->subview;
  $tpl = $app->page->template;
  $ctrl = $app->page->controller;
  $error = $app->page->scope->error;
  $partial_view = 0;
  
  print_log("0: request={$request} sub={$sub} base={$base} tpl:{$tpl} ctrl:{$ctrl} error:{$error} redirect:{$redirect}", 'app', LOG_LEVEL_TEST);
  if($redirect) redirect($redirect);
  
  if($app->page->user_level > $app->user_level) {
    if($sub) {
      unset($app->page->subtemplate);
      unset($_GET['_pv']);
      $base = str_replace("/{$sub}", "", $request);
      $base = remove_query_var($base, '_pv');      
      print_log("1: request={$request} sub={$sub} base={$base}", 'app', LOG_LEVEL_TEST);
      $request = $base;
    } else {
      print_log("2: request={$request}", 'app', LOG_LEVEL_TEST);
      //die("2: request={$request}");
    }
    $login_page = $_GET['_new'] ? site_page_link($app->site, PAGE_REGISTER) : site_page_link($app->site, PAGE_LOGIN);      
    $redirect = "{$login_page}?ref=".urlencode(remove_query_var($request, '_new')).($aid = $_GET['_aid'] ? "&_aid=$aid" : '');
    //print_log("2: request={$request}", 'set_env', LOG_LEVEL_TEST);
    //die("redirecting to {$redirect}");
    redirect($redirect);
  } else {
    print_log("3: request={$request} sub={$sub}", 'set_env', LOG_LEVEL_TEST);
    //die("3: request={$request} sub={$sub}");
  }
  if($app->live && $app->site->google_analytics_id) $app->header->code[] = google_tracker($app->site->google_analytics_id);
  if($app->page->user_level > $app->user_level) die("Not authorized (page level > user level)");  
  if($app->page->subtemplate && isset($_GET['_pv']) && $_GET['_pv']) {
    $partial_view = 1;
    $app->page->template = $app->page->subtemplate; /** load a partial view (ajax, subview) */
  }
  //if(isset($_GET['_pv'])) die("subtpl={$app->page->subtemplate}");
  $output = smarty_template($app, $app->page->template, []);
  if($partial_view && ($scope.error || $scope.warning || $scope.message)) { /** manually prepend messages */
    $output = smarty_template($app, '_page-messages.html', []).$output;
  }
  return $output;
}

// also used as drop-in for old style templates in mail_template() function
function smarty_mail_template($siteObj, $filename, $options = []) {
  $this_app = new stdClass;
  $pageObj = new stdClass;
  $pageObj->scope = $scope = isset($options['scope']) ? array2obj($options['scope']) : new stdClass;  
  $pageObj->controller = $controller = isset($options['controller']) ? $options['controller'] : '';  
  $this_app->site = $siteObj;
  $this_app->page = $pageObj;
  //print_log("smarty_mail_template: stack:".caller_stack(), 'app', LOG_LEVEL_TEST);
  //print_log("smarty_mail_template: file={$filename} ctrl={$controller} scope:".dump($scope,true), 'app', LOG_LEVEL_TEST);
  if($controller) {
    apply_controller($this_app, []);
  }
  unset($options['scope']);
  unset($options['controller']);
  $filename = 'mail/'.str_replace('mail/', '', $filename); // remove if alrady there 
  return smarty_template($this_app, $filename, $options);

}

function smarty_template($app, $filename, $options = array()) {
  $siteObj = $app->site;
  $pageObj = $app->page;  
  print_log("smarty_template: {$filename}", 'smarty', LOG_LEVEL_TEST);
  if(!$filename) mydie("smarty_template: no filename");
  if(0 && $app->live) {
    $approot = approot(false);
    if(!strpos($filename, '.html')) $filename .= ".html";
    if($filename[0] == '_') $filename = "components/{$filename}"; // components
    $tpl_filename = "{$approot}/smarty/templates/{$filename}";
  } else {
    if(!$pageObj->path) $pageObj->path = $app->page->path = this_path();
    $pageObj->page_elements = $pageElementArray = smarty_page_elements($app);    
    $tpl_filename = smarty_preparse_template($app, $filename); // parse our own template structure, write smarty template
    if(!$tpl_filename) die("smarty_template: preparse $filename returned null - template=$tpl_filename");
  }
  if($pageObj->http_header) header($pageObj->http_header);
  if($pageObj->http_status) header($pageObj->http_status);  
  $output = smarty_parse_template($app, $tpl_filename, $options); // now run in through smarty for further parsing      
  return $output;       
}

// takes our own template file, parses it, writes a smarty template and returns filename of parsed smarty template if successful
function smarty_preparse_template($app, $filename, $options = array()) {  
  global $__LOCAL_TEMPLATE, $__TEMPLATE, $__TEMPLATE_PATH;
  $siteObj = $app->site;
  $pageObj = $app->page;
  
  $approot = approot(true);
  $docroot = docroot(true);
  $libroot = libroot(true);
         
  $smarty_dir = $approot.'smarty/templates';
  
  if(!strpos($filename, '.html')) $filename .= ".html";
  if($filename[0] == '_') $filename = "components/{$filename}"; // components
  
  //if(0 && $_SESSION['live'] && file_exists("$smarty_dir/$filename")) { // for www, no preparsing, use already parsed smarty template
  //  return "$smarty_dir/$filename";
  //}                
  
  $__LOCAL_TEMPLATE = 0;
                     
  $site_template_file = $approot."pages/$filename";
  $app_template_file = $libroot."pages/$filename"; 
  if(file_exists($site_template_file)) {
    $__LOCAL_TEMPLATE = 1;
    $template_file = $site_template_file;
  } else if(file_exists($app_template_file)) {
    $template_file = $app_template_file;
  } else {
    if(!$_SESSION['live']) die("$filename not found in $site_template_file or $app_template_file");
    return ''; // no template
  }

  print_log("site: {$site_template_file} lib:{$app_template_file}", 'smarty', LOG_LEVEL_TEST);
  print_log("smarty_preparse_template {$filename} => {$template_file}", 'smarty', LOG_LEVEL_TEST);
  
  $__TEMPLATE = $filename;
  $__TEMPLATE_PATH = $template_file;
  $raw_template = read_file($template_file);
  if(!$raw_template) return '';
  //echo("read ".strlen($raw_template)." from $template_file<br>");
  $now = now();
  $options['context'] = $template_file;
  

  
  $parsed_template = "{* automatically generated on $now from $template_file. DO NOT EDIT *}".PHP_EOL.
    smarty_preparser($app, $raw_template, $options).PHP_EOL;
    
  $smarty_template = $approot."smarty/templates/$filename";
  $path_parts = pathinfo($smarty_template);
  $dir = $path_parts['dirname']."/";
  if(!is_dir($dir) && !mkdir($dir, 0775, true)) die("smarty_preparse_template: Failed to create $dir");
  if(!write_file($smarty_template, $parsed_template)) die("smarty_preparse_template: Failed to write to {$smarty_template}");     

  //echo("wrote ".strlen($parsed_template)." to $smarty_template<br>");
  return $smarty_template;
  
}


/** used to by preparser */
function smarty_preparser($app, $template, $options = array()) {
  global $__LIVE,$__LOCAL_TEMPLATE,$__TEMPLATE, $__AUTH_EDIT;
  
  print_log("smarty_preparser len=".strlen($template), 'smarty', LOG_LEVEL_TEST);

  $pattern = '|\{\{\s?(.*?)\s?\}\}|s'; // ungreedy to allow more on one line
    
  $output = preg_replace_callback (
      $pattern, 
      function($matches) use ($app) {       
        $match = $matches[1];
        if(!$match) return '';
        if($match[0] == '"' || $match[0] == "'" || $match[0] == "[") { // json?          
          return parse_template_json($app, $match);
        } else if($match[0] == '$') { // variable
          return parse_template_variable($app, $match);
        } else { // no match.
          return ' { '.$match.' } ';
        }
      }, 
      $template);
  $debug = '';
  //if(!$__LIVE) $debug = "tmp: {$__TEMPLATE} local:{$__LOCAL_TEMPLATE}<br>";
  return $debug.$output;
}


// template parsing functions
function parse_template_json($app, $match) { 
  global $__AUTH_EDIT, $__EDIT_MODE, $u_id;
  $json = $match[0] == '[' ? '["'.str_replace('"', '\"', substr($match, 1, -1)).'"]' : "{".$match."}"; // turn back into json array or object
  $json = str_replace("\n", '', $json);
  $data = json_decode($json);
  $siteObj = $app->site;
  $pageObj = $app->page;

  if(!$data) {
    return ''; // todo: show error
  }
  
  $pageElementArray = $pageObj->page_elements;  

  print_log("parse_template_json data:".dump($data,true), 'smarty', LOG_LEVEL_TEST);

  
  if(is_array($data) && count($data)) { // this allows simplified tags for page_elements {{[ and ]}}
    //echo("data is array".dump($data, true));
    $value = $data[0]; // remove [ and ]
    $data = new stdClass;
    
    $rand = hash('adler32', $value);
    $data->name = "pe-$rand";
    $data->type = $type = strip_tags($value) == $value ? "text" : "html"; // if strip_tags == str, text, else html
    $data->value = $value;
    //echo("data is now obj".dump($data, true));
    //die();
    
  } else if($filename = $data->extend) { // include file to be parsed
    //echo(" found include $filename, calling preparse...<br>");
    unset($data->extend);
    if($tpl_filename = smarty_preparse_template($app, $filename)) {
      return "{extends file=\"$tpl_filename\"}";
    }
    mydie("parse_template_json: preparse extend file $tpl_filename for file {$filename} returned null".dump($data,true));
  } else if($filename = $data->include) { // include file to be parsed
    //echo(" found include $filename, calling preparse...<br>");
    unset($data->include);
    $data_str = array2args($data);
    if($tpl_filename = smarty_preparse_template($app, $filename)) {
      return "{include file=\"$tpl_filename\" $data_str}";
    }
    die("parse_template_json: preparse include file {$filename} returned null");
  } else if($subviews = $data->subviews) { // include array of files to be parsed
    //return;
    if($prefix = $data->prefix) {
      unset($data->prefix);
    }
    $data_str = array2args($data);  
    $subviews_str = implode(',', $subviews);
    $i = 0;
    $output = '';
    foreach($subviews as $subview) {              
      $name = "{$prefix}{$subview}";
      if($template = smarty_preparse_template($app, $name)) {
        print_log("View::parseJsonTag: creating subview {$template} for {$name} prefix:{$prefix}", 'debug', LOG_LEVEL_TEST);
        if(!$i) $first_template = "{$name}.html";
        print_log("View::parseJsonTag: created subview {$template}");
        $i++;
      } else {
        $output .= print_alert("Subview: {$name} does not exist!", 'error').PHP_EOL;
      }
    }
    $output .= "<!-- {$i} Subviews: {$subviews_str} First: {$first_template}-->".PHP_EOL;
    if($prefix) $output .= "<div id='subview-prefix' class='collapse'>{$prefix}</div>".PHP_EOL;
    $output .= '  {$tpl = $app.page.subtemplate|default:"'.$first_template.'"}'.PHP_EOL;  /** include first view by default */
    $output .= '  {include file="components/$tpl" '.$data_str.'}'.PHP_EOL;
    return $output;
  }
  
  $debug = '';
  $name = $data->name;
  
  $dbObj = $pageElementArray[$name];
  $type = $data->type;
  $value = $dbObj ? $dbObj->content : $data->value;
  $options = $data->options ? (array) $data->options : array();
  $element_id = $dbObj->id;
  
  $keys = array('class', 'style', 'js', 'alt'); /** todo, add more here that's relevant for tag */
  foreach($keys as $key) {
    if(isset($data->$key)) $options[$key] = $data->$key;
  }
  
  $store = true;
  $field = 'content';
  $media_options = array();
  if(in_array($type, array('image','audio','video', 'media'))) {
    $media_options = media_options(array('resize' => 1, 'allow_multiple' => false));
    $field = 'media';
    $media_id = $dbObj->media;
    if(!$media_id && $element_id) { /** add to database so we can edit later */
      $page_id = $dbObj->page_id ?: $pageObj->id;
      $mediaData = array();
      $mediaData['name'] = $name;
      $mediaData['path'] = "/user/page/$page_id/$element_id/$type/";
      $mediaData['parent_type'] = 'page_element';
      $mediaData['parent_id'] = $element_id;
      $mediaData['parent_field'] = 'media';
      $mediaData['user_id'] = $u_id;      
      $mediaData['url'] = is_url($value) ? $value : site_full_url($siteObj)."/".ltrim($value, '/');
      $mediaData['resize'] = $data->resize ?: 800; /** resize to max 800px longest side ? */
      //$debug = dump($mediaData, true);
      if(1) {      
        print_log("parse_template_json: resize=".dump($mediaData['resize'], true), 'resize', LOG_LEVEL_TEST);
        list($media_id, $media_errors) = add_object('media', $mediaData);
        if($media_id) update_object('page_element', array('media' => $media_id), $dbObj->id);
        else if($media_errors) $debug = print_errors($media_errors);
      }
    }
    $media_src = $media_id ? print_media($media_id, array('source_only' => true)) : $value;
    //$media_src = $media_id ? print_media($media_id, array('source_only' => true)) : $value;
  }

  $obj_type = 'page_element'; // default  for page_element
  $edit_id = $dbObj->id; // default for page_element
  
  if($name[0] == '$') { // this is from page_content, not page_element
    $obj_type = 'page_content';
    $field = ltrim($name, '$');
    $edit_id = $pageObj->content_id;
    $store = false; // don't store, this is from $pageContentObj
    $class_def = get_class_def($obj_type, false);
    
    if($attr = $class_def->fields[$field]) {
      //dump($class_def);
      $type = $attr[ATT_TYPE];
      if($type == 'str') $type = 'text';
      $value = $pageObj->$field;
    } else {
      $output = "<!-- Parse error - Unknown page_content variable '$field': $json -->";
      return $output;      
    }
  }
    
  switch($type) {                   
  case 'text':
    $output = $value;
    break;
  case 'html':
    $output = $value;
    break;
  case 'image':
    //$output = $media_src;
    $output = html_image($media_src, $options['alt'], $options);
    //$output .= dump($dbObj, true);
    $media_options['allowed_types'] = 'image';
    break;
  case 'video':
    $output = html5_video($media_src, $options['alt'], $options);
    $media_options['allowed_types'] = 'video';
    break;
  case 'audio':
    $output = html5_audio($media_src, $options['alt'], $options);
    $media_options['allowed_types'] = 'audio';
    break;
  case 'media':
    $output = html_image($value, $options['alt'], $options);
    break;
  case 'variable':
    $store = false;
    $output = '<!--$'.$name.'-->'; "Variable $json";
    break;
  default:
    $store = false;
    $output = "<!-- Parse error - Unsupported type '$type': $json -->";
    break;
  }

  //echo("Name=$name type=$type field=$field store=$store<br>");

  if($store && !$pageElementArray[$name]) { // store in database if doesn't exist
    $path = $pageObj->path;
    $db_data = array();
    if($ms = $pageObj->minisite) {
      $path = "/{$ms}".$path;
    } 
    $db_data['page_id'] = $pageObj->id;

    $db_data['site_id'] = $siteObj->id;
    $db_data['path'] = $path;
    $db_data['language'] = $pageObj->language;
    $db_data['name'] = $name;
    $db_data['type'] = $type;
    $db_data['content'] = $value;
    $db_data['options'] = $options ? json_encode($options) : '';
    
    //$session_id = session_id();
    //$session_dir = session_save_path();    
    //mydie("sess: {$session_dir}/sess_{$session_id} adding page el for globals".dump($GLOBALS,true));
    //mydie("sess: {$session_dir}/sess_{$session_id} adding page el for session".dump($_SESSION,true));
    //dump($db_data);mydie("sess: {$session_dir}/sess_{$session_id} adding page el for page:".dump($pageObj,true));
    
    list($id, $errors) = add_object('page_element', $db_data);
    if($errors) {dump($db_data);mydie("<br>Add Element Error:<br>".dump($errors,true));}
    
    $edit_id = $id;
  }
  
  if($__AUTH_EDIT && $__EDIT_MODE) {
    $edit_class = '__edit'; 
    $media_button = '';
    if(in_array($type, array('image','audio','video', 'media'))) {
      $media_button = html_div(html_image("/images/icons/".$type.".png", 'edit'), 'ie_edit_link');
      $safe_name = url_safe($name);
      $page_id = $pageObj->id; // name might change, so use ID
      
      $upload_options = array();
      $upload_options['path'] = "/user/page/$page_id/$safe_name/"; 
      $upload_options['allow_multiple'] = false;
      
      $resize = $data->resize;
      if($media_id) {
        $output = '{$options = ["resize" => "'.$resize.'"]}'.PHP_EOL;
        $output .= '{$json = $options|@json_encode}'.PHP_EOL;
        $output .= '{"media"|uploader:'.$media_id.':"$json"}'.PHP_EOL;
      } else {
        $output = "parse_template_json: No media_id for $type $name (try reload)";
      }      
      return $output;
      
    } else if(0 && $type == 'html') {
      return "{\"$element_id\"|editor:\"full\"}";
    }
    
    
    $edit_id = site_edit_id($siteObj, $pageObj, $obj_type, $field, $edit_id, array('type' => $type));
    $edit_options = array('id' => $edit_id);
    //dump($edit_options);
    
    if($media_options) {
      $class_options->$field = array(ATT_OPTIONS => $media_options); // override options of field content
    }
    if(!strlen($output)) $output = "-- empty {$name} --";
    $output = html_div($output, '__edit', $edit_options);
  }
  //$debug = "Auth={$__AUTH_EDIT} edit={$__EDIT_MODE}<br>";
  //$debug = "Auth={$__AUTH_EDIT} edit={$__EDIT_MODE} edit_id={$edit_id}<br>";
  //$debug = '';
  return $debug.$output;
  
}

// template parsing functions
function parse_template_variable($app, $match) {
  $pattern = '#\$(page|site|template|user|scope)\.([a-zA-Z_\-0-9]+)\.?([a-zA-Z_\-0-9]+)?\.?([a-zA-Z_\-0-9]+)?\.?([a-zA-Z_\-0-9]+)?#';
  if(preg_match($pattern, $match, $m)) {
    $var = $m[1];
    $key = $m[2];
    if($var && $key) {
      switch($var) {
      case 'site':
        return $siteObj->$key;
        break;
      case 'page':
         $pageObj = $_SESSION['pageObj'];
         return $pageObj->$key;
        break;
      case 'template':
        $pageElementArray = $pageObj->page_elements;  
        $pageElementObj = $pageElementArray[$key];
        return $pageElementObj->content;
        
      case 'user':
        $userObj = $app->user;
        return $userObj->$key;
    
      case 'scope':
        $count = count($m);
        $scope = $pageObj->scope;
        $i=2;
        while($i < $count) { // traverse object chain to leaf
          $key = $m[$i];
          $result = $scope->$key;
          $scope = $result;
          $i++;
        }
        if(is_object($result)) {
          $chain = array_slice($m, 2);
          $result = implode(".",$chain).' = [object]';
        }
        return $result;
        break;
      default:
        return '';
        break;
      }
    }
  }
}


// move everything from one site to another
function move_site_content($from_site, $to_site, $options = array()) {
  $execute = $options['execute'];
  $classes = get_classes();
  $count = count($classes);
  echo("There are $count classes<br>");
  //dump($classes);die();
  $classAr = array();
  foreach($classes as $class) {
    $obj_type = $class->type;
    $fields = class_fields($obj_type, false);
    if(in_array('site_id', $fields)) {
      $classAr[] = $obj_type;
    }
  }
  $count = count($classAr);  
  echo("There are $count classes with site_id<br>");
  $rows = array();
  $rows[] = array("obj_type", "table", "count");
  foreach($classAr as $obj_type) {
    $table = get_object_table($obj_type);
    $count = db_getOne("SELECT count(*) from $table WHERE site_id=$from_site");
    if($count) {
      $rows[] = array($obj_type, $table, $count);
      if($execute) db_query("UPDATE $table SET site_id=$to_site WHERE site_id=$from_site");
    }
  }
  $output = html_table($rows, array('class' => 'web20'));
  return $output;
}


/** Delete all tables not defined in classes */
function delete_unused_tables($app, $delete = false) {
  $classes = get_classes();
  $count = count($classes);
  $all_tables = db_getCol("SHOW TABLES");
  $db_count = count($all_tables);
  
  $rows = $class_tables = array();
  foreach($classes as $classObj) {    
    $obj_type = $classObj->type;
    $table = $classObj->table;
    $class_tables[] = $table;
    //$skip = ['dictionary'];
    //if(in_array($table, $skip)) continue;    
  }

  $cl_count = count($class_tables);
    
  echo("<h2>Class Tables:</h2>");

  $output .= "<h2>{$count} classes, {$cl_count}/{$db_count} db classes</h2>";
              
  foreach($all_tables as $db_table) {
    if(in_array($db_table, $class_tables)) {
      $output .= html_div($db_table, 'green');
    } else {
      if($delete) db_query("Drop table $db_table");
      $output .= html_div($db_table.($delete ? " (deleted)" : ''), 'red strike');
    }      
  }
  return $output;
}

/** Delete all content in database that does not belong to this site/client */
function delete_site_content($siteObjorId, $options = array()) {
  $siteObj = any2obj('site', $siteObjorId);
  $site_id = $siteObj->id;
  $client_id = $siteObj->client_id;
  $legal_id = $siteObj->legal_entity_id;

  if(!$site_id) die("No site found: ".dump($siteObjorId, true));
  if(!$client_id) die("No client for site {$site_id}");
  if(!$legal_id) die("No legal entity for site {$site_id}");
  
  $execute = true;
  $classes = get_classes();
  $count = count($classes);

  $output .= "<h2>{$count} classes</h2>";
  //dump($classes);
  $rows = array();
  foreach($classes as $classObj) {    
    $obj_type = $classObj->type;
    $table = $classObj->table;
    $skip = ['dictionary', 'phpauth_config', 'phpauth_attempts', 'phpauth_requests', 'phpauth_sessions'];
    if(in_array($table, $skip)) continue; 
    $fields = class_fields($obj_type, false);
    
    if(in_array('site_id', $fields)) {
      if($execute) db_query("DELETE FROM {$table} WHERE site_id!={$site_id}");
      $rows[] = "Deleted by site_id from {$table}";
    } else if(in_array('client_id', $fields)) {
      if($execute) db_query("DELETE FROM {$table} WHERE client_id!={$client_id}");
      $rows[] = "Deleted by client_id from {$table}";
    } else if(in_array('legal_entity_id', $fields)) {
      if($execute) db_query("DELETE FROM {$table} WHERE legal_entity_id!={$legal_id}");
      $rows[] = "Deleted by legal_entity_id from {$table}";
    } else {
      $rows[] = "{$obj_type} ({$table}) has no site/legal/client in fields";
    }
  }
   if($execute) db_query("DELETE FROM site WHERE id!={$site_id}");
  
  $output .= implode(html_break(), $rows);
  return $output;
}


// exports all content needed for a site to live on a another database (sql file)
// todo: figure out which address to export;

function export_site_content($site_id, $options = array()) {
  $siteObj = get_object('site', $site_id);
  $client_id = $siteObj->client_id;  
  $clientObj = get_object('client', $siteObj->client_id);
  $legal_id = $siteObj->legal_entity_id;
  $legalObj = get_object('legal_entity', $siteObj->legal_entity_id);

  
  $exportAll = array('address_type','article_type','countries','currency', 'language', 'property_type', 'description_type','media_type','mime_type','page_type','phone_type','provider','provider_key','security','site_template','user_level','user_role', 'mail_queue_seq');   
  
  $export['all'] = $exportAll;   // export structure + all data
  $export['where'] = array();    // export structure + data matching WHERE condition
  $export['struct'] = array();   // export structure only

  
  $execute = $options['execute'];
  $classes = get_classes();
  $count = count($classes);
  
  $property_site = $siteObj->show_types; // todo check
  $payment_site = $siteObj->payment_site;
  $user_site = $siteObj->allow_user_login || $siteObj->allow_user_register;
  $gallery_site = $siteObj->gallery_site;
  
  
  $property_tables = array();
  $payment_tables = array();
  
  //echo("There are $count classes<br>"); 
  //dump($classes);die();
  $classAr = array();
  foreach($classes as $classObj) {    
    $obj_type = $classObj->type;
    $table = $classObj->table;
    $group = $classObj->group;
    if(!$property_site && in_array($group, array('event', 'property', 'enquiry', 'location', 'property-user', 'property-content'))) continue;
    if(!$user_site && $group == 'user') continue;
    if(!$payment_site && in_array($group, array('accounting', 'product'))) continue;
    // if(!$gallery_site && $group == 'gallery') continue; (we need these for image upload)
    if(!$pet_site && $group == 'pet') continue;
    if(!$golf_site && $group == 'golf') continue;
    if(!$social_site && $group == 'social') continue;
    if(!$content_site && $group == 'content') continue;
    

    $fields = class_fields($obj_type, false);

    $classObj->has_site   = $has_site   = in_array('site_id', $fields);
    $classObj->has_client = $has_client = in_array('client_id', $fields); 
    $classObj->has_legal  = $has_legal  = in_array('legal_entity_id', $fields);

    $classObj->site_count = $classObj->has_site ? db_getOne("SELECT count(*) from $table WHERE site_id=$site_id") : -1;    
    $classObj->client_count = $classObj->has_client ? db_getOne("SELECT count(*) from $table WHERE client_id=$client_id") : -1;
    $classObj->legal_count  = $classObj->has_legal  ? db_getOne("SELECT count(*) from $table WHERE legal_entity_id=$legal_id") : -1;
    
    $where = "";
    if($obj_type == 'site') {
      $has_site = true;
      $where = "id=$site_id";
      $classObj->site_count = db_getOne("SELECT count(*) from $table WHERE $where") ?: -1;
    } else if($obj_type == 'client') {
      $has_client = true;
      $where = "id=$client_id";
      $classObj->client_count = db_getOne("SELECT count(*) from $table WHERE $where") ?: -1;
    } else if($obj_type == 'legal_entity') {
      $has_legal = true;
      $where = "id=$legal_id";
      $classObj->legal_count = db_getOne("SELECT count(*) from $table WHERE $where") ?: -1;      
    }
    
    if($has_site || $has_client || $has_legal) {
      if(!$where) {
        if($classObj->site_count > 0) $where = "site_id=$site_id"; 
        else if($classObj->client_count > 0) $where = "client_id=$client_id"; 
        else if($classObj->legal_count > 0) $where = "legal_entity_id=$legal_id"; 
      }
      if($obj_type == 'user') $where = $where ? "$where OR id=501" : "id=501"; // add system user
      if($where) {
        $export['where'][$table] = $where;
      } else {
        $export['struct'][] = $table;
      }
      $classAr[] = $classObj;      
    }
    $all_tables = array_merge($exportAll, array_keys($export['where']), $export['struct']);
    $table_list = quoted_list($all_tables);
    $struct_table_list = quoted_list($export['struct']);
    $export['where']['db_sequence'] = "seq_name IN ($table_list)";
  }

  /** todo: this has changed
  $address_ids = array($siteObj->address_id, $legalObj->address_id, $clientObj->address_id, 44);
  if($user_aids = find_objects('user', array('site_id' => $site_id), array('field' => 'address_id'))) {
    $address_ids = array_merge($address_ids, $user_aids);
  }
  $address_id_list = implode(',', $address_ids);
  $export['where']['address'] = "id IN ($address_id_list)";
  */
  
  $export['where']['site_scheme'] = "id=$siteObj->scheme_id";
  $export['where']['site_config'] = "id=$siteObj->config_id";
  $export['where']['site_dim'] = "id=$siteObj->dim_id";
  $export['where']['property_config'] = "id=$siteObj->property_config_id";
  if(!$export['where']['site_content']) $export['struct'][] = 'site_content';
  
  // 'db_sequence', where seq_name in ();
  
  $count = count($classAr);  
  // echo("There are $count classes with site_id, client_id, or legal_entity_id<br>");
  //dump($classAr);exit;
  $rows = array();
  $rows[] = array("obj_type", "group", "table", "site", "client", "legal");
  foreach($classAr as $classObj) {
    $site_count   = $classObj->site_count > -1   ?  $classObj->site_count   : html_span("x", 'red');    
    $client_count = $classObj->client_count > -1 ?  $classObj->client_count : html_span("x", 'red');
    $legal_count  = $classObj->legal_count > -1  ?  $classObj->legal_count  : html_span("x", 'red');    
    $rows[] = array($classObj->type, $classObj->group, $classObj->table, $site_count, $client_count, $legal_count);
  }
  $output = html_table($rows, array('class' => 'web20'));

  $domain = $siteObj->domain;
  $base_dir = is_dir("/websites/test.$domain/db/dump") ? "/websites/test.$domain/db/dump" : "/backup/db/dump"; 

  $sql_export_filename = "$base_dir/export-site-".$siteObj->shortname."-".date('Ymd').".sql";
  $sql_export_file = '';
  $cmds = array();
  foreach($export['all'] as $table) $cmds[] = "mysqldump --user=apt --password=killa113 --compact --add-drop-table aptdb_v14 $table >> $sql_export_filename";
  foreach($export['struct'] as $table) $cmds[] = "mysqldump --user=apt --password=killa113 --compact --add-drop-table aptdb_v14 --no-data $table >> $sql_export_filename";
  foreach($export['where'] as $table => $where) $cmds[] = "mysqldump --user=apt --password=killa113 --compact --add-drop-table  aptdb_v14 $table --where=\"$where\" >> $sql_export_filename";
  
  $result = '';  
  
  if(is_file($sql_export_filename)) unlink($sql_export_filename);
  foreach($cmds as $cmd) {
    $result .= execRootCmd($cmd, $cmd_output, $return_var);
  }
  $shortname = $siteObj->shortname;
  
  $sql_import_filename = "$base_dir/import-site-$shortname-".date('Ymd').".sql";

  $sql_import_file = "source $sql_export_filename;\n";
  $sql_import_file .= "UPDATE db_sequence SET nextid=1000 WHERE seq_name in ($struct_table_list);\n";
  $len = write_file($sql_import_filename, $sql_import_file);
  if($len) {
    $output .= "OK, wrote $len to $sql_import_filename<br>";
  } else {
    $output .= "Error, failed to write to $sql_import_filename<br>";
  }

  if(is_file("$base_dir/../import-site-$shortname.sql")) unlink("$base_dir/../import-site-$shortname.sql");
  $cmd = "ln -s $sql_import_filename $base_dir/../import-site-$shortname.sql";  
  execRootCmd($cmd, $cmd_output, $return_var);
  $cmds[] = $cmd;
  if($options['debug']) {
    $cmd_str = dump($cmds,true);
    $output .= form_text('cmds', $cmd_str);
    $output .= form_text('result', $result);
  }
  
  return $output;
}

function controller_error($app, $error, $status = 400, $options = []) {
  $app->page->scope->error = $error;
  $app->page->template = $status;
  print_log("controller_error:".dump($error,true), 'app', LOG_LEVEL_ALWAYS);
  return;
}

function search_result($siteObj, $obj_type, $term, $search_fields, $id_field, $options = array()) {
  $scope = new stdClass;
  if(!$search_fields) die("search_result: no fields");
  $result = array();
  
  $length = $options['length'] ?: 80; // how many characters before/after to show

  // todo: start/offset for pagination
  $limit = $options['limit'] ?: 20; // how many characters before/after to show
  
  $obj_fields = object_fields($obj_type);
  $key = get_object_key($obj_type);
  $title_field = get_object_descriptor($obj_type);
  
  if(!is_array($search_fields)) $search_fields = (array) $search_fields;
  if(count($search_fields) > 1) {
    $search_array = array();
    foreach($search_fields as $field) $search_array[$field] = "*$term*";
    $search[DB_LOGICAL_OR] = $search_array;
  } else {
    $field = $search_fields[0];
    $search[$field] = "*$term*";
  }
  
  if(in_array('language', $obj_fields)) $search['language'] = $_SESSION['lang'] ?: $siteObj->default_language;
  if(in_array('active', $obj_fields)) $search['active'] = 1;
  $search['site_id'] = $siteObj->id;
    
  $ids = find_objects($obj_type, $search, array('field' => $key, 'limit' => $limit));

  $debug = false;
  //if($obj_type == 'comment') $debug = true;
  
  if($debug) {
    echo("Search for $obj_type");
    dump($search);
    dump($ids);
  }
                     
  // $debug = true;
  $page_ids = array();
  foreach($ids as $id) {
    $resultObj = new stdClass;
    $obj_fields = $search_fields;
    if(in_array($obj_type, array('page_element', 'page_content'))) $obj_fields[] = 'page_id';
    if(in_array('path', $obj_fields)) $obj_fields[] = 'path';
    if($obj_type == 'comment') $obj_fields[] = 'article_id';
    
    $obj = get_object($obj_type, $id, $obj_fields);
    if($debug) dump($obj);
    foreach($search_fields as $search_field) {
      if($debug) echo("Looking for $term in $search_field of $obj_type $id<br>");
      $val = count($obj_fields) > 1 ? $obj->$search_field : $obj; // the value of the field containing term
      $val = strip_tags($val);
      $pos = stripos($val, $term); 
      if($pos !== FALSE) {
        if($page_id = $obj->page_id) $page_ids[$page_id] = 1;
        
        $start = max($pos - round($length/2), 0);
        $str = substr($val, $start, $length);
        if($debug) echo(" - Found $term in $search_field of $obj_type $id<br>");
        if($debug) echo(" Result: $str<br>");
              
        $title = $obj->$title_field;
        
        // find link
        if($obj->path) { // e.g. page_content
          $link = $obj->path;
        } else if($page_id = $obj->page_id) { // e.g. page_element
          $link = site_page_link($siteObj, get_object('site_page', $page_id));
        } else { // article, event, other
          switch($obj_type) {
            case 'article';
              $link = "/nieuws/?id=$id";
            break;
            case 'comment';
              $link = "/nieuws/?id=".$obj->article_id;
              $title = get_object('article', $obj->article_id, 'title');
            break;
            case 'event';
              $link = "/evenementen/?id=$id";
            break;
            default:
            break;
          }
        }
        
        //$link = "/foo/";
        $resultObj->title = $title;
        $resultObj->link = $link;        
        $resultObj->string = $str;
        $result[] = $resultObj;
        break;
      } else {
        if($debug) echo(" - NO: pos=$pos<br>");
      }
    }
    
  }
  
  // special handling; page_elements, repeated code, but what the heck
  if($obj_type == 'page_content') {
    $search = array();
    $search['site_id'] = $siteObj->id;
    $search['content'] = "*$term*";
    if(count($page_ids)) $search['page_id'] = array('!=', array_keys($page_ids)); // exclude those already found    
    $search['language'] = $_SESSION['lang'] ?: $siteObj->default_language;    
    $ids = find_objects('page_element', $search, array('field' => 'id', 'limit' => $limit));
    foreach($ids as $id) {
      $obj = get_object('page_element', $id, array('page_id', 'content'));
      $page_id = $obj->page_id;
      if($page_ids[$page_id]) continue;
      $page_ids[$page_id] = 1;
      $pageObj = get_object('site_page', $page_id);

      $val = strip_tags($obj->content);
      $pos = stripos($val, $term); 
      $start = max($pos - round($length/2), 0);
      $str = substr($val, $start, $length);

      $resultObj = new stdClass;
      $resultObj->title = $pageObj->name; // todo: use page content title
      $resultObj->link = site_page_link($siteObj, $pageObj);
      $resultObj->string = $str;
      $result[] = $resultObj;
      
      //echo("Found $term in page_el $id on page $page_id<br>");     
    }
  }
  
  
  if($debug) dump($result);
  
  return $result;
}


function traveler_dependencies($app, $options = array()) {
  $siteObj = $app->site;
  $ver = $app->version ?: date('md');
  $v = "?v={$ver}";  
  $lang = strtolower($app->language);
  $dtp = 1; /** which datetimepicker */
  $tp = 1; /** which timepicker */
  $base_scripts = $app->base_scripts;
  $scripts = $app->scripts;
  $styles = $app->styles;

  $styles[] = "/afr/css/afr_base.css{$v}";  
  $styles[] = "/afr/css/afr_rental.css{$v}";  
  $styles[] = "/afr/css/afr_tooltip.css";

  if($u_id) $styles[] = "/afr/css/afr_cms.css?ver=$ver";
  if(is_file($_SERVER['DOCUMENT_ROOT']."/css/site_".$siteObj->shortname.".css")) $styles[] = "/css/site_".$siteObj->shortname.".css";
  $styles[] = "//fonts.googleapis.com/css?family=Roboto:400,300,100,500,700";
  $styles[] = "//fonts.googleapis.com/css?family=Open+Sans:400italic,400,300,600";
  
  $styles[] = "/afr/bower/bootstrap/dist/css/bootstrap.min.css";
  $styles[] = "/afr/font/afr/styles.css";

  $styles[] = "/traveler/css/icomoon.css";
  $styles[] = "/traveler/css/styles.css{$v}";
  $styles[] = "/traveler/css/mystyles.css{$v}";
  
  $styles[] = "/afr/bower/fancybox/source/jquery.fancybox.css";
  $styles[] = "/afr/bower/summernote/dist/summernote.css";
   
  $styles[] = "/afr/bower/bootstrap3-wysihtml5-bower/dist/bootstrap3-wysihtml5.min.css";
   
   
  $styles[] = "/afr/bower/bootstrap-toggle/css/bootstrap-toggle.min.css";
  $styles[] = "/afr/bower/bootstrap-modal/css/bootstrap-modal-bs3patch.css";
  $styles[] = "/afr/bower/bootstrap-modal/css/bootstrap-modal.css";  
  
  $styles[] = "/afr/bower/ionrangeslider/css/ion.rangeSlider.css";
  $styles[] = "/afr/bower/bootstrap-star-rating/css/star-rating.css";
  $styles[] = "/afr/css/google_pac.css"; 
  $styles[] = "/afr/bower/select2/select2.css";    
  $styles[] = "/afr/bower/flag-icon-css/css/flag-icon.min.css";    
  $styles[] = "/afr/bower/bootstrap-social/bootstrap-social.css";    
  $styles[] = "/afr/bower/intl-tel-input/build/css/intlTelInput.css"; /* Note: must set .iti-flag {background-image: url("/bower/intl-tel-input/build/img/flags.png");} in css */    
  $styles[] = "/afr/bower/magnific-popup/dist/magnific-popup.css";
  
  $styles[] = "/afr/bower/addtocalendar/atc.css?foo=bar";
    
  // top scripts
  $base_scripts[] = GMAP_API;  
  $base_scripts[] = "/afr/bower/jquery/dist/jquery.min.js"; // these are loaded first
  $base_scripts[] = "/afr/bower/jquery-ui/ui/minified/jquery-ui.min.js";
  $base_scripts[] = "/afr/node/moment/moment.js";    
  $base_scripts[] = "/afr/node/moment-range/dist/moment-range.js";  
  $base_scripts[] = "/afr/bower/bootstrap/dist/js/bootstrap.min.js";
  $base_scripts[] = "/afr/js/afr.js{$v}";
  $base_scripts[] = "/afr/js/afr_cms.js{$v}";
                                                              
  if($siteObj->property_site || $siteObj->property_type) $base_scripts[] = "/afr/js/afr_rental.js{$v}";
  $base_scripts[] = "/afr/bower/google-markerwithlabel/src/markerwithlabel.js";  
  $base_scripts[] = "/afr/js/google_pac.js{$v}";  
  $base_scripts[] = "/afr/bower/fancybox/source/jquery.fancybox.pack.js";
  $base_scripts[] = "/afr/bower/summernote/dist/summernote.min.js";
  $base_scripts[] = "/afr/bower/bootstrap-datepicker/js/bootstrap-datepicker.js?v=1.6.4";
  $base_scripts[] = "/afr/bower/bootstrap3-wysihtml5-bower/dist/bootstrap3-wysihtml5.all.min.js";

  $base_scripts[] = "https://use.fontawesome.com/a22cbf3bb2.js";
  $base_scripts[] ="script:window['moment-range'].extendMoment(moment)";
  if($lang !== 'en') $base_scripts[] = "/afr/bower/moment/locale/{$lang}.js";
  
  // end scripts
  $scripts[] = "/js/main.js{$v}";
  $scripts[] = "/traveler/js/slimmenu.js";
  $scripts[] = "/traveler/js/nicescroll.js";
  $scripts[] = "/traveler/js/dropit.js";
  $scripts[] = "/traveler/js/icheck.js";
  $scripts[] = "/traveler/js/fotorama.js";
  $scripts[] = "/traveler/js/typeahead.js";
  $scripts[] = "/traveler/js/card-payment.js";
  $scripts[] = "/traveler/js/owl-carousel.js";
  $scripts[] = "/traveler/js/fitvids.js";
  $scripts[] = "/traveler/js/tweet.js";
  $scripts[] = "/traveler/js/gridrotator.js";
  
  $scripts[] = "/afr/bower/magnific-popup/dist/jquery.magnific-popup.min.js";  
  $scripts[] = "/afr/bower/select-to-autocomplete/jquery.select-to-autocomplete.js";
  $scripts[] = "/afr/bower/bootstrap-validator/dist/validator.min.js";
  $scripts[] = "/afr/bower/bootstrap-confirmation2/bootstrap-confirmation.js";
  $scripts[] = "/afr/bower/bootstrap-toggle/js/bootstrap-toggle.min.js";
  $scripts[] = "/afr/bower/bootstrap-modal/js/bootstrap-modalmanager.js";
  $scripts[] = "/afr/bower/bootstrap-modal/js/bootstrap-modal.js";
  
  $scripts[] = "/afr/bower/ionrangeslider/js/ion.rangeSlider.min.js";
  $scripts[] = "/afr/bower/bootstrap-star-rating/js/star-rating.min.js";
  $scripts[] = "/afr/bower/select2/select2.min.js";    
  $scripts[] = "/afr/bower/jquery.countdown/dist/jquery.countdown.js";    
  $scripts[] = "/afr/bower/intl-tel-input/build/js/intlTelInput.min.js";    
  $scripts[] = "/afr/bower/jquery.parallax/jquery.parallax.js"; 
  $scripts[] = "/afr/bower/accounting.js/accounting.min.js"; 
  $scripts[] = "/afr/bower/character-countdown/build/characterCountdown.min.js"; 
  $scripts[] = "/afr/bower/addtocalendar/atc.min.js"; 
  $scripts[] = "/afr/bower/handlebars/handlebars.min.js";
  $scripts[] = "/afr/bower/jquery-truncate-html/jquery.truncate.js";
  $scripts[] = "/traveler/js/custom.js{$v}";   
  $scripts[] = "/afr/bower/bootstrap-table/src/extensions/filter-control/bootstrap-table-filter-control.js"; 

  if($app->user_level >= USER_LEVEL_ADMIN) {
    $styles[] = "/afr/bower/CodeMirror/lib/codemirror.css"; 
    $scripts[] = "/afr/bower/CodeMirror/lib/codemirror.js"; 
    $scripts[] = "/afr/bower/CodeMirror/mode/smarty/smarty.js"; 
  }
  if($siteObj->google_tag_id) $scripts[] = "/afr/js/lunametrics.js{$v}"; 
}



/** used; vpatina.com autogenerated menu from db */

// sitemap tree used for generating menus
function site_menu_tree($siteObj_or_id, $options=NULL) {
  global $u_level;
  $user_level = $u_level ?: 0;
  $siteObj = any2obj('site', $siteObj_or_id);

  $options = array();
  $options['skip_properties'] = true;
  $options['search'] = array('site_id' => $siteObj->parent_site_id ?: $siteObj->id, 'active' => 1, 'user_level' => array('<=', $user_level), 'show_menu' => 1);;
  return site_page_tree($siteObj_or_id, $options);
}


// return sitemap tree for properties
function site_sitemap_property_tree($siteObj_or_id, $options=NULL) {
  $siteObj = any2obj('site', $siteObj_or_id);
  if(!$siteObj) die("site_sitemap_property_tree: missing site:".dump($siteObj, true));
  $lang = $options['language'] ?: $_SESSION['language'];
  $search = property_filters($siteObj) ?: ['active' => 1];
  if(isset($options['search']) && is_array($options['search'])) $search = array_merge($search, $options['search']);
  $ptype = isset($options['type']) ? $options['type'] :  $siteObj->property_type;
  if(!$ptype) return [];
    
  $tree = [];
  $pageObj = site_find_pagetype($siteObj, property_type2page($ptype), $lang);
  $node = site_sitemap_node($siteObj, $pageObj, $lang);
  $node->collapse = true;
  
  $path = $pageObj->path;    

  $aptObjs = find_objects($ptype, $search, ["fields" => ['id', 'name', 'shortname', 'address_country', 'address_city'], "sort" => "address_country, address_city, name"]) ?: [];
  $aptNodes = [];
  foreach($aptObjs as $aptObj) {
    $aptNode = new stdClass;
    $aptNode->name = property_geoname($aptObj);
    $link = property_geolink($aptObj, ['path' => $path]) ;
    $aptNode->children = [];
    $aptNodes[$link] = $aptNode;
  }
  
  $node->children = $aptNodes;
  
  $tree[$path] = $node;
  return $tree;
}

function plugin_javascript($get) {
  global $__SITE, $__SITE_ID;
  $site_url = $_SERVER['HTTP_HOST'];
  $host_domains = explode(".", $site_url);
  $host_tld = array_shift($host_domains); // e.g. com
  $host_domain = implode('.', $host_domains);  
  $referer = $_SESSION['vp_referer'] = $_SERVER['HTTP_REFERER'];
  extract($get);
  
  $site_id = $__SITE_ID;
  $siteObj = $__SITE;
  $sharer_id = 0; // default - will show top frame if not set
  if(!$get) return("No input");
  
  //$share_id = isset($_GET['shid']) ? $_GET['shid'] : 0;
  //if($share_id) {
  //  $pluginObj = get_object('plugin');
  //  $sharer_id = $pluginObj->share_user_id ?: $pluginObj->user_id;
  //}
  
  // the plugin
  $plugin = plugin($get, array('viewer' => 'photoswipe', 'referer' => $referer));
  if(!$plugin) return '';
  $plugin = str_replace(array("\n", '"'), array(' ', '\"'), $plugin); // escape double quotes;
  
  $g_host_protocol = 'https';
  $g_site_url = "$g_host_protocol://".$site_url;
  $output = <<<EOF
// vPatina: referer=$referer

var d = new Date();
var ts = d.getTime();
var cache_bust = "?q=" + ts;
var g_list;
var g_cid = '';
var g_cids;
var g_keys;

var g_pswp = false;
var g_pswp_index = -1;
var g_site_url = '$g_site_url';
var g_aid = 10;
var g_site_id = '$site_id';
var g_user_id = '$u_id';
var g_sharer_id = '$sharer_id';
var g_host_protocol = '$g_host_protocol';
var g_host_tld = '$host_tld';
var g_host_domain = '$host_domain';
var g_client_protocol = document.location.protocol;
var g_follow = [];
var g_branch = '9d08da5';
var g_artists = {};
var g_json_data;
var g_local_file = '';
var g_local_data = '';
var g_plugin_options = '';
var g_popup_options = '';
var g_popup_buttons = '';
var g_popup_tabs = '';

// g_branch = 'master';
var production = false;
var git_sub = production ? 'cdn.' : '';

var domain = (g_client_protocol == 'https:' ? 'https://' : 'http://') + g_host_tld + '.' + g_host_domain;

var src1 = domain + '/afr/js/vp/vp.js' + cache_bust;
var src2 = "https://maps.googleapis.com/maps/api/js?key=AIzaSyDqzndmgSgkwaD6AEATZOkeEWUo4foizEo&libraries=places&language=en";

var style1 = domain + '/afr/js/vp/photoswipe.css?v=4.1.1-1.0.6';
var style2 = domain + '/afr/js/vp/default-skin/default-skin.css?v=4.1.1-1.0.7' + cache_bust;
var style3 = domain + '/afr/js/vp/vp.css' + cache_bust;

/**
if(production) {
  var src = 'https://' + git_sub + 'rawgit.com/kjakman/PhotoSwipe/' + g_branch + '/dist/vp.min.js' + cache_bust;
  var style = 'https://' + git_sub + 'rawgit.com/kjakman/PhotoSwipe/' + g_branch + '/dist/vp.min.css' + cache_bust;
} else { 
  var style1 = domain + '/afr/js/vp/photoswipe.css?v=4.1.1-1.0.5';
  var style2 = domain + '/afr/js/vp/default-skin/default-skin.css?v=4.1.1-1.0.5' + '&q=' + ts;
  var style3 = domain + '/afr/js/vp/vp.css' + cache_bust;  
}
*/

var g_afr_xss_loaded;
var _gaq = _gaq || [];
(function() {
  if(g_afr_xss_loaded) {
    console.log("xss already loaded"); 
    //vp_search_local();
    return false;
  } else {
  
    console.log("xss not loaded v3...");
    g_cids = {};
    g_json_data = {};
    g_list = {};
    g_keys = {};
    
    // add vp script
    var ga = document.createElement('script');
    ga.src = src1;
    ga.setAttribute('async', 'true');
    document.getElementsByTagName('head')[0].appendChild(ga);
    document.getElementsByTagName('body')[0].appendChild(ga);                    
    console.log("added vp.js v2 src=" + src1);
    
    // add google map
    var ga = document.createElement('script');
    ga.src = src2;
    ga.setAttribute('async', 'true');
    document.getElementsByTagName('head')[0].appendChild(ga);
    document.getElementsByTagName('body')[0].appendChild(ga);                    
    
    /** insert css */
    vp_requestStylesheet(style1)
    vp_requestStylesheet(style2)
    vp_requestStylesheet(style3)
    // vp_requestStylesheet("//maxcdn.bootstrapcdn.com/font-awesome/4.3.0/css/font-awesome.min.css");
    g_afr_xss_loaded = true;
    
    /** insert html - photoswipe container */
    var html = '' +
    '<div class="pswp" tabindex="-1" role="dialog" aria-hidden="true">' +
    '  <div class="pswp__bg"></div>' +
    '  <div class="pswp__scroll-wrap">' +
    '    <div class="pswp__container">' +
    '      <div class="pswp__item"></div>' +
    '      <div class="pswp__item"></div>' +
    '      <div class="pswp__item"></div>' +
    '    </div>' +
    '    <div class="pswp__ui pswp__ui--hidden">' +
    '      <div class="pswp__top-bar">' +
    '        <div class="pswp__counter"></div>' +
    '        <button class="pswp__button pswp__button--close" title="Close (Esc)"></button>' +
    '        <button class="pswp__button pswp__button--share" title="Share"></button>' +
    '        <button class="pswp__button pswp__button--like vp__favorite-link pswp__element--disabled" title="Like" id="likeBtn"></button>' +
    '        <button class="pswp__button pswp__button--user pswp__element--disabled" title="User" id="userBtn"></button>' +
    '        <button class="pswp__button pswp__button--chat pswp__element--disabled" title="Chat" id="chatBtn"></button>' +
    '        <button class="pswp__button pswp__button--contact pswp__element--disabled" title="Contact" id="contactBtn"></button>' +
    '        <button class="pswp__button pswp__button--calendar pswp__element--disabled" title="Calendar" id="calendarBtn"></button>' +
    '        <button class="pswp__button pswp__button--map" title="Map" id="mapBtn"></button>' +
    '        <button class="pswp__button pswp__button--zoom" title="Zoom in/out"></button>' +
    '        <button class="pswp__button pswp__button--info" title="Info" id="vp__infoBtn"></button>' +
    '        <button class="pswp__button pswp__button--fs" title="Toggle fullscreen"></button>' +
    '        <div class="pswp__preloader">' +
    '          <div class="pswp__preloader__icn">' +
    '            <div class="pswp__preloader__cut">' +
    '            <div class="pswp__preloader__donut"></div>' +
    '            </div>' +
    '          </div>' +
    '        </div>' +
    '      </div>' +
    '      <div class="pswp__share-modal pswp__share-modal--hidden pswp__single-tap" id="vp__share-modal">' +
    '        <div id="vp__share-modal-content" class="vp__share-modal-content arrow-box"></div> ' +
    '      </div>    ' +
    '      <button class="pswp__button pswp__button--arrow--left" title="Previous (arrow left)"></button>    ' +
    '      <button class="pswp__button pswp__button--arrow--right" title="Next (arrow right)"></button>    ' +
    '      <div class="pswp__caption">' +
    '        <div class="pswp__caption__center"></div>' +
    '      </div>    ' +
    '    </div>    ' +
    '  </div>    ' +
    '</div>' +
    '<div id="vp__my_modal" class="w3-modal my-modal-off" style="position:fixed;width:100%;height:100%;left:0;top:0;background:rgba(0,0,0,0.6);display:none">' +
    '  <div class="w3-modal-content vp__my-modal-content w3-animate-opacity" style="background-color: #fff;color:#000">' +
    '    <span onclick="document.getElementById(\'vp__my_modal\').style.display=\'none\'" style="padding-top: 0px !important;padding-bottom: 6px !important" class="w3-button w3-xlarge w3-display-topright">&times;</span>' +
    '    <div id="vp__my-modal-content" class="w3-container-off">' +
    '    </div>' +
    '  </div>' +
    '</div>';
   

    vp_appendHtml1(document.body, html);

    // '<div id="myModal" class="vp__modal" style="display:none"><div class="vp__modal-header"><span class="vp__close" id="closeBtn">×</span></div><div class="vp__modal-content" id="vp__modal-content"></div></div>' +
    //var modal = '<div id="myModal" class="vp__modal" style="display:none"><div class="vp__modal-header"><span class="vp__close" id="closeBtn">×</span></div><div class="vp__modal-content" id="vp__modal-content"></div></div>';
    //vp_appendHtml1(document.body, modal);
 
  }
})();
document.write("$plugin");

console.log('wrote output...');
vp_search_local();

function vp_search_local() {
  if(typeof vp_search === "function") {
    vp_search();                                                                     
  } else {
    console.log('vp_search not found');
  }
}


/** Begin runtime */
document.addEventListener("DOMContentLoaded", function(event) { 
 console.log("Local DOM Loaded");   
 vp_search_local();
});

function vp_requestScript(script_url) {
  var script = document.createElement('script');
  script.setAttribute('src', script_url);
  script.setAttribute('type', 'text/javascript');
  document.getElementsByTagName('head')[0].appendChild(script);
}
function vp_requestStylesheet(stylesheet_url) {
  stylesheet = document.createElement("link");
  stylesheet.rel = "stylesheet";
  stylesheet.type = "text/css";
  stylesheet.href = stylesheet_url;
  stylesheet.media = "all";
  document.getElementsByTagName("head")[0].appendChild(stylesheet);
}
function vp_appendHtml1(el, str) {
  var div = document.createElement('div');
  div.innerHTML = str;
  while (div.children.length > 0) {
    el.appendChild(div.children[0]);
  }
}
EOF;
  return $output;  
}

// legacy
function sharing_output($input, $options = array()) {
  return plugin($input, $options);
}
              
function plugin($input, $options = array()) {
  global $__SITE;
  
  $preview = $options['preview'];
  if($referer = $options['referer']) $_SESSION['vp_referer'] = $referer;

  $containerOptions = array();


  $url = $debug = '';
  
  $plugin_id = $input['shid'];
  $index = $input['index'];
  $plugin_id = 0;
  $style = '';
  
  if($plugin_id = $input['shid']) {
    $pluginObj = get_object('plugin', $plugin_id);
    if(!$pluginObj) return "Plugin ($plugin_id) not found";    
    $sharer_id = $pluginObj->share_user_id ?: $pluginObj->user_id;
    $siteObj = $pluginObj->site_id == $__SITE->id ? $__SITE : get_object('site', $pluginObj->site_id);
    $site_id = $siteObj->id;
    $site_url = site_full_url($siteObj);
    
    extract(obj2array($pluginObj));
    $pluginOptions = $pluginObj->plugin_options ? json_decode($pluginObj->plugin_options) : null;

    if(!$preview) db_query("UPDATE sharing SET count=count+1 WHERE id=$plugin_id");
    if($data_json = $pluginObj->data) {
      $data = json_decode($data_json, true);
      $product_id = $data['product_id'];
    }
    
    $container_id = '__vp_plugin-'.$plugin_id;

    // overrides from URL
    $title = $input['title'] ?: $pluginObj->title;
    $subtitle = $input['subtitle'] ?: $pluginObj->subtitle;
    $size = $input['size'] ?: $pluginObj->image_size;

    print_log("shid={$plugin_id} title={$title} sub={$subtitle} size={$size}", 'sharing', LOG_LEVEL_TEST);
    
  } else { /** not in use */
    extract($input); // assigns keys from input into local variables with same names
    $parent_id = pick_first($parent_id, $obj_id, $id);
    $parent_type = pick_first($parent_type, $type, $obj_type);
    $container_id = '__vp_plugin-'.$parent_type.'_'.$parent_id;
  }
  
  $client_id = $siteObj->client_id;  
  $style = $pluginObj->style;
  
  if(!$site_id) return "Sharing: missing site_id";
  if(!$parent_id) return "Sharing: missing obj_id";
  if(!$parent_type) return "Sharing: missing obj_type";
  if(!$type) $type = 'image';
  

  if($class) $containerOptions['class'] .= " $class";
    
  
  $containerOptions['viewer'] = $viewer;
  $containerOptions['format'] = 'json';
  $containerOptions['size'] = $size;
  $containerOptions['full'] = true;

  $search = $data ?: array();
  $search['obj_type'] = $parent_type;
  $search['obj_id'] = $parent_id;
  $search['plugin_id'] = $plugin_id;
  $search['container_id'] = $container_id;      
  $searchData = json_encode($search);
  $containerOptions['data-data'] = $searchData;

  if($url) {
    $containerOptions['data-url'] = $url;
    $link = 'javascript:void(0);';
  }
  $title_class = "afr_sharing_title";

  $containerOptions = ['id' => $container_id, 'data-data' => $searchData];
  if($style) $containerOptions['style'] = $style;
  if($url) $containerOptions['data-url'] = $url;
  
  
  if($preview) $containerOptions['js'] = "onclick='alert(\"Viewer preview: Coming soon!\");return false;'";
  
  $containerOptions['data-options'] =  $pluginObj->plugin_options;
  $containerOptions['data-sharer_id'] =  $sharer_id;
  $containerOptions['data-popup_options'] = $pluginObj->popup_options;
  
  $mapObjs = find_objects('media_collection_map', ['media_collection_id' => $pluginObj->parent_id], ['sort' => 'rank']);
  if(!$mapObjs) return "No media for {$parent_type} {$parent_id}";
  
  $object_tag = $pluginObj->object_tag ?: 'figure';

  $viewer = $options['viewer'] ?: 'photoswipe';     
  $pluginClass = "__vp-plugin-container vp-loader-{$viewer}";
  if($cc = $pluginObj->object_class) $pluginClass .= " $cc";

  $imageObj = null;
  switch($type) {
  case 'image':
    $title_image = $pluginObj->title_image;
    if($title_image) {
      if(isint($title_image)) {
        $imageObj = get_object('media', $title_image);
        $images = array_find_key($mapObjs, 'media_id');
        if(in_array($title_image, $images)) $containerOptions['data-index'] = $index = array_search($title_image, $images);
      }
    } else {
      $mapObj = $mapObjs[0];
      $title_image = $mapObj->media_id; // use first
    }
    
    $imageOptions = [];
    $imageOptions['class'] = $class = "__vp-plugin-image center-cropped";
        
    $size = $pluginObj->size ?: 'big';
    $src = print_media($title_image, ['size' => $size, 'source_only' => 1, 'full' => 1]);
    
    //$image = html_image($src, $title, $containerOptions); 
    $image = html_image($src, $title, $imageOptions); 

    /** info printed under plugin. Data from title image, not collection */
    $image_info = '';    
    if($pluginOptions && $imageObj) {
      $artist_id = $imageObj->artist_id;
      $artist_name = user_display_name($artist_id);
      /** todo, actually parsen pluginOptions */
      $image_title = $artist_name;
      $image_subtitle = "{$imageObj->name}";
      if($imageObj->original_year > 0) $image_subtitle .= " ({$imageObj->original_year})";
      if($image_title || $image_subtitle) {
        $image_info = "
        <div class=\"__vp-img-info __vp-photo-grid-title\">
          <h3>{$image_title}</h3>
          <h4>{$image_subtitle}</h4>
        </div>               
        ";
      }
    }
    
    $hover_effect = $pluginObj->hover_effect ?: 'zoom';
    $spinner = "<div style=\"width:20px;height:20px;display:inline-block\"><img src=\"{$site_url}/images/spinner.gif\"></div>";
    switch($hover_effect) {
      case 'flip':
        $back_image = html_image($src, $title, ['class' => '__vp-flip-back-image']); 
        $plugin = "
        <div class=\"__vp-flip-container\" ontouchstart=\"this.classList.toggle('hover');\">
          <div class=\"__vp-flipper\">
            <div class=\"__vp-flip-front\">
              {$image}
            </div>
            <div class=\"__vp-flip-back __vp-plugin-hover\">
              {$back_image}
              <div class=\"__vp-flip-back-card\">
                <h2>{$title}</h2><br>
                <div>{$subtitle}</div><br>
                
                <a href=\"#\" class=\"__vp-info __vp-view-button\" id=\"__vp-view-button-{$plugin_id}\">{$spinner} Loading...</a>
              </div>                                                   
            </div>
          </div>
        </div>
        ";    
      break;
      case 'none':
        $plugin = "
        <div class=\"__vp-plugin\">
          <div class=\"__vp-plugin-image\">
            {$image}
          </div>
          <div class=\"__vp-plugin-title\">
            <h2>{$title}</h2><br>
            <div>{$subtitle}</div><br>
          </div>
        </div>
        ";    
        break;
      
      case 'zoom':
      default:
        $pluginClass .= " view view-tenth";        
        $plugin = "
        {$image}
        <div class=\"mask __vp-plugin-hover\">
          <h2>{$title}</h2><br>
          <div>{$subtitle}</div><br>
          <a href=\"#\" class=\"__vp-info __vp-view-button\" id=\"__vp-view-button-{$plugin_id}\">{$spinner} Loading...</a>
        </div>
        ";
      break;
    }
        
    break;
  case 'link':
    $plugin = html_link($link, $title, ["class" => "__vp-plugin-link"]);
    break;
  case 'button':
    $plugin = html_link($link, $title, ["class" => "__vp-plugin-button"]);
    break;
  default:
    break;
  }    
  
  $output = html_tag($object_tag, $plugin, $pluginClass, $containerOptions);
  $output .= $image_info;
  
  // wrap tag in container tag
  if($tag = $pluginObj->container_tag) {
    $class_str = $pluginObj->container_class ? " class=\"{$pluginObj->container_class}\"" : '';
    $output = "<$tag{$class_str}>{$output}</$tag>";
  }

  $debug = '';
  $log = "id={$pluginObj->id} collection: {$parent_id}<br>container_tag=$tag object_tag={$object_tag}<br>title: {$title_image} index=$index<br>type: {$type} hover:{$hover_effect}";
  print_log($log, 'sharing', LOG_LEVEL_TEST);         
  //if($_SESSION['dev']) $debug = $log;

  if($style) {
    $output .= ''; /** todo: style for this #id */
  }
  //$debug = dump($pluginOptions, true);
  return $debug.$output;  
}
    
/** called from class_handlers 1028 */
function site_directory($siteObj_or_id) {
  $siteObj = any2obj('site', $siteObj_or_id);
  return $_SESSION['live'] ? $siteObj->directory : $siteObj->test_directory;
}

function scope_error($app, $error, $code=0) {
  $scope = $app->page->scope;
  $scope->error = $error;
  if($code) {
    $scope->hide_newsletter = 1;
    $app->page->template = $code;    
  }
  
  return;
}

/** used to parse GRAV CMS style minisite html files with --- delimitered header */
function site_parse_pagefile($filename) {
  $custom_file = read_file($filename);
  if(!$custom_file) return [[], html_alert("{$filename} not found...", 'error')];
  if(strpos($custom_file, '---') === false) return [[], $custom_file];
  $sections = explode('---', $custom_file);
  if(count($sections) < 3) return [[], $custom_file];
  $header = site_parse_pageheader($sections[1]);
  return [$header, $sections[2]];
}

/** used to parse the header of GRAV CMS style minisite html files */
function site_parse_pageheader($string) {
  $delim = "\r\n";
  $header = [];
  $tok = strtok($string, $delim);  
  while ($tok !== false) {
    if(strlen(trim($tok))) list($var,$val) = explode(':', trim($tok));
    if(trim($var) && strlen(trim($val))) $header[trim($var)] = trim($val);
    $tok = strtok($delim);
  }  
  strtok('', ''); // frees memory
  return $header;
}
  
function page_error($error = '', $status = 404) {
  $status_name = http_status_name($status) ?: "Not Found";
  
  $error = "Page not found".($error ? ": {$error}" : ".");  
  $pageObj = new stdClass;
  $pageObj->http_code = "{$status}";
  $pageObj->http_status = "Status: {$status} {$status_name}";
  $pageObj->http_header = "HTTP/1.1 {$status} {$status_name}";  
  $pageObj->http_status_name = $status_name;
  $pageObj->http_error = $error;
  $pageObj->scope = new stdClass;
  $pageObj->scope->error = $error;
  $pageObj->controller = '';
  $pageObj->template = '404';
  return $pageObj;
}
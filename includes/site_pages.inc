<?php

function site_page_google_calendar($siteObj, $pageObj, $options=NULL) {
  die('google calendar');
  
  $google_username = rawurlencode($options['google_username'] ?: $siteObj->google_username);
  if(!$google_username) return 'No google user id'.caller_stack();

  // options, defaults
  $height   = $options['height'] ?: 600; 
  $width    = pick_first(array($options['width'], $_SESSION['inner_width'], 800)); 
  $title    = rawurlencode($options['title'] ?: phrase('agenda', CAPITALIZE)); 
  $color    = $options['color'] ?: '060D5E'; 
  $bg_color = $options['background-color'] ?: 'FFFFFF'; 
  $time_zone = rawurlencode($options['time_zone'] ?: 'Europe%2FAmsterdam'); // todo: use site settings 
  $border_width = $options['border-width'] ?: 0; 
  $height = $options['height'] ?: 600; 
  $height = $options['height'] ?: 600; 

  return "<iframe class='grey-filter' src='https://www.google.com/calendar/embed?title=$title&amp;height=$height&amp;wkst=2&amp;bgcolor=%23$bg_color&amp;src=$google_username&amp;color=%23$color&amp;ctz=$time_zone' style=' border-width:$border_width' width='$width' height='$height' frameborder='0' scrolling='no'></iframe>;";
}
function site_page_edit_site($siteObj, $pageObj, $options=NULL) {
  global $u;
  global $u_id;
  global $__header;
  $__header['scripts'][] = '/afr/bower/ckeditor/ckeditor.js';
  $__header['scripts'][] = '/afr/bower/ckeditor/adapters/jquery.js';
  $__header['scripts'][] = '/afr/ckfinder/ckfinder.js';
  $__header['scripts'][] = '/afr/bower/summernote/dist/summernote.min.js';

  $__header['scripts'][] = '/afr/js/afr_cms.js';
  $__header['scripts'][] = '/afr/js/afr_suggest.js';
  $__header['scripts'][] = '/afr/bower/jquery-validate/dist/jquery.validate.min.js';
  $__header['scripts'][] = '/afr/js/custom_validators.js';

  $__header['scripts'][] = '/jquery/colorpicker/js/cp.min.js';
  $__header['scripts'][] = '/jquery/file-uploader-3.3/client/js/client.min.js';

  $__header['scripts'][] = '/afr/bower/dynatree/skin/jquery.dynatree.min.js';     
  if($_SESSION['lang'] != 'en') $__header['scripts'][] = '/afr/bower/jquery-validate/localization/messages_'.$_SESSION['lang'].'.js';
  //$__header['css'][] = '/jquery/file-uploader-3.3/client/fineuploader.css';
  $__header['css'][] = '/afr/bower/dynatree/dist/skin/ui.dynatree.css';
  
  $__header['css'][] = "/jquery/colorpicker/css/layout.css";     
  $__header['css'][] = "/jquery/colorpicker/css/colorpicker.css";
  $__header['css'][] = '/afr/bower/summernote/dist/summernote.css';
  
  if(!$u) die ("Not logged in");
  $ns1 = NAMESERVER_1;
  $ns2 = NAMESERVER_2;
  
  $inputVars = $_GET ? $_GET : $_POST;
  //dump($inputVars);
  
  $ref_name = urldecode($inputVars['ref_name']);
  $ref_link = urldecode($inputVars['ref_link']);
  $back_link = $ref_link ? html_link($ref_link, "Back to $ref_name", array('full' => true, 'class' => 'arrow_link')) : "";
  $header = html_h2("Edit site $siteObj->domain");
  
  $obj_type = $inputVars['obj_type'] ?: 'site';
  if($u_id == SYSTEM_USER_ID) {
    $id = pick_first($_SESSION['path_var'], $inputVars['id'], $siteObj->id); // edit site other than this
    if($id && $id != $siteObj->id) {
      $siteObj = get_object('site', $id);      
      $header = html_h2("Edit site $siteObj->domain");
      if($site_edit_link = site_page_link($siteObj, PAGE_EDIT_SITE, array('full' => true))) {
        $exists = url_exists($site_edit_link);
        if($exists) $header .= " Or use: ".html_link($site_edit_link, $site_edit_link);
        //if(url_exists($site_edit_link)) redirect($site_edit_link);
      }
    }
    
  } else {
    $id = $siteObj->id;
  }

  //echo("u_id = $u_id id=$id");
  $site_id = $inputVars['site_id'] ?: $id;
  $save_close = $inputVars['save_close'];
  $save = $inputVars['save'] || $save_close;
  $sel_section = $inputVars['section'];
  $action = 'edit';  

  $domain = $inputVars['domain'];
  
  $class_def  = get_class_def($obj_type, true);

  //echo("id=$id site_id = $site_id");  
  $siteObj = $site_id ? get_object('site', $site_id) : NULL;
  
  //dump($siteObj);
  
  $obj_name = get_object_name($obj_type);

  if($site_id && !$siteObj) {
    die("$obj_name $id does not exist");
  }

  // security
  //if(!auth_action($obj_type, $action, $id)) die("$action $obj_type $id not auth.");

  $sections = array('basic' => array('icon' => 'pricing'), 
                    'settings' => array('icon' => 'address'), 
                    'address' => array('icon' => 'address'), 
                    'content' => array('icon' => 'content'), 
                    'language' => array('icon' => 'address'), 
                    'dimensions' => array('icon' => 'address'), 
                    'scheme' => array('icon' => 'color'), 
                    'colors' => array('icon' => 'color'), 
                    'layout' => array('icon' => 'address'), 
                    'social' => array('icon' => 'social'),
                    'config' => array('icon' => 'address'),
                    'property' => array('icon' => 'address'));
  //if($siteObj->show_types) $sections['property'] = array('icon' => 'property');
  if($siteObj->show_types || $siteObj->gallery_site) $sections['galllery'] = array('icon' => 'gallery');
  $sections['pages'] = array('icon' => 'page', 'onload' => 'dynatree_init2');
  
  $section_keys = array_keys($sections);
  $sel_section = $inputVars['section'] ?: $section_keys[0];
  
  //$header = site_print($siteObj, $site_options);  
  $main   = edit_site_section($siteObj, $sel_section, $inputVars);
  
  $target_id = 'edit-site-pane';
  $edit_url = site_page_link($siteObj, PAGE_EDIT_SITE);
  //$base_url = $edit_url."$id/";                        
  $base_url = $edit_url;                        

  $title = $foo ? html_link($edit_url, html_span('', 'icon-24 arrow-180').' '.phrase('view_all', CAPITALIZE)) : '';
  $menu = html_vmenu($sections, array('title' => $title, 'selected' => $sel_section, 'base_url' => $base_url, 'target' => $target_id, 'handler' => 'edit-site-section', 'data' => '{"type": "'.$obj_type.'", "id" : "'.$site_id.'"}'));
  
  $output .= html_div($header, 'pad tbg2 row', "id='edit-site-header'");
  $output .= html_div($menu, 'col one-quarter', "id='edit-site-menu'");
  $output .= html_div($main, 'pad col three-quarters', "id='$target_id'");
  $output .= html_div('', 'content-row', "id='edit-site-footer'");

  // adds global JS variables needed by gallery edit
  //$gallery_options = site_gallery_options($siteObj, $aptObj);
  //$output .= html_script_inline(gallery_options_js($gallery_options));
  if($messages) {$output .= print_messages($messages); $messages = array();}
  if($errors)   {$output .= print_errors($errors); $errors = array();}

  return html_div($output, 'border-light round', "id='edit-site-container'");
}

/** not in use */



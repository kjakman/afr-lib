<?php
  //header('Content-type:application/json;charset=utf-8');
  include_once('set_env.inc');
  $timer['set_env'] = microtime(true);
  //require_once('api.inc');

  $app = $__APP;
  $timer=[];
  $timer['start'] = microtime(true);
  $result = new stdClass;
  $response = new stdClass;
  //echo("App Lang1: {$_SESSION['language']}<br>");

  //echo("App Lang1: {$__LANG}<br>");

 
  if($_REQUEST['type'] == 'artwork') {
      $obj_type= $_REQUEST['type'];
      $obj_id= $_REQUEST['id'];
      $obj_col= $_REQUEST['col'];
      $data= $_REQUEST['data'];
      $obj_data= array('obj_type'=>$obj_type,'obj_id'=>$obj_id,$obj_col=>$data);
      $result= update_object($obj_type, $obj_data, $obj_id);
      die("success");
  }
  
  if($_REQUEST['type'] == 'media_collection') {
      $obj_type= $_REQUEST['type'];
      $obj_id= $_REQUEST['id'];
      $obj_col= $_REQUEST['col'];
      $obj_col2= $_REQUEST['col2'];
      $date = $_REQUEST['date'];
      $datas= $_REQUEST['data'];
      if($date)
      {
        $data = date("Y-m-d", strtotime($date));
      }
      else
      {
        $data = $datas;
      }
      //dump($data);exit;
      //echo $_REQUEST['date'];die;
      if($obj_col == 'start_date')
      {
        $obj_data= array('obj_type'=>$obj_type,'obj_id'=>$obj_id,$obj_col=>$data,$obj_col2=>$data);  
      }
      else
      {
        $obj_data= array('obj_type'=>$obj_type,'obj_id'=>$obj_id,$obj_col=>$data);  
      }
      
      $result= update_object($obj_type, $obj_data, $obj_id);
      die("success");
  }

  //if($_SESSION['dev'] && $_GET['debug'] == 'cre8ive') $inputVars = $_GET; // allow using GET by setting debug=cre8ive
  //else $inputVars = $_GET;
  //else $inputVars = $_POST;  
  $oper       = $_GET['oper'] ?: $_POST['oper'];  // operation - same thing as action
  $action     = $_GET['ac'] ?: $_POST['ac'];      // action
  unset($_GET['oper']);
  unset($_GET['ac']);
    
  $inputVars = $_GET ?: $_POST; // prioritize $_GET for control, $_POST for data (below)
  $obj_type = $_POST['obj_type'] ?: $_GET['obj_type'];
  $obj_name = get_object_name($obj_type);

  if(isset($inputVars['__sl'])) {
    unset($_GET['__sl']);
    $lang = $inputVars['__sl'];
    if($lang != $_SESSION['lang']) $__LANG = $__LANGUAGE = $_SESSION['language'] = $lang;
  }
  $log_file   = $log_dir.'ajax.txt';

  $now = now();
  $ip = client_ip();
  print_log("{$now} from {$ip} "."oper=$oper", 'ajax', LOG_LEVEL_TEST);

  //die($log_file);
  $oper = $oper ?: $action;
  $id = ($oper && $oper != 'add') ? ($_POST['id'] ?: $_GET['id']) : 0;  
  
  if($oper == 'save') $oper = $id ? 'edit' : 'add'; // save can be used for add or edit
  if($oper == 'search') $oper = 'list'; // alias  
  if($oper == 'delete') $oper = 'del'; // alias 
  $success = true;
  $messages = array();
  $errors = array();

  //if($obj_type) $filters = get_object_filters($obj_type, $u_level, $u_id); // get forced filters from security model

  $allow_jsonp = array('get-gallery', 'gallery-favorite', 'get-data', 'user-login');
  $jsonp = in_array($oper,$allow_jsonp);
  
  $security = true;
  if($security) {
    if($jsonp) {
      // OK, allow access....
    } else if($_SERVER['SERVER_ADDR'] != $_SERVER['REMOTE_ADDR']) {
      //die("gh1 s=".$_SERVER['SERVER_ADDR']." r=".$_SERVER['REMOTE_ADDR']);
    } else if(isset( $_SERVER['HTTP_X_REQUESTED_WITH'] ) && ( $_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest' ) ) {
      // allow access....
    } else {                                                                                 
      die("gh2"); // log security violation
    }
  }
  
  // catch background upload
  if($qqfile = $_GET['qqfile'] ?: $_FILES['qqfile']) {
    $oper = 'upload';
    $qq_name = is_array($qqfile) ? $qqfile['name'] : $qqfile;
  }

  // todo: more security
  // basic hack to allow non-logged in user for only select objects actions
  print_log("u=$u_id oper=$oper obj_type=$obj_type id=$id\n", 'ajax', LOG_LEVEL_ALWAYS); // "\n------------\n\n"
  $u = $__USER;
  $u_id = $__USER->id;
  
  if($u) { // logged in user
    $db_ops = array('add', 'del', 'edit', 'list', 'view');
    $db_op = in_array($oper, $db_ops);
    $oper_auth = $oper == 'new_object_dialog' ? 'add' : $oper;
    
    if($db_op) {
      if(!$obj_type) json_error("$oper object: missing object type");
      if($oper != 'add' && $oper != 'list' && !$id) json_error("$oper $obj_type: missing id");
      if($obj_type && $db_op && !auth_action($obj_type, ($oper_auth ? $oper_auth : 'list'), $id)) {
        // dump($inputVars);        
        json_error("Not authorized $oper $obj_type $id");
      }
    }


  } else { // public: only allow certain types and operations
    $no_security = array('add-property', 'rental-availability', 'send-review',
      'mollie-pay', 'check-transaction', 'save-booking', 'send-mail', 'mail-preview', 'mail-preview-send',
      'share', 'helpful', 'fetch-template', 'jq-get-media', 'contact-form', 'location-search', 
      'property-searchbar', 'property-search', 'property-print', 'golf-fetch', 'golf-print', 
      'explore-gallery', 'load-function-json', 'upload', 'cart-add', 'cart-remove', 
      'cart-row-update', 'new-lead', 'check-payment', 'get-payment','process-payment',
      'get-gallery', 'get-data', 'user-login','provider-login', 'user-register', 
      'signup-form', 'load-more', 'request-form', 'package-form',  
      'rental-calendar-events', 'rental-calendar-availability', 'calendar-availability', 'agenda-calendar-events', 'reservation-request', 'property-slider', 'cal-picker', 
      'property-calendar', 'add-sharing', 'preview-sharing', 'price-breakdown', 
      'load-page-content', 'reset-password', 'edit-password', 'register-newsletter', 'interhome-price', 'test',
      'est-mapping', 'est-mapping-helper'
      );
    
    if(!in_array($oper, $no_security)) { // no security required for these
      $allowed_ops = array('add','new_object_dialog', 'list', 'view');
      $allowed_types = array('org', 'agent', 'message', 'comment', 'news', 'links', 'sharing');        
      if(!in_array($oper, $allowed_ops)) {json_error("Illegal operation $oper");}
      if(!in_array($obj_type, $allowed_types)) {json_error("Illegal type $obj_type");}
    }
  }
                                          
  if(!$oper) {
    echo("missing operand");
    json_error("missing operand");
    exit;
  }
  
  unset($_POST['id']);
  unset($_POST['oper']);
  if($_POST) {
    $obj_data = $_POST;
    if($_GET) {
      foreach($_GET as $k => $v) { // allow sending data with GET as well
        if(in_array($k, array('oper', 'obj_type'))) continue;
        $obj_data[$k] = $v;
      }
    }
  } else {
    $obj_data = $_GET;
  }

  $options =  $_GET['options'] ? json_decode(htmlspecialchars_decode($_GET['options']), true) : array();
  $errors = $messages = array();

  print_log("data:".dump($obj_data,true), 'ajax', LOG_LEVEL_TEST);
  
  $timer['switch'] = microtime(true);
  switch($oper) {
    
    /** Elastic Tools */
    case 'est-mapping-helper':
      
      if($error = check_required($obj_data, array('mapping' => "Input"))) json_error("Missing: ".implode(', ', $error));
      $lines = explode(PHP_EOL, $obj_data['mapping']);      
      foreach($lines as $i => $line) {
        if($line[0] == '.') unset($lines[$i]);
      }
      $last = array_pop($lines);
      $output = "GET ".implode(PHP_EOL."GET ", $lines).$last_line;
      $result->success = 1;
      $result->data = ['dsl' => $output];
      
      $time_str = elapsed($timer['start']);
      
      $result->message = "Good. Now copy the content below and paste in into Kibana.<br>Then copy the Kibana output and paste it into the mapping tool.";
      
      echo(json_encode($result));exit;      
      break;
      
      
      
      
    case 'est-mapping':
      
      if($error = check_required($obj_data, array('mapping' => "Mapping"))) json_error("Missing: ".implode(', ', $error));
      $index_dsl = $obj_data['mapping'];      

      require("classes/ElasticTools.php");
      $est = new EST\ElasticTools(); //create the controller and execute the action
      
      try {      
        $dsl = $est->convertIndices($index_dsl);
      } 
      catch(Exception $e) {
        json_error($e->getMessage());
      }

      $input_len = strlen($index_dsl);
      $input_len_h = bytes2human($input_len);
    
      $output_len = strlen($dsl);
      $output_len_h = bytes2human($output_len);
      
      //$result->obj = $dslArray;
      $result->success = 1;
      $result->data = ['dsl' => $dsl];
      
      $time_str = elapsed($timer['start']);
      
      $result->message = "OK. Read {$input_len_h} and wrote {$output_len_h} in {$time_str}s";
      
      echo(json_encode($result));exit;      
      break;
    /** End Elastic Tools */
    
    case 'interhome-price':
      $in = date2sql($_GET['in']);
      $out = date2sql($_GET['out']);
      $fid = $_GET['fid'] ;
      $price = $fid && $in && $out && $out > $in ? interhome_pricing($fid, $in, $out) : "??";
      $nd = period_diff($in, $out, 'd');
      echo(round($price));exit;
      echo("p=$price nd=$nd");exit;
      echo($nd > 0 ? round($price/$nd) : '');exit; 
      break;
      
    case 'location-search':
      if($error = check_required($obj_data, array('loc_types' => "Types", 'bounds' => "Bounds"))) json_error("Missing: ".implode(', ', $error));
  
      $locs = array();
      $loc_count = 0;    
      $loc_limit = 30;
      
      extract($obj_data);
      
      print_log("location-search: bounds=".dump($bounds,true), 'property-search', LOG_LEVEL_TEST);

      $boundsObj = bounds_json2obj($bounds);
      $search = bounds2search($boundsObj);
      
      print_log("location-search: boundsObj=".dump($boundsObj,true), 'property-search', LOG_LEVEL_TEST);
      print_log("location-search: search=".dump($search,true), 'property-search', LOG_LEVEL_TEST);
      $lat = $search['location_lat'];
      $lng = $search['location_long'];
      
      if(is_array($lat) && is_array($lng) && $loc_types) { // search is geographically restricted, add other location types
        
        $locSearchArray = array('active' => 1, 'location_lat' => $lat, 'location_long' => $lng);  
        $locOptions = array("limit" => $loc_limit, "sort" => "priority DESC");        
        $locSearchArray['parent_type'] = explode(',', $loc_types);

        $locObjs = find_objects('location', $locSearchArray, $locOptions);
        
        //if() {
        //  $locs = objs2locs($obj_type, $locObjs);
        //}
      }
      
      $result->locs = $locObjs;
      $result->count = count($locObjs);
      $result->success = 1;
      echo(json_encode($result));exit;      
      break;

    case 'save-gallery-settings':
      $obj_type = 'user_profile';
      extract($obj_data);
      if(!$obj_id) json_error("Missing ID");
      if(!$settings) json_error("Missing settings");
      $settings = json_decode($settings);
      if(!$settings) json_error("Invalid settings");
      
      $data = ['data' => json_encode(['gallery' => $settings])]; 
      list($id, $errors) = update_object($obj_type, $data, $obj_id);
      if($errors) json_error($errors);
      $result->success = 1;
      $result->message = "OK. Saved settings.";      
      echo(json_encode($result));exit;      
      break;

    case 'clone-artwork':
      extract($obj_data);
      if(!$obj_id) json_error("Missing artwork ID");
      $title = trim($title);
      if(!$title) json_error("Please enter a title");
      
      print_log("clone-artwork: type: {$obj_type} id: {$obj_id} data:".dump($obj_data,true), 'gallery', LOG_LEVEL_TEST);
      
      $artworkObj = get_object('artwork', $obj_id);
      if(!$artworkObj) json_error("Artwork {$obj_id} not found");
      
      $data = (array) $artworkObj;
      $data['title'] = $title;
      $unset = ['id', 'created', 'modified','title_image','title_image_json','related_media_json'];
      foreach($unset as $fld) unset($data[$fld]);
      list($id, $errors) = add_object('artwork', $data);
      if($errors) json_error($errors);
      
      $result->success = 1;
      $result->id = $id;
      $result->data = get_object('artwork', $id);
      $result->message = "OK. Cloned artwork \"{$artworkObj->title}\"";
      echo(json_encode($result));
      exit;      
      break;
    
    case 'save-artwork':
      extract($obj_data);

      print_log("save-artwork: type: {$obj_type} id: {$obj_id} data:".dump($obj_data,true), 'gallery', LOG_LEVEL_TEST);
      //if(isset($obj_data['type']) && !$type) json_error("Please select a category."); 
      //if(isset($obj_data['status']) && !$status) json_error("Please select a status."); 
      if(isset($obj_data['artist_ids']) && !count(json_decode($obj_data['artist_ids']))) json_error("Please select at least one artist."); 
      //echo "<pre>"; print_r($id);
      /**
      if($error = check_required($obj_data, 
          ['title' => "Title", 'primary_year' => "Primary Year", 'technique' => "Technique"])) 
          json_error("Missing: ".implode(', ', $error));
      */    
      if($obj_data['primary_year'] == "") $obj_data['primary_year'] = "";
      //dump($obj_data);exit;
      if($obj_id) {
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
        print_log("save-artwork: updated: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'gallery', LOG_LEVEL_TEST);
      } else {
        list($id, $errors) = add_object($obj_type, $obj_data);
        print_log("save-artwork: added {$obj_type} id: {$obj_id} errors".dump($errors,true), 'gallery', LOG_LEVEL_TEST);
      }              
          
      if($errors) json_error($errors);
      
      $obj = get_object($obj_type, $id);
      $obj->next_tab = $obj_data['next_tab'];
      
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved artwork \"{$obj->title}\"";
      
      echo(json_encode($result));
      exit;      
      break;

    case 'save-artwork-related':
      extract($obj_data);

      if(!$obj_data[dedit])
        if($error = check_required($obj_data, 
            ['type' => "Type", 'description' => "description"])) 
            json_error("Missing: ".implode(', ', $error));

      if($obj_id) {
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
      }else{
        list($id, $errors) = add_object($obj_type, $obj_data);
      }            
      if($errors) json_error($errors);
      
      $obj = get_object($obj_type, $id);
      
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved related artwork";
      
      echo(json_encode($result));
      exit;      
      break;

    case 'save-media-related':
      extract($obj_data);

      if(!$obj_data[dedit])
        if($error = check_required($obj_data, 
            ['type' => "Type"])) 
            json_error("Missing: ".implode(', ', $error));
      if($obj_id) {
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
      }else{
        list($id, $errors) = add_object($obj_type, $obj_data);
      }            
      if($errors) json_error($errors);
      
      $obj = get_object($obj_type, $id);
      
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved related artwork";
      
      echo(json_encode($result));
      exit;      
      break;

    case 'save-profile':
       extract($obj_data);
       //dump($obj_data);
       $id = $obj_data[obj_id];
       $user_id = $obj_data[user_id];
       $contact = $obj_data[contact];
       $obj_type = 'user_profile';

       $obj_data[date_of_birth] = date("Y-m-d", strtotime($obj_data[date_of_birth]));
       
       if($obj_data[date_of_birth]){
        $today = today();     
        $dateOfBirth = date("Y-m-d", strtotime($obj_data[date_of_birth]));      
        $diff = date_diff(date_create($dateOfBirth), date_create($today));
        $age = $diff->format("%y");
        if ($age < 18) json_error("Age should be greater than 18");
       }

       if($obj_data[opening_time]){
        $days = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'];
        $opening_time = $obj_data[opening_time];
        $closing_time = $obj_data[closing_time];
        $holiday = $obj_data[holiday];
        $time = [];
        for($i=0; $i<=6; $i++){
          $day = $days[$i];
          $opening_times = $start = $opening_time[$day];
          $closing_times = $end = $closing_time[$day];
          if($holiday[$day] == '1' ){
            $opening_times = '0:00';
            $closing_times = '0:00';
          }
           
          if( (strlen($start) != 5) ||  (strlen($end) != 5)){
            $start = str_pad($start, 5, "0", STR_PAD_LEFT);
            $end = str_pad($end, 5, "0", STR_PAD_LEFT);
          }
          if($start >= $end){
            if(($start != '00:00') && ($end != '00:00')) json_error($day."- End time must be after start time");
          } 
          $time[$day] = $opening_times.'-'.$closing_times;        
        }
        $obj_data['working_hours'] = json_encode($time);
       }

       $datas = [];
       $count = count($contact['title']);
       if($contact){
         for( $i=0; $i<$count; $i++){
          if($contact['title'][$i] != ''){
           $datas[$i]['title'] = $contact['title'][$i];
           $datas[$i]['email'] = $contact['email'][$i];
           $datas[$i]['phone'] = $contact['phone'][$i];
          }
         }
       }
       
       if($obj_data[contact]) $obj_data[contact] = json_encode($datas,True);
       if($obj_data[social])  $obj_data[social] = json_encode($obj_data[social]);
       $artist_name = $obj_data[display_name];
        
       if($id) {
         list($id, $errors) = update_object($obj_type, $obj_data, $id);       
       } else {
         list($id, $errors) = add_object($obj_type, $obj_data);
       }

       if($errors) json_error($errors);
       else {
        if($artist_name) $artist_update = update_artist_name($user_id, $artist_name);
       }

       $obj = get_object($obj_type, $id);
       $obj->next_tab = $obj_data['next_tab'];
       
       $result->success = 1;
       $result->data = $obj;
       $result->message = "OK. Saved profile";

       echo(json_encode($result));
       exit;      
       break;

       
      case 'save-details':
        extract($obj_data);
        
        $id = $obj_data[obj_id];
        $user_id = $obj_data[user_id];
        $contact = $obj_data[contact];
        $obj_type = 'user_profile';

        if($obj_data[opening_time]){
         $days = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'];
         $opening_time = $obj_data[opening_time];
         $closing_time = $obj_data[closing_time];
         $holiday = $obj_data[holiday];
         $time = [];
         for($i=0; $i<=6; $i++){
           $day = $days[$i];
           $opening_times = $start = $opening_time[$day];
           $closing_times = $end = $closing_time[$day];
           if($holiday[$day] == '1' ){
             $opening_times = '0:00';
             $closing_times = '0:00';
           }
            
           if( (strlen($start) != 5) ||  (strlen($end) != 5)){
             $start = str_pad($start, 5, "0", STR_PAD_LEFT);
             $end = str_pad($end, 5, "0", STR_PAD_LEFT);
           }
           if($start >= $end){
             if(($start != '00:00') && ($end != '00:00')) json_error($day."- End time must be after start time");
           } 
           $time[$day] = $opening_times.'-'.$closing_times;        
         }
         $obj_data['working_hours'] = json_encode($time);

        }

        $datas = [];
        $count = count($contact['title']);
        if($contact){
          for( $i=0; $i<$count; $i++){
           if($contact['title'][$i] != ''){
            $datas[$i]['title'] = $contact['title'][$i];
            $datas[$i]['email'] = $contact['email'][$i];
            $datas[$i]['phone'] = $contact['phone'][$i];
           }
          }
          $obj_data[contact] = json_encode($datas,True);
        }
        
        if($obj_data[social])  $obj_data[social] = json_encode($obj_data[social]);

        if($id) {
          list($id, $errors) = update_object($obj_type, $obj_data, $id);       
        } 
        if($errors) json_error($errors);

        $obj = get_object($obj_type, $id);
        $obj->next_tab = $obj_data['next_tab'];
        $result->success = 1;
        $result->data = $obj;
        $result->message = "OK. Saved profile details";
        echo(json_encode($result));
        exit;      
      break;

      case 'save-cv':
        extract($obj_data);
        
        function sortyear($a, $b){
          if ($a['year'] == $b['year']) return 0;
          list($ay,$am,$ad) = explode('-', $a['year']);
          list($by,$bm,$bd) = explode('-', $b['year']);
          if ($ay == $by){
            $am = strtotime($am); 
            $bm = strtotime($bm);
            return ($am < $bm ? 1 : -1);
          }              
          else return ($a['year'] < $b['year'] ? 1 : -1);
        }  

        if($obj_data[solo_shows_]){
          $solo_shows_ = $obj_data[solo_shows_];
          $count = count($solo_shows_['year']);
          $solo = [];
          for( $i=0; $i<$count; $i++){
            if($solo_shows_['year'][$i] != ''){
              $solo[$i]['year'] = $solo_shows_['year'][$i].'-'.$solo_shows_['month'][$i];
              $solo[$i]['description'] = $solo_shows_['description'][$i];
            }         
          } 
          uasort($solo, 'sortyear');
          $obj_data[solo_shows_] = json_encode(array_values($solo));
        }
          
        if($obj_data[group_shows_]){
          $group_shows_ = $obj_data[group_shows_];
          $count = count($group_shows_['year']);
          $group = [];
          for( $i=0; $i<$count; $i++){
            if($group_shows_['year'][$i] != ''){
              $group[$i]['year'] = $group_shows_['year'][$i].'-'.$group_shows_['month'][$i];
              $group[$i]['description'] = $group_shows_['description'][$i];
            }         
          } 
          uasort($group, 'sortyear');
          $obj_data[group_shows_] = json_encode(array_values($group));
        }
          
        if($obj_data[press_data]){
          $press_data = $obj_data[press_data];
          $count = count($press_data['year']);
          $press_arr = [];
          for( $i=0; $i<$count; $i++){
            if($press_data['year'][$i] != ''){
              $press_arr[$i]['year'] = $press_data['year'][$i].'-'.$press_data['month'][$i];
              $press_arr[$i]['description'] = $press_data['description'][$i];
            }         
          }   
          uasort($press_arr, 'sortyear');     
          $obj_data[press_data] = json_encode(array_values($press_arr));
        }
          
        if($obj_data[awards]){
          $awards = $obj_data[awards];
          $count = count($awards['year']);
          $datas = [];
          for( $i=0; $i<$count; $i++){
            if($awards['year'][$i] != ''){
              $datas[$i]['year'] = $awards['year'][$i].'-'.$awards['month'][$i];
              $datas[$i]['description'] = $awards['description'][$i];
            }         
          } 
          uasort($datas, 'sortyear');       
          $obj_data[awards] = json_encode(array_values($datas));
        }  

        if($obj_data[show_data])  $obj_data[show_data] = json_encode($obj_data[show_data],True);
        if($id){
          list($id, $errors) = update_object($obj_type, $obj_data, $id);
        }else{
          list($id, $errors) = add_object($obj_type, $obj_data);
        }

        $obj = get_object($obj_type, $id);
        $obj->next_tab = $obj_data['next_tab'];
        $result->success = 1;
        $result->data = $obj;
        $result->message = "OK. Saved profile CV";
        echo(json_encode($result));
        exit;      
      break;

      case 'save-minisite':
        extract($obj_data);
        $obj_type = 'minisite';
        $obj_data['url'] = str_replace(' ', '', $obj_data['url']);
        $obj_data['show_title'] = 1;
        $obj_data['show_subtitle'] = 1;
        $obj_data['show_timing'] = 0;
        $obj_data['show_social'] = 0;
        $obj_data['font_type'] = 1;
        $obj_data['menu_type'] = 2;
        $obj_data['hover_type'] = 1003;
        $obj_data['art_animation'] = 1003;
        $obj_data['icon_size'] = 10;
        $obj_data['fixed_header'] = 0;
        $obj_data['show_homemenu'] = 0;
        $user_id = $obj_data['user_id'];
        $duplicate = 0;

        $check_dups = find_objects('minisite', ['user_id'=> $user_id, 'active'=>1]);
        foreach($check_dups as $check_dup){
          if($check_dup->url == $obj_data['url'])
            $duplicate = 1;
        }

        if(!$obj_id){
          if(!$duplicate){
            list($id, $errors) = add_object($obj_type, $obj_data);
            print_log("save-minisite: added: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
            if($id){
              $data = add_default_page($id, $obj_data);
            } 
          }else{
            $errors = "Unfortunately '".$obj_data['url']."' is already taken by another user, Please try again to choose a unique vpatina URL (hint, you can use numbers as well as letters)";
          }
        }
        if($errors) json_error($errors);  

        $obj = get_object($obj_type, $id);
        $obj->next_tab = $obj_data['next_tab'];
        $result->success = 1;
        $result->data = $obj;
        $result->message = "OK. Saved minisite";
        echo(json_encode($result));
        exit;      
      break;

      case 'save-minisite-page':
        extract($obj_data);
        //dump($obj_data);
        $obj_type = "minisite_page";
        $obj_id = $obj_data[obj_id];  
        $user_id = $obj_data[user_id];
        $minisite_id = $obj_data[minisite_id]; 

        if($obj_data['menu_title']){ 
          $menu_title = url_safe($obj_data['menu_title'], true);
          $slug = $obj_data['slug'] = url_safe("/".$menu_title."/", true); // slashes allowed
        }
        if(!$obj_id)
          $col = find_objects('minisite_page', ['user_id' => $obj_data[user_id], 'minisite_id' => $obj_data[minisite_id],'slug' => $obj_data['slug'] , 'active' => 1]);
        else
          $col = $data = db_getAll("select * from minisite_page where user_id= '$obj_data[user_id]' AND active= '1' AND slug= '$slug' AND id!= '$obj_id' AND minisite_id = '$obj_data[minisite_id]' ");

        if($col)  json_error($obj_data['slug']." is already used, Please choose a unique menu title"); 
        
        if($obj_data[minisite_id]){
          if(!$obj_id){                                                       //Adding default values for each page
            if($obj_data[type] == 20){                                        //artist page
              $obj_data['page_layout'] = get_layout($obj_data[type], 1);
              $obj_data['second_layout'] = 131;
              $obj_data['template'] = 10;
              $obj_data['show_details'] = 1;
            }elseif($obj_data[type] == 30){                                   //exhibition
              $obj_data['data'] = get_exhibitionJson();
              $obj_data['page_layout'] = get_layout($obj_data[type], 1);
              $obj_data['second_layout'] = 133;
              $obj_data['template'] = 10;
              $obj_data['show_details'] = 1;
            }elseif($obj_data[type] == 40 ){                                   //contact 
              $obj_data['page_layout'] = get_layout($obj_data[type], 1);
            }elseif($obj_data[type] == 50){                                     //bio
              $obj_data['page_layout'] = 131;
              $obj_data['template'] = 10;
              $obj_data['show_details'] = 1;
            }elseif($obj_data[type] == 60){                                    //Single Exhibition
              $obj_data['data'] = get_sExhibitionJson();
               //$obj_data['page_layout'] = get_layout($obj_data[type], 2);
               $obj_data['page_layout'] = 294;
               $obj_data['template'] = 10;
               $obj_data['show_details'] = 1;
            }elseif($obj_data[type] == 80){                                   //Manual exhibition
              $obj_data['page_layout'] = 103;
              $obj_data['second_layout'] = 133;
              $obj_data['template'] = 10;
              $obj_data['show_details'] = 1;
            }elseif($obj_data[type] == 90){                                    
              $obj_data['template'] = 10;
            }
            $obj_data['row_order'] = get_rowOrder($minisite_id);

            list($id, $errors) = add_object($obj_type, $obj_data);
            print_log("save-minisite: added: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
          }else{
            $minisite = get_object('minisite_page', $obj_id);
            if(($minisite->type == 70) && ($obj_data[data])){
              if (filter_var($obj_data[data], FILTER_VALIDATE_URL) == false)  json_error("Invalid URL");
            }

            list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
            print_log("save-ministe: updated: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
          }
          
        }else{
          json_error("There is no minisite ID"); 
        }        
            
        if($errors) json_error($errors); 

        $obj = get_object($obj_type, $id);
        $obj->next_tab = $obj_data['next_tab'];
        $result->success = 1;
        $result->data = $obj;    
        $result->message = "OK. Minisite Pages Saved";       
        echo(json_encode($result));
        exit;      
      break;

      case 'save-minisite-artist':
        extract($obj_data); 

        $user_id = $obj_data[user_id];      
        $page_id = $obj_data[page_id];
        if($obj_data[artist_ids])
          $artist_ids = json_decode($obj_data[artist_ids], True);
        if($obj_data[artist_ids1])
          $artist_ids1 = json_decode($obj_data[artist_ids1], True);

        if($page_id){
          $data = find_object($obj_type, ['id' => $page_id, 'user_id' => $user_id, 'active' => '1']);

          if($artist_ids){
            if($data->search)
              $obj_data[search] = json_encode(array_values(array_unique(array_merge(json_decode($data->search, true),json_decode($obj_data[artist_ids], true)))));
            else
              $obj_data[search] = $obj_data[artist_ids];
          }elseif($artist_ids1){
            if($data->search1)
              $obj_data[search1] = json_encode(array_values(array_unique(array_merge(json_decode($data->search1, true),json_decode($obj_data[artist_ids1], true)))));
            else
              $obj_data[search1] = $obj_data[artist_ids1];
          }else
            json_error("Missing Artist ID");

          if(($obj_data[search] != 'null') || ($obj_data[search1] != 'null')){
            list($id, $errors) = update_object($obj_type, $obj_data, $page_id);
            print_log("save-ministe: updated: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
          }else
            json_error("Missing Artist ID");
        }else{
          json_error("Missing page ID");
        }

        $result->success = 1; 
        $result->message = "Artist will be added to minisite"; 
        echo(json_encode($result));
        exit;
      break;

    case 'save-minisite-basic':
      extract($obj_data);
      //print_r($obj_data[url_slug]);exit;
      $obj_type = $obj_data[obj_type];
     // print_r($obj_type);
      if($obj_data[icon_type] == 1)
        $obj_data[icon_size] = 1;

      if($obj_data[show_mail]){
        if(!$obj_data[mailtype] || !$obj_data[u_key] || !$obj_data[u_id] || !$obj_data[b_id])
          json_error("Please enter the required fields");
      }

      if($obj_data[domain]){

        $prot = $obj_data[domain];
        $port_needle = 'http://';
        if (strpos($prot,$port_needle) !== false) {
           $obj_data[url_slug] = $port_needle;
        }

        $prot_https = $obj_data[domain];
        $port_https_needle = 'https://';
        if (strpos($prot_https,$port_https_needle) !== false) {
           $obj_data[url_slug] = $port_https_needle;
        }
        
        $www = $obj_data[domain];
        $www_needle = 'http://www.';
        if (strpos($www,$www_needle) !== false) {
          $obj_data[url_slug] = $www_needle;
        }

        $obj_data[domain] = str_replace('https://', '', $obj_data[domain]);
        $obj_data[domain] = str_replace('http://', '', $obj_data[domain]);
        $obj_data[domain] = str_replace('www.', '', $obj_data[domain]);
        $obj_data[domain] = rtrim($obj_data[domain], '/');
      }


      $data = [];
      $data[action_url] = $obj_data[action_url] ? $obj_data[action_url] : 0;
      $data[show_mail] = $obj_data[show_mail] ? $obj_data[show_mail] : 0;
      $data[mailtype] = $obj_data[mailtype] ? $obj_data[mailtype] : 0;
      $data[u_key] = $obj_data[u_key] ? $obj_data[u_key] : 0;
      $data[u_id] = $obj_data[u_id] ? $obj_data[u_id] : 0;
      $data[b_id] = $obj_data[b_id] ? $obj_data[b_id] : 0;

      $obj_data['footer_data'] = json_encode($data);
      
      if($obj_id) {
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
        print_log("save-ministe: updated: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
      } else {
        list($id, $errors) = add_object($obj_type, $obj_data);
        print_log("save-minisite: added: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
      }              
      if($errors) json_error($errors);

      $obj = get_object($obj_type, $id);
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Minisite basic Saved";
      echo(json_encode($result));exit;      
    break;

    case 'save-minisite-style':
      extract($obj_data);
      
      if($obj_id){
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
        print_log("save-ministe-style: updated: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
      }
      if($errors) json_error($errors); 

      $obj = get_object($obj_type, $id);
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved minisite style";    
    break;

    case 'save-minisite-layout':
      extract($obj_data);
      if($obj_id){
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
        print_log("save-ministe-style: updated: {$obj_type} id: {$obj_id} errors".dump($errors,true), 'minisite', LOG_LEVEL_TEST);
      }
      if($errors) json_error($errors); 

      $obj = get_object($obj_type, $id);
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved minisite layout";    
    break;

    case 'save-minisite-single-exhibition':
      extract($obj_data);
      $data = [];
      $data[etype] = $obj_data[eType];
      $data[showtype] = $obj_data[showtype];
      $obj_data['data'] = json_encode($data);
      if($obj_id){
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
      }
      if($errors) json_error($errors); 

      $obj = get_object($obj_type, $id);
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved minisite exhibition"; 
    break;

    case 'save-minisite-mailform':
      extract($obj_data);

      if($obj_data[show_mail]){
        if(!$obj_data[mailtype] || !$obj_data[u_key] || !$obj_data[u_id] || !$obj_data[b_id])
          json_error("Please enter the required fields");
      }
      $data = [];
      $data[show_mail] = $obj_data[show_mail] ? $obj_data[show_mail] : 0;
      $data[mailtype] = $obj_data[mailtype] ? $obj_data[mailtype] : 0;
      $data[u_key] = $obj_data[u_key] ? $obj_data[u_key] : 0;
      $data[u_id] = $obj_data[u_id] ? $obj_data[u_id] : 0;
      $data[b_id] = $obj_data[b_id] ? $obj_data[b_id] : 0;

      $obj_datas['data'] = json_encode($data);
      
      if($obj_id){
        list($id, $errors) = update_object($obj_type, $obj_datas, $obj_id);
      }
      if($errors) json_error($errors); 

      $obj = get_object($obj_type, $id);
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved Mail Form"; 
    break;

    case 'save-folder':
      extract($obj_data);
      $obj_data[type] = "folder";
      $user_id = $obj_data[user_id];
      $filename = $obj_data[filename];
      $obj_data[tags] = $obj_data[tags] ? json_encode($obj_data[tags]) : '';
        
      if(!$obj_id){
        $get_duplicate = find_object($obj_type, ['user_id'=> $user_id, 'filename'=> $filename, 'active'=>1 ]);
        if(!$get_duplicate){
          list($id, $errors) = add_object($obj_type, $obj_data);
        }else{
          json_error("Duplicate Folder Name");
        }
      }else{
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
      }
      if($errors) json_error($errors); 
      $obj = get_object($obj_type, $id);

      if($obj){
        $obj->path = "/user/".$obj_data[user_id]."/vault/".$id;
        list($id1, $errors) = update_object($obj_type, $obj, $id);
      }
      $obj1 = get_object($obj_type, $id1);
      $result->success = 1;
      $result->data = $obj1;
      $result->message = "OK. Saved folder"; 
    break;

    case 'save-media':
      extract($obj_data);
      $obj_data[tags] = $obj_data[tags] ? json_encode($obj_data[tags]) : '';
        
      if($obj_id){
       list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
      }
      if($errors) json_error($errors);

      $obj = get_object($obj_type, $id);
      $result->success = 1;
      $result->data = $obj;
      $result->message = "OK. Saved media"; 
    break;

    case 'get_layout_id':
      extract($obj_data);
      $obj_type = $obj_data['obj_type'];
      $obj=[];
      if($obj_data['level_2']) $level_2 = $obj_data['level_2'] ;
      else      $level_2 = 0;
      if($obj_data['parent_id']){
        if(($obj_data['level'] == 1) || ($obj_data['level'] == 3) ){
          $get_datas = find_objects($obj_type, ['parent_id' => $parent_id], ['sort' => 'row_order asc']);
        }elseif($obj_data['level'] == 2){
          $get_datas = find_objects($obj_type, ['parent_id' => $parent_id, 'level1_id'=>$obj_data['level1_id']]);
        }
        if($get_datas){
         $i = 0;
         foreach($get_datas as $get_data){
           $obj[$i]['id'] = $get_data->id; 
           $obj[$i]['name'] = $get_data->name; 
           $i = $i+1;
         }
        }
      }
      $result = $obj;
    break;

    case 'get_billboard_level':
      extract($obj_data);

      $obj_type = $obj_data['obj_type'];
      $obj=[];

      if($obj_data['parent_id']){
        if($obj_data['level'] == 1){
          $get_datas = find_objects($obj_type, ['parent_id' => $parent_id, 'level_2'=>0], ['sort' => 'row_order asc']);
        }elseif($obj_data['level'] == 2){
          $get_datas = find_objects($obj_type, ['parent_id' => $parent_id, 'level_2'=>$obj_data['level1_id']], ['sort' => 'row_order asc']);
        }elseif($obj_data['level'] == 3){
          $get_datas = find_objects('minisite_layout', ['parent_id' => $parent_id, 'level1_id' =>$obj_data['level1_id']], ['sort' => 'row_order asc']);
        }
        if($get_datas){
         $i = 0;
         foreach($get_datas as $get_data){
           $obj[$i]['id'] = $get_data->id; 
           $obj[$i]['name'] = $get_data->name; 
           $i = $i+1;
         }
        }
      }
      $result = $obj;
    break;

    case 'save-event':
      $obj_type = 'event';
      print_log("save-event: data:".dump($obj_data,true), 'collection', LOG_LEVEL_TEST);

      extract($obj_data);
      if($error = check_required($obj_data, 
          ['parent_type' => "Parent Type", 'parent_id' => "parent_id", 'title' => "Title", 'event_date' => "Date", 'event_start_time' => "Start Time", 'event_end_time' => "End Time"]
        )) json_error("Missing: ".implode(', ', $error));
      
      $parentObj = get_object($parent_type, $parent_id);
      $sql_date = date2sql($obj_data['event_date']);
      if(!is_date($sql_date)) json_error("{$obj_data['event_date']} is not a valid date");
      if($sql_date < today()) json_error("Date cannot be in the past.");

      $obj_data['start_time'] = $start = "{$sql_date} {$obj_data['event_start_time']}:00";
      $obj_data['end_time']  = $end = "{$sql_date} {$obj_data['event_end_time']}:00";
      
      $lat = $obj_data['location_lat'];
      $lng = $obj_data['location_long'];

      $update_location = 0;
      $event_title = $obj_data['title'];
      $locData = ['user_id' => $parentObj->user_id ?: $u_id, 'parent_type' => $obj_type, 'name' => "Event: {$event_title}"];
      if($id) $locData = ['parent_id' => $id];
      else $update_location = 1;
      
      $loc_flds = ['location_lat','location_long','location_accuracy','address_street','address_number','address_state','address_postcode','address_city','address_country','address_formatted'];
      if($loc_id = $obj_data['location_id']) {
        $locObj = get_object('location', $loc_id);
        // copy data to this event object
        foreach($loc_flds as $fld) $obj_data[$fld] = $locObj->$fld;
      } else if($lat && $lng) {
        foreach($loc_flds as $fld) $locData[$fld] = $obj_data[$fld];
        list($loc_id, $loc_errors) = add_object('location', $locData);
        if($loc_errors) json_error($loc_errors);
        $obj_data['location_id'] = $loc_id;        
      } else {    
        json_error("Please select or enter a location.");
      }
      print_log("save-event: loc={$loc_id} date={$sql_date} start={$start} end={$end} data:".dump($obj_data,true), 'collection', LOG_LEVEL_TEST);
      
      if($start >= $end) json_error("End time must be after start time");
      
      
      if($obj_id) {
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
      } else {
        list($id, $errors) = add_object($obj_type, $obj_data);
      }
      if($errors) json_error($errors);

      if($update_location && $loc_id) {       
        $locData = ['parent_id' => $id];
        list($loc_id, $loc_errors) = update_object('location', $locData, $loc_id);
      }

      $result->success = 1;
      $result->message = "OK. Saved event \"{$obj->title}\"";
      
      echo(json_encode($result));exit;      
      break;
      
    case 'save-collection':
      $obj_type = 'media_collection';
      extract($obj_data);
      //dump($obj_data);
      $address_formatted = $obj_data['address_formatted'];
      $type_name = get_object('media_collection_type', $type, 'name'); 

      
      $id = $obj_id;
      if(isset($obj_data['subtype'])) { // first tab
      //=====================Add New Column Exhibition Sequence Vignesh 02-09-2010=============//
        if($obj_data['subtype'] == '40' || $obj_data['subtype'] == '50'){
            if($id != '')
            {
            $media_collection_map = db_getAll("SELECT  *,min(original_year) as year FROM media_collection_map WHERE media_collection_id=".$id." AND original_year != '' GROUP BY media_collection_id");
            //dump(count($media_collection_map));exit;
              if(count($media_collection_map) == 1)
              {
                foreach($media_collection_map as $mval){
                    $year = "".$mval->year."-01-01";
                    update_object('media_collection', ['exhibition_sequence' => $year], $mval->media_collection_id);
                }
              }
              else
              {
                  $year = "1000-01-01";
                  update_object('media_collection', ['exhibition_sequence' => $year], $id);
              } 
            }
            else
            {
               $obj_data['exhibition_sequence'] = "1000-01-01"; 
            }    
        }
      //=====================Add New Column Exhibition Sequence Vignesh 02-09-2010=============//
      //dump($obj_data['exhibition_sequence']);exit;
        if($error = check_required($obj_data, 
            ['title' => "Title", 'subtype' => "Type"]
          )) json_error("Missing: ".implode(', ', $error));
      } else { // later tab, gotta be an ID
        if(!$obj_id) json_error("Missing ID, not first tab");
        $obj = get_object('media_collection', $obj_id);

        if(isset($obj_data['location_id'])) { // location tab
          $lat = $obj_data['location_lat'];
          $lng = $obj_data['location_long'];
          
          //if(!strlen(trim($obj_data['start_date'])) 
            
          $update_location = 0;
          $ex_type = get_object('exhibition_type', $obj->subtype, 'name');
          $locData = ['user_id' => $obj->user_id, 'parent_type' => $obj_type, 'parent_id' => $obj_id, 'name' => "Exhibition - {$ex_type}: \"{$obj->title}\""];
          
          $loc_flds = ['location_lat','location_long','location_accuracy','address_street','address_number','address_state','address_postcode','address_city','address_country','address_formatted'];
          if($loc_id = $obj_data['location_id']) {
            //if(!$id) 
            $obj_data['address_formatted'] = get_object('location', $loc_id, 'address_formatted');
          } else if($lat && $lng) {
            foreach($loc_flds as $fld) $locData[$fld] = $obj_data[$fld];
            list($loc_id, $loc_errors) = add_object('location', $locData);
            if($loc_errors) json_error($loc_errors);
            $obj_data['location_id'] = $loc_id;        
          } else if(isset($obj_data['location_id'])) {    
            json_error("Please select or enter a location.");
          }
          $start_date = $obj_data['start_date'] = date2sql($obj_data['start_date']);
          $end_date = $obj_data['end_date'] = date2sql($obj_data['end_date']);
          //=====================Add New Column Exhibition Sequence Vignesh 02-09-2010=============//
          if($ex_type == 'Gallery Show' || $ex_type == 'Art Fair' || $ex_type == 'Other Event'){
            $obj_data['exhibition_sequence'] = $start_date;
          }
          //=====================End Column Exhibition Sequence Vignesh 02-09-2010=============//
          //print_r($obj_data);exit;
          print_log("save-collection: loc={$loc_id} date={$sql_date} start={$start_date} end={$end_date} data:".dump($obj_data,true), 'collection', LOG_LEVEL_TEST);
          
          if($start_date > $end_date) json_error("Start cannot be after end");
        }
      }
      //print_log("add:$address org:$address_formatted changed: $address_changed", 'collection', LOG_LEVEL_TEST);

      if($obj_id) {
        /* upload title image validation */         
        if($obj_data['title_image_option'] == 0){   //Inactive existing media if title image selected from artwork
          $data = find_object('media_collection', ['id' => $obj_id]);
          if($data->title_image_option == 1){
            $image_id = $data->title_image;
            if($image_id){
              $find_media = find_object('media', ['id' => $image_id]);
              $find_media->active = 0;
              list($id, $errors) = update_object('media', $find_media, $image_id);
            }
            if($errors) json_error($errors);
          }
          
          if($obj_data['title_image']){         //Json encode artwork image path
            $title_image = $obj_data['title_image']; 
            $find_media = find_object('media', ['id' => $title_image]);
            $title_image_arr[]['filename'] = $find_media->path.'medium/'.$find_media->src;
            $title_image_arr[]['id'] = $find_media->id;
            $title_image_arr[]['type'] = $find_media->type;
            $obj_data['title_image_json'] = json_encode($title_image_arr);
          }

        }else{
          $data = find_object('media_collection', ['id' => $obj_id]);   //Save image_id on upload
          $title_images = json_decode($data->title_image_json);
          foreach($title_images as $title_image){
            $obj_data['title_image'] = $title_image->id;
          }
        }
        if($address_formatted){
          $obj_data['address_formatted'] = $address_formatted;
        }
        if(($obj_data['subtype'] == 40) || ($obj_data['subtype']== 50)){
          $obj_data['start_date'] = '';
          $obj_data['end_date'] = '';
        }
        //print_r($obj_data);exit;
        list($id, $errors) = update_object($obj_type, $obj_data, $obj_id);
      } else {
        list($id, $errors) = add_object($obj_type, $obj_data);
      }
      //print_r($obj_data['exhibition_sequence']);
      if($errors) json_error($errors);
      if(!$id) json_error("Missing ID");
     
      if($errors) json_error($errors);
      
      $obj = get_object('media_collection', $id);
      $obj->next_tab = $obj_data['next_tab'];

      $result->message = "OK: Saved {$type_name} \"{$obj->title}\"";
      $result->data = $obj;
      $result->success = 1;
      //$result->redirect = "/backend/exhibition/{$id}/edit#tab2";
      
      echo(json_encode($result));exit;
      
      /**

      // get formatted address
      $address_changed = false;
      $address = '';
      if($address_street || $address_number || $address_postcode || $address_city || $address_state || $address_country) {
        $country_name = cc_name($address_country);
        $address = implode(",", array_filter([trim("$address_street $address_number"), trim("$address_postcode $address_city"), $address_state, $country_name]));
        if($address != $address_formatted) {          
          $address_changed = true;
          $obj_data['address_formatted'] = $address;
        }
      }  

      if($type == MEDIA_COLLECTION_TYPE_EXHIBITION) {
        $in  = $data['start_date'] = date2sql($start_date);
        $out  = $data['end_date'] = date2sql($end_date);
        
        if($address) {
          if($address_changed) {  // geocode if new or changed

            list($status, $acc, $lat, $long, $bounds) = google_geocode($address);
            // print_log("geocoding. status=$status", 'collection', LOG_LEVEL_TEST);
            if($status==200) {
              $locData = ['name' => $title, 'parent_type' => 'media_collection', 'parent_id' => $id, 'location_lat' => $lat, 'location_long' => $long, 'location_bounds' => $bounds];
              $locData['address_formatted'] = $address;
              foreach(['address_street', 'address_number', 'address_city', 'address_state', 'address_postcode', 'address_country'] as $fld) $locData[$fld] = $obj_data[$fld];

              if($location_id) {
                list($loc_id, $loc_errors) = update_object('location', $locData, $location_id);
              } else {
                list($loc_id, $loc_errors) = add_object('location', $locData);
                if($loc_id) update_object('media_collection', ['location_id' => $loc_id], $id);                
                $location_id = $loc_id;
                $obj_data['location_id'] = $loc_id;
              }              
              if($loc_errors) json_error($loc_errors);
              
            } else {
              json_error("Failed to geocode $address");
            }
          }
          
          // if($lat && $lng && !$bounds) $bounds = json_encode(radius2bounds($obj, DEFAULT_RADIUS));
        }
        
        // Associated event
        if($event_title || $event_date || $event_start_time || $event_end_time) {
          if(!$location_id) json_error("To set an associated event, please first set Exhibition dates and address (above).");
          $locObj = get_object('location', $loc_id);
          $event_in  = date2sql($event_date);          
          $event_start = "{$event_in} {$event_start_time}:00";
          $event_end = "{$event_in} {$event_end_time}:00";          
          $eventData = ['name' => $event_title, 'type' => EVENT_TYPE_OTHER, 'parent_type' => 'media_collection', 'parent_id' => $id, 'start_time' => $event_start, 'end_time' => $event_end];
          foreach(['address_street', 'address_number', 'address_city', 'address_state', 'address_postcode', 'address_country'] as $fld) $eventData[$fld] = $locObj->$fld;
          foreach(['location_lat', 'location_long', 'location_bounds'] as $fld) $eventData[$fld] = $locObj->$fld;
          $eventData['start_address'] = $address; 

          print_log("$event_date + $event_start_time => $event_start ", 'collection', LOG_LEVEL_TEST);
 
          if($event_id) {
            list($event_id, $event_errors) = update_object('event', $eventData, $event_id);
          } else {
            list($event_id, $event_errors) = add_object('event', $eventData);
            if($event_id) update_object('media_collection', ['event_id' => $event_id], $id);                
          }              
          if($event_errors) json_error($event_errors);          
        }        
      }
      */
      
       
      break;
      
    /** Public handler for saving name/address/arrival details only */    
    case 'save-booking':
      $obj_data = $_POST;
      extract($obj_data);
      if(!$res_id) json_error("Missing ID");
      $allowed_fields = array(
        'res_id', 'salutation' , 'first_name' , 'last_name', 
        'address1' , 'address2' , 'city', 
        'state' , 'zip' , 'country' , 'email_address', 'phone',
        'organization', 'title',
        'arrival_method','arrival_from', 'arrival_number','arrival_time','arrival_property','arrival_notes');
      foreach($obj_data as $k => $v) {
        if(!in_array($k, $allowed_fields)) json_error("$k ?");
      }
      list($id, $errors) = update_object('reservation', $obj_data, $res_id);
      if($errors) json_error($errors);
      $result->message = "OK: saved details";
      $result->success = 1;
      echo(json_encode($result));exit;      
      break;
      
    case 'save-page':      
      $redirect = '';
      $obj_type = 'site_page';
      $org_path = '';
      $language = $obj_data['language'] ?: $_SESSION['language'];
      $cms = $obj_data['cms']; /** set when called from edit-site */
      $site_id = $obj_data['site_id'] ?: $_SESSON['site_id'];
      $path = url_safe("/".$obj_data['path']."/", true); // slashes allowed
      $obj_data['path'] = clean_path($path);
      
      if($obj_data['parent_id'] < 0) $obj_data['parent_id'] = 0;
      
      if($id) { /** edit page */
        $content_id = $obj_data['content_id'];
        if($pcObj = find_object('page_content', array('path' => $path, 'site_id' => $site_id, 'id' => ['!=', $content_id]))) { // check if unique
          json_error("$path is already used by ".($pcObj->menu_title ?: $pcObj->title)." Please choose a unique path"); 
        }
        list($id, $errors) = update_object($obj_type, $obj_data, $id);
        $org_path = get_object('page_content', $content_id, 'path');
      } else { /** new page */
        list($id, $errors) = add_object($obj_type, $obj_data);
        if($errors) json_error($errors);
        if(!$id) json_error("Failed to add page");
        $content_id = find_object('page_content', ["page_id" => $id, "language" => $language], 'id');
      }
      if(!$content_id) json_error("missing content id");
      if($errors) json_error($errors);

      //$org_path = get_object('page_content', $content_id, 'path');
      //json_error("id:$content_id org=$org_path new={$obj_data['path']}");      
      unset($obj_data['page_id']);
      unset($obj_data['id']);
      list($content_id, $errors) = update_object('page_content', $obj_data, $content_id);
      if($errors) json_error($errors);
      
      if(!$cms && $org_path && $path != $org_path) $result->redirect = $redirect = $path; /** only redirect if we edit in front-end, not back-end */

       /** old 
      if($add_content && $content_id) {
        $path = $obj_data['path'];
        $path = trim($path) ? clean_path(str_replace(' ', '-', $path)) : '';
        $path = "/".trim($path, '/')."/";
        $org_path = get_object('page_content', $content_id, 'path');
        unset($obj_data['id']);
        if($path && $path != $org_path) { 
          if($pcObj = find_object('page_content', array('path' => $path, 'site_id' => $site_id))) { // check if unique
            json_error("$path is already used by ".$pcObj->menu_title ?: $pcObj->title);
          } else {
            $obj_data['path'] = $path; // cleaned
            if(!$cms) $result->redirect = $redirect = $path;
          }
          
        }
        list($id, $errors) = update_object('page_content', $obj_data, $content_id);
        if($errors) json_error($errors);
      }
      */
      
      //$result->error = "Got here: id=$id cid=$content_id path=$path org=$org_path redirect=$redirect";
      $result->message = "OK: saved page.";
      $result->success = 1;
      $result->id = $id;
      echo(json_encode($result));exit;      
      break;

    /** used in back-end
    case 'save-page-cms':
      $content_id = $obj_data['id'];      
      $page_id = $obj_data['page_id'];      
      list($id, $errors) = update_object('page_content', $obj_data, $content_id);
      json_error("id=$content_id".dump($obj_data,true));
      if($errors) json_error($errors);
      //list($id, $errors) = update_object('site_page', $obj_data, $page_id);
      //if($errors) json_error($errors);
        
      $result = json_success_object("OK, saved page");
      echo(json_encode($result));exit;    
      break;
    */
    
    case 'savefile':
      require_once('Smarty/Smarty.class.php');
      if($error = check_required($obj_data, array('filename' => "Filename", 'content' => "Content"))) json_error("Missing: ".implode(', ', $error));
      $filename = $obj_data['filename'];
      $content = $obj_data['content'];
      if(!is_file($filename)) json_error("$filename is not a file");
      $username = posix_getpwuid(posix_geteuid())['name'];
  
      $org_file = read_file($filename);
      
      $result = new stdClass;
      if(!is_writeable($filename)) json_error("$filename is not writeable by $username");
      if($result->length = $len = write_file($filename, $content)) {
        $result->success = 1;
        $len_h = bytes2human($len);
        $result->message = "Wrote $len_h to $filename";
        
        $template = smarty_parse_template($app, $filename);
        if(strpos($template, '__smarty-error')) {
          write_file($filename, $org_file);
          $error = find_delimited_str($template, ">", "<");
          json_error("Error parsing template: $error");
          
        }
      } else {
        $result->success = 0;
        $result->error = "Could not write to $filename";
      }
      echo(json_encode($result));exit;    
      break;
  
    case 'readfile':
      if($error = check_required($obj_data, array('filename' => "Filename"))) {echo("Missing: ".implode(', ', $error));exit;}
      $filename = $obj_data['filename'];
      if(!is_file($filename)) {echo("$filename is not a file"); exit;}
      $username = posix_getpwuid(posix_geteuid())['name'];
      if(!is_writeable($filename)) {echo("$filename is not writeable by $username"); exit;}
      $content = read_file($filename);
      echo($content);exit;
      break;
    
    case 'mollie-pay':
      //require_once 'vendor/mollie/mollie-api-php/src/Mollie/API/Autoloader.php';
      if($error = check_required($obj_data, 
        array('site_id' => "Site ID", 'currency' => "Currency", 'amount' => "Amount", 'inv_id' => "Invoice ID", 'description' => "Description", 'url' => "URL"))) 
      json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $mollie_key =  $_SESSION['live'] || $_SESSION['live_payments_test'] ? get_config('ml', $site_id) : get_config('mt', $site_id);  
      if(!$mollie_key) json_error("No $field for site $site_id");
  
      $mollie = new Mollie_API_Client;
      $mollie->setApiKey($mollie_key);
      
      try {
        $payment = $mollie->payments->create(
          array(
            'amount'      => $amount,
            'description' => $description,
            'locale' => 'en',
            'method' => $payment_method,
            'redirectUrl' => $url,
            'metadata'    => array_filter(array(
                'inv_id' => $inv_id, 'res_id' => $res_id, 'site_id' => $site_id, 'user_id' => $user_id
            ))
          )
        );
    
        $result->success = 1;
        $result->redirect = $payment->getPaymentUrl();
        echo(json_encode($result));exit;    
      } catch (Mollie_API_Exception $e) {
        $msg = htmlspecialchars($e->getMessage());
        $debug = " Key: $key";
        $debug = "";
        json_error($msg.$debug);
        json_error("API call failed: " . htmlspecialchars($e->getMessage()) . " on field " + htmlspecialchars($e->getField()));
      }
      exit;
      break;
      
    case 'check-transaction':
      if($error = check_required($obj_data, array('site_id' => "Site ID", 'txn_id' => "Transaction ID", 'inv_id' => "Invoice ID"))) json_error("Missing: ".implode(', ', $error));
      $txn_id = $obj_data['txn_id'];
      $txObj = find_object('transaction', array('txn_id' => $txn_id));
      if($txObj) {
        $txn_res = $txObj->txn_res;
        if($txn_res == "VERIFIED") {
          //if($txObj->inv_id != $inv_id) json_error("Invalid invoice");
          $result->message = "Success: Received $txObj->mc_currency $txObj->gross";
          $result->success = 1;
          $result->reload = 1;
          $result->data = $txObj;
          echo(json_encode($result));exit;    
          
        } else {
          json_error("Transaction $txn_res");
        }
      } else {
        $result = json_success_object("Please wait...");
        echo(json_encode($result));exit;    
      }
      break;
      
    case 'register-newsletter':
      //json_error(dump($obj_data,true));
      if($error = check_required($obj_data, array('site_id' => 'Site ID', 'email_address' => 'Email address', 'remote_ip' => 'IP Address'))) json_error("Missing: ".implode(', ', $error));
      $email = trim($obj_data['email_address']);
      if(!is_email($email)) json_error("$email is not a valid email address");
      if(find_object("newsletter", array("site_id" => $site_id, "email_address" => $email))) json_error("$email is already signed up for the newsletter");
      list($id, $errors) = add_object('newsletter', $obj_data);
      if($errors) json_error($errors);
      $result->success = true;
      $result->message = "Thank you for signing up for our newsletter";
      echo(json_encode($result));exit;    
      break;
      
    // fetch a smarty template and echo it
    case 'wishlist':
      if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'obj_id'))) json_error("Missing: ".implode(', ', $error));
      extract($_GET);
  
      $wishlistSearch = array("site_id" => $site_id, "user_id" => $user_id, "obj_type" => $obj_type, "obj_id" => $obj_id);
      $wishlist_id = find_object("wishlist", $wishlistSearch, "id");
  
      if($wishlist_id) { // remove from wishlist
        $action = "delete";
        delete_object('wishlist', $wishlist_id);
        $wishlist_id = 0;
      } else { // add to wishlist
        $action = "add";
        list($wishlist_id, $errors) = add_object('wishlist', $obj_data);
        if($errors) json_error($errors);
      }
      $result = json_success_object("OK, $action wishlist $wishlist_id");
      $result->id = $wishlist_id;
      echo(json_encode($result));exit;    
      break;
      
    case 'helpful':
      if($error = check_required($_GET, array('obj_type', 'obj_id'))) json_error("Missing: ".implode(', ', $error));
      extract($_GET);
      if(!object_has_field($obj_type, 'helpful')) json_error("$obj_type has no helpful field");
      $table = get_object_table($obj_type);
      
      $already_helpful = $_SESSION['helpful'][$obj_type];
      
      if(in_array($obj_id, $already_helpful)) json_error("$obj_type $obj_id is already marked as helpful");
      $_SESSION['helpful'][$obj_type][] = $obj_id;    
      db_query("UPDATE $table SET helpful=helpful+1 WHERE id=$obj_id");
      $result = json_success_object("OK, incremented helpful");
      $result->count = get_object($obj_type, $obj_id, 'helpful');
      echo(json_encode($result));exit;    
      break;


    case 'site-tree':
            
      $site_id = $_SESSION['site_id'];
      $options = ['show_menu' => $obj_data['show_menu'], 'active' => $obj_data['active'], 'language' => $__LANG];
      $tree = site_tree($site_id, $options);
      
      /** dummy root node */      
      $root = new stdClass;
      $root->text = get_object('site', $site_id, 'name');
      $root->children = $tree;
      $root->child_count = count($tree);
      $root->icon = "fa fa-sitemap";
      $root->root = 1;
      $root->obj_type = 'site';
      $root->obj_id = $site_id;
      $root->id = -1; 
      $root->active = 1;
      echo(json_encode($root));exit();
      
      break;      
      
    /** like gallery_explore(), but generic for backend view */
    case 'cms-explore':
      
      //extract($_GET);
      $id = $_GET['id'] ?: 0;
      $obj_type = $_GET['obj_type'] ?: "site_page";
      switch($obj_type) {
        case 'site':        
          $template = "_site-edit";
          break;
        case 'site_page':
          $preview = isset($_GET['preview']) ? $_GET['preview'] : false; 
          $template = $preview ? "_site-page-view" : "_site-page-edit";
          break;
        default:
          break;
      }
      $controller = "cms_explore";
      
      $siteObj = $obj_data['site_id'] ? get_object('site', $obj_data['site_id']) : $__SITE;
      $pObj = new stdClass;
      $pObj->template = $template;
      $pObj->controller = $controller;

      $scope = new stdClass;      
      foreach($_GET as $k => $v) $scope->$k = $v;
      
      $this_app = clone $app;
      $this_app->page = $pObj;
      
      $output = fetch_template($this_app, array('scope' => $scope));
      echo($output);exit;
      break;
    

    case 'fetch-template':
      
      extract($obj_data);
      $siteObj = $obj_data['site_id'] ? get_object('site', $obj_data['site_id']) : $__SITE;
      $pageObj = $obj_data['page_id'] ? get_object('site_page', $obj_data['page_id']) : new stdClass;
      
      $this_app = new stdClass;
      $tpl = $tpl ?: $template; // alias

      if(!$tpl) return "Fetch templates: missing template";
      $pageObj->template = $tpl;   
      $this_app->site = $siteObj;
      $this_app->page = $pageObj;
      
      /** this can probably go away, just leave pageObj->controller and fetch_template will handle */

      print_log("fetch-template: tpl={$tpl} controller=$controller scope pin/id={$scope->pin}/{$scope->res_id}", 'app', LOG_LEVEL_TEST);
       
      if($controller) {
        unset($obj_data['template']);
        unset($obj_data['controller']);
        $pageObj->controller = $controller;
        $pageObj->scope = array2obj($obj_data);
        apply_controller($this_app, []);
        unset($pageObj->controller); 
      } elseif($obj_type && $id) {
        $obj = get_object($obj_type, $id);
        $scope = (array) $obj;
      }

      if($error = $scope->error ?: $scope->errors) {
        echo(print_alert($error, "error"));exit();
      }
      
      $l = $obj_data['__sl'];
      $this_app->page = $pageObj;
      $output = fetch_template($this_app, array('scope' => $scope));
  
      echo($output);      
      exit();
      break;

  case 'mail-test':
    extract($obj_data);
    $required = array('site_id', 'from', 'to', 'subject', 'template');    
    if($error = check_required($obj_data, $required)) json_error("Missing: ".implode(', ', $error));
    $siteObj = get_object('site', $site_id);

    $tscope = [];
    $tscope = $obj_data['data'] ? json_decode($obj_data['data'], true) : [];
    $tscope['personal_message'] = $message;

    
    if(strpos($to,',')) {
      $emails = explode(',', $to);
      foreach($emails as $email) if(!is_email($email)) json_error("{$email} is not a valid email address");
    } elseif (!is_email($to)) {
      json_error("$to is not a valid email address");
    }
    
    $pObj = new stdClass;
    $pObj->template = $template;             
    $pObj->controller = $controller = $obj_data['controller'];
    
    $res_id = $tscope['res_id'];
    //json_error("id=$res_id");
    $body = smarty_mail_template($siteObj, $template, ['scope' => $tscope, 'controller' => $controller]);
    
    $result = send_mail($siteObj, $subject, $body, $to);
    echo json_encode($result);exit;        

    break;
      
  case 'share':
    //json_error(dump($obj_data, true));
    extract($obj_data);
    $required = array('site_id', 'from', 'to', 'subject');
    if(!$url) $required[] = 'body';
    
    if($error = check_required($obj_data, $required)) json_error("Missing: ".implode(', ', $error));
    $siteObj = get_object('site', $site_id);

    //print_log("oper=share obj_data=".dump($obj_data, true), 'gallery', LOG_LEVEL_TEST);
    
    /** Custom template */
    if($template = $obj_data['template']) {                  
      $pObj = new stdClass;
      $pObj->template = $template;             
      $pObj->controller = $controller = $obj_data['controller']; 

      $tscope = [];
      $tscope = $obj_data['data'] ? json_decode($obj_data['data'], true) : [];
      $tscope['url'] = $url;
      $tscope['personal_message'] = $body;

      $this_app = clone $app;
      $this_app->page = $pObj;
      
      $body = fetch_template($this_app, array('scope' => $tscope));
      // print_log("oper=share $template/$controller tscope=".dump($tscope, true), 'gallery', LOG_LEVEL_TEST);
      
    } else { /** Generic: just send a link */
      
      $link = html_link($url, $title ?: $url);
      $mail_scope = ['main_message' => phrase('reset password link', CAPS).": {$link}"];
      $body = smarty_mail_template($siteObj, 'mail/mail-generic', ['scope' => $mail_scope]);
      
    }
    if(strpos($to,',')) {
      $emails = explode(',', $to);
      foreach($emails as $email) if(!is_email($email)) json_error("{$email} is not a valid email address");
    } elseif (!is_email($to)) {
      json_error("$to is not a valid email address");
    }
    
    $result = send_mail($siteObj, $subject, $body, $to);
    echo json_encode($result);exit;        
    break;  
      
    case 'send-review':
      global $__SITE;
      if($error = check_required($_POST, array('site_id', 'parent_type', 'parent_id'))) json_error("Missing: ".implode(', ', $error));
      //json_error(dump($_POST,true));
      $result = review_send($_POST);
      if($result->success && $result->id) $result->redirect = site_page_link($__SITE->id, PAGE_REVIEW).$result->id;
      echo json_encode($result);exit;        
    break;  
    
    case 'address-book':
      $rows = [];
      $val = $_GET['q'];
      if(strlen($val) >= 3) {
        $search[DB_LOGICAL_OR] = ['first_name' => "*$val*", 'last_name' => "*$val*", 'display_name' => "*$val*", ];
        $rows = find_objects("user_profile", $search, ['fields' => ['user_id as id', 'display_name as name', 'display_name as value', 'concat(first_name, " ", last_name) as full_name', 'photo as media', 'email_address'], 'sort' => 'name']);        
      }
      
      $result->success = 1;
      $result->data = $rows;
      echo json_encode($result);exit;        
      break;
      
    case 'campaign-send':
      
      if(!$_POST) json_error("campaign-send: POST only");
      if($error = check_required($_POST, array('campaign_id' => "Campaign ID", 'time_to_send' => "Time to send"))) json_error("Missing: ".implode(', ', $error));
      $campaign_id = $_POST['campaign_id'];
      $campaignObj = get_object('mail_campaign', $campaign_id);
      if(!$campaignObj) json_error("Campaign $campaign_id not found");
      
      if($campaignObj->sent_time) json_error("Campaign $campaign_id was already sent on {$campaignObj->sent_time} to {$campaignObj->sent_count} recipients. To resend, bo back and click 'Reuse campaign'"); 
      
      $recipients = json_decode($campaignObj->recipients);
      if(!count($recipients)) json_error("No recipients in campaign");
      
        
      extract($_POST);
      $now = now();
      $time_to_send = date('Y-m-d H:i:s', strtotime($time_to_send));
      list($id, $errors) = update_object('mail_campaign', ['time_to_send' => $time_to_send], $campaign_id);
      if($errors) json_error($errors);
      
      if($time_to_send <= $now) { // send now
        //json_error("sending: {$time_to_send} now={$now} got here id={$campaign_id}");
        $result = send_campaign($campaign_id);
        
      } else {
        //json_error("NOT sending yet: {$time_to_send} now={$now} got here id={$campaign_id}");
        $result = new stdClass;
        $result->success = 1;
        $result->message = "Campaign will be mailed $time_to_send UTC";
      }
      
      echo(json_encode($result));exit;
      break;
      
    case 'campaign-add':
      if(!$_POST) json_error("campaign-add: POST only");
      if($error = check_required($_POST, array('site_id', 'template','subject', 'name', 'receiver_roles', 'receiver_count', 'recipient', 'sender', 'main_message'))) json_error("Missing: ".implode(', ', $error));
      $siteObj = get_object('site', $site_id);

      $data = $_POST;
      $template = $data['template'];
      $site_id = $data['site_id'];
      
      $mail_scope = ['receiver_name' => "__MAIL_RECIPIENT", 
                     'title' => $data['title'],
                     'campaign' => 1,
                     'main_message' => $data['main_message']];
      
      $body = smarty_mail_template($site_id, $template, ['scope' => $mail_scope]);
      if(!strlen($body)) json_error("Body was empty for template {$template}");
      
      $data['body'] = $body; 

      
      if($obj_id = $_POST['obj_id']) {
        list($id, $errors) = update_object('mail_campaign', $data, $obj_id);
        $action = "updated";
      } else {
        list($id, $errors) = add_object('mail_campaign', $data);
        $action = "added";
      }
      

      if($id) {

        /** send preview mail */
        $mail_options = ["sender" => $data['sender'], "parent_type" => 'mail_campaign', "parent_id" => $id];
        $result = send_mail($site_id, $data['subject'], $body, $data['recipient'], $mail_options);
        if($result->error) json_error($result->error);
        
        $result->success = 1;
        $result->id = $id;
        $result->redirect = site_page_link($site_id, PAGE_MAIL_CAMPAIGN)."{$id}?preview";
        $result->message = "OK: $action campaign $id";        
      } elseif($errors) {
        json_error($errors);
      } else {
        $result->success = 0;
        $result->error = "$action campaign failed";
      }
      
      echo(json_encode($result));exit;
      break;
      
    case 'mail-preview':
    case 'mail-preview-send':
      if(!$_POST) json_error("send-mail: POST only");
      if($error = check_required($_POST, array('site_id' => "Site ID", 'template' => "Template" ,'subject' => "Subject", 'sender' => "Sender", 'recipient' => "Recipient"))) json_error("Missing: ".implode(', ', $error));
      $siteObj = get_object('site', $site_id);
      $obj_name = 'object'; // gets overriden by extract if set
      extract($_POST);
           
      print_log("mail-preview scope:".dump($_POST,true), 'app', LOG_LEVEL_TEST);
      
      if(isset($_POST['g-recaptcha-response'])) {
        $captcha_response = $_POST['g-recaptcha-response'];
        if(!$captcha_response) json_error(phrase('are you a robot?', CAPS));
        //$secret = config('g-recaptcha-secret');
        $secret = '6LfJXyEUAAAAAC-8-dw-SaQaAuwu8Q67qPG9EM2i';
        $response = http_post("https://www.google.com/recaptcha/api/siteverify", ['secret' => $secret, 'response' => $captcha_response, 'remoteip' => $app->remote_ip]);
        if(!$response) json_error("No re-captcha response received");
        $response = json_decode($response);
        if(!$response) json_error("Re-captcha response not JSON");
        if(!$response->success) json_error("Google thinks you're a robot");        
        //json_error("Not a robot");
      }
      
      $send_mail = $oper == 'mail-preview-send' ? 1 : 0;

        
      /** sender */
      list($email, $name) = parse_recipient($sender); // only works for single recipient (by design)
      if(!is_email($email)) json_error(phrase("%s is not a valid email address", -1, $email ?: $sender));
      if(!$name) $name = $obj_data['sender_name']; /** used on contact form, share form */
      $sender = email_recipient($email, $name);

      /** recipient */
      $recipients = strpos($recipient,',') ? explode(',', $recipient) : [$recipient]; // allow multiple
      $count = count($recipients);
      $i=1;
      foreach($recipients as $rcpt) {   
        $rcpt = trim($rcpt); // remove spaces
        list($email, $name) = parse_recipient($rcpt); // only works for single recipient (by design)
        //print_log("$i: Recipient={$rcpt} name={$name} email={$email}", 'mail', LOG_LEVEL_TEST);
        if(!is_email($email)) json_error(phrase("%s is not a valid email address", -1, $email ?: $rcpt));
        $i++;
      }
      
      list($email, $name) = parse_recipient($recipients[0]); // use first for name
      if(!$name) $name = $obj_data['receiver_name'];
      if($count == 1) $recipient = email_recipient($email, $name);
      $receiver_name = $name;
      
      //print_log("Found {$count} recipients in {$recipient} name:{$name}", 'mail', LOG_LEVEL_TEST);
      
      /** replyto */
      if(!$replyto && $replyto_sender) $replyto = $sender; 
      
      //if($receiver_name) $mail_scope->receiver_name = $receiver_name;
      
      $_POST['created'] = now(); /** kludge to make sent time in mail-message to work */
      $obj = $obj_type && $obj_id ? get_object($obj_type, $obj_id) : obj2array($_POST);      

      $mailData = ['sender_id' => $sender_id ?: $__USER->id, 
                   'obj_type' => $obj_type, 'obj_id' => $obj_id, 'object' => $obj, 
                   'sender' => $sender,
                   'recipient' => $recipient,
                   'receiver_name' => $receiver_name,
                   'main_message' => $obj_data['main_message'],
                   'personal_message' => $obj_data['personal_message']
                  ];
                     
      $mail_scope = array2obj($mailData); /** used for template, not send_mail() */
      
      if($obj_type && $obj_id) {
        if($obj = get_object($obj_type, $obj_id)) {
          if(isset($obj->client_id) && $obj->client_id != $__CLIENT_ID) {
            $mail_scope->main_message = "Error: $obj_type $obj_id not found";
            $controller = '';
            $template = 'mail/mail-generic';
          /** todo: check auth/client id */
          //} elseif(!auth_object()) {
          } else {
            if($obj_name) $mail_scope->$obj_name = $obj;            
          }
        }
        
      }

      $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => $controller]);
      $len = strlen($body);
      
      print_log("mail-preview send=$send_mail sender: {$sender} recipient: {$recipient} tpl={$template} controller={$controller} len={$len} ", 'mail', LOG_LEVEL_TEST);
      //print_log("scope:".dump($scope,true), 'mail', LOG_LEVEL_TEST);
      
      $redirect = '';
      if($send_mail) {
        
        /** add to messages */
        $messageData = $_POST;
        $parent_id = $obj_id;
        $parent_type = $obj_type;
        if($obj_type == 'message') { /** contact form */
          $parent_type = '';
          if(!strip_blank($personal_message)) json_error(phrase('your message was blank', CAPS));
          $redirect = page_link(PAGE_CONTACT)."?c=1";
          //$messageData['email_address'] = $email_address;  
        } else {
          //$messageData['email_address'] = $from;  
        }
        
        //$messageData['message'] = $main_message.$personal_message;
        $messageData['parent_id'] = $parent_id;
        $messageData['parent_type'] = $parent_type;
        $messageData['sender'] = $sender;
        $messageData['recipient'] = $recipient;
        list($message_id, $merrors) = add_object('message', $messageData);
        if($merrors) json_error($merrors);
        if($obj_type == 'message') $obj_id = $message_id; /** stored in mail_queue */
        
        $mail_options = ['parent_type' => $obj_type, 'parent_id' => $obj_id, 
                         'sender_id' => $sender_id ?: $__USER->id, 'sender' => $sender, 'replyto' => $replyto, 
                         'attachment' => $attachment, 'attachment_name' => $attachment_name];
        $result = send_mail($site_id, $subject, $body, $recipient, $mail_options);
          
        if($result->success) {
          $recipient = $result->recipient;
          $subject = $result->subject;
          $result->message = "OK: Sent mail \"$subject\"";
          unset($result->body);
          if(!$redirect) $redirect = page_link(PAGE_MAIL)."?uid={$result->guid}";
          $result->redirect = $redirect; 
          
          
        } else {
          $scope->error = "Oops, something went wrong.<br>".$result->error;
        }
      } else {
        $result->success = 1;
        $result->obj_type = $obj_type;
        $result->obj_id = $obj_id;
        $result->object = $obj;
        $result->sender = $sender;
        $result->replyto = $replyto;
        $result->recipient = $recipient;
        $result->subject = $subject;
        $result->body = $body;
      }
      
      // $result->data = ['body' => $body];
      echo(json_encode($result));exit;
        
      break;

    /** no longer in use */
    case 'mail-send':
    case 'send-mail':
      if(!$_POST) json_error("send-mail: POST only");
      if($error = check_required($_POST, array('site_id', 'template','subject'))) json_error("Missing: ".implode(', ', $error));
      $siteObj = get_object('site', $site_id);
      extract($_POST);
      if($recipient) {
        if(strpos($recipient,',')) {
          $emails = explode(',', $recipient);
          foreach($emails as $email) if(!is_email($email)) json_error("{$email} is not a valid email address");
        } elseif (!is_email($recipient)) {
          json_error("$recipient is not a valid email address");
        }
      }
      if($obj_type && $obj_id) {
        $obj = get_object($obj_type, $obj_id);
        if(!$obj) json_error("$obj_type $obj_id not found");
        if($message) {
          $obj->message = $peronsal_message; /** insert custom message */
        }
      }
      //if(!$obj) json_error("no object");
      $user_level = $obj_data['user_level'] ?: USER_LEVEL_GUEST;
      $language = $obj_data['language'] ?: '';
      $param1 = $obj_data['param1'] ?: '';
      $param2 = $obj_data['param2'] ?: '';
      
      $receiver_name = $name; // default
    
      /** side effects - a bit kludgy but... */
      switch($action) {
      case 'send_invoice':      
      case 'invoice':
        $inv_data = array();
        if($obj->status == INV_STATUS_OPEN) {
          $inv_data['status'] = INV_STATUS_SENT;
          $inv_data['locked'] = 1;
          $inv_data['duedate'] = sql_add_days(today(), 30); // due in 30 days unless otherwise set		
          $inv_data['invoice_date'] = now(); 		
          list($inv_id, $inv_errors) = update_object('invoice', $inv_data, $obj_id);
          if($inv_errors) json_error($inv_errors);
        }
        break;
        
        
      default:
        break;
      }
      
      
      //list($subject, $body, $from, $to, $replyto) = mail_template($template, $obj_type, $obj, $action, $user_level, $language, $param1, $param2);
      $mail_scope = ['obj_type' => $obj_type, 'obj_id' => $obj_id, 'object' => $obj, 'controller' => $controller, 
                     'main_message' => $obj_data['main_message'] ?: "You message from contact form.", 'personal_message' => $obj_data['personal_message']];
      print_log("send-mail: template:{$template} controller={$controller} scope: ".dump($mail_scope, true), 'mail', LOG_LEVEL_TEST);
      if($mscope = $_POST['mail_scope']) {
        $mail_scope = object_merge($mail_scope, json_decode($mscope));
      }

      $len = strlen($body);      
      $body = smarty_mail_template($siteObj, $template, ['scope' => $mail_scope, 'controller' => $controller]);      
      print_log("mail-preview controller={$controller} len={$len} scope:".dump($mail_scope,true), 'mail', LOG_LEVEL_TEST);
      
      //json_error("action=$action template=$template controller=$controller site: $site_id len".strlen($body));
      if($subject === false) {
        json_error("Template $template not found");
      }
      
      if($recipient = $_POST['recipient']) $to = $recipient;
      if($sender = $_POST['email_address']) $from = $replyto = $sender;
      $s = $_POST['subject'];
      if($s && !$subject) $subject = $s;
      
      //json_error("Sending mail with subject=$subject from $from to $to (replyto: $replyto)");
      
      //json_error(dump($_POST, true));
      if($recipient) {
        $to = $recipient;
        if($receiver_name) $to = "$receiver_name <$recipient>";
      }
      
      $mail_options = ['parent_type' => $obj_type, 'parent_id' => $obj_id, 'sender' => $from, 'replyto' => $replyto, 'attachment' => $attachment, 'attachment_name' => $attachment_name];
      $result = send_mail($site_id, $subject, $body, $to, $mail_options);
      
      if($result->success) {
        $recipient = $result->recipient;
        $subject = $result->subject;
        $result->message = "OK: Sent mail to $recipient: $subject";
        
      } else {
        $scope->error = "Oops, something went wrong.<br>".$result->error;
      }
      echo(json_encode($result));exit;
      break;
      
    case 'property-print':
      //extract($_GET);      
      
      $site_id = $_GET['site_id'];
      $ptype = is_property_type($_GET['t']) ? $_GET['t'] : $_SESSION['property_type'];
      if(!is_property_type($ptype)) $ptype = 'shortstay';
      $id_list = $_GET['id_list'] ?: '';
      //json_error("t=".$_GET['t']." sp=".$_SESSION['property_type'].dump($_GET, true));
      
      if(!$site_id) json_error("Missing site id");
      if(!$ptype) json_error("Missing property type");
      if($id_list) {
        $ids = explode(',', $id_list);
        $output = property_list_simple($site_id, $ptype, $ids, array());
      } else {
        $vars = $_GET;
        $tips = rental_search_tips($vars) ?: array();
  
        // begin output
        $output = html_h3("We're sorry...");
        $output .= html_div("We could not find any properties that matched your search", "bold");  
        if(count($tips)) {
          $output .= html_h4(phrase('tips', CAPITALIZE));
          $output .= implode(html_break(), $tips);        
        }
        $output .= dump($_GET, true);
        
      }
      echo($output);exit;
      break;
      
    case 'golf-fetch': // fetch golf course details from Golf Switch
      require_once('golfswitch.inc');
      $site_id = $_GET['site_id'];
      $golf_id = $_GET['golf_id'];
      $fid = get_object('golf_course', $golf_id, 'fid');
      
      if(!$site_id) json_error("Missing site id");
      if(!$golf_id) json_error("Missing golf course id");
      if(!$fid) json_error("Missing golf course fid");
  
      list($client, $hdr) = gsSoapClient();
      $res = gsCourseInfo($client, $hdr, $fid);
      $course = $res->CourseInfoResult->Course;
      list($id, $errors) = gsAddCourseInfo($course);
      if(!$id || $errors) json_error("Failed to add course $course->nm: ".$errors[0]);
      if($id) $result = json_success_object("OK, fetched golf course $golf_id");
      echo(json_encode($result));exit;
      break;
  
    case 'golf-print': // print golf course details
      $site_id = $_GET['site_id'];
      $golf_id = $_GET['golf_id'];
      if(!$site_id) json_error("Missing site id");
      if(!$golf_id) json_error("Missing golf course id");
      $output = golf_course_content($site_id, $golf_id);
      echo($output);exit;
      break;
  
     /**
    case 'property-searchbar': // process input from searchbar
      extract($obj_data);
      $errors = $messages = array();
      $t = is_property_type($t) ? $t : $_SESSION['property_type'];
      if(!is_property_type($t)) $t = 'shortstay'; // fallback                      
      if(!$t) json_error(phrase('missing_property_type')); //.dump($obj_data, true));
  
      if(!($loc || $center || $bounds)) json_error(phrase('please_tell_us_where_you_are_going', CAPITALIZE));
      
      //if(!$in || !$out) json_error(phrase('please_tell_us_when_you_are_going', CAPITALIZE));
      
      $in = date2sql($in);
      $out = date2sql($out);
      
      if($in && $out) {
        if($in >= $out) json_error(phrase('checkin', CAPITALIZE)." ".phrase('must_be_before')." ".phrase('checkout'));
        $parsed_in = strtotime($in_disp ?: $in);
        $parsed_out = strtotime($out_disp ?: $out);    
        if(!$parsed_in) json_error("$in_disp ".phrase('is_not_a_valid_date'));
        if(!$parsed_out) json_error("$out_disp ".phrase('is_not_a_valid_date'));
      }
          
      if($loc && !$center) {      
        if($locObj = find_object('location', array('name' => "$loc"))) $location_id = $locObj->id; // exact search
        else if($locObj = find_object('location', array('name' => "$loc*"))) $location_id = $locObj->id; // begins with
        else if($locObj = find_object('location', array('name' => "*$loc*"))) $location_id = $locObj->id; // includes
        if(!$location_id) json_error(phrase('sorry_we_could_not_find', CAPITALIZE). ' "'.$loc.'"');
        $loc = $locObj->name;
        $obj_data['location_id'] = $location_id;
      }
      
      $page_type = property_type2page($t);
                            
      
      $response = new stdClass;
      if($errors) json_error($errors);
      $response->success = true;
      
      // here we could unset in_disp and out_disp for brevity
      //$unset = array('in_disp','out_disp','location_id',;
      $unset = array('location_id','location_lat','location_long');
      foreach($unset as $f) unset($obj_data[$f]); 
                   
      
      $redirect = $page_type ? get_page_link($page_type) : '';    
      if(!$redirect) json_error("No page found for property type $t");
      $response->redirect = $redirect."?".array2qs($obj_data)."&search=1";
      
      // if($redirect = $_GET['redirect']) $response->redirect = $redirect."?".array2qs($obj_data)."&search=1";
      
      echo(json_encode($response));exit;
      break;
     */
     
    case 'grid-select':
      $format = $inputVars['format']; // what format should we return
      $key        = get_object_key($obj_type);
      $table      = get_object_table($obj_type);
      $classFlags = get_object_flags($obj_type);
      $descriptor = get_object_descriptor($obj_type);
    
      // select box
      // special case: select (data for jqGrid select box, dynamically loaded with Ajax)
      $select_filters = urldecode($inputVars['select_filters']);
      $select_sort    = pick_first(urldecode($inputVars['select_sort']), $descriptor);
  
      $eos = array("0 : [Select $obj_type]");
      $options[0] = "[Select $obj_type]";
      
      $sql_descriptor = strpos($descriptor, ',') ? $descriptor : "$table.$descriptor";
      $sql_fields = explode(',', $sql_descriptor);
      $selected_fields = array_merge(array("$table.$key"), $sql_fields); // $sql_descriptor";
  
      $objs = get_objects($obj_type, $select_filters, array('sort' => $select_sort, 'fields' => $selected_fields));
      foreach($objs as $obj) {
        $value =  strpos($descriptor, ',') ? get_object_description($obj_type, $obj) : $obj->$descriptor;
        $eos[] = $obj->$key.':'.$value;
        $options[$obj->$key] = $value; 
      }
      echo($format == 'select' ? json_encode($options) : implode(';', $eos));exit();
      break;
  
    // new bounds based property search (air template)
    /**
    case 'location-search':
      $location_id = $_GET['id'];
      $name = $_GET['name'];
      
      if(!$id) json_error("Missing location id");
      $locationObj = get_object('location', $id);
      if(!$locationObj) json_error("Could not find ".($name ?: "location $id"));
      $bounds_json = $locationObj->location_bounds;
      if(!$bounds_json || !json_decode($bounds_json)) json_error("Missing or invalid bounds $bounds_json");
      $result->data = $bounds_json;
      $result->success = true;
      echo(json_encode($result)); exit;
      break;
    */
      
    case 'calendar-import': // import calendar from remote soure
      //json_error(dump($obj_data,true)); exit;
      require_once("xml/xml_airbnb.inc");
      
      if($error = check_required($obj_data, array("user_id" => "User id", "source_id" => "Source ID"))) json_error("Missing: ".implode(', ', $error));
  
      extract($obj_data);
      
      $importObj = get_object('calendar_import', $source_id);
      if(!$importObj) json_error("Source not found");
      
      $apt_id = $importObj->apt_id; 
      if(!$apt_id) json_error("Source has no rental ID");    
      $url = $importObj->url;
      if(!is_url($url)) json_error("$url is not valid");
      
      $fname = $importObj->fname;
      $map = $fname ? array($fname => $apt_id) : array(); 
      $result = airbnb_import($apt_id, $url, $map);
      
      //$result->message = "OK";
      //$result->success = true;
      //$result->data = $obj_data;
      if($result->success) update_object("calendar_import", array("last_sync" => now()), $source_id);
      echo(json_encode($result)); exit;
      break;
      
    /** limit to client_id and calendar host_id list */
    case 'calendar-search':
      if(!$u_id) json_error("Not logged in");
      if($u_level < USER_LEVEL_OWNER) json_error("Unauthorized");
      $userObj = get_object('user', $u_id);
      if($u_id == SYSTEM_USER_ID) { // special case, show all
        $host_ids = find_objects("property", array("type" => $type), array("field" => "distinct host_id"));
      } else {
        $host_ids = user_host_list($userObj);
        $host_ids = find_objects("property", array("host_id" => $host_ids), array("field" => "distinct host_id"));
      }
  
      $val = trim($_GET['q']);
      $type = $_GET['obj_type'] ?: 'reservation';
      $results = array();
  
      $calendar_link = page_link(PAGE_CALENDAR);
      $reservation_link = page_link(PAGE_STATUS);
      $invoice_link = page_link(PAGE_INVOICE);
  
      $res_fields = array("'$reservation_link' as link", "'reservation' as obj_type", 'res_id as id', 'apt_id', 'apartment_name', 'SUBSTRING(checkin,1,10) as start', 'SUBSTRING(checkout,1,10) as end', 'status', "TRIM(CONCAT_WS(' ',salutation,first_name,last_name)) as name");
      $resSearchOptions = array("fields" => $res_fields);
      $inv_fields = array("'$invoice_link' as link", "'invoice' as obj_type", 'invoice.parent_type', 'invoice.parent_id', 'invoice.id', 'invoice_id', 'SUBSTRING(start_date,1,10) as start', 'SUBSTRING(end_date,1,10) as end', 'status', "TRIM(CONCAT_WS(' ',salutation,first_name,last_name)) as name");
      $invSearchOptions = array("fields" => $inv_fields);
      
      if(isint($val)) { // id
        if($resObj = get_object('reservation', $val, $res_fields)) $results[] = $resObj;
        
        /** todo: limit to relevant invoices (host_id + client_id) */
        if($invObj = get_object('invoice', $val, $inv_fields)) $results[] = $invObj;
        if($invObjs = find_objects('invoice', array('invoice_id' => $val), $invSearchOptions)) $results = array_merge($results, $invObjs);
      } else {
        $search = array('host_id' => $host_ids);
        $status = $_GET['status'] ?: 1; // by default, don't show status < 1
        $search['status'] = array('>=', $status);
        
        if($client_id = $cal_params->filters['client_id']) $search['client_id'] = $client_id;
        if($property_type = $cal_params->filters['property_type']) $search['property_type'] = $property_type;
        if($city_id = $cal_params->filters['cid']) $search['city_id'] = $city_id;
  
        //$debug = dump($search, true);
  
        $search[DB_LOGICAL_OR] = array('first_name' => "*$val*", 'last_name' => "*$val*");
        if($objs = find_objects('reservation', $search, $resSearchOptions)) $results = array_merge($results, $objs);
  
        /**
        $search[DB_LOGICAL_OR] = array('first_name' => "*$val*", 'last_name' => "*$val*", 'invoice_id' => "*$val*");        
        if($objs = find_objects('invoice', $search, $invSearchOptions)) $results[] = array_merge($results, $objs);
        */
           
      }
      if(isset($_GET['dump'])) dump($results);
      $result->hits = count($results);
      $result->data = $results;
      $result->success = true;
      echo(json_encode($result)); exit;
      break;


    case 'rental-search':
    case 'property-search':
      
      //timer('property-search', "start", true);
      $result = rental_search($obj_data);
      $data = $result->data ?: new stdClass;
      $data->rental_search = 1;
      $rentals = $data->rentals ?: [];
      $count = count($rentals);
      if(count($rentals)) {
        $data->filters = $filters = rental_search_filters($obj_data);
      } else {
        $data->tips = $tips = rental_search_tips($obj_data);
        print_log("app-data count={$count} tips:".dump($tips, true), 'property-search', LOG_LEVEL_TEST);
      }
      $result->language = $__LANGUAGE;
      
      echo json_encode($result);exit();          
      break;
      
    case 'rest-api-call':
      $messages = $errors = array();
      $url = $obj_data['url'] ?: json_error("Missing API URL");
      $api_method = $obj_data['api_method'] ?: json_error("Missing API method");
      $http_method = $obj_data['http_method'] ?: "GET";
      $send_data = $obj_data['data']; // ? (array) json_decode($obj_data['data']) : array();
      $api_response = rest_api_call($url, $api_method, $send_data, array("http_method" => $http_method, 'username' => $obj_data['username'], 'password' => $obj_data['password']));
      $len = strlen($api_response);
      $messages[] = "Received $len bytes from $url".$api_method;
      $response->success = 1;
      if($api_response) {
        if(isJson($api_response)) {
          $response->data = json_decode($api_response); // json data
        } else {
          $messages[] = $api_response;
          $response->data = array("rest-response" => $api_response);
        }
     }
  
      $response->message = $messages; 
      echo(json_encode($response));exit();    
      break;
      
    case 'add-page':
      require_once('db_withdraw.inc');
      if($error = check_required($obj_data, array('name' => "Name",'user_id' => "User ID", "site_id" => "Site ID"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      list($id, $errors) = add_object('site_page', $obj_data);
      if(!$id || $errors) json_error($errors ?: "Unknown error: No ID received");
      $pageObj = $response->data = get_page($id);
      $response->success = 1;
      $response->redirect = $pageObj->path;
      $response->message = "Added $obj_type $id";
      echo(json_encode($response));exit();        
  
    break;
    
    case 'add-payout':
      require_once('db_withdraw.inc');
      if($error = check_required($obj_data, array('payment_ids' => "Payment IDs", 'host_id' => "Host ID", 'currency' => "Currency", 'amount' => "Amount", 'user_id' => "User ID", "site_id" => "Site ID"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      
      //json_error("Sending $currency $amount to $host_id: $payment_ids");
      
      list($errors, $messages) = withdraw_multiple_payments($host_id, $currency, explode(',', $payment_ids));
      if($errors) json_error($errors);
      $response->success = 1;
      $response->message = $messages;
      echo(json_encode($response));exit();        
    break;
    
    case 'add-payment':
      
      if($error = check_required($obj_data, array('payment_type' => "Payment Type", 'currency' => "Currency", 'amount' => "Amount", 'description' => 'Description', 'user_id' => "User ID", "site_id" => "Site ID"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      switch($payment_type) {
      case PAYMENT_TYPE_TRANSFER:
        $payment_type = "transfer";
        if(!$sender_id) json_error("Missing sender ID");
        if(!$recipient_id) json_error("Missing recipient ID");      
        if(!($amount > 0)) json_error("Amount must be positive");
        $obj_data['host_id'] = $recipient_id;
        break;
      case PAYMENT_TYPE_CORRECTION:
        $payment_type = "correction";
        if(!$host_id) json_error("Missing Host ID");
        break;
      default:
        json_error("Illegal payment type $payment_type");
        break;
      }
      $obj_data['payment_currency'] = $currency;
      $obj_data['payment_fee'] = 0;
      $obj_data['payment_gross'] = $obj_data['payment_net'] = $amount;
      $obj_data['payment_status'] = PAYMENT_STATUS_COMPLETED;
      
      list($id, $errors) = add_object('payment', $obj_data);
      if($errors) json_error($errors);
      $response->success = 1;
      $response->message = "Added $payment_type $id";
      echo(json_encode($response));exit();        
      break;

    /** more secure version of 'list' below, with implicit filters and security checks, used by bootstrap-table */
    case 'get-rows':
      extract($obj_data);
      $allowed_types = ['invoice', 'inv_lineitem', 'payment', 'withdrawal', 'user', 'shortstay', 'dictionary', 'reservation', 
                        'artwork', 'gallery', 'media_collection', 'media', 'media_folder' ,'plugin', 'minisite', 'uhuru_registration', 'uhuru_trial_lesson'];
      if(!in_array($obj_type, $allowed_types)) json_error("Listing $obj_type not supported");
      $options = array();
      foreach(array('offset', 'limit', 'sort', 'order', 'fields') as $fld) $options[$fld] = $obj_data[$fld];
      
      $search = array();
      // $fields = database_fields($obj_type);
      global $u_id, $u_level;
      
      $tbl = get_object_table($obj_type);
      
      if(in_array('client_id', $class_fields)) $search["{$tbl}.client_id"] = $_SESSION['client_id'];;
      if($u_level < USER_LEVEL_ADMIN && in_array('host_id', $class_fields)) $search["{$tbl}.host_id"] = user_host_list($userObj);
  
        
      $val = trim($obj_data['search']);
      $filter_data = trim($obj_data['filter']);
      
      $scope->searchvalue = $val;
      $_SESSION['searchvalue']=$val; 

      $search_options = $options;
      if($_GET['filters']) $search_options['filters'] = $_GET['filters'];
      if($obj_data['id']) $search_options['id'] = $obj_data['id'];
      
      $vals = explode(' ', $val);
      $objs = [];
      
      if(count($vals) > 1) {
        $search_options['associative'] = 1; /** return assoc. array */
        $assocArrays = [];
        foreach($vals as $v) { 

          if(!$filter_data)         
            $assocArrays[] = object_site_search($obj_type, $search, $v, $search_options);
          else
            $assocArrays[] = object_site_filter($obj_type, $search, $v, $search_options, $filter_data);
        }
        
        $objs = [];
        $result = call_user_func_array('array_intersect_key', $assocArrays);
        foreach($result as $id => $obj) $objs[] = $obj;
      } else {   
        if(!$filter_data){
          $objs = object_site_search($obj_type, $search, $val, $search_options);
          if(count($objs)>1){
            $extra_count = $objs[1];
            $objs = $objs[0];
          }
        }else{
          $objs = object_site_filter($obj_type, $search, $val, $search_options, $filter_data);
        }
      }

      if($mode = $obj_data['mode']) {  //TODO need to update the softdelete column using the ids  
       delete_objects($obj_type,$id); // need to update the softdelete db_object.inc file
       unset($id);
      }
      
      if($obj_data['return'] == 'rows') { // return array, not object (required when using javascript URL instead of data-url, for some reason */
        echo(json_encode($objs));exit;
      }
      //$response->total = intVal(count_objects($obj_type, $search));
  
      // get count and totals
      $terms = array();
      $terms[] = "count(*) as total_count";
      $attributes = get_object_attributes($obj_type, false);
      foreach($attributes as $field => $attr) {
        $flags = $attr[ATT_FLAGS];
        if($flags & AF_TOTAL) {
          $terms[] = "sum($field) as $field";
        }
      }

      $totalRows = find_objects($obj_type, $search, array('fields' => $terms));
      $totalRow = $totalRows[0];
      $response->total = $totalRow->total_count + $extra_count;
      unset($totalRow->total_count);
      
      $sums = array();
      foreach($totalRow as $k => $v) {
        $sums[$k] = $v;
      }
      if(count($sums)) $response->totals = $sums;

      $response->rows = $objs;
      echo(json_encode($response, JSON_PRETTY_PRINT));exit;
      
      break;

    case 'get-vault':
      extract($obj_data);
      
      if($obj_data[parent_id]){
        $search = ['user_id'=>$obj_data[user_id], 'folder_id'=>$obj_data[parent_id] ,'active'=>1];
        $data = find_objects($obj_type, $search );
      }else{
        $media = find_objects($obj_type, array('user_id'=>$obj_data[user_id], 'active'=>1, 'folder_id'=> 0) );
        $folder = find_objects('media_folder', array('user_id'=>$obj_data[user_id], 'active'=>1), ['sort' => 'created DESC'] );
        $data = array_merge($folder,$media);
      }
      $response->rows  = $data;
      echo(json_encode($response));exit;
    break;
     
    case 'list':
      if($u_level < USER_LEVEL_STAFF) json_error("list is disabled for non-admins");
      
      $return  = $obj_data['return'];
      $start  = $obj_data['start'] ?: $obj_data['offset'];
      $limit  = $obj_data['limit'];
      $search = $obj_data['search'];
      $sort   = $obj_data['sort'];
      $fields = $obj_data['fields'];
      $order = $obj_data['order'];
      $format = $obj_data['format'];
      
      if($search == "post") { // used obj-data to build search
        foreach(array('obj_type', 'oper', 'format', 'start', 'limit', 'search', 'sort', 'order', 'fields') as $fld) unset($obj_data[$fld]);
        foreach($obj_data as $k => $v) if(!strlen($v)) unset($obj_data[$k]); // remove blanks, but not zero
        if($id) $obj_data['id'] = $id; // special case, this is unset near top
        $search = $obj_data;
      } else {
        $search = isJson($search) ? json_decode($search) : $search;
      }
      
      $fields = isJson($fields) ? json_decode($fields) : $fields;    
      $options = array('sort' => $sort, 'order' => $order, 'start' => $start, 'limit' => $limit, 'fields' => $fields, 'format' => $format);
      $objs = find_objects($obj_type, $search, $options);
  
      if($return == 'rows') {
        $response = $objs; // e.g. bootstrap table just wants the rows
      } else {
        //$response->options = $options;
        $response->success = 1;
        $response->total = count_objects($obj_type, $search);
        $response->rows = $objs;
      }
      echo(json_encode($response));exit;
      break;
      
    // generic operation for calling any PHP function
    // todo: security; limit to allowed list
    case 'load-function':
      $result = call_function($_GET['function'], $_GET);
      echo($result);exit();        
      break;
  
    case 'load-function-json':
      $function = $_GET['function'];
      $allowed_functions = array('gallery_user_list', 'gallery_collection_list', 'gallery_folder_list', 'gallery_collection', 'gallery_folder');
      // pretty dangerous, make list of allowed functions depending on user level
      switch($u_level) {
      case USER_LEVEL_SYSTEM:
        break;
      default:
        break;
      }
      if(!in_array($function, $allowed_functions)) json_error("Function $function not in allowed list");
      $result = call_function($function, $_GET);
      echo(json_encode($result));exit();
      break;
      
    case 'cart-remove':
      global $u_id;
      extract($_GET);
      $messages = $errors = array();
      //$messages = dump($_GET);
      if(!$product_id) json_error("No product id");
      if(!$site_id) json_error("Missing site_id");
      $siteObj = get_object('site', $site_id);
      $user_id = $user_id ?: $u_id;
  
      $cartObj = get_cart($siteObj); // get the cart from session or DB
      if(!$cartObj) json_error("Failed to get cart");
  
      $rowObj = $cartObj->items[$product_id];
      // remove item from db
      if($row_id = $row_id ?: $rowObj->id) {
        list($count, $errors) = delete_object('cart_row', $row_id);
        if($errors) json_error($errors);
        //$messages[] = "Delete row $row_id from db";
        //if(!$count) json_error("Failed to delete row");
      }
  
      $product_name = $rowObj->name ?: get_object('product', $product_id, 'name');
  
      // remove product from cart
      unset($cartObj->items[$product_id]);
      $cartObj = cart_update($cartObj); // sets count and total
      $cartObj = save_cart($siteObj, $user_id, $cartObj); 
  
      $cart_link = $options['cart_link'] ?: site_page_link($siteObj, PAGE_CART);
      $cart_link_str = html_link($cart_link, phrase('cart'));
      $messages[] = "Removed $product_name from $cart_link_str.";
     
      $rows = cart_rows($siteObj, $cartObj, array('cart_link' => $cart_link));
      $cartObj->items_html = html_table($rows, array('class' => 'cart_table', 'header' => 0, 'footer' => 1));
  
      if(count($messages)) $result->message = print_messages($messages);
      if(count($errors)) $result->error = print_errors($errors);
      $result->cart = $cartObj;
  
      $_SESSION['cart'] = $cartObj; 
      echo(json_encode($result));exit();
      
      break;
  
    case 'cart-row-update':
      extract($_GET);
      global $u_id;
      $messages = $errors = array();
      //$messages = dump($_GET);
      if(!$product_id) json_error("No product id");
      if(!$site_id) json_error("Missing site_id");
      $user_id = $user_id ?: $u_id;
      $siteObj = get_object('site', $site_id);
      $productObj = get_object('product', $product_id);
      
      $cartObj = get_cart($siteObj); // get the cart from session or DB
      if(!$cartObj) json_error("Failed to get cart");
  
      $rowObj = $cartObj->items[$product_id];
  
      if($variant_id) {
        $variantObj = get_object('product_variant', $variant_id);
        $name = get_variant_name($variantObj, $productObj);
        $price = $variantObj->price;
      } else {
        $name = $productObj->name;
        $price = $productObj->price;
        $productObj->variant_id = 0;
      }
  
      $rowObj->variant_id = $variant_id ?: 0;
      $rowObj->name = $name;
      $rowObj->amount = $price;
      $rowObj->quantity = $quantity ?: 1;
      
      // update db row
      if($row_id = $row_id ?: $rowObj->id) {
        list($count, $errors) = update_object('cart_row', (array) $rowObj, $row_id);
        $rowObj = get_object('cart_row', $row_id); // reload
        if($errors) json_error($errors);
        $messages[] = "Updated row $row_id in db";
      } else { // Session cart
      }
  
      $cartObj->items[$product_id] = $rowObj;
      $cartObj = cart_update($cartObj); // sets count and total
      $cartObj = save_cart($siteObj, $user_id, $cartObj); // save cart
         
      $rows = cart_rows($siteObj, $cartObj, array('cart_link' => $cart_link));
      $cartObj->items_html = html_table($rows, array('class' => 'cart_table', 'header' => 0, 'footer' => 1));
  
      if(count($messages)) $result->message = print_messages($messages);
      if(count($errors)) $result->error = print_errors($errors);
      $result->cart = $cartObj;
  
      $_SESSION['cart'] = $cartObj; 
      echo(json_encode($result));exit();
      
      break;
  
  // new: 1 step payment (chained): check, get, process
  /**
    case 'rental-payment':
      extract($obj_data);
      if($cc_id) {
        $ccObj = get_object('cc', $cc_id);
        $cvc = $cc_cvv2 ?: $inputVars['cvc-'.$cc_id];
        if(!$cvc) json_error("CVC code requied");
        // if($cvc != $ccObj->cc_cvv2) json_error("The CVC code does not match the card on file");
        $last_four = substr($ccObj->cc_number, strlen($ccObj->cc_number)-4, 4);
        $cc_type = $ccObj->cc_type;
        $obj_data['ccof'] = "$cc_type-$cc_id-$last_four";
        //json_error("got here".dump($obj_data,true));
      } else { // new card                   
        if($cc_exp) $obj_data = cc_fix_input($obj_data);      
      }
      
      if(0) {
        $result->error = dump($obj_data, true);
        echo(json_encode($result));exit();
        
      }
      $result = check_payment($obj_data);
      // if we get here, no error (check_payment returns error object)
      
      // dump($result);dump($obj_data);exit();
      $result->update_form = 1; // update the form with return data
      $result->operation = 'get-payment';    
      
      // add guest message to reservation (which will add chat object in post_edit_reservation)
      $msg = strip_blank($obj_data['extra_info']);
      $pt = $obj_data['parent_type'];
      if($pt == 'reservation' && $msg) {
         $res_id = $obj_data['parent_id'];
         //json_error("Ok: message: $pt=$parent_type id=$res_id msg=$msg");
         list($id, $errors) = update_object('reservation', array('extra_info' => $obj_data['extra_info']), $res_id);
      } else {
        //json_error("No message: $pt=$parent_type msg=$msg");
      }
      $ts = now();    
      print_log("$ts: rental-payment: ".dump($result, true), 'payment', LOG_LEVEL_ALWAYS);        
      
      echo(json_encode($result));exit();
      break;
      
  
  // 3 step payment (chained): check, get, process    
    case 'check-payment':
      //json_message(dump($obj_data,true));
      if($obj_data['cc_exp']) $obj_data = cc_fix_input($obj_data); // get exp month/year + cc type  
      //json_error("data=".dump($obj_data,true));
      $result = check_payment($obj_data);
      $result->update_form = 1; // update the form with return data
      $result->operation = 'get-payment';
      $ts = now();    
      print_log("$ts: check-payment: ".dump($result, true), 'payment', LOG_LEVEL_ALWAYS);        
      
      echo(json_encode($result));exit();
      break;
  
    case 'get-payment':
      //json_error("get-payment obj-data=".dump($obj_data, true));
      $result = get_payment($obj_data);
      $result->update_form = 1; // update the form with return data
      //$result->operation = 'process-payment';
  
      $ts = now();    
      print_log("$ts: get-payment: ".dump($result, true), 'payment', LOG_LEVEL_ALWAYS);        
  
      if($result->success) {
        $data = $result->data;
        $result = process_payment($data);
        print_log("$ts: process_payment: ".dump($result, true), 'payment', LOG_LEVEL_ALWAYS);        
        $result->reload = 1;
      }
      echo(json_encode($result));exit();    
      break;
  */
  
    /**
    case 'process-payment':
      $result = process_payment($obj_data);
      if($result->success) {
        //if($result->status_link) $result->redirect = $result->status_link;
        if($redirect = $obj_data['redirect']) $result->redirect = $redirect;
      }
        
      echo(json_encode($result));exit();    
      break;
    */
  
  case 'verify-payment':
    if($error = check_required($obj_data, array('user_id' => "User ID", "payment_id" => "ID", "site_id" => "Site ID")))
      die(print_alert("Missing: ".implode(', ', $error), 'error'));
  
    extract($obj_data);
    if(!auth_action('payment', 'view', $payment_id)) die("Unauthorized");
    $paymentObj = get_object('payment', $payment_id);
    if(!$paymentObj) die("Invalid payment");
    
    $gateway_id = $paymentObj->gateway_id;
    $txn_id = $paymentObj->txn_id;
    if(!$txn_id) die("Missing transaction ID");
    $siteObj = get_object('site', $site_id);
    if(!$siteObj) die("Invalid site");
    
    switch($gateway_id) {
    case GATEWAY_MOLLIE:
      $response = mollie_payment($siteObj, $txn_id);
      break;
    case GATEWAY_PAYPAL:
      //die("txn=$txn_id");
      $response = paypal_payment($siteObj, $txn_id);
      break;
    default: 
      die("Unsupported or missing gateway");
    }
    $success = $response->success;  
    if(!$success) die(print_alert("Error: ".$response->error, 'error'));
    $output = '';
    $keys = array(
      'mode' => 'Mode', 
      'id' => 'Transaction ID',
      'currency' => 'Currency', 
      'amount' => 'Amount', 
      'fee' => 'Fee', // paypal
      'tax' => 'Tax', // paypal
      'method' => 'Method', 
      'status' => 'Status', 
      'createdDatetime' => 'Created', 
      'paidDatetime' => 'Paid',
      'receiver' => 'Receiver', // paypal
      'receiver_email' => 'Receiver email', // paypal
      'description' => 'Description', 
    );
    $meta_keys = array('inv_id' => "Invoice ID", 'res_id' => "Booking ID", 'site_id' => "Site ID", 'user_id' => "User ID", );
    $detail_keys = array(
      'bankName' => "Bank Name", 
      'bankAccount' => "Bank Account", 
      'transferReference' => "Transfer Reference", 
      'consumerName' => "Customer Name", 
      'consumerAccount' => "Customer Account", 
      'consumerBic' => "Customer BIC", 
      'bitcoinAddress' => "Bitcoin Address", 
      'bitcoinAmount' => "Bitcoin Amount", 
      'bitcoinRate' => "Bitcoin Rate", 
      'bitcoinUri' => "Bitcoin URI",
      'paypalReference' => "PayPal Reference"
    );
  
    $rows = array();
    if($data = $response->data) {
      $meta = $data->metadata;
      $details = $data->details;    
      foreach($keys as $key => $name) if(isset($data->$key) && strlen($data->$key)) $rows[] = array(html_b($name), $data->$key); 
      foreach($detail_keys as $key => $name) if(isset($details->$key) && strlen($details->$key)) $rows[] = array(html_b($name), $details->$key); 
      foreach($meta_keys as $key => $name) if(isset($meta->$key) && strlen($meta->$key)) $rows[] = array(html_b($name), $meta->$key); 
      $output = html_table($rows, array('header' => false, 'caption' => 'Live response from gateway', 'class' => 'table table-bordered table-striped table-booking-history'));
    } else {
      die("Missing data");
    }
    
    echo($output);
    //echo(dump($rows, true));
    //echo(dump($response, true));
    die();
    break;
    
  case 'refund-payment':
    if($error = check_required($obj_data, array('user_id' => "User ID", "payment_id" => "ID", "site_id" => "Site ID", 'currency' => "Currency", 'amount' => "Amount"))) 
      json_error("Missing: ".implode(', ', $error));
    $payment_id = $obj_data['payment_id'];
    //$siteObj = get_object('site', $site_id);
    $paymentObj = get_object('payment', $payment_id);
    if(!$paymentObj) json_error("Payment not found");
    $txn_id = $paymentObj->txn_id;
    if(!$txn_id) json_error("Payment data missing"); /** deliberately vague */
    $txObj = find_object('transaction', array('txn_id' => $txn_id, 'payment_id' => $payment_id, 'payment_status' => PAYMENT_STATUS_COMPLETED));
    if(!$txObj) json_error("Transaction not found"); /** deliberately vague */
    $tx_gateway = $txObj->tx_gateway;
    $payment_type = $paymentObj->payment_type;
    if(!get_object('payment_type', $payment_type, 'refundable')) {
      $payment_type_name = get_object('payment_type', $payment_type, 'name');
      json_error("$payment_type_name payments cannot be refunded");
    }
    
    $tx_currency = $txObj->mc_currency;
    $max_amount = $txObj->mc_gross;
    
    $currency = $paymentObj->currency;
    $amount = $obj_data['amount'];
    
    if($tx_currency != $currency) json_error("Currency mismatch tx: $tx_currency payment: $currency");
    if($amount > $max_amount) json_error("Max. refund amount = $max_amount");
    
    $message = $error = '';
    $ts = now();
    $notes = strip_blank($obj_data['description']) ? $obj_data['description'] : '';
    
    switch($tx_gateway) {
      case "MOLLIE";
      try {
        if($currency != "EUR") json_error("Only Euro payments can be refunded with Mollie");
        
        $response = mollie_payment($siteObj, $txn_id);
        $molliePaymentObj = $response->data;
  
        if(!$response->success || !$molliePaymentObj || $response->error) {
          print_log("mollie_payment response:".dump($response, true), "mollie", LOG_LEVEL_TEST);
          json_error($response->error ?: "Could not fetch payment");
        }
        if(!$molliePaymentObj->isPaid()) json_error("Payment not verified");
  
        $mollie_key =  $_SESSION['live'] || $_SESSION['live_payments_test'] ? get_config('ml', $site_id) : get_config('mt', $site_id);  
        if(!$mollie_key) json_error("Missing API key");      
  
        $mollie = new Mollie_API_Client;
        $mollie->setApiKey($mollie_key);
        
        
        $refund = $mollie->payments->refund($molliePaymentObj, $amount);
        
        //print_log("$ts: Mollie refund".dump($refund, true), 'payment', LOG_LEVEL_TEST);
        
        $amount_refunded = $refund->amount;
        $message = "OK: refunded $amount_refunded";
        
        //json_error("Ok refunding $currency $amount:".dump($molliePaymentObj,true));
        
        
      } catch (Mollie_API_Exception $e) {
        $error = htmlspecialchars($e->getMessage());
        print_log("$ts: Refund error: $error", 'mollie', LOG_LEVEL_ERROR);
      }
      
      break;
    case "PAYPAL":
      json_error("PayPal refund not yet implemented");
      break;
    default: 
      json_error("Invalid transaction data");
    }
    if($error) json_error($error);
    
    $result->success = 1;
    $result->message = $message;
    echo(json_encode($result));exit();    
    
    break;
    
    case 'check-verification':
      if($error = check_required($obj_data, 
        array(
        'user_id' => "User ID", 'profile_id' => "Profile ID", 'type' => "Type", 'code' => "Code", 'site_id' => "Site ID" 
        ))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $message = '';
  
      $profileObj = get_object('user_profile', $profile_id); 
      $json = $profileObj->additional ?: '';
      $jsonObj = $json ? json_decode($json) : new stdClass;
      $key = "_v_".$type;
      $stored_code = $jsonObj->$key;
      if(!$stored_code) json_error("No code is set for $type");
      
      $target = $v_key = '';
      switch($type) {
      case 'phone':
        $target = 'phone';
        $v_key = 'phone_verified';
        $value = $profileObj->$target;    
        if(!phone_valid_idd($value)) json_error("$value does not appear to be valid mobile international phone number - it should be begin with '+' followed by country code.");
        break;
      case 'email':
        $target = 'email_address';
        $v_key = 'email_verified';
        $value = $profileObj->$target;
        if(!is_email($value)) json_error("$value is not a valid email address");
        break;
      case 'social':
        json_error("Unsupported type $type");
        break;
      default:
        json_error("Unsupported type $type");
        break;
      }
      
      if($code && $stored_code == $code) {
        unset($jsonObj->$key);
        $json = json_encode($jsonObj);
        list($id, $errors) = update_object('user_profile', array($v_key => $value, 'additional' => $json), $profile_id);
        if($errors) json_error($errors);
      } else {
        json_error("Wrong code");
      }
      
      $result->success = 1;
      $result->message = $message;
      //$result->code = $code;
      echo(json_encode($result));exit();    
      break;
  
    case 'remove-special-offer':
      global $u_id;
      $so_id = $obj_data['special_offer_id'];
      if(!$so_id) json_error("Missing Special Offer ID");
      $offerObj = get_object('special_offer', $so_id);
      if(!$offerObj) json_error("Offer not found");
      $res_id = $offerObj->res_id;
      $resObj = get_object('reservation', $res_id);
      if(!$resObj) json_error("Booking not found");
      if($resObj->host_id != $u_id) json_error("Not your booking");
  
      if($resObj->org_currency && $resObj->org_rate_type && $resObj->org_rate_base > 0) {
        $data['rate_type'] = $resObj->org_rate_type;
        $data['rate_base'] = $resObj->org_rate_base;
        $data['currency'] = $resObj->org_currency;
        $data['fee_cleaning'] = $resObj->org_fee_cleaning;
        list($id, $errors) = update_object('reservation', $data, $res_id);
        if($errors) json_error($errors);
  
        list($id, $errors) = update_object('special_offer', ['active' => 0], $so_id);
        if($errors) json_error($errors);
  
        $message = "The special offer was removed.";      
      } else {
        json_error("Missing original rate information");
      }
      
      $result->success = 1;
      $result->message = $message;
      //$result->code = $code;
      echo(json_encode($result));exit();
      
      break;
    
      
    case 'add-chat':
      global $u_id;
      extract($obj_data);
      if(!strip_blank($message)) json_error("Please type a message...");
      $resObj = $parent_type == 'reservation' && $parent_id ? get_object('reservation', $parent_id) : null;
          
      if($rate_total && $resObj && in_array($resObj->status, [STATUS_REQUEST, STATUS_PROPOSED]) && $resObj->host_id == $u_id ) {
        if(!isint($rate_total)) json_error("Please fill in a whole number total rate with no decimals");
        list($oid, $oerrors) = add_special_offer($resObj, $rate_total);
        if($oerrors) json_error($oerrors);
        $obj_data['special_offer_id'] = $oid;
        
        $data = [];
        $data['rate_base'] = $rate_total;
        $data['rate_type'] = 'special';
        $data['fee_cleaning'] = 0;
        if($resObj->status == STATUS_REQUEST) $data['status'] = STATUS_PROPOSED;
        list($id, $errors) = update_object('reservation', $data, $parent_id);
        if($errors) json_error($errors);
      }
        
      list($id, $errors) = add_object('chat', $obj_data);
      if($errors) json_error($errors);
  
      $result->success = 1;
      $result->message = "Your message was sent";
      echo(json_encode($result));exit();    
      break;
      
    case 'user-verification':
      if($error = check_required($obj_data, 
        array(
        'user_id' => "User ID", 'profile_id' => "Profile ID", 'type' => "Type", 'target' => "Target", 'site_id' => "Site ID" 
        ))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $message = '';
      $code = unique_id(6);
      //json_error("type=$type");
      switch($type) {
      case 'phone':
        if(!phone_valid_idd($target)) json_error("$target does not appear to be valid mobile international phone number - it should be begin with '+' followed by country code.");
        $sms_result = send_sms($target, $code, '', 'user', $user_id);
        $success = $sms_result->success; 
        if(!$success) json_error("Failed to send SMS to $target");
        $message = "Enter the code sent to $target";
        $sim = $success == SMS_SIM; 
        if($sim) $message .= " (SMS SIM: code=$code)"; // SMS simulation, show code on screen
        break;
      case 'email':
        if(!is_email($target)) json_error("$target is not a valid email address");
        $siteObj = get_object("site", $site_id);
        $domain = $siteObj->domain;
        $site_name = $siteObj->name ?: $domain;
        $from = $siteObj->email_address ?: "info@$domain";
        $subject = "$site_name email verification code";
        $body = $code;
        $mail_result = send_mail($siteObj, $subject, $body, $target, array('parent_type' => 'user', 'parent_id' => $user_id));
        if($mail_result->error) json_error($mail_result->error);
        
        if(!$success) json_error("Failed to send email to $target");
        $message = "Enter the code sent to $target";
        break;
      case 'social':
        break;
      default:
        json_error("Unsupported type $type");
      }
      
      $key = "_v_".$type;
      $json = get_object('user_profile', $profile_id, 'additional') ?: '';
      $jsonObj = $json ? json_decode($json) : new stdClass;
      $jsonObj->$key = $code;
      $json = json_encode($jsonObj);
      list($id, $errors) = update_object('user_profile', array('additional' => $json), $profile_id);
      if($errors) json_error($errors);
      
      $result->success = 1;
      $result->message = $message;
      //$result->code = $code;
      echo(json_encode($result));exit();    
      break;
      
    case 'booking-response':
      if($error = check_required($obj_data, 
        array(
        'site_id' => "Site ID", 'user_id' => "User ID", 'res_id' => "Booking ID", 'response' => "Response" 
        ))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $data = $errors = array();
      $resObj = get_object('reservation', $res_id);
      if(!$resObj) json_error("Could not find this booking");
      if($resObj->expires && $resObj->expires < now()) json_error("This booking has expired");
      $res_id = $resObj->res_id;
      
      // security
      // Only system, host, and mapped users can respond
      $user_level = auth_res($resObj);
      if($user_level < USER_LEVEL_OWNER) json_error("Not authorized: level=$user_level");
      
      $message = $obj_data['message_'.$response];
      $stripped = strip_blank($message);
      $redirect = '';
      switch($response) {
      case RESPONSE_AVAILABLE:
        //json_error(dump($obj_data,true));
        if($status == STATUS_PROPOSED) { // an inquiry is being promoted
          $data['status'] = $status;
          $result->redirect = site_page_link($site_id, PAGE_STATUS).$res_id;
        } elseif(!in_array($resObj->status, array(STATUS_UNCONFIRMED,STATUS_PENDING))) {
          json_error("You cannot accept this booking as the status is ".get_object('res_status', $resObj->status, 'name'));
        } else {
          $data['status'] = $resObj->status == STATUS_UNCONFIRMED ? STATUS_PROPOSED : STATUS_RESERVED;
        }
        break;
      case RESPONSE_SPECIAL_OFFER:
        $data['status'] = STATUS_PROPOSED;
        $total = $obj_data['rate_total'];
        if($total < 10) json_error("The total must be at least {$resObj->currency} 10");
  
        $num_days = $resObj->num_days;
        
        list($oid, $oerrors) = add_special_offer($resObj, $total);
        if($oerrors) json_error($oerrors);
  
        $data['rate_base'] = $total;
        $data['rate_type'] = 'special';
        $data['fee_cleaning'] = 0;
              
        break;
      case RESPONSE_NOT_AVAILABLE:
        $data['status'] = STATUS_BLOCKED;
        break;
      case RESPONSE_NOT_GOOD_FIT:
        $data['status'] = STATUS_DECLINED;
        break;
      case RESPONSE_NOT_COMFORTABLE:
        $data['status'] = STATUS_DECLINED;
        break;
      case RESPONSE_NOT_ENOUGH_INFO:
        $data['status'] = STATUS_DECLINED;
        break;
      case RESPONSE_NOT_ATTRACTIVE:
        $data['status'] = STATUS_DECLINED;
        break;
      case RESPONSE_NOT_OTHER:
        $data['status'] = STATUS_DECLINED;
        break;
      case RESPONSE_MORE_INFO:
        if(!strlen($stripped)) $errors[] = "Please provide a message for the guest.";
        break;
      default:
        $errors[] = "Unknown response $response";
        break;
      }
  
      
      print_log("booking-response: data response=$response msg=$stripped data=".dump($obj_date, true), 'booking', LOG_LEVEL_TEST);
      
      
      if($data['status'] == STATUS_DECLINED) $data['decline_reason'] = $response;
      if($errors) json_error($errors);
  
      if(strlen($stripped)) $data['message'] = $message;    
      if(!count($data)) json_error("No data received: reason=$reason"); 
        
      list($id, $errors) = update_object("reservation", $data, $res_id);
      if($errors) json_error($errors);
  
      $result->success = 1;
      $result->message = "Updated booking $id";
      if($redirect) $result->redirect = $redirect;
      echo(json_encode($result));exit();    
      break;
  
    case 'receive-payment':
      if($error = check_required($obj_data, 
        array(
        'site_id' => "Site ID", 'user_id' => "User ID", 'inv_id' => "Invoice ID", 
        'currency' => "Currency", 'amount' => "Amount", 
        'payment_type' => "Payment Type", 'payment_date' => "Payment Date", 'payment_status' => "Payment Status"
        ))) json_error("Missing: ".implode(', ', $error));
      
      extract($obj_data);
      if(!is_numeric($amount)) json_error("Amount '$amount' is not valid");
      
      $today = today();
      $date = $obj_data['payment_date'] = date2sql($obj_data['payment_date']);
      if($date > $today) json_error("Payment date cannot be in the future"); //date={$date} today={$today}
      
      //json_error(dump($obj_data, true));
      list($pay_id, $errors)  = add_object('payment', $obj_data);
      if($errors) json_error($errors);
  
      $result->success = true;
      
      $paymentObj = get_object('payment', $pay_id);
      $amount = $paymentObj->amount;
      $currency = $paymentObj->currency;      
      $amount_str = "$currency $amount";
      $pay_cur = $paymentObj->payment_currency;
      $pay_amount = $paymentObj->payment_gross;
      
      if($pay_amount && $pay_cur != $currency) $amount_str .= " (Paid using $pay_cur $pay_amount)";
      
      $result->message = "Received $cur $amount_str on invoice $inv_id.";
      $result->reset = 1;
  
        
      echo(json_encode($result));exit();    
      break;

    case 'objects-copy':
      if($obj_data['field']) $obj_data['fields'] = $obj_data['field'];
      if($error = check_required($obj_data, array('obj_type' => "Object Type", 'obj_id' => "Object ID", 'target_ids' => "Target IDs", 'fields' => "Target fields"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $obj = get_object($obj_type, $obj_id);
      if(!$obj) json_error("invoice not found");
      $data = [];
      if(!is_array($fields)) $fields = array($fields);
      foreach($fields as $fld) $data[$fld] = $obj->$fld;      
      $key = get_object_key($obj_type);
      
      list($count, $errors) = update_objects($obj_type, $data, [], [$key => $target_ids]);
      if($errors) json_error($errors);
      
      json_success("Applied to {$count} objects.");
      
      break;

    case 'invoice-billing-copy':
      if($error = check_required($obj_data, array('user_id' => "User ID", 'inv_id' => "Invoice ID"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $inv_id = $obj_data['inv_id'];
      $invObj = get_object('invoice', $inv_id);
      if(!$invObj) json_error("invoice not found");
      if(!$invObj->parent_type) json_error("Missing parent type");
      if(!$invObj->parent_id) json_error("Missing parent id");
      if(!$invObj->sequence) json_error("Not a sequential invoice");
      $search = ['parent_type' => $invObj->parent_type, 'parent_id' => $invObj->parent_id];
      $search['invoice.id'] = array('!=', $inv_id);

      
      
      $applied = [];

      $ids = find_objects('invoice', $search, ['field' => 'invoice.id']);
      $inv_count = count($ids);
      if(!$inv_count) json_error("No invoices found");
      $applied[] = "{$inv_count} other invoice(s)";

      $fields = ['site_id' , 'agent_id', 'org_id' , 'contact_id', 'legal_entity_id', 'bill_to'];
      $data = [];      
      foreach($fields as $fld) $data[$fld] = $invObj->$fld;
      print_log("Updating {$inv_count} other invoices with data from {$inv_id}:".dump($data,true), 'invoice', LOG_LEVEL_TEST);
      list($count, $errors) = update_objects('invoice', $data, [], ['id' => $ids]);
      
      $ids_str = implode(",", $ids);
      
      //json_error("updated $count ids:$ids_str data:".dump($data,true));      
      //json_error(dump($data,true));
      
      $parent_type = $invObj->parent_type;
      $parent_id = $invObj->parent_id;
      $parentObj = get_object($parent_type, $parent_id);

      $fields = ['site_id' ,  'agent_id', 'org_id' , 'contact_id', 'legal_entity_id', 'bill_to'];
      $data = [];
      foreach($fields as $fld) if(isset($parentObj->$fld)) $data[$fld] = $invObj->$fld;
      if($data) {
        list($count, $errors) = update_object($parent_type, $data, $parent_id);
        if($errors) json_error($errors);
        $applied[] = "1 {$parent_type}";
        
      } else {
        //json_error("No fields found for parent $parent_type".dump($data,true));
      }
      
      //json_error("Ok; Applied to {$count} invoices.");
      $applied_str = implode(', ', $applied);
      json_success("Applied to {$applied_str}.");
      
      break;
      
    case 'invoice-address-copy':
      if($error = check_required($obj_data, array('user_id' => "User ID", 'inv_id' => "Invoice ID"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $inv_id = $obj_data['inv_id'];
      $invObj = get_object('invoice', $inv_id);
      if(!$invObj) json_error("invoice not found");
      if(!$invObj->parent_type) json_error("Missing parent type");
      if(!$invObj->parent_id) json_error("Missing parent id");
      if(!$invObj->sequence) json_error("Not a sequential invoice");
      $search = ['parent_type' => $invObj->parent_type, 'parent_id' => $invObj->parent_id];
      $search['invoice.id'] = array('!=', $inv_id);

      $applied = [];

      $addData = copy_address($invObj);
      $addData = array_filter($addData);
      if(!count($addData)) json_error("No address to copy");
      
      $invObjs = find_objects('invoice', $search);
      $inv_count = count($invObjs);
      if(!$inv_count) json_error("No invoices found");
      foreach($invObjs as $otherInvObj) {
        list($id, $errors) = update_object('invoice', $addData, $otherInvObj->id);
        if($errors) json_error($errors);
        
      }
      $applied[] = "{$inv_count} other invoice(s)";
      
      /** Don't apply name/address to booking 
      $parent_type = $invObj->parent_type;
      $parent_id = $invObj->parent_id;
              
      if($parentObj = get_object($parent_type, $parent_id)) {
        list($id, $errors) = update_object($parent_type, $addData, $parent_id);
        if($errors) json_error($errors);
        $applied[] = "1 {$parent_type}";
      }
      */
      
      if(!count($applied)) json_error("Nothing found to apply");
      
      $applied_str = implode(', ', $applied);
      json_success("Applied to {$applied_str}.");
      
      break;
      
    case 'export-invoice':
      if($error = check_required($obj_data, array('user_id' => "User ID", 'inv_id' => "Invoice ID"))) json_error("Missing: ".implode(', ', $error));
      $inv_id = $obj_data['inv_id'];
      $result =  reeleezee_export_invoice($inv_id, true, false);
      print_log("{$now}: reeleezee_export_invoice {$inv_id} success={$result->success}", 'reeleezee', LOG_LEVEL_ALWAYS);
      if($result->success) {
        list($id, $errors) = update_object('invoice', ['sent_date' => $now, 'status' => INV_STATUS_SENT, 'duedate' => add_day(today(), 14), 'locked' => 1], $inv_id);
        if($errors) $result->error = $errors;
      }
      echo(json_encode($result));    
      exit;
      break;
      
    case 'process-payment-old': // payments, the ajax way
  
  
      // get invoice or create it from reservation or order 
      // create order from cart, if needed.
      //if($inv_id) {
      //  //json_message("invoice id=$inv_id");
      //  $invObj = get_object('invoice', $inv_id);
      //  $parent_type = $invObj->parent_type;
      //  $parent_id = $invObj->parent_id;
      //} else {
      //  if($res_id) {
      //    if($invObj = find_booking_invoice($res_id)) { ; // find new booking invoice 
      //      $inv_id = $invObj->id;
      //    } else {  // find new booking invoice
      //      add_reservation_invoices($res_id);
      //      if($invObj = find_booking_invoice($res_id)) $inv_id = $invObj->id;
      //    }
      //    $parent_type = 'reservation';
      //    $parent_id = $res_id;
      //    
      //  } else if($order_id) {
      //  } else {
      //    if(!$inv_id) json_error("No source for invoice.".dump($inputVars, true));
      //  }
      //  if($errors) json_error($errors);
      //                  
      //  if(!$inv_id) json_error("Could not create invoice.".dump($errors, true));
      //
      //}
  
  
      // step 2 - process payment
      $invObj = get_object('invoice', $inv_id); // load fresh
      if(!$invObj) json_error("Ooops. Could not find invoice $inv_id");      
      
      $messages[] = "All good. OrderId=$order_id inv_id=$inv_id. Ready to process payment... ";
      
      //json_message("All good. OrderId=$order_id inv_id=$inv_id. Ready to process payment... ");
      if(!$errors) {
        $paymentOptions['parent_type'] = $parent_type;
        $paymentOptions['parent_id'] = $parent_id;
        $paymentOptions['data'] = $inputVars;
        list($errors, $messages, $resArray) = process_payment($payment_type, $currency, $amount, $invObj, $paymentOptions);
      }
      
      if($errors) json_error($errors);
  
      if($redirect = $resArray['redirect']) { // stop processing, redirect to paypal, iDeal, or whatever
        $result->redirect = $redirect;
        $result->message = $messages ? print_messages($messages) : "Redirecting to $redirect";
        $result->success = 1; 
        echo(json_encode($result));exit();
      }
  
      foreach($resArray as $k => $v) $inputVars[$k] = $v; // Handle return values from process payment, passed to get_payment
      //echo("process payment returned");dump($resArray);
      // dump($_POST);
      // end moved block
  
      if($cc_id = $resArray['cc_id']) {
        if($res_id) update_object('reservation', array('cc_id' => $cc_id), $res_id); // associate cc with res for later retrieval
        if($order_id) update_object('payment_order', array('cc_id' => $cc_id), $order_id); // associate cc with res for later retrieval
        
        update_object('invoice', array('cc_id' => $cc_id), $invObj->id); // associate cc with inv for later retrieval
        $hidden_fields['cc_id'] = $cc_id; // used in get_payment to store with new payment object
      }
      
      if($invObj->balance <= INV_TOLERANCE) {
        json_error("Invoice already paid");
      }
  
      // $payment_type_name = get_object('payment_type', $payment_type, 'name');
      // $messages[] = "Due now: $invObj->currency $due_now. Reservation ID: $resObj->res_id Invoice ID: $invObj->id";
      // $messages[] = "Collected $currency $amount using $payment_type_name.";
  
      // step 3 - check payment details
      // all ok - get payment
      list($charge_error, $charge_message, $resArray) = get_payment($currency, $amount, $invObj, $payment_type, $inputVars, NULL, $partial_payment);
  
      if($charge_error) { // charge failed
        $errors[] = "$charge_error: $charge_message";
        
        if($res_id) update_object('reservation', array('status' => STATUS_DRAFT), $res_id); // demote from draft if no payment was received
        elseif($order_id) update_object('payment_order', array('status' => PAYMENT_STATUS_DRAFT), $order_id); // demote from draft if no payment was received
        
        if(strpos(strtolower($charge_message), 'credit card verification number') !== FALSE) $errors[] = "(Credit Card Verification Number = ".phrase('security_code', CAPITALIZE).')'; // explanation
  
        // "having trouble" text and option to pay later - be contacted.
        $payment_attempt = $hidden_fields['pa'] = $inputVars['pa'] + 1;
        if($payment_attempt > 1) {
          $messages[] = phrase('text_payment_trouble'); 
          $hidden_fields['rid'] = $res_id; 
          $hidden_fields['pin'] = myhash($resObj->booking_pin);
          $hidden_fields['sk'] = 'payment'; // store the step we're displaying now
          $form = array2form($hidden_fields); // always include hidden fields on the form
          $form .= form_input('payment_trouble', 'submit', phrase('pay_later', CAPITALIZE));
          $messages[] = html_form($form); 
        }
        json_error($errors);
      } 
      
      // success    
      if($res_id && !$resObj->send_email && $resObj->type == RES_TYPE_DIRECT) update_object('reservation', array('send_email' => 1), $res_id); // turn on send_email (for direct bookings...)
      if($order_id) update_object('payment_order', array('payment_type' => $payment_type, 'gateway_id' => $gateway_id), $order_id); // turn on send_email (for direct bookings...)
      $result->message = $charge_message;
      $result->success = 1; 
      echo(json_encode($result));exit();
      break;
      
    case 'cart-add':
      extract($_GET);
      $messages = array();
      $errors = array();
      //$messages[] = "user_id=$user_id product_id=$product_id pt=$parent_type pid=$parent_id sid=$site_id";
      //echo(dump($_GET, true));
      if(!$site_id) json_error("Missing site_id");
      $siteObj = get_object('site', $site_id);
      
      $cartObj = cart_add_row($siteObj, $_GET);
      $_SESSION['cart'] = $cartObj; 
       
      if(count($messages)) $result->message = print_messages($messages);
      if(count($errors)) $result->error = print_errors($errors);
      $result->cart = $cartObj;
  
      echo(json_encode($result));exit();
         
      break;
  
    case 'download-media':
      extract($_GET);
      $messages = array();
      $errors = array();
      if(!$site_id) json_error("Missing site_id");
      if(!$user_id) json_error("Missing user_id");
      if($user_id != $u_id) json_error("You are not logged in as this user");    
      if($parent_type != 'media') json_error("Invalid type");
      if(!$parent_id) json_error("Missing media id");
      if(!$size) json_error("Missing size");
      if(!in_array($size, array('thumb', 'small', 'square', 'medium', 'big', 'full'))) json_error("Invalid size");
      $media_id = $parent_id;
      $mediaObj = get_object('media', $media_id);
      if(!$mediaObj) json_error("Medium not found in database");
      if(!gallery_user_owns_image($siteObj, $u_id, $mediaObj)) json_error("You do not own this medium");
           
      if(count($messages)) $result->message = print_messages($messages);
      if(count($errors)) $result->error = print_errors($errors);
  
      // find/create download dir
      $siteObj = any2obj('site', $site_id);
      $data_dir = site_data_directory($siteObj);
      $type = $mediaObj->type;
      $dst_path = "/user/$user_id/download/media/$type/";
      $dst_dir  = clean_path($data_dir.$dst_path);
      
      if(!is_dir($dst_dir) && !mkdir($dst_dir, 0777, true)) json_error("Failed to make user download directory $dst_dir");
  
      //$src_dir = $data_dir.$mediaObj->path; // not for now, looks in /web/data-dev/.../ 
      $src_dir = site_directory($siteObj).$mediaObj->path;
      
      if(!is_dir($src_dir)) json_error("Source directory not found");
  
      $src_file = $src_dir;
      $filename = $mediaObj->filename;
      $size_dir = image_size_dir($size);
      
      $src_filename = $src_dir."$size_dir/$filename"; 
      $dst_filename = $dst_dir.$filename;
      $dst_src = $dst_path.$filename; // relative to web doc root
      if(!is_file($src_filename)) json_error("Medium not found on disk");
      if(!is_file($dst_filename) && !copy($src_filename, $dst_filename)) json_error("Copy failed from $src_filename to $dst_filename");
  
      $data['media_id'] = $media_id; 
      $data['user_id'] = $user_id; 
      $data['type'] = $type; 
      $data['path'] = $dst_path; 
      $data['filename'] = $filename; 
      $result->success = 1;    
      $result->data = $data;
      echo(json_encode($result));exit();
         
      break;
      
    // generic operation for email sign-up for whatever
               
    case 'new-lead':
      $email = $obj_data['email_address'];
      $site_id = $obj_data['site_id'];
      $errors = $messages = array();
      if(!$email) json_error("Missing email");
      if(!$site_id) json_error("Missing site_id");
      if(!is_email($email)) json_error("Invalid email $email");
      
      $obj_type = 'lead';
      $success = false;
      $existingObj = find_object($obj_type, array('email_address' => $email, 'site_id' => $site_id));
      if($existingObj) {  
        //$message = "Your email address is already in our system. ($existingObj->id)";
        $message = "Thank you for your interest in $siteObj->name. We will get back to you.";      
      } else {
        list($id, $errors) = add_object($obj_type, $obj_data);
        if($id) {
          $success = true;
          $message = "Thank you for your interest in $siteObj->name. We will get back to you.";
          $resultObj = get_object($obj_type, $id);
          $resultObj->id = $id;
          $resultObj->obj_type = $obj_type;       
        } elseif($errors) {
          $message = $result['error'] = print_errors($errors);
        }
      }
      
      $result['message'] = $message;
      $result['success'] = $success;
      $result['data'] = $resultObj;                
      echo(json_encode($result));exit();
      break;
      
    case 'new-user':          
      //$output .= "GET: ".dump($_GET, true);
      //$output .= "POST: ".dump($_POST, true);
      $id = $_GET['id'];
      $site_id = $_GET['site_id'];
      $siteObj = get_object('site', $site_id);
      $userObj = get_user($id);
      $roles = array();
      if(user_has_role($userObj, USER_ROLE_ARTIST)) $role = USER_ROLE_ARTIST;
      else if(user_has_role($userObj, USER_ROLE_CURATOR)) $role = USER_ROLE_CURATOR;    
      
      if($role) {
        $role_name = get_object('user_role', $role, 'name');
        $url = gallery_user_link($siteObj, $userObj, array('role' => $role));
        $link = html_link($url, $userObj->display_name);
      } else {
        $role_name = 'user';
        $link = $userObj->display_name;
      }
        
      $output = "Added $role_name $link";
      
      //$url = gallery_collection_link($siteObj, $id);
      //$link = html_link($url, $obj->title);
      //$output = "Your new $collection_type $link was added.";
      echo($output);exit();
      break;
          
    case 'ckeditor_autosave':
      if(!$obj_type) $errors[] = "Missing object type";
      require_once('ckeditor/plugins/autosave/php/autosave.php');
      $autosave = new Autosave();
      $autosave->debugMode = true;
      $autosave->saveToDB($obj_type, $id, $obj_data['field'], $obj_data);
      return;
      break;
    
    // these are public ops, no security required
    case 'load-more':
      $searchObj =  $options['searchObj'];
      if(is_array($searchObj)) $options['searchObj'] = array2obj($searchObj);
      
      $options['nodiv'] = 1;
      $options['start'] = $_GET['start'];
      $options['hide_qs'] = 1; // don't show query string
      $output = property_list($siteObj, $pageObj, $options);
      echo($output);exit();
      break;

    // these are public ops, no security required
    case 'request-form':
      //$data =  $_GET['data'] ? json_decode($_GET['data']) : NULL;
      $data = array2obj($obj_data);
      $output = request_form($data->site_id, $data->type, $data->property_id, $options);
      echo($output);exit();
      break;

    case 'package-form':
      //$data =  $_GET['data'] ? json_decode($_GET['data']) : NULL;
      $data = array2obj($obj_data);
      $options['data'] = $data;
      $output = request_form($data->site_id, 'product', $data->package_id, $options);
      echo($output);exit();
      break;

    case 'test-pricing':
      $debug = isset($_GET['debug']);
     
      //$debug = print_r($obj_data, true);
      
      $searchObj = array2obj($obj_data);
      $searchObj->pt = $obj_data['property_type'];
      $searchObj->in = $obj_data['checkin'];
      $searchObj->out = $obj_data['checkout'];
      //$searchObj = search2in_out($searchObj);

      //json_error(dump($obj_data, true));
      $in = $searchObj->in;
      $wd = get_sql_weekday($in);
      $in_wd = $_SESSION['weekdays'][$wd];
      $out = $searchObj->out;
      $ng = $num_guests = $searchObj->ng;
      
      $apt_id = $searchObj->apt_id;
      $ptype = $type = $searchObj->pt;

      $aptObj = get_object($ptype, $apt_id);
      $stdAptObj = clone($aptObj);
      
      $exceptions = date_exceptions($in, $out, $aptObj->host_id, $aptObj->id);
      if($exceptions) { 
        list($non_loop_exceptions, $loop_exceptions) = split_exceptions($type, $exceptions, $searchObj, $apt_id);
        $aptObj = apply_exceptions($ptype, $apt_id, $exceptions, $searchObj); // apply        
      }

      if($debug) {
        echo("exceptions for $in-$out apt: $aptObj->id host:$aptObj->host_id");
        dump($exceptions);
      }
      
      if(0) {
        // reload after exceptions          
        $rate_base = $aptObj->rate_day;
        $rate_base_ng = get_rate_num_guests($rate_base, $aptObj, $num_guests);
                                   
        $book_now_str = $aptObj->book_direct ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red');


        $available = calendar_availability($apt_id, $in, $out);
        $available_str = $available ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red');
        $now_unavailable = $aptObj->unavailable;
        $now_available = $available && !$now_unavailable;
        $available_str = $now_available ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red');

        if($now_available) {
          list($button_text, $booking_url)  = print_request_link($aptObj, $searchObj);
          $booking_link = html_link($booking_url, $button_text, array('class' => 'non-html'));
          $available_str .= " $booking_link";
        } else {
          $unvailable_reason = implode(', ', array_filter(array($conflict, $aptObj->unavailable_reason)));
          $available_str .= ' ('.$unvailable_reason.')'; // todo: pretty print reason
        }
                  

        //$outputAr[] = print_rate_table($aptObj, $searchObj, array('weekend_pricing' => $weekend_pricing));
      }
      
      
      $apt_name = $aptObj->name;
      $min_stay = $aptObj->min_stay;
      $max_stay = $aptObj->max_stay;
      $max_guests = $aptObj->max_guests;
      $cur = get_currency_symbol($aptObj->currency, true);
      $co = $aptObj->changeover;
      if($co) $co_wd = $_SESSION['weekdays'][$co];
      
      $currency = $aptObj->currency;
      $num_days  = $nd = $searchObj->nd;
      $num_hours = $searchObj->nh;
      $rate_unit = $num_days > 0 ? 'day' : 'hour'; // todo: handle months ? 
      $num_units = $num_days ?: $num_hours; // todo: handle months ? 
      $unit_str = unit_string($rate_unit, $num_units);
      
      $book_now_str = $aptObj->book_direct ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red');
      $days_from_now = period_diff(today(), $searchObj->in, 'd');
      $rate_base = $aptObj->rate_day;
      $rate_base_ng = get_rate_num_guests($rate_base, $aptObj, $num_guests);
      

      $outputAr = array();
      
      $outputAr[] = html_b("Details for ".print_time_range($in, $out, array('show_date' => true)).":");
      $outputAr[] = "Length: $num_units $unit_str";
      $outputAr[] = "Checkin is $days_from_now ".($days_from_now == 1 ? phrase('day_from_now') : phrase('days_from_now'));

      $priceObj = price_breakdown($aptObj, $searchObj, $exceptions);

      //$outputAr[]= print_price_breakdown($aptObj, $priceObj);
      $name = phrase('price_breakdown', CAPITALIZE);
      $link = "/ajax.php?oper=price-breakdown&t={$ptype}&apt_id={$apt_id}&in={$in}&out={$out}";

      $fees = formatFloat($priceObj->fees);
      $tax = formatFloat($priceObj->tax);
      $total = formatFloat($priceObj->total);
      $gtotal = formatFloat($priceObj->grand_total);
      $avg = formatFloat($priceObj->average);
      $outputAr[] = "Total: {$cur}{$total}"; 
      if($tax > 0) $outputAr[] = "Tax: {$cur}{$tax}"; 
      if($fees > 0) $outputAr[] = "Fees: {$cur}{$fees}"; 
      if($tax > 0 || $fees > 0) $outputAr[] = "Grand Total: {$cur}{$gtotal}"; 
      $outputAr[] = "Average: {$cur}{$avg} ".html_fancybox_link($link, $name); 

      
      $weekend_rate_cutoff = $aptObj->weekend_rate_cutoff;  /** not in use */ 
      $weekend_rate_cutoff = 0; /** turn this off for now. always use weekend rates unless weekly rate is set */
      $weekend_pricing = (!$weekend_rate_cutoff || $num_days < $weekend_rate_cutoff) && $aptObj->rate_weekend > 0 && $aptObj->rate_weekend != $aptObj->rate_day; // will weekday affect the price
      if($weekend_pricing) {
        $weekend_str = "";
      } elseif($aptObj->rate_weekend <= 0) {
        $weekend_str = ": Weekend rate not set";
      } elseif($aptObj->rate_weekend && $weekend_rate_cutoff > 0 && $num_days >= $weekend_rate_cutoff) {
        $weekend_str =": Weekend rate does not apply to bookings >= $weekend_rate_cutoff days";
      }
      
      /**
      $ffObj = fit_factor_object($searchObj, $apt_id);
      //$outputAr[] = dump($ffObj, true);
      
      if($fit_factor = $ffObj->fit_between) {
        $outputAr[] = "Fit between: $fit_factor % ".html_image(fit_factor_image($ffObj, array('fit_type' => 'between')), "Fit between = $fit_factor %");
      }

      if($fit_factor = $ffObj->fit_next_to) {
        $outputAr[] = "Fit next to: $fit_factor % ".html_image(fit_factor_image($ffObj, array('fit_type' => 'next_to')), "Fit next to = $fit_factor %");
      }
      */
      

      // availability, including min_stay, num_guests, changeover, etc
      $available = calendar_availability($apt_id, $in, $out);
            
      $reasons = [];
      if(!$available) {
        if($res_ids = conflicting_reservations($in, $out, $ptype, $aptObj->city_id, $apt_id)) { 
          $conflict = phrase('booked', CAPITALIZE);
          $conflicts = [];
          foreach($res_ids as $res_id) {
            $resObj = get_object('reservation', $res_id);
            $conflicts[]= " &nbsp;- ".phrase('reservation', CAPITALIZE)." $resObj->res_id: ".print_time_range($resObj->checkin, $resObj->checkout, array('unit' => $rate_unit));
          }
          $reasons[] = "Conflict(s):".html_break().implode(html_break(), $conflicts);
        }
      }

      if($ng > $max_guests) $reasons[] = "Maximum occupancy is $max_guests.";
      if($min_stay && $nd < $min_stay) $reasons[] = "Minimum stay is $min_stay nights.";
      if($max_stay && $nd > $max_stay) $reasons[] = "Maximum stay is $max_stay nights.";
      
      if($co && $co != $wd) $reasons[] = "Checkin is on a $in_wd. Changeover is only allowed on $co_wd";
      
      $available_str = $available && !count($reasons) ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red');
      $outputAr[] = "Available?: $available_str";

      if(!$available || count($reasons)) {
        $outputAr[] = "<b>Reason(s):</b>";
        foreach($reasons as $reason) $outputAr[] = $reason;
        $outputAr[] = '';
      } else {
        $outputAr[] = "Book direct?: ".($aptObj->book_direct ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red'));
      }
      
      
      //$outputAr[] = "Weekend Pricing?: ".($weekend_pricing ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red')).$weekend_str;
      //$outputAr[] = '';
      //$outputAr[] = print_rate_table($aptObj, $searchObj, array('weekend_pricing' => $weekend_pricing));

      

      $outputAr[] = '';

      // repeated after exceptions
      $outputAr[] = html_b(phrase('standard_rate_and_terms', CAPITALIZE).':');
      $outputAr[] = "Minimum stay: $stdAptObj->min_stay $unit_str";
      $outputAr[] = "Maximum guests: $stdAptObj->max_guests ".unit_string("guest", $num_units);
      if($co = $stdAptObj->changeover) {
        $co_wd = $_SESSION['weekdays'][$co];      
        $outputAr[] = "Changeover: $co_wd ";
      }
      $outputAr[] = "Rate: {$cur}{$stdAptObj->rate_day}";
      if($stdAptObj->rate_weekend > 0) $outputAr[] = "Weekend Rate: {$cur}{$stdAptObj->rate_weekend}";
      $outputAr[] = "Maximum guests: $stdAptObj->max_guests ".unit_string("guest", $num_units);
      $outputAr[] = "Book direct?: ".($stdAptObj->book_direct ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red'));

      // end repeated after exceptions
      

      if($exceptions) {
        // repeated from above
        $outputAr[]= '';
        
        if($non_loop_exceptions) {
          $outputAr[] = html_b(phrase('exceptions_that_apply_to_all_dates', CAPITALIZE).':');
          $outputAr[] = print_exceptions($type, $apt_id, $non_loop_exceptions);
        }
        
        if($loop_exceptions) {
          $outputAr[] = html_b(phrase('exceptions_that_apply_to_some_dates', CAPITALIZE).':');
          $outputAr[] = print_exceptions($type, $apt_id, $loop_exceptions);
        }
        

        $outputAr[] = html_b(phrase('after_exceptions', CAPITALIZE).':');
        $outputAr[] = "Minimum stay: $aptObj->min_stay $unit_str";
        if($co = $aptObj->changeover) {
          $co_wd = $_SESSION['weekdays'][$co];      
          $outputAr[] = "Changeover: $co_wd ";
        }
        //$outputAr[] = "Available?: ".$available_str;
        $outputAr[] = "Book direct?: ".($aptObj->book_direct ? html_span(phrase('yes', CAPITALIZE), 'bold green') : html_span(phrase('no', CAPITALIZE), 'bold red'));
      }
      
      //$outputAr[] = dump($aptObj, true);
      
      //$outputAr[] = "Base Rate: $currency $rate_base / $rate_unit";
      //if($aptObj->rate_weekend > 0) {
      //  $outputAr[] = "Weekend Rate: $currency ".$aptObj->rate_weekend." / $rate_unit";
      //  $weekend_rate_ng = get_rate_num_guests($aptObj->rate_weekend, $aptObj, $num_guests);
      //}
      //
      //if($ng > $aptObj->rate_num_guests) {
      //  if($rate_base_ng != $rate_base) $outputAr[] = "Base Rate for $ng guests: $currency $rate_base_ng / $rate_unit";
      //  if($weekend_rate_ng != $rate_base) $outputAr[] = "Weekend Rate for $ng guests: $currency $weekend_rate_ng / $rate_unit";
      //}
      //if($aptObj->rate_change && $aptObj->rate_change != '0%') {
      //  $sign = floatval($aptObj->rate_change) > 0 ? '+' : '';
      //  $outputAr[] = "Discount: $sign".$aptObj->rate_change;
      //  $new_rate_ng = apply_surcharge($rate_base_ng, $aptObj->rate_change);
      //  $outputAr[] = "New Base Rate for $ng guests: $currency $new_rate_ng / $rate_unit";
      //}

      // end repeated from above
      //$outputAr[]= print_r($aptObj, true);
      //$outputAr[]= html_b(phrase('pricing_breakdown', CAPITALIZE));

      
                                       
      $output = implode(html_break(), $outputAr);
      
      //echo($output);exit();
      json_message($output, array('encode' => false));
      
      break;

    case 'load-page-content':
      $data = array2obj($_POST ?: $_GET);
      $site_id = $data->site_id;
      $page_id = $data->page_id;
      $page_type = $data->page_type;
      if(($site_id && $page_type)) {
        $search['site_id'] = $site_id;
        $search['page_type'] = $page_type;
        $pageObj = find_object('site_page', $search);
      } else if($page_id) {
        $pageObj = get_object('site_page', $page_id);
        $site_id = $pageObj->site_id;
      } else {
        json_error("Invalid data: specify site_id/page_type or page_id");
      }
      $siteObj = get_object('site', $site_id);
      if(!$pageObj) json_error("Page not found");
      if($pageObj->user_level > 0) json_error("Page not public");
      
      $language = $data->language ?: $_SESSION['language'];
      if($language) $language = get_object('site', $site_id, 'default_language');
           
      $contentObj = find_object('page_content', array('page_id' => $pageObj->id, 'language' => $language));
      if(!$contentObj) $contentObj = find_object('page_content', array('page_id' => $pageObj->id));
      if(!$contentObj) json_error("No content found");
      $content = $contentObj->content;
      if(!strip_tags($content)) $content = site_page_content($siteObj, $pageObj);// blank content, check site_pages
      $output = html_h2($contentObj->title);
      $output .= html_p($content);
      echo($output);exit();
      
      break;
      
    case 'price-breakdown':
      
      if($id = $inputVars['id']) {
        $priceObj = get_price_breakdown($id);
      } else {
        $searchObj = array2obj($inputVars);
        $searchObj->in = date2sql($searchObj->in);
        $searchObj->out = date2sql($searchObj->out);
        
        $ptype = $searchObj->t;
        $apt_id = $searchObj->apt_id;        
        $aptObj = get_object($ptype, $apt_id);
        
        $priceObj = price_breakdown($aptObj, $searchObj);
      }

      //$output = dump($searchObj, true);echo($output);exit();

      $output = print_price_breakdown($aptObj, $priceObj);
      if($_SESSION['dev'] && $inputVars['dump']) $output .= dump($priceObj, true);
      echo($output);exit();
      break;

    case 'account-linking-request':
      //json_error(dump($_POST,true));
      
      $host_user_id  = trim($obj_data['host_user_id']);
      $user_id  = trim($obj_data['user_id']);    
      $user = trim($obj_data['target_user']);
      $msg  = trim($obj_data['message']);
      $site_id = $_SESSION['site_id'];
      if(!$host_user_id && !$user_id) json_error("Missing User ID");
      print_log(dump($obj_data, true), 'debug', LOG_LEVEL_TEST);
      
      if(!$user) json_error("Missing Email or User ID");
      if(isint($user)) {
        if($userObj = find_object('user', ["user.id" => $user, "user.site_id" => $_SESSION['site_id'], "active" => 1])) {
          $target_user_id = $userObj->id;
        } else {
          json_error("Sorry, we could not find a user with this user ID");
        }
      } elseif(is_email($user)) {
        if($userObj = find_object('user', ["username" => $user, "user.site_id" => $_SESSION['site_id'], "active" => 1])) {
          $target_user_id = $userObj->id;
        } else if($profileObj = find_object('user', ["email_address" => $user, "user.site_id" => $_SESSION['site_id']])) {
          $target_user_id = $profileObj->user_id;
          $userObj = get_object('user', $user_id);
        } else {
          json_error("Sorry, we could not find a user with this email address");
        }
      } else {
        json_error("{$user} is neither a number nor a valid email address.");      
      }
  
      if($target_user_id == $host_user_id || $target_user_id == $user_id) json_error("You cannot link to yourself");
      
      if($host_user_id) { /** we're giving permissions */
        $config = $_SESSION['config'];
        $permissions = $config['link_permissions'];
        $permData = [];
        foreach($permissions as $k => $v) {
          foreach($v as $k1 => $v1) {
            $key = "{$k}-{$k1}";
            if(isset($obj_data[$key]) && $obj_data[$key]) {
              $permData[$k] = $k1;
            }
          }
        }
        if(count($permissions) && !count($permData)) json_error("Please grant at least one permission"); 
        $permJson = json_encode($permData);
        $data = ["host_user_id" => $host_user_id, "user_id" => $target_user_id, "site_id" => $site_id, "permissions" => $permJson];
        $verified = 1;
        //json_error('gh');
      } else { /** we're requesting permissions */ 
        $data = ["host_user_id" => $target_user_id, "user_id" => $user_id, "site_id" => $site_id];
        $verified = 0;
      }
      
      print_log("json: $permJson".dump($data, true), 'debug', LOG_LEVEL_TEST);
      //json_error($permJson);
      if($id = $obj_data['obj_id']) {
        list($id, $errors) = update_object('user_map', $data, $id);
      } else {
        if(find_object("user_map", $data)) json_error("This link already exists"); 
        $data['verified'] = $verified;        
        list($id, $errors) = add_object('user_map', $data);
      }
      if($errors) json_error($errors);
      $username = user_display_name($user_id);
      $result->success = 1;
      $result->message = "OK. Request for account linking was sent to {$username}";
      echo(json_encode($result));exit;      
      break;

    case 'edit-user':
      if($error = check_required($obj_data, array('pk' => 'User ID', 'value' => 'User Level'))) json_error("Missing: ".implode(', ', $error));
      $user_level =  $obj_data['value'];
      if(!$u) json_error("Not logged in");
      if($app->user->user_level < USER_LEVEL_ADMIN) json_error("Only for administrator");
      
      if($user_level < 1 || $user_level > 40) json_error("Please use a value between 1 and 40");
      $user_id = $obj_data['pk'];
      list($id, $errors) = update_object('user', ['user_level' => $user_level], $user_id);
      if($errors) json_error($errors);
      $result->success = 1;
      $result->message = "OK. Request for account linking was sent to {$username}";
      print_log("edit-user {$user_id} set level={$user_level} data=".dump($obj_data,true), 'user', LOG_LEVEL_ALWAYS);
      echo(json_encode($result));exit;      
      
      break;
      
    case 'edit-account':
      
      
      $username = $obj_data['username'];
      $user_id = $obj_data['user_id']; // explicit since it might otherwise grab it from above
      $site_id = $obj_data['site_id']; // explicit since it might otherwise grab it from above
      unset($obj_data['site_id']); // we don't want to save this
      
      if(!$user_id) json_error("Missing user id");
      if(!$site_id) json_error("Missing site id");
      
      if(!$username) json_error("Username is required");      
      $userObj = get_object('user', $user_id);
      if(!$userObj) json_error("Could not find user");
      if($username == $userObj->username) {
        //unset($obj_data['username']); // no need to save this, will trigger check in class handlers
      } else {  // changing username, check if available.
        //json_error("got here site_id=$site_id");
        if($userObj = find_object('user', array('username' => $username, 'site_id' => $site_id))) {
          //$debug = " by $userObj->id";
          json_error("The username/email $username is already in use.".$debug); 
        }
      }

      list($id, $errors) = update_object('user', $obj_data, $user_id);
      if($errors) json_error($errors);

      $account_link = site_pagetype_link($siteObj, PAGE_ACCOUNT);
      $response->redirect = $account_link;
      $response->success = 1;            
      $response->message = "Your account was successfully updated";
      echo(json_encode($response));exit;
      break;
      
    case 'user-account-promotion':
      extract($obj_data);
      
      if($error = check_required($obj_data, array('site_id', 'user_id', 'roles'))) json_error("Missing: ".implode(', ', $error));
      $userObj = get_object('user', $user_id);

      $roles = add_role($roles, 1); // add the trivial user role 1 
      $user_roles = $userObj->roles;
      $roles = add_role($roles, $user_roles); // add existing roles 

      $role_level = role2level($roles);
      $max_level = max($role_level, $userObj->user_level);

      if(1) {
        list($id, $errors) = update_object('user', array('user_level' => $max_level, 'roles' => $roles), $userObj->id);
        if($errors) $response->error = $errors;
        else if($id) {
          $response->data = $id;
          $response->success = 1;
          $user_name = $userObj->display_name;
          $response->message = "Ok, $user_name was upgraded and notified";
        }
      }
      //$response->message = "received: $obj_data[roles]<br>user:$user_roles<br>new:$roles<br>level:$max_level";
        
      echo(json_encode($response));exit;
      break;

    case 'accept-account-invitation':
      extract($obj_data);
      if($error = check_required($obj_data, array('site_id', 'user_id', 'code'))) json_error("Missing: ".implode(', ', $error));
      $response = user_account_upgrade($site_id, $user_id, $code);
      echo(json_encode($response));exit;
      break;

    case 'send-account-invitation':
      extract($obj_data);
      if($error = check_required($obj_data, array('site_id', 'user_id', 'code', 'email_address'))) json_error("Missing: ".implode(', ', $error));
      if(!is_email($email_address)) json_error("Invalid email"); 
      $response = send_user_account_invitation($site_id, $user_id, $code, $email_address);
      echo(json_encode($response));exit;
      break;
            
     
    /** we don't allow edit user, so we need custom handlers for this */
    case 'save-payout':
      global $u_id;
      $user_id = $obj_data['user_id']; // explicit since it might otherwise grab it from above
      extract($obj_data); 
      if(!$user_id) json_error("Missing user id");
      if($user_id != $u_id) json_error("Not your user id");
      $userObj = get_object('user', $user_id);
      if(!$userObj) json_error("Could not find user");
      $data = array_filter($obj_data);
      //array(
      //  'bank_name' => $bank_name, 
      //  'bank_name' => $account_number, 
      //  'bank_name' => $account_name, 
      //  'bank_name' => $account_iban, 
      //  'bank_name' => $account_country, 
      //  'bank_name' => $paypal_id));
      unset($data['user_id']);
      if($bank_name || $account_number || $account_name || $account_iban || $account_country) {
        if($error = check_required($obj_data,
          array(
            'bank_name' => 'Bank Name', 
            'account_name' => 'Account Name', 
            //'account_number' => 'Account Name', 
            'account_iban' => 'Account Number', 
            'account_country' => 'Account Country', 
           ))) json_error("Missing: ".implode(', ', $error));
      }
      if($account_iban) $data['account_iban'] = str_replace(' ', '', $account_iban);
      //json_error(dump($data, true));
      if(!$data) json_error("No data");
      list($id, $errors) = update_object("user", $data, $user_id);
      if($errors) json_error($errors);
      json_message("Saved");
      break;
            
    case 'reset-password':
      $site_id = $obj_data['site_id']; 
      $username_or_email = $obj_data['username_or_email'];
      if(!$site_id) json_error("Missing site id");
      if(!$username_or_email) json_error("Missing username or email");

      /** new: using phpauth */
      $phpauth = user_phpauth();
      $result = $phpauth->requestReset($username_or_email, true); /** try username - if email */
      
      if($result['error']) { /** try user profile */
        if($profileObj = find_object('user_profile', array('email_address' => $username_or_email, 'client_id' => $_SESSION['client_id']))) {
          if($user_id = $profileObj->user_id) {
            $username = get_object('user', $user_id, 'username');
            $result = $phpauth->requestReset($username, true);
          }              
        }
      }
      
      if($result['error']) {
        json_error($result['message']);
      }
      json_message($result['message']);
      exit();      
    break;
 
    /** we don't allow edit user, so we need custom handlers for this */
    case 'edit-password':
      $errors = array();
      $result = array();

      $user_id = $obj_data['user_id']; // explicit since it might otherwise grab it from above
      extract($obj_data); 
      if(!$site_id) json_error("Missing site id");
      if(!$user_id) json_error("Missing user id");
      if(!$password1) json_error("Missing New Password");
      if(!$password2) json_error("Missing Repeat Password");
      if($password1 != $password2) json_error("Passwords do not match");
      
      /** new: using phpauth */
      $phpauth = user_phpauth();
      //print_log("Edit Password 2 guid-$guid".dump($result, true), 'auth', LOG_LEVEL_ALWAYS); 
      
      if($guid) {
        $result = $phpauth->resetPass($guid, $password1, $password2);
        //print_log("Edit Password 3 guid-$guid".dump($result, true), 'auth', LOG_LEVEL_ALWAYS); 
        if(isset($result['error']) && !strlen(trim($result['message']))) $result['message'] = "Please use at least one UPPERCASE letter, one lowercase letter, and one number";
      } else {  
        if(!$u_id) json_error("You must be logged in to change your password");
        if(!$old_password) json_error("Please enter your current password");
        $result = $phpauth->changePassword($user_id, $old_password, $password1, $password2);
        //print_log("Validate password result".dump($result, true), 'auth', LOG_LEVEL_ALWAYS); 
        
        if($result['error'] && !strlen(trim($result['message']))) $result['message'] = "Please use at least one UPPERCASE letter, one lowercase letter, and one number";
      }
      if($result['error']) json_error($result['message']);
      json_message($result['message']);
      exit();      
      break;
      
    case 'print-log':
      $file = str_replace(array('\\', '/', '.'), '', $_POST['file']); // for security
      $string = $_POST['string'];
      $log_level = $_POST['log_level'] ?: $_SESSION['log_level'];
      if($file && $string) print_log($string, $file, $log_level);
      break;

    case 'contact-form':
      $siteObj = $__SITE;
      if($error = check_required($_POST, array('name', 'email', 'message'))) json_error("Missing: ".implode(', ', $error));
      extract($_POST);
      if(!is_email($email)) json_error("$email is not a valid email address");


      // create email body and send it	
      //list($subject, $body, $from, $to, $replyto) = mail_template("contact_form", $obj_type, $personObj, 'accounts', 0, $personObj->language, $changes);
      $subject = "$siteObj->name Contact Form: $name";
      $body = html_p("You have received a new message from your website's contact form.");
      $body .= html_b(phrase('name', CAPITALIZE)).": $name".html_break();
      if($phone) $body .= html_b(phrase('phone', CAPITALIZE)).": $phone".html_break();
      $body .= html_b(phrase('email', CAPITALIZE)).": $email".html_break();
      $body .= html_b(phrase('message', CAPITALIZE)).": $message".html_break();
      $from = "noreply@".$siteObj->domain;
      $reply_to = $email;	
      
      $site_email = site_email($siteObj);
      if(!$site_email) json_error("Oops. This site does not have an email address configured.");
      //if(!is_email($site_email)) json_error("Oops. This site has an invalid email address configured.");
      
      $result = send_mail($siteObj, $subject, $body, $site_email, array('replyto' => $reply_to));   
      $to = 'your-email@your-domain.com'; // PUT YOUR EMAIL ADDRESS HERE
      $success = $result->success;
      
      //mail($to,$email_subject,$email_body,$headers);
      //return true;
      if($success) $result = json_success_object(phrase("your_message_has_been_sent", CAPITALIZE).".");
      echo(json_encode($result));exit;
      break;

    case 'add-user-cc':
      //list($id, $errors) = add_object('cc', $obj_data);
      if($error = check_required($obj_data, array('site_id' => "Site ID", 'user_id' => "User ID", 'cc_number' => "Card Number",'cc_cvv2' => "CVC Code", 'cc_cardholder' => "Carholder Name",'cc_exp' => "Expiration"))) json_error("Required: ".implode(', ', $error));
      extract($obj_data);
      list($cc_exp_month, $cc_exp_year) = explode(' / ', $cc_exp);
      if(strlen($cc_exp_year) == 2) $cc_exp_year = "20".$cc_exp_year; // ok, won't work in 86 years ;)
      $obj_data['cc_exp_month'] = $cc_exp_month;
      $obj_data['cc_exp_year'] = $cc_exp_year;

      if($cc_type_name == 'unknown' || !$cc_type_name) return("$cc_number is not a recognized card type");
      if($cc_type_name == 'amex') $cc_type_name = "american express";
      if($cc_type_name == 'dinersclub') $cc_type_name = "diners club";
      $cc_type = $obj_data['cc_type'] = find_object('cc_type', array('name' => $cc_type_name), 'id');
      
      if(!$cc_type) json_error("$cc_number is not a recognized card type");

      //json_error("got here".dump($obj_data, true));
          
      list($id, $errors) = add_object('cc', $obj_data);
      
      $result = $id ? json_success_object(phrase("your_card_was_added", CAPITALIZE).".") : json_error_object($errors);
      echo(json_encode($result));exit;
      break;
      
    // ajax login
    case 'provider-login':
      
      if($error = check_required($obj_data, array('provider_name', 'site_id'))) json_error("Required: ".implode(', ', $error));
      extract($obj_data);
      //json_error(dump($obj_data, true));
      $user_id = user_provider_login($provider_name);
      json_error("got here: user=$user_id");
      $phpauth = user_phpauth();
      $result = $phpauth->login($username, $pw1, $remember);
      
      if($user_id) {
        $response->user_id = $user_id;
        $response->success = 1;
      }
      echo(json_encode($response));exit;

      break;
      
    case 'backdoor-login':
      //$auth = ($u_id == SYSTEM_USER_ID) || ($u_level >= USER_LEVEL_ADMIN && !$_SESSION['live']);
      $phpauth = user_phpauth();

      
      $user_id = $obj_data['user_id'];
      $auth = auth_login($user_id);
      if(!$auth) json_error("Unauthorized");
      $data = array2obj($obj_data);
      if(!$user_id) json_error("Missing user id");
      $result = $phpauth->third_party_login($user_id, true);
      $now = now();
      print_log("$now: backdoor-login: result=".dump($result, true), 'auth', LOG_LEVEL_TEST);
      //json_error($result);
      if($result['error'] || !$result['hash']) {
        $response->error = $result['error'] ?: "Login failed";
        $response->success = 0;
      } else {
        $response->message = $result['message'] ?: "Login succeeded";
        $response->success = 1;
        $response->redirect = $data->redirect ?: "/";
        $_SESSION['hash'] = $result['hash'];
        $_SESSION['u_id'] = $result['user_id'];
      }      
      echo(json_encode($response));exit;
      
      break;
      
    case 'user-login':
      $errors = array();
      $result = array();
      $siteObj = $__SITE;
      //if($u_id) json_error("Please log out before logging in.");
      
      /** new: using phpauth */
      $phpauth = user_phpauth();

      $remember = isset($obj_data['remember']);
      $data = array2obj($obj_data);
      $pw1 = $data->password;
      $username = $data->username;
      
      $remember = 1; // for testing
      
      $result = $phpauth->login($username, $pw1, $remember);
      $hash = $result['hash'];
      $message = $result['message'];
      
      $authData = [
        'client_id' => $_SESSION['client_id'], 
        'site_id' => $siteObj->id, 
        'provider_id' => 0, 
        'username' => $username, 
        'session_id' => session_id(), 
        'auth_token' => $hash, 
        'remote_ip' => client_ip(),
        'remote_cc' => client_cc(),
        'remote_agent' => $_SERVER['HTTP_USER_AGENT'],
        'message' => $result['message'],
        'status' => $result['status']];

      if($user_id = $result['user_id']) {
        $userObj = get_object('user', $user_id);
        $authData['user_id'] = $user_id;
        $authData['user_level'] = $userObj->user_level;
        $authData['display_name'] = $userObj->display_name ?: "{$userObj->first_name} {$userObj->last_name}";
      }
      
      list($log_id, $log_errors) = add_object('auth_log', $authData);

      if($error = $result['error']) {
        print_log(dump($result, true), "auth", LOG_LEVEL_TEST);        
        json_error($result['message']);
      }      
      $_SESSION['hash'] = $hash;

      if($log_id && $user_id){
        $site_url = site_full_url($siteObj->id);
        $update = create_manifest($user_id, $site_url);
      } 

      $response = new stdClass;
      $response->success = 1;
      $response->redirect = pick_first($data->redirect, $data->uri, '/');
      $response->message = $message;
      
      print_log(dump($result, true), "auth", LOG_LEVEL_TEST);
      echo(json_encode($response));exit;
      break;
      
    case 'user-register':
      case 'signup-form':
      //$data =  $_GET['data'] ? json_decode($_GET['data']) : NULL;
      global $u_level;
      global $u_id;
      
      $errors = array();
      $result = array();
      
      $parent_user_id = $obj_data['parent_user_id'];
      $site_id = $obj_data['site_id'];

      $data = array2obj($obj_data);
      
      $result['input'] = $data;

      $password = $data->pass = $data->password ?: $data->password1;
      if(strlen($password) < 8) json_error("The password must be at least 8 characters long, and contain at least one UPPERCASE letter, one lowercase letter, and one number");
      $username = $data->username;
      $pw1 = $data->password1;
      $pw2 = $data->password2;
      
      if($pw1 != $pw2) json_error("passwords do not match");
      
      $profileData = (array) $data;
      $profileData['client_id'] = $_SESSION['client_id'];
      $profileData['email_address'] = $profileData['username'];

      $params = []; // extra user data
      
      if($parent_user_id && $u_id == $parent_user_id && $u_level >= USER_LEVEL_OWNER) { // managed account
        $validate_email = false;
        $email = $profileData['email_address'] ?: $profileData['username'];
        if(!$email || !is_email($email)) $email = get_user_email($parent_user_id);
        $profileData['email_address'] = $email;
        
        /** todo: change per site according to config */
        $params = ['site_id' => $site_id, 'user_level' => 10, 'roles' => USER_ROLE_ARTIST, 'validate_email' => 0];
        
      } else {
        $validate_email = true;
        /*Automatically make a new user as curator - For Now*/  
        $params = ['site_id' => $site_id, 'roles' => USER_ROLE_ARTIST, 'user_level' => 20];
      }
      
      list($user_id, $errors) = user_register($username, $pw1, $profileData, $params);
      if($errors) json_error($errors);

      
      $message = $result['message'];

      $phpauth = user_phpauth();
      $result2 = $phpauth->login($username, $pw1, true);
      $hash = $result['hash'];
      if($hash) $_SESSION['hash'] = $hash;
      
      if($parent_user_id) { /** todo: change per site according to config */
        $perms = '{"vault":"edit","profile":"edit","account":"edit","login":"full"}';
        $mapData = ['site_id' => $site_id, 
                    'verified' => 1, 
                    'host_user_id' => $user_id, 
                    'user_id' => $parent_user_id, 
                    'permissions' => $perms];
        list($id, $errors) = add_object('user_map', $mapData);
        $data->redirect = page_link(PAGE_PROFILE).$user_id;
        if($errors) json_error($errors);        
      }
      
      //$message .= dump($result2, true);
      $response = new stdClass;
      $response->success = 1;
      $response->redirect = pick_first($data->redirect, $data->uri, '/');
      $response->message = $message;
      
      
      echo(json_encode($response));exit;
      break;

    case 'publish-site':
      $data =  $obj_data;
      $site_id = $obj_data['site_id'];
      $siteObj = get_object('site', $site_id);
      $base_dir = site_cache_dir($siteObj);
      $cmd = "rm -rf $base_dir"; // delete existing cache for this language
      exec($cmd);
      if(!mkdir($base_dir, 0775, true)) die("site_sitemap_html: Couldn't make $base_dir");
      if(!mkdir($base_dir.'css/font/', 0775, true)) die("site_sitemap_html: Couldn't make $base_dir".'css/font/');
      if(!mkdir($base_dir.'css/images/', 0775, true)) die("site_sitemap_html: Couldn't make $base_dir".'css/images/');
      exec("cd $base_dir;cp ../../../.htaccess .;ln -s ../../../user/; cp -ax ../../../build/ .;cp -ax ../../../afr/css/images css;cp -ax ../../../afr/css/font css;");
      $caption = "Publishing $siteObj->domain";
      $row = array('path', 'length', 'time', 'status', 'message');
      $output .= html_table(array($row), array('title' => $caption, 'class' => 'coffee'));
      echo($output); 
      die();
      break;

    case 'publish-node':
      $data =  $obj_data;
      $node =  $_GET['node'] ? json_decode($_GET['node']) : NULL;
      $item = site_spider_node($data['site_id'], $node);
      echo(json_encode($item));exit();
      break;
    
    /** old 
    case 'get-site-tree':
      extract($_GET);
      //dump($_GET);exit();
      $tree = site_sitemap($site_id, array('language' => $language ?: $_SESSION['language']));
      //dump($tree);exit();
      $count = tree_count($tree, array('search' => array('dynamic', 0)));
      $result->tree = $tree;
      $result->count = $count;
      echo(json_encode($result));exit();
      break;
    */
    
    case 'publish-page':
      //$data =  $obj_data;
      $options['overwrite'] = 1; // overwrite if just one page
      $site_id = $obj_data['site_id'];
      $content_id = $obj_data['content_id'];
      $contentObj = get_object('page_content', $content_id);
      $page_dir = clean_path(site_cache_dir($site_id).$contentObj->path);
      $cmd = "rm -rf $page_dir"; // delete existing cache for this language
      exec($cmd);

      //$output = html_div("Pubishing $siteObj->domain to $page_dir ");
      
      $item = site_spider_page($site_id, $content_id, $options);
      $output = array2table(array($item), array('class' => 'coffee', 'title' => 'Result'));
      echo($output);exit();
      //echo(json_encode($item));exit();
      break;
      
    case 'next-id':
      echo(nextId($_GET['table']));exit();        
      break;
                                            
    case 'progress-bar-create':
      $pb_id = progressbar_create($_GET['name'], $_GET['total']);
      echo($pb_id);
      die();
      break;

    case 'progress-bar':
      if($inc = $_GET['inc']) progressbar_increase_progress($_GET['id'], $inc);
      $progress = progressbar_get($_GET['id']);
      $pbObj->total = $progress->total;
      $pbObj->progress = $progress->progress;
      echo(json_encode($pbObj));
      die();
      break;

    case 'rental-import':
      if($error = check_required($obj_data, array('site_id' => "Site ID", 'property_type' => "Rental type", 'apt_id' => "Rental ID", 'source_id' => "Source ID"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      switch($source_id) {
        case SOURCE_ID_INTERHOME:
          require_once('xml/xml_interhome.inc');
          $result = interhome_import('import', $obj_data);
          break;
        default:
          json_error("Source {$source_id} not supported - please contact admin");
          break;
      }
      echo(json_encode($result));exit();
      break;
      
    // new handler used to set custom price for week or month
    case 'calendar-pricing':
      if($error = check_required($obj_data, array('site_id' => "Site ID", 'property_type' => "Rental type", 'apt_id' => "Rental ID", 'host_id' => "Host ID", 'start' => "Start", 'end' => "End"))) json_error("Missing: ".implode(', ', $error));

      extract($obj_data);
      $data = $obj_data;
      $id = $data['id']; // existing exception
      $in  = $data['start'] = date2sql($start);
      $out  = $data['end'] = date2sql($end);
      $range = print_time_range($in, $out);
      $errors = $messages = array();

      // look for existing exception
      $exSearch = array();
      $exSearch['active'] = 1;
      $exSearch['start'] = $in;
      $exSearch['end'] = $out;
      $exSearch['apt_id'] = $apt_id;
      
      if($rate_week > 0) {
        $exSearch['rate_week'] = array('>', 0);
        if($week_count == 3) {
          $rate_field = 'rate_3_weeks';
        } else if($week_count == 2) {
          $rate_field = 'rate_2_weeks';
        } else {
          $rate_field = 'rate_week';
        }
        $rate = $rate_week;
        print_log("count=$week_count field=$rate_field rate=$rate", 'calendar', LOG_LEVEL_TEST);
        
      } else if($rate_month > 0) {
        $exSearch['rate_month'] = array('>', 0);
        $rate_field = 'rate_month';
        $rate = $rate_month;
      } else {
        json_error("Required: rate");
      }

      $exObj = $id ? get_object('res_exception', $id) : find_object('res_exception', $exSearch);
      if($exObj) { // update
        $action = "Updated";
        list($id, $errors) = update_object('res_exception', array($rate_field => $rate), $exObj->id);
      } else { // add
        $action = "Added";
        $data[$rate_field] = $rate;
        list($id, $errors) = add_object('res_exception', $data);
      }
      if($errors) {
        json_error($errors);
      } else {
        $messages[] = "$action custom rate $rate for $range";
        $exObj = get_object('reservation', $id);
        $messages[] = "$obj_action $enq_type $id";
        $result->success = 1;
        $result->message = $messages;
        $result->data = $resObj;
        echo(json_encode($result));exit;        
      }            
      break;
      
    /** new handler used add/update reservation from calendar dialog */
    case 'calendar-booking':
      global $__SITE;
      if(!$_POST) json_error("POST required");
      
      /**'site_id' => "Site ID", 'property_type' => "Rental type", */
      if($error = check_required($_POST, array('status' => 'Status', 'apt_id' => "Rental ID", 'checkin' => "Checkin", 'checkout' => "Checkout"))) json_error("Missing: ".implode(', ', $error));
      $messages = [];
      $data = $_POST;
      extract($data);
      // print_log("data".dump($data,true), 'booking', LOG_LEVEL_TEST);

      if(!$site_id) $site_id = $__SITE->id;
      if(!$property_type) $property_type = $__SITE->property_type;
      
      $in  = $data['checkin'] = date2sql($checkin);
      $out  = $data['checkout'] = date2sql($checkout);
      if(!trim($first_name) && !trim($last_name)) json_error("First or last name is required");
      if(trim($phone) && !phone_valid_idd($phone)) json_error("$phone is not a valid phone number (include +country code, exclude leading zero)");

      /** for unchecked checkboxes, set to 0 */
      $checkbox_fields = ['charge_tax', 'tax_included', 'charge_city_tax', 'send_email'];
      foreach($checkbox_fields as $fld) {
        if(!isset($data[$fld])) $data[$fld] = 0;
      }
      
      $res_id = pick_first($res_id, $obj_id, $id);
      //print_log("calendar-booking:".dump($data,true), 'booking', LOG_LEVEL_TEST);
      if($res_id) {
        list($id, $errors) = update_object('reservation', $data, $res_id);
        $messages[] = "Updated Booking {$res_id}";        
      } else {
        $data['type'] = RES_TYPE_ADMIN; /** for some crazy reason, the hidden field in res_dialog does not store this */
        list($id, $errors) = add_object('reservation', $data);
        if($id) $messages[] = "Added Booking {$id}";
        
      }
      if($errors) json_error($errors);
      
      $result->success = 1;
      $result->message = $messages;
      $result->data = get_object('reservation', $id);
      echo(json_encode($result));exit;        

      break;
      
    /** new handler used to set availability/price from calendar-simple-dialog */
    case 'calendar-update':
      if($error = check_required($obj_data, array('site_id' => "Site ID", 'property_type' => "Rental type", 'status' => 'Availability', /*'apt_id' => "Rental ID",*/ 'checkin' => "Checkin", 'checkout' => "Checkout"))) json_error("Missing: ".implode(', ', $error));
      $errors = $messages = array();
      
      extract($obj_data);
      $data = $obj_data;
      $in  = $data['checkin'] = date2sql($checkin);
      $out  = $data['checkout'] = date2sql($checkout);
      $range = print_time_range($in, $out);
      
      if($out <= $in) json_error("Checkin must be before checkout");   
      if(!$out) json_error("Missing checkout");   
      if(!$in) json_error("Missing checkin");   
      
      if(!in_array($status, array(STATUS_BLOCKED, STATUS_CANCELLED))) json_error("Illegal status $status");
      
      // find intesecting reservations 
      //if($status == STATUS_CANCELLED) { // free dates
        
      $action = $status == STATUS_CANCELLED ? "free" : "block";
      $resSearch = array();
      $resSearch['status'] = array('>=', STATUS_PROPOSED);
      $resSearch['checkin'] = array('<', $out);
      $resSearch['checkout'] = array('>', $in);
      $resSearch['apt_id'] = $apt_id;
      
      $resObjs = find_objects('reservation', $resSearch);
      if($action == 'free' && !$resObjs && !$rate_day && !$min_stay && !$changeover) { // nothing to do
        json_error("Dates are already free - did you mean to set a price, minimum stay, or changeover? If not, please hit cancel.");
      }
      
      $addBlock = $action == 'block';
      foreach($resObjs as $resObj) {
        $res_in = strip_time($resObj->checkin);
        $res_out = strip_time($resObj->checkout);
        $resData = array();
        //$true = $action == 'block' && $in >= $res_in && $out <= $res_out ? "yes" : "no";
        //json_error("action = $action in=$in res_in=$res_in out=$out res_out=$out true=$true");
        
        if($resObj->status != STATUS_BLOCKED) json_error("Cannot $action $range due to conflicting reservation");
          
        if($action == 'block' && $in >= $res_in && $out <= $res_out) { // block: selection is inside existing: do nothing
          json_error("$range is already blocked");
        } else if($res_in >= $in && $res_out <= $out) { // existing is inside selection: delete (whether we free or block)
          list($count, $errors) = delete_object('reservation', $resObj->res_id);
          $messages[] = "Removed ".$resObj->res_id;
        } else {
          if($res_out > $out) {
            $addBlock = false; // update existing instead
            $new_in = $action == 'free' ? $out : $in;
            list($id, $errors) = update_object('reservation', array('checkin' => $new_in), $resObj->res_id);
            $messages[] = "Updated checkin ".$resObj->res_id;
            if($action == 'free' && $res_in < $in) {
              $resData = (array) $resObj;
              $resData['checkout'] = $in;
            }             
          } else if($res_in < $in) {
            $addBlock = false; // update existing instead
            $new_out = $action == 'free' ? $in : $out;
            list($id, $errors) = update_object('reservation', array('checkout' => $new_out), $resObj->res_id);
            $messages[] = "Updated checkout ".$resObj->res_id;
            //if($action == 'free' && $res_in < $in) {
            //  $resData = (array) $resObj;
            //  $resData['checkout'] = $in;
            //} 
          } else {
            json_error("illegal: in=$in res_in=$res_in out=$out res_out=$out");
          }

          // if we free in the middle of existing reservation, create copy on other side of gap
          if($resData) {
            list($id, $errors) = add_object('reservation', $resData);
            $messages[] = "Added block from $res_in to $in";
          } 
        }         
        if($errors) json_error($errors);          
        $obj = $resObj;          
      }
      $id = count($messages);
      //$action = "Freed";        
      
      if($addBlock) { // block dates unless overlap
        list($id, $errors) = add_object('reservation', $data);
        $action = "Blocked";
        $messages[] = "$action $range";
        $obj = $id ? get_object('reservation', $id) : null; 
      }

      if($action == 'free' && ($rate_day || $changeover || $min_stay)) { // add exception pricing

        $data = array();
        $data['apt_id'] = $apt_id;
        $data['property_type'] = $property_type;
        $data['host_id'] = $host_id ?: get_object($property_type, $apt_id, 'host_id');
        if(!$host_id && !$apt_id) json_error("Neither host nor rental specified");

        $search = $data;
        $search['start'] = ['<', $out];
        $search['end'] = ['>', $in];
        if($obj_id) $search['id'] = ['!=', $obj_id];
        if($exObj = find_object('res_exception', $search)) {
          $range = print_time_range($exObj->start, $exObj->end);
          json_error("You already have a Calendar Rule for {$range}");
        }
        
        //$data['host_id'] = $apt_id;       
        $data['start'] = $in;
        $data['end'] = $out;
        $rate_day = str_replace(" ", '', trim($rate_day));
        if(strpos($rate_day, '%') || strpos($rate_day,'+') !== false || strpos($rate_day,'-') !== false ) { // rate change
          $data['rate_change'] = str_replace("+", '', $rate_day);
        } else { // fixed rate
          $data['rate_day'] = $rate_day;
        }
        $data['description'] = $last_name ?: $notes;
        $data['changeover'] = $changeover;
        if($min_stay > 0) $data['min_stay'] = $min_stay;
        
        if($obj_id) {
          list($id, $errors) = update_object('res_exception', $data, $obj_id);
        } else {
          list($id, $errors) = add_object('res_exception', $data);
        }
        if($errors) json_error($errors);
        $obj = get_object('res_exception', $id);
        $messages[]= "Added custom pricing for $range";
      }
      
      if($id || $obj) {
        $result->success = 1;
        $result->data = $obj;
        $result->ids = array($id);
        $result->message = $messages;
      }
      //$result->error = dump($obj_data,true);
      echo(json_encode($result));exit;
      break;

    
    /** todo: use boostrap dialog with json data */
    /** no longer in user */
    case 'rental-calendar-dialog':
      
      extract($obj_data);
      $obj_id = pick_first($obj_data['id'], $obj_data['res_id'], $obj_data['obj_id']);
      
      $cal_params = $_SESSION['cal_params'];
      
      if(!$obj_type) $obj_type = $cal_params->obj_type;
      if(!$obj_type) json_error("Missing type");
      //if(!$obj_id) json_error("Missing id");
      
      print_log("load_dialog", 'calendar', LOG_LEVEL_TEST); 
      if(!$u_id) die("Not logged in");
        
      // move outside
      
      if($obj_id) { // edit
        $obj = get_object($obj_type, $obj_id);
        if(!$obj) json_error("Could not find {$obj_type} {$obj_id}");
        if(!auth_res($obj)) json_error("Not authorized");
        $obj->hourly = $obj->num_hours > 0;
      } else { // new - prepopulate object with what we know
        
        $in_h  = $_GET['checkin'];
        $out_h = $_GET['checkout'];
  
        $obj->checkin  = $in_h;
        $obj->checkout = $out_h;
        $obj->status = STATUS_PROPOSED;
        $obj->last_name = $_GET['last_name'];
        $obj->notes = $_GET['notes'];
  
        $obj->property_type   = $ptype = $_GET['property_type'];
        $obj->apt_id          = $apt_id = $_GET['apt_id'];
        $apt = get_object($obj->property_type, $obj->apt_id);
        $obj->apartment_name  = $apt->name;      
        if($obj->charge_tax = $apt->charge_tax) $obj->tax_included = $apt->tax_included;
        $host_id = $apt->host_id;
        
        $feeArray = $apt ? get_property_fees($apt) : array();
        if($cal_params->target_day > 0) {
          $units = $obj->num_hours = formatFloat(period_diff($in, $out, 'hour'));
          $rate  = $obj->rate_hour = $apt->rate_hour;
          $obj->rate_type = 'hour';
          $obj->hourly = true;
          $discount = $obj->discount = $apt->discount;
        } else { // shortstay - per day
          $units = $obj->num_days  = formatFloat(period_diff($in, $out, 'day'));
          
          /** no exceptions - this is handled after load by update_overlay_rate */
          
          $rate     = $obj->rate_day = $apt->rate_day;
          $discount = $obj->discount = $apt->discount;
                 
          $obj->rate_type = 'day';
          $obj->hourly = false;
        }
        $obj->rate_base = $rate;
        $obj->rate_extraperson = $apt->rate_extraperson;
        //$obj->currency = $apt ? get_city_currency($apt->city_id) : get_city_currency($cal_params->city_id);
        $obj->currency = 'EUR';
        foreach($feeArray as $key => $fee) $obj->$key = $fee;
        $obj->total = $total = $discount ? apply_discount($rate * $units, $discount) : $rate * $units; 
        $obj->grand_total = $total + $obj->fees;
  
        if($obj->charge_city_tax = $apt->charge_city_tax) {
          $obj->city_id = $apt->city_id; // maybe not needed, just in case
          list($tt_tax_amount, $tt_desc, $tt_quant) = get_city_tax($obj, $total);
          $obj->city_tax = $tt_tax_amount * $tt_quant;
        }
        
      }
      
      $output = res_dialog($cal_params, $obj);
      
      $response->success = 1;
      $response->output = $output;
      echo(json_encode($response));exit();      
      break;

    case 'rental-calendar-params': // get all reservations JSON data
      $cal_params = $_SESSION['cal_params'] ?: $_SESSION['cal_defaults'];
      $cal_params = get_cal_params($cal_params);
      $cal_params = calendar_init($cal_params);
      $_SESSION['cal_params'] = $cal_params; // store for use in CSS file
      echo(json_encode($cal_params));exit();
      break;
    
    /** todo: return json - use bootstrap table */
    /** no longer in use
    case 'rental-calendar-stats':
      $cal_params = $_SESSION['cal_params'];
      $cal_params->res_list = $_GET['res_list'];
      $stats = get_stats($cal_params);
      $output .= print_stats($cal_params, $stats);
      echo($output);die();   
      break;
    */
    
    /** todo: return json - use bootstrap table */
    case 'rental-calendar-changes':   // show changes
      $res_list = $_GET['res_list'];
      $obj_type = $_GET['obj_type'];
      if(!$obj_type) {echo("load_changes: missing obj_type");die();}
      if(!$res_list) {echo("load_changes: no reservations");die();}

      $res_ids = $res_list ? explode(',', $res_list) : array();
      $count = count($res_ids);
      $search['id'] = $res_ids;
      $changes = get_changes($obj_type, $search);
      $count = count($changes);
      $output = "$count $obj_type changes:";
      $output .= print_changes($changes);
      echo($output);die();
      break;

    case 'rental-calendar-get': 
      global $__SITE;
      if(!$u_id) json_error("Please log in");
      extract($obj_data);
      if(!$obj_type) json_error("Missing obj type");
      if(!$obj_id) json_error("Missing obj id");
      $obj = get_object($obj_type, $obj_id);
      if(!auth_res($obj)) json_error("Unauthorized");
      if(!$obj) json_error("Could not find {$obj_type} {$obj_id}");      
      $obj->id = $obj->obj_id = $obj_id;
      $obj->obj_type = $obj_type;
      
      if($site_id = $obj->site_id) {
        $obj->site_name = $site_id == $__SITE->id ? $__SITE->name : get_object('site', $site_id, 'name');
      }
      
      $links = [];
      $obj->range = print_time_range($obj->checkin, $obj->checkout);
      
      // if(!$obj->extended) $obj = add_calobj_helperfields($cal_params, $obj, array('extended' => true)); // load extra fields      
      // invoice action links for this reservation
      if($obj_type == 'reservation') {
        $resObj = $obj;
        $status_page = $site_status_page = get_page_link(PAGE_STATUS);
        if($site_id != $__SITE->id) $site_status_page = site_page_link($site_id, PAGE_STATUS, ['full' => true]);
        $locked_invoices = [];
        $invoice_page = get_page_link(PAGE_INVOICE);
        $obj->link = $status_page.$obj_id;
        $obj->site_link = $site_status_page.$obj_id;
        $obj->auth_level = $auth_level = auth_res($resObj);
        if($auth_level >= USER_LEVEL_HOST) {
          $invObjs = find_reservation_invoices($obj_id);
          $links = [];
          foreach($invObjs as $invObj) {
            $invoice_id = $invObj->invoice_id ?: $invObj->id;
            $lock = '';
            if($invObj->locked) {
              $locked_invoices[] = $invObj->id;
              $lock = html_i("", "fa fa-lock").' ';
            }
            $links[] = html_link($invoice_page.$invObj->id, $lock.html_i("", "fa fa-money")." Invoice $invoice_id", ["target" => "invoice"]);
          }
        }
        $resObj->guest_name = get_name_string($resObj);
        $resObj->locked_invoices = $locked_invoices;
      }

      if($links) $obj->links = $links;
      if($obj->source_id) $obj->source_name = get_object('source', $obj->source_id, 'name');
      if($obj->aff_id) $obj->aff_name = get_object('affiliate', $obj->aff_id, 'affiliate_name');
    
      $response->success = 1;
      $response->data = $obj;
      echo(json_encode($response));exit();      
      break;
      
  case 'rental-calendar-rate':
    $id = $_GET['apt_id'];
    $type = $_GET['t'] ?: get_object('property', $id, 'type');
    if(!$id) json_error("Missing id");
    if(!$type) json_error("Missing type");
    
    $aptObj = get_object($type, $id);
    $in_h  = $_GET['in'];
    $out_h = $_GET['out'];

    if(!$aptObj) json_error("Could not find {$type} {$id}");

    $in = date2sql($in_h);
    $out = date2sql($out_h);
    $priceObj = get_average_rate($aptObj, $in, $out, $_GET['ng'] ?: 1);
    $unit = $priceObj->unit;
    $units = $priceObj->num_days;
    $rate = round($priceObj->average, 2);
    $discount = $priceObj->discount;      
    unset($priceObj->prices);
    
    /** licensed vs non-licensed for Sicco */
    
    $response->success = $priceObj ? 1 : 0;
    $response->data = $priceObj;
    echo(json_encode($response));exit();
    break;
    
  case 'rental-calendar-draw':   // draw newly created booking
    $obj_id = pick_first($obj_data['id'], $obj_data['res_id'], $obj_data['obj_id']);
    if(!$u_id) json_error("Please log in");
    extract($obj_data);
    if(!$obj_type) json_error("Missing obj type");
    if(!$obj_id) json_error("Missing obj id");
    $cal_params = $_SESSION['cal_params'];
    $output = '';

    $obj = $resObj = get_object($obj_type, $obj_id);
    if(!$obj) json_error("Could not find {$obj_type} $obj_id");
    $res_id = $obj->res_id;
    if(!$res_id) json_error("Failed to add booking");
    
    $output = print_res_marker($cal_params, $obj);
    $response->message = "Added booking {$res_id}";
    $response->success = $success;
    $response->output = $output;
    echo(json_encode($response));exit();      
    break;
      
  case 'rental-calendar-help':   // show calendar stats
    $output .= print_statuses($cal_params);
    echo($output);exit();
    break;

  case 'rental-calendar-pricing':   // show calendar stats
    $exceptions = $cal_params->exceptions ?: array();
    if(!count($exceptions)) $output .= html_h4("No special pricing or rules for this period");
    
    foreach($exceptions as $ex) {
      $apt_name = $ex->apt_id ? get_object('property', $ex->apt_id, 'name') : phrase('all_rentals', CAPITALIZE);
      $items[] = html_b("$apt_name: ").print_exception($ex, $options);
    }
    $output .= html_ul($items);
    echo($output);exit();
    break;
    
            
    /** new json calendar data for reservations and exceptions, used by public and private fullcalendar */
    /** Now also used by admin calendar */
    
    case 'agenda-calendar-events':
      if($error = check_required($obj_data, array('site_id' => "Site ID", 'start' => "Start", 'end' => "End"))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      $search = array();
      $search["site_id"] = $site_id;
      $search["type"] = array(EVENT_TYPE_AGENDA, EVENT_TYPE_OTHER);
      $search["user_level"] = array("<=", $u_level);
      $search["start_time"] = array("<=",  "{$end} 00:00:00");
      $search["end_time"] = array(">=", "{$start} 00:00:00");
      $pretty = 0;
      
      $event_url = site_page_link($site_id, PAGE_EVENTS);
      
      $events = find_objects('event', $search);
      $eventAr = array();
      foreach($events as $eventObj) {
        $event = new stdClass;
        $event->data = $eventObj;
        
        // style     
        $title = $eventObj->name ?: "(No title)";

        $event->id = $eventObj->id;        
        $event->all_day = $eventObj->all_day;        
        $event->rendering = '';
        $event->className = array("fc-exception");
        $event->textColor = '#000';
        $event->backgroundColor = '#FFF';      
        $event->borderColor = 'transparent';
        
        $time = '';
        
        if($eventObj->all_day) {
          $event->start = strip_time($eventObj->start_time);
          $event->end = strip_time($eventObj->end_time);
          //$event->rendering = 'background';
          
        } else {
          $event->start = $eventObj->start_time;
          $event->end = $eventObj->end_time;
        }
        
        $event->link = site_page_link($site_id, PAGE_EVENTS).$eventObj->id;
        $event->title = $eventObj->title; //$eventObj->name;
        
        $eventAr[] = $event;
        
      }
      echo(json_encode($eventAr, $pretty));exit;
      break;
    
    /** not yet implemented - idea to return just apt_id/in/out for unaavailable dates */
    case 'calendar-availability':
    
      $property_type = 'shortstay';
      if($error = check_required($obj_data, array('apt_id' => "Rental ID", 'in' => "Checkin", 'out' => "Checkout"))) json_error("Missing: ".implode(', ', $error));
      
      extract($obj_data);
      $aptObj = get_object($property_type, $apt_id, ['calendar_availability', 'calendar_startdate']);

      if(1) { /** plan A */
        echo(json_encode($aptObj));
      } else {       /** plan B */

        $today  = today();
        $year_ago = add_day($date, -360);
        
        $wd     = get_sql_weekday($in); // day of week
        $nd     = day_diff($in, $out); // how many days
        $offset = day_diff($today, $in); // how many days from today
    
        $sql_start = $offset + 1; // mysql is 1 indexed
        $avail_fld = "substr(calendar_availability, {$sql_start} + datediff('{$today}', calendar_startdate), {$nd}) as avail";
        $fields[] = $avail_fld;
        $searchOptions = [];
        $searchOptions['fields'] = $fields; 
  
        $searchArray= [];
        $searchArray["length(calendar_availability) - datediff('{$today}', calendar_startdate)"] = ['>=', $nd]; // $cutoff;
        
        $searchArray['calendar_startdate'] = ['>=', $year_ago];
        $searchOptions['having'] = "avail not like '%N%'";
        $rentals = find_object($property_type, $searchArray, $searchOptions) ?: [];
        
              
        
        $resObjs = calendar_entries($start, $end, $apt_id, []);
        $events=[];
        foreach($resObjs as $resObj) {
          //$events[] = ['id' => $resObj->res_id, 'in' => strip_time($resObj->checkin), 'out' =>  strip_time($resObj->checkout, 'status' =>  $resObj->status];
        }
        echo(json_encode($events));
      }
      break;

    /** new simplified version of 'rental-calendar-events' (below) for public, use aptObj->calendar_availability instead of resObjs */
    case 'rental-calendar-availability':
      $apt_id = $obj_data['id'];
      $start = $obj_data['start'];
      $end = $obj_data['end'];
      
      $events = [];
      
      if($start && $end && $end > $start && $end > today()) {
        $aptObj = get_object('property', $apt_id);
        $source_id = $aptObj->source_id;
        $host_id = $aptObj->host_id;
        
        $resObjs = rental_blocked_dates($apt_id, $start, $end);
        foreach($resObjs as $resObj) {
          $events[] = ['start' => $resObj->checkin, 'end' => $resObj->checkout, 
                       'obj_id' => 0, 'obj_type' => 'reservation',
                       'rendering' => 'background', 'backgroundColor' => '#F88', 
                       'borderColor' => 'transparent', 'startEditable' => 0];
        }
  
        $get_rates = $obj_data['get_rates']; /** todo */
        $get_rates = 1; 
        if($get_rates) {
          $cur = '€'; // hard coded for now           
          $exObjs = rate_exceptions($start, $end, $host_id, $apt_id);
          $rate_keys = rate_exception_keys();

          foreach($exObjs as $exObj) {
            $event = [];
            foreach($rate_keys as $k) if($exObj->$k) $event[$k] = $exObj->$k;
            print_log("keys:".dump($rate_keys,true), 'booking', LOG_LEVEL_TEST);
            print_log("boj".dump($exObj,true), 'booking', LOG_LEVEL_TEST);
            if($event) {
              $events[] = array_merge($event, ['start' => $exObj->start, 'end' => $exObj->end, 
                         'obj_id' => $exObj->id, 'obj_type' => 'res_exception',
                         'rendering' => 'background', 'backgroundColor' => 'transparent', 
                         'borderColor' => 'transparent', 'startEditable' => 0]);
            }
          }
        }
      }
      $result = new stdClass;
      $result->success = 1;
      $result->events = $events;
      echo(json_encode($result, $pretty));exit;
      break;
      
    /** used by multi and single (edit) calendar, not by public calendar (uses rental-calendar-availability) 
       NO LONGER IN USE - use api_calendar */  
    case 'rental-calendar-events':      
      if(!$__USER) json_error("Please log in");
      if($__USER->user_level < USER_LEVEL_HOST) json_error("Not authorized");
      if($error = check_required($obj_data, array('host_id' => "Host ID", 'start' => "Start", 'end' => "End"))) json_error("Missing: ".implode(', ', $error));
      $result = api_calendar($obj_data);
      echo(json_encode($result, $pretty));exit;

      /** old code below */
      
      $pretty = isset($obj_data['pretty']) ? JSON_PRETTY_PRINT : 0;
      $id = $obj_data['apt_id'] ?: $obj_data['id'];
      $apt_id = list2array($id);
      $host_id = list2array($obj_data['host_id']);

      $start = $obj_data['start'];
      $end = $obj_data['end'];      
      $edit = $obj_data['edit'] ? true : false; // set edit=1 to allow editing
      
      $debug = isset($_GET['debug']) && $_GET['debug']; 
      $auth_host_ids = user_host_list($__USER->id);  
      
      if($diff = array_diff($host_id, $auth_host_ids)) json_error("Host not authorized:".dump($diff,true));
      
      $search = [];
      $search['host_id'] = $host_id; /** mandatory */
      if($apt_id) $search['apt_id'] = $apt_id;

      
      $exObjs = $resObjs = $invObjs = $eventObjs = [];

      $options = [];
      $status = $obj_data['status'] ?: STATUS_PROPOSED;
      $search['status'] = ['>=', $status];
      $resObjs = calendar_entries('reservation', $start, $end, $search, $options);
      $exObjs = calendar_entries('res_exception', $start, $end, $search, $options);
      //$exObjs  = date_exceptions($start, $end, $host_id, $apt_id, $options);

      $invObjs = $multi_calendar ? calendar_entries('invoice', $start, $end, $search, $options) : [];
      
      $events = array_merge($resObjs,$exObjs,$invObjs);
      
      $result = new stdClass;
      $result->count = count($events);
      $result->event_count = count($eventObjs);
      $result->reservation_count = count($resObjs);
      $result->invoice_count = count($invObjs);
      $result->exception_count = count($exObjs);
      $result->success = 1;
      $result->events = $events;
      echo(json_encode($result, $pretty));exit;
      
      
      
      /** no longer in use, at least for multi calendar */
      
      $events = array();
      
      $bookings = '';
      foreach($resObjs as $resObj) {
        
        $auth_rental = $resObj->apt_id ? $edit && auth_rental($resObj->apt_id, $userObj) : 1;
        $auth_res = $edit && $auth_rental && auth_res($resObj, $userObj);
        
        $typeObj = $pTypeAr[$resObj->property_type];
        $in = $typeObj->hourly ? $resObj->checkin :  strip_time($resObj->checkin);
        $out = $typeObj->hourly ? $resObj->checkout :  strip_time($resObj->checkout);
        $status_name = strtolower(get_object('res_status', $resObj->status, 'name'));
  
        $event = new stdClass(); //$resObj;
        $event->id = $res_id = $event->obj_id = $resObj->res_id;
        $event->apt_id = $resObj->apt_id;
        $event->obj_type = "reservation";
        $event->start = $in;
        $event->end = $out;
        $event->auth_rental = $auth_rental;
        $event->auth_res = $auth_res;
        $event->status_name = $status_name;

        /** for debugging */
        $event->user_id = $userObj->id;
        $event->user_level = $userObj->user_level;
        $event->host_id = $resObj->host_id;
        //$event->auth_res_bare = auth_res($resObj, $userObj);
        /** end for debugging */
  
        if($userObj->user_level == USER_LEVEL_CONTRACTOR) $auth_res = 0; /** quick hack for contractors for now */
        
        // text
        if($auth_res) {
          $title = pick_first(trim($resObj->last_name), trim($resObj->first_name), ucfirst($status_name));
          if($resObj->num_guests) $title .= " ($resObj->num_guests)";
          $event->title = $title;
          $event->description = $resObj->notes ?: 'no desc';
          foreach(array('created', 'timestamp', 'status', 'guest_id', 'grand_total', 'currency', 'paid', 'balance', 'type', 'notes','extra_info','alert') as $fld) $event->$fld = $resObj->$fld;
          $event->guest_name = get_name_string($resObj);
          
          /** tips */
          $tips = '';

          $expires = $resObj->expires;
          if($auth_edit && $expires && $expires > now()) {
            $tips .= html_tooltip("Expires: ".date("Y-m-d H:i:s", strtotime($expires)), 'time', array('bootstrap' => 1, 'class' => 'cal-alert tooltip-jq', 'style' => 'time'));    
          }
          
          if($auth_edit) {
            $not_paid = find_objects('payment', array('res_id' => $res_id, 'payment_status' => array("!=", PAYMENT_STATUS_COMPLETED)), array('fields' => 'distinct payment_status'));
            if($not_paid) {
              $not_paid = array_unique($not_paid);
              foreach($not_paid as $payment_status) {
                $ps_name = get_object('payment_status', $payment_status, 'name');
                $tips .= html_tooltip("Payment $ps_name", 'money', array('bootstrap' => 1, 'class' => 'cal-alert tooltip-jq', 'style' => 'money'));
              }
            }
          }

          if($mismatch = $auth_edit ? reservation_invoice_mismatch($resObj) : false) {
            $classes[] = "mismatch";
            $cur = get_currency_symbol($resObj->currency, true);
            //$tips .= tooltip('tip-inv-'.$id , "Invoice mismatch: $mismatch", array('data-tip' => 'foo', 'bootstrap' => 1, 'class' => 'cal-alert tooltip-jq', 'style' => 'error'));
            $tips .= html_tooltip("Invoice mismatch: $cur $mismatch", 'error', array('bootstrap' => 1, 'class' => 'cal-alert tooltip-jq', 'style' => 'error'));
        
          }
          
          
          if($resObj->alert) $tips .= html_tooltip($resObj->alert, 'info', array('bootstrap' => 1, 'class' => 'cal-alert tooltip-jq', 'style' => 'info'));
          
          $event->tips = $tips;
              
        } else { /** view only */
          $status_name = 'blocked';
          //$event->status = STATUS_BLOCKED;
          $event->status = $resObj->status; //STATUS_BLOCKED;
          $event->title = $event->description = '';
          //$event->status_name = $status_name;

          $title = pick_first(trim($resObj->last_name), trim($resObj->first_name), ucfirst($status_name));
          if($resObj->num_guests) $title .= " ($resObj->num_guests)";
          $event->title = $title;
          $event->description = $resObj->notes ?: 'no desc';
          $event->auth_res = 0;
          $event->auth_view = 1;          
        }
        
        // permissions
        $event->startEditable = $event->editable = $auth_res ? 1 : 0;
        
        // style
        $event->className = array("fc-reservation", "fc-status-$status_name");
        $blocked = $resObj->status == STATUS_BLOCKED;
        
        $event->rendering =  !$auth_res ? 'background' : '';
        //$event->rendering =  !$auth_res || $blocked ? 'background' : '';
        $status = $event->status;
        
        switch($status) {
          case STATUS_CANCELED:
            $bg_color = '#888';
            $status_name = 'fc-status-cancelled';
            break;
          case STATUS_PROPOSED:
            $bg_color = '#06F';          
            $status_name = 'fc-status-proposed';
            break;
          case STATUS_BLOCKED:
            $bg_color = '#F88';         
            $status_name = 'fc-status-blocked';
            break;
          default:
            $bg_color = '#000';          
            $status_name = 'fc-status-cancelled';
            break;          
        }
        
        if(!$event->status_name) $event->status_name = $status_name;
        print_log("$in-$out: event status=$status ($bg_color) name=$status_name en: {$event->status_name}", 'debug', LOG_LEVEL_TEST);
        $event->backgroundColor = $bg_color;
        
        $events[] = $event;
      }
      
      
      foreach($exObjs as $exObj) {
        $in = $typeObj->hourly ? $resObj->checkin :  strip_time($resObj->checkin);
        $out = $typeObj->hourly ? $resObj->checkout :  strip_time($resObj->checkout);
  
        $event = $exObj; // copy entire exception to event 
        $event->id = $event->obj_id = $exObj->id;      
        $event->obj_type = "res_exception";
  
        $auth_rental = auth_rental($exObj->apt_id);
        if($exObj->rate_week || $exObj->rate_month || !$auth_rental) {
          $event->title = '';
          $event->rendering = 'background';
          $event->backgroundColor = 'transparent';      
          $event->borderColor = 'transparent';         
        } else { // authorized, not weekly/monthly
            
          // event title
          $cur = '€'; // hard coded for now 
          $event->title = print_exception_text($exObj, $cur);
          
          // permissions
          $event->startEditable = true;
          
          // style
          $event->rendering = '';
          $event->className = array("fc-exception");
          $event->textColor = $edit ? '#000' : 'transparent';
          $event->backgroundColor = $edit ? '#AFA' : 'transparent';      
          $event->borderColor = 'transparent';      
        }
        
        // debug
        $event->host_id = $userObj->id;
        
        $events[] = $event;
      }
      
      echo(json_encode($events, $pretty));exit;
      break;

    // new handler used before bookings to get price + availability
    case 'rental-availability':
      $errors = $messages = array();
      $siteObj = $__SITE;
      $site_id = $siteObj->id;
      
      $property_type = pick_first($inputVars['t'], $inputVars['property_type'], $siteObj->property_type, 'shortstay'); 

      extract($obj_data);
      if(!$guest_id) $guest_id = $u_id;
      if(!$ng) $ng = 1;
      if($error = check_required($obj_data, array('apt_id' => "Rental ID", 'in' => "Checkin", 'out' => "Checkout"))) json_error("Missing: ".implode(', ', $error));
            
      $aptObj = get_object($property_type, $apt_id);

      $avail = calendar_availability($aptObj, $in, $out); /** uses calendar_availability field - no extra DB lookup */ 
      if(!$avail) json_error(phrase('%s is already booked %s', CAPS, $aptObj->name, print_time_range($in,$out)));
      
      //json_error("ok, avail=$avail");
      
      $name = $aptObj->name;
      $max_guests = $aptObj->max_guests;      

      // check availability
      $in = date2sql($in);
      $out = date2sql($out);
      
      // apply exceptions
      $exceptions = date_exceptions($in, $out, $aptObj->host_id, $apt_id);
      if($exceptions) { 
        $aptObj = apply_exceptions($aptObj->type, $apt_id, $exceptions, ['in' => $in, 'out' => $out, 'ng' => $ng]); // apply
        /**
        //json_error(print_exceptions($aptObj->type, $apt_id, $exceptions));
        $d = [];
        $d[] = "in:$in, out:$out";
        
        foreach($exceptions as $e) $d[] = "({$e->id}) {$e->start}-{$e->end}: {$e->min_stay}";
        json_error($d);
        json_error(count($exceptions)." exceptions: Ms={$aptObj->min_stay}");
        */
      }
      
      list($avail, $reason) = rental_availability($aptObj, ['in' => $in, 'out' => $out, 'ng' => $ng]);
      
      if(!$avail) json_error($reason ?: phrase('%s is not available from %s to %s', CAPITALIZE, $name, $in, $out));

      $priceObj = get_average_rate($aptObj, $in, $out, $ng);
      if($error = $priceObj->error) json_error($error);
      
      $result->success = 1;
      $result->message = "Yes!".dump($priceObj,true);
      $result->message = '';
      $result->data = $priceObj;
      echo json_encode($result);exit();				        
      break;      
      

    case 'rental-request':
    case 'rental-inquiry':
    case 'rental-booking':
      extract($obj_data);
      global $u_id, $__USER;
      $site_id = $_SESSION['site_id'];
      $message = "";
      if(!strip_blank($extra_info)) json_error("Please add a personal message to the host.");
      if($error = check_required($obj_data, ['in' => "Checkin", 'out' => "Checkout", 'ng' => "Number of guests"])) json_error("Missing: ".implode(', ', $error));

      $in = date2sql($in);
      $out = date2sql($out);
  
      // Check if available
      $aptObj = get_object($property_type, $apt_id);
      if(!$aptObj) json_error("Rental {$apt_id} not found ({$property_type}})");
      
      list($avail, $reason) = rental_availability($aptObj, ['in' => $in, 'out' => $out, 'ng' => $ng]);
      if(!$avail) json_error([phrase('%s is not available from %s to %s', CAPITALIZE, $name, $in, $out), $reason]);
      
      if(!$u_id) json_error("Not logged in");      
      if($u_id == $aptObj->host_id) json_error("You cannot book your own property");

      $res_id = $obj_data['res_id'];
      
      $resData = search2res($obj_data);
      $resData['guest_id'] = $u_id; /** copy address from guest - pre_add_reservation is too late, address already set */
      $addressData = get_address_data($__USER);
      foreach($addressData as $k => $v) $resData[$k] = $v;        
      $book_direct = 0;
                    
      switch($oper) {
        case 'rental-request':
          $type = RES_TYPE_DIRECT;
          $status = STATUS_REQUEST;
          break;
        case 'rental-inquiry':
          $type = RES_TYPE_INQUIRY;
          $status = STATUS_INQUIRY;
          break;
        case 'rental-booking':
          $book_direct = 1;
          $type = RES_TYPE_DIRECT;
          $status = STATUS_PROPOSED;
          if($resData['type'] != RES_TYPE_DIRECT) json_error("This rental is not directly bookable");            
          break;
        default: 
          json_error("Unknown operand $oper");
          break;
      }
      
      $resData['type'] = $type;
      $resData['status'] = $status;
      $resData['extra_info'] = $extra_info;
      $resData['send_email'] = 1;
      $resData['dest_id'] = $dest_id = $obj_data['dest_id'];

      $ng = $resData['num_guests'];
      
      if(!$res_id) { // reload ?
        $resObj = find_object('reservation', ['guest_id' => $u_id, 'status' => $status, 'apt_id' => $apt_id, 'checkin' => "{$in}*", 'checkout' => "{$out}*"], 'res_id');
        $result->message = "Found existing booking...";
        if($resObj) $res_id = $resObj->res_id; 
      }
      
      if(0) { // test
        if($res_id) {
          json_error("Ok, updating booking {$res_id} dest: {$dest_id} type:{$type} status:{$status} oper:{$oper} in:{$in} out:{$out} ng:{$ng}");
        } else {
          json_error("Ok, adding booking dest: {$dest_id} type:{$type} status:{$status} oper:{$oper} in:{$in} out:{$out} ng:{$ng}");
        }
      }
      
      
      if($res_id) {
        $message = "Updated {$res_id} ";
        list($res_id, $errors) = update_object("reservation", $resData, $res_id);
      } else {
        list($res_id, $errors) = add_object("reservation", $resData);
      }
      if($errors) json_error($errors);
      
      $result->success = 1;
      $result->message = $messsage."Please wait...";
      $result->data = get_object('reservation', $res_id);
        
      if($book_direct) {
        $invObj = find_booking_invoice($res_id);
        if(!$invObj) json_error("We could not find an invoice for this booking - please contact support. Booking ID: {$res_id}");        
        $result->redirect = site_page_link($site_id, PAGE_PAYMENT).$invObj->id;
      } else {
        $result->redirect = site_page_link($site_id, PAGE_STATUS).$res_id;
      }
      echo(json_encode($result));exit();    
      break;
  
    case 'booking-cancel':
      extract($obj_data);
      $data = $obj_data;
  
      if(!strip_blank($cancel_reason)) json_error("Please provide a reason for cancelling.");
      if($error = check_required($obj_data, array('user_id' => "User ID", 'res_id' => "Reservation ID", "cancel_reason" => "Cancellation Reasaon"))) json_error("Missing: ".implode(', ', $error));
      if($u_id != $user_id) json_error("User mismatch");
      
      $data['status'] = STATUS_CANCELED;
      $data['cancel_reason'] = trim($cancel_reason);
      $data['cancel_user_id'] = $user_id;
  
      list($id, $errors) = update_object("reservation", $data, $res_id);
      if($errors) json_error($errors);
      $result->success = 1;
      echo(json_encode($result));exit();    
      break;
      
    case 'set-invoice-id':
      $inv_id = $obj_data['inv_id'];
      if(!$inv_id) json_error("Missing inv_id");
      $invObj = get_object('invoice', $inv_id);
      if($invObj->invoice_id) json_error("Invoice ID is already set");
      
      $legalObj = get_object('legal_entity', $invObj->legal_entity_id);
      if(!$legalObj) json_error("Legal entity missing");
      $invoice_id = set_invoice_id($invObj->id, $legalObj);
      if(!$invoice_id) json_error("No invoice ID received");
      
      $result->success = true;
      $result->data = $invoice_id;
      echo json_encode($result);exit;      
      break;

    case 'add-invoice':
      $inv_id = $obj_data['inv_id'];
      if(!$inv_id) json_error("Missing inv_id");
      
      $result = copy_invoice($inv_id);
      $invoice_link = page_link(PAGE_INVOICE);
      $result->redirect = $invoice_link.$result->id;
      echo json_encode($result);exit;      
      break;

    case 'preview-sharing':
      //$output = "<h2>Preview:</h2>";
      $output .= sharing_output($obj_data);
      echo($output);exit();
      break;

    case 'new-sharing': // todo: use load_function
      echo(site_share_form($_GET)); die();
      break;

    case 'add-article':
      $obj_type = 'article';
      $articleData = $obj_data;
      $type = $articleData['type'];
      switch($type) {
        case ARTICLE_TYPE_BLOG:
          $page_type = PAGE_BLOG;
          break;
        case ARTICLE_TYPE_NEWS:
          $page_type = PAGE_NEWS;
          break;
        default;
          json_error("Unsupported article type $type");
      }
      
      if($id = $articleData['obj_id'] ?: $articleData['id']) {
        list($id, $errors) = update_object($obj_type, $articleData, $id);
        $action = 'Updated';
      } else {
        list($id, $errors) = add_object($obj_type, $articleData);
        $action = 'Added';
      }
      
      if($errors) json_error($errors);
      $redirect = site_page_link($siteObj, $page_type).$id;
      $result->data = get_object('article', $id);
      $result->success = 1;
      $result->message = "$action article $id";
      $result->redirect = $redirect;
      echo json_encode($result);exit();                
      break;
      
    case 'find-artists':
      $val = trim($obj_data['val']);
      //if($error = check_required($obj_data, array('site_id' => "Site ID", 'user_id' => "User ID"))) json_error("Missing: ".implode(', ', $error));
            
      if(!$app->user_id) json_error("not logged in");
     
      $artist_ids = ($app->user_level < USER_LEVEL_ADMIN) ? user_artist_list($app->user_id) : find_objects('user', ['user.site_id' => $app->site->id, 'roles & '.USER_ROLE_ARTIST => ['>', 0]], ['fields' => 'user.id']); 

      print_log("find-artist ul={$app->user_level} ids:".dump($artist_ids,true), 'backend', LOG_LEVEL_TEST);
      $rows = [];

      if($val) {
        $search[DB_LOGICAL_OR] = 
        array('first_name' => "*$val*", 
              'last_name' => "*$val*", 
              'display_name' => "*$val*", 
              // 'organization' => "*$val*", 
              'user_id' => "*$val*");
        $search['site_id'] = $app->site_id;
        $search['user_id'] = $artist_ids;
        print_log("artist search artist ids for {$user_id}".dump($search ,true), 'debug', LOG_LEVEL_TEST);
        print_log("artist ids for {$user_id}".dump($artist_ids,true), 'debug', LOG_LEVEL_TEST);
        $rows = find_objects('user_profile', $search, ['fields' => ['user_id', 'display_name', 'first_name', 'infix', 'last_name', 'city', 'country']]);
        
      }
      $result->data = $rows;
      $result->success = 1;
      echo json_encode($result);exit();
      break;

    case 'remove-artist':
      $artist_id = $obj_data['artist_id'];
      $page_id = $obj_data['page_id'];
      $primary = $obj_data['primary'];

      if($page_id){
        $getPage = find_object('minisite_page', ['id' => $page_id, 'active'=>1]);
        if($primary){
          $artistArr = json_decode($getPage->search, true);
          $getPage->search = json_encode(array_diff($artistArr, array($artist_id)));
        }else{
          $artistArr = json_decode($getPage->search1, true);
          $getPage->search1 = json_encode(array_diff($artistArr, array($artist_id)));
        }
        $output= update_object('minisite_page', $getPage, $getPage->id);
        if($errors) json_error($errors);
      }else{
        json_error("Missing Page Id");
      }

      $result->success = 1;
      $result->message = "Artist will be removed from website";
      echo(json_encode($result));
      exit;
      break;

    case 'delete-artist':
      $artist_id = $obj_data['artist_id'];
      
      if($artist_id){
        $getUser = get_user($artist_id);
        if($getUser){
          $getUser->active = 0;
          $output= update_object('user', $getUser, $getUser->id);
        }

        $getProfile = find_object('user_profile', ['user_id' => $artist_id]);
        if($getProfile)
          list($delete_profile, $errors)  = delete_object('user_profile', $getProfile->id);

        if($errors) json_error($errors);
      }else{
        json_error("Missing Artist Id");
      }

      $result->success = 1;
      $result->message = "Artist will be deleted";
      echo(json_encode($result));
      exit;
      break;

    case 'submit-order':
      $row = $obj_data['val'];
      $obj_type = $obj_data['obj_type'];
      $i = 1;
      foreach($row as $rows) {
        $data = find_object($obj_type, ['id' => $rows]);
        if(($obj_type == 'media_collection_map') || ($obj_type == 'exhibition_map') )
          $data->rank = $i;
        else
          $data->row_order = $i;

        $output = update_object($obj_type, $data, $data->id);
        $i++;
      }
      if($errors) json_error($errors);

      $result->success = 1;
      $result->message = "Re-ordered Successfully";
      echo(json_encode($result));
      exit;
    break;

    case 'event-delete':
      $id = trim($obj_data['val']);
      $obj_type = 'event';
      if($id){
        list($delete_count, $errors) = delete_object($obj_type, $id);
        if($errors) write_file($log_file, "Delete errors:".dump($errors, true), 'a'); // "\n------------\n\n"
      }
      $result->message = "Event Deleted";
      echo (json_encode($result));
      exit;
      break;

    case 'artwork-media-delete':
      $id = trim($obj_data['obj_id']);
      $obj_type = trim($obj_data['obj_type']);
      if($id){
        $data = find_object($obj_type, ['id' => $id]);
        $media_id = $data->media;
        if($media_id)
          list($delete_media, $errors) = delete_object('media', $media_id);
          list($delete_artwork, $errors) = delete_object($obj_type, $id);
        if($errors) write_file($log_file, "Delete errors:".dump($errors, true), 'a'); // "\n------------\n\n"
      }
      $result->message = "Media Deleted";
      echo (json_encode($result));
      exit;
      break;

    case 'collection-media-delete':
      $id = trim($obj_data['obj_id']);
      $collection_id = trim($obj_data['collection_id']);
      //dump($obj_data);exit;
      $obj_type = trim($obj_data['obj_type']);
      if($id){
        $data = find_object($obj_type, ['id' => $id]);
        $media_id = $data->media;
        if($media_id)
          list($delete_media, $errors) = delete_object('media', $media_id);
          list($delete_artwork, $errors) = delete_object($obj_type, $id);
          //=====================Update Exhibition Sequence=====vignesh 03-09-2020======================//
            $media_collection = db_getAll("SELECT id FROM media_collection WHERE id=".$collection_id." and subtype IN (40,50)");   
            //dump($media_collection);exit;
            foreach($media_collection as $val)
            {
              $media_collection_map = db_getAll("SELECT  *,min(original_year) as year FROM media_collection_map WHERE media_collection_id=".$val->id." AND original_year !='' GROUP BY media_collection_id");
              if(count($media_collection_map) == '1')
              {
                foreach($media_collection_map as $mval)
                {
                    $year = "".$mval->year."-01-01";
                    update_object('media_collection', ['exhibition_sequence' => $year], $mval->media_collection_id);
                  }
              }
              else
              {
                    $year = "1000-01-01";
                    update_object('media_collection', ['exhibition_sequence' => $year], $collection_id);
              }        
            }
            //=====================Update Exhibition Sequence=====vignesh 03-09-2020 End======================//
        if($errors) write_file($log_file, "Delete errors:".dump($errors, true), 'a'); // "\n------------\n\n"
      }
      $result->message = "Media Deleted";
      echo (json_encode($result));
      exit;
      break;

    case 'page-delete':
      $id = trim($obj_data['val']);
      $obj_type = 'minisite_page';
      $obj = get_object($obj_type, $id);
      $minisite_id = $obj->minisite_id;
      $del_row = $obj->row_order;
      if($id)
        list($d_id, $errors) = delete_object($obj_type, $id);

      if($errors) json_error($errors);
      else{
        $change_rows = find_objects('minisite_page', ['minisite_id'=> $minisite_id, 'active'=>1]);
        $count = count($change_rows);
        foreach($change_rows as $change_row){
          $row_order = $change_row->row_order;
          if($row_order > $del_row)
            $change_row->row_order = $row_order-1;
          $update_row = update_object('minisite_page', $change_row, $change_row->id);
          if($errors) json_error($errors);
        }
      }
      
      $result->success = 1;
      $result->message = "Pages Deleted";
      echo (json_encode($result));
      exit;
      break;

    case 'single_image':
      $page_id = trim($obj_data['page_id']);
      $obj_type = 'exhibition_map';
      if($obj_data['type'] == 'single')
        $get_exhibition = find_object($obj_type, ['page_id' =>$page_id, 'active' =>1, 'type'=>'single']);
      else
        $get_exhibition = find_object($obj_type, ['id' =>$page_id, 'active' =>1, 'type'=>'manual']);

      if($page_id){
        list($delete_id, $errors) = delete_object($obj_type, $get_exhibition->id);
      }
      if($errors) json_error($errors);

      $obj = get_object($obj_type, $delete_id);
      $result->success = 1;
      $result->data = $obj;
      $result->message = "Exhibition Deleted";
      echo (json_encode($result));
      exit;
      break;

    case 'add-artist':
      
      $obj_type = 'user';
      $userData = $obj_data;
      extract($userData);

      if($error = check_required($obj_data, array('site_id' => "Site ID", 'first_name' => "First Name", 'last_name' => "Last Name"))) json_error("Missing: ".implode(', ', $error));
      $host_user_id = $obj_data['aff_id']?$obj_data['aff_id']:$u->id;

      $ts = time();
      $userData['display_name'] = $name = trim($first_name.(trim($infix) ? " ".trim($infix)." " : " ").$last_name);
      $userData['username'] = $username = slugify($name)."_{$ts}"; /** ensure uniqueness */
      $userData['user_level'] = 1;
      $userData['auto_password'] = 1;
      $userData['roles'] = USER_ROLE_ARTIST;
      $userData['phone'] = $phone;        
      $password = "Auto_".random_int(1000,9999);; // generate random password
      
      list($user_id, $errors) = user_register($username, $password, $userData);
      if($errors) json_error($errors);
              
      $action = 'Added';
      
      if($errors) json_error($errors);
      //$redirect = site_page_link($siteObj, $page_type).$id;

      /** link users */
      $perms = '{"vault":"edit","profile":"edit","account":"edit","login":"full"}';
      $mapData = ['site_id' => $site_id, 
                  'verified' => 1, 
                  'host_user_id' => $user_id, 
                  'user_id' => $host_user_id, 
                  'permissions' => $perms];
      list($id, $errors) = add_object('user_map', $mapData);
      if($errors) json_error($errors);

      $result->data = get_user($user_id);
      $result->success = 1;
      $result->message = "$action artist {$name} ($user_id)";
      //$result->redirect = $redirect;
      echo json_encode($result);exit();                
      break;

    case 'save-plugin':
      $obj_type = 'plugin';
      $pluginData = $obj_data;
      if($id = $pluginData['obj_id']) {
        list($id, $errors) = update_object($obj_type, $pluginData, $id);
        $action = 'Saved ';
      } else {
        list($id, $errors) = add_object($obj_type, $pluginData);
        $action = 'Added';
      }
      
      if($errors) json_error($errors);
      
      //$result->redirect =  = reverse_route($__APP, 'plugin:id', ['id' => $id, 'name' => slugify($obj->name)]);
      
      $result->data = get_object('plugin', $id);
      $result->success = 1;
      $result->message = "{$action} plugin \"".($obj_data['name'] ?: $obj_data['screen_title']).'"';
      echo json_encode($result);exit();          
      
      break;
    
    case 'add-rental':
    case 'add-property':
      $data = $obj_data;
      global $u_id;
      if(!$u_id) json_error("Please log in or register first");
      if($error = check_required($obj_data, 
        array(
        'obj_type' => "Object type", 'user_id' => "User ID", 'site_id' => "Site ID", 'name' => "Name", 
        'hosting_property_type' => "Rental Type", 'hosting_room_type' => "Room Type"
        ))) json_error("Missing: ".implode(', ', $error));
      
      $obj_type = $data['obj_type'] ?: json_error("No object type received");;;
      $user_id = $data['user_id'] ?: json_error("No user ID received");;
      $site_id = $data['site_id'] ?: json_error("No site ID received");
      $name = $data['name'] ?: json_error("No name received");
      $siteObj = get_object('site', $site_id);
      $client_id = $data['client_id'];
      $legal_id = $data['legal_entity_id'];
      $host_id = $data['host_id'];
      
      if(!$client_id) $client_id = $siteObj->client_id ?: json_error("No client ID for $siteObj->domain");
      if(!$legal_id) $legal_id = find_object('legal_entity', array('client_id' => $client_id, 'default_entity' => 1), 'legal_entity.id') ?: json_error("No legal ID found for client $client_id");;
      
      print_log("add-property data: ".dump($data,true), 'rental', LOG_LEVEL_DEV);
      if($data['location_lat'] && $data['location_long']) {
          $mapObj = array2obj($data);
      } else {
        // geocode 
        require_once('geocode.inc');      
        $mapObj = geocode_address(array2obj($data));
        if($mapObj->status != 200) {
          json_error("We couldn't find this address in the map.");
        } else {
          $data['location_lat'] = $mapObj->location_lat;
          $data['location_long'] = $mapObj->location_long;
          $data['location_accuracy'] = $mapObj->location_accuracy;
          $data['location_bounds'] = $mapObj->location_bounds;
        }
      }
      $city_id = rental_find_city($data['address_country'], $mapObj) ?: json_error("Could not find a city in the database for this address");
      // $shortname = property_shortname($obj_type, $city_id, $name) ?: json_error("Failed to generate URL for $obj_type $name in $city_id");;
      $data['shortname'] = slugify($name);
      //json_message("site=$site_id client=$client_id legal=$legal_id lat=$data[location_lat] long=$data[location_long] acc=$data[location_accuracy]");
      $data['client_id'] = $client_id;
      $data['legal_entity_id'] = $legal_id;
      $data['city_id'] = $city_id;
      $data['host_id'] = $host_id ?: $user_id;
      //json_error(dump($data,true));
      
      
      list($id, $errors) = add_object($obj_type, $data);
      if($id && !$errors) {
        $redirect = ($edit_link = site_page_link($siteObj, PAGE_EDIT_PROPERTY)) ? "{$edit_link}{$id}" : '';

        $userObj = get_object('user', $user_id);
        if(!user_has_role($userObj, USER_ROLE_HOST)) user_add_role($userObj, USER_ROLE_HOST);  // promote to host (owner) 

        $result->success = 1;
        $result->redirect = $redirect;
        $result->data = get_object($obj_type, $id);
          
      } else {
        $result->success = 0;
        $result->error = $errors;
      }
      echo json_encode($result);exit();          
      break;


      
    case 'load_catering':
      $li_id = $_GET['lineitem_id'];              
      $supplier_id = $_GET['supplier_id'];
      $res_id = $_GET['res_id'];                
      if(!$li_id) {
        $output = "Missing lineitem ID";
      } elseif(!$res_id) {
        $output = "Missing reservation ID";
      } elseif(!$supplier_id) {
        $output = "Missing supplier ID";
      } else {
        $output = catering_form($res_id, $li_id, $supplier_id);
      }
        
      echo($output);exit();
      break;

    case 'load_event_form':
      $eventObj = ($event_id = $_GET['event_id']) ? get_object('event', $event_id) : NULL;
      $resObj = ($res_id = $_GET['res_id']) ? get_object('reservation', $res_id) : NULL;
      if(!$res_id) {
        $output = "Missing reservation ID";
      } else {
        $output = reservation_event_form($resObj, $eventObj);
      }
        
      echo($output);exit();
      break;


    case 'print_host_exceptions':
      //$exceptionObj = ($exception_id = $_GET['id']) ? get_object('res_exception', $exception_id) : NULL;
      $type = $_GET['property_type'] ?: $_GET['type']; // required
      $apt_id = $_GET['apt_id']; // required
      
      if(!$type) {
        $output = "Missing property type";
      } elseif(!$apt_id) {
        $output = "Missing property ID";
      } else {
        $aptObj = get_object($type, $apt_id);
        //if(!$host_id && $apt_id) $host_id = get_object($type, $apt_id, 'host_id');
        $output = print_host_exceptions($type, $aptObj->host_id, $aptObj->city_id, $apt_id);
      }
        
      echo($output);exit();
      break;

    case 'load_events':
      $res_id = $_GET['res_id'];                
      if(!$res_id) {
        $output = "Missing reservation ID";
      } else {
        $resObj = get_object('reservation', $res_id);
        $output = reservation_events($resObj, $hidden_fields);
      }
        
      echo($output);exit();
      break;

    case 'event-save': // also used for update
    case 'add_event': // also used for update
      $obj_type = 'event';
      if(strlen($obj_data['description']) && !$event_data['description']) $event_data['description'] = $obj_data['description']; /** event_form on evolution sends 'description' */

      if($error = check_required($obj_data, array('site_id' => "Site ID", 'title' => "Title", 'start_time' => "Start date/time", 'end_time' => "Start date/time"))) json_error("Missing: ".implode(', ', $error));

      
      /** todo: date2sql()  - event-form does this in javascript now */

      /** for all day, displayed out date is date before stored out date */
      if($all_day = $obj_data['all_day']) {
        $start = $obj_data['start_time'] = strip_time($obj_data['start_time']);
        $end = $obj_data['end_time'] = add_day(strip_time($obj_data['end_time']), 1);
      }
              
      //json_error("All day=$all_day start=$start end=$end");
      //json_error(dump($event_data,true));
      //$event_data = array_copy($obj_data, array('title', 'event_id', 'event_notes' => 'notes', 'event_type' => 'type', 'contractor_id','status','start_landmark_id', 'start_address', 'end_landmark_id', 'end_address', 'start_time','end_time'));
      $id = $obj_data['obj_id'];
      if($id) {
        list($id, $errors) = update_object('event', $obj_data, $id  );
      } else {
        list($id, $errors) = add_object('event', $obj_data);
      }
      if($errors) json_error($errors);
      $action = $id ? "Updated" : "Added";
      json_message("$action event $id"); 
      break;
      
    case 'delete_exception': // also used for update
      if($error = check_required($obj_data, array('obj_type' => "Type", 'obj_id' => "ID"))) json_error("Missing: ".implode(', ', $error));
      
      list($count, $errors) = delete_object($obj_data['obj_type'], $obj_data['obj_id']);
      if($errors) json_error($errors);
      
      json_message("Deleted Calendar Rule {$obj_data['obj_id']}"); 
      break;
      
    case 'add_exception': // also used for update
      $obj_type = 'res_exception';
      $exception_data = $obj_data;
      if(!$exception_data['apt_id'] || !isint($exception_data['apt_id'])) unset($exception_data['apt_id']); // has to be NULL, not 0
      //json_error(dump($exception_data, true));
      if($exception_id = $exception_data['exception_id']) {
        list($id, $errors) = update_object('res_exception', $exception_data, $exception_id);
        //if($id) json_message("Updated exception $id");
      } else {
        list($id, $errors) = add_object('res_exception', $exception_data);
        //if($id) json_message("Added exception $id");
      }
      if($errors) json_error($errors);
      //else json_message($
      break;

    case 'add_catering':
      $obj_type = 'event';
      $event_data = array_copy($obj_data, 
        array('catering_res_id' => 'res_id', 
              'catering_supplier_id' => 'supplier_id',
              'catering_lineitem_id' => 'lineitem_id',
              'catering_status' => 'status',
              'catering_contractor_id' => 'contractor_id',
              'catering_notes' => 'notes', 
              'event_id'));

      if($obj_data['catering_delivery']) {
        $event_data['type'] =  EVENT_TYPE_DELIVERY;
        $event_data['start_address'] = $obj_data['catering_pickup_address'];
        $event_data['end_landmark_id'] = $obj_data['catering_delivery_landmark_id']; // =>
        $event_data['end_time'] =  $end_time = $obj_data['catering_delivery_time']; // => 2012-09-15 14:00:00
        $event_data['start_time'] = add_interval($end_time, -1, 'HOUR'); // => 2012-09-15 14:00:00
      } else {
        $event_data['type'] =  EVENT_TYPE_PICKUP;
        $event_data['start_address'] =  $obj_data['catering_pickup_address']; // => 
        $event_data['end_landmark_id'] = $obj_data['catering_delivery_landmark_id']; // =>
        $event_data['start_time'] = $start_time = $obj_data['catering_pickup_time']; // => 2012-09-15 14:00:00
        $event_data['end_time'] = add_interval($start_time, 1, 'HOUR'); // => 2012-09-15 14:00:00
      }
      
      if($event_id = $event_data['event_id']) {
        list($id, $errors) = update_object('event', $event_data, $event_id);
      } else {
        list($id, $errors) = add_object('event', $event_data);
      }
      break;
      

    case 'load-dialog':
    case 'load_dialog':
      // options by GET, data by POST
      $obj = $id ? get_object($obj_type, $id) : array2obj($obj_data);
      if($id && $_POST) { // edit, but data posted (defaults, overrides) 
        foreach($_POST as $k => $v) $obj->$k = $v;
      }
      
      //if(isset($_GET['debug'])) {
      //  $debug = "POST:".dump($_POST, true);
      //  $debug .= "GET:".dump($_GET, true);
      //  $debug = "obj:".dump($obj, true);
      //  echo($debug);
      //}

      //$obj->site_id = 232;
      $fields = json_decode($_GET['fields']);
      $options = array('dialog' => true, 'fields' => $fields);
      $params = array('readonly', 'dialog_title', 'reload', 'reload_template', 'reload_target', 'reload_link', 'handler_operation', 'handler_data', 'success_handler', 'target', 'handler', 'parent_id','class','class_options');
      foreach($params as $param) $options[$param] = $_GET[$param];          

      $output = '';
      //$output .= "$obj_type $id:".dump($obj, true);
      //$output .= dump($options, true);
      $output .= object_form($obj_type, $obj, $options);
      //$output = dump($options, true);
      //$output = dump($_GET, true);
      echo($output);exit();
      break;

    // takes an object type + a comma separated list of IDS. Sets rank of object in order of id list
    case 'rank_update':
      $list = $_GET['id_list'];
      $obj_type = $_GET['obj_type'];
      if($obj_type && $list) {
        $response->success = 1;
        rank_update_list($obj_type, $list);
        print_log("rank_update {$obj_type} list={$list}", 'media', LOG_LEVEL_TEST);
      } else {
        $response->success = 0;
      }
      echo(json_encode($response));exit();
      break;

    case 'rank_up':
      $search = $_POST['search'] ?: array();
      $response->data = array2obj($search); 
      if($obj_type && $id) {
        $response->success = 1;
        rank_up($obj_type, $id, $search);
      } else {
        $response->success = 0;
      }
      echo(json_encode($response));exit();
      break;
      
    case 'rank_down':
      $search = $_POST['search'] ?: array();
      $response->data = array2obj($search); 
      if($obj_type && $id) {
        $response->success = 1;
        rank_down($obj_type, $id, $search);
      } else {
        $response->success = 0;
      }
      echo(json_encode($response));exit();
      break;

    case 'tree_update':
      $node   = $_GET['id'];        
      $mode   = $_GET['mode'];
      $trash = $_GET['trash'];
      $nodeObj = get_object('site_page', $node);
      if($target = $_GET['target']) {
        $targetObj = get_object('site_page', $target);
        $search['site_id'] = $targetObj->site_id ?: 0;
        $search['parent_id'] = $pid = $targetObj->parent_id ?: 0;
      }
      if($mode == 'over' && !$target) $mode = 'after'; // if we drop on [empty] trash
      switch($mode) {
      case 'foo':
        //rank_before('site_page', $node, $target, $search);
        break;
      case 'before':
        rank_before('site_page', $node, $target, $search);
        break;
      case 'after':
        rank_after('site_page', $node, $target, $search);
        break;
      case 'over':
        $rank = rank_next('page_id', $search);
        $pid = $targetObj->id;
        break;
      default:
        json_error("tree-update: Unknown mode $mode");
        break;          
      }
      $data['parent_id'] = $pid;
      if($rank) $data['rank'] = $rank;
      // echo("node is $nodeObj->active, trash = $trash");
      if($nodeObj->active && $trash) { // move from active to trash
        $data['active'] = 0;
      } else if(!$nodeObj->active && !$trash) { // move from trash to active
        $data['active'] = 1;
      }

      list($nid, $nerrors) = update_object('site_page', $data, $node);
      if($nerrors) $output .= print_errors($nerrors);
      $output = "Received $mode source=$node target=$target ($targetObj->name) target_parent=$pid trash=$trash".print_r($data, true);        
      echo($output);
      die();
      break;

    case 'del_layout': 
      if($del_id = $_GET['del_layout']) list($count, $errors) = delete_object('site_layout', $del_id);
      break;

    case 'save_layout': 
      // dump($_POST);
      //echo(dump($obj_data, true));exit();
      
      $obj_type = 'site_layout';
      
      //unset($_POST['css_save']);
      foreach($obj_data as $k => $v) $data[str_replace('-', '_', $k)] = $v;
      
      extract($obj_data);
      if(!$site_id)     json_error("Missing site_id");
      if(!$parent_id)   json_error("Missing parent_id");
      if(!$parent_type) json_error("Missing parent_type");
      if(!$selector)    json_error("Missing selector");

      //if(!$foo)     json_error("Missing foo");
      
      if($font = $data['font_family']) $data['font_family'] = find_object('font', array('css' => $font), 'id');
      $layoutSearch = array('site_id' => $site_id, 'parent_type' => $parent_type, 'parent_id' => $parent_id, 'selector' => $selector);

      //$layoutSearch['site_id'] = $data['site_id'];
      //$layoutSearch['selector'] = $_POST['selector'];
      ////if($user_id = $data['user_id']) $layoutSearch['div_select'] = $user_id;
      //if($parent_id = $data['parent_id']) $layoutSearch['div_select'] = $parent_id;
      //if($parent_type = $data['parent_type']) $layoutSearch['div_select'] = $parent_type;
      
      //if($_POST['css_allpages']) $layoutSearch['page_id'] = ATT_DEFAULT_NULL;
      //else $layoutSearch['page_id'] = $data['page_id'] = $_POST['page_id'];
    
      if($_POST['layout_id'] &&  $_POST['css_allpages']) list($count, $errors) = delete_object('site_layout', $_POST['layout_id']);
      
      
      if($lid = find_object('site_layout', $layoutSearch, 'id')) {
        list($id, $errors) = update_object('site_layout', $data, $lid);
        $action = "update";
      } else {
        $action = "add";
        list($id, $errors) = add_object('site_layout', $data);
      }
        //dump($data);
      if($errors) {
        $log = $out = "Error: Could not $action site_layout $id ($data[div_select])".print_r($errors, true);          
      } else {
        $log = $out = "OK: $action site_layout $id ($data[div_select])";
        // echo("$action $id");
      }
      echo($log);
      die();
      break;
      
    case 'delete_file': 
      global $upload_dir;
      if(!$upload_dir) json_error("Delete file: Data directory not set.");

      $filepath = $_POST['filename'];
      $full_file = $upload_dir.$filepath;
      if(!@unlink($full_file)) json_error("Delete file: Could not delete $filepath");
      
      // success if we get here
      $result['success'] = 1;
      $result['filename'] = $filepath;
      //$result['dir'] = $upload_dir;
      //$result['path'] = $upload_path;
      // $result['error'] = $upload_dir;

      echo json_encode($result);
      exit();				
      break;

    // below 2 should return json array with image info:
    /*
    {"files":[
      {"url":"https://jquery-file-upload.appspot.com/AMIfv97PPQvpSlC14l0-nbsiKXCfIvybzRRah1S8edz-3c4dXzJznISGS31SRn1sC_BEFqkUQY0_uvjM0uIcEE3wp6TA0qksz1_vc49QhB31Vc_S59pkP4krph9zfhUeOqiB44Al9orKGlyOFhVPwDvZpRku6f-o0Z0185vQ0Mmaq2iUreasjvo/2014-09-17%2013.53.23.jpg",
       "thumbnailUrl":"https://lh4.ggpht.com/-xMtWzrcS3oGaphrJMsgewnxhnbo7eumOw9ybjG1psPvqK82uhRcycsds7hNY01bcKutgXWeEofb71Bb1mRFuTTr8RPGU98=s80",
       "name":"2014-09-17 13.53.23.jpg",
       "type":"image/jpeg",
       "size":482535,
       "deleteUrl":"https://jquery-file-upload.appspot.com/AMIfv97PPQvpSlC14l0-nbsiKXCfIvybzRRah1S8edz-3c4dXzJznISGS31SRn1sC_BEFqkUQY0_uvjM0uIcEE3wp6TA0qksz1_vc49QhB31Vc_S59pkP4krph9zfhUeOqiB44Al9orKGlyOFhVPwDvZpRku6f-o0Z0185vQ0Mmaq2iUreasjvo/2014-09-17%2013.53.23.jpg?delete=true",
       "deleteType":"DELETE"}
    ]}
    */
    /*
    case 'jq-save-exif':
      print_log(dump($obj_data, true), 'exif', LOG_LEVEL_ALWAYS);
      extract($_GET);
      if($error = check_required($_GET, array('id'))) json_error("Missing: ".implode(', ', $error));
      list($id, $errors) = update_object('media', array('exif' => $exif), $mediaObj->id);
      if($id && !$errors) $result = json_success_object("OK, saved exif for media $id");
      else json_error("Failed update exif for media $id: ".$errors[0]);
      break;
    */
    
    case 'jq-get-media':
      if(!$u_id) {
        $result['success'] = false;
        $result['files'][0]->error = "Login is required to upload files";
        $result['reason'] = $error;
        header("Content-Type: text/plain");
        echo json_encode($result);exit;
      }
      extract($obj_data);
      $files = [];
      if($parent_type == 'media_collection_map' && $parent_field == 'map') { // special case
        //die("gh");
        $search = ['active' => 1, 'id' => $parent_id];
        $mapObjs = find_objects('media_collection_map', $search, ['sort' => 'rank']);
        $files = [];
        $edit_url = "edit-url";
        foreach($mapObjs as $mapObj) { // expensive
          if($mapObj->media_id){
            $mediaObj = get_object('media', $mapObj->media_id);
            $mediaObj->media_id = $mapObj->media_id;
            $mediaObj->map_id = $mediaObj->obj_id = $mapObj->id;
            $mediaObj->obj_type = 'media_collection_map';
            $mediaObj->delete_url = "/ajax.php?oper=del_custom&obj_type=media_collection_map&id=".$mapObj->id;
            //if($mapObj->artwork_id) {
            //  $file->title = get_object('artwork', $mapObjs->artwork_id, 'title');            
            //}
            $mediaObjs[] = $mediaObj; 
          }
          
        }
      } elseif($parent_type == 'user' && $parent_field == 'map_profile'){
        $getMedia = find_object('media_collection', ['user_id' => $parent_id, 'active'=>1, 'type'=>150]);
        $mapObjs = db_getAll("select * from media_collection_map where media_collection_id= '$getMedia->id' OR media_collection_id= '1' AND active= '1' AND user_id= '$parent_id' ");
        $edit_url = "edit-url";
        $files = [];
        foreach($mapObjs as $mapObj) { // expensive
          $mediaObj = get_object('media', $mapObj->media_id);
          $mediaObj->media_id = $mapObj->media_id;
          $mediaObj->map_id = $mediaObj->obj_id = $mapObj->id;
          $mediaObj->obj_type = 'media_collection_map';
          $mediaObj->delete_url = "/ajax.php?oper=del&obj_type=media_collection_map&id=".$mapObj->id;
          $mediaObjs[] = $mediaObj;
        }
      }elseif($parent_type == 'exhibition_map' && $parent_field == 'map'){
        $get_media = find_object('exhibition_map', ['page_id' => $parent_id, 'active'=>1, 'type'=>'single']);
        $edit_url = "edit-url";
        $files = [];
          $mediaObj = get_object('media', $mapObj->title_image);
          $mediaObj->media_id = $mapObj->title_image;
          $mediaObj->map_id = $mediaObj->obj_id = $mapObj->id;
          $mediaObj->obj_type = 'exhibition_map';
          $mediaObj->delete_url = "/ajax.php?oper=del&obj_type=exhibition_map&id=".$mapObj->id;
          $mediaObjs[] = $mediaObj;  

      }elseif($parent_type == 'user_profile' && $parent_field == 'photo'){
        $get_user = get_object('user_profile', $parent_id);
        if($get_user->photo){
          $mapObj =get_object('media', $get_user->photo);
          $files = [];
          $mediaObj = get_object('media', $mapObj->id);
          $mediaObj->media_id = $mapObj->id;
          $mediaObj->map_id = $mediaObj->obj_id = $get_user->id;
          $mediaObj->obj_type = 'user_profile';
          $mediaObj->delete_url = "/ajax.php?oper=del_custom&obj_type=user_profile&id=".$parent_id;  
          $mediaObjs[] = $mediaObj; 
        }
      }elseif($parent_type == 'artwork' && $parent_field == 'title_image'){
        $artwork = get_object('artwork', $parent_id);
        if($artwork->title_image){
          $mapObj = get_object('media', $artwork->title_image);
          $files = [];
          $mediaObj = get_object('media', $mapObj->id);
          $mediaObj->media_id = $mapObj->id;
          $mediaObj->map_id = $mediaObj->obj_id = $artwork->id;
          $mediaObj->obj_type = 'artwork';
          $mediaObj->delete_url = "/ajax.php?oper=del_custom&obj_type=artwork&id=".$parent_id;  
          $mediaObjs[] = $mediaObj; 
        }
      }elseif($parent_type == 'artwork_media' && $parent_field == 'media'){
        $artwork = get_object('artwork_media', $parent_id);
        if($artwork->media){
          $mapObj = get_object('media', $artwork->media);
          $files = [];
          $mediaObj = get_object('media', $mapObj->id);
          $mediaObj->media_id = $mapObj->id;
          $mediaObj->map_id = $mediaObj->obj_id = $artwork->id;
          $mediaObj->obj_type = 'artwork_media';
          $mediaObj->delete_url = "/ajax.php?oper=del_custom&obj_type=artwork_media&id=".$parent_id;  
          $mediaObjs[] = $mediaObj; 
        }
      }elseif($parent_type == 'minisite' && $parent_field == 'logo'){
        $minisite = get_object('minisite', $parent_id);
        if($minisite->logo){
          $mapObj = get_object('media', $minisite->logo);
          $files = [];
          $mediaObj = get_object('media', $mapObj->id);
          $mediaObj->media_id = $mapObj->id;
          $mediaObj->map_id = $mediaObj->obj_id = $minisite->id;
          $mediaObj->obj_type = 'minisite';
          $mediaObj->delete_url = "/ajax.php?oper=del_custom&obj_type=minisite&id=".$parent_id;  
          $mediaObjs[] = $mediaObj; 
        }
      }elseif($parent_type == 'media_collection' && $parent_field == 'title_image'){
        $collection = get_object('media_collection', $parent_id);
        if($collection->title_image){
          $mapObj = get_object('media', $collection->title_image);
          $files = [];
          $mediaObj = get_object('media', $mapObj->id);
          $mediaObj->media_id = $mapObj->id;
          $mediaObj->map_id = $mediaObj->obj_id = $collection->id;
          $mediaObj->obj_type = 'media_collection';
          $mediaObj->delete_url = "/ajax.php?oper=del_custom&obj_type=media_collection&id=".$parent_id;  
          $mediaObjs[] = $mediaObj; 
        }
      } elseif($inline) {
        $media = get_object($parent_type, $parent_id, $parent_field);
        $mediaObjs = isJson($media) ? json_decode($media) : explode(',', $media);       
      } else {
        $mediaSearch = array('active' => 1, 'parent_type' => $parent_type, 'parent_id' => $parent_id, 'parent_field' => $parent_field);
        $mediaObjs = find_objects('media', $mediaSearch);
      }
      $fileAr = array();
      foreach($mediaObjs as $mediaObj) {
        $url = print_media($mediaObj, array('size' => 'medium', 'source_only' => true));
        $thumb_size = $mediaObj->resize === 1 ? 'thumb' : ''; 
        $thumb_url = print_media($mediaObj, array('size' => $thumb_size, 'source_only' => true));
        $fileEl = array('url' => $url, 'name' => $mediaObj->user_filename, 'size' => $mediaObj->file_size);
        $fileEl['deleteUrl'] = $mediaObj->delete_url ?: "/ajax.php?oper=del&obj_type=media&id=".$mediaObj->id;
        $fileEl['deleteType'] = 'DELETE';
        if($fileEl['editUrl'] = ($edit_url ? "$edit_url&id=".$mediaObj->id : '')) {
          $fileEl['editTarget'] = $edit_target;
        }
        
        if($thumb_url) $fileEl['thumbnailUrl'] = $thumb_url;
        $fileEl['title'] = $mediaObj->name ?: $fileEl['name'];
        $fileEl['type'] = $mediaObj->type;
        $fileEl['obj_type'] = $mediaObj->obj_type ?: 'media';
        $fileEl['obj_id'] = $mediaObj->obj_id ?: $mediaObj->id;
        $fileAr[] = (object) $fileEl;
      }
      $json->files = $fileAr;
      echo(json_encode($json));exit;
      break;
      
    case 'jq-file-upload': // latest file upload using jquery-file-upload
      header("Content-Type: text/plain");
      $start_time = microtime(true); 
      global $__SITE_ID, $__CLIENT_ID;
      if($mediaData = $_GET['mediaData']) $mediaData = (array) json_decode($mediaData);
      
      if($options = $_GET['options']) $options = (array) json_decode($options);

      $file_keys = array_keys($_FILES);
      $file_field_id = $file_keys[0];
      $fileArray = $_FILES['qqfile'] = $_FILES[$file_field_id]; // allows use to use qqfile uploader

      //print_log("Field: $file_field_id Post:".dump($_POST, true)."Media:".dump($mediaData, true)."Files:".dump($fileArray, true)."Options:".dump($options, true), 'upload', LOG_LEVEL_ALWAYS);
      
      print_log("Upload for: {$mediaData['parent_type']} {$mediaData['parent_id']} {$mediaData['parent_field']}", 'upload', LOG_LEVEL_TEST);      
      /** handled by defaults 
      if(!$mediaData['site_id']) $mediaData['site_id'] = $__SITE_ID;
      if(!$mediaData['client_id']) $mediaData['client_id'] = $__CLIENT_ID;
      */     
      $result = object_media_upload($fileArray, $mediaData, $options);

      $result = (array) $result;
      
      if($error = $result['error']) {
        print_log("Upload Error:".dump($error,true), 'upload', LOG_LEVEL_TEST);              
        $result['success'] = false;
        $result['files'][0]->error = $error;
        $result['reason'] = $error;
        echo json_encode($result);exit;
      }
      
      $data = $result['data'];
      $mediaObj = array2obj($data);
      
      print_log("Result: success={$result->success} media_id={$mediaObj->id}", 'upload', LOG_LEVEL_TEST);      

      $result['fileArray'] = $fileArray;
      $result['_FILES'] = $_FILES;
      
      echo json_encode($result);
      
      $time = elapsed($start_time);
      print_log("Upload done in {$time}s", 'upload', LOG_LEVEL_TEST); 

      /** Post processing (faster user response) */
      $resize = $options['resize'];
      if(is_array($resize)) $resize = array_filter($resize); // some weirdness in json params passed

      $cloud = $options['cloud'];
      print_log("Cloud: $cloud Resize: {$resize}", 'upload', LOG_LEVEL_TEST); 
      
      $files = $result['files'] ?: [];
      $file = $files[0]; // even for multiple upload, only one file (this method is called for each file)
      
      foreach($files as $fileObj) {
        $file = $fileObj->url;
        
        if($resize) { // array where key->value is directory->size
          //if(!is_array($resize)) $resize = $_SESSION['resize_array']; // use default if not false and not array
          global $resizeArray;
          if($resize === 1 || $resize === true) $resize = $resizeArray ?: resize_array(); // use default array if 1 or true
          $resize_options = [];
          $resize_options['skip_small'] = isset($options['skip_small']) ?$options['skip_small'] : 0; // don't resize to bigger image if original is too small
          $resize_options['resize']    = $resize;
          $resize_options['path']      = $result['upload_path']; // array where key->value is directory->size
          $resize_options['directory'] = $result['upload_dir']; // array where key->value is directory->size
          
          //print_log("running resize media with data ".dump($data, true)." and options\n".print_r($resize_options, true), 'upload', LOG_LEVEL_TEST);
          //print_log("pre_add_media: resize2=".dump($resize_options, true), 'add-media', LOG_LEVEL_TEST);
          
          
          //print_log("calling resize media with:".dump($resize_options,true), 'upload', LOG_LEVEL_TEST);
          list($result, $errors, $messages) = resize_media($siteObj, $mediaObj, $resize_options);
          //print_log("resize media result = ".dump($result,true).dump($errors, true).dump($messages, true), 'upload', LOG_LEVEL_TEST);
          //dump($resize_errors);dump($resize_messages);
          // if(!$result || $errors) return array($data, array_merge($errors, $resize_errors));
          
          if($result && !$errors && $mediaObj->id) {
            print_log("Resize OK ({$mediaObj->id})", 'upload', LOG_LEVEL_ERROR);             
            update_object('media', ['resized' => 1], $mediaObj->id);
            $mediaObj->resized = 1;
          } else {
            print_log("Resize errors ({$mediaObj->id}):".dump($errors,true), 'upload', LOG_LEVEL_ERROR); 
          }
          
        }

        // upload to cloud
        if($cloud) {
          require_once("xml/xml_cloudinary.inc");
          $res = cloudinary_upload_media($mediaObj);
          if($res->success && !$res->error) {
            print_log("Cloud upload OK ({$mediaObj->id})", 'upload', LOG_LEVEL_ERROR);             
          } else {
            print_log("Cloud upload errors ({$mediaObj->id}):".dump($res->error,true), 'upload', LOG_LEVEL_ERROR); 
          }
        }
        
        
      }
      $what = $cloud ? "CloudUploaded" : "Resized";
      $time = elapsed($start_time);
      $count = count($files);
      print_log("$what {$count} items done in {$time}s", 'upload', LOG_LEVEL_TEST); 
      exit;
      break;
      
    case 'file-upload': // file upload from jqGrid using ajaxfileupload
      $obj_type = $obj_data['obj_type'];
      $obj_key = get_object_key($obj_type);
      $obj_id = $id ?: $obj_data[$obj_key]; // $_POST['id'] is unset at top
      $options = json_decode(urldecode($obj_data['options']), true);
      $mediaData['parent_type'] = $obj_type;
      $mediaData['parent_id'] = $obj_id;
      $mediaData['parent_field'] = $obj_data['obj_field'];


      $file_keys = array_keys($_FILES);
      $file_field_id = $file_keys[0];
      $fileArray = $_FILES['qqfile'] = $_FILES[$file_field_id]; // allows use to use qqfile uploader
      
      print_log("Field: $file_field_id Post:".dump($_POST, true)."Media:".dump($mediaData, true)."Files:".dump($_FILES, true)."Options:".dump($options, true), 'upload', LOG_LEVEL_ALWAYS);
      
      $result = object_media_upload($fileArray, $mediaData, $options);
      header("Content-Type: text/plain");
      echo json_encode($result);exit;
      break;
      
    case 'upload': // media upload using fine-uploader 
      if(!is_array($qqfile)) {                
        $qqfile = array(
          'name' => $qq_name,
          'type' => $type
        );
      }
      if($mediaData = $_GET['mediaData']) $mediaData = (array) json_decode($mediaData);
      if($options = $_GET['options']) $options = (array) json_decode($options);
    
      $result = object_media_upload($qqfile, $mediaData, $options); 
      header("Content-Type: text/plain");
      echo json_encode($result);exit;
      break;

    case 'save-class': /** add/edit class */ 
      global $__CLASSES, $db;
      if($error = check_required($obj_data, array('site_id' => 'Site ID', 'user_id' => 'User ID', 'name' => 'Name', 'group' => 'Group', 'key' => 'Key', 'first_id' => 'First ID', 'user_level' => 'User Level'))) json_error("Missing: ".implode(', ', $error));
      extract($obj_data);
      if (preg_match('/[^a-zA-Z ]+/', $name, $matches)) json_error('Invalid name: only letters and spaces are allowed');
      if (preg_match('/[^a-zA-Z]+/', $key, $matches)) json_error('Invalid key: only letters a-z allowed');

      $key = strtolower($key);
      $type = $table = strtolower(str_replace(' ', '_', $name));
      if($__CLASSES[$type]) json_error("Class $type already exists");

      $tables = $db->getCol("show tables");
      if(in_array($table, $tables)) json_error("Table $table already exists");
      
      $obj_data['key'] = $key;
      $obj_data['type'] = $type;
      $obj_data['table'] = $table;
      
      list($new_id, $errors) = add_object('class', $obj_data, array('preset_id' => $type));
      if($errors) json_error($errors);

      if($new_id) {        
        db_query("INSERT INTO db_sequence VALUES ('$type', $first_id);");
        
        /** add attributes (fields) */       
        $fields = $add_fields ? explode(",", $add_fields) : array();
        array_unshift($fields, $key);
        foreach($fields as $field) {
          $attData = array('class' => $type);
          $attData['field'] = $field;
          $flags = 0;
          $default = '';
          $length = 0;
          /** ['active', 'created', 'timestamp', 'client_id', 'site_id','user_id','name', 'description'] */
          switch($field) {
          case $key:
            $att_type = 'id';
            $name = "ID";
            $flags = AF_PRIMARY|AF_AUTOINCREMENT;
            $length = 11;
            break;
          case 'active':
            $att_type = 'bool';
            $name = "Active?";
            $length = 1;
            $default = 1;
            break;
          case 'created':
            $att_type = 'datetime';
            $name = "Created";
            $default = ATT_DEFAULT_NOW;
            break;
          case 'timestamp':
            $att_type = 'datetime';
            $name = "Modified";
            $default = ATT_DEFAULT_NOW;
            break;
          case 'site_id':          
            $att_type = 'link';
            $name = "Site ID";
            $default = ATT_DEFAULT_SITE_ID;
            break;
          case 'client_id':          
            $att_type = 'link';
            $name = "Client ID";
            $default = ATT_DEFAULT_CLIENT_ID;
            break;
          case 'user_id':
            $att_type = 'link';
            $name = "User ID";
            $default = ATT_DEFAULT_USER_ID;
            break;
          case 'name':
            $att_type = 'str';
            $name = "Name";
            $length = 128;
            break;
          case 'description':
            $att_type = 'text';
            $name = "Description";
            break;
          default: 
            break;
          }
          $attData['type'] = $att_type;
          $attData['flags'] = $flags;
          
          $attData['name'] = $name;
          $attData['default_value'] = $default;
          if($length) $attData['length'] = $length;
          list($att_id, $errors) = add_object('attribute', $attData);
          if($errors) json_error($errors);                    
        }
      }
      
      $result->success = 1;
      $result->message = "Added class $type ($name)";
      $result->redirect = site_page_link($siteObj, PAGE_ADMIN)."?obj_type=$type&ac=edit_class";
      echo json_encode($result);exit;
      break;


/** vpatina: gallery, art related */    


  /** Add artwork to user collection */
  case 'gallery-favorite':
    extract($obj_data);
    $jsonp = $_GET['jsoncallback'];

    
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'obj_id'))) {
      json_error("Missing: ".implode(', ', $error).dump($obj_data, true), array('jsonp' => $jsonp));
    }
    
    $siteObj = get_object('site', $site_id);
    $collection_id = gallery_find_user_collection($siteObj, $user_id, array('return' => 'id'));
    if(!$collection_id) json_error("Missing user collection", array('jsonp' => $jsonp));
    if(!in_array($obj_type, array('media', 'media_collection_map', 'media_collection'))) json_error("Invalid object type $obj_type", array('jsonp' => $jsonp));

    if($mapObj = find_object('media_collection_map', array('media_id' => $obj_id, 'media_collection_id' => $collection_id))) {
      $result->id = 0;
      $result->success = true;
      delete_object('media_collection_map', $mapObj->id);
    } else {
      $result = gallery_publish_image($siteObj, $obj_id, $collection_id);
    }

    header('Content-Type: application/x-javascript; charset=utf-8');
        
    echo $jsonp ? $_GET['jsoncallback'].'('.json_encode($result).')' : json_encode($result);exit();
    break;
     
  /** used by plugin to return data using jsonp */
  case 'get-data':
    extract($_GET);
    //echo(dump($_GET, true));exit;
    $jsonObj = NULL;
    $site_id = 247;
    $siteObj = get_object('site', $site_id);
    //$debug = dump($_GET, true);           
    
    if($id) {
      switch($obj_type) {
      case 'media':
        $data = gallery_info($siteObj, $_GET);      
        break;
      case 'user':
        $data = print_user_profile($siteObj, $id);      
        break;
      case 'media_collection':
        $data = site_collection_profile($siteObj, $id);      
        break;
      case 'site':
        $img = html_image('http://test.vpatina.com/img/vpatina-logo-256.png', "logo");
        $text = html_p("vPatina’s mission is to create technology that helps artists, galleries and curators connect to each other. We see a world where art lovers can intuitively follow the careers of the artists and curators they love, and effortlessly share these passions with their friends; where galleries and artists can control the websites, mobile applications and TV platforms their art is available on; and where technology helps create a world that is more vibrant and diverse.");        
        $data = html_div($img, 'profile_pic').html_div($text, 'profile_text').html_div('', 'profile_footer');
        break;
      default:
        break;
      }
      $jsonObj->data = html_div($data.$debug, $container_class);
    } else if($obj_type == 'share' || $share) {
      $title = "";
      $picture = ""; 
      switch($parent_obj_type) {
      case 'media':
        $title = "Arwork by Artist";
        $picture = print_media($parent_id, ["full" => true, "source_only" => true, "size" => "big"]);
        break;
      case 'user':
        $title = "Ze Artist";
        $picture = user_photo($parent_id, ["full" => true, "source_only" => true, "size" => "big"]);        
        break;
      case 'media_collection':
        $title = "Ze Collection";
        //$picture = collection_title_image($parent_id, ["full" => true, "source_only" => true, "size" => "big"]);
        /* not sure why above does not work ? */
        $picture = site_full_url($site_id).collection_title_image($parent_id);
        break;
      default:
        break;
      }
      $loader_class_popup = 'vp-loader-popup margin-left';
      $data = ["obj_type" => $obj_type, "obj_id" => $obj_id, "parent_obj_type" => $parent_obj_type, "parent_obj_id" => $parent_id];
      $output = '';      
      $output .= html_link('#', '<i class="fa fa-facebook-square fa-2x"></i>',      array('class' => $loader_class_popup, 'data-url' => $share_link, 'title' => phrase('share_with_facebook', CAPITALIZE), 'data-social' => 'facebook'));
      $output .= html_link('#', '<i class="fa fa-twitter-square fa-2x"></i>',       array('class' => $loader_class_popup, 'data-url' => $share_link, 'title' => phrase('share_with_twitter', CAPITALIZE), 'data-social' => 'twitter'));
      $output .= html_link('#', '<i class="fa fa-google-plus-square fa-2x"></i>',   array('class' => $loader_class_popup, 'data-url' => $share_link, 'title' => phrase('share_with_google', CAPITALIZE), 'data-social' => 'google'));
      $output .= html_link('#', '<i class="fa fa-pinterest fa-2x"></i>',            array('class' => $loader_class_popup, 'data-url' => $share_link, 'data-title' => $title, 'data-picture' => $picture, 'title' => phrase('share_with_pinterest', CAPITALIZE), 'data-social' => 'pinterest'));
      $output .= html_link('#', '<i class="fa fa-linkedin fa-2x"></i>',             array('class' => $loader_class_popup, 'data-url' => $share_link, 'data-title' => $title, 'title' => phrase('share_with_linkedin', CAPITALIZE), 'data-social' => 'linkedin'));
      //$output .= html_link('#', '<i class="fa fa-digg fa-2x"></i>',                 array('class' => $loader_class_popup, 'data-url' => $share_link, 'title' => phrase('share_with_digg', CAPITALIZE), 'data-social' => 'digg'));
      // $output .= html_link('#', '<i class="fa fa-pinterest-square fa-2x"></i>',  array('class' => $loader_class_popup, 'data-url' => $share_link, 'title' => phrase('share_with_pinterest', CAPITALIZE), 'data-social' => 'pinterest', ));
      $output .= html_link('#', '<i class="fa fa-envelope-square fa-2x"></i>',      array('class' => $loader_class_popup, 'data-url' => $share_link, 'data-data' => $data, 'title' => phrase('share_with_email', CAPITALIZE), 'data-social' => 'email'));
      $output .= html_break(2).form_input('link', 'text', $share_link,              array('class' => '', 'size' => 80));
      //$output .= "pid: $parent_obj_id ".dump($_GET,true);
      //$output .= dump($_GET, true);

      $jsonObj->data = html_div($output, 'sharing-form');

      // 'js' => "data-viewer='$viewer' data-parent='".json_encode(array('obj_type' => $parent_obj_type, 'id' => $parent_id))."' data-search='".json_encode(array('parent_type' => 'media', 'parent_id' => $media_id))."'", 'class' => $loader_class_popup)).' ';
      
    } else {
      $debug = dump($_GET, true);
      $jsonObj->data = html_div("Missing ID".$debug, $container_class);
    }
    
    header('Content-Type: application/x-javascript; charset=utf-8');
    $jsonp = $_GET['jsoncallback'];
    
    echo $jsonp ? $_GET['jsoncallback'].'('.json_encode($jsonObj).')' : json_encode($jsonObj);exit();
    break;

  case 'load-content':
    extract($_GET);
    // if(!$id) json_error("missing id");    
    if(!$obj_type) json_error("missing object type");    
    if(!$user_id) json_error("missing user id");
    if(!$site_id) json_error("missing site id");
    $siteObj = get_object('site', $site_id);
    $auth_edit = false;
    if($id) {
      $obj = get_object($obj_type, $id);
      if(!$obj) {
        $output = "$obj_type $id not found";
        echo($output);exit;
      }
      $owner_id = $obj->user_id;
      //$auth_edit = gallery_auth_media($obj_type, $obj, $user_id);
    }
    switch($obj_type) {
    case 'media_collection_map':
    case 'media':
      $output .= gallery_explore($site_id, $obj_type, $id, $obj_data);
      break;
    case 'media_collection':
      $output .= gallery_explore($site_id, $obj_type, $id, $obj_data);
      break;
    case 'gallery':
      $output .= gallery_explore($site_id, $obj_type, $id, $obj_data);
      break;
    case 'folder_list':
      $output = gallery_folder_list($site_id, $user_id, array());
      break;
    case 'trash_list':
      $output = gallery_folder_list($site_id, $user_id, array('trash' => 1));
      break;
    case 'user_profile': 
      //$output = print_user_profile($siteObj, $obj);
      $output .= gallery_explore($site_id, $obj_type, $id, $obj_data);
      //$output = "User Profile {$obj->id}";
      break;
    case 'user':
      $profileObj = find_user_profile($obj);
      $output = print_user_profile($siteObj, $profileObj);
      $output .= "id=$id".dump($profileObj, true);
      break;
    default:
      $output = "Unsupported object type $obj_type";
      break;
    }
    echo($output);exit;
    break;
    
  case 'update-tree':
    extract($_GET);
    $nodes = $objects = array();
    $load_url = "/ajax.php?oper=load-content&site_id=$site_id&user_level=$user_level&user_id=$user_id"; //&obj_type=media&id=$obj->id";
    list($obj_type, $obj_id) = explode('-', $key);
    if($obj_type && $obj_id) { // object
      switch($obj_type) {
      case 'media_collection': // one collection
        $load_url .= "&obj_type=media_collection_map";
        $objects = gallery_collection_images($site_id, $obj_id, array('return' => 'object'));
        foreach($objects as $obj) $nodes[] = (object) array('key' => "media_collection_map-".$obj->map_id, 'title' => $obj->name ?: "(No title)", 'isFolder' => false, 'url' => "$load_url&id=$obj->map_id", 'icon' => '/images/icons-16/palette-paint-brush.png');
        break;
      case 'gallery': // one folder
        $load_url .= "&obj_type=media";
        $objects = gallery_folder_images($site_id, $obj_id);
        foreach($objects as $obj) {
          $nodes[] = (object) array('key' => "media-".$obj->id, 'title' => $obj->name ?: "(No title)", 'isFolder' => false, 'url' => "$load_url&id=$obj->id", 'icon' => '/images/icons-16/document-image.png');
        }
        break;
      case 'user': // assume artist for now, show published projects
        $objects = gallery_collection_list($site_id, MEDIA_COLLECTION_TYPE_PROJECT, $obj_id, array('return' => 'object')) ?: array();
        foreach($objects as $obj) $nodes[] = (object) array('key' => "media_collection-".$obj->id, 'title' => $obj->title ?: "(No title)", 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=media_collection&id=$obj->id", 'icon' => '/images/icons-16/projection-screen-presentation.png');
        
        //if($type == USER_ROLE_ARTIST) {
        //}
        break;
      case 'media':
        break;      
      }      
    } else { // base type = top folders
      switch($key) {
      case 'vault':
        break;
      case 'exhibited': // = exhibitions
        $objects = gallery_collection_list($site_id, MEDIA_COLLECTION_TYPE_EXHIBITION, $user_id, array('return' => 'object')) ?: array();
        foreach($objects as $obj) $nodes[] = (object) array('key' => "media_collection-".$obj->id, 'title' => $obj->title ?: "(No title)", 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=media_collection&id=$obj->id", 'icon' => '/images/icons-16/projection-screen-presentation.png');
        break;
      case 'art':
        $objects = gallery_user_folder($site_id, $user_id);
        foreach($objects as $obj) $nodes[] = (object) array('key' => "gallery-".$obj->id, 'title' => $obj->title ?: "(No title)", 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=gallery&id=$obj->id");
        break;
      case 'artist':
        $objects = gallery_artist_list($site_id, array(), array('return' => 'object')) ?: array();
        foreach($objects as $obj) $nodes[] = (object) array('key' => "user-".$obj->id, 'title' => $obj->display_name ?: "$obj->first_name $obj->last_name", 'url' => "$load_url&obj_type=user_profile&id=$obj->id", 'icon' => '/images/icons-16/user-medium.png');
        break;
      case 'curator':
        $objects = gallery_curator_list($site_id, array(), array('return' => 'object')) ?: array(); 
        foreach($objects as $obj) $nodes[] = (object) array('key' => "user-".$obj->id, 'title' => $obj->display_name ?: "$obj->first_name $obj->last_name", 'url' => "$load_url&obj_type=user_profile&id=$obj->id", 'icon' => '/images/icons-16/user-black.png');
        break;
      case 'published': // = projects
        $objects = gallery_collection_list($site_id, MEDIA_COLLECTION_TYPE_PROJECT, $user_id, array('return' => 'object')) ?: array();
        foreach($objects as $obj) $nodes[] = (object) array('key' => "media_collection-".$obj->id, 'title' => $obj->title ?: "(No title)", 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=media_collection&id=$obj->id", 'icon' => '/images/icons-16/projection-screen-presentation.png');
        break;
      case 'shared': // = projects
        $shared_user_ids = linked_users($user_id);        
        $objects = gallery_artist_list($site_id, $shared_user_ids, array('return' => 'object')) ?: array();
        foreach($objects as $obj) {
          $artist_id = $obj->user_id;
          $name = pick_first($obj->display_name, trim("$obj->first_name $obj->last_name"), get_object('user', $artist_id, "username"));
            $nodes[] = (object) array('key' => "user-".$obj->user_id, 'title' => $name, 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=media_collection&artist_id=$artist_id&type=".MEDIA_COLLECTION_TYPE_PROJECT, 'icon' => '/images/icons-16/user-medium.png');
          //"$load_url&obj_type=user_profile&id=$obj->id"
        }

        //$objects = gallery_collection_list($site_id, MEDIA_COLLECTION_TYPE_PROJECT, $shared_user_ids, array('return' => 'object')) ?: array();
        //foreach($objects as $obj) $nodes[] = (object) array('key' => "media_collection-".$obj->id, 'title' => $obj->title ?: "(No title)", 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=media_collection&id=$obj->id", 'icon' => 'projection-screen-presentation.png');
        break;
      case 'trash':
        // folders
        $objects = gallery_folder_list($site_id, $user_id, array('return' => 'object', 'trash' => 1, 'format' => 'json')) ?: array(); 
        foreach($objects as $obj) $nodes[] = (object) array('key' => "gallery-".$obj->id, 'title' => $obj->title ?: "(No title)", 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=gallery&id=$obj->id");

        // collections
        $objects = gallery_collection_list($site_id, array(MEDIA_COLLECTION_TYPE_PROJECT, MEDIA_COLLECTION_TYPE_EXHIBITION), $user_id, array('search' => array('active' => 0), 'return' => 'object')) ?: array();
        foreach($objects as $obj) $nodes[] = (object) array('key' => "media_collection-".$obj->id, 'title' => $obj->title ?: "(No title)", 'isFolder' => true, 'isLazy' => true, 'expand' => false, 'url' => "$load_url&obj_type=media_collection&id=$obj->id", 'icon' => '/images/icons-16/projection-screen-presentation.png');

        // images
        $objects = find_objects('media', array("parent_type" => 'gallery', "user_id" => $user_id, "active" => 0));
        foreach($objects as $obj) $nodes[] = (object) array('key' => "media-".$obj->id, 'title' => $obj->name ?: "(No title)", 'isFolder' => false, 'isLazy' => false, 'expand' => false, 'url' => "$load_url&obj_type=media&id=$obj->id", 'icon' => '/images/icons-16/document-image.png');

        // images
        $objects = find_objects('media_collection_map', array("user_id" => $user_id, "active" => 0));
        foreach($objects as $obj) $nodes[] = (object) array('key' => "media_collection_map-".$obj->id, 'title' => $obj->name ?: "(No title)", 'isFolder' => false, 'isLazy' => false, 'expand' => false, 'url' => "$load_url&obj_type=media_collection_map&id=$obj->id", 'icon' => '/images/icons-16/palette-paint-brush.png');

        break;

      }
    }
      
    $tree_json = json_encode($nodes);
    echo($tree_json);exit;
    break;
    
  case 'load-tree':
    $child_nodes = $nodes = array();
    extract($_GET);
    $is_staff   = $user_level >= USER_LEVEL_STAFF;
    $is_admin   = $user_level >= USER_LEVEL_ADMIN;
    $is_curator = $user_id && user_has_role($user_id, USER_ROLE_CURATOR);
    $is_artist = $user_id && user_has_role($user_id, USER_ROLE_ARTIST);
    $debug = " ul=$user_level is_staff=$is_staff is_admin = $is_admin";
    $debug = '';
    $nodeAr = array('art' => array('title' => phrase('my_studio', CAPITALIZE), 'url' => "/ajax.php?oper=load-content&site_id=$site_id&user_level=$user_level&user_id=$user_id&obj_type=folder_list", 'hideCheckbox' => true), 
                    'published' => array('title' => phrase('shared', CAPITALIZE), 'icon' => '/images/icons-16/folder-share.png', 'url' => "/ajax.php?oper=load-content&site_id=$site_id&user_level=$user_level&user_id=$user_id&obj_type=media_collection&type=".MEDIA_COLLECTION_TYPE_PROJECT, 'hideCheckbox' => true), 
                    'shared' => array('title' => phrase('shared', CAPITALIZE), 'icon' => '/images/icons-16/user-share.png', 'hideCheckbox' => true), 
                    'artist' => array('title' => phrase('artists', CAPITALIZE), 'icon' => '/images/icons-16/user-medium.png', 'hideCheckbox' => true), 
                    'curator' => array('title' => phrase('curators', CAPITALIZE), 'icon' => '/images/icons-16/user-business.png','hideCheckbox' => true), 
                    'exhibited' => array('title' => phrase('my_exhibitions', CAPITALIZE), 'icon' => '/images/icons-16/blue-folder-open-image.png', 'url' => "/ajax.php?oper=load-content&site_id=$site_id&user_level=$user_level&user_id=$user_id&obj_type=media_collection&type=".MEDIA_COLLECTION_TYPE_EXHIBITION, 'hideCheckbox' => true), 
                    'trash' => array('title' => phrase('archive', CAPITALIZE), 'icon' => '/images/icons-16/paper-bag-recycle.png', 'url' => "/ajax.php?oper=load-content&site_id=$site_id&user_level=$user_level&user_id=$user_id&obj_type=trash_list", 'hideCheckbox' => true)
                   );
    if(!$is_curator) {unset($nodeAr['exhibited']);unset($nodeAr['shared']);}
    if(!$is_artist) {unset($nodeAr['art']);unset($nodeAr['published']);}
    if(!($is_staff || $is_admin)) {unset($nodeAr['artist']);unset($nodeAr['curator']);}
    if($is_admin) {unset($nodeAr['shared']);}

    foreach($nodeAr as $k => $ar) {
      $nodeAr = array_merge($ar, array('key' => $k, 'title' => $ar['title'], 'isFolder' => true, 'isLazy' => true, 'expand' => false));
      $nodes[] = (object) $nodeAr;
    }

    $tree_json = json_encode($nodes);
    echo($tree_json);exit;
    break;


  // obj_type gallery: create new project(s) from folder(s)
  // obj_type media_collection: create new exhibition(s) from project(s)
  case 'cms-exhibit':
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'id', 'dest_type'))) json_error("Missing: ".implode(', ', $error));
    extract($_GET);
    if(!in_array($obj_type, array('media_collection_map', 'media_collection', 'gallery'))) json_error("Invalid source $obj_type");
    
    $siteObj = get_object('site', $site_id);
    $collection_id = 0;      
    
    $srcAr = strpos($id, ',') === false ? array_filter([$id]) : array_filter(array_unique(explode(',', $id)));      
    if(!count($srcAr)) json_error("Empty source array");
    print_log("cms-exhibition $obj_type: src_id=$src_id dest_type={$dest_type} srcAr=".dump($srcAr, true), "gallery", LOG_LEVEL_TEST);
    foreach($srcAr as $id) {            
      if($obj_type == 'media_collection_map') { // exhibit collection
        if($dest_type == 'media_collection_map') {
          $collection_id = $collection_id ?: get_object('media_collection_map', $dest_id, 'media_collection_id'); 
        } else if($dest_type == 'media_collection') {
          $collection_id = $dest_id;        
        } else {
          json_error("Invalid destination for image");
        }
        $result = gallery_exhibit_image($siteObj, $id, $collection_id);
      } else if($obj_type == 'gallery') { // exhibit folder
        if(!in_array($dest_type, array('exhibited'))) json_error("Invalid destination $dest_type for folder");
        $result = gallery_publish_folder($siteObj, $obj_type, $id, array('collection_type' => MEDIA_COLLECTION_TYPE_EXHIBITION));
      } else { // exhibit collection
        if(!in_array($dest_type, array('exhibited', 'media_collection'))) json_error("Invalid destination $dest_type for collection");
        print_log("cms-exhibition: gallery_exhibit_collection {$id} to user {$user_id}", "gallery", LOG_LEVEL_TEST);        
        $result = gallery_exhibit_collection($siteObj, $id, $user_id);
      }
      if($result->error) json_error($result->error);
    }
      
    echo(json_encode($result));exit;
    //json_message($messages);

    break;

  // new version of collection-new (deprectated)
  // obj_type gallery: create new project(s) from folder(s)
  // obj_type media_collection: create new exhibition(s) from project(s)
  case 'cms-publish':
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'id', 'dest_type'))) json_error("Missing: ".implode(', ', $error));
    extract($_GET);
    if($dest_type == 'media_collection_map') {
      $collection_id = get_object('media_collection_map', $dest_id, 'media_collection_id');
    } else if($dest_type == 'media_collection') {
      $collection_id = $dest_id;
    } else if($dest_type == 'published') { // top level
      if($obj_type == 'media') json_error("Only folders can be shared at the top level");
      $collection_id = 0;
    } else {
      json_error("Invalid destination $dest_type");
    }
    $siteObj = get_object('site', $site_id);
    if($obj_type == 'media') {
      $result = gallery_publish_image($siteObj, $id, $collection_id);
    } else if($obj_type == 'gallery') { 
      $result = gallery_publish_folder($siteObj, $obj_type, $id);
    } else if($obj_type == 'media_collection') {
      $result = gallery_publish_collection($siteObj, $id, $user_id);      
    } else {
      json_error("Invalid source $obj_type");
    }
    echo(json_encode($result));exit;
    //json_message($messages);

    break;

  // move image (media_collection_map) from one collection to another
  case 'cms-change-collection':
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'id', 'dest_type', 'dest_id'))) json_error("Missing: ".implode(', ', $error));
    extract($_GET);    

    if($dest_type != 'media_collection_map' && $dest_type != 'media_collection') json_error("Invalid target $dest_type"); 

    $obj = get_object($obj_type, $id);
    $src_collection_id = $obj->media_collection_id;    
    if(!$obj) json_error("Could not find map");
    if(!$src_collection_id) json_error("Missing source collection");

    $dst_collection_id = $dest_type == 'media_collection_map' ? get_object($dest_type, $dest_id, 'media_collection_id') : $dest_id;
    if(!$dst_collection_id) json_error("No destination collection id");

    list($id, $error) = update_object('media_collection_map', array('media_collection_id' => $dst_collection_id), $id); // handlers take care of the rest

    if($id) $result->data = $id;
    $success = $result->success = $id && !$errors;
    if($errors) $result->error = $errors;
    if($success) rank_update('media_collection_map', array('media_collection_id' => $src_collection_id, 'active' => 1)); // update rank of source folder media

    echo(json_encode($result));exit;
    break;
    
  // move image from one folder to another
  case 'cms-move':
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'id', 'dest_type', 'dest_id'))) json_error("Missing: ".implode(', ', $error));
    extract($_GET);    

    $obj = get_object($obj_type, $id);
    if(in_array($dest_type, array('media', 'gallery'))) { // move image to folder
      $src_folder_id = $obj->parent_id;
      
      if(!$obj) json_error("Could not find image");
      if(!$src_folder_id) json_error("Missing source folder");
      if($dest_type != 'gallery') json_error("Not a folder image"); // get=".dump($_GET, true)." obj=".dump($obj,true));
      
      $srcGalleryObj = get_object('gallery', $src_folder_id);
      if(!$srcGalleryObj) json_error("Could not find current folder $dest_id");
  
      $dst_folder_id = $dest_type == 'media' ? get_object($dest_type, $dest_id, 'parent_id') : $dest_id;
      if(!$dst_folder_id) json_error("No destination folder id");
      
      $dstGalleryObj = get_object('gallery', $dst_folder_id);
      if(!$dstGalleryObj) json_error("Could not find new folder $dst_folder_id");
      if(!$srcGalleryObj->path) json_error("Destination folder $dst_folder_id has no path");
      if(!$dstGalleryObj->path) json_error("Destination folder $dst_folder_id has no path");

      $new_path = str_replace($srcGalleryObj->path, $dstGalleryObj->path, $obj->path);
      
      list($id, $errors) = update_object($obj_type, array('path' => $new_path, 'parent_id' => $dst_folder_id), $id); // handlers take care of the rest

      print_log("cms-move: update $obj_type $id path=$new_path id=$id errors:".dump($errors, true), 'media', LOG_LEVEL_TEST);

      $rank_search = array('parent_type' => 'gallery', 'parent_id' => $src_folder_id, 'active' => 1);
    } elseif(in_array($dest_type, array('media_collection_map', 'media_collection'))) { // move map to collection
      $src_collection_id = $obj->media_collection_id;
      $dst_collection_id = $dest_type == 'media_collection' ? $dest_id : $obj->media_collection_id;
      
      if(find_object('media_collection_map', array('media_id' => $obj->media_id, 'media_collection_id' => $dst_collection_id))) {
        $media_name = get_object('media', $obj->media_id, 'name') ?: phrase('this_image', CAPITALIZE);
        json_error("$media_name is already in this collection.");
      }

      if(!$obj) json_error("Could not find media map");
      if(!$src_collection_id) json_error("Missing source collection");
      if(!$dst_collection_id) json_error("Missing destination collection");
      list($map_id, $errors) = update_object('media_collection_map', array('media_collection_id' => $dst_collection_id), $id);
      $rank_search = array('media_collection_id' => $dst_collection_id, 'active' => 1);
      
    } else {
      json_error("Invalid target $dest_type");
    }
    if($id) $result->data = $id;
    $success = $result->success = $id && !$errors;
    if($errors) $result->error = $errors;
    if($success) rank_update($obj_type, $rank_search); // update rank of source folder or collection

    echo(json_encode($result));exit;
    break;
    
  // move image from folder to trash
  case 'cms-recycle':
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'id', 'dest_type'))) json_error("Missing: ".implode(', ', $error));
    extract($_GET);    
    list($id, $error) = update_object($obj_type, array('active' => $active), $id);
    if($id) $result->data = $id;
    $result->success = $id && !$errors;
    if($errors) $result->error = $errors;
    echo(json_encode($result));exit;
    break;
    
  // delete object
  case 'cms-delete':
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'id'))) json_error("Missing: ".implode(', ', $error));
    extract($_GET);
    if($error = check_required($_GET, array('site_id', 'user_id', 'obj_type', 'id'))) json_error("Missing: ".implode(', ', $error));
    //if(!auth_action($obj_type, 'delete', $id)) json_error("Not authorized to delete $obj_type $id");
    $obj = get_object($obj_type, $id);
    if(!gallery_auth_media($obj_type, $obj, $user_id)) json_error("Not authorized to delete $obj_type $id");
    
    $allowed_types = array('gallery', 'media', 'media_collection', 'media_collection_map');
    if(!in_array($obj_type, $allowed_types)) json_error("Deleting $obj_type is not supported");
    $rank_search = array();
    switch($obj_type) {
    case 'media':
      $rank_search = array('parent_id' => $obj->parent_type, 'parent_id' => $obj->parent_id);
      break;
    case 'media_collection_map':
      $rank_search = array('media_collection_id' => $obj->media_collection_id);
      break;
    default:
      break;
    }

    list($count, $error) = delete_object($obj_type, $id);
    $result->count = $count;
    $result->success = $id && !$error;

    $success = $result->count && !$errors;
    if($success && $rank_search) rank_update($obj_type, $rank_search); // update rank of source folder media

    if($error) $result->error = $error;
    echo(json_encode($result));exit;
    break;

  // http://test.hitabrick.com/serviced-apartment/?location_id_textbox=3371%2021st%20Street%2C%20San%20Francisco%2C%20Californi%C3%AB%2C%20Verenigde%20Staten&loc=%5B%2237.7566%22%2C%22-122.4226%22%5D&bounds=%5B%5B%2237.7565%22%2C%22-122.4227%22%5D%2C%5B%2237.7567%22%2C%22-122.4225%22%5D%5D&ng=1&search=1


  case 'new-media-collection':          
    //$output .= "GET: ".dump($_GET, true);
    //$output .= "POST: ".dump($_POST, true);
    $id = $_GET['id'];
    $site_id = $_GET['site_id'];
    $siteObj = get_object('site', $site_id);
    $obj = get_object('media_collection', $id);
    $collection_type = get_object('media_collection_type', $obj->type, 'name');      
    $url = gallery_collection_link($siteObj, $id);
    $link = html_link($url, $obj->title);
    $output = "Your new $collection_type $link was added.";
    echo($output);exit();
    break;
    
  case 'new-gallery-folder':          
    //$output .= "GET: ".dump($_GET, true);
    //$output .= "POST: ".dump($_POST, true);
    $id = $_GET['id'];
    $site_id = $_GET['site_id'];
    $siteObj = get_object('site', $site_id);
    $obj = get_object('gallery', $id);
    //$collection_type = get_object('gallery', $obj->type, 'name');      
    $url = gallery_folder_link($siteObj, $id);
    $link = html_link($url, $obj->title);
    $output = "Your new folder $link was added.";
    echo($output);exit();
    break;

  case 'gallery-move':
    extract($_POST);                        
    if(!$user_id) json_error("Missing user id");
    if(!$site_id) json_error("Missing site id");

    if(!$gallery_id) json_error("Missing gallery id");
    $galleryObj = get_object('gallery', $gallery_id);      
    if(!$galleryObj) json_error("Gallery $gallery_id not found");

    if(!$media_id) json_error("Missing media id");
    $media_ids = explode(',', $media_id);
    $messages = array();
    $ids = array();
    foreach($media_ids as $media_id) {
      $mediaObj = get_object('media', $media_id);
      if(!$mediaObj) json_error("Media $media_id not found");
      //json_message("cid=".$gallery_id);
      
      $parent_path = $galleryObj->path ?: "/user/$galleryObj->user_id/vault/$gallery_id/";
      $new_path = clean_path('/'.$parent_path.$mediaObj->parent_field.'/'.$mediaObj->type.'/');
      
      $mediaData = array('parent_type' => 'gallery' , 'parent_id' => $gallery_id, 'path' => $new_path);
      
      $gallery_title = $galleryObj->title ?: phrase('project', CAPITALIZE);
      $gallery_url = gallery_folder_link($siteObj, $galleryObj);
      $gallery_link = html_link($gallery_url, "$gallery_title $gallery_id");
      $image_title = $mediaObj->name ?: $media_id;
      //json_message("Updating $media_id with ".dump($mediaData, true));
      list($id, $errors) = update_object('media', $mediaData, $media_id);
      if($errors) {
        json_error($errors);
      } elseif(!$id) {
        json_error("Tried to move $image_title to $gallery_title, but no id received");
      } else {
        $messages[] = "Image $image_title was moved to $gallery_link.";
        $ids[] = $id;
      }
    }
    $resultData['id'] = implode(',', $ids);
    $result['message'] = implode(html_break(), $messages);
    $result['success'] = 1;
    $result['data'] = $resultData;                
    echo(json_encode($result));exit();
    break;
    
  // DEPRECATED
  // create new collection from folder(s) or collection(s)
  case 'collection-new':
    extract($_POST); 
    if(!$user_id) json_error("Missing user id");
    if(!$site_id) json_error("Missing site id");
    $siteObj = get_object('site', $site_id);
    $obj_type = 'media_collection'; // already in POST, but hey
     
    $ids = explode(',', $id);
    $messages = array();
    $collection_type_name = get_object('media_collection_type', $type, 'name');
    $parent_type_name = $obj_type == 'gallery' ? "Folder" : "Project"; 
    foreach($ids as $source_id) {
      $obj = get_object($obj_type, $source_id);
      if(!$obj) json_error("$parent_type_name $source_id not found");
      $colData = (array) $obj;
      $colData['type'] = $type;
      list($id, $errors) = add_object($obj_type, $colData);
      if($errors) $messages[] = implode(html_break(), $errors);
      elseif(!$id) json_error("Tried to add $collection_type_name $obj->title, but no id received");
      else {
        $collectionObj = get_object($obj_type, $id);
        $collector_id = $collectionObj->user_id;
        $collection_id = $collectionObj->id; 
        $col_url = gallery_collection_link($siteObj, $collectionObj);
        $collection_link = html_link($col_url, "$collection_type_name $collectionObj->title");

        if($obj_type == 'gallery') {
          $mediaObjs = gallery_folder_images($siteObj, $source_id);
        } else {               
          $mapObjs = gallery_collection_maps($siteObj, $source_id);
          $mediaObjs = array();
          foreach($mapObjs as $mapObj) $mediaObjs[] = get_object('media', $mapObj->media_id);  
          $media_count = count($mediaObjs);
          $map_count = count($mapObjs);
          //$messages[] = "Found $map_count maps and $media_count images found for $obj_type $source_id";
          //json_message($messages);exit;
        }

        $count = count($mediaObjs);                   
        foreach($mediaObjs as $mediaObj) {
          $media_id = $mediaObj->id;
          
          $mapData['site_id'] = $site_id;
          //$mapData['user_id'] = $user_id;
          $mapData['media_collection_id'] = $collection_id;
          $mapData['media_id'] = $media_id;
          $mapData['collection_type'] = $collectionObj->type;

          $mapData['artist_id'] = $artist_id = $mediaObj->artist_id ?: $mediaObj->user_id;
          if($collectionObj->type == MEDIA_COLLECTION_TYPE_EXHIBITION) {
            $mapData['curator_id'] = $collectionObj->user_id;
            $mapData['curator_name'] = user_display_name($collectionObj->user_id);
          }
          $mapData['name'] = $mediaObj->name;
          
          $mapData['media_id'] = $media_id;
          $mapData['rank'] = $mediaObj->rank;
          
          $mapData['artist_name'] = user_display_name($artist_id);
          $mapData['original_technique'] = $mediaObj->original_technique;
          $mapData['original_year'] = $mediaObj->original_year;
          
          $image_title = $mediaObj->name ?: $media_id;
          
          //$messages[] = dump($mapData, true);
          list($id, $errors) = add_object('media_collection_map', $mapData);
          if($errors) $messages[] = implode(html_break(), $errors);
          elseif(!$id) json_error("Tried to add $image_title to $collection_type_name $collectionObj->title, but no id received");
          //else $messages[] = "Image $image_title was added to the $collection_type_name $collection_link.";
        }
        $messages[] = "Added $collection_link with $count images.";
        if($type == MEDIA_COLLECTION_TYPE_EXHIBITION && !user_has_role($collector_id, USER_ROLE_CURATOR)) user_add_role($collector_id, USER_ROLE_CURATOR);
      }  
    }
    //$messages[] = "Type=$type collector_id=$collector_id is_curator: ".(user_has_role($collector_id, USER_ROLE_CURATOR) ? 'yes' : 'no');
    json_message($messages);
    
    //json_message(dump($_POST, true));
    //die();
    //if(!$user_id) json_error("Missing user id");
    //if(!$site_id) json_error("Missing site id");
    break;
  
  case 'collection-add':
    extract($_POST);      
    //json_error(dump($_POST, true));
    if(!$user_id) json_error("Missing user id");
    if(!$site_id) json_error("Missing site id");

    if(!$collection_id) json_error("Missing collection id");
    $collectionObj = get_object('media_collection', $collection_id);      
    if(!$collectionObj) json_error("Collection $collection_id not found");

    $collection_url = gallery_collection_link($siteObj, $collectionObj);
    $collection_type = get_object('media_collection_type', $collectionObj->type, 'name') ?: 'collection';
    $collection_link = html_link($collection_url, $collectionObj->title);

    if(!$media_id) json_error("Missing media id");
    $media_ids = explode(',', $media_id);
    $messages = array();
    foreach($media_ids as $media_id) {
      if($obj_type == 'media_collection_map') $media_id = get_object('media_collection_map', $media_id, 'media_id');
      $mediaObj = get_object('media', $media_id);
      if(!$mediaObj) json_error("Media $media_id not found");
      //json_message("cid=".$collection_id);
      
      $mapData['site_id'] = $site_id;
      $mapData['user_id'] = $user_id;
      $mapData['media_collection_id'] = $collection_id;
      $mapData['collection_type'] = $collectionObj->type;
      $mapData['media_id'] = $media_id;


      $mapData['artist_id'] = $artist_id = $mediaObj->artist_id ?: $mediaObj->user_id;
      if($collectionObj->type == MEDIA_COLLECTION_TYPE_EXHIBITION) {
        $mapData['curator_id'] = $collectionObj->user_id;
        $mapData['curator_name'] = user_display_name($collectionObj->user_id);
      }
      $mapData['artist_name'] = user_display_name($artist_id);
      $mapData['original_technique'] = $mediaObj->original_technique;
      $mapData['original_year'] = $mediaObj->original_year;

      $mapData['name'] = $mediaObj->name;
      
      $image_title = $mediaObj->name ?: $media_id;
          
      
      //$messages[] = dump($mapData, true);
      list($id, $errors) = add_object('media_collection_map', $mapData);
      if($errors) $messages[] = implode(html_break(), $errors);
      elseif(!$id) json_error("Tried to add $image_title to $collection_type $collectionObj->title, but no id received");
      else $messages[] = "Image $image_title was added to the $collection_type $collection_link.";
    }
    json_message($messages);
    //if($errors) 
    //echo($data);exit();
    break;

  case 'explore-gallery':
    extract($_GET);
    if(!$id) {echo("Explore: Missing id");exit;} 
    switch($obj_type) {
    case 'media_collection':
      break;
    case 'media_collection_map':
      break;
    case 'user':
      break;
    case 'media':
      break;
    default:
      echo("Explore: Missing or unsupported type $obj_type");exit;
      break;
    }
    $options = array('count' => $count, 'index' => $index, 'nav_source' => $nav_source, 'ref' => $ref);
    $debug = "app-data-site-id=$site_id<br>";
    $output = gallery_explore($site_id, $obj_type, $id, $options);
    //$output .= "Nav source = $nav_source";
    echo($output);exit;
    break;
    
  /** new generic json to return json data for art (or print using template) */
  case 'gallery-get-collection':
    if($error = check_required($_GET, array('site_id', 'obj_type', 'obj_id'))) json_error("Missing: ".implode(', ', $error));
    extract($_GET);
    
    if(!in_array($obj_type, array('media_collection_map', 'media_collection', 'gallery', 'media_collection_map', 'user'))) json_error("Invalid source $obj_type");
    $objects = array();    
    $siteObj = get_object('site', $site_id);

    $user_collection_id = $u_id ? gallery_find_user_collection($siteObj, $u_id, array('return' => 'id')) : 0;

    $scope = new stdClass;
    $scope->obj_type = $obj_type;
    
    if($uri = $_GET['url']) $scope->url = this_url($_GET['url']);
    
    $debug = "";
    //echo("rt=$returntype".dump($scope,true));exit;

    print_log("gallery-get-collection: ot: {$obj_type} rt: {$returntype}", 'gallery', LOG_LEVEL_TEST);
    
    if(!$returntype) $returntype = 'media_collection';   
    
    if(in_array($returntype, array('artist','curator','exhibition','media_collection'))) { /** todo: don't do this for user, media_collection_map */
      $collection_id = $obj_id;
      $sort = $collection_id == $user_collection_id ? "id DESC" : "rank";
      $mapObjs = find_objects('media_collection_map', array('media_collection_id' => $collection_id, 'active' => 1), array('sort' => $sort));
      $media_ids = array_find_key($mapObjs, 'media_id');
      $all_collection_ids = find_objects('media_collection_map', array('media_id' => $media_ids), array('fields' => 'distinct media_collection_id'));
    }
    switch($returntype) {
      case 'user':
        switch($obj_subtype) { /** not needed, since controller already does this, but... */
        case USER_ROLE_ARTIST:
          $template = '_user-profile-artist.html';
          break;
        case USER_ROLE_CURATOR:
          $template = '_user-profile-curator.html';
          break;
        default:
          $template = '_user-profile-collector.html';
          break;
        }
        $controller = 'artist_profile';
        $scope->obj_type = 'user';
        $scope->obj_id = $obj_id;
        //echo("subtype=$obj_subtype".dump($scope,true));exit;
        break;
        
      case 'artist':
        $artist_ids = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_PROJECT, 'id' => $all_collection_ids), array('fields' => 'distinct user_id'));
        $objects = find_objects("user_profile", array("user_id" => $artist_ids));
        $template = '_gallery-artists';
        $controller = 'gallery_artists';
        break;
      case 'curator':
        $curator_ids = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $all_collection_ids), array('fields' => 'distinct user_id'));
        $objects = find_objects("user_profile", array("user_id" => $curator_ids));
        $template = '_gallery-curators';
        $controller = 'gallery_curators';
        break;
      case 'exhibition':
        $exhibition_ids = find_objects('media_collection', array('type' => MEDIA_COLLECTION_TYPE_EXHIBITION, 'id' => $all_collection_ids),  array('fields' => 'distinct id'));
        $objects = find_objects("media_collection", array("id" => $exhibition_ids));
        $template = '_gallery-collections';
        $controller = 'gallery_collections';
        break;
      case 'media_collection': // collection
        //$objects = gallery_collection_images($site_id, $obj_id, array('return' => 'object'));
        $objects = $mapObjs;
        if($obj_id == $user_collection_id) {
          $template = '_gallery-artworks';
          $controller = 'gallery_artworks';
        } else {
          $template = '_gallery-collections';
          $controller = 'gallery_collections';
          $scope->obj_id = $obj_id;
        }
        //$scope->obj_type = 'media_collection_map';
        break;
      case 'media_collection_map': // image
        //$objects = $mapObjs;
        $template = '_gallery-artworks';
        $controller = 'gallery_artworks';
        $scope->obj_type = 'media_collection_map';
        $scope->obj_id = $obj_id;
        //echo(dump($scope,true));exit;
        break;
      default: // error
        json_error(print_errors("Unsupported type $returntype"));
        break;
    }
    $result->success = 1;
    $result->message = "Loaded $returntype for $obj_type $id (print=$print)";
    $result->data  = $objects;
    
    print_log("gallery-get-collection: sot: {$scope->obj_type}", 'gallery', LOG_LEVEL_TEST);
    
    //$debug = "r=$returntype t=$template c=$controller";
    if(isset($print)) {
      $scope->objects = $objects;
      $pObj = new stdClass;
      $pObj->template = $template;
      $pObj->controller = $controller;

      $this_app = clone $app;
      $this_app->page = $pObj;
      
      $output = fetch_template($this_app, array('scope' => $scope));
      if($debug) $output = $debug.$output;
      echo($output);exit;
      
    } else {
      echo json_encode($result);exit;
    }
    break;

  case 'get-gallery':
    extract($_GET);

    $user = [];
    if(!$site_id) $site_id = 247;
    if($id && !$obj_id) $obj_id = $id; // legacy - prefer obj_id

    $log_dir = $_SESSION['log_dir'];
    if(0) { // for testing only
      $output = json_encode($jsonObj);      
      header('Content-Type: application/x-javascript; charset=utf-8');
      echo $jsonp ? $_GET['jsoncallback']."($output)" : $output;exit();
    }

    $collection_id = $obj_id;

    if($obj_type == 'artwork_media')   
      $jsonObj = gallery_getartwork_json($obj_id);
    elseif($obj_type == 'related_images')
      $jsonObj = gallery_getrelated_json($obj_id);
    else
      $jsonObj = gallery_get_json($collection_id);
    $jsonObj->container_id = $target;
    $jsonObj->plugin_id = $plugin_id;
    
    $output = json_encode($jsonObj);
  
    $jsonp = $_GET['jsoncallback'];
    header('Content-Type: application/x-javascript; charset=utf-8');
    echo $jsonp ? $_GET['jsoncallback']."($output)" : $output;exit();
    break;

  case 'artist-list':
    break;
/** end vpatina */

/** list/edit/view/delete object: generic */


    case 'edit': 
      if(strpos($id, ',')) { // update multiple
        $obj_data['id'] = $id;
        $ids = explode(',', $id);
        $id_count = count($ids);
        $field = $_POST['data_field'];
        if(!$field) {
          $errors[] = "Missing data_field: required for multiple updates.";
        } else {
          $val = $_POST[$field];            
          $val_array = explode(',', $val);
          $val_count = count($val_array);
          if($id_count > 1 && $val_count == 1) {
            $val_array = array_fill(0, $id_count, $val); // update all with same value: note doesn't work if there is a comma in value
            $val_count = count($val_array);
          }
          
          if(!is_array($val_array)) {
            $errors[] = "data_field $data_field is not array: required for multiple updates.";
          } elseif($id_count != $val_count) {
            $errors[] = "id count=$id_count val=$val_count";
          } else {
            foreach($val_array as $val) {
              $data[$field] = $val;
              $data_array[] = $data;
            }
          }
        }            
        if(!$errors) list($id, $errors) = update_objects_array($obj_type, $data_array, $ids);
      } else {
        if($_SESSION['debug_invoice_address'] && $obj_type == 'invoice') {
          $cc1 = $obj_data['country'];
          $cc2 = get_object('invoice', $id, 'country');
          print_log("ajax invoice $id: before=$cc1", "address", LOG_LEVEL_ALWAYS);        
        }
        list($id, $errors) = update_object($obj_type, $obj_data, $id);
        if($_SESSION['debug_invoice_address'] && $obj_type == 'invoice') {
          print_log("ajax invoice $id: after=$cc2", "address", LOG_LEVEL_ALWAYS);        
        }
      }

  
      break;
    case 'view': 
      $obj = get_object($obj_type, $id);
      break;
    case 'add': 
      print_log("app_date: Adding object $obj_type\n", 'ajax', LOG_DEBUG);
      $key = get_object_key($obj_type);
      unset($obj_data[$key]); // jqgrid sends _empty in key
      list($id, $errors) = add_object($obj_type, $obj_data);
      break;

    case 'del_custom':
      if($id){
        $get_profile = find_object($obj_type, ['id'=>$id]);
        if(($get_profile) && ($obj_type=='user_profile')){
          $get_profile->photo = '';  
          $get_profile->photo_json = '';        
        }elseif(($get_profile) && ($obj_type=='minisite')){
          $get_profile->logo = '';
        }elseif(($get_profile) && ($obj_type=='artwork_media')){
          $get_profile->media = '';
        }elseif(($get_profile) && ($obj_type == 'media_collection_map') ){
          if($get_profile->media_id){
            $get_profile->media_id = '';
          }
        }elseif(($get_profile) && ($obj_type == 'minisite_page') ){
          $exb_id = $_GET[exb_id];
          if($get_profile->search){
            $exhibition_ids = json_decode($get_profile->search, True);
            unset( $exhibition_ids[array_search($exb_id, $exhibition_ids )] );
            $exhibition_ids = array_values($exhibition_ids);
            $get_profile->search = json_encode($exhibition_ids, TRUE);
          }
        }else{
          $get_profile->title_image = '';
          $get_profile->title_image_json = '';   
        }
        list($delete_count, $errors) = update_object($obj_type, $get_profile, $get_profile->id);
        if($errors) write_file($log_file, "Delete errors:".dump($errors, true), 'a'); // "\n------------\n\n"
      }
      break;
          
      case 'del_soft':      
      if($id){
        $get_profile = find_object('media_collection_map', ['id'=>$id]);
        if($get_profile){
          $get_profile->photo = '';
          list($delete_count, $errors) = update_object($obj_type, $get_profile, $get_profile->id);
          if($errors) write_file($log_file, "Delete errors:".dump($errors, true), 'a'); // "\n------------\n\n"
        }
      }
      break;
    case 'del':      
      if(strpos($id, ',')) { // delete multiple
        list($delete_count, $errors) = delete_objects($obj_type, explode(',', $id));
      } else {
        list($delete_count, $errors) = delete_object($obj_type, $id);
        if($errors) write_file($log_file, "Delete errors:".dump($errors, true), 'a'); // "\n------------\n\n"
      }
      break;
    case 'test':
      $timer['end'] = microtime(true);
      $timerAr = [];
      $time_start = $last = $timer['start'];
      foreach($timer as $name => $time) {
        $timerAr[$name] = [$time-$last, $time-$time_start];
        $last = $time;
      }
      echo("Test: Timer:".dump($timerAr, true));
      json_success("Done");exit;      
      break;
    default:      
      json_error("unknown operand $oper");
      exit;
      
      $id = $errors = NULL;
      break;      
  }

  if($errors) json_error($errors);
  if ($id) {
    switch($oper) {
    case 'add':
      $oper_str = phrase('added', CAPITALIZE);
      break;
    case 'edit':
      $oper_str = phrase('saved', CAPITALIZE);
      break;
    case 'del':
    case 'delete':
      $oper_str = phrase('deleted', CAPITALIZE);
      break;
    default:
      $oper_str = ucfirst($oper);
      break;
    }
    $descriptor = $descriptor ?: get_select_value2($obj_type, $id);
    //$log = $out = $message = "OK: $oper_str $obj_name $id ($descriptor)";
    $log = $out = $message = "OK: $oper_str $obj_name ".$descriptor; // ?: $id;
    $obj = get_object($obj_type, $id);
    if($obj) {
      $obj->obj_type = $obj_type;
      $obj->obj_id = $id;
    }
    // if($delete_count) $messages[] = "Deleted $delete_count $obj_type";
  } elseif($delete_count) {
    /// $messages[] = "Deleted $delete_count objects";
  } else {
    //$log = $out = "Strange: $oper $obj_type $id - neither id nor errrors";
  }
  
  // special case for output text
  if($oper == 'add' && $userTypeArray[$obj_type] && !$errors) {
    $out .= html_break()."An activation link was sent to ".get_user_email(array2obj($obj_data));
  }

  if ($_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest') {  
    $data = $obj ?: $obj_data;
    
    //if(!$obj) $messages[] = "Could not find obj for $obj_type $id, using data";
    if($messages) $message .= implode(html_break(), $messages);
    //if($id && !$obj_data[$key]) $obj_data[$key] = $id;
    $json = array(  
                'success' => $success,  
                'message' => $message,  
                'data' => json_encode($data),  
            );  
    if(!$success) header("HTTP/1.0 422 Unprocessable Entity");
    echo json_encode($json);  
    exit();      
  // Else, just display the message on a new page  
  }
  
  exit(); // nothing more to do 


<?php

require_once('xmlreader-iterators.php'); // https://gist.github.com/hakre/5147685


/** main import function - called from cronscripts */
function interhome_import($action, $options = []) {
  global $__SITE;
  $user='NL1001285';
  $pass='HITABRICK234';
  $tag = "accommodation";
  
  $output = '';
  $max_distance = 5000;
  $source_id = 30;
  $host_id = 17772;
  $site_id = $__SITE->id;
  
  $options['update'] = $action;
  $options['max_distance'] = $max_distance;
  
  /**
  $action = 'distance';
  $action = 'ftp';
  $action = 'facs';
  $action = 'split';
  $action = 'xml';
  $action = 'price';
  $action = 'availability';  
  $action = 'servicedestination';  
  $action = 'service';
  */
  
  $fids = null;  
  $limit = isset($options['limit']) ? $options['limit'] : 0;
  $country = isset($options['country']) ? $options['country'] : '';
  $limit = isset($options['limit']) ? $options['limit'] : 0;
  $ts = now();
  $today = today();

  
  /** special case: import single rental */
  $apt_id = $options['apt_id'];
  $property_type = $options['property_type'];
  $code = $options['code'];
  
  if($property_type && $apt_id) {
    $aptObj = get_object($property_type, $apt_id);
    if(!$aptObj) return json_error_object("{$property_type} {$apt_id} not found");

    $guid = $aptObj->guid;
    if(!$guid) return json_error_object("{$property_type} {$apt_id} has no foreign ID");

    $name = $aptObj->name;
    if($aptObj->source_id != $source_id) return json_error_object("{$name} ({$guid}) is not an Interhome rental");

    $country = $aptObj->address_country;
    if(!$guid) return json_error_object("{$property_type} {$apt_id} has no country");
    
    /** create (pretty) file with just this rental */
    $cc_file = $apt_file = 0;
    $dir = clean_path("/web/data/hitabrick");

    /** look for accommodation file */
    $file = "accommodation_{$guid}.xml";
    $xmlAptFile = "$dir/xml/interhome/xml/code/$file";
    if(file_exists($xmlAptFile)) {
      $date = date('Y-m-d', filemtime($xmlAptFile));
      if($date == today()) $apt_file = $cc_file = 1;
    }
    
    if(!$apt_file) {
      /** look for country file */
      $file = "accommodation_{$country}.xml";
      $xmlCountryFile = "$dir/xml/interhome/xml/$file";
      if(file_exists($xmlCountryFile)) {
        $date = date('Y-m-d', filemtime($xmlCountryFile));
        if($date == today()) $cc_file = 1;
      }
    }
    
    if(!$cc_file) {      
      //return json_success_object("Fetching $xmlCountryFile");
      $result = interhome_import('split');
      if($result->error) return $result;
    }
    
    if(!$apt_file) {
      $result = interhome_import('split_code', ["country" => $country, "code" => $guid]);
      if($result->error) return $result;
    }

    //return json_error_object("OK, ready to import accommodation_{$guid} apt_file=$apt_file cc_file=$cc_file");
    //'split', 'import', 'inside','outside','summary','vacancy','fix','price'
    
    /** import rental */
    $result = interhome_read($__SITE, "code/accommodation_{$guid}.xml", "accommodation", array("update" => "import", "code" => $guid, "overwrite" => 1));
    if($result->error) return $result;

      
    return $result;
  }
  
  print_log("{$ts} interhome: action=$action limit=$limit country=$country", 'interhome-xml', LOG_LEVEL_ALWAYS);
  if(!$action) return json_error_object("Missing action");
  
  
  switch($action) {
   
  case 'split_code':
    //$country = "PT";
    if(!$code) return json_error_object("split_code: missing code cc={$country} code={$code}");
    if(!$country) return json_error_object("split_code: missing country cc={$country} code={$code}");
    $options['overwrite'] = 1;
    $options['filter_key'] = 'code';
    $options['filter_value'] = $code;
    $file = "accommodation_{$country}.xml";
    $result = interhome_read($__SITE, $file, "accommodation", $options);
    break;
    
  case 'split':
    $file = "accommodation.xml";  
    $result = interhome_read($__SITE, $file, "accommodation", $options);  
    break;
      
  case 'import':
    $countries = strpos($country, ',') ? array_filter(explode(',', $country)) : array_filter(array($country));
    if(!count($countries)) return json_error_object("Interhome import: please specify country or countries"); /** force import per country to reduce time */
    $result = new stdClass();
    foreach($countries as $country) { 
      if(!$apt_id) db_query("UPDATE property SET active=0 WHERE source_id=30 and address_country='$country'");
      $file = "accommodation_{$country}.xml";  
      $res = interhome_read($__SITE, $file, "accommodation", $options);
      $result->$country = $res;
    }
    $result->success = 1;
    break;
    
  case 'inside':
    $file = "insidedescription_en.xml";
    $result = interhome_read($__SITE, $file, "description", $options);  
    break;
  
  case 'outside':
    $file = "outsidedescription_en.xml";
    $result = interhome_read($__SITE, $file, "description", $options);  
    break;
  
  case 'vacancy':  
  case 'availability':
    //$fids = interhome_ids();
    $file = "vacancy.xml";  
    $result = interhome_read($__SITE, $file, "vacancy", $options);
    //interhome_disable_inactive(); // disable properties with no vacancy
    break;
    
  case 'price':
    $today = today();
    //die("update interhome_price set id = id * -1 where created < '{$today}' and id > 0<br>");
    db_query("update interhome_price set id = id + 1000000 where created < '{$today}' and id < 1000000"); /** set high ID for old prices */
    //db_query("delete from interhome_price where created < '{$today}'"); /** better way? Double prices for a while */
    
    db_query("UPDATE db_sequence SET nextid=10 where seq_name='interhome_price'"); 
  
    $file = "dailyprice_2048_eur.xml";
    $result = interhome_read($__SITE, $file, "price", $options);

    if($result->success) {
      $today = today();
      db_query("delete from interhome_price where id > 1000000 and created < '{$today}'"); /** delete negative for existing */
    }
    break;

  /** enable/disable rentals */
  case 'fix':    
    $result = interhome_set_active($max_distance);
    break;
  
  /** below are not in use */
  case 'summary':
    $file = "propertysummary_2048_eur_en.xml";
    $result = interhome_read($__SITE, $file, "accommodation", array("update" => "summary", "limit" => $limit));  
    break;
    
  case 'overview':
    $file = "price_2048_eur.xml";
    $result = interhome_read($__SITE, $file, "price", array("update" => "overview", "limit" => $limit));
    break;
    
  case 'alert':
    $file = "alert_en.xml";
    $result = interhome_read($__SITE, $file, "alert", array("update" => "alert", "limit" => $limit));  
    break;
  
  case 'service':
    //$fids = interhome_ids();
    $file = "service_2048.xml";  
    $result = interhome_read($__SITE, $file, "service", array("update" => "service", "limit" => $limit));  
    break;
  
  case 'pretty':
    //$fids = interhome_ids();
  
    
    $file = "service_2048.xml";
    $result = interhome_read($__SITE, $file, "service", array("update" => "pretty", "limit" => $limit));  
  
    $file = "price_2048_eur.xml";  
    $result = interhome_read($__SITE, $file, "price", array("update" => "pretty", "limit" => $limit));  
  
    $file = "dailyprice_2048_eur.xml";  
    $result = interhome_read($__SITE, $file, "price", array("update" => "pretty", "limit" => $limit));  
    
    break;
      
  case 'servicedestination':
    //$fids = interhome_ids();
    $file = "servicedestination.xml";  
    $result = interhome_read($__SITE, $file, "service", array("update" => "servicedestination", "limit" => $limit));  
    break;
  
  case 'facs':
    $aptObjs = find_objects('shortstay', array('source_id' => $source_id, 'facilities' => ''), array('fields' => array('id', 'amenities', 'facilities', 'golf_distance')));
    $count = count($aptObjs);
    $output .= "Found $count active apts without facs<br>";
    foreach($aptObjs as $aptObj) {
      $id = $aptObj->id;
      
      $facArray = array();
      $ams = $aptObj->amenities;
      if(!$ams) continue;
      
      $amArray = json_decode($aptObj->amenities);
  
      foreach($amArray as $am) {
        $fac_id = interhome_facility_map($am);
        if($fac_id) $facArray[] = $fac_id;
      }   
      if($golf_distance && $golf_distance < 1000) $facArray[] = 942; // walking distance to golf course
      $facArray = array_unique($facArray);
      asort($facArray);
      $facs = implode(',', $facArray);      
      db_query("UPDATE property SET facilities='$facs' WHERE id=$id");
    }
    break;
    
    
  case 'xml'; /** not in use ? */
    
    //$xml_filtered =  str_replace(array(".xml", "/xml/"), array("-filtered.xml", "/xml/golf".$max_distance."/"), $xmlFile);
    //$size = bytes2human(filesize($xml_filtered));  
  
    $country = 'PT';
    $countries = array($country);
    foreach($countries as $country) {
      $file = str_replace('.xml', "_".$country.".xml", $xmlFile);
      $len = filesize($file);
      $output .= "Reading from $file len=$len<br>";
      $result = interhome_read($__SITE, $file, $tag, array("add" => 0, "update" => "property", "limit" => $limit, "max_distance" => $max_distance)); // load accommodations in country
    }
  
    break;
    
  case 'distance':
    $countries = array("PT", "FR", "ES");
    foreach($countries as $country) {
      $file = str_replace('.xml', "_".$country.".xml", $xmlFile);
      $aptObjs = find_objects('shortstay', array('source_id' => $source_id, 'address_country' => $country, 'active' => 1), array('fields' => array('id', 'distances')));
      $count = count($aptObjs);
      $output .= "Found $count active apts in $country<br>";
      foreach($aptObjs as $aptObj) {
        $id = $aptObj->id;
        if($distances = $aptObj->distances) {
          $distObj = json_decode($distances);
          $golf_distance = $distObj->golf_club;
          //echo("$id: distance = $golf_distance<br>");
          $active = $golf_distance > $max_distance ? 0 : 1;
          db_query("UPDATE property SET active=$active, golf_distance='$golf_distance' WHERE id=$id");
        }
      }
      
    }
    break;
   
  default:
    return json_error_object("Unknown action $action");
    break;
  }
  
  return $result;
}


/** unsused - automatic comment added on  */
function interhome_print($file, $root, $tag) {
  
  
  $docroot = docroot();
  $dir = clean_path("$docroot/afr/XML/Interhome/xml");
  $xmlFile = "$dir/$file";

  $result = interhome_read($xmlFile, $tag);
  $output = dump($result, true);
  
  if(0) {  
    $long = true;
    if($long) {
      $xml = interhome_read($xmlFile, $tag);  
      $xml = "<root>$xml</root>";
      $dom = new DOMDocument;
      $dom->preserveWhiteSpace = FALSE;
      $dom->loadXML($xml);
      $dom->formatOutput = TRUE;
      $pretty = $dom->saveXml();
    } else {
      $xml = read_file($file);
      $pretty = $xml;
    }
  
    $output .= form_text('xml', $pretty, array('rows' => 20, 'cols' => 80));
  }
  
  return $output;
}



function interhome_ftp($file) {
  $ftp_user='ihxmlpartner';
  $ftp_pass='XZpJ6LkG';
  $ftp_server = "ftp.interhome.com";
  
  $mode = FTP_BINARY;
  
  $dir = clean_path("/web/data/hitabrick/xml/interhome");

  $output .= "Dir: $dir<br>";

  $local_file = "$dir/zip/{$file}.zip";
  $remote_file = "{$file}.zip";
  $success = false;
  
  $ftp_stream = ftp_connect($ftp_server);
  if($login_result = ftp_login($ftp_stream, $ftp_user, $ftp_pass)) {
    $output .= "OK: Logged in to $ftp_server<br>";
    if($result = ftp_get ($ftp_stream, $local_file, $remote_file, $mode)) {
      $output .= "OK: Read from $ftp_server/$remote_file and wrote to $local_file<br>";
      $xml_dir = "$dir/xml/";
      $zip = new ZipArchive;
      $res = $zip->open($local_file);
      if ($res === TRUE) {
        $zip->extractTo($xml_dir);
        $zip->close();
        $output .= "OK: Unzipped to XML directory $xml_dir<br>";
        $success = true;
      } else {
        $output .= "Error: Unziping $local_file to $xml_dir failed<br>";
      }      
      
    } else {
      $output .= "Error: Could not read from $ftp_server:$remote_file and/or write to $local_file<br>";
    }
  } else {
    $output .= "Error: Failed to log in to $ftp_server<br>";
  }

  return array($success, $output);
}


function interhome_read($__SITE, $file, $tag, $options = array()) {  
  $result = new stdClass;
  $stats = [];
  
  $errors = $messages = [];
  
  $dir = clean_path("/web/data/hitabrick");
  $started  = false;
  $pretty_file = '';
  
  $output = '';
  $today = today();
  $action = $options['update'];
  $max_distance = $options['max_distance'] ?: 0;
  
  $xml_selection = $filtered = $options['filtered'];
  $fids = $option['fids']; // existing listings
  
  $xmlFile = "$dir/xml/interhome/xml/$file";
  $tmp_file = "$dir/xml/interhome/xml/{$file}.tmp";
  $done_file = "$dir/xml/interhome/xml/{$file}.tmp.done";
  $limit = $options['limit'] ?: 0;

  if(!$file) return json_error_object("interhome_read: missing file");
  if(!$file) return json_error_object("interhome_read: missing tag");
  if(!$action) return json_error_object("interhome_read: missing action");

  $filter_key = $options['filter_key'];
  $filter_value = $options['filter_value'];
  
  print_log(now().": file:$file tag:$tag update:$action", 'interhome-xml', LOG_LEVEL_ALWAYS);
  
  //dump($options);die();
  $overwrite = $options['overwrite'] || $_GET['overwrite'] ? 1 : 0;
  if(file_exists($done_file) && !$overwrite && $action != 'pretty') {
    $date = date('Y-m-d', filemtime($done_file));
    if($date == $today) return json_success_object("$file was already processed today");
  }

  if(file_exists($tmp_file)) {
    $date = date('Y-m-d', filemtime($tmp_file));
    $size = filesize($xmlFile);
    $tmp_size = filesize($tmp_file);
    $diff = abs($size - $tmp_size);
    if($date == $today && $diff < 20) {
      return json_success_object("$file was already processed today: org: $size tmp: $tmp_size diff: $diff");
    } else { /** todo: make new xmlFile with missing bits */
      
    }
  }
    
  $exists = file_exists($xmlFile);
    
  if($exists) { // check date + size
    $size = filesize($xmlFile);
    $size_h = bytes2human($size);
    $time = filemtime($xmlFile);
    $date = date('Y-m-d', $time);
    $output .= "$xmlFile exists ($size_h) date: $date<br>";
  } else { // download
    $output .= "$xmlFile does not exists<br>";
    print_log(now().": $xmlFile does not exist", 'interhome-xml', LOG_LEVEL_ALWAYS);
  }

  $download = !$exists || !$size || $date < $today;

  //$download = 1;
  
  if($download) { // download
    $output .= "File does not exist or is old - downloading <br>";
    list($result, $message) = interhome_ftp($file);
    print_log(now().": File does not exist or is old - downloading result:$result message:$message", 'interhome-xml', LOG_LEVEL_ALWAYS);
    $output .= "Download result: $result<br>";
    $output .= $message;
    if($result) {
      $size = filesize($xmlFile);
      $size_h = bytes2human($size);
      $time = filemtime($xmlFile);
      $date = date('Y-m-d', $time);
      $output .= "Downloaded $xmlFile ($size_h) date: $date<br>";
      print_log(now().": Downloaded $xmlFile ($size_h) date: $date", 'interhome-xml', LOG_LEVEL_ALWAYS);
    }
  }
  
  $fp = @fopen($xmlFile,'r');     
  $line = stream_get_line($fp, 200);
  if(!strpos($line, "<$tag>")) {
    echo(form_text('first', $line, array('cols'=>80, 'rows' => 2))).html_break();
    return json_error_object("Couldn't find &lt;$tag&gt;  in first 200bytes in $xmlFile ($size_h)");
  }
  
  $first = stream_get_line($fp, 500, "<$tag>");
  if($first == false) {
    echo(form_text('first', $first, array('cols'=>80, 'rows' => 2))).html_break();
    return json_error_object("Couldn't find &lt;$tag&gt; in $xmlFile ($size_h)");
  }
  
  if(!$limit) {
    if(!$fp) return json_error_object("Couldn't open $xmlFile for reading");
    $bytes = write_file($tmp_file, $first, 'w');
    if(!$bytes) return json_error_object("Couldn't write to $tmp_file");
    $len = strlen($first);
  }
    
  $reader = new XMLReader();
  $reader->open($xmlFile);
  $output = "";

  // options
  
  $time_start = microtime(true);
  
  $dom = new DOMDocument;
  $dom->preserveWhiteSpace = FALSE;

  if($action == 'filter') { // filter original file to subset or split into files
    write_file($xml_selection, "<?xml version=\"1.0\"?>\n<accommodations>\n", 'w');
  }
    
  
  /* @var $users XMLReaderNode[] - iterate over all <user> elements */
  $nodes = new XMLElementIterator($reader, $tag);
  //dump($nodes);
  
  /* @var $filteredUsers XMLReaderNode[] - iterate over elements with id="3" or id="8" */
  /**
  $filters = array('GB', 'HR');
  if(0) {
    $key = 'country';
    $filteredNodes = new XMLAttributeFilter($nodes, $key, $filters);
  } else {
    $filteredNodes = $nodes;
  }

  */
  
  $processed = $total = $added = $updated = $active = 0;
  $all_attributes = $latlngs = array();
  $source_id = 30;
  $host_id = 17772;
  $site_id = $__SITE->id;
  $files = array();
  $count = count($nodes);
  $ts = now();
  print_log("$ts: $file Start!", 'interhome-xml', LOG_LEVEL_ALWAYS);

  
  foreach ($nodes as $node) {    
    if($limit && $total >= $limit) {
      echo("breaking because $total >= $limit");
      break;
    }
    
    $total++;    
    $i = $total;

    $xml = $node->readOuterXml();
    $len = strlen($xml);
    
    $obj = json_decode(json_encode(simplexml_load_string($xml))); // plain ole object

    if($filter_key && $filter_value && isset($obj->$filter_key)) {
      if(is_array($filter_value)) {
        if(!in_array($obj->$filter_key, $filter_value)) continue;
      } else{
        if($obj->$filter_key != $filter_value) continue;
      }
    }
    
    $country = $obj->country;

    if(!$action) return json_error_object("No action");
    switch($action) {
    case 'pretty': /** add newlines */
      $pretty_file = str_replace('.xml', '.pretty.xml', $xmlFile); /** side effect at end, writes tag */
      
      if(!$started) {
        write_file($pretty_file, "<?xml version=\"1.0\"?>\n<{$tag}>", 'w');
        print_log("Started writing to $pretty_file", 'interhome-xml', LOG_LEVEL_TEST);
        $started = true;
      }
      
      $dom->loadXML($xml);
      $dom->formatOutput = TRUE;
      $xml_pretty = $dom->saveXml();
      $xml_pretty = str_replace('<?xml version="1.0"?>', '', $xml_pretty);
      if(!write_file($pretty_file, $xml_pretty, 'a')) return json_error_object("Couldn't write to $pretty_file");      
      break;
      
    case 'split_code': /** find accommodation with given code in country file: accommodations_CC.xml */
      $code = $options['code'];
      if(!$code) return json_error_object("split_code: code missing");
      $country = $options['country'];
      if(!$country) return json_error_object("split_country: country missing");

      $dir = clean_path("/web/data/hitabrick");
      $code_dir = "$dir/xml/interhome/xml/code";
      if(!is_dir($code_dir) && mkdir($code_dir, 0700, true)) return json_error_object("Couldn't make $code_dir");
      
      $apt_file = str_replace($file, "code/{$file}", $xmlFile);
      $apt_file = str_replace("{$country}.xml", "{$code}.xml", $file);
      $out_file = "{$code_dir}/{$apt_file}";
      
      //die("file=$file<br>apt:$apt_file<br>out:$out_file");
      $dom->loadXML($xml);
      $dom->formatOutput = TRUE;
      $xml_pretty = $dom->saveXml();
      $xml_pretty = str_replace('<?xml version="1.0"?>', "<?xml version=\"1.0\"?>\n<accommodations>", $xml_pretty);
      if(!write_file($out_file, "{$xml_pretty}\n</accommodations>", 'w')) return json_error_object("Couldn't write to $out_file");
      break;
      
    case 'split': /** split accommodations.xml per country, only include those close to golf course (if set) */

      $file = $files[$country];
      if(!$file) {
        $file = str_replace('.xml', "_".$country.".xml", $xmlFile);
        $files[$country] = $file;
        write_file($file, "<?xml version=\"1.0\"?>\n<accommodations>", 'w');
        print_log("Started writing to $file", 'interhome-xml', LOG_LEVEL_TEST);
      }
      
      $dom->loadXML($xml);
      $dom->formatOutput = TRUE;
      $xml_pretty = $dom->saveXml();
      $xml_pretty = str_replace('<?xml version="1.0"?>', '', $xml_pretty);
      $stats[$country]++;
      if(!write_file($file, $xml_pretty, 'a')) return json_error_object("Couldn't write to $file");
      break;

    case 'service':
      $fid = $obj->accommodationcode;
      $our_id = interhome_listed($fid);
      if(!$our_id) continue;
      $obj->created = today();
      
      list($id, $errors) = add_object('interhome_service', $obj);
      $added++;
      if($errors) return json_error_object($errors);
      break;

    case 'vacancy':
    case 'availability':
      $fid = $obj->code;
      $obj->created = today();
      
      if($fids) {
        if(in_array($fid, $fids)) continue;     
      } else {
        $our_id = interhome_rental($fid, true);
        if(!$our_id) continue;
      }
            
      $data = ['calendar_startdate'    => $obj->startday, 
               'calendar_availability' => $obj->availability, 
               'calendar_changeover'   => $obj->changeover, 
               'calendar_minstay'      => $obj->minstay, 
               'calendar_flexbooking'  => $obj->flexbooking];
      
      list($id, $errors) = update_object('property', $data, $our_id);
      if($errors) return json_error_object($errors);

      //$code=$obj->code;
      //$start=$obj->startday;
      //echo("$code ($start)<br>");
      //echo("$code ($start)<br>");
      //break;
      
      /** use Redis
      if(0 && $_SESSION['redis_cache']) {
        global $redis;
        $key = "interhome_availability:{$fid}";
        unset($obj->created);
        unset($obj->startday);
        unset($obj->code);
        $redis->set($key, json_encode($obj), 3600*24);
      } else {
        $exObj = find_object('interhome_availability', array('code' => $fid));
        if($exObj) {        
          list($id, $errors) = update_object('interhome_availability', $obj, $exObj->id);
          $updated++;
        } else {
          list($id, $errors) = add_object('interhome_availability', $obj);
          $added++;
        }
        if($errors) return json_error_object($errors);
      }
      */
      
      break;
      
    case 'weeklyprice':
    case 'price':
      $fid = $obj->code;
      $obj->created = today();
      $our_id = interhome_listed($fid);
      if(!$our_id) continue;

      if($action == 'weeklyprice') {
        dump($obj);
      }
      if(0 && $_SESSION['redis_cache']) { /** too big */
        global $redis;
        $key = "interhome_price:{$fid}";
        unset($obj->created);
        unset($obj->code);
        $redis->set($key, json_encode($obj), 3600*24);
      } else {
        list($id, $errors) = add_object('interhome_price', $obj);
      }
      
      $added++;
      if($errors) return json_error_object($errors);
      break;

    case 'overview':
      dump($obj);die();
      break;

      
    case 'filter':
      $golf_distance = LARGE_NUMBER;
      
      if(in_array('golfing', $attributes)) {
        $distances = $obj->distances->distance;
        if(is_array($distances)) {
          foreach($distances as $distance) {
            if($distance->type = 'golf') $golf_distance = $distance->value;
          }
        }
        
        if($max_distance && $golf_distance > $max_distance) continue; // too far away
          
        $dom->loadXML($xml);
        $dom->formatOutput = TRUE;
        $xml_pretty = $dom->saveXml();
        $xml_pretty = str_replace('<?xml version="1.0"?>', '', $xml_pretty);
        $stats[$country][$golf_distance]++;
        if(!write_file($xml_selection, $xml_pretty, 'a')) return json_error_object("Couldn't write to $xml_selection");
        
      } else {
        continue;
      }
      break;

    case 'import':
      $json = new stdClass;
      $name = $obj->name;
      $country = $obj->country;
      $fid = $obj->code;
      $code = $options['code'];
      
      $data = array('active' => 1, 'host_id' => $host_id, 'source_id' => $source_id, 'site_id' => $__SITE->id, 'legal_entity_id' => $__SITE->legal_entity_id, 'client_id' => $__SITE->client_id);
      $id = 0;
      $add = isset($options['add']) ? $options['add'] : 1;
      $exObj = find_object('shortstay', array('source_id' => $source_id, 'guid' => $fid), '', array('fields' => array('id', 'active' /**, 'amenities'*/)));
      $id = $exObj->id;
      

      $overwrite = $options['overwrite'] || $_GET['overwrite'] ? 1 : 0;
      $overwrite = 1; /** todo: look at date updated ? */
      if(!$exObj->active && !$overwrite) continue;

      
      $lat = $obj->geodata->lat;
      $lng = $obj->geodata->lng;

      $golf_distance = location_find_nearest('golf_club', $lat, $lng, array('limit' => 1, 'return' => 'distance'));
      if($golf_distance) {
        $golf_distance = $distObj->golf_club = $golf_distance * 1000;
        $data['golf_distance'] = $golf_distance;
      }
      if($max_distance && (!$golf_distance || $golf_distance > $max_distance)) {
        echo("$fid: Distance = $golf_distance - too far away (max: $max_distance)<br>");
        continue;
      }

      $map = array(
        'code' => 'guid', 
        'name' => 'name', 
        'country' => 'address_country',
        'zip' => 'address_postcode',
        'pax' => 'max_guests',
        'sqm' => 'living_size',
        'rooms' => 'num_rooms',
        'bedrooms' => 'num_bedrooms',
        'bathrooms' => 'num_bathrooms',
        'toilets' => 'num_toilets',

        /** not yet mapped */
        // 'type' => 'type',
        // 'details' => 'details',
        // 'quality' => 'quality',
        // 'brand' => 'brand',
      );
      foreach($map as $k => $v) $data[$v] = $obj->$k;
      
      $json_keys = array('type', 'details', 'quality', 'brand', 'region', 'place');
      foreach($json_keys as $k) $json->$k = $obj->$k;
      $data['data'] = json_encode($json);
      
      $lat = $data['location_lat'] = $obj->geodata->lat;
      $lng = $data['location_long'] = $obj->geodata->lng;
      $data['fids'] = json_encode(array($source_id => $fid));
      
      $latlng = $lat.$lng;
      if($latlngs[$latlng]) {
        echo("<b>skipping same loc: $lat/$lng</b><br>");
        continue;
      }
      $latlngs[$latlng] = 1;
      
      $type = $obj->type;
      
      if(!round(abs($lat) + abs($lng))) {
        echo("<b>Missing lat/lng: $lat/$lng</b><br>");
        continue;
      }
      if($rating = $obj->rating) {
        $data['review_score'] = $rating->overallrating * 2;
        $data['review_count'] = $rating->numberofreviews;
      }
      
      if($pictures = $obj->pictures->picture) {
        if(is_object($pictures)) $pictures = array($pictures);        
        $photos = array();
        foreach($pictures as $picture) $photos[] = $picture->url;
        $data['media'] = $media_list = implode(',', $photos);
      }
      $data['media_json'] = media_list2json($media_list);
      
      $city_id = $data['city_id'] = rental_find_city($country, array2obj($data));
      
      if(is_object($name)) {
        if(!$type || !in_array($type, array("D","T","A"))) {
          echo("<b>name of $fid is object and no type ($type)</b><br>");
          continue;
        } else {
          if($type == "A") $name = "Apartment";
          else $name = ($type == "D" ? "Detached" : "Terraced")." House";
        }
        $data['name'] = $name;
      }
      
      if(is_object($name)) {
        return json_error_object("Name is object".dump($name,true));
      }
      
      if($city_id) {
        $data['city_name'] = $data['address_city'] = get_object('geo_city', $city_id, 'name');
        $ascii = get_object('geo_city', $city_id, 'asciiname');        
        $data['address_street'] = ""; 
        $data['shortname'] = url_safe("$ascii-$name-".str_replace('.', '', $fid));
        
      } else {
        echo("Did not find city for $name in $country lat=$lat lng=$lng <br>");
        continue;
      }
      
      $data['hosting_room_type'] = 102;
      $data['cancellation_policy'] = CANCELLATION_POLICY_STRICT;
      
      if($type == "A") $data['hosting_property_type'] = 100;
      else $data['hosting_property_type'] = ($type == "T" ? 127 : 101);            
      
      $attributes = array();
      if($obj->attributes) {
        $attributes = $obj->attributes->attribute;
        $all_attributes = array_unique(array_merge($all_attributes, $attributes));
      }
      $data['amenities'] = count($attributes) ? json_encode($attributes) : '';
      if(count($attributes)) {
        $facs = array();
        foreach($attributes as $attribute) {
          if($fac = interhome_facility_map($attribute)) $facs[] = $fac;
        }
        asort($facs);
        $data['facilities'] = implode(',', $facs);
      }
      
      $distObj = new stdClass;
      if($distances = $obj->distances->distance) {
        if(is_object($distances)) $distances = array($distances);
        foreach($distances as $dist) {
          $key = $dist->type;
          $val = $dist->value;
          if(!$key) {
            dump($dist);dump($distances);die();
          }
          $distObj->$key = $val;
        }
      }
      if($data['active']) $active++;      
      $data['import_timestamp'] = now();      
      if($distObj) $data['distances'] = json_encode($distObj);
      
      //echo("read-{$action} id={$id} overwrite={$overwrite} data:".dump($data,true));die();

      if($id) {
        $updated++;
        $locked_json = $exObj->locked ?: '["name"]'; /** name is locked by default */
        $locked = json_decode($locked_json);
        foreach($locked as $fld) unset($data[$fld]); /** ignored when updated - keep manual edit */
        list($id, $errors) = update_object('shortstay', $data, $id); 
        $db_action = 'updated';
        
      } else {
        $data['activation_timestamp'] = now();      
        $added++;
        list($id, $errors) = add_object('shortstay', $data); 
        $db_action = 'added';
      }
      
      if($id) {
        //echo("OK: $i: $db_action $name $id ($fid) $lat/$lng<br>");
      } else {
        //echo("ERROR: not $db_action $name $id ($fid)<br>");        
      }
      $count++;
      break;
            
    case 'servicedestination':
      $code=$obj->code;
      $start=$obj->startday;
      echo("$code ($start)<br>");
      echo("$code ($start)<br>");
      dump($obj);die();
      break;
    case 'salesoffice':
      $code=$obj->code;
      $name=$obj->name;
      $country = $obj->country;
      echo("$code ($name) $country<br>");
      break;
      
    case 'summary':
      $fid = $obj->code;
      $pid = interhome_listed($fid);         
      if($pid) {
        $min = round($obj->minrentalprice / 7);
        $max = round($obj->maxrentalprice / 7);
        if($min > 0 && $max > 0) {
          $avg = round(($min+$max) / 2); // avg daily rate
          list($id, $errors) = update_object('shortstay', array("rate_day" => $avg, "rate_day_min" => $min, "rate_day_max" => $max, "currency" => "EUR"), $pid);
        }
      } else {
        //echo("Could not find $fid<br>");
      }

      //dump($obj);
      break;
    case 'inside':
    case 'outside':
      $fid = $obj->code;
      if(!$fid) return json_error_object("No code");

      $aptObj = interhome_rental($fid);      
      if($aptObj && $aptObj->active) {
        $desc = nl2br($obj->text);
        $desc_type = $action == 'outside' ? 4 : 50;
        $descData = array('property_type' => 'shortstay', 'property_id' => $aptObj->id, 'type' => $desc_type);
        if($did = find_object('property_description', $descData, 'id')) {
          $descData['site_id'] = $site_id;        
          $descData['host_id'] = $host_id;        
          $descData['description'] = $desc;        
          $descData['source_id'] = $source_id;        
          //list($did, $errors) = update_object('property_description', $descData, $did); 
          $db_action = 'updated';
          $updated++;
        } else{
          $descData['description'] = $desc;        
          $descData['site_id'] = $site_id;        
          $descData['host_id'] = $host_id;        
          $descData['source_id'] = $source_id;        
          list($did, $errors) = add_object('property_description', $descData); 
          if($errors) return json_error_object($errors);
          $db_action = 'added';
          $added++;
        }
      } else {
        $skipped++;
        continue;
      }
      break;
   
    default:  
      die("Unknown action $action");
    }

    $processed++;
    
    if(!$limit) write_file($tmp_file, $xml, 'a'); // write what we have processed to tmp file for recovery     
  }

  $ts = now();
  $time_end = microtime(true);
  $time = round($time_end - $time_start,2);

  $end_tag = "</{$tag}s>";
  if(!$limit) {
    write_file($tmp_file, $end_tag."\n", 'a'); // write what we have processed to tmp file for recovery     
    rename($tmp_file, "{$tmp_file}.done");
    print_log("$ts: $file Done! Elapsed: $time\n\n", 'interhome-xml', LOG_LEVEL_ALWAYS);
    
  }
  
  if($filter) {
    write_file($xml_selection, "</accommodations>\n", 'a');
    write_file($xml_selection.".result", dump($result,true), 'w');
  }

  /** write end tag and log */
  if(count($files)) {
    foreach($files as $file) {
      write_file($file, "</accommodations>\n", 'a');
    }
  }

  if($pretty_file) {
    write_file($pretty_file, "</{$tag}>\n", 'a');
  }
  
  $countries = array_keys($result);

  if(count($countries)) {
    foreach($countries as $country) {
      $file = $files[$country];
      $message .= "$ts: country:$country elapsed:$time file:$file total:$total ".$stats[$country]."\n";
    }
  } else {
    $message = "$ts: elapsed:$time file:$xmlFile total:$total processed:$processed";
    if($added || $updated) $message .= " added:$added updated:$updated active:$active";
  }
  print_log("{$message}\n\n", 'interhome-xml', LOG_LEVEL_ALWAYS);
    
  $messages[] = $message;    
  
  $stats['total'] = $total;
  $stats['processed'] = $processed;
  $stats['added'] = $added;
  $stats['updated'] = $updated;
  $stats['active'] = $active;
  $stats['skipped'] = $skipped;
  
  $result->success = 1;
  $result->stats = $stats;
  $result->errors = $errors;
  $result->messages = $messages;
  if($code = $options['code']) {
    $result->code = $code;
    $result->data = $data;
  }
  
  /** logging 
  asort($result);
  foreach($result as $k => $v) {
    $ar = $v;
    ksort($v);
    $result[$k] = $v;
  }
  */
  
  
  
  return $result;
  
}


/** 
  Map interhome facility to our own;
*/

// return array of all listed interhome ids
function interhome_ids() {
  $source_id = 30;
  $ids = find_objects('shortstay', array('source_id' => $source_id, 'active' => 1), '', array('fields' => array('distinct guid')));
  return $ids;
}

function interhome_rental($fid, $id_only = false) {
  $source_id = 30;          
  $aptObj = find_object('shortstay', array('source_id' => $source_id, 'guid' => $fid));
  if(!$aptObj) return null;
  return $id_only ? $aptObj->id : $aptObj;
}

function interhome_listed($fid) {
  $exObj = interhome_rental($fid);
  return $exObj->active ? $exObj->id : 0;
}

function interhome_facility_map($name = '') {
  $facMap = array(
  'airconditionning' => '100', // air conditioning                  
  'balcony' => '140', // balcony/terrace                   
  'terrace' => '140', // balcony/terrace                   
  'bath' => '150', // bathtub                           

  'childrenplayground' => '134', // child friendly                    
  'poolchildren' => '134', // child friendly                    
  'babycot' => '134', // child friendly                    
  
  'dishwasher' => '230', // dishwasher                        
  'elevator' => '260', // elevator                          
  'fireplace' => '310', // fireplace                         
  'fitness' => '300', // fitness                           
  'parkingprivate' => '500', // free parking
  
  'garden' => '360', // garden                            
  'poolindor' => '690', // indoor swimming pool              
  'internet' => '170', // internet/wi-fi
  'dsl' => '170', // internet/wi-fi
  'wlan' => '170', // internet/wi-fi


  'whirlpool' => '420', // jacuzzi                           
  'microwave' => '470', // microwave oven                    
  'nonsmoking' => '480', // no smoking                        
  'pool' => '937', // outdoor swimming pool             
  'oven' => '490', // oven                              
  'petsallowed' => '130', // pets allowed                      
  'tv' => '180', // satellite/cable TV                
  'sauna' => '600', // sauna                             
  'seaview' => '790', // sea view                          
  'changelinien' => '621', // sheets/towels                     
  'washingmachine' => '730', // washing machine                   
  'tumbler' => '240', // dryer                   

  // '' => '909', // central heating                   
  // '' => '800', // mountain view                     
  // '' => '132', // smoking allowed                   
  // '' => '942', // walking distance from golf course 
  // '' => '760', // wheelchair access                 
  // '' => '191', // clubs available                   
  // '' => '133', // elder access                      
  // '' => '311', // fire extinguisher                 
  // '' => '941', // golf view                         
  // '' => '390', // hair dryer                        
  // '' => '400', // heating                           
  // '' => '380', // iron                              
  );     
  return $name ? $facMap[$name] : $facMap;
}


// wrapper for interhome_webservice() with error handling. 
// returns JSON response object
function interhome_method($method, $params = array()) {
  $live = $_SESSION['live'];
  
  $success = 0;   
  $errors = $messages = [];

  $response = new stdClass;
  $response->success = 0;
  
  if(!$live) {
    $result->success = 1;
    $result->message = ["Not live; Simulated Interhome {$method}"];    
    return $result;
  }
  
  if($result = interhome_webservice($method, $params)) {
        
    $responseName = $method."Result"; 
    if($result = $result->$responseName) {
      $success = $result->Ok == 1;
      $response->result = $result;
      
      if($success) {
        $response->success = 1;
      } else {
        $ih_errors = $result->Errors->Error ?: array();
        
        if(is_object($ih_errors)) $ih_errors = array($ih_errors);
        foreach($ih_errors as $ih_error) $errors[] = $ih_error->Description;
        $response->success = 0;
        $response->error = $errors;        
      }
    }
  }
  
  return $response;
}

function interhome_webservice($method, $params = array()) {

  // Set username and password
  $ih_soap_user = 'NL1001285';
  $ih_soap_pass = 'HITABRICK234';

  // Set soap namespace
  $ih_soap_ns = 'http://www.interhome.com/webservice';
  // Create new instance of SoapClient with the interhome web service URL
  $client = new SoapClient('https://webservices.interhome.com/partnerV3/WebService.asmx?WSDL');
  
  // Create new soap header and set username and password
  $header = new SoapHeader($ih_soap_ns, 'ServiceAuthHeader', array('Username' => $ih_soap_user,'Password' => $ih_soap_pass), true);
  
  // Prepare parameters
  $params = array('inputValue' => $params);
  
  // Call web service
  $result = $client->__soapCall($method, array('parameters' => $params),null,$header);

  print_log("Interhome webservice: code=$code method=$method result=".strlen(dump($result,true)), 'interhome-xml', LOG_LEVEL_ALWAYS);
  return $result;

}

// cancel: 404017100528

function interhome_booking($resObj_or_id, $options = []) {
  $live = $_SESSION['live'];
  $result = new stdClass;
  $result->success = 0;
  
  $resObj = any2obj('reservation', $resObj_or_id);
  $aptObj = get_object($resObj->property_type, $resObj->apt_id);
  $checkin = $resObj->checkin;
  
  $errors = $messages = [];
  
  if($resObj->status < STATUS_PROPOSED) return json_error_object("Reservation status not proposed, reserved or complete");  
  if($aptObj->source_id != SOURCE_ID_INTERHOME) return json_error_object("Not an interhome property ({$aptObj->source_id})");
  //if(!$live && $checkin < '2017') return json_error_object("Checkin must be next year unless live ({$checkin})");
  
  $res_id = $resObj->res_id;
  
  $first = $live ? $resObj->first_name : "TEST {$resObj->first_name}";
  $last  = $live ? $resObj->last_name : "TEST {$resObj->last_name}";
  
  $code = $aptObj->guid;
  $method = "RetailerBooking";
  $params = array(
    'SalesOfficeCode' => INTERHOME_SALES_OFFICE,
    'RetailerCode' => INTERHOME_RETAILER_CODE,
    'AccommodationCode' => $code, 
    'CheckIn' => strip_time($resObj->checkin),
    'CheckOut' => strip_time($resObj->checkout),
    'CurrencyCode' => 'EUR',
    'LanguageCode' => 'EN',
    'Adults' => $resObj->num_guests,
    'Children' => 0,
    'Babies' => 0,
    //'CustomerSalutationType' => $resObj->salutation,
    'CustomerSalutationType' => 'MrAndMrs',
    'CustomerName' => $last,
    'CustomerFirstName' => $first,
    'PaymentType' => 'Invoice',
    
    'CreditCardType' => 'VISA',
    'CreditCardNumber' => '4111111111111111',
    'CreditCardCvc' => '111',
    'CreditCardExpiry' => '203001',
    'CreditCardHolder' => 'test test',
    );

  
  if(!$live) {
    $result->fid = $booking_id = "test-fid";
    $result->success = 1;
    $result->message = ["Not live; Simulated Interhome {$method} for rid:{$res_id}  booking ID = {$booking_id}"];
    return $result;
  }
  
  /** live, call interhome */  
  $response = interhome_method($method, $params);
  
  $ih_result = $result->data = $response->result;
  
  if($errors = $response->errors ?: $response->errors) $result->error = $errors;
  
  if($booking_id = $ih_result->BookingID) {
    $result->success = 1;
    $result->fid = $booking_id;
    $result->message = ["OK, Interhome booking ID = {$booking_id}"];
    
    /** class handler does this */
    //list($rid, $rerrors) = update_object('reservation', ["fid" => $booking_id], $res_id);     
  } else {
    $result->error = "No booking ID received";
  }
  
  return $result;
}

function interhome_cancel_booking($resObj_or_id, $options = []) {
  $resObj = any2obj('reservation', $resObj_or_id);
  $live = $_SESSION['live'];
  $method = "CancelBooking";
  $res_id = $resObj->res_id;
  $fid = (int) $resObj->fid;

  if(!$live) return json_success_object("Not live; Simulated {$method} on rid={$res_id} fid={$fid}");

  $aptObj = get_object($resObj->property_type, $resObj->apt_id);
  
  if($aptObj->source_id != SOURCE_ID_INTERHOME) return json_error_object("Not an interhome property ({$aptObj->source_id})");
  if(!$fid) return json_error_object("Missing interhome ID ({$resObj->fid})");
  

  $code = $aptObj->guid;
  $params = array(
    'SalesOfficeCode' => INTERHOME_SALES_OFFICE,
    'RetailerCode' => INTERHOME_RETAILER_CODE,
    'BookingID' => $fid,
    );

  print_log("Calling Interhome {$method} for $res_id ($fid)", "interhome", LOG_LEVEL_ALWAYS);
  return interhome_method($method, $params);
}

function interhome_ratify_booking($resObj_or_id, $options = []) {
  $resObj = any2obj('reservation', $resObj_or_id);

  $live = $_SESSION['live'];
  $res_id = $resObj->res_id;
  $fid = (int) $resObj->fid;
  $method = "RatifyBooking";
  $code = $aptObj->guid;

  //return json_error_object("fid=$fid");
  
  if(!$live) return json_success_object("Not live; Simulated {$method} on rid={$res_id} fid={$fid}");
  
  $aptObj = get_object($resObj->property_type, $resObj->apt_id);
  if($aptObj->source_id != SOURCE_ID_INTERHOME) return json_error_object("Not an interhome property ({$aptObj->source_id})");
  if(!$resObj->fid) return json_error_object("Missing interhome ID ({$resObj->fid})");

  $params = array(
    'SalesOfficeCode' => INTERHOME_SALES_OFFICE,
    'RetailerCode' => INTERHOME_RETAILER_CODE,
    'BookingID' => $fid,
    );
  print_log("Calling Interhome {$method} for $res_id ($fid)", "interhome", LOG_LEVEL_ALWAYS);
  return interhome_method($method, $params);

}



function interhome_set_active($max_distance) {
  $source_id = 30;
  $site_id = 263;
  $host_id = 17772;
  $yesterday = add_day(today(), -1);
  $last_month = add_day(today(), -30);
  
  $common = "source_id={$source_id} and host_id={$host_id} and site_id={$site_id}";
  
  $before_count = db_getOne("SELECT count(id) FROM property WHERE {$common} and active=1");  
  
  /** enable */
  $where = "{$common} and active=0 and timestamp > '{$yesterday}' and rate_day > 0 and calendar_startdate >= '$last_month' and golf_distance >0 and golf_distance <= {$max_distance}";
  $enable_count = db_getOne("SELECT count(id) FROM property WHERE $where");
  db_query("update property set active=1 where $where");

  /** disable */
  $where = "{$common} and active=1 and (timestamp < '{$yesterday}' or rate_day <= 0 or calendar_startdate < '$last_month' or golf_distance = 0 or golf_distance > {$max_distance})";
  $disable_count = db_getOne("SELECT count(id) FROM property WHERE $where");
  db_query("update property set active=0 where $where");
  
  $after_count = db_getOne("SELECT count(id) FROM property WHERE {$common} and active=1");
  
  return json_success_object("interhome_set_active: before: $before_count enabled:$enable_count disabled:$disable_count after: $after_count");
}



/**
0 - 500: 10%
500 - 1000: 9%
1000 - 1500: 8%
1500 - 2000: 7%
2000+: 6%
*/

/**
availability: 
Y – available
N – not available
Q – on request 

changeover:
X - no action possible
C - check-in and check-out
O - check-out only
I - check-in only
*/

function interhome_slice($vacObj, $key, $in, $out) {
  $start = $vacObj->startdate;
  $val = $vacObj->$key; 
  if($in < $start) return ''; // in the past
  if($out < $in) return ''; // invalid
  
  $len = strlen($val);
  $days = datediff('d', $in, $out);
  $start = datediff('d', $start, $in);
  if($start + $days > $len) return ''; // out is after last known day

  if($key == 'minstay') {
    $slice = $val[$start];
  } elseif($key == 'changeover') {
    $slice = $val[$start].$val[$start + $days];
  } else {
    $slice = substr($val, $start, $days);  
  }

  //echo("$key: $slice<br>");  
  //die("start: $start in:$in out:$out len=$len days=$days start:$start");
  //print_log("$fid: $key: $slice", 'interhome', LOG_LEVEL_TEST);
  return $slice;  
}

function interhome_get_availability($fid) {
  if($_SESSION['redis_cache']) {
    global $redis;
    $key = "interhome_availability:{$fid}";
    $value = $redis->get($key);
    $vacObj = $value ? json_decode($value) : '';
  } else {
    $vacObj = find_object('interhome_availability', array('code' => $fid));
  }
  return $vacObj;
}

// returns 1 if directly bookable, -1 if on-request, 0 if not available or bookable
function interhome_availability($fid, $in, $out) {
  $vacObj = interhome_get_availability($fid);

  if(!$vacObj) return [0, "Missing availability"]; // no availability, do not show
  
  $test = $_SESSION['dev'] && false;
  $days = datediff('d', $in, $out);
  //dump($vacObj);
  
  // availability
  $avail = interhome_slice($vacObj, 'availability', $in, $out);
  if(!$avail) return [0, "Availability not found or out of range"];
  
  if(!$test) if(strpos($avail, "N") !== false) return [0, "Rental is already booked"]; // of the days is booked
  $on_request = strpos($avail, "Q") !== false;

  // changeover
  $change = interhome_slice($vacObj, 'changeover', $in, $out);
  if(!$change) return [0, "Changeover not found or out of range"];
  
  if(!$test) if(strpos($change, "X") !== false) return [0, "Checkin/out not allowed on these days"]; // no change over  
  if(!$test) if($change[0] == "O") return [0, "Checkin not allowed on $in"]; // checkout only
  if(!$test) if($change[1] == "I") return [0, "Checkout not allowed on $out"]; // checkin only

  // minimum stay
  $minstay = interhome_slice($vacObj, 'minstay', $in, $out);
  if(!strlen($minstay)) return [0, "Mininum stay not found or out of range"]; 
  if(!$test) if($minstay == '0') return [0, "Mininum stay: not bookable"]; // false; // not bookable
  $min_stay = ord($minstay) - ord("A") + 1; // C=3, D=4 etc
  if(!$test) if($days < $min_stay) return [0, "Mininum stay is $min_stay days"]; // min stay  

  // flexbooking
  $whole_weeks = $days % 7 ? 0 : 1;
  //print_log("days:$days whole_weeks: $whole_weeks", 'interhome', LOG_LEVEL_TEST);
  if(!$whole_weeks) {
    $flex = interhome_slice($vacObj, 'flexbooking', $in, $out);
    if(!$test) if(strpos($flex, "N") !== false) return [0, "Only whole weeks allowed (7/14/etc days)"]; // no flex booking over
  }
  
  
  // not sure how to use this yet
  //print_log("$fid is available: on_request=$on_request", 'interhome', LOG_LEVEL_TEST);
  return $on_request ? [-1,''] : [1,''];
}

/** unsused - automatic comment added on  */
function interhome_price_count($fid, $in, $out) {
  $search = array('code' => $fid, 'startdate' => array('<', $out), 'enddate' => array('>', $in));
  $count = count_objects('interhome_price', $search);
  return $count;
}

// return average daily rate for interhome property
function interhome_pricing($fid, $in, $out, $options = array()) {
  $average = isset($options['average']) ? $options['average'] : true;
  
  $days = datediff('d', $in, $out);
  $search = array('code' => $fid, 'startdate' => array('<', $out), 'enddate' => array('>=', $in));
  $options = array('sort' => 'startdate');
  $priceObjs = find_objects('interhome_price', $search, $options) ?: [];
  if(!$average) return $priceObjs;
  
  $count = count($priceObjs);
  //if(!$count) return "$in-$out"; // no pricing found
  if(!$count) return 0; // no pricing found
  
  $grand_total = 0;
  foreach($priceObjs as $priceObj) {
    $fixed = $priceObj->fixprice;
    $price = $priceObj->rentalprice;
    $start = $priceObj->startdate;
    $end = $priceObj->enddate;
    $end = date("Y-m-d", strtotime($end."+1days"));   // add one day, as enddate of pricing is last calendar date, not checkout date
    $int = dates_intersect($start, $end, $in, $out);    
    $total = $int*$price;
    $grand_total += $total;
    if($fixed > 0) $grand_total += $fixed;
    //print_log("$fid: in=$in out=$out count=$count price=$price fixed=$fixed", 'interhome', LOG_LEVEL_TEST);
    
  }
  $avg = number_format($grand_total/$days, 2, '.', '');
  return $avg;  
}

function interhome_dayrate($rates, $date, $num_days=0) {
  foreach($rates as $rate) {    
    if($date >= $rate->startdate && $date <= $rate->enddate) {
      $fixed = $rate->fixprice > 0 && $num_days > 0 ? $rate->fixprice / $num_days : 0; 
      return $rate->rentalprice + $fixed;
    }
  }
  return 0;
}

function intherome_live_availability($code, $in, $out, $ng) {    
  require_once("xml_interhome.inc");
  $result = new stdClass;
  $errors = $messages = [];
  $success = 0;   
  
  $method = "PriceDetail";
  $params = array(
    'AccommodationCode' => $code, 
    'CheckIn' => $in,
    'CheckOut' => $out,
    'SalesOfficeCode' => "2048",
    'CurrencyCode' => 'EUR',
    'LanguageCode' => 'EN',
    'Adults' => $ng,
    'Children' => 0,
    'Babies' => 0,
    );

  print_log("interhome_webservice: calling {$method} for code:{$code} in:{$in} out:{$out} ng: {$ng}", 'interhome', LOG_LEVEL_TEST);
  
  if($ih_result = interhome_webservice($method, $params)) {
    if($ih_result = $ih_result->PriceDetailResult) {
      $success = $ih_result->Ok == 1 ? 1 : 0;     
      if($success) {
        $result->rate = $rate = $ih_result->Price;
        $result->total = $total = $ih_result->Total;  
        $result->currency = $currency = $ih_result->CurrencyCode;
        $messages[] = "OK: $code is available $in to $out for $ng guests: Rate: $currency $rate Total={$currency} {$total} ";                 
        
        $services = $ih_result->AdditionalServices->AdditionalServiceItem;
        if(is_object($services)) $services = array($services);
        
        foreach($services as $k=>$service) {
          if($service->Code == "RTCO") unset($services[$k]); // don't show Agent discount
        }
        $result->services = $services;
        print_log("interhome_webservice: {$rate} Total: {$total} cur: {$currency} services:".count($services), 'interhome', LOG_LEVEL_TEST);
        
      } else {
        $ih_errors = $ih_result->Errors->Error ?: array();
        if(is_object($ih_errors)) $services = array($ih_errors);
        foreach($ih_errors as $ih_error) $errors[] = $ih_error->Description;
      }
    }
  }
  $result->success = $success;
  $result->error = $errors;
  $result->message = $messages;
  return $result;
}



/** disable interhome properties without recent availability (today or yesterday)
// return number disabled
/** unsused - automatic comment added on  */
function interhome_disable_inactive($country = '') {
  return 0;
  $search = ['source_id' => 30, 'active' => 1];
  if($country) $search['address_country'] = $country;  
  $aptObjs = find_objects('shortstay', $search);
  $i = 0;
  $yesterday = yesterday();
  foreach($aptObjs as $aptObj) {
    $vacObj = interhome_get_availability($aptObj->guid); // fast: redis
    if(!$vacObj || $vacObj->startdate < $yesterday) {
      $i++;
      update_object('shortstay', ['active' => 0], $aptObj->id);
    }
  }
  return $i;
}

// set price or deactivate those that miss a price
/** unsused - automatic comment added on  */
/**
function interhome_fix_missing($options = array()) {
  $limit = $options['limit'] ?: 0;
  
  $pObjs = find_objects('shortstay', array('source_id' => 30, 'active' => 1, 'rate_day' => 0), array('sort' => 'created', 'limit' => $limit));
  $rows = array();
  $rows[] = array('created', 'name', 'id', 'guid', 'country', 'prices', 'vacancy', 'average');
  $count = $count_before = count($pObjs);
  
  $output = html_h2("$count properties miss prices");
  //return $output;
  
  foreach($pObjs as $pObj) {
    $id = $pObj->id;
    $fid = $pObj->guid;
    $pcount = count_objects('interhome_price', array('code' => $fid));    

    //
    $vcount = count_objects('interhome_availability', array('code' => $fid));
    if(!$pcount) {
      update_object('shortstay', array('active' => 0), $id);
      print_log("deactivating $id ($fid): p=$pcount v=$vcount", 'interhome-xml', LOG_LEVEL_ALWAYS);      
    } else {
      $avgObj = find_object('interhome_price', array('code' => $fid), array('field' => 'avg(rentalprice) as avg'));
      $avg = round($avgObj->avg);
      list($id, $errors) = update_object('shortstay', array('currency' => 'EUR', 'rate_day' => $avg), $id);
      //echo("set average of $id ($fid): $avg<br>");
      if($errors) {dump($errors); die();}      
    }
    //echo("Average for $fid:".dump($avg,true));die();    
    
    
    $rows[] = array(strip_time($pObj->created), $pObj->name, $pObj->id, $pObj->guid, $pObj->address_country, $pcount, $vcount, $avg);
    
  } 
  
  $pObjs = find_objects('shortstay', array('source_id' => 30, 'active' => 1, 'rate_day' => 0), array('sort' => 'created', 'limit' => $limit));
  $count = $count_after = count($pObjs);
  
  $output .= html_table($rows, array('class' => 'web20'));
  return array($output, $count_before, $count_after);
}

*/
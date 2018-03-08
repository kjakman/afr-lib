<?php

/*
  $url = "https://www.airbnb.com/calendar/ical/69040.ics?s=a4418e8da905d34e53d36e3510e6d330"; // Studio C
  $url = "https://www.airbnb.com/calendar/ical/69042.ics?s=ed7d9a6191006609851669b1c673375f"; // Studio D
  $map = array('Canal Apartments Studio C' => 2909,'Canal Apartments Studio D' => 2908); 
*/

function airbnb_import($apt_id, $url, $map) {
  require_once("/web/comps/lib/php/ical/class.iCalReader.php");
  $path = $_SESSION['data_dir'].'iCal/airbnb';
  $path_parts = pathinfo($url);
  $updated = $added = 0;
  print_log("airbnb_import {$apt_id} {$url}", 'import', LOG_LEVEL_TEST);
  if($filename = $path_parts['filename']) {
    $full_file = "$path/$filename.ics";
    //dump($path_parts);exit;    
    $options = array('debug' => 0, 'find' => array("\n "), 'replace' => array(''));
    $result = url2file($url, $full_file, $options);
    if($result->error) return json_error_object($result->error);
    db_query("DELETE FROM reservation WHERE source_id > 0 AND res_id < 0 AND apt_id=$apt_id");
    
    if($result->success) {
      $ical   = new ICal($full_file);
      $events = $ical->events();
      $count = count($events);
      print_log("airbnb_import {$apt_id} OK: $count events", 'import', LOG_LEVEL_TEST);
      
      foreach($events as $event) {
        foreach($event as $k => $v) { // get rid of pesky parameters in key 
          if(strpos($k, ';')) {
            list($new_key) = explode(';', $k);
            //echo("nk=$new_key v=$v<br>");
            $event[$new_key] = $v;
            unset($event[$k]);
          }
        }
        $data = airbnb_ical2res($apt_id, $event, array("source" => SOURCE_AIRBNB, "map" => $map));
        if(!$data) continue;
        
        //dump($event);dump($data);die();

        $fid = $data['fid'];
        $source_id = $data['source_id'];
        //dump($data);die();
        $complete = true;
        $required = array("source_id", "fid", "apt_id", "checkin","checkout");
        $output = '';
        foreach($required as $k) {
          if(!$data[$k]) {
            $complete = false;
            $output .= " - Missing $k<br>";
          }          
        }
        print_log("airbnb_import {$apt_id} fid:{$fid} Missing: $output", 'import', LOG_LEVEL_TEST);
        //echo($output);die();  
        
        // $complete = $source_id && $fid && $data['apt_id'] && $data['checkin'] && $data['checkout'];
        if(!$complete) {
          $output .= "Not complete, skipping<br>"; //.dump($data, true);          
          continue;
        }
        
                  
        $apt_name = $data['apartment_name'];
        $search = array('source_id' => $source_id, 'fid' => $fid);
        $client_id = $_SESSION['client_id'];
        
        // $res_id = find_object('reservation', $search, 'res_id', array('debug_query' => 0));        
        if(0) { /** AirBnB fid changes for each import. Not very handy. Thus we delete all and add */
          //echo("found res".dump($res_id,true));die();
          list($id, $errors) = update_object('reservation', $data, $res_id);
          $action = 'update';
          $updated++;
        } else {
          //echo("could not did not find source_id=$source_id and fid={$fid} client_id={$client_id}<br>");
          $data['client_id'] = $client_id;
          $data['status'] = STATUS_BLOCKED;
          $preset_id = nextId('cal_import'); // negative reservation ID
          list($id, $errors) = add_object('reservation', $data, array('debug_query' => 0, 'preset_id' => $preset_id));
          $resObj = get_object('reservation', $id);
          //print_log("preset_id: $preset_id id:$id res_id: {$resObj->res_id}", 'import', LOG_LEVEL_ALWAYS);
          $action = 'add';
          $added++;
        }
        if($errors) {
          return json_error_object($errors);
          //dump($errors);die();
        }
        $log_text = "$source_id: $action cal_import {$resObj->res_id} ($apt_name) fid:$fid in:{$data['checkin']} out:in:{$data['checkout']}";
        print_log($log_text, 'import', LOG_LEVEL_ALWAYS);
        $output .= "$log_text<br>";
        
        //echo($output);die();  
      }
      
      //$output .= dump($events, true);
    } else {
      print_log("airbnb_import {$apt_id}: url2file failed", 'import', LOG_LEVEL_TEST);
      return json_error_object("url2file failed");      
      die("url2file failed");
    }
  } else {
    return json_error_object("No filename:".dump($path_parts, true));      
    die("No filename".dump($path_parts, true));
  }
  return json_success_object("Added: $added Updated: $updated");
  return $output;
}

function airbnb_ical2res($apt_id, $event, $options = array()) {
  $map = $options['map'];
  $source_id = $options['source'];
  $ptype = $options['property_type'] ?: 'shortstay';
  $location = $event['LOCATION'];  
  
  //die("Location=$location Map=".dump($map, true).dump($event,true));
  if($location && is_array($map)) {    
    if($map_apt_id = $map[$location]) $data['apt_id'] = $apt_id;
    if($map_apt_id && $map_apt_id != $apt_id) {
      //echo("airbnb_ical2res: $location does not map to $apt_id<br");
      //dump($map);
      // die("mapped id=$map_apt_id");
      print_log("Mapped ID:$map_apt_id does not match id:$map_id", "import", LOG_LEVEL_ALWAYS);
      return [];
    } else {
      //echo("Ok. $map_apt_id = $apt_id<br>");
    }
  }

  $data = array();
  $aptObj = get_object($ptype, $apt_id);
  //list($data, $errors) = apt2res($data, $aptObj, false); /** Note: new system returns just $data */
  $data = apt2res($data, $aptObj, false); /** Note: new system returns just $data */

  //dump($data);die();
  
  $uid = str_replace(array('<', '>'), '', $event['UID']);
  if(!$uid) $uid = uniqid();
  $data['source_id'] = $source_id;
  
  $data['checkin'] = date('Y-m-d H:i:s', strtotime($event['DTSTART']));
  $data['checkout'] = date('Y-m-d H:i:s', strtotime($event['DTEND']));
  $data['notes'] = str_replace('\n', "\n", $event['DESCRIPTION']);
  
  $summary = $event['SUMMARY'];
  $pattern = "/([^\(]+) \(([a-zA-Z0-9]+)\).*/";
  if(preg_match($pattern, $summary, $match)) {
    $name = $match[1];
    $nameAr = explode(' ', $name);

    $data['last_name'] = array_pop($nameAr);
    $data['first_name'] = join(' ', $nameAr);    
    if($match[2]) $data['fid'] = $fid = $match[2];
    $data['status'] = STATUS_BLOCKED;
    $data['fids'] = json_encode(array("$source_id" => "$fid"));
  } else {
    $data['last_name'] = $summary;
    //return null; // not an airbnb reservation
    //dump($event);
    //die("no match");
  }
  if(!strlen($data['fid'])) $data['fid'] = $uid;
  return $data;
}         


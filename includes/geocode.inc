<?php
require_once('helpers_google.inc');
// geocoding UK postal codes
// http://www.tomanthony.co.uk/blog/geocoding-uk-postcodes-with-google-map-api/

// takes any object with address information, returns array (status, location_accuracy, location_lat, location_long)
function geocode_address($addObj) {

	global $db;
  $address = $address_street = $addObj->address_street;
  if($address_number = $addObj->address_number) {
    $address.=" $address_number";
  }
  
  $address_city = $addObj->address_city;
  $address_city = str_replace('\\', "", $address_city);
  
  $address_postcode = $addObj->address_postcode;
  $address_country = $addObj->address_country;
  if(!$address || !$address_city || !$address_country) return null;

  $country = db_getOne("select name from countries where id='$address_country'");
  $address="$address, $address_postcode $address_city, $country";
  list($status, $acc, $lat, $long, $bounds) = google_geocode($address);
	$mapObj->status = $status;
	$mapObj->location_accuracy = $acc;
	$mapObj->location_long= $long;
	$mapObj->location_lat = $lat;
	$mapObj->location_bounds = $bounds;

	return $mapObj;
}

function geocode($apts, $type, $override=false) {
  global $db;
  $messages = array();
  if(!is_array($apts)) $apts = array($apts);
  foreach($apts as $apt) {
    $id = $apt->id;
    if(!$id) continue;
    $lac = $apt->location_accuracy;
    if($apt->defaultapt) continue;

    $address = $address_street = $apt->address_street;
    if($address_number = $apt->address_number) {
      $address.=" $address_number";
    }
    
    $address_city = $apt->address_city;
    $address_city = str_replace('\\', "", $address_city);

    $address_postcode = $apt->address_postcode;
    $address_country = $apt->address_country;
    
    if(!$override && $apt->location_accuracy>0) {
      $messages[] = "location acc already set, skipping: '$apt->location_accuracy'"; // address is missing
      continue; // location is already set
    }
    if(!$apt->address_street) {
      $messages[] = "location street missing: city=$address_city '$apt->address_street'"; // address is missing
      // dump($apt);
      continue; // address is missing
    }
    if(!$apt->address_city) {
      $messages[] = "location city missing: street='$apt->address_street'"; // address is missing
      // dump($apt);
      continue; // address is missing
    }
    if(!$address_country) {
      $messages[] = "country missing: '$address_country' using NL"; // address is missing
//      dump($apt);
      $address_country = 'NL'; // default to NL for now
    }
    
    $country = db_getOne("select name from countries where id='$address_country'");

    $address="$address, $address_postcode $address_city, $country";

    list($status, $acc, $lat, $long, $bounds) = google_geocode($address);
        
    if($status==200 && $acc)  {
      $qs = "UPDATE property SET location_lat='$lat', location_long='$long', location_accuracy='8', location_bounds='$bounds'  WHERE id=$id";
      $messages[] = "Found $address on map: $lat / $long. Accuracy = $acc";
      //echo($add."<br>".$qs."<br>");
      db_query($qs);
      $qs = "UPDATE property SET location_lat='$lat', location_long='$long', location_accuracy='8', location_bounds='$bounds' WHERE id=$id";
      db_query($qs);
    } elseif(sizeof($result) == 4 && $status>=1 && $status < 1000)  {
      if($lac == NULL) {
        //echo("Lac was NULL<br>");
        $nlac=0;
      } elseif($lac<=0) {
        //echo("Lac was NOT NULL, but $lac<br>");
        $nlac=$lac-1;
      } else {
        //echo("Lac was neither NULL nor <= 0, but $lac<br>");        
      }

      $qs = "UPDATE property SET location_accuracy=$nlac WHERE id=$id";
      db_query($qs);
      $qs = "UPDATE property SET location_accuracy=$nlac WHERE id=$id";
      db_query($qs);
      $messages[] = "Problem with address $address for apt $id: Status: $status Accuracy: $acc";
    } else {
      $messages[] = "Problem with address $address for apt $id: Status: $status Accuracy: $acc";
      dump($result);
      //die();
    }
  }
  return $messages;
}

function geocode_loc($locations) {
  if(!is_array($locations)) $locations = array($locations);
  global $db;
  foreach($locations as $loc) {
    $loc_id = $loc->id;
    $map_id = $loc->map_id;
    $address_street = $loc->address_street;
    $address_number = $loc->address_number;
    $address_city = $loc->address_city;
    $address_postcode = $loc->address_postcode;
    //$address_country = $loc->address_country;
    //$address_city = get_field('city', 'code', $apt->address_city);
    $address_country = get_field('country', 'code', $loc->address_country);
    //$address_country = get_object_value('country', 'code', $loc->address_country);
    
    //if($loc->location_accuracy) continue; // location is already set
    if(!$loc->address_street) continue; // address is missing
    if(!$loc_id) die("Apt $id missing location id"); // address is missing

    $add="$address_street $address_number, $adddress_postcode $address_city, $address_country";
    $q=urlencode($add);
    $site = $_SERVER['HTTP_HOST'];
    $key = $host_sub=='dev' ? "ABQIAAAAKWsIzllCGVIR689AqZv1iBTCjpyz5srgts45bnDeBzFFf65dZBTeaLfjtf7dj8wuw3W8MORNbvVdcw" : "ABQIAAAAKWsIzllCGVIR689AqZv1iBQVyQUrsvEk94JRgSZBdjeO5TOggBQaTq-PrOGhpsaPuBctAjEbysYPXg";
    if($address_country=='GB') {
      $url = "http://$site/amsterdam/geocode_uk.php?postcode=".urlencode($address_postcode);
      $result = get_html($url);  
      $body = find_delimited_str($result, '<body>', '</body>');      
      $result2 = find_delimited_str($body, 'result=%', '%');
    } else {
      $url = "http://maps.google.com/maps/geo?output=csv&key=$key&q=$q";
      $result = get_html($url);  
    }
    


    dump("result from $url<br>body=$body '$result'<br><hr>");
    $result = split(",", $result);
    //dump($result);
    
    $status = $result[0];
    $acc = $result[1];
    $lat= $result[2];
    $long = $result[3];

    if($status==200 && $acc) {
      if(!$map_id)  {
        $map_id = nextId('map');
        $qs = "INSERT INTO map (id, location_lat, location_long, location_accuracy) VALUES ($map_id, '$lat', '$long', '$acc')";
        //echo($add."<br>".$qs."<br>");
        db_query($qs);

        $qs = "UPDATE location SET map_id=$map_id WHERE id=$loc_id";
        //echo($add."<br>".$qs."<hr>");
        db_query($qs);
      } else {
        $qs = "UPDATE map SET location_lat='$lat', location_long='$long', location_accuracy=$acc WHERE id=$map_id";
        //echo($add."<br>".$qs."<hr>");
        db_query($qs);
      }
    } else {
      echo("<br><hr><b>Problem with address for apt $id: $add - status: $status accuracy: $acc</b><br>Check the spelling of the street name, city<hr>");
    }
  }
}


function get_name_chain($type, $id) {
  global $db;
  if(!$id) return ""; //die("get_name_chain: no id");
  $types_list = array('continent', 'country', 'region', 'city', 'district', 'location');
  $type_index = array_search($type, $types_list);
  //echo("get_name_chain: types index for type='$type' id='$id' index = '$type_index'<br>");
  if(!$type_index) return FALSE; // return false for 'continent' and if not found
  $parent_type = $types_list[$type_index-1];
  //echo("get_name_chain: parent type of $type = $parent_type<br>");
  
  $locObj = db_getRow("SELECT * FROM $type WHERE id=$id");
  //dump($locObj);
  if(!$locObj) die("get_name_chain for type=$type, id=$id: $type $id not found");

  $name = $locObj->name;

  if(!$name) die("get_name_chain for type=$type, id=$id: no name");

  $p= $locObj->parent_id;
  $gp= $locObj->grandparent_id;

  // skip region for now
  if($type=='city') return $name;
  //if($type=='region') return $name;
  //if($type=='region') return get_name_chain($parent_type, $p);;
  
  // skip parent if gp is set
  if($gp) {
    $grandparent_type = $types_list[$type_index-2];
    if($grandparent_type) { //die("get_name_chain for type=$type, id=$id: grandparent_type = $grandparent_type; must be 1 or higher");
      $gpn = get_name_chain($grandparent_type, $gp);
      //echo("get_name_chain: grandparent type of $type = $grandparent_type - returning grandparent_name ($gpn) + $name ($type)<br>");
      return $gpn ? "$name, $gpn" : $name;
    } else {
      //echo("get_name_chain: grandparent type of $type is NOT > 1, returning just $name ($type)<br>");
      return $name;
    }
  } elseif($p) {
    $pn = get_name_chain($parent_type, $p);
    //echo("returning parent_name ($pn) + $name ($type)<br>");
    return $pn ? "$name, $pn" : $name;
  } else {
    //echo("returning just $name ($type)<br>");
    return $name;
  }  
}


function geocode3($type) {
  global $db;
  $types_list = array('continent', 'country', 'region', 'city', 'district', 'location');
  $type_index = array_search($type, $types_list);
  //$qs = "SELECT * FROM $type WHERE active=1 AND id>0";
  //$qs = "SELECT * FROM $type WHERE active=1 AND NOT map_id AND id>0";
  $qs = "SELECT * FROM $type WHERE NOT map_id AND id>0";
  $types = db_getAll($qs);
  $count = count($types);
  echo("geocode3: $type; found $count locations. qs=$qs<br>");
  $total=$ok=$failed = 0;
  foreach($types as $loc) {
    $id = $loc->id;
    $address = get_name_chain($type, $id);
    echo("address for $type $id = $address<br>");

    $map_id = $loc->map_id;
    $name = $loc->name;
    if($loc->location_accuracy) continue; // location is already set
    
    list($status, $acc, $lat, $long, $bounds) = google_geocode($address);

    $total++;
    if($status==200 && $acc) {
      echo("address '$address' = lat: $lat, long: $long, accuracy: $acc<hr>");
      $ok++;
      if(!$map_id)  {
        $map_id = nextId('map');
        $qs = "INSERT INTO map (id, location_lat, location_long, location_accuracy) VALUES ($map_id, '$lat', '$long', '$acc')";
        db_query($qs);
        $qs = "UPDATE $type SET map_id=$map_id WHERE id=$id";
        db_query($qs);
        echo "$add - <b>INSERT INTO</b> map (id, location_lat, location_long, location_accuracy) VALUES ($map_id, '$lat', '$long', '$acc')";
      } else {
        $mapObj = db_getRow("SELECT * FROM map WHERE id=$map_id");
        if($acc= $mapObj->location_accuracy) {
          echo("Map exists and accuracy=$acc: do nothing - ".$add - $qs."<hr>");
        } else {
          $qs = "UPDATE map SET location_lat='$lat', location_long='$long', location_accuracy=$acc WHERE id=$map_id";
          db_query($qs); // don't update if set
          echo("Map exists but accuracy=0: ".$add - $qs."<hr>");
        }
      }
    } else {
      $failed++;
      echo("<b>Problem with address '$address' <br>status: $status accuracy: $acc</b><hr>");
    }
  }
  echo("<hr><h1>$type - $total total, $ok ok, $failed failed</h1><hr>");
}

function get_msNameChain($type, $id) {
  $level = ms_index($type);
  $chain = array();

  while($level > 0) {
    $geoObj = get_msObj($type, $id);
    $chain[] = $geoObj->name;    
    $level--;
    
    if($level<=2) break; // region
    
    $pType = ms_type($level);    
    
    //if($pType == 'region') {
    //  $id = $geoObj->parent_id;
    //  $type = ms_type($level);
    //} else {      
    //  $id = $geoObj->parent_id;
    //  $type = $pType;
    //}
    
    $id = $geoObj->parent_id;
    $type = $pType;

  }

  return implode(",", $chain);
}

function msGeocodeAll($type, $new=false) {
  global $db;
  $qs = "SELECT id FROM property WHERE location_accuracy IS NULL";
  if($new) $qs .= " OR (location_accuracy<=0 AND location_accuracy>-3)"; 
  $cs = db_getAll($qs);
  $count = count($cs);
  echo("$count $type objects need geocoding<br>");
  foreach($cs as $c) {
    msGeocode($type, $c->id);
  }
}

function msGeocode($type, $id) {
  global $db;

  $address = get_msNameChain($type, $id);
  echo("Address of $type $id = $address<br>");  
  list($status, $acc, $lat, $long, $bounds) = google_geocode($address);
  
  if($type=='city' && $status != 200) { // failed, try just city, country (w/o state/district);
    $cityObj = get_msObj('city', $id);
    $countryObj = get_msObj('country', $cityObj->grandparent_id);
    $city_name = $cityObj->name;
    $country_name = $countryObj->name;
       
    $address = "$city_name, $country_name";
    echo(" *** Retrying with address of $type $id = $address<br>");  
    list($status, $acc, $lat, $long, $bounds) = google_geocode($address);
  }


  if($status==200 && $acc) {
    echo("address '$address' = lat: $lat, long: $long, accuracy: $acc<hr>");
    $qs = "UPDATE property SET location_lat='$lat', location_long='$long', location_bounds='$bounds', location_accuracy=$acc WHERE id=$id";
    db_query($qs); // don't update if set
  } else {
    echo("<b>Problem with address '$address' <br>status: $status accuracy: $acc</b><hr>");
    $pacc = db_getOne("SELECT location_accuracy FROM property WHERE id=$id");
    if(!$pacc) $pacc=0; // force to integer if NULL  
    $pacc--;
    $qs = "UPDATE property SET location_accuracy=$pacc WHERE id=$id";
    db_query($qs); // don't update if set
  } 
}

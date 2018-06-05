<?php

/** find center/bounds of objects in array */
function location_object_bounds($objects, $radius=0) {
  $min_lat = 90;  
  $max_lat = -90;  
  $min_lng = 180;  
  $max_lng = -180;
  
  if(!is_array($objects) || !count($objects)) return array([0,0], [[$min_lat,$min_lng],[$max_lat,$max_lng]]);
  $obj = $objects[0];
  if(!isset($obj->location_lat) || !isset($obj->location_long) || !strlen($obj->location_lat) || !strlen($obj->location_long)) array([0,0], [[$min_lat,$min_lng],[$max_lat,$max_lng]]);
  
  foreach($objects as $obj) {
    if($obj->location_lat < $min_lat) $min_lat = $obj->location_lat; 
    if($obj->location_lat > $max_lat) $max_lat = $obj->location_lat; 

    if($obj->location_long < $min_lng) $min_lng = $obj->location_long; 
    if($obj->location_long > $max_lng) $max_lng = $obj->location_long; 
  }
  
  if($radius > 0) {
    $add_degrees = meters2degrees($radius);
    $min_lat -= $add_degrees; 
    $min_lng -= $add_degrees; 
    $max_lat += $add_degrees; 
    $max_lng += $add_degrees; 
  }
  
  $lat = ($min_lat + $max_lat) / 2;
  $lng = ($min_lng + $max_lng) / 2;
  return array([$lat,$lng], [[$min_lat,$min_lng],[$max_lat,$max_lng]]);
}

// add location for give object
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_location($obj_type, $obj_or_id) {
  $obj = any2obj($obj_type, $obj_or_id);
  $lat = $obj->location_lat;
  $lng = $obj->location_long;
  if(!$lat && $lng) return false;

  $parent_id = get_object_id($obj_type, $obj);

  $locObj = find_object('location', ['parent_type' => $obj_type, 'parent_id' => $parent_id]);
  $id = 0;
  if($locObj) {
    $id = $locObj->id;
    if($locObj->location_lat == $lat && $locObj->location_long == $lng) {
      echo("{$obj_type} {$obj->name} exists ($id): correct loc<br>");
      return 0;
    } else {
      echo("{$obj_type} {$obj->name} exists ($id): incorrect loc: {$locObj->location_lat} != $lat && {$locObj->location_long} != $lng<br>");
    }
  } else {
    echo("{$obj_type} {$obj->name} <b>missing loc</b><br>");
  }


  $locObj = obj2loc  ($obj_type, $obj, true);
  //dump($obj);
  $locObj->parent_id = $parent_id;
  $locObj->parent_type = $obj_type;
  $locObj->address_formatted = $obj->address_formatted;
  //dump($locObj);die();

  if($id) {
    list($id, $errors) = update_object('location', $locObj, $id);
  } else {
    list($id, $errors) = add_object('location', $locObj);
  }
  if($errors) {
    dump($errors);
    die();
  }
  return $id;  
}


/** http://stackoverflow.com/questions/14750275/haversine-formula-with-php
 * Calculates the great-circle distance between two points, with
 * the Haversine formula.
 * @param float $latitudeFrom Latitude of start point in [deg decimal]
 * @param float $longitudeFrom Longitude of start point in [deg decimal]
 * @param float $latitudeTo Latitude of target point in [deg decimal]
 * @param float $longitudeTo Longitude of target point in [deg decimal]
 * @param float $earthRadius Mean earth radius in [m]
 * @return float Distance between points in [m] (same as earthRadius)
 */
/** unsused - automatic comment added on  */
function haversine_distance($latitudeFrom, $longitudeFrom, $latitudeTo, $longitudeTo, $earthRadius = 6371000) {
  // convert from degrees to radians
  $latFrom = deg2rad($latitudeFrom);
  $lonFrom = deg2rad($longitudeFrom);
  $latTo = deg2rad($latitudeTo);
  $lonTo = deg2rad($longitudeTo);

  $latDelta = $latTo - $latFrom;
  $lonDelta = $lonTo - $lonFrom;

  $angle = 2 * asin(sqrt(pow(sin($latDelta / 2), 2) +
    cos($latFrom) * cos($latTo) * pow(sin($lonDelta / 2), 2)));
  return $angle * $earthRadius;
}

// distance with your arccosine formula
/** unsused - automatic comment added on  */
function location_distance($center_lat, $center_lng, $lat, $lng, $options=[]) {
  $factor = 3959; // miles
  $factor = 6371; // kilometers
  
  $distance = ($factor * acos((cos(deg2rad($center_lat)) ) * (cos(deg2rad($lat))) * (cos(deg2rad($lng) - deg2rad($center_lng)) )+ ((sin(deg2rad($center_lat))) * (sin(deg2rad($lat))))));
  return $distance;
}

// MySQL Great Circle Distance (Haversine formula)
// http://stackoverflow.com/questions/574691/mysql-great-circle-distance-haversine-formula
// radius in miles
function location_find_nearest($obj_type, $lat, $long, $options = array()) {
  if(!(abs($lat) > 0 && abs($long) > 0)) return array(); // no location
  $class_fields = object_fields($obj_type);
  $factor = 3959; // miles
  $factor = 6371; // kilometers
  $return = $options['return'];
  $distance_only = $return == 'distance';

  $fields = $options['fields'] && is_array($options['fields']) ? $options['fields'] : array('id', 'name');
  $limit = isset($options['limit']) ? $options['limit'] : 10;
  $radius = $options['radius'] ?: 100; // limit result to this radius
  //$table = $options['table'] ?: 'location';
  $key = get_object_key($obj_type);
  $table = get_object_table($obj_type);
  $fields_str = implode(',', $fields);

  $search = $options['search'] ?: array();
  if(in_array('client_id', $class_fields)) $search['client_id'] = $_SESSION['client_id'];
  if($exclude = $options['exclude']) $search[$key] = array('!=', $exclude); // exclude this ID
  if($search) {
    $filters = search2filters($search);
    $where = "WHERE $filters";
  }

  if(in_array('location_lat', $class_fields)) { // for objects with built-in location
    if(in_array('active', $class_fields)) $filters[]= "active=1";
    if(is_property_type($obj_type) && $obj_type != 'property') $filters[] = "type='$obj_type'";
    $where = count($filters) ? " WHERE ".implode(' AND ', $filters) : '';

    if(isset($options['fields'])) {
      $fields_str = is_array($options['fields']) ? implode(',', $options['fields']) : $options['fields'];
    } else {
      $fields_str = "*";
    }
    
    $fields = $fields_str ?: "*";
    $star = $distance_only ? "" : "{$fields_str},";
    $query = "
      SELECT $star ($factor * acos(cos(radians($lat)) * cos( radians(location_lat)) * cos(radians(location_long ) - radians($long) ) + sin(radians($lat) ) * sin(radians(location_lat)))) AS distance
      FROM $table $where
      HAVING distance < $radius
      ORDER BY distance".($limit ? " LIMIT 0,$limit" : '');
      //echo($query.html_break());
  } else { // for objects using location table
    $filters[] = "location.parent_type = '$obj_type'";
    $filters[] = "location.parent_id=$table.$key";
    $where = "WHERE ".implode(' AND ', $filters);

    $star = $distance_only ? "" : "$table.*,";
    $query = "
      SELECT $star ($factor * acos(cos(radians($lat)) * cos( radians(location_lat)) * cos(radians(location_long ) - radians($long) ) + sin(radians($lat) ) * sin(radians(location_lat)))) AS distance
      FROM location, $table
      $where
      HAVING distance < $radius
      ORDER BY distance".($limit ? " LIMIT 0,$limit" : '');

  }

  $debug = 0;
  if($debug) {
    echo("Obj_type=$obj_type Key=$key<br>");
    dump($search);
    dump($filters);
    echo($query);
  }

  if($distance_only) {
    return $limit == 1 ? round(db_getOne($query),3) : db_getCol($query);
  } else {
    $locObjs = db_getAll($query);
  }
  return $locObjs;

};

// takes accuracy, returns radius in meters
function accuracy2radius($loc, $accuracy) {
  if(is_array($loc)) $loc = (object) $loc;
  if($accuracy < 0) $accuracy = 0;
  $exponent = (9 - $accuracy);
  $base = $exponent < 0 ? 2 : 3;
  $kilometers =$exponent < 0 ? 1 / pow($base, -1*$exponent) : pow($base, $exponent);
  return $kilometers * 1000;
}

// adds radius (in meters) to location and returns bounds
// if array is true, returns [[],[]] format
function radius2bounds($loc, $radius, $array = false) {
  if(is_array($loc)) $loc = (object) $loc;
  $add_degrees = meters2degrees($radius);
  $lat = $loc->location_lat;
  $lng = $loc->location_long;
  $ne->lat = round(minmax($lat + $add_degrees, -85, 85), LOCATION_ACCURACY);
  $ne->lng = round(minmax($lng + $add_degrees, -180, 180), LOCATION_ACCURACY);
  $sw->lat = round(minmax($lat - $add_degrees, -85, 85), LOCATION_ACCURACY);
  $sw->lng = round(minmax($lng - $add_degrees, -180, 180), LOCATION_ACCURACY);
  $bounds->northeast = $ne;
  $bounds->southwest = $sw;
  return $array ? [[$sw->lat,$sw->lng],[$ne->lat,$ne->lng]] : $bounds;
}

// takes point + accuracy (0 is lowest, 8 is roughly city (3km radius), 9 is 1km radius, 10 is 0.5, 11 is 0.25km etc), returns bounds (string)
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function accuracy2bounds($loc, $accuracy) {
  $radius = accuracy2radius($loc, $accuracy);
  $bounds = radius2bounds($loc, $radius);
  return $bounds;
}

function loc2center($locObj) {
  if(!$locObj || !isset($locObj->location_lat) || !isset($locObj->location_long)) return [];
  return [$locObj->location_lat, $locObj->location_long];
}

// returns point at the center of bounds
function bounds2center($bounds) {
  $locObj = bounds2loc($bounds);
  return [$locObj->location_lat, $locObj->location_long];
}


function any2bounds($bounds, $array = false) {
  if(!$bounds) return $array ? [] : new stdClass;
  if(is_object($bounds)) $boundsObj = $bounds;  
  else if(is_array($bounds)) $boundsObj =  bounds_array2obj($bounds);
  elseif(strlen($bounds)) $boundsObj =  bounds_json2obj($bounds);
  return $array ? bounds2array($boundsObj) : $boundsObj;
}

// returns point at the center of bounds
function bounds2loc($bounds) {
  $boundsObj = any2bounds($bounds);
  $ne = $boundsObj->northeast;
  $sw = $boundsObj->southwest;
  
  $locObj = new stdClass;
  $locObj->location_lat = ($ne->lat + $sw->lat) / 2;
  $locObj->location_long = ($ne->lng + $sw->lng) / 2;
  return $locObj;
}

// takes bounds object, returns array
function bounds2array($boundsObj) {
  if(!$boundsObj || !isset($boundsObj->southwest) || !isset($boundsObj->northeast)) return [];
  return [[$boundsObj->southwest->lat ?: 0,$boundsObj->southwest->lng ?: 0], [$boundsObj->northeast->lat ?: 0,$boundsObj->northeast->lng ?: 0]];
}

// takes boundsObj, return json array used in query string and DB((sw_lat,sw_lng),(ne_lat,nw_lng))
function bounds_obj2json($boundsObj) {
  return json_encode(array(array($boundsObj->southwest->lat ?:0, $boundsObj->southwest->lng ?: 0), array($boundsObj->northeast->lat ?: 0, $boundsObj->northeast->lng ?: 0)));
}

// takes [[],[]], returns old style ne/sw
function bounds_array2obj($bounds) {
  $boundsObj = new stdClass();
  if($bounds) {
    $boundsObj->southwest->lat = $bounds[0][0];
    $boundsObj->southwest->lng = $bounds[0][1];
    $boundsObj->northeast->lat = $bounds[1][0];
    $boundsObj->northeast->lng = $bounds[1][1];
  }
  return $boundsObj;
}

// takes bounds from old query string in format ((sw_lat,sw_lng),(ne_lat,nw_lng)) or new array [[],[]], returns bounds object (nw,se)
function bounds_json2obj($bounds_str, $searchArray = array()) {
  $bounds_json = str_replace(array('(',')'), array('[',']'), $bounds_str);  // should not be needed any more
  $bounds = json_decode($bounds_json);
  if($bounds->southwest && $bounds->northeast) return $bounds; // already in correct format
  return bounds_array2obj($bounds);
}

function bounds2radius($bounds) {
  $boundsObj = any2bounds($bounds);
  $ne = $boundsObj->northeast;
  $sw = $boundsObj->southwest;
  $d_lat = abs($ne->lat - $sw->lat);
  $d_lng = abs($ne->lng - $sw->lng);
  $diff = ($d_lat + $d_lng) / 2; // cheesy average instead of pythagoras since x and y are usually the same
  $r = degrees2meters($diff);
  return round($r / 2);
}



// takes center from query string in format [lat,lng]
// returns object with lat,long
function center2loc($center) {
  if(!is_array($center)) $center = json_decode($center);

  $locObj = new stdClass;
  $locObj->location_lat = $center[0];
  $locObj->location_long = $center[1];
  return $locObj;
}

// takes bounds json object or array, return property search array
function bounds2search($bounds, $searchArray = array()) {
  $boundsObj = any2bounds($bounds);
  if(!$boundsObj) return $searchArray;
  $searchArray['location_lat'] = array('><', $boundsObj->southwest->lat, $boundsObj->northeast->lat);
  $searchArray['location_long'] = array('><', $boundsObj->southwest->lng, $boundsObj->northeast->lng);
  return $searchArray;
}


// returns all locations in given bounds
/** unsused - automatic comment added on  */
function location_bounds_search($boundsObj, $search = array(), $options = array()) {
  if(!$boundsObj) return null;
  $locSearchArray = bounds2search($boundsObj, $search);
  $locSearchArray['active'] = 1;
  return find_objects('location', $locSearchArray, $options);
}

// takes an object with coordinates (landmark, city, country, airport, golf course etc)
// converts to location object, and inserts or updates
/** unsused - automatic comment added on  */
function obj2location($obj_type, $obj_or_id, $options=array()) {
  $obj = any2obj($obj_type, $obj_or_id);
  if(!$obj) return;
  $key = get_object_key($obj_type);
  $parent_id = $obj->$key;
  $debug = $options['debug'];
  $geocode = $options['geocode'];
  $output = '';
  $code = '';
  //if(!$obj->location_accuracy) return NULL;
  $errors = $messages = array();
  $name = $obj->name;
  switch($obj_type) {
    case 'continent';
      break;
    case 'region';
      break;
    case 'country';
      $default_acc = 2;
      $expected_country = $obj->code2;
      $expected_type = 'country';
      $country = $obj->code2;
      $geo_search = $name; // default
      break;
    case 'district';
      $expected_country = $obj->code2;
      $country_id = $obj->parent_id;
      break;
    case 'city';
      $expected_type = array('locality', 'sublocality', 'neighborhood', 'COLLOQUIAL_AREA', 'ADMINISTRATIVE_AREA_LEVEL_1', 'ADMINISTRATIVE_AREA_LEVEL_2');
      $city_id = $obj->id;
      $country_id = $obj->grandparent_id;
      break;
    case 'borough';
      $expected_type = array('locality', 'sublocality', 'neighborhood');
      $city_id = $obj->parent_id;
      break;
    case 'neighborhood';
      $expected_type = array('locality', 'sublocality', 'neighborhood', 'point_of_interest', 'natural_feature', 'route');
      $city_id = $obj->grandparent_id;
      break;
    break;
    case 'airport';
      $expected_country = $country = $obj->country;
      $expected_type = 'airport';
      $name = $obj->name;
      if(!stripos($geo_search, 'airport')) $name .= " airport";
      $city_name = $obj->city;
      $country_name = find_object('country', array('code2' => $country), 'name');
      $code = $obj->code;
    break;
    case 'landmark';
    break;
    case 'golf_club';
    case 'golf_course';
      $expected_type = array('golf_course', 'locality', 'sublocality');
      $name = $obj->name;
      //if(!stripos($geo_search, 'airport')) $name .= " airport";
      //echo("got here");
      //echo(dump($obj, true));exit;
      $expected_country = $country = $obj->address_country;
      $city_name = $obj->address_city;
      $country_name = find_object('country', array('code2' => $country), 'name');
    break;
    case 'geoname';
    break;
  default:
    $errors[] = "obj2location: Unsupported object type $obj_type";
    break;
  }
  if($errors) return array(0, $errors, $message);
  if($city_id) {
    $cityObj = get_object('geo_city', $city_id);
    $country_id = $cityObj->country_code;
    $city_name = $cityObj->name;
  }
  if($country_id) {
    $countryObj = get_object('country', $country_id);
    $expected_country = $countryObj->code2;
    $country_name = $countryObj->name;
  }
  if(!$geo_search) {
    $geo_search = implode(', ', array_unique(array_filter(array($name, $city_name, $country_name))));
  }
  //$data = (array) $obj;

  $data['parent_type'] = $obj_type;
  $data['parent_id'] = $parent_id;

  if($city_id) $data['city_id'] = $city_id;

  $lat = $obj->location_lat;
  $long = $obj->location_long;
  $bounds = $obj->location_bounds;
  $acc = $obj->location_accuracy ?: $default_acc;

  if($id = $obj->location_id) {
    $action = 'update';
  } elseif($locObj = find_object('location', array('parent_type' => $obj_type, 'parent_id' => $parent_id))) {
    $id = $locObj->id;
    $action = 'update';
  } else {
    $action = 'add';
    $id = '';
  }
  $messages[] = "$obj_type $parent_id: $action location $id";
  $header_row = array("Long", "Short", "Type");
  $altNameAr = array();
  $altNameAr [] = $name;
  $found_it = false;

  if($id && !$options['overwrite']) {
    $messages[] = "Found existing location $id for $geo_search";
    return array($id, $errors, $messages, $output);
  }

  $geo_code = $geo_search && ($options['geocode'] || !($lat && $long));
  if(!$geo_code) {
    $messages[] = "Not geocoding $geo_search";
    return array($id, $errors, $messages, $output);
  }

  $result = google_geocode($geo_search, array('full' => true));
  //dump($result);exit;
  $results = $result->results ?: array();
  if(!$results) {
    $errors[] = "Failed to geocode $geo_search";
    $errors[] = dump($result, true);
    return array($id, $errors, $messages, $output);
  }

  $count = count($results);
  $found_type = false;

  foreach($results as $result) {
    $rows = array();
    $rows[] = $header_row;
    $address_components = $result->address_components;

    $formatted_address = $result->formatted_address;
    $geometry = $result->geometry;
    $types = $result->types;
    $type = count($types) ? strtolower(array_shift($types)) : '';
    $parsed = parse_geometry($geometry);

    if(is_array($expected_type)) {
      foreach($expected_type as $etype) {
        if($expected_type && !strcasecmp($etype, $type)) {
          $found_type = "&#x2713;";
          break;
        }
      }
    } elseif($expected_type && !strcasecmp($expected_type, $type)) {
      $found_type = "&#x2713;";
    }

    list($status, $loc_type, $lat, $long, $bounds) = $parsed;
    //dump($geometry);dump($parsed);echo("lat=$lat");exit;
    $found_short = false;
    foreach($address_components as $address_component) {
      $check = '';
      $long_name = $address_component->long_name;
      $short_name = $address_component->short_name;
      $addr_types = $address_component->types;
      $addr_type = array_shift($addr_types); // use first
      switch($addr_type) {
      case 'establishment':
        if($long_name != $name) $altNameAr[] = $long_name;
        break;
      case 'administrative_area_level_1':
        $state_big = $short_name ?: $long_name;
        break;
      case 'administrative_area_level_2':
        $state_small = $short_name ?: $long_name;
        break;
      case 'locality':
        $city_big = $short_name ?: $long_name;
        break;
      case 'sublocality':
        $city_small = $short_name ?: $long_name;
        break;
      case 'route':
        $street = $short_name ?: $long_name;
        break;
      case 'postal_code':
        $postcode = $short_name ?: $long_name;
        break;
      case 'street_number':
        $number = $short_name ?: $long_name;
        break;
      default:
        break;
      }

      if($addr_type == 'country') {
        if($expected_country) {
          if(!strcasecmp($expected_country, $short_name)) {
            $found_short = true;
            $check = "&#x2713;";
          } else {
            $check = "&#x2717; ($expected_country))";
          }
        }
      } else {
        $check = '';
        //echo("addr_type = $addr_type<br>");
      }

      $rows[] = array($long_name, $short_name.$check, $addr_type);
    }

    $app_str = $approximate ? " (Approx) " : '';
    $caption = "$geo_search ($type $found_type)";
    $title = "$formatted_address: (".implode(",", $types).") $lat/$long ($loc_type)<br>-----<br><br>";
    //$output .= "expected: $expected_type found: $type equal=".;
    $found_it = $found_type && $found_short;
    $class = $found_it ? "green" : "red";
    $output .= html_table($rows, array('caption' => $caption, 'class' => 'web20')).html_div($title, $class);
    if($found_it) break;
  }

  if(!$found_it) {
    $errors[]= "Did not find $geo_search";
    $errors[] = dump($result, true);
    return array($id, $errors, $messages, $output);
  }

  // gecode
  if($obj->alt_names) $altNameAr[] = $obj->alt_names;
  if($code) $altNameAr[] = "($code)";
  if($altNameAr) $alt_names = implode(' ', $altNameAr);
  $data['name'] = $name ?: $obj->name;
  $data['description'] = $description ?: $obj->description;
  $data['alt_names'] = $alt_names;

  if(!$city) $city = $city_small ?: $city_big;
  if(!$state) $state = $state_big ?: $state_small;
  $data['address_street']   = $street ?: $obj->address_street;
  $data['address_number']   = $number ?: $obj->address_number;
  $data['address_city']     = $city ?: $obj->address_city;
  $data['address_postcode'] = $postcode ?: $obj->address_postcode;
  $data['address_state']    = $state ?: $obj->address_state;
  $data['address_country']  = $country ?: $obj->address_country;

  $data['location_long']     = $long;
  $data['location_lat']      = $lat;
  $data['location_bounds']   = $bounds;
  $data['location_accuracy'] = $acc;
  $data['location_zoom']     = $zoom = acc2zoom($acc);
  $data['address_formatted'] = $formatted_address;

  $messages[] = "OK: geocoded $geo_search lat=$lat long=$long acc=$acc zoom=$zoom bounds=$bounds";
  if($debug) $messages[] = dump($result, true);

  if(!$bounds) {
    $errors[] = "No bounds for $geo_search".dump($result, true);
    return array($id, $errors, $messages, $output);
  }

  if($id) {
    list($id, $errors) = update_object('location', $data, $id);
  } else {
    list($id, $errors) = add_object('location', $data);
  }
  if($id) update_object($obj_type, array('location_id' => $id), $parent_id);
  return array($id, $errors, $messages, $output);
}


// takes an array of object, returns an array indexed by lat/long of an array of dummy objects used for google map
// noise is added afterwards, so properties of same address are grouped together
function objs2locs($obj_type, $objs) {
  if(!count($objs)) return array();
  $ilocs = array();
  foreach($objs as $obj) { // make array indexed by lat/long
    if($loc = obj2loc($obj_type, $obj, true)) {
      $latlong = "$loc->location_lat,$loc->location_long";
      $ilocs[$latlong][] = $loc;
    }
  }

  //$noise = $obj_type == 'shortstay' && $siteObj->add_noise ? LOCATION_NOISE : 0; // add 200 meters of noise

  // now loop through and make "normal" array, not indexed, (and add noise if needed)
  $locs = array();
  foreach($ilocs as $latlong => $locAr) {
    $loc = $locAr[0];
    /**
    if($noise) {
      list($loc->location_lat, $loc->location_long) = add_noise($loc->location_lat, $loc->location_long, $noise); // add noise
      if(count($locAr)  > 1) foreach($locAr as $myloc) {
        $myloc->location_lat = $loc->location_lat;
        $myloc->location_long = $loc->location_long; // use same noise
      }
    }
    */
    $locs[] = count($locAr)  > 1 ? $locAr : $loc;
  }


  // now group together by property ids
  $markers = array();
  foreach($locs as $loc) {
    if(is_array($loc)) {
      $ids = array();
      foreach($loc as $l) $ids[] = $l->id;
      asort($ids);
      $id_list = implode(',', $ids);
      $markers[$id_list] = $loc;
    } else {
      $markers[$loc->id] = $loc;
    }
  }

  return $markers;
}

// takes an object, returns a dummy object used for google map
function obj2loc($obj_type, $obj, $ignore_noise=false) {
  global $__SITE;
  $siteObj = $__SITE;
  $site_id = $siteObj->id;
  $id = $obj->id;
  if(!($obj->location_lat && $obj->location_long)) return NULL;
  $loc = obj_copy($obj, array('id', 'location_accuracy', 'location_lat', 'location_long', 'name'));
  $show_address = true;
  
  switch($obj_type) {
    case 'shortstay';
      $show_address = $siteObj->hide_property_address ? false : true;
      $show_address = false;
      $sourceObj = $obj;
      $cur = currency2html($obj->currency);
      $loc->price = $label = "{$cur}{$obj->rate_day}";
      $loc->city_name = $obj->address_city;
      
      if($obj->book_direct) {        
        $loc->extra = '<i class="fa fa-bolt red"></i> '.phrase('book_direct', TITLECASE);
        $label .= '&#9889;';
      }
      
      $loc->label = $label;
      $loc->link = $link = property_geolink($obj);
      $loc->extra = "link={$link}";
      
      $loc->images = $obj->media_json; // ? media_json_sources($obj->media_json, 'square') : []; // get_media_sources($obj->media, 'square');
      //$loc->images = get_media_sources($obj->media, 'square');
      break;
      
    case 'location';
      $obj_type = $obj->parent_type;
      $obj_id = $obj->parent_id;
      $sourceObj = get_object($obj_type, $obj_id);
      if($img = object_images($obj_type, $obj_id, array('first' => true, 'size' => 'square', 'full_source' => true))) $loc->image = trim($img);
      break;
      
      
    default:
      $loc->type = $obj->type;
      $loc->description = '';
      break;
  }


  // map icon
  $icon_options['type'] = $loc->type;
  $icon_options['highlight'] = false;
  $loc->marker = gmap3_icon($obj_type, $icon_options);

  $icon_options['highlight'] = true;
  $loc->marker_hi = gmap3_icon($obj_type, $icon_options);

  if($show_address) $loc->address = get_address_string($obj);
  else $loc->address = '';//"(".phrase('approximate_location', CAPITALIZE).")";

  $loc->type = $obj_type;      
  
  if($sourceObj) {
    switch($obj_type) {
    case 'shortstay':
      break;
    case 'golf_course':
      $golf_link = site_page_link($siteObj, PAGE_GOLF_COURSE);
      if($golf_link && $obj_id) $loc->link = $course_link = $golf_link.$obj_id;
      //$loc->description = $sourceObj->short_description;
      break;
    case 'golf_club':
      $golf_link = site_page_link($siteObj, PAGE_GOLF_CLUB);
      if($golf_link && $obj_id) $loc->link = $club_link = $golf_link.$obj_id;
      $loc->golf_link = $golf_link;
      $loc->images = $sourceObj->images; // ? media_json_sources($obj->media_json, 'square') : []; // get_media_sources($obj->media, 'square');
      
      //$loc->description = $sourceObj->short_description;
      break;
    default:
      break;
    }
  }
  
  $loc->obj_type = $obj_type;

  if(0) { // debug
    $source_id = $obj->source_id;
    $loc->name .= "<br>$obj_type $id";
  }
  return $loc;
}

// alias of geodir (for city only)
/** unsused - automatic comment added on  */
function city_dir($id, $lang='') {
  if($name = get_object('geo_city', $id, 'asciiname')) return slug($name);
  return '';

  // return geodir($id, GEOLEVEL_CITY, $lang);
}

function geoname($id, $level='', $lang='') {
  return get_object('geo_city', $id, 'name');
  /** old
  $geoObj = geoobject($id, $level, $lang);
  return $geoObj->name;
  */
}

// same as geoname() but returns directory instead of name
function geodir($id, $level='', $lang='') {
  $asciiname = get_object('geo_city', $id, 'asciiname');
  return slug($asciiname);
  /** old code
  $geoObj = geoobject($id, $level, $lang);
  return $geoObj->directory;
  */
}


/** below not in use ? */

// returns array of (localized) geoobjects for given level/id
// if not found, returns the non-localized goeobject (e.g. city, country, continent)
// $level can be type of numeric
function geoobjects($id, $level='', $lang='') {
  if(!$level) $level = GEOLEVEL_CITY;
  else if(!is_numeric($level)) $level = loc_index($level);

  if(!$lang) $lang = $_SESSION['language'];
  if($geoObjs = find_objects('geoname', array('level' => $level, 'fid' => $id, 'lang' => array('=', $lang)), array('sort' => 'lang DESC, priority DESC'))) {
    //dump($geoObjs);
    //echo(dump(array('level' => $level, 'fid' => $id, 'lang' => array('=', $lang)), true)."name=".$geoObjs[0]->name.html_break());
    //die();
    return $geoObjs;
  } else { // didn't find localized name, return name from city/country etc table
    return array(get_object(loc_type($level), $id));
  }
}

// first geoobject found
function geoobject($id, $level='', $lang='') {
  $geoObjs = geoobjects($id, $level, $lang);
  //dump($geoObjs);
  return $geoObjs[0];
}


/** unsused - automatic comment added on  */
function cityname($id) {
  return geoname($id, GEOLEVEL_CITY);
}


// returns link to given loc object, including aff_id + lang if needed
function loc_link($level, $id=0, $lang='', $linkonly=true, $nolang=false) {
  if(!$lang) $lang = $_SESSION['language'];
  if($level == GEOLEVEL_WORLD) {
    $world_link = ($lang && $lang!='en') ? "/$lang/" : "/";
    return $linkonly ? $world_link : html_link($world_link, phrase('world', CAPITALIZE));
  }
  $geoObjs = geoobjects($id, $level, $lang);
  $geoObj = $geoObjs[0];
  $loc_dir = $geoObj->directory;
  $loc_name = $geoObj->name;
  $type = loc_type($level);
  //echo("loc_dir for level $level $type $id ($loc_name) in $lang = $loc_dir<br>");
  //if(!$loc_dir) dump($geoObj);
  if(!$nolang && $lang != 'en' && (count($geoObjs) > 1 || $geoObj->lang !=$lang)) $loc_dir = "$lang/$loc_dir"; // prepend language if needed
  if($aid=$_SESSION['aid']) $loc_dir = "aid$aid/$loc_dir";   // prepend affiliate id if needed
  return $linkonly ? "/$loc_dir/" : html_link("/$loc_dir/", $loc_name);
}

// returns URL for city (link only)
/** unsused - automatic comment added on  */
function city_link($id) {
  return loc_link(GEOLEVEL_CITY, $id);
}

// returns loctree object for given level / id (e.g. GEOLEVEL_CITY,1000 for City - Amsterdam)
function loc_object($level, $id) {
  if(!$level) return NULL;
  if(!$id) die("loc_object: id is blank (t=$t, id=$id)");
  return get_object(loc_type($level), $id);
}

function map_data($level, $id) {
  $children = loc_children($level, $id);
  if(count($children) == 1) return map_data($level+1, $children[0]->id);
  $trail = map_trail($level, $id);
  $sub_menu  = map_menu($level, $children);
  $menu = css_columns($sub_menu, html_header($trail,2));
  return array($level, $children, $menu);
}

// takes level + optional id, returns array of columns: (children, grand children)
function map_menu($level, $children) {
  if($level >= GEOLEVEL_CITY) return array();
  if($level == GEOLEVEL_DISTRICT) return array(loc_links($level+1, $children, '', false), '');
  foreach($children as $locObj) {
    $grand_children = loc_children($level+1, $locObj->id);
    if(count($grand_children) == 1) {
      $left[] = loc_link($level+2, $grand_children[0]->id, '', false); // only one grandchild, so use grandchild link instead of childlink
      $right[] = '';
    } elseif(count($grand_children)) {
      $left[] = loc_link($level+1, $locObj->id, '', false).":";
      $right[] = implode(', ', loc_links($level+2, $grand_children, '', false));
    } else {
      $left[] = loc_link($level+1, $locObj->id, '', false);
      $right[] = '';
    }
  }
  return array($left, $right);
}


function loc_links($level, $locObjs, $lang='', $link_only=true) {
  foreach($locObjs as $locObj) $links[] = loc_link($level, $locObj->id, $lang, $link_only);
  return $links;
}


// takes level + optional id, returns trail with links to parents
function map_trail($level, $id=0) {
  if(!$level) return html_b(phrase('world', CAPITALIZE));
  $obj = loc_object($level, $id);
  $trail []= geoname($id, $level); // add the name passed in (not link)
  while($id = $obj->parent_id) {  // keep adding while parent is set
    $level--;
    $obj = loc_object($level, $id);
    $name = geoname($obj->id, $level);
    // echo("level=$level id=$obj->id link=".loc_link($level, $obj->id))."<br>";// dump($obj);
    $num_children = loc_num_children($level, $obj->id);
    $trail_link = $num_children > 1 ? html_link(loc_link($level, $obj->id), $name) : $name;
    array_unshift($trail, $trail_link); // prepend parent link
  }
  $num_continents = loc_num_children(GEOLEVEL_WORLD, 0);
  $world_link = $num_continents > 1 ? html_link(loc_link(GEOLEVEL_WORLD), phrase('world', CAPITALIZE)) : phrase('world', CAPITALIZE);
  array_unshift($trail, $world_link); // prepend world link
  return implode(' -> ', $trail);
}

function loc_index($type) {
  global $db;
  return db_getOne("SELECT id FROM msLevel WHERE name='$type'");
}

function loc_type($index) {
  global $db;
  return db_getOne("SELECT name FROM msLevel WHERE id='$index'");
}

function loc_num_children($level, $id) {
  return sizeof(loc_children($level, $id));
}

function loc_children($level, $id) {
  if($level == GEOLEVEL_LOCATION) return 0;
  return find_objects(loc_type($level + 1), array('parent_id' => $id, 'active' => 1));
}

/** unsused - automatic comment added on  */
function zoom2acc($zoom) {
  if($zoom<=2) return 0;
  if($zoom<=4) return 1;
  if($zoom<=6) return 2;
  if($zoom<=8) return 3;
  if($zoom<=10) return 4;
  if($zoom<=12) return 5;
  if($zoom<=14) return 6;
  if($zoom<=16) return 7;
  return 8;
}

function acc2zoom($accuracy) {
  if($accuracy<=0) return 1;
  if($accuracy==1) return 3;
  if($accuracy==2) return 5;
  if($accuracy==3) return 7;
  if($accuracy==4) return 9;
  if($accuracy==5) return 11;
  if($accuracy==6) return 13;
  if($accuracy==7) return 15;
  return 17;
}

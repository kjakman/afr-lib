<?
// SOAP functions
function gsSoapClient() {
  $clientClass = 'SoapClient';
  $url =  "https://devxml.golfswitch.com/golfService.asmx?WSDL";
  $accountId  = "API"; 
    
  $client = new $clientClass($url, array("trace" => 1, "exception" => 0));
  $hdr = array('ResellerId' => 'API', 'PartnerId' => '', 'SourceCd' => 'A', 'Lang' => 'en', 'UserIp' => $_SERVER['REMOTE_ADDR'], 'UserSessionId' => '', 'AccessKey' => '', 'gsSource' => '', 'gsDebug' => ''); 
  return array($client, $hdr);
}

// get areas
function gsAreas($client, $hdr, $country='', $region='') {
  $req = array('CountryId' => $country, 'RegionId' => $region);
  $res = $client->Areas(array('Hdr'=>$hdr, 'Req'=>$req));
  return $res;
}

// get course list
function gsCourseList($client, $hdr, $country='', $region='') {
  $req = array('CountryId' => $country, 'RegionId' => $region, 'MaxDistance' => 10000, 'MaxDistanceType' => 'km', 'ShowAllStatus' => true, 'ShowDisConnected' => true, 'FeaturedOnly' => false);
  $res = $client->CourseList(array('Hdr'=>$hdr, 'Req'=>$req));
  return $res;
}

// get course info
function gsCourseInfo($client, $hdr, $course_id) {
  $req = array('CourseId' => $course_id);
  $res = $client->CourseInfo(array('Hdr'=>$hdr, 'Req'=>$req));
  return $res;
}

// Functions that update local DB

// add or update golf course
// if golfObjs is set, it will be updated. If not, function will still search for existing
function gsAddCourse($imageBase, $course, $golfObj=null) {
  $areaArray = $output = $errors = $loc_errors = array();
  $loc_id = 0;
  
  //$image = "http://devxml.golfswitch.com/img/course/$id/$image";
  $keys = array(
    'id' => 'fid',
    'sCou' => 'golf_country_code',
    'sReg' => 'golf_region_code',
    'sAr' => 'golf_district_code',
    'onReq' => 'on_request',
    'nm' => 'name',
    'lat' => 'location_lat',             
    'lon' => 'location_long',    
    'cou' => 'address_country',
    'st' => 'address_state',
    'cty' => 'address_city',
    'advDays' => 'adv_days',
    'insideDays' => 'inside_days',
    'dist' => 'length_men',
    'promo' => 'description',
    'rating' => 'rating_men',
    'img' => 'image',
    'lastUpd' => 'source_updated');
  $loc_keys = array('name', 'location_lat', 'location_long', 'address_city', 'address_state', 'address_country');
  $data = array_copy($course, $keys);
  $fid = $data['fid'];

  $data['image_base'] = "$imageBase/$fid/";
  //$data['image'] = $data['image'] ? "$imageBase/$fid/".$data['image'] : '';
  $data['source_updated'] = date('Y-m-d H:i:s', strtotime($data['source_updated']));


  // figure out country,region,district
  $city_name = $course->cty;
  $region_name = $course->sAr;
  $cCode = $course->sCou;
  $rCode = $course->sReg;
  $dCode = $course->sAr;

  //if(!($course->lat && $course->lon)) return array(0, array("Missing lat/long"));
  //if(!($cCode && $rCode && $dCode)) return array(0, array("Missing search area params"));
  
  list($c_id, $r_id, $d_id) = gsAreaIds($course, $areaArray);
  //if(!($c_id && $r_id && $d_id)) return array(0, array("Could not find area ids for $cCode/$rCode/$dCode"));

  $data['golf_country_id'] = $areaArray['country'][$cCode] = $c_id; 
  $data['golf_region_id'] = $areaArray['region'][$rCode] = $r_id; 
  $data['golf_district_id'] = $areaArray['district'][$dCode] = $d_id; 
  
  $golfObj = $golfObj ?: find_object('golf_course', array('fid' => $fid)); // search for existing

  if($course->lat && $course->lon) {
    $locData = array_copy($data, $loc_keys);
    $locData['description'] = $data['short_description'];
    $locData['alt_names'] = $data['name']." Golf Course $city_name $region_name";
    $cc = $data['address_country'];
    $country_name = $cc ? get_object('countries', $cc, 'name') : '';
    $locData['address_formatted'] = implode(',', array_filter(array($data['name'], $data['address_city'], $data['address_state'], $country_name)));
    $locData['location_radius'] = 1000;
    $locData['location_bounds'] = json_encode(radius2bounds($locData, 1000));
    $locData['parent_type'] = 'golf_course';
    $locData['parent_id'] = $golf_id;
    $locData['accuracy'] = 8;
    $locData['zoom'] = 17;

    // add or update location    
    if($loc_id = $golfObj->location_id) {
      $action = 'update';
      //list($loc_id, $loc_errors) = update_object('location', $locData, $loc_id);    
    } else {
      $action = 'add';
      list($loc_id, $loc_errors) = add_object('location', $locData);    
    }
    if($loc_errors) {echo("Failed to $action location".dump($loc_errors, true));}
  }

  // add or update golf course  
  if($loc_id) $data['location_id'] = $loc_id;
  
  if($golfObj) {
    list($id, $errors) = update_object('golf_course', $data, $golfObj->id);
    $action = "update";
  } else {
    list($id, $errors) = add_object('golf_course', $data);
    $action = "add";
  }

  if($errors) {echo("Failed to $action golf_course".dump($errors, true));}
  if($id && $loc_id && !$golfObj->location_id) db_query("UPDATE location SET parent_id=$id WHERE id=$loc_id");
  
  return array($id, $errors);
}

function gsAddCourseInfo($course) {
  
  $keys = array(
    
    // missing      
    //'dist' => 'length_men',
    //'lastUpd' => 'source_updated',
    //'img' => 'image',
  
    // existing
    'id' => 'fid',
    'onReq' => 'on_request',
    'nm' => 'name',
    'lat' => 'location_lat',             
    'lon' => 'location_long',    
    'cou' => 'address_country',
    'st' => 'address_state',
    'cty' => 'address_city',
    'advDays' => 'adv_days',
    'insideDays' => 'inside_days',
    'promo' => 'description',
    'rating' => 'rating_men',

    // new
    'addr1' => 'address_street',
    'zip' => 'address_postcode',
    'curr' => 'currency',
    'shortPromo' => 'short_description',
    'holes' => 'number_holes',
    'designer' => 'architect',

    // add
    'greens' => 'greens',
    'note' => 'notes',
    'ratingCnt' => 'rating_count',
    'ccAllow' => 'allowed_cc',
    'allowPlayers' => 'allowed_players',
    'yearBuilt' => 'year_built',
    'dressCode' => 'dress_code',
    'services' => 'services',
    'paymentTerms' => 'payment_terms',
    'fromPrice' => 'price_from',
    'toPrice' => 'price_to');

  $data = array_copy($course, $keys);
  $fid = $data['fid'];
  $golfObj = find_object('golf_course', array('fid' => $fid));
  $id = $golfObj->id;
  $name = $golfObj->name;
  
  if(!$id) return array(0, array("Could not find golf course fid=$fid"));
  
  // cancellation
  $data['cancel_type'] = $course->CxlPolicy->type;
  $data['cancel_count'] = $course->CxlPolicy->qty;
  $data['cancel_desc'] = $course->CxlPolicy->desc;

  // images
  $images = $course->Imgs->img;
  if(!is_array($images)) $images = array($images);

  $image = $golfObj->image; // main image, make this first
  if(count($images) > 1 && $images[0] != $image) {
    if(($key = array_search($image, $images)) !== false) {
      unset($images[$key]);
      array_unshift($images, $image); // image is now first    
    }
  }
  $data['images'] = implode(',', $images);
  
  // yardage
  $scorecard = $course->ScoreCard;
  $score_keys = $scorecard ? array_keys((array) $scorecard) : array();
  if($score_keys) {
    $data['scorecard'] = json_encode($scorecard);
    //dump($data);
    //echo("<b>$name: </b>:".implode(',', $score_keys).html_break());
    if($yardage = $scorecard->Yardages) {
      //$score_keys = $scorecard ? array_keys((array) $scorecard) : array();
      //dump($yardage);
    }
    
    // par
    if($par = $scorecard->Pars) {
      //dump($par);
    }
    
    // handicap
    if($handicap = $scorecard->Handicap) {
      //dump($handicap);
    }
  } else {
    //echo("$name: no<br>");
  }

  //dump($id);
  //die("updating $id");
  $data['details_updated'] = now();
  list($id, $errors) = update_object('golf_course', $data, $id);
  return array($id, $errors);
  
}

/* objects, arrays

    'Imgs' => '',
        (
            'img' => array
                (
                    '0' => '',
                    '1' => '',
                    '2' => '',
                )
    
        )
    
    'CxlPolicy' => object
        (
            'qty' => '',
            'type' => '',
            'desc' => '',
        )
    
    'ScoreCard' => '',
        (
        )
    )

*/

// traverse area data, add/update database
function gsAddAreas($res) {
  $countries = $res->AreasResult->Countries->Country  ?: array();
  if(!is_array($countries)) $countries = array($countries);
  $data = $output = array();
  foreach($countries as $country) {
    list($country_id, $errors) = gsAddGolfCountry($country);
    if($country_id) $output[]= "Added country $country_id";
    if($errors) {$output[] = "Failed adding country:".dump($country, true).dump($errors, true);continue;}
    $regions = $country->Regions->Region ?: array();
    if(!is_array($regions)) $regions = array($regions);
    foreach($regions as $region) {
      list($region_id, $errors) = gsAddGolfRegion($country_id, $region);
      if($region_id) $output[]= " - Added region $region_id";
      if($errors) {$output[] = "Failed adding region:".dump($region, true).dump($country,true).dump($errors, true);continue;}
      $districts = $region->Areas->Area ?: array();
      if(!is_array($districts)) $districts = array($districts);
      if(!$districts) {
        dump($districts);
        dump($region);die("No districts");
      }
      foreach($districts as $district) {
        list($district_id, $errors) = gsAddGolfDistrict($region_id, $district);
        if($district_id) $output[] = " -- Added district $district_id";
        if($errors) {$output[] = "Failed adding district:".dump($district, true).dump($region, true).dump($errors, true);continue;}
      }
    }
  }

  return implode(html_break(), $output);
}

function gsAddGolfCountry($country) {
  $data = array();        
  $obj_type = 'golf_country';
  $data['code'] = $code = $country->id;
  $data['name'] = $country_name = $country->nm;
  if($id = find_object($obj_type, $data, 'id')) {
    $output[]= "Found $obj_type $id";
  } else {
    list($id, $errors) = add_object($obj_type, $data);
  }
  return array($id, $errors);
}

function gsAddGolfRegion($country_id, $region) {
  $data = array();
  $obj_type = 'golf_region';
  $countryObj = get_object('golf_country', $country_id);
  if(!$countryObj) return array(0, array("Could not find country $country_id"));
  $country_code = $countryObj->code;
  $data['code'] = $region_code = $region->id;
  $data['golf_country_id'] = $country_id;                      
  if($id = find_object($obj_type, $data, 'id')) {
    $output[]= " - Found $obj_type $id";
  } else {
    $data['name'] = $region_name = $region->nm;
    if(in_array($country_code, array('CA', 'MX', 'USA', 'PR'))) {
      $cc = $country_code == 'USA' ? 'US' : $country_code; 
      $country_name = $countryObj->name;
      $address = "$region_name, $country_name";
    } else {
      $cc = $region_code; 
      $address = $region_name;
    }
    
    $data['country'] = $cc;
    $data['address'] = $address;
    $data['golf_country_id'] = $country_id;
    list($id, $errors) = add_object($obj_type, $data);
  }
  return array($id, $errors);
  
}

function gsAddGolfDistrict($region_id, $district) {
  $data = array();        
  $obj_type = 'golf_district';
  $regionObj = get_object('golf_region', $region_id);
  if(!$regionObj) return array(0, array("Could not find region $region_id"));
  $region_code = $regionObj->code;
  $region_name = $regionObj->name;
  $data['code'] = $district->id;
  $data['golf_region_id'] = $region_id;
  if($id = find_object($obj_type, $data, 'id')) {
    $output[]= " -- Found $obj_type $id";
  } else {
    $data['name'] = $district_name = $district->nm;
    $data['country'] = $regionObj->country;
    $data['golf_country_id'] = $regionObj->golf_country_id;
    $data['golf_region_id'] = $region_id;
    $data['address'] = "$district_name, ".$regionObj->address;
    list($id, $errors) = add_object($obj_type, $data);
  }
  return array($id, $errors);
}

// Helpers

// figure out our own IDs for Country, Region, District
// add if not found
function gsAreaIds($course, $savedArray) {
  $c_id = $r_id = $d_id = 0;
  $cCode = $course->sCou;
  $rCode = $course->sReg;
  $dCode = $course->sAr;
  if($cCode) {
    $c_id = $savedArray['country'][$cCode] ?: find_object('golf_country', array('code' => $cCode), 'id'); 
    if($rCode && $c_id) {
      $r_id = $savedArray['region'][$rCode] ?: find_object('golf_region', array('code' => $rCode, 'golf_country_id' => $c_id), 'id');
      //if(!$r_id) list($r_id, $errors) = gsAddGolfRegion($c_id, $region);
      if($dCode && $r_id) {
        $d_id = $savedArray['district'][$dCode] ?: find_object('golf_district', array('code' => $dCode, 'golf_region_id' => $r_id), 'id'); 
      }
    }
  }
  return array($c_id ?: 0, $r_id ?: 0, $d_id ?: 0);
}
  
?>

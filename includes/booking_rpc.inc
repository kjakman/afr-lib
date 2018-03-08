<?php
include_once("xmlrpc.inc");

/** from classes.inc */
$maps = array(
  'hotel' => array(
     'hotel_id' => 'guid',

     'address' => 'address_street',
     'zip' => 'address_postcode',
     'city' => 'address_city',
     'countrycode' => 'address_country',
     'location' => array('latitude' => 'location_lat', 'longitude' => 'location_long'),
     'checkin' => array('to' => 'checkin_to', 'from' => 'checkin_from'),
     'checkout' => array('to' => 'checkout_to', 'from' => 'checkout_from'),

     'nr_rooms' => 'num_rooms',
     'currencycode' => 'currency',

     'name' => 'name',

     'class' => 'rating',
     'maxrate' => 'rate_day',
     'minrate' => 'rate_day_min',
     'commission' => 'rental_commission',
     'ranking' => 'priority',
     'url' => 'source_url',
     'hoteltype_id' => 'hoteltype_id',
    ),
    

  'room' => array(
    'room_id' => 'guid',
    'ranking' => 'priority',
    'specification_required' =>'' ,
    'bookable_direct' =>'' ,
    'creditcard_required' =>'' ,
    'min_price' => 'min_price',
    'max_price' => 'max_price',
    'smoking_requested' => 'smoking_policy',
    'bookable_indirect' => 'bookable_indirect',
    'max_persons' => 'max_guests',
    'roomtype_id' => 'type',
   )
);

function rpc_process_booking($input, $show_test=false) {

  $xmlvalarray['show_test'] = new xmlrpcval($show_test, "int");
  $xmlvalarray['test_mode'] = new xmlrpcval($input['test_mode'], "int");

  $xmlvalarray['affiliate_id'] = new xmlrpcval($input['affiliate_id'], "int");
  $xmlvalarray['affiliate_label'] = new xmlrpcval($input['affiliate_label'], "string");

  $xmlvalarray['hotel_id'] = new xmlrpcval($input['bid'], "int");
  $xmlvalarray['begin_date'] = new xmlrpcval($input['in'], "dateTime.iso8601");
  $xmlvalarray['end_date'] = new xmlrpcval($input['out'], "dateTime.iso8601");

  // credit card
  $xmlvalarray['cc_cardholder'] = new xmlrpcval($input['cc_cardholder'], "string");
  $xmlvalarray['cc_number'] = new xmlrpcval($input['cc_number'], "string");
  $xmlvalarray['cc_expiration_date'] = new xmlrpcval($input['cc_exp_year'].'-'.$input['cc_exp_month'].'-01', "dateTime.iso8601");
  $xmlvalarray['cc_cvc'] = new xmlrpcval($input['cc_cvv2'], "int");
  $xmlvalarray['cc_type'] = new xmlrpcval($input['cc_type'], "int");

  // {dc_issue_number}
  // {dc_start_date}

  $state_zip  = $input['zip'];
  if($input['state']) $state_zip  = $input['state']." $state_zip";
  $xmlvalarray['guest_language' ] = new xmlrpcval($input['language'], "string");
  $xmlvalarray['guest_email'    ] = new xmlrpcval($input['email_address'], "string");
  $xmlvalarray['guest_street'   ] = new xmlrpcval($input['address1' ], "string");
  $xmlvalarray['guest_zip'      ] = new xmlrpcval($state_zip, "string"); // Booking doesn't use state field
  $xmlvalarray['guest_city'     ] = new xmlrpcval($input['city'     ], "string");
  $xmlvalarray['guest_country'  ] = new xmlrpcval($input['country'  ], "string");
  $xmlvalarray['guest_telephone'] = new xmlrpcval($input['phone'],    "string");
  $xmlvalarray['comments']        = new xmlrpcval($input['extra_info'],"string");


  // build RPC blocks
  $i=0;
  $ublocks = array(); // used to store unique ID + qty
  $blocks = array();  // used to store all other block info

  // add booker as first name
  $blocks['guest_names'][] = $input['first_name']." ".$input['last_name']; // name of booker first
  $total = 0;
  $block_text_ar = array();
  foreach($input as $k => $v) {
    //if($i) break;
    if(strpos($k, '_room')===0) {
      $block_text_ar[] = $v;
      $room_index = substr($k, 6);
      list($block_id, $cur, $rate, $max, $name) = explode(':', $v);
      
      // used for rpc
      $prefix = "room:".$room_index;
      if($ublocks[$block_id]) { // this block id is already registered
        $ublocks[$block_id]['qty']++;
        $total += $ublocks[$block_id]['price'];
      } else {
        $ublocks[$block_id]['qty'] = 1;
        $ublocks[$block_id]['price'] = $rate;
        $total += $rate;
      }
      $blocks['guest_names'][] = $input["$prefix:guest_name"];
      $blocks['guest_qtys'][]  = $input["$prefix:guest_qty"];
      $blocks['smoking'][]     = $input["$prefix:smoking"];
      //$blocks['pricing'][]     =  $rate;
      $i++;
      $num_guests += $input["$prefix:guest_qty"];
    }
  }
  foreach($ublocks as $ublock_id => $ublock) {
    $blocks['ids'][] =  $ublock_id;
    $blocks['qtys'][] =  $ublock['qty'];
    $blocks['pricing'][] =  $ublock['price'];
  }
  
  // dump($ublocks);
  // dump($input);
  // dump($blocks);
  // die();  
  $xmlvalarray['block_id']            = new xmlrpcval($blocks['ids'],         "array");
  $xmlvalarray['block_qty']           = new xmlrpcval($blocks['qtys'],        "array");
  $xmlvalarray['guest_name']          = new xmlrpcval($blocks['guest_names'], "array");
  $xmlvalarray['guest_qty']           = new xmlrpcval($blocks['guest_qtys'],  "array");
  $xmlvalarray['smoking']             = new xmlrpcval($blocks['smoking'],     "array");
  $xmlvalarray['incremental_prices']  = new xmlrpcval($blocks['pricing'],     "array");

  $struct = rpc_get_struct($xmlvalarray);
  $result = &xmlrpc('bookings.processBooking', $struct, 443);
  $result['num_guests'] = $num_guests;
  $result['currency'] = $cur;
  $result['total'] = $total;
  $result['block_text'] = implode('<BR />', $block_text_ar);
  
  return $result;

}

function rpc_get_city_availability($city_id, $checkin, $checkout, $show_test=false) {
  if(!$city_id)  die("rpc_get_city_availability: missing city id");
  if(!$checkin)  die("rpc_get_city_availability: missing checkin");
  if(!$checkout) die("rpc_get_city_availability: missing checkout");

  $booking_city_id = get_object_field('city', $city_id, 'booking_city_id');
  $xmlvalarray['show_test'] = new xmlrpcval($show_test, "int");
  $xmlvalarray['city_ids'] = new xmlrpcval($booking_city_id, "int");
  $xmlvalarray['arrival_date'] = new xmlrpcval($checkin, "dateTime.iso8601");
  $xmlvalarray['departure_date'] = new xmlrpcval($checkout, "dateTime.iso8601");
  $struct = rpc_get_struct($xmlvalarray);
  return rpc_get_availability('bookings.getHotelAvailability', $struct);
}

// hotel id can be single value or array of Booking Hotel IDS
function rpc_get_hotel_availability($hotel_ids, $checkin, $checkout, $show_test=false) {
  if(!$hotel_ids) die("rpc_get_hotel_availability: missing hotel ids");
  if(!$checkin)   die("rpc_get_hotel_availability: missing checkin");
  if(!$checkout)  die("rpc_get_hotel_availability: missing checkout");
  //dump($hotel_ids);
  $xmlvalarray['show_test'] = new xmlrpcval($show_test, "int");
  $xmlvalarray['hotel_ids'] = is_array($hotel_ids) ? new xmlrpcval($hotel_ids, "array") : new xmlrpcval($hotel_ids, "string");
  $xmlvalarray['arrival_date'] = new xmlrpcval($checkin, "dateTime.iso8601");
  $xmlvalarray['departure_date'] = new xmlrpcval($checkout, "dateTime.iso8601");
  $struct = rpc_get_struct($xmlvalarray);
  $avail = rpc_get_availability('bookings.getHotelAvailability', $struct);
  //dump($avail);
  return $avail;

}

// hotel id can be single value or array of Booking Hotel IDS
function rpc_get_block_availability($hotel_ids, $checkin, $checkout, $language='en', $show_test=false) {

  if(!$hotel_ids) die("rpc_get_block_availability: missing hotel ids");
  if(!$checkin) die("rpc_get_block_availability: missing checkin");
  if(!$checkout) die("rpc_get_block_availability: missing checkout");

  $single = false;
  if(is_array($hotel_ids)) { // several hotels
    $xmlvalarray['hotel_ids'] =  new xmlrpcval($hotel_ids, "array");
    $single = count($hotel_ids) == 1;
  } else { // one hotel - get details
    $xmlvalarray['hotel_ids'] = new xmlrpcval($hotel_ids, "string");
    $single=true;
  }

  $xmlvalarray['show_test'] = new xmlrpcval($show_test, "int");
  $xmlvalarray['arrival_date'] = new xmlrpcval($checkin, "dateTime.iso8601");
  $xmlvalarray['departure_date'] = new xmlrpcval($checkout, "dateTime.iso8601");
  $xmlvalarray['languagecode'] = new xmlrpcval($language, "string");
  if($single) $xmlvalarray['detail_level'] = new xmlrpcval(1, "int");

  $struct = rpc_get_struct($xmlvalarray);

  $blocks  = &xmlrpc('bookings.getBlockAvailability', $struct);
  //dump($blocks);
  if($blocks['error_code']) return $blocks; // error
  //if($single) dump($blocks);

  foreach($blocks as $block) {
    //dump($block);
    $booking_id = $block['hotel_id'];
    if($obj = find_object('hotel', array('source_id' => 23, 'guid' => $booking_id))) {
      $new_block[$obj->id] = $block;
    }
    $avail[$booking_id] = 1;
    $our_avail[$obj->id] = 1;
  }
  //dump($avail);
  //dump($our_avail);
  return $new_block;
}

// returns list (comma separated) of our IDS for hotels available
function rpc_get_availability($method, $struct) {
  $hotels = &xmlrpc($method, $struct);
//  dump($struct);
//  dump($hotels);
  if($hotels['error_code']) return $hotels; // error

  $i=0;
  $avail = array();

  foreach($hotels as $hotel) {
    //if(++$i > 2) break;
    $booking_id = $hotel['hotel_id'];
    if($obj = find_object('hotel', array('source_id' => 23, 'guid' => $booking_id))) {
      $rooms = $hotel['available_rooms'];
      //echo("Available: $obj->name: $rooms rooms<br>");
      $avail[] = $obj->id;
    }
  }
  return $avail;
}

//     [max_price] => 160
//     [min_total_price] => 280
//     [ranking] => 231
//     [hotel_id] => 10720
//     [max_total_price] => 640
//     [available_rooms] => 18
//     [min_price] => 70



function rpc_fetch_type($method, $obj_type, $id_field, $foreign_id_field='') {

  global $force_update;

  $xmlvalarray['languagecodes'] = new xmlrpcval('en', "string");

  $struct = rpc_get_struct($xmlvalarray);

  $type = &xmlrpc($method, $struct);
  $debug_output = array();
  $count = count($type);
  echo("<h3>Fetched $count members in type $type</h3>");

  foreach($type as $data) {
    $id = $data[$id_field];
    //if($data['bookable']) $debug_output[] = $id;
    if(!$id) {
      echo("Couldn' find id in data with key = [$id_field]<br>");
      dump($data);
      die();
    }

    if($foreign_id_field) {
      $obj = find_object($obj_type, array($foreign_id_field => $id));
    } else {
      $obj = get_object($obj_type, $id);
    }

    if($obj) { // update object
     //dump($obj);
     // die();
       //echo("found object $obj->id <br>");

      if($force_update) {
        update_object($obj_type, $data, $id);
        if($errors) dump($errors);
        else echo("Updated $obj_type $id<br>");
      }

    } else { // add object

      if($foreign_id_field) {
        $data[$foreign_id_field] = $id;
        list($nid, $errors) = add_object($obj_type, $data); // add normally (id = nextid)
        //echo("added object with dynamic id $nid<br>");
      } else {
        list($nid, $errors) = add_object($obj_type, $data, 0, $id); // add type with same ID as Booking
        //echo("added object with fixed id $id<br>");
      }

     if($errors) dump($errors);
     else echo("Added $obj_type $id<br>");
    }
    $max_len = max($max_len, strlen($data['name']));
  }
  $count = count($type);
  echo("<h3>$method ($obj_type): Processed $count items with max strlen $max_len</h3>");

  if(count($debug_output)) echo("Bookable:' ".implode(',',$debug_output)."'<br>");
}

function rpc_array2list($facs, $obj_type, $id_field, $foreign_id_field='') {
  //$facs = &xmlrpc($method, $struct);
  $facArray = array();

  foreach($facs as $fac) {
    //if(!$fac['value']) continue; // fac ooff
    $booking_fid = $fac[$id_field];

    if($foreign_id_field) {
      $obj = find_object($obj_type, array($foreign_id_field => $booking_fid));
    } else {
      $obj = get_object($obj_type, $booking_fid );
    }

    if($obj) {
      $our_id = $obj->id;
      $our_name = $obj->name;
      //echo("Found $obj_type booking_id=$booking_fid  our_id=$our_id ($our_name)<br>");
      $facArray[] = $our_id;
    } else {
      echo("<B>Warning: </b>Did not find $obj_type booking_id=$booking_fid<br>");
    }
  }
  $fac_str = array2sorted_unique_list($facArray);
  //echo("<h3>List $obj_type length = ".strlen($fac_str)."</h3>");
  return $fac_str;
}

function rpc_fetch_list($method, $struct, $obj_type, $id_field, $foreign_id_field='') {
  $facs = &xmlrpc($method, $struct);
  return rpc_array2list($facs, $obj_type, $id_field, $foreign_id_field);
}

// fetch one hotel or all in array of booking IDs
function rpc_fetch_hotels($booking_hotel_ids, $show_test = false) {
  global $force_update;
  global $langs;

  if(!$booking_hotel_ids) die("No booking hotel id");
  
  $xmlvalarray['supports_policies'] = new xmlrpcval(1, "int");  // removes obsolete descriptions
  $xmlvalarray['languagecodes'] = is_array($langs) ? new xmlrpcval($langs, "array") : new xmlrpcval($langs, "string");
  $xmlvalarray['hotel_ids'] = is_array($booking_hotel_ids) ? new xmlrpcval($booking_hotel_ids, "array") : new xmlrpcval($booking_hotel_ids, "string");
  $xmlvalarray['show_test'] = new xmlrpcval($show_test, "int");
  $struct = rpc_get_struct($xmlvalarray);

  $hotels = &xmlrpc('bookings.getHotels', $struct);

  if($hotels['error_code']) {
    dump($hotels);
  } else {  
    process_hotels($hotels, $booking_hotel_ids,0,$show_test);
  }
}


// fetch all hotels in given city
function rpc_fetch_city_hotels($cityObj, $limit, $show_test = false) {

  $city_id= $cityObj->id;
  $city_name = $cityObj->name;
  $booking_city_id = $cityObj->booking_city_id;
  $country = $cityObj->countrycode2;

  $i=0;

  global $test;
  global $hotels_short;
  global $fetch_photos;
  global $force_update;
  global $langs;

  // get hotels
  if($test) {
    echo("<h2>Using cached hotels</h2>");
    $all_hotels = $hotels_short;
  } else {
    // echo("Fetching hotels for $city_name ($city_id) Booking ID: $booking_city_id<br>");
    if(!$booking_city_id) die("No booking city id found for $city_name");
    $xmlvalarray['city_ids'] = new xmlrpcval($booking_city_id, "int");
    $xmlvalarray['supports_policies'] = new xmlrpcval(1, "int");  // removes obsolete descriptions
    $xmlvalarray['languagecodes'] = is_array($langs) ? new xmlrpcval($langs, "array") : new xmlrpcval($langs, "string");
    $xmlvalarray['show_test'] = new xmlrpcval($show_test, "int");
    $struct = rpc_get_struct($xmlvalarray);

    //$city_struct = rpc_get_struct($xmlvalarray);

    $all_hotels = &xmlrpc('bookings.getHotels', $struct);
  }

  $num_fetched = 0;
  $num_left = $count = count($all_hotels);
  $num_to_fetch = $n = min($count, $limit);

  while($num_left) {

    $hotels_slice = array_slice($all_hotels, $num_fetched, $num_to_fetch);
    $hotel_ids = &array_find_key($hotels_slice, 'hotel_id'); // returns array of hotel_ids
    $hotel_ids_str = implode(',', $hotel_ids);
    echo("<h2>Fetched $num_fetched / $count hotels in $city_name, $country - Num Left: $num_left<br>Now fetching $num_to_fetch ($hotel_ids_str)</h2>");
    
    process_hotels($hotels_slice, $hotel_ids, $city_id, $show_test);
       
    $num_fetched += $num_to_fetch;
    $num_left = $count - $num_fetched;
    $num_to_fetch = min($num_left, $limit);
  }
  echo("<h2>Done - fetched $i hotels</h2>");

  //$h_trans = &xmlrpc('bookings.getHotelTranslations',  $hotel_id_struct);

}

function process_hotels($hotels_slice, $hotel_ids, $city_id=0, $show_test=false) {

  global $fetch_photos;
  global $force_update;
  global $langs;
  
  $verbose = true;

  //dump($hotels_slice);echo(count($hotels_slice));die();

  // new struct - limit to N hotels
  $xmlvalarray['supports_policies'] = new xmlrpcval(1, "int");  // removes obsolete descriptions
  $xmlvalarray['languagecodes'] = is_array($langs) ? new xmlrpcval($langs, "array") : new xmlrpcval($langs, "string");
  $xmlvalarray['show_test'] = new xmlrpcval($show_test, "int");
  $xmlvalarray['hotel_ids'] = is_array($hotel_ids) ? new xmlrpcval($hotel_ids, "array") : new xmlrpcval($hotel_ids, "string");
  $count = is_array($hotel_ids) ? count($hotel_ids) : 1;
  
  $sub_struct = rpc_get_struct($xmlvalarray);

  // now get all facs, chains, cards for the city - process later
  $all_cards  = &xmlrpc('bookings.getCreditcards', $sub_struct);
  $all_facs   = &xmlrpc('bookings.getHotelFacilities', $sub_struct);
  $all_chains = &xmlrpc('bookings.getChains', $sub_struct);

  echo("<h3>Fetched ".count($all_cards)." cards for $count hotels</h3>");
  echo("<h3>Fetched ".count($all_facs)." facs for $count hotels</h3>");
  echo("<h3>Fetched ".count($all_chains)." chains for $count hotels</h3>");

  if($fetch_photos) {
    // hotel
    $all_photos =      $xml_photos['photos']        = &xmlrpc('bookings.getHotelPhotos',                  $sub_struct);
    $all_desc_photos = $xml_photos['desc_photos']   = &xmlrpc('bookings.getHotelDescriptionPhotos',       $sub_struct);
    $all_logo_photos = $xml_photos['logo_photos']   = &xmlrpc('bookings.getHotelLogoPhotos',              $sub_struct);

    echo("<h3>Fetched ".count($all_photos)." photos for $count hotels<br>");
    echo("<h3>Fetched ".count($all_desc_photos)." desc photos for $count hotels</h3>");
    echo("<h3>Fetched ".count($all_logo_photos)." logo photos for $count hotels</h3>");

    //dump($all_photos);
  }

  foreach($hotels_slice as $hotel) {
    //if($limit && $i++ > $limit) {echo("<h3>Fetched $limit - breaking out</h3>");break;}

    $name = $hotel['name'];
    $bid = $hotel['hotel_id'];
    echo("processing $name ($bid)<br>");

    if(!$show_test && $hotel['is_closed']) {
      echo("$name is closed<br>");
      continue; // skip if closed
    }
    $data = map_array($hotel, get_map('hotel'), false, true); // passthru=false, addslashes = true

    // look up our city id unless passed in as param
    if(!$city_id) {    
      $booking_city_id = $hotel['city_id'];
      $city_id = $city_ids[$booking_city_id]; // look up in array of stored ids - don't do DB look up on every loop
      if(!$city_id) {    
        $our_city_obj = find_object('city', array('booking_city_id' => $booking_city_id)); // search in DB      
        if($our_city_obj) {
          $city_id = $city_ids[$booking_city_id] = $our_city_obj->id; // store for later
        } else {
          dump($hotel);
          die("Couldn't find city with booking_city_id = $booking_city_id");
        }
      }
    }
      
    $data['city_id'] = $city_id;
    $data['input_lang'] = 'en';
    $data['source_id'] = 23;
    $data['owner_id'] = 166; // dummy - todo: use chain if exists
    $data['active'] = 1;
    $data['location_accuracy'] = $data['location_long'] ? 8 : 0; // set accuracy if coordinates are present

    $hotel_chains = &array_find($all_chains, 'hotel_id', $bid);
    $hotel_facs   = &array_find($all_facs, 'hotel_id', $bid);
    $hotel_cards  = &array_find($all_cards, 'hotel_id', $bid);

    $data['chains']         = $chains = rpc_array2list($hotel_chains, 'hotel_chain', 'chain_id');
    $data['facilities']     = $facs =   rpc_array2list($hotel_facs, 'hotel_facility', 'hotelfacilitytype_id', 'booking_id');
    $data['cards_accepted'] = $cards =  rpc_array2list($hotel_cards, 'cc_type', 'creditcard_id');

    if($verbose) {
      echo(" - Hotel $bid has ".count($hotel_cards)." cards: $cards<br>");
      echo(" - Hotel $bid has ".count($hotel_facs)." facs: $facs<br>");
      echo(" - Hotel $bid has ".count($hotel_chains)." chains: $chains<br>");
    }

    $id = rpc_save_item('hotel', $data, $bid, 'guid');
    $data['id'] = $id;

    $list_data = array();

    if($fetch_photos) {
      $xml_photos['photos']        = &array_find($all_photos, 'hotel_id', $bid);
      $xml_photos['desc_photos']   = &array_find($all_desc_photos, 'hotel_id', $bid);
      $xml_photos['logo_photos']   = &array_find($all_logo_photos, 'hotel_id', $bid);

      if($verbose) {
        echo(" - Hotel $bid has ".count($xml_photos['photos']     )." first photos<br>");
        echo(" - Hotel $bid has ".count($xml_photos['desc_photos'])." desc photos<br>");
        echo(" - Hotel $bid has ".count($xml_photos['logo_photos'])." logo photos<br>");
      }

      rpc_fetch_photos($data, $bid, $xml_photos);
    }
    $i++;
  } // end foreach hotel

  // now fetch all rooms, descriptions for city;
  rpc_fetch_rooms($sub_struct);
  rpc_fetch_hotel_descriptions($sub_struct);
}


function rpc_find_hotel($booking_hotel_id) {
  if($obj = find_object('hotel', array('source_id' => 23, 'guid' => $booking_hotel_id))) {
    return $obj->id;
  } else {
    die("rpc_find_hotel: Couldn't find Booking hotel with GUID=$booking_hotel_id");
    return 0;
  }
}

function rpc_find_room($booking_room_id) {
  if($obj = find_object('room', array('owner_id' => 166, 'guid' => $booking_room_id))) {
    return $obj->id;
  } else {
    die("rpc_find_room: Couldn't find Booking room with GUID=$booking_room_id");
    return 0;
  }
}

function rpc_fetch_rooms($struct) {
  global $fetch_photos;
  global $langs;
  $rooms = &xmlrpc('bookings.getRooms', $struct);
  echo("<h3>Fetched ".count($rooms)." rooms</h3>");

  // get all room facs
  $all_room_facs   = &xmlrpc('bookings.getRoomFacilities', $struct);
  echo("<h3>Fetched ".count($all_room_facs)." room facs</h3>");

  if($fetch_photos) {
    $all_room_photos = &xmlrpc('bookings.getRoomPhotos', $struct);
    echo("<h3>Fetched ".count($all_room_photos)." room photos</h3>");
  }

  $j=0;
  foreach($rooms as $room) {
    $booking_room_id = $room['room_id'];
    //echo("processing room $booking_room_id <br>");

    $booking_hotel_id = $room['hotel_id'];
    $hotel_id = rpc_find_hotel($booking_hotel_id);

    $data = map_array($room, get_map('room'), false, true); // passthru=false, addslashes = true
    $data['parent_id'] = $hotel_id;
    $data['owner_id'] =  166; // dummy - todo: use chain if exists
    $data['active'] = 1;

    $xmlvalarray['room_ids'] = new xmlrpcval($booking_room_id, "int");
    $room_id_struct = rpc_get_struct($xmlvalarray);

    $room_facs   = &array_find($all_room_facs, 'room_id', $booking_room_id);
    $data['facilities'] = rpc_array2list($room_facs, 'room_facility', 'roomfacilitytype_id', 'booking_id');

    if(!$data['type']) $data['type'] = 20; // set room type to 20 = unkwown if missing 
    
    $id = rpc_save_item('room', $data, $booking_room_id, 'guid');
    $data['id'] = $id;

    //echo("booking hotel_id=$booking_hotel_id, room_id=$booking_room_id<br>");


    if($fetch_photos) {
      $room_photos = &array_find($all_room_photos, 'room_id', $booking_room_id);
      if($verbose) echo(" - - Room $booking_room_id has ".count($room_photos)." photos<br>");
      if(count($room_photos)) {
        $aptObj = get_object('hotel', $hotel_id);
        $apt = obj2array($aptObj);
        rpc_fetch_room_photos($id, $apt, $room_photos);
      }
    }
    $j++;
  } // end foreach room

  rpc_fetch_room_description($struct);

  echo("<h3>Done - processed $j rooms</h3>");

}

function rpc_fetch_room_description($struct) {
  // $xmlvalarray['room_ids'] = new xmlrpcval($booking_room_id, "int");
  // $xmlvalarray['languagecodes'] = is_array($langs) ? new xmlrpcval($langs, "array") : new xmlrpcval($langs, "string");
  // $room_id_struct = rpc_get_struct($xmlvalarray);

  $descriptions = &xmlrpc('bookings.getRoomTranslations', $struct);
  $count = count($descriptions);
  echo("<h3>Fetched $count room descriptions</h3>");

  // process descriptions
  foreach($descriptions as $desc) {

    $descArray = array();

    $booking_hotel_id = $desc['hotel_id'];
    $hotel_id = rpc_find_hotel($booking_hotel_id);

    $booking_room_id = $desc['room_id'];
    $room_id = rpc_find_room($booking_room_id);

    $descArray['room_id'] = $room_id;
    $descArray['language'] = $desc['languagecode'];
    if($obj = find_object('room_description', $descArray)) $descArray['id'] = $obj->id;

    $descArray['property_id'] = $hotel_id;

    $did = rpc_save_item('room_description', $descArray);
  }

  //list($did, $errors) = add_or_update_object('room_description', $descArray);
  //if($errors) {dump($errors); die(); }

}

function rpc_fetch_hotel_descriptions($struct) {
  // $xmlvalarray['hotel_ids'] = new xmlrpcval($bid, "int");
  // $xmlvalarray['languagecodes'] = is_array($langs) ? new xmlrpcval($langs, "array") : new xmlrpcval($langs, "string");
  // $xmlvalarray['supports_policies'] = new xmlrpcval(1, "int");  // removes obsolete descriptions
  // $hotel_id_struct = rpc_get_struct($xmlvalarray);

  $descriptions = &xmlrpc('bookings.getHotelDescriptionTranslations', $struct);
  $count = count($descriptions);
  echo("<h3>Fetched $count descriptions</h3>");

  // process descriptions
  foreach($descriptions as $desc) {
    $booking_hotel_id = $desc['hotel_id'];
    $hotel_id = rpc_find_hotel($booking_hotel_id);

    $descArray = array();
    $descArray['property_id'] = $hotel_id;
    $descArray['property_type'] = 'hotel';
    $descArray['type'] = $desc['type'];
    $descArray['language'] = $desc['languagecode'];
    if($obj = find_object('property_description', $descArray)) $descArray['id'] = $obj->id;
    $did = rpc_save_item('property_description', $descArray);
  }
}

// used to save hotel or room
function rpc_save_item($obj_type, $data, $search_id=0, $id_field='id') {
  global $force_update;

  if($search_id) {
    $hObj = find_object($obj_type, array($id_field => $search_id));
    $id = $hObj->id;
  } else {
    $id=$data[$id_field];
    //if(!$id) die('save_item: missing id - and search id not set');
  }

  if($id) {
    if($force_update) {
      list($uid, $errors) = update_object($obj_type, $data, $id);
      $action = 'Update';
    } else {
      $action = 'Skip';
      echo("Skipping - $action $name ($id)<br>");
      //continue;
    }
  } else {
    $action = 'Add';
    list($id, $errors) = add_object($obj_type, $data);
  }

  if($errors) {
    echo("<h3>Problem with $action $obj_type</h3>");
    dump($data);
    dump($errors);
    die();
  }
  if($verbose) echo("<b> - $action $obj_type $name ($id)</b><br>");
  return $id;
}



function rpc_fetch_room_photos($room_id, $apt, $photos) {

  // hotel
  foreach($photos as $image) {
    $media = map_array($image, get_map('image'), false, true); // passthru=false, addslashes = true
    $media['descriptiontype_id'] = 2; // room information
    $media['room_id'] = $room_id;
    $imageArray[] = $media;
  }
  add_property_photos('hotel', $apt, $imageArray, $room_id);
}

function rpc_fetch_photos($apt, $bid, $xml_photos) {
  $id = $apt['id'];

  $photos =  $xml_photos['photos'];
  $desc_photos  = $xml_photos['desc_photos'];
  $logo_photos  = $xml_photos['logo_photos'];

  if(count($photos) > 1) echo("<h1> More than 1 photo (".count($photos).")for hotel $bid<br>");
  if(count($photos)) { // Add first photo
    $image = array_shift($photos);
    $media = map_array($image, get_map('image'), false, true); // passthru=false, addslashes = true
    $imageArray[] = $media;
  }

  foreach($desc_photos as $image) {
    $media = map_array($image, get_map('image'), false, true); // passthru=false, addslashes = true
    $imageArray[] = $media;
  }

  // process rest of photos
  $img_count = sizeof($imageArray);
  echo("  - Found $img_count pics for $bid ($id)<br>");

  $apt['type'] = 'hotel'; // used by add_property_photos to set type in media
  add_property_photos('hotel', $apt, $imageArray);

  // process logo
  if(count($logo_photos)) {
    $image = array_shift($logo_photos);
    $media = map_array($image, get_map('image'), false, true); // passthru=false, addslashes = true
    $media['parent_id'] = $id;
    $media['src'] = 'logo.jpg';
    $media['property_type'] = 'hotel';
    $media['source_id']       = $apt['source_id'];
    $media['city_id']         = $apt['city_id'];
    $media['estate_agent_id'] = $apt['estate_agent_id'];
    $media['owner_id']        = $apt['owner_id'];

    add_property_photo($media);

  }
}

function rpc_delete_hotels($city_id=0) {
  if($city_id) {
    db_query("DELETE FROM media WHERE property_type='hotel' AND city_id='$city_id'");
    db_query("DELETE FROM media WHERE room_id>0 AND city_id='$city_id'");
    db_query("DELETE FROM property WHERE type='hotel' AND city_id='$city_id'");
    db_query("DELETE FROM room");
    db_query("DELETE FROM room_description");
  } else {
    db_query("DELETE FROM media WHERE property_type='hotel'");
    db_query("DELETE FROM media WHERE room_id>0");
    db_query("DELETE FROM property WHERE type='hotel'");
    db_query("DELETE FROM room");
    db_query("DELETE FROM room_description");
  }
}

function rpc_fetch_types() {

  echo("fetching types<br>");
  rpc_fetch_type('bookings.getHotelDescriptionTypes','description_type', 'type');
  rpc_fetch_type('bookings.getFacilityTypes',      'facility_type', 'facilitytype_id');
  rpc_fetch_type('bookings.getChainTypes',         'hotel_chain', 'chain_id');
  rpc_fetch_type('bookings.getCreditcardTypes',    'cc_type', 'creditcard_id');
  rpc_fetch_type('bookings.getHotelTypes',         'hotel_type', 'hoteltype_id');
  rpc_fetch_type('bookings.getHotelFacilityTypes', 'hotel_facility', 'hotelfacilitytype_id', 'booking_id');
  rpc_fetch_type('bookings.getRoomFacilityTypes',  'room_facility', 'roomfacilitytype_id', 'booking_id');
}

// geo //

function rpc_get_struct($xmlvalarray) {
  global $test;
  if($test) $xmlvalarray['show_test'] = new xmlrpcval(1, "boolean");
  $struct=new xmlrpcval($xmlvalarray, "struct");
  return $struct;
}


function print_all_locations($cc='') {
  $countries = rpc_get_countries($cc);
  //dump($countries);
  // foreach($countries as $country) {
  //   $cc = $country['countrycode'];
  //   $area = $country['area'];
  //   $name = $country['name'];
  //   if($countryObj = find_object('country', array('code2' => $cc))) {
  //     $id = $countryObj->id;
  //     $our_name = $countryObj->name;      
  //     if($our_name == $name) {
  //       echo(" Found $name ($cc) = $id - Names match<br>");
  //     } else {
  //       echo("** Found $name ($cc) = $id - Names do not match ($name vs $our_name)<br>");
  //     }
  //   } else {
  //     echo("** DID NOT FIND $name ($cc)<br>");
  //   }
  //   $areas[$area]++;
  // }
  foreach($countries as $country) {
    $regions = rpc_get_regions($country['countrycode']);
    $cities = rpc_get_cities($country['countrycode']);
    $districts = rpc_get_districts($country['countrycode']);

    $nr = count($regions);
    $nc = count($cities);
    $nd = count($districts);
    echo("$country[name] has $nr regions, $nc cities, $nd districts<br>");
    
    // foreach($regions as $region) {
    //   //dump($region);
    //   $cities = rpc_get_cities($country['countrycode'], $region['id']);
    //   dump($cities);
    //   die();
    //   $nc = count($cities);
    //   echo("<b>$region[name] has $nc cities: </b>");
    // 
    //   foreach($cities as $city) {
    //     echo("$city[name] - ");
    //   }
    //   echo("<br>");
    // }
  }
}

function deactivate_childless_location($type) {
  global $db;
  $table = ms_table($type);
  $child_type = loctree_child($type);
  $child_table = ms_table($child_type);
  $objs = db_getCol("SELECT id FROM $table WHERE active=1");
  $num = db_getOne("SELECT count(id) FROM $table WHERE active=1");
  echo("BEFORE: $num active $type<br>");
  //db_query("update $table set active=1"); // set all active first
  foreach($objs as $pid) {
    $qs = "SELECT count(id) FROM $child_table WHERE parent_id=$pid AND active=1";
    $activeChildren = db_getOne($qs);
    if(!$activeChildren) {
      //echo("$type $pid has $activeChildren active children- deactivating<br>");
      db_query("UPDATE $table SET active=0 WHERE id=$pid");
    }
  }
  $num = db_getOne("SELECT count(id) FROM $table WHERE active=1");
  echo("AFTER: $num active $type<br><hr>");

}


//rpc_update_all();



//dump(rpc_get_country_details('us'));

//dump(rpc_get_regions());

// update_location_tree();
//  update_all_geonames();


// if a district has no active children, deactivate
// repeat for country, region,  continent
// rerun geoname
function update_location_tree() {
  global $db;
  rpc_update_all();
  db_query("update msCity set active=1 where booking_nrhotels>0");
  db_query("update msCity set active=0 where booking_nrhotels=0 and id != 4004");
  deactivate_childless_location('district');
  deactivate_childless_location('country');
  deactivate_childless_location('region');
  deactivate_childless_location('continent');
  update_all_geonames();
}

function update_all_geonames($lang='en') {
  update_geonames('city');
  update_geonames('district');
  update_geonames('country');
  update_geonames('region');
  update_geonames('continent');
}

function update_geonames($type, $lang='en') {
  global $db;
  $table = ms_table($type);
  $level = ms_index($type);
  $num = db_getOne("SELECT count(id) FROM msGeoname WHERE active=1 AND level=$level");
  echo("BEFORE: $num active $type in geonames<br>");
  $qs = "update $table, msGeoname set msGeoname.active=0 where $table.active=0 AND $table.id = msGeoname.fid AND msGeoname.level = $level";
  echo($qs."<br>");
  db_query($qs);
  $num = db_getOne("SELECT count(id) FROM msGeoname WHERE active=1 AND level=$level");
  echo("AFTER: $num active $type in geonames<br><hr><br>");
}



function rpc_get_countries($cc='', $lang='en') {
  $methodName = "bookings.getCountries";
  $xmlvalarray['languagecodes'] = new xmlrpcval($lang, "string");
  if($cc) $xmlvalarray['countrycodes'] = is_array($cc) ? new xmlrpcval($cc, "array") : new xmlrpcval(strtolower($cc), "string");
  $struct=new xmlrpcval($xmlvalarray, "struct");
  $arr = &xmlrpc($methodName, $struct);
  return $arr;
}

function rpc_get_regions($countrycode='', $lang='en') {
  $methodName = "bookings.getRegions";
  $xmlvalarray['languagecodes'] = new xmlrpcval($lang, "string");
  if($countrycode) $xmlvalarray['countrycodes'] = new xmlrpcval(strtolower($countrycode), "string");
  $struct=new xmlrpcval($xmlvalarray, "struct");
  $arr = &xmlrpc($methodName, $struct);
  return $arr;
}

function rpc_get_cities($countrycode='', $region='', $lang='en') {
  $methodName = "bookings.getCities";
  $xmlvalarray['languagecodes'] = new xmlrpcval($lang, "string");
  if($countrycode) $xmlvalarray['countrycodes'] = new xmlrpcval(strtolower($countrycode), "string");
  if($region) $xmlvalarray['region'] = new xmlrpcval($region, "int");
  $struct=new xmlrpcval($xmlvalarray, "struct");
  $arr = &xmlrpc($methodName, $struct);
  return $arr;
}

function rpc_get_districts($countrycode='', $city_id=0, $lang='en') {
  $methodName = "bookings.getDistricts";
  $xmlvalarray['languagecodes'] = new xmlrpcval($lang, "string");
  if($city_id) $xmlvalarray['city_ids'] = new xmlrpcval(strtolower($city_id), "string");
  if($countrycode) $xmlvalarray['countrycodes'] = new xmlrpcval(strtolower($countrycode), "string");
  $struct=new xmlrpcval($xmlvalarray, "struct");
  $arr = &xmlrpc($methodName, $struct);
  return $arr;
}

function rpc_get_country_details($countrycode, $lang='en') {
  $methodName = "bookings.getCountryDetails";
  $xmlvalarray['languagecodes'] = new xmlrpcval($lang, "string");
  $xmlvalarray['countrycodes'] = new xmlrpcval(strtolower($countrycode), "string");
  $struct=new xmlrpcval($xmlvalarray, "struct");
  $arr = &xmlrpc($methodName, $struct);
  return $arr;
}

function rpc_update_all($lang='en') {
  $countries = rpc_get_countries();
  foreach($countries as $country) {
    echo("<h2>Running update_cities $country[name] for lang $country[languagecode]</h2>");
    rpc_update_cities($country['countrycode'], $country['languagecode']);
  }
}

// country is 2 letter lowercase ISO code
function rpc_update_cities($country, $region, $lang='en') {
  global $db;
  $arr = rpc_get_cities($country);
  $n = count($arr);
  echo("Found $n cities in $country<br>");

  foreach($arr as $city) {
    $name = $city['name'];
    $city_id = $city['city_id'];
    $nr = $city['nr_hotels'];
    $cc = strtoupper($city['countrycode']);

    $id = db_getOne("SELECT id FROM msCity WHERE name=\"$name\" AND countrycode2='$cc'");
    if($id) {
      db_query("UPDATE msCity SET booking_nrhotels='$nr', booking_city_id='$city_id' WHERE id=$id");
    } else {
      echo("&nbsp;&nbsp;$country: $name: no match: $nr hotels<br>");
      //dump($city);
    }
    $bid = db_getOne("SELECT count(*) FROM booking_city WHERE id='$city_id'");
    if($bid) {
      db_query("UPDATE booking_city SET nr_hotels = '$nr', city_id='$id' WHERE id=$city_id");
    } else {
      db_query("INSERT INTO booking_city (id, countrycode, name, nr_hotels, city_id) VALUES ('$city_id', '$country', \"$name\", '$nr', '$id')");
    }
  }

}

// get room details
$methodName = "bookings.getRoomDetails";
$hotel_id = "19500";
$struct=new xmlrpcval(
    array(
      "hotel_id" => new xmlrpcval($hotel_id, "int"),
      "languagecode" => new xmlrpcval($language, "string")
    ), "struct");



// get districts
$methodName = "bookings.getDistricts";
$struct=new xmlrpcval(array(
    "countrycodes" => new xmlrpcval('us', "string"),
    "languagecodes" => new xmlrpcval('en', "string")
    ), "struct");

// get getCountryDetails
$methodName = "bookings.getCountryDetails";
$struct=new xmlrpcval(array(
    "countrycodes" => new xmlrpcval('us', "string"),
    "languagecodes" => new xmlrpcval('en', "string")
    ), "struct");




function &xmlrpc($methodName, $struct, $port=80) {
  global $rpc_test;
  $username = "aafr";
  $password = "f0rr4nt";
  $timestamp = $rpc_test ? "test" : date("Y-m-d_His");
  $target = $methodName;
  $xml_target_file = $_SERVER['DOCUMENT_ROOT']."upload/xml-rpc/".$target."_".$timestamp.".xml";

  //echo("Writing to $xml_target_file<br>");
  //init xml-rpc message
  $f=new xmlrpcmsg($methodName);
  if($struct) $result = $f->addParam($struct);
  //echo("addParam result=$result<br>");

  $method = $port == 443 ? 'https' : '';

  $c=new xmlrpc_client("/xml-rpc", "distribution-xml.booking.com", $port, $method);

  $c->setCredentials($username, $password);

  //print xml-rpc request
  //print "<PRE>" . htmlentities($struct->serialize()) . "</PRE>";

  //send msg
  $resp=$c->send($f);

  // log msg
  write_file($xml_target_file,$resp->serialize());

  $xmlrpcVal=$resp->value();

  if(!$xmlrpcVal) { // Error
    $response_str = $resp->serialize();
    if($_SESSION['dev']) {
      //dump($struct);
      print "<h2>Response from method $methodName</h2><textarea rows='10' cols='100'>" . $response_str . "</textarea>";
    }
    $error_code = find_delimited_str($response_str, '<int>', '</int>');
    $error_str = find_delimited_str($response_str, '<string>', '</string>');
    $error_str = trim_leading($error_str, "$methodName: ");

    //echo("<br>serialize result:<br> full='$response_str'<br>error_code = '$error_code'<br>error_str = '$error_str'<br>");
    $response['error_code'] = $error_code; // todo , parse real ID
    if($errors = explode(';', $error_str)) { // more than 1 error
      foreach($errors as &$error) $error = trim(trim_leading($error, "error: "));  
      $response['error_str'] = $error; // todo , parse real ID
    } else {
      $response['error_str'] = $error_str; // todo , parse real ID
    }
  } else { // OK 
    $response =php_xmlrpc_decode($xmlrpcVal);
  }
  return $response;
}
?>
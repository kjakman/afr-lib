<?php

function google_timezone($lat, $lng, $ts='') {
  if(!$ts) $ts = time();
  $url = "https://maps.googleapis.com/maps/api/timezone/json?location={$lat},{$lng}&timestamp={$ts}&key=YOUR_API_KEY";
  $url = "https://maps.googleapis.com/maps/api/timezone/json?location={$lat},{$lng}&timestamp={$ts}";
  $json = get_html($url);
  $result = json_decode($json);
  
  if($full = $options['full']) return $result;

  return $result;
  
  $status = $result->status;
  $results = $result->results;
  $last = array_pop($results);
  return $status == 'OK' ? parse_geometry($last->geometry) : array();

}

function google_geocode($address, $options=array()) {
  $q=urlencode($address);
  if(isset($options['sensor'])) {
    $sensor = $options['sensor'] ? 'true' : 'false';
  } else {
    $sensor = 'false';
  }
  $url = "https://maps.googleapis.com/maps/api/geocode/json?address=$q&sensor=$sensor";
  $json = get_html($url);
  $result = json_decode($json);
  print_log("Geocode result: ".dump($result, true), 'google', LOG_LEVEL_TEST);
  
  if($full = $options['full']) return $result;

  $status = $result->status;
  $results = $result->results;
  $last = array_pop($results);
  return $status == 'OK' ? parse_geometry($last->geometry) : array();
}

function google_parse_address($addressAr) {
  $address = [];
  foreach($addressAr as $addObj) {
    if (in_array('street_number', $addObj->types)) $address['address_number'] = $addObj->short_name;
    else if (in_array('route', $addObj->types)) $address['address_street'] = $addObj->short_name;
    else if (in_array('locality', $addObj->types)) $address['address_city'] = $addObj->short_name;
    else if (in_array('administrative_area_level_1', $addObj->types)) $address['address_state'] = $addObj->short_name;
    else if (in_array('postal_code', $addObj->types)) $address['address_postcode'] = $addObj->short_name;
    else if (in_array('country', $addObj->types)) $address['address_country'] = $addObj->short_name;
  }
  return $address;
}

function parse_geometry($geo) {
  $lat = $geo->location->lat;
  $lng = $geo->location->lng;
  $bounds = $geo->bounds ?: $geo->viewport;
  $bounds_json = $bounds ? json_encode($bounds) : '';
  return array(200, $geo->location_type, $lat, $lng, $bounds_json);
}

// move to helpers_twitter.inc
/** unsused - automatic comment added on  */
function twitter_script($options=NULL) {            
  return'<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0],p=/^http:/.test(d.location)?"http":"https";if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src=p+"://platform.twitter.com/widgets.js";fjs.parentNode.insertBefore(js,fjs);}}(document, "script", "twitter-wjs");</script>';
}

/** unsused - automatic comment added on  */
function twitter_share_button($tweetData, $options=NULL) {  
  if(is_array($tweetData)) foreach($tweetData as $key => $value) $options['data-'.$key] = $value;
  $options['class'] .= ' twitter-share-button';
  $name = isset($options['name']) ? $options['name'] : "Tweet";
  $output = html_link("https://twitter.com/share", $name, $options);
  return $output;
}


/** unsused - automatic comment added on  */
function twitter_tweet($url, $tweet, $options=NULL) {
  if(!$tweet) return '';
  global $__APP;
  $_APP->header->scripts[] = "https://platform.twitter.com/widgets.js";    
  $tweet = rawurlencode($tweet);
  $url = rawurlencode(add_http($url));
  $button = "<a href=\"http://twitter.com/share?text=$tweet&url=$url&related=$related\" class=\"twitter-share-button\" data-count=\"none\" target=\"_blank\" rel=\"nofollow\">Tweet</a>";
  return $button;
}


// format can be 'text' or 'html'
// max 2K for get, 5K for POST
/** unsused - automatic comment added on  */
function google_translate($term, $to_lang, $from_lang= '', $options=NULL) {
  $org_term = $term;
  $term = trim($term);
  if(!$term) return array('', array("google_translate: missing term for term=$term org='$org_term'"));
  if($options['html']) { // html  
    $method = 'html';
    $fields['q'] = $q = preg_replace('!\s+!', ' ', $term);
    $len = strlen(strip_tags($q));
  } else { // text - default 
    $method = 'text';
    $fields['q'] = $q = strip_tags($term); // remove tags
    $len = strlen($q);
  }
  if(!$len) return array('', array("google_translate: missing len for term=$term org='$org_term'"));
  if(!$to_lang) return array('', array("google_translate: missing to_lang for term=$term"));
  $max = $options['get'] ? 2000 : 5000;
  if($len > $max) {
    return array('', array("google_translate: input too long $len > $max"));
  }
  
  if($from_lang) $fields['source'] = $from_lang;
  $fields['target'] = $to_lang;
  $fields['format'] = $options['format'] ?: 'text'; // text or html
  list($text, $errors) = google_api_post('translate', $fields, $options);
  //echo("Google: t='$term' q='$q' result='$text' m=$method<br>");
  //die();
  return array($text, $errors);
}    

// detect language in a string
/** unsused - automatic comment added on  */
function google_detect($term, $full_result=false) {
  $fields['q'] = str_replace('  ', ' ', $term); // remove double spaces
  return google_api_post('detect', $fields, $full_result);
}

function google_tracker($acct) {
  return $acct ? "<script>
  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');
  ga('create', '$acct', 'auto');
  ga('send', 'pageview');
  //ga('require', 'ecommerce');   
  ga('require', 'ecommerce', 'ecommerce.js');   // Load the ecommerce plug-in.
  </script>" : '';
}                                                        

/** unsused - automatic comment added on  */
function google_tag_manager($id) {
  return "
  <!-- Google Tag Manager -->
  <noscript><iframe src=\"//www.googletagmanager.com/ns.html?id={$id}\"
  height=\"0\" width=\"0\" style=\"display:none;visibility:hidden\"></iframe></noscript>
  <script>(function(w,d,s,l,i){w[l]=w[l]||[];w[l].push({'gtm.start':
  new Date().getTime(),event:'gtm.js'});var f=d.getElementsByTagName(s)[0],
  j=d.createElement(s),dl=l!='dataLayer'?'&l='+l:'';j.async=true;j.src=
  '//www.googletagmanager.com/gtm.js?id='+i+dl;f.parentNode.insertBefore(j,f);
  })(window,document,'script','dataLayer','GTM-PK4BC4');</script>
  <!-- End Google Tag Manager -->
  ";
}

/** unsused - automatic comment added on  */
function google_add_calendar_event($client, $account_name, $eventAr, $options=NULL) {
  require_once 'google/google-api-php-client/src/apiClient.php';
  require_once 'google/google-api-php-client/src/contrib/apiCalendarService.php';

  $access_token = $client->getAccessToken();
  if(!$access_token) die("google_add_calendar_event: client not authorized"); // todo: authenticate
  $timeZone = $eventAr['timeZone'] ?: date_default_timezone_get();
  
  $service = new apiCalendarService($client);
  $event = new Event();
  
  if($summary = $eventAr['summary']) $event->setSummary($summary);
  if($location = $eventAr['location']) $event->setLocation($location);
  if($visibility = $eventAr['visibility']) $event->setVisibility($visibility);
  if($transparency = $eventAr['transparency']) $event->setTransparency($transparency);
    
  $start_time = $eventAr['start'];
  $start = new EventDateTime();
  if($timeZone) {
    $start->setTimeZone($timeZone);
    $datetime = gmdate('Y-m-d\TH:i:s', strtotime($start_time));
  } else {
    $datetime = sql2atom($start_time);
  }
  $start->setDateTime($datetime);
  $event->setStart($start);

  //dump($start);

  $end_time = $eventAr['end'];
  $end = new EventDateTime();
  if($timeZone) {
    $end->setTimeZone($timeZone);
    $datetime = gmdate('Y-m-d\TH:i:s', strtotime($end_time)); // no offset needed
  } else {
    $datetime = sql2atom($end_time); // with offset
  }
  $end->setDateTime($datetime);
  $event->setEnd($end);

  $attendees = $eventAr['attendees']; 
  $event_attendees = array();
  if(is_array($attendees)) {
    foreach($attendees as $att) {
      $attendee = new EventAttendee();
      if(is_object($att)) {
        $attendee->setEmail($att->email);
        $attendee->setDisplayName($att->displayName);
        //$attendee->setEmail($att);
      } else {
        $attendee->setEmail($att);
      }
      $event_attendees[] = $attendee;
    }
  $event->attendees = $event_attendees;
  }

  $createdEvent = $service->events->insert($account_name, $event);

  return $createdEvent;
}

/** Google Analytics e-commerce Javascript snippets */

// Function to return the JavaScript representation of a TransactionData object.
/** unsused - automatic comment added on  */
function getTransactionJs(&$trans) {
  return <<<HTML
ga('ecommerce:addTransaction', {
  'id': '{$trans['id']}',
  'affiliation': '{$trans['affiliation']}',
  'revenue': '{$trans['revenue']}',
  'shipping': '{$trans['shipping']}',
  'tax': '{$trans['tax']}'
});
HTML;
}

// Function to return the JavaScript representation of an ItemData object.
/** unsused - automatic comment added on  */
function getItemJs(&$transId, &$item) {
  return <<<HTML
ga('ecommerce:addItem', {
  'id': '$transId',
  'name': '{$item['name']}',
  'sku': '{$item['sku']}',
  'category': '{$item['category']}',
  'price': '{$item['price']}',
  'quantity': '{$item['quantity']}'
});
HTML;
}

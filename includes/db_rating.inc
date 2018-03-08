<?php

/** new review code */

function review_send($data) {
  
  //$siteObj = any2obj('site', $siteObj_or_id);
  //if(!$siteObj) return json_error_object('Missing site');
  
  $site_id = $data['site_id'];  
  $parent_id = $data['parent_id'];
  $parent_type = $data['parent_type'];
  $user_id = $data['user_id'];  
  $guest_id = $data['guest_id'];  
  
  $userObj = null;
  
  if($guest_id) {
    $userObj = get_user($guest_id);
  } elseif(isset($data['reviewer_name'])) {
  } elseif($user_id) {
    $userObj = get_user($user_id);    
  } else {
    return json_error_object(phrase("missing reviewer name and details"));
  }
  
  if($reviewObj = find_object('review', ['user_id' => $user_id, 'parent_type' => $parent_type, 'parent_id' => $parent_id])) {
    return json_error_object(phrase("you already rated this %s on %s", CAPS, get_object_name($parent_type), sql2human($reviewObj->created)));
  }
  
  if(!$site_id) return json_error_object('Missing site id');
  if(!$parent_id) return json_error_object('Missing parent id');
  if(!$parent_type) return json_error_object('Missing parent type');
  
  $parentObj = get_object($parent_type, $parent_id);
  if(!$parentObj) return json_error_object("Could not find $parent_type $parent_id");
  
  $scope->items = $items = find_objects('review_item', ['site_id' => $site_id, 'parent_type' => $parent_type]);
  if(!count($items))  return json_error_object("No review items found for $parent_type");
  foreach($items as $item) {
    $key = 'score_'.$item->id;
    //return json_error_object(dump($data,true));
    if(!$data[$key]) return json_error_object(phrase("please rate \"%s\"", CAPS, phrase($item->name, CAPS)));
  }

  if(isset($data['recommend']) && !$data['recommend']) {
    //return json_error_object(dump($data,true));
    return json_error_object(phrase("would you recommend this to others?", CAPS));
  }

  $min_words = 15;
  $word_count = str_word_count($data['review_comment']);
  if($word_count < $min_words) return json_error_object(phrase("please use at least %s words", CAPS, $min_words));
  
  $reviewData = $data;
  $reviewData['active'] = 0; /** activate when done */
  $reviewData['published'] = 0; /** later; publish option to publish when other party submits review or review period ends ? */
  
  if($userObj) {
    $reviewData['reviewer_name'] = get_name_string($userObj);
    $reviewData['reviewer_city'] = $userObj->city;
    $reviewData['reviewer_state'] = $userObj->state;
    $reviewData['reviewer_country'] = $userObj->country;
    $reviewData['reviewer_email'] = get_user_email($userObj, false); /** just the email address w/o name */
    $reviewData['reviewer_phone'] = $userObj->phone ?: $userObj->mobile;
  }
  
  
  list($id, $errors) = add_object('review', $reviewData);
  if($errors) return json_error_object($errors);
  
  $i = $total = 0;
  foreach($items as $item) {
    $key = 'score_'.$item->id;
    if(!isset($data[$key])) continue;
    $score = $data[$key];    
    $reviewData['review_id'] = $id;
    $reviewData['review_item_id'] = $item->id;
    $reviewData['score'] = $score;
    list($iid, $ierrors) = add_object('review_score', $reviewData);
    if($ierrors) return json_error_object($ierrors);
    $total += $score;
    $i++;
  }
  
  if($i) {
    $average = $total/$i; 
    list($id, $errors) = update_object('review', ['score' => $average, 'active' => 1, 'published' => 1], $id);
    if($errors) return json_error_object($errors);
    list($pid, $perrors) = update_object_review($parent_type, $parent_id);
    if($perrors) return json_error_object($perrors);
  } else {
    return json_error_object("No scores found"); /** should not happen */
  }
    
  $result = new stdClass;
  $result->success = 1;
  $result->id = $id;
  $result->message = phrase("thank you for your feedback", CAPITALIZE);
  return $result;
  
}

function update_object_review($obj_type, $obj_id) {
  $obj = get_object($obj_type, $obj_id);
  print_log("Update feedback: type={$obj_type} pid={$obj_id}", 'feedback', LOG_LEVEL_ALWAYS);
  if(isset($obj->review_score)) {
    $feedback_type = 'review'; // change to review soon
    $result = find_object($feedback_type, ['parent_type' => $obj_type, 'parent_id' => $obj_id], '', ['fields' => ['count(id) as count', 'sum(recommend) as recommend', 'avg(score) as score']]);
    $count = $result->count;
    $score = $result->score;    
    $recommend = $result->recommend;    
    print_log("Updating feedback: type={$obj_type} pid={$obj_id} total: $count score: $score", 'feedback', LOG_LEVEL_ALWAYS);

    $data = ['review_score' => $score];
    if(isset($obj->review_recommend)) $data['review_recommend'] = $recommend;
    if(isset($obj->review_count)) $data['review_count'] = $count;
    
    list($id, $errors) = update_object($obj_type, $data, $obj_id);
    
  } else {
    print_log("Not updating feedback: type={$obj_type} pid={$obj_id} - missing review_count/score", 'feedback', LOG_LEVEL_ALWAYS);
  }
  return $data;
}

/** old code */
// assoc. array of database keys => human readable name for rating
/** unsused - automatic comment added on  */
function rating_keys($siteObj, $property_type = 'shortstay') {
  $keys = array(
    "feedback_location" => phrase("location",CAPITALIZE), 
    "feedback_comfort" => phrase("comfort",CAPITALIZE),
    "feedback_cleaning" => phrase("cleaning",CAPITALIZE),
    "feedback_value" => phrase("value",CAPITALIZE),
    "feedback_overall" => phrase("overall",CAPITALIZE),
  );
  return $keys;
}

// assoc. array of rating value => human readable label
/** unsused - automatic comment added on  */
function rating_captions() {
  $captions = array(
      0 => phrase('not_rated', CAPITALIZE), 
      1 => phrase('awful', CAPITALIZE), 
      2 => phrase('terrible', CAPITALIZE), 
      3 => phrase('very_bad', CAPITALIZE), 
      4 => phrase('bad', CAPITALIZE), 
      5 => phrase('quite_bad', CAPITALIZE), 
      6 => "OK", 
      7 => phrase('quite_good', CAPITALIZE), 
      8 => phrase('good', CAPITALIZE), 
      9 => phrase('very_good', CAPITALIZE), 
      10 => phrase('excellent', CAPITALIZE),
  );
  return $captions;
}

// assoc. array of rating value => bootstrap classes used for star rating
/** unsused - automatic comment added on  */
function rating_classes() {
  $classes = array(
      1 => "label label-danger", 2 => "label label-danger", 3 => "label label-warning", 4 => "label label-warning", 
      5 => "label label-info", 6 => "label label-info", 7 => "label label-primary", 8 => "label label-primary", 
      9 => "label label-success", 10 => "label label-success");
  return $classes;
}


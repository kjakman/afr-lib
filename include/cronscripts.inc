<?php

/**
  function that implements cronscript operand
  extend by site in  function <shortname>_cronscript() in file  <shortname>_cronscripts.inc
*/

function cronscript($op) {
  $messages = $errors = [];
  $now = now();
  print_log("{$now}: global cronscript {$op} start", 'crontab', LOG_LEVEL_TEST);
  $result = new stdClass;
  switch($op) {
    case 'calendar-import':
      $result = calendar_import(); 
      break;
      
    case 'booking-reminder':
      $result = booking_checkin_reminder(); 
      break;

    case 'booking-review':
      $result = booking_review_reminder(); 
      break;

    default:
      $result->success = -1;
      break;
  }
  return $result;
}


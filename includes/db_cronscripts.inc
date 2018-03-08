<?php
/**
  run single cron script operaration, or crontab if no operaration is specififed
*/
function cronscript_run($op, $options=[]) {
  global $u_level;
  if(!$op) return crontab_run($options); // run all scripts in crontab.yml that are scheduled for now
  
  $ip = client_ip();
  $allowed_ips = ['178.62.240.47', '188.166.113.34', '62.194.215.87', '217.103.202.83'];
  if($u_level < USER_LEVEL_ADMIN && !in_array($ip, $allowed_ips)) return json_error_object("{$now}: Crontab: {$ip} is not in allowed list");

  global $__SITE;
  $now = now();
  $result = new stdClass;
  $success = false;
  $messages = $errors = array();
  
  $now = now();


  /** global cronscript */
  $result = cronscript($op);
  
  /** should we run both site and global, or just one ? */
  
  /** site cronscript */
  $site_fn = "{$__SITE->shortname}_cronscript";
  
  if(function_exists($site_fn)) {
    $result = call_user_func($site_fn, $op);
  } 
  
  return $result;  
}


/**
  parse crontab.yml and run all cron script operands that apply (match schedule)
*/
function crontab_run($options=[]) {
  global $__SITE, $u_level;
  $now = now();
  
  // if(!trim($oper)) return json_error_object("{$now}: Missing operand, called by $ip");
  print_log("\n{$now}: {$__SITE->name} Starting crontab (oper={$oper}) called by $ip", 'crontab', LOG_LEVEL_TEST);

  
  $time_start = microtime(true);
  $result = new stdClass;

  $obj_data = $_GET ?: $_POST; // prioritize $_GET for control, $_POST for data (below)
  
  $messages = $errors = $opers = array();
  
  
  $cronfile = docroot()."config/crontab.yml";
  $crontabs = is_file($cronfile) ? yaml_parse_file($cronfile) : [];
  // print_log("Crontabs:".dump($crontabs, true), 'crontab', LOG_LEVEL_TEST);  
  // dump($crontab);
  $ts = date("i-H-j-n-N");
  list($min, $hour, $day, $month, $weekday) = explode('-', $ts);
  foreach($crontabs as $crontab) {
    $schedule = $crontab['when'] ?: '';
    $op = $crontab['oper'] ?: '';
    $url = $crontab['url'] ?: '';
    $data = $crontab['data'] ?: [];

    print_log("{$now}: Checking for op=$op url=$url schedule=$schedule", 'crontab', LOG_LEVEL_TEST);

    if(!$schedule) continue;
    if(!$op && !$url) continue;
    
    
    list($min1, $hour1, $day1, $month1, $weekday1) = $ar = explode(' ', $schedule);    
    if($weekday1 == '0') $weekday1 = 7;

    $run = true;
    /** todo: syntax checking of schedule, continue if not correct */
    if(count($ar) != 5) $run = false;
    
    if(strpos($min1, "*/") === 0) { // allow */5 for every 5 minutes
      list($m, $min1) = explode("/", $min1);
      $min1 = intval($min1);
      print_log("Run every $min1 minutes");
      if($min % $min1 > 0) $run = false;  
    } else {    
      if($min1 != '*' && intval($min1) != intval($min)) $run = false;
    }

    if(strpos($hour1, "*/") === 0) { // allow */5 for every 5 hours
      list($m, $hour1) = explode("/", $hour1);
      $hour1 = intval($hour1);
      if($hour % $hour1 > 0) $run = false;  
    } else {    
      if($hour1 != '*' && intval($hour1) != intval($hour)) $run = false;
    }
    
    //if($hour1 != '*' && intval($hour1) != intval($hour)) $run = false;
    if($day1 != '*' && intval($day1) != intval($day)) $run = false;
    if($month1 != '*' && intval($month1) != intval($month)) $run = false;
    if($weekday1 != '*' && intval($weekday1) != intval($weekday)) $run = false;

    
    if($run) {
      print_log("{$now}: OK, running op=$op url=$url schedule=$schedule", 'crontab', LOG_LEVEL_ALWAYS);
      $opers[] = $op;
    } else {
      print_log("{$now}: NOT running op=$op url=$url schedule=$schedule", 'crontab', LOG_LEVEL_TEST);
      continue;
    }
         
  }
  
  if(!$errors && count($opers)) {
    $result = cronscripts_run($opers);
  } else {
    $result->success = $success;
    $result->error = $errors;
    $result->message = $messages;
  }
  
  return $result;
}


/**
  run array of cron script operands
*/
function cronscripts_run($opers) {
  global $__SITE;
  $now = now();
  $result = new stdClass;
  $success = false;
  $messages = $errors = array();
  
  foreach($opers as $op) {
    
    $time_start = microtime(true);
    $r = cronscript_run($op);
    $time_end = microtime(true);
    $success = $r->success;

    if($m = $r->message) $messages[] = $m;
    if($e = $r->error) $errors[] = $e;
    
    $time = 1000*($time_end - $time_start);
    if($errors || !$success) {
      print_log("{$now}: did not run {$op}: ".$errors[0], 'crontab', LOG_LEVEL_TEST);
    } else {
      print_log("{$now}: ran {$op} in $time ms", 'crontab', LOG_LEVEL_ALWAYS);
    }
    
  }
  
  $result->success = $success;
  $result->error = $errors;
  $result->message = $messages;
  return $result;
}


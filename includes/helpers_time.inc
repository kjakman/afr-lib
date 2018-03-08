<?php

// hourly
// case 1: same day:  27 November 2012: 22.00 - 23.00 or  just 22.00-23.00 or 10pm - 12pm
// case 2: same day:  27 November 2012 22.00 - 28 November 02.00

// daily
// case 1: same month:  27-29 November, 2012 
// case 2: not same month: 27 November - 3 December 2012 
// case 3: not same year:  27 November 2012 - 3 January 2013 

function print_time_range($in, $out, $options=NULL) {
  $start = strip_date($in);
  $end   = strip_date($out);
  
  list($in_day, $in_month, $in_year) = sql2date($in);
  list($out_day, $out_month, $out_year) = sql2date($out);

  $unit = $options['unit'];
  $hourly = $start && $end && (strip_time($in) == strip_time($out) || $options['hourly'] || $unit == 'hour');
  $show_in_year = $show_out_year = false;
  
  //echo("in=$in out=$out start=$start end=$end hourly=$hourly: ".dump($options,true));
  if($hourly) { // hourly
    $show_out_date = $options['show_date'] && $in_day != $out_day;
    return sql2human_time($in, $options).($end ? ' - '.sql2human_time($out, array('show_date' => $show_out_date)) : '');   
  } else { // daily
    $show_year = isset($options['show_year']) ? $options['show_year'] : true; 
    $show_month = $in_month == $out_month ? false : true; 
    $show_time = false; 
    $show_weekday = $options['show_weekday'];
    if($show_year && $in_year != $out_year) $show_in_year = true; // always show year if not the same
    
    $in_str = sql2human($in, array('show_weekday' => $show_weekday, 'show_year' => $show_in_year, 'show_month' => $show_month));
    $out_str = sql2human($out, array('show_weekday' => $show_weekday, 'show_year' => $show_year));
    return $in == $out ? $out_str : "$in_str - $out_str";
  }

}

// returns date in given timezone
/** unsused - automatic comment added on  */
function userdate($date, $timezone, $format="Y-m-d H:i:s") {
  $userTimezone = new DateTimeZone(!empty($timezone) ? $timezone : 'GMT');
  $gmtTimezone = new DateTimeZone('GMT'); // instead of UTC, due to leap second diff
  $userDateTime = new DateTime($date, $gmtTimezone);
  $offset = $userTimezone->getOffset($userDateTime);
  return date($format, strtotime($date) + $offset);
}

/** unsused - automatic comment added on  */
function sql2human_datetime($date, $options=NULL) {
  return sql2human($date)." ".sql2human_time($date);
}

function sql2human_time($date, $options=NULL) {
  $time = strip_date($date);  // gets 15:00:00
  return $options['show_date'] ? sql2human($date, $options)." ".substr($time, 0, 5) : substr($time, 0, 5); // returns 15:00
}


//function sql2human($date, $show_weekday=false, $show_time=false, $show_year=true, $show_month=true) {
function sql2human($date, $options=NULL) {
  $show_weekday = isset($options['show_weekday']) ? $options['show_weekday'] : false;
  $show_time    = isset($options['show_time'   ]) ? $options['show_time'   ] : false;
  $show_year    = isset($options['show_year'   ]) ? $options['show_year'   ] : true;
  $show_month   = isset($options['show_month'  ]) ? $options['show_month'  ] : true;
  $month_name   = isset($options['month_name'  ]) ? $options['month_name'  ] : true;
  $separator    = $options['separator'] ?: '-';
  
  
  $date_str = date_convert_sql2human($date, $month_name, $show_year, $show_month, $separator);

  $time = sql2human_time($date);
  if($show_weekday) {
    list($day, $month, $year) = sql2date($date);
    $weekday = get_weekday($day, $month, $year);
    $weekdays = WEEKDAYS;
    $weekday_str = $weekdays[$weekday];
    $date_str = "$weekday_str, $date_str";
  }
  if($show_time && $time) {
    $date_str .= " $time";
  }
  return $date_str;  
}

// takes yyyy-mm-dd + array of human readable months (January, February...)
// and returns human readable date, e.g. 13 September, 2004
function date_convert_sql2human($date, $show_month_name=true, $show_year=true, $show_month=true, $sep='-') {
   if(strpos($date, '0000') === 0) return ''; // invalid (db null) date
   list ($day, $month, $year) = sql2date($date);
   
   if(!$show_month) {
     return $day; // don't show year if show_month is false 
   } elseif($show_month_name) {
     $date_str = "$day ".month_name($month);
     return $show_year ? $date_str." $year" : $date_str;
    } else {
     return $show_year ? $day.$sep.$month.$sep.$year : "$day.$sep.$month";
   }
}

// thanks http://stackoverflow.com/questions/2915864/php-how-to-find-the-time-elapsed-since-a-date-time
// return array(count, unit)
// returns negative count if future
function time_elapsed($datetime) {
  if(!$datetime) die("no datetime");  
  $ts = strtotime($datetime);
  $now = time();
  $diff = $elapsed = $now - $ts;
  $sign = 1;
  if($elapsed < 0) { // future
    $sign = -1;
    $elapsed *= -1;
  }
  if(!$elapsed) return array(0, 'second');
  //echo("<h3>$datetime ts=$ts now=$now diff=$diff</h3>");
  
  $tokens = array (
    31536000 => 'year',
    2592000 => 'month',
    604800 => 'week',
    86400 => 'day',
    3600 => 'hour',
    60 => 'minute',
    1 => 'second'
  );

  foreach ($tokens as $seconds => $unit) {
    if ($elapsed < $seconds) {
      //echo(" - $elapsed < $seconds ($unit): continue<br>");
      continue;
    }
    $count = floor($elapsed / $seconds);
    //echo(" - $elapsed <b>count=$count $unit</b><br>");

    return array($sign * $count, $unit);
  }

}


// return array(count, unit)
/** unsused - automatic comment added on  */
function time_elapsed_human($datetime) {
  list($count, $unit) = time_elapsed($datetime);

  if($count >= 0 && $unit == 'second') return phrase('just_now');
  if(abs($count) == 1 && $unit == 'day') return $count < 1 ? phrase('tomorrow') : phrase('yesterday');
  
  $text = abs($count) == 1 ? phrase($unit) : phrase($unit."s");  
  
  return abs($count)." $text ".($count < 0 ? phrase('from_now') : phrase('ago'));
  
}

/** unsused - automatic comment added on  */
function sql2moment($date) {
  $time = strtotime($date);
  return date('d-m-Y', $time); 
}

/** unsused - automatic comment added on  */
function timer_end($start) {
  if(!$start) return "Timer: no start time";
  $finish = getTime();
  $total_time = round(($finish - $start), 4);
  return $total_time."s";
}

function getTime() { 
  return microtime(true);
} 
                  
/** unsused - automatic comment added on  */
function tomorrow($format = 'Ymd', $sep = '-') {  
  return add_day(today(), 1);
}

function yesterday($format = 'Ymd', $sep = '-') {  
  return add_day(today(), -1);
}

/** unsused - automatic comment added on  */
function get_tomorrow2() {
  return get_today(1);
}

// takes a time, like 04:00, and adds 24*$num_days to this time, returning 28:00 for 1 day
// seconds are stripped away if present
/** unsused - automatic comment added on  */
function add_days_to_time($in_time, $num_days) {
  $in_time = strip_sec($in_time);
  list($h, $m) = explode(':', $in_time);
  $h = strip_leading_zero($h) + 24 * $num_days;
  return add_leading_zero($h).':'.$m;  
}

// returns only hh:mm portion of datetime yyyy-mm-dd 00:00:00
function strip_date_sec($datetime) {
  $time = strip_date($datetime);
  return substr($time, 0, 5);
}

// returns only the hours:minutes portion time 00:00
function strip_sec($time) {
  return substr($time, 0, 5);
}

// returns only date portion of datetime yyyy-mm-dd 00:00:00
function strip_time($datetime) {
  return substr($datetime, 0, 10);
}

// returns only time portion of datetime yyyy-mm-dd 00:00:00
function strip_date($datetime) {
  return substr($datetime, 11);
}

// returns datetime for hourly, date otherwise
// datetime can be single value or array
function date_or_datetime($datetime, $hourly) {
  if(is_array($datetime)) {
    $result = array();
    foreach($datetime as $dt) $result[] = date_or_datetime($dt, $hourly);
    return $result;
  }
  return $hourly ? $datetime : strip_time($datetime);
}

// takes h:mm, hh:mm, h:mm:ss, or hh:mm:ss returns hh:mm:ss or blank if input format not hh:mm:ss or hh:mm  
function clean_time($time) {
  $reg_exp = "/^(([0-1][0-9])|([2][0-3]))(:([0-5][0-9])){1,2}$/"; // requires 2 digits for hours
  $reg_exp = "/^(([0-9])|([0-1][0-9])|([2][0-3]))(:([0-5][0-9])){1,2}$/"; // allows 1 digit for hours

  if(!preg_match($reg_exp, $time, $matches)) return '';
  $len = strlen($time);
  switch ($len) {
    case 4:
      return '0'.$time.':00';
    case 5:
      return $time.':00';
    case 7:
      return '0'.$time;
    default:
      return $time;
  }
}

// takes a date + time + num_hours, returns in+out
// time can be hh:mm:ss, hh:mm, or decimal (8, 8.5)
// assumes valid input, but checks for valid output or returns array with empty values
/** unsused - automatic comment added on  */
function date_time_hours2in_out($date, $time, $hours) {
  $hm_time = clean_time($time) ?: dectime2hm($time, true, true);
  $in = strip_time($date)." ".$hm_time;
  //if(!is_datetime($in)) return array("<b>$in</b> is not valid", '');
  if(!is_datetime($in)) return array('', '');
  $hour_minute = dectime2hm($hours);
  $out = $searchObj->out = add_interval($in, $hour_minute, 'HOUR_MINUTE');   
  //if(!is_datetime($out)) return array('', "<b>$out</b> is not valid");
  if(!is_datetime($out)) return array('', '');
  return array($in, $out);
}

// takes 4:30, return 4.5 hours
/** unsused - automatic comment added on  */
function hm2dectime($time) {
  if(!$time) return 0;
  list($h,$m) = explode(':', strip_sec($time));
  $h = strip_leading_zero($h);
  $m = strip_leading_zero($m);
  return $h + round($m/60, 2);
}

// takes 4.5 hours,returns 4:30 (or 04:30:00 if seconds and zeropad_hours are true)
// seconds are not calculated, always 00
function dectime2hm($num_hours, $seconds=false, $zeropad_hours=false) {
  $num_hours = clean_num($num_hours); // change 2.00 to 2
  $floor = floor($num_hours);
  if($num_hours == $floor) return ($zeropad_hours ? zero_pad($num_hours) : $num_hours).":00".($seconds ? ":00" : "");
  list($h,$d) = explode('.', $num_hours);
  if($zeropad_hours) $h = zero_pad($h);
  return "$h:".add_leading_zero(round(60*($num_hours - $floor))).($seconds ? ":00" : "");
}

function get_today($add=0) {
  $today = $add ? getdate(strtotime ("+$add day")) : getdate();
  return array($today['mday'], $today['mon'], $today['year']);
}

function stripdate($date) {
  return str_replace(array(' ',':','-'), '', $date);
}

function isToday($date) {
  $today_day=$_SESSION['today_day'];
  $today_month=$_SESSION['today_month'];
  $today_year=$_SESSION['today_year'];
  $today = ($date['day'] == $today_day && $date['month'] == $today_month && $date['year']== $today_year) ? true : false;
  return $today;
}

function isFuture($date) {
  $today_day=$_SESSION['today_day'];
  $today_month=$_SESSION['today_month'];
  $today_year=$_SESSION['today_year'];
  $future = (($date['year'] > $today_year) ||
            ($date['year'] == $today_year && $date['month'] > $today_month) ||
            ($date['year'] == $today_year && $date['month'] == $today_month && $date['day'] > $today_day)) ? true : false;
  return $future;
}

function isAfter($date, $date2, $inclusive=0) {
  $date = str_replace("-", "", $date);
  $date2 = str_replace("-", "", $date2);
  return $inclusive ? $date >= $date2 : $date > $date2;
}

function isBefore($date, $date2, $inclusive=0) {
  $date = str_replace("-", "", $date);
  $date2 = str_replace("-", "", $date2);
  return $inclusive ? $date <= $date2 : $date < $date2;
}

/** unsused - automatic comment added on  */
function isBeforeOrOn($date, $date2) {
  return isBefore($date, $date2, 1);
}

/** unsused - automatic comment added on  */
function isAfterOrOn($date, $date2) {
  return isAfter($date, $date2, 1);
}

/** unsused - automatic comment added on  */
function isPast($date) {
  $today = isToday($date);
  $future = isFuture($date);
  $past = !($future || $today);
  return $past;
}

/** unsused - automatic comment added on  */
function n_hours_ago($n) {
  return date('YmdHis', time() - ($n *  60 * 60));
}

/** unsused - automatic comment added on  */
function n_days_ago($n) {
  return date('Y-m-d', time() - ($n *  60 * 60 * 24));
}


// returns difference in seconds between startdate and enddate (mysql date or datetime)
/** unsused - automatic comment added on  */
function get_mysql_age($startdate, $enddate) {
  return mysqldate_to_unix_timestamp($startdate) - mysqldate_to_unix_timestamp($enddate);
}

// Returns the Unix timestamp of a MySQL date (YYYY-MM-DD) or MySQL datetime (YYYY-MM-DD 00:00:00)
function mysqldate_to_unix_timestamp($date) {
  if(strlen($date)==10) {
    list($year, $month, $day) = explode('-', $date);
    return mktime(0, 0, 0, $month, $day, $year);
  } elseif(strlen($date)==19) {
    $time = substr($date, 11);
    $date = substr($date, 0, 10);
    //echo("date='$date' time='$time'<br>");
    list($hour, $minute, $second) = explode(':', $time);
    list($year, $month, $day) = explode('-', $date);
    return mktime($hour, $minute, $second, $month, $day, $year);
  } else {
    die("$date is not a valid MySQL date or datetime");
  }
}

/** unsused - automatic comment added on  */
function sql2ts($date) {
  return mysqldate_to_unix_timestamp($date);
}


function local2utc($sql) {
  //die("gh");
  $system_tz = date_default_timezone_get();
  $the_date = strtotime($sql);
  date_default_timezone_set("UTC"); 
  $result = date("Y-m-d H:i:s", $the_date);
  if($system_tz) date_default_timezone_set($system_tz);
  return $result;
}

// takes SQL date or date-time, returns 20110326 or 20110326T090000Z (in GMT, Zulu time), using Server's timezone
/** unsused - automatic comment added on  */
function sql2zulu($date='') {
  if(!$date) $date = now();
  //$date = local2utc($time);  
  return strip_date($date) ? gmdate('Ymd\THis\Z', strtotime($date)) : date('Ymd', strtotime($date));
}

// converts sql time to RFC3339 (ATOM)
function sql2atom($date='') {
  if(!$date) $date = now();
  $utc = local2utc($date);
  $atom =  date(DATE_RFC3339, strtotime($utc));
  //echo("<b>utc for $date = $utc, atom=$atom</b><br>");
  return $atom;
}

// takes date of birth (sql), returns age in years 
/** unsused - automatic comment added on  */
function dob2age($dob, $on_date = 0) {
  if(!$on_date) $on_date = today();
  $now = new DateTime($on_date);  
  $born = new DateTime($dob); 
  return $born->diff($now)->y;
}

// takes $datetime yyyymmddhhmmss (20051117062949) and returns age in seconds
/** unsused - automatic comment added on  */
function get_age($datetime) {
  $year = substr($datetime, 0, 4);
  $month = substr($datetime, 4, 2);
  $day = substr($datetime, 6, 2);
  $hour= substr($datetime, 8, 2);
  $min = substr($datetime, 10, 2);
  $sec = substr($datetime, 12, 2);
  return mktime($hour, $min, $sec, $month, $day, $year)-time();
}

// also handles special case for sorting: reverse order if already sorting
// accepts either one argument (sql date) or two (month and year)
function get_daysofmonth($month_or_date, $year=0) {
   if(!$year) list($year, $month, $day) = explode('-', $month_or_date);
   else $month = $month_or_date;

   if ($month == 2) { // check for february 29th
      $isleap = ($year % 4 == 0 && ($year % 100 != 0 || $year % 400 == 0));
      return ($isleap ? "29" : "28");
   }
   if ($month==4 || $month==6 || $month==9 || $month==11) {
      return "30";
   } else {
      return "31";
   }
}

// CHANGED: Old returns a numberic representation of the day of the week 0 (for Sunday) through 6 (for Saturday)

// NOW: 1 = Monday, 7 = Sunday (ISO-8601)
function get_sql_weekday($date, $iso = true) {
  $format = $iso ? "N" : "w";
  return date($format, strtotime($date));
}

// N: 1 = Monday, 6 = Sunday (ISO-8601)
function get_weekday($day, $month, $year, $iso = true) {
  $format = $iso ? "N" : "w";
	return date($format, mktime(0, 0, 0, $month, $day, $year));
	//return date("w", mktime(0, 0, 0, $month, $day, $year)); //OLD: 
}

// returns date for given year, week number, and day (0 for sunday, 1 for monday, etc..)
function get_date_of_week($year, $week, $day=1) {
	$week = add_leading_zero($week);
	return date('Y-m-d',strtotime($year."W".$week.$day)); 
}


// returns ISO-8601 week and ISO-8601 year number 
// ISO-8601 year number has the same value as Y, except that if the ISO week number (W) 
// belongs to the previous or next year, that year is used instead.

function get_week_year($day, $month, $year) {
   $week = get_week($day, $month, $year);
   $week_year = date("o", mktime(0, 0, 0, $month, $day, $year));
	 return array($week, $week_year);	
}

function get_week($day, $month, $year) {
   return date("W", mktime(0, 0, 0, $month, $day, $year));
}

// Jan=1, no leading zerors
function get_prev_month($month, $year) {
   // find previous month/year
   if($month==1) {
      return array(12, $year-1);
   } else {
      return array($month-1, $year);
   }
}

// Jan=1, no leading zerors
function get_next_month($month, $year) {
   // find next month/year
   if($month==12) {
      return array(1, $year+1);
   } else {
      return array($month+1, $year);
   }
}

// add $n months to given year/month
// returns (month, year) (w/o leading zero, JAN=1)
/** unsused - automatic comment added on  */
function add_months($month, $year, $n) {
	global $db;
	$ym = $year.zero_pad($month);
	$nym = db_getOne("SELECT PERIOD_ADD($ym,$n)");
	return array(strip_leading_zero(substr($nym,4)), substr($nym,0,4));
}

/** unsused - automatic comment added on  */
function get_prev_week($week, $year) {
	$date_monday = get_date_of_week($year, $week, 1);
  list($day, $month, $year) = sql2date($date_monday);     
  list($yd, $ym, $yy) = add_days($day, $month, $year, -1);
	list($week, $year) = get_week_year($yd, $ym, $yy);
  return array($week, $year);
}

// returns ISO-8601 week and ISO-8601 year number of given week/year + num_weeks weeks
/** unsused - automatic comment added on  */
function get_next_week($week, $year, $num_weeks=1) {
  $add_days = $num_weeks*7;
	$date_monday = get_date_of_week($year, $week, 1);
  list($day, $month, $year) = sql2date($date_monday); 
  list($td, $tm, $ty) = add_days($day, $month, $year, $add_days);
	list($week, $year) = get_week_year($td, $tm, $ty);
  return array($week, $year);
}


// takes either sql calendar date array, returns next day either as sql (default) or array 
function get_next_day($date, $sql=true) {
  list($day, $month, $year) = sql2date($date);
  $ndays= get_daysofmonth($month, $year);
  $day++;
  if($day>$ndays) {
    $day=1;
    list($month, $year) = get_next_month($month, $year);
  }
  return $sql ? get_sql_date($day, $month, $year) : array($day, $month, $year);
}

// takes either sql calendar date array, returns previous day either as sql (default) or array
/** unsused - automatic comment added on  */
function get_prev_day($date, $sql=true) {
  list($day, $month, $year) = sql2date($date);
  $day--;
  if($day == 0) {
    list($month, $year) = get_prev_month($month, $year);
    $day = get_daysofmonth($month, $year);
  }
  return $sql ? get_sql_date($day, $month, $year) : array($day, $month, $year);
}


// convert day/month/year to sql date. Zero padded numbers ok. 
// As per mktime: The number of the year, may be a two or four digit value, with values between 0-69 mapping to 2000-2069 and 70-100 to 1970-2000
function get_sql_date($day, $month, $year) {
  return date('Y-m-d', mktime(0, 0, 0, ltrim($month, '0'), ltrim($day, '0'), $year));
}

function human2sql($date, $options=NULL) {
  return date("Y-m-d H:i:s", strtotime($date));
}


// use strtotime to return sql date
function date2sql($date) {
  if(!trim($date)) return '';
  return date("Y-m-d", strtotime($date));
}


/** unsused - automatic comment added on  */
function dates2human($date, $optins=NULL) {
  return sql2human(dates2sql($date), $options);
}

// same as above, but takes one argument: ass array (day=> month=> year=>) and returns yyyy-mm-dd
/** unsused - automatic comment added on  */
function dates2sql($date) {
  return get_sql_date($date['day'] ?: $date['d'], $date['month'] ?: $date['m'], $date['year'] ?: $date['y']);
}

// returns array (d,m,y) from sql date
function sql2date($date) {
  list($y,$m,$d) = explode("-", strip_time($date));
  return array(ltrim($d, '0'), ltrim($m, '0'), $y);
}

// counts number of whole months and number of rest days between in and out dates
function count_month_days($in, $out) {
  $date1 = new DateTime($in);
  $date2 = new DateTime($out);
  $interval = $date1->diff($date2);
  return array($interval->d, 12*$interval->y + $interval->m);
}
  
/** unsused - automatic comment added on  */
function iso2sql($iso) {
  return date('Y-m-d H:i:s', strtotime($iso));
}

//function iso2human($iso, $show_weekday=false, $show_time=false, $show_year=true) {
function iso2human($iso, $options=NULL) {
  return sql2human(iso2sql($iso), $options);
}


/* Works out the time since the entry post, takes a an argument in unix time (seconds) */
/** unsused - automatic comment added on  */
function time_since($original) {
  // array of time period chunks
  $chunks = array(
    array(60 * 60 * 24 * 365 , 'year'),
    array(60 * 60 * 24 * 30 , 'month'),
    array(60 * 60 * 24 * 7, 'week'),
    array(60 * 60 * 24 , 'day'),
    array(60 * 60 , 'hour'),
    array(60 , 'minute'),
  );
  
  $today = time(); /* Current unix time  */
  $since = $today - $original;
  
  // $j saves performing the count function each time around the loop
  for ($i = 0, $j = count($chunks); $i < $j; $i++) {    
    $seconds = $chunks[$i][0];
    $name = $chunks[$i][1];    
    if (($count = floor($since / $seconds)) != 0) break; // finding the biggest chunk (if the chunk fits, break) 
  }
  
  $print = ($count == 1) ? '1 '.$name : "$count {$name}s";
  
  if ($i + 1 < $j) {
    // now getting the second item
    $seconds2 = $chunks[$i + 1][0];
    $name2 = $chunks[$i + 1][1];
    
    // add second item if it's greater than 0
    if (($count2 = floor(($since - ($seconds * $count)) / $seconds2)) != 0) {
      $print .= ($count2 == 1) ? ', 1 '.$name2 : ", $count2 {$name2}s";
    }
  }
  return $print;
}

// takes a number seconds (float), returns x ms / s etc
function microtime2human($input) {
  $divAr = array('m' => 60, 's' => 1, 'ms' => 0.001, 'us' => 0.000001, 'ns' => 0.000000001);
  foreach($divAr as $unit => $q) {
    if($input >= $q) return formatFloat($input / $q).$unit;    
  }
  return $input.'seconds';
}


// takes yyyy-mm-dd + array of human readable months (January, February...)
// and returns human readable date, e.g. 13 September, 2004
/** unsused - automatic comment added on  */
function timestamp2human($timestamp, $month_names=0) {
   $year= substr($timestamp, 0, 4);
   $month= substr($timestamp, 4, 2);
   $month = $month_names ? $month_names[$month-1] : $month;
   $day= substr($timestamp, 6, 2);
   $hour = substr($timestamp, 8, 2);
   $min = substr($timestamp, 10, 2);
   $sec = substr($timestamp, 12, 2);

   return "$day-$month-$year $hour:$min:$sec";
}

// add days using PHP date function
function sql_add_days($date, $days) {
  if(!$date) $date = now();
	return date("Y-m-d H:i:s",strtotime($date) + 60*60*24*$days);	
}

// does for fractions, like 15 mins.
/** unsused - automatic comment added on  */
function sql_add_hours($date, $hours) {
  if(!$date) $date = now();
	return date("Y-m-d H:i:s",strtotime($date) + 60*60*$hours);	
}                                     

// add/subtract days from sql date, return new sql date
function add_day($date, $i) {
  if(!$date) $date = today();
  $term = $i < 0 ? " - ".abs($i)." days" : " + ".ltrim($i, '+')." days";
  return date("Y-m-d", strtotime($date.$term));
}

// takes day/month/year/num_days, returns new date as array(day/month/year)
function add_days($d, $m, $y, $num_days) {
  $sql = get_sql_date($d, $m, $y);
  $date = add_day($sql, $num_days);
  return sql2date($date);  
}

/** unsused - automatic comment added on  */
function get_tomorrow($td_day, $td_month, $td_year) {
  return add_days($td_day, $td_month, $td_year, 1);
}


// same as period_diff, using built-in PHP function diff
// valid units:
/*
    [y] => 0
    [m] => 1
    [d] => 0  Number of days (added to number of months)
    [h] => 0
    [i] => 0
    [s] => 0
    [invert] => 0
    [days] => 30 Number of days regardless of years/months
*/

/** period_diff does not return negative nubmers, this does. */

function day_diff($in, $out) {
  return floor( (strtotime(strip_time($out)) - strtotime(strip_time($in))) / (60 * 60 * 24) );
}
  
// takes in+out date, returns number of hours (2 decimals), days (integer), or month (3 decimals)
// uses datediff 
function period_diff($in, $out, $period) {
  $period = $period[0]; // first charachter  
  switch($period) {
    case 'h';
      return datediff('h', $in, $out, 2);
      break;
    case 'd';
      return datediff('d', $in, $out, 0); // whole days only
      break;
    case 'm';
      return datediff('m', $in, $out, 4);
      break;
    default:
      die("period_diff: invalid period $period");
      break;
  }
}

function datediff($unit, $in, $out, $decimal_places = 0) {
  $date1 = new DateTime($in);
  $date2 = new DateTime($out);
  $interval = $date1->diff($date2);
  $unit = $unit['0']; // use first letter (allow 'hour', 'hours', 'h' etc
  
  $interval_unit = $unit == 'd' ? 'days' : $unit;
  
  //if(!$fractions) return $internval->invert ? -1 * $result : $result;
    
  $result = $interval->$interval_unit; // returns a whole number

  // ok, return fractions:
  switch($unit) {
    case 'y':
      // if($decimal_places && $interval->d) $result += $interval->d / $days_in_month;
      break;
    case 'm':
      $result += $interval->y * 12;
      if($decimal_places && $interval->d) {
        list($out_y, $out_m, $out_d) = explode('-', strip_time($out));
        $days_in_last_month =  get_daysofmonth($out_m, $out_y); // todo: use actual
        //echo("Days in $out_m / $out_y ($out) = $days_in_last_month<br>"); 
        $result += $interval->d / $days_in_last_month;
      }
      break;
    case 'h':
      $result += $interval->days * 24;
      if($decimal_places && $interval->i) $result += $interval->i / 60;
      if($decimal_places && $interval->s) $result += $interval->s / 3600;
      break;
    case 'i':
      if($decimal_places && $interval->s) $result += $interval->s / 3600;
      break;
    case 's':
      break;
    case 'd':
      if($decimal_places && $interval->h) {
        //$hours = datediff('h', $in, $out, true); 
        $result += $interval->h / 24;
      }
      break;
  }
  if($decimal_places) $result = number_format($result, $decimal_places, '.', '');
  return clean_num($result);
}

// checks if input is valid time notation (hh:mm:ss)
function is_time($time) {
  return preg_match("/^([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$/", $time, $matches);
}

function is_datetime($date) {
  if(strlen($date) != 19) return false;
  return validateDate($date, 'Y-m-d H:i:s');
} 

function is_date($date) {
  if(strlen($date) != 10) return false;   
  return validateDate($date, 'Y-m-d');
}

// thanks glavic: http://php.net/manual/en/function.checkdate.php
function validateDate($date, $format = 'Y-m-d H:i:s') {
  $d = DateTime::createFromFormat($format, $date);
  return $d && $d->format($format) == $date;
}
// from db.inc
// Date and time functions


// takes a floating num_hours (e.g. 1.5) and returns H:M representation (1:30)
function num_hours2hm($num_hours) {
  $hours = floor($num_hours);
  $rest = $num_hours - $hours;  
  $minutes = zero_pad(round($rest * 60));
  return "$hours:$minutes";  
}

/** unsused - automatic comment added on  */
function n_hours_later($date, $num_hours) {
  return add_interval($date, num_hours2hm($num_hours), 'HOUR_MINUTE');
}

/** unsused - automatic comment added on  */
function n_days_later($date, $days) {
  return add_day($date, $days);
}

function next_day($date) {
  return add_day($date, 1);
}

/** unsused - automatic comment added on  */
function prev_day($date) {
  return add_day($date, -1);
}

function add_interval_string($date, $interval) {
  if(is_date($date)) $format = 'Y-m-d';
  else if(is_datetime($date)) $format = 'Y-m-d H:i:s';
  else return '';
  $pattern = "/([+-]?\d+) (DAY|HOUR)/"; // integer
  if(preg_match($pattern, $interval, $m)) {
    $count = $m[1];
    $unit = $m[2];
    $result = date($format, strtotime($date . " $count $unit"));
    return $result;
  }
  return '';
}

// add $n intervals ('DAY', 'HOUR' etc) to date
// see: http://dev.mysql.com/doc/refman/5.1/en/date-and-time-functions.html#function_date-add
// returns datetime if $date is datetime, date if $date is date
function add_interval($date, $n, $interval='DAY') {
  global $db;
  //echo("SELECT DATE_ADD('$date', INTERVAL '$n' $interval)<br>");
  return db_getOne("SELECT DATE_ADD('$date', INTERVAL '$n' $interval)");
}

// returns number of hours between $from and $to
/** unsused - automatic comment added on  */
function timediff($from, $to, $interval='HOUR') {
  return period_diff($from, $to, 'hour');
}

// returns a series (array) of dates or datetimes (depending on format of $dste)
// if duration is not set, returns array of $count elements array($date1, $date2, $date3, ...) spaced $n $intervals apart 
// if duration is set, returns array of $count ranges ($range1, $range2, $range3, ...)  spaced $n $intervals apart 
// where $range is an object ($range->start, $range->end) with duration $duration $duration_interval (e.g. 1 HOUR)
/** unsused - automatic comment added on  */
function sql_interval_series($start_date, $end_date, $n=1, $interval='DAY', $duration=0, $duration_interval='HOUR') {
  global $db;
  $series = array();
  // if(!$count) return $series;
  if($start_date >= $end_date) return $series;
  while(strip_time($start_date) <= strip_time($end_date)) {
    if($duration) {
      $range = NULL;
      $range->start = $start_date;
      $range->end   = add_interval($start_date, $duration, $duration_interval);      
      $series[] = $range;
    } else {
      $series[] = $start_date;
    }
    // if(!--$count) break; 
    $start_date = add_interval($start_date, $n, $interval);
  }
  return $series;
}


// return number of days between date1 and date2
/** unsused - automatic comment added on  */
function get_num_days($date1, $date2) { 
  global $db;
  $num_days = db_getOne("SELECT TO_DAYS('$date2')") - db_getOne("SELECT TO_DAYS('$date1')");
  return $num_days;
}

// returns array (1 -- 7);
/** unsused - automatic comment added on  */
function weekdays() {
  return array(WEEKDAY_MONDAY, WEEKDAY_TUESDAY, WEEKDAY_WEDNESDAY, WEEKDAY_THURSDAY, WEEKDAY_FRIDAY, WEEKDAY_SATURDAY, WEEKDAY_SUNDAY);
}

// returns name of weekday
/** unsused - automatic comment added on  */
function weekday($weekday) {
  
  //$wds = weekdays();
  //return $wds[$weekday];
  //return $_SESSION['weekday_names'][$weekday];
}

// returns the next weekday
function next_weekday($weekday) {
  return $weekday == WEEKDAY_SUNDAY ? WEEKDAY_MONDAY : $weekday + 1;
}

// counts how many of the days in array $days (1, 2, 3) occurs in the given time range
// does not count the $out day (as in checkout date)
function weekday_count($in, $out, $days) {
  if(!is_date(strip_time($in)) || !is_date(strip_time($out))) {
    mydie("weekday_count: $in or $out not valid date");
    return 0;
  }
  if($out < $in) return 0;
  $days = array_unique(array_intersect(weekdays(), $days));  
  $count = count($days);
  if(!$count) return 0; // no (valid) days  
  //asort($days); // sort
  
  $nd = period_diff($in, $out, 'd');
  if($count == 7) return $nd; // all days
  $whole_weeks = floor($nd / 7); // whole weeks
  $result = $count * $whole_weeks;
  $rest = $nd % 7;
  if(!$rest) return $result; // num days divides exactly in 7

  // dump($days);
  // echo("$in-$out = $nd days, input_weekday_count = $count whole weeks=$whole_weeks rest=$rest $first_day_in_rest = $weekday = ".weekday($weekday)." result=$result<br>");
  // loop through the rest days and count
  $first_day_in_rest = sql_add_days($in, 7*$whole_weeks);
  $weekday = get_sql_weekday($first_day_in_rest); 
  while($rest--) {
    if(in_array($weekday, $days)) $result++;
    //echo("weekday = ".weekday($weekday)." result=$result<br>");
    $weekday = next_weekday($weekday);
  }
  
  return $result;  
}

/** unsused - automatic comment added on  */
function num_days_text($days) {
  if ($days < -1) {$daysStr ="$days days ago";}
  elseif($days==-1) {$daysStr= "Yesterday";}
  elseif($days===0) {$daysStr= "Today";}
  elseif($days==1) {$daysStr= "Tomorrow"; }
  elseif ($days > 1) {$daysStr ="in $days days";}
  else {die("num_days_text: invalid or missing number of days: $days");}
  return $daysStr;
}

// checks if 2 date ranges intersect (at least 1 overlapping date)
// if intersect, return number of days overlap by default (can choose other - see period_diff())
function dates_intersect($start1, $end1, $start2, $end2, $period='d') {
  if(!($start1 && $end1 && $start2 && $end2)) return 0; // missing dates
  if(!($start1 < $end2 && $end1 > $start2)) return 0; // outside range
  return period_diff(max($start1, $start2), min($end1, $end2), $period);
}

// returns the fraction of the date range intersection count / range 1 count
function dates_intersect_fraction($start1, $end1, $start2, $end2, $period='d') {
  $count = period_diff($start1, $end1, $period);
  $intersect = dates_intersect($start1, $end1, $start2, $end2, $period);
  return $intersect/$count;
}

// returns 1-based array of month (1 = january) or weekday names (1 = monday)
// use option lang for language 
// use 'short' option for abbreviated version 
// returns UTF8 unless option html is set, in which case string is converted to html entities
// must add locales to server:
//
function datetime_names($type, $options=NULL) {
  global $__APP;
  $lang  = pick_first($options['lang'], $options['language'], $__APP->language, $_SESSION['language']);
  $short = $options['short'];
  $html  = $options['html'];
  switch($type) {
    case 'month':
      $range = range(1, 12);
      $format = $short ? '%b' : '%B';      
      break;
    case 'weekday':
      $weekday = true;
      $range = range(1, 7);
      $format = $short ? '%a' : '%A';
      break;
  }
  $locale = get_object('language', $lang, 'locale');

  if(!$locale) return array();

  $result = setlocale(LC_TIME, $locale);
  //echo("Setting locale to '$locale' result=".dump($result, true));
  if(!$result) return array();  
  foreach($range as $i) {
    $name =  strftime($format, $weekday ? mktime(0, 0, 0, 8, $i, 2011) : mktime(0, 0, 0, $i, 1, 2012)); // Agust 2011: Monday was on the 1st
    $names[] = $html ? htmlentities(utf8_decode($name)) : $name;
  }
  setlocale(LC_TIME, $_SESSION['locale']); // restore locale
  if($type == 'weekday') array_unshift($names, $names[6]); // copy Sunday to beginninng for comatability with Sunday=0 and Sunday = 7

  //echo("lang=$lang locale=$locale sesion:{$_SESSION['locale']} names=".dump($names,true));
  return $names;
}

// takes comma spearated list of weekday (1-6), returns locale-specific string (monday, wednesday)
/** unsused - automatic comment added on  */
function weekdays2str($days) {
  $dayAr = explode(',', $days);
  foreach($dayAr as $day) {
    $dayStrAr[] = weekday_name($day);
  }
  return implode(', ', $dayStrAr);
}


function weekday_name($wd) {
  $wdn = weekday_names();
  return $wdn[$wd];
}

// returns 1-based array of weekday names (1 = monday)
function weekday_names($options=NULL) {
  return datetime_names('weekday', $options);
}

// returns 1-based array of weekday names (1 = monday)
function month_names($options=NULL) {
  return datetime_names('month', $options);
}


function month_name($monthNum, $short=NULL) { // 1=january
  $month_names = month_names();
  return $month_names[$monthNum -1 ];
  
  /** haven't tested if this works for other languages */
  // http://stackoverflow.com/questions/18467669/convert-number-to-month-name-in-php
  $dateObj   = DateTime::createFromFormat('!m', $monthNum);
  $format = $short ? 'M' : 'F';
  return $dateObj->format($format);
}

// returns a date array of array('start_date' => $in, 'end_date' => $out) based on in / out
// used for monthly invoicing
// does not care about existing invoices (that's handled by invoice_date_array)
function monthly_date_array($in, $out) {	
  $debug = true;
  $dateArray = array();
  $in = strip_time($in);
  $out = strip_time($out);
  if(!$in || !$out || $in > $out) return array();
  
  list($num_days, $num_months) = count_month_days($in, $out);  // number of months + days

  $m_in = $m_out = $in;     
  while($num_months--) {            
    $in_plus_one_month = date("Y-m-d", strtotime(date("Y-m-d", strtotime($m_in))." +1 month"));
    $m_out = min($out, $in_plus_one_month);
    $dateArray[]= array('start_date' => $m_in, 'end_date' => $m_out, 'num_days' => period_diff($m_in, $m_out, 'd'), 'days_in_month' => get_daysofmonth($m_in));
    $m_in = $m_out;
  }
  if($m_out < $out) {
    $dateArray[] = array('start_date' => $m_out, 'end_date' => $out, 'num_days' => period_diff($m_out, $out, 'd'), 'days_in_month' => get_daysofmonth($m_out)); // we're done
  }
  return $dateArray;
}

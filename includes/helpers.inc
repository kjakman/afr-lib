<?php

/** functions used in set_env */

/** parse and return main config file */
function config_load() {
  $config_file = approot(true)."config/main.yml";  
  if(!is_file($config_file)) die("Did not find config file {$config_file} app:{$approot}");
  
  return yaml_parse_file($config_file);
}

function get_config($key, $site_id = -1) {
  if($site_id < 0) $site_id = $_SESSION['site_id'];
  $configAr = $_SESSION['config'];
  $password_file = $configAr['settings']['config_file'];
  if(!$password_file) die("get_config: secret file not set");  
  $password_file = approot(true).$password_file;
  if(!$password_file) die("get_config: did not find secret file {$password_file}");
  
  $str = file_get_contents($password_file);
  if(!$str) mydie("Failed to read secret file: {$password_file}");
  $config = json_decode($str);
  if(!$config) die("Failed to parse secret file: {$password_file}");
  return $site_id ? $config->sites->$site_id->$key : $config->$key;
}

function env_is_online() {
  return (checkdnsrr('google.com', 'ANY') && checkdnsrr('microsoft.com', 'ANY'));
}

function env_log_level($deployment) { // logging level
  switch($deployment) {
    case 'live': return LOG_LEVEL_LIVE;
    case 'dev': return LOG_LEVEL_DEV;
    case 'test':
    default:
      return LOG_LEVEL_TEST;
  }
}


/** check if URL is minisite - todo: use file cache */
function env_minisite($domain, $uri) {  
  $parts = array_filter(explode('/', $uri));
  $path = array_shift($parts);
  print_log("Looking for minisite wih uri: {$uri} domain: {$domain}", 'minisite', LOG_LEVEL_TEST);
  
  if($msObj = find_object('minisite', ['domain' => $domain])) {
    print_log("Found minisite domain: {$domain}", 'minisite', LOG_LEVEL_TEST);
    return $msObj;
  } else if($msObj = find_object('minisite', ['url' => $path])) {
    print_log("Found minisite uri {$path}", 'minisite', LOG_LEVEL_TEST);
    $msObj->internal = 1;
    return $msObj;
  }
  return null;
  
  /** old code */
  $internal = $domain == 'vpatina.com' ? 1 : 0;
  if($domain == 'stigtervandoesburg.com' || strpos($uri, '/stigtervandoesburg') === 0) {
    $minisite_basepath = $internal ? '/stigtervandoesburg' : '';    
    return ['internal' => $internal, 
            'site_id' => 247, 
            'language' => 'en', 
            'minisite' => "stigtervandoesburg", 
            'logo' => 'stigter_van_doesburg_logo.png', 
            'minisite_name' => 'Stigter van Doesburg', 
            'admin_user_id' => 20361, 
            'user_id' => 20361, 
            //'header_text' => 'Gallery closed for Summer. Reopens Saturday September 9th.',
            'type' => 'gallery',
            'social_links' => ['facebook' => 'https://www.facebook.com/stigtervandoesburg/', 'instagram' => 'https://www.instagram.com/explore/locations/285426524/stigter-van-doesburg/?hl=en'],
            'basepath' => $minisite_basepath];
    // 'home' => 'stigtervandoesburg/home', 
  }

  if($domain == 'kerrygroneng.com' || strpos($uri, '/kerrygroneng') === 0) {
    $minisite_basepath = $internal ? '/kerrygroneng' : '';    
    return ['internal' => $internal, 'site_id' => 247, 'language' => 'en', 'minisite' => "kerrygroneng", 'minisite_name' => 'Kerry Grøneng', 'admin_user_id' => [20049,17391], 'user_id' => 20049, 'type' => 'artist', 'basepath' => $minisite_basepath];
  }
  if($domain == 'gallerycarrie.com' || strpos($uri, '/gallerycarrie') === 0) {
    $minisite_basepath = $internal ? '/gallerycarrie' : '';    
    return ['internal' => $internal, 'site_id' => 247, 'language' => 'en', 'minisite' => "gallerycarrie", 'minisite_name' => 'Gallery Carrie', 'admin_user_id' => [20049,17391], 'user_id' => 17391, 'type' => 'gallery', 'basepath' => $minisite_basepath];
  }
  return [];
}


function reverse_route($app, $routeName, $params) {
  $router = $app->router;
  return $router->generate($routeName, $params);  
}

/** todo: replace by proper router */
function env_route($uri) {
  if($uri == '/') return ['/', '', '', '']; // home page

  $subview = '';
  $parts = pathinfo($uri);
  $ext = $parts['extension'];
  if($ext) return '';
  $dir = $parts['dirname'];
  $base = $parts['basename'];
  
  print_log("Route Uri:{$uri} dir:{$dir} Base={$base}", 'app', LOG_LEVEL_TEST);    

  $backend = strpos($uri, '/backend') === 0;
  if($backend) {

    print_log("backend Uri:{$uri} dir:{$dir} Base={$base}", 'app', LOG_LEVEL_TEST);
    
    return ["{$dir}/", 0, '', $base];
  }

  
  // only happens if uri has at least three parts /foo/bar/quux = subview
  // or dir is in sidenavs
  if(strpos(trim($dir,'/'), '/')) { 
    $subview = $base;
    $parts = pathinfo($dir);
    $dir = $parts['dirname'];
    $base = $parts['basename'];
    print_log("Subview: Uri:{$uri} dir:{$dir} Base={$base}", 'app', LOG_LEVEL_TEST);
  }
  
  print_log("\n\nUri:{$uri} dir:{$dir} Base={$base}", 'app', LOG_LEVEL_TEST);
  //if(!$base) return $dir == '/' ? $dir : "{$dir}/";
  
  $w = 'a-zA-Z0-9-';
  $pattern = "#^([$w]+\~)?([0-9\-]+)$#";
  $id = '';
  $name = '';
  $path = '';
  
  if($dir != '/' && preg_match($pattern, $base, $m)) { // contains ID
    $path = $dir == '/' ? "/" : "{$dir}/";
    $name = rtrim($m[1], '~');
    $id = $m[2];
    //$subview = $m[3];
  } else {
    $path = $dir == '/' ? "/{$base}/" : "{$dir}/{$base}/";
  }
  return [$path, $id, $name, $subview];  
}
/** end functions used in set_env */

// PHP_VERSION_ID is available as of PHP 5.2.7, if our 
// version is lower than that, then emulate it
if (!defined('PHP_VERSION_ID')) {
  $version = explode('.', PHP_VERSION);
  define('PHP_VERSION_ID', ($version[0] * 10000 + $version[1] * 100 + $version[2]));
}
             
/** pass name to start time, otherwise prints time elapsed since start to log file with name */
function timer($name, $where = '', $start=false) {
  if($start) {
    $_SESSION['timer'][$name]['start'] = $ts = microtime(true);
    return $ts;
  }
  
  $timer = $_SESSION['timer'][$name];
  if(!$timer) return;
  $ts = $timer['start'];
  print_log("Timer {$name}:".round(microtime(true) - $ts,3)." ({$where})", $name, LOG_LEVEL_TEST);    
}
      
function elapsed($start) {
  return round(microtime(true) - $start,3);
}

function redirect_secure($url = '') {
  if(!$url) $url = this_url();
  if($_SERVER['HTTP_X_FORWARDED_PROTO'] != 'https' &&  strpos($url, 'https://') === false) {
    $secure_url = str_replace('http://', 'https://', $url);
    redirect($secure_url);
  }
}

/** unsused - automatic comment added on  */
function html2txt($str) {
  return br2nl($str);
}

function br2nl($string) {
  return preg_replace('/\<br(\s*)?\/?\>/i', "\n", $string);
} 

/** unsused - automatic comment added on  */
function txt2html($str) {
  return nl2br(trim($str), false);
  //return str_ireplace(array("\n", "\r", "\r\n"), "<br>", trim($str));
}

// list of all 333 Dutch infixes (tussenvoegsels) sorted by longest first;
/** unsused - automatic comment added on  */
function infix_list() {
  return ["de van der","uijt te de","van van de","voor in 't","de die le","onder den","onder het","uit te de","van de l’","voor in t","boven d’","onder 't","onder de","over den","over het","uijt den","uijt ten","van de l","voor den","aan den","aan der","aan het","auf dem","auf den","auf der","auf ter","aus dem","aus den","aus der","bij den","bij het","boven d","onder t","over 't","over de","uijt 't","uijt de","uit den","uit het","uit ten","van den","van der","van gen","van het","van ter","von dem","von den","von der","voor 't","voor de","vor der","aan 't","aan de","aus 'm","bij 't","bij de","de die","de las","die le","in den","in der","in het","op den","op der","op gen","op het","op ten","over t","uit 't","uit de","van 't","van de","van la","von 't","aan t","am de","aus m","bij t","dalla","de l’","de la","de le","degli","della","in 't","in de","onder","op 't","op de","uit t","unter","van t","von t","dal’","de l","deca","in t","op t","over","thoe","thor","uijt","voor","aan","auf","aus","ben","bij","bin","dal","das","dei","del","den","der","des","don","dos","het","las","les","los","ten","ter","tho","toe","tot","uit","van","ver","vom","von","vor","zum","zur","'s","'t","af","al","am","d’","da","de","di","do","du","el","im","in","l’","la","le","lo","of","op","s’","te","to","zu","a","d","i","l","s","t"];
}

function lang2locale() {
  switch($language) {
    case 'de':return 'de_DE.UTF8';
    case 'en':return 'en_US.UTF8';
    case 'es':return 'es_ES.UTF8';
    case 'fr':return 'fr_FR.UTF8';
    case 'it':return 'it_IT.UTF8';
    case 'ja':return 'ja_JP.UTF8';
    case 'nl':return 'nl_NL.UTF8';
    case 'no':return 'nb_NO.UTF8';
    return '';
  }
}

/** unsused - automatic comment added on  */
function underscore2camel($str) {
  $str = str_replace('.', 'DOT', $str); // otherwise invoice.id and invoice_id conflict
  $str = preg_replace("/[^A-Za-z0-9_]/", '_', $str); // only alphanumeric and underscore
  $words = explode('_', strtolower($str));
  foreach ($words as $word) $return .= ucfirst(trim($word));
  return lcfirst($return); // headless camel
}

function camel2underscore($str) {
  return strtolower(preg_replace('/([a-z])([A-Z])/', '$1_$2', $str));
}

// like isset, but also returns true if key exists and is NULL
function myisset($haystack, $key) {
  if(isset($haystack[$key])) return true;
  if(is_object($haystack)) $haystack = (array) $haystack;
  if(is_array($haystack) && key_exists($key, $haystack)) return true;  
}

// return input if not empty, output otherwise
/** unsused - automatic comment added on  */
function return_if_empty($input, $output) {
  if($input === NULL || !strlen($input)) return $output;
  return $input;
}

function print_debug($str) {	 
  if(!$_SESSION['debug']) return;
  print_log($str, 'debug', LOG_LEVEL_DEBUG);
}

function print_log($str, $logfile=0, $level=0) {
  if(!$_SESSION['log_dir']) return 0;
  if(!$logfile) $logfile = $_SESSION['system_log_file'];
  if($level > $_SESSION['log_level']) return;
  if(($level == LOG_LEVEL_SEVERE || $level == LOG_LEVEL_ERROR) && $logfile != 'error') print_log($str, 'error', $level); // log all errors to separate error log

  if(is_array($str) || is_object($str)) $str = print_r($str, true);
  if(!strpos($logfile, '.')) {
    if($_SESSION['deployment'] && $_SESSION['deployment'] != 'live') $logfile .= '-'.$_SESSION['deployment'];
    $logfile .= '.txt'; // no extension, use 'txt'
  }
  if(!strpos($logfile, '/')) $logfile = $_SESSION['log_dir'].$logfile;  // no path, use standard log dir    
  $str = rtrim(html2txt($str)).PHP_EOL;

  $f=@fopen($logfile,'a');
  if (!$f) {
    if(!$_SESSION['live']) {
      return false;
      echo("Could not open $logfile for writing dir={$_SESSION['log_dir']}<br>").caller_stack();
      echo(dump($str,true));
      echo(print_r(error_get_last(),true));
      //die();
    }
    return false;
  }
  $b = fwrite($f,$str);
  fclose($f);
  
  return $b; // returns number of bytes written
}

function now() {
  return date('Y-m-d H:i:s');
}

function today($format = 'Ymd', $sep = '-') {  
  return date($format[0].$sep.$format[1].$sep.$format[2]);
}

// cache (APC)
// system cache
function system_cache_store($apc_key, $data, $expires = 24*3600) {
  if(!$_SESSION['system_cache']) return null;
  global $redis;
  if($result = $redis->set($apc_key, json_encode($data), $expires)) {
    print_log(now().": Stored $apc_key\n", 'cache', LOG_LEVEL_TEST);
  } else {    
    print_log(now().": Failed to store $apc_key\n", 'cache', LOG_LEVEL_TEST);
  }
  return $result; // true on success
}

/** unsused - automatic comment added on  */
function system_cache_fetch($apc_key) {
  if(!$_SESSION['system_cache']) return null;
  global $redis;
  $result = $apc_key ? $redis->get($apc_key) : null;
  if($result) print_log(now().": Fetched $apc_key\n", 'cache', LOG_LEVEL_TEST);
  return json_decode($result);
}


function system_cache_delete($apc_key) {
  if(!$_SESSION['system_cache']) return null;
  global $redis;
  print_log(now().": Deleted $apc_key\n", 'cache', LOG_LEVEL_TEST);
  return $apc_key ? $redis->delete($apc_key) : false;
}


// remote string $strip from end of $message
/** unsused - automatic comment added on  */
function strrtrim($message, $strip) {      
    $lines = explode($strip, $message); // break message apart by strip string 
    $last  = ''; 
    // pop off empty strings at the end 
    do { 
      $last = array_pop($lines); 
    } while (empty($last) && (count($lines)));      
    return implode($strip, array_merge($lines, array($last))); // re-assemble what remains 
} 

// quick and dirty check to guess if country code is present
/** unsused - automatic comment added on  */
function phone_valid_idd($num) {
  $num = str_replace(array(' ', '-', '(', ')'), '', trim($num));
  
  if(strpos($num, '+') === false) return false;
  if(strlen(ltrim($num, '+')) < 10) return false;   
  return true;
}

// returns a number properly formatted for sending as SMS
function clean_phonenumber($num) {
  $num = str_replace(array(' ', '-', '(', ')', '+','.'), '', $num);
  if(strpos($num, '00') === 0) $num = substr($num, 2); // remove leading 00 (int'd dialing prefix)
  return $num;
}

/** unsused - automatic comment added on  */
function phone_make_idd($num, $cc) {
  if(!trim($num)) return '';
  if(phone_valid_idd($num)) return $num;
  $num = clean_phonenumber($num);
  return "+$cc ".ltrim($num, 0);
}

// takes a string, adds a target='$target' attribute to all links.
// assumes target is not already set
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_href_target($str, $target='_new') {
  return str_replace(' href=', ' target="status" href=', $str);
}
  

function add_host($url, $host='') {
  if(!$host) $host = $_SERVER['HTTP_HOST'];
  return strpos($url, $host)===false ? add_http($host.$url) : $url;
}

/** unsused - automatic comment added on  */
function js_stripslashes($str) {
	return str_replace(array("'", '"', "\n", "\r\n", "\r"), array("&#39;", "&#34;", " ", " ", " "), $str);
}

function fix_color($hex_color) {
  return strlen($hex_color) == 3 ? strtoupper($hex_color[0].$hex_color[0].$hex_color[1].$hex_color[1].$hex_color[2].$hex_color[2]) : strtoupper($hex_color);
}

function fix_break($break) {
  if(!$break) return $break;
  if(in_array(strtolower($break), array('<br/>', '<br />', '<br>'))) return html_break(); // returns correct break depending on xhtml settings
  return $break;
}

function isint($mixed) {
  if(is_object($mixed) || is_array($mixed)) return false;
  return (string) (int) $mixed === (string) $mixed; // quicker ? 
  return ( preg_match( '/^\d*$/'  , $mixed) == 1 );
}

// used to split house number + extension into two strings
function number_extension_split($string) {
  $string = trim($string);
  if(!$string) return array('','');
  $pattern = "/^([0-9]+)[\s\-]*(.*)/";
  $result = preg_match($pattern, $string, $m);
  //dump($m);
  return array($m[1], $m[2]);
  
}

// takes an array of rows (array), returns correpsonding array of columns (array)
/** unsused - automatic comment added on  */
function rows2cols($rows, $glue='') {
  $glue = fix_break($glue);
  foreach($rows as $row) {
    $i=0;
    foreach($row as $cell) {
      $columns[$i][] = $cell;
      $i++;
    }
  }  
  foreach($columns as $col) $cols[] = $glue ? implode($glue, $col) : $col;
  return $cols;
}

function color_hex2dec($hex_color) {
  $hex_color = fix_color($hex_color);
  $r = hexdec($hex_color[0].$hex_color[1]);
  $g = hexdec($hex_color[2].$hex_color[3]);
  $b = hexdec($hex_color[4].$hex_color[5]);
  return array($r,$g,$b);
}


/** unsused - automatic comment added on  */
function hide_local($city_id) {
  //echo("session aff_id=$_SESSION[aid] aff_hide_local=$_SESSION[aff_hide_local]<br>");
  if($_SESSION['aid'] && !$_SESSION['aff_hide_local']) return false;
  if(!$city_id) return false;
  //echo("hide_local: city_id=$city_id<br>");
  $aptCountryObj = loctree_grandparent_object('city', $city_id);
  $user_country = $_SESSION['country_code'];
  //echo("user_country=$user_country, apt_country=$aptCountryObj->code<br>"); // if user is in same country as these apartments, turn on hide local
  return $user_country == $aptCountryObj->code; // if user is in same country as these apartments, turn on hide local
}

/** unsused - automatic comment added on  */
function get_script() {
  $script_array = explode("/", $_SERVER['SCRIPT_FILENAME']);
  $script1 = array_pop($script_array);
  $script2 = array_pop($script_array);
  return $script2 =='home' ? $script1 : "/$script2/$script1";
}

// strips all white space and tags from a text input field
function strip_blank($val) {
  $val = str_replace('&nbsp;', '', $val);
  return preg_replace('/\s+/', '',strip_tags(trim($val)));
}

function strip_style($str) {
  return strip_attribute($str, 'style');
}

function strip_attribute($str, $attribute) {
  $str = preg_replace('/ '.$attribute.'\s?=\s?"([^"]*)"/i', '', $str);
  $str = preg_replace("/ {$attribute}\s?=\s?'([^']*)'/i", '', $str);
  return $str;
}
  
/** text from db to be printed on screen */
function safe_text($str) {
  if(!strlen(strip_blank($str))) return '';
  return strip_style(strip_tags($str, '<p><br><a><b><em><i><h1><h2><h3><h4><li><ul><ol><hr><div>'));
}

function strip_quotes($val) {
  return trim($val,"\x22\x27");
}

function strip_query_string($url='', $remove_hash=true) {
  if(!$url) $url = $_SERVER['REQUEST_URI'];  
  $pos = strpos($url, '?');
  $url = $pos === FALSE ? $url : substr($url,0, $pos);
  if(!$remove_hash) return $url;
  $pos = strpos($url, '#');
  return $pos === FALSE ? $url : substr($url,0, $pos);
}

/** unsused - automatic comment added on  */
function plural($term, $count, $case=0, $options=NULL) {
  return $count == 1 ? phrase($term, $case, $options) : phrase($term.'s', $case, $options);
}

function generate_dictionary($siteObj_or_id, $language='', $options = array()) {
  if($_SESSION['redis_cache']) die("generate_dictionary: no longer in use - we're using redis");
  
  if(!function_exists('any2obj')) {
    print_log("Model not loaded", 'dicitionary', LOG_LEVEL_TEST); 
    return;
  }
  if(!$language) $language = $_SESSION['language'];
  
  $siteObj = any2obj('site', $siteObj_or_id);
  $terms = find_objects('dictionary', 
            array('language' => $language, 'term' => ['!=', 'text_*'], 'site_id' => ['$or', [$siteObj->id, '$NULL']]), 
            array('sort' => 'phrase', 'debug_query' => 0));
  $now = now();
  $file = docroot()."dic/terms_{$language}.inc";
  $count = count($terms);
  $output = "Found $count terms in $language fore site {$siteObj->name}<br>";
  $file_output = "<?php\n/** Machine generated on {$now} - DO NOT EDIT */\n";
  $file_output .= '$__DICTIONARY = ['.PHP_EOL;
  if($count) {
    foreach($terms as $dicObj) {
      $phrase = addslashes($dicObj->phrase);
      $file_output .= "'{$dicObj->term}' => '{$phrase}',\n";
    }
    $file_output .= '];'.PHP_EOL;
    $bytes = write_file($file, $file_output);
    $bytes_h = bytes2human($bytes);
  }
  $output .= "Wrote {$bytes_h} to $file<br>";
  
  $terms = find_objects('dictionary', 
            array('language' => $language, 'term' => 'text_*', 'site_id' => ['$or', [$siteObj->id, '$NULL']]), 
            array('sort' => 'phrase', 'debug_query' => 0));
  
  $file = docroot()."dic/text_{$language}.inc";
  $count = count($terms);
  $output .= " Found $count texts in $language fore site {$siteObj->name}<br>";
  $file_output = "<?php\n/** Machine generated on {$now} - DO NOT EDIT */\n";
  $file_output .= '$__TEXT = ['.PHP_EOL;
  if($count) {
    foreach($terms as $dicObj) {
      $phrase = addslashes($dicObj->phrase);
      $file_output .= "'{$dicObj->term}' => '{$phrase}',\n";
    }
    $file_output .= '];'.PHP_EOL;
    $bytes = write_file($file, $file_output);
    $bytes_h = bytes2human($bytes);
  }
  $output .= "Wrote {$bytes_h} to $file<br>";
  
  print_log("{$now}: Generated dictionary for {$siteObj->name}", 'dictionary', LOG_LEVEL_ALWAYS);
  return $output;  
}

/** new search/replace using sprintf: use 4th, 5th, etc arugments afer options[] */
function phrase($term, $case=-1, $options=NULL) {
  global $__TEXT, $__DICTIONARY, $__SITE, $__LIVE_DICTIONARY, $u_id;
  
  $case1 = $case;
  if($case == -1) { // use same capitalization as $term
    if(strtolower($term) == $term) $case = LOWERCASE;
    else if(strtoupper($term) == $term) $case = UPPERCASE;
    else if(ucfirst($term) == $term) $case = CAPITALIZE;
  }
    
  if(is_array($options)) extract($options); 
  $lang = pick_first($language, $lang, $_SESSION['language'], $__SITE->default_language, 'en');
  $site_id = $site_id ?: $__SITE->id; /** might be extracted from options */
  $user_id = $u_id ?: 0;

  $org_term = trim($term); // used for entering into dictionary
  $now = now();
  
  $debug = !$__LIVE;
  
  $term = strtolower(trim(str_replace(' ', '_', $term)));
  if(!$term) return '';
  
  $phrase = '';
  
  if($_SESSION['redis_cache']) {
   global $redis;
   $redis_key = "dictionary-0-{$lang}-{$term}";
   $redis_site_key = "dictionary-{$site_id}-{$lang}-{$term}";
   $phrase = $redis->get($redis_site_key) ?: $redis->get($redis_key);
  } 
    
  $add_redis = 0;
  if($phrase) {
    //if($debug) print_log("found $term for $lang in redis", 'dictionary', LOG_LEVEL_TEST);
  } elseif($phrase = db_getOne("SELECT phrase FROM dictionary WHERE term='$term' AND site_id in (0, '$site_id') ORDER BY site_id DESC limit 1")) { // couldn't find it, add to dictionary
    if($debug) print_log("found $term for $lang in db, not redis, adding to redis", 'dictionary', LOG_LEVEL_TEST);
    $add_redis = 1;
    
    /** add to redis */
  } else { // couldn't find it, add to dictionary
    if($debug) print_log("did not find $term for $lang, adding to db and redis", 'dictionary', LOG_LEVEL_TEST);
    /** add to redis and db */

    $id = nextId('dictionary');
    $phrase = str_replace('_', ' ', $org_term);

    db_query("INSERT into dictionary (id, term, language, phrase, accessed, site_id, user_id) VALUES ($id, '$term', '$lang', '$phrase', '$now', '$site_id', 0)");
    $add_redis = 1;
  }                      

  if($_SESSION['redis_cache'] && $add_redis) {
    $phrase = str_replace('_', ' ', $org_term);
    $redis_site_key = "dictionary-{$site_id}-{$lang}-{$term}";
    $redis->set($redis_site_key, $phrase);
  }   
  
  if(strpos($phrase, '%') !== false) {
    $numargs = func_num_args();
    if($numargs > 2) {
      $args = array_slice(func_get_args(), 2);
      array_unshift($args, $phrase);
      $phrase = call_user_func_array('sprintf', $args);
      $args = array_slice(func_get_args(), 2);
    }
  }
  
  if($case > -1) {
    if($case == CAPITALIZE) $phrase =  mb_ucfirst($phrase);
    else $phrase = mb_convert_case($phrase, $case, "UTF-8");
  }
  
  // new search/replace using sprintf
  

  // old search and replace
  $search = isset($options['search']) ? $options['search'] : '';
  $replace = isset($options['replace']) ? $options['replace'] : '';
  if($search && $replace) $phrase = str_replace($search, $replace, $phrase);

  if($_SESSION['log_dictionary']) {
    $now = now();
    db_query("UPDATE dictionary SET accessed='$now' WHERE term='$term' AND language='$lang'");
  }
    
  return $phrase;
}

// http://php.net/manual/en/function.ucfirst.php
// Simple multi-bytes ucfirst():
function mb_ucfirst($str) {
  $fc = mb_strtoupper(mb_substr($str, 0, 1));
  return $fc.mb_substr($str, 1);
}

// analyze a string byte for byte to examine encoding
/** unsused - automatic comment added on  */
function analyze($phrase, $bytes_per_char=1) {
  $len = strlen($phrase);
  echo("<h3>phrase = '$phrase' ($len bytes)</h3>");
  for($i=0; $i < $len; $i+=$bytes_per_char) {
    $char='';
    for($j=0;$j < $bytes_per_char; $j++) {
      $char .= $phrase[$i+$j];
    }
    $hex = bin2hex($char);
    echo("<b>char at $i = '$char'</b> hex: '$hex'<br>");
  }
}

function get_sections() {
  return array('General'    =>'general',
               'Site'       =>'site',
               'Property'   =>'property',
               'Text'       =>'text',
               'Relocation' =>'relocation',
               'Geographic' =>'geographic',
               'Template'   =>'template',
               'URLs'       =>'url',
               'City Guide' =>'guide',
               'Quotes'     =>'quote',
               'Other'      =>'');
}

// returns true if needle is contained in any member of ar
/** unsused - automatic comment added on  */
function in_array_strpos($haystack, $ar) {
  foreach($ar as $needle) {
    if(strpos($haystack, $needle) !== false) return true;
  }
  return false;
}

function in_array_case_insenitive($needle, $ar) {
  return count(preg_grep("/{$needle}/i" , $ar)) ? true : false;
}

// returns associate array where keys and values are reversed from input
/** unsused - automatic comment added on  */
function inverse_array($array) {
  $inv_array = array();
  foreach($array as $k => $v) $inv_array[$v] = $k;
  return $inv_array;
}

// returns associate array both keys and values are the values of input array
/** unsused - automatic comment added on  */
function assoc_array($array) {
  $assoc_array = array();
  foreach($array as $k => $v) $assoc_array[$v] = $v;
  return $assoc_array;
}

// returns dictionary for given lang (string) or langs (array) or all (blank)
// format 1 (multi-lingual): array['term'] = 'phrase';
// format 2 (one lang)     : array['term']['lang'] = 'phrase';

function get_dictionary($lang="", $section="") {
  global $db;
  if(!$lang) $langs = $_SESSION['language'];
  if($section == 'all') $section = get_sections(); // get all active
  $section_clause = is_array($section) ? "section IN ('".implode("','",array_values($section))."')" : "section='$section'";
  $qs = "SELECT term, phrase, array FROM dictionary WHERE language='$lang' AND $section_clause";
  //echo("getting dic for lang='$lang' and sec='$section'<br>$qs<hr>");
  $rows = db_getAll($qs);
  foreach($rows as $dic_row) {
    $dic[$dic_row->term] = is_array($dic_row) ? explode('|', $dic_row->phrase) : $dic_row->phrase;
  }
  return $dic;
}

function get_dictionary_old($lang="", $section="") {
  global $apt_path;
  if($lang) $langs = $_SESSION['language'];

  if($section) $sections = is_array($section) ? $section : array($section);
  else $sections = get_sections(); // get all active

  //echo("getting dic for lang='$lang' and sec='$section'<br>");

  $dic = array();
  foreach($sections as $section) {
    $file = "$lang/phrases_".$section.'.inc';
    $pfile = "$apt_path/include/$file";
    if(file_exists($pfile)) {
      include($file);
      $this_dic = $phrases[$section];
      //echo("found '$pfile' - getting dic for lang code='$lang' and sec='$section' file='$pfile'<br>");
    } else {
      //echo("did not find '$pfile' - getting dic for lang code='$lang' and sec='$section' file='$pfile'<br>");
      $this_dic = array();
    }
    $dic = array_merge($dic, $this_dic);
  }
  return $dic;
}

/** unsused - automatic comment added on  */
/*
function write_dictionary($phrases, $lang, $author=0, $script='', $section='') {
  foreach($phrases as $term => $phrase) {
    new_dictionary_entry($term, $phrase, $lang, $author, $script, $section);
  }
}
*/

function get_dictionary_entry($term, $lang='') {
  if(!$lang) $lang = $_SESSION['language'];
  if($_SESSION['log_dictionary']) {
    $now = now();
    db_query("UPDATE dictionary SET accessed='$now' WHERE term='$term' AND language='$lang'");
  }
  return db_getOne("SELECT phrase FROM dictionary WHERE term='$term' AND language='$lang'");
}

function new_dictionary_entry($term, $phrase, $lang='en', $options=NULL) {
  print_log("{$now}: Adding term {$term} to dictionary for lang={$lang}", 'dictionary', LOG_LEVEL_ALWAYS);
  if(get_dictionary_entry($term, $lang)) {
    print_log("{$now}: {$term} is already in dictionary for lang={$lang}", 'dictionary', LOG_LEVEL_ALWAYS);
    return false;
  }
  $id = nextId('dictionary');
  $now = now();
  if($site_id = $options['site_id']) {
    db_query("INSERT into dictionary (id, term, language, phrase, accessed, site_id) VALUES ($id, '$term', '$lang', '$phrase', '$now', '$site_id')");
  } else {
    db_query("INSERT into dictionary (id, term, language, phrase, accessed) VALUES ($id, '$term', '$lang', '$phrase', '$now')");
  }
  return true;
}

  /*  
  global $u_id;
  global $u_level;
  global $db;
  //if(!$phrase) return;
  if(!$lang) $lang = $_SESSION['language'];

  //echo("new_dic called: $term [$lang]<BR>");


  if(get_dictionary_entry($term, $lang) !== FALSE) {
    //echo("new_dic: $term [$lang] already in dic<BR>");
    $fld_ar[] = "phrase='".addslashes($phrase)."'"; // manual update - not using db_object, so addslashes here
    if($author) $fld_ar[] = "author='$author'";
    if($script) $fld_ar[] = "script='$script'";
    if($section) $fld_ar[] = "section='$section'";
    if($array) $fld_ar[] = "array='$array'";

    //$qs = "UPDATE dictionary SET ".implode(", ", $fld_ar)." WHERE term='".addslashes($term)."' AND language='$lang'"; // update db-dictionary
    //echo("dic update qs:<br>$qs<hr>");
    // db_query($qs); // store result in db-dictionary
  } else {
    echo("new_dic: did not find $term [$lang]<BR>");die();
    $data['term'] = $term;
    $data['language'] = $lang;
    $data['phrase'] = $phrase;
    $data['author'] = $author;
    $data['section'] = $section;
    //$data['array'] = $array;
    list($id, $errors) = add_object('dictionary', $data);
    return $id;
    //$qs = "INSERT INTO dictionary (term, language, phrase, author, script, section, array) VALUES ('$term', '$lang', '$phrase', '$author', '$script', '$section', '$array')";
    //db_query($qs); // store result in db-dictionary
  }
*/


function phrases($terms, $case=0, $search='', $replace='') {
  $phrases = array();
  foreach($terms as $term) {
    $phrases[] = phrase($term, $case, $search, $replace);
  }
  return $phrases;
}


/** unsused - automatic comment added on  */
function get_case($word, $case) {
  if(!$case) return $word;
  switch ($case) {
    case CAPITALIZE:
      return ucfirst($word);
      break;
    case UPPERCASE:
      //return strtoupper_utf8($word);
      return strtoupper($word);
      break;
    case LOWERCASE:
      return strtolower($word);
      break;
    default:
      return $word;
      break;
    }
}

function strtoupper_utf8($string){
    $string=utf8_decode($string);
    $string=strtoupper($string);
    $string=utf8_encode($string);
    return $string;
}

// removes newlines, leading/trailing whitespace, tabs.
// escapes quotes and other chars that need escaping for DB
function dbtrim($str) {
  $str = str_replace(array("\n", "&nbsp;"), array("", " "), $str);
  return trim($str);
}

/** unsused - automatic comment added on  */
function is_domain($domain) {
  if(!trim($domain)) return false;
  $regex = '^([a-z0-9]([-a-z0-9]*[a-z0-9])?\\.)+((a[cdefgilmnoqrstuwxz]|aero|arpa)|(b[abdefghijmnorstvwyz]|biz)|(c[acdfghiklmnorsuvxyz]|cat|com|coop)|d[ejkmoz]|(e[ceghrstu]|edu)|f[ijkmor]|(g[abdefghilmnpqrstuwy]|gov)|h[kmnrtu]|(i[delmnoqrst]|info|int)|(j[emop]|jobs)|k[eghimnprwyz]|l[abcikrstuvy]|(m[acdghklmnopqrstuvwxyz]|mil|mobi|museum)|(n[acefgilopruz]|name|net)|(om|org)|(p[aefghklmnrstwy]|pro)|qa|r[eouw]|s[abcdeghijklmnortvyz]|(t[cdfghjklmnoprtvwz]|travel)|u[agkmsyz]|v[aceginu]|w[fs]|y[etu]|z[amw])$/i';  
  return (preg_match($regex, $domain)) ? true : false;
}

// returns the filename portion of url
// eg. http://www.foo.com/blah/quux,php?bar returns quux.php
/** unsused - automatic comment added on  */
function basename_url($url) {
  return array_shift(explode('?', basename($url)));
}

function is_url($url, $path_required=false) {
  return $path_required ? filter_var($url,FILTER_VALIDATE_URL,FILTER_FLAG_PATH_REQUIRED) : filter_var($url,FILTER_VALIDATE_URL);
}

/** unsused - automatic comment added on  */
function is_url_simple($url) {
  return preg_match('|^http(s)?://[a-z0-9-]+(.[a-z0-9-]+)*(:[0-9]+)?(/.*)?$|i', $url);
}

/** unsused - automatic comment added on  */
function url_exists($url){//se passar a URL existe
    $c=curl_init();
    curl_setopt($c,CURLOPT_URL,$url);
    curl_setopt($c,CURLOPT_HEADER,1);//get the header
    curl_setopt($c,CURLOPT_NOBODY,1);//and *only* get the header
    curl_setopt($c,CURLOPT_RETURNTRANSFER,1);//get the response as a string from curl_exec(), rather than echoing it
    curl_setopt($c,CURLOPT_FRESH_CONNECT,1);//don't use a cached version of the url
    $exists = curl_exec($c);
    if(!$exists) return false;    
    $httpcode=curl_getinfo($c,CURLINFO_HTTP_CODE);
    return $httpcode<400 ? $httpcode : false;
    
    // if(!curl_exec($c)){
    //     echo $url.' inexists';
    //     return false;
    // } else {
    //     echo $url.' exists';
    //     return true;
    // }
    // $httpcode=curl_getinfo($c,CURLINFO_HTTP_CODE);
    
    //return ($httpcode<400);
}

// valid ISO_3166-1_alpha-2 country code
/** unsused - automatic comment added on  */
function is_country($country) {
  return in_array($country, array('AD','AE','AF','AG','AI','AL','AM','AO','AQ','AR','AS','AT','AU','AW','AX','AZ','BA','BB','BD','BE','BF','BG','BH','BI','BJ','BL','BM','BN','BO','BQ','BR','BS','BT','BV','BW','BY','BZ','CA','CC','CD','CF','CG','CH','CI','CK','CL','CM','CN','CO','CR','CU','CV','CW','CX','CY','CZ','DE','DJ','DK','DM','DO','DZ','EC','EE','EG','EH','ER','ES','ET','FI','FJ','FK','FM','FO','FR','GA','GB','GD','GE','GF','GG','GH','GI','GL','GM','GN','GP','GQ','GR','GS','GT','GU','GW','GY','HK','HM','HN','HR','HT','HU','ID','IE','IL','IM','IN','IO','IQ','IR','IS','IT','JE','JM','JO','JP','KE','KG','KH','KI','KM','KN','KP','KR','KW','KY','KZ','LA','LB','LC','LI','LK','LR','LS','LT','LU','LV','LY','MA','MC','MD','ME','MF','MG','MH','MK','ML','MM','MN','MO','MP','MQ','MR','MS','MT','MU','MV','MW','MX','MY','MZ','NA','NC','NE','NF','NG','NI','NL','NO','NP','NR','NU','NZ','OM','PA','PE','PF','PG','PH','PK','PL','PM','PN','PR','PS','PT','PW','PY','QA','RE','RO','RS','RU','RW','SA','SB','SC','SD','SE','SG','SH','SI','SJ','SK','SL','SM','SN','SO','SR','SS','ST','SV','SX','SY','SZ','TC','TD','TF','TG','TH','TJ','TK','TL','TM','TN','TO','TR','TT','TV','TW','TZ','UA','UG','UM','US','UY','UZ','VA','VC','VE','VG','VI','VN','VU','WF','WS','YE','YT','ZA','ZM','ZW'));
}

function is_email($email,$allow_multiple = true) {
  if($allow_multiple && strpos($email,',')) {
    $emails = explode(',', $email);
    foreach($emails as $email) if(!is_email($email)) return false;
    return true;
  }
  return filter_var($email,FILTER_VALIDATE_EMAIL);
}

// returns true if url points to a different server
/** unsused - automatic comment added on  */
function is_remote($url) {
  $parts = parse_url($url);
  $host = $parts['host'];
  $local_host = $_SERVER['HTTP_HOST'];
  $is_remote =  $host && $host != $local_host;
  //if($is_remote) echo("$url is remote: host = $host, local=$local_host<br>");
  return $is_remote;
}

function client_ip() {
  if (!empty($_SERVER['HTTP_CLIENT_IP'])) {
    $ip = $_SERVER['HTTP_CLIENT_IP'];
  } elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {
    $ip = $_SERVER['HTTP_CF_CONNECTING_IP'];
  } elseif (!empty($_SERVER['HTTP_CF_CONNECTING_IP'])) {
    $ip = $_SERVER['HTTP_X_FORWARDED_FOR'];
  } else {
    $ip = $_SERVER['REMOTE_ADDR'];
  } 
  return $ip;
  //return isset($_SERVER['HTTP_CF_CONNECTING_IP']) ? $_SERVER['HTTP_CF_CONNECTING_IP'] : $_SERVER['REMOTE_ADDR'];  
}

function client_cc() {  
  $cc = isset($_SERVER['HTTP_CF_IPCOUNTRY']) ? $_SERVER['HTTP_CF_IPCOUNTRY'] : $_SERVER['GEOIP_COUNTRY_CODE'];
  if(!$cc && function_exists('geoip_record_by_name')) $cc = geoip_country_code_by_name($_SERVER['REMOTE_ADDR']);
  return $cc;
}

// Recognizes ftp://, ftps://, http:// and https:// in a case insensitive way.
function add_http($url, $https=false) {
  $protocol = $https ? 'https' : 'http';
  return preg_match("~^(?:f|ht)tps?://~i", $url) ? $url : "{$protocol}://".ltrim($url, '/');
}

/** unsused - automatic comment added on  */
function add_https($url) {
  return add_http($url, true);
}

/** unsused - automatic comment added on  */
function strip_http($url) {
  if(preg_match("~^(?:f|ht)tps?://~i", $url)) {
    $pos = strpos($url, '://');
    $url = substr($url, $pos+3);
  }
  return $url;
}

// the following is meant for internal links only
/** unsused - automatic comment added on  */
function add_http_internal($url) {
  $host_protocol = $_SERVER['HTTPS'] == 'on' ? "https" : "http";
  if(strpos(strtolower($url), "$host_protocol://") !== 0) return "$host_protocol://$url";
  return $url;
}

function email_string($email, $name = '') {
  return $name ? "$name <$email>" : $email;
}

// from http://www.linuxjournal.com/article/9585
// from ILoveJackDaniels
/** unsused - automatic comment added on  */
function check_email_address($email) {
  // First, we check that there's one @ symbol, 
  // and that the lengths are right.
  if (!ereg("^[^@]{1,64}@[^@]{1,255}$", $email)) {
    // Email invalid because wrong number of characters 
    // in one section or wrong number of @ symbols.
    return false;
  }
  // Split it into sections to make life easier
  $email_array = explode("@", $email);
  $local_array = explode(".", $email_array[0]);
  for ($i = 0; $i < sizeof($local_array); $i++) {
    if(!ereg("^(([A-Za-z0-9!#$%&'*+/=?^_`{|}~-][A-Za-z0-9!#$%&?'*+/=?^_`{|}~\.-]{0,63})|(\"[^(\\|\")]{0,62}\"))$", $local_array[$i])) {
      return false;
    }
  }
  // Check if domain is IP. If not, 
  // it should be valid domain name
  if (!ereg("^\[?[0-9\.]+\]?$", $email_array[1])) {
    $domain_array = explode(".", $email_array[1]);
    if (sizeof($domain_array) < 2) {
        return false; // Not enough parts to domain
    }
    for ($i = 0; $i < sizeof($domain_array); $i++) {
      if(!ereg("^(([A-Za-z0-9][A-Za-z0-9-]{0,61}[A-Za-z0-9])|?([A-Za-z0-9]+))$", $domain_array[$i])) {
        return false;
      }
    }
  }
  return true;
}

/** unsused - automatic comment added on  */
function html2specialchars($str){
   $trans_table = array_flip(get_html_translation_table(HTML_ENTITIES));
   return strtr($str, $trans_table);
}

/** unsused - automatic comment added on  */
function searchboxes($searchArray, $show_title=true) {
  $fieldset = html_fieldset($searchArray, 'fieldset_top');
  $output .= html_form($fieldset);
  return $output;
}

// takes an asso  c. array of field => name pairs
// returns an array of links used to sort columns with given field type and name
function sortlinks($sortArray, $show_title=true) {
  if(!is_array($sortArray)) $sortArray=array($sortArray);
  $srt = $_GET['srt'] ?: 'priority';
  foreach($sortArray as $fld => $name) {
    
    $tfld = trim($fld);
    if(!strlen($tfld)) {
      $sortlinks[] = $name;
    } else if($srt==$fld) {
      $sortlink = html_b($name);
      if(!in_array($srt, array('priority', 'review_score DESC'))) { // show sort direction button
        $sort_link = replace_query_var($_SERVER['REQUEST_URI'],"rev", ($_GET['rev'] ? 0 : 1)); 
        $src = $_GET['rev'] ? '/images/buttons/arrow-down-afr.png' : '/images/buttons/arrow-up-afr.png';
        $sort_dir = $_GET['rev'] ? phrase('ascending', LOWERCASE) : phrase('descending', LOWERCASE);
        $sortlink .= html_image_link($sort_link, $src, phrase('sort', CAPITALIZE)." ".strtolower($name)." $sort_dir");
      }
      $sortlinks[]= $sortlink;
    } else {
      $sort_link = replace_query_var($_SERVER['REQUEST_URI'],"srt", $fld);
      $sortlinks[]= html_link($sort_link, $name);
    }
  }
  return $sortlinks;
}

/** unsused - automatic comment added on  */
function sortbar($sortArray, $show_title=true, $glue=' ') {
  if($show_title) $title = html_b(phrase('sort_by', CAPITALIZE)).": ";
  $sortlinks = sortlinks($sortArray);
  return $title.implode($glue, $sortlinks);
}
 
// replace & with &amp; etc, no double encoding   
/** unsused - automatic comment added on  */
function escape_uri($is) {
  return htmlspecialchars($is, ENT_COMPAT, 'UTF-8', false);
}


// returns true if user is permitted to see given reservation
/** unsused - automatic comment added on  */
function auth_guest($guest) {
   if(!($u=$_SESSION[loggedin])) {return false;}
   elseif($u['user_level']>= 30) {return true;}
   elseif($u['user_level']>= 20) {return false;} // todo: fix, let owner see reservation for that guest for own apartments}
   else { // guest
      return $guest==$u['user_id'];
   }
}

//function dump($obj, $return=false, $options=NULL) {
function dump($obj, $return=false, $php=false) {
  if($php && (is_object($obj) || is_array($obj))) { // print so it can copy/pasted sa PHP code
    foreach($obj as $key => $val) $copy[quote($key)] = quote($val).',';
    $obj = $copy; // test that we don't fuck up here
    //dump($obj);
  }  
  $dump = '<pre>'.print_r($obj, true).'</pre>';
  if($php) $dump = str_replace(array(" ['", "'] => "), array(" '", "' => "), $dump);
  if($return) return $dump;
  echo($dump);
}

// convert output from dump() to json, so we can easily make object of it
/** unsused - automatic comment added on  */
function dump2json($dump) {
  $json = str_replace(array("\t", "Array\n(\n", "\n", '[',']', '=> '), array('', '{', '", ', '"', '"', ':"'), $dump);
 //return $json;
  $json = str_replace(array('{ ', ', )',',)'), array('{','}','}'), $json); 
  return $json;
}

function print_error($str, $where='') {
  if($where) $where = "IN $where ";
  print_debug("ERROR $where: $str");
  print_log("ERROR $where: $str");
}

function microtime_float() {
	list($usec, $sec) = explode(" ", microtime());
	return ((float)$usec + (float)$sec);
}


// same as array_filter, but keeps elements with zero value
function remove_empty($array) {
  return array_filter($array, '_remove_empty_internal');
}

/** unsused - automatic comment added on  */
function _remove_empty_internal($value) {
  return !empty($value) || $value === 0;
}


/** unsused - automatic comment added on  */
function make_array($var) {
  //if(!$var) return array();
   return (is_array($var) ? $var : array($var));
}

/** unsused - automatic comment added on  */
function row_sort(&$rows, $index=0, $ascending=true, $sort_type = SORT_REGULAR) {
  $comp = $ascending ? '>' : '<';
  $sort_fn = create_function('$row1, $row2', 'return ($row1['.$index.'] '.$comp.' $row2['.$index.']);');   
  usort($rows, $sort_fn);
}

  //switch $sort_type {
  //  case SORT_NUMERIC:
  //    $sort_fn = create_function('$row1, $row2', 'return ($row1['.$index.'] '.$comp.' $row2['.$index.']);');   
  //    break;
  //  default: 
  //    $sort_fn = create_function('$row1, $row2', 'return ($row1['.$index.'] '.$comp.' $row2['.$index.']);');   
  //    break;
  //}  

/** unsused - automatic comment added on  */
function direct_book_sort($a, $b, $rev=0) {
  return ($a->book_direct > $b->book_direct) ? -1 : 1;
}

/** unsused - automatic comment added on  */
function rate_sort_rev($a, $b, $rev=0) {
  return ($a->rate_discount > $b->rate_discount) ? -1 : 1;
}

/** unsused - automatic comment added on  */
function rate_sort($a, $b) {
  return ($a->rate_discount < $b->rate_discount) ? -1 : 1;
}

/** unsused - automatic comment added on  */
function regexp_list($arr) {
   if(!is_array($arr)) return "";
   return implode($arr, '|');
}

// looks for word boundary
/** unsused - automatic comment added on  */
function smart_truncate($text, $cutoff_length, $show_elipses=false) {
  //$text = str_replace(' ', '', $text);
  if(strlen($text) > $cutoff_length) { // truncate
    $cutoff = strpos($text, '.') ?: strpos($text, '?');
    //echo("co=$cutoff l=$cutoff_length");
    $cutoff = ($cutoff && $cutoff < $cutoff_length * 2) ? $cutoff+1 : $cutoff_length; // look for sentence break. Use if found before 2 x cutoff
    $text = substr($text,0,$cutoff);
    if($show_elipses) $text .= "...";
  }
  return $text;
}

// Thanks: http://stackoverflow.com/questions/12268387/truncate-text-without-truncating-html
/** unsused - automatic comment added on  */
function truncate_html($string, $length, $postfix = '&hellip;', $isHtml = true) {
  $string = trim($string);
  $postfix = (strlen(strip_tags($string)) > $length) ? $postfix : '';
  $i = 0;
  $tags = []; // change to array() if php version < 5.4

  if($isHtml) {
    preg_match_all('/<[^>]+>([^<]*)/', $string, $tagMatches, PREG_OFFSET_CAPTURE | PREG_SET_ORDER);
    foreach($tagMatches as $tagMatch) {
      if ($tagMatch[0][1] - $i >= $length) {
        break;
      }

      $tag = substr(strtok($tagMatch[0][0], " \t\n\r\0\x0B>"), 1);
      if ($tag[0] != '/') {
        $tags[] = $tag;
      }
      elseif (end($tags) == substr($tag, 1)) {
        array_pop($tags);
      }

      $i += $tagMatch[1][1] - $tagMatch[0][1];
    }
  }

  return substr($string, 0, $length = min(strlen($string), $length + $i)) . (count($tags = array_reverse($tags)) ? '</' . implode('></', $tags) . '>' : '') . $postfix;
}

function isJson($string) {
  if(!$string || is_array($string) || is_object($string)) return false;
  json_decode($string);
  return (json_last_error() == JSON_ERROR_NONE);
}

/** unsused - automatic comment added on  */
function json_post($url, $data, $options = array()) {

  // Setup cURL
  $ch = curl_init($url);
  curl_setopt_array($ch, array(
    CURLOPT_POST => TRUE,
    CURLOPT_RETURNTRANSFER => TRUE,
    CURLOPT_HTTPHEADER => array(
      'Content-Type: application/json'
    ),
    CURLOPT_POSTFIELDS => json_encode($data)
  ));
  
  // Send the request
  $response = curl_exec($ch);
  
  // Check for errors
  if($response === FALSE){
    die(curl_error($ch));
  }
  
  // Decode the response
  $responseData = json_decode($response, TRUE);
  
  // Print the date from the response
  return $responseData;
}

function get_html_and_status($url) {
  $http = curl_init($url);
  curl_setopt($http, CURLOPT_URL,$url);
  curl_setopt($http, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($http, CURLOPT_FOLLOWLOCATION, 1);
  $result = curl_exec($http);
  $http_status = curl_getinfo($http, CURLINFO_HTTP_CODE);
  curl_close ($http);
  return array($result, $http_status);
}

function get_html($url, $cookie_file_path="") {
  if (isset($_SERVER['HTTP_USER_AGENT'])) {
  	$agent = $_SERVER["HTTP_USER_AGENT"]; // user user agent of caller
  } else {
  	$agent = "Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.4) Gecko/20030624 Netscape/7.1 (ax)";
  }
   $ch = curl_init();
   curl_setopt($ch, CURLOPT_URL,$url);
 	 curl_setopt($ch, CURLOPT_USERAGENT, $agent);
   curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
   curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
	if($cookie_file_path) {
	  curl_setopt($ch, CURLOPT_COOKIEFILE, $cookie_file_path);
	  curl_setopt($ch, CURLOPT_COOKIEJAR, $cookie_file_path);
	}
   $result = curl_exec ($ch);
   curl_close ($ch);
   return $result;
}

function http_post($url, $fields, $curl_options='', $options=NULL) {
	$curl_options[CURLOPT_URL] = $url;
	$curl_options[CURLOPT_RETURNTRANSFER] = 1;
	$curl_options[CURLOPT_POST] = 1;
	$curl_options[CURLOPT_POSTFIELDS] = $fields;

	// Set the curl parameters.
	$ch = curl_init();
  foreach($curl_options as $k => $v) curl_setopt($ch, $k, $v);

  print_log("sending to {$url}".dump($fields,true), 'app', LOG_LEVEL_TEST);
	
	// Get response from the server.
	$httpResponse = curl_exec($ch);
  $http_status = curl_getinfo($ch, CURLINFO_HTTP_CODE);

  print_log("status: {$http_status } response".dump($httpResponse,true), 'app', LOG_LEVEL_TEST);
  
	if(!$httpResponse) {
		exit(curl_error($ch).'('.curl_errno($ch).')');
	}
  return $options['get_status'] ? array($httpResponse, $http_status) : $httpResponse;
}                          

function currency2html($currency) {
  return get_currency_symbol($currency, true);
}

function get_currency_symbol($currency, $html=0) {
  switch($currency) {
    case "BTC";
      return $html ? "&#3647;" : "‡∏ø";
    case "EUR";
      return $html ? "&euro;" : "€"; //chr(128);
    case "USD";
      return "$";
    case "GBP":
      return $html ? "&pound;" : "£"; //chr(163);
    case "YEN":
       return $html ? "&yen;" : "¥"; //chr(165); 
    default:
      return $currency;
  }
}

// takes an object or assoc. array k => v, returns array of arrays (k, v), (k,v)
/** unsused - automatic comment added on  */
function assoc2rows($arrayOrObj) {
  $ar = [];
  foreach($arrayOrObj as $k => $v) $ar[] = [$k,$v];
  return $ar;
}

// like strpos, but needle can be array
/** unsused - automatic comment added on  */
function str_array_pos($haystack, $needle_array) {
  if(!is_array($needle_array)) return strpos($haystack, $needle);
  for ($i = 0, $n = count($needle_array); $i < $n; $i++)
    if (($pos = strpos($haystack, $needle_array[$i])) !== false) return $pos;
  return false;
}

// like array_extend, but shallow for just to args, array or object
/** unsused - automatic comment added on  */
function object_extend($obj1, $obj2) {
  return (object) array_merge((array) $obj1, (array) $obj2);
}

// merge array from json to array
function array_merge_json($array, $json) {
  if(!$json || !isJson($json)) return $array;
  $ar2 = array_filter(json_decode($json, true));
  return array_merge($array, $ar2);
}


// merge arrays, but be smart about objects and strings
function array_merge_smart($ar1, $ar2) {
  return array_merge(any2array($ar1), any2array($ar2));
}

/** unsused - automatic comment added on  */
function array_sort($array, $key, $reverse = false) {  
  if(!is_array($array) || !count($array)) return $array;

  usort($array, function($a, $b) use ($key, $reverse) {
    if(is_object($a)) {
      $ka = $a->$key;
      $kb = $b->$key;
    } else if(is_array($a)) {
      $ka = $a[$key];
      $kb = $b[$key];
    } else {
      return true;
    }
    return $reverse ? strcasecmp($kb, $ka) : strcasecmp($ka, $kb);
  });
  return $array;
}


// like jquery.extend, override first array with following
// takes any number of arrays as input
function array_extend() {
  $arrays = func_get_args();
  $base = array_shift($arrays);
  foreach ($arrays as $array) {
    reset($base);
    while (list($key, $value) = @each($array))
      if (is_array($value) && @is_array($base[$key]))
        $base[$key] = array_extend($base[$key], $value);
      else $base[$key] = $value;
  }
  return $base;
}
// array_key_relative - Returns a key in an associative array relative to another key without using foreach. Very useful for finding previous key or finding next key in array, etc
// - Written by Jamon Holmgren (www.jamonholmgren.com). Last revised 8/18/2009. Free for any use.
// function array_key_relative(array $array, string $current_key, int $offset)
function array_key_relative($array, $current_key, $offset = 1) {  
  $keys = array_keys($array); // create key map  
  $current_key_index = array_search($current_key, $keys); // find current key
  if(isset($keys[$current_key_index + $offset])) { // return desired offset, if in array, or false if not
    return $keys[$current_key_index + $offset];
  }
  return false;
}

/** remove element by key from assoc. array (or object) */
/** unsused - automatic comment added on  */
function array_remove_key($array, $key) {
  if(is_array($array)) unset($array[$key]);
  else if(is_object($array)) unset($array->$key);
  return $array;
}


// remove elements from associative array where value = $search (0=>'val1', 1=>'val2')
/** remove element by value from assoc. array (or object) */
/** unsused - automatic comment added on  */
function array_remove($input, $search, $strict=0) {
  foreach($input as $key=>$val) {
    if($strict) {
      if($val == $search) unset($input[$key]);
    } else {
      if($val === $search) unset($input[$key]);
    }
  }
  return $input;
}

/** unsused - automatic comment added on  */
function get_element_name($haystack, $needle) {
  //echo("looking for $needle in ".dump($haystack));
  $el = array_find_one($haystack, "SHORTNAME", $needle);
  return $el->NAME->data;
}

function any2array($ar, $recurse = true) {
  if(!$ar) return []; 
  if(is_array($ar)) return $ar;
  if(is_object($ar)) return obj2array($ar);
  if(!$recurse) return [$ar];
  
  $ar = json_decode(any2json($ar));
  return any2array($ar, false);
}


function array2list($ar, $separator=",") {
  return implode($separator, $ar);
}

/** splits string into array using any of separators, skipping empty elements. */
/** Elements with strlen (0) are left */
function list2array($list, $separators="\,\;\n\r") {
  $array = preg_split('/['.$separators.']+/', $list, -1, PREG_SPLIT_NO_EMPTY);
  return array_filter($array, 'strlen');
}

// json encodes array or object
// if string is separated by any of separators, turn into array and json encode
function any2json($any, $separators="\,\;\n\r") {
  if(isJson($any)) return $any;
  if(!$any) return '';
  if(is_array($any) || is_object($any)) return json_encode($any);  
  $list = list2array($any, $separators);
  if(!$list) return '';
  foreach($list as $k => $v) $list[$k] = trim($v); // remove whitespace and newline around $separator
  return json_encode($list);
}


function array2obj($ar, $options = array()) {
  if(is_object($ar)) return $ar;
  $map = isset($options['map']) ? $options['map'] : null; // custom mapping function
  $obj = new stdClass(); //create empty class
  if(!$ar || !is_array($ar)) return NULL;
  foreach($ar as $k=>$v) {
    if(is_array($v)) $v = array2obj($v, $options);
    if(!$k) {
      print_log("array2obj: k is empty for array:".dump($ar, true), 'error', LOG_LEVEL_WARNING);
      continue;
    }
    $obj->$k = $map ? call_user_func($map, $v) : $v;
  }
  return $obj;
}

// works on nested objects (recursive)
function object_to_array($obj) {
  if(is_object($obj)) $obj = (array) $obj;
  //die("stack=".caller_stack());
  if(is_array($obj)) {
    $new = array();
    foreach($obj as $key => $val) {
      $new[$key] = object_to_array($val);
    }
  }
  else $new = $obj;
  return $new;       
}

function obj2array($obj, $recursive = false) {
  return $recursive ? object_to_array($obj) : (array)($obj);
}

function object_merge($obj1, $obj2, $filter=false) {
 $ar2 = $filter ? array_filter((array) $obj2) : (array) $obj2;
 return (object) array_merge((array) $obj1, $ar2);
}

function array2form($array, $options=NULL) {
  $hidden = isset($options['hidden']) ? $options['hidden'] : true; 
  $type = $hidden ? "hidden" : "text";
  if(!$array or !is_array($array)) return "";
  foreach($array as $k=>$v) {
    
    $inputs .= ($hidden ? '' : "$k: ").form_input($k, $type, $v, $options);
    //$inputs .= "v=$v<br>";
  }
  return $inputs;
}

// from query string 2 array
/** unsused - automatic comment added on  */
function url2vars($url="") {
  if(!$url) $url = $_SERVER['REQUEST_URI'];
  if($qsp = strpos($url, '?')) {
    $qs=substr($url,$qsp+1);
    $qs = str_replace('&amp;', '&', $qs);
    $vars = explode('&', $qs);
    foreach($vars as $var) {
      list($k, $v) = explode('=', $var);
      $GET[$k]=rawurldecode($v);
    }
  }
  return $GET;
}


// returns a string with the caller stack: fn (file: line) < fn (file: line)...
// if array is set, returns result as array with functioin names only
function caller_stack($options=NULL) {
  $array = $options['array'];
  if($backtrace = debug_backtrace()) {
    array_shift($backtrace); // remove this function
    if($array) return array_find_key($backtrace, 'function');
    foreach($backtrace as $ar) {
      $files = explode('/', $ar['file']);
      $file = array_pop($files);
      $line = $ar['line'];
      $result[] = $ar['function']." ($file:$line) ";
    }
  }
  return implode(" < ", $result);
}

// from object to query string - EMPTY OR ZERO VALUES ARE REMOVED
/** unsused - automatic comment added on  */
function obj2qs($obj, $hidden=1) {
  $array = (array)$obj; 
  return array2qs($array);
}


// takes a list like 10,20,30 or just 10
// NOTE: 0 or '' values are filtered out
// returns search param (either array or string)
/** unsused - automatic comment added on  */
function list2search($list, $del=',') {  
  $ar = array_filter(explode(',', $list));  
  if(!count($ar)) return NULL; 
  return count($ar) == 1 ? $ar[0] : $ar;
}

// from array to query string - EMPTY OR ZERO VALUES ARE REMOVED
function array2qs($array, $urlencode=false) {

  $qs_ar=array();
  foreach($array as $k=>$v) {
    if(is_array($v)) {
      //dump($v);
    } else if($k && strlen($v)) {
      $qs_ar[] = "$k=".rawurlencode($v);
    }
  }
  $glue = $urlencode ? "&amp;" : "&";
  return implode($glue, $qs_ar);
}

// makes a form with hidden or text form input for each variable in the query string
// except for fields specified in skip_fields (single or array)
/** unsused - automatic comment added on  */
function qs2form($qs='', $skip_fields, $hidden=true, $id_prefix='') {
  if(!$qs) $qs = $_SERVER['QUERY_STRING'];
  if($skip_fields && !is_array($skip_fields)) $skip_fields = array($skip_fields);
  $array = qs2array($qs);
  $row_separator = $hidden ? "\n" : "<br />";
  foreach($array as $name => $value) {
    if($skip_fields && in_array($name, $skip_fields)) continue; // skip this field
    $type = $hidden ? 'hidden' : 'text';
    if(!$hidden) $form .= "$name: ";
    $form .= form_input($name, $type, $value, array('id' => $id_prefix.$name)).$row_separator;
  }
  return $form;
}

function qs2array($qs='', $separator='&') {
  if(!$qs) $qs = $_SERVER['QUERY_STRING'];
  $vars = explode($separator, $qs);
  foreach($vars as $var) {
    list($k, $v) = explode('=', $var);
    $array[$k] = $v;
  }
  return $array;
}

// takes a separated (e.g. ; or AND or & ) list of conditions (for WHERE statement), 
// returns search array
// todo: implement LIKE, IN, operators etc
/** unsused - automatic comment added on  */
function sql2search($qs, $glue=';') {
  $vars = explode($glue, $qs);
  foreach($vars as $var) {
    if($op = sql_find_operator($var)) {
      list($k, $v) = explode($op, $var);
      $array[$k] = strip_quotes($v);
    } else {
      die("Couldn't find operator in term: $var");
    }
  }
  return $ar;
}
function sql_find_operator($term) {
  $term = strtolower($term);
  $operators = get_operators();
  $operators[] = ' like ';
  $operators[] = ' in ';
  
  foreach($operators as $op) {
    if(strpos($term, $op)) return $op;
  }
}

function obj2form($obj, $options=NULL) {
  $array = (array)$obj;
  return array2form($array, $options);
}

// $inputVars = $_GET. ts= [timestamp]
// returns true if this = obj->timestamp
/** unsused - automatic comment added on  */
function check_link_cookie($obj, $inputVars) {
  $ts=$inputVars['ts'];
  if(!$ts) return false;
  return $ts == stripdate($obj->created);
}

/** unsused - automatic comment added on  */
function get_link_cookie($obj) {
  return stripdate($obj->created);
}

/** unsused - automatic comment added on  */
function get_activation_cookie($userObj) {
  $ts  = str_replace(array(' ',':','-'), '', $userObj->created);
  return $ts.$userObj->parent_id;
}

/** unsused - automatic comment added on  */
function merge_objects($obj1, $obj2) {
 return array2obj(array_merge((array)$obj1, (array)$obj2));
}


/** unsused - automatic comment added on  */
function get_phrase($term, $case=0) {
  return phrase($term, $case);
}

// converts a percentage to multiplication factor
//  -10 => 0.9, 25 => 1.25 etc 
// % sign is ignored
// if negate is true, 10 becomes 0.9, -10 becomes 1.1
function percentage2factor($percentage, $negate=false) {
  return $negate ? (100 - floatval($percentage)) / 100 : (100 + floatval($percentage)) / 100;
}

// takes two discounts, like '10%' and '10%'
// returns the new combined discount '19%' (1 - 0.9 * 0.9) * 100 if both are perentages (or zero/blank)
// if negate is set, mulitply percentages by -1 (for discounts); 
function add_percentages($percentage1, $percentage2, $negate=false) {
  list($abs_percentage1, $perc_percentage1) = split_discount($percentage1);
  list($abs_percentage2, $perc_percentage2) = split_discount($percentage2);
  if($abs_percentage1 || $abs_percentage2) return '? %';  // can't handle this case
  $new_percentage = $negate ? 
    (1 - percentage2factor($perc_percentage1, true) * percentage2factor($perc_percentage2, true)) * 100 :
    (percentage2factor($perc_percentage1) * percentage2factor($perc_percentage2) - 1) * 100;
  $new_percentage = round($new_percentage, 2);
  return $new_percentage ? $new_percentage."%" : ''; 
}

/** unsused - automatic comment added on  */
function add_discounts($discount1, $discount2) {
  return add_percentages($discount1, $discount2, true);
}

// alias
// takes percentage string, returns array (abs_discount, perc_discount)
function split_percentage($percentage) {
  return split_discount($percentage);
}

// takes discount string, returns array (abs_discount, perc_discount)
function split_discount($discount) {
  if($pos=strpos($discount, '%')) { // % discount
    //echo("split_discount: percentage =".);
    return array(0, substr($discount, 0, $pos));
  } else { // absolute discount
    return array($discount, 0);
  }
}

function print_discount($discount, $currency='') {
  if(!$discount) return "";
  list($abs_discount, $perc_discount) = split_discount($discount);
  if($perc_discount) return $discount;
  $discount = number_format($abs_discount, 2);
  return $currency ? "$currency $discount" : $discount;
}

// if percentage is a negative % (ie -10%) returns 10%, otherwise ''
function percent2discount($percentage) {
  list($abs_discount, $perc_discount) = split_discount($percentage);
  return $perc_discount < 0 ? (-1*$perc_discount).'%' : '';
}

// returns discount (e.g. 10%) if discount is percentage and not 0%
/** unsused - automatic comment added on  */
function percent_discount($discount) {
  list($abs_discount, $perc_discount) = split_discount($discount);
  return $perc_discount > 0 ? $discount : '';
}

function get_city_currency($city_id) {
  global $db;
  $cur = db_getOne("SELECT currency FROM msCountry, msCity WHERE msCountry.id=msCity.grandparent_id AND msCity.id=$city_id");
  return $cur ? $cur : 'EUR';
}

// takes a price + surcharge (as number or %) and returns surcharged price
// if surcharge_only is set, returns only the surcharge
function apply_surcharge($price, $surcharge, $surcharge_only=false, $precision=2) {
  list($abs_discount, $perc_discount) = split_discount($surcharge);
  $result = $abs_discount ? apply_discount($price, $abs_discount*-1) : apply_discount($price, ($perc_discount*-1).'%');
  return $surcharge_only ? $result - $price : number_format($result, $precision, '.', '');
}

// takes a price + discount (as number or %) and returns discounted price
function apply_discount($price, $discount, $precision=6) {
  if(!$discount) return $price;
  list($abs_discount, $perc_discount) = split_discount($discount);
  if($perc_discount) { 
    if(!is_numeric($perc_discount)) return $price;
    $discount_factor = (100-$perc_discount) / 100;
    return round($price * $discount_factor, $precision);
  }
  if(!is_numeric($abs_discount)) return $price;
  return $price - $abs_discount;
}

// takes a value and a percentage string like "10%" and returns 10% of $val
/** unsused - automatic comment added on  */
function apply_percentage($val, $perc_str, $precision = 6) {
  list($abs, $perc) = split_discount($perc_str);
  if(!$val || !$perc) return 0;
  $result = $val * ($perc / 100);
  return $precision ? round($result, $precision) : $result;
}

//** unsused - automatic comment added on  */
//function num_diff($number1, $number2) {
//  if ( is_numeric($number1) && is_numeric($number2) ) {
//  $absolute_difference = abs( $number1 - $number2 );
//  }
//}


// creates a numberic array from $min to $max with step = $step
// if progressive is set, step is ignored and it will progressively increase from min to max
function num_array($min, $max, $step=1, $suffix="", $number_format=true, $zero_pad=false, $progressive=0) {
  $result = array();
  if(!is_numeric($min)) {dump($min);mydie("num_array: min $min is not numeric");}
  if(!is_numeric($max)) {dump($max);mydie("num_array: max $max is not numeric");}
  if(!is_numeric($step)) {dump($step);mydie("num_array: step $step is not numeric");}

  $steps = ($max-$min) / $step;
  if($progressive && $steps > $progressive) {
    $pfactor = round($steps / $progressive);
    //echo("$steps steps > $progressive pfactor=$pfactor<br>");
  } 

  for($i=$min; $i <= $max; $i+=$step) {
    if($zero_pad) $i=zero_pad($i);
    if($pfactor && $j > $progressive / 4) {
      $step *=2;
      $j=0;
    } 
    $i_f = $i > 100 && $number_format ? number_format($i) : $i;
    array_push($result, array($i, $i_f.$suffix));
    $j++;
  }
  return $result;
}

// creates a numeric array from $min to $max with step = $step optimized for time
/** unsused - automatic comment added on  */
function time_array($min, $max, $step=1, $time_format_24=true) {
  $result = array();
  if(!is_numeric($min)) {dump($min);die("time_array: min $min is not numeric");}
  if(!is_numeric($max)) {dump($max);die("time_array: max $max is not numeric");}
  if(!is_numeric($step)) {dump($step);die("time_array: step $step is not numeric");}

  $steps = ($max-$min) / $step;

  for($i=$min; $i <= $max; $i+=$step) {
    $time = dectime2hm($i);
    array_push($result, array($i, $time));
    $j++;
  }
  return $result;
}

function get_or_post($name) {
  return $_GET[$name] ? $_GET[$name] : $_POST[$name];
}

/** unsused - automatic comment added on  */
function post_or_get($name) {
  return $_POST[$name] ? $_POST[$name] : $_GET[$name];
}

// returns first positive integeter found in string
/** unsused - automatic comment added on  */
function find_first_int($str) {
  if (preg_match ("/([0-9]+).*/", $str, $regs)) return $regs[1];
  return '';
}

// takes a list, returns first item
/** unsused - automatic comment added on  */
function list_first($str, $separator=',') {
  if(!$str) return '';
  if(!strpos($str, $separator)) return $str;
  return array_shift(explode($separator, $str));
}

// prints html price
/** unsused - automatic comment added on  */
function print_price($obj, $field='total', $cents=1) {
  $cur = get_currency_symbol($obj->currency, true);
  return formatFloat($obj->$field, $cents, $cur);
}
  
// looks for an integer in string
function parseInt($str) {
  return (int) preg_replace('/\D/', '', $str);
}

// returns true if $var is a whole number 
/** unsused - automatic comment added on  */
function is_whole_number($var){
  return (is_numeric($var)&&(intval($var)==floatval($var)));
}

/** unsused - automatic comment added on  */
function tofloat($num) {
  $dotPos = strrpos($num, '.');
  $commaPos = strrpos($num, ',');
  $sep = (($dotPos > $commaPos) && $dotPos) ? $dotPos : 
    ((($commaPos > $dotPos) && $commaPos) ? $commaPos : false);
   
  if (!$sep) {
    return floatval(preg_replace("/[^0-9]/", "", $num));
  } 

  return floatval(
    preg_replace("/[^0-9]/", "", substr($num, 0, $sep)) . '.' .
    preg_replace("/[^0-9]/", "", substr($num, $sep+1, strlen($num)))
  );
}

// formats a Float to a whole number or with 2 decimals; includes a dollar sign in front
function formatFloat($number, $cents = 1, $currency_symbol='', $decimals=2) { // cents: 0=never, 1=if needed, 2=always
  if (is_numeric($number)) { // a number
    if (!$number) { // zero
      $money = ($cents == FORMAT_ALWAYS ? '0.00' : '0'); // output zero
    } else { // value
      $number = str_replace(',', '', $number); // remove 1000 separator
      if (floor($number) == $number) { // whole number
        $money = number_format($number, ($cents == FORMAT_ALWAYS ? $decimals : 0)); // format
      } else { // cents
        $money = number_format(round($number, $decimals), ($cents == FORMAT_NEVER ? 0 : $decimals)); // format
      } // integer or decimal
    } // value
    return $currency_symbol ? $currency_symbol.$money : $money;
  } // numeric
} // formatMoney

/** unsused - automatic comment added on  */
function round_significatant($number, $sigdigs) { 
  $multiplier = 1; 
  while ($number < 0.1) { 
    $number *= 10; 
    $multiplier /= 10; 
  } 
  while ($number >= 1) { 
    $number /= 10; 
    $multiplier *= 10; 
  } 
  return round($number, $sigdigs) * $multiplier; 
} 

function file_safe($filename) {
 return strtolower(ereg_replace('[^A-Za-z0-9_.]', '-', $filename));
}

// below two from http://neo22s.com/slug/
// discussion: http://stackoverflow.com/questions/2668854/sanitizing-strings-to-make-them-url-and-filename-safe
// see also http://code.google.com/p/owasp-esapi-php/

/** unsused - automatic comment added on  */
function replace_accents($var){ //replace for accents catalan spanish and more
  //return iconv('UTF-8', 'ASCII//TRANSLIT', $string);
  
  $a = array('À', 'Á', 'Â', 'Ã', 'Ä', 'Å', 'Æ', 'Ç', 'È', 'É', 'Ê', 'Ë', 'Ì', 'Í', 'Î', 'Ï', 'Ð', 'Ñ', 'Ò', 'Ó', 'Ô', 'Õ', 'Ö', 'Ø', 'Ù', 'Ú', 'Û', 'Ü', 'Ý', 'ß', 'à', 'á', 'â', 'ã', 'ä', 'å', 'æ', 'ç', 'è', 'é', 'ê', 'ë', 'ì', 'í', 'î', 'ï', 'ñ', 'ò', 'ó', 'ô', 'õ', 'ö', 'ø', 'ù', 'ú', 'û', 'ü', 'ý', 'ÿ', 'Ā', 'ā', 'Ă', 'ă', 'Ą', 'ą', 'Ć', 'ć', 'Ĉ', 'ĉ', 'Ċ', 'ċ', 'Č', 'č', 'Ď', 'ď', 'Đ', 'đ', 'Ē', 'ē', 'Ĕ', 'ĕ', 'Ė', 'ė', 'Ę', 'ę', 'Ě', 'ě', 'Ĝ', 'ĝ', 'Ğ', 'ğ', 'Ġ', 'ġ', 'Ģ', 'ģ', 'Ĥ', 'ĥ', 'Ħ', 'ħ', 'Ĩ', 'ĩ', 'Ī', 'ī', 'Ĭ', 'ĭ', 'Į', 'į', 'İ', 'ı', 'Ĳ', 'ĳ', 'Ĵ', 'ĵ', 'Ķ', 'ķ', 'Ĺ', 'ĺ', 'Ļ', 'ļ', 'Ľ', 'ľ', 'Ŀ', 'ŀ', 'Ł', 'ł', 'Ń', 'ń', 'Ņ', 'ņ', 'Ň', 'ň', 'ŉ', 'Ō', 'ō', 'Ŏ', 'ŏ', 'Ő', 'ő', 'Œ', 'œ', 'Ŕ', 'ŕ', 'Ŗ', 'ŗ', 'Ř', 'ř', 'Ś', 'ś', 'Ŝ', 'ŝ', 'Ş', 'ş', 'Š', 'š', 'Ţ', 'ţ', 'Ť', 'ť', 'Ŧ', 'ŧ', 'Ũ', 'ũ', 'Ū', 'ū', 'Ŭ', 'ŭ', 'Ů', 'ů', 'Ű', 'ű', 'Ų', 'ų', 'Ŵ', 'ŵ', 'Ŷ', 'ŷ', 'Ÿ', 'Ź', 'ź', 'Ż', 'ż', 'Ž', 'ž', 'ſ', 'ƒ', 'Ơ', 'ơ', 'Ư', 'ư', 'Ǎ', 'ǎ', 'Ǐ', 'ǐ', 'Ǒ', 'ǒ', 'Ǔ', 'ǔ', 'Ǖ', 'ǖ', 'Ǘ', 'ǘ', 'Ǚ', 'ǚ', 'Ǜ', 'ǜ', 'Ǻ', 'ǻ', 'Ǽ', 'ǽ', 'Ǿ', 'ǿ'); 
  $b = array('A', 'A', 'A', 'A', 'A', 'A', 'AE', 'C', 'E', 'E', 'E', 'E', 'I', 'I', 'I', 'I', 'D', 'N', 'O', 'O', 'O', 'O', 'O', 'O', 'U', 'U', 'U', 'U', 'Y', 's', 'a', 'a', 'a', 'a', 'a', 'a', 'ae', 'c', 'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i', 'n', 'o', 'o', 'o', 'o', 'o', 'o', 'u', 'u', 'u', 'u', 'y', 'y', 'A', 'a', 'A', 'a', 'A', 'a', 'C', 'c', 'C', 'c', 'C', 'c', 'C', 'c', 'D', 'd', 'D', 'd', 'E', 'e', 'E', 'e', 'E', 'e', 'E', 'e', 'E', 'e', 'G', 'g', 'G', 'g', 'G', 'g', 'G', 'g', 'H', 'h', 'H', 'h', 'I', 'i', 'I', 'i', 'I', 'i', 'I', 'i', 'I', 'i', 'IJ', 'ij', 'J', 'j', 'K', 'k', 'L', 'l', 'L', 'l', 'L', 'l', 'L', 'l', 'l', 'l', 'N', 'n', 'N', 'n', 'N', 'n', 'n', 'O', 'o', 'O', 'o', 'O', 'o', 'OE', 'oe', 'R', 'r', 'R', 'r', 'R', 'r', 'S', 's', 'S', 's', 'S', 's', 'S', 's', 'T', 't', 'T', 't', 'T', 't', 'U', 'u', 'U', 'u', 'U', 'u', 'U', 'u', 'U', 'u', 'U', 'u', 'W', 'w', 'Y', 'y', 'Y', 'Z', 'z', 'Z', 'z', 'Z', 'z', 's', 'f', 'O', 'o', 'U', 'u', 'A', 'a', 'I', 'i', 'O', 'o', 'U', 'u', 'U', 'u', 'U', 'u', 'U', 'u', 'U', 'u', 'A', 'a', 'AE', 'ae', 'O', 'o'); 
  $var= str_replace($a, $b,$var);
  return $var; 
}

// new version of url_safe from http://neo22s.com/slug/
// if path is true, forward slashes are allowed 
// if ascii_only is false, accents are preserved.
// default separator is -
function friendly_url($url, $path=false, $ascii_only = true, $separator='-') {
	// everything to lower and no spaces begin or end
 
	//replace accent characters, depends your language is needed
	if($ascii_only) $url=replace_accents($url);
	$url = strtolower(trim($url));
 
	// decode html maybe needed if there's html I normally don't use this
	//$url = html_entity_decode($url,ENT_QUOTES,'UTF8');
 
	// adding - for spaces and union characters
	$find = array(' ', '&', '\r\n', '\n', '+',',');
	//$url = str_replace ($find, '-', $url);
	$url = str_replace ($find, $separator, $url);
 
	//delete and replace rest of special chars
	//$find = array('/[^a-z0-9\-<>]/', '/[\-]+/', '/<[^>]*>/');
	
	$allowed = $path ? '/[^a-z0-9_\.'.preg_quote($separator).'<>\/]/': '/[^a-z0-9_\.'.preg_quote($separator).'<>]/';
	$find = array($allowed, '/['.preg_quote($separator).']+/', '/<[^>]*>/');
	//$repl = array('', '-', '');
	$repl = array('', $separator, ''); // replaces 1) unsafe with blank 2) multiple -- with one -
	$url = preg_replace ($find, $repl, $url);
 
	//return the friendly url
	return $url; 
}

# alias for friendly_url
function url_safe($text, $path=false, $ascii_only = true, $separator='-') {
  return friendly_url($text, $path, $ascii_only, $separator);
}

/*  
  $text = convert_smart_quotes($text);
  $text = str_replace(" ","-",trim(stripslashes($text)));
  $text = str_replace(".","",$text);
  $text = str_replace(")","",$text);
  $text = str_replace("(","",$text);
  $text = str_replace("=","",$text);
  $text = str_replace(",","",$text);
  $text = str_replace("'","",$text);
  $text = str_replace('"','',$text);
  $text = str_replace("!","",$text);
  $text = str_replace("?","",$text);
  $text = str_replace("&","and",$text);
  $text = str_replace('\\',"",$text);
  $text = str_replace(":","",$text);
  $text = str_replace(";","",$text);
  $text = str_replace("<","",$text);
  $text = str_replace(">","",$text);
  $text = str_replace("@","",$text);
  $text = str_replace("+","-",$text);
  $text = str_replace("*","",$text);
  $text = str_replace("#","_",$text);
  //$text=strtr($text,"()!$'?: ,&+-/.?????????????????????????????????????????????????????????????????????", "--------------SOZsozYYuAAAAAAACEEEEIIIIDNOOOOOOUUUUYsaaaaaaaceeeeiiiionoooooouuuuyy");
  $text = str_replace("---","-",$text);
  $text = str_replace("--","-",$text);
  if(!$path) $text = str_replace("/","-",$text);

  $li = strlen($text)-1;
  if($text[$li]=='-') $text = substr($text, 0, $li); // remove last char if -
  return strtolower($text);
}
*/

/** unsused - automatic comment added on  */
function get_eol() {
  # Is the OS Windows or Mac or Linux
  if (strtoupper(substr(PHP_OS,0,3)=='WIN')) {
    $eol="\r\n";
  } elseif (strtoupper(substr(PHP_OS,0,3)=='MAC')) {
    $eol="\r";
  } else {
    $eol="\n";
  }
  return $eol;
}


// returns a list of checkboxes + hidden result field for flags attribute type
function flags_select($name, $rows, $val, $maxrows=0, $key='id', $descriptor='name') {
  foreach($rows as $k => $v) {
    $obj = NULL;
    $obj->id = $v;
    $obj->name = $k;
    $objs[] = $obj;
  }
  $options['flags'] = 1;
  return checkbox_select($name, $objs, $options);
}

// returns a list of checkboxes + hidden result field for msel attribute type
// if maxrows is negative, it indicates max number of columns
//function checkbox_select($name, $rows, $val, $maxrows=4, $key='id', $descriptor='name', $flags=false) {
function checkbox_select($name, $rows, $options = array()) {
  $val = pick_first($options['value'], $_GET[$name], $_POST[$name]);
  $key = $options['key'] ?: 'id';
  $descriptor = $options['descriptor'] ?: 'name';
  $flags = $options['flags'];    
  $maxrows = $options['rows'] ?: 0;
  $maxcols = $options['cols'] ?: 0;
  
  $class = "cb_group cb_".$name;
  $div_class = $maxcols == 1 ? 'label_checkbox_pair' : 'label_checkbox_pair inline-block';

  if($maxcols > 0) $maxrows = ceil(count($rows) / $maxcols);

  $debug = '';
   
  foreach($rows as $obj) {
    $id = $obj->$key;
    $objName = $obj->$descriptor;
    if(!$objName) continue; // don't show if blank    
    $tName = phrase($objName, CAPITALIZE);
    $tName = phrase($objName, CAPITALIZE); // ." ($id)";
    if($maxrows > 0 && $rownum >= $maxrows) {
      $columns[] = $column;
      $column = array();
      $colnum++;
      $rownum=0;
    }
    $checked = $flags ? $val & $id : in_array($id, explode(',', $val));
    $cb_id = $class.'_'.$id;    
    //$column[] = html_div(form_checkbox_simple($cb_id, $checked, array('value' => $id, 'class' => $class)).html_label($tName, $cb_id), 'label_checkbox_pair')."\n";
    $column[] = html_div(form_checkbox($cb_id, $checked, array('value' => $id, 'class' => $class)).html_label($tName, $cb_id), $div_class)."\n";
    $rownum++;    
  }
  $columns[] = $column; // add the last column
  if($maxrows > 0) $columns[] = array_fill(0, $maxrows, '&nbsp;'); // add a dummy column for pretty display

  $cb_result_id = 'cbr_'.$name; //.'_'.$id;
  $cb_group_id = 'cbg_'.$name;
  $boxes .= $colnum ? css_columns($columns, '', '', '', '', '') : implode("\n", $column);
  //$debug = "rownum=$rownum colnum=$colnum ";                       

  //$js = $flags ? 
  //'var $cbs = $("#cbg_'.$name.' input");$cbs.bind("click", function() {checkboxes2flags("'.$cb_group_id.'", "'.$cb_result_id.'");})' :
  //'var $cbs = $("#cbg_'.$name.' input");$cbs.bind("click", function() {checkboxes2string("'.$cb_group_id.'", "'.$cb_result_id.'");})';

  $type = $_SESSION['dev'] || $_SESSION['test'] ? 'text' : 'hidden';
  $type = 'hidden';
  $input = form_input($name, $type, $val, array('id' => $cb_result_id, 'class' => 'cb_target'))."\n";
  $output .= html_div($input.$boxes, 'checkbox_group', array('id' => $db_group_id, 'data-flags' => $flags ? 1 : 0));
  return $debug.$output;
}

/** unsused - automatic comment added on  */
function process_checkboxes($field_val_list, $db_name, $phrase_name) {
  if($field_val_list) {
    $field_vals = explode(',', $field_val_list);
    $name_array = array();
    foreach($field_vals as $field_val) {
      $name_array[] = get_name($db_name, $field_val);
    }
    return phrase($phrase_name, CAPITALIZE). ": ". join(", ", phrases($name_array, CAPITALIZE))."<br>";
  }
}

/** unsused - automatic comment added on  */
function get_source_link($source, $apt, $html=false) {
  global $db;
  $base_url = db_getOne("SELECT base_url FROM source WHERE id='$source'");
  if(!$base_url) die("get_source_link: No base URL found");
  $source_url = $apt->source_url;
  if(!$source_url) return "";
  $link = $base_url.$source_url;
  return $html ? html_link($link) : $link;
}

/** unsused - automatic comment added on  */
function minmax($value, $min, $max) {
  return min(max($value, $min), $max);
}


function degrees2meters($degrees) {
  $meters_per_degree = 111320; // = 40075161/360; 1 degree = 111.3 km
  return $degrees * $meters_per_degree;
}

function meters2degrees($meters) {
  $degrees_per_meter = 0.00000898312; // =  1 meter = 0.00000898312 degrees
  return $meters * $degrees_per_meter;
}

// adds noise to lat/long within given radius in meters (approx)
function add_noise($lat, $long, $radius=100) {
  print_log("add_noise: stack=".caller_stack(), 'rental', LOG_LEVEL_DEV);
  $add_degrees = meters2degrees($radius);
  $max=1000;
  //$rand1 = ((rand(0,$max)/$max)-0.5)/$scale; // random number between -0.5 and .5 divided by scale
  //$rand2 = ((rand(0,$max)/$max)-0.5)/$scale;                                   
  $rand1 = ((rand(0,$max)/$max)-0.5); // random number between -0.5 and .5 
  $rand2 = ((rand(0,$max)/$max)-0.5);
  $lat += $rand1 * $add_degrees * 2; 
  $long += $rand2 * $add_degrees * 2;
  return array($lat, $long);
}

// returns true if str only contains A-Za-z0-9_.
/** unsused - automatic comment added on  */
function is_valid_filename($str) {
  if(ereg('^[A-Za-z0-9_][A-Za-z0-9_]*$', $str)) return true;
  else return false;
}

// "new" - from Uhuru

// returns web url for page
/** unsused - automatic comment added on  */
function page_url($obj) {
  return str_replace(array('////', '///', '//'), '/', $obj->path.$obj->script);
}

// returns directory on server for page
/** unsused - automatic comment added on  */
function page_dir($obj) {
  // todo: get docroot from siteObj ?
  return str_replace(array('////', '///', '//'), '/', $_SERVER['DOCUMENT_ROOT'].$obj->path);
}

// returns file on server for page
/** unsused - automatic comment added on  */
function page_script($obj) {
  // todo: get docroot from siteObj ?
  return str_replace(array('////', '///', '//'), '/', $_SERVER['DOCUMENT_ROOT'].$obj->path.$obj->script);
}


// adds trailing slash to url if missing
/** unsused - automatic comment added on  */
function add_trailing_slash($url) {
  if(strrpos($url, '/') == strlen($url)-1) return $url;
  return $url."/";
}

/** unsused - automatic comment added on  */
function parse_domain($site_url) {
  $host_domains = explode(".", $site_url);
  if(count($host_domains) < 2) die("parse_domain: $site_url is not a valid URL");
  $host_tld = array_pop($host_domains); // e.g. com
  $host_dom = array_pop($host_domains); // e.g apartments-for-rent
  $host_sub =  count($host_domains) ? array_pop($host_domains) : ''; // e.g. www
  return array($host_sub, $host_dom, $host_tld);
}

///** unsused - automatic comment added on  */
//function find_site($site_url, $domain) {
//  global $db;
//  // dump($db);
//  //echo("SELECT * FROM site WHERE url='$site_url'<br>");
//  //echo("SELECT * FROM site WHERE domain='$domain'<br>");
//  if($s = db_getRow("SELECT * FROM site WHERE url='$site_url'")) {
//    $s->test = false;
//  } else if($s = db_getRow("SELECT * FROM site WHERE domain='$domain'")) {
//    $s->test = true;
//  }
// return $s;                                            
//}

/** unsused - automatic comment added on  */
function find_page($script='') {
  global $db;
  if(!$script) $script= substr($_SERVER['SCRIPT_FILENAME'], strlen($_SERVER['DOCUMENT_ROOT'])-1); // relative script
  $pageObj = db_getRow("SELECT * FROM page WHERE script='$script'");
  return $pageObj;
}

/** unsused - automatic comment added on  */
function is_ascii($string) {
    // From http://w3.org/International/questions/qa-forms-utf-8.html
    return preg_match('%^(?:[\x09\x0A\x0D\x20-\x7E])*$%xs', $string);
} // function is_ascii

// Returns true if $string is valid UTF-8 and false otherwise.
/** unsused - automatic comment added on  */
function is_utf8($string) {
    // From http://w3.org/International/questions/qa-forms-utf-8.html
    return preg_match('%^(?:
          [\x09\x0A\x0D\x20-\x7E]            # ASCII
        | [\xC2-\xDF][\x80-\xBF]             # non-overlong 2-byte
        |  \xE0[\xA0-\xBF][\x80-\xBF]        # excluding overlongs
        | [\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}  # straight 3-byte
        |  \xED[\x80-\x9F][\x80-\xBF]        # excluding surrogates
        |  \xF0[\x90-\xBF][\x80-\xBF]{2}     # planes 1-3
        | [\xF1-\xF3][\x80-\xBF]{3}          # planes 4-15
        |  \xF4[\x80-\x8F][\x80-\xBF]{2}     # plane 16
    )*$%xs', $string);

} // function is_utf8

// Returns true if $string contains only valid multi-byte UTF-8 and false otherwise.
// so false for ASCII
/** unsused - automatic comment added on  */
function is_utf8_mb($string) {
    return preg_match('%^(?:
          [\xC2-\xDF][\x80-\xBF]             # non-overlong 2-byte
        |  \xE0[\xA0-\xBF][\x80-\xBF]        # excluding overlongs
        | [\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}  # straight 3-byte
        |  \xED[\x80-\x9F][\x80-\xBF]        # excluding surrogates
        |  \xF0[\x90-\xBF][\x80-\xBF]{2}     # planes 1-3
        | [\xF1-\xF3][\x80-\xBF]{3}          # planes 4-15
        |  \xF4[\x80-\x8F][\x80-\xBF]{2}     # plane 16
    )*$%xs', $string);

} // function is_utf8_mb


/** unsused - automatic comment added on  */
function split_token($token) {
  $timestamp = substr($token, 0, 14);
  $id = substr($token, 14);
  return array($timestamp, $id);
}

/** unsused - automatic comment added on  */
function unauthorized($error='') {
  header("HTTP/1.1 401 Unauthorized");
  header("Status: 401 Unauthorized");
  if(strlen($error)) echo($error);
  die();
}

/** unsused - automatic comment added on  */
function badrequest($error='') {
  header("HTTP/1.1 400 Bad Request");
  header("Status: 400 Bad Request");
  if(strlen($error)) echo($error);
  die();
}

/** unsused - automatic comment added on  */
function gone($p='', $url='') {
  header("HTTP/1.1 410 Gone");
  header("Status: 410 Gone");
  $url1 = "/home/gone.php?p=$p&url=$url";
  header("Location: $url1");
}

function http_status_name($code) {
  switch ($code) {
    case 100: return 'Continue'; break;
    case 101: return 'Switching Protocols'; break;
    case 200: return 'OK'; break;
    case 201: return 'Created'; break;
    case 202: return 'Accepted'; break;
    case 203: return 'Non-Authoritative Information'; break;
    case 204: return 'No Content'; break;
    case 205: return 'Reset Content'; break;
    case 206: return 'Partial Content'; break;
    case 300: return 'Multiple Choices'; break;
    case 301: return 'Moved Permanently'; break;
    case 302: return 'Moved Temporarily'; break;
    case 303: return 'See Other'; break;
    case 304: return 'Not Modified'; break;
    case 305: return 'Use Proxy'; break;
    case 400: return 'Bad Request'; break;
    case 401: return 'Unauthorized'; break;
    case 402: return 'Payment Required'; break;
    case 403: return 'Forbidden'; break;
    case 404: return 'Not Found'; break;
    case 405: return 'Method Not Allowed'; break;
    case 406: return 'Not Acceptable'; break;
    case 407: return 'Proxy Authentication Required'; break;
    case 408: return 'Request Time-out'; break;
    case 409: return 'Conflict'; break;
    case 410: return 'Gone'; break;
    case 411: return 'Length Required'; break;
    case 412: return 'Precondition Failed'; break;
    case 413: return 'Request Entity Too Large'; break;
    case 414: return 'Request-URI Too Large'; break;
    case 415: return 'Unsupported Media Type'; break;
    case 500: return 'Internal Server Error'; break;
    case 501: return 'Not Implemented'; break;
    case 502: return 'Bad Gateway'; break;
    case 503: return 'Service Unavailable'; break;
    case 504: return 'Gateway Time-out'; break;
    case 505: return 'HTTP Version not supported'; break;
    default: return '';
  }
}

/** unsused - automatic comment added on  */
function page_moved($page_type_or_path) {
  if(isint($page_type_or_path)) {
    $link = get_page_link($page_type_or_path);
  } else {
    $link = $page_type_or_path;
  }  
  return "This page has moved ".($link ? html_link($link, 'here') : '');
}

function redirect($url) {
  //echo("redirecting to $url");return;
  if(strpos($url, '&amp;')) $url = htmlspecialchars_decode($url);
  if (!headers_sent()){    //If headers not sent yet... then do php redirect
    header( "HTTP/1.1 301 Moved Permanently" );
    header( "Location: $url" );
  } else {
    js_redirect($url);
  }
}

function js_redirect($url) {
  if(strpos($url, '&amp;')) $url = htmlspecialchars_decode($url);
  if($url) {
    echo(
    "<script type='text/javascript'>document.location='$url';</script>".
    "<noscript><meta http-equiv='refresh' content='0;url=$url'/></noscript>");
  }
}

/** unsused - automatic comment added on  */
function newline($count=1) {
  return str_repeat("\n", $count);
}

// returns assoc. array which is the the subset of assocArrHaystack with the keys given in arrFields
// http://php.osuosl.org/manual/en/ref.array.php#71426
/** unsused - automatic comment added on  */
function subArr($assocArrHaystack, $arrFields)
{
  $arrSub = array();
  dump($assocArrHaystack);
  foreach ($assocArrHaystack as $key => $value) {
    if (in_array($key, $arrFields)) $arrSub[$key] = $value;
  }

  return $arrSub;
}

// alias
function add_leading_zero($num) {
	return zero_pad($num);
}

// adds leading zero if missing to single digit number (including zero, so 0 => 00);
function zero_pad($number) {
  return (is_numeric($number) && strlen($number)==1 && $number < 10 && $number >= 0) ? "0".$number : $number;
}

// replace two or more slashes by one
function clean_path($path ) {
  //$path = str_replace(array('//', '///'), '/', $path);
  $path = preg_replace('#/+#', '/', $path);
  //$path = str_replace(array('http:/', 'https:/'), array('http://', 'https://'), $path);
  return $path;  
}


// return document root, always with trailing slash (as this can be inconsistent)
// http://serverfault.com/questions/173977/inconsistent-trailing-slash-in-document-root-variable-in-php-when-using-apache
/** location of index.php - web document root */
function docroot($trailing_slash = true) {
  $realpath = realpath ($_SERVER['DOCUMENT_ROOT']."/");
  $docroot = str_replace ("//", "/", $realpath);
  return rtrim($docroot, "/").($trailing_slash ? '/' : '');
}

/** location of AFR library */
function libroot($trailing_slash = true) {
  return docroot(false)."/afr-lib".($trailing_slash ? '/' : '');
}

/** location of web application directory - parent of docroot */
function approot($trailing_slash = true) {
  return str_replace('/www', '', docroot($trailing_slash));
}

// full URL of current URL, with http or https 
// with trailing slash unless $trailing = false
function siteURL($trailing_slash = true) { // uses current URL 
  $protocol = (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off' || $_SERVER['SERVER_PORT'] == 443 || $_SERVER['HTTP_X_FORWARDED_PROTO'] == 'https') ? "https://" : "http://";
  $domainName = $_SERVER['HTTP_HOST'].($trailing_slash ? '/' : '');
  return $protocol.$domainName;
}

function this_url($uri='') {
  $qs = get_query_string();
  $path = this_path($uri);
  $qs = $qs ? "?".ltrim($qs, "?") : '';
  return siteUrl(false).$path.$qs;
}


function full_path($path = '') {
  if(!$path) $path = this_path();
  return siteUrl(false).$path;
}

// returns the path after stripping away city/language/affiliate ID if any
function this_path($uri='') {
  if(!$uri) $uri = $_SERVER['REDIRECT_URL'] ?: $_SERVER['REQUEST_URI'];
  if(strpos($uri, 'ajax.php') && $_SERVER['HTTP_REFERER']) $uri = $_SERVER['HTTP_REFERER']; 
  $parts = parse_url($uri);
  return $parts['path'];
  
  /** old code */
  if($_SERVER['REDIRECT_URL']) return $_SERVER['REDIRECT_URL'];
  $p = strpos($_SERVER['REQUEST_URI'], '?');
  return $p ? substr($_SERVER['REQUEST_URI'], 0, $p) : $_SERVER['REQUEST_URI'];
}

// replaces variable from query string, returns new uri
/** unsused - automatic comment added on  */
function add_query_var($uri, $var, $replace) {
  return remove_query_var($uri, $var, $replace);
}

// replaces variable from query string, returns new uri
function replace_query_var($uri, $var, $replace) {
  return remove_query_var($uri, $var, $replace);
}

// removes (or adds or replaces) variable from query string, returns new uri
function remove_query_var($uri, $var, $replace='') {
  if(!$uri) $uri = this_url();
  $parts = parse_url($uri);
  $scheme = isset($parts['scheme']) && $parts['scheme'] ? "{$parts['scheme']}:" : "";
  $host = isset($parts['host']) && $parts['host'] ? "//".$parts['host'] : "";
  
  $site_url = $scheme.$host;
  
  $qs = get_query_string($uri);
  if(!$qs && !strlen($var)) return $uri;
  
  $data = [];
  parse_str($qs, $data);

  if(strlen($replace) || $replace) $data[$var] = $replace;
  else unset($data[$var]);
  $qs = http_build_query($data);
  
  $path = this_path($uri);
  return $qs ? "{$site_url}{$path}?{$qs}" : $path;
}

// returns the bit after the ? in the request_uri
// $_SERVER['QUERY_STRING'] doesn't work as expected due to mod rewrite
function get_query_string($url = '', $use_amps = false) {
  //return $_SERVER['QUERY_STRING'];
  
  $uri = $url ?: $_SERVER['REQUEST_URI'];
  if(strpos($uri, 'ajax.php') && $_SERVER['HTTP_REFERER']) $uri = $_SERVER['HTTP_REFERER']; 
  
  //$req_uri = str_replace(array('?__cam', '&__cam', '&amp;__cam'), '', $req_uri); // little hack as we don't want this (combine and modify used by spider/publish);
  if($pos = strpos($uri, '?')) return $use_amps ? str_replace('&', '&amp;', substr($uri, $pos+1)) : substr($uri, $pos+1);
  else return '';
}


// Thanks Cameron Spear: https://cameronspear.com/blog/smart-query-strings/
// pass assoc. array, will add/replace to current qs
// default is to use current uri
/** unsused - automatic comment added on  */
function build_query_string($vars, $url='') {
  $query_string = get_query_string($url); // $_SERVER['QUERY_STRING'] doesn't work as expected due to mod rewrite
  parse_str($query_string, $return);
  //echo("qs=$query_string<br>");
  //echo("return=".dump($return, true));
  
  foreach ($vars as $key => $value) $return[$key] = $value;
  if($return['act'] == 'logout') unset($return['act']); // cheesy hack to get rid of log-out
  return http_build_query($return);  
}  

// returns the path with last /path/ stripped
// if level > 1, strip $level levels off
// no trailing slash returned (so it will work also if last part is a script)
/** unsused - automatic comment added on  */
function parent_path($uri='', $level=1) {
  $path = this_path($uri);
  $parts = array_filter(explode('/', $path));
  array_pop($parts);
  return '/'.implode('/', $parts);  
}
  
/** unsused - automatic comment added on  */
function strip_affiliate($url) {
  return preg_replace('#\/aid[a-z]?[0-9]+\/#', '/', $url);  
}

// returns a "clean" number with max $decimals
// cleans means: no thousands separator. No zeros at end
/** unsused - automatic comment added on  */
function clean_format($number, $decimals) {
  return clean_num(number_format(str_replace(',', '', $number), $decimals, '.', ''));
}

// removes trailing zeros from decimal numbers, not integers
function clean_num( $num ){
  $pos = strpos($num, '.');
  if($pos === false) { // it is integer number
    return $num;
  } else { // it is decimal number
    return rtrim(rtrim($num, '0'), '.');
  }
}

function strip_leading_zero($num) {
  return ltrim($num, '0');
  //return (is_numeric($number) && $num<10 && strlen($num)==2) ? $num[1] : $num; // strip leading zero
}



/** unsused - automatic comment added on  */
function salutation2sex($salutation) {
  $salutation = strtolower(rtrim($salutation, '.'));
  if(!$salutation) return SEX_UNKNOWN;
  if(in_array($salutation, array('mr', 'dhr'))) return SEX_MALE;
  if(in_array($salutation, array('ms', 'mrs'))) return SEX_FEMALE;
  return SEX_UNKNOWN;
}

function get_name_string($addObj, $glue=' ') {
	$nsa = get_name_array($addObj);
	return implode($glue, $nsa);
}

// returns true if array is associative
function isAssoc($arr) {
  if(!is_array($arr)) return false;
  return array_keys($arr) !== range(0, count($arr) - 1);
}

function is_assoc($array) { // alternate
  return isAssoc($array);
}

function get_name_array($addObj) {
  if(is_array($addObj)) $addObj = (object) $addObj;
	$nsa = array();
  if($n = $addObj->salutation) $nsa[] = $n;
  if($n = $addObj->first_name) $nsa[] = $n;
  if($n = $addObj->infix) $nsa[] = $n;
  if($n = $addObj->last_name) $nsa[] = $n;
	return $nsa;
}

// returns street/city/state/zip/country address as string as per get_address_array below
function get_address_string($addObj, $glue='<br />', $show_contact=0, $show_suffix=0) {
	return implode($glue, get_address_array($addObj, $show_contact, $show_suffix));
}

function mydie($str) {
  global $__SITE, $u_id;
  echo("{$str}<br>");
  echo("Stack:".caller_stack());
  echo("Included:".dump(get_included_files(), true));
  echo("Site".dump($__SITE, true));
  if(!$_SESSION['live'] || $u_id == SYSTEM_USER_ID) echo("Backtrace:".dump(debug_backtrace(), true));
  print_log($str, 'error', LOG_LEVEL_SEVERE);
  die();
}

/** unsused - automatic comment added on  */
function myerror($str, $return=false) {
  global $u_id;
  if(0 && !$_SESSION['live']) {
    echo($str);
    if($u_id == SYSTEM_USER_ID) echo(caller_stack());
  }
  print_log($str, 'error', LOG_LEVEL_ERROR);
  return $return;
}

function fix_country($country) {
  $country = trim(strtoupper($country));
  if(!$country) return '';
  $ccs = cc_list1();
  if(!in_array($country, array_keys($ccs))) {
    if($_SESSION['debug_invoice_address']) print_log("fix_country: could not find $country, returning blank", "address", LOG_LEVEL_ALWAYS);        
    return '';
  }
  return $country;
}

// returns an array with all address fields in obj
function get_address_data($obj) {
  if(!$obj) return array();
  $data = array();
  if(is_array($obj)) $obj = array2obj($obj);
  $address_fields = array('salutation' , 'first_name' , 'infix', 'last_name' , 'organization' , 'title' , 'address1' , 'address2' , 'city' , 'state' , 'zip' , 'country' , 'email_address', 'phone');
  foreach($address_fields as $fld) $data[$fld] = $obj->$fld;
  return $data;
}


/** return array with address fields of object */
function copy_address($obj, $data = []) {
  
  if(is_object($data)) $data = (array) $data;
  if(!$obj) return $data;
  
  $ar = get_address_data($obj);
  if(is_array($data) && count($data)) {
    foreach($data as $k => $v) $ar[$k] = $v; /** overrides */  
  }
  return $ar;
}

// formats street, city, state, zip, country according to local recipient conventions 
// todo: complete
function get_address_array($addObj, $show_contact=0, $show_suffix=0) {
  $address = array();
  if(is_array($addObj)) $addObj = (object) $addObj;
  
  if(!$show_suffix) $addObj->address_suffix= '';
  
  if(isset($addObj->address_street)) { // this is a landmark or property
    $street1 = implode(' ', array_filter(array($addObj->address_street, $addObj->address_number, $addObj->address_suffix)));
    $street2 = '';
    $org     = '';
    $city    = $addObj->address_city ?: $addObj->city;
    $state   = $addObj->address_state ?: $addObj->state;
    $zip     = $addObj->address_postcode ?: $addObj->postcode;
    $cc      = $addObj->address_country ?: $addObj->country;
    $show_contact = false;
  } else {
    $org     = $addObj->organization ?: $addObj->name;
    $street1 = $addObj->address1;
    $street2 = $addObj->address2;
    $city    = $addObj->city;
    $state   = $addObj->state;
    $zip     = $addObj->zip;
    $cc      = $addObj->country;
  }
  
  $country = get_country_name($cc);
  
  switch($cc) {
    case "US":
      $address = array("$org", $addObj->title, "$street1", "$street2", "$city", trim("$state $zip"), $country);
      break;
    case "NL":
    	if($zip = $zip) $pc = strlen($zip==7) ? $zip : substr($zip, 0, 4)." ".substr($zip,4); // add space to post code
      $address = array("$org", $addObj->title, "$street1", "$street2", trim("$zip $city"), $country);
      break;
    case "CH":
      $address = array("$org", $addObj->title, "$street1", "$street2", trim("$city $zip $state"), $country);
      break;
    default:
      $address = strlen(trim($state)) ? array("$org", $addObj->title, "$street1", "$street2", trim("$zip $city"), "$state", $country) :
                                  array("$street1", "$street2", trim("$zip $city"), $country);
      break;
  }
  if($show_contact) {
    $web = $addObj->web_address ? html_link(add_http($addObj->web_address), $addObj->web_address) : '';
    $email = $addObj->email_address ? html_link("mailto:$addObj->email_address", $addObj->email_address) : '';
    $address = array_merge($address, array($addObj->phone, $web, $email));
  }
	return array_filter($address);
}


function get_lang_name($code) {
  global $db;
  return db_getOne("SELECT name FROM language WHERE code='$code'");
}


// extracts whole number from beginning of string
/** unsused - automatic comment added on  */
function strtoint($str) {
  $regexp = '#([0-9]+).*#';
  if(preg_match($regexp, $str, $m)) return $m[1];
  else return is_numeric($str) ? $str: '';
}

// converts 2.? or 2? or 2,? or 2 ? or 2.5 or 2,75 to 2.75 etc (returns string representation of floating point number)
/** unsused - automatic comment added on  */
function strtofloat($str) {
  $regexp = '#([0-9]+)(\.|\,|\s)?(?|?|?)?.*#';
  $regexp2 = '#([0-9]+)(\.|\,)?([0-9]+)?.*#';
  if(preg_match($regexp, $str, $m)) { // for N cars
    $whole = $m[1];
    if($fraction = $m[3]) {
      if($fraction == '?') return "$whole.25";
      elseif($fraction == '?')  return "$whole.5";
      elseif($fraction == '?')  return "$whole.75";
      else return '';
    }
    return $whole;
  } else if(preg_match($regexp2, $str, $m)) { // for N cars
    $whole = $m[1];
    if($fraction = $m[3]) return "$whole.$fraction";
    else return $whole;
  } else {
    return is_numeric($str) ? $str: '';
  }
}

// changes case of all keys of array or object
/** unsused - automatic comment added on  */
function change_key_case($obj_or_array, $case = CASE_LOWER) {
  if(is_array($obj_or_array)) return array_change_key_case($obj_or_array, $case);
  else if(is_object($obj_or_array)) {
    return (object) array_change_key_case((array) $obj_or_array, $case);
  } else {
    return $obj_or_array;
  }
}

/** unsused - automatic comment added on  */
function array_same($array) {
  return count(array_unique($array)) <= 1;
}
  
function obj_copy($source, $keys) {
  return array2obj(array_copy($source, $keys));
}

// returns a copy of source array with given keys  
// keys can either be a plain array ('field1', 'field2', etc)
// or associative (or partially assoc) ('field1' => 'destfield2', 'field2', etc)
// if all is true, all fields are copied first, before mapping (only makes sense if $keys is associative);
function array_copy($source, $keys, $all=false) {
  if(is_object($source)) $source = (array) $source;
  $dest = $all ? $source : array();
  if(isAssoc($keys)) { // assoc. array, we're mapping
    foreach($keys as $k => $v) {
      if(isint($k)) $dest[$v] = $source[$v];
      else $dest[$v] = $source[$k];
    }
  } else { // plain array
    if($all) return $dest; // we're done, just return a copy
    $dest = is_array($keys) ? array_intersect_key($source, array_flip($keys)) : $source;
  }
  return $dest;
}

// takes ass. array and return (comma by default)-separated list of keys
// handy for DB INSERT fields
/** unsused - automatic comment added on  */
function array_keys_list($array, $del=", ") {
  return implode($del, array_keys($array));
}

// takes 2 ass. arrays. Adds values of corresponding members and returns result
/** unsused - automatic comment added on  */
function array_add_members($arr1, $arr2) {
  foreach($arr1 as $k => $v) $result[$k] = $v + $arr2[$k];
  return $result;  
}

// takes 2 objects. Adds values of corresponding members and returns result
/** unsused - automatic comment added on  */
function obj_add_members($obj1, $obj2) {
  foreach($obj1 as $k => $v) $result->$k = $v + $obj2->$k;
  return $result;  
}

// takes ass. array and return (comma by default)-separated list of keys
// handy for DB INSERT values
/** unsused - automatic comment added on  */
function array_values_list($array, $del=", ") {
  $values = array_values($array);
  $qvals = array();
  foreach($values as $val) {
    //$pos = strpos($val, "'");
    //$start = $pos>5 ? $pos-5 : 0;
    //$len = strlen($val) > $start+10 ? 10 : strlen($val)-1;
    //$snippet = substr($val, $start, $len);
    $val = addslashes($val);
    //$snippet2 = substr($val, $start, $len);
    //if($pos) echo("Found ' in value at pos $pos - <br>before: $snippet<br>after: $snippet2<br>");
    if(strpos($val, "'")!==0) $val = "'$val'"; // quote value
    $qvals[] = $val;
  }
  return implode($del, $qvals);
}

// takes ass. array and return (comma by default)-separated list of key = 'val'
// handy for DB UPDATE statements
function array_tuplets($array, $del=", ") {
  $tuplets = array();
  foreach($array as $key=>$val) {
    $val = addslashes($val);
    $tuplets[] = "$key='$val'"; // quote value
  }
  return implode($del, $tuplets);
}

// takes an input array and a map, returns an array 
// with input data mapped to names given in map
// todo: add flag as 
// passthru: will return values of input even if not in map. If false, these will be removed
// addslashes: escape single quotes - prepare for mysql entry

function map_array($input, $map, $passthru=true, $addslashes=false) {
  $data = array();
  foreach($input as $k => $v) {
    //echo("k=$k<br>");
    if(is_array($v)) {
      //echo("$k is array ... ");
      if($submap = $map[$k]) {
        if(is_array($submap)) {
          //echo(" mapped key found, also array, recursing<br>");
          $subdata = map_array($v, $submap);
          $data = array_merge($data, $subdata);
          //dump($subdata);
        } else {
          echo(" mapped key found = '$submap' not array, skipping<br>");
        } 
      } else {
        //echo(" not in map, skipping<br>");
      }
    } elseif($new_key = $map[$k]) {
      $data[$new_key] = $v;
      //echo("k=$k new=$new_key<br>");
      //$data[$new_key] = ($addslashes ? addslashes($v): $v);
    } else if($passthru) {
      $data[$k] = $v; // key not mapped
      //if($passthru) $data[$k] = ($addslashes ? addslashes($v): $v); // key not mapped
    }
  }
  return $data;  
}

function asc2hex ($temp) {
  $len = strlen($temp);
  for ($i=0; $i<$len; $i++) {
    $char = substr($temp,$i,1);
    $data.= ($char.": ".sprintf("%02x ",ord($char)));
  }
  return $data;
}

// takes an array of values array(1, 2, 3, 4)
// returns a list (string) "100,110,120,140" (sorted, unique)
// with values corresponding to map: array(1=>110, 2=>120, etc)
/** unsused - automatic comment added on  */
function map2list($array, $map) {
  $list='';
  if(!$array) return '';
  if(!is_array($array)) die("map2list: 1st arg not array");
  foreach($array as $el) {
    $list = add2list($list, $map[$el]);
  }
  return $list;
}

// add item to sorted list
// e.g add add2list('1,2,6,10',5) => '1,2,5,6,10'
function add2list($list, $item) {
  $listArray = $list ? explode(',', $list) : array();
  $listArray[] = $item;
  $listArray = array_unique($listArray);
  //$listArray = array_filter($listArray);
  sort($listArray);
  $new_list = implode(',', $listArray);
  //echo("<b>Added $item to list; was '$list' now $new_list</b><br>");
  return $new_list;
}



// searches an array of arrays, checking if key of subarray (subarray[0])== $needle. 
// Returns corresponding key of main array on success. 
// Note: use === to check result - could be zero or false
/** unsused - automatic comment added on  */
function array_search_subkey($needle, $haystack, $strict=false) {
  foreach($haystack as $k => $subarr) {
    if($strict && $subarr[0] === $needle) return $k;
    if(!$strict && $subarr[0] == $needle) return $k;
  }
  return FALSE;
}

// takes an array of values e.g. array(5, 2, 1, 4,1)
// returns a sorted,unique comma separated string (list) e.g. "1,2,3,4,5"
/** unsused - automatic comment added on  */
function array2sorted_unique_list($ar, $glue=',') {
  sort($ar);
  return implode($glue, (array_unique($ar)));
}


// like array_search, but takes and array of objects, returns only the key of first matching object with $obj->$key = $value
/** unsused - automatic comment added on  */
function objarray_search($array, $key, $value) {
  if(!is_array($array)) mydie("objarray_search: ".dump($array, true)." is not an array");
  foreach($array as $i => $obj) if($obj->$key == $value) return $i;  
}

// like array_search, but takes and array of objects, returns the first matching object with $obj->$key = $value
/** unsused - automatic comment added on  */
function objarray_find($array, $key, $value) {
  if(!is_array($array)) mydie("objarray_search: ".dump($array, true)." is not an array");
  foreach($array as $i => $obj) if($obj->$key == $value) return $obj;  
}

// takes array of arrays or array of objects, returns only elements with ar[key] or obj->key = value
// call back function is run-time generated (lambda)
function array_find($input, $key, $value) {
  $callback = create_function(
    '$element', 
    'if(is_array($element)) {
       return $element['.$key.'] == '.$value.';
     } elseif(is_object($element)) {
       return $element->'.$key.' == '.$value.';
     } else {
       die("array_find: element is neither array nor object");
     }'); 
  return array_filter($input, $callback);
}

// takes array of arrays or objects or arrays, 
// if value_only, return the max value, otherwise return the object/array with the largest key
function array_find_max($input, $key, $value_only=false) {
  //if($value_only && PHP_VERSION_ID >= 50300) { // works as of version 5.3 
  //  $max = array_reduce($input, function($v, $w) {
  //    //echo("key=$key<br>");
  //    $w = (array) $w;
  //    return max($v, $w[$key]);
  //  }, -999999999);
  //  echo("gh max v=".PHP_VERSION_ID);
  //  return $max;
  //}
  $max = null;
  $i_max = null;
  foreach ($input as $index => $item) {
    $val = is_object($item) ? $item->$key : $item[$key]; // we assume either object or array here
    if($val > $max) {
      $i_max = $index;
      $max = $val;
    }
  }
  return $i_max === null ? null : ($value_only ? $max : $input[$i_max]);
}

// takes array of arrays or objects or arrays, returns the object/array with the largest key
// if value_only, return the min value, otherwise return the object/array with the smallest key
function array_find_min($input, $key, $value_only=false) {
  //if($value_only && PHP_VERSION_ID >= 50300) { // works as of version 5.3 
  //  $min = array_reduce($input, function($v, $w, $key) {
  //    $w = (array) $w;
  //    return min($v, $w[$key]);
  //  }, 999999999);
  //  echo("gh min v=".PHP_VERSION_ID);
  //  return $min;
  //}
  $min = null;
  $i_min = null;
  foreach ($input as $index => $item) {
    $val = is_object($item) ? $item->$key : $item[$key]; // we assume either object or array here
    if($val !== null && ($min === null || $val < $min)) {
      $i_min = $index;
      $min = $val;
    }
  }
  return $i_min === null ? null : ($value_only ? $min : $input[$i_min]);
}

// returns array where each element (strings) are stripped starting at the character startChar 
/** unsused - automatic comment added on  */
function array_rtrim($ar, $startChar) {
  foreach($ar as &$val) {
    $pos = strpos($val, $startChar);
    $val = $pos === false ? $val : substr($val, 0, $pos);
  }
  return $ar; 
}

// takes array of arrays or objects, returns array of values corresponding to given key
function array_find_key($input, $key) {
  $ar = array();
  if(!$input) return NULL; 
  if(!is_array($input)) $input = array($input);
  foreach ($input as $a) {
    if(is_object($a)) {
      $ar[]= $a->$key;
    } elseif(is_array($a)) {
      $ar[]= $a[$key];
    } else {
      dump($input);die("array_find_key: Looking for key: '$key', but element is neither array nor object:");
    }
  }
  return $ar;
}

// returns first found string in haystack between delimiters startdel and enddel
function find_delimited_str($haystack, $startdel, $enddel='', $offset=0) {
  $start = $startdel ? strpos($haystack, $startdel) : 0;
  if ($start === false) return "";
  $start += strlen($startdel);
  $end = $enddel ? strpos($haystack, $enddel, $start) : strlen($haystack);
  $len = $end - $start;
  return $len > 0 ? substr($haystack, $start, $len) : "";
}

// returns array of strings found in haystack between delimiters startdel and enddel
/** unsused - automatic comment added on  */
function find_delimited_str_array($haystack, $startdel, $enddel) {
  $offset = 0;
  $result = array();
  while($str = find_delimited_str($haystack, $startdel, $enddel)) {
    $start = strpos($haystack, $startdel);
    if ($start === false) die("Didn't find start del $startdel");
    $start += strlen($startdel);
    $end = strpos($haystack, $enddel, $start) + strlen($enddel);;
    if ($end === false) die("Didn't find end del $enddel");
    $haystack = substr($haystack, $end);
    $result[] = $str;
  }
  return $result;
}

// removes $needle and all after from $haystack
/** unsused - automatic comment added on  */
function trim_trailing($haystack, $needle) {
  if($pos= strpos($haystack, $needle)) return substr($haystack, 0, $pos);
  return $haystack;
}

// removes $needle and all after from $haystack
/** unsused - automatic comment added on  */
function trim_leading($haystack, $needle) {
  if(strpos($haystack, $needle) !== false) $haystack = substr($haystack, strlen($needle));
  return $haystack;
}

// Recursive Function to add slashes with posted array or object
function addslashes_many($input_arr){
  if(is_object($input_arr)){
      foreach ($input_arr as $key1 => $val){
         $tmp->$key1 = addslashes_many($val);
      }
      return $tmp;
  } elseif(is_array($input_arr)){
      $tmp = array();
      foreach ($input_arr as $key1 => $val){
         $tmp[$key1] = addslashes_many($val);
      }
      return $tmp;
  } else{
     return addslashes($input_arr);
  }
}

// takes any valid URL (or just a domain name w/o protocol)
// returns domainname.tld (w/o subdomain - www) if show_sub=false (default)
/** unsused - automatic comment added on  */
function url2domain($url, $show_sub=false) {
  $parts = parse_url(add_http($url));
  $host  = strtolower(rtrim($parts['host'], '/')); // trim trailing slash
  $parts = explode('.', $host);
  $tld = array_pop($parts);
  $dom = array_pop($parts);
  if(count($parts)) $sub = array_pop($parts);
  if($tld && $dom) {
    return $sub && $show_sub ? "$sub.$dom.$tld" : "$dom.$tld";
  } else {
   print_debug("url2domain: $url doesn't look like a valid domain<br>");
  }
}

/** unsused - automatic comment added on  */
function shell_command($command, $echo=false, $dryrun=false) {
  // sync photos for amsterdam
  if($dryrun) {
    echo("Shell Command: <b>$command </b><br/>");
    return;
  }
  $output = shell_exec($command);
  if($echo) echo("<b>$command:</b><br>".$output);
}

// returns true if IP is in list (list can be 216.95.120 which will allow all subs of that IP
/** unsused - automatic comment added on  */
function check_ip($ip, $list) {
  foreach($list as $list_ip) {
    if(strpos($ip, $list_ip) === 0) return true;
  }
  return false;
}

// return random string of length l (max 32)
function unique_id($l = 16) {
  if($l > 32) $l = 32;
  return substr(md5(uniqid(mt_rand(), true)), 0, $l);
}

// simple hash function that returns a short hash for URL from a number (not meant to be super secure)
function myhash($str) {
  return shorthash(md5($str));
}

// adler32
function shorthash($str) {
  return hash('adler32', $str);
}

// old: simple hash function that returns a short hash for URL from a number (not meant to be super secure)
function myhash_old($str) {
  $md = md5($str);
                              
  while ($i < strlen($md)) {
    $char = $md[$i];
    $ascii = ord($char);
    if($ascii >= 97 && $ascii <= 122) $hash .= $char; // lower case
    $i++;    
  }
  //echo("myhash of $str = $hash<br>");
  return $hash;
}

// picks first value set of N params unless first argument is array, then it checks the array
// if val1 is array, loop through and return first non-blank/non-null/non-zero memmber
function pick_first($val1) {
  if(is_array($val1)) {
    foreach($val1 as $val) if($val) return $val;
  } else {
    $numargs = func_num_args();
    while($i < $numargs) if($val = func_get_arg($i++)) return $val;
  }
}

/** unsused - automatic comment added on  */
function generate_mod_rewrite($lang) {
  global $db;
  global $apt_path;
  $dicObjs = db_getAll("SELECT * FROM dictionary WHERE section='url' and language='$lang'");
  $lang_name = get_lang_name($lang);
  $mod_str = "# $lang_name URLs\n";
  foreach($dicObjs as $dicObj) {
    $page_key = substr($dicObj->term, 4);
    $page = db_getRow("SELECT * FROM page WHERE page='$page_key'");
    $url = $dicObj->phrase;
    //dump($page);
    
    if(strpos($page->script, 'relocation.php')) { // special case for relocation services
      $ss = $page_key;
      $mod_str .= 'RewriteRule  ^(/aid[0-9]{4})?(/[a-z]{2})?(/[^/\.]+)?/'.$url.'/?$   '.$page->script.'?_aid=$1&_el=$2&_il='.$lang.'&_geo=$3&_s=relocation&_ss='.$ss.' [L]'."\n";
      //echo("mod_str = $mod_str<br>");
    } elseif(strpos($page->script, 'guide.php')) { // special case for city guide
      $ss = substr($dicObj->term, 10); // key is 'url_guide_'
      $mod_str .= 'RewriteRule  ^(/aid[0-9]{4})?(/[a-z]{2})?(/[^/\.]+)?/'.$url.'/?$   '.$page->script.'?_aid=$1&_el=$2&_il='.$lang.'&_geo=$3&_s=guide&_ss='.$ss.' [L]'."\n";
    } else {
      $mod_str .= 'RewriteRule  ^(/aid[0-9]{4})?(/[a-z]{2})?(/[^/\.]+)?/'.$url.'(/[^/\.]+)?(/[^/\.]+)?/?$  '.$page->script.'?_aid=$1&_el=$2&_il='.$lang.'&_geo=$3&_s=$4&_ss=$5 [L]'."\n";
    }

  }
  $file = "$apt_path/server/rewrite.$lang";
  write_file($file, $mod_str);
  $len = strlen($mod_str);
  //echo(str_replace("\n",'<br>',$mod_str));
  echo("Wrote $len bytes to '$file' [Web server URL file]<br>");
}

// applies implode($value) if array, returns $value otherwise
/** unsused - automatic comment added on  */
function smart_implode($value, $glue='') {
  return is_array($value) ? implode($glue, $value) : $value;
}

// takes an array and removes elments with value = $val
function array_remove_element($arr, $val){
  foreach ($arr as $key => $value) if ($value == $val) unset($arr[$key]);
  return $arr;
  //return array_values($arr);
}

// takes a sorted | separated list (string) of values 2,3,5,6
// adds $val (4) and returns sorted list 2,3,4,5,6
/** unsused - automatic comment added on  */
function var_list_add($var, $val, $glue='|') {
  if($val=='') return $var; // nothing to add
  $vars = $var ? explode($glue, $var) : array();
  if(in_array($val, $vars)) return $var; // already there
  $vars[] = $val;
  sort($vars, $val);
  $result = implode($glue, array_unique($vars));
  return $result;
}

// takes a sorted comma separated list of values 2,3,4,5,6
// removs $val (4) and returns sorted list (string) 2,3,5,6
/** unsused - automatic comment added on  */
function var_list_remove($var, $val, $glue='|') {
  $vars = explode($glue, $var);    
  $result = implode($glue, array_remove_element($vars, $val));
  return $result;
}

// takes a comma separated list of single values or ranges ( a - b ), e.g: a,b,c-e,x
// and returns true if $needle is in the list
// if fuzzy is true, then only the first N characters of $needle are considered, where N is strlen($haystack)
// e.g. : in_range('1015', 1015 PK) = true
/** unsused - automatic comment added on  */
function in_rangelist($haystack, $needle, $fuzzy=false) {
  if(!$haystack) return false;
  $listAr = explode(',', $haystack);
  foreach($listAr as $range) if(in_range($range, $needle, $fuzzy)) return true;
  return false;
}

/** unsused - automatic comment added on  */
function remove_whitespace($str) {
  return str_replace(' ', '', $str);
}

// takes a single value or a dash (-) separated list of two values e.g: a or c-e
// and returns true if $needle is in the range
function in_range($haystack, $needle, $fuzzy=false) {
  if(!$haystack) return false;
  $needle = trim($needle);  
  $haystack = trim($haystack);
  if(!strpos($haystack, '-')) return $fuzzy ? substr($needle, 0, strlen($haystack)) == $haystack : $needle == $haystack; // single value, must match (or first letters, if fuzzy)
  list($from, $to) = explode('-', $haystack); // range
  $from = trim($from);
  $to = trim($to);
  $needle_from = $fuzzy ? substr($needle, 0, strlen($from)) : $needle;
  $needle_to   = $fuzzy ? substr($needle, 0, strlen($to)) : $needle;  
  return $needle_from >= $from && $needle_to <= $to;
}

// returns value of attribute in tag
// e.g. find_attr("<input id='foo'>", "id") returns 'foo'
// case sensitive
/** unsused - automatic comment added on  */
function get_attribute($tag, $attr) {
  return get_attr($tag, $attr);
}

function get_attr($tag, $attr) {  
  $pattern =  '/.*\s+('.$attr.'\s?=\s?"([^"]*)"|'.$attr."\s?=\s?'([^']*)|".$attr."\s?=\s?([^\s\>\/]+)).*/";
  preg_match($pattern, " $tag", $m); // add space first, or pattern doesn't work (hack)
  if($s = count($m)) for($i=2;$i < $s; $i++) if($m[$i]) return $m[$i]; // return first found
  return '';
}  

// Get the URL that is currently being served
/** unsused - automatic comment added on  */
function current_url() {
  if(isset($_SERVER['HTTPS']) && $_SERVER['HTTPS']=='on') {
    $proto = "https";
    $standard_port = '443';
  } else {
    $proto = 'http';
    $standard_port = '80';
  }
  
  $authority = $_SERVER['HTTP_HOST'];
  if (strpos($authority, ':') === FALSE &&
    $_SERVER['SERVER_PORT'] != $standard_port) {
    $authority .= ':' . $_SERVER['SERVER_PORT'];
  }
  
  if (isset($_SERVER['REQUEST_URI'])) {
    $request_uri = $_SERVER['REQUEST_URI'];
  } else {
    $request_uri = $_SERVER['SCRIPT_NAME'] . $_SERVER['PATH_INFO'];
    $query = $_SERVER['QUERY_STRING'];
    if (isset($query)) {
       $request_uri .= '?' . $query;
    }
  }  
  return $proto . '://'.$authority.$request_uri;
}

function xml2obj($xml) {
  $simpleXml = is_object($xml) ? $xml : simplexml_load_string($xml, null, LIBXML_NOCDATA);
  $json = json_encode($simpleXml, JSON_PRETTY_PRINT);    
  return json_decode($json);
}

function xmlurl2obj($url, $options = array()) {
  $xml_file = '';
  $cache = $options['cache'];
  $now = now();
  $log_file = $options['log_file'] ?: 'xml';
  if($cache) { // full json filename to cache result in
    $path_parts = pathinfo($cache);
    $cache_dir = $path_parts['dirname'];
    $filename = $path_parts['filename']; // since PHP 5.2.0
    
    $xml_dir = $path_parts['dirname']."/xml";
    if(!is_dir($cache_dir) && !mkdir($cache_dir, 0775, true)) die("xml2obj: Couldn't make cache directory $cache_dir");
    if(!is_dir($xml_dir) && !mkdir($xml_dir, 0775, true)) die("xml2obj: Couldn't make xml directory $xml_dir");
    $xml_file = $filename ? "$xml_dir/$filename.xml" : '';

  }
  $debug = $options['debug'];
  $file_ts = '';
  $len = 0;
  
  $json_cache_exists = $cache && is_file($cache) && filesize($cache) > 10; // file exists and is not corrupt
  $xml_cache_exists = $xml_file && is_file($xml_file) && filesize($xml_file) > 10; // file exists and is not corrupt
  
  if($json_cache_exists && !$xml_cache_exists) $json_cache_exists = false; // force download to save XML


  if($cache && $json_cache_exists && !$options['live']) { // read JSON from cache
    $input_source = $cache;
    $json = file_get_contents($cache);
    $file_ts = date("Y-m-d H:i:s", filemtime($cache));
    $len = filesize($cache);
    print_log("Read $len from json cache: {$cache}", $log_file, LOG_LEVEL_TEST);
  } else {
    
    // Read from cached XML
    if($xml_file && $xml_cache_exists && !$options['live']) { // read XML from cache
      $input_source = $xml_file;
      $fileContents = file_get_contents($xml_file);
      $file_ts = date("Y-m-d H:i:s", filemtime($xml_file));
      $len = filesize($xml_file);
      $input_source = $xml_file;
      print_log("Read $len from xml cache: {$xml_file}", $log_file, LOG_LEVEL_TEST);
      
    } else { // read from URL

      //echo(" live v3: reading from url=$url<br>");
      
      $input_source = $url;
      $fileContents = file_get_contents($url);
      $file_ts = now();
      $len = strlen($fileContents);
      //$fileContents = htmlentities($fileContents, ENT_COMPAT, "UTF-8"); // get rid of Unicode
      //die(" live beta: file=".form_text('foo', $fileContents));
     print_log("Read $len from URL {$url}", $log_file, LOG_LEVEL_TEST);
       
 
      if($xml_file && $fileContents) write_file($xml_file, $fileContents);
    }

    if(0) {
      $fileContents = str_replace(array("\n", "\r", "\t"), '', $fileContents);  // is this needed?
      $fileContents = str_replace(array("<![CDATA[", "]]>"), '', $fileContents);  // is this needed?
      $fileContents = trim(str_replace('"', "'", $fileContents)); // is this needed?
      $fileContents = str_replace(array("&", "_ampersand_"), '', $fileContents);  // is this needed?
    }

    $simpleXml = simplexml_load_string($fileContents, null, LIBXML_NOCDATA);
    if($debug) {
      echo("<textarea rows='30' cols='80'>$fileContents</textarea>");
      echo(dump($simpleXml, true));
    }

    // store timestamp in json file
    if(is_object($simpleXml)) $simpleXml->_timestamp = now();
    else if(is_array($simpleXml)) $simpleXml['_timestamp'] = now(); // don't think this will ever happen, but hey
    
    //$json = json_encode($simpleXml, JSON_PRETTY_PRINT|JSON_UNESCAPED_UNICODE);
    $json = json_encode($simpleXml, JSON_PRETTY_PRINT);    
  }
  $obj = json_decode($json);
  
  if(!$file_ts) die("xml2obj: no timestamp from input $input_source");  
  if(!is_object($obj) || !$obj) {
    print_log("xml2obj: No Object:<br>file=$cache<br>size=".filesize($cache)."<br>source=$input_source<br>json=$json<br>", "xml", LOG_LEVEL_ERROR);
    return null;
    //die("No Object:<br>file=$cache<br>size=".filesize($cache)."<br>source=$input_source<br>json=$json<br>");    
  }
  
  // write to cache if exists and:
  // 1) live data and decode ok  
  // 2) there's missing _source or _timestamp  
  if($obj->_timestamp && !is_date(strip_time($obj->_timestamp))) $obj->_timestamp = ''; // not valid, unset
  //echo("cache=$cache<br>ts=".$obj->_timestamp."<br>source=".$obj->_source."<br>is=".$input_source."<br>url=".$url.html_break());
  if($cache && (!$obj->_timestamp || !$obj->_source || $input_source == $url)) { 
    if(strpos($cache, "club") === 0) die("write");
    $obj->_timestamp = $file_ts; // use file timestamp
    $obj->_source = $url; // use file timestamp
    $json = json_encode($obj, JSON_PRETTY_PRINT);
    $len_h = bytes2human(strlen($json));
    print_log("$now: Wrote $len_h to $cache<br>", 'xml', LOG_LEVEL_ALWAYS);

    write_file($cache, $json);
    //echo("<b>Write</b><br>");
  } else {
    //echo("<b>no write</b><br>");
  }
  $obj->_input_source = $input_source;
  $len_h = bytes2human($len);
  print_log("$now: Read $len_h from $input_source<br>", $log_file, LOG_LEVEL_ALWAYS);
  return $obj;
}

// replace unicode with html entities
// for &nbsp; juse use space
function unicode2html($str) {
  $str = htmlentities($str, ENT_COMPAT, "UTF-8");
  $str = str_replace('&nbsp;', " ", $str);
  return $str;
}

// map  ISO2 country codes to country currency codes
// http://country.io/currency.json
function country2currency($cc) {
  $map = '{"BD": "BDT", "BE": "EUR", "BF": "XOF", "BG": "BGN", "BA": "BAM", "BB": "BBD", "WF": "XPF", "BL": "EUR", "BM": "BMD", "BN": "BND", "BO": "BOB", "BH": "BHD", "BI": "BIF", "BJ": "XOF", "BT": "BTN", "JM": "JMD", "BV": "NOK", "BW": "BWP", "WS": "WST", "BQ": "USD", "BR": "BRL", "BS": "BSD", "JE": "GBP", "BY": "BYR", "BZ": "BZD", "RU": "RUB", "RW": "RWF", "RS": "RSD", "TL": "USD", "RE": "EUR", "TM": "TMT", "TJ": "TJS", "RO": "RON", "TK": "NZD", "GW": "XOF", "GU": "USD", "GT": "GTQ", "GS": "GBP", "GR": "EUR", "GQ": "XAF", "GP": "EUR", "JP": "JPY", "GY": "GYD", "GG": "GBP", "GF": "EUR", "GE": "GEL", "GD": "XCD", "GB": "GBP", "GA": "XAF", "SV": "USD", "GN": "GNF", "GM": "GMD", "GL": "DKK", "GI": "GIP", "GH": "GHS", "OM": "OMR", "TN": "TND", "JO": "JOD", "HR": "HRK", "HT": "HTG", "HU": "HUF", "HK": "HKD", "HN": "HNL", "HM": "AUD", "VE": "VEF", "PR": "USD", "PS": "ILS", "PW": "USD", "PT": "EUR", "SJ": "NOK", "PY": "PYG", "IQ": "IQD", "PA": "PAB", "PF": "XPF", "PG": "PGK", "PE": "PEN", "PK": "PKR", "PH": "PHP", "PN": "NZD", "PL": "PLN", "PM": "EUR", "ZM": "ZMK", "EH": "MAD", "EE": "EUR", "EG": "EGP", "ZA": "ZAR", "EC": "USD", "IT": "EUR", "VN": "VND", "SB": "SBD", "ET": "ETB", "SO": "SOS", "ZW": "ZWL", "SA": "SAR", "ES": "EUR", "ER": "ERN", "ME": "EUR", "MD": "MDL", "MG": "MGA", "MF": "EUR", "MA": "MAD", "MC": "EUR", "UZ": "UZS", "MM": "MMK", "ML": "XOF", "MO": "MOP", "MN": "MNT", "MH": "USD", "MK": "MKD", "MU": "MUR", "MT": "EUR", "MW": "MWK", "MV": "MVR", "MQ": "EUR", "MP": "USD", "MS": "XCD", "MR": "MRO", "IM": "GBP", "UG": "UGX", "TZ": "TZS", "MY": "MYR", "MX": "MXN", "IL": "ILS", "FR": "EUR", "IO": "USD", "SH": "SHP", "FI": "EUR", "FJ": "FJD", "FK": "FKP", "FM": "USD", "FO": "DKK", "NI": "NIO", "NL": "EUR", "NO": "NOK", "NA": "NAD", "VU": "VUV", "NC": "XPF", "NE": "XOF", "NF": "AUD", "NG": "NGN", "NZ": "NZD", "NP": "NPR", "NR": "AUD", "NU": "NZD", "CK": "NZD", "XK": "EUR", "CI": "XOF", "CH": "CHF", "CO": "COP", "CN": "CNY", "CM": "XAF", "CL": "CLP", "CC": "AUD", "CA": "CAD", "CG": "XAF", "CF": "XAF", "CD": "CDF", "CZ": "CZK", "CY": "EUR", "CX": "AUD", "CR": "CRC", "CW": "ANG", "CV": "CVE", "CU": "CUP", "SZ": "SZL", "SY": "SYP", "SX": "ANG", "KG": "KGS", "KE": "KES", "SS": "SSP", "SR": "SRD", "KI": "AUD", "KH": "KHR", "KN": "XCD", "KM": "KMF", "ST": "STD", "SK": "EUR", "KR": "KRW", "SI": "EUR", "KP": "KPW", "KW": "KWD", "SN": "XOF", "SM": "EUR", "SL": "SLL", "SC": "SCR", "KZ": "KZT", "KY": "KYD", "SG": "SGD", "SE": "SEK", "SD": "SDG", "DO": "DOP", "DM": "XCD", "DJ": "DJF", "DK": "DKK", "VG": "USD", "DE": "EUR", "YE": "YER", "DZ": "DZD", "US": "USD", "UY": "UYU", "YT": "EUR", "UM": "USD", "LB": "LBP", "LC": "XCD", "LA": "LAK", "TV": "AUD", "TW": "TWD", "TT": "TTD", "TR": "TRY", "LK": "LKR", "LI": "CHF", "LV": "EUR", "TO": "TOP", "LT": "LTL", "LU": "EUR", "LR": "LRD", "LS": "LSL", "TH": "THB", "TF": "EUR", "TG": "XOF", "TD": "XAF", "TC": "USD", "LY": "LYD", "VA": "EUR", "VC": "XCD", "AE": "AED", "AD": "EUR", "AG": "XCD", "AF": "AFN", "AI": "XCD", "VI": "USD", "IS": "ISK", "IR": "IRR", "AM": "AMD", "AL": "ALL", "AO": "AOA", "AQ": "", "AS": "USD", "AR": "ARS", "AU": "AUD", "AT": "EUR", "AW": "AWG", "IN": "INR", "AX": "EUR", "AZ": "AZN", "IE": "EUR", "ID": "IDR", "UA": "UAH", "QA": "QAR", "MZ": "MZN"}';
  $mapAr = json_decode($map, true);
  return $mapAr[$cc];
  
}

// find source, returns the xml from source's xml_url as object
/** unsused - automatic comment added on  */
function xml_import($source_name, $dryrun=false) {
  global $db;
  global $upload_dir;
  global $log_dir;
  $source = find_object('source', array('name' => $source_name));
  if(!$source) die("Couldn't find source $source_name");
  $xml_url = $source->xml_url;
  if(!$xml_url) die("$source_name has no xml url");
  
  $target = url_safe($source_name);
  if($dryrun) {
    // $xml_source = "$upload_dir/".$target.".xml";
    $xml_source = $log_dir.'XML/fetch/'.$target.".xml";
    echo("<h3>Fetching cached data from $xml_source<h3>");
  } else {
    $xml_source = fetch_xml($xml_url, $target);
    echo("<h3>Fetching live data from $xml_url<h3>");
  }
  $xml = read_file($xml_source);
  if(!$xml) die("Couldn't read from $xml_source");
  
  $options = LIBXML_NOWARNING|LIBXML_NOERROR|LIBXML_COMPACT;
  $options = LIBXML_NOWARNING|LIBXML_NOERROR;
  $obj = simplexml_load_string($xml, 'SimpleXMLElement', $options);
  if(!$obj) { // try UTF-8 encoding
    $obj = simplexml_load_string(utf8_encode($xml), 'SimpleXMLElement', $options);
    if(!$obj) die("Couldn't convert xml to object");
  }
  
  return $obj;    
}

/** unsused - automatic comment added on  */
function xmlspecialchars($text, $charset='UTF-8') {
   return str_replace('&#039;', '&apos;', htmlspecialchars($text, ENT_QUOTES, $charset));
}
                                         
function json_success_object($message="", $options=NULL) {
  $result->success = $options['success'] ?: 1;
  $result->message = $message;
  if($data = $options['data']) $result->data = $data;
  return $result;  
}

function json_error_object($errors, $options=NULL) {
  $result->success = 0;
  $result->error = $errors;
  if($data = $options['data']) $result->data = $data;
  return $result;  
}

function json_message($text, $options = NULL) {
  $options['field'] = $options['field'] ?: 'message'; 
  json_success($text, $options);
}

function json_success($message, $options = NULL) {      
  $options['field'] = 'message';
  $options['success'] = 1;  
  json_error($message, $options);
}

function json_error($text, $options=NULL) {
  $echo = isset($options['echo']) ? $options['echo'] : true; // echo or return error ?
  $json = isset($options['json']) ? $options['json'] : true; // if false, echo or return error
  $success = isset($options['success']) ? $options['success'] : 0;
  $encode = isset($options['encode']) ? $options['encode'] : true;   

  $field = $options['field'] ?: 'error';
  if(is_array($text)) {
    $text = $plain_text_error = implode(", ", $text);
    $encode = false;
  } else if(is_object($text)) {
    $text = dump($text, true); // for debugging
  } else {
    $plain_text_error = strip_tags(html2txt($text));
  }
    
  // allow json_error to be used as "normal" error (echo/return)
  if(!$json) {
    if(!$echo) return $text;
    echo($text);die();
  }
  
  if($success) {
  } else {
    $error_code = $options['error_code'] ?: 200;
    $http_status = "Unprocessable Entity";
    header("HTTP/1.0 $error_code $http_status: $plain_text_error");
  }
  
  $data = array('success' => $success, $field => $text);  
  $output = $encode ? htmlspecialchars(json_encode($data), ENT_NOQUOTES) : json_encode($data);
  if($options['jsonp']) $output = "($output)";
  echo($output);
  if($field == 'error') {
    global $log_dir;
    $log_file   = $log_dir.'ajax.txt';
    write_file($log_file, "JSON Error: $text\n", 'a');
  }
  
  die();
}

# execute command as given user
# like exec, last line is returned. For more lines, send $output (array)
# return value of the command optionally returned in $return_var
# www-data must have nopasswd sudo privileges for given user (www-data ALL=(phpexec) NOPASSWD:ALL)
# if root is set to true, command will be run as root (phpexec user must have nopasswd sudo privileges for root)
function execCmd($cmd, &$output='', &$return_var='', $user = 'phpexec', $root=false, $pwd=NULL) {
  global $u_level;
  $sudo = $root ? 'sudo' : '';
  if($u_level < USER_LEVEL_STAFF && $pwd != $_SESSION['sqlgarbage']) die("execCmd: Not authorized. Contact the system administrator (system@apartments-for-rent.com).");
  
  $full_command = $user == 'phpexec' ? "sudo -u $user $sudo $cmd" : $cmd;
  //echo("executing".html_break().$full_command.html_break());
  return exec($full_command, $output, $return_var);
}

function execRootCmd($cmd, &$output='', &$return_var='', $user = 'phpexec') {
  return execCmd($cmd, $output, $return_var, $user, true);
}

function execRemoteCmd($server, $cmd, &$output='', &$return_var='', $user = 'phpexec', $root=false) {
  $sudo = $root ? 'sudo' : '';
  $full_command = "ssh $user@$server $sudo $cmd";
  return execCmd($full_command, $output, $return_var, $user);
}

/** unsused - automatic comment added on  */
function execRemoteRootCmd($server, $cmd, &$output='', &$return_var='', $user = 'phpexec') {
  return execRemoteCmd($server, $cmd, $output, $return_var, $user, true);
}

// returns true if the first 3 octets match of the 2 addresses
/** unsused - automatic comment added on  */
function same_network($live_server_ip, $dev_server_ip) {  
  $live_ip_ar = explode('.', $live_server_ip);array_pop($live_ip_ar);
  $dev_ip_ar = explode('.', $dev_server_ip);array_pop($dev_ip_ar);
  return implode('.', $live_ip_ar) == implode('.', $dev_ip_ar); 
}

// replace every occurrence of a non-numeric character with its corresponding ascii value
function numerify($str) {
  for($i=0;$i < strlen($str); $i++) {
    $char = $str[$i];
    $ascii = ord($char);
    $new .= ($ascii < 48 || $ascii > 57) ? $ascii : $char;
  }
  return $new;
}

// takes a string like a=1;b=2;c=3 returns assoc array (a=>1, b=>2, c=>3);
/** unsused - automatic comment added on  */
function filters2array($str, $outer_delim=';', $inner_delim='=') {
  $filters = explode($outer_delim, $str);
  $array = array();
  foreach($filters as $filter) {
    list($k, $v) = explode($inner_delim, $filter);
    $array[$k] = $v;
  }
  return $array;
}

// returns an array of all items in fields (array) that are missing from obj_or_array
// if check_value is true, then the value must be non-null, otherwise it is enough that the key is set
function check_required($obj_or_array, $fields, $check_value = true) {
  $assoc = false;
  if(isAssoc($fields)) { // fields in format "fld => fld_name" for pretty printing
    $assoc = true;
    $terms = $fields;
    $fields = array_keys($fields);
  }
  $ar = (array) $obj_or_array;
  if($check_value) $ar = array_filter($ar);
  $keys = array_keys($ar);
  $missing = array_diff($fields, $keys);
  return $assoc ? array_values(array_intersect_key($terms, array_flip($missing))) : $missing;
}


// takes a string like a=1;b=2;c>=3;d IN (1,2,3) returns search array as used by my framework
/** unsused - automatic comment added on  */
function filters2search($str, $outer_delim=';', $inner_delim='=') {
  $filters = explode($outer_delim, $str);
  $array = array();
  foreach($filters as $filter) {
    list($k, $v) = explode($inner_delim, $filter);
    $val = strip_quotes($v);
    if(strlen($val)) {
      $array[$k] = $val;
    } else {
      if (preg_match ("/([a-zA-Z\-_]+)\s?(NOT IN|IN|>=|<=|>|<|<>|!=)\s?(.*)/", $k, $regs)) {
        $var = $regs[1];
        $op  = strtoupper($regs[2]);
        $val = $regs[3];
        //dump($regs);
        if($op == 'IN') {
          $array[$var] = explode(',', trim($val, '()'));
        } elseif($op == 'NOT IN') {
          $array[$var] = array('<>', explode(',', trim($val, '()')));
        } else {
          $array[$var] = array($op, $val);
        }
      }
    }
  }
  return $array;
}

// returns NULL if password is OK, and error message otherwise.
function check_password_strength($password, $password2='', $check_match=false) {
  if(strlen($password) < PASSWORD_LENGTH) return  "The password must be at least ".PASSWORD_LENGTH." characters";
  if($check_match && $password != $password2) return "The passwords do not match";
  return NULL;
}

function generateHash($plainText, $salt = null) {
  if ($salt === null) {
    $salt = substr(md5(uniqid(rand(), true)), 0, SALT_LENGTH);
  } else {
    $salt = substr($salt, 0, SALT_LENGTH);
  }
  return $salt . sha1($salt . $plainText);
}

/** unsused - automatic comment added on  */
function activation_link($userObj) {
  $site_url=$_SERVER['HTTP_HOST'];
  $id = $userObj->id;
  $cookie = myhash(stripdate($userObj->created));
  $activation_link = "http://$site_url/home/register.php?id=$id&cookie=$cookie";
  return html_link($activation_link, phrase('activate', CAPITALIZE));
}

/** unsused - automatic comment added on  */
function reset_link($siteObj, $guid, $text = 'here', $linkonly=false) {
  if(!$guid) return '';
  $site_url=site_full_url($siteObj);
  $reset_link = $site_url.site_page_link($siteObj, PAGE_PASSWORD)."?guid=$guid";
  return $linkonly ? $reset_link : html_link($reset_link, $text);
}

/** unsused - automatic comment added on  */
function implode_quoted($array, $q = "'") {
  if(!count($array)) return '';
  return "$q" . implode("$q,$q", $array) . "$q"; //Displays 'A', 'B', 'C'
}

// todo: escape, slashses or javascript ? 
function quoted_list($array, $quote="'") {
  //$array = array_map('addslashes', $array);
  return $quote.implode($quote.','.$quote, $array).$quote;
}

function quote($val, $quote="'") {
  return $quote.addslashes($val).$quote;
}

// return protocol hostname 
/**
function full_domain() {
  $pu = parse_url($_SERVER['REQUEST_URI']);
  return $pu["scheme"] . "://" . $pu["host"];
}
*/

// takes an assoc array (a=>1, b=>2, c=>3) or object returns string like a=1;b=2;c=3
// no quotes added around value (must do in passed array)
// by default, do not quote variables beginning with $
function array2filters($farray, $outer_delim=';', $inner_delim='=', $quote=false, $except_variables=true) {
  $array = array();
  foreach($farray as $k => $v) {
    $do_quote = $quote && ($v[0] != '$' || !$except_variables);
    $array[] = $k.$inner_delim.($do_quote ? quote($v) : $v);
  }
  
  return implode($outer_delim, $array);
}

// takes assoc. array or object (a=>1, b=>2, c=>3)
// returns a='1' b='2' c='3'
// if value begins with $, do not quote (smarty variable)
function array2args($farray) {
  return array2filters($farray, ' ', '=', true);
}

// takes css string like #foo{height:3pxx} #bar{width: 10px;} and returns array ('#foo' => "css", #bar => "css")
/** unsused - automatic comment added on  */
function css2array($css) {
  $reg_exp = '/\s*(.*)\s*\{(.*)\}\s*/';
  if($count = preg_match_all($reg_exp, $css, $m)) {
    //echo("css = $css<br>");
    for($i=0;$i<$count;$i++) {
      $ar[$m[1][$i]] = $m[2][$i];
    }
  }
  return $ar;
}

function clean_scayt($text) {
  $text = str_replace(array("\n", "&nbsp;"), array("", " "), $text);
  $word = 
  $pattern = '#<span\s*data-scayt_word\s*=\s*"([^"]+)"\s*data-scaytid\s*=\s*"[0-9]+">([^>]+)</span>#';
  $clean_text = preg_replace($pattern, "$1", $text, -1, $count);
  $clean_text = preg_replace('#\s+#', " ", $clean_text, -1, $count); // replace one or more spaces with one
  $clean_text = str_replace(array("<p> ", " </p>", " <p>", "</p> "), array("<p>", "</p>", "<p>", "</p>"), $clean_text); // remove whitespace just before and after paragraph markers
  return $clean_text;
}

// cleans all property descriptions with scaytid
/** unsused - automatic comment added on  */
function clean_pd_scayt() {
  $descs = find_objects('property_description', array('description' => '*scaytid*'));
  $count = count($descs);

  foreach($descs as $descObj) {
    $desc = clean_scayt($descObj->description);
    $id = $descObj->id;
    if(strpos($desc, 'scayt')) {
      $dirty++;
      if(is_property_type($descObj->property_type)) $apt = get_object($descObj->property_type, $descObj->property_id);
      $output .= "$id = not clean: '$apt->name': $descObj->property_type $descObj->property_id<br>$desc".form_text('foo', $desc, array('rows' => 10, 'cols' => 140));
    } else {
      $clean++;
    }
    $desc = addslashes($desc);
    db_query("UPDATE property_description SET description = '$desc' WHERE id=$id");
  }
  
  echo("total=$count clean=$clean dirty=$dirty<br>".$output);
}

// reutrns a bityly shortened URL
function url_shorten($url) {
  $login = 'kjakman'; // Bitly User id
  $key = 'R_a360b2026d81ca71e8c32b6d63a6bbc6'; // Bitly API key
  $service = "https://api-ssl.bitly.com/v3/shorten?login=$login&apiKey=$key&longUrl=".urlencode(add_http($url))."&format=json";
  $json = get_html($service);
  $result = json_decode($json);
  return $result->status_code == 200 ? $result->data->url : $url; 
}

function unit_string($unit, $num_units, $options=NULL) {
  $bold = $options['bold'];
  $num_units_str = formatFloat($num_units, FORMAT_IF_NEEDED);
  $unit_str = clean_num($num_units) == 1 ? phrase($unit, LOWERCASE) : phrase($unit.'s', LOWERCASE);
  if(!$options['show_count']) return $unit_str; // just hour(s), day(s) etc
  return ($bold ? html_b($num_units_str) : $num_units_str).' '.$unit_str; // 4 days, 1 hour etc
}


/** unsused - automatic comment added on  */
function uom_name($unit) {      
  switch($unit) {
    case 'PP':
      return 'p.p';
    case 'HR':
      return 'p.hr';
    case 'PPHR':
      return 'p.p/hr';
    case 'EA':
      return 'p/ea';
    default:
      return '';
      break;
  }
}

// searches tree (array of objects)
// returns node with key=$key or with options['search']['key'] = val
function tree_find($tree, $key='', $options=NULL) {
  $child_field = $options['child_field'] ?: 'children';  
  $search = $options['search'];
  if(is_array($options['search'])) list($search_key, $val) = $search; 
  foreach($tree as $path => $node) {
    if($path == $key) return $node;
    else if($search_key && $node->$search_key == $val) return $node;
    if($node->$child_field && is_array($node->$child_field) && count($node->$child_field)) {
      if($node = tree_find($node->$child_field, $key, $options)) return $node;      
    }
  }
  return NULL;
}

// recursively counts the total number of nodes in a tree (array of objects, where the childfield can be another array of objects)
// search: an array with (key, value); only count nodes that match 
function tree_count($tree, $options=NULL) {
  $count = 0;
  $search = $options['search'];
  $child_field = $options['child_field'] ?: 'children';  
  if(is_array($search)) list($key, $val) = $search; 
  foreach($tree as $path => $node) {
    if(!$search || $node->$key == $val) $count++;
    $nc = count($node->$child_field);
    if($node->$child_field && is_array($node->$child_field) && count($node->$child_field)) {
      $count += tree_count($node->$child_field, $options);
    }
  }
  return $count;
}

function tree_count_debug($tree, $options=NULL) {
  $count = 0;
  $options['level']++;
  $level =$options['level']; 
  $search = $options['search'];
  $child_field = $options['child_field'] ?: 'children';  
  $indent = str_repeat('&nbsp;', $level); 
  if(is_array($search)) list($key, $val) = $search; 
  foreach($tree as $path => $node) {
    $nc = count($node->$child_field);
    if(!$search || $node->$key == $val) {
      $count++;
      echo("$indent $level:$node->name matched ($nc children), count = $count<br>");
    }
    if($node->$child_field && is_array($node->$child_field) && count($node->$child_field)) {
      $cc = tree_count_debug($node->$child_field, $options);
      $count += $cc;
      //echo("$indent $level:$node->name children returned $cc, count = $count<br>");
    }
  }
  return $count;
}

// returns a link for given operation to /ajax.php
function ajax_operation_link($operation, $data=NULL, $options=NULL) {
  $public = $options['public'] || $options['unauthenticated'];
  $ajax_url = "/ajax.php?oper=$operation";
  if($data = (array) $data) $ajax_url .= "&".array2qs($data); 
  return $ajax_url;
}

/** unsused - automatic comment added on  */
function distance($lat1, $lng1, $lat2, $lng2, $miles = false) {
	$pi80 = M_PI / 180;
	$lat1 *= $pi80;
	$lng1 *= $pi80;
	$lat2 *= $pi80;
	$lng2 *= $pi80;
 
	$r = 6372.797; // mean radius of Earth in km
	$dlat = $lat2 - $lat1;
	$dlng = $lng2 - $lng1;
	$a = sin($dlat / 2) * sin($dlat / 2) + cos($lat1) * cos($lat2) * sin($dlng / 2) * sin($dlng / 2);
	$c = 2 * atan2(sqrt($a), sqrt(1 - $a));
	$km = $r * $c;
 
	return ($miles ? ($km * 0.621371192) : $km);
}

function call_function($function, $params) {
  if(!$function) return "load-function: missing function name";  
  if(!function_exists($function)) return "load-function: $function does not exist";
  if(isset($params['param4'])) return call_user_func($function, $params['param1'], $params['param2'], $params['param3'], $params['param4']); 
  if(isset($params['param3'])) return call_user_func($function, $params['param1'], $params['param2'], $params['param3']); 
  if(isset($params['param2'])) return call_user_func($function, $params['param1'], $params['param2']); 
  if(isset($params['param1'])) return call_user_func($function, $params['param1']);
  return call_user_func($function);
}

// returns the foreign key of object (json)
function object_foreign_key($obj_type, $obj_or_id, $field='fids') {
  return is_object($obj_or_id) ? $obj_or_id->$field :  get_object($obj_type, $obj_or_id, $field);
}

// returns the value corresponding to key in json 
function json_get_value($json, $key) {
  $fidArray = json_decode($json, true);
  return $fidArray[$key];
}

// returns the foreign id of object (json) for given key 
function object_foreign_id($obj_type, $obj_or_id, $key, $field='fids') {
  $fids = object_foreign_key('property', $obj_or_id, $field);
  return json_get_value($fids, $key);
}

// http://stackoverflow.com/questions/9802788/call-a-rest-api-in-php
// URL: basic url of service, e.g.:  https:// app.kigo.net /api/ra/v1/
// Method: method of API, e.g. getuser
// HTTP Method: POST, PUT, GET etc
// Data: array("param" => "value") ==> index.php?param=value
function rest_api_call($url, $api_method='', $data = array(), $options = array()) {

  $log_level = LOG_LEVEL_ALWAYS;
  $debug = $options['debug'];
  $log_level_debug = $options['debug'] ? LOG_LEVEL_ALWAYS : LOG_LEVEL_TEST;
  //$log_level = LOG_LEVEL_ALWAYS;  
  $http_method = $options['http_method'] ?: 'GET';

  print_log(now()." REST API: url=$url api-method=$api_method http-method=$http_method", 'api-rest', $log_level);

  $username = $options['username'];
  $password = $options['password'];
  $ssl = $options['secure'] || $options['ssl'] || stripos($url, 'https') === 0; 
  $ch = curl_init();
  $full_url = $api_method ? clean_path("$url/$api_method") : $url;

  print_log("REST API options=".dump($options, true), 'api-rest', $log_level_debug);  

  if(is_object($options)) {
    die("options is object:".dump($options, true));
  }
  //if(is_object($data)) {
  //  $file = $options['file'];
  //  dump($options);
  //  die("file=$file data is object:".dump($data, true));
  //}
  $file = $options['file'];
  $file_type = $options['file_type'];
  if($file && is_array($data)) {
    $options['content_type'] = "multipart/form-data";
    $options['encode_data'] = false;
    $curl_file = "@".realpath($file);
    if($file_type) $curl_file .= ";type=$file_type";
    $file_field = $options['file_field'] ?: 'file';
    $data[$file_field] = $curl_file; // POST MULTIPART
    print_log("REST API Upload file=$file field=$file_field: curl=$curl_file", 'api-rest', $log_level_debug);
  
  }

                             
  switch ($http_method) {
    case HTTP_POST:
    case HTTP_POST_MULTIPART:
    case "POST":        
    case "POST_MULTIPART":
      curl_setopt($ch, CURLOPT_POST, 1);
      if($data) {
        if($options['encode_data']) {
          $data_string = is_array($data) || is_object($data) ? json_encode($data) : $data;
          $data_len = strlen($data_string);
          print_log("REST API data (json string) len=$data_len\n\npost_data=$data_string ".dump($data, true), 'api-rest', $log_level_debug);
          if($data_string) {
            print_log("REST API setting post data len=$data_len post_data=$data_string ", 'api-rest', $log_level_debug);
            curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);
          }
        } else { // send data as array 
          print_log("REST API setting post data ARRAY or OBJECT=".dump($data, true), 'api-rest', $log_level_debug);
          curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
        }
      } else {
        print_log("REST API NO DATA: ".dump($data, true), 'api-rest', $log_level_debug);
        curl_setopt($ch, CURLOPT_POSTFIELDS, array());
      }
      break;
      
    case HTTP_PUT:
    case "PUT":
      curl_setopt($ch, CURLOPT_PUT, 1);
      break;

    case HTTP_DELETE:
    case "DELETE":
      curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "DELETE");
      break;

    case HTTP_GET:
    case "GET":
    default: // GET
      if($data) {
        print_log("REST API setting GET data ARRAY or OBJECT=".dump($data, true), 'api-rest', $log_level_debug);
        $full_url = sprintf("%s?%s", $full_url, http_build_query((array) $data));
      }
      print_log("REST API HTTP_GET url=$full_url", 'api-rest', $log_level_debug);
      break;
  }

  $http_header = array();
  if($access_token = $options['access_token']) $http_header[] = "Authorization: Bearer $access_token";
  if($content_type = $options['content_type']) $http_header[] = "Content-Type: $content_type";
  if($data_len) $http_header[] = "Content-Length: $data_len";
  if($accept = $options['accept']) $http_header[] = "Accept: $content_type";
  if($http_header) curl_setopt($ch, CURLOPT_HTTPHEADER, $http_header);  

  print_log("REST API content header=".dump($http_header, true), 'api-rest', $log_level_debug);

  $get_headers = isset($options['get_headers']) ? $options['get_headers'] : 1;  
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_VERBOSE, 1);
  curl_setopt($ch, CURLOPT_HEADER, $get_headers);
  if($ssl) {
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
  }
  //curl_setopt($ch, CURLOPT_HTTPHEADER, Array("Accept: application/json"));

  if($port = $options['port']) curl_setopt($ch, CURLOPT_PORT, $port);
  
  $log_output = $options['log_output'];
  $log_output = 1;
  if($log_output) { // log what we send
    curl_setopt($ch, CURLINFO_HEADER_OUT, 1);
  }
  
  // Optional Authentication:
  if($username && $password) {
    print_log("REST API Authentication: u=$username p=$password", 'api-rest', $log_level_debug);    
    curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_setopt($ch, CURLOPT_USERPWD, "$username:$password");
  } 
  
  curl_setopt($ch, CURLOPT_URL, $full_url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

  $response = curl_exec($ch);
  $len = strlen($response);

  if($debug) {
    $curlVersion = curl_version();
    extract(curl_getinfo($ch));
    $metrics = <<<EOD
    URL....: $url
    Code...: $http_code ($redirect_count redirect(s) in $redirect_time secs)
    Content: $content_type Size: $download_content_length (Own: $size_download) Filetime: $filetime
    Time...: $total_time Start @ $starttransfer_time (DNS: $namelookup_time Connect: $connect_time Request: $pretransfer_time)
    Speed..: Down: $speed_download (avg.) Up: $speed_upload (avg.)
    Curl...: v{$curlVersion['version']}
EOD;
    print_log("REST API debug: $metrics".PHP_EOL, 'api-rest', $log_level);
  }
  
  if(!$get_headers) {
    //print_log("REST API call: url=$full_url method=$http_method data-length=".(is_array($data) || is_object($data) ? count($data) : strlen($data)).PHP_EOL, 'api-rest', $log_level);
    print_log("REST API: url=$url api-method=$api_method http-method=$http_method response length=$len returning response since get_headers=$get_headers", 'api-rest', $log_level);
    return $response;
  }
  
  $header_size = curl_getinfo($ch, CURLINFO_HEADER_SIZE);
  $header = substr($response, 0, $header_size);
  $body = substr($response, $header_size);
  $body_len = strlen($body);
  
  $headers = http_parse_headers($header);
  $content_disp = $headers['Content-Disposition'] ?: '';
  $content_len = $headers['Content-Length'] ?: $body_len;
  $content_type = $headers['Content-Type'];
  $filename = $options['filename'] ?: http_header_filename($content_disp);
  $file_size = $options['filesize']; // expected file size
  $download_path = $options['download_path'];
  $bytes = 0;
  if($download_path && $filename && $content_len) {
    $result = new stdClass;
    $dest = clean_path("$download_path/$filename");           
    $result->filename = $filename;
    $result->type = $content_type;
    if($content_type == 'application/json') { // not what we expected
      print_log("REST API: download failed, received JSON (path=$dest type=$content_type length=$content_len) result=".dump($body, true), 'api-rest', $log_level_debug);
      return $body;      
    }
    
    $result->path = $dest;
    $result->size = $content_len;
    if($file_size && $content_len != $file_size) {
      $result->error = "size=$content_len expected $file_size";
      $result->success = 0;
    } else if($bytes = write_file($dest, $body)) {
      $result->success = 1;
      $result->size = $bytes;
      //$result = '{"file_size": "'.$result.'","file_name": "'.$file_name.'","file_type": "'.$content_type.'","file_path": "'.$dest.'",}';
    } else {
      $result->success = 0;
    }      
    print_log("REST API: download to $dest (type=$content_type length=$content_len) written=$bytes result=".dump($result, true), 'api-rest', $log_level_debug);
    return json_encode($result);
  }
  
  if($options['set_headers']) {  
    if($cookies = $headers['Set-Cookie']) {
      if($options['reverse_cookies']) $cookies = array_reverse($rcookies); // sanjeev, no idea if needed
      if(is_array($cookies)) {
        foreach($cookies as $cookie) {
          print_log("REST API: cookie set: $cookie", 'api-rest', $log_level_debug);    
          header('Set-Cookie: '.$cookie, false);
        }
      } else {
        print_log("REST API: cookie set: $cookie", 'api-rest', $log_level_debug);    
        header('Set-Cookie: '.$cookie, false);
      }
    }
  }

  if($log_output) { // log what we send
    $header_sent = curl_getinfo($ch, CURLINFO_HEADER_OUT);
    print_log("REST API: header sent=\n$header_sent\n------\n", 'api-rest', $log_level_debug);    
  }
  //print_log("REST API: response length=$len get_headers=$get_headers header size=$header_size", 'api-rest', $log_level);
  // below for loggin
  if(!$api_method && is_array($data)) $api_method = $data['method']; // for logging
  if($http_method == HTTP_GET) $http_method = "GET";
  else if($http_method == HTTP_PUT) $http_method = "PUT";
  else if($http_method == HTTP_DELETE) $http_method = "DELETE";
  else if($http_method == HTTP_POST) $http_method = "POST";
  else if($http_method == HTTP_POST_MULTIPART) $http_method = "POST MULTIPART";
  print_log("REST API: api-method=$api_method url=$url ($http_method) response-length=$len header-size=$header_size", 'api-rest', $log_level);
  print_log("header=\n$header\n------\n", 'api-rest', $log_level_debug);
  //if($body_len < 1000) print_log("body=\n$body\n------\n", 'api-rest', $log_level_debug);
  return $body;
}

function http_header_filename($content_disp) {
  if(!$content_disp) return '';
  $filename = '';
  if($pos = strpos($content_disp, 'filename="')) { // download
    $filename = substr($content_disp, $pos+9);
    $filename = trim($filename, '"');
    $filename = str_replace(' ', '_', $filename);
  }
  print_log("http_header_filename: in=$content_disp out=$filename", 'api-rest', $log_level);
  return $filename;
  
}


// Converts DMS ( Degrees / minutes / seconds ) 
// to decimal format longitude / latitude
/** unsused - automatic comment added on  */
function dms2dec($deg,$min,$sec,$dir='N') {
  $dir = strtoupper($dir);
  $sign = $dir == 'S' || $dir == 'W' ? -1 : 1;
  return $sign * $deg+((($min*60)+($sec))/3600);
}

// PECL http_parse_headers alternative
if( !function_exists( 'http_parse_headers')) {
   function http_parse_headers($header) {
     $retVal = array();
     $fields = explode("\r\n", preg_replace('/\x0D\x0A[\x09\x20]+/', ' ', $header));
     foreach( $fields as $field ) {
       if( preg_match('/([^:]+): (.+)/m', $field, $match) ) {
         $match[1] = preg_replace('/(?<=^|[\x09\x20\x2D])./e', 'strtoupper("\0")', strtolower(trim($match[1])));
         if( isset($retVal[$match[1]]) ) {
           $retVal[$match[1]] = array($retVal[$match[1]], $match[2]);
         } else {
           $retVal[$match[1]] = trim($match[2]);
         }
       }
     }
     return $retVal;
   }
}

// takes an array of objects. Writes as CSV to file
/** unsused - automatic comment added on  */
function array2csv($array, $file) {

  // Notice, you can only use a single character as a delimiter
  $delimiter = ',';
  
  if (count($array) > 0) {
    // prepare the file
    $fp = fopen($file, 'w');
    if(!$fp) die("Could not write to $file");
  
    // Save header
    $header = array_keys((array)$array[0]);
    fputcsv($fp, $header, $delimiter);
  
    // Save data
    foreach ($array as $element) {
        fputcsv($fp, (array)$element, $delimiter);
    }
  }
}


/** unsused - automatic comment added on  */
function permissions_edit($perms) {
  return 'foo';
  $perms = json_decode($permsJson, true);
  if(!$perms || !is_array($perms)) return '';
  $ar = [];
  foreach($perms as $k => $v) {
    $ar[]= ucfirst($v)." {$k}";
  }
  return implode(', ', $ar);
}

/** unsused - automatic comment added on  */
function permissions_print($permsJson) {
  $perms = json_decode($permsJson, true);
  if(!$perms || !is_array($perms)) return '';
  $ar = [];
  foreach($perms as $k => $v) {
    $ar[]= ucfirst($v)." {$k}";
  }
  return implode(', ', $ar);
}

/**
 * Returns a "safe" version of the given string - basically only US-ASCII and
 * numbers. Needed because filenames and titles and such, can't use all characters.
 *
 * @param  string  $str
 * @param  boolean $strict
 * @param  string  $extrachars
 * @return string
 */

function makeSafe($str, $strict = false, $extrachars = "") {
  // $str = \URLify::downcode($str);
  $str = str_replace(array("'", '"', "&amp;", "&quot;", "&hellip;", "&ldquo;", "&rdquo;", "&lsquo;", "&rsquo;", "&ndash;"), "", $str);
  $str = str_replace(array("&#039;", "&#39;"), '-', $str);
  $str = str_replace(array("&euml;"), array('e'), $str);

  $delim = '/';
  if ($extrachars != "") {
      $extrachars = preg_quote($extrachars, $delim);
  }
  if ($strict) {
      $str = strtolower(str_replace(" ", "-", $str));
      $regex = "[^a-zA-Z0-9_" . $extrachars . "-]";
  } else {
      $regex = "[^a-zA-Z0-9 _.," . $extrachars . "-]";
  }

  $str = preg_replace($delim . $regex . $delim, '', $str);

  return $str;
}

/**
 * Modify a string, so that we can use it for slugs. Like
 * safeString, but using hyphens instead of underscores.
 *
 * @param  string $str
 * @param  int    $length
 * @return string
 */

function slug($str, $length = 64) {
  if (is_array($str)) {
      $str = implode(" ", $str);
  }

  // Strip out timestamps like "00:00:00". We don't want timestamps in slugs.
  $str = preg_replace("/[0-2][0-9]:[0-5][0-9]:[0-5][0-9]/", "", $str);

  $str = makeSafe(strip_tags($str));

  $str = str_replace(" ", "-", $str);
  $str = transliterate($str);
  $str = strtolower(preg_replace("/[^a-zA-Z0-9_-]/i", "", $str));
  $str = preg_replace("/[-]+/i", "-", $str);

  if ($length > 0) {
      $str = substr($str, 0, $length);
  }

  $str = trim($str, " -");

  return $str;
}

// http://stackoverflow.com/questions/2955251/php-function-to-make-slug-url-string
function slugify($str, $max=128) { 
  $str = preg_replace("/[0-2][0-9]:[0-5][0-9]:[0-5][0-9]/", "", strip_tags($str)); // remove tags and timestamps

  $str = str_replace(" ", "-", $str);
  $str = transliterate($str);

  //$str = preg_replace('~[^\\pL\d]+~u', '', $str); // replace non letter or digits by -
  $str = strtolower(preg_replace("/[^a-zA-Z0-9_-]/i", "", $str)); // remove non-letter/digits
  $str = preg_replace("/[-]+/i", "-", $str);

  $str = trim($str, '-'); // trim  
  //$str = iconv('utf-8', 'us-ascii//TRANSLIT', $str); // transliterate

  $str = strtolower($str); // lowercase
  $str = preg_replace('~[^-\w]+~', '', $str); // remove unwanted characters
  if (empty($str)) return 'n-a';

  // leave 4 characters to allow object_slug to iterate slug-1, slug-2 etc
  $slug = strlen($str) > $max - 4 ? substr($str, 0, $max - 4 ) : $str;
  return $slug;  
}

// Replace "funny" characters with ascii according to below rule set
function transliterate($str) {
  $rules = transliterate_rules();
  return str_replace(array_keys($rules), array_values($rules), $str);
}

// from https://github.com/cocur/slugify/blob/master/src/Slugify.php
function transliterate_rules() {
  $rules = array(
  // Numeric characters
  '¹' => 1,
  '²' => 2,
  '³' => 3,
  // Latin
  '°' => 0,
  'æ' => 'ae',
  'ǽ' => 'ae',
  'À' => 'A',
  'Á' => 'A',
  'Â' => 'A',
  'Ã' => 'A',
  'Å' => 'A',
  'Ǻ' => 'A',
  'Ă' => 'A',
  'Ǎ' => 'A',
  'Æ' => 'AE',
  'Ǽ' => 'AE',
  'à' => 'a',
  'á' => 'a',
  'â' => 'a',
  'ã' => 'a',
  'å' => 'a',
  'ǻ' => 'a',
  'ă' => 'a',
  'ǎ' => 'a',
  'ª' => 'a',
  '@' => 'at',
  'Ĉ' => 'C',
  'Ċ' => 'C',
  'ĉ' => 'c',
  'ċ' => 'c',
  '©' => 'c',
  'Ð' => 'Dj',
  'Đ' => 'D',
  'ð' => 'dj',
  'đ' => 'd',
  'È' => 'E',
  'É' => 'E',
  'Ê' => 'E',
  'Ë' => 'E',
  'Ĕ' => 'E',
  'Ė' => 'E',
  'è' => 'e',
  'é' => 'e',
  'ê' => 'e',
  'ë' => 'e',
  'ĕ' => 'e',
  'ė' => 'e',
  'ƒ' => 'f',
  'Ĝ' => 'G',
  'Ġ' => 'G',
  'ĝ' => 'g',
  'ġ' => 'g',
  'Ĥ' => 'H',
  'Ħ' => 'H',
  'ĥ' => 'h',
  'ħ' => 'h',
  'Ì' => 'I',
  'Í' => 'I',
  'Î' => 'I',
  'Ï' => 'I',
  'Ĩ' => 'I',
  'Ĭ' => 'I',
  'Ǐ' => 'I',
  'Į' => 'I',
  'Ĳ' => 'IJ',
  'ì' => 'i',
  'í' => 'i',
  'î' => 'i',
  'ï' => 'i',
  'ĩ' => 'i',
  'ĭ' => 'i',
  'ǐ' => 'i',
  'į' => 'i',
  'ĳ' => 'ij',
  'Ĵ' => 'J',
  'ĵ' => 'j',
  'Ĺ' => 'L',
  'Ľ' => 'L',
  'Ŀ' => 'L',
  'ĺ' => 'l',
  'ľ' => 'l',
  'ŀ' => 'l',
  'Ñ' => 'N',
  'ñ' => 'n',
  'ŉ' => 'n',
  'Ò' => 'O',
  'Ô' => 'O',
  'Õ' => 'O',
  'Ō' => 'O',
  'Ŏ' => 'O',
  'Ǒ' => 'O',
  'Ő' => 'O',
  'Ơ' => 'O',
  'Ø' => 'O',
  'Ǿ' => 'O',
  'Œ' => 'OE',
  'ò' => 'o',
  'ô' => 'o',
  'õ' => 'o',
  'ō' => 'o',
  'ŏ' => 'o',
  'ǒ' => 'o',
  'ő' => 'o',
  'ơ' => 'o',
  'ø' => 'o',
  'ǿ' => 'o',
  'º' => 'o',
  'œ' => 'oe',
  'Ŕ' => 'R',
  'Ŗ' => 'R',
  'ŕ' => 'r',
  'ŗ' => 'r',
  'Ŝ' => 'S',
  'Ș' => 'S',
  'ŝ' => 's',
  'ș' => 's',
  'ſ' => 's',
  'Ţ' => 'T',
  'Ț' => 'T',
  'Ŧ' => 'T',
  'Þ' => 'TH',
  'ţ' => 't',
  'ț' => 't',
  'ŧ' => 't',
  'þ' => 'th',
  'Ù' => 'U',
  'Ú' => 'U',
  'Û' => 'U',
  'Ũ' => 'U',
  'Ŭ' => 'U',
  'Ű' => 'U',
  'Ų' => 'U',
  'Ư' => 'U',
  'Ǔ' => 'U',
  'Ǖ' => 'U',
  'Ǘ' => 'U',
  'Ǚ' => 'U',
  'Ǜ' => 'U',
  'ù' => 'u',
  'ú' => 'u',
  'û' => 'u',
  'ũ' => 'u',
  'ŭ' => 'u',
  'ű' => 'u',
  'ų' => 'u',
  'ư' => 'u',
  'ǔ' => 'u',
  'ǖ' => 'u',
  'ǘ' => 'u',
  'ǚ' => 'u',
  'ǜ' => 'u',
  'Ŵ' => 'W',
  'ŵ' => 'w',
  'Ý' => 'Y',
  'Ÿ' => 'Y',
  'Ŷ' => 'Y',
  'ý' => 'y',
  'ÿ' => 'y',
  'ŷ' => 'y',
  // Russian
  'Ъ' => '',
  'Ь' => '',
  'А' => 'A',
  'Б' => 'B',
  'Ц' => 'C',
  'Ч' => 'Ch',
  'Д' => 'D',
  'Е' => 'E',
  'Ё' => 'E',
  'Э' => 'E',
  'Ф' => 'F',
  'Г' => 'G',
  'Х' => 'H',
  'И' => 'I',
  'Й' => 'J',
  'Я' => 'Ja',
  'Ю' => 'Ju',
  'К' => 'K',
  'Л' => 'L',
  'М' => 'M',
  'Н' => 'N',
  'О' => 'O',
  'П' => 'P',
  'Р' => 'R',
  'С' => 'S',
  'Ш' => 'Sh',
  'Щ' => 'Shch',
  'Т' => 'T',
  'У' => 'U',
  'В' => 'V',
  'Ы' => 'Y',
  'З' => 'Z',
  'Ж' => 'Zh',
  'ъ' => '',
  'ь' => '',
  'а' => 'a',
  'б' => 'b',
  'ц' => 'c',
  'ч' => 'ch',
  'д' => 'd',
  'е' => 'e',
  'ё' => 'e',
  'э' => 'e',
  'ф' => 'f',
  'г' => 'g',
  'х' => 'h',
  'и' => 'i',
  'й' => 'j',
  'я' => 'ja',
  'ю' => 'ju',
  'к' => 'k',
  'л' => 'l',
  'м' => 'm',
  'н' => 'n',
  'о' => 'o',
  'п' => 'p',
  'р' => 'r',
  'с' => 's',
  'ш' => 'sh',
  'щ' => 'shch',
  'т' => 't',
  'у' => 'u',
  'в' => 'v',
  'ы' => 'y',
  'з' => 'z',
  'ж' => 'zh',
  // German characters
  'Ä' => 'AE',
  'Ö' => 'OE',
  'Ü' => 'UE',
  'ß' => 'ss',
  'ä' => 'ae',
  'ö' => 'oe',
  'ü' => 'ue',
  // Turkish characters
  'Ç' => 'C',
  'Ğ' => 'G',
  'İ' => 'I',
  'Ş' => 'S',
  'ç' => 'c',
  'ğ' => 'g',
  'ı' => 'i',
  'ş' => 's',
  // Latvian
  'Ā' => 'A',
  'Ē' => 'E',
  'Ģ' => 'G',
  'Ī' => 'I',
  'Ķ' => 'K',
  'Ļ' => 'L',
  'Ņ' => 'N',
  'Ū' => 'U',
  'ā' => 'a',
  'ē' => 'e',
  'ģ' => 'g',
  'ī' => 'i',
  'ķ' => 'k',
  'ļ' => 'l',
  'ņ' => 'n',
  'ū' => 'u',
  // Ukrainian
  'Ґ' => 'G',
  'І' => 'I',
  'Ї' => 'Ji',
  'Є' => 'Ye',
  'ґ' => 'g',
  'і' => 'i',
  'ї' => 'ji',
  'є' => 'ye',
  // Czech
  'Č' => 'C',
  'Ď' => 'D',
  'Ě' => 'E',
  'Ň' => 'N',
  'Ř' => 'R',
  'Š' => 'S',
  'Ť' => 'T',
  'Ů' => 'U',
  'Ž' => 'Z',
  'č' => 'c',
  'ď' => 'd',
  'ě' => 'e',
  'ň' => 'n',
  'ř' => 'r',
  'š' => 's',
  'ť' => 't',
  'ů' => 'u',
  'ž' => 'z',
  // Polish
  'Ą' => 'A',
  'Ć' => 'C',
  'Ę' => 'E',
  'Ł' => 'L',
  'Ń' => 'N',
  'Ó' => 'O',
  'Ś' => 'S',
  'Ź' => 'Z',
  'Ż' => 'Z',
  'ą' => 'a',
  'ć' => 'c',
  'ę' => 'e',
  'ł' => 'l',
  'ń' => 'n',
  'ó' => 'o',
  'ś' => 's',
  'ź' => 'z',
  'ż' => 'z',
  // Greek
  'Α' => 'A',
  'Β' => 'B',
  'Γ' => 'G',
  'Δ' => 'D',
  'Ε' => 'E',
  'Ζ' => 'Z',
  'Η' => 'E',
  'Θ' => 'Th',
  'Ι' => 'I',
  'Κ' => 'K',
  'Λ' => 'L',
  'Μ' => 'M',
  'Ν' => 'N',
  'Ξ' => 'X',
  'Ο' => 'O',
  'Π' => 'P',
  'Ρ' => 'R',
  'Σ' => 'S',
  'Τ' => 'T',
  'Υ' => 'Y',
  'Φ' => 'Ph',
  'Χ' => 'Ch',
  'Ψ' => 'Ps',
  'Ω' => 'O',
  'Ϊ' => 'I',
  'Ϋ' => 'Y',
  'ά' => 'a',
  'έ' => 'e',
  'ή' => 'e',
  'ί' => 'i',
  'ΰ' => 'Y',
  'α' => 'a',
  'β' => 'b',
  'γ' => 'g',
  'δ' => 'd',
  'ε' => 'e',
  'ζ' => 'z',
  'η' => 'e',
  'θ' => 'th',
  'ι' => 'i',
  'κ' => 'k',
  'λ' => 'l',
  'μ' => 'm',
  'ν' => 'n',
  'ξ' => 'x',
  'ο' => 'o',
  'π' => 'p',
  'ρ' => 'r',
  'ς' => 's',
  'σ' => 's',
  'τ' => 't',
  'υ' => 'y',
  'φ' => 'ph',
  'χ' => 'ch',
  'ψ' => 'ps',
  'ω' => 'o',
  'ϊ' => 'i',
  'ϋ' => 'y',
  'ό' => 'o',
  'ύ' => 'y',
  'ώ' => 'o',
  'ϐ' => 'b',
  'ϑ' => 'th',
  'ϒ' => 'Y',
  /* Arabic */
  'أ' => 'a',
  'ب' => 'b',
  'ت' => 't',
  'ث' => 'th',
  'ج' => 'g',
  'ح' => 'h',
  'خ' => 'kh',
  'د' => 'd',
  'ذ' => 'th',
  'ر' => 'r',
  'ز' => 'z',
  'س' => 's',
  'ش' => 'sh',
  'ص' => 's',
  'ض' => 'd',
  'ط' => 't',
  'ظ' => 'th',
  'ع' => 'aa',
  'غ' => 'gh',
  'ف' => 'f',
  'ق' => 'k',
  'ك' => 'k',
  'ل' => 'l',
  'م' => 'm',
  'ن' => 'n',
  'ه' => 'h',
  'و' => 'o',
  'ي' => 'y',
  /* Vietnamese */
  'ạ' => 'a',
  'ả' => 'a',
  'ầ' => 'a',
  'ấ' => 'a',
  'ậ' => 'a',
  'ẩ' => 'a',
  'ẫ' => 'a',
  'ằ' => 'a',
  'ắ' => 'a',
  'ặ' => 'a',
  'ẳ' => 'a',
  'ẵ' => 'a',
  'ẹ' => 'e',
  'ẻ' => 'e',
  'ẽ' => 'e',
  'ề' => 'e',
  'ế' => 'e',
  'ệ' => 'e',
  'ể' => 'e',
  'ễ' => 'e',
  'ị' => 'i',
  'ỉ' => 'i',
  'ọ' => 'o',
  'ỏ' => 'o',
  'ồ' => 'o',
  'ố' => 'o',
  'ộ' => 'o',
  'ổ' => 'o',
  'ỗ' => 'o',
  'ờ' => 'o',
  'ớ' => 'o',
  'ợ' => 'o',
  'ở' => 'o',
  'ỡ' => 'o',
  'ụ' => 'u',
  'ủ' => 'u',
  'ừ' => 'u',
  'ứ' => 'u',
  'ự' => 'u',
  'ử' => 'u',
  'ữ' => 'u',
  'ỳ' => 'y',
  'ỵ' => 'y',
  'ỷ' => 'y',
  'ỹ' => 'y',
  'Ạ' => 'A',
  'Ả' => 'A',
  'Ầ' => 'A',
  'Ấ' => 'A',
  'Ậ' => 'A',
  'Ẩ' => 'A',
  'Ẫ' => 'A',
  'Ằ' => 'A',
  'Ắ' => 'A',
  'Ặ' => 'A',
  'Ẳ' => 'A',
  'Ẵ' => 'A',
  'Ẹ' => 'E',
  'Ẻ' => 'E',
  'Ẽ' => 'E',
  'Ề' => 'E',
  'Ế' => 'E',
  'Ệ' => 'E',
  'Ể' => 'E',
  'Ễ' => 'E',
  'Ị' => 'I',
  'Ỉ' => 'I',
  'Ọ' => 'O',
  'Ỏ' => 'O',
  'Ồ' => 'O',
  'Ố' => 'O',
  'Ộ' => 'O',
  'Ổ' => 'O',
  'Ỗ' => 'O',
  'Ờ' => 'O',
  'Ớ' => 'O',
  'Ợ' => 'O',
  'Ở' => 'O',
  'Ỡ' => 'O',
  'Ụ' => 'U',
  'Ủ' => 'U',
  'Ừ' => 'U',
  'Ứ' => 'U',
  'Ự' => 'U',
  'Ử' => 'U',
  'Ữ' => 'U',
  'Ỳ' => 'Y',
  'Ỵ' => 'Y',
  'Ỷ' => 'Y',
  'Ỹ' => 'Y',
  // burmese consonants
  'က'     => 'k',
  'ခ'     => 'kh',
  'ဂ'     => 'g',
  'ဃ'     => 'ga',
  'င'     => 'ng',
  'စ'     => 's',
  'ဆ'     => 'sa',
  'ဇ'     => 'z',
  'စျ'    => 'za',
  'ည'     => 'ny',
  'ဋ'     => 't',
  'ဌ'     => 'ta',
  'ဍ'     => 'd',
  'ဎ'     => 'da',
  'ဏ'     => 'na',
  'တ'     => 't',
  'ထ'     => 'ta',
  'ဒ'     => 'd',
  'ဓ'     => 'da',
  'န'     => 'n',
  'ပ'     => 'p',
  'ဖ'     => 'pa',
  'ဗ'     => 'b',
  'ဘ'     => 'ba',
  'မ'     => 'm',
  'ယ'     => 'y',
  'ရ'     => 'ya',
  'လ'     => 'l',
  'ဝ'     => 'w',
  'သ'     => 'th',
  'ဟ'     => 'h',
  'ဠ'     => 'la',
  'အ'     => 'a',
  // consonant character combos
  'ြ'     => 'y',
  'ျ'     => 'ya',
  'ွ'     => 'w',
  'ြွ'    => 'yw',
  'ျွ'    => 'ywa',
  'ှ'     => 'h',
  // independent vowels
  'ဧ'     => 'e',
  '၏'     => '-e',
  'ဣ'     => 'i',
  'ဤ'     => '-i',
  'ဉ'     => 'u',
  'ဦ'     => '-u',
  'ဩ'     => 'aw',
  'သြော'  => 'aw',
  'ဪ'     => 'aw',
  '၍'     => 'ywae',
  '၌'     => 'hnaik',
  // numbers
  '၀'     => '0',
  '၁'     => '1',
  '၂'     => '2',
  '၃'     => '3',
  '၄'     => '4',
  '၅'     => '5',
  '၆'     => '6',
  '၇'     => '7',
  '၈'     => '8',
  '၉'     => '9',
  // virama and tone marks which are silent in transliteration
  '္'     => '',
  '့'     => '',
  'း'     => '',
  // dependent vowels
  'ာ'     => 'a',
  'ါ'     => 'a',
  'ေ'     => 'e',
  'ဲ'     => 'e',
  'ိ'     => 'i',
  'ီ'     => 'i',
  'ို'    => 'o',
  'ု'     => 'u',
  'ူ'     => 'u',
  'ေါင်'  => 'aung',
  'ော'    => 'aw',
  'ော်'   => 'aw',
  'ေါ'    => 'aw',
  'ေါ်'   => 'aw',
  '်'     => 'at',
  'က်'    => 'et',
  'ိုက်'  => 'aik',
  'ောက်'  => 'auk',
  'င်'    => 'in',
  'ိုင်'  => 'aing',
  'ောင်'  => 'aung',
  'စ်'    => 'it',
  'ည်'    => 'i',
  'တ်'    => 'at',
  'ိတ်'   => 'eik',
  'ုတ်'   => 'ok',
  'ွတ်'   => 'ut',
  'ေတ်'   => 'it',
  'ဒ်'    => 'd',
  'ိုဒ်'  => 'ok',
  'ုဒ်'   => 'ait',
  'န်'    => 'an',
  'ာန်'   => 'an',
  'ိန်'   => 'ein',
  'ုန်'   => 'on',
  'ွန်'   => 'un',
  'ပ်'    => 'at',
  'ိပ်'   => 'eik',
  'ုပ်'   => 'ok',
  'ွပ်'   => 'ut',
  'န်ုပ်' => 'nub',
  'မ်'    => 'an',
  'ိမ်'   => 'ein',
  'ုမ်'   => 'on',
  'ွမ်'   => 'un',
  'ယ်'    => 'e',
  'ိုလ်'  => 'ol',
  'ဉ်'    => 'in',
  'ံ'     => 'an',
  'ိံ'    => 'ein',
  'ုံ'    => 'on'
  );
  return $rules;      
}

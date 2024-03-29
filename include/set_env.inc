<?php
# New version June 2018
$start_time = microtime(true); // start timer
$expires = 3600*24*30; // 30 days (in seconds)
set_time_limit(90); // increase max allowed execution time.

header('Content-Type:text/html; charset=UTF-8');
header('Expires: '.gmdate('D, d M Y H:i:s', time()+$expires).' GMT', false);
ini_set("session.cache_expire", 30 * 24 * 60); // 30 days
ini_set("session.gc_maxlifetime","10800"); // default is 1440, which is only 24 minutes
ini_set("session.bug_compat_warn","off");
ini_set("display_errors", 1);
define("UPPERCASE", 0);
define("LOWERCASE",1);
define("TITLECASE", 2);
define("CAPITALIZE", 3);
define("CAPS", 3);

error_reporting( error_reporting() & ~E_NOTICE & ~E_WARNING);

$app = new stdClass;

// domain 
$http_host = $_SERVER['HTTP_HOST']; // e.g. www.afr.com
$host_domains = explode(".", $http_host);
$host_tld = array_pop($host_domains); // e.g. com

$host_port = 80;
if(strpos($host_tld, ':')) list($host_tld, $host_port) = explode(":", $host_tld);
$host_dom = array_pop($host_domains); // e.g apartments-for-rent
$host_sub =  count($host_domains) ? array_pop($host_domains) : ''; // e.g. www
$host_domain = $site_domain = "$host_dom.$host_tld"; // e.g afr.com
$host_protocol = $_SERVER['HTTPS'] == 'on' ? "https" : "http";

$referer = $_SERVER['HTTP_REFERER'];

$system_cache_dir = $_SESSION['system_cache_dir'] = "/web/cache/";
$system_log_file = $_SESSION['system_log_file'] = $_SESSION['main_log'] = $main_log = 'system_log.txt';

$live = $host_sub == 'www' || $host_sub == '';
$doc_root = rtrim($_SERVER['DOCUMENT_ROOT'], '/').'/';

//$app_root = is_dir("{$doc_root}../afr/") ? "{$doc_root}../afr/" : "{$doc_root}/afr/";
//if(!is_dir($app_root)) die("App root {$app_root} not found");
                               
set_include_path(get_include_path().PATH_SEPARATOR."{$doc_root}afr-lib".PATH_SEPARATOR."{$doc_root}afr-lib/include".PATH_SEPARATOR."{$doc_root}afr-lib/includes".PATH_SEPARATOR."{$doc_root}afr-lib/classes".PATH_SEPARATOR."{$doc_root}afr-lib/includes/services".PATH_SEPARATOR."{$doc_root}afr-lib/modules".PATH_SEPARATOR."{$doc_root}afr".PATH_SEPARATOR."{$doc_root}../include");

session_start();       
$cookie_hash = $_COOKIE['authID'];
$session_hash = $_SESSION['hash']; /** need this for phpauth logout below, before session is destroyed */

if(strpos($_SERVER['REQUEST_URI'], 'act=logout')) {
  $user_id = $_SESSION['u_id'];
  $filename = "./user/manifest/manifest_".$user_id.".json";
  if (file_exists($filename)){
    $jsonString = file_get_contents($filename);
    $datas = json_decode($jsonString, true);
    if($datas['start_url']){
      $remove_uid = explode('/', $datas['start_url']);
      array_pop($remove_uid);
      $url =  implode('/', $remove_uid); 
      $datas['start_url'] = $url;
      $newJsonString = json_encode($datas);
      $putdata = file_put_contents($filename, $newJsonString);
    }
  }
  session_unset(); // Destroy session on log out
}

/** move these to $app. get rid of all session vars except those really needed (user/site/lang/deployment) */
$_SESSION['auth_token'] = $_SESSION['auth_token'] ?: md5(rand(0, 100000)); // random token
$_SESSION['version'] = $version = 2022100107482275; // used for cachebusting. Don't change, hotfix.sh depends on this exact line.
$_SESSION['debug_db'] = false;
$_SESSION['legacy'] = true; /** don't use site_application() */
$_SESSION['media_server'] = $beta ? 1 : 0;
$_SESSION['cloudinary']   = 0;
$_SESSION['host'] = ['protocol' => $host_protocol, 'sub' => $host_sub, 'domain' => $host_domain, 'tld' => $host_tld, 'port' => $host_port];
$_SESSION['host_tld'] = $host_tld;
$_SESSION['host_dom'] = $host_dom;
$_SESSION['host_sub'] = $host_sub;   

$_SESSION['system_cache'] = 0;
$_SESSION['redis_cache'] = 0;
$_SESSION['mongodb'] = 0;

# live or dev/test/staging/beta/local ?
$_SESSION['local'] = $local = $host_sub == 'local' || $host_tld == 'dev' ? 1 : 0;

require_once("constants.inc");
require_once("class_constants.inc");
require_once("helpers.inc"); /** needed for env_is_online */

if($local) {
  $test = $_SESSION['test'] = 1;
} else {
  $_SESSION['dev'] = $dev = $host_sub == 'dev' ? 1 : 0;
  $_SESSION['test'] = $test = $host_sub == 'test' || $host_sub == 'vpatina' ? 1 : 0; /** last is dirty hack for Indians... */
  $_SESSION['staging'] = $staging = $host_sub == 'staging' ? 1 : 0;
  $_SESSION['staging_live'] = $staging_live = $host_sub == 'live' ? 1 : 0;
  $_SESSION['beta'] = $beta = $host_sub == 'beta' ? 1 : 0;
}

$_SESSION['online'] = $online = $local ? env_is_online() : true; // if running locally, check if online, otherwise assume true
$_SESSION['offline'] = !$online; 
$_SESSION['aid'] = isset($_GET['_aid']) ? $_GET['_aid'] : $_SESSION['aid'];   

//if(in_array($host_sub, ['ms', 'ww'])) $_SESSION['test'] = $test = 1; // minisite

$live = $_SESSION['live'] = !($dev || $test || $beta || $staging|| $staging_live || $local) ? 1 : 0; // www or no sub

$_SESSION['deployment'] = $live ? 'live' : $host_sub;
if($local) $_SESSION['deployment'] = 'local'; 
$_SESSION['send_mail'] = ['live' => 1, 'test' => 0, 'dev' => 0, 'staging' => 0, 'beta' => 0, 'local' => 0]; // whether to send if not live
if(!$_SESSION['live']) $version = $_SESSION['version'] = date('mdHis');
$_SESSION['log_db'] = !$_SESSION['live'] && isset($_GET['log_db']) ? 1 : 0;
$__header = ['end_scripts' => [], 'scripts' => [], 'styles' => []];


require_once("vendor/autoload.php");                          
require_once("phpauth/languages/en_GB.php");
$auth_lang = $lang; // $lang is included in file above. Do not delete this line.
require_once("phpauth/Config.php");
require_once("phpauth/Auth.php");

/** read config file */
$_SESSION['config'] = $config = !$_SESSION['config'] || isset($_GET['reload-config']) ? config_load() : $_SESSION['config'];
if(!$config) die("Failed to read config file");

$_SESSION['log_level'] = $_SESSION['system_log_level'] = env_log_level($deployment);

/** db */
require_once("PEAR.php");
require_once "Mail/RFC822.php";
require_once("db.inc");
require_once("arrays.inc");
require_once("classes.inc");
require_once("routing.inc");
require_once("class_handlers.inc");
require_once("controllers.inc");
require_once("db_object.inc");

/** helpers */
require_once("helpers_file.inc");
require_once("helpers_html.inc");
require_once("helpers_time.inc");
require_once("helpers_media.inc");
require_once("helpers_mail.inc");
require_once("helpers_site.inc"); /** This needs to be after, as it looks for controllers from above */

require_once("services/google/google.inc");

/** application specific includes */
include_once("app_classes.inc");
include_once("app_class_handlers.inc");
include_once("app_class_constants.inc");
include_once("app_helpers.inc");
include_once("app_controllers.inc");
include_once("app_cronscripts.inc");
include_once("app_routing.inc");


/** which site */
define('SITE_URL', siteURL());
  
/** Set $no_app=1 to include this file without loading application */
/** Used in setup.php */

if($no_app) { 

} else {
  $uri = strip_query_string($_SERVER['REQUEST_URI']);
  $minisite = null;
  
  $app->name = $app_name = $config['name'];  
  if(!$app_name) die("Application name must be set in config/main.yml");
  $app_slug = $config['settings']['slug'] ?: slugify($app_name);
  $shortname = $lib_prefix = $app_slug;

  if(!$live) $app_slug .= "-dev";
  $data_dir = $data_path = $config['settings']['data_path'] ? rtrim($config['settings']['data_path'], '/')."/{$app_slug}/" : "/data/dir/{$app_slug}/";
    
  if($site_id = $_SESSION['site_id'] = $config['site_id']) { /** single tenant */
    //die("site_id=$site_id");
  } else { /** multi tenant */
    $site_id = $_SESSION['site_id'] = env_site_id($site_domain); /** inline switch */
  }
    
  /** this allows for vpatina.com/kerrygroneng or treat www.kerrygroneng.com as vpatina */
  if($msObj = env_minisite($site_domain, $uri)) {
    if($msObj->internal) $uri = substr($uri, strlen($minisite_basepath));
    $site_id = $msObj->site_id;
  }
  
  if(!$site_id) die("Don't know {$site_domain}");
  
  $siteObj = $__SITE = get_object('site', $site_id); /** uses Redis */

  /** site specific directories */
    
  $log_dir = $_SESSION['log_dir'] = $data_dir."log/";
  $upload_dir = $_SESSION['upload_dir'] = $data_dir."upload/";
  $cache_dir = $_SESSION['cache_dir'] = $data_dir."cache/";
  if($_SESSION['test']) $cache_dir = $_SESSION['cache_dir'] = str_replace('/data-dev/', '/data/', $cache_dir);
  $xml_dir = $data_dir."XML/";
  
  $null = is_dir($data_dir) ?: mkdir($data_dir, 0700, true);
  $null = is_dir($log_dir) ?: mkdir($log_dir, 0700, true);
  $null = is_dir($cache_dir) ?: mkdir($cache_dir, 0700, true);
  
  print_log("Set-env: uri={$uri} site_id={$site_id} domain: {$domain}", 'minisite', LOG_LEVEL_ALWAYS);
  

  
  /** if login */
  require_once("db_user.inc");
  
  /** todo: move to site config */
  $siteObj->modules = ['gallery','payment','rating','rental'];
  
  if(in_array('gallery', $siteObj->modules)) {
    require_once("gallery/helpers_gallery.inc");
  }
  
  if(in_array('golf', $siteObj->modules)) {
    require_once("golf/golfswitch.inc");
  }
  
  if(in_array('payment', $siteObj->modules)) {
    require_once("payment/db_invoice.inc");
    require_once("payment/db_payment.inc");
    require_once("payment/invoice_pdf_writer.inc");
  } 
  
  if(in_array('rating', $siteObj->modules)) {
    require_once("rating/db_rating.inc");
  }
  
  if(in_array('rental', $siteObj->modules)) {
    require_once("rental/db_rental.inc");
    require_once("rental/db_booking.inc");
    require_once("rental/db_reservation.inc"); /** get rid of */
    require_once("rental/db_apartment.inc"); /** get rid of */
    require_once("rental/db_location.inc");  
  }
  
  
  $_SESSION['client_id'] = $siteObj->client_id; 
  $_SESSION['legal_id'] = $siteObj->legal_entity_id; 
  $_SESSION['site_url'] = $site_url = SITE_URL; // will be http or https as needed
  $site_lang = $siteObj->default_language;
  $langs = $_SESSION['languages'] = json_decode($siteObj->languages) ?: [$siteObj->default_language];
  
  
  /** which page */
  
  
    
  if($msObj) {  
    $int = $msObj->internal;
    $minisite = $msObj->url;
    $user_id = $msObj->user_id;
    
    print_log("SetEnv1: Minisite: {$minisite} Int:{$int} Path={$uri}", 'minisite', LOG_LEVEL_TEST);
    if($int) $uri = str_replace("/{$user_id}/{$minisite}", '', $uri);
    print_log("SetEnv2: Minisite: {$minisite} Int:{$int} Path={$uri}", 'minisite', LOG_LEVEL_TEST);
    
    $pageObj = new stdClass;
    $pageObj->active = 1;
    $pageObj->path = $uri;
    $pageObj->minisite = $siteObj->minisite = $msObj->url;
    $pageObj->language = $lang = $_SESSION['language'] = $msObj->default_language;
    $pageObj->controller = 'minisite';
    
    
    $keys = [];
    foreach($keys as $key) {
      $pageObj->scope->$key = $msObj->$key;
    }
    $_SESSION['page_id'] = 0;
  
    $siteObj->admin_user_id = $msObj->admin_user_id;
    $app->minisite = $msObj;
    
    //if(isset($_GET['foo'])) die("Set-env: uri={$uri} site_id={$site_id} domain: {$site_domain} ms:".dump($msObj,true));
  
    //dump($pageObj);
    //dump($msObj);
  } else {  /** Routing */
    
    $response = route($app); /** New Alto router, Don't look up page_content any more */
    if($response) {
      $pageObj = new stdClass;
      list($controller, $action, $template) = explode('#', $response['target']);
      $name = $response['name'] ?? '';
      $pageObj->active = 1;
      $pageObj->path = $uri;
      $pageObj->controller = $controller;
      $pageObj->template = $template ?: str_replace('_','-', $controller);
      $app->params = $pageObj->params = $params = $response['params'] ?? [];
      $app->object_id = $pageObj->object_id = $params['id'] ?? 0;
      $app->subview = $pageObj->subview = $subview = $params['subview'] ?? '';
      $app->action = $pageObj->action = $action = $params['action'] ?? '';
      $app->tab = $pageObj->tab = $tabview = $params['tab'] ?? '';
  
      $app->routing = $response;
      
      $pageObj->name = trim("{$name} {$action} {$subview}");
      
      print_log("Alto Router: ".dump($response, true), 'app', LOG_LEVEL_TEST);
      print_log("Page: ".dump($pageObj, true), 'app', LOG_LEVEL_TEST);
        
    } else { /** Deprectated - old style to be phased out */
      list($path, $obj_id, $obj_name, $subview) = env_route($uri);
    
      //$api = strpos($path, '/api/') === 0 ? 1 : 0;
      print_log("No Route: Path={$path} api=$api id={$obj_id} name={$obj_name} subview=$subview", 'app', LOG_LEVEL_TEST);
  
      if($path) {
        if($content = find_object('page_content', ['site_id' => $site_id, 'path' => $path])) {
          $page = get_object('site_page', $content->page_id);  
          if(!$page) {
            $pageObj = page_error("No page found for {$path}");
          } else {
            $pageObj = site_merge_page_object($page, $content);
            $pageObj->subview = $subview;
            $subviews = $pageObj->subviews ? json_decode($pageObj->subviews) : [];
            if($subview && !in_array($subview, $subviews)) die("404: {$page->name}: unknown subview {$subview}"); // 404
      
            $lang = $_SESSION['language'] = $__LANG = $pageObj->language;
            $obj_type = $pageObj->object_type;
            $path_var = $pageObj->path_var;
            if($obj_id) {
              if($path_var) {
                $app->object_id = $app->object_slug = $obj_id;
                $app->object_type = $obj_type;
                $app->object_name = $obj_name;
              } else {
                $pageObj = page_error("{$path}: don't know {$obj_id}");
              }
            }
          }
          print_log("path:{$path}  page: {$page->name} merged:{$pageObj->name} ({$pageObj->id}): id={$obj_id} ctrl:{$pageObj->controller} tpl:{$pageObj->template}", 'app', LOG_LEVEL_TEST); 
        } else {
          $pageObj = page_error("{$path} does not exist");
        }  
        
        $base_path = rtrim($path, '/');
        $this_path = $base_path && $obj_id ? "{$base_path}/{$obj_id}" : $base_path;    
        $full_path = $subview ? "{$this_path}/{$subview}" : $this_path;
        
        $app->page_id = $_SESSION['page_id'] = $pageObj->id;
        
        $app->base_path = $base_path; /** path of this page w/o id or subview */
        $app->this_path = $this_path; /** path of this page with id, without subview */
        $app->full_path = $full_path; /** path of this page with id and subview */
        //dump($app);
      } else {
        $pageObj = page_error("Missing path");
      }
    }
  }
  
  /** auth - login */
  if($u_id = $_SESSION['u_id']) {
    $userObj = $__USER = get_user($u_id); 
    $u_level = $userObj->user_level;
    $u = obj2array($userObj);
  
  } else {   
    $authConfigObj = new PHPAuth\Config($db);
    $authConfigObj->overrideAll($siteObj);
    $phpauth = new PHPAuth\Auth($db, $authConfigObj, $auth_lang);
      
    $auth_hash = $phpauth->getSessionHash();
    $hash = pick_first($session_hash, $auth_hash, $cookie_hash);
    
    $_SESSION['auth_debug'] = false; // $_SESSION['live'] ? false : true;
  
    if($provider_name = $_GET["provider"]) {
      $user_id = user_provider_login($provider_name);
      print_log("user_provider_login for {$provider_name} = {$user_id}", 'provider', LOG_LEVEL_ALWAYS);
      $provider_login = $user_id ? true : false;
      if ($provider_login) {
        $result = $phpauth->third_party_login($user_id);
        $hash = $result['hash'];
        print_log("user_provider_login hash={$hash} result:".dump($result,true), 'provider', LOG_LEVEL_ALWAYS);
      }
    }
      
    
    if($hash) {   
      $_SESSION['hash'] = $hash;
      $domain = $siteObj->domain;
      $is_logged = $phpauth->isLogged();
      $u_id = $_SESSION['u_id'];
      $auth_u_id = $phpauth->getSessionUID($hash);
      $valid = $phpauth->checkSession($hash);
      $u_id = $auth_u_id ?: $u_id;
      if ($u_id && $valid) {
        if($_GET['act']=="logout") { // Check for logout
          $phpauth->logout($hash);
          $_SESSION['loggedin'] =  $_SESSION['hash'] = $u = $u_level = $u_id = $u_addid = $u_personid = $u_type = $u_email = $userObj = NULL;
          session_destroy();
        } else {
          $_SESSION['loggedin'] = 1;
          
          $userObj = get_user($u_id); /** database lookup */
              
          if(!$userObj) die("no user $u_id");
          $upload_dir = $data_dir."user/"; // for all users
          $null = is_dir($upload_dir) ?: mkdir($upload_dir, 0700, true);
          $u_hostid = $userObj->user_level == USER_LEVEL_OWNER || user_has_role($userObj, USER_ROLE_OWNER) ? $u_id : 0;
          $_SESSION['root_user'] =  $u_id == SYSTEM_USER_ID ? 1 : 0;  
          $u_level = $u['user_level'] = $_SESSION['user_level'] = $userObj->user_level;
          $u = obj2array($userObj);
          $_SESSION['u_id'] = $_SESSION['user_id'] = $u_id;
        }
      } else {
        $_SESSION['loggedin'] = $_SESSION['hash'] = $u = $u_level = $u_id = $u_addid = $u_personid = $u_type = $u_email = $userObj = NULL;
      }
    }
    
    if($provider_login && $_GET['redirect']) {
      redirect($_GET['redirect']);
    }
    
  }
   
  /** check if page is active */
  if($pageObj && !$pageObj->active) {
    if($u_level >= USER_LEVEL_EDITOR) {
      $pageObj->scope->warning = "This page not active and is only visislbe to editors";
    } else {
      $pageObj = page_error("{$page->name} not active {$site_id}", 403);
    }
    print_log("Page not active. id:{$pageObj->id} site:{$site_id} url: {$uri}", 'error', LOG_LEVEL_ERROR);
  }
  
  /** gets set to Europe/Amsterdam somewhere above... */
  // set System Timezone to UTC
  if(date_default_timezone_get() != SYSTEM_TIMEZONE) {
    date_default_timezone_set(SYSTEM_TIMEZONE); // UTC: Also set MySQL?
  }
  
  
    
  /** global scope, all pages */
  $global = new stdClass; /** data passed to app */
  /** end global scope */
  
  
  /** store site globals */
  $__GLOBAL = $global;
  $app->live = $__LIVE   = $_SESSION['live'];
  $app->test = $__TEST   = $_SESSION['test'];
  $app->dev = $__DEV    = $_SESSION['dev'];
  $app->client_id = $__CLIENT_ID = $_SESSION['client_id'];
  $app->app_root = $__APP_ROOT = $_SESSION['app_root'] = approot(true);
  $lib->lib_root = $__LIB_ROOT = $_SESSION['lib_root'] = libroot(true);
  $app->doc_root = $__DOC_ROOT = $_SESSION['doc_root'] = $doc_root;
  $app->log_dir = $log_dir;
  $app->site_url = $site_url;
  
  $app->user = $__USER   = $userObj;
  $app->site = $__SITE   = $siteObj;
  $app->page = $__PAGE   = $pageObj;
  
  $app->page_id = $__PAGE_ID = $pageObj->id;
  $app->site_id = $__SITE_ID = $siteObj->id;
  $app->user_id = $__USER_ID = $userObj->id;        
  $app->user_level = $__USER_LEVEL = $userObj ? $userObj->user_level : 0;        
  $app->local_tempplate = $__LOCAL_TEMPLATE = 0; // set to 1 if the smarty template is local to this site
  $app->template = $__TEMPLATE = $__TEMPLATE_PATH = '';
  $app->language = $__LANG = $__LANGUAGE = $_SESSION['language'];
  $app->languages = $__LANGS = $__LANGUAGES = $_SESSION['languages'];
  $app->deployment = $__DEPLOYMENT = $_SESSION['deployment'];
  $app->user_currency = $__USER_CURRENCY = $_SESSION['user_currency'] = 'EUR'; /** todo: make configurable by user */
  $app->user_levels = $__USER_LEVELS = [USER_LEVEL_USER => 'Member', USER_LEVEL_EDITOR => 'Editor', USER_LEVEL_STAFF => 'Staff'];
  $app->path_var = $__PATH_VAR = $_SESSION['path_var'];
  $app->default_bounds = $__DEFAULT_BOUNDS = '[["28.4371","-16.0021"],["74.6767","41.6541"]]';
  $app->remote_ip = $app->client_ip = client_ip();
  $app->locale = lang2locale($app->language) ?: "en_US.utf8";;
  $app->host_sub = $host_sub;
  $app->host_protocol = $host_protocol;
  $app->host_domain = $host_domain;
  $app->config = $_SESSION['config'];
  
  $app->db = $db_db;
  $app->db_conf = $db_conf;
  $app->db_key = $db_key;
  $app->dev_db = $dev_db;
  $app->live_db = !$dev_db;
  
  $app->config = $config;
  $app->cache = array2obj($config['cache']);
  
  $app->name = $app_name;
  $app->slug = $app_slug;
  $app->description = $config['description'];
  $app->data_path = $data_path;
  $app->config_file = $config['settings']['config_file'];
  
  //$app->site_id = $config->site_id;  
  
  $__APP = $app; /** global used sparingly */
  
  $gapi_key = "AIzaSyDqzndmgSgkwaD6AEATZOkeEWUo4foizEo";
  define("GMAP_API", "https://maps.googleapis.com/maps/api/js?key={$gapi_key}&libraries=places&language={$__LANG}&v=3.26");
  
  
  //$client_ip == '213.93.144.196') 
  if(isset($_GET['send_mail'])) {
    $send_mail = $_GET['send_mail'];
    if($send_mail) { 
      $_SESSION['send_mail']['test'] = 'test@kjakman.com'; 
      $_SESSION['send_mail']['dev'] = 'dev@kjakman.com';
    } else {
      $_SESSION['send_mail'] = 0;
    }
    print_log("set_env: setting send_mail to {$send_mail} session=".dump($_SESSION['send_mail'], true), 'mail', $log_level);
    
  }

  if(isset($_GET['debug_mail'])) { 
    $debug_mail = $_SESSION['debug_mail'] = $_GET['debug_mail'];
    print_log("set_env: setting debug_mail to {$debug_mail}", 'mail', $log_level);
  }

  
  $__EDIT_MENU = ''; 
  $__AUTH_EDIT = auth_site($siteObj); // allowed to edit using CMS
  if($__AUTH_EDIT) {
    if(isset($_GET['__edit_mode'])) $_SESSION['__edit_mode'] = $_GET['__edit_mode'];
    $url = this_url();  
    if($_SESSION['__edit_mode']) {
      $button = html_link(replace_query_var($url, '__edit_mode', 0), "<i class='fa fa-times-circle'></i>", array('title' => 'Exit Edit Mode', 'class' => 'btn btn-danger __edit_site-off __edit_on-off'));
    } else {
      $button = html_link(replace_query_var($url, '__edit_mode', 1), "<i class='fa fa-pencil-square-o'></i>", array('title' => 'Enter Edit Mode', 'class' => 'btn btn-success __edit_page-off'));
    }
    $__EDIT_MENU = html_div($button, '__edit_button');
  }
  
  $__EDIT_MODE = $__AUTH_EDIT && $_SESSION['__edit_mode'];
  
  $app->version = $version;
  $app->header = new stdClass;
  $app->header->base_scripts = [];
  $app->header->scripts = [];
  $app->header->styles = [];
  $app->header->meta = [];
  $app->header->link = [];
  $app->header->code = [];
  $app->header->onload = [];
  
  $time = elapsed($start_time);
  $mem = bytes2human(memory_get_usage());
  //print_log("Auth={$__AUTH_EDIT} edit={$__EDIT_MODE}", 'set_env', LOG_LEVEL_TEST); 
  print_log("Loaded env in {$time}s using {$mem}", 'app', LOG_LEVEL_TEST); 
  $app->elapsed = $time;
  
  //echo("<br><br><br>");dump($dbconf);
  //dump($app->config);
  
  //dump($_SESSION['config']);
  
  //die("Local={$local}");
  
  /** get rid of this and use $app->cache->xxx */
  $_SESSION['system_cache'] = $app->cache->system;
  $_SESSION['redis_cache'] = $app->cache->redis;
  $_SESSION['mongodb'] = $app->cache->mongo;
  
}

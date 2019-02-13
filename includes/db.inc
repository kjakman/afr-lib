<?php

$dbType = 'mysql';  ## Valid values: mysql, pgsql, oci8, odbc, sybase, fbsql, ibase, ifx, msql, mssql (These are pear supported dbs)
$db_key = $_SESSION['live'] || $_SESSION['test'] || $_SESSION['staging_live'] ? 'db' : 'devdb';
$db_conf = get_config($db_key, 0);

if(!$db_conf) echo("Failed to fetch database parameters");
$db_db = $db_conf->db;

$_SESSION['db_dev'] = $dev_db = $db_key == 'devdb' ? 1 : 0;

$db_user = $db_conf->user;
$db_host = $db_conf->host ?: "localhost";
$db_password = $db_conf->pass;
$db_dsn = "mysql://$db_user:$db_password@$db_host/$db_db";

// connect to db
try {
  $db = new PDO("mysql:host={$db_host};dbname={$db_db};charset=utf8", $db_user, $db_password);
  $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  $db->setAttribute(PDO::ATTR_EMULATE_PREPARES, false);
  $db->setAttribute("dbname", $db_db);

}
catch(PDOException $e) {     
  echo $e->getMessage();
}

if(!$db) {
  echo("PDO No db: host:$db_host name:$db_db user:$db_user p:$db_password".dump($db,true));     
}
$app->db = $db;
$mysqli = mysqli_connect($db_host, $db_user, $db_password, $db_db); // used for escaping strings only
  
//die("Gh:".dump($db_conf,true));

if($app->cache->redis) {
  $redis = new Redis();
  $redis->connect('127.0.0.1');
  $app->redis = $redis;
} else {
  $redis = null;
}  

if($app->cache->mongo) {
  $client = new MongoDB\Client("mongodb://localhost:27017");
  $app->mongodb = $mongodb = $client->$db_db;
}

//if($db_host != 'localhost') {
//  echo("host=$db_host".dump($db,true));die("gh");
//}


function db_escape($val) {
  global $mysqli;
  return mysqli_real_escape_string($mysqli, $val); 
}

function nextId($sequence) {
  $id = db_getOne("select nextid from db_sequence where seq_name='$sequence'");
  if(!$id) return 0;
  if($id > 0) {
    $id++;
  } else {
    $id--;
  }
  setNextId($sequence, $id);
  return $id;
}

function setNextId($sequence, $id) {
  db_query("update db_sequence set nextid=$id where seq_name='$sequence'");
  return $id;
}


// New Basic DB functions

// sync all tables listed in class definitions to slave
// syncs dev db by default - set live=true to sync live DB
// requires identical table structures

// sync from aptdb_v16 to v14 on local server
// set live to true for actual sync, otherwise dryrun (useful to see differences)
/** unsused - automatic comment added on  */
function sync_live2dev_schema($live=false) {
	global $db_dev;         // aptdb_v16
	global $db_live;        // aptdb_v14
  return sync_db_schema($db_live, $db_dev, $live=false);
}


// sync from aptdb_v16 to v14 on local server
// set live to true for actual sync, otherwise dryrun (useful to see differences)
/** unsused - automatic comment added on  */
function sync_dev2live_schema($live=false) {
	global $db_dev;         // aptdb_v16
	global $db_live;        // aptdb_v14
  return sync_db_schema($db_dev, $db_live, $live=false);
}


function sync_db_schema($from, $to, $live=false) {
	global $db_local_host;  // local
	global $db_user;
	global $db_password;

	$time_start = microtime_float();
  include 'db_schema_sync/class.dbsync.php';

  $dbsync = new DBSync();
  
  $dbsync->SetHomeDatabase($from, 'mysql', $db_local_host, $db_user, $db_password);
  $dbsync->AddSyncDatabase($to, 'mysql', $db_local_host, $db_user, $db_password);

  $ok = true;
  if ($dbsync->Sync($live)) {
		$time_end = microtime_float();
		$diff = $time_end - $time_start;
		echo("<b>Done Synchronizing database schema in $diff seconds</b><br>");
  } else {
    echo('Something went wrong with synchronising...');
  }
}


// sync from aptdb_v16 on tango to v16 on diz (live_db = false, default)
// sync from aptdb_v14 on tango to v14 on diz (live_db = true)
// syncs schema with slave (alter table, drop table, add table, etc)
/** unsused - automatic comment added on  */
function sync_slave_schema($live=false, $live_db=false) {
	global $db_local_host;  // local
	global $db_master_host; // tango
	global $db_slave_host;  // diz
	global $db_user;
	global $db_password;
	global $db_dev;         // aptdb_v16
	global $db_live;        // aptdb_v14
	$db_db = $live_db ? $db_live : $db_dev;

	$time_start = microtime_float();
  include 'db_schema_sync/class.dbsync.php';

  $dbsync = new DBSync($live);
  
  $dbsync->SetHomeDatabase($db_db, 'mysql', $db_master_host, $db_user, $db_password);
  $dbsync->AddSyncDatabase($db_db, 'mysql', $db_slave_host, $db_user, $db_password);

  $ok = true;
  if ($dbsync->Sync()) {
		$time_end = microtime_float();
		$diff = $time_end - $time_start;
		echo("<b>Done Synchronizing database schema in $diff seconds</b><br>");
  } else {
    echo('Something went wrong with synchronising...');
  }
}

// not real sync - only adds/deletes rows, no updates
// Solution? Add Timestamp each class and use: http://www.phpclasses.org/browse/package/1803.html
// Send only new/updated rows. 

/** unsused - automatic comment added on  */
function sync_slave($live=false) {
	include "db_sync.php"; 

	global $db_master_host;
	global $db_slave_host;
	global $db_user;
	global $db_password;
	global $db_dev;
	global $db_live;
	$db_db = $live ? $db_live : $db_dev;

	$time_start = microtime_float();
	$sync = new SyncronizeDB(); 
		
	$n=0;
	$classes = get_classes();

	// add db sequence
	$db_sequence->key 	= 'seq_name';
	$db_sequence->table = 'db_sequence';
  $classes[] = $db_sequence;

	$test->key 	= 'id';
	$test->table = 'agent';
	$classes = array($test); 

	foreach($classes as $class) {
		$table = $class->table;
		$key= $class->key;

		echo("<h3>Syncing table=$table (key=$key)</h3>");		
		//masterSet(dbserver,user,password,db,table,index) 
		$sync->masterSet($db_master_host,$db_user,$db_password,$db_db,$table,$key); 
		$sync->slaveSet($db_slave_host,$db_user,$db_password,$db_db,$table,$key); 
		
		//syncronizing the slave table with the master table (at row level) 
		$sync->slaveSyncronization();
		$n++;
	}
	$time_end = microtime_float();
	$diff = $time_end - $time_start;
	
	echo("<b>Done Synchronizing $n tables in $diff seconds</b><br>");
}

/** unsused - automatic comment added on  */
function get_id($table, $field, $val) {
   global $db;
   return db_getOne("SELECT id from $table WHERE $field='$val'");
}

function get_parent_id($table, $id) {
   global $db;
   if(!$table || $id=='') {
     print_error("get_parent_id: table=$table id=$id<br>");
     return NULL;
   }
   return db_getOne("SELECT parent_id from $table WHERE id=$id");
}

function get_name($table, $id) {
   global $db;
   $qs = "SELECT name from $table WHERE id=$id";
   return db_getOne($qs);
}

/** unsused - automatic comment added on  */
function get_field($table, $field, $id) {
   global $db;
   $qs = "SELECT $field from $table WHERE id=$id";
   if(!$id || !$table || !$field) {return "";print_debug("get_field: missing parameters; $qs");}
   //echo("get_field query: $qs");
   return db_getOne($qs);
}

function db_log_query($query_str, $options=array()) {
  return false;
  $affected= isset($options['affected']) ? $options['affected'] : false;
  $error = $options['error'];
  $time = $options['time'];
  $this_log_level = $error ? LOG_LEVEL_ERROR : LOG_LEVEL_DEBUG; 
      
  $stack = caller_stack(array('array' => true));
  array_shift($stack); // remove first (this function)
  $function = array_shift($stack);
  $stack_str = count($stack) ? implode(" < ", $stack) : '';
  $actions = explode(" ", $query_str);
  $action = array_shift($actions); 
  $action = strtoupper($action);

  $log_level = $_SESSION['log_level'];
  //echo("ll=$log_level this=$this_log_level<br>");
  if($log_level >= $this_log_level) {
    $query_str = str_replace(array("`"), "", $query_str); // strip new lines
    $query_str = preg_replace('!\s+!', ' ', $query_str);  // strip new lines, tabs, multiple spaces
    $select = false;
    switch($action) {
    case "SELECT":
      $select = true;
      $regexp = "/select\s+(.*)\s+from\s+([a-zA-Z_]+)\s*(where)?\s*(.*)?/i";
      $what_i = 1; // 1-indexed, 0 = false
      $table_i = 2;
      $where_i = 4;
      
      break;
    case "UPDATE":
      $regexp = "/update\s+([a-zA-Z_]+)\s+set\s+(.*)\s*(where)?\s*(.*)?/i";
      $table_i = 1;
      $what_i = 2;
      $where_i = 4;
      break;
    case "DELETE":
      $regexp = "/delete\s+from\s+([a-zA-Z_]+)\s*(where)?\s*(.*)?/i";
      $table_i = 1;
      $what_i = 0;
      $where_i = 3;
      break;
    case "INSERT":
      $regexp = "/insert\s+into\s+([a-zA-Z_]+)\s+(.*)/i";
      $table_i = 1;
      $what_i = 2;
      $where_i = 0;
      break;
    default:
      $table = "";    
      break;
    }
    // log
    if ($regexp && preg_match ($regexp, $query_str, $res)) {
      $table = $table_i ? $res[$table_i] : '';
      $what = $what_i ? $res[$what_i] : '';
      $where = $where_i ? $res[$where_i] : '';
      $match = true; 
     //if($action == "SELECT") {
     //  echo($query_str.dump($res, true));
     //}
    } else {
      $match = false;
      $table = '??';
    }
    
    $page = $_SESSION['page_type'] ? $_SESSION['page_type'].'-'.$_SESSION['page_id'] : $_SERVER['REQUEST_URI'];
    $site = $_SESSION['site_id'] ?: $_SERVER['HTTP_HOST'];
    $context_str = "Site: $site Page: $page Function: $function".($stack_str ? " Stack: $stack_str" : "");

    $log_str = ($time ? "[".$time."s] " : "").($error ? "*** ERROR ***" : "")."$action [$table] ".($affected !== false ? "AFFECTED=[$affected] " : "");
    $log_str .= " $context_str";
    if($error) $log_str .= "\n $error: $query_str"; 
                                                                                                                                                                                   
    if($log_level >= LOG_LEVEL_DUMP) {
      $log_str .= ($what ? "\nWHAT=[$what]" : "").($where ? "\nWHERE [$where]" : "");
    }
    print_log("$log_str\n", 'db', $this_log_level);
    if(!$match) print_log("NO MATCH: $regexp\nQUERY=$query_str\n", 'db', LOG_LEVEL_WARNING);
    //print_log("$query_str\n$log_str\n\n", 'db', LOG_DUMP);
  }
  
}

function db_getOne($query_str, $values = array(), $options=array()) {
  global $db;
  if($_SESSION['log_db']) $time_start = microtime(true);
  
  try {
    $stmt = $db->prepare($query_str);    
    $stmt->execute($values);    
    $result = $stmt->fetchColumn(0);
  } catch(PDOException $ex) {
    pdo_exception($ex, $query_str, $values);
  }
    
  if($options['debug'] || $options['debug_query'] || $_SESSION['debug_db']) echo("getOne: $query_str result=$result<br>");

  if($_SESSION['log_db']) {
    $time_end = microtime(true);
    $total_time = round(($time_end - $time_start), 4);
    db_log_query($query_str, ['time' => $total_time]); // logging
  }
  
  return $result;
}

function db_getCol($query_str, $values = array(), $options=array()) {
  global $db;
  if($_SESSION['log_db']) $time_start = microtime(true);

  try {
    $stmt = $db->prepare($query_str);    
    $stmt->execute($values);    
    $result = $stmt->fetchAll(PDO::FETCH_COLUMN, 0);
  } catch(PDOException $ex) {
    pdo_exception($ex, $query_str, $values);
  }

  if($options['debug'] || $options['debug_query'] || $_SESSION['debug_db']) echo("getCol: $query_str result=".dump($result, true));
  
  if($_SESSION['log_db']) {    
    $time_end = microtime(true);
    $total_time = round(($time_end - $time_start), 4);
    db_log_query($query_str, ['time' => $total_time]); // logging
  }
  
  return $result;
}

function db_getRow($query_str, $values = array(), $options=array()) {
  global $db;
  if($_SESSION['log_db']) $time_start = microtime(true);

  try {
    $stmt = $db->prepare($query_str);    
    $stmt->execute($values);    
    $fetchmode = $options['fetchmode'] ? db_fetchmode($options['fetchmode']) : PDO::FETCH_OBJ;
    $row = $stmt->fetchAll($fetchmode);
    $result = $row[0];
  } catch(PDOException $ex) {
    pdo_exception($ex, $query_str, $values);
  }

  if($options['debug'] || $options['debug_query'] || $_SESSION['debug_db']) echo("getRow: $query_str result=".dump($result, true));
  
  if($_SESSION['log_db']) {    
    $time_end = microtime(true);
    $total_time = round(($time_end - $time_start), 4);
    db_log_query($query_str, ['time' => $total_time]); // logging
  }
  return $result;
}

function db_getAll($query_str, $values = array(), $options=array()) {
  global $db;
  //dump($query_str);
  $logOptions = array();
  if($_SESSION['log_db']) $time_start = microtime(true);

  try {
    $stmt = $db->prepare($query_str);    
    $stmt->execute($values);
    $fetchmode = $options['fetchmode'] ? db_fetchmode($options['fetchmode']) : PDO::FETCH_OBJ;
    $result = $stmt->fetchAll($fetchmode);
  } catch(PDOException $ex) {
    pdo_exception($ex, $query_str, $values);
  }
    
  if($options['debug'] || $options['debug_query'] || $_SESSION['debug_db']) echo("getAll: $query_str result=".dump($result, true));

  if($_SESSION['log_db']) {    
    $time_end = microtime(true);
    $total_time = round(($time_end - $time_start), 4);
    db_log_query($query_str, ['time' => $total_time]); // logging
  }
  
  return $result;
}

function db_query($query_str, $values=array(), $options=NULL) {
  global $db;
  if($_SESSION['readonly']) return false;
  
  $result = new stdClass;
  $error = '';
  $affected = 0;
  
  $action = strtoupper(array_shift(explode(" ", $query_str)));

  if($_SESSION['log_db']) {    
    $time_start = microtime(true);
    $logOptions = array();
  }
  
  try {
    $stmt = $db->prepare($query_str);    
    $result->affected = $stmt->execute($values);    
  } catch(PDOException $ex) {
    pdo_exception($ex, $query_str, $values);
    $result->error = $error = $ex->getMessage(); 
  }

  if($options['debug'] || $options['debug_query'] || $_SESSION['debug_db']) echo("db_query: $query_str result=".dump($result, true));

  if($_SESSION['log_db']) {    
    $time_end = microtime(true);
    $total_time = round(($time_end - $time_start), 4);  
    db_log_query($query_str, ['time' => $total_time, 'error' => $error, 'affected' => $affected]); // logging
  }
  if(!$error) $result->success = 1;

  return $result;
}


// hardly used, but a couple of places in db_object
function db_fetchmode($mode) {
  switch($mode) {
    case 'ordered':
      return PDO::FETCH_NUM;
    case 'assoc':
      return PDO::FETCH_ASSOC;
    case 'object':
      return PDO::FETCH_OBJ;
    default: 
      return null;
  }
}

// General to all tables

/** no longer in use */
function check_db_result($rows) {
  return;   
}


function pdo_exception($ex, $query_str = '', $values = array()) {
  $message = $ex->getMessage();
  $error = "pdo_exception: $message<br>";
  if($query_str) $error .= $query_str;
  if(count($values)) $error .= "Values:".dump($values, true);
  print_log("pdo_exception: $error", 'db', LOG_LEVEL_ERROR);
  if(!$_SESSION['live']) mydie($error);
}

/** no longer in use */
/** unsused - automatic comment added on  */
function db_error($query_str, $result) {
  return; 
}



function exists($table, $field, $target, $key=0, $id=0) {
   global $db;
   $query="SELECT $field FROM $table WHERE $field='$target'";
   if($key && $id) $query.=" AND $key <> '$id'";
   return(db_getOne($query));
}

// End New Basic DB functions


function get_name_substr($name, $length=3) {
  $first=substr($name, 0, $length);
  if($pos=strpos($name, "-")) {
    $last=substr($name, $pos+1, $length);
    return $first."-".$last;
  } else {
    return $first;
  }
}

/** unsused - automatic comment added on  */
function get_tx_status_id($st) {
  global $db;
  return db_getOne("select id from tx_status where name='$st'");
}

/** unsused - automatic comment added on  */
function get_tx_status_name($stid, $length=0) {
  global $db;
  if($stid=="") $stid=0;
  $name = db_getOne("select name from tx_status where id=$stid");
  return ($length ? get_name_substr($name, $length) : $name);
}


/** unsused - automatic comment added on  */
function get_res_status_id($st) {
  global $db;
  return db_getOne("select id from res_status where name='$st'");
}

/** unsused - automatic comment added on  */
function get_res_status_name($stid, $length=0) {
  global $db;
  if($stid=="") $stid=0;
  $name = db_getOne("select name from res_status where id=$stid");
  return ($length ? get_name_substr($name, $length) : $name);
}

/** unsused - automatic comment added on  */
function get_res_type_id($st) {
  global $db;
  return db_getOne("select id from res_type where name='$st'");
}

/** unsused - automatic comment added on  */
function get_res_type_name($stid, $length=0) {
  global $db;
  if($stid=="") $stid=0;
  $name = db_getOne("select res_type from res_type where res_type_id=$stid");
  return ($length ? get_name_substr($name, $length) : $name);
}

/** unsused - automatic comment added on  */
function get_res_statuses() {
  global $db;
  return db_getAll("select * from res_status ORDER by id");
}

/** unsused - automatic comment added on  */
function get_inv_statuses() {
  global $db;
  return db_getAll("select * from inv_status ORDER by id");
}

/** unsused - automatic comment added on  */
function get_inv_type_id($st) {
  global $db;
  return db_getOne("select id from inv_type where name='$st'");
}

/** unsused - automatic comment added on  */
function get_inv_type_name($stid, $length=0) {
  global $db;
  $name = db_getOne("select name from inv_type where id=$stid");
  return ($length ? get_name_substr($name, $length) : $name);
}

/** unsused - automatic comment added on  */
function get_payment_type_id($st) {
  global $db;
  return db_getOne("select id from payment_type where name='$st'");
}

/** unsused - automatic comment added on  */
function get_payment_type_name($stid, $length=0) {
  global $db;
  $name = db_getOne("select name  from payment_type where id=$stid");
  return ($length ? get_name_substr($name, $length) : $name);
}

/** unsused - automatic comment added on  */
function get_lineitem_type_id($name) {
  global $db;
  return db_getOne("select id from lineitem_type where name='$name'");
}

function get_lineitem_type_name($id, $length=0) {
  global $db;
  $name = db_getOne("select name from lineitem_type where id=$id");
  return ($length ? get_name_substr($name, $length) : $name);
}


/** unsused - automatic comment added on  */
function get_inv_status_id($st) {
  global $db;
  return db_getOne("select id from inv_status where name='$st'");
}

/** unsused - automatic comment added on  */
function get_inv_status_name($stid, $length=0) {
  global $db;
  if($stid=="") $stid=0;
  $name = db_getOne("select name from inv_status where id=$stid");
  return ($length ? get_name_substr($name, $length) : $name);
}

function get_country_name($id) {
  global $db;
  return db_getOne("select name from countries where id='$id'");
}

/** unsused - automatic comment added on  */
function get_country($id) {
  global $db;
  return db_getRow("select * from country where id='$id'");
}

function find_country($level, $id) {
  global $db;
  if(!$id) {
    print_debug("find_country level=$level no id given");
    return 0;
  }
  switch($level) {
    case 'location':
      $id = get_parent_id('location', $id);
      $pid = find_country('district', $id);
      break;
    case 'district':
      $id = get_parent_id('district', $id);
      $pid = find_country('city', $id);
      break;
    case 'city':
      $id = get_parent_id('city', $id);
      $pid = find_country('region', $id);
      break;
    case 'region':
      $id = get_parent_id('region', $id);
      $pid = $id;
      break;
    default:
      die("find_country: unknown level $level");
      break;
  }
    return $pid;
}





/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_quote($quoteObj) {
   $can_publish = $quoteObj->can_publish ? 1 : 0;
   $published = $quoteObj->published ? 1 : 0;
   $query_str = "INSERT INTO quote
                        (created, name, title, company, email,
                         city, language,
                         can_publish, published, quote)
                 VALUES ('$quoteObj->created', '$quoteObj->name', '$quoteObj->title', '$quoteObj->company', '$quoteObj->email',
                         '$quoteObj->city', '$quoteObj->language',
                         '$can_publish', '$published', '$quoteObj->quote')";
   //echo($query_str);
   db_query($query_str);
}




/** unsused - automatic comment added on  */
function print_inline_vars($obj, $vars, $dbprefix="") {
  if(!$obj) return "";
  $result_array = array();
  foreach($vars as $var) {
    $dbvar = $dbprefix.$var;
    $val = $obj->$var;
    //echo("looking for $var - val=$val<br>");

    if($val = $obj->$dbvar) {

      $word = phrase($var, CAPITALIZE) ? phrase($var, CAPITALIZE) : $var;
      $result_array[] = strtolower($val)=='y' ? $word : "$word ($val)";
    }
  }
  return implode(", ", $result_array);
}

/** unsused - automatic comment added on  */
function reduce_string($str) {
  $str2 = strtolower(str_replace(array(' ', '/', "'", '"', '+', '%', '*', "\\"), "", $str));
  //print_debug("str was $str now $str2");
  return $str2;
}

/** unsused - automatic comment added on  */
function get_languages() {
  return get_langs();
}


function get_langs($where="") {
   global $db;
   $filters = array('active=1');
   if($where) $filters[] = $where;
   $filters = implode(" AND ", $filters);
   if($filters) $filters = " WHERE ".$filters;
   $key = $_SESSION['livedb'] ? 'id' : 'code';
   return db_getAll("SELECT * from language $filters ORDER BY $key");
}

/** unsused - automatic comment added on  */
function get_images($apt_id, $table="media") {
  global $db;
  return db_getAll("SELECT * FROM $table WHERE parent_id='$apt_id'");
}

// takes a name or comma separated list of names if $list=1
// returns the corresponding id or comma-separated list of ids from given table.
/** unsused - automatic comment added on  */
function db_name2id($table, $name, $list=false, $type="", $name_fld='name', $key_fld='id') {
  global $db;
  global $dbmap;
  if(!$name) return 0;

  if($list) {
    $orgname=$name;
    // replace ' en ' with ','
    $name = str_replace(array(" met ", " en ", " ("), ",", $name);
    $name = str_replace(array(")"), "", $name);
    $id_list = array();
    $name_list = explode(',', $name);
    foreach($name_list as $thisname) {
      $thisname = dbtrim(strtolower($thisname));
      $qs ="SELECT $key_fld FROM $table WHERE $name_fld='$thisname'";
      if($type) $qs .= " AND $type=1";
      $id=db_getOne($qs);
      insert_or_update('funda_keys', $table, $thisname, $id);

      if($id || $id===0) {
        $id_list[] = $id;
      } else {
        $dbmap[$table][$thisname]++; // value not found, store in dbmap
      }
    }
    $id_list = array_unique($id_list);
    sort($id_list);
    $result = implode(",", $id_list);
    if($result[0]==',') {
      echo("$table: List was '$orgname' => '$name' Result: '$result';<br>");
      $result = substr($result, 1); // remove first comma
    }
    //if(sizeof($name_list) != sizeof($id_list)) {
    //  print_debug("<b>$table: #Name_list <> #id_list: $name - $result</b>");
    //  echo("<b>$table: #Name_list <> #id_list: $name - $result</b>");
    //}

  } else {
    $qs = "SELECT $key_fld FROM $table WHERE $name_fld='$name'";
    $name = dbtrim(strtolower($name));
    $id=db_getOne($qs);
    //echo("$qs<br>");

    insert_or_update('funda_keys', $table, $name, $id);

    if($id) {
      $result = $id;
    } else {
      $result = $name;
      $dbmap[$table][$name]=1; // value not found, store in dbmap
      //echo("<b>#no id found for $table: $name - $result</b>");
    }
    //echo("$table: Entry was '$name' Result: $result;<br>");
  }
  return $result;
}
// takes an id or comma separated list of ids if $list=1
// returns the corresponding name or comma-separated list of names from given table, translated using phrase() if translate=true.
// if split is set to true, each term will be split into separate terms (separated by space) before looking up in dictionary

// if a row is found in $table with name=$key and value=$val, then increase count, otherwise insert
function insert_or_update($table, $key, $val, $id) {
  global $db;
  if($row = db_getRow("SELECT * FROM $table WHERE name='$key' AND value='$val'")){
    //echo("<b>Updating $key / $val / $id </b><br>");
    db_query("UPDATE $table SET count = count+1, mapkey='$id' WHERE id=".$row->id);
  } else {
    //echo("<b>Inserting $key / $val / $id </b><br>");
    $nid = nextId($table);
    db_query("INSERT INTO $table (id, count, name, value, mapkey) VALUES ($nid, 1, '$key', '$val', '$id')");
  }       
}


function db_id2name($table, $id, $translate=false, $list=false, $split=false, $name_fld='name', $key_fld='id') {
  global $db;
  //echo("db_id2name: t=$table, id=$id<br>");
  if(!$id) return '';
  if($list) {
    $name_list = array();
    $id_list = explode(',', $id);
    //asort($id_list);
    $case = CAPITALIZE;
    foreach($id_list as $thisid) {
      $qs ="SELECT $name_fld FROM $table WHERE $key_fld='$thisid'";
      //echo("qs in db_id2name = $qs<br>");
      if($name=db_getOne($qs)){
        if($split) {
          $split_names = explode(' ', $name);
          $split_terms = $translate ? phrases($split_names, LOWERCASE) : $split_names;
          $name_list[] = ucfirst(implode(" ", $split_terms));
        } else {
          $name_list[] = $translate ? phrase($name, $case) : $name;
        }
      }
      $case = LOWERCASE;
      //echo("$qs<br>");
    }
    $result = implode(", ", $name_list);
    if(sizeof($name_list) != sizeof($id_list)) {
      print_debug("<b>$table: #Name_list <> #id_list: $id - $result</b>");
    }

  } else {
    $qs = "SELECT $name_fld FROM $table WHERE $key_fld='$id'";
    //echo("qs in db_id2name = $qs<br>");
    $name =db_getOne($qs);
    if($split) {
      $split_names = explode(' ', $name);
      $split_terms = $translate ? phrases($split_names, LOWERCASE) : $split_names;
      $result = ucfirst(implode(" ", $split_terms));
    } else {
      $result = $translate ? phrase($name, CAPITALIZE) : $name;
    }
    if(!$result) {
      print_debug("<b>#no name found for $table: $id - $result</b>");
    }
  }
  return $result;
}



/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_chat($from, $to, $message, $lang='en', $type='', $type_id=0) {
  global $db;
  $id = nextId('chat');
  $type_array = array('short'=>'res_id', 'long'=>'long_enq_id', 'sale'=>'sale_enq_id');
  $type_field=$type_array[$type];
  //echo("type=$type, type_id=$type_id, type_field=$type_field<br>");

  //$message = str_replace("\n\n", "<p>", $message);
  //$message = str_replace("\n", "<br>", $message);
  //$message = str_replace("\'", "'", $message);
  $message = addslashes($message);
  if($type_field && $type_id) {
    $qs = "INSERT INTO chat (id, sender_id, receiver_id, message, language, $type_field) VALUES ($id, '$from', '$to', '$message', '$lang', '$type_id')";
  } else {
    $qs = "INSERT INTO chat (id, sender_id, receiver_id, message, language) VALUES ($id, '$from', '$to', '$message', '$lang')";
  }
  //echo("add_chat qs<br>$qs;<br>");
  db_query($qs);
}

$chat_type_array = array('short'=>'res_id', 'long'=>'long_enq_id', 'sale'=>'sale_enq_id');
/** unsused - automatic comment added on  */
function update_chat($chat_id, $from, $to, $message, $lang='en', $type='', $type_id=0) {
  global $db;
  global $chat_type_array;
  $type_field=$type_array[$type];

  //$message = str_replace("\n\n", "<p>", $message);
  //$message = str_replace("\n", "<br>", $message);
  //$message = str_replace("\'", "'", $message);

  $message = addslashes($message);

  //echo("type=$type, type_id=$type_id, type_field=$type_field<br>");
  if(!$chat_id) die("update chat - id missing");
  if($type_field && $type_id) {
    $qs = "UPDATE chat SET sender_id='$from', receiver_id='$to', message='$message', language='$lang', $type_field=$type_id WHERE id=$chat_id";
  } else {
    $qs = "UPDATE chat SET sender_id='$from', receiver_id='$to', message='$message', language='$lang' WHERE id=$chat_id";
  }
  //echo("add_chat qs<br>$qs;<br>");
  db_query($qs);
}

/** unsused - automatic comment added on  */
function get_conversation($type, $type_id) {
  global $chat_type_array;
  global $db;
  $type_field=$chat_type_array[$type];
  if(!$type_field || !$type_id) die("get_conversation: missing type or type_id");
  $qs = "SELECT * FROM chat WHERE $type_field=$type_id";
  if($chatObjs = db_getAll($qs)) {
    foreach($chatObjs as $chatObj) {
      $senderObj = get_object('user', $chatObj->sender_id);      
      $sender_name = $chatObj->sender_id ? get_name_string($senderObj)." (".get_user_role($chatObj->sender_id).") " : "Guest";
      $message = stripslashes($chatObj->message);
      $conversation .= "<tr><td>[$chatObj->timestamp] &nbsp;</td><td><b>$sender_name:</b> &nbsp;</td><td>$message</td></tr>";
    }
    return "<table border='0' cellspacing='0' cellpadding>$conversation</table>";
  } else {
    return "";
  }
}

// returns # of currency per BTC
function get_btc_rate($fromCur) {
  // use BitPay since this is our gateway
  $url = "https://bitpay.com/api/rates";  
  $json = file_get_contents($url) ?: "[]";
  $data = json_decode($json, TRUE);
  foreach($data as $rateAr) {  
    if($rateAr['code'] == $fromCur) {
      $cur_per_btc = $rateAr['rate'];
      break;
    }
  }
  if(!$cur_per_btc) return 0; // error
  
  //$factor = $_SESSION['dev'] ? 100 : 1;
  $factor = 1;
  return $cur_per_btc * $factor  ?: 0;
  
  // old code
  $exchange = 'MTGOX';
  $converter = 'xe'; // or google
  $service = "http://btcrate.com/convert?from=$fromCur&to=$toCur&exch=$exchange&conv=$converter&amount=$amount";
  $result_json = get_html($service);
  $result = json_decode($result_json);
  //echo($service);
  //dump($result);
  if($fromCur == 'BTC') {
    $decimals = 2;
    if($dev) $result->converted *= $factor;
  } elseif($toCur == 'BTC') {
    $decimals = 8;
    if($dev) $result->converted /= $factor;
  } else {
    mydie("get_btc_rate: neither from nor to is BTC");
  }

  return $result->converted ? round($result->converted, $decimals) : 0;  
}

function get_exchange_rate($fromCur, $toCur, $amount=0, $skim=false) {
  if($fromCur == $toCur) return 1;
  if($fromCur == "BTC" || $toCur == "BTC") {
    $otherCur = $fromCur == "BTC" ? $toCur : $fromCur;  
    $cur_per_btc = get_btc_rate($otherCur);
    return $toCur == "BTC" ? $cur_per_btc : 1 / $cur_per_btc;
  }
  
  $data = file_get_contents("http://api.fixer.io/latest?base={$fromCur}&symbols={$toCur}");
  $json = json_decode($data);
  if(is_object($json) && $json->rates && $json->rates->$toCur) {    
    $live_rate = (float) $json->rates->$toCur;
    $now = now();
    db_query("UPDATE exchange_rate SET last_updated='$now', real_rate='$live_rate', our_rate='$live_rate' WHERE from_currency='$fromCur' AND to_currency='$toCur'");
    return $amount ? number_format($live_rate*$amount, 2, '.', '') : $live_rate;  
  }
  
  $field = $skim ? 'our_rate' : 'real_rate';
  $qs = "SELECT $field FROM exchange_rate WHERE from_currency='$fromCur' AND to_currency='$toCur'";
  $rate = db_getOne($qs);
  
  if(!$rate) return 0;

  return $amount ? number_format($rate*$amount, 2, '.', '') : $rate;
}

/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function agent_xml_login($login, $password) {
  $error = '';
  $secure = $_SERVER['HTTPS'] == 'on';
  //if(!$_SESSION['dev'] && !$secure) die("Secure connection required for live transactions. Please use https"); //return array(NULL, "Secure connection required for live transactions. Please use https://".$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF']); 
  if(!$login) return array(NULL, "Login required."); 
  if(!$password) return array(NULL, "Password required."); 
  $userObj = check_user($login, $password);
  if(!$userObj) {
    return array(NULL, "Authentication failed."); 
  } else {
    if($userObj->user_level != USER_LEVEL_AGENT) return array(NULL, "Only authorized agents may use this service."); 
    if(!$userObj->active) return array(NULL, "Agent not active."); 
  }
  $user_type = get_object('user_level', $userObj->user_level, 'user_type');
  $agentObj = find_object(strtolower($user_type), array('agent.parent_id' => $userObj->id));

  $_SESSION['loggedin']   = obj2array($userObj);
  $_SESSION['user_id']    = $userObj->id;
  $_SESSION['agent_id']   = $agentObj->id;
  $_SESSION['user_level'] = $userObj->user_level;
  $_SESSION['user_type']  = $user_type;
  $_SESSION['user_name']  = $agentObj->agent_name." [via XML]";

  return array($agentObj, '');  
}

// rank functions used to maintain ranking of rows in a table

// takes an ID list (comma separated), sets rank in that order
function rank_update_list($obj_type, $list, $rank_field='rank') {
  $table = get_object_table($obj_type);
  $query = "set @row_num=0";
  db_query($query);
  $query = "update $table set $rank_field = (SELECT @row_num := @row_num + 1)  WHERE id in ($list) order by FIELD(id,$list)";
  print_log($query, 'rank', LOG_LEVEL_DEBUG);
  db_query($query);
  
  // hacky side effect - also update parent object
  $ids = explode(',', $list);
  if($obj_type == 'media' && count($ids) > 1) {
    $id = $ids[0];
    if($mediaObj = get_object('media', $id)) {
      $pid = $mediaObj->parent_id;
      $pt = $mediaObj->parent_type;
      $pf = $mediaObj->parent_field;
      if($pid && $pt && $pf) {
        update_object($pt, array($pf => $list), $pid);
        $parentObj = get_object($pt, $pid);
        if(isset($parentObj->media_json)) {
          if($mediaJson = media2json($pt, $parentObj)) update_object($pt, ['media_json' => $mediaJson], $pid);
        }
        
      }
    }
  }
  
}


// selects all rows matching search, orders by rank, updates rank or each row from 1 - NUM_ROWS
function rank_update($obj_type, $search='', $rank_field='rank') {
  $table = get_object_table($obj_type);
  $query = "set @row_num=0";
  $where_str = is_array($search) ? search2where($search) : $search; 
  db_query($query);
  $query = "update $table set $rank_field = (SELECT @row_num := @row_num + 1)".($search ? " WHERE $where_str" : '')." order by $rank_field";
  //$query = "update $table set $rank_field = (SELECT @row_num := @row_num + 1)".($search ? " WHERE $where_str" : '');
  print_log($query, 'rank', LOG_LEVEL_DEBUG);
  db_query($query);
}

// rank functions used to maintain ranking of rows in a table
function rank_next($obj_type, $search='', $rank_field='rank') {
  global $db;
  $table = get_object_table($obj_type);
  $query = "SELECT max(abs($rank_field)) FROM $table".($search ? " WHERE ".search2where($search) : '');
  $query = "SELECT count(*) FROM $table".($search ? " WHERE ".search2where($search) : '');
  //echo($query.'<br>');
  $max = db_getOne($query) ?: 0;
  return  $max + 1;
}

function rank_set($obj_type, $id, $rank, $rank_field='rank') {
  global $db;
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $query = "UPDATE $table SET $rank_field=$rank WHERE $key='$id'";
  return db_getOne($query);
}
  
function rank_get($obj_type, $id, $rank_field='rank') {
  global $db;
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $query = "SELECT $rank_field FROM $table WHERE $key='$id'";
  return db_getOne($query);
}

// used to move object up 1 in list
function rank_up($obj_type, $id, $search='', $rank_field='rank') {
  rank_update($obj_type, $search); // make sure they're sequential
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $rank_before = rank_get($obj_type, $id) - 1;
  db_query("UPDATE $table SET $rank_field = $rank_field+1 WHERE $key != '$id' AND $rank_field = '$rank_before'".($search ? " AND ".search2where($search) : ''));
  db_query("UPDATE $table SET $rank_field = $rank_field-1 WHERE $key = '$id'");   
}

// used to move object down 1 in list
function rank_down($obj_type, $id, $search='', $rank_field='rank') { 
  rank_update($obj_type, $search); // make sure they're sequential
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $rank_after = rank_get($obj_type, $id) + 1;
  $qs = "UPDATE $table SET $rank_field = $rank_field-1 WHERE $key != '$id' AND $rank_field = '$rank_after'".($search ? " AND ".search2where($search) : '');
  db_query($qs);
  db_query("UPDATE $table SET $rank_field = $rank_field+1 WHERE $key = '$id'");   

  $log_file   = $_SESSION['log_dir'].'ajax.txt';
  write_file($log_file, "rank_down $obj_type $id \n".print_r($search, true).$qs, 'a'); // "\n------------\n\n"

}

// used to move object after target in list
function rank_after($obj_type, $id, $target, $search='', $rank_field='rank') {
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $target_rank = get_object($obj_type, $target, $rank_field);
  rank_set($obj_type, $id, $target_rank + 1);
  db_query("UPDATE $table SET $rank_field = $rank_field + 1 WHERE $key NOT IN('$id', '$target') AND $rank_field > '$target_rank'".($search ? " AND ".search2where($search) : ''));  
  return $target_rank;  
}

// used to move object before target in list
function rank_before($obj_type, $id, $target, $search='', $rank_field='rank') {
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $target_rank = $target ? get_object($obj_type, $target, $rank_field) : 0;
  rank_set($obj_type, $id, $target_rank);   
  db_query("UPDATE $table SET $rank_field = $rank_field +1 WHERE $key != '$id' AND $rank_field >= '$target_rank'".($search ? " AND ".search2where($search) : ''));
  return $target_rank;  
}

// used to monitor progress of a task (usually with jQuery progress bar)
function progressbar_create($name, $total=0) {
  $id = nextId('progress_bar');
  $now = now();
  db_query("INSERT INTO progress_bar (id, created, name, total, progress) values ($id, '$now', '$name', $total, 0)");
  return $id;
}

/** unsused - automatic comment added on  */
function progressbar_set_total($id, $total) {
  db_query("UPDATE progress_bar SET total = '$total' WHERE id=$id");
}

function progressbar_increase_progress($id, $inc=1) {
  db_query("UPDATE progress_bar SET progress = progress + $inc WHERE id=$id");
}

/** unsused - automatic comment added on  */
function progressbar_set_progress($id, $progress) {
  db_query("UPDATE progress_bar SET progress = '$progress' WHERE id=$id");
}

function progressbar_get($id, $field='') {
  global $db;
  return $field ? db_getOne("SELECT $field FROM progress_bar WHERE id=$id") : db_getRow("SELECT * FROM progress_bar WHERE id=$id");
}

/** unsused - automatic comment added on  */
function progressbar_destroy($id) {
  db_query("DELETE FROM progress_bar WHERE id=$id");
}

// delete all properties (except shortstay) that are not active, including media and descriptions 
/** unsused - automatic comment added on  */
function clean_property_db() {
  global $db;
  $ids = db_getCol("SELECT id FROM property WHERE type != 'shortstay' AND active=0");
  $ids_str = implode(',', $ids);
  $count = count($ids);
  
  $mCount = db_getOne("SELECT count(*) FROM media WHERE parent_type in ('hotel', 'realestate', 'longstay', 'boat', 'office') AND parent_id in ($ids_str)");
  $dCount = db_getOne("SELECT count(*) FROM property_description WHERE property_type in ('hotel', 'realestate', 'longstay', 'boat', 'office') AND property_id in ($ids_str)");

  if($count) {
    echo("DELETE FROM property WHERE id in ($ids_str)");                                  
    echo("DELETE FROM media WHERE parent_type in ('hotel', 'realestate', 'longstay', 'boat', 'office') AND parent_id in ($ids_str)");
    echo("DELETE FROM property_description WHERE property_type in ('hotel', 'realestate', 'longstay', 'boat', 'office') AND property_id in ($ids_str)");
               
    db_query("DELETE FROM property WHERE id in ($ids_str)");
    db_query("DELETE FROM media WHERE parent_type in ('hotel', 'realestate', 'longstay', 'boat', 'office') AND parent_id in ($ids_str)");
    db_query("DELETE FROM property_description WHERE property_type in ('hotel', 'realestate', 'longstay', 'boat', 'office') AND property_id in ($ids_str)");
  }
  echo("Deleting $count properties, $mCount media, $dCount descriptions");
}

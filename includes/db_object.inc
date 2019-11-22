<?php

/** new generice auth object function - no action (view/edit/delete) */
/** todo: use initially for invoice/res/etc */
/** return false for not authorized, 0 for not yet authorized */
function auth_object($obj_type, $obj_or_id, $userObj_or_id=0, $options=[]) {  
	global $__USER, $_SITE, $__CLIENT_ID;

	if(!$obj_type) die("auth_object: missing object type");
	$obj = any2obj($obj_type, $obj_or_id);
	if(!$obj) return 0;


	$user = $userObj_or_id ? get_user($userObj_or_id) : $__USER;
	if($user) {
    $user_level = $user->user_level;
    $user_id = $user->id;
    if($user_level == USER_LEVEL_SYSTEM) return USER_LEVEL_SYSTEM;

    if(isset($obj->client_id) && $obj->client_id != $__CLIENT_ID) return false;
    if(isset($obj->user_id) && $obj->user_id == $user_id) return $user_level;
    if(isset($obj->guest_id) && $obj->guest_id == $user_id) return USER_LEVEL_GUEST;
    if(isset($obj->host_id) && $obj->host_id == $user_id) return USER_LEVEL_OWNER;
  }

  /** todo: use pin_code in every cladss and _pc in every request */
  $pin_code = isset($options['pin_code']) ? $options['pin_code'] : $_GET['_pc'];
	if(isset($obj->pin_code) && $obj->pin_code && $pin_code) {
	  if($pin_code == $obj->pin_code) return 1;
	  if($pin_code == myhash($obj->pin_code)) return 1;
	}
    
	if(isset($obj->parent_type) && isset($obj->parent_id) && $obj->parent_type && $obj->parent_id) {
	  return auth_object($obj->parent_type, $obj->parent_id);
	}
	
	return 0;
}


/** stores classes with NF_CACHE or NF_CACHE_ASSOC in mongo */
function db2mongo($app) {
  $ts = microtime(true);
  $mongodb = $app->mongodb;
  if(!$_SESSION['mongodb'] || !$mongodb) return json_error_object("Mongo not found");
  
  $default_expiration = 0; // never expires

  $result = new stdClass;
  $errors = $messages = [];
  timer('set_env', 'db2mongo start');
  $classes = get_classes();
  $count = count($classes);
  $messages[] = "Found $count classes";
  //dump($classes);
  //die();
  
    $assoc_count = $cache_count = $row_count = 0;
  
  foreach($classes as $class) {
    $obj_type = $class->type;
    $key = get_object_key($obj_type);
    if(!$key) {
      $errors[] = "No key for $obj_type";
      continue;
    }
    $cache = $class->flags & NF_CACHE;
    $assoc = $class->flags & NF_CACHE_ASSOC;    
    if(!($cache || $assoc)) {
      //$messages[] = "{$obj_type}: no cachce";
      continue; // no caching
    }

    $cache_type = [];
    if($cache) $cache_type[] = "row";
    if($assoc) $cache_type[] = "assoc";    
    $type_str = implode(',', $cache_type);
    
    $objs = find_objects($obj_type, [], ['sort' => $key]);
    $obj_count = count($objs);
    //$messages[] = "{$obj_type}: {$type_str} count=$obj_count";
    
    $expiration = $default_expiration;

    if(!count($objs)) {
      $messages[] = "Skipping {$obj_type}, no objects";
      continue;
    }
    
    if($assoc) { /** store all rows in one associative array */
      $ar = [];
      foreach($objs as $obj) { /** store each rows */
        $obj_id = $obj->$key;
        if(!strlen($obj_id)) {
          $errors[] = "No id for ".dump($obj, true);
          continue;
        }
        $ar[$obj_id] = $obj;
      }

      $collection = $mongodb->$obj_type;
      $insertOneResult = $collection->insertOne($ar);
      $insertCount = $insertOneResult->getInsertedCount();
      $insertId = $insertOneResult->getInsertedId();
      
      $assoc_count++;
      $messages[] = "Stored assoc {$obj_type} len=".strlen($data)." count={$insertCount} id={$insertId}";
    }

    if($cache) {
      $collection = $mongodb->$obj_type;
      $insertManyResult = $collection->insertMany($objs);
      $insertCount = $insertManyResult->getInsertedCount();
      $insertIds = $insertManyResult->getInsertedIds();
      
      $messages[] = "Stored {$insertCount} {$obj_type}";
      $row_count += $insertCount;
      
      $cache_count++;              
    }    
  }
  
  $elapsed = round(microtime(true) - $ts,3);

  $messages[] = "Stored {$row_count} rows for {$cache_count} classes in $elapsed";
  $messages[] = "Stored {$assoc_count} assoc classes in $elapsed";
  
  //$message[] = dic2cache();
  
  $result->success = 1;
  $result->error = $errors;
  $result->message = $messages;
  
  timer('set_env', 'db2mongo end');
  
  return $result;
}

/** stores classes with NF_CACHE or NF_CACHE_ASSOC in flat file */
function db2filecache($app) {
  $result = new stdClass;
  $errors = $messages = [];
  timer('set_env', 'db2cache start');
  $classes = get_classes();
  $count = count($classes);
  $messages[] = "Found $count classes";
  //dump($classes);
  //die();
  
    $assoc_count = $cache_count = $row_count = 0;
  
  foreach($classes as $class) {
    $obj_type = $class->type;
    $key = get_object_key($obj_type);
    if(!$key) {
      $errors[] = "No key for $obj_type";
      continue;
    }
    $cache = $class->flags & NF_CACHE;
    $assoc = $class->flags & NF_CACHE_ASSOC;  
    if(!($cache || $assoc)) {
      continue; // no caching
    }

    $global = object_has_attribute($obj_type, 'site_id') ? 1 : 0;
    $cache_file = $global ? "{$app->app_root}/cache/global_cache.json" : "{$app->doc_root}/cache/site_cache.json";

    $cache_type = [];
    if($cache) $cache_type[] = "row";
    if($assoc) $cache_type[] = "assoc";    
    $type_str = implode(',', $cache_type);
    
    $objs = get_objects($obj_type);
    $obj_count = count($objs);
    //$messages[] = "{$obj_type}: {$type_str} count=$obj_count";
    
    $expiration = $default_expiration;

    
    if($assoc) { /** store all rows in one associative array */
      $ar = [];
      foreach($objs as $obj) { /** store each rows */
        $obj_id = $obj->$key;
        if(!strlen($obj_id)) {
          $errors[] = "No id for ".dump($obj, true);
          continue;
        }
        $ar[$obj_id] = $obj;
      }
      $redis_key = "objects-{$obj_type}";
      $data = json_encode($ar);
      if($expiration) {
        $redis->set($redis_key, $data, $expiration);
      } else {
        $redis->set($redis_key, $data);
      }
      $assoc_count++;
      $messages[] = "Stored $redis_key exp=$expiration len=".strlen($data);
    }

    if($cache) {
      foreach($objs as $obj) { /** store each rows */
        $obj_id = $obj->$key;
        if(!$obj_id) {
          $errors[] = "No id for ".dump($obj, true);
          continue;
        }
  
        $redis_key = "object-{$obj_type}-{$obj_id}";
        $data = json_encode($obj);
        
        if($expiration) {
          $redis->set($redis_key, $data, $expiration);
        } else {
          $redis->set($redis_key, $data);
        }
        //$messages[] = "Stored $redis_key exp=$expiration len=".strlen($data);
        $row_count++;      
      }
      $messages[] = "Stored {$row_count} rows for {$obj_type}";
      
      $cache_count++;              
    }    
  }
  
  $messages[] = "Stored {$row_count} rows for {$cache_count} classes";
  $messages[] = "Stored {$assoc_count} classes";
  
  $message[] = dic2cache();
  
  $result->success = 1;
  $result->error = $errors;
  $result->message = $messages;
  
  timer('set_env', 'db2cache end');
  
  return $result;
}

/** stores classes with NF_CACHE or NF_CACHE_ASSOC in redis */
function db2cache() {
  global $redis;
  $default_expiration = 0; // never expires
  
  if(!$_SESSION['redis_cache']) return json_error_object("No redis");
  $result = new stdClass;
  $errors = $messages = [];
  timer('set_env', 'db2cache start');
  $classes = get_classes();
  $count = count($classes);
  $messages[] = "Found $count classes";
  //dump($classes);
  //die();
  
    $assoc_count = $cache_count = $row_count = 0;
  
  foreach($classes as $class) {
    $obj_type = $class->type;
    $key = get_object_key($obj_type);
    if(!$key) {
      $errors[] = "No key for $obj_type";
      continue;
    }
    $cache = $class->flags & NF_CACHE;
    $assoc = $class->flags & NF_CACHE_ASSOC;    
    if(!($cache || $assoc)) {
      //$messages[] = "{$obj_type}: no cachce";
      continue; // no caching
    }

    $cache_type = [];
    if($cache) $cache_type[] = "row";
    if($assoc) $cache_type[] = "assoc";    
    $type_str = implode(',', $cache_type);
    
    $objs = get_objects($obj_type);
    $obj_count = count($objs);
    //$messages[] = "{$obj_type}: {$type_str} count=$obj_count";
    
    $expiration = $default_expiration;

    
    if($assoc) { /** store all rows in one associative array */
      $ar = [];
      foreach($objs as $obj) { /** store each rows */
        $obj_id = $obj->$key;
        if(!strlen($obj_id)) {
          $errors[] = "No id for ".dump($obj, true);
          continue;
        }
        $ar[$obj_id] = $obj;
      }
      $redis_key = "objects-{$obj_type}";
      $data = json_encode($ar);
      if($expiration) {
        $redis->set($redis_key, $data, $expiration);
      } else {
        $redis->set($redis_key, $data);
      }
      $assoc_count++;
      $messages[] = "Stored $redis_key exp=$expiration len=".strlen($data);
    }

    if($cache) {
      foreach($objs as $obj) { /** store each rows */
        $obj_id = $obj->$key;
        if(!$obj_id) {
          $errors[] = "No id for ".dump($obj, true);
          continue;
        }
  
        $redis_key = "object-{$obj_type}-{$obj_id}";
        $data = json_encode($obj);
        
        if($expiration) {
          $redis->set($redis_key, $data, $expiration);
        } else {
          $redis->set($redis_key, $data);
        }
        //$messages[] = "Stored $redis_key exp=$expiration len=".strlen($data);
        $row_count++;      
      }
      $messages[] = "Stored {$row_count} rows for {$obj_type}";
      
      $cache_count++;              
    }    
  }
  
  $messages[] = "Stored {$row_count} rows for {$cache_count} classes";
  $messages[] = "Stored {$assoc_count} classes";
  
  $message[] = dic2cache();
  
  $result->success = 1;
  $result->error = $errors;
  $result->message = $messages;
  
  timer('set_env', 'db2cache end');
  
  return $result;
}

function dic2cache() {
  $dicObjs = find_objects('dictionary');
  $count = count($dicObjs);
  $output .= "{$count} in dic";
  foreach($dicObjs as $dicObj) {
    $site_id = $dicObj->site_id ?: 0;
    $lang = $dicObj->language;
    $term = $dicObj->term;
    
    $redis_key = "dictionary-{$site_id}-{$lang}-{$term}";
    $redis->set($redis_key, $dicObj->phrase);
    
  }   
  return "Saved $count dictionary terms to cache";
}

// object functions used in admin

// reads classes from db, writes to file
function db2classes() {
  $classes = find_objects('class', array('active' => 1), array('sort' => '`group`, type', 'debug_query' => 0));
  //dump($classes);die();
  $delim = PHP_EOL;
  //$delim = html_break(); // for screen - testing
  $filename = "/web/comps/php/apt-dev/include/classes-test.inc";
  $count = count($classes);
  $output = '<?php'.$delim;
  $output .= '/** Automatically generated - do not edit */'.$delim;
  $output .= "/** There are $count active classes */".$delim.$delim;
  
  print_log("/** There are $count active classes */", 'classes', LOG_LEVEL_TEST);
  
  $classAr = array();
  $this_group = '';
  foreach($classes as $classObj) {
    $line = '';
    $class = $classObj->type;
    $group = $classObj->group;
    if($group != $this_group) {
      $this_group = $group;
      $line .= $delim."    /** $this_group */".$delim;
    }
    $line .= "    '$class' =>                array($classObj->user_level, '$group', '$classObj->table', '$classObj->key', $classObj->flags, '$classObj->descriptor', '$classObj->sort', '$classObj->parent_class', '$classObj->site_id')";
    $classAr[]= $line;
  }
  $output .= '$__CLASSES = array('.$delim.implode(",".$delim, $classAr).');'.$delim.$delim;
  
  $output .= '/** Attributes */'.$delim.$delim;
  $output .= '$__CLASS_ATTRIBUTES = array('.$delim;

  foreach($classes as $classObj) {
    $class = $classObj->type;
    $attributes = find_objects('attribute', array('class' => $classObj->type));
    $attAr = array();
    //$line = $delim."    /** $classObj->name */".$delim;
    $line = '';

    print_log("Adding class {$class}", 'classes', LOG_LEVEL_TEST);
    
    //dump($attributes);
    foreach($attributes as $attObj) {
      $line = '';
      $ar = array("'$attObj->name'", "'$attObj->type'", "'$attObj->default_value'", $attObj->flags);
      if(in_array($attObj->type, array('int','str'))) {
        $ar[] = $attObj->length;        
      } elseif(in_array($attObj->type, array('media','valSelect','intSelect'))) {
        $ar[] = "'$attObj->relation_filters'"; /** in 4th position */
      } elseif(in_array($attObj->type, array('link','many2one','msel','ssel', 'one2one'))) {
        $ar[] = "'$attObj->relation_class'";
        if($attObj->relation_filters) {
          $ar[] = "'$attObj->relation_filters'"; /** in 5th position for link, many2one */
          if($attObj->relation_sort) $ar[] = "'$attObj->relation_sort'";
        }
      }
      
      $default = $attObj->default;
      $line .= "    '$attObj->field'=> array(".implode(', ', $ar).")";
      $attAr[]= $line;
      print_log("Adding attribute {$attObj->name} ({$attObj->type})", 'classes', LOG_LEVEL_TEST);
    }
    //dump($attAr);die();
    $output .= '  "'.$class.'" => array('.$delim.implode(",".$delim, $attAr).'),'.$delim.$delim;
  }
  $output .= ');'.$delim;
  
  write_file($filename, $output);
  return $output;
}

// compare classes in DB to those in classes.inc
// if $class is empty, check all
/** unsused - automatic comment added on  */
function compare_db2classes($class = '') {
  global $__CLASSES, $__CLASS_ATTRIBUTES, $db;
  $class_defs = array_keys($__CLASSES);
  $attr_classes = array_keys($__CLASS_ATTRIBUTES);
  $attr_count = count($attr_classes);
  $count = count($class_defs);
  
  $output = '';
  $sqlAr = array();
  $outputAr = array();
  $missing_attributes = array_diff($class_defs, $attr_classes);
  $missing_classes = array_diff($attr_classes, $class_defs);

  if($table = $_GET['view']) {
    //$view_table = get_object_table($view);
    $output .= html_h3("Rows in $table");
    $rows = $db->getAll("SELECT * FROM $table");
    $output .= dump($rows, true);
    //$output .= object_table($view);
  }
  
  if($drop = $_GET['drop']) {
  }
  
  $output .= html_h3("There are $count classes and $attr_count attribute sets");
  if(count($missing_classes)) {
    $output .= html_h4("Missing classes").dump($missing_classes, true);
  }
  if(count($missing_attributes)) {
    $output .= html_h4("Missing attribute sets").dump($missing_attributes, true);
  }

  $class_tables = $db->getCol("SELECT distinct(`table`) FROM class");
  $db_tables = $db->getCol("show tables");
  $output .= html_h4(count($class_tables)." Class Tables");
  $output .= html_h4(count($db_tables)." DB Tables");
  
  $missing_tables = array_diff($class_tables, $db_tables);
  if($count = count($missing_tables)) {
    $output .= "$count classes have no database table<BR>";
    $output .= html_h4("Missing tables that have class definition").dump($missing_tables, true);
  }

  /** check for missing/extra tables */
  $extra_tables = array_diff($db_tables, $class_tables);
  //dump($missing_classes);
  if($count = count($extra_tables)) {
    //$output .= "$count tables can potentially be deleted from DB<BR>";
    $output .= html_h4("$count Missing classes that have DB tables");
    foreach($extra_tables as $table) {
      $row_count = $db->getOne("SELECT count(*) FROM $table");
      $output .= html_b($table)." ($row_count rows) ".html_link("?compare&drop=$table", phrase('drop'))." ".html_link("?compare&view=$table", phrase('view')).html_break();
      $sqlAr[]= "drop table $table;";
    }
  }
  
  $classes = find_objects('class');
  //$classes = array();
  $rootTables = array();
  
  /** loop through to find root class of classes with parent */
  foreach($classes as $classObj) {
    $table = $classObj->table;
    $obj_type = $classObj->type;
    $class_cols = find_objects('attribute', array('class' => $obj_type, 'type' => array('<>', array('one2many', 'many2many', 'dummy'))), array('field' => 'field'));
  
    if($classObj->parent_class) {
      $rootTables[$table][$obj_type] = $class_cols;
      continue;
    }
  }

  // dump($parentTables);
  $rootClassTables = array_keys($rootTables);
  
  /** check for missing/extra columns */
  foreach($classes as $classObj) {
    $table = $classObj->table;
    $obj_type = $classObj->type;
    if($classObj->parent_class) continue; // skip child classes 
       
    $class_cols = find_objects('attribute', array('class' => $obj_type, 'type' => array('<>', array('one2many', 'many2many', 'dummy'))), array('field' => 'field'));
    $cols = $db->getAll("show columns from $table");
    $db_cols = array_find_key($cols, 'field');

    $root_class = in_array($table, $rootClassTables);    
    if($root_class) {
      foreach($rootTables[$table] as $child_class => $childCols) {
        $class_cols = array_merge($class_cols, $childCols);
      }
      $class_cols = array_unique($class_cols);
      $output .= "$obj_type is root class (table=$table)<br>";
      //$output .= dump($rootTables[$table], true);
      $output .= dump($class_cols, true);
    } 
    
    $missing_fields = array_diff($class_cols, $db_cols);
    if($count = count($missing_fields)) {
      $output .= "$obj_type: $count Missing fields:".dump($missing_fields, true);
    }
    $extra_fields = array_diff($db_cols, $class_cols);
    if($count = count($extra_fields)) {
      $output .= "$obj_type: $count Extra fields:".dump($extra_fields, true);
      foreach($extra_fields as $column) $sqlAr[]= "alter table $table drop column $column";
    }
  }
  
  if(count($sqlAr)) {
    if(isset($_GET['execute'])) {
      $output .= html_h3("Executing...");
      $output .= implode(PHP_EOL, $sqlAr);
      foreach($sqlAr as $cmd) db_query($cmd);
      $output .= html_b("Done");
      
    } else {
      $sql = implode(PHP_EOL, $sqlAr);  
      $output .= html_h3("SQL").form_text('foo', $sql, array('cols' => 80, 'rows' => count($sqlAr))).html_break(2);
      $output .= html_link("?execute", "Execute", array("class" => "btn btn-danger"));
    }
  }
  return $output;
}

// reads classes from file, stores in new DB structure
/** unsused - automatic comment added on  */
function classes2db() {
  global $__CLASSES, $__CLASS_ATTRIBUTES;  

  $output = "There are ".count($__CLASSES)." classes<br>";
  // include site classes
  $siteObjs = find_objects('site', array('live' => 1));
  foreach($siteObjs as $siteObj) {
    $site_dir = site_directory($siteObj);
    $prefix = $siteObj->shortname;
    
    $class_file = "$site_dir/include/".$prefix."_classes.inc";
    if(file_exists($class_file)) {
      $output .= "Including $class_file<br>";
      include_once($class_file);
    }
  }
  $output .= "There are now ".count($__CLASSES)." classes<br>";

  $classes = get_classes();
  $count = count($classes);
  
  //dump($classes);die();
  $output = '';
  $class_count = $att_count = 0;
  $classAr = array();
  $missingTypes = array();

  print_log("/** There are $count  classes */", 'classes', LOG_LEVEL_TEST);
  
  foreach($classes as $class) {
    $obj_type = $class->type;
    if($obj = get_object('class', $class->type)) {
      //$output .= "class $class->type already exists, updating<br>";
      //list($id, $errors) = update_object('class', $class, $class->type);
    } else {
      print_log("Adding class {$obj_type}", 'classes', LOG_LEVEL_TEST);
      list($id, $errors) = add_object('class', $class, array('preset_id' => $class->type));
      //$output .= "name=$class->name type=$class->type added class $id<br>";
    }
    if($errors) {dump($errors); die();}
    $class_count++;
    $attributes = get_object_attributes($obj_type, 0);
    $acount = count($attributes);
    
    print_log(" * {$obj_type} has {$acount} attributes", 'classes', LOG_LEVEL_TEST);
    
    // dump($attributes);die();
    //if($obj_type == 'price_breakdown') {
    //  dump($class);
    //  dump($attributes);
    //  $count = count($attributes);
    //  echo("Found $count attributes for $obj_type");
    //  //die();
    //}
    $classAr[$obj_type] = 0;
    foreach($attributes as $field => $attr) {
      
      
      $attrObj = NULL;
      $attrObj->class = $class->type;
      $attrObj->field = $field;
      $attrObj->name = $attr[ATT_NAME];
      $attrObj->type = $type = $attr[ATT_TYPE];
      $attrObj->default_value = $attr[ATT_DEFAULT];
      $attrObj->flags = $attr[ATT_FLAGS];


      if(find_object('attribute', ['class' => $obj_type, 'field' => $field])) {
        print_log(" - attribute {$field} ({$type} already exists", 'classes', LOG_LEVEL_TEST);
        continue;
      }
      print_log(" + adding attribute {$field} ({$type})", 'classes', LOG_LEVEL_TEST);
      //if($obj_type == 'price_breakdown') echo("field=$field type=$type<br>");
      if(!$type) {
        dump($attributes);
        dump($attr);
        die("missing type in field=$field in $obj_type");
      }

      $dbTypeObj = get_object('attribute_type', $type);
      if(!$dbTypeObj) $missingTypes[$type]++; //echo("classes2db: No db_type found for $type in $obj_type<br>");
      
      if(in_array($type, array('valSelect', 'media', 'intSelect'))) {
        if($options = $attr[ATT_OPTIONS]) {
          $attrObj->relation_filters = json_encode($options);
        }
      } elseif(in_array($type, array('msel', 'ssel', 'one2one', 'link'))) {
        $attrObj->relation_class = $attr[ATT_RELATION_TYPE];
      } else if(in_array($type, array('many2one'))) {
        $attrObj->relation_class = $attr[ATT_RELATION_TYPE];
        if($filters = $attr[ATT_FILTERS]) {
          $attrObj->relation_filters = json_encode($filters);
        }
        if($sort = $attr[ATT_SORT]) $attrObj->relation_sort = $sort;
      } else if(in_array($type, array('str', 'int'))) {
        $attrObj->length = $attr[ATT_LENGTH];
      }
      
      // depending on type
      // $
      // $attrObj->relation_filters  = $attr[ATT_LENGTH];
      // $attrObj->relation_sort     = $attr[ATT_LENGTH];
      // $attrObj->args1             = $attr[ATT_LENGTH];
      // $attrObj->args2             = $attr[ATT_LENGTH];
      
      //dump($attr);dump($attrObj);die();

      if($att_id = find_object('attribute', array('class' => $class->type, 'field' => $field), 'id')) {
        list($id, $errors) = update_object('attribute', $attrObj, $att_id);
      } else {
        list($id, $errors) = add_object('attribute', $attrObj);
      }
      
      if($errors) {dump($errors); dump($attr); dump($attrObj); die();}
      $class_count++;
      $att_count++;
      $classAr[$obj_type]++;
      //$output .= "added attribute $id<br>";
    }   
  }
  echo("Missing types:".dump($missingTypes,true));
  return $classAr;
}

function admin_menu_items($siteObj, $options=NULL) {
  global $__CLASSES;
  global $u_id;
  global $u_level;
  $user_level = $options['user_level'] ?: $u_level;
  
  $menu_items = array();
  foreach($__CLASSES as $obj_type => $class) {
    $class_site_id = $class[CLASS_SITE_ID];
    if($class_site_id && $class_site_id != $siteObj->id) continue; // not for this site
    $min_u_level = $class[CLASS_USER_LEVEL];
    if($min_u_level == SYSTEM_USER_ID && $u_id != SYSTEM_USER_ID) continue; // for my eyes only  
    if($user_level < $min_u_level) continue; // not auth
    if($class[CLASS_FLAGS] & NF_NO_LIST || $class[CLASS_FLAGS] & NF_NO_ACTIONS) continue; // no list or no actions
    if(in_array($class[CLASS_GROUP], array('enquiry', 'property', 'event', 'location')) && !$siteObj->show_types) continue; // don't show property related stuff for non-property sites (todo: don't load the classes)  
    if(is_array($_SESSION['property_types']) && $class[CLASS_GROUP]=='property' && !in_array($obj_type, $_SESSION['property_types'])) continue; // don't show this property type 
    if(!auth_action($obj_type, 'list', 0, $user_level)) continue; // requires DB lookup
    $class_name = get_object_name($obj_type);
    if($options['return_types']) $menu_items[] = $obj_type;
    else $menu_items[phrase($class[CLASS_GROUP], CAPITALIZE)][$class_name] = $obj_type; //get_object_link($obj_type, 0);
  }
  if(!$options['return_types']) foreach($menu_items as $k=>$v) asort($v);
  return ($menu_items);
}

/** unsused - automatic comment added on  */
function print_admin_menu($siteObj, $options=NULL) {
	$menu_items = admin_menu_items($siteObj, $options);		
	return jquery_menu($menu_items, phrase('objects', CAPITALIZE), $jqOptions, array('id' => 'admin_menu'));
}


// returns all classes with defaults where not in $__CLASSES
function get_classes() {  
  global $__CLASSES;
  $classes = array();
  foreach($__CLASSES as $type => $class_array) {
    $class = NULL;
    $class->type  = $type;
    $class->name  = get_object_name($type);

    $class->active = 1;    
    $class->user_level = get_object_user_level($type);    
    $class->group = get_object_group($type);
    $class->table = get_object_table($type);
    $class->key   = get_object_key($type);
    $class->flags = get_object_flags($type);
    $class->descriptor = get_object_descriptor($type);
    $class->sort = get_object_sort($type);
    $class->parent_class = get_object_parent_class($type);
    $class->site_id = get_object_site_id($type);
    
    $classes[] = $class;
  }
  return $classes;
}

// return name of field which is the primary id of the object (typically 'id')
function get_object_key($type, $table = false) {                                                                                        
  global $__CLASSES;
  $class = $__CLASSES[$type];
  $key = $class[CLASS_KEY] ? $class[CLASS_KEY] : 'id';
  if($table) $key = get_object_table($type).'.'.$key;
  return $key;
  //$table = get_object_table($type);
  //return "$table.$key";
}

function get_object_user_level($type) {                                                                                        
  global $__CLASSES;
  $class = $__CLASSES[$type];
  $user_level = $class[CLASS_USER_LEVEL] ? $class[CLASS_USER_LEVEL] : 'id';
  return $user_level;
  //$table = get_object_table($type);
  //return "$table.$key";
}

function get_object_table($type) {
  global $__CLASSES;
  $class = $__CLASSES[$type];
  return $class[CLASS_TABLE] ? $class[CLASS_TABLE] : $type;
}

function get_object_name($type, $case=CAPITALIZE) {
  switch($type) {
    case 'inv_lineitem': return phrase('line item', $case);    
    case 'org': return phrase('organization', $case);    
    case 'reservation': return phrase('booking', $case);    
    case 'shortstay': return phrase('rental', $case);    
    case 'gallery': return phrase('folder', $case);; // hack for now
    case 'res_exception': return phrase('calendar rule', $case);; // hack for now
    case 'media_collection': return phrase('exhibition', $case);    
    case 'media_collection_map': return phrase('artwork', $case);    
    default: break;
  }
  $type = str_replace(['inv_', 'res_'], ['invoice_', 'booking_'], $type);
  return phrase($type, $case);
  
  //return ucfirst(str_replace(array('_', 'inv ', 'org'), array(' ', 'invoice ', 'organization'), $type));
  //global $__CLASSES;
  //$class = $__CLASSES[$type];
  //return $class[CLASS_NAME] ? $class[CLASS_NAME] : ucfirst(str_replace(array('_', 'inv ', 'org'), array(' ', 'invoice ', 'organization'), $type));
}

function get_object_group($type) {
  global $__CLASSES;
  $class = $__CLASSES[$type];
  return $class[CLASS_GROUP] ? $class[CLASS_GROUP] : 'unsorted';
}

function get_object_flags($type) {
  global $__CLASSES;
  $class = $__CLASSES[$type];
  return $class[CLASS_FLAGS] ? $class[CLASS_FLAGS] : 0;
}

function get_object_sort($type) {
  global $__CLASSES;
  $class = $__CLASSES[$type];
  return $class[CLASS_SORT] ? $class[CLASS_SORT] : '';
}

function get_object_parent_class($type) {
  global $__CLASSES;
  $class = $__CLASSES[$type];
  return $class[CLASS_PARENT_CLASS] ?: '';
}

function get_object_site_id($type) {
  global $__CLASSES;
  $class = $__CLASSES[$type];
  return $class[CLASS_SITE_ID] ?: 0;
}

// todo: move to class definition
//what's the diff between get_object_sort_clause and get_object_sort_default ??
function get_object_sort_clause($type) {
  global $__CLASSES;
  $class = $__CLASSES[$type];
  $table = get_object_table($type);
  if($class[CLASS_SORT]) return $class[CLASS_SORT];
  $key = get_object_key($type);
  return "$table.$key DESC"; //default - show newest first
}

//function add_object($obj_type, $data, $parent_id=0, $preset_id=0, $parent_type='') {
function add_object($obj_type, $data, $options = array()) {
  //echo("add_object $obj_type").dump($data);die();
  
  $data = (array) $data;
  $child_array = array(); // array of arrays [$child_type, $child_id]
  $errors = $child_errors = array();
  $messages = array();
  
	// for new objects, let's first recursively process children, as we don't want to add parent object if there are child errors 

  $fields = get_object_attributes($obj_type, 0);
  foreach($fields as $field => $class) {
    $att_flags = $class[ATT_FLAGS];
    $att_type = $class[ATT_TYPE];
    $child_type = $att_options = $class[ATT_LENGTH];
    if($att_type == 'one2one' && !$parent_id) { // set parent id to indictate that the parent already exists (e.g. adding owner to existing user)
      $child_data = $data; // copy
      $childs_parent_type_fields = find_class_fields($child_type, array(ATT_TYPE => 'parent_type'));
      foreach($childs_parent_type_fields as $childs_parent_type_field) $child_data[$childs_parent_type_field] = $obj_type; // look up name of field based on type 

      $child_required = $att_flags & AF_OBLIGATORY;
      list($child_id, $child_errors) = add_object($child_type, $child_data); // delegate to child (add or update)
      
      if($child_required && (!$child_id || $child_errors)) return array(0, $child_errors ?: array());
      if($child_id) {
        $data[$field] = $child_id; // ok, child added
        $child_array[] = array($child_type, $child_id);
      }
    }
  }
  
  if($child_errors) return array(0, $child_errors, $messages);
  //echo "<pre>"; print_r($options);
  list($id, $errors, $messages) = save_object_query($obj_type, $data, $options);
  
  if($id && count($child_array)) { // success, update children to point to parent
    foreach($child_array as $childAr) {
      list($child_type, $child_id) = $childAr;
      $parent_id_fields = find_class_fields($child_type, array(ATT_TYPE => 'parent_id'));
      $child_data = array();
      foreach($parent_id_fields as $parent_id_field) $child_data[$parent_id_field] = $id; 
      if($child_data && $child_type && $child_id) update_object($child_type, $child_data, $child_id);
    }
  }
  
  return array($id, $errors, $messages);
}

// get obj_id from page_id
function getobj_id($obj_type, $user_id, $page_id){
  $data = find_object($obj_type, ['user_id' => $user_id, 'page_id' =>$page_id , 'active' => 1]);
  $obj_id = $data->id;
  return $obj_id;
}

//Add default pages (Home, Artist, Exhibition, Contact)
function add_default_page($id, $data, $options = array()) {
  if(!data['user_id']) mydie ("add_default_pages: missing id");
  $obj_type = 'minisite_page';
  $minisite_id = $id;
  $type = ['20','30'];
  
  foreach($type as $types){
    if($types == '20'){
      $page = 'artist';
      $slug = url_safe("/".$page."/", true);
      $data['page_layout'] = get_layout($types, 1);
      $data['second_layout'] = get_layout($types, 2);
      $data['row_order'] = 2;
    } 
    if($types == '30'){
      $page = 'exhibition';
      $slug = url_safe("/".$page."/", true);
      $datas = get_exhibitionJson();
      $data['page_layout'] = get_layout($types, 1);
      $data['second_layout'] = get_layout($types, 2);
      $data['row_order'] = 1;
    } 
    $data['type'] = $types; 
    $data['menu_title'] = $page;
    $data['page_title'] = $page;
    $data['slug'] = $slug;
    $data['data'] = ($page == 'exhibition')?$datas:'';
    $data['minisite_id'] = $minisite_id;
    if($data){
      list($page_id, $errors) = add_object($obj_type, $data);
    }else{
      json_error("No datas found for default pages");
    }
    if($errors){
      json_error($errors);
    } 
  }
  
  return array($data, $errors, $messages);
}

// update one object
function update_object($obj_type, $data, $id, $options = array()) {

  if(!$id) mydie ("update_object $obj_type: missing id");

  $options['id'] = $id;
    
  list($id, $errors, $messages) = save_object_query($obj_type, $data, $options);

  if($debug) print_log("Update $obj_type $id:".caller_stack().dump($data, true), 'reservation', LOG_LEVEL_TEST);

  return array($id, $errors, $messages);
}

// update multiple objects (with different data - id and data are both array with corresponding indeces)
function update_objects_array($obj_type, $data_array, $ids, $options = array()) {
  if(!$ids) die ("update_objects_array $obj_type: missing id");
  $ids = is_array($ids) ? $ids : explode(',', $ids); // convert to array
  if(!is_array($ids)) die ("update_objects_array: id must be comma separated list or array");
  $i=0;
  $errors = array();
  foreach($ids as $id) {
    $data = $data_array[$i];
    list($nid, $nerrors) = update_object($obj_type, $data, $id, $options);
    if($nerrors) $errors = array_merge($errors, $nerrors);
    $i++;
  }
  return array($ids, $errors);
}

// deletes multiple objects
function delete_objects($obj_type, $ids, $check_relations=true) {
  $count = 0;
  if(!$ids) return array(0, array("delete_objects $obj_type: missing id"));
  $ids = is_array($ids) ? $ids : explode(',', $ids); // convert to array
  if(!is_array($ids)) return array(0, array("delete_objects: id must be comma separated list or array"));
  $errors = array();
  foreach($ids as $id) {
    list($c_count, $c_errors) = delete_object($obj_type, $id, $check_relations); 
    $count += $c_count; $errors = array_merge($errors, $c_errors); 
    //if($errors) return array($count, $errors); // no point deleting more?
  }
  return array($count, $errors);
}

// update multiple objects (with same data)
function update_objects($obj_type, $data, $search = array(), $options = array()) {
  if(isset($options['id']) && $options['id']) {
    if(is_array($options['id']) && count($options['id'])) $ids = $options['id'];
    else $ids = explode(',', $options['id']);
  } else { 
    $key = get_object_key($obj_type);
    $ids = find_objects($obj_type, $search, ['field' => $key]);
  }
  if(!$ids) return [0, ""]; // die ("update_objects $obj_type: missing id");
  $count = count($ids);
  $options['id'] = $ids;  
  list($id, $errors, $messages) = save_object_query($obj_type, $data, $options);
  return array($count, $errors);
}


// alias
/** unsused - automatic comment added on  */
function search2sql($search) {
  return search2where($search);
}

function search2where($search) {
  $filters = search2filters($search);
  return is_array($filters) ? implode(" AND ", $filters) : '';  
}

// same as add or update, but id passed explicitly
function save_object($obj_type, $data, $id=0) {    
  if(!$obj_type) die("Cannot save object - missing object type");
  if($id) return update_object($obj_type, $data, $id);
  //echo("adding object $obj_type");dump($data);die();
  return add_object($obj_type, $data);
}

// same as save, but id passed implicitly in data
function add_or_update_object($obj_type, $data, $options = array()) {    
  if($id=get_object_id($obj_type, $data)) return update_object($obj_type, $data, $id, $options);
  return add_object($obj_type, $data, $options);
}

function get_object_id($obj_type, $data) {
  $key = get_object_key($obj_type);
  return is_object($data) ? $data->$key : $data[$key];
}


// returns array of fields that have changed (optionally including details of what changed)
// optionally logs changes to table changlog
// todo implement test for relations (one2one, one2many, etc)
function compare_objects($obj_type, $orgObj, $newObj, $show_details=false, $log_changes=false, $get_relations=false) {
  $class_def = get_class_def($obj_type, $get_relations);

  $fields = $class_def->fields;
  $key = $class_def->key;
  $id = $orgObj->$key;
  $changes = array();
  foreach($fields as $field => $attributes) {
    $from = $to = $change_id = $change_str = '';
    
    $field_name = $attributes[ATT_NAME];
    $att_type = $attributes[ATT_TYPE];
    $att_flags = $attributes[ATT_FLAGS];
    $att_options = $attributes[ATT_RELATION_TYPE];
    $child_field = false;
    // only use db field name w/o parent. (so address.first_name => first_name) 
    if($get_relations && ($p=strrpos($field, '.'))) {
      $field = substr($field, $p+1);
      $child_field = true;
    }
    
    $from = $orgObj->$field;
    $to = $newObj->$field;
        
    if($att_type == 'dummy') {
      continue;
    } elseif($att_type == 'one2one') {
      continue;
    } elseif($att_type == 'one2many') {
      continue;
    } elseif($att_type == 'many2one') {
      $from_str = get_select_value2($att_options, $from);
      $to_str   = get_select_value2($att_options, $to);
    } else {
      $from_str = $from;
      $to_str = $to;
    }
    $changed = $from != $to;
    $change_str = "$field_name: '$from_str' => '$to_str'";
    if($changed) {
      if($log_changes && !$child_field && ($attributes[ATT_FLAGS] & AF_LOG)) {
        $changeArray['obj_type'] = $obj_type;
        $changeArray['parent_id'] = $id;
        $changeArray['field'] = $field;
        $changeArray['from_value'] = $from;
        $changeArray['to_value'] = $to;
        $changeArray['action'] = $change_str;
        list($change_id, $change_errors) = add_object('changelog', $changeArray);
      }
    
      if($show_details) {
        $changes[$field]['old'] = $from;
        $changes[$field]['new'] = $to;
        $changes[$field]['str'] = $change_str;
        $changes[$field]['id']  = $change_id;
      } else {
        $changes[] = $field;
      }
    }
  }
  return $changes;
}

// copies any object fields from $source to $dest if they are in class definition
// if overwrite = false, the value of destination will be preserved if set
/** unsused - automatic comment added on  */
function copy_object_vars($obj_type, $source, $dest=NULL, $overwrite=true) { 
  if(is_array($source)) $source = array2obj($source);
  if(is_array($dest)) $dest = array2obj($dest);
  $class_fields = get_object_attributes($obj_type, true);
  foreach($class_fields as $full_key => $val) {
    $key = ($pos = strpos($full_key, '.')) ? substr($full_key, $pos+1) : $full_key; // composite field, use last only
    $val = trim($source->$key);
    if(isset($source->$key) && ($overwrite || !isset($dest->$key))) $dest->$key = $val; // exists, not already set, copy
  }
  return $dest;
} 

// performs shallow copy (no relations)
function copy_object($obj_type, $src_id, $dest_id=0) {
	$src_obj = get_object($obj_type, $src_id);
  $key = get_object_key($obj_type);
	unset($src_obj->$key);
	if($dest_id) {
	  //echo("copy_object: updating $dest_id<br>");
	  list($new_id, $errors) = update_object($obj_type, obj2array($src_obj), $dest_id);
	} else {
	  //echo("copy_object: fresh copy<br>");
	  list($new_id, $errors) = add_object($obj_type, obj2array($src_obj));
	}
	return array($new_id, $errors);		
}

// return false if object should not be cached, true otherwise
/** unsused - automatic comment added on  */
function object_cache_filter($obj_type, $obj) {
  if(!$_SESSION['system_cache']) return false;
  $this_year = date("Y");
  $last_year = $this_year - 1;
  $next_year = $this_year + 1;
  
  $obj = (object) $obj;
  switch($obj_type) {
  case 'reservation':
    // todo: make +/- 1 month for hourly reservations
    return $obj->status > 0 && $obj->checkout > $last_year && $obj->checkin < $next_year; 
    break;
  case 'invoice':
    return $obj->status > 0 && $obj->end_date > $last_year && $obj->start_date < $next_year; 
    break;
  case 'shortstay':
    return $obj->active;
    break;
  default:
    return true;
  }
}

function objects_cache_key($obj_type) {
  if(!$_SESSION['system_cache']) return null;
  $dev = $_SESSION['dev'] ? "dev-" : '';
  return $obj_type ? $dev."objects-$obj_type" : '';
}

function object_cache_key($obj_type, $id) {
  if(!$_SESSION['system_cache']) return null;
  $dev = $_SESSION['dev'] ? "dev-" : '';
  return $id && $obj_type ? $dev."object-$obj_type-$id" : '';
}


/** unsused - automatic comment added on  */
function db_plain_fields($obj_type, $fields) { // the fields contain SQL syntax, use DB
  return true;
  foreach($fields as $field) {
    if(!preg_match()) return false;
  }
  return true;
}


// stores all objects as assoc. array
function object_cache_fetch_assoc($obj_type, $id=0, $fields=NULL) {
  if(!$_SESSION['system_cache']) return null;
  global $redis;
  
  $fields = is_array($fields) && count($fields) ? $fields : array();  
  if($fields && !db_plain_fields($obj_type, $fields)) return NULL; // the fields contain SQL syntax, use DB

  $apc_key = objects_cache_key($obj_type);    
  if($value = $redis->get($apc_key)) {
    $objs = json_decode($value);

    //echo("Gh: $apc_key ".dump($objs,true));   
    if($id) { // just one
      $result = $obj = $objs->$id;
      $field = $fields[0];
      if(count($fields) == 1) { // note: does not work for e.g: distinct(field), check if field == class field
        $result = $obj->$field;
        //echo("id=$id field=$field result=$result".dump($obj, true));
        //print_log("FETCH Assoc. $obj_type $id One field=$field Result=$result\n", 'db', LOG_LEVEL_DUMP);
        return $result; // return single value
      } else if($fields) {
        foreach($fields as $field) $cObj2->$field = $obj->$field;
        return $cObj2; // return object with juse some of the fields;
      } else {
        return $obj; // return all fields
      }
      
    } else { // get all
      $result = $objs;
    }

    //$debug = $id ? " Dump $obj_type $id: ".dump($result, true) : '';
    print_log("CACHE FETCH Assoc. $obj_type $id Found ".count($objs)." objects\n", 'cache', LOG_LEVEL_TEST);
    
  } else {
    print_log("CACHE FETCH Assoc. Did not find any $obj_type objects\n", 'cache', LOG_LEVEL_INFO);
  }

  return $result;
}

function object_cache_store_assoc($obj_type) {
  if(!$_SESSION['system_cache']) return null;
  global $redis;
  
  $apc_key = objects_cache_key($obj_type);
  $obj_key = get_object_key($obj_type);
  $data = array();
  if($objs = find_objects($obj_type)) {
    foreach($objs as $obj) {
      $id = $obj->$obj_key;
      $data[$id] = (object) $obj;
    }
    if($result = $redis->set($apc_key, json_encode($data))) {
      print_log("CACHE STORE Assoc. $apc_key for $obj_type (".count($objs).")", 'cache', LOG_LEVEL_TEST);
    } else {    
      print_log("CACHE FAIL Failed to store Assoc $obj_type\n", 'cache', LOG_LEVEL_WARNING);
    }     
  }
  return $result;
}


function object_cache_fetch($obj_type, $id, $fields=NULL) {
  if(!$_SESSION['system_cache']) return null;
  global $redis;
  $fields = (array) $fields;
  $apc_key = object_cache_key($obj_type, $id);    
  $count = count($fields);
  
  print_log("CACHE FETCH $obj_type $id fields:$count\n", 'cache', LOG_LEVEL_TEST);
  if($value = $redis->get($apc_key)) {                         
    $obj = json_decode($value);
    if($count == 1) {
      $field = $fields[0];
      $value = $obj->$field;
      print_log("CACHE FETCHED $obj_type $id $field: $value", 'cache', LOG_LEVEL_TEST);
      return $value;
    } else if($count > 1) {
      foreach($fields as $field) $cObj2->$field = $obj->$field;
      return $cObj2;
    }              

  } else {    
    print_log("Failed to fetch $obj_type $id", 'cache', LOG_LEVEL_WARNING);
    return null;
  }
  return $obj;  
}

function object_cache_delete($obj_type, $id) {
  $apc_key = object_cache_key($obj_type, $id);
  return system_cache_delete($apc_key);
}

function object_cache_store($obj_type, $id, $obj) {
  if(!$_SESSION['system_cache']) return null;  
  if(!$obj) return false;  
  $apc_key = object_cache_key($obj_type, $id);
  if(!object_cache_filter($obj_type, $obj)) return false; // filtered out
  return system_cache_store($apc_key, $obj);
}

function get_object($obj_type, $id, $selected_fields='', $options=array()) {
  if($options && !is_array($options)) die("get_object: options not array");
  
  if(!$id) {
    print_debug("get_object $obj_type: missing id");
    return NULL;
  }
  $fields = array();
  if($selected_fields) {
    $fields = is_array($selected_fields) ? $selected_fields : explode(',', $selected_fields);
    $field_count = count($fields);
  }
  
  $classFlags = get_object_flags($obj_type);
  if($classFlags & NF_CACHE_ASSOC) {
    if($cachedObj = object_cache_fetch_assoc($obj_type, $id, $fields)) {
      return $cachedObj;
    }
  } else if($classFlags & NF_CACHE) {
    $cachedObj = object_cache_fetch($obj_type, $id, $fields);
    
    if($cachedObj !== null) {
      return $cachedObj;
    }
    print_log("No $obj_type $id in cache", 'cache', LOG_LEVEL_TEST);
  }

  $qs = get_object_query($obj_type, $id, $fields, $options);
  
  $result = $field_count == 1 ? db_getOne($qs) : db_getRow($qs);
  check_db_result($result);    
  if($classFlags & NF_CACHE_ASSOC) object_cache_store_assoc($obj_type);
  elseif($classFlags & NF_CACHE && !$selected_fields) object_cache_store($obj_type, $id, $result);
  return $result;
}

// returns value of field from object with id
// IMPORTANT NOTE: This does not fetch child fields (one2one). Use get_object_value for this.
function get_object_field($obj_type, $id, $field) {
  if(!$field) return '';
  $key = get_object_key($obj_type);
  $table = get_object_table($obj_type);
  $qs = "SELECT $field FROM $table WHERE $key='$id'";
  return db_getOne($qs);
}

// returns column of field from objects in id_list (array or comma separated);
/** unsused - automatic comment added on  */
function get_objects_field($obj_type, $id_list, $field) {
  if(is_array($id_list)) $id_list = implode(',', $id_list); // handle array
  $key = get_object_key($obj_type);
  $table = get_object_table($obj_type);
  $qs = "SELECT $field FROM $table WHERE $key in ($id_list)";
  return db_getCol($qs);
}

// returns all objects that match filters;
// return assoc array id => descriptor  
function find_objects_assoc($obj_type, $searchArray=array(), $options = array()) {
  return get_objects_assoc($obj_type, search2filters($searchArray), $options);
}

// return assoc array where obj_key is key and descriptor (name) is value
// handy to pass to smarty for select boxes
// filters can be string or array
function get_objects_assoc($obj_type, $filters='', $options = array()) {
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $name = get_object_descriptor($obj_type);
  $options['fields'] = array("$table.$key", $name);
  $first = $options['first']; 
  
  if(is_array($filters) && is_assoc($filters)) $filters = search2filters($filters); 

  $objs = get_objects($obj_type, $filters, $options);

  //dump($objs);
  // can we use a mapping fn ?
  $result = array();
  if($first) $result[''] = $first;
  foreach($objs as $obj) $result[$obj->$key] = get_object_description($obj_type, $obj) ?: "(".$obj->$key.")"; 
  return $result;
}

// returns first object that matches search;
function find_object($obj_type, $search, $field='', $options = array()) {
  $options['field'] = $field;
  $options['limit'] = 1;
  $objs = find_objects($obj_type, $search, $options);
  $result = is_array($objs) ? $objs[0] : $objs; // might return just one value if $field is singular: todo: check for multiple fields
  return $result;
}

// returns all objects that match filters;
//function find_objects($obj_type, $searchArray='', $sort=0, $order=0, $start=0, $limit=0, $selected_fields='') {  
function find_objects($obj_type, $search=array(), $options = array()) {
  $debug = $options['debug_query'];
  if($search && count($search) == 1 && (array_keys($search))[0] == 'id') {
    $tbl = get_object_table($obj_type);
    $search = ["{$tbl}.id" => (array_values($search))[0]]; // we need tablename.id to avoid ambigiuity
  }
  list($filters, $values) = search2pdo($search);
  $options['values'] = $values;
  
  if($options['debug_query'] || $_SESSION['debug_db']) {
    $stack = caller_stack();
    echo("search $obj_type in $stack: ".dump($search,true));
    echo("terms: ".dump($filters,true));
    echo("values: ".dump($values,true));
    
    print_log("search $obj_type in $stack: ".dump($search,true), 'debug', LOG_LEVEL_TEST);
    print_log("terms: ".dump($filters,true), 'debug', LOG_LEVEL_TEST);
    print_log("values: ".dump($values,true), 'debug', LOG_LEVEL_TEST);    
  }
  return get_objects($obj_type, $filters, $options);
}                                      


//function get_objects($obj_type, $filters='', $sort=0, $order=0, $start=0, $limit=0, $selected_fields='') {
// for more than one fields, use array('field1', 'field2')
function get_objects($obj_type, $filters=array(), $options = array()) {
  $sort  = $options['sort'] ?: get_object_sort_clause($obj_type);
  $order = $options['order'] ?: 0;
  $start = $options['start'] ?: $options['offset'];
  $limit = $options['limit'] ?: 0;
  $assoc = $options['assoc'] ?: 0;
  $group = $options['group'] ?: '';
  $values = $options['values'] ?: array();
  
  if(strtolower($order) == 'asc') $order = 0; // allow 0 or 'asc' for ascending
  elseif(strtolower($order) == 'desc') $order = 1; // allow 1 or 'desc' for descending
  
  $selected_fields = $options['fields'] ?: $options['field'];
  $debug = $options['debug_query'];
  
  if(!$obj_type) {dump($filters); mydie("get_objects: missing type.");};
  $query_str = get_object_query($obj_type, '__none', $selected_fields, $options);

  /** apply class_filters() */
  if(!is_array($filters) && strlen($filters)) $filters = [$filters];
  
  // add mandatory class filters
  $class_filters = class_filters($obj_type) ?: [];
  if($class_filters) $filters = array_merge($class_filters, $filters); 
  
  if($filter_str = is_array($filters) ? implode(" AND ", $filters) : $filters) $query_str .= strpos($query_str, ' WHERE ') ? " AND ($filter_str)" : " WHERE $filter_str";
  
  $having = isset($options['having']) ? $options['having'] : ''; // used in where clause for computed columns, e.g "concat(a,b,c) as foo"
  if($having) $query_str  .= " HAVING $having";
  
  $explicit_order = strpos(strtolower($sort), ' desc') || strpos(strtolower($sort), ' asc'); // sort direction already specifiect in $sort var.    

  if($group) $query_str .= " GROUP BY $group";

  $query_str = $query_str.($sort ? " ORDER BY $sort":"").(($sort && $order && !$explicit_order) ? " DESC" : "");
  if($start || $limit) {
    if($start && $limit) { // start and limit
      $query_str = $query_str." LIMIT $start, $limit";
    } elseif($limit) { // limit only
      $query_str = $query_str." LIMIT $limit";
    } else { // start only
      $query_str = $query_str." LIMIT $start, 18446744073709551615";
    }
  }

  
  if($debug) {
    echo($query_str.html_break());
    echo("result=".dump($result, true));
    echo("method=$method<br>");
  }
    
  
  if(!$selected_fields || is_array($selected_fields) /*|| strpos($selected_fields, ',') */) { // 2 or more fields    
    $result = db_getAll($query_str, $values, $options);
    $method = "db_getAll";
  } else {
    $result = db_getCol($query_str, $values, $options); // just one field
    
    $count = count($result);
    if($options['format'] == "list") return implode(",", $result); // comma separated list
    $method = "db_getCol";
  }

  $count = count($result);

  check_db_result($result);
  //echo "<pre>";
    //print_r($query_str);
    //print_r($values);
  
  return $result ?: array();
}

function get_object_query($obj_type, $id='__none', $selected_fields='', $options=array()) {
  if($options && !is_array($options)) die("get_object_query: options not array");

  $ignore_filters = isset($options['ignore_filters']) ? $options['ignore_filters'] : false;
  
  $table = get_object_table($obj_type);
  if(is_object($obj_type)) {
    dump($obj_type);
    mydie("get_object_query obj_type is object");
  }


  $key = $table.".".get_object_key($obj_type);
  $get_relations = $id != '__none';
  $classVars = get_object_attributes($obj_type, 1);
                     
  $fields = array();
  $tables = array();
  $where = array();
  
  $tables[] = $table;

  foreach($classVars as $field=>$class) {
    $att_type = $class[ATT_TYPE];
    $att_flags= $class[ATT_FLAGS];

    if($att_type == 'one2many') continue;
    if($att_type == 'dummy' || $att_flags & AF_DUMMY) continue;
    if(!is_array($class)) continue;
    $elements = explode(".", $field);
    $level = sizeof($elements);
    if($elements[$level-1]=='0') continue;
    if($level>1) {
      $db_field = get_object_table($elements[$level-2]).".".$elements[$level-1];
    } else {
      $db_field = $table.".".$field;
    }

    // special MySQL access functions
    if($att_type == 'ip') {
      $db_field= "INET_NTOA($db_field) AS $field";
    } elseif($att_type == 'crypt' || $att_flags & AF_ENCRYPT) {
      $db_field = "AES_DECRYPT($db_field, '$_SESSION[sqlgarbage]') AS $field";
    }
    
    $fields[] = $db_field;
    // get one2one fields only for individual records - not list
    if($att_type == 'one2one' && (($att_flags & AF_ONE2ONE_INTEGRATE) || ($att_flags & AF_OBLIGATORY))) {
      $att_table = get_object_table($class[ATT_RELATION_TYPE]);
      $att_key = get_object_key($class[ATT_RELATION_TYPE]);
      $tables[$att_table] = $att_table;
      $where[] = "($db_field = $att_table.$att_key)";
    } else { // just show id as link
    }
  }
  
  if($selected_fields) {
    $fields = $selected_fields;  // limit query to one or more fields
  } else {
    //$fields = "*";
  }
  
  $fields_str = is_array($fields) ? implode(', ', $fields) : $fields;   
  $tables = implode(',', $tables);
    
  // dump($id);
  if(is_object($key) || is_object($id)) mydie("key or id is object"); 
  
  if($id != '__none') $where[] = "$key='$id'";
  $where = implode(" AND ", $where);
  if($where) $where = "WHERE $where";

  $qs = "SELECT $fields_str FROM $tables $where";
  
  if($_SESSION['log_db']) {
    print_log(" get: $obj_type", 'set_env', LOG_LEVEL_TEST);
  }
  return $qs;
}


function delete_object($obj_type, $id, $check_relations=true, $options=false) {
  $dryrun = $options['dryrun'];
  
  $errors = array();
  $count = 0; // how many objects were deleted
  $table = get_object_table($obj_type);
  $key = get_object_key($obj_type);
  $obj = get_object($obj_type, $id);

  if(!$id) return array(0, array("delete_object: couldn't delete $obj_type, missing id"));
  if(!$table) return array(0, array("delete_object: couldn't delete $obj_type id $id, no table found for $obj_type."));
  if(!$obj) return array(0, array("delete_object: couldn't delete $obj_type id $id, object not found."));

	list($messages, $errors) = pre_delete($obj_type, $obj);
	if($errors) return array(0, $errors);

	if($check_relations) {
	  print_debug(" ... checking dependents");
	  $fields = get_object_attributes($obj_type, 0);
	  foreach($fields as $field=>$attributes) {
	    $att_type = $attributes[ATT_TYPE];
	    $att_flags = $attributes[ATT_FLAGS];
	    $att_options = $attributes[ATT_RELATION_TYPE];
	    $att_args = $attributes[ATT_ARGS];
	    $att_args2 = $attributes[ATT_ARGS2];

	    if($att_type == 'dummy' || $att_flags & AF_DUMMY) continue;
	    if($att_type == 'one2one' && (($att_flags & AF_CASCADE_DELETE) || ($att_flags & AF_OBLIGATORY))) {
	      $child_table = get_object_table($att_options);
	      $child_id = db_getOne("SELECT $field FROM $table WHERE $key='$id'");
	      if($child_id) {
	        print_debug("type $obj_type has one2one $field of type $att_options, id=$child_id with Cascade Delete - deleting $att_options id=$child_id<br>");
	        list($c_count, $c_errors) = delete_object($att_options, $child_id);
          $count += $c_count;
          $errors = array_merge($errors, $c_errors);
	      } else {
	        print_debug("type $obj_type has one2one $field of type $att_options, id=$child_id with Cascade Delete - ID is NULL, not deleting $att_options id=$child_id<br>");
	      }
	    } else if($att_type == 'one2many' && ($att_flags & AF_CASCADE_DELETE)) {
        $child_type = parse_filter($att_options, $obj);
	      $child_table = get_object_table($child_type);
	      $child_key = get_object_key($child_type);

	      list($parent_type, $parent_key) = get_object_parent($child_type);
        print_debug("type $obj_type has one2many $field of child-type=$child_type child-key=$child_key child_table = $child_table pt=$parent_type pk=$parent_key arg1=$att_args arg2=$att_args2<br>");
        $child_filter = "";
        if(!$parent_key && $att_args) $parent_key = $att_args;
        $child_handlers = has_handler($child_type, 'delete');
        $child_cascade = has_cascade_delete($child_type); // todo: check if children has cascade delete
        if($parent_key && $child_table) {
          if($child_handlers || $child_cascade) {  // this will trigger child handlers, but is slower
            print_debug("Child $child_type has handlers or cascade delete, looping<br>");
            $child_ids = find_objects($child_type, array($parent_key => $id), array('fields' => $child_key));
            foreach($child_ids as $child_id) {
              if(!$dryrun) list($c_count, $c_errors) = delete_object($child_type, $child_id);
              $count += $c_count;
              $errors = array_merge($errors, $c_errors);
              print_debug("Deleted child $child_type $child_id<br>");
            }
          } else {  // this won't trigger handlers, but is faster
            $child_qs = "DELETE FROM $child_table WHERE $parent_key=$id";
            if(!$dryrun) $delete_count = db_query($child_qs);
            //print_debug("deleting kids count=$delete_count: ".$child_qs);
            $count += $delete_count;
          }
        }
	    }
	  }
	} else {
	  print_debug(" ... ignoring dependents");
	}

  if($errors) return array($count, $errors);
  $count++;

  $qs = "DELETE from $table WHERE $key='$id'";
	if(!$dryrun) $result = db_query($qs);
	post_delete($obj_type, $obj);

	// loggin
  $flags = get_object_flags($obj_type);
  if($flags & NF_LOG && $id && !$errors) add_object('changelog', array('obj_type' => $obj_type, 'parent_id' => $id, 'action' => "Delete $obj_type $id"));
  if($flags & NF_CACHE && $id && !$errors) object_cache_delete($obj_type, $id);

  if($_SESSION['log_db']) {
    print_log(" delete: $obj_type $id", 'set_env', LOG_LEVEL_TEST);
  }
  
	return array($count, $errors);
}

// returns first handler of type (add/edit/delete) for give object, or else blank
function has_handler($obj_type, $handler_type='') {
  if($handler_type) $handler_types = is_array($handler_type) ? $handler_type : array($handler_type);
  else $handler_types = array('add', 'edit', 'delete');
  foreach($handler_types as $type) {
    if(function_exists('pre_'.$type.'_'.$obj_type)) return 'pre_'.$type.'_'.$obj_type;
    if(function_exists('post_'.$type.'_'.$obj_type)) return 'post_'.$type.'_'.$obj_type;
  }
  return '';    
}

// returns first field of type one2one or one2many attribue with cascade_delete set (or obligatory if one2one)
function has_cascade_delete($obj_type) {
  $fields = get_object_attributes($obj_type, 0);
  foreach($fields as $field => $attributes) {
    $att_type = $attributes[ATT_TYPE];
    $att_flags = $attributes[ATT_FLAGS];
    if($att_type == 'one2one' && ($att_flags & AF_OBLIGATORY || $att_flags & AF_CASCADE_DELETE)) return $field;
    elseif($att_type == 'one2many' && $att_flags & AF_CASCADE_DELETE) return $field;
  }
  return '';  
}

/** unsused - automatic comment added on  */
function is_object_set($obj_type, $data) {
  $child_fields = get_object_attributes($obj_type, 0);
  foreach($child_fields as $child_field=>$child_class) {
    if(isset($data[$child_field])) return true;
  }
  return false;
}

function object_count($obj_type, $search=NULL, $id=0) {
  return get_objects_count($obj_type, search2filters($search), $id);
}

// same as get_objects_count, but with search array
function count_objects($obj_type, $search=NULL, $id=0) {
  return get_objects_count($obj_type, search2filters($search), $id);
}

function get_objects_count($obj_type, $filters='', $id=0) {
  $query_str = get_object_query($obj_type, 0, "count(*)");
  $key = get_object_key($obj_type);
  $table = get_object_table($obj_type);

  if($id) $filters[] ="$table.$key = '$id'";
  $class_filters = class_filters($obj_type) ?: [];
  if($class_filters) $filters = array_merge($class_filters, $filters); 
  
  if(is_array($filters)) {
    $filter_str = implode(" AND ", $filters);
  } else {
    $filter_str = $filters;
  }
  if($filter_str) {
    if(strpos($query_str, " WHERE ")) {
      $query_str.=" AND $filter_str";
    } else {
      $query_str.=" WHERE $filter_str";
    }
  }
  $count = db_getOne($query_str);
  check_db_result($count);    

  return $count;
}



// takes an object and returns it's description, typically $obj->name
function get_object_description($obj_type, $obj) {
  $descriptor = get_object_descriptor($obj_type);
  $fields = parse_descriptor_string($descriptor);
  foreach($fields as $table_field) {    
    $field = array_pop(explode('.', $table_field)); // use only field name, not table. dot notation
    $description .= $obj->$field." ";
  }
  return trim($description);
}

function parse_descriptor_string($fields) {
  // todo: use reg_ex to allow for characters other than space
  return explode(",", $fields);
}

// takes and array of fields in dot.notation and returns the result
// if id is set, returns only row where key=id

function get_object_value_query($obj_type, $target_fields, $filters="", $sort="", $id=NULL, $extra_fields=NULL) {
  if(!is_array($target_fields)) $target_fields = array($target_fields);
  if($extra_fields && !is_array($extra_fields)) $extra_fields = array($extra_fields);

  //dump($target_fields);
  $table = get_object_table($obj_type);
  $key = $table.".".get_object_key($obj_type);
  $classVars = get_object_attributes($obj_type, 1);
  $fields = array();
  $tables = array();
  $where = array();
  //if($filters) $where = is_array($filters) ? search2filters($filters) : explode(',', $filters); // can be comma separated list or searchArray
  if($filters) $where = is_array($filters) ? search2filters($filters) : array($filters); // can be comma separated list or searchArray
  $tables[] = $table;
  foreach($classVars as $field => $class) {
    $elements = explode(".", $field);
    $level = sizeof($elements);
    if($elements[$level-1]=='0') continue;
    if($level>1) {
      $db_field = get_object_table($elements[$level-2]).".".$elements[$level-1];
    } else {
      $db_field = $table.".".$field;
    }

    $att_type = $class[ATT_TYPE];
    $att_flags= $class[ATT_FLAGS];

    if(in_array($field, $target_fields) || in_array($db_field, $target_fields)) {
      $fields[] = $db_field;
    }
    if($att_type == 'dummy') continue;
    if($att_type == 'one2one' && (($att_flags & AF_ONE2ONE_INTEGRATE) || ($att_flags & AF_OBLIGATORY))) {
    //if($id > -1 && $att_type == 'one2one') {
      $att_table = get_object_table($class[ATT_RELATION_TYPE]);
      $att_key = get_object_key($class[ATT_RELATION_TYPE]);
      $tables[$att_table] = $att_table;
      $where[] = "$db_field = $att_table.$att_key";
    }
  }
  if($extra_fields) $fields = array_merge($fields, $extra_fields);
  $fields = implode(',', $fields);
  if(!sizeof($fields)) {
    $tf = implode(", ", $target_fields);
    die ("get_object_value_query: none of the target_fields $tf where found in the definition of $obj_type");
  }
  $tables = implode(',', $tables);
  if(!is_null($id)) $where[] = "$key='$id'";
  
  if(count($where)) $where_str = "WHERE ".implode(" AND ", $where);
    
  //echo("sort for $obj_type preset to '$sort' - ");
  $sort_clause = $sort ? $sort : get_object_sort_clause($obj_type);
  //echo("now '$sort_clause'<br>");

  $sort = $sort_clause ? "ORDER BY $sort_clause": "";
  //echo("fields='$fields'<br>");
  $qs = $fields ? "SELECT $key, $fields FROM $tables $where_str $sort" : "SELECT $key FROM $tables $where_str $sort";
  //$qs = $fields ? "SELECT $fields FROM $tables $where $sort" : "SELECT $key FROM $tables $where $sort";
  //$qs = "SELECT $fields FROM $tables $where";
  print_debug("get_object: $qs<br>");
  return $qs;
}


// function save_object_query($obj_type, $data, $id=0, $parent_id=0, $preset_id=0, $parent_type='') {
function save_object_query($obj_type, $data, $options = array()) {
  $time_start = microtime(true);
  $debug = $options['debug_query'] ? 1 : 0;
  
  $values = array();
  $errors = array();
  $messages = array();

  if($options && !is_array($options)) {
    $stack = caller_stack();
    print_log("soq: $obj_type in $stack: options not array".dump($options, true), "soq", LOG_LEVEL_ALWAYS);
  }

  
  $id = isset($options['id']) ? $options['id'] : 0; // insert or update
  $preset_id = isset($options['preset_id']) ? $options['preset_id'] : 0; // for inserts: not using db_sequence but preset ID
  //echo "<pre>"; print_r($preset_id);
  $parent_id = $options['parent_id'] ?: 0;
  $parent_type = $options['parent_type'] ?: '';
  $ids = array();
  $id_list = '';
  
  if($debug) print_log("soq: $obj_type id=$id data:".dump($data,true)." options:".dump($options, true), "soq", LOG_LEVEL_ALWAYS);
  
  if($_SESSION['readonly']) return array(0, "System is in readonly mode.");
  if(!$obj_type) {
    if($_SESSION['dev']) {
      mydie("Soq: missing object type");
    } else {
      $errors[] = "Save Object: missing object type";
      return array($id, $errors, $messages);
    }
  }
  
  $orgObj = $id ? get_object($obj_type, $id) : NULL;

  $new = $id ? false : true;

  if(is_object($data)) $data = obj2array($data);

  if(is_array($id)) {
    $ids = $id;
    $id_list = implode(',', $ids);
  } if($id && strpos($id, ',')) {
    $id_list = str_replace(' ', '', $id); // update multiple
    $ids = explode(',', $id_list);
  }

  $fields = get_object_attributes($obj_type, 0);
      
  $obj_keys = array_keys($fields);
  

  // pre-processing: pre_ handlers need proper dates, not "+1 day" etc
  foreach($data as $k => $v) {
    $v = trim($v);
    $class = $fields[$k];
    $att_type = $class[ATT_TYPE];
    $org_val = $orgObj->$k;
    if(in_array($att_type, array('date', 'datetime')) && $org_val && is_date(strip_time($org_val))) {
      if($debug) print_log("Date/time field {$k} org:{$org_val} val:{$v}", 'soq', LOG_LEVEL_TEST);
      if(strlen($v) && !is_date(strip_time($v))) {
        if($debug) print_log("Date/time field not valid, maybe an interval?", 'soq', LOG_LEVEL_TEST);
        if($v = add_interval_string($org_val, $v)) $data[$k] = $v;
      }
    }
    if($att_type == 'json') {
      
      print_log("Before:".$data[$k], 'campaign', LOG_LEVEL_TEST);
      $data[$k] = any2json($data[$k]);
      print_log("After:".$data[$k], 'campaign', LOG_LEVEL_TEST);
    }
  }

  if($debug) print_log("soq: gh1 data:".dump($data,true), "soq", LOG_LEVEL_ALWAYS);
  
	// generic handlers  
  if($new) {
		list($data, $errors) = pre_add($obj_type, $data);
		$handler = "pre_add_{$obj_type}";
	} elseif($id || count($ids)) {
	  //if($orgObj) foreach($orgObj as $f => $v) if(!isset($data[$f])) $data[$f] = $v; // copy from original if not set or handler might get confused	  
		list($data, $errors) = pre_edit($obj_type, $data, $orgObj);
		$handler = "pre_edit_{$obj_type}";
	}
	
	if($errors) return array(0, $errors);
	if(!is_array($data) || !count($data)) return [0, ["{$handler} did not return data"]];


  $data_keys = array_keys($data);
  if($debug) print_log("soq: gh2 keys:".dump($data_keys,true), "soq", LOG_LEVEL_ALWAYS);

  $table = get_object_table($obj_type);
  $flags = get_object_flags($obj_type);
  $obj_name = get_object_name($obj_type);
  $errors = array();
  $children = array();
  $terms = array();
  $flds = array();
  $vals = array();

  if($debug) print_log("soq: gh3", "soq", LOG_LEVEL_ALWAYS);
  
  foreach($fields as $field => $class) {

    /** cannot skip on add, as we need defaults */
    // if(!in_array($field, $data_keys)) continue;

    $escape = false; // by default, enclose values in single quotes
    $enclose = true; // by default, enclose fields in back ticks

    $att_options = $class[ATT_LENGTH];
    $att_default = $class[ATT_DEFAULT];
    $att_name = $class[ATT_NAME];
    $att_type = $class[ATT_TYPE];
    $att_flags = $class[ATT_FLAGS];
    $att_args = $class[ATT_ARGS];
    $skip_validation = false;

    
    if($att_type == 'many2many') continue;
    if($att_type == 'one2many') continue;

    if($att_type == 'dummy' || $att_flags & AF_DUMMY) continue;
    //if(!$id && $att_type == 'one2one') continue; // skip children for new objects, as these have been processed above
    if($debug) print_log("soq: field: {$field}", "soq", LOG_LEVEL_ALWAYS);
    
    if($id) { // this is an update
      if($att_type == 'one2one') { // child add/update
        $child_type = $att_options;
        //if($child_id = $data[$field]) {
        //  //echo("data: child id ($field) of $obj_type $id = $child_id<br>");
        //} else {
        //  $child_id = get_object_field($obj_type, $id, $field); // it might not be passed in data
        //  //echo("obj: child id ($field) of $obj_type $id = $child_id<br>");
        //}
        
        $child_id = $data[$field] ?: get_object_field($obj_type, $id, $field); // it might not be passed in data
        
        
        $child_data = $data; // copy
        $child_data['parent_type'] = $obj_type; 
        $child_data['parent_id'] = $id; // is this always ok ?
        list($new_child_id, $child_errors) = save_object($child_type, $child_data, $child_id); // delegate to child (add or update)
        //echo("Called update child of $obj_type $id: child=$child_type ($child_id) new id=$new_child_id<br>");
        //dump($child_data);
        if($child_errors) $errors = $child_errors;
        $data[$field] = $new_child_id; // a child was added, update parent
      } else if(!in_array($field, $data_keys)) {
        continue; // for updates, we only care about the data actually passed 
      }
    }
    
    $key = get_object_key($obj_type);
    
    // echo("$obj_type: field=$field type=$att_type val=$att_val<br>");

    // for new object, use default if field is not defined
    if($new) {
      $att_val = isset($data[$field]) ? $data[$field] : get_field_default($att_default);
      
    } else {     /** here we can skip fields that are not defined */
      
      if(!isset($data[$field]) && $att_type != 'one2one') continue; // skip field on update if not defined
      $att_val = $data[$field];
    }
    
    //if($obj_type == 'reservation' && $fld == 'apartment_name') echo("$fld = $val");
    //if($obj_type == 'reservation') echo("fld = $field  val = $att_val<br>");

    //if($obj_type == 'cc') echo("att_type = $att_type field=$field key=$key<br>");

    if($field==$key) continue; // do not process id for new objects
    if($field=='0') continue;

    if($parent_id && $att_type == 'parent_id' && !$att_val) $att_val = $parent_id; // set parent to passed value $parent_id
    if($parent_id && $att_type == 'parent_type' && !$att_val) $att_val = $parent_type; // set parent to passed value $parent_id
    if($att_type == 'color') $att_val = ltrim($att_val, '#');
    if($att_type == 'bool' && $att_val === 'on') $att_val = 1; // so if "on", sets to 1, otherwise 0
    if($att_type == 'html' && strlen($att_val)) $att_val = safe_text($att_val); // remove unwanted tags
              
    // special MySQL access functions
    if($att_type == 'ip') {
      $att_val = ip2long($att_val) ?: ''; // "INET_ATON('$att_val')";
      $enclose = false;
    } elseif($att_type == 'password') {
      $att_val = generateHash($att_val);
    } elseif($att_type == 'image') {
      $att_val = str_replace(' ', '+', $att_val); 
    } elseif($att_type == 'datetime' || $att_type == 'date') { // allow for "+1 days"

      /** convert datepickers from human2sql here */
      if(strlen($att_val)) {
        $date_format = $att_type == 'datetime' ? 'Y-m-d H:i:s' : 'Y-m-d';
        $valid = $att_type == 'datetime' && is_datetime($att_val) || $att_type == 'date' && is_date($att_val);
        if(!$valid) $att_val = date($date_format, strtotime($att_val));
      
        $pattern = "/([+-]?\d+) (DAY|HOUR)/"; // integer
        if(preg_match($pattern, $att_val, $m)) {
          $count = $m[1];
          $unit = $m[2];
          $org_val = $orgObj->$field;
          if($org_val) { // use PHP
            $att_val = $data[$field] = date($date_format, strtotime($org_val . " $count $unit")); 
          } else { // use sql
            $att_val = "DATE_ADD($field, INTERVAL $count $unit)";
            $skip_validation = true;
            $escape = false;
            //$enclose = false;
          }
        }
      } else {
        if($debug) print_log("Date/time is set, but blank", 'soq', LOG_LEVEL_TEST);        
      }

    } elseif($att_type == 'time') { // store hh:mm
      $len = strlen($att_val);
        //echo("att_type = time, att_val = $att_val len=$len<br>");
      if($len == 19) $att_val = strip_date_sec($att_val); // convert 0000-00-00 hh:mm:ss to hh:mm
      elseif($len == 16) $att_val = strip_date($att_val); // convert 0000-00-00 hh:mm to hh:mm
      else if(is_time($att_val)) $att_val = strip_sec($att_val); // convert hh:mm:ss to hh:mm
      else if(is_datetime($att_val)) $att_val = strip_date_sec($att_val); // convert valid yyyy-mm-dd hh:mm:ss to hh:mm 
    } elseif($att_type == 'crypt' || $att_flags & AF_ENCRYPT) {
      $att_val = "AES_ENCRYPT('$att_val', '$_SESSION[sqlgarbage]')";
      $escape = false;
      //$enclose = false;
    } 

    // check obligatory - this is first time we use errors     
    
    if($required = $att_flags & AF_OBLIGATORY) {
      // if($att_type == 'datetime') echo("Datetime: $att_name is required, value = '$att_val'<br>");
      // if(in_array($att_type, array()) && ) 

      // booleans: allow 0 or 1
      if($att_type == 'bool') {
        if(!isset($att_val)) $errors[] = "$obj_name: $att_name: ".phrase('required', CAPITALIZE);
      } elseif($att_type == 'one2one') { // supress error (assume message is triggered by other error (which is shown)
      } else { // others: must be value other than '' or 0
        //echo("fld=$field att_val = '$att_val'<br>");
        if(!$att_val) $errors[] = "$obj_name: $att_name: ".phrase('required', CAPITALIZE);
      }
    }
    
    // check uniqueness
    if($att_val && ($att_flags & AF_UNIQUE) && exists($table, $field, $att_val, $key, $id)) {
      $errors[] = "$att_name: '$att_val' already exists and must be unique - please choose another $att_name";
    }

    // other checks
    if(!$skip_validation) {
      if($error = object_attribute_validate($field, $att_type, $att_val, $required)) $errors[] = $error;
    }

    // if value is null, set to NULL, '', or 0 (depending on type and default)
    if($att_val == ATT_DEFAULT_NULL) { 
      if($att_default === ATT_DEFAULT_NULL) {
        $att_val = "NULL";
        $enclose = false; 
      } else {
        $att_val = in_array($att_type, array('text', 'html', 'str')) ? '' : 0;
      }
    }
    
    
    if(is_object($att_val)) {dump($att_val);mydie("$field is object");}
    //print_r($att_val);
    if(is_array($att_val)) {dump($att_val);mydie("{$obj_type}:{$field} is array val=".dump($att_val,true));}
    //if(!is_string($att_val)) {dump($att_val);die("$field is not string att_val='$att_val'");}
    if($att_val=='' && $att_default === ATT_DEFAULT_NULL) { // value is blank and default is null
      if(!$id) {
        continue;  // new - don't add to query
      } else {     // edit - use NULL value
        $att_val = "NULL";
        $enclose = false;      
      }
    }

    $binding = ":".underscore2camel($field); // last to avoid conflicts with reserved variables
    
    if($id) {
      $terms[] = "`$field` = $binding";
    } else {
      $flds[] = '`'.$field.'`';
      $vals[] = $binding;
    }
    $values[] = $att_val;
  }

  if($debug) print_log("soq: gh4", "soq", LOG_LEVEL_ALWAYS);
  
  // class specific error testing (validators)  
  $class_errors = object_validators($obj_type, $data, $id);
  if(count($class_errors)) $errors = array_merge($errors, $class_errors);
  $error_count = sizeof($errors);

  if($debug) print_log("soq error count={$error_count}:".dump($errors,true), "soq", LOG_LEVEL_ALWAYS);
  
  if($error_count) return array(0, $errors, $messages);
  

  
  if($id) {
    $action = 'update';
  } else { // insert query
    $action = 'insert';
    if($preset_id) {
      $id = $preset_id;
    } else {
      $id = nextId($table);
      if(!$id) return array(0, array("No ID sequence for $obj_type and no preset id"));
    }
  }

  $id_term = $id;
  if($id && !count($ids)) {
    $values[] = $id;
    $id_term = ":ID";
  } 
  
  if($action == 'insert') { // insert query

    $flds[] = '`'.$key.'`';
    $vals[] = $id_term;

    $num_fields = count($flds);
    $num_vals   = count($vals);
    if(!$num_fields) {
      $errors[] = "Add Object $obj_type: Invalid query, no fields";
      return array(0, $errors);
    }
    
    $flds_string = implode(",", $flds);
    $vals_string = implode(",", $vals);

    $query_str = "INSERT INTO $table
                  ($flds_string)
                  VALUES ($vals_string)";

  } else { // update query

    if($debug) print_log("soq gh5 terms=".dump($terms,true), "soq", LOG_LEVEL_ALWAYS);
    
    $num_terms = sizeof($terms);
    if(!$num_terms)   return array(0, ["Cannot update, missing terms"]);
    //echo("There are $num_terms terms in query for update $obj_type $id<br>");
    
    $terms_string = implode(",", $terms);
    $query_str = "UPDATE $table
                  SET $terms_string
                  WHERE ".(count($ids) ? "`$key` IN ($id_list)" : "`$key`=$id_term");


  }

  foreach($values as $k => $v) if(!strlen($v)) $values[$k] = ''; // not null
  if($debug) print_log("soq: gh6 qs={$query_str}", "soq", LOG_LEVEL_ALWAYS);

  $res = db_query($query_str, $values, $options);

  if($debug) print_log("soq: gh7 res=".dump($res,true), "soq", LOG_LEVEL_ALWAYS);
  
  if(!$_SESSION['live'] && $options['debug_query']) {
    $affected = $res->affected;
    echo("<b>$action $obj_type $id affected=$affected</b><br>qs=$qs".dump($values, true));
  }
      
  if($error = $res->error) {
    global $u_level;   
    $message = "$obj_name: $error";
    $errors[] = $message;

  } else {
  
    // generic handlers  
    if($new) {
      $errors = post_add($obj_type, $id);
    } else {
      if(count($ids)) {
        foreach($ids as $this_id) {
          if($this_errors = post_edit($obj_type, $this_id, $orgObj)) $errors = array_merge($errors, $this_errors);
        }
      } else {
        $errors = post_edit($obj_type, $id, $orgObj);
      }
    }
  }
  
  if($errors) {
    $action = $id ? 'Update' : 'Add';
    if($_SESSION['dev']) array_unshift($errors, "[dev] $action $obj_type errors:");
  }

  // write to cached
  $classFlags = get_object_flags($obj_type);
  if($classFlags & NF_LOG && $id && !$errors) {
    object_cache_delete($obj_type, $id); // delete cached object
  }

  if($_SESSION['log_db']) {
    print_log(" save: $obj_type $id", 'set_env', LOG_LEVEL_TEST);
  }
  
  if($errors) $id = 0;
  return array($id, $errors, $messages);

}
    
// defaulters run prior to rendering form, so shows up in form
// cannot depend on input values
/** unsused - automatic comment added on  */
function defaulter($obj_type, $data) {
	$fn_name = "defaulter_".$obj_type;
	if(function_exists($fn_name)) return call_user_func($fn_name, $obj_type, $data);
	return $data;
}

// pre add and edit ran after submit, but before DB update
// can depend on input values
function pre_add($obj_type, $data) {
  $errors = array();
	$fn_name = "pre_add_".$obj_type;
	if(function_exists($fn_name)) list($data, $errors) = call_user_func($fn_name, $obj_type, $data);
	
  return array($data, $errors);
}

function pre_edit($obj_type, $data, $orgObj) {
  $errors = array();
	$fn_name = "pre_edit_".$obj_type;
	if(function_exists($fn_name)) list($data, $errors) =  call_user_func($fn_name, $obj_type, $data, $orgObj);
	
  return array($data, $errors);
}

function pre_delete($obj_type, $obj) {
	$fn_name = "pre_delete_".$obj_type;
	if(function_exists($fn_name)) return call_user_func($fn_name, $obj_type, $obj);
  return array(array(), array());
}

function post_add($obj_type, $id) {
	$fn_name = "post_add_".$obj_type;
	if(function_exists($fn_name)) return call_user_func($fn_name, $id);
}

function post_edit($obj_type, $id, $orgObj) {
  
	$obj = get_object($obj_type, $id); // gets newly saved object

  $changes = compare_objects($obj_type, $orgObj, $obj, true, true, true); // compare, return changes, log changes, include  relations  

	$fn_name = "post_edit_".$obj_type;
	$exists = function_exists($fn_name) ? "yes" : "no";

  if(function_exists($fn_name)) return call_user_func($fn_name, $id, $changes);
}

function post_delete($obj_type, $obj) {
	$fn_name = "post_delete_".$obj_type;
	if(function_exists($fn_name)) return call_user_func($fn_name, $obj_type, $obj);
}

// returns array of links or buttons with custom actions for this object in given context (record, recordset, etc)
function class_actions($obj_type, $data, $context='recordset', $ref_name='', $ref_link='') {
	$fn_name = "class_actions_".$obj_type;
	if(function_exists($fn_name)) return call_user_func($fn_name, $obj_type, $data, $context, $ref_name, $ref_link);  
}
					
function object_validators($obj_type, $data, $id=0) {
	$fn_name = "validator_".$obj_type;
	if(function_exists($fn_name)) return call_user_func($fn_name, $obj_type, $data, $id);
}

/** unsused - automatic comment added on  */
function action_button($action, $obj_type, $parent_type, $parent_id) {
	$fn_name = "button_".$action."_".$obj_type;
	if(function_exists($fn_name)) return call_user_func($fn_name, $obj_type, $parent_type, $parent_id);
  return '';
}  



// returns a printable value for a given field of a given object
// can override field type to avoid redunancy
// use in ajax.php
function get_attribute_value_string($obj_type, $obj, $fld, $class, $fld_type='') {
  if(!$fld_type) $fld_type  = $class[ATT_TYPE];
  $att_flags = $class[ATT_FLAGS];
  $rel_type  = $class[ATT_RELATION_TYPE];    
  $fld_val = $obj->$fld;
 //if($fld_type=='valSelect') {dump($rel_type); die();}
  switch($fld_type) {
    case 'dummy':
		  $val_str = get_dummy_value($obj, $rel_type);
      break;
//    case 'color':
//		  $val_str = show_color($fld_val);
//      break;
    case 'many2one':
      if($rel_type && $fld_val) {
        // $descriptor_field = get_object_descriptor($rel_type);
        // $val_str = get_object($rel_type, $val, $descriptor_field);
        $val_str = get_select_value2($rel_type, $fld_val);         
      } else {
        $val_str = $fld_val ? $fld_val : '';
      }
      break;        
    case 'many2many':
      return get_attribute_value_string($obj_type, $obj, $fld, $class, 'many2one');
      break;
    case 'link':
      return get_attribute_value_string($obj_type, $obj, $fld, $class, 'many2one');
      break;
    default:
      $val_str = $fld_val;
      break;
  }
  return $val_str;
}


/** unsused - automatic comment added on  */
function recordset_buttons($obj_type, $row, $parent_id='', $parent_type='', $ref_name='', $ref_link='') {
  $actions = array();
  $key = get_object_key($obj_type);
  $this_name = $obj_type; // todo: fix
  $action = ''; 

  // $actions = default_class_actions($obj_type, $row->$key, $this_name, $action, $parent_id, $parent_type, $ref_name, $ref_link, 'recordset'); // standard buttons or links
  $actions = array(); 
  
  if($custom_actions = class_actions($obj_type, $row, 'recordset', $ref_name, $ref_link)) $actions = array_merge($actions, $custom_actions); // add custom buttons or links  
  return count($actions) ? implode(' ', $actions) : '';
}

// saves last object type/id/action to stac
/** unsused - automatic comment added on  */
function obj_stack_push($obj_type, $id='', $action='') {
  if(!$_SESSION['stack']) $_SESSION['stack'] = array();
  list($lo, $lid, $lac) = $_SESSION['stack'][0];
  if($lo == $obj_type && $lid == $id && $lac == $action) return; // reload or save w/o close
  //echo("'$lo' == '$obj_type' , '$lid' == '$id' , '$lac' == '$action'<br>");
  array_unshift($_SESSION['stack'], array($obj_type, $id, $action)); // add to top of stack
}

// saves last object type/id/action to stac
/** unsused - automatic comment added on  */
function obj_stack_pop() {
  $stack = $_SESSION['stack'];
  if(!is_array($stack)) return;
  return array_shift($_SESSION['stack']);
}

/** unsused - automatic comment added on  */
function back_button($ref_link, $ref_name) {
  if($ref_link) {
    $back_link = $ref_link; $back_name = $ref_name;
  } elseif(count($_SESSION['stack']) > 1) {
    list($lobj, $lid, $laction) = $_SESSION['stack'][1];
		$back_link  = get_object_link($lobj, $lid, $laction)."&amp;__ps";
    $back_name = "$laction ".get_object_name($lobj).($lid=='' ? " list" : " $lid");
  } else { // no referer set, stack is empty, use http referer
		$back_link  = $_SERVER['HTTP_REFERER'];
    $back_name = phrase('previous_page');
		$back_link  = '';
  }		
	return $back_link ? 
	  html_link($back_link, html_i('', 'fa fa-arrow-circle-left'), array('title' => phrase('back_to', CAPITALIZE)." ".$back_name)) : // add back button
    html_i('', 'fa fa-arrow-circle-left'); // disabled button
}

// returns the security level from security
// if NF_NO_SECURITY, check this before calling this function

/** unsused - automatic comment added on  */
function check_security_level($node, $action, $user_level) {
  if(!$user_level) return false; // must be logged in for back in access - 
  $req_level = db_getOne("SELECT user_level FROM security WHERE node='$node' AND action='$action'");
  // echo("req level for $action-$node = $req_level. This user = $user_level<br>");
  return $user_level >= $req_level;
}

/** rewrite - old */
function auth_action($node, $action='', $id=0, $user_level=0, $use_custom_handlers=true) {
  global $u_id;
  global $u_level;
  global $u;
  global $u_property_types;
  global $propertyTypeArray;
  global $eventTypes;
  //if(!$action) $action = 'list';

  $debug = !$_SESSION['live'] && isset($_GET['debug']);
  $debug = true;    
  
  if($eventTypes[$obj_type]) return auth_action('event', $action, $id, $user_level);
  if($action == 'delete') $action = 'del';
  $actions = array('view', 'list', 'add', 'edit', 'del', 'copy');
  
  if(!$action) return ["view" => auth_action($node, 'view'), "edit" => auth_action($node, 'edit'), "delete" => auth_action($node, 'delete')];
  
  if(!in_array($action, $actions)) {
    if($debug) print_log("Security failed for $action $node $id: Action '$action' not in ".implode(",", $actions), 'debug', LOG_LEVEL_TEST);
    return false; // not valid action                                                                                                      
  }
  if(!$user_level) $user_level=$u_level;
                                         
  // check if object allows this action
  $flags = get_object_flags($node);
  if($flags & NF_NO_SECURITY) return true; // check if object allows all actions by all users (no security)
  if($action=='search') $action='list'; // allow search if list is allowed
  if($action=='copy') $action = 'edit'; // same permissions for add and copy for now  
  if($action=='add' && ($flags & NF_NO_ADD))    {if($debug) print_log("Security failed for $action $node $id: Node flags $action<br>".caller_stack(), 'debug', LOG_LEVEL_TEST);return false;}
  if($action=='del' && ($flags & NF_NO_DELETE)) {if($debug) print_log("Security failed for $action $node $id: Node flags $action<br>".caller_stack(), 'debug', LOG_LEVEL_TEST);return false;}
  if($action=='list' && ($flags & NF_NO_LIST))  {if($debug) print_log("Security failed for $action $node $id: Node flags $action<br>".caller_stack(), 'debug', LOG_LEVEL_TEST);return false;}
  if($action=='view' && ($flags & NF_NO_VIEW))  {if($debug) print_log("Security failed for $action $node $id: Node flags $action<br>".caller_stack(), 'debug', LOG_LEVEL_TEST);return false;}
  if($action=='edit' && ($flags & NF_NO_EDIT))  {if($debug) print_log("Security failed for $action $node $id: Node flags $action<br>".caller_stack(), 'debug', LOG_LEVEL_TEST);return false;}
  if($action=='copy' && ($flags & NF_NO_COPY))  {if($debug) print_log("Security failed for $action $node $id: Node flags $action<br>".caller_stack(), 'debug', LOG_LEVEL_TEST);return false;} 

  if($user_level == USER_LEVEL_SYSTEM) return true; // allow all for "The Man"
  if(!$user_level) {if($debug) echo("Security failed for $action $node $id: User level = 0<br>".caller_stack());return false;}
 
  return true;
  
  /** todo: forget security table for now, just use level in class definiton */
  
  /**
  // if this is a property type and this user has property type filters, check if allowed  
//  if($propertyTypeArray[$node] && $u_property_types && !in_array($node, $u_property_types)) return false;
  if($propertyTypeArray[$node] && $u_property_types && !in_array($node, $u_property_types)) {if($debug) print_log("Security failed for $action $node $id: Property type array<br>", 'debug', LOG_LEVEL_TEST);return false;}
    
	// this should be in class_handlers (custom auth)
  if($id && $use_custom_handlers) { // if id of object is given, check if this particular user can $action this object using custom handlers
    $fn_name = 'auth_action_'.$node;
    if(function_exists($fn_name)) {                                                                   
      $result = call_user_func($fn_name, $action, $id, $user_level);
      if(!$result && $debug) echo("Security failed for $action $node $id: Custom handler");
      return $result;
    }
    //if($node=='reservation') return auth_action_reservation($action, $id, $user_level);  
    //if($node=='chat') return auth_action_chat($action, $id, $user_level);
  }
  if(!check_security_level($node, $action, $user_level)) {if($debug) print_log("Security failed for $action $node $id: Security level<br>", 'debug', LOG_LEVEL_TEST);return false;}  // check if action is allowed for this type by this user level (security table)
  
  if($id) { // if id of object is given, check if this particular user can $action this object
    $filters = get_object_filters($node, $user_level, $u_id);
    //dump($filters);
    //if($node == 'site') echo("Security failed for $action $node $id: Action = $action, still ok<br>");dump($filters);
    if(!get_objects_count($node, $filters, $id)) {if($debug) print_log("Security failed for $action $node $id: No objects found for id $id<br> filters:".dump($filters,true), 'debug', LOG_LEVEL_TEST);return false;}
  }
    */
    

  return true;
}

// checks if input is ID, object, or array
// returns object
//function param2object($type, $input) {
//   if(is_object($input)) $obj = $input;                               // $input is object
//   else if(is_numeric($input)) $obj = get_object($type, $input);      // $input is ID
//   else if(is_array($input)) $obj = array2obj($input);                // $input is array
//   else mydie("param2object: invalid input  '$input'");
//   return $obj;
// }

// alias

function any2object($obj_type, $array_obj_or_id) {
  return any2obj($obj_type, $array_obj_or_id);
}

function any2obj($obj_type, $array_obj_or_id) {
  if(is_object($array_obj_or_id)) return $array_obj_or_id;
  if(is_array($array_obj_or_id)) return array2obj($array_obj_or_id); // array is passed
  if($obj_type && $array_obj_or_id) return get_object($obj_type, $array_obj_or_id);
  return NULL;
}

function any2id($obj_type, $array_obj_or_id) {
  if(isint($array_obj_or_id)) return $array_obj_or_id;
  $obj = any2obj($obj_type, $array_obj_or_id);
  $key = get_object_key($obj_type);
  return $obj->$key;
}
  
// sorts array of objects according to specified key

function group_cmp ($a, $b) {
  if($a->group == $b->group) return 0;
  return ($a->group < $b->group) ? -1 : 1;
}

/** unsused - automatic comment added on  */
function group_sort($array) {
  return usort($array, 'group_cmp');
}


// return name of field which refers to the id of the object's parent (typically 'parent_id')
// if parent_type is set, also looks for a link or many2one to this type
/** unsused - automatic comment added on  */
function get_object_parent_key($type, $parent_type='') {
  if(!$type) die ("get_object_key: missing type");
  $attributes = get_object_attributes($type, 0);
  foreach($attributes as $key=>$att) {
    if($att[ATT_TYPE] == 'parent_id') return $key;  // this is primary id
    if($parent_type && ($att[ATT_TYPE] == 'link' || $att[ATT_TYPE] == 'many2one') && $att[ATT_RELATION_TYPE] == $parent_type) return $key;  
  }
  return '';
}

/** no longer in use */
function get_object_parent($type) {
  return array();
}

/** unsused - automatic comment added on  */
function get_select_box($field, $type, $selected_key=0) {
  list($table, $key, $val) = get_select_def($type);
    return select_from_db($field, $table, $selected_key, $key, $val, $first="Select $type");
}


function yes_no_chooser($name, $selected_key=0, $first="", $disabled="", $onchange="") {
  $y = phrase('yes', CAPITALIZE);
  $n = phrase('no', CAPITALIZE);
  $yn = "$y/$n";
  $rows = array('' => $yn, DB_YES => $y, DB_NO => $n);
  return select_from_array($name, $rows, $selected_key, array('first' => $first, 'onchange' => $onchange));
}

function yes_no_required_array() {
  $y = phrase('yes', CAPITALIZE);
  $n = phrase('no', CAPITALIZE);
  $r = phrase('required', CAPITALIZE);
  $ynr = "$y/$n/$r";
  $rows = array('' => $ynr, DB_YES => $y, DB_NO => $n, DB_REQUIRED => $r);
  return $rows;
}

/** unsused - automatic comment added on  */
function yes_no_required_chooser($name, $selected_key=0, $first="", $disabled="", $onchange="") {
  return select_from_array($name, yes_no_required_array(), $selected_key,  array('first' => $first, 'onchange' => $onchange));
}


// returns either drop down select box (form_select) for an object (used for many2one relation)
// or an ajax chooser (object_ajax_chooser) for an object (used for many2one relation) 
// data is additional fields from the object that will be included as data-[field] in the option 
function object_select($name, $obj_type, $options = NULL) {
  global $select_max_options;

  $search = class_filters($obj_type, true) ?: [];
  
  $filters = $options['search'] ?: $options['filters'];
  if(is_array($filters)) { // use search or filters; either way an array is expected
    $search = array_merge($search, $filters);
  }
  //dump($search);
  
  $options['search'] = $search;
  
  $count = 0;
  if(!$options['noajax']) {
    $max_options = pick_first($options['max_options'], $select_max_options, 200); // maximum number of options to show in a drop down menu (otherwise show AJAX chooser)
    $count = count_objects($obj_type, $search);
    check_db_result($count); 
    if($count > $max_options) return object_ajax_chooser($name, $obj_type, $options);
  }
  
  // not ajax chooser, use form_select
  $sort = $options['sort'] ?: get_object_sort($obj_type);
  $data = $options['data'];
  if($data && !is_array($data)) $data = array($data);
  $options['rows'] = get_object_descriptor_list($obj_type, $search, $sort, $data);
  $options['first'] = $options['first'] ?: "-- ".phrase('select', LOWERCASE)." --";

  return form_select($name, $options);    
}

function object_ajax_chooser($name, $obj_type, $options=NULL) {
  global $__APP;
  $__APP->header->scripts[]= '/afr/js/afr_suggest.js';

  if($filters = $options['filters']) {
    $options['search'] = $filters;
    unset($options['filters']);
  } else {
    $search  = $options['search'];
  }
  if($value = $options['value']) $dv = $options['display_value'] = get_select_value2($obj_type, $value);
  
  $search_options['obj_type'] = $obj_type;
  $search_options['search'] = $search;
  $search_options['id_field'] = $options['id_field'] ?: get_object_table($obj_type).".".get_object_key($obj_type);
  $search_options['value_field'] = $options['value_field'] ?: get_object_descriptor($obj_type);
  $search_options['search_field'] = $options['search_field'] ?: $search_options['value_field'];
  foreach($search_options as $k => $v) unset($options[$k]);
  
  $options['debug'] = true;
  //dump($search_options);  
  return form_autocomplete($name, $search_options, $options);  
}

function get_object_descriptor_list($obj_type, $filters="", $sort="", $extra_fields="") {
  $descriptor = get_object_descriptor($obj_type);
  //dump($filters);
  return get_object_value_list($obj_type, $descriptor, $filters, $sort, $extra_fields);
}

// object descriptor is the name as it will appear in a select box
function get_object_descriptor($type) {
  if(!$type) mydie ("get_object_descriptor: missing type");
  global $__CLASSES;
  $class = $__CLASSES[$type];
  if($descriptor = $class[CLASS_DESCRIPTOR]) return $descriptor; // explicit descriptor
  $class_def = get_object_attributes($type, false); // try 'name'
  if($class_def['name']) return 'name';
  return get_object_key($type);                     // return key
}

function get_object_value_list($obj_type, $desc_fields, $filters="", $sort="", $extra_fields="") {
  if(!$obj_type) die ("get_object_value_list: missing obj_type or desc_fields");
  if(!$desc_fields) die ("get_object_value_list: missing desc_fields");
  $target_fields = parse_descriptor_string($desc_fields);
  $query = get_object_value_query($obj_type, $target_fields, $filters, $sort, NULL, $extra_fields);
  $rows = db_getAll($query, array(), array('fetchmode' => 'ordered'));
  return $rows;
}

function get_object_value($obj_type, $desc_fields, $id) {
  if(!$obj_type) mydie ("get_object_value: missing obj_type or desc_fields");
  if(!$desc_fields) mydie ("get_object_value: missing desc_fields");
  if($id == '') mydie ("get_object_value: missing id");
  
  $target_fields = explode(',', $desc_fields);  
  $query = get_object_value_query($obj_type, $target_fields, "", "", $id);
  $count = count($target_fields);
  $row = $count>0 ? db_getRow($query, array(), array('fetchmode' => 'ordered')) : db_getOne($query);
  return $row;
}

function get_object_descriptor_string($obj_type, $id) {
  $descriptor = get_object_descriptor($obj_type);
  $value = get_object_value($obj_type, $descriptor, $id);
  return $value;
}

/** unsused - automatic comment added on  */
function get_select_value($type, $selected_key) {
  list($table, $key, $descriptor) = get_select_def($type);
  return db_getOne("SELECT $descriptor FROM $table WHERE $key='$selected_key'");
}

function get_select_value2($obj_type, $selected_key) {
  if(!$selected_key) return "";
  $row = get_object_descriptor_string($obj_type, $selected_key);
  if(count($row) > 1) array_shift($row); // don't want the ID

  $result = is_array($row) ? implode(" ", $row) : $row;
  if(!$_SESSION['live'] && $result && !is_array($row)) echo("get_select_value2($obj_type key=$selected_key): Row is not array:".dump($row, true));
  
  $result = trim($result);
  if(!strlen($result)) return $selected_key;
  return $result;
}

function get_select_def($type) {

  $table = get_object_table($type);
  $key = get_object_key($type);
  $descriptor = get_object_descriptor($type);
  return array($table, $key, $descriptor);
}

function select_from_db($name, $table, $selected_key=0, $key='id', $val='name', $first="") {
  if(!$first) $first = phrase('please select...', CAPITALIZE);
  $rows = db_getAll("SELECT $key, $val from $table");
  $select_box .= "<select name='$name'>";
  if($first) $select_box .= "<option value='0'>$first</option>";
  foreach($rows as $row) {
    $id = $row->$key;
    //$value = $row->$val;
    $value = phrase($row->$val, CAPITALIZE);
    $selected = $selected_key == $id || $_GET[$name] == $id || $_POST[$name] == $id ? "selected='selected'" : "";
    $select_box .= "<option value='$id' $selected>$value</option>";
  }
  $select_box .= "</select>";
  return $select_box;
}

function get_class_header($class) {
  global $__CLASSES;
  return $__CLASSES[$class];
}

/** returns class definition */
function get_class_def($type, $get_relations=0) {

  $class->type = $type;
  $class->name = get_object_name($type);
  $class->user_level = get_object_user_level($type);
  $class->group = get_object_group($type);
  $class->table = get_object_table($type);
  $class->key = get_object_key($type);

  $class->fields = get_object_attributes($type, $get_relations);
  $class->descriptors = get_object_descriptor($type);
  $class->flags = get_object_flags($type);
  return $class;
}

// returns link to object of given type/id
// if id is 0/null/'', returns link to list
// if admin is set, it returns admin link (app.php)
// otherwise, it returns public link (if exists)

function get_object_link($type='', $id=0, $action='', $parent_id='', $parent_type='', $section='',$ref_name='', $ref_link='') {
  $host = $_SERVER['HTTP_HOST'];
  $app_link = get_page_link(PAGE_ADMIN);
	$link = "http://$host".$app_link;
	
	if(!$type) return $link;
  $fn_name = 'object_'.$action.'_link_'.$type;
  if(function_exists($fn_name)) {
    // echo("<br>$fn_name exists, calling $fn_name($id)<br>");
    $link = call_user_func($fn_name, $id); // custom link
  } else {	// generic link
  	$link .= "?obj_type=$type";
  	if($id) $link .= "&amp;id=$id";
  	if($action && $action!='view') $link .= "&amp;ac=$action";
  	if($parent_id && $parent_type) { // todo: doesn't work if id is zero
  		if(!$parent_type) echo("get_object_link: type=$type id=$id action=$action pid='$parent_id' pt='$parent_type'; parent_id set but not type<br>");
  		$link .= "&amp;__parent_id=$parent_id";
  		$link .= "&amp;__parent_type=$parent_type";
  		$last = "&amp;sc=$type#$type"; // hash must come last
  	} elseif($section) {
  		$link .= "&amp;sc=$section#$section"; // expand child recordset and go there
  	}
  }
	if($ref_name && $ref_link) $link .= "&amp;ref_name=$ref_name&amp;ref_link=$ref_link";
	  
	return $link.$last;
}

function get_field_default($fld_default) {
  global $u_id;
  $userObj = $u_id ? get_user($u_id) : null;
  // special handler for default values
  switch($fld_default) {
    case ATT_DEFAULT_NULL:        return NULL; break;
    case ATT_DEFAULT_ZERO:        return 0; break;
    case ATT_DEFAULT_BLANK:       return ''; break;
    case ATT_DEFAULT_NOW:         return date("Y-m-d H:i:s"); break;
    case ATT_DEFAULT_NOW_SHORT:   return date("YmdHis"); break;
    case ATT_DEFAULT_TODAY:       return date("Y-m-d"); break;
    case ATT_DEFAULT_SESS_ID:     return session_id(); break;
    case ATT_DEFAULT_CLIENT_IP:   return $_SESSION['user_ip']; break;
    case ATT_DEFAULT_USER_AGENT:   return $_SESSION['user_agent']; break;
    case ATT_DEFAULT_CLIENT_CC:   return $_SESSION['user_cc']; break;
    case ATT_DEFAULT_CLIENT_CITY:   return $_SESSION['user_city']; break;
    case ATT_DEFAULT_CLIENT_CONTINENT:   return $_SESSION['user_continent']; break;
    case ATT_DEFAULT_CLIENT_LOCATION:   return $_SESSION['user_location']; break;
    case ATT_DEFAULT_LANG:        return $_SESSION['language'];break;
    case ATT_DEFAULT_LANGUAGE:     return $_SESSION['language']; break;
    case ATT_DEFAULT_COUNTRY:     return $_SESSION['country_code']; break;
    case ATT_DEFAULT_GEO_ID:      return $_SESSION['geo_id']; break;
    case ATT_DEFAULT_CITY_ID:     return $_SESSION['city_id']; break;
    case ATT_DEFAULT_CITY_NAME:   return $_SESSION['city_name']; break;
    case ATT_DEFAULT_USER_ID:     return $_SESSION['user_id']; break;
    case ATT_DEFAULT_USERNAME:    return $_SESSION['username']; break;
    case ATT_DEFAULT_USER_NAME:    return $_SESSION['user_name']; break;
    case ATT_DEFAULT_PROVIDER_ID:  return $_SESSION['provider_id']; break;
    case ATT_DEFAULT_PROVIDER_NAME:  return $_SESSION['provider_name']; break;
    case ATT_DEFAULT_DISPLAY_NAME: return $_SESSION['display_name']; break;
    case ATT_DEFAULT_USER_LEVEL:  return $_SESSION['user_level'] ? $_SESSION['user_level'] : $_SESSION['anon_user_level']; break;
    case ATT_DEFAULT_GUEST_ID:    return $_SESSION['guest_id']; break;
    case ATT_DEFAULT_ORG_ID:      return $_SESSION['org_id']; break;
    case ATT_DEFAULT_HOST_ID:      return $_SESSION['host_id']; break;
    case ATT_DEFAULT_STAFF_ID:      return $_SESSION['staff_id']; break;
    case ATT_DEFAULT_ADMIN_ID:      return $_SESSION['admin_id']; break;
    case ATT_DEFAULT_AGENT_ID:      return $_SESSION['agent_id']; break;
    case ATT_DEFAULT_AFF_ID:      return $_SESSION['aff_id']; break;
    case ATT_DEFAULT_PAGE_ID:     return $_SESSION['page_id']; break;
    case ATT_DEFAULT_SITE_ID:     return $_SESSION['site_id']; break;
    case ATT_DEFAULT_PROPERTY_ID: return $_SESSION['property_id']; break;
    case ATT_DEFAULT_PROPERTY_TYPE: return $_SESSION['property_type']; break;
    case ATT_DEFAULT_CLIENT_ID:     return $_SESSION['client_id']; break;
    case ATT_DEFAULT_LEGAL_ID:     return $_SESSION['legal_entity_id']; break;
    case ATT_DEFAULT_SERVER:        return SYSTEM_SERVER_ID; break;
    case ATT_DEFAULT_TIMEZONE:      return $userObj->timezone ?: SYSTEM_TIMEZONE; break;
    case ATT_DEFAULT_UUID:          return unique_id(); break;
    default: return $fld_default;
  }  
}

function get_dummy_value($row, $fld_options) {
	$display_val_array = array();
	$dummy_fields = explode(' ', $fld_options);
	foreach($dummy_fields as $dummy_field) {
		$display_val_array[]= $row->$dummy_field;
	}
	$display_val = implode(' ', $display_val_array);
	//dump($row); echo($fld_options);echo("res=$display_val");
	return $display_val;
}	



// for one record (class + id)
// returns assoc. array of Name + Field Value (input, search, or print, depending on action)
// optionally, pass in associate array $fields where key=field name, optional value=attribute overrides
function record_fields($class_def, $row, $this_id, $action, $parent_id=0, $parent_type='', $messages='', $fields=NULL) {
  global $xhtml;
  global $u_id;
  
  $st = $xhtml ? "/" : '';
  $recordArray = array();
  $edit = ($action=='edit' || $action=='search' || $action=='add' || $action=='copy') ? true : false;
  $search= $action=='search';
  if(is_array($row)) $row = array2obj($row);

  $this_type  = $class_def->name;
  $classVars  = $class_def->fields;
  $obj_type   = $class_def->type;
  $classTable = $class_def->table;
  $key        = $class_def->key;
  $id = $row->$key;
  
	$hidden_fields = "";
  if(count($fields)) { // pick out the fields we care about in the same order as the $fields argument
    foreach($fields as $field => $overrides) {
      $newClassVars[$field] = $classVars[$field];
      if(is_array($overrides)) {
        foreach($overrides as $k=>$v) $newClassVars[$field][$k] = $v; // overwrite
      }        
    }
    $classVars = $newClassVars;
  }
  
  
  foreach($classVars as $fld =>$class) {
    if($action == 'copy' && $fld == $key) continue; // copy is basically same as edit, but w/o the key, results in add with prefilled data

  	$record_str = "";
    $fld_default = 0;

    $level=0; $prefix='';
    if($pos=strrpos($fld, '.')) {
      $full_fld = str_replace('.', '-', $fld); // periods (.) don't work in POST, so use hat (-)
      $level = substr_count($fld, '.');
      $fld = substr($fld, $pos+1);
    } else {
      $full_fld = $classTable."-".$fld;
    }

    $v = $row->$fld;
    $fld_val = $val = trim($v);
       
    $fld_name = $class[ATT_NAME];
    $fld_default = $class[ATT_DEFAULT];
    $fld_flags= $att_flags = $class[ATT_FLAGS];
    $fld_type = $class[ATT_TYPE];
    $fld_options = $fld_len = $class[ATT_LENGTH];
    $fld_args = $class[ATT_ARGS];
    $fld_sort =  $class[ATT_SORT];
    $fld_args2 = $class[ATT_ARGS2];

    $fld_id_js = "id='$full_fld'"; // used by form_input 

    $select_options = [];
    if($parent_id && $fld_type == 'parent_id' && !$att_val) {
      $fld_default = $parent_id; // set default to passed value $parent_id
    } elseif($parent_id && $fld_type == 'parent_type' && !$att_val) {
      $fld_default = $parent_type; // set default to passed value $parent_type
    } elseif(!$att_val && $passed_default = pick_first($_GET['__default_'.$fld], $_POST['__default_'.$fld])) {
      $fld_default = $passed_default;
    } else {
      $fld_default = get_field_default($fld_default); // get default value
    }
    

    if($edit && $fld_type=='one2many' && $att_flags & AF_READONLY_EDIT) continue; // don't show lists in edit mode if readonly flag is set

  	
  	// locked, readonly, hide 
    $locked = $att_flags & AF_LOCKED ? "readonly = 'readonly'" : "";
    $readonly = $att_flags & AF_READONLY
            || ($action=='edit' && $att_flags & AF_PRIMARY)
            || ($action=='edit' && $att_flags & AF_READONLY_EDIT);
    $hide = $att_flags & AF_HIDE
            || ($action=='add' && $att_flags & AF_PRIMARY)
            || $fld_type=='one2one'
            || ($action=='edit' && $att_flags & AF_HIDE_EDIT)
            || ($action=='view' && $att_flags & AF_HIDE_VIEW)
            || ($action=='add' && $att_flags & AF_HIDE_ADD);
    
    if(!is_array($class)) {
      $i++;
      continue; // subheading
    }

    if(!$fld_name) $fld_name = phrase($fld);
    $fld_name = $prefix.$fld_name;
    		  
    $fld_edit=$edit;

    $search_len = 30;

		// value to display 
    if($fld_type=='date' || $fld_type=='datetime') {							
      $display_val = sql2human($fld_val);
      $time = substr($fld_val, 11);
      if($time != "00:00:00") $display_val .= " $time";
		} elseif($fld_type == 'one2one' || $fld_type == 'many2one') {
			$display_val = get_select_value2($fld_options, $val);
		} elseif($fld_type == 'dummy') {
			$display_val = get_dummy_value($row, $fld_options);
		} elseif($fld_type == 'password') {
			$display_val = "*****";
		} elseif($fld_type == 'color' && is_color($val)) {
		  $display_val = show_color($val);
		} elseif($fld_type == 'crypt') {
		  global $crypt_file;
		  $pwd = read_file($crypt_file);
		} elseif($fld_type == 'cc') {
		  global $crypt_file;
		  $pwd = read_file($crypt_file);

			$display_val = $val ? "xxxx-xxxx-xxxx-".substr($val,12) : "";
			if($val) {
				include_once("cc_check.inc");
				$cardname = get_select_value2('cc_type', $row->cc_type);
				$valid = checkCreditCard($val, $cardname, $errornumber,$errortext);				
				$display_val .=	$valid ? " &#10004; " : " &#10008; ";
			}
		} else {
			$display_val = $val;
		}
			
    if($fld_edit) { // edit or add
			$max_input_len = 40;

			// blank and adding - try to set default value
      if($action=='add' && !$val) { 
      	if($fld_type=='parent' && $parent_id) { // use parent id
      		$val = $parent_id;
      	} else {
	      	$val = $fld_default ? $fld_default : ""; // use default value when adding
	      	$display_val = $val;

	      	// special handler for default values
	      	$user_type_array = $_SESSION['user_type_array'];
	      	if($fld=='user_level' && ($ul_def = $user_type_array[ucfirst($obj_type)])) {
	      		$val = $ul_def;	
	      		$display_val = 	ucfirst($obj_type);
	      	}
	      }
	    }

      if($fld_type=='one2many') {        
        if($action == 'add') continue; // don't show child record sets in add mode
        $child_type = parse_filter($fld_options, $row);
        $childRows = get_object_children($child_type, $row, $this_id, $fld_options, $fld_args, $fld_args2);
        foreach($childRows as $childObj) {
          $child_key = get_object_key($child_type);
          $child_id = $childObj->$child_key;
          $record_str .= edit_object_inline($child_type, $child_id, $child_fields, $options);
        }
        
      } elseif($hide) {
        $record_str.= "<input name='$fld' id='$full_fld' type='hidden' value='$val'$st>";
      } elseif($readonly) {
        $record_str.= "<input name='$fld' id='$full_fld' type='hidden' value='$val'$st>$display_val";
      } elseif($fld_type=='many2one' || $fld_type=='many2many') {
        $select_options['value']   = $val;
        // $select_options['filters'] = $fld_args;
        if($filters = $fld_args) {
          foreach($filters as $k => $v) {
            $filters[$k] = parse_filter($v, $row);
          }
          $select_options['filters'] = $filters;
          //die("filters:".dump($filters,true));
        }
                
        $select_options['sort']    = $fld_sort;
        $select_options['id']      = $full_fld;
        if($att_flags & AF_FORCE_SELECT) $select_options['noajax'] = true; 

        $record_str.= $att_flags & AF_LARGE ? 
          object_ajax_chooser($fld, $fld_options, $select_options) :
          object_select($fld, $fld_options, $select_options);
          
        if($att_flags & AF_INLINE_ADD) {
          $title = 'New '.get_object_name($fld_options);
          $js = "onclick='new_object_dialog(\"$fld\", \"$fld_options\", \"$title\"); return false;'";
          
          $record_str .= ' '.html_link('/link/to/new/dialog/if/js/is/off/', phrase('new', CAPITALIZE).' '.get_object_name($fld_options), array('full' => true, 'title' => $title, 'js' => $js, 'class' => 'arrow_link'));
        }
      } elseif($fld_type=='intSelect') {
        $min = $fld_options[0];
        $max = $fld_options[1];
        $step = $fld_options[2];
        $postfix = $fld_options[3];

        $record_str.=  select_from_array($fld, num_array($min, $max, $step, $postfix), $val);
		  } elseif($fld_type == 'dummy') {
		  	continue;

		  } elseif($fld_type == 'media' || $fld_type == 'image') {
		    if(!$id) continue; // only allow for existing objects
		    $record_str = object_media_uploader($obj_type, $id, $fld);
		  } elseif($fld_type == 'file') {
		    if($obj_type == 'media') { // special case, get options from parent
		      if(!$id) continue; // only allow for existing objects
          $mediaObj = $row;
          if($mediaObj && is_object($mediaObj)) {
            $parent_id = $mediaObj->parent_id;
            $parent_type = $mediaObj->parent_type;		      
            $parent_field = $mediaObj->parent_field;
            if($parent_type && $parent_id && $parent_field) {
              if($parentObj = get_object($parent_type, $parent_id)) {
                $val = $mediaObj->id;
                $attribute = get_object_attribute($parent_type, $parent_field);
                $upload_options = $attribute[ATT_LENGTH];
                if($upload_options && is_array($upload_options)) {
                  $upload_options['allow_multiple'] = 0;
                  $mediaData = (array) $mediaObj; // copy existing
                  $upload_options['media_data'] = $mediaData;
                  $upload_options['data-value_list'] = $mediaObj->filename;
                  $upload_options['show_file_list'] = true; 
                }
              }
            }
          }
		      
		    } else {
		      $upload_options = $fld_options;
		    }
  		  
  		  if(!$upload_options) $record_str .= "Could not find upload options";
  		  elseif(!is_array($upload_options)) $record_str .= "Upload options not array";
  		  else $record_str .= jquery_file_uploader("$obj_type-$id-$fld", $val, $upload_options);
  		} elseif($fld_type == 'color') {
  		   $record_str.= show_color($val, $fld);

      } elseif($fld_type=='location_picker') { // widget, no data
        $max = 1; // default len is 20
        $len = 1;
        $lp_fld = "$classTable-locationsearch";
        $search_button = location_search($lp_fld);
        $record_str .= " <input type='text' class='' name='$lp_fld' id='$lp_fld' value='' size='20'  $st>";
        $record_str .= "<div id='searchContainer'>
                          <div id='suggestContainer' class='float_left suggestion_container'></div>
                          <div id='searchMap' class='float_right suggestion_searchmap' ></div>
                          <div class='float_clear'></div>
                        </div>";
      } elseif($fld_type=='map') {
        $max = 2; // default len is 20
        $len = 1;
        $record_str .= "<input type='text' name='$fld' id='$full_fld' value='$val' maxlength='$max' size='$len' $st>";
        $record_str .= " <input type='hidden' name='$classTable-viewport' id='$classTable-viewport' value='$val' size='50' $st>";
        $map_button = location_picker($full_fld, $classTable, $this_id, $row, true);
      } elseif($fld_type=='valSelect') {
        $record_str.= select_from_array($fld, $fld_options, $val);
      } elseif($fld_type=='ssel') {
        $qs = "SELECT id,name FROM $fld_options";
        if($fld_args) $qs.=" WHERE ".implode(' AND ', search2filters($fld_args));
        $record_str.=  select_from_array($fld, db_getAll($qs, array(), array('fetchmode' => 'ordered')), $val);
      } elseif($fld_type=='flags') { // array of name => value (int): values are logically or'ed into one value if checked
        $msel_str = flags_select($fld, $fld_options, $val, 10);          
        $record_str .= $msel_str; 
      } elseif($fld_type=='msel') { // comman separated list of ids into given table
        $child_type = $fld_options;
        $table      = get_object_table($child_type);
        $key        = get_object_key($child_type);        
        $descriptor = get_object_descriptor($child_type);        
        $qs = "SELECT $key, $descriptor FROM $table";
        if($fld_args) $qs.=" WHERE ".implode(' AND ', search2filters($fld_args));
        $objs = db_getAll($qs);
        $record_str .= checkbox_select($fld, $objs, array('value' => $val, 'cols' => 4, 'key' => $key, 'descriptor' => $descriptor));          

      } elseif($fld_type=='float') {
        $format = explode('.', $fld_options);
        $mag = $format[0];
        $frac = $format[1];
        $max = $mag + $frac + 1;
        if($max<3) $max=3;
        $len = $search ? $search_len : $max+2;
        $record_str.= "<input type='text' name='$fld' id='$full_fld' value='$val' maxlength='$max' size='$len' $locked $disabled $st>";
      } elseif($fld_type=='yesno') {
        $record_str.= yes_no_chooser($fld, $val);
      } elseif($fld_type=='bool') {
        $checked = $val || ($action == 'add' && $fld_default) ? "checked='checked'" : "";
        $record_str.= "<input name='$fld' type='hidden' value='0' $st><input name='$fld' id='$full_fld'  value='1' type='checkbox' $checked $locked $disabled $st>";

      } elseif($fld_type=='html') {
        $fck_h = $fld_len ? $fld_len * 30 : 200;
        $fck_w = 800;        
        $record_str .= form_text($fld, $val, array("class" => "summernote form-control special", "data-height" => $fck_h, "data-full" => 1));
                
      } elseif($fld_type=='text') {
        $len = $search ? $search_len : $fld_options;
        if($search) {
          $record_str.= "<input type='text' name='$fld' id='$full_fld' value='$val' maxlength='$max' size='$len' $locked $disabled $st>";
        } else {
          $text_options = array();
          $ta_cols = $_SESSION['inner_width'] ? round($_SESSION['inner_width'] / 12) : 60;
          if(is_array($fld_args)) {
            $text_options = $fld_args;
          } else if(isint($fld_args)) {
            $text_options['max'] = $fld_args;
          }           
          if(!$text_options['cols']) $text_options['cols'] = $ta_cols;
          if(!$text_options['rows']) $text_options['rows'] = 2;

          $record_str.= form_text($fld, $val, $text_options);          
          //$record_str.= form_text($fld, $val, $ta_rows, $ta_cols, "$locked $disabled ".$fld_id_js, $fld_len);          
        }

      } elseif($fld_type=='link') {
        $child_type = $fld_options;
        $target_description = $fld_val ? get_select_value2($child_type, $val) : "";                
        $name = $target_description ? $target_description : $val;
        $record_str.= $val ? html_link(get_object_link($child_type, $fld_val, 'add', $this_id, $obj_type), $name, array('title' => $target_description)) : $val;
        $record_str.= "<input type='hidden' name='$fld' id='$full_fld' value='$val' maxlength='$max' size='$len' $locked $disabled $st>";
	    } elseif($fld_type=='date' || $fld_type=='datetime') {
        
        $date = is_datetime($val) || is_date($val) ? date("d-m-Y", strtotime($val)) : '';
        $record_str.= form_input($fld, "text", $date, array('class' => 'datepicker', 'data-date' => $val));
        
	    } elseif($fld_type=='time') {
				$record_str.= timepicker($fld, array(), $fld_val);
      } else {
        $input_type = $fld_type == 'password' ? 'password' : 'text';      
        $max = $fld_len ? $fld_len : 20; // default len is 20
        $len = $search ? $search_len : max(round($max/3), 10);
        if($len > $max_input_len) $len = $max_input_len;
        $record_str.= "<input type='$input_type' name='$fld' id='$full_fld' value='$val' maxlength='$max' size='$len' $locked $disabled $st>";
	    }
			

    } else { // view
      if(($att_flags & AF_HIDE) || ($att_flags & AF_HIDE_VIEW)) continue; // this field is hidden or hidden in detail view
      if($fld_type=='one2one') continue; // don't show ID of children
      if($fld_type=='many2one' || $fld_type=='many2many') {
        $child_type = $fld_options;
        $target_description = $fld_val ? get_select_value2($fld_options, $fld_val) : "";         
        $descriptor = get_select_value2($fld_options, $fld_val);
        // echo("auth_action($fld_options, 'view', $fld_val) = ".auth_action($fld_options, 'view', $fld_val));
        $record_val = $fld_val && auth_action($fld_options, 'view', $fld_val) ? 
          html_link(get_object_link($child_type, $fld_val, 'view'), $descriptor, array('class' => 'view-object', 'data-obj_type' => $child_type, 'data-obj_id' => $fld_val, 'title' => $target_description)) 
          : $descriptor;
      } elseif($fld_type=='yesno') {
        $record_val = strlen($val) ? ($val ? "Yes" : "No") : '?';
      } elseif($fld_type=='bool') {
        $record_val = $val ? "Yes" : "No";
		  } elseif($fld_type == 'mail_subject') {
				$record_val = parse_mail_subject($row->headers);
      } elseif($fld_type=='valSelect') {
        $record_val = $fld_options[$val];
		  } elseif($fld_type == 'mail_body') {
				$record_val = parse_mail_body($val);
		  } elseif($fld_type == 'dummy') {
				$record_val = get_dummy_value($row, $fld_options);
      } elseif($fld_type=='file') {
        $record_val = file_list($val, $fld);
      } elseif($fld_type=='link') {
        $child_type = $fld_options;
        $target_description = $fld_val ? get_select_value2($child_type, $fld_val) : "";         
        $name = $target_description ? $target_description : $fld_val;
        
        $record_val = $fld_val && auth_action($child_type, 'view', $fld_val) ? 
          html_link(get_object_link($child_type, $fld_val, 'view'), $name, array('title' => $target_description)) 
          : $name;
  
      } elseif($fld_type=='url') {
        $link = $fld_options ? $fld_options.$fld_val : $fld_val;
        $record_val = "<a href='$link'>$fld_val</a>";
      } elseif($fld_type=='ssel') {
        $record_val = db_id2name(get_object_table($fld_options), $fld_val, true, false, false, get_object_descriptor($fld_options), get_object_key($fld_options));

      } elseif($fld_type=='msel') { // comman separated list of ids into given table
        $record_val = db_id2name(get_object_table($fld_options), $fld_val, true, true, false, get_object_descriptor($fld_options), get_object_key($fld_options));

      } elseif($fld_type=='one2many') {
        $child_type = parse_filter($fld_options, $row);
        $childRows = get_object_children($child_type, $row, $this_id, $fld_options, $fld_args, $fld_args2);
        $child_count = sizeof($childRows);
        if(1 || $child_count) {
          $child_recordset = recordset($child_type, $childRows, $child_count, $this_id, $obj_type, $action, $messages[$fld_options]);
          $show_child = get_or_post('sc') == $child_type;
          $record_val = show_hide_div($child_recordset, phrase('show', CAPITALIZE). " $fld_name", $show_child);
        } else {
          if(auth_action($fld_options, 'add')) {
  				  $add_url = get_object_link($child_type, '', 'add', $this_id, $obj_type);
            $add_text = phrase('add', CAPITALIZE)." $fld_name";
        	  $add_link = html_link($add_url, $add_text);
            $record_val = $add_link;
          } else {
            $record_val = "No $fld_name records";
          }
        }

      } else {
        $record_val = $display_val;
      }
      if(!strlen($record_val)) $record_val = '&nbsp;';
      $record_str .= html_span($record_val, 'margin-left');
    }
    if($hide) {
      $hidden_fields .= $record_str; // add to hidden string
    } else {
      $recordArray[$fld_name] = $record_str;  // add to output array
      $lastVisible = $fld_name;
    }
  }
  // add hidden fields to last record
  $recordArray[$lastVisible] .= $hidden_fields;
  return $recordArray;
  //return $record_fields;
}


// generic validation of type / value
function object_attribute_validate($field, $att_type, $att_val, $required=false) {    
  // attribute specific error testing
  if($required && !trim($att_val)) return "{$field}: required";
  
  if($att_val && $att_val !== ATT_DEFAULT_NULL) {
    if($att_type == 'password') {
      if($error = check_password_strength($att_val)) return $error;
    } elseif($att_type == 'email') {
      if(!is_email($att_val)) return  "$att_val is not a valid email address";
    } elseif($att_type == 'phone') {
      if(!phone_valid_idd($att_val)) return  "$att_val is not a valid phone number (include +country code, exclude leading zero)";
    } elseif($att_type == 'date') {
      if(($att_val == '0000-00-00' || !$att_val) && !$required) return ''; // ok with blank date if not req'd
      if(!is_date(strip_time($att_val)))  return  "$field: $att_val is not a valid date (use: yyyy-mm-dd)";
    } elseif($att_type == 'datetime' && strlen($att_val)==10) {
      if(($att_val == '0000-00-00' || !$att_val) && !$required) return ''; // ok with blank date if not req'd
      if(!is_date($att_val)) return  "$field: $att_val is not a valid date (use: yyyy-mm-dd)";
    } elseif($att_type == 'datetime') {
      if(($att_val == '0000-00-00 00:00:00' || !$att_val) && !$required) return ''; // ok with blank date if not req'd
      if(!is_datetime($att_val) && !is_date($att_val)) return  "$att_val is not a valid date/time (use: yyyy-mm-dd hh:mm:ss)";
    } elseif($att_type == 'time') {
      if(strlen($att_val) != 5) return "$field: $att_val is not a valid time (use: hh:mm)";
      elseif(!is_time($att_val.':00')) return "$field: $att_val is not a valid time";
    } elseif($att_type == 'int' || $att_type == 'intSelect') {
      if(!isint($att_val)) return  "$att_val is not an integer";
    } elseif($att_type == 'float') {
      if(!is_numeric($att_val)) return  "$att_val is not an number";
    } elseif(($att_type == 'discount' || $att_type == 'surcharge' || $att_type == 'commission' || $att_type == 'percent_or_number') && $att_val) {
      list($abs_discount, $perc_discount) = split_discount($att_val);
      $att_name = ucfirst($att_type);
      if($abs_discount && !is_numeric($abs_discount)) return "$att_name $abs_discount is not numeric.";
      if($perc_discount && !is_numeric($perc_discount)) return "$att_name $perc_discount is not numeric.";
      if($att_type == 'discount') {
        if($abs_discount < 0 || $perc_discount < 0) return "$att_name cannot be negative.";
        if($perc_discount > 100) return "$att_name cannot be greater than 100%";
      } elseif($att_type == 'surcharge' || $att_type == 'commission')  { // surcharge
        if($abs_discount < 0 || $perc_discount < 0) return "$att_name cannot be negative.";
      }
    }
  }
  return '';
}

// returns children of an object (one2many relation)
function get_object_children($child_type, $row, $this_id, $fld_options, $fld_args, $fld_args2='') {
  $child_table = get_object_table($child_type);
  $child_filters = "$child_table.$fld_args='$this_id'";
  if($fld_args2) $child_filters .= " AND ".implode(' AND ', search2filters($fld_args2)); // add custom filter if any
  $childRows = get_objects($child_type, $child_filters);
  return $childRows;
}

// replace __this->field with actual value (obj->field) if obj set 
function parse_filter($filter, $obj=NULL) {
  if(strpos($filter, '__this->')===false) return $filter; // no parsing needed
  $field = substr($filter, 8);
  return $obj->$field;
}
  

/** unsused - automatic comment added on  */
function parse_field($table, $field) { // converts class.class.class.field to table.field    
  if(!strpos($field, '.')) return array($field, "$table.$field");
  $elements = explode('.', $field);
  $field = array_pop($elements); // the field
  $table = get_object_table(array_pop($elements)); // the table of the parent type
  return array($field, "$table.$field");
}

/** unsused - automatic comment added on  */
function search_box($search_name, $search_val, $class, $js='') {
  $fld_type= $class[ATT_TYPE];
  $fld_options = $class[ATT_LENGTH];
  $fld_args= $class[ATT_ARGS];
  $fld_sort= $class[ATT_SORT];
  $fld_name = $class[ATT_NAME];  
  $filters = $class[ATT_FILTERS];
  if($fld_type=='many2one' || $fld_type=='many2many') {
    //return get_select_box_off($search_name, $fld_options, $search_val, false, $filters, $fld_sort, '', 15);
    return object_select($search_name, $fld_options, array('value' => $search_val, 'filters' => $filters, 'sort' => $fld_sort, 'size' => 15));
  } elseif($fld_type=='bool') {
    return yes_no_chooser($search_name, $search_val);
  } else {
    if(in_array($fld_type, array('link', 'id'))) $size=4;
    else $size = $fld_options ? $fld_options : 8; // min size = 6
    if($size>8) $size=8; // max size = 8
    return form_input($search_name, 'text', $search_val, array('size' => $size, 'js' => $js));
  }
}

// unless user is logged in, takes a list of changes, returns name + role of user responsible if found
function changed_by($changes) {

  $user_name = $_SESSION['user_name'];
  $user_id   = $_SESSION['user_id'];
  $role      = $_SESSION['user_type'];
  
  $who = "??";
  if(!$user_id) {
    foreach($changes as $change) {
      if($cid = $change['id']) {
        $changeObj = get_object('changelog', $cid);
        break;
      }
    }
    if($changeObj) {    
      $role = $changeObj->user_level ? get_object('user_level', $changeObj->user_level, 'user_type') : '';
      $user_id = $changeObj->user_id;
    }
  }
  if($user_name) {
    return $role ? "$user_name ($role)" : $user_name;
  } elseif($user_id) {
    $userObj = get_object('user', $user_id);
    $who = get_name_string($userObj).($role ? " ($role)" : "");
  } elseif($role) {
    $who = "The $role";
  }
  return $who;
}  

// new Ajax style recordset buttons
/** unsused - automatic comment added on  */
function recordset_buttons_inline($obj_type, $id, $fields='', $options=NULL) { 
  //dump($options);
  $options['context'] = 'recordset';
  if(auth_action($obj_type, 'edit', $id)) {
  	$buttons[] = edit_object_inline($obj_type, $id, $fields, $options); // add edit-link
  }
  
  if(auth_action($obj_type, 'del', $id)) {
    $buttons[] = delete_object_inline($obj_type, $id, $fields, $options); // add edit-link
  }
	$output = count($buttons) ? implode(" ", $buttons) : '';
	return $output;
}

function delete_object_inline($obj_type, $id, $fields='', $options=NULL) {    
  $obj_name = get_object_name($obj_type);
  $image = html_i("", "fa fa-trash-o");
  $confirm_text = "Are you sure you want to delete $obj_name $id";

  // options
  $handler =  $options['handler'];
  $reload = isset($options['reload']) ? $options['reload'] : 1; // reload is true by default (causes page to reload after successful save) 

  $desc = get_object_descriptor_string($obj_type, $id);
  if($desc) $confirm_text .= " ($desc[1])";
  $js = "onclick = 'if(confirm(\"$confirm_text ?\")) delete_object(\"$obj_type\", \"$id\", {\"handler\": \"$handler\", \"reload\": \"$reload\"}); return false;'";
  $title = phrase('delete', CAPITALIZE)." $obj_type $id";
  $button = html_link('#', $image, array('js' => $js, 'title' => $title));
  return $button;
}


// todo: only works if add_object_inline is used on same page (needs object_dialog) 
function edit_object_inline($obj_type, $id, $fields='', $options=NULL) {    
  if(!$fields) $fields = array(); 
  $action = $id ? 'edit' : 'add';
  $context = $options['context'];
  $data = $options['data'] = $options['data'] ?: array();
  $fields = $options['fields'] = $fields ?: array();
	$big_class = $context == 'recordset' ? "" : " fa-4x";

  if(!isset($options['reload'])) $options['reload'] = 1; // reload by default 
  $options_json = json_encode($options);
    
  $js = "onclick='object_dialog(\"$obj_type\", \"$id\", $options_json);return false;'";   
  $image = $options['image'];
  if(!$image) {
    if($id) {
      $image = html_i("", "fa fa-pencil-square".$big_class);
    } else {
      $image = html_i("", "fa fa-plus-square".$big_class);
    }
  }
  $title = phrase($action, CAPITALIZE)." ". ($field ? "$obj_type $field" : $obj_type);
  $button = html_link('#', $image, array('class' => $class, 'js' => $js, 'title' => $title));
  return $button;
}

/** unsused - automatic comment added on  */
function add_object_inline($obj_type, $fields='', $options=NULL) {    
  return edit_object_inline($obj_type, 0, $fields, $options);
}

/** unsused - automatic comment added on  */
function add_object_link($obj_type, $name='', $ref_name='', $ref_link='', $params='') {
  if(!$name) $name = get_object_name($obj_type);
  if($params && $params[0] != '&') $params = '&'.$params;

  $link = html_link(get_object_link($obj_type, 0, 'add', '', '', '', $ref_name, $ref_link).$params, html_i("", "fa fa-plus-square"), array('title' => phrase('add', CAPITALIZE)." $name"));
  return $link;                        
}

// jquery dialog for adding/editing objects
/** unsused - automatic comment added on  */
function object_dialog($obj_type, $obj=NULL, $options=NULL) {
  $options['dialog'] = true;
  $form = object_form($obj_type, $obj, $options);
  return html_div($form, 'hidden', "id='".$obj_type."_dialog'");
}

// returns a form for adding/updating object
// if id is __none, shows add form, otherwise edit form
// pass fields to select which fields to show
// if ajax is set, 
//function object_form($obj_type, $obj, $fields=0, $class='fieldset_top', $ajax=true, $handler='') {
function object_form($obj_type, $obj=NULL, $options=NULL) {
  global $__CLASSES;
  if(!$obj_type) return("object_form: no type");
  if(!$__CLASSES[$obj_type]) return("object_form: no definition for $obj_type");
  $class_def  = get_class_def($obj_type, true);
  $key = get_object_key($obj_type);  
  $edit = $add = false;
  if($id = $obj->$key) {
    $action = 'edit';
    $edit = true;
  } else {
    $action = 'add'; 
    $add = true;
  }
  if($readonly = $options['readonly']) $action = 'view';

  $title = pick_first($options['dialog_title'], $options['title'], phrase($action, CAPITALIZE). " ".get_object_name($obj_type).($id ? " $id" : ''));
  
  // options
  $show_form = isset($options['show_form']) ? $options['show_form'] : true; // if false, just return fields 
  $ajax = isset($options['ajax']) ? $options['ajax'] : true; // ajax submit true by default - do we want this? 
  $class = $options['class'] ?: 'fieldset_top'; 
  $captcha = $options['captcha']; 
  $autosave = $options['autosave'];

  // js options
  // todo: json_encode options
  
  $parent_id = $options['parent_id']; 
  $handler = $options['handler']; 
  $success_handler = $options['success_handler']; 
  $handler_data = $options['handler_data']; 
  $handler_operation = $options['handler_operation']; 
  $target = $options['target']; 
  $fields = $options['fields'] ?: array(); 
  $reload = $options['dialog'] ? (isset($options['reload']) ? $options['reload'] : 1) : 0; // whether or not to reload  
  $reload_link = $options['reload_link']; 
  $reload_template = $options['reload_template']; 
  $reload_target = $options['reload_target']; 
  $class_options = $options['class_options'] ? json_decode($options['class_options'], true) : array();
    // override standard class definition

  $form_name = $options['id'] ?: $action.'_'.$obj_type;
  $form_class = 'ajax-submit'; /** new */
  $form_options = array();
  $form_options['data-obj_type'] = $obj_type;
  $form_options['data-obj_id'] = $id;
  $form_options['id'] = $form_id = $options['id'] ?: $action."_".$obj_type.($id ? "_".$id : '');

  $errors = html_div('', 'error', "id='".$form_id."_errors'");
  $messages = html_div('', 'form_message', "id='".$form_id."_messages'");  

  if($ajax) { // javascript to handle ajax submission, display errors/messages + closing parent dialog
    if($edit && $autosave) { // save automatically on change
      $form_class = 'autosave';
    } else {
      $on_save = "onclick=\"return save_object('$form_id', '$obj_type', '$action', {'handler': '$handler', 'success_handler': '$success_handler', 'handler_operation': '$handler_operation', 'target': '$target', 'reload': '$reload', 'reload_template': '$reload_template', 'reload_target': '$reload_target', 'reload_link': '$reload_link', 'parent_id': '$parent_id'});\"";
      $on_save = ''; /** new */
      $on_cancel = "onclick=\"close_parent_dialog('$form_id');return false;\"";
    }
  }

  $links = array(); 
  if(count($fields)) {
    $fields_str = implode(',', $fields);
    $attr_fields = attribute_fields($fields);
    $type = $options['type'];
    if(count($fields) == 1 && $type) {
      $fld_name = $fields[0];
      $attr_fields[$fld_name][ATT_TYPE] = $type; // field type override
    }
  } else {
    $attr_fields = $class_def->fields;
  }
  
  if($obj_type == 'page_element' && $id) {
    $element_type = get_object('page_element', $id, 'type');
    $class_def->fields['content'][ATT_TYPE] = $element_type ?: 'text';
  }
  
  foreach($class_options as $field => $override) {    
    foreach($override as $att_type => $att_override) {
      $class_def->fields[$field][$att_type] = $att_override;
    }
  }
  $links_str = html_div(implode(' ', $links), 'abs_top_right');
  $record = record_fields($class_def, $obj, $id, $action, 0, '', 'messages', $attr_fields);
  
  if(!$options['autosave'] && !$options['hide_buttons']) {
    if($readonly) {
      $buttons[]= form_input('cancel_object', 'submit', ' Close ',  array('js' => $on_cancel, 'class' => 'btn btn-danger')); 
    } else {
      $buttons[] = form_input($action.'_object', 'submit', ' Save ', array('js' => $on_save, 'class' => 'btn btn-success')); 
      $buttons[]= form_input('cancel_object', 'submit', ' Cancel ',  array('js' => $on_cancel, 'class' => 'btn btn-danger')); 
    }
    $hidden_fields = array('obj_type' => $obj_type, 'action' => $action, 'oper' => $action);
  }

  if($id) $hidden_fields['id'] = $id; 
  $hidden_fields['obj_type'] = $obj_type; 
  
  
  
  $rows = array();
  foreach($record as $fld => $input) {
    $rows[] = array($fld, $input);
  }
  
  $fieldset = $errors.$messages.$links_str;
  $fieldset .= html_table($rows, array('class' => 'table table-bordered table-striped table-booking-history', 'header' => 0));
  $fieldset .= array2form($hidden_fields, array('hidden' => true));
  
  if(!$show_form) return $fieldset; // just return the fieldset

  if($buttons) $fieldset = $fieldset.html_break().implode(' ', $buttons);
  $form_options['data-handler'] = 'save';
  $form_options['data-success_handler'] = $success_handler;
  $form_options['data-handler_data'] = $handler_data;
  $form_options['data-reload_link'] = $reload_link;
  
  $form = $captcha ? html_secure_form($fieldset, $form_name) : html_form($fieldset, $form_name, '', 'POST', $form_class, $form_options);

  if($_SESSION['live']) $debug = '';
  //$debug = "HD:".dump($handler_data,true);
  
  return $debug.html_h4($title).$options['tip'].$form;   
}

// action buttons

/** unsused - automatic comment added on  */
function button_add_media($obj_type, $parent_type, $parent_id, $descriptiontype_id=0, $extra_fields='') {
  
  if(!$descriptiontype_id) $descriptiontype_id = DESC_TYPE_ALBUM; // default is photo album
  
  $parent_action = 'view'; // todo - allow edit, add ?

  // Image upload form (URL + file upload button)
  $max_upload_M=8; // maximum file size server will accept = 8 Megs
  $max_upload_K=$max_upload_M * 1024;
  $max_upload =$max_upload_K * 1024;

  $media->parent_id = $parent_id;
  $media->parent_type = $parent_type;
  $media->descriptiontype_id = $descriptiontype_id; // default is 10: photo album

  $hidden = array('__parent_id' => $parent_id, '__parent_type' => $parent_type, 'obj_type' => $obj_type, 'MAX_FILE_SIZE' => $max_upload);
  $add_button .= array2form($hidden);
  $add_button .= obj2form($media);

  $add_button .= "URL: ".form_input('url', 'text', '', array('size' => 15)); 
  $add_button .= " or File: ".form_input('userfile', 'file', '', array('size' => 10));   
  $add_button .= $extra_fields;
  $add_button .= " ".form_input('addinline', 'submit', ' Add '); 

  return html_form($add_button, 'upload_media', $_SERVER[PHP_SELF], $method='post', '', "enctype='multipart/form-data'");
}

// Option A: takes a simple array (a, b, c ...) returns an an associate array (a => array(), b => array(), c => array() ....)
// Option B: take an associative array (a => 'name', b => 'name2', return , returns an an associate array (a => array('name1'), b => array('name2')
function attribute_fields($fields) {
  if(isAssoc($fields)) {
    foreach($fields as $k=>$v) $fields[$k] = is_array($v) ? $v : array($v);
  } else {
    $fields = array_flip($fields);
    foreach($fields as $k=>$v) $fields[$k] = array(); // initalize as empty arrays
  }
  return $fields;  
}

// returns an array of all fields in class which match search
// search is array($index => $value, $index2 => $value2), where the index is the index of the attribute array (below)

//   CLASS    USER_LEVEL  GROUP TABLE KEY FLAGS DESCRIPTOR SORT  PARENT_CLASS PARENT_FIELD
//            0           1     2    3     4      5       6        7            8

// return the field names only
function class_fields($obj_type, $one2one_integrate=true) {
  return array_keys(get_object_attributes($obj_type, $one2one_integrate));
}

// return the field names only, and only if they are a database field 
function database_fields($obj_type, $one2one_integrate=true) {
  $attrs = get_object_attributes($obj_type, $one2one_integrate);
  $fields = array();
  foreach($attrs as $field => $att) {
    $type = $att[ATT_TYPE];    
    if(!in_array($type, array('many2many', 'dummy', 'one2many'))) {
      //echo("adding $field type=$type<br>");
      $fields[] = $field;
    }
  }
  return $fields;
}

function find_class_fields($type, $search) {
  $attributes = get_object_attributes($type, false);
  $fields = array();
  foreach($attributes as $field => $attAr) {
    foreach($search as $index => $value) {
      $match = $attAr[$index] == $value; 
      if(!$match) break;
    }
    if($match) $fields[] = $field;
  }
  return $fields;
}

// returns attributes for given type and field
function get_object_attribute($type, $field, $one2one_integrate=true) {
  $attrs = get_object_attributes($type, $one2one_integrate);
  return $attrs[$field];
}

// get attributes of a class. if $get_relations is set, then the fields of relations (type one2one or one2many) are also fetched.
function get_object_attributes($type, $one2one_integrate) {
  global $__CLASSES;
  global $__CLASS_ATTRIBUTES;
  $type = strtolower($type);
  $class = $__CLASSES[$type]; // ?: $__CLASSES['default_class'];
  $fields = $__CLASS_ATTRIBUTES[$type]; //  ?: $__CLASS_ATTRIBUTES['default_attributes'];
  
  if(!$class) mydie("get_object_attributes: Class '$type' not found");
  if(!$fields) mydie("get_object_attributes: Attributes for class $type not found");
  while($parent_type = $class[CLASS_PARENT_CLASS]) {
    $parent_class = $__CLASSES[$parent_type];
    $parent_fields  = $__CLASS_ATTRIBUTES[$parent_type];
    if($parent_class && $parent_type) { // parent must be defined in $__CLASSES and have attributes
      $class = $parent_class;
      $fields = array_merge($parent_fields, $fields); // child class overrides parent class
    }      
  }

  $result = $one2one_integrate ? one2one_integrate($type, $fields) : $fields;

  return $result;
}

// merges child classes (one2one integrate)
function one2one_integrate($type, $fields) {
  $child_fields = array();
  $org_fields = $fields;
  $unset = array();

  foreach($fields as $field => $attributes) {
    $att_type = $attributes[ATT_TYPE];
    if($att_type != 'one2one' ) continue;  
    $att_flags = $attributes[ATT_FLAGS];
    //if($type == 'site') echo("gh: fld=$field<br>");
    if($att_flags & AF_ONE2ONE_INTEGRATE || $att_flags & AF_OBLIGATORY) {
      $att_options = $attributes[ATT_RELATION_TYPE];
      $child_required = $att_flags & AF_OBLIGATORY ? "(**)" : "";
      $child_fields = get_object_attributes($att_options, true);
      $child_key = get_object_key($att_options);
      $child_table = get_object_table($att_options);
      $heading = get_object_name($att_options) . $child_required;
      array_push($fields, $heading); // first field is header
      //array_push($fields, get_object_name($att_options)); // first field is header
      foreach($child_fields as $cfield=>$cclass) {
        $cfields[$att_options.".".$cfield] = $child_fields[$cfield];
        //$fields[$child_table.".".$cfield] = $child_fields[$cfield];
      }
      $fields = $att_flags & AF_ONE2ONE_PREPEND ? array_merge($cfields, $fields) : array_merge($fields, $cfields); // which to show first? 
      $child_key_field = $att_options.".".$child_key;
      $unset[] = $child_key_field; 
      //unset($fields[$child_table.".".$child_key]); // don't include id of child
    }
  }
  foreach($unset as $fld) unset($fields[$fld]); // don't include id of child 
  return $fields;
}

// returns true if any of the fields (array) appear in changes
/** unsused - automatic comment added on  */
function object_changed($changes, $fields) {
  return array_intersect_key($changes, array_flip($fields));
}

function object_has_attribute($type, $attr) {
  $class_def = get_object_attributes($type, true);
  return $class_def[$attr] ? true : false;
}


// takes an array of objects, return unordered list
// if parent_field is set, will look for children
/** unsused - automatic comment added on  */
function objects2ul($obj_type, $objects, $options=NULL) {
  if(!$objects) return '';
  $depth = ++$options['depth'];
  $indent = str_repeat("  ", $depth);
  $key = get_object_key($obj_type);
  $classes = array();
  $name_field   = $options['name_field'] ?: 'name';
  $parent_field = $options['parent_field'];
  $item_prefix = $parent_field ? 'tree_' : 'list_';
  foreach($objects as $obj) {
    $list[$obj->$key] = $obj->$name_field;
    $classes[] = $obj->is_folder ? 'folder' : '';
    if($parent_field && ($children = find_objects($obj_type,  array($parent_field => $obj->$key), array('sort' => $options['sort'])))) {
      $list[$obj->$key] .= "\n$indent".objects2ul($obj_type, $children, $options);
    }    
  }
  return html_ul($list, $options['class'], array('js' => $options['js'], 'show_id' => true, 'item_classes' => $classes, 'item_prefix' => $item_prefix));
}

// recursively update all childeren objects with given data (silently, do not call update_object)
function object_update_children($obj_type, $id, $data, $parent_field = 'parent_id') {
   $key = get_object_key($obj_type);
   $table = get_object_table($obj_type);   
   $child_ids = db_getCol("SELECT $key FROM $table WHERE $parent_field='$id'");
   if(count($child_ids)) {
     $tuplets = array_tuplets($data);
     $query = "UPDATE $table SET $tuplets WHERE $parent_field='$id'";
     db_query($query);
     echo($query.html_break());
     foreach($child_ids as $child_id) object_update_children($obj_type, $child_id, $data, $parent_field);
   }
}

/** unsused - automatic comment added on  */
function is_class($class) {
  global $__CLASSES;
  return $__CLASSES[$class] ? true : false;
}

/** unsused - automatic comment added on  */
function object_has_field($obj_type, $field) {
  $fields = object_fields($obj_type, true);
  return in_array($field, $fields);
}

// returns a simple array with the field names of an object
function object_fields($type, $one2one_integrate=false, $remove_key=false) {
  $attrs = get_object_attributes($type, $one2one_integrate);
  if($remove_key) {
    $key = get_object_key($type);
    unset($attrs[$key]);
  }
  return array_keys($attrs);
}

function object_increment($obj_type, $id, $field) {
  if(!$obj_type) die("object_increment: missing $obj_type");
  if(!$id) die("object_increment: missing $id");
  if(!$field) die("object_increment: missing $field");
  $table = get_object_table($obj_type) ?: die("object_increment: no table for $obj_type");
  $key = get_object_key($obj_type) ?: die("object_increment: no key for $obj_type");
  
  db_query("UPDATE $table SET $field = $field+1 WHERE $key=$id");
}

function object_json_data($obj_type, $obj_or_id, $fields = array()) {
  $data = array();
  $obj = any2obj($obj_type, $obj_or_id);
  if(!$fields) $fields = object_fields($obj_type, true);
  foreach($fields as $fld) $data[$fld] = $obj->$fld;
  return json_encode($data);    
}

// returns lat/long of an object
/** unsused - automatic comment added on  */
function object_latlng($obj_type, $obj_or_id, $options = array()) {  
  $obj = any2obj($obj_type, $obj_or_id);
  if(abs($obj->location_lat) > 0 && abs($obj->location_long) > 0) return array($obj->location_lat, $obj->location_long); 
  $key = get_object_key($obj_type);
  $id = $obj->$key;
  $lat = $long = 0;
  $locObj = $obj->location_id ? get_object('location', $obj->location_id) : find_object('location', array('parent_type' => $obj_type, 'parent_id' => $id));
  if($locObj) {
    $lat = $locObj->location_lat;
    $long = $locObj->location_long;
  }
  return array($lat, $long);
}

// returns a fine uploader for give object
function object_media_uploader($obj_type, $obj_or_id, $field='media', $options = array()) {
  global $u_id, $std_media_options;
  if($obj_or_id) {
    $obj = any2object($obj_type, $obj_or_id);
    $key = get_object_key($obj_type);
    $id = $obj->$key;
    $val = $obj->$field;
  } else {
    $id = 0;
    $val = '';
  }
  
  $field_attributes = get_object_attribute($obj_type, $field);
  $fld_options = $field_attributes[ATT_OPTIONS];
  $upload_options = $fld_options ?: $std_media_options;
  if(!is_array($upload_options)) $upload_options = $std_media_options;
  if(is_array($options)) { // override standard options
    $data_keys = array(
     'allow_multiple', 'allowed_types', 'allowed_extensions', 'size_limit', 'handler', 'path', 'title', 'media_data', 'resize', 'target', 'cloud', 'view',
     'minimum_file_size', 'minimum_height','minimum_width', 'minimum_size', 'maximum_file_size', 'maximum_height','maximum_width', 'maximum_size', 'maximum_files');
    foreach($data_keys as $key) {
      if(isset($options[$key])) {
        $upload_options[$key] = $options[$key];
        unset($options[$key]); 
      }
    }
  }    

  $upload_options['media_data'] = array('parent_id' => $id, 'parent_type' => $obj_type, 'parent_field' => $field);
  if($upload_options['private'] || $options['private']) $upload_options['media_data']['user_id'] = $u_id; // private means files are uploaded in directory per user 
  
  $upload_options['data-value_list'] = $val;
  $idAr = array($obj_type, $id, $field);
  print_log('object_media_uploader: options='.dump($options,true), 'upload', LOG_LEVEL_TEST);
  return jquery_file_uploader(implode('-', $idAr), $val, $upload_options, $options);
}

// check if file to be uploaded is ok according to options
function check_uploaded_file($qq_file, $options) {
  if($error = $qq_file['error']) return $error;
  $errors = array();
  $tmp = $qq_file['tmp_name'];
  if(!is_uploaded_file($tmp)) return "File $tmp was not uploaded via POST";
  $uploaded_file = $qq_file['tmp_name'];
  
  $qq_name = $qq_file['name'];  
  $qq_mime = $qq_file['type'];
  $qq_size = $qq_file['size'];
  if($qq_mime == 'audio/mp3') $qq_mime = 'audio/mpeg'; // Chrome fix

  //extract($options);
  $allowed_types = $options['allowed_types'] ?: array();
  $allowed_extensions = $options['allowed_extensions'] ?: array();

  // type - based on mime_type
  
  $media_type = find_object('mime_type', array('mime' => "*$qq_mime*"), 'type');
  if(!$media_type) return "Unknown type $qq_mime";
  if($allowed_types && !in_array($media_type, $allowed_types)) return "Illegal file type in $qq_name. Allowed type(s):".implode(', ', $allowed_types);

  // error checks after file is uploaded and moved
  // type - based on filename
  list($root, $extension) = split_filename(strtolower($qq_name));
  if($extension) {
    if($allowed_extensions && !in_array($extension, $allowed_extensions)) return "Illegal extension $media_extension. Allowed extension(s):".implode(', ', $allowed_extensions);   
    $file_mime = mime_type($extension);
    if($file_mime && $file_mime != $qq_mime) return phrase('mime_type_mismatch', CAPITALIZE)." File: $qq_mime File name: $file_mime";
  } elseif(!$qq_mime) { // no extension (Mac?), use mime_type
    $errors[] = "Missing extension and mime type";
  }
  
  // size
  $size_limit_human = $options['maximum_file_size'] ? bytes2human($options['maximum_file_size']) : ini_get('upload_max_filesize'); // Specify max file size in bytes.
  $size_limit = human2bytes($size_limit_human);
  if($options['minimum_file_size'] && $qq_size < $options['minimum_file_size']) $errors[]= phrase('minimum_file_size', CAPITALIZE).":".bytes2human($options['minimum_file_size'])." (Size=".bytes2human($qq_size).")";
  if($options['maximum_file_size'] && $qq_size > $options['maximum_file_size']) $errors[]= phrase('maximum_file_size', CAPITALIZE).":".bytes2human($options['maximum_file_size'])." (Size=".bytes2human($qq_size).")";

  // type specific checks
  if($media_type == 'image') {

    // check mime type
    $img_info_array = getimagesize($uploaded_file); // size of original
    if(!$img_info_array) return "Not a valid image";
    $result['image_info'] = $img_info_array;
    $mime_type = $img_info_array['mime'];
    if($mime_type != $qq_mime) return "Illegal mime_type '$mime_type' for $qq_name - expected '$qq_mime'";
  
    // resize or move as needed
    $width = $img_info_array[0];
    $height = $img_info_array[1];
    $size = $long_side = max($height, $width);
    $short_side = min($height, $width);
    
    if($options['minimum_height'] && $height < $options['minimum_height']) $errors[]= phrase('minimum_height', CAPITALIZE).": ".$options['minimum_height']."px";   
    if($options['minimum_width']  && $width  < $options['minimum_width'] ) $errors[]= phrase('minimum_width', CAPITALIZE) .": ".$options['minimum_width']."px";
    if($options['minimum_size']) {
      if(is_array($options['minimum_size'])) { 
        list($min1,$min2) = $options['minimum_size'];
        $min_short = min($min1,$min2);
        $min_long = max($min1,$min2);
        if($short_side < $min_short && $long_side < $min_long) { 
          $errors[]= phrase('short_side_must_be_at_least', CAPITALIZE).": ".$min_short."px ".phrase('or', UPPERCASE)." ".phrase('long_side_must_be_at_least', CAPITALIZE).": ".$min_long."px";;              
          $errors[]= "This= $short_side x $long_side px";
        }
      } else {
       if($size < $options['minimum_size']  ) $errors[]= phrase('minimum_size', CAPITALIZE)  .": ".$options['minimum_size']."px";
      }
    }
    if($options['maximum_height'] && $height > $options['maximum_height']) $errors[]= phrase('maximum_height', CAPITALIZE).": ".$options['maximum_height']."px";
    if($options['maximum_width']  && $width  > $options['maximum_width'] ) $errors[]= phrase('maximum_width', CAPITALIZE) .": ".$options['maximum_width']."px";
    if($options['maximum_size']   && $size   > $options['maximum_size']  ) $errors[]= phrase('maximum_size', CAPITALIZE)  .": ".$options['maximum_size']."px";   
  }
  $ts = now();
  print_log("{$ts}: check_uploaded: t=$media_type h=$height w=$width s=$size minw=$options[minimum_width]", 'upload', LOG_LEVEL_ALWAYS);
  return count($errors) ? implode(', ', $errors) : '';
}


// handles the uploading of media from user file
// qqfile is $_FILES array, mediaData is parent object/field, options are required: max size, resize etc
function object_media_upload($qqfile, $mediaData, $options) {
  global $upload_dir;
  global $u_id;
  $errors = array();
  $$log_level_test = LOG_LEVEL_ALWAYS;

  //print_log("New upload: fileArray=".dump($qqfile, true)."options=\n".dump($options, true), 'upload', $log_level_test);
  extract($options);
  
  require_once("blueimp/UploadHandler.php");

  $target_field = $options['target_field'];

  if(is_array($qqfile['tmp_name'])) {
    $qqfile['name'] =     $qqfile['name'][0];    // => house.jpeg
    $qqfile['type'] =     $qqfile['type'][0];    // => image/jpeg
    $qqfile['tmp_name'] = $qqfile['tmp_name'][0];// => /tmp/phps7Pcl6
    $qqfile['error'] =    $qqfile['error'][0];   // => 0
    $qqfile['size'] =     $qqfile['size'][0];    // => 11546                                               
  } 

  $qq_name  = $qqfile['name']; 
  $qq_mime  = $qqfile['type']; 
  $qq_tmp   = $qqfile['tmp_name']; 
  $qq_error = $qqfile['error']; 
  $qq_size  = $qqfile['size'];     

  if($qq_mime == 'audio/mp3') $qq_mime = 'audio/mpeg'; // Chrome fix
  
  $user_id = $options['user_id'] ?: $u_id;
  $upload_path = $options['path'];
  
  print_log("Upload: name=$qq_name mime=$qq_mime tmp=$qq_tmp error=$qq_error size=$qq_size type=$type user=$user_id path=$upload_path", 'upload', $log_level_test);

  // before moving uploaded file to destination directory, check if file is acceptable
  if($error = check_uploaded_file($qqfile, $options)) {
    print_log("Upload: pre-process error: $error", 'upload', $log_level_test);      
    return json_error_object($error);  
  }
  
  // path:
  if($mediaData && is_array($mediaData)) { // add media object
    $parent_user  = $mediaData['user_id']; // set this in media data to group files by user, otherwise global
    $parent_type  = $mediaData['parent_type'];
    $parent_id    = $mediaData['parent_id'];
    $parent_field = $mediaData['parent_field'];
    $obj_dir = implode('/', array_filter(array($parent_type, $parent_id, $parent_field)));
  
    //print_log("Upload: media data: ".dump($mediaData, true), 'upload', $log_level_test);      
    //print_log("Upload: path was: $path", 'upload', $log_level_test);      
    if($upload_path) $upload_path = str_replace(array('$parent_user', '$parent_type', '$parent_id', '$parent_field'), array($parent_user, $parent_type, $parent_id, $parent_field), $upload_path); // variables in path
    //print_log("Upload: path now: $path", 'upload', $log_level_test);      

    if(!$upload_path) $upload_path = $obj_dir ? clean_path("/user/$parent_user/object/$obj_dir/$type/") : clean_path("/user/$user_id/upload/$type/");

  } else if(1 || $target_field) { // don't add media yet, just upload the file and write path to field

    if(!$upload_path) $upload_path = clean_path("/user/$user_id/upload/$target_field/$type/");
    
    $result['success'] = 1;
    
  } else { // no data or target
    
    $result['error'] = "No media data or target field";
    $result['success'] = 0;
    echo json_encode($result);
    exit;
    
    $upload_path = $options['path'] ?: clean_path("/user/$user_id/upload/$type/");
  }

  $upload_dir  = clean_path($_SERVER['DOCUMENT_ROOT'].$upload_path);
  if(!is_dir($upload_dir) && !mkdir($upload_dir, 0777, true)) json_error("Upload file: Could not make $upload_dir");    
  
  
  // set options

  $uploaderOptions['upload_dir'] = $upload_dir;
  $uploaderOptions['upload_url'] = $upload_path;
  
  $uploader = new UploadHandler($uploaderOptions, false); // false to not initialize
  
  // upload file
  $result = $uploader->post(false); // don't generate response
  $result_file = $result['files'][0];

  foreach($qqfile as $k => $v) if(!isset($result[$k])) $result[$k] = $v;
  $result['size'] = $qq_size = $result_file->size;
  $result['name'] = $qq_name = $upload_name = $result_file->name;
  $result['type'] = $qq_mime = $result_file->type;
  $result['error'] = $qq_error = $error = $result_file->error ?: '';
  $result['tmp_name'] = $qq_tmp = $result_file->tmp_name;
  $result['success'] = $qq_size && !$qq_error ? 1 : 0; 
  if($qq_error) return json_error_object($qq_error);
  
  //json_error("dir={$upload_dir} lib={$uploadlib}");

  
  // now invoke PHP handler to move uploaded file - incl. addition error checking
  /** Not in use: Valum Fine Uploader 
  if($uploadlib == 'jquery_fileupload') {
    
  } else {  
    $uploader = new qqFileUploader();
  
    $uploader->sizeLimit = $size_limit;
    $uploader->inputName = 'qqfile'; // Specify the input name set in the javascript.
    
    //$uploader->chunksFolder = 'chunks'; // If you want to use resume feature for uploader, specify the folder to save parts.
    
    $result = $uploader->handleUpload($upload_dir);
    $upload_name = $uploader->getUploadName();

    foreach($qqfile as $k => $v) if(!isset($result[$k])) $result[$k] = $v;

  }
  */
  
  
  $result['upload_name'] = $upload_name;
  $result['upload_path'] = $upload_path;
  $result['upload_dir'] = $upload_dir;
  $result['size_limit'] = $size_limit;
  $result['size_limit_human'] = $size_limit_human;

  //print_log("Uploader ($uploadlib) Handler Result Mime=$qq_mime name=$upload_name \n".dump($result, true), 'upload', $log_level_test);

  // error checks after file is uploaded and moved
  // type - based on filename
  $filename  = strtolower($qqfile['name']); 
  list($root, $extension) = split_filename(strtolower($upload_name));
  if($extension) {
    $file_mime = mime_type($extension);
    if($file_mime && $file_mime != $qq_mime) return phrase('mime_type_mismatch', CAPITALIZE)." File: $qq_mime File name: $file_mime";
  } elseif($qq_mime) { // no extension (Mac?), use mime_type
    $extension = find_object('mime_type', array('mime' => $qq_mime, 'extension'));
    $new_upload_name = $upload_name.".$extension";
    if(!rename($uploaded_file, $upload_dir.$upload_name)) $errors[] = "Couldn't move $upload_name to $new_upload_name";
    else $upload_name = $new_upload_name;
  } else {
    $errors[] = "Missing extension and mime type";
  }

        
  if($success = $result['success']) {
    $uploaded_file = $upload_dir.$upload_name;
                                                                                                                                 
    //print_log("Options = ".dump($options,true), 'upload', $log_level_test);
    //print_log("Success!", 'upload', $log_level_test);
    if(!$errors && $mediaData && is_array($mediaData)) { // add media object
      $mediaData['path']      = $upload_path;
      $mediaData['directory'] = $upload_dir;
      $mediaData['filename']  = $upload_name; // the actual name of the uploaded file

      $mediaData['mime']      = $qq_mime;
      $mediaData['type']      = find_object('mime_type', array('mime' => $qq_mime), 'type');
      $mediaData['filesize']  = $qq_size;
      $mediaData['user_filename']  = $qq_name; // the file name on user's disk
      
      $old_media_id = $mediaData['id'];     
      if(!$old_media_id && !$allow_multiple) { // only one allowed, look for existing
        $mediaSearch['parent_type'] = $parent_type; 
        $mediaSearch['parent_id'] = $parent_id; 
        if($parent_field) $mediaSearch['parent_field'] = $parent_field;             
        if($oldMediaObj = find_object('media', $mediaSearch)) {
          $copyAr = array('name', 'comment');
          foreach($copyAr as $k) $data[$k] = $oldMediaObj->$k; // copy vars from old object (do we want this?)
          $old_media_id = $oldMediaObj->id;
        }              
      }
      
      // add meta data used by pre_add_media handler
      $mediaData['ajax_upload'] = 1; 
      $mediaData['resize'] = is_object($options['resize']) ? (array) $options['resize'] : $options['resize']; 
      $mediaData['allowed_extensions'] = is_object($options['allowed_extensions']) ? (array) $options['allowed_extensions'] : $options['allowed_extensions'];

      if($old_media_id) { // existing media - only used when editing existing mediaObject
        unset($mediaData['created']);
        unset($mediaData['modified']);
        unset($mediaData['src']);
        unset($mediaData['id']);            
      }
      
      //print_log("Adding Media with data = ".dump($mediaData,true), 'upload', $log_level_test);
      list($mid, $merrors) = add_object('media', $mediaData, array('parent_id' => $media_id)); // add media
      //print_log("Result=$mid errors = ".dump($merrors,true), 'upload', $log_level_test);

      if($mid) { // return all fields of media with result
        print_log("Ok, added media $mid", 'upload', $log_level_test); 
        $mediaObj = get_object('media', $mid);
        if($old_media_id) list($d_count, $derrors) = delete_object('media', $old_media_id);
        $result['data'] = obj2array($mediaObj);
        if($mediaObj->path != $upload_path) { // media was moved by pre_add_media
          $result['upload_path'] = $mediaObj->path;
          $result['upload_dir'] = clean_path($_SERVER['DOCUMENT_ROOT'].$mediaObj->path);
        }
        if($mediaObj->src != $upload_path) { // media was renamed by pre_add_media
          $result['upload_name'] = $result['files'][0]->name = $mediaObj->src;
        }
        
        if($derrors) {
          //$errors = array_merge($errors, $derrors);              
        } else {
          //$result[''] = "Deleted $
        }
        $result['files'][0]->deleteUrl = "/ajax.php?oper=del&obj_type=media&id=$mid"; // used by jquery-file-uploader
        $result['files'][0]->url = print_media($mediaObj, array('size' => 'medium', 'source_only' => true));
        if($mediaObj->resize) $result['files'][0]->thumbnailUrl = print_media($mediaObj, array('size' => 'thumb', 'source_only' => true));
        
      } elseif($merrors) {
        $errors = array_merge($errors, $merrors);
        $result['success'] = 0;              
        print_log("Add media errors:".dump($merrors, true), 'upload', $log_level_test); 
        
      } else {
        $errors[] = "No Media ID received";
        $result['success'] = 0;              
        //$errors = array_merge($errors
      }
    }
    //print_log("Mid=$mid && $parent_type && $parent_id && $parent_field\n", 'upload', $log_level_test);
  } else {
    print_log("Uploader Initial Fail\n".dump($result, true), 'upload', $log_level_test);
    $result['success'] = 0;                       
    if(!$result['error']) {
      $size = $result['size']; 
      if($size > $size_limit) $result['error'] = "$size > $size_limit";
      else $result['error'] = "Upload failed for unknown reason (permissions?)";
    }
  }

  if($errors) {
    @unlink($uploaded_file); // delete uploaded file
    $result['success'] = 0;
    if(is_object($errors)) $errors = (array) $errors;
    $result['error'] = implode("\n", $errors);
    print_log("Upload Errors - unlinking\n", 'upload', $log_level_test); 
    $result['files'][0]->errors = $result['error']; // for jquery file-uploader
    $result['files'][0]->size = 0;
  }  
  
  // log result
  //print_log("Upload Result\n".dump($result, true), 'upload', $log_level_test);
  return $result;
}


/* 
  Misnamed: to be confused with above
  iterates through images and uploads to local server 
  object must have following fields:
  -images: 
   1) comma separated list, relative to image_base (or configured using options['field']
   2) json array where each element must be an object with src or url + optionally other media atributes (from the media object)
  -media: IDs of uploaded images will be stored (or configured using options['target_field']

  -image_base: (optional base URL)
*/
 
function object_image_uploader($obj_type, $obj_or_id, $path, $options=array()) {
  $errors = array();
  $debug = $options['debug']; 
  $obj = any2obj($obj_type, $obj_or_id);
  $key = get_object_key($obj_type);
  $id = $obj->$key;
  
  //print_log("object_image_uploader: $obj_type $obj_or_id path=$path option=".dump($options, true), 'resize',  LOG_LEVEL_TEST);
  //print_log("object_image_uploader stack: ".caller_stack(), 'resize',  LOG_LEVEL_TEST);
  
  if(!$obj_type) return array(0, array("object_image_uploader: Missing obj_type")); 
  if(!$id) return array(0, array("object_image_uploader: Missing id")); 
  if(!$path) return array(0, array("object_image_uploader: Missing path")); 
  //$obj = get_object($obj_type, $id);
  if(!$obj) return array(0, array("object_image_uploader for $obj_type $id: Couldn't find object"));

  $site_id = pick_first($options['site_id'], $obj->site_id, $_SESSION['site_id']);

  $field = $options['field'] ?: 'images'; // the source 
  $format = $options['format'] ?: '';  
  $map = $options['map'] && is_array($options['map']) ? $options['map'] : null; // assoc. array to map fields
   
  $media = $obj->$field; // comma separted list (or json), including main image (which should be first)
  if(!$media) return array(0, array("object_image_uploader for $obj_type $id: Couldn't find media in field '$media'"));

  $image_base = $obj->image_base ? rtrim($obj->image_base,'/').'/' : ''; // optional base URL
  $images = $format == 'json' ? json_decode($media, true) : explode(',', $media);
  if(!is_array($images)) $images = array($images); // in case of single json object
  
  $site_dir = site_directory($site_id) ?: $_SERVER['DOCUMENT_ROOT'];
  $site_url = site_full_url($site_id);
  $imagedir = clean_path($site_dir.$path);
  if(!is_dir($imagedir) && !mkdir_recursive($imagedir)) return array(0, array("object_image_uploader for $obj_type $id: Couldn't make upload directory $imagedir"));

  $keys = array('resize', 'min_size', 'allowed_extensions');
  $data = array_copy($options, $keys); 
  
  if(!isset($options['resize'])) $options['resize'] = 1; // resize by default
  $data['resize'] = $options['resize'];
  $data['parent_id'] = $id;
  $data['parent_type'] = $obj_type;
  $data['parent_field'] = $options['target_field'] ?: 'media'; // the destination
  $data['site_id'] = $site_id;
  $data['path'] = $path;
  $data['directory'] = $imagedir;
  $base_data = $data;

  if($debug) dump($data);
  
  $i = 0;
  foreach($images as $image) {
    if($format == 'json' && is_array($image)) {
      $data = array_merge($base_data, $image); // rest of data
      if($map) $data = map_array($data, $map); 
      $url = $data['src'] ?: $data['url'];
    } else {
      $url = $image_base.$image;
    }

    if(is_url($url)) { // full URL or local path allowed
      $data['url'] = $url;
    } else if(is_url($site_url.$url)) { // full URL or local path allowed
      $url = $site_url.$url;
      $data['url'] = $url;
    } else {
      $is_url = is_url($url) ? "yes" : "no";
      $errors[] = "$url is not a valid URL, and neither is $site_url".$url;
      if($debug) die("object_image_uploader: not url: $url is_url=$is_url<br>".dump($data, true).dump($image, true));
      continue;
    }
    //echo("object_image_uploader data=".dump($data, true));exit;
    list($media_id, $media_errors) = add_object('media', $data);
    if($media_errors) {
      $errors = array_merge($errors, $media_errors);
      if($debug) {dump($errors);dump($data);dump($map);dump($image  );die();}
    } else if($media_id) {
      if($debug) echo("Added media $media_id for $obj_type $id<br>");
      $i++;
    }
  }
  return array($i, $errors);
}

/** unsused - automatic comment added on  */
function object_site($obj) {
  global $__SITE, $__SITE_ID;
  $site_id = $obj->site_id ?: $__SITE_ID;
  $siteObj = $site_id ? get_object('site', $site_id) : $__SITE;
  return $siteObj;
}

// returns slug field for object type
/** unsused - automatic comment added on  */
function object_slug_field($obj_type) {
  if(is_property_type($obj_type)) return 'shortname';
  if($fields = find_class_fields($obj_type, array(ATT_TYPE => 'slug'))) return $fields[0];
  return '';
}

// returns a unique slug for object given the name
function object_slug($obj_type, $name, $id=0, $field='slug', $length=128) {
  $key = get_object_key($obj_type);
  
  $base_slug = $slug = slugify($name, $length);
  $i=1;
 
  $search = array($field => $slug);
  if($id) $search[$key] = array('<>', $id);
  
  while(find_object($obj_type, $search)) {
    $slug = $search[$field] = "$base_slug-$i";
    $i++;
  }
  return $slug;
}


function object_change_log($obj_type, $obj_or_id, $options = array()) {                 
  $id = any2id($obj_type, $obj_or_id);  
  $logObjs  = find_objects('changelog', array('parent_id' => $id, 'obj_type' => $obj_type));
  return $logObjs;                                                                            
}

function object_mail_log($obj_type, $obj_or_id, $options = array()) {
  global $u_level;
  $user_level = $u_level;
  
  $id = any2id($obj_type, $obj_or_id);  
  $search = ['parent_id' => $id, 'parent_type' => $obj_type];
  $mailObjs  = find_objects('mail_queue', $search);
  return $mailObjs;  
}


/** old: used for MDB2 */
// converts associate array of search terms to array of sql filters
function search2filters($array, $noquotes=false) {
  if(!$array) return;
  $filters = array();
  $operators = get_operators();
  if(is_object($array)) $array = (array) $array;

  if(!is_array($array) && ($_SESSION['dev'] || $_SESSION['test'])) {
    echo("Search2filters: arg not array");
    dump($array);
    $array = (array) $array;
  }
  foreach($array as $k => $v) {    
    $filter = $filter2 = '';
    if($k == DB_LOGICAL_OR && is_array($v) && count($v) >= 2) { 
      // can only be used once, and only for different keys like this - see below for use for same key
      // $search[DB_LOGICAL_OR] = array('city_id' => 1000, 'site_id' => 10); 
      $terms = search2filters($v);
      if(count($terms) >= 2) $filter= '('.implode(' OR ', $terms).')';
    // usage: $search[DB_LOGICAL_AND] = array('city_id' => ['>', 1000], 'city_id' => ['<', 2000]); 
    /** not tested */
    } elseif($k == DB_LOGICAL_AND && is_array($v) && count($v) >= 2) {      
      $terms = search2filters($v);
      if(count($terms) >= 2) $filter= '('.implode(' AND ', $terms).')';
    } elseif(is_array($v) && count($v) > 1) {
      $operator = $v[0];
      $val1 = is_array($v[1]) ? $v[1] : db_escape($v[1]);     
      $val2 = is_array($v[2]) ? $v[2] : db_escape($v[2]);     

      if(is_array($val1)) {
        // usage: $search['city'] = array(DB_LOGICAL_OR, array("*$city_id*", 0, ATT_DEFAULT_NULL, array('>', 1000)));
        if(($operator == DB_LOGICAL_OR || $operator == DB_LOGICAL_AND) && is_array($val1) && count($val1) >= 2) {
          $sep = $operator == DB_LOGICAL_OR ? " OR " : " AND ";
          $t = array();
          foreach($val1 as $v1) {
            $tf = search2filters(array($k => $v1), $noquotes);
            $t[] = count($tf) > 1 ? '('.implode(' AND ', $tf).')' : $tf[0];
          }
          $filter = '('.implode($sep, $t).')';
        } elseif($operator == '=') {
          $filter= "$k IN ('".implode("','", $val1)."')";    
        } elseif($operator == '!=' || $operator == '<>') {
          $filter= "$k NOT IN ('".implode("','", $val1)."')";    
        }
      } elseif($operator == '&') { // bitwise AND
        $filter= "$k & $val1";    
      } elseif($operator == '|') { // bitwise OR
        $filter= "$k | $val1";    
      } elseif($operator == '=' && $val1 === ATT_DEFAULT_NULL) {
        $filter= "$k IS NULL";
      } elseif($operator == '!=' && $val1 === ATT_DEFAULT_NULL) {
        $filter= "$k IS NOT NULL";
      } elseif($operator == '><' && strlen($val2)) {
        $filter= "$k >= '$val1'";
        $filter2= "$k <= '$val2'";
      } elseif($operator == DB_CONTAINS_OPERATOR) {
        $filter = "$k LIKE '%$val1%'";
      } elseif($operator == DB_BEGINS_WITH_OPERATOR) {
        $filter = "$k LIKE '$val1%'";
      } elseif($operator == DB_ENDS_WITH_OPERATOR) {
        $filter = "$k LIKE '%$val1'";
      } elseif(in_array($operator, $operators)) {
        $filter= "$k $operator '$val1'";
      } else { // operator not found, assume "IN"
        $filter= "$k IN ('".implode("','", $v)."')";    
      }
    } else {
      if(is_array($v)) $v = $v[0]; // array with only one element
      $val1 = db_escape($v);
      if(strpos($val1, '*') !== FALSE) { // allow wild cards (textual begins with, ends with, contains)
        $filter = "$k LIKE '".str_replace('*', '%', $val1)."'";
      } elseif($val1 === ATT_DEFAULT_NULL) {
        $filter= "$k IS NULL";
      } else {
        $filter= "$k = '$val1'";
      }
    }  
  
    $filters[] = $noquotes ? str_replace("'", '', $filter) : $filter; 
    if($filter2) $filters[] = $noquotes ? str_replace("'", '', $filter2) : $filter2; 
  }         
  return $filters;
}

function get_operators() {
  return array('>','>=','<','<=','<>','!=','=', '|', '&');
}

/** New PDO: converts associate array of search terms to array of PDO bound statments and variables */
function search2pdo($array, $level=0) {
  //dump($array);
  if(!$array) return;
  $filters = $values = array();
  $operators = get_operators();
  if(is_object($array)) $array = (array) $array;

  if(!is_array($array) && ($_SESSION['dev'] || $_SESSION['test'])) {
    echo("Search2filters: arg not array");
    dump($array);
    $array = (array) $array;
  }
  //dump($array);
  foreach($array as $k => $v) {
    //dump($k);
    $val = $subvals = array();
    $filter = array();
    $binding = ":".underscore2camel($k);
    $binding2 = ":".underscore2camel($k)."2";
    if($level > 0) {
      $binding .= "_".$level;
      $binding2 .= "_".$level;
    }
    
    $or = strpos($k, '$or') !== false || strpos($k, '$or2') !== false; // the latter allows for more than one, use $or1, $or2
    $and = strpos($k, '$and') !== false; // the latter allows fand mande than one, use $and1, $and2
    
    if($or && is_array($v) && count($v) >= 2) {  // $search['$or'] = array('city_id' => 1000, 'site_id' => 10); 
      list($terms, $subvals) = search2pdo($v, $level + 1);
      if(count($terms) >= 2) $filter[0] = '('.implode(' OR ', $terms).')';
    } elseif($and && is_array($v) && count($v) >= 2) {      
      list($terms, $subvals) = search2pdo($v, $level + 1);
      if(count($terms) >= 2) $filter[0] = '('.implode(' AND ', $terms).')';

    } elseif(is_array($v) && count($v) > 1) {
      $operator = $v[0];
      $val[0] = $v[1];     

      $or = strpos($operator, '$or') !== false;
      $and = strpos($operator, '$and') !== false;

      $like = !is_array($val[0]) && strpos($val[0], '*') !== FALSE; // LIKE operator
      
      if(is_array($val[0])) { 
        // usage: 
        // $search['city'] = array(DB_LOGICAL_OR, array("*$city_id*", 0, ATT_DEFAULT_NULL, array('>', 1000)));
        // $search['max_stay'] = ['$or', [0, ['>=', $nd]]];
        
        if(($or || $and) && count($val[0]) >= 2) {
          $sep = $or ? " OR " : " AND ";
          $t = array();
          $i = 0;
          foreach($val[0] as $v1) {
            list($tf, $sv) = search2pdo(array($k => $v1), $level + $i+ 1);
            
            if(count($sv)) $subvals = array_merge($subvals, $sv);
            $t[] = count($tf) > 1 ? '('.implode(' AND ', $tf).')' : $tf[0];
            $i++;
          }
          $filter[0] = '('.implode($sep, $t).')';
          $val = array(); // IN clause not handled well by PDO, so no prepared statement :(
        } elseif($operator == '=') {
          $filter[0] = "$k IN ('".implode("','", $val[0])."')";    
          $val = array(); // IN clause not handled well by PDO, so no prepared statement :(
        } elseif($operator == '!=' || $operator == '<>') { // IN clause not handled well by PDO, so no prepared statement :(
          $filter[0] = "$k NOT IN ('".implode("','", $val[0])."')";  
          $val = array(); // IN clause not handled well by PDO, so no prepared statement :(
        }
      } elseif($operator == '=' && $val[0] === ATT_DEFAULT_NULL) {
        $filter[0] = "$k IS NULL";
        $val = array(); // no value
      } elseif($operator == '!=' && $val[0] === ATT_DEFAULT_NULL) {
        $filter[0] = "$k IS NOT NULL";
        $val = array(); // no value
      } elseif($operator == '=' && $like) {
        $filter[0] = "$k LIKE {$binding}";
        $val[0] = str_replace('*', '%', $val[0]);
      } elseif($operator == '!=' && $like) {
        $filter[0] = "$k NOT LIKE {$binding}";
        $val[0] = str_replace('*', '%', $val[0]);
      } elseif($operator == '><' && count($v) > 2) {
        $filter[0] = "$k >= $binding";
        $filter[1] = "$k <= $binding2";
        $val[1] = $v[2];
      } elseif(in_array($operator, $operators)) {
        $filter[0] = "$k $operator $binding";
      } else { // operator not found, assume "IN"
        $filter[0] = "$k IN ('".implode("','", $v)."')";
        $val = array(); // IN clause not handled well by PDO, so no prepared statement :(
      }
    } else {
      if(is_array($v)) $v = $v[0]; // array with only one element
      $val[0] = $v;
      
      if(strpos($val[0], '*') !== FALSE) { // allow wild cards (textual begins with, ends with, contains)
        $filter[0] = "$k LIKE $binding";
        $val[0] = str_replace('*', '%', $val[0]);
      } elseif($val[0] === ATT_DEFAULT_NULL) {
        $filter[0] = "$k IS NULL";
        $val = array(); // no value
      } else {
        $filter[0] = "$k = $binding";
      }
    }  
    if(count($val)) $values[] = $val[0];
    if(count($val) > 1) $values[] = $val[1];
    if(count($subvals)) $values = array_merge($values, $subvals); 
      
    if(count($filter)) $filters[] = $filter[0];
    if(count($filter) > 1) $filters[] = $filter[1];
    //dump($filters);
  } 

  //foreach($values as $k => $v) if(!strlen($v)) $values[$k] = ''; // not null
  return array($filters, $values);
}


function object_site_search($obj_type, $search, $val, $options = []) {     
  switch($obj_type) {
    case 'minisite':
      if($val) $search[DB_LOGICAL_OR] = 
        array('minisite.id' => "*$val*", 
              'name' => "*$val*", 
              'domain' => "*$val*", 
              'url' => "*$val*");              
      break;
    case 'plugin':
      if($val) $search[DB_LOGICAL_OR] = 
        array('plugin.id' => "*$val*", 
              'title' => "*$val*", 
              'subtitle' => "*$val*");              
      break;
    case 'media':
      if($val) $search[DB_LOGICAL_OR] = 
        array('media.id' => "*$val*", 
              'name' => "*$val*",
              'filename' => "*$val*", 
              'tags' => "*$val*", 
              'comment' => "*$val*");              
      break;
    case 'media_folder':
      if($val) $search[DB_LOGICAL_OR] = 
        array('media_folder.id' => "*$val*", 
              'filename' => "*$val*", 
              'tags' => "*$val*");              
      break;
    case 'media_collection':
      if($val) $search[DB_LOGICAL_OR] = 
        array('media_collection.id' => "*$val*", 
              'title' => "*$val*");              
      break;
    case 'artwork':
      if($val) $search[DB_LOGICAL_OR] = 
        array('artwork.id' => "*$val*", 
              'title' => "*$val*", 
              'artist_name' => "*$val*",
              'short_description' => "*$val*",
              'primary_year' => "*$val*",
              'technique' => "*$val*");              
      break;
    case 'gallery':
      if($val) $search[DB_LOGICAL_OR] = 
        array('gallery.id' => "*$val*", 
              'title' => "*$val*", 
              'description' => "*$val*");              
      break;
    case 'dictionary':
      if($val) $search[DB_LOGICAL_OR] = 
        array('term' => "*$val*", 
              'phrase' => "*$val*");              
      break;
      break;
    case 'payment':
      if($val) $search[DB_LOGICAL_OR] = 
        array('payment.id' => "*$val*", 
              'res_id' => "*$val*", 
              'inv_id' => "*$val*", 
              'description' => "*$val*");              
      break;
    case 'reservation':
      if($val) $search[DB_LOGICAL_OR] = 
        array('reservation.res_id' => "*$val*", 
              'first_name' => "*$val*", 
              'last_name' => "*$val*", 
              'city_name' => "*$val*", 
              'apartment_name' => "*$val*");              
      break;
    case 'shortstay':
      if($val) $search[DB_LOGICAL_OR] = 
        array('name' => "*$val*", 
              'address_city' => "*$val*", 
              'city_name' => "*$val*", 
              'address_street' => "*$val*");
      break;
    case 'invoice':
      if($val) $search[DB_LOGICAL_OR] = 
        array('first_name' => "*$val*", 
              'last_name' => "*$val*", 
              'invoice.id' => "*$val*", 
              'parent_id' => "*$val*", 
              'invoice_id' => "*$val*", 
              'concerning' => "*$val*");              
      break;
    case 'inv_lineitem':
      if($val) $search['inv_id'] = $val;
      else json_error("Missing invoice ID");
      break;
    case 'user':
      $tbl1 = get_object_table('user');
      $tbl2 = get_object_table('user_profile');
      if($val) $search[DB_LOGICAL_OR] = 
        array("{$tbl2}.first_name" => "*$val*", 
              "{$tbl2}.last_name" => "*$val*", 
              "{$tbl2}.display_name" => "*$val*", 
              "{$tbl2}.email_address" => "*$val*", 
              "{$tbl1}.id" => "$val*", 
              "{$tbl1}.username" => "*$val*");              
      break;
    default:
      break;
  }

  if($filters = $options['filters']) {
    $filters = json_decode($filters, true);
    if(is_array($filters)) {
      foreach($filters as $k => $v) $search[$k] = $v;
    }          
  }

  if($ids = $options['id']) { // limit to selected IDs
    $search['id'] = $ids;
  }
  
  print_log("get-rows {$obj_type} GET: ".dump($_GET, true), 'debug', LOG_LEVEL_TEST);
  print_log("get-rows {$obj_type} filters: ".dump($filters, true), 'debug', LOG_LEVEL_TEST);
  print_log("get-rows {$obj_type} search: ".dump($search, true), 'debug', LOG_LEVEL_TEST);

  $key = get_object_key($obj_type);      
  if(!$options['sort']) $options['sort'] = get_object_sort($obj_type) ?: "{$key} desc";

  $objs = find_objects($obj_type, $search, $options);

  $count = count($objs);
  print_log("get-rows $obj_type val=$val count=$count", 'debug', LOG_LEVEL_TEST);
  //print_log("First:".dump($objs[0], true), 'debug', LOG_LEVEL_TEST);
  
  $extra_count = 0;
  /** post processing - expensive */
  switch($obj_type) {
  case 'user':
    if($siteObj->property_site && is_class('property')){
      foreach($objs as &$obj) {
        $obj->rentals = count_objects('property', array('host_id' => $obj->id));
        $obj->listed_rentals = count_objects('property', array('host_id' => $obj->id, 'active' => 1));
      }
    }
    break;
  case 'gallery': /** look for media in this folder */
    $folder_id = $search['parent_id'] ?: 0;
    $mediaObjs = find_objects('media', ['active' => 1, 'parent_type' => 'gallery', 'parent_id' => $folder_id, 'parent_field' => 'media']);
    $extra_count = count($mediaObjs);
    foreach($mediaObjs as $mediaObj) {
      $objs[] = ['obj_type' => 'media', 'id' => $mediaObj->id, 'created' => $mediaObj->created, 'type' => $mediaObj->type, 'filename' => $mediaObj->src,
      'title' => $mediaObj->name, 'user_id' => $mediaObj->user_id, 'path' => $mediaObj->path, 'src' => $mediaObj->src];
    }
    break;
  default:
    break;
  }
  $result = [];
  if(!$options['associative']) return array($objs, $extra_count);
  
  foreach($objs as $obj) {
    $id = get_object_id($obj_type, $obj);
    $result[$id] = $obj;
  }

  return $result;
}

// For Drop down filters in artwork and exhibition table
function object_site_filter($obj_type, $search, $val, $options = [], $filter_data) {  
  
  $arr = [];
  $col = json_decode($filter_data, True);
  $count = count($col);
  
  switch($obj_type) {
    
    case 'artwork':
      foreach($col as $key=>$val){
        $arr['artwork.active'] = "1";
        $arr[$key] = "$val";
      }         
      $search[DB_LOGICAL_AND] = $arr;          
    break;

    case 'media_collection':
      $arr['media_collection.filter'] = "1";
      foreach($col as $key=>$val){
        if(($count == 1) && ($key == subtype)){
          $arr['subtype'] = "$val";
        }elseif(($count == 1) && ($key == active)){
          $arr['active'] = "$val";
        }else{
          $arr[$key] = "$val";
        }        
      }         
      $search[DB_LOGICAL_AND] = $arr;
    break;
    
    default:
      break;
  }
  //dump($search);
  if($filters = $options['filters']) {
    $filters = json_decode($filters, true);
    if(is_array($filters)) {
      foreach($filters as $k => $v) $search[$k] = $v;
    }          
  }

  if($ids = $options['id']) { // limit to selected IDs
    $search['id'] = $ids;
  }
  
  print_log("get-rows {$obj_type} GET: ".dump($_GET, true), 'debug', LOG_LEVEL_TEST);
  print_log("get-rows {$obj_type} filters: ".dump($filters, true), 'debug', LOG_LEVEL_TEST);
  print_log("get-rows {$obj_type} search: ".dump($search, true), 'debug', LOG_LEVEL_TEST);
  
  $key = get_object_key($obj_type);      
  if(!$options['sort']) $options['sort'] = get_object_sort($obj_type) ?: "{$key} desc";
  
  $objs = find_objects($obj_type, $search, $options);

  $count = count($objs);
  print_log("get-rows $obj_type val=$val count=$count", 'debug', LOG_LEVEL_TEST);
  //print_log("First:".dump($objs[0], true), 'debug', LOG_LEVEL_TEST);
  
  $result = [];
  if(!$options['associative']) return $objs;
  
  foreach($objs as $obj) {
    $id = get_object_id($obj_type, $obj);
    $result[$id] = $obj;
  }
  return $result;
}
      

/**

class db
{
	public function find() {
	  return $this;
	}

	public function limit() {
	  return $this;
	}

	public function sort() {
	  return $this;
	}

	public function order() {
	  return $this;
	}
}
*/

function find_page_menus($obj_id, $user_id, $minisite_id, $slug){

  $val = find_object('minisite_page', ['user_id' => $user_id, 'minisite_id' => $minisite_id,'slug' => $slug, 'active' => 1]);
  if($val->type == '30'){
    $data = json_decode($val->data, True);
    if($data[show_gallery])  $gallery = '10';
    if($data[show_artfair])  $artfair = '20';
    if($data[show_others])   $others = '30'; 
    if($data[show_current])  $current = 1;
    if($data[show_past])     $past = 1;
    if($data[show_future])   $future = 1;

    $search = ['user_id' => $user_id, 'type' => '120', 'active' => 1, 'subtype' => [$gallery,$artfair,$others] ];
    $today = today(); 
    if($current && $future && past){
      $search['start_date'] = ['<=', $today];
      $search['end_date'] = ['>=', $today]; 
    }
    elseif($current && $future){
      $search['start_date'] = ['<=', $today];
      $search['end_date'] = ['>=', $today]; 
    }elseif($current && $past){
      $search['start_date'] = ['<=', $today];
      $search['end_date'] = ['>=', $today]; 
    }elseif($future && $past){
      $search['start_date'] = ['>', $today];
      unset($search['end_date']);
    }
    elseif($current){
      $search['start_date'] = ['<=', $today];
      $search['end_date'] = ['>=', $today]; 
    }elseif($future){
      $search['start_date'] = ['>', $today];
      unset($search['end_date']);
    }elseif($past){
      unset($search['start_date']);
      $search['end_date'] = ['<', $today];
    }

    $exhibitionObjs = find_objects('media_collection', $search, ['sort' => 'start_date asc']) ?: [];
    if($exhibitionObjs) $get_data = loop_exhibition($exhibitionObjs);
    
    if(!$get_data){
      if($current && $future && past){
        $search['start_date'] = ['>', $today];
        unset($search['end_date']);
      }
      elseif($current && $future){
        $search['start_date'] = ['>', $today];
        unset($search['end_date']);
      }elseif($current && $past){
        unset($search['start_date']);
        $search['end_date'] = ['<', $today];
      }elseif($future && $past){
        unset($search['end_date']);
        $search['end_date'] = ['<', $today];
      }
      elseif($current){
        $search['start_date'] = ['<=', $today];
        $search['end_date'] = ['>=', $today]; 
      }elseif($future){
        $search['start_date'] = ['>', $today];
        unset($search['end_date']);
      }elseif($past){
        unset($search['start_date']);
        $search['end_date'] = ['<', $today];
      }
      $exhibitionObjs = find_objects('media_collection', $search) ?: [];
      if($exhibitionObjs) $get_data = loop_exhibition($exhibitionObjs);
    }
    if($get_data){
      $datas = array_merge($data, $get_data);
      return $datas;
    }    
  }else{
    return 0;
  }
  
}

function loop_exhibition($exhibitionObjs){
  $today = today(); 
  $exhibition_ids = $exhibitions = $latestexhibition = [];
  $current_exist = $future_exist = $past_exist = 0;
  foreach($exhibitionObjs as $exhibitionObj) {
    $exhibitionObj->slug = slugify($exhibitionObj->title);
    $current = "";
    if($exhibitionObj->start_date && $exhibitionObj->end_date) {
      if($exhibitionObj->end_date < $today) {
        $current = "past";
        $past_exist = 1;
      }elseif($exhibitionObj->start_date > $today) {
        $current =  "future";
        $future_exist = 1;
      }else {
        $current = "current";
        $current_exist = 1 ;
      }
    } else {
      continue; /** skip exhibitions w/o dates */
    }
    $exhibitionObj->current = $current;   
    $exhibitionObj->page_slug = $page_slug; 
    $exhibitions[$exhibitionObj->slug] = $exhibitionObj;
    $exhibition_ids[] = $exhibitionObj->id;
  }
  //dump($past_exist.' '.$future_exist.' '.$current_exist);
  if($future_exist) $pre['future'] = 1;
  elseif($current_exist) $pre['current'] = 1;
  elseif($past_exist) $pre['past'] = 1;
  else{
    if($data['show_latest'] == 1)
      $pre['latest'] = 1;  
  }
  return $pre;
}

function update_artist_name($user_id, $artist_name){
  $find_artworks = find_objects('artwork', ['artist_id' => $user_id, 'active' => 1]);
  foreach($find_artworks as $artwork){
    $artwork_id = $artwork->id;
    $artwork->artist_name = $artist_name;
    list($id, $errors) = update_object('artwork', $artwork, $artwork_id);  
  }
  if($errors) json_error($errors);
  else return 1;
}

function get_template($parts, $minisite_id){
  $tpl_values = [];
  $parts1_tpls = find_objects('minisite_tpl', ['parts' => 1]);
  $parts2_tpls = find_objects('minisite_tpl', ['parts' => 2]);

  if($parts){ 
    $slug = '/'.$parts[0].'/';                                                    //to simplify paths
    $page = find_object('minisite_page', ['minisite_id' => $minisite_id, 'slug' => $slug, 'active' => 1]); 
    if(count($parts) == 1){
      foreach($parts1_tpls as $parts1_tpl){
        if($page->type == $parts1_tpl->type){
          $tpl_values['page'] = $parts1_tpl->template;
          $tpl_values['tpl'] = $parts1_tpl->page;
        }
      }
    }else{
      foreach($parts2_tpls as $parts2_tpl){
        if($page->type == $parts2_tpl->type){
          if($page->type == 30){
            if($parts[1] == 'exhibitions'){
              $tpl_values['page'] = 'exhibitions';
              $tpl_values['tpl'] = 'exhibitions';
            }else{
              $tpl_values['page'] = 'exhibition';
              $tpl_values['tpl'] = 'exhibition';
            }
          }elseif($page->type == 80){
            if($parts[1] == 'manualexbs'){
              $tpl_values['page'] = 'manualexbs';
              $tpl_values['tpl'] = 'manualexbs';
            }else{
              $tpl_values['page'] = 'manualexb';
              $tpl_values['tpl'] = 'manualexb';
            }
          }else{
            $tpl_values['page'] = $parts2_tpl->template;
            $tpl_values['tpl'] = $parts2_tpl->page;
          }
        }
      }
    }
  }else{
    $get_home = find_object('minisite_page', ['minisite_id' => $minisite_id, 'row_order' => 1, 'active' => 1]);
    foreach($parts1_tpls as $parts1_tpl){
      if($get_home->type == $parts1_tpl->type){
        $tpl_values['page'] = $parts1_tpl->template;
        $tpl_values['tpl'] = $parts1_tpl->page;
        $tpl_values['get_slug'] = str_replace('/', '', str_replace('/', '', $get_home->slug));
      }
    }
  }
  return $tpl_values;
}

function get_collection($exhibitions, $page_slug){
  $data = [];
  foreach($exhibitions as $collectionObj){
    $collectionObj->page_slug = $page_slug;
    $collectionObj->slug = slugify($collectionObj->title);
    $data[] = $collectionObj;
  }
  return $data;
}

function get_rowOrder($id){
  $get_row = find_objects('minisite_page', ['minisite_id' => $id, 'active' => 1]);
  $row_order = count($get_row)+1;
  return $row_order;
}

function get_exhibitionJson(){
  $search = [];
  $search['show_recent'] = 1;
  $search['show_past'] = 1;
  $search['show_current'] = 1;
  $search['show_future'] = 1;
  $search['show_latest'] = 1;
  $search['show_gallery'] = 1;
  $search['show_artfair'] = 1;
  $search['show_others'] = 1;
  return (json_encode($search));
}

function get_sExhibitionJson(){
  $search = [];
  $search[eType] = 1;
  $search[gallery] = 10;
  $search[artfair] = 20;
  $search[others] = 30;
  return (json_encode($search));
}

function get_artwork_type($type){
  if($type == 10) $text = "Photography";
  elseif($type == 11) $text = "Print";
  elseif($type == 20) $text = "Painting";
  elseif($type == 21) $text = "Drawing";
  elseif($type == 22) $text = "Mixed Media on paper/canvas";
  elseif($type == 30) $text = "Sculpture";
  elseif($type == 40) $text = "Cast (Bronze, etc)";
  elseif($type == 50) $text = "Film & Video";
  elseif($type == 60) $text = "Installation";
  elseif($type == 70) $text = "Performance";
  elseif($type == 99) $text = "Other";

  return $text;
}

function get_layout($type, $subtype){
  $data = find_object('minisite_layout', ['type' => $type, 'subtype' => $subtype ,'active' => 1, 'row_order' => 1]);
  return $data->id;
}

function get_artist($user_id, $artist_ids, $page_slug){
    /** Artists - only do when needed */  
    foreach($artist_ids as $artist_id) {
      $uObj = get_user($artist_id);    
      
      $colSearch = ['user_id' => $user_id, 'artist_id' => $artist_id, 'active' => 1, 'type' => COLLECTION_TYPE_EXHIBITION];
      $collectionObj = find_object('media_collection', $colSearch);

      print_log("artist {$uObj->display_name} ({$artist_id}) id={$collectionObj->id} type={$collectionObj->type} title={$collectionObj->title}", 'debug', LOG_LEVEL_TEST);
      
      if(!$collectionObj) {         
        //echo("{$uObj->display_name} has no projects<br>");
        //continue; /** Show atleast biography */
      }
      $uObj->collection = $collectionObj;
      $uObj->collection_id = $collectionObj->id;
      $uObj->slug = $this_slug = slugify($uObj->display_name);
      $uObj->page_slug = $page_slug;
      
      $uObj->title_image = $title_image = $uObj->photo ?: collection_title_image($collectionObj, 'ig');        
      //$uObj->title_image = collection_title_image($collectionObj, 'ig') ?: $uObj->photo; /** (David asked for this */

      print_log("{$this_slug} ({$artist_id}) u_photo: {$uObj->photo} title: {$title_image} collection: {$collectionObj->title} ({$collectionObj->id})", "minisite", LOG_LEVEL_TEST);
      
      $artistAr[] = $uObj;
    }
    return $artistAr;
}

function related_images_type($type){
  if($type == 10)      $val= 'Blank - leave blank';
  elseif($type == 20)  $val= 'Detail';
  elseif($type == 30)  $val= 'Installation';
  elseif($type == 40)  $val= 'Studio';
  elseif($type == 50)  $val= 'Inspiration';
  elseif($type == 60)  $val= 'Influences';
  elseif($type == 70)  $val= 'Event';
  return $val;
}

function sortMonths($a, $b) {
  if ($a->year == $b->year)     return 0;
  list($ay,$am,$ad) = explode('-', $a->year);
  list($by,$bm,$bd) = explode('-', $b->year);
  if ($ay == $by){
    $am = strtotime($am); 
    $bm = strtotime($bm);
    return ($am < $bm ? 1 : -1);
  }
  else
    return ($a->year < $b->year ? 1 : -1);
}

function get_page_type($type_id){
  $page_types = get_objects('minisite_page_type');
  
  foreach($page_types as $page_type){
    if ($type_id == $page_type->id){
      $type[tab] = $page_type->tab;
      $type[page_type] = $page_type->name;
    }
  }
  return $type;
}

function get_exhibition_tpl1($layout_id){
  if($layout_id == '101')       $template = 'timeline_view';          
  elseif($layout_id == '103')   $template = 'masonry_3C';          
  elseif($layout_id == '104')   $template = 'masonry_4C';          
  elseif($layout_id == '105')   $template = 'masonry_3C_TS';          
  elseif($layout_id == '106')  $template = 'masonry_4C_TS';
  elseif($layout_id == '137')  $template = 'masonry_3CY';
  elseif($layout_id == '138')  $template = 'masonry_4CY';

  elseif($layout_id == '186')  $template = 'landscape_2C';          
  elseif($layout_id == '187')  $template = 'landscape_2C_TS';          
  elseif($layout_id == '188')  $template = 'landscape_2CY';          
  elseif($layout_id == '107')  $template = 'landscape_3C';          
  elseif($layout_id == '108')  $template = 'landscape_4C';          
  elseif($layout_id == '109')  $template = 'landscape_3C_TS';          
  elseif($layout_id == '110')  $template = 'landscape_time'; //landscape_4C_TS 
  elseif($layout_id == '139')  $template = 'landscape_3CY';
  elseif($layout_id == '140')  $template = 'landscape_4CY';

  elseif($layout_id == '111')  $template = 'natural';      //Hanging Line - 3 Column
  elseif($layout_id == '112')  $template = 'tiles';        //Hanging Line - 4 Column
  elseif($layout_id == '113')  $template = 'hangingline_3C_TS';          
  elseif($layout_id == '114')  $template = 'natural_time';    //hangingline_4C_TS 
  elseif($layout_id == '141')  $template = 'hangingline_3CY';
  elseif($layout_id == '142')  $template = 'hangingline_4CY';

  elseif($layout_id == '180')  $template = '16ratio9_2C';
  elseif($layout_id == '181')  $template = '16ratio9_2CT';
  elseif($layout_id == '182')  $template = '16ratio9_2CY';
  elseif($layout_id == '143')  $template = '16ratio9_3C';
  elseif($layout_id == '144')  $template = '16ratio9_4C';
  elseif($layout_id == '145')  $template = '16ratio9_3CT';
  elseif($layout_id == '146')  $template = '16ratio9_4CT';
  elseif($layout_id == '147')  $template = '16ratio9_3CY';
  elseif($layout_id == '148')  $template = '16ratio9_4CY';

  elseif($layout_id == '183')  $template = '4ratio3_2C';
  elseif($layout_id == '185')  $template = '4ratio3_2CY';
  elseif($layout_id == '184')  $template = '4ratio3_2CT';
  elseif($layout_id == '149')  $template = '4ratio3_3C';
  elseif($layout_id == '150')  $template = '4ratio3_4C';
  elseif($layout_id == '151')  $template = '4ratio3_3CT';
  elseif($layout_id == '152')  $template = '4ratio3_4CT';
  elseif($layout_id == '153')  $template = '4ratio3_3CY';
  elseif($layout_id == '154')  $template = '4ratio3_4CY';

  elseif($layout_id == '158')  $template = 'standing_3C';
  elseif($layout_id == '159')  $template = 'standing_4C';
  elseif($layout_id == '160')  $template = 'standing_3CT';          
  elseif($layout_id == '161')  $template = 'standing_4CT';
  elseif($layout_id == '162')  $template = 'standing_3CY';
  elseif($layout_id == '163')  $template = 'standing_4CY';

  elseif($layout_id == '115')  $template = 'coverpage'; 
  elseif($layout_id == '116')  $template = 'coverpage_landscape';   

  elseif($layout_id == '189')  $template = '3ratio2_2C';
  elseif($layout_id == '190')  $template = '3ratio2_2CT';
  elseif($layout_id == '191')  $template = '3ratio2_2CY';
  elseif($layout_id == '192')  $template = '3ratio2_3C';
  elseif($layout_id == '193')  $template = '3ratio2_3CT';
  elseif($layout_id == '194')  $template = '3ratio2_3CY';
  elseif($layout_id == '195')  $template = '3ratio2_4C';
  elseif($layout_id == '196')  $template = '3ratio2_4CT';
  elseif($layout_id == '197')  $template = '3ratio2_4CY'; 

  elseif($layout_id == '178')  $template = 'banner';
  elseif($layout_id == '176')  $template = 'banner_time';
  elseif($layout_id == '177')  $template = 'banner_year';
  
  elseif($layout_id == 198 || $layout_id == 222 || $layout_id == 239 || $layout_id == 263 || $layout_id == 279)  
    $template = '16X9_2C'; //balti
  elseif($layout_id == 199 || $layout_id == 223 || $layout_id == 240 || $layout_id == 264 || $layout_id == 280)  
    $template = '16X9_3C';
  elseif($layout_id == 200 || $layout_id == 224 || $layout_id == 241 || $layout_id == 265 || $layout_id == 281)  
    $template = '16X9_4C';
  elseif($layout_id == 201 || $layout_id == 225 || $layout_id == 242 || $layout_id == 266 || $layout_id == 282)  
    $template = '4X3_2C'; //casa
  elseif($layout_id == 202 || $layout_id == 226 || $layout_id == 243 || $layout_id == 267 || $layout_id == 283)  
    $template = '4X3_3C';
  elseif($layout_id == 203 || $layout_id == 227 || $layout_id == 244 || $layout_id == 268 || $layout_id == 284)  
    $template = '4X3_4C';
  elseif($layout_id == 204 || $layout_id == 228 || $layout_id == 245 || $layout_id == 269 || $layout_id == 285)  
    $template = '3X2_2C'; //nor
  elseif($layout_id == 205 || $layout_id == 229 || $layout_id == 246 || $layout_id == 270 || $layout_id == 286)  
    $template = '3X2_3C';
  elseif($layout_id == 206 || $layout_id == 230 || $layout_id == 247 || $layout_id == 271 || $layout_id == 287)  
    $template = '3X2_4C';

  elseif($layout_id == 212 || $layout_id == 232 || $layout_id == 251 || $layout_id == 273 || $layout_id == 289)  
    $template = 'niagra_1'; //casa
  elseif($layout_id == 213 || $layout_id == 233 || $layout_id == 254 || $layout_id == 274 || $layout_id == 290)  
    $template = 'niagra_2'; //nor
  elseif($layout_id == 214 || $layout_id == 231 || $layout_id == 248 || $layout_id == 272 || $layout_id == 288)  
    $template = 'niagra_3'; //balti
  
  elseif($layout_id == '215')  $template = 'parallax_1';

  elseif($layout_id == 216 || $layout_id == 235 || $layout_id == 258)  $template = 'granada_1'; //casa
  elseif($layout_id == 217 || $layout_id == 236 || $layout_id == 259)  $template = 'granada_2';
  elseif($layout_id == 218 || $layout_id == 234 || $layout_id == 257)  $template = 'granada_3';
  elseif($layout_id == 219 || $layout_id == 237 || $layout_id == 260)  $template = 'granada_4';

  elseif($layout_id == 220 || $layout_id == 238 || $layout_id == 261)  $template = 'zig-zag';

  elseif($layout_id == 262)  $template = 'life';
  elseif($layout_id == 221)  $template = 'parallax';
  elseif($layout_id == 275)  $template = 'g_baltimore';
  elseif($layout_id == 276)  $template = 'g_casablanca';
  elseif($layout_id == 277)  $template = 'g_normandy';
  elseif($layout_id == 278)  $template = 'g_combo';
  
  elseif($layout_id == 291)  $template = 'niagara_169';
  elseif($layout_id == 292)  $template = 'niagara_43';
  elseif($layout_id == 293)  $template = 'niagara_32';

  return $template;
}

function get_exhibition_tpl2($layout_id){
  if($layout_id == '132')      $template = 'exhibition';
  elseif($layout_id == '133')  $template = 'exhibition1';
  elseif($layout_id == '168')  $template = 'exh_masonry';
  elseif($layout_id == '169')  $template = 'exh_hanging'; 
  elseif($layout_id == '170')  $template = 'exh_standing'; 
  elseif($layout_id == '171')  $template = 'exh_masonryExt'; 
  elseif($layout_id == '210')  $template = 'kyoto_WLoc'; 
  elseif($layout_id == '211')  $template = 'seoul_WLoc'; 

  return $template;
}

function add_manual_exb($ids, $page_id){
  $ids = explode(',',$ids);
  
  $minisitePage = get_object('minisite_page', $page_id);
  foreach($ids as $id){
    $get_media = find_object('media_collection', ['id'=>$id, 'active'=>1]);
    if($get_media){
      $duplicate = find_object('exhibition_map', ['media_collection_id'=>$id, 'page_id'=>$page_id, 'active'=>1]);
      if(!$duplicate){
        $data['media_collection_id'] = $id;
        $data['page_id'] = $page_id;
        $data['media_id'] = $get_media->title_image;
        $data['active'] = 1;
        $data['user_id'] = $minisitePage->user_id;
        $data['name'] = $get_media->title;
        $data['type'] = 'manual';
        $data['page_id'] = $page_id;
        $data['curator_id'] =  $get_media->curator_id;
        list($newid, $errors) = add_object('exhibition_map', $data);
        $obj[] = get_object('exhibition_map', $newid);
      }else{
        $error = array_merge_smart("Failed - Exhibition already added to the list", $errors);
      }
    }else{
        $error = array_merge_smart("Failed - Exhibition is not published", $errors);
    }
  }
  if($error) return json_error($error);
  return $obj;
}

function add_single_exb($exhibition_ids, $page_id){
  $exhibition_id = explode(',',$exhibition_ids);
  $length = count($exhibition_id);
  $changed = 'exb';
  if(count($exhibition_id)>1) 
    $error = array_merge_smart("Failed to add multiple exhibitions", $errors);

  $get_media = find_object('media_collection', ['id'=> $exhibition_id[0], 'active'=> 1]);
  if(($get_media) && ($get_media->title_image)){
    $get_collection = find_object('exhibition_map', ['page_id'=> $page_id, 'active' =>1]);
    if($get_collection){
      $collection_id = $get_collection->id;
      $get_collection->media_collection_id = $get_media->id;
      $get_collection->media_id = $get_media->title_image;
      $get_collection->artist_id = $get_media->artist_id;
      $get_collection->name = $get_media->title;
      list($id, $errors) = update_object('exhibition_map', $get_collection, $collection_id);
    }else{
      $data = [];
      $data['media_collection_id'] =  $get_media->id;
      $data['media_id']            =  $get_media->title_image;
      $data['user_id']             =  $user_id;
      $data['site_id']             =  $get_media->site_id;
      $data['artist_id']           =  $get_media->artist_id;
      $data['name']                =  $get_media->title;
      $data['curator_id']          =  $get_media->curator_id;
      $data['type']                =  'single';
      $data['page_id']             =  $page_id;
      list($id, $errors) = add_object('exhibition_map', $data);
      if($errors) json_error($errors);
    }
  }else
    $error = "Failed to add exhibition - Missing Media";
  
  switch($changed) { // reload
    case 'exb':
      $art = find_object('exhibition_map', ['active' => 1, 'page_id' => ['>', 0]]);
      break;           
  }
  if($error) return json_error($error);
  $obj = get_object('exhibition_map', $id);
  return $obj;
}

function create_ngnix(){
  $filename = "./etc/ngnix/sites-enabled/testfile.txt";
  $txt = "test content";
  $myfile = fopen($filename, "w") or die("Unable to open file!");
  fwrite($myfile, $txt);
  fclose($myfile);
}



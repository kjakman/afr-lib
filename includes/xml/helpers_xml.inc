<?php

// returns an array of elements in the $input array where $key contains $target
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_find($input, $key, $target) {
   $ar = array();
   foreach ($input as $a) {
      if(is_integer(strpos($a->$key->data,$target))) {
         array_push($ar, $a);
      }
   }
   //echo("array_find: Found ".sizeof($ar)." elements in array where $key contains $target<br>");
   return $ar;
}

// returns an array of elements in the $input array where $key == $target
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_find_exact($input, $key, $target) {
   $ar = array();
   foreach ($input as $a) {
      //echo("<br>looking for $target. Elements $key is ".$a->$key->data);
      if($a->$key->data==$target) {
         array_push($ar, $a);
      }
   }
   //echo("array_find_exact: Found ".sizeof($ar)." elements in array where $key == $target<br>");
   return $ar;
}

// returns the first elements in the $input array where $key == $target
function array_find_one($input, $key, $target) {
   foreach ($input as $a) {
      if($a->$key->data==$target) {
         return $a;
      }
   }
   return null;
}

// returns an array of elements in $input where $key is found in array $targets
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_find_many($input, $key, $targets) {
   $ar = array();
   if(!is_array($targets)) $targets = array($targets);
   //$target_list = implode(",", $targets);
   foreach ($input as $a) {
      //echo("<br>looking for $target. Elements $key is ".$a->$key->data);
      if(in_array($a->$key->data, $targets)) {
         array_push($ar, $a);
      }
   }
   //echo("array_find_many: Found ".sizeof($ar)." elements in array of size ".sizeof($input)." where $key is in $target_list (".sizeof($targets).")<br>");
   return $ar;
}


/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_localized($input) {
   $size = sizeof($input);
   //print_debug("array_localized: input[$size]");
   return array_find_attribute($input, "LANG", $_SESSION['language']);
}

// takes array, returns element with matching attribute
function array_find_attribute($input, $attribute, $target) {
   foreach($input as $inp) {
      $attr = $inp->attr[$attribute];
      if($attr == $target) {
         //if($_SESSION['debug']) {print_debug("array_find_attribute: Found attr = $attr with target=$target<br>");}
         return $inp;
      }
   }
   //if($_SESSION['debug']) {print_debug("array_find_attribute: element with attribute $attribute=$target not found, returning first element");}
   //return $input[0]; // default if attr=target not found
   return null;
}

// takes array, returns array of data (or key->data if key is specified)
function array_data($input, $key="", $len=0) {
   $ar = array();
   if(!is_array($input)) $input = array($input);
   foreach ($input as $a) {
      $val = ($key ? $a->$key->data : $a->data);
      $val = ($len ? substr($val, 0, $len): $val);
      array_push($ar, $val);
   }
   return $ar;
}

// takes array of arrays, returns array of data (or key->data if key is specified) from the array with current language
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_data_localized($input, $key="", $len=0) {
   $size = sizeof($input);
   if($_SESSION['debug']) {print_debug("array_data_localized: input has $size elements, key=$key len=$len");}
   $input = array_find_attribute($input, "LANG", $_SESSION['language']);
   return array_data($input, $key, $len);
}
// takes array of elements, returns data of element with current langugae
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_language_data($input, $lang="") {
   $lang = ($lang ? $lang : $_SESSION['language']);
   $el = array_find_attribute($input, "LANG", $lang);
   return $el->data;
}

// takes array of elements, returns array of element->NAME->data
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_names($arr, $len=0) {
   return array_data($arr, "NAME", $len);
}
/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_names_localized($input, $len=0) {
   $ar = array();
   foreach ($input as $a) {
      $a = array_find_attribute($a->NAME, "LANG", $_SESSION['language']);
      $val = $a->data;
      $val = ($len ? substr($val, 0, $len): $val);
      array_push($ar, $val);
   }
   return $ar;
}

/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_shortnames($arr, $len=3) {
   return array_data($arr, "SHORTNAME", $len);
}

/** unsused - automatic comment added on  */
function getObjId($a) {
  return $a->id;
}

/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function array_ids($arr) {
  return array_map("getObjId", $arr);
}

/** unsused - automatic comment added on  */
function xml_remove_empty_tags($xml) {
  $lines = explode("\n", $xml);
  
  $n = sizeof($lines);
  $c = strlen($xml);
  echo("xml contains $n lines - xml is $c chars<br>");
  
  for($i=0; $i<$n; $i++) {
    if(strpos($lines[$i], "></")) unset($lines[$i]); 
  }
  //$lines = array_filter($lines);
  $n = sizeof($lines);
  $xml = implode("\n", $lines);  
  $c = strlen($xml);
  $xml = str_replace("\t", "", $xml);
  $xml = str_replace("  ", " ", $xml);
  $xml = str_replace("\n\n", "\n", $xml);
  echo("xml now contains $n lines - xml is $c chars<br>");
  return $xml;  
}

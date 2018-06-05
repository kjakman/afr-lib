<?php
/**
 * Delete a file or recursively delete a directory
 *
 * @param string $str Path to file or directory
 */
function recursiveDelete($str) {
  $str = trim($str);
  if(!$str || $str == '/') mydie("recursiveDelete called with '$str' - too dangerous - exiting");
  if(is_file($str)) return @unlink($str);
  if(is_dir($str)){
    $scan = glob(rtrim($str,'/').'/*');
    foreach($scan as $index=>$path) recursiveDelete($path);
    return @rmdir($str);
  }
}

/** get array of arrays of all included files, divided in user/vendor/all */
function included_files() {
  $files = get_included_files();
  $root = approot();
  $ar = ['user' => [], 'vendor' => [], 'all' => $files];
  foreach($files as $file) {
    if(strpos($file, $root) === 0) {
      $ar['user'][] = $file;
    } else {
      $ar['vendor'][] = $file;
    }
  }
  return $ar;
}
 
/** return all defined functions */
function user_defined_functions() {
  $files = included_files();
  $user_files = $files['user'];
  $ar = [];
  foreach($user_files as $user_file) {
    
    $functions = get_functions_in_file($user_file, true);
    $content = file_get_contents($user_file);
    if(!$content) die("$path was blank");
    
    $functions = file_functions($content);
    foreach($functions as $function) $ar[$function] = $user_file;
    //echo("Funtions in {$user_file}:".dump($functions, true));
  }
  //$arr = get_defined_functions();
  return $ar;
  
}

/** like get_defined_functions, but user functions are split into our own and vendor functions */
function defined_functions() {
  $user_functions = array_keys(user_defined_functions());

  //dump($user_functions);die();
  $arr = get_defined_functions();
  $internal = $arr['internal'];
  $user = $arr['user'];
  $arr['vendor'] = [];
  $arr['user'] = [];
  //unset($arr['internal']);
  foreach($user as $function) {
    if(in_array_case_insenitive($function, $user_functions)) { /** get defined functions returns all lower case */
      $arr['user'][] = $function;
    } else {
      $arr['vendor'][] = $function;      
    }
  }
  return $arr;
}


/** not working... */
// https://gist.github.com/tonylegrone/8742453
function get_functions_in_file($file, $sort = FALSE) {
  $file = file($file);
  $functions = array();
  foreach ($file as $line) {
    $line = trim($line);
    if (stripos($line, ' function ') !== false) {
      $function_name = str_ireplace([
        'public', 'private', 'protected',
        'static'
          ], '', $line);

      if (!in_array($function_name, ['__construct', '__destruct'])) {
        $functions[] = trim(substr($function_name, 9, strpos($function_name, '(') - 9));
      }
    }
  }
  if ($sort) {
    asort($functions);
    $functions = array_values($functions);
  }
  return $functions;
}

/** return undefined function calls in all included user files */
function print_undefined_functions_calls() {
  //require_once("ajax.php");
  //require_once("admin/cronscripts.php");
  require_once("payment/db_withdraw.inc");
  //require_once("golfswitch.inc");
  require_once("xml/xml_airbnb.inc");
  require_once("classes/sms_api.class.inc");
  require_once("xml/xml_cloudinary.inc");
  require_once("Smarty/Smarty.class.php");    
  require_once("paypal/do_refund.inc");
  require_once("paypal/paypal.inc");
  require_once("xml/xml_interhome.inc");
  require_once("services/helpers_kigo.inc");
  require_once("bower/jquery-file-upload/php/UploadHandler.php");
  //require_once("jquery/file-uploader-3.3/server/php/qqFileUploader.php");

  $files = included_files();
  $user_files = $files['user'];
  $app_root = approot(false);
  $user_files[] = "{$app_root}/ajax.php";
  $user_files[] = "{$app_root}/admin/cronscripts.php";
  foreach($user_files as $file) {
    $functions = undefined_function_calls($file);
    $count = count($functions);
    $output .= html_h2("{$count} calls to undefined functions in {$file}");
    $output .= dump($functions, true);
  }
  return $output;
}

/** return undefined function calls in $file */
function undefined_function_calls($file) {
  
  $arr = defined_functions();
  $internal_functions = $arr['internal'];
  //dump($arr);die();
  //dump($internal_functions);die();
  
  $content = read_file($file);
  $tokens = token_get_all($content);
  $ar = [];
  $lines = [];
  foreach($tokens as $token) {
    $type = $token[0];
    $val = $token[1];
    $linenum = $token[2];
    $name = token_name($type);
    
    
    if(is_array($token)) {
      
      if(!is_array($lines[$linenum])) {
        $lines[$linenum] = $line = [];
      }
      
      $line[] = $name;
      //echo "Line {$linenum}: ", $name, " ('{$val}')", "<br>", PHP_EOL;
      if($name == "T_STRING") {
        $count = count($line);
        $previous = '';
        $first = $line[0];
        if($count >= 2) {
          $last = $count - 2;
          $previous = $line[$last];
        }
        
        if($first == 'T_FUNCTION' || 
           $previous == 'T_OBJECT_OPERATOR' || 
           in_array($val,['true', 'false']) || 
           defined($val) ||
           class_exists($val) ||
           strtoupper($val) == $val || /** probably some undefined constant */ 
           in_array($val, $internal_functions)) {
          //echo("Skipping {$val}. First={$first} Previous={$previous}<br>");
        } else {
          //$ar[$val] = $line;
          if(!function_exists($val)) $ar[$val] = "{$file}: {$linenum}";
        }
      }
    }
  }    
  return $ar;
  return array_keys($ar);
}

/** print table of function reference count */  
function print_function_reference_count() {  
  $result = function_reference_count();
  $data = $result->data;
  $rows = [];
  $rows[] = ['name', 'count', 'file'];
  foreach($data as $fn => $obj) {
    $rows[] = [$obj->function_name, $obj->reference_count, $obj->definition];
  }
  
  $output = '';
  $output .= html_h2("{$result->count} Functions. Unsused: {$result->unused_count}");
  $output .= html_table($rows, ['class' => 'web20']);

  $output .= dump($result->unused, true);
  return $output;
}

/** find referenced functions that are not defined */
function print_function_missing($options = []) { 
  $data = function_missing();
  $output = '';
  $output .= dump($data,true);
  return $output;
}

/** find referenced functions that are not defined */
function function_missing($options = []) { 
  $data = function_reference_count();
  
  return $data;
}

/** return data about which functions are called in which files */
function function_reference_count($options = []) {
  $data = function_count();
  $result = new stdClass;
  $refArray = [];
  $files = $data->files;
  $functions = $data->functions;
  $result->unused_count = 0;
  $result->count = count($functions);
  $result->unused = [];
  $exclude = ['tmp2','xmlreader-iterators','invoice_pdf_writer.inc', 'xml_leadingcourses.inc'];
  
  foreach($functions as $function => $fn_file) {
    $basename = basename($fn_file);
    $refObj = new stdClass;
    $refObj->definition = $fn_file;
    $ref_count = 0;
    foreach($files as $path) {
      
      $content = file_get_contents($path);
      if(!$content) die("$path was blank");
      $refs = file_function_references($content, $function);
      
      //$tokens = token_get_all($content);
      //die("Tokens in {$fn_file}:".dump($tokens,true));
  
      $count = count($refs);
      if($count && $fn_file == $path) $count--; /** uncount function def */
      if($count) {
        //echo("{$count} refs to function {$function} in {$path}<br>");
        $ref_count += $count;        
      } else {
        //echo(" no function {$function} in {$file}<br>");
      } 
    }
    $refObj->function_name = $function;
    $refObj->reference_count = $ref_count;
    if(!$ref_count) {
      $result->unused_count++;
      $result->unused[$fn_file]++;
      if(!in_array($basename, $exclude)) file_add_unused_comment($fn_file, $function);
    }
    $refArray[$function] = $refObj;    
  }
  asort($result->unused);
  usort($refArray, function($a, $b) {
    return $a->reference_count < $b->reference_count;
  });
  //ksort($refArray);
  $result->data = $refArray;
  return $result;
}

/** return data about which functions are defined in which files */
function function_count($options = []) {
  $dirs = ['include', 'includes', 'admin', 'mollie']; // exclude 'include' 
  $extensions = ['php', 'inc'];
  
  //$exclude = ['tmp2','xmlreader-iterators','invoice_pdf_writer.inc', 'xml_leadingcourses.inc'];
  $exclude = ['tmp2','xmlreader-iterators'];
  $ignore  = ['controller_', 'pre_add', 'post_add', 'pre_edit', 'post_edit', 'pre_delete', 'post_delete'];
  $i = $total = $bytes = 0;
  $base_dir = $_SESSION['app_root']; // /web/comps/php/apt-dev/
  $funcArray = $files = [];
  foreach($dirs as $dir) {
    $fileAr = file_array("{$base_dir}/{$dir}");
    //echo("files in {$dir}:".dump($fileAr,true));
    foreach($fileAr as $path => $file) {     
      if(in_array($file, $exclude)) continue;
      $extension = file_extension($path);
      if(!in_array($extension, $extensions)) continue;      
      $content = file_get_contents($path);      
      if(!$content) die("$path was blank");
      $functions = file_functions($content);
      $count = count($functions);
      $files[] = $path; /** include in files even if no functions, needed to check for references */
      if(!$count) continue;
      
      $bytes += strlen($content);
      foreach($functions as $fn) {
        $insert = true;
        foreach($ignore as $ign) {     
          if(strpos($fn, $ign) !== false) {
            $insert = false;
            break;
          }
        }          
        if($insert) {
          $total++;
          $funcArray[$fn] = $path;
        }
      }
      $i++;
    }
  }
  sort($files);
  ksort($funcArray);
    
  $result = new stdClass;
  $result->bytes = bytes2human($bytes);
  $result->file_count = $i;
  $result->function_count = $total;
  $result->files = $files;
  $result->functions = $funcArray;
  return $result;
}

/** 
    returns list of function definitions in content
*/
function file_functions($content) {  
  $pattern = '/function ([a-zA-Z0-9_]+)\s?\(/';
  $count = preg_match_all($pattern, $content, $m);  
  if(!$count) return [];
  return $m[1];
}

/** 
    returns list of function definitions in content
*/
function file_add_unused_comment($filename, $function) {
  $comment = "/** unsused - automatic comment added on $ts */";
  $file = read_file($filename);
  if(!$file) die("could not read from {$filename}");
  $file = str_replace("function {$function}(", "{$comment}\nfunction {$function}(", $file);
  if(!is_writable($filename)) die("Cannot not write to {$filename}");
  $b = 0;              
  if(0) {
    $b = write_file($filename, $file);
    if(!$b) die("Could not write to {$filename}");
    //die("Added unused comment to function {$function} in {$filename}. Wrote {$b}");
  }
  return $b;
}



/**
    returns references to this function (including function definition) 
*/
function file_function_references($content, $fn) {
  $pattern = "/ ({$fn})\(.*\)/";
  $count = preg_match_all($pattern, $content, $m);
  return $m[1];
}

/**
 * Read files in directory
 *
 * @param string $str Path to file or directory
 *
 * Return assoc. array ('file' => 'filename')
 */

function file_array($dir) {
  $files = array();
  $dir = rtrim($dir, '/').'/'; // always trailing slash;
  if($handle = opendir($dir)) {
    while (false !== ($entry = readdir($handle))) {
      if(strpos($entry, '.') === 0) continue; // skip hidden, this, and up
      if(!filesize($dir.$entry)) continue; // empty file
      list($filename, $ext) = split_filename($entry);
      $files[$dir.$entry] = $filename;
    }
    closedir($handle);
  }
  asort($files);
  return $files;
}
 
 
// like file_exists, but also searches include path
function fileexists($file) {
  if(file_exists($file)) return true;
  $ps = explode(":", ini_get('include_path')); // try include path
  foreach($ps as $path) if(file_exists($path.'/'.$file)) return true;
  return false;
}

// list all files in a directory, returnes as array
// from frasq: http://php.net/manual/en/function.readdir.php
// recursive option added by me
function listdir($dir='.', $recursive=false) { 
  if (!is_dir($dir)) { 
      return false; 
  } 
  
  $files = array(); 
  listdiraux($dir, $files, $recursive); 

  return $files; 
} 

// helper function for listdir
function listdiraux($dir, &$files, $recursive=false) { 
  $handle = opendir($dir); 
  while (($file = readdir($handle)) !== false) { 
      if ($file == '.' || $file == '..') { 
          continue; 
      } 
      $filepath = $dir == '.' ? $file : $dir . $file; 
      if (is_link($filepath)) 
          continue; 
      if (is_file($filepath)) 
          $files[] = $filepath; 
      else if ($recursive && is_dir($filepath)) 
          listdiraux($filepath, $files); 
  } 
  closedir($handle); 
} 


// can use built in PHP fns here
function split_filename($path) {
  $info = pathinfo($path);
  return array($info['filename'], $info['extension']);
}

function get_file_attributes($filename) {
   if($changed = @filectime($filename)) {
      $filesize = filesize($filename);
      $age = time()-$changed;
      //echo "age: $age limit: $_SESSION[max_age]";
   }       
   return array($filesize, $age);
}

function is_allowed_extension($fileName, $allowedExtensions) {
  $path_parts = pathinfo($fileName);
  $extension = $path_parts['extension'];
  return in_array(strtolower($extension), $allowedExtensions);
}

function display_filesize($filesize) {
  if(is_numeric($filesize)) {
    $decr = 1024; $step = 0;
    $prefix = array('Byte','KB','MB','GB','TB','PB');
       
    while(($filesize / $decr) > 0.9) {
        $filesize = $filesize / $decr;
        $step++;
    }
    return round($filesize,2).' '.$prefix[$step];
  } else {  
    return 'NaN';
  }   
}

function mkdir_recursive($path, $rights = 0777) {
  return mkdir($path, $rights, true);
}

/* not needed anymore  
  if(is_dir($path)) return true;
  $folder_path = array(strstr($path, '.') ? dirname($path) : $path);

  while(!@is_dir(dirname(end($folder_path)))
         && dirname(end($folder_path)) != '/'
         && dirname(end($folder_path)) != '.'
         && dirname(end($folder_path)) != '') {
    //print_debug("mkdir_recursive: checking for exisence of ".end($folder_path));
    array_push($folder_path, dirname(end($folder_path)));
  }

  while($parent_folder_path = array_pop($folder_path)) {
    // echo("mkdir_recursive: $parent_folder_path<br>");
    if(!@is_dir($parent_folder_path) && !@mkdir($parent_folder_path, $rights)) {
      user_error("Can't create folder \"$parent_folder_path\".");
      return false; // failure
    }
  }

  // added by kjetil - doesn't seem to create last folder for some reason
  if(is_dir($path)) return true;
  if(!@mkdir($path, $rights)) {
    user_error("Can't create last folder \"$path\".");
    return false; // failure
  }
  // end added by kjetil 
  return true; // success
}
*/

// moves files
function mv($source, $dest) {
  if(!copy($source, $dest)) {
    echo("mv: Couldn't move $source to $dest<br>");
    return false;
  }
  if(!unlink($source)) {
    echo("mv: Couldn't unlink $source<br>");
    return false;
  }
  return true;
}

function write_file($filename, $data, $mode='w', $echo=false) {
  if(!$data) {
    if($echo) echo("write_file: empty input".html_break());
    return false;
  }
  $f=@fopen($filename,$mode);
  if (!$f) {
    if($echo) echo("Couldn't open $filename for $mode".html_break());
    return false;
  } else {
    if($b = fwrite($f,$data)) {
      if($echo) echo("OK - wrote ($mode) $b bytes to $filename".html_break());
    } else {
      if($echo) echo("Couldn't $mode $data to $filename".html_break());
    }
    fclose($f);
    return $b; // returns number of bytes written
  }
}

function get_https($url) {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
  curl_setopt($ch, CURLOPT_HEADER, false);
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_REFERER, $url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  $result = curl_exec($ch);
  curl_close($ch);
  return $result;
}

function url2file($url, $filename, $options = array()) {
  $debug = $options['debug'] ?: false;
  $find = $options['find'] ?: '';
  $replace = $options['replace'] ?: '';
  
  if(!is_url($url)) {
    return json_error_object("url2file: $url is not a valid URL");
  }
  $url_parts = parse_url($url);
  $scheme = $url_parts['scheme'];
  if(!in_array($scheme, array('http', 'https', 'webcal'))) {
    return json_error_object("url2file: $url scheme is $scheme; only http(s) supported");
  }
  
  if($scheme == 'webcal') $url = str_replace('webcal://', 'http://', $url);
  //echo("scheme=$scheme url=$url");
  $data = $scheme == 'https' ? get_https($url) : file_get_contents($url);
  $len = strlen($data);
  if(!$len) {
    return json_error_object("url2file: $url returned zero bytes");
  }
  
  if($find) {
    $data = str_replace($find, $replace, $data);
    $len = strlen($data);
  }
    
  $path_parts = pathinfo($filename);
  $dir = $path_parts['dirname'];
  if(!is_dir($dir) && !mkdir_recursive($dir)) {
    return json_error_object("url2file: Could not make $dir");
  }
  
  $bytes = file_put_contents($filename, $data);
  //$bytes = write_file($filename, $data);
  if(!$bytes) {    
    $len_h = bytes2human($len);
    return json_error_object("url2file: Could not write $len_h to $filename");
  }
  if($debug) echo("url2file: Wrote $bytes to $filename. Expected: $len<br>");
  
  return json_success_object("Read $bytes bytes from $url, wrote to $filename");
}

function read_file($filename, $filesize=0, $binary=false, $echo=false) {
  $filename = trim($filename);
  if(!$binary) return file_get_contents($filename);
  if(!$filename) {
    echo("read_file: filename was blanks<br>");
    return false;
  }
  if(!$filesize) $filesize = filesize($filename);
  if(!$filesize) {
    if($echo) echo("read_file: file $filename was empty<br>");
    return false;
  }
  $handle = @fopen($filename,'r');
  $output = fread($handle, $filesize);
  fclose($handle);
  if($output === '') {
    if($echo) echo("read_file: Couldn't read $filename<br>");
    return false;
  }
  return $output;
}

// returns false on success.
function download($file_source, $file_target) {
  $file_source = str_replace(" ", "%20", $file_source);
  $rh = fopen($file_source, 'rb');
  $wh = fopen($file_target, 'wb');
  if ($rh===false || $wh===false) {
    // error reading or opening file
    return false;
  }
  while (!feof($rh)) {
    if (fwrite($wh, fread($rh, 1024)) === FALSE) {
      echo('Download error: Cannot write to file ('.$file_target.')');
      return false;
    }
  }
  fclose($rh);
  fclose($wh);

  if(file_exists($file_target) && !filesize($file_target)) {
    echo('Download error: File exists but 0 bytest, deleting ('.$file_target.')');
    unlink($file_target);
    return false;
  }
  // No error
  return true;
}
define('READ_LEN', 4096);

// uses ffmpeg php to return video about audio/video files
function get_av_data($filename) {
  if($movie = new ffmpeg_movie($filename)) {      
    $data['length'] = $movie->getDuration();
    $data['bit_rate'] = $movie->getBitRate();
    if($movie->hasVideo()) {      
      $data['frame_rate'] = $movie->getFrameRate();
      $data['height'] = $movie->getFrameHeight();
      $data['width'] = $movie->getFrameWidth();
      //$data['video_codec'] = $movie->getVideoCodec();
    }
    if($movie->hasAudio()) {      
      $data['audio_codec'] = $movie->getAudioCodec();
      $data['sample_rate'] = $movie->getAudioSampleRate();
      $data['channels'] = $movie->getAudioChannels();
    }
  } 
  return $data;
}

//   pass two file names
//   returns TRUE if files are the same, FALSE otherwise
function files_identical($fn1, $fn2) {
  if(filetype($fn1) !== filetype($fn2)) return FALSE;
  if(filesize($fn1) !== filesize($fn2)) return FALSE;
  if(!$fp1 = fopen($fn1, 'rb')) return FALSE;

  if(!$fp2 = fopen($fn2, 'rb')) {
    fclose($fp1);
    return FALSE;
  }

  $same = TRUE;
  while (!feof($fp1) and !feof($fp2)) {
    if(fread($fp1, READ_LEN) !== fread($fp2, READ_LEN)) {
      $same = FALSE;
      break;
    }
  }

  if(feof($fp1) !== feof($fp2)) $same = FALSE;

  fclose($fp1);
  fclose($fp2);

  return $same;
}

// checks the global $_FILES for user uploaded file (form input)
// moves to $dest if ok. $allowed_extension (lower case) = array of allowed extensions
function upload_user_file($imagedir, $allowed_extensions='') {

  $f_name  = $_FILES['userfile']['name']; // The original name of the file on the client machine.
  $f_type  = $_FILES['userfile']['type']; // The mime type of the file, if the browser provided this information. An example would be "image/gif". This mime type is however not checked on the PHP side and therefore don't take its value for granted.
  $f_size  = $_FILES['userfile']['size']; // The size, in bytes, of the uploaded file.
  $f_tmp   = $_FILES['userfile']['tmp_name']; // The temporary filename of the file in which the uploaded file was stored on the server.
  $f_error = $_FILES['userfile']['error']; // error msg
  $f_basename  = basename($f_name); // The original base name 

  if(!$imagedir) die ("upload_user_file: missing imagedir $imagedir");
  if(!is_dir($imagedir)) if(!mkdir_recursive($imagedir)) die ("pre_add_media: couldn't make imagedir $imagedir");

  $allowed_str = is_array($allowed_extensions) ? implode(', ', $allowed_extensions) : $allowed_extensions;
  $uploadfile = $imagedir.$f_name;

  $error_strings = array(
    UPLOAD_ERR_OK =>"There is no error, the file uploaded with success",
    UPLOAD_ERR_INI_SIZE => 'The uploaded file exceeds the upload_max_filesize directive in php.ini.',
    UPLOAD_ERR_FORM_SIZE => 'The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form.',
    UPLOAD_ERR_PARTIAL => 'The uploaded file was only partially uploaded.',
    UPLOAD_ERR_NO_FILE => 'No file was uploaded.',
    UPLOAD_ERR_NO_TMP_DIR => 'Missing a temporary folder.',
    UPLOAD_ERR_CANT_WRITE => 'Failed to write file to disk.',
    UPLOAD_ERR_EXTENSION => 'File upload stopped by extension.',
    100 => 'Missing upload directory.',
    101 => 'Could not create upload directory.',
    102 => "Illegal file extension in '$f_name': valid types are $allowed_str.",
  );

  if($allowed_str && !is_allowed_extension($f_name, $allowed_extensions)) return array(102, $error_strings[102]);

  //echo("upload_user_file: $fsize bytes -> $uploadfile<br>");

  if ($f_error !== UPLOAD_ERR_OK) {
    if(isset($error_strings[$errorCode])) {
      //throw new Exception($uploadErrors[$errorCode]);
      $errors[]= $error_strings[$f_error];
    } else {
      // throw new Exception("Unknown error uploading file.");
      $errors[]= "Upload failed.";
    }      
    print_log("upload_user_file: f_error={$f_error}".dump($errors,true), 'upload', LOG_LEVEL_ERROR);
  } else {
    print_log("upload_user_file: f_error={$f_error} Upload OK", 'upload', LOG_LEVEL_ALWAYS);

    if($f_type=='image/pjpeg' || $f_type =='image/jpg') $f_type='image/jpeg'; // Progressive jpeg 
    // echo("Upload OK - Moving uploaded file to $uploadfile<br>");
    $correct_mime_type = mime_type($f_name);
    if ($f_size>=$min_upload) {
      if (is_uploaded_file($f_tmp)) {
        if ($f_type == $correct_mime_type) {
          if (move_uploaded_file($f_tmp, $uploadfile)) {
            chmod($uploadfile, 0644);
            if($f_size < $rec_upload) $errors[]= "Warning: The image is less than $rec_upload_K KB. We recommend that you upload a higher quality picture.";                          
          } else {
            $errors[]= "MOVE_UPLOADED_FILE FAILED: Possible file upload attack!";
            if($f_error) $errors[]= $error_strings[$f_error];
          }
        } else {
          $errors[]= "FILE TYPE CHECK FAILED: This type = $f_type, should be $correct_mime_type \n";
          if($f_error) $errors[]= $error_strings[$f_error];
        }
      } else {
        $errors[]= "IS_UPLOADED_FILE FAILED: Possible file upload attack";
        if($f_error) $errors[]= $error_strings[$f_error];
      }
    } else {
        $errors[]= "The image quality is too low for use on this site. Please choose an image that is at least $min_upload_K KB";
        if($f_error) $errors[]= $error_strings[$f_error];
    }
  } // end if not f_error

  return array($f_error, $errors);
}
    
// returns false on success.
function upload_file($method, $source, $destination=0, $server=0, $user=0, $password=0) {

  if($method=="FTP" || $method=="SFTP") {
    // set up basic connection
    $conn_id = $method=="FTP" ? ftp_connect($server) : ftp_ssl_connect($server, 22);
    echo("upload_file: connecting to $server using $method <br>");
    dump($conn_id);
      
    // login with username and password
    $login_result = ftp_login($conn_id, $user, $password);
    dump($login_result);

    // check connection
    if ((!$conn_id) || (!$login_result)) {
           echo "FTP connection has failed!";
           echo "Attempted to connect to $server for user $user<br>";
           exit;
       } else {
           echo "Connected to $server, for user $user<br>";
           echo "Trying to upload $source to $server as $destination<br>";
       }

    // turn passive mode on
    ftp_pasv($conn_id, true);

    // upload the file
    $upload = ftp_put($conn_id, $destination, $source, FTP_ASCII);

    // check upload status
    if (!$upload) {
           echo("FTP upload has failed!<br>");
           return true;
       } else {
           echo("Upload OK. Uploaded $source to $server as $destination");
       }

    // close the FTP stream
    ftp_close($conn_id);
  } else {
    echo("upload_file: Unsupported protocol $method");
    return true;
  }
  return false;
}

function fetch_xml($xml_url, $target) {
  global $log_dir;
  // log XML file with timestamp
  $timestamp = date("Y-m-d_His");
  if($pos=strpos(strtolower($target), ".xml")) $target = substr($target, 0, $pos); // strip away .xml if present
  $xml_target_file = "$log_dir/XML/fetch/".$target."_".$timestamp.".xml"; // original download file
  $xml_target_file2 = "$log_dir/XML/fetch/".$target.".xml";  // cleaned up file ready to parse
  echo("Reading file <a href='$xml_url'>$xml_url</a>, writing to $xml_target_file<br>");

  if(!download($xml_url, $xml_target_file)) {
    echo ("<h3>Download Failed</h3>");
    return "";
  } else {
    echo ("<h3>Download OK</h3>Saved file as $xml_target_file<br>");
    $xml = read_file($xml_target_file);

    // this replaces empty elements <Tag></Tag>  with <Tag>__empty__</Tag>
    // because PHP XML parser->characterData() ignores tempty elements

    // $pattern = '#<([a-zA-Z]+)></\1>#';
    // $replacement = '<$1>__empty__</$1>';
    // $xml = preg_replace($pattern, $replacement, $xml);

    // $pattern = '#<\!\[CDATA\[(.*)\]\]>#';
    // $replacement = '$1';
    // $xml = preg_replace($pattern, $replacement, $xml);

    if(write_file($xml_target_file2, $xml)) {
      echo("Wrote to $xml_target_file<br>");
    } else{
      die("Couldn't write to $xml_target_file");
    }

    //echo("<br><textarea name='xml' rows='10' cols='160'>$xml</textarea><p><br>");
  }
  return $xml_target_file2;
}

// returns file basename
function file_name($file) {  
  return basename($file);
}
             
// returns file type (category) for given extension
function file_type($file_or_extension) {
  if(!$file_or_extension) return '';
  return get_object('mime_type', file_extension($file_or_extension), 'type');
}

// returns mime type object for file or extension
function file_mime($file_or_extension) {
  if(!$file_or_extension) return '';
  return get_object('mime_type', file_extension($file_or_extension));
}


/**
 file
 file-code-o
 file-video-o (file-movie-o)
 file-audio-o (file-sound-o)
 file-archive-o (file-zip-o)
 file-image-o (file-photo-o, file-picture-o)
 file-powerpoint-o
 file-excel-o
 file-word-o
 file-pdf-o
 file-text
 file-text-o
 file-o
*/

/** return preferred extension for most common mime types */
function mime_extension($mime_type) {
  if(!$mime_type) return '';
  switch($mime_type) {
    case 'image/jpeg': return 'jpg';
    case 'image/png': return 'png';
    case 'image/gif': return 'gif';
    case 'image/bmp': return 'bmp';
    case 'image/x-icon': return 'ico';

    case 'audio/mpeg': return 'mp3';
    case 'audio/x-aiff': return 'aif';
    case 'audio/x-flac': return 'flac';
    case 'audio/x-wav': return 'wav';

    case 'video/quicktime': return 'mov';
    case 'video/x-matroska': return 'mkv';
    case 'video/mpeg': return 'mpg';
    case 'video/mp4': return 'mp4';
      
    default: return '';
  }
}

// return font awesome class for extension
// use result like this "fa fa-file-{$result}-o" or "fa fa-file-{$result}"
function file_awesome_class($file_or_extension) {
  if(!$file_or_extension) return 'file';
  $extension = file_extension($file_or_extension);
  if(!$extension) return 'no ext';
  /** handle most common w/o db lookup */
  if($extension == 'pdf') return 'pdf';
  if(in_array($extension, ['doc','docx', 'odt', 'pages'])) return 'word';
  if(in_array($extension, ['xls','numbers'])) return 'excel';
  if(in_array($extension, ['ppt','pps', 'rar', 'gz', 'z', 'tgz', 'bz2', 'iso', 'dmg', 'jar'])) return 'powerpoint';
  if(in_array($extension, ['txt','css', 'html', 'text'])) return 'text';
  if(in_array($extension, ['zip','tar', 'rar', 'gz', 'z', 'tgz', 'bz2', 'iso', 'dmg', 'jar'])) return 'archive';
  if(in_array($extension, ['jpg','jpeg', 'gif', 'png', 'raw', 'ico', 'bmp'])) return 'image';
  if(in_array($extension, ['aif','mp3', 'wav', 'flac', 'wma', 'm3u'])) return 'audio';
  if(in_array($extension, ['avi','m4u', 'mov', 'mp4', 'movie', 'mpeg', 'mpg', 'mkv', 'vob'])) return 'video';
  if(in_array($extension, ['php','js'])) return 'code';
  
  // nothing found - try with mime type
  // application,audio,video,image,text,x-conference,model,chemical,message,
  $mimeObj = file_mime($extension);
  if(!$mimeObj) return '';
  if($mimeObj->icon) return $mimeObj->icon;
  $type = $mimeObj->type;  
  $subtype = $mimeObj->subtype;
  if(in_array($type, ['audio','video', 'image', 'text'])) return $type;
    
  if($type == 'application') {
    switch($subtype) {
      default: return 'file';
    }
  }
  return 'file';
}

function file_extensions($type='', $allow_upload=1) {
  $mimeSearch['allow_upload'] = $allow_upload;
  if($type && $type != 'all') $mimeSearch['type'] = $type;
  return find_objects('mime_type', $mimeSearch,  array('fields' => 'extension'));
}

function file_extension($file) {
  if(strpos($file, '.') === false && strpos($file, '/') === false) return strtolower($file); // already extension
  return strtolower(pathinfo($file, PATHINFO_EXTENSION));
}

// return path (w/o trailing slash)
function file_path($file) {
  return pathinfo($file, PATHINFO_DIRNAME);
}


// strips docroot from path, adds domain
function file_loc($file, $siteObj_or_id=null) {
  global $__SITE_ID;
  if(!$siteObj_or_id) $siteObj_or_id = $__SITE_ID;
  if(strpos($file, 'http') === 0) return $file;
  $docroot = docroot(true);  
  if(strpos($file, $docroot) === 0) {
     $file = str_replace($docroot, '', $file);
     return site_full_url($siteObj_or_id, false).$file;
  }
  return $file;
}

// returns path to logo for give file type
function file_icon($file_or_extension) {
  $extension = file_extension($file_or_extension);

  $type = file_awesome_class($extension);
  $class = $type ? "-{$type}" : '';
  return html_i('', "fa fa-file{$class}-o");
}                                                               


// list of valid file extension for upload per category
// note: sync this with /ckeditor/ckfinder/config.php
// by default, only return those allowed for upload
// will work with array

function remote_filesize($url) {
  static $regex = '/^Content-Length: *+\K\d++$/im';
  if (!$fp = @fopen($url, 'rb')) {
    return false;
  }
  if (
    isset($http_response_header) &&
    preg_match($regex, implode("\n", $http_response_header), $matches)
  ) {
    return (int)$matches[0];
  }
  return strlen(stream_get_contents($fp));
}

// returns human readable file size (rounded)
function bytes2human($size) {
  $units = array(' B', ' KB', ' MB', ' GB', ' TB', ' PB');
  for ($i = 0; $size >= 1024; $i++) { $size /= 1024; }
  return round($size, 2).$units[$i];
}        

// returns size in bytes give human readable file size
// e.g 1K returns 1024
function human2bytes($str){
  $str = strtoupper(str_replace(' ', '', $str)); // remove white space, uppercase
  $bytes = 0;
  $bytes_array = array(
      'B' => 1,
      'KB' => 1024,
      'MB' => 1024 * 1024,
      'GB' => 1024 * 1024 * 1024,
      'TB' => 1024 * 1024 * 1024 * 1024,
      'PB' => 1024 * 1024 * 1024 * 1024 * 1024,
      'K' => 1024,
      'M' => 1024 * 1024,
      'G' => 1024 * 1024 * 1024,
      'T' => 1024 * 1024 * 1024 * 1024,
      'P' => 1024 * 1024 * 1024 * 1024 * 1024,
  );
  $bytes = floatval($str);
  if (preg_match('#([KMGTP]?B?)$#si', $str, $matches) && !empty($bytes_array[$matches[1]])) {
      $bytes *= $bytes_array[$matches[1]];
  }
  $bytes = intval(round($bytes, 2));
  return $bytes;
}        

// takes a comma separated list of paths.
// strips away if don't exist
function file_exist_list($data_dir, $val) {
	if(!$val) return '';
	$files = explode(',', $val);
	$ex_files = array();
	foreach($files as $file) if(file_exists($data_dir.$file)) $ex_files[] = $file;
	return implode(',', $ex_files);
}

// todo: make it work w/o type
function file_count($dir, $type) {
  $glob = glob($dir . "*.$type");
  return $glob === false ? '' : count($glob);
}
// displays list of files with preview image (for images), icons for rest + link to file
// if auth_del is set to true, display delete icon
// val can be list of media IDs or list of paths/urls
function file_list($val, $options=NULL) {
  global $data_dir;
  if(!$val) return '';
  $auth_del=$options['auth_del'];

  $list_id = $options['id'] ?: 'file_list';
  $class = $options['class'] ?: 'file_list'; 
  //$class .= " sortable";
  
  $mediaOptions = $options;
  $mediaOptions['size'] = 'thumb';
  if($mediaData = $options['media_data']) {
    $parent_type  = $mediaData['parent_type'];
    $parent_id    = $mediaData['parent_id'];
    $parent_field = $mediaData['parent_field'];
    $update_parent = $parent_type && $parent_id && $parent_field ? 1 : 0; // all must be set     
    //echo("gh up=$update_parent").dump($mediaData);
  }
  
  //array_merge(array('size' => 'thumb', 'class' => "preview"), $options);
    
  
  $files = is_array($val) ? $val : explode(',', $val);
  $items = array();  
  $imageExtensions = file_extensions('image');
  $i = 1; // jquery's nthchild is CSS based - counting starts with 1
  foreach($files as $file) {
    if(!$file) continue;
    if(is_object($file) || is_array($file) || isint($file)) { // mediaObject 
      $mediaObj = any2obj('media', $file);
      $id = $mediaObj->id;
      if(!$mediaObj) continue;
      $src = print_media($mediaObj, array('source_only' => true, 'size' => 'thumb'));
      $name = $mediaObj->title ?: basename($src);
      $data = array('obj_type' => 'media', 'id' => $mediaObj->id);      
    } else if(strlen($file)) { // must be URL (backward compat.)      
      $mediaObj = NULL;
      $id = $src = $file;
      $name = basename($file);
      $data = array('src' => $src, 'dir' => clean_path($SERVER['DOCUMENT_ROOT'].$src));
    } else { 
      continue;
    }
    
    $prefix = 'file_thumb_';
    $data_json = json_encode($data);
    $delOptions['handler'] = 'delete_div_handler'; 
    $delOptions['prefix'] = $prefix; 
    
    //$image = html_image($src, $name, $mediaOptions);
    $image = html_image($src);
  	$del_link = $auth_del && $data_dir ? html_image_link('#', '/images/icons/delete_tiny.png', "Delete $filename", array('js' => "onclick='if(confirm(\"Are you sure you want to delete $file?\")) {delete_uploaded_file(\"$id\", \"$i\", \"$file\");return false; } else { return false; }'")) : '';
  	
  	
  	$iconAr = array();
  	$iconAr[html_link('#', 'i', array('title' => phrase('information', CAPITALIZE), 'class' => 'tooltip-ajax info-link', 'data-url' => "/ajax.php?oper=load-function&function=media_info&param1=$id"))] = array('class' => 'tiny-icon info');
  	$iconAr[html_link('#', 'x', array('title' => phrase('delete', CAPITALIZE), 'class' => 'delete_object', 'data-options' => json_encode($delOptions), 'data-data' => $data_json))] = array('class' => 'tiny-icon delete');
    $icons = html_ul($iconAr, 'tiny-icons', array('item_class' => ''));
    
    
    //$image = print_media($file, array('size' => 'square', 'class' => "preview"));
    $items[$id]= html_div($image.$icons, 'tinyCont relative inline-block'); // preview
  }
  if($update_parent) $update_parent_script = '$.ajax({ url: "/ajax.php?obj_type='.$parent_type.'", type: "POST", data: "obj_type='.$parent_type.'&oper=edit&id='.$parent_id.'&'.$parent_field.'="+ids_str });';
  
  $script = html_script_inline('
    $("#'.$list_id.'").sortable({
      containment: $("#'.$list_id.'"),
      cursor: "move",
      update: function(event, ui) {
        var imgArray = $(this).sortable("toArray");
        var ids = new Array();
        var ranks = new Array();
        $.each(imgArray, function(index, value) {
          id = value.split("_").pop();
          rank = parseInt(index) + 1;
          ids.push(id);
          ranks.push(rank);
        });
        var ids_str = ids.toString();
        var ranks_str = ranks.toString();
        $.ajax({ url: "/ajax.php?obj_type=media", type: "POST", data: "obj_type=media&oper=edit&data_field=rank&id="+ ids_str + "&rank="+ranks_str });
        '.$update_parent_script.'
      }
   });    
');
  return html_ul($items, $class, array('item_class' => 'inline', 'id' => $list_id, 'item_prefix' => $prefix)).$script;
}                                                                           


// returns the mime type (or another field, optionally) based on the extension of a file name
function mime_type($filename, $field='mime') {
  list($name, $ext) = split_filename(strtolower($filename));
  return find_object('mime_type', array('extension' => $ext), $field);   
}


// returns array of valid mime types for upload
function mime_types() {
  return array('application', 'audio', 'video', 'image', 'text');
}
    

// reads a CSV file (with header)
// inserts or updates objects in database
// header fields must correspond to object fields
// handler is an array fld => function which calls function with data to find fld value  
// if update is true, objects will be added/updated, otherwise data is just returned
// if strict is set, column count of each row must match that of the headers
// function csv2objects($obj_type, $filename, $key='', $handlers=NULL, $update=true, $echo=true, $strict=false) {
function csv2objects($obj_type, $filename, $options = array()) {

  $key = $options['key'] ?: '';
  $handlers = $options['handlers'] ?: NULL;
  $update = $options['update'] ?: false;
  $echo = $options['echo'] ?: true; 
  $strict = $options['strict'] ?: false;
  $map = $options['map'] ?: null;

  //dump($options);
  //echo("Reading $filename...<br>");
  $csv = read_file($filename);
  $len = strlen($csv);

  if($echo) echo("Read $len from $filename...<br>");
  $data = array();
  $lines = explode("\n", rtrim($csv, "\n")); //parse the rows
  $header_csv = array_shift($lines);
  $header = str_getcsv($header_csv, ",");
  
  if($map && is_array($map)) {
    $header = array_flip(map_array(array_flip($header), $map));
  }
  //dump($header);die();
  //dump($header);
  
  // if(!$key) $key = get_object_key($obj_type); // uncommented = always add if key is not provided
  
  $descriptor = get_object_descriptor($obj_type);  
  //dump($lines);die();
  $i=1;
  foreach($lines as &$row_str) {
    //dump($row_str);die();
    $row = str_getcsv($row_str, ","); //parse the items in rows
    //dump($row);die();
    $missing_cols = count($header) - count($row); 
    if($missing_cols) {
      if($strict) {
        dump($header);
        dump($row);
        die("csv2objects: Header / row count mismatch in line $i<br>row=$row_str");
      } else {
        echo("csv2objects: Header / row count mismatch in line $i<br>row=$row_str<br>Missing cols=$missing_cols".html_break());
        $row = array_pad($row, count($header), '');
      }
    }
    $obj = array2obj(array_combine($header, $row));
    $data[] = $obj;
    $i++;
  }
  if(count($handlers)) { // loop through all rows and all fields, call handler for each
    foreach($data as $obj) {
      foreach($handlers as $fld => $fn) {
        $obj->$fld = call_user_func($fn, $obj);
      }

      // slow and silly way:      
      // foreach($obj as $fld => $val) {
      //   if($fn = $handlers[$fld]) {
      //     $obj->$fld = call_user_func($fn, $obj);
      //   }
      // }
    }
  }
// dump($data[0]);
// die();  

  //dump($data);die();
  if(!$update) return $data;  

  $obj_key = get_object_key($obj_type);
  foreach($data as $index => $obj) {
    $i++;  
    if($key && ($member = find_object($obj_type, array($key => $obj->$key)))) { // key passed in as argument
      $id = $member->$obj_key;
      list($mid, $errors) = update_object($obj_type, $obj, $id);
      $action = 'Updated';
    } elseif($obj_key && isset($obj->$obj_key)) { // the object key was found in the data
      list($mid, $errors) = add_object($obj_type, $obj, array('preset_id' => $obj->$obj_key));
      $action = 'Added';
    } else {
      list($mid, $errors) = add_object($obj_type, $obj);
      $action = 'Added';
    }
    if($errors) {
      dump($errors);
      die();
    } else {
      $id = $mid;
      $output .= "$action $obj_type $id (".$obj->$descriptor.")<br>";
    }
  }
  if($echo) echo($output);
  return $data;
}

/** unsused - automatic comment added on  */
function analyze_log($file='') {
  if(!$file) $file = '/web/logs/affiliate_log.txt';
  $lines = file($file);
  $count = count($lines);
  echo("there are $count lines in $file<br>");
  
  // GET /aid1010/amsterdam/serviced-apartment/
  
  $pattern = '#\[(.*)\] "GET /aid([0-9]{4}/)?([a-z]{2}/)?([^/\.]+/)?([^/\.]+/)?([a-z\-]+/)?#';
  //$limit = 25;
  foreach($lines as $line) {
    $line = substr($line, 0, strpos($line, ' HTTP'));
    preg_match($pattern, $line, $m);
    //print_r($m);
    if(count($m) > 1) {
      $date = trim($m[1], '/');
      $aid = trim($m[2], '/');
      $lang = trim($m[3], '/');
      $city = trim($m[4], '/');
      $page = trim($m[5], '/');
      $apt = trim($m[6], '/');
      
      //$results['aff'][$aid]['date'] = $date;
      $results['aff-date'][$aid] = $date;
      $results['aff'][$aid]++;
      $results['lang'][$lang]++;
      $results['city'][$city]++;
      $results['page'][$page]++;
      $results['apt'][$apt]++;
      
      //if(!$aid)  echo("<b>aid=$aid lang=$lang city=$city page=$page apt=$apt</b> line=$line<br>");

    }
    if($limit && ++$i > $limit) break;
  }
  asort($results['aff']);
  asort($results['lang']);
  asort($results['city']);
  asort($results['page']);
  asort($results['apt']);
  foreach($results['aff'] as $aid => $count) {
    $aff_name = get_object('affiliate', $aid, 'affiliate_name');
    $results['aff'][$aid] .= " ($aff_name) Last seen: ".$results['aff-date'][$aid];
  }
  unset($results['aff-date']);
  dump($results);
}

// end file helpers

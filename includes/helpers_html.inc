<?php

// html5 functions
/** unsused - automatic comment added on  */
function html5_header($content, $options='') {
 return html_tag('header', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_footer($content, $options='') {
 return html_tag('footer', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_figure($content, $options='') {
 return html_tag('figure', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_time($content, $options='') {
 return html_tag('time', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_title($content, $options='') {
 return html_tag('title', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_hgroup($content, $options='') {
 return html_tag('hgroup', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_article($content, $options='') {
 return html_tag('article', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_aside($content, $options='') {
 return html_tag('aside', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html5_nav($content, $options='') {
 return html_tag('nav', $content, '', $options);
}

function html5_section($content, $options='') {
 return html_tag('section', $content, '', $options);
}

/** unsused - automatic comment added on  */
function html_button($content, $options='') {
 return html5_button($content, $options);
}

function html5_button($content, $options='') {
 return html_tag('button', $content, '', $options);
}

function html5_video($src, $name, $options=array()) {
  return "
<video width='320' height='240' controls>
  <source src='$src' alt='$name' type='video/mp4'>
  Your browser does not support the video tag.
</video>
";
}

function html5_audio($src, $name, $options=array()) {
  return "
<audio width='320' height='240' controls>
  <source src='$src' alt='$name' type='audio/mp4'>
  Your browser does not support the audio tag.
</audio>
";
}

/** unsused - automatic comment added on  */
function html5_handwritten_note($title, $text, $options = array()) {
 global $__header;
 $width = $options['width'] ?: 500;
 $factor = ($width - 50) / 10;
 // echo("Factor = $factor<br>");
 $num_lines = $options['num_lines'] ?: round(strlen($text) / $factor) + 2;
 $output = html_h3($title);
 $output .= html_div(html_div($text));
 $output .= str_repeat("<div><div></div></div>", $num_lines);
 return html5_section($output, array('class' => 'personal_note'));
}

function html_section($content, $class, $options='') {
 return html_tag('section', $content, $class, $options);
}

function html_break($repeat=0) {
  $br = "<br>";
  return $repeat ? str_repeat($br, $repeat) : $br;
}

function html_hr($repeat=0) {
  $br = "<hr>";
  return $repeat ? str_repeat($br, $repeat) : $br;
}


// generic html helper
function html_tag($tag, $content='', $class='', $options = array(), $extra_attr = NULL) {
  $tag = strtolower($tag);
  $attr_string = html_options($class, $options, $extra_attr);
  if(in_array($tag, html_singleton_tags())) return "<{$tag}{$attr_string}>";
  return "<{$tag}{$attr_string}>{$content}</{$tag}>";
}


/** unsused - automatic comment added on  */
function html_h1($title, $class='', $options=[]) {
  return html_tag('h1', $title, $class, $options);
}

function html_h2($title, $class='', $options=[]) {
  return html_tag('h2', $title, $class, $options);
}

function html_h3($title, $class='', $options=[]) {
  return html_tag('h3', $title, $class, $options);
}

function html_h4($title, $class='', $options=[]) {
  return html_tag('h4', $title, $class, $options);
}

/** unsused - automatic comment added on  */
function html_h5($title, $class='', $options=[]) {
  return html_tag('h5', $title, $class, $options);
}

function html_i($content, $class='', $options='') {
 return html_tag('i', $content, $class, $options);
}

function html_p($content, $class='', $options='') {
 return html_tag('p', $content, $class, $options);
}

/** unsused - automatic comment added on  */
function html_bold($content, $class='', $options = array()) {
 return html_tag('b', $content, $class, $options);
}

function html_b($content, $class='', $options = array()) {
 return html_tag('b', $content, $class, $options);
}

/** unsused - automatic comment added on  */
function html_em($content, $class='', $options = array()) {
 return html_tag('em', $content, $class, $options);
}

/** unsused - automatic comment added on  */
function html_pre($content, $class='', $options = array()) {
 return html_tag('pre', $content, $class, $options);
}                        

/** unsused - automatic comment added on  */
function html_strong($content, $class='', $options = array()) {
 return html_tag('strong', $content, $class, $options);
}

function html_div($content, $class='', $options='') {
 return html_tag('div', $content, $class, $options);
}

function html_li($content, $class='', $options='') {
 return html_tag('li', $content, $class, $options);
}

function html_span($content, $class='', $options = array()) {
 return html_tag('span', $content, $class, $options);
}

function html_image($url, $alt="", $options = array()) {
 $options['src'] = $url;
 if(!$options['alt']) $options['alt'] = $alt ?: basename($url);
 return html_tag('img', '', $class, $options, array('src', 'alt', 'height', 'width'));
}

//function html_image_link($url, $image, $title='', $full=false, $class='', $js='', $h=0, $w=0) {
function html_image_link($url, $image, $title='', $options = array()) {
	$options['title'] = $options['title'] ?: $title; 
 return html_link($url, html_image($image, $title), $options);
}

function html_link($url, $name="", $options = array()) {
 $options['href'] = $options['full'] ? add_host($url) : $url;
 unset($options['full']);
 $extra_attrs = array('href', 'target', 'rel', 'download');
 return html_tag('a', $name ?: $url, $class, $options, $extra_attrs);
}

// used to print 1 or 2 values, 1s is overstriked, 2nd bold
// typically full price and discount price
/** used by deprecated function print_rate */
function html_strike($val1, $val2='') {
  $output = "<span class='strike'>$val1</span>";
  if($val2) $output .= " &nbsp; $val2";
  return $output;
}

function html_singleton_tags() {
 return array('area','base','br','col','command','embed','hr','img','input','keygen','link','meta','param','source','track','wbr');   
}

/** unsused - automatic comment added on  */
function html_meta_tag($options) {
  if(!$options || !is_array($options)) return '';
  return html_tag('meta', '', '', $options, array('name', 'property', 'content', 'language', 'charset', 'scheme', 'http-equiv'));
}

/** unsused - automatic comment added on  */
function html_header_link($options) {
  if(!$options || !is_array($options)) return '';
  return html_tag('link', '', '', $options, array('name', 'property', 'content', 'language', 'charset', 'scheme', 'http-equiv'));
}

// takes a string or an array of options, like id, class, style, returns string for HTML elements
function html_options($class = '', $options = array(), $extra_attr = NULL) {
  if(is_object($options)) $options = (array) $options;
  if(is_array($options)) {
  if(strlen(trim($class))) $options['class'] = trim("$class ".$options['class']);
  
  // Global HTML attributes, Others must be passed in extra_attr or options['js'] (cathcall)
  // 'data-*' is also ok, handled below. Itemprop added by me (microtagging)
  $std_attr = array('accesskey','class','contenteditable','contextmenu','dir','draggable','dropzone','hidden','id','itemprop','lang','spellcheck','style','tabindex','title','translate'); 
  
  if($extra_attr) {
    if(is_object($extra_attr)) $extra_attr = (array) $extra_attr;
    if(!is_array($extra_attr)) $extra_attr = array($extra_attr); // allow passing single value
    $std_attr = array_merge($std_attr, $extra_attr); // allow optional extra attributes
  }
  $attrs = array();
  foreach($options as $key => $val) {
   if(in_array($key, $std_attr) || strpos($key, 'data-') === 0) {
     if(is_array($val) || is_object($val)) {
       if(strpos($key, 'data-') === 0) { // only data attributes may have arrays/objects as value
         $val = json_encode($val);
       } else {
         $val = '';
       }
     } else {
       $val = trim($val);
     }
     
     if(strlen($val)) $attrs[] = "$key='".str_replace("'", "\'", $val)."'";
     unset($options[$key]); // remove to avoid double processing in html_style
   }
  }
  $options_str = ' '.implode(' ', $attrs);
  if($js = $options['js']) { // catchall for attributes not in std_attr  
   $options_str .= " $js";
  }
  
  // only loops through options not already processed
  if(!$options['style'] && ($style_str = html_style($options))) {
   $options_str .= " $style_str";
  } 

 } else { // not array
  if(is_array($class) && ($_SESSION['dev'] || $_SESSION['test'])) mydie("html_options: class is array: ".dump($class, true));
  if(strlen(trim($class))) $options_str = " class='$class'";
  if(strlen(trim($options))) $options_str .= " $options";
 }

 return $options_str;
}


// takes array like "height" => 50, "width" => 60
// or "height" => array(50, '%'), "widht" => 60
// return inling style string like style='height: 50px;width: 60px;' 
function html_style($styleAr, $options = array()) {
  if(!$styleAr) return '';
  
  $css_keys = ["border-color","border-style","border-width","outline-color","outline-style","outline-width","background-color","background-image","background-repeat","background-attachment","background-position","position","top","left","right","bottom","overflow","clear","visibility","cursor","display","z-index","height","width","margin","padding","color","text-decoration","text-align","text-transform","text-indent","line-height","letter-spacing","word-spacing","vertical-align","white-space","direction","font-style","font-size","font-weight","list-style-image","list-style-position","list-style-type","opacity"];
  $css_dim  = ["border-style","border-width","outline-width","top","left","right","bottom","height","width","margin","padding","text-indent","line-height","letter-spacing","word-spacing","font-size"];
  $css_color = ["border-color","outline-color","background-color","color"];
  
  $styleAr = (array) $styleAr;
  $items = array();
  foreach($styleAr as $k => $v) {  
    if(!in_array($k, $css_keys)) continue; // this is not a style
    $unit = in_array($k, $css_dim) ? ($v['unit'] ?: 'px') : '';
    $hash = in_array($k, $css_color) && $v[0] != '#' ? '#' : ''; // doesn't work for 'pink', 'green', etc
    $items[] = "$k: ".$hash.$v.$unit;
  }
  return count($items) ? " style='".implode(';', $items)."'" : '';
}

/** unsused - automatic comment added on  */
function html_style_tag($style, $type = "text/css", $media="screen") {
  if(strpos($style, "style:") === 0) return html_style_inline(substr($style, 6));
  return "<link rel=\"StyleSheet\" href=\"$style\" type=\"$type\" media=\"$media\"/>";
}

/** unsused - automatic comment added on  */
function html_script_tag($src, $type="text/javascript", $id='') {
  if(strpos($src, "literal:") === 0) { // inline script
    return substr($src, 8);    
  } if(strpos($src, "script:") === 0) { // inline script
    return html_script_inline(substr($src, 7), $type);    
  } else if(strpos($src, "{") === 0) { // json object
    $obj = json_decode($src);
    return html_script($obj->src, $obj->type, $obj->id);
  }
  $id_str = $id ? " id=\"$id\"" : '';
  return "<script type=\"$type\" src=\"$src\"$id_str></script>";
}

function html_script_inline($script, $type = "text/javascript") {
  if(!trim($script)) return ''; 
  return "<script>\n    $script\n  </script>"; // HTML5
}



// returns a blank image used for spacing
/** unsused - automatic comment added on  */
function html_spacer($h=1, $w=1) {
  return html_image('/images/gif/blank.gif', 'spacer', array('height' => $h, 'width' => $w));
}

/** unsused - automatic comment added on  */
function html_checkmark($val, $options = array()) {
  return $val ? html_i("", "fa fa-check text-success", $options) : html_i("", "fa fa-times text-danger", $options);
}

/** unsused - automatic comment added on  */
function html_color($val, $encoding = 'hex') {
  if(!strlen(trim($val))) return '';
  echo("val=$val<br>");
  switch ($encoding) {
  case 'hex':
    return "#".$val;
  default: 
    return '';
  }  
}

// returns a link which opens content of $link into a fancybox
/** unsused - automatic comment added on  */
function html_fancybox_link($link, $name, $options = array()) {
 global $__APP;
 $__APP->header->onload[] = "fancybox_loader();";
 $options['class'] .= ' fancybox fancybox.ajax';
 return html_link($link, $name, $options).$script;
}


/** unsused - automatic comment added on  */
function html_iframe($src, $w=600, $h=500, $options=array()) {
  $options['src'] = $src;                         
  $options['height'] = $h;                         
  $options['width'] = $w;
  if($srcdoc = $options['srcdoc'] ? htmlspecialchars($options['srcdoc']) : '') $options['srcdoc'] = $srcdoc;
  return html_tag('iframe', "Sorry, your browser does not support frames", 'foo', $options, ['srcdoc', 'height', 'width', 'src']);
}

/** unsused - automatic comment added on  */
function html_embed_youtube($resource_id, $h=200, $w=250, $params=NULL) {
  $src="http://www.youtube.com/v/$resource_id";
  if($params) $src .= $params;
  return html_flash($src, $h, $w);
}

/** returns an unordered list for pagination - used by smarty */
/** unsused - automatic comment added on  */
function html_pagination($count, $start=0, $limit=6, $options = array()) {
  $num_pages = ceil($count/$limit);
  if($num_pages <= 1) return ''; // no pagination needed
  $current_page = $n = floor($start/$limit) + 1;
  
  $link_class = $options['link_class'] ?: '';

  $base_link = $options['base_link'] ?: '';
  $target = $options['target'] ?: '';
  $handler = $options['success_handler'] ?: '';  
  $ajax = $target ? 1 : 0;
  
  // how many boxes to show before/after current page (1, ..., n-1, n, n+1 ... Last) , where n is current page
  $pages = $options['pages'] ?: 1; 
  if($pages > $num_pages-2) $pages = 0;

  $items = array();
  $options = [];
  
  // previous page
  if($current_page > 1) {
    $page = $current_page - 1;
    $start = ($page-1) * $limit;
    //$options = $ajax ? ['class' => $link_class, 'data-target' => $link_target, 'data-data' => array('start' => $start)] : [];
    //$link = $ajax ? "#" : "?start={$start}";
    $link = "{$base_link}?start={$start}";        
    $items[] = array('item' => html_link($link, html_i('', 'fa fa-caret-left'), $options), 'class' => '');      
  }

  // first add 1 for first page
  $class = $current_page == 1 ? 'active' : '';
  //$options = $ajax ? ['class' => $link_class, 'data-target' => $link_target, 'data-data' => array('start' => 0)] : [];
  //$link = $ajax ? "#" : "?start=0";
  $start = 0;
  $link = "{$base_link}?start={$start}";        
  
  $items[] = array('item' => html_link($link, 1, $options), 'class' => $class);      
  
  if($num_pages > 2) {
    // add ... if n-1 > 2
    if($n - $pages > 2) $items[] = array('item' => '...', 'class' => 'dots');

    if($n > 2) {
      // then add n-2, n-1 ...  pages times
      $p = $pages;
      while($p) {
        $page = $n - $p;
        $start = ($page-1) * $limit;
        //$options = $ajax ? ['class' => $link_class, 'data-target' => $link_target, 'data-data' => array('start' => $start)] : [];
        //$link = $ajax ? "#" : "?start={$start}";
        $link = "{$base_link}?start={$start}";        
        $class = $page == $current_page ? 'active' : '';
        $items[] = array('item' => html_link($link, $page, $options), 'class' => $class);
        $p--;
      }
    }
    
    if($n == 1) $n++; // first page, set n=2 since first page is already shown
    if($n > 1 && $n < $num_pages) {
      // then add n (current page)
      $class = $n == $current_page ? 'active' : '';
      $page = $n;
      $start = ($page-1) * $limit;
      //$options = $ajax ? ['class' => $link_class, 'data-target' => $link_target, 'data-data' => array('start' => $start)] : [];
      //$link = $ajax ? "#" : "?start={$start}";
      $link = "{$base_link}?start={$start}";        
      $items[] = array('item' => html_link($link, $page, $options), 'class' => $class);
      
    }
    
    // then add n+1, n+2 ... (pages time)
    $i = 1;
    while($i <= $pages && $n + $i < $num_pages) {
      $page = $n + $i;
      $class = $page == $current_page ? 'active' : '';
      $start = ($page-1) * $limit;
      //$options = $ajax ? ['class' => $link_class, 'data-target' => $link_target, 'data-data' => array('start' => $start)] : [];
      //$link = $ajax ? "#" : "?start={$start}";
      $link = "{$base_link}?start={$start}";        
      $items[] = array('item' => html_link($link, $page, $options), 'class' => $class);
      $i++;
    }
  
    // then add ... if needed
    if($n + $i < $num_pages) $items[] = array('item' => '...', 'class' => 'dots');
  }

  // then add last, if needed
  $class = $current_page == $num_pages ? 'active' : '';
  $page = $num_pages;
  $start = ($page-1) * $limit;
  //$link = $ajax ? "#" : "?start={$start}";
  $link = "{$base_link}?start={$start}";        
  if($i <= $num_pages) $items[] = array('item' => html_link($link, $num_pages, $options), 'class' => $class);

  // next page
  if($current_page < $num_pages) {
    $page = $current_page + 1;
    $start = ($page-1) * $limit;
    //$link = $ajax ? "#" : "?start={$start}";
    $link = "{$base_link}?start={$start}";        
    $items[] = array('item' => html_link($link, html_i('', 'fa fa-caret-right'), $options), 'class' => '');
  }
    
  $data = $ajax ? ['data-target' => $target, 'data-success_handler' => $handler] : [];
  $output = html_ul($items, 'pagination subview-nav', $data);
  //$output .= html_p("c=$count s=$start l=$limit p=$num_pages n=$current_page ba=$pages");
  return $output;
}


/** prints associative arrays (e.g. fields of record),
    either has html table rows, or some other predefined style
    used by smarty print_array */
function print_array_assoc($record_fields, $style='left', $translate=false) {
 $lines = array();
 if($translate) {
  foreach($record_fields as $fld_name => $val) {
   $tmp_fields[phrase($fld_name, CAPITALIZE)] = $val;
  }
  $record_fields = $tmp_fields;
 }

 switch($style) {
  case 'table':
   foreach($record_fields as $fld_name => $val) {
   	$rowclass = (++$i%2) ? "class='odd'" : "";
    $record_str.= "<tr $rowclass><td>$fld_name</td><td>$val</td></tr>\n";
   }
   return $record_str;

  case 'break':
   foreach($record_fields as $fld_name => $val) {
    $lines[] = "$fld_name:".html_break()."$val";
   }
   return implode(html_break(), $lines);

  case 'left': // prints $field: $value (w/o fixed column width)
   foreach($record_fields as $fld_name => $val) {
    $lines []= "$fld_name: $val";
   }
   return implode(html_break(), $lines);

  case 'line':
   foreach($record_fields as $fld_name => $val) {
    $record_str.= "<div class='record_field float_left'>$fld_name:".html_break()."$val</div>\n";
   }
   return $record_str;

  case 'plaintext':
   foreach($record_fields as $fld_name => $val) {
    $lines []= "$fld_name: $val";
   }
   return implode("\n", $lines);

  case 'lines':
   foreach($record_fields as $fld_name => $val) {
    $lines []= html_b("$fld_name:")." $val";
   }
   return implode(html_break(), $lines);

  case 'div':
   $names = implode(html_break(), array_keys($record_fields));
   $values = implode(html_break(), array_values($record_fields));
   $record_str = "
    <div class='record_names'>$names</div>\n
    <div class='record_values'>$values</div>\n";
   return $record_str;

  default:
   return die("unknown style $style");
 }
}

/** used by smarty */
/** unsused - automatic comment added on  */
function hide_false($val) {
  return $val ? $val : '';
}

/** Pure css tooltip
  $anchor is the shown text 
  $tip is the pop-up tip
  used by smarty 'tooltip'
*/

function html_tooltip($tip, $style = "info", $options = array()) {
  
  $class = trim('afr-tooltip '.$options['class']);
  $jq_tooltip = strpos($class, 'tooltip-jq') !== false;
  
  if(!$style) $style = $options['style'] ?: 'help';
  if(!in_array($style, array('info', 'warn', 'warning', 'help', 'critical', 'error', 'check', 'success', 'time', 'money'))) $style = 'help';
  list($icon, $color) = html_tooltip_icon($style);
  
  $w = $options['width'] ?: 16;
  $h = $options['height'] ?: 16;

  // the anchor
  $anchor = html_i("", "$icon");
  //if(!$anchor) $anchor = html_image("/images/tooltip/$style.png", $style, array('height' => $h, 'width' => $w));

  $link_options = array('class' => $class);
                                    
  $inset_tip = $inset_image = '';

  if($options['bootstrap']) {
    $link_options['data-toggle'] = 'tooltip';
    $link_options['data-title'] = $tip;      
  } elseif($options['jquery'] || $jq_tooltip) { // store tip in data element
    $tip = htmlspecialchars($tip, ENT_QUOTES, 'UTF-8', false);
    //$link_options['data-toggle'] = 'tooltip';
    $link_options['data-title'] = $tip;      
  } else { // display popup inline (z-index issues)
    $inset_anchor = html_i("", "$icon fa-4x msgicon $color");
    $inset_tip = html_span($inset_anchor.$tip, "ztop custom $style");
  }
  
  return html_link('#', $anchor.$inset_tip, $link_options);
}


// helper for below: returns font-awesome icon
function html_tooltip_icon($style) {
  switch($style){
    case "info":  return array("fa fa-info-circle", "text-info");
    case "warn":  return array("fa fa-exclamation-circle", "text-warning");
    case "warning":  return array("fa fa-exclamation-circle", "text-warning");
    case "help":  return array("fa fa-question-circle", "text-primary");
    case "error":  return array("fa fa-exclamation-triangle", "text-danger");
    case "critical":  return array("fa fa-exclamation-triangle fa-spin", "text-danger");
    case "check":  return array("fa fa-check-circle", "text-success");
    case "success":  return array("fa fa-check-circle", "text-success");
    case "time":  return array("fa fa-clock-o", "text-danger");
    case "money":  return array("fa fa-money", "text-danger");
    default:  return "";
  }
}

/** displays a flag using sprites
   new: using https://github.com/lipis/flag-icon-css
   used by smarty: 'flag' */
/** unsused - automatic comment added on  */
function html_flag($country_code, $options = array()) {
  if(strtolower($country_code) == 'en') $country_code = 'gb';
  if($country = cc_list1(strtoupper($country_code))) $options['title'] = $country['name'];
  return html_span('', "flag-icon flag-icon-".strtolower($country_code), $options);
}

// takes a template_id or template string with <!--$name--> and populates it with the data
/** unsused - automatic comment added on  */
function html_template($html, $data) {
  if(is_whole_number($html)) $html = get_object('site_template', $html, 'html');
  foreach($data as $k => $v) $html = str_replace("<!--$".$k."-->", $v, $html);
  return $html; 
}

// old, deprectated
/** unsused - automatic comment added on  */
function style_string($height=0, $width=0, $bg='', $fg='', $border='') {
 $style_ar = array();
 if($width) $style_ar[] = "width:".$width."px";
 if($height) $style_ar[] = "height:".$height."px";
 if($border) $style_ar[] = "border: 1px solid #".$border;
 if($bg) $style_ar[] = is_color($bg) ? "background-color:#$bg" : "background-image: url($bg); background-repeat: repeat-x; "; // add background color or image
 if($fg) $style_ar[] = "color:#$fg";
 return $style_ar ? "style='".implode(';', $style_ar)."'" : '';
}

// HTML ul
// items is array. If indexed, the index is the key used for item ID
// use item_class for same class for all items, or array item_classes for different classes
// use sub-array item_options for full control of item tag
function html_ul($list, $class='', $options = array()) {
  if(!$list || !is_array($list) || !count($list)) return '';
  if($item_options = $options['item_options']) unset($options['item_options']); 
  if($item_classes = $options['item_classes']) unset($options['item_classes']); // array, one element for each item
  $show_item_id = $options['show_id'] || $options['item_prefix'];
  $item_class  = $options['item_class'] ?: $item_options['class']; // same for all items
  
  $items = array();
  foreach($list as $key => $item) {
    if(isAssoc($item)) { // item is asscociate array
      $item_value = $item['item'];
      $item_class = $item['class'];
      $item_option = isset($item['options']) && is_array($item['options']) ? $item['options'] : array();
     
    } elseif(is_array($item)) { // item is an array, the key is the value, item is options
      $item_value = $key; 
      $item_option = $item;
    } else { // key is optional ID, item is the LI value
      $item_value = $item;
      $item_class = is_array($item_classes) ? array_shift($item_classes) : $item_class;
      $item_option = is_array($item_options) ? array_shift($item_options) : $item_options;
    }
    if($show_item_id) $item_option['id'] = $options['item_prefix'].$key;
    $items[] = html_li($item_value, $item_class, $item_option);
  }
  $content = "\n ".implode("\n ", $items)."\n";
  return "\n".html_tag('ul', $content, $class, $options)."\n";
}

function html_label($content, $for='', $class='') {
  if($for) $attr = " for='$for'";
  if($class) $attr .= " class='$class'";
  return "<label$attr>$content</label>";
}


// takes an associtative array or an array of ass. arrays, returns an HTML table with keys as header row
function array2table($arrays, $options = array()) {
 if(!is_array($arrays)) return '';
 if(!is_array($arrays[0])) return array2table(array($arrays), $options);
 $keys = array_keys($arrays[0]);
 $rows[] = $keys;
 foreach($arrays as $array) {
  $row = array();
  foreach($keys as $key) { // we don't know if each member is sorted the same way
   $row[] = $array[$key];
  }
  $rows[] = $row;
 }
 return html_table($rows, $options);
}

/** unsused - automatic comment added on  */
function css_table($rows, $options = array()) {
  $class = isset($options['class']) ? $options['class'] : '';
  $output = "<div class=\"css-table {$class}\">\n";
  $i = 0;
  foreach($rows as $row) {
    $output .= "  <div class=\"css-table-tr".($i ? "" : " first-row")."\">\n";
    $j = 0;
    foreach($row as $cell) {
      $output .= "    <div class=\"css-table-td".($j ? "" : " first-cell")."\">{$cell}</div>\n";
      $j++;
    }
    $output .= "  </div>\n";
    $i++;
  }
  $output .= "</div>\n";
  return $output;
}

// list must be array
//function html_table($rows, $caption='', $class='', $header=1, $footer=0, $header_col=0, $echo=false, $rows_js=NULL, $js='') {
function html_table($rows, $options=NULL) {
  $caption = $options['title'] ?: $options['caption'];
  $header = isset($options['header']) ? $options['header'] : 1; // default is one header row
  $header_col = $options['header_col'];
  $footer = $options['footer'];
  $class = $options['class'] ? $options['class'] : 'standard_table'; 
  $js = $options['js'];
  $rows_js = $options['rows_js'];

  $str="<table class='$class' $js>";
  if($caption) $str .= "<caption>$caption</caption>\n";

  if(!is_array($rows)) $rows=array($rows);
  foreach($rows as &$row) { // loop through to find # columns
    if(!is_array($row)) $row=array($row);
    $col_count = max($col_count, count($row));
  }

	$header_rows = array();
	if($header) {
	  $str .= " <thead>\n";
	  while($header--) $header_rows[] = array_shift($rows); // shift first N rows off (headers)
	  foreach($header_rows as $hrow) {
	    if(!count($hrow)) continue;
	    $row_js = $rows_js[$i];
	    $hstr = "  <tr $row_js>\n";
	    $colspan_str = count($hrow)==1 ? " colspan='$col_count'" : "";
	    foreach($hrow as $cell) {
	      $hstr .= "   <th scope='col'$colspan_str>$cell</th>\n";
	    }
	    $hstr .= "  </tr>\n";
	    $str .= $hstr;
	    $i++;
	  }
		$str .= " </thead>\n";
	}

  $footer_str = '';
  if($footer) {
		$footer_str .= " <tfoot>\n";
    $footer_rows = array();
    while($footer--) $footer_rows[] = array_pop($rows); // pop last N rows off (footers)
    $colspan = count($rows[0]);
    $footer_rows = array_reverse($footer_rows); // since last was popped first
    foreach($footer_rows as $frow) {
      if(!count($frow)) continue;
      $row_js = $rows_js[$i];
      $fstr = "  <tr $row_js>\n";
      $colspan_str = count($frow)==1 ? " colspan='$col_count'" : "";
      foreach($frow as $cell) {
        $fstr .= "   <th scope='col'$colspan_str>$cell</th>\n";
      }
      $fstr .= "  </tr>\n";
      $footer_str .= $fstr;
      $i++;
    }
		$footer_str .= " </tfoot>\n";
	}
	$i=0;

	$str .= " <tbody>\n";

  foreach($rows as $trow) {
  	if(!count($trow)) continue;
  	$row_js = $rows_js[$i] ?: '';
  	$tstr = "  <tr $row_js>\n";
		$colspan_str = count($trow)==1 ? " colspan='$col_count'" : "";
    $j = $header_col; // how many columns are TH (header columns)
  	foreach($trow as $cell) { // todo: add scope='row' to opening TH
	    $el = $j-- > 0 ? 'th' : "td";
	    $tstr .= "   <$el $colspan_str>$cell</$el>\n";
		}
		$tstr .= "  </tr>\n";
		$str .= $tstr;
		$i++;
  }
	$str .= " </tbody>\n";
	if($footer_str) $str .= $footer_str."\n";
	$str .= "</table>";
	if($echo) echo($str); else return $str;
}

/** alias */
/** unsused - automatic comment added on  */
function html_alert($lines, $style='', $options = array()) {
  return print_alert($lines, $style, $options);
}

// print boostrap style alert. if options === 0/false or !options['dismiss'], no close icon is shown
function print_alert($lines, $style='', $options = array()) {
  if(!$lines) return;
  $glue = $options['glue'] ?: html_break();

  $dismiss = $options === 0 || $options === '0' || $options === false || (isset($options['dismiss']) && !$options['dismiss']) ? 0 : 1; 
  //$style = $options['style'] ?: 'info';
  switch($style) {
  case 'success':   
  case 'message':   
    $class = "success";
    $icon = "ok";
    break;
  case 'warning';
    $class = "warning";
    $icon = "warning-sign";
    break;
  case 'danger':
  case 'error':
    $class = "danger";
    $icon = "exclamation-sign";
    break;
  default: // info
    $class = "info";
    $icon = "info-sign";
    break;
  }
  if(!is_array($lines)) $lines = array($lines);
  $lines = array_filter($lines);
  if(!count($lines)) return;
  $text = implode($glue, $lines);
  return 
    "<div class=\"alert alert-$class\" role=\"alert\">".
      ($dismiss ? "<a href=\"#\" class=\"close\" data-dismiss=\"alert\">&times;</a>" : "").        
      "<span class=\"glyphicon glyphicon-$icon\" aria-hidden=\"true\"></span> ".
      $text.
    "</div>\n";
}


// takes array of error messages
function print_errors($errors, $options = []) {
  return print_alert($errors, 'error', $options);
}

function print_messages($messages, $options = []) {
  return print_alert($messages, 'success', $options);
}


/** unsused - automatic comment added on  */
function country_name($cc) {
  return cc_name($cc);
}

function cc_name($cc) {
  $ccArray = cc_list2();
  return isset($ccArray[$cc]) ? $ccArray[$cc][0] : '';
}

/** unsused - automatic comment added on  */
function html_yes_no($yn) {
  if($yn < 0) return html_i('', 'text-danger fa fa-times');
  if($yn > 0) return html_i('', 'text-success fa fa-check');
  return '';
}

/** return font-awesome stars for 5-based rating (0-5);
    set show_empty to show empty stars and half-empty stars */
/** unsused - automatic comment added on  */
function star_rating($rating, $options=[]) {
  $max_rating = isset($options['max_rating']) && $options['max_rating'] > 0 ? $options['max_rating'] : 5;
  $show_empty = isset($options['show_empty']) ? $options['show_empty'] : false;
  $stars = '';

  $step = $max_rating / 5;  
  if($rating < 0 || $rating > $max_rating) return '';  
  for($i=1; $i <= 5; $i++) {
    if($rating > 0.75) $star_class = 'star';    
    elseif($rating > 0.25) $star_class = $show_empty ? 'star-half-o' : 'star-half'; 
    else $star_class = $show_empty ? 'star-o' : ''; 
    
    if($star_class) {
      $star = '<i class="fa fa-'.$star_class.'"></i>';
      $stars .= $star;
    }
    $rating -= $step;
  }
  return html_span($stars, 'star-rating', $options);
}


/** file upload/download */

// New uploader using Blueimp jquery-file-upload:
// https://blueimp.github.io/jQuery-File-Upload/
// Bootsrap and Jquery UI version
function jquery_file_uploader($id='fileupload', $val='', $options = array(), $container_options = array()) {
  global $__APP;  
  if(!$options) $options = array();       
  //print_log("jquery_file_uploader: options=".dump($options, true), 'upload', LOG_LEVEL_TEST);
  $options = jfu_fix_options($options);
  print_log("jquery_file_uploader: cloud={$options['cloud']}", 'upload', LOG_LEVEL_TEST);
  extract($options);
  if($error) $error = html_div($error, 'alert alert-danger');
  if(!$version) $version = 'bootstrap';
  jfu_dependencies($__APP, $options);  
  
  if($coptions = jfu_container_options($id, $options)) $container_options = array_merge($container_options ?: array(), $coptions); // options for DIV container
  $container_options['data-tip'] = jfu_tip($options, true);
  $container_options['data-version'] = $version;
  $container_options['data-view'] = isset($options['view']) ? $options['view'] : "list";
  $classes = array('jquery-fileupload');
  if($class = $options['class']) $classes[] = $class;
  $output = html_div($error.html_div('', implode(' ', $classes), $container_options), 'jquery-file-uploader-container'); //."id=$id".dump($options, true);
  //$output .= dump($options, true);
  //print_log("jquery_file_uploader: options=".dump($options, true), 'upload', LOG_LEVEL_TEST);
  
  return $output;
}

// thanks https://github.com/johnculviner/jquery.fileDownload/blob/master/src/Scripts/jquery.fileDownload.js
/** unsused - automatic comment added on  */
function file_downloader() {
  $output = '
<div id="preparing-file-modal" title="Preparing report..." style="display: none;">
  We are preparing your report, please wait... 
  <div class="ui-progressbar-value ui-corner-left ui-corner-right" style="width: 100%; height:22px; margin-top: 20px;"></div>
</div>
<div id="error-modal" title="Error" style="display: none;">
  There was a problem generating your report, please try again.
</div>';
  return $output;
}


// load JS + CSS for jquery-file-upload
function jfu_dependencies($app, $options = array()) {
  $__header = $app->header;
  $download_template = isset($options['download_template']) ? $options['download_template'] : "";
  $upload_template = isset($options['upload_template']) ? $options['upload_template'] : "";

  if($app->uploader_loaded) {
    //echo("uploader loaded...".caller_stack());
    print_log("already loaded download_template={$download_template} options:".dump($options,true), 'gallery', LOG_LEVEL_TEST);
    //return;
  } else {
    print_log("loading jfu_dependencies download_template={$download_template} options:".dump($options,true), 'gallery', LOG_LEVEL_TEST);
    //echo("uploader *not* loaded...".caller_stack());
  }
  $app->uploader_loaded = 1;

  
  $__header->scripts[] = '/afr/bower/blueimp-tmpl/js/tmpl.js?v=2';
  $__header->scripts[] = '/afr/bower/blueimp-load-image/js/load-image.all.min.js';
  $__header->scripts[] = '/afr/bower/blueimp-canvas-to-blob/js/canvas-to-blob.min.js';
  $__header->scripts[] = '/afr/bower/blueimp-gallery/js/jquery.blueimp-gallery.min.js';
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/vendor/jquery.ui.widget.js';  
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.iframe-transport.js';
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload.js';
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload-process.js';
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload-image.js';
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload-audio.js';
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload-video.js';
  
  //$__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload-validate.js';
  $__header->scripts[] = '/afr/js/jquery.fileupload-validate.js';
             
  $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload-ui.js';
  //$__header->scripts[] = '/afr/bower/exif-js/exif.js';
  
  //if($version == 'jquery') $__header->scripts[] = '/afr/bower/blueimp-file-upload/js/jquery.fileupload-jquery-ui.js';
  $__header->scripts[] = "literal:".jfu_download_template($download_template);
  $__header->scripts[] = "literal:".jfu_upload_template($upload_template);
  $__header->styles[] = '/afr/bower/blueimp-gallery/css/blueimp-gallery.min.css';
  $__header->styles[] = '/afr/bower/blueimp-file-upload/css/jquery.fileupload-ui.css';
  $__header->styles[] = '/afr/bower/blueimp-file-upload/css/jquery.fileupload.css';
  $__header->styles[] = '/afr/css/fileupload.css';             
  
}


// tooltip helper for file uploader
function jfu_tip($options, $plain_text=false) {
  extract($options);
  $tipAr = array();
  $tipAr[]= phrase('allowed_types', CAPITALIZE).": ".implode(',', $allowed_types); 
  $tipAr[]= phrase('allowed_extensions', CAPITALIZE).": ".implode(',', $allowed_extensions);
  
  //if($size_limit) $tipAr[]= phrase('maximum_size', CAPITALIZE).": $size_limit";
  
  // tooltip
  if($minimum_file_size) $tipAr[]= phrase('minimum_file_size', CAPITALIZE).": ".bytes2human($minimum_file_size);
  if($maximum_file_size) $tipAr[]= phrase('maximum_file_size', CAPITALIZE).": ".bytes2human($maximum_file_size);
                                                                
  // image specific messages
  if(in_array('image', $allowed_types)) {                                     
   if($minimum_height)  $tipAr[]= phrase('minimum_height', CAPITALIZE).": ".$minimum_height."px";  
   if($minimum_width)   $tipAr[]= phrase('minimum_width', CAPITALIZE).": ".$minimum_width."px";
   if($minimum_size) {
     if(is_array($minimum_size)) {
       list($short,$long) = $minimum_size;
       $tipAr[] = "The longest side must be at least $long pixels, or the shortest side must be at least $short pixels";
       $tipAr[] = "The highest published resolution is $long x $short pixels.";
     } else {
       $tipAr[]= phrase('minimum_size', CAPITALIZE).": ".$minimum_size."px";
     }
   }
  }
  //$tipAr[]= dump($options, true);
  $max_files = $allow_multiple && $maximum_files ? " (Max. $maximum_files)" : "";
  $tipAr[]= phrase('allow_multiple', CAPITALIZE).": ".($allow_multiple ? phrase('yes', CAPITALIZE) : phrase('no', CAPITALIZE)).$max_files;
  $tipAr[]= phrase('automatically_resized', CAPITALIZE).": ".($resize ? phrase('yes', CAPITALIZE) : phrase('no', CAPITALIZE));
  $tip_text = implode(html_break(), $tipAr);
  if($plain_text) return $tip_text;
  
  $tip_options = array('height' => 24, 'width' => 24);
  $tip = html_tooltip($tip_text, "help", $tip_options);
  return $tip;
}

// fixed file upload options, filling in defaults where needed
function jfu_fix_options($options) {
  extract($options);
  
  // resize
  if($resize) {
    if($resize === 1 || $resize === true) $resize = 1;
    else if (is_object($resize)) $resize = (array) $resize;
    else if (!is_whole_number($resize) && !is_array($resize)) $resize = 0; // must be array, object, or true/1
    $options['resize'] = $resize;
  }

  // defaults
  if(!isset($options['show_file_list'])) $options['show_file_list'] = true; // show file list by default
  if(!isset($options['show_tip'])) $options['show_tip'] = true; // show tool tip by default
  if(!isset($options['allow_multiple'])) $options['allow_multiple'] = 0; // only one file by default
  if(!isset($options['maximum_files'])) $options['maximum_files'] = 0; // new: how many files we can upload, if not set and allow_multiple is set, then no limit

  // size
  $system_post_size = ini_get('post_max_size');
  $system_file_size = ini_get('upload_max_filesize');
  
  $system_post_size_b = human2bytes($system_post_size);
  $system_file_size_b = human2bytes($system_file_size);
  $system_file_size_b = min($system_post_size_b, $system_file_size_b);
  
  $maximum_file_size = pick_first($options['size_limit'], $options['maximum_file_size'], ini_get('upload_max_filesize'));
  $size_limit_b = human2bytes($maximum_file_size);  
  $options['maximum_file_size'] = $size_limit_b; // use bytes from now on 
 
  if($size_limit_b > $system_file_size_b) {
    $size_limit_b = $system_file_size_b;
    $system_file_size = bytes2human($system_file_size_b);
    $options['error'] = "System Max. Filesize = {$system_file_size}. Required: {$maximum_file_size} <br>Contact system administrator.";
  }
  
  if($minimum_file_size) {
    $size_limit_b = human2bytes($minimum_file_size);  
    $options['minimum_file_size'] = $size_limit_b; // use bytes from now on 
  }
  
  // types, extensions
  if(is_array($options['allowed_types'])) $options['allowed_types'] = array_filter($options['allowed_types']);
  if(is_array($options['allowed_extensions'])) $options['allowed_extensions'] = array_filter($options['allowed_extensions']);
  
  if($options['allowed_types'] && !is_array($allowed_types)) $options['allowed_types'] =  explode(',', $allowed_types); // if comma separated, make array
  if($options['allowed_extensions'] && !is_array($allowed_extensions)) $options['allowed_extensions'] =  explode(',', $allowed_extensions);

  if(!$options['allowed_types']) $options['allowed_types'] = mime_types();
  if(!$options['allowed_extensions']) $options['allowed_extensions'] = $ax = file_extensions($options['allowed_types']);
  //dump($ax);
  
  // title  
  //if(!isset($options['title'])) {
  // if(count($allowed_types) == 1) {
  //  $allowed_type= $allowed_types[0];
  //  $type_string = " ".($allow_multiple ? phrase($allowed_type.'s') : phrase($allowed_type));
  //  $options['title'] = phrase('click_or_drag_to_upload', CAPITALIZE).$type_string;
  // } else {
  //  $options['title'] = phrase('click_or_drag', CAPITALIZE).$type_string;
  // }
  //}
  
  // full UI (delete,cancel,upload buttons on top + checkboxes on items)
  $full_ui = !isset($options['ui']) ? $options['allow_multiple'] : $options['ui'] == "full" && $options['allow_multiple'];
  $options['data-ui'] = $full_ui ? 'full' : 'basic';
  return $options;
}

function jfu_container_options($id, $options) { // options for DIV container
  $container_options = $options;
  $options['id'] = $options['data-id'] = $id;
  $data_keys = array(
   'allow_multiple', 'allowed_types', 'allowed_extensions', 'size_limit', 'path', 'title', 'resize', 'cloud', 'target',
   'minimum_file_size', 'minimum_height','minimum_width', 'minimum_size', 'maximum_file_size', 'maximum_height','maximum_width', 'maximum_size', 'edit_url', 'edit_target');
  foreach($data_keys as $key) {
    if(isset($options[$key])) {
      $jsOptions[$key] = $options[$key];
      unset($options[$key]); 
    }
  }
  if($handler = $options['handler']) {
    $options['handler_options']['handler'] = $handler; // copy handler to handler_options, needed by javascript success_handler function
  }
  // move these options to HTML5 data, with optional json_encoding
  $data_keys = array('handler' => false, 'handler_options' => true, 'media_data' => true, 'upload_template' => false, 'download_template' => false);
  foreach($data_keys as $key => $encode) {
    if($options[$key]) {
      $options["data-".$key] = $encode ? json_encode($options[$key]) : $options[$key];
      unset($options[$key]); 
    }
  }
  if(count($jsOptions)) $options['data-options'] = json_encode($jsOptions);   
  return $options;
}
     

/** unsused - automatic comment added on  */
function jfu_upload_template($version = 'full') {
  return '
<!-- The template to display files available for upload -->
<script id="template-upload" type="text/x-tmpl">
{% for (var i=0, file; file=o.files[i]; i++) { %}
    <tr id="template-upload-row" class="template-upload fade">
        <td>
            <span class="preview"></span>
        </td>
        <td>
            <p class="name">{%=file.name%}</p>
            <strong class="error text-danger"></strong>
        </td>
        <td>
            <p class="size">Processing...</p>
            <div class="progress progress-striped active" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="0"><div class="progress-bar progress-bar-success" style="width:0%;"></div></div>
        </td>
        <td>
            {% if (!i && !o.options.autoUpload) { %}
                <button class="btn btn-primary start" disabled>
                    <i class="glyphicon glyphicon-upload"></i>
                    <span>Upload</span>
                </button>
            {% } %}
            {% if (!i) { %}
                <button class="btn btn-warning cancel">
                    <i class="glyphicon glyphicon-ban-circle"></i>
                    <span>Cancel</span>
                </button>
            {% } %}
        </td>
    </tr>
{% } %}
</script>
';
}

// if template is set, only one is returned, otherwise all with buttons for user to choose
/** unsused - automatic comment added on  */
function jfu_download_template($template='') {
  //if($template == '') return '(none)';

  // compact version - grid with images
  $templateAr['grid'] = '
<script id="template-download-grid" type="text/x-tmpl">
{% var file_count = o.files.length;%}
{% if(file_count >0) { %}
{% console.log("template-download-grid: file_count=" + file_count);%}
{% console.log(o); var ui= o.options ? o.options.ui : "";%}
{% var checkbox_class = ui == "full" && file_count > 1 ? "" : "hidden ";%}
{% var handle_class = file_count > 1 ? "" : "hidden ";%}
    <tr id="template-download-row">
        <td>
          <ul class="file-container"> 

{% for (var i=0, file; file=o.files[i]; i++) { %}
{% var file_type = file.type || "";%}
{% if(!file.title) file.title = file.name;%}
{% if(!file.obj_id) {file.obj_type = "dummy";file.obj_id = i;} %}
{% if (file.error) { console.log("error=" + file.error);%}
          <li class="relative template-download margin-right">
          <span class="text text-danger">{%=file.error%}</span>
          </li>
          
{% } else { %}
{% console.log("File for template:");console.log(file);%}
          <li class="relative template-download margin-right" id="{%=file.obj_type%}-{%=file.obj_id%}" 
              data-obj_id="{%=file.obj_id%}" data-obj_type="{%=file.obj_type%}" data-type="{%=file.type%}" data-url="{%=file.url%}"> 
            <span class="preview">
                {% if (file_type.indexOf("audio") == 0) { %}  
                    <audio src="{%=file.url%}" controls="" data-type="audio" class="thumbCont margin_center" ></audio>
                {% } else if (file_type.indexOf("video") == 0) { %}  
                    <video src="{%=file.url%}" controls="" data-type="video" class="thumbCont margin_center"></video>
                {% } else if (file.thumbnailUrl) { %}
                    <a href="{%=file.url%}" title="{%=file.title%}" data-type="image" class="thumbCont margin_center" download="{%=file.name%}" data-gallery><img src="{%=file.thumbnailUrl%}"></a>
                {% } else if (file_type.indexOf("image") == 0) { %}  
                    <a href="{%=file.url%}" title="{%=file.title%}" data-type="image" class="thumbCont margin_center" download="{%=file.name%}" data-gallery><img src="{%=file.url%}"></a>
                {% } else { %}
                    <a href="{%=file.url%}" title="{%=file.title%}" data-type="{%=file.type%}" download="{%=file.name%}">{%=file_type%}</a>
                {% } %}
            </span>
            <div class="name">
                {% if (file.url && file_type.indexOf("image") == 0) { %}
                    <a href="{%=file.url%}" title="{%=file.title%}" download="{%=file.name%}" {%=file.thumbnailUrl?\'data-gallery\':\'\'%}>{%=file.title%}</a>
                {% } else { %}
                    <span>{%=file.title%}</span>
                {% } %}
            </div>
            {% if (file.error) { %}
                <div><span class="label label-danger">Error</span> {%=file.error%}</div>
            {% } %}
            {% if(0) { %} <div class="size">{%=o.formatFileSize(file.size)%}</div>{% } %}
            <div class="buttons">
                <span class="{%=handle_class%}btn btn-xs btn-primary fa fa-arrows handle"></span>
            {% if (file.editUrl) { %}
              {% var editClass = file.editTarget ? "ajax-loader " : "edit-object "; %}
                <button class="{%=editClass%}btn btn-xs btn-primary edit" 
                        data-target="{%=file.editTarget%}" 
                        data-url="{%=file.editUrl%}"
                        data-obj_type="{%=file.obj_type%}" 
                        data-obj_id="{%=file.obj_id%}"
                        data-fields=\'["name"]\' 
                        data-reload_link="#rental-photos" 
                        
                        {% if (file.deleteWithCredentials) { %}
                        data-xhr-fields=\'{"withCredentials":true}\'
                        {% } %}>
                    <i class="glyphicon glyphicon-edit"></i>
                </button>
            {% } %}
            {% if (file.deleteUrl) { %}
                <button id="jfu-delete-{%=file.obj_id%}" class="btn btn-xs btn-danger delete" data-toggle="confirmation" title="Sure?" data-placement="right" data-type="{%=file.deleteType%}" data-url="{%=file.deleteUrl%}"{% if (file.deleteWithCredentials) { %} data-xhr-fields=\'{"withCredentials":true}\'{% } %}>
                    <i class="glyphicon glyphicon-trash"></i>
                </button>
                <input type="checkbox" name="delete" value="1" data-url="{%=file.deleteUrl%}" class="{%=checkbox_class%}toggle">
            {% } else { %}
                <button class="btn btn-xs btn-warning cancel">
                    <i class="glyphicon glyphicon-ban-circle"></i>
                    <span>Cancel</span>
                </button>
            {% } %}
            </div>
          </li>
{% }  %}
{% }  %}
{% }  %}
          </ul>
        </td>
    </tr>
</script>
';
  
  if($template == 'grid') return $templateAr['grid'];
  
  // full version (list with item per row)
  $templateAr['list'] = '
<script id="template-download-list" type="text/x-tmpl">
{% var file_count = o.files.length;%}
{% if(file_count >0) { %}
{% console.log("template-download-list file_count=" + file_count);%}
{% console.log(o); var ui= o.options ? o.options.ui : "" ;%}
{% var checkbox_class = ui == "full" && file_count > 1 ? "" : "hidden ";%}
{% var handle_class = file_count > 1 ? "" : "hidden ";%}
{% for (var i=0, file; file=o.files[i]; i++) { %}
{% var file_type = file.type || "";%}
{% if(!file.title) file.title = file.name;%}
{% console.log("File for template:");console.log(file);%}

    <tr id="template-download-row" class="template-download fade" id="{%=file.obj_type%}-{%=file.obj_id%}" 
        data-obj_id="{%=file.obj_id%}" data-obj_type="{%=file.obj_type%}">
        <td>
            <span class="preview">
                {% if (file_type.indexOf("audio") == 0) { %}  
                    <audio src="{%=file.url%}" controls=""></audio>
                {% } else if (file_type.indexOf("video") == 0) { %}  
                    <video src="{%=file.url%}" controls=""></video>
                {% } else if (file.thumbnailUrl) { %}
                    <a href="{%=file.url%}" title="{%=file.title%}" class="thumbCont" download="{%=file.name%}" data-gallery><img src="{%=file.thumbnailUrl%}"></a>
                {% } else if (file_type.indexOf("image") == 0) { %}  
                    <a href="{%=file.url%}" title="{%=file.title%}" class="thumbCont" download="{%=file.name%}" data-gallery><img src="{%=file.url%}"></a>
                {% } else { %}
                    <a href="{%=file.url%}" title="{%=file.title%}" download="{%=file.name%}">{%=file_type%}</a>
                {% } %}
            </span>
        </td>
        <td>
            <p class="name">
                {% if (file.url) { %}
                    <a href="{%=file.url%}" title="{%=file.title%}" download="{%=file.name%}" {%=file.thumbnailUrl?\'data-gallery\':\'\'%}>{%=file.title%}</a>
                {% } else { %}
                    <span>{%=file.title%}</span>
                {% } %}
            </p>
            {% if (file.error) { %}
                <div><span class="label label-danger">Error</span> {%=file.error%}</div>
            {% } %}
        </td>
        <td>
            <span class="size">{%=o.formatFileSize(file.size)%}</span>
        </td>
        <td>
            {% if (file.editUrl) { %}
              {% var editClass = file.editTarget ? "ajax-loader " : "edit-object "; %}
                <button class="{%=editClass%}btn btn-primary edit" 
                data-target="{%=file.editTarget%}" 
                data-url="{%=file.editUrl%}"
                data-obj_type="{%=file.obj_type%}" 
                data-obj_id="{%=file.obj_id%}"
                data-fields=\'["name"]\' 
                data-reload_link="#rental-photos" 
               {% if (file.deleteWithCredentials) { %} 
                data-xhr-fields=\'{"withCredentials":true}\'
                {% } %}>
                    <i class="glyphicon glyphicon-edit"></i>
                    <span>Edit</span>
                </button>
            {% } %}
            {% if (file.deleteUrl) { %}
                <button id="jfu-delete-{%=file.obj_id%}" class="btn btn-danger delete" data-type="{%=file.deleteType%}" data-toggle="confirmation" data-url="{%=file.deleteUrl%}"{% if (file.deleteWithCredentials) { %} data-xhr-fields=\'{"withCredentials":true}\'{% } %}>
                    <i class="glyphicon glyphicon-trash"></i>
                    <span>Delete</span>
                </button>
                <input type="checkbox" name="delete" value="1" data-url="{%=file.deleteUrl%}" class="{%=checkbox_class%}toggle">
            {% } else { %}
                <button class="btn btn-warning cancel">
                    <i class="glyphicon glyphicon-ban-circle"></i>
                    <span>Cancel</span>
                </button>
            {% } %}
        </td>
        <td><span style="font-size:2em;font-weight:bold;padding:10px;" class="{%=handle_class%}handle fa fa-sort"></span></td>
    </tr>
{% } %}
{% } %}
</script>
';
  if($template == 'list') return $templateAr['list'];

  return implode(PHP_EOL, $templateAr); // return all
}


/** form */
function html_form($content, $name='form', $action='', $method='post', $class='', $options = array()) {
  $options['action'] = $action; // pick_first($action, $_SERVER['REDIRECT_URL'], $_SERVER['PHP_SELF']);
  $options['method'] = strtolower(pick_first($method, 'post'));
  $options['id'] = isset($options['id']) ? $options['id'] : $name;
  return html_tag('form', $content, $class, $options, array('action', 'method', 'onsubmit'));
}


// this checkbox includes a hidden field with the same name set to 0.
// this ensures that the field is posted, even if the checkbox is not set
function form_checkbox($name, $checked=false, $options = array()) {
 if($checked) $options['checked'] = 1;
 $value = $options['value'] ?: 1;  
 //$value = 10;
 return form_input($name, 'hidden', 0, array('id' => false)).form_input($name, 'checkbox', $value, $options);
}

// if limit is set, then a text box shows how many characters you have left
function form_text($name, $value='', $options = array()) {
  $rows  = $options['rows'] ?: 5;
  $cols  = $options['cols'] ?: 40;
  
  return form_input($name, 'textarea', $value, $options);
  
  
  $js    = $options['js']; 
  $limit = $options['max'] ?: $options['limit'];
  $resizable  = isset($options['resizable']) ? $options['resizable']: true;
  $spellcheck = isset($options['spellcheck']) ? $options['spellcheck'] : true;
  $class_str = $class = $options['class'] ? " class='$class'" : '';
  $id = get_attr($js, 'id') ?: $name;
  $id_str = $id ? "id='$id'" : ''; // default id to name
  if($limit) {
   if($cols && !$rows) $rows = round($limit / $cols); // auto calculate number of rows
   $js .= " onKeyDown='limitText(\"$name\", \"".$name."_limit\", $limit);'";
  }
  
  if($limit) $ta = html_span($limit, 'tiny abs_right transparent', "id='".$name."_limit'"); //.' '.phrase('characters_left', LOWERCASE);
  
  // $ta .= "<textarea name='$name' $id_str rows='$rows' cols='$cols' $class_str $js>$value</textarea>";

  $extra_attrs = array('name', 'rows', 'cols', 'placeholder', 'maxlength', 'required', 'autofocus', 'readonly', 'disabled', 'wrap', 'form');
  $ta .= html_tag('textarea', '', '', $options, $extra_attrs);

  $str = html_div($ta, 'relative');
	if($id && $resizable) {  
  	$str .= html_script_inline('
  	$(function() {
  		$("#'.$id.'").resizable({
  			handles: "se"
  		});
  	});'."\n");
  }
  // if($spellcheck) {
  // 	$str .= html_script_inline('
  // 	$(function() {
  // 		$("#'.$id.'").spellcheck({ events: "keyup" });
  // 	});
  // 	');
  // }
  //$str .= dump($options, true).caller_stack();
  return $str;
}

// todo: use html_tag
function form_input($name, $type='text', $value='', $options = array()) {
  if(!$name) $name = $value ? url_safe($value) : 'input'; // shouldn't happen
  $options['name'] = $name;
  $options['type'] = $type;
  if(strlen($value) && !$options['value']) {
    if($value === true) $value = trim(urldecode($_GET[$name] ?: $_POST[$name])); // set from GET/POST
    $options['value'] = $value;
  }
  
  if($options['id'] !== false) { // do not show id if false
    $options['id'] =  $id = pick_first($options['id'], get_attr($js, 'id'), implode('-', array_filter(array($options['prefix'], $name))));    
  }
                                    
  // below for xhtml compat.
  if($options['readonly']) $options['readonly'] = 'readonly';
  if($options['disabled']) $options['disabled'] = 'disabled';
  if($options['checked'])  $options['checked']  = 'checked';
  if($options['max'] && !$options['maxlength']) {
    $options['maxlength'] = $options['max']; // max was introduced in HTML5. In old code, I used 'max' instead of 'maxlength': todo fix
    unset($options['max']);
  }
    
  if($label = $options['label'] ?: '') {
    unset($options['label']);
    $label = html_label($label, $id);
  }
  
  $wrapper_class = $options['wrapper_class'];
  
  //if(!isset($wrapper_class) && $_SESSION['bootstrap'] && in_array($type, array('text','textarea','select'))) {
  //  $wrapper_class = 'form-group'; // set wrapper_class to false to turn off if using Bootstrap
  //  $options['class'] = trim($options['class']." form-control"); // 100% width, bootstrap style
  //}


  $tag_value = ''; // only used for text area
  if(in_array($type, array('select', 'textarea'))) {
    $tag = $type;
    if($type == 'textarea') {
      $extra_attrs = array('name', 'rows', 'cols', 'placeholder', 'maxlength', 'required', 'autofocus', 'readonly', 'disabled', 'wrap', 'form');
      $tag_value = $value;
    } else if($type == 'select') {
      $extra_attrs = array('name', 'required', 'multiple', 'size', 'autofocus','disabled', 'form');
    }
       
  } else {
    // this list is not complete: see http://www.w3schools.com/tags/tag_input.asp
    $extra_attrs = array('type', 'name', 'value', 'size', 'max', 'maxlength', 'alt', 'align', 'autocomplete', 'autofocus', 'align', 'accept', 'placeholder', 'readonly', 'disabled', 'checked');
    $tag = 'input';
  }

  //if($type == 'textarea' && !$_SESSION['live'] && isset($_GET['debug'])) {
  //  echo("Input Form Text: $name Val:$value<br>").dump($options, true);
  //}
  
  $input = $label.html_tag($tag, $tag_value, '', $options, $extra_attrs);
  

  return $wrapper_class ? html_div($input, $wrapper_class) : $input; // wrap input + label in div

}

function form_select($name, $options = array()) {
 $rows   = $options['rows'] ?: array();
 $vars = $_POST ?: $_GET;
 if(myisset($options, 'value')) $options['selected'] = $options['value']; // can use selected or value
 $sel_key = myisset($options, 'selected') ? $options['selected'] : $vars[$name];
 return select_from_array($name, $rows, $sel_key, $options);
}

function select_from_array($name, $rows, $selected_key=0, $options = array()) {
//function select_from_array($name, $rows, $selected_key=0, $first="", $onchange="", $js="", $titles='', $id='', $data=NULL, $class='') {
 $first = $options['first'];
 $onchange = $options['onchange'];
 $js = $options['js'];
 $titles = $options['titles'];
 $id = $options['id'];
 $data = $options['data'];
 if($data && !is_array($data)) $data = array($data);
 
 $class = $options['class'];
 if(!$id) $id=$name;
 $class_str = $options['class'] ? "class='$options[class]' " : '';
 if($onchange) $onchange="onchange='$onchange'";
 $select_box .= "<select id='$id' $class_str name='$name' $disabled $onchange $js>\n";
 if($first) $select_box .= "<option value=''>$first</option>\n";
 if(!$rows) $rows = array();
 
 //if($_SESSION['dev']) {
 //  if(MDB2::isError ($rows)) dump($rows);
 //  if(!$rows && !$first) echo("select_from_array: empty array for $name<br>");
 // //mydie();
 //}
 $keys = array_keys($rows);
 $i=0;
 $count = count($rows);
 //dump($rows);
 
 foreach($rows as $row) {  
  if(is_array($data) && count($data) && count($row) > 2) {
   $data_str = '';
   $j=2;
   foreach($data as $fld) {
    if(count($row > $j)) {
     $data_str .= " data-$fld=\"$row[$j]\"";
     unset($row[$j]);
    }
    $j++;
   }
  }

  if(is_array($row)) { // array of arrays
   //dump($row);
   $id = $row[0];
   array_shift($row); // remove first (ID);
   $value = implode(" ", $row);
  } else { // associative array
   $id = array_shift($keys);
   $value = $row;
  }
  if(isset($options['last_suffix']) && $i == $count-1) $value .= $options['last_suffix'];
  if(!$_SESSION['live'] && is_object($value)) {
    mydie("select_from_array($name): Value is object:".dump($value, true));
  }
    
  if(!trim($value) && $id) $value = " ($id) ";
  $selected = strcmp($id, $selected_key)==0 ? " selected='selected'" : "";
  $title = ($t = $titles[$i]) ? " title='".$t."'" : '';
  $select_box .= "<option value='$id'$title$selected$data_str>$value</option>\n";
  $i++;
 }
 $select_box .= "</select>";
 return $select_box;
}

/** used in /db_object.inc on line 2278 */
// same as form_ajax_chooser, but using the new jQuery UI 1.8 autocomplete widget
function form_autocomplete($name, $search_options, $options=array()) {
  global $__APP;
  $app = $__APP;
  $__header = $app->header;
  $ac_options = array();  

  //dump($options);
  $search_options['value_field'] = $search_options['value_field'] ?: 'name'; // DB field we search for 
  $search_options['id_field'] = $search_options['id_field'] ?: 'id';  // value of DB row stored in hidden alt field
  
  $alt_field = $search_options['id_field'] == $search_options['value_field'] ? false : true; // do we keep value in hidden field ?  
  $input_id = $alt_field ? $name.'_textbox' : $name;
  
  // additional form fields
  $type = $options['debug'] && !$_SESSION['live'] ? 'text' : 'hidden';
  $type = 'hidden';
  $extra_fields = '';
  if($alt_field) {
    $value = pick_first($options['value'], trim(urldecode($_GET[$name] ?: $_POST[$name])));
    $extra_fields .= form_input($name, $type, $value, array('placeholder' => $options['placeholder'] ?: name)); // stores the hidden value posted
  }

  if($search_options['google_autocomplete']) { // integrate result with google places autocomplete
    $__header->styles[] = '/afr/css/google_pac.css';        
    $__header->styles[] = '//fonts.googleapis.com/css?family=Roboto:300,400,500';
    $__header->scripts[] = GMAP_API; 
    $__header->scripts[] = "/afr/js/google_pac.js?ver={$_SESSION['version']}";      
    $__header->onload[] = "google.maps.event.addDomListener(window, 'load', initialize);";
    $extra_fields .= form_input('location_lat', $type, true, array('placeholder' => 'lat'));
    $extra_fields .= form_input('location_long', $type, true, array('placeholder' => 'long'));
    $extra_fields .= form_input('bounds', $type, true, array('placeholder' => 'bounds', 'data-target' => $name, 'data-target_display' => $input_id));
    $ac_options['select_handler'] = 'google_pac_select';
    $ac_options['response_handler'] = 'google_pac_response';
    $ac_options['item_renderer'] = 'google_pac_renderer';
  }
  if($alt_field) {
    if(!$ac_options['select_handler']) $ac_options['select_handler'] = 'autocomplete_alt_select';
    $options['data-target'] = $name; // ID of alt field
  }
  if($ac_options) $options['data-options'] = json_encode($ac_options);
  $options['data-search_options'] = json_encode($search_options);
  $options['class'] = trim("jq-autocomplete ".$options['class']);
  $options['autocomplete'] = "off";

  //if($options['image'] !== false) $image = $options['image'] ?: '/images/icons/suggest_bg.png';
  $image = '';
  $value = pick_first($options['display_value'], trim(urldecode($_GET[$input_id] ?: $_POST[$input_id])), $options['value']);
  unset($options['value']);
  if($image) {
    $output =form_image_input($input_id, $image, $value, $options);
  } else { /** use bootstrap now */    
    $output = form_input($input_id, 'text', $value, $options); // shows the value printed
  }
  
  return $output.$extra_fields;
}


/** only used in soon to be deprecated function gallery_object_list_item in helpers_gallery */
// takes an array of columns (minimum 2), optionally preceeded by $headers and followed by $footers and displays them side by side
// if an array element (a column) is an array, then they are imploded with <br>
// if class is array, the 1st element is for the 1st column etc (none for header/footer)
function css_columns($columns, $header_row='', $footer_row='', $classes='', $js='', $glue='__none') {
 if($glue == '__none') $glue = html_break();
 $output = html_div($header_row, "header_row ".($classes && !is_array($classes) ? " $classes" : ""), $js);
 $count = count($columns);

 if($columns) {
  $class = is_array($classes) ? array_shift($classes) : $classes;
  $output .= html_div(smart_implode(array_shift($columns), $glue), "left_column ".($class ? " $class" : ""), $js);

  $class = is_array($classes) ? array_shift($classes) : $classes;
  $output .= html_div(smart_implode(array_shift($columns), $glue), "middle_column ".($class ? " $class" : ""), $js); // always at least one middle column for left alignment
  
  while(count($columns) > 1) {
   $class = is_array($classes) ? array_shift($classes) : $classes;
   $output .= html_div(smart_implode(array_shift($columns), $glue), "middle_column ".($class ? " $class" : ""), $js);
  }

  $class = is_array($classes) ? array_pop($classes) : $classes;
  $output .= html_div(smart_implode(array_pop($columns), $glue), "right_column ".($class ? " $class" : ""), $js); // always a right column, even if empty
 }
 $output .= html_div($footer_row, "footer_row ".($classes && !is_array($classes) ? " $classes" : ""), $js);
 //return html_div($output, 'border');
 return $output;
}

// shows color name + in actual color
// if fld is set, shows color picker
// if update_bg or update_fg is set, all elements of that class or id will automatically have their foreground color updated - use .classname for class or #id for id
function show_color($val, $fld='', $update_fg='', $update_bg='') {
 if(!is_color($val)) return $val;
 $val = ltrim($val, '#');
 $brightness = get_brightness($val);
 $fg = $brightness < 0.5 ? "FFF" : "000"; // pick suitable fg color
	if($fld) {
 	$output = "# <input id='$fld' class='colorpicker_input' type='text' maxlength='6' name='$fld' value='$val' style='color:#$fg;background-color:#$val;width:75px;' />";
 } else {
 	$output = "<span style='color:#$fg;background:#$val;width:55px;'>#$val</span>";
 }

 $script = "
$('#".$fld."').ColorPicker({
	onSubmit: function(hsb, hex, rgb, el) {
		$(el).val(hex);
		$(el).ColorPickerHide();
 	var brightness = (rgb.r+ rgb.g + rgb.b) / 765;
 	var fg = brightness < 0.5 ? '#FFFFFF' : '#000000';
 	$('#".$fld."').css('color', fg);
 	$('#".$fld."').css('background-color', '#' + hex);
".($update_fg ? " 	$('".$update_fg."').css('color', '#' + hex);" : "")."
".($update_bg ? " 	$('".$update_bg."').css('background-color', '#' + hex);" : "")."
	},
	onBeforeShow: function () {
		$(this).ColorPickerSetColor(this.value);
	},

	onChange: function (hsb, hex, rgb) {
		$('#".$fld."').val(hex);
		$('#".$fld."').css('background-color', '#' + hex);
".($update_fg ? " 	$('".$update_fg."').css('color', '#' + hex);" : "")."
".($update_bg ? " 	$('".$update_bg."').css('background-color', '#' + hex);" : "")."
	}

})
.bind('keyup', function(){
	$(this).ColorPickerSetColor(this.value);
});
";
 $output .= html_script_inline($script);

 return $output;
}

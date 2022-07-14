console.log("loading afr.js 201911200225");

if(typeof g_ready_scripts == "undefined") var g_ready_scripts = [];
g_ready_scripts.push('afr_ready');

var g_date_format = "dd-mm-yyyy";
var g_moment_format = "DD-MM-YYYY";
var g_moment_sql_format = "YYYY-MM-DD";
var g_rates = {};
var g_dp_options = {"autoclose": 1, "weekStart": 1, "format": g_date_format}
var g_fc_dates = {};
var g_subview_set = 0;
var g_form_dirty = 0;

if($.fn.datepicker && $.fn.datepicker.defaults) $.fn.datepicker.defaults.format = g_date_format;

if (!window.console) window.console = {};
if (!window.console.log) window.console.log = function () { };

//console.log = function() {} // when live to disable 

var bootstrap_enabled = (typeof $().modal == 'function');
var bootstrap3_enabled = (typeof $().emulateTransitionEnd == 'function');
  
function bdpBeforeShowDay(date) {
  console.log("bdpBeforeShowDay: date=", date);
}

// bootstrap toggle error function for form validation
$.fn.toggleInputError = function(erred) {
  this.parent('.form-group').toggleClass('has-error', erred);
  return this;
};

/**
if(window.addEventListener) document.addEventListener('DOMMouseScroll', stopScroll, false);
document.onmousewheel = stopScroll;
function stopScroll() {
  //console.log("Scroll stop");
  $('html, body').stop();
  $(window).stop(true, false);  // Stops and dequeue's animations
}
*/

function in_iframe() {
  try {
    return window.self !== window.top;
  } catch (e) {
    return true;
  }
}

function isDefined(thevar){
   return (typeof(thevar) == 'undefined') ? false : true;
}

function isExisting(obj){
  return typeof(obj)!='undefined';
}

//returns true is it is an array
function isArray(obj) {
  return (obj.constructor.toString().indexOf("Array") == -1) ? false : true;
}

function is_array(input){
  return typeof(input)=='object' && (input instanceof Array);
}

function in_array(needle, haystack) {
  var length = haystack.length;
  for(var i = 0; i < length; i++) {
      if(haystack[i] == needle) return true;
  }
  return false; 
}

// return a clone of object members (not prototypes);
function clone_object(obj) {
  return JSON.parse(JSON.stringify(obj));
}

function truncate(str, len, elipsis) {
  if(!str) return str;
  if(str.length <= len) return str;
  if(typeof elipsis == "undefined") var elipsis = "...";
  return str.substring(0, len) + elipsis;
}

// http://kevin.vanzonneveld.net
function is_object (mixed_var) {
  if (Object.prototype.toString.call(mixed_var) === '[object Array]') {
    return false;
  }
  return mixed_var !== null && typeof mixed_var == 'object';
}

// array unique function
// https://stackoverflow.com/questions/1584370/how-to-merge-two-arrays-in-javascript-and-de-duplicate-items
Array.prototype.unique = function() {
  var a = this.concat();
  for(var i=0; i<a.length; ++i) {
    for(var j=i+1; j<a.length; ++j) {
      if(a[i] === a[j])
        a.splice(j--, 1);
    }
  }
  return a;
};


// Thanks http://stackoverflow.com/questions/5223/length-of-a-javascript-object-that-is-associative-array
Object.size = function(obj) {
  var size = 0, key;
  for (key in obj) {
      if (obj.hasOwnProperty(key)) size++;
  }
  return size;
};

// Thanks John Resig
Array.max = function( array ){
    return Math.max.apply( Math, array );
};
 
Array.min = function( array ){
    return Math.min.apply( Math, array );
};

// use to search/replace and preserve case:

// http://stackoverflow.com/questions/17264639/replace-text-but-keep-case
function matchCase(text, pattern) {
  var result = '';

  for(var i = 0; i < text.length; i++) {
    var c = text.charAt(i);
    var p = pattern.charCodeAt(i);

    if(p >= 65 && p < 65 + 26) {
      result += c.toUpperCase();
    } else {
      result += c.toLowerCase();
    }
  }

  return result;
}

function form_input_is_int(input) {
  return !isNaN(input)&&parseInt(input)==input;
}

function currency2symbol(currency) {
  var currency_symbols = {
      'USD': '$', // US Dollar
      'EUR': '€', // Euro
      'CRC': '₡', // Costa Rican Colón
      'GBP': '£', // British Pound Sterling
      'ILS': '₪', // Israeli New Sheqel
      'INR': '₹', // Indian Rupee
      'JPY': '¥', // Japanese Yen
      'KRW': '₩', // South Korean Won
      'NGN': '₦', // Nigerian Naira
      'PHP': '₱', // Philippine Peso
      'PLN': 'zł', // Polish Zloty
      'PYG': '₲', // Paraguayan Guarani
      'THB': '฿', // Thai Baht
      'UAH': '₴', // Ukrainian Hryvnia
      'VND': '₫', // Vietnamese Dong
      'BTC': '฿', // Vietnamese Dong
  };
  var symbol = currency_symbols[currency];
  if(!symbol) return currency;
  return symbol;
}

// http://phpjs.org/functions/extract/
function extract (arr, type, prefix) {
  // From: http://phpjs.org/functions
  // +   original by: Brett Zamir (http://brett-zamir.me)
  // %        note 1: Only works by extracting into global context (whether called in the global scope or
  // %        note 1: within a function); also, the EXTR_REFS flag I believe can't be made to work
  // *     example 1: size = 'large';
  // *     example 1: var_array = {'color' : 'blue', 'size' : 'medium', 'shape' : 'sphere'};
  // *     example 1: extract(var_array, 'EXTR_PREFIX_SAME', 'wddx');
  // *     example 1: color+'-'+size+'-'+shape+'-'+wddx_size;
  // *     returns 1: 'blue-large-sphere-medium'
  if (Object.prototype.toString.call(arr) === '[object Array]' &&
    (type !== 'EXTR_PREFIX_ALL' && type !== 'EXTR_PREFIX_INVALID')) {
    return 0;
  }
  var targetObj = this.window;
  if (this.php_js && this.php_js.ini && this.php_js.ini['phpjs.extractTargetObj'] && this.php_js.ini['phpjs.extractTargetObj'].local_value) { // Allow designated object to be used instead of window
    targetObj = this.php_js.ini['phpjs.extractTargetObj'].local_value;
  }
  var chng = 0;

  for (var i in arr) {
    var validIdent = /^[_a-zA-Z$][\w|$]*$/; // TODO: Refine regexp to allow JS 1.5+ Unicode identifiers
    var prefixed = prefix + '_' + i;
    try {
      switch (type) {
      case 'EXTR_PREFIX_SAME' || 2:
        if (targetObj[i] !== undefined) {
          if (prefixed.match(validIdent) !== null) {
            targetObj[prefixed] = arr[i];
            ++chng;
          }
        } else {
          targetObj[i] = arr[i];
          ++chng;
        }
        break;
      case 'EXTR_SKIP' || 1:
        if (targetObj[i] === undefined) {
          targetObj[i] = arr[i];
          ++chng;
        }
        break;
      case 'EXTR_PREFIX_ALL' || 3:
        if (prefixed.match(validIdent) !== null) {
          targetObj[prefixed] = arr[i];
          ++chng;
        }
        break;
      case 'EXTR_PREFIX_INVALID' || 4:
        if (i.match(validIdent) !== null) {
          if (prefixed.match(validIdent) !== null) {
            targetObj[prefixed] = arr[i];
            ++chng;
          }
        } else {
          targetObj[i] = arr[i];
          ++chng;
        }
        break;
      case 'EXTR_IF_EXISTS' || 6:
        if (targetObj[i] !== undefined) {
          targetObj[i] = arr[i];
          ++chng;
        }
        break;
      case 'EXTR_PREFIX_IF_EXISTS' || 5:
        if (targetObj[i] !== undefined && prefixed.match(validIdent) !== null) {
          targetObj[prefixed] = arr[i];
          ++chng;
        }
        break;
      case 'EXTR_REFS' || 256:
        throw 'The EXTR_REFS type will not work in JavaScript';
      case 'EXTR_OVERWRITE' || 0:
        // Fall-through
      default:
        targetObj[i] = arr[i];
        ++chng;
        break;
      }
    } catch (e) { // Just won't increment for problem assignments
    }
  }
  return chng;
}


// following 2 from http://www.andrewpeace.com/javascript-is-int.html
function is_int(input) {
  return typeof(input)=='number'&&parseInt(input)==input;
}

function remove_null(obj_or_array) {
  for (i in obj_or_array) {
    //alert("i=" + i + " val=" + obj_or_array[i] + ' is null=' + (obj_or_array[i] === null ? "true" : "false"));
    if (obj_or_array[i] === null || obj_or_array[i] === undefined) {      
      delete obj_or_array[i];
    }
  }
  return obj_or_array;
}

// finds object(s) with key=val in obj (or array)
function findObjects(obj, key, val) {
  var objects = [];
  for (var i in obj) {
    if (!obj.hasOwnProperty(i)) continue;
    if (typeof obj[i] == 'object') {
      objects = objects.concat(findObjects(obj[i], key, val));
    } else if (i == key && obj[key] == val) {
      objects.push(obj);
    }
  }
  return objects;
}

function debug(str) {
  log(str);
  return;
}

// takes a {top,left} pair and a grid sizer (top;left), crops position to fit bounds
function position_bounds(pos, size) {
  return {"top": math_clip(pos.top, size.top-1, 0), "left": math_clip(pos.left, size.left-1, 0)} 
}
  
// clips num to min and max
function math_clip(num, max, min) {
  if(!min) min = 0;
  return num < min ? min : Math.min(num, max);
}

function math_truncate(num) {
  return num < 0 ? Math.ceil(num) : Math.floor(num);
}

function log() {
  if(arguments.length > 0) { 
    // join for graceful degregation         
    var args = (arguments.length > 1) ? Array.prototype.join.call(arguments, " ") : arguments[0];          
    
    // this is the standard; firebug and newer webkit browsers support this        
    try {
      window.console && console.log(args);
      return true;
    } catch(e) {
      // newer opera browsers support posting erros to their consoles 
      try {
        opera.postError(args);
        return true;
      } catch(e) { }
    } 

    // catch all; a good old alert box         
    alert(args);
    return false;
  }
} 

             
// takes a jquery object, splits id, returns 2nd part
// so foo_233 returns 233
function get_target_id($target) {
  var tid = $target.prop("id");
  var ar = tid.split("_");
  return ar[1];
}
                  
// removes empty elements from form before submitting (makes for cleaner query string.)
function cleanEmptyFields(theform) {
  //return;
  //var theform = thebutton.form;
  //return true;    
  var fs='';
  for(i=0; i<theform.elements.length; i++) {
    var fldName = theform.elements[i].name;
    //alert(fldName + " = " + theform.elements[i].value); 
    if(theform.elements[i].value == '') theform.elements[i].disabled = true; 
  }
  return true;    
}


// takes a query string and returns object with keys/values
// expects only the bit after the ?
// so first use: var queryString = url.substring( query.indexOf('?') + 1); 
// Thanks http://www.joezimjs.com/javascript/3-ways-to-parse-a-query-string-in-a-url/
function parseQueryString (queryString) {
  var params = {}, queries, temp, i, l;
  queries = queryString.split("&"); // Split into key/value pairs
 
  // Convert the array of strings into an object
  for ( i = 0, l = queries.length; i < l; i++ ) {
    temp = queries[i].split('=');
    params[temp[0]] = temp[1];
  } 
  return params;
}

// thanks http://stackoverflow.com/questions/5999118/add-or-update-query-string-parameter
function replace_query_var(uri, key, value) {
  if(uri == '#') return "?" + key + "=" + encodeURIComponent(value);
  var hash = '';  
  if(uri.indexOf('#') > 0) { // handle hash
    var parts = uri.split('#');
    uri = parts[0];
    hash = "#" + parts[1];
    console.log("uri=" + uri + " hash=" + hash);
  }
  var re = new RegExp("([?&])" + key + "=.*?(&|$)", "i");
  var separator = uri.indexOf('?') !== -1 ? "&" : "?";
  if (uri.match(re)) {
    return uri.replace(re, '$1' + key + "=" + encodeURIComponent(value) + '$2') + hash;
  } else {
    return uri + separator + key + "=" + encodeURIComponent(value) + hash;
  }
}

function hideEl(el){
  //return;
  if (document.getElementById(el)){
        document.getElementById(el).style.display = 'none';
  } else {
    //alert('hideEl: no element'+el);
  }
}

function showEl(el){
  //alert('showEl:'+el);
  if (document.getElementById(el)){
      document.getElementById(el).style.display = 'block';
  } else {
    alert('showEl: no element'+el);                              
  }
}

// limits input to text field and show # characters left
function limitText(limitField, limitCount, limitNum) {
  var $limitField = $('#'+limitField);
  var $limitCount = $('#'+limitCount);
  if ($limitField.val().length > limitNum) {
    $limitField.val($limitField.val().substring(0, limitNum));
  } else {
    $limitCount.html(limitNum - $limitField.val().length);
  }
}

function autocomplete_off(el_id) { // hack since autocomplete='off' is not strict (x)html
  if(el = document.getElementById(el_id)) el.setAttribute("autocomplete", "off");
}

// helper function for print_message()
function message_icon(style) {
  switch(style){
    case "info":  return "fa fa-info-circle text-info";
    case "warn":  return "fa fa-exclamation-circle text-warning";
    case "warning":  return "fa fa-exclamation-circle text-warning";
    case "help":  return "fa fa-question-circle text-primary";
    case "error":  return "fa fa-exclamation-triangle text-danger";
    case "critical":  return "fa fa-exclamation-triangle text-danger fa-spin";
    case "check":  return "fa fa-check-circle text-success";
    case "success":  return "fa fa-check-circle text-success";
    default:  return "";
  }
}

// prints message. supported styles: help,info,warn(ing), error/critical, success
function print_message(message, style) {
  var valid_styles = ['info','help','warning','error','success'];
  var valid = $.inArray(style, valid_styles) > -1;  
  var msg_class = message_icon(style);
  var icon = valid ? "<i class='msgicon fa-4x " + msg_class + "'></i>" : '';  
  var div_class = valid ? "custom " + style : "classic"; 
  return "<div class='" + div_class + "'>" + icon + "<span>" + message + "</span></div>";
}

/* Function : dump()  http://www.openjs.com/scripts/others/dump_function_php_print_r.php */
function dumpArr(arr,level) {
  var dumped_text = "";
  if(!level) level = 0;
  
  //The padding given at the beginning of the line.
  var level_padding = "";
  for(var j=0;j<level+1;j++) level_padding += "    ";
  
  if(typeof(arr) == 'object') { //Array/Hashes/Objects 
    for(var item in arr) {
      var value = arr[item];
      
      if(typeof(value) == 'object') { //If it is an array,
        dumped_text += level_padding + "'" + item + "' ...\n";
        dumped_text += dump(value,level+1);
      } else {
        dumped_text += level_padding + "'" + item + "' => \"" + value + "\"\n";
      }
    }
  } else { //Stings/Chars/Numbers etc.
    dumped_text = "===>"+arr+"<===("+typeof(arr)+")";
  }
  return dumped_text;
}

/* repeatString() returns a string which has been repeated a set number of times */ 
function repeatString(str, num) {
  out = '';
  for (var i = 0; i < num; i++) {
    out += str; 
  }
  return out;
}

// Thanks: http://stackoverflow.com/questions/603987/what-is-the-javascript-equivalent-of-var-dump-or-print-r-in-php
/*
dump() displays the contents of a variable like var_dump() does in PHP. dump() is
better than typeof, because it can distinguish between array, null and object.  
Parameters:
  v:        The variable
  howDisplay:   "none" or blank or "text", "body" or "html", "alert" (default)
  recursionLevel: Number of times the function has recursed when entering nested
          objects or arrays. Each level of recursion adds extra space to the 
          output to indicate level. Set to 0 by default.
Return Value:
  A string of the variable's contents 
Limitations:
  Can't pass an undefined variable to dump(). 
  dump() can't distinguish between int and float.
  dump() can't tell the original variable type of a member variable of an object.
  These limitations can't be fixed because these are *features* of JS. However, dump()
*/
function dump(v, howDisplay, recursionLevel) {
  howDisplay = (typeof howDisplay === 'undefined' || howDisplay == '' || howDisplay == 'text') ? "none" : howDisplay;
  recursionLevel = (typeof recursionLevel !== 'number') ? 1 : recursionLevel;


  var vType = typeof v;
  var out = vType;

  switch (vType) {
    case "number":
      /* there is absolutely no way in JS to distinguish 2 from 2.0
      so 'number' is the best that you can do. The following doesn't work:
      var er = /^[0-9]+$/;
      if (!isNaN(v) && v % 1 === 0 && er.test(3.0))
        out = 'int';*/
    case "boolean":
      out += ": " + v;
      break;
    case "string":
      out += "(" + v.length + '): "' + v + '"';
      break;
    case "object":
      //check if null
      if (v === null) {
        out = "null";

      }
      //If using jQuery: if ($.isArray(v))
      //If using IE: if (isArray(v))
      //this should work for all browsers according to the ECMAScript standard:
      else if (Object.prototype.toString.call(v) === '[object Array]') {  
        out = 'array(' + v.length + '): {\n';
        for (var i = 0; i < v.length; i++) {
          out += repeatString('   ', recursionLevel) + "   [" + i + "]:  " + 
            dump(v[i], "none",  recursionLevel + 1) + "\n";
        }
        out += repeatString('   ', recursionLevel) + "}";
      }
      else { //if object  
        sContents = "{\n";
        cnt = 0;
        for (var member in v) {
          //No way to know the original data type of member, since JS
          //always converts it to a string and no other way to parse objects.
          sContents += repeatString('   ', recursionLevel) + "   " + member +
            ":  " + dump(v[member], "none", recursionLevel + 1) + "\n";
          cnt++;
        }
        sContents += repeatString('   ', recursionLevel) + "}";
        out += "(" + cnt + "): " + sContents;
      }
      break;
  }

  if (howDisplay == 'body' || howDisplay == 'html') {
    var pre = document.createElement('pre');
    pre.innerHTML = out;
    document.body.appendChild(pre)
  }
  else if (howDisplay == 'alert') {
    alert(out);
  }

  return out;
}


// shorthand for currency formatting
function formatFloat(value, precision) {
  if(typeof precision == "undefined") var precision = 2;
  return toFixedCond(value, precision);
}

// Numeric helpers 
// like toFixed (but w/o IE problem where 0.9 becomes 0 and with proper rounding)
// if integer, returns integer (thus 4.00 => 4)
function toFixedCond(value, precision) {
  var dec=parseFloat(value) - parseInt(value);
  if(!dec) return parseInt(value);
  else return toFixedUncond(value, precision);
}

function toFixedUncond(value, precision) {
  var power = Math.pow(10, precision || 0);
  var result = Math.round(value * power) / power; // round to given precision  
  return result.toFixed(precision); 
}

// if any form field changes, uses will be warned if trying to leave the page
var needToConfirm = false;
var formChangedFlag = false; 
window.onbeforeunload = confirmExit;

function confirmExit() {
  return;
  if (!needToConfirm || !formChangedFlag) return;
  return "You have attempted to leave this page. If you have madeAre you sure any changes to the fields without Saving, your changes will be lost. Are you sure you want to exit this page?";
}

function nl2br(str) {
  return str.replace(/(?:\r\n|\r|\n)/g, '<br />');
}

function d2h(d) {return d.toString(16);} // dec 2 hex
function h2d(h) {return parseInt(h,16);} // hex 2 dec


function capitalize(str) {
  return ucfirst(str);
}

function ucfirst(str){
  return str.charAt(0).toUpperCase() + str.slice(1);
};

/** simplified version: for full RFC 2822, see http://stackoverflow.com/questions/46155/validate-email-address-in-javascript */
function isEmail(em) {
  var re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
  return re.test(em);
}

/*
JSTarget function by Roger Johansson, www.456bereastreet.com
http://www.456bereastreet.com/archive/200610/opening_new_windows_with_javascript_version_12/
*/
var JSTarget = {
init: function(att,val,warning) {
if(!text_new_window) var text_new_window = '(Opens in new window)';
if (document.getElementById && document.createElement && document.appendChild) {
var strAtt = ((typeof att == 'undefined') || (att == null)) ? 'class' : att;
var strVal = ((typeof val == 'undefined') || (val == null)) ? 'h' : val;
var strWarning = ((typeof warning == 'undefined') || (warning == null)) ? ' ('+ text_new_window +')' : warning;
var oWarning;
var arrLinks = document.getElementsByTagName('a');
var oLink;
var oRegExp = ("(^|\\s)" + strVal + "(\\s|$)");
for (var i = 0; i < arrLinks.length; i++) {
oLink = arrLinks[i];
if ((strAtt == 'class') && (oRegExp.test(oLink.className)) || (oRegExp.test(oLink.getAttribute(strAtt)))) {
oWarning = document.createElement("em");
oWarning.appendChild( document.createTextNode(strWarning) );
oLink.appendChild(oWarning);
oLink.onclick = JSTarget.openWin;
}
oWarning = null;
}
}
},
openWin: function(e) {
var event = (!e) ? window.event : e;
if (event.shiftKey || event.altKey || event.ctrlKey || event.metaKey) return true;
else {
var oWin = window.open(this.getAttribute('href'), '_blank');
if (oWin) {
if (oWin.focus) oWin.focus();
return false;
}
oWin = null;
return true;
}
},
/*
addEvent function from http://www.quirksmode.org/blog/archives/2005/10/_and_the_winner_1.html
*/
addEvent: function(obj, type, fn) {
if (obj.addEventListener)
obj.addEventListener(type, fn, false);
else if (obj.attachEvent) {
obj["e"+type+fn] = fn;
obj[type+fn] = function() {obj["e"+type+fn]( window.event );}
obj.attachEvent("on"+type, obj[type+fn]);
}
}
};

// end JS Target
// end base1.js


// start datetime.js 

// date/time helpers 

// takes a standard UTC date (Javascript), returns local Date-time
function get_local_time(date) {      
  var offset = date.getTimezoneOffset();
  var ts = date.valueOf();
  var ts_local = ts + (offset*60000); // in msec
  var date_local = new Date(ts_local);
  return date_local;
}


function date2sql(date) {
  var dateStr = date.getFullYear() + '-' + padStr(1 + date.getMonth()) + '-' + padStr(date.getDate());
  return dateStr;
//                padStr(temp.getHours()) +
//                padStr(temp.getMinutes()) +
//                padStr(temp.getSeconds());
}

function padStr(i) {
  return (i < 10) ? "0" + i : "" + i;
}

/** new: same as our own PHP function - 
 expects: any date moment can parse  
 returns sql format YYYY-MM-DD 
*/
function date_today() {
  return moment().format(g_moment_sql_format);
}

function add_day(date, num_days) {
  return moment(date).add(num_days, 'days').format(g_moment_sql_format);
}

function add_month(date, num_months) {
  return moment(date).add(num_months, 'months').format(g_moment_sql_format);
}

function add_year(date, num_years) {
  return moment(date).add(num_years, 'years').format(g_moment_sql_format);
}

function add_days(start_date, num_days) {
  var sd = new Date(start_date.getTime() + num_days * 1000 * 3600 * 24);
  return sd;
}

function add_hours(start_date, num_hours) {
  var sd = new Date(start_date.getTime() + num_hours * 1000 * 3600);
  return sd;
}

// returns Thursay September 24, 2009
function sql2human(dateStr) {
  //return "foo";
  if(!dateStr) return 'N/A';
  if(!is_date(dateStr)) return dateStr; // not sql, already formatted?
  
  var momdate = sql2date(dateStr);
  return momdate.format(g_moment_format);
  
  /** old code */
  var date = sql2date(dateStr);
  return date.toLocaleDateString();
}

// returns 24 Sep 2009
function sql2human_short(dateStr) {
  if(!dateStr) return 'N/A';

  var momDate = sql2date(dateStr);
  return momDate.format("D MMM YYYY");

  /** old */
  var date = sql2date(dateStr);
  return date.getDate() + " " + monthNamesShort[date.getMonth()] + " " + date.getFullYear();
}

// returns 24 Sep
function sql2human_shortest(dateStr) {
  if(!dateStr) return 'N/A';
  var momDate = sql2date(dateStr);
  return momDate.format("D MMM");
  /** old */
  return date.getDate() + " " + monthNamesShort[date.getMonth()];
}

// returns Thu 24 Sep 2009
function sql2human_short_day(dateStr) {
  if(!dateStr) return 'N/A';
  var date = sql2date(dateStr);
  return dayNamesShort[date.getDay()] + " " + date.getDate() + " " + monthNamesShort[date.getMonth()] + " " + date.getFullYear();
}

// returns Thu 24 Sep 10:30
function sql2human_short_day_time(dateStr) {
  if(!dateStr) return 'N/A';
  var momDate = sql2datetime(dateStr);
  return momDate.format("D MMM HH:mm");

  /** old */
  
  var date = sql2date(dateStr);
  var date_str = dayNamesShort[date.getDay()] + " " + date.getDate() + " " + monthNamesShort[date.getMonth()];
  if(strip_date(dateStr) && strip_date(dateStr) != '00:00:00') date_str = date_str + ' ' + zero_pad(date.getHours()) + ":" + zero_pad(date.getMinutes());
  return date_str;
}

// returns 10:30
function sql2human_short_time(dateStr) {
  if(!dateStr) return 'N/A';
  var momDate = sql2datetime(dateStr);
  //console.log("ds=" + dateStr);
  return momDate.format("HH:mm");

  /** old */
  var date = sql2date(dateStr);
  return zero_pad(date.getHours()) + ":" + zero_pad(date.getMinutes());
}

// prepends zero to day/month/hour/minute if < 10
function zero_pad(val) {
  var str = val.toString();
  return str.length < 2 ? '0' + str : str;
}

// by kj
function fixDate(dateStr) {
  if(!dateStr) return;
  var datePat = /^(\d{4})(\/|-)(\d{1,2})(\/|-)(\d{1,2})$/;
  var matchArray = dateStr.match(datePat); // is the format ok?
  if(!matchArray) return '';
  year = matchArray[1]; // p@rse date into variables
  month = matchArray[3];
  day = matchArray[5];
  if(day.length<2)
     day = "0" + day;
  if(month.length<2)
     month = "0" + month;
  return year + "-" + month + "-" + day;
}

function sql2ymd(inputDate) {
  var dateTimeArray = inputDate.split(' ');
  var dateString = dateTimeArray[0]
  var pattern = /(^\d{4})-(\d{2})-(\d{2}$)/
  var result = dateString.match(pattern);
  return {y: result[1], m: result[2], d: result[3]}
}

function sql2datetime(inputDate) {
  if(!inputDate) return '<NAD>';
  if(is_date(inputDate)) return moment(inputDate, "YYYY-MM-DD HH:mm:ss");
  return moment(inputDate, g_moment_format);
}

function sql2date(inputDate) {
  if(!inputDate) return '<NAD>';
  //if(is_date(inputDate)) return moment(inputDate, "YYYY-MM-DD");
  return moment(inputDate, g_moment_sql_format);
  
  /** old code below */
  // mysql date time as YYYY-MM-DD hh:mm:ss
  var dateTimeArray = inputDate.split(' ');
  var dateString = dateTimeArray[0]
  var timeString = dateTimeArray[1]

  // parse the date from YYYY-MM-DD
  var pattern = /(^\d{4})-(\d{2})-(\d{2}$)/
  var result = dateString.match(pattern);

  var year = result[1];
  var month = result[2];
  var day = result[3];
    
  var date = new Date(year, month - 1, day, 0, 0, 0)
  
  if(timeString) {
    // now parse the time from HH:MM:SS
    pattern = /(^\d{2}):(\d{2}):(\d{2})/
    result = timeString.match(pattern);
    
    var hour   = result[1];
    var minute = result[2];
    var second = result[3];
      
    date.setHours(hour, minute, second);
  }
  
  return date;
}

function sql2ms(dateStr) {
  var momData = sql2datetime(dateStr);
  //console.log("sql2ms: date=" + dateStr);
  return momData.millisecond();
  
  /** old below */
  var date1 = sql2date(dateStr);
  return date1.getTime();
}

function hours_between(datein, dateout) {
  // alert("in=" + datein + " out=" + dateout); 
  var one_hour = 1000 * 60 * 60;
  return time_between(datein, dateout, one_hour);
}

function days_between(datein, dateout) {
  var one_day = 1000 * 60 * 60 * 24;
  return time_between(datein, dateout, one_day);
}

// todo: consider # days in month
function months_between(datein, dateout) {
  var one_month = 1000 * 60 * 60 * 24 * 30.41666;
  return time_between(datein, dateout, one_month);
}


// returns number of units between 2 days (sql format) (e.g. days/hours/months)
function time_between(datein, dateout, unit_in_ms) {
  if(!unit_in_ms) unit_in_ms = 1000; // default is 1 second
  return (sql2ms(dateout) - sql2ms(datein)) / unit_in_ms;  
  
  // return round_result ? Math.round(difference_ms/unit) : difference_ms/unit;
}

// jquery plugins

// newWindow. From: http://stackoverflow.com/questions/233467/whats-the-best-way-to-open-new-browser-window

(function($){  
  $.fn.newWindow = function(options) {       
    var defaults = {
        titleText: 'Link opens in a new window'         
    };

    var options = $.extend(defaults, options);

     return this.each(function() {  
       var obj = $(this);

       if (options.titleText) {            
           if (obj.prop('title')) {
                     var newTitle = obj.prop('title') + ' (' 
                                                + options.titleText + ')';
           } else {
                    var newTitle = options.titleText;
           };                      
           obj.prop('titls', newTitle);                    
       };          

       obj.click(function(event) {
          event.preventDefault();  
          var newBlankWindow = window.open(obj.prop('href'), '_blank');
          newBlankWindow.focus();
        });     
       });    
  };  
 })(jQuery);

 
/*
 * jQuery Hotkeys Plugin
 * Copyright 2010, John Resig
 * Dual licensed under the MIT or GPL Version 2 licenses.
 *
 * Based upon the plugin by Tzury Bar Yochay:
 * http://github.com/tzuryby/hotkeys
 *
 * Original idea by:
 * Binny V A, http://www.openjs.com/scripts/events/keyboard_shortcuts/
*/

(function(jQuery){
  
  jQuery.hotkeys = {
    version: "0.8",

    specialKeys: {
      8: "backspace", 9: "tab", 13: "return", 16: "shift", 17: "ctrl", 18: "alt", 19: "pause",
      20: "capslock", 27: "esc", 32: "space", 33: "pageup", 34: "pagedown", 35: "end", 36: "home",
      37: "left", 38: "up", 39: "right", 40: "down", 45: "insert", 46: "del", 
      96: "0", 97: "1", 98: "2", 99: "3", 100: "4", 101: "5", 102: "6", 103: "7",
      104: "8", 105: "9", 106: "*", 107: "+", 109: "-", 110: ".", 111 : "/", 
      112: "f1", 113: "f2", 114: "f3", 115: "f4", 116: "f5", 117: "f6", 118: "f7", 119: "f8", 
      120: "f9", 121: "f10", 122: "f11", 123: "f12", 144: "numlock", 145: "scroll", 191: "/", 224: "meta"
    },
  
    shiftNums: {
      "`": "~", "1": "!", "2": "@", "3": "#", "4": "$", "5": "%", "6": "^", "7": "&", 
      "8": "*", "9": "(", "0": ")", "-": "_", "=": "+", ";": ": ", "'": "\"", ",": "<", 
      ".": ">",  "/": "?",  "\\": "|"
    }
  };

  function keyHandler( handleObj ) {
    // Only care when a possible input has been specified
    if ( typeof handleObj.data !== "string" ) {
      return;
    }
    
    var origHandler = handleObj.handler,
      keys = handleObj.data.toLowerCase().split(" ");
  
    handleObj.handler = function( event ) {
      // Don't fire in text-accepting inputs that we didn't directly bind to
      if ( this !== event.target && (/textarea|select/i.test( event.target.nodeName ) ||
         event.target.type === "text") ) {
        return;
      }
      
      // Keypress represents characters, not special keys
      var special = event.type !== "keypress" && jQuery.hotkeys.specialKeys[ event.which ],
        character = String.fromCharCode( event.which ).toLowerCase(),
        key, modif = "", possible = {};

      // check combinations (alt|ctrl|shift+anything)
      if ( event.altKey && special !== "alt" ) {
        modif += "alt+";
      }

      if ( event.ctrlKey && special !== "ctrl" ) {
        modif += "ctrl+";
      }
      
      // TODO: Need to make sure this works consistently across platforms
      if ( event.metaKey && !event.ctrlKey && special !== "meta" ) {
        modif += "meta+";
      }

      if ( event.shiftKey && special !== "shift" ) {
        modif += "shift+";
      }

      if ( special ) {
        possible[ modif + special ] = true;

      } else {
        possible[ modif + character ] = true;
        possible[ modif + jQuery.hotkeys.shiftNums[ character ] ] = true;

        // "$" can be triggered as "Shift+4" or "Shift+$" or just "$"
        if ( modif === "shift+" ) {
          possible[ jQuery.hotkeys.shiftNums[ character ] ] = true;
        }
      }

      for ( var i = 0, l = keys.length; i < l; i++ ) {
        if ( possible[ keys[i] ] ) {
          return origHandler.apply( this, arguments );
        }
      }
    };
  }

  jQuery.each([ "keydown", "keyup", "keypress" ], function() {
    jQuery.event.special[ this ] = { add: keyHandler };
  });

})( jQuery );


// end jquery plugins

// jquery date(time)-picker helpers

// thank you rick stahl: http://www.west-wind.com/weblog/posts/891992.aspx
$.maxZIndex = $.fn.maxZIndex = function(opt) {
    var def = { inc: 10, group: "*" };
    $.extend(def, opt);
    var zmax = 0;
    $(def.group).each(function() {
        var cur = parseInt($(this).css('z-index'));
        zmax = cur > zmax ? cur : zmax;
    });
    if (!this.jquery)
        return zmax;

    return this.each(function() {
        zmax += def.inc;
        $(this).css("z-index", zmax);
    });
}

function update_timefield(fld_name) {
  var $date = $('#' + fld_name);
  var date = strip_time($date.val()); 
  var time = $('#'+ fld_name + '_time').val(); 
  //alert("updating to:" + date + " + " + time);  
  if(date && time) { 
    var datetime = date+ " " + time + ":00";
    $date.val(datetime);
  }
}

function is_date(datetime) {
  var date = strip_time(datetime);
  return moment(date, 'YYYY-MM-DD', true).isValid();  
}

// yyyy-mm-dd hh:mm:ss => yyyy-mm-dd
function strip_time(datetime) {
  return datetime.substring(0, 10);
}

// yyyy-mm-dd hh:mm:ss => hh:mm:ss
function strip_date(datetime) {
  return datetime.substring(11);
}

// yyyy-mm-dd hh:mm:ss => hh:mm
function strip_date_sec(datetime) {
  var time = strip_date(datetime);
  return time.substring(0, 5);
}

// checks if javscript Date object date1 is after date2
function is_after(date1, date2) {
  return time_between(date2sql(date1), date2sql(date2)) < 0;
}

// end datetime.js

// start jquery/init.js
if(1) {
  $(document).ready(function() {
    $('input').on('change', function() { formChangedFlag = 'X'; });   
    $('a[rel=external]').click(function() {
      window.open($(this).prop('href'), 'Edit Dialog', "height=900,width=550,modal=yes,alwaysRaised=yes,toolbar=0,location=0,menubar=0");
    });
  });
  
  
  var $site_data = $("#_site_data");
  var $body = $("BODY");
  if($site_data.length && $body.length) {
    var site_data = $site_data.data('data');
    //console.log("Found data and body", site_data);
    var user_level = site_data.user_level;
    if(user_level >= 30) { // admin
      var edit_button = '<div id="site_edit_button" style="display:none;position:fixed;top:10px;left:10px;z-index:100000"><a href="?_edit_site=1" role="button" class="btn btn-success"><i class="fa fa-pencil-o"></i> Edit Site</a></div>';
      //$body.addClass("relative");
      $body.prepend(edit_button);

    }
  }
}

// end jquery/init.js


function reload_page() {
  $("body").addClass('disabled');
  $("body").addClass('spinning_wheel');
  window.location.reload();
}

// begin searchbar.js

// removed from now - didn't validate
// end searchbar.js


// start request.js
function update_pickup(checkin) {
  var $checkbox = $('#airport_pickup');
  var too_late = false;

  if(checkin)  {
    var now = new Date();
    var checkinDate = new Date(sql2date(checkin));
    var deadline = add_days(checkinDate, -1);
    too_late =  now > deadline;
    
  }
  
  if(!too_late && $('#arrival_method').val()=='air' && $('#arrival_time_disp').val() && $('#arrival_number').val()) {
    $checkbox.prop('disabled', false);
  } else {
    $checkbox.prop('disabled', true);
  }
}

function update_pickup_fee() {
  var checked = $('#airport_pickup').is(':checked');
  var $fee = $('#fee_pickup');  
  $fee.prop('disabled', (checked ? false : true));
  update_overlay_fees(); // only works in calendar...
}

// called when user fills in custom amount to pay
function btc_update_amount() {
  var custom_amount = $("#btc_custom_amount").val();
  var cur_per_btc = $("#cur_per_btc").val();
  var amount_btc = custom_amount/cur_per_btc;
  $("#custom_amount_btc").val(amount_btc);
  $("#custom_amount_btc_text").text(amount_btc);
  //alert(amount_btc);  
  
}

// end request.js

// start property.js

function feedback_update(scores) {
  for(var key in scores) {
    var score = scores[key];
    if(score > 0) {
      $('#feedback_' + key).css('width', scores[key] * 11 - 1);
      $('#feedback_' + key + '_score').html(scores[key]);
      $('#feedback_label_' + key).removeClass('disabled');
    } else { // not rated
      $('#feedback_' + key).css('width', 0);
      $('#feedback_' + key + '_score').html('');
      $('#feedback_label_' + key).addClass('disabled');
    }
  }
  $('#feedback_total').html(scores['overall']);
}

function feedback_load(profile, apt_id, language, options) {
  var ajax_url = '/home/ajax/feedback.php?profile=' + profile + '&apt_id=' + apt_id + '&lang=' + language + '&options=' + array2json(options);
  //alert(ajax_url);
  $('#feedback_ind').load(ajax_url);
}  

// end property.js
  

// wrapper for jquery parseJson. Removes any (unwanted) debug text that may preceed or follow json
function parse_json(json) {
  var len = json.length;
  var first = json.charAt(0);
  var last = json.charAt(len-1);
  if(first == '{' && last == '}') return jQuery.parseJSON(json);
  if(first == '[' && last == ']') return jQuery.parseJSON(json);

  return 0;
  
  // get rid of garbage (only works for {} )
  // todo: test if start and end were found, implement for arrays []
  var start = json.indexOf('{');
  var end = json.lastIndexOf('}');
  len = end - start;
  var substr = json.substr(start, len+1);
  //alert("sub of json from " + start + " to " + end + " = " + substr);
  return jQuery.parseJSON(substr);  
}

/**        
 * Converts the given data structure to a JSON string.
 * Argument: arr - The data structure that must be converted to JSON
 * Example: var json_string = array2json(['e', {pluribus: 'unum'}]);
 *      var json = array2json({"success":"Sweet","failure":false,"empty_array":[],"numbers":[1,2,3],"info":{"name":"Binny","site":"http:\/\/www.openjs.com\/"}});
 * http://www.openjs.com/scripts/data/json_encode.php
 */
function array2json(arr) {
  return JSON.stringify(arr);
  
  /** old code below */
  
    var parts = [];
    var is_list = (Object.prototype.toString.apply(arr) === '[object Array]');

    for(var key in arr) {
      var value = arr[key];
        if(typeof value == "object") { //Custom handling for arrays
            //alert(dump(value) + " is object"); 
            if(is_list) parts.push(array2json(value)); /* :RECURSION: */
            else parts[key] = array2json(value); /* :RECURSION: */
        } else {
            var str = "";
            if(!is_list) str = '"' + key + '":';

            //Custom handling for multiple data types
            if(typeof value == "number") str += value; //Numbers
            else if(value === false) str += 'false'; //The booleans
            else if(value === true) str += 'true';
            else str += '"' + value + '"'; //All other things
            // :TODO: Is there any more datatype we should be in the lookout for? (Functions?)

            parts.push(str);
        }
    }
    var json = parts.join(",");
    
    if(is_list) {
      //alert('[' + json + ']');//Return numerical JSON
      return '[' + json + ']';//Return numerical JSON
    }
    return '{' + json + '}';//Return associative JSON
}

function update_gallery_trash(params, data) {
  var $count = $('#gallery_trash SPAN.count');
  var old_count = parseInt($count.text());
  $trash = $('#gallery_trash');
  if($trash.length) {
    var ids = data.id;
    var count = ids.split(',').length;
    var new_count = old_count + count;    
    //alert('old=' + old_count  + "count = " + count + " new=" + new_count);
    $count.text(new_count);  
    $trash.removeClass('invisible').show('fast');
  }
}

/*
 * debug
 * Simply loops thru each jquery item and logs it
 */
jQuery.fn.debug = function() {
  return this.each(function(){
    $.log(this);
  });
};

/*
 * log
 * Send it anything, and it will add a line to the logging console.
 * If firebug is installed, it simple send the item to firebug.
 * If not, it creates a string representation of the html element (if message is an object), or just uses the supplied value (if not an object).
 */
jQuery.log = function(message){
  // only if debugging is on
  if( window.DEBUG ){
    // if no firebug, build a debug line from the actual html element if it's an object, or just send the string
    var str = message;
    if( !('firebug' in console) ){
      if( typeof(message) == 'object' ){
        str = '&lt;';
        str += message.nodeName.toLowerCase();
        for( var i = 0; i < message.attributes.length; i++ ){
          str += ' ' + message.attributes[i].nodeName.toLowerCase() + '="' + message.attributes[i].nodeValue + '"';
        }
        str += '&gt;';
      }
    }
    console.console.log(str);
  }
};

// end logging

/*
 * Image preview script 
 * powered by jQuery (http://www.jquery.com)
 * 
 * written by Alen Grakalic (http://cssglobe.com)
 * 
 * for more info visit http://cssglobe.com/post/1695/easiest-tooltip-and-image-preview-using-jquery
 *
 */
 
this.imagePreview = function(){ 
  /* CONFIG */
    
    yOffset = 500;
    xOffset = 30;
    
    // these 2 variable determine popup's distance from the cursor
    // you might want to adjust to get the right result
    
  /* END CONFIG */
  $("a.preview, img.image_preview").hover(function(e){
    this.t = this.title;
    this.title = "";  
    //alert(this.href);
    var c = (this.t != "") ? "<br/>" + this.t : "";
    $("body").append("<p id='preview' style='z-index: 1000'><img src='"+ (this.href ? this.href : this.src) +"' alt='Image preview' />"+ c +"</p>");                 
    var height = parseInt($('#preview').css('height'));
    // if(height > 700) yOffset += 300;
    // $("#preview").append("<p> h ="+ height +"</p>");                
    $("#preview")                                                         
      .css("top",(e.pageY - yOffset) + "px")
      .css("left",(e.pageX + xOffset) + "px")
      .fadeIn("fast");            
    },
  function(){
    this.title = this.t;  
    $("#preview").remove();
    }); 
  // $("a.preview").mousemove(function(e){
  //  $("#preview")
  //    .css("bottom",(e.pageY - yOffset) + "px")
  //    .css("left",(e.pageX + xOffset) + "px");
  // });      
};

function neosmart_wall(user, limit, offset) {  
  //var ajax_url = '/home/ajax/facebook.php?user=' + user + '&limit=' + limit + '&offset=' + offset + '&action=neosmart_wall';
  //alert(ajax_url);
  var token = '299906836726434|g0UGaDPzf4otbNo_8OtMlMSP7Zk';
  var like_this = 'vinden dit leuk';
  var likes_this = 'vindt dit leuk';
  var people = 'mensen';
  $("#neosmart_wall").fbWall({id: user, 
                      accessToken: token, 
                      max: limit,
                      offset: offset,
                      translateLikeThis: like_this,
                      translateLikesThis: likes_this,
                      translatePeople: people,
                      showGuestEntries: true,
                      showComments: true,
                      timeConversion:24
                      });                     
  // $(content).appendTo($("#neosmart_wall"));
}


function facebook_wall(user, limit, offset) {  
  var ajax_url = '/home/ajax/facebook.php?user=' + user + '&limit=' + limit + '&offset=' + offset;
  //alert(ajax_url);
  $("#facebook_wall").load(ajax_url);    
}


function facebook_wall_js(div_id, user, token, limit, offset) {
  $('#' + div_id).facebookWall({
    id: user,
    access_token: token,
    limit: limit,
    offset: offset
  });
}

// removes a paramter from query string
function removeURLParameter(url, parameter) {
  var urlparts= url.split('?');   
  if (urlparts.length>=2) {
    var prefix= encodeURIComponent(parameter); //+'=';
    var pars= urlparts[1].split(/[&;]/g);
    for (var i= pars.length; i-- > 0;) { //reverse iteration as may be destructive  
      if (pars[i].lastIndexOf(prefix, 0) !== -1) {  
        pars.splice(i, 1);
      }
    }
    url= urlparts[0]+'?'+pars.join('&');
    return url;
  } else {
    return url;
  }
}

// wrapper for jQuery.param, which uses + instead of %20 for spaces
function obj2qs(data) {
  var params = jQuery.param(data);
  return params.replace(/\+/g, '%20');
}

function iphoneSwitchOnOff($target, value) {
  var handler = $target.data('handler');
  var ajax_url = '/ajax.php?oper=' + handler;
  var id = $target.prop('id');
  var target_field = $target.data('target');
  var data = $target.data('data') || {};
  var obj_type = $target.data('obj_type');
  if(target_field) ajax_url = ajax_url + '&' + target_field + '=' + value;
  if(data) {
    ajax_url = ajax_url + '&' + obj2qs(data);
    //console.log("on-off handler: url=" + ajax_url);
    $('#' + id + '-output').load(ajax_url); // switch on
  } else {
    //console.log("on-off handler: no data");
  }
}

// loops through all checkboxes in target and returns comma-separated list of the last part of the corresponding IDS (after the last underscore) 
function get_checked_ids(target_selector) {
  var $target = $(target_selector);
  if(!$target.length) return '';
  var $checked = $target.find(':checkbox:checked');
  var vals = [];
  $.each($checked, function() {
    var $checkbox = $(this);
    var id = $checkbox.prop('id').split('_').pop();
    vals.push(id);
  });
  var ids = vals.join(",");
  return ids;
}

// this must be bound before the fancybox link, or data will not work
$(document).on("click", ".price-breakdown-link", function() {
   $link = $(this);
   var url = $link.data('url');
   var data = $link.data('data');
   //console.log("price breakdown", data);
   var href = url + '&' + obj2qs(data);
   //console.log("price breakdown href=" + href);
   if(url && data) $link.attr('href', href);
   //$.each(data, function(key, value) {
   //  //console.log(key + '=' + value);
   //});
   return false;
});


/* generic event handlers */
$(document).ready(function() {
  ready_script();
});
/* end generic event handlers */

$(document).on("click", ".jfu-switch-view", function (e) {
  var template = $(this).data("template");
  var $uploader = $(this).closest(".jquery-file-uploader-container").find(".jquery-fileupload");
  var view = $uploader.data("view");
  if(view == template) return false; // already shown
  
  //console.log("Switch to " + template + " of uploader with len=" + $uploader.length);
  if(template && $uploader.length) {
    $uploader.fileupload({downloadTemplateId: "template-download-" + template});
    jfu_load($uploader); // triggers redraw (better way?)
    $uploader.data("view", template);
    $(".jfu-switch-view").removeClass("btn-positive").addClass("btn-default");
    $(this).removeClass("btn-default").addClass("btn-positive");
  }
  return false;
});

$(document).on("click", ".no-click", function (e) {
  //console.log('non clickable link');
  return false;
});

// for inputs with class "input", allow enter to submit (if there is exactly one element in form with class "submit")
// causes double submit

$(document).on("keypress", "form.enter-to-submit input:text, form.enter-to-submit input:password", function(e) {
  var code = (e.keyCode ? e.keyCode : e.which);
  console.log("code=" + code);
  if(code == 13) {
    $(this).blur();
    var $form = $(this).closest('form');
    console.log("code=" + code + " submitting form id=" + $form.attr('id'));
    if($form.length) $form.submit();
  }
});

$(document).on("click", ".scroll-top", function(e) {
  if(!in_iframe()) $('html, body').animate({scrollTop:0}, 'slow');
});

$(document).on("click", ".payment-button", function(e) {
  var $target = $(this);
  var $form = $('#payment_form');
  var currency = $target.data('currency');
  var amount = $target.data('amount');
  var payment_type = $target.data('payment_type');
  $('#amount').val(amount);
  $('#payment_type').val(payment_type);
  //console.log("Paying " + currency + " " + amount + " using " + payment_type);   
});

// new handler for saving form
$(document).on("click", ".save-form", function(e) {
  var $form = $(this).closest('form');
  if(!$form.length) {
    //console.log("save-form: could not find form");
    return true; // delegate to form action
  }
  var form_id = $form.attr('id');
  if(!form_id) {
    //console.log("error: save-form: could not find form ID (required)");
    return false; // delegate to form action
  }
  var obj_type = $form.data('obj_type');
  var obj_id = $form.data('obj_id') || $form.find('input[name="id"]').val() || 0;
  //console.log("Save form " + form_id + " for obj_type=" + obj_type + " and id=" + obj_id);
  var $messages = form_messages(form_id); // get message div      
  if(obj_type && obj_id) {
    if($messages.length) $messages.html(bootstrap_success_message('<p class="spinning_wheel">Saving....</p>')); 
    //console.log("Autosaving form " + form_id + " for " + obj_type + " " + obj_id);
    save_object(form_id, obj_type, 'edit'); // {'handler': '$handler', 'handler_operation': '$handler_operation', 'target': '$target', 'reload': '$reload', 'parent_id': '$parent_id'});\"";
  }
  return false;
});

// new handler for cancelling form
// if element is contained in a dialog, close this
// if not, redirect to URL minus 'edit' in query string (e.g. /profile/?foo=bar&edit => /profile/?foo=bar)
$(document).on("click", ".cancel-form", function(e) {
  //console.log("cancel form");
  if(close_parent($(this))) { // it was not a dialog
    var url = window.location.href;
    //console.log("Form was not in dialog url=" + url);
    url = removeURLParameter(url, 'edit');
    //console.log("now=" + url);
    window.location.href = url;
    return false;
  }
  return false;
});

function reset_form(form_id, except_hidden) {
  
  $("#" + form_id + ' input[type=text]').val('');  
  $("#" + form_id + ' input[type=email]').val('');  
  $("#" + form_id + ' input[type=number]').val('');  
  $("#" + form_id + ' input[type=phone]').val('');  
  $("#" + form_id + ' textarea').val(''); 
  $("#" + form_id + ' input[type=select]').val('').val(0);
  $("#" + form_id + ' input[type=radio]').val('');
  
  /** none of these work, thus the div.note-editable ! */
  $("#" + form_id + ' textarea.summernote').summernote('code', '');     
  //$("#" + form_id + ' textarea.summernote').summernote('reset');  
  $('DIV.note-editable').html("");
  
  if(!except_hidden) {
    //console.log("reset hidden too len=" + $("#" + form_id + ' input[type=hidden]:not([readonly]').length);    
    //console.log("hidden forms including readonly=" + $("#" + form_id + ' input[type=hidden]').length);    
    $("#" + form_id + ' input[type=hidden]:not([readonly]').val('');
    //:input:not([readonly])
  }
}


function reset_parent_form($target, submit) {
  var $form = $target.closest("FORM");
  if(!$form.length) {
    //console.log("reset_parent_form: no form found target=", $target); 
    return true;
  }
    
  var form_id = $form.attr('id');
  if(!form_id) {
    //console.log("reset_parent_form: form has no id target=", $target);     
    return true;
  }
  
  //console.log("reset_parent_form: OK, resetting " + form_id); 
  reset_form(form_id, true);
  if(submit) {
    //console.log("reset_parent_form: submitting " + form_id); 
    $form.submit();
  }
  return false; // prevent submit
}

$(document).on("click", ".reset-form", function(e) {
  var $target = $(this);
  return reset_parent_form($target);
});

$(document).on("click", ".reset-form-submit", function(e) {
  var $target = $(this);
  return reset_parent_form($target, true);
});

// show div in fancybox
$(document).on("click", ".show-fancybox-div", function(e) {
  var $link = $(this);
  var target = $link.data('target');
  var $target = target ? $(target) : 0;
  if(target && $target.length) {
    var height = $link.data('height') || 640;
    var width = $link.data('width') || 480;
    $.fancybox(
      $target.html(), 
      {
        'width'       : height,
        'height'      : width,
        'autoScale'     : false,
        'transitionIn'    : 'none',
        'transitionOut'   : 'none',
        'hideOnContentClick': false,
        'onStart': function () {
          //On Start callback if needed
        }
      }
    );
  }
  return false;
});

// checkbox changed, set target to 0 or 1
$(document).on("change",'INPUT.cb-update-target',function() {
  var $cb = $(this);
  var checked = $cb.prop("checked");
  var target = $cb.data('target');
  var $target;
  $target = target ? $(target) : null;
  
  console.log("cb-update-target checked=" + checked + " target=" + target + " len=" + $target.lenth);
  if($target.length) {
    $target.val(checked ? 1 : 0);
    $target.trigger("change");
  }
  
}); 


// for checkboxes in group, update parent field
$(document).on("change",'.checkbox_group input:checkbox',function(){
  var $cb = $(this);
  var cb_id = $cb.data("id") || $cb.attr('id');
  var $parent = $cb.closest('.checkbox_group');
  if($parent) {
    if($target = $parent.find(".cb_target")) {
      if(flags = $parent.data('flags')) {
        checkboxes2flags($parent, $target);
      } else {
        checkboxes2string($parent, $target);
      }
      //console.log(this.name + " changed plen="+ $parent.length + " target len=" + $target.length + " flags=" + flags) // use the ajax here
    } else {
      //console.log(this.name + " changed plen="+ $parent.length + " but could not find target") // use the ajax here
    }
  } else {
    //console.log(this.name + "id=" + cb_id + " changed but could not find checkbox group");
  }
});

   
// if delete content from date picker, also clear alt field
$(document).on("change", "form .hasDatepicker, form .date_input", function(e) {
  var id = $(this).prop('id');
  var disp_id = id.replace('_disp', '');
  var $target = $('#' + disp_id);
  var val = $.trim($(this).val());
  if($target.length && !val) $target.val(''); // clear alt field
});

// Submit parent form if button has class "submit")
$(document).on("click", "form .submit", function() {
  var $form = $(this).closest('form');
  if($form.length) {
    $form.submit();
  }
});

// This link is used for Bitcoin payments to check if user paid
/*
$(document).on("click", "A.process-payment", function() {    
  var data = $(this).data('data') || {};
  var url = $(this).attr('href');
  var $div = $('#process-payment-result');  
  if(url && data) {
    log("Process payment url=" + url);
    log(data);
    var ajax_url = '/ajax.php?oper=process-payment'  + '&' + obj2qs(data);
    $div.load(ajax_url, function(responseText) {
       log(responseText);
    });      
    //$form.submit();
  } else {
    log("Process payment missing url or data");
  }
  
  return false;
});
*/

$(document).on("click", "A.goto-tab", function() {
  var $tabs = $(this).closest('.ui-tabs');
  var active = $(this).data('index') || 0;
  if($tabs.length) {
    $tabs.tabs( "option", "active", active);
    return false;
  } 
  var $accordion = $(this).closest('.ui-accordion');  
  if($accordion.length) {
    $accordion.tabs( "option", "active", active);
    return false;
  }
});

$(document).on("click", ".tooltip-ajax", function() {
  $(this).tooltip({
    content:function(callback) { //callback
      var url = $(this).data("url");
      $.get(url,{}, function(data) {
        callback(data); //call the callback function to return the value
      });
    },
  });
});

$(document).on("click", ".image-rotation-control", function() {
  $cont = $(this).prevAll('.image-rotation');
  var images = $cont.data('images') || [];
  var replace = $cont.data('image-replace');
  var replace_find, replace_with, filename;
  if(replace) {
    var replaceAr = replace.split('|');
    if(replaceAr.length == 2) {
      replace_find = replaceAr[0];
      replace_with = replaceAr[1];
    }
  }
  
  var first = images[0];
  if(typeof first == 'object') { // convert from array of objects (media_json) to plain array
    var ar = images;
    var img = {};
    images= [];
    for(var i=0;i < ar.length;i++) {
      img = ar[i];
      filename = img.filename;
      if(replace_find && replace_with) {
        filename = filename.replace(replace_find, replace_with);
      }
      images.push(filename);
    }
  }
  
  var current = $cont.data('current');
  var count = images.length;  
  var action = $(this).hasClass('image-rotation-prev') ? 'previous' : 'next';
  if(count <= 1) return false;
  if(action == 'previous') image_slider_previous($cont, images);
  else image_slider_next($cont, images);
   
});  

$(document).on("contextmenu", ".copyright", function() {
  //alert("IMG click Right clicks are not permitted due to copyright settings");
  // more functionality code goes here
  return false;
});


// finds a set of checkboxes in parent
// writes separated list of values if checked to result
function checkboxes2string($parent, $result) {
  var checkArray = new Array();
  var checkboxes_on = $parent.find("input:checked")
  for (var i = 0; i < checkboxes_on.length; i++) {
    var checkbox = checkboxes_on[i];
    var cb_val = checkbox.value;
    if(cb_val != '') checkArray.push(cb_val);
  }
  checkArray.sort();
  $result.val(checkArray.join(','));
}

// finds a set of checkboxes in parent
// writes logigally or'ed value to result
function checkboxes2flags($parent, $result) {
  var checkboxes_on = $parent.find("input:checked")
  var result = 0;
  for (var i = 0; i < checkboxes_on.length; i++) {
    var checkbox = checkboxes_on[i];
    var cb_val = checkbox.value;
    if(cb_val != '') result |= cb_val;
  }
  $result.val(result);
}
                   
// takes a selector with data-images, replaces source of child IMG with given index
function image_slider_set($selector, images, index) {
  var $img = $selector.find('img');
  var src = images[index];
  $img.attr('src', src);
  $selector.data('current', index);  
}

function image_slider_next($selector, images) {
  var current = $selector.data('current');
  var length = images.length;
  var next = current >= images.length - 1 ? 0 : current + 1;
  //console.log("next: current=" + current + " length=" + length + " next=" + next);
  image_slider_set($selector, images, next);
}

function image_slider_previous($selector, images) {
  var current = $selector.data('current');
  var length = images.length;
  var previous = current <= 0 ? images.length - 1 : current - 1;
  //console.log("previous: current=" + current + " length=" + length + " previous=" + previous);
  image_slider_set($selector, images, previous);
}


// sets value of alt field on autocomplete select
function autocomplete_alt_select(event, ui) {
  var $sel = $(this);
  var target = $sel.data('target'); 
  var $target = target ? $('#' + target) : null;  

  if(!ui.item) return false;

  //console.log(ui.item ? ("Autocomplete_alt_select Selected: " + ui.item.value + " aka " + ui.item.id + " target=" + target) : "Nothing selected, input was " + this.value);
  var item = ui.item;
  if(item.id && target && $target.length) {
    //console.log("Setting value of target to " + item.id);
    $target.val(item.id);
  }
}  

function show_autocomplete_menu($ul) {
  if(!$ul.length) {
    console.log("show_autocomplete_menu: missing UL");
    return;
  }
  if($ul.css('display') == 'none') {
    var parent_id = $('#bounds').data('target_display');
    var parent_width = $("#" + parent_id).css('width');
    var $parent = $("#" + parent_id);
    //console.log("show_autocomplete_menu: UL len=" + $ul.length + " pid=" + parent_id + " pl=" + $parent.length);
    
    if(!$parent.length) {
      console.log("show_autocomplete_menu: could not find parent " + parent_id);
      return;
    }
    //console.log('pid=' + parent_id + ' pw=' + parent_width);
    $ul.html('');
    $ul.css('display', 'block'); 
    $ul.css('width', parent_width);
    $ul.position({
      my: "top center",
      at: "center bottom",
      of: "#" + parent_id
    });
  }
}

function datetimepicker_set($input, v) {
  var name = $input.attr("name");
  var dp = $input.data("datepicker");
  var $dp = $(dp);
  //console.log("datetimepicker_set: Setting value of " + name + " to " + v + " dp=" + dp + " len=" + $dp.length);  
  if($dp.length) {
    //var d = new Date(v);
    $dp.val(v);
    $dp.datetimepicker('update');
  }
}

$(document).on("click", ".reset-form", function() {
  var $target = $(this);
  var form = $target.data('form');
  var $form = $(form);
  if($form.length) {
    //console.log("Reseting form");
    $form.trigger("reset");
  }
});

// populate a form using a data object
// Thanks: http://stackoverflow.com/questions/7298364/using-jquery-and-json-to-populate-forms
function populate(form_id, data) {   
  form_id = ltrim(form_id, '#'); // allow id or selector
  $.each(data, function(key, value){  
    var $ctrl = $('#' + form_id + ' [name='+key+']');
    //console.log("setting " + key + "=" + value);
    
    switch($ctrl.attr("type"))  
    {  
        case "text":   
        case "hidden":  
        $ctrl.val(value);   
        break;   
        case "radio": 
        case "checkbox":   
        $ctrl.each(function(){
           if($(this).attr('value') == value) {  $(this).attr("checked",value); } });   
        break;  
        default:
        $ctrl.val(value);
        break;        
    }      
  });  
}

// copy data from data-data attribute of parent to target form
$(document).on("click", ".fill-form", function() {
  var $target = $(this);
  var data = $target.data('data');
  var form = $target.data('form');
  var $form = $(form);
  if(!$form.length) {
    //console.log("fill-form: did not find " + form);
    return false;    
  }
  var form_id = $form.attr('id');
  console.log(".fill-form " + form_id + " data len=" + Object.size(data));

  fill_form(form_id, data);   
  ready_script(); // trigger handlers on special fields
});

function fill_form(form_id, data) {
  console.log("fill form " + form_id + " data len=" + Object.size(data));
  if(!form_id) return false;
  if(!Object.size(data)) return false;  
  var form = '#' + form_id;
  var $form = $(form);
  if(!$form.length) return false;
  
  $.each(data, function(k, v) {
    //console.log("k=" + k + " v=" + v);
    var form_elements = ["INPUT", "SELECT", "TEXTAREA"];
    $.each(form_elements, function(i, type) {
      var $input = $(form + " " + type + "[name='"+ k +"']");
      if(type=='INPUT' && k == 'id') { // ID - special case: we sometimes use obj_id for the name
        var $id = $(form + " " + type + "[name='obj_id']");
        if($id.length) $id.val(v);
      }
      
      if($input.length) { // standard input
        type = type.toLowerCase();
        var input_type = type == 'input' ? $input.attr('type') : '';
        //console.log("Found " + type + " for " + k + " type=" + type + ' input_type=' + input_type + " value=" + v);        
        if(type == 'textarea') {
          //console.log("textarea class=" + $input.attr('class'));
          if($input.hasClass('summernote')) {
            $input.html(v);
            //var $panel = $(form + " " + type + "[name='"+ k +"'] .note-editable");
            var id = $input.attr('id');
            //var $panel = $("#" + id + " DIV.panel-body"); /** to: find() not working. this matches all on page */
            var $panel = $(".note-editable"); /** to: find() not working. this matches all on page */
            $panel.html(v);
            //console.log("Input=", $input);
            //$input.summernote();   
            //console.log("Setting summernote id=" + id + " editable len= " + $panel.length + "  value to " + v);
          } else {
            $input.html(v);
            //console.log("Setting text area html to " + v);
          }
        } else if(input_type == 'checkbox') {
          var checked = v > 0 ? true : false;
          //console.log(k + " is checkbox v=" + v + ' checked=' + checked);
          $input.prop('checked', checked);
        } else {
          $input.val(v);
        }
        if(edit_form_handler = $input.data("edit-form-handler")) { // custom handler (e.g. for datepicker)
          //console.log("Found custom handler " + edit_form_handler + " for " + k);
          window[edit_form_handler]($input, v); // eval = evil
        }
        return false; // break                
      } else {
        //console.log("Did not find " + type + " for " + k);
      }
    });   
  });
}

// return a random letter + timestamp to use as a "random" element ID
function random_id() {
  var randLetter = String.fromCharCode(65 + Math.floor(Math.random() * 26));
  return randLetter + Date.now();
}

/** used to load URL from back-end. Show spinner. Used by invoice export */
$(document).on("click", ".ajax-loader", function(e) {
  console.log('ajax-loader');
  var $target = $(this);
  var url = $target.data('url') || $target.attr('href') || '';
  if($target.data('breakout')) {
    url = replace_query_var(url, '_pv', 0)
    console.log("breakout - use hard redirect to url=" + url);
    window.location.href = url; // don't use soft load
    return;
  }
  if(g_form_dirty) {
    e.preventDefault();
    console.log("ajax-loader: form dirty a href=" + url);
    if(!confirm_modal($target)) return;
  }
  
  var target = $target.data('target');
  var org_text = $target.html();
  var load_text = $target.data('load_text') || org_text;
  var $target_div = $(target);
  var spinner = '<i class="fa fa-spinner fa-spin"></i> ';
  console.log('url=' + url + ' target=' + target + " len=" + $target_div.length);
  if(url && target && $target_div.length) {
    $target.removeClass("hidden").html(spinner + load_text).show("fast");
    $target_div.load(url, function() {
      $target.html(org_text); 
      ready_script();      
    });
  }
  return false;
});

/** disable empty fields to clean URL for GET requests */
$(document).on("submit", ".clean-submit", function() {
  var $form = $(this);
  console.log("Cleaning form method=" + $form.attr('method').toLowerCase());
  if($form.attr('method').toLowerCase() == "get") {
    form_disable_blank($form);
  }
  return true;  
});

function form_disable_blank($form) {
  $form.find('select,input,textarea').filter(function() {
     var val = $(this).val();
     var set =  val.length && val != '0';
     console.log("name=" + $(this).attr('name') + " val=" + val + " set2=" + set);
     return !set;
  }).prop('disabled', true);  // attr may work as well.
}

function array_filter(myObj) {
  for (var key in myObj) {
    if(!myObj[key]) delete myObj[key];
  }
  return myObj;
}

//alert('yah');
/** why do the following three not work ? */
$(document).on(".location-name", "keydown", function() {
  console.log( "Loc .keydown()." );
});

$(document).on(".location-name", "change", function() {
  console.log( "Loc .change()." );
});

$(document).on(".location-name", "focus", function() {
  alert("focus");
});

function searchbar_submit($form) {
  var location_url = $("#location_url").html();
  var shortstay_url = $("#shortstay_url").html();
  var geo_url = $("#geo_url").html();
  var slug = $("#location_slug").val();
  
  console.log("searchbar_submit slug=" + slug + " url=" + location_url);
  if(slug && location_url) {
    $form.attr('action', location_url + slug);
    $(".location-bounds").val('');
    $(".location-center").val('');
    $(".location-name").val('');
    //$(".location-name").css('background-color', 'red').val('');
    //return false;
  } else if(shortstay_url) {
    $form.attr('action', shortstay_url);
  }
  return true;
}

$(document).on("submit", ".searchbar-form", function() {
  console.log("searchbar-form submit");
  var $form = $(this);
  searchbar_submit($form);
  form_disable_blank($form);
  return true;
});  

$(document).on("submit", ".ajax-submit", function() {
  //alert("okay2");
  console.log("submitting ajax-submit form...");
  var $form = $(this); 
  return ajax_handler($form);  
});

/** similiar to ajax-loader, but supports all the options of ajax-submit for forms */

$(document).on("click", ".ajax-link", function() {
  var $target = $(this);

  var load_text = $target.data('load_text'); // change link while loading
  if(load_text) {
    var org_text = $target.html();
    $target.data('org_text', org_text);
    var spinner = '<i class="fa fa-spinner fa-spin"></i> ';
    $target.html(spinner + load_text);
  }

  return ajax_handler($target);
});

/** bootstrap table edit link  */
/** todo: finish, generalize  */
$(document).on("click", ".bst-edit-link", function(e) {
  var $target = $(this);
  var modal = $target.data('target');
  var $modal = $(modal);
  var $table = $modal.find(".table"); 
  var ajax_url = $target.data('url');
  var obj_type = $target.data('obj_type') || 'foo';
  var edit_url = $target.data('edit_url') || '/ajax.php?oper=edit&obj_type=' + obj_type;
  if(!g_columns) {
    alert("Missing column model");
    return;
  }
  
  $table.bootstrapTable({
    url: ajax_url,
    responseHandler: bstResponseHandler,
    colums: g_columns
  })
  .on('editable-save.bs.table', function(e, params, obj) {  
    return {"success": false, "msg": "server error"};
    return "that's an error";
  })
  .on('editable-init.bs.table', function(e) {      
    $editable = $('.editable');
    $.each($editable, function(index) {
      $(this).attr('data-pk', "id");
      $(this).attr('data-url', "/ajax.php?oper=foo");
    });
    
    $('.editable').data('url', "/ajax.php?oper=foo");
    
    $('.editable').on('save',function(e, params) {
      var responseJson = params.response;
      if(responseJson) {
        var response = $.parseJSON(responseJson);
        if(response.success) {
          var dataJson = response.data;
          var data = dataJson ? $.parseJSON(dataJson) : {};
          var subtotal = data.subtotal;
          if(subtotal) {
            //console.log("total=",subtotal);
          }
        }
      }
    });
  });
});
  
/** todo: finish and generalize */
  function bstResponseHandler(res) {
    return res.rows;
  }
  function operateFormatter(value, row, index) {
      return [
          '<a class="like" href="javascript:void(0)" title="Like">',
          '<i class="glyphicon glyphicon-heart"></i>',
          '</a>  ',
          '<a class="remove" href="javascript:void(0)" title="Remove">',
          '<i class="glyphicon glyphicon-remove"></i>',
          '</a>'
      ].join('');
  }
  window.operateEvents = {
      'click .like': function (e, value, row, index) {
          alert('You click like action, row: ' + JSON.stringify(row));
      },
      'click .remove': function (e, value, row, index) {
          $table.bootstrapTable('remove', {
              field: 'id',
              values: [row.id]
          });
      }
  };
  function totalTextFormatter(data) {
      return 'Total';
  }
  function totalNameFormatter(data) {
      return data.length;
  }
  function totalPriceFormatter(data) {
      var total = 0;
      $.each(data, function (i, row) {
          total += +(row.price.substring(1));
      });
      return '$' + total;
  }
  function getHeight() {
      return $(window).height() - $('h1').outerHeight(true);
  }

/** searchbar on rental detail page change */
$(document).on("change", "#sbd_in,  #sbd_out, #sbd_ng", function(e) {
  console.log("booking form changed...");
  booking_form_update();
});
  
/** todo: saving/saved after label */
$(document).on("change", ".autosave INPUT, .autosave TEXTAREA, .autosave SELECT", function(e) {
  var $form = $(this).closest('form');
  autosave($form);  
});

$(document).on("change", ".mail-form INPUT.auto-preview, .mail-form TEXTAREA.auto-preview, .mail-form SELECT.auto-preview", function(e) {
  var $form = $(this).closest('form');
  var form_id = $form.attr('id');
  mail_preview(form_id);
});

function escapeHtml(unsafe) {
  return unsafe
       .replace(/&/g, "&amp;")
       .replace(/</g, "&lt;")
       .replace(/>/g, "&gt;")
       .replace(/"/g, "&quot;")
       .replace(/'/g, "&#039;");
}
 
function mail_preview(form_id) {
  var $selected = $('#mail_template').find(":selected");
  if($selected.length) {
    $("#mail_obj_type").val($selected.data('obj_type'));
    $("#mail_obj_name").val($selected.data('obj_name'));
    $("#mail_controller").val($selected.data('controller'));
  }
  
  $("#mail_preview").attr("srcdoc", "Loading preview...");
  var ajax_url = "/ajax.php?oper=mail-preview";
  console.log("url=" + ajax_url);
  jQuery.ajax({
    type: "POST",
    data: form_data(form_id),
    url: ajax_url,
    success: function (responseText) {
      var result = parse_json(responseText);
      var error = result.error;
      var message = result.message;
      var success = result.success;
      console.log("success=" + result.success + " body=" + (result.body ? result.body.length : -1));

      if(success && result.body) {
        var headers = '';
        headers = headers + "<b>From:</b> " + escapeHtml(result.sender) + " <br>";        
        headers = headers + "<b>To:</b> " + escapeHtml(result.recipient) + "<br>";        
        if(result.replyto) headers = headers + "<b>Replyto:</b> " + escapeHtml(result.replyto) + "<br>";        
        headers = headers + "<b>Subject:</b> " + escapeHtml(result.subject) + "<br><br>";
        //if(result.attachment) headers = headers + "<b>Attachment:</b> " + escapeHtml(result.attachment) + "<br><br>";
        
        $("#mail_headers").html(headers).show();
        $("#mail_preview").attr("srcdoc",  result.body).show();
        if(result.redirect) window.location.href = result.redirect; // simple redirect, preserves query string params
        $("#mail_send").prop('disabled', false).show('fast');
      } else if(error) {
        $("#mail_headers").html(bootstrap_error_message(error || "Unknown error"));
        $("#mail_preview").hide();
        $("#mail_send").prop('disabled', true);
      } else {
      }
    }
  });
}   

// automatic is set to true for textareas that save w/o user asking to
function autosave($form, automatic) {
  var form_id = $form.attr('id');
  var obj_type = $form.data('obj_type') || $("#" + form_id + ' input[name=obj_type]').val();
  var obj_id = $form.data('obj_id') || $("#" + form_id + ' input[name=id]').val();
  var $messages = form_messages(form_id); // get message div
 
  var options = {"autosave": 1, "scroll": 0, "reload_link": ""};
  var show_message = $form.data('show_message');
  show_message = true;
  if(automatic) {
    options.success_handler = "";  // don't run on automatic autosave 
    options.show_message = false;  // don't show on automatic autosave
    show_message = false;
  }
   
  // $form.data('show_message', show_message); // whether or not to show messages/errors...  

  console.log("autosave form_id= " + form_id + " ot=" + obj_type + " id= " + obj_id + " msg len=" + $messages.length + " sm=" + show_message);  
  if(obj_type && obj_id) { /** not sure if this is needed ? */
    if(show_message && $messages.length) $messages.html(bootstrap_success_message('<i class="fa fa-spinner fa-spin"></i> Saving....')); 
    //$form.data('autosave', 1); // do not scroll up unless submit button is pushed. 
    ajax_handler($form, options);
  } else {
    $form.data('autosave', 1);
    $form.submit(); // this will invoke the ajax_handler below
  }
}

/** bootstrapTable: below used by data-formatter from template */


function ajax_handler($target, override) {
  var options = $target.data('options') || $target.data('handler_options') || {};
  options = jQuery.extend(options, $target.data());
  if(override) {
    console.log("was", options);
    console.log("override with", override);
    options = jQuery.extend(options, override);
  }
  console.log("ajax_handler options=", options);
  console.log("ajax_handler target id=", $target.attr('id'));
  //return false;
  
  
  //extract(options, 'EXTR_PREFIX_SAME', 'ajax');
  
  var id = options.form_id = $target.attr("id");
  if(!id) {
    // assign random ID
    id = random_id();
    $target.attr('id', id);
    options.form_id = id;
  }
  
  if(id && options.handler) {
    var script = options.script || options.handler_url || "/ajax.php";
    var url = options.url = script + "?oper=" + options.handler;
    console.log("\n\n\najax_handler url=" + url);
    //var url = options.url = "/ajax.php?oper=" + options.handler;
    
    delete(options.handler); // ajax method: to be confused with success_handler
    
    // pre processing
    
    // intl-tel-input    
    var $phones = $("#" + id + " input[type='tel']");  
    $phones.each(function() {
      var $phone = $(this);
      var intlNumber = $phone.intlTelInput("getNumber");
      if(intlNumber) $phone.val(intlNumber);
    });
        
    var org_text = $target.data('org_text');
    if(org_text) options.org_text = org_text;
    ajaxSubmit(options);
    console.log("OK: Ajax submitted, returning false");
    return false;
  }
  
  console.log("Not Submitting form id=" + id + " handler=" + options.handler);
  //console.log("Options=", options);
  //console.log("Data=", $target.data());
  return true; // let form submit    
}

// used by bootstrap datepicker to set value
function datepicker_setval($dp) {
  var val = $dp.val();
  var name = $dp.attr("name");
  console.log("datepicker_setval " + name + " val = " + val);
  if(val) {
    var is_valid = is_date(val);
    if(is_valid) {
      var new_val = moment(val).format(g_moment_format);
      //var new_val = val;
      //console.log("dp " + name + " val = " + val + " (valid) new=" + new_val);
      $dp.val(new_val)
    } else {
      //console.log("dp " + name + " val = " + val + " is not yyyy-mm-dd");            
    }
  }
}

/** not in use 
function init_date_range() {
  init_datepicker_ranges();
  return false;
}
function init_datepicker_range($dp) {
  
  return;
  var id = $dp.attr('id');
  var initialized = $dp.data('init');
  if($dp.data('init')) {                              
    //console.log("init_datepicker_range: " + id + " is already initialized");
    //return;
  }
  console.log("init_datepicker_range: " + id + " initialized=" + initialized);
  $dp.datepicker('destroy'); // destroy, unbind
  
  $dp.datepicker(g_dp_options);
  //console.log("returning...\n\n");
  //return;
  
  var $start = $dp.find('input').eq(0);
  var $end = $dp.find('input').eq(1);
  var start = $start.val();
  var end = $end.val();
  
  console.log("init_datepicker_range: start=" + $start.attr('name') + ' val=' + start + ' end=' + end);
  //console.log("end=" + $end.attr('name'));
  var update = $dp.data('update');
  if(typeof update == "undefined") var update = true;
  //console.log("datepicker range id= " + id + " update=" + update);
  
  var sd1 = $start.data("date-start-date");
  var sd2 = $end.data("date-start-date");
  var ed1 = $start.data("date-end-date");
  var ed2 = $end.data("date-end-date");
  if(sd1) $start.datepicker('setStartDate', sd1); 
  if(sd2) $end.datepicker('setStartDate', sd2); 
  if(ed1) $start.datepicker('setEndDate', ed1); 
  if(ed2) $end.datepicker('setEndDate', ed2); 
  
  
  $dp.datepicker(g_dp_options).on('changeDate', function(e) {
    var handler = $dp.data('onchange')
    var $start = $dp.find('input').eq(0);
    var $end = $dp.find('input').eq(1);
    var start = $start.val();
    var end = $end.val();
    
    console.log("dp range change handler=" + handler + " start=" + $start.val() + " end=" + $end.val());
    if(handler) { // another handler is set
      console.log("Handler=" + handler);
      window[handler](); // eval = evil
    }
    datepicker_set_duration($dp, 'days');
    if(handler = $dp.data('onchange')) { // another handler is set
      window[handler](); // eval = evil
    }
    
  });
  
  datepicker_set_duration($dp, 'days');
  if($start.val() || $end.val()) {
    update = false; // value already set
    //console.log("no update, vals set");
  } else {
    //console.log("no vals:, start=" + $start.val());
  }
      
  if(update) {
    console.log("Running dp update start=" + start + " end=" + end);
    var minDate = $start.data('start');
    if(minDate) $start.datepicker('setStartDate', minDate);
    $start.datepicker().on('changeDate', function(e) {
       var start_date = $start.datepicker('getDate');
       var momStart = moment(start_date);
       var momTomorrow = momStart.add(1, 'd');
       
       //console.log("start date=", start_date);
       //console.log("momStart=", momStart);
       //console.log("momTomorrow=", momTomorrow);
       //console.log("tomorrow date=", momTomorrow.format(g_moment_sql_format));
       
       $end.datepicker('setDate', start_date);
       $end.datepicker('setStartDate', start_date);
       $start.datepicker('hide');
       $end.datepicker('show');
    });
  }  
  $dp.data('init', 1);
}
*/

function init_datepicker_ranges() {
  var $dps = $(".input-daterange");
  //if($dps.length) console.log("Found " +  $dps.length + " datepicker ranges v3 options=",g_dp_options);
  $dps.each(function() {
    var $range = $(this);
    var id = $range.attr('id');
    var onchange = $range.data('onchange');
    //console.log("Found range id=" + id + " onchange=" + onchange + " class=" + $range.attr('class') + " this:", $range);

    $range.datepicker(g_dp_options);
    if(onchange) {
      window[onchange]($range); // eval = evil: call on init
      $range.datepicker().on('changeDate', function(e) {
        window[onchange]($range); // eval = evil
      });
    }
    
    // update out when in changes
    var $start = $range.find('input').eq(0);
    var $end = $range.find('input').eq(1);    
        
    /** debug */
    
    $start.datepicker().on('changeDate', function(e) {
      var startdate = $start.datepicker('getDate');
      var enddate = $end.datepicker('getDate');
      //console.log("datepicker range change: start=", startdate, " end=", enddate);

      var momStart = moment($start.datepicker('getDate'))
      var momEnd = moment($end.datepicker('getDate'))
      var start_date = momStart.format(g_moment_format);
      var end_date = momEnd.format(g_moment_format);

      var default_period = 1; 
      if (typeof $range.data('default-period') !== 'undefined') {
        default_period =  $range.data('default-period');
      }
      
      if(!$end.val() || start_date >= end_date) {        
        //console.log("ok, opening 2nd picker...");
        var momTomorrow = momStart.add(default_period, 'd');
        var tomorrow = momTomorrow.format(g_moment_format)
        
        $end.datepicker('setDate', tomorrow);
        $start.datepicker('hide');
        $end.datepicker('show');        
      }
    });
    
  });
}

function datepicker_set_duration($dp) {

  var $duration = $dp.find('.duration');  // num days
  if(!$duration.length) return;
  var $inputs = $dp.find("input");
  var $dur = $inputs.eq(2);     
  if(!$dur.length) return;
    
  var num_days = datepicker_get_duration($dp);  
  $dur.val(num_days);
  var text = num_days + ' ' + (num_days == 1 ? 'day' : 'days');
  $duration.html(text);

  // console.log("datepicker_set_duration: days=" + num_days + " text=" + text);
}

// looks for element with class duration in date range, sets number of days (by default)
function datepicker_get_duration($dp, unit) {
  if(typeof unit == "undefined") var unit = 'days';
  var $inputs = $dp.find("input");
  
  var $inp = $inputs.eq(0);
  var $out = $inputs.eq(1);
  if(!$inp.length ||  !$out.length) return 0;

  if($inp.val() && $out.val()) {
    var moment_in = moment($inp.val(), g_moment_format);
    var moment_out = moment($out.val(), g_moment_format);
    var num_units = moment_out.diff(moment_in, unit, true);
    console.log("datepicker_get_duration: number of " + unit + " between " + $inp.val() +  " and " + $out.val() + " = " + num_units);
    return num_units;
  }
  return 0;
}

function on_paste(content) {
  setTimeout(function () {
      editor.code(content.target.textContent);
  }, 10);
}

function caller_stack() {
  return '';
  
  var callee = arguments.callee;
  if(callee && callee.caller) {
    var thecaller = callee.caller;
    if(thecaller.caller) {
      return thecaller.caller.name;
    }
    return 'self';
  }
  return 'none';  
  //return arguments.callee.caller.name;
}

var ready;

function ready_script() {
  console.log("Ready Script: calling:", g_ready_scripts);
  for(i=0;i < g_ready_scripts.length; i++) {
    var fn = g_ready_scripts[i];
    console.log("Calling " + fn);
    window[fn](); // eval = evil      
  }
}

function afr_ready() {
  if(ready) {
    console.log("AFR Ready v2.1 x2");
    //return;
  } else {    
    console.log("AFR Ready v2.1 ");
  }
  
  var pageHeight = $(window).height();
  var pageWidth = $(window).width();
  var navigationHeight = $("#navigation").outerHeight();
  
  /**
  *   ON RESIZE, check again
  */
  $(window).resize(function () {
    pageWidth = $(window).width();
    pageHeight = $(window).height();
  });
                                                
  //if($('.bootstrap-switch').length) {
  //  //console.log("Found " + $('.bootstrap-switch').length + " bootstrap-switch");
  //  $('.bootstrap-switch').bootstrapSwitch().on('switchChange.bootstrapSwitch', function (e, checked) {
  //     //var $cb=(this);
  //     //if(checked) $cb.prop('checked', "checked");
  //     // //console.log("this=",$cb);
  //     //console.log("sw change val=",checked);
  //     
  //  });
  //}

  $countdown = $(".character-countdown");
  if($countdown.length) {
    //console.log("Found " + $countdown.length + " countdowns");
    $.each($countdown, function(index) {
      var max = $(this).attr('maxlength') || $(this).data('max');
      var name = $(this).attr('name');
      var target = $(this).data('countdownTarget');
      var $target = target ? $(target) : $(this).parent().find('.countdown-characters');
      
      //console.log("Name=" + name + " max=" + max + " target=" + target + " len=" + $target.length);
      //if(max > 0 && $target.length) {
      if(max > 0) {
        $(this).characterCountdown({
          countdownTarget: '.ccl',
          maxChars: max
        });
      } 
    });
  }
    
  var $geo = $('.geocomplete');
  console.log("Found " + $geo.length + " geocomplete");
  if($geo.length) {
    
    $('.geocomplete').geocomplete({
      details: "form",
      //details: ".geo-details",
      detailsAttribute: "data-geo"        
      
    }).on("geocode:result", function(event, result){    
      console.log("geocomplete v1 result=", result);
      var $bounds = $('.location-bounds');
      var $center = $('.location-center');
      if($center.length || $bounds.length) {
        var loc = pac_parse_geometry(result);
        $center.val(JSON.stringify(loc.center));
        $bounds.val(JSON.stringify(loc.bounds));
      }
      
    });
  }

  /** normal = 98  long = 196 */
  $expandable = $(".expandable");
  //console.log("Found " + $expandable.length + " expandables");
  if($expandable.length) {
    $.each($expandable, function(index) {
      var $ex = $(this);
      var id = $ex.attr("id");
      var $content = $ex.find(".expandable-content");      
      var lng = $ex.hasClass("expandable-content-long");
      var maxh = lng ? 196 : 98;
      var h = $content.height();
      
      if(h > maxh) {
        //console.log("h > max: " + h + "/" + maxh + " collapse");
        //console.log("has expanded ?=" + $ex.hasClass("expanded"));
        $ex.removeClass('expanded');
      } else {
        //console.log("h < max: " + h + "/" + maxh);
        //$trigger.hide();
      }
      
      //console.log("Found expandable id=" + id + " long=" + lng + " h=" + $ex.height() + " ch=" + $content.height());
      //if(!id) console.log("Found expandable NO id=", $ex);
    });      
  }
     
  var $bst = $('.bootstrap-toggle');    
  if($bst.length) {
    console.log("AFR: Found " + $bst.length + " bootstrap-toggle");
    $bst.bootstrapToggle();
  }
  
  if($("#language-tag-list").length) {
    //console.log("GH: Found language tag list");
    $(function() {
      var lang_str = $('#language-tag-list').data('langs');
      var langs = lang_str ? lang_str.split(',') : [];
      //console.log("Langs=", langs);      
      var tags = $('#language-tag-list').tags({
          tagData: langs,
          suggestions:["English", "French", "German", "Spanish", "Portugese", "Italian", "Japanese", "Chinese", "Norwegian", "Dutch", "Catalan", "Danish", "Swedish", "Icelandic", "Russian", "Greek", "Hebrew"],
          //suggestions: {"en": "English", "fr": "French"},
          excludeList:[],
          caseInsensitive: true,
          promptText: "Type language then press enter for each",
          afterAddingTag: function(str) {
            var $inputfld = $("#languages_input");
            //console.log("added " + str);
            var allTags = tags.getTags();
            //console.log("tags= ", allTags);
            $inputfld.val(allTags.join(','));
            //console.log("new2: setting input with len=" + $inputfld.length + " to " + allTags.join(','));
          },
          afterDeletingTag: function(str) {
            var $input = $("#languages_input");
            //console.log("deleted " + str);
            var allTags = tags.getTags();
            //console.log("tags= ", allTags);
            $input.val(allTags.join(','));
          },
      });
    });
  } 
  
  if($(".summernote").length) {
    //console.log("GH: Found " + $('.summernote').length + " summernote");
    //var toolbar = {};
    $(".summernote").each(function() {
      var $editor = $(this);
      var default_toolbar = [
          ["style", ["bold", "italic", "underline", "clear"]]
        ];
      var toolbar = $editor.data('toolbar') || default_toolbar;
      var options = {onpaste: on_paste};
      var full = $editor.data("full");
      if(!full) options.toolbar = toolbar;
      //console.log("summernote: full=" + full + " toolbar =", toolbar);
      $editor.summernote(options);
    });
  }
  
  if($(".wysiwyg").length) {
    //console.log("GH: Found " + $('.wysiwyg').length + " wysiwyg");
    $(".wysiwyg").each(function() {
      var $editor = $(this);
      $editor.wysihtml5();
    });
  }
  
  if($(".jquery-fileupload").length) {
    //console.log("Found " +  $(".jquery-fileupload").length + " fileuploaders");
    $(".jquery-fileupload").each(function() {
      var $target = $(this);
      if(!$target.data('initialized')) { 
        //console.log("ready script: calling init_fileuploader for " + $(this).attr('id'));
        init_fileuploader($(this));
      }
    });
  }

  /** new, using bootstrap-timepicker or timepicker-jt */
  
  if($(".timepicker").length) {
    console.log("Found " + $('.timepicker').length + " BS timepickers");
    if(typeof $.fn.timepicker == 'function') {
      $('.timepicker').timepicker({
        'showMeridian': false,        
      });
    } else {
      console.log("Timepicker function missing...");      
    }
  }
  
  
  /**
  if(0 && $(".periodpicker").length) {
    $(".periodpicker").each(function() {
      var $range = $(this);
      var $start = $range.find('input').eq(0);
      var $end = $range.find('input').eq(1);
      var start_id = $start.length ? $start.attr('id') : '';
      var end_id = $end.length ? $end.attr('id') : '';
      console.log("Found " + $('.periodpicker').length + " periodpickers start=" + start_id + " end=" + end_id);
      if(start_id && end_id) {
        $('#' + start_id).periodpicker({
         end: '#' + end_id,
         timepicker: true, // use timepicker
         timepickerOptions: {
           hours: true,
           minutes: true,
           seconds: false,
           ampm: false
         }
         
        });
      }
    });
  }
  */
  
  /**
  if($(".datetimepicker").length) {
    console.log("Found " + $('.datetimepicker').length + " datetimepickers");
    $(".datetimepicker").datetimepicker({
      //format: "dd-mm-yyyy - hh:ii",
      //linkField: "end_date",
      //linkFormat: "yyyy-mm-dd hh:ii:ss",
      autoclose: true,
      todayBtn: true,
      minuteStep: 15
    });
  }
  */
  
  if($(".datepicker").length) {
    console.log("Found " + $('.datepicker').length + " datepicker");
    $('.datepicker').datepicker(g_dp_options);
  }

  /**
  $('.timepicker').on('changeTime', function() {
    console.log("Changed to:" + $(this).val());
  });
  */
  
  init_datepicker_ranges();
  
  $("SELECT.auto-select").each(function() {
    var $target = $(this);
    var name = $target.attr('name');
    var value = $target.data("value");
    // console.log("auto-select name=" + name + " value=" + value);
    if(value) $target.val(value);
    if($target.hasClass('autocomplete') && typeof $.fn.selectToAutocomplete == 'function') {
      $target.selectToAutocomplete();
    }
  
  });
  
  $(".ajax-switch").each(function() {
    var $target = $(this);
    var disabled = $target.hasClass('disabled') ? true : false;
    var state = $(this).data('state');
    //console.log("On-Off Switch id=" + $target.attr('id'));
    $target.iphoneSwitch(
      state, // start state
      function() {
        iphoneSwitchOnOff($target, 1);
      },
     function() {
        iphoneSwitchOnOff($target, 0);
     },
     {
       path: '/afr/jquery/ajax-switch/',
       disabled: disabled
     });

  });

 
  $(".jq-autocomplete").each(function() {
    var options = $(this).data('options') || {};
    var item_renderer = options.item_renderer
    //console.log(".jq-autocomplete options=");
    //console.log(options);
    //console.log("item renderer=" + item_renderer);
    
    var search_options = $(this).data('search_options') || {};
    var id = $(this).attr('id');
    var ajax_url = "/home/ajax/searchJSON.php?" + obj2qs({"options": JSON.stringify(search_options)});    
    $(this).autocomplete({
      source: ajax_url,
      minLength: 2,
      select: options.select_handler ? eval(options.select_handler) : '', 
      response: options.response_handler ? eval(options.response_handler) : '',
    });
    if(item_renderer) $(this).data("ui-autocomplete")._renderItem = eval(item_renderer);
  });
  
  $('[data-toggle="popover"]').popover(); 
  
  //console.log("Found " + $('[data-toggle="validator"]').length + " bootstrap validator");  
  var $valid = $('[data-toggle="validator"]');
  if($valid.length) {
    console.log("Found " + $valid.length + " validators");
    $valid.validator({
      custom: {
        'iban': function($el) {
          var iban = $el.val();
          var valid = IBAN.isValid(iban)
          //console.log("Checking iban:" + iban + " valid=" + valid);
          return valid;
        }
      },
      errors: {
        match: "Does not match",
        minlength: "Not long enough",
        iban: "Not a valid IBAN number"
      }    
    });
  }
  
  var $conf = $('[data-toggle="confirmation"]');
  if($conf.length) {
    $conf.confirmation({
      'singleton':true      
    });
  }    
    
  //console.log("Found " + $('[data-toggle="tooltip"]').length + " bootstrap tooltips");  
  $('[data-toggle="tooltip"]').tooltip({
    'container':'body'      
  });
  
  var $bs = $('.tooltip-bs');
  //console.log("Found " + $bs.length + " bs tooltips");
  $('.tooltip-bs').tooltip(); // bootstrap
  
 
  if(0) {
    //console.log("Found " + $('.tooltip-jq').length + " jquery tooltips");
    $('.tooltip-jq').uitooltip({ // use jquery
      content: function( event, ui ) {
        var tip = $(this).data("tip");
        if(tip) return tip;
      }
    });
  } else {
    //console.log("Found " + $('.tooltip-jq').length + " jquery tooltips (now using bootstrap)");
    $('.tooltip-jq').tooltip({ // was jquery, not bootstrap
      'container':'body'
    });      
  }

  //console.log("Found " + $('.tooltip-ajax').length + " ajax tooltips");
  $('.tooltip-ajax').tooltip({
    content:function(callback) { //callback
      var url = $(this).data("url");
      $.get(url,{}, function(data) {
        callback(data); //call the callback function to return the value
      });
    },
  });


  // $(".fine-uploader").each(function() {
  //   var $target = $(this);
  //   if(!$target.data('initialized')) { 
  //     //console.log("calling init_uploader for " + $(this).attr('id'));
  //     init_uploader($(this));
  //   }
  // });
  
  // dialog handlers: todo only do if target is dialog
  $(".jquery-tabs").each(function() {
    var $target = $(this);
    var options = $target.data('options') || {};
    $target.tabs(options); // magnifiying glass
  });  
  
  fancybox_loader();

  
  // simple accordion for UL 
  // https://css-tricks.com/snippets/jquery/simple-jquery-accordion/
  // UL must be in DIV with class accordion
  if($('UL.option-list').length) {
    var allPanels = $('UL.option-list > li > .option-list-content');    
    //console.log("found " + $('UL.action-list').length + " lists with # panels=" + allPanels.length);
      
    $('.option-list > li > a').click(function() {
      var $content = $(this).parent().find(".option-list-content");
      var id = $content.attr('id');
      //console.log("click on option list link id=" + id);
      
      allPanels.hide();
      $(this).parent().find(".option-list-content").removeClass('hide').show();
      return false;
    });
  }

  if($('UL.action-list').length) {
    //console.log("found " + $('UL.action-list').length + " action lists with # actions=" + allActions.length);
    $('.action-list > li.action-list-item INPUT:radio').click(function() {      
      var $list = $(this).closest('UL');
      var $item = $(this).closest('LI');
      //console.log("click on action item link class=" + $item.attr("class") + " in list with class=" + $list.attr("class"));
      $list.find('.action-list-content').hide();
      $item.find('.action-list-content').removeClass('hide').show();
      return true;
    });
  }
  
  $tel = $('input[type=tel]');
  if($tel.length) {
    console.log("found " + $tel.length + " phone inputs");
    $tel.intlTelInput({
      initialCountry: "auto",
      geoIpLookup: function(callback) {
        $.get('https://ipinfo.io', function() {}, "jsonp").always(function(resp) {
          var countryCode = (resp && resp.country) ? resp.country : "";
          callback(countryCode);
        });
      },
      utilsScript: "/afr/bower/intl-tel-input/build/js/utils.js" // just for formatting/placeholders etc
    });      
    
    //$tel.intlTelInput({
    //  nationalMode: true,
    //  utilsScript: "/afr/bower/intl-tel-input/lib/libphonenumber/build/utils.js" // just for formatting/placeholders etc
    //});
  }
  
  $popup = $('.ajax-popup-link');
  if($popup.length) {
    //console.log("found " + $popup.length + " magnificPopup popup links");
    //$($popup).magnificPopup({
    //  type: 'ajax'
    //});    
  }
         
  $parallax = $('.parallax');

  $agenda_calendar = $('#agenda_calendar');
  if($agenda_calendar.length) {
    var options = $agenda_calendar.data('options') || {};

    var url = options.url ? options.url : $agenda_calendar.data('url');
    var editable = options.editable || 0;

    //console.log("Found agenda calendar url=" + url);
    console.log("Found agenda calendar options=", options);
    //console.log("Found agenda calendar data=", $agenda_calendar.data());
    console.log("Found agenda calendar editable=" + editable);
    //var start = '2015-08-01';
    //var start = '2015-08-31';
    options.eventMouseover = function(calEvent, e, view) {
      var data = calEvent.data
      var $event = $(this);
      $event.addClass('fc-event-selected');
      $event.css("background-color", "#8f8");
      var title = data.title;
      //var range = moment.range(data.start_time, data.end_time);
      var range = '';
      if(data.all_day) {
      } else {
      }
      title = title + ":" + range;
      
      var address = data.address_formatted ?  data.address_formatted : data.start_address;
      if(address) title = title + "<br>" + address;
      $event.popover({html:true,title:title,content:jQuery.truncate(data.description,{length: 100}),placement:'top',container:'body'}).popover('show');
    };
    
    options.eventMouseout = function(calEvent, e, view) {
      console.log("mouseout", calEvent.data);
      var $event = $(this);
      $event.removeClass('fc-event-selected');
      $event.css("background-color", "#fff");
      $event.popover('hide');
    };
   
    options.eventClick = function(calEvent, e, view) {
      var $div = $(this);                 
      var link = calEvent.link;
      console.log("event click",calEvent);

      if(editable) {
        var data = calEvent.data;
        console.log("event data",data);
        
        event_form_set(data);
        $('#event-form-modal').modal('show');
      } else if(link) {
        console.log("click on event (not editable) link=" + link);
        window.open(link, 'event');
        //if(link) 
      }
      //fc_dialog($div, calEvent);

    };
    
    options.events = function(start, end, timezone, callback) {
      //alert("Loading agenda events from " + url);
      var options = $agenda_calendar.data('options') || {};
      var url = options.url ? options.url : $agenda_calendar.data('url');
  
      var checkin = start.format("YYYY-MM-DD");
      var checkout = end.format("YYYY-MM-DD");
      
      console.log("Found agenda calendar url=" + url);
      
      $.ajax({
        url: url,
        dataType: 'json',
        data: {
          start: checkin,
          end: checkout
        },          
        success: function(events) {
          console.log("Loaded agenda events from url=" + url + " date=" + checkin + " - " + checkout);
          $.each(events, function(i, event) {
            //console.log("Loaded event ", event);
          });
          callback(events);
          
        }
      });
    };
    
    if(editable) {
      options.select = function( start, end, jsEvent, view) {
        var num_days = end.diff(start, 'days');
        var checkin = start.format("YYYY-MM-DD");
        var checkout = end.format("YYYY-MM-DD");
  
        var g_fc_selection = {"start": checkin, "end": checkout, "days": num_days}; // save selection
        $fullcalendar.data('selection', g_fc_selection);
        
        console.log("selected start=",start);
        var event = {"start": start, "end": end};
        console.log("Event:", event);
  
        var data = $agenda_calendar.data('data') || {};
        data.start_time = checkin;
        data.end_time = checkout;
        data.all_day = 1;
        //var data = {'start_date': checkin, 'end_date': checkout};
        event_form_set(data);
        
        
        $('#event-form-modal').modal('show');
      };
    } 

    
    $agenda_calendar.fullCalendar(options);
    
  }
  
    
  /**
  if($parallax.length) {
    //console.log("Found " + $parallax.length + " elements with parallax effect pageWidth=" + pageWidth);
    if (1 || typeof parallax == 'function') {        
      if(pageWidth > 980) {
        $.each($parallax, function(i, el) {
          //console.log("Calling parallax on " + $(this).attr("class"));
          $(el).parallax("90%", 0.15);
        });
      }
    } else {
      //console.log("Parallax is not installed");
    }
  }
  */
  /** start fullcalendar */
  $fullcalendar = $('#fullcalendar');
  if($fullcalendar.length) {
    fullcalendar_init($fullcalendar);
  }

  $('.bootstrap-table').each(function() {
    var $table = $(this);
    $table.bootstrapTable()  
  });
  
  
  $('INPUT.typeahead').each(function() {
    var $input = $(this);
    var url = $input.data('url');
    var operation = $input.data('operation');
    var public_operation = $input.data('public');
    var path = public_operation ? "/ajax.php" : "/ajax.php";
    var ajax_url = operation ? path + "?oper=" + operation : url;
    
    var target_name = $input.data('target');
    var target_id = $input.data('target_id');
    var target_email = $input.data('target_email');

    var engine, remoteHost, template, empty;
    $.support.cors = true;
  
    //remoteHost = 'https://typeahead-js-twitter-api-proxy.herokuapp.com';
    template = Handlebars.compile($("#result-template").html());
    empty = Handlebars.compile($("#empty-template").html());
    
    console.log("typeahead url=" + ajax_url);
    $input.typeahead({
        hint: true,
        highlight: true,
        minLength: 3,
        limit: 8,
      }, {
        source: function(q, cb) {
          return $.ajax({
            dataType: 'json',
            type: 'get',
            url: ajax_url + '&q=' + q,
            cache: false,
            success: function(response) {
              if(response.success) {
                var data = response.data || {};
                var result = [];
                $.each(data, function(index, val) {
                  result.push(val);
                  //result.push({
                  //  id: val.id,
                  //  value: val.name
                  //});
                });
                cb(result);
              } else {
                cb(response.error || "unknown error");
              }
            }
          });
        },
        templates: {
          //suggestion: template,
          empty: empty
        }
        
      })
      .on('typeahead:selected', function(obj, datum) {
        console.log("selected obj:", obj);
        console.log("selected datum:", datum);
        if(target_id) {
          console.log("setting " + target_id + " to " + datum.id);
          $(target_id).val(datum.id).trigger('change');                                  
        }
        if(target_name) {
          console.log("setting " + target_name + " to " + datum.value);
          $(target_name).val(datum.value).trigger('change');
        }
        
        if(target_email) {
          var email_string = datum.full_name + ' <' + datum.email_address + '>';
          console.log("setting " + target_email + " to " + email_string + " datum:", datum);
          $(target_email).val(email_string);
        }
        
        close_parent($(this));
      })
      .on('typeahead:asyncrequest', function() {
        console.log('showing spinner');
        $('.typeahead-spinner').show();
      })
      .on('typeahead:asynccancel typeahead:asyncreceive', function() {
        $('.typeahead-spinner').hide();
      });
            
      
    });
  
    
  /**    
  $('INPUT.typeahead').each(function() {
    var $input = $(this);
    var url = $input.data('url');
    var operation = $input.data('operation');
    var ajax_url = operation ? "/ajax.php?oper=" + operation : url;
    if(ajax_url) {
      var bh = new Bloodhound({
        datumTokenizer: Bloodhound.tokenizers.obj.whitespace('value'),
        queryTokenizer: Bloodhound.tokenizers.whitespace,
        //prefetch: '../data/films/post_1960.json',
        remote: {
          url: ajax_url,
          wildcard: '%QUERY'
        }
      });
      
      $input.typeahead(null, {
        name: 'best-pictures',
        display: 'value',
        source: bh
      });
    }
  });
  */
  

  // Lighbox gallery
  $('#popup-gallery').each(function() {
      $(this).magnificPopup({
          delegate: 'a.popup-gallery-image',
          type: 'image',
          gallery: {
              enabled: true
          }
      });
  });
  
  if($('.popup-image').length || $('.popup-text').length || $('.popup-ajax').length || $('.popup-iframe').length) {
    // Lighbox image
    $('.popup-image').magnificPopup({
        type: 'image'
    });
    
    // Lighbox text
    $('.popup-text').magnificPopup({
        removalDelay: 500,
        closeBtnInside: true,
        callbacks: {
            beforeOpen: function() {
                this.st.mainClass = this.st.el.attr('data-effect');
            },
            open: function() {
                var panel = this.st.el.attr('data-panel') || '';
                var handler = this.st.el.attr('data-mfp-handler') || '';
                if(panel) {
                  $(".toggled-div").hide();
                  $(panel).removeClass('hidden').show();
                  console.log("mp: after-open panel=" + panel);
                }
                if(handler) {
                  var value = this.st.el.attr('data-value');
                  //var data = $(this).data();
                  window[handler](value); // eval = evil
                  
                }
            }
        },
        midClick: true
    });
    // Lightbox iframe
    // console.log("mfp iframe count=" + $('.popup-iframe').length);
    $('.popup-iframe').magnificPopup({
        dispableOn: 700,
        type: 'iframe',
        removalDelay: 160,
        mainClass: 'mfp-fade',
        preloader: false
    });
  
    // console.log("mfp ajax count=" + $('.popup-ajax').length);
    $('.popup-ajax').magnificPopup({
        dispableOn: 700,
        type: 'iframe',
        removalDelay: 160,
        mainClass: 'mfp-fade',
        preloader: false
    });    
  }
  

  
  // Thanks: http://stackoverflow.com/questions/7862233/twitter-bootstrap-tabs-go-to-specific-tab-on-page-reload-or-hyperlink
  var url = document.location.toString();
  if($().length)
  console.log("url=" + url);
  if ($('ul.nav-tabs').length && url.match('#')) {
    var selector = 'ul.nav-tabs li a[href="#' + url.split('#')[1] + '"]';
    if($(selector).length) $(selector).tab('show');
  }

  //console.log("gh: subview len=" + $(".subview-nav").length + " set=" + g_subview_set);
  /** this is used to highlight the correct subview nav link in left menu */
  if($(".subview-nav").length && !g_subview_set) {
    
    // select subview menu item based on path
    var path = window.location.pathname;   
    var $list = $(".subview-nav").find("UL LI");
    var pin_code = $(".subview-nav").data('pin_code');
    if(pin_code) path = path + "?pin=" + pin_code;
    var $match = $(".subview-nav").find('a[href="' + path + '"]');
    //console.log("Subview: Looking for:" + path + " pin=" + pin_code + " found:" + $match.length); 
    if($list.length && $match.length) {
      $list.removeClass('active');
      $match.parent().addClass('active');
      g_subview_set = 1;
    }
  }
  
    
  
  // Javascript to enable link to tab
  
  
  console.log("Ready script href " + window.location.href);
  console.log("Ready script location: " + document.location);
  console.log("Ready script hash " + document.location.hash);
  
  ready = 1; 
}

/** below don't work in ready_script because window.location is old */
// Change hash for page-reload
$('.nav-tabs a').on('shown.bs.tab', function (e) {
  window.location.hash = e.target.hash;
})

// Javascript to enable link to bootstrap tab
var hash = document.location.hash;
if (hash) {
  console.log("hash=" + hash);
  $('.nav-tabs a[href="'+hash+'"]').tab('show');
}

// Change hash for page-reload
$('a[data-toggle="tab"]').on('show.bs.tab', function (e) {
  window.location.hash = e.target.hash;
});

/** called to early, sets old value... arghh ?? */
function request_login_handler(value) {
  var ref = $("#register-redirect").val();
  // $(".tpl-link").attr('href', ref);
}

function fullcalendar_init($fullcalendar) {
    
  var options = $fullcalendar.data('options') || {};
  
  var apt = $fullcalendar.data('rental') || {}; // {"id": 10, "name": "fooapt", "type": "shortstay"};
  var url = $fullcalendar.data('url') || '';
  var checkin = apt.in || '';
  var checkout = apt.out || '';
  $fullcalendar.data('max_date', '');
  $fullcalendar.data('min_date', '');
  $fullcalendar.data('dates', {});
  $fullcalendar.data('weeks', {});
  $fullcalendar.data('weeks2', {});
  $fullcalendar.data('weeks3', {});
  $fullcalendar.data('months', {});
  $fullcalendar.data('selection', null);
  
  options.weekNumberCalculation = "ISO"; // used internally, not displayed
  options.firstDay = 1; // monday
  
  //console.log("fullcal apt", apt, " options=", options);
  if(apt.id) {
    options.weekNumbers = apt.rate_week > 0 ? true : false;
    
    //console.log("fullcal apt_id=" + apt.id);
    var editable = options.editable;
    if(editable) {
      //console.log("calendar is editable");

      options.select = function( start, end, jsEvent, view) {
        var num_days = end.diff(start, 'days');
        var g_fc_selection = {"start": checkin, "end": checkout, "days": num_days}; // save selection
        $fullcalendar.data('selection', g_fc_selection);
        
        var event = {"apt": apt, "start": start, "end": end};
        console.log("selected event=",event);
        if(0) { // direc to reservation edit
          fc_booking_dialog($(this), event);
        } else {
          fc_dialog($(this), event);
        }
      }
      options.dayClick = function(date, jsEvent, view) {
        var $td = $(this);
        var exception_id = $td.data('exception_id');
        if(exception_id) {
          console.log("dayclick id=" + exception_id + " date:", date);
        }
      }
      options.eventResize = function (event, delta, revertFunc, jsEvent, ui, view ) {
        fc_reset_day_rate(apt);
        var days = delta.days();
        var obj_type = event.obj_type;
        var obj_id = event.obj_id;
        var success = false;
        //console.log("resized " + obj_type + " " + obj_id + " days=" + days);
        if((obj_type =='reservation' || obj_type == 'res_exception') && obj_id && days) {
          var end_field = obj_type == 'reservation' ? 'checkout' : 'end';
          var ajax_url = '/ajax.php?oper=edit&obj_type=' + obj_type + '&id=' + obj_id + "&" + end_field + "=" + event.end.format(g_moment_sql_format);
          //console.log("Calling " + ajax_url);          
          $.getJSON(ajax_url, function(json) {
            success = json.success;
            var error = json.error;
            if(error) alert(error);
            //console.log("Result=", json);            
            if(!success) fc_revert(apt,revertFunc);
           });
        } else {
          //console.log("No Change: revert: ");
          fc_revert(apt,revertFunc);  
        }        
      }
      
      options.eventDrop = function (event, delta, revertFunc, jsEvent, ui, view ) {
        fc_reset_day_rate(apt);
        var days = delta.days();
        var obj_type = event.obj_type;
        var obj_id = event.obj_id;
        var success = false;
        //console.log("dragged " + obj_type + " " + obj_id + " days=" + days);
        if((obj_type =='reservation' || obj_type == 'res_exception') && obj_id && days) {
          var end_field = obj_type == 'reservation' ? 'checkout' : 'end';
          var start_field = obj_type == 'reservation' ? 'checkin' : 'start';
          var ajax_url = '/ajax.php?oper=edit&obj_type=' + obj_type + '&id=' + obj_id + "&" + start_field + "=" + event.start.format(g_moment_sql_format) + "&" + end_field + "=" + event.end.format(g_moment_sql_format);
          //console.log("Calling " + ajax_url);          
          $.getJSON(ajax_url, function(json) {
            success = json.success;
            var error = json.error;
            if(error) alert(error);
            //console.log("Result=", json);            
            if(!success) fc_revert(apt,revertFunc);
           });
        } else {
          //console.log("No Change: revert: ");
          fc_revert(apt,revertFunc);          
        }        
      }
      
      options.eventClick = function(calEvent, e, view) {
        var $div = $(this);                 
        console.log("event click",calEvent);
        calEvent.apt = apt;
        fc_dialog($div, calEvent);

        /** place cal_menu at mouse pointer */
        var offset = $("#fullcalendar").offset();
        var x = (e.pageX - offset.left);
        var y = (e.pageY - offset.top);      
        $("#cal_menu").css( { "left": x + "px", "top": y + "px" } );
      }

    } else { // not editable
      //console.log("calendar is not editable (readonly)");
      options.selectOverlap = function(event) {
          return event.obj_type == 'reservation' ? false : true;
      }
      options.select = function( start, end, jsEvent, view) {
        var num_days = end.diff(start, 'days');
        var checkin = start.format(g_moment_sql_format);
        var checkout = end.format(g_moment_sql_format);          
        var g_fc_selection = {"start": checkin, "end": checkout, "days": num_days}; // save selection
        $fullcalendar.data('selection', g_fc_selection);
        $price_link = $('A.price-breakdown-link');
        if($price_link.length) {
          var data = $price_link.data('data') || {};
          data.in = checkin;
          data.out = checkout;
          $price_link.data('data', data);
        }
        
        //console.log("non-editable: select start=" + checkin + " end=" + checkout + " =" +num_days + " days");
        $("#sbd_in").val(start.format(g_moment_format));
        $("#sbd_out").val(end.format(g_moment_format)).trigger('change');          
        fc_update_average_rate($fullcalendar);
      }
    }
    
    options.dayRender = function (momdate, cell) {
      //cell.css("background-color", "red");
      var cur = apt.cur;
      var date = momdate.format('YYYY-MM-DD')   
      var weekday = momdate.day();
      var weekend = weekday == 5 || weekday == 6; // Friday or Saturday

      var day_rate = fc_get_day_rate(apt, weekend);
      
      var g_fc_dates = $fullcalendar.data('dates') || {};
      
      // print date
      if(weekend && apt.rate_weekend > 0 && apt.rate_weekend != apt.rate_day) {
        var rate_class = editable ? 'fc-weekend-rate' : '';
        var innerText = '<span class="fc-rental-rate ' + rate_class + '">' + cur + day_rate + '</span>';
      } else {
        var innerText = '<span class="fc-rental-rate">' + cur + day_rate + '</span>';
      }
      
      cell.html(innerText);        
      cell.data('rate_day', day_rate);
      g_fc_dates[date] = day_rate;
      $fullcalendar.data('dates', g_fc_dates)
    }
    
    options.eventRender = function(event, element) {
      element.html = event.description;
      element.css('background-color', '#39g');
    }
    
    options.unselect = function(event, element) {
      //console.log("unselect");
      //$fullcalendar.data('selection', null);
      //fc_update_average_rate($fullcalendar);
    }
    
    options.eventAfterRender = function (event, element, view) {
      //console.log("Done rendering exception event=" + event.obj_type + " " + event.obj_id);
      fc_event_loaded($fullcalendar, event, true);
      
      /**
      var tooltip = "foobar";
      var $tips = $(".afr-tooltip");
      $.each($tips, function(i, tip) {
        var $cell = $(tip).closest("td");
        //console.log(
        $(element).attr("data-original-title", tooltip)
        $(element).tooltip({ container: "body"})
      });
      */
    }

    options.loading = function(bool) {
      if(bool) {
        //console.log("calendar is loading..."); 
        //$('#fullcalendar-loading').show();
      } else { 
        //$('#fullcalendar-loading').hide();
        var dates = $fullcalendar.data('dates');
        var weeks = $fullcalendar.data('weeks');
        var weeks2 = $fullcalendar.data('weeks2');
        var weeks3 = $fullcalendar.data('weeks3');
        var months = $fullcalendar.data('months');
        var date_keys = Object.keys(dates);
        date_keys.sort();
        var date_min = date_keys[0];
        var date_max = date_keys[date_keys.length - 1];
        $fullcalendar.data("max_date", date_max);
        $fullcalendar.data("min_date", date_min);
        if(apt.rate_week > 0 || apt.rate_2_weeks > 0 || apt.rate_3_weeks > 0) fc_reset_week_rate($fullcalendar, apt);
        if(apt.rate_month > 0) fc_reset_month_rate($fullcalendar, apt);
        
        console.log("Loaded fullcal");
        
        var co = apt.changeover;
        
        if(co > 0) {
          var day = weekdays(co).toLowerCase().substr(0,3);
          $("th.fc-day-header.fc-" + day).append(' <i class="fa fa-refresh"></i>');
        }
        //console.log("first child=", $("th.fc-day-header:nth-child(6)"));
      }
    }
    
    // Our own event handler to we can process events that are not rendered
    options.events = function(start, end, timezone, callback) {
      //alert("Loading events from " + url);
      var checkin = start.format("YYYY-MM-DD");
      var checkout = end.format("YYYY-MM-DD");
      //console.log("Event URL:" + url + " options:", options);
      var editable = options.editable;
      $.ajax({
        url: url,
        dataType: 'json',
        data: {
          start: checkin,
          end: checkout
        },          
        success: function(response) {
          console.log("Loaded events from " + checkin + " - " + checkout + " = ", response);
          var events = response.events;
          var status, obj_type;
          $.each(events, function(i, event) {
            console.log("Loaded event " + event.obj_id + " title=" + event.title);
            if(editable) event = fc_event_data(event);              
            console.log("data=", event);
            fc_event_loaded($fullcalendar, event, false);        
          });          
          callback(events);
        }
      });
    }
    
  } else {
    options.selectable = false;
  }
  
  
  //console.log("found " + $fullcalendar.length + " fullcalendars: options=",options);
  $fullcalendar.fullCalendar(options);
  if(checkin && checkout) {
    //console.log("in=" + checkin + " out=" + checkout);
    //$fullcalendar.fullCalendar('select', moment(checkin, "YYYY-MM-DD"), moment(checkout, "YYYY-MM-DD")); // select dates
  }
  $fullcalendar.data('init', true);
}

function fc_refresh() {
  console.log("fc_refresh");
  var $fullcalendar = $("#fullcalendar");
  $fullcalendar.fullCalendar('refetchEvents');
}

function fc_exception_text(event) {
  var rate = '';
  var text = '';

  if(event.description) text += event.description +": ";
  else text = "Price rule: ";
  
  var currency = 'EUR'; /** todo: store with exception */
  var cur = currency2symbol(currency);
  
  if(event.rate_day > 0) {
    rate = cur + '' + event.rate_day;
  } else if(event.rate_change) {
    //list(abs_change, perc_change) = split_discount(val);
    rate = event.rate_change;
    console.log("rate change: " + event.rate_change);
    /**
    if(abs_change > 0 || perc_change > 0) {
      out_name = phrase('surcharge', CAPITALIZE);
      rate = perc_change > 0 ? "+".val : "+".cur.val;
    } else {
      out_name = phrase('discount', CAPITALIZE);
      val = ltrim(val, '-');
      rate = perc_change < 0 ? "-".val : "-".cur.val;
    }
    */
  } else {
  }
  
  //rate = event.rate_day ?: event.rate_change;
  text += rate;
  if(event.min_stay) text += " Min: " + event.min_stay +" days.";
  if(event.changeover) text += " Changeover: " + weekdays(event.changeover);
  return text;  
}

function fc_event_data(event) {
  obj_type = event.obj_type;
  status = parseInt(event.status);
  event.textColor = '#FFF';
  event.borderColor = 'transparent';            
  event.rendering = '';            
  
  console.log("Rendering " + obj_type + " status=" + status);
  var status = event.status;
  if(obj_type == 'res_exception') {
    event.className = "fc-exception";
    event.title = fc_exception_text(event);
    event.backgroundColor = '#0A0';      
  } else if(obj_type == 'invoice') { /** not shown in single calendar */
    return event;
  } else if(obj_type == 'reservation') {
    event.className = "fc-reservation";
    if(status < 0) {
      event.backgroundColor = '#888';
      event.status_name = 'fc-status-cancelled';
    } else {
      event.title = event.title + " (" + event  .num_guests + "pp)";
      
      switch(status) {
        case 1:
          event.backgroundColor = '#ff0';
          event.status_name = 'fc-status-draft';
          break;
        case 5:
          event.backgroundColor = '#880';
          event.status_name = 'fc-status-inquiry';
          break;
        case 10:
          event.backgroundColor = '#f80';
          event.status_name = 'fc-status-request';
          break;
        case 15:
          event.backgroundColor = '#06f';
          event.status_name = 'fc-status-proposed';
          break;
        case 20:
          event.backgroundColor = '#06f';
          event.status_name = 'fc-status-pending';
          break;
        case 30:
          event.backgroundColor = '#c00';
          event.status_name = 'fc-status-reserved';
          break;
        case 35:
          event.backgroundColor = '#800';
          event.status_name = 'fc-status-blocked';
          event.rendering = 'background';                   
          break;
        case 40:
          event.backgroundColor = '#080';
          event.status_name = 'fc-status-complete';
          break;
        default:
          event.backgroundColor = '#000';
          event.status_name = 'fc-status-unknown';
          break;
      }
    }
  }
  return event;  
}
    

function weekdays(day) {
  var weekdays = {1:"Monday", 2:"Tuesday", 3:"Wednesday", 4:"Thursday", 5:"Friday", 6:"Saturday", 7:"Sunday"};
  return weekdays[day];
}

function fc_booking_dialog($target, obj) {
  var apt = obj.apt;
  var start = obj.start;
  var end = obj.end;
  var obj_type = obj.obj_type;
  var obj_id = obj.obj_id || 0;

  var data = {"notes": '', "rate_day": '', "status": 0, "min_stay": 0, "changeover": 0, "last_name": '', "comments": '', "obj_type": obj_type, "obj_id": obj_id, "property_type": apt.type, "apt_id": apt.id, "apartment_name": apt.name, "checkin": start, "checkout": end};
  console.log("show booking dialog", data);

  cal_booking_dialog(data);
  
}

// simple dialog 
function show_simple_dialog(data) {
  var options = {};
  var modal = "#overlay_content_simple"; // '#calModal'
  $("#test_pricing").html('');
  $(modal).find(".show-none").show();
  $(modal).modal(options);
}


function periodpicker_init($range, all_day) {  
  if(!$range.length) {
    console.log("Range not found");
    return false;
  }
  var $start = $range.find('input').eq(0);
  var $end = $range.find('input').eq(1);
  var start_id = $start.length ? $start.attr('id') : '';
  var end_id = $end.length ? $end.attr('id') : '';
  
  if($range.data('init')) {
    $('#' + start_id).periodpicker('destroy');    
    $range.data('init', 0);    
  }
  
  var today = date_today();
  var two_years = add_day(today, 720);
  
  //console.log("Init range:" + $range.attr('id'));
  if(start_id && end_id) {
    $('#' + start_id).periodpicker({
     formatDate: 'YYYY-MM-DD', /** how it's stored internally w/o time */
     formatDateTime: 'YYYY-MM-DD HH:mm:ss', /** how it's stored internally with time */
     formatDecoreDateTime: 'DD MMMM YYYY HH:mm', /** how it's displayed with time */
     formatDecoreDateTimeWithoutMonth: 'DD MMMM YYYY HH:mm', /** how it's displayed with time */
     formatDecoreDateTimeWithYear: 'DD MMMM YYYY HH:mm', /** how it's displayed with time */
     end: '#' + end_id,
     //noHideSourceInputs: true,
     minDate: today,
     maxDate: two_years,
     timepicker: !all_day, // use timepicker
     timepickerOptions: {
       defaultTime: '19:00',
       defaultEndTime: '20:00',
       hours: true,
       minutes: true,
       seconds: false,
       ampm: false
     }
     
    });
    $range.data('init', 1);    
    return true;
  }
}
  

function fc_dialog($target, obj) {
  hideMenu();         
  
  var apt = obj.apt;
  var start = obj.start;
  var end = obj.end;
  var obj_type = obj.obj_type;
  var obj_id = obj.obj_id || 0;
  var cin = start.format(g_moment_format);
  var cout = end.format(g_moment_format);

  var data = {"notes": '', "rate_day": '', "status": 0, "min_stay": 0, "changeover": 0, "last_name": '', "comments": '', "obj_type": obj_type, "obj_id": obj_id, "property_type": apt.type, "apt_id": apt.id, "apartment_name": apt.name, "checkin": cin, "checkout": cout};
  if(obj.id) data = jQuery.extend(data, obj); // edit
  console.log("fc_dialog data=", data);

  console.log("ok: type=" + obj_type + " id=" + obj_id + " data:", data);

  if(obj_type == 'reservation') {
    //edit_reservation($target, obj_type, obj_id)
    showMenuSingle($target, obj_type, obj_id);
    return false;
  }
  
  // show simple dialog  
  var frm = "calendar_simple_form";
  $("#" + frm + " .messages").remove();
  populate(frm, data);
  
  // init_date_range();
  if(cin && cout) {
    $("#simple_in").data('date', cin).val(cin);
    $("#simple_out").data('date', cout).val(cout);
    
    $('#simple_dialog_range input').each(function() {
      var $dp = $(this);
      var id =$dp.attr('id');
      $dp.datepicker("clearDates");
    });
    
    var $range = $("#simple_dialog_range");
    var $dp = $range.data("datepicker");
    $dp.pickers[0].update(cin);
    $dp.pickers[1].update(cout);    
    datepicker_set_duration($range);
      
  } else {
    console.log("fc_dialog: missing dates: in=" + data.checkin + " out=" + data.checkout);
  }

  var title = dialog_title(frm);
  $("#" + frm + " .overlay-title").html(title);
  $("#" + frm + " .btn-group button").removeClass("btn-success btn-danger").addClass('btn-default');
  $("#" + frm + " .day-rate").hide();
  
  
  if(obj_id) { // edit res exception
    $("#title_field").val(obj.description);
    $("#simple_rate").val(obj.rate_change ? obj.rate_change : obj.rate_day);
    console.log("Setting rate to " + obj.rate_change + " obj:", obj);
    
    console.log("setting tf  len=" + $("#title_field").length + " to " + obj.description); 
    $("#avail_buttons,#new_res_button").hide();
    $("#overlay_content_simple button.btn-available").trigger("click").click();
    $("#delete_exception").show();
    
  } else {
    $("#avail_buttons,#new_res_button").show();
    $("#" + frm + " .show-block").hide();
    $("#" + frm + " .show-free").hide();
    $("#delete_exception").hide();
  }
  show_simple_dialog(data);
}

function delete_exception() {
  var obj_type = $("#obj_type").val();
  var obj_id = $("#obj_id").val();
  var data = {"obj_id": obj_id, "obj_type": obj_type};
  console.log("Deleting data:",data);
  if(obj_type == 'res_exception' && obj_id > 0) {
    ajax_handler($("#delete_exception"), {"data": data});
    close_parent($("#delete_exception"));
    $("#rental-calendar-link").click();
  } 
}



// apply rate change
function fc_apply_rate_change(base_rate, rate_change) {
  if(rate_change.indexOf('%')) { // percentage change
    var factor = (100 + parseFloat(rate_change)) / 100;
    var new_rate = parseInt(Math.ceil(factor * base_rate));
  } else { // absolute change
    var new_rate = parseInt(Math.ceil(base_rate + rate_change));
  }
  return new_rate;
}

// used to set rates after event is loaded
// if render is set, update view
function fc_event_loaded($fullcalendar, event, render) {
  var apt = $fullcalendar.data('rental');
  var options = $fullcalendar.data('options') || {};
  var g_fc_dates = $fullcalendar.data('dates') || {};
  
  var editable = options.editable;
  var innerText = null;
  var start = event.start;
  var end = event.end;
  var cur = apt.cur;
  
  console.log("fc_event_loaded: render=" + render + " event=", event);
  // for weekly/monthly pricing, not need to loop
  if(event.obj_type == 'res_exception' && (event.rate_week || event.rate_month)) {
    if(((event.rate_week && !apt.rate_week) || (event.rate_month && !apt.rate_month))) return; // does not apply
    
    //console.log("start=", event.start);    
    var momdate = sql2date(event.start);
    var apt_copy = clone_object(apt);
    //console.log("start now =", event.start);    
    if(event.rate_week) {
      var weeks = $fullcalendar.data('weeks');
      var weeks2 = $fullcalendar.data('weeks2');
      var weeks3 = $fullcalendar.data('weeks3');
      var weeknum = momdate.isoWeek();
      var weekyear = momdate.isoWeekYear();
      var key = weeknum + '-' + weekyear;
      //console.log("Event: storing week rate: " + key + " = " + week_rate);
      //console.log("Event storing: ", event);
      //console.log("Date storing: ", momdate);
      
      apt_copy.rate_week = event.rate_week;
      var week_rate = fc_get_week_rate(apt_copy, momdate)
      weeks[key] = week_rate;
      $fullcalendar.data('weeks', weeks); // store in data
      
      apt_copy.rate_2_weeks = event.rate_2_weeks;
      var week2_rate = fc_get_week_rate(apt_copy, momdate, 2)
      weeks2[key] = week2_rate;
      $fullcalendar.data('weeks2', weeks2); // store in data

      apt_copy.rate_3_weeks = event.rate_3_weeks;
      var week3_rate = fc_get_week_rate(apt_copy, momdate, 3)
      weeks3[key] = week3_rate;
      $fullcalendar.data('weeks3', weeks3); // store in data
      
      fc_reset_week_rate($fullcalendar, apt); // redraw (all, lazy)       
    } else {
      var months = $fullcalendar.data('months');
      var month = momdate.month();
      var year = momdate.year();
      var key = month + "-" + year;
      apt_copy.rate_month = event.rate_month;
      var month_rate = fc_get_month_rate(apt_copy, momdate)
      
      months[key] = month_rate;
      //console.log("Event: storing month rate: " + key + " = " + month_rate);
      $fullcalendar.data('months', months); // store in data
      fc_reset_month_rate($fullcalendar, apt); // redraw (all, lazy)       
    }
    $fullcalendar.data('dates', g_fc_dates);  
    return;
  }

  for (var m = moment(start); m.isBefore(end); m.add(1, 'days')) {
    var date = m.format('YYYY-MM-DD') 
    var $day1 = render ? $("TD.fc-day[data-date='" + date + "']") : null;
    var weekday = m.day();
    if(weekday == 0) weekday = 7; // momentjs return 0 for Sunday. We use 7.
    var weekend = weekday == 5 || weekday == 6; // Friday or Saturday

    var base_rate = fc_get_day_rate(apt, weekend);
    //var base_rate = weekend && apt.rate_weekend > 0 && apt.rate_weekend != apt.rate_day ? apt.rate_weekend : apt.rate_day;

    var new_rate;
    //console.log("base rate for " + date + " is " + base_rate);
    if(event.obj_type == 'res_exception') {
      if(g_fc_dates[date] == 'blocked') {
        if(render) $day1.html('');
        return;
      }
      var rate_exception = event.rate_day > 0 || event.rate_weekend > 0 || event.rate_extraperson > 0 || event.rate_change != '';
      if(!rate_exception) {                                   
        //console.log("Exception does not affect rate, returning");
        return;
      }
      
      if(event.rate_change) { // apply percentage or absolute change to existing rate
        var rate_change = event.rate_change;
        new_rate = fc_apply_rate_change(base_rate, rate_change);
        //console.log("Applied rate_change: " + base_rate + " + " + rate_change + " = " + new_rate);
      } else {  // override existing rate
        var apt_copy = clone_object(apt);
        if(event.rate_day > 0) apt_copy.rate_day = event.rate_day;
        if(event.rate_weekend > 0) apt_copy.rate_weekend = event.rate_weekend;
        else if(event.rate_day > 0) apt_copy.rate_weekend = event.rate_day;
        if(event.rate_extraperson > 0) apt_copy.rate_extraperson = event.rate_extraperson;              

        if(event.rate_week > 0) apt_copy.rate_week = event.rate_week;              
        if(event.rate_2_weeks > 0) apt_copy.rate_2_weeks = event.rate_2_weeks;              
        if(event.rate_3_weeks > 0) apt_copy.rate_3_weeks = event.rate_3_weeks;              

        var weekday = m.day();
        var weekend = weekday == 5 || weekday == 6; // Friday or Saturday

        new_rate = fc_get_day_rate(apt_copy, weekend);
        //console.log("Applied new rate: was:" + base_rate + " now:" + new_rate);
      }
      
      var rate_class = editable ? 'fc-special-rate' : '';
      g_fc_dates[date] = new_rate;
      var rate_string = editable ? '<a href="#" class="edit-res-exception" data-id="' + event.id + '">' + cur + new_rate + "</a>" : cur + new_rate;
      innerText = '<span class="fc-rental-rate ' + rate_class + ' ">' + rate_string + '</span>';

      var co = event.changeover;
      var ms = event.min_stay;

      if(co && co == weekday) innerText = innerText + ' <i class="fa fa-refresh"></i>';        
      if(ms > 0) {
        msText = "+" + ms.toString() + "d ";
        innerText = innerText + ' <span class="fc-rental-rate red" style="" title="Minimum stay: '+ ms + ' days">' + msText + '</span>';
      }


      /** not working 
      if(0 && co && co == weekday) {
        var tooltip = '<a class="afr-tooltip" rel="tooltip" data-toggle="tooltip" title="If you leave thi" href="#"><i class="fa fa-question-circle"></i><span class="ztop custom info">'
        + '<i class="fa fa-info-circle fa-4x msgicon text-info"></i>'
        +'Checkin is only possible on this weekday</span></a>';
        tooltip = ''; 
      }    
      */
      
    } else if(event.obj_type == 'reservation') {
      if(event.status == 35) {
        g_fc_dates[date] = 'blocked';
        innerText = '';
      }
    }
    
    if(render && innerText !== null) {
      var existing_id = $day1.data('exception_id');
      var ar = existing_id ? existing_id.split(',') : [];
      if($.inArray(event.id, ar) == -1) ar.push(event.id);
      var event_list = ar.join(",");
      $day1.data('exception_id', event_list);
      $day1.html(innerText);
    }
  }

  // if area is selected, update average rate
  var g_fc_selection = $fullcalendar.data('selection');
  
  if(g_fc_selection) fc_update_average_rate($fullcalendar);
  $fullcalendar.data('dates', g_fc_dates);  
}

function fc_reset_month_rate($fullcalendar, apt) {
  var cur = apt.cur;
  var momdate = $fullcalendar.fullCalendar('getDate');
  var months = $fullcalendar.data('months');

  var month_rate = fc_get_month_rate(apt, momdate);
  var day = momdate.date();
  var month = momdate.month();
  var year = momdate.year();
 
  var format = "DD-MM-YYYY"; // human readable
  var firstday = year + "-" + month + "-1";
  var days_in_month = momdate.daysInMonth();
  //console.log("days in month=" + days_in_month);
  
  var key = month + '-' + year;
  var stored_rate = months[key];
  var this_month_rate,innerText;
  
  if(stored_rate) {
    //console.log('stored_rate for month ' + key + '=' + stored_rate);
    this_month_rate = stored_rate;
  } else {
    //console.log('reset_month: no stored_rate for month ' + key);
    //console.log("reset_month: storing month rate: " + key + " = " + month_rate);
    months[key] = this_month_rate = month_rate;
  }
  
  
  var momstart = moment([year, month, 1]);
  var momend = moment([year, month, 1]).add(days_in_month, 'days');
  var start = momstart.format(format);
  var end = momend.format(format);
  
  //console.log("month rate=" + month_rate);
  //console.log("day=" + day + " month=" + month + " year=" + year);
  //console.log("month: firstday=" + firstday);
  //console.log("month=" + start + "-" + end);
  
  var $center = $("#fullcalendar > div.fc-toolbar > div.fc-center");  
  var options = $fullcalendar.data('options');
  if(options.editable) {
    innerText = '<h3 class="fc-month-rate"><a href="#" data-rate="' + this_month_rate + '" data-start="' + start + '" data-end="' + end + '" data-month="' + month + '" data-year="' + year + '" class="fc-monthly-rate">' + cur + this_month_rate + '</a><span class="small">/month</span></h3>';
  } else {
    innerText = '<h3 class="fc-month-rate">' + cur + this_month_rate + '<span class="small">/month</span></h3>';
  }
  var debug = start + '-' + end;
  $center.html(innerText);
  $fullcalendar.data('months', months); // store in data

}

// reset price of all weekdays
function fc_reset_week_rate($fullcalendar, apt) {
  var cur = apt.cur;
  var momdate = $("#fullcalendar").fullCalendar('getDate');
  var day = momdate.date();
  var month = momdate.month();
  var year = momdate.year();

  var weeks = $fullcalendar.data('weeks');
  var weeks2 = $fullcalendar.data('weeks2');
  var weeks3 = $fullcalendar.data('weeks3');

  var week_rate = fc_get_week_rate(apt, momdate, 1);
  var week2_rate = fc_get_week_rate(apt, momdate, 2);
  var week3_rate = fc_get_week_rate(apt, momdate, 3);

  var innerText;
  
  //console.log("week rate=" + week_rate);
  //console.log("day=" + day + " month=" + month + " year=" + year);
  
  var $weeks = $("TD.fc-week-number SPAN");
  //console.log("Found " + $weeks.length + " weeks", $weeks);
  
  $.each($weeks, function(i, week) {
    var $week = $(week);
    var weeknum = $week.html();
    var weekyear = year;
    if(month == 0 && weeknum > 50) weekyear--; // last year
    else if(month ==11 && weeknum < 10) weekyear++; // next year

    var format = "DD-MM-YYYY"; // human readable
    var monday = weekyear + "-" + weeknum + "-1";
    var momstart = moment(monday, "YYYY-W-E");
    var momend = moment(monday, "YYYY-W-E").add(7, 'days');
    var start = momstart.format(format);
    var end = momend.format(format);
    var key = weeknum + '-' + weekyear;
    var this_week_rate,this_week2_rate,this_week3_rate;
    
    this_week_rate = weeks[key] ? weeks[key] : week_rate;
    this_week2_rate = weeks2[key] ? weeks2[key] : week2_rate;
    this_week3_rate = weeks3[key] ? weeks3[key] : week3_rate;
    
    //var stored_rate = weeks[key];    
    //if(stored_rate) {
    //  //console.log('stored_rate for week ' + key + '=' + stored_rate);
    //  this_week_rate = stored_rate;
    //} else {
    //  //console.log('reset_week: no stored_rate for week ' + key);
    //  //console.log("reset_week: storing week rate: " + key + " = " + week_rate);
    //  weeks[key] = this_week_rate = week_rate;
    //}
    
    var options = $fullcalendar.data('options');
    if(options.editable) {
      innerText = '<a href="#" title="Weekly rate" data-rate="' + this_week_rate + '" data-start="' + start + '" data-count="1" data-end="' + end + '" data-week="' + weeknum + '" data-year="' + weekyear + '" class="fc-weekly-rate">' + cur + this_week_rate + '</a> ';
      if(this_week2_rate > 0) innerText += '<br><a href="#" title="Rate for 2 weeks" data-rate="' + this_week2_rate + '" data-count="2" data-start="' + start + '" data-end="' + end + '" data-week="' + weeknum + '" data-year="' + weekyear + '" class="fc-weekly-rate">' + cur + this_week2_rate + '</a> ';
      if(this_week3_rate > 0) innerText += '<br><a href="#" title="Rate for 3 weeks" data-rate="' + this_week3_rate + '" data-count="3" data-start="' + start + '" data-end="' + end + '" data-week="' + weeknum + '" data-year="' + weekyear + '" class="fc-weekly-rate">' + cur + this_week3_rate + '</a>';
    } else {
      innerText = cur + this_week_rate;
      if(this_week2_rate > 0) innerText += '<br>' + cur + this_week2_rate;
      if(this_week3_rate > 0) innerText += '<br>' + cur + this_week3_rate;
    }
    
    var $rates = $week.parent().find("DIV.fc-rental-rate");
    if($rates.length) { // update
      //console.log("reset_week rate: update");
      $rates.html(innerText);
    } else {
      var debug = '';
      $week.after('<div class="fc-rental-rate fc-week-rate">' + innerText +'</div>');
      $('#fullcalendar').fullCalendar('option', 'height', $('#fullcalendar').height()); // trigger resize so weeks will fit in column    
    }
    $week.hide(); // don't show the confusing ISO week number    
    $fullcalendar.data('weeks', weeks); // store in data
    $fullcalendar.data('weeks2', weeks2); // store in data
    $fullcalendar.data('weeks3', weeks3); // store in data

  });
  
  
  
}

/** Below are used only for single-calendar (fullcalendar) */

$(document).on("click", "A.fc-weekly-rate, A.fc-monthly-rate", function(evt) {
  var $fullcalendar = $("#fullcalendar");
  var $link = $(this);          
  var apt = $fullcalendar.data("rental");
  var count = $link.data('count');
  var rate = $link.data('rate');
  var title = '';
  

  var data = {"apt_id": apt.id, "host_id": apt.host_id, "property_type": apt.type, "start": $link.data('start'), "end": $link.data('end')};
  if($link.hasClass('fc-weekly-rate')) {
    //console.log("show_week_dialog data=", data);
    fc_show_weekly_dialog(data, rate, count);
  } else {
    //console.log("show_month_dialog data=", data);
    fc_show_monthly_dialog(data, rate);
  }
});


// new weekly rate dialog using Bootstrap modal
function fc_show_weekly_dialog(data, rate, count) {
  var options = {};
  var modal = "#overlay_content_week"; // '#calModal'
  var frm = "calendar_week_form";
  $("#" + frm + " .messages").remove();
  if(count == 3) {
    title = "Rate for 3 weeks:";  
  } else if(count == 2) {
    title = "Rate for 2 weeks:";  
  } else {
    title = "Weekly rate:";  
  }
  
  var range = data.start + ' - ' + data.end; 
  
  $("#week_count").val(count);
  $("#week_rate").val(rate);
  
  $("#overlay_content_week .overlay-title").html(title);
  $("#overlay_content_week .overlay-title-range").html(range);
  populate(frm, data);
  $(modal).modal(options);
}

// new monthly rate dialog using Bootstrap modal
function fc_show_monthly_dialog(data, rate) {
  var options = {};
  var modal = "#overlay_content_month"; // '#calModal'
  var frm = "calendar_month_form";
  var momdate = sql2date(data.start);
  var monthNames = moment.months();
  var month_name = monthNames[momdate.month()];
  $("#_rate").val(rate);
  $("#" + frm + " .messages").remove();
  $("#overlay_content_month .overlay-title-range").html(month_name);
  populate(frm, data);
  $(modal).modal(options);
}

// reset price of all weekdays                                                                                                                                                     
function fc_reset_day_rate(apt) {
  var cur = apt.cur;
  var day_rate = fc_get_day_rate(apt, false);
  var weekend_rate = fc_get_day_rate(apt, true);
  
  $("TD.fc-day").html('<span class="fc-rental-rate">' + cur + day_rate + '</span>');
  if(weekend_rate != day_rate) {
    //console.log("weekend rate > 0 and != day rate:" + apt.rate_weekend + " day=" + apt.rate_day);
    $("TD.fc-day.fc-fri").html('<span class="fc-rental-rate fc-weekend-rate">' + cur + weekend_rate + '</span>');
    $("TD.fc-day.fc-sat").html('<span class="fc-rental-rate fc-weekend-rate">' + cur + weekend_rate + '</span>');
  }
}

function fc_render_weeks($fullcalendar) {
  var apt = $fullcalendar.data('rental') || {};
  var cur = apt.cur;
  $weeks = $('TD.fc-week-number SPAN');
  //console.log("Rendering " + $weeks.length + " weeks");
  $.each($weeks, function(i, week) {
    var $week = $(week);
    //console.log("week=", $week.html());
    
  });
}

// returns the additional daily rate per day for give number of guests
function fc_rate_extra_guests(apt, ng) {
  var rate_extraperson = parseInt(apt.rate_extraperson || 0);
  var rate_num_guests = parseInt(apt.rate_num_guests || 2);

  // if num_guests
  var ng = apt.ng || 1;
  var extra_guests = Math.max(ng - rate_num_guests, 0);
  if(extra_guests > 0 && rate_extraperson > 0) { // add fee for extra guests
    return rate_extraperson * extra_guests; 
  }
  return 0;
}

// get the default weekly rate for given number of guests (date is ignored now)
function fc_get_week_rate(apt, momdate, weeks) {
  if(typeof weeks == "undefined") var weeks = 1;
  
  var rate_week = 0;
  var days;
  if(weeks == 1) {
    rate_week = parseInt(apt.rate_week);
    days = 7;
  } else if(weeks==2 && apt.rate_2_weeks > 0) {
    rate_week = parseInt(apt.rate_2_weeks);
    days = 14;
  } else if(weeks==3 && apt.rate_3_weeks > 0) {
    rate_week = parseInt(apt.rate_3_weeks);
    days = 21;
  }
  
  if(!rate_week) return 0;
  
  var ng = apt.ng || 1;
  var extra_guest_rate = fc_rate_extra_guests(apt, ng);
  rate_week = rate_week + days * extra_guest_rate; 
  return rate_week;  
}

// get the default monthly rate for given number of guests (date is used only for number of days in month)
function fc_get_month_rate(apt, momdate) {
  var rate_month = parseInt(apt.rate_month);
  if(!rate_month) return 0;
  
  var days_in_month = momdate.daysInMonth();
  //console.log("days in month=" + days_in_month);
  var ng = apt.ng || 1;
  var extra_guest_rate = fc_rate_extra_guests(apt, ng);
  rate_month = rate_month + days_in_month * extra_guest_rate; 
  return rate_month;  
}

// full calendar helper functions
function fc_get_day_rate(apt, weekend) {

  var ng = apt.ng || 1;
  var extra_guest_rate = fc_rate_extra_guests(apt, ng);

  var rate_day = parseInt(apt.rate_day);
  var rate_weekend = parseInt(apt.rate_weekend);

  if(weekend && apt.rate_weekend > 0 && apt.rate_weekend != apt.rate_day) return rate_weekend + extra_guest_rate;
  return rate_day + extra_guest_rate;
  



  /**
  rate_day += extra_guest_rate; 
  rate_weekend += extra_guest_rate; 
  var rate_extraperson = parseInt(apt.rate_extraperson || 0);
  var rate_num_guests = parseInt(apt.rate_num_guests || 2);
  // if num_guests
  var ng = apt.ng || 1;
  var extra_guests = Math.max(ng - rate_num_guests, 0);
  if(extra_guests > 0 && rate_extraperson > 0) { // add fee for extra guests
    rate_day = rate_day + rate_extraperson * extra_guests; 
    rate_weekend = rate_weekend + rate_extraperson * extra_guests; 
  }
  //console.log("Extra guests not included in price=" + extra_guests);
  
  // if dates
  var num_days = apt.nd;
  if(weekend && apt.rate_weekend > 0 && apt.rate_weekend != apt.rate_day) return rate_weekend;
  return rate_day;
  */
}

function fc_revert(apt, revertFunc) {
  fc_reset_day_rate(apt)
  revertFunc();
}

// Updates average rate after selection is made
function fc_update_average_rate($fullcalendar) {  
  var apt = $fullcalendar.data('rental') || {};
  if(!apt) return;
  
  var selection = $fullcalendar.data('selection') || {};
  var start = selection.start;
  var end = selection.end;
  var num_days = selection.days;
  var average, unit;
  var cur = apt.cur;
  var discount = 0;
  
  // apt.rate_month = apt.rate_week = 0; /** turn off for now */   
  if(!num_days) {
    average = apt.rate_average || apt.rate_day;
    unit = apt.rate_unit || 'night';
  } else if(num_days >= 28 && apt.rate_month > 0) {
    unit = 'month';
    average = fc_average_rate($fullcalendar, start, end, unit);
  } else if(num_days >= 21 && apt.rate_3_weeks > 0) {
    unit = 'week';
    average = fc_average_rate($fullcalendar, start, end, 'week3');
    average = parseInt(average / 3);    
  } else if(num_days >= 14 && apt.rate_2_weeks > 0) {
    unit = 'week';
    average = fc_average_rate($fullcalendar, start, end, 'week2');
    average = parseInt(average / 2);    
  } else if(num_days >= 7 && apt.rate_week > 0) {
    unit = 'week';
    average = fc_average_rate($fullcalendar, start, end, unit);
  } else {
    unit = 'night';
    average = fc_average_rate($fullcalendar, start, end, unit);
    if(num_days >= 28 && apt.discount_month) discount = apt.discount_month; 
    if(num_days >= 21 && apt.discount_2_weeks) discount = apt.discount_2_weeks; 
    if(num_days >= 14 && apt.discount_3_weeks) discount = apt.discount_3_weeks; 
    else if(num_days >= 7 && apt.discount_week) discount = apt.discount_week; 
    
  }
  var avg = Math.round(average);
  console.log('no selection, using num_days=' + num_days + ' avg=' + avg + ' rate_2_weeks=' + apt.rate_2_weeks + " unit=" + unit); 
  
  $("P.booking-item-header-price SPAN.rate").html(avg);
  $("P.booking-item-header-price SPAN.unit").html(unit);

  
  if(discount > 0) {
    var discount_rate = Math.round(avg * (100 - discount)/100);
    //console.log('discount = ' + discount);
    $("P.booking-item-header-discount SPAN.discount").html(discount);
    $("P.booking-item-header-discount-rate SPAN.rate").html(discount_rate);
    $("P.booking-item-header-discount, P.booking-item-header-discount-rate").removeClass("hidden").show('fast');
  } else {
    $("P.booking-item-header-discount-rate SPAN.rate, P.booking-item-header-discount SPAN.discount").html('')
    $("P.booking-item-header-discount, P.booking-item-header-discount-rate").hide();
  }
}

// loop through dates to figure out average rate per day
function fc_average_rate($fullcalendar, start, end, unit) {
  var average;
  var i = 0;
  var total = 0;
  var weeknum,weekyear,month,year,key,unit_val,unit_rate;
  var dates = $fullcalendar.data('dates');
  var weeks = $fullcalendar.data('weeks');
  var weeks2 = $fullcalendar.data('weeks2');
  var weeks3 = $fullcalendar.data('weeks3');
  var months = $fullcalendar.data('months');
  console.log("weeks", weeks);
  console.log("weeks2", weeks2);
  console.log("weeks3", weeks3);
  var apt = $fullcalendar.data('rental') || {};

  for (var m = moment(start); m.isBefore(end); m.add(1, 'days')) {
    var date = m.format('YYYY-MM-DD') 
    var date_val = dates[date];
    
    if(date_val == 'blocked') {
      //console.log("average rate:" + date + " is not available");
      //alert(date + " is not available");
      return 0;
    }

    if(unit == 'week') {
      weeknum = m.isoWeek();
      weekyear = m.isoWeekYear();
      key = weeknum + "-" + weekyear;
      unit_val = weeks[key] ? weeks[key] : apt.rate_week;
      unit_rate = parseInt(unit_val);
    } else if(unit == 'week2') {
      weeknum = m.isoWeek();
      weekyear = m.isoWeekYear();
      key = weeknum + "-" + weekyear;
      unit_val = weeks2[key] ? weeks2[key] : apt.rate_2_weeks;
      unit_rate = parseInt(unit_val);
    } else if(unit == 'week3') {
      weeknum = m.isoWeek();
      weekyear = m.isoWeekYear();
      key = weeknum + "-" + weekyear;
      unit_val = weeks3[key] ? weeks3[key] : apt.rate_3_weeks;
      unit_rate = parseInt(unit_val);
    } else  if(unit == 'month') {
      month = m.month();
      year = m.year();
      key = month + "-" + year;
      unit_val = months[key] ? months[key] : apt.rate_month;
      unit_rate = parseInt(unit_val);
      console.log("month rate for " + key + "=" + unit_rate);
    } else { // day
      unit_val = date_val;
      unit_rate = parseInt(unit_val);
    }
    
    console.log(unit + " rate for " + date + " " + unit_val + " = " + unit_rate);
    if(unit_rate > 0) {
      total += unit_rate;
    } else {
      //console.log(dates);
      //alert(date + " has no rate");
      //console.log("average rate:" + key + " has no rate for unit=" + unit);
      return 0;
    }
    i++;
  }
  average = total / i;
  console.log("total rate for " + i + " units = " + average);

  
  //console.log('total = ' + total + ' average for ' + i + "=" + average);
  
  return average;  
}


function init_tooltip() {
  //console.log("init tooltip()");
  $('.tooltip-ajax').tooltip({
    content:function(callback) { //callback
      var url = $(this).data("url");
      $.get(url,{}, function(data) {
        callback(data); //call the callback function to return the value
      });
    },
  });
  //console.log("init_tooltip(): jquery len=" + $('.tooltip-jq').length);
  $('.tooltip-jq').tooltip({
    content: function( event, ui ) {
      var tip = $(this).data("tip");
      if(tip) return tip;
    }
  });
}

function init_uploader($target) {
  $target.data('initialized', true);
  var options = $target.data('options') || {};
  var handler_options = $target.data('handler_options') || options ;
  
  var disabled = $target.hasClass('disabled') ? true : false;
  var div_id = $target.prop('id');
  //  alert("id=" + div_id);

  // defaults
  var extensions = options.allowed_extensions || ['jpeg', 'jpg', 'gif', 'png'];
  var endpoint = options.endpoint || '/ajax.php?oper=upload';
  var allow_multiple = options.allow_multiple || false;
  var title = options.title || 'Click or Drop';
  var sizeLimit = options.maximum_file_size || 10 * 1024 * 1024; // 10M default

  var mediaData = options.media_data || {};
  var resize = options.resize || {};
  var handler = options.handler;
  var target_field = options.target_field;
  
  handler_options.handler = handler; // needed for success_handler
  
  var thumb_dir = '';
  var thumbAr = [];
  if(thumbAr = resize.thumb) thumb_dir = thumbAr[1];
  
  //console.log("init_uploader for " + div_id); // + " data=" + dump(mediaData) + " options=" + dump(options));
  //console.log(options);
  //console.log("extension=");//console.log(extensions);
  delete options.media_data;
  
  if(!mediaData.parent_id) {
    var parent_id = $target.data('parent_id');
    //console.log("No parent ID, looking at element, found:" + parent_id);
    mediaData.parent_id = parent_id;
  }
  //alert("data" + dump(mediaData));
  //alert("options" + dump(options));
  var params = {
      options: JSON.stringify(options),
      mediaData: JSON.stringify(mediaData)
    };
    
  if(params) endpoint += '&' + obj2qs(params); // this shouln't be needed as params below should handle this. Doesn't work for some reason      
  

  $target.fineUploader({
    request: {
      endpoint: endpoint
    },
    multiple: allow_multiple,
    validation: {
      allowedExtensions: extensions,
      sizeLimit: sizeLimit 
    },
    text: {
      uploadButton: title
    },
    params: params,
    paramsInBody: false,
    showMessage: function(message) { // Using Bootstrap's classes
      //alert('message for ' + div_id);
      $('#' + div_id).append('<div class="relative error_box"><div class="error">Message:' + message + '<span class="close_icon close_parent"></span></div></div>');
    },
    failedUploadTextDisplay: {
      mode: 'custom',
      //maxChars: 40,
      responseProperty: 'error',
      enableTooltip: true
    }      
  }).on('complete', function(event, id, fileName, responseJSON) {
    //alert("complete name=" + fileName + dump(responseJSON));
    if (responseJSON.success) {
      var src = "";
      var path = responseJSON.upload_path;
      var filename = responseJSON.upload_name;
      var data = responseJSON.data;
      var media_id = data && responseJSON.data.id ? responseJSON.data.id : 0; // either media id or else the path to the uploaded file
      
      if(path && filename) src = path + filename;
      switch(responseJSON.type) {
        case 'image':
          if(path && filename) src = path + (thumb_dir ? thumb_dir + '/' : '') + filename;
          break;
        default:            
          break;
      }        
      
      // update the data field (when used in a form)
      //var target = options.target;
      var value = $(this).data('value_list') || ''; // list of media IDs or paths/urls
      //alert("hurray id=" + media_id + " type=" + type + " val=" + value);
      //var val = value || $input.val(); // existing target value
      //var $input = $('#' + target);

      var result_val = media_id; // || src; // either media id or else the path to the uploaded file

      if(result_val && allow_multiple==1) { // append          
        if(value) value = value + "," + result_val;
        else value = result_val; 
      } else { // overwrite
        value = result_val;
      }

      //if($input.length) $input.val(result_val);

      $(this).data('value_list', value);

      var $file_list = $('#' + div_id + '_file-list');        

      if(!media_id) {
        alert("no media_id for '" + src + "'" + dump(responseJSON));
      } else {
        //alert("mid=" + media_id + ' fl=' + $file_list.length);
      }


      if(media_id)
        if($file_list.length) {
          var ajax_url = '/ajax.php?oper=load-function&function=file_list&param1=' + encodeURIComponent(value);
          //alert(ajax_url);
          $file_list.load(ajax_url, function(responseText) {   
          });                    
        } 
      }
      if(target_field) $('#' + target_field).val(media_id || src); // write result to input field
        
      success_handler(handler_options, data);      
      //if(handler) {
      //  //if(!is_object(result_data)) result_data = parse_json(result_data); 
      //  //if(handler_data) jQuery.extend(result_data, data);
      //  var funcCall = handler  + "(handler_options, data);";
      //  //alert('func:' + funcCall);
      //  eval(funcCall);
      //}
      
  }).on('error', function(event, id, name, reason) {
    //alert('error for ' + div_id + " name:" + name + " reason:" + reason  + " id:" + id);
    //if(reason) $(this).append(reason); //'<div class="relative error_box1"><div class="error">Message:' + reason + '<span class="close_icon close_parent"></span></div></div>').show();
      if(reason) $(this).append('<div class="relative error_box1 alert alert-danger"><div class="error">' + reason + '<span class="close_icon close_parent"></span></div></div>').show();
  });    
}

  
// used by xss
function requestStylesheet(stylesheet_url) {
  stylesheet = document.createElement("link");
  stylesheet.rel = "stylesheet";
  stylesheet.type = "text/css";
  stylesheet.href = stylesheet_url;
  stylesheet.media = "all";
  document.getElementsByTagName("head")[0].appendChild(stylesheet);
  //document.lastChild.firstChild.appendChild(stylesheet);
}

function requestScript(script_url) {
  var script = document.createElement('script');
  script.setAttribute('src', script_url);
  script.setAttribute('type', 'text/javascript');
  document.getElementsByTagName('head')[0].appendChild(script);
}

function fancybox_loader() {
  //if(!$(".fancybox").length) {
  //  //console.log("fancybox loader - none found");
  //  return;
  //}
  $fb = $(".fancybox");
  if(!$fb.length) return;
  
  $(".fancybox").fancybox({
    beforeShow: function () {
      $.fancybox.wrap.on("contextmenu", function (e) { /* Disable right click */
        return false; 
      });
      var $img = this.element.find('img');
      var alt = $img.attr('alt');
      this.inner.find('img').attr('alt', alt);
      this.title = alt;
      var info = $img.data('info-link');
      var target_id = $img.data('info-target') || '';
      if(info) {
        log("loader target=" + target_id);
        var button_html = "<a title='Show info' href='#' class='info-link new-tab' data-target='" + target_id + "' data-data='" + JSON.stringify(info) + "'><div class='mini-sprite-32 s212 abs_bottom_left_outside'></div></a>";
        $(button_html).appendTo($('.fancybox-outer'));
        //log(info);
      }

    }
  });

  //log("fancybox loader found:" + $(".fancybox").length);

  /*
   *  Different effects
   */

  $(".fancybox-noscroll").fancybox({    
    helpers: {
      overlay: {
        locked: false
      }
    },  
    type: 'ajax',
    closeClick: false, // prevents closing when clicking INSIDE fancybox 
    openEffect: 'none',
    closeEffect: 'none',
    minWidth: 600,
    minHeight: 600,    
  });

  $(".fancybox-ajax").fancybox({
    type: 'ajax',
    closeClick: false, // prevents closing when clicking INSIDE fancybox 
    openEffect: 'none',
    closeEffect: 'none',
    minWidth: 600,
    minHeight: 600,    
  });
   
  // Change title type, overlay closing speed
  $(".fancybox-effects-a").fancybox({
    helpers: {
      title : {
        type : 'outside'
      },
      overlay : {
        speedOut : 0
      }
    }
  });

  // Disable opening and closing animations, change title type
  $(".fancybox-effects-b").fancybox({
    openEffect  : 'none',
    closeEffect : 'none',

    helpers : {
      title : {
        type : 'over'
      }
    }
  });

  // Set custom style, close if clicked, change title type and overlay color
  $(".fancybox-effects-c").fancybox({
    wrapCSS    : 'fancybox-custom',
    closeClick : true,

    openEffect : 'none',

    helpers : {
      title : {
        type : 'inside'
      },
      overlay : {
        css : {
          'background' : 'rgba(238,238,238,0.85)'
        }
      }
    }
  });

  // Remove padding, set opening and closing animations, close if clicked and disable overlay
  $(".fancybox-effects-d").fancybox({
    padding: 0,

    openEffect : 'elastic',
    openSpeed  : 150,

    closeEffect : 'elastic',
    closeSpeed  : 150,

    closeClick : true,

    helpers : {
      overlay : null
    }
  });

  /*
   *  Button helper. Disable animations, hide close button, change title type and content
   */

  $('.fancybox-buttons').fancybox({
    openEffect  : 'none',
    closeEffect : 'none',

    prevEffect : 'none',
    nextEffect : 'none',

    closeBtn  : false,

    helpers : {
      title : {
        type : 'inside'
      },
      buttons : {}
    },

    afterLoad : function() {
      this.title = 'Image ' + (this.index + 1) + ' of ' + this.group.length + (this.title ? ' - ' + this.title : '');
    }
  });


  /*
   *  Thumbnail helper. Disable animations, hide close button, arrows and slide to next gallery item if clicked
   */

  $('.fancybox-thumbs').fancybox({
    prevEffect : 'none',
    nextEffect : 'none',

    closeBtn  : false,
    arrows    : false,
    nextClick : true,

    helpers : {
      thumbs : {
        width  : 50,
        height : 50
      }
    }
  });

  /*
   *  Media helper. Group items, disable animations, hide arrows, enable media and button helpers.
  */
  $('.fancybox-media')
    .prop('rel', 'media-gallery')
    .fancybox({
      openEffect : 'none',
      closeEffect : 'none',
      prevEffect : 'none',
      nextEffect : 'none',

      arrows : false,
      helpers : {
        media : {},
        buttons : {}
      }
    });

  /*
   *  Open manually
   */

  $("#fancybox-manual-a").click(function() {
    $.fancybox.open('1_b.jpg');
  });

  $("#fancybox-manual-b").click(function() {
    $.fancybox.open({
      href : 'iframe.html',
      type : 'iframe',
      padding : 5
    });
  });

  $("#fancybox-manual-c").click(function() {
    $.fancybox.open([
      {
        href : '1_b.jpg',
        title : 'My title'
      }, {
        href : '2_b.jpg',
        title : '2nd title'
      }, {
        href : '3_b.jpg'
      }
    ], {
      helpers : {
        thumbs : {
          width: 75,
          height: 50
        }
      }
    });
  });
}

/*
 * ******************************************************************************
 *  jquery.mb.components
 *  file: jquery.mb.browser.js
 *
 *  Copyright (c) 2001-2013. Matteo Bicocchi (Pupunzi);
 *  Open lab srl, Firenze - Italy
 *  email: matteo@open-lab.com
 *  site:   http://pupunzi.com
 *  blog: http://pupunzi.open-lab.com
 *  http://open-lab.com
 *
 *  Licences: MIT, GPL
 *  http://www.opensource.org/licenses/mit-license.php
 *  http://www.gnu.org/licenses/gpl.html
 *
 *  last modified: 17/01/13 0.12
 *  *****************************************************************************
 */

/*******************************************************************************
 *
 * jquery.mb.browser
 * Author: pupunzi
 * Creation date: 16/01/13
 *
 ******************************************************************************/
/*Browser detection patch*/

(function($){

  var jQversion = jQuery.fn.jquery.split(".");
  if(jQversion[1]<8)
    return;

  jQuery.browser = {};
  jQuery.browser.mozilla = false;
  jQuery.browser.webkit = false;
  jQuery.browser.opera = false;
  jQuery.browser.msie = false;

  var nAgt = navigator.userAgent;
  jQuery.browser.name  = navigator.appName;
  jQuery.browser.fullVersion  = ''+parseFloat(navigator.appVersion);
  jQuery.browser.majorVersion = parseInt(navigator.appVersion,10);
  var nameOffset,verOffset,ix;

// In Opera, the true version is after "Opera" or after "Version"
  if ((verOffset=nAgt.indexOf("Opera"))!=-1) {
    jQuery.browser.opera = true;
    jQuery.browser.name = "Opera";
    jQuery.browser.fullVersion = nAgt.substring(verOffset+6);
    if ((verOffset=nAgt.indexOf("Version"))!=-1)
      jQuery.browser.fullVersion = nAgt.substring(verOffset+8);
  }
// In MSIE, the true version is after "MSIE" in userAgent
  else if ((verOffset=nAgt.indexOf("MSIE"))!=-1) {
    jQuery.browser.msie = true;
    jQuery.browser.name = "Microsoft Internet Explorer";
    jQuery.browser.fullVersion = nAgt.substring(verOffset+5);
  }
// In Chrome, the true version is after "Chrome"
  else if ((verOffset=nAgt.indexOf("Chrome"))!=-1) {
    jQuery.browser.webkit = true;
    jQuery.browser.name = "Chrome";
    jQuery.browser.fullVersion = nAgt.substring(verOffset+7);
  }
// In Safari, the true version is after "Safari" or after "Version"
  else if ((verOffset=nAgt.indexOf("Safari"))!=-1) {
    jQuery.browser.webkit = true;
    jQuery.browser.name = "Safari";
    jQuery.browser.fullVersion = nAgt.substring(verOffset+7);
    if ((verOffset=nAgt.indexOf("Version"))!=-1)
      jQuery.browser.fullVersion = nAgt.substring(verOffset+8);
  }
// In Firefox, the true version is after "Firefox"
  else if ((verOffset=nAgt.indexOf("Firefox"))!=-1) {
    jQuery.browser.mozilla = true;
    jQuery.browser.name = "Firefox";
    jQuery.browser.fullVersion = nAgt.substring(verOffset+8);
  }
// In most other browsers, "name/version" is at the end of userAgent
  else if ( (nameOffset=nAgt.lastIndexOf(' ')+1) <
      (verOffset=nAgt.lastIndexOf('/')) )
  {
    jQuery.browser.name = nAgt.substring(nameOffset,verOffset);
    jQuery.browser.fullVersion = nAgt.substring(verOffset+1);
    if (jQuery.browser.name.toLowerCase()==jQuery.browser.name.toUpperCase()) {
      jQuery.browser.name = navigator.appName;
    }
  }
// trim the fullVersion string at semicolon/space if present
  if ((ix=jQuery.browser.fullVersion.indexOf(";"))!=-1)
    jQuery.browser.fullVersion=jQuery.browser.fullVersion.substring(0,ix);
  if ((ix=jQuery.browser.fullVersion.indexOf(" "))!=-1)
    jQuery.browser.fullVersion=jQuery.browser.fullVersion.substring(0,ix);

  jQuery.browser.majorVersion = parseInt(''+jQuery.browser.fullVersion,10);
  if (isNaN(jQuery.browser.majorVersion)) {
    jQuery.browser.fullVersion  = ''+parseFloat(navigator.appVersion);
    jQuery.browser.majorVersion = parseInt(navigator.appVersion,10);
  }
  jQuery.browser.version = jQuery.browser.majorVersion;
})(jQuery)

function array2tr(ar) {
  var row = '';
  
  $.each(ar, function(key, val) {
    row = row + '<td>' + val + '</td>';
  });
  return '<tr>' + row + '</tr>';
}


function build_cart(cart) {
  var items_html = '';

  // divs
  var $cart = $('#floating_cart');
  var $cart_count = $('#cart_icon .cart_count');
  if(!$cart.length) return false;
  var $items = $('#cart_items');
  var $total = $('#cart .cart_total, #floating_cart .cart_total');

  if(typeof cart == "undefined") var cart = {};
  
  // data
  var total = cart.total || 0 ;
  var items = cart.items || [];
  var item_count = items.length || 0;
  var currency = cart.currency || 'EUR';
  
  if(!cart) {
    items_html = 'Basket is empty';
  } else if(items_html = cart.items_html) { // drawn in PHP
    $items.html(items_html);
  } else { // draw in JS (todo: sync with PHP)
    $.each(items, function(i, item) {
      var photo = item.photo ? "<img src='" + item.photo + "'>" : 'X';
      var name = item.name;
      var price = item.total;
      items_html += array2tr([photo, name, price]);
    });
    items_html = '<table>' + items_html + '</table>';
  }

  // output  
  if($items.length) $items.html(items_html);  
  if($total.length) $total.html(total);  
  if($cart_count.length) $cart_count.html(cart.count);  
  
  return true;
}

// hide these containers when click outside
$(document).on("mouseup", function (e) {
  var $containers = $(".click_hide");
  // window.console && console.log("mouseup found " + $containers.length);
  if ($containers.length && $containers.has(e.target).length === 0) $containers.hide();
});

// for now used for form elements (radio buttons), so better change than click in case of keyboard nav.
$(document).on("change", ".set-target-value", function() {
  var $target = $(this);  
  var target = $target.data("target");
  var value = $target.data("value") || '';
  
  // window.console && console.log("set-target-value target=" + target + " value=" + value + " len=" + $(target).length);
  if($(target).length) {
    $(target).val(value);
  }
});

$(document).on("click", "A.expandable-trigger-more", function() {
  var $target = $(this);
  console.log("expandable v2 target=", $target);
  var $expandable = $target.hasClass('expandable') ? $target : $target.closest('.expandable');
  var $content = $expandable.find(".expandable-content");
  //console.log("expandable expandable len=" + $expandable.length + " class=" + $expandable.attr("class") + " content len=" + $content.length);
  $expandable.addClass('expanded');
  return false;
  //return false;
});


$(document).on("click", "#cart_icon", function() {
  // window.console && console.log("enter cart.");
  $('#floating_cart').removeClass('hidden').show('fast');
  //$('#floating_cart').trigger('mouseenter');
});

// show on enter, clear timeout
$(document).on("mouseenter", ".show_over", function() {
  $target = $(this);  
  // window.console && console.log("enter show_over. id=" + $target.attr('id'));
  clearTimeout($target.data('timeoutId'));
  $target.removeClass('hidden').show('fast');
});

// hide after 3 seconds if mouse leaves
$(document).on("mouseleave", ".show_over", function() {
  $target = $(this);
  // window.console && console.log("leave show_over. id=" + $target.attr('id'));
  var timeoutId = setTimeout(function () {
    $target.hide('fast');
  }, 400);
  $target.data('timeoutId', timeoutId);
});


// handler for downloading image
$(document).on("click", ".download_media", function() {
  $target = $(this);
  var data = $target.data('data') || {};
  var operation = 'download-media';
  var ajax_url = "/ajax.php?oper=" + operation + "&" + obj2qs(data);

  //var $messages = $('#ajax_messages');
  //var $errors = $('#ajax_errors');
  //var $messages = form_messages(form_id); // get message div      
  
  jQuery.ajax({
    type: "get",
      url: ajax_url,
      success: function (responseText) {
        var result = parse_json(responseText);
        var error = result.error;
        var message = result.message;
        var success = result.success;
        var data = result.data || {};
        var filename = data.filename;
        var path = data.path;
        if(error) { 
          alert(error);
          
        } else if(success && path && filename) {
          // plain redirect
          var download_link = "/admin/download.php?" + obj2qs(data);
          //alert(download_link);
          //return false;

          window.location.href = download_link;

          // hidden iframe         
        }
        
        //alert("download result filename=" + filename);
      }
  });
  return false;
});


$(document).on("click", "[data-update-map]", function(e) {
  var map = $(this).data("update-map");
  var lat = $(this).data("lat");
  var lng = $(this).data("lng");
  var title = $(this).data("title") || '';
  var address = $(this).data("address") || '';
  //console.log("Update map " + map + " " + lat + "/" + lng);
  gmap_update(map, lat, lng, title, address);
  return false;
  //e.preventDefault();
});


// new function to show modal map using bootstrap3
function gmap_update(map_id, lat, lng, title, address) {
  var map;
  myLatlng = new google.maps.LatLng(lat,lng);
  var mapOptions = {
    center: myLatlng,
    zoom: 14,
    mapTypeControl: false,
    center:myLatlng,
    panControl:false,
    rotateControl:false,
    streetViewControl: false,
    mapTypeId: google.maps.MapTypeId.ROADMAP
  };
  map = new google.maps.Map(document.getElementById(map_id), mapOptions);

  var contentString = '<div id="mapInfo"><p><strong style="color:#000">' + address + '</strong></p></div>';
  var infowindow = new google.maps.InfoWindow({
    content: contentString
  });
  
  var marker = new google.maps.Marker({
    icon: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACcAAAArCAYAAAD7YZFOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAKT2lDQ1BQaG90b3Nob3AgSUNDIHByb2ZpbGUAAHjanVNnVFPpFj333vRCS4iAlEtvUhUIIFJCi4AUkSYqIQkQSoghodkVUcERRUUEG8igiAOOjoCMFVEsDIoK2AfkIaKOg6OIisr74Xuja9a89+bN/rXXPues852zzwfACAyWSDNRNYAMqUIeEeCDx8TG4eQuQIEKJHAAEAizZCFz/SMBAPh+PDwrIsAHvgABeNMLCADATZvAMByH/w/qQplcAYCEAcB0kThLCIAUAEB6jkKmAEBGAYCdmCZTAKAEAGDLY2LjAFAtAGAnf+bTAICd+Jl7AQBblCEVAaCRACATZYhEAGg7AKzPVopFAFgwABRmS8Q5ANgtADBJV2ZIALC3AMDOEAuyAAgMADBRiIUpAAR7AGDIIyN4AISZABRG8lc88SuuEOcqAAB4mbI8uSQ5RYFbCC1xB1dXLh4ozkkXKxQ2YQJhmkAuwnmZGTKBNA/g88wAAKCRFRHgg/P9eM4Ors7ONo62Dl8t6r8G/yJiYuP+5c+rcEAAAOF0ftH+LC+zGoA7BoBt/qIl7gRoXgugdfeLZrIPQLUAoOnaV/Nw+H48PEWhkLnZ2eXk5NhKxEJbYcpXff5nwl/AV/1s+X48/Pf14L7iJIEyXYFHBPjgwsz0TKUcz5IJhGLc5o9H/LcL//wd0yLESWK5WCoU41EScY5EmozzMqUiiUKSKcUl0v9k4t8s+wM+3zUAsGo+AXuRLahdYwP2SycQWHTA4vcAAPK7b8HUKAgDgGiD4c93/+8//UegJQCAZkmScQAAXkQkLlTKsz/HCAAARKCBKrBBG/TBGCzABhzBBdzBC/xgNoRCJMTCQhBCCmSAHHJgKayCQiiGzbAdKmAv1EAdNMBRaIaTcA4uwlW4Dj1wD/phCJ7BKLyBCQRByAgTYSHaiAFiilgjjggXmYX4IcFIBBKLJCDJiBRRIkuRNUgxUopUIFVIHfI9cgI5h1xGupE7yAAygvyGvEcxlIGyUT3UDLVDuag3GoRGogvQZHQxmo8WoJvQcrQaPYw2oefQq2gP2o8+Q8cwwOgYBzPEbDAuxsNCsTgsCZNjy7EirAyrxhqwVqwDu4n1Y8+xdwQSgUXACTYEd0IgYR5BSFhMWE7YSKggHCQ0EdoJNwkDhFHCJyKTqEu0JroR+cQYYjIxh1hILCPWEo8TLxB7iEPENyQSiUMyJ7mQAkmxpFTSEtJG0m5SI+ksqZs0SBojk8naZGuyBzmULCAryIXkneTD5DPkG+Qh8lsKnWJAcaT4U+IoUspqShnlEOU05QZlmDJBVaOaUt2ooVQRNY9aQq2htlKvUYeoEzR1mjnNgxZJS6WtopXTGmgXaPdpr+h0uhHdlR5Ol9BX0svpR+iX6AP0dwwNhhWDx4hnKBmbGAcYZxl3GK+YTKYZ04sZx1QwNzHrmOeZD5lvVVgqtip8FZHKCpVKlSaVGyovVKmqpqreqgtV81XLVI+pXlN9rkZVM1PjqQnUlqtVqp1Q61MbU2epO6iHqmeob1Q/pH5Z/YkGWcNMw09DpFGgsV/jvMYgC2MZs3gsIWsNq4Z1gTXEJrHN2Xx2KruY/R27iz2qqaE5QzNKM1ezUvOUZj8H45hx+Jx0TgnnKKeX836K3hTvKeIpG6Y0TLkxZVxrqpaXllirSKtRq0frvTau7aedpr1Fu1n7gQ5Bx0onXCdHZ4/OBZ3nU9lT3acKpxZNPTr1ri6qa6UbobtEd79up+6Ynr5egJ5Mb6feeb3n+hx9L/1U/W36p/VHDFgGswwkBtsMzhg8xTVxbzwdL8fb8VFDXcNAQ6VhlWGX4YSRudE8o9VGjUYPjGnGXOMk423GbcajJgYmISZLTepN7ppSTbmmKaY7TDtMx83MzaLN1pk1mz0x1zLnm+eb15vft2BaeFostqi2uGVJsuRaplnutrxuhVo5WaVYVVpds0atna0l1rutu6cRp7lOk06rntZnw7Dxtsm2qbcZsOXYBtuutm22fWFnYhdnt8Wuw+6TvZN9un2N/T0HDYfZDqsdWh1+c7RyFDpWOt6azpzuP33F9JbpL2dYzxDP2DPjthPLKcRpnVOb00dnF2e5c4PziIuJS4LLLpc+Lpsbxt3IveRKdPVxXeF60vWdm7Obwu2o26/uNu5p7ofcn8w0nymeWTNz0MPIQ+BR5dE/C5+VMGvfrH5PQ0+BZ7XnIy9jL5FXrdewt6V3qvdh7xc+9j5yn+M+4zw33jLeWV/MN8C3yLfLT8Nvnl+F30N/I/9k/3r/0QCngCUBZwOJgUGBWwL7+Hp8Ib+OPzrbZfay2e1BjKC5QRVBj4KtguXBrSFoyOyQrSH355jOkc5pDoVQfujW0Adh5mGLw34MJ4WHhVeGP45wiFga0TGXNXfR3ENz30T6RJZE3ptnMU85ry1KNSo+qi5qPNo3ujS6P8YuZlnM1VidWElsSxw5LiquNm5svt/87fOH4p3iC+N7F5gvyF1weaHOwvSFpxapLhIsOpZATIhOOJTwQRAqqBaMJfITdyWOCnnCHcJnIi/RNtGI2ENcKh5O8kgqTXqS7JG8NXkkxTOlLOW5hCepkLxMDUzdmzqeFpp2IG0yPTq9MYOSkZBxQqohTZO2Z+pn5mZ2y6xlhbL+xW6Lty8elQfJa7OQrAVZLQq2QqboVFoo1yoHsmdlV2a/zYnKOZarnivN7cyzytuQN5zvn//tEsIS4ZK2pYZLVy0dWOa9rGo5sjxxedsK4xUFK4ZWBqw8uIq2Km3VT6vtV5eufr0mek1rgV7ByoLBtQFr6wtVCuWFfevc1+1dT1gvWd+1YfqGnRs+FYmKrhTbF5cVf9go3HjlG4dvyr+Z3JS0qavEuWTPZtJm6ebeLZ5bDpaql+aXDm4N2dq0Dd9WtO319kXbL5fNKNu7g7ZDuaO/PLi8ZafJzs07P1SkVPRU+lQ27tLdtWHX+G7R7ht7vPY07NXbW7z3/T7JvttVAVVN1WbVZftJ+7P3P66Jqun4lvttXa1ObXHtxwPSA/0HIw6217nU1R3SPVRSj9Yr60cOxx++/p3vdy0NNg1VjZzG4iNwRHnk6fcJ3/ceDTradox7rOEH0x92HWcdL2pCmvKaRptTmvtbYlu6T8w+0dbq3nr8R9sfD5w0PFl5SvNUyWna6YLTk2fyz4ydlZ19fi753GDborZ752PO32oPb++6EHTh0kX/i+c7vDvOXPK4dPKy2+UTV7hXmq86X23qdOo8/pPTT8e7nLuarrlca7nuer21e2b36RueN87d9L158Rb/1tWeOT3dvfN6b/fF9/XfFt1+cif9zsu72Xcn7q28T7xf9EDtQdlD3YfVP1v+3Njv3H9qwHeg89HcR/cGhYPP/pH1jw9DBY+Zj8uGDYbrnjg+OTniP3L96fynQ89kzyaeF/6i/suuFxYvfvjV69fO0ZjRoZfyl5O/bXyl/erA6xmv28bCxh6+yXgzMV70VvvtwXfcdx3vo98PT+R8IH8o/2j5sfVT0Kf7kxmTk/8EA5jz/GMzLdsAAAAgY0hSTQAAeiUAAICDAAD5/wAAgOkAAHUwAADqYAAAOpgAABdvkl/FRgAABONJREFUeNrEmMFvG0UUh7+13dI0Ng0pVEJIEJCQcgmEI1zo7pEDyh+A1JY7EhUnTglIvSG1cEGIQ3JBAg5VwglBWW9JSQWFkoCsxFjJOgpWtlXjNE6dOl57h8vbauV61/baEU8aRfaMZ7/83pvfzKymlCIqDMOYBM4Bk8DZNkMs4DowBxSj5jJNk15CC4MzDOMsMB0CFBYWcBFYHgRcIgTsMpDtEQwZ/ycwwwAi1QI1IlCTfc47DbwAXOhnklblBgHmx3lgdiBwkspBgQUB34/7Y00p5Rd/tovxy1L0e8ApYAoY6+J3LwLFXhdEKlAjnVbhhTZWcVEWQSfVp+PUX0J8LGpVzpmmqZumWYwAf018Liq9Y3Fq7lxE/7xpmt3+xxfC/E1iKg5clGoXe5wvavybceAmI9JZ7HE+K0K9sdhW0iZWYjqAFfL95CDhlmPC7Q3KJKPgxvifIwru1ZhzhhV+MQ7c/TBvkoNALzEWsfpjwYXV1kiMffFyRF9R07SE9ngQ1hIdCn/aMIzzYZ3ZbFaTllBKvRtltJ7n5YDjwBPSjsv2mRKRtHZ76/UOCs0ahjFmmuZMEEomTExMTIyOjo5+omnaO1GSViqVW0AaUIEG0AQa0pqA5/dpuq6PALtdpKwIzHuet9hsNveVUqeTyeTbyWTyLTmhhIZSasuyrNcD6mgCoAlQE6gDh9I8QPlHpjhH8q6j0Wh8s7i4+AFwTBRPtaTRA1ygCjzwAX0rWThKv2o2mwvAAfBQFEsBQ8BJaWlR/0n5PgloPtzcEbIVl5aWvhVFHggksihOAsOBlpbvE49M2DTN+8D8EcHN67ruF71fU0og0oE2HADTWneIT48ILjivJik90aKYD6YFVq1KBC68VhwX76QaUBTrSYlCzwBPi8n7qp0QNatATeAe21s/GiSZUuqzbDZ7TGrrNPA88BLwHPAUkJE+gH3ZSmuPfK71dYRhGPYgTiRKqUXLsqbk4aeAM8CzAumvyIZAbQHrQEnU8x678QfUm+0XznGcr4BXBGxUlEoHvM4H2wX+Be4ErCb8RU6/6tVqtX9u3rz5uSg0FNhPE/JwV1K4CeQBWz43gnCJkJR83I9qtm2vAuOB+jojBjssyj2UFOZlEe61goXCWZY1p5S6EQdsZ2en6DhOXWprRKDSUnuaKFQA/gY2JK1uK1jkSbher1+KsU256+vrm7IK0/LX97AG4AA5eU223i6VHeGUUmppaSnruu7VXuC2t7e3q9VqMuD4Q6JWRdS6Bfwhqaz4ZhvnDtGwbftDpVS1G7CDg4OHhUJhR6BOymHSBe7KNfMX4LbYRrUTWCc4VSqVnN3d3SvdwBUKhXuBlalJkeeBG3Kg/QvYlo3f6+v2pZTygNrKyspsrVbLR01SKpX2y+WyJ75ZE4u4BfwE/CyQ5bDCj6McUqxl27ZnPM87bDfg8PCwadv2gTz4jqTwR+B74FcB3dd1vdELWEc4Ua/qOM5vjuN83W7M2tranuu6O8CavIBcAK6JVdwFDnVd9+LYUqqbUzZwL5/Pf5nJZN7IZDIv+x2bm5uVcrmcl3q6LarZUm9uXKhu0+qrdwDYq6url+r1elVWZ21jY+Ma8B1wVdTKATtAvV+wbpXzr2+71Wr190Kh8MX4+Ph7uVxuAfhBfGtLjuCuruuKAcV/AwDnrxMM7gFGVQAAAABJRU5ErkJggg==',
    position: myLatlng,
    map: map,
    title: title,
    maxWidth: 200,
    maxHeight: 400
  });
  google.maps.event.addListener(marker, 'click', function() {
     infowindow.open(map,marker);
  });
  google.maps.event.trigger(map, "resize");
  google.maps.event.trigger(marker, "click");
  map.setCenter(myLatlng);
  
  return map;
}

/** click child input of parent */


$(document).on("click", ".click-parent-input", function() {
  var $input = $(this).parent().find("INPUT");
  $(this).parent().find("INPUT").click();
});

$(document).on("click", ".click-parent-select", function() {
  var $input = $(this).parent().find("SELECT");
  console.log("click parent select foo len=" + $input.length + " name=" + $input.attr('name'));
  openSelect($input);
});

$(document).on("click", ".click-parent-datepicker", function() {
  var $input = $(this).parent().find("INPUT");
  $input.datepicker('show');
});

$(document).on("click", "A.append-query", function() {
  var $link = $(this);
  var field = $(this).data('field');
  var $field = $(field);
  var href= $link.attr('href');
  console.log("append-query: field=" + field + " len=" + $field.length); 
  if($field.length) {
    var value = $field.val();
    var key = ltrim(field, "#");
    var href = replace_query_var(href, key, value)
    console.log("append-query: value=" + value + " href=" + href); 
    $link.attr('href', href);
  }
  return true;
});

$(document).on("change", ".checkbox2array", function(e) {
  var target = $(this).data('target');
  var collection = $(this).data('collection');
  var handler = $(this).data('handler');
  if(!target || !$(target).length) return;
  if(!collection ||!$(collection).length) return;
  var options = {};
  var name='';
  var checked=false;
  var ar = [];
  $.each($(collection), function(index) {
    name = $(this).attr('name');
    checked = $(this).prop("checked") ? 1 : 0;
    if(checked) ar.push(name);
  });
  var json = JSON.stringify(ar);
  $(target).val(json);
  
  if(handler) window[handler]();
});

$(document).on("click", "A.click-target", function() {
  var target = $(this).data('target');
  var $target = $(target);
  console.log("click-target:" + target + " len=" + $target.length + " class=" + $target.attr('class'));
  if($target.length) {
    $target.trigger("click");
    return false;
  } 
  
  return true;
});

$(document).on("click", "A.click-parent", function() {
  $(this).parent().trigger("click");
  return false;
});

$(document).on("click", ".set-active", function() {
  $(this).siblings().removeClass("active");
  $(this).addClass("active");
});

// used to show/hide elements of group with given class
// side effect: set class active of target
$(document).on("click", ".class-filter", function() {
  $(this).siblings().removeClass("active");
  $(this).addClass("active");
  
  var target = $(this).data('target');
  var on = $(this).data('on');
  var $target = $(target);
  if($target.length) {
    $target.hide();
    var show_class = target + on;
    //console.log("Hiding all except: " + show_class);
    $(show_class).show();
    return false;
  }

});
 


// checkbox changed, set target to 0 or 1
/** same as below
$(document).on("change",'INPUT.cb-hide-target',function() {
  var $cb = $(this);
  var checked = $cb.prop("checked");
  var target = $cb.data('target');
  var $target;
  $target = target ? $(target) : null;
  //console.log("cb-hide-target checked=" + checked + " target=" + target + " len=" + $target.legth);
  if($target.length) {
    $target.val(checked ? 1 : 0);
    $target.trigger("change");
  }
  
}); 
*/

// for links:
// like tabs, but then just toggle visibility of two divs
// both divs must be parent div of link and contain data-target pointing to other div
$(document).on("click", "A.toggle-div", function (e) {
  var $shown = $(this).closest('div');
  var target = $(this).data('target');
  var $target = $(target);
  if(!$target.length) {
    //console.log("Toggle divs: could not find target=" + target + " returning true");
    return true; // fall back on href
  }
  //console.log("Toggle divs: target=" + target + " len=" + $target.length);
  //console.log("id=" + $target.attr('id'));
  //console.log($target);
  
  var effect = $(this).data('effect') || "fast";
  $shown.addClass("hidden").hide(effect);
  $target.removeClass("hidden").show(effect);
  return false;

});

// used by checkboxes to show/hide div defined in data-target
// default is to show when checked. Set data-reverse to hide when checked
$(document).on("change", ".toggle-div", function() {
  var target = $(this).data('target');
  var reverse = $(this).data('reverse') || false;
  var $target = $(target);
  //console.log("Toggle div target=" + target + " len=" + $target.length + " checked=" + $(this).is(':checked'));
  if($target.length) {
    if($(this).is(':checked')){
      if(reverse) $target.hide(); else $target.removeClass("hidden").show();
    } else {
      if(reverse) $target.removeClass("hidden").show(); else $target.hide();
    }
  }
});

$(document).on("change", ".variant_chooser", function() {
  $target = $(this);
  var variant_id = $target.val();
  var $selected = $(this).find('option:selected');
  var product_id = $selected.data('product_id');  
  var site_id = $selected.data('site_id');  
  var price = $selected.data('price');
  var name = $selected.data('name');
  $target = $(this);
  var data = {'site_id': site_id, 'product_id': product_id, 'variant_id': variant_id, 'price': price, 'name': name};
  var operation = 'cart-row-update';
  var ajax_url = "/ajax.php?oper=" + operation + "&" + obj2qs(data);
  //console.log(ajax_url);
  ajax_cart_update($target, ajax_url, operation); // server side
  //return false;
  
  // client side code
  
  //var $parent_form = $target.closest('form');
  var $this_cell = $target.parent(); // chooser cell
  var $name_cell  = $this_cell.prev(); // prev table cell
  var $price_cell = $this_cell.next().find('.row_total'); // next table cell
  $name_cell.html(name);
  $price_cell.html(price);

  var $prices = $('#cart_form .row_total');
  var total = 0; 
  $.each($prices, function(i) {
    total += parseFloat($(this).text());
  });
  $('.cart_total').html(total);
  //console.log("id=" + variant_id + " price=" + price + " cell=" + $price_cell.);
});

$(document).on("click", ".add_cart, .remove_cart, .remove_floating_cart", function() {
  $target = $(this);
  var data = $target.data('data') || {};
  var floating_cart = $target.hasClass('remove_floating_cart') || $target.hasClass('add_cart') ? true : false;
  var operation = $target.hasClass('add_cart') ? 'cart-add' : 'cart-remove';
  var ajax_url = "/ajax.php?oper=" + operation + "&" + obj2qs(data);
  ajax_cart_update($target, ajax_url, operation);
  return false;
});

function ajax_cart_update($target, ajax_url, operation) {

  var $messages = $('#ajax_messages');
  var $errors = $('#ajax_errors');

  jQuery.ajax({
    type: "get",
      url: ajax_url,
      success: function (responseText) {
        var result = parse_json(responseText);
        var error = result.error;
        var message = result.message;
        var cart = result.cart || {};
        var cart_page = $('#cart').length ? true : false;
        //alert(dump(result));
        build_cart(cart);

        if(cart_page && operation == 'cart-remove') {  // also update cart page, it's showing
          $target.closest('tr').remove();
          // todo: update total
        }
        
        var count = cart.count;
        if(count > 0) {
          $('#cart_icon .cart_hide').removeClass('hidden').show('fast');
          $('#cart_icon .cart_show').hide('fast');
          $('#cart_icon').removeClass('hidden').show('fast');
        } else {
          $('#cart_icon .cart_hide').hide('fast');
          $('#cart_icon .cart_show').removeClass('hidden').show('fast');
          $('#cart_icon').hide('fast');
        }
        var total = cart.total || 0;
        //alert(total);
        $('#cart .cart_total, #floating_cart .cart_total').html(total);

        // should we show the floating cart
        $('#floating_cart').removeClass('hidden').show('fast');

        //if(cart) alert(dump(cart));
        if(error) $errors.html(error).parent().show();
        if(message) $messages.html(message).parent().show();
        //alert(responseText);
    }
  });
}
  
// hide cc form if charge card on file
$(document).on("change", "#ccof", function() {
  $target = $(this);
  var cc_id = $target.val();
  var $parent_form = $target.closest('form');
  if(cc_id) {
    $('#cc_form').hide('fast');
    //alert("this=" + $target.attr('id') + ' form=' + $form.attr('id'));
  } else {
    $('#cc_form').show('fast');
  }
});

// set cc_type from cc_number
$(document).on("change", "#cc_number", function() {
  $target = $(this);
  var cc_number = $target.val()
  cc_number = $.trim(cc_number);// sanitize
  cc_number = cc_number.replace(/[^0-9]/g,'');
  $(this).val(cc_number);

  var card = cc_find(cc_number);
  var $cc_icon = $('#cc_type_icon');
  $cc_icon.removeClass().hide();
  $cc_icon.addClass('cc-sprite');
  if(card) {
    $('#cc_type').val(card.id);
    $cc_icon.addClass(card.css_class);
    if(card.accepted) {
      $cc_icon.show('fast');
    } else {
      $cc_icon.show('fast').addClass('off');
      alert("Unforunately we do not accept " + card.name);
    }
  } else {
    $('#cc_type').val(0);
  }

});

$(document).on("change", "#cc_type", function() {
  $target = $(this);
  var cc_type = $target.val()
  var cc_number = $('#cc_number').val();
  if(cc_type && cc_number) {
    var valid =  cc_validate(cc_type, cc_number);
  }
  //var val = resAr[0] || 0;
  //$('#cc_type').val(val);  
  //var accepted = resAr[1];
  //var type = resAr[2];  
  //if(val && !accepted) alert("Unforunately we do not accept " + type);
});

$(document).on("click", "A.popup-link", function() {
  window.open($(this).prop("href"), "Edit Dialog", "height=900,width=550,modal=yes,alwaysRaised=yes,toolbar=0,location=0,menubar=0");
  return false;
});

// my own drop down function (pre-bootstrap)
$(document).on("click", ".dropdown", function() {
  var target = $(this).data("toggle");
  if(!target) return true; // avoid conflict with Bootstrap dropdown  
  var $target = $("#"+target);
  if(!$target.length) return true;
  $target.toggle(200, "swing");
  return false;
});

function submit_parent_form() {
  var $form = $(this).closest("form");
  console.log("submit parent len=" + $form.length + " name=" + $form.attr('id'));
  if($form.length) {
    $form.submit();
    return false;
  }
  return true;
}

$(document).on("click", ".submit_parent_form, .submit_parent, .submit-parent-form, .submit-parent", function() {
  //alert('submit parent' + $(this).closest("form").prop('id'));
  var $form = $(this).closest("form");
  var form_id = $form.attr('id');  
  console.log("submit parent form id=" + form_id + " len=" + $form.length);
  if(!$form.length) return false;
  
  if(form_id) { // in case of multiple submit buttons, store name of button used in the form field submit_action
    //var $action = $("#" + form_id + " .submit_action");
    var $action = $("#" + form_id + " [name=submit_action]");  
    var action = $(this).attr('name') || $(this).data('action') || '';
    if(form_id && $action.length && action) {
      $action.val(action);
      //console.log("submit_parent_form: action=" + action);
    }
  }

  // option to set form input before submitting: use data-set_form_input and data-value  
  var target = $(this).data('set_form_input');
  var $target = target ? $("#" + form_id + " [name=" + target + "]") : null;  
  if(target && $target.length) { // set form value before submitting    
    var value = $(this).data('value') || '';
    $target.val(value);
    //console.log("submit-parent: setting " + target + " = " + value);
  }
  
  $form.submit();
  return false;
});


/** third party login link */
$(document).on("click", ".tpl-link", function() {
  //var ref = $("#register-redirect").val();
  var ref = $(".login-redirect").val();
  console.log("tpl link v3 ref=" + ref);
  if(ref) {
    href = $(this).attr('href');
    $(this).attr('href', href + "&redirect=" + encodeURIComponent(ref))
  }
  return true;
});

$(document).on("click", ".set-target-value", function() {    
  var target = $(this).data('target');
  var value = $(this).data('value');
  var field = $(this).data('append-field');
  var $target = $(target);
  console.log("set target value t=" + target + " v=" + value + " l=" + $target.length);

  if(!target) return true;
  if(!$target.length) return true;
  if(field && $(field).length) { // field value from field
    var aval = $(field).val();
    var key = ltrim(field, "#");
    value = replace_query_var(value, key, aval);
    console.log("field-query: value=" + value + " key=" + key + " aval=" + aval); 
  }
  $target.val(value);
  
  console.log("stv: " + value);
  // $(".tpl-link").attr('href', value); /** hack: can't get it working in time given, should happen in request_login_handler() */
  return false;
});

var timeoutId;
var g_autosave_timeout = 1000; // 15s
$(document).on("input propertychange change", "TEXTAREA.autosave, FORM.autosave TEXTAREA", function() {    
  var timeout = g_autosave_timeout;
  var $textarea = $(this);
  //console.log('Textarea Change ta=' + $textarea.attr('name'));
  if($(this).data('timeout')) timeout = $(this).data('timeout');
  clearTimeout(timeoutId);
  timeoutId = setTimeout(function() { // Runs 1 second (1000 ms) after the last change    
    saveToDB($textarea);
  }, timeout);
});

function saveToDB($textarea) {
  $form = $textarea.closest("form");
  var $label = $textarea.parent().find('label');
  if(!$label.length) {
    $(".autosaving-label").remove();
    $label = $('<div class="autosaving-label"></div>').insertBefore($textarea);  // append if doesn't exist   
  }
  console.log("label len=" + $label.length);
  var org_label = $label.data('org-label');
  var label = $label.html();
  if(!org_label) {
    org_label = label;
    $label.data('org-label', org_label);
  }
  
  var saving = '<span class="text-success"><i class="fa fa-spinner fa-spin"></i> Saving...</span>';
  var saved = '<span class="text-success"><i class="fa fa-check"></i> Saved!</span>';
  
  $label.html(org_label + ' ' + saving);
  //$textarea.css("border", "2px solid green");
  autosave($form, true);
  $label.html(org_label + ' ' + saved);
  //$textarea.css("border", "none");
}


$(document).on("click", ".cancel_parent, .close_parent_dialog", function() {
  return close_parent($(this));
});

$(document).on("click", ".close_parent", function() {
  $(this).parent().hide('fast');
  return false;
});

$(document).on("click", ".onclick-share", function() {
  share_object($(this).data("share"));
  return false;
});


// http://stackoverflow.com/questions/5999118/add-or-update-query-string-parameter
function updateQueryStringParameter(uri, key, value) {
  var re = new RegExp("([?&])" + key + "=.*?(&|$)", "i");
  var separator = uri.indexOf('?') !== -1 ? "&" : "?";
  if (uri.match(re)) {
    return uri.replace(re, '$1' + key + "=" + value + '$2');
  } else {
    return uri + separator + key + "=" + value;
  }
}

// social pop-up share
$(document).on("click", ".fb-loader-popup", function() {
  var url = $(this).data('url');
  if(!url) return true;

  var tpl = $(this).data('template');
  var ctrl = $(this).data('controller');
  var obj_type = $(this).data('obj_type');
  var obj_id = $(this).data('obj_id');
  var data = $(this).data('data');
  
  if(tpl) url = updateQueryStringParameter(url, 'template', tpl);
  if(ctrl) url = updateQueryStringParameter(url, 'controller', ctrl);
  if(obj_type) url = updateQueryStringParameter(url, 'obj_type', obj_type);
  if(obj_id) url = updateQueryStringParameter(url, 'obj_id', obj_id);
  if(data) url = updateQueryStringParameter(url, 'data', JSON.stringify(data));
  
  console.log("fb-loader tpl=" + tpl + " ctrl=" + ctrl + " obj_type=" + obj_type + " obj_id=" + obj_id + " data=", data);
  window.open(url, 'sharer', 'width=626,height=436');
  return false;
});

$(document).on("click", ".vp-loader-galleria", function() {
  var search = $(this).data('search') || {};
  var aid = search.aid;
  var id = search.id;
  var ajax_url = '/ajax.php?oper=get-gallery';
  if(aid) ajax_url += '&aid=' + aid;
  if(id) ajax_url += '&id=' + id;
  //$('.galleria-image').fadeIn('fast');
  $('#galleria .galleria-stage').addClass("loading-big");
  //$('#galleria').data('galleria').load([]);
  var $galleria = $('#galleria').data('galleria');
  $galleria.exitFullscreen();
  //alert(ajax_url);
  $.getJSON(ajax_url, function(json) {        
    //alert("galleria length=" + $galleria.length + dump(json));
    //$('.galleria-image').fadeOut('fast');
    $('#galleria .galleria-stage').removeClass("loading-big");
    $galleria.load(json.items);
   });
  return false;
});

$(document).on("click", ".verify-cancel", function() {
  var $link = $(this);
  $div = $link.closest('.user-verification-form');
  $button = $div.prev('.verify-link');
  $div.hide();
  $div.closest('.alert').hide();
  $div.find('.alert').hide();
  
  $button.show().attr('disabled', false);
  //console.log('verfication cancel div=' + $div.length + ' button=' + $button.length);
  return false;
});

$(document).on("click", ".verify-link", function() {
  var $link = $(this);
  var html = $link.html();
  var loading = '<i class="fa fa-spinner fa-spin"></i> Verifying...';
  var data = $link.data();
  var ajax_url = '/ajax.php?oper=user-verification';
  $link.html(loading)    
  $link.attr('disabled', true);
  //console.log('url=' + ajax_url);
  //console.log('data=', data);
  $.getJSON(ajax_url, data, function(response) {
     //console.log(response);
     $link.html(html);
     if(response.success) {
       //console.log('1');
       $link.before(bootstrap_success_message(response.message));
       //console.log('2');
       var $form = $link.next('.user-verification-form');
       //console.log('3');
       //console.log("Form len=" + $form.length);
       $form.removeClass('hide').show();
       $link.hide();
     } else {
       $link.before(bootstrap_error_message(response.error));
       $link.attr('disabled', false);
     }
     
  });
  return false;
});

$(document).on("click", ".vp-loader-data", function() {
  var search = $(this).data('search') || {};
  var aid = search.aid;
  var id = search.id;
  var ajax_url = '/ajax.php?oper=get-gallery';
  var params = {
    format: "json",
    viewer: "galleria",
    oper: "get-gallery",
    obj_type: search.obj_type,
    id: search.id,
  };

  //$('.galleria-image').fadeIn('fast');
  $('#galleria .galleria-stage').addClass("loading-big");
  //$('#galleria').data('galleria').load([]);
  var $galleria = $('#galleria').data('galleria');
  $galleria.exitFullscreen();
  //alert(ajax_url);
  $.getJSON(ajax_url, function(json) {        
    //alert("galleria length=" + $galleria.length + dump(json));
    //$('.galleria-image').fadeOut('fast');
    $('#galleria .galleria-stage').removeClass("loading-big");
    $galleria.load(json.items);
   });
  return false;
});

$(document).on("click", ".onclick-close, .click-close", function() {
  $(this).hide();
  return false;
});

$(document).on("change", ".phone_idd_input", function() {
  var selected = $(this).find('option:selected');
  var idd = selected.data('idd_prefix');
  //console.log(selected);
  var idd_id = $(this).next().prop('id');
  //console.log("idd changed to "  + idd_id);
  $('#' + idd_id).val("+" + idd);
  $('#' + idd_id).next().html("+" + idd);
});

function form_data(form_id) {
  var data = {};
  var $form = $('#' + form_id);
  if(!$form.length) return {};
  //return $form.serializeArray();
  //return JSON.stringify($form.serializeObject()); // we don't want a string, but an object */                 
  $.each($form.serializeArray(), function(_, kv) {
    data[kv.name] = kv.value;
  });
  return data;
}

function template_load(target, template, controller, options, data) {  
  if(typeof controller == "undefined") var controller = '';
  if(typeof options == "undefined") var options = {};
  if(typeof data == "undefined") var data = {};
  var success_handler = options.success_handler || '';

  var language = options.language || '';
  
  var prepend = options.prepend;
  
  options.unauth = 1; /** todo, change */
 
  $target = $(target);
  console.log("Loading template=" + template + " into " + target + " len=" + $target.length);
  console.log("controller=" + controller + " data= ", data);
  console.log("language=" + language + " options= ", options);

  if($target.length && template) {
    var base_url = options.unauth ? '/ajax.php' : '/ajax.php';
    ajax_url = base_url + '?oper=fetch-template&template=' + template + '&controller=' + controller;
    if(language) ajax_url += "&__sl=" + language;
    //if(data) ajax_url = ajax_url + '&' + obj2qs(data);

    if(0) { // backdrop template
      var backdropTemplate = $('<div class="modal-backdrop-off fade-off" style="width: 100%;height:100%;margin: 0px auto;color: #ccc"><i class="fa fa-spinner fa-4x fa-spin grey"></i></div>');
      $target.html(backdropTemplate);
    } else { // fade
      $target.addClass('loading'); //html(backdropTemplate);
    }

    console.log("template_load: posting to url= " + ajax_url + " with data=", data);
    
    $.post(ajax_url, data, function(responseText) {
      $target.removeClass('loading'); //html(backdropTemplate);
  
      if(prepend) {  
        //console.log("Template loaded. Prepending " + responseText.length + " bytes");
        //$(responseText).prependTo($target).fadeIn("slow");
        
        $target.prepend(responseText);
      } else {
        //console.log("Template loaded. Overwriting " + responseText.length + " bytes");
        $target.html(responseText);
      }
      if(success_handler) {
        console.log("template_load done, calling " + success_handler);
        window[success_handler](); // eval = evil
      }
    });
    
    //$target.load(ajax_url, data, function(responseText) {        
    //  if(success_handler) {
    //    //console.log("done, calling " + success_handler);
    //    window[success_handler](); // eval = evil
    //  }
    //});
    
    if(!in_iframe()) $('html, body').animate({scrollTop:0}, 'slow');
    
    return false;
  } else { // non-ajax 
    //console.log("Loading template=" + template + " into " + target + " len=" + $target.length);
    //console.log("Failed!");return false;
    return true; // let the normal link do its thing
  }
  
}

$(document).on("click", "A.helpful-link", function() {
  var $item = $(this);
  var data = {};
  var obj_type = data.obj_type = $item.data('obj_type');
  var obj_id = data.obj_id = $item.data('obj_id');
  //console.log("helpful on " + obj_type + " " + obj_id);
  var ajax_url = "/ajax.php?oper=helpful";
  if(obj_type && obj_id) {
    $item.removeClass("fa-thumbs-up").addClass("fa-spinner fa-spin");    
    $.getJSON(ajax_url, data, function(result) {
      $item.removeClass("fa-spinner fa-spin").addClass("fa-thumbs-up");    
      if(result.success) {
        var count = result.count;
        //console.log("Success: count=" + count);
        //console.log("Helpful len=" + $item.next('.helpful-count').length);
        $item.next('.helpful-count').html(count);
      } else {
        //console.log("Error:", result);
      }
    });
  }
  return false;
});

$(document).on("click", "A.wishlist-link", function() {
   var $item = $(this);
   var data=$item.data('data');
   var $icon = $item.find("i");
   //var $remove = $item.hasClass("remove");
   
   //console.log("wishlist data=", data);   
   //console.log("wishlist icon class=", $icon.attr('class'));
   ajax_url = "/ajax.php?oper=wishlist&" + obj2qs(data);
   $icon.removeClass("fa-heart fa-heart-o fa-times").addClass("fa-spinner fa-spin");
   //console.log("Remove=" + remove);
   //return false;
   
   jQuery.ajax({
     type: "get",
       url: ajax_url,
       success: function (responseText) {
          var result = parse_json(responseText);
          //console.log("response", result);
          //console.log("result", result);
          if(result.success) {
            //console.log("success");
            var icon_class = result.id ? "fa-heart" : "fa-heart-o";
            $icon.removeClass("fa-spinner fa-spin").addClass(icon_class);
          } else {
            //console.log("error");
            $icon.removeClass("fa-spinner fa-spin").addClass("fa-exclamation-triangle fa-danger");
          }         
       }
   });
   
   return false;
});

// Thanks: https://github.com/johnculviner/jquery.fileDownload
$(document).on("click", "a.fileDownloader", function() {
  $.fileDownload($(this).attr('href'), {
      preparingMessageHtml: "We are preparing your download, please wait...",
      failMessageHtml: "There was a problem generating your download, please try again."
  });
  return false; //this is critical to stop the click event which will trigger a normal file download!
});

// click on a linkref, and this code will trigger clicking of another link specified in target
// fallback on link clicked
$(document).on("click", "A.linkref", function() {
  var $item = $(this);
  var target = $item.data('target');
  var $target = $(target);
  console.log("linkref 5: target=" + target + " len=" + $target.length);
  if($target.length) {
    var data = $item.data('data');
    var href = $target.attr('href');
    var org_href = href;
    
    console.log("clicking link with href=" + href + " data=",data);
    if(data) {
      $target.data(data); // so target's handler can access data
      //var delim = href.indexOf('?') > 0 ? '&' : '?';
      //href = href + delim + obj2qs(data);
      console.log("target data=", data);
    }
    
    
    /**
    if(data) delete data.target; // already used    
    $target.data(data); // so target's handler can access data
    */
    
    $target.trigger("click");
    // restore
    //$target.parent().click();
    //$target.attr('href', org_href);
    //console.log("restored href");
    
    return false;
  } else {
    console.log("couldn't find target=" + target);
  }
  return true;
});


// new version of 2017, load smarty template into div
$(document).on("click", ".subview-nav A", function(e) {
  var $link = $(this);
  var href = $link.attr('href');
  if($link.data('breakout')) return true; // don't use soft load 
    
  var prefix = $("#subview-prefix").val();  
  var $nav = $link.closest(".subview-nav");  
  //var pin_code = $nav.data('pin_code');
  //if(pin_code) path = path + "?pin=" + pin_code;
  var target = $nav.data('target');
  var success_handler = $nav.data('success_handler');
  var $target = $(target);

  console.log('subview-nav: target=' + target + ' len=' + $target.length + ' data:', $nav.data());
  
  if(href.charAt(0) == '#') return true;

  if(g_form_dirty) {
    e.preventDefault();
    console.log("subview-nav: form dirty a href=" + href);
    if(!confirm_modal($target)) return false;
  }
  
  if(href && $target.length) {
    console.log("OK> Click " + href + " handler=" + success_handler);
    soft_load($link, target, href, success_handler);
    
    $(this).closest('ul').find('li').removeClass('active');
    $(this).parent().addClass('active');

    return false;
  }
  
  //console.log('gh  missing href or target');return false
  return true; // fallback on regular load  
});


// new version of vmenu, load smarty template into div
/**
$(document).on("click", "A.smtlink", function() {
  var $item = $(this);
  var $anchor = $("a", this);
  var $nav = $item.closest("NAV");
  var target = $nav.data('target') || $item.data('target');
  var handler = $nav.data('handler');
  var success_handler = $nav.data('success_handler') || '';
  var data = $nav.data('data') || {};
  var controller = $nav.data('controller') || $item.data('controller') || '';
  var options = {};
  options.success_handler = success_handler;
  var link_data = $item.data();
  console.log("smtlink link data=", link_data);
  if(link_data) {
    //console.log("smtlink nav data=", data);
    console.log("smtlink link data=", link_data);
    data = $.extend(data, link_data);
    //console.log("smtlink combined data=", data);
  }
  data.page_id = $("BODY").data('id');
  var tpl = $item.data("tpl") || $item.data("template") || '';
  console.log("smtlink: Loading tpl=" + tpl + " into " + target + " data=", data);
  return template_load(target, tpl, controller, options, data);     
                                                              
});
    
$(document).on("click", ".vmenu-item", function() {
  var $item = $(this);
  var $anchor = $("a", this);
  var $nav = $item.closest("NAV");
  if($anchor.hasClass('static')) return true; // static link
  $item.siblings().removeClass('selected');
  $item.addClass('selected');
  var $icon = $("div", this);    
  var ajax_url = $anchor.prop("href");
  var icon_pos = $icon.css('background-position');
  $icon.css('background-position', '40px 40px'); // hide icon
  $icon.html('<div class="loading">&nbsp; &nbsp; &nbsp; </div>'); // show wheel
  
  var target = $nav.data('target');
  var handler = $nav.data('handler');
  var data = $nav.data('data') || {};
  var section = ajax_url.split('?section=')[1];
  //alert("handler=" + handler + " target=" + target + " section=" + section);       
  if(handler && target && section) {      
    ajax_url = '/ajax.php?oper=' + handler + '&section=' + section; // todo: allow unauth ?
    if(data) ajax_url = ajax_url + '&' + obj2qs(data);
    $('#' + target).load(ajax_url, function(responseText) {        
      var onload = $anchor.data('onload'); // optionally call JS function on load
      // done, restore
      $icon.css('background-position', icon_pos); // hide icon
      $icon.html(''); // hide wheel
      if(onload) eval(onload + "(data);");
    });     
  } else { // non-ajax 
    return true; // let the normal link do its thing
  }
  
  return false;
});
*/

$(document).on("click", ".loader", function() {
  $("body").addClass("disabled");
  $("#content").addClass("loading-big");
});

$(document).on("mouseenter", ".hilite_target", function() {
  var target_id = $(this).data('target');
  var target_class = $(this).data('class') || "hover";
  if(target_id) $('#' + target_id).addClass(target_class);
  //console.log("enter target=" + target_id);
});

$(document).on("mouseleave", ".hilite_target", function() {
  var target_id = $(this).data('target');
  var target_class = $(this).data('class') || "hover";
  if(target_id) $('#' + target_id).removeClass(target_class);
  //console.log("leave target=" + target_id);
});


// bust out of iframe
$(document).on("click", ".break-frame", function(e) {
  var $link = $(this);
  var href = $link.attr('href') || '';
  if (!href) {
    //console.log("no link");
    return false;
  }
  if (top.location != self.location) {
    //console.log("setting top to " + href);    
    top.location = href;
    return false;
  } else {
    //console.log("do nothing");
    return true;
  } 
  return false;
});

$(document).on("click", ".show-hide-icon, .show-hide-title", function(event) {
  var $div = $(this).parent().find('DIV');
  //console.log("show-hide: div class=" + $div.attr('class'));
  // not sure why toggle doesn't work
  if($div.hasClass('hidden')) {
    $div.removeClass('hidden');//.show('fast');
  } else {
    $div.addClass('hidden');//.hide('fast');
  }
});

$(document).on("click", "A.google-pac-link", function(event) {
  var $link = $(this);
  var ref = $link.data('reference');
  //console.log("Handled by select, do nothing");
  //google_pac_details(ref);
});

$(document).on("click", ".new-tab", function(event) {
  var $link = $(this);

  // special case: in cms, no tabs
  var $cms = $('#cms_viewer');
  var $tree = $("#tree");
  if($cms.length && $tree.length) {
    return cms_link_handler($link);
  }

  var href = $(this).attr('href') || '';
  
  var name = $link.data('target-tab') || 'Explore';
  var data = $link.data('data') || {};
  var nav_data = $link.data('nav') || {};
  var data_source = nav_data.nav_source;
  var target_id = $link.data('target') || '';
  var $tabs = target_id ? $('#' + target_id) : $link.closest('.jquery-tabs');
  
  data.ref = href == '#' ? '' : href;
  
  //console.log("new-tab target = " + target_id);
  //console.log(data);
  //console.log("tab count=" + $tabs.length);
  //console.log($tabs);
  
  if(target_id) {
    event.preventDefault();
    $.fancybox.close();
  } else if(href && href.length > 1) {
    var queryString = href.substring(href.indexOf('?') + 1 );
    var params = parseQueryString(queryString);
    
    data.index = data.index || params.i || 0;
    data.count = data.count || params.c || 0;
    data.nav_source = data.nav_source || params.nav_source;
  } else if(nav_data && data_source) {
    data = data_source.indexOf('explore-data') > -1 ? $('body').data(data_source) : $('#' +data_source).data('result');
    data = $.extend(data, nav_data);
    data.id = data.id_list[data.index];
    //console.log("nav result");//console.log(data);
  }
  
  var safename = name.replace(' ', '_').toLowerCase();
  var tabs_id = $tabs.attr('id') || 'tabs';
  var tab_id = tabs_id + '_tab_' + safename;
  if($tabs.length) {
    if(!$('.jquery-tabs a[href="#' + tab_id + '"]').length) { // add tab if not found
      $( "<li><a href='#" + tab_id + "'>" + name + "</a></li>" ).appendTo(".jquery-tabs .ui-tabs-nav" );
      $tabs.append("<div id='" + tab_id + "'></div>");    
    }
    //console.log("tab id=" + tab_id);
    $tabs.tabs( "refresh" );
    $tabs.tabs('option', 'active', -1); //  Activate the last one
    var $content = $('#' + tab_id);
    $content.html("Loading...").load("/ajax.php?oper=explore-gallery" + '&' + obj2qs(data));
  }
  var target_id = $(this).data('target');
  event.preventDefault();
  return false;
  //console.log("leave target=" + target_id);
});

$(document).on("click", ".gmenu UL LI A.previous, .gmenu UL LI A.next", function() {
  var $link = $(this); // the anchor
  //var $list = $item.parent(); // the gmenu UL
  //var options = $link.data('options') || {};
  //console.log("prev-next");
  var data = $link.data('nav') || {};
  var options = $link.data('options') || {};

  var site_id = data.site_id;
  var index = parseInt(data.index);
  var count = parseInt(data.count);
  var new_index = -1;
  var previous = $link.hasClass('previous');
  var next = $link.hasClass('next');
  if(previous) new_index = index - 1;
  else if(next) new_index = index + 1;
  
  var operation = '';    
  //alert(dump(data));
    //alert(dump(options));
                       
  options.format = 'json';
  options.no_session = 1; 

  var obj_type = data.obj_type;
  var parent_type = data.parent_type;
  var parent_id = 0;
  switch(obj_type) {
  case 'user':
    operation = 'gallery_user_list';
    options.role = data.obj_subtype;
    options.collection_type = data.child_obj_subtype;
    break;
  case 'media_collection':
    operation = 'gallery_collection_list';
    var search = {"type": data.obj_subtype, "user_id": data.parent_id}
    options.search = search;
    break;
  case 'gallery':
    operation = 'gallery_folder_list';
    var search = {"user_id": data.parent_id}
    options.search = search;
    options.auth_edit = 0;
    break;
  case 'media':
    parent_id = data.parent_id;
    var search = {"user_id": data.parent_id}
    if(parent_type == 'media_collection') {
      search.type = data.obj_subtype;
      operation = 'gallery_collection';
    } else if(parent_type == 'gallery') {
      operation = 'gallery_folder';
      options.auth_edit = 0;      
    } else {
    }
    options.search = search;
    break;
  default:
    break;
  }
  if(operation) {                                                                                                          
    var ajax_url = parent_id ? 
      '/ajax.php?oper=load-function-json&function=' + operation + '&param1=' + site_id + '&param2=' + parent_id + '&param3=' +  JSON.stringify(options) :
      '/ajax.php?oper=load-function-json&function=' + operation + '&param1=' + site_id + '&param2=' +  JSON.stringify(options);
      
      //alert(ajax_url);return;
    
    var $messages = $('#ajax_messages');
    var $errors = $('#ajax_errors');
    $(this).css("background", 'url(/images/animated/loading.gif) 10px 4px no-repeat #f3f3f3');
    jQuery.ajax({
      type: "get",
        url: ajax_url,
        success: function (responseText) {
          var error = '';
          var message = '';
          
          // //console.log(responseText);
          var links = parse_json(responseText);
          
          //alert(dump(links));
          var link = links[new_index];
          if(link) {
            var separator = link.indexOf('?') > -1 ? '&' : '?';
            link = link + separator + "c=" + count + "&i=" + new_index;
            window.location.href = link;
            return false;
          }
          //alert(dump(message)); return false;
          //if(error) $errors.html(error).parent().show();
          //if(message) $messages.html(message).parent().show();
          //alert(message);
          
          //alert(dump(responseText));
        }
    });
    
  }
  //return false;  
  //var obj_type = options.obj_type || $link.data('obj_type') || '';
  
  return false;
}); 

// New boostrap event handlers
$(document).on("click", '#navLogin .dropdown-menu', function(evt) { 
  //console.log("click on nav login drop down");
  //console.log($(this));
  evt.stopPropagation();
});

// Javascript code copyright 2009 by Fiach Reid : www.webtropy.com
 // This code may be used freely, as long as this copyright notice is intact.
       
function luhn_validate(number) {
  var LuhnDigit = parseInt(number.substring(number.length-1,number.length));   
  var LuhnLess = number.substring(0,number.length-1);                        
  if (lunh_calc(LuhnLess)==parseInt(LuhnDigit)) return true;
  return false;
}

function lunh_calc(number) {
  var sum = 0;
  for (i=0; i<number.length; i++ ) {
    sum += parseInt(number.substring(i,i+1));
  }
  var delta = new Array (0,1,2,3,4,-4,-3,-2,-1,0);
  for (i=number.length-1; i>=0; i-=2 ) {    
    var deltaIndex = parseInt(number.substring(i,i+1));
    var deltaValue = delta[deltaIndex]; 
    sum += deltaValue;
  } 
  var mod10 = sum % 10;
  mod10 = 10 - mod10; 
  if (mod10==10) {    
    mod10=0;                                              
  }
  return mod10;
}

function cc_cards() {
  var cards = [
    {'id': 1,  'accepted': 1, 'css_class': 'amex', 'name': 'American Express', 'length': '15',  'prefixes': '34,37', 'checkdigit': true},
    {'id': 2,  'accepted': 1, 'css_class': 'visa', 'name': 'Visa',  'length': '13,16',  'prefixes': '4', 'checkdigit': true},
    {'id': 3,  'accepted': 1, 'css_class': 'mc', 'name': 'Mastercard',  'length': '16',  'prefixes': '51,52,53,54,55', 'checkdigit': true},
    {'id': 4,  'accepted': 1, 'css_class': 'maestro', 'name': 'Maestro',  'length': '12,13,14,15,16,17,18,19',  'prefixes': '5018,5020,5038,5893,6304,6759,6761,6762,6763,0604', 'checkdigit': true},
    {'id': 5,  'accepted': 1, 'css_class': 'visa', 'name': 'Visa Electron',  'length': '16',  'prefixes': '4026,417500,4405,4508,4844,4913,4917', 'checkdigit': true},
    {'id': 6,  'accepted': 1, 'css_class': 'discover', 'name': 'Discover',  'length': '16',  'prefixes': '6011,622,644,645,646,647,648,649,65', 'checkdigit': true},
    {'id': 10, 'accepted': 0, 'css_class': 'diners', 'name': 'Diners Club',  'length': '16', 'prefixes': '54,55', 'checkdigit': true},
    {'id': 11, 'accepted': 0, 'css_class': 'jcb', 'name': 'JCB',  'length': '16',  'prefixes': '35', 'checkdigit': true},
    {'id': 15, 'accepted': 0, 'css_class': 'solo', 'name': 'Solo',      'length': '16,18,19', 'prefixes': '6334,6767', 'checkdigit': true},
    {'id': 16, 'accepted': 0, 'css_class': 'switch', 'name': 'Switch',    'length': '16,18,19', 'prefixes': '4903,4905,4911,4936,564182,633110,6333,6759', 'checkdigit': true},
    {'id': 17, 'accepted': 0, 'css_class': 'diners', 'name': 'Carte Blanche',  'length': '14',  'prefixes': '300,301,302,303,304,305', 'checkdigit': true},
    {'id': 10, 'accepted': 0, 'css_class': 'diners',   'name': 'Enroute',  'length': '15', 'prefixes': '2014,2149', 'checkdigit': true},
    {'id': 20, 'accepted': 0, 'css_class': 'bankcard', 'name': 'Bankcard',  'length': '16', 'prefixes': '5610,560221,560222,560223,560224,560225', 'checkdigit': true},
    ];
    
  return cards;
}
// should be identical to PHP implementation in cc_check.inc
// takes a cc number, returns db ID, type, and if accepted (by default)
// if validate is true, also performs luhn check (above)
// default: not for validation, just to help user automatically select type
// expects number to be sanitized (digits only)
function cc_validate(type, number, echo) {
  //echo = true;
  if(!number) {
    if(echo) alert("Missing credit card number");
    return false;
  }

  var isnum = /^\d+$/.test(number);
  if(!isnum) {
    if(echo) alert(number + " is not numeric");
    return false;
  }

  var cards = cc_cards();  
  //alert(dump(cards)); 
  
  var card = findObjects(cards, 'id', type);
  if(card.length < 1) return false;
  if(card.length > 1) return false; // will never happen

  card = card[0];
  var lengths = card.length.split(',');
  var prefixes = card.prefixes.split(',');
  var name = card.name;
  var checkdigit = card.checkdigit;

  // check length
  var validLength = false;  
  $.each(lengths, function(i, length) {
    // window.console && console.log("checking if " + number + " begins with " + length);
    if(number.length == length) {
      // window.console && console.log("YES. " + number + " has length " + length);
      validLength = true;
      return false;
    }
    // window.console && console.log("No. " + number + " does not have length " + length);
  });
  if(!validLength) {
    // window.console && console.log("No. " + number + " has invalid length: " + number.length);
    if(echo) alert("Invalid length");
    return false;
  }
  
  // check prefix
  var validPrefix = false;  
  $.each(prefixes, function(i, prefix) {
    // window.console && console.log("checking if " + number + " begins with " + prefix);
    if(number.indexOf(prefix) == 0) {
      // window.console && console.log("YES. " + number + " begins with " + prefix);
      validPrefix = true;
      return false;
    }
    // window.console && console.log("No. " + number + " does not begin with " + prefix);
  });
  if(!validPrefix) {
    // window.console && console.log("No. " + number + " has invalid prefix");
    if(echo) alert("Invalid prefix");
    return false;
  }
  
  if(checkdigit) { // Luhn
    if(!luhn_validate(number)) {
      // window.console && console.log("No. " + number + " has invalid Luhn checksum");
      if(echo) alert("Invalid checksum");
      return false;
    } else {
      // window.console && console.log("Yes. " + number + " has valid Luhn checksum");
    }
  }
  
  // all good
  // window.console && console.log("Yes. " + number + " is valid for " + name);
  return true;
}

function cc_find(number) {
  var cards = cc_cards();
  var valid_card = null;
  $.each(cards, function(i, card) {
    if(cc_validate(card.id, number)) {
      valid_card = card;
      return false;
    }
  });
  return valid_card;
}

function cc_get_type(number) {
  var card = cc_find(number);
  if(card) {
    return [card.id, card.accepted, card.type];  
  }
  return [0, 0, "Unknown"];  
}

function createjscssfile(filename, filetype){
 if (filetype=="js"){ //if filename is a external JavaScript file
  var fileref=document.createElement('script')
  fileref.setAttribute("type","text/javascript")
  fileref.setAttribute("src", filename)
 }
 else if (filetype=="css"){ //if filename is an external CSS file
  var fileref=document.createElement("link")
  fileref.setAttribute("rel", "stylesheet")
  fileref.setAttribute("type", "text/css")
  fileref.setAttribute("href", filename)
 }
 return fileref
}

function replacejscssfile(oldfilename, newfilename, filetype){
 var targetelement=(filetype=="js")? "script" : (filetype=="css")? "link" : "none" //determine element type to create nodelist using
 var targetattr=(filetype=="js")? "src" : (filetype=="css")? "href" : "none" //determine corresponding attribute to test for
 var allsuspects=document.getElementsByTagName(targetelement)
 for (var i=allsuspects.length; i>=0; i--){ //search backwards within nodelist for matching elements to remove
  if (allsuspects[i] && allsuspects[i].getAttribute(targetattr)!=null && allsuspects[i].getAttribute(targetattr).indexOf(oldfilename)!=-1){
   var newelement=createjscssfile(newfilename, filetype)
   allsuspects[i].parentNode.replaceChild(newelement, allsuspects[i])
  }
 }
}

//replacejscssfile("oldscript.js", "newscript.js", "js") //Replace all occurences of "oldscript.js" with "newscript.js"
//replacejscssfile("oldstyle.css", "newstyle", "css") //Replace all occurences "oldstyle.css" with "newstyle.css"


function fancyAlert(msg) {
  jQuery.fancybox({
    'modal' : true,
    'content' : "<div style=\"margin:1px;width:240px;\">"+msg+"<div style=\"text-align:right;margin-top:10px;\"><input style=\"margin:3px;padding:0px;\" type=\"button\" onclick=\"jQuery.fancybox.close();\" value=\" Ok \"></div></div>"    });  
}

function fancyConfirm(msg, callback) {
  var ret;
  jQuery.fancybox({
    'modal' : true,
    'content' : "<div style=\"margin:1px;width:240px;\">"+msg+"<div style=\"text-align:right;margin-top:10px;\"><input id=\"fancyConfirm_cancel\" style=\"margin:3px;padding:0px;\" type=\"button\" value=\" Cancel \"><input id=\"fancyConfirm_ok\" style=\"margin:3px;padding:0px;\" type=\"button\" value=\" Ok \"></div></div>",
    afterShow: function() {
      //alert('complete');
      $("#fancyConfirm_cancel").click(function() {
        //alert('cancel');
        jQuery.fancybox.close();
        var ret = true;
        return ret;
      })
      $("#fancyConfirm_ok").click(function() {
        jQuery.fancybox.close();
        var ret = false;
        return ret;
      })
    },
    afterClose: function() {
      //alert("closed");
      if (typeof callback == 'function') {
        callback.call(this, ret); 
      }
    }
  });
}

function fancyConfirm_text() {
  fancyConfirm("Ceci est un test", function(ret) {
    alert(ret)
  })
}

(function($){
    $.fn.myDisableSelection = function() {
        return this
                 .attr('unselectable', 'on')
                 .css('user-select', 'none')
                 .on('selectstart', false);
    };
})(jQuery);

// jquery redirect
(function(d){d.fn.redirect=function(a,b,c){void 0!==c?(c=c.toUpperCase(),"GET"!=c&&(c="POST")):c="POST";if(void 0===b||!1==b)b=d().parse_url(a),a=b.url,b=b.params;var e=d("<form></form");e.attr("method",c);e.attr("action",a);for(var f in b)a=d("<input />"),a.attr("type","hidden"),a.attr("name",f),a.attr("value",b[f]),a.appendTo(e);d("body").append(e);e.submit()};d.fn.parse_url=function(a){if(-1==a.indexOf("?"))return{url:a,params:{}};var b=a.split("?"),a=b[0],c={},b=b[1].split("&"),e={},d;for(d in b){var g= b[d].split("=");e[g[0]]=g[1]}c.url=a;c.params=e;return c}})(jQuery);
    
// form handlers

//function save_object(form_id, obj_type, action, handler, reload, keepalive) {
function save_object(form_id, obj_type, action, options) {
  if(typeof options == "undefined") var options = {};
  var ajax_url = '/ajax.php?obj_type='+ obj_type + '&oper=' + action;
  submit_form(form_id, ajax_url, options);
  return false;
}

function bootstrap_success_message(message) {
  if(!message) return '';
  if(is_array(message)) message = message.join('<BR>'); 
  return "<div class='alert alert-success'><button type='button' class='close' data-dismiss='alert' aria-hidden='true'>&times;</button> " + message + "</div>";
}  

function bootstrap_error_message(message) {
  if(!message) return '';
  if(is_array(message)) message = message.join('<BR>'); 
  return "<div class='alert alert-danger'><button type='button' class='close' data-dismiss='alert' aria-hidden='true'>&times;</button> " + message + "</div>";
}

// generic handler called after an ajax form has been successfully submitted
// requires that handler_operation is defined (the php operation in ajax.php)
function success_handler(params, data, parent_id) {
  var target_id = params.handler_target || params.target || '';
  if(target_id) target_id = target_id.replace(/^#/, ''); // strip leading # if exists
  
  var form_id = params.form;
  var operation = params.handler_operation;
  var handler = params.handler || params.success_handler || '';
  var $target = $('#' + target_id);
  var $form = $('#' + form_id);

  console.log("success handler=" + handler);
  
  if(handler) {
    var handlers = handler.split(',');
    $.each(handlers, function(k, h) {
      if(h != 'success_handler') { // otherwise endless loop
        var funcCall = parent_id ? h  + "(params,data,parent_id);" : h  + "(params, data);";
        eval(funcCall);
      }
    });

  }

  var autosave = params.autosave;
  
  //console.log("Success handler; Got here autosave=",autosave);
  
  // call ajax operation and print result to target
  if(operation && target_id && $target.length) {
    $target.addClass("loading-big");
    var ajax_url = '/ajax.php?oper=' + operation;
    if(data) ajax_url = ajax_url + '&' + obj2qs(data);
    //alert(dump(data));
  
    $target.load(ajax_url, function(responseText) {
      $form.html('').css('border', '').hide('fast');
      $target.removeClass("loading-big");
      $target.show('fast');   
      $target.parent().show('fast'); // now ajax_messages is in hidden container, why 
      
      //$target.parent().show('fast').removeClass('hidden');   
      //$target.html(responseText); // this shouldn't be needed   
      //alert("target=" + target_id + " l=" + $target.length);
      if(autosave) { /** turn off scrolling for autosave */
        //console.log("Not scrolling");        
      } else { 
        //console.log("Scrolling to top");
        var position = $target.position();
        //var pos = target.offset().top;
        //console.log("sh:", params);
        scroll(0,position.top);
        if(!in_iframe()) $('html, body').animate({scrollTop:0}, 'slow');
      }
    });
  }
}

  
// generic function to load something into a div using a handler
// the params (object) are added as query string parameters to the handler
function load_div(handler, div_id, params, options) {
  if(typeof options == "undefined") var options = {};
  if(typeof params == "undefined") var params = {};
  if(options.dialog) {
    options.unauthenticated = true;
    options.overwrite = true;
    return generic_load_dialog(div_id, handler, params, options)
  }
      
  var $dialog = $('#' + div_id);
  if(!$dialog.length) {
    return false;
    alert("load error: could not find " + div_id);
  }
  $dialog.show('fast');      
  $dialog.addClass("loading-big");
  $dialog.html('<div class="round pad border margin_top">Loading...</div>');

  var ajax_url = '/ajax.php?oper=' + handler;
  if(params) ajax_url = ajax_url + '&' + obj2qs(params);
  //alert(ajax_url);

  $dialog.load(ajax_url, function(responseText) {   
    $dialog.removeClass("loading-big");
    var onload = options.onload;
    if(onload) {
      var funcCall = onload  + "(params);";
      eval(funcCall);
    }
  });                          
}

// generic wrapper for dialog.
// method must be defined in ajax.php
// only works if data contains at least one key/value pair (or else nothing is posted)
function generic_load_dialog(dialog_id, method, data, options) {
  if(typeof options == "undefined") var options = {};
  //if(typeof data == "undefined") var data = {};

  var $dialog = $('#' + dialog_id);
  $dialog = $dialog.length && $dialog || $('<div id="' + dialog_id + '"></div>').appendTo('body');  // append if doesn't exist
  var html = options.html ? options.html : "<div class='spinning_wheel'>Loading...</div>";  
  $dialog.html(html);
  delete options.html;
  
  var title = options.title || 'Loading...';
  var height = options.height || 700;
  var width = options.width || 500;
  var unauthenticated = options.unauthenticated;
  var overwrite = (typeof options.overwrite =="undefined") ? 1 : options.overwrite; // on by default
  var handler = options.handler;

  //if(options.data) data = $.extend(data, options.data);
  //data.options = options;
  
  $dialog.dialog({   
   autoOpen: false,
   closeOnEscape: false,
   modal: true,
   title: title,
   width: width,
   height: height
  });
  // alert(dump(data));return;
  $dialog.dialog('option', 'title', title); 
  $dialog.dialog('open'); // jquery call to show overlay
  ajax_url = "/ajax.php?oper=" + method;
  if(options) ajax_url += "&options=" + escape(array2json(options));

  //alert(ajax_url);
  //var data = [];
  //data.options = JSON.stringify(options);
  
  jQuery.ajax({
    type: "POST",
    data: data,
    url: ajax_url,
    success: function (responseText) {
      var messages = [];
      if(overwrite) {
        $dialog.html(responseText);
      } else {
        $dialog.append(responseText)
      }
      success_handler(options, data);
     }
  });
}

function enableForm(form_id) {
  var $form = $("#" + form_id);
  if($form.length) {
    $form.removeClass("disabled loading-big");
    $spf = $("#" + form_id +' input[type="submit"]' + ",#" + form_id + " .buttons, #" + form_id + " .submit_parent, #" + form_id + " .submit_parent_form");
    $spf.show('fast');
  }
}

function disableForm(form_id) {
  var $form = $("#" + form_id);
  if($form.length) {
    $form.addClass("disabled loading-big");
    $spf = $("#" + form_id +' input[type="submit"]' + ",#" + form_id + " .buttons, #" + form_id + " .submit_parent, #" + form_id + " .submit_parent_form");
  }
}

// ajax print log
function print_log(string, file, level) {
  if(typeof options == "level") var level = 0;
  var ajax_url = '/ajax.php?oper=print-log';
  jQuery.ajax({
    type: "POST",
    data: {'string': string, 'file': file, 'log_level': level},
    url: ajax_url
  });
  
}

// find or prepend form message div
function form_messages(form_id) {
  var $form = $('#' + form_id);
  if(!$form.length) return '';
  if($('#' + form_id + ' .messages').length) return $('#' + form_id + ' .messages'); // new

  if ($form.is("form")) {
    $form.prepend("<div class='messages'></div>" ); // prepend
    return  $('#' + form_id + ' .messages'); // reference to newly prepended
  } else { // not form (link?), prepend to parent
    var $msgs = $form.parent().find(' .messages'); // reference to newly prepended
    if($msgs.length) return $msgs; // exists
    $form.parent().prepend("<div class='messages'></div>" );
    return $form.parent().find(' .messages');
  }
}
   
/** load partial view href into $target from $link */
function soft_load($link, target, href, success_handler) {
  var $target = $(target);
  if(!$target.length) return true;  
  var d = new Date();
  var ts = d.getTime();    
  var link = replace_query_var(href, '_pv', 1);
  var hash = '';
  link = replace_query_var(link, 'ts', ts);

  if(link.indexOf('#') > 0) { // handle hash
    var parts = link.split('#');
    link = parts[0];
    hash = parts[1];
  }
  console.log("soft_load: uri=" + link + " hash=" + hash);
  
  $icon = $link.find('i');
  if($icon.length) $icon.addClass('fa-spinner fa-spin');
  console.log("Soft load " + href);
  $target.load(link, function() {
    if($icon.length) $icon.removeClass('fa-spinner fa-spin');
    //window.location.href = href;
    if(hash) {
      if(history.pushState) {
        history.pushState(null, null, '#' + hash);
      } else {
        location.hash = '#' + hash;
      }
    }
    
    // select active tab
    if (hash) {
      console.log("hash=" + hash);
      $('.nav-tabs a[href="'+hash+'"]').tab('show');
    }
    
    if(success_handler) {
      console.log("template_load done, calling " + success_handler);
      window[success_handler](); // eval = evil
    }
    ready_script();    
    return false;
  }); // partial view
}


function soft_reload(url, container) {
  if(typeof container == "undefined") var container = 'body';
  if(typeof url == "undefined") var url = window.location.href
  var hash = '';
  var $container = $(container);
  console.log("Loading url=" + url + " into container=" + container + " len=" + $container.length);
  if(container && $container.length) {
    console.log("Soft reload " + url);
    window.location.href = url;
    if(hash) {
      if(history.pushState) {
        history.pushState(null, null, '#' + hash);
      } else {
        location.hash = '#' + hash;
      }  
    }
    $container.load(url);
  } else { /** hard reload */
    console.log("Hard reload...");
    location.reload();
  }
}

// alert generic function to submit form via ajax
function ajaxSubmit(params) {
  if(this.submitting) {    
    //console.log("already submitting");
    //console.log(params);
    //return;
  } else {
  }
  console.log("ajaxSubmit params=", params);
  this.submitting = true;
  var self = this;
  var form_id = params.form_id;

  // extract(params);
  
  //alert(dump(params));
  var form_id =  params.form_id || params.form || '';
  var url = params.url;
  var type = params.type || 'post';
  var redirect = params.redirect;
  var reset = params.reset;
  var soft_reload = params.soft_reload || '';
  var controller = params.controller || '';
  var reload = params.reload || false;
  var reload_link = params.reload_link || '';
  var org_text = params.org_text;
  var operation = params.operation;
  var script = params.script || params.handler_url || "/ajax.php";
  
  console.log("\n\nscript=" + script + "\n\n"); 
  
  var handler = params.handler;
  var handler_data = params.handler_data;
  var keep_messages = params.keep_messages || false; 
  var keepalive = params.keepalive || false;
  var scroll = true;
  var show_message = true;
  var run_handler = true;
  if('scroll' in params) scroll = params.scroll;
  if('show_message' in params) show_message = params.show_message;
  if('run_handler' in params) {
    run_handler = params.run_handler;
    console.log("rund h=" + run_handler);
  }
  

  var $form;  
  var input_data = params.data;
  if(!form_id) {
    alert("form ID is required for ajax sumit");
    return false
  }
  
  summernote_value(form_id);
  $form = $("#" + form_id);
  disableForm(form_id);
  var form_data = $form.serialize();
  var autosave = params.autosave = $form.data('autosave') ? 1 : 0; // set by $(document).on("change", ".autosave INPUT, above

  //console.log("Form data=", form_data);
  if(input_data) {
    input_data = is_object(input_data) || is_array(input_data) ? input_data : parse_json(input_data);      
    if(input_data) form_data = jQuery.extend(form_data, input_data);
    //form_data + '&' + $.param(input_data); // doesn't check for duplicate keys      
  }
  input_data = form_data;

  $.ajax({
    type: type,
    url: url,
    data: input_data, // serializes the form's elements.
    success: function(responseText) {
      var $messages = form_messages(form_id); // get message div      
      //console.log("ajaxSubmit response: msg len=" + $messages.length);
      //console.log(responseText);
      if(org_text) $form.html(org_text); // used for links, change back to pre-loading state
      if(!keep_messages) $messages.html(''); // clean

      
      var result = parse_json(responseText);
      //console.log("result val",result);
      if(!result) {
        result = {'success':0, 'error':'invalid result: ' + responseText};
        console.log("res=", responseText);
      }
      //console.log("done parsing result=", result);
      
      var success = result.success;
      
      var update_form = result.update_form;
      var message = result.message && isArray(result.message) ? result.message.join("<BR>") : result.message || '';
      var error = result.error && isArray(result.error) ? result.error.join("<BR>") : result.error || '';
      var input = result.input;      
      var data = result.data ? result.data : {}; // convert data into object if not already and remove null values

      console.log("data before parse:", data);
      data = is_object(data) || is_array(data) ? data : parse_json(data);
      data = remove_null(data);

      if(handler_data) {
        jQuery.extend(data, handler_data);
      }
      
      if(message && show_message) {
        //console.log("Ok, msg=" + message + " custom=" + params.message); 
        if(params.message) message = params.message; 
        $messages.append(bootstrap_success_message(message)).parent().show();
      }
      
      if(error) { // show even if show_message is false ?
        $messages.append(bootstrap_error_message(error)).parent().show(); // append
      }
      
      //console.log("Scrolling to " + topoff);
      if(scroll) {
        var topoff = 0;      
        if(!params.scrolltop) topoff = $messages.offset().top || 0; // scroll to messages
        if(error && !in_iframe()) $('html,body').animate({scrollTop:topoff}, 'slow'); //$messages.animate({scrollTop:0}, 'slow');
        else if(message && !autosave) { 
          //console.log("Got here autosave, scrolltop=" + params.scrolltop + " scrolling to " + topoff);
          if(!in_iframe()) $('html,body').animate({scrollTop:topoff}, 'slow'); 
        }
      }

      if(data && update_form && $form.length) { // update the form with data returned, also on error if update_form is set
        $.each(data, function(key, val) {
          var $form_el = $('#' + form_id + ' [name=' + key + ']');
          if($form_el.length) $form_el.val(val);
          //row = row + '<td>' + val + '</td>';
          //console.log("Updating " + key + "=" + val);
        });
      }

      if(result.success == 1 && !error) {
        g_form_dirty = 0;
        
        //console.log("got here, success, params=", params);
        //console.log("got here2, success, data=", data);

        if(reset) {
          reset_form(form_id, true);
        }
        
        success_handler(params, data);

        
        if(!redirect) redirect = result.redirect; // redirect URL in result
        if(!operation) operation = result.operation; // next AJAX operation to load on success

        console.log('ajaxSubmit: success  redirect=' + redirect + " oper=" + operation);

        if(redirect) {
          //console.log("now redirecting to" + redirect);
          //$messages.append("<br>Redirecting to " + redirect).parent().show(); // overwrite
          //$messages.parent().show();
          if(input) {
            //alert("redirecting to " + redirect + " with data " + dump());return;
            $().redirect(redirect, input); // redirects, posting the data from the form
          } else {

            if (0 && in_frame()) {
              //alert("bust iframe and redirecting to " + redirect);return;
              //console.log("iframe? self=", self, " top=", top);
              //console.log("loc=" + top.location);
              
              if(top.location) top.location.replace(redirect);
            } else { 
              //alert("local redirect to " + redirect);return;
              window.location.href = redirect; // simple redirect, preserves query string params
            }
          }
        } else if(operation) {
          self.submitting = false; // but do not enable, user cannot click yet
          params.url = script + "?oper=" + operation;
          params.keep_messages = true;
          params.data = JSON.stringify(data);
          params.type = "POST"; // needed ?

          console.log("next operation=" + operation);
          //console.log("params=", params);

          ajaxSubmit(params);
          
        } else { // no operation or redirect, last step if multi-step
          enableForm(form_id);
          //var controller = "inbox";
          
          console.log('ajaxSubmit: gh soft_reload =' + soft_reload + " ctrl=" + controller + " content len=" + $("#content_container").length);
          
          if(reload_link && $(reload_link).length) {
            console.log('ajaxSubmit: last step, success: reload-link=' + reload_link + " len=" + $(reload_link).length);
            
            bootstrap_modal_hide($form); // hack to force hiding bootstrap overlay due to unknown bug

            $(reload_link).trigger("click"); // link which will trigger soft reload
          } else if(soft_reload && controller && $("#content_container").length) {
            console.log('ajaxSubmit: soft reload tpl=' + soft_reload + " ctrl=" + controller);
            
            bootstrap_modal_hide($form); // hack to force hiding bootstrap overlay due to unknown bug
            
            template_load("#content_container", soft_reload, controller, params, {});
            
            //$(reload_link).trigger("click"); // link which will trigger soft reload
          }             
          if(reload) location.reload();
          if(!keepalive && form_id) close_parent_dialog(form_id);
          //if(message) $messages.html(message).parent().show(); // overwrite
        }
        
      } else { // failure
        enableForm(form_id);
        //if(message) $messages.html(message).parent().show();
      }
      //alert(data); // show response from the php script.
      self.submitting = false;

    },
    error: function(responseText) {
      //console.log("AjaxSubmit handler: Error");
      //console.log(dump(responseText));
    }
  });
}


// for some reason, summernote does not store value in the HTML tag
function summernote_value(form_id) {
  var $summernote = $('#' + form_id + ' .summernote');
  // //console.log("Submit form: summernote len=" + $summernote.length);
  if(!$summernote.length) return;
  $.each($summernote, function (index, editor) {
     var $editor = $(editor);
     $editor.val($editor.summernote('code'));
  });                               
}

// same as save object, but then for any form (not a system object)
// TODO: change last  into optoins object
// function submit_form(form_id, ajax_url, handler, reload, keepalive, overwrite) {
function submit_form(form_id, ajax_url, options) {
  if(typeof options == "undefined") var options = {};
  var handler = options.handler;
  var reload = options.reload;
  var reload_link = options.reload_link || '';
  var reload_template = options.reload_template || '';
  var reload_target = options.reload_target || '';
  var keepalive = options.keepalive;
  var overwrite = options.overwrite;
  var parent_id = options.parent_id;
  var data = options.data;
  var unauthenticated = options.unauthenticated;
  var fields = options.fields;
  var $form = $('#' + form_id);
  var $messages = form_messages(form_id); // get message div      
  var $errors = $messages;
  //var $messages = $('#' + form_id + '_messages');
  //var $errors = $('#' + form_id + '_errors');

  //alert("submit_form: options=" + dump(options));

  summernote_value(form_id);  
  
  //alert("id=" + form_id + " len=" + $form.length + " scount= " + $summernote.length);
  //return false;
  
  //alert(ajax_url);
  if(overwrite) {
    $messages.html(bootstrap_success_message('<p class="spinning_wheel">Sending....</p>'));
  }

  var error;
  var form_data = $("#" + form_id).serialize();
  //console.log("form data=", form_data);
  
  jQuery.ajax({
    type: "POST",
    data: form_data,
    url: ajax_url,
    success: function (responseText) {
      
      //console.log("submitForm ajax-success response:",responseText);
      $messages.html('');
      var messages = [];
      //alert("response:" + responseText);
      //return;
      var result = parse_json(responseText);
      //alert("parse success success:" + dump(result));


      var success = result.success;
      var message = result.message;
      var data = result.data;
      var error = result.error;      
      if(message) messages.push(message);
      // alert("keepalive=" + keepalive + dump(result));
      if(result.success == 1) { 
        $errors.html(''); // remove errors
        
        if(handler) {
          // the data is already encoded
          // success_handler(options, array2json(data), parent_id);
          
          var funcCall = parent_id ? handler  + "(options, " + data + ", parent_id);" : handler  + "(options, " + data + ");";
          eval(funcCall);
          //alert(funcCall);                                            


          //var funcCall = parent_id ? handler  + "(" + options + "," + data + ", '" + parent_id + "');" : handler  + "(" + options + "," + data + ");";
          //eval(funcCall);
          // alert("returned from " + handler + " keepalive=" + keepalive + " reload=" + reload);
          // alert(dump(options));
        }
        if(keepalive == 1) {
          //messages.push("Added product " + data.name);
          //alert("Added product " + data.name);
        } else {
          close_parent_dialog(form_id);

          if(reload_template && reload_target && $(reload_target).length) { // soft reload using smarty template
            var controller = ''; 
            if(!controller) controller = $(reload_target).data('controller');
            //console.log('success: handler=' + handler + " reload_target=" + reload_target + " reload_template=" + reload_template + " reload_link=" + reload_link + " controller=" + controller);
          } else if(reload_link && $(reload_link).length) {
            //console.log('success: reload-link=' + reload_link);
            $(reload_link).click(); // link which will trigger soft reload
          } else {
            //console.log('success: no soft reload: reload-link=' + reload_link);
            
          }
          
          if(reload == 1) reload_page();
        }
      } else {
        //$messages.html(message);
        //alert('failure:' + message + " selector=" + selector +  " len=" + $(selector).length);
      }
      if(messages.length) {
        if(overwrite) {
          $messages.html(bootstrap_success_message(message)); // overwrite      
        } else {
          $messages.append(bootstrap_success_message(message)); // append      
        }
      }
      if(error) {
        //console.log("Error:", error);
        //console.log("Msg len:", $messages.length);
        
        $messages.html(bootstrap_error_message(error)); 
        return false;      
      } 
      if(error || messages.length) {
        // if(!in_iframe()) $('html,body').animate({scrollTop:0}, 'slow');
      }

      return true;
    },
    error: function(jqXHR, textStatus, errorThrown) {
      //console.log("submitForm HTTP error");
      //alert("error=" + errorThrown + dump(textStatus));
      alert('HTTP Error: '+errorThrown);
      // alert('Error Message: '+textStatus);
      var $errors = $('#' + form_id + '_errors');
      $errors.html(textStatus);
      return false;
    }
  });
}

function close_parent_dialog(id) {
  $el = $('#' + id);
  //console.log("close_parent_dialog: id=" + id)
  if(!$el.length) return true;
  close_parent($el);
  return false;
}

function close_parent($el) {
  $dialog = $el.closest('.ui-dialog-content');
  //$fancybox = $el.closest('.fancybox-overlay'); 
  $fancybox = $el.closest('.fancybox-wrap'); 
  $modal = $el.closest('.modal');
  //console.log("close parent id=" + $el.attr("class") + " class=" + $el.attr("class") + " len=" + $el.length + " jq-dialog=" + $dialog.length+ " fb-dialog=" + $fancybox.length);
  //console.log("close parent modal len=" + $modal.length);
  //console.log("el=", $el.attr('id'));
  //console.log("caller is " + arguments.callee.caller.toString());
  if($dialog.length) { // jquery UI dialog
    $dialog.dialog('close');    
    return false;
  } else if($fancybox.length) { // fancybox ?
    $.fancybox.close();
    return false;
  } else if($modal.length) {
    //console.log("hiding modal");
    $modal.modal('hide');
  }
   
  return true; // will cause normal link to be handled
}

function bootstrap_modal_hide($el) {
  var len = 0;
  var $backdrop = $('.modal-backdrop');
  
  if(len = $backdrop.length) {
    console.log("afr.js bootstrap_modal_hide() called, found " + len + " backdrops");

    $backdrop.removeClass('in').addClass('out');
    
  }

  if(!$el || $el.length) {
    console.log("afr.js: bootstrap_modal_hide() called, but $el parameter was empty");
    return;
  }
  
  $modal = $el.closest('.modal');

  console.log("afr.js: bootstrap_modal_hide() called");

  if(len = $modal.length) {
    console.log("afr.js: bootstrap_modal_hide(): found " + len + " modals, hiding modals");

    $modal.modal('hide');
    
  } else {
    console.log("afr.js: bootstrap_modal_hide(): no modal found");
  }
}


// Begin fileuploader
// copy relevant options from our uploader options to those for jquery-fileupload
function jq_uploader_options(options) {
  var uploader_options = {};
  if(options.allowed_extensions) uploader_options.acceptFileTypes = new RegExp('(\.|\/)(' + options.allowed_extensions.join('|') +')', 'i');

  uploader_options.maxFileSize = options.maximum_file_size || 0;
  uploader_options.minFileSize = options.minimum_file_size || 0;
  uploader_options.maxNumberOfFiles = options.allow_multiple ? options.maximum_files : 1;

  return uploader_options;
}

/** Return array of uploaded files */
function jfu_file_list($target) {
  var files = [];
  var file = {};
  var $item;
  var $items = $target.find('LI.template-download');
  //console.log("jfu_file_list: items=", $items);
  $.each($items, function(i, item) {
    $item = $(this);
    file = $item.data();
    file.id = $item.attr('id');
    files.push(file);
  });
  //console.log("jfu_file_list: files=", files);
  return files;
}

// new uploader, using Blueimp jquery-fileupload
function init_fileuploader($target, loadData, extra_options) {
  if(typeof extra_options === 'undefined') var extra_options = {}; // overrides
  if(typeof loadData === 'undefined') loadData = true;
 
  var ui = $target.data('ui') || 'basic';
  var options = $target.data('options') || {};
  options = $.extend(options, extra_options);
  
  var view = $target.data('download_template') || $target.data('view') || options.view || "list"; // grid (compact) or list
  if(!(view == "grid" || view == "list")) view = "list";
  
  var mediaData = $target.data('media_data') || {};
  
  $target.data('initialized', 1);
  $target.data("upload-count", 0); // number of files in the upload queue
  $target.data("download-count", 0); // number of files in the download table (filled in later by jfu_load)
  $target.data("send-count", 0); // number of files being sent
  $target.data('view', view);
  
  jfu_buttons($target, ui); // append buttons
  if(tip = $target.data('tip')) {
    jfu_tip($target, tip); // tooltip
  }
  
  var disabled = $target.hasClass('disabled') ? true : false;
  var div_id = $target.prop('id');
  //  alert("id=" + div_id);

  // defaults
  var extensions = options.allowed_extensions || ['jpeg', 'jpg', 'gif', 'png'];
  var endpoint = options.endpoint || '/ajax.php?oper=upload';
  var allow_multiple = options.allow_multiple || false;
  var title = options.title || 'Click or Drop';
  var sizeLimit = options.maximum_file_size || 10 * 1024 * 1024; // 10M default

  var resize = options.resize || {};
  var handler_options = $target.data('handler_options') || {};
  var handler = handler_options.handler || $target.data('handler') || '';
  var target_field = options.target_field || '';
  

  //console.log("init_uploader for " + div_id + " handler=" + handler + " view=" + view + " extra options="); // + " data=" + dump(mediaData) + " options=" + dump(options));
  //console.log(extra_options);
  //console.log(options);
  //console.log("extension=");//console.log(extensions);
  //console.log("media data=");//console.log(mediaData);
  delete options.media_data;
  
  //if(!mediaData.parent_id) {
  //  var parent_id = $target.data('parent_id');
  //  //console.log("No parent ID, looking at element, found:" + parent_id);
  //  mediaData.parent_id = parent_id;
  //}

  var endpoint = '/ajax.php?oper=jq-file-upload';
  var params = {
      options: JSON.stringify(options),
      mediaData: JSON.stringify(mediaData)
    };
           
  if(params) endpoint += '&' + obj2qs(params); // this shouln't be needed as params below should handle this. Doesn't work for some reason      

  //console.log("Our options:");
  //console.log(options);

  var uploader_options = jq_uploader_options(options);
  
  console.log("Endpoint options:", options);
  console.log("Uploader options:", uploader_options);
  //console.log(uploader_options);
  
  /** todo: turn off local_resize, set imageMaxWidth/Height in options */
  var localResize = /Android(?!.*Chrome)|Opera/.test(window.navigator && navigator.userAgent);
  var crop = false;
  
  console.log("local resize=" + localResize);
    
  $target.fileupload({
    // Uncomment the following to send cross-domain cookies:
    //xhrFields: {withCredentials: true},
    url: endpoint,
    options: uploader_options,
    acceptFileTypes: uploader_options.acceptFileTypes,
    maxFileSize: uploader_options.maxFileSize,
    minFileSize: uploader_options.minFileSize,
    maxNumberOfFiles: uploader_options.maxNumberOfFiles,
    downloadTemplateId: "template-download-" + view,    
    disableImageResize: localResize,

    imageMaxWidth: 1920,
    imageMaxHeight: 1920,
    imageCrop: crop, // Force cropped images
    
    dropZone: $(this),
    autoUpload: false
   }).on('fileuploadchange', function (e, data) {
     if(is_object(data)) {
       var valid_count = 0;
       var $files = $target.find("TABLE TBODY.files TR");
       //console.log('fileuploadchange rows=' + $files.length);
       var er = data.files.error;
       $.each(data.files, function (index, file) {
         if(!file.error) valid_count++;
       });
     } else {
       var org = data;
       data = {};
       data.files = [];
       data.files.error = data;
     }
     //console.log('len=' + data.files.length + ' valid files=' + valid_count + " gler=" + er + " fer=" + data.files[0].error + " 2nd=" + (data.files.length > 1 ? data.files[1].error : ' none'));
     
   });

   $(document).on('click', "INPUT.check-all, INPUT[type='checkbox'][name='delete']", function() {
     var $target = $(this).closest('.jquery-fileupload');
     console.log("delete click, wait target=" + $target.attr('id'));
     setTimeout(function(){console.log("run");jfu_del_update($target); }, 30);     
   });
   
   
   $target.on('fileuploaddestroyed', function(e, data) { 
     //console.log("fileuploadcompleted: data=");
     //console.log(data);

     //$target.data("download-count", $target.data("download-count") - 1);
     jfu_ui_update($target);
     
     var up_count = $target.data("upload-count") || 0;
     var down_count = $target.data("download-count") || 0;
       
     //console.log("fileuploaddestroyed: up=" + up_count + " down=" + down_count);

   });      
       
  $target.on('fileuploadprocessdone', function (e, data) {
      
      console.log("fileuploadprocessdone:");
      //console.log(data.exif.getAll());
  });

   $target.on('fileuploadcompleted', function(e, data) {
     var result = data ? data.result : {};
     var success = result.success;
     var error = result.error;
     console.log("fileuploadcompleted: success=" + success + " error=" + error);
     //if(error) console.log("fileuploadcompleted: error=" + error);
     var file = data.files ? data.files[0] : null;
     console.log("fileuploadcompleted v2: data=",data);
     //console.log(file);

     if(error) {
       var $messages = form_messages(div_id); // get message div
       console.log("msg len=" + $messages.length);
       $messages.append(bootstrap_error_message(error)).parent().show(); // append       
     } else if(file) { // file successfully uploaded 
       var file_error = file.error;
       
       if(!file_error) $target.data("send-count", Math.min($target.data("send-count") - 1, 0));

       //if(!error) jfu_ui_update($target);

       var upload_count = $target.data("upload-count");
       var send_count = $target.data("send-count");

       //console.log("fileuploadcompleted with file: error=" + error + " send count=" + send_count + " upload count=" + upload_count);
       if(upload_count >= 0) {
         //console.log("calling load");
         jfu_load($target); // calls sortable, adds edit buttons
       }
       
     }
     
     var up_count = $target.data("upload-count") || 0;
     var down_count = $target.data("download-count") || 0;
     var handler_options = $target.data('handler_options') || {};
     var handler = handler_options.handler || $target.data('handler') || '';
       
     console.log("\n\n---\n\nfileuploadcompleted: up=" + up_count + " down=" + down_count + " handler=" + handler + " data=", data);
     var uploaded_files = jfu_file_list($target);
     if(uploaded_files.length && handler && !error) {
       data = {"files": uploaded_files}            
       console.log("fileuploadcompleted calling " + handler + " with ", data);
       success_handler(handler_options, data);
     }

   });      

   $target.on('fileuploadstart', function(e, data) { 
     console.log("fileuploadstart");
     console.log(data);
   });      

   $target.on('fileuploadsend', function(e, data) { 
     
     $target.data("send-count", $target.data("send-count") + 1);
     jfu_ui_update($target);
     
     //console.log("fileuploadsend");
     //console.log(data); 
   });      

   $target.on('fileuploadadd', function(e, data) { 
     
     //console.log("fileuploadadd data=", data);

     // Note: for this to work delete the line "delete file.error" in jquery.fileupload-validate.js;
     //data.files[0].error = "Foobar error";
     //data.files.error = true;
     //data.messages = [error];
 
     var file = data.files[0];
     //console.log("fileuploadadd file=");
     //console.log(file);
     var type = file ? file.type : '';
     if(type.indexOf('image') == 0) {
       file_get_dimensions(file, options, function(file, options) {
          
         //console.log("Done with aysnc dimension check dim=" + file.width + "/" + file.height);
         //console.log("Options=");
         //console.log(options);
         //console.log(file);
         file = file_check_dimensions(file, options);
         data.files[data.index] = file;
         //console.log("calling check_file: error= " + file.error + " file=");
         //console.log(file);
       
         var error = file.error; 
         if(error) data.files.error = true;
         //console.log("fileuploadadd: Adding file: error =" + error);
         
         //console.log(file);        
         data.process().done(function () {
           //console.log("Done processing"); 
           //data.submit();
         });
       });
     }
      //validation.done(function(e, data) {
     //    //console.log("validation done");
     //});      
   });      
  
   $target.on('fileuploadfail', function(e, data) {
     console.log("fileuploadfail: Fail to add file: data=", data);
     if(is_object(data) && data.files) {
       var file = data.files[0];
       var error = file.error;
     } else {
       var error = data;
     }
   });      

   $target.on('fileuploadfailed', function(e, data) {
     //var file = data.files[0];
     //var error = file.error;
     //console.log(data);     
     //if(1) { // removed from upload queue or download table ?
     //  //if(!error) $target.data("upload-count", $target.data("upload-count") - 1);
     //  var count = $target.data("upload-count") || 0;
     //} else {
     //  //if(!error) $target.data("download-count", $target.data("download-count") - 1);
     //  var count = $target.data("download-count") || 0;
     //}
     //console.log("fileuploadfailed: Failed to add file: error=" + error + " count=" + count);
     jfu_ui_update($target);
     
   });      
   
   $target.on('fileuploadadded', function(e, data) { // only called after successful add with no errors
     var upload_count = jfu_upload_count($target);
     
     //var upload_count = $target.data("upload-count") || 0;
     
     var file = data.files[0];
     //var mediaData = $target.data('media_data') || {};     
     //file = file_get_exif(file, mediaData);
     
     var error = file.error;
     //console.log(data);
     //console.log("fileuploadadded: Added file: count =" + upload_count + " data=", data);
     jfu_ui_update($target);
     
   });      
   
   if(handler) {
     $target.on('fileuploaddestroyed', function(e, data) {       
       console.log("upload destroyed");

       var uploaded_files = jfu_file_list($target);
       if(uploaded_files.length && handler) {
         data = {"files": uploaded_files}            
         console.log("fileuploaddestroyed calling " + handler + " with ", data);
         success_handler(handler_options, data);
       }
       
       //success_handler(handler_options, data);
     });
   }

  //console.log($target.fileupload());
  
  // Enable iframe cross-domain access via redirect option:
  $target.fileupload(
    'option',
    'redirect',
    window.location.href.replace(
      /\/[^\/]*$/,
      '/cors/result.html?%s'
    )
  );   
  
  // Load existing files:
  $target.addClass('fileupload-processing');
  if(loadData) {
    //console.log("jquery_file_uploader: loading data as loadData=" + loadData);
    jfu_load($target, mediaData, false);
  }

}                                   

function jfu_download_count($target) {
  $file_cont = $target.parent().find('.file-table tbody');
  $files = $file_cont.find('.template-download');
  //console.log("File table download file count=" + $files.length);
  var download_count = $files.length;     
  return download_count;
}

// Not sure why, but this button stopeed working
// workaround, trigger click of individual upload buttons
$(document).on('click', 'BUTTON.upload-all', function() {
  var $buttons = $(".template-upload button.start");
  console.log("Found " + $buttons.length);
  $buttons.trigger("click");
   
});


function jfu_upload_count($target) {
  $file_cont = $target.parent().find('.file-table tbody');
  $files = $file_cont.find('.template-upload');
  //console.log("File table upload file count=" + $files.length);
  var upload_count = $files.length;     
  return upload_count;
}

function jfu_delete_all() {
  var $target = $(this).closest('.jquery-fileupload');
  console.log("del all tlen=" + $target.length);
  jfu_del_update($target, true);
}

function jfu_del_update($target, del) {
  var $cb = $("INPUT[type='checkbox'][name='delete']");
  var $checked = $("INPUT[type='checkbox'][name='delete']:checked");
  var checked_count = $checked.length;
  console.log("CB len= " + $cb.length + " Checked len=" + checked_count + " target=" + $target.attr('id'));
  var $delete_button = $target.find('BUTTON.delete-all');
  if(checked_count > 0) { // if more than one, make sortable and show Delete-all button on top
    $delete_button.removeClass("hidden").show();
    if(del) {
      console.log("Deleting " + checked_count);
      $.each($checked, function() {
        var $cb = $(this);
        var ajax_url = $(this).data('url');
        if(ajax_url) {
          console.log("Deleting URL=" + ajax_url);
          $.ajax({ url: ajax_url}).done(function() {
            var $el = $cb.closest("LI");              
            console.log("Done. Removing LI len=" + $el.length);
            $el.remove()
          });
        }
      });
    }
  } else {
    $delete_button.addClass("hidden").hide();      
  }
}

// show hide UI buttons depending on settings (ui) and file count (upload/download)
function jfu_ui_update($target) {
  var ui = $target.data("ui");
  var mediaData = $target.data('media_data') || {}
  
  var upload_count = jfu_upload_count($target);
  var download_count = jfu_download_count($target);
  var send_count = $target.data('send-count');

  $target.data("upload-count", upload_count || 0);
  $target.data("download-count", download_count || 0);
  
  $('[data-toggle="confirmation"]').confirmation({
    //'singleton':true      
  });
  
  if(ui != "full") return; 
  
  var $upload_button = $target.find('BUTTON.upload-all');
  var $cancel_button = $target.find('BUTTON.cancel-all');
  var $check_all = $target.find('INPUT.check-all');
  var $view_buttons = $target.find('.jfu-switch-view');

  $file_cont = $target.parent().find('.file-table tbody');
  $checkboxes = $file_cont.find('INPUT.toggle');

  if(send_count > 1) {
    $cancel_button.removeClass("hidden").show();      
  } else {
    $cancel_button.addClass("hidden").hide();      
  }             
  
  var show_view_buttons = false;

  

  
  if(download_count > 1) { // if more than one, make sortable and show Delete-all button on top
    $check_all.removeClass("hidden").show();      
    $checkboxes.removeClass("hidden").show();
    show_view_buttons = true;
  } else {
    //console.log("hiding delete button");
    // $delete_button.addClass("hidden").hide();      
    $check_all.addClass("hidden").hide();      
    $checkboxes.addClass("hidden").hide();      
  }

  if(upload_count) show_view_buttons = false;
  
  if(upload_count > 1) { // if more than one, show upload-all button on top
    $upload_button.removeClass("hidden").show();      
  } else {
    $upload_button.addClass("hidden").hide();      
  }
  
  if(show_view_buttons) {
    var view = $target.data("view");
    $view_buttons.removeClass("hidden").show();
    if(view) $(".jfu-switch-view.jfu-view-" + view).removeClass("btn-default").addClass("btn-positive");
    
  } else {
    $view_buttons.addClass("hidden").hide();      
  }

  var view = $target.data("view") || 'list';
  
  var download_count = $target.data("download-count");
  if(download_count > 1) {
    var sortable_options = {'handle': '.handle'};
    var handler_options = $target.data('handler_options') || {};
    var handler = handler_options.handler || $target.data('handler') || '';
    sortable_options.handler = handler;                         
    sortable_options.handler_options = handler_options;
    var $parent = $target.parent();
    if(view == 'list') {
      sortable_options.helper = fixHelper; // needed so table row won't shrink
      $file_cont = $parent.find('.file-table tbody');
    } else { // grid view
      //sortable_options.helper = fixHelper;
      $file_cont = $parent.find('.file-table UL.file-container');
    }
        
    if(mediaData.inline) {
      sortable_options.inline = 1;
      sortable_options.obj_type = mediaData.parent_type;
      sortable_options.obj_id = mediaData.parent_id;
      sortable_options.obj_field = mediaData.parent_field;
    }

    if(upload_count > 0) {
      //$("html, body").animate({ scrollTop: $(document).height() }, "fast");
      if(0) { // swap upload/download template
        $(".template-upload").insertBefore("#template-download-row");
      } else { // scroll to download template
        $('html, body').animate({
            scrollTop: $("#template-download-row").offset().top + $("#template-download-row").height() 
        }, 100);                                                                                                
        console.log("Scrolling to download row len=" + $("#template-download-row").length + " upload_count=" + upload_count + "top=" + $("#template-download-row").offset().top + " height=" + $("#template-download-row").height());
      }        
    }
    
    console.log("Target:" + $target.attr('id') + " parent=" + $parent.attr('class') + " dl count=" + download_count + " file_cont=" + $file_cont.length);
    //console.log("Media Data:", mediaData);
    //console.log("Sortable options:", sortable_options);
    sortable_object_list($file_cont, sortable_options);
  }                 
}

// makes a list sortable
function sortable_object_list($target, options) {
  
  if(!$target.length) {
    console.log("sortable_object_list: Missing target. Options:", options);
    return false;
  }
    
  // there's the gallery and the trash
  if(typeof options === 'undefined') var options = {};
  var obj_type = options.obj_type || 'media';
  var obj_id = options.obj_id || 0;
  var obj_field = options.obj_field || '';
  
  var inline = options.inline ? 1 : 0;

  if(inline) obj_type == 'dummy';
  
  var container = options.container || 'parent';
  if(container && !$(container).length) container = 'parent';
  var handler = options.handler || $target.data('handler') || '';
  var handler_options = options.handler_options || $target.data('handler_options') || {};
  handler_options.handler = handler;
  var axis = options.axis || '';
  
  var defaults = {
    containment: container,
    cursor: 'move',
    update: function(event, ui) {
      var files = [];
      var id, file, $item, href, href_str;
      var ar = [];
      var imgArray = $(this).sortable("toArray");
      var ids = [];
      var hrefs = [];
      //var ranks = new Array();
      var prefix = inline ? "dummy_" : obj_type + '_';
      console.log(imgArray);
      $.each(imgArray, function(index, value) {
        $item = $("#" + value);
        
        ar = value.split("-");
        obj_type = ar[0];
        obj_id = ar[1];
        
        console.log("sortable: obj_type=" + obj_type + " obj_id=" + obj_id);
        id = value.replace(prefix, "");
        file = {};
        file.id = id = file.obj_id = obj_id;
        file.obj_type = obj_type;
        file.index = index;
        file.value = value;

        href = '';
        if(id && $item.length) {
          file.type = $item.data('type');
          file.url = href = $item.data('url');
          file.obj_type = obj_type;
          file.obj_id = obj_id;
          
          file.item = $item;
          console.log("href of id=" + id + " = " + href);
        } else {
          file.type = $(this).attr('type');
          file.url = href = $(this).attr('src');
          console.log("href of item = " + href + " item=", value);
          
        }
        
        
        //rank = parseInt(index) + 1;
        if(href) hrefs.push(href);
        if(id) ids.push(id);
        if(file) files.push(file);
        //ranks.push(rank);
      });
      var first_src = $("#"+ids[0]+"").prop("src");
      if($('#title_photo_' + id).length) $('#title_photo_' + id).prop("src", first_src);
      var ids_str = ids.toString();
      var href_str = hrefs.toString();
      
      //var ranks_str = ranks.toString();
      //$.ajax({ url: "/ajax.php?obj_type=" + obj_type, type: "POST", data: "obj_type="+obj_type+"&oper=edit&data_field=rank&id="+ ids_str + "&rank="+ranks_str });
      if(inline) {
        if(hrefs.length && obj_id && obj_field) { 
          ajax_url = "/ajax.php?oper=save&obj_type=" + obj_type + "&id=" + obj_id + "&" + obj_field + "=" + encodeURIComponent(href_str);
        } else {
          console.log("sortable_object_list inline: no data, returning...");
          return;
        }
      } else {      
        ajax_url = "/ajax.php?oper=rank_update&obj_type=" + obj_type + "&id_list=" + ids_str;
      }
      console.log("Rank update url=" + ajax_url);
      
      //return;
        //console.log("rank update completed handler=" + handler);
      
      $.ajax({ url: ajax_url}).done(function() {
        //console.log("rank update completed handler=" + handler);
        var data = {};
        data.files = files;
        if(handler) success_handler(handler_options, data);
      });;
      
    }
    //cancel: 'a.ui-icon',// clicking an icon won't initiate dragging
    //revert: 'invalid', // when not dropped, the item will revert back to its initial position
    //helper: 'clone',
  }
  options = $.extend(defaults,options);

  //console.log("sortable: target length=" + $target.length + " ot=" + obj_type + " options=");
  //console.log(options);
  //console.log($target);

  // make the target be sortable
  $target.disableSelection();
  $target.sortable(options);
}

function jfu_load($target, mediaData, call_handler) {
  if(typeof mediaData === 'undefined') var mediaData = $target.data('media_data') || {};
  if(typeof call_handler === 'undefined') var call_handler = true;
                       
  var pid = mediaData.parent_id;
  var pt = mediaData.parent_type;
  var pf = mediaData.parent_field;
  var inline = mediaData.inline ? 1 : 0;
  
  var options = $target.data("options") || {};
  var edit_url = options.edit_url;
  var edit_target = options.edit_target;
  var url = '/ajax.php?oper=jq-get-media&parent_type=' + pt + '&parent_id=' + pid + '&parent_field=' + pf + '&inline=' + inline;
  if(edit_url) url = url + "&edit_url=" + encodeURIComponent(edit_url);
  if(edit_target) url = url + "&edit_target=" + encodeURIComponent(edit_target);
  
  //console.log("Load existing files from " + url + " target id=" + $target.attr("id"));
  $.ajax({
    // Uncomment the following to send cross-domain cookies:
    //xhrFields: {withCredentials: true},
    url: url,
    dataType: 'json',
    context: $target[0]
  }).always(function () {
    $(this).removeClass('fileupload-processing');
  }).done(function (result) {
    $("table tbody.files").empty();
    //console.log("result=", result);
    //console.log("this=", this);
    
    $(this).fileupload('option', 'done')
      .call(this, $.Event('done'), {result: result});
    
    $file_cont = $target.parent().find('.file-table tbody');
    $files = $file_cont.find('TR.template-download');
    //console.log("File table file count=" + $files.length);

    $target.data("download-count", $files.length);
    jfu_ui_update($target)

    var options = $target.data("options") || {};

    var handler_options = $target.data('handler_options') || {};
    var handler = handler_options.handler || $target.data('handler') || '';
    //console.log("jfu_load handler=" + handler + " handler options=");
    //console.log(handler_options);
    
    if(handler && call_handler) { // handler is stored in handler_options.handler      
      //console.log("jfu_load: Calling handler: " + handler);
      result.files = $files;
      success_handler(handler_options, result);
      //success_handler(handler_options, false); /** does this break vpatina ? */
    }

  })

}

function jfu_tip($uploader, tip) {
  var version = $uploader.data("version") || 'bootstrap';
  var $upload_button = $uploader.find('.fileinput-button');
  if(!tip) return;
  //console.log("Appending tip to button with length =" + $upload_button.length);
  var tooltip = '<i class="glyphicon glyphicon-question-sign uploader-tip" data-toggle="tooltip" data-html="true" data-placement="right" title="' + tip + '"></i>';
  //var tooltip = '\
  //  <button type="button" class="btn btn-primary tooltip-bs" data-toggle="tooltip" data-html="true" data-placement="right" \
  //    title="' + tip + '">\
  //    <i class="glyphicon glyphicon-question-sign"></i>\
  //  </button>\
  //';
  $upload_button.append(tooltip);
  $('.uploader-tip').tooltip({container: 'body'}); // bootstrap
}

function jfu_buttons($uploader, ui) {
  var version = $uploader.data("version") || 'bootstrap';
  //var button_class = ui == 'full' ? '' : 'hidden ';
  var options = $uploader.data("options") || {};

  //console.log("jfu_buttons options=", options);
  // buttons to switch view; only if multiple files are allowed and template not set
  var view_buttons = options.allow_multiple && !$uploader.data("download_template") ? 
  '<button type="button" class="jfu-switch-view jfu-view-grid margin-right margin-bottom btn btn-default" data-template="grid" title="Grid view"><i class="black fa fa-th" aria-hidden="true"></i></button>' +
  '<button type="button" class="jfu-switch-view jfu-view-list margin-right margin-bottom btn btn-default" data-template="list" title="List view"><i class="black fa fa-list" aria-hidden="true"></i></button>' : '';
  
  var output = '';
  if(version == 'bootstrap')  { // bootstrap version
    var button_bar = '\
    <div class="row fileupload-buttonbar">\
      <div class="col-lg-7">\
        <span class="btn btn-success fileinput-button">\
          <i class="glyphicon glyphicon-plus"></i>\
          <span>Upload</span>\
          <input class="file-input" type="file" name="files[]" multiple>\
        </span>\
        <button type="submit" class="hidden upload-all btn btn-primary start">\
          <i class="glyphicon glyphicon-upload"></i>\
          <span>Start upload</span>\
        </button>\
        <button type="reset" class="hidden cancel-all btn btn-warning cancel">\
          <i class="glyphicon glyphicon-ban-circle"></i>\
          <span>Cancel upload</span>\
        </button>\
        <button type="button" class="hidden delete-all btn btn-danger delete" data-toggle="confirmation" title="Sure?" data-on-confirm="jfu_delete_all">\
          <i class="glyphicon glyphicon-trash"></i>\
          <span>Delete</span>\
        </button>\
        <input type="checkbox" class="check-all hidden toggle">\
        <span class="fileupload-process"></span>\
      </div>\
      <div class="col-lg-5 fileupload-progress fade">\
        <div class="progress progress-striped active" role="progressbar" aria-valuemin="0" aria-valuemax="100">\
          <div class="progress-bar progress-bar-success" style="width:0%;"></div>\
        </div>\
        <div class="progress-extended">&nbsp;</div>\
      </div>\
    </div>';
    
    var file_table = view_buttons + '\
    <table role="presentation" class="file-table table table-striped"><tbody class="files"></tbody></table>';
    
    output = file_table + button_bar;
    
  } else { // jquery version
    output = '\
    <div class="fileupload-buttonbar">\
        <div class="fileupload-buttons">\
            tip\
            <span class="fileinput-button">\
                <span>Upload</span>\
                <input type="file" name="files[]" multiple>\
            </span>\
            <button type="submit" class="hidden upload-all start">Start upload</button>\
            <button type="reset" class="hidden cancel-all cancel">Cancel upload</button>\
            <button type="button" class="hidden delete-all delete">Delete</button>\
            <input type="checkbox" class="hidden toggle">\
            <span class="fileupload-process"></span>\
        </div>\
        <div class="fileupload-progress fade" style="display:none">\
            <div class="progress" role="progressbar" aria-valuemin="0" aria-valuemax="100"></div>\
            <div class="progress-extended">&nbsp;</div>\
        </div>\
    </div>' + view_buttons + '\
    <table role="presentation"><tbody class="files"></tbody></table>\
';
  }
//' + view_buttons + '\

  //console.log("Adding Buttonbar to id=" + $uploader.attr("id") + " class=" + $uploader.attr("class") + " len=" + $uploader.length);
  
  $(output).appendTo($uploader);
  var gallery = '\
  <!-- The blueimp Gallery widget -->\
  <div id="blueimp-gallery" class="blueimp-gallery blueimp-gallery-controls" data-filter=":even">\
      <div class="slides"></div>\
      <h3 class="title"></h3>\
      <a class="prev">‹</a>\
      <a class="next">›</a>\
      <a class="close">×</a>\
      <a class="play-pause"></a>\
      <ol class="indicator"></ol>\
  </div>\
  ';
  $uploader.after(gallery);
}

function file_check_dimensions(file, options) {
  //console.log("file_check_dimensions dim=" + file.width + "/" + file.height);
  //console.log(options);
  if ($.type(file.width) === 'number' && $.type(file.height) === 'number') {
    var w = file.width;
    var h = file.height;  
    var size = Math.max(h, w);
    var long_side = size; 
    var short_side = Math.min(h, w);
    
    // below should be corresponse to PHP check_uploaded_file in db_object    
    var minH = options.minimum_width || 0;
    var maxH = options.maximum_width || 0;
    var minW = options.minimum_height || 0;
    var maxW = options.maximum_height || 0;
    var minS = options.minimum_size || 0;
    var maxS = options.maximum_size || 0;

    if(maxH && h > maxH) file.error = "Maximum height = " + maxH;
    else if(minH && h < minH) file.error = "Minimum height = " + minH;
    if(maxW && w > maxW) file.error = "Maximum width = " + maxW;
    else if(minW && w < minW) file.error = "Minimum width = " + minW;
    else if(minS) {
      //console.log("min Size is set ");
      
      if(is_array(minS)) {
        var min1 = minS[0];
        var min2 = minS[1];
        var min_short = Math.min(min1,min2);
        var min_long = Math.max(min1,min2);
        //console.log("min Size is array short = " + min_short + " long=" + min_long + " short=" + short_side + " long=" + long_side);
        if(short_side < min_short && long_side < min_long) { 
          file.error = "Short side must be at least " + min_short + "px or long side must be at least " + min_long + "px";              
          //errors[]= "This= short_side x long_side px";
        }
      } else {
        if(size < minS) file.error = "Minimum size = " + minS + "px";
      }
    } else if(maxS) {
    }

    //console.log("max H=" + maxH + " W=" + maxW + " S=" + maxS);
    //console.log("min H=" + minH + " W=" + minW + " S=" + minS);
    //console.log("min Size=");
    //console.log(minS);
    //console.log("error=" + file.error);

  }
  return file;
}
// asynchronous function to get file_dimensions: works w/o exif
function file_get_dimensions(file, settings, callback) {
  var fr = new FileReader;
  fr.onload = function() { // file is loaded
    var img = new Image;

    img.onload = function() {
      //console.log("file_get_dimensions dim=" + img.width + "/" + img.height);
      file.height = img.height;
      file.width = img.width;
      file = callback(file, settings);
    };

    img.src = fr.result; // is the data URL because called with readAsDataURL
  };
  
  fr.readAsDataURL(file); // I'm using a <input type="file"> for demonstrating
}

// reads exif
function file_get_exif(file, mediaData) { // By Kjetil
  //console.log("file_get_exif for " + file.name);
  pid = mediaData.parent_id;
  pt = mediaData.parent_type;
  pf = mediaData.parent_field;
  var ajax_url = '/ajax.php?oper=jq-save-exif&parent_type=' + pt + '&parent_id=' + pid + '&parent_field=' + pf;
  if(!(pid && pt && pf)) {
    //console.log("file_get_exif: no media data, exiting");
  }

  loadImage.parseMetaData(
      file,
      function (data) {
        if(data.exif && Object.size(data.exif)) {
          var allTags = data.exif.getAll();
          //console.log("exif=");
          //console.log(allTags);
          //$.getJSON(ajax_url + "&exif=" + JSON.stringify(allTags), function(json) {
          //  //console.log("file_get_exif: saved to " + ajax_url + " with result");              
          //  //console.log(json);              
          //});
          file.exif = data.allTags;
          var resX = data.exif.getText("PixelXDimension");
          var resY = data.exif.getText("PixelYDimension");
          //console.log("exif data");
          //console.log(file.exif);
          if(resX > 0 && resY > 0) {
            file.width = resX;
            file.height = resY;
            //console.log("exif size for file " + file.name + "=" + resX + "/" + resY);
          } else {
            file.width = 0; // for easier checking later
            file.height = 0;
            //console.log("no exif size for file " + file.name);
          }
        } else {
          file.width = 0; // for easier checking later
          file.height = 0;
          //console.log("no exif for " + file.name);
        }
      },
      {
      }
  );
  return file;
}
                
// End fileuploader

// Return a helper with preserved width of cells
// That way draggable table rows keep original width during dragging
var fixHelper = function(e, ui) {
  ui.children().each(function() {
    $(this).width($(this).width());
    $(this).height($(this).height());
  });
  return ui;
};


// link that shows popup populated by content from smarty template
$(document).on("click", ".template-popup", function() {
  var $link = $(this);
  var template = $link.data('template');
  var ajax_url = "/ajax.php?oper=fetch-template&template=" + template;

  if(!template) return true;
  console.log("Template=" + template);
  console.log("URL=" + ajax_url);
  
  $.get(ajax_url, {}, function(responseText) {
    console.log("Response=" + responseText);
    $.magnificPopup.open({
      items: {
        src: responseText,
        type: 'inline'
      }
    });
  });
  
  return false;
  
});

$(document).on("click", ".close-collapse", function() {
  var $link = $(this);
  var target = $link.data('target');
  var $target = $(target);
  if($target.length) {
    //console.log("Closing collapse " + target + " len=" + $target.length);
    $target.collapse('hide');
  }
});


$(document).on("click", ".close-modal", function() {
  var $link = $(this);
  var target = $link.data('target');
  var $target = $(target);
  if($target.length) {
    //console.log("Closing modal " + target + " len=" + $target.length);
    $target.modal('hide');
  }
});
   
// populate form with data and resubmit
// target can be id or #selector
$(document).on("click", ".form-resubmit", function() {
  var $link = $(this);
  var target = $link.data('target');
  var data = $link.data('data') || '';
  var $target = $(target);
  
  /** not (yet) in use - and implementation not done
  console.log("Form resubmit " + target + " len=" + $target.length + " data=", data);
  if(!data && $target.data('default')) {
    var def = $target.data('default');
    console.log("Default for target " + target + " len=" + $target.length + " data=", data, " default=" + def);
  }
  */
  
  if($target.length && data) {
    //console.log("Resubmitting form " + target + " data=", data);
    populate(target, data);
    $target.submit();
    return false;
  }
  return true;
});


function ltrim(str, ch) {
  if(str.charAt(0) == ch) return str.substring(1);
  return str;
}
  

// serialize a form into object
// http://stackoverflow.com/questions/1184624/convert-form-data-to-javascript-object-with-jquery
$.fn.serializeObject = function() {
  var o = {};
  var a = this.serializeArray();
  $.each(a, function() {
    if (o[this.name] !== undefined) {
      if (!o[this.name].push) {
          o[this.name] = [o[this.name]];
      }
      o[this.name].push(this.value || '');
    } else {
      o[this.name] = this.value || '';
    }
});
  return o;
};

// select all text when clicking a suggestion input
/**
$(document).on("click", ".suggestion_input, .jq-autocomplete", function (e) {
   //console.log("click suggestion input");
   $(this).val();
   $(this).next('input').val();
   $(this).select();
});
*/

/** This handles user pressing enter in autocompletee field w/o selecting from list: 
Simulate that user selects first */
/** Hmm, this triggers in all form fields, disabling return to submit...
$(document).on("focusin", ".jq-autocomplete", function () {
  //console.log("focus on autocomplete");
  $(window).keydown(function(event) {
    if(event.keyCode == 13) {
      console.log("enter1 in autocomplete");
      
      // find first
      var $first = $("UL.ui-autocomplete LI").first();
      //console.log("first = ", $first);
      $first.click();
      event.preventDefault();      
      return false;
    }
  });
});
*/

// http://stackoverflow.com/questions/16157459/how-to-open-select-element-using-jquery
var openSelect = function(selector){
  var element = $(selector)[0], worked = false;
  if (document.createEvent) { // all browsers
    var e = document.createEvent("MouseEvents");
    e.initMouseEvent("mousedown", true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null);
    worked = element.dispatchEvent(e);
  } else if (element.fireEvent) { // ie
    worked = element.fireEvent("onmousedown");
  }
  if (!worked) { // unknown browser / error
    console.log("openSelect: It didn't work in your browser.");
  }   
}


// https://css-tricks.com/snippets/jquery/get-query-params-object/
$.extend({
  getQueryParameters : function(str) {
      str = str || document.location.search;
      return (!str && {}) || str.replace(/(^\?)/,'').split("&").map(function(n){return n = n.split("="),this[n[0]] = n[1],this}.on({}))[0];
  }
});

/** location helpers */
function map_get_bounds(map) {
  var gbounds = map.getBounds();
  console.log('gbounds=', gbounds);
  return gbounds2bounds(gbounds);
}

// converts Google Bounds to simple array
function gbounds2bounds(boundsObj) {
  if(!boundsObj) return [];
  var ne = boundsObj.getNorthEast();
  var sw = boundsObj.getSouthWest();

  var g_loc_accuracy = 4;               

  var lat0 =  sw ? parseFloat(sw.lat()).toFixed(g_loc_accuracy) : 0;
  var lng0 =  sw ? parseFloat(sw.lng()).toFixed(g_loc_accuracy) : 0;
  var lat1 = ne ? parseFloat(ne .lat()).toFixed(g_loc_accuracy) : 0;
  var lng1 = ne ? parseFloat(ne .lng()).toFixed(g_loc_accuracy) : 0;
  return [[lat0,lng0],[lat1,lng1]];
}

function degrees2meters(degrees) {
  var meters_per_degree = 111320; // = 40075161/360; 1 degree = 111.3 km
  return degrees * meters_per_degree;
}

function meters2degrees(meters) {
  var degrees_per_meter = 0.00000898312; // =  1 meter = 0.00000898312 degrees
  return meters * degrees_per_meter;
}

// http://jeffjason.com/2011/12/google-maps-radius-to-zoom/
function radius2zoom(radius) {
  if(radius < 1000) return 15; // max zoom
  var miles = radius/1600;
  return Math.round(14 - Math.log(miles)/Math.LN2);
}


function radius2bounds(loc, radius) {
  add_degrees = meters2degrees(radius);
  var lat = loc[0];
  var lng = loc[1];
  var ne_lat = lat + add_degrees;
  var ne_lng = lng + add_degrees;
  var sw_lat = lat - add_degrees;
  var sw_lng = lng - add_degrees;  
  return [[sw_lat,sw_lng],[ne_lat,ne_lng]];
}

//zoom = 14 - log(miles) / 2.303;
//log(miles) = 14 - zoom * LN2;

function bounds_extend(bounds, min_radius) {
  var radius = bounds2radius(bounds);
  if(bounds.length && radius < min_radius) return radius2bounds(bounds2center(bounds), min_radius);
  return bounds;
}

/** expects [[],[]] */
function bounds2radius(bounds) {
  if(!bounds.length) return 0;
  var sw = bounds[0];
  var ne = bounds[1];
  var d_lat = Math.abs(ne[0] - sw[0]);
  var d_lng = Math.abs(ne[1] - sw[1]);
  var diff = (d_lat + d_lng) / 2; // cheesy average instead of pythagoras since x and y are usually the same
  var r = degrees2meters(diff);
  return Math.round(r / 2);
}

function array2bounds(boundsArray) {
  return {"southwest":{"lat":boundsArray[0][0],"lng":boundsArray[0][1]}, "northeast":{"lat":boundsArray[1][0],"lng":boundsArray[1][1]}};
}

function bounds2center(boundsArray) {
  console.log("b2c: lat01=" + parseFloat(boundsArray[0][0])) ;
  return [(parseFloat(boundsArray[0][0]) + parseFloat(boundsArray[1][0])) / 2, (parseFloat(boundsArray[0][1]) + parseFloat(boundsArray[1][1])) / 2]; 
}

function bounds_obj2json(bounds) {
  if(!g_loc_accuracy) g_loc_accuracy = 4; // set in google_pac.js
  console.log("bounds_obj2json: bounds=",bounds);        
  var lat0 = bounds.southwest ? parseFloat(bounds.southwest.lat).toFixed(g_loc_accuracy) : 0;
  var lng0 = bounds.southwest ? parseFloat(bounds.southwest.lng).toFixed(g_loc_accuracy) : 0;
  var lat1 = bounds.northeast ? parseFloat(bounds.northeast.lat).toFixed(g_loc_accuracy) : 0;
  var lng1 = bounds.northeast ? parseFloat(bounds.northeast.lng).toFixed(g_loc_accuracy) : 0;
  var bounds = [[lat0,lng0],[lat1,lng1]];
  return JSON.stringify(bounds);
}         

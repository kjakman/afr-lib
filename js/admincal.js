// globals
var redraw_interval_active = 5*1000;      // redraw every 5 seconds if activity
var redraw_interval_inactive = 5*60*1000; // redraw every 5 minutes if no activity
var redraw_interval_active_id   = 0;
var redraw_interval_inactive_id   = 0;
var auto_redraw = false; 

var g_menu_cached_id = 0;
var g_menu_show_id = 0; 
var g_timeout = 0; 
var g_timeout_interval = 25; // milliseconds
var g_loading_menu = false;
var g_data = {}; /** default data from PHP controller */
var g_obj_data = []; /** data loaded from back end */
var g_booked = []; /** data loaded from back end */
var g_invoiced = []; /** data loaded from back end */
var g_paid = []; /** data loaded from back end */

/** below are new, last row are variables, rest are fixed - at least for now */
var g_cal_params = {
  "obj_type": "reservation", "obj_name": "booking", "property_type": "shortstay", "view": "cal", "show":"reservation",
  "day_font_size":12, "org_w":24, "grid_h":20,"title_w":150, 
  "bar_x":16,"bar_y":5, "status": 15, "show": "reservation"
};

g_cal_params.cell_h = g_cal_params.grid_h;
g_cal_params.cell_w = g_cal_params.grid_w = g_cal_params.org_w;
g_cal_params.bar_h = 0.7 * g_cal_params.cell_h;
g_cal_params.bar_h = 0.7 * g_cal_params.cell_h;
g_cal_params.col_w = 3 * g_cal_params.grid_w; // stats columns
    
/** g_cal_params: missing */
// base_class, user_level, apt_list

/** locale specific month/day names */
var g_months = moment.months();
var g_monthsShort = moment.monthsShort();
var g_weekdays = moment.weekdays();
g_weekdays[7] = g_weekdays[0];
var g_weekdaysShort = moment.weekdaysShort();
g_weekdaysShort[7] = g_weekdaysShort[0];
var g_weekdaysMin = moment.weekdaysMin();
g_weekdaysMin[7] = g_weekdaysMin[0];
var g_today = date_today();

/** end new */

var g_id = 0;
var g_cal_entries = {};
var g_simple_dialog = false; // use bootstrap pre-dialog (blocked/available/new res)
var g_new_dialog = true; // use new bootstrap dialog
// new functions foobar

// keyboard shortcuts
// $(document).bind('keydown', 'pageup', function() {ajax_zoom(0.25);return false;});
// $(document).bind('keydown', 'pagedown', function() {ajax_zoom(-0.25);return false;});
$(document).bind('keydown', 'ctrl+f', function() {$('#cal_search').focus().select();return false;});
$(document).bind('keydown', 'meta+f', function() {$('#cal_search').focus().select();return false;});
$(document).bind('keydown', 'esc', function() {if(g_menu_cached_id) hideMenu();return false;});
$(document).bind('keydown', 'ctrl+right', function() {var np=g_period + 1; document.location = '?p=' + np;});
$(document).bind('keydown', 'ctrl+left', function() {if(g_period > 1) {var np=g_period-1; document.location = '?p=' + np;}});
$(document).bind('keydown', 'del', function() {if(g_menu_cached_id) {hideMenu();if(confirm('Are you sure you want to delete ' + g_menu_cached_id)) alert('del'); return false;}});
//$(document).myDisableSelection();

$(document).ajaxSuccess(function() {
  $('[data-toggle="tooltip"]').tooltip({
    'container':'body'      
  });
});


function fixResPos() {
  var grid_w = g_cal_params.grid_w;
  var numCols = g_cal_params.num_cols;
  var $first = $("#cal_guide-0-0");
  var last_sel = "#cal_guide-0-" + (numCols-1);
  var $last = $(last_sel);
  //console.log("first=", $first ," last = " + last_sel + " ", $last);
  
  var lastDate = $last.offset().left;
  var firstDate = $first.offset().left;
  var actual_w = (lastDate - firstDate) / numCols  
  //console.log("fixResPos grid=" + grid_w + " actual=" + actual_w);
  $resObjs = $(".resBar");
  //console.log("Found " + $resObjs.length);

  
  $.each($resObjs, function(index) {
    var $res = $(this);
    var res_width = $res.data('width');
    var in_date = $res.data('in');
    var in_sel = '*[data-date="' + in_date + '"]';
    var $guide = $(in_sel);
    if($guide.length) {
      var guide_pos = $guide.offset().left;
      var new_left = guide_pos - firstDate;
      var was_width = $res.css("width");
      var new_width = actual_w * res_width;
      $res.css('left', new_left + grid_w / 2);
      $res.css('width', new_width + "px");
      //console.log("OK: found guide with data-date=" + in_date + " w=" + res_width + " = " + new_width + " was:" + was_width);
      //$res.left(new_left + grid_w / 2);
    } else {
      //console.log(" could not find guide with data-date=" + in_date + " sel=" + in_sel);
    }
  });
}
        
// new version of abspos2index, returns more data
function canvas2pos($canvas, evt) {
  var $container = $canvas.closest(".cal-container");
  var container_offset = $container.offset(); // offset of calendar container relative to document
  var canvas_offset = $canvas.offset(); // offset of calendar canvas relative to document
  var offset = {'top':  canvas_offset.top, 'left': canvas_offset.left - container_offset.left}; // left = width of Property title 
  
  var relPos = {'top': evt.pageY - offset.top, 'left': evt.pageX - canvas_offset.left}; // mouse position relative to canvas
  var index = relpos2index(relPos) || {};

  var data = {};
  data.mouse = {'left': evt.pageX, 'top': evt.pageY};
  data.offset = offset;
  data.pos = relPos;
  data.index = index;

  //console.log("canvas2pos: data:", data, " size:", $canvas.data('size'), " id=" + $canvas.attr('id'));
  
  data.index = position_bounds(index, $canvas.data('size')); // clip to size: don't allow < 0 or > max
  //console.log("canvas2pos: index:", index, " new-index:", data.index, " size:", $canvas.data('size'));

  return data;  
}

// returns index of relative position in canvas
function relpos2index(pos) {
  return pos2index(pos.left, pos.top, g_cal_params.grid_w, g_cal_params.cell_h);
}

var isTouchSupported = 'ontouchstart' in window; // touch devices
isTouchSupported = false; /** this makes moving around impossible */

var startEvent = isTouchSupported ? 'touchstart' : 'mousedown';
var moveEvent = isTouchSupported ? 'touchmove' : 'mousemove';
var endEvent = isTouchSupported ? 'touchend' : 'mouseup';

//console.log("Touch supported=" + isTouchSupported + " startEvent=" + startEvent);
/* left: 110, right: 261: width: 151 */

// called if click on open date, return date/apt index
function event_init(evt, $canvas, $target) {
  var ci = $canvas.data('index');
  var data = canvas2pos($canvas, evt);
  //console.log("index=" + ci + " init data=", data);
  var $apt = $("#cal_apt-" + ci + "-" + data.index.top);
  //console.log("#cal_apt-" + ci + "-" + data.index.top);
  //console.log("apt len=" + $apt.length);
  //console.log("data" + $apt.data('apt'));
  
  var date = $target.data('date');
  var index_left = $target.data('index');
  data.index.left = index_left; 
  data.date = date;
  data.apt = apt = $apt.data('apt') || {};
  data.canvas = $canvas;
  data.size = $canvas.data('size') || {};

  return data;
}

function calendar_date_string(index, ci) {          
  var checkin = index2date(index, ci);
  var str = g_cal_params.target_day > 0 ? sql2human_short_time(checkin) : sql2human_shortest(checkin);
  //console.log("index2date: td=" + g_cal_params.target_day + " i=" + index + "=" + checkin + ": " + str);
  
  return str;
}  

$(document).on(startEvent, ".cal-guide .guide-box", function(evt) {    
  var $guide = $(this).parent();
  var $canvas = $guide.closest('.cal-container').find('.cal-canvas');
  var ci = $canvas.data('index');
  var data = event_init(evt, $canvas, $guide);
  //new_entry_handler(evt, data);
  //console.log("event-init data:", data);
  
  var apt = data.apt;
  var apt_id = apt.id;
  var checkin = data.date;  
  var $apt = $("#cal_apt-" + ci + '-' + data.index.top);
  var $day = $("#cal_guide-" + ci + '-' + data.index.left);
  var date = $day.data('date');
  var day_x = $day.offset().left;

  // make new entry
  $("#new_entry").remove(); // delete if exists
  var iw = g_cal_params.grid_w/4; // initial width, store a
  //iw = 20;
  var ih = 0.75 * g_cal_params.bar_h; // initial width, store a
  var top = 0.15 * g_cal_params.cell_h;
  var max = data.size;
  
  var new_pos = index2pos(data.index);
  //var left = new_pos.left + data.offset.left; /** not accurate */
  var left = day_x;

  //console.log("Start resize data: i=",data.index.left + " date=" + data.date + " day_x=" + day_x + " left=" + left + " max:", max);
  
  var $entry = $apt.append("<div class='resBar reserved round3' id='new_entry' style='width:"+ iw + "px;top:0;left:" + left + "px;top:" + top + "px'>New</div>");
  var $entry = $('#new_entry');
  
  $('#new_entry').width(iw); // why needed?
  var $canvas = data.canvas;
  var ci = $canvas.data('index');

  $entry.data('apt_id', data.apt.id);
  var start = data.index; 
  $entry.data('start', start);  
  $entry.data('end', start);  
  $entry.data('start-left', left);  


  var text = apt.name + ': ' + calendar_date_string(start.left, ci);
  calendar_tip($canvas, text, 'info', data.index);

  
  hideMenu();
  apt_hover(ci, start.top);
  guide_hover(ci, start.left);
  
  $(document).bind(moveEvent, function(evt){ // new entry
    var right2left = false; // allow dragging to the left
    
    var $entry = $('#new_entry');
    var data = canvas2pos($canvas, evt);
    var current = data.index;
    $entry.data('current', current);
    var delta = delta_index(start, current);
    var start_left = $entry.data('start-left');
    
    var max_w = (max.left - start.left - 1) * g_cal_params.cell_w;
    var max_h = (max.top - start.top - 1) * g_cal_params.cell_h + ih;
    
    if(right2left) {
      var width = Math.max(Math.abs(delta.left) * g_cal_params.grid_w, iw);    
      var left = delta.left < 0 ? start_left + delta.left * g_cal_params.cell_w : start_left;
      $entry.css("left", left + 'px');
    } else {
      var left = dateindex2pos(ci, start.left);
      var right = dateindex2pos(ci, current.left);
      var width = right - left;
    }
    
    //console.log("w=" + width + " max=" + max_w + " delta=" + delta.left + " left=" + left + " right=" + right);
    // width and position
    $entry.width(width);
    
    //console.log("M: start=" + start.left + " current=" + current.left + " Delta w=" + delta.left + " iw=" + iw + " Width=" + width + " Acutal=" + $("#new_entry").width());
    
    /** Block reservations turned off
    if(delta.top > 0) { // multiple apartments
      var height = Math.min(g_cal_params.bar_h + delta.top * g_cal_params.cell_h, max_h);
      $entry.height(height);
      multiple_count = Math.min(delta.top, max.top);
      var text = "Block reservation " + calendar_date_string(start.left, ci) + " - " + calendar_date_string(start.left + delta.left, ci);
    } else 
    */
    
    if(delta.left > 0) {
      //var text = "New " + g_cal_params.obj_name + " @ " + apt.name + ' ' + calendar_date_string(start.left, ci) + " - " + calendar_date_string(start.left + delta.left, ci);
      var text = apt.name + ': ' + calendar_date_string(start.left, ci) + " - " + calendar_date_string(start.left + delta.left, ci);;
      $entry.height(g_cal_params.bar_h);
    } else {
      //var text = "New " + g_cal_params.obj_name + " @ " + apt.name + ' ' + calendar_date_string(start.left, ci);
      var text = apt.name + ': ' + calendar_date_string(start.left, ci);
      $entry.height(g_cal_params.bar_h);
    }

    calendar_tip($canvas, text, 'info');
    
    //apt_hover(ci, start.top, current.top);
    apt_hover(ci, start.top);
    guide_hover(ci, start.left, current.left);
    evt.preventDefault(); // disable selection
  });     
                   
  $('body').bind(endEvent, function(evt){   // custom stop event
    guide_hover(ci);
    apt_hover(ci);
    $('.cal-tip').hide('fast');
    var data = canvas2pos($canvas, evt);
    var end = data.index;
    //console.log("endEvent (mouseup)", data);
    //console.log("start", start);
    //console.log("end", end);
    $entry.data('end', end);
    if(start && end) {
      var delta = delta_index(start, end);
      var checkout = index2date(end.left);
      if(checkin > checkout) {var tmp=checkout;checkout=checkin;checkin=tmp;} // switch
      //console.log("delta= " + delta.left + " in=" + checkin + " out=" + checkout);
      if(delta.left > 0) { // show dialog
        var title = 'New ' + g_cal_params.obj_name;
        var apt_list = apt.id;

        /** Block reservations turned off
        if(delta.top > 0) { // multiple apartments
          if(confirm("You are about to create multiple reservations. Click 'Ok' to continue, 'Cancel' to make just one reservation for " + apt.name + ".")) {                
            var apts = [];
            var $apts = $("#cal_apts-" + ci + " LI").slice(start.top, end.top+1);
            $.each($apts, function(index) {
              var apt = $(this).data('apt'); 
              apts.push(apt.id);
            });
            apt_list = apts.join(',');
          }
        }
        */
        
        var title = dialog_title(frm);
        $("#" + frm + " .overlay-title").html(title);
        $("#" + frm + " .btn-group button").removeClass("btn-success btn-danger").addClass('btn-default');
        $("#" + frm + " .day-rate").hide();

        if(g_new_dialog) { // add/edit booking
          checkin =  moment(checkin).format(g_moment_format);
          checkout = moment(checkout).format(g_moment_format);
          console.log("calling cal_booking_dialog in=" + checkin + " out=" + checkout + " data=", data);
          
          var data = {"notes": '', "rate_day": '', "status": 0, "last_name": '', "comments": '', "obj_type": g_cal_params.obj_type, "property_type": g_cal_params.property_type, "apt_id": apt.id, "apartment_name": apt.name, "checkin": checkin, "checkout": checkout};
          cal_booking_dialog(data);
        } else {
          
          var moment_in = moment(checkin);
          var moment_out = moment(checkout);
          var data = {"notes": '', "rate_day": '', "status": 0, "last_name": '', "comments": '', "obj_type": g_cal_params.obj_type, "property_type": g_cal_params.property_type, "apt_id": apt.id, "apartment_name": apt.name, "checkin": moment_in.format(g_moment_format), "checkout": moment_out.format(g_moment_format)};
  
          var frm = "calendar_simple_form";
          $("#" + frm + " .messages").remove();
          populate(frm, data);
          
          init_datepicker_ranges();
          
          if(g_simple_dialog) {
            show_simple_dialog(data);
          } else {
            var  title = "New " + g_cal_params.obj_name;
            cal_dialog_old(title, data, {})
          }
        }
        
      } else {
        $entry.remove();
      }        
    }
    $('body').unbind("mouseup");
    $(document).unbind("mousemove");
    
  });

  return;
  // end new entry handler  
});


// Disable single-click on res if dbl-click
// Thanks http://stackoverflow.com/questions/1067464/need-to-cancel-click-mouseup-events-when-double-click-event-detected/1067484#1067484
$(document).on('click', '.cal-entries .auth_view, .cal-entries .auth_edit', function(e) {
  var that = this;
  var $target = $(this);
  $(".cal-entries .selected").removeClass('selected');
  $target.addClass('selected');
  
  //console.log("singleClick on ", that);
  if(!$target.hasClass("auth_edit")) { // view only, show now
    //console.log("view only, show now");
    singleClick.call(that, e);    
  } else { // can edit, wait 200ms to check for double click
    //console.log("can edit, wait 200ms");
    setTimeout(function() {
        var dblclick = parseInt($(that).data('double'), 10);
        if (dblclick > 0) {
            $(that).data('double', dblclick-1);
        } else {
            singleClick.call(that, e);
        }
    }, 200);
  }
})    

$(document).on('dblclick', '.cal-entries .auth_edit', function(e) {
  $(this).data('double', 2);
  doubleClick.call(this, e);
});
             
function calendar_tip($canvas, text, style, pos) {
  
  //console.log("Tip on pos:", pos);
  var index = $canvas.data('index') || 0;
  var id = 'cal_tip-' + index;
  var $tip = $('#' + id);
  if(!$tip.length) {
    $tip = $('<div id="' + id + '" class="cal-tip msg click-close"></div>').appendTo($canvas);  // append if doesn't exist
    //console.log("tip "+ id + " append to " + index);
  }

  //if(text) $tip.html(print_message("foo")).show('fast');
  if(text) $tip.html(print_message(text, style)).show('fast');
  else $tip.hide('fast').html('');
  //$tip.css('top', '100px');
  //$tip.css('left', '0px');
  return $tip;
}


  
function apt_hover(ci, start, end) {
  if(end < start) return;
  if(typeof start === 'undefined') start = -1;
  if(typeof end === 'undefined') end = -1;
  $("#cal_apts-" + ci + " LI").removeClass('hover');
  if(start > -1 && end > -1) {
    if(start > end) {var tmp = end;end=start;start=tmp;} // swap   
    $("#cal_apts-" + ci + " LI").slice(start, end*1+1).addClass('hover');
    //console.log("start=" + start + " end=" + end + " sel='" + "#cal_apts-" + ci + " LI'" +  " size=" + $("#cal_apts-" + ci + " LI").slice(start, end*1+1).length)
  } else if(start > -1) {
    //console.log("single: start=" + start + " sel='" + "#cal_apts-" + ci + " LI'" +  " size=" + $("#cal_apts-" + ci + " LI").slice(start, end*1+1).length)
    $("#cal_apt-" + ci + "-" + start).addClass('hover');
  }

  /** side effect - set tip to this height */
  var id = 'cal_tip-' + ci;
  var $tip = $('#' + id);
  if($tip.length && start > -1) {
    var top = start * g_cal_params.cell_h;
    $tip.css('top', top + 'px');
  }
  
}

function guide_hover(ci, start, end) {
  if(end < start) return;
  if(typeof start === 'undefined') start = -1;
  if(typeof end === 'undefined') end = -1;
  $("#cal_guides-" + ci + " LI .guide_title").removeClass('hover');
  if(start > -1 && end > -1) {
    if(start > end) {var tmp = end;end=start;start=tmp;} // swap   
    $("#cal_guides-" + ci + " LI .guide_title").slice(start, end*1+1).addClass('hover');
    //console.log("start=" + start + " end=" + end + " len=" + $("#cal_guides-" + ci + " LI .guide_title").slice(start, end*1+1).length);
  } else if(start > -1) {
    $("#cal_guide-" + ci + "-" + start + " .guide_title").addClass('hover');
  }
}

// sets target element back to original size/pos and hides messages
function reset($target, start_pos, width) { 
  $target.css(start_pos);
  //$target.css({'left': start_pos.left + 'px', 'top': start_pos.top + 'px'});
  if(width > 0) $target.width(width);
  $target.removeClass('disabled_bg');
  $('.cal-tip').hide('fast');
  $('#cal_messages').css('top', '0px').hide('fast');
}

// end new functions

function delta_string_unit(count, unit, show_sign) {
  if(Math.abs(count) != 1) unit += 's'; // plural s
  var sign = show_sign && count > 0 ? '+' : '';
  return " (" + sign + count + ' ' + unit + ")";
}

function delta_string(count, show_sign) {  
  if(g_cal_params.target_day > 0) {                
    var unit = 'hour';
    count = count / g_cal_params.resolution;
  } else {
    var unit = 'day';
  }
  return delta_string_unit(count, unit, show_sign);
}


function create_entry_handlers($canvas, target) {
  if(!target) target = ".cal-entries ";
  console.log("create_entry_handlers: target=" + target + " len=" + $(target).length + " canvas=" + $canvas.prop('id'));
  
  if(!$(target).length) target = ".cal-entries ";
  create_resizable_handler($canvas, target);
  create_draggable_handler($canvas, target);
}

function create_resizable_handler($canvas, target) {    
  // resizable  - repetetive code due to known issue: http://bugs.jqueryui.com/ticket/3423
  if(!target) target = '.cal-entries ';
  var $resize_ew = $(target + ".resize_ew");
  console.log("Found " + $resize_ew.length + " resize_ew for target='" + target + ".resize_ew" +  "'"); 
  $(target + ".resize_ew").resizable({
			grid: g_cal_params.grid_w,
      handles: 'e, w', //$(this).data('resize'), //'e,w',
      start: resize_start,
      resize: resize_move,
      stop: resize_stop
  });
  $(target + ".resize_e").resizable({
			grid: g_cal_params.grid_w,
      handles: 'e', //$(this).data('resize'), //'e,w',
      start: resize_start,
      resize: resize_move,
      stop: resize_stop
  });
  $(target + ".resize_w").resizable({
			grid: g_cal_params.grid_w,
      handles: 'w', //$(this).data('resize'), //'e,w',
      start: resize_start,
      resize: resize_move,
      stop: resize_stop
  });      
}  

function resize_start(evt, ui) {
  //console.log("resize start");
  hideMenu();
  var $target = $(evt.target);        
  var $canvas = $target.closest('.cal-container').find('.cal-canvas');
  var data = event_init(evt, $canvas, $target);
  var ci = $canvas.data('index');        

  // set max width        
  var start = data.index; 
  var w = $target.width();  
  var max = data.size;
  var max_w = w + (max.left - start.left - 1) * g_cal_params.cell_w;
  $(this).resizable( "option", "maxWidth", max_w );
  $target.removeClass('selected');
  $target.addClass('hover');
  guide_hover(ci, start.left);                
  apt_hover(ci, start.top);                
}

function resize_move(evt, ui) {
  var $target = $(evt.target);        
  var $canvas = $target.closest('.cal-container').find('.cal-canvas');
  var canvas_id = $canvas.prop('id');

  var start_in = p2i(ui.originalPosition.left, g_cal_params.grid_w);
  var start_out = p2i(ui.originalPosition.left + ui.originalSize.width, g_cal_params.grid_w);
  var delta_pos =  Math.round((ui.position.left - ui.originalPosition.left) / g_cal_params.grid_w);
  var delta_w = Math.round((ui.size.width - ui.originalSize.width) / g_cal_params.cell_w);
  var res_x_offset = g_cal_params.grid_w / 4;          
  
  var ci = $canvas.data('index');        
  var current = delta_pos ? start_in + delta_pos : start_out + delta_w;
  var max = g_cal_params.num_cols - 1;
  if(current < 0) current = 0;
  if(current > max) {
    delta_w = max - start_out; 
    current = max;
  }
  
  var date_str = calendar_date_string(current, ci);  

  // not sure why needed, but this forces height and top to remain the same
  var h = ui.originalSize.height;
  var t = ui.originalPosition.top;
  
  $target.css("height", h);
  $target.css({top: t});
  
  // $target.height(g_cal_params.bar_h); // not sure why needed
  //console.log("Resize: id=" + $target.attr('id') + " dw=" + delta_w + " dp=" + delta_pos + " h=" + h);
  if(delta_pos) {
    guide_hover(ci, current, start_in);
    var start_name = g_cal_params.obj_type == 'reservation' ? 'checkin' : 'start date/time';
    var msg = 'Change ' + start_name + ' to ' + date_str + delta_string(delta_pos, true);
  } else if (delta_w) {
    guide_hover(ci, start_out, current);
    //console.log("1: add hover to " + start_out + " - " + current);

    // new code to align with actual dates  
    //console.log("start_in=" + start_in + " start_out=" + start_out + " delta_pos=" + delta_pos + " delta_w=" + delta_w + " current=" + current);
    var left = dateindex2pos(ci, start_out);
    var right = dateindex2pos(ci, current);      
    var delta_width = right - left;
    var width = ui.originalSize.width + delta_width + res_x_offset;
    $target.css("width", width);
    
    var end_name   = g_cal_params.obj_type == 'reservation' ? 'checkout' : 'end date/time';
    var msg = 'Change ' + end_name + ' to ' + date_str + delta_string(delta_w, true);
  } else {
    var width = ui.originalSize.width + res_x_offset;
    $target.css("width", width);
    var msg = "No change";
    guide_hover(ci);
  }            

  if(msg) calendar_tip($canvas, msg, 'info', {'top': t, 'left': current});             
}

function resize_stop(evt, ui) {
  auto_redraw = true;
  changed = false;          
  dragging = false;

  var $target = $(evt.target);        
  var $canvas = $target.closest('.cal-container').find('.cal-canvas');
  var changed = false;          
  var start_in = p2i(ui.originalPosition.left, g_cal_params.grid_w);
  var start_out = p2i(ui.originalPosition.left + ui.originalSize.width, g_cal_params.grid_w);
  var delta_pos =  Math.round((ui.position.left - ui.originalPosition.left) / g_cal_params.grid_w);
  var delta_w = Math.round((ui.size.width - ui.originalSize.width) / g_cal_params.cell_w);

  var ci = $canvas.data('index');        
  var current = delta_pos ? start_in + delta_pos : start_out + delta_w;

  var max = g_cal_params.num_cols - 1;
  if(current < 0) current = 0;
  if(current > max) {
    delta_w = max - start_out; 
    current = max;
  }
  var date_str = calendar_date_string(current, ci);
  
  var obj_id = $target.data('obj_id');
  var obj_type = $target.data('obj_type');
  
  var start_field = g_cal_params.start_field || 'checkin';
  var end_field = g_cal_params.end_field || 'checkout';
  var confirm_text = '';
  if(delta_pos) {
    confirm_text = 'Are you sure you want to change the checkin of ' + obj_type + ' ' + obj_id + ' to ' + date_str + '?';
    //fancyConfirm(confirm_text, change_reservation, obj_type, id, 'checkin', checkin);
    if(confirm(confirm_text)) change_reservation($canvas, $target, obj_type, obj_id, start_field, index2date(current, ci));else reset($target, ui.originalPosition, ui.originalSize.width);
  } else if(delta_w) {   
    confirm_text = 'Are you sure you want to change the checkout of ' + obj_type + ' ' + obj_id + ' to ' + date_str + '?';
    //fancyConfirm(confirm_text, change_reservation, obj_type, id, 'checkout', checkout);
    if(confirm(confirm_text)) change_reservation($canvas, $target, obj_type, obj_id, end_field, index2date(current, ci)); else reset($target, ui.originalPosition, ui.originalSize.width);
  } else { // No change
    $('.cal-tip').hide('fast');
  }
  //console.log("Confirm text=" + confirm_text);
  $target.removeClass('hover');
  apt_hover(ci);
  guide_hover(ci);
}



// http://stackoverflow.com/questions/1572950/how-to-make-jquery-draggable-with-fixed-x-and-y-axis
$.fn.draggableXY = function(options) { 
  //console.log("draggableXY");
  //console.log(options);
  var defaultOptions = { 
    distance: 5, 
    dynamic: false 
  }; 
  options = $.extend(defaultOptions, options); 

  this.draggable({ 
    //grid: options.grid,
    distance: options.distance,
    containment: options.containment,
    start: function (event, ui) { 
      ui.helper.data('draggableXY.originalPosition', ui.position || {top: 0, left: 0}); 
      ui.helper.data('draggableXY.newDrag', true);
      var start_fn=options.start;
      if(typeof(start_fn) !== 'undefined') start_fn(event, ui);
      
    }, 
    drag: function (event, ui) { 
      var originalPosition = ui.helper.data('draggableXY.originalPosition'); 
      var deltaX = Math.abs(originalPosition.left - ui.position.left); 
      var deltaY = Math.abs(originalPosition.top - ui.position.top); 

      var newDrag = options.dynamic || ui.helper.data('draggableXY.newDrag'); 
      ui.helper.data('draggableXY.newDrag', false); 

      var xMax = newDrag ? Math.max(deltaX, deltaY) === deltaX : ui.helper.data('draggableXY.xMax'); 
      ui.helper.data('draggableXY.xMax', xMax); 

      var newPosition = ui.position; 
      if(xMax) { 
        newPosition.top = originalPosition.top; 
      } 
      if(!xMax){ 
        newPosition.left = originalPosition.left; 
      } 
      // own grid impl.
      if(options.grid) {
        var gx = options.grid[0];
        var gy = options.grid[1];
        var x_offset = options.grid[2] || 0;
        var y_offset = options.grid[3] || 0;
        //console.log("grid x= " + gx + " y=" + gy + " offset=" + x_offset + '/' + y_offset);
        newPosition.top = newPosition.top - newPosition.top % gy + y_offset; 
        newPosition.left = newPosition.left - newPosition.left % gx + x_offset; 
      }
      var drag_fn=options.drag;
      if(typeof(drag_fn) !== 'undefined') drag_fn(event, ui);
      
      return newPosition; 
    },
    stop: function (event, ui) {
      var stop_fn=options.stop;
      if(typeof(stop_fn) !== 'undefined') stop_fn(event, ui);
    }
    
  }); 
};


function drag_start(evt, ui) {
  var $target = $(evt.target);
  $target.addClass('hover');
}

function res_drag(evt, ui, axis) {
  var $target = $(evt.target);        
  var $canvas = $target.closest('.cal-container').find('.cal-canvas');
  var ci = $canvas.data('index');  
  var deltaX =  axis == 'y' ? 0 : (ui.position.left - ui.originalPosition.left) / g_cal_params.grid_w;
  var deltaY =  axis == 'x' ? 0 : (ui.position.top - ui.originalPosition.top) / g_cal_params.cell_h;  
  var delta_xi =  Math.round(deltaX);
  var delta_yi =  Math.round(deltaY);
  
  var start_date = $target.data("in");
  var start_left = date2index(ci, start_date);  
  //console.log("start date=" + start_date + " index=" + start_left);
  
  var start = {'top': p2i(ui.originalPosition.top, g_cal_params.cell_h), 'left': start_left};
  var current = {'top': start.top + delta_yi , 'left': start.left + delta_xi};

  if(delta_xi > 0) {
    guide_hover(ci, start.left, current.left);
  } else if (delta_xi < 0) {
    guide_hover(ci, current.left, start.left);
  } else {
    guide_hover(ci, start.left);
  }
  
  if(delta_yi > 0) {
    apt_hover(ci, start.top, current.top);
  } else if (delta_yi < 0) {
    apt_hover(ci, current.top, start.top);
  } else {
    apt_hover(ci, start.top);
  }
  
  //console.log("axis=" + axis + " start=" + start.left + " current=" + current.left + " d=" + delta_xi);
  
  //console.log("p=" + ui.originalPosition.left + " i=" + p2i() + " d=" + d);
  var date_string = calendar_date_string(current.left, ci);
  /** new code: using position of guide */

  if(axis != 'x') {
    var top = Math.round(aptindex2pos(ci, current.top));
    var res_y_offset = g_cal_params.cell_h / 4;
    //console.log("Drag X: date=" + date_string + " current=" + current.top + " top=" + top); 
    ui.position.top = top + res_y_offset;
  }
    
  if(axis != 'y') {
    var left = Math.round(dateindex2pos(ci, current.left));
    var res_x_offset = g_cal_params.grid_w / 2;          
    //console.log("Drag X: date=" + date_string + " current=" + current.left + " left=" + left); 
    ui.position.left = left + res_x_offset;
  }
  
  if(delta_xi || delta_yi) {
    var msg = delta_xi ? "Change checkin to " + date_string + ' ' + delta_string(delta_xi, true) : "Change property to " + index2apt(current.top, 'name');
  } else {
    var msg = "No change";
  }
  
  calendar_tip($canvas, msg, 'info');        
}

function drag_stop(evt, ui, axis) {
  var $target = $(evt.target);        
  var $canvas = $target.closest('.cal-container').find('.cal-canvas');
  var ci = $canvas.data('index');
  var deltaX =  axis == 'y' ? 0 : (ui.position.left - ui.originalPosition.left) / g_cal_params.grid_w;
  var deltaY =  axis == 'x' ? 0 : (ui.position.top - ui.originalPosition.top) / g_cal_params.cell_h;  
  var delta_xi =  Math.round(deltaX);
  var delta_yi =  Math.round(deltaY);
  var start = {'top': p2i(ui.originalPosition.top, g_cal_params.cell_h), 'left': p2i(ui.originalPosition.left, g_cal_params.grid_w)};
  var current = {'top': start.top + delta_yi , 'left': start.left + delta_xi};
  //console.log("start=" + start.top + '/' + start.left + " current=" + current.top + '/' + current.left); 
  
  if(delta_xi || delta_yi) {
    var apt=index2apt(current.top);
    var msg = "Are you sure you want to move the reservation to " + (delta_xi ? calendar_date_string(current.left, ci) : apt.name );              
    if(confirm(msg)) {
      var obj_id = $target.data('obj_id');
      var obj_type = $target.data('obj_type');
          
      if(delta_xi) { // change dates
        var start_field = g_cal_params.start_field || 'checkin';
        var end_field = g_cal_params.end_field || 'checkout';
        var vars = [start_field, end_field];
        var unit = g_cal_params.target_day > 0 ? ' HOUR' : ' DAY';
        var vals = ['+ ' + delta_xi + unit, delta_xi + unit];
      } else { // change apt
        var vars = 'apt_id';
        var vals = apt.id;
      }
      change_reservation($canvas, $target, obj_type, obj_id, vars, vals);
    } else {
      reset($target, ui.originalPosition);      
    }
  } else { // No change
    $('.cal-tip').hide('fast');
    $target.css({top: ui.originalPosition.top, left: ui.originalPosition.left});
  }
  guide_hover(ci);
  apt_hover(ci);
   
  $target.removeClass('hover');  
}

function change_reservation($canvas, $target, obj_type, obj_id, variable, value) {
  //console.log("change res: ot=" + obj_type + " id=" + obj_id + " var=" + variable + " val=" + value);
  if(is_array(variable) && is_array(value) && variable.length > 1 && variable.length == value.length) { // array
    var qstr = '';
    $.each(variable, function(index) {
      qstr = qstr + '&' + variable[index] + '=' + escape(value[index]);
    });      
  } else {
    var qstr = '&' + variable + '=' + escape(value)
  }
  
  console.log("Change res: " + ajax_url);
  if(obj_type && obj_id) {
    var ajax_url = '/admin/app_data.php?oper=edit&obj_type=' + obj_type + '&id=' + obj_id + qstr;
    ajax_load($canvas, $target, ajax_url); // change checkin
  } else {
    calendar_error($canvas, "Missing object type or id");
  }
}

function calendar_error($canvas, error) {
  $tip = $canvas.find('.cal-tip');
  $tip.html(print_message(error, 'error')).show('fast');
}

// clears and hides msgs div, loads url into messages div and shows it when ready
function ajax_load($canvas, $target, ajax_url) {
  auto_redraw = true;
  //console.log(ajax_url);
  $tip = $canvas.find('.cal-tip');
  $tip.html(print_message("Processing...", 'info')).show('fast');
  //console.log("target len=" + $target.length);
  $.ajax({
      url: ajax_url,            
      success: function (responseText) {
        $tip.show('fast');
        //console.log(responseText);
        var result = parse_json(responseText);
        var success = result.success;
        var message = result.message;
        var warning = result.warning;
        var error = result.error;
        if(error) $tip.html(print_message(error, 'error'));
        else if(success) $tip.html(print_message(message, 'success'));
        else if(warning) $tip.html(print_message(message, 'warning'));
        else $tip.html(print_message('??', 'warning'));
      }
  });  
}
  

function create_draggable_handler($canvas, target) {
  if(!target) target = '.cal-entries ';
  var offset = $canvas.offset();
  var size = $canvas.data('size');
  
  var grid = [g_cal_params.grid_w, g_cal_params.cell_h, g_cal_params.bar_x, g_cal_params.bar_y] 
  var distance = g_cal_params.grid_w / 2;
  var containment = [offset.left, offset.top, offset.left + (size.left - 1) * g_cal_params.grid_w, offset.top +  + (size.top - 1)  * g_cal_params.cell_h];
  //console.log("containment of " + $canvas.prop('id'));
  //console.log(containment);
  
  $(target + ".drag_xy").draggableXY({  
    //grid: grid,
    distance: distance,
    containment: containment,
    start: function(event, ui) {console.log('start xy');drag_start(event, ui);},
    stop: function(event, ui) {console.log('stop xy');drag_stop(event, ui, 'xy');},
    drag: function(event, ui) {console.log('drag xy');res_drag(event, ui, 'xy');}
  });
  
  $(target + ".drag_y").draggable({
    grid: [g_cal_params.grid_w, g_cal_params.cell_h],
    distance: g_cal_params.grid_w / 2,
    containment: containment,
    axis: 'y',
    start: function(event, ui) {console.log('start ns');drag_start(event, ui);},
    stop: function(event, ui) {console.log('stop ns');drag_stop(event, ui, 'y');},
    drag: function(event, ui) {console.log('drag ns');res_drag(event, ui, 'y');}
  });

  $(target + ".drag_x").draggable({
    grid: [g_cal_params.grid_w, g_cal_params.cell_h],
    axis: 'x',
    distance: g_cal_params.grid_w / 2,
    containment: containment,
    start: function(event, ui) {console.log('start ew');drag_start(event, ui);},
    stop: function(event, ui) {console.log('stop ew');drag_stop(event, ui, 'x');},
    drag: function(event, ui) {console.log('drag ew');res_drag(event, ui, 'x');}
  });

}

$(document).on("click", ".btn-available", function() {
  $("#avail_select").hide();
  var $button = $(this);
  var $other_button = $button.parent().find('.btn-not_available');
  var $input = $button.parent().find('input');
  var $form = $button.closest('form');
  $form.find(".collapse").hide();
  $form.find(".show-free").show();
  $form.find(".show-none").hide();
  //console.log("free=" + $form.find(".show-free").length);
  //var $hide = $form.find(".hide-block");
  //$hide.removeClass("hide").show();
  
  $input.val(-20);
  $button.removeClass('btn-default').addClass('btn-success');
  $other_button.removeClass('btn-danger').addClass('btn-default');

  console.log("Available click");  
});

$(document).on("click", ".btn-not_available", function() {
  $("#avail_select").hide();

  var $button = $(this);
  var $other_button = $button.parent().find('.btn-available');
  var $input = $button.parent().find('input');
  var $form = $button.closest('form');
  $form.find(".collapse").hide();
  $form.find(".show-block").show();
  $form.find(".show-none").hide();

  //$hide.hide();
  
  $input.val(35);
  $button.removeClass('btn-default').addClass('btn-danger');
  $other_button.removeClass('btn-success').addClass('btn-default');  
  //console.log("other button len=" + $other_button.length);
});

$(document).on("click", "#cal_menu .close-button", function() {
  //console.log("close menu");
  $(".cal-entries .selected").removeClass('selected');
})

$(document).on("hide.bs.modal", "#overlay_content", function() {
  //console.log("close modal");
  $(".cal-entries .selected").removeClass('selected');
  $('#new_entry').remove();

  // do something...
})


$(document).on('click', '.test-pricing', function () {
  var qs = $("#calendar_simple_form").serialize();
  var ajax_url = "/admin/app_data.php?oper=test-pricing&" + qs; 
  console.log(ajax_url);

  $.ajax({
    url: ajax_url,            
    success: function (responseText) {
      var result = parse_json(responseText);
      var success = result.success;
      var message = result.message;
      $("#test_pricing").html(message);
    }
  });
  
  $("#test_pricing").load();
  return false;
});


$(document).on('hide.bs.modal', '#overlay_content_simple', function () {
  $("#new_entry").remove(); // delete if exists
  //console.log("Modal hide");
})
  
function index2apt(index, field) {
  var $apt = $("#cal_apt-0-" + index);
  var apt = $apt.data('apt');
  //console.log(apt);
  return (typeof(field) == 'undefined') ? apt : apt[field];
}

function index2date(index, parent_index) {
  if(!parent_index) parent_index = 0;
  var $date = $('#cal_guide-' + parent_index + '-' + index);
  var date = $date.data('date'); 
  //console.log("Date for index=" + index + " = " + date);
  return date;
}


/** new functions using guide for position instead of globals */
// returns a canvas position given an index
function index2pos(index) {
  var y_offset = g_cal_params.bar_y;
  var x_offset = g_cal_params.target_day > 0 ? 0 : 1/3 * g_cal_params.grid_w;
  var left = index.left * g_cal_params.cell_w + x_offset;
  var top = index.top * g_cal_params.cell_h + y_offset;
  return {"top": top, "left": left};
}

// takes canvas_index + date, returns pixel offset (x axis)
function date2pos(ci, date) {
  var $dates = $("#cal_guides-" + ci);
  var $first = $("#cal_guide-" + ci + "-0");
  var firstOffset = $first.offset().left;

  $date = $dates.find('[data-date="' + date + '"]');
  if(!$date.length) {
    console.log("Could not find  date=" + date);    
    return -1;
  }
  date_offset = $date.offset().left - firstOffset;
  return date_offset;  
}

// takes date, returns x-index of date
function date2index(ci, date) {
  var $dates = $("#cal_guides-" + ci);
  $date = $dates.find('[data-date="' + date + '"]');
  if(!$date.length) {
    console.log("Could not find  date=" + date);    
    return -1;
  }
  return $date.data('index');  
}

// takes canvas_index + date, returns pixel offset (x axis)
function dateindex2pos(ci, index) {
  var $dates = $("#cal_guides-" + ci);
  var $first = $("#cal_guide-" + ci + "-0");
  var firstOffset = $first.offset().left;
  var max = g_cal_params.num_cols - 1;
  if(index > max) index = max;
  if(index < 0) index = 0;
  $date = $dates.find('[data-index="' + index + '"]');
  if(!$date.length) {
    console.log("Could not find date with index=" + index + " selector: #cal_guides-" + ci + ' [data-index="' + index + '"]');    
    return -1;
  }
  date_offset = $date.offset().left - firstOffset;
  return date_offset;  
}

// takes canvas_index + date, returns pixel offset (x axis)
function aptindex2pos(ci, index) {
  var $apts = $("#cal_apts-" + ci);
  var aptOffset = $apts.offset().top;

  $apt = $apts.find('[data-index="' + index + '"]');
  if(!$apt.length) {
    console.log("Could not find apt for =" + index);
    return -1;
  }
  apt_index = $apt.length ? $apt.data('index') : -1;
  apt_offset = $apt.offset().top - aptOffset;
  return apt_offset;
}

// takes canvas_index + apt_id, returns pixel offset (y axis)
function apt2pos(ci, apt_id) {
  var $apts = $("#cal_apts-" + ci);
  var aptOffset = $apts.offset().top;

  $apt = $apts.find('[data-apt_id="' + apt_id + '"]');
  if(!$apt.length) {
    console.log("Could not find apt for =" + apt_id);
    return -1;
  }
  apt_index = $apt.length ? $apt.data('index') : -1;
  apt_offset = $apt.offset().top - aptOffset;
  return apt_offset;
}

function load_calendar_entries(params, $target) {  
  var $canvas = g_cal_params.canvas;
  
  if(typeof params === 'undefined') var params = {};  
  if(typeof $target === 'undefined') var $target = null;
  var target = params.target;
  
  console.log("Loading entries: ",params, " target=" + target);
  
  var status = params.status || g_cal_params.status || 1;
  var ajax_url = "/api/calendar/?start=" + g_cal_params.start_date + "&end=" + g_cal_params.end_date + "&host_id=" + g_cal_params.host_list + "&apt_id=" + g_cal_params.id_list + "&status=" + status;
  
  $.ajax({
    url: ajax_url,
    data: {}, // serializes the form's elements.
    success: function(responseText) {
      $(".cal-entries, .cal-stats").html('');
      var response = parse_json(responseText);
      if(!response) {
        response = {'success':0, 'error':'invalid result: ' + responseText};
        console.log("res=", response);
      }
      if(response.success) {
        g_obj_data = obj_data = response.events;
        g_booked = response.booked;
        g_invoiced = response.invoiced;
        g_paid = response.paid;
        console.log("Booked:", g_booked);
        var size = Object.size(obj_data);
        calendar_draw_entries(obj_data);
        
        //console.log("Entries Response from " + ajax_url + ":", response);
        
        var stats = calendar_get_stats(g_cal_params);
        calendar_print_stats(stats, 0);
    
        calendar_show(g_cal_params.show); // reservation or invoice?
        calendar_view(g_cal_params.view); // calendar or stats?
        
        if($target && $target.length) $target.fadeTo("fast", 1); // target of button than caused reload
        if(target) {
          console.log("triggering click on " + target + " with len=" + $("#" + target).length);
          $("#" + target).trigger('click'); // target reservation to hi-lite
        }
      } else {
        var error = response.error || "Failed to load data:<br>" + responseText;
        calendar_error($canvas, error);
      }
    },
    error: function(responseText) {
      alert("ajax error:" + responseText);
    }
  });
}

// returns the top/left index of a given element by id
function id2index(cache_index) {
  var $el = $('#' + cache_index);  
  if($el.length) return pos2index(parseInt($el.css('left')), parseInt($el.css('top')), g_cal_params.grid_w, g_cal_params.cell_h);
  return {left: 0, top: 0};
}

function index2hour(index) {
  // which date?
  if(g_period == 1) { // shortcut: only one day shown
    var theDate = fixDate(g_cal_params.target_year + '-' + g_cal_params.target_month + '-' + g_cal_params.target_day);    
  } else {
    if(g_vcount > 1) { // vertical stacking
      var theDate = day2date(ci);
    } else {
      var day_index = Math.floor(index / g_cells_per_day);
      var theDate = day2date(day_index);
      index -= day_index * g_cells_per_day;
    }
  }
    
  // what time?
  var theTime = index2hm(index);

  return theDate + ' ' + theTime;
}

// returns the hour/minute time for given index (ie 09:30)
function index2hm(index) {
  var hour = index2dectime(index);
  var whole_hour = Math.floor(hour);
  var rest = hour - whole_hour;
  var minutes = 60 * rest;
  var theTime = zero_pad(whole_hour) + ':' + zero_pad(minutes) + ':00';
  return theTime;
}

// returns the decimal time for given index (ie 9.5 for 09:30)
function index2dectime(index) {  
  var start = g_start_hour;
  var hour = start + index / g_cal_params.resolution;
  return hour;
}

function hideMenu() {
  var $menu = $('#cal_menu');
  $menu.hide();
  $(".cal-entries .selected").removeClass('selected');
  g_menu_cached_id = 0;  
  if(g_menu_show_id) {
    $(g_menu_show_id).removeClass('selected');
    g_menu_show_id = 0;
  }
}


// singleClick to show menu (view res)
function singleClick(e) {
  console.log('click ' + $(this).data('obj_type') + " " + $(this).data('obj_id'));
  showMenu2($(this).data('obj_type'), $(this).data('obj_id'), e);
}

// doubleClick to show dialog (edit res)
function doubleClick(e) {
  edit_reservation($(this));
}

// called for selected item via URL
function showMenu2(obj_type, obj_id, event) {
  if(g_loading_menu) return; // prevent double loading
  
  var $menu = $('#cal_menu');
  
  var cache_index = obj_type + '-' + obj_id;
  var inds = id2index(cache_index);
  
  var $target = $('#' + cache_index);
  var $canvas = $target.closest('.cal-container').find('.cal-canvas');
  var ci = $canvas.data('index');
  if(event && event.pageX) {
    var data = canvas2pos($canvas, event);
    inds.left = data.index.left; // mouse position
  }
  
  //console.log("showMenu Cache: canvas = " + $canvas.prop('id') + " index=" + ci + ' pos=' + inds.top + '/' + inds.left);
  
  cached_obj_data = g_cal_entries && Object.size(g_cal_entries) ? g_cal_entries[cache_index] : {};
  $menu.html('');  
  showMenu($canvas, cached_obj_data, inds, true);
   
  // load live
  var ajax_url = "/admin/app_data.php?oper=rental-calendar-get&obj_type=" + obj_type + "&obj_id=" + obj_id;
  g_loading_menu = true;
  $.getJSON(ajax_url, function(response) { // start loading data from server
    if(response.success) {
      obj_data = response.data;
      g_cal_entries[cache_index] = obj_data; // cache with client
      showMenu($canvas, obj_data, inds, false); // show menu with cached or no data
    } else {
      $menu.html(bootstrap_error_message(response.error));          
    }
    g_loading_menu = false; 
  }); 
}

function showMenu($canvas, obj_data, pos, cached) {
  var indX = pos.left;
  var indY = pos.top;
  var $menu = $('#cal_menu');
  var obj_id = obj_data ? obj_data.id : 0;
  var obj_type = obj_data ? obj_data.type : 0;
  
  if(cached && obj_id) {
    g_menu_cached_id = g_cal_params.base_class + '-' + obj_data.id; // cached, set until live
  } else {
    // uncomment kjetil june 2013
    //if(g_cal_params.base_class + '-' + obj_data.id != menu_cached_id) return; // prevents late showing of late data after clicking other res.
    // menu_cached_id = 0; // showing live, clear
  }

  var offset = $canvas.offset(); // if you really just want the current element's offset
  var canvasY = offset.top;

  var y = g_cal_params.standalone ? 0 : canvasY + (indY+1)*g_cal_params.cell_h;
  var x  = g_cal_params.title_w + indX*g_cal_params.grid_w;
  var coords = {left: x, top: y};
  $menu.css( { "left": x + "px", "top": y + "px" } );

  $menu.removeClass('hidden').show();

  //var apt=index2apt(indY);
  var content = obj_id ? menuContent(obj_data, cached)  : '<div class="mt10 ml10"><i class="fa fa-spinner fa-spin"></i> Loading...</div>'; // , indX, indY, cached)
  $menu.html(content);  
  var tid = '#' + obj_type + '-' + obj_id;
  g_menu_show_id = tid;

  var $menu_obj = $(tid);
  $menu_obj.removeClass('hover');
  $menu_obj.addClass('selected');
  
}

function showMenuSingle($target, obj_type, obj_id) {
  var $menu = $('#cal_menu');

  var offset = $target.offset();
  var x = offset.top;
  var y = offset.left;
  x = 0;
  y = 0;
  
  console.log(offset);
  var coords = {left: x, top: y};
  $menu.css( { "left": x + "px", "top": y + "px" } );
  $menu.removeClass('hidden').show();

  $menu.html("Loading...");  
  
  // load live
  var ajax_url = "/admin/app_data.php?oper=rental-calendar-get&&obj_type=" + obj_type + "&obj_id=" + obj_id;
  g_loading_menu = true;
  $.getJSON(ajax_url, function(response) { // start loading data from server
    if(response.success) {
      obj_data = response.data;
       content = menuContent(obj_data);
       $menu.html(content);       
    } else {
      $menu.html(bootstrap_error_message(response.error));          
    }
  }); 
  
  
}
function calendar_obj_name(obj_type) {
  switch(obj_type) {
  case 'reservation':
    return 'booking';
  case 'res_exception':
    return 'rule';
  default:
    return obj_type;
  }
}

function menuContent(obj_data, cached) { // , indX, indY, cached) {
  var obj_type = obj_data.obj_type;
  var obj_id = obj_data.id;
	//if(obj_type == 'reservation') obj_data.id = obj_data.res_id;
	
	//console.log(obj_data);
  var change_date_text = g_cal_params.target_day > 0 ? 'time' : 'dates';

  var time_str = '';
  if(obj_data.num_days > 0) time_str = delta_string_unit(obj_data.num_days, 'day'); // ' (' + obj_data.num_days + ' days)';
  else if(obj_data.num_hours > 0) time_str = delta_string_unit(obj_data.num_hours, 'hour'); // ' (' + obj_data.num_hours + ' hours)';
  
  var type_str = '';
  if(obj_data.type_name) type_str = capitalize(obj_data.type_name) + ' ';
  var create_date = obj_data.created ? sql2human_short(obj_data.created) : '';
  var status_date = '';
  if(obj_data.status_change_timestamp) {
    status_date = sql2human_short(obj_data.status_change_timestamp);
    var status_date_str = ' on ' + status_date;
  }

  var create_date_str = '';
  if(!status_date || status_date != create_date) create_date_str = 'Created on ' + create_date; // only show if diff from status date string

  // reservation/event/invoice info
  var menuTextAr = [];  
  var blockTextAr = [];  
  var auth_level = obj_data.auth_level;
  var auth_edit = auth_level >= 20;

  var edit_button = " ";
  if(!cached && auth_edit) {
    edit_button = "<a href='#' class='edit_res' data-apt_id='" + obj_data.apt_id + "' data-in='" + obj_data.checkin + "' data-out='" + obj_data.checkout + "' data-obj_type='" + obj_type + "' data-obj_id='" + obj_id + "'><i class='fa fa fa-pencil-square-o'></i></a>";
  }
  
  var wheel = cached ? " <i class='fa fa-spinner fa-spin'></i>" : edit_button;
  menuTextAr.push(wheel);

  var obj_name = calendar_obj_name(obj_type)
  
  var title = capitalize(obj_name) + ' ' + obj_data.id;
  if(obj_data.link) title = '<a href="' + obj_data.link + '" target="status">' + title + '</a>'; 
  menuTextAr.push(title);
  
  if(!cached) {    
    if(obj_data.source_name) menuTextAr.push("<b>Source: " + obj_data.source_name + "</b>");
    if(obj_data.site_name) menuTextAr.push(obj_data.site_link ? '<a href="' + obj_data.site_link + '" target="status">' + obj_data.site_name + '</a>' : obj_data.site_name);
    if(obj_data.aff_name) menuTextAr.push("via " + obj_data.aff_name);
  }
  if(obj_data.agent_name) menuTextAr.push("Agent: " + obj_data.agent_name);
  if(obj_type != 'reservation' && obj_data.res_id > 0) menuTextAr.push('Res. ID ' + obj_data.res_id);
  if(obj_data.guest_name)  menuTextAr.push('<b>' + obj_data.guest_name +  (obj_data.num_guests ?  ' ('  + obj_data.num_guests + ' pp)</b>' : '</b>'));
  if(obj_data.customer_name)  menuTextAr.push('<b>' + obj_data.customer_name +  (obj_data.customer_count ?  ' ('  + obj_data.customer_count + ' pp)</b>' : '</b>'));
  if(obj_data.name) menuTextAr.push(obj_data.name); /** org name */

  if(obj_data.apartment_name) menuTextAr.push(obj_data.apartment_name + time_str);

  if(create_date_str) menuTextAr.push(create_date_str);
  if(obj_data.status_name) menuTextAr.push(obj_data.status_name + status_date_str);

  if(obj_data.author_name) menuTextAr.push('By: ' + obj_data.author_name + (obj_data.user_type ? ' (' + obj_data.user_type + ')' : ''));

  var range = obj_data.range;
  if(range) {
    menuTextAr.push("<b>" + obj_data.range + "</b>");
  } else {
    if(obj_type == 'reservation') {
      menuTextAr.push('Checkin: ' + sql2human_short_day_time(obj_data.checkin));
      menuTextAr.push('Checkout: ' + sql2human_short_day_time(obj_data.checkout));
    } else if(obj_type == 'invoice') {
      menuTextAr.push('Start date: ' + sql2human_short(obj_data.start_date));
      menuTextAr.push('End date: ' + sql2human_short(obj_data.end_date));
    } else if(obj_type == 'res_exception') {
      menuTextAr.push('Start date: ' + sql2human_short(obj_data.start));
      menuTextAr.push('End date: ' + sql2human_short(obj_data.end));
    } else if(obj_type == 'event') { // event
      menuTextAr.push('Start: ' + sql2human_short_day_time(obj_data.start_time));
      menuTextAr.push('End: ' + sql2human_short_day_time(obj_data.end_time));
    }
  }

  //console.log("menu content: cached = " + cached + " ul=" + obj_data.auth_level);
  if(auth_edit) { 
  	if(obj_data.alert) menuTextAr.push("<div class='memo_field'>NB!<br>" + obj_data.alert + '</div>');
  	if(obj_data.notes) menuTextAr.push("<div class='memo_field'>" + nl2br(obj_data.notes) + '</div>');
  	if(obj_data.customer_notes) menuTextAr.push("<div class='memo_field'>Customer notes:<br>" + nl2br(obj_data.customer_notes) + '</div>');
  	if(obj_data.extra_info)     menuTextAr.push("<div class='memo_field'>Customer notes:<br>" + nl2br(obj_data.extra_info) + '</div>');
    // payment info reservations
    var paymentTextAr = [];
    if(obj_data.rate_base > 0) {
      var base = parseFloat(obj_data.rate_base);
      paymentTextAr.push('<b>' + ucfirst(obj_data.rate_type) + ' rate: ' + base.toFixed(2) + ' ' + obj_data.currency + '</b>');
      if(obj_data.city_tax > 0)  paymentTextAr.push('City tax: ' + obj_data.city_tax + ' ' + obj_data.currency);
      if(obj_data.fees > 0)      paymentTextAr.push('Fees: '    + obj_data.fees + ' ' + obj_data.currency);
      if(parseFloat(obj_data.discount) > 0)  paymentTextAr.push('Discount: '    + obj_data.discount);
      if(obj_data.grand_total > 0 && (obj_data.grand_total != obj_data.rate_base)) paymentTextAr.push('Total: '   + obj_data.grand_total + ' ' + obj_data.currency);
      if(obj_data.commission) paymentTextAr.push('Commission: ' + obj_data.commission + (obj_data.commission.indexOf('%') > -1 ? '' : ' ' + obj_data.currency));
    } else if(obj_type == 'invoice') { // payment info invoice
      if(obj_data.total > 0) paymentTextAr.push('Total: '   + obj_data.total + ' ' + obj_data.currency);
    }
  
    // payment: common to res + invoice
    if(obj_data.paid > 0) {
      paymentTextAr.push('Paid: '    + obj_data.paid + ' ' + obj_data.currency);
      paymentTextAr.push('Balance: ' + obj_data.balance + ' ' + obj_data.currency);
    }

    // add payment text for reservation or invoice
    if(paymentTextAr.length) menuTextAr.push("<div id='menu_payment'>" + paymentTextAr.join('<br/>') + '</div>');
    
    // block info
    // if(obj_data.block_id && !obj_data.block_parent) {
    if(obj_data.block_id) {
      if(obj_data.block_grand_total > 0) blockTextAr.push('Block Total: '   + obj_data.block_grand_total + ' ' + obj_data.currency);
      if(obj_data.block_paid > 0) blockTextAr.push('Block Paid: '   + obj_data.block_paid + ' ' + obj_data.currency);
      if(obj_data.block_balance > 0) blockTextAr.push('Block Balance: '   + obj_data.block_balance + ' ' + obj_data.currency);
      if(blockTextAr.length) menuTextAr.push("<div id='menu_block' class='border'>" + blockTextAr.join('<br/>') + '</div>');
    }
    
    var links = obj_data.links || [];
    if(!cached && links.length) {
      for(var i=0;i<links.length;i++) {
        menuTextAr.push(links[i]);
      }
    }
  }
  

  // action links
  //menuTextAr.push('<hr/>');

  //menuText = '<span>' + menuTextAr.join('</span><span>') + '</span>';
  var close = "<a href='#' class='close-button close_parent'><i class='fa fa-times-circle fa-2x'></i></div></a>";
  menuText = menuTextAr.join('<br/>');

  return close + menuText;
}


// basic extra security
// replicates auth_res in class handlers
/**
function cal_auth(data, action) {
  if(!data) return false;
  //console.log("cal_auth: action= " + action + " g_cal_params.user_level=" + g_cal_params.user_level + " g_user_id=" + g_user_id  + " g_agent_id=" + g_agent_id  + " author_id =" + data.author_id  + " agent_id=" + data.agent_id );
  if(g_cal_params.user_level < 20) return false;
  if(g_cal_params.user_level >= 30) return true;

  // todo: add the other user levels
  if(g_cal_params.user_level == 20 && data.owner_id != g_owner_id) return false;
  if(g_cal_params.user_level == 22 && data.aff_id   != g_aff_id) return false; 
  if(g_cal_params.user_level == 24 && data.contractor_id != g_contractor_id) return false; 
  if(g_cal_params.user_level == 26 && data.agent_id != g_agent_id) return false; 
  //if(g_cal_params.user_level == 30 && data.staff_id != g_staff_id) return false; 
  
  if(action == 'view') return true; // view OK 
  if(action == 'edit' || action == 'del') return g_user_id == data.author_id; // if edit
  return false; // should not come here (action not 'view', 'edit', or 'del'
}


// update the search bar corresponding to actions in the property calendar
function update_search_bar(urlParams, start, delta) {
  var time = index2dectime(start);
  $('#time').val(time);
  //$msgs.show();
  //$msgs.css('top', '0px');
  if(start+delta > g_max_xInd[0]+1) return;
  if(delta) {
    var hours = delta / g_cal_params.resolution;
    $('#nh').val(hours); 
  }
}

*/


$(document).on("submit", "#cal-search-form", function(e) {
  var val = $('#cal_search').val();
  
  var obj_type = g_cal_params.base_class || g_cal_params.obj_type;
  if(obj_type && form_input_is_int(val)) { // on this page
    var selector = '#' + obj_type + '-' + val;
    var $target = $(selector);
    //console.log(selector + " len=" + $target.length);
    if($target.length) {
      $target.click(); 
      return false;
    }
  }
  
  // search db
  
  if(val) {
    var ajax_url = '/admin/app_data.php?oper=calendar-search&obj_type=' + g_cal_params.obj_type + '&q=' + val;
    var options = {};
    var modal = "#calendar_search_dialog"; // '#calModal'

    var $title = $("#calendar_search_dialog .modal-title");
    var $table = $("#calendar_search_results");
    
    //console.log("show search dialog title len=" + $title.length + " table-len=" + $table.length);
    
    $title.html("<i class='fa fa-spinner fa-spin'></i> Searching for '" + val + "'...");
    //$table.bootstrapTable('removeAll');
    
    init_search_table();
    
    $.getJSON(ajax_url, function(result) {
      var hits = result.hits;
      var rows = result.data;
      //console.log(result);
      //console.log("Found " + hits + " matches, loading data into table len=" + rows.length);
      $title.html("Found " + hits + " matches");
      $table.bootstrapTable('load', rows);
    $(modal).modal(options);
      
    });
    
    //$('#cal_search').val(''); // reset
    //show_fancy_dialog(ajax_url, 'Search for ' + val)
  }
  return false;
});

// <a class="month-link cal_nav" data-nav="{&quot;y&quot;:&quot;2015&quot;,&quot;m&quot;:7,&quot;d&quot;:0,&quot;w&quot;:0}" href="?y=2015&amp;m=7&amp;d=0&amp;w=0">Jul</a>

function cal_search_truncate(value, row, index) {
  return truncate(value, 20);
}

function cal_search_link_formatter(value, row, index) {
  var public_link,text;
  var obj_type = row.obj_type;
  var apt_id = row.apt_id;
  var start = row.start;
  var ymd = sql2ymd(start);
  
  var id = row.id;
  var target = obj_type + '-' + id;
  if(obj_type == 'invoice') {
    var res_id = row.parent_type == 'reservation' && row.parent_id ? row.parent_id : row.res_id;
    if(res_id) target = 'reservation-' + res_id;
  }
    
  start = ymd.y + '-' + ymd.m + '-01'; /** start 1st of month */
  var link = row.link;
  var what = ' ' + ucfirst(obj_type) + ' ' + (row.invoice_id ? row.invoice_id : id);
  if(id && link) {
    public_link = link + id;
    text = '<a href="' + public_link + '" target="public"><i class="fa fa-globe"></i></a> ';
  }
  // console.log("start=" + row.start + "new start=" + start + " ymd=", ymd);
  //var calendar_link = "?y=" + ymd.y + "&m=" + ymd.m + "&d=0&w=0&target=" + obj_type + '-' + id;
  var calendar_link = "?start=" + start + "&target=" + target;
  text += '<a href="' + calendar_link + '" class="cal_nav start" data-dismiss="modal" data-status="' + row.status + '" data-value="' + start + '" data-target="' + target + '"><i class="fa fa-calendar">' + what + '</i></a>'
  return text;
}

function init_search_table() {
  $search_table = $("#calendar_search_results");
  if($search_table.length) {
    //console.log("\n\ninit search result table\n\n");
    $search_table.bootstrapTable({
     striped: true,
     pagination: true,
     columns: [{
         title: 'What',
         formatter: cal_search_link_formatter
     }, {
         field: 'name',
         title: 'Who',
         formatter: cal_search_truncate
     }, {
         field: 'apartment_name',
         title: 'Where',
         formatter: cal_search_truncate
     }, {
         field: 'start',
         title: 'When'
     }, {
         field: 'status',
         title: 'Status'
     }],
     data: []
    })
  }   
}

function calendar_search_handler(options, data) {
  //console.log("Cal search handler data=", data);
}

$(document).on("click", "#cal_search", function() {
  //console.log("click search");
  $(this).focus();
});
    
$(document).on("click", "#cal_show_help", function() {
  var ajax_url = "/admin/app_data.php?oper=rental-calendar-help";
  show_fancy_dialog(ajax_url, "Help");
  return false;
});

$(document).on("click", "#cal_show_stats", function() {
  
  var id_list = g_cal_params.apt_list;
  var cal_start = g_cal_params.start_date.date;
  var cal_end = g_cal_params.end_date.date;
  var res_list = g_cal_params.res_list;
  
  var ajax_url = "/admin/app_data.php?oper=rental-calendar-stats&res_list=" + res_list;
  //console.log("stats url=" + ajax_url);
  show_fancy_dialog(ajax_url, "Stats");
  return false;
});

$(document).on("click", "#cal_show_pricing", function() {
  var ajax_url = "/admin/app_data.php?oper=rental-calendar-pricing";
  show_fancy_dialog(ajax_url, "Pricing and Rules");
  return false;
});

$(document).on("click", "#cal_show_changes", function() {
  var $target = $(this);
  var $changes = $("#cal_changes"); 
  var res_list = g_cal_params.res_list;
  
  var obj_type = $target.data('obj_type');
  if(!obj_type) {
    console.log("No obj_type");
    return false;
  }
  
  var title = 'Changes';
  var ajax_url = "/admin/app_data.php?oper=rental-calendar-changes&obj_type=" + obj_type + "&res_list=" + res_list;
  show_fancy_dialog(ajax_url, title);
  return false;
});


/** old ? where is it used ? */
function cal_search_handler(params, data) {
  alert('not in use 1');
  return;
  //console.log("cal_search_handler params=", params);
  //console.log("cal_search_handler data=", data);
  if(!data) data = 0;
  
  var form_id = params.form;
  var inputs = $("#" + form_id + " SELECT");
  var nav = {};
  $.each(inputs, function(k, input) {
    var $input = $(input);
    //console.log("input=", $input);
    var name = $input.attr("name");
    var val = $input.val();
    //console.log(name + " = " + val);
    if(val) nav[name] = val;
  });

  nav.apt_list = data.join();
  //console.log("cal_search_handler nav=", nav);
  
  load_cal(nav, $("#cal_reload"));  
  
}

// New handler for template based search _calendar-search.html
function calendar_search_handler(params, data) {
  alert('not in use 2');
  return;
  load_cal(data, $("#cal_reload"));  
}

function calendar_reload() {
  console.log("Calendar reload...");
  var $target = $("#cal_reload");
  if($target.length) { /** multi calendar */
    console.log("multi calendar..");
    $target.fadeTo("fast", 0.33);  
    load_calendar_entries({}, $target);
  } else { /** single calendar */
    console.log("single calendar..");
    fc_refresh();    
  }
}

$(document).on("click", ".cal_nav", function() {
  var nav = $(this).data('nav');
  if(!nav) return;  
  load_cal(nav, $(this));  
  return false;  
});

$(document).on("mouseup", ".cal-guide .guide-box", function(evt) {
  $('.cal-apt').removeClass('active');
});

// returns index of absolute position in canvas
function abs2index(x, y) {
  return pos2index(x - canvasX[ci], y - canvasY[ci], g_cal_params.grid_w, g_cal_params.cell_h);
}

// returns difference between start and current (pos or index)
function delta_index(start, current) {
  return {left: current.left - start.left, top: current.top - start.top};
}

// returns index of absolute position in canvas
function abspos2index(pos) {
  return pos2index(pos.left - canvasX[ci], pos.top - canvasY[ci], g_cal_params.grid_w, g_cal_params.cell_h);
}

// returns index of x/y position relative to grid (0,0 is top left) with grid size w/h
function pos2index(x, y, w, h) {
	return {left: p2i(x,w), top: p2i(y,h)};
}

function p2i(x, w) {
  return parseInt(Math.floor(x/w));
}

// converts absolute screen mouse position to canvas relative position
function rel_pos(pos) {
  return {left: Math.floor(pos.left - canvasX[ci]), top: Math.floor(pos.top - canvasY[ci])}
}



/** latest code */

function calendar_get_dates(start, period) {
  var result = {"start_date": start, "period": period};
  
  var months=[];
  var days=[];
  var days_in_month, i,j, dt, wd,my;
  var date = start;
  var day_count = 0, month_count=0;
  var month = [];
  var months = {};
  
  
  console.log("calendar_get_dates start:", start);
  for(i=0;i<period;i++) {
    my = date.substr(0,7); // extract month/year: '2017-01'
    days_in_month = moment(my, "YYYY-MM").daysInMonth() // days in month 29
    //console.log("Period: " + i + " Days in " + my + '=' + days_in_month);
    for(j=0;j<days_in_month;j++) {
      dt = moment(date, "YYYY-MM-DD");
      wd = dt.format('E');
      my = date.substr(0,7); // could have changed if we don't start on the 1st
      if(!months[my]) {
        months[my] = [];
        month_count++;
        //console.log("changed month to " + my);
      }
      
      month = months[my];
      
      day = {'date': date, 'day': dt.format('D'), 'wd': wd, 'weekday': g_weekdaysMin[wd]};
      month.push(day);
      
      date = add_day(date,1);
      day_count++;
    }
    start_month = date.substr(0,7);

  }
  
  result.months = months;
  result.num_months = month_count;
  result.num_days = day_count;
  result.end_date = date; // end date is day after last shown. add_day(date, -1);
  return result;
}

/** print stats table guide */
function calendar_stats_guide(fields, dates, index) {
  var items = [];
  var fld,box,row,title,subtitle;
  var tax_text = "<span class='tax-incl-text'>incl. VAT</span>";
  /** print guide */
  for (var fld in fields) {
    //fld = fields[i];
    title = fields[fld];
    subtitle = $.inArray(fld, ['rate','rent','net']) > -1 ? tax_text : '';
    box = '<div class="guide_title cal-guide-font">' + title + '<br>' + subtitle + '</div><div class="guide-box"></div>';
    item = '<li class="cal_guide cal_day col_w">' + box + '</li>';
    items.push(item);
  }
  //console.log("Dates:",dates);
 
  var checked =  g_cal_params.tax_included  ? 'checked' : '';  
  var guide_title = "<span class='pull-right'>Incl. VAT? <input class='cal-stats-tax' type='checkbox' " + checked + "></span>";
  guide_title += dates.start_date + " - " + dates.end_date + " (" + dates.num_days + ' days)';
  console.log("tax incl=" + g_cal_params.tax_included);
  // guide_title += "<span class='pull-right'>Incl. VAT? <input class='cal-stats-tax' type='checkbox' " + checked + "></span>";
  var cols = '<ul class="cal-guide-day striped-light nowrap">' + items.join("\n") + '</ul>';
  var period = '<li class="cal-month" id="cal_month-' + index + '-0">' + guide_title + cols + '</li>';
  var list = '<div class="guide cal-months"><ul id="cal_tables-' + index + '"class="month-list striped nowrap">' + period + '</ul></div>';
      
  return list;
}

$(document).on('change', '.cal-stats-tax', function(e) {
  var checked = $(this).prop('checked');
  var $text = $(".tax-incl-text");
  console.log("stats tax checked =" + checked);
  if(checked) {
    $text.html("incl. VAT");
    $('.cal-tax-incl').show();
    $('.cal-tax-excl').hide();
    g_cal_params.tax_included = 1;
  } else {
    $text.html('excl. VAT');
    $('.cal-tax-incl').hide();
    $('.cal-tax-excl').show();
    g_cal_params.tax_included = 0;
  }
});

function calendar_guide(dates, index) {
  var months = dates.months;
  var cal_days, item, day;
  var items = [];
  var start_index = 0, i=0;
  for (var month in months) {
    days = months[month];
    cal_days = calendar_guide_days(days, index, start_index);
    item = '<li class="cal-month" id="cal_month-' + index + '-' + i + '" data-index="' + i + '" data-date="' + month +'">' + month + cal_days + '</li>';
    items.push(item);
    start_index += days.length;
    i++;
  }
  var list = '<div class="guide cal-months"><ul id="cal_guides-' + index + '" class="month-list striped nowrap">' + items.join("\n") + '</ul></div>';
  //console.log("returning guide list:", list);
  return list;
}

function calendar_guide_days(days, index, start_index) {
  var item,day,date,weekday,weekend, today;
  var items = [];
  for (i = 0; i < days.length; i++) {
    day = days[i];
    weekend = day.wd == 6 || day.wd == 7 ? 1 : 0; 
    today = day.date == g_today ? 1 : 0;
    day_link = '<a href="?start=' + day.date +'" class="cal_nav start" data-value="' + day.date + '">' + day.day + '</a>';
    item = '<li id="cal_guide-' + index + '-' + (i + start_index) + '" data-index="' + (i + start_index) + '" data-date="' + day.date +'" class="cal_guide cal_day cell_w' + (weekend ? ' weekend' : '') + (today ? ' today' : '') + '">' +
      '<div class="guide_title cell_w_font">' + day_link + '<br>' + day.weekday + '</div><div class="guide-box"></div></li>';
    items.push(item);
  }
  var list = '<ul class="cal-guide-day striped-light nowrap">' + items.join("\n") + '</ul>';
  //console.log("returning guide list:", list);
  return list;
}



// return comma separated list of rental IDs
function calendar_rental_list(rentals) {
  var apts = [];
  for (i = 0; i < rentals.length; i++) {
    rental = rentals[i];
    apts.push(rental.id);    
  }
  return apts.join(',');
}

function calendar_rentals(rentals, index) {
  var item,rental,rental_link;
  var items = [];
  for (i = 0; i < rentals.length; i++) {
    rental = rentals[i];
    rental_link = rental.link ? '<a href="' + rental.link + '" target="rental">' + rental.name + '</a>' : rental.name;
    item = '<li id="cal_apt-' + index + '-' + i + '" data-index="' + i + '" data-apt_id="' + rental.id + '" data-apt=\'' + JSON.stringify(rental) + '\' class="cal-apt"><div class="rental-checkbox collapse"><input type="checkbox" class="cal-rental-checkbox" data-3id="' + rental.id + '"></div><span class="sortable-handle"><i class="fa fa-sort"></i></span><div class="calendar-property-title">' + rental_link + '</div></li>';
    items.push(item);
  }
  var list = '<ul id="cal_apts-' + index + '" class="striped">' + items.join("\n") + '</ul>';
  // console.log("returning list:", list);
  return list;
}

$(document).on('click', '.cal_nav', function(e) {
  $(this).closest('ul.cal_menu').find('li a').removeClass('active');
  $(this).addClass('active');
});

/** handlers for menu */
$(document).on('click', '.cal-apt-select-box', function(e) {
  var checked = $(this).prop('checked');
  var $text = $('.cal-apt-select-text');
  if(checked) {
    $('.rental-checkbox').show()
    $text.show();
  } else {
    $('.rental-checkbox').hide()
    $text.hide();
  }
});

$(document).on('change', '.cal-rental-checkbox', function(e) {
  var $selected = $('input.cal-rental-checkbox:checked');
  var count = $selected.length;
  var $text = $('.cal-apt-select-text');
  console.log(count + " checked text=" + $text.length);
  if(count) {
    $text.html("<a href='#' class='cal-remove-rentals'>Click to remove (" + count + ")</a>");
  } else {
    $text.html('');
  }
});

$(document).on('click', '.cal-remove-rentals', function(e) {
  var $selected = $('input.cal-rental-checkbox:checked');
  var $text = $('.cal-apt-select-text');
  var count = $selected.length;
  var $el, id;
  if(count) {
    $.each($selected, function(i, el) {
      $el = $(this);
      id = $el.data('id');
      console.log("removing " + id);
      $el.closest('.cal-apt').remove();
    });
    $text.html('');
    var $ul = $("#admincal_full").find('.admincal .cal-apts UL');
    calendar_rental_reindex($ul, true);
    $('.cal-apt-select-box').trigger('click');
  }
});


$(document).on('click', '.cal_nav.start', function(e) {
  var $target = $(this);
  var start = $target.data('value');
  var target = $target.data('target');
  var status = $target.data('status');

  var nav = {'start': start};
  if(target) nav.target = target;
    
  if(status && status < g_cal_params.status) {
    nav.status = status;
    g_cal_params.status = status;    
  }
  console.log("cal_nav start, status=" + status + ' cal status=' +  g_cal_params.status + " target=" + target + " nav:", nav);
  
  load_cal(nav, $target, g_timeout_interval);
  return false;
});

$(document).on('click', '.cal_nav.reload', function(e) {
  var $target = $(this);
  if($target.length) $target.fadeTo("fast", 0.33);  
  load_calendar_entries({}, $target);
  return false;
});

$(document).on('click', '.cal_nav.status', function(e) {
  var $target = $(this);
  var status = $target.data('value');
  g_cal_params.status = status;
  console.log("reload cal entries with status=" + status);
  load_calendar_entries({"status": status}, $target);
  return false;
});

/** toggle view: calendar/stats */
$(document).on('click', '.cal_nav.view', function(e) {
  var $target = $(this);
  var view = g_cal_params.view = $target.data('value');
  if(view == 'cal') {
    $target.removeClass('active');
    $target.attr('href', '?view=stats').data('value', 'stats');
  } else {
    $target.attr('href', '?view=cal').data('value', 'cal');
  }
  calendar_view(view);
  return false;
});


/** show/hide invoice or reservation bars */
function calendar_show(show) {
  $(".resBar").hide();
  console.log("show:" + show);
  $(".resBar." + show).show(); // show res/inv
}

/** toggle show: reservation/invoice */
$(document).on('click', '.cal_nav.show', function(e) {
  var $target = $(this);
  var show = g_cal_params.show = $target.data('value');
  if(show == 'reservation') {
    $target.removeClass('active');
    $target.attr('href', '?show=invoice').data('value', 'invoice');
  } else {
    $target.attr('href', '?show=reservation').data('value', 'reservation');
  }
        
  calendar_show(show);
  return false;
});


/** show/hide bars or stats, sets width */
function calendar_view(view) {
  $(".admincal .calview").hide();
  $(".admincal .calview."+view).show(); // show stats/cal
  
  // set width of cal
  var num_cols = view == 'stats' ? g_cal_params.num_fields : g_cal_params.num_days;
  var cell_w = view == 'stats' ? g_cal_params.col_w : g_cal_params.grid_w;

  var w = cell_w * num_cols; // canvas width   
  var cal_w = w + g_cal_params.title_w; // calendar width - includeds rentals (title)

  console.log("view:" + view + " cols:" + num_cols + " cell_w:" + cell_w + " cal_w:" + cal_w);

  $('#calendar').width(cal_w);
  $('.admincal').width(cal_w);  
  $('.cell_w').width(cell_w); // day width
}

$(document).on('click', '.cal_nav.previous,.cal_nav.next,.cal_nav.previous2,.cal_nav.next2', function(e) {
  var $target = $(this);
  var start;      
  var start_date = g_cal_params.start_date;
  console.log("cal_nav next/prev, start=" + start_date);
  if($target.hasClass('previous') || $target.hasClass('next')) { 
    start = add_month(start_date, $target.hasClass('previous') ? -1 : 1);
    load_cal({'start': start}, $target, g_timeout_interval);
  } else if($target.hasClass('previous2') || $target.hasClass('next2')) { 
    start = add_year(start_date, $target.hasClass('previous2') ? -1 : 1);
    load_cal({'start': start}, $target, g_timeout_interval);
  }
  return false;
});

$(document).on('click', '#cd-cancel', function(e) {
  console.log("Dialog cancel");
  $('#new_entry').remove();
});

$(document).on('click', '.cal_nav.period', function(e) {
  var $target = $(this);
  var period = $target.data('value');
  var cell_w = g_cal_params.org_w;
  var font_size;
  if(period >= 6) {
    cell_w = 8;
    font_size = 6;    
  } else if(period >= 4) {
    cell_w = 12;
    font_size = 9;
  } else if(period >= 3) {
    cell_w = 16;
    font_size = 11;
  } else {
    font_size = 12;
  }
  g_cal_params.cell_w = g_cal_params.grid_w = cell_w;
  g_cal_params.day_font_size = font_size;
  load_cal({'period': $target.data('value')}, $target);
  
  return false;
});

/** reindex rentals after re-ordering/removing rentals */
function calendar_rental_reindex($ul, reload) {
  $apts = $ul.find('li');
  var apts = [];
  var rentals = [];
  var rental,id, $apt;
  $.each($apts, function(i, apt) {
    $apt = $(this);
    id = 'cal_apt-0-' + i;
    rental = $apt.data('apt');
    //console.log("i=" + i + " len=" + $apt.length + " data:", rental);    
    $apt.data('index', i);
    $apt.attr('id', id);
    apts.push(rental.id);    
    rentals.push(rental);    
  });

  var id_list = apts.join(',');
  console.log("reindex list=" + id_list);
  g_cal_params.id_list = id_list;
  g_cal_params.rentals = rentals;

  if(reload) {
    load_cal();
  } else {
    load_calendar_entries({});
  }  
}

/**
+-----+------------+--------+------------------------------------------------------------------------------+
| id  | name       | color  | description                                                                  |
+-----+------------+--------+------------------------------------------------------------------------------+
| -20 | Refunded   | 666666 | You refunded the payment.                                                    |
| -10 | Cancelled  | 888888 | We canceled the invoice (e.g. reservation was cancelled).                    |
|  -1 | Draft      | ffcc00 | Draft - payments can not be accepted.                                        |
|   1 | Open       | ff8800 | Voucher - not a formal invoice, but payments can be accepted.                |
|  10 | Sent       | cc0000 | The invoice has been sent.                                                   |
|  20 | In dispute | 000000 | There is a dispute over this invoice.                                        |
|  30 | Partial    | 0066ff | A partial payment has been accepted (but not full payment).                  |
|  40 | Pending    | 0000cc | Paid in full, but one or more payments are pending (check or authorization). |
|  50 | Complete   | 008800 | The invoice was paid in full.                                                |
+-----+------------+--------+------------------------------------------------------------------------------+

| -40 | Expired   | 444444 | default | Expired due to lack of response.         |
| -30 | Rejected  | 666666 | default | Rejected by guest                        |
| -20 | Cancelled | 888888 | default | Cancelled by guest or staff              |
| -10 | Declined  | aaaaaa | default | Declined by owner as unavailable         |
|   1 | Draft     | ffff00 | warning | Draft request                            |
|   5 | Inquiry   | 888800 | warning | Inquiry sent by guest                    |
|  10 | Request   | ff8800 | warning | Requested by guest                       |
|  15 | Proposed  | 0066ff | primary | Request accepted by owner.               |
|  20 | Pending   | 0066ff | info    | Paid by guest, pending owner acceptance. |
|  30 | Reserved  | cc0000 | danger  | Confirmed Reservation                    |
|  35 | Blocked   | 880000 | danger  | Blocked by owner                         |
|  40 | Complete  | 008800 | success | Paid in full                             |
*/

function calendar_status_name(obj_type,status) {
  switch(obj_type) {
  case 'invoice':
    switch(status) {
      case -20 : return 'refunded';
      case -10 : return 'cancelled';
      case 1 : return 'open';
      case 10 : return 'sent';
      case 20 : return 'dispute';
      case 30 : return 'partial';
      case 40 : return 'pending';
      case 50 : return 'complete';
      default: return '';
    }
    break;
  case 'reservation':
    switch(status) {
      case -40 : return 'expired';
      case -30 : return 'rejected';
      case -20 : return 'cancelled';
      case -10 : return 'declined';
      case 1 : return 'request';
      case 5 : return 'request';
      case 10 : return 'inquiry';
      case 15 : return 'proposed';
      case 20 : return 'pending';
      case 30 : return 'reserved';
      case 35 : return 'blocked';
      case 40 : return 'complete';
      default: return '';
    }
    break;
  default:
    return '';
  }
  return '';
}

/** easily spoofed, do not trust in backend */
function calendar_auth(obj) {
  var host_ids = g_cal_params.host_ids || [];
  var user_id = g_cal_params.user_id;
  var user_level = g_cal_params.user_level;
  //console.log("Auth ul:" + user_level + " host ids:",host_ids);
  return obj.user_id == user_id || $.inArray(obj.host_id, host_ids) > -1 ? user_level : 0;
}

/** draw reservation bars using data from ajax */
function calendar_draw_entries(obj_data) {
  var ci = 0; /** todo: get from canvas */
  var $entries = $("#cal_entries-" + ci);

  var grid_w = g_cal_params.grid_w;
  var cell_h = g_cal_params.cell_h;
  var cell_w = g_cal_params.cell_w;
  var res_x_offset = grid_w / 2;
  var res_y_offset = cell_h / 4;
  var res_end_offset = grid_w / 4;
  var resArray = [];
  var canvas_w =  g_cal_params.canvas_w;
  var bar_h = g_cal_params.bar_h;
  var num_cols = g_cal_params.num_cols;

  
  /** this because canvas_w / cell_w are not correct atm */
  var last_date = add_day(g_cal_params.end_date, -1);
  var real_canvas_w = date2pos(ci, last_date);
  var real_cell_w = real_canvas_w / (num_cols - 1);
  real_canvas_w += real_cell_w;
  console.log("47005 canvas_w:" + canvas_w + " real: " + real_canvas_w + " cell_w: " + cell_w + " real: " + real_cell_w + " last:" + last_date);
  
  console.log("47005 canvas_w:" + canvas_w + " cols:" + num_cols + " cell_w:" + cell_w + " WxC=" + num_cols * cell_w);
  
  $.each(obj_data, function(i, obj) {
      
    var cal_start = g_cal_params.start_date;
    var cal_end = g_cal_params.end_date;
    var title = obj.title;
    var obj_type = obj.obj_type;
    var obj_id = obj.obj_id;
    var apt_id = obj.apt_id;
    
    var status = obj.status;
    var status_name = calendar_status_name(obj_type,status);
    var alert = obj.alert;
      
    var tips = obj.tips || '';
    
    //console.log("obj=",obj);
    if(obj_type == 'reservation' && apt_id != 0) title = title + " (" + obj.num_guests + "pp)";
    if(alert) {
      var alert_text = alert;
      alert = alert.replace('"', '&quot;');
      alert = alert.replace('<', '&lt;');
      alert = alert.replace('>', '&gt;');
      tips = tips + "<a class='afr-tooltip cal-alert tooltip-jq' data-toggle='tooltip' title='" + alert_text + "' href='#'><i class='fa fa-info-circle'></i></a>";
    }
    
    if(obj.start) obj.checkin = obj.start;
    if(obj.end) obj.checkout = obj.end;
    
    var cache_index = obj_type + '-' + obj_id;
    g_cal_entries[cache_index] = obj; // cache with client
    
    var start = strip_time(obj.start);
    var end = strip_time(obj.end);
    var apt_id = obj.apt_id;


    
    
    var start_offset = (start < cal_start) ? 0 : date2pos(ci, start) + res_x_offset;
    var end_offset = (end >= cal_end) ? real_canvas_w : date2pos(ci, end);
    var res_width = end_offset - start_offset;
    var extra_width = end >= cal_end ? 0 : res_end_offset;        

    
    var apt_offset = apt_id ? apt2pos(ci, apt_id) : 0;
          
    /** The crucial bit - position, width */
    //var num_cells = end_index - start_index;        
    var top = apt_offset;
    if(obj_type != 'res_exception') top = top + res_y_offset;
    
    var left = start_offset;
    
    var width = end_offset - start_offset + extra_width;
    //console.log(" style=" + style);

    //console.log(obj_type + ' ' + obj_id + ': '+ title + "@" + apt_id + ": in=" + start + " out=" + end + " cal_end:" + cal_end + " end_offset: " + end_offset + " left" + left + " width:" + width + " nd=" + obj.num_days + (start > cal_start && end < cal_end ? " cw=" + width/obj.num_days : ''));
    
    var auth_level = calendar_auth(obj);
    var auth_edit = obj.auth_edit = auth_level > 1 ? 1 : 0;
    var auth_view = obj.view = auth_edit || obj.auth_view > 0 ? 1 : 0;
    
    var height = bar_h;
    var barClass = 'resBar';        
    switch(obj_type) {
    case 'reservation':
      resArray.push(obj.id);
      break;
    case 'res_exception':
      height = apt_id ? cell_h : g_cal_params.canvas_h;
      style = style + 'height:' + height + 'px;';
      break;
    case 'invoice':
      var auth_edit = obj.auth_edit = 0;
      break;
    }
      
    var style = 'height: ' + height + 'px;width:' + width + 'px;top:' + top + 'px;left:' + left + 'px;display:block;';
    

    //console.log("res: ", obj);
    //console.log("auth_level: " + auth_level + " edit:" + auth_edit);
    
    var classAr = [barClass, obj_type, status_name];
    if(auth_view) classAr.push('auth_view');

    /** add classes to allow move/resize if auth edit */
    if(auth_edit) {
      classAr.push('auth_edit');
      
      // drag
      if(start < cal_start && end > cal_end) { 
        classAr.push('drag_y');
      } else if (start < cal_start) {
        classAr.push('drag_y');
        classAr.push('resize_e');  
        classAr.push('round3-right');
      } else if (end > cal_end) {
        classAr.push('drag_y');
        classAr.push('resize_w');
        classAr.push('round3-left');
        
      } else {
        classAr.push('drag_xy');
        classAr.push('resize_ew');            
        classAr.push('round3');
      }
    }
    
    //if(1) classAr.push('drag_x');        
    //var classes = 'resBar reservation ' + status_name + '  auth_edit'; // mismatch drag_y round3-left resize_w';

    var classes = classAr.join(' ');
    var resbar_id = obj_type + '-' + obj_id;
    //var res_text_style = 'style="width: 633px"'; /** do we need this ? */

    if(obj_type == 'res_exception') {
      console.log("Exception:" + obj_type + ' ' + obj_id + ': '+ title + "@" + apt_id + ": in=" + start + " out=" + end + " cal_end:" + cal_end + " end_offset: " + end_offset + " left" + left + " width:" + width + " height: " + height + " style=" + style);
    }
    
    var resBar = '<div data-obj_id="' + obj_id + '" data-obj_type="' + obj_type + '" data-apt_id="' + apt_id + '"data-in="' + start + '" data-out="' + end + '" class="' + classes + '" id="' + resbar_id + '" style="' + style + '">' + tips + '<div class="res-text">' + title + '</div></div>';
    $entries.append(resBar);
  });
  
  $canvases = $('.cal-canvas');
  $.each($canvases, function(index) {
    var $canvas = $(this);
    console.log("creating handlers for " + $canvas.prop('id'));    
    create_entry_handlers($canvas);
  });  
}

/** return 12 month links for under calendar */
function month_links(start) {    
    var startAr = start.split('-');
    var start_year = startAr[0];
    var start_month = startAr[1];
    var monthAr = [];
    var month,this_month,month_date, month_name, m, year;
    var this_m = parseInt(start_month, 10);
    var start_m = (12 + (this_m - 3)) % 12;
    var year = start_year;
    if(this_m - 3 < 0) year--;
    
    for(var i=start_m;i<12+start_m;i++) {
      if(i==12) year++;      
      m = i%12;
      if(this_m == m+1) {
        month_name = g_months[m];
        month = '<b>' + month_name + '</b>';
      } else {
        month_name = g_monthsShort[m];     
        month_date = year + "-" + zero_pad(m+1) + "-01";
        month = '<a href="?start=' + month_date +'" class="cal_nav start" data-value="' + month_date + '">' + month_name + '</a>';
      }
      monthAr.push(month);
    }
    return monthAr.join(' ');
}


function calendar_get_stats(data) {
  var dates = data.dates;
  
  var fields = data.fields;
  var stats = {};
  var rentals = {};
  var num_apts = data.num_rows - 1; // substract first row
  
  var obj_data = g_obj_data;
  var size = Object.size(obj_data);
  var i =0;
  var apt_id,start,end,row,nd;
  const range1 = moment.range(dates.start_date, dates.end_date);
  var num_days = range1.diff('days');
  var range2, overlap, overlap_count, overlap_fraction, days,com_str,com_base,commission;
  var total, total_ex, total_incl, rent, rent_ex, rent_incl, net, net_ex, rate_ex, rate_incl, rate;
  var net, net_ex, net_incl, gt, grand_total, booked,invoiced,city_tax;
  
  //console.log("range1 days=" + num_days + " range:", range1);
  //console.log("Stats data:", obj_data);
  //console.log("Stats end date=", dates.end_date);
  
  /** todo: use fields */
  var blank_row =  {'days' : 0, 'occupancy': 0, 'rent': 0, 'rent_ex': 0, 'commission': 0, 'gt': 0, 'booked': 0, 'invoiced': 0, 'paid': 0, 'city_tax': 0};
  var total_row = rentals[0] = clone_object(blank_row);
  var tax_rate = 1.06; /** todo: depends on country */ 
  
  
  /** Loop through bookings, count days stayed */
  $.each(obj_data, function(i, data) {
    apt_id = data.apt_id;
    start = data.start;
    end = data.end;
    nd = data.num_days;
    total = data.total;
    grand_total = data.grand_total;
    if(tax_rate) {
      total_ex = data.charge_tax && data.tax_included ? Math.round(total/tax_rate) : total;   
      total_incl = data.charge_tax && data.tax_included ? total : Math.round(total*tax_rate);
    } else {
      total_ex = total_incl = total;   
    }
    
    if(!data.apt_id) return true;
    if($.inArray(data.status, [30,40]) < 0) return true;
    if(start >= dates.end_date || end <= dates.start_date) return true; /** outside visible dates */
    
    range2 = moment.range(start, end);
    overlap = range1.intersect(range2);
    if(!overlap) {
      console.log("NO OVERLAP - SHOULD NOT HAPPEN with start:"+start + " end:" + end);
      return true;
    }
    if(!nd) {
      console.log("NO num_days",data);
      return true;
    }
    if(!total) {
      console.log("NO total",data);
      return true;
    }
    
    if(!rentals[apt_id]) rentals[apt_id] = clone_object(blank_row);
    row = rentals[apt_id];
    
    overlap_count = days = overlap.diff('days');   // days overlaps
    overlap_fraction = overlap_count/nd; // fraction of entire booking
    rate_incl = total_incl/nd; // average rate per day (excl. fees, city tax)
    rate_ex = total_ex/nd; // average rate per day (excl. fees, city tax)
    rate = total/nd;    
    
    rent = overlap_count * rate;
    rent_ex = overlap_count * rate_ex;
    rent_incl = overlap_count * rate_incl;
    gt = overlap_fraction * grand_total;
    
    if(data.charge_city_tax && data.city_tax > 0) {
      city_tax = overlap_fraction * data.city_tax;
    }
    
    
    //console.log("ct=" + data.charge_tax + " ti=" + data.tax_included + " total:" + total + " total ex:" + total_ex + " rate_ex:" + rate_ex + " rent_ex:" + rent_ex);
    //console.log("data grand_total=" + grand_total + " fraction=" + overlap_fraction + " gt" + gt);

    com_str = data.commission;
    if(com_str) {
      com_base = com_str.indexOf('%') > -1 ? (parseFloat(com_str)/100) * rent : parseFloat(com_str);
      commission = com_base * overlap_fraction;    
      //console.log("Commission: " + com_str + " of " + rent + " = " + com_base);
      //console.log(" => " + commission + " fraction:" + overlap_fraction + " (" + overlap_count + "/" + nd + ")");
    }

    /** per rental */
    row['commission'] += Math.round(commission);
    row['days'] += days; // count    
    row['rent'] += Math.round(rent_incl);     
    row['rent_ex'] += Math.round(rent_ex);    
    row['gt'] += Math.round(gt); // grand total    
    row['city_tax'] += Math.round(city_tax); // city tax   
    
    /** total on top */
    rentals[0]['days'] += days;
    rentals[0]['rent'] += Math.round(rent_incl);
    rentals[0]['rent_ex'] += Math.round(rent_ex);
    rentals[0]['commission'] += Math.round(commission);
    rentals[0]['gt'] += Math.round(gt); // sum grand total
    rentals[0]['city_tax'] += Math.round(city_tax); // sum grand total
    
    //console.log(i + ": " + data.obj_type + "-" + data.obj_id + " apt:" + apt_id + " st:" + data.status + " in:" + start + " end:" + end + " overlap:" + overlap_count);
    i++;
  });
  
  /** Loop through booking made in this period */
  $.each(g_booked, function(i, data) {
    apt_id = data.apt_id;
    if(!rentals[apt_id]) rentals[apt_id] = clone_object(blank_row);
    row = rentals[apt_id];
    row['booked'] += Math.round(data.grand_total); // grand total    
    rentals[0]['booked'] += Math.round(data.grand_total);
    //console.log("Booked apt_id=" + apt_id + " gt=" + data.grand_total + " date:" + data.booked_timestamp);
  });
  
  /** Loop through payments made in this period */
  $.each(g_paid, function(i, data) {
    apt_id = data.apt_id;
    if(!rentals[apt_id]) rentals[apt_id] = clone_object(blank_row);
    row = rentals[apt_id];
    row['paid'] += Math.round(data.amount); // grand total    
    rentals[0]['paid'] += Math.round(data.amount);
    //console.log("Paid apt_id=" + apt_id + " gt=" + data.amount + " date:" + data.payment_date);
  });
  
  /** Loop through invoices sent in this period */
  $.each(g_invoiced, function(i, data) {
    apt_id = data.apt_id;
    if(!rentals[apt_id]) rentals[apt_id] = clone_object(blank_row);
    row = rentals[apt_id];
    row['invoiced'] += Math.round(data.total);     
    rentals[0]['invoiced'] += Math.round(data.total);
    console.log("Invoiced apt_id=" + apt_id + " gt=" + data.total + " date:" + data.invoie_date);
  });
  
  //rentals[0]['total'] = total_row.total;
  
  /** Loop through stats, calculate averages, total */
  
  //console.log("looping through rentals:",rentals);
  
  var total, avg_days, avg_rate, ex_class, collapse;
  for (apt_id in rentals) {    
    row = rentals[apt_id];
    days = row['days']; // count
    rent_incl = row['rent']; // rent
    rent_ex = row['rent_ex']; // rent excl. vat    
    gt = row['gt']; // rent excl. vat    
    com = row['commission'] || 0; // commission
    net = rent - com;
    net_ex = rent_ex - com;
    net_incl = rent_incl - com;
    
    avg_days = days/num_days;
    rate = rent/days;
    rate_incl = rent_incl/days;
    rate_ex = rent_ex/days;
    
    if(apt_id == '0' || !apt_id) {
      avg_days = avg_days/num_apts;
    }

    row['occupancy'] = Math.round(avg_days*100) + '%';
    ex_class = Math.round(rate_incl) == Math.round(rate_ex) ? ' text-danger' : '';
    row['rate'] = '<span class="cal-tax-incl' + (g_cal_params.tax_included ? '' : ' collapse') + '">' + Math.round(rate_incl) + '</span><span class="cal-tax-excl' + (g_cal_params.tax_included ? ' collapse' : '') + ex_class + '">' + Math.round(rate_ex) + '</span>';
    row['rent'] = '<span class="cal-tax-incl' + (g_cal_params.tax_included ? '' : ' collapse') + '">' + Math.round(rent_incl) + '</span><span class="cal-tax-excl' + (g_cal_params.tax_included ? ' collapse' : '') + ex_class + '">' + Math.round(rent_ex) + '</span>';
    row['net']  = '<span class="cal-tax-incl' + (g_cal_params.tax_included ? '' : ' collapse') + '">' + Math.round(net_incl)  + '</span><span class="cal-tax-excl' + (g_cal_params.tax_included ? ' collapse' : '') + ex_class + '">' + Math.round(net_ex) + '</span>';
    row['total']  = Math.round(gt);
    row['booked']  = Math.round(row['booked']);
    row['invoiced']  = Math.round(row['invoiced']);
    row['paid']  = Math.round(row['paid']);
    row['city_tax']  = Math.round(row['city_tax']);
    
    //row['net_ex'] = rent_ex - com;
    //row['rate_ex'] = Math.round(avg_rate_ex);
    
    //console.log("apt_id:" + apt_id + " total:" + total + " occ:" + occ);
  }
  
  stats.rentals = rentals;
  console.log(size + "bookings stats:",stats);
  return stats;
}


/** print the stats */
function calendar_print_stats(stats, ci) {
  var fld,row,span,i,apt,fld, top,left;
  var rentals = stats.rentals;
  var fields = g_cal_params.fields;
  
  //console.log("Stats:",stats);
  //var $apts = $("#cal_apts-" + index);
  var $stats = $("#cal_stats-" + ci);
  
  for (apt_id in rentals) {
    //$apt = $apts.find('[data-apt_id="' + apt_id + '"]');
    row = rentals[apt_id];
    left = 0;
    //console.log("apt_id: " + apt_id + " row:", row, " apt length:" + $apt.length);
    for (fld in fields) {
      //console.log("apt_id: " + apt_id + " fld:" + fld + " val:" + row[fld]);
      //console.log("apt:" + apt_id + ' top:' + top + ' left:' + left);
      top = apt2pos(ci, apt_id);
      cls = apt_id == '0'? 'cal-cell header' : 'cal-cell';
      span = '<span class="' + cls + ' col_w" style="top:' + top + 'px;left:' + left + 'px">' + row[fld] + '</span>';
      left += g_cal_params.col_w;
      //$apt.html('foo').css('border:2px solid blue');
      $stats.append(span);
    }
  }
}

function calendar_change_date() {
  var date = $("#cal-current").datepicker('getDate');
  console.log("Change date to " + date);
}


$(document).ready(function() {
  if($('#cal_parent').length) {
    calendar_init();
    init_search_table();
  }
});

// this is the main calendar init function
function calendar_init() { 
  var data = g_data = $("#admincal_full").data('data'); /** default data from PHP controller */

  var start = data.start;  
  var start_date = moment(start).format(g_moment_format);
  $("#cal-current").datepicker('setDate', start_date).on('changeDate', function(e) {
    var date = e.date;
    var start_sql = date2sql(date);
    console.log("Inline Change date to ", date,"=" + start_sql + " event:", e);
    load_cal({'start': start_sql});
  });

  // stats fields
  var fields = {'days':'Nights', 'occupancy':'Occ%', 'rate':"Rate", 'rent':'Rent', 'commission':'Comm.', 'net': "Net", 'total': "Total", 'booked': "Booked", 'invoiced': "Invoiced", 'paid': "Paid", 'city_tax': "City Tax"};
  g_cal_params.fields = fields;
  g_cal_params.num_fields = Object.size(fields);
  
  var host_ids = g_data.host_ids || [];
  // copy  
  g_cal_params.user_level = g_data.user_level;
  g_cal_params.host_ids = host_ids;
  g_cal_params.host_list = host_ids.join(',');
  g_cal_params.user_id = g_data.user_id;
  
  g_cal_params.tax_included = 1;
  
  load_cal({});
}


/** load and draw calendar + entries */
function load_cal(nav, $target, timeout) {
  if(typeof nav === 'undefined') var nav = {};  
  if(typeof timeout === 'undefined') var timeout = 0;  
  hideMenu();

  console.log("load_cal:", nav);
  
  
  var data = g_data;
  var view = g_cal_params.view || data.view;
  var show = g_cal_params.show || data.show;
  var period = g_cal_params.period || data.period;
  var start = g_cal_params.start || data.start;
    
  if(!g_cal_params.rentals) g_cal_params.rentals = data.rentals; /** only needed first time */

  
  //console.log("load_cal: start=" + start);

  if(nav.period) {
    g_cal_params.period = period = nav.period;
    nav.start = g_cal_params.start;
  }

  if(nav.start) {
    g_cal_params.start = start = nav.start;
    var start_human = sql2human(start);
    console.log("start human=" + start_human);
    $('#cal-current').data({date: start_human}).val(start_human);
    $('#cal-current').datepicker('update');    
  }
    
  
  var rentals = g_cal_params.rentals;
  var $calendar, $apts, $guide, $canvas;
  var month_guide, day_guide;
  
  // get days from start date
  var dates = g_cal_params.dates = calendar_get_dates(start, period);
    
  var months = dates.months;  
  var apt_list = calendar_rental_list(rentals);
  
  g_cal_params.num_cols = g_cal_params.num_days = dates.num_days;
  g_cal_params.id_list = apt_list;
  g_cal_params.start_date = dates.start_date;
  g_cal_params.end_date = dates.end_date;
  g_cal_params.period = dates.period;
    
  var cell_w = g_cal_params.cell_w;        
  var cell_h = g_cal_params.grid_h;
  
  //console.log("days from " + start + " for period=" + period + ":", dates);
  
  // draw rentals, guide for each calendar
  var $calendars = $("#admincal_full").find('.admincal');
  var guide, num_rows, num_cols, table, $table, $guide;
  
  $.each($calendars, function(index) {
    $calendar = $(this);
    $apts = $calendar.find('.cal-apts');     
    $guide = $calendar.find('.cal-guide');
    $table = $calendar.find('.cal-table');
    
    // get dates + rentals

    
    // draw stats table
    table = calendar_stats_guide(g_cal_params.fields, dates, index);
    $table.html(table);
    
    //tz = fields.length;
        
    
    /** if stats 
    g_cal_params.cell_w = cell_w = g_cal_params.col_w;
    */
    
    guide = calendar_guide(dates, index);
    $guide.html(guide);
    $('.cell_w_font').css('font-size', g_cal_params.day_font_size + 'px');
    $('.cell_w').css('width', g_cal_params.cell_w + 'px');
    console.log("setting " + $('.cell_w').length + " items to " + g_cal_params.cell_w + 'px');
    
    $apts.html(calendar_rentals(rentals, index));
    var $ul = $apts.find("UL");
    $ul.sortable({ 
       axis: 'y', 
       stop: function( event, ui ) {
         calendar_rental_reindex($ul);
       } 
    });
    
    
    // get height / width 
    // var h = $("#cal_apts-0").height(); // canvas height
    num_rows = rentals.length;
    num_cols = dates.num_days;

    var h = cell_h * num_rows; // canvas height
    var canvas_h = num_rows * g_cal_params.cell_h; // canvas height

    var w = cell_w * num_cols; // canvas width   
    var canvas_w = num_cols * g_cal_params.cell_w; // canvas height
    var cal_w = w + g_cal_params.title_w; // calendar width - includeds rentals (title)
                
    // set screen dims
    $('.guide-box, .cal-canvas').height(h); // vertical stripes + canvas

    //$('#calendar').width(cal_w); // horizontal stripes + rental names
    //$('.admincal').width(cal_w); // horizontal stripes + rental names

    //console.log("rows:" + num_rows + " cell_h=" + cell_h + " H= " + h + " W=" + w + " cal_w=" + cal_w + " guide_h=" + guide_h + " title_h=" + guide_title_h + " cell_w=" + cell_w + "   et=" + guide_title_h);
    
    // draw month links
    var links = month_links(start);
    $('.month-links').html(links);
    
    // store global data    
    var size = {"top": num_rows, "left": num_cols}; // rows/cols
    $canvas = $calendar.find('.cal-canvas');
    
    $canvas.data('size', size);
    $canvas.data('index', index);

    calendar_view(g_cal_params.view); // calendar or stats, sets width/height
    
    g_cal_params.num_rows = num_rows;
    g_cal_params.num_cols = num_cols;

    g_cal_params.canvas_h = canvas_h;
    g_cal_params.canvas_w = canvas_w;    
    
    $('.cal-canvas').width(w); // canvas = booking bar area
    g_cal_params.canvas = $canvas; /** future, allow multiple ? */
  });  
  
  
  //if(!load_entries) return;

  $(".cal-entries, .cal-stats").html('<div class="mt10 ml10"><i class="fa fa-spinner fa-spin"></i> Loading...</div>.');
  
  /** print stats */
  
  /** print entries */
  var options = {};
  if(nav.target) options.target = nav.target;
  if(timeout) {
    if(g_timeout) {
      console.log("clear timeout");
      clearTimeout(g_timeout);
    }    
    g_timeout = setTimeout(function() {
        load_calendar_entries(options);
    }, timeout)
    //console.log("Timeout= " + g_timeout_interval);
  } else {
    //console.log("No timeout");
    load_calendar_entries(options);    
  }
}


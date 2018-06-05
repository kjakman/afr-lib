console.log("loaded event-form.js v4.3");

function event_handler(params, data) {
  console.log("event_handler params:", params, " data:", data);
  if($('#agenda_calendar').length) $('#agenda_calendar').fullCalendar('refetchEvents');  
}

// populate event form 
function event_form_set(data) {
  if(typeof data == "undefined") var data = {};
  var form_id = "event-form";
  $("#" + form_id + " .messages").remove();  
  
  var event_id = data.event_id;
  
  var reset = data.reset || !data.id;
  if(reset) { // add event
    reset_form('event-form', false);
    $("#event_user_level").val(0);
  }
  
  fill_form('event-form', data);

  var all_day = $("#all_day").is(':checked');
  var is_event = $("#periodpicker").data('event') == 1 ? 1 : 0;
  
  var start_date = data.start_time;
  var end_date = data.end_time;
  //console.log("Event form show start=" + start_date + " end=" + end_date + " all_day=" + all_day + " reset=" + reset);
  if(all_day && is_event && end_date > start_date) {
    end_date = add_day(end_date, -1);  // use day before for events    
    $("#end_time").val(end_date);
  }
  
  var $range = $("#periodpicker");
  periodpicker_init($range, all_day);
}
            
$(function() {
  console.log("Event page ready");

  // delete confirmation
  $('[data-toggle="confirmation"]').confirmation({singleton:true});
  
  // maps    
  $(".mapmodals").on("click", function () {
    var $target = $(this);
    var lat = $target.data("lat").replace(',', '.');
    var lng = $target.data("lng").replace(',', '.');
    var address = $target.data("address");
    var title = $target.data("title");
    console.log(".mapmodals click: lat=" + lat + " lng=" + lng + " title=" + title + " adress=" + address);
    if(lat && lng) {
      $("#mapmodals").data($target.data());
      console.log("lat=" + lat + " lng=" + lng);
    }    
  });
  
  $(document).on("click", ".event-form-open", function(e) {
    var data = $(this).data('data');
    var reset = $(this).data('reset') || data.reset ? 1 : 0;        
    event_form_set(data);
  });
  
  $(document).on("change", "#all_day", function() {
    var all_day = $(this).is(':checked');
    var start_time = $('#start_time').val();
    var end_time = $('#end_time').val();
    
    var $range = $("#periodpicker");
    periodpicker_init($range, all_day);
    
    $('#start_time').val(start_time);
    $('#end_time').val(end_time);      
    $('#start_time').periodpicker('change');
    
    console.log("All day change=" + all_day + " start=" + start_time + " end=" + end_time);
 });
});


$(document).on('shown.bs.modal', '#mapmodals', function() {
  var $target = $(this);
  var lat = $target.data("lat").replace(',', '.');
  var lng = $target.data("lng").replace(',', '.');
  var address = $target.data("address");
  var title = $target.data("title");
  var data = $target.data();
  console.log(data);

  if(lat && lng) {
    myLatlng = new google.maps.LatLng(lat,lng);
    console.log("show modal lat=" + lat + " lng=" + lng + " title=" + title + " adress=" + address);
    var title = $target.data("title");
    $("#myCity").html(title);
    var map = gmap3_update("map_canvas", lat, lng, title, address);
  }
});


/**
function period_picker_update($range, all_day) {
  var $start = $range.find('input').eq(0);
  var start_id = $start.length ? $start.attr('id') : '';
  if(start_id) {
    $('#' + start_id).periodpicker('change');
  }
}
*/

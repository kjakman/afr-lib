/** new clean (?) javascript for rentals */
var g_map_timeout = 100;
var g_infowindow = null;

if(typeof g_deployment == "undefined") var g_deployment = '';
console.log("afr_rental: deployment = " + g_deployment);

function property_search_handler(params, data) {  
  if($("#search_result_map").length) rental_map_update(data); // hard coded map id here  
}

function property_photo_handler(params, data) {
  window.console && console.log("property_photo_handler params=", params);
  window.console && console.log("property_photo_handler data=", data);
  var file = null;
  if(data && data.files) file = data.files[0];
  window.console && console.log("property_photo_handler file=", file, " type=" + (file ? file.type : 'none'));
  if(file && file.type == 'image') {
    var src = file.url;
    console.log("src=" + src + " target len=" + $("#property_profile_photo").length);
    $("#property_profile_photo").attr('src', src);
  }
  return true;
}
  
function booking_form_update() {
  var cin = $("#sbd_in").val();
  var cout = $("#sbd_out").val();
  var ng = $("#sbd_ng").val();
  var qstr = "in=" + cin + "&out=" + cout + "&ng=" + ng;
  // $("#sbd_q").val(qstr); /** for debugging */
  console.log("booking_form_update: str=" + qstr);
  
  var $rr_links = $(".rr-link");
  $.each($rr_links, function(i,link) {
    var data = $(this).data('value');
    var href = $(this).attr("href");
    var parts = [];
    if(data) {
      parts = data.split('?');
      $(this).data('value', parts[0] + '?' + qstr);
      // console.log("data-value=" + parts[0] + '?' + qstr);
    } else {
      parts = href.split('?'); 
      $(this).attr('href', parts[0] + '?' + qstr);
      // console.log("href=" + parts[0] + '?' + qstr);
    }
  });
  
  var apt_id = $("#sbd_apt_id").val();
  var $form = $("#booking-form");
  //var sql_in = moment(cin).format(g_moment_format);
  var today = moment().format(g_moment_sql_format);
  var moment_in = moment($("#sbd_in").val(), g_moment_format).format(g_moment_sql_format);
  var moment_out = moment($("#sbd_out").val(), g_moment_format).format(g_moment_sql_format);
  //console.log("today =" + today);
  if(moment_in && moment_out && moment_in >= today && moment_out > moment_in && ng > 0) {  
    var submitting = $form.data('submitting');
    //console.log("OK: in=" + cin + " out=" + cout + " ng=" + ng + " sql:" + moment_in + " submitting=" + submitting);
    if(submitting) {
      //console.log("already submitting...");
      return;
    }
    $form.data('submitting', 1);
    $("#rr-spinner").show();
    $("#rr-message").html('').hide();
    $("#rr-table").hide();
    
    $("#rr-check-availability").hide();
    $("#rr-book-now").hide();
    $("#rr-request").hide();
    
    var data = {"apt_id": apt_id, "in": moment_in, "out": moment_out, "ng": ng};
    var operation = $form.data('handler');
    var handler = $form.data('success_handler');
    console.log("op=" + operation);
    if(operation) {
      var ajax_url = "/home/app_data.php?oper=" + operation;    
      $.getJSON(ajax_url, data, function(json) {
        $form.data('submitting', 0);          
        $("#rr-spinner").hide();
        success = json.success;
        var message = json.message;
        var error = json.error;
        console.log("Result=", json);            
        if(success) {
          //if(!message) message = "OK!";
          if(message) $("#rr-message").html(bootstrap_success_message(message)).show();
          if(handler) {
            console.log("Handler=" + handler);
            success_handler({"success_handler": handler}, json.data);            
          }
        } else {
          $("#rr-check-availability").show();          
          if(!error) error = "Unknown reason";
          var msg = bootstrap_error_message(error);
          $("#rr-message").html(msg).show();
        }
      });
    }
  }
}

// print template on the right, update the map on the left 
function rental_map_update(data, map_id) {
  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var options = $map.data('options');
  if(typeof options == "undefined") var options = {};
  var map = $map.data('map');

  if(!data) return;

  var rentals = data.rentals || [];
  var count = rentals.length;
  var bounds = data.bounds;
  var lang = data.language;
  
  console.log("Ok, returned with " + count + " rentals total count=" + data.total_count + " bounds:", bounds + " lang=" + lang);
  
  template_load("#property-list", "_rental-list.html", "rental_search", {"language": lang}, data)

  map_remove_markers(map_id);
  var gmarkers = map_add_markers(rentals);

  var loc_types = $('#loc_types').length ? $('#loc_types').html() : '';
  if(loc_types && bounds) {
    console.log("rental_map_update: loc_types=" + loc_types);
    map_add_locations(loc_types, JSON.stringify(bounds)); // load other markers
  }
}

/** update price/availability of single rental given checkin/out */
function rental_rate_update(cin, cout, $item, json, org_price) {
  var $price = $item.find("span.price-amount");
  var $currency = $item.find("span.currency");
  var $status = $item.find("div.listing-status");
  var $discount_div = $item.find("div.discount-div");
  var $discount = $item.find("span.price-discount");
  var $discount_rate, $check;
  
  var data = $item.data('data');
  var type = $item.data('type');
  var id   = $item.data('id');
  var fid  = $item.data('fid');
  var cid  = cin + cout + id;

  var mid  = type + '-' + id; /** google map marker array key */
  
  success = json.success;
  var message = json.message;
  var error = json.error;
  if(success) {
    var data = json.data;
    var cur = currency2symbol(data.currency);
    var avg = data.average;
    if(!data || !(avg > 0)) {
      $price.html(org_price);      
      console.log("Returning:  No price for " + id + " json:",json);
      return;
    }
    
    var avg_round = Math.round(avg);
    var discount = data.discount;
    var avgd = data.average_discounted;
    var avgd_round = avgd > 0 ? Math.round(data.average_discounted) : 0;
    var book_direct = data.book_direct;
    var gm = map_set_marker(mid, 'labelContent', cur + '' + avg_round);            
    $currency.html(cur);
    $price.html(avg_round);
    
    console.log("discount=" + discount + " avgd=" + avgd_round);
    if($discount_div.length && discount && avgd_round > 0) {
      $discount_rate = $discount_div.find(".discount-rate");
      $discount_div.show();
      $price.addClass("overstrike");
      $discount.html("-" + discount);
      $discount_rate.html(avgd_round);
      console.log("setting dis v2 len=" + $discount_rate.length + " to " + avgd_round + " dislen=" + $discount_div.length);
      $check = $discount_div.find("span.discount-status");
    } else {
      $discount_div.hide();
      $price.removeClass("overstrike");
      $discount.html('');
      //$discount_rate.html('');
      $check = $item.find("span.price-status");
    }

    $check.html("<i class='fa fa-check text-success'></i>");
    
  } else if(error) {
    $price.html(org_price);
    console.log("error=" + error + " org=" + org_price);
    $status.html("<span class='text text-danger'><i class='fa fa-exclamation-triangle'></i> " + error + "</span>");
  } else { // shouldn't happen
  }
  
  /** dynamically update info window - not done */
  if(0 && gm && gm.m) {
    
    var infowindow = gm.infowindow;
    if(infowindow) {
      var content = gmap3_info_window_content(gm.m, {});
      //console.log("Found iw for " + cid + ":",content);
      infowindow.setContent(content);
    } else {
      //console.log("No iw for " + cid);
    }
      
  }
}

/** for some reason doesn't work if array - check */
function map_add_locations(loc_types, bounds, map_id) {
  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var options = $map.data('options');
  if(typeof options == "undefined") var options = {};
  var map = $map.data('map');

  var bounds_str = typeof bounds == 'array' ? JSON.stringify(bounds) : bounds;
  console.log("map_add_locations types=", loc_types);
  console.log("map_add_locations bounds=", bounds);
  console.log("map_add_locations bounds_str=" + bounds_str + " id=" + map_id);
  var ajax_url = "/home/app_data.php?oper=location-search&loc_types=" + loc_types + "&bounds=" + encodeURIComponent(bounds_str);
  console.log("find-locations url:", ajax_url);
  $.getJSON(ajax_url, function(response) {
    console.log("find-locations returned:", response);
    if(response.success) {
      var locs = response.locs;      
      console.log("Adding markers for locations:" + locs.length);
      map_add_markers(locs, map_id);

      console.log("resize map2 map:", map);
      google.maps.event.trigger(map, 'resize');
      
    } else {
      console.log("Response failure:", response);
    }
  });
}

function map_info_window(marker, map_id) {
  
  var base_link, content = ''; 
  var obj = marker.object;
  var object_id = obj.id;
  
  var name = obj.name;
  var type = marker.type;

  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var options = $map.data('options');
  if(typeof options == "undefined" || !options) var options = {};
  
  var base_links = options.base_links || {};
  var base_link = base_links[type]; 
  
  //console.log("base-links:", base_links);
  switch(type) {
    case 'shortstay':
      if(!base_link) base_link = obj.site_id==263 ? "/golf-homes/" : "/serviced-apartments/";
      break;
    case 'golf_club':
      if(!base_link) base_link = "/golf-club/";
      object_id = obj.parent_id;
      break;
    default:
      break;
  }
  
  var link = marker.link = base_link ? base_link + object_id : '';
  var title = link ? '<a href="'+ link +'" class="property-detail-link">' + name + '</a>' : name;
  
  if (marker.images) { // array of image urls, show slider
    json_data = marker.images;
    var images = jQuery.parseJSON(json_data);
    var first = images[0] || {};
    
    content = content  
    +'  <div class="listing-img media-photo" unselectable="on" style="-webkit-user-select: none;">'
    +'    <div class="listing-img-container media-photo image-rotation" data-current="0" '
    +'         data-images=\''+ json_data + '\'>'
    +'      <img src="' + first.filename + '" class="rental-list-photo" data-url="' + marker.link + '">'
    +'    </div>';
    if(images.length > 1) {
      content = content  
      +'    <div class="image-rotation-control block-link image-rotation-prev">'
      +'      <i class="icon icon-chevron-left listing-slideshow-chevron">&lt;</i>'
      +'    </div>'
      +'    <div class="image-rotation-control block-link image-rotation-next">'
      +'      <i class="icon icon-chevron-right listing-slideshow-chevron">&gt;</i>'
      +'    </div>'
    }
    content = content +'  </div>';

  } else if(marker.image) {
    if(marker.link) content = content + '<a href="' + marker.link + '" class="property-detail-link"><img width="100%" src="' + marker.image + '"></a>';
    else content = content + '<img width="100%" src="' + marker.image + '">';
  }
  
  content += '<h3>' + title + '</h3>';
  if(obj.price) content += obj.address_city + "<span class='pull-right'>" + currency2symbol(obj.currency) + obj.price + "</span><br>"; 
  if(obj.extra) content += obj.extra;
  
  if(0) {
    var desc = obj.short_description ? obj.short_description : obj.description;
    if(desc) {
      var max = 255;
      var elipsis = desc.length > max ? "..." : '';
      content += '<p><small>' + desc.substr(0, max) + elipsis + "</small></p>";
    }
  }
  console.log("obj:",obj);
  var rating = obj.review_score > 0 ? obj.review_score : 0; // convert from 10 based to 5 based
  if(rating > 0 && rating <= 5) {
    var stars = star_rating(rating);
    content = content + '<div class="star-rating">' + stars + '<div>';
  }
  
  return '<div class="gmap_info_content air-item">' + content + '</div>';
  
}

// return font-awesome stars for 5-based rating (0-5);
// set show_empty to show empty stars and half-empty stars
function star_rating(rating, show_empty) {
  if(typeof show_empty == "undefined") var show_empty = false;
  rating = parseFloat(rating);
  if(rating < 0 || rating > 5) return '';  
  var star='', stars = '', star_class='';
  for(var i=1;i<=5;i++) {
    if(rating > 0.75) star_class = 'star';    
    else if(rating > 0.25) star_class = show_empty ? 'star-half-o' : 'star-half'; 
    else star_class = show_empty ? 'star-o' : ''; 
    
    if(star_class) {
      star = '<i class="fa fa-' + star_class + '"></i>';
      stars = stars + star;
    }
    rating -= 1;
  }
  return stars;
}

// print the markers
function map_add_markers(objs,map_id) {
  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var options = $map.data('options');
  if(typeof options == "undefined") var options = {};
  var map = $map.data('map');
  
  var gmarker,mid;
  var gmarkers = $map.data('gmarkers') || [];
  var gmarkersObj = $map.data('gmarkersObj') || {};
  var marker;
  
  $.each(objs, function(i, obj) {
    //console.log('marker obj=', obj);
    marker = map_marker(map, obj);
    marker.object = obj;
    var gmarker = map_gmarker(map, marker);
    gmarkers.push(gmarker);
    gmarkersObj[marker.id] = gmarker; // associate "array"
    
  });
  $map.data('gmarkersObj', gmarkersObj);    
  $map.data('gmarkers', gmarkers);    

  return gmarkers;
}


// turn our object into marker options
function map_marker(map, obj) {
  var loc = {lat: parseFloat(obj.location_lat), lng: parseFloat(obj.location_long)};
  var type = obj.type ? obj.type : obj.parent_type;

  
  var images = obj.media_json;
  var opts = {};
  var id = obj.id;
  
  var priority = 0;
  if(obj && obj.priority) priority = parseInt(obj.priority);
  
  var mid = type + '-' + id;
  var markerOpts = {
    id: mid,
    type: type,
    position: loc,
    map: map,
    images: images,
    title: obj.name,
    zIndex: 100 + priority
  };
  
  if(type == 'shortstay') {
    var price = obj.price = obj.rate_day;
    var label = currency2symbol(obj.currency) + price;
    if(obj.book_direct) {
      obj.extra = '<i class="fa fa-bolt red"></i> Book Direct';
      label = label + '&#9889;';
    }
    markerOpts.labelContent = label;
    markerOpts.raiseOnDrag = true;
    markerOpts.labelContent = label;
    markerOpts.labelClass = "white bluebg"; // the CSS class for the label
    markerOpts.labelInBackground = false;
    markerOpts.icon = svgSymbol('bubble', 'blue');
    markerOpts.labelAnchor = new google.maps.Point(-10, -2)
    
  } else {
    var icon = '';
    switch(type) {
      case 'golf_club':
        id = obj.parent_id;
        icon = "/images/map-icons/png/sport_golf.n.16.png";      
        break;
      case 'airport':
        id = obj.parent_id;
        icon = "/images/map-icons/png/transport_airport.n.16.png";      
        break;
      default:
        break;      
    }
    if(icon) markerOpts.icon = icon;    
  }
  
  return markerOpts;
}

function map_gmarker(map, marker) {
  
  var obj = marker.object;
  var name = obj.name;
  
  if(marker.labelContent) {
    var gmarker = new MarkerWithLabel(marker);
  } else {
    var gmarker = new google.maps.Marker(marker);
  }

  // add info-window
  var contentString = map_info_window(marker);
  var infowindow = new google.maps.InfoWindow({
    content: contentString
  });
  
  google.maps.event.addListener(gmarker, 'click', function() {    
    if (g_infowindow) g_infowindow.close();
    g_infowindow = infowindow;
    var mid = gmarker.id;

    // remove other selects
    map_reset_markers();
    
    // hilite this left and right
    map_hilite_marker(gmarker, 1)
    
    if($("#" + mid).length) {
      $("#" + mid).addClass('selected');
      $('html,body').animate({
         scrollTop: $("#" + mid).offset().top - 100
      });    
    }
    console.log("click on " + mid);
    infowindow.open(map, gmarker);
  });
  return gmarker;
}

function map_remove_markers(map_id) {
  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var options = $map.data('options');
  if(typeof options == "undefined") var options = {};
  var map = $map.data('map');
  
  ///var gmarkers = $map.data('gmarkers');
  var gmarkers = $map.data('gmarkers');
  
  console.log("removing " + gmarkers.length + " markers");  
  for(var i = 0; i < gmarkers.length; i++) {
    gmarkers[i].setMap(null);
  }
  
  $map.data('gmarkersObj', {});    
  $map.data('gmarkers', []);    
}

// print the map
function rental_map(map_id) {
  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var options = $map.data('options');
  if(typeof options == "undefined" || !options) var options = {};
  //console.log("rental_map options:", options);
  var bounds = $map.data('bounds');
  if(!bounds.length) {
    console.log("rental_map: no bounds, returning");
    return null;
  }
  
  console.log("rental_map: bounds=", bounds, " options:", options);
  
  if(typeof options == "undefined") var options = {};

  var center = bounds2center(bounds);
  var radius = bounds.length ? bounds2radius(bounds) : 2002;
  var zoom = radius2zoom(radius);    
  var visible = options.visible = $map.is(":visible"); 
  console.log("New map: radius=" + radius + " zoom=" + zoom + " type=" + type);
  
  var type = options.type == 'satellite' ? 'hybrid' : 'roadmap';

  // 
  var map = new google.maps.Map(document.getElementById(map_id), {
    center: {lat: center[0], lng: center[1]},
    zoom: zoom,
    mapTypeId: type
  });


  $map.data('map', map);

  map.addListener('projection_changed', function() {
    console.log("projection changed");
    map_loaded(map);    
  });
  
  map.addListener('dragend', function() {
    map_moved(map);      
  });
  
  map.addListener('zoom_changed', function() {
    map_moved(map);      
  });
    
  /** custom controls */
  var centerControlDiv = document.createElement('div');
  var centerControl = new CenterControl(centerControlDiv, map);

  centerControlDiv.index = 1;
  map.controls[google.maps.ControlPosition.TOP_RIGHT].push(centerControlDiv);
  
  return map;
}

function map_loaded(map, map_id) {
  var live = true;
  console.log("\n\n\nmap_loaded: deployment = " + g_deployment);
  if(g_deployment && g_deployment != 'live') live = false;
  
  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);

  var bounds = $map.data('bounds');
  var radius = bounds.length ? bounds2radius(bounds) : 2002;
  console.log("Search bounds: ", bounds);

  var actual_bounds = map_get_bounds(map);
  var actual_radius = bounds2radius(actual_bounds);
  console.log("Actual bounds: ", actual_bounds);
  
  console.log("Search radius: " + radius + " bounds:" + JSON.stringify(bounds));
  console.log("Actual radius: " + actual_radius + " bounds:" + JSON.stringify(actual_bounds));
  console.log("Difference: " + ((parseInt(actual_radius)/parseInt(radius)) - 1) * 100 + "%");
  
  // draw box
  if(bounds.length && !live) {
    
    console.log("Drawing rectangle...\n\n\n");
    var rectangle = new google.maps.Rectangle({
      strokeColor: '#FF0000',
      strokeOpacity: 0.8,
      strokeWeight: 2,
      fillColor: '#FF0000',
      fillOpacity: 0.35,
      map: map,
      bounds: {
        north: parseFloat(bounds[1][0]),
        south: parseFloat(bounds[0][0]),
        east: parseFloat(bounds[1][1]),
        west: parseFloat(bounds[0][1])
      }
    });
  }
    
}


function map_moved(map) {
  var auto_search = $('#map-auto-refresh-checkbox').is(':checked') ? 1 : 0;
  var bounds = map_get_bounds(map);
  var zoom = map.getZoom();
  var radius = bounds.length ? bounds2radius(bounds) : 2002;
  
  var bounds_str = JSON.stringify(bounds);
  console.log("map_moved:" + bounds_str + " search=" + auto_search + " radius=" + radius + " zoom=" + zoom);
  $(".location-bounds").val(bounds_str);
  map_timer = setTimeout(function() {
    var zoom = map.getZoom();
      
    console.log("update map - zoom=" + zoom);
    if(auto_search) $("#rental_search_form").submit();
  }, g_map_timeout);
  
}

function map_get_marker(marker_id, label, map_id) {
  if(typeof map_id == "undefined") var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var gmarkers = $map.data('gmarkersObj');
  
  //console.log("markers for map len=" + $map.length + " =", gmarkers);
  return gmarkers ? gmarkers[marker_id] : null;
}

// labelContent
function map_set_marker(marker_id, variable, value) {
  var gm = map_get_marker(marker_id);
  if(gm) {
    console.log("Found gmarker type=" +  typeof gm);
    if(gm.labelContent) { // MarkerWithLabel
      gm.set(variable, value);
    } else {
      console.log("Found gmarker type=" + typeof gm + " NOT LABEL");
    }
  } else {
    console.log("Did not find gmarker " + marker_id);
  }
  return gm;
}

function map_reset_markers(map_id) {
  var map_id = 'search_result_map';
  var $map = $('#' + map_id);
  var gmarkers = $map.data('gmarkersObj');
  var count = Object.size(gmarkers);
  
  $(".air-item").removeClass('selected'); // right side
  
  //console.log("resetting " + count + " markers");  
  $.each(gmarkers, function(mid, gm) {
    map_hilite_marker(gm, 0)
  });
  
}

function map_hilite_marker(marker_or_id, hilite) {
  var gm = typeof marker_or_id == 'object' ? marker_or_id : map_get_marker(marker_or_id);
  
  if(gm) {
    var obj = gm.object;
    var priority = 0;
    if(obj && obj.priority) priority = parseInt(obj.priority);
    
    //console.log("Found gmarker hilite=" + hilite + ' prio=' + priority);
    if(gm.labelContent) { // MarkerWithLabel
      //gm.setZIndex(hilite ? 1000 : 100);
      if(hilite) gm.setZIndex(1000 + priority);
      else gm.setZIndex(100 + priority);
      
      var lclass = hilite ? "white redbg" : "white bluebg";
      var licon = hilite ? svgSymbol('bubble', 'red') : svgSymbol('bubble', 'blue');
      gm.set("labelClass", lclass);
      gm.set("icon", licon);
    } else {
      var icon = gm.icon; 
      //console.log("Found gmarker icon=" + icon);
      //var pin = icon ? gm.icon_hi : gm.icon_lo;
      //var type = gm.type;
      //gm.setIcon(pin);
    }

  }
}

/**
 * The CenterControl adds a control to the map that recenters the map on
 * Chicago.
 * This constructor takes the control DIV as an argument.
 * @constructor
 */
function CenterControl(controlDiv, map) {

  // Set CSS for the control border.
  var controlUI = document.createElement('div');
  controlUI.style.backgroundColor = '#fff';
  controlUI.style.border = '2px solid #fff';
  controlUI.style.borderRadius = '3px';
  controlUI.style.boxShadow = '0 2px 6px rgba(0,0,0,.3)';
  controlUI.style.cursor = 'pointer';
  controlUI.style.marginBottom = '22px';
  controlUI.style.textAlign = 'center';
  controlUI.title = 'Check to automatically search as you move the map';
  controlDiv.appendChild(controlUI);

  // Set CSS for the control interior.
  var controlText = document.createElement('div');
  controlText.style.color = 'rgb(25,25,25)';
  controlText.style.fontFamily = 'Roboto,Arial,sans-serif';
  controlText.style.fontSize = '16px';
  controlText.style.lineHeight = '38px';
  controlText.style.paddingLeft = '5px';
  controlText.style.paddingRight = '5px';
  controlText.innerHTML = '<input type="checkbox" checked id="map-auto-refresh-checkbox"> &nbsp;<label for="map-auto-refresh-checkbox"><small>Search as I move the map</small></label>';
  controlUI.appendChild(controlText);

  // Setup the click event listeners: simply set the map to Chicago.
  //controlUI.addEventListener('click', function() {
  //  map.setCenter(chicago);
  //});

}

/** rental search */

/** hi-lite */
$(document).on("mouseover", ".air-item", function() {
  map_reset_markers();
  map_hilite_marker($(this).prop("id"), 1);
});

$(document).on("mouseleave", ".air-item", function() {
  //map_hilite_marker($(this).prop("id"), 0);
});

/** search link - append vars from form */
/** todo: return from rental_search and append to actual link */
$(document).on("click", "A.property-detail-link", function() {
  var href = $(this).attr('href');
  var url = href  + rentalform2qs();
  console.log("property-detail-link new url=" + url);
  window.location = url;
  return false;
});

$(document).on("click", ".rental-list-photo", function() {
  var url = $(this).data('url');
  if(url) window.location = url + rentalform2qs();
});

function rentalform2qs() {
  var $form = $("#rental_search_form");
  var formData = $form.find(":input").filter(function () { /** remove empty, 0, and loc_types */
      return $.trim(this.value).length > 0 && $.trim(this.value) != "0" && this.name != "__sl";
  }).serialize();
  var id_list = []; //$("#rental_list").data("id_list") || '';
  var $items = $('.air-item');
  $.each($items, function(i) {    
    id_list.push($(this).data('id'));
  });
  console.log("rentalform2qs list=", id_list);
  return query_string = "?" + formData + "&id_list=" + id_list;  
}

$(document).on("click", ".map-zoom-out", function() {
  var target = "search_result_map";
  var $target = $("#" + target);
  console.log("Zooming out " + target + " len=" + $target.length);

  if($target.length) {
    var options = $target.data('options');
    var map = $target.data('map');
    console.log("Map options=", options);
    
    var bounds = map_get_bounds(map);
    var zoom = map.getZoom();

    if(!zoom) {
      console.log("Can't zooming out " + target + " zoom=" + zoom);
      return true; // fall back to reload page
    } else if(zoom > 1) {
      console.log("Zooming out checked=" + $('#map-auto-refresh-checkbox').attr('checked'));
      $('#map-auto-refresh-checkbox').prop('checked', true);
      var newZoom = parseInt(zoom) - 1;
      console.log("Zooming out " + target + " found zoom=" + zoom + " newZoom=" + newZoom + " checked=" + $('#map-auto-refresh-checkbox').attr('checked'));
      map.setZoom(newZoom);
    }
    return false;
    //map.setZoom(1);
  }
  return true;
});


/** rental details */
/**
$(document).ready(function() {
  if($('#sbd_in').length) {
    if($('#datepicker_options').length) {
      var dp_json = $('#datepicker_options').html();
      var dp_options = jQuery.parseJSON(dp_json);
      db_options = jQuery.extend(g_dp_options,dp_options);
      $("#sbd_in").datepicker('remove');
      $('#sbd_in').datepicker(dp_options);

      delete dp_options.daysOfWeekDisabled; // only apply changeover to checkin for now
      $("#sbd_out").datepicker('remove');
      $('#sbd_out').datepicker(dp_options);
      
    }
  }
});
*/   

$(document).on('shown.bs.tab', 'a[data-toggle="tab"]', function (e) {
  var target = $(e.target).attr("href");
  console.log("dynamic tab clicked - target=" + target);
  switch(target) {
  case "#rental-calendar":
    var $fullcalendar = $('#fullcalendar');
    if($fullcalendar.length) {
      rental_init_calendar($fullcalendar);
    }
    break;
  case "#rental-map":
    var $map = $('#map-canvas');
    if ($map.length) {
      map = init_rental_map($map);  
      var types = $map.data('types') || '';
      
      var bounds = map_get_bounds(map);
      bounds = bounds_extend(bounds, 5000);
      
      map_add_locations(types, JSON.stringify(bounds), 'map-canvas');
      
    }
    break;
  default:
    break;
  }
  return true;
});


$(document).on("click", ".rental-previous,.rental-next", function() {
  var $nav = $(this).closest('nav');
  var url = $nav.data('detailurl');
  var $header = $("#rental_detail_header");

  if($nav.length && $header.length) {
    var id = $header.data('id');
    var data = $nav.data('data');
    var id_list = data.id_list;
    var ids = id_list.split(',');
    var index = ids.indexOf(id.toString());
    
    console.log("id=" + id + " count=" + count + " index = " + index + " list=", ids);
    if(index < 0) index = 0; /** if this property is not in list */
    
    var count = ids.length;
    var previous = $(this).hasClass("rental-previous");
    var new_index;
    if(previous) {
      new_index = index > 0 ? index - 1 : count-1; 
      console.log("previous: new: " + new_index);
    } else {
      new_index = index < count-1 ? index + 1 : 0;       
      console.log("next: new: " + new_index);
    }
    var new_id = ids[new_index];
    
    data.ajax = 1;
    data.obj_id = new_id;
    
    var options = {"success_handler": "traveler_ready"};
    console.log("Loading index=" + new_index + " id=" + new_id + " data=", data);
    return template_load("#rental_detail_container", '_rental-detail.html', 'rental_detail', options, data);     

    return false;
    //delete data.id_list;
    
    
  }
  return true;
});


/** geocomplete - add rental */
function geocomplete_init($geo) {    
  var lat = $geo.data('lat') || 0;
  var lng = $geo.data('lng') || 0;
  var zoom = $geo.data('zoom') || 10;
  var options = {
    details: ".geo-details",
    detailsAttribute: "data-geo",
    map: "#property_map",
    mapOptions: {
      zoom: zoom
    },
    markerOptions: {
      draggable: true
    }
  };
  if(lat && lng) options.location = [lat,lng];
  console.log("geocomplete_init: lat=" + lat + " lng=" + lng + " options=", options);
  
  // init Google PAC
  $geo.geocomplete(options).bind("geocode:result", function(event, result) {
    console.log("Geocode OK!", result);
    $("#next_button").removeClass("disabled");
    $("#formatted_address").html("<i class=\"fa fa-check\"> " + result.formatted_address + " <i class=\"fa fa-caret-down\"></i>");
    //$("#property_map").removeClass("visuallyhidden");
    //console.log(result);
  });

  $geo.bind("geocode:dragged", function(event, latLng) {
    $("input[name=location_lat]").val(latLng.lat());
    $("input[name=location_long]").val(latLng.lng());
    $("#address_form").collapse('show');

    $("#reset").show();
  });
  
  
  $("#reset").click(function(){
    $geo.geocomplete("resetMarker");
    $("#reset").hide();
    return false;
  });
  
  $("#find").click(function(){
    $geo.trigger("geocode");
  }).click();
}


function rental_rate_handler(params, data) {
  //console.log("rental_rate_handler v2 data=", data);  
  //console.log("params=", params);
  var cur = "&euro;";
  
  var $rr = $("#rr-table");
  $rr.removeClass("loading");
  //$(".discount-row").hide().addClass("collapse border");
  
  var keys = ['total', 'total_discounted', 'subtotal', 'average', 'average_discounted', 'fee_cleaning', 'service_fee', 'discount_week', 'discount_2_weeks', 'discount_3_weeks', 'discount_month', 'city_tax', 'tax', 'grand_total', 'nd', 'ng'];
  $("#rr-average").removeClass('overstrike');
  $("#rr-average_discounted").hide();
  $.each(keys, function(i, key) {
    var $fld = $("#rr-" + key);
    if(!$fld.length) $fld = $(".rr-" + key); // try class (used for num days)
    
    var val = data[key];
    var val_rounded, cur_val;
    // console.log("Field = " + key + " len=" + $fld.length + " val=" + val);
    if($fld.length && (val > 0 || val < 0)) {
      val_rounded = Math.round(val);
      cur_val = key == 'nd' ? val : (cur + val_rounded); 
      $fld.html(cur_val);
      
      if(key == 'average_discounted' && Math.round(data.average_discounted) < Math.round(data.average)) {
        console.log("discounted? data:",data)
        $("#rr-average").addClass('overstrike');
        $("#rr-average_discounted").show();
      }
      // console.log("Setting field = " + cur + val_rounded);

      $fld.closest('tr').show();
      
    } else {
      $fld.closest('tr').hide();
      //console.log("Hiding row of field = " + key + " fldlen=" + $fld.length + " row:",$fld.closest('tr'));
    }
  });
  
  $("#rr-table").show();
  $("#rr-check-availability").hide();
  if(data.book_direct) {
    $("#rr-book-now").show();
    $("#rr-request").hide();
  } else {
    $("#rr-book-now").hide();
    $("#rr-request").show();
  }
  $("#rr-inquiry").show();
  //$("#rr-total").html(cur + data.total);
  //$("#rr-fee_cleaning").html(cur + data.fee_cleaning);
  //$("#rr-grand_total").html(cur + data.grand_total);
  //$("#rr-grand_total").html(cur + data.grand_total);
}

/** from traveler custom.js: init rental-detail/rental-edit map */
function init_rental_map($map) {
  var lat = $map.data('lat') || 0;
  var lng = $map.data('lng') || 0;
  var types = $map.data('types') || '';
  
  console.log("init_rental_map: lat=" + lat + " lng=" + lng + " types=" + types);
  if(!(lat && lng)) return false; 
  var latlng = new google.maps.LatLng(lat, lng);
  var myOptions = {
    zoom: 14,
    center: latlng,
    mapTypeId: google.maps.MapTypeId.ROADMAP,
    scrollwheel: false
  };

  var map_id = $map.attr('id');
  if(!map_id) {
    console.log("init_rental_map: please give the map an ID");
    return;
  }
  var map = new google.maps.Map(document.getElementById(map_id), myOptions);
  console.log('map1', bounds);


  var marker = new google.maps.Marker({
      position: latlng,
      map: map
  });
    
  marker.setMap(map);
  var $map = $("#" + map_id);
  $map.data('map', map);

  $('a[href="#rental-map"]').on('shown.bs.tab', function(e) {
      console.log("resize map1.1 latlng=" + lat + "/" + lng + " on map:", map);
      google.maps.event.trigger(map, 'resize');
      map.setCenter(latlng);
  });
  
  bounds_timer = 0;
  if(map && types) { // load markers for golf course, airport, etc
    
    google.maps.event.addListener(map, 'bounds_changed', function() {
      if(bounds_timer) {
        console.log("bounds timer is set - do nothing");
        bounds_timer = 0;
      } else {
        bounds_timer = setTimeout(function() {
          var bounds = map_get_bounds(map);
          bounds = bounds_extend(bounds, 5000);
          console.log("adding " +types + " location for bounds", bounds);
          map_add_locations(types, JSON.stringify(bounds), map_id);
        }, 800);
        console.log("bounds timer = " + bounds_timer);
      }
    });   
    
  }
  return map;
}

function rental_init_calendar($fullcalendar) {
  console.log("found fullcalendar");
  var init = $fullcalendar.data('init');
  if(!init) fullcalendar_init($fullcalendar);
    
  $fullcalendar.fullCalendar('render');
  var apt = $fullcalendar.data('rental') || {}; // {"id": 10, "name": "fooapt", "type": "shortstay"};
  var checkin = apt.in || '';
  var checkout = apt.out || '';
  var min_date = $fullcalendar.data("min_date");
  var max_date = $fullcalendar.data("max_date");

  if(apt.id && checkin && checkout) {
    console.log("fullcal init: in=" + checkin + " out=" + checkout);
    var momentin = moment(checkin, "YYYY-MM-DD");
    var momentout = moment(checkout, "YYYY-MM-DD");
    var selmoment = null;
    var seldate = '';
    // first select all months in order to calculate price for these dates
    var inyear = momentin.year();
    var inmonth = momentin.month();
    var outyear = momentout.year();
    var outmonth = momentout.month();
    console.log("in = " + inmonth + '/' + inyear);
    console.log("out = " + outmonth + '/' + outyear);

    // check if we already looped 
    if(min_date && max_date && checkin >= min_date && checkout <= max_date) {
      console.log("Already initialized");
      $fullcalendar.fullCalendar('select', momentin, momentout); // select dates
      
    } else {
      // if not, show spinner and loop
      // loop through and select all dates
      while(outyear > inyear || (outyear == inyear && outmonth >= inmonth)) {
        selmoment = moment({"day": 0, "month": outmonth, "year": outyear});
        seldate = selmoment.format("YYYY-MM-DD");
        console.log("selecting " + seldate);
        outmonth--;
        if(outmonth < 0) {
          outmonth = 11;
          outyear--;
        }
        $fullcalendar.fullCalendar('gotoDate', selmoment); // select dates
      }
      
    }
    
    var dates = $fullcalendar.data('dates');
    console.log('dates=', dates);
    $fullcalendar.fullCalendar('select', momentin, momentout); // select dates
  } 
}


/** marker helpers */
function pinSymbol(color) {  
  return svgSymbol('pin', color);
}

function bubbleSymbol(color) {  
  return svgSymbol('bubble', color);
}

function svgSymbol(style, color) {  
  return {
    path: svgPath(style),
    fillColor: color,
    fillOpacity: 1,
    strokeColor: '#000',
    strokeWeight: 2,
    scale: 0.5
  };
}

function svgPath(style) {
  switch (style) {
    case 'bubble': return'M88.8,0.1H20c-6.2,0-11.2,5-11.2,11.3v8.1c0,0-5.1,3.7-8.5,2.5c0,0,2.8,5.3,8.6,3.3c0.7,5.5,5.4,9.8,11.1,9.8h68.8c6.2,0,11.2-5,11.2-11.3V11.4C100,5.2,95,0.1,88.8,0.1z';
    case 'pin': return 'M 0,0 C -2,-20 -10,-22 -10,-30 A 10,10 0 1,1 10,-30 C 10,-22 2,-20 0,0 z';
    default: return 'M 0,0 C -2,-20 -10,-22 -10,-30 A 10,10 0 1,1 10,-30 C 10,-22 2,-20 0,0 z';
  }
}

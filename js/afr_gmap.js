// start googlemap.js

var gmap_classic = 1;
gmap_classic = 0;

// new gmap function using API v3 + the jquery gmap plugin from http://code.google.com/p/jquery-ui-map/
var gmap; // global handle to the map;
gmap_icon = '/images/markers/pushpin_red.png';
gmap_icon_hi = '/images/markers/pushpin_yellow.png'; 
var map_timer;
var g_map_timeout = 100;
var g_just_loaded;

// returns corresponding gmarker for given marker_id in my set of markers
function gmap3_gmarker2(map_id, marker_id) {
  var $map = $('#' + map_id);
  var gmarkers = $map.data('gmarkers'); 
  var mymarkers = $map.data('markers'); 
  if(gmarkers) {
    $.each(gmarkers, function(i, gmarker) {
      var m = mymarkers[i];
      if(m.id == marker_id) return gmarker;
    });
  }
}

function gmap3_update_list(map_id, prefix) {
  var $map = $('#' + map_id);
  var gmarkers = $map.data('gmarkers'); 
  var mymarkers = $map.data('markers');
  if(!gmarkers) {
    return;
  }
  var count = Object.size(gmarkers);
  var show=0;
  var hide=0;
  if(gmarkers) gmap3_show_hide(gmarkers, map_id, prefix);
}

// helper function for above
function gmap3_show_hide(markers, map_id, prefix) {
  var $map = $('#' + map_id);
  var mymarkers = $map.data('markers'); 
  var count = markers.length;
  var show=[];
  var hide=[];
  var m_id_list;
  
  console.log("gmap3_show_hide v2");
  $.each(markers, function(i, gmarker) {
    var isInViewport = $map.gmap('inViewport', gmarker);
    var m = mymarkers[i] || [];
    var id = m.id;
    m_id_list = gmarker.id_list || [];
    //if(id_list) alert("id_list=" + id); // deal with array: multiple properties same location 
    if(isInViewport) {
      //log('count=showing #' + prefix + m.id);
      if(m_id_list.length) {  // multipls
        $.each(m_id_list, function(i, lid) {
          $('#' + prefix + lid).show();
          show.push(lid);
        });
      } else { // single
        $('#' + prefix + m.id).show();
        show.push(id);
      }
    } else {
      if(m_id_list.length) {  // multipls
        $.each(m_id_list, function(i, lid) {
          $('#' + prefix + lid).hide();// .css('border', '5px solid black');
          hide.push(lid);
        });          
      } else { // single
        $('#' + prefix + m.id).hide();// .css('border', '5px solid black');
        hide.push(id);
      }
      //log('hiding #' + prefix + m.id);
    }        
  });    
  //log('mymarker count=' + mymarkers.length + ' gmarker count='+count+ ' showing=' + show.length + ' hiding=' + hide.length);
  //log('count='+count+ ' showing=' + show.join(':') + ' hiding=' + hide.join(':'));
}

// prints content for gmap infoWindow (text bubble)
function gmap3_info_window_content(marker, map_options) {
  
  var name = '';
  var content = '';
  var link = '';
  
  if(is_array(marker)) { // several properties at this location
    var m = marker[0];
    
    for(var i in marker) {
    	name += gmap3_info_window_title(marker[i], map_options);
    }
  } else { // just one property at this locatoin  
    var m = marker;
    switch(m.type) {
      case 'shortstay':
        link = m.site_id==263 ? "/golf-homes/" : "/serviced-apartments/";
        break;
      case 'golf_club':
        link = "/golf-club/";
        break;
      default:
        break;
    }
    m.link = link ? link + m.id : '';
    
    //console.log("type=" + m.  type + " link=" + m.link + " site_id=" + m.site_id);
    
    name = gmap3_info_window_title(marker, map_options);
  }
  
  if (m.images) { // array of image urls, show slider
    json_data = m.images;
    var images = jQuery.parseJSON(json_data);
    var first = images[0] || {};
    //var json_data = JSON.stringify(images);

    
    content = content  
    +'  <div class="listing-img media-photo" unselectable="on" style="-webkit-user-select: none;">'
    +'    <div class="listing-img-container media-photo image-rotation" data-current="0" '
    +'         data-images=\''+ json_data + '\'>'
    +'      <img src="' + first.filename + '" class="rental-list-photo" data-url="' + m.link + '">'
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

  } else if(m.image) {
    if(m.link) content = content + '<a href="' + m.link + '" class="property-detail-link"><img width="100%" src="' + m.image + '"></a>';
    else content = content + '<img width="100%" src="' + m.image + '">';
    //console.log("image=" + m.image);
  }
  
  if(name) content += '<h3>' + name + '</h3>';

  //content += '<input type="number" readonly class="rating" value="8.5" data-min="0" data-max="10" data-size="xxs" data-show-clear="false" data-show-caption="false">';
  
  //var content = '<b>' + m.name + '</b><br>';

  // use same address for all
  //if(m.address) content += m.address + "<br>"; 
  
  if(m.price) content += m.address_city + "<span class='pull-right'>" + currency2symbol(m.currency) + m.price + "</span><br>"; 

  if(m.extra) content += m.extra;
  
  
  if(m.description) content += '<p>' + m.description + "</p>";
  
  return '<div class="gmap_info_content air-item">' + content + '</div>';

}
// var mo = $("#" + ' + map_options.id + '").data("options"); 

// returns the title of the property (often link) used in infoWindow
function gmap3_info_window_title(m, map_options) {
  var title = '';
  var name = m.name;
  // alert(dump(map_options));
  if(map_options.select_target) { 
    title += '<a href="#" class="property-detail-link" onclick="var st = $(\'#' + map_options.id + '\').data(\'select_target\'); $(\'#\' + st).val(' + m.id + ');close_parent_dialog(\''+ map_options.id +'\');return false;">' + name + '</a>';
  } else if(m.link) {
    title += '<a href="'+ m.link +'" class="property-detail-link">' + name + '</a>';
  } else {
    title += name;
  }
  return title;
}

function gmap3_removemarkers(id) {
  var $map = $('#' + id);
  var gmarkers = $map.data('gmarkers');
  var gmarkersAr = $map.data('gmarkersAr');

  /** doesn't work for some reason
  console.log("\n\n------   Removing v2 all markers for " + id + " len=" + Object.size(gmarkers) + "----\n\n");  
  for (var key in gmarkers) {
    var m = gmarkers[key];
    console.log("Removing marker " + key + "=",m);
    m.setMap(null);
    //(gmarkers[key]).setMap(null);
  }
  */
  
  console.log("\n\n------   Removing v34 all markers for " + id + " len=" + gmarkersAr.length + "----\n\n");  
  for(var i = 0; i < gmarkersAr.length; i++) {
    gmarkersAr[i].setMap(null);
  }
  
  $map.data('markers', []);
  $map.data('gmarkers', {});    
  $map.data('gmarkersAr', []);    
  $map.data('marker_ids', []);
  //console.log("Remove All Markers");
}

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

var gmarkers = {};

function gmap3_loadmarkers(id, markers, set_bounds) {
  console.log("gmap3_loadmarkers len=" + markers.length + " markers",markers);
  //console.log("markers", markers);
  var $map = $('#' + id);
  
  // var map = new google.maps.Map(document.getElementById(id));
  var map = gmap3_get(id);
    
  var mymarkers = [];
  var gmarkersAr = []; /** not sure why gmarkers object does not work for this */
  var marker_ids = $map.data('marker_ids') || [];
  var map_options = $map.data('options') || {};
  
  if(map_options.center && map_options.accuracy && markers.length == 1) set_bounds = false; // do not auto set bounds
  var hilite_prefix = map_options.hilite;
  var update_list = map_options.update_list;
  var m0; // the first marker
  var m_id_list = [];
  var gmid = 0;
  var existing_markers = $map.data('markers', mymarkers) || [];
  $.each(markers, function(i, m) {   
    var midAr = [];
    var marker_id = i;
    //console.log("iterate i=",i);
    //console.log("iterate m=",m);
    if($.inArray(i, marker_ids) >= 0) {
      return true; // = continue
    }
    var label = '';
    
    marker_ids.push(i); // list of loaded markers
    
    if(is_array(m)) { // special case: multiple properties at same location
      $.each(m, function(i, mi) {
        midAr.push(mi.id);
      });
      midAr.sort();
      m0 = m[0];
      //allmarkers = allmarkers.concat(m);
      name = m0.name + " + " + (m.length - 1);
      m_id_list = midAr;
      var obj_type = m0.obj_type || 'marker';
      gmid = obj_type + '_' + m_id_list; //midAr.join('-'); // use first
    } else {
      m0 = m;
      name = m0.name;
      label = m0.label;
      
      m_id_list = [];
      var obj_type = m0.type || 'marker';
      gmid = obj_type + '_' + m0.id;
      
    }
    
    switch(m0.type) {
      case 'shortstay':
        var price = m0.price = m0.rate_day;
        label = currency2symbol(m0.currency) + price;
        if(m0.book_direct) {
          m0.extra = '<i class="fa fa-bolt red"></i> Book Direct';
          label = label + '&#9889;';
        }
        break;
      case 'golf_club':
        break;
      default:
        break;
      
    }

    m0.obj_type = m0.type;
    if(m.media_json) {      
      m0.images = m.media_json;
    } else {
    }
    
    
    // copy back to m
    if(is_array(m)) {
      m[0] = m0;
    } else {
      m = m0;
    }
    
    //console.log("gmid=" + gmid);
    var icon = m0.marker ? m0.marker : gmap_icon;
    var icon_hi = m0.marker_hi ? m0.marker_hi : gmap_icon_hi;
    var markerOpts = {};    
      
    if(map_options.radius) {
      var circle_options = {};
      circle_options.center = markerOpts.position;
      circle_options.radius = map_options.radius;
      gmap3_circle(id, circle_options); // don't draw marker, but a circle with approximate location
    } else if(label) { // marker with label
      var colors = ['#a50000', '#0040a8', '#2d2d2d'];
      markerOpts = {
        //'id': 'marker_' + marker_id, 
        id: 'marker_' + gmid, 
        ref: gmid, 
        title: name,
        type: "label",
        map: map,
        position: new google.maps.LatLng(m0.location_lat, m0.location_long), 
        bounds: set_bounds,
        
        raiseOnDrag: true,
        labelContent: label,
        labelClass: "white bluebg", // the CSS class for the label
        labelInBackground: false,
        icon: svgSymbol('bubble', 'blue'),
        labelAnchor: new google.maps.Point(-10, -2),
      }; 

      var gmarker = new MarkerWithLabel(markerOpts);
      gmarker.m = m;
      
      //gmarkers.push(gmarker);
      //console.log("Added marker: " + gmid);
      var content = gmap3_info_window_content(m, map_options);
      var iw = new google.maps.InfoWindow({
        content: content
      });
      gmarker.infowindow = iw;

      
      google.maps.event.addListener(gmarker, "click", function(e) {
        iw = $map.gmap('openInfoWindow', {'content': content, 'maxWidth': 220}, this);
        //console.log('openInfoWindow returned:', iw);
        //iw.open(map, gmarker);
      });
      gmarkers[gmid] = gmarker;
      gmarkersAr.push(gmarker);
      
      
    } else {
      markerOpts = {
        //'id': 'marker_' + marker_id, 
        id: 'marker_' + gmid, 
        ref: gmid, 
        title: name,
        type: "icon",
        map: map,
        position: new google.maps.LatLng(m0.location_lat, m0.location_long), 
        icon: (m0.marker ? m0.marker : gmap_icon), 
        bounds: set_bounds
      }; 
      
      $map.gmap('addMarker', markerOpts, function(map, gmarker) {
        gmarker.id = gmid;
        gmarker.obj_type = obj_type;
        gmarker.id_list = m_id_list;
        gmarker.icon_lo = icon;
        gmarker.icon_hi = icon_hi;
        mymarkers.push(m0);
        $(gmarker).mouseover(function() {
           stopScroll();
           if(hilite_prefix) gmap3_hilite_item(hilite_prefix, m, true);
           gmarker.setIcon(icon_hi);
        }).mouseout(function() {
          stopScroll();
          if(hilite_prefix) gmap3_hilite_item(hilite_prefix, m, false);
          gmarker.setIcon(icon);
        });
        var content = gmap3_info_window_content(m, map_options);
        gmarker.m = m;
        gmarkers[gmid] = gmarker;
        gmarkersAr.push(gmarker);
        //gmarkers.push(gmarker);
        
        //var infowindow = new google.maps.InfoWindow({
        //    content: content,
        //    maxWidth: 200
        //});
        //google.maps.event.addListener(gmarker, 'click', function() {
        //  infowindow.open(map,gmarker);
        //});        
        $(gmarker).click(function() {
          $map.gmap('openInfoWindow', {'content': content, 'maxWidth': 220}, this);
        });
        if(map_options.zoom) {
          //alert(map_options.zoom);
          //map.setZoom(map_options.zoom);
        }
      });
    }
  });
  
  //var bar1 = $map.data('markers'); 
  //alert(' bar1 count =' + bar1.length + ' marker lenght=' + markers.length );
  //allmarkers = allmarkers.concat(mymarkers);
  //markers = mymarkers;
  
  var $map = $('#' + id);
  $map.data('markers', mymarkers);
  $map.data('gmarkers', gmarkers);
  $map.data('gmarkersAr', gmarkersAr);
   
  console.log("Saving " + Object.size(gmarkers) + " markers");
  $map.data('marker_ids', marker_ids);
  return gmarkers;
  //$map.data('allmarkers', allmarkers);
}

function gmap3_hilite_item(hilite_prefix, marker, hilite, i) {
  if(!i) i = 0;
  if(is_array(marker)) {
    for(var i in marker) gmap3_hilite_item(hilite_prefix, marker[i], hilite, i);
  } else {    
    var $property = $('#' + hilite_prefix + marker.id);
    if(hilite && $property.length) {
      if(i==0) {
        var scrollTo = $('#property_right').scrollTop() + $property.position().top;
        $('#property_right').delay(400).animate({scrollTop: scrollTo}, 1000); // only scroll to first in list
        
      }
      $property.addClass('selected');
    } else {
      $property.removeClass('selected');
    }
  }
}
  
// loop through markers, return marker with given id, or where id is in list
function gmap3_find_marker(markers, id) {
  for(var i in markers) {
    var marker = markers[i];
    if(is_array(marker)) {
      var id_list = i;
      // alert('list=' + id_list);
      //alert("array = " + marker.id);
      //alert(dump(marker));
      for(var j in marker) {
        var m2 = marker[j];
        m2.id_list = i;
        if(m2.id == id) return m2;
        // //console.log(dump(m2));
        // gmap3_find_marker(m2, id));
        // alert("id=" + id + " IN array = " + m2.id);
        //if(var m = gmap3_find_marker(m2, id)) {
        //  return m;
        //}
      }
    } else {
      // alert(dump(marker));
      if(marker.id == id) return marker;
      // alert("id=" + id + " NOT array = " + marker.id);
      // //console.log(marker.name + ' is not array, id = ' + marker.id + dump(marker));
      
      //var idAr = marker_id.indexOf(',') ? marker_id.split(',') : [];
      //if(marker_id == id || (idAr.length && in_array(id, idAr))) return marker;
    }
  }
  return null;
}


// takes json bounds, returns google bounds
function gmap3_bounds(bounds) {
  //console.log("ne=" + bounds.northeast.lat + "," + bounds.northeast.lng);
  //console.log("sw=" + bounds.southwest.lat + "," + bounds.southwest.lng);

  var myNortheast = new google.maps.LatLng(bounds.northeast.lat, bounds.northeast.lng);
  var mySouthwest = new google.maps.LatLng(bounds.southwest.lat, bounds.southwest.lng);
  var myBounds = new google.maps.LatLngBounds(mySouthwest, myNortheast);
  return myBounds;  
}

function gmap3_get(map_id) {
  return $('#' + map_id).gmap('get', 'map');
}

function gmap3_set_bounds(map_id, bounds) {  
  var map = gmap3_get(map_id);
  var gBounds = gmap3_bounds(bounds); 
  map.fitBounds(gBounds);
}


  
/*
ajax_load: 1
bounds: "{"southwest":{"lat":"37.0966","lng":"-8.4968"},"northeast":{"lat":"37.1146","lng":"-8.4788"}}"
center: nf
class: "overview-map"
hilite: "-"
id: "search_result_map"
loc_types: "golf_course,airport"
site_id: "263"
type: "satellite"
update_list: "property-list"
visible: true
zoom: 12
search: Object
  bounds: "[["37.0748","-8.5291"],["37.1323","-8.4412"]]"
  center: "["37.1036","-8.4851"]"
  loc: "Pestana Alto Golf Resort Golf Course Portimao Algarve"
  loc_types: "golf_course,airport"
  ng: "1"
  search: "1"
  site_id: "263"
  t: "shortstay"
*/

function gmap_center(center) {
  if(is_object(center)) return center;
  if(!center) center = [0,0];
  var center = center || '[0,0]';
  var centerAr = center.split(",");
  var centerLatLng = new google.maps.LatLng(centerAr[0], centerAr[1]);
  fixed_options.center = centerLatLng;
  return fixed_options;
}
  
function gmap_markerOpts(map, marker) {
  return {
    title: marker.name,
    type: "label",
    map: map,
    position: new google.maps.LatLng(marker.location_lat, marker.location_long), 
    //bounds: set_bounds,    
    raiseOnDrag: true,
    labelContent: marker.label,
    labelClass: "white bluebg", // the CSS class for the label
    labelInBackground: false,
    icon: svgSymbol('bubble', 'blue'),
    labelAnchor: new google.maps.Point(-10, -2),
  };
} 

/** new: plain gmap */
function gmap_loadmarkers(map, markers) {
  $.each(markers, function(i, m) {
    var type = m.type;
    var label = m.label;
    var loc = {lat: marker.location_lat, lng: marker.location_long};
    console.log("Marker:",m);
    var marker = m;
    var markerOpts = gmap_markerOpts(map, marker);
    var gmarker = new MarkerWithLabel(markerOpts);
  });
  
}

/** todo: change to jQuery plugin */
function gmap3(options) {
  g_just_loaded = 1;
  
  if(typeof options == "undefined") var options = {};
  //console.log("gmap3: options=", options);
  var id = options.id = options.id || "search_result_map";
  var $map = $('#' + id);
  var cont_options = $map.data('options') || {};
  options = $.extend(options,cont_options);
  
  options.update_list = "property-list";
  options.hilite = "-";

  var selected_id = options.selected_id;
  var hilite_prefix = options.hilite;
  var update_list = options.update_list;
  var bounds_str = '';     
  var center_str = '';
  var centerAr = options.center || [];   
  var boundsAr = options.bounds || []; 
  var zoom = 0;
  var markers = options.markers || [];
  var type = options.type = options.type || "roadmap";

  //console.log("options center array=", centerAr);
  //console.log("options bounds array=", boundsAr);

  /**
  var form_id = options.form_id = options.form_id || "";
  if(form_id) {
    var $form = $("#" + form_id);
    if(!$form.length) { 
      alert("gmap3: could not find form " + form_id);
      return false;
    }
    bounds_str = $("#" + form_id + " .location-bounds").val();
    if(bounds_str && !centerAr) {
      boundsAr = $.parseJSON(bounds_str);
    }
    center_str = $("#" + form_id + " .location-center").val();
    if(center_str && !boundsAr) {
      centerAr = $.parseJSON(center_str);
    }
  }
  */
  
  var visible = options.visible = $map.is(":visible"); 
    
  if(boundsAr.length) { // read data from form, parse it, and store it in options
    var boundsObj = array2bounds(boundsAr);
    var bounds = options.bounds = boundsObj;
    var gBounds = gmap3_bounds(bounds);       
  } else {
    alert("no bounds");
    return;
    zoom = options.zoom || acc2zoom(options.accuracy) || 12;
  }
  
  centerAr = bounds2center(boundsAr);
  //console.log("computed center:", centerAr);
  if(centerAr.length) {
    var gCenter = options.center = new google.maps.LatLng(centerAr[0], centerAr[1]);
  } else {
    alert("no map center");
  }
  
  /**
  console.log("center string=" + center_str);
  console.log("center array=", centerAr);
  //console.log("center obj=", gCenter);
  
  //console.log("Found form " + form_id + " bounds=" + bounds_str + " center=" + center_str);  
  console.log("bounds string=" + bounds_str);
  console.log("bounds array=", boundsAr);
  console.log("bounds object=", boundsObj);
  */
  
  var markers_loaded = false;
  var search_markers = [];
  if(gCenter) $map.data('center', gCenter);
  if(gBounds) $map.data('bounds', gBounds);
  if(boundsObj) $map.data('boundsObject', boundsObj);
  $map.data('zoom', zoom);  
  $map.data('options', options);
  if(options.select_target) $map.data('select_target', options.select_target); // shortcut

  console.log("map options v2=", options);
  //$map.gmap({'center': gCenter, 'bounds': gBounds});

  if(gmap_classic) {
    var radius = boundsAr ? bounds2radius(boundsAr) : 2000;
    var zoom = radius2zoom(radius);  
    //console.log("Calling old gmap with radius=" + radius + " bounds: ", boundsAr);
    console.log("Old map: radius=" + radius + " zoom=" + zoom + " type=" + type); 
    var type = options.type == 'satellite' ? 'hybrid' : 'roadmap';
    var map = new google.maps.Map(document.getElementById(id), {
      center: {lat: centerAr[0], lng: centerAr[1]},
      zoom: zoom,
      mapTypeId: type
    });
    
    map.addListener('dragend', function() {
      console.log("center changed");
      map_timer = setTimeout(function() {
        console.log("update map - drag");
        //gmap3_update_map(id, map);
        markers_loaded = true;
      }, g_map_timeout);
    });
    
    map.addListener('zoom_changed', function() {
      console.log("zoom changed");
      map_timer = setTimeout(function() {
        console.log("update map - zoom");
        //gmap3_update_map(id, map);
        markers_loaded = true;
      }, g_map_timeout);
    });
    
    
    
    return map;
    //gmap_loadmarkers(map, markers);
    
    //gmap3_loadmarkers(id, markers, true); 
    
  } else {
    $map.gmap({'center': gCenter, 'zoom': zoom, 'bounds': gBounds}).bind('init', function(evt, map) {
      if(!visible) { // mobile, not showing map      
        gmap3_update_map(id, map);
        console.log("\n\n\MAP NOT VISIBLE\n\n\n");
      } else { // update list of locations to match those in view
        if(options.type == 'satellite') {
          console.log("\n\n\nSetting map to satellite\n\n\n");
          map.setMapTypeId(google.maps.MapTypeId.HYBRID);
        } else {
          console.log("\n\n\nMap type=" + options.type + " \n\n\n options=",options);
        }
        
        google.maps.event.addListener(map, 'dragend', function() {
  
          if(g_just_loaded) {
            console.log("dragend: map was just loaded - do nothing");
            gmap_check_bounds(map, boundsAr);
            g_just_loaded = 0;
          } else if(map_timer) {
            console.log("dragend: map timer is set - do nothing");
          } else {
            map_timer = setTimeout(function() {
              gmap3_update_map(id, map);
              markers_loaded = true;
            }, g_map_timeout);
            console.log("dragend: set map timer = " + g_map_timeout);
          }
        });
        google.maps.event.addListener(map, 'zoom_changed', function() {
          if(g_just_loaded) {
            console.log("zoom_changed: map was just loaded - do nothing");
            gmap_check_bounds(map, boundsAr);
            g_just_loaded = 0;
          } else if(map_timer) {
            // console.log("map timer is set - do nothing");
          } else {
            map_timer = setTimeout(function() {
              gmap3_update_map(id, map);
              markers_loaded = true;
            }, g_map_timeout);
            console.log("zoom_changed: set map timer = " + g_map_timeout + " just_loaded=" + g_just_loaded);
          }
        });
      }                        
      if(!markers_loaded) gmap3_loadmarkers(id, markers, true); 
      if(selected_id) gmap3_marker_click(id, selected_id); 
      
      if(!markers.length || markers.length == 1) {
        if(zoom) map.setZoom(zoom);
        else if(gBounds) map.fitBounds(gBounds);
      }
      gmap = map; // store globally
  
    });
  
  }
  
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
  if(radius < 1000) return 17; // max zoom
  //if(radius > 1000) return 17;

  var miles = radius/1600;
  return Math.round(14-Math.log(miles)/Math.LN2);
}

/** expects [[],[]] */
function bounds2radius(bounds) {  
  var sw = bounds[0];
  var ne = bounds[1];
  var d_lat = Math.abs(ne[0] - sw[0]);
  var d_lng = Math.abs(ne[1] - sw[1]);
  var diff = (d_lat + d_lng) / 2; // cheesy average instead of pythagoras since x and y are usually the same
  var r = degrees2meters(diff);
  return Math.round(r / 2);
}

function gmap_check_bounds(map, boundsAr) {
  var gbounds = map.getBounds();
  var cbounds = gmap3_bounds2json(gbounds);
  var radius = boundsAr ? bounds2radius(boundsAr) : 0;
  var cradius = cbounds ? bounds2radius(cbounds) : 0;
  var id = "search_result_map";
  
  if(cradius > radius * 1.1) { // actual more than 10% bigger
    console.log("Actual radius = " + cradius + " more than 10% bigger than " + radius);
    return ;
    map_timer = setTimeout(function() {
      gmap3_update_map(id, map);
    }, g_map_timeout);
  }

  return;
}

// fit bounds to markers
function gmap3_fitbounds(map_id, gmarkers) {
  var map = gmap3_get(map_id);
  var bounds = new google.maps.LatLngBounds();
  for (var key in gmarkers) {
    var m = gmarkers[key];
    bounds.extend(m.getPosition());
  } 
  map.fitBounds(bounds);
}

// converts Google Bounds to simple array
function gmap3_bounds2json(boundsObj) {
  var ne = boundsObj.getNorthEast();
  var sw = boundsObj.getSouthWest();

  //console.log("map is visible bounds ne=", ne);
  var g_loc_accuracy = 4; // global out of scope?

  var lat0 =  sw ? parseFloat(sw.lat()).toFixed(g_loc_accuracy) : 0;
  var lng0 =  sw ? parseFloat(sw.lng()).toFixed(g_loc_accuracy) : 0;
  var lat1 = ne ? parseFloat(ne .lat()).toFixed(g_loc_accuracy) : 0;
  var lng1 = ne ? parseFloat(ne .lng()).toFixed(g_loc_accuracy) : 0;
  return [[lat0,lng0],[lat1,lng1]];
}
  
// just for debugging
function gmap3_bounds2jsonstr(boundsObj) {
  var ne = boundsObj.getNorthEast();
  var sw = boundsObj.getSouthWest();
  var lat0 =  sw ? parseFloat(sw.lat()).toFixed(g_loc_accuracy) : 0;
  var lng0 =  sw ? parseFloat(sw.lng()).toFixed(g_loc_accuracy) : 0;
  var lat1 = ne ? parseFloat(ne .lat()).toFixed(g_loc_accuracy) : 0;
  var lng1 = ne ? parseFloat(ne .lng()).toFixed(g_loc_accuracy) : 0;
  return '{"northeast":{"lat":' + lat0 + ',"lng":' + lng0 + '},"southwest":{"lat":' + lat1 + ',"lng":' + lng1 + '}}  ';
}
  

function gmap3_update_map(id, map) {
  
  map_timer = 0;
  var $map = $('#' + id);
  var options = $map.data("options");
  var hilite_prefix = options.hilite;
  var visible = options.visible;
  
  //console.log("gmap3_update_map options=", options);

  if(options.type == 'satellite') {
    //console.log("Setting type to satellite");
    //map.setMapTypeId(google.maps.MapTypeId.SATELLITE);
  }
  
  //$('#property-list').addClass('transparent disabled disabled_bg loading-big');
  gmap3_update_list(id,  hilite_prefix);

  if(1) {
    //search = $map.data('search') || options.search || {};
    if(visible) {
      var currentBounds = map.getBounds();
      var currentZoom = map.getZoom();
      $map.data('bounds', currentBounds);

      $map.data('zoom', currentZoom); // gmap('option', 'zoom') doesn't work for some reason, so store with map div
      
      //search.bounds = encodeURIComponent(currentBounds);
      
      var ne = currentBounds.getNorthEast();
      var sw = currentBounds.getSouthWest();

      //console.log("map is visible bounds ne=", ne);
      var g_loc_accuracy = 4; // global out of scope?

      //var lat0 =  sw ? parseFloat(sw.lat()).toFixed(g_loc_accuracy) : 0;
      //var lng0 =  sw ? parseFloat(sw.lng()).toFixed(g_loc_accuracy) : 0;
      //var lat1 = ne ? parseFloat(ne .lat()).toFixed(g_loc_accuracy) : 0;
      //var lng1 = ne ? parseFloat(ne .lng()).toFixed(g_loc_accuracy) : 0;

      var bounds = gmap3_bounds2json(currentBounds);
      var bounds_str = JSON.stringify(bounds);            


      //search.bounds = bounds_str;

      var center = currentBounds.getCenter();
      var lat =  center ? parseFloat(center.lat()).toFixed(g_loc_accuracy) : 0;
      var lng =  center ? parseFloat(center.lng()).toFixed(g_loc_accuracy) : 0;
      var loc = [lat,lng];      
      var loc_str = JSON.stringify(loc);            
      //search.center = loc_str;

      var dbBounds = gmap3_bounds2jsonstr(currentBounds);
  
      console.log("center:", loc_str);
      console.log("zoom:", currentZoom);
      console.log("bounds:", bounds_str);
      console.log("db bounds:", dbBounds);
      
      $(".location-center").val(loc_str);
      $(".location-bounds").val(bounds_str);

      
      //console.log("map is visible bounds=", currentBounds);
      //console.log("map is visible loc str=", loc_str);
      //console.log("map is visible bounds str=", bounds_str);
      //console.log("map is visible zoom str=", currentZoom);
      
    } else {
      var currentBounds = $map.data('bounds');
      //console.log("Map not visible bounds=");
      //console.log(currentBounds);
    }
    
    var form_id = options.form_id || "";
    if(form_id) {
      var form_data = $("#" + form_id).serialize();
      
      var query_data = $.getQueryParameters() || {};
      query_data = array_filter(query_data);

      //console.log("form data=", form_data);
      var options = $map.data('options') || {};
  
      var ajax_url = "/ajax.php?oper=property-search&" + form_data;
      console.log("gmap search url=" + ajax_url);
      if(query_data) {
        var qvars = ['p', 'pp', 'cc'];
        for(var i in qvars) {
          var fld = qvars[i];
          if(query_data[fld]) {
            ajax_url + "&" + fld + "=" + query_data[fld];
          } else {
            //console.log(fld + " not in data: val=" + query_data[fld]);
          }
        }
        //ajax_url = ajax_url + "&" + obj2qs(query_data);
        //console.log("additional GET data=", query_data, " qs: " + ajax_url);
      }
        
      //console.log("Dynamically loading properites from " + ajax_url);
      $.getJSON(ajax_url, function(response) {
        //$('#property-list').removeClass('transparent disabled disabled_bg loading-big');
        if(response.success) {
          var data = response.data;
          gmap3_update_search(id, data);
          // property_search_stats(data);
        } else {
          //console.log("Search failure");
          //console.log(response);
        }
      });
    }
  }
}

function get_markers(apts) {
  var apt = apts[0];
  var markers = [];
  console.log("get_markers:",apt);
 
  return markers;
}

function gmap3_get_locations(id, loc_types, bounds_str) {
  var ajax_url = "/ajax.php?oper=find-locations&loc_types=" + loc_types + "&bounds=" + encodeURIComponent(bounds_str);
  console.log("find-locations url:", ajax_url);
  $.getJSON(ajax_url, function(response) {
    console.log("find-locations returned:", response);
    if(response.success) {
      var locs = response.locs;      
      console.log("Adding markers for locations:" + locs.length);
      gmap3_loadmarkers(id, locs, false);
    } else {
      console.log("Response failure:", response);
    }
  });
}

// called with response from property-search
// call property-print
function gmap3_update_search(id, data) {
  var $map = $('#' + id);
  console.log("gmap3_update_search v2: id=" + id + " data=", data);
  
  if(!data) return;

  var apts = data.rentals || [];
  var count = apts.length;
  
  console.log("Ok, returned with " + count + " apts total count=" + data.total_count);
  var options = {};
  
  template_load("#property-list", "_rental-list.html", "rental_search", options, data)

  gmap3_removemarkers(id); 
  var gmarkers = gmap3_loadmarkers(id, apts, false);
  var bounds = data.bounds;
  var bounds_str = JSON.stringify(bounds);              
  var loc_types = $('#loc_types').length ? $('#loc_types').html() : '';
  console.log("loc types=" + loc_types + " len=" + $('#loc_types').length + " bounds_str=" + bounds_str);
  if(loc_types && bounds_str) { /** now load other markers; airport, landmark, golf_course */
    gmap3_get_locations(id, loc_types, bounds_str);  
  }  
    
  /** old
        
    search_markers = data.locs;
    
    $('html, body').animate({scrollTop:0}, 'slow');
    gmap3_removemarkers(id); 
    
    //update_search_title(data); // update title
    //update_property_filters(data); // update min/max price, max guests, facilities in filters
    
    var lang = data.language;
    var id_list = data.id_list;
    
    $map.data('apt_list', id_list);
  
    var form_id = "rental_search_form";    
    var map_options = $map.data('options') || {};
    
    var options = {};
    if(map_options.success_handler) options.success_handler = map_options.success_handler;
      
    var data = form_data(form_id);
    
    data.count = data.count;
    data.total_count = data.total_count;
    data.id_list = id_list;
    data.language = lang;
    
    console.log("printing properties v2 count=", count);
    options.unauth = 1; // allow users who are not logged in
    
    template_load("#property-list", "_rental-list.html", "rental_search", options, data)
    
    // var ajax_url = "/ajax.php?oper=property-print&id_list=" + id_list + "&" + formData;
      
    gmap3_removemarkers(id); 
    var gmarkers = gmap3_loadmarkers(id, search_markers, false); 
    //gmap3_fitbounds(id, gmarkers);
  }
  */ 
}

function gmap3_reset_markers(map_id) {
  $('#' + map_id).gmap('find', 'markers', { }, function(gmarker) {
    gmarker.setIcon(gmarker.icon_lo);
  });
}

function gmap3_set_marker_label(map_id, marker_id, label) {
  var $map = $('#' + map_id);
  var gmarkers = $map.data('gmarkers'); 
  var gm = gmarkers[marker_id];
  
  //console.log("set marker label markers object size=" + Object.size(gmarkers) + " ml=" + $map.length + " markers=", gmarkers);
  
  if(gm) {
    var type = gm.type;
    if(type == 'label') {
      gm.set('labelContent', label);
      //console.log("Found gmarker " + marker_id + " type=" + type + " new=" + label);
    }
  } else {
    //console.log("Did not find gmarker " + marker_id);
  }
  return gm;
}

function gmap3_set_marker_image(map_id, marker_id, icon) {
  var ids = marker_id.split(',');
  var len = ids.length;
  var $map = $('#' + map_id);
  var gmarkers = $map.data('gmarkers');
  if(!gmarkers) return;
  var gm = gmarkers[marker_id];
  if(gm) {
    //console.log("Found gmarker " + marker_id);
    var type = gm.type;
    if(type == 'label') {
      if(icon) {
        // set ztop of parent
        gm.setZIndex(1000);
      } else {
        gm.setZIndex(100);
        // clear ztop of parent
      }
      var lclass = icon ? "white redbg" : "white bluebg";
      var licon = icon ? svgSymbol('bubble', 'red') : svgSymbol('bubble', 'blue');
      gm.set("labelClass", lclass);
      gm.set("icon", licon);
    } else {
      var pin = icon ? gm.icon_hi : gm.icon_lo;
      var type = gm.type;
      //console.log("Found marker with icon type=" + type + " id=" + gm.id);
      gm.setIcon(pin);
    }
  } else {
    //console.log("Did not find gmarker " + marker_id);
  }
  
  /** used to work 
  $('#' + map_id).gmap('find', 'markers', { }, function(gmarker) {
    var icon = gmarker.id == marker_id ? gmarker.icon_hi : gmarker.icon_lo;
    var type = gmarker.type;
    //console.log("Found marker with type=" + type + " id=" + gmarker.id);
    gmarker.setIcon(icon);
  });

  $("#" + map_id).gmap("find", "markers", {"property": "ref", "value": marker_id }, function(gmarker, isFound) { // doesnt work as intended
    if(isFound) {
      //console.log("found icon=" + icon);
      if(!icon) icon = gmarker.icon_lo; // reset to standard
      gmarker.setIcon(icon);
    }
  });
  */
  return null;
}

// given id in property-list, get marker id
function gmap3_get_marker_id(markers, elid) {
  var elar = elid.split('-');
  
  var obj_type = elar[0];
  var id = elar[1];
  
  var m = markers[id];
  if(!m) m = gmap3_find_marker(markers, id);
  if(list = m.id_list) {
    marker_id = obj_type + '_' + m.id_list;
  } else {
    marker_id = obj_type + '_' + m.id;
  }
  
  return marker_id;
}

/**

function gmap3_maplink_init(map_options) {
  var search_marker;
  var map_id = map_options.id;
  var $map = $('#' + map_id);  
  var accuracy = map_options.accuracy;
  var center = map_options.center;
  $map.data('options', map_options);
  $map.hide();
  var acc2 = 0;
  console.log("gmap3_maplink_init");
  $(".property-list-box").hover(
    function () {
      var elid = $(this).prop("id");
      var marker_id = gmap3_get_marker_id(markers, elid);
      var icon = "/images/markers/pushpin_yellow.png";
      console.log("enter " + marker_id);
      gmap3_set_marker_image(map_id, marker_id, icon);
    }, 
    function () {
      var elid = $(this).prop("id");
      var marker_id = gmap3_get_marker_id(markers, elid);
      console.log("leave " + marker_id);
      gmap3_set_marker_image(map_id, marker_id, '');
    }
  );
  $(".map_link").hover(
    function () {
      var $map = $('#gmap_popup');
      if(!$map.length) {
        //console.log("could not find gmap_popup");
        return;
      }
      var id = $(this).prop("id").split('-').pop();
      var m = markers[id];
      if(!m) m = gmap3_find_marker(markers, id);  
      if(m) {
        var marker_id = m.id_list ? m.id_list : m.id;
        var mymarkers = {id: marker_id};
        var $property = $('#property-' + id);
        var $of = $property.length ? $property : $(this);
        $map.show().position({of: $of, my: "right top", at: "right top"});
        //var center = new google.maps.LatLng(centerAr[0], centerAr[1])
        var position = new google.maps.LatLng(m.location_lat, m.location_long);
        gmap3(map_id, {}, {center: center, zoom: 10}); // no markers      
        var markerOpts = {"id": "marker_" + m.id, "title": m.name, "icon": m.marker ? m.marker : gmap_icon, "position": position, "bounds": true};
        $map.gmap("addMarker", markerOpts); // add markers

        // invisble city marker to prevent zooming all the way into to single property marker
        var markerOpts = {"id": "marker_city", "title": "amsterdam", "icon": gmap_icon, "position": center, "bounds": true, "visible": false};
        $map.gmap("addMarker", markerOpts); // add markers
      }
    }, 
    function () {
      var id = $(this).prop("id").split('-').pop();
      var m = markers[id];
      if(!m) m = gmap3_find_marker(markers, id);
      if(m) {
        var marker_id = m.id_list ? m.id_list : m.id;
        $map.gmap("clear", "markers");
        $("#gmap_popup").hide();
      }
    }
  );
}
*/

function gmap3_circle(map_id, options) {
  var $map = $('#' + map_id);
  var circleOptions = {
    strokeColor: "#FF0000",
    strokeOpacity: 0.8,
    strokeWeight: 2,
    fillColor: "#FF0000",
    fillOpacity: 0.35,
    center: options.center,
    radius: options.radius
  };
  //alert("added circle to " + map_id + " with these options " + dump(circleOptions));
  $map.gmap('addShape', 'Circle', circleOptions);

}

function boarding_map_init(which_map) {
  var map_id = "boarding_map";
  var $map = $("#" + map_id);
  var my_select_target = which_map == "show_boarding_map" ? "pickup_location" : "dropoff_location";
  $map.data("select_target", my_select_target);

  $( "#map_container" ).dialog({
      width: 650,
      height: 660,
      resizeStop: function(event, ui) {$map.gmap("refresh");},
      close: function(event, ui) {
        // $map.gmap("destroy");
      },
      open: function(event, ui) {
        var selected_id = $("#" + my_select_target).val();
        gmap3(map_id, markers, {"select_target": my_select_target, "selected_id": selected_id});
        if(selected_id) gmap3_marker_click(map_id, selected_id); 
      }
  });                 
}                 

function gmap3_marker_click(map_id, marker_id) {
  $("#" + map_id).gmap("find", "markers", {"property": "id", "value": "marker_" + marker_id }, function(gmarker) { // doesnt work as intended
    if(gmarker.id == "marker_" + marker_id) google.maps.event.trigger(gmarker, "click");
  });
}

function zoom2acc(zoom) {
  if(zoom<=2) return 0;
  if(zoom<=4) return 1;
  if(zoom<=6) return 2;
  if(zoom<=8) return 3;
  if(zoom<=10) return 4;
  if(zoom<=12) return 5;
  if(zoom<=14) return 6;
  if(zoom<=16) return 7;
  return 8;
}

function acc2zoom(accuracy) {
  if(accuracy<=0) return 1;
  if(accuracy==1) return 3;
  if(accuracy==2) return 5;
  if(accuracy==3) return 7;
  if(accuracy==4) return 9;
  if(accuracy==5) return 11;
  if(accuracy==6) return 13;
  if(accuracy==7) return 15;
  return 17;
}
$(document).on("mouseover", ".property-list-box, .air-item", function() {
  var map_id = 'search_result_map';
  var idAr = $(this).prop("id").split('-');
  var obj_type = idAr[0];
  var obj_id = idAr[1];
  var marker_id = obj_type + '_' + obj_id;
  var icon = "/images/markers/pushpin_yellow.png";
  //console.log('mousein on ' + marker_id);
  
  // clear all, just in case
  var $map = $('#' + map_id);
  var gmarkers = $map.data('gmarkers'); 
  for (var key in gmarkers) {
    gmap3_set_marker_image(map_id, key, '');
  }
  
  gmap3_set_marker_image(map_id, marker_id, icon);

});

$(document).on("mouseleave", ".property-list-box, .air-item", function() {
  gmap3_reset_markers('search_result_map');
  var map_id = 'search_result_map';
  var id = $(this).prop("id").split('-').pop();
  var marker_id = 'shortstay_' + id;
  //console.log('mouseout on ' + marker_id);
  gmap3_set_marker_image(map_id, marker_id, '');
});


// new function to show modal map using bootstrap3


function gmap3_update(map_id, lat, lng, title, address) {
  var map;
  //console.log("gmap3_update id=" + map_id + " lat=" + lat + " lng=" + lng + " title=" + title + " addr=" + address);
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
    icon: 'data:image/png;base64,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',
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

/** Dynamially load markers of given type(s) to existing map */
function gmap3_load_markers(map, types) {
  var gbounds = map.getBounds();
  var bounds = gmap3_bounds2json(gbounds);
  var bounds_str = JSON.stringify(bounds);            
  var ajax_url = "/ajax.php?oper=find-locations&loc_types=" + encodeURIComponent(types) + "&bounds=" + encodeURIComponent(bounds_str);
  $.getJSON(ajax_url, function(response) {
    if(response.success) {
      bounds_timer = 0;
      var markers = response.locs;
      var count = response.loc_count;
      //console.log("OK, found " + count + " locs response:", response);
      $.each(markers, function(id, marker) {
        //console.log("marker:",marker);
        var latLng = new google.maps.LatLng(marker.location_lat, marker.location_long);
        var gmarker = new google.maps.Marker({
          position: latLng,
          map: map,
          icon: marker.marker
        });
        var contentString = gmap3_info_window_content(marker, {});
        var infowindow = new google.maps.InfoWindow({
          content: contentString
        });  
        gmarker.infowindow = infowindow;
        gmarker.addListener('click', function() {
          infowindow.open(map, gmarker);
        });
        
      });
    }
  });
}


function rental_map_update(cin, cout, $item, json, org_price) {
  var $price = $item.find("span.price-amount");
  var $currency = $item.find("span.currency");
  var $check = $item.find("span.price-status");
  var $status = $item.find("div.listing-status");
  var data = $item.data('data');
  var type = $item.data('type');
  var id   = $item.data('id');
  var fid  = $item.data('fid');
  var cid  = cin + cout + id;

  var mid  = type + '_' + id; /** google map marker array key */
  
  success = json.success;
  var message = json.message;
  var error = json.error;
  if(success) {
    var data = json.data;
    var cur = currency2symbol(data.currency);
    var avg = data.average;
    var avg_round = Math.round(avg);
    var discount = data.discount;
    var avgd = data.average_discounted;
    var book_direct = data.book_direct;
    var gm = gmap3_set_marker_label(map_id, mid, cur + '' + avg_round);            
    $currency.html(cur);
    $price.html(avg_round);
    $check.html("<i class='fa fa-check text-success'></i>");
    
    var iw = gm.iw;
    if(gm.iw) {
      //console.log("found iw for " + cid);
    }
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


/** Initialize map + marker from data elements */
function gmap_init(map_id) {
  var $map = $("#" + map_id);
  if(!$map.length) return;
  var lat = $map.data('lat'); 
  var lng = $map.data('lng'); 
  if(!lat && lng) return;
  var title = $map.data('title');
  var address = $map.data('address');
  console.log("initModalMap at " + lat + "/" + lng);
  var mapOptions = {
    center: new google.maps.LatLng(lat, lng),
    zoom: 12,
    mapTypeId: google.maps.MapTypeId.ROADMAP
  };
  var map = new google.maps.Map(document.getElementById(map_id), mapOptions);
 
  if(title || address) {
    var contentString = "<h4>" + title + "</h4>" + address;
    var infowindow = new google.maps.InfoWindow({
      content: contentString
    });
      
    var marker = new google.maps.Marker({
      position: new google.maps.LatLng(lat, lng),
      title: title
    });
    marker.addListener('click', function() {
      infowindow.open(map, marker);
    });          
    marker.setMap(map);
  }
}      

// end googlemap.js

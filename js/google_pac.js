g_default_radius = 2000;
g_default_radius = 30; // close to max-zoom
g_loc_accuracy = 4; // decimal places in lat/lng

function initialize() {
  // Create the autocomplete object, restricting the search
  // to geographical location types.
  if(!document.getElementById('pac_autocomplete')) return false;
  autocomplete = new google.maps.places.Autocomplete(document.getElementById('pac_autocomplete')  , { types: ['geocode'] });
      
  // When the user selects an address from the dropdown,
  // populate the address fields in the form.
  google.maps.event.addListener(autocomplete, 'place_changed', function() {
    fillInAddress();
  });
}        


function google_pac_select(event, ui) {
  var $pac = $('#bounds');
  var target = $pac.data('target'); 
  var $target = target ? $('#' + target) : null;  
        
  if(!ui.item) return false;

  var loc_id = ui.item.id || 0;
  $("#location_slug").val('');
  
  //console.log("google_pac_select: form=" + $form.length + " auto=" + autosubmit);
  console.log(ui.item ? ("Selected: " + ui.item.value + " aka " + loc_id + " target=" + target) : "Nothing selected, input was " + this.value);
  
  if(ui.item.value) {
    console.log("OK: item", ui.item);
  } else {
    //alert("oops");
    console.log("oops, no value item", ui.item);
    return false;
  }
  //return;
  
  console.log("gpac v2.6d");
  var bounds = [];
  var bounds_str = '';
  var item = ui.item;
  if(item.ref) {
    console.log("ref=" + item.ref);
    google_pac_details(item.ref);          
  } else if(item.location_bounds || (item.location_lat && item.location_long)) {

    var lat = item.location_lat;
    var lng = item.location_long;

    lat = lat ? parseFloat(lat).toFixed(g_loc_accuracy) : 0;
    lng = lng ? parseFloat(lng).toFixed(g_loc_accuracy) : 0;
    var loc = [lat,lng];

    console.log("loc=", loc)
    
    var loc_str = JSON.stringify(loc);

      
    if(!item.location_bounds) {
      var lat0 = item.location_lat;
      var lng0 = item.location_long;
      var lat1 = item.location_lat;
      var lng1 = item.location_long;
      bounds = [[lat0,lng0],[lat1,lng1]];
      console.log("No bounds, using point:", bounds);
      bounds_str = JSON.stringify(bounds);
    } else {    
      console.log("db bounds = " + item.location_bounds)
      var boundsObj = parse_json(item.location_bounds) || {};
      console.log("BoundsObj=", boundsObj)
      
      if(boundsObj.northeast) {
        var ne = boundsObj.northeast;    
        var sw = boundsObj.southwest;
        var lat0 =  sw ? parseFloat(sw.lat).toFixed(g_loc_accuracy) : 0;
        var lng0 =  sw ? parseFloat(sw.lng).toFixed(g_loc_accuracy) : 0;
        var lat1 = ne ? parseFloat(ne.lat).toFixed(g_loc_accuracy) : 0;
        var lng1 = ne ? parseFloat(ne.lng).toFixed(g_loc_accuracy) : 0;      
        console.log("ne=" + lat0 + "/" + lng0 + " lng=" + lat1 + "/" + lng1);
        bounds = [[lat0,lng0],[lat1,lng1]];
        bounds_str = JSON.stringify(bounds);
      } else {
        console.log("bounds already array");
        bounds_str = boundsObj; // already json in database
      }
    }

    var slug = item.slug || '';
    if(slug && loc_id) slug = slug + '~' + loc_id;
    $("#location_slug").val(slug);    
    $(".location-center").val(loc_str);
    $(".location-bounds").val(bounds_str);

    var $form = $pac.length ? $pac.closest("FORM") : null;
    var autosubmit = $form ? $form.hasClass('autosubmit') : false;
    $("#loc").val(ui.item.value);
    
    console.log("setting slug field of len= " + $("#location_slug").length + " to " + slug);
    // console.log("loc=" + loc_str + " bounds=" + bounds_str);
    //console.log('google_pac_select: loc len= ' + $("#loc").length + ' status=' + status + " autosubmit=" + autosubmit);
    //if(autosubmit) {
    //  $(".jq-autocomplete-submit").trigger("click");
    //}
    ////alert('gh');
    
    if(autosubmit) {
      $form.submit();
    }

  } 
  
}  
  
function google_pac_response(e,ui) {
  var input = $(this).val()
  console.log("google_pac_response v2 input=" + input);
  
  // these reset location, in case user clicks submit instead of selecting PAC response
  $("#location_slug").val('');    
  $(".location-center").val('');
  $(".location-bounds").val('');
  
  var google_service = new google.maps.places.AutocompleteService();
  google_service.getPlacePredictions({ input: input, types: ['geocode']}, pac_callback); // integrate google places autocomplete with search result
}

function google_pac_renderer(ul, item) {

  var obj_type = item.obj_type;
  //var term = $(this).val();
  var inner_match = '', outer_rest = '';
  var inner_rest = item.label;
  if(rest = item.address_formatted) {
    var restAr = rest.split(', ');
    restAr.shift();
    //console.log("restAr=");
    //console.log(restAr);
    if(restAr.length) outer_rest= restAr.join(', ');
  }
  var icon_class = '';
  switch(obj_type) {
  case 'airport':
    icon_class = 'fa fa-plane';
  break;
  case 'golf_club':
    icon_class = 'glyphicon glyphicon-flag';
    break;
  case 'golf_course':
    icon_class = 'glyphicon glyphicon-flag';
    break;
  }
  var google_item = google_pac_item(inner_match, inner_rest, outer_rest, icon_class);
  //console.log("render item =");
  //console.log(item);
  //console.log(google_item);
  return $("<li>").append('<a class="ui-corner-all">' + google_item + '</a>').appendTo(ul);
};

function geolocate() {
  if (navigator.geolocation) {
    navigator.geolocation.getCurrentPosition(function(position) {
      var geolocation = new google.maps.LatLng(
          position.coords.latitude, position.coords.longitude);
      autocomplete.setBounds(new google.maps.LatLngBounds(geolocation,
          geolocation));
    });
  }
}
         




// Old:
// The START and END in square brackets define a snippet for our documentation:
// [START region_fillform]
function fillInAddress() {
  alert("fillInAddress - not in use?");
  return false;
  
  // Get the place details from the autocomplete object.
  var place = autocomplete.getPlace();
  $('#pac_city_name').val(place.name);
  $('#pac_location_lat').val(place.geometry.location.lat());
  $('#pac_location_long').val(place.geometry.location.lng());
     
  console.log("fillInAddress: lat len=" + $('#pac_location_lat').length)
  
  if(place.geometry.viewport) {
    console.log("fillInAddress: viewport = yes");
    $('#pac_bounds_ne').val(place.geometry.viewport.getNorthEast());
    $('#pac_bounds_sw').val(place.geometry.viewport.getSouthWest());
  } else {
    console.log("fillInAddress: viewport = no");
    $('#pac_bounds_ne').val('');
    $('#pac_bounds_sw').val('');
  }    
 
}
// [END region_fillform]



function initialize_map() {
  var mapOptions = {
    center: new google.maps.LatLng(-33.8688, 151.2195),
    zoom: 13
  };
  var map = new google.maps.Map(document.getElementById('map-canvas'),
    mapOptions);

  var input = (
      document.getElementById('pac-input'));

  var types = document.getElementById('type-selector');
  map.controls[google.maps.ControlPosition.TOP_LEFT].push(input);
  map.controls[google.maps.ControlPosition.TOP_LEFT].push(types);

  console.log("init map");
  
  var autocomplete = new google.maps.places.Autocomplete(input);
  autocomplete.bindTo('bounds', map);

  var infowindow = new google.maps.InfoWindow();
  var marker = new google.maps.Marker({
    map: map
  });

  google.maps.event.addListener(autocomplete, 'place_changed', function() {
    infowindow.close();
    marker.setVisible(false);
    var place = autocomplete.getPlace();
    if (!place.geometry) {
      return;
    }

    // If the place has a geometry, then present it on a map.
    if (place.geometry.viewport) {
      map.fitBounds(place.geometry.viewport);
    } else {
      map.setCenter(place.geometry.location);
      map.setZoom(17);  // Why 17? Because it looks good.
    }
    marker.setIcon(({
      url: place.icon,
      size: new google.maps.Size(71, 71),
      origin: new google.maps.Point(0, 0),
      anchor: new google.maps.Point(17, 34),
      scaledSize: new google.maps.Size(35, 35)
    }));
    marker.setPosition(place.geometry.location);
    marker.setVisible(true);

    var address = '';
    if (place.address_components) {
      address = [
        (place.address_components[0] && place.address_components[0].short_name || ''),
        (place.address_components[1] && place.address_components[1].short_name || ''),
        (place.address_components[2] && place.address_components[2].short_name || '')
      ].join(' ');
    }

    infowindow.setContent('<div><strong>' + place.name + '</strong><br>' + address);
    infowindow.open(map, marker);
  });

  // Sets a listener on a radio button to change the filter type on Places
  // Autocomplete.
  function setupClickListener(id, types) {
    var radioButton = document.getElementById(id);
    google.maps.event.addDomListener(radioButton, 'click', function() {
      autocomplete.setTypes(types);
    });
  }

  setupClickListener('changetype-all', []);
  setupClickListener('changetype-establishment', ['establishment']);
  setupClickListener('changetype-geocode', ['geocode']);
}

// from afr.js

// get details for ref
function google_pac_details(ref) {
  console.log('google_pac_details: ref=' + ref);
  if(ref) {
    var request = {
      reference: ref
    };      
    var node = document.getElementById('bounds');
    service = new google.maps.places.PlacesService(node);
    service.getDetails(request, google_details_callback);
  }
}

// get details (lat,lng,bounds) of a place
function google_details_callback(place, status) {
  
  var $pac = $('#bounds');
  var $form = $pac.length ? $pac.closest("FORM") : null;
  var autosubmit = $form ? $form.hasClass('autosubmit') : false;
  console.log('google_details_callback: status=' + status + " autosubmit=" + autosubmit);
  //console.log(place);
  if (status == google.maps.places.PlacesServiceStatus.OK) {
    console.log("ok, details=");
    console.log(place);
       
    var name = place.formatted_address;
    console.log("ok, name=" + name);

    var result = pac_parse_geometry(place);
    //console.log("custom result =", result);

    var loc = result.center;
    var bounds = result.bounds;

    var loc_str = JSON.stringify(loc);
    var bounds_str = JSON.stringify(bounds);
    console.log("loc str=" + loc_str + " bounds str=" + bounds_str);
    $(".location-center").val(loc_str);
    $(".location-bounds").val(bounds_str);

    if(autosubmit) $form.submit();
    
  }
}

// parses geometry returned by google place service
function pac_parse_geometry(place) {
  console.log("pac_parse_geometry v1:",place);

  var name = place.formatted_address;
  var geometry = place.geometry;
  var location = geometry.location;
  var lat=location.lat();
  var lng=location.lng();


  lat = lat ? parseFloat(lat).toFixed(g_loc_accuracy) : 0;
  lng = lng ? parseFloat(lng).toFixed(g_loc_accuracy) : 0;
  var loc = [lat,lng];
  var loc_str = JSON.stringify(loc);
  
  // set location_id to 0
  var target_id = $('#bounds').data('target');
  var $target = $('#' + target_id);
  $target.val('');
  
  // set location_id_textbox to place name
  var target_display_id = $('#bounds').data('target_display');
  var $target_display = $('#' + target_display_id);
  //$target_display.val(name);

  var gbounds = geometry.viewport;
  if(gbounds) {
    //console.log("bounds=");
    //console.log(gbounds);
    var ne = gbounds.getNorthEast();                
    var sw = gbounds.getSouthWest();

    var lat0 =  sw ? parseFloat(sw.lat()).toFixed(g_loc_accuracy) : 0;
    var lng0 =  sw ? parseFloat(sw.lng()).toFixed(g_loc_accuracy) : 0;
    var lat1 = ne ? parseFloat(ne .lat()).toFixed(g_loc_accuracy) : 0;
    var lng1 = ne ? parseFloat(ne .lng()).toFixed(g_loc_accuracy) : 0;

    //console.log("ne=" + lat0 + "/" + lng0 + " lng=" + lat1 + "/" + lng1);
    var bounds = [[lat0,lng0],[lat1,lng1]];
    var bounds_str = JSON.stringify(bounds);
    //console.log(bounds);
  } else {        
    var radius = g_default_radius;
    var bounds = radius2bounds(lat, lng, radius);
    var bounds_str = bounds_obj2json(bounds);

    console.log("no bounds calculating using lat/lng + radius = " + radius);
  }
  
  var result = {"center": loc, "bounds": bounds};
  console.log("result = ",result);
  
  return result;
}

// google places autocomplete
function pac_callback(predictions, status) {
  if (status != google.maps.places.PlacesServiceStatus.OK) {
    //alert(status);        
    return;
  }
  var $ul = $("UL.ui-autocomplete");
  if(!$ul.length) return; 
  
  //console.log(predictions);
  var description='', inner_term='',inner_match='', inner_rest = '', other_terms='',outer_rest= '', results='';
  var length=0,inner_offset=0,outer_offset=0;
  var list_items = [];
  console.log("pac_callback: len=" + predictions.length);
  for (var i = 0, prediction; prediction = predictions[i]; i++) {
    description = prediction.description; // e.g. "Laguna Hills, CA, United States"
    var matched_subs = prediction.matched_substrings;
    var terms = prediction.terms;

    if(terms.length == 1) {
      inner_term = terms[0].value; // e.g. "Laguna Hills"
      outer_rest = '';            
    } else if(terms.length > 1) {
      inner_term = terms[0].value; // e.g. "Laguna Hills"
      outer_offset = terms[1].offset;
      outer_rest = description.substring(outer_offset); // e.g. ", CA, United States"            
    } else {
      inner_term = description;
    }
    
    
    if(matched_subs.length) {
      var ms = matched_subs[0]; // ignore the rest
      inner_offset = ms.offset;
      inner_length = ms.length;
      inner_match = inner_term.substr(inner_offset, inner_length);
      inner_rest = inner_term.substring(inner_length);
    } else {
      inner_match = inner_term;   
      inner_rest = '';
    }
    
    var google_item = google_pac_item(inner_match, inner_rest, outer_rest);
    
    var desc = prediction.description;
    desc = desc.replace(/"/g, ""); /** no double quotes */
    desc = desc.replace(/'/g, ""); /** escape single quotes */
    
    var item = {
      "id": "3982",
      "obj_type": "location",
      "address_country": "",
      //"address_formatted": desc,
      //"label": desc,
      "value": desc,
      "ref": prediction.reference,
    }
    
    results += '<li role="presentation" data-ui-autocomplete-item=\'' + JSON.stringify(item) + '\' class="ui-menu-item"><a class="ui-corner-all google-pac-link" data-reference="' + prediction.reference + '">' + google_item +'</a></li>';
  }

  if(results) { // old way
    show_autocomplete_menu($ul);
    $ul.append(results);
  }
}

// returns an <a> element used in autocomplete that looks like Google's own 
function google_pac_item(inner_match, inner_rest, outer_rest, icon_class) {
  var icon_class = icon_class ? 'icon ' + icon_class : 'pac-icon pac-icon-marker';
  var marker = '<span class="' + icon_class + '"></span>';
  var matched = '<span class="pac-matched">' + inner_match + '</span>' + inner_rest;
  var item_query = '<span class="pac-item-query">' + matched + '</span>';
  var other_terms = '<span>' + outer_rest + '</span>';
  
  var google_item = '<div class="pac-item">'+ marker + item_query + other_terms + '</div>';
  return google_item;
}


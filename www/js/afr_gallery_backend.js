console.log("afr_gallery_backend.js v2.11");

if(typeof g_section == "undefined") {
  var g_section = '';
  console.log("\n\n\nafr_gallery_bakend, no g_section defined..");
} else {
  console.log("\n\n\n afr_gallery_bakend: ok, g_section is defined... val=" + g_section);
  $(".navbar-nav li").removeClass('active');    
  $("#nav_section_" + g_section).addClass('active');
}

if(typeof g_ready_scripts == "undefined") var g_ready_scripts = [];
g_ready_scripts.push('gallery_backend_ready');

var g_section = 'undef';

function gbe_plugin_handler(params, data) {
  console.log("gbe_plugin_handler data=", data);
  window.location.href = "/backend/plugin/" + data.id;
}

function gbe_artist_handler(params, data) {
  $("#artistTags").tagit("createTag", data.display_name + '|' + data.user_id);
}

function gbe_title_image_handler(params, data) {
  window.console && console.log("gbe_title_image_handler params=", params);
  window.console && console.log("gbe_title_image_handler data=", data);
  var file = null;
  if(data && data.files) file = data.files[0];
  window.console && console.log("property_photo_handler file=", file, " type=" + (file ? file.type : 'none'));
  if(file && file.type == 'image') {
    var src = file.url;
    console.log("src=" + src + " target len=" + $("#collection_title_image").length);
    $("#collection_title_image").attr('src', src);
  }
  return true;
}
  
function gbe_collection_handler(params, data) {
  console.log("gbe_collection_handler. data=", data);
  console.log("gbe_collection_handler. next="+ data.next_tab + " data:", data);
  if(data.id) {
    var tab = data.next_tab || 'location';
    if(((data.subtype == 40) || (data.subtype == 50)) && (tab == 'location')) tab = 'media';
    var href = '/backend/exhibition/' + data.id + '/edit/' + tab;
    var $link = $("#submit");
    soft_load($link, "#subview-container", href);
  }
}

function gbe_artwork_handler(params, data) {
  console.log("gbe_artwork_handler. params:", params);
  console.log("gbe_artwork_handler. next="+ data.next_tab + " data:", data);
  if(data.id) {    
    var tab = data.next_tab || 'details';
    var href = '/backend/artwork/' + data.id + '/edit/' + tab;
    var $link = $("#submit");
    soft_load($link, "#subview-container", href);
  }
}

function gbe_network_handler(params, data) { //pradeepa
  console.log("gbe_network_handler. data", data);
  console.log("gbe_network_handler. next="+ data.next_tab + " data:", data);
  if(data.id) {    
    var tab = data.next_tab || 'about';
    var href = '/backend/profile/' + data.id + '/edit/' + tab;
    var $link = $("#submit");
    soft_load($link, "#subview-container", href);
  }
}

function gbe_website_handler(params, data) { //pradeepa
  console.log("gbe_website_handler. data", data);
  console.log("gbe_website_handler. next="+ data.next_tab + " data:", data);
  if(data.id) {    
    var tab = data.next_tab || 'exhibition';
    var href = '/backend/website/' + data.id + '/edit/' + tab;
    var $link = $("#submit");
    soft_load($link, "#subview-container", href);
  }
}

function gbe_artists_tagit() {
  console.log("gbe_artists_tagit");
  $("#artistTags").tagit({
    allowSpaces: true,
    placeholderText: "Type artist name",
    allowDuplicates: false,
    //singleField: true,
    //singleFieldNode: $("#artist_ids"),
    autocomplete: { 
      delay: 0, 
      minLength: 2,
      source: function( request, response ) {
      $.ajax({
        url: "/ajax.php",
        dataType: "json",
        data: {
          oper: "find-artists",
          limit: 12,
          val: request.term
        },
        success: function( result ) {
          if(result.success) {
            console.log("OK, data:", result);
            response( $.map( result.data, function( item ) {
              var name = (item.display_name ? item.display_name : (item.first_name + ' ' + item.last_name));
              return {
                label: name, // + ", " + item.city,
                value: name + '|' + item.user_id
                //value: (item.display_name ? item.display_name : (item.first_name + ' ' + item.last_name)) 
              }
            }));
          } else {
            alert("error:" + result.error);
          }
        }
      });
    },
    minLength: 2,            
  },
  //preprocessTag: function(val) {
  //  // do something special
  //  var $tag = $(ui.tag);
  //  var $span = $tag.find('span');
  //  var $text = $span.html();
  //  var ar = $text.split('|');
  //  $span.html(ar[0]);
  //  $tag.data('id', ar[1]);
  //  console.log("Added tag id='" + ar[1] + "' name=" + ar[0]);
  //},
  beforeTagAdded: function(event, ui) {
    var $tag = $(ui.tag);
    var $span = $tag.find('span');
    var text = $span.html();
    var ar = text.split('|');
    var name = ar[0];
    var id = parseInt(ar[1]);
    console.log('adding tag text=' + text);
    
    //if(id && ar.length < 2) return true; // not from form
    
    if(!id) {
      var infix_list = ["de van der","uijt te de","van van de","voor in 't","de die le","onder den","onder het","uit te de","van de l’","voor in t","boven d’","onder 't","onder de","over den","over het","uijt den","uijt ten","van de l","voor den","aan den","aan der","aan het","auf dem","auf den","auf der","auf ter","aus dem","aus den","aus der","bij den","bij het","boven d","onder t","over 't","over de","uijt 't","uijt de","uit den","uit het","uit ten","van den","van der","van gen","van het","van ter","von dem","von den","von der","voor 't","voor de","vor der","aan 't","aan de","aus 'm","bij 't","bij de","de die","de las","die le","in den","in der","in het","op den","op der","op gen","op het","op ten","over t","uit 't","uit de","van 't","van de","van la","von 't","aan t","am de","aus m","bij t","dalla","de l’","de la","de le","degli","della","in 't","in de","onder","op 't","op de","uit t","unter","van t","von t","dal’","de l","deca","in t","op t","over","thoe","thor","uijt","voor","aan","auf","aus","ben","bij","bin","dal","das","dei","del","den","der","des","don","dos","het","las","les","los","ten","ter","tho","toe","tot","uit","van","ver","vom","von","vor","zum","zur","'s","'t","af","al","am","d’","da","de","di","do","du","el","im","in","l’","la","le","lo","of","op","s’","te","to","zu","a","d","i","l","s","t"];
      var len = infix_list.length;
      var infix;              
      for (var i = 0; i < len; i++) {
        infix = infix_list[i];
        if(name.indexOf(" " + infix + " ") > -1) {
          $("#af_infix").val(infix.toLowerCase());
          name = name.replace(" " + infix, '');
          break;
        }
      }
      var names = name.split(' ');
      var last = names.pop();              
      $("#af_last_name").val(last);
      $("#af_first_name").val(names.join(' '));              
      $("#newArtist").trigger('click');
      return false;
    }
    
    $span.html(name);
    $tag.data('id', id);
    console.log("Added tag id='" + id + "' name=" + name);
    var ids = $("#artist_ids").val() ? $.parseJSON($("#artist_ids").val()) : [];
    if($.inArray(id, ids) == -1) ids.push(id);
    $("#artist_ids").val(JSON.stringify(ids));
  },
  beforeTagRemoved: function(event, ui) {
    var $tag = $(ui.tag);
    // do something special
    var id = parseInt($tag.data('id'));
    console.log("Removed id=" + id);
    var ids = $("#artist_ids").val() ? $.parseJSON($("#artist_ids").val()) : [];
    var index = ids.indexOf(id);
    if (index > -1) ids.splice(index, 1);
    $("#artist_ids").val(JSON.stringify(ids));
  }  
  });
}


$(document).on("click", "#gbe_use_default_settings", function() {
  var default_settings_json = $("#gbe_default_settings").val();
  if(!default_settings_json) {
    alert("No default settings found");
    return false;
  }
  var default_settings = jQuery.parseJSON(default_settings_json);
  console.log("Settings defaults:", default_settings);
  var keys = ['title', 'full', 'buttons', 'tabs'];
  var ar, settings, target,key,$div, $cbs, $cb, checked, default_val,name;

  var $sections = $(".gbe-settings"); 
  $.each($sections, function(i, div) {
    $div = $(this);
    target = $div.data('target');
    key = $div.data('key');
    $cbs = $(this).find('input');
    settings = default_settings[key] || [];
    //console.log("key=" + key + " has " + $cbs.length + " inputs settings=", settings);   
    $.each($cbs, function(j, cb) {
      $cb = $(this);
      checked = $cb.prop('checked');
      name = $cb.attr('name')
      default_val = $.inArray(name, settings) > -1;
      //console.log(key + ":" + name + " checked " + checked + " default=" + default_val);         
      if(checked != default_val) {
        console.log("Changing " + key + ":" + name);
        $cb.bootstrapToggle('toggle');
      }
      //$cb.prop('checked', default_val);
    });
  });
});


$(document).on("change", ".gbe-settings input", function() {
  var $div = $(this).closest('.gbe-settings');
  var id = $div.attr('id');
  var settings = $div.data('settings');
  var target = $div.data('target');
  var key = $div.data('key');
  var checked = $(this).prop('checked');
  var $cb, checked, json;
  var ar = [];
  $.each(settings, function(key,val) {
    $cb = $div.find('input[name="' + key + '"]');
    checked = $cb.prop('checked');
    //console.log('key=' + key + ' val=' + checked);
    if(checked) ar.push(key);  
  });
  json = JSON.stringify(ar);
  console.log(target + "=" + json);
  $(target).val(json);  
  
  var options_json = $("#gallery_options").val();
  var options = options_json ? jQuery.parseJSON(options_json) : {};
  options[key] = ar;
  $("#gallery_options").val(JSON.stringify(options));  
  //console.log("active=" + checked + " id=" + id + " target=" + target + " settings=", settings);
});


$(document).on("click", "#sel_clear", function() {
  var $target = $(this); // fires when hidden for some reason
  console.log("sel clear target=",$target);
  $("INPUT[name='btSelectAll']").trigger('click');

  g_selections = [];
  bstUpdateSelection([]);
  $("#table").bootstrapTable('refresh', {pageSize: 20});
});


$(document).on("click", "#sel_add", function() {
  $link = $(this);
  console.log("sel add ids=", g_selections);
  var collection_id = $link.data('target_id');
  if(collection_id && g_selections.length) {
    var href = '/backend/exhibition/' + collection_id + '/edit/media?' + add_what + '=' + g_selections.join();
    var $link = $("#submit");
    console.log("Calling soft-load: href=" + href);
    soft_load($link, "#subview-container", href);
  } else {
  }
});

$(document).on("click", "#sel_add_profile", function() {
  $link = $(this);
  console.log("sel add ids=", g_selections);
  var artist_id = $link.data('target_id');
  console.log("target_id=", artist_id);
  if(artist_id && g_selections.length) {
    var href = '/backend/profile/' + artist_id + '/edit/art?' + add_what + '=' + g_selections.join();
    var $link = $("#submit");
    console.log("Calling soft-load: href=" + href);
    soft_load($link, "#subview-container", href);
    g_selections = [];
    bstUpdateSelection([]);
  } else {
  }

});

$(document).on("click", "#sel_show", function() {
  console.log("sel show ids=", g_selections);
  $("#table").bootstrapTable('refresh', {query: {id: g_selections}, pageSize: 100});  
});

$(document).on("click", "#sel_del_success", function() { //Bala
  console.log("sel Del ids=", g_selections);
    $('#table').bootstrapTable('remove', {
                field: 'id',
                //~ values: g_selections
            });
  $("#table").bootstrapTable('refresh', {query: {id: g_selections, mode: 'del'}, pageSize: 10});
  $("#table").bootstrapTable('refresh', {pageSize: 20});
});

//Delete artist in minisite
$(document).on("click", '#soft_delete', function(){
  var artist_id = $(this).data('value');
  var page_id = $(this).data('page');
  var obj_type = "minisite";
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "artist-delete",
      artist_id: artist_id,
      page_id: page_id
    },     
  });     
});

//Delete minisite pages
$(document).on("click", '#page_delete', function(){
  var id = $(this).data('value');
  var obj_type = "minisite_page";
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "page-delete",
      val: id
    },     
  });     
});

//Delete events
$(document).on("click", '#event_delete', function(){
  var id = $(this).data('value');
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "event-delete",
      val: id
    },
    success:function(response_data_json) {
      $("#gbe_events_link").trigger('click');
    }     
  });     
});

//dynamic table edit on artwork module 
$(document).on("click", ".edit", function() { 
  var id = $(this).data('value');
  var obj_data = $(this).text();
  var obj_type= "artwork";
  var col = $(this).data('col');
  if(obj_data == 'N/A'){
    $(this).prop('contenteditable', false);
  }else{
    $(this).prop('contenteditable', true);
    $(this).css('display', 'inline-block');
    $(this).css('width', '100%');
  }    
});

$(document).on("focusout", ".edit", function() { 
  $(this).prop('contenteditable', false);
  var id = $(this).data('value');
  var col = $(this).data('col');
  var data = $(this).text();
  if(col=="price" && data==""){
    $(this).text("0.00");
  }
  if(col=="primary_year" && data.length!=4){
    alert("Year is not proper. Please check");
    return false;
  }
  $.ajax({ url: "/ajax.php?obj_type=" + obj_type,
   type: "POST",
    data: {
      type : "artwork",
      col: col,
      data : data,
      id : id,
      oper : "edit",
    }  
  });    
});  

//For price option in artwork
$(document).on("click", "#phide", function() { 
  $('.phide'+id).show();
  var id = $(this).data('value');
  var obj_type= "artwork";
  var obj_data = $(this).text();
  var col = $(this).data('col');
  if(col=='price_option'){ 
    $('.phide'+id).hide();
    $('#shows'+id).show();
    $('#select').remove();      
    $('#shows'+id).append('<select id= "select" class="select'+id+'"><option>Select</option><option id= "option" value="1">Show price</option><option  id= "option" value="0">Hide price</option><option  id= "option" value="-1">Price on enquiry</option></select>');
    $(document).on("change", "#select", function() {
      var value1 = $(this).parent().prop('id');
      value1 = value1.replace('shows','');
      var value = $('#shows'+value1+' #select').val();
      var obj_type= "artwork";
      var col = $('#phide').data('col');
      $('.select'+value1).show(value);
      $.ajax({ url: "/ajax.php?obj_type=" + obj_type,
        type: "POST",
        data: {
          type : "artwork",
          col: col,
          data : value,
          id : value1,
          oper : "edit",
        },
        success:function(response_data_json) {
          var value2='Hide Price';
          $('#shows'+value1).hide();
          if(value==0)       
             value2="Hide Price";
          if(value==1)       
             value2="Show Price";
          if(value==-1)       
            value2="Price on enquiry";
            $('.phide'+value1).text(value2);
          $('.phide'+value1).show();
        }
      });    
    });
  }    
});

// For Status drop down in artwork
$(document).on("click", "#status", function() { 
  $('.status'+id).show();
  var id = $(this).data('value');
  var obj_type= "artwork";
  var obj_data = $(this).text();
  var col = $(this).data('col');

  if(col=='status'){ 
    $('.status'+id).hide();
    $('#cstatus'+id).show();
    $('#select1').remove();      
    $('#cstatus'+id).append('<select id= "select1" class="select1'+id+'"><option>Select</option><option id= "option" value="10">Available</option><option  id= "option" value="20">Reserved</option><option  id= "option" value="30">Sold</option><option  id= "option" value="40">Inactive</option><option  id= "option" value="50">On loan, etc.</option></select>');

    $(document).on("change", "#select1", function() {
      var value1 = $(this).parent().prop('id');
      value1 = value1.replace('cstatus','');
      var value = $('#cstatus'+value1+' #select1').val();
      var obj_type= "artwork";
      var col = $('#status').data('col');
      $('.select1'+value1).show(value);
      $.ajax({ url: "/ajax.php?obj_type=" + obj_type,
        type: "POST",
        data: {
          type : "artwork",
          col: col,
          data : value,
          id : value1,
          oper : "edit",
        },
        success:function(response_data_json) {
          var value2='Available';
           $('#cstatus'+value1).hide();
           if(value==10)       
             value2="Available";
           if(value==20)       
             value2="Reserved";
           if(value==30)       
            value2="Sold";
           if(value==40)       
            value2="Inactive";
           if(value==50)       
            value2="On loan, etc.";
            $('.status'+value1).text(value2);
          $('.status'+value1).show();
        }
      });    
    });
  }            
});

//dynamic table edit on exhibition module 
$(document).on("click", ".edit_exhibition", function() { 
  $(this).prop('contenteditable', true);
  $(this).css('display', 'inline-block');
  $(this).css('width', '100%');
  var id = $(this).data('value');
  var obj_type= "media_collection";
  var obj_data = $(this).text();
  var col = $(this).data('col');
}); 

$(document).on("focusout", ".edit_exhibition", function() { 
  $(this).prop('contenteditable', false);
  var obj_type= "media_collection";
  var id = $(this).data('value');
  var col = $(this).data('col');
  var data = $(this).text();
  $.ajax({ url: "/ajax.php?obj_type=" + obj_type,
   type: "POST",
    data: {
      type : "media_collection",
      id : id,
      col: col,
      data : data,      
      oper : "edit",
    }
  
  });  
}); 

//change font type
$(document).on("change", ".font_type", function() {
  var font_value = $('.font_type:checked').val();
  var obj_id = $('.obj_id').val();
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "save-minisite-style",
      obj_id: obj_id,
      obj_type:'minisite',
      font_type: font_value
    }
  }); 
});

//change menu layout
$(document).on("change", ".menu_type", function() {
  var menu_type = $('.menu_type:checked').val();
  var obj_id = $('.obj_id').val();
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "save-minisite-style",
      obj_id: obj_id,
      obj_type:'minisite',
      menu_type: menu_type
    }
  }); 
});

//change exhibition page layout
$(document).on("change", ".page_layout", function() {
  var page_layout = $('.page_layout:checked').val();
  var obj_id = $('.obj_id').val();
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "save-minisite-style",
      obj_id: obj_id,
      obj_type:'minisite_page',
      page_layout: page_layout
    }
  }); 
});

//change exhibition layout
$(document).on("change", ".exhibition_layout", function() {
  var exhibition_layout = $('.exhibition_layout:checked').val();
  var obj_id = $('.obj_id').val();
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "save-minisite-style",
      obj_id: obj_id,
      obj_type:'minisite_page',
      exhibition_layout: exhibition_layout
    }
  }); 
});

//change contact page layout
$(document).on("change", ".contact_page_layout", function() {
  var page_layout = $('.contact_page_layout:checked').val();
  var obj_id = $('.obj_id').val();
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "save-minisite-style",
      obj_id: obj_id,
      obj_type:'minisite_page',
      page_layout: page_layout
    }
  }); 
});

//change menu colour
$(document).on("change", ".bg_colour, .colour", function() {
  var exhi_colour = $('.bg_colour:checked').val();
  var obj_id = $('.obj_id').val();
  var minisite_id = $('.minisite_id').val();
  var colour = null;

  if(exhi_colour == 2)
    var colour = $('#focus-demo').val();
 
  $.ajax({
    url: "/ajax.php",
    dataType: "json",
    data: {
      oper: "save-minisite-page",
      obj_id: obj_id,
      minisite_id: minisite_id,
      colour: colour
    }
  }); 
});

function bstMinisiteArtistFormatter(value, row, index) {
  if(!value) return '';
  return g_artists[value];
}

$(document).on("change", "#gbe_collection_active", function() {
  var checked = $(this).prop('checked');
  $cbs = $('.gbe-visibility-row');
  if(checked) $cbs.show('fast');
  else $cbs.hide('fast');
  
  //$cbs.prop('disabled', (checked ? false : true));
  //console.log("active=" + checked + " cbs=" + $cbs.length);
});
  
$(document).on('dirty.areYouSure', 'form', function() {
  // Enable save button only as the form is dirty.
  console.log("Form dirty form=" + $(this).attr('id'));
  //$(this).find('input[type="submit"]').removeAttr('disabled');
  g_form_dirty = 1;
});

$(document).on('clean.areYouSure', 'form', function() {
  // Form is clean so nothing to save - disable the save button.
  console.log("Form clean");
  //$(this).find('input[type="submit"]').attr('disabled', 'disabled');
  g_form_dirty = 0;
});

$(document).on('click', 'a', function(e) {
  $link = $(this);
  var href = $link.attr('href');
  
  if($link.hasClass('ajax-loader') || $link.hasClass('subview-nav-link')) {
    console.log("click on A ajax-loader or subview-nav-link, delegate");
    return;
  }

  if(href == '#' || !href) {
    console.log("click on A w/o href, do nothing");
    return;
  }
    
  if(g_form_dirty) {
    return; /** disabled for now */
    e.preventDefault();
    console.log("click on A - form dirty a href=" + $(this).attr('href'));
    // return confirm_modal($link);
    //return false;      
  } else {
    console.log("click on A - form clean");
  }    
});
                                  
function confirm_modal($link) {
  return confirm('You have unsaved changes. Are you sure you want to leave?');
  
  console.log("Showing confirm modal");
  $link.confirmModal({
    confirmTitle     : 'Custom please confirm',
    confirmMessage   : 'Custom are you sure you want to perform this action ?',
    confirmOk        : 'Custom yes',
    confirmCancel    : 'Cutom cancel',
    confirmDirection : 'rtl',
    confirmStyle     : 'primary',
    confirmCallback  : gbe_confirm_callback,
    confirmDismiss   : true,
    confirmAutoOpen  : false
  });      
}

function bstUpdateSelection(selections) {
  console.log("Selections = ",selections);
  if(selections.length) {
    g_selections = selections.concat(g_selections).unique();  
  }  
  console.log("GSelections = ",g_selections);
  var count = g_selections.length;
  $("#sel_count").text(count);
  if(count) {
    $("#selection").show();
  } else {
    $("#selection").hide();
  }
}
function getIdSelections() {
  return $.map($("#table").bootstrapTable('getSelections'), function (row) {
    return row.id
  });
}

function responseHandler(res) {
  $.each(res.rows, function (i, row) {
    row.state = $.inArray(row.id, selections) !== -1;
  });
  return res;
}
// function bstArtworkTypeFormatter(value, row, index) {
//   switch(parseInt(row.type)) {
//     case 10: return 'Photography';
//     case 11: return 'Print';
//     case 20: return 'Painting';
//     case 21: return 'Drawing';
//     case 22: return 'Mixed Media on paper/canvas';
//     case 30: return 'Sculpture';
//     case 40: return 'Cast (Bronze, etc)';
//     case 50: return 'Film & Video';
//     case 60: return 'Installation';
//     case 70: return 'Performance';
//     case 99: return 'Other';
//     default: return '';
//   }      
// }
// function bstArtworkStatusFormatter(value, row, index) {
//   switch(parseInt(row.status)) {           
//     case 10: return 'Available';
//     case 20: return 'Reserved';
//     case 30: return 'Sold';
//     case 40: return 'Inactive';
//     case 50: return 'On loan, etc.';
//     default: return '';
//   }      
// }

function bstCollectionSubtypeFormatter(value, row, index) {
  switch(parseInt(row.subtype)) {           
    case 10: return 'Gallery';
    case 20: return 'Art Fair';
    case 30: return 'Other Event';
    case 40: return 'Selection';
    default: return '';
  }      
}

function bstPublishedFormatter(value, row, index) {
  if(parseInt(row.active)) return 'ON';
  else return 'OFF';   
}

// function bstDateFormatter(value, row, index) {
//   if(!row.created) return '';
//   return sql2human_short(row.created);
// }

function bstStartDateFormatter(value, row, index) {
  if(!row.start_date) return '';
  return sql2human_short(row.start_date);
}

function bstEndDateFormatter(value, row, index) {
  if(!row.end_date) return '';
  return sql2human_short(row.end_date);
}

function bstArtistFormatter(value, row, index) {
  if(!value) return '';
  return g_artists[value];
}

function bstLocationFormatter(value, row, index) {
  if(!row.address_formatted) return '';
  return row.address_formatted;
  
}

function bstArtistsFormatter(value, row, index) {
  if(!value) return '';
  var val = JSON.parse(value);
  var new_val = [];
  for(var i=0; i<val.length; i++){
     new_val += g_artists[val[i]] + ",";
  }

    return new_val.slice(0, -1); 
}

function gbe_confirm_callback() {
  console.log("Confirmed...");
}

function gallery_backend_ready() {
  console.log("gallery_backend_ready v2.1 ARE YOU SURE");

  $('form').areYouSure( {'silent':true} );
  
  //$('form').trigger('reinitialize.areYouSure');
  

  $(document).on("change",'#gbe_loc_id',function() {
    var $cb = $(this);
    var val = $cb.val();
    if(val == 0) {
      $("#gbe_location").show();
    } else {
      $("#gbe_location").hide();
    }
    console.log("loc type change val=" + val);
  });
  
  var $table = $('#table');
  $remove = $('#remove');
    
  if(typeof g_section == "undefined") var g_section = '?';
  console.log("Backend Ready... section=" + g_section);
  if($table.length) {
    console.log("Backend Ready, loading table...");
    $table.bootstrapTable().on('load-success.bs.table', function(e, data) {
      if(g_selections.length) {
        var rows = data.rows;
        $.each(rows, function(k, v) {

           console.log("id=" + v.id);
           if($.inArray(v.id, g_selections) > -1) $table.bootstrapTable('check', k);
        });
      }

      console.log("load sel=" + g_selections.length + " data=", data);      
    });  
    
    $table.on('uncheck.bs.table', function (e, row) {
      $.each(g_selections, function(index, id) {
        if (id === row.id) {
          g_selections.splice(index, 1);
        }
      });
      console.log("Ok, removed " + row.id + " sel=",g_selections);
      bstUpdateSelection([]);
    });
    
    $table.on('check.bs.table check-all.bs.table', function () {
      var $target = $(this);
      var selections = getIdSelections();
      bstUpdateSelection(selections);
    });
    
    $table.on('uncheck-all.bs.table', function () {
      var rows = $table.bootstrapTable('getData');
      var selections = g_selections.slice(); // copy
      
      //var pageIds = [];
      var ids = $.map(rows, function (row) {
        return row.id
      });
      console.log("uncheck all rows:", ids);
      console.log("sel:", selections);
      $.each(selections, function(index, id) {
        if($.inArray(id, ids) > -1) {
          g_selections.shift();
          console.log(index + ": Remove id=" + id + " sel:", g_selections);
        } else {
          console.log(index + ": Do not remove id=" + id + " not in:", g_selections);
        }
      });
      console.log("local sel:", selections);
      
      bstUpdateSelection([]);
    });
    
    $(document).on("click", "#remove", function() {
      var ids = getIdSelections();
      console.log("Remove click: ids=",ids);
      if(ids) {
        $("#table").bootstrapTable('remove', {
          field: 'id',
          values: ids
        });
        console.log("Removed:",ids);
        $remove.prop('disabled', true);
      }
    });
    
  }
  $(".export.btn-group").append(" <span class='export_datan'><b>Download Records</b><a data-toggle='tooltip' data-placement='right' title='Download the art records shown in the table (to download all records set the row selector to All ) Make sure you add the correct extension to the downloaded file (for example .xls for Excel files, artrecords.xls)'> <i class='fa fa-info-circle'></i></a></span>");
  $(".keep-open.btn-group").append(" <span class='export_datan'><b>Select Columns</b></span>");  
     
	 $(".pagination-detail > .dropdown-toggle").click(function(){        
      $(".btn-group.dropup").removeClass("dropup");
  });
   
      $("body").tooltip({ selector: '[data-toggle=tooltip]' });


}

$(document).on("click", ".clone_modal_icon", function() {  
  $("#clone_dialog_form input[name=obj_id]").val($(this).attr('id'));
});

$(document).on("click", "apply-delete",function (e) {
   // Hide image on click delete
    e.preventDefault();
    if($(this).closest('li').length){
      $(this).closest('li').hide();
      console.log("media deleted");
    }             
    else{
      $(this).closest('tr').hide();
      console.log("artwork media deleted");
    }
});







/**
$(document).ready(function() {  
  var $table = $("TABLE.table");
  $table.bootstrapTable().on('load-success.bs.table', function(e, data) {      
    console.log("load data=", data);
    var totals = data.totals || [];
    $.each(totals, function(k, v) {
      console.log("total k=" + k + " v=" + v);
      if(v) {
        v = parseFloat(v);
        v = v.toFixed(2);
        var $target = $("TH[data-total-field='" + k + "']");
        if(v && $target.length) {
          $target.html(v);
        }
      }
    });
    $.each(totals, function(k, v) {
      
    });
  });
});
*/

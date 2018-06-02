/* begin CMS */
console.log("Loaded afr CMS v001");
var uploader = 0;
var g_dir_array = ['top', 'right', 'bottom', 'left'];
var g_css_changed = false;   
var g_divs_onscreen = new Array;
var g_div_edited = '';
var g_selector = 0;
var $g_target = 0;
var g_dir_selectors = new Array;

// note: not spaces in tab - used for ID
var g_all_css_selectors = [
 {'tab': 'Dimensions', 'label': 'Width',       'name': 'width',                 'type': 'slider', 'max': 1000, 'unit': 'px'},
 {'tab': 'Dimensions', 'label': 'Height',      'name': 'height',                'type': 'slider', 'max': 1000, 'orientation': 'horizontal', 'unit': 'px'},
 {'tab': 'Dimensions', 'label': 'Margin',      'name': 'margin',                'type': 'slider', 'min': -50, 'max': 50, 'unit': 'px', 'directional' : true},
 {'tab': 'Dimensions', 'label': 'Padding',     'name': 'padding',               'type': 'slider', 'max': 50, 'unit': 'px', 'directional' : true},
                                               
 {'tab': 'Position',   'label': 'Position',    'name': 'position',              'type': 'select', 'values': ['', 'absolute', 'relative', 'fixed', 'static', 'inherit']},
 {'tab': 'Position',   'label': 'Left',        'name': 'left',                  'type': 'slider', 'max': 500, 'min': -500, 'unit': 'px'},
 {'tab': 'Position',   'label': 'Top',         'name': 'top',                   'type': 'slider', 'max': 500, 'min': -500, 'orientation': 'horizontal', 'unit': 'px'},
 {'tab': 'Position',   'label': 'Bottom',      'name': 'bottom',                'type': 'slider', 'max': 500, 'min': -500, 'orientation': 'horizontal', 'unit': 'px'},
 {'tab': 'Position',   'label': 'Right',       'name': 'right',                 'type': 'slider', 'max': 500, 'min': -500, 'orientation': 'horizontal', 'unit': 'px'},
 {'tab': 'Position',   'label': 'Overflow',    'name': 'overflow',              'type': 'select', 'values': ['', 'visible', 'hidden', 'scroll', 'auto', 'inherit']},
 {'tab': 'Position',   'label': 'Clear',       'name': 'clear',                 'type': 'select', 'values': ['', 'left', 'right', 'both', 'none', 'inherit']},
 {'tab': 'Position',   'label': 'Visibility',  'name': 'visibility',            'type': 'select', 'values': ['', 'visible', 'hidden', 'collapse', 'inherit']},
 {'tab': 'Position',   'label': 'Cursor',      'name': 'cursor',                'type': 'select', 'values': ['', 'auto', 'default', 'crosshair', 'help', 'move', 'pointer', 'progress', 'text', 'wait', 'e-resize', 'n-resize', 's-resize', 'e-resize', 'w-resize', 'ne-resize', 'nw-resize', 'se-resize', 'sw-resize', 'inherit' ]},
 {'tab': 'Position',   'label': 'Display',     'name': 'display',               'type': 'select', 'values': ['', 'none', 'block', 'inline', 'inline-block', 'inline-table', 'list-item', 'table', 'table-caption','table-cell','table-column','table-column-group','table-footer-group','table-header-group','table-row', 'table-row-group','inherit']},
 {'tab': 'Position',   'label': 'Z-Index',     'name': 'z-index',               'type': 'slider', 'max': 1000, 'min': -1000, 'unit': ''},
                                               
 {'tab': 'Border',     'label': 'Color',       'name': 'border-color',          'type': 'color', 'directional' : true, 'size': 6},
 {'tab': 'Border',     'label': 'Style',       'name': 'border-style',          'type': 'select', 'directional' : true, 'values': ['', 'none', 'solid', 'dotted', 'dashed', 'double', 'groove', 'ridge', 'inset', 'outset'], 'size': 6},
 {'tab': 'Border',     'label': 'Width',       'name': 'border-width',          'type': 'slider', 'max': 20, 'unit': 'px', 'directional' : true, 'size': 6},

 {'tab': 'Outline',    'label': 'Color',       'name': 'outline-color',          'type': 'color', 'directional' : false, 'size': 6},
 {'tab': 'Outline',    'label': 'Style',       'name': 'outline-style',          'type': 'select', 'directional' : false, 'values': ['', 'none', 'solid', 'dotted', 'dashed', 'double', 'groove', 'ridge', 'inset', 'outset'], 'size': 6},
 {'tab': 'Outline',    'label': 'Width',       'name': 'outline-width',          'type': 'slider', 'max': 20, 'unit': 'px', 'directional' : false, 'size': 6},
                                               
 {'tab': 'Background', 'label': 'Color',       'name': 'background-color',      'type': 'color', 'size': 6},
 {'tab': 'Background', 'label': 'Repeat',      'name': 'background-repeat',     'type': 'select', 'values': ['', 'no-repeat', 'repeat', 'repeat-x', 'repeat-y', 'inherit']},
 {'tab': 'Background', 'label': 'Attachment',  'name': 'background-attachment', 'type': 'select', 'values': ['','scroll','fixed','inherit']},
 {'tab': 'Background', 'label': 'Position',    'name': 'bg-position-select',    'type': 'select', 'altfield': 'background-position', 'keyvalues': ['', '', '50% 50%', 'center', '0% 50%', 'left', '100% 50%', 'right', '0% 0%', 'top left', '50% 0%', 'top', '100% 0%', 'top right', '0% 100%', 'bottom left', '50% 100%', 'bottom', '100% 100%', 'bottom right']},
 {'tab': 'Background', 'label': 'Image',       'name': 'background-image',      'type': 'upload'},

 {'tab': 'Text',       'label': 'Color',          'name': 'color',                 'type': 'color', 'size': 6},
 {'tab': 'Text',       'label': 'Decoration',     'name': 'text-decoration',       'type': 'select', 'values': ['', 'none', 'underline', 'overline', 'line-through', 'blink', 'inherit']},
 {'tab': 'Text',       'label': 'Weight',         'name': 'font-weight',           'type': 'select', 'values': ['', 'normal', 'bold', 'bolder', 'lighter', '100', '200', '300', '400', '500', '600', '700' , '800' ,'900']},
 {'tab': 'Text',       'label': 'Style',          'name': 'font-style',            'type': 'select', 'values': ['', 'normal', 'italic', 'oblique']},
 {'tab': 'Text',       'label': 'Font',           'name': 'font-family',           'type': 'select', 'keyvalues': ['Verdana, Geneva, sans-serif', 'Verdana', '"Trebuchet MS", Helvetica, sans-serif', 'Trebucht', '"Times New Roman", Times, serif', 'Times New Roman', 'Tahoma, Geneva, sans-serif', 'Tahoma', '"Palatino Linotype", "Book Antiqua", Palatino, serif', 'Palatino', '"Lucida Console", Monaco, monospace', 'Lucida Console', '"Lucida Sans Unicode", "Lucida Grande", sans-serif', 'Lucida', 'Impact, Charcoal, sans-serif', 'Impact', 'Georgia, serif', 'Georgia', '"Courier New", Courier, monospace', 'Courier New', '"Comic Sans MS", cursive, sans-serif', 'Comic Sans MS', 'Arial Black, Gadget, sans-serif', 'Arial Black', 'Arial, Helvetica, sans-serif', 'Arial']},
 {'tab': 'Text',       'label': 'Size',           'name': 'font-size',             'type': 'slider', 'max': 50, 'unit': 'px'},
 {'tab': 'Text',       'label': 'Line Height',    'name': 'line-height',           'type': 'slider', 'max': 50, 'unit': '', 'step': 0.1},
 {'tab': 'Text',       'label': 'Letter Spacing', 'name': 'letter-spacing',        'type': 'slider', 'max': 50, 'unit': 'px', 'step': 0.1},
 {'tab': 'Text',       'label': 'Word Spacing',   'name': 'word-spacing',          'type': 'slider', 'max': 50, 'unit': 'px', 'step': 0.1},
 {'tab': 'Text',       'label': 'Indent',         'name': 'text-indent',           'type': 'slider', 'max': 50, 'unit': 'px', 'step': 0.1},                                                                                                            
 {'tab': 'Text',       'label': 'H Align',        'name': 'text-align',            'type': 'select', 'values': ['', 'left', 'right', 'center', 'justify', 'inherit']},
 {'tab': 'Text',       'label': 'V Align',        'name': 'vertical-align',        'type': 'select', 'values': ['', 'baseline', 'top', 'middle', 'bottom', 'sub', 'super', 'text-top', 'text-bottom', 'inherit']},
 {'tab': 'Text',       'label': 'Transform',      'name': 'text-transform',        'type': 'select', 'values': ['', 'none', 'capitalize', 'uppercase', 'lowercase', 'inherit']},
 {'tab': 'Text',       'label': 'White space',    'name': 'white-space',           'type': 'select', 'values': ['', 'normal', 'nowrap', 'pre', 'pre-line', 'pre-wrap', 'inherit']},
 {'tab': 'Text',       'label': 'Direction',      'name': 'direction',             'type': 'select', 'keyvalues': ['', '', 'ltr', 'Left-to-right', 'rtl', 'Right-to-left', 'inherit', 'inherit']},

 {'tab': 'List',       'label': 'Image',       'name': 'list-style-image',      'type': 'upload'},
 {'tab': 'List',       'label': 'Position',    'name': 'list-style-position',   'type': 'select', 'values': ['', 'inside', 'outside', 'inherit']},
 {'tab': 'List',       'label': 'Style',       'name': 'list-style-type',       'type': 'select', 'values': ['', 'none', 'circle', 'disc', 'square', 'armenian', 'decimal', 'decimal-leading-zero', 'georgian', 'lower-alpha', 'lower-greek', 'lower-latin', 'lower-roman', 'upper-alpha', 'upper-latin', 'upper-roman', 'inherit']},  

 {'tab': 'Opacity',    'label': 'Opacity',        'name': 'opacity',               'type': 'slider', 'max': 1, 'step': 0.1, 'unit': ''}
 
]; // [{"id":"12","unique":"1","name":"layout-title","description":null}, {"id":"12","unique":"1","name":"layout-title","description":null},]


function toggle_edit() {
  var editing = $('body').data('editing');
  //console.log("Toggle edit");
  if(editing) { // turn off edit
    $('body').data('editing', 0);
    $("#site_edit_panel").hide();    
    $(".__edit_link").hide();    
    $(".__edit, .__add").removeClass("_selected");    
    //console.log("edit now off: data=" + $('body').data('editing'));
  } else { // turn on edit
    $('body').data('editing', 1);
    $("#site_edit_panel").removeClass("hidden").show();    
    $(".__edit_link").show();    
    // $('html, body').animate({scrollTop:0}, 'fast');
    
    $(".__edit, .__add").addClass("_selected");    
    //console.log("edit now on: data=" + $('body').data('editing'));
  }
  /* old code */
  /**
  $("#site_edit_button").toggle();
  $("#page_edit_panel").toggle();
  $("#page_edit_panel_sh_icon").toggleClass("ui-icon-plusthick");
  $("#page_edit_panel_sh_icon").toggleClass("ui-icon-minusthick");
  $(".ie_edit_link, .ie_edit_text").toggle();
  $(".__edit").toggleClass("__edit_selected");
  $(".__add").toggleClass("__add_selected");
  $(".__edit").css("border", "2px solid red");
  $(".__edit, .__add").addClass("_selected");
  */
}

hotkeyDown = false;
function hotkey_edit() {
  alert("hk edit");
  var ctrlKey = 17, shiftKey = 16, f2Key=113, vKey = 86, cKey = 67;
  $(document).keydown(function(e) {
    //alert(e.keyCode);
    //console.log("Keydown on " + e.keyCode);
    if (e.keyCode == ctrlKey || e.keyCode == f2Key) {
      console.log("Hotkey edit");
      toggle_edit();
    }
  }).keyup(function(e) {
  });
}

function get_selectors(selectors) {
  var css_selectors = [];
  $.each(g_all_css_selectors, function(i, selector) {
    if($.inArray(selector.name, selectors) > -1) css_selectors.push(selector); 
  });
  return css_selectors;
}

// creates the layout form for interactive css editing
// parent_id is the id of the DIV where inserted
// data should be an object with parent_type, parent_id, site_id, user_id
function layout_create(parent_id, data, options) {
  if(!data) {
    alert("layout_create " + parent_id + " : no data");
    return '';
  }
  var form_id = parent_id + '_layout_form';
  var form = ' \
  <div class="error" id="' + form_id + '_errors"></div> \
  <div class="message" id="' + form_id + '_messages"></div> \
  <form name="layout_form" id="' + form_id + '" action="" method="post"> \
  <input name="site_id"     type="hidden" value="' + data.site_id + '"> \
  <input name="user_id"     type="hidden" value="' + data.user_id + '"> \
  <input name="page_id"     type="hidden" value="' + data.page_id + '"> \
  <input name="parent_id"   type="hidden" value="' + data.parent_id + '"> \
  <input name="parent_type" type="hidden" value="' + data.parent_type + '"> \
  </form> \
  '

  var $parent = $('#' + parent_id);
  //$('#' + form_id, '#' + parent_id + ' .error', '#' + parent_id + ' .message').remove(); // destroy old
  $parent.html(form);
  console.log("Create layout form for  " + parent_id + ' with data ' + data);
  return form_id;
}
  
// creates form + accordion 
function layout_init(parent_id, target_divs, selectorAr) {
  //var db_divs = eval(db_divs_json);      
  //alert(dump(db_divs));

  if(selectorAr) { // show subset
    var css_selectors = get_selectors(selectorAr);
    //alert("sar=" + dump(selectorAr));
  } else { // show all
    var css_selectors = g_all_css_selectors;
  }
  g_css_selectors = css_selectors; // save to global
  // add controls

  console.log("Layout init for " + parent_id + ' with ' + target_divs.length + " divs and " + css_selectors.length + ' selectors');
  
  create_controls(parent_id, target_divs);
  create_accordion(parent_id, css_selectors);  
}

// populates the layout selector with divs that actually exist on page
function create_controls(parent_id, divs) {
  var $parent = $('#' + parent_id);
  $('#' + parent_id + ' .css_controls').remove(); // destroy old
  $parent.append("<div class='css_controls' data-parent_id='" + parent_id + "'></div>");
  console.log("Create controls for  " + parent_id + ' with ' + divs.length + " divs");
  
  var $control_div = $('#' + parent_id + " .css_controls");  
  var sel = '<select name="selector" class="css_select" class="filter">' + "\n" +
            '  <option value="0">Select Element...</option>';  
  //alert(dump(divs));            
  for( var i = 0; i < divs.length; i++ ) {
    var divObj = divs[i];
    var selector = divObj.name || divObj; 
    var title = divObj.title || selector;
    //console.log(divObj);
    var data_string = divObj.selectors ? "data-selectors='" + JSON.stringify(divObj.selectors) + "' " : '';
    //console.log('ds=' + data_string);

    var remote = $(window.opener).length == 1;
    var $target = $g_target = remote ? window.opener.$(selector) : $(selector);

    if($target.length) {
      sel += '<option ' + data_string + 'onmouseover="layout_hover(\'' + parent_id + '\', \'' + selector + '\'); " value="'+ selector + '">' + title +'</option>';
    } else {      
      //alert("couldn't find " + selector + " length=" + $(selector).length) + " title=" + title;
    }
  }
  sel += "</select>\n";

  // add save / cancel buttons + saving span
  sel += '<input name="css_save" type="submit" value="Save" class="css_save" > \
          <input name="css_cancel" type="submit" value="Cancel" class="css_cancel"> \
          <span class="spinning_wheel_small css_saving visuallyhidden">Saving...</span>';

  $control_div.html(sel);

  layout_show_save(parent_id, false);
  
  console.log("Create controls save length = " + $("#" + parent_id + " .css_save").length);
  
}

function create_accordion(parent_id, css_selectors) {
  var $parent = $('#' + parent_id);
  console.log("Create accordion for " + parent_id + ' with ' + css_selectors.length + ' selectors');

  // create accordion                                             
  var accordion = '';
  var accordionAr = new Array;
  for( var i = 0; i < css_selectors.length; i++ ) {
    var css_selector = css_selectors[i];    
    var selector_html = layout_selector(parent_id, css_selector);
    accordionAr[css_selector.tab] = accordionAr[css_selector.tab] ? accordionAr[css_selector.tab] + selector_html : selector_html;
  }
  console.log("Created accordion for " + parent_id + ' with ' + css_selectors.length + ' selectors');
          
  for(var tab in  accordionAr) accordion += "<h3 id='" + parent_id + "_" + tab + "'><a href='#'>" + tab + "</a></h3><div>" + accordionAr[tab] + "</div>";

  //alert(accordion);  
  $('#' + parent_id + ' .layout-accordion').remove(); // destroy old
  $parent.append("<div class='layout-accordion visuallyhidden'>" + accordion + "</div>");
  console.log("Begin init");
  layout_selectors_init(parent_id, css_selectors); // binds 
  layout_selectors_init(parent_id, g_dir_selectors); // binds 
  console.log("End  init");
}

// $css_form['dimensions'] .= html_div(html_label('Width:', 'width').form_input('width', 'text', '', 6).html_div('','slider','id="width-slider"'));
  //                                               
  // $css_form['position'] .= html_div(html_label('Position:', 'position').select_from_array('position', $position_array));
  // $css_form['border'] .= html_div(form_input('border-color', 'text', '', 5, 0, '', 'colorpicker_input'));
  // $css_form['background'] .= html_div(form_input('background-image', 'text', '', 20, 0, '', '').html_div('', '', 'id="background-image-uploader"'));  

// returns a select box with given name and values
// if associate is set, 1st value is key, 2nd value is value etc
function html_select_box(name, values, associative, selected, id) {
  if(!id) id = name;
  var options = '';
  if(associative) {
    for( var i = 0; i < values.length; i+=2 ) {
      var key = values[i];
      var value = values[i+1];
      var selected_str = selected == key ? " selected='selected'" : '';   
      options += "<option value='" + key + "'" + selected_str + ">" + value + "</option>\n";
    }
  } else {
    for( var i = 0; i < values.length; i++ ) {
      var value = values[i];  
      var selected_str = selected == value ? " selected='selected'" : '';   
      options += "<option value='" + value + "'" + selected_str + ">" + value + "</option>\n";
    }
  }
  return "<select name='" + name + "' id='" + id + "'>" + options + "</select>";
}
  
// creates the HTML placeholder code for selectors/widgets (sliders, colorpicker, select boxes etc)
function layout_selector(parent_id, css_selector) {
  var name = css_selector.name;
  var id =  parent_id + '_' + name;
  var output = "<div><label for='" + name + "'>" + css_selector.label + ":</label>";
  var cb = css_selector.directional ? " <input name='" + name + "-directional' id='" + id + "-directional'  type='checkbox'   /> " + css_selector.label + " per side" : "";
  switch(css_selector.type) {     
    case 'color':
      output += "<input name='" + name + "' id='" + id + "' type='text' value='' size='5' class='colorpicker_input' />" + cb;
      break;
    case 'slider':     
      output += "<input name='" + name + "' id='" + id + "' type='text' value='' size='6'/>" + cb + "<div class='slider' id='" + id + "-slider' ></div>";
      break;
    case 'select':
      if(values = css_selector.values) {
        output += html_select_box(name, css_selector.values, false, '', id) + cb;
      } else if(values = css_selector.keyvalues) {
        output += html_select_box(name, css_selector.keyvalues, true, '', id) + cb;
      }
      break;
    case 'upload':
      var g_target = 'layout-left';
      // create_file_uploader(id, target, attr, allowed_extensions, size_limit, allow_multiple); 
      //<div class='fine-uploader' id='" + id + "-uploader' > \
      //Fineuploader</div>";
      
      var uploader_id = id + '-uploader';
      output += "<input name='" + name + "' id='" + id + "' type='text' value='' size='20' />";
      output += '<div id="' + uploader_id + '" data-options=\'{"target_field": "' + id + '","allow_multiple":true,"allowed_types":["image"],"allowed_extensions":["gif","jpe","jpeg","jpg","png"],"handler":"","title":"Click or drag to upload image","minimum_file_size":"1K","minimum_size":100,"maximum_file_size":"10M"}\' class=\'fine-uploader\'></div>';
      //output += '<div><img title=\'&nbsp;\' data-tip=\'Allowed types: image&lt;br&gt;Allowed extensions: gif,jpe,jpeg,jpg,png;Minimum file size: 1K&lt;br&gt;Maximum file size: 10M&lt;br&gt;Minimum size: 100px&lt;br&gt;Allow multiple: Yes\' class=\'tooltip pad clear_float\' height=\'24\' width=\'24\' src=\'/images/icons/tooltip_tiny.png\' alt=\'tooltip\'></div>';
      output += "<script type='text/javascript'> init_uploader($('#" + uploader_id + "'));</script>";                                   
      output += "<div class='qq-upload-button browse-button' onclick='CKFinder_BrowseServer(\"" + id + "\");'>Browse the server</div>";
      break;
    default:
      break;
  }
    
  if(af = css_selector.altfield) output += "<input name='" + af + "' id='" + parent_id + '_' + af + "' type='text' value='' size='10'/>";
  if(css_selector.directional) {  
    var dir_selectors = '';
    var css_selectors = new Array(css_selector, css_selector, css_selector, css_selector);
    css_selectors = eval(array2json(css_selectors)); // clone                       
    for(var i = 0; i < g_dir_array.length; i++) {
      var dir = g_dir_array[i];    
      css_selectors[i].parent = css_selector.name;
      css_selectors[i].label = ucfirst(dir) + " " + css_selector.label;
      css_selectors[i].values = css_selector.values; 
      css_selectors[i].keyvalues = css_selector.keyvalues;
      css_selectors[i].name = layout_directional_id(css_selector.name, dir);
      css_selectors[i].directional = false;
      dir_selectors += layout_selector(parent_id, css_selectors[i]);
      
    }
    g_dir_selectors = g_dir_selectors.concat(css_selectors); 
    output += '<div id="' + id + '-dir_controls">' + dir_selectors  + '</div>'
    //layout_selectors_init(css_selectors);
    //return;
  }
  output += "</div>";
  return output;
}  

//function layout_dir_selector(parent

// create custom selectors/widgets (sliders, colorpicker, select boxes etc)
// bind change/hover functions to input elements
function layout_selectors_init(parent_id, css_selectors) {           
  for( var i = 0; i < css_selectors.length; i++ ) {
    css_selector = css_selectors[i];  
    var input_id = parent_id + '_' + css_selector.name;
    var $input = $("#" + input_id);
    $input.data('css_selector', css_selector); // store selector with input
    if(css_selector.directional) {
      //var input_id = css_selector.name;
      //var unit = css_selector.unit;
      var $cb = $input.parent().find(':checkbox');
      $cb.data('css_selector', css_selector);
      $cb.on('change', function () {layout_direction_update(parent_id, $(this).data('css_selector'), true);});
      layout_direction_update(parent_id, css_selector, true);
    }             
                                                                                                             
    switch(css_selector.type) {
      case 'color':
        layout_color_picker(parent_id, css_selector);
        break;
      case 'slider':                            
        layout_slider(parent_id, css_selector); 
        break;
      case 'select':
        layout_select_box(parent_id, css_selector);
        break;
      case 'upload':
        // create_file_uploader(id, target, attr, allowed_extensions, size_limit, allow_multiple); 
        //uploader = create_file_uploader('background-image', '', 'background-image', ['jpg', 'gif', 'png', 'jpeg'], 0, false, 'layout_changed(parent_id);'); 
        break;
      default:
        break;
    }
    //if(css_selector.directional) layout_directional_inputs(css_selector);
  }
}  

function layout_select_page(parent_id, page_id, link) { 
  var remote = $(window.opener).length == 1;
  if(remote) window.opener.location = link;
  else window.location = link;
  $('#' + parent_id + ' .css_select').val(0);
  layout_select_div(parent_id, 0);
}

function layout_select_div(parent_id, selector) {
  g_selector = selector;

  console.log("layout_select_div pid=" + parent_id + " selector=" + selector);
  
  //alert(selector);
  var $accordion = $('#' + parent_id + ' .layout-accordion');
  if(!$accordion.length) {
  //$accordion.css('left', '0px'); // show the accordion
    alert("Couldn't find accordion: length of " + '#' + parent_id + ' .layout-accordion = ' + $accordion.length);
  }

  // destroy resizable/draggable 
  if(g_div_edited && $(g_div_edited).is('.ui-resizable')) $(g_div_edited).resizable('destroy');
  if(g_div_edited && $(g_div_edited).is('.ui-draggable')) $(g_div_edited).draggable('destroy');
  
  
  if(selector && selector != 0) {
    //$accordion.css('left', '0px');
    $accordion.removeClass('visuallyhidden');
    $accordion.accordion({
      heightStyle: "content",
      collapsible: true
    });      
  } else {
    $g_target = 0;
    //$accordion.css('left', '-1000px');
    $accordion.addClass('visuallyhidden');
    return;
  }         
  g_div_edited = selector;


  var remote = $(window.opener).length == 1;
  var $target = $g_target = remote ? window.opener.$(selector) : $(selector);

  console.log("layout_select_div pid=" + parent_id + " selector=" + selector + " start save");
  layout_save_original(parent_id, $target, g_css_selectors);
  console.log("layout_select_div pid=" + parent_id + " selector=" + selector + " end save ");

  var index = -1;
  //alert("target=" + $target.prop('id'));
  //layout_reset();
  console.log("layout_select_div pid=" + parent_id + " selector=" + selector + " start resizable");

  $target.resizable({
     resize: function(event, ui) {
       var w = ui.size.width;
       var h = ui.size.height;
       var unit = 'px'; // todo: allow others?
       $('#' + parent_id + '_width').val(w + unit);
       $('#' + parent_id + '_height').val(h + unit);
       $('#' + parent_id + '_width-slider').slider('value', w);
       $('#' + parent_id + '_height-slider').slider('value', h);
       console.log("resize change");
       layout_changed(parent_id);
       var tabId = '#' + parent_id + "_Dimensions";
       var tabItem = $(tabId);
       var index = $accordion.index(tabItem);
       //console.log('Index of ' + tabId + '=' + index);
       if(index > -1) {
         $accordion.accordion("option", "active", index);
       }
     },
     stop: function(event, ui) {
     }
  }); 

  console.log("layout_select_div pid=" + parent_id + " selector=" + selector + " start draggable");
  
  $target.draggable({
     drag: function(event, ui) {
       var top = ui.position.top;
       var left = ui.position.left;
       var unit = 'px'; // todo: allow others?
       $('#' + parent_id + '_top').val(top + unit);
       $('#' + parent_id + '_left').val(left + unit);
       $('#' + parent_id + '_top-slider').slider('value', top);
       $('#' + parent_id + '_left-slider').slider('value', left);
       console.log("drag change");
       layout_changed(parent_id);
       
       var tabId = '#' + parent_id + "_Position";
       var tabItem = $(tabId);
       var index = $accordion.index(tabItem);
       //console.log('Index of ' + tabId + '=' + index);
       if(index > -1) {
         $accordion.accordion("option", "active", index);
       }
       //$accordion.accordion("activate", $("#h3-Position"));
     },
     stop: function(event, ui) {
     }
  });
  console.log("layout_select_div pid=" + parent_id + " selector=" + selector + " start hover");
  layout_hover(parent_id, selector);
  console.log("layout_select_div pid=" + parent_id + " selector=" + selector + " end");
}

// sets value and bg-color of fld to hex , fg-color to black or white depending on brightness - used by color_picker
function set_color(fld, hex) {
  var bg = hex ? hex : 'FFFFFF'; 
  if(!hex) hex = '';
  var $el = $('#' + fld);
  $el.val(hex);
  var brightness = hex ? color_brightness(hex) : 1;
  var fg = brightness < 0.5 ? '#FFFFFF' : '#000000';
  $el.css('color', fg);
  $el.css('background-color', '#' + bg);
}

function layout_reset(parent_id) {
  var $selected = $("#" + parent_id + " .css_select option:selected"); 
  var val  = $selected.val();
  var remote = $(window.opener).length == 1;
  var $target = remote ? window.opener.$(val) : $(val);

  g_css_changed = false;
  console.log("layout_reset for " + parent_id + " div=" + val);
  // alert('selected=' + val);
  // restore original values on cancel
  for( var i = 0; i < g_css_selectors.length; i++ ) {
    var css_selector = g_css_selectors[i];    
    var css_property = css_selector.target ? css_selector.target : css_selector.name;
  
    var org_val = $('#' + parent_id + '_' + css_selector.name).data('orgval');
    var input_id = parent_id + '_' + css_selector.name;
    
    console.log("orgval of " + css_selector.name + " = " + org_val);
    switch(css_selector.type) {
      case 'color':
        if(!org_val) org_val = '';
        // alert("setting color of " + css_selector.name + " to " + org_val);
        $target.css(css_selector.name, '#' + org_val);
        set_color(input_id, org_val);
        break;
      case 'slider':
        $('#' + input_id).val(org_val);
        $('#' + input_id + '-slider').slider('value', parseInt(org_val));
        $target.css(css_selector.name, org_val);
        break;
      case 'select':
        $('#' + input_id).val(org_val);
        if(css_selector.target) {
          //alert("resetting " + css_selector.target + " to " + org_val);
          $('#' + css_selector.target).val(org_val);
        }
        $target.css(css_property, org_val);
        break;
      default:
        $('#' + input_id).val(org_val);
        $target.css(css_property, org_val);
        break;
    }
    if(css_selector.directional) {
      var $dir_input = $('#'  + parent_id + '_' + css_selector.name + '-directional');
      $dir_input.prop('checked', $dir_input.data('orgval'));
      layout_direction_update(parent_id, css_selector, false);
    }
  }
  $("#page_id").prop('disabled', false);

  layout_show_save(parent_id, false);
  
  //alert("enabled select disabled=" + $("#div_select").prop('disabled') + " id=" + $("#div_select").prop('id'));
}

function layout_show_save(parent_id, show) {
  console.log("Show save/cancel = " + show);
  if(show) {
    $("#" + parent_id + " .css_page_select").hide('fast');
    $("#" + parent_id + " .css_select").hide('fast');
    $("#" + parent_id + " .css_save").show('fast');
    $("#" + parent_id + " .css_cancel").show('fast');
    $("#" + parent_id + " .css_saving").show('fast');  
  } else {
    $("#" + parent_id + " .css_page_select").show('fast');
    $("#" + parent_id + " .css_select").show('fast');
    $("#" + parent_id + " .css_save").hide('fast');
    $("#" + parent_id + " .css_cancel").hide('fast');
    $("#" + parent_id + " .css_saving").hide('fast');  
  }
}

function layout_hover(parent_id, selector) {
  var remote = $(window.opener).length == 1;
  var $target = remote ? window.opener.$(selector) : $(selector);
  for( var i = 0; i < g_css_selectors.length; i++ ) {
    var css_selector = g_css_selectors[i];
    var val = layout_css_read($target, css_selector);
    console.log("selector=" + selector + " css=" + css_selector.name + " val=" + dump(val));
    layout_set_selector(parent_id, css_selector, val);
    if(val && css_selector.directional) {
      val_arr = val.split(' ');
      $('#' + css_selector.name + '-directional').prop('checked', val_arr.length == 4 ? true : false);
      layout_direction_update(parent_id, css_selector, false);
    }
  }
  // $target.effect('highlight', {}, 500);
}

function layout_set_selector(parent_id, css_selector, val) {
  var input_id = parent_id + '_' + css_selector.name;
  var $input = $('#' + input_id);
  switch(css_selector.type) {                          
    case 'color':
      set_color(input_id, val);
      break;
    case 'upload':
      // alert("setting " + css_selector.name + " to " + $target.css(css_selector.name));
      $input.val(val);
      break;
    case 'select':
      $input.val(val);
      if(altfield = css_selector.altfield) {
        // alert("Setting " + altfield + " to " + val);
        //$target.css(altfield, value); // update target css
        $('#' +parent_id + '_' +  altfield).val(val); // write to alt input
      }
      break;
    case 'slider':
      var intVal = parseInt(val);
      var $slider = $('#' + css_selector.name+ '-slider');
      $input.val(val);
      $slider.slider('value', intVal);

      // var max = Math.max(intVal * 2, css_selector.max);
      // var min = Math.min(intVal * 2, css_selector.min);
      // $slider.slider('option', 'max', max);
      // $slider.slider('option', 'min', min);
      // $('#' + css_selector.name+ '-slider').slider('value', val);
      break;
    default:
      $('#' + css_selector.name).val(val);
      break;
  }
}
  
function layout_direction_join(parent_id, css_selector) {
  var input_id = parent_id + '_' + css_selector.name;
  var $parent = $('#' + input_id);
  //var parent_id = $parent.prop('id');
  var val_array = new Array();
  for(var i = 0; i < g_dir_array.length; i++) {
    var dir = g_dir_array[i];    
    var dir_id = layout_directional_id(input_id, dir);
    val_array[i] = $('#' + dir_id).val();
  }
  var value = val_array.join(' ');
  // alert(value);
  $parent.val(value);

  var $target = $parent.data('target');
  //alert("val=" + value);
  return value;
}
                                                                   
function layout_direction_update(parent_id, css_selector, update) {
  var input_id = parent_id + '_' + css_selector.name;
  var unit = css_selector.unit;
  var $input = $('#' + input_id);
  var checked = $('#' + input_id + '-directional').prop('checked');
  var type = $input.prop('type');

  var $parent = css_selector.type == 'slider' ? $('#' + input_id + '-slider') : $input;
  var parent_val = $input.val();
  var val_array = parent_val ? parent_val.split(' ') : [];
  var first_val = val_array.length==4 && val_array[0] ? val_array[0] : '';
  // var css_selector = $input.data('css_selector');
  
  console.log("layout_direction_update pid " + parent_id + ' sel=' + css_selector.name + ' update=' + update); 

  if(checked) {
    if(type == 'text') $input.prop('size', 28);
    if(css_selector.type == 'color') $input.removeClass('colorpicker_input');
    else if(css_selector.type == 'select') {
      $input.replaceWith("<input name='" + $input.prop('id') + "' id='" + $input.prop('id') + "' type='text' size='28' value='" + $input.val() + "'>");
      $input = $('#' + input_id); // reload
    } else if(css_selector.type == 'slider') {
      $('#' + input_id + '-slider').hide();
    }

    $('#' + input_id + '-dir_controls').show();

    for(var i = 0; i < g_dir_array.length; i++) {
      var dir = g_dir_array[i];    
      var dir_id = layout_directional_id(input_id, dir);
      var $dir_input = $('#' + dir_id);
      var css_dir_selector = $dir_input.data('css_selector');
      var dir_val = $dir_input.val();
      if(!dir_val) {
        dir_val = val_array.length==4 ? val_array[i] : parent_val;
        layout_set_selector(parent_id, css_dir_selector, dir_val);
      }
    }       
    layout_direction_join(parent_id, css_dir_selector);
    $input.hide();
        
  } else {
    if(update && first_val) { // user just unchecked the box. Set value to first of four
      parent_val = first_val;
      $input.val(first_val);    
    }

    if(!update) { // hover or reset - unchecked. clear all directional values
      for(var i = 0; i < g_dir_array.length; i++) {
        var dir = g_dir_array[i];    
        var dir_id = layout_directional_id(input_id, dir);
        var $dir_input = $('#' + dir_id);
        $dir_input.val('');
      }       
    }
    
    $input.show();
    $('#' + input_id + '-dir_controls').hide();
    console.log("Hiding dir controls = " + '#' + input_id + '-dir_controls'); 
    if(type == 'text') $input.prop('size', 7);
    if(css_selector.type == 'color') {
      $input.removeClass('colorpicker_input');
    } else if(css_selector.type == 'select' && type == 'text') {
      if(css_selector.values) {
        var selectbox = html_select_box(css_selector.name, css_selector.values, false, parent_val, input_id);
      } else if(css_selector.keyvalues) {
        var selectbox = html_select_box(css_selector.name, css_selector.keyvalues, true, parent_val, input_id);
      }
      $input.replaceWith(selectbox);
      $input = $('#' + input_id); // reload
    } else if(css_selector.type == 'slider') {
      $('#' + input_id + '-slider').show();
    }
  }
  if($g_target && update) layout_css_write(parent_id, $g_target, css_selector, $input.val());
}
   
function layout_directional_id(input_id, dir) {
  var id_arr = input_id.split('-');
  return id_arr.length > 1 ? id_arr[0] + '-' + dir + '-' + id_arr[1] : input_id + '-' + dir;
}

function layout_css_write(parent_id, $target, css_selector, value) {
  if(css_selector.type == 'color') { // prepend # to colors
    var val_array = value.split(' ');
    if(value.length==6) {
      value = '#' + value;
    } else if(value.length ==27 && val_array.length == 4) {
      var old_len = value.length;
      var new_vals = [];
      for(i=0;i<4;i++) new_vals[i] = '#' + val_array[i];
      value = new_vals.join(' ');
    }
  }
  if(css_selector.tab == 'List') {
   var selector = g_selector + ' UL, ' + g_selector + ' OL';
   var remote = $(window.opener).length == 1;
   $target = remote ? window.opener.$(selector) : $(selector);
  }
  
  if(altfield = css_selector.altfield) {
    $target.css(altfield, value); // update target css
    $('#' +altfield).val(value); // write to alt input
  } else {
    //alert("Setting " + css_selector.name + ' of ' + $target.prop('id') + " to " + value);
    $target.css(css_selector.name, value);
  }
  console.log("css write selector="+css_selector.name + " value=" + value);
  //layout_changed(parent_id);
}

function layout_css_read($target, css_selector) {
  //var remote = $(window.opener).length == 1;
  //var $target = remote ? window.opener.$(selector) : $(selector);
  if(!$target) {
    console.log("layout_css_read called with empty selector for empty target " + (css_selector ? " selector=" + css_selector : " selector also empty"));
    return;
  }
  if(!css_selector) {
    //alert(dump($target));
    console.log("layout_css_read called with empty selector for target=" + $target.attr('id'));
    return;
  }
  var css_property = css_selector.altfield ? css_selector.altfield : css_selector.name;
  var val_array = new Array();
  var all_same = true;
  
  if(css_selector.directional) {
    for(var i = 0; i < g_dir_array.length; i++) {
      var dir = g_dir_array[i];    
      var dir_id = layout_directional_id(css_property, dir);
      var $dir_input = $('#' + dir_id);
      var css_dir_selector = $dir_input.data('css_selector');
      val_array[i] = layout_css_read($target, css_dir_selector); // $target.css(css_dir_property);
      //val_array[i] = $target.css(css_dir_property);
      if(i > 0 && val_array[i] != val_array[0]) all_same = false;
    }
    return all_same ? val_array[0] : val_array.join(' ');
    //alert(val_array.join(' '));
  }
    
  // below is a hack since jQuery doesn't recognize "border-width", only "border-top-width" 
  var parts = [];
  if(css_property.indexOf('border-') ==0) {
    parts = css_property.split('-');
    if(parts.length==2) css_property = parts[0] + '-top-' + parts[1];
  } else if(css_property == 'margin' || css_property == 'padding') {
    css_property = css_property + '-top';
  }
  
  var css_val = $target.css(css_property);
  if(css_selector.type == 'color') css_val = rgb2hex(css_val);
  else if(css_selector.type == 'slider' && isNaN(parseFloat(css_val))) css_val = ''; // don't allow non-numbers
  
  return css_val
}


function layout_select_box(parent_id, css_selector) {
  var input_id = parent_id + '_' + css_selector.name;
  var $input = $("#" + input_id);
  $input.on('change', function() {
    //layout_css_write($(this).data('target'), css_selector, this.value);
    var value = this.value;
    layout_css_write(parent_id, $g_target, css_selector, value);
    console.log("select change selector=" + css_selector.name + " value=" + value);
    layout_changed(parent_id);
    if(css_selector.parent) layout_direction_join(parent_id, css_selector);
  }); 
  
  $('#' + input_id + ' option').hover(function() {
    //layout_css_write($(this).parent().data('target'), css_selector, this.value);
    var value = this.value;
    console.log("select hover selector=" + css_selector.name + " value=" + value);
    layout_css_write(parent_id, $g_target, css_selector, value);
  });                
                                                              
  if(altfield = css_selector.altfield) {
    var $altTarget = $('#' + altfield);
    $altTarget.on('change', function() {
        //layout_css_write($('#' + css_selector.name).data('target'), css_selector, this.value);
        var value = this.value;
        layout_css_write(parent_id, $g_target, css_selector, value);
        console.log("select change selector=" + css_selector.name + " value=" + value);
        layout_changed(parent_id);
        if(css_selector.parent) layout_direction_join(parent_id, css_selector);
      }); 
  }
}

function layout_slider(parent_id, css_selector) {
  var min = css_selector.min ? css_selector.min : 0;
  var max = css_selector.max ? css_selector.max : 1000; 
  var orientation = css_selector.orientation ? css_selector.orientation : 'horizontal'; 
  var step = css_selector.step ? css_selector.step : 1;
  var unit = css_selector.unit; // todo: use actual, default to selector unit
  // if(orientation == 'vertical') {
  //   var min0 = min;
  //   min = max;
  //   max = min0;
  //   step = -1;
  // }
  var $input = $("#" + parent_id + '_' + css_selector.name);
  var $slider = $("#" + parent_id + '_' + css_selector.name + '-slider');
  // $slider.slider("destroy");      
  
  // var css_val = $target.css(css_selector);
  // var css_intval = css_val ? parseInt(css_val) : 0;

  //if(css_selector == 'border-width') alert("target=" + target + " bw=" + css_val + " border=" + $target.css('border'));
  $slider.slider({
    min: min,     
    max: max,
    step: step,
//    orientation: orientation,
//    range: "min",      
//     value: 0, // set dynamically somewhere else
//      alert(css_selctor.name + " of val = " + ui.value);
//      var id = $target.prop('id');
//      alert(css_selctor.name + " of target id = " + id + ' val=' + ui.value);

  slide: function(event, ui) {
      //var $target = $input.data('target');
      if(!$g_target) return;
      layout_css_write(parent_id, $g_target, css_selector, ui.value)
      //$g_target.css(css_selector.name, ui.value);
      var value = ui.value + unit;
      $input.val(value);
      if(css_selector.parent) layout_direction_join(parent_id, css_selector);
      // if(css_selector == 'border-top-width') alert("Setting " + css_selector + " of " + $target.prop('id') + ' to ' + ui.value);
    }
//    change: layout_changed
  });
  $input.on('change', function() {
    if(!$g_target) return;
    var value = parseInt(this.value);
    console.log("slide change selector=" + css_selector.name + " value=" + value);
    layout_changed(parent_id);
    //var $target = $input.data('target');
    $slider.slider("value", value);
    layout_css_write(parent_id, $g_target, css_selector, this.value)
    if(css_selector.parent) layout_direction_join(parent_id, css_selector);
    //$g_target.css(css_selector.name, this.value);
  });

}


// turns a text input into a color picker
// if target is set, dynamically update color of target
function layout_color_picker(parent_id, css_selector) {   
   var input_id = parent_id + '_' + css_selector.name;
   var $input = $("#" + input_id);
   $input.ColorPicker({
     onSubmit: function(hsb, hex, rgb, el) {
       $(el).val(hex);
       $(el).ColorPickerHide();
       var brightness = (rgb.r+ rgb.g + rgb.b) / 765;
       var fg = brightness < 0.5 ? '#FFFFFF' : '#000000';
       $(el).css('color', fg);
       $(el).css('background-color', '#' + hex);        
     },
     onBeforeShow: function () {
       $(this).ColorPickerSetColor(this.value);
     }, 
     onChange: function (hsb, hex, rgb, el) {                                
       // $(el).val(hex);
       // $(el).css('background-color', '#' + hex);
       if(!$g_target) return;
       set_color(input_id, hex);
       //var $target = $input.data('target');
       var value = '#' + hex;
       $g_target.css(css_selector.name, value);     
       if(css_selector.parent) layout_direction_join(parent_id, css_selector);
        
       console.log("color change selector=" + css_selector.name + " value=" + value);
       layout_changed(parent_id);     
     }  
   })
   .on('change', function() {
     if(!$g_target) return;
     $(this).ColorPickerSetColor(this.value);
     set_color(input_id, this.value);
     //var $target = $input.data('target');
     $g_target.css(css_selector.name, '#' + this.value);
     if(css_selector.parent) layout_direction_join(parent_id, css_selector);
   });
  //set_color(css_selector, ) // handled by hover
}                                                

function layout_changed(parent_id) {
  // if(g_css_changed) return;
  g_css_changed = true;
  console.log("CHANGE layout of " + parent_id);
  $("#page_id").prop('disabled', true);
  
  layout_show_save(parent_id, true);
}

// store original values for reset and target input
function layout_save_original(parent_id, $target, css_selectors) {
  if(!css_selectors) var css_selectors = g_css_selectors;
  var remote = $(window.opener).length == 1;
  // var $target = remote ? window.opener.$(selector) : $(selector);
  // alert('saving original remote=' + remote + ' selector=' + selector + ' target=' + $target.prop('id') + ' num selectors=' + $(css_selectors).length);
  for( var i = 0; i < css_selectors.length; i++ ) {
    css_selector = css_selectors[i];
    var unit = css_selector.unit;
    var $input = $('#' + parent_id + '_' + css_selector.name);
    var org_val = layout_css_read($target, css_selector);

    //if(css_selector.name == 'width' && $target.prop('id') == 'footer') alert(css_selector.name + ' of ' + $target.prop('id') + ' = ' + org_val);
    console.log(css_selector.name + ' of target' + $target.prop('id') + ' = ' + org_val);
    
    $input.data('orgval', org_val);
    $input.data('target', $target);
    if(css_selector.directional) {
      var $dir_input = $('#' + css_selector.name + '-directional');
      $dir_input.data('orgval', $dir_input.prop('checked') ? true : false);
      //alert('org val of ' + '#' + css_selector.name + '-directional' + ' = ' + $dir_input.prop('checked'));
    }
  }
}

function layout_close() {
  if(g_css_changed && !confirm("You have unsaved changes to the layout. Are you sure you want to close the dialog (your changes will be lost) ?")) return false;  
  if(g_div_edited) {
    $(g_div_edited).resizable('destroy');
    $(g_div_edited).draggable('destroy');
  }
  
  var remote = $(window.opener).length == 1;
  var $target = remote ? window.opener.$("a") : $("a");
  $target.unbind('click'); 
  layout_reset();
  if(remote) window.close();
  return true;
}

function color_brightness(hexcolor) {
  var rgb = color_rgb(hexcolor);
  //alert("rgb for " + hexcolor + " = " + rgb.r + '/' + rgb.g + '/' + rgb.b + ' = ' + (rgb.r+ rgb.g + rgb.b) / 765);
  return (rgb.r+ rgb.g + rgb.b) / 765;
}

// returns decimal RGB values from hex color (FFF or FFFFFF)
function color_rgb(hexcolor) {
  var rgb = new Object;
  if(hexcolor.length == 3) {
    rgb.r = h2d(hexcolor.charAt(0)) ^ 2;
    rgb.g = h2d(hexcolor.charAt(1)) ^ 2;
    rgb.b = h2d(hexcolor.charAt(2)) ^ 2;
  } else if(hexcolor.length == 6) {
    rgb.r = h2d(hexcolor.charAt(0) + '' + hexcolor.charAt(1));
    rgb.g = h2d(hexcolor.charAt(2) + '' + hexcolor.charAt(3));
    rgb.b = h2d(hexcolor.charAt(4) + '' + hexcolor.charAt(5));
    //alert(hexcolor.charAt(0) + '' + hexcolor.charAt(1) + ' => ' + h2d(hexcolor.charAt(0) + '' + hexcolor.charAt(1)));
  }
  
  return rgb;
}

// takes a string (from jquery's $el.css('color') like "rgb(20,40,50) and returns hex value (w/o #)
function rgb2hex(rgb) {
  if (rgb.search("rgb") == -1 ) {
    return rgb;
  } else {
    rgb = rgb.match(/^rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*(\d+))?\)$/);
    function hex(x) {
      return ("0" + parseInt(x).toString(16)).slice(-2);
    }
    //return "#" + hex(rgb[1]) + hex(rgb[2]) + hex(rgb[3]); 
    return hex(rgb[1]) + hex(rgb[2]) + hex(rgb[3]); 
 }
}

// Dynatree functions 
function dynatree_addnode(rootNode, data) {        
  var childNode = rootNode.addChild({
    key: data.id,
    title: data.name,
    isFolder: data.is_folder == 1
  });
  return childNode;
}

function dynatree_removenode(rootNode, title, is_folder) {        
}

function dynatree_totrash(sourceNode) {
  var $trash = $('#trash_tree');               
  var node = $trash.dynatree("getRoot");
  var hitMode = 'over';
  var target_id = '0';

  var source_key = sourceNode.data.key; 
  var source_id = source_key.split('_').pop();
  var key = node.data.key; 
  var id = key.split('_').pop();

  var ajax_url = "/ajax.php?oper=tree_update&obj_type=site_page&ac=tree_update&id=" + source_id + "&target=" + id + "&mode=" + hitMode + "&trash=1";
  alert("source=" + source_id + " target=" + id + " url=" + ajax_url);

  //var ajax_url = "/ajax.php?oper=tree_update&obj_type=site_page&ac=tree_update&id=" + sourceNode.data.key + "&target=" + target_id + "&mode=" + hitMode + "&trash=1";
  // alert(ajax_url);
  //return;
  jQuery.ajax({
    type: "get",
      url: ajax_url,
      success: function (responseText) {
        // alert(responseText);
        logMsg("tree.onDrop(%o, %o, %s)", node, sourceNode, hitMode);
        sourceNode.move(node, hitMode);
        dynatree_update_trash();
    }
  });

}

function dynatree_empty_trash(node) {
  if(!confirm('Are you sure you want to permanently delete the images in the trash?')) return false;
  var ids = new Array();
  var $trash = $('#trash_tree');               
  var node = $trash.dynatree("getRoot");
  var cb = node.toDict(true, function(dict){
    var key = dict.key;
    var id = key.split('_').pop();
    ids.push(id);
  });

  var ids_str = ids.toString();
  $.ajax({ 
    url: "/ajax.php?obj_type=site_page", 
    type: "POST", 
    data: "obj_type=site_page&oper=del&id="+ ids_str,
    success: function(responseText) {
      var $trash = $('#trash_tree');               
      var node = $trash.dynatree("getRoot");
      node.removeChildren();
      dynatree_update_trash();
    }    
  });
  return false;
}

// adds empty icon, hides "empty trash link"
// or removes empty icon, shows "empty trash link" as needed

function dynatree_update_trash() {
  $trash = $('#trash_tree');
  var trash_tree = $trash.dynatree("getTree");
  var trash_root = $trash.dynatree("getRoot");
  var count = trash_tree? trash_tree.count() : 0;
  if(!count || count==1) {
    var node_data = [];
    node_data.name = '[empty]';
    node_data.is_folder = true;
    node_data.id = '__trash';
    dynatree_addnode(trash_root, node_data)
    $('#tree_empty_trash').hide();
  } else if(count >= 2) {
    var empty_node = $trash.dynatree("getTree").getNodeByKey("__trash");
    if(empty_node) empty_node.remove();
    $('#tree_empty_trash').show();
  }  
  return;  
}

// handler for cms save (add page)
// called from cms site when page is successfully saved
function cms_save(options, data) {
  alert(dump(data));
  window.location = '/?goto_page&page_id=' + data.id;
}

// handler for dynatree save
function dynatree_save(options, data) {
  //alert(data_json);
  //var data = parse_json(data_json);
  //alert(dump(data));
  
  var id = parseInt(data.id);
  var parent_id = parseInt(data.parent_id);
  //alert("id=" + id + " pid=" + parent_id);
  
  var $tree = $('#pages_tree');
  var active = $tree.dynatree("getActiveNode");
  var rootNode = active && active.data.title != 'Home' ? active : $tree.dynatree("getRoot");
  if(id) { // update
    var node = $tree.dynatree("getTree").selectKey(id);
    if(node) { 
      node.data.title = data.name;
      node.data.isFolder = data.is_folder == 1;
      node.render();
    } else {
      var childNode = dynatree_addnode(rootNode, data);      
    }
  } else { // add
    alert("dynatree_save: no id, shouldn't get here");
    var childNode = dynatree_addnode(rootNode, data);
  }

  if(parent_id) { // expand parent
    alert(parent_id);    
    var parent_node = $tree.dynatree("getTree").selectKey("tree_" + parent_id);
    parent_node.expand();
  }
  //alert('hello' + dump(data) + " title=" + data.name);
}

function dynatree_init2(data) {
  //alert(dump(data));
  var site_id = data.id;
  if(!site_id) {
    alert("dynatree_init2: missing site_id");
    return;
  }
  
  dynatree_init(site_id, 'trash_tree', 1);
  dynatree_init(site_id, 'pages_tree', 0); 
  $('#tree_empty_trash').click(dynatree_empty_trash);
}
                                              
function dynatree_init(site_id, id, trash) {
  var $tree = $('#' +id);
  $tree.data('site_id', site_id);
  $tree.dynatree({
  onClick: function(node, event) {
      logMsg("tree.onClick(%o, %o)", node, event);
      var active = $tree.dynatree("getActiveNode");
      var tree = $tree.dynatree("getTree");
      if(active && active.data.key == node.data.key) { // deactivate active key if clicked again
        $tree.dynatree("activeKey");
        tree.activateKey("");
        tree.deactivateKey(0);
      }
      //alert("click on " + node.data.key + " active=" + (active ? active.data.key : ""));        
      //if( node.getEventTargetType(event) == "title" ) node.toggleSelect();
    },

  onDblClick: function(node, event) {
      if(node.data.title == '[empty]') return false;
      var key = node.data.key; 
      var id = key.split('_').pop();
      var tree = $tree.dynatree("getTree");
      var title = node.data.title;
      var page_fields = ['site_id', 'parent_id', 'name', 'page_type', 'show_menu', 'show_footer', 'localized', 'is_folder'];
      var data = [];
      object_dialog('site_page', id, {'fields': page_fields, 'data': data, 'handler' :'dynatree_save'});
    },
    
    onKeydown: function(node, event) {
      // alert(event.which);
      if( event.which == 46 ) { // delete        
        if(node.data.title == '[empty]' || node.data.title == 'Home') return false;
        dynatree_totrash(node);
        return false;
      }
    },

    dnd: {
      onDragStart: function(node) {
        //return true;
        if(node.data.title == '[empty]' || node.data.title == 'Home') return false;
        logMsg("tree.onDragStart(%o)", node);
        /** This function MUST be defined to enable dragging for the tree.
         *  Return false to cancel dragging of node.
         */
        return true;
      },
      onDragStop: function(node) {
        // This function is optional.
        logMsg("tree.onDragStop(%o)", node);
      },
      autoExpandMS: 1000,
      preventVoidMoves: true, // Prevent dropping nodes \'before self\', etc.
      onDragEnter: function(node, sourceNode) {
        /** sourceNode may be null for non-dynatree droppables.
         *  Return false to disallow dropping on node. In this case
         *  onDragOver and onDragLeave are not called.
         *  Return \'over\', \'before, or \'after\' to force a hitMode.
         *  Return [\'before\', \'after\'] to restrict available hitModes.
         *  Any other return value will calc the hitMode from the cursor position.
         */
        logMsg("tree.onDragEnter(%o, %o)", node, sourceNode);
        // Prevent dropping a parent below it\'s own child
//                if(node.isDescendantOf(sourceNode))
//                    return false;
        // Prevent dropping a parent below another parent (only sort
        // nodes under the same parent)
//                if(node.parent !== sourceNode.parent)
//                    return false;
//              if(node === sourceNode)
//                  return false;
        // Don\'t allow dropping *over* a node (would create a child)
//        return ["before", "after"];
        return true;
      },
      onDragOver: function(node, sourceNode, hitMode) {
        /** Return false to disallow dropping this node.
         *
         */
        logMsg("tree.onDragOver(%o, %o, %o)", node, sourceNode, hitMode);
        // Prohibit creating childs in non-folders (only sorting allowed)
//        if( !node.isFolder && hitMode == "over" )
//          return "after";
      },
      onDrop: function(node, sourceNode, hitMode, ui, draggable) {
        /** This function MUST be defined to enable dropping of items on
         * the tree.
         */
        
        //alert("tree.onDrop(%o, %o, %s)" + "node=" + node.data.title + " source= " + sourceNode.data.key + " hitMode=" + hitMode + " type=" + typeof(node));
        if(node.data.title == '[empty]' || node.data.title == 'Home') hitMode = 'after';

        var source_key = sourceNode.data.key; 
        var source_id = source_key.split('_').pop();
        var key = node.data.key; 
        var id = key.split('_').pop();
        //alert("source=" + source_id + " target=" + id);
        //return;    
        ajax_url = "/ajax.php?oper=tree_update&obj_type=site_page&ac=tree_update&id=" + source_id + "&target=" + id + "&mode=" + hitMode + "&trash=" + trash;
        jQuery.ajax({
          type: "get",
          url: ajax_url,
          success: function (responseText) {
            logMsg("tree.onDrop(%o, %o, %s)", node, sourceNode, hitMode);
            sourceNode.move(node, hitMode);
            dynatree_update_trash();
          }
        });
        
        if(sourceNode) {
          logMsg("tree.onDrop(%o, %o, %s)", node, sourceNode, hitMode);
          sourceNode.move(node, hitMode);
        } else {           
          alert("no sn");
        }
        // expand the drop target   
//        sourceNode.expand(true);
      },
      onDragLeave: function(node, sourceNode) {
        /** Always called if onDragEnter was called.
         */
        logMsg("tree.onDragLeave(%o, %o)", node, sourceNode);
      }
    }
  });

  if(trash) return;
}
  
  
    //add_page_dialog(site_id, parent_id, is_folder, 'dynatree_save');

    // return false;

    //var title = $("#page_name").val();
    //if(!title) {alert("Please fill in a name");return false;}
    //var page_type = $("#page_type").val();
    //
    //var rootNode = active ? active : $tree.dynatree("getRoot");
    //var ajax_url = "/ajax.php?oper=add&obj_type=site_page&oper=add";
    //alert(ajax_url);
    //jQuery.ajax({
    //  type: "POST",
    //  data: "obj_type=media&oper=edit&site_id=" + site_id + "&parent_id="+ parent_id + "&name=" + title + "&is_folder=" + is_folder + "&page_type=" + page_type,
    //  url: ajax_url,
    //  success: function (responseText) {
    //    //var result = parse_json(responseText);
    //    var childNode = dynatree_addnode(rootNode, title, is_folder);
    //    $("#page_name").val("");
    //
    //    if(active) active.expand(true);
    //    alert('success:' + responseText);
    //  },
    //  error: function(request, error) {
    //    alert("Error: " + error)     
    //  }
    //});

// End Dynatree functions 


/* end CMS */

/* back end code (some CMS related ) */

// DEPRECATED: see init_uploader in afr.js
//function create_file_uploader(input_id, target, attr, allowed_extensions, size_limit, allow_multiple, handler, upload_dir, mediaData, resizeData) {
function create_file_uploader(input_id, options) {
  var target               = options.target; 
  var attr                 = options.attr; 
  var allowed_extensions   = options.allowed_extensions; 
  var size_limit           = options.size_limit; 
  var allow_multiple       = options.allow_multiple; 
  var handler              = options.handler; 
  var upload_dir           = options.upload_dir; 
  var mediaData            = options.mediaData || options.media_data; 
  var resizeData           = options.resizeData;
  
  //console.log("got here, options=");
  //console.log(options);
  // if (typeof upload_dir === 'undefined') {
  //   alert(upload_dir);
  //   upload_dir = '';
  // }
  var uploader = new qq.FileUploader({

    // container element DOM node (ex. $(selector)[0] for jQuery users)
    element: $('#' + input_id + '-uploader')[0],
    //element: document.getElementById(id + '-uploader'),
            
    debug: true,
    action: '/ajax.php',
    params: {
        param1: upload_dir,
        param2: mediaData
    },
    button: null,
    multiple: true,
    maxConnections: 3,
    allowedExtensions: allowed_extensions,        
    sizeLimit: size_limit,
    minSizeLimit: 0,                             
    onSubmit: function(id, fileName){},
    onProgress: function(id, fileName, loaded, total){},
    onCancel: function(id, fileName){},
    onComplete: function(id, fileName, responseJSON) {
      // var allow_multiple = ".($allow_multiple ? 'true' : 'false').";
      // alert(dump(responseJSON));
      var error = responseJSON.error;
      var success = responseJSON.success == 1;
      var path = responseJSON.path;
      var src = path && fileName ? path + fileName : '';      
      if(success && src) {
        var $input = $('#' + input_id);
        var val = $input.val();
        var target_val = attr == 'background-image' ? "url('" + src + "')" : src; 
        var id = responseJSON.id;
        //var $target = $input.data('target');
        var $target = $g_target;
        if(!$target.length && target) $target = $(target);  
        if($target.length && attr) $target.css(attr, target_val);
          //alert("input_id=" + input_id + " orgval =" + $input.data('orgval') + " target=" + target + " $target id=" + $target.prop('id')); // target = " + $('#' + input_id).data('target'));
          //var $target = target ? $(target) : $('#' + input_id).data('target');
          // var camel_attr = attr.toCamel;
          // camel_attr = "backgroundImage";
          // alert("camel of " + attr + " = " + camel_attr);
          // $target.prop('backgroundColor', '');
          // alert('Upload completed. id= ' + input_id + ' ' + camel_attr + ' of ' + target + ' = ' + $(target).prop(camel_attr) + ' path=' + path + ' src=' + src);
          // alert($target.prop('id') + ' = ' + $target.prop('backgroundImage')); 
        if(handler) {  // function name
          var funcCall = handler  + "(responseJSON);";
          eval(funcCall);
        }

        // if(onComplete) eval(onComplete); // code
        //allow_mulitiple = parseInt(allow_multiple);
        //alert('am=' + allow_multiple);
        if(val && allow_multiple==1) { // append
          // alert(allow_multiple);
          $input.val(val + ',' + target_val);
        } else { // overwrite
          $input.val(target_val);
        }
        // if(error) alert("Upload completed, but with the following error: " + error);
      } else {
        // if(error) alert("Upload failed with following error: " + error);
      }
      // alert('Upload completed. id=' + id + ' filename=' + fileName + ' to ' + path + ' success=' + success + ' error=' + error + ' target=' + target + ' attr=' + attr);
    },
    messages: {
      typeError: '{file} has invalid extension. Only {extensions} are allowed.',
      sizeError: '{file} is too large, maximum file size is {sizeLimit}.',
      minSizeError: '{file} is too small, minimum file size is {minSizeLimit}.',
      emptyError: '{file} is empty, please select files again without it.',
      onLeave: 'The files are being uploaded, if you leave now the upload will be cancelled.'            
    },
    showMessage: function(message){
      alert(message);
    }
  });
  return uploader;
}
                                
function CKFinder_BrowseServer(input_id) {                                                                                                                                                       
	var finder = new CKFinder();
	finder.BasePath = '/ckfinder/' ;
	finder.selectActionFunction = CKFinder_SetFileField;
	//finder.SelectFunctionData = input_id + '|' + target_id ;
	finder.selectActionData = input_id;
	finder.popup() ;
}
 

// This is a sample function which is called when a file is selected in CKFinder.
function CKFinder_SetFileField(fileUrl, data) {
  if(g_userid) fileUrl = fileUrl.replace('/user/', '/user/' + g_userid + '/');
  var input_id =  data["selectActionData"];
  var target_val = input_id == 'background-image' ? "url('" + fileUrl + "')" : fileUrl;   
	var remote = $(window.opener).length == 1;
                                                                                                  
	var $input_field = $('#' + input_id);
  var size = data['fileSize'];
  var nameAr = fileUrl.split('/');
  var name = nameAr[nameAr.length-1];
  if($g_target) $g_target.css(input_id, target_val);
  $input_field.val(target_val);
  $('#background-image-uploader UL').html('<li class="qq-upload-success"><span class="qq-upload-file">' + name + '</span><span style="display: inline;" class="qq-upload-size">' + size + 'kB</span><span class="qq-upload-failed-text">Failed</span></li>');
  layout_changed(parent_id);
}

var myCodeMirror;

function codemirror_close(cm) {
  myCodeMirror.setValue("");
  myCodeMirror.clearHistory();
  myCodeMirror.toTextArea();    
  //myCodeMirror.destroy();
  var $container = $('#template_edit_container');

  $container.hide();    
}


$(document).on("submit", ".save-template", function() {
  $form = $(this); 
  //codemirror_close(myCodeMirror);
 
  return ajax_handler($form);  
});


$(document).on("click", ".template_edit_cancel", function() {
  var $textarea = $('#codemirror_text'); 
  $textarea.val();
  codemirror_close(myCodeMirror);
});

$(document).on("click", ".__edit_template", function() {
  var file = $(this).data('template');
  console.log("edit template " + file);
  var ajax_url = "/ajax.php?oper=readfile&filename=" + encodeURI(file);
  var $textarea = $('#codemirror_text'); 
  $textarea.html('');
  var $form = $('#template_edit_form');
  var $container = $('#template_edit_container');
  //$textarea = $textarea.length && $textarea || $(form).prependTo('body');  // append if doesn't exist
  $("#codemirror_filename").val(file);
  $("#codemirror_title").html(file);
  $textarea.load(ajax_url, function(result) {
    var len = result.length;
    $container.removeClass("hidden").show();
    console.log("Loaded " + len + " bytes from " + ajax_url);
    //return;
    var CM = document.getElementById("codemirror_text");
    myCodeMirror = CodeMirror.fromTextArea(CM, {
      lineNumbers: true,
      mode: "htmlmixed"
    });
    //myCodeMirror.refresh();
    myCodeMirror.setSize("100%", 600);
    $('.CodeMirror').resizable({
      resize: function() {
        editor.setSize($(this).width(), $(this).height());
      }
    });
  });
  
});
    
$(document).on("click", ".delete_object, .delete-object", function() {
  $target = $(this);
  var reload = 0;
  var options = $target.data('options') || {};
  // var data    = $target.data('data') || {};
  //var confirm = $target.data('confirm') || false;

  var handler = $target.data('handler') || options.handler || $target.data('success_handler') || options.success_handler || '';
  var obj_type = $target.data('obj_type') || options.obj_type;
  var obj_id =  $target.data('id') ||  $target.data('obj_id') || options.id || options.obj_id || '';
  var handler_data = $target.data("handler_data") || options.handler_data || '';
  var reload_link = $target.data('reload_link') || options.reload_link || '';
  
  if(handler) options.handler = handler;
  if(handler && handler_data) options.handler_data = handler_data;
  if(reload_link) options.reload_link = reload_link;
  
  console.log("delete_link: data=",$target.data()," options=",options);
  
  // if(handler_data && !is_object(handler_data)) handler_data = parse_json(handler_data);

  console.log("Delete object obj_type=" + obj_type + " id=" + obj_id + " handler=" + handler + " hdata=", handler_data + " options=", options);

  if(!obj_type) {alert("delete_object: missing object type");return false;}
  if(!obj_id) {alert("delete_object: missing object id");return false;}


  return delete_object(obj_type, obj_id, options);
  
  /** old
  //if(id) reload = 1;
  var fields = $target.data('fields');
  var form_class = $target.data('form_class');
  
  if(confirm("Are you sure you want to delete " + obj_type + " " + id + "?")) delete_object(obj_type, id, options);
  return false;
  */
});

// delete an element from DOM
function delete_element(selector, options) {
  if(typeof selector == "undefined") { // try data
    $el = $(this);
    var selector = $el.data("target");
  }
  var $target = $(selector);
  if($target.length) {
    console.log("Removing from DOM:" + selector);
    $target.hide('slow', function(){ $target.remove(); });
  }
}

// handler for delete-object: set handler_data.target to delete DOM element
function delete_object_handler(params, data) {
  console.log("delete object_handler params=");
  console.log(params);
  var handler_data = params.handler_data || {};
  var target = handler_data.target;
  console.log("delete object_handler target=" + target);
  if(target) delete_element(target);
}

// add an element from DOM using template
function add_element(selector, template, controller, data) {
  var $target = $(selector);
  var options = {};
  console.log("Add element to " + selector + " len=" + $target.length + " from template=" + template + " with controller=" + controller);
  console.log("Add element data=", data);
  if($target.length) {
    console.log("Adding template to DOM:" + template);
    options.prepend = 1;
    template_load(selector, template, controller, options, data)
  }
}


// handler for add-object: set handler_data.target to add DOM element
function add_object_handler(params, data) {
  console.log("add object_handler params=",params);
  console.log("add object_handler data=",data);
  var handler_data = params.handler_data || {};
  var target = handler_data.target;
  var template = handler_data.template;
  var controller = handler_data.controller;
  console.log("add object_handler target=" + target);
  if(target && template) add_element(target, template, controller, data);
}

// new ajax functions for back-end
// function delete_object(obj_type, id, handler, reload, keepalive) {
function delete_object(obj_type, id, options) {
  if(typeof options == "undefined") var options = {};

  var reload_link = options.reload_link || '';  
  if(typeof obj_type == "undefined") { // try data
    //alert("delete_object: no obj_type"); /** no this in context??, should be in click handler  */
    $el = $(this);
    var obj_type = $el.data("obj_type");
    var id = $el.data("id") || $el.data("obj_id");
    var reload_link = $el.data("reload_link");
    var handler = $el.data("handler");
    var handler_data = $el.data("handler_data") || {};
    console.log("delete_object: " + obj_type + " " + id);
    if(handler_data) {
      console.log("delete_object: handler_data=",handler_data);
      //handler_data = parse_json(handler_data);
    }
    
    if(handler) options.handler = handler;
    if(handler_data) options.handler_data = handler_data;
  }
  
  
  if(obj_type && id) {  
    console.log("delete object: obj_type=" + obj_type + " id=" + id + " handler=" + handler + " reload_link=" + reload_link);
    
    //return false;
    var success = operation_object(obj_type, id, 'delete', options);
    if(reload_link && $(reload_link).length) {
      console.log('ajaxSubmit: last step, success: reload-link=' + reload_link);
      $(reload_link).click(); // link which will trigger soft reload
      console.log("Return false to avoid jump");
      return false;
    }      
    if(redirect = $el.data('redirect')) {
      console.log('redirecting to:' + redirect);
      window.location.replace(redirect);
    }
    if(options.handler) return false;
    
    return success; // return true will cause link to be used 
  }
  return false;
}

function add_object(obj_type, options, data) {
  return operation_object(obj_type, 0, 'add', options, data);
}

function update_object(obj_type, id, options, data) {
  return operation_object(obj_type, id, 'edit', options, data);
}

// add/update/delete object, or some other handler in app_data
function operation_object(obj_type, id, action, options, data) {
  if(typeof options == "undefined") var options = {};
  if(typeof data == "undefined") var data = {};
  //var action = options.action;
  if(action == 'delete') action = 'del'; // alias
  if(action == 'update') action = 'edit'; // alias
  
  if((action == 'edit' || action=='del') && !id) {
    alert("update/delete object: missing id");
    return false;
  }

  if((action == 'add' || action=='edit') && !data) {    
    alert("add/update object: missing data");
    return false;
  }

  //if(!(action =='add' || action == 'edit' || action=='del')) {
  // alert("add/update/delete object: illegal operation action " + action);
  if(!action) {
    alert("operation object: missing operation action " + action);
    return false;
  }

  var handler = options.handler;
  var handler_data = options.handler_data;
  var reload = options.reload;
  var keepalive = options.keepalive;

  var ajax_url = '/ajax.php?obj_type='+ obj_type + '&id=' + id + '&oper=' + action;
  //alert(dump(options));
  //alert(dump(data));
  //alert(ajax_url);
  jQuery.ajax({
    type: "POST",
    data: data,
    url: ajax_url,
    success: function (responseText) {
      //alert(dump(responseText));
      var result = parse_json(responseText);
      var success = result.success;
      var message = result.message;
      var result_data = result.data;
      var error = result.error;
      var message = result.message;
      
      var $messages = $('#ajax_messages');
      var $errors = $('#ajax_errors');

      //alert("oo handler=" + handler + " result=" + dump(result));
      // var $messages = $('#' + form_id + '_messages');
      // var $errors = $('#' + form_id + '_errors');
      // if(error) $errors.html(error);
      // if(message) $messages.html(message);
      // alert("keepalive=" + keepalive + dump(result));
      if(error) alert(error);
      if(result.success) {        
        if(!is_object(result_data)) result_data = parse_json(result_data); 
        if(handler_data) jQuery.extend(result_data, handler_data);
        //alert("calling sh");   
        success_handler(options, result_data);
        //alert('success:' + message);
        //if(handler) {           
        //  if(!is_object(result_data)) result_data = parse_json(result_data); 
        //  if(handler_data) jQuery.extend(result_data, handler_data);
        //  var funcCall = handler  + "(options, result_data);";
        //  //alert('func:' + funcCall);
        //  eval(funcCall);
        //}
        if(keepalive == 1) {
        } else {
          //close_parent_dialog(form_id);
          if(reload) reload_page();
        }
      } else {
        if(error) $errors.html(error).parent().show();
        if(message) $messages.html(message).parent().show();
        // $messages.html(message);
        //alert('failure:' + message + " selector=" + selector +  " len=" + $(selector).length);
      }
      //ready_script();
      //cms_ready_script();

    },
    error: function(request, error) {
      alert("Error: " + error)     
    }
  });
  return false;
}

function print_host_exceptions(data) {
  console.log("gh: print_host_exceptions");
  console.log(data);
  load_div('print_host_exceptions', 'host_exceptions', data);
}


function rest_handler(params, data) {
  window.console && console.log("rest_handler params=");
  console.log(params);
  window.console && console.log("rest_handler data=");
  console.log(data);
  //return;
  $('#rest_api_response').html(data);
  return true;
}

function user_photo_handler(params, data) {
  window.console && console.log("user_photo_handler params=", params);
  window.console && console.log("user_photo_handler data=", data);
  var file = null;
  if(data && data.files) file = data.files[0];
  if(file && file.type == 'image') {
    var src = file.url;
    console.log("src=" + src + " target len=" + $("#user_profile_photo").length);
    $("#user_profile_photo").attr('src', src);
  }
  return true;
}
  

function cms_media_handler(params, data) {
  var site_id = data.site_id; 
  var obj_type = data.obj_type || params.obj_type || 'media';
  var parent_type = data.parent_type;
  var parent_id = data.parent_id;   
  var user_id = data.user_id;

  // add node to tree
  var tree = $("#tree").dynatree("getTree");
  var parent_node = tree.getNodeByKey(parent_type + '-' + parent_id);
  if(parent_node) {
    parent_node.expand(true);
    parent_node.activate();  
    var newNode = parent_node.addChild({
      key: obj_type + '-' + data.id,
      url: '/ajax.php?oper=load-content&site_id=' + site_id + '&user_level=10&user_id=' + user_id + '&obj_type=media&id=' + data.id,
      title: data.name || "(No title)",
      icon: 'palette-paint-brush.png',
      isFolder: false
    });
    //var url = parent_node.data.url;
    //if(url) $("#cms_viewer").load(url);
  }
  
  console.log("cms_media_handler");
  console.log(params);
  console.log(data);  
}

function new_media_handler(params, data) {
  var site_id = data.site_id; 
  //alert(dump(data));
  var obj_type = data.obj_type || params.obj_type || 'media';
  //alert("ot=" + obj_type);
  var ajax_url = '/ajax.php?oper=load-function&function=gallery_object_list_item&param1=' + site_id + '&param2=' + obj_type + '&param3=' + data.id + '&param4=' + JSON.stringify(params);
  //alert(ajax_url);
  
  $.get(ajax_url, function(response) {
    var $gc = $('#gallery_content');
    if(!$gc.length) return;    
    var $ul = $('#gallery_content > UL');
    if(!$ul.length) $gc.html("<ul class='gallery_list box-drop-shadow'></ul>");
    
    $('#gallery_content > UL').append('<li id="media-item-' + data.id + '" class="border-bottom-light relative">'+response+'</li>');
  });
}

function new_pet_handler(params, data) {
  var site_id = data.site_id; 
  var user_id = data.user_id;
  console.log(params);
  console.log(data);
  var ajax_url = '/ajax.php?oper=load-function&function=pet_list&param1=' + site_id +'&param2=' + user_id;
  console.log(ajax_url);
  $.get(ajax_url, function(response) {
    var $pl = $('#pet_list');
    if(!$pl.length) return;    
    $pl.html(response);
  });
}

// used in calendar after new org is added inline
function new_org_handler(params, data, parent_id) {
  var copy_fields = ['salutation', 'first_name', 'last_name', 'address1', 'address2', 'zip', 'city', 'state', 'country', 'phone', 'email_address'];
  //alert(dump(data));
  $.each(copy_fields, function(k, v) {
   var source = data[v]; 
   var $dest = $("#" + v); 
   if((!$dest.val() || v == 'country') && source) $dest.val(source);
  });
  
  update_parent(data, parent_id);
  
}                                                                                                                 

// generic submit handler for submit_form
// updates parent_id with object id of new object
function update_parent(data, parent_id) {
  //alert(dump(data));
  //return;

  var obj_id    = data.obj_id || data.id;
  var descriptor = data.obj_descriptor || data.name; // todo: make generic

  var $parent = $('#' + parent_id);
  var $parent_text = $('#' + parent_id + '_textbox');
  if ($parent.is('select')) { // select box
    $parent.
      append($("<option></option>").
      attr("value",obj_id).
      text(descriptor));
    $parent.val(obj_id);
    // alert(parent_id + " is select box");
  } else if ($parent_text.length && $parent_text) { // ajax chooser: todo, check for in
    //alert('new ajax_chooser');
    $parent.val(obj_id);
    $parent_text.val(descriptor);
  } else if ($parent.length) { // some - div info text field
    // alert('writing' + responseText + ' to ' + parent_id);
    $parent.html(responseText);
  } else {
    alert('??');
    $parent.val(obj_id);
  }
}

function save_object_dialog(parent_id, obj_type, form_id, unauthenticated) {
  var $dialog = $('#new_object_dialog');
  var $dialog_errors = $('#new_object_dialog_errors');
  var $dialog_messages = $('#new_object_dialog_messages');
  var form_data_str = $('#' + form_id).serialize();
  //ajax_url = "/ajax.php?oper=add&" + form_data_str;
  // var form_data = $('#' + form_id);

  ajax_url = "/" + (unauthenticated ? "home" : "admin") + "/app_data.php?oper=add&obj_type=" + obj_type;
  //alert(ajax_url);

  jQuery.ajax({
    type: "post",                                                         
    data: $('#' + form_id).serialize(),
    url: ajax_url,
    success: function (responseText) {
      $dialog_messages.html(responseText);
      if(responseText.indexOf('OK:') == 0) {
        //alert('YES! ' + responseText);
        var pos;
        //if(pos = responseText.indexOf("\n")) responseText 
        // remove custom lines if any
        
        temp = responseText.split(' ');
        var result    = temp.shift();
        var oper      = temp.shift();
        var obj_type  = temp.shift();
        var obj_id    = temp.shift();
        var descriptor = temp.join(' ');
        descriptor = descriptor.slice(1, descriptor.length-1); // remove opening and closing parenthesis
        var $parent = $('#' + parent_id);
        var $parent_text = $('#' + parent_id + '_textbox');
        if ($parent.is('select')) { // select box
          $parent.
            append($("<option></option>").
            attr("value",obj_id).
            text(descriptor));
          $parent.val(obj_id);
        } else if ($parent_text.length && $parent_text) { // ajax chooser: todo, check for in
          //alert('new ajax_chooser');
          $parent.val(obj_id);
          $parent_text.val(descriptor);
        } else if ($parent.length) { // some - div info text field
          // alert('writing' + responseText + ' to ' + parent_id);
          $parent.html(responseText);
        } else {
          alert('??');
          $parent.val(obj_id);
        }
        // alert(' Result:' + result + ' oper:' + oper + ' ot:' + obj_type + ' id:' + obj_id + ' desc:' + descriptor);
        $dialog.dialog('close');
      } else {
        // alert('nope');
      }
    }
  });
}

// the latest
//function object_dialog(obj_type, id, fields, data, handler, reload, unauthenticated) {
function object_dialog(obj_type, id, options) {
  if(typeof options == "undefined") var options = {};
  var dialog_id = 'object_dialog';
  //var id = parseInt(id);
  var fancybox = true;  
                        
  if(fancybox) {
    var ajax_url = load_dialog(obj_type, id, options, true);
    //alert(ajax_url);return;
    console.log("object_dialog url=" + ajax_url);
    $.fancybox({
      type: 'ajax',
      href: ajax_url,
      closeClick: false, // prevents closing when clicking INSIDE fancybox 
      openEffect: 'none',
      closeEffect: 'none',
      minWidth: 600,
      minHeight: 600,
      
      afterShow: function() {        
        loadEditors();
        ready_script();
        cms_ready_script();
        $.fancybox.update(); 
      },
      helpers: { 
       overlay: {closeClick: false} // prevents closing when clicking OUTSIDE fancybox 
      }      
    });
  } else { // jquery dialog
    var $dialog = $('#' + dialog_id);
    $dialog = $dialog.length && $dialog || $('<div id="' + dialog_id + '"></div>').appendTo('body');  // append if doesn't exist
    $dialog.html('Loading...');
    $dialog.dialog({   
     autoOpen: false,
     closeOnEscape: false,
     modal: true,
     //draggable: true,
     title: "Loading...",
     width: 500,
     //position: 'top'
    });
  
  //   width: 1000,
  //   height: 1000
    
    $dialog.dialog('open'); // jquery call to show overlay
    load_dialog(obj_type, id, options);
  }
}

function publish_page(site_id, content_id, options) {
  if(typeof options == "undefined") var options = {};
  dialog_id = 'publish_page_dialog';
  var data = {'site_id': site_id, 'content_id': content_id};
  if(options.data) {
    data = $.extend(data, options.data);
    delete options.data;
  }
  options.title = "Publish Page " + content_id;
  options.unauthenticated  = 0;  
  return generic_load_dialog(dialog_id, 'publish-page', data, options);
}                                         

function publish_site(site_id, options) {
  if(typeof options == "undefined") var options = {};
  dialog_id = 'publish_site_dialog';
  var data = {'site_id': site_id};
  if(options.data) {
    data = $.extend(data, options.data);
    delete options.data;
  }
  options.handler = 'publish_site_handler';
  options.overwrite = 0;
  options.title = "Publish Site " + site_id;
  options.unauthenticated  = 0;
  options.html = "<div class='progressbar'></div>Processed <span class='progress_count'>0</span> of <span class='progress_total'>?</span>";

  //var url = '/ajax.php?oper=progress-bar-create&name=kj4&total=100';
  var url = '/ajax.php?oper=get-site-tree&site_id='+site_id;
  $.ajax({
    url: url,            
    success: function (msg) {      
      var result = parse_json(msg);
      data.result = result;
      generic_load_dialog(dialog_id, 'publish-site', data, options);
    }
  });
  return;
}

function publish_site_handler(params, data) {
  dialog_id = 'publish_site_dialog';
  var result = data.result;  
  var count = parseInt(result.count);
  var tree = result.tree;
  var site_id = data.site_id;
  
  var progress_bar = '#' + dialog_id + ' .progressbar';
  var progress_count = '#' + dialog_id + ' .progress_count';
  var progress_total = '#' + dialog_id + ' .progress_total';
  $(progress_bar).progressbar();
  $(progress_bar).progressbar("value", 0);
  $(progress_count).html('0');
  $(progress_total).html(count);
  traverseTree(site_id, dialog_id, tree);
}

function add_property_handler(params, data) {
  var base_url = data.base_url;
  var id = data.id;
  var redirect = base_url + id + '/';
  //alert("We added your property - you can now edit your property");
  window.location.replace(redirect);
}

// this will delete a div with an ID = prefix + data.id
function delete_div_handler(params, data) {
  var id = data.id;
  //alert(dump(data));
  var ids = id.split(',');
  
  var prefix = params.prefix || 'media-item-';
  //alert("delete_media_handler: id=" + id + " params" + dump(params) + " data" + data);   
  //alert("delete_div_handler: id=" + id + " prefix" + prefix);   
  $.each(ids, function(index, id) {
    var selector = '#' + prefix + id;
    $div = $(selector);
    if($div.length) {
      $div.fadeOut(300, function() { $(this).remove(); });
    } else {
      //alert("couldn't find " + selector);
    }
  });
}

/*
function gallery_move_handler(params, data) {
  alert('move' + dump(params));
  var id = data.id;
  var selector = '#media-item-' + id;
  //alert('selector=' + selector + ' len=' + $(selector).length);
  $('#media-item-' + id).hide('fast');
}
*/


function save_profile_handler(params, data) {
  $(".buttons").removeClass("disabled loading-big");
  $("#cancel_edit").show();
  var text = "Your profile was updated.";
  if(link = params.referer) text = text + " <a href='" + link + "'>Check it out >></a>";
  $('html, body').animate({scrollTop:0}, 'slow');
  $("#profile_form_messages").html(text);
}

function traverseTree(site_id, dialog_id, tree) {
  $.each(tree, function(path,node) {
    //alert(dump(node));
    var children = node.children;
    //alert(dump(node.children));
    var $dialog = $('#' + dialog_id);
    var childCount = parseInt(node.child_count);
    var progress_bar = '#' + dialog_id + ' .progressbar';
    var progress_count = '#' + dialog_id + ' .progress_count';
    var progress_total = '#' + dialog_id + ' .progress_total';
    var dyn_items = [];
    
    if(node.dynamic == 0) {
      var url = '/ajax.php?oper=publish-node&site_id='+site_id+'&node=' + escape(array2json(node));;
      $.ajax({
        url: url,            
        success: function (msg) {      
          var item = parse_json(msg)
          var count = $(progress_bar).progressbar("value");
          var count = parseInt($(progress_count).html()); 
          var total = parseInt($(progress_total).html()); 
          var next = count + 1;
          var perc = (next / total) * 100;
          //$dialog.append('published ' + node.name + ': ' + msg + '<br>');
          $('#' + dialog_id +' table > tbody:last').append('<tr><td>' + item.path + '</td><td>' + item.length + '</td><td>' + item.time + '</td><td>' + item.status + '</td><td>' + item.message + '</td></tr>');
          //$('#' + dialog_id +' table > tbody:last').append(array2tr(item));          
          $(progress_count).html(next);
          $(progress_bar).progressbar("value", perc);  
        }
      });
    }    
    if(children && childCount > 0) {
      traverseTree(site_id, dialog_id, children);
    }
  });
  //return dynItems;
}

function progress_bar_handler(dialog_id, pb_id) {
  timeout = 1000; // 1 seconds
  if(!pb_id) {
    alert("progress_bar_handler: no id");
    return;
  }
  //var url = '/ajax.php?oper=progress-bar&id=' + pb_id;
  var url = '/ajax.php?oper=progress-bar&id=' + pb_id;
  $.ajax({
    url: url,             
    success: function (msg) {
      var data = parse_json(msg);
      var $dialog = $('#' + dialog_id);
      var count = parseInt(data.progress);
      var total = parseInt(data.total);
      var progress_bar = '#' + dialog_id + ' .progressbar';
      var progress_count = '#' + dialog_id + ' .progress_count';
      var progress_total = '#' + dialog_id + ' .progress_total';
      
      $dialog.append('p=' + count + ' t=' + total + '<br>');
      $(progress_bar).progressbar("value", count);
      $(progress_count).html(count);
      $(progress_total).html(total);

      if(count >= total) {     
        isDone = true;
        //$dialog.append('done<br>');
      } else {
        //$dialog.append('Not done, calling pbh with dialog_id = '+ dialog_id + ' and pb_id=' + pb_id + ' <br>');
        setTimeout(progress_bar_handler(dialog_id, pb_id), timeout);
      }                    
    }
  });
}


// creates a dialog (jQuery UI or fancybox) with content from app_data operation
// alternatively; just return the ajax_url (options: url_only)
//function load_dialog(obj_type, id, fields, data, handler, reload, unauthenticated) {  
function load_dialog(obj_type, id, options, url_only) {  
  if(typeof options == "undefined") var options = {};
    
  // fields, data, handler, reload, unauthenticated
  var operation = options.operation || 'load_dialog';

  var handler = options.handler;
  var success_handler = options.success_handler;
  var handler_operation = options.handler_operation;
  var handler_data = options.handler_data;
  var target = options.target;
  var reload = options.reload;
  var readonly = options.readonly;
  var keepalive = options.keepalive;
  var overwrite = options.overwrite;
  var parent_id = options.parent_id;
  var form_class = options.form_class;
  var data = options.data;
  var class_options = options.class_options;
  var unauthenticated = options.unauthenticated;
  var fields = options.fields || [];
  var dialog_title = options.dialog_title;

  var dialog_id = 'object_dialog';
  var $dialog = $('#' + dialog_id);
  //id = parseInt(id);
  var action = id ? 'Edit' : 'Add';
  //alert("ld data=" + dump(data));
  //alert('id=' + id + ' action=' + action);
  var title = action + ' ' + obj_type + ' ' + id;
  var reload_bool = typeof reload !== "undefined" && reload ? 1 : 0;
  var reload_template = options.reload_template || '';
  var reload_target = options.reload_target || '';
  var reload_link = options.reload_link || '';
  
  //alert(dump(data_fields));

  // $dialog.data('reloadfoo', (reload ? 1 : 0));
  // $dialog.data('reloadfoo', "hello");
  // alert("load: reload=" + reload + ' bool =' + reload_bool); 

  //alert(dump(options));
 
  ajax_url = "/" + (unauthenticated ? "home" : "admin") + "/app_data.php?oper=" + operation + "&obj_type=" + obj_type + "&id=" + id + "&reload=" + reload_bool;
  
  if(fields) ajax_url += "&fields=" + array2json(fields);
  if(handler) ajax_url += '&handler=' + handler;
  if(handler_operation) ajax_url += '&handler_operation=' + handler_operation;
  if(success_handler) ajax_url += '&success_handler=' + success_handler;
  if(target) ajax_url += '&target=' + target;
  if(parent_id) ajax_url += '&parent_id=' + parent_id;
  if(form_class) ajax_url += '&class=' + form_class;
  if(dialog_title) ajax_url += '&dialog_title=' + encodeURIComponent(dialog_title);
  if(reload_template) ajax_url += '&reload_template=' + encodeURIComponent(reload_template);
  if(reload_link) ajax_url += '&reload_link=' + encodeURIComponent(reload_link);
  if(reload_target) ajax_url += '&reload_target=' + encodeURIComponent(reload_target);
  if(readonly) ajax_url += '&readonly=' + readonly;
  console.log("url2=" + ajax_url);

  //if(data && !id) ajax_url += '&' + obj2qs(data);
  if(data) ajax_url += '&' + obj2qs(data);
  if(handler_data) ajax_url += '&handler_data=' + encodeURIComponent(JSON.stringify(handler_data));   
  if(class_options) ajax_url += '&class_options=' + encodeURIComponent(JSON.stringify(class_options)); //array2json(obj_options);
  
  //alert(ajax_url);
  if(url_only) return ajax_url;
  
  // alert(dump(data));
  var fancybox = true;
  if(!$dialog.length) { // fancybox removes the ID of the enclosing DIV
    $dialog = $('.fancybox-inner');
    if($dialog.length) {
      fancybox = true;
      $dialog.prop("id", dialog_id);
    }
    //alert("load dialog " + obj_type + " " + id + " url=" + ajax_url + " dialog length=" + $dialog.length);
  }
  $dialog.load(ajax_url, data, function (responseText) {
    if(fancybox) {
      loadEditors();
      $.fancybox.update(); 
      setupCKeditor(dialog_id);
      //alert('done loading');
    } else {
      $dialog.dialog('option', 'title', title);;
      $('textarea').each(function() {
        try {
          $(this).ckeditorGet().destroy();
        } catch(e) {
        }
      });
      setupCKeditor(dialog_id);
    }
    ready_script();
    cms_ready_script(); // call document ready script to invoke handlers
    
    //if(fancybox) $.fancybox.update(); 

    
    //loadEditors('#'+dialog_id + ' textarea.jqckeditor');

    //alert("loaded " + ajax_url);
    // loop through all text areas - destroy if ckeditor
    // http://stackoverflow.com/questions/1794219/ckeditor-instance-already-exists

    //.ckeditor();
  });                                    

  //jQuery.ajax({
  //  type: "POST",
  //  data: data,
  //  url: ajax_url,
  //  success: function (responseText) {
  //    $dialog.html(responseText);
  //  },
  //  error: function(request, error) {
  //    alert("Error: " + error)     
  //  }
  //});

}
  
/*
	var editor = CKEDITOR.replace( 'editor1' );
	editor.setData( '<p>Just click the <b>Image</b> or <b>Link</b> button, and then <b>&quot;Browse Server&quot;</b>.</p>' );

	// Just call CKFinder.setupCKEditor and pass the CKEditor instance as the first argument.
	// The second parameter (optional), is the path for the CKFinder installation (default = "/ckfinder/").
	CKFinder.setupCKEditor( editor, '../' ) ;
*/
function ckeditor_enable(id) {
  var config = ckeditor_config(id);
	var editor = CKEDITOR.replace(id, config);

	//alert(id);
	//CKFinder.setupCKEditor( editor, '/ckfinder/') ;

	// Just call CKFinder.setupCKEditor and pass the CKEditor instance as the first argument.
	// The second parameter (optional), is the path for the CKFinder installation (default = "/ckfinder/").

	/*
 CKEDITOR.replace(id,
  {
  // toolbar : 'Basic',
  filebrowserBrowseUrl :      '/ckeditor/ckfinder/ckfinder.html',
  filebrowserUploadUrl :      '/ckeditor/ckfinder/core/connector/php/connector.php?command=QuickUpload&type=Images',
 	filebrowserImageBrowseUrl : '/ckeditor/ckfinder/ckfinder.html?type=Images',
 	filebrowserFlashBrowseUrl : '/ckeditor/ckfinder/ckfinder.html?type=Flash'

 });
 */
 
}

// gets the options for the editor
function ckeditor_config(id) {
  //options = { toolbar : [ ['Source'], ['Bold','Italic','Underline','Strike'], ], height: 600, width: 900 };
  var $editor = $('#' + id);
  var data = $editor.data('data');
  var options = $editor.data('options') || {};
  var autosave = $editor.data('autosave') || options.autosave;
  var w = options.width || 600;
  var h = options.height || 200;
  //alert(dump(options));  
  var config = { height: h, width: w};
  config.forcePasteAsPlainText = true;
  var toolbar = options.toolbar || $editor.data('toolbar');
  console.log("ckeditor toolbar=" + toolbar);
  var toolbarAr = [];
  if(options.toolbar) console.log("Toolbar=" + options.toolbar);
  switch(toolbar) {
    case 'none':
      toolbarAr = [];
      break;
    case 'minimal':
      toolbarAr = ['Bold','Italic'];
      break;                                  
    case 'basic':                              
      toolbarAr = ['Bold','Italic','Underline'];
      break;                                  
    case 'rich':                              
      toolbarAr = ['Bold','Italic','Underline','NumberedList','BulletedList'];
      break;                                  
    case 'color':                              
      toolbarAr = ['Bold','Italic','Underline','NumberedList','BulletedList','TextColor','BGColor'];
      break;                                  
    case 'full':                              
      toolbarAr = ['Source', 'Bold','Italic','Underline','StrikeThrough','TextColor','BGColor', 'NumberedList','BulletedList','Image','Table','Link'];
      config.forcePasteAsPlainText = false;
      break;
    default:
      toolbarAr = ['Source', 'Bold','Italic','Underline','StrikeThrough','TextColor','BGColor', 'NumberedList','BulletedList','Image','Table','Link'];
      break;
  }
  
  console.log(toolbarAr);                                                                        
  if(autosave) {
    toolbarAr.unshift('Autosave');
    config.extraPlugins = 'autosave';
    config.autosaveTargetUrl = '/ajax.php?oper=ckeditor_autosave';
    if(data) config.autosaveRequestParams = obj2qs(data);
  } else if(0) { // not sure when we can show save button yet
    toolbarAr.unshift('Save');
  }
  config.toolbar = [toolbarAr];

  return config;
}

// from http://stackoverflow.com/questions/1794219/ckeditor-instance-already-exists
function setupCKeditor(editorName) {
  // define editor configuration      
  var config = {skin : 'kama'};

  // Remove and recreate any existing CKEditor instances
  var count = 0;
  if (CKEDITOR.instances !== 'undefined') {
    for(var i in CKEDITOR.instances) {
      var oEditor   = CKEDITOR.instances[i];
      var editorName = oEditor.name;

       // Get the editor data.
      var data = $('#'+editorName).val();
      var editorId = $('textarea.jqckeditor').prop('id');

      // Check if current instance in loop is the same as the textarea on current page
      if (editorId == editorName) {
        if(CKEDITOR.instances[editorName]) {
          // delete and recreate the editor
          delete CKEDITOR.instances[editorName];
          $('#'+editorName).ckeditor(function() { },config);
          count++;
        }
      }   
    }
  }

  console.log("set up CK editors, found " + count);
  // If no editor's exist in the DOM, create any that are needed.             
  if (count == 0){
    loadEditors({'selector': '#'+editorName + ' textarea.jqckeditor'});    
    //loadEditors('#'+editorName + ' textarea.jqckeditor');    
  }
}


// loops through all elements in selector and make them ckeditors unless they're already so
//function loadEditors(selector) {
function loadEditors(data) {
  
  if(typeof data == "undefined") var data = {};
  var selector = data.selector;
  if(typeof selector == "undefined") var selector = '.jqckeditor';
  //$(selector).each(function() {
  //  try {
  //    $(this).ckeditorGet().destroy();
  //  } catch(e) {
  //  }
  //});

  var $editors = $(selector);
  var count = $editors.length;
  console.log("load CK editors found " + count);

  //alert("seletor = " + selector + " len=" + count);  
  if (count) {
    $editors.each(function() {
      var $editor = $(this);
      var editorID = $editor.prop("id");
      var config = ckeditor_config(editorID);
      //var data = $editor.data('data');

      //alert(editorID + " autosave=" + autosave + " data=" + dump(data));
      var instance = CKEDITOR.instances[editorID];        
      if (instance) {            
        //alert(editorID + " exists");
        CKEDITOR.remove(instance);
        CKEDITOR.replace(editorID, config);
      } else {          
        //alert(editorID + " does not exist");
        $editor.ckeditor(config);
        instance = CKEDITOR.instances[editorID];        
      }
      CKFinder.setupCKEditor( instance, '/ckfinder/' ) ;
      //alert(1);

      //instance.on('afterAutosave', function(event) {
      //  alert(1);
      //  //alert(dump(event.data));
      //}, null, data);
      //
      //instance.on('beforeAutosave', function(event) {
      //  alert('before');                  
      //}, null, 'foo');
                                          
      //CKEDITOR.replace(editorID,
      // {
      // // toolbar : 'Basic',
      // filebrowserBrowseUrl :      '/ckeditor/ckfinder/ckfinder.html',
      // filebrowserUploadUrl :      '/ckeditor/ckfinder/core/connector/php/connector.php?command=QuickUpload&type=Images',
      // filebrowserImageBrowseUrl : '/ckeditor/ckfinder/ckfinder.html?type=Images',
      // filebrowserFlashBrowseUrl : '/ckeditor/ckfinder/ckfinder.html?type=Flash'
      //
      //});

    });
  }
}


function new_object_dialog(parent_id, obj_type, title, unauthenticated) {
  var $dialog = $('#new_object_dialog');
  $dialog = $dialog.length && $dialog || $('<div id="new_object_dialog"></div>').appendTo('body');  // append if doesn't exist
  $dialog.html('Loading...');
  $dialog.dialog({   
   autoOpen: false,
   closeOnEscape: false,
   modal: true,
   title: title,
   width: 450,
   height: 700
  });
  $dialog.dialog('option', 'title', 'Loading...'); 
  $dialog.dialog('open'); // jquery call to show overlay
  ajax_url = "/" + (unauthenticated ? "home" : "admin") + "/app_data.php?oper=new_object_dialog&obj_type=" + obj_type + "&inline_pid=" + parent_id;
  //alert(ajax_url);
  $dialog.load(ajax_url, function (responseText) {
    $dialog.dialog('option', 'title', title);
  });                                    
}

function catering_delivery_update() {
  var delivery = $('#add_catering_div #catering_delivery').val();
  if(delivery == "1") {
    $("#catering_pickup_address").closest(".fieldset_div").hide();
    $("#catering_pickup_time").closest(".fieldset_div").hide();
    $("#catering_contractor_id").closest(".fieldset_div").hide();
    $("#catering_delivery_address").closest(".fieldset_div").show();
    $("#catering_delivery_time").closest(".fieldset_div").show();
  } else {
    $("#catering_delivery_address").closest(".fieldset_div").hide();
    $("#catering_delivery_time").closest(".fieldset_div").hide();
    $("#catering_pickup_address").closest(".fieldset_div").show();
    $("#catering_pickup_time").closest(".fieldset_div").show();
    $("#catering_contractor_id").closest(".fieldset_div").show();
  }
}

function load_events(params, data) {
  //alert(dump(data));
  var res_id = data.res_id;
  if(!res_id) {
    alert("Error: no reservation ID received.");
    return false;
  }
  var $events = $('#reservation_events_div');
  var $dialog = $('#add_catering_div');
  $events.addClass("loading-big");
  var ajax_url = '/ajax.php?oper=load_events&res_id=' + res_id;
  $events.load(ajax_url, function(responseText) {
    $dialog.html('').hide('fast');
    $events.removeClass("loading-big");
    var position = $events.position();
    scroll(0,position.top);
    //$('html, body').animate({scrollTop:0}, 'fast');
  });                          
}

function load_event_form(res_id, event_id) {
  if(!res_id) {
    alert("Error: no reservation ID received.");
    return false;
  }

  data.res_id = res_id;
  data.event_id = event_id;  
  load_div('add_event_div', 'load_event_form', data); 


  //alert(dump(data));
  // alert("res_id=" + res_id + " event_id=" + event_id);
  
  var $dialog = $('#add_event_div');
  $dialog.show('fast');      
  $dialog.addClass("loading-big");
  $dialog.html('<div class="round pad border margin_top">Loading...</div>');

  var ajax_url = '/ajax.php?oper=load_event_form&res_id=' + res_id+"&event_id=" + event_id;
  //alert(ajax_url);
  $dialog.load(ajax_url, function(responseText) {   
    //alert("loaded");
    $dialog.removeClass("loading-big");
  });                          
}


function load_catering(res_id, lineitem_id) {
  if(!lineitem_id) {
    alert("Error: no lineitem ID received.");
    return false;
  }
  if(!res_id) {
    alert("Error: no reservation ID received.");
    return false;
  }
  var $dialog = $('#add_catering_div');
    
  var supplier_id = $('#supplier_id_' + lineitem_id).val();
  if(!supplier_id) {
    alert("Please choose a supplier first");
    return false;
  }
  $dialog.show('fast');      
  $dialog.addClass("loading-big");
  $dialog.html('<div class="round pad border margin_top">Loading...</div>');

  var ajax_url = '/ajax.php?oper=load_catering&lineitem_id=' + lineitem_id + "&supplier_id=" + supplier_id + "&res_id=" + res_id;
  $dialog.load(ajax_url, function(responseText) {
    $dialog.removeClass("loading-big");
    catering_delivery_update();
  });                             
}

function delete_uploaded_file(parent_id, list_index, filename) {
  ajax_url = "/ajax.php?oper=delete_file";
  jQuery.ajax({
    type: "post",
    data: "filename="+filename,
		// contentType: "application/json; charset=utf-8",
		// dataType: "json",
    url: ajax_url,
    success: function (responseText) {
    	var result = parse_json(responseText);
    	if(result.success) {
				// remove item from screen list and hidden field
				$('#' + parent_id +'-filelist li:nth-child(' + list_index + ')').remove();
				//$item.remove();
				var $target = $('#' + parent_id);
				var val = $target.val();
				if(val && list_index) {
					var files = val.split(",");										
					//files = removeArrayElement(files, list_index - 1, 0); // index is 1 based. Arrays zero based
					files.splice(list_index - 1, 1); // List_index is 1 based. Arrays zero based
          // alert(val + '<br>' + files.join(',') + '<br>' + 'list_index = ' + list_index);
					$target.val(files.join(','));							
      	}
      } else {
    		alert('Could not delete file ' + filename);      	
    		// alert('Could not delete file:' + responseText);      	
    	}
    }
  });
}

function save_layout(parent_id) {
  var $errors = $('#' + parent_id + '_errors');
  var $messages = $('#' + parent_id + '_messages');

  
  // var form_data_str = $('#layout_form').serialize();
  ajax_url = "/ajax.php?oper=save_layout"; 
  var $selected = $("#" + parent_id + " .css_select option:selected"); 
  var val  = $selected.val();
  if(!val) {alert("Save Layout: No element selected");return false;}
  var selector = $selected.text();
  var remote = $(window.opener).length == 1;
  var $target = remote ? window.opener.$(selector) : $(selector);
                       
  // alert(ajax_url);  
  $("#" + parent_id + " .css_select").hide('fast');
  $("#" + parent_id + " .css_save").hide('fast');
  $("#" + parent_id + " .css_cancel").hide('fast');  
  $("#" + parent_id + " .css_saving").show();

  //var form_data = $('#' + parent_id).serialize(); // add data
  var form_data = {}; // add data
  // only data when different from orgval
  console.log("save_layout " + parent_id);
  $('#' + parent_id + ' input,select').each(function(){
    var $input = $(this);
    var input_id = $input.prop('id');
    
    var name = $input.prop('name');
    //if(input_id && name) {
    if(name) { // 
      var orgval = $input.data('orgval');
      var current = $input.val();
      if(orgval) {
        if(orgval != current) {
          form_data[name] = current; // changed, so add
          console.log("Orgval of " + input_id + " = " + orgval + " !=  current=" + current + " so adding");
        } else {
          console.log("Orgval of " + input_id + " = " + orgval + " ==  current=" + current + " so deleting");
        }
      } else {
        form_data[name] = current; // not a user changable field, so add
      }
    }
    //if(orgval && orgval == current) {
    //  console.log("Orgval of " + input_id + " = " + orgval + " ==  current=" + current + " so deleting");
    //  delete form_data[name];
    //}
    //console.log("Orgval of " + input_id + " = " + orgval + " current=" + current); 
  });
  //alert(dump(form_data));
  //console.log("Form data (changes) " + form_data); 
  //alert('gh');
  jQuery.ajax({
    type: "post",
    data: form_data,    
    url: ajax_url,
    success: function (responseText) {
      if(responseText.indexOf('OK:') == 0) {
        $messages.html(responseText);
        layout_select_div(parent_id, selector); // this calls layout_save_original()

        layout_show_save(parent_id, false);        
        //var loc = document.location.toString();
        //var location_arr = loc.split('?');
        // window.opener.document.location = location_arr[0] + '?css_edit=' + escape(selector); // reload
      } else {
        $errors.html(responseText);
        $("#" + parent_id + " .css_saving").hide('fast');
        $("#" + parent_id + " .css_save").show('fast');
        $("#" + parent_id + " .css_cancel").show('fast');  
      }
    }
  });
}			

// opens a modal dialog for previewing a uri
function preview_dialog(title, ajax_url) {
  var $dialog = $('#preview_dialog');
  $dialog = $dialog.length && $dialog || $('<div id="preview_dialog"><iframe id="preview_iframe" width="100%" height="100%">No iFrame support</iframe></div>').appendTo('body');  // append if doesn't exist
  //$dialog.html('Loading...');
  $dialog.dialog({   
   autoOpen: false,
   closeOnEscape: false,
   modal: true,
   title: title,
   width: 1024,
   height: 768
  });                                                 
  $dialog.dialog('open'); // jquery call to show overlay
  $('#preview_iframe').prop('src', ajax_url); 
}

// end new ajax function for back-end

// fetch product from database, populate dialog
function product_fetch() {
  // alert('product update');
  var product_id = $("#product_id").val();
  if(product_id) {
    ajax_url = "/ajax.php?oper=view&obj_type=product&id=" + product_id;
    jQuery.ajax({
      type: "POST",
      url: ajax_url,
      data: "nh=1",
      cache: false,
      async: false,
      success: function(responseText) {
        var response = parse_json(responseText);            
        var product = parse_json(response.data);
        // alert(dump(product));

        $("#currency").val(product.currency);
        $("#amount").val(product.price);
        $("#description").val(product.name);
        $("#tax_category").val(product.tax_category);
        //$("#tax_percentage").val(product.tax_percentage);
        $("#tax_included").val(1); // tax_included hack
        // $("#tax_included").val(product.tax_included);
         
        product_update();

      }
    });
  }
}

// calculate subtotal, total, show in dialog
// let class handlers handle tax
function product_update() {
  var amount = parseFloat($("#amount").val());
  var quantity = parseFloat($("#quantity").val());
  var discount = $("#discount").val();
  if(discount) amount = apply_discount(amount, discount);
  
  var subtotal = parseFloat(amount * quantity).toFixed(2); // todo: apply discount

  $("#subtotal").val(subtotal);
  var total = subtotal;
  $("#total").val(total);
  return;
  
  // don't do the taxes here, handled by class handlers (always included for now)
  var tax_included = 1; // tax_included hack
  var tax_rate = parseFloat($("#tax_percentage").val());
  if(tax_included) {
    var amount_ex_tax = (subtotal / ((tax_rate / 100) + 1)).toFixed(2);       
    var tax_of_which = (subtotal - amount_ex_tax).toFixed(2);
    var tax = 0;
  } else {
    var tax = (subtotal * (tax_rate / 100)).toFixed(2);
    var tax_of_which = 0;
  }
          
  var total = (parseFloat(subtotal) + parseFloat(tax)).toFixed(2);
//  var total = (subtotal + tax).toFixed(2);

  if(tax_rate > 0) { 
    $("#tax_included_text").html(tax_included ? "Including tax" : "Excluding tax");
  } else {
    $("#tax_included_text").html('');
  }
  
  $("#total").val(total);
  $("#subtotal").val(subtotal);

  // $("#discount").val(0);
  $("#tax_text").html(tax_rate > 0 ? "Of which tax: " +  (tax_included ? tax_of_which : tax) : '');
  //alert(responseText);
}

// returns array(abs_discount, perc_discount)
function split_discount(discount) {
  pos = discount.indexOf('%');
  if(pos > 0) { // % discount
    return [0, parseFloat(discount.substr(0, pos))];
  } else { // absolute discount
    return [parseFloat(discount), 0];
  }
}

function apply_discount(price, discount) {
  if(!discount) return price;
  var disAr = split_discount(discount);
  var abs_discount = disAr[0];
  var perc_discount = disAr[1];
  if(perc_discount > 0) {
    return (price * (100 - perc_discount) / 100).toFixed(2); 
  } else if(abs_discount > 0) {
    return (price - abs_discount).toFixed(2);
  }
}

function reset_product_form() {
  $("#product_id").val("");
  $("#product_id_textbox").val("");
  $("#quantity").val("1");
  $("#amount").val("");
  $("#description").val("");
  $("#total").val("");
  $("#tax_percentage").val("");
  $("#tax_included").val("");
  $("#tax_included_text").val("");
}

function add_product_handler(params, data) {
  reset_product_form();
}

function add_gallery_handler(params, data) {
  //alert(dump(data));
  var $gallery = $('#gallery');

  var id = data.id;
  
  var title = data.title;
  var thumb = '';
  var big = '';
  //var filename = data.src;
  //var info_fields = data.info_fields;

  $(".buttons").removeClass("disabled loading-big");
  var text = "We added your new gallery.";
  //alert(dump(data));
  if(link = params.referer) text = text + " <a href='" + link + "?id=" + data.id + "&edit'>Click here to add images >></a>";
  $('html, body').animate({scrollTop:0}, 'slow');
  $("#user_gallery_messages").html(text);

  // add thumb
  var img = ' 	<li id="li_' + id + '" class="ui-widget-content ui-corner-tr float_left gallery_item">\n' +
            '    <h5 id="lih_' + id + '" class="ui-widget-header">' + title + '</h5>\n' +
            '    <img id="img_' + id + '" src="' + thumb + '" alt="' + title + '" width="96" height="72"/>\n' +
            '    <a id="mag_' + id + '" href="' + big + '" target="pic" title="' + title + '" class="preview ui-icon ui-icon-zoomin">View larger</a>\n' +
            '    <a href="link/to/trash/script/when/we/have/js/off" title="Delete this image" class="ui-icon ui-icon-trash">Delete image</a>\n' +
            '  </li>';   
    
  $gallery.append(img);
  gallery_update_handler(params, data);
  imagePreview();
  edit_gallery_init('gallery');

}

function gallery_upload_handler(params, data) {
  // alert(dump(data));
  var $gallery = $('#gallery');
  var id = data.id;
  var path = data.path;
  var filename = data.src;
  var info_fields = data.info_fields;
  
  // alert('file=' + data.qqfile + " fn=" + data.filename);
  
  var desc = 'Coming soon...';
  var thumb_dir = gallery_resize.thumb ? gallery_resize.thumb[1] + '/' : '';
  var big_dir = gallery_resize.big ? gallery_resize.big[1]  + '/' : '';
  var full_dir = gallery_resize.full ? gallery_resize.full[1]  + '/' : '';

  var thumb = path + thumb_dir + filename;
  var big = path + big_dir + filename;
  var full = path + full_dir + filename;
    
  var img = ' 	<li id="li_' + id + '" class="ui-widget-content ui-corner-tr float_left">\n' +
            '    <h5 id="lih_' + id + '" class="ui-widget-header">' + filename + '</h5>\n' +
            '    <img id="img_' + id + '" src="' + thumb + '" alt="' + filename+ '" width="96" height="72"/>\n' +
            '    <a id="mag_' + id + '" href="' + big + '" target="pic" title="' + desc + '" class="preview ui-icon ui-icon-zoomin">View larger</a>\n' +
            '    <a href="link/to/trash/script/when/we/have/js/off" title="Delete this image" class="ui-icon ui-icon-trash">Delete image</a>\n' +
            '  </li>';   
    
  $gallery.append(img);
  gallery_update_handler(params, data);
  imagePreview();
  edit_gallery_init('media');
}                    

function gallery_update_handler(params, data) {
  var $img = $('#img_' + data.id);
  var $mag = $('#mag_' + data.id);
  $img.data('info', data);
  var vals = [$img.prop('alt')];
  //alert("guh" + dump(data) + dump(gallery_info_fields));
  $.each(gallery_info_fields, function(key, name) {
    var val = eval('data.' + key);
    vals.push(val);
  });
  var new_title = vals.join("<br>");     
  $mag.prop('title', new_title);
}

function share_object(data) {
  //alert(dump(data));
  var obj_type = data.obj_type;
  var site_id = data.site_id;
  var user_id = data.user_id;
  var title = data.title ? data.title : '';
  $('#share_form').show();
  $('#share_form_panel').show('fast');
  $("#sharing_script").val('');
  $("#share_form_script").hide();
  
  $('#share_form #share_title').val(title) 
  //alert("site: " + site_id + " wants to share " + obj_type + " with options " + dump(data));
  
  return false;
}

// sharing form
function sharing_form_setup(form_id, div_id) {
}


function sharing_form_handler(form_id, div_id) {
  var $form = $("#" + form_id); 
  if(!$form.length) {alert(form_id + " not found");return false;}
  $(".input").keypress(function(e) {
     if(e.which == 13) {
         jQuery(this).blur();
         jQuery("#submit_form").focus().click();
     }
  });

  var hide = ['show_title']; // what to hide by default
  $.each(hide, function(index, value) {
    $("#" + form_id + " #" + value).parent().hide();
  });

  hide = [];
  var show = [];
  // when the form changes
  $("#" + form_id + " input,select").change(function() {
    var type = $form.find('select[name="type"]').val();
    switch(type) {
    case 'image':
      show = ['show_title'];
      hide = [];
      //selectorAr = ['height','width','color','text-decoration','font-weight', 'font-style', 'font-family','font-size'];
      break;
    case 'button':
      show = [];
      hide = ['show_title'];
      //selectorAr = ['height','width','color','text-decoration','font-weight', 'font-style', 'font-family','font-size'];
      break;
    case 'link':       
      show = [];
      hide = ['show_title'];
      //selectorAr = ['color','text-decoration','font-weight', 'font-style', 'font-family','font-size'];    
      break;
    default:      
      break;
    }
    
    var selectors_text  = ['font-family','font-size', 'color','text-decoration','font-weight', 'font-style'];
    var selectors_image = ['height','width', 'top', 'left'];
    var selectors_button = ['height','width','background-color', 'border-color','border-style','border-width', 'text-decoration','font-weight', 'font-style', 'font-family','font-size'];
    var selectors_div = ['height','width','margin', 'padding', 'background-color','border-color','border-style','border-width'];
    
    var divs = [];
    divs.push({"name": ".afr_sharing_image", "title": "image", "selectors": selectors_image});
    divs.push({"name": ".afr_sharing_title", "title": "title", "selectors": selectors_text});
    divs.push({"name": ".afr_sharing_link", "title": "link", "selectors": selectors_text});
    divs.push({"name": ".afr_sharing_button", "title": "button", "selectors": selectors_button});
    divs.push({"name": ".afr_sharing_container", "title": "container", "selectors": selectors_div});
    //alert("type =" + type);
    ajax_url = "/afr/ajax.php?oper=preview-sharing"
    var selectorAr = [];    

    $.each(show, function(index, value) {
      console.log("showing sharing form div: " + "#" + form_id + " #" + value);
      $("#" + form_id + " #" + value).parent().show('fast');
    });
    $.each(hide, function(index, value) {
      $("#" + form_id + " #" + value).parent().hide('fast');
    });
    
    console.log('sfh:' + form_id);
    
    if(type) {
      $.ajax({ 
      url: ajax_url, 
      type: "POST", 
      data: $form.serialize(),
      success: function(responseText) {
        $("#share_form_preview").html(responseText);    
          console.log("div length = " + divs.length + " id=" + div_id);
          $css_div = $('#' + div_id);
          $css_div.data('target_divs', divs);
        }
      });
    }

     
    return false;
  });
  $("#submit_form").click(function() {
    $form.submit();
    return false;
  });
  $form.validate({ 
  ignore: ":hidden",
  rules: { 
    title: { 
      required: true, 
    }, 
    link: { 
      required: true, 
    }, 
    type: { 
      required: true, 
    }, 
    style: { 
      required: true, 
    }, 
  }, 
  submitHandler: function(form) {
    var ajax_url = '/afr/ajax.php?oper=add-sharing';                                                                                                                   
    var handler   =  'sharing_handler';
    $("#share_form_panel").addClass("disabled loading-big");
    ajaxSubmit({form: form_id, url: ajax_url, type: "post", "handler": handler, "div_id": div_id});
  }
  });
}

function sharing_handler(params, data) {
  
  $("#share_form_preview").show();

  //alert("done adding params = " + dump(params) + " data=" + dump(data));
  ajax_url = "/ajax.php?oper=view&obj_type=sharing&id=" + data.id;
  jQuery.ajax({
    type: "POST",
    url: ajax_url,
    //data: "nh=1",
    //cache: false,
    //async: false,
    success: function(responseText) {
      //alert(dump(responseText));
        
      $("#share_form_panel").removeClass("disabled loading-big");
      
      var response = parse_json(responseText);
      var sharing = parse_json(response.data);
      var shorturl = sharing.shorturl;
      var script = "<script src='" + shorturl + "'></script>";

      $("#share_form").hide();
      $("#share_form_script").show();
      $("#share_form_preview").hide();
      //$("#share_form_preview").hide();
      $("#sharing_script").val(script);
      $("#sharing_script").data('clipboard-text', script);
      //$("#sharing_script").zclip({
      //  path:"/afr/jquery/zclip/ZeroClipboard.swf",
      //  copy: $("#sharing_script").val()
      //});

      var client = new ZeroClipboard($("#sharing-script"));
      
      client.on( "ready", function( readyEvent ) {
        //alert( "ZeroClipboard SWF is ready!" );      
        client.on( "aftercopy", function( event ) {
          // `this` === `client`
          // `event.target` === the element that was clicked
          event.target.style.display = "none";
          alert("Copied text to clipboard: " + event.data["text/plain"] );
        } );
      });
      
      // create the css styling section
      var div_id = params.div_id;
      var $div = $('#' + div_id);
      //var data = $('#' + div_id).data('data');
      if($div.length && data) {
        $("#share_form_panel").hide("fast");
        console.log("hiding share_form_panel with length=" + $("#share_form_panel").length); 
        data.parent_id = data.id;
        data.parent_type = data.obj_type;
        var target_divs = $div.data('target_divs');
        /** vpatina: remove styling options for now */
        //var css_form_id = layout_create(div_id, data);
        //if(css_form_id) layout_init(css_form_id, target_divs);
      }
      
    }
  });
  //$("#share_form_panel").html("<input type='text' value='' size='40' readonly='readonly'>");
  
  //$(".buttons").removeClass("disabled loading-big");
  //$("#cancel_edit").show();
  //var text = Your profile was updated.";
  //if(link = params.referer) text = text + " <a href='" + link + "'>Check it out >></a>";
  //$('html, body').animate({scrollTop:0}, 'slow');
  //$("#profile_form_messages").html(text);
}

function page_layout_handler(div_id, db_divs, data) {
  var css_form_id = layout_create(div_id, data);
  if(css_form_id) layout_init(css_form_id, db_divs);

  // $("a").click(function() { alert("Links disabled - use page selector in Edit Layout panel or close the panel"); return false; }); 
  //alert("plh: div_id=" + div_id + " data=" + dump(data));
  //layout_init("layout_div_select", "layout_form", db_divs);  
  //var remote = $(window.opener).length == 1;
  //if(remote) {
  // window.opener.$("a").click(function() { alert("Links disabled - use page selector in Edit Layout panel or close the panel"); return false; }); 
  //  $(window).unload(function() {
  //    alert("Handler for .unload() called.");
  //  });
  //}
  //$("#page_layout_panel").dialog({ height: 800, width: 500, title: "Edit Layout", open: function() {
  //$("#layout-accordion").accordion({ autoHeight: false });},beforeClose: layout_close});$(".ie_edit_link").hide();$("#page_edit_panel").hide();return false;' : '')."
  //$("#layout-panel .input").on("change", function(){
  //alert("ready: layout change");
  //layout_changed();
  //});
}



function edit_gallery_init(obj_type) {
  var $gallery = $('#gallery'), $trash = $('#trash');
  sortable_object_list($gallery, obj_type, {"container": "#gallery_frame"});

  // let the trash be droppable, accepting the gallery items
  $trash.droppable({
    accept: '#gallery > li',
    activeClass: 'ui-state-highlight',
    drop: function(ev, ui) {
      deleteImage(ui.draggable);
    }
  });

  // let the gallery be droppable as well, accepting items from the trash
  $gallery.droppable({
    accept: '#trash li',
    activeClass: 'custom-state-active',
    drop: function(ev, ui) {
      recycleImage(ui.draggable);
    }
  });

  function emptyTrashButton(correction) {
    var $was= $('li',$trash).length;
    var $now = $was + correction;
    if($now > 0) $("#gallery_empty_trash").show();
    else $("#gallery_empty_trash").hide();
  }

  // image deletion function
  var recycle_icon = '<a href="" title="Recycle this image" class="ui-icon ui-icon-refresh">Recycle image</a>';
  function deleteImage($item) {
    $item.fadeOut(function() {
      var $list = $('ul',$trash).length ? $('ul',$trash) : $('<ul class="gallery ui-helper-reset"/>').appendTo($trash);
      //alert("got here2 trash length = " + $('ul',$trash).length);
      $item.find('a.ui-icon-trash').remove();
      $item.append(recycle_icon)
        .appendTo($list)
        .fadeIn(function() {
          $item.animate({ width: '48px' })
          .find('img')
          .animate({ height: '36px' });
        });
    });
    // use ajax to set image inactive
    var img_id = $item.find('img').prop("id");				
    img_id = img_id.substr(4); // strip leaing img_
    var $url = "/ajax.php?obj_type=" + obj_type;
    //alert($url + "id=" + img_id);
    $.ajax({ url: $url, type: "POST", data: "obj_type=" + obj_type + "&oper=edit&active=0&id="+img_id});
    emptyTrashButton(1);
  }

  // image recycle function
  var trash_icon = '<a href="link/to/trash/script/when/we/have/js/off" title="Delete this image" class="ui-icon ui-icon-trash">Delete image</a>';
  function recycleImage($item) {
    $item.fadeOut(function() {
      $item.find('a.ui-icon-refresh').remove();
      $item.css('width','96px').append(trash_icon).find('img').css('height','72px').end().appendTo($gallery).fadeIn();
    });
    // use ajax to set image active
    var img_id = $item.find('img').prop("id");
    img_id = img_id.substr(4); // strip leaing img_
    $.ajax({ url: "/ajax.php?obj_type=" + obj_type, type: "POST", data: "obj_type=" + obj_type + "&oper=edit&active=1&id="+img_id});
    emptyTrashButton(-1);
  }

  // image preview function, demonstrating the ui.dialog used as a modal window
  function viewInfoDialog($item, $link) {
    var id = get_target_id($item);
    //alert(id);
    if(id) {
      var prefix = "";
      var $img = $("#img_" + id);
      $("#image_info_preview").prop("src", $img.prop("src"));
      var data = $img.data("info");
      //alert(dump(data));
      $("#" + prefix + "id").val(id);
      $.each(data, function(k, v) {
        $("#" + prefix + k).val(v);
        //alert(k + "=" + v); 
      });
    }
    var title = $img.prop("alt");
    var dialog = $("#image_info_dialog");
    setTimeout(function() {
      dialog.dialog({
          title: title,
          width: 400,
          modal: true
        });
    }, 1);

  }

  $('#gallery_empty_trash').click(function(ev) {
    if(!confirm('Are you sure you want to permanently delete the images in the trash?')) return false;
    var imgArray = $('#trash ul img'); //.sortable("toArray");
    var ids = new Array();
    $.each(imgArray, function(index, value) {
      img_id = value.id;
      img_id = img_id.substr(4); // strip leaing img_
      ids.push(img_id);
    });
    var ids_str = ids.toString();
    $.ajax({ url: "/ajax.php?obj_type=" + obj_type, type: "POST", data: "obj_type=" + obj_type + "&oper=del&id="+ ids_str });
    $('#trash ul li').remove();
    emptyTrashButton(-1);
    return false;
  });                 

  // resolve the icons behavior with event delegation
  $('ul.gallery > li').click(function(ev) {
    var $item = $(this);
    var $target = $(ev.target);

    if ($target.is('a.ui-icon-trash')) {
      deleteImage($item);
    } else if ($target.is('a.ui-icon-zoomin')) {
      viewInfoDialog($item, $target);
    } else if ($target.is('a.ui-icon-refresh')) {
      recycleImage($item);
    }

    return false;
  });
  
  $('ul.gallery > li').dblclick(function(ev) {         
//  $('ul.gallery > li').on("dblclick", function(ev) {         
    var $item = $(this);
    var $target = $(ev.target);
    viewInfoDialog($item, $target);
    return false;
  });
  imagePreview(); // initalize preview 

}

/* Back end code */

// start app.js

function search_submit() { 
  var searchForm = $('#advanced_search_form');
  for (var i = 0; i < searchForm.length; i++) {
    var sel_field;
    var search_field;
    var iname = searchForm[i].name;
    var itype  = searchForm[i].type;
    var ivalue = searchForm[i].value;      

    if(iname.indexOf('_select_')==0 && ivalue) {
      fld = iname.substr(8);
      searchfld = document.getElementById('_search_'  +fld);
      searchfld.name = '_search_' + ivalue; // set/update name of search field
    }

    if((itype!= 'submit' && itype != 'hidden') || ivalue=='' || iname=='search') { // remove all fields except add/delete buttons and hidden fields
      searchForm[i].disabled=true;
    }
          
  }

  return true;

}

// converts search operator + value1, value2 into search terms to text used in GET or POST field to search recordset
// also; enables disabled inputs based on value of preceeding inputs
function search2text(fld) { 
  var searchForm = $('#advanced_search_form');
  var field_el    = document.getElementById('_select_'  +fld);
  var operator_el = document.getElementById('_op_'  +fld);
  var value1_el   = document.getElementById('_val1_'+fld);
  var value2_el   = document.getElementById('_val2_'+fld);  

  var field    = field_el.value;
  var operator = operator_el.value;
  var value1   = value1_el.value;
  var value2   = value2_el ? value2_el.value : '';

  //alert("fld=" + fld + " op=" + operator + " val1=" +value1 + " val2=" + value2);  

  // calculate result
  if(!field || !operator) {
    result = '';
  } else if(operator == '=') {
    result = value1;
  } else if(operator == '><') {
    result =  value2 != '' ? operator + value1 + ',' + value2 : '';
  } else if(!value1) {
    result = '';
  } else {
    var result = operator + value1;
  } 

  
  // enable/disable inputs
  operator_el.disabled = field ? false : true;
  value1_el.disabled = operator && !operator_el.disabled ? false : true; 
  value2_el.disabled = operator == '><' && value1 && !value1_el.disabled ? false : true; 
  if(operator == '><' && value1 !='' && value2 !='' && value2 < value1) {
    result = '';
    alert("Value 2 must be greater than or equal to value1");
  }
  if(fld=='_new_') $('#_show_search').prop("disabled", (result ? false : true)); 

  // set the result field
  $('#_adv_search_'+fld).val(result); 
  
  alert("search2text: " + '#_adv_search_'+fld + "=" + result + " now:" + $('#_adv_search_'+fld).val());

}

// disabled all empty fields in a form
function cleanSearch(obj) {
  //obj.parent.disabled=true;
  //obj.ref_link.disabled=true;
  //obj.ref_name.disabled=true;
  for (var i = 0; i < obj.length; i++) {
    var iname = obj[i].name;
    var ivalue = obj[i].value;
    if(ivalue == '') obj[i].disabled=true;    
    if(iname.indexOf('__comp__')>-1 && ivalue=='') { // this is an empty comparison operator - disable
      obj[i].disabled=true;
    }
  }
  return true;
}

// end app.js

// for now: non-generic version of article list
function article_list(obj_type, target_id, search, options) {
  var ajax_url = '/home/ajax/cms.php?obj_type=' + obj_type + '&search=' + array2json(search) + '&options=' + array2json(options); 
  //alert(ajax_url);
  $("#" + target_id).load(ajax_url);    

  //var ajax_url = '/home/ajax/cms.php?obj_type=' + obj_type + '&search=' + array2json(search) + '&limit=' + options.limit + '&offset=' + options.offset;
  // alert(dump(search));
  //alert("ot=" + obj_type + " t=" + target_id + " limit=" + options.limit + " offset=" + options.offset + " " + array2json(search));
}

// tbc: generic function for listing articles
// function article_list(div_id, obj_type, search, sort, fields, limit, offset, handler) {  
//   $("#" + div_id).load(ajax_url);    
// }

// same as find_objects in back-end
function find_objects(obj_type, search, sort, fields, limit, offset, handler) {  
  ajax_url = "/" + (unauthenticated ? "home" : "admin") + "/app_data.php?oper=list&obj_type=" + obj_type + "&id=" + id + "&reload=" + reload_bool;

  if(search) ajax_url += "&search=" + array2json(search);
  if(fields) ajax_url += "&fields=" + array2json(fields);
  if(sort) ajax_url += '&sort=' + sort;
  if(limit) ajax_url += '&limit=' + limit;
  if(offset) ajax_url += '&offset=' + offset;
  if(handler) ajax_url += '&handler=' + handler;

  alert(ajax_url);
  $dialog.load(ajax_url, data, function (responseText) {
    alert(dump(responseText));
    //.ckeditor();
  });                                      
}

function exception_script(params) {
  $("#city_id").closest(".fieldset_div").hide();
  $("#apt_id").css("border", "4px solid red");
  $("input[name=apt_id]").on("change",function() {
   var val = parseInt($(this).val());
   switch(val) {
     case 0:
       $("#city_id").closest(".fieldset_div").show();
       break;
     default: 
       $("#city_id").closest(".fieldset_div").hide();
       break;
    }
  });
}

$(document).ready(function() {
  cms_ready_script();
});

function cms_ready() {
  console.log("cms ready success handler");
  cms_ready_script();
}

function cms_ready_script() {
  $sortable = $(".sortable");
  //alert('cms ready sortable count = ' + $sortable.length);
  
  $(".sortable").each(function() {
    var $target = $(this);
    var options = $target.data('options') || {};
    var id = $target.prop('id') || $target.parent().prop('id');
    var handler = options.handler || $target.data('handler') || '';
    var obj_type = options.obj_type || $target.data('obj_type');
    //alert("making " + id + " sortable ot=" + obj_type);
    if(obj_type) {
      $target.sortable({
        containment: 'document', //id,
        cursor: "move",
        update: function(event, ui) {
          var imgArray = $(this).sortable("toArray");
          var ids = new Array();
          var ranks = new Array();
          $.each(imgArray, function(index, value) {
            id = value.split("-").pop();
            rank = parseInt(index) + 1;
            ids.push(id);
            ranks.push(rank);
          });
          var ids_str = ids.toString();
          var ranks_str = ranks.toString();
          //alert("ot=" + obj_type + ' ' + ids_str + '=' + ranks_str);
          $.ajax({ url: "/ajax.php?obj_type=" + obj_type, type: "POST", data: "obj_type=" + obj_type + "&oper=edit&data_field=rank&id="+ ids_str + "&rank="+ranks_str });
        }
      });
    }
  });
    
  fancybox_loader();
  //loadEditors();
}


// gmenu handlers
$(document).on("click", ".gshow", function() {
//$(".gshow").on("click", function() {
  var $checked = $('.gshow:checkbox:checked');
  var count = $checked.length;
  if(count) {
    $('.gmenu .ghide').show('fast');
  } else {
    $('.gmenu .ghide').hide('fast');
  }
});

$(document).on("click", "LI.check_all A", function() {
//$("LI.check_all A").on("click", function() {
  var target_selector = $(this).closest("UL").data('target');
  var $target = $(target_selector);
  $target.find(':checkbox').prop('checked', true);
  $('.gmenu .check_on_off').prop('checked', true);
  $('.gmenu .ghide').show('fast');
  return false;
});

$(document).on("click", "LI.check_none A", function() {
//$("LI.check_none A").on("click", function() {
  var target_selector = $(this).closest("UL").data('target');
  var $target = $(target_selector);
  $target.find(':checkbox').prop('checked', false);
  $('.gmenu .check_on_off').prop('checked', false);
  $('.gmenu .ghide').hide('fast');
  return false;
});

$(document).on("click", "INPUT:checkbox.check_on_off", function() {
//$("INPUT:checkbox.check_on_off").on("click", function() {
  var target_selector = $(this).data('target');
  var $target = $(target_selector);
  var $checkbox = $(this);
  var checked = $checkbox.prop("checked");
  if(checked) {
    $('.gmenu .ghide').show('fast');
  } else {
    $('.gmenu .ghide').hide('fast');
  }
  $target.find(':checkbox').prop('checked', checked);    
});

$(document).on("click", "UL.add_to LI A, UL.move_to LI A", function() {
  $item = $(this).parent(); // the List Item   
  var $list = $item.parent();
  var data = $list.data('data');
  var target_selector = $list.data('target');
  var handler = $list.data('handler');
  var operation = $list.data('operation');
  var $target = $(target_selector);
  var ids = get_checked_ids(target_selector);
  
  //alert("ts=" + target_selector + " ids=" + ids);
  if(!ids) {alert('Nothing selected'); return false;}  
  data.collection_id = $item.data('collection_id');
  data.gallery_id = $item.data('gallery_id');
  data.media_id = ids;
  var ajax_url = "/ajax.php?oper=" + operation; // + '&' + obj2qs(data);
  var params = {};
  params.url = ajax_url;
  params.data = data;
  params.handler = handler;
  ajaxSubmit(params);
  return false;
}); 

$(document).on("click", ".gmenu LI A.gshare, .gmenu LI A.publish, .gmenu LI A.exhibit, .gmenu LI A.gdelete, .gmenu LI A.gundelete", function() {
  var $link = $(this); // the anchor
  var $item = $link.parent(); // the List Item       
  var $list = $item.parent(); // the gmenu UL
  var options = $link.data('options') || {};
  var data = $link.data('data') || {};
  var ids = 0;
  
  //alert(dump(options));
  var obj_type = options.obj_type || data.obj_type || '';
  var action = options.action || data.action || '';

  if(!action) {
    alert("missing action");
    return false;
  }
  if(!obj_type) {
    alert("cannot " + action + ": missing object type");
    return false;
  }
  var handler = options.handler || $list.data('handler') || '';
  var operation = options.operation || $list.data('operation');
  var target_selector = options.target || $list.data('target') || '';
  if(target_selector) ids = get_checked_ids(target_selector);
  else ids = options.id || data.id;
  

  //var options = {"handler": handler, "operation": operation};
  if(obj_type && ids) {
    //alert("target= " + target_selector + " checked=" + ids + " handler=" + handler + dump($list.data('data')));return;
    switch(action) {
    case 'recycle':
      var data = {"data_field": "active", "active" : 0};
      update_object(obj_type, ids, options, data);
      return false;
    case 'restore':
      var data = {"data_field": "active", "active" : 1};
      update_object(obj_type, ids, options, data);
      return false;
    case 'share':
      operation = 'new-sharing';
      options.operation = operation;
      var id = ids;
      options.data = data;
      //alert(dump(data));
      object_dialog(obj_type, id, options);      
      return false;
    case 'delete':
      if(confirm("Are you sure you want to permanently delete " + obj_type + " " + ids + "?")) delete_object(obj_type, ids, options);
      break;
    case 'publish':
      data.id = ids;
      //switch(obj_type) {
      //case 'gallery':
      //  data.gallery_id = ids;
      //  break;
      //case 'media_collection':        
      //  data.media_collection_id = ids;
      //  break;
      //default:
      //  alert("Cannot publish " + obj_type);
      //  return;
      //}
      return false;
    default:
      alert("Unknown action " + action);
      return false;
    }
  }

  if(!operation) {
    alert("No operation" + dump(options));
    return false;
  }

  alert("click ac=" + action + ' ot=' + obj_type + ' ts=' + target_selector + ' ids=' + ids + dump(options));

  //alert(dump(data));
  var ajax_url = "/ajax.php?oper=" + operation; // + '&' + obj2qs(data);
  console.log("going to: " + ajax_url + ' data=' + dump(data));
  var params = {"url": ajax_url, "data": data, "handler": handler};
  ajaxSubmit(params);
  
  return false;
}); 

$(document).on("click", ".gmenu LI A.grotate", function() {
//$(".gmenu LI A.grotate").on("click", function() {
  $link = $(this); // the anchor
  $item = $link.parent(); // the List Item       
  var $list = $item.parent(); // the gmenu UL
  //alert(dump(options));
  var options = $link.data('options') || {};
  
  var obj_type = options.obj_type || $link.data('obj_type') || '';
  if(!obj_type) {
    alert("cannot delete: missing obj_type " + dump(options));
    return;
  }
  var target_selector = options.target || $list.data('target') || '';
  var handler = options.handler || $list.data('handler') || '';
  var operation = options.operation || $list.data('operation');
  var ids = get_checked_ids(target_selector);
  var degrees = options.degrees || $list.data('degrees');
  var idAr = ids.split(',');
  //var options = {"handler": handler, "operation": operation};
  if(obj_type && ids) {
    //alert("target= " + target_selector + " checked=" + ids + " handler=" + handler + dump($list.data('data')));return;
    //if(confirm("Are you sure you want to permanently delete " + obj_type + " " + ids + "?")) delete_object(obj_type, ids, options);
    var ajax_url = '/ajax.php?oper=load-function&function=rotate_media&param1=' + ids + '&param2=' + degrees;
    //alert("Rotate " + ids + " " + degrees + " url=" + ajax_url); return false;
    
    var $messages = $('#ajax_messages');
    var $errors = $('#ajax_errors');

    jQuery.ajax({
      type: "get",
        url: ajax_url,
        success: function (responseText) {
          var error = '';
          var message = responseText;
          if(error) $errors.html(error).parent().show();
          if(message) $messages.html(message).parent().show();
          var prefix = options.prefix || 'media-item-';
          $.each(idAr, function(index, id) {
            var selector = '#' + prefix + id;
            $div = $(selector);
            //  alert(selector + ' div l=' + $div.length);
            if($div.length) {
              $img = $div.find('IMG:first');
              //alert(selector + ' img l=' + $img.length);
              if($img.length) {
                var src = $img.prop('src');
                //$div.fadeOut(300, function() { $(this).remove(); });
                //alert("selector=" + selector + " length=" + $div.length + ' src=' + src);
                var new_src = src+'?random';
                $img.prop('src', new_src);
              }
            }
          });
          
          //alert(responseText);
        }
    });
    
  }
  
  return false;
}); 


// handler for generic add_object link/button
$(document).on("click", ".add_object, .edit_object, .add-object, .edit-object, .view-object", function() {
  $target = $(this);
  //ale
  var reload = $target.data('reload') || 0;
  var reload_template = $target.data('reload_template') || '';
  var reload_link = $target.data('reload_link') || '';
  var reload_target = $target.data('reload_target') || '';
  if(reload_target && !$(reload_target).length) {
    alert("reload target " + reload_target + " not found");
    reload_template = '';
  }
  if(reload_template && !reload_target) {
    alert("reload template set, but no target");
    reload_template = '';
  }
  var handler = $target.data('handler') || '';
  var success_handler = $target.data('success_handler') || '';
  var fields = $target.data('fields') || [];
  var data = $target.data('data') || {};
  var handler_data = $target.data('handler_data') || {};
  var obj_type = $target.data('obj_type') || data.obj_type;
  var form_class = $target.data('form_class');
  var id = data.id || $target.data('obj_id') || 0;
  
  if(id && !reload_template && !reload_link) reload = 1;
  
  var options = {'fields': fields, 'data': data, 'handler' : handler,  'handler_data': handler_data, 'success_handler' : success_handler, 'reload': reload, 'reload_template': reload_template, 'reload_target': reload_target, 'reload_link': reload_link, 'form_class': form_class};
  if($target.hasClass("view-object")) options.readonly = 1;
  
  var more_options = $target.data('options');
  if(more_options) jQuery.extend(options, more_options);
  console.log("add/edit object " + obj_type + " " + id + " fields=", fields);  
  console.log("options=",options);
  //alert(dump(data));
  
  object_dialog(obj_type, id, options);                              
  return false;
});


function add_contact_handler(params, data) {
  console.log("Contact added params:", params, " data:", data);
}


$(document).on("click", ".css_save", function() {
  if($(this).is(':disabled') == true) {
    alert("css_save is disabled");return false;
  }
  var parent_id = $(this).parent().data("parent_id");
  //alert("css_save parent=" + parent_id);
  save_layout(parent_id);return false;
});

$(document).on("click", ".css_cancel", function() {
  var parent_id = $(this).parent().data("parent_id");
  //alert("css_cancel parent=" + parent_id);
  layout_reset(parent_id);return false;
});

$(document).on("change", ".css_select", function() {
  var parent_id = $(this).parent().data("parent_id");
  var selectors = $(this).find(":selected").data("selectors");
  if(selectors) {
    var css_selectors = get_selectors(selectors);
    g_css_selectors = css_selectors; // save to global    
    create_accordion(parent_id, css_selectors);
  }
  //alert("sel:" + dump(selectors));
  //alert("css_select parent=" + parent_id + " div=" + this.value);
  layout_select_div(parent_id, this.value); 
});


$(document).on("change", ".css_page_select", function() {
//$('#page_id').on('change', function() {
  alert("Fix this: change css page");
  //var $option = $("#page_id option:selected");   
  var link = $(this).find(":selected").data('link');
  layout_select_page(parent_id, this.value, link);
});    

// bind handlers to buttons Add Page/Add Folder
$(document).on("click", ".add_node", function() {
  $target = $(this);
  var handler = $target.data('handler');
  var parent_id = 0;
  var is_folder = 0;
  if(handler == 'dynatree_save') { // add node in site-edit (dynatree)
    var $tree = $('#pages_tree');
    var active = $tree.dynatree("getActiveNode");  
    var site_id = $tree.data('site_id');
    is_folder = $(this).prop("id") == "add_folder" ? 1 : 0;
    if(active) {
      var parent_key = active.data.key;
      parent_id = parent_key.split('_').pop();      
    }
  } else if(handler == 'cms_save') { // this for add_node on page
    var link_id = $target.attr("id");
    var site_id = $target.data('site_id');
    var parent_id = $target.data('parent_id');
  } else {
    return false;
  }
   
  var page_fields = ['site_id', 'parent_id', 'name', 'page_type', 'show_menu', 'show_footer', 'localized', 'is_folder'];
  var data = {"site_id": site_id, "is_folder": is_folder, "parent_id": parent_id};
  //alert(dump(data));
  object_dialog('site_page', 0, {'fields': page_fields, 'data': data, 'handler' : handler});                                      
  return false;
});

// csm add/edit handlers

$("input").on("change", function() { 
  formChangedFlag = "X"; 
});   

//$("non-html").css("border", "2 px solid red");
$(document).on("click", ".__edit, .__add", function() {
  var $target = $(this);
  //if(!$target.hasClass("_selected")) return;
  //$target.removeClass("_selected");
  //$(".ie_edit_link, .ie_edit_text, #page_edit_panel, #site_edit_button").hide();
  
  var el_id = $(this).attr("id");
  if(!el_id) {
    alert("Cannot edit element, missing ID");
    return;
  }
  var parts = el_id.split("-");
  var action    = parts[0];
  var obj_type  = parts[1];
  var field     = parts[2];
  var id        = parts[3];
  var site_id   = parts[4];
  var page_id   = parts[5];
  var lang      = parts[6];
  var dummy     = parts[7]; // used to make add_id unique
  var type      = parts[8] || 0;
  var data = {"site_id" : site_id, "page_id": page_id, "language": lang};
  
  var moreData = $("#" + el_id).data("data");
  if(moreData) data = $.extend({}, data, moreData);

  var class_options = $("#" + el_id).data("class_options");
  //if(options) data = $.extend({}, data, options);

  var fields = field.split("|");     

  console.log("__edit click on id=" + el_id);
  console.log(class_options);
  var handler_data = {"target": el_id};
  var dialog_options = {"fields": fields, "type": type, "target": el_id, "handler_data": handler_data, "data": data, "handler": "reload_page_element", "reload": 0, "unauthenticated": 0};
  if(class_options) dialog_options.class_options = class_options;
  object_dialog(obj_type, id, dialog_options);
  
  return false;
});

function reload_page_element(params, data) {
  console.log("reload_page_element " + target + " params=",params);
  console.log("data=", data);
  var target = params.target;
  var $target = $("#" + target);
  if($target.length) {
    $target.html(data.content);
    close_parent_dialog(target);    
  } else {
    alert("couldn't find target " + target);
  }
  
}


// below used by jqgrid for image upload
function UploadImage(field_name, field_id, options, response, postdata) {
  var $field = $("#" + field_id);
  console.log("UploadImage: obj_field=" + field_name + " field=" + field_id + " len=" + $field.length);
  var val = $field.length ? $field.val() : '';
  
  var result = $.parseJSON(response.responseText);
  if (result.success == true) {
    console.log("OK: Field val=" + val);
    console.log(result);
    if (val != "") {
      var data = jQuery.parseJSON(result.data)
      data.options = options;
      data.obj_field = field_name;
      ajaxFileUpload(field_id, data);
    }
  }  
  return [data.success, data.message, data.id];
}

function ajaxFileUpload(field_id, data) {
  console.log("ajaxFileUpload:  id=" + field_id + " data=");
  console.log(data);
  
  
  $("#loading")
  .ajaxStart(function () {
    $(this).show();
  })
  .ajaxComplete(function () {
    $(this).hide();
  });

  $.ajaxFileUpload
  (
    {
      url: '/ajax.php?oper=file-upload',
      secureuri: false,
      fileElementId: field_id,
      dataType: 'json',
      data: data,
      success: function (data, status) {
        console.log("Returned from file_upload with status=" + status + " data=");
        console.log(data);
        if (typeof (data.success) != 'undefined') {
          if (data.success == true) {
            alert("yup");
            return;
          } else {
            alert(data.message);
          }
        }
        else {
          return alert('Failed to upload image!');
        }
      },
      error: function (data, status, e) {
        return alert('Failed to upload image!');
      }
    }
  )
}  
    
//hotkey_edit(); // push key (ctrl) to show edit icons


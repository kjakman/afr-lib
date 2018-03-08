<?php

// new recordset based on jqGrid
function jqGrid($obj_type, $filters = array(), $options = array()) {
  global $__header;
  //$__header['scripts'][]= '/jquery/jqGrid/js/jquery.jqGrid.min.js';
  //$__header['scripts'][]= '/jquery/jqGrid/js/i18n/grid.locale-'.strtolower($_SESSION['language']).'.js';
  //$__header['css'][]= '/jquery/jqGrid/css/ui.jqgrid.css';

  $__header['scripts'][]= '/afr/bower/jqgrid/js/minified/jquery.jqGrid.min.js';
  $__header['scripts'][]= '/afr/bower/jqgrid/js/i18n/grid.locale-'.strtolower($_SESSION['language']).'.js';
  //$__header['scripts'][]= '/jquery/ajaxfileuploader/ajaxfileupload.js';
  $__header['css'][]= '/afr/bower/jqgrid/css/ui.jqgrid.css';
  
  //$__header['css'][]= '/afr/css/layout-jqgrid.css';
  
  $table_id = 'jqGrid_table_'.$obj_type;
  $pager_id = 'jqGrid_pager_'.$obj_type;
  $obj_name = get_object_name($obj_type);
  
  $flds = array();
  $errors='';
  $classFlags = get_object_flags($obj_type);
  $expanded_listing = $classFlags & NF_COMPACT_LIST ? 0 : 1;
  $classVars = get_object_attributes($obj_type, $expanded_listing);

  // node actions
  $class_def        = get_class_def($obj_type, true);
  $key              = $class_def->key;
  $descriptor_field = $class_def->descriptor;
  $table            = $class_def->table;
  $class_flags      = $class_def->flags;
  $this_type        = $class_def->name;
  $show_rowbuttons  = !($classFlags & NF_NO_ACTIONS);
  $show_custom      = false;

  $col_names    = array();
  $fields       = array();
  $attributes   = array();

  // class level auth
  $can_edit = 'true'; // todo: implement
  $can_add  = 'true';
  $can_del  = 'true';
  $can_view = 'true';
  //$caption = "$this_type records $first_record - $last_record of $count";  
  //$caption = "$this_type records ($count)";
  $caption = "$this_type records";
  $select_types = array();  
  $show_total = false;
  $custom_edit_handlers = '';
  //dump($filters);
  foreach($classVars as $k => $class) {
    if(!is_array($class)) continue;    
    $att_flags = $class[ATT_FLAGS];
    if($att_flags & AF_HIDE) continue; // this field is hidden or hidden in list view
    $hidden = $att_flags & AF_HIDE_LIST;

    list($fld, $sql_fld) = parse_field($table, $k); // converts class.class.class.field to array(field, table.field)
    $fld_name = $class[ATT_NAME];
    $fld_type = $class[ATT_TYPE];
    $rel_type = $class[ATT_RELATION_TYPE];

    if(in_array($fld_type, array('one2one', 'one2many'))) continue;
		$no_search = $att_flags & AF_NO_SEARCH; // todo: implement (searchable by default)
		$no_sort = $att_flags & AF_NO_SORT;
		$no_edit = $att_flags & AF_READONLY || $att_flags & AF_READONLY_EDIT || $att_flags & AF_HIDE_EDIT || $fld == $key;
		$no_add  = $att_flags & AF_READONLY || $att_flags & AF_READONLY_ADD || $att_flags & AF_HIDE_ADD || $fld == $key;
    $required = $att_flags & AF_OBLIGATORY;
		if($att_flags & AF_TOTAL) $show_total = true;
    
    //$searchable = $att_flags & AF_SEARCAHBLE;
    $searchable = true;

    $fields[] = $fld;
    $col_names[] = "'$fld_name'";

    // Col Model: field options
    $options = array();
    $options['name']  = "'$fld'";
    $options['index'] = "'$sql_fld'";
    $options['sortable'] = $no_sort ? 'false' : 'true';
                                
    if($fld_type == 'id') $options['key'] = 'true';
    if(in_array($fld_type, array('file', 'image', 'video', 'audio', 'media'))) {
      $uploader = object_media_uploader($obj_type, 0, $fld);
      $custom_edit_handlers .= jqGridUploader($obj_type, $fld, $uploader);
    }

    if($hidden) {
      $options['hidden'] = 'true';
      $edit_rules['edithidden'] = 'true';
    }

    $value = '';
    if($filters[$fld]) { // only works for string or integer for now todo: fix
      $no_edit = true;
      //$value = $filters[$fld];
    }
    
    if($can_edit && !$no_edit) $options['editable'] = 'true';

    
    $options = jqGrid_field_options($fld, $options, $obj_type, $class, $value);
    //dump($options);
    // $searchable = false;
    
    $searchable = $att_flags & AF_SEARCHABLE;
    if(!$searchable) {
      $options['search'] =  "false";
      unset($options['searchoptions']);
    }

    $attributes[] = jquery_options($options);
    
    // load select box data using Ajax if editoptions are not already set (will be added to JS run after load)
    if($options['edittype'] == "'select'" && !$options['editoptions']) {
      $load_complete .= "$(\"#".$table_id."\").setColProp('".$fld."', { editoptions: { value: jqGrid_select_".$rel_type."} });\n";
      $select_types[$rel_type] = array($class[ATT_FILTERS], $class[ATT_SORT]);
    }
  }
   
  $col_model = implode(",\n", $attributes);

  $sort_field = $fields[0];  
  $limit = 50;

  // Build Javascript
  //$app_link = get_page_link(PAGE_ADMIN) ?: '/admin/app.php';

  $url = "/admin/app_data.php?oper=grid-list-old&obj_type=$obj_type";
  if($filters) $url .= "&search=".urlencode(json_encode($filters));
  //$url = "/admin/app_data.php?oper=grid-list&obj_type=$obj_type";
  $edit_url = "/admin/app_data.php?oper=edit&obj_type=$obj_type";
  $add_url = "/admin/app_data.php?oper=add&obj_type=$obj_type";
  $del_url = "/admin/app_data.php?oper=del&obj_type=$obj_type";

  if($filters) $add_url .= "&".http_build_query($filters);
  
  // begin javascript
  $script = '';
  if(count($select_types)) foreach($select_types as $select_type => $sel_options) {
    $script .= jqGrid_select($select_type, $sel_options[0], $sel_options[1]);
  }
  
  $script .= '
  var table_id = "'.$table_id.'";
  var $table = $("#'.$table_id.'");
  var $pager = $("#'.$pager_id.'");
  var lastsel;
  var inEdit = true; // are we editing (or adding)
  var currentsel;
  var uploader_init = 0;
  function pickdates(id) { 
    jQuery("#"+id+"_sdate",$table).datepicker({dateFormat:"yy-mm-dd"});
  } 

  function passwordcheck(value, colname) {
    if (value.length < 6) return [false, "Please enter at least 6 characters"];
    return [true,""];
  }

  function jgImageFormatter (cellValue, options, rowObject) {
    //console.log("jgImageFormatter value=" + cellValue);
    return cellValue;
  }
  
  function colorFormatter(cellValue, options, rowObject) {
  	var brightness = color_brightness(cellValue);
  	var fg = brightness < 0.5 ? "#FFFFFF" : "#000000";
    return "<div class=\'colorpicker_input\' style=\'background-color:#" + cellValue + ";color:" + fg + ";width:75px\'>" + cellValue + "</div>";
  }

  function unFormatter(cellValue, options, cellObject) {
    return cellValue;
  }
  
  function color_picker(value, options) {
  	var brightness = color_brightness(value);
  	var fg = brightness < 0.5 ? "#FFFFFF" : "#000000";
  	var output = "#<input class=\'colorpicker_input\' type=\'text\' maxlength=\'6\' value=\'" + value + "\' style=\'color:" + fg + ";background-color:#" + value + ";width:75px;\' />";
  	return output;
  }
  
  function color_value(value) {
  	return value.val();
  	return colorFormatter(value.val());
  }
              
  function myelement(value, options) {
    return $(\'<input type="text" value="\' + value + \'" disabled="disabled"/>\');
  }
  
  function myvalue(elem, operation, value) {
    return elem.val();    
    //console.log("myvalue oper=" + operation + " value=" + value);
    //if(operation === "get") {
    //  return "myval GET " + value;
    //   return $(elem).val();
    //} else if(operation === "set") {
    //  return "myval SET " + value;
    //   $("input",elem).val(value);
    //}
  }
  
  // look for file uploader in row
  
  function jqFindRowFile(index) {
  }

  jQuery.extend($.fn.fmatter , {
      currencyFmatter : function(cellvalue, options, rowdata) {
      return "€"+cellvalue;
  }
  });
  jQuery.extend($.fn.fmatter.currencyFmatter , {
      unformat : function(cellvalue, options) {
      return cellvalue.replace("€","");
  }
  });
          
  var formEditingOptions = {
    width: 800,
    errorTextFormat: function (response) {
      var responseText = parse_json(response.responseText);
      var errorText = responseText.error || responseText.message || responseText;
      console.log("add/edit error");
      console.log(responseText);
      return errorText;
      return \'<span class="ui-icon ui-icon-alert" style="float:left; margin-right:.3em;"></span>\' + errorText;
    },
    afterSubmit : function(response, postdata) { 
      return [true, ""];
    },
    //recreateForm: true,
    closeAfterEdit: true,
    beforeShowForm: function(formId) {
      //alert("hey");
    },
    afterShowForm: function($form) {
      
      var $loader = $form.find(".jquery-fileupload");
      var loader_id = $loader.attr("id");

      if($loader.length) {
        var mediaData = $loader.data("media_data");
        mediaData.parent_id = currentsel;
        $loader.data("media_data", mediaData);
        console.log("afterShowForm with uploaders: edit=" + inEdit);
        if(inEdit) $loader.closest("TR").show();
        else $loader.closest("TR").hide();
        
        if(!uploader_init) {
          console.log("jqgrid script: calling init_fileuploader for " + $loader.attr("id"));
          init_fileuploader($loader);
          uploader_init = 1;
        } else {
          jfu_load($("#" + loader_id), mediaData);
        }
      }
      return [true, ""];
      
    },
    edittitle: "'.phrase('edit',CAPITALIZE).' '.$obj_name.'"
  }
  var editOptions = jQuery.extend(true, {}, formEditingOptions); // deep copy
  editOptions.url = "'.$edit_url.'";
  editOptions.closeAfterEdit = true;
  editOptions.beforeInitData = function() {
    inEdit = true;
  }

  var addOptions = jQuery.extend(true, {}, formEditingOptions); // deep copy
  addOptions.url = "'.$add_url.'";
  addOptions.beforeInitData = function() {
    inEdit = false;
  }
  //alert(dump(editOptions));

  var delOptions = jQuery.extend(true, {}, formEditingOptions); // deep copy
  delOptions.url = "'.$del_url.'";
  
  $table.jqGrid({     
    url: "'.$url.'", 
    datatype: "json",
    mtype: "GET",
    colNames: ['.implode(",", $col_names).'], 
    colModel: ['.$col_model.'],
    rowNum: '.$limit.', 
    
		rowNum : 25,
		rowList : [10, 25, 50, 100],
		toolbarfilter: true,
		viewrecords : true,
		sortorder : "asc",

    pager: "#'.$pager_id.'", 
    viewrecords: true, 
    caption: "'.$caption.'",
    sortName: "'.$sort_field.'", 
    sortOrder: "desc", 
    sortable: true, 
    autowidth: true, 
    rownumbers: false, 
    height:600,
    forceFit: false, 
    // cellEdit: '.$can_edit.', 
    // cellurl: "'.$edit_url.'", 
    editurl: "'.$edit_url.'", 
    serializeRowData: function(postData) {
      console.log("serialize lastsel=" + lastsel);
      console.log(postData);
      $table.jqGrid("restoreRow",lastsel);
      return postData;
    },
	  altRows: true,';

  if($show_total) $script .= '
    footerrow: true,
	  userDataOnFooter: true,';       

  $script .= '     
	onSelectRow: function(id) {
	  currentsel = id; // store global for use in custom edit function
		if(id && id!==lastsel) {
			$table.jqGrid("restoreRow",lastsel);
			$table.jqGrid("editRow",id,true,function(res) {
  			//$table.jqGrid("restoreRow",id);
			  console.log("Done edit id=" + id + " last=" + lastsel);
			  console.log(res);
			});
			lastsel=id;
		}
	},   
	gridComplete: function() { // icons per row - not in use
	  return;
    var ids = jQuery("#jqgrid").jqGrid(\'getDataIDs\');
    for(var i=0;i < ids.length;i++){
      var cl = ids[i];
      be = "<button class=\'btn btn-xs btn-default btn-quick\' title=\'Edit Row\' onclick=\"jQuery(\'#jqgrid\').editRow(\'"+cl+"\');\"><i class=\'fa fa-pencil\'></i></button>"; 
      se = "<button class=\'btn btn-xs btn-default btn-quick\' title=\'Save Row\' onclick=\"jQuery(\'#jqgrid\').saveRow(\'"+cl+"\');\"><i class=\'fa fa-save\'></i></button>";
      ca = "<button class=\'btn btn-xs btn-default btn-quick\' title=\'Cancel\' onclick=\"jQuery(\'#jqgrid\').restoreRow(\'"+cl+"\');\"><i class=\'fa fa-times\'></i></button>";  
      jQuery("#jqgrid").jqGrid(\'setRowData\',ids[i],{act:be+se+ca});
    }	
  },
		
  ondblClickRow: function(rowid, iRow, iCol, e) {
    inEdit = true;
    $(this).jqGrid("editGridRow", rowid, formEditingOptions);
    e.stopPropagation();
  }, 
	loadComplete: function() {
      debug("loaded jqGrid");
      '.$load_complete.'  
    },
  });
  var navOptions = {edit:'.$can_edit.',add:'.$can_add.',del:'.$can_del.',view:'.$can_view.',search:true,refresh:true};
  var searchOptions = {closeOnEscape: true, multipleSearch: true, closeAfterSearch: true};
  var viewOptions = {};

  $table.jqGrid("navGrid", "#'.$pager_id.'",
    navOptions,
    editOptions, // edit options 
    addOptions, // add options 
    delOptions, // del options 
    searchOptions, // search options
    viewOptions
  );   
  $table.jqGrid("gridResize",{minWidth:350,maxWidth:1920,minHeight:80, maxHeight:1000});
  $table.jqGrid("sortableRows");
  $table.jqGrid("filterToolbar"); 
  if(0) {
    $table.jqGrid("navButtonAdd",$pager,{ caption: "Columns", title: "Reorder Columns", 
      onClickButton : function () {
        $table.jqGrid("setColumns"); 
        //jQuery($pager).jqGrid("columnChooser"); 
      } 
    }); 
  
    $table.jqGrid("navButtonAdd",$pager,{caption:"Toggle",title:"Toggle Search Toolbar", buttonicon :"ui-icon-pin-s", 
      onClickButton:function() { 
        //$table.toggleToolbar() 
      } 
    }); 
    $table.jqGrid("navButtonAdd",$pager,{caption:"Clear",title:"Clear Search",buttonicon :"ui-icon-refresh", 
      onClickButton:function() { 
        //$table.clearSearch() 
      } 
    });  
  }
    
  if(bootstrap_enabled) {
    /* Add tooltips */
    jQuery(".navtable .ui-pg-button").tooltip({
      container : "body"
    });
  
    console.log("Bootstrap enabled, using font awesome icons");
    jQuery(".ui-jqgrid").removeClass("ui-widget ui-widget-content");
    jQuery(".ui-jqgrid-view").children().removeClass("ui-widget-header ui-state-default");
    jQuery(".ui-jqgrid-labels, .ui-search-toolbar").children().removeClass("ui-state-default ui-th-column ui-th-ltr");
    jQuery(".ui-jqgrid-pager").removeClass("ui-state-default");
    jQuery(".ui-jqgrid").removeClass("ui-widget-content");
  
    jQuery(".ui-jqgrid-htable").addClass("table table-bordered table-hover");
    jQuery(".ui-pg-div").removeClass().addClass("btn btn-sm btn-primary");
    jQuery(".ui-icon.ui-icon-plus").removeClass().addClass("fa fa-plus");
    jQuery(".ui-icon.ui-icon-pencil").removeClass().addClass("fa fa-pencil");
    jQuery(".ui-icon.ui-icon-trash").removeClass().addClass("fa fa-trash-o").parent(".btn-primary").removeClass("btn-primary").addClass("btn-danger");;
    jQuery(".ui-icon.ui-icon-search").removeClass().addClass("fa fa-search");
    jQuery(".ui-icon.ui-icon-refresh").removeClass().addClass("fa fa-refresh");
    jQuery(".ui-icon.ui-icon-disk").removeClass().addClass("fa fa-save").parent(".btn-primary").removeClass("btn-primary").addClass("btn-success");
    jQuery(".ui-icon.ui-icon-cancel").removeClass().addClass("fa fa-times").parent(".btn-primary").removeClass("btn-primary").addClass("btn-danger");
    jQuery(".ui-icon.ui-icon-document").removeClass().addClass("fa fa-eye").parent(".btn-primary").removeClass("btn-primary").addClass("btn-success");
  
    jQuery( ".ui-icon.ui-icon-seek-prev" ).wrap( "" );
    jQuery(".ui-icon.ui-icon-seek-prev").removeClass().addClass("fa fa-backward");
  
    jQuery( ".ui-icon.ui-icon-seek-first" ).wrap( "" );
    jQuery(".ui-icon.ui-icon-seek-first").removeClass().addClass("fa fa-fast-backward");		  	
  
    jQuery( ".ui-icon.ui-icon-seek-next" ).wrap( "" );             
    jQuery(".ui-icon.ui-icon-seek-next").removeClass().addClass("fa fa-forward");
  
    jQuery( ".ui-icon.ui-icon-seek-end" ).wrap( "" );
    jQuery(".ui-icon.ui-icon-seek-end").removeClass().addClass("fa fa-fast-forward");
  }
  ';
  $script .= $custom_edit_handlers;
  
  
  // begin output  
  $output .=  "<table class='jqgrid-table' id='$table_id'></table>\n";   
  $output .= "<div class='jqgrid-pager' id='$pager_id'></div>\n";   
  $output .= html_script_inline($script);
  return $output;
}

function jqGrid_select($obj_type, $filters='', $sort='') {
  $select_url = "/admin/app_data.php?oper=grid-select&obj_type=$obj_type&format=jq_select";
  if($filters) $select_url .= "&select_filters=".urlencode(seaarch2sql($filters));
  if($sort) $select_url .= "&select_sort=".urlencode($sort);
  $script = "
    //get all $obj_type
    var jqGrid_select_".$obj_type." = $.ajax({url: '".$select_url."', async: false, success: function(data, result) {
      if (!result) alert('Failure to retrieve $obj_type data.');}}).responseText;
  ";
  return $script;
}

/*
http://www.secondpersonplural.ca/jqgriddocs/index.htm
The predefined types are
    *  integer
    * number
    * currency
    * date (uses formats compatable with php function date. For more info visit www.php.net)
    * checkbox
    * mail
    * link
    * showlink
    * select (this is not a real select but a special case for editing modules. See note below)
*/

function jqGrid_field_options($fld, $field_options, $obj_type, $class, $value='') {
  $fld_type = $class[ATT_TYPE];
  $rel_type = $fld_options = $class[ATT_RELATION_TYPE];
  $fld_options = $class[ATT_OPTIONS];
  //dump($class);die();
  $editRulesAr = array();
  if($value) $fld_type = 'str';
  // field options
  switch($fld_type) {
    case 'email':
      $field_options['formatter'] = "'mail'"; 
      $search_options = "sopt:['eq','ne']";
      $editRulesAr[] = "email:true";
      $field_options['width'] = 50;
      $field_options['sorttype'] = "'text'";
      break;
    case 'dummy':
      $field_options['width'] = 50;
      break;
    case 'date':
      $field_options['formatter'] = "'date'"; 
      $format_options = "srcformat:'Y-m-d',newformat:'d-M-Y'";
      $search_options = "sopt:['eq','ne','lt','le','gt','ge']";
      $field_options['sorttype'] = "'date'";
      $field_options['width'] = 50;
      $edit_options = "size: 10, maxlengh: 10,
      dataInit: function(element) {
        $(element).datepicker({dateFormat: 'yy-mm-dd'})
      }";
      break;
    case 'datetime':
      $field_options['formatter'] = "'date'"; 
      $format_options = "srcformat:'Y-m-d H:i:s',newformat:'d-M-Y H:i'";
      $search_options = "sopt:['eq','ne','lt','le','gt','ge']";
      $field_options['sorttype'] = "'date'";
      $field_options['width'] = 70;
      $edit_options = "size: 10, maxlengh: 10,
      dataInit: function(element) {
        $(element).datepicker({dateFormat: 'yy-mm-dd'})
      }";
      break;

    case 'autocomplete':
      $search_options = "sopt:['eq','ne']";
      $field_options['width'] = 60;
      $field_options['edittype'] = "'text'";

      $handler = 'http://dev.apartments-for-rent.com/home/ajax/searchJSON.php';
      $search_field = 'name';
      $value_field  = 'id';
      $filters = '';      
      $script = '';


      $edit_options = "dataInit: function (elem) {
        setTimeout(function() {
          alert(1);
          $(elem).autocomplete({
              source: ['c++', 'java', 'php', 'coldfusion', 'javascript', 'asp', 'ruby']
          });
        },200);
      }";

      break;

    case 'image':
      if(1) { // fine-uploader
        $field_options['edittype'] = "'custom'";
        $fn_name = $obj_type."_".$fld."_uploader";
        $uploader_id = "$obj_type-0-$fld";
        $edit_options = 'custom_element: '.$fn_name.', custom_value: myvalue, dataInit: function(element) {          
          var $el = $(element);
          var $loader = $el.find(".jquery-fileupload");
          var loader_id = $loader.attr("id");
          
          var parent_type = "'.$obj_type.'";
          var parent_id = currentsel;
          var parent_field = "'.$fld.'";
          var mediaData = {"parent_type": parent_type, "parent_id": parent_id, "parent_field": parent_field};
          $loader.data("media_data", mediaData);
          var edit_form_id = "editmod" + table_id; 
          var $form = $("#" + edit_form_id);
          var display = $form.length ? $form.css("display") : "";
          var edit_form_open = $form.length > 0 && display == "block";
          if(uploader_init && edit_form_open) {
            setTimeout(function() {
              console.log("jqgrid: form is open, calling load data");
              jfu_load($("#" + loader_id), mediaData);
            },200);
          }
          
        }
        ';
      } elseif(0) { // old school + ajaxfileuploader
        $field_options['edittype'] = "'file'";
        $class_options_json = urlencode(json_encode($fld_options));        
        $edit_options = "enctype: 'multipart/form-data', class:'fileuploader', field: '$fld', options:'$class_options_json'";
      }
      $field_options['formatter'] = 'jgImageFormatter';                                    
      break;

    case 'media-not-impl':
      $field_options['formatter'] = 'mediaFormatter'; 
      $field_options['unformat'] = 'mediaUnformatter'; 
      break;
      
    case 'color':
      $field_options['formatter'] = 'colorFormatter'; 
      $field_options['unformat'] = 'unFormatter';
      //$field_options['edittype'] = "'custom'";
      //$edit_options .= "custom_element:color_picker,custom_value:color_value";
      $edit_options .= "size: 10, maxlengh: 6, dataInit: function(element) {
        $(element).ColorPicker({
          onSubmit: function(hsb, hex, rgb) {
          	$(element).val(hex);
        		$(element).ColorPickerHide();
          },
          onBeforeShow: function () {
          	$(this).ColorPickerSetColor(this.value);
          },
        	onChange: function (hsb, hex, rgb) {
        		$(element).val(hex);
        		$(element).css('background-color', '#' + hex);
          	var brightness = color_brightness(hex);
          	var fg = brightness < 0.5 ? '#FFFFFF' : '#000000';
        		$(element).css('color', fg);
        	},
        	eventName: 'focus'

        }).bind('keyup', function(){
	        $(this).ColorPickerSetColor(this.value);
        });
    		// $(element).ColorPickerShow();
        }";

      break;
    case 'bool':
      $field_options['edittype'] = "'checkbox'";
      $field_options['formatter'] = "'checkbox'"; 
      $search_options = "sopt:['eq','ne'], value: ':All;1:Yes;0:No'";
      $edit_options = "value:'1:0'";
      $field_options['stype'] = "'select'"; // search type
      $field_options['width'] = 30;
      break;
    case 'money':
      $field_options['align'] = "'right'"; 
      $field_options['formatter'] = "'currencyFmatter'"; 
      //$format_options = "prefix:'&euro;'"; // todo: allow other currencies
      $search_options = "sopt:['eq','ne','lt','le','gt','ge']";
      $editRulesAr[] ="number:true";
      $field_options['width'] = 60;
      $field_options['sorttype'] = "'currency'";
      break;
    case 'float':
      $field_options['align'] = "'right'"; 
      $field_options['formatter'] = "'number'"; 
      $search_options = "sopt:['eq','ne','lt','le','gt','ge']";
      $editRulesAr[] = "number:true";
      $field_options['width'] = 25;
      $field_options['sorttype'] = "'float'";
      break;
    case 'int':
      $field_options['align'] = "'right'"; 
      $field_options['formatter'] = "'integer'"; 
      $search_options = "sopt:['eq','ne','lt','le','gt','ge']";
      $editRulesAr[] = "integer:true";
      $field_options['width'] = 25;
      $field_options['sorttype'] = "'int'";
      break;
    case 'id':
      $search_options = "sopt:['eq','ne','lt','le','gt','ge']";
      $field_options['width'] = 30;
      //$field_options['sorttype'] = "'int'";
      // $field_options['formatter'] = 'integer'; 
      // $editRulesAr[] = "number:true";

      break;
    case 'url':
      $field_options['formatter'] = "'link'"; 
      $search_options = "sopt:['eq','ne','lt','le','gt','ge']";
      $field_options['width'] = 60;
      break;
    case 'link':
      $field_options['edittype'] = "'select'";
      $field_options['formatter'] = "'showlink'"; 
      $search_options = "sopt:['eq','ne']";
      $field_options['width'] = 40;
      $app_link = get_page_link(PAGE_ADMIN);
      $format_options = "baseLinkUrl:'$app_link', addParam: '&obj_type=$rel_type&action=view'";
//       formatter:'showlink', 
//       formatoptions:{baseLinkUrl:'someurl.php', addParam: '&action=edit'}
             
      break;
    case 'many2one':
      $sel_filters = $class[ATT_FILTERS];
      if($options['editable'] == 'false') { // just show value
        // do nothing
      } else {
        $field_options['edittype'] = "'select'";
        // editoption: { value: "FE:FedEx; IN:InTime; TN:TNT" }
      }
      $search_options = "sopt:['eq','ne']";
      $field_options['width'] = 60;
      break;
    case 'valSelect':
      $field_options['edittype'] = "'select'";
      $eos = array();
      foreach($rel_type as $k => $v) $eos[] = "$k:$v";
      $edit_options = 'value: "'.implode(';', $eos).'"';
      $field_options['width'] = 60;
      break;
    case 'intSelect':
      $field_options['edittype'] = "'select'";
      $ar = num_array($fld_options[0], $fld_options[1], $fld_options[2], $fld_options[3]);
      $eos = array();
      foreach($ar as $a) $eos[] = "$a[0]:$a[1]";
      $edit_options = 'value: "'.implode(';', $eos).'"';
      $field_options['width'] = 30;
      break;
    case 'str':
      // $field_options['formatter'] = ''; 
      $search_options = "sopt:['bw','ew','cn']";
      $field_options['width'] = 60;
      $field_options['sorttype'] = "'text'";
      break;
    case 'text':
      //$field_options['formatter'] = ''; 
      $field_options['edittype'] = "'textarea'";
      $search_options = "sopt:['bw','ew','cn']";
      $edit_options = "rows:'3',cols:'35'";
      $field_options['width'] = 150;
      $field_options['sorttype'] = "'text'";
      break;
    case 'phone':
      $search_options = "sopt:['bw','ew','cn']";
      $field_options['width'] = 50;
      $field_options['sorttype'] = "'text'";
      break;
    case 'password':
      $search_options = "sopt:['bw','ew','cn']";
      $field_options['width'] = 50;
      $edit_options = "custom:true, custom_func:passwordcheck";
      break;
    case 'html':
      //$field_options['formatter'] = ''; 
      $field_options['edittype'] = "'textarea'";
      $search_options = "sopt:['bw','ew','cn']";
      $edit_options = "rows:'3',cols:'35'";
      $field_options['width'] = 150;
      break;
    case 'ip':
      $field_options['width'] = 50;
      break;
    case 'discount':
      $field_options['width'] = 50;
      break;
    case 'commission':
      $field_options['width'] = 50;
      break;
    case 'surcharge':
      $field_options['width'] = 50;
      break;
    case 'yesno':
      $field_options['width'] = 40;
      break;
    case 'msel':
      break;
    case 'image':
      break;
    case 'map':
      break;

    default:
      echo("Field type $fld_type not implemented yet<br>");
      break;
  }

  $att_flags = $class[ATT_FLAGS];
  
  // generic for all
  
  // show in edit even if hidden in grid
  if($att_flags & AF_HIDE_LIST && !($att_flags & AF_HIDE_EDIT) && !($att_flags & AF_HIDE)) $editRulesAr[] = "edithidden: true"; 
  if($att_flags & AF_OBLIGATORY) $editRulesAr[] = "required: true";
  
  if(count($editRulesAr)) $field_options['editrules']     = "{".implode(",", $editRulesAr)."}";

  if($value) { // todo: use implode like with edit rules
    if($edit_options) $edit_options .= ",";
    $edit_options .= "readonly:'readonly',disabled:'disabled',value:'$value',custom_element:myelement,custom_value:myvalue"; // hidden or disabled
  }
  
  if($format_options) $field_options['formatoptions'] = "{".$format_options."}";
  if($edit_options)   $field_options['editoptions']   = "{".$edit_options."}";
  //if($search_options) $field_options['searchoptions'] = "{"."sopt:[".$search_options."]}";
  if($search_options) $field_options['searchoptions'] = "{".$search_options."}";
  return $field_options;
}

// converts from textual operator (ne, eq, lt etc) to SQL operator
// used by jqGrid's advanced search
function operator_text2sql($field, $op, $data) {
  switch($op) {
    case 'eq': // equal ( = )                            
      return "$field = '$data'";
    case 'ne': // not equal ( <> )                       
      return "$field <> '$data'";
    case 'lt': // less than ( < )                        
      return "$field < '$data'";
    case 'le': // less than or equal to ( <= )           
      return "$field <= '$data'";
    case 'gt': // greater than ( > )                     
      return "$field > '$data'";
    case 'ge': // greater than or equal to ( >= )        
      return "$field >= '$data'";
    case 'bw': // begins with ( LIKE val% )              
      return "$field LIKE '".$data."%'";
    case 'ew': // ends with (LIKE %val )                 
      return "$field LIKE '%".$data."'";
    case 'cn': // contain (LIKE %val% )                  
      return "$field LIKE '%".$data."%'";
    default:
      return "";
  }
}

function jqGridUploader($obj_type, $fld, $uploader) {
  $fn_name = $obj_type."_".$fld."_uploader";
  $uploader = str_replace("'", "\'", $uploader);
  return ";
  function $fn_name(value, options) {
    console.log('custom edit: $fn_name value=' + value + ' options=');
    console.log(options);
    return '$uploader';
  }
  ";
}

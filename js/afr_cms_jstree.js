  function jst_rankUpdate(id) {
    var $node = $("#" + id);
    var $parent = $node.parent();
    var $siblings = $parent.find("LI");
    var child_type = 'site_page';
    var childArray = [];
    console.log("jst ru id=" + id + ' sibs', $siblings);
    $.each($siblings, function(i, item) {
      $item = $(this);
      var sib_id = $item.attr('id');
      childArray.push(sib_id);                    
    });
    var id_list = childArray.join(',');
    var ajax_url = '/admin/app_data.php?oper=rank_update&obj_type=' + child_type + '&id_list=' + id_list;
    $.getJSON(ajax_url, function(json) {        
      if(json.success) {
        console.log('ok, update rank of ' + child_type);
      }
    });
  }
  
  $(document).on('click', '#btnReload, .treeReload', function() {
    jst_refresh();
  });

  
  function jst_viewNode(node) {
    var obj = node.original;  
    var id = node.id;  
    var child_count = node.children ? node.children.length : 0;
    var add = true;
    var obj_id = obj && obj.obj_id ? obj.obj_id : 0;
    if(!obj_id) {
      var obj_type = 'site_page';
    } else {
      var obj_type = obj.obj_type;
      var name = '';
    }
    //console.log("org: ", obj);
    //console.log("org id: ", obj_id);
    //console.log("Select node: ",node, " ccount=" + child_count);
    
    var lang = $('#jst_language').val();    
    var ajax_url = "/admin/app_data.php?oper=cms-explore&site_id=" + site_id + "&user_id=" + user_id + "&obj_type=" + obj_type + "&obj_id=" + obj_id+"&child_count=" + child_count + "&name=" + encodeURIComponent(node.text) + "&parent_id=" + node.parent + '&language=' + lang;
    $('#cms_viewer').html('<i class="fa fa-spinner fa-spin"></i>Loading...');
    $('#cms_viewer').load(ajax_url, function() {
      console.log("cms load ready");
      traveler_ready();     
    });
  }

  function jst_delete(obj_id) {
    var update_tree = false;
    if(!obj_id) {
      var $target = $(this);
      var obj_id = $target.data('id');
      update_tree = true;
    }
    console.log("Jst delete id=" + obj_id);
    var ajax_url = '/admin/app_data.php?oper=delete&obj_type=site_page&id=' + obj_id;
    $.getJSON(ajax_url, function(json) {        
      if(json.success) {
        console.log('ok, deleted node,');
        jst_refresh();
        if(update_tree) {          
          //console.log('manually removing from tree');
          //$("#tree").jstree(true).delete_node([obj_id]);
        }
      }
    });        
    
  }
  
  function jst_refresh() {
    var active = $('#jst_show_active').is(':checked') ? 1 : 0;
    var show_menu = $('#jst_show_menu').is(':checked')  ? 1 : 0;
    var lang = $('#jst_language').val();    
    var url = '/admin/app_data.php?oper=site-tree&active=' + active + '&show_menu=' + show_menu + '&lang=' + lang;
    console.log('refresh with url=' + url);
    $('#tree').jstree(true).settings.core.data.url = url;
    $('#tree').jstree(true).refresh();    
  }
  
  function jst_init(site_id, user_id) {
    var lang = $('#jst_language').val();    
    var options = {
      "core" : {
        "animation" : 0,
        "check_callback" : true,
        "check_callback_off" : function (op) {
          console.log("true: callback op=" + op);
          return true;
          if(op == 'delete_node') { return confirm('Are you sure you want to delete this page and all its content?'); }          
          return true;
        },
        "themes" : { "stripes" : true },
        'data' : {
          'url' : '/admin/app_data.php?oper=site-tree&active=1&show_menu=1&lang=' + lang,
          /**
          function (node) {
            // return node.id === '#' ? 'ajax_demo_roots.json' : 'ajax_demo_children.json';
            var active = 1;
            var show_menu = 1;
            return '/admin/app_data.php?oper=site-tree&site_id=' + site_id + '&node_id='+ node.id + '&active=' + active + '&show_menu=' + show_menu;
          },
          */
          "dataType" : "json",
          'data' : function (node) {
            return { 'id' : node.id, 'text': node.name};
          }
        }
      },
      "types" : {
        "#" : {
          "max_children" : 1,
          "max_depth" : 4,
          "valid_children" : ["root"]
        },
        "root" : {
          "icon" : "/afr/images/icons-16/tree.png",
          "valid_children" : ["default"]
        },
        "default" : {
          "valid_children" : ["default","file"]
        },
        "file" : {
          "icon" : "glyphicon glyphicon-file",
          "valid_children" : []
        }
      },
      "plugins" : [
        "contextmenu", "dnd", "search","state", "types", "wholerow", "unique"
      ]
    };
    console.log("init options:", options)
    
    $('#tree').jstree(options).bind("select_node.jstree", function (e, data) {
      console.log("Select node",data);
      jst_viewNode(data.node);
    }).bind("create_node.jstree", function (e, data) {
      console.log("Create node",data);
      data.node.icon = "fa fa-plus-square";
      
    }).bind("delete_node.jstree", function (e, data) {
      var obj_id = data.node.id;
      jst_delete(obj_id);

    }).bind("copy_node.jstree", function (e, data) {
      $('#tree').jstree(true).set_icon(data.node.id, 'fa fa-plus-square');
      console.log("Copy node id=" + data.node.id + " new 2",data);
      //var obj_id = data.node.id;
      
    }).bind("paste.jstree", function (e, data) {
      $('#tree').jstree(true).set_icon(data.node.id, 'fa fa-plus-square');
      console.log("Paste node new icon",data);
      //var obj_id = data.node.id;

    }).bind("rename_node.jstree", function (e, data) {
      var obj_id = data.node.id;
      var name = data.node.text;
      console.log("Rename node data:", data, " val=" + name);
      var ajax_url = '/admin/app_data.php?oper=save&obj_type=site_page&id=' + obj_id + '&name=' + encodeURIComponent(name);
      console.log("saving..." + ajax_url);
      $.getJSON(ajax_url, function(json) {        
        if(json.success) {
          console.log('ok, saved node, now view');
          jst_viewNode(data.node);
        }
      });        
      
    }).bind("move_node.jstree", function (e, data) {
      //console.log("Moved node data:",data);
      var parent = data.parent;
      var old_parent = data.old_parent;
      var pos = data.position;
      var id = data.node.original && data.node.original.obj_id ? data.node.original.obj_id : 0;

      console.log("Moved node: id=" + id + " p=" + parent + ' op=' + old_parent);
      jst_viewNode(data.node);            
    
      if(id) {
        jst_rankUpdate(id);
        if(parent != old_parent) {
          var parent_id = parent < 0 ? 0 : parent;
          var ajax_url = '/admin/app_data.php?oper=save&obj_type=site_page&id=' + id + '&parent_id=' + parent_id;
          console.log("saving..." + ajax_url);
          $.getJSON(ajax_url, function(json) {        
            if(json.success) {
              console.log('ok, moved node');
            }
          });        
          jst_rankUpdate(old_parent);
        }
      }
      //data.instance.refresh(); 
    });
  }
  
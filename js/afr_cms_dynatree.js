// new CMS dynatree code for gallery CMS

$(document).on("click", "#btnReload", function() {
  var activeNode = $(".dynatree").dynatree("getActiveNode");
  if(activeNode) refreshActiveNode(); 
  else reload_tree();
});

$(document).on("click", ".cms-sharing-link", function() {
  var $link = $(this);
  var data = $link.data('data');
  console.log("new sharing");
  console.log(data);
  var ajax_url = '/admin/app_data.php?oper=new-sharing&' + obj2qs(data);
  $('#cms_viewer').load(ajax_url);  
  return false;
});

function reload_tree() {
  console.log("Reloading tree");
  var tree = $("#tree").dynatree("getTree");
  var activeNode = $(".dynatree").dynatree("getActiveNode");
  var activeNodes = tree.getSelectedNodes();
  console.log("active nodes");
  console.log(activeNodes);
  console.log("active node");
  console.log(activeNode);
  var nodeAr = ['art', 'artist', 'curator', 'published', 'exhibited', 'shared', 'trash']; // the top level lazy nodes
  $.each(nodeAr, function(i, nodeKey) {
    var topNode = tree.getNodeByKey(nodeKey);
    if(topNode) topNode.reloadChildren(function(topNode, isOk) {});
  });
  // if(activeNode) activateNode(activeNode); // makes double folders, for some reason();
}

$(document).on("click", "#btnReloadActive", function() {
  var node = $(".dynatree").dynatree("getActiveNode");
  if(node && node.isLazy()) {
    node.reloadChildren(function(node, isOk) {});
  } else {
    alert("Please activate a lazy node first.");
  }
});

$(document).on("click", "#btnToggleSelect", function() {
  $(".dynatree").dynatree("getRoot").visit(function(node){
    node.toggleSelect();
  });
  return false;
});
$(document).on("click", "#btnDeselectAll", function() {
  $(".dynatree").dynatree("getRoot").visit(function(node){
    node.select(false);
  });
  return false;
});
$(document).on("click", "#btnSelectAll", function() {
  $(".dynatree").dynatree("getRoot").visit(function(node){
    node.select(true);
  });
  return false;
});

function cms_link_handler($target) {
  var data = $target.data('data');
  if(data) {
    var site_id = data.site_id;
    var obj_type = data.obj_type;
    var id = data.id || data.obj_id;
    var tree = $("#tree").dynatree("getTree");
    var key = obj_type + '-' + id;
    var node = tree.getNodeByKey(key);
    if(node) {
      node.expand(true);
      node.activate();  
      console.log("found node");
      console.log(node);
      if(!in_iframe()) $('html, body').animate({scrollTop:0}, 'slow');
    } else {
      console.log("Did not find node with key=" + key);
    }
    
    console.log(data);
    return false;
  }
  return false;
}

$(document).on("click", ".cms-link", function() {
  var $target = $(this);
  return cms_link_handler($target);
});


$(document).on("click", ".cms-delete", function() {
  var $target = $(this);
  var sourceNode = $("#tree").dynatree("getActiveNode");
  console.log("Deleting Node");
  console.log(sourceNode);
  if(sourceNode && sourceNode.data) {
    var obj_type = getNodeType(sourceNode);
    var id = getNodeId(sourceNode);
    var site_id = $target.data('site_id');
    var user_id = $target.data('user_id');
    var message = '';
    var ajax_url = '/admin/app_data.php?oper=cms-delete&obj_type=' + obj_type + '&site_id=' + site_id + '&user_id=' + user_id + '&id=' + id;
    console.log(ajax_url);
    $.getJSON(ajax_url, function(json) {        
      console.log(json);
      if(json.success) {
        message = json.message || "ok, deleted " + obj_type + " " + id;
        console.log(message);
        sourceNode.remove();        
      } else {
        message = json.error || "Unknown error";
      }
      $("#cms_viewer").html(json.success ? bootstrap_success_message(message) : bootstrap_error_message(message));
    });      
  }
    
});

// cms create button
$(document).on("click", ".cms-create", function() {
  var $target = $(this);
  var site_id = $target.data('site_id');
  var user_id = $target.data('user_id');
  var obj_type = $target.data('obj_type');
  var type = $target.data('type') || 0;
  console.log($target.data());
  
  //return false;
  //console.log("cms create " + obj_type);
  
  $('#gallery-create-menu').hide();

  var parent_key = '', what = '';
  switch(obj_type) {
  case 'gallery':
    what = 'folder';
    parent_key = 'art';
    break;
  case 'media_collection':
    if(!in_array(type, [100,120])) { // 100 = project, 120 = exhibition
      alert("Creating " + obj_type + " of type " + type + " is not supported");
      return false;
    }
    parent_key = type == 120 ? 'exhibited' : 'published';
    what = type == 120 ? 'exhibition' : 'shared folder';
    break;
  case 'media':
    alert("Creating " + obj_type + " not yet implemented");
    return false;
    what = 'media';
    break;
  default:
    alert("Creating " + obj_type + " is not supported");
    return false;
    break;
  }

  var tree = $("#tree").dynatree("getTree");
  var parent_node = parent_key ? tree.getNodeByKey(parent_key) : null;

  var title = "New " + what;
  var ajax_url = '/admin/app_data.php?oper=add&obj_type=' + obj_type + '&site_id=' + site_id + '&user_id=' + user_id + '&title=' + title + '&active=1';
  if(type) ajax_url = ajax_url + "&type=" + type;
  console.log(ajax_url);
  $.getJSON(ajax_url, function(json) {        
    console.log(json);
    if(json.success) {
      console.log("ok, created new " + obj_type);
      var newObj = parse_json(json.data);
      console.log(newObj);
      //reload_tree();
      if(parent_node) {
        console.log("Expanding parent " + parent_key);
        parent_node.reloadChildren(function(topNode, isOk) {
          console.log("Done reloading children");
          parent_node.expand(true);
          parent_node.activate();  
          var new_key = obj_type + '-' + newObj.id;
          newNode = $("#tree").dynatree("getTree").getNodeByKey(new_key); // get new tree
          if(newNode) {
            console.log("ok, found node " + new_key);
            newNode.activate();
            //editNode(newNode);
          } else {
            console.log("did not find new node " + new_key);
          }
        });
      }
    } else {
      alert("Oops, something went wrong in creating the new " + what);
    }
    
  });
});

// returns ajax method used for dragging to determine if we are allowed to drop here
function dragGetMethod(node, sourceNode) {  
  var source_level = node.tree.getNodeByKey(sourceNode.data.key).getLevel();
  if(source_level == 1) return false; // cannot move top folder
  
  var source_obj_type = getNodeType(sourceNode);
  var source_id = getNodeId(sourceNode);
  var source_category = getNodeCategory(sourceNode);

  var dest_level = node.tree.getNodeByKey(node.data.key).getLevel();
  var dest_obj_type = getNodeType(node);
  var dest_category = getNodeCategory(node);
  var drop_ba = ['before', 'after']; // drop before/after, not over
  
  console.log(source_category + '/' + source_obj_type + '(' + source_level + ') => ' + dest_category + '/' + dest_obj_type + '(' + dest_level + ')');    

  switch(source_category) {
  case 'art':
    if(dest_category == 'trash' && dest_level == 1)  return {"method": "cms-recycle", "drop": dest_level==1 ? 'over' : drop_ba};

    if(dest_category == 'published' || dest_category == 'exhibited') {
      var method = dest_category == 'published' ? 'cms-publish' : 'cms-exhibit';
      if(source_level == 2) { // we're moving a folder
        if(dest_level == 3) return false; // cannot drop a folder on or before/after image
        return {"method": method, "drop": dest_level==1 ? 'over' : drop_ba};
      } else if(source_level == 3) { // we're moving an image
        if(dest_level == 1) return false; // cannot drop an image in root folder
        return {"method": method, "drop": dest_level==2 ? 'over' : drop_ba};
      }
      //source_level == dest_level + 1) { // publish art to project (share)
    }
    if(dest_category == 'art') { // move art to folder
      if(source_level < 3 || dest_level == 1) return false; // only allow moving images, not folders, and do not allow moving into top folder "art"
      return {"method": "cms-move", "drop": dest_level==2 ? 'over' : drop_ba}; // can only drop over folder, between images
    }
    break;
  case 'published':
    if(dest_category == 'trash' && source_level >= 2) return {"method": "cms-recycle", "drop": dest_level==1 ? 'over' : drop_ba};
    //if(dest_category == 'trash' && source_level == 3) return {"method": "cms-delete", "drop": dest_level==1 ? 'over' : drop_ba};
    if(dest_category == 'exhibited' && source_level == 2 && dest_level <= 2) return {"method": "cms-exhibit", "drop": dest_level==1 ? 'over' : drop_ba};
    if(dest_category == 'exhibited' && source_level == 3 && dest_level >= 2) return {"method": "cms-exhibit", "drop": dest_level==2 ? 'over' : drop_ba};
    if(dest_category == 'published' && source_level == 3 && dest_level >= 2) return {"method": "cms-move", "drop": dest_level==2 ? 'over' : drop_ba};
    
    break;
  case 'exhibited':
    if(dest_category == 'trash' && source_level >= 2) return {"method": "cms-recycle", "drop": dest_level==1 ? 'over' : drop_ba};
    //if(dest_category == 'trash' && source_level == 3) return {"method": "cms-delete", "drop": dest_level==1 ? 'over' : drop_ba};
    if(dest_category == 'exhibited' && source_level == 3 && dest_level >= 2) return {"method": "cms-move", "drop": dest_level==2 ? 'over' : drop_ba};
    if(dest_category == 'published' && source_level == 2 && dest_level <= 2) return {"method": "cms-publish", "drop": dest_level==1 ? 'over' : drop_ba};
    if(dest_category == 'published' && source_level == 3 && dest_level >= 2) return {"method": "cms-publish", "drop": dest_level==2 ? 'over' : drop_ba};
    break;

  case 'shared':
    if(dest_category == 'exhibited' && source_level == 3 && dest_level <= 2) return {"method": "cms-exhibit", "drop": dest_level==1 ? 'over' : drop_ba};
    if(dest_category == 'exhibited' && source_level == 4 && dest_level >= 2) return {"method": "cms-exhibit", "drop": dest_level==2 ? 'over' : drop_ba};
    
    break;
  case 'trash':
    if(source_obj_type == 'gallery' && dest_category == 'art' && dest_level == 1) return {"method": "cms-recycle", "drop": dest_level==1 ? 'over' : drop_ba};
    if(source_obj_type == 'media_collection' && dest_category == 'published' && dest_level == 1) return {"method": "cms-recycle", "drop": dest_level==1 ? 'over' : drop_ba};
    break;
  default:
    return null;
  }
  
  return null;
  
}


function dragEnter(node, sourceNode, drop) {        
  var source_level = node.tree.getNodeByKey(sourceNode.data.key).getLevel();
  var dest_level = node.tree.getNodeByKey(node.data.key).getLevel();
  var source_category = getNodeCategory(sourceNode);
  if(node.parent == sourceNode.parent && source_level == dest_level) {
    if(in_array(source_category, ['artist', 'curator', 'shared', 'trash'])) return false;
    return ['before', 'after']; // Don't allow dropping *over* a node (would create a child)
  } else { // if(node.parent !== sourceNode.parent) {
    if(data = dragGetMethod(node, sourceNode)) {
      var method = data.method;
      var drop = data.drop; // array with at least one of ['before', 'after', 'over']
      console.log('dnd: method = ' + method + ' drop=');
      console.log(drop);
      node.data.method = method;
      node.data.source_category = source_category; // this is overwritte on drop, so store now
      return drop;
    }
    return false;
  }
}


function rankUpdate(parent) {
  var parent_id = getNodeId(parent);
  var parent_type = getNodeType(parent);
  var children = parent.getChildren();
  if(children && children.length) {
    var childArray = [];
    var child_type = getNodeType(children[0]);
    $.each(children, function(i, childNode) {
      var id = getNodeId(childNode);
      childArray.push(id);            
    });
    var id_list = childArray.join(',');
    console.log('parent=' + parent_type + ' ' + parent_id);
    console.log(child_type + ' order=' + id_list);
    var ajax_url = '/admin/app_data.php?oper=rank_update&obj_type=' + child_type + '&id_list=' + id_list;
    $.getJSON(ajax_url, function(json) {        
      if(json.success) {
        console.log('ok, update rank of ' + child_type);
        // if(url = parent.data.url) $('#cms_viewer').load(url); // reload
      }
    });
  }
}

function dragDrop(node, sourceNode, hitMode, ui, draggable) {
  var method = '';
  var reload_top = false;
  if(node.parent == sourceNode.parent) { // update rank
    
    sourceNode.move(node, hitMode); // move it first to get correct rank 
    var parent = node.parent;
    rankUpdate(parent); // update rank
    var parent = node.parent;
    if(url = parent.data.url) {
      var $uploader = $(".jquery-fileupload");
      var view = $uploader.length == 1 ? $uploader.data("view") : '';
      console.log('dnd: same parent url=' + url + "&view=" + view + " uploader len=" + $uploader.length);
      if(view) url = url + "&view=" + view; // preserve view (grid / list) of uploader
      $('#cms_viewer').load(url); // reload
    }
    return true;
  } else {
    var refresh = true;
    var source_level = node.tree.getNodeByKey(sourceNode.data.key).getLevel();
    var source_obj_type = getNodeType(sourceNode);
    var source_id = getNodeId(sourceNode);
    var source_category = node.data.source_category;
  
    var dest_level = node.tree.getNodeByKey(node.data.key).getLevel();
    var dest_obj_type = getNodeType(node);
    var dest_category = getNodeCategory(node);
    var dest_id = getNodeId(node);

    var site_id = $('#tree').data('site_id');
    var user_id = $('#tree').data('user_id');
    var user_level = $('#tree').data('user_level');
    
    var method = node.data.method;
    
    var ajax_url = ajax_tail = '';
    var send = false;
    var move = false;
    switch(method) {
    case 'cms-recycle':
      var active = dest_category == 'trash' ? 0 : 1;
      ajax_tail = "&active=" + active;
      send = true;
      move = true;
      break;
    case 'cms-delete':
      send = true;
      move = true;
      break;
    case 'cms-publish':
      send = true;
      move = false;
      refresh = false;
      break;
    case 'cms-exhibit':
      send = true;
      move = false;
      break;
    case 'cms-change-collection':
      send = true;
      move = true;
      break;
    case 'cms-move':
      send = true;
      move = true;
      break;
    }
    console.log("m=" + method + ' ' + source_category + '/' + source_obj_type + ' ' + source_id + ' (' + source_level + ') => ' + dest_category + '/' + dest_obj_type + ' ' + dest_id + ' (' + dest_level + ')');
  }

  /** multiple in selection ? */
  var selNodes = node.tree.getSelectedNodes(); // Display list of selected nodes
  var selCount = selNodes.length;
  var selKeys = [];
  if(selCount && selCat == source_category) { 
    selKeys = $.map(selNodes, function(node) { // convert to title/key array
      var key = node.data.key;
      var typeAr = key.split("-");
      var obj_type = typeAr[0];
      var id = typeAr[1];
      if(obj_type == source_obj_type) return id;
    });
    if(selKeys.length) {
      source_id = source_id + "," + selKeys.join(',');
      console.log("update multiple: selCount=" + selCount + " cat: " + selCat + " list:" + source_id);
    } 
  } else {
    console.log("Nope: selCount=" + selCount + " cat: " + selCat + " source:" +  source_category);    
  }
  
  //send = false;
  
  if(send) {
    ajax_url = '/admin/app_data.php?oper=' + method + '&obj_type=' + source_obj_type + '&id=' + source_id + "&dest_type=" + dest_obj_type + "&dest_id=" + dest_id + ajax_tail;
    ajax_url = ajax_url + '&site_id=' + site_id + '&user_id=' + user_id + '&user_level=' + user_level;
    $.getJSON(ajax_url, function(json) {        
      console.log("After drop. Calling:" + ajax_url);
      console.log(json);
      if(json.success) {
        console.log("ok, " + method + " returned success hitmode=" + hitMode);
        var folder = dest_level == 3 ? node.parent : node;

        if(move) sourceNode.move(node, hitMode); // move the node
        else refreshNode(folder); // update the tree
        
        rankUpdate(folder); // update rank
        folder.activate(); // activate parent of target

        if(dest_level == 1 || (dest_level == 2 && hitMode != 'over')) {
          console.log("Refresh top node " + dest_category);
          if(refresh) refreshNode(node.tree.getNodeByKey(dest_category)); // reload top level node 
        } else if(url = folder.data.url) {
          console.log("Refresh node from url = " + url);
          $('#cms_viewer').load(url); // reload
        }
        
        console.log("dest-level=" + dest_level + " activating node:" + folder.data.key);
        //if(url = folder.data.url) {
        //  console.log("reloading " + url);
        //  $('#cms_viewer').load(url); // reload
        //}

        if(selKeys.length) { /** we're done: unselect */
          deselectAll(node);      
        }

        return true;
      } else if(json.error) {
        alert(json.error);
        return false;
      }
    });
  }
  return false;
}

function editNode(node){
  var prevTitle = node.data.title,
    tree = node.tree;
  // Disable dynatree mouse- and key handling
  tree.$widget.unbind();
  // Replace node with <input>
  $(".dynatree-title", node.span).html("<input id='editNode' value='" + prevTitle + "'>");
  // Focus <input> and bind keyboard handler
  $("input#editNode")
    .focus()
    .keydown(function(event){
      switch( event.which ) {
      case 27: // [esc]
        // discard changes on [esc]
        $("input#editNode").val(prevTitle);
        $(this).blur();
        break;
      case 13: // [enter]
        // simulate blur to accept new value
        $(this).blur();
        break;
      }
    }).blur(function(event){
      // Accept new value, when user leaves <input>
      var title = $("input#editNode").val();
      // handler for live update
      var key = node.data.key;
      var typeAr = key.split("-");
      var obj_type = typeAr[0];
      var id = typeAr[1];
      if(!obj_type || !id) return false; 

      var title_field = '';  
      var what = obj_type;
      
      switch(obj_type) {
      case 'gallery':
        title_field = 'title';
        what = 'folder';  
        break;
      case 'media_collection':
        title_field = 'title';
        what = 'folder';  
        break;
      case 'media':
        title_field = 'name';
        what = 'artwork';  
        break;
      default:
        alert("Editing " + obj_type + " is not supported");
        return false;
        break;
      }
      
      var ajax_url = '/admin/app_data.php?oper=edit&obj_type=' + obj_type + '&id=' + id + '&' + title_field + '=' + title + '&active=1';
      $.getJSON(ajax_url, function(json) {        
        //console.log(json);
        if(json.success) {
          console.log("ok, updated " + obj_type + ' ' + id);
          node.setTitle(title);
          // Re-enable mouse and keyboard handlling
          tree.$widget.bind();
          node.focus();
          reloadNode(node); 

          //var node = $("#tree").dynatree("getActiveNode");
          //node.reloadChildren(function(node, isOk) {
          //  node.expand(true);
          //  node.activate();  
          //  newNode.activate();
          //  editNode(newNode);
          //});
        } else {
          alert("Oops, something went wrong in editing the " + what);
        }
        
      });
      
    });
}

/*
$(function(){
  var isMac = /Mac/.test(navigator.platform);  
  $("#tree").dynatree({
    title: "Event samples",
    onClick: function(node, event) {
      if( event.shiftKey ){
        editNode(node);
        return false;
      }
    },
    onDblClick: function(node, event) {
      editNode(node);
      return false;
    },
    onKeydown: function(node, event) {
      switch( event.which ) {
      case 113: // [F2]
        editNode(node);
        return false;
      case 13: // [enter]
        if( isMac ){
          editNode(node);
          return false;
        }
      }
    }
  });
});
    
*/

function getPath(key) {
  var tree = $("#tree").dynatree("getTree");
}

// Make sure that a node with a given ID is loaded, by traversing - and loading - its parents. This method is ment for lazy hierarchies.
// A callback is executed for every node as we go.
function activateNode(node) {
  var path = node.getKeyPath();
  console.log("Path of " + node.data.key + " = " +  path);
  if(!path) return false;
  var tree = $("#tree").dynatree("getTree");
  tree.loadKeyPath(path, function(node, status){
    if(status == "loaded") {
      // 'node' is a parent that was just traversed.
      // If we call expand() here, then all nodes will be expanded
      // as we go
      node.expand();
    }else if(status == "ok") {
      // 'node' is the end node of our path.
      // If we call activate() or makeVisible() here, then the
      // whole branch will be exoanded now
      node.activate();
    } else if(status == "notfound") {
      var seg = arguments[2],
        isEndNode = arguments[3];
    }
  });
}

function getNodeType(node) {
  var key = node.data.key;          
  var typeAr = key.split("-");
  return typeAr[0];
}

function getNodeId(node) {
  var key = node.data.key;          
  var typeAr = key.split("-");
  return typeAr.length >= 2 ? typeAr[1] : 0;
}

function getNodeCategory(node) {
  var level = node.tree.getNodeByKey(node.data.key).getLevel();
  var top;
  switch(level) {
  case 1:
    top = node;
    break;
  case 2:
    top = node.parent;    
    break;
  case 3:
    var parent = node.parent;    
    top = parent.parent;    
    break;
  case 4:
    var parent = node.parent;    
    var grand_parent = parent.parent;    
    top = grand_parent.parent;    
    break;
  }
  var category = getNodeType(top);
  return category;
}

function refreshActiveNode(params, reload) {
  if(typeof params == "undefined") var params = {};
  if(typeof reload == "undefined") var reload = false;
  
  console.log("refreshActive reload = " + reload + " params=");
  console.log(params);
  
  var activeNode = $(".dynatree").dynatree("getActiveNode");
  if(!activeNode) return false;
  var level = activeNode.getLevel();
  //var folder = level == 3 ? activeNode.parent : activeNode;
  var folder = level >= 2 ? activeNode.parent : activeNode;
  refreshNode(folder);  // updates tree on left
  if(reload) reloadNode(folder); // updates view on right
  //folder.expand(true);
}

// refresh lazy folder node
function refreshNode(node) {
  if(node && node.isLazy()) {
    node.reloadChildren(function(node, isOk) {});
  } else {
    console.log("No node or node is not lazy");
  }
}

function reloadNode(node) {
  if(url = node.data.url) {
    $("#cms_viewer").load(url, function() {
      console.log("Reloaded Node " + node.data.key);
      ready_script();
    });
  } else {
    console.log("reloadNode: Node has no URL");
  }
}

function deselectAll(node) {
  var selNodes = node.tree.getSelectedNodes(); // Display list of selected nodes
  console.log("Deselect count:" + selNodes.length);
  $.each(selNodes, function(i, node) {
    console.log("Deselect:",node);
    node.select(false);
  });
  selCat = '';
}


function updateActive(node) {
  /** example */
  var selNodes = node.tree.getSelectedNodes(); // Display list of selected nodes        
  var selKeys = $.map(selNodes, function(node) { // convert to title/key array
     return "[" + node.data.key + "]: '" + node.data.title + "'";
  });
 console.log("updateActive: selected: ", selNodes);
  var html = selKeys.join("<br>");
  if(activeNode) html = html + "<br>" + "Active:" + "[" + activeNode.data.key + "]: '" + activeNode.data.title + "'";       
  
  //$("#treeSelection").html(html);
  /** end example */
}

var activeNode;
var selCat;

$(function() {
  $("#tree").dynatree({ 
      checkbox: false,
      selectMode: 2,
      clickFolderMode: 3,
      imagePath: '/images/icons-16/',
      autoFocus: true,

      /** From example */
      onQuerySelect: function(select, node) {
        if( node.data.isFolder )
          return false;
      },
      onClick: function(node, event) {
        activeNode = null;
        if(!node.data.isFolder) {
          //node.toggleSelect();          
        }
      },
      onDblClick: function(node) {
        var category = getNodeCategory(node);
        logMsg("onDblClick(%o, %o)", node, event);
        //editNode(node);
        if(!node.data.isFolder && (!selCat || selCat == category)) {
          node.toggleSelect();          
        } else {
          console.log("not selecting: " + category + " selCat=" + selCat);          
        }
        node.toggleExpand();
      },
      onKeydown: function(node, event) {
        if( event.which == 32 ) {
          var category = getNodeCategory(node);
          if(!node.data.isFolder && (!selCat || selCat == category)) {
            node.toggleSelect();
          }
          return false;
        }
      },
      /** end example */
      
      onActivate: function(node) {
        var tree = $("#tree").dynatree("getTree");
        $("#cms_viewer").html("<i class='fa fa-spinner fa-spin'></i> Loading...");
        
        //console.log(tree.data);
        if(node) {
          var level = node.tree.getNodeByKey(node.data.key).getLevel();
          var category = getNodeCategory(node);
          //console.log("activate level=" + level + " cat=" + category);    
          //console.log(node.data);
          console.log("Activate Cat=" + category + " level=" + level);
          activeNode = node;
          updateActive(node);        
          
          if(category == "trash") {
            if(level > 1 && $("#cms-create-button").is(":visible")) {
              $("#cms-create-button").hide("fast");  
              $("#cms-delete-button").removeClass("hidden").show();
            }
          } else {
            if($("#cms-delete-button").is(":visible")) {
              $("#cms-delete-button").hide("fast");
              $("#cms-create-button").removeClass("hidden").show();  
            }
          }
          reloadNode(node);
        }
      },
      onExpand: function(flag, node) {
        console.log("expand");
        console.log(flag);
        console.log(node.data);
      },
      onDectivate: function(node) {
        $("#cms_viewer").html("");
      },
      onSelect: function(select, node) {
        var category = getNodeCategory(node);
        var this_key = node.data.key;
        var typeAr = this_key.split("-");
        var this_type = typeAr.length == 2 ? typeAr[0] : "";
        console.log("selected cat=" + category + " type=" + this_type + " key=" + this_key);
        if(!selCat) {
          selCat = category;
          console.log("selCat is now:" + category);
        }
        
        var selNodes = node.tree.getSelectedNodes();
        var count = selNodes.length;
        if(1) {
          console.log("cms buttons (share/delete) not yet enabled");
        } else {
          if(count < 0) $(".cms_button_hidden").removeClass("hidden").show("");
          else $(".cms_button_hidden").hide("");
        }
        if(this_type) {
          $.each(selNodes, function(i, selNode) {
            var sel_key = selNode.data.key;          
            var typeAr = sel_key.split("-");
            var sel_type = typeAr.length == 2 ? typeAr[0] : "";
            if(sel_type && sel_type != this_type) { 
              console.log("unselecting node with type " + sel_type);
            }
          });
        }
        console.log("sel count=" + count);
        if(!count) selCat = '';
        
        updateActive(node);        
      },
 });
});


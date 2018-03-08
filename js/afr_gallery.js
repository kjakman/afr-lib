console.log("afr_gallery.js v2.1");



$(document).on("change", "#popup-option-info", function(e) {
  var checked = $(this).prop("checked") ? 1 : 0;
  console.log("info button changed: checked=" + checked);
  if(checked) $("#lightbox_tabs").show(); 
  else $("#lightbox_tabs").hide();
});
  

function collection_image_handler(params, data) {
  console.log("collection_image_handler params:", params);
  console.log("data:", data);
  
}

function update_popup_options() {
  var tabs_json = $("#popup_tabs").val();
  var tabs = $.parseJSON(tabs_json)

  var buttons_json = $("#popup_buttons").val();
  var buttons = $.parseJSON(buttons_json)

  var titles_json = $("#popup_titles").val();
  var titles = $.parseJSON(titles_json)

  var options = {"buttons": buttons, "tabs": tabs, "title": titles};

  //console.log("update options tabs=" + tabs_json + " tabs:",tabs);
  
  $("#popup_options").val(JSON.stringify(options))
}

var fotorama_disabled = false;
$(document).on("click", ".gallery-link", function(e) {
  var $target = $(this);
  var $nav = $target.closest("nav");
  var url = $target.attr('href');

  var pdata = $nav.data('data') || {};
  var idata = $target.data('data') || {};
  var data = jQuery.extend(pdata, idata);
  data.url = url;
  
  var returntype = $target.data('returntype') || data.obj_type;
  data.returntype = returntype; 

  var target = data.target = data.target || '#gallery-viewer';
  var $viewer = $(target);
  console.log("gallery-link target=" + target + " viewerlen=" + $viewer.length + " url=" + url + " return=" + returntype + " data=", data);

  if(!$viewer.length) {
    console.log("Going to " + url);
    window.location = url;
    fotorama_disabled = true;
    return true; // fall back to hard link
  }
  
  gallery_print(target, data);
  return false;
  //gallery_load(data.target, data);
});

// When clicking on link while in my-vault (cms viewer)
$(document).on("click", ".cms-link", function(event) {
  var $link = $(this);
  var $cms = $('#cms_viewer');
  var $tree = $("#tree");
  if($cms.length && $tree.length) {
    return cms_link_handler($link);
  }
  return true;
});

$(document).on("click", ".photo-grid-container", function(event) {
  if(fotorama_disabled) {
    console.log("fotorama disabled");
    return;
  }
  var $container = $(this);
  var index = $container.data('index');
  var $grid = $container.closest(".photo-grid");
  var $fotorama = $grid.parent().find(".fotorama");
  console.log("fotorama click on " + $container.attr('id') + " index=" + index + " fotorama len=" + $fotorama.length);

  //var $fotorama = $(".fotorama"); /** only one per page ? */
  //$fotorama.removeClass('invisible').show();

  var fotorama = $fotorama.data('fotorama');
  console.log(fotorama);
  if(!fotorama) return false;
  
  fotorama.show(index);
  fotorama.requestFullScreen();
  //var $fotorama_img = $fotorama.find(".fotorama-" + index);
  //var $fotorama_img = $(".fotorama__thumb:eq(" + index + ")");
  //if($fotorama_img.length) {
  //  $fotorama_img.click();
  //  console.log("Clicked on photo " + index + " grid=" + $grid.length + " fotorama=" + $fotorama.length + " fotorama_img=" + $fotorama_img.length);
  ///}

  //console.log("Fotorama:", $fotorama_img);
  
  
  return false;
});

/** turn photo grid into fotorama */
function gallery_init(map_id) {
  console.log("gallery_init");
  $photo_grid = $(".photo-grid");
  //console.log("grid len=" + $photo_grid.length);
  $.each($photo_grid, function(i, item) {
    var $grid = $(this);
    var $photos = $grid.find(".photo-grid-container img");
    var $parent = $grid.parent();
    var $fotorama = $parent.find(".fotorama");
    var len = $fotorama.length;
    var parent_id = $parent.attr('id');
    
    var fotorama = '<div class="fotorama fotorama--hidden" data-auto="false" data-allowfullscreen="native" data-nav="thumbs"></div>';

    if(len) {
      console.log("gallery_init ALREADY INIT grid " + i + " len=" + len + " id=" + parent_id);
    } else {
      console.log("gallery_init on grid " + i + " len=" + len + " id=" + parent_id);
      $fotorama = $fotorama.length && $fotorama || $(fotorama).insertAfter($grid);  // append if doesn't exist
      
      //$fotorama = $fotorama.length && $fotorama || $(fotorama).appendTo("BODY");  // append if doesn't exist
      $.each($photos, function(j, photo) {
        var src = $(this).attr("src");
        var img = '<img data-index="' + j + '" src="' + src +'">';
        $(img).appendTo($fotorama);
        //console.log("appended " + img);
      });
      $fotorama.fotorama();
      $fotorama.on('fotorama:fullscreenexit', function() {
         //$(this).addClass("invisible");
         console.log("exit full screen");
      });
      
    }
    //$photos.appendTo($fotorama);           
  });


  if(map_id) {
    console.log("fotorama selected=" + map_id);
    //var selector = ".photo-grid-container[data-media_id='" + media_id + "']";
    var selector = "#map_" + map_id; 
    var $cont = $(selector);
    if($cont.length) {
      var index = $cont.data('index');
      console.log("found mid=" + map_id + " index=" + index);
      $parent = $cont.closest(".gallery-related");
      parent_id = $parent.attr('id');
      $fotorama = $parent.find(".fotorama");
      len = $fotorama.length;
      console.log("fotorama len=" + len + " parent=" + parent_id);
      if(len) {
        console.log("Showing index " + index);
        fotorama = $fotorama.data('fotorama');
        fotorama.show(index);
        fotorama.requestFullScreen();
      }
    }
  }        
}

// json data is printed by PHP from templates
function gallery_print(target, data) {  
  var $viewer = $(target);
  $viewer.css('opacity', '0.5');
  
  var output = '';
  var returntype = data.returntype;
  var ajax_url = "/admin/app_data.php?oper=gallery-get-collection&print&" + obj2qs(data); // obj_type=media_collection&obj_ id=" + my_collection_id + "&site_id=" + site_id + "&user_id=" + user_id;
  console.log("gallery_load: target=" + target + " len=" + $viewer.length + " url=" + ajax_url + " data=", data );
  $viewer.load(ajax_url, function() {
    $viewer.css('opacity', '1');    
    if(!in_iframe()) $('html,body').animate({scrollTop:top}, 'slow');
    
    console.log("Calling gallery init from gallery_print()...");
    gallery_init();
    
    // alert( "Load was performed." );
  });
}

/** not longer in use */
function gallery_follow(follow_id, message) {
  return false;
  
  var wname = window.name;
  var parent = window.parent;
  var opener = window.opener;
  var loc = window.location;
  var ploc = parent ? parent.location : 'no ploc';
  var wloc = opener ? opener.location : 'no wloc';
  console.log("wname =" + wname + " parent=" + parent + " loc=" + loc + " ploc=" + ploc + " wloc=" + wloc);
  //console.log("parent ", parent);
  //console.log("opener ", opener);
  if(window.opener) {
    //close();
    var $wrap = window.opener.$('.global-wrap');
    if($wrap.length) {
      message = message + "<div class='center' style='margin-top:15%'><h2>Congratulations, you are logged in!</h2>Click the arrow <i class='fa fa-arrow-circle-left fa-2x'></i> in the top left corner to continue exploring art.<br></div>";

      $wrap.html(message);
      var close_text = "Closing this window and returning you to the gallery";      
      $('.global-wrap').html(close_text);
      //$(close_text).appendTo($('.global-wrap'));
      
      close();
    }
    //opener.location = "https://www.google.com";
  }  
}

/**
   var error = "{$scope.error}";
    var warning = "{$scope.warning}";
    var message = "{$scope.message}";
    var text, msg, alert_class, alert_icon;
    if(error) {
      text = error;
      alert_class = 'error';
      alert_icon = 'glyphicon-check';
      alert_icon = 'glyphicon-exclamation-sign';
    } else if (warning) {
      text = warning;
      alert_class = 'warning';
      alert_icon = 'glyphicon-exclamation-sign';
    } else if (message) {
      text = message;
      alert_class = 'success';
      alert_icon = 'glyphicon-check';
    } else {
      text = '';
    }
    var msg='' + 
      '<div class="alert alert-' + alert_class + '" role="alert">' +
      '  <a href="#" class="close" data-dismiss="alert">&times;</a>' +        
        '  <span class="glyphicon ' + alert_icon + '" aria-hidden="true"></span>' +  text + 
      '</div>';
    msg = msg + "<div class='row'><div class='col-sm-4>{$img}</div></div>";      
    msg = msg + "<div>Use the back arrow above to go back to the gallery.</div>";
*/

// loads json data, print using javascript
function gallery_load(target, data) {  
  var $viewer = $(target);
  var output = '';
  var returntype = data.returntype;
  var ajax_url = "/admin/app_data.php?oper=gallery-get-collection&" + obj2qs(data); // obj_type=media_collection&obj_ id=" + my_collection_id + "&site_id=" + site_id + "&user_id=" + user_id;
  console.log("gallery_load: url=" + ajax_url + " data=", data);
  $.getJSON(ajax_url, function(response) { // start loading data from server
    if(response.success) {
      var data = response.data;
      var image,title,link,desc;
      console.log("Loaded :", data);
      $.each(data, function(i, item) {
        var image = '';
        switch(returntype) {
        case 'artist':
        case 'curator':
          title = item.display_name;
          break;
        case 'exhibition':
          title = item.title;
          break;
        default:
          title = item.name;
          break;
        }
        output = output + image + "<br>" + title + "<br>";
      });
      $viewer.html(output);
    } else {
      $viewer.html("error loading");
    }
  });
}

  $(document).on("click", "A.favorite-link", function() {
     var $item = $(this);
     var data = $item.data('item');
     var $icon = $item.find("i");
     //var $remove = $item.hasClass("remove");
     
     console.log("wishlist data=", data);   
     //console.log("wishlist icon class=", $icon.attr('class'));
  
     //ajax_url = "/admin/app_data.php?oper=wishlist&" + obj2qs(data);
     $icon.removeClass("fa-star fa-star-o fa-times").addClass("fa-spinner fa-spin");
     //console.log("Remove=" + remove);
     //return false;
      var ajax_url = '/admin/app_data.php';
      //$('#galleria .galleria-stage').addClass("loading-big");
      //var $galleria;
      //var $galleria_div = $('#galleria');
      var id = data.obj_id;
  
      var params = {
        format: "json",
        viewer: "html",
        oper: "gallery-favorite",
        obj_type: data.follow_type ? data.follow_type : data.obj_type,
        user_id: data.user_id,
        site_id: data.site_id,
        obj_id: data.follow_id ? data.follow_id : data.obj_id
      };
      //console.log("wishlist url=" + ajax_url);
      $.getJSON(ajax_url, params,function(result) {
        //var result = parse_json(responseText);
        //console.log("response", result);
        //console.log("result", result);
        if(result.success) {
          console.log("success id=" + result.id);
          var icon_class = result.id ? "fa-star" : "fa-star-o";
          $icon.removeClass("fa-spinner fa-spin").addClass(icon_class);
          //console.log("follow:", followArray);
          
        } else {
          console.log("error");
          $icon.removeClass("fa-spinner fa-spin").addClass("fa-exclamation-triangle fa-danger");
        }         
      });
     
     return false;
  });

  /** Thanks http://stackoverflow.com/questions/26823237/bootstrap-image-hover-overlay-with-icon */
//$(document).on("hover", ".pdf-thumb-box", function(evt) {


$(document).ready(function() {
    
  
  $(document).on("mouseenter", ".pdf-thumb-box", function(evt) {
  
    var $overlay = $(this).find(".pdf-thumb-box-overlay");
    $overlay.css("visibility", "visible");


    // var imgWidth = $(this).children("img").width();
    // var imgHeight = $(this).children("img").height();
    // var negImgWidth = imgWidth - imgWidth - imgWidth;
    // //$(this).children(".pdf-thumb-box-overlay").fadeTo(0, 0.8);
    // 
    // $(this).css("width", (imgWidth)+"px");
    // $(this).css("height", (imgHeight)+"px");

    //var len = $(this).children(".pdf-thumb-box-overlay").length;

    
    //console.log("thumb-box mouseenter len=" + len + " offset=", $overlay.offset());
  
    //$(this).children(".pdf-thumb-box-overlay").css("left", negImgWidth+"px");
    //$(this).children(".pdf-thumb-box-overlay").animate({"left": 0}, 250);
  });

  $(document).on("mouseleave", ".pdf-thumb-box", function(evt) {
    //console.log("thumb-box mouseleave");
  
    //var imgWidth = $(this).children("img").width();
    //var imgHeight = $(this).children("img").height();
    //var negImgWidth = imgWidth - imgWidth - imgWidth;
  
    $(this).children(".pdf-thumb-box-overlay").css("visibility", "hidden");
    //$(this).children(".pdf-thumb-box-overlay").animate({"left": negImgWidth}, 250);
   });
  

/**   

                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53313-1437932338" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53315-1437932357" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53318-1437932397" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53317-1437932377" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53311-1437932318" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53321-1437932437" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53323-1437932457" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53308-1437932312" alt=""  />
                                          <img src="http://images.interhome.com/BE4852.100/partner-medium/225503-1-53320-1437932417" alt=""  />
                */
  
});
    

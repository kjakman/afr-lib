//============================Masonary ==================================================//

jQuery(window).on('load', function() {
        $('#grid-masonry-collection').masonry({
          itemSelector: '.grid-item',
          //columnWidth: 160,
          horizontalOrder: true
      });
        $('#grid-masonry-project').masonry({
          itemSelector: '.grid-item',
          //columnWidth: 160,
          horizontalOrder: true
      });
        $('#grid-masonry-featuring').masonry({
          itemSelector: '.grid-item',
          //columnWidth: 160,
          horizontalOrder: true
      });
  });

//============================End Masonary ==================================================//

//============================Isotope ==================================================//
jQuery(window).on('load', function() {
        $('#grid-artworks').isotope({
          itemSelector: '.grid-item',
          masonry: {
        }
    });
        $('#grid-isotope-showroom').isotope({
          itemSelector: '.grid-item',
          masonry: {
        }
    });
        $('#grid-selected-artworks').isotope({
          itemSelector: '.grid-item',
          masonry: {
        }
    });
});
//============================End Isotope ==================================================//
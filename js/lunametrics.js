  
  
  (function($) {

    'use strict';

    var n = 0;
    init();

    function init(n) {
      // console.log("Init Lunametrics");

      // Ensure jQuery is available before anything
      if (typeof jQuery !== 'undefined') {

        bindToAjax();

      // Check for up to 10 seconds
      } else if (n < 20) {
	
        n++;
        setTimeout(init, 500);

      }

    }

    function bindToAjax() {
      // console.log("bindToAjax");

      $(document).bind('ajaxComplete', function(evt, jqXhr, opts) {
          // console.log("ajaxComplete");

        // Create a fake a element for magically simple URL parsing
        var fullUrl = document.createElement('a');
        fullUrl.href = opts.url;

        // IE9+ strips the leading slash from a.pathname because who wants to get home on time Friday anyways
        var pathname = fullUrl.pathname[0] === '/' ? fullUrl.pathname : '/' + fullUrl.pathname;
        // Manually remove the leading question mark, if there is one
        var queryString = fullUrl.search[0] === '?' ? fullUrl.search.slice(1) : fullUrl.search;
        // Turn our params and headers into objects for easier reference
        var queryParameters = objMap(queryString, '&', '=', true);
        var headers = objMap(jqXhr.getAllResponseHeaders(), '\n', ':');

        var ajax_obj = {
              // Return empty strings to prevent accidental inheritance of old data
              'type': opts.type || '',
              'url': fullUrl.href || '',
              'queryParameters': queryParameters,
              'pathname': pathname || '',
              'hostname': fullUrl.hostname || '',
              'protocol': fullUrl.protocol || '',
              'fragment': fullUrl.hash || '',
              'statusCode': jqXhr.status || '',
              'statusText': jqXhr.statusText || '',
              'headers': headers,
              'timestamp': evt.timeStamp || '',
              'contentType': opts.contentType || '',
              // Defer to jQuery's handling of the response
              'response': (jqXhr.responseJSON || jqXhr.responseXML || jqXhr.responseText || '')
            };
        
        var data = {
          'event': 'luna.ajaxComplete',
          // We use the luna namespace in case GTM ever claims 'ajax'
          'luna': {
            'ajax': ajax_obj
          }
        };
        var responseText = jqXhr.responseJSON || jqXhr.responseXML || jqXhr.responseText || '';
        
        // console.log("Pushing to GTM");
        // console.log(ajax_obj);
        // console.log("params");
        // console.log(queryParameters);
        var oper = queryParameters ? queryParameters.oper : '';
        var event_type = '';
        var json = false;
        var event = '';
        switch(oper) {
          case 'user-login':
          case 'user-register':
            event_type = 'user';
            json = true;
            break;

          case 'add-property':
            event_type = 'rental';
            json = true;
            break;

          case 'rental-booking':
          case 'rental-book-now':
          case 'rental-request-now':
          case 'rental-inquiry':
          case 'booking-cancel':
            event_type = 'booking';
            json = true;
            break;
          
          case 'mollie-pay':
            event_type = 'payment';
            json = true;
            break;
            
          default:
            break;
        }
        
        if(oper && event_type) {
          if(!event) event = oper;
          // console.log("UA event: " + event);
          if(json) {
            var response = responseText ? parse_json(responseText) : {};
            var data = response.data || {};
            
            if(response.success) {

              if($.inArray(event, ['rental-book-now', 'rental-request-now']) > -1) {
                ga_push_booking(data);
              } else {                
                // console.log("Success! Sending event of type " + event_type + " and event=" + event);
                ga('send', 'event', event_type, event, '');
              }
            } else {
              // console.log("No success :(");
            }
          } else {
            // console.log("No JSON :(");
          }
         
        }
        
        // Blindly push to the dataLayer because this fires within GTM

      });

    }

    function objMap(data, delim, spl, decode) {

      var obj = {};

      // If one of our parameters is missing, return an empty object
      if (!data || !delim || !spl) {

        return {};

      }

      var arr = data.split(delim);
      var i;

      if (arr) {

        for (i = 0; i < arr.length; i++) {

          // If the decode flag is present, URL decode the set
          var item = decode ? decodeURIComponent(arr[i]) : arr[i];
          var pair = item.split(spl);

          var key = trim_(pair[0]);
          var value = trim_(pair[1]);

          if (key && value) {
            obj[key] = value;

          }

        }

      }

      return obj;

    }

    // Basic .trim() polyfill
    function trim_(str) {

      if (str) {

        return str.replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g, '');

      }

    }

  })(jQuery);

  
function ga_push_booking(data, event) {
  var res_id, site_name,currency,grand_total,fee_service,cleaning_fee,num_days,apt_id,rate_day,rate_total;
  console.log("ga_push_booking:", data);
  if(!data) return;
  if(data.status >= 10) {
    res_id = data.res_id;
    currency = data.currency;
    grand_total = data.grand_total;
    fee_service = data.fee_service;
    rate_day = data.rate_base;                  
  }
  
  if(res_id && grand_total > 0) { /** ecommerce event */                
    ga('ecommerce:addTransaction', {
      'id': res_id,                     // Transaction ID. Required.
      'affiliation': site_name,         // Affiliation or store name.
      'revenue': grand_total,           // Grand Total.
      'currency': currency,
      'shipping': 0,                    // Shipping.
      'tax': 0                          // Tax.
    });
    if(rate_day > 0 && num_days > 0) {
      ga('ecommerce:addItem', {
        'id': res_id,                  // Transaction ID. Required.
        'name': "Accommodation",       // Product name. Required.
        'sku': apt_id,                 // SKU/code.
        'category': 'Booking',         // Category or variation.
        'price': rate_day,             // Unit price.
        'quantity': num_days           // Quantity.
      });
    }
    if(fee_service > 0) {
      ga('ecommerce:addItem', {
        'id': res_id,                  // Transaction ID. Required.
        'name': "Service Fee",         // Product name. Required.
        'sku': 10,                     // SKU/code.
        'category': 'Service',         // Category or variation.
        'price': fee_service,          // Unit price.
        'quantity': 1                  // Quantity.
      });
    }
    if(cleaning_fee > 0) {
      ga('ecommerce:addItem', {
        'id': res_id,                  // Transaction ID. Required.
        'name': "Cleaning Fee",        // Product name. Required.
        'sku': 20,                     // SKU/code.
        'category': 'Cleaning',        // Category or variation.
        'price': cleeaning_fee,        // Unit price.
        'quantity': 1                  // Quantity.
      });                
    }
    console.log("Success! Sending e-commerce data: res_id:" + res_id + " total:" + grand_total + " event:" + event);
    ga('ecommerce:send');
    ga('send', 'event', 'ecommerce', event, 'E-commerce ev:' + event + " rid=" + res_id + " gt:" + grand_total);
  } else {
    console.log("Nope! Not sending e-commerce data - missing res_id or total");
  }
  
  console.log("Sending back-up event");
  ga('send', 'event', 'ga-push-booking', event, '');
  
}

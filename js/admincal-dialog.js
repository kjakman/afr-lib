

// add edit booking
function cal_booking_dialog(data, reset) {
  if(typeof reset == "undefined") var reset = 1;

  console.log("cal_booking_dialog reset=" + reset + "data=", data);

  var form_id = 'calendar_dialog_form';
  var $messages = form_messages(form_id); // get message div      
  console.log("Removing messages with len=" + $messages.length);
  $messages.remove();
  $("#cd-source_obj_id").val('');
  
  if(reset) {
    $('#cal_tab1').tab('show');
    reset_form(form_id, true);
    cal_reset_fees();
    fill_form(form_id, data);
    
    if(data.obj_id || data.res_id) { // edit
      if(data.res_id && !data.obj_id) data.obj_id = data.res_id; 
      $("#cd-status option").show();
      cal_fetch_object(data.obj_type, data.obj_id);
      cal_update_status(data.status);

    } else { // new
      var default_status = $("#cd-status").data('default');
      console.log("New, hide most statuses default=" + default_status);
      $(".cal-rate-field").prop("disabled", false);
      $("#cd-invoice_locked_text").hide();
      
      $("#cd-status option[data-hide-new='1']").hide();
      $("#cd-status").val(default_status);
      $('#cd-rate_locked').prop('checked', false);
      $('#cd-send_email').prop('checked', false);
      $("#cd-salutation").val('');
      $("#cd-phone").val('');
      $('#cd-num_guests').val(1);
      
    }
    
    $("#cd-obj_id").val(data.obj_id);  
    $("#cd-apt_id").val(data.apt_id);  
    
    /** sql 2 human readable dates */
    //var checkin =  moment(data.checkin).format(g_moment_format);
    //var checkout = moment(data.checkout).format(g_moment_format);
    var checkin = data.checkin;
    var checkout = data.checkout;
    console.log("cbd: setting cin/out to " + data.checkin + " / " + data.checkout);
    $("#cd-checkin").val(data.checkin);
    $("#cd-checkout").val(data.checkout); 
    $("#cd-reset").hide();
    
  } else { /** + button: just keep all the data in the form except obj_id */
    
    var checkin = $("#cd-checkin").val();
    var checkout = $("#cd-checkout").val();
    if($("#cd-obj_id").val()) {
      $("#cd-source_obj_id").val($("#cd-obj_id").val());
      $("#cd-obj_id").val('');
    } 
    $("#cd-reset").show();
    data.obj_id = 0;
  }

  var options = {};
  var modal = "#cal_dialog"; // '#calModal'
  var $modal = $(modal);

  $("#cd-tab1 A").trigger('click');
  console.log("cal_booking_dialog in=" + checkin + " out=" + checkout + " data=", data);
  $modal.modal(options);
}

/** handler changing existing fee */
$(document).on('change', '.fee-field', function(e) {
  cal_update_fees(); 
});        

$(document).on('change', '#cd-status', function(e) {
  var status = $(this).val();
  console.log("Status = " + status);
  cal_update_status(status);
});        

function cal_update_status(status) {  
  if(status == 35) { // blocked
    $("#cd-tab2").hide();
    $("#cd-tab3").hide();
    $("#cal_dialog .cd-hide-block").hide();    
  } else {
    $("#cal_dialog .cd-tab").show();
    $("#cal_dialog .cd-hide-block").show();
  }  
}

/** reset button - fetch live rate */
$(document).on('click', '#cd-reset', function(e) {
  console.log("Rest form");
  var form_id = 'calendar_dialog_form';
  var $messages = form_messages(form_id); // get message div      
  $messages.remove();
  
  reset_form(form_id, true);
  cal_reset_fees();
  
  $("#cd-rate_locked").prop('checked', false); // unlock rate lock
  cal_update_rate(); /** fetches live rates if rate_lock is false */ 
});

/** handler for adding/removing fee */
$(document).on('click', '.btn-add-fee', function(e) {
  var controlForm = $('#cd-fee-inputs'),
      currentEntry = $(this).parents('.entry:first');

  var amount = currentEntry.find('.fee-amount').val().trim();
  var amt = amount ? parseFloat(amount) : 0;  
  var type = currentEntry.find('.fee-type').val();
  var desc = currentEntry.find('.fee-description').val().trim();
  var len = desc.length;
  
  console.log("Len=" + len + " Amount=" + amount + " type=" + type + " desc=" + desc + " zero:" + (amt ==0 ? 'yes':'no'));

  if(!type) {
    alert("Type is required");
    return false;
  }
  
  if(!desc.length && (type==180 || type==110)) {
  //if(!desc.length && (type==180 || type==110 || amount==0)) {
    alert("Description is required if amount is 'Other' or 'Service'");
    return false;
  }
  
  if(isNaN(amt)) {
    alert(amount + " is not a number");
    return false;
  }

  console.log("add fee");
  e.preventDefault();
  var newEntry = $(currentEntry.clone()).appendTo(controlForm);
  
  newEntry.find('input').val('');
  //controlForm.find('.entry:not(:last) INPUT').
  
  controlForm.find('.entry:not(:last) .btn-add-fee')
    .removeClass('btn-add-fee').addClass('btn-remove-fee')
    .removeClass('btn-success').addClass('btn-danger')
    .html('<span class="glyphicon glyphicon-minus"></span>');
    
  cal_update_fees(); 
  return false;  
}).on('click', '.btn-remove-fee', function(e) {
  console.log("remove fee");
  $(this).parents('.entry:first').remove();
  e.preventDefault();
  cal_update_fees(); 
  return false;
});

function cal_update_fees() {
  var feeForm = $('#cd-fee-inputs');
  var feeAr = [];
  var fees = 0;
  var feeObj,quantity,amount,total,desc,type,tax,currentEntry;
  var $fees = feeForm.find('.entry');
  var $button;
  $.each($fees, function(i,k) {
    currentEntry = $(this);
    type = currentEntry.find('.fee-type').val();
    $button = currentEntry.find('.fee-button');
    console.log("button class=" + $button.attr('class'));
    if($button.hasClass('btn-remove-fee')) { /** already been added */
      amount = currentEntry.find('.fee-amount').val().trim();
      amount = amount ? parseFloat(amount) : 0;  
      desc = currentEntry.find('.fee-description').val().trim();
      tax = currentEntry.find('.fee-tax').val().trim();
      quantity = currentEntry.find('.fee-quantity').val().trim() || 1;
      total = parseFloat(quantity*amount);
      console.log("tax=" + tax + " Amount=" + amount + " Quantity=" + quantity + " Total=" + total + " type=" + type + " desc=" + desc + " zero:" + (amount ==0 ? 'yes':'no'));
      feeObj = {'type':type,'amount':amount,'quantity':quantity,'total':total,'tax':tax,'description':desc};
      feeAr.push(feeObj);
      fees += total;
    }
  });
  console.log("feeAr:", feeAr);
  var fee_json = JSON.stringify(feeAr);
  $('#cd-fee_json').val(fee_json);
  $('#cd-fees').val(fees);
  if($('#cd-fee_json_debug').length) $('#cd-fee_json_debug').val(fee_json);
  cal_update_total();
}

function edit_reservation($target) {
  $('#cal_menu').hide();
  var options = {};  
  var obj_id = $target.data('obj_id');
  var obj_type = $target.data('obj_type');
  var cache_index = obj_type + '-' + obj_id;
  var cached_data = g_cal_entries && Object.size(g_cal_entries) ? g_cal_entries[cache_index] : null;
  console.log("edit_res:  cached data index=" + cache_index + " size=" + Object.size(g_cal_entries) + " cached data=", cached_data);
  var data = cached_data || {"obj_type": $target.data('apt_id'), "obj_type": obj_type, "obj_id": obj_id, "apt_id": $target.data('apt_id'), "checkin": $target.data('in'), "checkout": $target.data('out')};
  
  //data.checkin = sql2date(data.checkin).format(g_moment_format);
  //data.checkout = sql2date(data.checkout).format(g_moment_format);
  
  
  console.log("edit_res: target=",$target.data());//checkin=", strip_time(data.checkin), " checkout=", strip_time(data.checkout));
  console.log("edit_res: cached_data=",cached_data);//checkin=", strip_time(data.checkin), " checkout=", strip_time(data.checkout));
  console.log("edit_res: data=",data);//checkin=", strip_time(data.checkin), " checkout=", strip_time(data.checkout));

  var cin = sql2date(strip_time(data.checkin)).format(g_moment_format);
  var cout = sql2date(strip_time(data.checkout)).format(g_moment_format);

  var range1 = moment.range(strip_time(data.checkin), strip_time(data.checkout));
  var num_days = range1.diff('days');
  $('#cd-num_days').val(num_days);
  
  console.log("after: in=", cin, " out=", cout, " days=" + num_days);
  
  data.checkin = cin;
  data.checkout = cout;
  console.log("edit_res: data=", data);
  
  cal_booking_dialog(data);
  return false;
  
}


/** after showing dialog - bind date handler, then unbind on close */
/** This because of double firing and expensive ajax calls */
$(document).on('shown.bs.modal', '#cal_dialog', function(){
  console.log("showing modal - enable update period");
  
  cal_bind_date_handlers();
  cal_update_title();
  
  $(this).on('hidden.bs.modal', function(){
    var visible = $("#cal_dialog").hasClass('in') ? 1 : 0;
    console.log("inline hidden modal - disable update period name=" + $(this).attr('id') + " visible=" + visible);
    $("#cd-checkout").unbind('change');
    $("#cd-checkin").unbind('change');
  });
  
});

function cal_bind_date_handlers() {
  var cin = $("#cd-checkin").val();
  var cout = $("#cd-checkout").val();
  var rid = $("#cd-obj_id").val();
  console.log("cal_bind_date_handlers: rid=" + rid + " in=" + cin + " out=" + cout);
  
  $("#cd-checkin").datepicker('setDate', cin);
  $("#cd-checkout").datepicker('setDate', cout).datepicker('hide');  
  // if(!rid) 
  cal_update_period($("#booking_dates"));
  
  
  $("#cd-checkin").on('change', function() {
    var update = 1; /** if dialog exists, only update if visible */
    if($("#cal_dialog").length) update = $("#cal_dialog").hasClass('in') ? 1 : 0;         
    console.log("checkin changed - update period modal update = " + update);
    if(update) cal_update_period($("#booking_dates"));      
  });

  $("#cd-checkout").on('change', function() {
    var update = 1; /** if dialog exists, only update if visible */
    if($("#cal_dialog").length) update = $("#cal_dialog").hasClass('in') ? 1 : 0;         
    console.log("checkin changed - update period modal update = " + update);
    if(update) cal_update_period($("#booking_dates"));      
  });
}

/** handler for edit reservation - link (simple dialog) */
$(document).on('click', '.new-res', function(event) {
  var data = {};
  
  /** from simple dialog */ 
  var $form = $(this).closest('form');
  if($form.length) { 
    data = $form.serializeObject();
    var form_id = $form.attr('id');
    console.log("Found form " + form_id + " with data=", data);
    var moment_in = moment(data.checkin, g_moment_format);
    var moment_out = moment(data.checkin, g_moment_format);

    /** convert to sql - that's what cal_booking_dialog likes */
    var checkin = data.checkin = moment_in.format(g_moment_sql_format);
    var checkout = data.checkout = moment_out.format(g_moment_sql_format);
  } else { 
    /** todo: use this month (or preserve form ?) */
    //var checkin = data.checkin = moment().format(g_moment_sql_format);
    //var checkout = data.checkout = 
    var checkin = '';
    var checkout = '';
    //var data = {"obj_type": g_cal_params.obj_type, "apt_id": 0, "checkin": checkin, "checkout": checkout};
    var data = {"obj_type": g_cal_params.obj_type};
  }
  
  
  cal_booking_dialog(data, false); /** don't reset form */
  
  return false;
});

/** handler for edit reservation - link */
$(document).on('click', '.edit_res', function(event) {
  edit_reservation($(this));
  return false;
});

/** dialog: handler to watch for change of rate_base/rate_type */ 
$(document).on("change", ".cal-rate-field", function() {
  var name = $(this).attr('name');
  console.log(name + " changed, lock rate");
  $('#cd-rate_locked').prop('checked', true);
  cal_update_total();  
});

/** dialog: handler to watch for change of title affecting fields */ 
$(document).on("change", ".cal-update-title", function() {
  var name = $(this).attr('name');
  console.log(name + " changed, update title");
  cal_update_title();
});

/** dialog: handler to watch for change of rate affecting fields */ 
$(document).on("change", ".cal-update-rate", function() {
  var name = $(this).attr('name');
  console.log(name + " changed, update rate");
  //cal_update_title();
  cal_update_rate();
});

$(document).on("change", ".cal-update-total", function() {
  var name = $(this).attr('name');
  console.log(name + " changed, update total");
  cal_update_total();
});

$(document).on("change", "#cd-charge_tax", function() {
  var name = $(this).attr('name');
  console.log(name + " changed, update total");
  if($(this).prop('checked')) {
    $("#cd-tax_included_row").show();
  } else {
    $("#cd-tax_included_row").hide();
  }
 
  cal_update_total();
  
});

/** dialog: handler to watch for change VAT included checkbox */ 
$(document).on("change", "#cd-tax_included", function() {
  var checked = $(this).prop('checked') ? 1 : 0;
  var tax_rate_low = $("#cd-tax_rate_low").val() || 1.06;
  var tax_rate_high = $("#cd-tax_rate_high").val() || 1.21;  
  var tax_rate, tax_val;
  
  var $fields = $('.cal-amount-field')
  var val,new_val,tax_val;
  console.log("VAT change, checked=" + checked + " len=" + $fields.length);
  $.each($fields, function(i,fld) {
    val = $(this).val();
    tax_rate = tax_rate_low; /** todo: use hi/low/none for fees */      
    $parent = $(this).closest('.entry');
    $tax_field = $(this).closest('.entry').find('.tax-rate-field');
    if($tax_field.length) tax_val = $tax_field.val();
    else tax_val = "L";
    switch(tax_val) {
      case 'H':
        tax_rate = tax_rate_high;
        break;
      case 'L':
        tax_rate = tax_rate_low;
        break;
      case 'N':
        tax_rate = 0;
        break;
      default:
        tax_rate = 0;
        break;        
    }
    if(!tax_rate) return true;
    
    console.log("fld=" + $(this).attr('name') + " val=" + val + " taxlen=" + $tax_field.length + " tax=" + tax_rate + " plen=" + $parent.length);
    if(val.indexOf('%') > -1 || isNaN(parseFloat(val))) {
      console.log("fld=" + $(this).attr('name') + " val=" + val + " is % or not a number");
      return true;
    }            
    
    $tax_field.length
    new_val = checked ? val*tax_rate : val/tax_rate;
    console.log("fld=" + $(this).attr('name') + " val=" + val + " new=" + new_val);
    $(this).val(formatFloat(new_val));
  });
  cal_update_fees();
  //cal_update_total(); (called by cal_update_fees())
});

function cal_reset_fees() {  
  var $controlForm = $('#cd-fee-inputs');
  var $entries = $controlForm.find('.entry');
  var $entry, $btn;
  $.each($entries, function(i, entry) {
    $entry = $(this);
    $btn = $entry.find('.fee-button');
    if($btn.hasClass('btn-remove-fee')) $btn.trigger('click');
  });  
}

/** dialog: update title */ 
function cal_update_title(title) {
  if(typeof title == "undefined") var title = dialog_title();  
  var $title = $('#cal_dialog .overlay-title');
  $title.html(title);
}

/** dialog: called when dates change */ 
function cal_update_period($dp) {
  if(!$dp) return;
  console.log("cal: update period: range id=" + $dp.attr('id') + " len=" + $dp.length);
  datepicker_set_duration($dp); // sets num days
  var nd = datepicker_get_duration($dp, 'days'); // get number of months
  var nm = datepicker_get_duration($dp, 'months'); // get number of months
  console.log("cal_update_period: days=" + nd + " months=" + nm);
  $("#cd-num_months").val(nm);
  $("#cd-num_days").val(nd);
  cal_update_title();
  cal_update_rate();
}

function cal_update_rate() {
  var status = $('#cd-status').val();
  if(status == 35) return;
  var locked = $("#cd-rate_locked").prop('checked') ? 1 : 0;
  console.log("cal_update_rate: locked=" + locked)
  if(locked) cal_update_total();
  else cal_fetch_rate();
}

/** dialog: update total based on current rates */
function cal_update_total() {
  var data = dialog_data('calendar_dialog_form');

  var rate_type = $("#cd-rate_type").val();
  var rate_base = $("#cd-rate_base").val();
  var discount_str = $("#cd-discount").val();
  if(discount_str == '0') {
    $("#cd-discount").val('');
    discount_str = '';
  }
  
  var discount = 0;
  var feeAr = [];
  var fee_json = $('#cd-fee_json').val();
  if(fee_json) feeAr = $.parseJSON(fee_json);
  
  switch(rate_type) {
  case 'day':
    var count = $("#cd-num_days").val();
    break;
  case 'month':
    count = $("#cd-num_months").val();
    break;
  case 'total':
    count = 1;
    total = rate_base;
    break;
  default:
    alert("Unsupported rate type " + rate_type);
    break;
  }
  
  /** (sub) total for stay */
  var total = 0;
  total = count * rate_base;

  
  /** discount */
  var total_discounted = total;
  if(discount_str) {
    var discount = parseFloat(discount_str);
    if(discount > 0) {
      var discount_rate = discount_str.indexOf('%') > -1 ? rate_base * (100-discount)/100 : rate_base - discount;
      console.log("discount_str=" + discount_str + " base=" + rate_base + " discount_rate=" + discount_rate);
      total_discounted = count * discount_rate;
      total = total_discounted;
    } else {
      alert(discount_str + " is not a valid discount");
    }
  }

  /** total ex VAT */  
  var tax_rate_low = $("#cd-tax_rate_low").val() || 1.06;
  var tax_rate_high = $("#cd-tax_rate_high").val() || 1.21;    
  var charge_tax = $("#cd-charge_tax").prop('checked') ? 1 : 0;
  var tax_included = $("#cd-tax_included").prop('checked') ? 1 : 0;
  var total_discounted_ex_tax = charge_tax && tax_included ? total_discounted / tax_rate_low : total_discounted; /** todo: maybe high rate in other countries ? */

  /**  tax */
  var tax = 0;
  var tax_of_which = 0;
  var tax_hi = tax_rate_high - 1;
  var tax_lo = tax_rate_low - 1;
  if(charge_tax) {
    
    /** tax on the rent */
    if(tax_included) {
      tax_of_which = total_discounted - total_discounted_ex_tax;
      console.log("rent of which tax=" + tax);
    } else {
      tax = total_discounted * tax_lo;
      console.log("rent tax=" + tax);
    }

    /** tax on fees */
    if(feeAr.length) {
      var fee_tax, fee_total;
      console.log("FeeArray=", feeAr);
      $.each(feeAr, function(i, obj) {
        fee_tax = obj.tax;
        fee_total = obj.total;
        if(tax_included) {
          if(fee_tax == 'H') tax_of_which += fee_total/tax_rate_high;
          if(fee_tax == 'L') tax_of_which += fee_total/tax_rate_low;
        } else {
          if(fee_tax == 'H') tax += fee_total * tax_hi;
          if(fee_tax == 'L') tax += fee_total * tax_lo;
        }
        console.log("Fee: total=" + fee_total + " tax=" + fee_tax + " tax=" + tax + " tow=" + tax_of_which);
      });     
    }
  }

  /** fees */
  var fees = parseFloat($("#cd-fees").val()) || 0;
  
  /** city tax */
  var city_tax = 0;
  var city_tax_rate = $("#cd-city_tax_rate").val();
  var charge_city_tax = $("#cd-charge_city_tax").prop('checked') ? 1 : 0;
  console.log("Charge city tax? charge_city_tax=" + charge_city_tax + " rate=" + city_tax_rate);
  if(charge_city_tax && city_tax_rate) {    
    var tax_percentage = parseFloat(city_tax_rate);
    if(!isNaN(tax_percentage)) {
      city_tax = parseFloat(total_discounted_ex_tax * (tax_percentage/100));
      console.log("OK. Charge city tax: total=" + total + " total_ex=" + total_discounted_ex_tax + " rate=" + city_tax_rate + " perc=" + tax_percentage + " tax=" + city_tax);
    }
  }
  
  /** service fee */
  var service_fee = 0;

  /** grand total */
  var grand_total = 0;
  grand_total = total + tax + city_tax + fees + service_fee;

  console.log("cal_update_total rate_type=" + rate_type + " base=" + rate_base + " count=" + count + " total=" + total + " gt=" + grand_total);
  
  $("#cd-service_fee").val(service_fee);    
  //$("#cd-fees").val(fees);    
  $("#cd-city_tax").val(city_tax);    
  $("#cd-total").val(total);
  $("#cd-grand_total").val(grand_total);
  $("#cd-tax").val(tax);    
  $("#cd-tax_of_which").val(tax_of_which);    
  
  cal_update_rate_text();  
}

/** dialog: ajax call to fetch live booking */
function cal_fetch_object(obj_type, obj_id) {
  console.log("Fetching live " + obj_type + " " + obj_id);
  var ajax_url = "/ajax.php?oper=rental-calendar-get&obj_type=" + obj_type + "&obj_id=" + obj_id;
  $("#dt_total").hide();
  $("#dt_spinner").show();
  $("#cd-save").prop("disabled", true);
  
  cal_reset_fees();
  
  $.getJSON(ajax_url, function(response) { // start loading data from server
    $("#dt_total").show();
    $("#dt_spinner").hide();
    $("#cd-save").prop("disabled", false);

    var form_id = 'calendar_dialog_form';
    if(response.success) {
      var cache_index = obj_type + '-' + obj_id;
      var data = response.data;
      data.obj_type = obj_type;
      data.obj_id = obj_id;
      //if(!data.city_tax_rate) delete data.city_tax_rate;
      
      var cin = data.checkin;
      var cout = data.checkout;
      
      /** convert to human */
      var checkin =  moment(data.checkin).format(g_moment_format);
      var checkout = moment(data.checkout).format(g_moment_format);
      
      console.log("Loaded in/out=" + cin + '/' + cout);
      console.log("Now in/out=" + checkin + '/' + checkout + ' days=' + data.num_days);
      console.log("Loaded data:", data);
      
      data.checkin = checkin;
      data.checkout = checkout;
      
      g_cal_entries[cache_index] = data; // cache with client
      fill_form(form_id, data);

      cal_update_rate_text();
      cal_update_fee_rows(data);

      $('#cd-num_days').val(data.num_days);
      
      
      var locked_invoices = data.locked_invoices || [];
      var invoice_locked = locked_invoices.length
      console.log(invoice_locked + " locked invoices:", locked_invoices);
      if(invoice_locked) {
        $(".cal-rate-field").prop("disabled", true);
        $("#cd-invoice_locked_text").show();
      } else {
        $(".cal-rate-field").prop("disabled", false);
        $("#cd-invoice_locked_text").hide();
      }
      
    } else {
      var $form = $('#' + form_id);
      var $messages = form_messages(form_id); // get message div      
      $messages.html(bootstrap_error_message(response.error));          
    }
  }); 
}

/** dialog: ajax call to fetch rate for new bookings (or if changed and rate is not locked) */ 
function cal_fetch_rate() {
  
  if($("#cd-rate_locked").prop('checked')) {
    console.log("cal_update_rate: rate is locked");
    cal_update_total();
    return;
  }
  
  console.log("/n/n/expensive ajax: cal: update rate");
  
  var cin = $("#cd-checkin").val();  
  var out = $("#cd-checkout").val();  
  var ng = $("#cd-num_guests").val();  
  var apt_id = $("#cd-apt_id").val();
  
  if(!cin || !out || !apt_id) {
    console.log("missing in or out - returning");
    return;
  }

  
  var ajax_url = "/ajax.php?oper=rental-calendar-rate&apt_id=" + apt_id + "&in=" + cin + "&out=" + out + "&ng=" + ng;
  $("#cd-save").prop("disabled", true);
  cal_reset_fees();
  
  $("#dt_total").hide();
  $("#dt_spinner").show();  
  $.getJSON(ajax_url, function(response) { // start loading data from server
    if(response.success) {
      var result = response.data;
      console.log("Fetched rate:", result);
      $("#cd-save").prop("disabled", false);
      $("#dialog-total-loading").hide();
      $("#dialog-total").show();
      
      cal_update_rate_tab(result); // set form data      
      cal_update_rate_text(); // print in footer
    } else {
      
    }
  });
}

/** dialog: update the rate tab with live ajax rate data */
function cal_update_rate_tab(result) {
  $("#cd-rate_type").val(result.unit);
  $("#cd-rate_base").val(result.average);
  $("#cd-charge_city_tax").prop('checked', result.charge_city_tax);
  $("#cd-tax_included").prop('checked', result.tax_included);
  $("#cd-bill_monthly").prop('checked', result.bill_monthly);
  $("#cd-charge_tax").prop('checked', result.charge_tax);

  if(result.corporate) {
    $("#cd-corporate").show();
  } else {
    $("#cd-corporate").hide();
  }

  if(result.charge_tax) {
    $("#cd-tax_included_row").show();
  } else {
    $("#cd-tax_included_row").hide();
  }
  
  var ar = ['rate_class', 'currency', 'fees', 'deposit_percentage', 'commission', 'city_tax', 'city_tax_rate', 'service_fee', 'total', 'discount', 'grand_total', 'agent_id', 'site_id', 'legal_entity_id', 'org_id'];
  $.each(ar, function(i, key) {
    //console.log("setting " + key + "=" + result[key]);
    $("#cd-" + key).val(result[key]);
  });
  
  console.log("cal_update_rate_tab: result=", result);
 
  cal_update_fee_rows();
  
}

/** set fee rows from data */
function cal_update_fee_rows(result) {
  console.log("cal_update_fees result:", result);

  if(typeof result == "undefined") {
    var result = {}; /** booking-status-edit */
    result.fee_json = $("#cd-fee_json").val();
    console.log("cal_update_fee_rows: no result, using " + result.fee_json);
  }
  var feeRows = result.fee_rows || []; /** new bookings - from get-rate */
  if(!feeRows.length && result.fee_json) { /** existing bookings */
    feeRows = jQuery.parseJSON(result.fee_json);
    console.log("parsed json:" + result.fee_json);
    console.log("fee rows parsed:", feeRows);
  }
  
  if(feeRows && feeRows.length) {
    var $controlForm = $('#cd-fee-inputs');
    var $lastEntry,$input,$btn;
    $.each(feeRows, function(i, row) {
        $lastEntry = $controlForm.find('.entry:last');
        $.each(row, function(key, val) {
          $input = $lastEntry.find('.fee-' + key).val(val);  
          console.log("fee row: " + i + " setting " + key + "=" + val + " inputlen=" + $input.length);
        });
        var $btn = $lastEntry.find('.btn-add-fee');
        $btn.trigger('click');
        console.log("trigger click on btn with len=" + $btn.length);
    });
  
  }
}

/** dialog: update the printed rate in the dialog footer */
function cal_update_rate_text(form_id) {
  if(typeof form_id == "undefined") var form_id = 'calendar_dialog_form';
  
  var rate_base = $("#cd-rate_base").val();
  var rate_class = $("#cd-rate_class").val();
  var rate_type = $("#cd-rate_type").val();
  var currency = $("#cd-currency").val();
  var total = $("#cd-total").val();
  var tax = $("#cd-tax").val();
  var tax_of_which = $("#cd-tax_of_which").val();
  
  var grand_total = $("#cd-grand_total").val();
  
  var quantity = 1;

  var city_tax = $("#cd-city_tax").val();
  var charge_city_tax = $("#cd-charge_city_tax").prop('checked') ? 1 : 0;
  
  var fees = $("#cd-fees").val();
  var service_fee = $("#cd-service_fee").val();
  var discount = $("#cd-discount").val();

  if(city_tax > 0 && !charge_city_tax) {
    var $form = $('#' + form_id);
    var $messages = form_messages(form_id);      
    $messages.html(bootstrap_error_message("Charge city tax is not set, but booking has " + currency2symbol(currency) + city_tax + " city tax. If you save the booking, the city tax will be removed."));          
  }
  
  $("#dt_spinner").hide();
  if(rate_type && rate_base > 0) {
    switch(rate_type) {
      case 'day':
        $("#dt_quantity").show();
        quantity = $("#cd-num_days").val();
        break;
      case 'month':
        $("#dt_quantity").show();
        quantity = formatFloat(parseFloat($("#cd-num_months").val()));
        break;
      case 'total':
        $("#dt_quantity").hide();
        quantity = 1;
        break;
    }
    $("#dt_total").show();
    $(".reservation-quantity").html(quantity);
    $(".reservation-currency").html(currency2symbol(currency));
    $(".reservation-rate").html(formatFloat(rate_base));
    $(".reservation-total").html(formatFloat(grand_total));
    $(".reservation-rate_type").html(rate_type);
    $(".reservation-discount").html(discount);
  } else {
    $("#dt_total").hide();
    return;
  }
  
  $(".reservation-city_tax").html(formatFloat(city_tax));  
  $(".reservation-tax").html(formatFloat(tax));  
  $(".reservation-fees").html(formatFloat(fees));
  $(".reservation-service_fee").html(formatFloat(service_fee));

  if(fees > 0) $("#dt_fees").show(); else $("#dt_fees").hide();   
  if(city_tax > 0)  $("#dt_city_tax").show(); else $("#dt_city_tax").hide();   
  if(service_fee > 0)  $("#dt_service_fee").show(); else $("#dt_service_fee").hide();   
  if(discount) {
    if(discount.indexOf('%') > -1) {
      $("#dt_discount .reservation-currency").hide();
      $("#dt_discount_unit").hide();
    } else {
      $("#dt_discount .reservation-currency").show();
      $("#dt_discount_unit").show();
    }    
    $("#dt_discount").show();
  } else {
    $("#dt_discount").hide();
  }
  if($("#cd-charge_tax").prop('checked')) {
    console.log("Charge tax");
    $("#dt_ch_tax").show();
    if($("#cd-tax_included").prop('checked')) {
      $("#dt_tax").hide(); 

      $("#dt_ex_tax").hide(); 
      $("#dt_in_tax").show(); 
    } else {
      $("#dt_tax").show(); 

      $("#dt_ex_tax").show();      
      $("#dt_in_tax").hide(); 
    }  
  } else {
    console.log("Do not Charge tax");
    $("#dt_tax").hide();
    $("#dt_ch_tax").hide();
    $("#dt_ex_tax").hide(); 
    $("#dt_in_tax").hide(); 
  }
  
  return; /** old code */ 
}


/** dialog: get form data as js object */
function dialog_data(form_id) {
  if(typeof form_id == "undefined") var form_id = 'calendar_dialog_form';
  return form_data(form_id);
	//var checkin   = $("#" + form_id + " input[name=checkin]").val();
	//var checkout  = $("#" + form_id + " input[name=checkout]").val();
	//var hourly  = $("#" + form_id + " input[name=hourly]").val();
	//var apt_id  = $("#" + form_id + " input[name=apt_id]").val();
	//var res_id  = $("#" + form_id + " input[name=res_id]").val();
	//var obj_type  = $("#" + form_id + " input[name=obj_type]").val();
	//var apartment_name  = $("#" + form_id + " input[name=apartment_name]").val();
	//var first_name  = $("#" + form_id + " input[name=first_name]").val();
	//var last_name  = $("#" + form_id + " input[name=last_name]").val();
	//var salutation  = $("#" + form_id + " input[name=salutation]").val();
	//return data;
}
  
/** dialog: set title in header */
function dialog_title(form_id) {
  if(typeof form_id == "undefined") var form_id = 'calendar_dialog_form';
  var data = form_data(form_id);
  //console.log("dialog_title: Dialog data:",data);
  
  var apt_name = data.apt_id ? $('#cd-apt_id').find(":selected").data('name') : '';
  var nameAr = [data.salutation, data.first_name, data.last_name];
  var name_str = data.salutation || data.first_name || data.last_name ? nameAr.join(' ') : '';
  
  var action = data.obj_id && data.obj_id != 0 ? 'Edit ' + data.obj_type + ' ' + data.obj_id : 'New ' + data.obj_type;
  var source_obj_id = $("#cd-source_obj_id").val();
  
  if(source_obj_id) {
    action = "Copy Booking " + source_obj_id;
  }
  
  var checkin = $("#cd-checkin").val();
  
  var title = '<h2>' + action + '</h2>'; 
  var count = '', whowhere  = '';
  
  // who, where
  if(name_str && apt_name) whowhere = name_str + '@' + apt_name;
  else whowhere = name_str ? name_str : apt_name;
  if(whowhere) title = title + whowhere + ': ';
  
  // when
  if(checkin) {
    var moment_in = moment(checkin, g_moment_format);
    title = title + moment_in.format('D MMM YYYY')  + ' ';
  }
  
  // num guests, days
  var num_guests = $("#cd-num_guests").val();
  var ng = num_guests && num_guests > 1 ? num_guests + 'pp' : '';
  var nd = data.num_days ? data.num_days + (data.num_days == 1 ? ' day' : ' days') : '';
  if(ng && nd) count = ng + ', ' + nd;
  else count = ng ? ng : nd;
  if(count) title = title + ' (' + count + ')';
  
  return title;
}

/** dialog: watch period changed  
$(document).on("change", ".cal-update-period", function() {
  var name = $(this).attr('name');
  console.log(name + " changed, update period");
  cal_update_title();
  cal_update_period($("#booking_dates"));
});
*/

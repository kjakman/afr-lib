<?php

// returns name of product variant for screen printing
function get_variant_name($variantObj_or_id, $productObj_or_id = NULL) {
  $variantObj = any2obj('product_variant', $variantObj_or_id);
  if($variantObj) return $variantObj->name;
  $productObj = $productObj_or_id ? any2obj('product', $productObj_or_id) : get_object('product', $variantObj->product_id);
  if($productObj) return $productObj->name;
  return '';

  // $name = $variantObj->name ?: $productObj->name;
  // $name .= ' ('.phrase($variantObj->option_value, CAPITALIZE).' '.phrase($variantObj->option_name).')';
  // return $name;
}

// returns database value of product variant (e.g. "low_resolution");
function get_variant_value($variantObj_or_id, $productObj_or_id = NULL) {
  $variantObj = any2obj('product_variant', $variantObj_or_id);
  if(!$variantObj) return '';
  $productObj = $productObj_or_id ? any2obj('product', $productObj_or_id) : get_object('product', $variantObj->product_id);
  if(!$productObj) return '';
  return $variantObj->option_value.'_'.$variantObj->option_name;
}

function find_product_variant($product_id, $option_id, $value) {
  return find_object('product_variant', array('product_id' => $product_id, 'option_id' => $option_id, 'option_value' => $value));
}

function add_product_variants($productObj_or_id, $option_id, $options = array()) {
  $productObj = any2obj('product', $productObj_or_id);
  $product_id = $productObj->id;
  $optionObj = get_object('product_option', $option_id);
  $prices = $options['prices'];
  $values = $options['values'];
  
  $data = (array) $productObj;
  $outputAr = array();
  if($optionObj) {
    if(!$values) $values = explode(',', $optionObj->values);
    if(!$prices) $prices = array_fill(0, count($values), $productObj->price);
    //dump($values);dump($prices);die();
    $i = 0;
    foreach($values as $value) {
      $data = array();
      $data['price'] = $variant_price = $prices[$i];
      $data['name'] = $variant_name = $productObj->name." ($value $option_name)";
      $variantObj = find_product_variant($product_id, $option_id, $value);
      if($variantObj) {
        $outputAr[] = "Found variant $variantObj->name for ($product_id/$option_id/$value)";
        $variant_id = $variantObj->id;
        $variant_name = $variantObj->name;
        list($id, $errors) = update_object('product_variant', $data, $variant_id);
        $action = 'Update';
      } else {
        $outputAr[] = "Did not find variant for ($product_id/$option_id/$value)";
        $data['product_id'] = $product_id;
        $data['option_id'] = $option_id;
        $data['option_name'] = $option_name = $optionObj->name;
        $data['option_value'] = $value;
        list($id, $errors) = add_object('product_variant', $data);
        $action = 'Add';
      }        
      if($id) $outputAr[] = "$action variant $variant_name ($variant_price)";
      if($errors) {dump($errors);die("Failed to $action product");}
      $i++;
    }
  }
  return implode(html_break(), $outputAr).html_break();
}

function product_has_variant($product_id) {
  return $product_id ? db_getOne("SELECT count(*) FROM product_variant WHERE product_id=$product_id AND active=1") : 0;
}

// session function (with DB sideeffects if logged in)
// create cart in Session (and db if logged in), add one row from $_GET, called from app_data
function cart_add_row($siteObj, $input, $options=NULL) {
  extract($input);
  global $u_id;
  $user_id = $u_id; // use global, not input (easy to hack) 
  $json = isset($options['json']) ? $options['json'] : true; // called from app_data or directly ?
  $json_options = array('json' => $json); 
  $client_id = $siteObj->client_id;
  
  if(!$product_id) {
    //if(!$parent_id || !$parent_type) mydie("No product id or parent id: ".dump($input, true));
    if(!$parent_id) json_error("No product id or parent id", $json_options);
    if(!$parent_type) json_error("No product id or parent type", $json_options);
  }
  $variantObj = NULL;
  if($variant_id) {
    $variantObj = get_object('product_variant', $variant_id);
    $product_id = $variantObj->product_id;
  }
  if(!$product_id) { // find or add product
    if($productObj = find_object('product', array('client_id' => $client_id, 'source_type' => $parent_type, 'fid' => $parent_id))) { // look for existing products
      $product_id = $productObj->id;
    } else {
      $productData = obj2product($site_id, $parent_type, $parent_id);
      $productData['client_id'] = $client_id;
      list($product_id, $errors) = add_object('product', $productData); // make new product
      if($errors) json_error($errors);
      if($product_id) {
        $productObj = array2obj($productData);
      }
    }
  }      
  if(!$product_id) json_error("No product id".dump($message, true), $json_options);    
  if(!$productObj) $productObj = get_object('product', $product_id);
  if($variant_id) $productObj->variant_id = $variant_id;
  
  $cartObj = get_cart($siteObj); // get the cart from session or DB
  //dump($cartObj);
  
  $cart_id = $cartObj->id;


  //$messages[] = "Cart: $cart_id product: $product_id Row: $row_id quantity = $quantity";
  $cart_link = $options['cart_link'] ?: site_page_link($siteObj, PAGE_CART);
  $cart_link_str = html_link($cart_link, phrase('cart'));    
  
  // add row to db
  if($cartObj->items[$product_id]) {
    $messages[] = "$productObj->name is already in $cart_link_str.";
  } else if($cart_id > 0) {
    list($row_id, $quantity, $errors) = add_cart_row($cart_id, $productObj, $variantObj);
    if($errors) json_error($errors, $json_options);
    if(!$row_id) json_error("Failed to add row", $json_options);
    $messages[] = "Added $productObj->name to $cart_link_str.";
  }

  // add row to session
  $rowObj = product2cartrow($productObj, $variantObj, $quantity > 0 ? $quantity : 1);
  if($row_id) $rowObj->row_id = $row_id;
  $cartObj->items[$product_id] = $rowObj;

  // remove all products where variant exists but is not set, except the product now being added.
  // this to avoid showing choose product variant for another product, which is confusing.
  $items = $cartObj->items;
  foreach($items as $item_pid => $item) {
    if($item_pid != $product_id && !$item->variant_id && product_has_variant($item_pid)) {
      // echo("Removing product $item_pid from cart, as variant was never chosen<br>");
      unset($cartObj->items[$item_pid]);
    }
  }
  
  $cartObj = cart_update($cartObj); // sets count and total
  if($user_id) $cartObj = save_cart($siteObj, $user_id, $cartObj);
  //json_error(dump($cartObj));
  // draw items, store
  $options['cart_link'] = $cart_link;
  $rows = cart_rows($siteObj, $cartObj, $options);
  $cartObj->items_html = html_table($rows, array('class' => 'cart_table', 'header' => 0, 'footer' => 1));

  
  
  //if($errors) echo(print_errors($errors));die();
  //echo(print_messages($messages));
  //die();
  return $cartObj;
}

// returns rows from orderObj
function order_rows($siteObj, $orderObj, $items=NULL, $options=NULL) {
  global $u_id;
  $rows = array();
  if(!$items) $items = find_objects('order_row', array('order_id' => $orderObj->id));
  $show_quant = false;
  foreach($items as $item) {
    if($item->quantity > 1 || $item->quantity < 1) {
      $show_quant = true;
      break;
    }
  }
  $rows[] = array(phrase('description', CAPITALIZE), phrase('amount', CAPITALIZE));
  if($show_quant) array_unshift($rows, '#');
  foreach($items as $item) {
    $name   = $item->description;
    $price  = $item->total;
    $row = array($name, $price);
    if($show_quant) array_unshift($row, clean_num($item->quantity));
    $rows[] = $row; 
  }
  return $rows;
}

function product_variant_chooser($site_id, $cartObj, $row) {
  $variant_id = $row->variant_id;
  $cur = get_currency_symbol($cartObj->currency ?: 'EUR', true);
  if(!$variant_id) return '';
  $variantObj = get_object('product_variant', $variant_id);
  $option_id = $variantObj->option_id;
  $product_id = $variantObj->product_id;
  $productObj = get_object('product', $product_id);
  $search = array('product_id' => $product_id, 'option_id' => $option_id);
  //return object_select('product_variant', 'product_variant', array('search' => $search));
  $variantObjs = find_objects('product_variant', array('product_id' => $product_id, 'option_id' => $option_id));
  foreach($variantObjs as $vObj) {
    $resolution = $vObj->option_name == 'resolution' ? $vObj->option_value : '';
    if($resolution != 'medium' && !($variantObj->price > 0)) continue; // cannot choose high res w/o price

    $vName = get_variant_name($vObj, $productObj);
    $rows[] = array($vObj->id, phrase($vObj->option_value).": ".$cur.$vObj->price, $site_id, $vObj->product_id, $vObj->price, $vName);
  }
  return form_select('variant_id', array('rows' => $rows, 'value' => $variant_id, 'class' => 'variant_chooser', 'data' => array('site_id', 'product_id', 'price', 'name'))); 
}

// returns rows from cartObj
function cart_rows($siteObj, $cartObj, $options=NULL) {
  global $u_id;
  $full = $options['full'];
  $compact = $options['compact'];
  $popup = $options['popup'];
  $debug = $options['debug'];

  $rows = array();
  $items = $cartObj->items ?: array();
  $remove_class = $popup ? 'remove_floating_cart' : 'remove_cart';
  $cur = get_currency_symbol($cartObj->currency ?: 'EUR', true);
  $has_variant = false;
  //if($full) foreach($items as $item) if($item->variant_id) $has_variant = true;
  foreach($items as $item) {
    $name   = $item->url ? html_link($item->url, $item->name) : $item->name;
    $photo  = $item->photo ? ($item->url ? html_image_link($item->url, $item->photo, $item->name, array('class' => 'cart_photo')) : html_image($item->photo, $item->name, array('class' => 'cart_photo'))) : '';
    $item_data = array('row_id' => $item->id, 'product_id' => $item->product_id, 'site_id' => $cartObj->site_id ?: $siteObj->id, 'user_id' => $u_id);
    if(!$compact) $x_link = html_link('#', $options['remove_text'] ?: phrase('remove', CAPITALIZE), array('class' => "delete_link $remove_class", 'data-data' => json_encode($item_data)));
    $price  = $cur.html_span($item->total ?: '0.00', 'row_total');
    $variant_chooser = '';
    if($full && $item->variant_id) {
      $variant_chooser = product_variant_chooser($siteObj->id, $cartObj, $item);
      $has_variant = true;
    }
    $row = $has_variant ? array($photo, $name, $variant_chooser, $price, $x_link) : array($photo, $name, $price, $x_link);
    if($debug) $row[] = $item->variant_id;
    $rows[] = $row; 
  }

  $total_str = $cur.html_span($cartObj->total ?: '0.00', 'cart_total');
  if($cart_link = $options['cart_link']) $count_str = " (".html_link($cart_link, html_span($cartObj->count ?: 0, 'cart_count')." ".($cartObj->count ==1 ? phrase('item') : phrase('items'))).")";  

  $rows[] = array('', phrase('total', CAPITALIZE).': '.$count_str, $total_str, '');
  return $rows;
}

// gets db cart
function find_cart($siteObj, $user_id) {
  return $user_id ? find_object('cart', array('site_id' => $siteObj->id, 'user_id' => $user_id, 'order_id' => 0)) : session_cart();
}

// gets cart from DB or adds one if doesn't exist, if no user, return session cart
function get_cart($siteObj, $cart_id=0, $options=NULL) {
  global $u_id;
  if(!$u_id) return session_cart();

  $cartObj = $cart_id ? get_object('cart', $cart_id) : find_cart($siteObj, $u_id);
  if(!$cartObj) $cartObj = add_cart($siteObj, $user_id); // no cart in DB, but we're logged in, so add it
  $rowObjs = find_objects('cart_row', array('cart_id' => $cartObj->id));  
  $items = array();
  foreach($rowObjs as $rowObj) $items[$rowObj->product_id] = $rowObj;
  $cartObj->items = $items;
  $cartObj->count = count($items);
  return $cartObj;
}

// returns session cart or creates one if doesn't exist
// if cartObj is passed, this is saved to session
function session_cart($cartObj=NULL) {
  if($cartObj) $_SESSION['cart'] = cart_update($cartObj);
  if($_SESSION['cart']) return $_SESSION['cart']; // already exists 
  $_SESSION['cart'] = (object) array('currency' => 'EUR', 'total' => '0.00', 'count' => 0, 'items' => array());
  return $_SESSION['cart'];
}

// DB helpers, user_id required

// add cart to DB
function add_cart($siteObj, $user_id, $cartObj=NULL) {
  if($cartObj = find_cart($siteObj, $user_id)) return $cartObj; // look for existing cart, shouldn't happen, but...
  $cartObj->site_id = $siteObj->id;
  $cartObj->user_id = $user_id;
  list($cart_id, $errors) = add_object('cart', (array) $cartObj);
  return get_object('cart', $cart_id);
}    

// sets count and total for object (no DB)
function cart_update($cartObj) {
  $total = 0;
  $items = $cartObj->items ?: array();
  foreach($items as &$itemObj) {
    $itemObj->total = number_format($itemObj->amount * $itemObj->quantity, 2, '.', '');
    $total += $itemObj->total;
    //echo("adding price for $itemObj->name ($itemObj->total)<br>");
  }
  $cartObj->total = number_format($total, 2, '.', '');
  $cartObj->count = count($cartObj->items);
  //echo("cart total =$total<br>");
  return $cartObj;
}

// save cartObj to DB, including line items

function save_cart($siteObj, $user_id, $cartObj) {
  if(!$user_id) return session_cart($cartObj); // saves to session
  $items = $cartObj->items ?: array();
  $cart_id = $cartObj->id;
  if(!$cart_id) {
    $dbCartObj = find_cart($siteObj, $user_id) ?: add_cart($siteObj, $user_id); // existing ?
    $cart_id = $cartObj->id = $dbCartObj->id;
  }
  foreach($items as $item) {
    //echo("added $item->product_id with v=$item->variant_id to cart $cart_id<br>");
    add_cart_row($cart_id, $item->product_id, $item->variant_id, $item->quantity ?: 1); // will add or update cart row
  }
  //$cartObj = cart_update($cartObj); // set total
  //list($cart_id, $errors) = update_object('cart', (array) $cartObj, $cart_id);

  return get_cart($siteObj, $cart_id);
}    


// prints cart. Must correspond to JS function build_cart
function print_cart($siteObj, $cartObj, $options = array()) {
  $options = (array) $options;
  $compact = $options['compact'];
  $debug = $options['debug'];
  
  if($popup = $options['popup']) { // cart in pop-up menu    
    $show_form = false;      
    $cart_link = $options['cart_link'] ?: site_page_link($siteObj, PAGE_CART); 
    if(!$cart_link) return "Could not find cart page"; // cart page is required
  } else {
    $show_form = true;
    $form_name = 'cart_form';
    $action = site_page_link($siteObj, PAGE_CHECKOUT);
    if(!$action) return "Could not find checkout page"; // checkout page required
  }
  
                  
  $options['id'] = $options['id'] ?: 'cart_icon';
  $options['class'] .= ' cart_icon ';
  $count = count($cartObj->items);
  $hide_class = $count > 0 ? '' : 'hidden';
  $show_class = $count > 0 ? 'hidden' : '';
  
  //$hide_class = '';
  //dump($cartObj); 
	//formatFloat($rate_base, FORMAT_IF_NEEDED)." ".$currency;

  //$total_html = phrase('total', CAPITALIZE).': '."$cartObj->currency $cartObj->total";
  //$cart .= html_div($total_html, 'cart_total', array('id' => 'cart_total'));  

  
  // floating cart
  if($cart_link) $options['cart_link'] = $cart_link;
  $rows = cart_rows($siteObj, $cartObj, $options);  
  
  $items_table = html_table($rows, array('class' => 'cart_table cart_hide '.$hide_class, 'header' => 0, 'footer' => 1));
   
  $items_table .= html_div(" [".phrase('empty', CAPITALIZE)."] ", 'cart_show '.$show_class);

  if($compact || $debug) {
    $output = $items_table;
  } else if($popup) {
    //$floating_cart = html_div(html_h3(phrase('shopping_cart', CAPITALIZE)).html_link($cart_link, phrase('view_cart', CAPITALIZE),array('class' => 'cart_hide '.$hide_class)), 'cart_header').html_close_icon();
    $floating_cart = html_div(html_link($cart_link, phrase('view_cart', CAPITALIZE),array('class' => 'cart_hide '.$hide_class)), 'cart_header').html_close_icon();
    
    $floating_cart .= html_div($items_table, 'cart_items', array('id' => 'cart_items'));  
    $floating_cart .= html_div(html_social_button($cart_link, phrase('check_out', CAPITALIZE), array('class' => 'green_back cart_hide '.$hide_class)), 'cart_footer');  
    $floating_cart = html_div($floating_cart, 'hidden click_hide show_over', array('id' => 'floating_cart'));
  
    //$cartObj->count = 48;
    $cart .= html_span($cartObj->count ?: 0, 'cart_count', array('id' => 'cart_count')); // this shows always, also when floating cart is hidden
    $hidden_class = $cartObj->count ? '' : ' hidden';
    $output = html_div($cart.$floating_cart, 'cart_icon'.$hidden_class, $options);
  } else { // cart page, show form
    $output = html_form($items_table, $form_name, $action, 'POST');
    
  }     
  //$output .= dump($rows, true);
  return $output;
}

//    $rowData = (array) $productObj;
//    $rowData['product_id'] = $productObj->id;
//    if($rowData['supplier_id'] = $productObj->supplier_id) $rowData['supplier_name'] = get_object('supplier', $productObj->supplier_id);
//    $rowData['variant_id'] = $variantObj->id;
//    $rowData['cart_id'] = $cart_id;
//    $rowData['quantity'] = $quantity ?: 1;
//    $rowData['amount'] = $variantObj ? $variantObj->price : $productObj->price;
//    $rowData['photo'] = $productObj->media ? print_media($productObj->media, array('size' => 'thumb', 'source_only' => true)) : '';
//    //$rowData['supplier_id'] = '';
//    
//    $rowData['url'] = '';
//    $rowData['name'] = $variantObj ? get_variant_name($variantObj, $productObj) : $productObj->name;
//    
//    //array('cart_id' => $cart_id, 'product_id' => $product_id, 'quantity' => $quantity, 'name' => $productObj->name, 'amount' => $productObj->price)

function product2cartrow($productObj, $variantObj=NULL, $quantity=1) {
  $rowObj->product_id = $productObj->id;
  $rowObj->variant_id = $variantObj->id;
  $rowObj->quantity = $quantity;
  $rowObj->amount = $amount = $variantObj ? $variantObj->price : $productObj->price;
  $rowObj->total = $amount * $quantity;
  $rowObj->url = '';
  $rowObj->photo = print_media($productObj->media, array('size' => 'thumb', 'source_only' => true));;
  if($rowObj->supplier_id = $productObj->supplier_id) $rowObj->supplier_name = get_object('supplier', $productObj->supplier_id);
  $rowObj->name = $variantObj ? get_variant_name($variantObj, $productObj) : $productObj->name;
  $rowObj->currency = $productObj->currency;
  return $rowObj;
}

// add product to existing DB cart
function add_cart_row($cart_id, $productObj_or_id, $variantObj_or_id = NULL, $quantity=1, $options=NULL) {
  $bump_quantity = $options['bump_quantity'];
  $unique_variant = $options['unique_variant'] = true; // if true, cannot have red or blue t-shirt, only one

  $productObj = any2obj('product', $productObj_or_id);
  $product_id = $productObj->id;  
  //if(!$variantObj_or_id && $productObj->variant_id) $variantObj_or_id = $productObj->variant_id;

  $variantObj = $variantObj_or_id ? any2obj('product_variant', $variantObj_or_id) : NULL;
  $variant_id = $variantObj->id ?: 0;
  
  $search = array('cart_id' => $cart_id, 'product_id' => $product_id);
  if(!$unique_variant && $variant_id) $search['variant_id'] = $variant_id;
  
  if($cartRowObj = find_object('cart_row', $search)) { // found, bump quantity or warn
    $id = $cartRowObj->id;
    if($unique_variant && $variant_id != $cartRowObj->variant_id) {
      $rowData = (array) product2cartrow($productObj, $variantObj, $quantity > 0 ? $quantity : 1);
      $rowData['cart_id'] = $cart_id;
      list($id, $errors) = update_object('cart_row', $rowData, $cartRowObj->id);
    } else if($bump_quantity) {
      $quantity = $cartRowObj->quantity + $quantity;
      list($id, $errors) = update_object('cart_row', array('quantity' => $quantity), $id);
    } else {
      $errors[] = "Already in basket";
    }
  } else { // add    
    $rowData = (array) product2cartrow($productObj, $variantObj, $quantity > 0 ? $quantity : 1);
    $rowData['cart_id'] = $cart_id;
    list($id, $errors) = add_object('cart_row', $rowData);
  }
  
  return array($id, $quantity, $errors);
}

// turns some other object into a product
// for now: assumes media - vpatina
function obj2product($siteObj_or_id, $parent_type, $parent_id, $options=NULL) {
  $siteObj = any2obj('site', $siteObj_or_id);
  $id = 0;
  $errors = array();
  $data = array();
  $data['fid'] = $parent_id;
  $data['source_type'] = $parent_type;
  $data['client_id'] = $siteObj->client_id;
  $data['currency'] = 'EUR';
  $data['tax_included'] = 1;
  $data['unit'] = 'EA';

  if($obj = get_object($parent_type, $parent_id)) {
    switch($parent_type) {
    case 'media':
      $data['price'] = $obj->digital_price ?: '4.99';
      $data['category_id'] = 1008; // Download
      $data['tax_category'] = 'H';
      $data['name'] = $obj->name ?: phrase('no_title', CAPITALIZE);
      $data['description'] = pick_first($obj->short_description, $obj->original_technique, $obj->comment);
      //$data['url'] = '';
      //$data['supplier'] = 'The artist';
      $data['image'] = print_media($obj, array('size' => 'thumb', 'source_only' => true));
      $data['media'] = $obj->id; // todo: copy image
      break;
    default:
      break;
    }
  }
  return $data;
}
  
function product_dialog($invObj_or_id=NULL) {
  //$invObj = any2obj('invoice', $invObj_or_id);
  //if(!$invObj) {
  //  $errors[] = "No invoice found";
  //} else {
  //  if($invObj->status < INV_STATUS_DRAFT) $errors[] = "Invoice status = ".get_object('inv_status', $invObj->status, 'name');
  //  if($invObj->status == INV_STATUS_COMPLETE) $errors[] = "This invoice has been paid in full and is closed.";
  //}
  
  global $__header;
  //$prodSearch[''] = 
  $form_name = 'product_form';
  $handler = 'add_product_handler';

  $onchange = 'product_update();';            

  //$fields['inv_id']  = form_input('inv_id', 'hidden', '', 6).;

  //$fields['product']  = get_select_box_off('product_id', 'product', $selected_key=0, $disabled="", $filters="", "name", $onchange, $size=30, $noajax=0);  
  $fields['product']  = object_select('product_id', 'product');
  $fields['currency']  = form_input('currency', 'text', 'EUR',  array('size' => 3, 'max' => 3, 'readonly' => true));

  $fields['price']  = form_input('amount', 'text', '', array('size' => 6)).' '.html_span('', 'bold', "id='tax_included_text'");
  $fields['quantity']  = form_input('quantity', 'text', '', array('size' => 2, 'max' => 6));
  $fields['discount']  = form_input('discount', 'text', '', array('size' => 4))." (Number or percentage)";
  $fields['subtotal']  = form_input('subtotal', 'text', '', array('size' => 4, 'readonly' => true));

  $fields['tax']  = object_select('tax_category', 'tax_category', array('filters' => array('active' => 1)));
  //$fields['tax']  = form_input('tax_percentage', 'text', '', 6, "readonly='readonly'").' %';
  $fields['total']  = form_input('total', 'text', '', array('size' => 6, 'readonly' => true)).' '.html_span('', 'bold', "id='tax_text'");
  $fields['description']  = form_input('description', 'text', '', array('size' => 30));

  // $fields['tax']  = form_input('amount', 'text', '', 6);
  // $fields['tax_of_which']  = form_input('amount', 'text', '', 6);

  $on_save = "onclick=\"save_object('$form_name', 'inv_lineitem', 'add', {'handler': '$handler', 'reload': 1, 'keepalive': 1}); return false;\"";
  $on_done = "onclick=\"save_object('$form_name', 'inv_lineitem', 'add', {'handler': '$handler', 'reload': 1, 'keepalive': 0}); \$('#product_dialog').dialog('close'); return false;\"";
  $on_cancel = "onclick=\"\$('#product_dialog').dialog('close'); return false;\"";
  $fields[' &nbsp ']  = form_input('save', 'submit', ' Ok ', array('js' => $on_save)).' '.form_input('cancel', 'submit', ' Close / Cancel ', array('js' => $on_cancel)); //. ' '.form_input('done', 'submit', ' Done ', 0, 0, $on_done);;

  //$fields['price']  = html_div('', 'bold', "id='price'");
  //$fields['total']  = html_div('', 'bold', "id='total'");
  $hidden_fields['type'] = LI_TYPE_PRODUCT;
  $hidden_fields['inv_id'] = 0;
  $hidden_fields['tax_included'] = 1;       
  
  $product_form = array2form($hidden_fields).html_fieldset($fields, 'fieldset_left_wide');
  $form = html_form($product_form, $form_name);         
  $output = html_div('', 'message', "id='product_form_messages'");
  // $output = html_div('', 'errors', "id='product_form_errors'");
  $output .= $form;

  $__header['onload'] .= '
    $("#product_id, #product_id_textbox").change(product_fetch);
    // $("#quantity").focus(product_update);

    $("#quantity, #amount, #discount, #tax_percentage").change(product_update);
                            
    //function() {
    //  product_update();
    //  var amount = $("#amount").val();
    //  var quantity = $("#quantity").val();
    //  var total = amount * quantity;
    //  $("#total").val(total);    
    //});
    
    $(".cart").click(function() {
      var title = "foo";
      var id = $(this).attr("id");
      var idAr = id.split(";");
      var res_id = idAr[0].substr(4);
      var inv_id = idAr[1].substr(4);
      //alert("res_id=" + res_id + " inv_id=" + inv_id);
      if(res_id && inv_id) {
        ajax_url = "/ajax.php?oper=view&obj_type=reservation&id=" + res_id;
        //alert(ajax_url);
        jQuery.ajax({
          type: "POST",
          url: ajax_url,
          data: "nh=1",
          cache: false,
          async: false,
          success: function(responseText) {                         
            //alert(dump(responseText));
            var response = jQuery.parseJSON(responseText);
            //alert(dump(response));
            var reservation = jQuery.parseJSON(response.data);
            var title = "" + reservation.first_name + " " + reservation.last_name;
            $("#product_dialog").show();
            $("#res_id").val(res_id);
            $("#inv_id").val(inv_id);
            $("#product_dialog").show();  
            //alert("ok len=" + $("#product_dialog").length);
            $("#product_dialog").dialog({
              title: title, 
              modal: true, 
              position: "center",
              width: 400,
              height: 600,
              close: function(event, ui) {reload_page();}
            });
            reset_product_form();
          },
        });
      }

      return false;
    });
  ';

  return html_div($output, 'visuallyhidden', "id='product_dialog'");
}


// end product.inc

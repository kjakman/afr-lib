<?php

// begin class definitions  
$__CLASSES = array(                                

//   CLASS                          USER_LEVEL  GROUP TABLE KEY FLAGS DESCRIPTOR SORT  PARENT_CLASS PARENT_FIELD
//                                    0           1     2    3     4      5       6        7            8

// security - used by php auth
    'phpauth_config' =>                array(USER_LEVEL_SYSTEM, 'external', 'config', 'id', 0, 'setting', '', ''),
    'phpauth_attempts' =>              array(USER_LEVEL_SYSTEM, 'external', 'attempts', 'id', 0, 'ip', '', ''),
    'phpauth_requests' =>              array(USER_LEVEL_SYSTEM, 'external', 'requests', 'id', 0, 'rkey', '', ''),
    'phpauth_sessions' =>              array(USER_LEVEL_SYSTEM, 'external', 'sessions', 'id', 0, 'ip', '', ''),

// object model
    'class' =>                array(USER_LEVEL_SYSTEM, 'object_model', '', 'type', NF_CACHE, 'name', '', ''),
    'class_group' =>          array(USER_LEVEL_SYSTEM, 'object_model', '', 'name', NF_CACHE, 'name', '', ''),
    'attribute' =>            array(USER_LEVEL_SYSTEM, 'object_model', '', 'id', NF_CACHE,   'name', 'id', ''),
    'attribute_type' =>       array(USER_LEVEL_SYSTEM, 'object_model', '', 'name', NF_CACHE_ASSOC,   'name', '', ''),
    'object_description' =>   array(USER_LEVEL_STAFF, 'object_model', '', '', 0, '', '', ''),
    //'default_class' =>        array(USER_LEVEL_SYSTEM, 'object_model', '', '', 0, '', '', ''),
    'default_attributes' =>   array(USER_LEVEL_SYSTEM, 'object_model', '', '', 0, '', '', ''),

// system
    'dictionary' =>           array(USER_LEVEL_SYSTEM, 'system', '', '', 0, 'term language', '', ''), /** special caching */
    'security' =>             array(USER_LEVEL_SYSTEM, 'system', '', '', 0, '', '', ''),
    'changelog' =>            array(USER_LEVEL_SYSTEM, 'system', '', '', NF_NO_ACTIONS, '', '', ''),
    'mail_campaign' =>        array(USER_LEVEL_SYSTEM, 'system', '', '', NF_NO_ACTIONS, '', '', ''),      
    'mail_queue' =>           array(USER_LEVEL_SYSTEM, 'system', '', '', NF_NO_ACTIONS, '', '', ''),      
    'mail_log' =>             array(USER_LEVEL_SYSTEM, 'system', '', '', NF_NO_ACTIONS, '', '', ''),      
    'sms_queue' =>            array(USER_LEVEL_SYSTEM, 'system', '', '', NF_NO_ACTIONS, '', '', ''),
    'exchange_rate' =>        array(USER_LEVEL_SYSTEM, 'system', '', '', 0, '', '', ''),
    'language' =>             array(USER_LEVEL_SYSTEM, 'system', '', 'code', NF_CACHE_ASSOC, '', '', ''),
    'currency' =>             array(USER_LEVEL_SYSTEM, 'system', '', 'code', NF_CACHE_ASSOC, 'code', 'code', ''),
    'countries' =>            array(USER_LEVEL_SYSTEM, 'system', '', '', NF_CACHE, '', 'name', ''),
    'weekday' =>              array(USER_LEVEL_SYSTEM, 'system', '', '', NF_CACHE_ASSOC, '', 'name', ''),
    'gender' =>               array(USER_LEVEL_SYSTEM, 'system', '', '', NF_CACHE_ASSOC, '', 'name', ''),
    'access_token' =>         array(USER_LEVEL_SYSTEM, 'system', '', '', 0, '', '', ''),
    'mime_type' =>            array(USER_LEVEL_SYSTEM, 'system', '', 'extension', 0, '', 'mime', ''),
    'description_type' =>     array(USER_LEVEL_SYSTEM, 'system', '', '', NF_CACHE_ASSOC, '', '', ''),
    'address' =>              array(USER_LEVEL_SYSTEM, 'system', '', '', NF_NO_DELETE|NF_NO_LIST, 'first_name,last_name,address1,city', '', ''),
    'address_type' =>         array(USER_LEVEL_SYSTEM, 'system', '', '', NF_CACHE_ASSOC, '', '', ''),
    'phone_type' =>           array(USER_LEVEL_SYSTEM, 'system', '', '', NF_CACHE_ASSOC, '', '', ''),
    'source' =>               array(USER_LEVEL_SYSTEM, 'system', '', '', 0, '', '', ''),
    'client' =>               array(USER_LEVEL_SYSTEM, 'system', '', '', 0, '', '', ''),
    'legal_entity' =>         array(USER_LEVEL_SYSTEM, 'system', '', '', 0, '', '', ''),
    'media' =>                array(USER_LEVEL_USER,   'system', '', '', NF_NO_LIST, '', '', ''),
    'db_sequence' =>          array(USER_LEVEL_SYSTEM, 'system', '', 'seq_name', 0, 'seq_name', 'seq_name', ''),
    'profile_media_map' =>    array(USER_LEVEL_USER,   'system', '', '', NF_NO_LIST, '', '', ''),

// site
    /** get rid of the next 2 - use config file */
    'site' =>                 array(USER_LEVEL_ADMIN, 'site', '', '', NF_CACHE_OFF|NF_COMPACT_LIST, 'name', '', ''),
    'site_content' =>         array(USER_LEVEL_STAFF,  'site', '', '', 0, 'title', '', ''),
    
    
    /** get rid of the next 4 - CMS no longer in use */
    'site_page' =>            array(USER_LEVEL_STAFF,  'site', '', '', 0, '', 'parent_id, rank, id', ''),
    'page_content' =>         array(USER_LEVEL_STAFF,  'site', '', '', NF_NO_ADD, 'title', '', ''),
    'page_element' =>         array(USER_LEVEL_STAFF,  'site', '', '', 0, '', 'rank', ''),
    'page_type' =>            array(USER_LEVEL_SYSTEM, 'site', '', '', NF_CACHE, '', 'id', ''),

    'message' =>              array(USER_LEVEL_STAFF,  'site', '', '', 0, '', '', ''),
    'google_application' =>   array(USER_LEVEL_USER,   'site', '', '', 0, '', '', ''),
    'font' =>                 array(USER_LEVEL_SYSTEM, 'site', '', '', 0, '', '', ''),

    'sharing' =>              array(USER_LEVEL_STAFF,  'social', '', '', 0, 'title', '', ''),
    'share_log' =>            array(USER_LEVEL_STAFF,  'social', '', '', 0, 'title', '', ''),

    'news' =>                 array(USER_LEVEL_STAFF,  'content', '', '', 0, 'title', '', ''),
    'article' =>              array(USER_LEVEL_USER,   'content', '', '', 0, 'title', 'created desc', ''),
    'comment' =>              array(USER_LEVEL_USER,   'content', '', '', 0, 'title', 'created desc', ''),
    'newsletter' =>           array(USER_LEVEL_USER,   'content', '', '', 0, 'email_address', 'created desc', ''),
    'article_type' =>         array(USER_LEVEL_USER,   'content', '', '', NF_CACHE_ASSOC, '', '', ''),
    'minisite' =>             array(USER_LEVEL_ADMIN, 'site', '', '', NF_CACHE|NF_COMPACT_LIST, 'name', '', ''),
    'minisite_page_type' =>   array(USER_LEVEL_ADMIN, 'site', '', '', NF_CACHE|NF_COMPACT_LIST, 'name', '', ''),
    'minisite_page' =>        array(USER_LEVEL_ADMIN, 'site', '', '', NF_CACHE|NF_COMPACT_LIST, 'page_name', '', ''),
                                                                             

// user                            

    'user' =>                 array(USER_LEVEL_SYSTEM, 'user', 'user', '', NF_NO_DELETE, 'username', '', ''),
    'user_level' =>           array(USER_LEVEL_SYSTEM, 'user', '', 'user_level', NF_CACHE_ASSOC, 'user_type', '', ''),
    'user_role' =>            array(USER_LEVEL_SYSTEM, 'user', '', '', NF_CACHE_ASSOC|NF_NO_ACTIONS, '', '', ''),
    'user_map' =>             array(USER_LEVEL_SYSTEM, 'user', '', '', 0, '', '', ''),
    'auth_log' =>             array(USER_LEVEL_SYSTEM, 'user', '', '', NF_NO_DELETE, 'display_name', '', ''),
    'user_profile' =>         array(USER_LEVEL_STAFF,  'user', '', '', 0, 'display_name', '', ''),
    'opening_hours' =>        array(USER_LEVEL_STAFF,  'user', '', '', 0, '', '', ''),
    'provider_profile' =>     array(USER_LEVEL_STAFF,  'user', '', '', 0, 'display_name', '', ''),
    // 'password_reset' =>       array(USER_LEVEL_SYSTEM, 'user', '', '', NF_NO_ACTIONS, '', '', ''),
    'wishlist' =>             array(USER_LEVEL_SYSTEM, 'user', '', '', 0, '', '', ''),

    
    'org' =>                  array(USER_LEVEL_STAFF,  'user', '', '', 0, 'name', 'name', ''),
    'contact' =>              array(USER_LEVEL_SYSTEM, 'user', '', '', 0, 'first_name,last_name,organization', '', ''),

    'contact_status' =>       array(USER_LEVEL_SYSTEM, 'user', '', '', NF_CACHE_ASSOC, '', '', ''),

    'provider' =>             array(USER_LEVEL_SYSTEM, 'user', '', '', 0, '', '', ''),
    'provider_key' =>         array(USER_LEVEL_SYSTEM, 'user', '', '', 0, '', '', ''),
    
// res
    'reservation' =>          array(USER_LEVEL_ADMIN, 'enquiry', '', 'res_id', NF_LOG|NF_COMPACT_LIST, '', '', ''),
    'special_offer' =>        array(USER_LEVEL_ADMIN,  'enquiry', '', '', NF_LOG|NF_COMPACT_LIST, '', '', ''),
    'res_status' =>           array(USER_LEVEL_SYSTEM, 'enquiry', '', '', NF_CACHE_ASSOC, '', '', ''),
    'res_type' =>             array(USER_LEVEL_SYSTEM, 'enquiry', '', '', NF_CACHE_ASSOC, '', '', ''),
    'rate_class' =>           array(USER_LEVEL_SYSTEM, 'enquiry', '', '', NF_CACHE_ASSOC, '', '', ''),
    'enq_status' =>           array(USER_LEVEL_SYSTEM, 'enquiry', '', '', 0, '', '', ''),
    'res_exception' =>        array(USER_LEVEL_SYSTEM, 'enquiry', '', '', 0, '', 'rank', ''),

    'price_breakdown'      => array(USER_LEVEL_USER,   'enquiry', '', '', 0, '', '', ''),
    'price_breakdown_row'  => array(USER_LEVEL_USER,   'enquiry', '', '', 0, '', 'date', ''),
    'calendar_import' =>      array(USER_LEVEL_SYSTEM, 'system', '', '', 0, '', '', ''),

// property
    'agent' =>                array(USER_LEVEL_ADMIN,  'user', '', '', 0, 'name', 'name', ''),

// review
    'review' =>              array(USER_LEVEL_ADMIN,  'review', '', '', 0, '', 'id', ''),
    'review_item' =>         array(USER_LEVEL_ADMIN,  'review', '', '', 0, '', 'id', ''),
    'review_score' =>        array(USER_LEVEL_ADMIN,  'review', '', '', 0, '', 'id', ''),
    'chat' =>                 array(USER_LEVEL_SYSTEM, 'enquiry', '', '', NF_NO_ACTIONS, '', 'created DESC', ''),
    'feedback' =>             array(USER_LEVEL_SYSTEM, 'enquiry', '', '', 0, '', '', ''),
    'feedback_purpose' =>     array(USER_LEVEL_SYSTEM, 'enquiry', '', '', 0, 'name', 'id', ''),
    'feedback_profile' =>     array(USER_LEVEL_SYSTEM, 'enquiry', '', '', 0, 'name', 'id', ''),
                                    
// event
    'event' =>                array(USER_LEVEL_OWNER,  'event', '',      '', NF_LOG, '', '', ''),
    'event_type' =>           array(USER_LEVEL_SYSTEM, 'event', '', '', NF_CACHE_ASSOC, '', '', ''),
    'event_status' =>         array(USER_LEVEL_SYSTEM, 'event', '', '', NF_CACHE_ASSOC, '', '', ''),
    
// product            
    'product' =>              array(USER_LEVEL_STAFF,  'product', '', '', 0, 'name', 'name', ''),
    'product_category' =>     array(USER_LEVEL_SYSTEM, 'product', '', '', 0, '', '', ''),    
    'product_option' =>       array(USER_LEVEL_STAFF,  'product', '', '', 0, '', '', ''),    
    'product_variant' =>      array(USER_LEVEL_STAFF,  'product', '', '', 0, '', '', ''),    
    'product_unit' =>         array(USER_LEVEL_SYSTEM, 'product', '', 'code', 0, '', 'code', ''),    
    'supplier' =>             array(USER_LEVEL_STAFF,  'product', '', '', 0, 'address.organization', 'address.organization', ''),
    'product_supplier_map' => array(USER_LEVEL_STAFF,  'product', '', '', 0, 'product_id', 'product_id', ''),
//    'cart'                 => array(USER_LEVEL_USER,   'product', '', '', 0, '', '', ''),
//    'cart_row'             => array(USER_LEVEL_USER,   'product', '', '', 0, '', '', ''),
    
// accounting
    'invoice' =>              array(USER_LEVEL_OWNER,  'accounting', '', '', NF_LOG|NF_COMPACT_LIST, '', '', ''),
    'inv_status' =>           array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
    'inv_type' =>             array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
    'inv_lineitem' =>         array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_LOG|NF_NO_LIST, '', 'type', ''),
    'lineitem_type' =>        array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_CACHE_ASSOC, 'name', '', ''),
    'payment' =>              array(USER_LEVEL_ADMIN,  'accounting', '', '', NF_LOG|NF_NO_ADD|NF_NO_DELETE, '', '', ''),
    'payment_type' =>         array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
    'payment_source' =>       array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
    'payment_status' =>       array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
    'withdrawal' =>           array(USER_LEVEL_SYSTEM,    'accounting', '', '', NF_LOG|NF_NO_ADD|NF_NO_EDIT|NF_NO_DELETE, '', '', ''),
    'charge' =>               array(USER_LEVEL_SYSTEM,    'accounting', '', '', NF_LOG|NF_NO_ADD|NF_NO_EDIT|NF_NO_DELETE, '', '', ''),
    'gateway' =>              array(USER_LEVEL_SYSTEM,    'accounting', '', '', 0, 'name', '', ''),
    'transaction' =>          array(USER_LEVEL_SYSTEM,    'accounting', '', 'tx_id', NF_LOG|NF_NO_ACTIONS, '', '', ''),
    'tx_status' =>            array(USER_LEVEL_SYSTEM,    'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
    'cc' =>                   array(USER_LEVEL_SYSTEM,    'accounting', '', '', NF_NO_LIST, '', '', ''),
    'cc_type' =>              array(USER_LEVEL_SYSTEM,    'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
    'tax_category' =>         array(USER_LEVEL_SYSTEM, 'accounting', '', 'code', NF_CACHE_ASSOC, '', '', ''),
    'billing_period' =>       array(USER_LEVEL_SYSTEM, 'accounting', '', '', NF_CACHE_ASSOC, '', '', ''),
                                                                              

// location
    'country' =>              array(USER_LEVEL_SYSTEM, 'location', 'msCountry', '', NF_CACHE, '', '', ''),
    'geo_city' =>             array(USER_LEVEL_SYSTEM, 'location', 'geo_city', '', 0, 'name', 'name', ''),

    'landmark' =>             array(USER_LEVEL_STAFF,  'location', 'landmark'        , 'id', 0, '', '', ''),
    'landmark_type' =>        array(USER_LEVEL_STAFF,  'location', 'landmark_type'   , 'id', NF_CACHE_ASSOC, '', '', ''),
    'location' =>             array(USER_LEVEL_SYSTEM, 'location', 'location'        , 'id', 0, '', '', ''),
    'airport' =>              array(USER_LEVEL_SYSTEM, 'location', 'airport'         , 'id', 0, '', '', ''),

    'landmark_type' =>        array(USER_LEVEL_STAFF,  'location', 'landmark'        , 'id', NF_CACHE_ASSOC, '', '', ''),


);
// end class definitions

// begin class attributes

$__CLASS_ATTRIBUTES = array(                                

//   CLASS ATTRIBUTES                       0                  1       2       3            4          5
//   db field name                       Screen name          type   default flags  relation/options filters

  
  "cc" => 
  
  array(
     'id'=>                        array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT),
     'guest_id'=>                  array('Guest Id',          'link', ATT_DEFAULT_NULL, AF_HIDE_ADD|AF_READONLY|AF_SEARCHABLE, 'user'),
     'user_id'=>                   array('User Id',           'link', ATT_DEFAULT_USER_ID, AF_HIDE_ADD|AF_READONLY|AF_SEARCHABLE, 'user'),
     'site_id'=>                   array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_HIDE_LIST|AF_LOG, 'site'),
     'cc_type'=>                   array('Credit Card Type',  'many2one', 0, 1, 'cc_type'),
     'cc_number'=>                 array('Credit Card Number', 'cc',     0, AF_OBLIGATORY|AF_READONLY_EDIT, 20),
     'cc_crypt'=>                  array('CC crypt',           'cc',  0, AF_READONLY_EDIT|AF_ENCRYPT, 40),
     'cc_cardholder'=>             array('Cardholder full name','str',    '', 1, 50),
  //         'cc_exp_month'=>              array('Exp. Month',         'intSelect', 0, AF_OBLIGATORY, array(1,12,1)),
  //         'cc_exp_year'=>               array('Exp. Year',          'intSelect', 0, AF_OBLIGATORY, array(2009,2020,1)),
  
     'cc_exp_month'=>              array('Exp. Month',        'str',    '', 1, 2),
     'cc_exp_year'=>               array('Exp. Year',         'str',    '', 1, 4),
     'cc_cvv2'=>                   array('Security Code',     'str',    '', 0, 4),
     'last_charge' =>              array('Last charged',      'datetime',  ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY),
  
     'charge'=>                    array('Charge log',        'one2many', 0, AF_HIDE_LIST|AF_READONLY, 'charge', 'cc_id'),
     'payments'=>                  array('Payments',          'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'payment', 'cc_id'),
     'remember'=>                  array('Store card?',        'bool', 0, AF_READONLY),
  
     'default_card' =>             array('Primary card?',      'bool',     0, AF_HIDE_LIST|AF_READONLY),
     
  ),
  
  /** new feedback */
  "review" => 
  
  array(

    
     'id'=>                        array('ID',                'id',          0, AF_HIDE_LIST|AF_PRIMARY|AF_AUTOINCREMENT),
     'created'=>                   array('Review Date',       'datetime',    ATT_DEFAULT_NOW, 0, 20),
     'active' =>                   array('Active?',           'bool',        1, AF_HIDE_LIST|AF_READONLY),
     'client_id'=>                 array('Client',    			  'link',        ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>                   array('Site',              'link',        ATT_DEFAULT_SITE_ID, AF_OBLIGATORY|AF_HIDE_LIST, 'site'),
     'user_id'=>                   array('User',              'link',        ATT_DEFAULT_USER_ID, AF_HIDE_ADD, 'user'),
     'host_id'=>                   array('Host',              'link',        0, AF_HIDE_ADD|AF_READONLY, 'user'),
     'guest_id'=>                  array('Guest',             'link',        0, AF_HIDE_ADD|AF_READONLY, 'user'),

     'source_id' =>                array('Source',             'link',        0, AF_HIDE_ADD|AF_READONLY, 'source'),
     'fid' =>                      array('Foreign ID',         'str',        0, AF_HIDE_ADD|AF_READONLY, 20),
     'source_url' =>               array('Source URL',         'url',         0, AF_HIDE_ADD|AF_READONLY),
     
     'parent_type'=>               array('Parent Type',       'parent_type', 0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD),
     'parent_id'=>                 array('Parent Id',         'parent_id',   0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, 11),

     'reviewed_obj_type'=>         array('Reviewed Type',     'str',         0, AF_READONLY|AF_HIDE_ADD, 128),
     'reviewed_obj_id'=>           array('Reviewed Id',       'int',         0, AF_READONLY|AF_HIDE_ADD, 11),

     'language'=>                  array('Language',          'str',        '', AF_HIDE_LIST, 2),
     'published' =>                array('Published?',        'bool',        0, AF_HIDE_LIST|AF_READONLY),
     'anonymous' =>                array('Anonymous?',        'bool',        0, AF_HIDE_LIST|AF_READONLY),
     'helpful'=>                   array('# Helpful',         'int',         0, AF_READONLY, 4),
     'recommend'=>                 array('Recommend?',        'yesno',       0, AF_READONLY, 1),

     'reviewer_name' =>            array('Name',              'str',        '', 0, 128),
     'reviewer_city' =>            array('City',              'str',        '', 0, 128),
     'reviewer_state' =>           array('State',              'str',        '', 0, 32),
     'reviewer_country' =>         array('Country',           'str',        '', 0, 2),
     'reviewer_email' =>           array('Email',             'email',        '', 0, 128),                          
     'reviewer_phone' =>           array('Phone',             'phone',        '', 0, 16),
     
     'review_title' =>             array('Title',              'str',        '', 0, 128),
     'review_comment' =>           array('Comment',           'text',       '', AF_HIDE_LIST),
     'review_private' =>           array('Private feedback',   'text',       '', AF_HIDE_LIST),

     'media' =>                    array('Media',             'media',      '', 0, $single_media_options),
     'score' =>                    array('Score',             'float',        0, AF_HIDE_ADD, '2.2'),
  ),
  

  /** a review category for a given parent_type and a given site */
  "review_item" =>
  array(
     'id'=>            	    array('ID',                'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',              'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 128),
     'site_id'=>            array('Site',              'link',        ATT_DEFAULT_SITE_ID, AF_OBLIGATORY|AF_HIDE_LIST|AF_LOG, 'site'),
     'parent_type'=>        array('Parent Type',       'parent_type', 0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, $reqTypeArray),
     'rank' =>              array('Rank',              'int', 0, 0, 6),
  ),
  
  /** the review of one category from one user 0-10 */
  "review_score" =>
  array(
     'id'=>            	    array('ID',                'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     
     /** below four duplicated for DB usability */
     'client_id'=>          array('Client',    			   'link',        ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>            array('Site',              'link',        ATT_DEFAULT_SITE_ID, AF_OBLIGATORY|AF_HIDE_LIST, 'site'),                            
     'parent_type'=>        array('Parent Type',       'parent_type', 0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, $reqTypeArray),
     'parent_id'=>          array('Parent Id',         'parent_id',   0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, 11),

     'review_id'=>          array('Review ID',         'link',  0, AF_OBLIGATORY|AF_HIDE_LIST|AF_LOG, 'review'),
     'review_item_id'=>     array('Review Item ID' ,   'link',  0, AF_OBLIGATORY|AF_HIDE_LIST|AF_LOG, 'review_item'),
     'score' =>             array('Score',             'int',   0, AF_HIDE_ADD),
  ),

  /** legacy - used to print old quotes on home page */
  
  "feedback_purpose" =>
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),
  "feedback_profile" =>
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),

  "feedback" =>                                                                                       
  
  array(

    
     'id'=>                        array('ID',                'id',          0, AF_HIDE_LIST|AF_PRIMARY|AF_AUTOINCREMENT),
     'parent_id'=>                 array('Parent Id',         'parent_id',   0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, 11),
     'parent_type'=>               array('Parent Type',       'parent_type', 0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, $reqTypeArray),
     'apt_id' =>                   array('Property',          'link',        0, AF_HIDE_EDIT|AF_HIDE_ADD, 'property'),
     'city_id' =>                  array('City',              'link',        0, AF_HIDE_EDIT|AF_HIDE_ADD, 'geo_city'),
     'site_id'=>                   array('Site',              'many2one',   ATT_DEFAULT_SITE_ID, AF_OBLIGATORY|AF_HIDE_LIST|AF_LOG, 'site'),
     'page_id' =>                  array('Page',              'link',        0, AF_HIDE_EDIT|AF_HIDE_ADD, 'site_page'),
     'product_id' =>               array('Product',           'link',        0, AF_HIDE_EDIT|AF_HIDE_ADD, 'product'),
     'property_type'=>             array('Property Type',     'valSelect',   0, AF_READONLY, $propertyTypeArray),

     'source_id'=>                 array('Source',            'link',        ATT_DEFAULT_NULL, 0, 'source'),
     'source_url'=>                array('Source URL',        'url',         0, 0, 255),
     'fid'=>                       array('Foreign ID',        'str',         0, 0, 10),
     
     'user_id'=>                   array('User',              'link',        ATT_DEFAULT_USER_ID, AF_HIDE_ADD, 'user'),
//     'owner_id'=>                  array('Owner',             'link',        ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'owner'),
     'host_id'=>                   array('Host',              'link', 0,     AF_HIDE_ADD|AF_READONLY, 'user'),
     'guest_id'=>                  array('Guest',             'link', 0,     AF_HIDE_ADD|AF_READONLY, 'user'),
     'agent_id'=>                  array('Agent',             'link',        ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'agent'),
     'client_id'=>                 array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
  
     'created'=>                   array('Review Date',       'datetime',    ATT_DEFAULT_NOW, 0, 20),
     'language'=>                  array('Language',          'str',    '', AF_HIDE_LIST, 2),
  
     'checkin'=>                   array('Checkin',           'date',   '', AF_HIDE_LIST, 50),
     'checkout'=>                  array('Checkout',          'date',   '', AF_HIDE_LIST, 50),
     'currency'=>                  array('Currency',          'valSelect', 0, AF_HIDE_LIST, $currencyArray),
     'total'=>                     array('Total',             'money',   0, AF_HIDE_LIST|AF_READONLY),
     'grand_total'=>               array('Grand Total',       'money',   0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL),
     'num_days'=>                  array('# Days',            'int',    0, AF_READONLY, 4),
  //         'num_hours'=>                 array('# Hours',           'float',    0, AF_HIDE_LIST|AF_READONLY, '2.2'),
     'num_guests'=>                array('# Guests',          'intSelect', 0, 0, array(1,8,1, '')),
  
     'profile'=>                    array('Profile',          'many2one', 0, 0, 'feedback_profile'),
     'purpose'=>                    array('Purpose',          'many2one', 0, 0, 'feedback_purpose'),
     'ref'=>                        array('Referer',          'many2one', 0, AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'referer'),
     'ref_other'=>                  array('Ref (other)',      'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 20),
     'ref_searchterm'=>             array('Searchterm',       'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 50),
  
     'salutation'=>        array('Salutation',        'valSelect', '', 0, $salutationArray),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 50),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 50),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 50),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 50),
     'country'=>           array('Country',           'valSelect', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE, $countryArray),
  
  //         'feedback_website'    =>       array('Website'    , 'valSelect', 0, AF_READONLY, $ratingArray),
     'feedback_booking'    =>       array('Booking'    , 'int', 0, AF_READONLY),
     'feedback_pickup'     =>       array('Pickup'     , 'int', 0, AF_READONLY),
     'feedback_checkin'    =>       array('Checkin'    , 'int', 0, AF_READONLY),
     'feedback_catering'    =>      array('Catering'   , 'int', 0, AF_READONLY),
     'feedback_service'    =>       array('Service'    , 'int', 0, AF_READONLY),
     'feedback_location'   =>       array('Location'   , 'int', 0, AF_READONLY),
     'feedback_size'       =>       array('Size'       , 'int', 0, AF_READONLY),
     'feedback_decoration' =>       array('Decoration' , 'int', 0, AF_READONLY),
     'feedback_comfort'    =>       array('Comfort'    , 'int', 0, AF_READONLY),
     'feedback_cleaning'   =>       array('Cleaning'   , 'int', 0, AF_READONLY),
     'feedback_apps'       =>       array('Applicances', 'int', 0, AF_READONLY),
     'feedback_value'      =>       array('Value'      , 'int', 0, AF_READONLY),
     'feedback_overall'    =>       array('Score (0-10)'    , 'float', 0, 0, "2.1"),
   // 'feedback_overall'=>                array('Score (0-10)',          'intSelect', 0, AF_LOG, array(1,10,0.5, '')),

     'feedback_recommend'  =>       array('Recommend'  , 'bool', 0, AF_READONLY),
  
  //         'can_publish' =>               array('Can publish?', 'bool',     0, AF_HIDE_LIST|AF_READONLY),
     'published' =>                 array('Published?',   'bool',   0, AF_HIDE_LIST|AF_READONLY),
     'anonymous' =>                 array('Anonymous?',   'bool',   0, AF_HIDE_LIST|AF_READONLY),
     'helpful'=>                    array('# Helpful',    'int',    0, AF_READONLY, 4),

     'feedback_title' =>            array('Title',        'str',  '', 0, 128),
     'feedback_comments' =>         array('Comments',        'text',  '', AF_HIDE_LIST),
     'feedback_comments_neg' =>     array('Suggestions for improvement', 'text',  '', AF_HIDE_LIST),
     'feedback_private' =>          array('Private feedback', 'text',  '', AF_HIDE_LIST),

     'media' =>            array('Media',               'media', '', AF_HIDE_ADD, $single_media_options),
     
  ),
  
  
  "chat" => 
  
  array(
     'id'=>                        array('ID',                'id',   0, AF_HIDE_LIST|AF_PRIMARY|AF_AUTOINCREMENT),
     'parent_id'=>                 array('Parent Id',         'int',   0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, 11),
     'parent_type'=>               array('Parent Type',       'str', 0, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD),
     'created'=>                   array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY, 20),
     'timestamp'=>                 array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY, 20),
     'read_on'=>                   array('Read',              'datetime', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD, 20),

     'status_change' =>            array('Status Change',     'link', 0, AF_HIDE_EDIT|AF_HIDE_ADD, 'res_status'),
     'special_offer_id' =>         array('Special Offer',     'link', 0, AF_HIDE_EDIT|AF_HIDE_ADD, 'special_offer'),

     'user_id' =>                  array('Author',            'link', ATT_DEFAULT_USER_ID, AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),
     'sender_id'=>                 array('Sender',            'link', 0, AF_SEARCHABLE|AF_READONLY, 'user'),
     'sender_level'=>              array('Sender Level',      'many2one', 0, AF_SEARCHABLE|AF_READONLY, 'user_level'),
     'receiver_id'=>               array('Recipient',         'link', 0, AF_SEARCHABLE|AF_READONLY, 'user'),
     'receiver_level'=>            array('Recipient Level',   'many2one', 0, AF_SEARCHABLE|AF_READONLY, 'user_level'),
     'thread' =>                   array('Thread',            'str', '', 0, 32),
     'notify'=>                    array('Notify',            'bool', 1, AF_HIDE_EDIT|AF_HIDE_ADD),
     'message'=>                   array('Message',           'text', '', AF_HIDE_EDIT|AF_OBLIGATORY|AF_HIDE_ADD),
     'language'=>                  array('Language',          'many2one',  ATT_DEFAULT_LANG, AF_HIDE_LIST, 'language'),
     
  ),
  
  "special_offer" => 
  
  array(
     'id'=>                        array('ID',                'id',   0, AF_HIDE_LIST|AF_PRIMARY|AF_AUTOINCREMENT),
     'created'=>                   array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY, 20),
     'timestamp'=>                 array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY, 20),
     'active' =>                   array('Active?',           'bool',     1, 0),
     
     'site_id'=>                   array('Site',    			    'link',    ATT_DEFAULT_SITE_ID, AF_READONLY|AF_HIDE, 'site'),
     'res_id' =>                   array('Reservation',       'link',     0, AF_OBLIGATORY|AF_HIDE_EDIT|AF_HIDE_ADD, 'reservation'),
     'user_id' =>                  array('Author',            'link', ATT_DEFAULT_USER_ID, AF_OBLIGATORY|AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),
     'host_id'=>                   array('Host',              'link',     0, AF_OBLIGATORY|AF_HIDE_ADD|AF_READONLY, 'user'),
     'guest_id'=>                  array('Guest',             'link',     0, AF_OBLIGATORY|AF_HIDE_ADD|AF_READONLY, 'user'),

     'property_type'=>             array('Property Type',     'valSelect', 0, AF_READONLY|AF_SEARCHABLE|AF_HIDE_ADD|AF_OBLIGATORY, $propertyTypeArray),
     'apt_id'=>                    array('Property ID',       'link', 0, AF_OBLIGATORY|AF_HIDE_ADD|AF_READONLY, 'user'),
     'num_guests'=>                array('# Guests',          'int', 0, AF_OBLIGATORY|AF_LOG),
     'num_days'=>                  array('# Days',            'int', 0, AF_OBLIGATORY),
     'checkin'=>                   array('Checkin',           'date',   '', AF_LOG),
     'checkout'=>                  array('Checkout',          'date',   '', AF_LOG),
                                   
     'currency'=>                  array('Currency',          'valSelect', 0, AF_OBLIGATORY|AF_HIDE_LIST|AF_LOG, $currencyArray),
     'rate_type'=>                 array('Rate Type',         'valSelect', 'day', AF_OBLIGATORY|AF_HIDE_LIST|AF_LOG, $rateTypeArray),
     'rate_base'=>                 array('Base Rate',         'money',   0, AF_OBLIGATORY|AF_HIDE_LIST|AF_LOG),
     
  ),
  

  "security" => 
  array(
     'id'=>                    array('ID',                'id',   0, AF_HIDE|AF_PRIMARY|AF_AUTOINCREMENT),
     'node'=>                  array('Node',          		'valSelect', 0, AF_SEARCHABLE|AF_OBLIGATORY, $nodeTypeArray),
     'action'=>                array('Action',          	'valSelect', 0, AF_SEARCHABLE|AF_OBLIGATORY, $actionArray),
     'user_level'=>            array('User Level',        'many2one',  0, AF_SEARCHABLE|AF_OBLIGATORY, 'user_level'),
  ),
  
  "event" => 
  
  array(
    'id' =>                        array('ID',                'id',   0, AF_SEARCHABLE|AF_PRIMARY|AF_AUTOINCREMENT),
    
    'active' =>                    array('Active?',           'bool',     1, 0),
    'client_id'=>                  array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
    'site_id'=>                    array('Site',    			    'link',     ATT_DEFAULT_SITE_ID, AF_READONLY|AF_HIDE, 'site'),
    'user_id' =>                   array('Author',            'link',     ATT_DEFAULT_USER_ID, AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),

    'host_id'=>                    array('Host',              'link',     0, AF_HIDE_ADD|AF_READONLY, 'user'),
    'parent_id'=>                 array('Parent Id',          'parent_id',   ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST, 11),
    'parent_type'=>               array('Parent Type',        'parent_type', ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST),
    'user_level'=>                 array('User Level',        'many2one', 0, 0, "user_level"),
    
    'created' =>                   array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY),
    'timestamp' =>                 array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
    'type' =>                      array('Event type',        'many2one', 100, AF_OBLIGATORY|AF_SEARCHABLE, 'event_type'),
    'status' =>                    array('Status',            'many2one', EVENT_STATUS_UNCONFIRMED, AF_OBLIGATORY|AF_LOG|AF_SEARCHABLE , 'event_status'),
    'status_change_timestamp'=>    array('Status Changed on', 'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),

    'language'=>                   array('Language',          'many2one', ATT_DEFAULT_LANGUAGE, 0, 'language'),


    /** changed from name/notes */
    'title'=>                      array('Name',              'str',      '', AF_OBLIGATORY, 128),
    'description'=>                array('description',       'text',      '', AF_HIDE_LIST),

    'start_time' =>                array('Start',             'datetime', '', AF_OBLIGATORY|AF_SEARCHABLE|AF_LOG, 'end_time'), // options = dependent field
    'end_time' =>                  array('End',               'datetime', '', AF_OBLIGATORY|AF_SEARCHABLE|AF_LOG),
    'timezone' =>                  array('Timezone',          'timezone', ATT_DEFAULT_TIMEZONE, 0, 32),
    
    //'start_address' =>             array('Location',          'text',     '', AF_SEARCHABLE|AF_LOG, 200),
    //'end_address' =>               array('End Location',      'text',     '', AF_SEARCHABLE|AF_LOG, 200),
    'all_day' =>                   array('All day?',          'bool',     0, 0),

    'address_street'=>    array('Street',            'str',  '', AF_HIDE_LIST, 50 ),
    'address_number'=>    array('Number',            'str',  '', AF_HIDE_LIST, 12 ),
    'address_city'=>      array('City',              'str',  '', AF_HIDE_LIST, 50 ),
    'address_postcode'=>  array('Postcode',          'str',  '', AF_HIDE_LIST, 50 ),
    'address_state'=>     array('State',             'str',  '', AF_HIDE_LIST, 50 ),
    'address_country'=>   array('Country',           'link', '', AF_HIDE_LIST, 'countries'),
    'address_formatted'=> array('Adddress',          'text', '', AF_HIDE_LIST),

     'location_id' =>     array('Location',          'link',   0, AF_READONLY|AF_HIDE_ADD, 'location'),
    
    'location_lat'         => array('Latitude'         , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
    'location_long'        => array('Longitude'        , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
    'location_bounds' =>  array('Bounds',            'json', '', AF_HIDE_ADD|AF_HIDE_LIST),
    
    'num_hours'=>          array('# Hours',           'float',    0, AF_READONLY, '2.2'),
    'alert'=>              array('NB!',               'str',     '', AF_HIDE_LIST, 100),

    'booking_pin'=>                array('Booking PIN',       'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD|READONLY, 10),
    'media'                => array('Media',             'media',     '', 0, $event_media_options),

    /** not yet in use 
    'repeat_id' =>               array('Repeat ID',       'str',     '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 13),
    'repeat_interval' =>         array('Repeat Interval', 'str',     '', AF_HIDE_LIST, 4), // todo; add class, use many2one
    'repeat_until' =>            array('Repeat Until',    'datetime', '', AF_HIDE_LIST),
  
    'reminder_period1' =>         array('Reminder 1',       'str',      '', AF_HIDE_LIST, 6), // todo; add class, use many2one
    'reminder_time1' =>           array('Remdinder Time 1', 'datetime', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY),
    'reminder_sent1' =>           array('Remdinder Sent 1', 'bool',     0, AF_HIDE_LIST|AF_READONLY),
    'reminder_period2' =>         array('Reminder 2',       'str',      '', AF_HIDE_LIST, 6), // todo; add class, use many2one
    'reminder_time2' =>           array('Remdinder Time 2', 'datetime', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY),
    'reminder_sent2' =>           array('Remdinder Sent 2', 'bool',     0, AF_HIDE_LIST|AF_READONLY),
    'reminder_phone' =>          array('Reminder Phone',    'phone',    '', AF_HIDE_LIST, 20),
    'reminder_email' =>          array('Reminder email',    'email',    '', AF_HIDE_LIST, 20),        
    */
    
     
    ),
   
  "reservation" => 
  
  array(
     'res_id'=>                    array('ID',                'id', 0, AF_SEARCHABLE|AF_PRIMARY|AF_AUTOINCREMENT),
     'site_id'=>                   array('Site',              'many2one',   ATT_DEFAULT_SITE_ID, AF_LOG|AF_SEARCHABLE, 'site'),
     'status'=>                    array('Status',            'many2one',   STATUS_DRAFT, AF_OBLIGATORY|AF_LOG|AF_SEARCHABLE , 'res_status'),
     'type'=>                      array('Type',              'many2one',   5, AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD|AF_LOG, 'res_type'),
     'event_id'=>                  array('Event ID',          'link', 0, AF_HIDE_LIST, 'event'),
     'client_id'=>                 array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'source_id'=>                 array('Source',    			  'many2one', ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_LIST|AF_HIDE_ADD|AF_HIDE_EDIT, 'source'),
     'dest_id'=>                   array('Destination',    	  'many2one', ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_LIST|AF_HIDE_ADD|AF_HIDE_EDIT, 'source'),
     'fid'=>                       array('Foreign ID',        'str',  ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_LIST|AF_HIDE_ADD|AF_HIDE_EDIT, 20),
     'fids'                     => array('Foreign IDs'       ,'json', '', AF_HIDE_LIST|AF_HIDE_ADD, 255),
     'fgid'=>                      array('Foreign Guest ID',  'str',  ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_LIST|AF_HIDE_ADD|AF_HIDE_EDIT, 20),
     'property_type'=>             array('Property Type',     'valSelect', 0, AF_OBLIGATORY|AF_READONLY|AF_SEARCHABLE, $propertyTypeArray),
     'corporate'=>                 array('Corporate?',        'bool', 0, AF_READONLY),
     'created'=>                   array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY),
     'timestamp'=>                 array('Modified',          'timestamp',  ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY),
     'status_change_timestamp'=>   array('Status Changed on', 'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
     'booked_timestamp'=>          array('Booked on',         'datetime',  ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
     'cancel_timestamp'=>          array('Canceled on',       'datetime',  ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
     'expires'=>                   array('Expires',           'datetime',  ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_LIST, 6 ),
     'hidden'=>                    array('Hidden',            'bool',   0, AF_HIDE),
  
     'checkin'=>                   array('Checkin',           'datetime',   '', AF_OBLIGATORY|AF_LOG, 19),
     'checkout'=>                  array('Checkout',          'datetime',   '', AF_OBLIGATORY|AF_LOG, 19),
  
      // shift hours
     //'start_hour' =>                array('Start Time',        'str', '', AF_SEARCHABLE|AF_HIDE_LIST|AF_LOG, 5),
     //'end_hour' =>                  array('End Time',          'str', '', AF_SEARCHABLE|AF_HIDE_LIST|AF_LOG, 5),
  
     'num_months'=>                array('# Months',          'float',    0, AF_READONLY|AF_HIDE_LIST, '3.2'),
     'num_days'=>                  array('# Days',            'int',    0, AF_READONLY, 4),
     'num_hours'=>                 array('# Hours',           'float',    0, AF_HIDE_LIST|AF_READONLY, '2.2'),
     'num_guests'=>                array('# Guests',          'intSelect', 0, AF_LOG, array(1,8,1, '')),
      //'num_packages'=>              array('# Packages',         'float',    0, AF_HIDE_LIST|AF_READONLY, '4.2'),
                    
     //'package_unit'=>              array('Package Unit',    	 'many2one', 'EA', AF_HIDE_LIST|AF_READONLY, 'product_unit'),
     //'package_id' =>               array('Package',            'many2one', 0, AF_READONLY|AF_LOG|AF_SEARCHABLE, 'product', array('active' => 1, 'category_id' => PRODUCT_CATEGORY_PACKAGE), 'name'),
     //'catering_id' =>              array('Catering',           'many2one', 0, AF_READONLY|AF_LOG|AF_SEARCHABLE, 'product', array('active' => 1, 'category_id' => PRODUCT_CATEGORY_CATERING), 'name'),
  
     'user_id'=>                   array('Author',            'link',   ATT_DEFAULT_USER_ID, AF_HIDE_LIST|AF_SEARCHABLE|AF_READONLY, 'user'),
     'city_id'=>                   array('City',              'link',   0, AF_READONLY|AF_SEARCHABLE, 'geo_city'),
     'apt_id'=>                    array('Apartment',         'link',   0, AF_SEARCHABLE|AF_LOG, 'property'),
     'apartment_name'=>            array('Apartment Name',    'str',        '', AF_HIDE, 50),
     'city_name'=>                 array('City Name',         'str',        '', 0, 128),

     'guest_id'=>                  array('Guest ID',          'link',   0, AF_HIDE_ADD|AF_READONLY|AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),
     'host_id'=>                   array('Host',              'link',   0, AF_HIDE_ADD|AF_READONLY, 'user'),

      // 'traveler_id'=>               array('Old Guest ID',      'link',   ATT_DEFAULT_GUEST_ID, AF_HIDE_ADD|AF_READONLY|AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'guest'),
     'org_id'=>                    array('Org',               'link',   ATT_DEFAULT_ORG_ID, AF_SEARCHABLE, 'org', '', 'org.name'),
     'contact_id'=>                array('Contact',    			  'link',   ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY_EDIT, 'contact'),
     'bill_to'=>                   array('Bill To',           'valSelect', 0, AF_SEARCHABLE, $billtoArray),
     
     'agent_id'=>                  array('Agent',             'link',   ATT_DEFAULT_AGENT_ID, AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'agent'),
     'agent_contact'=>             array('Agent Contact',     'link',   ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'contact'),
      
      //'cc_id'=>                     array('CC',                'link',       0, AF_HIDE, 'cc'), // last used cc
     'remote_ip'=>                 array('IP',                'ip',        ATT_DEFAULT_CLIENT_IP, AF_SEARCHABLE|AF_READONLY, 15 ),
     'remote_cc'=>                  array('Country Code',     'many2one', ATT_DEFAULT_CLIENT_CC, AF_HIDE_LIST, 'countries'),
     'legal_entity_id'=>           array('Legal Entity',      'many2one', ATT_DEFAULT_LEGAL_ID, AF_SEARCHABLE|AF_LOG, 'legal_entity'),
  
     'salutation'=>        array('Salutation',        'str',    '', 0, 10),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 64),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 64),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
     'organization'=>      array('Company/Dept.',     'str',    '', AF_SEARCHABLE, 128),
     'title'=>             array('Title/Attn',        'str',    '', AF_HIDE_LIST, 64),
     
     'language'=>                   array('Language',          'many2one', ATT_DEFAULT_LANGUAGE, 0, 'language'),
  
     'arrival_method'=>            array('Arrive by',         'valSelect',    '', AF_HIDE_LIST|AF_LOG, $arrivalTypeArray),
     'arrival_from'=>              array('City of departure',  'str',    '', AF_HIDE_LIST|AF_LOG, 32),
     'arrival_number'=>            array('Flight/Train #',     'str',    '', AF_HIDE_LIST|AF_LOG, 50),
     'arrival_time'=>              array('Arrival airport/station',   'time',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_LOG),
     'arrival_property'=>          array('Arrival property',  'time',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_LOG),
     'arrival_notes'=>             array('Arrival notes',     'str',    '', AF_HIDE_LIST|AF_LOG, 100),
     'airport_pickup'=>            array('Pickup',            'bool',   0, AF_LOG, 30),
  
     //'pickup_location'=>           array('Pickup location',    'many2one', 0, AF_HIDE_LIST|AF_LOG, 'landmark', array('type' => LANDMARK_TYPE_BOARDING)), // for boats
     //'dropoff_location'=>           array('Dropoff location',   'many2one', 0, AF_HIDE_LIST|AF_LOG, 'landmark', array('type' => LANDMARK_TYPE_BOARDING)), // for boats
  
     'extra_info'=>                array('Guest notes',        'text',   '', AF_HIDE_LIST|AF_READONLY_EDIT|AF_HIDE_ADD),
     'message'=>                   array('Message to guest',  'text',   '', AF_HIDE_LIST),
     'alert'=>                     array('NB!',               'str',    '', AF_HIDE_LIST, 100),
     'notes'=>                     array('Our Notes',         'text',    '', AF_HIDE_LIST),
     'invoice_notes'=>             array('Invoice Notes',     'str',    '', AF_HIDE_LIST, 200),

     'cancel_reason'=>             array('Cancellation reason', 'text',    '', AF_HIDE_LIST),
     'cancel_user_id'=>            array('Canceled by',       'link',    ATT_DEFAULT_NULL, AF_HIDE_LIST, 'user'),
                          
     //'ref'=>                       array('Ref',               'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 20),
     //'ref_other'=>                 array('Ref (other)',       'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 20),
     //'ref_searchterm'=>            array('Searchterm',        'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 50),

     
     'currency'=>                  array('Currency',          'valSelect', 0, AF_HIDE_LIST|AF_LOG, $currencyArray),
     'rate_type'=>                 array('Rate Type',         'valSelect', 'day', AF_HIDE_LIST|AF_OBLIGATORY|AF_LOG, $rateTypeArray),

     'rate_class'=>                array('Rate Class',        'many2one', RATE_CLASS_STANDARD, AF_HIDE_LIST|AF_LOG, 'rate_class'),
     'rate_base'=>                 array('Base Rate',         'money',   0, AF_HIDE_LIST|AF_LOG),
     'rate_locked'=>               array('Lock Rate?',        'bool',  0, AF_HIDE_LIST),
     'rate_locked'=>               array('Lock Rate?',        'bool',  0, AF_HIDE_LIST),
     'invoice_locked'=>            array('Invoice locked?',   'bool',  0, AF_HIDE_LIST),
     'guest_registered'=>          array('Guest registered?', 'bool',  0, AF_HIDE_LIST),

     //'fee_locked'=>                array('Lock Fee?',         'bool',  0, AF_HIDE_LIST),
     'send_email'=>                array('Send email updates?', 'bool',  0, AF_HIDE_LIST),
     'monthly_billing'=>           array('Bill monthly?',   	'bool', 0, AF_HIDE_ADD|AF_HIDE_LIST),
     //'price_breakdown_id'=>        array('Price breakdown',   'link', 0, AF_HIDE_LIST|AF_LOG, 'price_breakdown'),

     'hourly'=>                    array('Hourly billing',    'bool', 0, 0), // redundant, but for simplicity
     'billing_period'=>            array('Billing period',    'many2one', BILLING_PERIOD_DAY, AF_OBLIGATORY, 'billing_period', array('active' => 1)),
     'invoice_contact'=>           array('Invoice Contact?',  'bool', 0, AF_HIDE_ADD|AF_HIDE_LIST),
  
     'discount'=>                  array('Base Discount',     'discount',  0, AF_HIDE_LIST|AF_LOG),
     //'discount_locked'=>           array('Lock Discount?',    'bool',  1, AF_HIDE_LIST),
  
     'total'=>                     array('Total',             'money',   0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL|AF_LOG),
     'tax'=>                       array('Tax',               'money',   0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL|AF_LOG),
     'tax_of_which'=>              array('Of which tax',      'money',   0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL|AF_LOG),
                                                                        
     'charge_city_tax' =>          array('Charge City Tax?' ,'bool', 0, AF_LOG, 0),
     'city_tax_rate' =>            array('City Tax Rate' ,   'surcharge', '', AF_LOG, 0),
     'charge_tax' =>               array('Charge V.A.T?'       ,'bool', 0, AF_LOG, 0),
     'tax_included' =>             array('Tax included?'       ,'bool', 1, AF_LOG, 0),
     
     'fee_service' =>              array('Service fee',       'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'fee_cleaning' =>             array('Cleaning fee',      'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'fee_local'    =>             array('Local fee',         'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'fee_booking'  =>             array('Booking fee',       'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'fee_other'    =>             array('Other fee',         'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'fee_pickup'   =>             array('Pickup fee',        'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     //'fee_callout'=>               array('Call-out fee',      'money',  0,  AF_HIDE_ADD|AF_HIDE_LIST, 8),
     //'fee_callout_quantity'=>      array('Call-out fee quantity', 'float',  0,  AF_HIDE_ADD|AF_HIDE_LIST, '3.2'),
     //'fee_waiting'=>               array('Waiting fee',       'money',  0,  AF_HIDE_ADD|AF_HIDE_LIST, 8),
     //'fee_waiting_quantity'=>      array('Waiting fee quantity',  'float',  0,  AF_HIDE_ADD|AF_HIDE_LIST, '3.2'),
  
     'fee_json'=>                  array('Fees',              'json',   0, AF_READONLY|AF_HIDE_LIST|AF_LOG),
     'fees'=>                      array('Total Fees',        'money',   0, AF_READONLY|AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'city_tax'=>                  array('City Tax',          'money',  0, AF_READONLY|AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'other_charges'=>             array('Other charges',     'money',   0, AF_HIDE_LIST|AF_READONLY|AF_LOG|AF_TOTAL),
     'grand_total'=>               array('Grand Total',       'money',   0, AF_HIDE_LIST|AF_READONLY|AF_LOG|AF_TOTAL),
  
     'deposit'=>                   array('Deposit',           'money',   0, AF_HIDE_LIST|AF_READONLY|AF_LOG),
     'deposit_percentage'=>        array('Deposit %',         'int',   0, AF_HIDE_LIST|AF_LOG, 2),
  
     'paid'=>                      array('Paid',              'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'authorized'=>                array('Authorized',        'money',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'balance'=>                   array('Balance',           'money',   0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL),
     'commission'=>                array('Commisssion',       'commission',   0, AF_HIDE_LIST|AF_LOG|AF_TOTAL),
     'invoice'=>                   array('Invoices',     'one2many', 0, AF_HIDE_LIST|AF_READONLY, 'invoice', 'res_id'),
  
     'booking_id'=>                array('Booking ID',        'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD|AF_READONLY, 20),
     'booking_pin'=>               array('Booking PIN',       'str',    '', AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD|AF_READONLY, 10),
  
     //'feedback_sent'=>             array('Feedback request sent?', 'bool',  0, AF_READONLY|AF_HIDE_LIST),
     //'feedback_confirmed'=>        array('Feedback confirmed?', 'bool',  ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_LIST|AF_LOG),
     //'feedback_score'=>            array('Feedback score?',     'int',  ATT_DEFAULT_NULL, AF_READONLY|AF_LOG),
     //'review_confirmed'=>          array('Review confirmed?', 'bool',  ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_LIST|AF_LOG),

     'reminder_sent'=>             array('Reminder sent?',     'yesno',  0, AF_READONLY|AF_HIDE_LIST),
     'review_sent'=>               array('Review request sent?', 'yesno',  0, AF_READONLY|AF_HIDE_LIST),
     'review_score'=>              array('Review score?',     'int',  ATT_DEFAULT_NULL, AF_READONLY|AF_LOG),
     
     //'block_id'=>                  array('Block ID',    	     'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'block'),
     //'block_parent'=>              array('Block Parent',    	 'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'reservation'),

     'org_currency'=>              array('Original Currency',  'valSelect', 0, AF_HIDE_LIST, $currencyArray),
     'org_rate_type'=>             array('Original Rate Type', 'valSelect', 'day', AF_HIDE_LIST, $rateTypeArray),
     'org_rate_base'=>             array('Original Base Rate', 'money',   0, AF_HIDE_LIST),
     'org_fee_cleaning'=>          array('Original Cleaning Fee', 'money',   0, AF_HIDE_LIST),

     'chat_count'=>                array('Chat count',         'int', 0, AF_HIDE_ADD|AF_READONLY, 3),
     
     'decline_reason'=>            array('Declined reason',     'int',    '', AF_HIDE_LIST),
     'cancel_reason'=>             array('Cancellation reason', 'text',    '', AF_HIDE_LIST),
     'upload'                =>    array('Uploaded Documents',  'media',     '', 0, $reservation_media_options),

     /** one2many - not stored in table */
     'charge'=>                    array('Charges',           'one2many', 0, AF_HIDE_LIST|AF_READONLY, 'charge', 'res_id'),
     'payments'=>                  array('Payments',          'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'payment', 'res_id'),
     'transactions'=>              array('Transactions',      'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'transaction', 'res_id'),
     'withdrawals'=>               array('Withdrawals',       'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'withdrawal', 'res_id'),
     'changelog'=>                 array('Log',               'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE|AF_READONLY, 'changelog',  'parent_id', array('obj_type' => 'reservation')), // last is filter
     'chat'=>                      array('Correspondence',    'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE|AF_READONLY, 'chat',       'parent_id', array('parent_type' => 'reservation')),
     'maillog'=>                   array('All sent mails',    'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE|AF_READONLY, 'mail_queue', 'parent_id', array('parent_type' => 'reservation')), // last is filter
  ),
  
  
  
  // reservation exceptions
  "res_exception" => 
  
  array(
     'id'=>                        array('ID',                'id',         0, AF_PRIMARY|AF_AUTOINCREMENT),
     'active'                   => array('Active',            'bool',       1, AF_SEARCHABLE, 1),
     'created'=>                   array('Created',           'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD),
     'timestamp'=>                 array('Updated',           'timestamp',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD, 6 ),
     'user_id' =>                   array('Author',            'link',     ATT_DEFAULT_USER_ID, AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),
     //'author_id'=>                 array('Author',            'link',   ATT_DEFAULT_USER_ID, AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),
     'org_id' =>                   array('Organization',      'many2one',   ATT_DEFAULT_NULL, AF_SEARCHABLE, 'org', array('client_id' => $_SESSION['client_id'])),
     'site_id'=>                   array('Site',              'link',   ATT_DEFAULT_SITE_ID, 0, 'site'),
                                                                            
  // which city/type/owner/apt (todo - make linked ajax choosers)                                
  
     'rank'=>                      array('Rank',              'int', 0, AF_HIDE_ADD, 3),

     'city_id' =>                  array('City',             'many2one',   0, AF_SEARCHABLE, 'geo_city', array('active' => 1), 'name'),
     'property_type'=>             array('Property Type',     'valSelect',  0, AF_OBLIGATORY, $propertyTypeArray),
//     'owner_id'=>                  array('Owner',             'link',   ATT_DEFAULT_NULL, AF_SEARCHABLE, 'owner', array('shortstay' => 1)),
     'host_id'=>                   array('Host',              'link',   ATT_DEFAULT_NULL, AF_HIDE_ADD|AF_READONLY, 'user'),
     'apt_id' =>                   array('Property',          'link',   ATT_DEFAULT_NULL, AF_SEARCHABLE, 'property'),

  //   'priority'                 => array('Priority'          ,'int', 0, 0, 6),
  
  // reservation parameters (input vars)
     'start'=>                     array('From Date',              'date',       ATT_DEFAULT_NULL, 0),
     'end'=>                       array('To Date',                'date',       ATT_DEFAULT_NULL, 0),

     'start_time'=>                array('From Time',              'time',       ATT_DEFAULT_NULL),
     'end_time'=>                  array('To Time',                'time',       ATT_DEFAULT_NULL),
//     'num_months'=>                array('# Months',          'float',      ATT_DEFAULT_NULL, 0, '3.2'),
     'num_days'=>                  array('# Days >=',         'int',        ATT_DEFAULT_NULL, 0),
     'num_hours'=>                 array('# Hours >=',        'float',      ATT_DEFAULT_NULL, 0, '2.2'),
     'num_guests'=>                array('# Guests ',         'intSelect',  ATT_DEFAULT_NULL, 0, array(1,8,1, '')),
  
  // calculated reservation parameters (input vars)
     //'fit_factor'=>                 array('Fit factor (>= 0-100%)', 'int', ATT_DEFAULT_NULL, 0),
     'fit_between'=>                array('Fit factor between (>= 0-100%)', 'int', ATT_DEFAULT_NULL, 0, 3),
     'fit_next_to'=>                array('Fit factor next to (>= 0-100%)', 'int', ATT_DEFAULT_NULL, 0, 3),
     'days_from_now'=>              array('# Days from now <=',     'int', ATT_DEFAULT_NULL, 0),
     'days_from_now_gt'=>           array('# Days from now >',      'int', ATT_DEFAULT_NULL, 0),
  
  // property exceptions (out vars) 
     'book_direct' =>              array('Book Direct?',       'yesno',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'max_guests'=>                array('Max guests',        'int',        ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'min_stay'=>                  array('Min stay',          'int',        ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'max_stay'=>                  array('Max stay',          'int',        ATT_DEFAULT_NULL, AF_SEARCHABLE),                                                                                

     'currency'=>                  array('Currency',          'valSelect', 0, AF_HIDE_LIST|AF_LOG, $currencyArray),     
     'rate_day'=>                  array('Day rate',          'int',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'rate_weekend'=>              array('Weekend rate',       'int',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
  //         'rate_hour'=>                 array('Hour rate',         'money',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'rate_extraperson'=>          array('Rate Extra person', 'int',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'rate_week'=>                 array('Week rate',          'int',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'rate_2_weeks'=>              array('2-Week rate',          'int',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'rate_3_weeks'=>              array('3-Week rate',          'int',      ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'rate_month'=>                array('Month rate',          'int',      ATT_DEFAULT_NULL, AF_SEARCHABLE),

     'changeover'=>                array('Changeover',        'int',  0, 0),

     //   'discount'=>                  array('Discount',          'discount',   ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'surcharge'=>                 array('Surcharge',         'surcharge',  ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'rate_change'=>               array('Rate Change',       'percent_or_number',  ATT_DEFAULT_NULL, AF_SEARCHABLE),
     'last'=>                      array('Stop processing',    'bool',  0, AF_SEARCHABLE),  
     'description'=>               array('Description',        'str',    '', 0, 255),
   ),
  
  
  "calendar_import" => 
  array(
     'id'=>                  array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT),
     'active'=>              array('Active?',           'bool', 1, AF_READONLY|AF_HIDE_ADD),
     'created'=>             array('Created',           'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
     'last_sync'=>           array('Last Sync',         'datetime',   ATT_DEFAULT_NULL, AF_HIDE_ADD|AF_READONLY),
     'site_id'=>             array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_OBLIGATORY, 'site'),
     'client_id'=>           array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_HIDE_ADD|AF_SEARCHABLE|AF_READONLY_EDIT|AF_OBLIGATORY, 'client'),
     'source_id'=>           array('Source',    			  'many2one', ATT_DEFAULT_SOURCE_ID, AF_OBLIGATORY|AF_SEARCHABLE|AF_READONLY_EDIT, 'source'),
     'user_id'=>             array('User',              'link', ATT_DEFAULT_USER_ID, AF_HIDE_ADD|AF_READONLY|AF_OBLIGATORY, 'user'),
     'host_id'=>             array('Host',              'link', 0, AF_HIDE_ADD|AF_READONLY|AF_OBLIGATORY, 'user'),
     'property_type'=>       array('Property Type',     'valSelect', 0, AF_READONLY|AF_SEARCHABLE|AF_HIDE_ADD|AF_OBLIGATORY, $propertyTypeArray),
     'apt_id'=>              array('Property ID',       'link', 0, AF_HIDE_ADD|AF_READONLY, 'user'),
     'name'               => array('Name',              'str',  '', AF_SEARCHABLE|AF_OBLIGATORY,255),
     'fid'=>                 array('Foreign ID',        'str',  '', 0, 40),
     'fname'               => array('Foreign Name',     'str',  '', AF_SEARCHABLE,255),
     'url'                => array('URL',               'url',  '', AF_SEARCHABLE|AF_OBLIGATORY),
  ),

  
  "invoice" => 
  
  array(       
     'id'=>                        array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT),
     'cc_id'=>                     array('CC',                'link',       0, AF_HIDE, 'cc'), // last used cc
     'res_id'=>                    array('Reservation ID',    'link', 0, AF_SEARCHABLE, 'reservation'),
     //'order_id'=>                  array('Order ID',          'link', 0, AF_SEARCHABLE, 'payment_order'),

     'parent_id'=>                 array('Parent Id',         'parent_id',   ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST, 11),
     'parent_type'=>               array('Parent Type',       'parent_type', ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST),

     'apt_id'=>                    array('Property',          'link', 0, AF_READONLY|AF_SEARCHABLE|AF_HIDE_ADD, 'property'),
     'property_type'=>             array('Property Type',     'valSelect', 0, AF_READONLY|AF_SEARCHABLE|AF_HIDE_ADD, $propertyTypeArray),
     'site_id'=>                   array('Site',              'many2one',   ATT_DEFAULT_SITE_ID, 0, 'site'),
     'city_id'=>                   array('City',              'link',   0, AF_READONLY|AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'geo_city', array('active' => 1), 'name'),
//     'country_id'=>                array('Country',           'link',   0, AF_READONLY|AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'country', array('active' => 1), 'name'),
//     'owner_id'=>                  array('Owner',             'link',   ATT_DEFAULT_OWNER_ID, AF_SEARCHABLE|AF_READONLY|AF_HIDE_EDIT|AF_HIDE_ADD, 'owner', array('shortstay' => 1), 'first_name'),
     'host_id'=>                   array('Host',              'link', 0, AF_HIDE_ADD|AF_READONLY, 'user'),
     'agent_id'=>                   array('Agent',            'many2one',  ATT_DEFAULT_NULL, AF_SEARCHABLE, 'agent'),
     'legal_entity_id'=>           array('Legal Entity',      'many2one', 0, AF_SEARCHABLE|AF_LOG, 'legal_entity'),
     'client_id'=>                 array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'head_invoice'=>              array('Head invoice#',     'bool', 0, AF_READONLY),
     'locked'=>                    array('Locked?',           'bool', 0, AF_READONLY),
     'corporate'=>                 array('Corporate?',        'bool', 0, AF_READONLY),
     'due'=>                       array('Due',               'bool', 0, 0),
     'received'=>                  array('Received on',       'datetime', ATT_DEFAULT_NULL, AF_READONLY),
     'user_id'=>                   array('User ID',           'link', ATT_DEFAULT_USER_ID, AF_HIDE_ADD|AF_READONLY|AF_SEARCHABLE, 'user'),
     'guest_id'=>                  array('Guest ID',          'link', 0, AF_HIDE_ADD|AF_READONLY|AF_SEARCHABLE, 'user'),
  //         'legacy_id'=>                 array('Custom inv. ID',         	'str',   '', 0, 20),
     'guest_name'=>                array('Guest',         		'str',   '', AF_READONLY, 40),
     'org_id'=>                    array('Employer',          'many2one', 0, AF_SEARCHABLE, 'org',["active" => 1]),
     'contact_id'=>                array('Department',    	  'many2one',   ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD, 'contact', ["org_id" => "__this->org_id", "active" => 1]),
     'bill_to'=>                   array('Bill To',           'valSelect', 0, AF_SEARCHABLE, $billtoArray),
     'type'=>                      array('Type',              'many2one', 0, AF_READONLY_EDIT, 'inv_type'),
     'timestamp'=>                 array('Updated',           'timestamp',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
     'status'=>                    array('Status',            'many2one', INV_STATUS_DRAFT, AF_SEARCHABLE|AF_LOG, 'inv_status'),
     'status_change_timestamp'=>   array('Status Changed on', 'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
     'created'=>                   array('Created',           'datetime', ATT_DEFAULT_NOW, AF_READONLY|AF_HIDE_LIST),
     'invoice_date'=>              array('Invoice Date',      'date',   ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_LOG),
     'duedate'=>                   array('Due',               'date',   ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_LOG),
     'start_date'=>                array('From',              'datetime',   '', AF_LOG),
     'end_date'=>                  array('To',                'datetime',   '', AF_LOG),
     'sequence'=>                  array('Sequence #',        'int',    0, AF_READONLY|AF_HIDE_LIST),
  
     'booking_pin' =>              array('Booking PIN',       'str',    '',  AF_HIDE|AF_READONLY, 10),
  
     'invoice_id'=>                array('Invoice ID',        'str',    '', AF_SEARCHABLE|AF_LOG, 30),
     'invoice_id_required'=>       array('Invoice ID Required?', 'bool', 0, AF_HIDE_LIST|AF_READONLY),
     'concerning'=>                array('Concerning',        'str',    '', AF_HIDE_LIST, 200),
     'customer_notes'=>            array('Customer Notes',    'str',    '', AF_HIDE_LIST, 200),
     'location'=>                  array('Location',          'str',    '', AF_HIDE_LIST, 200),
     'notes'=>                     array('Notes',             'text',    '', AF_HIDE_LIST),
     'message'=>                   array('Message',           'text',    '', AF_HIDE_LIST),
  
     'sent_to'=>                   array('Sent to',           'str',    '', AF_HIDE_LIST|AF_LOG, 255),
     'sent_date'=>                 array('Sent Date',         'datetime',   ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_LOG),
  
     'export_timestamp'=>          array('Export timestamp',  'datetime',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD|AF_HIDE_EDIT|AF_READONLY|AF_LOG),
     'export_gateway'=>            array('Export gateway',    'str',    '', AF_HIDE_LIST|AF_HIDE_ADD|AF_LOG|AF_HIDE_EDIT|AF_READONLY, 20),
  
     'currency'=>                  array('Cur',               'valSelect', 0, AF_READONLY, $currencyArray),
     'shipping'=>                  array('Shipping',          'money',  0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL),
     'handling'=>                  array('Handling',          'money',  0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL),
  //         'discount'=>                  array('Discount',          'discount',  0, AF_HIDE_LIST),
     'tax'=>                       array('Tax',               'money',  0, AF_HIDE_LIST|AF_READONLY|AF_TOTAL),
     'tax_of_which'=>              array('Of which tax',      'money',  0, AF_HIDE_ADD|AF_LOG|AF_READONLY),
     'amount'=>                    array('Amount',            'money',  0, AF_READONLY|AF_TOTAL),
     'subtotal'=>                  array('Subtotal',          'money',  0, AF_HIDE_ADD|AF_READONLY),
     'total'=>                     array('Total',             'money',  0, AF_READONLY|AF_TOTAL),
     'paid'=>                      array('Paid',              'money',  0, AF_READONLY|AF_TOTAL|AF_LOG),
     'balance'=>                   array('Balance',           'money',  0, AF_READONLY|AF_TOTAL),
     'pending'=>                   array('Pending Payments',  'money',  0, AF_READONLY|AF_TOTAL|AF_LOG),
     'authorized'=>                array('Authorized Payments', 'money',  0, AF_READONLY|AF_TOTAL|AF_LOG),

     'lineitems'=>                 array('# Lines',           'int',    0, AF_READONLY|AF_HIDE_LIST),
     'paid_date'=>                 array('Paid date',         'datetime',   '', AF_READONLY|AF_HIDE_LIST),

     'salutation'=>        array('Salutation',        'str',    '', 0, 10),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 64),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 64),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
          
     'organization'=>      array('Company/Dept.',     'str',    '', AF_SEARCHABLE, 128),
     'title'=>             array('Title/Attn',        'str',    '', AF_HIDE_LIST, 64),
     
     'inv_lineitems'=>             array('Lineitems',         'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'inv_lineitem', 'inv_id'),
                                
     'charge_city_tax' =>          array('Charge City Tax?' ,'bool', 0, AF_LOG, 0),
     'charge_tax' =>               array('Charge V.A.T?'       ,'bool', 0, AF_LOG, 0),
     'tax_included' =>             array('Tax included?'       ,'bool', 0, AF_LOG, 0),
  
  //         'block_id'=>                  array('Block ID',           'str',      ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 13),
     'block_id'=>                  array('Block ID',    	     'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'block'),
     'block_parent'=>              array('Block Parent',    	 'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'invoice'),
  
     'processed'                => array('Processed'         ,'bool', 0, AF_HIDE),
  
     'payments'     =>             array('Payments',       'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'payment', 'inv_id'),
     'withdrawals'  =>             array('Withdrawals',    'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'withdrawal', 'inv_id'),      
  ),
  
  
  "inv_lineitem" => 
  
  array(                    
     'id'=>                        array('ID',                'id',        0, AF_PRIMARY|AF_AUTOINCREMENT|AF_HIDE),
     'inv_id'=>                    array('InvID',             'parent_id', 0, AF_HIDE|AF_OBLIGATORY, 'invoice'),
     'user_id'=>                   array('User',              'link',      ATT_DEFAULT_USER_ID, AF_HIDE_ADD|AF_READONLY, 'user'),
     'product_id'=>                array('Product',           'many2one',  ATT_DEFAULT_NULL, 0, 'product'),
     'variant_id'=>                array('Variant ID',    		'link',      0, 0, 'product_variant'),
     'supplier_id'=>               array('Supplier',          'many2one',  0, AF_HIDE, 'supplier'),
     'type'=>                      array('Type',              'many2one',  0, 1, 'lineitem_type'),
     'timestamp'=>                 array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY, 6 ),
     'created'=>                   array('Created',           'date',       ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_READONLY),
     'item_number'=>               array('Item number',       'str',       '', AF_HIDE_ADD|AF_READONLY, 40),
     'quantity'=>                  array('Quantity',          'float',     1, 1, '2.4'),
     'currency'=>                  array('Currency',          'valSelect', 0, 1, $currencyArray),
     'amount'=>                    array('Unit price',        'money',     0, AF_LOG),
     'discount'=>                  array('Discount',          'discount',  0, AF_LOG),
     'subtotal'=>                  array('Subtotal',          'money',     0, AF_HIDE_ADD|AF_READONLY),
     'tax_category'=>              array('Tax Category',      'many2one',  0, AF_OBLIGATORY, 'tax_category', array('active' => 1)),
     'tax_percentage'=>            array('Tax %',             'float',     0, AF_LOG, '2.2'),
     'tax_of_which'=>              array('Of which tax',      'money',     0, AF_HIDE_ADD|AF_LOG|AF_READONLY),
     'tax'=>                       array('Tax (added)',       'money',     0, AF_HIDE_ADD|AF_LOG|AF_READONLY),
     'shipping'=>                  array('Shipping',          'money',     0, 0),
     'handling'=>                  array('Handling',          'money',     0, 0),
     'total'=>                     array('Total',             'money',     0, AF_HIDE_ADD|AF_LOG|AF_READONLY),
     'description'=>               array('Description',       'text',  '', 0, 255),
     'account_number' =>           array('Account #',         'str',   '', AF_HIDE_LIST, 20),
  
  //         'block_id'=>                  array('Block ID',           'str',      ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 13),
     'block_id'=>                  array('Block ID',    	     'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'block'),
     'block_parent'=>              array('Block Parent',    	 'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'invoice'),         
  ),
  
  
  "charge" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'status'=>            array('Status',            'int',   0, 0, 4 ),
     'user'=>              array('User',              'int',   ATT_DEFAULT_USER_ID, 0, 6 ),
     'ipn_id'=>            array('IPN',               'int',   0, 0, 6 ),
     'cc_id'=>             array('CC ID',             'link',  1, AF_OBLIGATORY|AF_READONLY, 'cc'),
     'inv_id'=>            array('InvID',             'link', 0, AF_OBLIGATORY|AF_READONLY|AF_SEARCHABLE, 'invoice'),
     'res_id'=>            array('ResId',             'link',  1, AF_OBLIGATORY|AF_READONLY|AF_SEARCHABLE, 'reservation'),
     'timestamp'=>         array('Timestamp',         'timestamp',  ATT_DEFAULT_NOW, 0, 6 ),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_READONLY),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
  
     'paymentaction'=>     array('Action',            'str',  '', AF_HIDE_LIST, 20 ),
     'acct'=>              array('Acct',              'str',  '', AF_HIDE_LIST, 20 ),
     'currency'    =>      array('Currency',          'valSelect', 0, 0, $currencyArray),
     'amount'=>            array('Amount',            'money', 0, 1),
     'description'=>       array('Desc',              'str',  '', AF_HIDE_LIST, 100 ),
     'remote_timestamp'=>  array('RTimeS',            'str',  '', AF_HIDE_LIST, 20 ),
     'correlationid'=>     array('CorId',             'str',  '', AF_HIDE_LIST, 20 ),
     'ack'=>               array('Ack',               'str',  '', 0, 10 ),
     'version'=>           array('Version',           'str',  '', AF_HIDE_LIST, 10 ),
     'build'=>             array('Build',             'str',  '', AF_HIDE_LIST, 10 ),
     'avscode'=>           array('AVS OK',            'str',  '', AF_HIDE_LIST, 2 ),
     'cvv2match'=>         array('CVV OK',            'str',  '', AF_HIDE_LIST, 2 ),
     'transactionid'=>     array('TX ID',             'str',  '', 0,  20 ),
     'amt'=>               array('Amt',               'money', 0, AF_HIDE_LIST),
     'payment_error'=>     array('Error',             'text',  '', AF_HIDE_LIST),
  ),
  
  
  "gateway" => 
  
  array(
     'id'               => array('ID'             ,'id',   0, AF_PRIMARY|AF_AUTOINCREMENT|AF_HIDE),
     'name'             => array('Name'           ,'str', '', AF_SEARCHABLE|AF_OBLIGATORY, 50),
  
     'username'         => array('UserName'      ,'str', '', 0, 100),
     'password'         => array('Password'      ,'str', '', 0, 100),
     'signature'        => array('Signature'     ,'str', '', 0, 200),
     'endpoint'         => array('Endpoint'      ,'str', '', 0, 200),
  
     'test_username'         => array('Tetst UserName'      ,'str', '', 0, 100),
     'test_password'         => array('Tetst Password'      ,'str', '', 0, 100),
     'test_signature'        => array('Tetst Signature'     ,'str', '', 0, 200),
     'test_endpoint'         => array('Tetst Endpoint'      ,'str', '', 0, 200),
  
  ),
  
  
  "transaction" => 
  // for testing
  // $inv_id = 2679;
  // $pay_id = 1145;
  // $res_id = 35195;
  
  array(
     'tx_id'=>             array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     //'tx_st'=>             array('User level',        'many2one', 0, 'tx_st'),
     'cc_id'=>             array('CC ID',             'link',  ATT_DEFAULT_NULL, AF_READONLY_EDIT, 'cc'),
     'inv_id'=>            array('InvID',             'link',  ATT_DEFAULT_NULL, AF_READONLY_EDIT, 'invoice'),
     'res_id'=>            array('ResId',             'link',  ATT_DEFAULT_NULL, AF_READONLY_EDIT, 'reservation'),
     // 'order_id'=>          array('Order',    			    'link',  ATT_DEFAULT_NULL, AF_READONLY, 'payment_order'),
     'payment_id'=>        array('PaymentId',         'link',  ATT_DEFAULT_NULL, AF_READONLY_EDIT, 'payment'),
//     'owner_id'=>          array('OwnerId',           'link',  ATT_DEFAULT_NULL, AF_READONLY_EDIT, 'owner'),
     'host_id'=>           array('Host',              'link',  ATT_DEFAULT_NULL, AF_HIDE_ADD|AF_READONLY, 'user'),
     'client_id'=>         array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>           array('Site',    			    'link', ATT_DEFAULT_SITE_ID, AF_READONLY|AF_HIDE, 'site'),
  
     'tx_st'=>               array('TxSt',              'many2one', '', AF_SEARCHABLE, 'tx_status' ),
     'timestamp'=>           array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_READONLY),
     'tx_gateway'=>          array('Gateway',           'str',      '', AF_SEARCHABLE, 10 ),
     'tx_type'=>             array('TX Type',           'str',      '', AF_SEARCHABLE, 40 ),
     'ipn_test'=>            array('Test',              'bool',     0, 0),
     'txn_res'=>             array('TxnRes',            'str',      '', AF_SEARCHABLE, 10 ),
     'txn_id'=>              array('TxnId',             'str',      '', AF_SEARCHABLE, 50 ),
     'parent_txn_id'=>       array('Parent TxnId',      'str',      '', AF_SEARCHABLE, 50 ),
     'item_name'=>           array('Item Name',         'str',      '', AF_SEARCHABLE, 255),
     'item_number'=>         array('Item Number',       'str',      '', AF_SEARCHABLE, 20 ),
     'payment_date'=>        array('Date',              'str',      '', AF_SEARCHABLE, 22 ),
  
     'payment_status'=>    array('Payment Status',    'many2one', 0, AF_READONLY_EDIT, 'payment_status'),
     'pending_reason'=>    array('Pending Reason',    'str',    ATT_DEFAULT_NULL, AF_READONLY, 20),
     'reason_code'=>       array('Reason Code',       'str',    ATT_DEFAULT_NULL, AF_READONLY, 30),
  
     'payer_email'=>       array('Sender',          'str',    '', AF_SEARCHABLE, 20 ),
     'receiver_email'=>    array('Received',        'str',      '', AF_SEARCHABLE, 20 ),
  
     'memo'=>                array('Memo',             'str',      '', AF_SEARCHABLE|AF_HIDE_LIST, 255 ),
     'tx_notes'=>            array('Notes',           'str',      '', AF_SEARCHABLE, 4 ),
  
     'mc_gross'=>        array('Gross',                 'str',      '', AF_SEARCHABLE, 22 ),
     'mc_currency'=>     array('Currency',              'str',      '', AF_SEARCHABLE, 22 ),
     'mc_fee'=>          array('Fee',                   'str',      '', AF_SEARCHABLE, 22 ),
  
     'settle_currency'=>     array('Settle Currency',     'str',      '', AF_SEARCHABLE, 3 ),
     'settle_amount'=>       array('Settle Amount',       'str',      '', AF_SEARCHABLE, 22 ),
     'exchange_rate'=>       array('Exchange rate',       'str',      '', AF_SEARCHABLE, 22 ),
     
  
     'auth_id'=>         array('Auth ID',              'str',      '', AF_HIDE_LIST|AF_SEARCHABLE, 20),
     'auth_amount'=>     array('Auth Amount',          'str',      '', AF_HIDE_LIST|AF_SEARCHABLE, 20),
     'auth_exp'=>        array('Auth Expiration',      'str',      '', AF_HIDE_LIST|AF_SEARCHABLE, 25),
  
     'first_name'=>        array('First Name',      'str',      '', AF_HIDE_LIST|AF_SEARCHABLE, 50),
     'last_name'=>         array('Last Name',       'str',      '', AF_HIDE_LIST|AF_SEARCHABLE, 50),
     'residence_country'=> array('Country',         'str',      '', AF_HIDE_LIST|AF_SEARCHABLE, 2),

     'refund_amount'=>     array('Refund Amount',    'money', 0, AF_SEARCHABLE),
     'bank_id'=>          array('Bank ID',              'str',      '', AF_SEARCHABLE, 10 ),
      'bank_name'=> array('Bank name', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'bank_account'=> array('Bank account', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'transfer_reference'=> array('Transfer reference', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'consumer_name'=> array('Consumer name', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'consumer_account'=> array('Consumer account', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'consumer_bic'=> array('Consumer bic', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'bitcoin_address'=> array('Bitcoin address', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'bitcoin_amount'=> array('Bitcoin amount', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'bitcoin_rate'=> array('Bitcoin rate', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'bitcoin_uri'=> array('Bitcoin uri', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'paypal_reference'=> array('Paypal reference', 'str', '', AF_HIDE_LIST|AF_SEARCHABLE, 128),
      'amount_remaining'=> array('Amount remaining', 'money', 0, AF_HIDE_LIST|AF_SEARCHABLE),
  ),
  
  
  "payment_type" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>              array('Name',              'str',   '', 1, 20 ),
     'user_select'=>       array('Selectable source?',  'bool', 0, AF_READONLY|AF_HIDE_ADD),
     'refundable'=>        array('Refundable?',  'bool', 0, AF_READONLY|AF_HIDE_ADD),
  ),
  
  
  "payment_source" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>              array('Name',              'str',   '', 1, 20 ),
     'description'=>       array('Description',       'str',   '', 1, 200 ),
  ),
  
  
  "payment_status" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>              array('Name',              'str',   '', 1, 20 ),
  ),
  
  
  "withdrawal" => 
  array(
     'id'=>                array('ID',                'id',   0,    AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE, 14 ),
     'created'=>           array('Date',              'datetime',   ATT_DEFAULT_NOW, AF_READONLY),
     'payment_id'=>        array('Payment',           'link',       ATT_DEFAULT_NULL, AF_SEARCHABLE, 'payment'),
//     'owner_id'=>          array('Owner',             'link',       ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY, 'owner'),
     'recipient_id'=>      array('Recipient',         'link', 0, AF_HIDE_ADD|AF_READONLY, 'user'),
     'host_id'=>           array('Host',              'link',       ATT_DEFAULT_NULL, AF_HIDE_ADD|AF_READONLY, 'user'),
     'agent_id'=>          array('Agent',             'link',       ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_HIDE|AF_READONLY, 'agent'),
     'client_id'=>         array('Client',    			  'many2one',   ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'gateway_id'=>        array('Gateway',           'many2one',   ATT_DEFAULT_NULL, AF_OBLIGATORY, 'gateway'),
     'inv_id'=>            array('InvID',             'link',       ATT_DEFAULT_NULL, AF_SEARCHABLE, 'invoice'),
     'res_id'=>            array('ResId',             'link',       ATT_DEFAULT_NULL, AF_SEARCHABLE, 'reservation'),
     'tx_id'=>             array('Our TXID',          'link',       ATT_DEFAULT_NULL, 0, 'transaction'),
  
     'payment_type'=>      array('Payment Method',    'many2one',   0, AF_READONLY_EDIT, 'payment_type'),
     'currency'=>          array('Currency',          'valSelect',  0, AF_READONLY|AF_OBLIGATORY, $currencyArray),
     'gross'=>             array('Amount',            'money',      0, AF_READONLY|AF_OBLIGATORY|AF_TOTAL),
     'fee'=>               array('Fee',               'money',      0, AF_READONLY|AF_OBLIGATORY|AF_TOTAL),
     'correction'=>        array('Correction',        'money',      0, AF_READONLY),
     'net'=>               array('Net',               'money',      0, AF_READONLY|AF_OBLIGATORY|AF_TOTAL),
     'notes'=>             array('Notes',             'str',        '', AF_HIDE_LIST, 255),
     'invoices'=>          array('Invoices',          'text',        '', AF_HIDE_LIST, 255),
     'recipient'=>         array('Recipient',         'str',        '', AF_OBLIGATORY, 255),
  
     'gateway_fee'=>       array('Gatweay Fee',         'money',    ATT_DEFAULT_NULL, AF_HIDE|AF_TOTAL),
     'gateway_ts'=>        array('Gatweay Timestamp',   'str',      ATT_DEFAULT_NULL, AF_READONLY, 30),
     'txn_id'=>            array('Gateway TXID',        'str',      ATT_DEFAULT_NULL, AF_READONLY|AF_SEARCHABLE, 50),
  
     'payments'=>          array('Payments',          'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'payment', 'withdrawal_id'),
  
  ),
  
  
  "payment" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'parent_id'=>         array('Parent Payment',    'link',   0, AF_SEARCHABLE, 'payment'),
     'client_id'=>         array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>           array('Site',    			    'link', ATT_DEFAULT_SITE_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'site'),
     'legal_entity_id'=>   array('Legal Entity',      'link', ATT_DEFAULT_LEGAL_ID, AF_SEARCHABLE|AF_LOG, 'legal_entity'),
     'timestamp'=>         array('Timestamp',         'timestamp',  ATT_DEFAULT_NOW, AF_HIDE, 14 ),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_SEARCHABLE|AF_READONLY),
     'payment_date'=>      array('Paid Date',         'datetime',  ATT_DEFAULT_NOW, AF_SEARCHABLE|AF_READONLY),
     'tx_id'=>             array('Our TXID',           'link', 0, 0, 'transaction'),
     'inv_id'=>            array('InvID',             'link', 0, 0, 'invoice'),
     'invoice_id'=>        array('Invoice ID',        'str',    '', AF_SEARCHABLE|AF_LOG, 30),
     'res_id'=>            array('ResId',             'link',  0, AF_SEARCHABLE, 'reservation'),
     'org_id'=>            array('OrgId',             'link',  0, AF_SEARCHABLE, 'org'),
     'apt_id'=>            array('Rental Id',         'link',  0, AF_SEARCHABLE, 'property'),
     // 'order_id'=>          array('Order',    			    'link', ATT_DEFAULT_NULL, AF_READONLY, 'payment_order'),
     'user_id'=>           array('Author',            'link',   ATT_DEFAULT_USER_ID, AF_HIDE_LIST|AF_SEARCHABLE|AF_READONLY, 'user'),
//     'from_owner_id'=>     array('From Owner',        'link',  0, AF_SEARCHABLE|AF_READONLY_EDIT, 'owner'),
//     'owner_id'=>          array('Owner',             'link',  0, AF_SEARCHABLE|AF_READONLY_EDIT, 'owner'),
     'sender_id'=>         array('Sender',            'many2one', 0, 0, 'user'),
     'recipient_id'=>      array('Recipient',         'many2one', 0, 0, 'user'),
     'host_id'=>           array('Host',              'link', 0, AF_HIDE_ADD|AF_READONLY, 'user'),
     'guest_id'=>          array('Guest',             'link',   ATT_DEFAULT_NULL, AF_HIDE_ADD|AF_READONLY|AF_HIDE_LIST|AF_SEARCHABLE, 'user'),
     'agent_id'=>          array('Agent',             'link',  ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY_EDIT, 'agent'),
     'cc_id'=>             array('CC ID',             'link',  0, AF_SEARCHABLE, 'cc'),
     'charge_id'=>         array('Charge',            'link',  0, AF_SEARCHABLE, 'charge'),
     'gateway_id'=>        array('Gateway',           'many2one', 0, AF_SEARCHABLE, 'gateway'),
     'remote_ip'=>         array('IP',                'ip',   ATT_DEFAULT_CLIENT_IP, AF_READONLY|AF_SEARCHABLE, 15 ),
  
     'verified'=>          array('Verified?',         'bool', 0, AF_READONLY),
     'txn_id'=>            array('Gateway TXID',      'str',  '', AF_READONLY|AF_SEARCHABLE, 30),

     'show_invoice'=>      array('Show on invoice?',  'bool', 1, AF_HIDE_ADD),
     'payment_type'=>      array('Payment Method',    'many2one', 0, AF_SEARCHABLE|AF_READONLY_EDIT, 'payment_type'),
     'payment_source'=>    array('Payment Source',    'many2one', PAYMENT_SOURCE_GUEST, AF_SEARCHABLE|AF_READONLY_EDIT, 'payment_source'),
     'source_id'=>         array('Source ID',         'int', ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY_EDIT), // really a link to agent/guest/org/owner/client etc..
  
     'currency'=>          array('Currency',          'valSelect', 0, AF_OBLIGATORY, $currencyArray),
     'amount'=>            array('Amount',            'money', 0, AF_OBLIGATORY|AF_TOTAL),
     'amount_remaining'=>  array('Amount remaining',  'money', 0, AF_HIDE_LIST|AF_SEARCHABLE),
     'description'=>       array('Desc',              'str',  '', AF_HIDE_LIST, 255),
  //         'transaction_id'=>    array('Tx ID',            'link',  1, AF_SEARCHABLE|AF_HIDE, 'transaction'),
  
     'payment_currency'=>  array('Pay Cur',           'str',    ATT_DEFAULT_NULL, AF_READONLY, 3),
     'payment_gross'=>     array('Gross',             'money',  ATT_DEFAULT_NULL, AF_READONLY|AF_TOTAL, 'payment_currency'),
     'payment_fee'=>       array('Fee',               'money',  ATT_DEFAULT_NULL, AF_READONLY|AF_TOTAL, 'payment_currency'),
     'payment_net'=>       array('Net',               'money',  ATT_DEFAULT_NULL, AF_READONLY|AF_TOTAL, 'payment_currency'),
     'payment_notes'=>     array('Notes',             'str',    ATT_DEFAULT_NULL, AF_HIDE_LIST, 255 ),
  
     'settle_currency'=>   array('Rcv Cur',           'str',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY, 3),
     'settle_amount'=>     array('Rcvd',              'money',  ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_TOTAL),
     'exchange_rate'=>     array('Rate',              'float',  ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY, '6.6'),
  
     'payment_status'=>    array('Payment Status',    'many2one', 0, AF_SEARCHABLE|AF_READONLY_EDIT, 'payment_status'),
     'pending_reason'=>    array('Pending Reason',    'str',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY, 20),
     'reason_code'=>       array('Reason Code',       'str',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY, 30),
  
     'withdrawal_id'=>        array('Withdrawal ID',                'link',     ATT_DEFAULT_NULL, AF_READONLY, 'withdrawal'),
     'withdrawal_date'=>      array('Withdrawal Date',              'datetime', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY),
     'withdrawal_recipient'=> array('Withdrawal Recipient',         'str',      ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY, 100),
     'withdrawal_currency'=>  array('Withdrawal Currency',          'str',      ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY, 3),
     'withdrawal_amount'=>    array('Withdrawal Amount',            'money',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_TOTAL),
     'withdrawal_fee'=>       array('Withdrawal Fee',               'money',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY),
     'withdrawal_type'=>      array('Withdrawal Payment Method',    'many2one', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY, 'payment_type'),
     'withdrawal_tx_id'=>     array('Withdrawal TXID',              'link',     ATT_DEFAULT_NULL, AF_READONLY, 'transaction'),
  
     'withdrawals'=>       array('Withdrawals',       'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE, 'withdrawal', 'payment_id'),
  
  //         'block_id'=>                  array('Block ID',           'str',      ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 13),
     'block_id'=>                  array('Block ID',    	     'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'block'),
     'block_parent'=>              array('Block Inv ID',    	 'link',     ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 'invoice'),         
  
     'inv_lineitem_list'    =>  array('Lineitem list',    	   'str',     '', AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD|AF_HIDE_EDIT, 255),
  ),
  

  

  "address" => 
  
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT|AF_HIDE),
     'salutation'=>        array('Salutation',        'valSelect', '', 0, $salutationArray),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 50),
     'infix'=>             array('Infix',             'str',    '', AF_SEARCHABLE, 20),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 50),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'organization'=>      array('Company/Dept.',     'str',    '', AF_SEARCHABLE, 200),
     'title'=>             array('Title/Attn',        'str',    '', AF_HIDE_LIST, 50),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 50),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 50),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 50),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 50),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 10),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'home_phone'=>        array('Home Phone',        'phone',  '', AF_HIDE_LIST, 20),
     'work_phone'=>        array('Work Phone',        'phone',  '', AF_HIDE_LIST, 20),
     'cell_phone'=>        array('Cell Phone',        'phone',  '', AF_HIDE_LIST, 20),
     'phone1'=>            array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
     'phone1_type'=>       array('Phone Type',        'many2one',  0, AF_HIDE_LIST, 'phone_type'),
     'phone2'=>            array('Phone2',            'phone',  '', AF_HIDE_LIST, 20),
     'phone2_type'=>       array('Phone2 Type',       'many2one',  0, AF_HIDE_LIST, 'phone_type'),
     'phone3'=>            array('Phone3',            'phone',  '', AF_HIDE_LIST, 20),
     'phone3_type'=>       array('Phone3 Type',       'many2one',  0, AF_HIDE_LIST, 'phone_type'),
     'fax'=>               array('Fax',               'phone',  '', AF_HIDE_LIST, 20),
     'web_address'=>       array('Web',               'url',    '', AF_HIDE_LIST, 50),

     'facebook'=>          array('Facebook',          'url',    '', AF_HIDE_LIST, 255),
     'twitter'=>           array('Twitter',           'str',    '', AF_HIDE_LIST, 255),
     'address_formatted'=> array('Formatted Address', 'text',   '', AF_HIDE_LIST),

     'address_type'=>      array('Address type',    	'many2one',   ADDRESS_TYPE_PHYSICAL, AF_HIDE_LIST, 'address_type'),
     'address_parent_id'=>         array('Parent Id',         'parent_id',   ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST, 11),
     'address_parent_type'=>       array('Parent Type',       'parent_type', ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST),
  
  ),
  
  "user" => 
  array(
     'id'=>                array('ID',                'id',         0,   AF_PRIMARY|AF_AUTOINCREMENT),
     'created'=>           array('Created',           'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY),
     'last_login'=>        array('Last login',        'datetime',   ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD|AF_HIDE_EDIT|AF_READONLY),
     'active'=>            array('Active?',           'bool', 1, AF_READONLY|AF_HIDE_ADD),
     'client_id'=>         array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>           array('Site',    			    'link', ATT_DEFAULT_SITE_ID, AF_HIDE_ADD|AF_SEARCHABLE|AF_READONLY_EDIT, 'site'),
     'aff_id'=>            array('Affiliate',         'link', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_SEARCHABLE|AF_READONLY, 'user'),
     'auth_sites'=>        array('Allowed Sites',     'msel',     '', 0, 'site', array('active' => 1)),
//     'profile_id'=>        array('Profile',    			  'link', 0,   0, 'user_profile'),
//     'profile_id'=>        array('Profile',    			  'one2one', 0,   AF_HIDE_LIST|AF_CASCADE_DELETE|AF_ONE2ONE_INTEGRATE, 'user_profile'),
     'profile_id'=>        array('Profile',    			  'one2one', 0,   AF_HIDE_LIST|AF_CASCADE_DELETE|AF_OBLIGATORY|AF_ONE2ONE_INTEGRATE, 'user_profile'),
     'verified'=>          array('Verified?',         'bool', 1, AF_READONLY|AF_HIDE_ADD),
     'user_level'=>        array('Type',              'many2one',   USER_LEVEL_USER,   AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY|AF_OBLIGATORY, "user_level"),
     'roles'=>             array('Roles',             'flags', 1, AF_OBLIGATORY|AF_READONLY|AF_HIDE_ADD, $__USER_ROLES),
     'username'=>          array('Username',          'str', '',    AF_UNIQUE|AF_OBLIGATORY|AF_SEARCHABLE, 50),
     'password'=>          array('Password',          'str',   '',  AF_HIDE, 32),
     'auto_password'=>     array('Generated password?', 'bool', 0, AF_READONLY|AF_HIDE_ADD),
  ),
  
  

  
  
  "org" => 
  
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT),
     'active'=>            array('Active',            'bool',   1, AF_HIDE_ADD),
     'name'=>              array('Name',  'str', '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
     'invoice'=>           array('Pay by invoice?',   'bool', 0, AF_HIDE_LIST),
     'monthly_billing'=>   array('Bill monthly?',   	'bool', 0, AF_HIDE_LIST),

     'salutation'=>        array('Salutation',        'str',    '', 0, 10),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 64),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 64),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
          
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_HIDE_ADD|AF_SEARCHABLE|AF_READONLY_EDIT, 'client'),
  
     'export_timestamp'=>          array('Export timestamp',  'datetime',    ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD|AF_HIDE_EDIT|AF_READONLY|AF_LOG),
     'export_gateway'=>            array('Export gateway',    'str',    '', AF_HIDE_LIST|AF_HIDE_ADD|AF_LOG|AF_HIDE_EDIT|AF_READONLY, 20),
  
     'default_address'=>     array('Default Address',    	  'many2one', ADDRESS_TYPE_PHYSICAL, AF_OBLIGATORY|AF_HIDE_LIST, 'address_type'),
     'comment' =>        	   array('Comment',        			  'text',     '',  AF_HIDE_LIST),
     'coc_number'=>          array('Chamber of Commerce #', 'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'coc_city'=>            array('Ch. of Com. City',      'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'tax_number'=>          array('Tax number',            'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'bank_name'=>           array('Bank name',             'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'bank_city'=>           array('Bank city',             'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'bank_account_number'=> array('Bank Acct. #',          'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 30),
  //         'bank_branch_id'=>      array('Bank branch',         'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 100),
     'bank_iban_number'=>    array('Bank IBAN #',           'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 30),          
     'bank_routing_number'=> array('Bank Routing #',        'str',      '',  AF_SEARCHABLE|AF_HIDE_LIST, 20),
  
     'addresses'=>           array('Address List',          'one2many', 0, AF_HIDE_LIST, 'address', 'address_parent_id', array('address_parent_type' => 'org')),
     'contacts' =>           array('Contacts',              'one2many', 0, AF_HIDE_LIST, 'contact', 'org_id'),
                                                                                                          
  ),
  
  
  "contact_status" =>
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),

  "contact" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'modified'=>          array('Modified',          'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, 0),
  //         'name'=>              array('Name',              'str',   '', AF_OBLIGATORY, 50 ),
     'client_id'=>         array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id' =>          array('Site',              'link', ATT_DEFAULT_SITE_ID, AF_OBLIGATORY, 'site'),
     'org_id'=>            array('Organization',      'link',  ATT_DEFAULT_NULL, AF_SEARCHABLE, 'org'),                               
     'agent_id'=>          array('Agent',             'link',  ATT_DEFAULT_NULL, AF_SEARCHABLE, 'agent'),                               
     'language'=>          array('Language',    			'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),                               

     'salutation'=>        array('Salutation',        'str',    '', 0, 10),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 64),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 64),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
     
     'organization'=>      array('Company/Dept.',     'str',    '', AF_SEARCHABLE, 128),
     'title'=>             array('Title/Attn',        'str',    '', AF_HIDE_LIST, 64),
     
     ),
  
  
  "product" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'modified'=>          array('Modified',          'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, AF_HIDE_ADD),
     'source_type'=>       array('Source Type',       'str',  '', AF_HIDE, 50),
     'fid'=>               array('Foreign ID',        'str',  '', AF_SEARCHABLE, 20),
     'client_id'=>         array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_HIDE_LIST|AF_OBLIGATORY|AF_SEARCHABLE, 'client'),
     'category_id'=>       array('Category',    		  'many2one', 0, AF_OBLIGATORY|AF_SEARCHABLE, 'product_category'),
  //         'language'=>          array('Language',    			'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),                               
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100 ),
     'description'=>       array('Description',       'text',  '', AF_HIDE_LIST),
     'comment'=>           array('Comment',           'str',   '', AF_HIDE_LIST, 255),
     'unit'=>              array('Unit',    		      'many2one', 'EA', AF_OBLIGATORY, 'product_unit'),
     'currency' =>         array('Currency',          'valSelect', 'EUR', AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST, $currencyArray),
     'price'=>             array('Price',             'money',   0, 0),
     'minimum_quantity'=>  array('Min. Quantity',     'int',   0, 0),
     'cost_price'=>        array('Cost Price',        'money',   0, 0),
  //         'tax_percentage'=>    array('Tax Percentage',    'float', 0, 0, '2.2'),
     'tax_category'=>      array('Tax Category',      'many2one',  0, AF_OBLIGATORY, 'tax_category', array('active' => 1)),
     'tax_included'=>      array('Tax Included?',     'bool', 1, 0),
     'image' =>            array('Image',             'image', '', AF_HIDE_LIST, 128),
     'media' =>            array('Media',             'media', '', AF_HIDE_LIST, $product_media_options),
     'num_hours'=>         array('#Hours (packages)', 'float', 0, 0, '2.2'),
//     'stock'=>             array('Stock',             'float', 0, 0, '2.2'),
     ),
  
  "product_variant" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'modified'=>          array('Modified',          'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'product_id'=>        array('Product ID',    		'link', ATT_DEFAULT_NULL, AF_OBLIGATORY, 'product'),
     'name'=>              array('Name',              'str', ATT_DEFAULT_NULL, AF_OBLIGATORY, 100),
     'option_id'=>         array('Option ID',    	    'link', ATT_DEFAULT_NULL, AF_OBLIGATORY, 'product_option'),
     'option_name'=>       array('Option Name',       'str', ATT_DEFAULT_NULL, AF_OBLIGATORY, 100),
     'option_value'=>      array('Option Value',      'str', ATT_DEFAULT_NULL, AF_OBLIGATORY, 50),
     'price'=>             array('Price',             'money',   0, 0),
     'stock'=>             array('Stock',             'float', 0, 0, '2.2'),
     ),
     
  "product_option" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY, 50 ),
     'description'=>       array('Description',       'text',   '', AF_HIDE_LIST),
     'values'=>            array('Values',            'text',   '', AF_HIDE_LIST),
     ),
    
  "product_category" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'modified'=>          array('Modified',          'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY, 'client'),
     'language'=>          array('Language',    			'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),                               
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY, 50 ),
     'description'=>       array('Description',       'text',   '', AF_HIDE_LIST),
     'comment'=>           array('Comment',           'str',   '', AF_HIDE_LIST, 255),
     'unit'=>              array('Default Unit',              'str',   '', AF_HIDE_LIST, 10),
     'tax_percentage'=>    array('Default Tax Percentage',    'float',   0, 0),
     'tax_category'=>      array('Default Tax Category',      'many2one',  0, 0, 'tax_category'),
     'tax_included'=>      array('Default Tax Included?',     'bool',    0, 0),
     ),
  
  
  "product_unit" => 
  array(
     'code'=>              array('Code',              'str',   0, AF_PRIMARY, 2),
     'active'=>            array('Active',            'bool',   1, 0),
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY, 50 ),
     ),
  
  "product_supplier_map" => 
  array(
     'product_id'=>       array('Product',            'many2one',  0, AF_OBLIGATORY, 'product'),
     'supplier_id'=>      array('Supplier',           'many2one',  0, AF_OBLIGATORY, 'supplier'),
     ),
  
  "user_map" => 
  array(
     'id'=>            array('ID',             'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'created'=>       array('Created',        'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY),
     'active'=>        array('Active',         'bool',  1, 0),
     'soft_delete'=>   array('soft_delete',    'bool',  1, 0),
     'verified'=>      array('Verified',       'bool',  0, 0),
     'site_id'=>       array('Site ID',        'link',  0, AF_OBLIGATORY, 'site'),
     'user_id'=>       array('User',           'link',  0, AF_OBLIGATORY, 'user'),
     //'page_id'=>       array('Page ID',         'text',  0, AF_OBLIGATORY, 'minisite_page'),
     'host_user_id'=>  array('Host',           'link',  0, AF_OBLIGATORY, 'user'),
     'permissions' =>  array('Permissions',    'json', '', 0),
     ),               

  "auth_log" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE|AF_READONLY, 14 ),
     //'expires'=>           array('Expires',           'datetime',   ATT_DEFAULT_NULL, AF_HIDE|AF_READONLY, 14 ),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'user_id' =>          array('User ID',           'link', ATT_DEFAULT_USER_ID, 0, 'user'),     
     'provider_id' =>      array('Provider ID',       'link', 0, 0, 'provider'),     
     'username' =>         array('Username',          'str', ATT_DEFAULT_USERNAME, 0, 50),     
     'user_level' =>       array('User Level',        'link', ATT_DEFAULT_USER_LEVEL, 0, 'user'),     
     'status' =>           array('Status',            'int', 0, 0),     
     'display_name' =>     array('Display Name',      'str', ATT_DEFAULT_DISPLAY_NAME, 0, 128 ),
     'remote_ip'=>         array('IP',                'ip',  ATT_DEFAULT_CLIENT_IP, AF_HIDE_LIST|AF_SEARCHABLE|AF_READONLY, 20),
     'remote_cc'=>         array('CC',                'many2one', ATT_DEFAULT_CLIENT_CC, AF_HIDE_LIST, 'countries'),
     'remote_city'=>       array('City',              'str', ATT_DEFAULT_CLIENT_CITY, AF_HIDE_LIST, 64),
     'remote_continent'=>  array('City',              'str', ATT_DEFAULT_CLIENT_CONTINENT, AF_HIDE_LIST, 2),
     'remote_location'=>   array('City',              'str', ATT_DEFAULT_CLIENT_LOCATION, AF_HIDE_LIST, 128),
     'remote_agent'=>      array('User Agent',        'str', ATT_DEFAULT_USER_AGENT, AF_HIDE_LIST),
     'message' =>          array('Message',           'str', '', 0, 255 ),
     
     'session_id'     => array('Session ID',          'str', ATT_DEFAULT_SESS_ID, 0, 128 ),
     'auth_token'     => array('Auth Token',          'str', '', 0, 128 ),
     
  ),

  "mail_log" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE|AF_READONLY, 14 ),

     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'user_id' =>          array('User ID',           'link', ATT_DEFAULT_USER_ID, 0, 'user'),     
     'mail_id' =>          array('Mail ID'  ,         'link', 0, AF_OBLIGATORY, 'mail_queue'),     
     'receiver_id' =>      array('Receiver ID',       'link', 0, 0, 'user'),     
     
     /** not yet impl */
     'campaign_id' =>      array('Mail Campaign ID',  'link', 0, 0, 'mail_campaign'),     

     'mail_guid'=>       array('GUID',             'str', '', 24),

     'remote_ip'=>         array('IP',                'ip',  ATT_DEFAULT_CLIENT_IP, AF_HIDE_LIST|AF_SEARCHABLE|AF_READONLY, 20),
     'remote_cc'=>         array('CC',                'link', ATT_DEFAULT_CLIENT_CC, AF_HIDE_LIST, 'countries'),
     'remote_city'=>       array('City',              'str', ATT_DEFAULT_CLIENT_CITY, AF_HIDE_LIST, 64),
     'remote_continent'=>  array('City',              'str', ATT_DEFAULT_CLIENT_CONTINENT, AF_HIDE_LIST, 2),
     'remote_location'=>   array('City',              'str', ATT_DEFAULT_CLIENT_LOCATION, AF_HIDE_LIST, 128),
     'remote_agent'=>      array('User Agent',        'str', ATT_DEFAULT_USER_AGENT, AF_HIDE_LIST, 128),
     'remote_referer'=>      array('Referer',         'str', ATT_DEFAULT_USER_AGENT, AF_HIDE_LIST, 128),

     'data'             => array('data',              'json',     '', AF_HIDE_ADD),
     
  ),

  "wishlist" => 
  array(
     'id'=>            array('ID',             'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'created'=>       array('Created',        'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'site_id'=>       array('Product',        'link',  0, AF_OBLIGATORY, 'site'),
     'user_id'=>       array('User',           'link',  0, AF_OBLIGATORY, 'user'),
     'obj_type'=>      array('Object ID',      'str',  0, AF_OBLIGATORY, 128),
     'obj_id'=>        array('Object ID',      'int',  0, AF_OBLIGATORY),
     'name'=>          array('Name',           'str',  '', 0, 128 ),
     'description'=>   array('Descriptions',   'html', '', AF_HIDE_LIST),
     
     ),               

  "price_breakdown" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE|AF_READONLY, 14 ),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
     'res_id' =>           array('Site',              'link', ATT_DEFAULT_SITE_ID, AF_OBLIGATORY, 'reservation'),
     'apt_id' =>           array('Property',          'link', ATT_DEFAULT_USER_ID, 0, 'property'),     
     'currency' =>         array('Currency',          'valSelect', 'EUR', AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST, $currencyArray),
     'city_tax'=>          array('City Tax',          'money',  0, AF_READONLY),     
     'fees'=>              array('Fees',              'money',  0, AF_READONLY),     
     'total'=>             array('Total',             'money',  0, AF_READONLY),     
     'average'=>           array('Averate',           'money',  0, AF_READONLY),     
     'count'=>             array('Item count',        'int',  0, AF_READONLY),     
     'discount'=>          array('Discount',          'discount', '', AF_HIDE_LIST|AF_LOG),
     'reason'=>            array('Reason',            'str',  '', 0, 200),
     'exceptions'=>        array('Exceptions',        'str',  '', 0, 200),
     'weekend_pricing'=>   array('Weekend pricing',   'bool', 0, 0),
     'checkin'=>           array('Checkin',           'date',   '', AF_HIDE_LIST, 50),
     'num_days'=>          array('# Days',            'int',  0, AF_READONLY, 3),
     'num_guests'=>        array('# Guests',          'int',   0, AF_READONLY, 3),
     'base_rate'=>         array('Base rate',         'money',   0, AF_HIDE_LIST|AF_READONLY),
     'base_rate_weekend'=> array('Base rate weekend', 'money',   0, AF_HIDE_LIST|AF_READONLY),
     'rate_type'=>         array('Rate Type',         'valSelect', 'day', AF_HIDE_LIST|AF_LOG, $rateTypeArray),
     'rate_class'=>        array('Rate Class',        'many2one', RATE_CLASS_STANDARD, AF_HIDE_LIST|AF_LOG, 'rate_class'),
     'rows'  =>            array('Rows',              'one2many', 0, AF_HIDE_LIST|AF_READONLY|AF_CASCADE_DELETE|AF_HIDE_ADD, 'price_breakdown_row', 'price_breakdown_id'),     
     ),
  
  "price_breakdown_row" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'price_breakdown_id'=>array('Parent ID',         'parent_id',  0, AF_OBLIGATORY, 'price_breakdown'),
     'weekend_rate'=>      array('Weekend rate',      'bool', 0, 0),
     'weekday'=>           array('Weekday',    			  'many2one', 0, AF_OBLIGATORY, 'weekday'),
     'date'=>              array('Date',              'date',  0, AF_OBLIGATORY),
     'currency' =>         array('Currency',          'valSelect', 'EUR', AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST, $currencyArray),
     'base_rate'=>         array('Base rate',         'money',   0, 0),
     'rate_change'=>       array('Rate change',       'percent_or_number', 0, 0),
     'rate'=>              array('Rate',              'money',   0, 0),
     'comment'=>           array('Comment',           'str',  '', 0, 200),
     ),  

  "supplier" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'modified'=>          array('Modified',          'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
     'language'=>          array('Language',    			'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),                               
     'delivery'=>          array('Delivery?',         'bool',   0, 1),

     'organization'=>      array('Company/Dept.',     'str',    '', AF_SEARCHABLE, 128),
     'salutation'=>        array('Salutation',        'str',    '', 0, 10),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 64),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 64),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
       
     ),
  
  "tax_category" => 
  array(
     'code'=>              array('Code',              'str',   0, AF_PRIMARY, 2),
     'active'=>            array('Active',            'bool',   1, 0),
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY, 50 ),
     'percentage'=>        array('Percentage',        'float',   0, 0, '2.2'),
     'country_code'=>      array('Country code',      'str',     0, 0, '2'),
     ),

  

  
  
  "agent" => 
  array(
     'id'=>                   array('ID',                'id',       0, AF_PRIMARY|AF_AUTOINCREMENT),
     'active'=>            array('Active',            'bool',   1, 0),
     'name'=>                 array('Agent Name',        'str',      '', AF_OBLIGATORY|AF_LOG, 30),

     'client_id'=>            array('Client',    		  	 'link',       ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),

     'invoice'=>              array('Pay by invoice?',   'bool', 0, AF_HIDE_LIST),

     'salutation'=>        array('Salutation',        'str',    '', 0, 10),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 64),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 64),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
     
//     'type'=>                 array('Agent Type',        'many2one', '', 0, 'agent_type'),
//     'languages'=>          array('Languages',         'json',     '', 0, 'language'),
     'custom_filters'=>       array('Custom Filters',    'text',   '', AF_HIDE|AF_HIDE_LIST|AF_HIDE_ADD, 0),
     'commission'=>           array('Standard Commisssion', 'commission', '', AF_READONLY|AF_HIDE_ADD|AF_LOG),
     'commission_charged'=>   array('Commission charged by agent?', 'bool', 0, AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST|AF_LOG),
     'balance_charged'=>      array('Balance charged by agent?', 'bool', 0, AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST|AF_LOG),
     'currency' =>            array('Currency',          'valSelect', 'EUR', AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST, $currencyArray),
     'booking_fee'=>          array('Booking Fee',       'money',   0, AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST),
     'send_feedback'=>        array('Send feedback request to guests from this agent?', 'bool', 0, AF_READONLY|AF_HIDE_ADD|AF_HIDE_LIST|AF_LOG),
     'logo' =>                array('Logo',                 'image',      '', AF_HIDE_LIST, 128),
  
     'partner_agent_id'=>    array('Partner Agent',         'link', 0, AF_HIDE, 'agent'),
     'contacts' =>           array('Contacts',              'one2many', 0, AF_HIDE_LIST, 'contact', 'agent_id'),
     'api_key'         =>    array('API KEY'    , 'str', '', AF_HIDE, 32),
     
  //         'booking_fee'=>        array('Standard Commisssion', 'commission',   0, AF_HIDE_LIST|AF_LOG),
  
  ),
  
  "airport" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY, 2),
     'location_id'=>       array('Location',          'link', 0, 0,'location'),
     'code'=>              array('Code',              'str',   '', AF_OBLIGATORY|AF_UNIQUE, 3),
     'name'=>              array('Name',              'str',   '', 1, 100 ),
     'city'=>              array('City',              'str',   '', 1, 100 ),
     'country'=>           array('Country',           'many2one', 0, AF_HIDE_LIST, 'countries'),
     'location_accuracy'=>  array('Accuracy',          'map',    0, 0, 1),
     'location_lat'         => array('Latitude'         , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_long'        => array('Longitude'        , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_bounds'      => array('Bounds'           , 'json', '', AF_HIDE_ADD|AF_HIDE_LIST, 200),
  ),
  

  "countries" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY, 2),
     'idd_prefix'=>        array('IDD Prefix',        'int',   0, AF_OBLIGATORY, 3),
     'name'=>              array('Name',              'str',   '', 1, 80 ),
  ),
  
  "currency" => 
  array(
     'code'=>              array('Code',              'str',   '', AF_PRIMARY, 3 ),
     'numeric_code'=>      array('Nunmeric Code',     'int',   '', 1, 3),
     'name'=>              array('Name',              'str',   '', 1, 50 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'accepted'=>          array('Accepted',          'bool',   0, 1),
     'minor_unit'=>        array('Minor Unit',        'int',   0, 0, 1),
  ),

  "user_level" => 
  array(
     'user_level'=>        array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'user_type'=>         array('Name',              'str',   '', 1, 12 ),
     'parent_object_type'=>array('Object type',       'str',   '', 0, 20 ),
  ),
  
  
  "user_role" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY, 2),
     'name'=>              array('Name',              'str',   '', 1, 20 ),
     'description'=>       array('Description',       'str',   '', 1, 255),
     'user_level'=>        array('ID',                'int',   0, 0, 2),
  ),
    
  "inv_type" => 
  array(
     'id'=>             array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>           array('Name',              'str',   '', 1, 20 ),
     'description'=>   array('Description',       'str',   '', 1, 200 ),
  ),
  
  
  "event_type" => 
  array(
     'id'=>             array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>           array('Name',              'str',   '', 1, 20 ),
     'description'=>    array('Description',       'str',   '', 1, 200 ),
     'property_types'=> array('Property types',    'msel',  '', AF_HIDE_LIST, 'property_type', array('active' => 1)),
  ),
  
  
  "lineitem_type" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>              array('Name',              'str',   '', 1, 20 ),
     'can_add'=>           array('User can add?',     'bool', 0, 0),
     'tax_category'=>      array('Tax Category',      'many2one',  0, AF_OBLIGATORY, 'tax_category', array('active' => 1)),
  ),
  
  
  "cc_type" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>              array('Name',              'str',   '', 1, 50 ),
  ),
  
  
  "tx_status" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>              array('Name',              'str',   '', 1, 50 ),
  ),
  
  "db_sequence" => 
  array(
     'seq_name'     => array('Table',              'str', '', AF_PRIMARY|AF_AUTOINCREMENT, 100),
     'nextid'       => array('Last ID',            'int', 11, AF_OBLIGATORY),
  ),

  "res_type" => 
  array(
     'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>          array('Name',              'str',   '', 1, 20 ),
     'description'=>   array('Description',       'str',   '', 1, 200 ),
  ),
  
  "rate_class" => 
  array(
     'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>          array('Name',              'str',   '', 1, 20 ),
     'description'=>   array('Description',       'str',   '', 1, 200 ),
  ),
  
  "address_type" => 
  array(
     'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>          array('Name',              'str',   '', 1, 20 ),
  ),
  
  "phone_type" => 
  array(
     'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>          array('Name',              'str',   '', 1, 20 ),
  ),
  
  "res_status" =>       
  array(
     'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>          array('Name',              'str',   '', 1, 20 ),
     'description'=>   array('Description',       'str',   '', 1, 200 ),
     'label'=>         array('Label class',       'str',   '', 0, 32 ),
     'color'=>         array('Color',             'color',   '000000', 0, 6),
  ),
                      
  "event_status" => 
  array(
     'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>          array('Name',              'str',   '', 1, 20 ),
     'description'=>   array('Description',       'str',   '', 1, 200 ),
     'color'=>         array('Color',             'color',   '000000', 1, 6),
  ),
  

  "enq_status" => 
  array(
     'id'=>              array('ID',                  'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>            array('Name',                'str',   '', 1, 20 ),
     'description' =>    array('Description',         'str',   '', 1, 200 ),
     'color'=>           array('Color',               'color',   '000000', 1, 6),
  ),
  
  
  "inv_status" => 
  array(
     'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 2),
     'name'=>          array('Name',              'str',   '', 1, 20 ),
     'description'=>   array('Description',       'str',   '', 1, 200 ),
     'color'=>         array('Color',             'color',   '000000', 1, 6),
  ),
  

  "country" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'grandparent_id'=>         array('Continent',         'many2one', 1, 0,'continent'),
     'location_id'=>       array('Location',          'link', 0, 0,'location'),
     'parent_id'=>         array('Region',            'many2one', 0, 0,'region'),
     'active'=>            array('Active',            'bool', 1, 0),
     'name'=>              array('Name',              'str',   '', AF_SEARCHABLE|AF_OBLIGATORY, 50 ),
     'alt_names'=>         array('Alt Names',         'text',   '', AF_SEARCHABLE),
     'priority'=>          array('Priority',          'float',  1, 0, '3.2'),
     'code'=>              array('Code',              'str',   '', 0, 3 ),
     'code2'=>             array('Code2',             'str',   '', 1, 2 ),                  
     'idd_code'=>          array('IDD Code (Regex)',  'str',   '', 0, 20),
     'mobile_regex'=>      array('Mobile Regex',      'str',   '', 0, 200),
     'tld'=>               array('TLD',               'str',   '', 0, 2),
     'location_id'      => array('Location'         , 'link', 0, AF_READONLY_EDIT, 'landmark'),
     'location_accuracy'=>  array('Accuracy',          'map',    0, 0, 1),
     'location_lat'         => array('Latitude'         , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_long'        => array('Longitude'        , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_bounds'      => array('Bounds'           , 'json', '', AF_HIDE_ADD|AF_HIDE_LIST, 200),
     'tax_description'=>    array('Tax Description',   'str', '', AF_HIDE_LIST, 50),
  
     'tax_rate_li_types'=>  array('Taxed @ Rate (Hi)',     'msel','', 0, 'lineitem_type'),
     'tax_rate2_li_types'=> array('Taxed @ Rate 2 (Lo)',   'msel','', 0, 'lineitem_type'),
     'tax_rate3_li_types'=> array('Taxed @ Rate 3 (None)', 'msel','', 0, 'lineitem_type'),
     'tax_rate_category' => array('Tax Rate Category (Hi)', 'str', 'H', 0, 10),
     'tax_rate2_category' => array('Tax Rate Category (Lo)', 'str', 'H', 0, 10),
     'tax_rate3_category' => array('Tax Rate Category (None)', 'str', 'H', 0, 10),
  ),
  

  "location" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'site_id'=>           array('Site',              'link',  ATT_DEFAULT_SITE_ID, AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'site'),
     'client_id'=>          array('Client',    		  	 'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
     'city_id'=>           array('City',              'link',   ATT_DEFAUL_NULL, AF_SEARCHABLE, 'geo_city'),
     'user_id'=>           array('User',              'link', 0, AF_OBLIGATORY, 'user'),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE),
     'active'=>            array('Active',            'bool', 1, 0),
     'priority'=>           array('Priority'          ,'int', 0, AF_HIDE, 8),

     'parent_type' =>      array('Parent Type',       'str', ATT_DEFAULT_NULL, 0, 50),
     'parent_id' =>        array('Parent ID',         'int', ATT_DEFAULT_NULL, 0),
     
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 50 ),
     'alt_names'=>         array('Alt Names',         'text',   '', AF_SEARCHABLE),
     'description'=>       array('Description',       'html',  '', AF_HIDE_LIST),                                 
     'slug'=>              array('Slug',              'slug',   '', AF_SEARCHABLE, 128 ),

     'address_street'=>    array('Street',            'str',  '', AF_HIDE_LIST, 50 ),
     'address_number'=>    array('Number',            'str',  '', AF_HIDE_LIST, 12 ),
     'address_city'=>      array('City',              'str',  '', AF_HIDE_LIST, 50 ),
     'address_postcode'=>  array('Postcode',          'str',  '', AF_HIDE_LIST, 50 ),
     'address_state'=>     array('State',             'str',  '', AF_HIDE_LIST, 50 ),
     'address_country'=>   array('Country',           'link', '', AF_HIDE_LIST, 'countries'),
     'address_formatted'=> array('Adddress',          'text', '', AF_HIDE_LIST),

     'location_lat'         => array('Latitude'         , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_long'        => array('Longitude'        , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_accuracy'=> array('Accuracy',          'int',  0, 0),
     'location_zoom'=>     array('Zoom',              'int',  0, 0),
     'location_bounds' =>  array('Bounds',            'json', '', AF_HIDE_ADD|AF_HIDE_LIST),
     'location_radius' =>  array('Radius',            'int', DEFAULT_RADIUS, AF_HIDE_ADD|AF_HIDE_LIST),
     
     'media'                => array('Media',             'media',     '', 0, $property_media_options),
     'media_json'           => array('Media (json)',      'json',     ''),
   
  ),

  /** new geo system from: http://www.geonames.org/export/*/
  "geo_city" => 
  array(
    'id'=>                 array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
    'fid' =>               array('Foreign ID',        'int',  0, AF_HIDE_LIST),
    'active'=>             array('Active',             'bool',   1, AF_HIDE_LIST),
    'created'=>            array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY),
    'modified'=>           array('Modified',          'date',  ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_READONLY),

    'name'=>               array('Name',              'str',   '', AF_SEARCHABLE|AF_OBLIGATORY, 50 ),
    'asciiname'=>          array('Name (ASCII)',      'slug',   '', AF_SEARCHABLE|AF_OBLIGATORY|AF_HIDE_LIST, 200 ),
    'alt_names'=>          array('Alt Names',         'text',   '', AF_SEARCHABLE|AF_HIDE_LIST),
    
     'location_lat'         => array('Latitude'         , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_long'        => array('Longitude'        , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
    'location_bounds'   => array('Bounds',            'json', '', AF_HIDE_ADD|AF_HIDE_LIST, 200),
    'location_zoom'=>     array('Zoom',              'int',  0, 0),
                                                       
    'country_code' =>      array('Country Code',      'str', '', 0, 2),
    'cc2' =>               array('Alt.Country Codes', 'str', '', AF_HIDE_LIST, 200),

    'feature_class' =>     array('Feature Class',     'str', '', 0, 1),
    'feature_code' =>      array('Feature code',      'str', '', 0, 10),
                                                      
    'admin1_code' =>       array('Admin1 code',       'str', '', AF_HIDE_LIST, 20),
    'admin2_code' =>       array('Admin2 code',       'str', '', AF_HIDE_LIST, 80),
    'admin3_code' =>       array('Admin3 code',       'str', '', AF_HIDE_LIST, 20),
    'admin4_code' =>       array('Admin4 code',       'str', '', AF_HIDE_LIST, 20),


    'population'=>         array('Population',        'int',  0, 0, 8),
    'elevation'=>          array('Elevation',         'int',  0, AF_HIDE_LIST, 8),
    'dem'=>                array('Dem',               'int',  0, AF_HIDE_LIST, 8),

    'timezone' =>          array('Timezone',          'str', '', 0, 40),

    'city_tax'=>           array('City Tax',           'surcharge', ATT_DEFAULT_NULL, AF_HIDE_LIST),         
    'city_tax_description'=>   array('City Tax Description', 'str', '', AF_HIDE_LIST, 50),
    'city_boat_tax'=>           array('City Boat Tax',           'surcharge', ATT_DEFAULT_NULL, AF_HIDE_LIST),         
    'city_boat_tax_description'=>   array('City Boat Tax Description', 'str', '', AF_HIDE_LIST, 50),
    
  ),
  
  
  "landmark_type" => 
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 20),
     'description'=>        array('Description',       'str',   '', 1, 200 ),
  ),
  
  
  "landmark" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'active'=>            array('Active',            'bool',   1, 0),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE),
     'type'=>              array('Type',              'many2one',  0, AF_SEARCHABLE, 'landmark_type'),
//     'dummy'=>              array('Search',           'location_picker',  0, AF_DUMMY),
  
     'city_id'=>           array('City',              'many2one',   1000, AF_SEARCHABLE, 'geo_city', array('active' => 1)),
     'site_id'=>           array('Site',              'many2one',  0, AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'site'),

     'parent_type' =>      array('Parent Type',       'str', ATT_DEFAULT_NULL, 0, 50),
     'parent_id' =>        array('Parent ID',         'int', ATT_DEFAULT_NULL, 0),
     
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 50 ),
  
     'address_street'=>    array('Street',            'str',   '', AF_HIDE_LIST, 50 ),
     'address_number'=>    array('Number',            'str',   '', AF_HIDE_LIST, 12 ),
     'address_state'=>     array('State',             'str',   '', AF_HIDE_LIST, 50 ),
     'address_postcode'=>  array('Postcode',          'str',   '', AF_HIDE_LIST, 50 ),
     'address_city'=>      array('City',              'str',   '', AF_HIDE_LIST, 50 ),
     'address_country'=>   array('Country',           'many2one', 0, AF_HIDE_LIST, 'countries'),
  
     'location_accuracy'    => array('Accuracy'    			, 'map',  0, 0),
     'location_lat'         => array('Latitude'         , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_long'        => array('Longitude'        , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_bounds'      => array('Bounds'           , 'json', '', AF_HIDE_ADD|AF_HIDE_LIST, 200),
  
     'descriptions'=>       array('Descriptions',       'one2many', 0, AF_HIDE_LIST|AF_CASCADE_DELETE|AF_READONLY, 'object_description', 'parent_id', array('parent_type' => 'landmark')), // last is filter
  ),
  

  "language" => 
  array(
  //         'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'code'=>              array('Code',              'str',   '', AF_PRIMARY, 3 ),
     'name'=>              array('Name',              'str',   '', 1, 50 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'allow_input'=>       array('Allow input?',      'bool',   1, 1),
     'locale'=>            array('Locale',            'str',   '', 1, 10 ),
  ),
  
  "client" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime',  ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
     'modified'=>          array('Modified',          'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY, 50 ),
     'shortname'=>         array('Short Name',        'str',   '', AF_OBLIGATORY, 10 ),
  
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),

     'cities'=>            array('Cities',            'msel',     '', AF_HIDE_LIST, 'geo_city', array('active' => 1), 'name'),
     'types'=>             array('Property Types',    'msel',     '', AF_HIDE_LIST, 'property_type'),
     'language'=>           array('Language',    			'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),                               
     'languages'=>         array('Languages',         'json',     '', AF_HIDE_LIST, 'language'),
     'mollie_profile'=> array('Mollie Profile', 'str',   '', AF_HIDE_LIST|AF_HIDE_ADD, 8),
  ),
  
  
  "legal_entity" => 
  array(
     'id'=>                  array('ID',                 'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'name'=>                array('Name',               'str',   '', 1, 100 ),
                             
     'client_id'=>           array('Client',    			   'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
  
     'bank_name'=>           array('Bank name',          'str',   '', AF_HIDE_LIST, 100),
     'account_name'=>        array('Account name',       'str',   '', AF_HIDE_LIST, 100),
     'account_number'=>      array('Account number',     'str',   '', AF_HIDE_LIST, 200),
     'tax_id'=>              array('Legal/tax id',       'str',   '', AF_HIDE_LIST, 100),
     'commerce_id'=>         array('Business association id',  'str',   '', AF_HIDE_LIST, 100 ),
     'paypal_id_business' => array('Paypal ID Business', 'str',  '', AF_HIDE_LIST, 80),
     'paypal_id_private'  => array('Paypal ID Private',  'str',  '', AF_HIDE_LIST, 80),
  
     'logo'   =>           array('Logo',                 'image',      '', AF_HIDE_LIST, 128),
     'logo_small' =>       array('Logo - Small',         'image',   '', AF_HIDE_LIST, 128),
     'logo_print' =>       array('Logo - Print',         'image',   '', AF_HIDE_LIST, 128),

     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
     
     'invoice_prefix'=>    array('Invoice Prefix',        'str',   '', 0, 10),
     'invoice_nextid'=>    array('Invoice Next ID',       'int',   1000, 0, 6),
     'invoice_id_required'=>  array('Invoice ID Required?',    'bool',   0, 0, 1),
  
     'reeleezee_username'=>    array('Reeleezee Username',  'str',   '', 0, 50),
     'reeleezee_password'=>    array('Reeleezee Password',  'str',   '', 0, 50),
     
     'mollie_id'=>      array('Mollie ID', 'str',   '', AF_HIDE_LIST|AF_HIDE_ADD, 8),
     'mollie_profile'=> array('Mollie Profile', 'str',   '', AF_HIDE_LIST|AF_HIDE_ADD, 8),
  
     'charge_tax' =>               array('Charge V.A.T?'       ,'bool', 0, AF_LOG, 0),
     'tax_included' =>             array('Tax included?'       ,'bool', 0, AF_LOG, 0),
  
     ),
  
  
  // per language defaults for site      
  "site_content" => 
  array(
     'id'=>                array('ID',                  'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'site_id'=>           array('Site',                'many2one',  0, AF_SEARCHABLE|AF_HIDE, 'site'),
  
     'created'=>           array('Created',             'datetime',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',           'timestamp',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
  
     'active'=>            array('Active?',             'bool',     1, 0),
  
     'language'=>           array('Language',    			  'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),                               
     'title'=>             array('Screen Title',        'str',  '', AF_OBLIGATORY, 255 ),
     'subtitle'=>          array('Screen Subtitle',     'str',  '', AF_HIDE_LIST, 255),
  
     'se_title'=>          array('Search Engine Title',       'str',  '__pagename __pathvar __sitename', AF_HIDE_LIST, 255),
     'se_description'=>    array('Search Engine Description', 'text', '__pagename __pathvar __sitename', AF_HIDE_LIST),
     'se_keywords'=>       array('Search Engine Keywords',    'text', '', AF_HIDE_LIST),
  
     //'site_header'=>           array('Header (HTML)',            'html', '', AF_HIDE_LIST),
     //'site_footer'=>           array('Footer (HTML)',            'html', '', AF_HIDE_LIST),
     //'site_extra'=>            array('Extra  (HTML)',            'html', '', AF_HIDE_LIST),
                                                                                         
     //'mail_header'=>           array('Header (HTML)',            'html', '', AF_HIDE_LIST),
     //'mail_footer'=>           array('Footer (HTML)',            'html', '', AF_HIDE_LIST),
     
     //'alt_site_id'=>     array('Alternate Site for this language', 'many2one', ATT_DEFAULT_NULL, 0, 'site'),
     'machine_translated'=>    array('Machine Translated on',    'str',  '', AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 20),
              
     
  ),

  "minisite" => 
  array(
     'id'=>                 array('ID',                 'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>            array('Created',            'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>          array('Timestamp',          'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
     'active'  =>           array('Active?',            'bool',     1,  AF_HIDE_ADD, 0),
     'site_id'=>            array('Site',               'link',  0, AF_SEARCHABLE|AF_HIDE, 'site'),
     'client_id'=>          array('Client',             'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
     'user_id' =>           array('User ID',            'link', ATT_DEFAULT_USER_ID, 0, 'user'),
     'admin_users' =>       array('Admin Users',        'json', '', 0),
     'type'=>               array('Type',               'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 32 ),
     'name'=>               array('Name',               'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 128 ),
     'sub_title'=>          array('Sub Title',          'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 128 ),
     'description'=>        array('Description',          'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 128 ),
     'url'=>                array('Url',                'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 64 ),
     'domain'=>             array('Domain',             'str',  '', AF_SEARCHABLE, 128 ),
     'email_address'=>      array('Email',              'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'languages'=>          array('Languages',          'json', 'en', AF_OBLIGATORY, 'language', array('active' => 1)),
     'default_language'=>   array('Language',               'valSelect', 'en', AF_OBLIGATORY|AF_SEARCHABLE, $langArray),
     'data' =>              array('Data',               'json', '', 0),
     'template' =>          array('Template',           'str', '', 128),
     'logo'   =>            array('Logo',               'image',   '', AF_HIDE_LIST, 128),
     'logo_small' =>        array('Logo - Small',       'image',   '', AF_HIDE_LIST, 128),

     'text_color'=>         array('Text Color',         'color',    '', AF_HIDE_LIST, 6),
     'bg_color'=>           array('BG Color',           'color',    '', AF_HIDE_LIST, 6),
     'theme_color1'=>       array('Theme Color 1',      'color',    '', AF_HIDE_LIST, 6),
     'theme_color2'=>       array('Theme Color 2',      'color',    '', AF_HIDE_LIST, 6),
     'font'=>               array('Font',               'str',  '', 0, 64 ),
     'css'=>                array('Css',                'str',  '', 0, 64 ),
     'location_id'=>        array('Location',           'link',   0, AF_HIDE_ADD, 'location'),
     'footer'=>             array('Footer',             'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 128 ),
     'show_title'  =>       array('Show title',         'bool',     1,  AF_HIDE_ADD, 0),
     'show_subtitle'  =>    array('Show subtitle',      'bool',     1,  AF_HIDE_ADD, 0),
     'show_timing'  =>      array('Show timing',        'bool',     1,  AF_HIDE_ADD, 0),
     'show_social'  =>      array('Show social',        'bool',     1,  AF_HIDE_ADD, 0),
   ),

   "minisite_page_type" => 
   array(
      'id'=>            array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 4),
      'name'=>          array('Name',              'str',   '', 1, 32 ),
   ),

    "minisite_page" => 
   array(
      'id'=>                 array('ID',               'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
      'created'=>            array('Created',          'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
      'timestamp'=>          array('Timestamp',        'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
      'active'  =>           array('Active?',          'bool',     1,  AF_HIDE_ADD, 0),
      'minisite_id'=>        array('Minisite',         'link',  0, AF_SEARCHABLE|AF_HIDE, 'minisite'),
      'site_id'=>            array('Site',             'link',  0, AF_SEARCHABLE|AF_HIDE, 'site'),
      'client_id'=>          array('Client',           'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
      'user_id' =>           array('User ID',          'link', ATT_DEFAULT_USER_ID, 0, 'user'),
      'type' =>              array('Type',             'many2one', 0, AF_LOG|AF_SEARCHABLE , 'minisite_page_type'),
      'menu_title'=>         array('Menu Title',  'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 128 ),
      'page_title'=>         array('Page Title',  'str',  '', AF_SEARCHABLE, 128 ),
      'slug'=>               array('Slug',             'str',   '', 0, 128 ),
      'search'=>             array('Search',           'json',   ''),
      'row_order'=>          array('Row Order',        'int',   '', 0, 11 ),
   ),

  "site" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'parent_site_id'=>    array('Parent site',       'many2one', 0, 0, 'site'),
     'source_site_id'=>    array('Source site',       'many2one', 0, 0, 'site'),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
     'active'  =>           array('Active?',          'bool',     1,  AF_HIDE_ADD, 0),
  
     'legal_entity_id'=>   array('Legal Entity',      'many2one', 0, AF_OBLIGATORY, 'legal_entity'),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
  
     'languages'=>            array('Languages',    	'json', 'en', AF_OBLIGATORY, 'language', array('active' => 1)),
     'default_language'=>     array('Language',    		'valSelect', 'en', AF_OBLIGATORY|AF_SEARCHABLE, $langArray),
     
     'template' =>         array('Template',          'str', '', 0, 128),
     //'template_id'=>       array('Template ID',       'many2one', 1014, AF_HIDE_LIST, 'site_template', array('skel_directory' => array('!=', ''))), // baris
  
     'theme_color'=>        array('Theme Color',        'color',    '474747', AF_HIDE_LIST, 6),
     
     'built'  =>            array('Built?',             'bool', 0, AF_HIDE_ADD, 0),
     'live'   =>            array('Live?',              'bool', 0,  AF_HIDE_ADD, 0),
     'secure'   =>          array('Secure?',            'bool', 0,  AF_HIDE_ADD, 0),
          
     'name'=>              array('Name / Title',      'str',   '', 1, 100 ),
     'shortname'=>         array('Shortname',         'str',   '', AF_OBLIGATORY|AF_UNIQUE, 50),
     'library_prefix'=>    array('Libarary Prefix',   'str',   '', AF_HIDE_ADD, 20),
     'domain'=>            array('Domain (w/o www.)', 'str',   '', 1, 100 ),
     'url'=>               array('URL',               'url',   '', AF_HIDE_LIST|AF_HIDE_ADD, 100 ),
     'test_url'=>          array('URL - test server', 'url',   '', AF_HIDE_LIST|AF_HIDE_ADD, 100 ),
  
     'app_server'=>        array('App. Server',       'many2one',   SYSTEM_SERVER_ID, AF_OBLIGATORY|AF_HIDE_ADD|AF_READONLY, 'server'),
     'dev_server'=>        array('Dev. Server',       'many2one',   DEV_SERVER_ID, AF_OBLIGATORY|AF_HIDE_ADD, 'server'),
     'live_server'=>       array('Live Server',       'many2one',   '', AF_OBLIGATORY, 'server', array('active' => 1, 'production' => 1)),
  
     'directory'=>         array('Directory',         'str',   '', AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 255),
     'test_directory'=>    array('Test Directory',    'str',   '', AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 255),
  
     'email_forward'=>      array('Forward email to:', 'email',  '' , AF_OBLIGATORY|AF_HIDE_LIST, 100),
      
     'gallery_site'=>         array('Gallery site?',       'bool', 0, 0),
     'property_site'=>         array('Property site?',       'bool', 0, 0),
     'payment_site'=>         array('Payment site?',       'bool', 0, 0),
     'property_type'=>         array('Property type',       'str', '', 32),
     'property_sort'=>         array('Property sort',       'str', '', 64),
     'golf_site'=>             array('Golf site?',       'bool', 0, 0),
     'pet_site'=>             array('Pet site?',       'bool', 0, 0),

     'require_user_login'=>   array('Require user login?',   'bool', 0, 0),         
     'allow_user_login'=>     array('Allow user login?',    'bool', 0, 0),         
     'allow_user_register'=>  array('Allow user register?',   'bool', 0, 0),         
     'logo'   =>              array('Logo',                 'image',   '', AF_HIDE_LIST, 128),
     'logo_small' =>          array('Logo - Small',         'image',   '', AF_HIDE_LIST, 128),
     'logo_print' =>          array('Logo - Print',         'image',   '', AF_HIDE_LIST, 128),
     'default_city'=>         array('Default City',         'many2one',  0, AF_HIDE_LIST, 'geo_city'),
     'show_cities'=>          array('Show cities',          'msel', '', AF_HIDE_LIST, 'geo_city'),
     'show_types'=>           array('Show types',           'msel', '', AF_HIDE_LIST, 'property_type'),
     'default_bounds'=>       array('Default Bounds',       'json',  0, AF_HIDE_LIST),

     'property_filters' =>    array('Property Filters',     'json', '', AF_HIDE_LIST|AF_HIDE_ADD),
     'partner_filters'=>      array('Partner Filters',      'json', '', AF_HIDE_LIST|AF_HIDE_ADD),
     'cloudinary'=>      array('Cloudinary Config',      'json', '', AF_HIDE_LIST|AF_HIDE_ADD),
     'charge_service_fee' =>  array('Charge Service Fee',   'bool', 0, 0),
     
     'pages'  =>            array("Pages",    'one2many', 0, AF_HIDE_LIST|AF_READONLY|AF_CASCADE_DELETE|AF_HIDE_ADD, 'page_content', 'site_id'),
     'content'  =>          array("Content",  'one2many', 0, AF_HIDE_LIST|AF_READONLY|AF_CASCADE_DELETE|AF_HIDE_ADD, 'site_content', 'site_id'),

     'organization'=>      array('Company/Dept.',     'str',    '', AF_SEARCHABLE, 128),
     'salutation'=>        array('Salutation',        'str',    '', 0, 10),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 64),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 64),
     'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 128),
     'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 128),
     'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 64),
     'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 64),
     'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 64),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
     'phone'=>             array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
     
  ),
  
  
  "page_type" => 
  array(
     'id'=>                array('ID',                 'id',   0, AF_PRIMARY|AF_AUTOINCREMENT),
     'name'=>              array('Name',               'str',   '', 1, 20),
     'description'=>       array('Description',        'str',   '', 0, 80),

     'template'=>          array('Default template',   'str', '', 0, 128),
     'controller'=>        array('Default controller', 'str', '', 0, 128),
     'user_level' =>       array('User level',         'int',  0, 0),  
     'show_menu'=>         array('Show in menu?',      'bool', 0, 0),
     
     'path_var'  =>        array('Path var',           'str', '', 0, 40),
     'object_type' =>      array('Obj type',           'str', '', 0, 50),
     'cms_add'=>           array('CMS auto-add?',      'flags',   0, 0, $__CMS_ADD_FLAGS),

     /** below are old */
     /**     
     'object_subtype' =>   array('Obj subtype',         'int', 0, AF_HIDE),
     'object_id' =>        array('Obj id',              'int', 0, AF_HIDE),
     
     'cms_show'=>          array('CMS show?',         'bool',   1, 0),
     'property'=>          array('Property',          'bool',   0, 0),

     'localized'=>         array('Localized?',          'bool',  1, 0),
     'secure'=>            array('Secure?',             'bool',  0, 0),
     'static'=>            array('Static?',             'bool',  0, 0),
     'relative'=>          array('Relative to city',    'bool',  0, 0),
     'admin'=>             array('Admin page?',         'bool',  0, 0),
     'dynamic'=>           array('Dynamic (no spider)?','bool',  0, 0),
     'external'=>          array('External?',           'bool',  0, 0),

     'show_footer'=>       array('Show link in footer?', 'bool',   0, 0),
     'show_title'=>        array('Show page title?',     'bool',   1, 0),
     'template_id'=>       array('Default template?',    'many2one', 0, 0, 'site_template'),
     */
  ),
  
  
  "site_page" => 
  array(
     'id'=>                array('ID',                  'id',       0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'created'=>           array('Created',             'datetime',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',           'timestamp',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
     'active'=>            array('Active?',             'bool',     1, 0),
     'site_id'=>           array('Site',                'many2one',  0, AF_OBLIGATORY|AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'site'),
     //'template_id'=>       array('Template',            'many2one', 0, AF_HIDE_LIST, 'site_template'),
     'parent_id' =>        array('Parent page',         'many2one', '', AF_SEARCHABLE|AF_READONLY_ADD, 'site_page'),
     'user_id' =>          array('Author',              'link', ATT_DEFAULT_USER_ID, 0, 'user'),

     
     'name'=>              array('Name',                'str',  '', AF_OBLIGATORY, 50 ),
     'template'=>          array('Template file',        'str',  '', 0, 128 ),
     'controller'=>        array('Controller',           'str',  '', 0, 128 ),

     'page_type' =>        array('Page type',           'many2one', '', AF_SEARCHABLE, 'page_type'),
     'object_type' =>      array('Obj type',            'str', '', AF_HIDE, 50),
     'object_subtype' =>   array('Obj subtype',         'int', 0, AF_HIDE),
     'object_id' =>        array('Obj id',              'int', 0, AF_HIDE),
     'path_var'  =>        array('Path var',            'str', '', AF_HIDE, 40),
     'subviews'  =>        array('Sub views',           'json', '', AF_HIDE),
  
     'show_menu'=>         array('Show in menu?',       'bool', 1, AF_HIDE_LIST, 0),
     'user_level'=>        array('User Level',          'many2one',  0, AF_SEARCHABLE, 'user_level'),
     'rank'=>              array('Rank',                'int',  0, 0, 3),

     'is_folder' =>        array('Is folder?',          'bool', 0, 0),
     'local'=>             array('Local template?',     'bool', 0, 0),
 
  ),
  
  "page_content" =>  
  array(
     'id'=>                array('ID',                  'id',       0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'site_id'=>           array('Site',                'many2one',  0, AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'site'),
     'page_id'=>           array('Parent Page',         'many2one',  0, AF_OBLIGATORY|AF_HIDE_EDIT, 'site_page'),
     'created'=>           array('Created',             'datetime',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',           'timestamp',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
     'published'=>         array('Published?',          'bool',     0, AF_READONLY),
     'language'=>          array('Language',    			  'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),
     'path'=>              array('Path',                'str',  '', AF_SEARCHABLE, 128 ),
     'menu_title'=>        array('Menu title',          'str',  '', AF_HIDE_LIST, 100 ),

     //'title'=>             array('Screen Title',        'str',  '', AF_SEARCHABLE, 100 ),
     //'subtitle'=>          array('Screen Subtitle',     'str',  '', AF_HIDE_LIST, 100 ),
     //'content'=>           array('Content (HTML)',            'html', '', AF_HIDE_LIST, 30),

     'se_title'=>          array('Search Engine Title',       'str',  '', AF_HIDE_LIST, 100 ),
     'se_keywords'=>       array('Search Engine Keywords',    'text', '', AF_HIDE_LIST, 255 ),
     'se_description'=>    array('Search Engine Description', 'text', '', AF_HIDE_LIST),
     

   ),  
  // generic 
  
  
  // language specific
  "page_element" =>  
  array(
     'id'=>                array('ID',                  'id',       0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'active'=>            array('Active',              'bool',   1, 0),
     'site_id'=>           array('Site',                'link',  0, AF_OBLIGATORY|AF_SEARCHABLE|AF_HIDE_EDIT|AF_HIDE_ADD, 'site'),
     'page_id'=>           array('Parent Page',         'link',  0, AF_HIDE_EDIT, 'site_page'),
     'path'=>              array('Path',                'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 128 ),
     'created'=>           array('Created',             'datetime',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'rank'=>              array('Rank',                'int', 1, AF_HIDE_ADD, 3),
     'timestamp'=>         array('Timestamp',           'timestamp',     ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
     'language'=>          array('Language',    			  'many2one', 'en', AF_OBLIGATORY|AF_SEARCHABLE, 'language'),                               
     'name'=>              array('Name',                'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 100 ),
     'type'=>              array('Type',                'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 50 ),
     'content'=>           array('Content',             'text',  '', 0),
     'media' =>            array('Media',               'media', '', 0, $single_media_options),
     'resize' =>           array('Resize Media?',       'bool',  0, 0),
     'options'=>           array('Options',             'text',  '', 0),
   ),


  "font" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'name'=>              array('Name',              'str',   '', 1, 20 ),
     'css'=>               array('CSS',               'str',   '', 1, 255 ),
  ),
  

  "message" => 
  array(
     'id'=>                array('ID',                'id',       0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST, 14 ),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST, 14 ),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY_EDIT, 'client'),
     'site_id'=>           array('Site',              'many2one', ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'parent_id'=>         array('Parent Id',         'parent_id',   0, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD, 11),
     'parent_type'=>       array('Parent Type',       'parent_type', '', AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD),
     'remote_ip'=>         array('IP',                'ip',      ATT_DEFAULT_CLIENT_IP, AF_HIDE_ADD|AF_READONLY, 20 ),
      //'city_id'=>           array('City',              'many2one', ATT_DEFAULT_CITY_ID, AF_SEARCHABLE, 'geo_city', array('active' => 1), 'name'),
     'read'=>              array('Read',              'bool',     0, AF_HIDE_LIST),
     'replied'=>           array('Replied',           'bool',     0, AF_HIDE_LIST),
     'name'=>              array('Name',              'str',      '', AF_SEARCHABLE, 60 ),
     'title'=>             array('Title',             'str',      '', 0, 50 ),
     'company'=>           array('Company',           'str',      '', AF_SEARCHABLE, 60 ),
     'country'=>           array('Country',           'many2one', ATT_DEFAULT_CLIENT_CC, AF_HIDE_LIST, 'countries'),
     'language'=>          array('Language',          'many2one', ATT_DEFAULT_LANGUAGE, 0, 'language'),
     'subject'=>           array('Subject',           'str',      '', AF_SEARCHABLE, 60 ),
     'phone'=>             array('Phone',             'str',    '', AF_HIDE_LIST, 20),

     /** old */
     'message'=>           array('Message',           'text',     '', 0, 0),     
     'email_address'=>     array('Email',             'email',    '', AF_HIDE_LIST, 60),
     
     /** old */
     'main_message'=>      array('Main Message',      'text',     '', 0, 0),
     'personal_message'=>  array('Personal Message',  'text',     '', 0, 0),
     'sender'=>            array('Sender',            'str',      '', AF_HIDE_LIST, 128),
     'recipient'=>         array('Recipient',         'str',      '', AF_HIDE_LIST, 128),
  ),                                                            
  
  
  "changelog" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT|AF_HIDE, 11),
     'timestamp'=>         array('Timestamp',         'timestamp',  ATT_DEFAULT_NOW, AF_SEARCHABLE, 14),
     //'obj_type'=>          array('Obj Type',          'str',   '', AF_HIDE, 20),
     //'parent_id'=>         array('ObjId',             'parent_id',  1, AF_SEARCHABLE|AF_HIDE, 'baseclass'),
     'obj_type'=>          array('Obj Type',          'str',   '', 0, 20),
     'parent_id'=>         array('ObjId',             'parent_id',  1, 0, 'baseclass'),
     'user_level'=>        array('User Level',        'many2one',  ATT_DEFAULT_USER_LEVEL, AF_SEARCHABLE, 'user_level'),
     'user_id'=>           array('User',              'many2one',  ATT_DEFAULT_USER_ID, AF_SEARCHABLE, 'user'),
     'field'=>             array('Field',             'str',   '', AF_SEARCHABLE, 50 ),
     'from_value'=>        array('Old value',         'str',   '', AF_SEARCHABLE, 255 ),
     'to_value'=>          array('New value',         'str',   '', AF_SEARCHABLE, 255 ),
     'action'=>            array('Action',            'str',   '', AF_SEARCHABLE, 255 ),
  ),
  
  
  "news" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'created'=>           array('Created',           'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST, 6 ),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'site_id'=>           array('Site',              'many2one',   ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'user_level'=>        array('User Level',        'many2one',  0, AF_SEARCHABLE, 'user_level'),
     'title'=>             array('Title',             'str',   '', AF_OBLIGATORY, 255),
     'content'=>           array('Content',           'html',  '', AF_HIDE_LIST|AF_OBLIGATORY),
     'language'=>          array('Language',    			'many2one', ATT_DEFAULT_LANGUAGE, AF_OBLIGATORY|AF_SEARCHABLE, 'language'),
     'image'=>             array('Image',             'image',  '', AF_HIDE_LIST, 128),
  ),
  
  
  
  "article" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST, 6 ),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE|AF_READONLY, 14 ),
     'post_date'=>         array('Article Date',      'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST, 6 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'published'=>         array('Published',         'bool',   0, 0),
     'type' =>             array('Article.Type',      'many2one', 0, AF_SEARCHABLE, 'article_type'),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'user_id'=>           array('User',              'link',  ATT_DEFAULT_USER_ID, AF_SEARCHABLE, 'user'),
     'user_level'=>        array('User Level',        'many2one',  0, AF_SEARCHABLE, 'user_level'),
     'salutation'=>        array('Salutation',        'valSelect', '', 0, $salutationArray),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 50),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 50),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'title'=>             array('Title',             'text',   '', 0, 255),
     'subtitle'=>          array('Subtitle',          'str',   '', 0, 255),
     'content'=>           array('Content',           'html',  '', AF_HIDE_LIST|AF_OBLIGATORY),
     'language'=>          array('Language',    			'many2one', ATT_DEFAULT_LANGUAGE, AF_OBLIGATORY|AF_SEARCHABLE, 'language'),
     'link'=>              array('URL',               'str',    '', AF_HIDE_ADD|AF_HIDE_LIST, 100),
     'image'=>             array('Image',             'image',  '', AF_HIDE_LIST, 128),
     'media'            => array('Media',             'media',     '', 0, $gallery_media_options),
     'views'=>             array('Views',             'int',    0, AF_HIDE_ADD|AF_READONLY|AF_HIDE_LIST),
     
     //array('type' => IMAGE_TYPE_PROFILE, 'resize' => true, 'minimum' => 640)),
  
  ),

  "comment" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'created'=>           array('Created',           'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST, 6 ),
     'timestamp'=>         array('Timestamp',         'timestamp',   ATT_DEFAULT_NOW, AF_HIDE|AF_READONLY, 14 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'article_id'=>        array('Article',           'link',   ATT_DEFAULT_ARTICLE_ID, AF_READONLY, 'article', $article_filters),
     'comment_id'=>        array('Comment',           'link',   ATT_DEFAULT_COMMENT_ID, AF_READONLY, 'comment', $comment_filters),
     'user_id'=>           array('User',              'link',  ATT_DEFAULT_USER_ID, AF_SEARCHABLE, 'user'),
     'language'=>          array('Language',    			'many2one', ATT_DEFAULT_LANGUAGE, AF_OBLIGATORY|AF_SEARCHABLE, 'language'),
//     'salutation'=>        array('Salutation',        'valSelect', '', 0, $salutationArray),
     'first_name'=>        array('Name',              'str',    '', AF_OBLIGATORY|AF_SEARCHABLE, 50),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 50),
     'email_address'=>     array('Email',             'email',  '', AF_OBLIGATORY|AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'title'=>             array('Title',             'str',   '', 0, 255),
     'content'=>           array('Content',           'html',  '', AF_OBLIGATORY|AF_HIDE_LIST|AF_OBLIGATORY),
  ),                                                                  
  
  "newsletter" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 8),
     'created'=>           array('Created',           'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST, 6 ),
     'active'=>            array('Active',            'bool',   1, 0),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'remote_ip'=>         array('IP',                'ip',       ATT_DEFAULT_CLIENT_IP, AF_HIDE_LIST|AF_SEARCHABLE|AF_READONLY, 20),
     'remote_cc'=>         array('CC',                'many2one', ATT_DEFAULT_CLIENT_CC, AF_HIDE_LIST, 'countries'),
     'user_id'=>           array('User',              'link',  ATT_DEFAULT_USER_ID, AF_SEARCHABLE, 'user'),
     'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 50),
     'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 50),
     'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 50),
     'language'=>          array('Language',    			'many2one', ATT_DEFAULT_LANGUAGE, AF_OBLIGATORY|AF_SEARCHABLE, 'language'),
  ),

  

  "mail_queue" => 
  array(
     'id'=>                array('ID',            'id',     0, AF_PRIMARY|AF_HIDE_LIST|AF_AUTOINCREMENT, 20),
     'guid'=>              array('GUID',          'uuid',        ATT_DEFAULT_UUID, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD, 24),
     'parent_id'=>         array('Parent Id',     'parent_id',   ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD, 11),
     'parent_type'=>       array('Parent Type',   'parent_type', ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD),
     'client_id'=>         array('Client',    		'link',       ATT_DEFAULT_CLIENT_ID, AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>           array('Site',          'link',       ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'user_id' =>          array('User ID',       'link', ATT_DEFAULT_USER_ID, 0, 'user'),     
     'sender_id'=>         array('Sender ID',    	'link',       0, AF_READONLY, 'user'),
     'receiver_id'=>       array('Receiver ID',   'link',       0, AF_READONLY, 'user'),
     'receiver_role'=>     array('Receiver Role', 'str',        '', AF_READONLY, 32),
     'receiver_name'=>     array('Receiver Name', 'str',        '', AF_READONLY, 128),
     'campaign_id' =>      array('Campaign',      'link',    0, 0, 'mail_campaign'),

     'create_time'=>       array('Created',       'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD, 21 ),
     'time_to_send'=>      array('Time to Send',  'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST, 21 ),
     'sent_time'=>         array('Sent',          'datetime',   ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_ADD|AF_READONLY_EDIT, 21 ),
     // 'id_user'=>           array('User',          'link',   1, AF_SEARCHABLE|AF_HIDE_ADD|AF_READONLY_EDIT, 'user'),
     'ip'=>                array('IP',            'ip',    ATT_DEFAULT_CLIENT_IP, AF_HIDE_ADD|AF_READONLY, 20 ),
     'sender'=>            array('Sender',        'str',    '', AF_OBLIGATORY, 50 ),
     'recipient'=>         array('Recipient',     'text',   '', AF_SEARCHABLE|AF_OBLIGATORY),
     'cc'=>                array('CC',            'text',   '', AF_SEARCHABLE),
     'bcc'=>               array('BCC',           'text',   '', AF_SEARCHABLE),
     'replyto'=>           array('Reply-to',      'text',   '', AF_SEARCHABLE),
     'headers'=>           array('Headers',       'text',   '', AF_HIDE_LIST),

     'subject'=>           array('Subject',       'str',  '', AF_OBLIGATORY, 150 ),
     'body'=>              array('Body',          'mail_body',   '', AF_HIDE_LIST|AF_OBLIGATORY),
     'attachment'=>        array('Attachment',    'text',   '', AF_HIDE_LIST),
     'try_sent'=>          array('Try sent',      'int',  0, AF_READONLY|AF_HIDE_ADD|AF_READONLY_EDIT, 6 ),
     'read_count' =>       array('Read Count',    'int',  0, AF_READONLY|AF_HIDE_ADD, 3),
  ),
  
  "mail_campaign" => 
  array(
     'id'=>                array('ID',            'id',     0,  AF_PRIMARY|AF_HIDE_LIST|AF_AUTOINCREMENT, 20),
     'guid'=>              array('GUID',          'uuid',        ATT_DEFAULT_UUID, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD, 24),
     'parent_id'=>         array('Parent Id',     'parent_id',  ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD, 11),
     'parent_type'=>       array('Parent Type',   'parent_type',ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD),
     'client_id'=>         array('Client',    		'link',       ATT_DEFAULT_CLIENT_ID, AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>           array('Site',          'link',       ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'user_id' =>          array('User ID',       'link',       ATT_DEFAULT_USER_ID, 0, 'user'),
     'sender_id'=>         array('Sender ID',    	'link',       0, AF_READONLY, 'user'),

     'name'=>              array('Name',          'str',    '', AF_SEARCHABLE|AF_OBLIGATORY, 32),
     'description'=>       array('Description',   'text',     '', AF_HIDE_LIST),
     
     'created'=>           array('Created',       'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD, 21 ),
     'time_to_send'=>      array('Time to Send',  'datetime',   ATT_DEFAULT_NULL, AF_HIDE_LIST, 21 ),
     'sent_time'=>         array('Sent',          'datetime',   ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_ADD|AF_READONLY_EDIT, 21 ),

     'receiver_count'=>    array('Receiver Count', 'int',   0, AF_READONLY|AF_HIDE_ADD, 4 ),
     'sent_count' =>       array('Sent Count',     'int',   0, AF_READONLY|AF_HIDE_ADD, 4 ),

     'receiver_roles'=>    array('Receiver Roles', 'text',   '', AF_READONLY),
     'active_since'=>      array('Active Since',   'str',   '', 0, 21),
     
     'filters'=>           array('Filters',       'json',   '', AF_HIDE_LIST),

     'title'=>             array('Title',          'str',    '', AF_SEARCHABLE, 255),
     'sender'=>            array('Sender',        'str',    '', AF_OBLIGATORY, 50 ),
     'recipient'=>         array('Preview Recipient', 'text',   '', AF_SEARCHABLE|AF_OBLIGATORY),
     'recipients'=>        array('Recipients',    'json',   '', AF_HIDE_LIST),
     'failed'=>            array('Failed',        'json',   '', AF_HIDE_LIST),
     'guids'=>             array('GUIDS',         'json',   '', AF_HIDE_LIST),
     'cc'=>                array('CC',            'text',   '', AF_SEARCHABLE),
     'bcc'=>               array('BCC',           'text',   '', AF_SEARCHABLE),
     'replyto'=>           array('Reply-to',      'text',   '', AF_SEARCHABLE),
     'headers'=>           array('Headers',       'text',   '', AF_HIDE_LIST),
     
     'subject'=>           array('Subject',       'str',    '', AF_OBLIGATORY, 255),
     'main_message'=>      array('Message',       'text',   '', AF_HIDE_LIST|AF_OBLIGATORY),
     'template' =>         array('Template',      'str',    '', AF_HIDE_LIST|AF_OBLIGATORY, 255),
     'body'=>              array('Body',          'text',   '', AF_HIDE_LIST),
     'attachment'=>        array('Attachment',    'text',   '', AF_HIDE_LIST),
  ),
     
  "sms_queue" => 
  array(
     'id'=>                array('ID',            'id',       0, AF_PRIMARY|AF_HIDE_LIST|AF_AUTOINCREMENT, 20),
     'guid'=>              array('GUID',          'uuid',        ATT_DEFAULT_UUID, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD, 24),
     'parent_id'=>         array('Parent Id',     'parent_id',   ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_ADD, 11),
     'parent_type'=>       array('Parent Type',   'parent_type', ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_ADD),
     'site_id'=>           array('Site',          'link',     ATT_DEFAULT_SITE_ID, AF_READONLY, 'site'),
     'client_id'=>         array('Client',    	  'link',     ATT_DEFAULT_CLIENT_ID, AF_READONLY|AF_HIDE, 'client'),
     'create_time'=>       array('Created',       'datetime', ATT_DEFAULT_NOW, AF_READONLY|AF_HIDE_ADD, 21 ),
     'time_to_send'=>      array('Time to Send',  'datetime', ATT_DEFAULT_NOW, 0, 21 ),
     'sent_time'=>         array('Sent',          'datetime', ATT_DEFAULT_NULL, AF_READONLY|AF_HIDE_ADD|AF_READONLY_EDIT, 21 ),
     'user_id'=>           array('User',          'link',     ATT_DEFAULT_USER_ID, AF_SEARCHABLE|AF_HIDE_ADD|AF_READONLY_EDIT, 'user'),
     'session_id'=>        array('Session ID',    'str',      '', AF_SEARCHABLE, 40),
     'ip'=>                array('IP',            'ip',      ATT_DEFAULT_CLIENT_IP, AF_HIDE_ADD|AF_READONLY, 20 ),
     'sender'=>            array('Sender',        'str',      '', AF_OBLIGATORY, 20),
     'recipient'=>         array('Recipient(s)',  'text',     '', AF_SEARCHABLE|AF_OBLIGATORY, 20),
     'body'=>              array('Body',          'text',     '', AF_HIDE_LIST|AF_OBLIGATORY),
     'result' =>           array('Result',        'text',      '', 0, 20),
     'try_sent'=>          array('Try sent',      'int',      0, AF_READONLY|AF_HIDE_ADD|AF_READONLY_EDIT, 6 ),
  ),
  
  "mime_type" =>
  array(
     'extension'=>          array('Extension',          'str',   '', AF_PRIMARY|AF_OBLIGATORY|AF_SEARCHABLE, 20),
     'type'=>               array('Type',               'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 255),
     'subtype'=>            array('Subtype',            'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 50),
     'mime'=>               array('Mime',               'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 50),
     'icon'=>               array('Icon Class',         'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 64),
     'allow_upload' =>      array('Allow upload',       'bool', 0, AF_HIDE_ADD|AF_HIDE_LIST, 1),     
     'description'=>        array('Description',        'str',   '', AF_SEARCHABLE, 200),
  ),
  
  "media" => 
  array(
    'id'                   =>array('ID'            , 'id',   0, AF_PRIMARY|AF_HIDE_LIST|AF_AUTOINCREMENT, 11),
    'created'=>            array('Created',          'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
    'modified'=>           array('Timestamp',        'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
    'site_id'            => array('Site Id'     ,     'link', ATT_DEFAULT_SITE_ID, AF_OBLIGATORY|AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 'site'),
    'client_id'            =>array('Client Id'     , 'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 'client'),
    'parent_id'            =>array('Parent Id'     , 'parent_id', 0, AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 11),
    'folder_id'            =>array('Folder Id'     , 'folder_id', 0, AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 11),
    'parent_type'          =>array('Parent Type'   , 'parent_type', '', AF_HIDE_LIST|AF_READONLY, 30),
    'parent_field'         =>array('Parent Field'  , 'str', 0, AF_HIDE_LIST|AF_READONLY, 100),
    'property_type'        =>array('Property Type' , 'parent_type', 0, AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 30),
    'guid'=>                 array('GUID',           'uuid',        ATT_DEFAULT_UUID, AF_SEARCHABLE|AF_READONLY|AF_HIDE_ADD, 24),
    'source_id'            =>array('Source'        , 'many2one',   0, AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 'source'),
    'city_id'              =>array('City Id'       , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 6),
    'user_id'              =>array('User Id'       , 'link', ATT_DEFAULT_USER_ID, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'),
    'host_id'              =>array('Host ID'       , 'link', 0, AF_HIDE_ADD|AF_READONLY|AF_HIDE_LIST|AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),
    'artist_id'            =>array('Artist Id'     , 'link', ATT_DEFAULT_USER_ID, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'),
    'product_id'           =>array('Product Id'    , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),
    'rank'                 =>array('Rank'          , 'int', 0, AF_HIDE_ADD, 3),
    'path'                 =>array('Path'          , 'str', 0, AF_HIDE_LIST, 255),
    'src'                  =>array('Server src'    , 'str', 0, AF_OBLIGATORY|AF_READONLY_EDIT|AF_HIDE_LIST, 255),
    'filename'             =>array('Original File' , 'str', '', AF_OBLIGATORY|AF_READONLY_EDIT, 255),
    'user_filename'        =>array('User File name', 'file', '', AF_OBLIGATORY),

    'name'                 =>array('Name'          , 'str', '', 0, 255),
    'comment'              =>array('Comment'       , 'html', '', 0),
    'short_description'    => array('Short Description', 'str',    '', AF_HIDE_LIST, 200),

    'type'                 =>array('Type'          , 'str', '', AF_READONLY|AF_HIDE_ADD, 20),
    'mime'                 =>array('Mime'          , 'str', '', AF_READONLY|AF_HIDE_ADD, 50),
    'size'                 =>array('File size'     , 'int', 0, AF_HIDE_LIST|AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),
  
    'active'               =>array('Active'        , 'bool', 1, AF_HIDE_ADD|AF_HIDE_LIST, 1),
    'missing_file'         =>array('Missing file?' , 'bool', 0, AF_HIDE_ADD|AF_HIDE_LIST, 1),
    'missing_parent'       =>array('Missing parent?' , 'bool', 0, AF_HIDE_ADD|AF_HIDE_LIST, 1),
    'resize'               =>array('Resize?'       , 'bool', 0, AF_HIDE_ADD|AF_HIDE_LIST, 1),
    'resized'              =>array('Resized?'      , 'bool', 0, AF_HIDE_ADD|AF_HIDE_LIST, 1),
    'optimized'            =>array('Optimized?'    , 'bool', 0, AF_READONLY),
    'processed'            =>array('Processed?'    , 'bool', 0, AF_HIDE_LIST, 1),    

    
    'filesize'             =>array('Filesize'      , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),

    // image or video
    'height'               =>array('Height'        , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),
    'width'                =>array('Width'         , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),

    // image
    'depth'                =>array('Bit Depth'     , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 2),
    'exif'                 =>array('Exif'          , 'json', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 2),

    // image/audio/video
    'channels'             =>array('Channels'      , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 2),

    // video    
    'frame_rate'           =>array('Frame rate (fps)' , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),
    'video_codec'          =>array('Video Codec' ,  'str', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 50),

    // audio/video    
    'length'               =>array('Length (sec.)' , 'float', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, '6.2'),
    'bit_rate'             =>array('Bit rate (bps)' , 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),
    'sample_rate'          =>array('Sample rate (Hz)', 'int', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 11),
    'audio_codec'          =>array('Audio Codec' ,  'str', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 50),
    
    'url'                  =>array('Url'           , 'str', '', AF_HIDE_ADD|AF_HIDE_LIST, 255),
    'descriptiontype_id'   =>array('Desc.Type'     , 'many2one', '', AF_HIDE_ADD|AF_SEARCHABLE, 'description_type'),
  
    'var1'                 =>array('Var1'          , 'str', '', AF_HIDE_LIST, 50),
    'var2'                 =>array('Var2'          , 'str', '', AF_HIDE_LIST, 50),
    'var3'                 =>array('Var3'          , 'str', '', AF_HIDE_LIST, 50),
    'var4'                 =>array('Var4'          , 'str', '', AF_HIDE_LIST, 50),
                                                                                  
    'original_currency'   => array('Original Currency' , 'valSelect', 'EUR', AF_HIDE_LIST|AF_READONLY_EDIT, $currencyArray),     
    'original_price'      => array('Original price'    , 'money', 0, AF_HIDE_LIST, '8.2'),  
    'original_height'     => array('Original height'   , 'float', 0, AF_HIDE_LIST, '4.1'),    
    'original_width'      => array('Original width'    , 'float', 0, AF_HIDE_LIST, '4.1'),    
    'original_url'        => array('Original url'      , 'url', '', AF_HIDE_LIST, 255),   
    'original_count'      => array('Original count'    , 'int', 0, AF_HIDE_LIST, 4),    
    'original_available'  => array('Original available', 'bool', 0, AF_HIDE_LIST, 4),    
    'original_year'       => array('Original year'     , 'str', '', AF_HIDE_LIST, 4),     
    'original_technique'  => array('Original technique', 'str', '', AF_HIDE_LIST, 255),     

    'digital_currency'    => array('Digital Currency' , 'valSelect', 'EUR', AF_HIDE_LIST|AF_READONLY_EDIT, $currencyArray),     
    'digital_price'       => array('Digital price'     , 'money', 0, AF_HIDE_LIST, '8.2'),  
    'digital_height'      => array('Digital height'    , 'int', 0, AF_HIDE_LIST, 4),    
    'digital_width'       => array('Digital width'     , 'int', 0, AF_HIDE_LIST, 4),    

    'copyright_name'      => array('Copyright name',     'str', '', AF_HIDE_LIST, 128),     
    'copyright_url'       => array('Copyright URL',      'url', '', AF_HIDE_LIST, 255),     
                                                                                
    'cloud_url'       => array('Cloud URL',      'url', '', AF_HIDE_LIST),     
    'cloud_data'       => array('Cloud Data',     'json', '', AF_HIDE_LIST),  
    'tags'                 =>array('tags'          , 'json', 0, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 2),
  ),

  "profile_media_map" => 
  array(
    'id'                   => array('ID'            , 'id',   0, AF_PRIMARY|AF_HIDE_LIST|AF_AUTOINCREMENT, 11),   
    'user_id'              => array('User Id'       , 'link', ATT_DEFAULT_USER_ID, AF_OBLIGATORY|AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'),
    'media_id'             => array('Media Id'     , 'link', ATT_DEFAULT_NULL, AF_HIDE_ADD, 'media'),
    'site_id'              => array('Site Id'     ,    'link', ATT_DEFAULT_SITE_ID,  AF_OBLIGATORY|AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 'site'),
    'artist_id'            => array('Artist Id'     , 'link', ATT_DEFAULT_NULL, AF_OBLIGATORY|AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'),
    'name'                 => array('Name'  , 'str', '', AF_SEARCHABLE, 128), 
    'curator_id'           => array('Curator Id'    , 'link', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'), 
    'original_year'        => array('Original year'     , 'str', '', AF_HIDE_LIST, 4),     
    'original_technique'   => array('Original technique', 'str', '', AF_HIDE_LIST, 255),
    'artist_name'          => array('Artist Name'  , 'str', '', AF_SEARCHABLE, 50), 
    'curator_name'         => array('Curator Name'  , 'str', '', AF_SEARCHABLE, 50), 
    'created'              => array('Created',          'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
    'artwork_id'           =>array('Artwork Id'     , 'link', ATT_DEFAULT_NULL, AF_HIDE_ADD, 'artwork'),
    'active'               => array('Active'        , 'bool', 1, 0, 1),
   ),
  
  
  "artwork" => 
  array(
    'id'                   =>array('ID'            , 'id',   0, AF_PRIMARY|AF_HIDE_LIST|AF_AUTOINCREMENT, 11),
    'site_id'              =>array('Site Id'     ,    'link', ATT_DEFAULT_SITE_ID, AF_OBLIGATORY|AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 'site'),
    'client_id'            =>array('Client Id'     , 'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY, 'client'),
    'user_id'              =>array('User Id'       , 'link', ATT_DEFAULT_USER_ID, AF_OBLIGATORY|AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'),
    'artist_id'            =>array('Artist Id'     , 'link', ATT_DEFAULT_NULL, AF_OBLIGATORY|AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'),
    'curator_id'           =>array('Curator Id'    , 'link', ATT_DEFAULT_NULL, AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 'user'),
    'artist_ids'=>         array('Artists',           'json',  ATT_DEFAULT_NULL, AF_SEARCHABLE|AF_HIDE_ADD),
    
    //'media_id'             =>array('Media Id'     , 'link', ATT_DEFAULT_NULL, AF_HIDE_ADD, 'media'),

    'artist_name'          =>array('Artist Name(s)'  , 'text', '', AF_HIDE_ADD|AF_READONLY),

    'created'=>            array('Created',          'datetime',   ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY, 6 ),
    'modified'=>           array('Timestamp',        'timestamp',   0, AF_HIDE|AF_READONLY, 14 ),
    'active'               =>array('Active'        , 'bool', 1, 0, 1),
    'public'=>            array('Public',            'bool',   1, 0),
    'show_website'=>      array('Show Website',      'bool',   1, 0),
    //'type'                 =>array('Type'          , 'str', '', AF_SEARCHABLE, 20),

    'type'                 =>array('Type'          , 'str', '', AF_SEARCHABLE, 20),
    'status' =>              array('Status',         'many2one', 0, AF_LOG|AF_SEARCHABLE , 'artwork_status'),
    
    //'type'=>        array('Type',                'one2dci', 0,   AF_HIDE_LIST|AF_ONE2ONE_INTEGRATE|AF_HIDE_ADD, 'artwork_type'),

    //'status' =>              array('Status',         'many2one', 0, AF_LOG|AF_SEARCHABLE , 'artwork'),
    //'status'=>        array('Status',                'one2dci', 0,   AF_HIDE_LIST|AF_ONE2ONE_INTEGRATE|AF_HIDE_ADD, 'artwork_status'),

    'title'                 =>array('Title'          , 'str', '', 0, 255),
    'comment'              =>array('Comment'       , 'html', '', 0),
    'short_description'    => array('Short Description', 'str',    '', AF_HIDE_LIST, 200),
    'description'    =>     array('Description', 'html',    '', AF_HIDE_LIST),

                                                                              
    'currency'   => array('Currency' , 'valSelect', 'EUR', AF_HIDE_LIST|AF_READONLY_EDIT, $currencyArray),     
    'price'      => array('price'    , 'money', 0, AF_HIDE_LIST, '8.2'),  
    //'price_option'  => array('price option'    , 'valSelect', 0, AF_HIDE_LIST, ['','Display Price','Hide Price','Price on inquiry']),  
    'price_option'  => array('price option'    , 'int', 0, AF_HIDE_LIST, 1),  
    'height'     => array('height'   , 'float', 0, AF_HIDE_LIST, '4.1'),    
    'width'      => array('width'    , 'float', 0, AF_HIDE_LIST, '4.1'),    
    'depth'      => array('depth'    , 'float', 0, AF_HIDE_LIST, '4.1'),    
    'unit'       => array('unit'     , 'valSelect', 0, AF_HIDE_LIST, ['','cm','inches','meter','feet']),    
    'url'        => array('url'      , 'url', '', AF_HIDE_LIST, 255),   
    //'available'  => array('available', 'bool', 0, AF_HIDE_LIST, 4),    
    'technique'  => array('technique', 'str', '', AF_HIDE_LIST, 255),     
    'passepartout' =>array('passepartout' , 'yesno', 0, AF_HIDE_ADD|AF_HIDE_LIST),
    'frame'        =>array('frame'        , 'yesno', 0, AF_HIDE_ADD|AF_HIDE_LIST),
    'presentation' => array('presentation'    , 'int', 0, AF_HIDE_LIST, 5), 

    'editions'      => array('editions'    , 'int', 0, AF_HIDE_LIST, 4),    
    'edition'      => array('edition'    , 'int', 0, AF_HIDE_LIST, 4),    
              
    'primary_year'       => array('Primary year'     , 'str', '', AF_HIDE_LIST, 4),     
    'primary_year_type'  => array('Primary year type', 'valSelect', '', AF_HIDE_LIST, ['','Started', 'Created', 'Negative', 'Painted', 'Sculpted', 'Conceived']),     

    'secondary_year'          => array('Secondary year'     ,  'str', '', AF_HIDE_LIST, 4),     
    'secondary_year_type'     => array('Secondary year type',  'valSelect', '', AF_HIDE_LIST, ['','Finished','Printed','Restored','Cast','Created']),     

    'copyright_name'      => array('Copyright name',     'str', '', AF_HIDE_LIST, 128),     
    'copyright_url'       => array('Copyright URL',      'url', '', AF_HIDE_LIST, 255),     

    'video_time'          => array('Video time'    ,      'time', '', AF_HIDE_LIST),
    'video_director'      => array('Video director',      'str', '', AF_HIDE_LIST, 128),
    'video_producer'      => array('Video producer',      'str', '', AF_HIDE_LIST, 128),
    'video_narrator'      => array('Video narrator',      'str', '', AF_HIDE_LIST, 128), 
    'video_sound'         => array('Video sound'  ,       'str', '', AF_HIDE_LIST, 128), 
    'video_editor'        => array('Video editor' ,       'str', '', AF_HIDE_LIST, 128), 
    'video_cast'          => array('Video cast'   ,       'text', '', AF_HIDE_LIST, 255),
           
    'title_image' =>      array('Title Media',           'media',      '', 0, $single_media_options),
    'title_image_json'  => array('Image (json)',      'json',     '', 0),

    'related_media' =>    array('Related Media',         'media',      '', 0, $gallery_media_options),
    'related_media_json'  => array('Media (json)',      'json',     '', 0),
        
  ),
    
  "source" => 
  array(
     'id'                   =>array('ID',                    'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'active'=>            array('Active',            'bool',   0, 0),
     'created'=>           array('Created',           'date',   '', AF_HIDE_LIST, 6 ),
     'timestamp'=>         array('Timestamp',         'timestamp',   0, AF_HIDE_LIST, 6 ),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_READONLY|AF_HIDE_ADD, 'site'),
     'scraped'=>           array('Scraped',          'date',   '', AF_HIDE_LIST, 6 ),
     'name'=>              array('Name',              'str',   '', AF_OBLIGATORY, 100 ),
     'address_street'       => array('Address Street'  , 'str', '', AF_HIDE_LIST, 100),
     'address_postcode'     => array('Address Postcode', 'str', '', AF_HIDE_LIST, 10),
     'address_city'         => array('Address City'    , 'str', '', AF_HIDE_LIST, 50),
     'base_url'=>          array('Base URL',             'url',  '', AF_HIDE_LIST, 100),
     'object_url'=>        array('Object URL',           'url',  '', AF_HIDE_LIST, 100),
     'image_url'=>         array('Image URL',            'url',  '', AF_HIDE_LIST, 100),
     'local_dir'=>         array('Local Dir',            'url',  '', AF_HIDE_LIST, 100),
     'xml_url'=>           array('XML URL',              'url',  '', AF_HIDE_LIST, 100),
     'api_key'         =>  array('API KEY'    , 'str', '', AF_HIDE, 32),
  ),
  

  
  "object_description" => 
  array(
     'id'=>            	    array('ID',                	'id',       '', AF_PRIMARY|AF_AUTOINCREMENT|AF_HIDE, 11),
     'parent_type'=>        array('Parent Type',        'parent_type', '', AF_OBLIGATORY, $parentTypeArray),
     'parent_id'=>          array('Parent Id',          'parent_id',      0, AF_OBLIGATORY, 11),
     'site_id' =>           array('Site',               'link',     ATT_DEFAULT_SITE_ID, 0, 'site'),
     'created'=>            array('Created',            'datetime',  ATT_DEFAULT_NOW, AF_HIDE_ADD|AF_HIDE_LIST|AF_READONLY|AF_HIDE_LIST, 6),
     'timestamp'=>          array('Timestamp',          'timestamp',  ATT_DEFAULT_NOW, AF_HIDE|AF_HIDE_ADD|AF_READONLY, 14),
     'user_id'=>            array('Author',             'many2one', ATT_DEFAULT_USER_ID, AF_HIDE_LIST|AF_HIDE_ADD|AF_SEARCHABLE|AF_READONLY, 'user'),
     'machine_translated'=> array('Machine Translated', 'bool',  0, AF_HIDE_ADD|AF_READONLY),
     'language'=>           array('Language',    			  'valSelect', 'en', AF_OBLIGATORY|AF_SEARCHABLE, $langArray),
     'type' =>              array('Desc.Type',          'many2one', 0, AF_SEARCHABLE, 'description_type'),
     'title'=>              array('Title (text)',       'text',     '', AF_HIDE_LIST, 255),
     'description'=>        array('Content (HTML)',     'html',     '', AF_OBLIGATORY|AF_HIDE_LIST),
     'image' =>            array('Image',             'image',      '', AF_HIDE_LIST, 128),
    ),

  "dictionary" => 
  array(
     'id'=>            	    array('ID',                	'id',       '', AF_PRIMARY|AF_AUTOINCREMENT|AF_HIDE, 11),
     'site_id' =>           array('Site',               'link',     0, 0, 'site'),
     'user_id'=>            array('Author',             'link',     ATT_DEFAULT_USER_ID, AF_HIDE|AF_SEARCHABLE|AF_READONLY, 'user'),
     'machine_translated'=> array('Machine Translated?', 'bool',  0, AF_HIDE|AF_HIDE_ADD|AF_SEARCHABLE|AF_READONLY),

     'language'=>           array('Language',           'many2one',  ATT_DEFAULT_LANG, AF_OBLIGATORY|AF_SEARCHABLE|AF_READONLY_EDIT, 'language'),
     'term'=>            	  array('Term',               'str',       '', AF_OBLIGATORY|AF_SEARCHABLE|AF_READONLY_EDIT, 100),
     'phrase'=>             array('Phrase (HTML)',      'text',     '', AF_OBLIGATORY|AF_HIDE_LIST, 30),

     /** get rid of author */
     //'author'=>             array('Author',             'link',     ATT_DEFAULT_USER_ID, AF_HIDE|AF_SEARCHABLE|AF_READONLY, 'user'),
     'timestamp'=>          array('Timestamp',          'timestamp',     ATT_DEFAULT_NOW, AF_HIDE|AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 14),
     'accessed'=>          array('Last used',          'datetime',     ATT_DEFAULT_NULL, AF_HIDE|AF_HIDE_LIST|AF_HIDE_ADD|AF_READONLY, 14),

    ),
  
  

  "description_type" => 
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
     'private'  =>          array('Private?',            'bool',  0,  0, 0),
     'generic'  =>          array('Generic?',            'bool',  0,  0, 0),
     'description'=>        array('Description',         'text',     '', AF_HIDE_LIST),
  ),
  

  "article_type" => 
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),
  
  
  "billing_period" => 
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'active'=>             array('Active',             'bool',  1, AF_SEARCHABLE, 1),
     'name'=>               array('Name',               'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),
  
  
  "exchange_rate" => 
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'last_updated'=>       array('Timestamp',          'timestamp',     ATT_DEFAULT_NOW, AF_READONLY, 14),
     'from_currency'=>      array('From Currency',      'valSelect', 0, AF_OBLIGATORY, $currencyArray),
     'to_currency'=>        array('To Currency',        'valSelect', 0, AF_OBLIGATORY, $currencyArray),
     'real_rate'=>          array('Bank rate',          'float',    0, AF_READONLY, '8.8'),
     'our_rate'=>           array('Our rate',           'float',    0, AF_READONLY, '8.8'),
     'skim'=>               array('Our commission %',   'float',    0, 0, '2.2'),
  ),
  
  
  "class_group" => 
  array(
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 128),
     'description'=>        array('Description',         'str',   '', AF_HIDE_LIST, 128),
  ),
  
  "class" => 
  array(
     'type'=>            	  array('Type',                'str',   '', AF_PRIMARY, 128),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 128),
     'active'  =>           array('Active?',             'bool',     1,  AF_HIDE_ADD, 0),
     //'site_id'=>            array('Site',                'link', 0, 0, 'site'),
     'user_level'=>         array('User Level',          'many2one',  ATT_DEFAULT_USER_LEVEL, AF_SEARCHABLE|AF_OBLIGATORY, 'user_level'),
     'group'=>              array('Group',               'many2one',  ATT_DEFAULT_USER_LEVEL, AF_SEARCHABLE|AF_OBLIGATORY, 'class_group'),
     'table'=>              array('Table',               'str',   '', 0, 128),
     'key'=>                array('Key',                 'str',   '', 0, 128),
     'flags'=>              array('Flags',               'flags',   0, 0, $__CLASS_FLAGS),
     'descriptor'=>         array('Descriptor',          'str',   '', 0, 128),
     'sort'=>               array('Sort',                'str',   '', 0, 128),
     'parent_class'=>       array('Parent class',        'str',   '', AF_SEARCHABLE, 128),
     'attributes'=>         array('Attributes',          'one2many', 0, 0, 'attribute', 'class'),
  
  ),
  
  
  "attribute" => 
  array(
     'id'=>            	    array('ID',                  'int',   '', AF_PRIMARY|AF_AUTOINCREMENT, 100),
     'field'=>            	array('DB Field',            'str',   '', AF_OBLIGATORY, 100),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
     'class'=>              array('Class',               'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
     'type'=>               array('Type',                'many2one', '', AF_SEARCHABLE, 'attribute_type'),
     'length'=>             array('Length',              'int',   0, 0, 4),
     'flags'=>              array('Flags',               'flags',   0, 0, $__ATTR_FLAGS),
     'default_value'=>      array('Default Value',       'str',   '', 0, 128),
     'relation_class'=>     array('Relation',            'str',   '', 0, 128),
     'relation_filters'=>   array('Filters',             'json',   '', 0),
     'relation_sort'=>      array('Sort',                'str',   '', 0, 128),
//     'args1'=>              array('Arguments 1',         'str',   '', 0, 128),
//     'args2'=>              array('Arguments 2',         'str',   '', 0, 128),
  ),
  
  
  "attribute_type" => 
  array(
//     'id'=>            	    array('ID',                  'int',   '', AF_PRIMARY|AF_AUTOINCREMENT, 100),
     'name'=>            	  array('Type',                'str',   '', AF_PRIMARY, 128),
     'modifier'=>           array('Modifier',            'str',   0, 0, 16),
     'length'=>             array('Length',              'int',   0, 0, 4),
     'db_type'=>            array('DB Type',             'valSelect', '', AF_OBLIGATORY|AF_SEARCHABLE, $db_types),
     'description'=>        array('Description',         'str',   '', AF_SEARCHABLE, 255),
  ),
  
  
  "provider" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
     'active'  =>          array('Active?',           'bool',     1,  AF_HIDE_ADD, 0),
  
     'name'          =>    array('name'         ,    'str',   '', 1, 30 ),
     'provider_id'   =>    array('provider_id'  ,    'str',   '', 1, 20 ),
     'key_required'  =>    array('key_required' ,    'bool',     1,  AF_HIDE_ADD, 0),
     'key_name'      =>    array('key_name'     ,    'str',   '', 1, 10 ),
     'allow_auth'    =>    array('allow_auth'   ,    'bool',     1,  AF_HIDE_ADD, 0),
     'allow_profile' =>    array('allow_profile',    'bool',     1,  AF_HIDE_ADD, 0),
     'allow_contact' =>    array('allow_contact',    'bool',     1,  AF_HIDE_ADD, 0),
     'allow_stream'  =>    array('allow_stream' ,    'bool',     1,  AF_HIDE_ADD, 0),
     'allow_post'    =>    array('allow_post'   ,    'bool',     1,  AF_HIDE_ADD, 0),
     'allow_api'     =>    array('allow_api'    ,    'bool',     1,  AF_HIDE_ADD, 0),
     'url'           =>    array('url'          ,    'str',   '', 1, 50 ),
     'url_reg'       =>    array('url_reg'      ,    'str',   '', 1, 200 ),
     'url_doc'       =>    array('url_doc'      ,    'str',   '', 1, 200 ),
  ),
  
  
  "provider_key" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
     'active'  =>          array('Active?',           'bool',     1,  AF_HIDE_ADD, 0),
     'site_id'=>           array('Site',              'link', 0, 0, 'site'),
     'provider_id'=>       array('Provider',          'link', 0, 0, 'provider'),
     'api_key'=>           array('API Key',           'str',   '', 0, 100 ),
     'api_secret'=>        array('API Secret',        'str',   '', 0, 100 ),
     'scope'=>             array('Scope',             'str',   '', 0, 100 ), // A comma-separated list of permissions you want to request from the user. See the Facebook docs for a full list of available permissions: http://developers.facebook.com/docs/reference/api/permissions.
     'display'=>           array('Display',           'str',   '', 0, 20 ),  // The display context to show the authentication page. Options are: page, popup, iframe, touch and wap. Read the Facebook docs for more details: http://developers.facebook.com/docs/reference/dialogs#display. Default: page
  ),
  
  
  "provider_profile" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
  //         'active'  =>          array('Active?',           'bool',     1,  AF_HIDE_ADD, 0),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
//     'site_id'          => array('Site',              'many2one', 0, AF_OBLIGATORY, 'site'),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_HIDE_LIST|AF_LOG, 'site'),
     'user_id'          => array('User',              'many2one', 0, AF_OBLIGATORY, 'user'),
     'provider_id'      => array('Provider',          'many2one', 0, AF_OBLIGATORY, 'provider'),
     'identifier'       => array('identifier'   ,      'str',      '', 1, 50 ),
     'url_web'          => array('webSiteURL'   ,      'str',      '', 0, 255 ),
     'url_profile'       => array('profileURL'   ,     'str',      '', 0, 255 ),
     'photo'             => array('photoURL'     ,     'str',      '', 0, 255 ),
     'display_name'      => array('displayName'  ,     'str',      '', 0, 50 ),
     'description'      => array('description'  ,      'str',      '', 0, 50 ),
     'first_name'        => array('firstName'    ,     'str',      '', 0, 50 ),
     'last_name'         => array('lastName'     ,     'str',      '', 0, 50 ),
     'gender'           => array('gender'       ,      'str',      '', 0, 10 ),
     'language'         => array('language'     ,      'str',      '', 0, 2 ),
     'age'              => array('age'          ,      'str',      '', 0, 3 ),
     'birth_day'         => array('birthDay'     ,     'str',      '', 0, 2 ),
     'birth_month'       => array('birthMonth'   ,     'str',      '', 0, 2 ),
     'birth_year'        => array('birthYear'    ,     'str',      '', 0, 2 ),
     'email'            => array('email'        ,      'str',      '', 0, 50 ),
     'email_verified'    => array('emailVerified',     'str',      '', 0, 50 ),
     'phone'            => array('phone'        ,      'str',      '', 0, 30 ),
     'address'          => array('address'      ,      'str',      '', 0, 200 ),
     'country'          => array('country'      ,      'str',      '', 0, 2 ),
     'region'           => array('region'       ,      'str',      '', 0, 50 ),
     'city'             => array('city'         ,      'str',      '', 0, 50 ),
     'zip'              => array('zip'          ,      'str',      '', 0, 20 ),
     'friends'          => array('friends'      ,      'json',     ''),
  ),
  
  
  "user_profile" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_EDIT, 14 ),
  //         'active'  =>          array('Active?',           'bool',     1,  AF_HIDE_ADD, 0),
     'site_id'=>           array('Site',              'link',   ATT_DEFAULT_SITE_ID, AF_HIDE_LIST|AF_LOG, 'site'),
     'client_id'=>         array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'rank'=>              array('Rank',               'int',  0, 0, 3),
  
     'location_id'=>       array('Location ID',        'link',   0, 0, 'location'),
     
     'user_id'=>           array('User',              'link', 0, AF_OBLIGATORY, 'user'),
     'url_web'          => array('WebSiteURL'   ,      'str',      '', 0, 255 ),
     'photo'            => array('Photo(s)'     ,      'media',     '', 0, $user_media_options),
     'photo_json'       => array('Photo (json)',        'json',     '', 0),
     
     'display_name'     => array('Display name'  ,     'str',      '', 0, 50 ),
     'gallery_name'     => array('Gallery name'  ,     'str',      '', 0, 50 ),
     'first_name'       => array('First name'    ,     'str',      '', 0, 50 ),
     'infix'            => array('Infix'     ,         'str',      '', 0, 20 ),
     'last_name'        => array('Last name'     ,     'str',      '', 0, 50 ),
     'gender'           => array('Gender'       ,      'str',      '', 0, 10 ),
     'title'            => array('Title',              'str',      '', 0, 50 ),
     'bio_description'      => array('Description'  ,      'html',     '', 0),
     'interests'        => array('Interests'  ,        'text',     '', 0),
     'location'         => array('Where you live',     'str',      '', 0, 100 ),
     'timezone'         => array('Timezone',           'timezone', '', 0, 32),
     
     'tags'             => array('Tags',               'text',     '', 0),
     'language'         => array('Language'     ,      'str',      '', 0, 2 ),
     'languages'=>         array('Languages',          'json',     '',   AF_HIDE_LIST),
     'age'              => array('Age'          ,      'str',      '', 0, 3 ),
     'birth_day'        => array('Birth day'     ,     'str',      '', 0, 2 ),
     'birth_month'      => array('Birth month'   ,     'str',      '', 0, 2 ),
     'birth_year'       => array('Birth year'    ,     'str',      '', 0, 2 ),
     'date_of_birth'    => array('Date of birth' ,     'date',     '', 0, 2 ),
     'email_address'    => array('Email'        ,      'str',      '', 0, 128 ),
     'email_verified'   => array('Email Verified',     'str',      '', AF_READONLY, 128),     
     'phone'            => array('Phone'        ,      'phone',    '', 0, 30 ),
     'phone_verified'   => array('Phone Verifed',      'str',      '', AF_READONLY, 30),
     'passport'         => array('Passport'        ,   'media',    '', 0, $passport_media_options),
     'passport_verified'=> array('Passport Verifed',   'str',      '', AF_READONLY, 128),
     'mobile'           => array('Mobile'        ,     'str',      '', 0, 30 ),
     'fax'              => array('Fax'        ,     'str',      '', 0, 30 ),
     'address'          => array('Address'      ,      'str',      '', 0, 200 ), // pradeepa
     'address1'         => array('Address'      ,      'str',      '', 0, 200 ),
     'address2'         => array('Address...',         'str',      '', 0, 200 ),
     'city'             => array('City'         ,      'str',      '', 0, 50 ),
     'zip'              => array('Postal code'  ,      'str',      '', 0, 20 ),
     'state'            => array('State/region',       'str',      '', 0, 50 ),
     'country'          => array('Country'      ,      'many2one', 0, 0, 'countries'),
     'following'        => array('Following'     ,     'int',      0, 0, 6),
     'followers'        => array('Followers'     ,     'int',      0, 0, 6),
     'additional'       => array('Additional fields',  'json',     ''),

     'paypal_id'        => array('PayPal ID',          'str',      '', AF_READONLY, 128),
     'bank_name'=>          array('Bank Name',           'str',  '',  AF_HIDE_LIST, 100),
     'account_name'=>       array('Bank Account Name',   'str',  '',  AF_HIDE_LIST, 100),
     'account_number'=>     array('Bank Account Number', 'str',  '',  AF_HIDE_LIST, 100),
     'account_iban'=>       array('Bank Account IBAN',   'str',  '',  AF_HIDE_LIST, 100),
     'account_routing'=>    array('Bank Account Routing','str',  '',  AF_HIDE_LIST, 100),
     'account_country'=>    array('Bank Account Country','str',  '',  AF_HIDE_LIST, 2),    

     'tax_id'=>              array('Legal/tax id',       'str',   '', AF_HIDE_LIST, 32),
     'commerce_id'=>         array('Business association id',  'str',   '', AF_HIDE_LIST, 32),

     'golf_home_course' => array('Home Course',        'link', 0, 0, 'golf_club'),
     'golf_favorite_course' => array('Favorite Course',        'link', 0, 0, 'golf_club'),
     'golf_handicap'        => array('Handicap'     ,     'int',      0, 0, 6),
     'location_lat'         => array('Latitude'         , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_long'        => array('Longitude'        , 'str','', AF_HIDE_ADD|AF_HIDE_LIST, 24),
     'location_bounds'      => array('Bounds',            'json', '', AF_HIDE_ADD|AF_HIDE_LIST),
     'address_formatted'    => array('Formatted Address', 'text',   '', AF_HIDE_LIST),
     
     'gallery_solo_shows'   => array('Solo Shows'  ,      'html',     '', 0),
     'gallery_group_shows'  => array('Group Shows'  ,      'html',     '', 0),
     'press'                => array('Press'  ,      'html',     '', 0),
     'awards'=>        array('Awards',           'json',  '', AF_SEARCHABLE|AF_HIDE_ADD),
     'data'=>          array('Data',           'json',  '', AF_SEARCHABLE|AF_HIDE_ADD),
     'working_hours'=>        array('Working Hours',           'json',  '', AF_SEARCHABLE|AF_HIDE_ADD),
     'social'=>         array('Social',           'json',  '', AF_SEARCHABLE|AF_HIDE_ADD),
     'artist'  =>       array('artist',      'bool',     1,  AF_HIDE_ADD, 1),
     'contact'=>        array('More Contacts',           'json',  '', AF_SEARCHABLE|AF_HIDE_ADD),
     'solo_shows_'          => array('Solo Shows'  ,      'html',     '', 0),
     'group_shows_'         => array('Group Shows'  ,      'html',     '', 0),
     'press_data'               => array('Press data'  ,      'html',     '', 0),
     'show_data'            => array('Show Data'  ,      'html',     '', 0),
     'free_text'            => array('Free Text'  ,      'html',     '', 0),
  ),



//array('allowed_types' => 'image', 'description_type' => IMAGE_TYPE_PROFILE, 'resize' => true, 'minimum_size' => 640)

  "sharing" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD, 14 ),
     'client_id'=>         array('Client',    			  'many2one', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>           array('Site',              'many2one',   ATT_DEFAULT_SITE_ID, AF_HIDE_LIST|AF_HIDE_ADD|AF_LOG, 'site'),
     'page_id' =>          array('Page',              'link',        0, AF_HIDE_EDIT|AF_HIDE_ADD, 'site_page'),  
     'user_id'=>           array('User',              'link', ATT_DEFAULT_USER_ID, AF_OBLIGATORY|AF_HIDE_ADD, 'user'),
     'parent_type'      => array('object type',       'str',      ATT_DEFAULT_NULL, AF_HIDE_ADD, 50),
     'parent_id'        => array('object id',        'int',      ATT_DEFAULT_NULL, AF_HIDE_ADD),
     'name'             => array('Name',              'str',      '', 0, 255),
     'title'            => array('Title',             'str',      '', 0, 255),
     'subtitle'         => array('Subtitle',          'str',      '', 0, 128),
     'description'      => array('Description',       'text',     '', AF_HIDE_ADD),
     'language'=>          array('Language',          'many2one',  ATT_DEFAULT_LANG, AF_HIDE_LIST|AF_HIDE_ADD, 'language'),
     'count'            => array('count',             'int',      0, AF_HIDE_ADD),
     'type'             => array('type',              'valSelect', 0, AF_OBLIGATORY, $shareTypeArray),
     'show_title'       => array('Show title?',        'valSelect', 0, AF_HIDE_LIST, $positionArray),
     'share_user_id'=>     array('Sharer',            'link', ATT_DEFAULT_USER_ID, AF_HIDE_ADD, 'user'),
     'image_index'     => array('plugin image index',  'int',      0, AF_HIDE_ADD),
     'style'            => array('style',             'text',     '', 0),
     'link'             => array('link',              'str',      '', AF_HIDE_ADD, 255),
     'size'             => array('size',              'str',      '', AF_HIDE_ADD, 16),

     'plugin_options'   => array('Plugin Options',    'json',     '', AF_HIDE_ADD),
     'popup_options'   => array('Popup Options',    'json',     '', AF_HIDE_ADD),
     /** don't include URL or plug-in will break: change to full_url */
     
     //     'url'              => array('full url',          'str',      '', AF_HIDE_ADD, 255),
     'shorturl'         => array('shorturl',          'str',      '', AF_HIDE_ADD, 50),
     'data'             => array('data',              'text',     '', AF_HIDE_ADD),
     'title_image'      => array('Title image',       'str', 0, AF_HIDE_LIST, 200),
     'container_class'  => array('Container Class',   'str', '', AF_HIDE_LIST, 200),
     'container_tag'  => array('Container Tag',   'str', '', AF_HIDE_LIST, 16),
     'object_tag'  => array('Object Tag',   'str', '', AF_HIDE_LIST, 16),
     'object_class'  => array('Object Class',   'str', '', AF_HIDE_LIST, 200),
  ),

  
  "share_log" => 
  array(
     'id'=>                array('ID',                'id',   0, AF_PRIMARY|AF_AUTOINCREMENT, 3),
     'created'=>           array('Created',           'datetime', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD),
     'timestamp'=>         array('Timestamp',         'timestamp', ATT_DEFAULT_NOW, AF_HIDE_LIST|AF_READONLY|AF_HIDE_ADD, 14 ),

     'client_id'=>         array('Client',    			  'link', ATT_DEFAULT_CLIENT_ID, AF_OBLIGATORY|AF_READONLY|AF_HIDE, 'client'),
     'site_id'=>           array('Target Site',       'link', ATT_DEFAULT_SITE_ID, AF_HIDE_LIST|AF_HIDE_ADD|AF_LOG, 'site'),
     'page_id' =>          array('Target Page',       'link', ATT_DEFAULT_PAGE_ID, AF_HIDE_EDIT|AF_HIDE_ADD, 'site_page'),  
     'user_id'=>           array('Target User',       'link', ATT_DEFAULT_USER_ID, AF_HIDE_ADD, 'user'),

     'sid'=>               array('Share ID',          'str',  '', AF_OBLIGATORY, 20),
     'parent_sid'=>        array('Share parent ID',   'str',  '', 0, 20),

     'source_site_id'=>    array('Source Site',       'link', ATT_DEFAULT_SITE_ID, AF_HIDE_LIST|AF_HIDE_ADD|AF_LOG, 'site'),
     'source_page_id' =>   array('Source Page',       'link', 0, AF_HIDE_EDIT|AF_HIDE_ADD, 'site_page'),  
     'source_user_id'=>    array('Source User',       'link', ATT_DEFAULT_USER_ID, AF_HIDE_EDIT|AF_HIDE_ADD, 'user'),

     'obj_type'         => array('object type',       'str',  ATT_DEFAULT_NULL, AF_HIDE_ADD, 50),
     'obj_id'           => array('object id',         'int',  ATT_DEFAULT_NULL, AF_HIDE_ADD),
                                                      
     'count'            => array('Direct hits',       'int',  0, AF_HIDE_ADD),
     'pcount'           => array('Indirect hits',     'int',  0, AF_HIDE_ADD),
                                                      
     'title'            => array('title',             'str',      '', 0, 255),
     'language'=>          array('Language',          'many2one',  ATT_DEFAULT_LANG, AF_HIDE_LIST|AF_HIDE_ADD, 'language'),
     'url'              => array('url',               'str',      '', AF_HIDE_ADD, 255),
     'referer'          => array('referer',           'str',      '', AF_HIDE_ADD, 255),
     'remote_ip'        => array('IP',                'ip',       ATT_DEFAULT_CLIENT_IP, AF_SEARCHABLE|AF_READONLY, 15 ),
     'remote_cc'        => array('Country Code',      'many2one', ATT_DEFAULT_CLIENT_CC, AF_HIDE_LIST, 'countries'),
     'data'             => array('data',              'json',     '', AF_HIDE_ADD),
  ),
  
  "weekday" =>
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),

  "gender" =>
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),

  "access_token" =>
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'service'=>            array('service',            'str',  'google', AF_OBLIGATORY|AF_SEARCHABLE, 40),
     'account_id'=>         array('account id',         'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
     'account_name'=>       array('account name',       'str',  '', AF_SEARCHABLE, 100),
     'scope'  =>            array('scope'  ,            'text', '', 0),
     'token'  =>            array('token'  ,            'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 200),
  ),
  
  "google_application" =>
  array(
     'id'=>            	    array('ID',              'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('name',            'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 40),
     'api_key'=>            array('API Key',         'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 40),
     'client_id'=>          array('client id',       'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 60),
     'client_secret'=>      array('client secret',   'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 40),
     'redirect_uri'  =>     array('Redirect URI'  ,  'str',  '', AF_OBLIGATORY|AF_SEARCHABLE, 255),
     'scope'  =>            array('scope',           'text', '', 0),
  ),  

  "default_attributes" =>
  array(
     'id'=>            	    array('ID',                	'id',   '', AF_PRIMARY|AF_AUTOINCREMENT, 11),
     'name'=>               array('Name',                'str',   '', AF_OBLIGATORY|AF_SEARCHABLE, 100),
  ),
  
);


<?php


// the user ID of system super-user

define("SYSTEM_TIMEZONE", "UTC");
define("SYSTEM_CURRENCY", "EUR");

define("SITE_ID_AFR",       10);
define("SYSTEM_USER_ID",    501);
define("SYSTEM_SERVER_ID",  1006);
define("DEV_SERVER_ID",     1006);
define("NAMESERVER_ID",     1006);
define("NAMESERVER_SLAVE_ID", 1007);
define("ADMIN_TEMPLATE_ID",   1010);

define("RENTAL_SEARCH_LIMIT", 200); // hard limit for rental search

define("BOOKING_REVIEW_PERIOD", 140); // how many days do we allow for a booking review
define("BOOKING_REMINDER_PERIOD", 3); // how many days in advance do we notify of an upcoming booking


define("DATEPICKER_FORMAT_JS", 'dd-mm-yyyy'); /** human readable for date pickers, should match global g_date_format on top of  /js/afr.js */
define("DATEPICKER_FORMAT_PHP", 'd-m-Y'); /** same as above, but formatted for php's date() function */
define("DATEPICKER_FORMAT_SMARTY", '%d-%m-%Y'); /** same as above, but formatted for php's strftime() function */
define("DATE_FORMAT", "j-n-Y"); /** d-m-yyyy w/o leading zeros, used in on screen printing */

define("SITE_TEMPLATE_BLANK", 1);

define("LOCATION_NOISE", 100); // 50 meters of noise to shortstay

define("CREDIT_CARD_TEXT_SHORT",   "APT.RENTAL");
define("CREDIT_CARD_TEXT_LONG",    "APARTMENT RENTAL");
define("CREDIT_CARD_COMPANY_TEXT", "Apartments-for-rent");

define("SYSTEM_SERVER_IP",  '195.66.135.147');
define("MEDIA_SERVER_IP",  '159.203.158.1');

define("NAMESERVER_1", 'NS2.KNXX.NET');
define("NAMESERVER_2", 'NS3.KNXX.NET');

define("PHONE_TYPE_HOME", 1);
define("PHONE_TYPE_CELL", 2);
define("PHONE_TYPE_MOBILE", 2);
define("PHONE_TYPE_WORK", 3);
define("PHONE_TYPE_OFFICE", 3);
define("PHONE_TYPE_BUSINESS", 3);

// for offices
define("DAY_SHIFT",       1);
define("MORNING_SHIFT",   2);
define("AFTERNOON_SHIFT", 3);

// for invoices
define("BILLTO_GUEST",   0);
define("BILLTO_ORG",     1);
define("BILLTO_CONTACT", 2);
define("BILLTO_AGENT",   3);

define("WEEKEND_RATE_CUTOFF", 7); // only applies to stays shorter than 7 days

define("PROPERTY_SORT", 'priority DESC, source_id, book_direct DESC, created DESC');

define("PROPERTY_SITE_CORPORATE"       , 2); // applies to site->property_site

define("PROPERTY_TYPE_SHORTSTAY"       , 101);
define("PROPERTY_TYPE_LONGSTAY"        , 102);
define("PROPERTY_TYPE_REALESTATE"      , 103);
define("PROPERTY_TYPE_BOAT"            , 104);
define("PROPERTY_TYPE_OFFICE"          , 105);
define("PROPERTY_TYPE_HOTEL"           , 106);
define("PROPERTY_TYPE_SHORTSTAY_OFFICE", 107);

define("BOAT_TYPE_BARGE"        , 100);
define("BOAT_TYPE_CANAL"        , 101);
define("BOAT_TYPE_CLASSIC"      , 102);
define("BOAT_TYPE_MONUMENTAL"   , 103);
define("BOAT_TYPE_OPEN"         , 104);
define("BOAT_TYPE_RIVER"        , 105);
define("BOAT_TYPE_SAIL"         , 106);
define("BOAT_TYPE_SALON"        , 107);
define("BOAT_TYPE_SELF_OPERATED", 108);
define("BOAT_TYPE_STEAM"        , 109);
define("BOAT_TYPE_PARTY"        , 110);

define("FEE_ALL", 0);
define("FEE_AFR", 1);
define("FEE_OWNER", 2);

define("DEFAULT_RADIUS", 5000); // 1000 meter radius

define("FEE_NAME",          0);
define("FEE_FOR_WHOM",      1);
define("FEE_SHOW_NULL",     2);
define("FEE_LI_TYPE",       3);
define("FEE_PER_BLOCK",     4);
define("FEE_SHOW_QUANTITY", 5);

define ("RES_TYPE_ADMIN", 5);
define ("RES_TYPE_DIRECT", 10);
define ("RES_TYPE_REQUEST", 20);
define ("RES_TYPE_INQUIRY", 30);
//define ("RES_TYPE_REQUEST_OWNER", 20);

define ("RATE_CLASS_NONE", 0);
define ("RATE_CLASS_STANDARD", 10);
define ("RATE_CLASS_WEEKEND", 15);
define ("RATE_CLASS_SPECIAL", 20);
define ("RATE_CLASS_CUSTOM", 30);

define ("CC_TYPE_AMERICAN_EXPRESS", 1);
define ("CC_TYPE_VISA",             2);
define ("CC_TYPE_MASTERCARD",       3);

define ("DEFAULT_CHECKIN_TIME", "15:00:00");
define ("DEFAULT_CHECKOUT_TIME", "11:00:00");
define ("STATUS_REQUEST_EXPIRES", 2*24*60*60); // 24 hours before Proposed reservation expires
define ("STATUS_PENDING_EXPIRES", 2*24*60*60); // 24 hours before Pending reservation expires
define ("STATUS_PROPOSED_EXPIRES", 2*24*60*60); // 24 hours before Proposed reservation expires

// reservation status 
define ("STATUS_ALL", -100);
define ("STATUS_EXPIRED", -40);
define ("STATUS_REJECTED", -30);    // declined by guest
define ("STATUS_CANCELED", -20);   // cancelled by guest/owner/staff
define ("STATUS_CANCELLED", -20);   // cancelled by guest/owner/staff
define ("STATUS_DECLINED", -10);    // declined by owner
define ("STATUS_DRAFT", 1);
define ("STATUS_INQUIRY", 5);
define ("STATUS_REQUEST", 10);
define ("STATUS_UNCONFIRMED", 10);
define ("STATUS_PROPOSED", 15);
define ("STATUS_PENDING", 20);
define ("STATUS_RESERVED", 30);
define ("STATUS_BLOCKED", 35);
define ("STATUS_COMPLETE", 40);
define ("STATUS_COMPLETED", 40);
define ("STATUS_CLEANING", 100);
define ("STATUS_MAINTENANCE", 110);
define ("STATUS_PICKUP", 120);

// define ("EVENT_TYPE_OTHER",       1);
// define ("EVENT_TYPE_CHECKIN",     10);
// define ("EVENT_TYPE_CHECKOUT",    20);
// define ("EVENT_TYPE_CLEANING",    30);
// define ("EVENT_TYPE_TRANSPORT",   40);
// define ("EVENT_TYPE_MAINTENANCE", 50);
// define ("EVENT_TYPE_MEETING",     60);
// define ("EVENT_TYPE_TICKET",      70);

                                       
define("EVENT_TYPE_OTHER"       , 100); //100,  'Other',      
define("EVENT_TYPE_CHECKIN"     , 101); //101,  'Checkin',    
define("EVENT_TYPE_CHECKOUT"    , 102); //102,  'Checkout',   
define("EVENT_TYPE_CLEANING"    , 103); //103,  'Cleaning',   
define("EVENT_TYPE_DRIVING"     , 104); //104,  'Driving',    
define("EVENT_TYPE_PILOTING"    , 105); //105,  'Piloting',   
define("EVENT_TYPE_TRANSPORT"   , 106); //106,  'Transport',  
define("EVENT_TYPE_PICKUP"      , 107); //107,  'Pickup',     
define("EVENT_TYPE_DELIVERY"    , 108); //108,  'Delivery',   
define("EVENT_TYPE_MAINTENANCE" , 109); //109,  'Maintenance',
define("EVENT_TYPE_MEETING"     , 110); //110,  'Meeting',    
define("EVENT_TYPE_CATERING"    , 111); //111,  'Catering',   
define("EVENT_TYPE_COOKING"     , 112); //112,  'Cooking',    
define("EVENT_TYPE_SERVING"     , 113); //113,  'Serving',    
define("EVENT_TYPE_TICKET"      , 114); //114,  'Ticket Event'
define("EVENT_TYPE_AGENDA"      , 115); //114,  'Ticket Event'
                                       

define ("EVENT_STATUS_ALL", -100);
define ("EVENT_STATUS_EXPIRED", -40);
define ("EVENT_STATUS_CANCELED", -20);   // cancelled by guest/owner/staff
define ("EVENT_STATUS_CANCELLED", -20);   // cancelled by guest/owner/staff
define ("EVENT_STATUS_DECLINED", -10);    // declined by owner
define ("EVENT_STATUS_UNCONFIRMED", 10);
define ("EVENT_STATUS_PROPOSED", 20);
define ("EVENT_STATUS_CONFIRMED", 30);
define ("EVENT_STATUS_COMPLETE", 40);

// enquiry status 
define ("ENQ_STATUS_ALL", -100);
define ("ENQ_STATUS_EXPIRED", -30);
define ("ENQ_STATUS_REJECTED", -20);    // declined by guest
define ("ENQ_STATUS_CANCELED", -10);   // cancelled by guest/owner/staff
define ("ENQ_STATUS_CANCELLED", -10);   // cancelled by guest/owner/staff
define ("ENQ_STATUS_UNAVAILABLE", -1); // declined by owner or agent
define ("ENQ_STATUS_REQUEST", 1);
define ("ENQ_STATUS_RECEIVED", 5);
define ("ENQ_STATUS_REPLIED", 10);
define ("ENQ_STATUS_CONFIRMED", 20);
define ("ENQ_STATUS_PROCESSING", 30);
define ("ENQ_STATUS_COMPLETE", 40);

define ("SHOW_INVOICE", -10);

define ("SHOW_CALENDAR", 10);
define ("SHOW_LIST",    20);
//define ("SHOW_CHANGES", 30);
//define ("SHOW_CHECKINS", 30);
//define ("SHOW_CLEANING", 100);
//define ("SHOW_MAINTENANCE", 110);
//define ("SHOW_PICKUPS", 120);

define ("USER_LEVEL_USER", 1);
define ("USER_LEVEL_GUEST", 10);
define ("USER_LEVEL_ORG", 15);
define ("USER_LEVEL_OWNER", 20);
define ("USER_LEVEL_HOST", 20);
define ("USER_LEVEL_AFFILIATE", 22);
define ("USER_LEVEL_CONTRACTOR", 24);
define ("USER_LEVEL_AGENT", 26);   
define ("USER_LEVEL_CURATOR", 26);   
define ("USER_LEVEL_GALLERY", 26);   
define ("USER_LEVEL_EDITOR", 28);   
define ("USER_LEVEL_STAFF", 30);   
define ("USER_LEVEL_ADMIN", 40);   
define ("USER_LEVEL_SYSTEM", 50);  

define ("INV_TOLERANCE", 0.1); // tolerance for invoices. Paid if less than this number

define ("INV_TYPE_BOOKING", 10); // non-corporate invoice for deposit + booking fee
define ("INV_TYPE_CORRECTION", 20);
define ("INV_TYPE_BALANCE", 30);
define ("INV_TYPE_SUPPLEMENTAL", 40);
define ("INV_TYPE_SECURITY_DEPOSIT", 50);   
define ("INV_TYPE_PARTIAL", 60);  
define ("INV_TYPE_COMPLETE", 70); 
define ("INV_TYPE_BLOCK", 80); 
define ("INV_TYPE_AGENT", 90); 
define ("INV_TYPE_CONSULTING", 100); 

define ("INV_STATUS_REFUNDED",-20);
define ("INV_STATUS_CANCELED",-10);
define ("INV_STATUS_CANCELLED",-10);
define ("INV_STATUS_DRAFT",-1);
define ("INV_STATUS_OPEN",1);
define ("INV_STATUS_SENT",10);
define ("INV_STATUS_IN_DISPUTE",20);
define ("INV_STATUS_PARTIAL",30);
define ("INV_STATUS_PENDING",40);
define ("INV_STATUS_COMPLETE",50);
                        
define ("ORDER_STATUS_REFUNDED",-20);
define ("ORDER_STATUS_CANCELLED",-10);
define ("ORDER_STATUS_DRAFT",-1);
define ("ORDER_STATUS_OPEN",1);
define ("ORDER_STATUS_PENDING",10);
define ("ORDER_STATUS_PARTIAL_PAYMENT",15);
define ("ORDER_STATUS_PAID",20);
define ("ORDER_STATUS_BACKORDER",25);
define ("ORDER_STATUS_PROCESSING",30);
define ("ORDER_STATUS_PARTIAL",35);
define ("ORDER_STATUS_SHIPPED",40);
define ("ORDER_STATUS_RECEIVED",45);
define ("ORDER_STATUS_COMPLETE",50);

define ("GATEWAY_PAYPAL", 10 );
define ("GATEWAY_IDEAL",  20 );
define ("GATEWAY_MOLLIE",  30 );
define ("GATEWAY_PAYSIUS",  40 );
define ("GATEWAY_BITPAY",  50 );

define ("PAYSIUS_STATUS_INCOMPLETE", 0); // Incomplete	0	Missing required data          
define ("PAYSIUS_STATUS_PREPARED",   1); // Prepared	1	Complete and ready for checkout  
define ("PAYSIUS_STATUS_PENDING",    2); // Pending	2	Awaiting payment from the customer.
define ("PAYSIUS_STATUS_VERIFIED",   3); // Verified	3	Payment has been verified.

define ("SMS_FLAG_REQUEST",   1);
define ("SMS_FLAG_CONFIRMED", 2);
define ("SMS_FLAG_CANCELED",  4);
define ("SMS_FLAG_CANCELLED",  4);
define ("SMS_FLAG_PROPOSED",  8);

define ("BILLING_PERIOD_MINUTE",  10);
define ("BILLING_PERIOD_HOUR",    20);
define ("BILLING_PERIOD_SHIFT",   30);
define ("BILLING_PERIOD_DAY",     40);
define ("BILLING_PERIOD_WEEKDAY", 50);
define ("BILLING_PERIOD_WEEK",    60);
define ("BILLING_PERIOD_MONTH",   70);
define ("BILLING_PERIOD_QUARTER", 80);
define ("BILLING_PERIOD_YEAR",    90);      

//define ("PAYMENT_STATUS_CANCELED_REVERSAL", 10 );
//define ("PAYMENT_STATUS_CREATED",           30 );
//define ("PAYMENT_STATUS_DENIED",            40 );
//define ("PAYMENT_STATUS_PROCESSED",         100);
//define ("PAYMENT_STATUS_VOIDED",            110);
//define ("PAYMENT_STATUS_DEDUCTED",          120);

define ("PAYMENT_STATUS_EXPIRED",           -10);
define ("PAYMENT_STATUS_FAILED",            -20);
define ("PAYMENT_STATUS_CANCELLED",         -30);
define ("PAYMENT_STATUS_CANCELED",          -30);
define ("PAYMENT_STATUS_VOIDED",            -40);

define ("PAYMENT_STATUS_OPEN",               10);
define ("PAYMENT_STATUS_PENDING",            70);
define ("PAYMENT_STATUS_PROCESSING",         15);
define ("PAYMENT_STATUS_COMPLETE",           20);
define ("PAYMENT_STATUS_COMPLETED",          20);
define ("PAYMENT_STATUS_REFUNDED",           80);
define ("PAYMENT_STATUS_PARTIALLY_REFUNDED", 85);
define ("PAYMENT_STATUS_REVERSED",           90);



define("PAYMENT_TYPE_CREDIT_CARD"         , 10);
define("PAYMENT_TYPE_CREDIT_CARD_AUTH"    , 20);
define("PAYMENT_TYPE_CREDIT_CARD_CAPTURE" , 30);
define("PAYMENT_TYPE_CASH"                , 40);
define("PAYMENT_TYPE_BANK"                , 50);
define("PAYMENT_TYPE_PAYPAL"              , 60);
define("PAYMENT_TYPE_ECHECK"              , 70);
define("PAYMENT_TYPE_IDEAL"               , 80);
define("PAYMENT_TYPE_REFUND"              , 90);
define("PAYMENT_TYPE_TRANSFER"            , 100);
define("PAYMENT_TYPE_CORRECTION"          , 110);
define("PAYMENT_TYPE_AGENT"               , 120);
define("PAYMENT_TYPE_AGENT_OWES"          , 130);
define("PAYMENT_TYPE_BLOCK"               , 140);
define("PAYMENT_TYPE_PROMISSORY"          , 150);
define("PAYMENT_TYPE_BITCOIN"             , 160);
define("PAYMENT_TYPE_PIN"                 , 170);
define("PAYMENT_TYPE_MOLLIE"              , 180);

define("PAYMENT_TYPE_BANCONTACT"          , 190);
define("PAYMENT_TYPE_SOFORT"              , 200);
define("PAYMENT_TYPE_SEPA"                , 210);
define("PAYMENT_TYPE_BELFIUS"             , 220);
define("PAYMENT_TYPE_PAYSAFECARD"         , 230);
define("PAYMENT_TYPE_SEPA_DIRECT"         , 240);

define("PAYMENT_SOURCE_GUEST"             , 10);
define("PAYMENT_SOURCE_ORG"               , 20);
define("PAYMENT_SOURCE_OWNER"             , 30);
define("PAYMENT_SOURCE_AFF"               , 40);
define("PAYMENT_SOURCE_AGENT"             , 50);
define("PAYMENT_SOURCE_CLIENT"            , 60);
define("PAYMENT_SOURCE_SYSTEM"            , 70);

define("BILLING_MINUTE"  , 10);
define("BILLING_HOUR"    , 20);
define("BILLING_SHIFT"   , 30);
define("BILLING_DAY"     , 40);
define("BILLING_WEEKDAY" , 50); 
define("BILLING_WEEK"    , 60);
define("BILLING_MONTH"   , 70);
define("BILLING_QUARTER" , 80);
define("BILLING_YEAR"    , 90);

define("AGENT_TYPE_HOTEL"               , 10);

define("CONTRACTOR_TYPE_CLEANER"         , 10);
define("CONTRACTOR_TYPE_DRIVER"          , 20);
define("CONTRACTOR_TYPE_CAPTAIN"         , 30);
define("CONTRACTOR_TYPE_SKIPPER"         , 30);
define("CONTRACTOR_TYPE_HANDYMAN"        , 40);
define("CONTRACTOR_TYPE_CHEF"            , 50);
define("CONTRACTOR_TYPE_WAITER"          , 60);
define("CONTRACTOR_TYPE_CATERER"         , 70);

define("TX_ST_OK"                       ,  1);
define("TX_ST_HTTP_ERROR"               ,  2);
define("TX_ST_TEST"                     ,  3);
define("TX_ST_TXN_NOT_VERIFIED"         , 10);
define("TX_ST_EXISTING_TXN_ID"          , 20);
define("TX_ST_INCOMPLETE"               , 30);
define("TX_ST_WRONG_RECEIVER_EMAIL"     , 40);
define("TX_ST_MISSING_AMOUNT"           , 50);
define("TX_ST_UNKNOWN_CURRENCY"         , 60);
define("TX_ST_UNKNOWN_RESERVATION_ID"   , 70);
define("TX_ST_MISSING_RESERVATION_ID"   , 80);
define("TX_ST_UNKNOWN_INVOICE_ID"       , 90);
define("TX_ST_MISSING_INVOICE_ID"       , 100);

define("LI_TYPE_DEPOSIT"          , 10 );
define("LI_TYPE_BALANCE"          , 20 );
define("LI_TYPE_RENT"             , 30 );
//define("LI_TYPE_PARTIAL"          , 40 );
define("LI_TYPE_CHANGE_LENGTH"    , 50 );
define("LI_TYPE_CHANGE_GUESTS"    , 60 );
define("LI_TYPE_CHANGE_PROPERTY"  , 70 );
define("LI_TYPE_CHANGE_RATE"      , 80 );
define("LI_TYPE_CHANGE_DISCOUNT"  , 90 );
define("LI_TYPE_CHANGE_TOTAL"     , 100);
define("LI_TYPE_CITY_TAX"         , 105);
define("LI_TYPE_SERVICES"         , 110);
define("LI_TYPE_EXTRAS"           , 120);
define("LI_TYPE_SECURITY_DEPOSIT" , 130);
define("LI_TYPE_SERVICE_FEE"      , 135);
define("LI_TYPE_BOOKING_FEE"      , 140);
define("LI_TYPE_CLEANING_FEE"     , 150);
define("LI_TYPE_LOCAL_FEE"        , 160);
define("LI_TYPE_PICKUP_FEE"       , 170);
define("LI_TYPE_OTHER_FEE"        , 180);
define("LI_TYPE_CALLOUT_FEE"      , 181);
define("LI_TYPE_WAITING_FEE"      , 182);
define("LI_TYPE_CC_FEE"           , 190);

define("LI_TYPE_GATEWAY_FEE"         , 191);
define("LI_TYPE_SOFTWARE_USAGE_FEE"  , 192);
define("LI_TYPE_SOFTWARE_HOSTING_FEE", 193);
define("LI_TYPE_MESSAGING_FEE"       , 194);

define("LI_TYPE_PRODUCT"          , 200);
define("LI_TYPE_COMMISSION"       , 210);
define("LI_TYPE_PACKAGE"          , 220);
define("LI_TYPE_CATERING"         , 230);
define("LI_TYPE_CONSULTING"       , 240);

define("PRODUCT_CATEGORY_CATERING", 1002);
define("PRODUCT_CATEGORY_PACKAGE" , 1004);
                    
define("DESC_TYPE_HOTEL"      ,   1);
define("DESC_TYPE_ROOM"       ,   2);
define("DESC_TYPE_FOOD"       ,   3);
define("DESC_TYPE_AREA"       ,   4);
define("DESC_TYPE_TRAVEL"     ,   5);
define("DESC_TYPE_QUICK"      ,   6);
define("DESC_TYPE_IMPORTANT"  ,   7);
define("DESC_TYPE_RULES"      ,   8);
define("DESC_TYPE_DIRECTIONS" ,   9);
define("DESC_TYPE_ALBUM"      ,  10);
define("DESC_TYPE_STRAP"      ,  11);
define("DESC_TYPE_GOLF"       ,  12);
define("DESC_TYPE_GUIDE"       , 13);
define("DESC_TYPE_INTERACTION", 14);
define("DESC_TYPE_PROPERTY"   ,  50);
define("DESC_TYPE_CHECKIN"    ,  51);
define("DESC_TYPE_LOGO"       , 101);
define("DESC_TYPE_BANNER"     , 102);
define("DESC_TYPE_SLIDESHOW"  , 103);

define("IMAGE_TYPE_ALBUM"      , 100);
define("IMAGE_TYPE_LOGO"       , 110);
define("IMAGE_TYPE_BANNER"     , 120);
define("IMAGE_TYPE_ARTICLE"    , 130);
define("IMAGE_TYPE_PRODUCT"    , 140);
define("IMAGE_TYPE_TITLE"      , 150);
define("IMAGE_TYPE_PROFILE"    , 160);
define("IMAGE_TYPE_INFO"       , 170);
define("IMAGE_TYPE_SLIDESHOW"  , 180);
define("IMAGE_TYPE_ARTWORK"    , 190);
define("IMAGE_TYPE_SPLASH"     , 200);
define("IMAGE_TYPE_ICON"       , 210);
define("IMAGE_TYPE_PHOTO"      , 220);
define("IMAGE_TYPE_MAP"        , 230);

define("LANDMARK_TYPE_ATTRACTION"         , 100);
define("LANDMARK_TYPE_PARK"               , 110);
define("LANDMARK_TYPE_TRANSPORT"          , 120);
define("LANDMARK_TYPE_ACCOMMODATION"      , 130);
define("LANDMARK_TYPE_RESTAURANT"         , 140);
define("LANDMARK_TYPE_BAR"                , 150);
define("LANDMARK_TYPE_VENUE"              , 160);
define("LANDMARK_TYPE_BOARDING"           , 170);
define("LANDMARK_TYPE_PLACE"              , 180);
define("LANDMARK_TYPE_GOLFCOURSE"         , 190);

define("ARTICLE_TYPE_BLOG"         , 100);
define("ARTICLE_TYPE_NEWS"         , 110);
define("ARTICLE_TYPE_TESTIMONIAL"  , 120);
define("ARTICLE_TYPE_FAQ"          , 130);

define("LEGAL_ENT_OPAO",    1001);
define("LEGAL_ENT_HOUSIST", 1007);
define("LEGAL_ENT_YAH",     1008);

define("ADDRESS_TYPE_PHYSICAL", 10);
define("ADDRESS_TYPE_DELIVERY", 20);

define("DUMMY_CUSTOMER_ID", 100);

define("SOURCE_ID_AFR", 1);
define("SOURCE_ID_TEQA", 25);
define("SOURCE_ID_KIGO", 26);
define("SOURCE_ID_LEADING_COURSES", 28);
define("SOURCE_ID_AIRBNB", 29);
define("SOURCE_LEADING_COURSES", 28);
define("SOURCE_AIRBNB", 29);
define("SOURCE_ID_INTERHOME", 30);

//define("INTERHOME_SALES_OFFICE", 2048);
define("INTERHOME_SALES_OFFICE", 4040);
define("INTERHOME_RETAILER_CODE", '');


define("MEDIA_COLLECTION_TYPE_PROJECT",    100);
define("MEDIA_COLLECTION_TYPE_COLLECTION", 110);
define("MEDIA_COLLECTION_TYPE_EXHIBITION", 120);
define("MEDIA_COLLECTION_TYPE_FAVORITES",  130);
define("MEDIA_COLLECTION_TYPE_FEATURED",   140);

define("COLLECTION_TYPE_PROJECT",    100);
define("COLLECTION_TYPE_COLLECTION", 110);
define("COLLECTION_TYPE_EXHIBITION", 120);
define("COLLECTION_TYPE_FAVORITES",  130);
define("COLLECTION_TYPE_FEATURED",   140);

define("CANCELLATION_POLICY_FLEXIBLE", 1);
define("CANCELLATION_POLICY_MODERATE", 2);
define("CANCELLATION_POLICY_STRICT",   3);

// response to reservation request
define("RESPONSE_AVAILABLE", 1);
define("RESPONSE_SPECIAL_OFFER", 2);
define("RESPONSE_MORE_INFO", 3);

define("RESPONSE_NOT_AVAILABLE", 10);
define("RESPONSE_NOT_GOOD_FIT", 11);
define("RESPONSE_NOT_COMFORTABLE", 12);
define("RESPONSE_NOT_ENOUGH_INFO", 13);
define("RESPONSE_NOT_ATTRACTIVE", 14);
define("RESPONSE_NOT_OTHER", 15);

define("PROVIDER_OPENID"      , 1000);
define("PROVIDER_GOOGLE"      , 1001);
define("PROVIDER_GOOGLE_PLUS" , 1001);
define("PROVIDER_FACEBOOK"    , 1002);
define("PROVIDER_TWITTER"     , 1003);
define("PROVIDER_YAHOO"       , 1004);
define("PROVIDER_MYSPACE"     , 1005);
define("PROVIDER_WINDOWS"     , 1006);
define("PROVIDER_WINDOWS_LIVE", 1006);
define("PROVIDER_LINKEDIN"    , 1007);
define("PROVIDER_FOURSQUARE"  , 1008);
define("PROVIDER_AOL"         , 1009);



function cc_list1($code = '') {
$ccArray = array(
    "AF" =>  array('name' =>"Afghanistan افغانستان"),
    "AX" =>  array('name' =>"Åland Islands Aaland Aland" ,'priority' => "0.5"),
    "AL" =>  array('name' =>"Albania"),
    "DZ" =>  array('name' =>"Algeria الجزائر"),
    "AS" =>  array('name' =>"American Samoa" ,'priority' => "0.5"),
    "AD" =>  array('name' =>"Andorra" ,'priority' => "0.5"),
    "AO" =>  array('name' =>"Angola"),
    "AI" =>  array('name' =>"Anguilla" ,'priority' => "0.5"),
    "AQ" =>  array('name' =>"Antarctica" ,'priority' => "0.5"),
    "AG" =>  array('name' =>"Antigua And Barbuda" ,'priority' => "0.5"),
    "AR" =>  array('name' =>"Argentina"),
    "AM" =>  array('name' =>"Armenia Հայաստան"),
    "AW" =>  array('name' =>"Aruba" ,'priority' => "0.5"),
    "AU" =>  array('name' =>"Australia" ,'priority' => "1.5"),
    "AT" =>  array('name' =>"Austria Österreich Osterreich Oesterreich"),
    "AZ" =>  array('name' =>"Azerbaijan"),
    "BS" =>  array('name' =>"Bahamas"),
    "BH" =>  array('name' =>"Bahrain البحرين"),
    "BD" =>  array('name' =>"Bangladesh বাংলাদেশ" ,'priority' => "2"),
    "BB" =>  array('name' =>"Barbados"),
    "BY" =>  array('name' =>"Belarus Беларусь"),
    "BE" =>  array('name' =>"Belgium België Belgie Belgien Belgique" ,'priority' => "1.5"),
    "BZ" =>  array('name' =>"Belize"),
    "BJ" =>  array('name' =>"Benin"),
    "BM" =>  array('name' =>"Bermuda" ,'priority' => "0.5"),
    "BT" =>  array('name' =>"Bhutan भूटान"),
    "BO" =>  array('name' =>"Bolivia"),
    "BQ" =>  array('name' =>"Bonaire, Sint Eustatius and Saba"),
    "BA" =>  array('name' =>"Bosnia and Herzegovina Босна и Херцеговина"),
    "BW" =>  array('name' =>"Botswana"),
    "BV" =>  array('name' =>"Bouvet Island"),
    "BR" =>  array('name' =>"Brazil Brasil" ,'priority' => "2"),
    "IO" =>  array('name' =>"British Indian Ocean Territory"),
    "BN" =>  array('name' =>"Brunei Darussalam"),
    "BG" =>  array('name' =>"Bulgaria България"),
    "BF" =>  array('name' =>"Burkina Faso"),
    "BI" =>  array('name' =>"Burundi"),
    "KH" =>  array('name' =>"Cambodia កម្ពុជា"),
    "CM" =>  array('name' =>"Cameroon"),
    "CA" =>  array('name' =>"Canada" ,'priority' => "2"),
    "CV" =>  array('name' =>"Cape Verde Cabo"),
    "KY" =>  array('name' =>"Cayman Islands" ,'priority' => "0.5"),
    "CF" =>  array('name' =>"Central African Republic"),
    "TD" =>  array('name' =>"Chad تشاد‎ Tchad"),
    "CL" =>  array('name' =>"Chile"),
    "CN" =>  array('name' =>"China Zhongguo Zhonghua Peoples Republic 中国/中华" ,'priority' => "3.5"),
    "CX" =>  array('name' =>"Christmas Island" ,'priority' => "0.5"),
    "CC" =>  array('name' =>"Cocos (Keeling) Islands" ,'priority' => "0.5"),
    "CO" =>  array('name' =>"Colombia"),
    "KM" =>  array('name' =>"Comoros جزر القمر"),
    "CG" =>  array('name' =>"Congo"),
    "CD" =>  array('name' =>"Congo, the Democratic Republic of the Congo-Brazzaville Repubilika ya Kongo"),
    "CK" =>  array('name' =>"Cook Islands" ,'priority' => "0.5"),
    "CR" =>  array('name' =>"Costa Rica"),
    "CI" =>  array('name' =>"Côte d'Ivoire Cote dIvoire"),
    "HR" =>  array('name' =>"Croatia Hrvatska"),
    "CU" =>  array('name' =>"Cuba"),
    "CW" =>  array('name' =>"Curaçao Curacao"),
    "CY" =>  array('name' =>"Cyprus Κύπρος Kýpros Kıbrıs"),
    "CZ" =>  array('name' =>"Czech Republic Česká Ceska"),
    "DK" =>  array('name' =>"Denmark Danmark" ,'priority' => "1.5"),
    "DJ" =>  array('name' =>"Djibouti جيبوتي‎ Jabuuti Gabuuti"),
    "DM" =>  array('name' =>"Dominica Dominique" ,'priority' => "0.5"),
    "DO" =>  array('name' =>"Dominican Republic"),
    "EC" =>  array('name' =>"Ecuador"),
    "EG" =>  array('name' =>"Egypt" ,'priority' => "1.5"),
    "SV" =>  array('name' =>"El Salvador"),
    "GQ" =>  array('name' =>"Equatorial Guinea"),
    "ER" =>  array('name' =>"Eritrea إرتريا ኤርትራ"),
    "EE" =>  array('name' =>"Estonia Eesti"),
    "ET" =>  array('name' =>"Ethiopia ኢትዮጵያ"),
    "FK" =>  array('name' =>"Falkland Islands (Malvinas)" ,'priority' => "0.5"),
    "FO" =>  array('name' =>"Faroe Islands Føroyar Færøerne" ,'priority' => "0.5"),
    "FJ" =>  array('name' =>"Fiji Viti फ़िजी"),
    "FI" =>  array('name' =>"Finland Suomi"),
    "FR" =>  array('name' =>"France République française" ,'priority' => "2.5"),
    "GF" =>  array('name' =>"French Guiana"),
    "PF" =>  array('name' =>"French Polynesia Polynésie française"),
    "TF" =>  array('name' =>"French Southern Territories"),
    "GA" =>  array('name' =>"Gabon République Gabonaise"),
    "GM" =>  array('name' =>"Gambia"),
    "GE" =>  array('name' =>"Georgia საქართველო"),
    "DE" =>  array('name' =>"Germany Bundesrepublik Deutschland" ,'priority' => "3"),
    "GH" =>  array('name' =>"Ghana"),
    "GI" =>  array('name' =>"Gibraltar" ,'priority' => "0.5"),
    "GR" =>  array('name' =>"Greece Ελλάδα" ,'priority' => "1.5"),
    "GL" =>  array('name' =>"Greenland grønland" ,'priority' => "0.5"),
    "GD" =>  array('name' =>"Grenada"),
    "GP" =>  array('name' =>"Guadeloupe"),
    "GU" =>  array('name' =>"Guam"),
    "GT" =>  array('name' =>"Guatemala"),
    "GG" =>  array('name' =>"Guernsey" ,'priority' => "0.5"),
    "GN" =>  array('name' =>"Guinea"),
    "GW" =>  array('name' =>"Guinea-Bissau"),
    "GY" =>  array('name' =>"Guyana"),
    "HT" =>  array('name' =>"Haiti"),
    "HM" =>  array('name' =>"Heard Island and McDonald Islands"),
    "VA" =>  array('name' =>"Holy See (Vatican City State)" ,'priority' => "0.5"),
    "HN" =>  array('name' =>"Honduras"),
    "HK" =>  array('name' =>"Hong Kong 香港"),
    "HU" =>  array('name' =>"Hungary Magyarország"),
    "IS" =>  array('name' =>"Iceland Island"),
    "IN" =>  array('name' =>"India भारत गणराज्य Hindustan" ,'priority' => "3"),
    "ID" =>  array('name' =>"Indonesia" ,'priority' => "2"),
    "IR" =>  array('name' =>"Iran, Islamic Republic of ایران"),
    "IQ" =>  array('name' =>"Iraq العراق‎"),
    "IE" =>  array('name' =>"Ireland Éire" ,'priority' => "1.2"),
    "IM" =>  array('name' =>"Isle of Man" ,'priority' => "0.5"),
    "IL" =>  array('name' =>"Israel إسرائيل ישראל"),
    "IT" =>  array('name' =>"Italy Italia" ,'priority' => "2"),
    "JM" =>  array('name' =>"Jamaica"),
    "JP" =>  array('name' =>"Japan Nippon Nihon 日本" ,'priority' => "2.5"),
    "JE" =>  array('name' =>"Jersey" ,'priority' => "0.5"),
    "JO" =>  array('name' =>"Jordan الأردن"),
    "KZ" =>  array('name' =>"Kazakhstan Қазақстан Казахстан"),
    "KE" =>  array('name' =>"Kenya"),
    "KI" =>  array('name' =>"Kiribati"),
    "KP" =>  array('name' =>"Korea, Democratic People's Republic of North Korea"),
    "KR" =>  array('name' =>"Korea, Republic of South Korea" ,'priority' => "1.5"),
    "KW" =>  array('name' =>"Kuwait الكويت"),
    "KG" =>  array('name' =>"Kyrgyzstan Кыргызстан"),
    "LA" =>  array('name' =>"Lao People's Democratic Republic"),
    "LV" =>  array('name' =>"Latvia Latvija"),
    "LB" =>  array('name' =>"Lebanon لبنان"),
    "LS" =>  array('name' =>"Lesotho"),
    "LR" =>  array('name' =>"Liberia"),
    "LY" =>  array('name' =>"Libyan Arab Jamahiriya ليبيا"),
    "LI" =>  array('name' =>"Liechtenstein"),
    "LT" =>  array('name' =>"Lithuania Lietuva"),
    "LU" =>  array('name' =>"Luxembourg"),
    "MO" =>  array('name' =>"Macao"),
    "MK" =>  array('name' =>"Macedonia, The Former Yugoslav Republic Of Македонија"),
    "MG" =>  array('name' =>"Madagascar Madagasikara"),
    "MW" =>  array('name' =>"Malawi"),
    "MY" =>  array('name' =>"Malaysia"),
    "MV" =>  array('name' =>"Maldives"),
    "ML" =>  array('name' =>"Mali"),
    "MT" =>  array('name' =>"Malta"),
    "MH" =>  array('name' =>"Marshall Islands" ,'priority' => "0.5"),
    "MQ" =>  array('name' =>"Martinique"),
    "MR" =>  array('name' =>"Mauritania الموريتانية"),
    "MU" =>  array('name' =>"Mauritius"),
    "YT" =>  array('name' =>"Mayotte"),
    "MX" =>  array('name' =>"Mexico Mexicanos" ,'priority' => "1.5"),
    "FM" =>  array('name' =>"Micronesia, Federated States of"),
    "MD" =>  array('name' =>"Moldova, Republic of"),
    "MC" =>  array('name' =>"Monaco"),
    "MN" =>  array('name' =>"Mongolia Mongγol ulus Монгол улс"),
    "ME" =>  array('name' =>"Montenegro"),
    "MS" =>  array('name' =>"Montserrat" ,'priority' => "0.5"),
    "MA" =>  array('name' =>"Morocco المغرب"),
    "MZ" =>  array('name' =>"Mozambique Moçambique"),
    "MM" =>  array('name' =>"Myanmar"),
    "NA" =>  array('name' =>"Namibia Namibië"),
    "NR" =>  array('name' =>"Nauru Naoero" ,'priority' => "0.5"),
    "NP" =>  array('name' =>"Nepal नेपाल"),
    "NL" =>  array('name' =>"Netherlands Holland Nederland" ,'priority' => "1.5"),
    "NC" =>  array('name' =>"New Caledonia" ,'priority' => "0.5"),
    "NZ" =>  array('name' =>"New Zealand Aotearoa"),
    "NI" =>  array('name' =>"Nicaragua"),
    "NE" =>  array('name' =>"Niger Nijar"),
    "NG" =>  array('name' =>"Nigeria Nijeriya Naíjíríà" ,'priority' => "1.5"),
    "NU" =>  array('name' =>"Niue" ,'priority' => "0.5"),
    "NF" =>  array('name' =>"Norfolk Island" ,'priority' => "0.5"),
    "MP" =>  array('name' =>"Northern Mariana Islands" ,'priority' => "0.5"),
    "NO" =>  array('name' =>"Norway Norge Noreg" ,'priority' => "1.5"),
    "OM" =>  array('name' =>"Oman عمان"),
    "PK" =>  array('name' =>"Pakistan پاکستان" ,'priority' => "2"),
    "PW" =>  array('name' =>"Palau" ,'priority' => "0.5"),
    "PS" =>  array('name' =>"Palestinian Territory, Occupied فلسطين"),
    "PA" =>  array('name' =>"Panama"),
    "PG" =>  array('name' =>"Papua New Guinea"),
    "PY" =>  array('name' =>"Paraguay"),
    "PE" =>  array('name' =>"Peru"),
    "PH" =>  array('name' =>"Philippines Pilipinas" ,'priority' => "1.5"),
    "PN" =>  array('name' =>"Pitcairn" ,'priority' => "0.5"),
    "PL" =>  array('name' =>"Poland Polska" ,'priority' => "1.25"),
    "PT" =>  array('name' =>"Portugal Portuguesa" ,'priority' => "1.5"),
    "PR" =>  array('name' =>"Puerto Rico"),
    "QA" =>  array('name' =>"Qatar قطر"),
    "RE" =>  array('name' =>"Réunion Reunion"),
    "RO" =>  array('name' =>"Romania Rumania Roumania România"),
    "RU" =>  array('name' =>"Russian Federation Rossiya Российская Россия" ,'priority' => "2.5"),
    "RW" =>  array('name' =>"Rwanda"),
    "BL" =>  array('name' =>"Saint Barthélemy St. Barthelemy"),
    "SH" =>  array('name' =>"Saint Helena St."),
    "KN" =>  array('name' =>"Saint Kitts and Nevis St."),
    "LC" =>  array('name' =>"Saint Lucia St."),
    "MF" =>  array('name' =>"Saint Martin (French Part) St."),
    "PM" =>  array('name' =>"Saint Pierre and Miquelon St."),
    "VC" =>  array('name' =>"Saint Vincent and the Grenadines St."),
    "WS" =>  array('name' =>"Samoa"),
    "SM" =>  array('name' =>"San Marino"),
    "ST" =>  array('name' =>"Sao Tome and Principe"),
    "SA" =>  array('name' =>"Saudi Arabia السعودية"),
    "SN" =>  array('name' =>"Senegal Sénégal"),
    "RS" =>  array('name' =>"Serbia Србија Srbija"),
    "SC" =>  array('name' =>"Seychelles" ,'priority' => "0.5"),
    "SL" =>  array('name' =>"Sierra Leone"),
    "SG" =>  array('name' =>"Singapore Singapura சிங்கப்பூர் குடியரசு 新加坡共和国"),
    "SX" =>  array('name' =>"Sint Maarten (Dutch Part)"),
    "SK" =>  array('name' =>"Slovakia Slovenská Slovensko"),
    "SI" =>  array('name' =>"Slovenia Slovenija"),
    "SB" =>  array('name' =>"Solomon Islands"),
    "SO" =>  array('name' =>"Somalia الصومال"),
    "ZA" =>  array('name' =>"South Africa RSA Suid-Afrika"),
    "GS" =>  array('name' =>"South Georgia and the South Sandwich Islands"),
    "SS" =>  array('name' =>"South Sudan"),
    "ES" =>  array('name' =>"Spain España" ,'priority' => "2"),
    "LK" =>  array('name' =>"Sri Lanka ශ්‍රී ලංකා இலங்கை Ceylon"),
    "SD" =>  array('name' =>"Sudan السودان"),
    "SR" =>  array('name' =>"Suriname शर्नम् Sarnam Sranangron"),
    "SJ" =>  array('name' =>"Svalbard and Jan Mayen" ,'priority' => "0.5"),
    "SZ" =>  array('name' =>"Swaziland weSwatini Swatini Ngwane"),
    "SE" =>  array('name' =>"Sweden Sverige" ,'priority' => "1.5"),
    "CH" =>  array('name' =>"Switzerland Swiss Confederation Schweiz Suisse Svizzera Svizra" ,'priority' => "1.5"),
    "SY" =>  array('name' =>"Syrian Arab Republic Syria سورية"),
    "TW" =>  array('name' =>"Taiwan, Province of China 台灣 臺灣"),
    "TJ" =>  array('name' =>"Tajikistan Тоҷикистон Toçikiston"),
    "TZ" =>  array('name' =>"Tanzania, United Republic of"),
    "TH" =>  array('name' =>"Thailand ประเทศไทย Prathet Thai"),
    "TL" =>  array('name' =>"Timor-Leste"),
    "TG" =>  array('name' =>"Togo Togolese"),
    "TK" =>  array('name' =>"Tokelau" ,'priority' => "0.5"),
    "TO" =>  array('name' =>"Tonga"),
    "TT" =>  array('name' =>"Trinidad and Tobago"),
    "TN" =>  array('name' =>"Tunisia تونس"),
    "TR" =>  array('name' =>"Turkey Türkiye Turkiye"),
    "TM" =>  array('name' =>"Turkmenistan Türkmenistan"),
    "TC" =>  array('name' =>"Turks and Caicos Islands" ,'priority' => "0.5"),
    "TV" =>  array('name' =>"Tuvalu" ,'priority' => "0.5"),
    "UG" =>  array('name' =>"Uganda"),
    "UA" =>  array('name' =>"Ukraine Ukrayina Україна"),
    "AE" =>  array('name' =>"United Arab Emirates UAE الإمارات"),
    "GB" =>  array('name' =>"United Kingdom Great Britain England UK Wales Scotland Northern Ireland" ,'priority' => "2.5"),
    "US" =>  array('name' =>"United States USA US United States of America" ,'priority' => "3.5"),
    "UM" =>  array('name' =>"United States Minor Outlying Islands"),
    "UY" =>  array('name' =>"Uruguay"),
    "UZ" =>  array('name' =>"Uzbekistan Ўзбекистон O'zbekstan O‘zbekiston"),
    "VU" =>  array('name' =>"Vanuatu"),
    "VE" =>  array('name' =>"Venezuela"),
    "VN" =>  array('name' =>"Vietnam Việt Nam" ,'priority' => "1.5"),
    "VG" =>  array('name' =>"Virgin Islands, British" ,'priority' => "0.5"),
    "VI" =>  array('name' =>"Virgin Islands, U.S." ,'priority' => "0.5"),
    "WF" =>  array('name' =>"Wallis and Futuna" ,'priority' => "0.5"),
    "EH" =>  array('name' =>"Western Sahara لصحراء الغربية"),  
    "YE" =>  array('name' =>"Yemen اليمن"),
    "ZM" =>  array('name' =>"Zambia"), 
    "ZW" =>  array('name' =>"Zimbabwe")
);
    return $code ? $ccArray[$code] : $ccArray;
}

// simple ass. array for select box
function cc_array($first = '') {
  $ccs = cc_list2();
  $result = [];
  if($first) $result[''] = $first;
  foreach($ccs as $code => $ar) $result[$code] = $ar[0];
  return $result;
}

function cc_list2() {
//country","country code","starting number","regexp country","regexp mobile number'),
// idd prefixes: US 011, Japan 010, Autralia 0011, all others 00
$ccArray = array(
  "AF" => array("Afghanistan","9","(00|011|010|0011|\+)?93[0-9]{0,14}"),
  "AL" => array("Albania","3","(00|011|010|0011|\+)?355[0-9]{0,14}","((00|011|010|0011|\+)?35568[0-9]{0,14}|(00|011|010|0011|\+)?35569[0-9]{0,14})"),
  "DZ" => array("Algeria","2","(00|011|010|0011|\+)?213[0-9]{0,14}","(00|011|010|0011|\+)?21361[0-9]{0,14}"),
  "AS" => array("American Samoa","1","(00|011|010|0011|\+)?1684[0-9]{0,14}"),
  "AD" => array("Andorra","3","(00|011|010|0011|\+)?376[0-9]{0,14}","(00|011|010|0011|\+)?3763[0-9]{0,14}"),
  "AO" => array("Angola","2","(00|011|010|0011|\+)?244[0-9]{0,14}","(00|011|010|0011|\+)?2449[0-9]{0,14}"),
  "AI" => array("Anguilla","1","(00|011|010|0011|\+)?1264[0-9]{0,14}"),
  "AQ" => array("Antarctica","6","(00|011|010|0011|\+)?6721[0-9]{0,14}"),
  "AG" => array("Antigua and Barbuda","1","(00|011|010|0011|\+)?1268[0-9]{0,14}"),
  "AR" => array("Argentina","5","(00|011|010|0011|\+)?54[0-9]{0,14}","(00|011|010|0011|\+)?5415[0-9]{0,14}"),
  "AM" => array("Armenia","3","(00|011|010|0011|\+)?374[0-9]{0,14}","(00|011|010|0011|\+)?3749[0-9]{0,14}"),
  "AW" => array("Aruba","2","(00|011|010|0011|\+)?297[0-9]{0,14}","((00|011|010|0011|\+)?29773[0-9]{0,14}|(00|011|010|0011|\+)?29774[0-9]{0,14})"),
  "AC" => array("Ascension","2","(00|011|010|0011|\+)?247[0-9]{0,14}"),
  "AU" => array("Australia","6","(00|011|010|0011|\+)?61[0-9]{0,14}","(00|011|010|0011|\+)?614[0-9]{0,14}"),
  "AT" => array("Austria","4","(00|011|010|0011|\+)?43[0-9]{0,14}","((00|011|010|0011|\+)?43664[0-9]{0,14}|(00|011|010|0011|\+)?43660[0-9]{0,14}|(00|011|010|0011|\+)?43676[0-9]{0,14}|(00|011|010|0011|\+)?43699[0-9]{0,14}|(00|011|010|0011|\+)?43650[0-9]{0,14}|(00|011|010|0011|\+)?43680[0-9]{0,14}|(00|011|010|0011|\+)?43681[0-9]{0,14}|(00|011|010|0011|\+)?43688[0-9]{0,14})"),
  "AZ" => array("Azerbaijan","9","(00|011|010|0011|\+)?994[0-9]{0,14}"),
  "BS" => array("Bahamas","1","(00|011|010|0011|\+)?1242[0-9]{0,14}","((00|011|010|0011|\+)?124235[0-9]{0,14}|(00|011|010|0011|\+)?124245[0-9]{0,14}|(00|011|010|0011|\+)?124255[0-9]{0,14})"),
  "BH" => array("Bahrain","9","(00|011|010|0011|\+)?973[0-9]{0,14}","(00|011|010|0011|\+)?9733[0-9]{0,14}"),
  "BD" => array("Bangladesh","8","(00|011|010|0011|\+)?880[0-9]{0,14}"),
  "BB" => array("Barbados","1","(00|011|010|0011|\+)?1246[0-9]{0,14}"),
  "BY" => array("Belarus","3","(00|011|010|0011|\+)?375[0-9]{0,14}"),
  "BE" => array("Belgium","3","(00|011|010|0011|\+)?32[0-9]{0,14}","(00|011|010|0011|\+)?324[0-9]{8}"),
  "BZ" => array("Belize","5","(00|011|010|0011|\+)?501[0-9]{0,14}"),
  "BJ" => array("Benin","2","(00|011|010|0011|\+)?229[0-9]{0,14}"),
  "BM" => array("Bermuda","1","(00|011|010|0011|\+)?1441[0-9]{0,14}"),
  "BT" => array("Bhutan","9","(00|011|010|0011|\+)?975[0-9]{0,14}"),
  "BO" => array("Bolivia","5","(00|011|010|0011|\+)?591[0-9]{0,14}"),
  "BA" => array("Bosnia and Herzegovina","3","(00|011|010|0011|\+)?387[0-9]{0,14}"),
  "BW" => array("Botswana","2","(00|011|010|0011|\+)?267[0-9]{0,14}","(00|011|010|0011|\+)?2677[0-9]{0,14}"),
  "BR" => array("Brazil","5","(00|011|010|0011|\+)?55[0-9]{0,14}","((00|011|010|0011|\+)?557[0-9]{0,14}|(00|011|010|0011|\+)?558[0-9]{0,14}|(00|011|010|0011|\+)?559[0-9]{0,14})"),
  "BN" => array("Brunei","6","(00|011|010|0011|\+)?673[0-9]{0,14}"),
  "BG" => array("Bulgaria","3","(00|011|010|0011|\+)?359[0-9]{0,14}","((00|011|010|0011|\+)?35998[0-9]{0,14}|(00|011|010|0011|\+)?35999[0-9]{0,14}|(00|011|010|0011|\+)?35948[0-9]{0,14}|(00|011|010|0011|\+)?35988[0-9]{0,14}|0035987[0-9]{0,14}|(00|011|010|0011|\+)?35989[0-9]{0,14})"),
  "BF" => array("Burkina Faso","2","(00|011|010|0011|\+)?226[0-9]{0,14}"),
  "BI" => array("Burundi","2","(00|011|010|0011|\+)?257[0-9]{0,14}","((00|011|010|0011|\+)?25729[0-9]{0,14}|(00|011|010|0011|\+)?2577[0-9]{0,14})"),
  "KH" => array("Cambodia","8","(00|011|010|0011|\+)?855[0-9]{0,14}"),
  "CM" => array("Cameroon","2","(00|011|010|0011|\+)?237[0-9]{0,14}","((00|011|010|0011|\+)?2376[0-9]{0,14}|(00|011|010|0011|\+)?2377[0-9]{0,14}|(00|011|010|0011|\+)?2378[0-9]{0,14}|(00|011|010|0011|\+)?2379[0-9]{0,14})"),
  "CA" => array("Canada","1","(00|011|010|0011|\+)?1[0-9]{0,14}"),
  "CV" => array("Cape Verde","2","(00|011|010|0011|\+)?238[0-9]{0,14}"),
  "KY" => array("Cayman Islands","1","(00|011|010|0011|\+)?1345[0-9]{0,14}"),
  "CF" => array("Central African Republic","2","(00|011|010|0011|\+)?236[0-9]{0,14}"),
  "TD" => array("Chad","2","(00|011|010|0011|\+)?235[0-9]{0,14}"),
  "CL" => array("Chile","5","(00|011|010|0011|\+)?56[0-9]{0,14}","((00|011|010|0011|\+)?568[0-9]{0,14}|(00|011|010|0011|\+)?569[0-9]{0,14})"),
  "CN" => array("China","8","(00|011|010|0011|\+)?86[0-9]{0,14}"),
  "CX" => array("Christmas Island","6","(00|011|010|0011|\+)?618[0-9]{0,14}"),
  "CC" => array("Cocos (Keeling) Islands","6","(00|011|010|0011|\+)?618[0-9]{0,14}"),
  "CO" => array("Colombia","5","(00|011|010|0011|\+)?57[0-9]{0,14}","(00|011|010|0011|\+)?573[0-9]{0,14}"),
  "KM" => array("Comoros","2","(00|011|010|0011|\+)?269[0-9]{0,14}"),
  "CG" => array("Congo","2","((00|011|010|0011|\+)?243[0-9]{0,14}|(00|011|010|0011|\+)?242[0-9]{0,14})"),
  "CK" => array("Cook Islands","6","(00|011|010|0011|\+)?682[0-9]{0,14}"),
  "CR" => array("Costa Rica","5","(00|011|010|0011|\+)?506[0-9]{0,14}"),
  "CI" => array("Côte d'Ivoire","2","(00|011|010|0011|\+)?225[0-9]{0,14}"),
  "HR" => array("Croatia","3","(00|011|010|0011|\+)?385[0-9]{0,14}","(00|011|010|0011|\+)?3859[0-9]{0,14}"),
  "CU" => array("Cuba","5","(00|011|010|0011|\+)?53[0-9]{0,14}"),
  "CY" => array("Cyprus","3","(00|011|010|0011|\+)?357[0-9]{0,14}","((00|011|010|0011|\+)?35799[0-9]{0,14}|*35796[0-9]{0,14}|(00|011|010|0011|\+)?35797[0-9]{0,14})"),
  "CZ" => array("Czech Republic","4","(00|011|010|0011|\+)?420[0-9]{0,14}","((00|011|010|0011|\+)?4206[0-9]{0,14}|(00|011|010|0011|\+)?4207[0-9]{0,14})"),
  "DK" => array("Denmark","4","(00|011|010|0011|\+)?45[0-9]{0,14}","((00|011|010|0011|\+)?4530[0-9]{0,14}|(00|011|010|0011|\+)?4540[0-9]{0,14})"),
  "DG" => array("Diego Garcia","2","(00|011|010|0011|\+)?246[0-9]{0,14}"),
  "DJ" => array("Djibouti","2","(00|011|010|0011|\+)?253[0-9]{0,14}"),
  "DM" => array("Dominica","1","(00|011|010|0011|\+)?1767[0-9]{0,14}"),
  "DO" => array("Dominican Republic","1","((00|011|010|0011|\+)?1809|(00|011|010|0011|\+)?1829[0-9]{0,14})"),
  "TL" => array("East Timor","6","(00|011|010|0011|\+)?670[0-9]{0,14}"),
  "EC" => array("Ecuador","5","(00|011|010|0011|\+)?593[0-9]{0,14}","((00|011|010|0011|\+)?5939[0-9]{0,14}|(00|011|010|0011|\+)?5938[0-9]{0,14})"),
  "EG" => array("Egypt","2","(00|011|010|0011|\+)?20[0-9]{0,14}"),
  "SV" => array("El Salvador","5","(00|011|010|0011|\+)?503[0-9]{0,14}"),
  "GQ" => array("Equatorial Guinea","2","(00|011|010|0011|\+)?240[0-9]{0,14}"),
  "ER" => array("Eritrea","2","(00|011|010|0011|\+)?291[0-9]{0,14}"),
  "EE" => array("Estonia","3","(00|011|010|0011|\+)?372[0-9]{0,14}","(00|011|010|0011|\+)?3725[0-9]{0,14}"),
  "ET" => array("Ethiopia","2","(00|011|010|0011|\+)?251[0-9]{0,14}"),
  "FO" => array("Faeroe Islands","5","(00|011|010|0011|\+)?500[0-9]{0,14}"),
  "FK" => array("Falkland Islands","2","(00|011|010|0011|\+)?298[0-9]{0,14}"),
  "FJ" => array("Fiji","6","(00|011|010|0011|\+)?679[0-9]{0,14}"),
  "FI" => array("Finland","3","(00|011|010|0011|\+)?358[0-9]{0,14}","((00|011|010|0011|\+)?3584[0-9]{0,14}|(00|011|010|0011|\+)?35850[0-9]{0,14})"),
  "FR" => array("France","3","(00|011|010|0011|\+)?33[0-9]{0,14}","((00|011|010|0011|\+)?336[0-9]{0,14}|(00|011|010|0011|\+)?337[0-9]{0,14})"),
  "GF" => array("French Guiana","5","(00|011|010|0011|\+)?594[0-9]{0,14}","((00|011|010|0011|\+)?594694[0-9]{0,14}|[0-9]{0,14})"),
  "PF" => array("French Polynesia","6","(00|011|010|0011|\+)?689[0-9]{0,14}"),
  "GA" => array("Gabon","2","(00|011|010|0011|\+)?241[0-9]{0,14}"),
  "GM" => array("Gambia","2","(00|011|010|0011|\+)?220[0-9]{0,14}"),
  "GE" => array("Georgia","9","(00|011|010|0011|\+)?995[0-9]{0,14}","((00|011|010|0011|\+)?9957[0-9]{0,14}|(00|011|010|0011|\+)?9959[0-9]{0,14})"),
  "DE" => array("Germany","4","(00|011|010|0011|\+)?49[0-9]{0,14}","((00|011|010|0011|\+)?491[0-9]{0,14}|(00|011|010|0011|\+)?49700[0-9]{0,14})"),
  "GH" => array("Ghana","2","(00|011|010|0011|\+)?233[0-9]{0,14}"),
  "GI" => array("Gibraltar","3","(00|011|010|0011|\+)?350[0-9]{0,14}"),
  "GR" => array("Greece","3","(00|011|010|0011|\+)?30[0-9]{0,14}","(00|011|010|0011|\+)?3069[0-9]{0,14}"),
  "GL" => array("Greenland","2","(00|011|010|0011|\+)?299[0-9]{0,14}"),
  "GD" => array("Grenada","1","(00|011|010|0011|\+)?1473[0-9]{0,14}"),
  "GP" => array("Guadeloupe","5","(00|011|010|0011|\+)?590[0-9]{0,14}","(00|011|010|0011|\+)?5906[0-9]{0,14}"),
  "GU" => array("Guam","1","(00|011|010|0011|\+)?1671[0-9]{0,14}"),
  "GT" => array("Guatemala","5","(00|011|010|0011|\+)?502[0-9]{0,14}"),
  "GN" => array("Guinea","2","(00|011|010|0011|\+)?224[0-9]{0,14}"),
  "GW" => array("Guinea-Bissau","2","(00|011|010|0011|\+)?245[0-9]{0,14}"),
  "GY" => array("Guyana","5","(00|011|010|0011|\+)?592[0-9]{0,14}"),
  "HT" => array("Haiti","5","(00|011|010|0011|\+)?509[0-9]{0,14}"),
  "HN" => array("Honduras","5","(00|011|010|0011|\+)?504[0-9]{0,14}"),
  "HK" => array("Hong Kong","8","(00|011|010|0011|\+)?852[0-9]{0,14}"),
  "HU" => array("Hungary","3","(00|011|010|0011|\+)?36[0-9]{0,14}","((00|011|010|0011|\+)?3620[0-9]{0,14}|(00|011|010|0011|\+)?3670[0-9]{0,14}|(00|011|010|0011|\+)?3630[0-9]{0,14})"),
  "IS" => array("Iceland","3","(00|011|010|0011|\+)?354[0-9]{0,14}","((00|011|010|0011|\+)?3543[0-9]{0,14}|(00|011|010|0011|\+)?3546[0-9]{0,14}|(00|011|010|0011|\+)?357[0-9]{0,14}|(00|011|010|0011|\+)?358[0-9]{0,14})"),
  "IN" => array("India","9","(00|011|010|0011|\+)?91[0-9]{0,14}","(00|011|010|0011|\+)?919[0-9]{0,14}"),
  "ID" => array("Indonesia","6","(00|011|010|0011|\+)?62[0-9]{0,14}"),
  "IR" => array("Iran","9","(00|011|010|0011|\+)?98[0-9]{0,14}"),
  "IQ" => array("Iraq","9","(00|011|010|0011|\+)?964[0-9]{0,14}"),
  "IE" => array("Ireland","3","(00|011|010|0011|\+)?353[0-9]{0,14}","(00|011|010|0011|\+)?3538[0-9]{0,14}"),
  "IL" => array("Israel","9","(00|011|010|0011|\+)?972[0-9]{0,14}","(00|011|010|0011|\+)?9725[0-9]{0,14}"),
  "IT" => array("Italy","3","(00|011|010|0011|\+)?39[0-9]{0,14}","(00|011|010|0011|\+)?393[0-9]{0,14}"),
  "JM" => array("Jamaica","1","(00|011|010|0011|\+)?1876[0-9]{0,14}"),
  "JP" => array("Japan","8","(00|011|010|0011|\+)?81[0-9]{0,14}","((00|011|010|0011|\+)?810[0-9]{0,14}|(00|011|010|0011|\+)?8170[0-9]{0,14}|(00|011|010|0011|\+)?8180[0-9]{0,14}|(00|011|010|0011|\+)?8190[0-9]{0,14})"),
  "JO" => array("Jordan","9","(00|011|010|0011|\+)?962[0-9]{0,14}","(00|011|010|0011|\+)?9627[0-9]{0,14}"),
  "KZ" => array("Kazakhstan","7","(00|011|010|0011|\+)?77[0-9]{0,14}"),
  "KE" => array("Kenya","2","(00|011|010|0011|\+)?254[0-9]{0,14}"),
  "KI" => array("Kiribati","6","(00|011|010|0011|\+)?686[0-9]{0,14}"),
  "KP" => array("Korea (North)","8","(00|011|010|0011|\+)?850[0-9]{0,14}"),
  "KR" => array("Korea (South)","8","(00|011|010|0011|\+)?82[0-9]{0,14}","(00|011|010|0011|\+)?821[0-9]{0,14}"),
  "KW" => array("Kuwait","9","(00|011|010|0011|\+)?965[0-9]{0,14}"),
  "KG" => array("Kyrgyzstan","9","(00|011|010|0011|\+)?996[0-9]{0,14}"),
  "LA" => array("Laos","8","(00|011|010|0011|\+)?856[0-9]{0,14}"),
  "LV" => array("Latvia","3","(00|011|010|0011|\+)?371[0-9]{0,14}","(00|011|010|0011|\+)?3712[0-9]{0,14}"),
  "LB" => array("Lebanon","9","(00|011|010|0011|\+)?961[0-9]{0,14}","((00|011|010|0011|\+)?96170[0-9]{0,14}|(00|011|010|0011|\+)?96171[0-9]{0,14}|(00|011|010|0011|\+)?9613[0-9]{0,14})"),
  "LS" => array("Lesotho","2","(00|011|010|0011|\+)?266[0-9]{0,14}"),
  "LR" => array("Liberia","2","(00|011|010|0011|\+)?231[0-9]{0,14}"),
  "LY" => array("Libya","2","(00|011|010|0011|\+)?218[0-9]{0,14}"),
  "LI" => array("Liechtenstein","4","(00|011|010|0011|\+)?423[0-9]{0,14}","((00|011|010|0011|\+)?4236[0-9]{0,14}|(00|011|010|0011|\+)?4237[0-9]{0,14})"),
  "LT" => array("Lithuania","3","(00|011|010|0011|\+)?370[0-9]{0,14}","(00|011|010|0011|\+)?3706[0-9]{0,14}"),
  "LU" => array("Luxembourg ","3","(00|011|010|0011|\+)?352[0-9]{0,14}","(00|011|010|0011|\+)?3526[0-9]{0,14}"),
  "MO" => array("Macau","8","(00|011|010|0011|\+)?853[0-9]{0,14}","(00|011|010|0011|\+)?8536[0-9]{0,14}"),
  "MK" => array("Macedonia","3","(00|011|010|0011|\+)?389[0-9]{0,14}","(00|011|010|0011|\+)?3897[0-9]{0,14}"),
  "MG" => array("Madagascar","2","(00|011|010|0011|\+)?261[0-9]{0,14}"),
  "MW" => array("Malawi","2","(00|011|010|0011|\+)?265[0-9]{0,14}","((00|011|010|0011|\+)?2653[0-9]{0,14}|(00|011|010|0011|\+)?2654[0-9]{0,14}|(00|011|010|0011|\+)?2655[0-9]{0,14}|(00|011|010|0011|\+)?2657[0-9]{0,14}|(00|011|010|0011|\+)?2658[0-9]{0,14}|(00|011|010|0011|\+)?2659[0-9]{0,14})"),
  "MY" => array("Malaysia","6","(00|011|010|0011|\+)?60[0-9]{0,14}","(00|011|010|0011|\+)?601[0-9]{0,14}"),
  "MV" => array("Maldives","9","(00|011|010|0011|\+)?960[0-9]{0,14}","((00|011|010|0011|\+)?96096[0-9]{0,14}|(00|011|010|0011|\+)?96097[0-9]{0,14}|(00|011|010|0011|\+)?96098(00|011|010|0011|\+)?96099[0-9]{0,14})"),
  "ML" => array("Mali","2","(00|011|010|0011|\+)?223[0-9]{0,14}"),
  "MT" => array("Malta","3","(00|011|010|0011|\+)?356[0-9]{0,14}","((00|011|010|0011|\+)?3567[0-9]{0,14}|(00|011|010|0011|\+)?3569[0-9]{0,14})"),
  "MH" => array("Marshall Islands","6","(00|011|010|0011|\+)?692[0-9]{0,14}"),
  "MQ" => array("Martinique","5","(00|011|010|0011|\+)?596[0-9]{0,14}","(00|011|010|0011|\+)?5966[0-9]{0,14}"),
  "MR" => array("Mauritania","2","(00|011|010|0011|\+)?222[0-9]{0,14}"),
  "MU" => array("Mauritius","2","(00|011|010|0011|\+)?230[0-9]{0,14}"),
  "YT" => array("Mayotte","5","(00|011|010|0011|\+)?52[0-9]{0,14}","(00|011|010|0011|\+)?521[0-9]{0,14}"),
  "MX" => array("Mexico","6","(00|011|010|0011|\+)?691[0-9]{0,14}"),
  "FM" => array("Micronesia","3","(00|011|010|0011|\+)?373[0-9]{0,14}"),
  "MD" => array("Moldova","3","(00|011|010|0011|\+)?377[0-9]{0,14}","((00|011|010|0011|\+)?3774[0-9]{0,14}|(00|011|010|0011|\+)?3776[0-9]{0,14})"),
  "MC" => array("Monaco","9","(00|011|010|0011|\+)?976[0-9]{0,14}"),
  "MN" => array("Mongolia","3","(00|011|010|0011|\+)?382[0-9]{0,14}"),
  "MS" => array("Montserrat","1","(00|011|010|0011|\+)?1664[0-9]{0,14}","((00|011|010|0011|\+)?1664349[0-9]{0,14}|(00|011|010|0011|\+)?1664492[0-9]{0,14}|(00|011|010|0011|\+)?1664493[0-9]{0,14}|(00|011|010|0011|\+)?1664495[0-9]{0,14}|(00|011|010|0011|\+)?1664496[0-9]{0,14})"),
  "MA" => array("Morocco","2","(00|011|010|0011|\+)?212[0-9]{0,14}","(00|011|010|0011|\+)?2126[0-9]{0,14}"),
  "MZ" => array("Mozambique","2","(00|011|010|0011|\+)?258[0-9]{0,14}","(00|011|010|0011|\+)?2588[0-9]{0,14}"),
  "MM" => array("Myanmar","9","(00|011|010|0011|\+)?95[0-9]{0,14}"),
  "NA" => array("Namibia","2","(00|011|010|0011|\+)?264[0-9]{0,14}","(00|011|010|0011|\+)?2648[0-9]{0,14}"),
  "NR" => array("Nauru","6","(00|011|010|0011|\+)?674[0-9]{0,14}"),
  "NP" => array("Nepal","9","(00|011|010|0011|\+)?977[0-9]{0,14}"),
  "NL" => array("Netherlands","3","(00|011|010|0011|\+)?31[0-9]{0,14}","(00|011|010|0011|\+)?316[0-9]{0,14}"),
  "AN" => array("Netherlands Antilles","5","(00|011|010|0011|\+)?599[0-9]{0,14}"),
  "NC" => array("New Caledonia","6","(00|011|010|0011|\+)?687[0-9]{0,14}"),
  "NZ" => array("New Zealand","6","(00|011|010|0011|\+)?64[0-9]{0,14}","(00|011|010|0011|\+)?642[0-9]{0,14}"),
  "NI" => array("Nicaragua","5","(00|011|010|0011|\+)?505[0-9]{0,14}"),
  "NE" => array("Niger","2","(00|011|010|0011|\+)?227[0-9]{0,14}"),
  "NG" => array("Nigeria","2","(00|011|010|0011|\+)?234[0-9]{0,14}"),
  "NU" => array("Niue","6","(00|011|010|0011|\+)?683[0-9]{0,14}"),
  "NF" => array("Norfolk Island","6","(00|011|010|0011|\+)?6723[0-9]{0,14}"),
  "MP" => array("Northern Marianas","1","(00|011|010|0011|\+)?1670[0-9]{0,14}"),
  "NO" => array("Norway","4","(00|011|010|0011|\+)?47[0-9]{0,14}","((00|011|010|0011|\+)?474[0-9]{0,14}|(00|011|010|0011|\+)?479[0-9]{0,14})"),
  "OM" => array("Oman","9","(00|011|010|0011|\+)?968[0-9]{0,14}","(00|011|010|0011|\+)?9689[0-9]{0,14}"),
  "PK" => array("Pakistan","9","(00|011|010|0011|\+)?92[0-9]{0,14}","(00|011|010|0011|\+)?923[0-9]{0,14}"),
  "PW" => array("Palau","6","(00|011|010|0011|\+)?680[0-9]{0,14}"),
  "PS" => array("Palestinian Settlements","9","(00|011|010|0011|\+)?970[0-9]{0,14}"),
  "PA" => array("Panama","5","(00|011|010|0011|\+)?507[0-9]{0,14}","((00|011|010|0011|\+)?507648[0-9]{0,14}|(00|011|010|0011|\+)?507649[0-9]{0,14}|(00|011|010|0011|\+)?67565[0-9]{0,14}|(00|011|010|0011|\+)?67566[0-9]{0,14})"),
  "PG" => array("Papua New Guinea","6","(00|011|010|0011|\+)?675[0-9]{0,14}"),
  "PY" => array("Paraguay","5","(00|011|010|0011|\+)?595[0-9]{0,14}","(00|011|010|0011|\+)?5959[0-9]{0,14}"),
  "PE" => array("Peru","5","(00|011|010|0011|\+)?51[0-9]{0,14}","((00|011|010|0011|\+)?51941[0-9]{0,14}|(00|011|010|0011|\+)?5195[0-9]{0,14}|(00|011|010|0011|\+)?5195[0-9]{0,14}|(00|011|010|0011|\+)?5196[0-9]{0,14}|(00|011|010|0011|\+)?5197[0-9]{0,14}|(00|011|010|0011|\+)?5198[0-9]{0,14}|(00|011|010|0011|\+)?51949[0-9]{0,14}|(00|011|010|0011|\+)?5199[0-9]{0,14}|(00|011|010|0011|\+)?51942[0-9]{0,14})"),
  "PH" => array("Philippines","6","(00|011|010|0011|\+)?63[0-9]{0,14}"),
  "PL" => array("Poland","4","(00|011|010|0011|\+)?48[0-9]{0,14}","((00|011|010|0011|\+)?4850[0-9]{0,14}| (00|011|010|0011|\+)?4851[0-9]{0,14}|(00|011|010|0011|\+)?4860[0-9]{0,14}|(00|011|010|0011|\+)?4866[0-9]{0,14}|(00|011|010|0011|\+)?4869[0-9]{0,14}|(00|011|010|0011|\+)?4872[0-9]{0,14}|(00|011|010|0011|\+)?4878[0-9]{0,14}|(00|011|010|0011|\+)?4879[0-9]{0,14})"),
  "PT" => array("Portugal","3","(00|011|010|0011|\+)?351[0-9]{0,14}","(00|011|010|0011|\+)?3511[0-9]{0,14}"),
  "PR" => array("Puerto Rico","1","((00|011|010|0011|\+)?1787|(00|011|010|0011|\+)?1939[0-9]{0,14})"),
  "QA" => array("Qatar","9","(00|011|010|0011|\+)?974[0-9]{0,14}"),
  "RE" => array("Réunion","2","(00|011|010|0011|\+)?262[0-9]{0,14}","((00|011|010|0011|\+)?26269[0-9]{0,14}|(00|011|010|0011|\+)?26220[0-9]{0,14}|(00|011|010|0011|\+)?26221[0-9]{0,14}|(00|011|010|0011|\+)?2623[0-9]{0,14}|(00|011|010|0011|\+)?26265[0-9]{0,14}|(00|011|010|0011|\+)?26266[0-9]{0,14}|(00|011|010|0011|\+)?26267[0-9]{0,14}|(00|011|010|0011|\+)?26268[0-9]{0,14})"),
  "RO" => array("Romania","4","(00|011|010|0011|\+)?40[0-9]{0,14}","(00|011|010|0011|\+)?407[0-9]{0,14}"),
  "RU" => array("Russia","7","(00|011|010|0011|\+)?7[0-9]{0,14}","((00|011|010|0011|\+)?7407[0-9]{0,14}|(00|011|010|0011|\+)?7810[0-9]{0,14}|(00|011|010|0011|\+)?79[0-9]{0,14})"),
  "RW" => array("Rwanda","2","(00|011|010|0011|\+)?250[0-9]{0,14}"),
  "SH" => array("Saint Helena","2","(00|011|010|0011|\+)?290[0-9]{0,14}"),
  "KN" => array("Saint Kitts and Nevis","1","(00|011|010|0011|\+)?1869[0-9]{0,14}"),
  "LC" => array("Saint Lucia","1","(00|011|010|0011|\+)?1758[0-9]{0,14}"),
  "PM" => array("Saint Pierre and Miquelon","5","(00|011|010|0011|\+)?508[0-9]{0,14}"),
  "VC" => array("Saint Vincent and Grenadines","1","(00|011|010|0011|\+)?1784[0-9]{0,14}"),
  "WS" => array("Samoa","6","(00|011|010|0011|\+)?685[0-9]{0,14}"),
  "SM" => array("San Marino","3","(00|011|010|0011|\+)?378[0-9]{0,14}"),
  "ST" => array("São Tomé and Príncipe","2","(00|011|010|0011|\+)?239[0-9]{0,14}"),
  "SA" => array("Saudi Arabia","9","(00|011|010|0011|\+)?966[0-9]{0,14}"),
  "SN" => array("Senegal","2","(00|011|010|0011|\+)?221[0-9]{0,14}"),
  "RS" => array("Serbia","3","(00|011|010|0011|\+)?381[0-9]{0,14}","(00|011|010|0011|\+)?3816[0-9]{0,14}"),
  "SC" => array("Seychelles","2","(00|011|010|0011|\+)?248[0-9]{0,14}","((00|011|010|0011|\+)?24851[0-9]{0,14}|(00|011|010|0011|\+)?24852[0-9]{0,14}|(00|011|010|0011|\+)?24853[0-9]{0,14}|(00|011|010|0011|\+)?24854[0-9]{0,14}|(00|011|010|0011|\+)?24855[0-9]{0,14}|(00|011|010|0011|\+)?24856[0-9]{0,14}|(00|011|010|0011|\+)?24857[0-9]{0,14}|(00|011|010|0011|\+)?24858[0-9]{0,14}|(00|011|010|0011|\+)?24859[0-9]{0,14}|(00|011|010|0011|\+)?2487[0-9]{0,14})"),
  "SL" => array("Sierra Leone","2","(00|011|010|0011|\+)?232[0-9]{0,14}","((00|011|010|0011|\+)?23223[0-9]{0,14}|(00|011|010|0011|\+)?23230[0-9]{0,14}|(00|011|010|0011|\+)?23233[0-9]{0,14}|(00|011|010|0011|\+)?23235[0-9]{0,14}|(00|011|010|0011|\+)?23240[0-9]{0,14}|(00|011|010|0011|\+)?23250[0-9]{0,14}|(00|011|010|0011|\+)?23276[0-9]{0,14}|(00|011|010|0011|\+)?23277[0-9]{0,14})"),
  "SG" => array("Singapore","6","(00|011|010|0011|\+)?65[0-9]{0,14}","((00|011|010|0011|\+)?658[0-9]{0,14}|(00|011|010|0011|\+)?659[0-9]{0,14})"),
  "SK" => array("Slovakia","4","(00|011|010|0011|\+)?421[0-9]{0,14}","(00|011|010|0011|\+)?4219[0-9]{0,14}"),
  "SI" => array("Slovenia","3","(00|011|010|0011|\+)?386[0-9]{0,14}","((00|011|010|0011|\+)?38630[0-9]{0,14}|(00|011|010|0011|\+)?38631[0-9]{0,14}|(00|011|010|0011|\+)?38640[0-9]{0,14}|(00|011|010|0011|\+)?38641[0-9]{0,14}|(00|011|010|0011|\+)?38649[0-9]{0,14}|(00|011|010|0011|\+)?38651[0-9]{0,14}|(00|011|010|0011|\+)?38664[0-9]{0,14}|(00|011|010|0011|\+)?38665[0-9]{0,14}|(00|011|010|0011|\+)?38666[0-9]{0,14}|(00|011|010|0011|\+)?38667[0-9]{0,14}|(00|011|010|0011|\+)?38668[0-9]{0,14}|(00|011|010|0011|\+)?38669[0-9]{0,14}|(00|011|010|0011|\+)?38670[0-9]{0,14}|(00|011|010|0011|\+)?38671[0-9]{0,14})"),
  "SB" => array("Solomon Islands","6","(00|011|010|0011|\+)?677[0-9]{0,14}"),
  "SO" => array("Somalia","2","(00|011|010|0011|\+)?252[0-9]{0,14}"),
  "ZA" => array("South Africa","2","(00|011|010|0011|\+)?27[0-9]{0,14}","((00|011|010|0011|\+)?2772[0-9]{0,14}|(00|011|010|0011|\+)?2773[0-9]{0,14}|(00|011|010|0011|\+)?2774[0-9]{0,14}|(00|011|010|0011|\+)?2776[0-9]{0,14}|(00|011|010|0011|\+)?2778[0-9]{0,14}|(00|011|010|0011|\+)?2782[0-9]{0,14}|(00|011|010|0011|\+)?2783[0-9]{0,14}|(00|011|010|0011|\+)?2784[0-9]{0,14})"),
  "ES" => array("Spain","3","(00|011|010|0011|\+)?34[0-9]{0,14}","(00|011|010|0011|\+)?346[0-9]{0,14}"),
  "LK" => array("Sri Lanka","9","(00|011|010|0011|\+)?94[0-9]{0,14}"),
  "SD" => array("Sudan","2","(00|011|010|0011|\+)?249[0-9]{0,14}"),
  "SR" => array("Suriname","5","(00|011|010|0011|\+)?597[0-9]{0,14}"),
  "SZ" => array("Swaziland","2","(00|011|010|0011|\+)?268[0-9]{0,14}","(00|011|010|0011|\+)?2686[0-9]{0,14}"),
  "SE" => array("Sweden","4","(00|011|010|0011|\+)?46[0-9]{0,14}","((00|011|010|0011|\+)?4610[0-9]{0,14}|(00|011|010|0011|\+)?4670[0-9]{0,14}|(00|011|010|0011|\+)?4672[0-9]{0,14}|(00|011|010|0011|\+)?4673[0-9]{0,14}|(00|011|010|0011|\+)?4676[0-9]{0,14})"),
  "CH" => array("Switzerland","4","(00|011|010|0011|\+)?41[0-9]{0,14}","((00|011|010|0011|\+)?4176[0-9]{0,14}|(00|011|010|0011|\+)?4177[0-9]{0,14}|(00|011|010|0011|\+)?4178[0-9]{0,14}|(00|011|010|0011|\+)?4179[0-9]{0,14})"),
  "SY" => array("Syria","9","(00|011|010|0011|\+)?963[0-9]{0,14}"),
  "TW" => array("Taiwan","8","(00|011|010|0011|\+)?886[0-9]{0,14}","(00|011|010|0011|\+)?8669[0-9]{0,14}"),
  "TJ" => array("Tajikistan","9","(00|011|010|0011|\+)?992[0-9]{0,14}","(00|011|010|0011|\+)?9929[0-9]{0,14}"),
  "TZ" => array("Tanzania","2","(00|011|010|0011|\+)?255[0-9]{0,14}"),
  "TH" => array("Thailand","6","(00|011|010|0011|\+)?66[0-9]{0,14}","(00|011|010|0011|\+)?668[0-9]{0,14}"),
  "TG" => array("Togo","2","(00|011|010|0011|\+)?228[0-9]{0,14}"),
  "TK" => array("Tokelau","6","(00|011|010|0011|\+)?690[0-9]{0,14}"),
  "TO" => array("Tonga","6","(00|011|010|0011|\+)?676[0-9]{0,14}"),
  "TT" => array("Trinidad and Tobago","1","(00|011|010|0011|\+)?1868[0-9]{0,14}"),
  "TN" => array("Tunisia","2","(00|011|010|0011|\+)?216[0-9]{0,14}"),
  "TR" => array("Turkey","9","(00|011|010|0011|\+)?90[0-9]{0,14}","(00|011|010|0011|\+)?905[0-9]{0,14}"),
  "TM" => array("Turkmenistan","9","(00|011|010|0011|\+)?993[0-9]{0,14}"),
  "TC" => array("Turks and Caicos Islands","1","(00|011|010|0011|\+)?1649[0-9]{0,14}"),
  "TV" => array("Tuvalu","6","(00|011|010|0011|\+)?688[0-9]{0,14}"),
  "UG" => array("Uganda","2","(00|011|010|0011|\+)?256[0-9]{0,14}","(00|011|010|0011|\+)?2567[0-9]{0,14}"),
  "UA" => array("Ukraine","3","(00|011|010|0011|\+)?380[0-9]{0,14}"),
  "AE" => array("United Arab Emirates","9","(00|011|010|0011|\+)?971[0-9]{0,14}"),
  "GB" => array("United Kingdom","4","(00|011|010|0011|\+)?44[0-9]{0,14}","((00|011|010|0011|\+)?4475[0-9]{0,14}|(00|011|010|0011|\+)?4477[0-9]{0,14}|(00|011|010|0011|\+)?4478[0-9]{0,14}|(00|011|010|0011|\+)?4479[0-9]{0,14}|[0-9]{0,14})"),
  "US" => array("United States","1","(00|011|010|0011|\+)?1[0-9]{0,14}"),
  "UY" => array("Uruguay","5","(00|011|010|0011|\+)?598[0-9]{0,14}","(00|011|010|0011|\+)?5989[0-9]{0,14}"),
  "VI" => array("US Virgin Islands","1","(00|011|010|0011|\+)?1340[0-9]{0,14}"),
  "UZ" => array("Uzbekistan","9","(00|011|010|0011|\+)?998[0-9]{0,14}"),
  "VU" => array("Vanuatu","6","(00|011|010|0011|\+)?678[0-9]{0,14}"),
  "VE" => array("Venezuela","5","(00|011|010|0011|\+)?58[0-9]{0,14}","((00|011|010|0011|\+)?58412[0-9]{0,14}|(00|011|010|0011|\+)?58414[0-9]{0,14}|(00|011|010|0011|\+)?58415[0-9]{0,14}|(00|011|010|0011|\+)?58416[0-9]{0,14}|(00|011|010|0011|\+)?58417[0-9]{0,14}|(00|011|010|0011|\+)?58418[0-9]{0,14})"),
  "VN" => array("Vietnam","8","(00|011|010|0011|\+)?84[0-9]{0,14}"),
  "VG" => array("Virgin Islands","1","(00|011|010|0011|\+)?1284[0-9]{0,14}"),
  "WK" => array("Wake Island","8","(00|011|010|0011|\+)?808[0-9]{0,14}"),
  "WF" => array("Wallis and Futuna","6","(00|011|010|0011|\+)?681[0-9]{0,14}"),
  "YE" => array("Yemen","9","(00|011|010|0011|\+)?967[0-9]{0,14}"),
  "ZM" => array("Zambia","2","(00|011|010|0011|\+)?260[0-9]{0,14}","(00|011|010|0011|\+)?2609[0-9]{0,14}"),
  "ZW" => array("Zimbabwe","2","(00|011|010|0011|\+)?263[0-9]{0,14}","((00|011|010|0011|\+)?26311[0-9]{0,14}|(00|011|010|0011|\+)?26323[0-9]{0,14}|(00|011|010|0011|\+)?26391[0-9]{0,14}"));
return $ccArray;
}


/** unsused - automatic comment added on  */
function html_country_list() {
   $output = "
   <option value=\"AF\" data-alternative-spellings=\"AF AF  Afghanistan افغانستان\">Afghanistan</option>
   <option value=\"AX\" data-alternative-spellings=\"AX Åland Islands Aaland Aland\" data-relevancy-booster=\"0.5\">Åland Islands</option>
   <option value=\"AL\" data-alternative-spellings=\"AL Albania\">Albania</option>
   <option value=\"DZ\" data-alternative-spellings=\"DZ Algeria الجزائر\">Algeria</option>
   <option value=\"AS\" data-alternative-spellings=\"AS American Samoa\" data-relevancy-booster=\"0.5\">American Samoa</option>
   <option value=\"AD\" data-alternative-spellings=\"AD Andorra\" data-relevancy-booster=\"0.5\">Andorra</option>
   <option value=\"AO\" data-alternative-spellings=\"AO Angola\">Angola</option>
   <option value=\"AI\" data-alternative-spellings=\"AI Anguilla\" data-relevancy-booster=\"0.5\">Anguilla</option>
   <option value=\"AQ\" data-alternative-spellings=\"AQ Antarctica\" data-relevancy-booster=\"0.5\">Antarctica</option>
   <option value=\"AG\" data-alternative-spellings=\"AG Antigua And Barbuda\" data-relevancy-booster=\"0.5\">Antigua And Barbuda</option>
   <option value=\"AR\" data-alternative-spellings=\"AR Argentina\">Argentina</option>
   <option value=\"AM\" data-alternative-spellings=\"AM Armenia Հայաստան\">Armenia</option>
   <option value=\"AW\" data-alternative-spellings=\"AW Aruba\" data-relevancy-booster=\"0.5\">Aruba</option>
   <option value=\"AU\" data-alternative-spellings=\"AU Australia\" data-relevancy-booster=\"1.5\">Australia</option>
   <option value=\"AT\" data-alternative-spellings=\"AT Austria Österreich Osterreich Oesterreich \">Austria</option>
   <option value=\"AZ\" data-alternative-spellings=\"AZ Azerbaijan\">Azerbaijan</option>
   <option value=\"BS\" data-alternative-spellings=\"BS Bahamas\">Bahamas</option>
   <option value=\"BH\" data-alternative-spellings=\"BH Bahrain البحرين\">Bahrain</option>
   <option value=\"BD\" data-alternative-spellings=\"BD Bangladesh বাংলাদেশ\" data-relevancy-booster=\"2\">Bangladesh</option>
   <option value=\"BB\" data-alternative-spellings=\"BB Barbados\">Barbados</option>
   <option value=\"BY\" data-alternative-spellings=\"BY Belarus Беларусь\">Belarus</option>
   <option value=\"BE\" data-alternative-spellings=\"BE Belgium België Belgie Belgien Belgique\" data-relevancy-booster=\"1.5\">Belgium</option>
   <option value=\"BZ\" data-alternative-spellings=\"BZ Belize\">Belize</option>
   <option value=\"BJ\" data-alternative-spellings=\"BJ Benin\">Benin</option>
   <option value=\"BM\" data-alternative-spellings=\"BM Bermuda\" data-relevancy-booster=\"0.5\">Bermuda</option>
   <option value=\"BT\" data-alternative-spellings=\"BT Bhutan भूटान\">Bhutan</option>
   <option value=\"BO\" data-alternative-spellings=\"BO Bolivia\">Bolivia</option>
   <option value=\"BQ\" data-alternative-spellings=\"BQ Bonaire, Sint Eustatius and Saba\">Bonaire, Sint Eustatius and Saba</option>
   <option value=\"BA\" data-alternative-spellings=\"BA Bosnia and Herzegovina Босна и Херцеговина\">Bosnia and Herzegovina</option>
   <option value=\"BW\" data-alternative-spellings=\"BW Botswana\">Botswana</option>
   <option value=\"BV\" data-alternative-spellings=\"BV Bouvet Island\">Bouvet Island</option>
   <option value=\"BR\" data-alternative-spellings=\"BR Brazil Brasil\" data-relevancy-booster=\"2\">Brazil</option>
   <option value=\"IO\" data-alternative-spellings=\"IO British Indian Ocean Territory\">British Indian Ocean Territory</option>
   <option value=\"BN\" data-alternative-spellings=\"BN Brunei Darussalam\">Brunei Darussalam</option>
   <option value=\"BG\" data-alternative-spellings=\"BG Bulgaria България\">Bulgaria</option>
   <option value=\"BF\" data-alternative-spellings=\"BF Burkina Faso\">Burkina Faso</option>
   <option value=\"BI\" data-alternative-spellings=\"BI Burundi\">Burundi</option>
   <option value=\"KH\" data-alternative-spellings=\"KH Cambodia កម្ពុជា\">Cambodia</option>
   <option value=\"CM\" data-alternative-spellings=\"CM Cameroon\">Cameroon</option>
   <option value=\"CA\" data-alternative-spellings=\"CA Canada\" data-relevancy-booster=\"2\">Canada</option>
   <option value=\"CV\" data-alternative-spellings=\"CV Cape Verde Cabo\">Cape Verde</option>
   <option value=\"KY\" data-alternative-spellings=\"KY Cayman Islands\" data-relevancy-booster=\"0.5\">Cayman Islands</option>
   <option value=\"CF\" data-alternative-spellings=\"CF Central African Republic\">Central African Republic</option>
   <option value=\"TD\" data-alternative-spellings=\"TD Chad تشاد‎ Tchad\">Chad</option>
   <option value=\"CL\" data-alternative-spellings=\"CL Chile\">Chile</option>
   <option value=\"CN\" data-alternative-spellings=\"CN China\" data-relevancy-booster=\"3.5 Zhongguo Zhonghua Peoples Republic 中国/中华\">China</option>
   <option value=\"CX\" data-alternative-spellings=\"CX Christmas Island\" data-relevancy-booster=\"0.5\">Christmas Island</option>
   <option value=\"CC\" data-alternative-spellings=\"CC Cocos (Keeling) Islands\" data-relevancy-booster=\"0.5\">Cocos (Keeling) Islands</option>
   <option value=\"CO\" data-alternative-spellings=\"CO Colombia\">Colombia</option>
   <option value=\"KM\" data-alternative-spellings=\"KM Comoros جزر القمر\">Comoros</option>
   <option value=\"CG\" data-alternative-spellings=\"CG Congo\">Congo</option>
   <option value=\"CD\" data-alternative-spellings=\"CD Congo, the Democratic Republic of the Congo-Brazzaville Repubilika ya Kongo\">Congo, the Democratic Republic of the</option>
   <option value=\"CK\" data-alternative-spellings=\"CK Cook Islands\" data-relevancy-booster=\"0.5\">Cook Islands</option>
   <option value=\"CR\" data-alternative-spellings=\"CR Costa Rica\">Costa Rica</option>
   <option value=\"CI\" data-alternative-spellings=\"CI Côte d'Ivoire Cote dIvoire\">Côte d\"Ivoire</option>
   <option value=\"HR\" data-alternative-spellings=\"HR Croatia Hrvatska\">Croatia</option>
   <option value=\"CU\" data-alternative-spellings=\"CU Cuba\">Cuba</option>
   <option value=\"CW\" data-alternative-spellings=\"CW Curaçao Curacao\">Curaçao</option>
   <option value=\"CY\" data-alternative-spellings=\"CY Cyprus Κύπρος Kýpros Kıbrıs\">Cyprus</option>
   <option value=\"CZ\" data-alternative-spellings=\"CZ Czech Republic Česká Ceska\">Czech Republic</option>
   <option value=\"DK\" data-alternative-spellings=\"DK Denmark Danmark\" data-relevancy-booster=\"1.5\">Denmark</option>
   <option value=\"DJ\" data-alternative-spellings=\"DJ Djibouti جيبوتي‎ Jabuuti Gabuuti\">Djibouti</option>
   <option value=\"DM\" data-alternative-spellings=\"DM Dominica Dominique\" data-relevancy-booster=\"0.5\">Dominica</option>
   <option value=\"DO\" data-alternative-spellings=\"DO Dominican Republic\">Dominican Republic</option>
   <option value=\"EC\" data-alternative-spellings=\"EC Ecuador\">Ecuador</option>
   <option value=\"EG\" data-alternative-spellings=\"EG Egypt\" data-relevancy-booster=\"1.5\">Egypt</option>
   <option value=\"SV\" data-alternative-spellings=\"SV El Salvador\">El Salvador</option>
   <option value=\"GQ\" data-alternative-spellings=\"GQ Equatorial Guinea\">Equatorial Guinea</option>
   <option value=\"ER\" data-alternative-spellings=\"ER Eritrea إرتريا ኤርትራ\">Eritrea</option>
   <option value=\"EE\" data-alternative-spellings=\"EE Estonia Eesti\">Estonia</option>
   <option value=\"ET\" data-alternative-spellings=\"ET Ethiopia ኢትዮጵያ\">Ethiopia</option>
   <option value=\"FK\" data-alternative-spellings=\"FK Falkland Islands (Malvinas)\" data-relevancy-booster=\"0.5\">Falkland Islands (Malvinas)</option>
   <option value=\"FO\" data-alternative-spellings=\"FO Faroe Islands Føroyar Færøerne\" data-relevancy-booster=\"0.5\">Faroe Islands</option>
   <option value=\"FJ\" data-alternative-spellings=\"FJ Fiji Viti फ़िजी\">Fiji</option>
   <option value=\"FI\" data-alternative-spellings=\"FI Finland Suomi\">Finland</option>
   <option value=\"FR\" data-alternative-spellings=\"FR France République française\" data-relevancy-booster=\"2.5\">France</option>
   <option value=\"GF\" data-alternative-spellings=\"GF French Guiana\">French Guiana</option>
   <option value=\"PF\" data-alternative-spellings=\"PF French Polynesia Polynésie française\">French Polynesia</option>
   <option value=\"TF\" data-alternative-spellings=\"TF French Southern Territories\">French Southern Territories</option>
   <option value=\"GA\" data-alternative-spellings=\"GA Gabon République Gabonaise\">Gabon</option>
   <option value=\"GM\" data-alternative-spellings=\"GM Gambia\">Gambia</option>
   <option value=\"GE\" data-alternative-spellings=\"GE Georgia საქართველო\">Georgia</option>
   <option value=\"DE\" data-alternative-spellings=\"DE Germany Bundesrepublik Deutschland\" data-relevancy-booster=\"3\">Germany</option>
   <option value=\"GH\" data-alternative-spellings=\"GH Ghana\">Ghana</option>
   <option value=\"GI\" data-alternative-spellings=\"GI Gibraltar\" data-relevancy-booster=\"0.5\">Gibraltar</option>
   <option value=\"GR\" data-alternative-spellings=\"GR Greece Ελλάδα\" data-relevancy-booster=\"1.5\">Greece</option>
   <option value=\"GL\" data-alternative-spellings=\"GL Greenland grønland\" data-relevancy-booster=\"0.5\">Greenland</option>
   <option value=\"GD\" data-alternative-spellings=\"GD Grenada\">Grenada</option>
   <option value=\"GP\" data-alternative-spellings=\"GP Guadeloupe\">Guadeloupe</option>
   <option value=\"GU\" data-alternative-spellings=\"GU Guam\">Guam</option>
   <option value=\"GT\" data-alternative-spellings=\"GT Guatemala\">Guatemala</option>
   <option value=\"GG\" data-alternative-spellings=\"GG Guernsey\" data-relevancy-booster=\"0.5\">Guernsey</option>
   <option value=\"GN\" data-alternative-spellings=\"GN Guinea\">Guinea</option>
   <option value=\"GW\" data-alternative-spellings=\"GW Guinea-Bissau\">Guinea-Bissau</option>
   <option value=\"GY\" data-alternative-spellings=\"GY Guyana\">Guyana</option>
   <option value=\"HT\" data-alternative-spellings=\"HT Haiti\">Haiti</option>
   <option value=\"HM\" data-alternative-spellings=\"HM Heard Island and McDonald Islands\">Heard Island and McDonald Islands</option>
   <option value=\"VA\" data-alternative-spellings=\"VA Holy See (Vatican City State)\" data-relevancy-booster=\"0.5\">Holy See (Vatican City State)</option>
   <option value=\"HN\" data-alternative-spellings=\"HN Honduras\">Honduras</option>
   <option value=\"HK\" data-alternative-spellings=\"HK Hong Kong 香港\">Hong Kong</option>
   <option value=\"HU\" data-alternative-spellings=\"HU Hungary Magyarország\">Hungary</option>
   <option value=\"IS\" data-alternative-spellings=\"IS Iceland Island\">Iceland</option>
   <option value=\"IN\" data-alternative-spellings=\"IN India भारत गणराज्य Hindustan\" data-relevancy-booster=\"3\">India</option>
   <option value=\"ID\" data-alternative-spellings=\"ID Indonesia\" data-relevancy-booster=\"2\">Indonesia</option>
   <option value=\"IR\" data-alternative-spellings=\"IR Iran, Islamic Republic of ایران\">Iran, Islamic Republic of</option>
   <option value=\"IQ\" data-alternative-spellings=\"IQ Iraq العراق‎\">Iraq</option>
   <option value=\"IE\" data-alternative-spellings=\"IE Ireland Éire\" data-relevancy-booster=\"1.2\">Ireland</option>
   <option value=\"IM\" data-alternative-spellings=\"IM Isle of Man\" data-relevancy-booster=\"0.5\">Isle of Man</option>
   <option value=\"IL\" data-alternative-spellings=\"IL Israel إسرائيل ישראל\">Israel</option>
   <option value=\"IT\" data-alternative-spellings=\"IT Italy Italia\" data-relevancy-booster=\"2\">Italy</option>
   <option value=\"JM\" data-alternative-spellings=\"JM Jamaica\">Jamaica</option>
   <option value=\"JP\" data-alternative-spellings=\"JP Japan Nippon Nihon 日本\" data-relevancy-booster=\"2.5\">Japan</option>
   <option value=\"JE\" data-alternative-spellings=\"JE Jersey\" data-relevancy-booster=\"0.5\">Jersey</option>
   <option value=\"JO\" data-alternative-spellings=\"JO Jordan الأردن\">Jordan</option>
   <option value=\"KZ\" data-alternative-spellings=\"KZ Kazakhstan Қазақстан Казахстан\">Kazakhstan</option>
   <option value=\"KE\" data-alternative-spellings=\"KE Kenya\">Kenya</option>
   <option value=\"KI\" data-alternative-spellings=\"KI Kiribati\">Kiribati</option>
   <option value=\"KP\" data-alternative-spellings=\"KP Korea, Democratic People's Republic of North Korea\">North Korea</option>
   <option value=\"KR\" data-alternative-spellings=\"KR Korea, Republic of South Korea\" data-relevancy-booster=\"1.5\">South Korea</option>
   <option value=\"KW\" data-alternative-spellings=\"KW Kuwait الكويت\">Kuwait</option>
   <option value=\"KG\" data-alternative-spellings=\"KG Kyrgyzstan Кыргызстан\">Kyrgyzstan</option>
   <option value=\"LA\" data-alternative-spellings=\"LA Lao People's Democratic Republic\">Lao People\"s Democratic Republic</option>
   <option value=\"LV\" data-alternative-spellings=\"LV Latvia Latvija\">Latvia</option>
   <option value=\"LB\" data-alternative-spellings=\"LB Lebanon لبنان\">Lebanon</option>
   <option value=\"LS\" data-alternative-spellings=\"LS Lesotho\">Lesotho</option>
   <option value=\"LR\" data-alternative-spellings=\"LR Liberia\">Liberia</option>
   <option value=\"LY\" data-alternative-spellings=\"LY Libyan Arab Jamahiriya ليبيا\">Libyan Arab Jamahiriya</option>
   <option value=\"LI\" data-alternative-spellings=\"LI Liechtenstein\">Liechtenstein</option>
   <option value=\"LT\" data-alternative-spellings=\"LT Lithuania Lietuva\">Lithuania</option>
   <option value=\"LU\" data-alternative-spellings=\"LU Luxembourg\">Luxembourg</option>
   <option value=\"MO\" data-alternative-spellings=\"MO Macao\">Macao</option>
   <option value=\"MK\" data-alternative-spellings=\"MK Macedonia, The Former Yugoslav Republic Of Македонија\">Macedonia, The Former Yugoslav Republic Of</option>
   <option value=\"MG\" data-alternative-spellings=\"MG Madagascar Madagasikara\">Madagascar</option>
   <option value=\"MW\" data-alternative-spellings=\"MW Malawi\">Malawi</option>
   <option value=\"MY\" data-alternative-spellings=\"MY Malaysia\">Malaysia</option>
   <option value=\"MV\" data-alternative-spellings=\"MV Maldives\">Maldives</option>
   <option value=\"ML\" data-alternative-spellings=\"ML Mali\">Mali</option>
   <option value=\"MT\" data-alternative-spellings=\"MT Malta\">Malta</option>
   <option value=\"MH\" data-alternative-spellings=\"MH Marshall Islands\" data-relevancy-booster=\"0.5\">Marshall Islands</option>
   <option value=\"MQ\" data-alternative-spellings=\"MQ Martinique\">Martinique</option>
   <option value=\"MR\" data-alternative-spellings=\"MR Mauritania الموريتانية\">Mauritania</option>
   <option value=\"MU\" data-alternative-spellings=\"MU Mauritius\">Mauritius</option>
   <option value=\"YT\" data-alternative-spellings=\"YT Mayotte\">Mayotte</option>
   <option value=\"MX\" data-alternative-spellings=\"MX Mexico Mexicanos\" data-relevancy-booster=\"1.5\">Mexico</option>
   <option value=\"FM\" data-alternative-spellings=\"FM Micronesia, Federated States of\">Micronesia, Federated States of</option>
   <option value=\"MD\" data-alternative-spellings=\"MD Moldova, Republic of\">Moldova, Republic of</option>
   <option value=\"MC\" data-alternative-spellings=\"MC Monaco\">Monaco</option>
   <option value=\"MN\" data-alternative-spellings=\"MN Mongolia Mongγol ulus Монгол улс\">Mongolia</option>
   <option value=\"ME\" data-alternative-spellings=\"ME Montenegro\">Montenegro</option>
   <option value=\"MS\" data-alternative-spellings=\"MS Montserrat\" data-relevancy-booster=\"0.5\">Montserrat</option>
   <option value=\"MA\" data-alternative-spellings=\"MA Morocco المغرب\">Morocco</option>
   <option value=\"MZ\" data-alternative-spellings=\"MZ Mozambique Moçambique\">Mozambique</option>
   <option value=\"MM\" data-alternative-spellings=\"MM Myanmar\">Myanmar</option>
   <option value=\"NA\" data-alternative-spellings=\"NA Namibia Namibië\">Namibia</option>
   <option value=\"NR\" data-alternative-spellings=\"NR Nauru Naoero\" data-relevancy-booster=\"0.5\">Nauru</option>
   <option value=\"NP\" data-alternative-spellings=\"NP Nepal नेपाल\">Nepal</option>
   <option value=\"NL\" data-alternative-spellings=\"NL Netherlands Holland Nederland\" data-relevancy-booster=\"1.5\">Netherlands</option>
   <option value=\"NC\" data-alternative-spellings=\"NC New Caledonia\" data-relevancy-booster=\"0.5\">New Caledonia</option>
   <option value=\"NZ\" data-alternative-spellings=\"NZ New Zealand Aotearoa\">New Zealand</option>
   <option value=\"NI\" data-alternative-spellings=\"NI Nicaragua\">Nicaragua</option>
   <option value=\"NE\" data-alternative-spellings=\"NE Niger Nijar\">Niger</option>
   <option value=\"NG\" data-alternative-spellings=\"NG Nigeria Nijeriya Naíjíríà\" data-relevancy-booster=\"1.5\">Nigeria</option>
   <option value=\"NU\" data-alternative-spellings=\"NU Niue\" data-relevancy-booster=\"0.5\">Niue</option>
   <option value=\"NF\" data-alternative-spellings=\"NF Norfolk Island\" data-relevancy-booster=\"0.5\">Norfolk Island</option>
   <option value=\"MP\" data-alternative-spellings=\"MP Northern Mariana Islands\" data-relevancy-booster=\"0.5\">Northern Mariana Islands</option>
   <option value=\"NO\" data-alternative-spellings=\"NO Norway Norge Noreg\" data-relevancy-booster=\"1.5\">Norway</option>
   <option value=\"OM\" data-alternative-spellings=\"OM Oman عمان\">Oman</option>
   <option value=\"PK\" data-alternative-spellings=\"PK Pakistan پاکستان\" data-relevancy-booster=\"2\">Pakistan</option>
   <option value=\"PW\" data-alternative-spellings=\"PW Palau\" data-relevancy-booster=\"0.5\">Palau</option>
   <option value=\"PS\" data-alternative-spellings=\"PS Palestinian Territory, Occupied فلسطين\">Palestinian Territory, Occupied</option>
   <option value=\"PA\" data-alternative-spellings=\"PA Panama\">Panama</option>
   <option value=\"PG\" data-alternative-spellings=\"PG Papua New Guinea\">Papua New Guinea</option>
   <option value=\"PY\" data-alternative-spellings=\"PY Paraguay\">Paraguay</option>
   <option value=\"PE\" data-alternative-spellings=\"PE Peru\">Peru</option>
   <option value=\"PH\" data-alternative-spellings=\"PH Philippines Pilipinas\" data-relevancy-booster=\"1.5\">Philippines</option>
   <option value=\"PN\" data-alternative-spellings=\"PN Pitcairn\" data-relevancy-booster=\"0.5\">Pitcairn</option>
   <option value=\"PL\" data-alternative-spellings=\"PL Poland Polska\" data-relevancy-booster=\"1.25\">Poland</option>
   <option value=\"PT\" data-alternative-spellings=\"PT Portugal Portuguesa\" data-relevancy-booster=\"1.5\">Portugal</option>
   <option value=\"PR\" data-alternative-spellings=\"PR Puerto Rico\">Puerto Rico</option>
   <option value=\"QA\" data-alternative-spellings=\"QA Qatar قطر\">Qatar</option>
   <option value=\"RE\" data-alternative-spellings=\"RE Réunion Reunion\">Réunion</option>
   <option value=\"RO\" data-alternative-spellings=\"RO Romania Rumania Roumania România\">Romania</option>
   <option value=\"RU\" data-alternative-spellings=\"RU Russian Federation Rossiya Российская Россия\" data-relevancy-booster=\"2.5\">Russian Federation</option>
   <option value=\"RW\" data-alternative-spellings=\"RW Rwanda\">Rwanda</option>
   <option value=\"BL\" data-alternative-spellings=\"BL Saint Barthélemy St. Barthelemy\">Saint Barthélemy</option>
   <option value=\"SH\" data-alternative-spellings=\"SH Saint Helena St.\">Saint Helena</option>
   <option value=\"KN\" data-alternative-spellings=\"KN Saint Kitts and Nevis St.\">Saint Kitts and Nevis</option>
   <option value=\"LC\" data-alternative-spellings=\"LC Saint Lucia St.\">Saint Lucia</option>
   <option value=\"MF\" data-alternative-spellings=\"MF Saint Martin (French Part) St.\">Saint Martin (French Part)</option>
   <option value=\"PM\" data-alternative-spellings=\"PM Saint Pierre and Miquelon St.\">Saint Pierre and Miquelon</option>
   <option value=\"VC\" data-alternative-spellings=\"VC Saint Vincent and the Grenadines St.\">Saint Vincent and the Grenadines</option>
   <option value=\"WS\" data-alternative-spellings=\"WS Samoa\">Samoa</option>
   <option value=\"SM\" data-alternative-spellings=\"SM San Marino\">San Marino</option>
   <option value=\"ST\" data-alternative-spellings=\"ST Sao Tome and Principe\">Sao Tome and Principe</option>
   <option value=\"SA\" data-alternative-spellings=\"SA Saudi Arabia السعودية\">Saudi Arabia</option>
   <option value=\"SN\" data-alternative-spellings=\"SN Senegal Sénégal\">Senegal</option>
   <option value=\"RS\" data-alternative-spellings=\"RS Serbia Србија Srbija\">Serbia</option>
   <option value=\"SC\" data-alternative-spellings=\"SC Seychelles\" data-relevancy-booster=\"0.5\">Seychelles</option>
   <option value=\"SL\" data-alternative-spellings=\"SL Sierra Leone\">Sierra Leone</option>
   <option value=\"SG\" data-alternative-spellings=\"SG Singapore Singapura சிங்கப்பூர் குடியரசு 新加坡共和国\">Singapore</option>
   <option value=\"SX\" data-alternative-spellings=\"SX Sint Maarten (Dutch Part)\">Sint Maarten (Dutch Part)</option>
   <option value=\"SK\" data-alternative-spellings=\"SK Slovakia Slovenská Slovensko\">Slovakia</option>
   <option value=\"SI\" data-alternative-spellings=\"SI Slovenia Slovenija\">Slovenia</option>
   <option value=\"SB\" data-alternative-spellings=\"SB Solomon Islands\">Solomon Islands</option>
   <option value=\"SO\" data-alternative-spellings=\"SO Somalia الصومال\">Somalia</option>
   <option value=\"ZA\" data-alternative-spellings=\"ZA South Africa RSA Suid-Afrika\">South Africa</option>
   <option value=\"GS\" data-alternative-spellings=\"GS South Georgia and the South Sandwich Islands\">South Georgia and the South Sandwich Islands</option>
   <option value=\"SS\" data-alternative-spellings=\"SS South Sudan\">South Sudan</option>
   <option value=\"ES\" data-alternative-spellings=\"ES Spain España\" data-relevancy-booster=\"2\">Spain</option>
   <option value=\"LK\" data-alternative-spellings=\"LK Sri Lanka ශ්‍රී ලංකා இலங்கை Ceylon\">Sri Lanka</option>
   <option value=\"SD\" data-alternative-spellings=\"SD Sudan السودان\">Sudan</option>
   <option value=\"SR\" data-alternative-spellings=\"SR Suriname शर्नम् Sarnam Sranangron\">Suriname</option>
   <option value=\"SJ\" data-alternative-spellings=\"SJ Svalbard and Jan Mayen\" data-relevancy-booster=\"0.5\">Svalbard and Jan Mayen</option>
   <option value=\"SZ\" data-alternative-spellings=\"SZ Swaziland weSwatini Swatini Ngwane\">Swaziland</option>
   <option value=\"SE\" data-alternative-spellings=\"SE Sweden Sverige\" data-relevancy-booster=\"1.5\">Sweden</option>
   <option value=\"CH\" data-alternative-spellings=\"CH Switzerland Swiss Confederation Schweiz Suisse Svizzera Svizra\" data-relevancy-booster=\"1.5\">Switzerland</option>
   <option value=\"SY\" data-alternative-spellings=\"SY Syrian Arab Republic Syria سورية\">Syrian Arab Republic</option>
   <option value=\"TW\" data-alternative-spellings=\"TW Taiwan, Province of China 台灣 臺灣\">Taiwan, Province of China</option>
   <option value=\"TJ\" data-alternative-spellings=\"TJ Tajikistan Тоҷикистон Toçikiston\">Tajikistan</option>
   <option value=\"TZ\" data-alternative-spellings=\"TZ Tanzania, United Republic of\">Tanzania, United Republic of</option>
   <option value=\"TH\" data-alternative-spellings=\"TH Thailand ประเทศไทย Prathet Thai\">Thailand</option>
   <option value=\"TL\" data-alternative-spellings=\"TL Timor-Leste\">Timor-Leste</option>
   <option value=\"TG\" data-alternative-spellings=\"TG Togo Togolese\">Togo</option>
   <option value=\"TK\" data-alternative-spellings=\"TK Tokelau\" data-relevancy-booster=\"0.5\">Tokelau</option>
   <option value=\"TO\" data-alternative-spellings=\"TO Tonga\">Tonga</option>
   <option value=\"TT\" data-alternative-spellings=\"TT Trinidad and Tobago\">Trinidad and Tobago</option>
   <option value=\"TN\" data-alternative-spellings=\"TN Tunisia تونس\">Tunisia</option>
   <option value=\"TR\" data-alternative-spellings=\"TR Turkey Türkiye Turkiye\">Turkey</option>
   <option value=\"TM\" data-alternative-spellings=\"TM Turkmenistan Türkmenistan\">Turkmenistan</option>
   <option value=\"TC\" data-alternative-spellings=\"TC Turks and Caicos Islands\" data-relevancy-booster=\"0.5\">Turks and Caicos Islands</option>
   <option value=\"TV\" data-alternative-spellings=\"TV Tuvalu\" data-relevancy-booster=\"0.5\">Tuvalu</option>
   <option value=\"UG\" data-alternative-spellings=\"UG Uganda\">Uganda</option>
   <option value=\"UA\" data-alternative-spellings=\"UA Ukraine Ukrayina Україна\">Ukraine</option>
   <option value=\"AE\" data-alternative-spellings=\"AE United Arab Emirates UAE الإمارات\">United Arab Emirates</option>
   <option value=\"GB\" data-alternative-spellings=\"GB United Kingdom Great Britain England UK Wales Scotland Northern Ireland\" data-relevancy-booster=\"2.5\">United Kingdom</option>
   <option value=\"US\" data-alternative-spellings=\"US United States\" data-relevancy-booster=\"3.5 USA United States of America\">United States</option>
   <option value=\"UM\" data-alternative-spellings=\"UM United States Minor Outlying Islands\">United States Minor Outlying Islands</option>
   <option value=\"UY\" data-alternative-spellings=\"UY Uruguay\">Uruguay</option>
   <option value=\"UZ\" data-alternative-spellings=\"UZ Uzbekistan Ўзбекистон O'zbekstan O‘zbekiston\">Uzbekistan</option>
   <option value=\"VU\" data-alternative-spellings=\"VU Vanuatu\">Vanuatu</option>
   <option value=\"VE\" data-alternative-spellings=\"VE Venezuela\">Venezuela</option>
   <option value=\"VN\" data-alternative-spellings=\"VN Vietnam Việt Nam\" data-relevancy-booster=\"1.5\">Vietnam</option>
   <option value=\"VG\" data-alternative-spellings=\"VG Virgin Islands, British\" data-relevancy-booster=\"0.5\">Virgin Islands, British</option>
   <option value=\"VI\" data-alternative-spellings=\"VI Virgin Islands, U.S.\" data-relevancy-booster=\"0.5\">Virgin Islands, U.S.</option>
   <option value=\"WF\" data-alternative-spellings=\"WF Wallis and Futuna\" data-relevancy-booster=\"0.5\">Wallis and Futuna</option>
   <option value=\"EH\" data-alternative-spellings=\"EH Western Sahara لصحراء الغربية\">Western Sahara</option>
   <option value=\"YE\" data-alternative-spellings=\"YE Yemen اليمن\">Yemen</option>
   <option value=\"ZM\" data-alternative-spellings=\"ZM Zambia\">Zambia</option> 
   <option value=\"ZW\" data-alternative-spellings=\"ZW Zimbabwe\">Zimbabwe</option>
";
  return $output;
}

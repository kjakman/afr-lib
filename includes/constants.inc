<?php
define("UPPERCASE", 0);
define("LOWERCASE",1);
define("TITLECASE", 2);
define("CAPITALIZE", 3);

define("SMS_ERROR", 0);
define("SMS_OK", 1);
define("SMS_SIM", 2);

define("LOGIN_OK", 0);
define("LOGIN_WRONG_PASSWORD",    -1);
define("LOGIN_PROVIDER_ERROR",    -2);
define("LOGIN_WRONG_CAPTCHA",     -3);
define("LOGIN_USER_BLOCKED",      -4);
define("LOGIN_WRONG_EMAIL",       -5);
define("LOGIN_INVALID_REMEMBER",  -6);
define("LOGIN_INVALID_USER",      -7);
define("LOGIN_INVALID_CLIENT_ID", -8);
define("LOGIN_INACTIVE_ACCOUNT",  -9);
define("LOGIN_INVALID_SESSION",  -10);

                                
define("AUTH_EXPIRATION", 3600*24*1); // 1 day

define ("POSITION_TOP",    1);
define ("POSITION_RIGHT",  2);
define ("POSITION_BOTTOM", 3);
define ("POSITION_LEFT",   4);

// ISO-8601 numeric representation of the day of the week
define ("WEEKDAY_MONDAY",    1);
define ("WEEKDAY_TUESDAY",   2);
define ("WEEKDAY_WEDNESDAY", 3);
define ("WEEKDAY_THURSDAY",  4);
define ("WEEKDAY_FRIDAY",    5);
define ("WEEKDAY_SATURDAY",  6);
define ("WEEKDAY_SUNDAY",    7);

define("LOG_LEVEL_ALWAYS",  0);  // default, always logging
define("LOG_LEVEL_SEVERE",  1);  // severe errors that require program exit (e.g., in an application, you ran out of disk space).
define("LOG_LEVEL_ERROR",   2);  // error messages that can't be recovered from but the program can continue to run (e.g., in a server application, client sent through bad data but other clients can continue to run).
define("LOG_LEVEL_WARNING", 3);  // recoverable problem that you should be notified about (e.g., invalid value in a configuration file, so you fell back to the default).
define("LOG_LEVEL_WARN",    3);  // recoverable problem that you should be notified about (e.g., invalid value in a configuration file, so you fell back to the default).
define("LOG_LEVEL_LIVE",    3);  // recoverable problem that you should be notified about (e.g., invalid value in a configuration file, so you fell back to the default).
define("LOG_LEVEL_INFO",    4);  // informational messages.
define("LOG_LEVEL_TEST",    4);  // informational messages.
define("LOG_LEVEL_DEV",     5);  // general debugging messages, basically useful information that can be output on a single line.
define("LOG_LEVEL_DEBUG",   5);  // general debugging messages, basically useful information that can be output on a single line.
define("LOG_LEVEL_DUMP",    6);  // far more detailed debugging messages such as hex dumps of buffers.

define("LARGE_NUMBER", 1000000000000000);
define("SALT_LENGTH", 9);
define("PASSWORD_LENGTH", 8);

define("LOCATION_ACCURACY", 4); // decimal places in lat/lng

// Logging
define("LOG_ERROR", 0);
define("LOG_WARN",  1);
define("LOG_ALL",   2);

define("DB_LOGICAL_OR",  '$or');
define("DB_LOGICAL_OR2",  '$or2'); /** to allow for more than one or term */
define("DB_LOGICAL_AND", '$and');

define("DB_CONTAINS_OPERATOR", '__DB_CONTAINS');
define("DB_BEGINS_WITH_OPERATOR", '__DB_BEGINS_WITH');
define("DB_ENDS_WITH_OPERATOR", '__DB_ENDS_WITH');

// search order
define ("SORT_ASCENDING", 0);
define ("SORT_DESCENDING", 1);

define ("FORMAT_NEVER", 0);
define ("FORMAT_IF_NEEDED", 1);
define ("FORMAT_ALWAYS", 2);

define ("SEX_UNKNOWN", 0);
define ("SEX_MALE", 1);
define ("SEX_FEMALE", 2);

define ("ACTION_PREPEND", 1);
define ("ACTION_APPEND", 2);
define ("ACTION_OVERWRITE", 3);

define ("DB_NO", 0);
define ("DB_YES", 1);
define ("DB_REQUIRED", 2);

// index into class array
define ("CLASS_USER_LEVEL", 0);
// define ("CLASS_NAME",         0);
define ("CLASS_GROUP",        1);
define ("CLASS_TABLE",        2);
define ("CLASS_KEY",          3);
define ("CLASS_FLAGS",        4);
define ("CLASS_DESCRIPTOR",   5);
define ("CLASS_SORT",         6);
define ("CLASS_PARENT_CLASS", 7);
define ("CLASS_SITE_ID", 8);

// index into attribute array
define ("ATT_NAME", 0);
define ("ATT_TYPE", 1);
define ("ATT_DEFAULT", 2);
define ("ATT_FLAGS", 3);
define ("ATT_LENGTH", 4);
define ("ATT_OPTIONS", 4);
define ("ATT_RELATION_TYPE", 4);
define ("ATT_ARGS", 5);
define ("ATT_FILTERS", 5);
define ("ATT_ARGS2", 6);
define ("ATT_SORT", 6);

define ("TASK_TYPE_CHECKIN", 10);
define ("TASK_TYPE_CLEANING", 20);
define ("TASK_TYPE_TRANSPORT", 30);
define ("TASK_TYPE_MAINTENANCE", 40);
define ("TASK_TYPE_OTHER", 99);

define ("ATT_DEFAULT_NOW"       , '__NOW__'); // Defaults to current date time 0000-00-00 00:00:00 (19 chars)
define ("ATT_DEFAULT_NOW_SHORT" , '__NOW_SHORT__'); // Defaults to current date time yyyymmddhhmmss (14 chars)
define ("ATT_DEFAULT_NULL"      , '$NULL'); // DB NULL
define ("ATT_DEFAULT_ZERO"      , '__ZERO__'); // DB ZERO
define ("ATT_DEFAULT_BLANK"     , '__BLANK__'); // DB BLANK
define ("ATT_DEFAULT_TODAY"     , '__TODAY__'); // Defaults to current date 0000-00-00
define ("ATT_DEFAULT_CITY_ID"   , '__CITY_ID__');   
define ("ATT_DEFAULT_GEO_ID"    , '__GEO_ID__');   
define ("ATT_DEFAULT_PAGE_ID"   , '__PAGE_ID__');   
define ("ATT_DEFAULT_SESS_ID"   , '__SESS_ID__');   
define ("ATT_DEFAULT_LANG"      , '__LANG__');      
define ("ATT_DEFAULT_SITE_ID"   , '__SITE_ID__');   
define ("ATT_DEFAULT_AFF_ID"    , '__AFF_ID__');    
define ("ATT_DEFAULT_USER_ID"   , '__USER_ID__');   
define ("ATT_DEFAULT_GUEST_ID"  , '__GUEST_ID__');   
define ("ATT_DEFAULT_PROPERTY_ID"   , '__PROPERTY_ID__');   
define ("ATT_DEFAULT_PROPERTY_TYPE"   , '__PROPERTY_TYPE__');   
define ("ATT_DEFAULT_SERVER"   , '__THIS_SERVER__');                       
define ("ATT_DEFAULT_PROVIDER_ID"      , '__PROVIDER_ID__');
define ("ATT_DEFAULT_USERNAME"         , '__USERNAME__');
define ("ATT_DEFAULT_USER_NAME"        , '__USER_NAME__');
define ("ATT_DEFAULT_USER_LEVEL"       , '__USER_LEVEL__');
define ("ATT_DEFAULT_DISPLAY_NAME"     , '__DISPLAY_NAME__');
define ("ATT_DEFAULT_CLIENT_IP"        , '__CLIENT_IP__');
define ("ATT_DEFAULT_CLIENT_CC"        , '__CLIENT_CC__');
define ("ATT_DEFAULT_CLIENT_CITY"      , '__CLIENT_CITY__');
define ("ATT_DEFAULT_CLIENT_CONTINENT" , '__CLIENT_CONTINENT__');
define ("ATT_DEFAULT_CLIENT_LOCATION"  , '__CLIENT_LOCATION__');
define ("ATT_DEFAULT_USER_AGENT"       , '__USER_AGENT__');
define ("ATT_DEFAULT_TODAY"            , '__TODAY__');
define ("ATT_DEFAULT_COUNTRY"    , '__COUNTRY__');
define ("ATT_DEFAULT_CLIENT_ID"  , '__CLIENT_ID__');
define ("ATT_DEFAULT_LANGUAGE"   , '__LANGUAGE__');
define ("ATT_DEFAULT_ARTICLE_ID" , '__ARTICLE_ID__');
define ("ATT_DEFAULT_COMMENT_ID" , '__COMMENT_ID__');
define ("ATT_DEFAULT_ORG_ID"     , '__ORG_ID__');
define ("ATT_DEFAULT_OWNER_ID"   , '__OWNER_ID__');
define ("ATT_DEFAULT_AGENT_ID"   , '__AGENT_ID__');
define ("ATT_DEFAULT_STAFF_ID"   , '__STAFF_ID__');       
define ("ATT_DEFAULT_ADMIN_ID"   , '__ADMIN_ID__');
define ("ATT_DEFAULT_LEGAL_ID"   , '__LEGAL_ID__');
define ("ATT_DEFAULT_SOURCE_ID"  , '__SOURCE_ID__');

define ("GEOLEVEL_WORLD"       , 0);
define ("GEOLEVEL_CONTINENT"   , 1); 
define ("GEOLEVEL_REGION"      , 2); 
define ("GEOLEVEL_COUNTRY"     , 3); 
define ("GEOLEVEL_DISTRICT"    , 4); 
define ("GEOLEVEL_CITY"        , 5); 
define ("GEOLEVEL_BOROUGH"     , 6); 
define ("GEOLEVEL_NEIGHBORHOOD", 7); 
define ("GEOLEVEL_LOCATION"    , 8); 

define("PAGE_TEST"      , 1);
define("PAGE_HOME"      , 1000);
define("PAGE_ABOUT"     , 1001);                                
define("PAGE_FAQ"       , 1002);
define("PAGE_LINKS"     , 1003);
define("PAGE_NEWS"      , 1004);
define("PAGE_TERMS"     , 1005);
define("PAGE_GUESTBOOK" , 1006);
define("PAGE_SITEMAP"   , 1007);
define("PAGE_DIRECTIONS", 1008);
define("PAGE_MAP"       , 1009);
define("PAGE_CONTACT"   , 1010);
define("PAGE_REGISTER"  , 1011);
define("PAGE_PASSWORD"  , 1012);
define("PAGE_AFFILIATE" , 1013);
define("PAGE_RELOCATION", 1014);
define("PAGE_GUIDE"     , 1015);
define("PAGE_GOOGLE_CALENDAR", 1016);
define("PAGE_EVENT_CALENDAR", 1016);
define ("PAGE_PRIVACY_POLICY", 1017);
define ("PAGE_PRIVACY", 1017);
define ("PAGE_CANCELLATION_POLICY", 1018);
define ("PAGE_CANCELLATION", 1018);

define("PAGE_ADMIN",      1100);
define("PAGE_STATS",      1101);
define("PAGE_LOGIN",      1104);
define("PAGE_CALENDAR",   1105);
define("PAGE_EVENTS",     1114);
define("PAGE_WITHDRAW",   1106);
define("PAGE_EDIT_PROPERTY", 1107);
define("PAGE_EDIT_RENTAL", 1107);
define("PAGE_EDIT_SITE", 1108);
define("PAGE_PROFILE",   1109);
define("PAGE_USER_PROFILE",   1109);
define("PAGE_EDIT_PROFILE", 1121);
define("PAGE_ACCOUNT",   1110);
define("PAGE_INVOICES",  1111);
define("PAGE_PAYMENTS",  1112);
define("PAGE_PAYOUT",    1113);
define("PAGE_WITHDRAWALS", 1113);
define("PAGE_USERS", 1117);
define("PAGE_DICTIONARY", 1118);
define("PAGE_EDIT_PROFILE",   1121);
define("PAGE_GALLERY_BACKEND",   1123);

define("PAGE_EDIT_GALLERY",   1120);
define("PAGE_ADD_SITE", 1115);
define("PAGE_ADD_PROPERTY", 1116);

define ("PAGE_REQUEST",          1200);
define ("PAGE_ENQUIRE",          1201);
define ("PAGE_INQUIRY",          1201);
define ("PAGE_BOOK",             1202);
define ("PAGE_STATUS",           1203);
define ("PAGE_FEEDBACK",         1204);
define ("PAGE_TICKET",           1205);
define ("PAGE_BLOG"             ,1206);
define ("PAGE_FORUM"            ,1207);
define ("PAGE_INBOX"            ,1208);
define ("PAGE_DASHBOARD"        ,1209);
define ("PAGE_INVOICE"          ,1210);
define ("PAGE_REVIEW",          1211);
define ("PAGE_BOOKINGS",        1212);
define ("PAGE_RENTAL_LIST",     1122);

define ("PAGE_PROPERTY"         ,1300);
define ("PAGE_SHORTSTAY"        ,1301);
define ("PAGE_LONGSTAY"         ,1302);
define ("PAGE_REALESTATE"       ,1303);
define ("PAGE_BOAT"             ,1304);
define ("PAGE_HOTEL"            ,1305);
define ("PAGE_OFFICE"           ,1306);
define ("PAGE_SHORTSTAY-OFFICE" ,1307);
define ("PAGE_GOLF_COURSE"      ,1350);
define ("PAGE_GOLF_CLUB"        ,1351);
define ("PAGE_GEO_LOCATION"     ,1360);
define ("PAGE_LOCATION"         ,1361);

define ("PAGE_RENTAL_SEARCH"    ,1301);
define ("PAGE_RENTAL_DETAIL"    ,1307);

define ("PAGE_GALLERY"          ,1401);
define ("PAGE_PACKAGES"         ,1402);
define ("PAGE_FACEBOOK"         ,1403);
define ("PAGE_CATERING"         ,1404);
define ("PAGE_SEARCH"           ,1405);
define ("PAGE_DATA"             ,1406);
define ("PAGE_PHOTOS"           ,1407);
define ("PAGE_CART"             ,1408);
define ("PAGE_CHECKOUT"         ,1409);
define ("PAGE_PAYMENT"          ,1410);
define ("PAGE_ORDER_CONFIRMATION",1411);
define ("PAGE_SHARE", 1412);
define ("PAGE_MAIL", 1413);
define ("PAGE_MAIL_CAMPAIGN", 1414);
define ("PAGE_PLUGIN", 1415);


// node flags
// see http://www.achievo.org/wiki/Node_Flags for defs
$__CLASS_FLAGS = array("NF_ADD_LINK" => 1, "NF_READONLY" => 2, "NF_COPY" => 4, "NF_NO_ADD" => 8, "NF_NO_DELETE" => 16, "NF_NO_EDIT" => 32, "NF_NO_VIEW" => 64, "NF_NO_SEARCH" => 128, "NF_NO_EXTENDED_SEARCH" => 256, "NF_NO_FILTER" => 512, "NF_NO_SECURITY" => 1024, "NF_LOCK" => 2048, "NF_COMPACT_LIST" => 4096, "NF_COMPACT_RECORD" => 8192, "NF_NO_ACTIONS" => 16384, "NF_NO_LIST" => 32768, "NF_LOG" => 65536, "NF_CACHE" => 131072, "NF_CACHE_ASSOC" => 262144);
foreach($__CLASS_FLAGS as $k => $v) define ($k, $v);

$__ATTR_FLAGS  = array("AF_OBLIGATORY" => 1, "AF_UNIQUE" => 2, "AF_PRIMARY" => 4, "AF_LOG" => 8, "AF_AUTOINCREMENT" => 16, "AF_FORCE_SELECT" => 32, "AF_HIDE_LIST" => 64, "AF_HIDE_ADD" => 128, "AF_HIDE_EDIT" => 256, "AF_HIDE_SELECT" => 512, "AF_HIDE_VIEW" => 1024, "AF_HIDE_SEARCH" => 2048, "AF_HIDE" => 4096, "AF_READONLY_ADD" => 8192, "AF_READONLY_EDIT" => 16384, "AF_READONLY" => 32768 , "AF_NO_LABEL" => 65536, "AF_ONE2ONE_INTEGRATE" => 131072, "AF_BLANK_LABEL" => 262144, "AF_INLINE_ADD" => 524288, "AF_NO_SORT" => 1048576, "AF_LOCKED" => 2097152, "AF_SEARCHABLE" => 4194304, "AF_TOTAL" => 8388608, "AF_POPUP" => 16777216, "AF_CASCADE_DELETE" => 33554432, "AF_LARGE" => 67108864, "AF_NO_FILTER" => 134217728, "AF_PARENT" => 268435456, "AF_DUMMY" => 536870912, "AF_MULTILANGUAGE" => 1073741824, "AF_ALLOW_MULTIPLE" => 2147483648, "AF_ONE2ONE_PREPEND"=> 4294967296);
foreach($__ATTR_FLAGS as $k => $v) define ($k, $v);

$__CMS_ADD_FLAGS  = array("CMS_ADD_ALWAYS" => 1, "CMS_ADD_PROPERTY" => 2, "CMS_ADD_USER_LOGIN" => 4, "CMS_ADD_USER_REGISTER" => 8, "CMS_ADD_GALLERY" => 16, "CMS_ADD_PAYMENTS" => 32, "CMS_ADD_CART" => 64);
foreach($__CMS_ADD_FLAGS as $k => $v) define ($k, $v);

$__USER_ROLES = array("USER_ROLE_USER" => 1, "USER_ROLE_GUEST" => 2, "USER_ROLE_ORG" => 4, "USER_ROLE_HOST" => 8, "USER_ROLE_OWNER" => 8, "USER_ROLE_AFFILIATE" => 16, "USER_ROLE_CONTRACTOR" => 32, "USER_ROLE_AGENT" => 64, "USER_ROLE_ARTIST" => 128, "USER_ROLE_CURATOR" => 256, "USER_ROLE_FOO3" => 512, "USER_ROLE_FOO4" => 1024, "USER_ROLE_FOO5" => 2048, "USER_ROLE_FOO6" => 4096, "USER_ROLE_FOO7" => 8192, "USER_ROLE_STAFF" => 16384, "USER_ROLE_ADMIN" => 32768, "USER_ROLE_SYSTEM" => 65536);
foreach($__USER_ROLES as $k => $v) define ($k, $v);

$__SITE_ROLES = array(
  "SITE_ROLE_LOGIN" => 1, "SITE_ROLE_REGISTER" => 2, "SITE_ROLE_PAYMENT" => 4, 
  "SITE_ROLE_RENTAL" => 8, "SITE_ROLE_BOAT" => 16, "SITE_ROLE_LONGSTAY" => 32, "SITE_ROLE_REALESTATE" => 64, "SITE_ROLE_HOTEL" => 128, 
  "SITE_ROLE_FOO1" => 256, "SITE_ROLE_FOO3" => 512, "SITE_ROLE_FOO4" => 1024, "SITE_ROLE_FOO5" => 2048, 
  "SITE_ROLE_FOO6" => 4096, "SITE_ROLE_FOO7" => 8192, 
  "SITE_ROLE_GOLF" => 16384, "SITE_ROLE_GALLERY" => 32768, "SITE_ROLE_PET" => 65536);
foreach($__SITE_ROLES as $k => $v) define ($k, $v);

/*
define ("CMS_ADD_ALWAYS", 1); // Alaways add
define ("CMS_ADD_PROPERTY", 2); // Add if this is a property site
define ("CMS_ADD_USER_LOGIN", 4); // Add if users can login
define ("CMS_ADD_USER_REGISTER", 8); // Add if users new can register
define ("CMS_ADD_GALLERY", 16); // Add if this is a gallery site


define ("NF_ADD_LINK", 1);
define ("NF_READONLY", 2);
define ("NF_COPY", 4);
define ("NF_NO_ADD", 8);
define ("NF_NO_DELETE", 16);
define ("NF_NO_EDIT", 32);
define ("NF_NO_VIEW", 64);
define ("NF_NO_SEARCH", 128);
define ("NF_NO_EXTENDED_SEARCH", 256);
define ("NF_NO_FILTER", 512);
define ("NF_NO_SECURITY", 1024);
define ("NF_LOCK", 2048);

// added self
define ("NF_COMPACT_LIST", 4096); // when set, children (one2one, one2many) are not expanded in the list view
define ("NF_COMPACT_RECORD", 8192); // when set, children (one2one, one2many) are not expanded in the record view (view/add/edit)
define ("NF_NO_ACTIONS", 16384);
define ("NF_NO_LIST", 32768);
define ("NF_LOG", 65536); // logs add/delete of objects
define ("NF_CACHE", 131072); // cache to memory
define ("NF_CACHE_ASSOC", 262144); // cache all to memory as associative array


// attribute flags
define ("AF_OBLIGATORY", 1); // Field is required. A value must be entered by the user.
define ("AF_UNIQUE", 2); // The value entered by the user must be unique.
define ("AF_PRIMARY", 4); // This attribute is (part of) the primary key of the table represented by the node.
define ("AF_LOG", 8); // Log changes to this field
define ("AF_AUTOINCREMENT", 16); // The attribute value is incremented automatically for each newly added record.
//define ("AF_AUTOKEY", 32); // Indicates that the attribute is a hidden, autoincremented primary key. (This is actually an alias for AF_AUTOINCREMENT|AF_PRIMARY|AF_HIDE.)
define ("AF_FORCE_SELECT", 32); // Force select box (no ajax chooser)
define ("AF_HIDE_LIST", 64); // The attribute is hidden in record lists.
define ("AF_HIDE_ADD", 128); // The attribute is hidden in add-forms.
define ("AF_HIDE_EDIT", 256); // The attribute is hidden in edit-forms.
define ("AF_HIDE_SELECT", 512); // The attribute is hidden in select-pages.
define ("AF_HIDE_VIEW", 1024); // The attribute is hidden in view-pages.
define ("AF_HIDE_SEARCH", 2048); // The attribute is hidden in extended search pages.
define ("AF_HIDE", 4096); // Attribute is always hidden.
define ("AF_READONLY_ADD", 8192); // In add-pages, the attribute is read-only.
define ("AF_READONLY_EDIT", 16384); // In edit-pages, the attribute is read-only.
define ("AF_READONLY", 32768 ); // The attribute is read-only in both edit- and add-pages.
define ("AF_NO_LABEL", 65536); // Don't display the attribute name as a label in front of the attribute. The attribute is displayed directly at the position the label would normally be.
define ("AF_ONE2ONE_INTEGRATE", 131072); // If set, one2one children will be integrated to parent object as if they were one
define ("AF_ENCRYPT", 262144); // Was: AF_BLANK_LABLEL - Don't display the attribute name as a label in front of the attribute. The difference with AF_NO_LABEL is that with this flag the label is replaced with empty space, leaving the attribute itself at its original position.
define ("AF_INLINE_ADD", 524288); // For many2one: Show add link which displays modal dialog for adding this
define ("AF_NO_SORT", 1048576); // In recordlist, the column for this attribute is not sortable by the user.
define ("AF_LOCKED", 2097152); // Attribute shown as input, but input is readonly.
define ("AF_SEARCHABLE", 4194304); // This attribute is searchable through the quick-search bar in recordlists.
define ("AF_TOTAL", 8388608); // In recordlists, the column for this attribute is automatically totalized, and the total is displayed in an extra row right under the recordlist.
define ("AF_POPUP", 16777216); // Attributes that support this flag, like the atkFileAttribute, open a popup instead of showing their contents directly in the recordlist.
define ("AF_CASCADE_DELETE", 33554432); // Attributes that support cascade deletes, like the atkOneToManyRelation, will perform delete functionality when this flag is set and a record is deleted.
define ("AF_LARGE", 67108864); // Attributes that support this flag use this flag as a hint that a large number of records will need to be processed. The atkManyToOneRelation uses this flag to prevent dropdowns with too many records.
define ("AF_NO_FILTER", 134217728); // Attributes that support this flag, like the atkManyToOneRelation, will use this flag to determine whether filters on related nodes should be executed.
define ("AF_PARENT", 268435456); // When using an atkTreeNode instead of atkNode, this flag tells the node that the attribute is the foreign key in the parent/child relation. (Note: the atkTreeNode is not available in the ATK-light version that ships with Achievo.)
define ("AF_DUMMY", 536870912); // Atk puts quotes around field values when storing them in the database. Use this flag to indicate that it shouldn't use quotes. This is useful when the value should be treated as a SQL expression instead of a value.
define ("AF_MULTILANGUAGE", 1073741824); // If an attribute is multilingual, ATK uses this flag to enable the auto-multilanguage feature for this attribute. (Note: this functionality is not available in the ATK-light version that ships with Achievo.)
define ("AF_ALLOW_MULTIPLE", 2147483648); // Allow multiple file uploads
define ("AF_ONE2ONE_PREPEND", 4294967296); // If set, one2one children will be integrated to parent object as if they were one
*/

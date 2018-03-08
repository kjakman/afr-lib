<?php

/** global arrays - move to yaml config, can override per site, store in constants */

$nodeTypeArray = array();
//$nodeTypes = array_keys($__CLASSES);
//foreach($nodeTypes as $nodeType) $nodeTypeArray[$nodeType] = ucfirst($nodetype);  

$y = phrase('yes', CAPITALIZE);
$n = phrase('no', CAPITALIZE);
$r = phrase('required', CAPITALIZE);
$ynr = "$y/$n/$r";

$global_scope = [];

/** move all these to yaml */
$rateTypeArray = array(''=>'', 'hour' => 'hour', 'day' => 'day', 'month' => 'month', 'total' => 'total', 'special' => 'special');  
$currencyArray = array('EUR' => 'Euro', 'USD' => 'US Dollar', 'GBP' =>'British Pound', 'BTC' => 'Bitcoin');
$db_type_array = array('mysql', 'pgsql', 'oci8', 'odbc', 'sybase', 'fbsql', 'ibase', 'ifx', 'msql', 'mssql'); // (These are pear supported dbs)
$arrivalTypeArray = array('' => phrase('select', CAPITALIZE).'...', 'air' => phrase('air', CAPITALIZE), 'car' => phrase('car', CAPITALIZE), 'train' => phrase('train', CAPITALIZE), 'bus' => phrase('bus', CAPITALIZE), 'boat' => phrase('boat', CAPITALIZE), 'other' => phrase('other', CAPITALIZE));
$langArray = array('en'=> 'English', 'nl' => 'Dutch', 'no' => 'Norwegian');  
$countryArray = cc_array("Select country...");
// $boatTypeArray = array('open'=>'Open Boat', 'classic'=>'Classic Canal Boat', 'canal' => 'Canal Boat', 'salon'=>'Salon Boat', 'steam' => 'Steam Boat', 'monumental'=>'Monumental Boat', 'self-operated'=>'Self operated',  'sail' => 'Sail Boat', 'barge' => 'Barge', 'ij' => 'IJ boat');

$salutationArray = array(''=>'', 'Mr.' => phrase('mr.', CAPITALIZE), 'Ms.'=>phrase('ms.', CAPITALIZE), 'Mrs.'=>phrase('mrs.', CAPITALIZE), 'Dr.'=>phrase('dr.', CAPITALIZE));
$locationArray = array('landmark'=>'Landmark', 'continent' => 'Continent', 'region' => 'Region', 'country' => 'Country', 'district' => 'District', 'city' => 'City', 'borough' => 'Borough', 'neighborhood'=> 'Neighborhood', 'location' => 'Location');
$reqTypeArray = $enqTypeArray = array('enquiry'=>'Enquiry', 'reservation'=>'Reservation');

$weekdaysArray = [0 => 'None', 1 => phrase('Monday'), 2 => phrase('Tuesday'), 3 => phrase('Wednesday'), 4 => phrase('Thursday'), 5 => phrase('Friday'), 6 => phrase('Saturday'), 7 => phrase('Sunday')];

$billtoArray = [0 => phrase('guest', CAPITALIZE), 1 => phrase('employer', CAPITALIZE), 2 => phrase('department', CAPITALIZE), 3 => phrase('agent', CAPITALIZE), 4 => phrase('other', CAPITALIZE)];

$resizeArray = [
  'org' =>    [0, 'org'], 
  'tiny'   => [40, 'tiny', ['crop' => 1]],                        
  'thumb' =>  [100, 'thumbs'],
  'square' => [200, 'square', ['crop' => 1]],                        
  'small' =>  [200, 'small'], 
  'medium' => [640, 'medium'], 
  'ig' =>     [640, 'ig', ['crop' => 1]], // instagram square
  'big'   =>  [1280, 'big'], 
  'full'  =>  [1920, 'full']
];

$yes_no_req = array('' => $ynr, DB_YES => $y, DB_NO => $n, DB_REQUIRED => $r);
$actionArray  = array('list'=>'List','view'=>'View','add'=>'Add','edit'=>'Edit','del'=>'Delete');
$frequencyArray = array('instant' => phrase('instant', CAPITALIZE), 'daily' => phrase('daily', CAPITALIZE), 'weekly' => phrase('weekly', CAPITALIZE), 'monthly' => phrase('monthly', CAPITALIZE));
$sms_flags = array('New request' => SMS_FLAG_REQUEST, 'Confirmed Reservation' => SMS_FLAG_CONFIRMED, 'Cancellations' => SMS_FLAG_CANCELLED);

$db_types1 = array('', 'int', 'tinyint', 'smallint', 'mediumint', 'bigint', 'float', 'decimal', 'date', 'datetime', 'timestamp', 'time', 'char', 'varchar', 'binary', 'varbinary', 'text', 'blob', 'set', 'enum');
foreach($db_types1 as $db_type) $db_types[$db_type] = $db_type;

$descriptionActionArray = array('' => 'Select Action', ACTION_PREPEND => 'Prepend', ACTION_APPEND => 'Append', ACTION_OVERWRITE => 'Overwrite');

$reservation_media_options = $product_media_options = $event_media_options = $single_media_options = $user_media_options = $gallery_media_options = $property_media_options = $std_media_options = $std_image_options = array(
  'minimum_size' => 100, 
  'minimum_file_size' => '1K', 
  'maximum_file_size' => '100M', 
  'allow_multiple' => true, 
  'allowed_types' => array('image','audio','video'), 
  'resize' => 1
);
 

$event_media_options['allowed_types'] = array('image','video', 'audio', 'text', 'application');
$user_media_options['allowed_types'] = array('image','video');
$property_media_options['allowed_types'] = array('image','video');
$single_media_options['allow_multiple'] = $std_image_options['allow_multiple'] = false;
$reservation_media_options['resize'] = 0;
$reservation_media_options['allowed_types'] = array('image','video', 'audio', 'text', 'application');

$std_image_options['allowed_types'] = 'image';
$positionArray = array(0 => phrase('no', CAPITALIZE), POSITION_TOP => phrase('above', CAPITALIZE), POSITION_BOTTOM => phrase('below', CAPITALIZE), POSITION_RIGHT => phrase('right', CAPITALIZE), POSITION_LEFT => phrase('left', CAPITALIZE)); 
$shareTypeArray = array('' => phrase('select'), 'image' => phrase('image', CALITALIZE), 'link' => phrase('link', CALITALIZE), 'button' => phrase('button', CALITALIZE), 'frame' => phrase('frame', CALITALIZE), 'buy' => phrase('buy_now', CALITALIZE));

$passport_media_options = $std_image_options;
$passport_media_options['minimum_file_size'] = '1M';
$passport_media_options['minimum_size'] = '1024';
  
  
$addressArray = 

array(
   'salutation'=>        array('Salutation',        'valSelect', '', 0, $salutationArray),
   'first_name'=>        array('First Name',        'str',    '', AF_SEARCHABLE, 50),
   'infix'=>             array('Infix',             'str',    '', AF_SEARCHABLE, 20),
   'last_name'=>         array('Last Name',         'str',    '', AF_SEARCHABLE, 50),
   'email_address'=>     array('Email',             'email',  '', AF_SEARCHABLE|AF_HIDE_LIST, 128),
   'organization'=>      array('Organization',      'str',    '', AF_SEARCHABLE, 200),
   'title'=>             array('Title',             'str',    '', AF_HIDE_LIST, 30),
   'address1'=>          array('Address1',          'str',    '', AF_HIDE_LIST, 50),
   'address2'=>          array('Address2',          'str',    '', AF_HIDE_LIST, 50),
   'city'=>              array('City',              'str',    '', AF_HIDE_LIST, 50),
   'state'=>             array('State',             'str',    '', AF_HIDE_LIST, 50),
   'zip'=>               array('Postal code',       'str',    '', AF_HIDE_LIST, 10),
   'country'=>           array('Country',           'many2one', ATT_DEFAULT_COUNTRY, AF_SEARCHABLE|AF_FORCE_SELECT, 'countries'),
   'phone '=>            array('Phone',             'phone',  '', AF_HIDE_LIST, 20),
   'phone2'=>            array('Phone2',            'phone',  '', AF_HIDE_LIST, 20),
   'phone3'=>            array('Phone3',            'phone',  '', AF_HIDE_LIST, 20),
   'url_web'=>           array('Web',               'url',    '', AF_HIDE_LIST, 50),
   'facebook'=>          array('Facebook',          'url',    '', AF_HIDE_LIST, 255),
   'twitter'=>           array('Twitter',           'str',    '', AF_HIDE_LIST, 255),
   'address_formatted'=> array('Formatted Address', 'text',   '', AF_HIDE_LIST));

define("WEEKDAYS", $weekdaysArray);
define("RESIZE_ARRAY", $resizeArray);
define("BILLTO", $billtoArray);

<?php
 
/**

  Very little of the below is now in use, as the calendar is now javascript + smarty templates
  
*/


function cal_defaults() {
	$admincal = new stdClass;
	$admincal->cell_height   = 20;

	$admincal->cell_width    = 24;
	$admincal->zoom = 1;
	$admincal->period['d'] = 1;
	$admincal->period['w'] = 1;
	$admincal->period['m'] = 1;
  $admincal->show = 'reservation'; // show reservations

	$admincal->def_status['reservation'] = STATUS_UNCONFIRMED; 
	$admincal->def_status['invoice']     = INV_STATUS_DRAFT; 
	$admincal->def_status['event']       = EVENT_STATUS_UNCONFIRMED; 
	$admincal->def_status['checkin']     = EVENT_STATUS_UNCONFIRMED; 
	$admincal->def_status['checkout']    = EVENT_STATUS_UNCONFIRMED; 
	$admincal->def_status['cleaning']    = EVENT_STATUS_UNCONFIRMED; 
	$admincal->def_status['transport']   = EVENT_STATUS_UNCONFIRMED; 
	$admincal->def_status['maintenance'] = EVENT_STATUS_UNCONFIRMED; 

	$admincal->status        = $admincal->def_status[$admincal->show]; 
	$admincal->resolution    = 2;
	$admincal->target_month  = $_SESSION['today_month'];
	$admincal->target_year   = $_SESSION['today_year'];
	$admincal->start_hour    = 8;
	$admincal->end_hour      = 24;
  $admincal->mode          = SHOW_CALENDAR; // graphical view
  $admincal->follow        = $u ? 1 : 0; // follow mode is on for logged in users by default;
  
	$admincal->property_type = 'shortstay';
  $admincal->cal_search  = array();
  $admincal->cal_auth['view']  = 1;
  $admincal->cal_auth['edit']  = 0;
  $admincal->cal_auth['past']  = 0;
  $admincal->cal_auth['stats']  = 0;
  $admincal->cal_auth['changes']  = 0;
  $admincal->cal_show['filters']  = 1;
  $admincal->cal_show['menu']     = 1;
  $admincal->cal_show['search']   = 1;
  $admincal->cal_show['inactive'] = 0;
  $admincal->cal_show['rentals']  = 1;
  return $admincal;
}
                
// return filters (parameters) using either GET or defaults
function get_cal_params($params = array()) {
  global $u;
  global $u_id;
  global $u_hostid;
  global $u_affid;
  global $u_level;
  $siteObj = $_SESSION['siteObj'];

  $errors = array();
  $debug_str = "";
  $debug = true;
  
  $_SESSION['cal_change'] = NULL; // store what we have changed, so we know what we can pull from cache
  
  $defaults = $_SESSION['cal_defaults'];
  if($params) $defaults = object_extend($defaults, $params); // override with params 

  //$debug_str .= "Defaults=".dump($defaults, true);
  $cal_params = $defaults;
  
  $reset = isset($_GET['reset']) || $params->reset; 
  if($reset) {
    $_SESSION['cal_filters'] = $_SESSION['cal_show'] = $_SESSION['cal_search'] = array();
    //$debug_str .= "reset<br>";
  } else {
    //$debug_str .= "no reset<br>";
  }
  $cal_auth    = $_SESSION['cal_auth'] ?: $cal_params->cal_auth; // auth view/edit/past
  $cal_show    = $_SESSION['cal_show'] ?: $cal_params->cal_show; // show menu, rentals, search, etc
  $cal_search  = $_SESSION['cal_search'] ?: $cal_params->cal_search; // which properties to show (y-axis)
  $cal_filters = $_SESSION['cal_filters'] ?: $cal_params->cal_filters; // which dates to show  (x-axis)

  // below allows changing cal_show by using $_GET['show_menu'], $_GET['show_rentals'], etc                                                                  
  $showAr = array('menu', 'search', 'filters', 'rentals');         
  foreach($showAr as $key) {
    if(isset($_GET['show_'.$key])) $cal_show[$key] = $_GET['show_'.$key]; //(isset($cal_show[$key]) ? $cal_show[$key] : $defaults->cal_show[$key]);
  }                               
  
  // which properties to show (y-axis)
  
  // which period to show (x-axis)
  $keys = array('d', 'm', 'w', 'y', 'f', 'sh', 'eh', 'p');
  foreach($keys as $key) {
    if(isset($_GET[$key])) $cal_filters[$key] = $key; //(isset($cal_show[$key]) ? $cal_show[$key] : $defaults->cal_show[$key]);
  }

  $type = $cal_filters['t'] = $cal_filters['t'] ?: $defaults->property_type;
  $show = $cal_filters['show'] = isset($_GET['show'] ) ? $_GET['show'] : (isset($cal_filters['show']) ? $cal_filters['show'] : $defaults->show);
  
  $client_ids = array($siteObj->client_id);
  $defaults->search['client_id'] = $client_ids;

  // get all hosts this user can see 
  $userObj = get_object('user', $u_id);
  if($u_id == SYSTEM_USER_ID) { // special case, show all
    $host_ids = find_objects("property", array("type" => $type), array("field" => "distinct host_id"));
  } else {
    $host_ids = user_host_list($userObj);
    $host_ids = find_objects("property", array("host_id" => $host_ids), array("field" => "distinct host_id"));
  }
  
  $defaults->search['host_id'] = $host_ids;
      
  $host_count = count($host_ids);
  if($host_count <= 1) $cal_show['search'] = 0; // don't show filters unless multiple hosts                    
  if($host_count) { 
    $cal_search['host_id'] = $host_ids;  
  } else { // user is not allowed to see any properties
    $cal_search['host_id'] = -1;
  }
  //dump($host_ids);


  // these are new: selectors or defaults: used to determine property filters below
  // client_id, host_id, property_type, apt_id, apt_list  

  $host_id = $cal_search['host_id'] = isset($_GET['host_id'] ) ? $_GET['host_id'] : (isset($cal_search['host_id']) ? $cal_search['host_id'] : $defaults->search['host_id']);
  $client_id = $cal_search['client_id'] = isset($_GET['client_id'] ) ? $_GET['client_id'] : (isset($cal_search['client_id']) ? $cal_search['client_id'] : $defaults->search['client_id']);
  $id = $cal_search['id'] = isset($_GET['id'] ) ? $_GET['id'] : (isset($cal_search['id']) ? $cal_search['id'] : $defaults->search['id']);
  $ng = $cal_search['max_guests'] = isset($_GET['max_guests'] ) ? $_GET['max_guests'] : (isset($cal_search['max_guests']) ? $cal_search['max_guests'] : $defaults->search['max_guests']);
  $apt_list = $cal_search['apt_list'] = isset($_GET['apt_list'] ) ? $_GET['apt_list'] : (isset($cal_search['apt_list']) ? $cal_search['apt_list'] : $defaults->search['apt_list']);
  
  // copy to property search
  $property_search = array();
  foreach($cal_search as $k => $v) {
    if($k == 'apt_list' && $v) $property_search['id'] = explode(',', $v);
    else if($k == 'max_guests' && $v) $property_search['max_guests'] = array('>=', $v);
    else $property_search[$k] = $v;
  }
  $property_search = array_merge_json($property_search, $siteObj->property_filters);

  $cal_params->property_search = $property_search = array_filter($property_search);

  if($default_client_id = $defaults->cal_search['client_id']) {
    //$debug_str .= "default client_id=$default_client_id<br>";
    $cal_params->clients = array();
    unset($cal_search['client_id']); // so the x-filter-box doesn't show
  } else {
    //$debug_str .= "NO default client_id: ".dump($defaults, true);
    $client_ids = find_objects("reservation", "", array("field" => "distinct client_id"));
    $clients = find_objects("client", array("client.id" => $client_ids), array("fields" => array("client.id as id", "client.name as name")));
    $cal_params->clients = $clients;
  }
  
  $hostSearch = array();
  if($client_id) $hostSearch['client_id'] = $client_id;
  $host_ids = find_objects("reservation", $hostSearch, array("field" => "distinct host_id"));

  $tbl = get_object_table('user');
  $user_fields = $_SESSION['phpauth'] ? array("{$tbl}.id", 'concat(user_profile.first_name, " ", user_profile.last_name) as name') : 
  $hosts = find_objects('user', array("{$tbl}.id" => $host_ids), array('fields' => $user_fields));

  $cal_params->hosts = $hosts;
  
  /** old code */

  if(isset($_GET['srt'])) $cal_params->sort = $_GET['srt'];
  if(isset($_GET['rev'])) $cal_params->order = $_GET['rev'];
  if(isset($_GET['id']))  $cal_params->id = $_GET['id'];
  $cal_params->user_level = isset($defaults->user_level) ? $defaults->user_level : $u_level;
  
  $u_level = $cal_params->user_level;
  if(!isset($cal_params->hide_past)) $cal_params->hide_past = $u_level <= USER_LEVEL_GUEST;
    
  // what has changed
  $change_time = array();
  $change_time_vars = array('d', 'm', 'w', 'y', 'f', 'sh', 'eh', 'p');
  foreach($change_time_vars as $var) if(isset($_GET[$var]) && $_GET[$var] != $cal_filters[$var]) $change_time[] = $var;

  $change_apt = array();
  $change_apt_vars = array('host_id', 'cid', 'si', 'id');
  foreach($change_apt_vars as $var) if(isset($_GET[$var]) && $_GET[$var] != $cal_filters[$var]) $change_apt[] = $var;
  
  $change_view = array();
  $change_view_vars = array('z');
  foreach($change_view_vars as $var) if(isset($_GET[$var]) && $_GET[$var] != $cal_filters[$var]) $change_view[] = $var;
  
  $_SESSION['cal_change']['apts'] = $change_apt;
  $_SESSION['cal_change']['time'] = $change_time;
  $_SESSION['cal_change']['view'] = array_merge($change_time,$change_apt,$change_view);

  // set filters from 1) GET, then 2) existing, then 3) default (in that order)
  $d    = $cal_filters['d']    = isset($_GET['d']    ) ? $_GET['d']    : (isset($cal_filters['d']   ) ? $cal_filters['d']    : $defaults->target_day   );
  $w    = $cal_filters['w']    = isset($_GET['w']    ) ? $_GET['w']    : (isset($cal_filters['w']   ) ? $cal_filters['w']    : $defaults->target_week  );
  $m    = $cal_filters['m']    = isset($_GET['m']    ) ? $_GET['m']    : (isset($cal_filters['m']   ) ? $cal_filters['m']    : $defaults->target_month );
  $y    = $cal_filters['y']    = isset($_GET['y']    ) ? $_GET['y']    : (isset($cal_filters['y']   ) ? $cal_filters['y']    : $defaults->target_year  );
  $z    = $cal_filters['z']    = isset($_GET['z']    ) ? $_GET['z']    : (isset($cal_filters['z']   ) ? $cal_filters['z']    : $defaults->zoom         );

  if($d > 0) $period_view = 'd';
  else if($w > 0) $period_view = 'w';
  else $period_view = 'm';
  //echo("period view = $period_view<br>");
                               
  if(isset($_GET['p'])) $cal_filters['pp'][$period_view] = $_GET['p'];
   
  // which date(s)
  $cal_filters['pp']['d'] = isset($cal_filters['pp']['d']) ? $cal_filters['pp']['d'] : $defaults->period['d'];
  $cal_filters['pp']['w'] = isset($cal_filters['pp']['w']) ? $cal_filters['pp']['w'] : $defaults->period['w'];
  $cal_filters['pp']['m'] = isset($cal_filters['pp']['m']) ? $cal_filters['pp']['m'] : $defaults->period['m'];

  // how many periods
  $p = $cal_filters['p'] = $cal_filters['pp'][$period_view];
  
  // hourly
  $sh   = $cal_filters['sh']   = isset($_GET['sh']   ) ? $_GET['sh']   : (isset($cal_filters['sh']  ) ? $cal_filters['sh']   : $defaults->start_hour   );
  $eh   = $cal_filters['eh']   = isset($_GET['eh']   ) ? $_GET['eh']   : (isset($cal_filters['eh']  ) ? $cal_filters['eh']   : $defaults->end_hour     );
  $r    = $cal_filters['r']    = isset($_GET['r']    ) ? $_GET['r']    : (isset($cal_filters['r']   ) ? $cal_filters['r']    : $defaults->resolution   );

  // list or grid
  $mode = $cal_filters['mode'] = isset($_GET['mode'] ) ? $_GET['mode'] : (isset($cal_filters['mode']) ? $cal_filters['mode'] : $defaults->mode);

  // 1st of month
  $follow = $cal_filters['f']  = isset($_GET['f']    ) ? $_GET['f']    : (isset($cal_filters['f']   ) ? $cal_filters['f']    : $defaults->follow);

  // multiple calendars (stach)
  $stack = $cal_filters['stack'] = isset($_GET['stack'] ) ? $_GET['stack'] : (isset($cal_filters['stack']) ? $cal_filters['stack'] : $defaults->stack);


  // status
  $st   = $cal_filters['st']   = isset($_GET['st']   ) ? $_GET['st']   : (isset($cal_filters['def_status'][$show]  ) ? $cal_filters['def_status'][$show] : $defaults->def_status[$show]);
  
  // below apply to which apartments are shown
  $si = $cal_filters['si'] = isset($_GET['si'] ) ? $_GET['si'] : (isset($cal_filters['si']) ? $cal_filters['si'] : $defaults->show_inactive);
  $apt_list = $cal_filters['apt_list'] = isset($_GET['apt_list'] ) ? $_GET['apt_list'] : (isset($cal_filters['apt_list']) ? $cal_filters['apt_list'] : $defaults->host_id      );
  $city = $cal_filters['cid']  = isset($_GET['cid']  ) ? $_GET['cid']  : (isset($cal_filters['cid'] ) ? $cal_filters['cid']  : $defaults->city_id      );
  // $host_id = $cal_filters['host_id'] = isset($_GET['host_id'] ) ? $_GET['host_id'] : (isset($cal_filters['host_id']) ? $cal_filters['host_id'] : $defaults->host_id      );

  /** end old code */
  
  // hack to show 24 hours a day in day view if period > 1 unless stacked vertically
  if($stack == 'v') {
    $follow = $cal_filters['f']  = 0; // turn off follow for vertical stacking
  } elseif($d && $p > 1) {
    $sh = $cal_filters['sh'] = 0;
    $eh = $cal_filters['eh'] = 24;
  }

  // not sure why this is necc.
  $cal_filters['property_type'] = $cal_filters['t'];

  $cal_params->show = $show;
  $cal_params->show_inactive = $si;
  // $obj_type = $cal_params->obj_type = in_array($show, array('reservation', 'invoice')) ? $show : 'event'; // hack: if $show = 'cleaning', obj_type = 'event';
  $cal_params->obj_type = $obj_type = $show;
  $cal_params->obj_name = get_object_name($obj_type);
  
  global $eventTypes;
  $cal_params->base_class = $base_class = $eventTypes[$obj_type] ? 'event' : $obj_type;

  list($start_field, $end_field) = object_timefields($obj_type);
  $start_field = 'checkin';
  $end_field = 'checkout';
  $cal_params->start_field = $start_field;
  $cal_params->end_field = $end_field;

	// echo("sf=$cal_params->start_field ef=$cal_params->end_field<br>");
  // $cal_params = calendar_db_specs($cal_params);


  if(isset($_GET['st'])) $cal_filters['def_status'][$obj_type] = $_GET['st']; // store user-set status for this type (res, inv, event)

  // store query string
  $cal_params->base_url = get_page_link(PAGE_CALENDAR);
  $is = $cal_params->base_url = $cal_params->base_url ?: ($_SERVER['REDIRECT_URL'] ?: $_SERVER['PHP_SELF']);

  // used for affiliate calendar
  if($go=$_GET['aff_host_id']) {
    //echo("aff host = $go");
    $cal_filters['host_id'] = $host_id = $go;
    $is .= "?aff_host_id=$go";
    $cal_params->allow_week_view = false;
    $cal_params->allow_day_view = false;
  } else {
    $cal_params->allow_week_view = isset($cal_params->allow_week_view) ? $cal_params->allow_week_view : true;
    $cal_params->allow_day_view = isset($cal_params->allow_day_view) ? $cal_params->allow_day_view : true;
  }

  $is_encoded = urlencode($is);
  $cal_params->request_uri = $is;
  //echo("isfoo=$is<br>");
  $cal_params->request_uri_encoded = $is_encoded;
    
  // set up the calendar
//  $cal_params->filters        = $_SESSION['cal_filters'] = $cal_filters;
  $cal_params->site_id        = $siteObj->id;
  $cal_params->client_id      = $client_id;
  $cal_params->filters        = $cal_filters;

  $cal_params->property_search = $property_search;
  //$cal_params->property_filters = $property_filters;
  
  $cal_params->target_day     = $cal_filters['d'];
  $cal_params->target_week    = $cal_filters['w'];
  $cal_params->target_month   = $cal_filters['m'];
  $cal_params->target_year    = $cal_filters['y'];
  $cal_params->zoom           = $cal_filters['z'];
  $cal_params->period         = $cal_filters['p'];
  $cal_params->status         = $cal_filters['st'];
  $cal_params->resolution     = $cal_filters['r'];
  $cal_params->property_type  = $cal_filters['t'] ?: 'shortstay';
  $cal_params->start_hour     = $cal_filters['sh'];
  $cal_params->end_hour       = $cal_filters['eh'];

  $cal_params->mode           = $cal_filters['mode'];
  $cal_params->show           = $cal_filters['show'] ?: 'reservation';
  $cal_params->follow         = $cal_filters['f'];
  $cal_params->stack          = $cal_filters['stack'];

  // store filters, search, show to remember
  $cal_params->cal_filters = $_SESSION['cal_filters'] = $cal_filters;
  $cal_params->cal_search = $_SESSION['cal_search'] = $cal_search;
  $cal_params->cal_show = $_SESSION['cal_show'] = $cal_show;
  $cal_params->show_inactive = $cal_filters['si'];

  $cal_params->host_count = $host_count;

  $currency = "EUR"; // todo: lookup
  $cal_params->currency = $currency;
  $cal_params->cur = get_currency_symbol($currency, true);

  //echo(dump($cal_params->cal_show, true));
   
  if($cal_params->target_day > 0) $cal_params->target_mode = 'day';
  elseif($cal_params->target_week > 0) $cal_params->target_mode = 'week';
  else $cal_params->target_mode = 'month';
  //dump($cal_params);die("gh");
  
  //$debug_str = "si=".$cal_params->show_inactive;
  //echo($debug_str);
  //$debug_str .= " cw=$cal_params->cell_width menu=$cal_show[menu] m=$cal_filters[m]";
  if($debug) $cal_params->debug = $debug_str;
  return $cal_params;
}


function calendar_init($cal_params) {

  global $u;              
  $filters = $cal_params->filters;

  $max_screen_size = 1200; // cutoff size for grid before downscaling

  $cal_params->language = $_SESSION['language'];

  // shrink cell width if period > 1
  $period = $cal_params->period;

  // copy values from filters
  $cal_params->cell_height = $_SESSION['cal_defaults']->cell_height;
  $cal_params->cell_width  = $_SESSION['cal_defaults']->cell_width;


  $calendar_dates = get_calendar_dates($cal_params);
  $cal_params->num_cols = count($calendar_dates); 

  $cal_params = get_calendar_start_end($calendar_dates, $cal_params);
  if($cal_params->target_day > 0) {
    $cal_params->target_weekday  = $cal_params->start_date['weekday'];
    $cal_params->target_weekday_name = $cal_params->start_date['weekday_name'];
    $cal_params->target_weekday_name_short = $cal_params->start_date['weekday_name_short'];
  }
  

  $cal_params = get_calendar_properties($cal_params);
  
  $num_days = count($calendar_dates);                           // num days
  $num_hours = $cal_params->end_hour - $cal_params->start_hour; // hours per day
  $num_cells = $cal_params->target_day ? $num_hours * $num_days : $num_days; // number of cells
  $num_cols = $cal_params->target_day ? $cal_params->resolution * $num_cells : $num_cells; // number of cols (1, 2, or 4 per cell)

  $cell_width = $cal_params->cell_width; // non - zoomed cell width

  // auto scale x if many cells to fit screen better
  $canvas_est_w  = $num_cells * $cell_width; // estimated canvas width
  //echo("estimate canvas width = ".$canvas_est_w."<br>");
  if ($cal_params->stack != 'v') {
    if ($canvas_est_w > $max_screen_size) { // width doesn't fit on most screens, down size X
      $excess_perc = 100 * (($canvas_est_w - $max_screen_size) / $max_screen_size);
      $x_factor = auto_scale_factor($excess_perc);
      // echo(" = ".$excess_perc." % to many pixels, x_factor=".$x_factor." result=".$canvas_est_w * $x_factor."<br>");
      $canvas_new_w  = $num_cells * $cell_width * $x_factor; // estimated canvas width
      $cell_width  = round($canvas_new_w / $num_cells);
    }  
  }
  
  $post_x_factor = 1; // default, no scaling
  // post X scaling depending on wether viewing month / day / week - before zooming, doesn't affect font-size
  if($cal_params->target_day && $num_cells < 30) {
    $post_x_factor = 2; // wider cells for day view if less than 30 hours total
  } else if($cal_params->target_week && $num_cells < 20) {
    $post_x_factor = 4; // wider cells for week view if less than 30 hours total
  }
                        
  // echo("stack=$cal_params->stack xfactor=$x_factor post=$post_x_factor");
  // zoom and size
  $cell_h             = round($cal_params->cell_height * $cal_params->zoom);
  $cell_w             = round($cell_width * $cal_params->zoom);

  $org_cell_w = $cell_w; // zoomed width, not affected by post scaling

  // font size - IS affected by zoom, but NOT post X scaling
  $cal_params->var_font_size = round($cell_h * 0.7);
  $cal_params->med_font_size = round($cell_h * 0.6);
  $cal_params->small_font_size = round($cell_h * 0.5);
  $cal_params->day_font_size = round($cell_w * 0.5);

  $cell_w *= $post_x_factor; // now apply widening so it doesn't affect font size

  $cal_params->cell_h = $cell_h;
  $cal_params->cell_w = $cell_w;
  $cal_params->org_cell_w = $org_cell_w;

  //$cal_params->grid_w = $grid_w = ($cal_params->target_day ? round($cell_w / $cal_params->resolution) : $cell_w);
  $cal_params->grid_w = $grid_w = ($cal_params->target_day ? $cell_w / $cal_params->resolution : $cell_w);

  $cal_params->bar_h = $bar_h = round($cell_h * 0.66);
  $cal_params->bar_x = round($cell_w * 0.66); // the starting x position in cell of reservation bar
  $cal_params->bar_y = round($cell_h * 0.25); // the starting y position in cell of reservation bar

  // dims and css + js
  $num_apts  = $cal_params->num_apts;
  $canvas_h  = $num_apts * $cell_h;
  $canvas_w  = $num_cols * $grid_w; // use grid_w instead of num_cells * cell_w in case grid_w is rounded to avoid rounding error

  // calculated calendar params

  $cal_params->canvas_h     = $canvas_h;
  $cal_params->canvas_w     = $canvas_w;
  $cal_params->num_cols     = $num_cols;
  $cal_params->num_cells    = $num_cells;

  // this is the org_cell_width unaffected by auto scale and post scale - used for apartment titles and stats
  $org_cell_w2 = round($cal_params->cell_width * $cal_params->zoom);

  if($cal_params->cal_show['rentals']) {
    $cal_zoom = $cal_params->zoom;
    $title_w = $first_col_w = 150 * $cal_zoom;
    $cal_params->title_w = $title_w; // total, apts column
  } else {
    $first_col_w = $cal_params->first_col_w = $cal_params->w1 = $cal_params->w2 = $cal_params->title_w = 0;
  }
  
  $cal_params->cal_w          = $canvas_w + $first_col_w;

  // PHP vars used in javascript
  $cal_params->res_x_offset   = $cal_params->target_day ? 0 : round($grid_w * 0.5);
  $cal_params->res_y_offset   = round($cell_h * 0.25);

  $cal_params->font_size      = round($cell_h * 0.75);

  //dump($cal_params);
  //echo("cell_w=".$cal_params->cell_w. " widht=".$cal_params->cell_width ." zoom=".$cal_params->zoom);
  
  return $cal_params;
}


function get_calendar_exceptions($cal_params) {
  $cal_params->exceptions = $exceptions = array();
  
  $apt_ids = $cal_params->apt_ids;
  $host_ids = $cal_params->host_ids;
  $host_count = $cal_params->host_count;
  $in = strip_time($cal_params->start);
  $out = strip_time($cal_params->end);
  $apt_list = $cal_params->apt_list;
  if(!$apt_list) return $cal_params;
  $host_id = $host_count == 1 ? $host_ids[0] : 0;
  
  $cal_params->exceptions = date_exceptions($in, $out, $host_id, $apt_ids);  
  $cal_params->exception_ids = &array_find_key($cal_params->exceptions, 'id');
  $cal_params->exception_count = count($cal_params->exception_ids);
  return $cal_params;
}


function get_calendar_properties($cal_params) {
  $u_level = $cal_params->user_level; 

  $errors = array();
  
  $search = array_filter($cal_params->property_search) ?: array();

  //$options['debug_query'] = true;
  
  if(!$cal_params->show_inactive) $search['active'] = 1;
  $search['source_id'] = 0;
  $search['test'] = 0;
  
  //dump($search);  
  $options['sort'] = $cal_params->sort ?: 'host_id, address_street, address_number, address_suffix';
  $options['limit'] = $cal_params->limit ?: 100;

  $obj_type = $cal_params->property_type ?: 'shortstay';
  $apts = find_objects($obj_type, $search, $options);
  
  
  $apt_ids = &array_find_key($apts, 'id');
  $in = $cal_filters['in']; 
  $out = $cal_filters['out']; 
  if($in && $out) { // filter out based on availability
    $booked_ids = booked_apartments($in, $out, $type, $city_id, $apt_ids);
    if(count($booked_ids) >= count($apts)) {
      if($host_id = $cal_filters['host_id']) {
        $host_str = "by ".$cal_params->host_name;
        $all_hosts_link = html_break().html_link(replace_uri_var('', 'host', 0), 'Try all apartments in '.get_object('geo_city', $cal_params->city_id, 'name'));
      }
      $cal_params->messages = print_errors("No apartments $host_str available from $in to $out".($ng > 1 ? " for $ng guests" : "").$all_hosts_link); 
    } else {
      $avail_ids = array_diff($apt_ids, $booked_ids);
      $apts = find_objects($type, array('id' => $avail_ids), array('sort' => $cal_params->sort, 'order' => $cal_params->order));
      $apt_ids = $avail_ids;        
    }
  }

  $aptArrray = array(); // store as assoc for select box
  if($u_level >= USER_LEVEL_STAFF && count($apt_ids) > 1) { // add "parking lot" - unknown apartment
    array_unshift($apt_ids, '0');
    $aptArray[''] = phrase('all', CAPITALIZE);
  }

  $host_ids = $city_ids = array();
  foreach($apts as $apt) {
    $aptArray[$apt->id] = $apt->name;
    $host_ids[$apt->host_id] = 1;
    $city_ids[$apt->city_id] = 1;
  }
  $cal_params->host_ids = array_keys($host_ids);
  if(count($host_ids) == 1) $cal_params->host_id = $cal_params->host_ids[0];

  $cal_params->city_ids = array_keys($city_ids);
  if(count($city_ids) == 1) $cal_params->city_id = $cal_params->city_ids[0];

  $cal_params->host_count = count($host_ids);
  
  $cal_params->apt_ids = $apt_ids;
  $cal_params->apt_list = $apt_list = $apt_ids ? implode(',', $apt_ids) : '';
  $num_apts = count($apt_ids);
  $cal_params->num_apts = $cal_params->num_rows = $num_apts; 
  $cal_params->apts = $aptArray;
  
                                                                                                            
  return $cal_params;  
  // return array($apts, $apt_ids);
}    

function calendar_find($type, $name_or_id) {
  $link = '';
  $count = 0;
  if(is_numeric($name_or_id)) { // id
    $id = $name_or_id;
    if($type == 'reservation') { // reservation mode, so stay here even if invoice is found
      if($resObj = get_object($type, $id)) {
        $count = 1;
      } else if($invObj = get_object('invoice', $id)) { // try invoice ID
        $message = "Did not find $type $name_or_id, but found an invoice with this ID";
        $count = 1; 
      } else if($invObjs = find_objects('invoice', array('invoice_id' => $id))) { // try invoice number
        $invObj = $invObjs[0];
        $count = count($invObjs); 
        $message = "Did not find $type $name_or_id, but found an invoice with this ID";
      }
      if($res_id = $invObj->res_id) $resObj = get_object('reservation', $res_id);
    } else {
      if($resObj = get_object($type, $id)) $count = 1;
    }
  }
  if(!$count) { // name
    $name = $name_or_id;
    if($resObjs = find_objects($type, array('last_name' => "*$name*"))) {
      $count = count($resObjs);
      $resObj = $resObjs[0];
    } else if($resObjs = find_objects($type, array('first_name' => "*$name*"))) {
      $count = count($resObjs);
      $resObj = $resObjs[0];
    } else if($type == 'invoice' && ($resObjs = find_objects($type, array('invoice_id' => $name)))) {
      $count = count($resObjs);
      $resObj = $resObjs[0];
    }
  }
  if($count == 1) {
    $link = calendar_link($type, $resObj);
  } elseif($count) {
    $link = get_object_link($type)."&_search_address.last_name=*".$name."*&search=Search&limit=$count";
  } else {
    $message = "Did not find $type with name or ID = $name_or_id";
  }
  return array($link, $count, $message);
}

function add_calobj_helperfields($cal_params, $resObj, $options=NULL) {
  global $eventTypes;
  global $eventTypeArray;
  global $u_level;
  $user_level = $cal_params->user_level ?: $u_level;
  
  $extended = $options['extended'] ?: false; // load all (time intensive)
	$links = [];
  
  $load = $options['load']; // to load extra 
  
  $obj_type = $cal_params->obj_type ?: 'reservation';
  
  //list($start_field, $end_field) = object_timefields($obj_type);
  
  $start_field = 'checkin';
  $end_field = 'checkout';

  $resObj->in = $start_field;
  $resObj->out = $end_field;
  $resObj->obj_type = $obj_type;
  $resObj->base_class = $base_class = $eventTypes[$obj_type] ? 'event' : $obj_type;

  $apt_ids = $cal_params->apt_list ? explode(',', $cal_params->apt_list) : array();
  if($apt_id = $resObj->apt_id) $resObj->apt_index = array_search($apt_id, $apt_ids);

                        
  switch($obj_type) {  
  case 'reservation':
    $resObj->id = $resObj->res_id;
    break;
  case 'invoice':
    break;
  case 'event':
    break;
  case 'res_exception':
    break;
  default:
    break;
  }
  
  $id = $resObj->id;
  
  // the below use APC, no DB
  if($resObj->status) {
    if($resObj->status_type = $status_type = object_status_objtype($obj_type)) {
      $statusObj = get_object($status_type, $resObj->status);
      $resObj->status_name = $statusObj->name;
      $resObj->status_color = $statusObj->color;
    }
  }

  if($resObj->type) {
    if($resObj->type_type = $type_type = object_type_objtype($obj_type)) {
      $resObj->type_name = get_object_field($type_type, $resObj->type, 'name');
    }
  }

  // permissions
  $other = $show && $show != $cal_params->base_class;
  if($other) {  // no edit if showing res in event mode or vice versa
    $resObj->auth_view   = false;
    $resObj->auth_edit   = false;
    $resObj->auth_delete = false;
    $resObj->auth_other = "Other mode: no auth of $obj_type $id (show='$show' ot=$cal_params->obj_type";
    $resObj->other   = 1;
  } else {
    $resObj->auth_view   = 1; //auth_res('view', $resObj);
    $resObj->auth_edit   = 1; //auth_res('edit', $resObj);    
    $resObj->auth_delete = 1; //auth_res('del',  $resObj);    
    $resObj->auth_other = 0;
    $resObj->other   = 0;
  }
  
  $auth_level = auth_res($resObj);
  
  if($resObj->auth_view) {
    switch($obj_type) {                 
      case 'reservation' :
  		  $resize = $cal_params->target_day || $resObj->num_days; // allow resize unless hourly reservation in month view
        $res_text = $resObj->last_name ?: $resObj->first_name;
        //$res_text = 'x';
        break;
      case 'res_exception' :             
        $resObj->status = STATUS_RESERVED;
        $resObj->status_name = 'Reserved';
        $res_text = $resObj->description;
        $resize   = true;
        break;
      case 'invoice' :
        $resize   = false;
        $res_name = $resObj->guest_name;
        $amount   = number_format($invObj->total, 2);
        if($sequence = $resObj->sequence) $res_name = "$sequence: $res_name";
        $res_text = $org_name ? "$res_name ($org_name)" : $res_name;
        if($amount > 0) $res_text .= " [$cur_sym $amount]";
        break;
  //    case 'event' :
  		default: // event
  			if(!($eventTypes[$obj_type] || $obj_type == 'event')) die("helper_fields: unknown obj_type $obj_type");
  		  $resize = $cal_params->target_day || $resObj->num_days; // allow resize unless hourly reservation in month view
        $res_text = "Event $id";
        $res_text = '';
        // $evt_type = $resObj->type ? get_object('event_type', $resObj->type, 'name') : '';
        // $classes[] = $evt_type;
  			$resObj->base_class = 'event';
        break;
  //      die("cal helper fields: Unknown type $cal_params->show");
    }

  } else { // no auth view - unset members so can't be seen in JS ?
  }

  if($obj_type == 'res_exception') {
    $resObj->notes = print_exception($resObj);
    //return $resObj;
  }
  
	$resObj->calendar_resize = $resize;
	$resObj->calendar_title = $res_text;
  $status_page = get_page_link(PAGE_STATUS);
  $invoice_page = get_page_link(PAGE_INVOICE);

	if($extended) { // load extended time intensive data for cal-menu on demand
	  $resObj->extended = true;
	  
    $resObj->ts  = $u_level >= USER_LEVEL_OWNER ? myhash(stripdate($resObj->created)) : '';
    $resObj->pin = $u_level == USER_LEVEL_GUEST || $u_level >= USER_LEVEL_STAFF ? myhash($resObj->booking_pin) : ''; // hide booking PIN unless staff or higher
  
    $cal_link = get_page_link(PAGE_CALENDAR);
    $cal_name = phrase('calendar', CAPITALIZE);
                                                                              
    // action buttons on top of menu
    if($obj_type != 'res_exception') $resObj->action_buttons = add_href_target(recordset_buttons($obj_type, $resObj, '', '', $cal_name, $cal_link), 'status');
  
    // action links on bottom of menu
    $hidden_fields = array_filter(array('t' => $obj_type, 'id' => $id, 'ref_link' => $cal_link, 'ref_name' => $cal_name, 'ts' => $resObj->ts, 'pin' => $resObj->pin));
    
    if($obj_type == 'res_exception') return $resObj;
    
    
    $base_link = $status_page.'?'.array2qs($hidden_fields);
    $status_link = $status_page.$resObj->res_id;
    
    $links[]= html_link($status_link, html_i("", "fa fa-eye")." Reservation Details", ["target" => "status"]);         
    if($auth_level >= USER_LEVEL_OWNER) {
      if($resObj->status == STATUS_BLOCKED) {
        $selector = "#reservation-$id";
        $links[]= html_link("#", html_i("", "fa fa-times-circle")." Delete", 
          array("class" => "ajax-link", 
                "data-data" => array("obj_type" => "reservation", "id" => $resObj->res_id), 
                "data-handler" => "delete",
                "data-success_handler" => "delete_object_handler", 
                "data-handler_data" => json_encode(array("target" => $selector))
                )
          
          );
      } elseif($resObj->status >= STATUS_PROPOSED) {
        $links[]= html_link("$status_link?view=_booking-status-cancel.html", html_i("", "fa fa-times-circle")." Cancel Reservation", ["target" => "status"]);         
      }      
    }
    
    // invoice action links for this reservation
    if($obj_type == 'reservation') {
      $invObjs = find_reservation_invoices($id);
      if($blockObj && ($blockInv=find_block_invoice($block_id))) $invObjs[] = $blockInv;
      foreach($invObjs as $invObj) {
        $invoice_id = $invObj->invoice_id ?: $invObj->id;
        $inv_link = $invoice_page.$invObj->id;
        $links[] = html_link($inv_link, html_i("", "fa fa-money")." Invoice $invoice_id", ["target" => "invoice"]);
      }
    }
    
    
    /** no longer in use
    $resObj->action_links .= add_href_target(enq_actions('reservation', $id, $base_link, $u_level, true), 'status');
    $resObj->status_link = add_href_target(action_link($obj_type, $id, array('user_level' => $u_level)), 'status'); // link to status page
    */
    
    if($resObj->source_id) $resObj->source_name = get_object('source', $resObj->source_id, 'name');
    if($resObj->site_id) $resObj->site_name = add_href_target(html_link(public_action_link($id, $obj_type, array('user_level' => $user_level)), get_object('site', $resObj->site_id, 'name')), 'status');
    if($resObj->aff_id) $resObj->aff_name = get_object('affiliate', $resObj->aff_id, 'affiliate_name');
    if($agent_id = $resObj->agent_id) {
      $agent_type = get_object('agent', $agent_id, 'type');
      $resObj->agent_name = get_object('agent', $resObj->agent_id, 'name');
      if($agent_contact_id = $resObj->agent_contact) {
        $contactObj = get_object('contact', $agent_contact_id);
        $resObj->agent_name .= " (".get_name_string($contactObj).")";
      }
    }
  
    // echo("cp: $cal_params->obj_type resObj: $resObj->obj_type, show=$show other=$other auth_view=$resObj->auth_view<br>");  
    if($org_id = $resObj->org_id)  $resObj->org_name = get_object_field('org', $org_id, 'org_name');       // used by Ajax chooser - todo; let the chooser handle it
    if($cnt_id = $resObj->country) $resObj->country_name = get_object_field('countries', $cnt_id, 'name'); // used by Ajax chooser - todo; let the chooser handle it
    
	}
	
	
  // these do require DB lookup, but doesn't happen often	
	if($block_id = $resObj->block_id) $blockObj = get_object('block', $block_id);
	
	// block info
	if($blockObj) {
	  $resObj->block_grand_total = $blockObj->grand_total;
	  $resObj->block_paid = $blockObj->paid;
	  $resObj->block_balance = $blockObj->balance;
	}
	
  $resObj->guest_name = get_name_string($resObj);
  $resObj->action_links = implode(html_break(), $links);
  return $resObj;
  
  
  
  
	if(!$extended) return $resObj;

  // db/server/time intenstive, only load on demand

  $u_level = $cal_params->user_level; 

  $obj_type = $show ?: $cal_params->obj_type;
	if($obj_type == 'event' && $resObj->type) $obj_type = strtolower($eventTypeArray[$resObj->type]);

	if($block_id = $resObj->block_id) $blockObj = get_object('block', $block_id);
                            
  $resObj->apt_index = objarray_search($cal_params->apts, 'id', $resObj->apt_id);
  $resObj->action_links = '';

  list($start_field, $end_field) = object_timefields($obj_type);
  $start_field = 'checkin';
  $end_field = 'checkout';
	// $start_field = $cal_params->start_field;
	// $end_field   = $cal_params->end_field;

  $resObj->in = $resObj->$start_field;
  $resObj->out = $resObj->$end_field;

  $key = get_object_key($obj_type);
  if($key != 'id') $resObj->id = $resObj->$key; // hack to also use id instead of res_id for reservations, for consistency
  $resObj->obj_type = $obj_type;
  $resObj->base_class = $base_class = $eventTypes[$obj_type] ? 'event' : $obj_type;
  $id = $resObj->id;

  if($resObj->status) {
    if($resObj->status_type = $status_type = object_status_objtype($obj_type)) {
      $statusObj = get_object($status_type, $resObj->status);
      $resObj->status_name = $statusObj->name;
      $resObj->status_color = $statusObj->color;
    }
  }

  if($resObj->type) {
    if($resObj->type_type = $type_type = object_type_objtype($obj_type)) {
      $resObj->type_name = get_object_field($type_type, $resObj->type, 'name');
    }
  }

  // first - let's add some helpful info to our resObj
  if($resObj->user_id) {
    $userObj             = get_object('user', $resObj->user_id);
    $resObj->user_type   = get_object_field('user_level', $userObj->user_level, 'user_type');
    $resObj->author_name = $userObj->first_name." ".$userObj->last_name;
  }

  if($resObj->apt_id) $resObj->apartment_name = get_object_field('property', $resObj->apt_id, 'name');
  if($resObj->guest_id) {
    $guestObj = get_object('user', $resObj->guest_id);
    $guest_name = get_name_string($guestObj);
  } else { //if(!$resObj->guest_name) {
  	$guest_name = get_name_string($resObj);
  }

  if($agent_type == AGENT_TYPE_HOTEL && $resObj->title) $resObj->guest_name = "$guest_name - ".phrase('room', CAPITALIZE).' '.$resObj->title; // the agent is a hotel, show room field
  else $resObj->guest_name = $guest_name;
  return $resObj;
}

// generic function to get all objects in range with at least status $status
function calendar_entries($start, $end, $apt_ids, $options = array()) {
  
  $obj_type = 'reservation';
  $ptype   = isset($options['property_type']) ? $options['property_type'] : 'shortstay';
  $host_id = isset($options['host_id']) ? $options['host_id'] : 0;
  $status  = isset($options['status']) ? $options['status'] : STATUS_PROPOSED;

  list($start_field, $end_field) = object_timefields($obj_type);
  $start_field = 'checkin';
  $end_field = 'checkout';
  
  $searchArray = ['client_id' => $_SESSION['client_id']];
  $searchArray[$start_field]  = array('<', $end);
  $searchArray[$end_field]    = array('>=', $start);
  $searchArray['status']      = array('>=', $status);
  $searchArray['property_type'] = $ptype;
  if($apt_ids) $searchArray['apt_id'] = $apt_ids;    
  if(is_numeric($host_id) && $host_id > 0) $searchArray['host_id'] = $host_id;
  
  $sort = "status DESC, $start_field";
                                             
  $key = get_object_key($obj_type);
  $table = get_object_table($obj_type);
  
  $debug = 0;
  $resObjs = find_objects($obj_type, $searchArray,  array('sort' => $sort, 'debug_query' => $debug));

  $_SESSION['calendar_search'] = $searchArray; // used by change link 
  return $resObjs;

  
  /**
  switch($obj_type) {
  case 'res_exception':
    $sort = "$start_field";
    if($host_id) {
      $host_ids = array($host_id);
    } elseif($apt_list) {
      $host_ids = find_objects($ptype, array('id' => $apt_ids),  array('fields' => 'DISTINCT host_id'));
    } else {
      die("No apts or hosts");
    }
    //echo("hosts = ".dump($host_ids, true));
    $searchArray[DB_LOGICAL_OR] = array('apt_id' => $apt_ids, 'host_id' => $host_ids); // 100 is dummy org for particulier 
    break;
  default:
    $sort = "status DESC, $start_field";
    $searchArray['status']      = array('>=', $status);
                                             
    $searchArray['property_type'] = $ptype;
    if($apt_list) $searchArray['apt_id'] = $apt_ids;    
    else if(is_numeric($host_id) && $host_id > 0) 
      $searchArray['host_id'] = $host_id;
    else 
      unset($searchArray['host_id']);
    break;
  } 
  */
  

  /**
  if($use_cache && $apc_key) {
    $id_list = implode(',', $ids);    
    $result = apc_store($apc_key, $id_list);
    if($result) {
      print_log(now().": Stored $apc_key: ".$id_list, 'apc');
    } else {
      print_log(now().": Failed to sto
re $apc_key: ".$id_list, 'apc');
    }
  }
  */
  
  //dump($result);
}

// returns a scaling factor that will reduce x size depending on how excessive it is
function auto_scale_factor($excess) {
  if($excess >= 1600) return 1/8;
  else if($excess >= 800) return 2/8;
  else if($excess >= 400) return 3/8;
  else if($excess >= 200) return 4/8;
  else if($excess >= 96) return 5/8;
  else if($excess >= 48) return 6/8;
  else if($excess >= 24) return 7/8;
  else return 8/8;
}
                                     


function calendar_menu_items($cal_params) {
  global $u_level;

  // which menus to show
  $menus = array();
  $user_level = $cal_prams->user_level ?: $user_level;
  if($cal_params->target_day) { // show day(s)
    $menus = array('period', 'status', 'show', 'mode', 'resolution');
    if($cal_params->period == 1 || $cal_params->stack == 'v') {
      $menus[] = 'start';
      $menus[] = 'end';
    }
  } elseif($cal_params->target_week) { // show week(s)
    //$menus = array('period', 'status', 'show', 'mode');
    $menus = array('period', 'status');
  } else { // show month(s)
    //$menus = array('period', 'status', 'show', 'mode');
    $menus = array('period', 'status');
    $this_month = $cal_params->target_year == date('Y') && $cal_params->target_month == date('m'); 
    if($this_month) $menus[] = 'starting';  
  }
  //if($cal_params->period > 1) $menus[] = 'stack';
  $menus[] = 'filters';
  
  return $menus; 
  
  // build assoc array of items
  $siteObj = $_SESSION['siteObj'];
  foreach($menus as $menu) {
    $title = phrase($menu, CAPITALIZE);
    $sub_menu = html_link('#', $title.html_span('', 'garrow'), array('title' => $title, 'class' => 'left gdropdown_medium gbutton'));
    
    $sub_menu .= function_exists($menu."_menu") ? call_user_func($menu."_menu" ,$cal_params) : 'no sub';

    $items[] = $sub_menu;
    $item_classes[] = $hide_class;
  }
  
  return $items;    
}

// used for cal menu
function calendar_menu($cal_params, $items) {
	$menu_items = array();
	foreach($items as $item) $menu_items[phrase($item, CAPITALIZE)] = call_user_func($item."_menu" ,$cal_params);
	$title = 'Menu';	
	$jqOptions = array();
	$ul = html_ul($menu_items, '', array('id' => 'cal_topmenu_list'));
	return jquery_menu($ul, '', $jqOptions, array('id' => 'cal_topmenu'));
}

// takes an associative array like this ('EUR' => 10, 'USD' => 20) and prints one line for each currency
function print_mulitple_currencies($ar, $bold = false, $format=FORMAT_IF_NEEDED, $append='') {
  if(!$ar) return '';
  if(!is_array($ar)) mydie("print_mulitple_currencies: input is not array");
  // if($append && !is_array($append)) mydie("print_mulitple_currencies: append is not array");
  ksort($ar);
  $lines = array();
  foreach($ar as $currency => $amount) {
    $ap = is_array($append) ? $append[$currency] : $ap;
    $line = formatFloat($amount, $format, get_currency_symbol($currency, true)).$ap;
    $lines[] = $bold ? html_b($line) : $line;
  }
  return implode(html_break(), $lines);
}


function calendar_cache_filter($filters) {
  $this_year = date("Y");
  $last_year = $this_year - 1;
  $next_year = $this_year + 1;
  return $filters['y'] >= $last_year && $filters['y'] <= $next_year;
}

function calendar_cache_key($cal_params) {
  $dev = $_SESSION['dev'] ? "dev-" : '';
  $filters = $cal_params->filters;
  $filter_vars = array('t', 'show','cid','host_id','p','y','m','d','w', 'st');
  $apc_vars = array("cal");
  foreach($filter_vars as $var) $apc_vars[] = "$var:".$filters[$var];
  $apc_key = $dev.implode('-', $apc_vars);
  return $apc_key;
}


function get_changes($obj_type, $search) {
  global $user_level_array;

  $limit = 0;
  $ids = $search['id'] ?: array(); 
  if($ids) {
    $searchArray['parent_id'] = is_array($ids) ? $ids : explode(',', $ids);
  } elseif($search) {
    $key = get_object_key($cal_params->obj_type);
    $key = $obj_type == 'reservation' ? 'res_id' : 'id'; 
    $searchArray['parent_id'] = find_objects($obj_type, $search,  array('fields' => $obj_type.'.'.$key));
  } 
  //if(!$ids) return array();
  
  //$searchArray['key'] = $key;
  $searchArray['obj_type'] = $obj_type;
  //return $searchArray;
  
  $changes = find_objects('changelog', $searchArray, array('sort' => 'TIMESTAMP DESC', 'limit' => $limit));
  return $changes;
}


function get_stats($cal_params) {
  if(is_array($cal_params)) $cal_params = array2obj($cal_params);
  
  if($cal_params->property_type == 'boat') return get_boat_stats($cal_params);
  
  $u_level = $cal_params->user_level; 
  $cols = $cal_params->num_cols;
  $apt_ids = $cal_params->apt_list ? explode(',', $cal_params->apt_list) : array();

  $num_apts = count($apt_ids); // deduct the ? apt (removed)  

  $squares = $cols * $num_apts;
  $grid = array();
  $end_date = $cal_params->end_date['date'];
  $start_date = $cal_params->start_date['date'];
  $software_rate = strip_time($start_date) < '2014-02-01' ? 0.05 : 0.01;

  $res_ids = $cal_params->res_list ? explode(',', $cal_params->res_list) : array();

  $obj_type = $cal_params->obj_type;
  // create grid for each square (date/apt) with occupancy and rate
  foreach($res_ids as $res_id) {
    $resObj = get_object($obj_type, $res_id);    
    if(!$resObj || !$resObj->apt_id) continue; // no stats for ? apt
    if(!in_array($resObj->status, array(STATUS_RESERVED, STATUS_COMPLETE))) continue; // only count reserved and complete
    $id = $resObj->res_id;
    $currency = get_city_currency($resObj->city_id);
    if($resObj->currency != $currency) {
      //echo("Warning: reservation $id is denoted in currency $resObj->currency; should be $currency. Skipping this reservation for statistics purposes.");
      continue;
    }

    $in = substr($resObj->checkin, 0, 10);
    $out = substr($resObj->checkout, 0, 10);
    $apt_id = $resObj->apt_id;

    $total = $resObj->total;
    $paid = $resObj->paid;
    $due  = $resObj->balance;
    $extras = $resObj->other_charges;
    $grand_total = $resObj->grand_total;

    // paid and due
    $stats[$apt_id]['paid'] += $paid;
    $stats[$apt_id]['due']  += $due;

    $total_paid += $paid;
    $total_due  += $due;

    // $occ += $days;


    if($cal_params->property_type == 'boat') {
      $hours = $resObj->num_hours;
      $total_hours += $hours;

    } else {
      $days = $resObj->num_days;
      $total_days += $days;
      
      $stats[$apt_id]['res_count']++;
      $stats[$apt_id]['res_days'] += $days;
  
      $day_rate = $days ? $resObj->total / $days : 0;
      if($resObj->commission && $days) {
        list($abs_commission, $perc_commission) = split_discount($resObj->commission);
        $total_commission = $abs_commission ? $abs_commission : $total * ($perc_commission/100);
        $day_comm = $days ? $total_commission / $days : 0;
        $day_net = $day_rate - $day_comm;
        //echo("res $id $in-$out: comm = '$resObj->commission' ($perc_commission): of $total = $total_commission  = $day_comm per day ($days days(<br />");
      } else {
        $day_comm = 0;
        $day_net = $day_rate;
      }
  
      if($out > $in && !$days) { // num days missing, fix
        $num_days = get_num_days($in, $out);
        db_query("UPDATE reservation SET num_days=$num_days WHERE res_id=$id");
      }
      if($day_rate == 0) $day_rate = '0.00';

      $calendar_dates = get_calendar_dates($cal_params);
      foreach($calendar_dates as $dateArray) {
        $date = $dateArray['date'];
        $val = $grid[$apt_id][$date]; // what's already there
        if($val >= $day_rate) continue; // already filled with higher rate (if overlap, pick highest rate)
        if($date < $in) continue; // previous period, skip
        if($date >= $out) break; // end of period, break
        //echo("$apt_id: $date is within reservation $id: $in - $out; rate = $day_rate (val='$val')<br />");
  
        // fees are counted on day of checkin
        $res_fee  = ($resObj->fees > 0 && $in == $date) ? $resObj->fees : 0;
        // echo("Res $id $in-$out ($apt_id) starts in this period and has $res_fees fees <br />");
  
        $grid[$apt_id][$date] = array($day_rate, $day_comm, $day_net, $res_fee);
      }
    }

    //echo("<hr>");

  }

  //if(isset($_GET['foo'])) dump($grid);

  // traverse grid to find occupancy etc

  foreach($grid as $apt_id => $date_array) {
    $occ=$apt_reveneue=$apt_comm=$apt_net=$apt_fees = $paid_days=0;
    foreach($date_array as $rate_array) {
      $rate = $rate_array[0];
      $comm = $rate_array[1];
      $net  = $rate_array[2];
      $fees = $rate_array[3];
      if($rate) {
        $occ++; // also counts if rate = 0.00
        if($rate > 0) $paid_days++;
        $apt_reveneue  += $rate; // apt revenue
        $apt_comm += $comm; // apt commission
        $apt_net  += $net;  // apt net
        $apt_fees += $fees;  // apt fees
      }
    }
    $stats[$apt_id]['occupied']   = $occ;                 // # booked days (also blocked)
    $stats[$apt_id]['paid_days']  = round($paid_days);    // # booked days where rate is > 0 (paid_days days)
    $stats[$apt_id]['occupancy']  = $cols ? round($occ / $cols * 100).'%' : 0;
    $stats[$apt_id]['revenue']    = round($apt_reveneue);
    $stats[$apt_id]['avg_rate']   = $paid_days ? round($apt_reveneue / $paid_days) : 0;
    $stats[$apt_id]['commission'] = round($apt_comm);
    $stats[$apt_id]['net']        = round($apt_net);
    $stats[$apt_id]['fees']       = round($apt_fees);

    $stats[$apt_id]['paid'] = round($stats[$apt_id]['paid']); // round and print
    $stats[$apt_id]['due']  = round($stats[$apt_id]['due']); // round and print

    $stats[$apt_id]['length'] = $avg_length = round($stats[$apt_id]['res_days'] / $stats[$apt_id]['res_count'], 1);  // average length of stay

    //echo("$apt_id: revenue = $apt_reveneue paid_days=$paid_days avg = $avg<br />");
    //echo("$apt_id: occ = $occ / $cols = ".$stats[$apt_id]['occupancy']."<br />");

    // add to totals
    $total_apt_revenue  += $apt_reveneue;
    $total_apt_comm     += $apt_comm;
    $total_apt_net      += $apt_net;
    $total_occ          += $occ;
    $total_apt_fees     += $apt_fees;
    $total_paid_days    += $paid_days;
    $total_length       += $avg_length;
  }

  // totals
  $stats['total']['occupied']   = $total_occ;
  $stats['total']['revenue']    = round($total_apt_revenue);
  $stats['total']['commission'] = round($total_apt_comm);
  $stats['total']['net']        = round($total_apt_net);
  $stats['total']['fees']       = round($total_apt_fees);
  $stats['total']['paid_days']  = round($total_paid_days);
  $stats['total']['avg_rate']   = $total_paid_days ? round($total_apt_revenue / $total_paid_days) : 0;
  $stats['total']['occupancy']  = $squares ? round(($total_occ / $squares * 100),0).'%' : 0;
  $stats['total']['paid']       = round($total_paid);
  $stats['total']['due']        = round($total_due);
  //$stats['total']['length']     = count($resObjs) ? round($total_days / count($resObjs), 1) : 0;
  $stats['total']['length']     = $num_apts ? round($total_length / $num_apts, 1) : 0;

  $stats['total']['software_fee'] = round($total_apt_net * $software_rate);      
  $stats['total']['city_id']    = $cal_params->city_id;
  $stats['total']['currency']   = $currency;
  $stats['total']['start_date'] = $start_date;
  $stats['total']['end_date']   = $end_date;

  // echo("Total occ = ".$stats['total']['occupied']." / $squares = ".$stats['total']['occupancy']."<br />");
  //echo("There are $cols days x $apts apts = $squares squares used: $total_occ revenue: $total_apt_revenue paid=$total_paid  <br />");

  // finally, loop through and round + add currency sign (or % sign)

  return $stats;
}

// returns calendar stats for given host
function get_host_stats($hostObj_or_id, $start, $options=NULL) {
  $debug  = $options['debug'];
  $period = $options['period'] ?: 1; // default is 1 month
  
  $hostObj = any2obj('user', $hostObj_or_id);
  $host_id = $hostObj->id;
    
  $ptype = 'shortstay';
  $city_id = 1000;

  // set up calendar parameters
  list($year, $month, $day) = explode('-', $start);
                                     
  $cal_params->filters = array();
  $cal_params->host_id = $host_id;
  $cal_params->city_id = $city_id;
  $cal_params->user_level = USER_LEVEL_ADMIN;
  $cal_params->property_type = $ptype;
  $cal_params->show = 'reservation';
  $cal_params->start = $start;
  //$cal_params->end = $end;
  $cal_params->status = STATUS_RESERVED;
  $cal_params->period['m'] = $period;
  $cal_params->target_month  = $month;
	$cal_params->target_year   = $year;
	$cal_params->target_day   = 0;

	$cal_params->def_status['reservation'] = STATUS_RESERVED; 
  $cal_params->follow = 0; 
	$cal_params->show_inactive = $options['show_inactive'];

  $cal_params = get_cal_params($cal_params);
  $cal_params = calendar_init($cal_params);
  
  $cal_params->period = $period;
    
  $apt_ids = $cal_params->apt_list ? explode(',', $cal_params->apt_list) : array();
  $count = count($apt_ids);
    
  $stats = get_stats($cal_params);
  if($debug) echo("<b>get_host_stats for host $host_id staring $start property_type=$ptype city_id=$city_id Fee=".$stats['total']['currency']. ' '.$stats['total']['software_fee']."</b><br>");

  return $stats;
}

function print_exception_text($exObj, $cur='') {
  if($val = $exObj->rate_day) {
    $rate = $cur.$val;
  } else if($val = $exObj->rate_change) {
    list($abs_change, $perc_change) = split_discount($val);
    if($abs_change > 0 || $perc_change > 0) {
      $out_name = phrase('surcharge', CAPITALIZE);
      $rate = $perc_change > 0 ? "+".$val : "+".$cur.$val;
    } else {
      $out_name = phrase('discount', CAPITALIZE);
      $val = ltrim($val, '-');
      $rate = $perc_change < 0 ? "-".$val : "-".$cur.$val;
    }
    //echo("rate=$rate<br>");
  } else {
  }
  
  //$rate = $exObj->rate_day ?: $exObj->rate_change;
  $text = $rate;
  if($ms = $exObj->min_stay) $text .= " Min: {$ms} days.";
  if($co = $exObj->changeover) $text .= " Changeover: ".$_SESSION['weekdays'][$co];
  if($desc = $exObj->description) $text .= " {$desc}";
  return $text;  
}

// prints the colored bar on the calendar for a price/exception
function print_pricing_marker($cal_params, $exObj) {
  if($cal_params->target_day) return ''; // only show in week/month view for now
  $cur = $cal_params->cur;

  $in = strip_time($exObj->start);
  $out = strip_time($exObj->end);  
  $apt_id = $exObj->apt_id;
  list($h, $w, $top, $left) = res_marker_pos($cal_params, $in, $out, $apt_id);
  
  $text = print_exception_text($exObj, $cur);
  $class = 'priceBar';
  $style = "width:".$w."px;height:".$h."px;top:".$top."px;left:".$left."px;";  

  // begin output
  $output = html_div(html_span($text, 'price-text'), $class, array('style' => $style));
  //$output .= $icon.$tips;
  //$output .= html_div($res_text, 'res-text', array('width' => round(0.9*$w)));
  
  return $output;  
}

function res_marker_pos($cal_params, $in, $out, $apt_id=0) {
  $apt_ids   = $cal_params->apt_ids;
  $cell_h    = $cal_params->cell_h;
  $cell_w    = $cal_params->cell_w;
  $grid_w    = $cal_params->grid_w;
  $canvas_w  = $cal_params->canvas_w;
  $canvas_h  = $cal_params->canvas_h ?: count($apt_ids) * $cell_h;
  $start     = $cal_params->start;
  $end       = $cal_params->end;
  $cols      = $cal_params->num_cols;
  
  // top and height
  if($apt_id) { // one property
    $apt_index = array_search($apt_id, $apt_ids);
    $top = $apt_index * $cell_h;
    $h = $cell_h;
  } else { // all properties
    $top = 0;
    $h = $canvas_h;
  }

  //$resObj->date_index = objarray_search($cal_params->apts, 'id', $resObj->apt_id);

  
  $start_ts     = strtotime($start);
  $end_ts       = strtotime($end);
  $total_diff   = $end_ts - $start_ts;
  $sec_per_pix  = $total_diff / $canvas_w;
  $in_ts        = strtotime($in) - $start_ts;
  $out_ts       = strtotime($out) - $start_ts;

  // left and width
  $left = round($in_ts/$sec_per_pix) + 1;
  $right = round($out_ts/$sec_per_pix);

  if($left < 0) $left=0;           
  if($right > $canvas_w) $right=$canvas_w;

  $w = $right-$left;
  
  return array($h, $w, $top, $left);
}


// used for search
function calendar_search($cal_params) {
  return html_link(get_object_link($cal_params->obj_type)."&amp;_show_search=1", phrase('advanced', CAPITALIZE).' '.phrase('search'));
}

// menu items
function generic_menu($cal_params, $menu_items, $cal_var, $uri_var) {
  $items = array();
  $is = $cal_params->request_uri;
  //if($uri_var == 'p') {
  //  dump($menu_items);
  //  dump($cal_params);
  //}
  foreach($menu_items as $var => $desc) {
    $sub_menu = NULL;
    if(is_array($desc)) {
      $sub_menu = $desc[1];
      $desc = $desc[0];
    }
    //if($uri_var == 'p') echo(" cal_var=$cal_var (".$cal_params->$cal_var.") var=$var<br>");
    $link = replace_uri_var($is, $uri_var, $var);
    $st_link = $var == $cal_params->$cal_var ? html_link($cal_params->base_link, html_span($desc, 'ui-state-active'), array('class' => 'noclick')) : html_link(replace_uri_var($is, $uri_var, $var), $desc, array('data-value' => $var));
    $items[] = $st_link.$sub_menu;
  }
  return html_ul($items, 'cal_menu', array('data-var' => $uri_var));
}

function start_menu($cal_params) {
  $menu_items = array('0' => phrase('midnight', CAPITALIZE), 6 => "06.00", 8 => "08.00", 10 => "10.00", 12 => phrase('noon', CAPITALIZE));
  return generic_menu($cal_params, $menu_items, 'start_hour', 'sh');
}

function end_menu($cal_params) {
  $menu_items = array(16 => "16.00", 18 => "18.00", 20 => "20.00", 22 => "22.00", 24 => phrase('midnight', CAPITALIZE));
  return generic_menu($cal_params, $menu_items, 'end_hour', 'eh');
}

function starting_menu($cal_params) {
  $menu_items = array('0' => "1st of month", 1 => "1 week ago");
  return generic_menu($cal_params, $menu_items, 'follow', 'f');
}

function resolution_menu($cal_params) {
  $menu_items = array('1'=>'1 '. phrase('hour'),'2'=>'30 '.phrase('minutes'),'4'=>'15 '.phrase('minutes'));
  return generic_menu($cal_params, $menu_items, 'resolution', 'r');
}

function status_menu($cal_params) {
  if($cal_params->obj_type == 'invoice') {
    $menu_items = array(STATUS_ALL=>phrase('all', CAPITALIZE), INV_STATUS_DRAFT=>phrase('draft', CAPITALIZE), INV_STATUS_OPEN=>phrase('open', CAPITALIZE), INV_STATUS_COMPLETE=>phrase('complete', CAPITALIZE));
  } elseif($cal_params->obj_type == 'event') {
    $menu_items = array(STATUS_ALL=>phrase('all', CAPITALIZE), EVENT_STATUS_UNCONFIRMED=>phrase('unconfirmed', CAPITALIZE), EVENT_STATUS_CONFIRMED=>phrase('confirmed', CAPITALIZE), EVENT_STATUS_COMPLETE=>phrase('complete', CAPITALIZE));
  } else {
    $menu_items = array(STATUS_ALL=>phrase('all', CAPITALIZE), STATUS_UNCONFIRMED=>phrase('unconfirmed', CAPITALIZE), STATUS_PROPOSED=>phrase('proposed', CAPITALIZE), STATUS_RESERVED=>phrase('reserved', CAPITALIZE));
  }
  return generic_menu($cal_params, $menu_items, 'status', 'st');
}

// event lists chooser
function mode_menu($cal_params) {
	$menu_items = array(SHOW_CALENDAR =>phrase('calendar', CAPITALIZE), SHOW_LIST => phrase('list', CAPITALIZE));
  return generic_menu($cal_params, $menu_items, 'mode', 'mode');
}

function stack_menu($cal_params) {
	$menu_items = array('h' =>phrase('horizontal', CAPITALIZE), 'v' => phrase('vertical', CAPITALIZE));
  return generic_menu($cal_params, $menu_items, 'stack', 'stack');
}

function filters_menu($cal_params) {
	$menu_items = array('1' =>phrase('show_inactive', CAPITALIZE), '0' => phrase('hide_inactive', CAPITALIZE));
  return generic_menu($cal_params, $menu_items, 'show_inactive', 'si');
}

function show_menu($cal_params) {
  $u_level = $cal_params->user_level; 
	//$menu_items = array('reservation' =>phrase('reservations', CAPITALIZE), 'event' => array(phrase('events', CAPITALIZE), event_menu($cal_params)));
	$menu_items = array('reservation' =>phrase('reservations', CAPITALIZE), 'event' => phrase('events', CAPITALIZE), 'res_exception' => phrase('exceptions', CAPITALIZE));
	if($u_level == USER_LEVEL_OWNER || $u_level >= USER_LEVEL_STAFF) $menu_items['invoice'] = phrase('invoices', CAPITALIZE);
  return generic_menu($cal_params, $menu_items, 'show', 'show');
}

function event_menu($cal_params) {
	global $eventTypeArray;
	foreach($eventTypeArray as $k => $v) {
	  if($k == 1) continue; // "Other"
	  $obj_type = strtolower($v);
	  $menu_items[$obj_type] = phrase($obj_type, CAPITALIZE);
	}
  return generic_menu($cal_params, $menu_items, 'show', 'show');
}


function period_menu($cal_params) {
  $items = array();
  $is = $cal_params->request_uri;
  // months
  if($cal_params->target_day) {
  	$unit_singular  = phrase('day', LOWERCASE);
  	$unit_plural    = phrase('days', LOWERCASE);
    $period_factors = array(1,2,3,5,6,7);
  } elseif($cal_params->target_week) {
  	$unit_singular  = phrase('week', LOWERCASE);
  	$unit_plural    = phrase('weeks', LOWERCASE);
    $period_factors = array(1,2,3,4);
	} else {
  	$unit_singular  = phrase('month', LOWERCASE);
  	$unit_plural    = phrase('months', LOWERCASE);
    $period_factors = array(1,2,3,4,6,12);
	}
  foreach($period_factors as $period) $menu_items[$period] = $period.' '.($period == 1 ? $unit_singular : $unit_plural);
  return generic_menu($cal_params, $menu_items, 'period', 'p');
}

// return an array of months shown
function get_calendar_months($cal_params) {
  $start_day   = $cal_params->start_date['day'];
  $start_month = $month = $cal_params->start_date['month'];
  $start_year  = $year = $cal_params->start_date['year'];
  $end_day     = $cal_params->end_date['day'];
  $end_month   = $cal_params->end_date['month'];
  $end_year    = $cal_params->end_date['year'];
  if(!($start_month && $start_year && $end_month && $end_year)) return array(); 
  $calendar_months = array();
  $mn=$_SESSION['month_names'];
  $first = true;
  $last = false;
  while(true) {
    $days_shown = 0;
    list($next_month, $next_year) = get_next_month($month, $year);
    if($next_year > $end_year || ($next_year == $end_year && $next_month > $end_month)) $last = true;

    $days_in_month = $days_shown = get_daysofmonth($month, $year);
    if($first && $start_day > 1) $days_shown = $days_in_month - ($start_day - 1);
    elseif($last && $end_day < $days_in_month) $days_shown = $days_in_month - ($days_in_month - $end_day);
    
    $calendar_months[] = array('month' => $month, 'year' => $year, 'name' => $mn[$month-1], 'days' => $days_in_month, 'days_shown' => $days_shown);
    if($last) break;
    $first = false;
    $month = $next_month;
    $year = $next_year;
  }
  return $calendar_months; 
}

// return an array dates to display in calendar:
// NOTE todo: wrong day numbering (0=sun, 1=mon, etc)
function get_calendar_dates($cal_params) {
  $period = $cal_params->period;

  if($cal_params->follow && !$cal_params->target_day && !$cal_params->target_week && $cal_params->target_month == $_SESSION['today_month'] && $cal_params->target_year == $_SESSION['today_year']) {
    $target_date->target_day   = 0;
    $target_date->target_week  = 0;
    $week_ago_sql = add_interval(today(), -7);
    list($week_ago_d, $week_ago_m, $week_ago_y) = sql2date($week_ago_sql);
    $first_day = $week_ago_d;
    $target_date->target_month = $week_ago_m;
    $target_date->target_year  = $week_ago_y;
  } else {
    // copy to local var
    $first_day = 1;
    $target_date->target_day   = $cal_params->target_day;
    $target_date->target_week  = $cal_params->target_week;
    $target_date->target_month = $cal_params->target_month;
    $target_date->target_year  = $cal_params->target_year;
  }

	// get first period
	$date_array = $dates = get_calendar_dates2($cal_params, $target_date, $first_day);

	if($period> 1) {     // more than 1 unit in period
  	$date_array = $dates;

  	while(--$period) {
  		$day	 = $target_date->target_day;
  		$week	 = $target_date->target_week;
  		$month = $target_date->target_month;
  		$year	 = $target_date->target_year;

  		// advance to next period
  		if($day) { // return one day only
  		  $today['day'] = $day;
  		  $today['month'] = $month;
  		  $today['year'] = $year;

  			$tomorrow = get_next_day(dates2sql($today));
  			list($next_day, $next_month, $next_year) = sql2date($tomorrow); // get_next_day uses SQL format

  			$target_date->target_day   = $next_day;
  			$target_date->target_month = $next_month;
  			$target_date->target_year  = $next_year;

  		} else if($week) {
  			list($next_week, $next_year) = get_next_week($week, $year);
  			$target_date->target_week  = $next_week;
  			$target_date->target_year  = $next_year;

  		} else {
  			list($next_month, $next_year) = get_next_month($month, $year);
  			$target_date->target_month = $next_month;
  			$target_date->target_year  = $next_year;

  		}
  		$date_array = array_merge($date_array, get_calendar_dates2($cal_params, $target_date, $first_day));
                        
  	}
  	// set target back to original values
  	//$cal_params->target_day   = $cal_params->filters['d'];
  	//$cal_params->target_week  = $cal_params->filters['w'];
  	//$cal_params->target_month = $cal_params->filters['m'];
  	//$cal_params->target_year  = $cal_params->filters['y'];
    //
    //$cal_params->calendar_dates=$date_array;
  }

  return $date_array;
  // get beginning and end time of calendar
  return $cal_params;
}

// same as above, one period only
function get_calendar_dates2($cal_params, $target_date, $first_day) {
  //dump($cal_params);
  //dump($target_date);
	if($cal_params->target_day) { // return one day only

      $this_day = $target_date->target_day;
  		$this_month = $target_date->target_month;
  		$this_year = $target_date->target_year;

  		$weekdaynum = get_weekday($this_day, $this_month, $this_year); //sun=0, mon=1,...sat=6
  		//$weeknum = get_week($weekdaynum, $this_month, $this_year);
  		list($weeknum, $week_year) = get_week_year($this_day, $this_month, $this_year);

		  $calendar_days[0]['day'] = $this_day;
		  $calendar_days[0]['month'] = $this_month;
		  $calendar_days[0]['year'] = $this_year;
		  $calendar_days[0]['week_year'] = $week_year;
		  $calendar_days[0]['week'] = $weeknum;
		  $calendar_days[0]['weekday'] = $weekdaynum==7 ? 0 : $weekdaynum;
		  $calendar_days[0]['weekday_name'] = $_SESSION['weekday_names'][$weekdaynum];
		  $calendar_days[0]['weekday_name_short'] = $_SESSION['weekday_shortestnames'][$weekdaynum];

		  $calendar_days[0]['date'] = get_sql_date($this_day, $this_month, $this_year);

	} elseif($cal_params->target_week) { // return one week only
		$weeknum=$target_date->target_week;
		$year = $target_date->target_year;
		for($i=0;$i<7;$i++) {
			$weekdaynum = $i+1;
			$date = get_date_of_week($year, $weeknum, $weekdaynum);
			list($this_day, $this_month, $this_year) = sql2date($date);
  		list($foo, $week_year) = get_week_year($this_day, $this_month, $this_year);

		  $calendar_days[$i]['day'] = $this_day;
		  $calendar_days[$i]['month'] = $this_month;
		  $calendar_days[$i]['year'] = $this_year;
		  $calendar_days[$i]['week_year'] = $week_year;
		  $calendar_days[$i]['week'] = $weeknum;
		  $calendar_days[$i]['weekday'] = $weekdaynum==7 ? 0 : $weekdaynum;
		  $calendar_days[$i]['weekday_name'] = $_SESSION['weekday_names'][$weekdaynum];
		  $calendar_days[$i]['weekday_name_short'] = $_SESSION['weekday_shortestnames'][$weekdaynum];
		  $calendar_days[$i]['date'] = $date;
      $calendar_days[$i]['today'] = $this_day == $_SESSION['today_day'] && $this_month == $_SESSION['today_month'] && $this_year == $_SESSION['today_year'];
		}

	} else { // return one month

		$month = $target_date->target_month;
		$year = $target_date->target_year;

		// return one month, starting on 1st or today - 3 days
		$days_in_month = get_daysofmonth($month, $year);
		$weekdaynum = get_weekday($first_day, $month, $year); //sun=0, mon=1,...sat=6
		list($weeknum, $week_year) = get_week_year($first_day, $month, $year);
	  $firstweek = $weeknum;

    $day = $first_day;
		for($i=0;$i < $days_in_month;$i++) {
			if($day > $days_in_month) { // advance to next month
        $day = 1;
			  list($month, $year) = get_next_month($month, $year);
			}
		  $calendar_days[$i]['day'] = $day;
		  $calendar_days[$i]['month'] = $month;
		  $calendar_days[$i]['year'] = $year;
		  $calendar_days[$i]['week_year'] = $week_year;
		  $calendar_days[$i]['week'] = $weeknum;
		  $calendar_days[$i]['weekday'] = $weekdaynum;
		  $weekday_index = $weekdaynum==7 ? 0 : $weekdaynum;
		  $calendar_days[$i]['weekday_name'] = $_SESSION['weekday_names'][$weekday_index];
		  $calendar_days[$i]['weekday_name_short'] = $_SESSION['weekday_shortestnames'][$weekday_index];
		  $calendar_days[$i]['date'] = get_sql_date($day, $month, $year);
      $calendar_days[$i]['today'] = $day == $_SESSION['today_day'] && $month == $_SESSION['today_month'] && $year == $_SESSION['today_year'];

			$weekdaynum++;
			if($weekdaynum==8) $weekdaynum=1; 
			if($weekdaynum==1) list($weeknum, $week_year) = get_week_year($day+1, $month, $year); // next week
			$day++;
		}
	}
	//dump($calendar_days);
	return $calendar_days;
}

function get_calendar_start_end($calendar_dates, $cal_params) {
  $d_count = sizeof($calendar_dates);
  $cal_params->end_date = $end_date = $calendar_dates[$d_count-1];
  $cal_params->start_date = $start_date = $calendar_dates[0];
  $end_y    = $end_date['year'];
  $end_m    = $end_date['month'];
  $end_d    = $end_date['day'];
  //dump($calendar_dates);
  if($cal_params->target_day) { // day view
    $cal_params->target_date = date("Y-m-d", mktime(0, 0, 0, $cal_params->target_month, $cal_params->target_day, $cal_params->target_year));
    $cal_params->start = date("Y-m-d H:i:s", mktime($cal_params->start_hour, 0, 0, $cal_params->target_month, $cal_params->target_day, $cal_params->target_year));
    $cal_params->end = date("Y-m-d H:i:s", mktime($cal_params->end_hour, 0, 0, $end_m, $end_d, $end_y));
  } else { // month or week view
    $cal_params->target_date = '';
    $cal_params->start = $calendar_dates[0]['date']." 00:00:00";
    $cal_params->end = date("Y-m-d H:i:s", mktime(24, 0, 0, $end_m, $end_d, $end_y));
  }
  //dump($end_date);echo("end = $cal_params->end<br>");
  //dump($cal_params);
  return $cal_params;
}

function print_status_css($cal_params) {
  $obj_type = $cal_params->obj_type;
  if($status_type = object_status_objtype($obj_type)) {
    $statusObjs = find_objects($status_type) ?: array();
    foreach($statusObjs as $statusObj) {
      $status = strtolower($statusObj->name);
      $bg = $statusObj->color;
      $zind = 100+$statusObj->id;
      $output .= ".admincal .$status {background-color: #$bg; }\n";
    }
  }
  return $output;
}


// boxes with filters that can be clicked away  
function calendar_search_filters($vars, $optons = array()) {
  //$boxOptions = array("class" => "");
  $filters = array();
  
  // client
  if($vars['client_id']) {
    $data = array("client_id" => 0);
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('client', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

  // host
  if($vars['host_id']) {
    $data = array("host_id" => 0);
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('host', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

  // city
  if($vars['city_id']) {
    $data = array("city_id" => 0);
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('city', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

  // property id
  if($vars['id']) {
    $data = array("id" => 0);
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('rental', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

  // number of guests
  if($vars['max_guests']) {
    $data = array("max_guests" => 0);
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('guests', CAPITALIZE), 'data' => $data, 'link' => $link);
  }

  // property_type: not in use now
  if($vars['t']) {
    $data = array("t" => 0);
    $link = "?".array2qs(array_merge($vars, $data));    
    $filters[] = array('name' => phrase('property_type', CAPITALIZE), 'data' => $data, 'link' => $link);
  }
  
  // allow search by name ?
  
  return $filters; 
  
}



/** end old dialog */

<?php
 
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
                                     

function print_guide($cal_params, $i=0) {
  // print the guide (months, days, hours, etc)
  switch($cal_params->target_mode) {
  case "day": // dates, hours
    $output = print_calendar_dates($cal_params, $i);
    $output .= print_calendar_hours($cal_params, $i);
    break;
  case "week": // days
    $output = print_calendar_days($cal_params, $i);
    break;
  case "month": // months, days
    $output = print_calendar_months($cal_params);
    $output .= print_calendar_days($cal_params);
    break;
  default:
    dump($cal_params);
    die("Unsupported target mode $cal_params->target_mode");
    break;
  }
      
  // guides      
  return $output;
}
     
function print_calendar_month_links($cal_params) {
  $back_months = $cal_params->hide_past ? 0 : -4;
  $target_year = $cal_params->target_year;
  $target_month = $cal_params->target_month;
  $target_week = $cal_params->target_week;

  if($max_months = $cal_params->max_months) {
    $first_month = $_SESSION['today_month'];
    $first_year = $_SESSION['today_year'];
  } else {
    $first_month = $cal_params->target_month;
    $first_year = $cal_params->target_year;
    $max_months = 12;
  }
  $m=1;
  list($m, $y) = $back_months ? add_months($first_month, $first_year, $back_months) : array($first_month, $first_year); // start N months ago
  for($i = 0; $i < $max_months; $i++) { // loop through 12 months
    if($target_week || $m!=$target_month) {
       $month_name = $_SESSION['month_shortnames'][$m-1];
       $links[] = print_calendar_link($cal_params, $month_name, array('y' => $y, 'm' => $m, 'd' => 0, 'w' => 0), array('class' => 'month-link'));
    } else {
      $links[] = html_b(strtoupper($_SESSION['month_names'][$m-1]));
    } // end if
    list($m, $y) = get_next_month($m, $y);
  }
  //dump($links);
  return html_ul($links, 'month-links');
}


// prints calendar (grid + apt + guide + res)
//function print_single($cal_params, $i=0) {
//  $footers = array(); 
//  $calendar_grid = print_grid($cal_params, $i);                                              
//  return html_div($grids, '', array('id' => 'grids'));
//}

// print the grid (guide, apts, reservations, canvas)
function print_grid($cal_params, $i=0) {
  $links = html_div(print_calendar_action_links($cal_params), 'cal-links'); 
  
  $reservations = html_div(print_calendar_reservations($cal_params, $i), 'cal-entries', array('id' => "cal_entries-$i"));
  $guide        = html_div(print_guide($cal_params, $i), 'cell_w_font cal-guide'); // the guide on top (days, weeks, months, hours)
  $properties   = html_div(print_calendar_properties($cal_params, $i), 'cal-apts', array());
  $canvas       = html_div($reservations, 'cal-canvas', array('width' => $cal_params->canvas_w, 'height' => $cal_params->canvas_h, 'id' => "cal_canvas-$i", 'data-index' => $i, 'data-size' => json_encode(array('top' => $cal_params->num_apts, 'left' => $cal_params->num_cols))));  
  $footer       = html_div(print_calendar_month_links($cal_params).$links, 'cal-footers title_margin canvas_w center'); // months on bottom (or days for day view?)  
  $grid         = html_div($guide.$properties.$canvas, 'cal-container', array('id' => "calendar_grid-$i", 'width' => $cal_params->canvas_w + $cal_params->title_w));
  $output       = html_div($grid.$footer, 'admincal', array('id' => "admincal-$i"));
  return $output;
}

function print_calendar_exceptions($cal_params) {
  $exceptions = $cal_params->exceptions ?: array();
  foreach($exceptions as $exObj) {
    $output .= print_pricing_marker($cal_params, $exObj).PHP_EOL;
  }
  return $output;  
}

function print_calendar_reservations($cal_params) {
  global $u;
  global $u_level;
  global $u_hostid;
  
  $res_ids = $cal_params->res_list ? explode(',', $cal_params->res_list) : array();
  $obj_type = $cal_params->obj_type;
  if(!$obj_type) die("no object type".dump($cal_params, true));
  foreach($res_ids as $res_id) {
    $resObj = get_object($obj_type, $res_id);  
    if(!$resObj) {
      $error = "Did not find $obj_type $res_id";
      $errors[] = $errors;
      echo($error);
      continue;
    }
    $resObj = add_calobj_helperfields($cal_params, $resObj);
    $ap = $resObj->apt_id;

    if($u && $resObj->hidden && $resObj->host_id != $u_hostid && $u_level < USER_LEVEL_SYSTEM) continue; // hide hidden reservations on private calendar for less than system or this is the host
    $output  .= "      ".print_res_marker($cal_params, $resObj).PHP_EOL;
  }
  return $output;
}
                
function print_calendar_properties($cal_params, $i=0) {
  $siteObj = $_SESSION['siteObj'];
  $apt_ids = $cal_params->apt_list ? explode(',', $cal_params->apt_list) : array();
  $property_type = $cal_params->property_type;
  if(!$property_type) die("print_calendar_properties: no property type: ".caller_stack());
  $auth_edit = false;
  
  $items = array();
  $item_options = array();
  $index = 0;
  foreach ($apt_ids as $index => $apt_id) {
    if($apt_id) {
      $apt_link = property_link($siteObj, $apt_id);
      $apt_name = get_object($property_type, $apt_id, 'name');
    } else {
      $apt_name = " ? ";
    }
    $aptObj->id = $apt_id;
    $aptObj->name = $apt_name;

    if($cal_params->cal_show['rentals']) {
      $p_link = $apt_link ? html_link($apt_link, $apt_name) : $apt_name;      
      if($auth_edit) { // show menu
        $title = html_link('#', $apt_name.' '.html_span('&#9660;', 'caret tinylinks'), array('class' => 'toggle-div dropdown relpos', 'js' => 'data-apt_id="'.$apt_id.'" data-toggle="apt-dropdown-menu"'));
      } else {         
        $title = $apt_id ? $p_link : $tips .= html_tooltip("Parking", 'help');
      }
    } else {
      $title = "";
    }
    $items[] = html_div($title, 'calendar-property-title');   
    //$items[] = $title;   
    if(!$i) $item_options[] = array('id' => "cal_apt-$i-$index", 'data-apt_id' => $apt_id, 'data-index' => $index, 'data-apt' => json_encode($aptObj)); // we only want this once
    $classes[] = $index%2 ? 'cal-apt odd' : 'cal-apt even';
    $index++;
    //$items[] = html_div($title, 'apt-title').html_div('', 'apt-box');
  }
  //dump($classes);
  $output = html_ul($items, '', array('id' => "cal_apts-$i", 'item_classes' => $classes, 'item_options' => $item_options));
  
  if($auth_edit) {
    // menu
    $menu_items = array();
    $menu_items[] = html_link('?edit', phrase('edit', CAPITALIZE));
    $menu = html_ul($menu_items);
  
    $output .= html_div($menu, 'user-menu ztop', "id='apt-dropdown-menu'");
  }

  return $output;     
}


// prints one (or more) calendars or lists
function print_calendar($cal_params, $options = NULL) {
  switch($cal_params->mode) {
  case SHOW_LIST: // list
    $calendar_list = print_list($cal_params);
    $calendar = html_div($calendar_list, '', array('id' => 'cal_list'));
    break;

  default: // calendar
    $vcount = $cal_params->stack == 'v' ? $cal_params->period : 1;
    if($vcount > 1) {
      list($cal_params, $cals) =  print_stack($cal_params, $vcount);
    } else {
      $cals = print_grid($cal_params);
    }
    $calendar = html_div($cals, '', "id='calendar'");
    break;
  }
  
  return $calendar;
}
  
  
// prints one or more calendars (grid + apt + guide + res)
function print_stack($cal_params, $vcount=1) {
  if($vcount == 1) return print_grid($cal_params);
  
  $max_w = 0;
  $cals = array();

  // make array of filters
  $params_stack = array();
  $cal_params_clone = clone $cal_params;
  $cal_params_clone->period = 1;
  
  // prepare
  for($i=0;$i<$vcount;$i++) {      
     
    $cal_params_clone = calendar_init($cal_params_clone);

    $params_stack[] = clone $cal_params_clone;  // memory intensive, but...

    //echo("i=$i m=$cal_params_clone->target_month x=$cal_params_clone->grid_w w=$cal_params_clone->canvas_w<br>");

    // get next day/week/month
    
    $td = $target['day']   = $cal_params_clone->target_day;
    $tw = $target['week']  = $cal_params_clone->target_week;
    $tm = $target['month'] = $cal_params_clone->target_month;
    $ty = $target['year']  = $cal_params_clone->target_year;
    //dump($target);
    $next_day = $next_week = $next_month = $next_year = 0;
    
    if($td) { // show days    
      list($next_day, $next_month, $next_year) = sql2date(get_next_day(dates2sql($target))); 
    } else if($tw) { // show weeks
      list($next_week, $next_year) = get_next_week($tw, $ty);
    } else { // show months
      list($next_month, $next_year) = get_next_month($tm, $ty);
    }
                           
    $cal_params_clone->target_day   = $next_day;
    $cal_params_clone->target_week  = $next_week;
    $cal_params_clone->target_month = $next_month;
    $cal_params_clone->target_year  = $next_year;              

    $max_w = max($max_w, $cal_params_clone->canvas_w);
  }                              
  //dump($params_stack);die();

  // draw
  $cals = array();
  $i = 0;
  foreach($params_stack as $param_copy) {
    $param_copy->canvas_max_w = $max_w;
    // echo("print: m=$param_copy->target_month x=$param_copy->grid_w w=$param_copy->canvas_w<br>");
    $cals[]= print_grid($param_copy, $i++);
  }
  $cal_params->canvas_max_w = $max_w; 
  $cal_params = calendar_init($cal_params); // restore
  return array($cal_params, implode(PHP_EOL, $cals));
}

// prints reservations or events as a table instead of graphically
function print_list($cal_params) {
  global $u;
  $u_level = $cal_params->user_level; 
  $rows = array();
  $obj_type = $cal_params->obj_type;
  $res_ids  = $cal_params->res_list ? explode(',', $cal_params->res_list) : array();

  $cal_link = get_page_link(PAGE_CALENDAR);
  $cal_name = phrase('calendar', CAPITALIZE);

  switch($obj_type) {
    case 'reservation':
      $header_row = array('', 'Checkin', 'Checkout', 'Status', 'Apartment', 'Guest', 'Stay', 'Fees','Total', 'Commission', 'NB!', 'Notes');
      foreach($res_ids as $id) {
        $obj = get_object($obj_type, $id);
        if(!$obj) continue;
        $actions = recordset_buttons($obj_type, $obj, '', '', $cal_name, $cal_link);
        $name = $obj->auth_view ? get_name_string($obj) : '';
        $row = array($actions, sql2human($obj->checkin, array('show_weekday' => true, 'show_time' => $show_time)), sql2human($obj->checkout, array('show_weekday' => true, 'show_time' => $show_time)), get_object('res_status', $obj->status, 'name'), $obj->apartment_name, $name);
        if($obj->auth_edit) { // accounting info if auth
          $row[] = $obj->total;
          $row[] = $obj->fees;
          $row[] = $obj->grand_total;
          $row[] = $obj->commission;
          $row[] = $obj->alert;
          $row[] = $obj->notes;
        } else {
          $row = array_pad($row, count($row) + 4, '');
        }
        $rows[] = $row;
      }
      break;

    case 'invoice':
      $header_row = array('', 'Start date', 'End date', 'Status', 'Apartment', 'Guest', 'Total', 'Paid', 'Balance', 'Payments', 'Fees', 'Net', 'Withdrawn', 'Outstanding', 'Notes', '');
      $currencies = array();
      foreach($objs as $obj) {
        if(!$cur) $cur = get_currency_symbol($obj->currency, true);
        $actions = recordset_buttons($obj_type, $obj, '', '', $cal_name, $cal_link);
        $paymentObjs = find_objects('payment', array('inv_id' => $obj->id, 'payment_status' => array(PAYMENT_STATUS_COMPLETED, PAYMENT_STATUS_REFUNDED, PAYMENT_STATUS_PARTIALLY_REFUNDED)));
        $pcount = count($paymentObjs);
        $payment_list = print_payments($paymentObjs, 0);
        $tip = $pcount ? tooltip('ptip_'.$obj->id, $payment_list) : '';
        // was tooltip()
        $p_total = $p_fees = $p_net = $p_sent = $p_outstanding = $sum_pcurcount = array();
        foreach($paymentObjs as $paymentObj) {
          list($p_currency, $p_amount) = payment_currency_amount($paymentObj);
          $currencies[$p_currency] = 1;
          $pay_cur = get_currency_symbol($p_currency, true);
          list($w_currency, $gross) = payment_net_outstanding($paymentObj);
          if($gross && $w_currency != $p_currency) die("payment currency ($p_currency) != withdrawal currency ($w_currency): payment $paymentObj->id. Contact the administrator.");
          // echo("Net outstanding for $paymentObj->id = $gross<br>");
          $p_total[$p_currency] += $p_amount; // always in invoice currency
          $p_fees[$p_currency]  += $paymentObj->payment_fee;
          $p_net[$p_currency]   += $paymentObj->payment_net;
          $p_sent[$p_currency]  += $paymentObj->withdrawal_amount;
          $p_outstanding[$p_currency] += $gross;
          $pcurcount[$p_currency] ++;
          if($gross > 0) $p_ids[$p_currency][] = $paymentObj->id;
        }
        $resObj = get_object('reservation', $obj->res_id);
        $guest_name = get_name_string($resObj);
        $rows[] = array($actions, sql2human($obj->start_date), sql2human($obj->end_date), get_object('inv_status', $obj->status, 'name'), get_object('property', $obj->apt_id, 'name'), $guest_name,
                        $obj->total, $obj->paid, $obj->balance,
                        print_mulitple_currencies($p_total).($pcount ? " ($pcount)" : '').$tip, print_mulitple_currencies($p_fees), print_mulitple_currencies($p_net), print_mulitple_currencies($p_sent), print_mulitple_currencies($p_outstanding),
                        $obj->notes, '');

        $sum_total   += $obj->total;
        $sum_paid    += $obj->paid;
        $sum_balance += $obj->balance;
        $sum_pcount  += $pcount;
        // dump($currencies);
        foreach(array_keys($currencies) as $thiscur) {
          $sum_payments[$thiscur]     += $p_total[$thiscur];
          $sum_fees[$thiscur]         += $p_fees[$thiscur];
          $sum_net[$thiscur]          += $p_net[$thiscur];
          $sum_withdrawn[$thiscur]    += $p_sent[$thiscur];
          $sum_outstanding[$thiscur]  += $p_outstanding[$thiscur];
          $sum_pcurcount[$thiscur]    += $pcurcount[$thiscur];
        }
      }

      // withdraw links for each currency
      foreach(array_keys($currencies) as $thiscur) {
        $sum_pcurcount[$thiscur] = " ($sum_pcurcount[$thiscur])";
        if(count($p_ids[$thiscur])) $w_links[$thiscur] = ' '.html_image_link("/admin/withdraw.php?currency=$thiscur&pids=".implode(',', $p_ids[$thiscur]), '/images/icons/withdraw_tiny.png',
                                       phrase('withdraw', CAPITALIZE).' '.formatFloat($sum_outstanding[$thiscur], FORMAT_IF_NEEDED, get_currency_symbol($cur, true)));
      }

      $rows[] = array(html_b(count($objs).' invoices'), '', '', '', '', '',
          html_b(formatFloat($sum_total, FORMAT_NEVER, $cur)), html_b(formatFloat($sum_paid, FORMAT_NEVER, $cur)), html_b(formatFloat($sum_balance, FORMAT_NEVER, $cur)),
          print_mulitple_currencies($sum_payments, true, FORMAT_NEVER, $sum_pcurcount).html_break().html_b("$sum_pcount payments"), print_mulitple_currencies($sum_fees, true, FORMAT_NEVER), print_mulitple_currencies($sum_net, true, FORMAT_NEVER), print_mulitple_currencies($sum_withdrawn, true, FORMAT_NEVER), print_mulitple_currencies($sum_outstanding, true, FORMAT_NEVER, $w_links),
          '', '');
      break;
    default:
      $rows = event_list($objs, $cal_name, $cal_link, true);
      break;
  }

  //if(!isset($_GET['print'])) $print_link = html_image_link("?print", '/images/icons/print_tiny.png', phrase('print', CAPITALIZE), array('class' => 'pad_left non-html'));
  if(!isset($_GET['print'])) $print_link = html_link($url, "".html_i("", "fa fa-print".$big_class), array("class" => "text-danger", "js" => $js, "title" => phrase('delete', CAPITALIZE)));
  
  $caption = phrase($cal_params->show, CAPITALIZE).' '.sql2human($cal_params->start).' - '.sql2human($cal_params->end). ' '.$print_link;
  if($header_row) array_unshift($rows, $header_row);
  return html_table($rows, array('title' => $caption, 'class' => 'web20'));
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

                                                                       
// return description of this day/week/month + links 
function print_calendar_date($cal_params, $options=NULL) {
  //dump($cal_params);
  $day   = $d = $cal_params->target_day;
  $week  = $w = $cal_params->target_week;
  $month = $m = $cal_params->target_month;
  $year  = $y = $cal_params->target_year;
  $month_name = $_SESSION['month_names'][$month-1];
  //return "day=$day week=$week month=$month";

  if($day > 1) { // prev/next day
    $wd = $cal_params->target_weekday_name;
    list($w, $y) = get_week_year($day, $month, $year);

    $link = print_calendar_link($cal_params, $month_name, array('y' => $y, 'm' => $m, 'd' => 0, 'w' => 0), array('class' => 'glink cal_link cal_nav'));
    $output = "$wd $day $link $y";
    if(!$cal_params->hide_weeks || $cal_params->allow_week_view) {
      $week_link = print_calendar_link($cal_params, phrase('week', CAPITALIZE).' '.$w, array('y' => $y, 'm' => $m, 'd' => 0, 'w' => $w), array('class' => 'glink cal_link cal_nav'));
      $output .=  html_break().$week_link;
    }    
  } elseif($week > 1) { // prev/next week
    $link = print_calendar_link($cal_params, $month_name." $y", array('y' => $y, 'm' => $m, 'd' => 0, 'w' => 0), array('class' => 'glink cal_link cal_nav'));
    $output = phrase('week', CAPITALIZE)." $w $link";

  } else {  // prev/next month
    $output = $month_name." $year";
  }
  return $output;
}

// returns array (prev, next) where each is assoc array. with day/week/month/year used by buttons
function calendar_prev_next($cal_params, $options=NULL) { 
  $day   = $cal_params->target_day;
  $week  = $cal_params->target_week;
  $month = $cal_params->target_month;
  $year  = $cal_params->target_year;
  $prev2 = $next2 = array();  
  if($day > 1) { // prev/next day
    list($prev_day, $prev_month, $prev_year) = add_days($day, $month, $year, -1 * $cal_params->period);
    list($next_day, $next_month, $next_year) = add_days($day, $month, $year, 1 * $cal_params->period);
    $prev_title = dates2human(dates2sql(array('d' => $prev_day, 'm' => $prev_month, 'y' => $prev_year)));
    $next_title = dates2human(dates2sql(array('d' => $next_day, 'm' => $next_month, 'y' => $next_year)));
                      
    $prev1 = $prev_prev = array('d' => $prev_day, 'w' => 0, 'm' => $prev_month, 'y' => $prev_year, 'title' => $prev_title);
    $next1 = $next_next = array('d' => $next_day, 'w' => 0, 'm' => $next_month, 'y' => $next_year, 'title' => $next_title);
         
  } elseif($week > 1) { // prev/next week
    list($prev_week, $prev_year) = get_prev_week($week, $year);
    list($next_week, $next_year) = get_next_week($week, $year);
    $prev_title = phrase('week', CAPITALIZE)." $prev_week $prev_year";
    $next_title = phrase('week', CAPITALIZE)." $next_week $next_year";

    $prev2 = array('d' => 0, 'w' => $week, 'm' => 0, 'y' => $year-1, 'title' => phrase('week', CAPITALIZE)." $week ".($year-1));
    $prev1 = array('d' => 0, 'w' => $prev_week, 'm' => 0, 'y' => $prev_year, 'title' => $prev_title);
    $next1 = array('d' => 0, 'w' => $next_week, 'm' => 0, 'y' => $next_year, 'title' => $next_title);
    $next2 = array('d' => 0, 'w' => $week, 'm' => 0, 'y' => $year+1, 'title' => phrase('week', CAPITALIZE)." $week ".($year-1));

  } else { // prev/next month
    list($prev_month, $prev_year) = get_prev_month($month, $year);    
    list($next_month, $next_year) = get_next_month($month, $year);

    $prev2 = array('d' => 0, 'w' => 0, 'm' => $month, 'y' => $year-1, 'title' => month_name($month).' '.($year - 1));
    $prev1 = array('d' => 0, 'w' => 0, 'm' => $prev_month, 'y' => $prev_year, 'title' => month_name($prev_month).' '.$prev_year);
    $next1 = array('d' => 0, 'w' => 0, 'm' => $next_month, 'y' => $next_year, 'title' => month_name($next_month).' '.$next_year);
    $next2 = array('d' => 0, 'w' => 0, 'm' => $month, 'y' => $year+1, 'title' => month_name($month).' '.($year + 1));
  }
  
  return array($prev2, $prev1, $next1, $next2);
}                   

function calendar_gmenu($cal_params) {             
  $items = array(); // gmenu items

  // prev / next (day/week/month/year)
  if(1) {
    list($prev2, $prev, $next, $next2) = calendar_prev_next($cal_params);
    $now = print_calendar_date($cal_params);
    // dump($prev);dump($next); echo("now=$now");    
    if($prev2) $items[] = print_calendar_link($cal_params, '&nbsp;', $prev2, array('class' => 'gbutton previous2', 'title' => $prev2['title']));$item_classes[] = '';
    $items[] = print_calendar_link($cal_params, '&nbsp;', $prev, array('class' => 'gbutton previous', 'title' => $prev['title']));$item_classes[] = '';
    $items[] = html_span($now, '', array('class' => 'center gtext margin_right')); $item_classes[] = '';
    $items[] = print_calendar_link($cal_params, '&nbsp;', $next, array('class' => 'gbutton next', 'title' => $next['title']));$item_classes[] = '';
    if($next2) $items[] = print_calendar_link($cal_params, '&nbsp;', $next2, array('class' => 'gbutton next2', 'title' => $next2['title']));$item_classes[] = '';
  }

  $rrOptions = array('url' => "?reset");
  $items[] = print_calendar_link($cal_params, '&nbsp;', array('refresh' => 1), array('id' => 'cal_reload', 'class' => 'gbutton rotate_right'));
  //$items[] = html_link('#', '&nbsp;', array('class' => 'gbutton left rotate_right', 'title' => phrase('refresh', CAPITALIZE), 'data-options' => json_encode($rrOptions))); 
  $item_classes[] = '';

  // new res  
  $items[] = html_link('#cal_dialog', ' + ', array('title' => phrase('new', CAPITALIZE).' '.phrase($cal_params->obj_type), 'class' => 'left gbutton new-res big-text', 'data-toggle' => 'modal')).' &nbsp';
  //$items[] = html_link('#', ' + ', array('title' => phrase('new', CAPITALIZE).' '.phrase($cal_params->obj_type), 'class' => 'left gbutton ajax-popup-link big-text')).' &nbsp';
  $item_classes[] = '';   
         
  if(!$cal_params->simple_menu) {
    $menus = calendar_menu_items($cal_params); 
    //dump($menus);
    $siteObj = $_SESSION['siteObj'];
    foreach($menus as $menu) {
      $title = phrase($menu, CAPITALIZE);
      $sub_menu = html_link('#', $title.html_span('', 'garrow'), array('title' => $title, 'class' => 'left gdropdown_medium gbutton'));
      
      $sub_menu .= function_exists($menu."_menu") ? call_user_func($menu."_menu" ,$cal_params) : 'no sub';
    
      $items[] = $sub_menu;
      $item_classes[] = $hide_class;
    }
        
    // search  
    //$search_box = form_input('cal_search', 'text', '', array('size' => 8, 'class' => 'left'));
    /** todo: move to template */
    $search_box .= '<input class="left" name="cal_search" type="text" id="cal_search" style="width: 80px;height:30px !important;color:#000 !important"> ';
    $search_box .= '<button type="submit" class="btn btn-success btn-sm"><i class="fa fa-search" style=""></i></button>';
            
    //form_input('submit', 'submit', "<i class='fa fa-search'></i>", array("class" => "btn btn-positive"));
    $items[] = html_form($search_box, 'cal-search-form');
    $item_classes[] = '';
  }  
  $gmenu_options = array('item_classes' => $item_classes, 'data' => $data);
 
 
  return html_div(site_gmenu($siteObj, $items, $gmenu_options).$cal_search, 'menu_container');
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

// prints list of last 25 changes to the reservations shown
function print_changes($changes) {
  global $user_level_array;
  $rows = array();
  $rows_js = array();

  $rows[] = array(phrase('timestamp', CAPITALIZE), phrase('ID', UPPERCASE), phrase('property', CAPITALIZE), phrase('by', CAPITALIZE), phrase('role', CAPITALIZE), phrase('change', CAPITALIZE));
  //$rows_js[] = '';
  foreach($changes as $change) {
    $id = $change->parent_id;
    $el = $change->obj_type.'-'.$id;
    $user = get_name_string(get_object('user', $change->user_id));
    $rows_js[] = "data-target='$el' data-class='selected' class='hilite_target'";
    $rows[] = array($change->timestamp, $id, get_object('reservation', $id, 'apartment_name'), $user, $user_level_array[$change->user_level], $change->action);
  }
  $change_str = html_table($rows, array('title' => $caption, 'class' => 'web20', 'header' => 1, 'rows_js' => $rows_js));
  return $change_str;
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

// statistics
function print_stats($cal_params, $stats=NULL) {
  $w2     = $cal_params->w2;
  $top    = 0;
  $left   = 0;
  $apt_ids = $cal_params->apt_list ? explode(',', $cal_params->apt_list) : array();

  $currency = $stats['total']['currency'];
  $cur = get_currency_symbol($currency, true);

  // stats header
  switch($cal_params->property_type) {
  case 'boat':
    $headers = array('Name', 'Boat', 'Pack.', 'Cat.', 'BP', 'BPC', 'Prod.', 'Total', 'ResT', 'ResGT', 'C.Tax', 'Fees', 'Comm.', 'Net');                 
    $stats_fields = array('boat', 'package', 'catering', 'bp', 'bpc', 'product', 'total', 'res_total', 'res_gtotal', 'city_tax', 'fees', 'commission', 'net');  
    break;
  default:
    $headers = array('Property', 'Occ%', 'Avg.', 'Total', 'Fees', 'Comm.', 'Net', 'Paid', 'Due', 'Length');
    $stats_fields = array('occupancy', 'avg_rate', 'revenue', 'fees', 'commission', 'net', 'paid', 'due', 'length');
    break;
  }

  $rows = array();
  $rows[] = $headers;
  
  // stats per apartment
  foreach ($apt_ids as $apt_id) {
    $apt_stats = $stats[$apt_id];
    $apt_name = get_object('property', $apt_id, 'name');
    $row = array(html_b($apt_name));
    foreach($stats_fields as $stats_field) $row[] = $apt_stats[$stats_field];
    $rows[] = $row;
  }
    
  // stats totals
  if($cal_params->user_level == USER_LEVEL_SYSTEM) $stats_fields[] = 'software_fee';
  $stats = $stats['total'];
  $row = array('Total');
  foreach($stats_fields as $stats_field) $row[] = $stats[$stats_field];
  $rows[] = $row;
  return html_table($rows, array('title' => 'Stats', 'class' => 'web20'));
}


function get_boat_stats($cal_params) {
  // 'boat', 'package', 'catering', 'product', 'total', 'res_total', 'res_gtotal', 'city_tax', 'fees', 'commission', 'net');
  global $u_level;
  
  $apt_ids = $cal_params->apt_list ? explode(',', $cal_params->apt_list) : array();
  
  $start_date = $cal_params->start_date['date'];
  $software_rate = strip_time($start_date) < '2014-02-01' ? 0.05 : 0.01;

  //dump($apt_ids);
  foreach($apt_ids as $apt_id) {
    if(!$apt_id) continue;
    $start = $start_date = $cal_params->start_date['date'];
    $end = sql_add_days($cal_params->end_date['date'], 1);
    
    $qs = "select distinct(res_id) from reservation where apt_id=$apt_id and checkout >='$start' and checkin < '$end' and status in (30, 40)";
    //echo($qs.html_break());
    $res_count = $inv_count = $li_count = 0;
    if($res_ids = db_getCol($qs)) {
      //dump($res_ids);die();
      $res_id_list = implode(',', $res_ids);
      $res_count = count($res_ids);

      $rest = db_getOne("SELECT sum(total) from reservation WHERE res_id in ($res_id_list)");
      //dump($rest);die();
      $regt = db_getOne("SELECT sum(grand_total) from reservation WHERE res_id in ($res_id_list)");
      $stats[$apt_id]['res_total']  = round($rest);
      $stats[$apt_id]['res_gtotal'] = round($regt);
      $total_rest += $rest;
      $total_regt += $regt;            

      if($debug) echo("res_ids for $apt_id = $res_id_list<br>");
      if($inv_ids = db_getCol("select distinct(id) from invoice where res_id in ($res_id_list) AND status >= -1")) {
      //if($inv_ids = db_getCol("select distinct(id) from invoice where res_id in ($res_id_list)")) {
        $inv_id_list = implode(',', $inv_ids);
        $inv_count = count($inv_ids);
        if($debug) echo("inv_ids for $apt_id = $inv_id_list<br>");
        if($li_ids = db_getCol("select distinct(id) from inv_lineitem where inv_id in ($inv_id_list)")) {
          $li_id_list = implode(',', $li_ids);
          $li_count = count($li_ids);

          $rent_types = implode(',', lineitem_rent_types());
          $fee_types = implode(',', lineitem_fee_types());

          $boat = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list) and type in ($rent_types)");
          $pack = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list) and type = ".LI_TYPE_PACKAGE);
          $catr = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list) and type = ".LI_TYPE_CATERING);
          $prod = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list) and type = ".LI_TYPE_PRODUCT);

          $tota = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list)");
          $ctax = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list) and type = ".LI_TYPE_CITY_TAX);
          $fees = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list) and type in ($fee_types)");
          $comm = db_getOne("SELECT sum(total) from inv_lineitem WHERE id in ($li_id_list) and type = ".LI_TYPE_COMMISSION);
          
          $bopa = $boat + $pack;
          $bopc = $boat + $pack + $catr;
          
          $stats[$apt_id]['boat']       = round($boat);
          $stats[$apt_id]['package']    = round($pack);
          $stats[$apt_id]['catering']   = round($catr);
          $stats[$apt_id]['bp']         = round($bopa);
          $stats[$apt_id]['bpc']        = round($bopc);
          $stats[$apt_id]['product']    = round($prod);
          $stats[$apt_id]['total']      = round($tota);
          $stats[$apt_id]['city_tax']   = round($ctax);
          $stats[$apt_id]['fees']       = round($fees);
          $stats[$apt_id]['commission'] = round($comm);
          
        }                
      } else {
        echo("warning: couldn't find any invoice for reservation $res_id<br>");
      }
   
        
      if($res_count != $inv_count) {
        echo("warning: invoice count doesn't match reservation count for $apt_id<br>");
      } 
        
      $total_boat += $boat;
      $total_pack += $pack;
      $total_catr += $catr;
      $total_bopa += $bopa;
      $total_bopc += $bopc;
      $total_prod += $prod;
      $total_tota += $tota;
      $total_ctax += $ctax;
      $total_fees += $fees;
      $total_comm += $comm;
    }
  }


  $stats['total']['boat']       = round($total_boat);
  $stats['total']['package']    = round($total_pack);
  $stats['total']['catering']   = round($total_catr);
  $stats['total']['bp']         = round($total_bopa);
  $stats['total']['bpc']        = round($total_bopc);
  $stats['total']['product']    = round($total_prod);
  $stats['total']['total']      = round($total_tota);
  $stats['total']['res_total']  = round($total_rest);
  $stats['total']['res_gtotal'] = round($total_regt);
  $stats['total']['city_tax']   = round($total_ctax);
  $stats['total']['fees']       = round($total_fees);
  $stats['total']['commission'] = round($total_comm);
  
  $stats['total']['software_fee'] = $swfee = round($total_bopa * $software_rate);      
  $stats['total']['city_id']    = $cal_params->city_id;
  //$stats['total']['currency'] = get_city_currency($resObj->city_id);
  $stats['total']['start_date'] = $cal_params->start_date['date'];
  $stats['total']['end_date']   = $cal_params->end_date['date'];
  
  return $stats;
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

function print_res_block($cal_params, $resObj) {
  $in = strip_time($resObj->checkin);
  $out = strip_time($resObj->checkout);  
  list($h, $w, $top, $left) = res_marker_pos($cal_params, $in, $out, $resObj->apt_id);
  $h = $cal_params->cell_h; // don't fill canvas if apt_id is missing 

  $cur = $cal_params->cur;
  $text = $resObj->last_name ?: "N/A";
  $class = 'blockBar';
  $style = "width:".$w."px;height:".$h."px;top:".$top."px;left:".$left."px;";  

  // begin output
  $output = html_div(html_span($text, 'price-text'), $class, array('style' => $style));
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

// prints the colored bar on the calendar
function print_res_marker($cal_params, $resObj) {
  global $u;
  if(!$resObj->in) $resObj = add_calobj_helperfields($cal_params, $resObj);

  if($resObj->status == 35 && $cal_params->user_level < USER_LEVEL_STAFF) return print_res_block($cal_params, $resObj);
  
  $u_level = $cal_params->user_level; 
  $target_day     = $cal_params->target_day ?: 0;
  $target_date    = $cal_params->target_date;

  $cell_h    = $cal_params->cell_h;
  $cell_w    = $cal_params->cell_w;
  $grid_w    = $cal_params->grid_w;
  $canvas_w  = $cal_params->canvas_w;
  $start     = $cal_params->start;
  $end       = $cal_params->end;
  $cols      = $cal_params->num_cols;

  $days      = $resObj->num_days;
  $hours     = $resObj->num_hours;
  $guests    = $resObj->num_guests;
  $stsid     = $resObj->status;
  $bgcolor   = '#'.$resObj->status_color;
  $sts       = $resObj->status_name;

  $ap        = $resObj->apt_id;

  $obj_type  = $resObj->obj_type;
  $auth_view = $resObj->auth_view;
  $auth_edit = $resObj->auth_edit;

  $id       = $resObj->id;
  $in       = $resObj->in;
  $out      = $resObj->out;
  $res_text = $resObj->calendar_title;

  // echo("ot of $resObj->id = $resObj->obj_type in=$in out=$out st=$stsid ($sts) auth_view=$resObj->auth_view<br>");
  if(!$res_text) $res_text = $sts;

  global $pTypeAr;
  $typeObj = $pTypeAr[$resObj->property_type];
     
  // not day mode - default in/out times if not set (todo: default on resObj instead ?)
  if(!$typeObj->hourly && ($resObj->obj_type == 'invoice' || $resObj->obj_type == 'res_exception' || $resObj->num_days)) {
    //if(strip_date($in) == '00:00:00')  $in = strip_time($in)." 16:00:00";
    //if(strip_date($out) == '00:00:00') $out = strip_time($out)." 10:00:00";
    $in_date = strip_time($in);
    $out_date = strip_time($out);
    $in = strip_time($in)." 16:00:00";
    $out = strip_time($out)." 04:00:00";
  }

  //if($resObj->status == 35) { // blocked
  //  $in = strip_time($in)." 00:00:00";
  //  $out = strip_time($out)." 00:00:00";
  //}

  //dump($resObj);
  
  $start_ts     = strtotime($start);
  $end_ts       = strtotime($end);
  $total_diff   = $end_ts - $start_ts;
  $sec_per_pix  = $total_diff / $canvas_w;
  $in_ts        = strtotime($in) - $start_ts;
  $out_ts       = strtotime($out) - $start_ts;

  $leftOff = $rightOff = false;                              

  $left = round($in_ts/$sec_per_pix) + 1;
  $right = round($out_ts/$sec_per_pix);
          
  
  
  if(!$typeObj->hourly && ($resObj->obj_type == 'invoice' || $resObj->num_days)) {
    //$left += $cal_params->bar_x;
    //$right += $cal_params->bar_x; 
    //if(strip_date($in) == '00:00:00')  $in = strip_time($in)." 16:00:00";
    //if(strip_date($out) == '00:00:00') $out = strip_time($out)." 10:00:00";
  }
  
  if($left < 0) $left=0;           
  if($right > $canvas_w) $right=$canvas_w;
  $w = $right-$left;
  //print_log("id=$id in=$in out=$out left=$left right=$right w=$w", "calendar", LOG_LEVEL_DEBUG);
  $top = $resObj->apt_index * $cell_h; // top of each line



  list($h, $w, $top, $left) = res_marker_pos($cal_params, $in, $out, $resObj->apt_id);
  $cell_count = round($w/$cell_w) + 1;
  
  if($resObj->base_class == 'invoice' && $cal_params->base_class == 'reservation') { // we're showing an invoice on top of res
    $top_b = $top;       // where to draw the bar
    $bar_h = round($cal_params->bar_h / 4);
    $resObj->calendar_title = '';
  } else {
    $top_b = $top + $cal_params->bar_y;       // where to draw the bar
    $bar_h = $cal_params->bar_h;
  }

  //$auth = $auth_view || $auth_edit || $u_level == USER_LEVEL_GUEST;
  $auth = $auth_view || $auth_edit;
  if($auth) {
    $max_l = round($w/$cell_w) * 4;
    //$res_text = "w=$w m=$max_l ".substr($res_text, 0, $max_l);
    $res_text = substr($res_text, 0, $max_l);
    if($res_text && $resObj->status != STATUS_BLOCKED && $resObj->num_guests > 0) $res_text .= " ($resObj->num_guests)";

    $border_w = 1;
    $border= "
    border-left-style: solid;
    border-left-color: black;
    border-left-width: ".$border_w."px;
    border-right-style: solid;
    border-right-color: black;
    border-right-width: ".$border_w."px;
    ";
    $border="";
  } else {
    $res_text     = "";
    $bgcolor      = "#800";
    //$onclick      = "";
    $border       = "";
  }


  $org_cell_w = $cal_params->org_cell_w;


  $resize = $resObj->calendar_resize;

  $classes = array('resBar');
  $classes[]= "$obj_type ".strtolower($resObj->status_name);
  $marker_id = $resObj->base_class.'-'.$id;
	if($resObj->base_class && $resObj->base_class != $obj_type) $classes[] = $resObj->base_class; // adds "event" in addition to "cleaning"
	if($auth_edit) $classes[]= 'auth_edit';
  else if($auth_view) $classes[]= 'auth_view';

  $expires = $resObj->expires;
  if($auth_edit && $expires && $expires > now()) {
    $tips .= html_tooltip("Expires: ".date("Y-m-d H:i:s", strtotime($expires)), "time", array('class' => 'cal-alert tooltip-jq'));    
  }
  
  if($mismatch = $auth_edit ? reservation_invoice_mismatch($resObj) : false) {
    $classes[] = "mismatch";
    $cur = get_currency_symbol($resObj->currency, true);
    //$tips .= tooltip('tip-inv-'.$id , "Invoice mismatch: $mismatch", array('data-tip' => 'foo', 'class' => 'cal-alert tooltip-jq', 'style' => 'error'));
    $tips .= html_tooltip("Invoice mismatch: $cur $mismatch", "error", array('class' => 'cal-alert tooltip-jq'));

  }
 
  if($auth_edit) {
    // $classes[]= 'auth_edit';

      // $has_children = $resOb
    if($block_id = $resObj->block_id) {
      $obj_type = $resObj->base_class;
      $key = get_object_key($obj_type);
      $children_ids = ($res_list = get_object('block', $block_id, 'res_list')) ? explode(',', $res_list) : array();
      //$res_list = $block_
      if(!$resObj->block_parent) { // parent
        foreach($children_ids as $cid) $cselar[] = "#$obj_type-$cid";
        $csel = implode(',', $cselar);
        $tip_options = array();
        $tip_options['mouseenter'] = 'function() {$("'.$csel.'").addClass("selected")}';
        $tip_options['mouseleave'] = 'function() {$("'.$csel.'").removeClass("selected")}';
        $tip_id = $marker_id.'-parent1';
        //$tip_js = "id='$tip_id' "; 
        $tip_title = "Block: Parent";
        $tip_js .= 'title="'.$tip_title.'" onMouseOver="$(\''.$csel.'\').addClass(\'selected\')" onMouseOut="$(\''.$csel.'\').removeClass(\'selected\')"';
        $tip_image = '/images/icons/hier_parent_filled.png';
        $tip_class = 'tooltip';
        $tips .= html_image($tip_image, $tip_title, array('js' => $tip_js, 'class' => $tip_class, 'height' => round(0.9 * $bar_h), 'width' => round(0.9 * $bar_h)))."&nbsp;";
        $classes[] = 'block'; // allow drag right/left + up/down (change dates + apartment)
      } else { // child
        $parent_res_marker_id = $resObj->base_class.'-'.$resObj->block_parent;
        $tip_options = array();
        $tip_options['mouseenter'] = 'function() {$("#'.$parent_res_marker_id.'").addClass("selected")}';
        $tip_options['mouseleave'] = 'function() {$("#'.$parent_res_marker_id.'").removeClass("selected")}';
        $tip_image = '/images/icons/hier_child_filled.png';
        $tip_class = 'tooltip';
        $csel = "#$parent_res_marker_id";
        $tip_title = "Block: Child";
        $tip_js .= 'title="'.$tip_title.'" onMouseOver="$(\''.$csel.'\').addClass(\'selected\')" onMouseOut="$(\''.$csel.'\').removeClass(\'selected\')"';
        $tips .= html_image($tip_image, $tip_title, array('js' => $tip_js, 'class' => $tip_class, 'height' => round(0.9 * $bar_h), 'width' => round(0.9 * $bar_h)))."&nbsp;";
        $classes[] = 'block'; // allow drag right/left + up/down (change dates + apartment)
      }
      
    }
    //else {dump($resObj);die();}
  
    if($cal_params->show != 'invoice') {
      
      if($resObj->alert) $tips .= html_tooltip($resObj->alert, 'info', array('class' => 'cal-alert tooltip-jq'));

      // was
      // if($resObj->alert) $tips .= tooltip('tip-'.$id , $resObj->alert, array('data-tip' => 'foo', 'class' => 'cal-alert tooltip-jq', 'style' => 'warning')); 

      if($left > 0 && $right < $canvas_w) {        
        $classes[] = $cal_params->num_apts > 1 ? 'drag_xy' : 'drag_x'; // allow drag right/left + up/down (change dates + apartment)
        if($resize) $classes[] = 'round3 resize_ew'; // allow resize right/left (checkin and checkout)
      } else {
        $classes[] = 'drag_y'; // change apartment, but not dates
        if($resize) {
          if($left > 0) {
            $classes[] = 'round3-left resize_w'; // allow resize left (change echeckin
          } else if($right < $canvas_w) {
            $classes[] = 'round3-right resize_e'; // allow resize right (checking)
          }
        }
      }
    }    
  }

  
  // if($overlap) $classes[] = 'transparent';
  //$classes[] = 'resize_ew';
  $classes_str = implode(' ', $classes);
  $z = 200 - $resObj->status; // higher z-index for lower status (requests)
  if($resObj->base_class == 'reservation' && $resObj->other) $z -= 100; // reservations should show behind events in event mode
  $small_font_size = round($cal_params->cell_h * 0.6);
  $icon = $resObj->base_class == 'event' ? event_icon($obj_type, $bar_h, -$bar_h, $marker_id.'-icon') : ''; // event icon

  // $data_json = "data-data='".json_encode(array('res_id' => $id, 'apt_id' => $resObj->apt_id, 'checkin' => $in, 'checkout' => $out))."'";

  // begin output
  $output = "<div data-in='".$in_date."' data-out='".$out_date."' data-width='".$cell_count."' class='$classes_str' $data_json id='$marker_id' $js style='width:".$w."px;top:".$top_b."px;left:".($left ?: 1)."px;'>".$icon.$tips.html_div($res_text, 'res-text', array('width' => round(0.9*$w)))."</div>";

  $invoce_print = false;
  if($invoce_print) { // expensive DB lookup
    if($resObj->org_id && $u_level >= USER_LEVEL_STAFF && $resObj->obj_type == 'reservation' && $resObj->status == STATUS_RESERVED && $resObj->paid > 0 && $resObj->corporate) {
      $inv_search = array('res_id' => $id, 'start_date' => array('<', $end), 'end_date' => array('>', $start), 'status' => INV_STATUS_COMPLETE, 'sequence' => array('>', 0));
      //dump($inv_search);
      if($invObjs = find_objects('invoice', $inv_search)) {
        foreach($invObjs as $invObj) {
          $invObj = add_calobj_helperfields($cal_params, $invObj);
          $output .= print_res_marker($cal_params, $invObj);
        }
      }
    }
  }
  return $output;
}


// print guide (days or hours)
function print_calendar_hours($cal_params, $i=0, $options=NULL) {
  $calendar_dates = get_calendar_dates($cal_params);
  $start_hour = $cal_params->start_hour;
  $end_hour   = $cal_params->end_hour;
  $period     = $cal_params->period;
  $dateAr = $calendar_dates[0];
  $today = $dateAr['date'];
  $items = $classes = $item_options = array();
  $index=0;
  for($p=0;$p < $period; $p++) {
    for($hour=$start_hour;$hour < $end_hour; $hour++) {
      $class = 'cal_guide cal_day cell_w';
      $classes[] = $class;
      $item_options[] = array('id' => "cal_guide-$i-$index", 'data-date' => $today.' '.dectime2hm($hour, true, true)); 
      $items[] = html_div($hour, 'guide_title').html_div('', 'guide-box');       
      $index++;
    }
  }
  $list = html_ul($items, '', array('id' => "cal_guides-$i", 'item_classes' => $classes, 'item_options' => $item_options));
  return $list;
}

function print_calendar_days($cal_params, $i=0, $options=NULL) {
  if($cal_params->hide_days) return '';
  $calendar_dates = get_calendar_dates($cal_params);
  //dump($calendar_dates);
  $field = $options['field'] ?: 'day'; 
  $show_weekday = isset($options['show_weekday']) ? $options['show_weekday'] : true;
  $items = $classes = $item_options = array();
  foreach($calendar_dates as $index => $dateAr) {
    $weekdaynum = $dateAr['weekday'];
    $class = 'cal_guide cal_day cell_w';
    if($dateAr['today']) $class .= " today";
    elseif($weekdaynum==6 || $weekdaynum==7) $class .= " weekend";    
    $classes[] = $class;
    $day = $dateAr[$field];
    $dateAr['index'] = $index;
    
    $nav = array('d' => $day);
    $day_link = html_link("?d=$day", $day,  array('data-url' => $url, 'data-nav' => json_encode($nav), 'title' => phrase('view_day', CAPITALIZE), 'class' => 'cal_nav')); 

    $title = ($cal_params->allow_day_view ? $day_link : $day).($show_weekday ? html_break().$dateAr['weekday_name_short'] : '');
    $item_options[] = array('id' => "cal_guide-$i-$index", 'data-index' => $index, 'data-date' => $dateAr['date']); // json_encode($dateAr) 
    $items[] = html_div($title, 'guide_title').html_div('', 'guide-box'); // ." ".$monthAr['year'];
  }
  //$item_options[] = array('data-nav' => $nav);
  $list = html_ul($items, 'cal-guide-day', array('id' => "cal_guides-$i", 'item_classes' => $classes, 'item_options' => $item_options));
  return $list;
}

// prints guide for each day
// only used in day view for multiple days
function print_calendar_dates($cal_params) {  
  $period = $cal_params->period;
  if($cal_params->hide_days || $period <= 1) return '';    
  $dates = get_calendar_dates($cal_params);
  $output .= "<div class='dates'>\n";
  $width = $cal_params->canvas_w / $period;
  for($i=0;$i < $period;$i++) {
    $date = $dates[$i];
    $classes[]= ++$i%2 ? 'odd' : 'even';
    $item_options[] = array('width' => $width);
		$items[] = print_calendar_date($cal_params, array('date' => $date, 'class' => $class, 'left' => $left));
  }
  $output = html_div(html_ul($items, '', array('item_classes' => $classes, 'item_options' => $item_options)), 'guide cal-months');
  return $output;
}

// guide for months
function print_calendar_months($cal_params) {
  return html_div('<ul class="month-list"></ul>', 'guide cal-months');
  /** old code 
  if($cal_params->hide_months) return '';
  $calendar_months = get_calendar_months($cal_params);
  //dump($calendar_months);
  $items = $classes = $item_options = array();
  foreach($calendar_months as $monthAr) $total_days += $monthAr['days_shown'];
  
  foreach($calendar_months as $monthAr) {
    $classes[]= ++$i%2 ? 'odd' : 'even';
    $days = $monthAr['days_shown'];
    $percent = ($days / $total_days) * 100;
    //$item_options[] = array('width' => $monthAr['days_shown'] * $cal_params->cell_w);
    $item_options[] = array('style' => 'width:' + $percent + '%;display:table-cell');
    $debug = "$days / $total_days = $percent %";
    $items[] = $monthAr['name'].' '.$monthAr['year']. " ".$debug; // ." ".$monthAr['year'];
    
  }
  //$output = html_div(html_ul($items, '', array('item_classes' => $classes, 'item_options' => $item_options)), 'guide cal-months');
  // $output = html_div(html_ul($items, 'red-border', array('style' => 'width:100%;display:table', 'item_classes' => $classes, 'item_options' => $item_options)), 'guide cal-months');
  $output = html_div('<div class="guide cal-months"><ul class="month-list"></ul></div>', 'guide cal-months');
  return $output;
  */
}


function print_calendar_action_links($cal_params) {
  $items = array();
  $items[] = change_link($cal_params);
  $items[] = stats_link($cal_params);
  $items[] = help_link($cal_params);
  $items[] = pricing_link($cal_params);
  
  $output = html_ul($items);
  return $output;  
}
  

function pricing_link($cal_params) {
  $linkOptions['title'] = "Click to show pricing rules";    
  $linkOptions['id'] = 'cal_show_pricing';
  $output = html_link('#', "Pricing", $linkOptions);
  return $output;  
}

function help_link($cal_params) {
  $linkOptions['title'] = "Click to show help screen";    
  $linkOptions['id'] = 'cal_show_help';
  $output = html_link('#', "Help", $linkOptions);
  return $output;  
}

function stats_link($cal_params) {
  $linkOptions['title'] = "Click to show the statistics";    
  $linkOptions['id'] = 'cal_show_stats';
  $output = html_link('#', "Stats", $linkOptions);
  return $output;  
}

function change_link($cal_params) {
  global $u_level;  //$u_level = $cal_params->user_level; 
  $auth = $u_level == USER_LEVEL_OWNER || $u_level >= USER_LEVEL_STAFF;
  if(!$auth) return '';
  
  $res_ids = $cal_params->res_list ? explode(',', $cal_params->res_list) : array();
  $count = count($res_ids);
  $res_search = $_SESSION['calendar_search'];
  
  if(!($res_ids || $res_search))  return 'No changes';  // what's more efficient: sending res_ids or params ? (start/end/host_id/city);
  
  $linkOptions = array();
  $linkOptions['title'] = "Click to show the latest changes";    
  $linkOptions['data-obj_type'] = $cal_params->obj_type;
  
  if($res_ids && count($res_ids) < 200) $res_search = array('id' => implode(',',$res_ids));  // speeds up retrieval, but don't use if too long (long HTML)

  $linkOptions['data-search'] = json_encode($res_search);
  $linkOptions['id'] = 'cal_show_changes';
  $output = html_link('#', "Changes", $linkOptions).html_div('', 'hidden', array('id' => 'cal_changes'));
  return $output;  
}

// old code
function bottom_table($cal_params) {
  $u_level = $cal_params->user_level; 
  $auth_details = $u_level == USER_LEVEL_OWNER || $u_level >= USER_LEVEL_STAFF;

	$col1 .= print_statuses($cal_params).html_break();

  //if($cal_params->base_class == 'event' || $cal_params->base_class == 'invoice') $col1 .= print_res_toggle($cal_params);
  if($cal_params->standalone) return $col1;
  
  if($cal_params->base_class == 'event') $col1 .= print_res_toggle($cal_params);
  // else if($cal_params->base_class == 'reservation') $col1 .= print_inv_toggle($cal_params);
  //if($cal_params->obj_type == 'event' || ($auth_details && $cal_params->show == 'reservation')) $col1 .= html_div(print_event_types($cal_params), 'event_types');

  if($cal_params->obj_type == 'event') $col1 .= html_div(print_event_types($cal_params), 'event_types');
	$cols[] = $col1;
  return css_columns($cols);

}

function print_statuses($cal_params) {
  $height = 16;
  $st = object_status_objtype($cal_params->obj_type);
  $toggle = $cal_params->toggle_data;

  $statuses = find_objects($st, array('id' => array('>=', $cal_params->status)));
  $obj_type = $cal_params->obj_type;
  foreach ($statuses as $st) {
    $id = "toggle_status_".$st->name;
    $on = !isset($toggle->$id) || $toggle->$id;
    $selector = ".cal_entries .$obj_type.".$st->name;
    $rows[] = array(
      // toggle_selector($id, $selector, 'checkbox', $on).' ',
      html_span($st->name, 'bold', "style='color:#$st->color'"),
      $st->description);
  }
  return html_break().html_table($rows, array('class' => 'padded-table', 'header' => 0));
}

function event_icon($evt_type, $size, $offset=0, $id='') {
  if($evt_type == 'Reservations') return '';
  $style = $offset ? "style='position:absolute;left:".$offset."px'" : '';
  if($id) $style .= " id='$id'";
  return html_image("/images/icons/".strtolower($evt_type)."_big.png", $evt_type, array('js' => $style, 'height' => $size, 'width' => $size));
}

function print_res_toggle($cal_params) {
  $selector = ".cal_entries .reservation"; // todo: add complete, blocked
  $toggle = $cal_params->toggle_data;
  $onclick = 'if($(this).is(":checked")) {ajax_draw_cal_entries("reservation");} ';
  $id = 'toggle_res';
  $script .= "toggle_selector_array['$id'] = '$selector';\n";
  if($on = $toggle->$id) $script .= 'ajax_draw_cal_entries("reservation");';
  return toggle_selector($id, $selector, 'checkbox', $on, $onclick)." ".html_b("Reservations").html_script_inline($script);
}

function print_inv_toggle($cal_params) {
  $selector = ".cal_entries .invoice"; // todo: add complete, blocked
  $toggle = $cal_params->toggle_data;
  $onclick = 'if($(this).is(":checked")) {ajax_draw_cal_entries("invoice");} ';
  $id = 'toggle_inv';
  $script .= "toggle_selector_array['$id'] = '$selector';\n";
  if($on = $toggle->$id) $script .= 'ajax_draw_cal_entries("invoice");';
  return toggle_selector($id, $selector, 'checkbox', $on, $onclick)." ".html_b("Invoices").html_script_inline($script);
}

function print_event_types($cal_params) {
  $height = 16;
  $types = find_objects('event_type');
  $toggle = $cal_params->toggle_data;
  $draw = false;
  foreach ($types as $type) {
    $name = strtolower($type->name);
    $id = 'toggle_event_'.$name;
    $selector = "#cal_entries .".$name;
    if($on = $toggle->$id || $cal_params->show == 'event') $draw = true;
    $onclick = $cal_params->obj_type == 'event' ? '' : 'if($(this).is(":checked")) {ajax_draw_cal_entries("event");} ';
    $script .= "toggle_selector_array['$id'] = '$selector';\n";
    $col0[] = toggle_selector($id, $selector, 'checkbox', $on, $onclick).html_spacer($height);
    $col1[] = event_icon($name, $height);
    $col2[] = html_span($name, 'bold').html_spacer($height);
  }
  if($draw) $script .= 'ajax_draw_cal_entries("event");'; // one was checked, draw now
  return css_columns(array($col0, $col1, $col2, '')).html_script_inline($script);
}

// calendar filters, called from /home/ajax/calfilters.php
function print_filters($cal_params, $filters) {
  $vars = array('t' => 'property_type', 'host' => 'host_id', 'cid' => 'city_id');
  foreach($vars as $g => $c) $myfilters[$g] = $filters[$g] ?: $cal_params->$c;
  //dump($myfilters);

  $output .= print_city_filter($cal_params,  $myfilters);
  $output .= print_type_filter($cal_params,  $myfilters);
  $output .= print_host_filter($cal_params, $myfilters);
  
  return $output;  
}

function print_city_filter($cal_params, $filters) {
  $type    = $filters['t'];

  if(!$type) return 'no type';
  
  $cities = $_SESSION['show_cities'];
  if(!$cities) return 'no cities';
  
  //if(!$cities) return 'c:<input name="cid" type="text" value="'.$cal_params->city_id.'">';

  $city_ids = implode(',', array_keys($cities));
  
  $output = "<select name='cid' class='cal_filter' id='city_filter'>\n";

  $qs = "select msCity.id, msCity.name, count(property.id) as count from property, msCity
         WHERE property.city_id=msCity.id AND msCity.active=1 AND property.active=1
         AND msCity.id in ($city_ids)
         GROUP BY msCity.name";
  //echo($qs);
  // AND property.type='$type' 
  $cities = db_getAll($qs);

  foreach($cities as $cityObj) {
    $id = $cityObj->id;
    $output .= "  <option value='$id' ".($filters['cid']==$id ? "selected='selected'" : "")."> $cityObj->name </option>\n";
  }
  $output .= "</select>\n";
  return $output;
}

function print_type_filter($cal_params, $filters) {
  $city_id = $filters['cid'];
  if(!$city_id) return "No city";
  $types = $_SESSION['show_types'];
  if(!$types) return 't:<input name="t" type="text" value="$cal_params->property_type">';
  $output = "<select name='t' class='cal_filter' id='type_filter'>\n";

  $qs = "select property_type.id, property_type.name, count(property.id) as count from property, property_type
         WHERE property.city_id=$city_id AND property.active=1 and property.type = property_type.name
         GROUP BY property_type.name";
  $types = db_getAll($qs);

  foreach($types as $pTypeObj) {
    $type = strtolower($pTypeObj->name);
    $output .= "  <option value='$type' ".($filters['t']==$type ? "selected='selected'" : "").">".ucfirst($type)."</option>\n";
  }
  $output .= "</select>\n";
  return $output;
}


function print_available_filter($cal_params, $filters) {
  $ptype = $filters['t'];
  $city_id = $filters['cid'];
  $searchbar = searchbar($ptype, $city_id, array('show_submit' => false, 'show_headers' => false, 'init_date' => false));
  return $ptype && $city_id ? html_div(' '.phrase("available:", CAPITALIZE).' '.$searchbar, 'cal_filter') : '';
}


// used for search
function calendar_search($cal_params) {
  // return html_image_link(get_object_link($cal_params->obj_type)."&amp;_show_search=1", "/images/icons/search.png", phrase('search', CAPITALIZE));
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

function event_dialog($cal_params, $obj=NULL, $id=0, $apt_list='', $obj_type='event') {
  global $eventTypeArray;
  global $eventTypes;
  global $eventStatusArray;
  global $u;

  if(!$id && $obj_type != 'event') $obj->type = $eventTypes[$obj_type];

  $repeatArray = array('' => phrase('none'), '1 DAY' => phrase('every_day'), '1 WEEK' => phrase('every_week'), '2 WEEK' => phrase('every_2_weeks'), '1 MONTH' => phrase('every_month'), '1 YEAR' => phrase('every_year'));
  if(!$cal_params->target_day) unset($repeatArray['1 DAY']); // don't show repeat daily in month view
  $remindArray = array('' => phrase('none'), '-5 MINUTE' => '5 '.phrase('minutes').' '.phrase('before'), '-15 MINUTE' => '15 '.phrase('minutes').' '.phrase('before'), '-30 MINUTE' => '30 '.phrase('minutes').' '.phrase('before'), '-1 HOUR' => '1 '.phrase('hour').' '.phrase('before'), '-2 HOUR' =>'2 '.phrase('hours').' '.phrase('before'), '-1 DAY' => '1 '.phrase('day').' '.phrase('before'), '-2 DAY' => '2 '.phrase('days').' '.phrase('before'));

  $fieldset_evt = array('Type'   => select_from_array('type', $eventTypeArray, $obj->type, array("class" => "form-control")),
                        'Status' => select_from_array('status', $eventStatusArray, $obj->status, array("class" => "form-control")),
                        'Res. ID'   => form_input('res_id', 'text', $obj->res_id, array('size' => 8)));

  $fieldset_guest = array('Customer'   => form_input('customer_name', 'text', $obj->customer_name, array('size' => 40)),
                          '# Guests'   => select_from_array('customer_count', num_array(1, 10, 1), $obj->customer_count, array("class" => "form-control")));

  $fieldset_notes = array('Name'   => form_input('name', 'text', $obj->name, array('size' => 40, 'max' => 100)),
                          'NB!'   => form_text('alert', $obj->alert, array('rows' => 1, 'cols' => 57, 'resizable' => false)),
                          'Notes'    => form_text('notes', $obj->notes, array('rows' => 2, 'cols' => 57, 'resizable' => false)));

  // $calparams['defaultDate'] = "'3-4-10'";
  // $calparams['defaultDate'] = "'10-11-12'"; // can't get this to work...
  $fieldset_repeat = array('Repeat' => select_from_array('repeat_interval', $repeatArray, $obj->repeat, array("class" => "form-control")),
                           'Repeat Until' => datepicker('repeat_until', $calparams, $obj->repeat_until));

  $fieldset_remind = array('Reminder phone' => form_input('reminder_phone', 'text', $obj->reminder_phone ? $obj->reminder_phone : $u['phone'], array('size' => 15, 'max' => 20)),
                             'Reminder email' => form_input('reminder_email', 'text', $obj->reminder_email ? $obj->reminder_email : $u['email'], array('size' => 30, 'max' => 50)),
                             'Reminder 1' => select_from_array('reminder_period1', $remindArray, $obj->reminder_period1, array("class" => "form-control")),
                             'Reminder 2' => select_from_array('reminder_period2', $remindArray, $obj->reminder_period2, array("class" => "form-control")));

  $guest   = html_fieldset($fieldset_guest,  'fieldset_top', array('translate' => true));
  $event   = html_fieldset($fieldset_evt,    'fieldset_top', array('translate' => true));
  $notes   = html_fieldset($fieldset_notes,  'fieldset_top', array('translate' => true));
  $repeat  = html_fieldset($fieldset_repeat, 'fieldset_top', array('translate' => true));
  $remind  = html_fieldset($fieldset_remind, 'fieldset_top', array('translate' => true));


  // dates
  $dates = html_break().html_div(dialog_period($cal_params, $obj), '', "id='dialog_period'").html_break(2);
  $hidden_fields = array();
  return array2form($hidden_fields).implode("\n", array($event, $dates, $guest, $notes, $repeat, $remind));
}

// The custom javascript pop-up modal dialogs used for new/change reservation in the calendar
function res_dialog($cal_params, $obj=NULL) {
  global $salutationArray;
  global $langArray;
  global $u_sites;
  global $u_hostid;
  $city_id = $cal_params->city_id ?: $obj->city_id;
  $city_tt_rate = get_object('geo_city', $city_id, 'city_tax');
  $debug = '';
  //$debug = "city_id=$city_id tax=$city_tt_rate<br>";
  $id = $obj->id = $obj->res_id ?: $obj->id;
  
  
  $user_level = $cal_params->user_level; 
  $apt_id = $obj->apt_id;
  $ptype = $cal_params->property_type ?: $obj->property_type;
  $apt_list = $cal_params->apt_list;
  $status_name = $id ? get_object('res_status', $obj->status, 'name') : '';
  
  if($apt_id && $ptype) {
    $apt = get_object($ptype, $obj->apt_id);
  } 
  
  if($id) $head_invoice_locked = find_object('invoice', array('res_id' => $id, 'head_invoice' => 1, 'locked' => 1), 'invoice.id'); // the first invoice is paid in full, lock down rates and fees  
  if($head_invoice_locked) $lock_rate = true;  

  $rate_lock_check      = $obj ? ($lock_rate || $obj->rate_locked ? true : false) : false;             
  $discount_lock_check  = $obj ? ($lock_rate || $obj->discount_locked ? true : false) : false;
  $fee_lock_check       = $obj ? ($lock_rate || $obj->fee_locked ? true : false) : true;
  $monthly_lock_check   = $obj ? ($obj->monthly_billing ? true : false) : true;
  $tt_lock_check        = $obj ? ($obj->charge_city_tax ? true : false) : $apt->charge_city_tax;
  $invoice_check        = true;
  
  //$debug = "rate_locked=$rate_lock_check lock_rate=$lock_rate id=$head_invoice_locked".dump($obj, true);
  //$debug = "Obj:".dump($obj, true)."Apt:".dump($apt, true);
  // status, name, num_guests, email, company
  //$num_guest_array = num_array(1, $apt ? $apt->max_guests : 8);

  $max_guests = db_getOne("SELECT max(max_guests) FROM property WHERE id IN ($apt_list)");
  $num_guest_array = num_array(1, $max_guests * 2);

  // res statuses
  $resStatusObjects = $_SESSION['resStatusObjects'] = find_objects('res_status', [], array('sort' => 'id'));
  foreach($resStatusObjects as $pStatusObj) $resStatusArray[$pStatusObj->id] = $pStatusObj->name;

  //$stAr = array(STATUS_UNCONFIRMED => 'Unconfirmed', STATUS_PROPOSED => 'Proposed', STATUS_RESERVED => 'Reserved', STATUS_BLOCKED => 'Blocked');
  $stAr = array(STATUS_PROPOSED => 'Proposed', STATUS_RESERVED => 'Reserved' /*, STATUS_BLOCKED => 'Blocked'*/);
  $stAr2 = array(STATUS_CANCELLED => 'Cancelled', STATUS_DRAFT => 'Draft', STATUS_UNCONFIRMED => 'Unconfirmed', STATUS_PROPOSED => 'Proposed', STATUS_RESERVED => 'Reserved', STATUS_BLOCKED => 'Blocked', STATUS_COMPLETE => 'Complete');
  $stAr3 = array(STATUS_CANCELLED => 'Delete', STATUS_BLOCKED => 'Blocked');
  
  $statusArray = $id ? $resStatusArray : $stAr;

  $blocked = $id && $obj->status == STATUS_BLOCKED;
  if($blocked) {
    $statusArray = $stAr3;
  } else if($id && $obj->status > 0 && $obj->checkin < today()) {
    $statusArray = $stAr2; // don't allow cancel past reservations  
  }
  
  /** are we allowed to change reservation status ? */
  $auth_edit_status = 1; // ($id  && $user_level < USER_LEVEL_STAFF) || $blocked;  

  $fieldset_name = array('salutation' => select_from_array('salutation', $salutationArray, $obj->salutation, array("class" => "form-control-off")),
                         'first'      => form_input('first_name', 'text', $obj->first_name, array('size' => 10, 'max' => 50)),
                         'last'       => form_input('last_name', 'text', $obj->last_name, array('size' => 16, 'max' => 50)),
                         'guests'     => select_from_array('num_guests', $num_guest_array, $id ? $obj->num_guests : 1,  array('onchange' => "update_overlay_rate()", "class" => "form-control-off")));

  $fieldset_email = array('email'      => form_input('email_address', 'text', $obj->email_address, array('size' => 20, 'max' => 50)).
                                         form_checkbox('send_email', $obj->send_email ? true : false).phrase('send_mail', CAPITALIZE).'?'.
                                         '',
                          'phone'      => form_input('cell_phone', 'text', $obj->cell_phone, array('size' => 20, 'max' => 50)),
                          'status'     => $auth_edit_status ? select_from_array('status', $statusArray, $obj->status, array("class" => "form-control-off", "first" => " ")) : $status_name ,
//                          'language'   => select_from_array('language', $langArray, pick_first($obj->language, $_SESSION['language'], 'en')), array("class" => "form-control-off")
                          );


  // AFR is default site ; todo: depends on who is logged in
  
  // search
  if($user_level >= USER_LEVEL_STAFF) {
    $site_id = 0;
    $site_filters['live'] = 1;  
    $site_filters['active'] = 1;  
    $site_filters['client_id'] = $_SESSION['client_id'];  
    if($u_sites) $site_filters['site.id'] = explode(',', $u_sites);
  
    $default_site_id = $_SESSION['site_id']; // hack - boats
    $site_name = $id && $obj->site_id ? get_object('site', $obj->site_id, 'name') : '';

    $fieldset_email['site'] = object_select('site_id', 'site', array('value' => $obj->site_id ? $obj->site_id : $default_site_id, 'search' => $site_filters, 'sort' => 'name'));
  } else {
    $site_id = $apt->site_id ?: $_SESSION['site_id'];  
  }
  
  $agent_name = $obj->agent_id ? get_object('agent', $obj->agent_id, 'name') : '';

  /**
  if(!$obj->apt_id) { // host chooser for parking lot (unknown apartment)
    if($host_id = $cal_params->host_id && is_numeric($cal_params->host_id) ? $cal_params->host_id : $u_host_id) { // We know the host
      $host_filters['host.id'] = $host_id;
      //$host_filters = "host.id=$host_id";
      $obj->host_id = $host_id;
    } else {  // pick the host
      // $host_filters = "types like '%$obj->property_type%' AND cities like '%$city_id%'";
    }
    if($_SESSION['client_id']) $host_filters['client_id'] = $_SESSION['client_id'];
    // $fieldset_email['host'] = get_select_box_off('host_id', 'host', $obj->host_id, $onchange, $owne§r_filters, '').html_break();
    $fieldset_email['host'] = object_select('host_id', 'host', array('value' => $obj->host_id, 'filters' => $host_filters)).html_break();
  }
  */

  if($user_level >= USER_LEVEL_STAFF) {  //&& ($name || !$id)) {
    $fieldset_email['agent'] = object_select('agent_id', 'agent', array('value' => $obj->agent_id, 'sort' => 'name')).html_break();
  }

  $org_name = $obj->org_id ? get_object('org', $obj->org_id, 'org_name') : '';

  if($_SESSION['client_id']) $org_filters['client_id'] = $_SESSION['client_id'];
  $org_filters['active'] = 1;
  
  if($user_level >= USER_LEVEL_STAFF) {
    
    $org_fields = array('org_name', 'invoice', 'comment', 'address.salutation', 'address.first_name', 'address.last_name', 'address.address1', 'address.address2', 'address.city', 'address.state', 'address.zip', 'address.country', 'address.work_phone', 'address.cell_phone', 'address.email_address');
    $add_contact_button = add_object_inline('org', $org_fields, array('context' => 'recordset', 'reload' => 0, 'handler' => 'new_org_handler', 'parent_id' => 'org_id')); 
    
    // org chooser
    //$fieldset_email['company'] = get_select_box_off('org_id', 'org', $obj->org_id, $disabled, $org_filters, $fld_sort).$add_contact_button;    
    $fieldset_email['company'] = object_select('org_id', 'org', array('value' => $obj->org_id, 'filters' => $org_filters, 'sort' => 'org_name'))." ".$add_contact_button;    
    

    if($org_id = $obj->org_id) {
      $contact_filters['org_id'] = $org_id; 
      $contact_class = '';
    } else {
      $contact_filters['org_id'] = -1; // hack: show none
      $contact_class = 'hidden';
    }

    $contact_checkbox = form_checkbox('invoice_contact', $obj->invoice_contact ? true : false).phrase('invoice_contact', CAPITALIZE);
    if($id) { // existing reservation, just show contact (no edit)
      if($contact_id = $obj->contact_id) {
        $contactObj = get_object('contact', $contact_id);
        $contact_chooser = get_name_string($contactObj);
        
      }
    } else { // new reservation
      $contact_chooser = object_select('contact_id', 'contact', array('value' => $obj->contact_id, 'filters' => $contact_filters, 'noajax' => true));
    }

    $fieldset_email['contact'] = html_div("$contact_chooser $contact_checkbox", $contact_class, 'id="org_contact"');
  }
  
  $fieldset_note = array('NB!'      => form_text('alert', $obj->alert, array('rows' => 1, 'cols' => 50, 'max' => 255, 'resizable' => false)),
                          'staff notes'  => form_text('notes', $obj->notes, array('rows' => 1, 'cols' => 50, 'resizable' => false)),
                          'message to guest' => form_text('message', $obj->message, array('rows' => 1, 'cols' => 50, 'resizable' => false)));

  if($obj->property_type == 'shortstay') {
    // arrival
    $obj->admincal = true; // hack to allow airport pickup for any apt when admin-booking
    $fieldset_arri = arrival_fields($obj);
  } else if($obj->property_type == 'boat') {
    $obj->city_id = $cal_params->city_id;
    $locs = boarding_locs($obj);
    $fieldset_boar = boarding_fields($obj, $locs, false);
  }

  // address
  $fieldset_addr = address_fields($obj, array('hide_phone' => 1));

  // fees
  $fieldset_fees = fee_fields($obj, $lock_rate);
  
  // rate
  if($day_view = is_object($obj) ? $obj->num_hours > 0 : $cal_params->target_day > 0) { // show date or time ?
    $rate_type_array['hour'] = phrase('hour', CAPITALIZE);
  } else { // multi-day view
    $rate_type_array['day'] = phrase('day', CAPITALIZE);
    //$rate_type_array['month'] = array(phrase('month', CAPITALIZE), "disabled='disabled'");
    $rate_type_array['month'] = phrase('month', CAPITALIZE);
    //if($obj->rate_type == 'month' || $obj->num_days >= 30) $rate_type_array['month'] = phrase('month', CAPITALIZE);
  }
  $rate_type_array['total'] = phrase('total', CAPITALIZE);
  if($ptype == 'boat') {
    $rate_type_array['person'] = phrase('person', CAPITALIZE);
    $rate_type_array['pp_hour'] = phrase('person_per_hour', CAPITALIZE);
    //$rate_type_array['package'] = phrase('package', CAPITALIZE);
  }
  
  if($lock_rate) {
    $lock_text = " (".phrase('locked', CAPITALIZE).")"; //: Invoice $head_invoice_locked paid in full<br>");
    $lock_js = "readonly='readonly'";
    $lock_cb = " onclick='return false' onkeydown='return false'";
  } else {
    //$lock_cb = 
  }
  
  /** no rate info for blocks */
  if(!$blocked) {
    $rate_type_chooser = $lock_rate ? phrase($obj->rate_type).form_input('rate_type', 'hidden', $obj->rate_type) : select_from_array('rate_type', $rate_type_array, $obj->rate_type, array("onchange" => "update_overlay_rate_type()", "class" => "form-control-off"));
    $fieldset_rate = array('Rate'     => form_input('rate_base', 'text', $obj->rate_base, array('size' => 10, 'js' => "$lock_js onchange='custom_rate();update_overlay_total()' autocomplete='off'")).html_b('/').
                                         $rate_type_chooser.
                                         html_span('Locked?'.form_checkbox('rate_locked', $rate_lock_check, array('js' => $lock_cb)), 'abs_right'),
                                         'discount' => form_input('discount', 'text', $obj->discount, array('size' => 10, 'js' => $lock_js ?: " onchange='lock_discount();update_overlay_total()' autocomplete='off'")).
                                         html_span('Locked?'.form_checkbox('discount_locked', $discount_lock_check, array('js' => $lock_cb)), 'abs_right'),
                           'Commission' => form_input('commission', 'text', $obj->commission, array('size' => 10, 'js' => "autocomplete='off'")).
                                         html_span('Bill monthly?'.form_checkbox('monthly_billing', $monthly_lock_check), 'abs_right')
                          );
    
    $deposit_percentage = $id ? $obj->deposit_percentage : $apt->deposit;

    $fieldset_rate['Deposit %'] = form_input('deposit_percentage', 'text', $deposit_percentage, array('size' => 10, 'js' => "autocomplete='off'")).
      //html_span(form_input('city_tax_rate', 'text', $obj->city_tax_rate ?: $city_tt_rate, array('size' => 4, 'js' => " onchange='update_overlay_total()' autocomplete='off'")).
      html_span(' City tax?'.form_checkbox('charge_city_tax', $tt_lock_check, array('js' => ($lock_cb ?: "onchange='update_overlay_total()'"))), 'abs_right');
                                  
    if($ptype == 'boat') { 
      $fieldset_rate[phrase('package', CAPITALIZE)] = package_chooser($ptype, $apt_id, array('value' => $obj->package_id, 'onchange' => 'update_package_rate();'));  
      $fieldset_rate[phrase('catering', CAPITALIZE)] = catering_chooser($ptype, $apt_id, array('value' => $obj->catering_id));  
      //$fieldset_rate['package'] = object_select('package_id', 'product', array('value' => $obj->package_id, 'onchange' => 'update_package_rate();', 'filters' => array('category_id' => PRODUCT_CATEGORY_PACKAGE))); 
  
    }
 

    $cur = html_span($obj->currency, 'reservation-currency');
    $rate_str = html_span(($obj->rate_base ? formatFloat($obj->rate_base,2) : '0.00'), 'currency_total reservation-rate');
    $type_str = html_span(($obj->rate_type ?: '??'), 'currency_total reservation-rate_type');
    $discount_str = html_span($obj->discount, 'reservation-discount');
  
    $total_str = html_span(($obj->grand_total ? formatFloat($obj->grand_total,2) : '0.00'), 'currency_total reservation-total');
    $fees_str = html_span(($obj->fees ? formatFloat($obj->fees,2) : '0.00'), 'currency_total reservation-fees');
    $tax_str  = html_span(($obj->city_tax ? formatFloat($obj->city_tax,2) : '0.00'), 'currency_total reservation-tax');
  
    $fieldset_rate['City tax'] = $tax_str." ".$cur;
    $fieldset_rate['Fees']  = $fees_str." ".$cur;
    $fieldset_rate['Total'] = $total_str." ".$cur;
    $fieldset_rate[' '] = form_input('grand_total_new_res', 'hidden');
  
        
    $rate_link = html_link('#cal_dialog_tab_Rate', phrase('rate', CAPITALIZE).": $rate_str / $type_str", array('class' => 'goto-tab', 'data-index' => 1));
    $fees_link = html_link('#cal_dialog_tab_Fees', " + ".phrase('fees', CAPITALIZE).": $fees_str", array('class' => 'fee_link goto-tab'.($obj->fees > 0 ? "" : " hidden"), 'data-index' => 2));
    $tax_link  = html_link('#cal_dialog_tab_Tax',  " + ".phrase('tax', CAPITALIZE).": $tax_str" , array('class' => 'tax_link goto-tab'.($obj->city_tax > 0 ? "" : " hidden"), 'data-index' => 1));
    $discount_link  = html_link('#cal_dialog_tab_Rate',  " - ".phrase('discount', CAPITALIZE).": $discount_str" , array('class' => 'discount_link goto-tab'.(parseInt($obj->discount) > 0 ? "" : " hidden"), 'data-index' => 1));
    //$discount_link .= "** dis=$obj->discount **";                                
    $rates = html_div("$rate_link $discount_link $tax_link $fees_link = $total_str $cur", 'dialog-total', array('id' => 'dialog-total'));
    $rates .= html_div('<i class="fa fa-spinner fa-spin"></i> Fetching rates...', 'hidden', array('id' => 'dialog-total-loading')); 
  
    $rate_class = $obj->rate_class ?: RATE_CLASS_STANDARD;
    $rate_class_name = get_object('rate_class', $rate_class, 'name');
    $rate_class_str = ucfirst($rate_class_name)." ".phrase('rate', CAPITALIZE);
    if($pbid = $obj->price_breakdown_id) {
      $name = get_object('price_breakdown', $pbid, 'reason');
      $link = "/home/app_data.php?oper=price-breakdown&id=$pbid";
      $rate_class_str .= ": ".html_fancybox_link($link, $name); 
    }
    $rates .= html_span($rate_class_str, '', array('id' => 'special_rate')).html_span($obj->rate_locked ? '('.phrase('locked').')' : '', 'margin_left', array('id' => 'special_rate_lock'));

    /** to hold num hours/day/months */
    $rf = " nh:".html_span(0, '', array('id' => 'cal_dialog_nh'));
    $rf .= " nd:".html_span(0, '', array('id' => 'cal_dialog_nd'));
    $rf .= " nm:".html_span(0, '', array('id' => 'cal_dialog_nm'));
    $rf .= " cur:".html_span($obj->currency, '', array('id' => 'cal_dialog_currency'));
    $rf .= " rd:".html_span($obj->rate_base, '', array('id' => 'rate_day'));
    $rf .= " fees:".html_span($obj->fees, '', array('id' => 'cal_dialog_fees'));
    $rf .= " ct:".html_span($obj->city_tax_rate ?: $city_tt_rate, '', array('id' => 'city_tax_rate'));

    $rates .= html_div($rf, 'hidden');
  }
 // begin output

 
  // first tab with hidden fields  
  $num_months = $obj->num_months ?: period_diff($obj->checkin, $obj->checkout, 'm'); 
  
  $hidden_fields = array("site_id" => $obj->site_id, "property_type" => $obj->property_type, "city_id" => $obj->city_id, "user_id" => $obj->user_id, "apartment_name" => $obj->apartment_name);
  if($id) $hidden_fields['res_id'] = $id; /** existing res */
  else $hidden_fields['footype'] = RES_TYPE_ADMIN; /** new res */
  
  if(!$apt_id) {
    $apts = explode(',', $apt_list);    
    foreach($apts as $pid) $aptArray[$pid] = get_object('property', $pid, 'name');
    $fieldset_apt = array(property_type_name($ptype) => select_from_array('apt_id', $aptArray, $apt_id, array('onchange' => 'update_overlay_rate();', "class" => "form-control-off")));
    $apt_ch  = html_fieldset($fieldset_apt, 'fieldset_left', array('translate' => true));
  } else {
    $hidden_fields['apt_id'] = $apt_id;
    //$first_tab = "Apt id=$apt_id";
  }
  
  $name  = html_fieldset($fieldset_name, 'fieldset_top', array('translate' => true));
  $email = css_table(assoc2rows($fieldset_email));
  
  $dates = html_break().html_div(dialog_period($cal_params, $obj), '', "id='dialog_period'").html_break(2);
  
  $first_tab .= array2form($hidden_fields, array('hidden' => true));
  
  $first_tab .= implode("\n", array($apt_ch, $name, $email, $dates, $rates));
  
  $lock_text = '';

  // accordion              
  $tabs[phrase('basic', CAPITALIZE)] = $first_tab;
  if(!$blocked) {
    $tabs[phrase('rate', CAPITALIZE).$lock_text]    = css_table(assoc2rows($fieldset_rate));
    $tabs[phrase('fees', CAPITALIZE).$lock_text]    = css_table(assoc2rows($fieldset_fees)); // already translated
  }
  $tabs[phrase('address', CAPITALIZE)] = html_fieldset($fieldset_addr, 'fieldset_top');
  $tabs[phrase('notes', CAPITALIZE)]   = html_fieldset($fieldset_note, 'fieldset_top', array('translate' => true));

  if($fieldset_arri) $tabs[phrase('arrival', CAPITALIZE)] = html_fieldset($fieldset_arri, 'fieldset_top');
  if($fieldset_boar) $tabs[phrase('boarding', CAPITALIZE)] = html_fieldset($fieldset_boar, 'fieldset_top');

  $output = bootstrap_tabs($tabs, array('id' => 'cal_tabs')); 

  if($id) {
    $buttons = array();
    $buttons[]= html_link(page_link(PAGE_STATUS).$id, html_i('', 'fa fa-globe')." ".phrase('status', CAPITALIZE), array('target' => '_status'));
    $output .= html_div(implode(' ', $buttons), 'margin-top buttons');
      
  }
  //$output .= dump($obj, true);
  return $output;
}

// shows either date pickers (month or week view) or time pickers (day view)
function dialog_period($cal_params, $obj=NULL) {
  $day_view = is_object($obj) ? $obj->num_hours > 0 : $cal_params->target_day > 0; // show date or time ?
  //$start_field = $cal_params->start_field;
  //$end_field = $cal_params->end_field;
  $start_field = 'checkin';
  $end_field = 'checkout';

  if($day_view) {
    $output .= phrase('from', CAPITALIZE).': '.timepicker('checkin', array('onChange' => 'update_overlay_period'), $obj->$start_field);
    $output .= phrase('to', CAPITALIZE).': '.timepicker('checkout', array('onChange' => 'update_overlay_period'), $obj->$end_field);
    $period = $obj ? $obj->num_hours : 0;
    $unit = $period == 1 ? phrase('hour') : phrase('hours');
    $period_str = "$period $unit";
  } else {
      
    /*
    <div class="row input-daterange">
      <div class="col-sm-4">
        <input type="text" value="" class="form-control checkin" name="checkin" placeholder="Checkin">
        <span class="input-icon"><i class="icon-calendar"></i></span>
      </div>
      <div class="col-sm-4">
        <input type="text" value="" class="form-control checkout"name="checkout" placeholder="Checkout">    
        <span class="input-icon"><i class="icon-calendar"></i></span>
      </div>
      <div class="col-sm-4">
        <span class="duration">? days</span>
        <input type="hidden" name="num_days">            
      </div>
    </div>
    */
  
    $row1 = form_input('checkin', 'text', sql2moment($obj->$start_field), array('class' => 'form-control checkin', 'placeholder' => 'Checkin')).html_span(html_i('', 'icon-calendar'), 'input-icon');
    $row2 = form_input('checkout', 'text', sql2moment($obj->$end_field), array('class' => 'form-control checkout', 'placeholder' => 'Checkout')).html_span(html_i('', 'icon-calendar'), 'input-icon');
    $row3 = form_input('num_days', 'hidden').html_span("? days", 'duration');

    $col1 = html_div($row1, 'col-sm-4');
    $col2 = html_div($row2, 'col-sm-4');
    $col3 = html_div($row3, 'col-sm-4');
    $cols = $col1.$col2.$col3;

    //$class = $obj->id ? "" : "input-daterange"; // no javascript for existing  
    $class = "input-daterange";
    //$period_str = html_div($cols, 'row input-daterange', array('data-onchange' => 'update_overlay_period'));
    $period_str = html_div($cols, "row {$class}", array('id' => 'cal_dialog_period', 'data-update' => '1', 'data-onchange' => 'update_overlay_period'));
                                                           
  }

  $output .= html_span($period_str, 'bold', "id='overlay_period'");
  return $output;
}


/** end old dialog */

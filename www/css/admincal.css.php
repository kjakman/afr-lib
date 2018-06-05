<?php
  include_once('../include/set_env.inc');
  include_once("helpers_calendar.inc");
  $cal_params = $_SESSION['cal_params'];
  //dump($cal_params);
  // dump($_GET);
  // overrides
  if(is_array($_GET)) {
    foreach($_GET as $k => $v) {
      if(isset($cal_params->$k)) {
        $cal_params->$k = $v;
      }
    }
  }

  header("Content-type: text/css; charset: utf-8");

?>

body {font-size:16px;}

/* We could generate this from DB */
.admincal .resBar.reservation {background-color: #000}
.admincal .resBar.complete {background-color: #008800 !important;}
.admincal .resBar.blocked {background-color: #880000 !important;}
.admincal .resBar.reserved {background-color: #cc0000 !important;}
.admincal .resBar.proposed {background-color: #0066ff !important;}
.admincal .resBar.pending {background-color: #ffff00 !important;}
.admincal .resBar.request {background-color: #ff8800 !important;}
.admincal .resBar.draft {background-color: #ffff00 !important;}
.admincal .resBar.declined {background-color: #aaaaaa !important;}
.admincal .resBar.cancelled {background-color: #888888 !important;}
.admincal .resBar.rejected {background-color: #666666 !important;}
.admincal .resBar.expired {background-color: #444444 !important;}

.admincal .resBar.invoice {background-color: #888888}
.admincal .resBar.draft {background-color: #ffff00 !important;}
.admincal .resBar.open {background-color: #0066ff !important;}
.admincal .resBar.sent {background-color: #cc0000 !important;}
.admincal .resBar.complete {background-color: #008800 !important;}

.admincal .resBar {opacity: 0.7 !important}

/* new elastic model */

.cal_width {width:<?= ($cal_params->canvas_w + $cal_params->title_w) ?>px;}
.canvas_h {width:<?=$cal_params->canvas_h ?>px;}
.canvas_w, cal_grid_width, .cal_canvas_width {width:<?=$cal_params->canvas_w ?>px;}
.grid_w {width:  <?=$cal_params->grid_w ?>px}
.cell_w {width:  <?=$cal_params->cell_w ?>px !important}
.cell_h {height: <?=$cal_params->cell_h ?>px}
.title_margin {margin-left: <?=$cal_params->title_w ?>px;}


.cell_w_font {
  font-size: <?= $cal_params->day_font_size ?>px;
}

#admincal_full {
  padding: 5px 5px 5px 10px;
}

#calendar. .admincal {
  text-align: left;
}

.admincal .cal-container {
  position: relative;  
  display:inline-block;
}

.admincal .cal-apts {
  position: absolute;
  _z-index: 1;
  bottom: 0px;
  width: 100%;
}

.admincal .cal-apts UL {
  margin: 0 !important;
  padding: 0 !important;  
}

.admincal .cal-apt {
  position: relative;
}

.admincal .cal-apts .apt-title {
}

.admincal .cal-guide {
  z-index: 3;
  position:relative;
  margin-left: <?=$cal_params->title_w ?>px;
  box-sizing:border-box;-moz-box-sizing:border-box;-webkit-box-sizing:border-box;
}

.admincal .cal-guide-day {
  display:block;
  bottom: 0px;
  right: 0px;
}


.admincal .cal-guide .guide-box {
  height: <?=$cal_params->canvas_h ?>px;
  border-left: 1px solid grey;
  box-sizing:border-box;-moz-box-sizing:border-box;-webkit-box-sizing:border-box;
}

/*
.cal-guide.cal_day {width:  <?=$cal_params->cell_w ?>px !important;box-sizing:border-box;-moz-box-sizing:border-box;-webkit-box-sizing:border-box;}
.cal-guide.cal_day .guide_title {width:  <?=$cal_params->cell_w ?>px !important;box-sizing:border-box;-moz-box-sizing:border-box;-webkit-box-sizing:border-box;}
.cal-guide.cal_day .guide-box {width:  <?=$cal_params->cell_w ?>px !important;box-sizing:border-box;-moz-box-sizing:border-box;-webkit-box-sizing:border-box;border:none !important}
*/

.admincal .canvas_h {
  height: <?=$cal_params->canvas_h ?>px;
}

.admincal .cal-canvas {
  position: absolute;
  bottom: 0px;
  right: 0px;
  height: <?=$cal_params->canvas_h ?>px;
  left: <?=$cal_params->title_w ?>px;
  _z-index: 2;
  -webkit-box-shadow: 5px 6px 5px rgba(50, 50, 50, 0.37);
  -moz-box-shadow:    5px 6px 5px rgba(50, 50, 50, 0.37);
  box-shadow:         5px 6px 5px rgba(50, 50, 50, 0.37);
}

#cal_search {
  height: 20px;
}

#cal_menu .close-button {
  position: absolute;
  top: 4px;
  right: 4px;
  cursor:pointer;
}

#cal_menu {
  background: #FFFFAA; 
  border: 1px solid #FFAD33; 	
  border-radius: 5px 5px; -moz-border-radius: 5px; -webkit-border-radius: 5px; 
  box-shadow: 5px 5px 5px rgba(0, 0, 0, 0.5); -webkit-box-shadow: 5px 5px rgba(0, 0, 0, 0.5); -moz-box-shadow: 5px 5px rgba(0, 0, 0, 0.5);
  font-family: Calibri, Tahoma, Geneva, sans-serif;
  position: absolute; 
  left: 1em; 
  top: 2em; 
  margin-left: 0; 
  z-index: 99 !important;
  max-width: 350px;
  text-align:center;
  padding:10px 10px 10px 10px; 
}
#cal_menu .memo_field {background-color: #fff;margin-top: 4px;padding:5px;border-radius: 5px}


.menu_container {
  margin: 10px 0;
  height: 40px;
  max-width: 1024px;
}

.cal_title {
}


.admincal .cal-guide {
  text-align: right;
}

.admincal .cal-guide .cal-months UL LI {
  border: 1px solid #ccc;box-sizing:border-box;-moz-box-sizing:border-box;-webkit-box-sizing:border-box;overflow:hidden;
}

.admincal .cal-guide UL {
  text-align: left;
  list-style-type: none;
  padding: 0;
  margin: 0;
}

.admincal .cal-guide LI {
  float: left;
  text-align: center;
  white-space: nowrap;  
}

.menu_container { width: 100%}

/* below prevents wrapping */
.menu_container, .admincal .cal-guide > UL, .admincal .cal-months > UL, .admincal .gmenu > UL, .admincal .cal_menu > UL {
  white-space: nowrap;  
  margin: 0;
}

.admincal .cal-months > UL > LI, .admincal .cal-guide > UL > LI, .admincal .gmenu > UL > LI, .admincal .cal_menu > UL > LI  {
  display: table-cell;
  white-space: nowrap;  
  float: none;
  margin: 0; 
}


/* Days */

.admincal .cal-guide, .admincal .cal-guide A {
}


.admincal .cal-guide UL LI {
  margin: 0 !important;
}


.admincal .cal-entries .hover {
  border: 1px dotted black;
  margin: -1px 0 0 -1px;
  z-index: 30 !important;
}


.admincal .cal-apts LI.hover A {
  background-color: #cfc;
  color: #080;
}  

.admincal .cal-apts LI.hover {
  background-color: #cfc;
  color: #080;
}  

.admincal .hover {
  background-color: #cfc;
}  

/*
.admincal .cal-guide UL LI:hover {
}
*/

.admincal .cal-guide UL LI.cal_day {
  white-space: nowrap;  
  margin: 0 !important;
}

.admincal .cal-guide UL LI.today, .admincal .cal-guide UL LI.today A {
  background-color: #fdd;
  color:#f00;
  font-weight: bold;
  margin: 0 !important;
}

.admincal .cal-guide UL LI.weekend, .admincal .cal-guide UL LI.weekend A {
  color:#800;
  margin: 0 !important;
}


/* help/stats/changes links */
.admincal .cal-links {
  display: inline;
  white-space: nowrap;  
  position: relative;
  right: 0px;
}
.admincal .cal-links UL {
  list-style-type: none;
}

.admincal .cal-links UL LI {
  display: inline;
  white-space: nowrap;  
}

.admincal .cal-links A {
  font-size: 0.8em;
}

/* month links */
.admincal UL.month-links  {
  list-style-type: none;
  font-size: 1em;
}

.admincal UL.month-links  LI {
  display: inline;
  white-space: nowrap;  
}

.admincal UL.month-links  LI A {
  font-size: 0.8em;
}

/* nav */
.admincal .cal_nav {
}

/* apts */
.admincal .cal-apts, .admincal .cal-apts A {
  font-size: 0.9em;
}

.admincal .cal-apts {
  _border: 2px solid green;
}

.admincal .cal-apts UL {
  list-style-type: none;        
  text-align:right;
  _overflow: hidden;
}

.admincal .cal-apts UL LI {
  height:<?=$cal_params->cell_h ?>px;
  width: 100%;
  white-space: nowrap; 
  text-align: left;
  padding-left: 10px;
  clear: both;
  _border: 1px solid red;
}

.admincal .cal-apts UL LI DIV.calendar-property-title {
  width: 140px !important;
  text-overflow: ellipsis;
  overflow: hidden;
}

.admincal UL LI.odd {
  background-color:#fff;
}

.admincal UL LI.even {
  background-color:#ddd;
}

.admincal .cal-apts UL LI.active {
  background-color:#cfc !important;
}

/* End New June 2013 */


.admincal .priceBar, .admincal .blockBar  {
  text-align:center;
  vertical-align:top;
  font-family: verdana;
  white-space: nowrap;
  font-size: 0.6em;
  position : absolute;
  z-index: 2;
  _padding: 0 .2em;
}

.admincal .priceBar {
  background-color: #7ACC52;
  opacity: 0.5;
}
.admincal .blockBar  {
  background-color: #FF8888;
}

.admincal .priceBar .price-text, .admincal .blockBar .price-text, .admincal .blockBar .res-text {
  font-weight: bold;
  margin: 0px auto;
  overflow:hidden;
}

.admincal .priceBar .price-text, .admincal .priceBar .res-text {
  margin-top: 4px !important;
  color: #000;
}
.admincal .blockBar .price-text {
  color: #FFF;
}


.admincal .resBar {
  cursor: pointer;
  color: #fff;
  text-align:left;
  vertical-align:top;
  font-family: verdana;
  white-space: nowrap;
  font-size: 0.6em;
  position : absolute;
  height:<?= $cal_params->bar_h ?>px;
  z-index: 3;
  padding: 0 .2em;
}

.admincal .resBar DIV.res-text{
  overflow: hidden;
  display: inline-block;
}

.admincal .resBar .tooltip SPAN {
  z-index: 99 !important;
  _opacity: 1;
}

.admincal .resBar.drag:actvive, .admincal .resBar.drag_ew:active, .admincal .resBar.drag_ns:active {
  cursor: move !important;
}

.admincal  .selected {
  position:absolute;
  border: 1px solid black;
  margin: -1px;
  z-index: 30 !important;
  
}
.admincal .selected-disabled-filters {
  filter: brightness(250%);-webkit-filter: brightness(250%);
  filter: saturate(250%);-webkit-filter: saturate(250%);
  -webkit-filter: drop-shadow(6px 6px 7px rgba(0,0,0,0.5));
  filter: drop-shadow(6px 6px 7px rgba(0,0,0,0.5));
  filter: url(shadow.svg#drop-shadow);
  -ms-filter: "progid:DXImageTransform.Microsoft.Dropshadow(OffX=6, OffY=6,Color='#444')";
  filter: "progid:DXImageTransform.Microsoft.Dropshadow(OffX=6, OffY=6, Color='#444')"; 
}  

.admincal .ui-draggable-overlapping {
  filter:alpha(opacity=50);-moz-opacity:0.5;-khtml-opacity: 0.5;opacity: 0.5;
}

.admincal #new_entry {
  background-color: #f00;
  border:1px dotted black;
  z-index: 40;
}

#overlay_content  {
  font-size: 1.0em;
}

#overlay_content .dialog-total {
  font-size: 1.2em;
}

#overlay_content .error, #overlay_content A.error {
  color:#f00;
  font-weight: bold;
}

#overlay_content .rate_fieldset .currency_total {
  width: 150px; 
  text-align:right;
}

#overlay_content .fieldset_left .fieldset_div label {width: 70px !important;}

.overlay H2  {
  text-align: center;
  font-size: 1.5em;
  border-bottom: 1px solid #ddd;
}


.overlay form  {  /* set width in form, not fieldset (still takes up more room w/ fieldset width */
  font-size:0.8em;
  margin: 0;
  padding: 0;
  min-width: 300px;
  max-width: 480px;
  width: 460px;
}

.overlay label { 
  width: 60px !important;
}

.overlay {
  display: block;
  font-family:sans-serif;font-weight:bold;
  font-size: 10px;
  margin-right: 20px; 
}

.overlay INPUT {background-color: #ffe; color: #111; margin-right: 4px; font-size: 10px;}

#cal_messages {
  left: 300px;
  top: 50px;
  position: absolute;
  font-weight:bold;
  padding: 4px;
  border: 2px solid black;
  background-color: white;
  z-index: 39 !important;  
}


.admincal .resBar .afr-tooltip {
  border: none !important;
  margin:0 !important;
}

/* small tooltip icon */
.admincal .resBar .afr-tooltip > I {
  font-size: 1em !important;
  margin-right: 2px;
  position: relative;
  color: #00f;
  top: -4px;
}

/* bigger tooltip text */
.admincal .resBar .afr-tooltip SPAN {
  font-size: 2em !important;
}

/*
A.cal-alert  {
  background-color: #44f;
  font-size: 0.8em;
  color: #FFF !important;
  font-weigth: bold;
  border-radius: 3px;
  margin-right:4px;
  padding: 0 2px;
  top: 0px;
  left: 0px;
}
*/


/* big icon */
.cal-tip DIV I {
}

.cal-tip DIV SPAN {
  padding-left: 20px;
}

.cal-tip {
  left: 0px !important;
  top: -50px !important;
  position: absolute;
  display:none;
}



.msg {
  color: #000;
  text-align: left;
  font-size: 1em;
  z-index: 99 !important; 
  font-family: Calibri, Tahoma, Geneva, sans-serif;  
}

.msg DIV {
  margin-left: 0;   
  border-radius: 5px 5px; -moz-border-radius: 5px; -webkit-border-radius: 5px; 
  box-shadow: 5px 5px 5px rgba(0, 0, 0, 0.1); -webkit-box-shadow: 5px 5px rgba(0, 0, 0, 0.1); -moz-box-shadow: 5px 5px rgba(0, 0, 0, 0.1);
}
.msg DIV.custom {
  left: 1em;
}

.msg .custom I { 
  border: 0; margin: -15px 0 0 -50px;
  float: left; position: absolute;
  z-index: 99 !important;
}
.msg .classic   { padding: 0.8em 1em; background: #FFFFAA; border: 1px solid #FFAD33; }
.msg .custom    { padding: 0.5em 0.8em 0.8em 2em; }
.msg .help      { background-color: #9FDAEE; border: 1px solid #2BB0D7;}
.msg .info      { background-color: #9FDAEE; border: 1px solid #2BB0D7;}
.msg .success   { background-color: #F0FFF0; border: 1px solid #3D8B37;}
.msg .critical, .msg .error { background-color: #FFCCAA; border: 1px solid #FF3334;}
.msg .warning, .msg .warn   { background-color: #FFFFAA; border: 1px solid #FFAD33;}


/* form - dialog */
#overlay_content {
  minwidth: 450px;
}

#cal_dialog {
  min-width: 570px;
  _width:100%;
}
.cal-form label {
  white-space: nowrap;
}  

.cal-form label.error {
  display: inline !important;
}  

.cal-form input, .uneditable-input {
  font-size: 12pt;
  line-height: 100%;
  height: 24px;
  font-weight: 300;
  color: #444444;
}

.cal-form input.button {
  height: 20px !important;
  padding-left: 10px;
  padding-right: 10px;
}
.cal-form select, .cal-form input.button {
  height: 28px;
  min-width: 50px;
  font-size: 14px;
  font-weight; bold;
}
.cal-form input, .cal-form textarea, .uneditable-input {
  box-shadow: 0px 1px 1px rgba(0, 0, 0, 0.075) inset;
}

.cal-form input, .cal-form textarea, .uneditable-input {
  margin-left: 0px;
}

.cal-form input:focus, .cal-form textarea:focus {
  border-color: rgba(82, 168, 236, 0.8);
  box-shadow: 0px 1px 1px rgba(0, 0, 0, 0.075) inset, 0px 0px 8px rgba(82, 168, 236, 0.6);
  outline-width: 0px;
  outline-style: none;
  outline-color: -moz-use-text-color;
}

.cal-form input[type="checkbox"] {
  width: 20px !important;
  margin-left: 8px;
  margin-right: 8px;
}
  
.cal-form input[type="button"], .cal-form input[type="submit"] {
  height: 20px;
}

UL.cal_menu LI A SPAN.ui-state-active
{
  border:none !important;
}

UL.cal_menu LI A:hover {
}

.weekend_rate {color:#a00}


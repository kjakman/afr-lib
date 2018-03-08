<? include_once('../include/set_env.inc');?>
<? include_once("helpers_calendar.inc");?>
<?php 
  header("Content-type: text/css; charset: utf-8");
  $cal_params = $_SESSION['cal_params'];
?>

body {font: 12px/1.4 sans-serif;z-index: -10}

.admincal {z-index: -10;border: 0px solid green;}

.admincal A { font-family:arial;font-size:<?=$cal_params->font_size ?>px;font-weight:bold;font-style: normal; text-decoration: none; }
.admincal A:link { color: #008; text-decoration: none; }
.admincal A:visited { color: #008; text-decoration: none;}
.admincal A:hover { color: #00a; text-decoration: underline; }
.admincal A:active { color: #00a; text-decoration: underline; }
.admincal B {font-family:arial;font-size:<?=$cal_params->font_size ?>px;}

#grids {
  position: relative;
}

#calendar_grid-1 {
  _border: 2px solid blue;
}

#grids_containment {
  position: absolute;
  z-index: -1;
  width:<?=$cal_params->canvas_max_w?>px;
  height:<?=$cal_params->canvas_h?>px;
  top:0px;
  left:<?=$cal_params->title_w?>px;
  _border: 2px solid pink;
}

#cal_controls {position:relative;height:40px;left:<?=$cal_params->title_w?>px;z-index: 1} 
#cal_mode_button    {float:left;padding-top:8px;margin-right:2px;} 
#cal_zoom           {float:left;margin-right:20px;} 
#cal_choosers_left  {float:left;margin-right:20px;text-align:center;} 
#cal_choosers_right {float:left;margin-right:20px;height:30px;width: 400px;z-index: 2;}

.cal_top {position:relative;width:<?= ($cal_params->canvas_w + $cal_params->title_w + $cal_params->stats_w) ?>px; } 
.cal_sortbar {position:absolute;bottom:2px;left:0px;width:<?=$cal_params->title_w ?>px; text-align:right;vertical-align:bottom}  
.cal_headers {position:relative;width:<?=$cal_params->canvas_w ?>px;left:<?=$cal_params->title_w ?>px;}  
.cal_footers {position:relative;width:<?=$cal_params->canvas_w ?>px;left:<?=$cal_params->title_w ?>px;text-align:center;margin-top:<?=$cal_params->cell_h ?>px;}
#cal_filters {}
.cal_stats          {font-size: <?= $cal_params->med_font_size ?>px;text-align:right;position:absolute;top:-<?=$cal_params->cell_h?>px; left:<?= ($cal_params->canvas_max_w + $cal_params->title_w) ?>px;}
.cal_stats .stat    {position:absolute;height:<?=$cal_params->cell_h ?>px;width:<?=$cal_params->w2 ?>px;background-color:#fff;}
.cal_stats .statAlt {position:absolute;height:<?=$cal_params->cell_h ?>px;width:<?=$cal_params->w2 ?>px;background-color:#ddd;}

#changes {display:none}

.memo_field {background-color: #fff;border: 1px dotted black;margin-top: 4px}

.grid_links {}
.grid_link {position:absolute;text-align:center;width: <?=$cal_params->cell_w ?>px;z-index:20;}
.grid_link A  {color: #080 !important}

.cal_entries {}

.cal_filter {display:inline;margin:0;padding:0;}

.admincal .canvas {
    top: 0px;
    position: absolute;
    background-color: #efefef;
    _border: 1px solid red;
}

.admincal .today_marker {
  background-color: #faa;
  z-index: 0;
  position: absolute;
} 

.admincal .avail_search {
  background-color: #afa;
  z-index: 0;
  position: absolute;
} 

.admincal .canvas_containment {
    position: absolute;
    z-index: -1;
}

.admincal .grid {
  position:absolute; top:0px;left:0px; 
  border: 1px solid grey;
  z-index: 1;
}
 
.admincal .apts {
    position: relative;
    height: <?=$cal_params->canvas_h ?>px;
    text-align: right;
}

#menu{
    position: relative;
    font-family:arial;
    font-size:<?=$cal_params->font_size ?>px;
    position: absolute;
    display: none;
    background-color: #F9ECCD;
    width: <?= (220*$cal_params->zoom) ?>px;
    color: #111;
    z-index: 2000;
    text-align:center;
    padding:10px 5px 10px 5px;
   	border: 1px solid #FB7A31;
}

.admincal #menu_payment{
    background-color: #F9ECCD;
    margin-left: <?= (15*$cal_params->zoom) ?>px;
    width: <?= (160*$cal_params->zoom) ?>px;
    color: #111;
    text-align:right;
}

.admincal .apt {
  text-align:right;position:absolute;height:<?=$cal_params->cell_h ?>px;color:#000;background-color:#fff;font-size:<?= $cal_params->var_font_size ?>px;
}

.admincal .aptAlt {
  text-align:right;position:absolute;height:<?=$cal_params->cell_h ?>px;color:#000;background-color:#ddd;font-size:<?= $cal_params->var_font_size ?>px;
}

.admincal .resBar {
  color: #fff;
  text-align:left;
  vertical-align:top;
  padding-left: 2px;
  font-family: verdana;
  white-space: nowrap;
  font-size:<?= $cal_params->small_font_size ?>px;
  position : absolute;
  height:<?= $cal_params->bar_h ?>px;
  padding: 0 0 0.2em 0

}

.admincal  .selected {
  position:absolute;
  border: 3px solid black;
  z-index: 300 !important;
  margin: -3px 0 0 -3px;
}

.admincal .ui-draggable-overlapping {
  filter:alpha(opacity=50);-moz-opacity:0.5;-khtml-opacity: 0.5;opacity: 0.5;
}

.admincal  .hover {
  position:absolute;
  border: 1px dotted black;
  margin: -1px 0 0 -1px;
  z-index: 300 !important;
}

.admincal .auth_view {
  cursor: pointer;
}

.admincal .event {
  z-index: 301;
}

.admincal .resMarker {
  top:0px;left:0px;
}

.admincal .dragclass {
  position : absolute;
  cursor : move;
  height:<?= ($cal_params->cell_h/2) ?>px;z-index:300 !important;color:#000;
}
.admincal .dragclass_y {
  position : absolute;
  cursor : n-resize;
  height:<?= ($cal_params->cell_h/2) ?>px;z-index:300 !important;color:#000;
}

.admincal .dragclass_x {
  position : absolute;
  cursor : w-resize;
  height:<?= ($cal_params->cell_h/2) ?>px;z-index:300 !important;color:#000;
}

.admincal .resizeclass_x {
  position : absolute;
  cursor : w-resize;
  height:<?= ($cal_params->cell_h/2) ?>px;z-index:300 !important;;color:#000;
}

.admincal .res_noauth {
  position:absolute;height:<?=$cal_params->cell_h ?>px;z-index:300 !important;;color:#000;cursor:pointer;
}

.admincal .weeks {
    position: relative;
    height: <?=$cal_params->cell_h ?>px;
    width: <?=$cal_params->canvas_max_w ?>px;
    font-size: <?= $cal_params->day_font_size ?>px;
    text-align:center;	
    padding: 0 4px 4px 0;
}
.admincal .week {
  position:absolute;height:<?=$cal_params->cell_h ?>px;top:0px;width:<?=$cal_params->cell_w ?>px;color:#000;background-color:#ccc;
}

.admincal .weekAlt {
  position:absolute;height:<?=$cal_params->cell_h ?>px;top:0px;width:<?=$cal_params->cell_w ?>px;color:#000;background-color:#aaa;
}


.admincal .months {
    position: relative;
    height: <?=$cal_params->cell_h ?>px;
    width: <?=$cal_params->canvas_max_w ?>px;
    font-size: <?= $cal_params->day_font_size ?>px;
    text-align:center;	
    font-weight:bold;	
    padding: 0 4px 4px 0;
}
.admincal .month {
  position:absolute;top:0px;width:<?=$cal_params->cell_w ?>px;color:#000;background-color:#ccc;height: <?=$cal_params->cell_h ?>px;
}

.admincal .monthAlt {
  position:absolute;top:0px;width:<?=$cal_params->cell_w ?>px;color:#000;background-color:#aaa;height: <?=$cal_params->cell_h ?>px;
}

.admincal .days {
    position: relative;
    font-size: <?= $cal_params->day_font_size ?>px;
    height: <?= $cal_params->day_font_size ?>px;
    width: <?=$cal_params->canvas_w ?>px;
    padding: 0 4px 4px 0;
}

.admincal .days A {
    font-size: <?= $cal_params->day_font_size ?>px;
}

.admincal .days B {
    font-size: <?= $cal_params->day_font_size ?>px;
}

.admincal .day {
  position:absolute;top:0px;color:#000;
  height:<?=$cal_params->cell_w ?>px;width:<?=$cal_params->cell_w ?>px;
}

.admincal .dayWeekend {
  position:absolute;top:0px;color:#a00;
  height:<?=$cal_params->cell_w ?>px;width:<?=$cal_params->cell_w ?>px;
}

.admincal .dayToday{
  position:absolute;
  height:<?=$cal_params->cell_w ?>px;width:<?=$cal_params->cell_w ?>px;
  top:0px;color:#f00;
}

.admincal .dayToday A {
  color: #F00;
}

img.today_marker{
	width: 70%;
}

.overlay form  {  /* set width in form, not fieldset (still takes up more room w/ fieldset width */
  font-size:10px;
  margin: 0;
  padding: 0;
  min-width: 300px;
  max-width: 400px;
  width: 340px;
}

.overlay label { 
  width: 60px !important;
}

.overlay {
  display: none;
  font-family:sans-serif;font-weight:bold;
  font-size: 10px;
  margin-right: 20px; 
}

.overlay INPUT {background-color: #ffe; color: #111; margin-right: 4px; font-size: 10px;}

.dragclass {position : relative; cursor : move;}

#cal_messages {
  left: 300px;
  top: 50px;
  position: absolute;
  font-weight:bold;
  padding: 4px;
  border: 2px solid black;
  background-color: white;
  z-index: 399 !important;  
}

#cal_messages .message {
}


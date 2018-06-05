<?php 
require_once('set_env.inc');
header('Content-Type: application/x-javascript; charset=utf-8');
$output = plugin_javascript($_GET);
echo($output);  

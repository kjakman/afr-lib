<?php

$db_live = "aptdb_v14";
$db_dev =  "aptdb_v16";
$db_user = "apt";
$db_password = "killa113";

$db_local_host = "localhost";
//$db_cloud_host = "s3db";
$db_cloud_host = "50.56.58.218"; // louis is "cloud host" for now.
$db_master_host = "50.56.58.218"; // louis is master
$db_slave_host = "69.59.146.253"; // diz is slave

$cloud_db = false;
$remote_db = false;
$server_ip = $_SERVER['SERVER_ADDR'];
//echo("Server ip = $server_ip master=$db_master_host");
if($remote_db && $server_ip != $db_master_host) { // app server is on remote server, connect to master DB
  $db_host = $cloud_db ? $db_cloud_host : $db_master_host;
} else {
  $db_host = $cloud_db ? $db_cloud_host : $db_local_host;
}

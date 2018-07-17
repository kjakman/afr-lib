<?php

  //header('Content-type:application/json;charset=utf-8');

 $obj_type= $_REQUEST['type'];
 $id = $_REQUEST['row_id'];
 $obj_id= $_REQUEST['id'];
 $value2= $_REQUEST['value2'];
 $value3= $_REQUEST['obj_id'];

 $obj_datas = db_getAll("select * from artwork where id='$id'");

foreach($obj_datas as $obj_data){

$obj_id='';

$obj_data->title = $obj_data->title.'_'.$value2;

 $result= add_object($obj_type, $obj_data);
$value3=$value2+1;
$clone_data =array("obj_data" => $result,"obj_id" => $obj_id,
                "values" => $value3);

 }


 echo(json_encode($clone_data));
 exit;



		
	
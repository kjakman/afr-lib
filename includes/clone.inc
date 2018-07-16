<?php

  //header('Content-type:application/json;charset=utf-8');

 $id = $_REQUEST['row_id'];
 $obj_id= $_REQUEST['id'];
 $value2= $_REQUEST['value2'];
 $value3= $_REQUEST['obj_id'];



 $obj_datas = db_getAll("select * from artwork where id='$id'");

foreach($obj_datas as $obj_data)
 {
 $obj_id='';
$obj_data->title = $obj_data->title.'_'.$value2;
$obj_data->id='';
$value2=$value2+1;

$clone_data =array("obj_data" => $obj_data,"obj_id" => $obj_id,
                "values" => $value2);

echo json_encode($clone_data);
	exit();                
	break;

 }






		
	
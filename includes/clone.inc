<?php
	//header('Content-type:application/json;charset=utf-8');

		$obj_type= $_REQUEST['type'];
		$id = $_REQUEST['row_id'];
		$value2= $_REQUEST['value2'];
		$cloneval= $_REQUEST['col'];
		$obj_col= $_REQUEST['cloneval'];

		$obj_datas = db_getAll("select * from artwork where id='$id'");


		foreach($obj_datas as $obj_data){

			if($id != ''){

					$obj_id = $id;

					
	    			$obj_data->clone_id= $obj_data->clone_id + $cloneval;

	     			print_r($obj_data);

					$results= update_object($obj_type, $obj_data, $obj_id);

					print_r($results);

					$clone_id = $data;

					

				}
			}


		foreach($obj_datas as $obj_data){

			$obj_id='';

			$obj_data->title = $obj_data->title.'_'.$clone_id;

			$result= add_object($obj_type, $obj_data);

			$clone_data =array("obj_data" => $result,"obj_id" => $obj_id,"values" => $clone_id);
		}

	echo(json_encode($clone_data));
	exit;

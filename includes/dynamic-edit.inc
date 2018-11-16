<?php
	
      
      //echo "<pre>"; print_r($obj_data);
      $obj_type= $_REQUEST['type'];
      $obj_id= $_REQUEST['id'];
      $obj_col= $_REQUEST['col'];
      $data= $_REQUEST['data'];
      $obj_data= array('obj_type'=>$obj_type,'obj_id'=>$obj_id,$obj_col=>$data);
      echo "<pre>"; print_r($obj_data);
     
      $result= update_object($obj_type, $obj_data, $obj_id);
       echo "<pre>"; print_r($result);
      die("success");

?>
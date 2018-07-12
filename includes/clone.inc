<?php
echo "<pre>"; print_r($_REQUEST['row_id']);
  //header('Content-type:application/json;charset=utf-8');

 $id = $_REQUEST['row_id'];

 $name = db_getAll("select * from artwork where id='$id'");

print_r($name->title);
 if($name !== '')
 {

 $title=$name->title;

 $qs = "INSERT INTO artwork (title) VALUES ($title)";
        //echo($add."<br>".$qs."<br>");
        db_query($qs);

 }
 
 return $name;
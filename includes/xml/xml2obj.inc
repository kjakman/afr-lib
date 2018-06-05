<?php

function startElement($parser, $name, $attrs) {
   global $obj;

   // If var already defined, make array
   eval('$test=isset('.$obj->tree.'->'.$name.');');
   if ($test) {
     eval('$tmp='.$obj->tree.'->'.$name.';');
     eval('$arr=is_array('.$obj->tree.'->'.$name.');');
     if (!$arr) {
       eval('unset('.$obj->tree.'->'.$name.');');
       eval($obj->tree.'->'.$name.'[0]=$tmp;');
       $cnt = 1;
     }
     else {
       eval('$cnt=count('.$obj->tree.'->'.$name.');');
     }

     $obj->tree .= '->'.$name."[$cnt]";
   }
   else {
     if($obj->debug) echo("&lt;$name&gt;");
     $obj->tree .= '->'.$name;
   }
   if (count($attrs)) {
       eval($obj->tree.'->attr=$attrs;');
   }
}

function endElement($parser, $name) {
   global $obj;
   // Strip off last ->
   if($obj->debug) echo("&lt;/$name&gt;<br>");
   for($a=strlen($obj->tree);$a>0;$a--) {
       if (substr($obj->tree, $a, 2) == '->') {
           $sub = substr($obj->tree, 0, $a);
           $obj->tree = $sub;
           break;
       }
   }
}

function characterData($parser, $data)
{
   global $obj;
   $data = trim(addslashes($data));
   if($obj->debug) echo($data);
   eval($obj->tree."->data.='".$data."';");
   //eval($obj->tree."='".trim($data)."';");
}

function xml2obj($filename) {
    
   global $obj;
   $obj->tree = '$obj->xml';
   $obj->xml = '';
   $obj->debug = 0;
   $xml_parser = xml_parser_create();
   xml_set_element_handler($xml_parser, "startElement", "endElement");
   xml_set_character_data_handler($xml_parser, "characterData");
   
   if (!($fp = fopen($filename, "r"))) {
      die("could not open XML input");
   }

   while ($data = fread($fp, 4096)) {
      if (!xml_parse($xml_parser, $data, feof($fp))) {
          die(sprintf("XML error: %s at line %d",
                      xml_error_string(xml_get_error_code($xml_parser)),
                      xml_get_current_line_number($xml_parser)));
      }
   }
   xml_parser_free($xml_parser);
   //print_r($obj->xml);
   //printf("Ran xml2obj<p>");

   return $obj->xml;
}

function xml2obj2($data) {

   //$filename = 'portfolio.xml';
   global $obj;
   $obj->tree = '$obj->xml';
   $obj->xml = '';

   $xml_parser = xml_parser_create();
   xml_set_element_handler($xml_parser, "startElement", "endElement");
   xml_set_character_data_handler($xml_parser, "characterData");
   if (!xml_parse($xml_parser, $data, feof($fp))) {
        die(sprintf("XML error: %s at line %d",
                    xml_error_string(xml_get_error_code($xml_parser)),
                    xml_get_current_line_number($xml_parser)));
   }
   xml_parser_free($xml_parser);

   return $obj->xml;
}


?>

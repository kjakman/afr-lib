<?php
namespace EST;

/** 
  Helper functions for Elastic Tools 
  
  TODO: Don't add keyword to text if inside fields of keyword
  type suggest: removed the support for the payload options
  add missing field types
  
  paste output of "GET _cat/indices?h=i" here... (generate GET [index] list)
  paste GET [index] list into kibana, and copy/paste output here
    
  handle arrays
  
  deal with missing analyzers
  
  https://www.elastic.co/guide/en/elasticsearch/reference/5.5/breaking_50_mapping_changes.html
  
  deal with _timestamp and _ttl
  
  Get rid of:
  "_all": {
    "enabled": false
  }


*/

class ElasticTools {
    private $dsl;
    private $settings;
    private $aliases;
    private $mappings;
    private $index;
    private $type;
    public function __construct()
    {
    }
    
    /** takes one or more outputs from GET [index], optionally including the GET command */
    public function convertIndices($dsls, $options = [])
    {
        $output = '';
        /** split up possible multiple dsls */
        $pattern = '#^\{.*^\}#smU';
        preg_match_all($pattern, $dsls, $out, PREG_PATTERN_ORDER);
        $dsls = $out[0];
        if(!count($dsls)) throw new \Exception("Could not find any JSON blocks in input");
        $i = 1;
        foreach($dsls as $dsl) {
          $this->dsl = $dsl;
          $output .= $this->convertIndex($dsl, $options);
          print_log("{$i} OK: Index: {$this->index} Type: {$this->type}\n", 'elastictools', LOG_LEVEL_TEST);
          $i++;
        }
        return $output;
    }
    
    /** takes outputs from single GET [index] (dsl only, not the GET line) */
    private function convertIndex($index_dsl, $options = [])
    {
              
        
        /** decode input */
        $dslArray = json_decode($index_dsl, true);
        if(!$dslArray) throw new \Exception("The input is not valid JSON");
        
        /** do processing */
        
        if(count($dslArray) > 1) {
          $keys = array_keys($dslArray);
          $keys_str = implode(', ', $keys);
          throw new \Exception("Not a valid mapping - found more than one index in the json: {$keys_str}");
        }
              
        reset($dslArray);
        $indexName = $this->index = key($dslArray);
        if($indexName[0] == '.') return "# Skipping system index {$indexName}".PHP_EOL;

        $this->settings = $settings = $dslArray[$indexName]['settings']['index'];
        if(!$settings || !is_array($settings) || !count($settings)) throw new \Exception("Couldn't find settings in {$indexName}");
        //dump($settings);

        $this->mappings = $mappings = $dslArray[$indexName]['mappings'];
        if(!$mappings || !is_array($mappings) || !count($mappings)) throw new \Exception("Couldn't find mappings in {$indexName}");

        $this->aliases = $aliases = $dslArray[$indexName]['aliases'];
        if(count($aliases)) { /** this is because we want {} and not [] */
          foreach($aliases as $name => $alias) {
            if(is_array($alias) && !count($alias)) {
              $aliases[$name] = new \stdClass;
            }
          }
        }
        //dump($aliases);
        
        reset($mappings);
        //print_log("Size of incoming mapping:".count($mappings), 'elastictools', LOG_LEVEL_TEST);
        //print_log("Keys of incoming mapping:".dump(array_keys($mappings), true), 'elastictools', LOG_LEVEL_TEST);

        $typeName = $this->type = key($mappings);
        if(!$typeName) throw new \Exception("Could not find a valid type in {$indexName}");
        if($typeName == '_default_') {
          /** todo: handle */
          return '';
        }
        
        if($mappings[$typeName]['_all']) {
          print_log("Enabled: ".$mappings[$typeName]['_all']['enabled'], 'elastictools', LOG_LEVEL_TEST);
          if(!$mappings[$typeName]['_all']['enabled']) {
            unset($mappings[$typeName]['_all']);
          }
        }
            
        
        $propArray = $mappings[$typeName]['properties'];      
        if(!$propArray || !count($propArray)) throw new \Exception("Couldn't find properties in {$indexName}->mappings->{$typeName}");
        unset($mappings[$typeName]['properties']);
        
        $numFields = count($propArray);
                                                    
        //print_log("len={$obj_len} index={$indexName} type={$typeName}", 'elastictools', LOG_LEVEL_TEST);
        
        /** convert properties to 6.x */
        $newPropArray = [];
        foreach($propArray as $field => $fields) {
          $newPropArray[$field] = $this->convertField($field, $fields);
          //print_log("Property Field: {$field}", 'elastictools', LOG_LEVEL_TEST);
        }
        $mappings[$typeName]['properties'] = $newPropArray;

        $count = count($newPropArray);
        print_log("Mapping for type={$typeName} with {$count} fields", 'elastictools', LOG_LEVEL_TEST);
        
        /** encode result */
        //$new_mapping = json_encode($dslArray, JSON_PRETTY_PRINT);
        
        /** Remove search and other unsupported members of mapping */
        foreach($mappings as $k => $v) {
          if($k != $typeName && $k[0] != '_') unset($mappings[$k]);
        }
        
        //$newSettings = ['number_of_shards' => $settings['index']['number_of_shards'] ?: 1, 'number_of_replicas' => $settings['index']['number_of_replicas'] ?: 0];
        
        $newDSL = [];
        
        $unset = ['creation_date', 'uuid', 'version', 'mappings'];
        foreach($unset as $fld) unset($settings[$fld]);
        
        $newDSL['settings'] = $settings; /** do we want to copy all settings ? */
        if(count($aliases)) $newDSL['aliases'] = $aliases;
        $newDSL['mappings'] = $mappings;
        
        //print_log("Size of outgoing mapping:".count($mappings), 'elastictools', LOG_LEVEL_TEST);
        //print_log("Keys of outgoing mapping:".dump(array_keys($mappings), true), 'elastictools', LOG_LEVEL_TEST);
        
        
        $output_dsl = "PUT {$indexName}\n".json_encode($newDSL, JSON_PRETTY_PRINT);
        
        return $output_dsl.PHP_EOL;
    }
                
    private function convertField($name, $field, $level=0) {
        //return $field; // for testing - no conversion at all
                                
        $spacer = str_repeat('-', $level);
        //print_log("{$spacer} convert {$name} ($level)", 'elastictools', LOG_LEVEL_TEST);

        $type = $field['type'];
        $index = $field['index'] ?? ''; /** php7 Null Coalesce Operator */ 
        $fields = $field['fields'] ?? [];
        $keywordField = $subfields ? [] : ['keyword' => ['type' => 'keyword', 'ignore_above' => 256]];
        
        $numeric_types = ['long', 'integer', 'short', 'byte', 'double', 'float', 'half_float', 'scaled_float'];
        $range_types = ['integer_range', 'float_range', 'long_range', 'double_range', 'date_range'];
        $other_types = ['text','keyword','ip','token_count','murmur3', 'binary'];
        $geo_types = ['geo_point', 'geo_shape'];
        
        $unset = [];
        if(in_array($type, $numeric_types)) $unset = ['precision_step']; /** no longer supported */
        if($type == 'geo_point') $unset = ['geohash', 'geohash_prefix', 'geohash_precision', 'lat_lon'];
        if(count($unset)) {
          foreach($unset as $f) unset($field[$f]);
        }
        
        $unchanged_types = array_merge($numeric_types, $range_types, $geo_types, $other_types);
        if($type[0] == '_' || in_array($type, $unchanged_types)) return $field; /** nothing to do */
        
        /** below handles nested, as well as query/bool/filter/match */
        $props = $field['properties'] ?? [];
        if(is_array($props) && count($props)) {
          foreach($props as $fld => $flds) {
            $subFields[$fld] = $this->convertField($fld, $flds, $level + 1);
          }
          $field['properties'] = $subFields;
          return $field;
        }

        
        
 
        /** todo: fix */
        $standard_analyzers = ['standard', 'simple', 'whitespace', 'stop', 'keyword', 'pattern', 'fingerprint'];
        $language_analyzers = ['arabic', ' armenian', ' basque', ' bengali', ' brazilian', ' bulgarian', ' catalan', ' cjk', ' czech', ' danish', ' dutch', ' english', ' finnish', ' french', ' galician', ' german', ' greek', ' hindi', ' hungarian', ' indonesian', ' irish', ' italian', ' latvian', ' lithuanian', ' norwegian', ' persian', ' portuguese', ' romanian', ' russian', ' sorani', ' spanish', ' swedish', ' turkish', ' thai']; 
        $all_analyzers = array_merge($standard_analyzers, $language_analyzers);
        $analyzer = $field['analyzer'] ?? '';
        if($analyzer && !in_array($analyzer, $all_analyzers)) {
          //throw new \Exception("The field '{$name}' in '{$this->index}' uses a custom analyzer '{$analyzer}' - not yet supported.");
          unset($field['analyzer']);
        }
        
        /** now only true/false */
        if($index && !in_array($type, ['text', 'string'])) {
          $field['index'] = $index == 'analyzed' ? true : false;
        }
        
        //$reserved = ['search', 'query', 'term', 'value', 'owner'];
        //if(in_array($name, $reserved)) return [$name => $this->convertField($name, $field, true)];
        
        $subFields = [];        
        if(is_array($fields) && count($fields)) {
          foreach($fields as $fld => $flds) {
            $subFields[$fld] = $this->convertField($fld, $flds, $level + 1);
          }
        }

        $debug = "Index: {$this->index} Type: {$this->type}\n";
        $debug .= "Field".dump($field,true);
        $debug .= "Mapping:".dump($this->mapping,true);
        
        
        switch($type) {          
          case 'string':
            if($index == 'not_analyzed') {
              $field['type'] = 'keyword';
              unset($field['index']);
            } else {
              $field['type'] = 'text';                       
              if(!isset($field['fields']) && count($keywordField)) $field['fields'] = $keywordField; /** make optional ? */
            }
            break;
          case 'completion':
            unset($field['payloads']);
            break;
          case 'date':
            break;
          case 'boolean':
            break;            
          case 'percolator':
            break;
          case 'join':
            break;
          case 'object':
            break;
          default:
            print_log("Unknown field type='{$type}' {$debug}", 'elastictools', LOG_LEVEL_TEST);
            throw new \Exception("Unknown field type='{$type}'");            
            break;
        }
        if(count($subFields)) $field['fields'] = $subFields;
        
        return $field;
    }
}


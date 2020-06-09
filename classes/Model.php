<?php
namespace AFR;

class Model 
{
    protected static $database;
    protected static $classes;
    
    public function __construct()
    {
      
        global $__CLASSES;
        global $db;
        self::$database = $db;
        self::$classes = $__CLASSES;
        return;
        
        /** todo: use below, get rid of global $db
  
        $dbconf = get_config('db', 0);        
        if(!$dbconf) die("Failed to fetch database parameters");
        $db_db =    $_SESSION['dev'] || $_SESSION['staging'] ? $dbconf->devdb : $dbconf->db;
        $_SESSION['db_dev'] = $db_db == $dbconf->devdb ? 1 : 0;
        
        $db_user = $dbconf->user;
        $db_host = $dbconf->host ?: "localhost";
        $db_password = $dbconf->pass;
        $db_dsn = "mysql://$db_user:$db_password@$db_host/$db_db";
        
        // connect to db
        try {
            $db = new \PDO("mysql:host={$db_host};dbname={$db_db};charset=utf8", $db_user, $db_password);
            
            $db->setAttribute(\PDO::ATTR_ERRMODE, \PDO::ERRMODE_EXCEPTION);
            $db->setAttribute(\PDO::ATTR_EMULATE_PREPARES, false);
            self::$database = $db;
            self::$classes = $__CLASSES;
        }
        catch(PDOException $e) {         
            echo $e->getMessage();
        }
        */ 
    }
    
    public function find($obj_type, $search=[], $options=[])
    {      
        if(!self::$classes[$obj_type]) die("No class $obj_type");
        return find_object($obj_type, $search, $options);   
    }
  
    public function get($obj_type, $id, $options=[])
    {
        if(!self::$classes[$obj_type]) die("No class $obj_type");
        return get_object($obj_type, $id, $options);         
    }  
}

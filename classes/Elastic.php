<?php
namespace AFR;
use GuzzleHttp\Client;
use GuzzleHttp\Psr7;
use GuzzleHttp\Exception\RequestException;

class Elastic 
{
    protected static $database;
    protected static $classes;
    protected static $url;
    protected static $index;
    protected static $client;
    
    public function __construct($index)
    {      
        global $__CLASSES;
        global $db;
        
        self::$url = $url = "http://localhost:9200";
        self::$database = $db;
        self::$classes = $__CLASSES;
        self::$index = $index;
        $client = new Client([
          'base_uri' => $url, // Base URI is used with relative requests
          'timeout'  => 2.0, // You can set any number of default request options.
        ]);        
        self::$client = $client;
        return;
    }
    
    public function health() {
        $response = self::$client->request('GET', "/_cluster/health");
        return json_decode($response->getBody()->getContents());
    }
    
    public function find($type, $search=[], $options=[])
    {      
        $url = self::$url;
        
    }
  
    public function get($type, $id, $options=[])
    {
        $index = self::$index;
        $endpoint = "/{$index}/{$type}/{$id}";
        try {
            $response = self::$client->request('GET', $endpoint);
            return json_decode($response->getBody()->getContents());
        } catch (RequestException $e) {
            //echo Psr7\str($e->getRequest());
            //return $e->getBody()->getContents();
            if ($e->hasResponse()) {
                $response= $e->getResponse()->getBody(true);
                return json_decode($response);
                
                $error = Psr7\str($e->getResponse());
                echo("Exception: {$error}");
                return json_decode($error);
            }
        }
    }  
    public function insert($type, $obj, $id = 0, $options=[])
    {
        $index = self::$index;
        $body = (array) $obj;
        $endpoint = $id ? "/{$index}/{$type}/{$id}" : "/{$index}/{$type}";
        $response = self::$client->request('POST', $endpoint, ['body' => json_encode($body)]);
        return json_decode($response->getBody()->getContents());

        echo("INSERT {$url}<br>");
        
        return json_send($endpoint, "POST", $obj);
        //return json_post($endpoint, $obj);
    }
    public function update($type, $id, $obj, $options=[])
    {
        $url = self::$url;
        $index = self::$index;
        $endpoint = "{$url}/{$index}/{$type}/{$id}";
        return json_send($endpoint, "POST", $obj);
        //return json_post($endpoint, $obj);
    }
    public function delete($type, $id, $options=[])
    {
        //$url = self::$url;
        $index = self::$index;

        $endpoint = "/{$index}/{$type}/{$id}";
        $response = self::$client->request('DELETE', $endpoint);
        return json_decode($response->getBody()->getContents());
        
        /** using scripting - disabled */
        $endpoint = "{$url}/{$index}/{$type}/{$id}/_update";
        // return json_post($endpoint, ["script" => "ctx.op=delete"]);

        /** using DELETE - need to look up curl params */
        $endpoint = "{$url}/{$index}/{$type}/{$id}";
        return json_send($endpoint, "DELETE");
        
        /** using bulk - works */
        $endpoint = "{$url}/_bulk";
        return json_post($endpoint, "\n".json_encode(["delete" => ["_index" => $index, "_type" => $type, "_id" => $id]])."\n");
    }
    public function status($type, $id, $options=[])
    {
        $url = self::$url;
        return json_get("{$url}/_cluster/health");
    }
}
         
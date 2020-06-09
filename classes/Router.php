<?php
namespace AFR;

/** Seiously simple router
    Thanks: http://upshots.org/php/php-seriously-simple-router
  */

class Router {
	
	private static $routes = array();
	
	private function __construct() {}
	private function __clone() {}
	
	public static function route($pattern, $callback) {
		$pattern = '/^' . str_replace('/', '\/', $pattern) . '$/';
		self::$routes[$pattern] = $callback;
	}
	
	public static function execute($url) {
		foreach (self::$routes as $pattern => $callback) {
			if (preg_match($pattern, $url, $params)) {
				array_shift($params);
				return call_user_func_array($callback, array_values($params));
			}
		}
	}
}
  

/** Usage:
Router::route('blog/(\w+)/(\d+)', function($category, $id){
  print $category . ':' . $id;
});
Router::execute($_SERVER['REQUEST_URI']);
// if url was http://example.com/blog/php/312 you'd get back "php:312"...

// nginx - redirect all to index if file/directory don't exist
location / {
  try_files $uri $uri/ /index.php;
}
          
*/
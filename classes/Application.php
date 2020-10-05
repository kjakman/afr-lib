<?php
namespace AFR;

use Monolog\Logger;
use Monolog\Handler\StreamHandler;
//use Monolog\Handler\StreamHandler;

class Application {
    
    public $config;     
    
    public $site;
    public $site_id;
    public $page;
    public $page_id;
    public $content_id;                                     
    public $language;

    /** following based on path and page */
    public $controller;
    public $action;
    public $name;
    public $obj_id;
    public $obj_type;
    public $view;
    public $template;
    public $view_template;
    
    public $path;
    public $model;
    
    //store the URL values on object creation
    public function __construct($siteObj, $userObj)
    {
        $this->model = new Model();
        $this->site = $siteObj;
        $this->site_id = $siteObj->id;
        $this->user = $userObj ?: null;
        $this->user_id = $userObj ? $userObj->id : 0;
        $this->user_level = $userObj ? $userObj->user_level : 0;
        $this->uri = $uri = $_SERVER['REQUEST_URI'];
        
        $parts = parse_url($uri);
        $this->path = rtrim($parts['path'],'/'); // remove trailing slash in case nginx didn't;
        $this->query = $parts['query'];
        
        /** todo: config */
        $config = new \stdClass;        
        $config->app_root = "/web/comps/php/apt-dev";
        $config->doc_root = rtrim($_SERVER['DOCUMENT_ROOT'], '/');                
        $config->data_dir = $data_dir = "/web/data-dev/afr";                
        $config->log_dir = $log_dir = "{$data_dir}/monolog";                
        $config->live = 0;
        $this->config = $config;

        // $container = DI\ContainerBuilder::buildDevContainer();

        // create a log channel
        $this->logger = $log = new \Monolog\Logger('name');
        $log->pushHandler(new StreamHandler($log_dir, Logger::WARNING));        
    }
    
    public function run()
    {
        global $__PAGE; /** remmove later - needed for now by functional part */

        // add records to the log
        $this->logger->info('Running app');

        //$w = '-a-zA-Z0-9@:%._\+~#='; // characters allowed in url (no tilde or period)
        $w = 'a-zA-Z0-9-';
        /** look for page matching route */

        /** [lang?][controller?][action?] */
        Router::route("(/[a-z]{2})?(/[$w]{3,})?(/[$w]{3,})?", function ($lang, $controller, $action) {
            return ['rule' => 0, 'lang' => ltrim($lang,'/'), 'controller' => ltrim($controller,'/'), 'action' => ltrim($action,'/'), 'name' => '', 'obj_id' => '', 'view' => ''];
        });         

        /** [lang?][controller?][id?][view?] */
        /** allows us skip the action for view so /product/33 instead of /product/view/33 */
        Router::route("(/[a-z]{2})?/([$w]{3,})/([$w]+~)?(\d+)(/[\w-_]+)?", function ($lang, $controller, $name, $id, $view) {
            return ['rule' => 1, 'lang' => ltrim($lang,'/'), 'controller' => $controller, 'action' => 'view', 'name' => rtrim($name,'~'), 'obj_id' => $id, 'view' => ltrim($view,'/')];
        });

        /** [lang?][controller?][action][id?][view?] */
        Router::route("(/[a-z]{2})?/([$w]{3,})/([$w]{3,})/([\w-_]+~)?(\d+)(/[\w]+)?", function ($lang, $controller, $action, $name, $id,$view) {
            return ['rule' => 2, 'lang' => ltrim($lang,'/'), 'controller' => $controller, 'action' => $action, 'name' => rtrim($name,'~'), 'obj_id' => $id, 'view' => ltrim($view,'/')];
        });

        $data = Router::execute($this->path) ?: []; 
        if(!$data['controller']) $data['controller'] = 'home'; 
        if(!$data['action']) $data['action'] = 'index'; 
         foreach($data as $k => $v) $this->$k = $v;    
        
         
         $this->template = $tpl = "{$data['controller']}/{$data['action']}";
                                             
        print_log("App::run:    tpl: {$tpl} Path: {$this->path} Request: ".$_SERVER['REQUEST_URI']." Data:".dump($data,true), 'app', LOG_LEVEL_TEST);
                                                                    
        /** expensive DB look-up, move to below ? */
        if($page = $this->route()) {
            //print_log("found Page for {$this->site_id} {$this->path}:".dump($page,true), 'app', LOG_LEVEL_TEST);
            if($page->user_level && $page->user_level > $this->user_level) {
                redirect_login($this->path);
            }
                                                    
            if($page->controller) $this->controller = $controller =    $page->controller;            
            if($page->template) $this->template = $page->template;
            if($page->language) {
                $this->setLanguage($page->language);
            }
            
            
            $this->page = $__PAGE = $page;
                                
        } else {
            //print_log("No Page for {$this->site_id} {$this->path}", 'app', LOG_LEVEL_TEST);
        }
                
        
        /** get scope - try new controller class */
        if($ctrl = $this->createController()) {            
            $scope = $this->scope = $ctrl->executeAction();
        } elseif(function_exists("controller_{$this->controller}")) { /** fallback on scope from old controller function */
            //echo($namespace.ucfirst($this->controller)." does not exist<br>");
            //echo("Found controller_{$this->controller} tpl={$this->template}");
            $scope = $this->scope = call_user_func("controller_{$this->controller}", $this->site, $this->page);
            
        } else {
            $ctrl = new Error("badUrl: controller {$this->controller} not found", 404);
            $scope = $this->scope = $ctrl->executeAction();            
        }
             
        if($scope->view_template) $this->view_template = $scope->view_template; /** for partial views */
        
        
        /** render template with scope */
        global $__header;
        $this->header = $__header;
        $partial = isset($_GET['_partial-view']) ? true : false;
        $output = $this->render($partial);
        echo($output);
    }

    public function setLanguage($language)
    {
        $this->language = $language;
    }

    /** return "page" (template, controller, user-level) based on data from path */
    protected function route($data)
    {
        /** first, try config file - yaml */
        
        /** then try database */
        $model = $this->model;
        $path = "{$this->path}/"; // look in database for page for this path
        $search = ['site_id' => $this->site_id, 'path' => $path];
        //print_log("Looking for page_content:".dump($search,true), 'app', LOG_LEVEL_TEST);
        
        $content = $model->find('page_content', $search);
        if(!$content) return null;
        
        //print_log("Found page_content:".dump($content,true), 'app', LOG_LEVEL_TEST);
        $this->content_id = $content->id;
        $this->page_id = $content->page_id;
        $page = Model::get('site_page', $this->page_id);
        if(!$page) die("No page with id={$this->page_id}"); /** todo: exception */
        
        $page = object_merge($content, $page);
        $page->content_id = $content->id;
        return $page;
    }
    
    protected function render($partial = false)
    {
        $viewloc = 'views/' . get_class($this) . '/' . $this->action . '.php';
        
        $template = $this->template;
        if($partial && $this->view_template) {
            $template = "components/".$this->view_template;
        }

        print_log("Render: tpl={$template} partial:{$partial}", 'app', LOG_LEVEL_TEST);
        
        $view = new \AFR\View($this, $template);        
        $scope = $this->scope;
        
        $output = $view->render($this);
        return $output;
        
    }
    

    /** establish the requested controller as an object
            performs routing based on query vars */
    protected function createController() {
        $controller = $this->controller;
        
        include_once("controllers/".ucfirst($controller).".php");

        $namespace = "AFR\\Controller\\";
        if(class_exists($namespace.ucfirst($this->controller))) { // new; allow define controller w/o page in db
            $this->controllerClass = ucfirst($this->controller);
            
            $parents = class_parents('\\AFR\\Controller\\'.$this->controllerClass);

            //does the class extend the controller class?
            if (in_array($namespace."BaseController",$parents)) {
                
                //does the class contain the requested method?
                if (method_exists($namespace.$this->controllerClass,$this->action)) {
                    $ctrl = $namespace.$this->controllerClass;
                    //echo("ok, found class {$this->controllerClass} with action={$this->action}");
                    return new $ctrl($this, $this->template, $this->action, $this->view);
                } else {
                    //bad method error
                    return new Error("badUrl: no method {$this->action} for {$this->controller}", 404);
                }
            } else {
                //bad controller error
                return new Error("badUrl: not a child of BaseController", 404);
            }
        } else {
            return null;
            //die("No content or class for path={$path}");
        }
    }
}



/** Helper classes */

class Error {
    private $error;
    private $data;
    private $status;
    public function __construct($error, $status=404, $vars='') {
        $this->data = $vars;
        $this->status = $status;
        $this->error = $error;
    }
    public function executeAction() {
        
        $error = $this->error ?: "unknown error";
        echo("Error: {$error}<br>");
        if($path = $this->path) echo("Path: {$path}");
    }
}
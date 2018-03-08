<?php
namespace AFR\Controller;

abstract class BaseController
{
    protected $urlvalues;
    protected $action;
    protected $template;
    protected $app;
    protected $scope;
    
    public function __construct($app, $template, $action, $view='')
    {
        $this->app = $app;
        $this->action = $action;
        $this->view = $view;
        $this->template = $template;
        $this->scope = [];
    }
    
    public function executeAction()
    {
        $this->scope = $scope = $this->{$this->action}($this->view);
        print_log("Executed action={$this->action} view:{$this->view}");
        return $this->scope;
    }
}

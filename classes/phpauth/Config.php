<?php
namespace PHPAuth;

/** By Kjetil From https://github.com/PHPAuth/PHPAuth */
/**
 *
 */
class Config
{
    private $dbh;
    private $config;
    private $config_table = 'config';

    /**
     *
     * Config::__construct()
     *
     * @param \PDO $dbh
     * @param string $config_table
     */
    public function __construct(\PDO $dbh, $config_table = 'config')
    {
        $this->dbh = $dbh;

        if (func_num_args() > 1)
            $this->phpauth_config_table = $config_table;

        $this->config = array();

        $query = $this->dbh->query("SELECT * FROM {$this->config_table}");

        while($row = $query->fetch()) {
            $this->config[$row['setting']] = $row['value'];
        }

        $this->setForgottenDefaults(); // Danger foreseen is half avoided.
    }

    /**
     * Config::__get()
     * 
     * @param mixed $setting
     * @return string
     */
    public function __get($setting)
    {
        return $this->config[$setting];
    }

    /**
     * Config::__set()
     * 
     * @param mixed $setting
     * @param mixed $value
     * @return bool
     */
    public function __set($setting, $value)
    {
        $query = $this->dbh->prepare("UPDATE {$this->phpauth_config_table} SET value = ? WHERE setting = ?");

        if($query->execute(array($value, $setting))) {
            $this->config[$setting] = $value;
            return true;
        } 
        return false;
    }

    /**
     * Config::override()
     * 
     * @param mixed $setting
     * @param mixed $value
     * @return bool
     */
    public function override($setting, $value){

        $this->config[$setting] = $value;
        return true;

    }

    /**
     * Config::overrideAll() - by Kjetil: override site specific variables
     * 
     * @param mixed $setting
     * @param mixed $value
     * @return bool
     */
    public function overrideAll($siteObj) {
      
      $domain = $siteObj->domain;
      $site_url = site_full_url($siteObj);
      
      $this->override('client_id'               , $siteObj->client_id);
      $this->override('site_id'                 , $siteObj->id);
      $this->override('site_name'               , $siteObj->name);
      $this->override('site_url'                , $site_url);
      $this->override('site_email'              , $siteObj->email_address ?: "info@{$domain}");
      $this->override('site_key'                , sha1($domain).":Pkw)(qLbcJ");
      //$this->override('site_timezone'           , $siteObj->name);
      $this->override('site_activation_page'    , ltrim(site_page_link($siteObj, PAGE_PASSWORD), '/'));
      $this->override('site_password_reset_page', ltrim(site_page_link($siteObj, PAGE_PASSWORD), '/'));
        
      //site_name                           | PHPAuth                                     |
      //site_url                            | http://dev.apartments-for-rent.com/auth/php |
      //site_email                          | no-reply@apartments-for-rent.com            |
      //site_key                            | fKLkjd7&90:Pkw)(qLbcJ                    |
      //site_timezone                       | Europe/Amsterdam                            |
      //site_activation_page                | activate                                    |
      //site_password_reset_page            | reset                                       |
                
      $this->config[$setting] = $value;
      return true;

    }

    /**
     * Danger foreseen is half avoided.
     *
     * Set default values.
     * REQUIRED FOR USERS THAT DOES NOT UPDATE THEIR `config` TABLES.
     */
    private function setForgottenDefaults()
    {
        // verify* values.

        if (! isset($this->config['verify_password_min_length']) )
            $this->config['verify_password_min_length'] = 3;

        if (! isset($this->config['verify_password_max_length']) )
            $this->config['verify_password_max_length'] = 150;

        if (! isset($this->config['verify_password_strong_requirements']) )
            $this->config['verify_password_strong_requirements'] = 1;

        if (! isset($this->config['verify_email_min_length']) )
            $this->config['verify_email_min_length'] = 5;

        if (! isset($this->config['verify_email_max_length']) )
            $this->config['verify_email_max_length'] = 100;

        if (! isset($this->config['verify_email_use_banlist']) )
            $this->config['verify_email_use_banlist'] = 1;

        // emailmessage* values

        if (! isset($this->config['emailmessage_suppress_activation']) )
            $this->config['emailmessage_suppress_activation'] = 0;

        if (! isset($this->config['emailmessage_suppress_reset']) )
            $this->config['emailmessage_suppress_reset'] = 0;

    }


}

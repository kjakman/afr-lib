# Patina - An old mishmash of code

This project was started more than 10 years ago to implement a rental platform for short stay housing. At the time, there were not really any good PHP frameworks, and OOP was a joke, so I wrote my own MVC framework inspired by 	[ATK](https://github.com/atkphpframework/atk).  

It was never cleaned up and includes tons of old unused code. It's all procedural, and does not adhere to modern coding standards. 

Whereas this code is actually used in production for [Apartments-For-Rent](https://www.apartments-for-rent.com), [YourAmsterdamHousing](https://www.youramsterdamhousing.com), [vPatina](https://www.vpatina.com), [Hitabrick](https://www.hitabrick.com), and [ElasticTools](https://elastictools.io) the code in this repository will not (yet) run out of the box. I will complete this when I have time.

## Model
Defined in `afr/include/classes.inc`

`$__CLASSES`: An array that defines the available "classes", like this:

`CLASS_NAME => [USER_LEVEL,GROUP,TABLE,KEY,FLAGS,DESCRIPTOR,SORT] PARENT_CLASS PARENT_FIELD`

`$__CLASS_ATTRIBUTES`

An array of arrays with the attributes for each "class" These define fields that correspond to database fields.

`db_field_name => ['Screen name', 'type', 'default', 'flags', 'relation/options', 'filters']`

Methods to manipulate the model are found in `afr/includes/db_object.inc`, the most important being:

`$obj = get_object($class_name, $id, [$field]);`

`$objs = find_objects($class_name, $search, [$options]);`

`$obj = find_object($class_name, $search, [$field],
 [$options]);`
 
`list($id, $errors, $messages) = update_object($class_name,
 $dat÷a, $id);÷`
 
`list($count, $errors) = delete_object($class_name, $id);`


These are wrappers for PDO functions that manipulate the underlying database. 


## Views
Smarty template files defined in afr/pages, can be overwritten per site

`afr/pages/base`: base views

`afr/pages/*`: correspond to pages

`afr/pages/compoents`: subviews for pages

## Controllers
Defined in afr/include/controllers.inc

`function controller_[name]($app, $options = []) {
}`

The controller writes data to `$scope = $app->page->scope`
The value of `$scope->foo` is accessible in the smarty template as `{scope.foo}`. Note that even though `$scope` is an object in the controller, it's converted to an array before being passed to smarty.

## Bootstrap
`index.php`

Loads `afr/include/set_env.inc` and calls `site_application($app)`

`site_application()` is defined in includes/helpers_site.inc

## Config files
`config/main.yml`: Non-sensitive config data. Loaded into `$app->config` with `config_load()` in `set_env.inc`
`/web/.priv/.priv.txt`: Json file with sensitive passwords for database etc, outside web document root. Use `get_config($key, $site_id)` to read key.
`/web/.priv/.priv-dev.txt`: Same, but for dev subdomain (see below)

# Subdomains
`dev.domain.com`:   Dev database, dev codebase
`test.domain.com`:  Live database, dev codebase
`www.domain.com`:   Live database, live codebase


## set_env.inc

## afr/include/class_handlers.inc

## Directories

## Files

## Composer

## Started OOP, but unfished:
In `afr/classes/` is the beginning of a slightly more modern version, using OOP classes for Model/View/Controller

## Logging


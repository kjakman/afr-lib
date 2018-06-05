<?php
/**
 * Smarty {json} plugin
 *
 * Type:       function
 * Name:       json
 * Date:       Apr 12, 2009
 * Purpose:    Read JSON from file, decode and assign data to Smarty template variable
 * Syntax:     {json file="filename.json"}: 'file' is a required parameter (URL)
 *             Predefined additional parameters:
 *             - assign="data": assign all JSON data to template variable $data
 *             - obj2obj [ Boolean | default:false ]:
 *               decodes JSON objects as either PHP associative arrays or PHP objects
 *             - debug [ Boolean | default:false ]: print decoded data in template
 *             Variable parameters:
 *             {json file="filename.json" home="homepage" lang="languages"}:
 *               assign (JSONdata)["homepage"] to template variable $home
 *               and (JSONdata)["languages"] to $lang,
 *               compare to: {config_load file="filename.conf" section="homepage"}
 * Install:    Drop into the plugin directory
 * @link       http://jlix.net/extensions/smarty/json
 * @author     Sander Aarts <smarty at jlix dot net>
 * @copyright  2009 Sander Aarts
 * @license    LGPL License
 * @version    1.0.1
 * @param      array
 * @param      Smarty
 
 * Rewritten by Kjetil Larsen for Smarty v3 in 2016
 
 */
function smarty_function_json($params, &$smarty)
{
	if(!is_callable('json_encode')) {
		return "{json} requires json_encode() function (PHP 5.2.0+)";
	}
	if (empty($params['file'])) {
		return "{json} parameter 'file' must not be empty";
	}
	if (isset($params['assign'], $params[$params['assign']])) {
		return "{json} parameter 'assign' conflicts with a variable assign parameter (both refer to the same variable)";
	}
	
	$data = file_get_contents($_SERVER['DOCUMENT_ROOT'].$params['file']);
	if (empty($data))
		$json = '';
	else
		$json = json_decode($data, true);
		
	unset($params['file']);
	
	if (!empty($params['assign'])) {
		$smarty->assign($params['assign'], $json);
		return ''; 
	}
	
	return $json;
}

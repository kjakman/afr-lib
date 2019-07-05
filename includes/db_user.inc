<?php
/* From db_user */

function get_user_level($id) {
  return get_object('user', $id, 'user_level');
}

function get_user_level_name($id) {
  return get_object('user_level', $id, 'user_type');
}

/** unsused - automatic comment added on  */
function get_user_type($user_id) {
  return strtolower(get_user_role($user_id));
}

/** unsused - automatic comment added on  */
function get_user_role($user_id) {
  return get_user_level_name(get_user_level($user_id));
}

/** unsused - automatic comment added on  */
function check_user_pass($username, $password1, $password2) {  
  if(strlen($password1) < 8) return phrase('the password must be at least 8 characters long', CAPITALIZE);
  if($password1 != $password2) return phrase('the passwords do not match', CAPITALIZE);
  if (!preg_match('@[A-Z]@', $password1) || !preg_match('@[a-z]@', $password1) || !preg_match('@[0-9]@', $password1)) {
    return phrase('the password must contain at least one UPPERCASE letter, one lowercase letter, and one number', CAPITALIZE);
  }
  if(!check_username($username)) return phrase('the username %s is taken, please choose another', CAPITALIZE, $username);
  
  return '';
}

// get auth object
function user_phpauth() {
  global $phpauth, $db, $__APP;
  if($phpauth) return $phpauth;
  $config = new PHPAuth\Config($db);
  $config->overrideAll($__APP->site);
  $phpauth = new PHPAuth\Auth($db, $config, $__APP->language);    
  return $phpauth;    
}

/** new user reg function */
function user_register($username, $password, $profileData=[], $params=[]) {
  $phpauth = user_phpauth();

  $errors = [];
  $result = $phpauth->register($username, $password, $password, $params);
    
  if(isset($result['error']) && !strlen(trim($result['message']))) $result['message'] = "Please use at least one UPPERCASE letter, one lowercase letter, and one number in the password";
  if($error = $result['error']) return array(0, $result['message']); //array(0, $error);
  
  $user_id = $result['id'];  
  $profileData['user_id'] = $user_id;

  print_log("New user:".dump($result,true), 'user', LOG_LEVEL_TEST);

  print_log("New profile".dump($profileData,true), 'user', LOG_LEVEL_TEST);
  
  list($profile_id, $errors) = add_object('user_profile', $profileData);
  if($errors) return array(0, $errors); /** Failed to add user profile - but user was added - not good */ 
  
  $userData = array('profile_id' => $profile_id);
  $flds = ['aff_id', 'user_level', 'roles', 'auto_password'];
  foreach($flds as $fld) {
    if(isset($profileData[$fld])) $userData[$fld] = $profileData[$fld];
  }
  
  // Temporary fix by dci
  $user_level = $roles = $auto_password;
  $user_level = $profileData['user_level'];
  $roles = $profileData['roles'];

  $auto_password = $profileData['auto_password'];
  if(!$user_level) $user_level = $params['user_level'];
  if(!$roles) $roles = $params['roles'];
  if(!$auto_password) $auto_password = 0;
    
  print_log("Edit user:".dump($userData,true), 'user', LOG_LEVEL_TEST);  
  
  // Pradeepa hack
  $data = db_query("UPDATE user
                    SET profile_id = ".$profile_id.",  user_level = ".$user_level.", roles=".$roles.", auto_password = ".$auto_password."
                    WHERE id = ".$user_id);
  
  // Existing code flow
  //list($id, $errors) = update_object('user', $userData, $user_id);
  return array($user_id, $errors);
}      


/** todo: allow for separate internal email for staff */
/** unsused - automatic comment added on  */
function staff_email($siteObj_or_id, $name=true) {
  return site_email($siteObj_or_id, $name);
}

function host_email($obj, $name=true) {
  if($host_id = $obj->host_id) return user_email($host_id, $name);
  return '';
}

function guest_email($obj, $name=true) {
  if($guest_id = $obj->guest_id) return user_email($guest_id, $name);
  return '';
}

/** public email address for the site - fallback to legal entity or info@domain.com */
function site_email($siteObj_or_id, $name=true) {
  $siteObj = any2obj('site', $siteObj_or_id);
  if(!$siteObj) return '';
  if($name === true || $name === 1) $name = $siteObj->name ?: $siteObj->domain;  
  $email = '';
  if(is_email($siteObj->email_address)) {
    $email = $siteObj->email_address;
  } else {
    $legal_email = get_object('legal_entity', $siteObj->legal_entity_id, 'email_address');
    if(is_email($legal_email)) $email = $legal_email;
  }
  if(!$email) $email = "info@{$siteObj->domain}";
  
  return $name ? email_recipient($email, $name) : $email;
}

function site_replyto($siteObj_or_id, $name=true) {
  $siteObj = any2obj('site', $siteObj_or_id);
  if(!$siteObj) return '';
  if($name === true || $name === 1) $name = phrase('do not reply', CAPITALIZE);
  $email = "do-not-reply@{$siteObj->domain}"; 
  return $name ? email_recipient($email, $name) : $email;  
}
  
/** alias - but default to use name of user. Use -1 for display name */
function user_email($userObj_or_id, $name=1) {
  return get_user_email($userObj_or_id, $name);
}

// if name is set, return 'Tobias Brox <tobix@cpan.org>', else return just the email address
function get_user_email($userObj_or_id, $name='') {
  $userObj = any2obj('user', $userObj_or_id);
  $profileObj = find_user_profile($userObj);
  if($name === true || $name === 1) $name = ($profileObj->first_name || $profileObj->last_name) ? get_name_string($profileObj) : get_user_name($userObj);
  elseif($name === -1) $name = $profileObj->display_name ?: user_display_name($userObj);
  
  //dump($profileObj);
  $email = '';
  if(is_email($profileObj->email_address)) $email = $profileObj->email_address;  
  else if(is_email($userObj->email_address)) $email = $userObj->email_address;
  else if(is_email($userObj->username)) $email = $userObj->username;  
  
  if(!$email) return ''; //"none:".$userObj->username;
  
  return $name ? email_recipient($email, $name) : $email;
}

function username($userObj_or_id) {
  $userObj = any2obj('user', $userObj_or_id);
  return $userObj->username;
}


/** unsused - automatic comment added on  */
function get_user_level_id($st) {
  global $db;
  return db_getOne("select user_level from user_level where user_type='$st'");
}

/** unsused - automatic comment added on  */
function auth_user($source, $target) {
  //echo("auth_user: source: $source target: $target");
  if($source == $target) return true;
  return get_user_level($source) > get_user_level($target);
}


function get_user_language($userObj_or_id, $options=NULL) {
  $user = get_user($userObj_or_id);
  return $user->language;
}

// returns either name or username
function get_user_name($userObj_or_id, $options=NULL) {
  if(!$userObj_or_id) return '';
  $userObj = any2obj('user', $userObj_or_id);
  //dump($userObj);
  //$userObj = get_user($userObj->id);
  if($userObj->first_name || $userObj->last_name) return get_name_string($userObj);
  return $userObj->username;
}

// returns true if username is available
/** unsused - automatic comment added on  */
function check_username($username) {
  global $__CLIENT_ID;
  if(!$username) return false;
  $id = find_object('user', ['username' => $username, 'user.client_id' => $__CLIENT_ID], ['fields' => 'user.id']);
  return $id ? false : true;
  
  //return $userObj ? false : true;
}

// returns user object if username and password are correct
function check_user($username, $password) {
  global $__CLIENT_ID;
  $userObj = find_object('user', array('username' => $username, 'client_id' => $__CLIENT_ID));
  return generateHash($password, $userObj->pass) == $userObj->pass ? $userObj : NULL;
}

// returns userObj merged with profileObj
function get_user($userObj_or_id, $array = false) {
  
  $userObj = any2obj('user', $userObj_or_id); /** we use one2one integration */
  $userObj->user_id = $userObj->id;
  if(!trim($userObj->display_name)) $userObj->display_name = pick_first(trim($userObj->first_name), trim(get_name_string($userObj)), $userObj->username);
  
  return $array ? obj2array($userObj) : $userObj;
  
  /** not in use
  if($userObj = any2obj('user', $userObj_or_id)) {
    $userObj =  user_merge_profile($userObj);
  }
  return $array ? obj2array($userObj) : $userObj;
   */
}


/** not in use 
// look for profile and merge with user
// create profile if doesn't exist
function user_merge_profile($userObj) {
  $user_id = $userObj->id;
  $mergedObj = $userObj;
  $profileObj = find_user_profile($userObj);
  if(!$profileObj) { // TODO: create it 
  }
  if($profileObj) {
    $profile_id = $profileObj->id;
    unset($profileObj->id);
    
    if(!$profileObj->display_name) $profileObj->display_name = pick_first(trim($profileObj->first_name), trim(get_name_string($profileObj)), $userObj->username);
    $arr = explode(' ',trim($profileObj->display_name));
    $profileObj->display_name_short = $arr[0]; // first name or first part of display name
    
    $profileObj->profile_picture = user_profile_picture($profileObj, array('size' => 'big'));
    $profileObj->profile_picture_small = user_profile_picture($profileObj, array('size' => 'thumb'));

    $mergedObj = object_merge($userObj, $profileObj, true);
    $mergedObj->profile_id = $profile_id;
    $mergedObj->user_id = $mergedObj->id = $user_id;
  } else {
    $mergedObj->display_name = pick_first($mergedObj->display_name, trim($mergedObj->first_name), trim(get_name_string($mergedObj)), $userObj->username);
  }
  unset($mergedObj->password);
  unset($mergedObj->pass);
  return $mergedObj;
}
*/

// get display name from user_id
function user_display_name($userObj_or_id, $options=NULL) {
  $userObj = get_user($userObj_or_id); /** get_user() takes care of picking first */
  return $userObj->display_name;
  
  /** old
  if(!$user_id || !is_whole_number($user_id)) return '';
  $profileObj = get_user($user_id);
  print_log("user_display_name id=$user_id profile:".dump($profileObj,true), 'debug', LOG_LEVEL_TEST);
  return $profileObj->user_display_name; 
   */
  
  //if(!$profileObj) return get_user_name($userObj_or_id);
  //return 'foo';
  //return 
}

/** get display name from profile or user object
function get_display_name($user_or_profileObj, $options=NULL) {
  $profileObj = $user_or_profileObj->user_id ? $user_or_profileObj : find_user_profile($user_or_profileObj);
  if(!$profileObj) return '';
  return pick_first($profileObj->display_name, get_name_string($profileObj), get_user_name($profileObj->user_id));
}
*/

/* end user fn */

/** unsused - automatic comment added on  */
function user_message_count() {
  global $u_id;
  $count = $alert = 0;
  if($u_id) {
    $search = ['receiver_id' => $u_id, 'read_on' => ATT_DEFAULT_NULL, 'notify' => 1];
    $count = object_count('chat', $search);
    $search = ['receiver_id' => $u_id, 'read_on' => ATT_DEFAULT_NULL, 'notify' => 1, 'status_change' => ['>=', 0]];
    $alert = object_count('chat', $search);
  }
  
  return [$count, $alert];
}

/** unsused - automatic comment added on  */
/** unsused - automatic comment added on  */
function add_user_profile($userObj_or_id) {
  $userObj = any2obj('user', $userObj_or_id);
  if(find_user_profile($userObj)) return array(0, "User $userObj->id already has a user profile");
  
  $user_profile['client_id'] = $userObj->client_id;
  $user_profile['user_id'] = $userObj->id;
  $user_profile['title'] = phrase('about_me', CAPITALIZE);
  if(is_email($userObj->username)) {
    $user_profile['email_address'] = $userObj->username;
    $user_profile['display_name'] = $userObj->first_name;
  } else {
    $user_profile['display_name'] = $userObj->first_name ?: $userObj->username;
    $user_profile['email_address'] = $userObj->email_address;
  }
  $user_profile['url_web'] = $userObj->web_address;
  $user_profile['country'] = pick_first($userObj->country, $userObj->user_country, $_SESSION['country_code']);
  $user_profile['phone'] = $userObj->phone;
  
  $copyAr = array('client_id', 'site_id', 'first_name', 'infix', 'last_name', 'address1', 'address2', 'zip', 'state', 'city');
  foreach($copyAr as $key) $user_profile[$key] = $userObj->$key;
  
  list($up_id, $up_errors) = add_object('user_profile', $user_profile);
  if($up_errors) {
    echo("Oops".dump($user_profile, true));
    dump($up_errors);
  }
  return array($up_id, $up_errors); 
}


function user_profile_uploader($userObj_or_id, $options = array()) {
  //global $u_id, $site_id,$u_level;
  $userObj = get_user($userObj_or_id);
  
  //any2obj('user', $userObj_or_id);
  //$userObj = user_merge_profile($userObj);

  $user_id = $userObj->user_id;
  $profile_id = $userObj->profile_id;

  $fld = $options['field'] ?: 'photo';
  
  $class_attrs = get_object_attributes('user_profile', false);
  $class_def = $class_attrs[$fld];
  if(!$class_def) return("uploader: $fld not found");
  
  $upload_options = $class_def[ATT_OPTIONS];
  if($upload_options && !is_array($upload_options)) $upload_options = json_decode($upload_options, true); /** we now use json in attribute definition */ 
  
  $upload_options = array_extend($upload_options, $options);

  $upload_options['allow_multiple'] = 0; /** disable for now */
  
  //$upload_options['minimum_size'] = array(1080, 1920); // either short side >= 1080 or long side >= 1920
  $val = $userObj->$fld;
     
  $mediaData['user_id'] = $user_id;
  $mediaData['parent_id'] = $profile_id;
  $mediaData['parent_type'] = 'user_profile';
  $mediaData['parent_field'] = $fld;
  
  $upload_options['download_template'] = $options['download_template'] ?: "";
  
  $upload_options['media_data'] = $mediaData;
  $upload_options['data-value_list'] = $val;
  $upload_options['show_file_list'] = false;
  $upload_options['title'] = "Click or drag to upload profile image";
  
  $handler = isset($options['handler']) ? $options['handler'] : '';
  if($upload_options['handler'] = $handler) {
    $handler_options = array();
    $upload_options['handler_options'] = $handler_options;
  }

  $uploader = jquery_file_uploader("user-$profile_id-$fld", $val, $upload_options);

  return $uploader;
}


function find_user_profile($userObj_or_id) {
  $userObj = any2obj('user', $userObj_or_id);
  $profileSearch['user_id'] = $userObj->id;
  return find_object('user_profile', $profileSearch);
}

/** unsused - automatic comment added on  
function pet_profile_link($siteObj, $petProfileObj_or_id, $options = NULL) {
  $source_only = isset($options['source_only']) ? $options['source_only'] : true; 
  $petProfileObj = any2obj('pet_profile', $petProfileObj_or_id);
  if($petProfilePageObj = find_object('site_page', array('site_id' => $siteObj->id, 'page_type' => PAGE_PROFILE, 'object_type' => 'pet_profile'))) {
    $base_link = site_page_link($siteObj, $petProfilePageObj, $options);
    $profile_link = $base_link.$petProfileObj->id.'/';
    return $source_only ? $profile_link : html_link($profile_link, get_display_name($petProfileObj_or_id));
  }
  return '';
}
*/

function user_photo($userObj_or_id, $size = "small", $options=[]) {
  $userObj = any2obj('user', $userObj_or_id);
  $options['size'] = $size;
  return user_profile_picture($userObj, $options);
}

function user_profile_picture($user_or_profileObj, $options = NULL) {
  $profileObj = $user_or_profileObj->user_id ? $user_or_profileObj : find_user_profile($user_or_profileObj);
  //$size = $options['size'] ?: 'tiny';
  if($profileObj->photo) {
    $options['source_only'] = true;
    if(!$options['size']) $options['size'] = 'tiny';
    $options['full'] = true;    
    $photo_url = print_media($profileObj->photo, $options);
  } else {
    $photo_url = in_array($size, array('thumb', 'tiny')) ? '/images/png/user_profile_pic_80.png' : '/images/png/user_profile_pic_225.png';
  }
  return $photo_url;
}

// find users matching username or email (for password recovery)
function find_users($siteObj, $username_or_email, $use_site_id=true) {
  $username_or_email = trim($username_or_email);
  if(!$username_or_email) return array();
  $search['active'] = 1;
  if(is_email($username_or_email)) {
    $search[DB_LOGICAL_OR] = array('username' => $username_or_email, 'email_address' => $username_or_email);
  } else {
    $search['username'] = $username_or_email;
  }
  $tbl = get_object_table("user");
  if($use_site_id) $search[$tbl.'site_id'] = $siteObj->id; // try this site first
  $userObjs = find_objects('user', $search);

  if(!$userObjs && $use_site_id) return find_users($siteObj, $username_or_email, false); // try all sites
  return $userObjs;
}

// takes a bitwise user roles, return the max. user level
function role2level($roles) {
  $roleObjs = find_objects('user_role', array(), array('order' => 'id desc'));
  foreach($roleObjs as $roleObj) {
    $role = (int) $roleObj->id;
    if(has_role($roles, $role)) return $roleObj->user_level;
  }
  return 0;
}

// returns search array for given role
function user_has_role($userObj_or_id, $role) {
  $userObj = any2obj('user', $userObj_or_id);
  return ((int) $userObj->roles & (int) $role);
}

/** unsused - automatic comment added on  */
function has_role($roles, $role) {
  return ((int) $roles & (int) $role);
}

function add_role($roles, $role) {
  return ((int) $roles | (int) $role);
}

// returns an array of users linked to this user
// if host = 1; only users that are hosting the relationship
// if host = -1; only users that are hosted by this user
function linked_users($userObj_or_id, $host=0) {
  $userObj = any2obj('user', $userObj_or_id);
  $user_id = $userObj->id;
  if($host > 0) {
    $search['user_id'] = $user_id;
  } else if($host < 0) {
    $search['host_user_id'] = $user_id;
  } else {
    $search[DB_LOGICAL_OR] = array('host_user_id' => $user_id, 'user_id' => $user_id);
  }
  $mapObjs = find_objects('user_map', $search); // user is host
  $user_ids = array();
  foreach($mapObjs as $mapObj) {
    $user_ids[] = $mapObj->user_id == $user_id ? $mapObj->host_user_id : $mapObj->user_id;
  }
  return array_unique($user_ids);
}


function send_user_account_invitation($site_id, $user_id, $code, $email_address) {
  if(!$site_id) return json_error_object("Missing site id");
  if(!$user_id) return json_error_object("Missing user id");;
  if(!$code) return json_error_object("Missing code");
  $userObj = get_object('user', $user_id);
  if(!$userObj) return json_error_object("Invalid user");
  if(!is_email($email_address)) return json_error_object("Invalid email");
  if($error = send_account_invitation_mail($site_id, $userObj, $code, $email_address)) return json_error_object($error);
  return json_success_object("Invitation sent to $email_address");   
}


function user_account_upgrade($site_id, $user_id, $code) {
  if(!$site_id) return json_error_object("Missing site id");
  if(!$user_id) return json_error_object("Missing user id");
  if(!$code) return json_error_object("Missing code");
  $userObj = get_object('user', $user_id);
  if(!$userObj) return json_error_object("Invalid user");
  $hash = substr($code, 0,8);
  $host_user_id = substr($code, 8);
  $sh = shorthash($host_user_id);
  if(!$host_user_id || !is_whole_number($host_user_id)) return json_error_object("Invalid host user");
  if($sh != $hash) return json_error_object("Invalid code");
  $hostUserObj = get_object('user', $host_user_id);
  if(!$hostUserObj) return json_error_object("Host not found");
  if($user_id == $host_user_id) return json_error_object("You cannot link to yourself");
  
  $auth_sites = $userObj->auth_sites ? explode(',',$userObj->auth_sites) : array();
  if($userObj->user_level < USER_LEVEL_SYSTEM && $userObj->site_id != $site_id && !in_array($site_id, $auth_sites)) return json_error_object("User not authorized");
  
  $host_auth_sites = $userObj->auth_sites ? explode(',',$hostUserObj->auth_sites) : array(); 
  if($hostUserObj->user_level < USER_LEVEL_SYSTEM && ($hostUserObj->site_id != $site_id && !in_array($site_id, $host_auth_sites))) return json_error_object("Host not authorized");
    
  //if(!user_has_role($host_user_id, USER_ROLE_CURATOR)) return json_error_object("Host not authorized");    
  //if(!user_has_role($user_id, USER_ROLE_ARTIST)) user_add_role($userObj, USER_ROLE_ARTIST); 

  $host_name = user_display_name($hostUserObj);
  $data = array('site_id' => $site_id, 'user_id' => $user_id, 'host_user_id' => $host_user_id);
  if($mapObj = find_object('user_map', $data)) {
    $result->message = $mapObj->verified ? "This account was already linked" : "An account link was already requested, but is pending approval from $host_name";
  } else {
    list($id, $errors) = add_object('user_map', $data);
    if($errors) return json_error_object($errors);
    if($id) {
      $success = 1;
      $result->message = "Thank you. Your account will be linked once approved by $host_name. You will be notified by email.";
    }
  }
  
  $result->success = 1;
  //return json_error_object($hostUserObj->username);
  
  return $result;  
}

function user_add_role($userObj_or_id, $role) {
  $debug = false;
  $userObj = any2obj('user', $userObj_or_id);
  if(!$userObj || !$role) {
    if($debug) echo("Couldn't find user $userObj_or_id");
    return;
  }
  $roleObj = get_object('user_role', $role);
  if(!$roleObj) {
    if($debug) echo("Couldn't find role $role");
    return;
  }
  
  $user_level = max($roleObj->user_level, $userObj->user_level); 
  db_query("UPDATE user SET user_level='$user_level', roles= roles | $role WHERE id=$userObj->id");
  if($debug) echo("UPDATE user SET user_level='$user_level', roles= roles | $role WHERE id=$userObj->id");
  //list($id, $errors) = update_object('user', $userData, $userObj->id);    
}

// returns search array for given role
/** unsused - automatic comment added on  */
function user_role_search($role) {  
  return array("roles & $role" => array('>', 0));
}

function provider_find_profile($siteObj, $provider_name, $provider_identifier) {
  $debug = $_SESSION['auth_debug'];
  if(stripos($provider_name, 'live') === 0) $provider_name = "Windows Live"; // we use Windows Live, HybridAuth uses Live
  if($provider_id = find_object('provider', array('name' => $provider_name), 'id')) {
    $profileSearch['site_id'] = $siteObj->id;
    $profileSearch['provider_id'] = $provider_id;
    $profileSearch['identifier'] = $provider_identifier;    
    $profileSearch['client_id'] = $siteObj->client_id;
    if($debug) dump($profileSearch);
    return find_object('provider_profile', $profileSearch);
  }
  return NULL;
}

// note: The "providerProfileObj" is not our object, but from HybridAuth
function provider_create_new_user($siteObj, $provider_name, $providerProfileObj) {
  $debug = $_SESSION['auth_debug'];
  
  $provider_identifier = $providerProfileObj->identifier;

  if(!$provider_name) {if($debug) print_log("provider_create_new_user:missing provider_name", 'provider', LOG_LEVEL_ALWAYS); return NULL;}
  if(!$providerProfileObj) {if($debug) print_log("provider_create_new_user: missing profile object", 'provider', LOG_LEVEL_ALWAYS); return NULL;}
  if(!$provider_identifier) {if($debug) print_log("provider_create_new_user: missing identifier for $provider_name", 'provider', LOG_LEVEL_ALWAYS); return NULL;}
  if($provider_name == 'Live') $provider_name = "Windows Live"; // we use Windows Live, HybridAuth uses Live
  $provider_id = find_object('provider', array('name' => $provider_name), 'id');  
  if(!$provider_id) {if($debug) print_log("provider_create_new_user: no provider found for $provider_name", 'provider', LOG_LEVEL_ALWAYS); return NULL;}

  // if email is found and verified, and it matches existing email, then do not create new user
  $provider_email = $providerProfileObj->email;
  $email_verified = $provider_email && $providerProfileObj->emailVerified == $provider_email ? 1 : 0;

  //dump($provider_profile);
  if($debug) print_log("Provider email: $provider_email Verified:$email_verified<br>", 'provider', LOG_LEVEL_ALWAYS);

  $tbl = get_object_table('user');
  
  // if($debug && $provider_email && $email_verified) print_log("Found verified provider profile email $provider_email(user_id=$user_id)<br>");
  if($user_id = $_SESSION['user_id']) { // user is (was) already logged in (but this is called before auth in set_env, so maybe we should double check?
    //$userObj = get_object('user', $user_id);    
    $userProfileObj = find_object('user_profile', array('user_id' => $user_id));
  } else  if($email_verified) {
    if($userProfileObj = find_object('user_profile', array('email_address' => $provider_email, 'email_verified' => 1, 'client_id' => $siteObj->client_id))) {
      $user_id = $userProfileObj->user_id;                                                                                                        
      if($debug) print_log("Found user_profile with email $provider_email (user_id=$user_id)<br>", 'provider', LOG_LEVEL_ALWAYS);    
    } elseif($userObj = find_object('user', array('email_address' => $provider_email, 'active' => 1, "{$tbl}.client_id" => $siteObj->client_id))) {
      $user_id = $userObj->id;
      if($debug) print_log("Found active user with email $provider_email (user_id=$user_id)<br>", 'provider', LOG_LEVEL_ALWAYS);    
    } elseif($userObj = find_object('user', array('username' => $provider_email, 'active' => 1, "{$tbl}.client_id" => $siteObj->client_id))) {
      $user_id = $userObj->id;
      if($debug) print_log("Found active user with username $provider_email (user_id=$user_id)<br>", 'provider', LOG_LEVEL_ALWAYS);    
    } else {
      if($debug) print_log("No active user found with username $provider_email for client=$siteObj->client_id, creating<br>", 'provider', LOG_LEVEL_ALWAYS);
      //if($debug) print_log("site=".dump($siteObj,true), 'provider', LOG_LEVEL_ALWAYS);
    }    
  } 

  //if($debug) print_log("GH: user_id=$y", 'provider', LOG_LEVEL_ALWAYS);
  
  if(!$user_id) { // this user does not exist in our system, create the user

    $username =  $provider_email ?: $provider_name."-".$provider_identifier; // todo: make friendlier
    if($debug) print_log("Creating new user with username $username<br>", 'provider', LOG_LEVEL_ALWAYS);    
                                                                   
    if($userObj = find_object('user', array('username' => $username, "{$tbl}.client_id" => $siteObj->client_id))) {
      //dump($userObj);
      $user_id = $userObj->id;
      if($debug) print_log("Found existing user for $siteObj->domain with username $username (user_id=$user_id)<br>", 'provider', LOG_LEVEL_ALWAYS);
    } else {
      if($debug) print_log("No user found, creating with id=$user_id username=$username...<br>", 'provider', LOG_LEVEL_ALWAYS);
  
      // ok, 3rd party login worked, create new user in our system
      //dump($provider_profile);
      print_log("provider_create_new_user: Provider: $provider_name site:$siteObj->id user: $user_id", "provider", LOG_LEVEL_ALWAYS);
      
    
      $phpauth = user_phpauth();
      
      $pw1 = $pw2 = "A".substr(generateHash($provider_identifier), 0, 8)."!";      
      $result = $phpauth->register($username, $pw1, $pw2);
      
      $user_id = $result['id'];
      if(!$user_id || $result['error']) {
        print_log("PHPAUTH User register failed. uname=$username pw=$pw1 error:{$result['message']}".dump($result,true), 'provider', LOG_LEVEL_ALWAYS);
      } else {
        print_log("PHPAUTH OK id=$user_id", 'provider', LOG_LEVEL_ALWAYS);
        $user_data = provider_map_profile($providerProfileObj); // map to our fields
  
        $user_data['user_id'] = $user_id;
        $user_data['username'] = $username;
        $user_data['email_address'] = $user_data['email'] = $provider_email;
        $user_data['user_level']  = USER_LEVEL_USER;
        $user_data['language']    = $user_profile['language'];
        $user_data['web_address'] = $user_profile['url_web'];
        $user_data['phone'] = $user_profile['phone'];
        $user_data['address1'] = $user_profile['address1'];
        $user_data['site_id'] = $siteObj->id;
        $user_data['client_id'] = $siteObj->client_id;
        $user_data['email_verified'] = $user_data['verified'] = $email_verified;
        
        list($profile_id, $user_errors) = add_object('user_profile', $user_data); // this will also create user_profile
        
        if($debug && $user_errors) print_log("Failed to add user_id errors:".dump($user_errors, true), 'provider', LOG_LEVEL_ALWAYS);
        elseif($debug && $profile_id) print_log("Added user_id=$user_id<br>", 'provider', LOG_LEVEL_ALWAYS);
        
        if($profile_id) {
          update_object('user', array('profile_id' => $profile_id, 'auto_password' => 1), $user_id);
        }
        
      }
      
    }
  }
  
  if($user_id) {
    
    list($pp_id, $pp_errors) = provider_add_profile($siteObj, $provider_id, $user_id, $providerProfileObj);
    if($debug && $pp_id) print_log("Added provider profile $pp_id<br>", 'provider', LOG_LEVEL_ALWAYS);
    if($pp_errors) dump($pp_errors);
    
    if($userProfileObj) { // do we want to update profile ? redirect ?
      
    } else { // add new user profile
      /* this is now done in class handlers 
      $user_profile = $user_data;
      $user_profile['client_id'] = $siteObj->client_id;
      $user_profile['user_id'] = $user_id;
      $user_profile['title'] = $user_profile['title'] ?: phrase('about_me', CAPITALIZE);
      list($up_id, $up_errors) = add_object('user_profile', $user_profile);
      if($up_errors) dump($up_errors);
      */
    }
    
    if($debug && $up_id) print_log("Added user profile $up_id<br>", 'provider', LOG_LEVEL_ALWAYS);
    //print_log("added user profile $up_id<br>");
  }
  //dump($user_errors);
  //dump($pp_errors);
  //dump($up_errors);
  return $user_id;
}

// add provider_profile object: caller must first check if exists
function provider_add_profile($siteObj, $provider_id, $user_id, $providerProfileObj) {
  //echo("added user $user_id<br>");
  $provider_profile = provider_map_profile($providerProfileObj); // map to our fields
  //$provider_profile = obj2array($providerProfileObj);
  $provider_profile['site_id'] = $siteObj->id;
  $provider_profile['client_id'] = $siteObj->client_id;
  $provider_profile['user_id'] = $user_id;
  $provider_profile['friends'] = $providerProfileObj->friends;  
  $provider_profile['provider_id'] = $provider_id;
  $provider_profile['email_verified'] = $providerProfileObj->emailVerified;

  if($debug) {
    echo("Adding provider profile with data:");
    dump($provider_profile);
  }
  
  return add_object('provider_profile', $provider_profile);
}


// maps profile from hybridauth to our user_profile
function provider_map_profile($providerProfileObj) {
  $provider_profile = obj2array($providerProfileObj);
  $provider_profile['photoURL'] = str_replace('?type=square', '?type=large', $provider_profile['photoURL']); // get rid of tiny FB photo from HybridAuth
  
  $map = array(
   'websiteURL'    => 'url_web', 
   'photoURL'      => 'photo', 
   'profileURL'    => 'url_profile', 
   'displayName'   => 'display_name', 
   'firstName'     => 'first_name', 
   'lastName'      => 'last_name', 
   'birthDay'      => 'birth_day', 
   'birthMonth'    => 'birth_month', 
   'birthYear'     => 'birth_year',
  );
  return map_array($provider_profile, $map);  
}

// fetch profile using Hybridauth
/** Called in controllers.inc when linking */
function user_provider_profile($siteObj, $provider_id, $user_id, $options = array()) {
  $debug = false;
                       
  $config = docroot(false).'/afr/auth/config.php';
  
  //$apt_path = $_SESSION['app_root'];
  //require_once("$apt_path/auth/Hybrid/Auth.php");
  //$config = $apt_path.'/auth/config.php';

  $provider_options = array();
  $profileObj = null;
  $provider_name = get_object('provider', $provider_id, 'provider_id'); // HybridAuths provider name 
  //if(stripos($provider_name, 'windows') === 0) $provider_name = "Live"; // we use Windows Live, HybridAuth uses Live
  //echo("user_provider_profile");
  try {
    $hybridauth = new Hybrid_Auth($config); // initialize Hybrid_Auth with a given file
    $adapter = $hybridauth->authenticate($provider_name, $provider_options); // try to authenticate with the selected provider
    $profileObj = $adapter->getUserProfile();

    $user_contacts = $adapter->getUserContacts() ?: array();  // grab the user's friends list
    $profileObj->friends = $user_contacts ? json_encode($user_contacts) : '';
    $provider_identifier = $profileObj->identifier;
    if($ppfObj = provider_find_profile($siteObj, $provider_name, $provider_identifier)) { // if user exist on database
      if($ppfObj->user_id != $user_id) $error = "This provider is associated with another user account";
      // update ?
      //die("gh, found profile");
    } else { // add this profile
      list($pp_id, $pp_errors) = provider_add_profile($siteObj, $provider_id, $user_id, $profileObj);
      if($debug && $pp_id) echo("Added provider profile $pp_id<br>");
      if($pp_errors) $errors = $pp_errors;      
    }
    
  } catch(Exception $e) {
    $error = "Error: ".$e->getMessage();
    echo($error);
    //die("gh2");
  }

  $result = new stdClass;
  if($error) {
    $result->success = 0;
    $result->error = $error;    
  } else if($profileObj) {
    $result->success = 1;
    $result->data = $profileObj;    
  } else {
    $result->success = 0;
    $result->error = "Unknown error fetching $provider_name profile";
  }
  return $result;
}

// third party login using Hybridauth
function user_provider_login($provider_name, $options = array()) {
  global $__SITE;
  $apt_path = $_SESSION['app_root'];
  $debug = $_SESSION['auth_debug'];

  //require_once( "$apt_path/auth/Hybrid/Auth.php");
  //require_once( "../vendor/autoload.php");

  //$config = docroot(false).'/afr/auth/config.php';
  $config = docroot(false).'/config/hybridauth.php';

  //die("config=$config");
  $output = array();
  $output[]= now().": user_provider_login: $provider_name";
  
  $siteObj = $__SITE;
  $provider_id = find_object('provider', array('name' => $provider_name), 'id');
  

  $provider_options = array();

  $ts = now();
  print_log("$ts: user_provider_login Config:{$config} Provider: $provider_name (id: $provider_id) site:$siteObj->id", "provider", LOG_LEVEL_TEST);
  
  if(!$provider_id) return 0;
  $user_id = 0;
  $contact_json = '';
  try {
    //require_once('/web/comps/php/vendor/hybridauth/hybridauth/hybridauth/Hybrid/Auth.php'); #Hybridauth API
    //require_once('/web/comps/php/vendor/hybridauth/hybridauth-master/vendor/autoload.php'); #Facebook API
    $hybridauth = new Hybrid_Auth($config); // initialize Hybrid_Auth with a given file
    print_log("ha obj:".dump($hybridauth,true), "provider", LOG_LEVEL_TEST);
    $adapter = $hybridauth->authenticate(ucfirst($provider_name)); // try to authenticate with the selected provider
    print_log("adapter:".dump($adapter,true), "provider", LOG_LEVEL_TEST);
    if($provider_profile = $adapter->getUserProfile()) { // then grab the user profile      
      $output[] = "OK: Found $provider_name party profile for $provider_name: id=$provider_profile->identifier";

      if($provider_name == 'facebook') { // limit friends list to facebook for now
        $user_contacts = $adapter->getUserContacts() ?: array();  // grab the user's friends list
        if($user_contacts) { // get friends every time in case updated
          print_log("Found ".count($user_contacts)." contacts for $provider_name", 'provider', LOG_LEVEL_TEST);
          $contact_json =  $provider_profile->friends = json_encode($user_contacts);
        } else {
          print_log("No $provider_name contacts for $provider_profile->display_name", 'provider', LOG_LEVEL_TEST);
        }
      }
      
      if($debug) print_log(dump($provider_profile, true), 'provider', LOG_LEVEL_TEST);;
      $provider_identifier = $provider_profile->identifier;
      print_log("Found profile ID: $provider_identifier", 'provider', LOG_LEVEL_TEST);
      
      if($ppfObj = provider_find_profile($siteObj, $provider_name, $provider_identifier)) { // if user exist on database
        $user_id = $ppfObj->user_id;
        print_log("user_id=$user_id", 'provider', LOG_LEVEL_TEST);
        if($_SESSION['u_id'] && $_SESSION['u_id'] != $user_id) {
          if($debug) print_log("This provider profile is already linked to another account with user_id=$user_id (yours is $_SESSION[u_id])", 'provider', LOG_LEVEL_TEST);
          $user_id = 0;          
        } else {
          if($contact_json) update_object('provider_profile', array('friends' => $contact_json), $ppfObj->id);
        }
        if($debug) {
          //dump($ppfObj);
          print_log("Found provider profile with user_id=$user_id", 'provider', LOG_LEVEL_TEST);
        }
        
      } else { // if not, create a new one
        print_log("Gh2", 'provider', LOG_LEVEL_TEST);
        
        if($debug) print_log("Did not find provider profile - Creating provider profile", 'provider', LOG_LEVEL_TEST);
        $user_id = provider_create_new_user($siteObj, $provider_name, $provider_profile); 
        if($debug) print_log("New user_id=$user_id", 'provider', LOG_LEVEL_TEST);
      }
      
      if($user_id) {
        $userObj = get_object('user', $user_id);
        $_SESSION['provider_id'] = $provider_id;
        $_SESSION['provider_name'] = $provider_name;

        $authData = array('user_id' => $user_id, 'provider_id' => $provider_id, 'user_level' => $userObj->user_level, 'username' => $userObj->username, 'display_name' => user_display_name($userObj), 'status' => LOGIN_OK, 'expires' => date('Y-m-d H:i:s', time() + AUTH_EXPIRES));
        list($log_id, $log_errors) = add_object('auth_log', $authData);        
      }
    }
  } catch(Exception $e) {

    if(0 && strtolower($provider_name) == "google") {
      $hybridauth->logoutAllProviders();
      $hybridauth = new Hybrid_Auth($config);
      $adapter = $hybridauth->authenticate("Google");      
    }
    
    echo "Error: please try again!<br>".$e->getMessage();
    print_log("Error: please try again!<br>".$e->getMessage(), 'provider', LOG_LEVEL_TEST);
    $authData = array('user_id' => $user_id, 'provider_id' => $provider_id, 'status' => LOGIN_PROVIDER_ERROR);
    /** don't log - too many spider entries */
    //list($log_id, $log_errors) = add_object('auth_log', $authData);         
  }
  print_log("Done:".implode(PHP_EOL, $output), 'auth', LOG_LEVEL_TEST);

  return $user_id;
}


function chat_thread_id($data) {
  extract($data);
  if(!$sender_id) $sender_id = $data['parent_type']."_".$data['parent_id'];
  if(!$receiver_id) $receiver_id = $data['parent_type']."_".$data['parent_id'];
  
  return min($sender_id, $receiver_id).'-'.max($sender_id, $receiver_id);
}

// return list of hosts that this user can act on behalf of
function user_host_list($userObj_or_id, $options = array()) {
  if(!$userObj_or_id) return [];
  $userObj = any2obj('user', $userObj_or_id);
  $host_ids = array();
  if($userObj->user_level >= USER_LEVEL_OWNER || user_has_role($userObj, USER_ROLE_OWNER)) $host_ids[] = $userObj->id; // this user is a host
  if($linked_host_ids = linked_users($userObj)) { // todo: filter by role = owner
    $host_ids = array_merge($host_ids, $linked_host_ids);
  }
  return $host_ids;
}

// return list of artists that this user can act on behalf of
function user_artist_list($userObj_or_id, $options = array()) {
  if(!$userObj_or_id) return [];
  $userObj = any2obj('user', $userObj_or_id);
  $artist_ids = [];
  //if($userObj->user_level >= USER_LEVEL_CURATOR || !user_has_role($userObj, USER_ROLE_CURATOR)) return [];
  if($userObj->user_level >= USER_LEVEL_ARTIST || user_has_role($userObj, USER_ROLE_ARTIST)){
    $artist_ids[] = $userObj->id; // this user is a artist
  } 

  if($mapObjs = find_objects('user_map', ['user_id' => $userObj->id, 'verified' => 1, 'active' => 1])) {
    //$artist_ids = array();
    foreach($mapObjs as $mapObj) {
      
      $perms = json_decode($mapObj->permissions);
      $edit = $perms ? $perms->vault : '';
      if($edit == 'edit') $artist_ids[] = $mapObj->host_user_id;
    }
  }
  return $artist_ids;
}

function fix_profile_location($app, $options) {
  $limit = $options['limit'];
  $client_id = $app->client_id;
  $profileObjs = find_objects('user_profile', ['location_id' => 0, 'address1' => ['!=', '']], $options);
  $count = count($profileObjs);
  $output .= html_h2("$count profiles needs fixing");
  $rows = [];
  $rows[] = ['id', 'name', 'ad1', 'ad2', 'zip', 'city', 'state', 'co', 'lat', 'lng', 'status', 'loc_id', 'notes'];
  foreach($profileObjs as $profileObj) {
    if(!trim($profileObj->display_name)) $profileObj->display_name = pick_first(trim($profileObj->first_name), trim(get_name_string($profileObj)), get_object('user', $profileObj->user_id, 'username'));
    $data = [];
    $loc_id = $status = 0;
    $notes = '';
    $country_name = get_country_name($profileObj->country);
    if($profileObj->address1 && $profileObj->city && $profileObj->country) {
      $address = implode(",", array_filter([$profileObj->address1, $profileObj->address2, trim("{$profileObj->zip} {$profileObj->city}"), $profileObj->state, $country_name]));
      $data['address_formatted'] = $address;
      
      $result = google_geocode($address, ['full' => 1]);
      
      $status = $result->status;
      $results = $result->results;
      $last = array_pop($results);
      if($status == 'OK') {
        list($status, $acc, $lat, $lng, $bounds) = parse_geometry($last->geometry);
      } else {
        $status = 0;
      }
      
      
      if($status==200) {
        $scope->http_code = "200"; //override error set in set_env.inc due to unknown slug
        $center = [$lat, $lng];
        $locObj = center2loc($center);
        $boundsObj = $bounds ? json_decode($bounds) : radius2bounds($locObj, DEFAULT_RADIUS);
        $bounds = any2bounds($boundsObj, true);
        
        $data['parent_type'] = 'user_profile';
        $data['parent_id'] = $profileObj->id;
        $data['user_id'] = $profileObj->user_id;
        $data['name'] = $profileObj->display_name;
        
        $data['location_lat'] = $lat;
        $data['location_long'] = $lng;
        $data['location_bounds'] = $boundsJson = json_encode($bounds);

        $address = google_parse_address($last->address_components);
        if($last->formatted_address) $data['address_formatted'] = $last->formatted_address;
        
        //echo("Add:".dump($address,true));die();
        $data = array_merge($data, $address);        
        //echo("Data:".dump($data,true).dump($profileObj,true));die();
        
        list($loc_id, $errors) = add_object('location', $data);
        if($errors) {dump($errors);die();}

        $profileData = ['location_id' => $loc_id, 'display_name' => $profileObj->display_name, 
                        'location_lat' => $lat, 'location_long' => $lng, 'location_bounds' => $boundsJson,
                        'address_formatted' => $data['address_formatted'], 'zip' => $data['address_postcode'], 
                        'last_name' => trim($profileObj->last_name), 'first_name' => trim($profileObj->first_name)];
        list($id, $errors) = update_object('user_profile', $profileData, $profileObj->id);
        if($errors) {dump($errors);die();}
        if($id && $loc_id) $notes = html_span("OK", 'green');
        
      } else {
        //$output .= "Updated $id with ".dump($data,true);
        $notes = html_span("geocode failed", 'red');
      }
    } else {
      $notes = html_span("missing address", 'red');
    }
    
    
    $rows[] = [$profileObj->id, $profileObj->display_name, $profileObj->address1, $profileObj->address2, 
               $profileObj->zip, $profileObj->city, $profileObj->state, $country_name, $lat, $lng, $status, $loc_id, $notes];
    
  }
  $output .= html_table($rows, ['class' => 'web20']);
  return $output;
}

<?php

require "vendor/autoload.php";

use Abraham\TwitterOAuth\TwitterOAuth;

/** unsused - automatic comment added on  */
function twitter_fetch($screenname, $count=4, $auth=null) {
  if(!$auth) {
    $auth = array();
    $auth['customer_key'] = '0ENssb9IzMkc7kEgsKVnUwk8R';;
    $auth['customer_secret'] = 'v2OP4BZHiLbeYcyYXPfKeJDWbZNdwi9Ylo5yOacQw6qzJXoN65';
    $auth['access_token'] = '16577414-ZYAz9rIizSJP5x6EXNmY7nzm9Q2pRmyaW8gsJLLvD';
    $auth['access_token_secret'] = '584uUKUGxpVdpBbWQ1UB9ofa7p7fpNuKTxUOHLNaPt46b'; 
  }

  $twitter_customer_key           = $auth['customer_key'];
  $twitter_customer_secret        = $auth['customer_secret'];
  $twitter_access_token           = $auth['access_token'];
  $twitter_access_token_secret    = $auth['access_token_secret'];
  
  $connection = new TwitterOAuth($twitter_customer_key, $twitter_customer_secret, $twitter_access_token, $twitter_access_token_secret);
  
  $my_tweets = $connection->get('statuses/user_timeline', array('screen_name' => $screenname, 'count' => $count));
  
  if(isset($my_tweets->errors)) {           
    echo 'Error :'. $my_tweets->errors[0]->code. ' - '. $my_tweets->errors[0]->message;
  } else {
    foreach($my_tweets as &$tweet) {
      $tweet->text = twitter_format_link($tweet->text);
    }
  }
  return $my_tweets; 
}

//function to convert text url into links.
function twitter_format_link($s) {
  return preg_replace('@(https?://([-\w\.]+[-\w])+(:\d+)?(/([\w/_\.#-]*(\?\S+)?[^\.\s])?)?)@', '<a target="blank" rel="nofollow" href="$1" target="_blank">$1</a>', $s);
}

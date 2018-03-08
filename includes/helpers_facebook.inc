<?php
function facebook_sdk($app_id = '299906836726434') {
  //$lang = 'en_US';  
  $lang = substr($_SESSION['locale'], 0, 5); // exclude UTF8
  return "  
  <div id=\"fb-root\"></div>
  <script>(function(d, s, id) {
    var js, fjs = d.getElementsByTagName(s)[0];
    if (d.getElementById(id)) return;
    js = d.createElement(s); js.id = id;
    js.src = \"//connect.facebook.net/$lang/all.js#xfbml=1\";
    fjs.parentNode.insertBefore(js, fjs);
  }(document, 'script', 'facebook-jssdk'));</script>
  ";
  


  //echo("lang=$lang");
//  return "
//  (function(d, s, id) {
//    var js, fjs = d.getElementsByTagName(s)[0];
//    if (d.getElementById(id)) return;
//    js = d.createElement(s); js.id = id;
//    js.src = '//connect.facebook.net/$lang/all.js#xfbml=1&appId=$app_id';
//    fjs.parentNode.insertBefore(js, fjs);
//  }(document, 'script', 'facebook-jssdk'));
//  ";

//return "
  //<div id='fb-root'></div>
  //<script>(function(d, s, id) {
  //  var js, fjs = d.getElementsByTagName(s)[0];
  //  if (d.getElementById(id)) return;
  //  js = d.createElement(s); js.id = id;
  //  js.src = '//connect.facebook.net/en_US/all.js#xfbml=1&appId=$app_id';
  //  fjs.parentNode.insertBefore(js, fjs);
  //}(document, 'script', 'facebook-jssdk'));</script>
  //";
}

// simple like button, sdk must be installed
function facebook_like($url, $options=NULL) {
  $button = "<div class=\"fb-like\" data-send=\"false\" data-layout=\"button_count\" data-show-faces=\"false\" data-href=\"$url\"></div>";
  return html_span($button, 'up-3px-cont');
}

function facebook_likebox($user, $options=NULL) {
  global $__header;
  $fb_options['data-colorscheme'] = $options['colorscheme'] ?: 'light';
  $fb_options['data-height']      = $options['height']      ?: '70';
  $fb_options['data-width']       = $options['width']       ?: '292';
  $fb_options['data-stream']      = $options['stream']      ?: 'false';
  $fb_options['data-show-faces']  = $options['show-faces']  ?: 'false';
  $fb_options['data-header']      = $options['header']      ?: 'false';
  if($options['iframe']) {
    return '<iframe src="http://www.facebook.com/plugins/like.php?href=http%3A%2F%2Fwww.facebook.com%2F'.urlencode($user).'&amp;layout=standard&amp;show_faces='.$fb_options['data-show-faces'].'&amp;width='.$fb_options['data-width'].'&amp;action=like&amp;font=arial&amp;colorscheme='.$fb_options['data-colorscheme'].'&amp;height=27" scrolling="no" frameborder="0" style="border:none; overflow:hidden; width:630px; height:27px; float:left; margin-left:10px;" allowTransparency="true"></iframe>';
  }
  $__header['onload'] .= facebook_sdk();
  foreach($fb_options as $k => $v) $items[] = "$k='$v'";
  $options_str = implode(' ', $items);
  return "<div id='fb-root'></div><div class='fb-like-box' data-href='http://www.facebook.com/$user' $options_str></div>";
}

// my own server-side facebook feed using graph API 
function facebook_feed($user, $options=NULL) {
  $limit= $options['limit'] + 1 ?: 0; // fetch one more to see if we need to show next link
  $offset= $options['offset'] ?: 0;
  $token = $_SESSION['facebook_access_token'];
  $url = "https://graph.facebook.com/$user/feed/?access_token=$token";
  if($limit) $url .= "&limit=$limit";
  if($offset) $url .= "&offset=$offset";
  $json = get_html($url);
  $data = json_decode($json);
  return $data->data;
}

// formats data returned by my own server-side facebook_feed using graph API 
function facebook_wall($user, $options=NULL) {
  $limit= $options['limit'] ?: 0;
  $offset= $options['offset'] ?: 0;
  if(!$user) die("site_facebook_wall: no user");
  $dataAr = facebook_feed($user, $options);
  $items = array();
  if(!$dataAr) return '';
  foreach($dataAr as $data) {
    $data->internal_link = "/facebook/?offset=".($i + $offset);
    $items[] = facebook_list_item($data, $options);
    if($limit && ++$i >= $limit) break;
  }

  $output .= article_list($items, $options);
  
  $links = array();
  if($offset > 0) { // there are more messages
    $previous = max($offset - $limit, 0);
    $next_js = "onclick = \"facebook_wall('$user', '$limit', '$previous'); return false\"";
    $prev_link = html_link('#', phrase('previous', CAPITALIZE), array('class' => 'abs_left', 'js' => $next_js));
    $links[] = $prev_link;
  }

  if($limit && $i >= $limit && count($dataAr) > $limit) { // there are more messages
    $next = $offset + $limit;
    $next_js = "onclick = \"facebook_wall('$user', '$limit', '$next'); return false\"";
    $next_link = html_link('#', phrase('next', CAPITALIZE), array('class' => 'abs_right', 'js' => $next_js));
    $links[] = $next_link;
  }
  // dump($links);
  if(count($links)) $output .= implode(' ', $links);

  // dump($dataAr);
  return html_div($output, 'facebook', "id='facebook_wall'");
}

// one wall item for my own facebook_wall (compact list view)
function facebook_list_item($data, $options=NULL) {    
  $article->title = $data->from->name;
  $article->content = facebook_format_text($data->message);
  $article->image = $data->picture;
  $article->created  = iso2human($data->created_time, array('show_year' => false));
  $article->external_link = $data->link;    
  $article->internal_link = $data->internal_link; // set in  facebook_wall

  // currently not in use   
  $icon  = $data->icon;
  $type  = $data->type;
  $likes = $data->likes && $data->likes->count > 0 ? $data->likes->data : array();
  $comments = $data->comments && $data->comments->count > 0 ? $data->comments->data : array();

  return article_list_item($article, $options);

}

// formats the "message" in the wall item for my own facebook_wall
function facebook_format_text($msg) {
  $msg = str_replace("\n", '<br />', $msg); // breaks
  // links
  
  return $msg;  
}

// client side facebook wall using jQuery plugin "neosmart"
function neosmart_wall($user, $token = '299906836726434|g0UGaDPzf4otbNo_8OtMlMSP7Zk', $options=NULL) {
  $div = $options['div_id'] ?: 'neosmart_wall';
  global $__header;
  $limit  = $options['limit'] ?: 5;
  $offset = $options['offset'] ?: 0;

  // $like_this = phrase('like_this', CAPITALIZE);
  // $likes_this = phrase('likes_this', CAPITALIZE);
  // $people = phrase('people', CAPITALIZE);
  // $more = phrase('older_posts', CAPITALIZE);

  $like_this = 'vinden dit leuk';
  $likes_this = 'vindt dit leuk';
  $people = 'mensen';
  $more = 'Meer verslagen';

  $__header['css'][] = "/jquery/neosmart.fb.wall/wall.css";
  $__header['scripts'][] = "/jquery/neosmart.fb.wall/wall.js";
  $__header['onload'] .= '$("#'.$div."\").fbWall({id: '$user', 
                                                  accessToken:'$token', 
                                                  max:$limit, 
                                                  offset: $offset,
                                                  translateLikeThis: '$like_this',
                                                  translateLikesThis: '$likes_this',
                                                  translatePeople: '$people',
                                                  showGuestEntries:true,
                                                  showComments:true,
                                                  timeConversion:24
                                                  });";

  //$debug = "token=$token limit=$limit offset=$offset ".print_r($options, true);
  
  $more_js = 'onclick="neosmart_wall(\'UhuruSports\', \''.$limit.'\', \''. ($offset + $limit).'\');");return false;"';
  $more_link = html_link('#', $more, array('js' => $more_js)); 
  $output = html_div('', '', "id='$div'").html_div($more_link, 'article_more');
  return $output;
  
  // $('#example1').fbWall({ id:'162253007120080',accessToken:'206158599425293|7809823973348bcf8cd72f6d.1-100000221135225|BW9n2eoyL7EYvJs7GEmv61NbBFk'});   
}

// client side facebook wall using code from: 
function facebook_wall_js($user, $token, $options=NULL) {
  $limit  = $options['limit'] ?: 5;
  $offset = $options['offset'] ?: 0;

  global $__header;
  $__header['css'][] = "/jquery/facebook-wall/css/styles.css";
  $__header['scripts'][] = "/jquery/facebook-wall/js/jquery.tmpl.min.js";
  $__header['scripts'][] = "/jquery/facebook-wall/js/script.js";
  $__header['onload'] .= "facebook_wall_js('wall', '$user', '$token', $limit, $offset);";

  $output = '

<div id="fb_container">
	<div id="wall" class="fb_wall"></div>
</div>

<script id="headingTpl" type="text/x-jquery-tmpl">
<h1>${name}<span>on Facebook</span></h1>
</script>

<script id="feedTpl" type="text/x-jquery-tmpl">
<li>
	<img src="${from.picture}" class="fb_avatar" />
	
	<div class="fb_status">
		<h2><a href="http://www.facebook.com/profile.php?id=${from.id}" target="_blank">${from.name}</a></h2>
		<p class="fb_message">{{html message}}</p>
		{{if type == "link" }}
			<div class="fb_attachment">
				{{if picture}}
					<img class="fb_picture" src="${picture}" />
				{{/if}}
				<div class="fb_attachment-data">
					<p class="fb_name"><a href="${link}" target="_blank">${name}</a></p>
					<p class="fb_caption">${caption}</p>
					<p class="fb_description">${description}</p>
				</div>
			</div>
		{{/if}}
	</div>
	
	<p class="fb_meta">${created_time} · 
	{{if comments}}
		${comments.count} Comment{{if comments.count>1}}s{{/if}}
	{{else}}
		0 Comments
	{{/if}} · 
	{{if likes}}
		${likes.count} Like{{if likes.count>1}}s{{/if}}
	{{else}}
		0 Likes
	{{/if}}
	</p>
	
</li>
</script>
';

   return $output;
}

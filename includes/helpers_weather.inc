<?php

/** unsused - automatic comment added on  */
function weather_get($city) {
  $city = "London,uk";
  $url = "http://api.openweathermap.org/data/2.5/weather?q=$city";
  $json = file_get_contents($url);
  $obj = json_decode($json);
  dump($obj);
}

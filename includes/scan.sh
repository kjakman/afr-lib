#!/bin/sh

ls –l –R –full-time > ~/domainname.files
grep 'base64' -H * -r > ~/domainname.base64
grep '46esab' -H * -r > ~/domainname.46esab
grep 'stripslashes' -H * -r > ~/domainname.strip
grep 'sql' -H * -r > ~/domainname.sql
grep ' preg_replace ' -H * -r > ~/domainname.sql
grep ' HTTP_REFERER ' -H * -r > ~/domainname.referer
grep ' HTTP_USER_AGENT' -H * -r > ~/domainname.user



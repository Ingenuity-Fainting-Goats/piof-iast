#!/bin/bash

service rsyslog restart
sleep 2

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
SRCPATH=$SCRIPTPATH/..

echo "#############################"
echo "[i] Running cli tests..."
#(cd $SRCPATH/modules && pwd && php -dextension=./piof.so $SCRIPTPATH/test.php)
echo "#############################"
echo "[i] Running apache2handler tests...\n"
echo "<?php md5('admin');?>" > /var/www/html/md5.php
echo "[i] md5"
curl --header "X-PIOF-IAST: strlen,eval,md5,mysql_query" -v http://localhost/md5.php
echo "#############################"
echo "<?php function prova($a) { return true; } prova('aaa');?>" > /var/www/html/prova.php
echo "[i] prova, md5"
curl --header "X-PIOF-IAST: strlen,exit,md5,prova" -v http://localhost/prova.php
echo "#############################"
echo "[i] testFunc1,testFunc2,testFunc3,md5"
cp $SCRIPTPATH/test.php /var/www/html/test.php
curl --header "X-PIOF-IAST: md5,testFunc2,testFunc1,testFunc3" -v http://localhost/test.php
echo "#############################"
tail -f /var/log/piof.debug.log 
# PIOF-iast - PHP Open Iast Agent based on PIOF project
![PIOF-iast PHP Open Iast Agent based on PIOF project](https://github.com/ingenuity-ninja/piof)

# What
PIOF-iast - Is a IAST ( Interactive Application Security Testing ) Agent for PHP language based on [PIOF Project](https://github.com/ingenuity-ninja/piof).

# Why
It is build on top of [PIOF Project](https://github.com/ingenuity-ninja/piof) with the aim of create a IAST Agent useful to identify Web Based vulnerabilities using instrumentation technique.

# Where
Tested on:
```
PHP 7.2.10-0ubuntu0.18.04.1 (cli) (built: Sep 13 2018 13:45:02) ( NTS )
Copyright (c) 1997-2018 The PHP Group
Zend Engine v3.2.0, Copyright (c) 1998-2018 Zend Technologies
    with Zend OPcache v7.2.10-0ubuntu0.18.04.1, Copyright (c) 1999-2018, by Zend Technologies
```
# Who
Alessandro - twitter.com/rhpco - rakkapriccio [ $placeholder_at ] gmail [ $placeholder_dot ] com

Martino - twitter.com/martinolessio - mlessio6 [ $placeholder_at ] gmail [ $placeholder_dot ] com

# Quick Start
```
./run.sh
```
## How it works
The Agent is a PHP Extension that could be loaded as module in a web server setup, such as Apache server, as shown in the docker testing environment.
Moreover the Agent support request and response custom header `X-PIOF-IAST`.

### Request
With following command, containing a `X-PIOF-IAST` custom header, it is possibile to choose which functions should be hooked by the Agent:
```
curl -I --header "X-PIOF-IAST: custom_function,mysql_query,md5" http://localhost/index.php 
```
In the above example, following functions:
..* [custom_function()](https://www.php.net/manual/en/functions.user-defined.php)
..* [mysql_query()](https://www.php.net/manual/en/function.mysql-query.php)
..* [md5()](https://www.php.net/manual/en/function.md5.php)

Are going to be hooked during the execution of the page `index.php`.

Below an example of raw request containing the custom header.
```
> GET /index.php HTTP/1.1
> Host: localhost
> User-Agent: curl/7.52.1
> Accept: */*
> X-PIOF-IAST: md5,custom_function,mysql_query
```

### Response
The header `X-PIOF-IAST` it is returned by the Agent enabled in the server containing the stacktrace and parameters details related to the hooked functions.
Below is shown an example of response containing the custom header filled with the stacktrace of the hooked function executed.
```
< HTTP/1.1 200 OK
< Date: Sun, 07 Jul 2019 19:37:19 GMT
< Server: Apache/2.4.25 (Debian)
< X-Powered-By: PHP/7.2.15
< X-PIOF-IAST: W3sACQkiZnVuY3Rpb25fbmFtZSI6CSJtZDUiLAoJCSJzdGFja3RyYWNlIjoJW3sKCQkJCSJib2R5IjoJImFycmF5IChcbiAnZmlsZScgPT4gJy92YXIvd3d3L2h0bWwvbWQ1LnBocCcsXG4gJ2xpbmUnID0+IDEsXG4gJ2Z1bmN0aW9uJyA9PiAnbWQ1JyxcbiAnYXJncycgPT4gXG4gYXJyYXkgKFxuICAgMCA9PiAnYWRtaW4nLFxuICksXG4pIgoJCQl9XQoJfV0
< Content-Length: 0
< Content-Type: text/html; charset=UTF-8
```
In details the Base64 custom header contains:
```
 $echo "W3sACQkiZnVuY3Rpb25fbmFtZSI6CSJtZDUiLAoJCSJzdGFja3RyYWNlIjoJW3sKCQkJCSJib2R5IjoJImFycmF5IChcbiAnZmlsZScgPT4gJy92YXIvd3d3L2h0bWwvbWQ1LnBocCcsXG4gJ2xpbmUnID0+IDEsXG4gJ2Z1bmN0aW9uJyA9PiAnbWQ1JyxcbiAnYXJncycgPT4gXG4gYXJyYXkgKFxuICAgMCA9PiAnYWRtaW4nLFxuICksXG4pIgoJCQl9XQoJfV0" | base64 -d

[{              "function_name":        "md5",
                "stacktrace":   [{
                                "body": "array (\n 'file' => '/var/www/html/md5.php',\n 'line' => 1,\n 'function' => 'md5',\n 'args' => \n array (\n   0 => 'admin',\n ),\n)"
                        }]
        }]
```
Above stacktrace was returned by following source code executed by hooked `md5` function.
```
$ cat /var/www/html/md5.php 
<?php md5('admin');?>
```


### Useful commands

Script
```
$ cat /var/www/html/test.php 
<?php

function testFunc1(){
    //var_dump(func_get_args());
    testFunc2(9,2,"ciccio", true);

}

function testFunc2(){
    //var_dump(func_get_args());
}

function testFunc3(){
    testFunc1();
}

//testFunc1("test 1");
testFunc2(1,2,"asf", true);
testFunc3("test 3");
md5("admin");
```
Output:

```
$ curl -I --header "X-PIOF-IAST: testFunc1,testFunc2,testFunc3,md5" http://localhost/test.php --silent | grep -Fi X-PIOF-IAST  | cut -d " " -f2 | base64 -d
[{              "function_name":        "testFunc2",
                "stacktrace":   [{
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 11,\n 'function' => 'unknown',\n)"
                        }, {
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 18,\n 'function' => 'testFunc2',\n 'args' => \n array (\n   0 => 1,\n   1 => 2,\n   2 => 'asf',\n   3 => true,\n ),\n)"
                        }]
        }, {
                "function_name":        "testFunc3",
                "stacktrace":   [{
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 14,\n 'function' => 'unknown',\n)"
                        }, {
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 19,\n 'function' => 'testFunc3',\n 'args' => \n array (\n   0 => 'test 3',\n ),\n)"
                        }]
        }, {
                "function_name":        "testFunc1",
                "stacktrace":   [{
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 5,\n 'function' => 'unknown',\n)"
                        }, {
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 14,\n 'function' => 'testFunc1',\n 'args' => \n array (\n ),\n)"
                        }, {
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 19,\n 'function' => 'testFunc3',\n 'args' => \n array (\n   0 => 'test 3',\n ),\n)"
                        }]
        }, {
                "function_name":        "testFunc2",
                "stacktrace":   [{
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 11,\n 'function' => 'unknown',\n)"
                        }, {
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 5,\n 'function' => 'testFunc2',\n 'args' => \n array (\n   0 => 9,\n   1 => 2,\n   2 => 'ciccio',\n   3 => true,\n ),\n)"
                        }, {
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 14,\n 'function' => 'testFunc1',\n 'args' => \n array (\n ),\n)"
                        }, {
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 19,\n 'function' => 'testFunc3',\n 'args' => \n array (\n   0 => 'test 3',\n ),\n)"
                        }]
        }, {
                "function_name":        "md5",
                "stacktrace":   [{
                                "body": "array (\n 'file' => '/var/www/html/test.php',\n 'line' => 20,\n 'function' => 'md5',\n 'args' => \n array (\n   0 => 'admin',\n ),\n)"
                        }]
        }]
```

# License
3-clause BSD license (BSD-3-Clause)

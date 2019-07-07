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

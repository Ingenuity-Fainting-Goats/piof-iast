#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "SAPI.h"
#include "Zend/zend_ini.h"
#include "Zend/zend_smart_str.h"
#include "base64.h"
#include "cJSON.h"
#include "php.h"
#include "piof_config.h"
#include "string.h"
#include "zend_exceptions.h"

#define PIOF_IAST_HEADER "X-PIOF-IAST: "

char *piof_get_ini_value(char *k);
zend_string *get_function_name(zend_string *function_name);
void piof_copy_args(zend_execute_data *execute_data, zval **args,
                    int *ret_num_args);
bool piof_is_function_hooked(char *function_name);
bool piof_find_value_with_separator(char *, char *);
char *trimLeft(char *s);
char *trimRight(char *s);
char *trim(char *s);
bool start_with(const char *a, const char *b);
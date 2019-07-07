#include "piof_globals.h"

bool piof_is_function_hooked(char *function_name) {
  zend_string *name;
  zval *header;
  name =
      zend_string_init("HTTP_X_PIOF_IAST", sizeof("HTTP_X_PIOF_IAST") - 1, 0);
  zval *carrier = NULL, *ret;
  zend_bool jit_initialization = PG(auto_globals_jit);
  if (jit_initialization) {
    zend_string *server_str =
        zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
    zend_is_auto_global(server_str);
    zend_string_release(server_str);
  }
  carrier = &PG(http_globals)[TRACK_VARS_SERVER];
  header = zend_hash_find(Z_ARRVAL_P(carrier), (zend_string *)name);
  zend_string_release(name);
  if (header && Z_TYPE_P(header) == IS_STRING &&
      piof_find_value_with_separator(Z_STRVAL_P(header), function_name))
    return true;
  return false;
}

bool piof_find_value_with_separator(char *string, char *value) {
  char *token = strtok(string, ",");
  while (token != NULL) {
    if (strncasecmp(token, value, strlen(value)) == 0) {
      return true;
    }
    token = strtok(NULL, ",");
  }
  return false;
}

char *trimLeft(char *s) {
  while (isspace(*s)) {
    s++;
  }
  return s;
}

char *trimRight(char *s) {
  int len = strlen(s);
  if (len == 0) {
    return s;
  }
  char *pos = s + len - 1;
  while (pos >= s && isspace(*pos)) {
    *pos = '\0';
    pos--;
  }
  return s;
}

char *trim(char *s) { return trimRight(trimLeft(s)); }

bool start_with(const char *a, const char *b) {
  if (strncmp(a, b, strlen(b)) == 0) return 1;
  return 0;
}

int file_exists(const char *filename) {
  FILE *file;
  if (file = fopen(filename, "r")) {
    fclose(file);
    return 1;
  }
  return 0;
}

void piof_copy_args(zend_execute_data *execute_data, zval **args,
                    int *ret_num_args) {
  int i, num_args = ZEND_CALL_NUM_ARGS(execute_data), has_scope = 0;
  zval *arguments = emalloc((num_args + 1) * sizeof(zval));
  *args = arguments;

  if (getThis() != NULL) {
    has_scope = 1;
    ZVAL_COPY(&arguments[0], getThis());
  }

  for (i = 0; i < num_args; i++) {
    ZVAL_COPY(&arguments[i + has_scope], ZEND_CALL_VAR_NUM(execute_data, i));
  }
  *ret_num_args = num_args + has_scope;
}

char *piof_get_ini_value(char *k) {
  char *v;
  zval *zv;

  v = zend_ini_string(k, strlen(k), 0);

  if (v) return v;

  zv = cfg_get_entry(k, strlen(k));

  if (zv) {
    return Z_STRVAL_P(zv);
  }

  return v;
}

zend_string *get_function_name(zend_string *function_name) {
  zend_string *result;
  if (!function_name) {
    return NULL;
  }

  return zend_string_copy(function_name);
}

#include "piof_globals.h"

void (*original_zend_execute_ex)(zend_execute_data *execute_data);
void (*original_zend_execute_internal)(zend_execute_data *execute_data,
                                       zval *return_value);
zend_op_array *(*old_compile_string)(zval *source_string,
                                     char *filename TSRMLS_DC);
zend_op_array *(*old_compile_file)(zend_file_handle *file_handle, int type);
void piof_zend_execute_ex(zend_execute_data *execute_data TSRMLS_DC);
void piof_zend_execute_internal(INTERNAL_FUNCTION_PARAMETERS);
void piof_debug_backtrace(zend_string *);
cJSON *json_flows = NULL;
zend_class_entry *piof_exception;

void piof_init_exception(TSRMLS_D) {
  zend_class_entry e;
  INIT_CLASS_ENTRY(e, "piofException", NULL);
  piof_exception =
      zend_register_internal_class_ex(&e, zend_exception_get_default());
}

void piof_debug_backtrace(zend_string *function_name) {
  // PIOF_PRINTF("Should print the trace of testFunc2\n");
  // json object con function_name e conterrà array delle call
  cJSON *json_flow = cJSON_CreateObject();
  cJSON *json_function_name = cJSON_CreateString(ZSTR_VAL(function_name));
  cJSON_AddItemToObject(json_flow, "function_name", json_function_name);
  cJSON *json_stacktrace = cJSON_CreateArray();

  zval retval;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  zval *fname;
  int result;
  char *func_name = "debug_backtrace";
  size_t func_name_len = (size_t)strlen(func_name);
  fci.size = sizeof(fci);
  ZVAL_STRINGL(&fci.function_name, func_name, func_name_len);
  fci.retval = &retval;
  fci.object = NULL;
  fci.no_separation = 1;
  fci.param_count = 0;
  result = zend_call_function(&fci, NULL);
  if (result == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
    // PIOF_PRINTF("CALLED debug_backtrace %d - %d\n\n",result,sizeof(retval));
    if (Z_ISREF(retval)) {
      zend_unwrap_reference(&retval);
    } else if (Z_TYPE_P(&retval) == IS_ARRAY) {
      HashTable *array_hash = Z_ARRVAL_P(&retval);
      zend_long lkey;
      zend_string *skey;
      zval *val;
      // json array calls empty
      /**
       * 	Inspired from
       * https://phpinternals.net/docs/zend_hash_foreach_key_val
       *
       */
      ZEND_HASH_FOREACH_KEY_VAL(array_hash, lkey, skey, val) {
        smart_str buf = {0};
        php_var_export_ex(val, 0, &buf);
        smart_str_0(&buf);
        // PHPWRITE(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s)); // logger
        // json add call to array
        cJSON *json_call = cJSON_CreateObject();
        cJSON *json_body = cJSON_CreateString(ZSTR_VAL(buf.s));
        cJSON_AddItemToObject(json_call, "body", json_body);
        cJSON_AddItemToArray(json_stacktrace, json_call);
        smart_str_free(&buf);
      }
      ZEND_HASH_FOREACH_END();
      // Add stacktrace to the single flow
      cJSON_AddItemToObject(json_flow, "stacktrace", json_stacktrace);
    } else {
      // PIOF_PRINTF("else totale\n");
    }
  } else {
    // PIOF_PRINTF("Errore debug_backtrace\n");
  }
  // Add single flow of intercepted function to the list of all intercepted
  // flows
  cJSON_AddItemToArray(json_flows, json_flow);
  // free retval
  zval_ptr_dtor(&retval);
}

ZEND_DECLARE_MODULE_GLOBALS(piof)
zend_module_entry piof_module_entry = {
    STANDARD_MODULE_HEADER,
    PIOF_NAME,
    NULL, /* All exposed functions, only to test POC, will get removed later */
    PHP_MINIT(piof), /* On module startup */
    PHP_MSHUTDOWN(piof),
    PHP_RINIT(piof),
    PHP_RSHUTDOWN(piof),
    NULL, /* Module info, used in phpinfo(); */
    PIOF_VERSION,
    STANDARD_MODULE_PROPERTIES};

ZEND_GET_MODULE(piof)

void piof_zend_execute_ex(zend_execute_data *execute_data) {
  zend_string *function_name =
      get_function_name(EG(current_execute_data)->func->common.function_name);

  if (function_name != NULL &&
      strncasecmp(ZSTR_VAL(function_name), "debug_backtrace",
                  strlen("strncasecmp")) != 0) {
    syslog(LOG_DEBUG | LOG_LOCAL0, "External Execute - %s",
           ZSTR_VAL(function_name));
    piof_debug_backtrace(function_name);
  }

  original_zend_execute_ex(execute_data);
  return;
}

void resume_execute_internal(INTERNAL_FUNCTION_PARAMETERS) {
  if (original_zend_execute_internal) {
    original_zend_execute_internal(INTERNAL_FUNCTION_PARAM_PASSTHRU);
  } else {
    execute_data->func->internal_function.handler(
        INTERNAL_FUNCTION_PARAM_PASSTHRU);
  }
}

void piof_zend_execute_internal(INTERNAL_FUNCTION_PARAMETERS) {
  zend_string *function_name =
      get_function_name(EG(current_execute_data)->func->common.function_name);
  if (function_name != NULL &&
      strncasecmp(ZSTR_VAL(function_name), "debug_backtrace",
                  strlen("strncasecmp")) != 0) {
    syslog(LOG_DEBUG | LOG_LOCAL0, "Internal Execute - %s",
           ZSTR_VAL(function_name));
    piof_debug_backtrace(function_name);
  }

  resume_execute_internal(INTERNAL_FUNCTION_PARAM_PASSTHRU);
  // zend_throw_exception(piof_exception, "piof.c Custom Exception
  // zend_throw_exception", 0L);
}

PHP_MINIT_FUNCTION(piof) {
  openlog("piof - MINIT", LOG_PID, LOG_LOCAL0);
  ALLOC_HASHTABLE(PIOF_G(piof_hooked_functions));
  zend_hash_init(PIOF_G(piof_hooked_functions), 16, NULL, ZVAL_PTR_DTOR, 0);
  json_flows = cJSON_CreateArray();

  original_zend_execute_ex = zend_execute_ex;
  zend_execute_ex = piof_zend_execute_ex;
  syslog(LOG_DEBUG | LOG_LOCAL0, "Hooked zend_execute_ex %p",
         *piof_zend_execute_ex);

  original_zend_execute_internal = zend_execute_internal;
  zend_execute_internal = piof_zend_execute_internal;
  syslog(LOG_DEBUG | LOG_LOCAL0, "Hooked zend_execute_internal %p",
         *piof_zend_execute_internal);
  closelog();
  return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(piof) {
  openlog("piof - MSHUTDOWN", LOG_PID, LOG_LOCAL0);
  /*syslog(LOG_DEBUG | LOG_LOCAL0, "Cleaning hooked functions... ");
  zend_execute_ex = original_zend_execute_ex;
  zend_execute_internal = original_zend_execute_internal;
  zend_compile_string = old_compile_string;
  zend_compile_file = old_compile_file;
  zend_hash_destroy(PIOF_G(piof_hooked_functions));
  FREE_HASHTABLE(PIOF_G(piof_hooked_functions));*/
  closelog();
  return SUCCESS;
}

PHP_RINIT_FUNCTION(piof) {
  openlog("piof - RINIT", LOG_PID, LOG_LOCAL0);
  syslog(LOG_DEBUG | LOG_LOCAL0, "Initialization... ");
  /**
   * @brief SAPI module checker
   *
   */
  syslog(LOG_DEBUG | LOG_LOCAL0, "SAPI Module=%s", sapi_module.name);
  if (strcmp("cli", sapi_module.name) != 0) {
    syslog(LOG_DEBUG | LOG_LOCAL0,
           "SAPI Module=%s - URL=%s - METHOD=%s - URL_QUERY=%s - COOKIE=%s",
           sapi_module.name, SG(request_info).request_uri,
           SG(request_info).request_method, SG(request_info).query_string,
           SG(request_info).cookie_data);
  } else {
  }

  closelog();
  return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(piof) {
  openlog("piof - RSHUTDOWN", LOG_PID, LOG_LOCAL0);
  // Return header with json content
  int json_payload_size = strlen(cJSON_Print(json_flows));
  char *json_payload = (char *)emalloc(json_payload_size * sizeof(char));
  // memset(json_payload,'\0',json_payload_size*sizeof(char));
  json_payload = strtok(cJSON_Print(json_flows),"\n");
  char *b64_json_payload = (char *)emalloc(json_payload_size * sizeof(char));
  // memset(json_payload,'\0',json_payload_size*sizeof(char));
  Base64encode(b64_json_payload, json_payload, json_payload_size);

  syslog(LOG_DEBUG | LOG_LOCAL0, "Payload=%s", b64_json_payload);
  if (strcmp("cli", sapi_module.name) != 0) {
    char *header_payload = (char *)emalloc(
        strlen(b64_json_payload) * sizeof(char) + strlen(PIOF_IAST_HEADER));
    memset(header_payload, '\0', strlen(header_payload) * sizeof(char));
    strcat(header_payload, PIOF_IAST_HEADER);
    strcat(header_payload, b64_json_payload);
    syslog(LOG_DEBUG | LOG_LOCAL0, "Header=%s", header_payload);
    sapi_header_line ctr = {0};
    ctr.line = header_payload;
    ctr.line_len = strlen(header_payload) - 1;
    sapi_header_op(SAPI_HEADER_REPLACE, &ctr TSRMLS_CC);
  }

  closelog();
  return SUCCESS;
}

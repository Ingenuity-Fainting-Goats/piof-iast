#ifndef PIOF_CONFIG_H
#define PIOF_CONFIG_H

#include "php.h"

#define PIOF_NAME "piof-iast"
#define PIOF_VERSION "0.1.0"

#define PIOF_DEBUG 1

#ifndef MYBOOLEAN_H
#define MYBOOLEAN_H
#define false 0
#define true 1
typedef int bool;
#endif

#if PIOF_DEBUG == 1
#define PIOF_PRINTF(...) php_printf(__VA_ARGS__)
#else
#define PIOF_PRINTF(...) ((void)0)
#endif

extern zend_module_entry piof_module_entry;
#define phpext_piof_ptr &piof_module_entry

typedef struct {
  int status;
  char *message;
} pipeline_message;
typedef pipeline_message PipelineMessage;

typedef struct {
  const char *message;
  int result;
  int error;
} ValidatorData;

typedef void t_executor_internal(INTERNAL_FUNCTION_PARAMETERS);

typedef PipelineMessage t_hook(INTERNAL_FUNCTION_PARAMETERS,
                               t_executor_internal e);

typedef zend_op_array *t_hook_compiler(zend_file_handle *file_handle, int type);

typedef zend_op_array *t_hook_string_compiler(zval *source_string,
                                              char *filename TSRMLS_DC);

PHP_MINIT_FUNCTION(piof);
PHP_MSHUTDOWN_FUNCTION(piof);
PHP_RINIT_FUNCTION(piof);
PHP_RSHUTDOWN_FUNCTION(piof);

ZEND_BEGIN_MODULE_GLOBALS(piof)
HashTable *piof_hooked_functions;
ZEND_END_MODULE_GLOBALS(piof)

extern ZEND_DECLARE_MODULE_GLOBALS(piof)

#ifdef ZTS
#define PIOF_G(v) TSRMG(piof_globals_id, zend_piof_globals *, v)
#else
#define PIOF_G(v) (piof_globals.v)
#endif

#define piof_write_output(format, args...)           \
  FILE *output = fopen(PIOF_INTERCEPT_OUTPUT, "a+"); \
  fprintf(output, format, ##args);                   \
  fclose(output);

#endif

#include "php_gmpi.h"
#include "php_ini.h"
#include "ext/standard/info.h"

zend_class_entry *php_gmpi_ce;
ZEND_DECLARE_MODULE_GLOBALS(gmpi);

static void *gmp_alloc(size_t sz) {
	return emalloc(sz);
}

static void *gmp_realloc(void *ptr, size_t old_sz, size_t sz) {
	return erealloc(ptr, sz);
}

static void gmp_free(void *ptr, size_t sz) {
	efree(ptr);
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("gmpi.precision", "48", PHP_INI_ALL, OnUpdateLongGEZero,
		precision, zend_gmpi_globals, gmpi_globals)
	STD_PHP_INI_ENTRY("gmpi.padding", "5", PHP_INI_ALL, OnUpdateLongGEZero,
		padding, zend_gmpi_globals, gmpi_globals)
PHP_INI_END()

ZEND_BEGIN_ARG_INFO_EX(gmpi_binary_op_arginfo, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

static zend_function_entry gmpi_methods[] = {
	PHP_ABSTRACT_ME(GMPi, add, gmpi_binary_op_arginfo)
	PHP_ABSTRACT_ME(GMPi, sub, gmpi_binary_op_arginfo)
	PHP_ABSTRACT_ME(GMPi, div, gmpi_binary_op_arginfo)
	PHP_ABSTRACT_ME(GMPi, mul, gmpi_binary_op_arginfo)
	PHP_ABSTRACT_ME(GMPi, cmp, gmpi_binary_op_arginfo)
	PHP_ABSTRACT_ME(GMPi, neg, NULL)
	PHP_ABSTRACT_ME(GMPi, abs, NULL)

	PHP_ABSTRACT_ME(GMPi, toFloat, NULL)
	PHP_ABSTRACT_ME(GMPi, toInteger, NULL)
	PHP_ABSTRACT_ME(GMPi, __toString, NULL)
	PHP_ABSTRACT_ME(GMPi, __debugInfo, NULL)
	PHP_FE_END
};

/* {{{ MINIT */
static PHP_MINIT_FUNCTION(gmpi) {
	zend_class_entry ce;

	REGISTER_INI_ENTRIES();

	mp_set_memory_functions(gmp_alloc, gmp_realloc, gmp_free);

	INIT_CLASS_ENTRY(ce, "GMPi", gmpi_methods);
	php_gmpi_ce = zend_register_internal_class(&ce);
	php_gmpi_ce->ce_flags |= ZEND_ACC_INTERFACE;

	return
		((FAILURE == PHP_MINIT(gmpi_int)(INIT_FUNC_ARGS_PASSTHRU)) ||
		 (FAILURE == PHP_MINIT(gmpi_float)(INIT_FUNC_ARGS_PASSTHRU)))
		? FAILURE : SUCCESS;
} /* }}} */

/* {{{ MSHUTDOWN */
static PHP_MSHUTDOWN_FUNCTION(gmpi) {
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ GINIT */
static PHP_GINIT_FUNCTION(gmpi)
{
#if defined(COMPILE_DL_GMPI) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	gmpi_globals->precision = 48;
	gmpi_globals->padding = 5;
}
/* }}} */

/* {{{ MINFO */
static PHP_MINFO_FUNCTION(gmpi) {
	php_info_print_table_start();
	php_info_print_table_row(2, "gmpi support", "enabled");
	php_info_print_table_row(2, "libgmp version", gmp_version);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
} /* }}} */

/* {{{ gmpi_module_entry
 */
#ifdef COMPILE_DL_GMPI
static
#endif
zend_module_entry gmpi_module_entry = {
	STANDARD_MODULE_HEADER,
	"gmpi",
	NULL, /* functions */
	PHP_MINIT(gmpi),
	NULL, /* MSHUTDOWN */
	NULL, /* RINIT */
	NULL, /* RSHUTDOWN */
	PHP_MINFO(gmpi),
	"1.0.0",
	PHP_MODULE_GLOBALS(gmpi),
	PHP_GINIT(gmpi),
	NULL, /* GSHUTDOWN */
	NULL, /* RPOSTSHUTDOWN */
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_GMPI
ZEND_GET_MODULE(gmpi)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

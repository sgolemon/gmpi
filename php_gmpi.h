#ifndef PHP_GMPI_H
#define PHP_GMPI_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include <gmp.h>

extern zend_class_entry *php_gmpi_ce;
extern zend_class_entry *php_gmpint_ce;
extern zend_class_entry *php_gmpfloat_ce;

typedef struct _php_gmpint_object {
	mpz_t num;
	zend_object std;
} php_gmpint_object;

static inline php_gmpint_object*
php_gmpint_object_from_zend_object(zend_object *obj) {
	return (php_gmpint_object*)(
		((char*)obj) - XtOffsetOf(php_gmpint_object, std)
	);
}

typedef struct _php_gmpfloat_object {
	mpf_t num;
	zend_object std;
} php_gmpfloat_object;

static inline php_gmpfloat_object*
php_gmpfloat_object_from_zend_object(zend_object *obj) {
	return (php_gmpfloat_object*)(
		((char*)obj) - XtOffsetOf(php_gmpfloat_object, std)
	);
}

PHP_MINIT_FUNCTION(gmpi_int);
PHP_MINIT_FUNCTION(gmpi_float);

zend_string* gmpint_to_string(const mpz_t m1, int base);
int gmpfloat_do_operation(zend_uchar, zval*, zval*, zval*);

ZEND_BEGIN_MODULE_GLOBALS(gmpi)
	zend_long precision;
	zend_long padding;
ZEND_END_MODULE_GLOBALS(gmpi)

ZEND_EXTERN_MODULE_GLOBALS(gmpi)

#if defined(ZTS) && defined(COMPILE_DL_GMPI)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#define GMPIG(v) ZEND_MODULE_GLOBALS_ACCESSOR(gmpi, v)

#ifndef COMPILE_DL_GMPI
extern zend_module_entry gmpi_module_entry;
# define phpext_gmpi_ptr &gmpi_module_entry
#endif

#endif	/* PHP_GMPI_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */

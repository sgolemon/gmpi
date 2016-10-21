#include "php_gmpi.h"
#include "zend_exceptions.h"
#include "zend_inheritance.h"

zend_class_entry *php_gmpfloat_ce;
static zend_object_handlers handlers;

static inline void gmpfloat_throw_number_type_error(zval *arg) {
	zend_throw_exception(zend_ce_type_error,
		"Expected instance of GMPi\\Integer, GMPi\\Float, int, float, "
		"or a numeric string", 0);
}

static inline int gmpfloat_ui_helper(mpf_t mr, mpf_t m1, mpf_t m2,
                    void (*op)(mpf_t, const mpf_t, unsigned long int)) {
	if ((mpz_sgn(m2) < 0) || !mpf_fits_ulong_p(m2)) {
		zend_throw_exception(zend_ce_type_error,
			"exponent operation required non-negative exponent which fits "
			"within PHP_INT_MAX", 0);
		return FAILURE;
	}
	op(mr, m1, mpf_get_ui(m2));
	return SUCCESS;
}

static zend_string* gmpfloat_to_string(mpf_t num, int base, int padlimit) {
	int sgn = mpf_sgn(num);
	zend_bool neg = sgn < 0;
	int slen = neg ? 1 : 0;
	mp_exp_t exp;
	char *mantissa, *p;
	size_t mlen;
	zend_string *ret;

	if ((base < 2) || (base > 62)) {
		zend_throw_exception_ex(zend_ce_type_error, 0,
			"Base must be in the range 2..62, %d given", base);
		return ZSTR_EMPTY_ALLOC();
	}

	if (sgn == 0) {
		/* Special case zero */
		return zend_string_init("0", 1, 0);
	}

	mantissa = mpf_get_str(NULL, &exp, base, 0, num) + slen;
	mlen = strlen(mantissa);

	if ((exp <= 0) && ((-exp) <= padlimit)) {
		/* 0.1234E-2 == 0.001234 */
		ret = zend_string_alloc(slen + 2 + (mlen - exp), 0);
		p = ZSTR_VAL(ret);
		if (neg) *(p++) = '-';
		*(p++) = '0';
		*(p++) = '.';
		memset(p, '0', -exp);
		p -= exp;
		memcpy(p, mantissa, mlen);
		p[mlen] = 0;
	} else if ((exp > 0) && (exp < mlen)) {
		/* 0.1234E+2 == 12.34 */
		ret = zend_string_alloc(slen + mlen + 1, 0);
		p = ZSTR_VAL(ret);
		if (neg) *(p++) = '-';
		memcpy(p, mantissa, exp);
		p += exp;
		*(p++) = '.';
		memcpy(p, mantissa + exp, mlen - exp);
		p[mlen - exp] = 0;
	} else if ((exp >= mlen) && ((exp - mlen) <= padlimit)) {
		/* 0.1234E+4 == 1234 */
		/* 0.1234E+6 == 123400 */
		ret = zend_string_alloc(slen + exp, 0);
		p = ZSTR_VAL(ret);
		if (neg) *(p++) = '-';
		memcpy(p, mantissa, mlen);
		p += mlen;
		memset(p, '0', exp - mlen);
		p[exp - mlen] = 0;
	} else {
		/* Else, scientific notation */
		char expstr[40];
		size_t elen = snprintf(expstr, sizeof(expstr), "%+d", (int)(exp - 1));
		ret = zend_string_alloc(slen + mlen + 2 + elen, 0);
		p = ZSTR_VAL(ret);
		if (neg) *(p++) = '-';
		*(p++) = mantissa[0];
		if (mlen > 1) {
			*(p++) = '.';
			memcpy(p, mantissa + 1, mlen - 1);
			p += mlen - 1;
		}
		*(p++) = 'E';
		memcpy(p, expstr, elen);
		p[elen] = 0;
	}

	efree(mantissa - slen);
	return ret;
}

/* Emulate base discovery for octals, hex, and binary */
static int gmpfloat_set_str(mpf_t mr, const char *val, int base) {
	if ((base == 2) && (val[0] == '0') &&
		((val[1] == 'b') || (val[1] == 'B'))) {
		return mpf_set_str(mr, val + 2, base);
	}
	if ((base == 16) && (val[0] == '0') &&
		((val[1] == 'x') || (val[1] == 'X'))) {
		return mpf_set_str(mr, val + 2, base);
	}

	if (base || !val[0] || !val[1] || (val[0] != '0')) {
		return mpf_set_str(mr, val, base);
	}
	if ((val[1] == 'x') || (val[1] == 'X')) {
		return mpf_set_str(mr, val + 2, 16);
	}
	if ((val[1] == 'b') || (val[1] == 'B')) {
		return mpf_set_str(mr, val + 2, 2);
	}
	return mpf_set_str(mr, val + 1, 8);
}

static int gmpfloat_get_float(mpf_t mr, zval *val) {
	switch (Z_TYPE_P(val)) {
		case IS_LONG:
			mpf_set_si(mr, Z_LVAL_P(val));
			return SUCCESS;
		case IS_DOUBLE:
			mpf_set_d(mr, Z_DVAL_P(val));
			return SUCCESS;
		case IS_STRING:
			/* Try float parsing first */
			if (gmpfloat_set_str(mr, Z_STRVAL_P(val), 0)) {
				return FAILURE;
			}
			return SUCCESS;
		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(val), php_gmpfloat_ce)) {
				mpf_set(mr,
					php_gmpfloat_object_from_zend_object(Z_OBJ_P(val))->num);
				return SUCCESS;
			} else if (instanceof_function(Z_OBJCE_P(val), php_gmpint_ce)) {
				mpf_set_z(mr,
					php_gmpint_object_from_zend_object(Z_OBJ_P(val))->num);
				return SUCCESS;
			} else {
				ZEND_ASSERT(0);
				/* gmpfloat_normalize_object() should prevent this */
				return FAILURE;
			}

		default:
			return FAILURE;
	}
}

static zval* gmpfloat_normalize_object(zval *val, zval *tmp) {
	if (Z_TYPE_P(val) != IS_OBJECT) return val;
	if (instanceof_function(Z_OBJCE_P(val), php_gmpint_ce) ||
		instanceof_function(Z_OBJCE_P(val), php_gmpfloat_ce)) {
		return val;
	}
	ZVAL_ZVAL(tmp, val, 1, 0);
	convert_to_string(tmp);
	return tmp;
}

int gmpfloat_do_operation(zend_uchar op, zval *return_value,
	                      zval *op1, zval *op2) {
	zval tmp1, tmp2;
	mpf_t m1, m2;
	zval result;

	mpf_init2(m1, GMPIG(precision));
	ZVAL_UNDEF(&tmp1);
	op1 = gmpfloat_normalize_object(op1, &tmp1);
	if (FAILURE == gmpfloat_get_float(m1, op1)) {
		mpf_clear(m1);
		gmpfloat_throw_number_type_error(op1);
		return FAILURE;
	}
	zval_dtor(&tmp1);

	mpf_init2(m2, GMPIG(precision));
	ZVAL_UNDEF(&tmp2);
	op2 = gmpfloat_normalize_object(op2, &tmp2);
	if (FAILURE == gmpfloat_get_float(m2, op2)) {
		mpf_clears(m1, m2, NULL);
		gmpfloat_throw_number_type_error(op2);
		return FAILURE;
	}
	zval_dtor(&tmp2);

	if (op == ZEND_SPACESHIP) {
		RETVAL_LONG(mpf_cmp(m1, m2));
		mpf_clears(m1, m2, NULL);
		return SUCCESS;
	}

	object_init_ex(&result, php_gmpfloat_ce);
#define mr php_gmpfloat_object_from_zend_object(Z_OBJ(result))->num
	switch (op) {
		case ZEND_ADD: mpf_add(mr, m1, m2); break;
		case ZEND_SUB: mpf_sub(mr, m1, m2); break;
		case ZEND_MUL: mpf_mul(mr, m1, m2); break;
		case ZEND_DIV: mpf_div(mr, m1, m2); break;
		case ZEND_POW:
			if (FAILURE == gmpfloat_ui_helper(mr, m1, m2, mpf_pow_ui)) {
				goto failure;
			}
			break;

		/* Supportable Integer operations */
		case ZEND_SL:
			if (FAILURE == gmpfloat_ui_helper(mr, m1, m2, mpf_mul_2exp)) {
				goto failure;
			}
			break;
		case ZEND_SR:
			if (FAILURE == gmpfloat_ui_helper(mr, m1, m2, mpf_div_2exp)) {
				goto failure;
			}
			break;

		/* Unsupportable Integer operations */
		case ZEND_BW_OR:
		case ZEND_BW_AND:
		case ZEND_BW_XOR:
		case ZEND_BW_NOT:
		case ZEND_MOD:
		default:
failure:
			zval_dtor(&result);
			mpf_clears(m1, m2, NULL);
			return FAILURE;
	}
#undef mr
	mpf_clears(m1, m2, NULL);
	zval_dtor(return_value);
	ZVAL_ZVAL(return_value, &result, 0, 0);
	return SUCCESS;
}

/***************************************************************************/

/* {{{ proto void GMPi\Float::__construct(mixed $val[, int $base = 0
                          [, int $precision = ini_get('gmpi.precision')]]) */
ZEND_BEGIN_ARG_INFO_EX(gmpfloat_ctor_arginfo, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, val)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, precision)
ZEND_END_ARG_INFO();
static PHP_METHOD(Float, __construct) {
	php_gmpfloat_object *objval =
		php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()));
	zval *val, tmp;
	zend_long base = 0, precision = GMPIG(precision);

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z|ll",
	                                &val, &base, &precision) == FAILURE) {
		return;
	}


	if (base && ((base < 2) || (base > 62))) {
		zend_throw_exception_ex(zend_ce_type_error, 0,
			"Base must be in the range 2..62, %ld given", base);
		return;
	}
    if (precision < 1) {
		zend_throw_exception_ex(zend_ce_type_error, 0,
			"Precision must be at least 1, %ld given", precision);
		return;
	}

	ZVAL_UNDEF(&tmp);
	mpf_set_prec(objval->num, precision);
	switch (Z_TYPE_P(val)) {
		case IS_LONG:
			mpf_set_si(objval->num, Z_LVAL_P(val));
			break;
		case IS_DOUBLE:
			mpf_set_d(objval->num, Z_DVAL_P(val));
			break;
		case IS_OBJECT:
			ZVAL_ZVAL(&tmp, val, 1, 0);
			convert_to_string(&tmp);
			val = &tmp;
			/* fallthrough */
		case IS_STRING:
			if (gmpfloat_set_str(objval->num, Z_STRVAL_P(val), base)) {
				gmpfloat_throw_number_type_error(val);
			}
			break;
	}
	zval_dtor(&tmp);
}
/* }}} */


ZEND_BEGIN_ARG_INFO_EX(gmpfloat_binary_op_arginfo, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

/* {{{ Float GMPi\Float::op(Integer|Float $value) */
#define GMPFLOAT_BINARY_OP(method, op) \
static PHP_METHOD(Float, method) { \
    zval *arg; \
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z", &arg) == FAILURE) { \
        return; \
    } \
    gmpfloat_do_operation(op, return_value, getThis(), arg); \
}
GMPFLOAT_BINARY_OP(add, ZEND_ADD)
GMPFLOAT_BINARY_OP(sub, ZEND_SUB)
GMPFLOAT_BINARY_OP(mul, ZEND_MUL)
GMPFLOAT_BINARY_OP(div, ZEND_DIV)
GMPFLOAT_BINARY_OP(pow, ZEND_POW)
GMPFLOAT_BINARY_OP(cmp, ZEND_SPACESHIP)
#undef GMPFLOAT_BINARY_OP
/* }}} */

/* {{{ proto Float GMPi\Float::neg() */
static PHP_METHOD(Float, neg) {
    object_init_ex(return_value, php_gmpfloat_ce);
    mpf_neg(php_gmpfloat_object_from_zend_object(Z_OBJ_P(return_value))->num,
            php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()))->num);
}
/* }}} */

/* {{{ proto Float GMPi\Float::abs() */
static PHP_METHOD(Float, abs) {
    object_init_ex(return_value, php_gmpfloat_ce);
    mpf_abs(php_gmpfloat_object_from_zend_object(Z_OBJ_P(return_value))->num,
            php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()))->num);
}
/* }}} */

/* {{{ proto array GMPi\Float::toParts([int $base = 10]) */
ZEND_BEGIN_ARG_INFO_EX(gmpfloat_toparts_arginfo, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO();
static PHP_METHOD(Float, toParts) {
	php_gmpfloat_object *objval =
		php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()));
	zend_long base = 10;
	char *mantissa, *p, *m;
	int mlen;
	zend_string *mstr;
	mp_exp_t exp;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|l", &base) == FAILURE) {
		return;
	}

	if ((base < 2) || (base > 62)) {
		zend_throw_exception_ex(zend_ce_type_error, 0,
			"Base must be in the range 2..62, %d given", base);
		return;
	}

	if (!mpf_sgn(objval->num)) {
		array_init(return_value);
		add_assoc_string(return_value, "mantissa", "0");
		add_assoc_long(return_value, "exponent", 0);
		return;
	}

	mantissa = mpf_get_str(NULL, &exp, base, 0,
		php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()))->num);

	/* GMP's mantissa/exponent format is a bit weird */
	mlen = strlen(mantissa);
	mstr = zend_string_alloc(mlen + 2, 0);
	p = ZSTR_VAL(mstr);
	m = mantissa;
	if (m[0] == '-') {
		*(p++) = '-';
		++m;
		--mlen;
	}
	*(p++) = '0';
	*(p++) = '.';
	memcpy(p, m, mlen + 1);
	efree(mantissa);

	array_init(return_value);
	add_assoc_str(return_value, "mantissa", mstr);
	add_assoc_long(return_value, "exponent", exp);
}

/* {{{ proto Integer GMPi\Float::toInteger() */
static PHP_METHOD(Float, toInteger) {
	object_init_ex(return_value, php_gmpint_ce);
	mpz_set_f(php_gmpint_object_from_zend_object(Z_OBJ_P(return_value))->num,
	          php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()))->num);
}
/* }}} */

/* {{{ proto Float GMPi\Float::toFloat() */
static PHP_METHOD(Float, toFloat) {
	ZVAL_ZVAL(return_value, getThis(), 1, 0);
}
/* }}} */

/* {{{ proto string GMPi\Float::toString([int $base = 10[, int $pad = 5]]) */
ZEND_BEGIN_ARG_INFO_EX(gmpfloat_tostring_arginfo, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, padlimit)
ZEND_END_ARG_INFO();
static PHP_METHOD(Float, toString) {
	php_gmpfloat_object *objval =
		php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()));
	zend_long base = 10, padlimit = GMPIG(padding);

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(),
	                                "|ll", &base, &padlimit) == FAILURE) {
		return;
	}

	RETURN_STR(gmpfloat_to_string(objval->num, base, padlimit));
}
/* }}} */

/* {{{ proto string GMPi\Float::__toString() */
static PHP_METHOD(Float, __toString) {
	php_gmpfloat_object *objval =
		php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()));
	RETURN_STR(gmpfloat_to_string(objval->num, 10, GMPIG(padding)));
}
/* }}} */

/* {{{ proto string GMPi\Float::__debugInfo() */
static PHP_METHOD(Float, __debugInfo) {
	php_gmpfloat_object *objval =
		php_gmpfloat_object_from_zend_object(Z_OBJ_P(getThis()));

	array_init(return_value);
	add_assoc_str(return_value, "dec",
		gmpfloat_to_string(objval->num, 10, GMPIG(padding)));
	add_assoc_str(return_value, "hex",
		gmpfloat_to_string(objval->num, 16, GMPIG(padding)));
}
/* }}} */

static zend_function_entry gmpfloat_methods[] = {
	PHP_ME(Float, __construct, gmpfloat_ctor_arginfo,
		ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

	PHP_ME(Float, add, gmpfloat_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Float, sub, gmpfloat_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Float, mul, gmpfloat_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Float, div, gmpfloat_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Float, pow, gmpfloat_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Float, cmp, gmpfloat_binary_op_arginfo, ZEND_ACC_PUBLIC)

	PHP_ME(Float, neg, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Float, abs, NULL, ZEND_ACC_PUBLIC)

	PHP_ME(Float, toParts, gmpfloat_toparts_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Float, toInteger, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Float, toFloat, NULL, ZEND_ACC_PUBLIC)

    PHP_ME(Float, toString, gmpfloat_tostring_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Float, __toString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Float, __debugInfo, NULL, ZEND_ACC_PUBLIC)

	PHP_FE_END
};

/***************************************************************************/

static zend_object* gmpfloat_create_object(zend_class_entry *ce,
	                                     php_gmpfloat_object **pobjval) {
	php_gmpfloat_object *objval = ecalloc(1,
	    sizeof(php_gmpfloat_object) + zend_object_properties_size(ce));
	zend_object *zobj = &(objval->std);

	zend_object_std_init(zobj, ce);
	zobj->handlers = &handlers;
	mpf_init2(objval->num, GMPIG(precision));

	if (pobjval) { *pobjval = objval; }
	return zobj;
}

static zend_object* gmpfloat_ctor(zend_class_entry *ce) {
	return gmpfloat_create_object(ce, NULL);
}

static zend_object* gmpfloat_clone(zval *obj) {
	zend_object *zold = Z_OBJ_P(obj);
	php_gmpfloat_object *oldobj = php_gmpfloat_object_from_zend_object(zold);
	php_gmpfloat_object *newobj;
	zend_object *znew = gmpfloat_create_object(Z_OBJCE_P(obj), &newobj);

	mpf_set(newobj->num, oldobj->num);
	zend_objects_clone_members(znew, zold);
	return znew;
}

static void gmpfloat_free(zend_object *obj) {
	mpf_clear(php_gmpfloat_object_from_zend_object(obj)->num);
}


PHP_MINIT_FUNCTION(gmpi_float) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "GMPi\\Float", gmpfloat_methods);
	php_gmpfloat_ce = zend_register_internal_class(&ce);
	php_gmpfloat_ce->create_object = gmpfloat_ctor;
	zend_do_implement_interface(php_gmpfloat_ce, php_gmpi_ce);

	memcpy(&handlers, zend_get_std_object_handlers(),
	       sizeof(zend_object_handlers));
	handlers.offset = XtOffsetOf(php_gmpfloat_object, std);
	handlers.clone_obj = gmpfloat_clone;
	handlers.free_obj = gmpfloat_free;
	handlers.do_operation = gmpfloat_do_operation;

	return SUCCESS;
}

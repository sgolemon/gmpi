#include "php_gmpi.h"
#include "zend_exceptions.h"
#include "zend_inheritance.h"

zend_class_entry *php_gmpint_ce;
static zend_object_handlers handlers;

/***************************************************************************/

static inline void gmpint_throw_int_type_error(zval *arg) {
	zend_throw_exception(zend_ce_type_error,
		"Expected instance of GMPi\\Integer or int, "
		"or a numeric string", 0);
}

static inline void gmpint_throw_number_type_error(zval *arg) {
	zend_throw_exception(zend_ce_type_error,
		"Expected instance of GMPi\\Integer, GMPi\\Float, int, float, "
		"or a numeric string", 0);
}

static inline int gmpint_ui_helper(mpz_t mr, mpz_t m1, mpz_t m2,
                    void (*op)(mpz_t, const mpz_t, unsigned long int)) {
	if ((mpz_sgn(m2) < 0) || !mpz_fits_ulong_p(m2)) {
		zend_throw_exception(zend_ce_type_error,
			"exponent operation required non-negative exponent which fits "
			"within PHP_INT_MAX", 0);
		return FAILURE;
	}
	op(mr, m1, mpz_get_ui(m2));
	return SUCCESS;
}

static int gmpint_get_int(mpz_t mr, zval *val) {
	switch (Z_TYPE_P(val)) {
		case IS_LONG:
			mpz_set_si(mr, Z_LVAL_P(val));
			return SUCCESS;

		case IS_STRING:
			if (mpz_set_str(mr, Z_STRVAL_P(val), 0)) {
				return FAILURE;
			}
			return SUCCESS;

		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(val), php_gmpint_ce)) {
				mpz_set(mr,
					php_gmpint_object_from_zend_object(Z_OBJ_P(val))->num);
				return SUCCESS;
			} else if (instanceof_function(Z_OBJCE_P(val), php_gmpfloat_ce)) {
				return FAILURE;
			} else {
				ZEND_ASSERT(0);
				/* gmpint_normalize_object() should prevent this */
				return FAILURE;
			}

		default:
			return FAILURE;
	}
}

static zval* gmpint_normalize_object(zval *val, zval *tmp) {
	if (Z_TYPE_P(val) != IS_OBJECT) return val;
	if (instanceof_function(Z_OBJCE_P(val), php_gmpint_ce) ||
	    instanceof_function(Z_OBJCE_P(val), php_gmpfloat_ce)) {
		return val;
	}
	ZVAL_ZVAL(tmp, val, 1, 0);
	convert_to_string(tmp);
	return tmp;
}

static int gmpint_do_operation(zend_uchar op,
                               zval *return_value, zval *op1, zval *op2) {
	zval tmp1, tmp2;
	mpz_t m1, m2;
	zval result;

	if (op == ZEND_DIV) {
		return gmpfloat_do_operation(op, return_value, op1, op2);
	}

	ZVAL_UNDEF(&tmp1);
	op1 = gmpint_normalize_object(op1, &tmp1);
	ZVAL_UNDEF(&tmp2);
	op2 = gmpint_normalize_object(op2, &tmp2);

	mpz_inits(m1, m2, NULL);
	if ((FAILURE == gmpint_get_int(m1, op1)) ||
	    (FAILURE == gmpint_get_int(m2, op2))) {
		int ret = gmpfloat_do_operation(op, return_value, op1, op2);
		mpz_clears(m1, m2, NULL);
		zval_dtor(&tmp1);
		zval_dtor(&tmp2);
		return ret;
	}
	zval_dtor(&tmp1);
	zval_dtor(&tmp2);

	if (op == ZEND_SPACESHIP) {
		RETVAL_LONG(mpz_cmp(m1, m2));
		mpz_clears(m1, m2, NULL);
		return SUCCESS;
	}

	object_init_ex(&result, php_gmpint_ce);
#define mr php_gmpint_object_from_zend_object(Z_OBJ(result))->num
	switch (op) {
		case ZEND_ADD:     mpz_add(mr, m1, m2); break;
		case ZEND_SUB:     mpz_sub(mr, m1, m2); break;
		case ZEND_MUL:     mpz_mul(mr, m1, m2); break;
		case ZEND_POW:
			if (FAILURE == gmpint_ui_helper(mr, m1, m2, mpz_pow_ui)) {
				goto failure;
			}
			break;
		case ZEND_MOD:     mpz_mod(mr, m1, m2); break;
		case ZEND_SL:
			if (FAILURE == gmpint_ui_helper(mr, m1, m2, mpz_mul_2exp)) {
				goto failure;
			}
			break;
		case ZEND_SR:
			if (FAILURE == gmpint_ui_helper(mr, m1, m2, mpz_fdiv_q_2exp)) {
				goto failure;
			}
			break;
		case ZEND_BW_OR:   mpz_ior(mr, m1, m2); break;
		case ZEND_BW_AND:  mpz_and(mr, m1, m2); break;
		case ZEND_BW_XOR:  mpz_xor(mr, m1, m2); break;
		case ZEND_BW_NOT:  mpz_com(mr, m1);     break;

		default:
failure:
			zval_dtor(&result);
			mpz_clears(m1, m2, NULL);
			RETVAL_NULL();
			return FAILURE;
	}
#undef mr
	mpz_clears(m1, m2, NULL);
	zval_dtor(return_value);
	ZVAL_ZVAL(return_value, &result, 0, 0);
	return SUCCESS;
}

static int gmpint_do_compare(zval *result, zval *op1, zval *op2) {
	return gmpint_do_operation(ZEND_SPACESHIP, result, op1, op2);
}

zend_string* gmpint_to_string(const mpz_t m1, int base) {
	zend_string *ret;
	if ((base < 2) || (base > 62)) {
		zend_throw_exception_ex(zend_ce_type_error, 0,
			"Base must be in the range 2..62, %d given", base);
		return ZSTR_EMPTY_ALLOC();
	}
	ret = zend_string_alloc(mpz_sizeinbase(m1, base) + 2, 0);
	mpz_get_str(ZSTR_VAL(ret), base, m1);
	ZSTR_LEN(ret) = strlen(ZSTR_VAL(ret));
	return ret;
}

static int gmpint_parse_integer_arg(mpz_t mr, zval *arg, int argnum) {
	switch (Z_TYPE_P(arg)) {
		case IS_LONG:
			mpz_set_si(mr, Z_LVAL_P(arg));
			return SUCCESS;
		case IS_STRING:
			if (!mpz_set_str(mr, Z_STRVAL_P(arg), 0)) {
				return SUCCESS;
			}
			break;
		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(arg), php_gmpint_ce)) {
				mpz_set(mr, php_gmpint_object_from_zend_object(Z_OBJ_P(arg))->num);
				return SUCCESS;
			} else {
				zval tmp;
				int ret;
				ZVAL_ZVAL(&tmp, arg, 1, 0);
				convert_to_string(&tmp);
				ret = gmpint_parse_integer_arg(mr, &tmp, argnum);
				zval_dtor(&tmp);
				return ret;
			}
			break;
	}
	gmpint_throw_int_type_error(arg);
	return FAILURE;
}

/***************************************************************************/

/* {{{ proto void GMPi\Integer::__construct(mixed $value[, int $base = 0]) */
ZEND_BEGIN_ARG_INFO_EX(gmpint_ctor_arginfo, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO();
static PHP_METHOD(Integer, __construct) {
	php_gmpint_object *objval =
		php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()));
	zval *arg;
	zend_long base = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z|l",
	                                &arg, &base) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(arg) == IS_LONG) {
		mpz_set_si(objval->num, Z_LVAL_P(arg));
	} else if (Z_TYPE_P(arg) == IS_STRING) {
		if (base && ((base < 2) || base > 62)) {
			zend_throw_exception_ex(zend_ce_type_error, 0,
				"Base must be in the range 2..62 or 0, %d given", (int)base);
		}
		if (mpz_set_str(objval->num, Z_STRVAL_P(arg), base)) {
			zend_throw_exception_ex(zend_ce_type_error, 0,
				"Initial value invalid for provided base: %d", (int)base);
		}
	} else {
		zend_throw_exception_ex(zend_ce_type_error, 0,
			"GMPi\\Integer::__construct expects an integer or numeric string, "
			"%s provided", zend_get_type_by_const(Z_TYPE_P(arg)));
	}
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(gmpint_binary_op_arginfo, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

/* {{{ Integer|Float GMPi\Integer::op(Integer|Float $value) */
#define GMPINT_BINARY_OP(method, op) \
static PHP_METHOD(Integer, method) { \
	zval *arg; \
	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z", &arg) == FAILURE) { \
		return; \
	} \
	gmpint_do_operation(op, return_value, getThis(), arg); \
}
GMPINT_BINARY_OP(add, ZEND_ADD)
GMPINT_BINARY_OP(sub, ZEND_SUB)
GMPINT_BINARY_OP(mul, ZEND_MUL)
GMPINT_BINARY_OP(div, ZEND_DIV)
GMPINT_BINARY_OP(pow, ZEND_POW)
GMPINT_BINARY_OP(mod, ZEND_MOD)
GMPINT_BINARY_OP(and, ZEND_BW_AND)
GMPINT_BINARY_OP(or,  ZEND_BW_OR)
GMPINT_BINARY_OP(xor, ZEND_BW_XOR)
GMPINT_BINARY_OP(cmp, ZEND_SPACESHIP)
#undef GMPINT_BINARY_OP
/* }}} */

/* {{{ proto Integer GMPi\Integer::neg() */
static PHP_METHOD(Integer, neg) {
	object_init_ex(return_value, php_gmpint_ce);
	mpz_neg(php_gmpint_object_from_zend_object(Z_OBJ_P(return_value))->num,
	        php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()))->num);
}
/* }}} */

/* {{{ proto Integer GMPi\Integer::abs() */
static PHP_METHOD(Integer, abs) {
	object_init_ex(return_value, php_gmpint_ce);
	mpz_abs(php_gmpint_object_from_zend_object(Z_OBJ_P(return_value))->num,
	        php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()))->num);
}
/* }}} */

#define TRINARY_OP_POWM 1
#define TRINARY_OP_CONGRUENT 2
static void do_trinary_op(INTERNAL_FUNCTION_PARAMETERS, zend_uchar op) {
	zval *valarg, *modarg;
	mpz_t val, mod;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(),
	                                "zz", &valarg, &modarg) == FAILURE) {
		return;
	}

	mpz_inits(val, mod, NULL);
	if ((FAILURE == gmpint_parse_integer_arg(val, valarg, 1)) ||
	    (FAILURE == gmpint_parse_integer_arg(mod, modarg, 2))) {
		mpz_clears(val, mod, NULL);
		RETURN_NULL();
	}

	switch (op) {
		case TRINARY_OP_POWM:
			object_init_ex(return_value, php_gmpint_ce);
			mpz_powm(php_gmpint_object_from_zend_object(Z_OBJ_P(return_value))->num,
					 php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()))->num,
					 val, mod);
			break;
		case TRINARY_OP_CONGRUENT:
			RETVAL_BOOL(mpz_congruent_p(
				php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()))->num,
			    val, mod)
			);
			break;
	}
	mpz_clears(val, mod, NULL);
}

/* {{{ proto Integer GMPi\Integer::powMod(Integer $exp, Integer $mod) */
ZEND_BEGIN_ARG_INFO_EX(gmpint_powmod_arginfo, 0, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, exp)
	ZEND_ARG_INFO(0, mod)
ZEND_END_ARG_INFO();
static PHP_METHOD(Integer, powMod) {
	do_trinary_op(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRINARY_OP_POWM);
}
/* }}} */

/* {{{ proto bool GMPi\Integer::congruent(Integer $val, Integer $mod) */
ZEND_BEGIN_ARG_INFO_EX(gmpint_congruent_arginfo, 0, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, mod)
ZEND_END_ARG_INFO();
static PHP_METHOD(Integer, congruent) {
	do_trinary_op(INTERNAL_FUNCTION_PARAM_PASSTHRU, TRINARY_OP_CONGRUENT);
}
/* }}} */

/* {{{ proto Float GMPi\Integer::toFloat() */
static PHP_METHOD(Integer, toFloat) {
	object_init_ex(return_value, php_gmpfloat_ce);
	mpf_set_z(php_gmpfloat_object_from_zend_object(Z_OBJ_P(return_value))->num,
              php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()))->num);
}
/* }}} */

/* {{{ proto Integer GMPi\Integer::toInteger() */
static PHP_METHOD(Integer, toInteger) {
	ZVAL_ZVAL(return_value, getThis(), 1, 0);
}
/* }}} */

/* {{{ proto string GMPi\Integer::toString([int $base = 10]) */
ZEND_BEGIN_ARG_INFO_EX(gmpint_tostring_arginfo, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO();
static PHP_METHOD(Integer, toString) {
	zend_long base = 10;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|l", &base) == FAILURE) {
		return;
	}

	RETURN_STR(gmpint_to_string(
		php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()))->num, base));
}
/* }}} */

/* {{{ proto string GMPi\Integer::__toString() */
static PHP_METHOD(Integer, __toString) {
	RETURN_STR(gmpint_to_string(
		php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()))->num, 10));
}
/* }}} */

/* {{{ proto array GMPi\Integer::__debugInfo() */
static PHP_METHOD(Integer, __debugInfo) {
	php_gmpint_object *objval =
		php_gmpint_object_from_zend_object(Z_OBJ_P(getThis()));

	array_init(return_value);
	add_assoc_str(return_value, "dec", gmpint_to_string(objval->num, 10));
	add_assoc_str(return_value, "hex", gmpint_to_string(objval->num, 16));
}
/* }}} */

static zend_function_entry gmpint_methods[] = {
	PHP_ME(Integer, __construct, gmpint_ctor_arginfo,
	       ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

	PHP_ME(Integer, add, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, sub, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, mul, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, div, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, pow, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, mod, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, and, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, or,  gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, xor, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)

	PHP_ME(Integer, cmp, gmpint_binary_op_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, neg, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, abs, NULL, ZEND_ACC_PUBLIC)

	PHP_ME(Integer, powMod, gmpint_powmod_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, congruent, gmpint_congruent_arginfo, ZEND_ACC_PUBLIC)

	PHP_ME(Integer, toFloat, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, toInteger, NULL, ZEND_ACC_PUBLIC)

	PHP_ME(Integer, toString, gmpint_tostring_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, __toString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Integer, __debugInfo, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/***************************************************************************/

static zend_object* gmpint_create_object(zend_class_entry *ce,
                                         php_gmpint_object **pobjval) {
    php_gmpint_object *objval = ecalloc(1,
	    sizeof(php_gmpint_object) + zend_object_properties_size(ce));
    zend_object *zobj = &(objval->std);

    zend_object_std_init(zobj, ce);
    zobj->handlers = &handlers;
	mpz_init(objval->num);

    if (pobjval) { *pobjval = objval; }
    return zobj;
}

static zend_object* gmpint_ctor(zend_class_entry *ce) {
	return gmpint_create_object(ce, NULL);
}

static zend_object* gmpint_clone(zval *obj) {
	zend_object *zold = Z_OBJ_P(obj);
	php_gmpint_object *oldobj = php_gmpint_object_from_zend_object(zold);
	php_gmpint_object *newobj;
	zend_object *znew = gmpint_create_object(Z_OBJCE_P(obj), &newobj);

	mpz_set(newobj->num, oldobj->num);
	zend_objects_clone_members(znew, zold);
	return znew;
}

static void gmpint_free(zend_object *obj) {
	mpz_clear(php_gmpint_object_from_zend_object(obj)->num);
}

PHP_MINIT_FUNCTION(gmpi_int) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "GMPi\\Integer", gmpint_methods);
	php_gmpint_ce = zend_register_internal_class(&ce);
	php_gmpint_ce->create_object = gmpint_ctor;
	zend_do_implement_interface(php_gmpint_ce, php_gmpi_ce);

	memcpy(&handlers, zend_get_std_object_handlers(),
	       sizeof(zend_object_handlers));
	handlers.offset = XtOffsetOf(php_gmpint_object, std);
	handlers.clone_obj = gmpint_clone;
	handlers.free_obj = gmpint_free;
	handlers.do_operation = gmpint_do_operation;
	handlers.compare = gmpint_do_compare;

	return SUCCESS;
}

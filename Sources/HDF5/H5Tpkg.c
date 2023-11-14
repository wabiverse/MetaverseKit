/**
 * TODO: Generate the appropriate code and do this correctly,
 * jamming this muck in here just to resolve the missing linker
 * symbols for now.
 */

/****************/
/* Module Setup */
/****************/
#define H5T_FRIEND		/*suppress error about including H5Tpkg	  */

/***********/
/* Headers */
/***********/
#include "H5private.h"      /* Generic Functions                        */
#include "H5Iprivate.h"     /* Error handling                           */
#include "H5Tpkg.h"         /* IDs                                      */

char H5libhdf5_settings[] = "";

/*-------------------------------------------------------------------------
 * Function:	H5T__init_native
 *
 * Purpose:	Initialize pre-defined native datatypes from code generated
 * during the library configuration by H5detect.
 *
 * Return:	Success:	non-negative
 * Failure:	negative
 *
 * Programmer:	Robb Matzke
 * Wednesday, December 16, 1998
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5T__init_native(void)
{
    H5T_t	*dt = NULL;
    herr_t	ret_value = SUCCEED;

    FUNC_ENTER_PACKAGE

	  /* The part common to fixed and floating types */

    if(NULL == (dt = H5T__alloc())) {
        HGOTO_ERROR(H5E_DATATYPE, H5E_NOSPACE, FAIL, "datatype allocation failed")
    }
    dt->shared->state = H5T_STATE_IMMUTABLE;
    // if (d[i].msize) {
      dt->shared->type = H5T_FLOAT;
    // } else {
    //   dt->shared->type = H5T_INTEGER;
    // }
    
    //dt->shared->size = d[i].size;

    // if(byte_order==-1)
        dt->shared->u.atomic.order = H5T_ORDER_VAX;
    // else if(byte_order==0)
    //     dt->shared->u.atomic.order = H5T_ORDER_LE;
    // else
    //     dt->shared->u.atomic.order = H5T_ORDER_BE;


    //dt->shared->u.atomic.offset = d[i].offset;
    //dt->shared->u.atomic.prec = d[i].precision;
    dt->shared->u.atomic.lsb_pad = H5T_PAD_ZERO;
    dt->shared->u.atomic.msb_pad = H5T_PAD_ZERO;
    /*HDassert((d[i].perm[0]>0)==(byte_order>0));*/   /* Double-check that byte-order doesn't change */

	// if (0 == d[i].msize) {
	    /* The part unique to fixed point types */
      // if (d[i].sign)
      // {
        dt->shared->u.atomic.u.i.sign = H5T_SGN_2;
      // }
      // else
      // {
      //   dt->shared->u.atomic.u.i.sign = H5T_SGN_NONE;
      // }
	// } else {
	//     /* The part unique to floating point types */
  //   dt->shared->u.atomic.u.f.sign = d[i].sign;
  //   dt->shared->u.atomic.u.f.epos = d[i].epos;
  //   dt->shared->u.atomic.u.f.esize = d[i].esize;
  //   dt->shared->u.atomic.u.f.ebias = (unsigned long)(d[i].bias);
  //   dt->shared->u.atomic.u.f.mpos = d[i].mpos;
  //   dt->shared->u.atomic.u.f.msize = d[i].msize;
  //   dt->shared->u.atomic.u.f.norm = d[i].imp ? H5T_NORM_IMPLIED : H5T_NORM_NONE;
  //   dt->shared->u.atomic.u.f.pad = H5T_PAD_ZERO;
	// }

    hid_t result = FAIL;
    result = H5T_NATIVE_SCHAR_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UCHAR_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_SHORT_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_USHORT_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_LONG_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_ULONG_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_LLONG_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_ULLONG_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_FLOAT_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_DOUBLE_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_LDOUBLE_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_B8_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_B16_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_B32_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_B64_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_OPAQUE_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_HADDR_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_HSIZE_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_HSSIZE_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_HERR_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_HBOOL_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT8_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT8_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_LEAST8_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_LEAST8_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_FAST8_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_FAST8_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT16_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT16_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_LEAST16_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_LEAST16_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_FAST16_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_FAST16_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT32_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT32_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_LEAST32_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_LEAST32_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_FAST32_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_FAST32_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT64_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT64_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_LEAST64_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_LEAST64_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_INT_FAST64_g = H5I_register(H5I_DATATYPE, dt, FALSE);
    result = H5T_NATIVE_UINT_FAST64_g = H5I_register(H5I_DATATYPE, dt, FALSE);

	/* Atomize the type */
    if(result < 0)
        HGOTO_ERROR(H5E_DATATYPE, H5E_CANTINIT, FAIL, "can't register ID for built-in datatype")

        H5T_NATIVE_INT8_ALIGN_g = 0;
        H5T_NATIVE_UINT8_ALIGN_g = 0;
        H5T_NATIVE_INT_LEAST8_ALIGN_g = 0;
        H5T_NATIVE_UINT_LEAST8_ALIGN_g = 0;
        H5T_NATIVE_INT_FAST8_ALIGN_g = 0;
        H5T_NATIVE_UINT_FAST8_ALIGN_g = 0;
        H5T_NATIVE_INT16_ALIGN_g = 0;
        H5T_NATIVE_UINT16_ALIGN_g = 0;
        H5T_NATIVE_INT_LEAST16_ALIGN_g = 0;
        H5T_NATIVE_UINT_LEAST16_ALIGN_g = 0;
        H5T_NATIVE_INT_FAST16_ALIGN_g = 0;
        H5T_NATIVE_UINT_FAST16_ALIGN_g = 0;
        H5T_NATIVE_INT32_ALIGN_g = 0;
        H5T_NATIVE_UINT32_ALIGN_g = 0;
        H5T_NATIVE_INT_LEAST32_ALIGN_g = 0;
        H5T_NATIVE_UINT_LEAST32_ALIGN_g = 0;
        H5T_NATIVE_INT_FAST32_ALIGN_g = 0;
        H5T_NATIVE_UINT_FAST32_ALIGN_g = 0;
        H5T_NATIVE_INT64_ALIGN_g = 0;
        H5T_NATIVE_UINT64_ALIGN_g = 0;
        H5T_NATIVE_INT_LEAST64_ALIGN_g = 0;
        H5T_NATIVE_UINT_LEAST64_ALIGN_g = 0;
        H5T_NATIVE_INT_FAST64_ALIGN_g = 0;
        H5T_NATIVE_UINT_FAST64_ALIGN_g = 0;

        /* Variables for alignment of compound datatype */
        H5T_NATIVE_SCHAR_COMP_ALIGN_g = 0;
        //H5T_NATIVE_UCHAR_COMP_ALIGN_g = 0;
        H5T_NATIVE_SHORT_COMP_ALIGN_g = 0;
        //H5T_NATIVE_USHORT_COMP_ALIGN_g = 0;
        H5T_NATIVE_INT_COMP_ALIGN_g = 0;
        //H5T_NATIVE_UINT_COMP_ALIGN_g = 0;
        H5T_NATIVE_LONG_COMP_ALIGN_g = 0;
        //H5T_NATIVE_ULONG_COMP_ALIGN_g = 0;
        H5T_NATIVE_LLONG_COMP_ALIGN_g = 0;
        //H5T_NATIVE_ULLONG_COMP_ALIGN_g = 0;
        H5T_NATIVE_FLOAT_COMP_ALIGN_g = 0;
        H5T_NATIVE_DOUBLE_COMP_ALIGN_g = 0;
        #if H5_SIZEOF_LONG_DOUBLE !=0
        H5T_NATIVE_LDOUBLE_COMP_ALIGN_g = 0;
        #endif
    }

     H5T_native_order_g = H5T_ORDER_LE;
     // H5T_native_order_g = H5T_ORDER_BE;

    /* Structure alignment for pointers, hvl_t, hobj_ref_t, hdset_reg_ref_t */
    H5T_POINTER_COMP_ALIGN_g = 0;
    H5T_HVL_COMP_ALIGN_g = 0;
    H5T_HOBJREF_COMP_ALIGN_g = 0;
    H5T_HDSETREGREF_COMP_ALIGN_g = 0;

done:
    if(ret_value < 0) {
        if(dt != NULL) {
            dt->shared = H5FL_FREE(H5T_shared_t, dt->shared);
            dt = H5FL_FREE(H5T_t, dt);
        } /* end if */
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value);

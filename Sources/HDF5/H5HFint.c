/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:     H5HFint.c
 *              Fall 2012
 *              Dana Robinson
 *
 * Purpose:     Internal routines for fractal heaps.
 *
 *-------------------------------------------------------------------------
 */

/**********************/
/* Module Declaration */
/**********************/

#include "H5HFmodule.h"         /* This source code file is part of the H5HF module */


/***********************/
/* Other Packages Used */
/***********************/


/***********/
/* Headers */
/***********/
#include "H5private.h"      /* Generic Functions                            */
#include "H5ACprivate.h"    /* Metadata Cache                               */
#include "H5Eprivate.h"     /* Error handling                               */
#include "H5HFpkg.h"        /* Fractal Heaps                                */


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:    H5HF__create_flush_depend
 *
 * Purpose:     Create a flush dependency between two data structure components
 *
 * Return:      SUCCEED/FAIL
 *
 * Programmer:  Dana Robinson
 *              Fall 2012
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF__create_flush_depend(H5AC_info_t *parent_entry, H5AC_info_t *child_entry)
{
    herr_t ret_value = SUCCEED;             /* Return value */

    FUNC_ENTER_NOAPI_NOINIT
    
    /* Sanity check */
    HDassert(parent_entry);
    HDassert(child_entry);

    /* Create a flush dependency between parent and child entry */
    if(H5AC_create_flush_dependency(parent_entry, child_entry) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_CANTDEPEND, FAIL, "unable to create flush dependency")

done:
    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5HF__create_flush_depend() */


/*-------------------------------------------------------------------------
 * Function:    H5HF__destroy_flush_depend
 *
 * Purpose:     Destroy a flush dependency between two data structure components
 *
 * Return:      SUCCEED/FAIL
 *
 * Programmer:  Dana Robinson
 *              Fall 2012
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF__destroy_flush_depend(H5AC_info_t *parent_entry, H5AC_info_t *child_entry)
{
    herr_t ret_value = SUCCEED;             /* Return value */

    FUNC_ENTER_NOAPI_NOINIT
    
    /* Sanity check */
    HDassert(parent_entry);
    HDassert(child_entry);

    /* Destroy a flush dependency between parent and child entry */
    if(H5AC_destroy_flush_dependency(parent_entry, child_entry) < 0)
        HGOTO_ERROR(H5E_HEAP, H5E_CANTUNDEPEND, FAIL, "unable to destroy flush dependency")

done:
    FUNC_LEAVE_NOAPI(ret_value)
}   /* end H5HF__destroy_flush_depend() */


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
 * Created:         H5FAstat.c
 *
 * Purpose:         Fixed array metadata statistics functions.
 *
 *-------------------------------------------------------------------------
 */

/**********************/
/* Module Declaration */
/**********************/

#include "H5FAmodule.h"         /* This source code file is part of the H5FA module */


/***********************/
/* Other Packages Used */
/***********************/


/***********/
/* Headers */
/***********/
#include "H5private.h"      /* Generic Functions                        */
#include "H5Eprivate.h"     /* Error handling                           */
#include "H5FApkg.h"        /* Fixed Arrays                             */


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
 * Function:    H5FA_get_stats
 *
 * Purpose:     Query the metadata stats of an array
 *
 * Return:      SUCCEED/FAIL
 *
 * Programmer:  Vailin Choi
 *              Thursday, April 30, 2009
 *
 *-------------------------------------------------------------------------
 */
BEGIN_FUNC(PRIV, NOERR,
herr_t, SUCCEED, -,
H5FA_get_stats(const H5FA_t *fa, H5FA_stat_t *stats))

    /* Local variables */

#ifdef H5FA_DEBUG
HDfprintf(stderr, "%s: Called\n", FUNC);
#endif /* H5FA_DEBUG */

    /*
     * Check arguments.
     */
    HDassert(fa);
    HDassert(stats);

    /* Copy fixed array statistics */
    HDmemcpy(stats, &fa->hdr->stats, sizeof(fa->hdr->stats));

END_FUNC(PRIV)  /* end H5FA_get_stats() */


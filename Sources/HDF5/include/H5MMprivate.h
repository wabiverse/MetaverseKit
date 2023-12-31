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
 * Created:             H5MMprivate.h
 *                      Jul 10 1997
 *                      Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:             Private header for memory management.
 *
 *-------------------------------------------------------------------------
 */
#ifndef _H5MMprivate_H
#define _H5MMprivate_H

#include "H5MMpublic.h"

/* Private headers needed by this file */
#include "H5private.h"

#if defined H5_MEMORY_ALLOC_SANITY_CHECK
/*#define H5MM_PRINT_MEMORY_STATS */
#define H5MM_free(Z)	H5MM_xfree(Z)
#else /* H5_MEMORY_ALLOC_SANITY_CHECK */
#define H5MM_free(Z)	HDfree(Z)
#endif /* H5_MEMORY_ALLOC_SANITY_CHECK */

/*
 * Library prototypes...
 */
H5_DLL void *H5MM_malloc(size_t size);
H5_DLL void *H5MM_calloc(size_t size);
H5_DLL void *H5MM_realloc(void *mem, size_t size);
H5_DLL char *H5MM_xstrdup(const char *s);
H5_DLL char *H5MM_strdup(const char *s);
H5_DLL void *H5MM_xfree(void *mem);
#if defined H5_MEMORY_ALLOC_SANITY_CHECK
H5_DLL void H5MM_sanity_check_all(void);
H5_DLL void H5MM_final_sanity_check(void);
#endif /* H5_MEMORY_ALLOC_SANITY_CHECK */

#endif /* _H5MMprivate_H */


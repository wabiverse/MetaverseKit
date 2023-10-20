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
 * Created:     H5B2cache.c
 *              Jan 31 2005
 *              Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose:     Implement v2 B-tree metadata cache methods.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#include "H5B2module.h"         /* This source code file is part of the H5B2 module */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5B2pkg.h"		/* v2 B-trees				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5WBprivate.h"        /* Wrapped Buffers                      */


/****************/
/* Local Macros */
/****************/

/* B-tree format version #'s */
#define H5B2_HDR_VERSION 0              /* Header */
#define H5B2_INT_VERSION 0              /* Internal node */
#define H5B2_LEAF_VERSION 0             /* Leaf node */


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/

/* Metadata cache callbacks */
static herr_t H5B2__cache_hdr_get_load_size(const void *image_ptr, void *udata, 
    size_t *image_len, size_t *actual_len,
    hbool_t *compressed_ptr, size_t *compressed_image_len_ptr);
static htri_t H5B2__cache_hdr_verify_chksum(const void *image_ptr, size_t len, void *udata);
static void *H5B2__cache_hdr_deserialize(const void *image, size_t len,
    void *udata, hbool_t *dirty);
static herr_t H5B2__cache_hdr_image_len(const void *thing, size_t *image_len,
    hbool_t *compressed_ptr, size_t *compressed_image_len_ptr);
static herr_t H5B2__cache_hdr_serialize(const H5F_t *f, void *image, size_t len,
    void *thing);
static herr_t H5B2__cache_hdr_notify(H5AC_notify_action_t action, void *thing);
static herr_t H5B2__cache_hdr_free_icr(void *thing);

static herr_t H5B2__cache_int_get_load_size(const void *image_ptr, void *udata, 
    size_t *image_len, size_t *actual_len,
    hbool_t *compressed_ptr, size_t *compressed_image_len_ptr);
static htri_t H5B2__cache_int_verify_chksum(const void *image_ptr, size_t len, void *udata);
static void *H5B2__cache_int_deserialize(const void *image, size_t len,
    void *udata, hbool_t *dirty);
static herr_t H5B2__cache_int_image_len(const void *thing, size_t *image_len,
    hbool_t *compressed_ptr, size_t *compressed_image_len_ptr);
static herr_t H5B2__cache_int_serialize(const H5F_t *f, void *image, size_t len,
    void *thing);
static herr_t H5B2__cache_int_notify(H5AC_notify_action_t action, void *thing);
static herr_t H5B2__cache_int_free_icr(void *thing);

static herr_t H5B2__cache_leaf_get_load_size(const void * mage_ptr, void *udata, 
    size_t *image_len, size_t *actual_len,
    hbool_t *compressed_ptr, size_t *compressed_image_len_ptr);
static htri_t H5B2__cache_leaf_verify_chksum(const void *image_ptr, size_t len, void *udata);
static void *H5B2__cache_leaf_deserialize(const void *image, size_t len,
    void *udata, hbool_t *dirty);
static herr_t H5B2__cache_leaf_image_len(const void *thing, size_t *image_len,
    hbool_t *compressed_ptr, size_t *compressed_image_len_ptr);
static herr_t H5B2__cache_leaf_serialize(const H5F_t *f, void *image, size_t len,
    void *thing);
static herr_t H5B2__cache_leaf_notify(H5AC_notify_action_t action, void *thing);
static herr_t H5B2__cache_leaf_free_icr(void *thing);

/*********************/
/* Package Variables */
/*********************/

/* H5B2 inherits cache-like properties from H5AC */
const H5AC_class_t H5AC_BT2_HDR[1] = {{
    H5AC_BT2_HDR_ID,                    /* Metadata client ID */
    "v2 B-tree header",                 /* Metadata client name (for debugging) */
    H5FD_MEM_BTREE,                     /* File space memory type for client */
    H5AC__CLASS_NO_FLAGS_SET,           /* Client class behavior flags */
    H5B2__cache_hdr_get_load_size,      /* 'get_load_size' callback */
    H5B2__cache_hdr_verify_chksum,
    H5B2__cache_hdr_deserialize,        /* 'deserialize' callback */
    H5B2__cache_hdr_image_len,          /* 'image_len' callback */
    NULL,                               /* 'pre_serialize' callback */
    H5B2__cache_hdr_serialize,          /* 'serialize' callback */
    H5B2__cache_hdr_notify, 		/* 'notify' callback */
    H5B2__cache_hdr_free_icr,           /* 'free_icr' callback */
    NULL,				/* 'clear' callback */
    NULL,				/* 'fsf_size' callback */
}};

/* H5B2 inherits cache-like properties from H5AC */
const H5AC_class_t H5AC_BT2_INT[1] = {{
    H5AC_BT2_INT_ID,                    /* Metadata client ID */
    "v2 B-tree internal node",          /* Metadata client name (for debugging) */
    H5FD_MEM_BTREE,                     /* File space memory type for client */
    H5AC__CLASS_NO_FLAGS_SET,           /* Client class behavior flags */
    H5B2__cache_int_get_load_size,      /* 'get_load_size' callback */
    H5B2__cache_int_verify_chksum,	/* 'verify_chksum' callback */
    H5B2__cache_int_deserialize,        /* 'deserialize' callback */
    H5B2__cache_int_image_len,          /* 'image_len' callback */
    NULL,                               /* 'pre_serialize' callback */
    H5B2__cache_int_serialize,          /* 'serialize' callback */
    H5B2__cache_int_notify,		/* 'notify' callback */
    H5B2__cache_int_free_icr,           /* 'free_icr' callback */
    NULL,				/* 'clear' callback */
    NULL,				/* 'fsf_size' callback */
}};

/* H5B2 inherits cache-like properties from H5AC */
const H5AC_class_t H5AC_BT2_LEAF[1] = {{
    H5AC_BT2_LEAF_ID,                   /* Metadata client ID */
    "v2 B-tree leaf node",              /* Metadata client name (for debugging) */
    H5FD_MEM_BTREE,                     /* File space memory type for client */
    H5AC__CLASS_NO_FLAGS_SET,           /* Client class behavior flags */
    H5B2__cache_leaf_get_load_size,     /* 'get_load_size' callback */
    H5B2__cache_leaf_verify_chksum,	/* 'verify_chksum' callback */
    H5B2__cache_leaf_deserialize,       /* 'deserialize' callback */
    H5B2__cache_leaf_image_len,         /* 'image_len' callback */
    NULL,                               /* 'pre_serialize' callback */
    H5B2__cache_leaf_serialize,         /* 'serialize' callback */
    H5B2__cache_leaf_notify,		/* 'notify' callback */
    H5B2__cache_leaf_free_icr,          /* 'free_icr' callback */
    NULL,				/* 'clear' callback */
    NULL,				/* 'fsf_size' callback */
}};


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_hdr_get_load_size
 *
 * Purpose:     Compute the size of the data structure on disk.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@hdfgroup.org
 *              May 18, 2010
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_hdr_get_load_size(const void *_image, void *_udata, size_t *image_len, size_t *actual_len,
    hbool_t H5_ATTR_UNUSED *compressed_ptr, size_t H5_ATTR_UNUSED *compressed_image_len_ptr)
{
    const uint8_t *image = (const uint8_t *)_image;       		    /* Pointer into raw data buffer */
    H5B2_hdr_cache_ud_t *udata = (H5B2_hdr_cache_ud_t *)_udata; /* User data for callback */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(udata);
    HDassert(udata->f);
    HDassert(image_len);

    if(image == NULL) {
	/* Set the image length size */
	*image_len = H5B2_HEADER_SIZE_FILE(udata->f);
    } else {
	HDassert(actual_len);
	HDassert(*actual_len == *actual_len);
    }

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5B2__cache_hdr_get_load_size() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_hdr_verify_chksum
 *
 * Purpose:     Verify the computed checksum of the data structure is the
 *              same as the stored chksum.
 *
 * Return:      Success:        TRUE/FALSE
 *              Failure:        Negative
 *
 * Programmer:	Vailin Choi; Aug 2015
 *
 *-------------------------------------------------------------------------
 */
static htri_t
H5B2__cache_hdr_verify_chksum(const void *_image, size_t len, void H5_ATTR_UNUSED *_udata)
{
    const uint8_t *image = (const uint8_t *)_image;       /* Pointer into raw data buffer */
    uint32_t stored_chksum;     /* Stored metadata checksum value */
    uint32_t computed_chksum;   /* Computed metadata checksum value */
    htri_t ret_value = TRUE;	/* Return value */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(image);

    /* Get stored and computed checksums */
    H5F_get_checksums(image, len, &stored_chksum, &computed_chksum);

    if(stored_chksum != computed_chksum)
	ret_value = FALSE;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5B2__cache_hdr_verify_chksum() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_hdr_deserialize
 *
 * Purpose:	Loads a B-tree header from the disk.
 *
 * Return:	Success:	Pointer to a new B-tree.
 *		Failure:	NULL
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Feb 1 2005
 *
 *-------------------------------------------------------------------------
 */
static void *
H5B2__cache_hdr_deserialize(const void *_image, size_t H5_ATTR_UNUSED len,
    void *_udata, hbool_t H5_ATTR_UNUSED *dirty)
{
    H5B2_hdr_t		*hdr = NULL;    /* B-tree header */
    H5B2_hdr_cache_ud_t *udata = (H5B2_hdr_cache_ud_t *)_udata;
    H5B2_create_t       cparam;         /* B-tree creation parameters */
    H5B2_subid_t        id;		/* ID of B-tree class, as found in file */
    uint16_t            depth;          /* Depth of B-tree */
    uint32_t            stored_chksum;  /* Stored metadata checksum value */
    const uint8_t	*image = (const uint8_t *)_image;       /* Pointer into raw data buffer */
    H5B2_hdr_t		*ret_value = NULL;      /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(image);
    HDassert(udata);

    /* Allocate new B-tree header and reset cache info */
    if(NULL == (hdr = H5B2__hdr_alloc(udata->f)))
        HGOTO_ERROR(H5E_BTREE, H5E_CANTALLOC, NULL, "allocation failed for B-tree header")

    /* Magic number */
    if(HDmemcmp(image, H5B2_HDR_MAGIC, (size_t)H5_SIZEOF_MAGIC))
	HGOTO_ERROR(H5E_BTREE, H5E_BADVALUE, NULL, "wrong B-tree header signature")
    image += H5_SIZEOF_MAGIC;

    /* Version */
    if(*image++ != H5B2_HDR_VERSION)
	HGOTO_ERROR(H5E_BTREE, H5E_BADRANGE, NULL, "wrong B-tree header version")

    /* B-tree class */
    id = (H5B2_subid_t)*image++;
    if(id >= H5B2_NUM_BTREE_ID)
        HGOTO_ERROR(H5E_BTREE, H5E_BADTYPE, NULL, "incorrect B-tree type")

    /* Node size (in bytes) */
    UINT32DECODE(image, cparam.node_size);

    /* Raw key size (in bytes) */
    UINT16DECODE(image, cparam.rrec_size);

    /* Depth of tree */
    UINT16DECODE(image, depth);

    /* Split & merge %s */
    cparam.split_percent = *image++;
    cparam.merge_percent = *image++;

    /* Root node pointer */
    H5F_addr_decode(udata->f, (const uint8_t **)&image, &(hdr->root.addr));
    UINT16DECODE(image, hdr->root.node_nrec);
    H5F_DECODE_LENGTH(udata->f, image, hdr->root.all_nrec);

    /* checksum verification already done in verify_chksum cb */

    /* Metadata checksum */
    UINT32DECODE(image, stored_chksum);

    /* Sanity check */
    HDassert((size_t)(image - (const uint8_t *)_image) == hdr->hdr_size);

    /* Initialize B-tree header info */
    cparam.cls = H5B2_client_class_g[id];
    if(H5B2__hdr_init(hdr, &cparam, udata->ctx_udata, depth) < 0)
        HGOTO_ERROR(H5E_BTREE, H5E_CANTINIT, NULL, "can't initialize B-tree header info")

    /* Set the B-tree header's address */
    hdr->addr = udata->addr;

    /* Sanity check */
    HDassert((size_t)(image - (const uint8_t *)_image) <= len);

    /* Set return value */
    ret_value = hdr;

done:
    if(!ret_value && hdr)
        if(H5B2__hdr_free(hdr) < 0)
            HDONE_ERROR(H5E_BTREE, H5E_CANTRELEASE, NULL, "can't release v2 B-tree header")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5B2__cache_hdr_deserialize() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_hdr_image_len
 *
 * Purpose:     Compute the size of the data structure on disk.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@hdfgroup.org
 *              May 20, 2010
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_hdr_image_len(const void *_thing, size_t *image_len,
    hbool_t H5_ATTR_UNUSED *compressed_ptr, size_t H5_ATTR_UNUSED *compressed_image_len_ptr)
{
    const H5B2_hdr_t *hdr = (const H5B2_hdr_t *)_thing;      /* Pointer to the B-tree header */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(hdr);
    HDassert(image_len);

    /* Set the image length size */
    *image_len = hdr->hdr_size;

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5B2__cache_hdr_image_len() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_hdr_serialize
 *
 * Purpose:	Flushes a dirty B-tree header to disk.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Feb 1 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_hdr_serialize(const H5F_t *f, void *_image, size_t H5_ATTR_UNUSED len,
    void *_thing)
{
    H5B2_hdr_t *hdr = (H5B2_hdr_t *)_thing;     /* Pointer to the B-tree header */
    uint8_t *image = (uint8_t *)_image;         /* Pointer into raw data buffer */
    uint32_t metadata_chksum;                   /* Computed metadata checksum value */

    FUNC_ENTER_STATIC_NOERR

    /* check arguments */
    HDassert(f);
    HDassert(image);
    HDassert(hdr);

    /* Magic number */
    HDmemcpy(image, H5B2_HDR_MAGIC, (size_t)H5_SIZEOF_MAGIC);
    image += H5_SIZEOF_MAGIC;

    /* Version # */
    *image++ = H5B2_HDR_VERSION;

    /* B-tree type */
    *image++ = hdr->cls->id;

    /* Node size (in bytes) */
    UINT32ENCODE(image, hdr->node_size);

    /* Raw key size (in bytes) */
    UINT16ENCODE(image, hdr->rrec_size);

    /* Depth of tree */
    UINT16ENCODE(image, hdr->depth);

    /* Split & merge %s */
    H5_CHECK_OVERFLOW(hdr->split_percent, /* From: */ unsigned, /* To: */ uint8_t);
    *image++ = (uint8_t)hdr->split_percent;
    H5_CHECK_OVERFLOW(hdr->merge_percent, /* From: */ unsigned, /* To: */ uint8_t);
    *image++ = (uint8_t)hdr->merge_percent;

    /* Root node pointer */
    H5F_addr_encode(f, &image, hdr->root.addr);
    UINT16ENCODE(image, hdr->root.node_nrec);
    H5F_ENCODE_LENGTH(f, image, hdr->root.all_nrec);

    /* Compute metadata checksum */
    metadata_chksum = H5_checksum_metadata(_image, (hdr->hdr_size - H5B2_SIZEOF_CHKSUM), 0);

    /* Metadata checksum */
    UINT32ENCODE(image, metadata_chksum);

    /* Sanity check */
    HDassert((size_t)(image - (uint8_t *)_image) == len);

    /* Clear shadowed node lists, as the header has been flushed and all
     * nodes must be shadowed again (if doing SWMR writes).  Note that this
     * algorithm does one extra iteration at the end, as the last node's
     * shadowed_next pointer points to itself. */
    while(hdr->shadowed_internal) {
	H5B2_internal_t *next = hdr->shadowed_internal->shadowed_next;

	HDassert(!hdr->shadowed_internal->cache_info.is_dirty);
	hdr->shadowed_internal->shadowed_next = NULL;
	hdr->shadowed_internal->shadowed_prev = NULL;
	hdr->shadowed_internal = next;
    } /* end while */
    while(hdr->shadowed_leaf) {
	H5B2_leaf_t *next = hdr->shadowed_leaf->shadowed_next;

	HDassert(!hdr->shadowed_leaf->cache_info.is_dirty);
	hdr->shadowed_leaf->shadowed_next = NULL;
	hdr->shadowed_leaf->shadowed_prev = NULL;
	hdr->shadowed_leaf = next;
    } /* end while */

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* H5B2__cache_hdr_serialize() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_hdr_notify
 *
 * Purpose:     Handle cache action notifications
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Neil Fortner
 *              nfortne2@hdfgroup.org
 *              Apr 24 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_hdr_notify(H5AC_notify_action_t action, void *_thing)
{
    H5B2_hdr_t 	*hdr = (H5B2_hdr_t *)_thing;
    herr_t 	ret_value = SUCCEED;

    FUNC_ENTER_STATIC

    /*
     * Check arguments.
     */
    HDassert(hdr);

    /* Check if the file was opened with SWMR-write access */
    if(hdr->swmr_write) {
        switch(action) {
            case H5AC_NOTIFY_ACTION_AFTER_INSERT:
	    case H5AC_NOTIFY_ACTION_AFTER_LOAD:
	    case H5AC_NOTIFY_ACTION_AFTER_FLUSH:
		/* do nothing */
                break;

	    case H5AC_NOTIFY_ACTION_BEFORE_EVICT:
                /* If hdr->parent != NULL, the v2 B-tree header
                 * must be employed as the index for a chunked
                 * data set which has been modified by the SWMR writer.
                 * 
                 * In this case, hdr->parent must contain a
                 * pointer to the object header proxy which is the flush 
                 * dependency parent of the v2 B-tree header.
                 *
                 * hdr->parent is used to destroy the flush dependency
                 * before the v2 B-tree header is evicted.
                 */
                if(hdr->parent) {
                    /* Sanity checks */
                    HDassert(((H5AC_info_t *)hdr->parent)->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
                    HDassert(((H5AC_info_t *)hdr->parent)->type);
                    HDassert(((H5AC_info_t *)hdr->parent)->type->id == H5AC_OHDR_PROXY_ID);

		    /* Destroy flush dependency on object header proxy */
		    if(H5B2__destroy_flush_depend((H5AC_info_t *)hdr->parent, (H5AC_info_t *)hdr) < 0)
                        HGOTO_ERROR(H5E_BTREE, H5E_CANTUNDEPEND, FAIL, "unable to destroy flush dependency")
		} /* end if */
		break;

            default:
#ifdef NDEBUG
                HGOTO_ERROR(H5E_BTREE, H5E_BADVALUE, FAIL, "unknown action from metadata cache")
#else /* NDEBUG */
                HDassert(0 && "Unknown action?!?");
#endif /* NDEBUG */
        } /* end switch */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5B2__cache_hdr_notify() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_hdr_free_icr
 *
 * Purpose:	Destroy/release an "in core representation" of a data
 *              structure
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Mike McGreevy
 *              mcgreevy@hdfgroup.org
 *              June 18, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_hdr_free_icr(void *thing)
{
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(thing);

    /* Destroy v2 B-tree header */
    if(H5B2__hdr_free((H5B2_hdr_t *)thing) < 0)
        HGOTO_ERROR(H5E_BTREE, H5E_CANTFREE, FAIL, "unable to free v2 B-tree header")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5B2__cache_hdr_free_icr() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_int_get_load_size
 *
 * Purpose:     Compute the size of the data structure on disk.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@hdfgroup.org
 *              May 18, 2010
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_int_get_load_size(const void *_image, void *_udata, size_t *image_len, size_t *actual_len,
    hbool_t H5_ATTR_UNUSED *compressed_ptr, size_t H5_ATTR_UNUSED *compressed_image_len_ptr)
{
    const uint8_t *image = (const uint8_t *)_image;       /* Pointer into raw data buffer */
    H5B2_internal_cache_ud_t *udata = (H5B2_internal_cache_ud_t *)_udata; /* User data for callback */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(udata);
    HDassert(udata->hdr);
    HDassert(image_len);

    if(image == NULL) {
	/* Set the image length size */
	*image_len = udata->hdr->node_size;
    } else {
	HDassert(actual_len);
	HDassert(*actual_len == *image_len);
    }

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5B2__cache_int_get_load_size() */

/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_int_verify_chksum
 *
 * Purpose:     Verify the computed checksum of the data structure is the
 *              same as the stored chksum.
 *
 * Return:      Success:        TRUE/FALSE
 *              Failure:        Negative
 *
 * Programmer:	Vailin Choi; Aug 2015
 *
 *-------------------------------------------------------------------------
 */
static htri_t
H5B2__cache_int_verify_chksum(const void *_image, size_t H5_ATTR_UNUSED len, void *_udata)
{
    const uint8_t *image = (const uint8_t *)_image;       			/* Pointer into raw data buffer */
    H5B2_internal_cache_ud_t *udata = (H5B2_internal_cache_ud_t *)_udata;     	/* Pointer to user data */
    size_t chk_size;       	/* Exact size of the node with checksum at the end */
    uint32_t stored_chksum;     /* Stored metadata checksum value */
    uint32_t computed_chksum;   /* Computed metadata checksum value */
    htri_t ret_value = TRUE;	/* Return value */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(image);
    HDassert(udata);

    /* Internal node prefix header + records + child pointer triplets: size with checksum at the end */
    chk_size = H5B2_INT_PREFIX_SIZE + (udata->nrec * udata->hdr->rrec_size) + ((size_t)(udata->nrec + 1) * H5B2_INT_POINTER_SIZE(udata->hdr, udata->depth));

    /* Get stored and computed checksums */
    H5F_get_checksums(image, chk_size, &stored_chksum, &computed_chksum);

    if(stored_chksum != computed_chksum)
	ret_value = FALSE;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5B2__cache_int_verify_chksum() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_int_deserialize
 *
 * Purpose:	Deserialize a B-tree internal node from the disk.
 *
 * Return:	Success:	Pointer to a new B-tree internal node.
 *              Failure:        NULL
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Feb 2 2005
 *
 *-------------------------------------------------------------------------
 */
static void *
H5B2__cache_int_deserialize(const void *_image, size_t H5_ATTR_UNUSED len,
    void *_udata, hbool_t H5_ATTR_UNUSED *dirty)
{
    H5B2_internal_cache_ud_t *udata = (H5B2_internal_cache_ud_t *)_udata;     /* Pointer to user data */
    H5B2_internal_t	*internal = NULL;       /* Internal node read */
    const uint8_t	*image = (const uint8_t *)_image;       /* Pointer into raw data buffer */
    uint8_t		*native;        /* Pointer to native record info */
    H5B2_node_ptr_t	*int_node_ptr;  /* Pointer to node pointer info */
    uint32_t            stored_chksum;  /* Stored metadata checksum value */
    unsigned		u;              /* Local index variable */
    H5B2_internal_t	*ret_value = NULL;      /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(image);
    HDassert(udata);

    /* Allocate new internal node and reset cache info */
    if(NULL == (internal = H5FL_MALLOC(H5B2_internal_t)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")
    HDmemset(&internal->cache_info, 0, sizeof(H5AC_info_t));

    /* Increment ref. count on B-tree header */
    if(H5B2__hdr_incr(udata->hdr) < 0)
        HGOTO_ERROR(H5E_BTREE, H5E_CANTINC, NULL, "can't increment ref. count on B-tree header")

    /* Share B-tree information */
    internal->hdr = udata->hdr;
    internal->parent = udata->parent;
    internal->shadowed_next = NULL;
    internal->shadowed_prev = NULL;

    /* Magic number */
    if(HDmemcmp(image, H5B2_INT_MAGIC, (size_t)H5_SIZEOF_MAGIC))
        HGOTO_ERROR(H5E_BTREE, H5E_BADVALUE, NULL, "wrong B-tree internal node signature")
    image += H5_SIZEOF_MAGIC;

    /* Version */
    if(*image++ != H5B2_INT_VERSION)
        HGOTO_ERROR(H5E_BTREE, H5E_BADVALUE, NULL, "wrong B-tree internal node version")

    /* B-tree type */
    if(*image++ != (uint8_t)udata->hdr->cls->id)
        HGOTO_ERROR(H5E_BTREE, H5E_BADTYPE, NULL, "incorrect B-tree type")

    /* Allocate space for the native keys in memory */
    if(NULL == (internal->int_native = (uint8_t *)H5FL_FAC_MALLOC(udata->hdr->node_info[udata->depth].nat_rec_fac)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed for B-tree internal native keys")

    /* Allocate space for the node pointers in memory */
    if(NULL == (internal->node_ptrs = (H5B2_node_ptr_t *)H5FL_FAC_MALLOC(udata->hdr->node_info[udata->depth].node_ptr_fac)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed for B-tree internal node pointers")

    /* Set the number of records in the leaf & it's depth */
    internal->nrec = udata->nrec;
    internal->depth = udata->depth;

    /* Deserialize records for internal node */
    native = internal->int_native;
    for(u = 0; u < internal->nrec; u++) {
        /* Decode record */
        if((udata->hdr->cls->decode)(image, native, udata->hdr->cb_ctx) < 0)
            HGOTO_ERROR(H5E_BTREE, H5E_CANTDECODE, NULL, "unable to decode B-tree record")

        /* Move to next record */
        image += udata->hdr->rrec_size;
        native += udata->hdr->cls->nrec_size;
    } /* end for */

    /* Deserialize node pointers for internal node */
    int_node_ptr = internal->node_ptrs;
    for(u = 0; u < (unsigned)(internal->nrec + 1); u++) {
        /* Decode node pointer */
        H5F_addr_decode(udata->f, (const uint8_t **)&image, &(int_node_ptr->addr));
        UINT64DECODE_VAR(image, int_node_ptr->node_nrec, udata->hdr->max_nrec_size);
        if(udata->depth > 1)
            UINT64DECODE_VAR(image, int_node_ptr->all_nrec, udata->hdr->node_info[udata->depth - 1].cum_max_nrec_size)
        else
            int_node_ptr->all_nrec = int_node_ptr->node_nrec;

        /* Move to next node pointer */
        int_node_ptr++;
    } /* end for */

    /* checksum verification already done in verify_chksum cb */

    /* Metadata checksum */
    UINT32DECODE(image, stored_chksum);

    /* Sanity check parsing */
    HDassert((size_t)(image - (const uint8_t *)_image) <= len);

    /* Set return value */
    ret_value = internal;

done:
    if(!ret_value && internal)
        if(H5B2__internal_free(internal) < 0)
            HDONE_ERROR(H5E_BTREE, H5E_CANTFREE, NULL, "unable to destroy B-tree internal node")

    FUNC_LEAVE_NOAPI(ret_value)
} /* H5B2__cache_int_deserialize() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_int_image_len
 *
 * Purpose:     Compute the size of the data structure on disk.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@hdfgroup.org
 *              May 20, 2010
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_int_image_len(const void *_thing, size_t *image_len,
    hbool_t H5_ATTR_UNUSED *compressed_ptr, size_t H5_ATTR_UNUSED *compressed_image_len_ptr)
{
    const H5B2_internal_t *internal = (const H5B2_internal_t *)_thing;      /* Pointer to the B-tree internal node */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(internal);
    HDassert(internal->hdr);
    HDassert(image_len);

    /* Set the image length size */
    *image_len = internal->hdr->node_size;

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5B2__cache_int_image_len() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_int_serialize
 *
 * Purpose:	Serializes a B-tree internal node for writing to disk.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Feb 3 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_int_serialize(const H5F_t *f, void *_image, size_t H5_ATTR_UNUSED len,
    void *_thing)
{
    H5B2_internal_t *internal = (H5B2_internal_t *)_thing;      /* Pointer to the B-tree internal node */
    uint8_t *image = (uint8_t *)_image; /* Pointer into raw data buffer */
    uint8_t *native;        /* Pointer to native record info */
    H5B2_node_ptr_t *int_node_ptr;      /* Pointer to node pointer info */
    uint32_t metadata_chksum; /* Computed metadata checksum value */
    unsigned u;             /* Local index variable */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_STATIC

    /* check arguments */
    HDassert(f);
    HDassert(image);
    HDassert(internal);
    HDassert(internal->hdr);

    /* Magic number */
    HDmemcpy(image, H5B2_INT_MAGIC, (size_t)H5_SIZEOF_MAGIC);
    image += H5_SIZEOF_MAGIC;

    /* Version # */
    *image++ = H5B2_INT_VERSION;

    /* B-tree type */
    *image++ = internal->hdr->cls->id;
    HDassert((size_t)(image - (uint8_t *)_image) == (H5B2_INT_PREFIX_SIZE - H5B2_SIZEOF_CHKSUM));

    /* Serialize records for internal node */
    native = internal->int_native;
    for(u = 0; u < internal->nrec; u++) {
        /* Encode record */
        if((internal->hdr->cls->encode)(image, native, internal->hdr->cb_ctx) < 0)
            HGOTO_ERROR(H5E_BTREE, H5E_CANTENCODE, FAIL, "unable to encode B-tree record")

        /* Move to next record */
        image += internal->hdr->rrec_size;
        native += internal->hdr->cls->nrec_size;
    } /* end for */

    /* Serialize node pointers for internal node */
    int_node_ptr = internal->node_ptrs;
    for(u = 0; u < (unsigned)(internal->nrec + 1); u++) {
        /* Encode node pointer */
        H5F_addr_encode(f, &image, int_node_ptr->addr);
        UINT64ENCODE_VAR(image, int_node_ptr->node_nrec, internal->hdr->max_nrec_size);
        if(internal->depth > 1)
            UINT64ENCODE_VAR(image, int_node_ptr->all_nrec, internal->hdr->node_info[internal->depth - 1].cum_max_nrec_size);

        /* Move to next node pointer */
        int_node_ptr++;
    } /* end for */

    /* Compute metadata checksum */
    metadata_chksum = H5_checksum_metadata(_image, (size_t)(image - (uint8_t *)_image), 0);

    /* Metadata checksum */
    UINT32ENCODE(image, metadata_chksum);

    /* Sanity check */
    HDassert((size_t)(image - (uint8_t *)_image) <= len);

    /* Clear rest of internal node */
    HDmemset(image, 0, len - (size_t)(image - (uint8_t *)_image));

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5B2__cache_int_serialize() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_int_notify
 *
 * Purpose:     Handle cache action notifications
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Neil Fortner
 *              nfortne2@hdfgroup.org
 *              Apr 25 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_int_notify(H5AC_notify_action_t action, void *_thing)
{
    H5B2_internal_t 	*internal = (H5B2_internal_t *)_thing;
    herr_t   		ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

    /*
     * Check arguments.
     */
    HDassert(internal);

    /* Check if the file was opened with SWMR-write access */
    if(internal->hdr->swmr_write) {
        HDassert(internal->parent);
        switch(action) {
            case H5AC_NOTIFY_ACTION_AFTER_INSERT:
	    case H5AC_NOTIFY_ACTION_AFTER_LOAD:
                /* Create flush dependency on parent */
                if(H5B2__create_flush_depend((H5AC_info_t *)internal->parent, (H5AC_info_t *)internal) < 0)
                    HGOTO_ERROR(H5E_BTREE, H5E_CANTDEPEND, FAIL, "unable to create flush dependency")
                break;

	    case H5AC_NOTIFY_ACTION_AFTER_FLUSH:
		/* do nothing */
		break;

            case H5AC_NOTIFY_ACTION_BEFORE_EVICT:
		/* Destroy flush dependency on parent */
		if(H5B2__destroy_flush_depend((H5AC_info_t *)internal->parent, (H5AC_info_t *)internal) < 0)
		    HGOTO_ERROR(H5E_BTREE, H5E_CANTUNDEPEND, FAIL, "unable to destroy flush dependency")

                break;

            default:
#ifdef NDEBUG
                HGOTO_ERROR(H5E_BTREE, H5E_BADVALUE, FAIL, "unknown action from metadata cache")
#else /* NDEBUG */
                HDassert(0 && "Unknown action?!?");
#endif /* NDEBUG */
        } /* end switch */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5B2__cache_int_notify() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_int_free_icr
 *
 * Purpose:	Destroy/release an "in core representation" of a data
 *              structure
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Mike McGreevy
 *              mcgreevy@hdfgroup.org
 *              June 18, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_int_free_icr(void *_thing)
{
    H5B2_internal_t *internal = (H5B2_internal_t *)_thing;
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(internal);
    HDassert(internal->hdr);

    /* Unlink from shadowed list */
    if(internal->shadowed_next) {
        if(internal->shadowed_next != internal) {
            internal->shadowed_next->shadowed_prev = internal->shadowed_prev;

            if(internal->shadowed_prev)
                internal->shadowed_prev->shadowed_next = internal->shadowed_next;
            else {
                HDassert(internal->hdr->shadowed_internal = internal);

                internal->hdr->shadowed_internal = internal->shadowed_next;
            } /* end else */
        } /* end if */
        else {
            if(internal->shadowed_prev)
                internal->shadowed_prev->shadowed_next = internal->shadowed_prev;
            else {
                HDassert(internal->hdr->shadowed_internal = internal);

                internal->hdr->shadowed_internal = NULL;
            } /* end else */
        } /* end else */
    } /* end if */

    /* Release v2 B-tree internal node */
    if(H5B2__internal_free(internal) < 0)
        HGOTO_ERROR(H5E_BTREE, H5E_CANTFREE, FAIL, "unable to release v2 B-tree internal node")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5B2__cache_int_free_icr() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_leaf_get_load_size
 *
 * Purpose:     Compute the size of the data structure on disk.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@hdfgroup.org
 *              May 18, 2010
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_leaf_get_load_size(const void *_image, void *_udata, size_t *image_len, size_t *actual_len,
    hbool_t H5_ATTR_UNUSED *compressed_ptr, size_t H5_ATTR_UNUSED *compressed_image_len_ptr)
{
    const uint8_t *image = (const uint8_t *)_image;       		      /* Pointer into raw data buffer */
    H5B2_leaf_cache_ud_t *udata = (H5B2_leaf_cache_ud_t *)_udata; /* User data for callback */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(udata);
    HDassert(udata->hdr);
    HDassert(image_len);

    if(image == NULL) {
	/* Set the image length size */
	*image_len = udata->hdr->node_size;
    } else {
	HDassert(actual_len);
	HDassert(*actual_len == *image_len);
    }

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5B2__cache_leaf_get_load_size() */

/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_leaf_verify_chksum
 *
 * Purpose:     Verify the computed checksum of the data structure is the
 *              same as the stored chksum.
 *
 * Return:      Success:        TRUE/FALSE
 *              Failure:        Negative
 *
 * Programmer:	Vailin Choi; Aug 2015
 *
 *-------------------------------------------------------------------------
 */
static htri_t
H5B2__cache_leaf_verify_chksum(const void *_image, size_t H5_ATTR_UNUSED len, void *_udata)
{
    const uint8_t *image = (const uint8_t *)_image;       			/* Pointer into raw data buffer */
    H5B2_internal_cache_ud_t *udata = (H5B2_internal_cache_ud_t *)_udata;     	/* Pointer to user data */
    size_t chk_size;       	/* Exact size of the node with checksum at the end */
    uint32_t stored_chksum;     /* Stored metadata checksum value */
    uint32_t computed_chksum;   /* Computed metadata checksum value */
    htri_t ret_value = TRUE;	/* Return value */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(image);
    HDassert(udata);

    /* Leaf node prefix header + records: size with checksum at the end */
    chk_size = H5B2_LEAF_PREFIX_SIZE + (udata->nrec * udata->hdr->rrec_size);

    /* Get stored and computed checksums */
    H5F_get_checksums(image, chk_size, &stored_chksum, &computed_chksum);

    if(stored_chksum != computed_chksum)
	ret_value = FALSE;

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5B2__cache_leaf_verify_chksum() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_leaf_deserialize
 *
 * Purpose:	Deserialize a B-tree leaf from the disk.
 *
 * Return:	Success:	Pointer to a new B-tree leaf node.
 *		Failure:	NULL
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Feb 2 2005
 *
 *-------------------------------------------------------------------------
 */
static void *
H5B2__cache_leaf_deserialize(const void *_image, size_t H5_ATTR_UNUSED len,
    void *_udata, hbool_t H5_ATTR_UNUSED *dirty)
{
    H5B2_leaf_cache_ud_t *udata = (H5B2_leaf_cache_ud_t *)_udata;
    H5B2_leaf_t		*leaf = NULL;   /* Pointer to lead node loaded */
    const uint8_t	*image = (const uint8_t *)_image;       /* Pointer into raw data buffer */
    uint8_t		*native;        /* Pointer to native keys */
    uint32_t            stored_chksum;  /* Stored metadata checksum value */
    unsigned		u;              /* Local index variable */
    H5B2_leaf_t		*ret_value = NULL;      /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(image);
    HDassert(udata);

    /* Allocate new leaf node and reset cache info */
    if(NULL == (leaf = H5FL_MALLOC(H5B2_leaf_t)))
	HGOTO_ERROR(H5E_BTREE, H5E_CANTALLOC, NULL, "memory allocation failed")
    HDmemset(&leaf->cache_info, 0, sizeof(H5AC_info_t));

    /* Increment ref. count on B-tree header */
    if(H5B2__hdr_incr(udata->hdr) < 0)
        HGOTO_ERROR(H5E_BTREE, H5E_CANTINC, NULL, "can't increment ref. count on B-tree header")

    /* Share B-tree header information */
    leaf->hdr = udata->hdr;
    leaf->parent = udata->parent;
    leaf->shadowed_next = NULL;
    leaf->shadowed_prev = NULL;

    /* Magic number */
    if(HDmemcmp(image, H5B2_LEAF_MAGIC, (size_t)H5_SIZEOF_MAGIC))
	HGOTO_ERROR(H5E_BTREE, H5E_BADVALUE, NULL, "wrong B-tree leaf node signature")
    image += H5_SIZEOF_MAGIC;

    /* Version */
    if(*image++ != H5B2_LEAF_VERSION)
	HGOTO_ERROR(H5E_BTREE, H5E_BADRANGE, NULL, "wrong B-tree leaf node version")

    /* B-tree type */
    if(*image++ != (uint8_t)udata->hdr->cls->id)
        HGOTO_ERROR(H5E_BTREE, H5E_BADTYPE, NULL, "incorrect B-tree type")

    /* Allocate space for the native keys in memory */
    if(NULL == (leaf->leaf_native = (uint8_t *)H5FL_FAC_MALLOC(udata->hdr->node_info[0].nat_rec_fac)))
	HGOTO_ERROR(H5E_BTREE, H5E_CANTALLOC, NULL, "memory allocation failed for B-tree leaf native keys")

    /* Set the number of records in the leaf */
    leaf->nrec = udata->nrec;

    /* Deserialize records for leaf node */
    native = leaf->leaf_native;
    for(u = 0; u < leaf->nrec; u++) {
        /* Decode record */
        if((udata->hdr->cls->decode)(image, native, udata->hdr->cb_ctx) < 0)
            HGOTO_ERROR(H5E_BTREE, H5E_CANTENCODE, NULL, "unable to decode B-tree record")

        /* Move to next record */
        image += udata->hdr->rrec_size;
        native += udata->hdr->cls->nrec_size;
    } /* end for */

    /* checksum verification already done in verify_chksum cb */

    /* Metadata checksum */
    UINT32DECODE(image, stored_chksum);

    /* Sanity check parsing */
    HDassert((size_t)(image - (const uint8_t *)_image) <= udata->hdr->node_size);

    /* Sanity check */
    HDassert((size_t)(image - (const uint8_t *)_image) <= len);

    /* Set return value */
    ret_value = leaf;

done:
    if(!ret_value && leaf)
        if(H5B2__leaf_free(leaf) < 0)
            HDONE_ERROR(H5E_BTREE, H5E_CANTFREE, NULL, "unable to destroy B-tree leaf node")

    FUNC_LEAVE_NOAPI(ret_value)
} /* H5B2__cache_leaf_deserialize() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_leaf_image_len
 *
 * Purpose:     Compute the size of the data structure on disk.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@hdfgroup.org
 *              May 20, 2010
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_leaf_image_len(const void *_thing, size_t *image_len,
    hbool_t H5_ATTR_UNUSED *compressed_ptr, size_t H5_ATTR_UNUSED *compressed_image_len_ptr)
{
    const H5B2_leaf_t *leaf = (const H5B2_leaf_t *)_thing;      /* Pointer to the B-tree leaf node  */

    FUNC_ENTER_STATIC_NOERR

    /* Check arguments */
    HDassert(leaf);
    HDassert(leaf->hdr);
    HDassert(image_len);

    /* Set the image length size */
    *image_len = leaf->hdr->node_size;

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5B2__cache_leaf_image_len() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_leaf_serialize
 *
 * Purpose:	Serializes a B-tree leaf node for writing to disk.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Feb 2 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_leaf_serialize(const H5F_t *f, void *_image, size_t H5_ATTR_UNUSED len,
    void *_thing)
{
    H5B2_leaf_t *leaf = (H5B2_leaf_t *)_thing;      /* Pointer to the B-tree leaf node  */
    uint8_t *image = (uint8_t *)_image;         /* Pointer into raw data buffer */
    uint8_t *native;            /* Pointer to native keys */
    uint32_t metadata_chksum;   /* Computed metadata checksum value */
    unsigned u;                 /* Local index variable */
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_STATIC

    /* check arguments */
    HDassert(f);
    HDassert(image);
    HDassert(leaf);
    HDassert(leaf->hdr);

    /* magic number */
    HDmemcpy(image, H5B2_LEAF_MAGIC, (size_t)H5_SIZEOF_MAGIC);
    image += H5_SIZEOF_MAGIC;

    /* version # */
    *image++ = H5B2_LEAF_VERSION;

    /* B-tree type */
    *image++ = leaf->hdr->cls->id;
    HDassert((size_t)(image - (uint8_t *)_image) == (H5B2_LEAF_PREFIX_SIZE - H5B2_SIZEOF_CHKSUM));

    /* Serialize records for leaf node */
    native = leaf->leaf_native;
    for(u = 0; u < leaf->nrec; u++) {
        /* Encode record */
        if((leaf->hdr->cls->encode)(image, native, leaf->hdr->cb_ctx) < 0)
            HGOTO_ERROR(H5E_BTREE, H5E_CANTENCODE, FAIL, "unable to encode B-tree record")

        /* Move to next record */
        image += leaf->hdr->rrec_size;
        native += leaf->hdr->cls->nrec_size;
    } /* end for */

    /* Compute metadata checksum */
    metadata_chksum = H5_checksum_metadata(_image, (size_t)((const uint8_t *)image - (const uint8_t *)_image), 0);

    /* Metadata checksum */
    UINT32ENCODE(image, metadata_chksum);

    /* Sanity check */
    HDassert((size_t)(image - (uint8_t *)_image) <= len);

    /* Clear rest of leaf node */
    HDmemset(image, 0, len - (size_t)(image - (uint8_t *)_image));

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5B2__cache_leaf_serialize() */


/*-------------------------------------------------------------------------
 * Function:    H5B2__cache_leaf_notify
 *
 * Purpose:     Handle cache action notifications
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Neil Fortner
 *              nfortne2@hdfgroup.org
 *              Apr 25 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_leaf_notify(H5AC_notify_action_t action, void *_thing)
{
    H5B2_leaf_t 	*leaf = (H5B2_leaf_t *)_thing;
    herr_t              ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

    /*
     * Check arguments.
     */
    HDassert(leaf);

    /* Check if the file was opened with SWMR-write access */
    if(leaf->hdr->swmr_write) {
        HDassert(leaf->parent);
        switch(action) {
            case H5AC_NOTIFY_ACTION_AFTER_INSERT:
	    case H5AC_NOTIFY_ACTION_AFTER_LOAD:
                /* Create flush dependency on parent */
                if(H5B2__create_flush_depend((H5AC_info_t *)leaf->parent, (H5AC_info_t *)leaf) < 0)
                    HGOTO_ERROR(H5E_BTREE, H5E_CANTDEPEND, FAIL, "unable to create flush dependency")
                break;

	    case H5AC_NOTIFY_ACTION_AFTER_FLUSH:
                /* do nothing */
                break;

            case H5AC_NOTIFY_ACTION_BEFORE_EVICT:
		/* Destroy flush dependency on parent */
                if(H5B2__destroy_flush_depend((H5AC_info_t *)leaf->parent, (H5AC_info_t *)leaf) < 0)
                    HGOTO_ERROR(H5E_BTREE, H5E_CANTUNDEPEND, FAIL, "unable to destroy flush dependency")
                break;

            default:
#ifdef NDEBUG
                HGOTO_ERROR(H5E_BTREE, H5E_BADVALUE, FAIL, "unknown action from metadata cache")
#else /* NDEBUG */
                HDassert(0 && "Unknown action?!?");
#endif /* NDEBUG */
        } /* end switch */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5B2__cache_leaf_notify() */


/*-------------------------------------------------------------------------
 * Function:	H5B2__cache_leaf_free_icr
 *
 * Purpose:	Destroy/release an "in core representation" of a data
 *              structure
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Mike McGreevy
 *              mcgreevy@hdfgroup.org
 *              June 18, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5B2__cache_leaf_free_icr(void *_thing)
{
    H5B2_leaf_t *leaf = (H5B2_leaf_t *)_thing;
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_STATIC

    /* Check arguments */
    HDassert(leaf);
    HDassert(leaf->hdr);

    /* Unlink from shadowed list */
    if(leaf->shadowed_next) {
        if(leaf->shadowed_next != leaf) {
            leaf->shadowed_next->shadowed_prev = leaf->shadowed_prev;

            if(leaf->shadowed_prev)
                leaf->shadowed_prev->shadowed_next = leaf->shadowed_next;
            else {
                HDassert(leaf->hdr->shadowed_leaf = leaf);

                leaf->hdr->shadowed_leaf = leaf->shadowed_next;
            } /* end else */
        } /* end if */
        else {
            if(leaf->shadowed_prev)
                leaf->shadowed_prev->shadowed_next = leaf->shadowed_prev;
            else {
                HDassert(leaf->hdr->shadowed_leaf = leaf);

                leaf->hdr->shadowed_leaf = NULL;
            } /* end else */
        } /* end else */
    } /* end if */

    /* Destroy v2 B-tree leaf node */
    if(H5B2__leaf_free(leaf) < 0)
        HGOTO_ERROR(H5E_BTREE, H5E_CANTFREE, FAIL, "unable to destroy B-tree leaf node")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5B2__cache_leaf_free_icr() */


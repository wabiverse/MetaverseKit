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
 * Created:		H5ACprivate.h
 *			Jul  9 1997
 *			Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:		Constants and typedefs available to the rest of the
 *			library.
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5ACprivate_H
#define _H5ACprivate_H

#include "H5ACpublic.h"		/*public prototypes			*/

/* Pivate headers needed by this header */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Cprivate.h"		/* Cache				*/
#include "H5Fprivate.h"		/* File access				*/
#include "H5Pprivate.h"		/* Property lists			*/

#ifdef H5_METADATA_TRACE_FILE
#define H5AC__TRACE_FILE_ENABLED	1
#else /* H5_METADATA_TRACE_FILE */
#define H5AC__TRACE_FILE_ENABLED	0
#endif /* H5_METADATA_TRACE_FILE */

/* Global metadata tag values */
#define H5AC__INVALID_TAG      (haddr_t)0
#define H5AC__IGNORE_TAG       (haddr_t)1
#define H5AC__SUPERBLOCK_TAG   (haddr_t)2
#define H5AC__FREESPACE_TAG    (haddr_t)3
#define H5AC__SOHM_TAG         (haddr_t)4
#define H5AC__GLOBALHEAP_TAG   (haddr_t)5
#define H5AC__COPIED_TAG       (haddr_t)6

/* Types of metadata objects cached */
typedef enum {
    H5AC_BT_ID = 0, 			/* ( 0) B-tree nodes				     */
    H5AC_SNODE_ID,			/* ( 1) symbol table nodes			     */
    H5AC_LHEAP_PRFX_ID, 		/* ( 2) local heap prefix			     */
    H5AC_LHEAP_DBLK_ID, 		/* ( 3) local heap data block			     */
    H5AC_GHEAP_ID,			/* ( 4) global heap				     */
    H5AC_OHDR_ID,			/* ( 5) object header				     */
    H5AC_OHDR_CHK_ID,			/* ( 6) object header chunk			     */
    H5AC_OHDR_PROXY_ID, 		/* ( 7) object header proxy                        */
    H5AC_BT2_HDR_ID,			/* ( 8) v2 B-tree header			     */
    H5AC_BT2_INT_ID,			/* ( 9) v2 B-tree internal node		     */
    H5AC_BT2_LEAF_ID,			/* (10) v2 B-tree leaf node			     */
    H5AC_FHEAP_HDR_ID,			/* (11) fractal heap header			     */
    H5AC_FHEAP_DBLOCK_ID, 		/* (12) fractal heap direct block		     */
    H5AC_FHEAP_IBLOCK_ID, 		/* (13) fractal heap indirect block		     */
    H5AC_FSPACE_HDR_ID,			/* (14) free space header			     */
    H5AC_FSPACE_SINFO_ID,		/* (15) free space sections			     */
    H5AC_SOHM_TABLE_ID, 		/* (16) shared object header message master table  */
    H5AC_SOHM_LIST_ID,  		/* (17) shared message index stored as a list      */
    H5AC_EARRAY_HDR_ID,			/* (18) extensible array header		     */
    H5AC_EARRAY_IBLOCK_ID, 		/* (19) extensible array index block	             */
    H5AC_EARRAY_SBLOCK_ID, 		/* (20) extensible array super block	             */
    H5AC_EARRAY_DBLOCK_ID, 		/* (21) extensible array data block	             */
    H5AC_EARRAY_DBLK_PAGE_ID, 		/* (22) extensible array data block page           */
    H5AC_FARRAY_HDR_ID,			/* (23) fixed array header		     	     */
    H5AC_FARRAY_DBLOCK_ID, 		/* (24) fixed array data block	     	     */
    H5AC_FARRAY_DBLK_PAGE_ID,		/* (25) fixed array data block page          	     */
    H5AC_SUPERBLOCK_ID, 		/* (26) file superblock                            */
    H5AC_DRVRINFO_ID,   		/* (27) driver info block (supplements superblock)*/
    H5AC_TEST_ID,			/* (28) test entry -- not used for actual files    */
    H5AC_NTYPES				/* Number of types, must be last              */
} H5AC_type_t;

/* H5AC_DUMP_STATS_ON_CLOSE should always be FALSE when
 * H5C_COLLECT_CACHE_STATS is FALSE.
 *
 * When H5C_COLLECT_CACHE_STATS is TRUE, H5AC_DUMP_STATS_ON_CLOSE must
 * be FALSE for "make check" to succeed, but may be set to TRUE at other
 * times for debugging purposes.
 *
 * Hence the following, somewhat odd set of #defines.
 *
 * NOTE: test/cache plays games with the f->shared->cache, and thus 
 *       setting H5AC_DUMP_STATS_ON_CLOSE will generate constant, 
 *	 irrelevant data when run with that test program.  See 
 * 	 comments on setup_cache() / takedown_cache() in test/cache_common.c.
 *       for details.
 *
 *	 If you need to dump stats at file close in test/cache.c,
 *	 use the dump_stats parameter to takedown_cache(), or call 
 *	 H5C_stats() directly.
 *					JRM -- 4/12/15
 */
#if H5C_COLLECT_CACHE_STATS

#define H5AC_DUMP_STATS_ON_CLOSE	0

#else /* H5C_COLLECT_CACHE_STATS */

#define H5AC_DUMP_STATS_ON_CLOSE	0

#endif /* H5C_COLLECT_CACHE_STATS */

/* Default max metadata cache size and min clean size are give here.
 * At present, these are the same as those given in H5Cprivate.h.
 */

#define H5AC__DEFAULT_MAX_CACHE_SIZE	H5C__DEFAULT_MAX_CACHE_SIZE
#define H5AC__DEFAULT_MIN_CLEAN_SIZE	H5C__DEFAULT_MIN_CLEAN_SIZE

/* Check if we are sanity checking tagging */
#if H5C_DO_TAGGING_SANITY_CHECKS
#define H5AC_DO_TAGGING_SANITY_CHECKS 1
#else
#define H5AC_DO_TAGGING_SANITY_CHECKS 0
#endif

/*
 * Class methods pertaining to caching.	 Each type of cached object will
 * have a constant variable with permanent life-span that describes how
 * to cache the object.
 */

#define H5AC__SERIALIZE_RESIZED_FLAG	H5C__SERIALIZE_RESIZED_FLAG
#define H5AC__SERIALIZE_MOVED_FLAG	H5C__SERIALIZE_MOVED_FLAG
#define H5AC__SERIALIZE_COMPRESSED_FLAG	H5C__SERIALIZE_COMPRESSED_FLAG

/* Cork actions: cork/uncork/get cork status of an object */
#define H5AC__SET_CORK             	H5C__SET_CORK
#define H5AC__UNCORK             	H5C__UNCORK
#define H5AC__GET_CORKED             	H5C__GET_CORKED

/* Aliases for the "ring" type and values */
typedef H5C_ring_t       H5AC_ring_t;
#define H5AC_RING_INV    H5C_RING_UNDEFINED
#define H5AC_RING_US     H5C_RING_USER
#define H5AC_RING_FSM    H5C_RING_FSM
#define H5AC_RING_SBE    H5C_RING_SBE
#define H5AC_RING_SB     H5C_RING_SB
#define H5AC_RING_NTYPES H5C_RING_NTYPES

/* Aliases for 'notify action' type & values */
typedef H5C_notify_action_t     H5AC_notify_action_t;
#define H5AC_NOTIFY_ACTION_AFTER_INSERT H5C_NOTIFY_ACTION_AFTER_INSERT
#define H5AC_NOTIFY_ACTION_AFTER_LOAD   H5C_NOTIFY_ACTION_AFTER_LOAD
#define H5AC_NOTIFY_ACTION_AFTER_FLUSH	H5C_NOTIFY_ACTION_AFTER_FLUSH
#define H5AC_NOTIFY_ACTION_BEFORE_EVICT H5C_NOTIFY_ACTION_BEFORE_EVICT

#define H5AC__CLASS_NO_FLAGS_SET 	H5C__CLASS_NO_FLAGS_SET
#define H5AC__CLASS_SPECULATIVE_LOAD_FLAG H5C__CLASS_SPECULATIVE_LOAD_FLAG
#define H5AC__CLASS_COMPRESSED_FLAG	H5C__CLASS_COMPRESSED_FLAG

/* The following flags should only appear in test code */
/* The H5AC__CLASS_SKIP_READS & H5AC__CLASS_SKIP_WRITES flags are used in H5Oproxy.c */
#define H5AC__CLASS_NO_IO_FLAG		H5C__CLASS_NO_IO_FLAG
#define H5AC__CLASS_SKIP_READS		H5C__CLASS_SKIP_READS
#define H5AC__CLASS_SKIP_WRITES		H5C__CLASS_SKIP_WRITES

typedef H5C_get_load_size_func_t	H5AC_get_load_size_func_t;
typedef H5C_verify_chksum_func_t	H5AC_verify_chksum_func_t;
typedef H5C_deserialize_func_t		H5AC_deserialize_func_t;
typedef H5C_image_len_func_t		H5AC_image_len_func_t;

#define H5AC__SERIALIZE_NO_FLAGS_SET    H5C__SERIALIZE_NO_FLAGS_SET
#define H5AC__SERIALIZE_RESIZED_FLAG    H5C__SERIALIZE_RESIZED_FLAG
#define H5AC__SERIALIZE_MOVED_FLAG      H5C__SERIALIZE_MOVED_FLAG

typedef H5C_pre_serialize_func_t	H5AC_pre_serialize_func_t;
typedef H5C_serialize_func_t		H5AC_serialize_func_t;
typedef H5C_notify_func_t		H5AC_notify_func_t;
typedef H5C_free_icr_func_t		H5AC_free_icr_func_t;
typedef H5C_clear_func_t		H5AC_clear_func_t;
typedef H5C_get_fsf_size_t		H5AC_get_fsf_size_t;

typedef H5C_class_t			H5AC_class_t;

/* Cache entry info */
typedef H5C_cache_entry_t		H5AC_info_t;


/* Typedef for metadata cache (defined in H5Cpkg.h) */
typedef H5C_t	H5AC_t;

#define H5AC_RING_NAME  "H5AC_ring_type"

/* Dataset transfer property lists for metadata calls */
H5_DLLVAR hid_t H5AC_ind_read_dxpl_id;
#ifdef H5_HAVE_PARALLEL
H5_DLLVAR hid_t H5AC_coll_read_dxpl_id;
#endif /* H5_HAVE_PARALLEL */

/* DXPL to be used in operations that will not result in I/O calls */
H5_DLLVAR hid_t H5AC_noio_dxpl_id;

/* DXPL to be used for raw data I/O operations when one is not
   provided by the user (fill values in H5Dcreate) */
H5_DLLVAR hid_t H5AC_rawdata_dxpl_id;

/* Default cache configuration. */

#define H5AC__DEFAULT_METADATA_WRITE_STRATEGY   \
                                H5AC_METADATA_WRITE_STRATEGY__DISTRIBUTED

#ifdef H5_HAVE_PARALLEL
#define H5AC__DEFAULT_CACHE_CONFIG                                            \
{                                                                             \
  /* int         version                = */ H5AC__CURR_CACHE_CONFIG_VERSION, \
  /* hbool_t     rpt_fcn_enabled        = */ FALSE,                           \
  /* hbool_t     open_trace_file        = */ FALSE,                           \
  /* hbool_t     close_trace_file       = */ FALSE,                           \
  /* char        trace_file_name[]      = */ "",                              \
  /* hbool_t     evictions_enabled      = */ TRUE,                            \
  /* hbool_t     set_initial_size       = */ TRUE,                            \
  /* size_t      initial_size           = */ ( 2 * 1024 * 1024),              \
  /* double      min_clean_fraction     = */ 0.3f,                            \
  /* size_t      max_size               = */ (32 * 1024 * 1024),              \
  /* size_t      min_size               = */ (1 * 1024 * 1024),               \
  /* long int    epoch_length           = */ 50000,                           \
  /* enum H5C_cache_incr_mode incr_mode = */ H5C_incr__threshold,             \
  /* double      lower_hr_threshold     = */ 0.9f,                            \
  /* double      increment              = */ 2.0f,                            \
  /* hbool_t     apply_max_increment    = */ TRUE,                            \
  /* size_t      max_increment          = */ (4 * 1024 * 1024),               \
  /* enum H5C_cache_flash_incr_mode       */                                  \
  /*                    flash_incr_mode = */ H5C_flash_incr__add_space,       \
  /* double      flash_multiple         = */ 1.0f,                            \
  /* double      flash_threshold        = */ 0.25f,                           \
  /* enum H5C_cache_decr_mode decr_mode = */ H5C_decr__age_out_with_threshold, \
  /* double      upper_hr_threshold     = */ 0.999f,                          \
  /* double      decrement              = */ 0.9f,                            \
  /* hbool_t     apply_max_decrement    = */ TRUE,                            \
  /* size_t      max_decrement          = */ (1 * 1024 * 1024),               \
  /* int         epochs_before_eviction = */ 3,                               \
  /* hbool_t     apply_empty_reserve    = */ TRUE,                            \
  /* double      empty_reserve          = */ 0.1f,                            \
  /* size_t	 dirty_bytes_threshold  = */ (256 * 1024),                    \
  /* int	metadata_write_strategy = */                                  \
				       H5AC__DEFAULT_METADATA_WRITE_STRATEGY  \
}
#else /* H5_HAVE_PARALLEL */
#define H5AC__DEFAULT_CACHE_CONFIG                                            \
{                                                                             \
  /* int         version                = */ H5C__CURR_AUTO_SIZE_CTL_VER,     \
  /* hbool_t     rpt_fcn_enabled        = */ FALSE,                           \
  /* hbool_t     open_trace_file        = */ FALSE,                           \
  /* hbool_t     close_trace_file       = */ FALSE,                           \
  /* char        trace_file_name[]      = */ "",                              \
  /* hbool_t     evictions_enabled      = */ TRUE,                            \
  /* hbool_t     set_initial_size       = */ TRUE,                            \
  /* size_t      initial_size           = */ ( 2 * 1024 * 1024),              \
  /* double      min_clean_fraction     = */ 0.01f,                           \
  /* size_t      max_size               = */ (32 * 1024 * 1024),              \
  /* size_t      min_size               = */ ( 1 * 1024 * 1024),              \
  /* long int    epoch_length           = */ 50000,                           \
  /* enum H5C_cache_incr_mode incr_mode = */ H5C_incr__threshold,             \
  /* double      lower_hr_threshold     = */ 0.9f,                            \
  /* double      increment              = */ 2.0f,                            \
  /* hbool_t     apply_max_increment    = */ TRUE,                            \
  /* size_t      max_increment          = */ (4 * 1024 * 1024),               \
  /* enum H5C_cache_flash_incr_mode       */                                  \
  /*                    flash_incr_mode = */ H5C_flash_incr__add_space,       \
  /* double      flash_multiple         = */ 1.4f,                            \
  /* double      flash_threshold        = */ 0.25f,                           \
  /* enum H5C_cache_decr_mode decr_mode = */ H5C_decr__age_out_with_threshold,\
  /* double      upper_hr_threshold     = */ 0.999f,                          \
  /* double      decrement              = */ 0.9f,                            \
  /* hbool_t     apply_max_decrement    = */ TRUE,                            \
  /* size_t      max_decrement          = */ (1 * 1024 * 1024),               \
  /* int         epochs_before_eviction = */ 3,                               \
  /* hbool_t     apply_empty_reserve    = */ TRUE,                            \
  /* double      empty_reserve          = */ 0.1f,                            \
  /* size_t	 dirty_bytes_threshold  = */ (256 * 1024),                    \
  /* int	metadata_write_strategy = */                                  \
				       H5AC__DEFAULT_METADATA_WRITE_STRATEGY  \
}
#endif /* H5_HAVE_PARALLEL */


/*
 * Library prototypes.
 */

/* #defines of flags used in the flags parameters in some of the
 * following function calls.  Note that they are just copies of
 * the equivalent flags from H5Cprivate.h.
 */

#define H5AC__NO_FLAGS_SET		  H5C__NO_FLAGS_SET
#define H5AC__SET_FLUSH_MARKER_FLAG	  H5C__SET_FLUSH_MARKER_FLAG
#define H5AC__DELETED_FLAG		  H5C__DELETED_FLAG
#define H5AC__DIRTIED_FLAG		  H5C__DIRTIED_FLAG
#define H5AC__PIN_ENTRY_FLAG		  H5C__PIN_ENTRY_FLAG
#define H5AC__UNPIN_ENTRY_FLAG		  H5C__UNPIN_ENTRY_FLAG
#define H5AC__FLUSH_INVALIDATE_FLAG	  H5C__FLUSH_INVALIDATE_FLAG
#define H5AC__FLUSH_CLEAR_ONLY_FLAG	  H5C__FLUSH_CLEAR_ONLY_FLAG
#define H5AC__FLUSH_MARKED_ENTRIES_FLAG   H5C__FLUSH_MARKED_ENTRIES_FLAG
#define H5AC__FLUSH_IGNORE_PROTECTED_FLAG H5C__FLUSH_IGNORE_PROTECTED_FLAG
#define H5AC__READ_ONLY_FLAG		  H5C__READ_ONLY_FLAG
#define H5AC__FREE_FILE_SPACE_FLAG	  H5C__FREE_FILE_SPACE_FLAG
#define H5AC__TAKE_OWNERSHIP_FLAG         H5C__TAKE_OWNERSHIP_FLAG
#define H5AC__FLUSH_LAST_FLAG		  H5C__FLUSH_LAST_FLAG
#define H5AC__FLUSH_COLLECTIVELY_FLAG	  H5C__FLUSH_COLLECTIVELY_FLAG
#define H5AC__EVICT_ALLOW_LAST_PINS_FLAG  H5C__EVICT_ALLOW_LAST_PINS_FLAG


/* #defines of flags used to report entry status in the
 * H5AC_get_entry_status() call.
 */

#define H5AC_ES__IN_CACHE	0x0001
#define H5AC_ES__IS_DIRTY	0x0002
#define H5AC_ES__IS_PROTECTED	0x0004
#define H5AC_ES__IS_PINNED	0x0008
#define H5AC_ES__IS_FLUSH_DEP_PARENT	0x0010
#define H5AC_ES__IS_FLUSH_DEP_CHILD	0x0020
#define H5AC_ES__IS_CORKED	0x0040


/* external function declarations: */

H5_DLL herr_t H5AC_init(void);
H5_DLL herr_t H5AC_create(const H5F_t *f, H5AC_cache_config_t *config_ptr);
H5_DLL herr_t H5AC_get_entry_status(const H5F_t *f, haddr_t addr,
    unsigned *status_ptr);
H5_DLL herr_t H5AC_insert_entry(H5F_t *f, hid_t dxpl_id, const H5AC_class_t *type,
    haddr_t addr, void *thing, unsigned int flags);
H5_DLL herr_t H5AC_pin_protected_entry(void *thing);
H5_DLL herr_t H5AC_create_flush_dependency(void *parent_thing, void *child_thing);
H5_DLL void * H5AC_protect(H5F_t *f, hid_t dxpl_id, const H5AC_class_t *type,
    haddr_t addr, void *udata, unsigned flags);
H5_DLL herr_t H5AC_resize_entry(void *thing, size_t new_size);
H5_DLL herr_t H5AC_unpin_entry(void *thing);
H5_DLL herr_t H5AC_destroy_flush_dependency(void *parent_thing, void *child_thing);
H5_DLL herr_t H5AC_unprotect(H5F_t *f, hid_t dxpl_id, const H5AC_class_t *type,
    haddr_t addr, void *thing, unsigned flags);
H5_DLL herr_t H5AC_flush(H5F_t *f, hid_t dxpl_id);
H5_DLL herr_t H5AC_mark_entry_dirty(void *thing);
H5_DLL herr_t H5AC_move_entry(H5F_t *f, const H5AC_class_t *type,
    haddr_t old_addr, haddr_t new_addr, hid_t dxpl_id);
H5_DLL herr_t H5AC_dest(H5F_t *f, hid_t dxpl_id);
H5_DLL herr_t H5AC_evict(H5F_t *f, hid_t dxpl_id);
H5_DLL herr_t H5AC_expunge_entry(H5F_t *f, hid_t dxpl_id,
    const H5AC_class_t *type, haddr_t addr, unsigned flags);
H5_DLL herr_t H5AC_get_cache_auto_resize_config(const H5AC_t * cache_ptr,
    H5AC_cache_config_t *config_ptr);
H5_DLL herr_t H5AC_get_cache_size(H5AC_t *cache_ptr, size_t *max_size_ptr,
    size_t *min_clean_size_ptr, size_t *cur_size_ptr, int32_t *cur_num_entries_ptr);
H5_DLL herr_t H5AC_get_cache_hit_rate(H5AC_t *cache_ptr, double *hit_rate_ptr);
H5_DLL herr_t H5AC_reset_cache_hit_rate_stats(H5AC_t *cache_ptr);
H5_DLL herr_t H5AC_set_cache_auto_resize_config(H5AC_t *cache_ptr,
    H5AC_cache_config_t *config_ptr);
H5_DLL herr_t H5AC_validate_config(H5AC_cache_config_t *config_ptr);
H5_DLL herr_t H5AC_close_trace_file(H5AC_t *cache_ptr);
H5_DLL herr_t H5AC_open_trace_file(H5AC_t *cache_ptr, const char *trace_file_name);

/* Tag & Ring routines */
H5_DLL herr_t H5AC_tag(hid_t dxpl_id, haddr_t metadata_tag, haddr_t *prev_tag);
H5_DLL herr_t H5AC_flush_tagged_metadata(H5F_t * f, haddr_t metadata_tag, hid_t dxpl_id);
H5_DLL herr_t H5AC_evict_tagged_metadata(H5F_t * f, haddr_t metadata_tag, hid_t dxpl_id);
H5_DLL herr_t H5AC_retag_copied_metadata(const H5F_t *f, haddr_t metadata_tag);
H5_DLL herr_t H5AC_ignore_tags(const H5F_t *f);
H5_DLL herr_t H5AC_cork(H5F_t *f, haddr_t obj_addr, unsigned action, hbool_t *corked);
H5_DLL herr_t H5AC_get_entry_ring(const H5F_t *f, haddr_t addr, H5AC_ring_t *ring);
H5_DLL herr_t H5AC_set_ring(hid_t dxpl_id, H5AC_ring_t ring, H5P_genplist_t **dxpl,
    H5AC_ring_t *orig_ring);
H5_DLL herr_t H5AC_reset_ring(H5P_genplist_t *dxpl, H5AC_ring_t orig_ring);

#ifdef H5_HAVE_PARALLEL
H5_DLL herr_t H5AC_add_candidate(H5AC_t * cache_ptr, haddr_t addr);
#endif /* H5_HAVE_PARALLEL */

#ifndef NDEBUG  /* debugging functions */
H5_DLL herr_t H5AC_stats(const H5F_t *f);
H5_DLL herr_t H5AC_dump_cache(const H5F_t *f);
H5_DLL herr_t H5AC_get_entry_ptr_from_addr(const H5F_t *f, haddr_t addr,
    void ** entry_ptr_ptr);
H5_DLL herr_t H5AC_verify_entry_type(const H5F_t * f, haddr_t addr,
    const H5AC_class_t * expected_type, hbool_t * in_cache_ptr,
    hbool_t * type_ok_ptr);
#endif /* NDEBUG */ /* end debugging functions */

#endif /* !_H5ACprivate_H */


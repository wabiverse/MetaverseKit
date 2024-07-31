/*
 * ompt-general.cpp -- OMPT implementation of interface functions
 */

//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

/*****************************************************************************
 * system include files
 ****************************************************************************/

#include <assert.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if KMP_OS_UNIX
#include <dlfcn.h>
#endif

/*****************************************************************************
 * ompt include files
 ****************************************************************************/

/*
 * ompt-specific.cpp -- OMPT internal functions
 */

//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//******************************************************************************
// include files
//******************************************************************************

#include "kmp.h"
#include "ompt-specific.h"

#if KMP_OS_UNIX
#include <dlfcn.h>
#endif

#if KMP_OS_WINDOWS
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif

#define OMPT_WEAK_ATTRIBUTE KMP_WEAK_ATTRIBUTE_INTERNAL

//******************************************************************************
// macros
//******************************************************************************

#define LWT_FROM_TEAM(team) (team)->t.ompt_serialized_team_info

#define OMPT_THREAD_ID_BITS 16

//******************************************************************************
// private operations
//******************************************************************************

//----------------------------------------------------------
// traverse the team and task hierarchy
// note: __ompt_get_teaminfo and __ompt_get_task_info_object
//       traverse the hierarchy similarly and need to be
//       kept consistent
//----------------------------------------------------------

ompt_team_info_t *__ompt_get_teaminfo(int depth, int *size) {
  kmp_info_t *thr = ompt_get_thread();

  if (thr) {
    kmp_team *team = thr->th.th_team;
    if (team == NULL)
      return NULL;

    ompt_lw_taskteam_t *next_lwt = LWT_FROM_TEAM(team), *lwt = NULL;

    while (depth > 0) {
      // next lightweight team (if any)
      if (lwt)
        lwt = lwt->parent;

      // next heavyweight team (if any) after
      // lightweight teams are exhausted
      if (!lwt && team) {
        if (next_lwt) {
          lwt = next_lwt;
          next_lwt = NULL;
        } else {
          team = team->t.t_parent;
          if (team) {
            next_lwt = LWT_FROM_TEAM(team);
          }
        }
      }

      depth--;
    }

    if (lwt) {
      // lightweight teams have one task
      if (size)
        *size = 1;

      // return team info for lightweight team
      return &lwt->ompt_team_info;
    } else if (team) {
      // extract size from heavyweight team
      if (size)
        *size = team->t.t_nproc;

      // return team info for heavyweight team
      return &team->t.ompt_team_info;
    }
  }

  return NULL;
}

ompt_task_info_t *__ompt_get_task_info_object(int depth) {
  ompt_task_info_t *info = NULL;
  kmp_info_t *thr = ompt_get_thread();

  if (thr) {
    kmp_taskdata_t *taskdata = thr->th.th_current_task;
    ompt_lw_taskteam_t *lwt = NULL,
                       *next_lwt = LWT_FROM_TEAM(taskdata->td_team);

    while (depth > 0) {
      // next lightweight team (if any)
      if (lwt)
        lwt = lwt->parent;

      // next heavyweight team (if any) after
      // lightweight teams are exhausted
      if (!lwt && taskdata) {
        if (next_lwt) {
          lwt = next_lwt;
          next_lwt = NULL;
        } else {
          taskdata = taskdata->td_parent;
          if (taskdata) {
            next_lwt = LWT_FROM_TEAM(taskdata->td_team);
          }
        }
      }
      depth--;
    }

    if (lwt) {
      info = &lwt->ompt_task_info;
    } else if (taskdata) {
      info = &taskdata->ompt_task_info;
    }
  }

  return info;
}

ompt_task_info_t *__ompt_get_scheduling_taskinfo(int depth) {
  ompt_task_info_t *info = NULL;
  kmp_info_t *thr = ompt_get_thread();

  if (thr) {
    kmp_taskdata_t *taskdata = thr->th.th_current_task;

    ompt_lw_taskteam_t *lwt = NULL,
                       *next_lwt = LWT_FROM_TEAM(taskdata->td_team);

    while (depth > 0) {
      // next lightweight team (if any)
      if (lwt)
        lwt = lwt->parent;

      // next heavyweight team (if any) after
      // lightweight teams are exhausted
      if (!lwt && taskdata) {
        // first try scheduling parent (for explicit task scheduling)
        if (taskdata->ompt_task_info.scheduling_parent) {
          taskdata = taskdata->ompt_task_info.scheduling_parent;
        } else if (next_lwt) {
          lwt = next_lwt;
          next_lwt = NULL;
        } else {
          // then go for implicit tasks
          taskdata = taskdata->td_parent;
          if (taskdata) {
            next_lwt = LWT_FROM_TEAM(taskdata->td_team);
          }
        }
      }
      depth--;
    }

    if (lwt) {
      info = &lwt->ompt_task_info;
    } else if (taskdata) {
      info = &taskdata->ompt_task_info;
    }
  }

  return info;
}

//******************************************************************************
// interface operations
//******************************************************************************

//----------------------------------------------------------
// thread support
//----------------------------------------------------------

ompt_data_t *__ompt_get_thread_data_internal() {
  if (__kmp_get_gtid() >= 0) {
    kmp_info_t *thread = ompt_get_thread();
    if (thread == NULL)
      return NULL;
    return &(thread->th.ompt_thread_info.thread_data);
  }
  return NULL;
}

//----------------------------------------------------------
// state support
//----------------------------------------------------------

void __ompt_thread_assign_wait_id(void *variable) {
  kmp_info_t *ti = ompt_get_thread();

  if (ti)
    ti->th.ompt_thread_info.wait_id = (ompt_wait_id_t)(uintptr_t)variable;
}

int __ompt_get_state_internal(ompt_wait_id_t *omp_wait_id) {
  kmp_info_t *ti = ompt_get_thread();

  if (ti) {
    if (omp_wait_id)
      *omp_wait_id = ti->th.ompt_thread_info.wait_id;
    return ti->th.ompt_thread_info.state;
  }
  return ompt_state_undefined;
}

//----------------------------------------------------------
// parallel region support
//----------------------------------------------------------

int __ompt_get_parallel_info_internal(int ancestor_level,
                                      ompt_data_t **parallel_data,
                                      int *team_size) {
  if (__kmp_get_gtid() >= 0) {
    ompt_team_info_t *info;
    if (team_size) {
      info = __ompt_get_teaminfo(ancestor_level, team_size);
    } else {
      info = __ompt_get_teaminfo(ancestor_level, NULL);
    }
    if (parallel_data) {
      *parallel_data = info ? &(info->parallel_data) : NULL;
    }
    return info ? 2 : 0;
  } else {
    return 0;
  }
}

//----------------------------------------------------------
// lightweight task team support
//----------------------------------------------------------

void __ompt_lw_taskteam_init(ompt_lw_taskteam_t *lwt, kmp_info_t *thr, int gtid,
                             ompt_data_t *ompt_pid, void *codeptr) {
  // initialize parallel_data with input, return address to parallel_data on
  // exit
  lwt->ompt_team_info.parallel_data = *ompt_pid;
  lwt->ompt_team_info.master_return_address = codeptr;
  lwt->ompt_task_info.task_data.value = 0;
  lwt->ompt_task_info.frame.enter_frame = ompt_data_none;
  lwt->ompt_task_info.frame.exit_frame = ompt_data_none;
  lwt->ompt_task_info.scheduling_parent = NULL;
  lwt->heap = 0;
  lwt->parent = 0;
}

void __ompt_lw_taskteam_link(ompt_lw_taskteam_t *lwt, kmp_info_t *thr,
                             int on_heap, bool always) {
  ompt_lw_taskteam_t *link_lwt = lwt;
  if (always ||
      thr->th.th_team->t.t_serialized >
          1) { // we already have a team, so link the new team and swap values
    if (on_heap) { // the lw_taskteam cannot stay on stack, allocate it on heap
      link_lwt =
          (ompt_lw_taskteam_t *)__kmp_allocate(sizeof(ompt_lw_taskteam_t));
    }
    link_lwt->heap = on_heap;

    // would be swap in the (on_stack) case.
    ompt_team_info_t tmp_team = lwt->ompt_team_info;
    link_lwt->ompt_team_info = *OMPT_CUR_TEAM_INFO(thr);
    *OMPT_CUR_TEAM_INFO(thr) = tmp_team;

    // link the taskteam into the list of taskteams:
    ompt_lw_taskteam_t *my_parent =
        thr->th.th_team->t.ompt_serialized_team_info;
    link_lwt->parent = my_parent;
    thr->th.th_team->t.ompt_serialized_team_info = link_lwt;
#if OMPD_SUPPORT
    if (ompd_state & OMPD_ENABLE_BP) {
      ompd_bp_parallel_begin();
    }
#endif

    ompt_task_info_t tmp_task = lwt->ompt_task_info;
    link_lwt->ompt_task_info = *OMPT_CUR_TASK_INFO(thr);
    *OMPT_CUR_TASK_INFO(thr) = tmp_task;
  } else {
    // this is the first serialized team, so we just store the values in the
    // team and drop the taskteam-object
    *OMPT_CUR_TEAM_INFO(thr) = lwt->ompt_team_info;
#if OMPD_SUPPORT
    if (ompd_state & OMPD_ENABLE_BP) {
      ompd_bp_parallel_begin();
    }
#endif
    *OMPT_CUR_TASK_INFO(thr) = lwt->ompt_task_info;
  }
}

void __ompt_lw_taskteam_unlink(kmp_info_t *thr) {
  ompt_lw_taskteam_t *lwtask = thr->th.th_team->t.ompt_serialized_team_info;
  if (lwtask) {
    ompt_task_info_t tmp_task = lwtask->ompt_task_info;
    lwtask->ompt_task_info = *OMPT_CUR_TASK_INFO(thr);
    *OMPT_CUR_TASK_INFO(thr) = tmp_task;
#if OMPD_SUPPORT
    if (ompd_state & OMPD_ENABLE_BP) {
      ompd_bp_parallel_end();
    }
#endif
    thr->th.th_team->t.ompt_serialized_team_info = lwtask->parent;

    ompt_team_info_t tmp_team = lwtask->ompt_team_info;
    lwtask->ompt_team_info = *OMPT_CUR_TEAM_INFO(thr);
    *OMPT_CUR_TEAM_INFO(thr) = tmp_team;

    if (lwtask->heap) {
      __kmp_free(lwtask);
      lwtask = NULL;
    }
  }
  //    return lwtask;
}

//----------------------------------------------------------
// task support
//----------------------------------------------------------

int __ompt_get_task_info_internal(int ancestor_level, int *type,
                                  ompt_data_t **task_data,
                                  ompt_frame_t **task_frame,
                                  ompt_data_t **parallel_data,
                                  int *thread_num) {
  if (__kmp_get_gtid() < 0)
    return 0;

  if (ancestor_level < 0)
    return 0;

  // copied from __ompt_get_scheduling_taskinfo
  ompt_task_info_t *info = NULL;
  ompt_team_info_t *team_info = NULL;
  kmp_info_t *thr = ompt_get_thread();
  int level = ancestor_level;

  if (thr) {
    kmp_taskdata_t *taskdata = thr->th.th_current_task;
    if (taskdata == NULL)
      return 0;
    kmp_team *team = thr->th.th_team, *prev_team = NULL;
    if (team == NULL)
      return 0;
    ompt_lw_taskteam_t *lwt = NULL,
                       *next_lwt = LWT_FROM_TEAM(taskdata->td_team);

    while (ancestor_level > 0) {
      // next lightweight team (if any)
      if (lwt)
        lwt = lwt->parent;

      // next heavyweight team (if any) after
      // lightweight teams are exhausted
      if (!lwt && taskdata) {
        // first try scheduling parent (for explicit task scheduling)
        if (taskdata->ompt_task_info.scheduling_parent) {
          taskdata = taskdata->ompt_task_info.scheduling_parent;
        } else if (next_lwt) {
          lwt = next_lwt;
          next_lwt = NULL;
        } else {
          // then go for implicit tasks
          taskdata = taskdata->td_parent;
          if (team == NULL)
            return 0;
          prev_team = team;
          team = team->t.t_parent;
          if (taskdata) {
            next_lwt = LWT_FROM_TEAM(taskdata->td_team);
          }
        }
      }
      ancestor_level--;
    }

    if (lwt) {
      info = &lwt->ompt_task_info;
      team_info = &lwt->ompt_team_info;
      if (type) {
        *type = ompt_task_implicit;
      }
    } else if (taskdata) {
      info = &taskdata->ompt_task_info;
      team_info = &team->t.ompt_team_info;
      if (type) {
        if (taskdata->td_parent) {
          *type = (taskdata->td_flags.tasktype ? ompt_task_explicit
                                               : ompt_task_implicit) |
                  TASK_TYPE_DETAILS_FORMAT(taskdata);
        } else {
          *type = ompt_task_initial;
        }
      }
    }
    if (task_data) {
      *task_data = info ? &info->task_data : NULL;
    }
    if (task_frame) {
      // OpenMP spec asks for the scheduling task to be returned.
      *task_frame = info ? &info->frame : NULL;
    }
    if (parallel_data) {
      *parallel_data = team_info ? &(team_info->parallel_data) : NULL;
    }
    if (thread_num) {
      if (level == 0)
        *thread_num = __kmp_get_tid();
      else if (lwt)
        *thread_num = 0;
      else if (!prev_team) {
        // The innermost parallel region contains at least one explicit task.
        // The task at level > 0 is either an implicit task that
        // corresponds to the mentioned region or one of the explicit tasks
        // nested inside the same region. Note that the task isn't the
        // innermost explicit tasks (because of condition level > 0).
        // Since the task at this level still belongs to the innermost parallel
        // region, thread_num is determined the same way as for level==0.
        *thread_num = __kmp_get_tid();
      } else
        *thread_num = prev_team->t.t_master_tid;
      //        *thread_num = team->t.t_master_tid;
    }
    return info ? 2 : 0;
  }
  return 0;
}

int __ompt_get_task_memory_internal(void **addr, size_t *size, int blocknum) {
  if (blocknum != 0)
    return 0; // support only a single block

  kmp_info_t *thr = ompt_get_thread();
  if (!thr)
    return 0;

  kmp_taskdata_t *taskdata = thr->th.th_current_task;
  kmp_task_t *task = KMP_TASKDATA_TO_TASK(taskdata);

  if (taskdata->td_flags.tasktype != TASK_EXPLICIT)
    return 0; // support only explicit task

  void *ret_addr;
  int64_t ret_size = taskdata->td_size_alloc - sizeof(kmp_taskdata_t);

  // kmp_task_t->data1 is an optional member
  if (taskdata->td_flags.destructors_thunk)
    ret_addr = &task->data1 + 1;
  else
    ret_addr = &task->part_id + 1;

  ret_size -= (char *)(ret_addr) - (char *)(task);
  if (ret_size < 0)
    return 0;

  *addr = ret_addr;
  *size = (size_t)ret_size;
  return 1;
}

//----------------------------------------------------------
// team support
//----------------------------------------------------------

void __ompt_team_assign_id(kmp_team_t *team, ompt_data_t ompt_pid) {
  team->t.ompt_team_info.parallel_data = ompt_pid;
}

//----------------------------------------------------------
// misc
//----------------------------------------------------------

static uint64_t __ompt_get_unique_id_internal() {
  static uint64_t thread = 1;
  static THREAD_LOCAL uint64_t ID = 0;
  if (ID == 0) {
    uint64_t new_thread = KMP_TEST_THEN_INC64((kmp_int64 *)&thread);
    ID = new_thread << (sizeof(uint64_t) * 8 - OMPT_THREAD_ID_BITS);
  }
  return ++ID;
}

ompt_sync_region_t __ompt_get_barrier_kind(enum barrier_type bt,
                                           kmp_info_t *thr) {
  if (bt == bs_forkjoin_barrier)
    return ompt_sync_region_barrier_implicit;

  if (bt != bs_plain_barrier)
    return ompt_sync_region_barrier_implementation;

  if (!thr->th.th_ident)
    return ompt_sync_region_barrier;

  kmp_int32 flags = thr->th.th_ident->flags;

  if ((flags & KMP_IDENT_BARRIER_EXPL) != 0)
    return ompt_sync_region_barrier_explicit;

  if ((flags & KMP_IDENT_BARRIER_IMPL) != 0)
    return ompt_sync_region_barrier_implicit;

  return ompt_sync_region_barrier_implementation;
}


/*****************************************************************************
 * macros
 ****************************************************************************/

#define ompt_get_callback_success 1
#define ompt_get_callback_failure 0

#define no_tool_present 0

#define OMPT_API_ROUTINE static

#ifndef OMPT_STR_MATCH
#define OMPT_STR_MATCH(haystack, needle) (!strcasecmp(haystack, needle))
#endif

// prints for an enabled OMP_TOOL_VERBOSE_INIT.
// In the future a prefix could be added in the first define, the second define
// omits the prefix to allow for continued lines. Example: "PREFIX: Start
// tool... Success." instead of "PREFIX: Start tool... PREFIX: Success."
#define OMPT_VERBOSE_INIT_PRINT(...)                                           \
  if (verbose_init)                                                            \
  fprintf(verbose_file, __VA_ARGS__)
#define OMPT_VERBOSE_INIT_CONTINUED_PRINT(...)                                 \
  if (verbose_init)                                                            \
  fprintf(verbose_file, __VA_ARGS__)

static FILE *verbose_file;
static int verbose_init;

/*****************************************************************************
 * types
 ****************************************************************************/

typedef struct {
  const char *state_name;
  ompt_state_t state_id;
} ompt_state_info_t;

typedef struct {
  const char *name;
  kmp_mutex_impl_t id;
} kmp_mutex_impl_info_t;

enum tool_setting_e {
  omp_tool_error,
  omp_tool_unset,
  omp_tool_disabled,
  omp_tool_enabled
};

/*****************************************************************************
 * global variables
 ****************************************************************************/

ompt_callbacks_active_t ompt_enabled;

ompt_state_info_t ompt_state_info[] = {
#define ompt_state_macro(state, code) {#state, state},
    FOREACH_OMPT_STATE(ompt_state_macro)
#undef ompt_state_macro
};

kmp_mutex_impl_info_t kmp_mutex_impl_info[] = {
#define kmp_mutex_impl_macro(name, id) {#name, name},
    FOREACH_KMP_MUTEX_IMPL(kmp_mutex_impl_macro)
#undef kmp_mutex_impl_macro
};

ompt_callbacks_internal_t ompt_callbacks;

static ompt_start_tool_result_t *ompt_start_tool_result = NULL;

#if KMP_OS_WINDOWS
static HMODULE ompt_tool_module = NULL;
#define OMPT_DLCLOSE(Lib) FreeLibrary(Lib)
#else
static void *ompt_tool_module = NULL;
#define OMPT_DLCLOSE(Lib) dlclose(Lib)
#endif

/*****************************************************************************
 * forward declarations
 ****************************************************************************/

static ompt_interface_fn_t ompt_fn_lookup(const char *s);

OMPT_API_ROUTINE ompt_data_t *ompt_get_thread_data(void);

/*****************************************************************************
 * initialization and finalization (private operations)
 ****************************************************************************/

typedef ompt_start_tool_result_t *(*ompt_start_tool_t)(unsigned int,
                                                       const char *);

#if KMP_OS_DARWIN

// While Darwin supports weak symbols, the library that wishes to provide a new
// implementation has to link against this runtime which defeats the purpose
// of having tools that are agnostic of the underlying runtime implementation.
//
// Fortunately, the linker includes all symbols of an executable in the global
// symbol table by default so dlsym() even finds static implementations of
// ompt_start_tool. For this to work on Linux, -Wl,--export-dynamic needs to be
// passed when building the application which we don't want to rely on.

static ompt_start_tool_result_t *ompt_tool_darwin(unsigned int omp_version,
                                                  const char *runtime_version) {
  ompt_start_tool_result_t *ret = NULL;
  // Search symbol in the current address space.
  ompt_start_tool_t start_tool =
      (ompt_start_tool_t)dlsym(RTLD_DEFAULT, "ompt_start_tool");
  if (start_tool) {
    ret = start_tool(omp_version, runtime_version);
  }
  return ret;
}

#elif OMPT_HAVE_WEAK_ATTRIBUTE

// On Unix-like systems that support weak symbols the following implementation
// of ompt_start_tool() will be used in case no tool-supplied implementation of
// this function is present in the address space of a process.

_OMP_EXTERN OMPT_WEAK_ATTRIBUTE ompt_start_tool_result_t *
ompt_start_tool(unsigned int omp_version, const char *runtime_version) {
  ompt_start_tool_result_t *ret = NULL;
  // Search next symbol in the current address space. This can happen if the
  // runtime library is linked before the tool. Since glibc 2.2 strong symbols
  // don't override weak symbols that have been found before unless the user
  // sets the environment variable LD_DYNAMIC_WEAK.
  ompt_start_tool_t next_tool = (ompt_start_tool_t)KMP_DLSYM_NEXT("ompt_start_tool");
  if (next_tool) {
    ret = next_tool(omp_version, runtime_version);
  }
  return ret;
}

#elif OMPT_HAVE_PSAPI

// On Windows, the ompt_tool_windows function is used to find the
// ompt_start_tool symbol across all modules loaded by a process. If
// ompt_start_tool is found, ompt_start_tool's return value is used to
// initialize the tool. Otherwise, NULL is returned and OMPT won't be enabled.

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

// The number of loaded modules to start enumeration with EnumProcessModules()
#define NUM_MODULES 128

static ompt_start_tool_result_t *
ompt_tool_windows(unsigned int omp_version, const char *runtime_version) {
  int i;
  DWORD needed, new_size;
  HMODULE *modules;
  HANDLE process = GetCurrentProcess();
  modules = (HMODULE *)malloc(NUM_MODULES * sizeof(HMODULE));
  ompt_start_tool_t ompt_tool_p = NULL;

#if OMPT_DEBUG
  printf("ompt_tool_windows(): looking for ompt_start_tool\n");
#endif
  if (!EnumProcessModules(process, modules, NUM_MODULES * sizeof(HMODULE),
                          &needed)) {
    // Regardless of the error reason use the stub initialization function
    free(modules);
    return NULL;
  }
  // Check if NUM_MODULES is enough to list all modules
  new_size = needed / sizeof(HMODULE);
  if (new_size > NUM_MODULES) {
#if OMPT_DEBUG
    printf("ompt_tool_windows(): resize buffer to %d bytes\n", needed);
#endif
    modules = (HMODULE *)realloc(modules, needed);
    // If resizing failed use the stub function.
    if (!EnumProcessModules(process, modules, needed, &needed)) {
      free(modules);
      return NULL;
    }
  }
  for (i = 0; i < new_size; ++i) {
    (FARPROC &)ompt_tool_p = GetProcAddress(modules[i], "ompt_start_tool");
    if (ompt_tool_p) {
#if OMPT_DEBUG
      TCHAR modName[MAX_PATH];
      if (GetModuleFileName(modules[i], modName, MAX_PATH))
        printf("ompt_tool_windows(): ompt_start_tool found in module %s\n",
               modName);
#endif
      free(modules);
      return (*ompt_tool_p)(omp_version, runtime_version);
    }
#if OMPT_DEBUG
    else {
      TCHAR modName[MAX_PATH];
      if (GetModuleFileName(modules[i], modName, MAX_PATH))
        printf("ompt_tool_windows(): ompt_start_tool not found in module %s\n",
               modName);
    }
#endif
  }
  free(modules);
  return NULL;
}
#else
#error Activation of OMPT is not supported on this platform.
#endif

#if KMP_OS_WINDOWS
/** 
 * dlerror() does not exist on windows,
 * this is a custom implementation. */
static char* dlerror()
{
  LPVOID lpMsgBuf;
  char* win32Error;
  if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL)) {
    win32Error = (LPSTR)lpMsgBuf;
  }
  LocalFree(lpMsgBuf);
  return win32Error;
}
#endif // KMP_OS_WINDOWS

static ompt_start_tool_result_t *
ompt_try_start_tool(unsigned int omp_version, const char *runtime_version) {
  ompt_start_tool_result_t *ret = NULL;
  ompt_start_tool_t start_tool = NULL;
#if KMP_OS_WINDOWS
  // Cannot use colon to describe a list of absolute paths on Windows
  const char *sep = ";";
#else
  const char *sep = ":";
#endif

  OMPT_VERBOSE_INIT_PRINT("----- START LOGGING OF TOOL REGISTRATION -----\n");
  OMPT_VERBOSE_INIT_PRINT("Search for OMP tool in current address space... ");

#if KMP_OS_DARWIN
  // Try in the current address space
  ret = ompt_tool_darwin(omp_version, runtime_version);
#elif OMPT_HAVE_WEAK_ATTRIBUTE
  ret = ompt_start_tool(omp_version, runtime_version);
#elif OMPT_HAVE_PSAPI
  ret = ompt_tool_windows(omp_version, runtime_version);
#else
#error Activation of OMPT is not supported on this platform.
#endif
  if (ret) {
    OMPT_VERBOSE_INIT_CONTINUED_PRINT("Success.\n");
    OMPT_VERBOSE_INIT_PRINT(
        "Tool was started and is using the OMPT interface.\n");
    OMPT_VERBOSE_INIT_PRINT("----- END LOGGING OF TOOL REGISTRATION -----\n");
    return ret;
  }

  // Try tool-libraries-var ICV
  OMPT_VERBOSE_INIT_CONTINUED_PRINT("Failed.\n");
  const char *tool_libs = getenv("OMP_TOOL_LIBRARIES");
  if (tool_libs) {
    OMPT_VERBOSE_INIT_PRINT("Searching tool libraries...\n");
    OMPT_VERBOSE_INIT_PRINT("OMP_TOOL_LIBRARIES = %s\n", tool_libs);
    char *libs = __kmp_str_format("%s", tool_libs);
    char *buf;
    char *fname = __kmp_str_token(libs, sep, &buf);
    // Reset dl-error
    dlerror();

    while (fname) {
#if KMP_OS_UNIX
      OMPT_VERBOSE_INIT_PRINT("Opening %s... ", fname);
      void *h = dlopen(fname, RTLD_LAZY);
      if (!h) {
        OMPT_VERBOSE_INIT_CONTINUED_PRINT("Failed: %s\n", dlerror());
      } else {
        OMPT_VERBOSE_INIT_CONTINUED_PRINT("Success. \n");
        OMPT_VERBOSE_INIT_PRINT("Searching for ompt_start_tool in %s... ",
                                fname);
        dlerror(); // Clear any existing error
        start_tool = (ompt_start_tool_t)dlsym(h, "ompt_start_tool");
        if (!start_tool) {
          char *error = dlerror();
          if (error != NULL) {
            OMPT_VERBOSE_INIT_CONTINUED_PRINT("Failed: %s\n", error);
          } else {
            OMPT_VERBOSE_INIT_CONTINUED_PRINT("Failed: %s\n",
                                              "ompt_start_tool = NULL");
          }
        } else
#elif KMP_OS_WINDOWS
      OMPT_VERBOSE_INIT_PRINT("Opening %s... ", fname);
      HMODULE h = LoadLibrary(fname);
      if (!h) {
        OMPT_VERBOSE_INIT_CONTINUED_PRINT("Failed: Error %u\n",
                                          (unsigned)GetLastError());
      } else {
        OMPT_VERBOSE_INIT_CONTINUED_PRINT("Success. \n");
        OMPT_VERBOSE_INIT_PRINT("Searching for ompt_start_tool in %s... ",
                                fname);
        start_tool = (ompt_start_tool_t)GetProcAddress(h, "ompt_start_tool");
        if (!start_tool) {
          OMPT_VERBOSE_INIT_CONTINUED_PRINT("Failed: Error %u\n",
                                            (unsigned)GetLastError());
        } else
#else
#error Activation of OMPT is not supported on this platform.
#endif
        { // if (start_tool)
          ret = (*start_tool)(omp_version, runtime_version);
          if (ret) {
            OMPT_VERBOSE_INIT_CONTINUED_PRINT("Success.\n");
            OMPT_VERBOSE_INIT_PRINT(
                "Tool was started and is using the OMPT interface.\n");
            ompt_tool_module = h;
            break;
          }
          OMPT_VERBOSE_INIT_CONTINUED_PRINT(
              "Found but not using the OMPT interface.\n");
          OMPT_VERBOSE_INIT_PRINT("Continuing search...\n");
        }
        OMPT_DLCLOSE(h);
      }
      fname = __kmp_str_token(NULL, sep, &buf);
    }
    __kmp_str_free(&libs);
  } else {
    OMPT_VERBOSE_INIT_PRINT("No OMP_TOOL_LIBRARIES defined.\n");
  }

  // usable tool found in tool-libraries
  if (ret) {
    OMPT_VERBOSE_INIT_PRINT("----- END LOGGING OF TOOL REGISTRATION -----\n");
    return ret;
  }

#if KMP_OS_UNIX
  { // Non-standard: load archer tool if application is built with TSan
    const char *fname = "libarcher.so";
    OMPT_VERBOSE_INIT_PRINT(
        "...searching tool libraries failed. Using archer tool.\n");
    OMPT_VERBOSE_INIT_PRINT("Opening %s... ", fname);
    void *h = dlopen(fname, RTLD_LAZY);
    if (h) {
      OMPT_VERBOSE_INIT_CONTINUED_PRINT("Success.\n");
      OMPT_VERBOSE_INIT_PRINT("Searching for ompt_start_tool in %s... ", fname);
      start_tool = (ompt_start_tool_t)dlsym(h, "ompt_start_tool");
      if (start_tool) {
        ret = (*start_tool)(omp_version, runtime_version);
        if (ret) {
          OMPT_VERBOSE_INIT_CONTINUED_PRINT("Success.\n");
          OMPT_VERBOSE_INIT_PRINT(
              "Tool was started and is using the OMPT interface.\n");
          OMPT_VERBOSE_INIT_PRINT(
              "----- END LOGGING OF TOOL REGISTRATION -----\n");
          return ret;
        }
        OMPT_VERBOSE_INIT_CONTINUED_PRINT(
            "Found but not using the OMPT interface.\n");
      } else {
        OMPT_VERBOSE_INIT_CONTINUED_PRINT("Failed: %s\n", dlerror());
      }
    }
  }
#endif
  OMPT_VERBOSE_INIT_PRINT("No OMP tool loaded.\n");
  OMPT_VERBOSE_INIT_PRINT("----- END LOGGING OF TOOL REGISTRATION -----\n");
  return ret;
}

void ompt_pre_init() {
  //--------------------------------------------------
  // Execute the pre-initialization logic only once.
  //--------------------------------------------------
  static int ompt_pre_initialized = 0;

  if (ompt_pre_initialized)
    return;

  ompt_pre_initialized = 1;

  //--------------------------------------------------
  // Use a tool iff a tool is enabled and available.
  //--------------------------------------------------
  const char *ompt_env_var = getenv("OMP_TOOL");
  tool_setting_e tool_setting = omp_tool_error;

  if (!ompt_env_var || !strcmp(ompt_env_var, ""))
    tool_setting = omp_tool_unset;
  else if (OMPT_STR_MATCH(ompt_env_var, "disabled"))
    tool_setting = omp_tool_disabled;
  else if (OMPT_STR_MATCH(ompt_env_var, "enabled"))
    tool_setting = omp_tool_enabled;

  const char *ompt_env_verbose_init = getenv("OMP_TOOL_VERBOSE_INIT");
  // possible options: disabled | stdout | stderr | <filename>
  // if set, not empty and not disabled -> prepare for logging
  if (ompt_env_verbose_init && strcmp(ompt_env_verbose_init, "") &&
      !OMPT_STR_MATCH(ompt_env_verbose_init, "disabled")) {
    verbose_init = 1;
    if (OMPT_STR_MATCH(ompt_env_verbose_init, "STDERR"))
      verbose_file = stderr;
    else if (OMPT_STR_MATCH(ompt_env_verbose_init, "STDOUT"))
      verbose_file = stdout;
    else
      verbose_file = fopen(ompt_env_verbose_init, "w");
  } else
    verbose_init = 0;

#if OMPT_DEBUG
  printf("ompt_pre_init(): tool_setting = %d\n", tool_setting);
#endif
  switch (tool_setting) {
  case omp_tool_disabled:
    OMPT_VERBOSE_INIT_PRINT("OMP tool disabled. \n");
    break;

  case omp_tool_unset:
  case omp_tool_enabled:

    //--------------------------------------------------
    // Load tool iff specified in environment variable
    //--------------------------------------------------
    ompt_start_tool_result =
        ompt_try_start_tool(__kmp_openmp_version, ompt_get_runtime_version());

    memset(&ompt_enabled, 0, sizeof(ompt_enabled));
    break;

  case omp_tool_error:
    fprintf(stderr,
            "Warning: OMP_TOOL has invalid value \"%s\".\n"
            "  legal values are (NULL,\"\",\"disabled\","
            "\"enabled\").\n",
            ompt_env_var);
    break;
  }
  if (verbose_init && verbose_file != stderr && verbose_file != stdout)
    fclose(verbose_file);
#if OMPT_DEBUG
  printf("ompt_pre_init(): ompt_enabled = %d\n", ompt_enabled);
#endif
}

extern "C" int omp_get_initial_device(void);

void ompt_post_init() {
  //--------------------------------------------------
  // Execute the post-initialization logic only once.
  //--------------------------------------------------
  static int ompt_post_initialized = 0;

  if (ompt_post_initialized)
    return;

  ompt_post_initialized = 1;

  //--------------------------------------------------
  // Initialize the tool if so indicated.
  //--------------------------------------------------
  if (ompt_start_tool_result) {
    ompt_enabled.enabled = !!ompt_start_tool_result->initialize(
        ompt_fn_lookup, omp_get_initial_device(),
        &(ompt_start_tool_result->tool_data));

    if (!ompt_enabled.enabled) {
      // tool not enabled, zero out the bitmap, and done
      memset(&ompt_enabled, 0, sizeof(ompt_enabled));
      return;
    }

    kmp_info_t *root_thread = ompt_get_thread();

    ompt_set_thread_state(root_thread, ompt_state_overhead);

    if (ompt_enabled.ompt_callback_thread_begin) {
      ompt_callbacks.ompt_callback(ompt_callback_thread_begin)(
          ompt_thread_initial, __ompt_get_thread_data_internal());
    }
    ompt_data_t *task_data;
    ompt_data_t *parallel_data;
    __ompt_get_task_info_internal(0, NULL, &task_data, NULL, &parallel_data,
                                  NULL);
    if (ompt_enabled.ompt_callback_implicit_task) {
      ompt_callbacks.ompt_callback(ompt_callback_implicit_task)(
          ompt_scope_begin, parallel_data, task_data, 1, 1, ompt_task_initial);
    }

    ompt_set_thread_state(root_thread, ompt_state_work_serial);
  }
}

void ompt_fini() {
  if (ompt_enabled.enabled
#if OMPD_SUPPORT
      && ompt_start_tool_result && ompt_start_tool_result->finalize
#endif
  ) {
    ompt_start_tool_result->finalize(&(ompt_start_tool_result->tool_data));
  }

  if (ompt_tool_module)
    OMPT_DLCLOSE(ompt_tool_module);
  memset(&ompt_enabled, 0, sizeof(ompt_enabled));
}

/*****************************************************************************
 * interface operations
 ****************************************************************************/

/*****************************************************************************
 * state
 ****************************************************************************/

OMPT_API_ROUTINE int ompt_enumerate_states(int current_state, int *next_state,
                                           const char **next_state_name) {
  const static int len = sizeof(ompt_state_info) / sizeof(ompt_state_info_t);
  int i = 0;

  for (i = 0; i < len - 1; i++) {
    if (ompt_state_info[i].state_id == current_state) {
      *next_state = ompt_state_info[i + 1].state_id;
      *next_state_name = ompt_state_info[i + 1].state_name;
      return 1;
    }
  }

  return 0;
}

OMPT_API_ROUTINE int ompt_enumerate_mutex_impls(int current_impl,
                                                int *next_impl,
                                                const char **next_impl_name) {
  const static int len =
      sizeof(kmp_mutex_impl_info) / sizeof(kmp_mutex_impl_info_t);
  int i = 0;
  for (i = 0; i < len - 1; i++) {
    if (kmp_mutex_impl_info[i].id != current_impl)
      continue;
    *next_impl = kmp_mutex_impl_info[i + 1].id;
    *next_impl_name = kmp_mutex_impl_info[i + 1].name;
    return 1;
  }
  return 0;
}

/*****************************************************************************
 * callbacks
 ****************************************************************************/

OMPT_API_ROUTINE ompt_set_result_t ompt_set_callback(ompt_callbacks_t which,
                                                     ompt_callback_t callback) {
  switch (which) {

#define ompt_event_macro(event_name, callback_type, event_id)                  \
  case event_name:                                                             \
    ompt_callbacks.ompt_callback(event_name) = (callback_type)callback;        \
    ompt_enabled.event_name = (callback != 0);                                 \
    if (callback)                                                              \
      return ompt_event_implementation_status(event_name);                     \
    else                                                                       \
      return ompt_set_always;

    FOREACH_OMPT_EVENT(ompt_event_macro)

#undef ompt_event_macro

  default:
    return ompt_set_error;
  }
}

OMPT_API_ROUTINE int ompt_get_callback(ompt_callbacks_t which,
                                       ompt_callback_t *callback) {
  if (!ompt_enabled.enabled)
    return ompt_get_callback_failure;

  switch (which) {

#define ompt_event_macro(event_name, callback_type, event_id)                  \
  case event_name: {                                                           \
    ompt_callback_t mycb =                                                     \
        (ompt_callback_t)ompt_callbacks.ompt_callback(event_name);             \
    if (ompt_enabled.event_name && mycb) {                                     \
      *callback = mycb;                                                        \
      return ompt_get_callback_success;                                        \
    }                                                                          \
    return ompt_get_callback_failure;                                          \
  }

    FOREACH_OMPT_EVENT(ompt_event_macro)

#undef ompt_event_macro

  default:
    return ompt_get_callback_failure;
  }
}

/*****************************************************************************
 * parallel regions
 ****************************************************************************/

OMPT_API_ROUTINE int ompt_get_parallel_info(int ancestor_level,
                                            ompt_data_t **parallel_data,
                                            int *team_size) {
  if (!ompt_enabled.enabled)
    return 0;
  return __ompt_get_parallel_info_internal(ancestor_level, parallel_data,
                                           team_size);
}

OMPT_API_ROUTINE int ompt_get_state(ompt_wait_id_t *wait_id) {
  if (!ompt_enabled.enabled)
    return ompt_state_work_serial;
  int thread_state = __ompt_get_state_internal(wait_id);

  if (thread_state == ompt_state_undefined) {
    thread_state = ompt_state_work_serial;
  }

  return thread_state;
}

/*****************************************************************************
 * tasks
 ****************************************************************************/

OMPT_API_ROUTINE ompt_data_t *ompt_get_thread_data(void) {
  if (!ompt_enabled.enabled)
    return NULL;
  return __ompt_get_thread_data_internal();
}

OMPT_API_ROUTINE int ompt_get_task_info(int ancestor_level, int *type,
                                        ompt_data_t **task_data,
                                        ompt_frame_t **task_frame,
                                        ompt_data_t **parallel_data,
                                        int *thread_num) {
  if (!ompt_enabled.enabled)
    return 0;
  return __ompt_get_task_info_internal(ancestor_level, type, task_data,
                                       task_frame, parallel_data, thread_num);
}

OMPT_API_ROUTINE int ompt_get_task_memory(void **addr, size_t *size,
                                          int block) {
  return __ompt_get_task_memory_internal(addr, size, block);
}

/*****************************************************************************
 * num_procs
 ****************************************************************************/

OMPT_API_ROUTINE int ompt_get_num_procs(void) {
  // copied from kmp_ftn_entry.h (but modified: OMPT can only be called when
  // runtime is initialized)
  return __kmp_avail_proc;
}

/*****************************************************************************
 * places
 ****************************************************************************/

OMPT_API_ROUTINE int ompt_get_num_places(void) {
// copied from kmp_ftn_entry.h (but modified)
#if !KMP_AFFINITY_SUPPORTED
  return 0;
#else
  if (!KMP_AFFINITY_CAPABLE())
    return 0;
  return __kmp_affinity_num_masks;
#endif
}

OMPT_API_ROUTINE int ompt_get_place_proc_ids(int place_num, int ids_size,
                                             int *ids) {
// copied from kmp_ftn_entry.h (but modified)
#if !KMP_AFFINITY_SUPPORTED
  return 0;
#else
  int i, count;
  int tmp_ids[ids_size];
  for (int j = 0; j < ids_size; j++)
    tmp_ids[j] = 0;
  if (!KMP_AFFINITY_CAPABLE())
    return 0;
  if (place_num < 0 || place_num >= (int)__kmp_affinity_num_masks)
    return 0;
  /* TODO: Is this safe for asynchronous call from signal handler during runtime
   * shutdown? */
  kmp_affin_mask_t *mask = KMP_CPU_INDEX(__kmp_affinity_masks, place_num);
  count = 0;
  KMP_CPU_SET_ITERATE(i, mask) {
    if ((!KMP_CPU_ISSET(i, __kmp_affin_fullMask)) ||
        (!KMP_CPU_ISSET(i, mask))) {
      continue;
    }
    if (count < ids_size)
      tmp_ids[count] = i;
    count++;
  }
  if (ids_size >= count) {
    for (i = 0; i < count; i++) {
      ids[i] = tmp_ids[i];
    }
  }
  return count;
#endif
}

OMPT_API_ROUTINE int ompt_get_place_num(void) {
// copied from kmp_ftn_entry.h (but modified)
#if !KMP_AFFINITY_SUPPORTED
  return -1;
#else
  if (!ompt_enabled.enabled || __kmp_get_gtid() < 0)
    return -1;

  int gtid;
  kmp_info_t *thread;
  if (!KMP_AFFINITY_CAPABLE())
    return -1;
  gtid = __kmp_entry_gtid();
  thread = __kmp_thread_from_gtid(gtid);
  if (thread == NULL || thread->th.th_current_place < 0)
    return -1;
  return thread->th.th_current_place;
#endif
}

OMPT_API_ROUTINE int ompt_get_partition_place_nums(int place_nums_size,
                                                   int *place_nums) {
// copied from kmp_ftn_entry.h (but modified)
#if !KMP_AFFINITY_SUPPORTED
  return 0;
#else
  if (!ompt_enabled.enabled || __kmp_get_gtid() < 0)
    return 0;

  int i, gtid, place_num, first_place, last_place, start, end;
  kmp_info_t *thread;
  if (!KMP_AFFINITY_CAPABLE())
    return 0;
  gtid = __kmp_entry_gtid();
  thread = __kmp_thread_from_gtid(gtid);
  if (thread == NULL)
    return 0;
  first_place = thread->th.th_first_place;
  last_place = thread->th.th_last_place;
  if (first_place < 0 || last_place < 0)
    return 0;
  if (first_place <= last_place) {
    start = first_place;
    end = last_place;
  } else {
    start = last_place;
    end = first_place;
  }
  if (end - start <= place_nums_size)
    for (i = 0, place_num = start; place_num <= end; ++place_num, ++i) {
      place_nums[i] = place_num;
    }
  return end - start + 1;
#endif
}

/*****************************************************************************
 * places
 ****************************************************************************/

OMPT_API_ROUTINE int ompt_get_proc_id(void) {
  if (!ompt_enabled.enabled || __kmp_get_gtid() < 0)
    return -1;
#if KMP_HAVE_SCHED_GETCPU
  return sched_getcpu();
#elif KMP_OS_WINDOWS
  PROCESSOR_NUMBER pn;
  GetCurrentProcessorNumberEx(&pn);
  return 64 * pn.Group + pn.Number;
#else
  return -1;
#endif
}

/*****************************************************************************
 * compatability
 ****************************************************************************/

/*
 * Currently unused function
OMPT_API_ROUTINE int ompt_get_ompt_version() { return OMPT_VERSION; }
*/

/*****************************************************************************
 * application-facing API
 ****************************************************************************/

/*----------------------------------------------------------------------------
 | control
 ---------------------------------------------------------------------------*/

int __kmp_control_tool(uint64_t command, uint64_t modifier, void *arg) {

  if (ompt_enabled.enabled) {
    if (ompt_enabled.ompt_callback_control_tool) {
      return ompt_callbacks.ompt_callback(ompt_callback_control_tool)(
          command, modifier, arg, OMPT_LOAD_RETURN_ADDRESS(__kmp_entry_gtid()));
    } else {
      return -1;
    }
  } else {
    return -2;
  }
}

/*****************************************************************************
 * misc
 ****************************************************************************/

OMPT_API_ROUTINE uint64_t ompt_get_unique_id(void) {
  return __ompt_get_unique_id_internal();
}

OMPT_API_ROUTINE void ompt_finalize_tool(void) { __kmp_internal_end_atexit(); }

/*****************************************************************************
 * Target
 ****************************************************************************/

OMPT_API_ROUTINE int ompt_get_target_info(uint64_t *device_num,
                                          ompt_id_t *target_id,
                                          ompt_id_t *host_op_id) {
  return 0; // thread is not in a target region
}

OMPT_API_ROUTINE int ompt_get_num_devices(void) {
  return 1; // only one device (the current device) is available
}

/*****************************************************************************
 * API inquiry for tool
 ****************************************************************************/

static ompt_interface_fn_t ompt_fn_lookup(const char *s) {

#define ompt_interface_fn(fn)                                                  \
  fn##_t fn##_f = fn;                                                          \
  if (strcmp(s, #fn) == 0)                                                     \
    return (ompt_interface_fn_t)fn##_f;

  FOREACH_OMPT_INQUIRY_FN(ompt_interface_fn)

  return NULL;
}

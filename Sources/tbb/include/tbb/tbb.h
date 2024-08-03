#ifndef __WABI_TBB_tbb_H__
#define __WABI_TBB_tbb_H__

/**
    This header serves as a compatibility layer between
    the OneTBB we build, and consumers using the common
    tbb include paths, to include the headers from that
    build.
**/

#include <tbb/blocked_range.h>
#include <tbb/blocked_range2d.h>
#include <tbb/blocked_range3d.h>
#include <tbb/cache_aligned_allocator.h>
#include <tbb/combinable.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/collaborative_call_once.h>
#include <tbb/concurrent_priority_queue.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_map.h>
#include <tbb/concurrent_set.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/flow_graph.h>
#include <tbb/global_control.h>
#include <tbb/info.h>
#include <tbb/null_mutex.h>
#include <tbb/null_rw_mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_invoke.h>
#include <tbb/parallel_pipeline.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_scan.h>
#include <tbb/parallel_sort.h>
#include <tbb/partitioner.h>
#include <tbb/queuing_mutex.h>
#include <tbb/queuing_rw_mutex.h>
#include <tbb/spin_mutex.h>
#include <tbb/spin_rw_mutex.h>
#include <tbb/task.h>
#include <tbb/task_arena.h>
#include <tbb/task_group.h>
#include <tbb/task_scheduler_observer.h>
#include <tbb/tbb_allocator.h>
#include <tbb/tick_count.h>
#include <tbb/version.h>

#endif // __WABI_TBB_tbb_H__

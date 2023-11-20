/*
    Copyright (c) 2005-2023 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef __TBB_flow_graph_H
#define __TBB_flow_graph_H

#include <atomic>
#include <memory>
#include <type_traits>

#include "detail/_config.h"
#include "detail/_namespace_injection.h"
#include "spin_mutex.h"
#include "null_mutex.h"
#include "spin_rw_mutex.h"
#include "null_rw_mutex.h"
#include "detail/_pipeline_filters.h"
#include "detail/_task.h"
#include "detail/_small_object_pool.h"
#include "cache_aligned_allocator.h"
#include "detail/_exception.h"
#include "detail/_template_helpers.h"
#include "detail/_aggregator.h"
#include "detail/_allocator_traits.h"
#include "detail/_utils.h"
#include "profiling.h"
#include "task_arena.h"

#if TBB_USE_PROFILING_TOOLS && ( __unix__ || __APPLE__ )
   #if __INTEL_COMPILER
       // Disabled warning "routine is both inline and noinline"
       #pragma warning (push)
       #pragma warning( disable: 2196 )
   #endif
   #define __TBB_NOINLINE_SYM __attribute__((noinline))
#else
   #define __TBB_NOINLINE_SYM
#endif

#include <tuple>
#include <list>
#include <queue>
#if __TBB_CPP20_CONCEPTS_PRESENT
#include <concepts>
#endif

/** @file
  \brief The graph related classes and functions

  There are some applications that best express dependencies as messages
  passed between nodes in a graph.  These messages may contain data or
  simply act as signals that a predecessors has completed. The graph
  class and its associated node classes can be used to express such
  applications.
*/

namespace tbb {
namespace detail {

namespace d1 {

//! An enumeration the provides the two most common concurrency levels: unlimited and serial
enum concurrency { unlimited = 0, serial = 1 };

//! A generic null type
struct null_type {};

//! An empty class used for messages that mean "I'm done"
class continue_msg {};

} // namespace d1

#if __TBB_CPP20_CONCEPTS_PRESENT
namespace d0 {

template <typename ReturnType, typename OutputType>
concept node_body_return_type = std::same_as<OutputType, tbb::detail::d1::continue_msg> ||
                                std::convertible_to<OutputType, ReturnType>;

// TODO: consider using std::invocable here
template <typename Body, typename Output>
concept continue_node_body = std::copy_constructible<Body> &&
                             requires( Body& body, const tbb::detail::d1::continue_msg& v ) {
                                 { body(v) } -> node_body_return_type<Output>;
                             };

template <typename Body, typename Input, typename Output>
concept function_node_body = std::copy_constructible<Body> &&
                             std::invocable<Body&, const Input&> &&
                             node_body_return_type<std::invoke_result_t<Body&, const Input&>, Output>;

template <typename FunctionObject, typename Input, typename Key>
concept join_node_function_object = std::copy_constructible<FunctionObject> &&
                                    std::invocable<FunctionObject&, const Input&> &&
                                    std::convertible_to<std::invoke_result_t<FunctionObject&, const Input&>, Key>;

template <typename Body, typename Output>
concept input_node_body = std::copy_constructible<Body> &&
                          requires( Body& body, tbb::detail::d1::flow_control& fc ) {
                              { body(fc) } -> adaptive_same_as<Output>;
                          };

template <typename Body, typename Input, typename OutputPortsType>
concept multifunction_node_body = std::copy_constructible<Body> &&
                                  std::invocable<Body&, const Input&, OutputPortsType&>;

template <typename Sequencer, typename Value>
concept sequencer = std::copy_constructible<Sequencer> &&
                    std::invocable<Sequencer&, const Value&> &&
                    std::convertible_to<std::invoke_result_t<Sequencer&, const Value&>, std::size_t>;

template <typename Body, typename Input, typename GatewayType>
concept async_node_body = std::copy_constructible<Body> &&
                          std::invocable<Body&, const Input&, GatewayType&>;

} // namespace d0
#endif // __TBB_CPP20_CONCEPTS_PRESENT

namespace d1 {

//! Forward declaration section
template< typename T > class sender;
template< typename T > class receiver;
class continue_receiver;

template< typename T, typename U > class limiter_node;  // needed for resetting decrementer

template<typename T, typename M> class successor_cache;
template<typename T, typename M> class broadcast_cache;
template<typename T, typename M> class round_robin_cache;
template<typename T, typename M> class predecessor_cache;
template<typename T, typename M> class reservable_predecessor_cache;

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
namespace order {
struct following;
struct preceding;
}
template<typename Order, typename... Args> struct node_set;
#endif


} // namespace d1
} // namespace detail
} // namespace tbb

#include "detail/_task.h"
#include "task_group.h"
#include "task_arena.h"
#include "flow_graph_abstractions.h"

#include "concurrent_priority_queue.h"

#include <list>

//! The graph class
namespace tbb {
namespace detail {

namespace d1 {

class graph_task;
static graph_task* const SUCCESSFULLY_ENQUEUED = (graph_task*)-1;
typedef unsigned int node_priority_t;
static const node_priority_t no_priority = node_priority_t(0);

class graph;
class graph_node;

template <typename GraphContainerType, typename GraphNodeType>
class graph_iterator {
    friend class graph;
    friend class graph_node;
public:
    typedef size_t size_type;
    typedef GraphNodeType value_type;
    typedef GraphNodeType* pointer;
    typedef GraphNodeType& reference;
    typedef const GraphNodeType& const_reference;
    typedef std::forward_iterator_tag iterator_category;

    //! Copy constructor
    graph_iterator(const graph_iterator& other) :
        my_graph(other.my_graph), current_node(other.current_node)
    {}

    //! Assignment
    graph_iterator& operator=(const graph_iterator& other) {
        if (this != &other) {
            my_graph = other.my_graph;
            current_node = other.current_node;
        }
        return *this;
    }

    //! Dereference
    reference operator*() const;

    //! Dereference
    pointer operator->() const;

    //! Equality
    bool operator==(const graph_iterator& other) const {
        return ((my_graph == other.my_graph) && (current_node == other.current_node));
    }

#if !__TBB_CPP20_COMPARISONS_PRESENT
    //! Inequality
    bool operator!=(const graph_iterator& other) const { return !(operator==(other)); }
#endif

    //! Pre-increment
    graph_iterator& operator++() {
        internal_forward();
        return *this;
    }

    //! Post-increment
    graph_iterator operator++(int) {
        graph_iterator result = *this;
        operator++();
        return result;
    }

private:
    // the graph over which we are iterating
    GraphContainerType *my_graph;
    // pointer into my_graph's my_nodes list
    pointer current_node;

    //! Private initializing constructor for begin() and end() iterators
    graph_iterator(GraphContainerType *g, bool begin);
    void internal_forward();
};  // class graph_iterator

// flags to modify the behavior of the graph reset().  Can be combined.
enum reset_flags {
    rf_reset_protocol = 0,
    rf_reset_bodies = 1 << 0,  // delete the current node body, reset to a copy of the initial node body.
    rf_clear_edges = 1 << 1   // delete edges
};

void activate_graph(graph& g);
void deactivate_graph(graph& g);
bool is_graph_active(graph& g);
graph_task* prioritize_task(graph& g, graph_task& arena_task);
void spawn_in_graph_arena(graph& g, graph_task& arena_task);
void enqueue_in_graph_arena(graph &g, graph_task& arena_task);

class graph;

//! Base class for tasks generated by graph nodes.
class graph_task : public task {
public:
    graph_task(graph& g, small_object_allocator& allocator
               , node_priority_t node_priority = no_priority
    )
        : my_graph(g)
        , priority(node_priority)
        , my_allocator(allocator)
    {}
    graph& my_graph; // graph instance the task belongs to
    // TODO revamp: rename to my_priority
    node_priority_t priority;
    template <typename DerivedType>
    void destruct_and_deallocate(const execution_data& ed);
protected:
    template <typename DerivedType>
    void finalize(const execution_data& ed);
private:
    // To organize task_list
    graph_task* my_next{ nullptr };
    small_object_allocator my_allocator;
    // TODO revamp: elaborate internal interfaces to avoid friends declarations
    friend class graph_task_list;
    friend graph_task* prioritize_task(graph& g, graph_task& gt);
};

struct graph_task_comparator {
    bool operator()(const graph_task* left, const graph_task* right) {
        return left->priority < right->priority;
    }
};

typedef tbb::concurrent_priority_queue<graph_task*, graph_task_comparator> graph_task_priority_queue_t;

class priority_task_selector : public task {
public:
    priority_task_selector(graph_task_priority_queue_t& priority_queue, small_object_allocator& allocator)
        : my_priority_queue(priority_queue), my_allocator(allocator), my_task() {}
    task* execute(execution_data& ed) override {
        next_task();
        __TBB_ASSERT(my_task, nullptr);
        task* t_next = my_task->execute(ed);
        my_allocator.delete_object(this, ed);
        return t_next;
    }
    task* cancel(execution_data& ed) override {
        if (!my_task) {
            next_task();
        }
        __TBB_ASSERT(my_task, nullptr);
        task* t_next = my_task->cancel(ed);
        my_allocator.delete_object(this, ed);
        return t_next;
    }
private:
    void next_task() {
        // TODO revamp: hold functors in priority queue instead of real tasks
        bool result = my_priority_queue.try_pop(my_task);
        __TBB_ASSERT_EX(result, "Number of critical tasks for scheduler and tasks"
            " in graph's priority queue mismatched");
        __TBB_ASSERT(my_task && my_task != SUCCESSFULLY_ENQUEUED,
            "Incorrect task submitted to graph priority queue");
        __TBB_ASSERT(my_task->priority != no_priority,
            "Tasks from graph's priority queue must have priority");
    }

    graph_task_priority_queue_t& my_priority_queue;
    small_object_allocator my_allocator;
    graph_task* my_task;
};

template <typename Receiver, typename Body> class run_and_put_task;
template <typename Body> class run_task;

//********************************************************************************
// graph tasks helpers
//********************************************************************************

//! The list of graph tasks
class graph_task_list : no_copy {
private:
    graph_task* my_first;
    graph_task** my_next_ptr;
public:
    //! Construct empty list
    graph_task_list() : my_first(nullptr), my_next_ptr(&my_first) {}

    //! True if list is empty; false otherwise.
    bool empty() const { return !my_first; }

    //! Push task onto back of list.
    void push_back(graph_task& task) {
        task.my_next = nullptr;
        *my_next_ptr = &task;
        my_next_ptr = &task.my_next;
    }

    //! Pop the front task from the list.
    graph_task& pop_front() {
        __TBB_ASSERT(!empty(), "attempt to pop item from empty task_list");
        graph_task* result = my_first;
        my_first = result->my_next;
        if (!my_first) {
            my_next_ptr = &my_first;
        }
        return *result;
    }
};

//! The graph class
/** This class serves as a handle to the graph */
class graph : no_copy, public graph_proxy {
    friend class graph_node;

    void prepare_task_arena(bool reinit = false) {
        if (reinit) {
            __TBB_ASSERT(my_task_arena, "task arena is nullptr");
            my_task_arena->terminate();
            my_task_arena->initialize(task_arena::attach());
        }
        else {
            __TBB_ASSERT(my_task_arena == nullptr, "task arena is not nullptr");
            my_task_arena = new task_arena(task_arena::attach());
        }
        if (!my_task_arena->is_active()) // failed to attach
            my_task_arena->initialize(); // create a new, default-initialized arena
        __TBB_ASSERT(my_task_arena->is_active(), "task arena is not active");
    }

public:
    //! Constructs a graph with isolated task_group_context
    graph();

    //! Constructs a graph with use_this_context as context
    explicit graph(task_group_context& use_this_context);

    //! Destroys the graph.
    /** Calls wait_for_all, then destroys the root task and context. */
    ~graph();

    //! Used to register that an external entity may still interact with the graph.
    /** The graph will not return from wait_for_all until a matching number of release_wait calls is
    made. */
    void reserve_wait() override;

    //! Deregisters an external entity that may have interacted with the graph.
    /** The graph will not return from wait_for_all until all the number of reserve_wait calls
    matches the number of release_wait calls. */
    void release_wait() override;

    //! Wait until graph is idle and the number of release_wait calls equals to the number of
    //! reserve_wait calls.
    /** The waiting thread will go off and steal work while it is blocked in the wait_for_all. */
    void wait_for_all() {
        cancelled = false;
        caught_exception = false;
        try_call([this] {
            my_task_arena->execute([this] {
                wait(my_wait_context, *my_context);
            });
            cancelled = my_context->is_group_execution_cancelled();
        }).on_exception([this] {
            my_context->reset();
            caught_exception = true;
            cancelled = true;
        });
        // TODO: the "if" condition below is just a work-around to support the concurrent wait
        // mode. The cancellation and exception mechanisms are still broken in this mode.
        // Consider using task group not to re-implement the same functionality.
        if (!(my_context->traits() & task_group_context::concurrent_wait)) {
            my_context->reset();  // consistent with behavior in catch()
        }
    }

    // TODO revamp: consider adding getter for task_group_context.

    // ITERATORS
    template<typename C, typename N>
    friend class graph_iterator;

    // Graph iterator typedefs
    typedef graph_iterator<graph, graph_node> iterator;
    typedef graph_iterator<const graph, const graph_node> const_iterator;

    // Graph iterator constructors
    //! start iterator
    iterator begin();
    //! end iterator
    iterator end();
    //! start const iterator
    const_iterator begin() const;
    //! end const iterator
    const_iterator end() const;
    //! start const iterator
    const_iterator cbegin() const;
    //! end const iterator
    const_iterator cend() const;

    // thread-unsafe state reset.
    void reset(reset_flags f = rf_reset_protocol);

    //! cancels execution of the associated task_group_context
    void cancel();

    //! return status of graph execution
    bool is_cancelled() { return cancelled; }
    bool exception_thrown() { return caught_exception; }

private:
    wait_context my_wait_context;
    task_group_context *my_context;
    bool own_context;
    bool cancelled;
    bool caught_exception;
    bool my_is_active;

    graph_node *my_nodes, *my_nodes_last;

    tbb::spin_mutex nodelist_mutex;
    void register_node(graph_node *n);
    void remove_node(graph_node *n);

    task_arena* my_task_arena;

    graph_task_priority_queue_t my_priority_queue;

    friend void activate_graph(graph& g);
    friend void deactivate_graph(graph& g);
    friend bool is_graph_active(graph& g);
    friend graph_task* prioritize_task(graph& g, graph_task& arena_task);
    friend void spawn_in_graph_arena(graph& g, graph_task& arena_task);
    friend void enqueue_in_graph_arena(graph &g, graph_task& arena_task);

    friend class task_arena_base;

};  // class graph

template<typename DerivedType>
inline void graph_task::destruct_and_deallocate(const execution_data& ed) {
    auto allocator = my_allocator;
    // TODO: investigate if direct call of derived destructor gives any benefits.
    this->~graph_task();
    allocator.deallocate(static_cast<DerivedType*>(this), ed);
}

template<typename DerivedType>
inline void graph_task::finalize(const execution_data& ed) {
    graph& g = my_graph;
    destruct_and_deallocate<DerivedType>(ed);
    g.release_wait();
}

//********************************************************************************
// end of graph tasks helpers
//********************************************************************************


#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
class get_graph_helper;
#endif

//! The base of all graph nodes.
class graph_node : no_copy {
    friend class graph;
    template<typename C, typename N>
    friend class graph_iterator;

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    friend class get_graph_helper;
#endif

protected:
    graph& my_graph;
    graph& graph_reference() const {
        // TODO revamp: propagate graph_reference() method to all the reference places.
        return my_graph;
    }
    graph_node* next = nullptr;
    graph_node* prev = nullptr;
public:
    explicit graph_node(graph& g);

    virtual ~graph_node();

protected:
    // performs the reset on an individual node.
    virtual void reset_node(reset_flags f = rf_reset_protocol) = 0;
};  // class graph_node

inline void activate_graph(graph& g) {
    g.my_is_active = true;
}

inline void deactivate_graph(graph& g) {
    g.my_is_active = false;
}

inline bool is_graph_active(graph& g) {
    return g.my_is_active;
}

inline graph_task* prioritize_task(graph& g, graph_task& gt) {
    if( no_priority == gt.priority )
        return &gt;

    //! Non-preemptive priority pattern. The original task is submitted as a work item to the
    //! priority queue, and a new critical task is created to take and execute a work item with
    //! the highest known priority. The reference counting responsibility is transferred (via
    //! allocate_continuation) to the new task.
    task* critical_task = gt.my_allocator.new_object<priority_task_selector>(g.my_priority_queue, gt.my_allocator);
    __TBB_ASSERT( critical_task, "bad_alloc?" );
    g.my_priority_queue.push(&gt);
    using tbb::detail::d1::submit;
    submit( *critical_task, *g.my_task_arena, *g.my_context, /*as_critical=*/true );
    return nullptr;
}

//! Spawns a task inside graph arena
inline void spawn_in_graph_arena(graph& g, graph_task& arena_task) {
    if (is_graph_active(g)) {
        task* gt = prioritize_task(g, arena_task);
        if( !gt )
            return;

        __TBB_ASSERT(g.my_task_arena && g.my_task_arena->is_active(), nullptr);
        submit( *gt, *g.my_task_arena, *g.my_context
#if __TBB_PREVIEW_CRITICAL_TASKS
                , /*as_critical=*/false
#endif
        );
    }
}

// TODO revamp: unify *_in_graph_arena functions

//! Enqueues a task inside graph arena
inline void enqueue_in_graph_arena(graph &g, graph_task& arena_task) {
    if (is_graph_active(g)) {
        __TBB_ASSERT( g.my_task_arena && g.my_task_arena->is_active(), "Is graph's arena initialized and active?" );

        // TODO revamp: decide on the approach that does not postpone critical task
        if( task* gt = prioritize_task(g, arena_task) )
            submit( *gt, *g.my_task_arena, *g.my_context, /*as_critical=*/false);
    }
}

} // namespace d1
} // namespace detail
} // namespace tbb

namespace tbb {
namespace detail {
namespace d1 {

static inline std::pair<graph_task*, graph_task*> order_tasks(graph_task* first, graph_task* second) {
    if (second->priority > first->priority)
        return std::make_pair(second, first);
    return std::make_pair(first, second);
}

// submit task if necessary. Returns the non-enqueued task if there is one.
static inline graph_task* combine_tasks(graph& g, graph_task* left, graph_task* right) {
    // if no RHS task, don't change left.
    if (right == nullptr) return left;
    // right != nullptr
    if (left == nullptr) return right;
    if (left == SUCCESSFULLY_ENQUEUED) return right;
    // left contains a task
    if (right != SUCCESSFULLY_ENQUEUED) {
        // both are valid tasks
        auto tasks_pair = order_tasks(left, right);
        spawn_in_graph_arena(g, *tasks_pair.first);
        return tasks_pair.second;
    }
    return left;
}

//! Pure virtual template class that defines a sender of messages of type T
template< typename T >
class sender {
public:
    virtual ~sender() {}

    //! Request an item from the sender
    virtual bool try_get( T & ) { return false; }

    //! Reserves an item in the sender
    virtual bool try_reserve( T & ) { return false; }

    //! Releases the reserved item
    virtual bool try_release( ) { return false; }

    //! Consumes the reserved item
    virtual bool try_consume( ) { return false; }

protected:
    //! The output type of this sender
    typedef T output_type;

    //! The successor type for this node
    typedef receiver<T> successor_type;

    //! Add a new successor to this node
    virtual bool register_successor( successor_type &r ) = 0;

    //! Removes a successor from this node
    virtual bool remove_successor( successor_type &r ) = 0;

    template<typename C>
    friend bool register_successor(sender<C>& s, receiver<C>& r);

    template<typename C>
    friend bool remove_successor  (sender<C>& s, receiver<C>& r);
};  // class sender<T>

template<typename C>
bool register_successor(sender<C>& s, receiver<C>& r) {
    return s.register_successor(r);
}

template<typename C>
bool remove_successor(sender<C>& s, receiver<C>& r) {
    return s.remove_successor(r);
}

//! Pure virtual template class that defines a receiver of messages of type T
template< typename T >
class receiver {
public:
    //! Destructor
    virtual ~receiver() {}

    //! Put an item to the receiver
    bool try_put( const T& t ) {
        graph_task *res = try_put_task(t);
        if (!res) return false;
        if (res != SUCCESSFULLY_ENQUEUED) spawn_in_graph_arena(graph_reference(), *res);
        return true;
    }

    //! put item to successor; return task to run the successor if possible.
protected:
    //! The input type of this receiver
    typedef T input_type;

    //! The predecessor type for this node
    typedef sender<T> predecessor_type;

    template< typename R, typename B > friend class run_and_put_task;
    template< typename X, typename Y > friend class broadcast_cache;
    template< typename X, typename Y > friend class round_robin_cache;
    virtual graph_task *try_put_task(const T& t) = 0;
    virtual graph& graph_reference() const = 0;

    template<typename TT, typename M> friend class successor_cache;
    virtual bool is_continue_receiver() { return false; }

    // TODO revamp: reconsider the inheritance and move node priority out of receiver
    virtual node_priority_t priority() const { return no_priority; }

    //! Add a predecessor to the node
    virtual bool register_predecessor( predecessor_type & ) { return false; }

    //! Remove a predecessor from the node
    virtual bool remove_predecessor( predecessor_type & ) { return false; }

    template <typename C>
    friend bool register_predecessor(receiver<C>& r, sender<C>& s);
    template <typename C>
    friend bool remove_predecessor  (receiver<C>& r, sender<C>& s);
}; // class receiver<T>

template <typename C>
bool register_predecessor(receiver<C>& r, sender<C>& s) {
    return r.register_predecessor(s);
}

template <typename C>
bool remove_predecessor(receiver<C>& r, sender<C>& s) {
    return r.remove_predecessor(s);
}

//! Base class for receivers of completion messages
/** These receivers automatically reset, but cannot be explicitly waited on */
class continue_receiver : public receiver< continue_msg > {
protected:

    //! Constructor
    explicit continue_receiver( int number_of_predecessors, node_priority_t a_priority ) {
        my_predecessor_count = my_initial_predecessor_count = number_of_predecessors;
        my_current_count = 0;
        my_priority = a_priority;
    }

    //! Copy constructor
    continue_receiver( const continue_receiver& src ) : receiver<continue_msg>() {
        my_predecessor_count = my_initial_predecessor_count = src.my_initial_predecessor_count;
        my_current_count = 0;
        my_priority = src.my_priority;
    }

    //! Increments the trigger threshold
    bool register_predecessor( predecessor_type & ) override {
        spin_mutex::scoped_lock l(my_mutex);
        ++my_predecessor_count;
        return true;
    }

    //! Decrements the trigger threshold
    /** Does not check to see if the removal of the predecessor now makes the current count
        exceed the new threshold.  So removing a predecessor while the graph is active can cause
        unexpected results. */
    bool remove_predecessor( predecessor_type & ) override {
        spin_mutex::scoped_lock l(my_mutex);
        --my_predecessor_count;
        return true;
    }

    //! The input type
    typedef continue_msg input_type;

    //! The predecessor type for this node
    typedef receiver<input_type>::predecessor_type predecessor_type;

    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    // execute body is supposed to be too small to create a task for.
    graph_task* try_put_task( const input_type & ) override {
        {
            spin_mutex::scoped_lock l(my_mutex);
            if ( ++my_current_count < my_predecessor_count )
                return SUCCESSFULLY_ENQUEUED;
            else
                my_current_count = 0;
        }
        graph_task* res = execute();
        return res? res : SUCCESSFULLY_ENQUEUED;
    }

    spin_mutex my_mutex;
    int my_predecessor_count;
    int my_current_count;
    int my_initial_predecessor_count;
    node_priority_t my_priority;
    // the friend declaration in the base class did not eliminate the "protected class"
    // error in gcc 4.1.2
    template<typename U, typename V> friend class limiter_node;

    virtual void reset_receiver( reset_flags f ) {
        my_current_count = 0;
        if (f & rf_clear_edges) {
            my_predecessor_count = my_initial_predecessor_count;
        }
    }

    //! Does whatever should happen when the threshold is reached
    /** This should be very fast or else spawn a task.  This is
        called while the sender is blocked in the try_put(). */
    virtual graph_task* execute() = 0;
    template<typename TT, typename M> friend class successor_cache;
    bool is_continue_receiver() override { return true; }

    node_priority_t priority() const override { return my_priority; }
}; // class continue_receiver

#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
    template <typename K, typename T>
    K key_from_message( const T &t ) {
        return t.key();
    }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */

} // d1
} // detail
} // tbb

#include "profiling.h"
#if (_MSC_VER >= 1900)
    #include <intrin.h>
#endif

namespace tbb {
namespace detail {
namespace d1 {

template< typename T > class sender;
template< typename T > class receiver;

#if TBB_USE_PROFILING_TOOLS
    #if __TBB_FLOW_TRACE_CODEPTR
        #if (_MSC_VER >= 1900)
            #define CODEPTR() (_ReturnAddress())
        #elif __TBB_GCC_VERSION >= 40800
            #define CODEPTR() ( __builtin_return_address(0))
        #else
            #define CODEPTR() nullptr
        #endif
    #else
        #define CODEPTR() nullptr
    #endif /* __TBB_FLOW_TRACE_CODEPTR */

static inline void fgt_alias_port(void *node, void *p, bool visible) {
    if(visible)
        itt_relation_add( ITT_DOMAIN_FLOW, node, FLOW_NODE, __itt_relation_is_parent_of, p, FLOW_NODE );
    else
        itt_relation_add( ITT_DOMAIN_FLOW, p, FLOW_NODE, __itt_relation_is_child_of, node, FLOW_NODE );
}

static inline void fgt_composite ( void* codeptr, void *node, void *graph ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, node, FLOW_NODE, graph, FLOW_GRAPH, FLOW_COMPOSITE_NODE );
    suppress_unused_warning( codeptr );
#if __TBB_FLOW_TRACE_CODEPTR
    if (codeptr != nullptr) {
        register_node_addr(ITT_DOMAIN_FLOW, node, FLOW_NODE, CODE_ADDRESS, &codeptr);
    }
#endif
}

static inline void fgt_internal_alias_input_port( void *node, void *p, string_resource_index name_index ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, p, FLOW_INPUT_PORT, node, FLOW_NODE, name_index );
    itt_relation_add( ITT_DOMAIN_FLOW, node, FLOW_NODE, __itt_relation_is_parent_of, p, FLOW_INPUT_PORT );
}

static inline void fgt_internal_alias_output_port( void *node, void *p, string_resource_index name_index ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, p, FLOW_OUTPUT_PORT, node, FLOW_NODE, name_index );
    itt_relation_add( ITT_DOMAIN_FLOW, node, FLOW_NODE, __itt_relation_is_parent_of, p, FLOW_OUTPUT_PORT );
}

template<typename InputType>
void alias_input_port(void *node, receiver<InputType>* port, string_resource_index name_index) {
    // TODO: Make fgt_internal_alias_input_port a function template?
    fgt_internal_alias_input_port( node, port, name_index);
}

template < typename PortsTuple, int N >
struct fgt_internal_input_alias_helper {
    static void alias_port( void *node, PortsTuple &ports ) {
        alias_input_port( node, &(std::get<N-1>(ports)), static_cast<string_resource_index>(FLOW_INPUT_PORT_0 + N - 1) );
        fgt_internal_input_alias_helper<PortsTuple, N-1>::alias_port( node, ports );
    }
};

template < typename PortsTuple >
struct fgt_internal_input_alias_helper<PortsTuple, 0> {
    static void alias_port( void * /* node */, PortsTuple & /* ports */ ) { }
};

template<typename OutputType>
void alias_output_port(void *node, sender<OutputType>* port, string_resource_index name_index) {
    // TODO: Make fgt_internal_alias_output_port a function template?
    fgt_internal_alias_output_port( node, static_cast<void *>(port), name_index);
}

template < typename PortsTuple, int N >
struct fgt_internal_output_alias_helper {
    static void alias_port( void *node, PortsTuple &ports ) {
        alias_output_port( node, &(std::get<N-1>(ports)), static_cast<string_resource_index>(FLOW_OUTPUT_PORT_0 + N - 1) );
        fgt_internal_output_alias_helper<PortsTuple, N-1>::alias_port( node, ports );
    }
};

template < typename PortsTuple >
struct fgt_internal_output_alias_helper<PortsTuple, 0> {
    static void alias_port( void * /*node*/, PortsTuple &/*ports*/ ) {
    }
};

static inline void fgt_internal_create_input_port( void *node, void *p, string_resource_index name_index ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, p, FLOW_INPUT_PORT, node, FLOW_NODE, name_index );
}

static inline void fgt_internal_create_output_port( void* codeptr, void *node, void *p, string_resource_index name_index ) {
    itt_make_task_group(ITT_DOMAIN_FLOW, p, FLOW_OUTPUT_PORT, node, FLOW_NODE, name_index);
    suppress_unused_warning( codeptr );
#if __TBB_FLOW_TRACE_CODEPTR
    if (codeptr != nullptr) {
        register_node_addr(ITT_DOMAIN_FLOW, node, FLOW_NODE, CODE_ADDRESS, &codeptr);
    }
#endif
}

template<typename InputType>
void register_input_port(void *node, receiver<InputType>* port, string_resource_index name_index) {
    // TODO: Make fgt_internal_create_input_port a function template?
    fgt_internal_create_input_port(node, static_cast<void*>(port), name_index);
}

template < typename PortsTuple, int N >
struct fgt_internal_input_helper {
    static void register_port( void *node, PortsTuple &ports ) {
        register_input_port( node, &(std::get<N-1>(ports)), static_cast<string_resource_index>(FLOW_INPUT_PORT_0 + N - 1) );
        fgt_internal_input_helper<PortsTuple, N-1>::register_port( node, ports );
    }
};

template < typename PortsTuple >
struct fgt_internal_input_helper<PortsTuple, 1> {
    static void register_port( void *node, PortsTuple &ports ) {
        register_input_port( node, &(std::get<0>(ports)), FLOW_INPUT_PORT_0 );
    }
};

template<typename OutputType>
void register_output_port(void* codeptr, void *node, sender<OutputType>* port, string_resource_index name_index) {
    // TODO: Make fgt_internal_create_output_port a function template?
    fgt_internal_create_output_port( codeptr, node, static_cast<void *>(port), name_index);
}

template < typename PortsTuple, int N >
struct fgt_internal_output_helper {
    static void register_port( void* codeptr, void *node, PortsTuple &ports ) {
        register_output_port( codeptr, node, &(std::get<N-1>(ports)), static_cast<string_resource_index>(FLOW_OUTPUT_PORT_0 + N - 1) );
        fgt_internal_output_helper<PortsTuple, N-1>::register_port( codeptr, node, ports );
    }
};

template < typename PortsTuple >
struct fgt_internal_output_helper<PortsTuple,1> {
    static void register_port( void* codeptr, void *node, PortsTuple &ports ) {
        register_output_port( codeptr, node, &(std::get<0>(ports)), FLOW_OUTPUT_PORT_0 );
    }
};

template< typename NodeType >
void fgt_multioutput_node_desc( const NodeType *node, const char *desc ) {
    void *addr =  (void *)( static_cast< receiver< typename NodeType::input_type > * >(const_cast< NodeType *>(node)) );
    itt_metadata_str_add( ITT_DOMAIN_FLOW, addr, FLOW_NODE, FLOW_OBJECT_NAME, desc );
}

template< typename NodeType >
void fgt_multiinput_multioutput_node_desc( const NodeType *node, const char *desc ) {
    void *addr =  const_cast<NodeType *>(node);
    itt_metadata_str_add( ITT_DOMAIN_FLOW, addr, FLOW_NODE, FLOW_OBJECT_NAME, desc );
}

template< typename NodeType >
static inline void fgt_node_desc( const NodeType *node, const char *desc ) {
    void *addr =  (void *)( static_cast< sender< typename NodeType::output_type > * >(const_cast< NodeType *>(node)) );
    itt_metadata_str_add( ITT_DOMAIN_FLOW, addr, FLOW_NODE, FLOW_OBJECT_NAME, desc );
}

static inline void fgt_graph_desc( const void *g, const char *desc ) {
    void *addr = const_cast< void *>(g);
    itt_metadata_str_add( ITT_DOMAIN_FLOW, addr, FLOW_GRAPH, FLOW_OBJECT_NAME, desc );
}

static inline void fgt_body( void *node, void *body ) {
    itt_relation_add( ITT_DOMAIN_FLOW, body, FLOW_BODY, __itt_relation_is_child_of, node, FLOW_NODE );
}

template< int N, typename PortsTuple >
static inline void fgt_multioutput_node(void* codeptr, string_resource_index t, void *g, void *input_port, PortsTuple &ports ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, input_port, FLOW_NODE, g, FLOW_GRAPH, t );
    fgt_internal_create_input_port( input_port, input_port, FLOW_INPUT_PORT_0 );
    fgt_internal_output_helper<PortsTuple, N>::register_port(codeptr, input_port, ports );
}

template< int N, typename PortsTuple >
static inline void fgt_multioutput_node_with_body( void* codeptr, string_resource_index t, void *g, void *input_port, PortsTuple &ports, void *body ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, input_port, FLOW_NODE, g, FLOW_GRAPH, t );
    fgt_internal_create_input_port( input_port, input_port, FLOW_INPUT_PORT_0 );
    fgt_internal_output_helper<PortsTuple, N>::register_port( codeptr, input_port, ports );
    fgt_body( input_port, body );
}

template< int N, typename PortsTuple >
static inline void fgt_multiinput_node( void* codeptr, string_resource_index t, void *g, PortsTuple &ports, void *output_port) {
    itt_make_task_group( ITT_DOMAIN_FLOW, output_port, FLOW_NODE, g, FLOW_GRAPH, t );
    fgt_internal_create_output_port( codeptr, output_port, output_port, FLOW_OUTPUT_PORT_0 );
    fgt_internal_input_helper<PortsTuple, N>::register_port( output_port, ports );
}

static inline void fgt_multiinput_multioutput_node( void* codeptr, string_resource_index t, void *n, void *g ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, n, FLOW_NODE, g, FLOW_GRAPH, t );
    suppress_unused_warning( codeptr );
#if __TBB_FLOW_TRACE_CODEPTR
    if (codeptr != nullptr) {
        register_node_addr(ITT_DOMAIN_FLOW, n, FLOW_NODE, CODE_ADDRESS, &codeptr);
    }
#endif
}

static inline void fgt_node( void* codeptr, string_resource_index t, void *g, void *output_port ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, output_port, FLOW_NODE, g, FLOW_GRAPH, t );
    fgt_internal_create_output_port( codeptr, output_port, output_port, FLOW_OUTPUT_PORT_0 );
}

static void fgt_node_with_body( void* codeptr, string_resource_index t, void *g, void *output_port, void *body ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, output_port, FLOW_NODE, g, FLOW_GRAPH, t );
    fgt_internal_create_output_port(codeptr, output_port, output_port, FLOW_OUTPUT_PORT_0 );
    fgt_body( output_port, body );
}

static inline void fgt_node( void* codeptr, string_resource_index t, void *g, void *input_port, void *output_port ) {
    fgt_node( codeptr, t, g, output_port );
    fgt_internal_create_input_port( output_port, input_port, FLOW_INPUT_PORT_0 );
}

static inline void  fgt_node_with_body( void* codeptr, string_resource_index t, void *g, void *input_port, void *output_port, void *body ) {
    fgt_node_with_body( codeptr, t, g, output_port, body );
    fgt_internal_create_input_port( output_port, input_port, FLOW_INPUT_PORT_0 );
}


static inline void  fgt_node( void* codeptr, string_resource_index t, void *g, void *input_port, void *decrement_port, void *output_port ) {
    fgt_node( codeptr, t, g, input_port, output_port );
    fgt_internal_create_input_port( output_port, decrement_port, FLOW_INPUT_PORT_1 );
}

static inline void fgt_make_edge( void *output_port, void *input_port ) {
    itt_relation_add( ITT_DOMAIN_FLOW, output_port, FLOW_OUTPUT_PORT, __itt_relation_is_predecessor_to, input_port, FLOW_INPUT_PORT);
}

static inline void fgt_remove_edge( void *output_port, void *input_port ) {
    itt_relation_add( ITT_DOMAIN_FLOW, output_port, FLOW_OUTPUT_PORT, __itt_relation_is_sibling_of, input_port, FLOW_INPUT_PORT);
}

static inline void fgt_graph( void *g ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, g, FLOW_GRAPH, nullptr, FLOW_NULL, FLOW_GRAPH );
}

static inline void fgt_begin_body( void *body ) {
    itt_task_begin( ITT_DOMAIN_FLOW, body, FLOW_BODY, nullptr, FLOW_NULL, FLOW_BODY );
}

static inline void fgt_end_body( void * ) {
    itt_task_end( ITT_DOMAIN_FLOW );
}

static inline void fgt_async_try_put_begin( void *node, void *port ) {
    itt_task_begin( ITT_DOMAIN_FLOW, port, FLOW_OUTPUT_PORT, node, FLOW_NODE, FLOW_OUTPUT_PORT );
}

static inline void fgt_async_try_put_end( void *, void * ) {
    itt_task_end( ITT_DOMAIN_FLOW );
}

static inline void fgt_async_reserve( void *node, void *graph ) {
    itt_region_begin( ITT_DOMAIN_FLOW, node, FLOW_NODE, graph, FLOW_GRAPH, FLOW_NULL );
}

static inline void fgt_async_commit( void *node, void * /*graph*/) {
    itt_region_end( ITT_DOMAIN_FLOW, node, FLOW_NODE );
}

static inline void fgt_reserve_wait( void *graph ) {
    itt_region_begin( ITT_DOMAIN_FLOW, graph, FLOW_GRAPH, nullptr, FLOW_NULL, FLOW_NULL );
}

static inline void fgt_release_wait( void *graph ) {
    itt_region_end( ITT_DOMAIN_FLOW, graph, FLOW_GRAPH );
}

#else // TBB_USE_PROFILING_TOOLS

#define CODEPTR() nullptr

static inline void fgt_alias_port(void * /*node*/, void * /*p*/, bool /*visible*/ ) { }

static inline void fgt_composite ( void* /*codeptr*/, void * /*node*/, void * /*graph*/ ) { }

static inline void fgt_graph( void * /*g*/ ) { }

template< typename NodeType >
static inline void fgt_multioutput_node_desc( const NodeType * /*node*/, const char * /*desc*/ ) { }

template< typename NodeType >
static inline void fgt_node_desc( const NodeType * /*node*/, const char * /*desc*/ ) { }

static inline void fgt_graph_desc( const void * /*g*/, const char * /*desc*/ ) { }

template< int N, typename PortsTuple >
static inline void fgt_multioutput_node( void* /*codeptr*/, string_resource_index /*t*/, void * /*g*/, void * /*input_port*/, PortsTuple & /*ports*/ ) { }

template< int N, typename PortsTuple >
static inline void fgt_multioutput_node_with_body( void* /*codeptr*/, string_resource_index /*t*/, void * /*g*/, void * /*input_port*/, PortsTuple & /*ports*/, void * /*body*/ ) { }

template< int N, typename PortsTuple >
static inline void fgt_multiinput_node( void* /*codeptr*/, string_resource_index /*t*/, void * /*g*/, PortsTuple & /*ports*/, void * /*output_port*/ ) { }

static inline void fgt_multiinput_multioutput_node( void* /*codeptr*/, string_resource_index /*t*/, void * /*node*/, void * /*graph*/ ) { }

static inline void fgt_node( void* /*codeptr*/, string_resource_index /*t*/, void * /*g*/, void * /*input_port*/, void * /*output_port*/ ) { }
static inline void  fgt_node( void* /*codeptr*/, string_resource_index /*t*/, void * /*g*/, void * /*input_port*/, void * /*decrement_port*/, void * /*output_port*/ ) { }

static inline void fgt_node_with_body( void* /*codeptr*/, string_resource_index /*t*/, void * /*g*/, void * /*output_port*/, void * /*body*/ ) { }
static inline void fgt_node_with_body( void* /*codeptr*/, string_resource_index /*t*/, void * /*g*/, void * /*input_port*/, void * /*output_port*/, void * /*body*/ ) { }

static inline void fgt_make_edge( void * /*output_port*/, void * /*input_port*/ ) { }
static inline void fgt_remove_edge( void * /*output_port*/, void * /*input_port*/ ) { }

static inline void fgt_begin_body( void * /*body*/ ) { }
static inline void fgt_end_body( void *  /*body*/) { }

static inline void fgt_async_try_put_begin( void * /*node*/, void * /*port*/ ) { }
static inline void fgt_async_try_put_end( void * /*node*/ , void * /*port*/ ) { }
static inline void fgt_async_reserve( void * /*node*/, void * /*graph*/ ) { }
static inline void fgt_async_commit( void * /*node*/, void * /*graph*/ ) { }
static inline void fgt_reserve_wait( void * /*graph*/ ) { }
static inline void fgt_release_wait( void * /*graph*/ ) { }

template< typename NodeType >
void fgt_multiinput_multioutput_node_desc( const NodeType * /*node*/, const char * /*desc*/ ) { }

template < typename PortsTuple, int N >
struct fgt_internal_input_alias_helper {
    static void alias_port( void * /*node*/, PortsTuple & /*ports*/ ) { }
};

template < typename PortsTuple, int N >
struct fgt_internal_output_alias_helper {
    static void alias_port( void * /*node*/, PortsTuple & /*ports*/ ) { }
};

#endif // TBB_USE_PROFILING_TOOLS

} // d1
} // namespace detail
} // namespace tbb

#include <functional>

#include "detail/_containers_helpers.h"

namespace tbb {
namespace detail {
namespace d1 {

// included in namespace tbb::detail::d1 (in flow_graph.h)

typedef std::uint64_t tag_value;


// TODO revamp: find out if there is already helper for has_policy.
template<typename ... Policies> struct Policy {};

template<typename ... Policies> struct has_policy;

template<typename ExpectedPolicy, typename FirstPolicy, typename ...Policies>
struct has_policy<ExpectedPolicy, FirstPolicy, Policies...> :
    std::integral_constant<bool, has_policy<ExpectedPolicy, FirstPolicy>::value ||
                                 has_policy<ExpectedPolicy, Policies...>::value> {};

template<typename ExpectedPolicy, typename SinglePolicy>
struct has_policy<ExpectedPolicy, SinglePolicy> :
    std::integral_constant<bool, std::is_same<ExpectedPolicy, SinglePolicy>::value> {};

template<typename ExpectedPolicy, typename ...Policies>
struct has_policy<ExpectedPolicy, Policy<Policies...> > : has_policy<ExpectedPolicy, Policies...> {};

namespace graph_policy_namespace {

    struct rejecting { };
    struct reserving { };
    struct queueing  { };
    struct lightweight  { };

    // K == type of field used for key-matching.  Each tag-matching port will be provided
    // functor that, given an object accepted by the port, will return the
    /// field of type K being used for matching.
    template<typename K, typename KHash=tbb_hash_compare<typename std::decay<K>::type > >
        __TBB_requires(tbb::detail::hash_compare<KHash, K>)
    struct key_matching {
        typedef K key_type;
        typedef typename std::decay<K>::type base_key_type;
        typedef KHash hash_compare_type;
    };

    // old tag_matching join's new specifier
    typedef key_matching<tag_value> tag_matching;

    // Aliases for Policy combinations
    typedef Policy<queueing, lightweight> queueing_lightweight;
    typedef Policy<rejecting, lightweight> rejecting_lightweight;

} // namespace graph_policy_namespace

// -------------- function_body containers ----------------------

//! A functor that takes no input and generates a value of type Output
template< typename Output >
class input_body : no_assign {
public:
    virtual ~input_body() {}
    virtual Output operator()(flow_control& fc) = 0;
    virtual input_body* clone() = 0;
};

//! The leaf for input_body
template< typename Output, typename Body>
class input_body_leaf : public input_body<Output> {
public:
    input_body_leaf( const Body &_body ) : body(_body) { }
    Output operator()(flow_control& fc) override { return body(fc); }
    input_body_leaf* clone() override {
        return new input_body_leaf< Output, Body >(body);
    }
    Body get_body() { return body; }
private:
    Body body;
};

//! A functor that takes an Input and generates an Output
template< typename Input, typename Output >
class function_body : no_assign {
public:
    virtual ~function_body() {}
    virtual Output operator()(const Input &input) = 0;
    virtual function_body* clone() = 0;
};

//! the leaf for function_body
template <typename Input, typename Output, typename B>
class function_body_leaf : public function_body< Input, Output > {
public:
    function_body_leaf( const B &_body ) : body(_body) { }
    Output operator()(const Input &i) override { return tbb::detail::invoke(body,i); }
    B get_body() { return body; }
    function_body_leaf* clone() override {
        return new function_body_leaf< Input, Output, B >(body);
    }
private:
    B body;
};

//! the leaf for function_body specialized for Input and output of continue_msg
template <typename B>
class function_body_leaf< continue_msg, continue_msg, B> : public function_body< continue_msg, continue_msg > {
public:
    function_body_leaf( const B &_body ) : body(_body) { }
    continue_msg operator()( const continue_msg &i ) override {
        body(i);
        return i;
    }
    B get_body() { return body; }
    function_body_leaf* clone() override {
        return new function_body_leaf< continue_msg, continue_msg, B >(body);
    }
private:
    B body;
};

//! the leaf for function_body specialized for Output of continue_msg
template <typename Input, typename B>
class function_body_leaf< Input, continue_msg, B> : public function_body< Input, continue_msg > {
public:
    function_body_leaf( const B &_body ) : body(_body) { }
    continue_msg operator()(const Input &i) override {
        body(i);
        return continue_msg();
    }
    B get_body() { return body; }
    function_body_leaf* clone() override {
        return new function_body_leaf< Input, continue_msg, B >(body);
    }
private:
    B body;
};

//! the leaf for function_body specialized for Input of continue_msg
template <typename Output, typename B>
class function_body_leaf< continue_msg, Output, B > : public function_body< continue_msg, Output > {
public:
    function_body_leaf( const B &_body ) : body(_body) { }
    Output operator()(const continue_msg &i) override {
        return body(i);
    }
    B get_body() { return body; }
    function_body_leaf* clone() override {
        return new function_body_leaf< continue_msg, Output, B >(body);
    }
private:
    B body;
};

//! function_body that takes an Input and a set of output ports
template<typename Input, typename OutputSet>
class multifunction_body : no_assign {
public:
    virtual ~multifunction_body () {}
    virtual void operator()(const Input &/* input*/, OutputSet &/*oset*/) = 0;
    virtual multifunction_body* clone() = 0;
    virtual void* get_body_ptr() = 0;
};

//! leaf for multifunction.  OutputSet can be a std::tuple or a vector.
template<typename Input, typename OutputSet, typename B >
class multifunction_body_leaf : public multifunction_body<Input, OutputSet> {
public:
    multifunction_body_leaf(const B &_body) : body(_body) { }
    void operator()(const Input &input, OutputSet &oset) override {
        tbb::detail::invoke(body, input, oset); // body may explicitly put() to one or more of oset.
    }
    void* get_body_ptr() override { return &body; }
    multifunction_body_leaf* clone() override {
        return new multifunction_body_leaf<Input, OutputSet,B>(body);
    }

private:
    B body;
};

// ------ function bodies for hash_buffers and key-matching joins.

template<typename Input, typename Output>
class type_to_key_function_body : no_assign {
    public:
        virtual ~type_to_key_function_body() {}
        virtual Output operator()(const Input &input) = 0;  // returns an Output
        virtual type_to_key_function_body* clone() = 0;
};

// specialization for ref output
template<typename Input, typename Output>
class type_to_key_function_body<Input,Output&> : no_assign {
    public:
        virtual ~type_to_key_function_body() {}
        virtual const Output & operator()(const Input &input) = 0;  // returns a const Output&
        virtual type_to_key_function_body* clone() = 0;
};

template <typename Input, typename Output, typename B>
class type_to_key_function_body_leaf : public type_to_key_function_body<Input, Output> {
public:
    type_to_key_function_body_leaf( const B &_body ) : body(_body) { }
    Output operator()(const Input &i) override { return tbb::detail::invoke(body, i); }
    type_to_key_function_body_leaf* clone() override {
        return new type_to_key_function_body_leaf< Input, Output, B>(body);
    }
private:
    B body;
};

template <typename Input, typename Output, typename B>
class type_to_key_function_body_leaf<Input,Output&,B> : public type_to_key_function_body< Input, Output&> {
public:
    type_to_key_function_body_leaf( const B &_body ) : body(_body) { }
    const Output& operator()(const Input &i) override {
        return tbb::detail::invoke(body, i);
    }
    type_to_key_function_body_leaf* clone() override {
        return new type_to_key_function_body_leaf< Input, Output&, B>(body);
    }
private:
    B body;
};

// --------------------------- end of function_body containers ------------------------

// --------------------------- node task bodies ---------------------------------------

//! A task that calls a node's forward_task function
template< typename NodeType >
class forward_task_bypass : public graph_task {
    NodeType &my_node;
public:
    forward_task_bypass( graph& g, small_object_allocator& allocator, NodeType &n
                         , node_priority_t node_priority = no_priority
    ) : graph_task(g, allocator, node_priority),
    my_node(n) {}

    task* execute(execution_data& ed) override {
        graph_task* next_task = my_node.forward_task();
        if (SUCCESSFULLY_ENQUEUED == next_task)
            next_task = nullptr;
        else if (next_task)
            next_task = prioritize_task(my_node.graph_reference(), *next_task);
        finalize<forward_task_bypass>(ed);
        return next_task;
    }

    task* cancel(execution_data& ed) override {
        finalize<forward_task_bypass>(ed);
        return nullptr;
    }
};

//! A task that calls a node's apply_body_bypass function, passing in an input of type Input
//  return the task* unless it is SUCCESSFULLY_ENQUEUED, in which case return nullptr
template< typename NodeType, typename Input >
class apply_body_task_bypass : public graph_task {
    NodeType &my_node;
    Input my_input;
public:

    apply_body_task_bypass( graph& g, small_object_allocator& allocator, NodeType &n, const Input &i
                            , node_priority_t node_priority = no_priority
    ) : graph_task(g, allocator, node_priority),
        my_node(n), my_input(i) {}

    task* execute(execution_data& ed) override {
        graph_task* next_task = my_node.apply_body_bypass( my_input );
        if (SUCCESSFULLY_ENQUEUED == next_task)
            next_task = nullptr;
        else if (next_task)
            next_task = prioritize_task(my_node.graph_reference(), *next_task);
        finalize<apply_body_task_bypass>(ed);
        return next_task;
    }

    task* cancel(execution_data& ed) override {
        finalize<apply_body_task_bypass>(ed);
        return nullptr;
    }
};

//! A task that calls a node's apply_body_bypass function with no input
template< typename NodeType >
class input_node_task_bypass : public graph_task {
    NodeType &my_node;
public:
    input_node_task_bypass( graph& g, small_object_allocator& allocator, NodeType &n )
        : graph_task(g, allocator), my_node(n) {}

    task* execute(execution_data& ed) override {
        graph_task* next_task = my_node.apply_body_bypass( );
        if (SUCCESSFULLY_ENQUEUED == next_task)
            next_task = nullptr;
        else if (next_task)
            next_task = prioritize_task(my_node.graph_reference(), *next_task);
        finalize<input_node_task_bypass>(ed);
        return next_task;
    }

    task* cancel(execution_data& ed) override {
        finalize<input_node_task_bypass>(ed);
        return nullptr;
    }
};

// ------------------------ end of node task bodies -----------------------------------

template<typename T, typename DecrementType, typename DummyType = void>
class threshold_regulator;

template<typename T, typename DecrementType>
class threshold_regulator<T, DecrementType,
                  typename std::enable_if<std::is_integral<DecrementType>::value>::type>
    : public receiver<DecrementType>, no_copy
{
    T* my_node;
protected:

    graph_task* try_put_task( const DecrementType& value ) override {
        graph_task* result = my_node->decrement_counter( value );
        if( !result )
            result = SUCCESSFULLY_ENQUEUED;
        return result;
    }

    graph& graph_reference() const override {
        return my_node->my_graph;
    }

    template<typename U, typename V> friend class limiter_node;
    void reset_receiver( reset_flags ) {}

public:
    threshold_regulator(T* owner) : my_node(owner) {
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }
};

template<typename T>
class threshold_regulator<T, continue_msg, void> : public continue_receiver, no_copy {

    T *my_node;

    graph_task* execute() override {
        return my_node->decrement_counter( 1 );
    }

protected:

    graph& graph_reference() const override {
        return my_node->my_graph;
    }

public:

    typedef continue_msg input_type;
    typedef continue_msg output_type;
    threshold_regulator(T* owner)
        : continue_receiver( /*number_of_predecessors=*/0, no_priority ), my_node(owner)
    {
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }
};

// included in namespace tbb::detail::d1 (in flow_graph.h)

//! A node_cache maintains a std::queue of elements of type T.  Each operation is protected by a lock.
template< typename T, typename M=spin_mutex >
class node_cache {
    public:

    typedef size_t size_type;

    bool empty() {
        typename mutex_type::scoped_lock lock( my_mutex );
        return internal_empty();
    }

    void add( T &n ) {
        typename mutex_type::scoped_lock lock( my_mutex );
        internal_push(n);
    }

    void remove( T &n ) {
        typename mutex_type::scoped_lock lock( my_mutex );
        for ( size_t i = internal_size(); i != 0; --i ) {
            T &s = internal_pop();
            if ( &s == &n )
                break;  // only remove one predecessor per request
            internal_push(s);
        }
    }

    void clear() {
        while( !my_q.empty()) (void)my_q.pop();
    }

protected:

    typedef M mutex_type;
    mutex_type my_mutex;
    std::queue< T * > my_q;

    // Assumes lock is held
    inline bool internal_empty( )  {
        return my_q.empty();
    }

    // Assumes lock is held
    inline size_type internal_size( )  {
        return my_q.size();
    }

    // Assumes lock is held
    inline void internal_push( T &n )  {
        my_q.push(&n);
    }

    // Assumes lock is held
    inline T &internal_pop() {
        T *v = my_q.front();
        my_q.pop();
        return *v;
    }

};

//! A cache of predecessors that only supports try_get
template< typename T, typename M=spin_mutex >
class predecessor_cache : public node_cache< sender<T>, M > {
public:
    typedef M mutex_type;
    typedef T output_type;
    typedef sender<output_type> predecessor_type;
    typedef receiver<output_type> successor_type;

    predecessor_cache( successor_type* owner ) : my_owner( owner ) {
        __TBB_ASSERT( my_owner, "predecessor_cache should have an owner." );
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }

    bool get_item( output_type& v ) {

        bool msg = false;

        do {
            predecessor_type *src;
            {
                typename mutex_type::scoped_lock lock(this->my_mutex);
                if ( this->internal_empty() ) {
                    break;
                }
                src = &this->internal_pop();
            }

            // Try to get from this sender
            msg = src->try_get( v );

            if (msg == false) {
                // Relinquish ownership of the edge
                register_successor(*src, *my_owner);
            } else {
                // Retain ownership of the edge
                this->add(*src);
            }
        } while ( msg == false );
        return msg;
    }

    // If we are removing arcs (rf_clear_edges), call clear() rather than reset().
    void reset() {
        for(;;) {
            predecessor_type *src;
            {
                if (this->internal_empty()) break;
                src = &this->internal_pop();
            }
            register_successor(*src, *my_owner);
        }
    }

protected:
    successor_type* my_owner;
};

//! An cache of predecessors that supports requests and reservations
template< typename T, typename M=spin_mutex >
class reservable_predecessor_cache : public predecessor_cache< T, M > {
public:
    typedef M mutex_type;
    typedef T output_type;
    typedef sender<T> predecessor_type;
    typedef receiver<T> successor_type;

    reservable_predecessor_cache( successor_type* owner )
        : predecessor_cache<T,M>(owner), reserved_src(nullptr)
    {
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }

    bool try_reserve( output_type &v ) {
        bool msg = false;

        do {
            predecessor_type* pred = nullptr;
            {
                typename mutex_type::scoped_lock lock(this->my_mutex);
                if ( reserved_src.load(std::memory_order_relaxed) || this->internal_empty() )
                    return false;

                pred = &this->internal_pop();
                reserved_src.store(pred, std::memory_order_relaxed);
            }

            // Try to get from this sender
            msg = pred->try_reserve( v );

            if (msg == false) {
                typename mutex_type::scoped_lock lock(this->my_mutex);
                // Relinquish ownership of the edge
                register_successor( *pred, *this->my_owner );
                reserved_src.store(nullptr, std::memory_order_relaxed);
            } else {
                // Retain ownership of the edge
                this->add( *pred);
            }
        } while ( msg == false );

        return msg;
    }

    bool try_release() {
        reserved_src.load(std::memory_order_relaxed)->try_release();
        reserved_src.store(nullptr, std::memory_order_relaxed);
        return true;
    }

    bool try_consume() {
        reserved_src.load(std::memory_order_relaxed)->try_consume();
        reserved_src.store(nullptr, std::memory_order_relaxed);
        return true;
    }

    void reset() {
        reserved_src.store(nullptr, std::memory_order_relaxed);
        predecessor_cache<T, M>::reset();
    }

    void clear() {
        reserved_src.store(nullptr, std::memory_order_relaxed);
        predecessor_cache<T, M>::clear();
    }

private:
    std::atomic<predecessor_type*> reserved_src;
};


//! An abstract cache of successors
template<typename T, typename M=spin_rw_mutex >
class successor_cache : no_copy {
protected:

    typedef M mutex_type;
    mutex_type my_mutex;

    typedef receiver<T> successor_type;
    typedef receiver<T>* pointer_type;
    typedef sender<T> owner_type;
    // TODO revamp: introduce heapified collection of successors for strict priorities
    typedef std::list< pointer_type > successors_type;
    successors_type my_successors;

    owner_type* my_owner;

public:
    successor_cache( owner_type* owner ) : my_owner(owner) {
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }

    virtual ~successor_cache() {}

    void register_successor( successor_type& r ) {
        typename mutex_type::scoped_lock l(my_mutex, true);
        if( r.priority() != no_priority )
            my_successors.push_front( &r );
        else
            my_successors.push_back( &r );
    }

    void remove_successor( successor_type& r ) {
        typename mutex_type::scoped_lock l(my_mutex, true);
        for ( typename successors_type::iterator i = my_successors.begin();
              i != my_successors.end(); ++i ) {
            if ( *i == & r ) {
                my_successors.erase(i);
                break;
            }
        }
    }

    bool empty() {
        typename mutex_type::scoped_lock l(my_mutex, false);
        return my_successors.empty();
    }

    void clear() {
        my_successors.clear();
    }

    virtual graph_task* try_put_task( const T& t ) = 0;
};  // successor_cache<T>

//! An abstract cache of successors, specialized to continue_msg
template<typename M>
class successor_cache< continue_msg, M > : no_copy {
protected:

    typedef M mutex_type;
    mutex_type my_mutex;

    typedef receiver<continue_msg> successor_type;
    typedef receiver<continue_msg>* pointer_type;
    typedef sender<continue_msg> owner_type;
    typedef std::list< pointer_type > successors_type;
    successors_type my_successors;
    owner_type* my_owner;

public:
    successor_cache( sender<continue_msg>* owner ) : my_owner(owner) {
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }

    virtual ~successor_cache() {}

    void register_successor( successor_type& r ) {
        typename mutex_type::scoped_lock l(my_mutex, true);
        if( r.priority() != no_priority )
            my_successors.push_front( &r );
        else
            my_successors.push_back( &r );
        __TBB_ASSERT( my_owner, "Cache of successors must have an owner." );
        if ( r.is_continue_receiver() ) {
            r.register_predecessor( *my_owner );
        }
    }

    void remove_successor( successor_type& r ) {
        typename mutex_type::scoped_lock l(my_mutex, true);
        for ( successors_type::iterator i = my_successors.begin(); i != my_successors.end(); ++i ) {
            if ( *i == &r ) {
                __TBB_ASSERT(my_owner, "Cache of successors must have an owner.");
                // TODO: check if we need to test for continue_receiver before removing from r.
                r.remove_predecessor( *my_owner );
                my_successors.erase(i);
                break;
            }
        }
    }

    bool empty() {
        typename mutex_type::scoped_lock l(my_mutex, false);
        return my_successors.empty();
    }

    void clear() {
        my_successors.clear();
    }

    virtual graph_task* try_put_task( const continue_msg& t ) = 0;
};  // successor_cache< continue_msg >

//! A cache of successors that are broadcast to
template<typename T, typename M=spin_rw_mutex>
class broadcast_cache : public successor_cache<T, M> {
    typedef successor_cache<T, M> base_type;
    typedef M mutex_type;
    typedef typename successor_cache<T,M>::successors_type successors_type;

public:

    broadcast_cache( typename base_type::owner_type* owner ): base_type(owner) {
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }

    // as above, but call try_put_task instead, and return the last task we received (if any)
    graph_task* try_put_task( const T &t ) override {
        graph_task * last_task = nullptr;
        typename mutex_type::scoped_lock l(this->my_mutex, /*write=*/true);
        typename successors_type::iterator i = this->my_successors.begin();
        while ( i != this->my_successors.end() ) {
            graph_task *new_task = (*i)->try_put_task(t);
            // workaround for icc bug
            graph& graph_ref = (*i)->graph_reference();
            last_task = combine_tasks(graph_ref, last_task, new_task);  // enqueue if necessary
            if(new_task) {
                ++i;
            }
            else {  // failed
                if ( (*i)->register_predecessor(*this->my_owner) ) {
                    i = this->my_successors.erase(i);
                } else {
                    ++i;
                }
            }
        }
        return last_task;
    }

    // call try_put_task and return list of received tasks
    bool gather_successful_try_puts( const T &t, graph_task_list& tasks ) {
        bool is_at_least_one_put_successful = false;
        typename mutex_type::scoped_lock l(this->my_mutex, /*write=*/true);
        typename successors_type::iterator i = this->my_successors.begin();
        while ( i != this->my_successors.end() ) {
            graph_task * new_task = (*i)->try_put_task(t);
            if(new_task) {
                ++i;
                if(new_task != SUCCESSFULLY_ENQUEUED) {
                    tasks.push_back(*new_task);
                }
                is_at_least_one_put_successful = true;
            }
            else {  // failed
                if ( (*i)->register_predecessor(*this->my_owner) ) {
                    i = this->my_successors.erase(i);
                } else {
                    ++i;
                }
            }
        }
        return is_at_least_one_put_successful;
    }
};

//! A cache of successors that are put in a round-robin fashion
template<typename T, typename M=spin_rw_mutex >
class round_robin_cache : public successor_cache<T, M> {
    typedef successor_cache<T, M> base_type;
    typedef size_t size_type;
    typedef M mutex_type;
    typedef typename successor_cache<T,M>::successors_type successors_type;

public:

    round_robin_cache( typename base_type::owner_type* owner ): base_type(owner) {
        // Do not work with the passed pointer here as it may not be fully initialized yet
    }

    size_type size() {
        typename mutex_type::scoped_lock l(this->my_mutex, false);
        return this->my_successors.size();
    }

    graph_task* try_put_task( const T &t ) override {
        typename mutex_type::scoped_lock l(this->my_mutex, /*write=*/true);
        typename successors_type::iterator i = this->my_successors.begin();
        while ( i != this->my_successors.end() ) {
            graph_task* new_task = (*i)->try_put_task(t);
            if ( new_task ) {
                return new_task;
            } else {
               if ( (*i)->register_predecessor(*this->my_owner) ) {
                   i = this->my_successors.erase(i);
               }
               else {
                   ++i;
               }
            }
        }
        return nullptr;
    }
};



using namespace graph_policy_namespace;

template <typename C, typename N>
graph_iterator<C,N>::graph_iterator(C *g, bool begin) : my_graph(g), current_node(nullptr)
{
    if (begin) current_node = my_graph->my_nodes;
    //else it is an end iterator by default
}

template <typename C, typename N>
typename graph_iterator<C,N>::reference graph_iterator<C,N>::operator*() const {
    __TBB_ASSERT(current_node, "graph_iterator at end");
    return *operator->();
}

template <typename C, typename N>
typename graph_iterator<C,N>::pointer graph_iterator<C,N>::operator->() const {
    return current_node;
}

template <typename C, typename N>
void graph_iterator<C,N>::internal_forward() {
    if (current_node) current_node = current_node->next;
}

//! Constructs a graph with isolated task_group_context
inline graph::graph() : my_wait_context(0), my_nodes(nullptr), my_nodes_last(nullptr), my_task_arena(nullptr) {
    prepare_task_arena();
    own_context = true;
    cancelled = false;
    caught_exception = false;
    my_context = new (r1::cache_aligned_allocate(sizeof(task_group_context))) task_group_context(FLOW_TASKS);
    fgt_graph(this);
    my_is_active = true;
}

inline graph::graph(task_group_context& use_this_context) :
    my_wait_context(0), my_context(&use_this_context), my_nodes(nullptr), my_nodes_last(nullptr), my_task_arena(nullptr) {
    prepare_task_arena();
    own_context = false;
    cancelled = false;
    caught_exception = false;
    fgt_graph(this);
    my_is_active = true;
}

inline graph::~graph() {
    wait_for_all();
    if (own_context) {
        my_context->~task_group_context();
        r1::cache_aligned_deallocate(my_context);
    }
    delete my_task_arena;
}

inline void graph::reserve_wait() {
    my_wait_context.reserve();
    fgt_reserve_wait(this);
}

inline void graph::release_wait() {
    fgt_release_wait(this);
    my_wait_context.release();
}

inline void graph::register_node(graph_node *n) {
    n->next = nullptr;
    {
        spin_mutex::scoped_lock lock(nodelist_mutex);
        n->prev = my_nodes_last;
        if (my_nodes_last) my_nodes_last->next = n;
        my_nodes_last = n;
        if (!my_nodes) my_nodes = n;
    }
}

inline void graph::remove_node(graph_node *n) {
    {
        spin_mutex::scoped_lock lock(nodelist_mutex);
        __TBB_ASSERT(my_nodes && my_nodes_last, "graph::remove_node: Error: no registered nodes");
        if (n->prev) n->prev->next = n->next;
        if (n->next) n->next->prev = n->prev;
        if (my_nodes_last == n) my_nodes_last = n->prev;
        if (my_nodes == n) my_nodes = n->next;
    }
    n->prev = n->next = nullptr;
}

inline void graph::reset( reset_flags f ) {
    // reset context
    deactivate_graph(*this);

    my_context->reset();
    cancelled = false;
    caught_exception = false;
    // reset all the nodes comprising the graph
    for(iterator ii = begin(); ii != end(); ++ii) {
        graph_node *my_p = &(*ii);
        my_p->reset_node(f);
    }
    // Reattach the arena. Might be useful to run the graph in a particular task_arena
    // while not limiting graph lifetime to a single task_arena::execute() call.
    prepare_task_arena( /*reinit=*/true );
    activate_graph(*this);
}

inline void graph::cancel() {
    my_context->cancel_group_execution();
}

inline graph::iterator graph::begin() { return iterator(this, true); }

inline graph::iterator graph::end() { return iterator(this, false); }

inline graph::const_iterator graph::begin() const { return const_iterator(this, true); }

inline graph::const_iterator graph::end() const { return const_iterator(this, false); }

inline graph::const_iterator graph::cbegin() const { return const_iterator(this, true); }

inline graph::const_iterator graph::cend() const { return const_iterator(this, false); }

inline graph_node::graph_node(graph& g) : my_graph(g) {
    my_graph.register_node(this);
}

inline graph_node::~graph_node() {
    my_graph.remove_node(this);
}

// included in namespace tbb::detail::d1

// included in namespace tbb::detail::d1

// the change to key_matching (adding a K and KHash template parameter, making it a class)
// means we have to pass this data to the key_matching_port.  All the ports have only one
// template parameter, so we have to wrap the following types in a trait:
//
//    . K == key_type
//    . KHash == hash and compare for Key
//    . TtoK == function_body that given an object of T, returns its K
//    . T == type accepted by port, and stored in the hash table
//
// The port will have an additional parameter on node construction, which is a function_body
// that accepts a const T& and returns a K which is the field in T which is its K.
template<typename Kp, typename KHashp, typename Tp>
struct KeyTrait {
    typedef Kp K;
    typedef Tp T;
    typedef type_to_key_function_body<T,K> TtoK;
    typedef KHashp KHash;
};

// wrap each element of a tuple in a template, and make a tuple of the result.
template<int N, template<class> class PT, typename TypeTuple>
struct wrap_tuple_elements;

// A wrapper that generates the traits needed for each port of a key-matching join,
// and the type of the tuple of input ports.
template<int N, template<class> class PT, typename KeyTraits, typename TypeTuple>
struct wrap_key_tuple_elements;

template<int N, template<class> class PT,  typename... Args>
struct wrap_tuple_elements<N, PT, std::tuple<Args...> >{
    typedef typename std::tuple<PT<Args>... > type;
};

template<int N, template<class> class PT, typename KeyTraits, typename... Args>
struct wrap_key_tuple_elements<N, PT, KeyTraits, std::tuple<Args...> > {
    typedef typename KeyTraits::key_type K;
    typedef typename KeyTraits::hash_compare_type KHash;
    typedef typename std::tuple<PT<KeyTrait<K, KHash, Args> >... > type;
};

template< int... S > class sequence {};

template< int N, int... S >
struct make_sequence : make_sequence < N - 1, N - 1, S... > {};

template< int... S >
struct make_sequence < 0, S... > {
    typedef sequence<S...> type;
};

//! type mimicking std::pair but with trailing fill to ensure each element of an array
//* will have the correct alignment
template<typename T1, typename T2, size_t REM>
struct type_plus_align {
    char first[sizeof(T1)];
    T2 second;
    char fill1[REM];
};

template<typename T1, typename T2>
struct type_plus_align<T1,T2,0> {
    char first[sizeof(T1)];
    T2 second;
};

template<class U> struct alignment_of {
    typedef struct { char t; U    padded; } test_alignment;
    static const size_t value = sizeof(test_alignment) - sizeof(U);
};

// T1, T2 are actual types stored.  The space defined for T1 in the type returned
// is a char array of the correct size.  Type T2 should be trivially-constructible,
// T1 must be explicitly managed.
template<typename T1, typename T2>
struct aligned_pair {
    static const size_t t1_align = alignment_of<T1>::value;
    static const size_t t2_align = alignment_of<T2>::value;
    typedef type_plus_align<T1, T2, 0 > just_pair;
    static const size_t max_align = t1_align < t2_align ? t2_align : t1_align;
    static const size_t extra_bytes = sizeof(just_pair) % max_align;
    static const size_t remainder = extra_bytes ? max_align - extra_bytes : 0;
public:
    typedef type_plus_align<T1,T2,remainder> type;
};  // aligned_pair

// support for variant type
// type we use when we're not storing a value
struct default_constructed { };

// type which contains another type, tests for what type is contained, and references to it.
// Wrapper<T>
//     void CopyTo( void *newSpace) : builds a Wrapper<T> copy of itself in newSpace

// struct to allow us to copy and test the type of objects
struct WrapperBase {
    virtual ~WrapperBase() {}
    virtual void CopyTo(void* /*newSpace*/) const = 0;
};

// Wrapper<T> contains a T, with the ability to test what T is.  The Wrapper<T> can be
// constructed from a T, can be copy-constructed from another Wrapper<T>, and can be
// examined via value(), but not modified.
template<typename T>
struct Wrapper: public WrapperBase {
    typedef T value_type;
    typedef T* pointer_type;
private:
    T value_space;
public:
    const value_type &value() const { return value_space; }

private:
    Wrapper();

    // on exception will ensure the Wrapper will contain only a trivially-constructed object
    struct _unwind_space {
        pointer_type space;
        _unwind_space(pointer_type p) : space(p) {}
        ~_unwind_space() {
            if(space) (void) new (space) Wrapper<default_constructed>(default_constructed());
        }
    };
public:
    explicit Wrapper( const T& other ) : value_space(other) { }
    explicit Wrapper(const Wrapper& other) = delete;

    void CopyTo(void* newSpace) const override {
        _unwind_space guard((pointer_type)newSpace);
        (void) new(newSpace) Wrapper(value_space);
        guard.space = nullptr;
    }
    ~Wrapper() { }
};

// specialization for array objects
template<typename T, size_t N>
struct Wrapper<T[N]> : public WrapperBase {
    typedef T value_type;
    typedef T* pointer_type;
    // space must be untyped.
    typedef T ArrayType[N];
private:
    // The space is not of type T[N] because when copy-constructing, it would be
    // default-initialized and then copied to in some fashion, resulting in two
    // constructions and one destruction per element.  If the type is char[ ], we
    // placement new into each element, resulting in one construction per element.
    static const size_t space_size = sizeof(ArrayType);
    char value_space[space_size];


    // on exception will ensure the already-built objects will be destructed
    // (the value_space is a char array, so it is already trivially-destructible.)
    struct _unwind_class {
        pointer_type space;
        int    already_built;
        _unwind_class(pointer_type p) : space(p), already_built(0) {}
        ~_unwind_class() {
            if(space) {
                for(size_t i = already_built; i > 0 ; --i ) space[i-1].~value_type();
                (void) new(space) Wrapper<default_constructed>(default_constructed());
            }
        }
    };
public:
    const ArrayType &value() const {
        char *vp = const_cast<char *>(value_space);
        return reinterpret_cast<ArrayType &>(*vp);
    }

private:
    Wrapper();
public:
    // have to explicitly construct because other decays to a const value_type*
    explicit Wrapper(const ArrayType& other) {
        _unwind_class guard((pointer_type)value_space);
        pointer_type vp = reinterpret_cast<pointer_type>(&value_space);
        for(size_t i = 0; i < N; ++i ) {
            (void) new(vp++) value_type(other[i]);
            ++(guard.already_built);
        }
        guard.space = nullptr;
    }
    explicit Wrapper(const Wrapper& other) : WrapperBase() {
        // we have to do the heavy lifting to copy contents
        _unwind_class guard((pointer_type)value_space);
        pointer_type dp = reinterpret_cast<pointer_type>(value_space);
        pointer_type sp = reinterpret_cast<pointer_type>(const_cast<char *>(other.value_space));
        for(size_t i = 0; i < N; ++i, ++dp, ++sp) {
            (void) new(dp) value_type(*sp);
            ++(guard.already_built);
        }
        guard.space = nullptr;
    }

    void CopyTo(void* newSpace) const override {
        (void) new(newSpace) Wrapper(*this);  // exceptions handled in copy constructor
    }

    ~Wrapper() {
        // have to destroy explicitly in reverse order
        pointer_type vp = reinterpret_cast<pointer_type>(&value_space);
        for(size_t i = N; i > 0 ; --i ) vp[i-1].~value_type();
    }
};

// given a tuple, return the type of the element that has the maximum alignment requirement.
// Given a tuple and that type, return the number of elements of the object with the max
// alignment requirement that is at least as big as the largest object in the tuple.

template<bool, class T1, class T2> struct pick_one;
template<class T1, class T2> struct pick_one<true , T1, T2> { typedef T1 type; };
template<class T1, class T2> struct pick_one<false, T1, T2> { typedef T2 type; };

template< template<class> class Selector, typename T1, typename T2 >
struct pick_max {
    typedef typename pick_one< (Selector<T1>::value > Selector<T2>::value), T1, T2 >::type type;
};

template<typename T> struct size_of { static const int value = sizeof(T); };

template< size_t N, class Tuple, template<class> class Selector > struct pick_tuple_max {
    typedef typename pick_tuple_max<N-1, Tuple, Selector>::type LeftMaxType;
    typedef typename std::tuple_element<N-1, Tuple>::type ThisType;
    typedef typename pick_max<Selector, LeftMaxType, ThisType>::type type;
};

template< class Tuple, template<class> class Selector > struct pick_tuple_max<0, Tuple, Selector> {
    typedef typename std::tuple_element<0, Tuple>::type type;
};

// is the specified type included in a tuple?
template<class Q, size_t N, class Tuple>
struct is_element_of {
    typedef typename std::tuple_element<N-1, Tuple>::type T_i;
    static const bool value = std::is_same<Q,T_i>::value || is_element_of<Q,N-1,Tuple>::value;
};

template<class Q, class Tuple>
struct is_element_of<Q,0,Tuple> {
    typedef typename std::tuple_element<0, Tuple>::type T_i;
    static const bool value = std::is_same<Q,T_i>::value;
};

// allow the construction of types that are listed tuple.  If a disallowed type
// construction is written, a method involving this type is created.  The
// type has no definition, so a syntax error is generated.
template<typename T> struct ERROR_Type_Not_allowed_In_Tagged_Msg_Not_Member_Of_Tuple;

template<typename T, bool BUILD_IT> struct do_if;
template<typename T>
struct do_if<T, true> {
    static void construct(void *mySpace, const T& x) {
        (void) new(mySpace) Wrapper<T>(x);
    }
};
template<typename T>
struct do_if<T, false> {
    static void construct(void * /*mySpace*/, const T& x) {
        // This method is instantiated when the type T does not match any of the
        // element types in the Tuple in variant<Tuple>.
        ERROR_Type_Not_allowed_In_Tagged_Msg_Not_Member_Of_Tuple<T>::bad_type(x);
    }
};

// Tuple tells us the allowed types that variant can hold.  It determines the alignment of the space in
// Wrapper, and how big Wrapper is.
//
// the object can only be tested for type, and a read-only reference can be fetched by cast_to<T>().

using tbb::detail::punned_cast;
struct tagged_null_type {};
template<typename TagType, typename T0, typename T1=tagged_null_type, typename T2=tagged_null_type, typename T3=tagged_null_type,
                           typename T4=tagged_null_type, typename T5=tagged_null_type, typename T6=tagged_null_type,
                           typename T7=tagged_null_type, typename T8=tagged_null_type, typename T9=tagged_null_type>
class tagged_msg {
    typedef std::tuple<T0, T1, T2, T3, T4
                  //TODO: Should we reject lists longer than a tuple can hold?
                  #if __TBB_VARIADIC_MAX >= 6
                  , T5
                  #endif
                  #if __TBB_VARIADIC_MAX >= 7
                  , T6
                  #endif
                  #if __TBB_VARIADIC_MAX >= 8
                  , T7
                  #endif
                  #if __TBB_VARIADIC_MAX >= 9
                  , T8
                  #endif
                  #if __TBB_VARIADIC_MAX >= 10
                  , T9
                  #endif
                  > Tuple;

private:
    class variant {
        static const size_t N = std::tuple_size<Tuple>::value;
        typedef typename pick_tuple_max<N, Tuple, alignment_of>::type AlignType;
        typedef typename pick_tuple_max<N, Tuple, size_of>::type MaxSizeType;
        static const size_t MaxNBytes = (sizeof(Wrapper<MaxSizeType>)+sizeof(AlignType)-1);
        static const size_t MaxNElements = MaxNBytes/sizeof(AlignType);
        typedef aligned_space<AlignType, MaxNElements> SpaceType;
        SpaceType my_space;
        static const size_t MaxSize = sizeof(SpaceType);

    public:
        variant() { (void) new(&my_space) Wrapper<default_constructed>(default_constructed()); }

        template<typename T>
        variant( const T& x ) {
            do_if<T, is_element_of<T, N, Tuple>::value>::construct(&my_space,x);
        }

        variant(const variant& other) {
            const WrapperBase * h = punned_cast<const WrapperBase *>(&(other.my_space));
            h->CopyTo(&my_space);
        }

        // assignment must destroy and re-create the Wrapper type, as there is no way
        // to create a Wrapper-to-Wrapper assign even if we find they agree in type.
        void operator=( const variant& rhs ) {
            if(&rhs != this) {
                WrapperBase *h = punned_cast<WrapperBase *>(&my_space);
                h->~WrapperBase();
                const WrapperBase *ch = punned_cast<const WrapperBase *>(&(rhs.my_space));
                ch->CopyTo(&my_space);
            }
        }

        template<typename U>
        const U& variant_cast_to() const {
            const Wrapper<U> *h = dynamic_cast<const Wrapper<U>*>(punned_cast<const WrapperBase *>(&my_space));
            if(!h) {
                throw_exception(exception_id::bad_tagged_msg_cast);
            }
            return h->value();
        }
        template<typename U>
        bool variant_is_a() const { return dynamic_cast<const Wrapper<U>*>(punned_cast<const WrapperBase *>(&my_space)) != nullptr; }

        bool variant_is_default_constructed() const {return variant_is_a<default_constructed>();}

        ~variant() {
            WrapperBase *h = punned_cast<WrapperBase *>(&my_space);
            h->~WrapperBase();
        }
    }; //class variant

    TagType my_tag;
    variant my_msg;

public:
    tagged_msg(): my_tag(TagType(~0)), my_msg(){}

    template<typename T, typename R>
    tagged_msg(T const &index, R const &value) : my_tag(index), my_msg(value) {}

    template<typename T, typename R, size_t N>
    tagged_msg(T const &index,  R (&value)[N]) : my_tag(index), my_msg(value) {}

    void set_tag(TagType const &index) {my_tag = index;}
    TagType tag() const {return my_tag;}

    template<typename V>
    const V& cast_to() const {return my_msg.template variant_cast_to<V>();}

    template<typename V>
    bool is_a() const {return my_msg.template variant_is_a<V>();}

    bool is_default_constructed() const {return my_msg.variant_is_default_constructed();}
}; //class tagged_msg

// template to simplify cast and test for tagged_msg in template contexts
template<typename V, typename T>
const V& cast_to(T const &t) { return t.template cast_to<V>(); }

template<typename V, typename T>
bool is_a(T const &t) { return t.template is_a<V>(); }

enum op_stat { WAIT = 0, SUCCEEDED, FAILED };

    // Output of the indexer_node is a tbb::flow::tagged_msg, and will be of
    // the form  tagged_msg<tag, result>
    // where the value of tag will indicate which result was put to the
    // successor.

    template<typename IndexerNodeBaseType, typename T, size_t K>
    graph_task* do_try_put(const T &v, void *p) {
        typename IndexerNodeBaseType::output_type o(K, v);
        return reinterpret_cast<IndexerNodeBaseType *>(p)->try_put_task(&o);
    }

    template<typename TupleTypes,int N>
    struct indexer_helper {
        template<typename IndexerNodeBaseType, typename PortTuple>
        static inline void set_indexer_node_pointer(PortTuple &my_input, IndexerNodeBaseType *p, graph& g) {
            typedef typename std::tuple_element<N-1, TupleTypes>::type T;
            graph_task* (*indexer_node_put_task)(const T&, void *) = do_try_put<IndexerNodeBaseType, T, N-1>;
            std::get<N-1>(my_input).set_up(p, indexer_node_put_task, g);
            indexer_helper<TupleTypes,N-1>::template set_indexer_node_pointer<IndexerNodeBaseType,PortTuple>(my_input, p, g);
        }
    };

    template<typename TupleTypes>
    struct indexer_helper<TupleTypes,1> {
        template<typename IndexerNodeBaseType, typename PortTuple>
        static inline void set_indexer_node_pointer(PortTuple &my_input, IndexerNodeBaseType *p, graph& g) {
            typedef typename std::tuple_element<0, TupleTypes>::type T;
            graph_task* (*indexer_node_put_task)(const T&, void *) = do_try_put<IndexerNodeBaseType, T, 0>;
            std::get<0>(my_input).set_up(p, indexer_node_put_task, g);
        }
    };

    template<typename T>
    class indexer_input_port : public receiver<T> {
    private:
        void* my_indexer_ptr;
        typedef graph_task* (* forward_function_ptr)(T const &, void* );
        forward_function_ptr my_try_put_task;
        graph* my_graph;
    public:
        void set_up(void* p, forward_function_ptr f, graph& g) {
            my_indexer_ptr = p;
            my_try_put_task = f;
            my_graph = &g;
        }

    protected:
        template< typename R, typename B > friend class run_and_put_task;
        template<typename X, typename Y> friend class broadcast_cache;
        template<typename X, typename Y> friend class round_robin_cache;
        graph_task* try_put_task(const T &v) override {
            return my_try_put_task(v, my_indexer_ptr);
        }

        graph& graph_reference() const override {
            return *my_graph;
        }
    };

    template<typename InputTuple, typename OutputType, typename StructTypes>
    class indexer_node_FE {
    public:
        static const int N = std::tuple_size<InputTuple>::value;
        typedef OutputType output_type;
        typedef InputTuple input_type;

        // Some versions of Intel(R) C++ Compiler fail to generate an implicit constructor for the class which has std::tuple as a member.
        indexer_node_FE() : my_inputs() {}

        input_type &input_ports() { return my_inputs; }
    protected:
        input_type my_inputs;
    };

    //! indexer_node_base
    template<typename InputTuple, typename OutputType, typename StructTypes>
    class indexer_node_base : public graph_node, public indexer_node_FE<InputTuple, OutputType,StructTypes>,
                           public sender<OutputType> {
    protected:
       using graph_node::my_graph;
    public:
        static const size_t N = std::tuple_size<InputTuple>::value;
        typedef OutputType output_type;
        typedef StructTypes tuple_types;
        typedef typename sender<output_type>::successor_type successor_type;
        typedef indexer_node_FE<InputTuple, output_type,StructTypes> input_ports_type;

    private:
        // ----------- Aggregator ------------
        enum op_type { reg_succ, rem_succ, try__put_task
        };
        typedef indexer_node_base<InputTuple,output_type,StructTypes> class_type;

        class indexer_node_base_operation : public aggregated_operation<indexer_node_base_operation> {
        public:
            char type;
            union {
                output_type const *my_arg;
                successor_type *my_succ;
                graph_task* bypass_t;
            };
            indexer_node_base_operation(const output_type* e, op_type t) :
                type(char(t)), my_arg(e) {}
            indexer_node_base_operation(const successor_type &s, op_type t) : type(char(t)),
                my_succ(const_cast<successor_type *>(&s)) {}
        };

        typedef aggregating_functor<class_type, indexer_node_base_operation> handler_type;
        friend class aggregating_functor<class_type, indexer_node_base_operation>;
        aggregator<handler_type, indexer_node_base_operation> my_aggregator;

        void handle_operations(indexer_node_base_operation* op_list) {
            indexer_node_base_operation *current;
            while(op_list) {
                current = op_list;
                op_list = op_list->next;
                switch(current->type) {

                case reg_succ:
                    my_successors.register_successor(*(current->my_succ));
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;

                case rem_succ:
                    my_successors.remove_successor(*(current->my_succ));
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                case try__put_task: {
                        current->bypass_t = my_successors.try_put_task(*(current->my_arg));
                        current->status.store( SUCCEEDED, std::memory_order_release);  // return of try_put_task actual return value
                    }
                    break;
                }
            }
        }
        // ---------- end aggregator -----------
    public:
        indexer_node_base(graph& g) : graph_node(g), input_ports_type(), my_successors(this) {
            indexer_helper<StructTypes,N>::set_indexer_node_pointer(this->my_inputs, this, g);
            my_aggregator.initialize_handler(handler_type(this));
        }

        indexer_node_base(const indexer_node_base& other)
            : graph_node(other.my_graph), input_ports_type(), sender<output_type>(), my_successors(this)
        {
            indexer_helper<StructTypes,N>::set_indexer_node_pointer(this->my_inputs, this, other.my_graph);
            my_aggregator.initialize_handler(handler_type(this));
        }

        bool register_successor(successor_type &r) override {
            indexer_node_base_operation op_data(r, reg_succ);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        bool remove_successor( successor_type &r) override {
            indexer_node_base_operation op_data(r, rem_succ);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        graph_task* try_put_task(output_type const *v) { // not a virtual method in this class
            indexer_node_base_operation op_data(v, try__put_task);
            my_aggregator.execute(&op_data);
            return op_data.bypass_t;
        }

    protected:
        void reset_node(reset_flags f) override {
            if(f & rf_clear_edges) {
                my_successors.clear();
            }
        }

    private:
        broadcast_cache<output_type, null_rw_mutex> my_successors;
    };  //indexer_node_base


    template<int N, typename InputTuple> struct input_types;

    template<typename InputTuple>
    struct input_types<1, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef tagged_msg<size_t, first_type > type;
    };

    template<typename InputTuple>
    struct input_types<2, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef tagged_msg<size_t, first_type, second_type> type;
    };

    template<typename InputTuple>
    struct input_types<3, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type> type;
    };

    template<typename InputTuple>
    struct input_types<4, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef typename std::tuple_element<3, InputTuple>::type fourth_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type,
                                                      fourth_type> type;
    };

    template<typename InputTuple>
    struct input_types<5, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef typename std::tuple_element<3, InputTuple>::type fourth_type;
        typedef typename std::tuple_element<4, InputTuple>::type fifth_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type,
                                                      fourth_type, fifth_type> type;
    };

    template<typename InputTuple>
    struct input_types<6, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef typename std::tuple_element<3, InputTuple>::type fourth_type;
        typedef typename std::tuple_element<4, InputTuple>::type fifth_type;
        typedef typename std::tuple_element<5, InputTuple>::type sixth_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type,
                                                      fourth_type, fifth_type, sixth_type> type;
    };

    template<typename InputTuple>
    struct input_types<7, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef typename std::tuple_element<3, InputTuple>::type fourth_type;
        typedef typename std::tuple_element<4, InputTuple>::type fifth_type;
        typedef typename std::tuple_element<5, InputTuple>::type sixth_type;
        typedef typename std::tuple_element<6, InputTuple>::type seventh_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type,
                                                      fourth_type, fifth_type, sixth_type,
                                                      seventh_type> type;
    };


    template<typename InputTuple>
    struct input_types<8, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef typename std::tuple_element<3, InputTuple>::type fourth_type;
        typedef typename std::tuple_element<4, InputTuple>::type fifth_type;
        typedef typename std::tuple_element<5, InputTuple>::type sixth_type;
        typedef typename std::tuple_element<6, InputTuple>::type seventh_type;
        typedef typename std::tuple_element<7, InputTuple>::type eighth_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type,
                                                      fourth_type, fifth_type, sixth_type,
                                                      seventh_type, eighth_type> type;
    };


    template<typename InputTuple>
    struct input_types<9, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef typename std::tuple_element<3, InputTuple>::type fourth_type;
        typedef typename std::tuple_element<4, InputTuple>::type fifth_type;
        typedef typename std::tuple_element<5, InputTuple>::type sixth_type;
        typedef typename std::tuple_element<6, InputTuple>::type seventh_type;
        typedef typename std::tuple_element<7, InputTuple>::type eighth_type;
        typedef typename std::tuple_element<8, InputTuple>::type nineth_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type,
                                                      fourth_type, fifth_type, sixth_type,
                                                      seventh_type, eighth_type, nineth_type> type;
    };

    template<typename InputTuple>
    struct input_types<10, InputTuple> {
        typedef typename std::tuple_element<0, InputTuple>::type first_type;
        typedef typename std::tuple_element<1, InputTuple>::type second_type;
        typedef typename std::tuple_element<2, InputTuple>::type third_type;
        typedef typename std::tuple_element<3, InputTuple>::type fourth_type;
        typedef typename std::tuple_element<4, InputTuple>::type fifth_type;
        typedef typename std::tuple_element<5, InputTuple>::type sixth_type;
        typedef typename std::tuple_element<6, InputTuple>::type seventh_type;
        typedef typename std::tuple_element<7, InputTuple>::type eighth_type;
        typedef typename std::tuple_element<8, InputTuple>::type nineth_type;
        typedef typename std::tuple_element<9, InputTuple>::type tenth_type;
        typedef tagged_msg<size_t, first_type, second_type, third_type,
                                                      fourth_type, fifth_type, sixth_type,
                                                      seventh_type, eighth_type, nineth_type,
                                                      tenth_type> type;
    };

    // type generators
    template<typename OutputTuple>
    struct indexer_types : public input_types<std::tuple_size<OutputTuple>::value, OutputTuple> {
        static const int N = std::tuple_size<OutputTuple>::value;
        typedef typename input_types<N, OutputTuple>::type output_type;
        typedef typename wrap_tuple_elements<N,indexer_input_port,OutputTuple>::type input_ports_type;
        typedef indexer_node_FE<input_ports_type,output_type,OutputTuple> indexer_FE_type;
        typedef indexer_node_base<input_ports_type, output_type, OutputTuple> indexer_base_type;
    };

    template<class OutputTuple>
    class unfolded_indexer_node : public indexer_types<OutputTuple>::indexer_base_type {
    public:
        typedef typename indexer_types<OutputTuple>::input_ports_type input_ports_type;
        typedef OutputTuple tuple_types;
        typedef typename indexer_types<OutputTuple>::output_type output_type;
    private:
        typedef typename indexer_types<OutputTuple>::indexer_base_type base_type;
    public:
        unfolded_indexer_node(graph& g) : base_type(g) {}
        unfolded_indexer_node(const unfolded_indexer_node &other) : base_type(other) {}
    };


// in namespace tbb::flow::interfaceX (included in _flow_graph_node_impl.h)

//! Expandable buffer of items.  The possible operations are push, pop,
//* tests for empty and so forth.  No mutual exclusion is built in.
//* objects are constructed into and explicitly-destroyed.  get_my_item gives
// a read-only reference to the item in the buffer.  set_my_item may be called
// with either an empty or occupied slot.

template <typename T, typename A=cache_aligned_allocator<T> >
class item_buffer {
public:
    typedef T item_type;
    enum buffer_item_state { no_item=0, has_item=1, reserved_item=2 };
protected:
    typedef size_t size_type;
    typedef std::pair<item_type, buffer_item_state> aligned_space_item;
    typedef aligned_space<aligned_space_item> buffer_item_type;
    typedef typename allocator_traits<A>::template rebind_alloc<buffer_item_type> allocator_type;
    buffer_item_type *my_array;
    size_type my_array_size;
    static const size_type initial_buffer_size = 4;
    size_type my_head;
    size_type my_tail;

    bool buffer_empty() const { return my_head == my_tail; }

    aligned_space_item &item(size_type i) {
        __TBB_ASSERT(!(size_type(&(my_array[i&(my_array_size-1)].begin()->second))%alignment_of<buffer_item_state>::value), nullptr);
        __TBB_ASSERT(!(size_type(&(my_array[i&(my_array_size-1)].begin()->first))%alignment_of<item_type>::value), nullptr);
        return *my_array[i & (my_array_size - 1) ].begin();
    }

    const aligned_space_item &item(size_type i) const {
        __TBB_ASSERT(!(size_type(&(my_array[i&(my_array_size-1)].begin()->second))%alignment_of<buffer_item_state>::value), nullptr);
        __TBB_ASSERT(!(size_type(&(my_array[i&(my_array_size-1)].begin()->first))%alignment_of<item_type>::value), nullptr);
        return *my_array[i & (my_array_size-1)].begin();
    }

    bool my_item_valid(size_type i) const { return (i < my_tail) && (i >= my_head) && (item(i).second != no_item); }
#if TBB_USE_ASSERT
    bool my_item_reserved(size_type i) const { return item(i).second == reserved_item; }
#endif

    // object management in buffer
    const item_type &get_my_item(size_t i) const {
        __TBB_ASSERT(my_item_valid(i),"attempt to get invalid item");
        item_type* itm = const_cast<item_type*>(reinterpret_cast<const item_type*>(&item(i).first));
        return *itm;
    }

    // may be called with an empty slot or a slot that has already been constructed into.
    void set_my_item(size_t i, const item_type &o) {
        if(item(i).second != no_item) {
            destroy_item(i);
        }
        new(&(item(i).first)) item_type(o);
        item(i).second = has_item;
    }

    // destructively-fetch an object from the buffer
    void fetch_item(size_t i, item_type &o) {
        __TBB_ASSERT(my_item_valid(i), "Trying to fetch an empty slot");
        o = get_my_item(i);  // could have std::move assign semantics
        destroy_item(i);
    }

    // move an existing item from one slot to another.  The moved-to slot must be unoccupied,
    // the moved-from slot must exist and not be reserved.  The after, from will be empty,
    // to will be occupied but not reserved
    void move_item(size_t to, size_t from) {
        __TBB_ASSERT(!my_item_valid(to), "Trying to move to a non-empty slot");
        __TBB_ASSERT(my_item_valid(from), "Trying to move from an empty slot");
        set_my_item(to, get_my_item(from));   // could have std::move semantics
        destroy_item(from);

    }

    // put an item in an empty slot.  Return true if successful, else false
    bool place_item(size_t here, const item_type &me) {
#if !TBB_DEPRECATED_SEQUENCER_DUPLICATES
        if(my_item_valid(here)) return false;
#endif
        set_my_item(here, me);
        return true;
    }

    // could be implemented with std::move semantics
    void swap_items(size_t i, size_t j) {
        __TBB_ASSERT(my_item_valid(i) && my_item_valid(j), "attempt to swap invalid item(s)");
        item_type temp = get_my_item(i);
        set_my_item(i, get_my_item(j));
        set_my_item(j, temp);
    }

    void destroy_item(size_type i) {
        __TBB_ASSERT(my_item_valid(i), "destruction of invalid item");
        item(i).first.~item_type();
        item(i).second = no_item;
    }

    // returns the front element
    const item_type& front() const
    {
        __TBB_ASSERT(my_item_valid(my_head), "attempt to fetch head non-item");
        return get_my_item(my_head);
    }

    // returns  the back element
    const item_type& back() const
    {
        __TBB_ASSERT(my_item_valid(my_tail - 1), "attempt to fetch head non-item");
        return get_my_item(my_tail - 1);
    }

    // following methods are for reservation of the front of a buffer.
    void reserve_item(size_type i) { __TBB_ASSERT(my_item_valid(i) && !my_item_reserved(i), "item cannot be reserved"); item(i).second = reserved_item; }
    void release_item(size_type i) { __TBB_ASSERT(my_item_reserved(i), "item is not reserved"); item(i).second = has_item; }

    void destroy_front() { destroy_item(my_head); ++my_head; }
    void destroy_back() { destroy_item(my_tail-1); --my_tail; }

    // we have to be able to test against a new tail value without changing my_tail
    // grow_array doesn't work if we change my_tail when the old array is too small
    size_type size(size_t new_tail = 0) { return (new_tail ? new_tail : my_tail) - my_head; }
    size_type capacity() { return my_array_size; }
    // sequencer_node does not use this method, so we don't
    // need a version that passes in the new_tail value.
    bool buffer_full() { return size() >= capacity(); }

    //! Grows the internal array.
    void grow_my_array( size_t minimum_size ) {
        // test that we haven't made the structure inconsistent.
        __TBB_ASSERT(capacity() >= my_tail - my_head, "total items exceed capacity");
        size_type new_size = my_array_size ? 2*my_array_size : initial_buffer_size;
        while( new_size<minimum_size )
            new_size*=2;

        buffer_item_type* new_array = allocator_type().allocate(new_size);

        // initialize validity to "no"
        for( size_type i=0; i<new_size; ++i ) { new_array[i].begin()->second = no_item; }

        for( size_type i=my_head; i<my_tail; ++i) {
            if(my_item_valid(i)) {  // sequencer_node may have empty slots
                // placement-new copy-construct; could be std::move
                char *new_space = (char *)&(new_array[i&(new_size-1)].begin()->first);
                (void)new(new_space) item_type(get_my_item(i));
                new_array[i&(new_size-1)].begin()->second = item(i).second;
            }
        }

        clean_up_buffer(/*reset_pointers*/false);

        my_array = new_array;
        my_array_size = new_size;
    }

    bool push_back(item_type &v) {
        if(buffer_full()) {
            grow_my_array(size() + 1);
        }
        set_my_item(my_tail, v);
        ++my_tail;
        return true;
    }

    bool pop_back(item_type &v) {
        if (!my_item_valid(my_tail-1)) {
            return false;
        }
        v = this->back();
        destroy_back();
        return true;
    }

    bool pop_front(item_type &v) {
        if(!my_item_valid(my_head)) {
            return false;
        }
        v = this->front();
        destroy_front();
        return true;
    }

    // This is used both for reset and for grow_my_array.  In the case of grow_my_array
    // we want to retain the values of the head and tail.
    void clean_up_buffer(bool reset_pointers) {
        if (my_array) {
            for( size_type i=my_head; i<my_tail; ++i ) {
                if(my_item_valid(i))
                    destroy_item(i);
            }
            allocator_type().deallocate(my_array,my_array_size);
        }
        my_array = nullptr;
        if(reset_pointers) {
            my_head = my_tail = my_array_size = 0;
        }
    }

public:
    //! Constructor
    item_buffer( ) : my_array(nullptr), my_array_size(0),
                     my_head(0), my_tail(0) {
        grow_my_array(initial_buffer_size);
    }

    ~item_buffer() {
        clean_up_buffer(/*reset_pointers*/true);
    }

    void reset() { clean_up_buffer(/*reset_pointers*/true); grow_my_array(initial_buffer_size); }

};

//! item_buffer with reservable front-end.  NOTE: if reserving, do not
//* complete operation with pop_front(); use consume_front().
//* No synchronization built-in.
template<typename T, typename A=cache_aligned_allocator<T> >
class reservable_item_buffer : public item_buffer<T, A> {
protected:
    using item_buffer<T, A>::my_item_valid;
    using item_buffer<T, A>::my_head;

public:
    reservable_item_buffer() : item_buffer<T, A>(), my_reserved(false) {}
    void reset() {my_reserved = false; item_buffer<T,A>::reset(); }
protected:

    bool reserve_front(T &v) {
        if(my_reserved || !my_item_valid(this->my_head)) return false;
        my_reserved = true;
        // reserving the head
        v = this->front();
        this->reserve_item(this->my_head);
        return true;
    }

    void consume_front() {
        __TBB_ASSERT(my_reserved, "Attempt to consume a non-reserved item");
        this->destroy_front();
        my_reserved = false;
    }

    void release_front() {
        __TBB_ASSERT(my_reserved, "Attempt to release a non-reserved item");
        this->release_item(this->my_head);
        my_reserved = false;
    }

    bool my_reserved;
};

template< typename T, typename A >
class function_input_queue : public item_buffer<T,A> {
public:
    bool empty() const {
        return this->buffer_empty();
    }

    const T& front() const {
        return this->item_buffer<T, A>::front();
    }

    void pop() {
        this->destroy_front();
    }

    bool push( T& t ) {
        return this->push_back( t );
    }
};

//! Input and scheduling for a function node that takes a type Input as input
//  The only up-ref is apply_body_impl, which should implement the function
//  call and any handling of the result.
template< typename Input, typename Policy, typename A, typename ImplType >
class function_input_base : public receiver<Input>, no_assign {
    enum op_type {reg_pred, rem_pred, try_fwd, tryput_bypass, app_body_bypass, occupy_concurrency
    };
    typedef function_input_base<Input, Policy, A, ImplType> class_type;

public:

    //! The input type of this receiver
    typedef Input input_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef predecessor_cache<input_type, null_mutex > predecessor_cache_type;
    typedef function_input_queue<input_type, A> input_queue_type;
    typedef typename allocator_traits<A>::template rebind_alloc<input_queue_type> allocator_type;
    static_assert(!has_policy<queueing, Policy>::value || !has_policy<rejecting, Policy>::value, "");

    //! Constructor for function_input_base
    function_input_base( graph &g, size_t max_concurrency, node_priority_t a_priority, bool is_no_throw )
        : my_graph_ref(g), my_max_concurrency(max_concurrency)
        , my_concurrency(0), my_priority(a_priority), my_is_no_throw(is_no_throw)
        , my_queue(!has_policy<rejecting, Policy>::value ? new input_queue_type() : nullptr)
        , my_predecessors(this)
        , forwarder_busy(false)
    {
        my_aggregator.initialize_handler(handler_type(this));
    }

    //! Copy constructor
    function_input_base( const function_input_base& src )
        : function_input_base(src.my_graph_ref, src.my_max_concurrency, src.my_priority, src.my_is_no_throw) {}

    //! Destructor
    // The queue is allocated by the constructor for {multi}function_node.
    // TODO: pass the graph_buffer_policy to the base so it can allocate the queue instead.
    // This would be an interface-breaking change.
    virtual ~function_input_base() {
        delete my_queue;
        my_queue = nullptr;
    }

    graph_task* try_put_task( const input_type& t) override {
        if ( my_is_no_throw )
            return try_put_task_impl(t, has_policy<lightweight, Policy>());
        else
            return try_put_task_impl(t, std::false_type());
    }

    //! Adds src to the list of cached predecessors.
    bool register_predecessor( predecessor_type &src ) override {
        operation_type op_data(reg_pred);
        op_data.r = &src;
        my_aggregator.execute(&op_data);
        return true;
    }

    //! Removes src from the list of cached predecessors.
    bool remove_predecessor( predecessor_type &src ) override {
        operation_type op_data(rem_pred);
        op_data.r = &src;
        my_aggregator.execute(&op_data);
        return true;
    }

protected:

    void reset_function_input_base( reset_flags f) {
        my_concurrency = 0;
        if(my_queue) {
            my_queue->reset();
        }
        reset_receiver(f);
        forwarder_busy = false;
    }

    graph& my_graph_ref;
    const size_t my_max_concurrency;
    size_t my_concurrency;
    node_priority_t my_priority;
    const bool my_is_no_throw;
    input_queue_type *my_queue;
    predecessor_cache<input_type, null_mutex > my_predecessors;

    void reset_receiver( reset_flags f) {
        if( f & rf_clear_edges) my_predecessors.clear();
        else
            my_predecessors.reset();
        __TBB_ASSERT(!(f & rf_clear_edges) || my_predecessors.empty(), "function_input_base reset failed");
    }

    graph& graph_reference() const override {
        return my_graph_ref;
    }

    graph_task* try_get_postponed_task(const input_type& i) {
        operation_type op_data(i, app_body_bypass);  // tries to pop an item or get_item
        my_aggregator.execute(&op_data);
        return op_data.bypass_t;
    }

private:

    friend class apply_body_task_bypass< class_type, input_type >;
    friend class forward_task_bypass< class_type >;

    class operation_type : public aggregated_operation< operation_type > {
    public:
        char type;
        union {
            input_type *elem;
            predecessor_type *r;
        };
        graph_task* bypass_t;
        operation_type(const input_type& e, op_type t) :
            type(char(t)), elem(const_cast<input_type*>(&e)), bypass_t(nullptr) {}
        operation_type(op_type t) : type(char(t)), r(nullptr), bypass_t(nullptr) {}
    };

    bool forwarder_busy;
    typedef aggregating_functor<class_type, operation_type> handler_type;
    friend class aggregating_functor<class_type, operation_type>;
    aggregator< handler_type, operation_type > my_aggregator;

    graph_task* perform_queued_requests() {
        graph_task* new_task = nullptr;
        if(my_queue) {
            if(!my_queue->empty()) {
                ++my_concurrency;
                new_task = create_body_task(my_queue->front());

                my_queue->pop();
            }
        }
        else {
            input_type i;
            if(my_predecessors.get_item(i)) {
                ++my_concurrency;
                new_task = create_body_task(i);
            }
        }
        return new_task;
    }
    void handle_operations(operation_type *op_list) {
        operation_type* tmp;
        while (op_list) {
            tmp = op_list;
            op_list = op_list->next;
            switch (tmp->type) {
            case reg_pred:
                my_predecessors.add(*(tmp->r));
                tmp->status.store(SUCCEEDED, std::memory_order_release);
                if (!forwarder_busy) {
                    forwarder_busy = true;
                    spawn_forward_task();
                }
                break;
            case rem_pred:
                my_predecessors.remove(*(tmp->r));
                tmp->status.store(SUCCEEDED, std::memory_order_release);
                break;
            case app_body_bypass: {
                tmp->bypass_t = nullptr;
                __TBB_ASSERT(my_max_concurrency != 0, nullptr);
                --my_concurrency;
                if(my_concurrency<my_max_concurrency)
                    tmp->bypass_t = perform_queued_requests();
                tmp->status.store(SUCCEEDED, std::memory_order_release);
            }
                break;
            case tryput_bypass: internal_try_put_task(tmp);  break;
            case try_fwd: internal_forward(tmp);  break;
            case occupy_concurrency:
                if (my_concurrency < my_max_concurrency) {
                    ++my_concurrency;
                    tmp->status.store(SUCCEEDED, std::memory_order_release);
                } else {
                    tmp->status.store(FAILED, std::memory_order_release);
                }
                break;
            }
        }
    }

    //! Put to the node, but return the task instead of enqueueing it
    void internal_try_put_task(operation_type *op) {
        __TBB_ASSERT(my_max_concurrency != 0, nullptr);
        if (my_concurrency < my_max_concurrency) {
            ++my_concurrency;
            graph_task * new_task = create_body_task(*(op->elem));
            op->bypass_t = new_task;
            op->status.store(SUCCEEDED, std::memory_order_release);
        } else if ( my_queue && my_queue->push(*(op->elem)) ) {
            op->bypass_t = SUCCESSFULLY_ENQUEUED;
            op->status.store(SUCCEEDED, std::memory_order_release);
        } else {
            op->bypass_t = nullptr;
            op->status.store(FAILED, std::memory_order_release);
        }
    }

    //! Creates tasks for postponed messages if available and if concurrency allows
    void internal_forward(operation_type *op) {
        op->bypass_t = nullptr;
        if (my_concurrency < my_max_concurrency)
            op->bypass_t = perform_queued_requests();
        if(op->bypass_t)
            op->status.store(SUCCEEDED, std::memory_order_release);
        else {
            forwarder_busy = false;
            op->status.store(FAILED, std::memory_order_release);
        }
    }

    graph_task* internal_try_put_bypass( const input_type& t ) {
        operation_type op_data(t, tryput_bypass);
        my_aggregator.execute(&op_data);
        if( op_data.status == SUCCEEDED ) {
            return op_data.bypass_t;
        }
        return nullptr;
    }

    graph_task* try_put_task_impl( const input_type& t, /*lightweight=*/std::true_type ) {
        if( my_max_concurrency == 0 ) {
            return apply_body_bypass(t);
        } else {
            operation_type check_op(t, occupy_concurrency);
            my_aggregator.execute(&check_op);
            if( check_op.status == SUCCEEDED ) {
                return apply_body_bypass(t);
            }
            return internal_try_put_bypass(t);
        }
    }

    graph_task* try_put_task_impl( const input_type& t, /*lightweight=*/std::false_type ) {
        if( my_max_concurrency == 0 ) {
            return create_body_task(t);
        } else {
            return internal_try_put_bypass(t);
        }
    }

    //! Applies the body to the provided input
    //  then decides if more work is available
    graph_task* apply_body_bypass( const input_type &i ) {
        return static_cast<ImplType *>(this)->apply_body_impl_bypass(i);
    }

    //! allocates a task to apply a body
    graph_task* create_body_task( const input_type &input ) {
        if (!is_graph_active(my_graph_ref)) {
            return nullptr;
        }
        // TODO revamp: extract helper for common graph task allocation part
        small_object_allocator allocator{};
        typedef apply_body_task_bypass<class_type, input_type> task_type;
        graph_task* t = allocator.new_object<task_type>( my_graph_ref, allocator, *this, input, my_priority );
        graph_reference().reserve_wait();
        return t;
    }

    //! This is executed by an enqueued task, the "forwarder"
    graph_task* forward_task() {
        operation_type op_data(try_fwd);
        graph_task* rval = nullptr;
        do {
            op_data.status = WAIT;
            my_aggregator.execute(&op_data);
            if(op_data.status == SUCCEEDED) {
                graph_task* ttask = op_data.bypass_t;
                __TBB_ASSERT( ttask && ttask != SUCCESSFULLY_ENQUEUED, nullptr);
                rval = combine_tasks(my_graph_ref, rval, ttask);
            }
        } while (op_data.status == SUCCEEDED);
        return rval;
    }

    inline graph_task* create_forward_task() {
        if (!is_graph_active(my_graph_ref)) {
            return nullptr;
        }
        small_object_allocator allocator{};
        typedef forward_task_bypass<class_type> task_type;
        graph_task* t = allocator.new_object<task_type>( graph_reference(), allocator, *this, my_priority );
        graph_reference().reserve_wait();
        return t;
    }

    //! Spawns a task that calls forward()
    inline void spawn_forward_task() {
        graph_task* tp = create_forward_task();
        if(tp) {
            spawn_in_graph_arena(graph_reference(), *tp);
        }
    }

    node_priority_t priority() const override { return my_priority; }
};  // function_input_base

//! Implements methods for a function node that takes a type Input as input and sends
//  a type Output to its successors.
template< typename Input, typename Output, typename Policy, typename A>
class function_input : public function_input_base<Input, Policy, A, function_input<Input,Output,Policy,A> > {
public:
    typedef Input input_type;
    typedef Output output_type;
    typedef function_body<input_type, output_type> function_body_type;
    typedef function_input<Input, Output, Policy,A> my_class;
    typedef function_input_base<Input, Policy, A, my_class> base_type;
    typedef function_input_queue<input_type, A> input_queue_type;

    // constructor
    template<typename Body>
    function_input(
        graph &g, size_t max_concurrency, Body& body, node_priority_t a_priority )
      : base_type(g, max_concurrency, a_priority, noexcept(tbb::detail::invoke(body, input_type())))
      , my_body( new function_body_leaf< input_type, output_type, Body>(body) )
      , my_init_body( new function_body_leaf< input_type, output_type, Body>(body) ) {
    }

    //! Copy constructor
    function_input( const function_input& src ) :
        base_type(src),
        my_body( src.my_init_body->clone() ),
        my_init_body(src.my_init_body->clone() ) {
    }
#if __INTEL_COMPILER <= 2021
    // Suppress superfluous diagnostic about virtual keyword absence in a destructor of an inherited
    // class while the parent class has the virtual keyword for the destrocutor.
    virtual
#endif
    ~function_input() {
        delete my_body;
        delete my_init_body;
    }

    template< typename Body >
    Body copy_function_object() {
        function_body_type &body_ref = *this->my_body;
        return dynamic_cast< function_body_leaf<input_type, output_type, Body> & >(body_ref).get_body();
    }

    output_type apply_body_impl( const input_type& i) {
        // There is an extra copied needed to capture the
        // body execution without the try_put
        fgt_begin_body( my_body );
        output_type v = tbb::detail::invoke(*my_body, i);
        fgt_end_body( my_body );
        return v;
    }

    //TODO: consider moving into the base class
    graph_task* apply_body_impl_bypass( const input_type &i) {
        output_type v = apply_body_impl(i);
        graph_task* postponed_task = nullptr;
        if( base_type::my_max_concurrency != 0 ) {
            postponed_task = base_type::try_get_postponed_task(i);
            __TBB_ASSERT( !postponed_task || postponed_task != SUCCESSFULLY_ENQUEUED, nullptr);
        }
        if( postponed_task ) {
            // make the task available for other workers since we do not know successors'
            // execution policy
            spawn_in_graph_arena(base_type::graph_reference(), *postponed_task);
        }
        graph_task* successor_task = successors().try_put_task(v);
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning (push)
#pragma warning (disable: 4127)  /* suppress conditional expression is constant */
#endif
        if(has_policy<lightweight, Policy>::value) {
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning (pop)
#endif
            if(!successor_task) {
                // Return confirmative status since current
                // node's body has been executed anyway
                successor_task = SUCCESSFULLY_ENQUEUED;
            }
        }
        return successor_task;
    }

protected:

    void reset_function_input(reset_flags f) {
        base_type::reset_function_input_base(f);
        if(f & rf_reset_bodies) {
            function_body_type *tmp = my_init_body->clone();
            delete my_body;
            my_body = tmp;
        }
    }

    function_body_type *my_body;
    function_body_type *my_init_body;
    virtual broadcast_cache<output_type > &successors() = 0;

};  // function_input


// helper templates to clear the successor edges of the output ports of an multifunction_node
template<int N> struct clear_element {
    template<typename P> static void clear_this(P &p) {
        (void)std::get<N-1>(p).successors().clear();
        clear_element<N-1>::clear_this(p);
    }
#if TBB_USE_ASSERT
    template<typename P> static bool this_empty(P &p) {
        if(std::get<N-1>(p).successors().empty())
            return clear_element<N-1>::this_empty(p);
        return false;
    }
#endif
};

template<> struct clear_element<1> {
    template<typename P> static void clear_this(P &p) {
        (void)std::get<0>(p).successors().clear();
    }
#if TBB_USE_ASSERT
    template<typename P> static bool this_empty(P &p) {
        return std::get<0>(p).successors().empty();
    }
#endif
};

template <typename OutputTuple>
struct init_output_ports {
    template <typename... Args>
    static OutputTuple call(graph& g, const std::tuple<Args...>&) {
        return OutputTuple(Args(g)...);
    }
}; // struct init_output_ports

//! Implements methods for a function node that takes a type Input as input
//  and has a tuple of output ports specified.
template< typename Input, typename OutputPortSet, typename Policy, typename A>
class multifunction_input : public function_input_base<Input, Policy, A, multifunction_input<Input,OutputPortSet,Policy,A> > {
public:
    static const int N = std::tuple_size<OutputPortSet>::value;
    typedef Input input_type;
    typedef OutputPortSet output_ports_type;
    typedef multifunction_body<input_type, output_ports_type> multifunction_body_type;
    typedef multifunction_input<Input, OutputPortSet, Policy, A> my_class;
    typedef function_input_base<Input, Policy, A, my_class> base_type;
    typedef function_input_queue<input_type, A> input_queue_type;

    // constructor
    template<typename Body>
    multifunction_input(graph &g, size_t max_concurrency,Body& body, node_priority_t a_priority )
      : base_type(g, max_concurrency, a_priority, noexcept(tbb::detail::invoke(body, input_type(), my_output_ports)))
      , my_body( new multifunction_body_leaf<input_type, output_ports_type, Body>(body) )
      , my_init_body( new multifunction_body_leaf<input_type, output_ports_type, Body>(body) )
      , my_output_ports(init_output_ports<output_ports_type>::call(g, my_output_ports)){
    }

    //! Copy constructor
    multifunction_input( const multifunction_input& src ) :
        base_type(src),
        my_body( src.my_init_body->clone() ),
        my_init_body(src.my_init_body->clone() ),
        my_output_ports( init_output_ports<output_ports_type>::call(src.my_graph_ref, my_output_ports) ) {
    }

    ~multifunction_input() {
        delete my_body;
        delete my_init_body;
    }

    template< typename Body >
    Body copy_function_object() {
        multifunction_body_type &body_ref = *this->my_body;
        return *static_cast<Body*>(dynamic_cast< multifunction_body_leaf<input_type, output_ports_type, Body> & >(body_ref).get_body_ptr());
    }

    // for multifunction nodes we do not have a single successor as such.  So we just tell
    // the task we were successful.
    //TODO: consider moving common parts with implementation in function_input into separate function
    graph_task* apply_body_impl_bypass( const input_type &i ) {
        fgt_begin_body( my_body );
        (*my_body)(i, my_output_ports);
        fgt_end_body( my_body );
        graph_task* ttask = nullptr;
        if(base_type::my_max_concurrency != 0) {
            ttask = base_type::try_get_postponed_task(i);
        }
        return ttask ? ttask : SUCCESSFULLY_ENQUEUED;
    }

    output_ports_type &output_ports(){ return my_output_ports; }

protected:

    void reset(reset_flags f) {
        base_type::reset_function_input_base(f);
        if(f & rf_clear_edges)clear_element<N>::clear_this(my_output_ports);
        if(f & rf_reset_bodies) {
            multifunction_body_type* tmp = my_init_body->clone();
            delete my_body;
            my_body = tmp;
        }
        __TBB_ASSERT(!(f & rf_clear_edges) || clear_element<N>::this_empty(my_output_ports), "multifunction_node reset failed");
    }

    multifunction_body_type *my_body;
    multifunction_body_type *my_init_body;
    output_ports_type my_output_ports;

};  // multifunction_input

// template to refer to an output port of a multifunction_node
template<size_t N, typename MOP>
typename std::tuple_element<N, typename MOP::output_ports_type>::type &output_port(MOP &op) {
    return std::get<N>(op.output_ports());
}

inline void check_task_and_spawn(graph& g, graph_task* t) {
    if (t && t != SUCCESSFULLY_ENQUEUED) {
        spawn_in_graph_arena(g, *t);
    }
}

// helper structs for split_node
template<int N>
struct emit_element {
    template<typename T, typename P>
    static graph_task* emit_this(graph& g, const T &t, P &p) {
        // TODO: consider to collect all the tasks in task_list and spawn them all at once
        graph_task* last_task = std::get<N-1>(p).try_put_task(std::get<N-1>(t));
        check_task_and_spawn(g, last_task);
        return emit_element<N-1>::emit_this(g,t,p);
    }
};

template<>
struct emit_element<1> {
    template<typename T, typename P>
    static graph_task* emit_this(graph& g, const T &t, P &p) {
        graph_task* last_task = std::get<0>(p).try_put_task(std::get<0>(t));
        check_task_and_spawn(g, last_task);
        return SUCCESSFULLY_ENQUEUED;
    }
};

//! Implements methods for an executable node that takes continue_msg as input
template< typename Output, typename Policy>
class continue_input : public continue_receiver {
public:

    //! The input type of this receiver
    typedef continue_msg input_type;

    //! The output type of this receiver
    typedef Output output_type;
    typedef function_body<input_type, output_type> function_body_type;
    typedef continue_input<output_type, Policy> class_type;

    template< typename Body >
    continue_input( graph &g, Body& body, node_priority_t a_priority )
        : continue_receiver(/*number_of_predecessors=*/0, a_priority)
        , my_graph_ref(g)
        , my_body( new function_body_leaf< input_type, output_type, Body>(body) )
        , my_init_body( new function_body_leaf< input_type, output_type, Body>(body) )
    { }

    template< typename Body >
    continue_input( graph &g, int number_of_predecessors,
                    Body& body, node_priority_t a_priority )
      : continue_receiver( number_of_predecessors, a_priority )
      , my_graph_ref(g)
      , my_body( new function_body_leaf< input_type, output_type, Body>(body) )
      , my_init_body( new function_body_leaf< input_type, output_type, Body>(body) )
    { }

    continue_input( const continue_input& src ) : continue_receiver(src),
                                                  my_graph_ref(src.my_graph_ref),
                                                  my_body( src.my_init_body->clone() ),
                                                  my_init_body( src.my_init_body->clone() ) {}

    ~continue_input() {
        delete my_body;
        delete my_init_body;
    }

    template< typename Body >
    Body copy_function_object() {
        function_body_type &body_ref = *my_body;
        return dynamic_cast< function_body_leaf<input_type, output_type, Body> & >(body_ref).get_body();
    }

    void reset_receiver( reset_flags f) override {
        continue_receiver::reset_receiver(f);
        if(f & rf_reset_bodies) {
            function_body_type *tmp = my_init_body->clone();
            delete my_body;
            my_body = tmp;
        }
    }

protected:

    graph& my_graph_ref;
    function_body_type *my_body;
    function_body_type *my_init_body;

    virtual broadcast_cache<output_type > &successors() = 0;

    friend class apply_body_task_bypass< class_type, continue_msg >;

    //! Applies the body to the provided input
    graph_task* apply_body_bypass( input_type ) {
        // There is an extra copied needed to capture the
        // body execution without the try_put
        fgt_begin_body( my_body );
        output_type v = (*my_body)( continue_msg() );
        fgt_end_body( my_body );
        return successors().try_put_task( v );
    }

    graph_task* execute() override {
        if(!is_graph_active(my_graph_ref)) {
            return nullptr;
        }
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning (push)
#pragma warning (disable: 4127)  /* suppress conditional expression is constant */
#endif
        if(has_policy<lightweight, Policy>::value) {
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning (pop)
#endif
            return apply_body_bypass( continue_msg() );
        }
        else {
            small_object_allocator allocator{};
            typedef apply_body_task_bypass<class_type, continue_msg> task_type;
            graph_task* t = allocator.new_object<task_type>( graph_reference(), allocator, *this, continue_msg(), my_priority );
            graph_reference().reserve_wait();
            return t;
        }
    }

    graph& graph_reference() const override {
        return my_graph_ref;
    }
};  // continue_input

//! Implements methods for both executable and function nodes that puts Output to its successors
template< typename Output >
class function_output : public sender<Output> {
public:

    template<int N> friend struct clear_element;
    typedef Output output_type;
    typedef typename sender<output_type>::successor_type successor_type;
    typedef broadcast_cache<output_type> broadcast_cache_type;

    function_output(graph& g) : my_successors(this), my_graph_ref(g) {}
    function_output(const function_output& other) = delete;

    //! Adds a new successor to this node
    bool register_successor( successor_type &r ) override {
        successors().register_successor( r );
        return true;
    }

    //! Removes a successor from this node
    bool remove_successor( successor_type &r ) override {
        successors().remove_successor( r );
        return true;
    }

    broadcast_cache_type &successors() { return my_successors; }

    graph& graph_reference() const { return my_graph_ref; }
protected:
    broadcast_cache_type my_successors;
    graph& my_graph_ref;
};  // function_output

template< typename Output >
class multifunction_output : public function_output<Output> {
public:
    typedef Output output_type;
    typedef function_output<output_type> base_type;
    using base_type::my_successors;

    multifunction_output(graph& g) : base_type(g) {}
    multifunction_output(const multifunction_output& other) : base_type(other.my_graph_ref) {}

    bool try_put(const output_type &i) {
        graph_task *res = try_put_task(i);
        if( !res ) return false;
        if( res != SUCCESSFULLY_ENQUEUED ) {
            // wrapping in task_arena::execute() is not needed since the method is called from
            // inside task::execute()
            spawn_in_graph_arena(graph_reference(), *res);
        }
        return true;
    }

    using base_type::graph_reference;

protected:

    graph_task* try_put_task(const output_type &i) {
        return my_successors.try_put_task(i);
    }

    template <int N> friend struct emit_element;

};  // multifunction_output

//composite_node
template<typename CompositeType>
void add_nodes_impl(CompositeType*, bool) {}

template< typename CompositeType, typename NodeType1, typename... NodeTypes >
void add_nodes_impl(CompositeType *c_node, bool visible, const NodeType1& n1, const NodeTypes&... n) {
    void *addr = const_cast<NodeType1 *>(&n1);

    fgt_alias_port(c_node, addr, visible);
    add_nodes_impl(c_node, visible, n...);
}


//! An executable node that acts as a source, i.e. it has no predecessors

template < typename Output >
    __TBB_requires(std::copyable<Output>)
class input_node : public graph_node, public sender< Output > {
public:
    //! The type of the output message, which is complete
    typedef Output output_type;

    //! The type of successors of this node
    typedef typename sender<output_type>::successor_type successor_type;

    // Input node has no input type
    typedef null_type input_type;

    //! Constructor for a node with a successor
    template< typename Body >
        __TBB_requires(input_node_body<Body, Output>)
     __TBB_NOINLINE_SYM input_node( graph &g, Body body )
         : graph_node(g), my_active(false)
         , my_body( new input_body_leaf< output_type, Body>(body) )
         , my_init_body( new input_body_leaf< output_type, Body>(body) )
         , my_successors(this), my_reserved(false), my_has_cached_item(false)
    {
        fgt_node_with_body(CODEPTR(), FLOW_INPUT_NODE, &this->my_graph,
                           static_cast<sender<output_type> *>(this), this->my_body);
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename Body, typename... Successors>
        __TBB_requires(input_node_body<Body, Output>)
    input_node( const node_set<order::preceding, Successors...>& successors, Body body )
        : input_node(successors.graph_reference(), body)
    {
        make_edges(*this, successors);
    }
#endif

    //! Copy constructor
    __TBB_NOINLINE_SYM input_node( const input_node& src )
        : graph_node(src.my_graph), sender<Output>()
        , my_active(false)
        , my_body(src.my_init_body->clone()), my_init_body(src.my_init_body->clone())
        , my_successors(this), my_reserved(false), my_has_cached_item(false)
    {
        fgt_node_with_body(CODEPTR(), FLOW_INPUT_NODE, &this->my_graph,
                           static_cast<sender<output_type> *>(this), this->my_body);
    }

    //! The destructor
    ~input_node() { delete my_body; delete my_init_body; }

    //! Add a new successor to this node
    bool register_successor( successor_type &r ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        my_successors.register_successor(r);
        if ( my_active )
            spawn_put();
        return true;
    }

    //! Removes a successor from this node
    bool remove_successor( successor_type &r ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        my_successors.remove_successor(r);
        return true;
    }

    //! Request an item from the node
    bool try_get( output_type &v ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        if ( my_reserved )
            return false;

        if ( my_has_cached_item ) {
            v = my_cached_item;
            my_has_cached_item = false;
            return true;
        }
        // we've been asked to provide an item, but we have none.  enqueue a task to
        // provide one.
        if ( my_active )
            spawn_put();
        return false;
    }

    //! Reserves an item.
    bool try_reserve( output_type &v ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        if ( my_reserved ) {
            return false;
        }

        if ( my_has_cached_item ) {
            v = my_cached_item;
            my_reserved = true;
            return true;
        } else {
            return false;
        }
    }

    //! Release a reserved item.
    /** true = item has been released and so remains in sender, dest must request or reserve future items */
    bool try_release( ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        __TBB_ASSERT( my_reserved && my_has_cached_item, "releasing non-existent reservation" );
        my_reserved = false;
        if(!my_successors.empty())
            spawn_put();
        return true;
    }

    //! Consumes a reserved item
    bool try_consume( ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        __TBB_ASSERT( my_reserved && my_has_cached_item, "consuming non-existent reservation" );
        my_reserved = false;
        my_has_cached_item = false;
        if ( !my_successors.empty() ) {
            spawn_put();
        }
        return true;
    }

    //! Activates a node that was created in the inactive state
    void activate() {
        spin_mutex::scoped_lock lock(my_mutex);
        my_active = true;
        if (!my_successors.empty())
            spawn_put();
    }

    template<typename Body>
    Body copy_function_object() {
        input_body<output_type> &body_ref = *this->my_body;
        return dynamic_cast< input_body_leaf<output_type, Body> & >(body_ref).get_body();
    }

protected:

    //! resets the input_node to its initial state
    void reset_node( reset_flags f) override {
        my_active = false;
        my_reserved = false;
        my_has_cached_item = false;

        if(f & rf_clear_edges) my_successors.clear();
        if(f & rf_reset_bodies) {
            input_body<output_type> *tmp = my_init_body->clone();
            delete my_body;
            my_body = tmp;
        }
    }

private:
    spin_mutex my_mutex;
    bool my_active;
    input_body<output_type> *my_body;
    input_body<output_type> *my_init_body;
    broadcast_cache< output_type > my_successors;
    bool my_reserved;
    bool my_has_cached_item;
    output_type my_cached_item;

    // used by apply_body_bypass, can invoke body of node.
    bool try_reserve_apply_body(output_type &v) {
        spin_mutex::scoped_lock lock(my_mutex);
        if ( my_reserved ) {
            return false;
        }
        if ( !my_has_cached_item ) {
            flow_control control;

            fgt_begin_body( my_body );

            my_cached_item = (*my_body)(control);
            my_has_cached_item = !control.is_pipeline_stopped;

            fgt_end_body( my_body );
        }
        if ( my_has_cached_item ) {
            v = my_cached_item;
            my_reserved = true;
            return true;
        } else {
            return false;
        }
    }

    graph_task* create_put_task() {
        small_object_allocator allocator{};
        typedef input_node_task_bypass< input_node<output_type> > task_type;
        graph_task* t = allocator.new_object<task_type>(my_graph, allocator, *this);
        my_graph.reserve_wait();
        return t;
    }

    //! Spawns a task that applies the body
    void spawn_put( ) {
        if(is_graph_active(this->my_graph)) {
            spawn_in_graph_arena(this->my_graph, *create_put_task());
        }
    }

    friend class input_node_task_bypass< input_node<output_type> >;
    //! Applies the body.  Returning SUCCESSFULLY_ENQUEUED okay; forward_task_bypass will handle it.
    graph_task* apply_body_bypass( ) {
        output_type v;
        if ( !try_reserve_apply_body(v) )
            return nullptr;

        graph_task *last_task = my_successors.try_put_task(v);
        if ( last_task )
            try_consume();
        else
            try_release();
        return last_task;
    }
};  // class input_node

//! Implements a function node that supports Input -> Output
template<typename Input, typename Output = continue_msg, typename Policy = queueing>
    __TBB_requires(std::default_initializable<Input> &&
                   std::copy_constructible<Input> &&
                   std::copy_constructible<Output>)
class function_node
    : public graph_node
    , public function_input< Input, Output, Policy, cache_aligned_allocator<Input> >
    , public function_output<Output>
{
    typedef cache_aligned_allocator<Input> internals_allocator;

public:
    typedef Input input_type;
    typedef Output output_type;
    typedef function_input<input_type,output_type,Policy,internals_allocator> input_impl_type;
    typedef function_input_queue<input_type, internals_allocator> input_queue_type;
    typedef function_output<output_type> fOutput_type;
    typedef typename input_impl_type::predecessor_type predecessor_type;
    typedef typename fOutput_type::successor_type successor_type;

    using input_impl_type::my_predecessors;

    //! Constructor
    // input_queue_type is allocated here, but destroyed in the function_input_base.
    // TODO: pass the graph_buffer_policy to the function_input_base so it can all
    // be done in one place.  This would be an interface-breaking change.
    template< typename Body >
        __TBB_requires(function_node_body<Body, Input, Output>)
     __TBB_NOINLINE_SYM function_node( graph &g, size_t concurrency,
                   Body body, Policy = Policy(), node_priority_t a_priority = no_priority )
        : graph_node(g), input_impl_type(g, concurrency, body, a_priority),
          fOutput_type(g) {
        fgt_node_with_body( CODEPTR(), FLOW_FUNCTION_NODE, &this->my_graph,
                static_cast<receiver<input_type> *>(this), static_cast<sender<output_type> *>(this), this->my_body );
    }

    template <typename Body>
        __TBB_requires(function_node_body<Body, Input, Output>)
    function_node( graph& g, size_t concurrency, Body body, node_priority_t a_priority )
        : function_node(g, concurrency, body, Policy(), a_priority) {}

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename Body, typename... Args>
        __TBB_requires(function_node_body<Body, Input, Output>)
    function_node( const node_set<Args...>& nodes, size_t concurrency, Body body,
                   Policy p = Policy(), node_priority_t a_priority = no_priority )
        : function_node(nodes.graph_reference(), concurrency, body, p, a_priority) {
        make_edges_in_order(nodes, *this);
    }

    template <typename Body, typename... Args>
        __TBB_requires(function_node_body<Body, Input, Output>)
    function_node( const node_set<Args...>& nodes, size_t concurrency, Body body, node_priority_t a_priority )
        : function_node(nodes, concurrency, body, Policy(), a_priority) {}
#endif // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

    //! Copy constructor
    __TBB_NOINLINE_SYM function_node( const function_node& src ) :
        graph_node(src.my_graph),
        input_impl_type(src),
        fOutput_type(src.my_graph) {
        fgt_node_with_body( CODEPTR(), FLOW_FUNCTION_NODE, &this->my_graph,
                static_cast<receiver<input_type> *>(this), static_cast<sender<output_type> *>(this), this->my_body );
    }

protected:
    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    using input_impl_type::try_put_task;

    broadcast_cache<output_type> &successors () override { return fOutput_type::my_successors; }

    void reset_node(reset_flags f) override {
        input_impl_type::reset_function_input(f);
        // TODO: use clear() instead.
        if(f & rf_clear_edges) {
            successors().clear();
            my_predecessors.clear();
        }
        __TBB_ASSERT(!(f & rf_clear_edges) || successors().empty(), "function_node successors not empty");
        __TBB_ASSERT(this->my_predecessors.empty(), "function_node predecessors not empty");
    }

};  // class function_node

//! implements a function node that supports Input -> (set of outputs)
// Output is a tuple of output types.
template<typename Input, typename Output, typename Policy = queueing>
    __TBB_requires(std::default_initializable<Input> &&
                   std::copy_constructible<Input>)
class multifunction_node :
    public graph_node,
    public multifunction_input
    <
        Input,
        typename wrap_tuple_elements<
            std::tuple_size<Output>::value,  // #elements in tuple
            multifunction_output,  // wrap this around each element
            Output // the tuple providing the types
        >::type,
        Policy,
        cache_aligned_allocator<Input>
    >
{
    typedef cache_aligned_allocator<Input> internals_allocator;

protected:
    static const int N = std::tuple_size<Output>::value;
public:
    typedef Input input_type;
    typedef null_type output_type;
    typedef typename wrap_tuple_elements<N,multifunction_output, Output>::type output_ports_type;
    typedef multifunction_input<
        input_type, output_ports_type, Policy, internals_allocator> input_impl_type;
    typedef function_input_queue<input_type, internals_allocator> input_queue_type;
private:
    using input_impl_type::my_predecessors;
public:
    template<typename Body>
        __TBB_requires(multifunction_node_body<Body, Input, output_ports_type>)
    __TBB_NOINLINE_SYM multifunction_node(
        graph &g, size_t concurrency,
        Body body, Policy = Policy(), node_priority_t a_priority = no_priority
    ) : graph_node(g), input_impl_type(g, concurrency, body, a_priority) {
        fgt_multioutput_node_with_body<N>(
            CODEPTR(), FLOW_MULTIFUNCTION_NODE,
            &this->my_graph, static_cast<receiver<input_type> *>(this),
            this->output_ports(), this->my_body
        );
    }

    template <typename Body>
        __TBB_requires(multifunction_node_body<Body, Input, output_ports_type>)
    __TBB_NOINLINE_SYM multifunction_node(graph& g, size_t concurrency, Body body, node_priority_t a_priority)
        : multifunction_node(g, concurrency, body, Policy(), a_priority) {}

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename Body, typename... Args>
        __TBB_requires(multifunction_node_body<Body, Input, output_ports_type>)
    __TBB_NOINLINE_SYM multifunction_node(const node_set<Args...>& nodes, size_t concurrency, Body body,
                       Policy p = Policy(), node_priority_t a_priority = no_priority)
        : multifunction_node(nodes.graph_reference(), concurrency, body, p, a_priority) {
        make_edges_in_order(nodes, *this);
    }

    template <typename Body, typename... Args>
        __TBB_requires(multifunction_node_body<Body, Input, output_ports_type>)
    __TBB_NOINLINE_SYM multifunction_node(const node_set<Args...>& nodes, size_t concurrency, Body body, node_priority_t a_priority)
        : multifunction_node(nodes, concurrency, body, Policy(), a_priority) {}
#endif // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

    __TBB_NOINLINE_SYM multifunction_node( const multifunction_node &other) :
        graph_node(other.my_graph), input_impl_type(other) {
        fgt_multioutput_node_with_body<N>( CODEPTR(), FLOW_MULTIFUNCTION_NODE,
                &this->my_graph, static_cast<receiver<input_type> *>(this),
                this->output_ports(), this->my_body );
    }

    // all the guts are in multifunction_input...
protected:
    void reset_node(reset_flags f) override { input_impl_type::reset(f); }
};  // multifunction_node

//! split_node: accepts a tuple as input, forwards each element of the tuple to its
//  successors.  The node has unlimited concurrency, so it does not reject inputs.
template<typename TupleType>
class split_node : public graph_node, public receiver<TupleType> {
    static const int N = std::tuple_size<TupleType>::value;
    typedef receiver<TupleType> base_type;
public:
    typedef TupleType input_type;
    typedef typename wrap_tuple_elements<
            N,  // #elements in tuple
            multifunction_output,  // wrap this around each element
            TupleType // the tuple providing the types
        >::type  output_ports_type;

    __TBB_NOINLINE_SYM explicit split_node(graph &g)
        : graph_node(g),
          my_output_ports(init_output_ports<output_ports_type>::call(g, my_output_ports))
    {
        fgt_multioutput_node<N>(CODEPTR(), FLOW_SPLIT_NODE, &this->my_graph,
            static_cast<receiver<input_type> *>(this), this->output_ports());
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    __TBB_NOINLINE_SYM split_node(const node_set<Args...>& nodes) : split_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    __TBB_NOINLINE_SYM split_node(const split_node& other)
        : graph_node(other.my_graph), base_type(other),
          my_output_ports(init_output_ports<output_ports_type>::call(other.my_graph, my_output_ports))
    {
        fgt_multioutput_node<N>(CODEPTR(), FLOW_SPLIT_NODE, &this->my_graph,
            static_cast<receiver<input_type> *>(this), this->output_ports());
    }

    output_ports_type &output_ports() { return my_output_ports; }

protected:
    graph_task *try_put_task(const TupleType& t) override {
        // Sending split messages in parallel is not justified, as overheads would prevail.
        // Also, we do not have successors here. So we just tell the task returned here is successful.
        return emit_element<N>::emit_this(this->my_graph, t, output_ports());
    }
    void reset_node(reset_flags f) override {
        if (f & rf_clear_edges)
            clear_element<N>::clear_this(my_output_ports);

        __TBB_ASSERT(!(f & rf_clear_edges) || clear_element<N>::this_empty(my_output_ports), "split_node reset failed");
    }
    graph& graph_reference() const override {
        return my_graph;
    }

private:
    output_ports_type my_output_ports;
};

//! Implements an executable node that supports continue_msg -> Output
template <typename Output, typename Policy = Policy<void> >
    __TBB_requires(std::copy_constructible<Output>)
class continue_node : public graph_node, public continue_input<Output, Policy>,
                      public function_output<Output> {
public:
    typedef continue_msg input_type;
    typedef Output output_type;
    typedef continue_input<Output, Policy> input_impl_type;
    typedef function_output<output_type> fOutput_type;
    typedef typename input_impl_type::predecessor_type predecessor_type;
    typedef typename fOutput_type::successor_type successor_type;

    //! Constructor for executable node with continue_msg -> Output
    template <typename Body >
        __TBB_requires(continue_node_body<Body, Output>)
    __TBB_NOINLINE_SYM continue_node(
        graph &g,
        Body body, Policy = Policy(), node_priority_t a_priority = no_priority
    ) : graph_node(g), input_impl_type( g, body, a_priority ),
        fOutput_type(g) {
        fgt_node_with_body( CODEPTR(), FLOW_CONTINUE_NODE, &this->my_graph,

                                           static_cast<receiver<input_type> *>(this),
                                           static_cast<sender<output_type> *>(this), this->my_body );
    }

    template <typename Body>
        __TBB_requires(continue_node_body<Body, Output>)
    continue_node( graph& g, Body body, node_priority_t a_priority )
        : continue_node(g, body, Policy(), a_priority) {}

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename Body, typename... Args>
        __TBB_requires(continue_node_body<Body, Output>)
    continue_node( const node_set<Args...>& nodes, Body body,
                   Policy p = Policy(), node_priority_t a_priority = no_priority )
        : continue_node(nodes.graph_reference(), body, p, a_priority ) {
        make_edges_in_order(nodes, *this);
    }
    template <typename Body, typename... Args>
        __TBB_requires(continue_node_body<Body, Output>)
    continue_node( const node_set<Args...>& nodes, Body body, node_priority_t a_priority)
        : continue_node(nodes, body, Policy(), a_priority) {}
#endif // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

    //! Constructor for executable node with continue_msg -> Output
    template <typename Body >
        __TBB_requires(continue_node_body<Body, Output>)
    __TBB_NOINLINE_SYM continue_node(
        graph &g, int number_of_predecessors,
        Body body, Policy = Policy(), node_priority_t a_priority = no_priority
    ) : graph_node(g)
      , input_impl_type(g, number_of_predecessors, body, a_priority),
        fOutput_type(g) {
        fgt_node_with_body( CODEPTR(), FLOW_CONTINUE_NODE, &this->my_graph,
                                           static_cast<receiver<input_type> *>(this),
                                           static_cast<sender<output_type> *>(this), this->my_body );
    }

    template <typename Body>
        __TBB_requires(continue_node_body<Body, Output>)
    continue_node( graph& g, int number_of_predecessors, Body body, node_priority_t a_priority)
        : continue_node(g, number_of_predecessors, body, Policy(), a_priority) {}

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename Body, typename... Args>
        __TBB_requires(continue_node_body<Body, Output>)
    continue_node( const node_set<Args...>& nodes, int number_of_predecessors,
                   Body body, Policy p = Policy(), node_priority_t a_priority = no_priority )
        : continue_node(nodes.graph_reference(), number_of_predecessors, body, p, a_priority) {
        make_edges_in_order(nodes, *this);
    }

    template <typename Body, typename... Args>
        __TBB_requires(continue_node_body<Body, Output>)
    continue_node( const node_set<Args...>& nodes, int number_of_predecessors,
                   Body body, node_priority_t a_priority )
        : continue_node(nodes, number_of_predecessors, body, Policy(), a_priority) {}
#endif

    //! Copy constructor
    __TBB_NOINLINE_SYM continue_node( const continue_node& src ) :
        graph_node(src.my_graph), input_impl_type(src),
        function_output<Output>(src.my_graph) {
        fgt_node_with_body( CODEPTR(), FLOW_CONTINUE_NODE, &this->my_graph,
                                           static_cast<receiver<input_type> *>(this),
                                           static_cast<sender<output_type> *>(this), this->my_body );
    }

protected:
    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    using input_impl_type::try_put_task;
    broadcast_cache<output_type> &successors () override { return fOutput_type::my_successors; }

    void reset_node(reset_flags f) override {
        input_impl_type::reset_receiver(f);
        if(f & rf_clear_edges)successors().clear();
        __TBB_ASSERT(!(f & rf_clear_edges) || successors().empty(), "continue_node not reset");
    }
};  // continue_node

//! Forwards messages of type T to all successors
template <typename T>
class broadcast_node : public graph_node, public receiver<T>, public sender<T> {
public:
    typedef T input_type;
    typedef T output_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;
private:
    broadcast_cache<input_type> my_successors;
public:

    __TBB_NOINLINE_SYM explicit broadcast_node(graph& g) : graph_node(g), my_successors(this) {
        fgt_node( CODEPTR(), FLOW_BROADCAST_NODE, &this->my_graph,
                  static_cast<receiver<input_type> *>(this), static_cast<sender<output_type> *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    broadcast_node(const node_set<Args...>& nodes) : broadcast_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM broadcast_node( const broadcast_node& src ) : broadcast_node(src.my_graph) {}

    //! Adds a successor
    bool register_successor( successor_type &r ) override {
        my_successors.register_successor( r );
        return true;
    }

    //! Removes s as a successor
    bool remove_successor( successor_type &r ) override {
        my_successors.remove_successor( r );
        return true;
    }

protected:
    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    //! build a task to run the successor if possible.  Default is old behavior.
    graph_task *try_put_task(const T& t) override {
        graph_task *new_task = my_successors.try_put_task(t);
        if (!new_task) new_task = SUCCESSFULLY_ENQUEUED;
        return new_task;
    }

    graph& graph_reference() const override {
        return my_graph;
    }

    void reset_node(reset_flags f) override {
        if (f&rf_clear_edges) {
           my_successors.clear();
        }
        __TBB_ASSERT(!(f & rf_clear_edges) || my_successors.empty(), "Error resetting broadcast_node");
    }
};  // broadcast_node

//! Forwards messages in arbitrary order
template <typename T>
class buffer_node
    : public graph_node
    , public reservable_item_buffer< T, cache_aligned_allocator<T> >
    , public receiver<T>, public sender<T>
{
    typedef cache_aligned_allocator<T> internals_allocator;

public:
    typedef T input_type;
    typedef T output_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;
    typedef buffer_node<T> class_type;

protected:
    typedef size_t size_type;
    round_robin_cache< T, null_rw_mutex > my_successors;

    friend class forward_task_bypass< class_type >;

    enum op_type {reg_succ, rem_succ, req_item, res_item, rel_res, con_res, put_item, try_fwd_task
    };

    // implements the aggregator_operation concept
    class buffer_operation : public aggregated_operation< buffer_operation > {
    public:
        char type;
        T* elem;
        graph_task* ltask;
        successor_type *r;

        buffer_operation(const T& e, op_type t) : type(char(t))
                                                  , elem(const_cast<T*>(&e)) , ltask(nullptr)
                                                  , r(nullptr)
        {}
        buffer_operation(op_type t) : type(char(t)), elem(nullptr), ltask(nullptr), r(nullptr) {}
    };

    bool forwarder_busy;
    typedef aggregating_functor<class_type, buffer_operation> handler_type;
    friend class aggregating_functor<class_type, buffer_operation>;
    aggregator< handler_type, buffer_operation> my_aggregator;

    virtual void handle_operations(buffer_operation *op_list) {
        handle_operations_impl(op_list, this);
    }

    template<typename derived_type>
    void handle_operations_impl(buffer_operation *op_list, derived_type* derived) {
        __TBB_ASSERT(static_cast<class_type*>(derived) == this, "'this' is not a base class for derived");

        buffer_operation *tmp = nullptr;
        bool try_forwarding = false;
        while (op_list) {
            tmp = op_list;
            op_list = op_list->next;
            switch (tmp->type) {
            case reg_succ: internal_reg_succ(tmp); try_forwarding = true; break;
            case rem_succ: internal_rem_succ(tmp); break;
            case req_item: internal_pop(tmp); break;
            case res_item: internal_reserve(tmp); break;
            case rel_res:  internal_release(tmp); try_forwarding = true; break;
            case con_res:  internal_consume(tmp); try_forwarding = true; break;
            case put_item: try_forwarding = internal_push(tmp); break;
            case try_fwd_task: internal_forward_task(tmp); break;
            }
        }

        derived->order();

        if (try_forwarding && !forwarder_busy) {
            if(is_graph_active(this->my_graph)) {
                forwarder_busy = true;
                typedef forward_task_bypass<class_type> task_type;
                small_object_allocator allocator{};
                graph_task* new_task = allocator.new_object<task_type>(graph_reference(), allocator, *this);
                my_graph.reserve_wait();
                // tmp should point to the last item handled by the aggregator.  This is the operation
                // the handling thread enqueued.  So modifying that record will be okay.
                // TODO revamp: check that the issue is still present
                // workaround for icc bug  (at least 12.0 and 13.0)
                // error: function "tbb::flow::interfaceX::combine_tasks" cannot be called with the given argument list
                //        argument types are: (graph, graph_task *, graph_task *)
                graph_task *z = tmp->ltask;
                graph &g = this->my_graph;
                tmp->ltask = combine_tasks(g, z, new_task);  // in case the op generated a task
            }
        }
    }  // handle_operations

    inline graph_task *grab_forwarding_task( buffer_operation &op_data) {
        return op_data.ltask;
    }

    inline bool enqueue_forwarding_task(buffer_operation &op_data) {
        graph_task *ft = grab_forwarding_task(op_data);
        if(ft) {
            spawn_in_graph_arena(graph_reference(), *ft);
            return true;
        }
        return false;
    }

    //! This is executed by an enqueued task, the "forwarder"
    virtual graph_task *forward_task() {
        buffer_operation op_data(try_fwd_task);
        graph_task *last_task = nullptr;
        do {
            op_data.status = WAIT;
            op_data.ltask = nullptr;
            my_aggregator.execute(&op_data);

            // workaround for icc bug
            graph_task *xtask = op_data.ltask;
            graph& g = this->my_graph;
            last_task = combine_tasks(g, last_task, xtask);
        } while (op_data.status ==SUCCEEDED);
        return last_task;
    }

    //! Register successor
    virtual void internal_reg_succ(buffer_operation *op) {
        __TBB_ASSERT(op->r, nullptr);
        my_successors.register_successor(*(op->r));
        op->status.store(SUCCEEDED, std::memory_order_release);
    }

    //! Remove successor
    virtual void internal_rem_succ(buffer_operation *op) {
        __TBB_ASSERT(op->r, nullptr);
        my_successors.remove_successor(*(op->r));
        op->status.store(SUCCEEDED, std::memory_order_release);
    }

private:
    void order() {}

    bool is_item_valid() {
        return this->my_item_valid(this->my_tail - 1);
    }

    void try_put_and_add_task(graph_task*& last_task) {
        graph_task *new_task = my_successors.try_put_task(this->back());
        if (new_task) {
            // workaround for icc bug
            graph& g = this->my_graph;
            last_task = combine_tasks(g, last_task, new_task);
            this->destroy_back();
        }
    }

protected:
    //! Tries to forward valid items to successors
    virtual void internal_forward_task(buffer_operation *op) {
        internal_forward_task_impl(op, this);
    }

    template<typename derived_type>
    void internal_forward_task_impl(buffer_operation *op, derived_type* derived) {
        __TBB_ASSERT(static_cast<class_type*>(derived) == this, "'this' is not a base class for derived");

        if (this->my_reserved || !derived->is_item_valid()) {
            op->status.store(FAILED, std::memory_order_release);
            this->forwarder_busy = false;
            return;
        }
        // Try forwarding, giving each successor a chance
        graph_task* last_task = nullptr;
        size_type counter = my_successors.size();
        for (; counter > 0 && derived->is_item_valid(); --counter)
            derived->try_put_and_add_task(last_task);

        op->ltask = last_task;  // return task
        if (last_task && !counter) {
            op->status.store(SUCCEEDED, std::memory_order_release);
        }
        else {
            op->status.store(FAILED, std::memory_order_release);
            forwarder_busy = false;
        }
    }

    virtual bool internal_push(buffer_operation *op) {
        __TBB_ASSERT(op->elem, nullptr);
        this->push_back(*(op->elem));
        op->status.store(SUCCEEDED, std::memory_order_release);
        return true;
    }

    virtual void internal_pop(buffer_operation *op) {
        __TBB_ASSERT(op->elem, nullptr);
        if(this->pop_back(*(op->elem))) {
            op->status.store(SUCCEEDED, std::memory_order_release);
        }
        else {
            op->status.store(FAILED, std::memory_order_release);
        }
    }

    virtual void internal_reserve(buffer_operation *op) {
        __TBB_ASSERT(op->elem, nullptr);
        if(this->reserve_front(*(op->elem))) {
            op->status.store(SUCCEEDED, std::memory_order_release);
        }
        else {
            op->status.store(FAILED, std::memory_order_release);
        }
    }

    virtual void internal_consume(buffer_operation *op) {
        this->consume_front();
        op->status.store(SUCCEEDED, std::memory_order_release);
    }

    virtual void internal_release(buffer_operation *op) {
        this->release_front();
        op->status.store(SUCCEEDED, std::memory_order_release);
    }

public:
    //! Constructor
    __TBB_NOINLINE_SYM explicit buffer_node( graph &g )
        : graph_node(g), reservable_item_buffer<T, internals_allocator>(), receiver<T>(),
          sender<T>(), my_successors(this), forwarder_busy(false)
    {
        my_aggregator.initialize_handler(handler_type(this));
        fgt_node( CODEPTR(), FLOW_BUFFER_NODE, &this->my_graph,
                                 static_cast<receiver<input_type> *>(this), static_cast<sender<output_type> *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    buffer_node(const node_set<Args...>& nodes) : buffer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    //! Copy constructor
    __TBB_NOINLINE_SYM buffer_node( const buffer_node& src ) : buffer_node(src.my_graph) {}

    //
    // message sender implementation
    //

    //! Adds a new successor.
    /** Adds successor r to the list of successors; may forward tasks.  */
    bool register_successor( successor_type &r ) override {
        buffer_operation op_data(reg_succ);
        op_data.r = &r;
        my_aggregator.execute(&op_data);
        (void)enqueue_forwarding_task(op_data);
        return true;
    }

    //! Removes a successor.
    /** Removes successor r from the list of successors.
        It also calls r.remove_predecessor(*this) to remove this node as a predecessor. */
    bool remove_successor( successor_type &r ) override {
        // TODO revamp: investigate why full qualification is necessary here
        tbb::detail::d1::remove_predecessor(r, *this);
        buffer_operation op_data(rem_succ);
        op_data.r = &r;
        my_aggregator.execute(&op_data);
        // even though this operation does not cause a forward, if we are the handler, and
        // a forward is scheduled, we may be the first to reach this point after the aggregator,
        // and so should check for the task.
        (void)enqueue_forwarding_task(op_data);
        return true;
    }

    //! Request an item from the buffer_node
    /**  true = v contains the returned item<BR>
         false = no item has been returned */
    bool try_get( T &v ) override {
        buffer_operation op_data(req_item);
        op_data.elem = &v;
        my_aggregator.execute(&op_data);
        (void)enqueue_forwarding_task(op_data);
        return (op_data.status==SUCCEEDED);
    }

    //! Reserves an item.
    /**  false = no item can be reserved<BR>
         true = an item is reserved */
    bool try_reserve( T &v ) override {
        buffer_operation op_data(res_item);
        op_data.elem = &v;
        my_aggregator.execute(&op_data);
        (void)enqueue_forwarding_task(op_data);
        return (op_data.status==SUCCEEDED);
    }

    //! Release a reserved item.
    /**  true = item has been released and so remains in sender */
    bool try_release() override {
        buffer_operation op_data(rel_res);
        my_aggregator.execute(&op_data);
        (void)enqueue_forwarding_task(op_data);
        return true;
    }

    //! Consumes a reserved item.
    /** true = item is removed from sender and reservation removed */
    bool try_consume() override {
        buffer_operation op_data(con_res);
        my_aggregator.execute(&op_data);
        (void)enqueue_forwarding_task(op_data);
        return true;
    }

protected:

    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    //! receive an item, return a task *if possible
    graph_task *try_put_task(const T &t) override {
        buffer_operation op_data(t, put_item);
        my_aggregator.execute(&op_data);
        graph_task *ft = grab_forwarding_task(op_data);
        // sequencer_nodes can return failure (if an item has been previously inserted)
        // We have to spawn the returned task if our own operation fails.

        if(ft && op_data.status ==FAILED) {
            // we haven't succeeded queueing the item, but for some reason the
            // call returned a task (if another request resulted in a successful
            // forward this could happen.)  Queue the task and reset the pointer.
            spawn_in_graph_arena(graph_reference(), *ft); ft = nullptr;
        }
        else if(!ft && op_data.status ==SUCCEEDED) {
            ft = SUCCESSFULLY_ENQUEUED;
        }
        return ft;
    }

    graph& graph_reference() const override {
        return my_graph;
    }

protected:
    void reset_node( reset_flags f) override {
        reservable_item_buffer<T, internals_allocator>::reset();
        // TODO: just clear structures
        if (f&rf_clear_edges) {
            my_successors.clear();
        }
        forwarder_busy = false;
    }
};  // buffer_node

//! Forwards messages in FIFO order
template <typename T>
class queue_node : public buffer_node<T> {
protected:
    typedef buffer_node<T> base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::buffer_operation queue_operation;
    typedef queue_node class_type;

private:
    template<typename> friend class buffer_node;

    bool is_item_valid() {
        return this->my_item_valid(this->my_head);
    }

    void try_put_and_add_task(graph_task*& last_task) {
        graph_task *new_task = this->my_successors.try_put_task(this->front());
        if (new_task) {
            // workaround for icc bug
            graph& graph_ref = this->graph_reference();
            last_task = combine_tasks(graph_ref, last_task, new_task);
            this->destroy_front();
        }
    }

protected:
    void internal_forward_task(queue_operation *op) override {
        this->internal_forward_task_impl(op, this);
    }

    void internal_pop(queue_operation *op) override {
        if ( this->my_reserved || !this->my_item_valid(this->my_head)){
            op->status.store(FAILED, std::memory_order_release);
        }
        else {
            this->pop_front(*(op->elem));
            op->status.store(SUCCEEDED, std::memory_order_release);
        }
    }
    void internal_reserve(queue_operation *op) override {
        if (this->my_reserved || !this->my_item_valid(this->my_head)) {
            op->status.store(FAILED, std::memory_order_release);
        }
        else {
            this->reserve_front(*(op->elem));
            op->status.store(SUCCEEDED, std::memory_order_release);
        }
    }
    void internal_consume(queue_operation *op) override {
        this->consume_front();
        op->status.store(SUCCEEDED, std::memory_order_release);
    }

public:
    typedef T input_type;
    typedef T output_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;

    //! Constructor
    __TBB_NOINLINE_SYM explicit queue_node( graph &g ) : base_type(g) {
        fgt_node( CODEPTR(), FLOW_QUEUE_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    queue_node( const node_set<Args...>& nodes) : queue_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    //! Copy constructor
    __TBB_NOINLINE_SYM queue_node( const queue_node& src) : base_type(src) {
        fgt_node( CODEPTR(), FLOW_QUEUE_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }


protected:
    void reset_node( reset_flags f) override {
        base_type::reset_node(f);
    }
};  // queue_node

//! Forwards messages in sequence order
template <typename T>
    __TBB_requires(std::copyable<T>)
class sequencer_node : public queue_node<T> {
    function_body< T, size_t > *my_sequencer;
    // my_sequencer should be a benign function and must be callable
    // from a parallel context.  Does this mean it needn't be reset?
public:
    typedef T input_type;
    typedef T output_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;

    //! Constructor
    template< typename Sequencer >
        __TBB_requires(sequencer<Sequencer, T>)
    __TBB_NOINLINE_SYM sequencer_node( graph &g, const Sequencer& s ) : queue_node<T>(g),
        my_sequencer(new function_body_leaf< T, size_t, Sequencer>(s) ) {
        fgt_node( CODEPTR(), FLOW_SEQUENCER_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename Sequencer, typename... Args>
        __TBB_requires(sequencer<Sequencer, T>)
    sequencer_node( const node_set<Args...>& nodes, const Sequencer& s)
        : sequencer_node(nodes.graph_reference(), s) {
        make_edges_in_order(nodes, *this);
    }
#endif

    //! Copy constructor
    __TBB_NOINLINE_SYM sequencer_node( const sequencer_node& src ) : queue_node<T>(src),
        my_sequencer( src.my_sequencer->clone() ) {
        fgt_node( CODEPTR(), FLOW_SEQUENCER_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }

    //! Destructor
    ~sequencer_node() { delete my_sequencer; }

protected:
    typedef typename buffer_node<T>::size_type size_type;
    typedef typename buffer_node<T>::buffer_operation sequencer_operation;

private:
    bool internal_push(sequencer_operation *op) override {
        size_type tag = (*my_sequencer)(*(op->elem));
#if !TBB_DEPRECATED_SEQUENCER_DUPLICATES
        if (tag < this->my_head) {
            // have already emitted a message with this tag
            op->status.store(FAILED, std::memory_order_release);
            return false;
        }
#endif
        // cannot modify this->my_tail now; the buffer would be inconsistent.
        size_t new_tail = (tag+1 > this->my_tail) ? tag+1 : this->my_tail;

        if (this->size(new_tail) > this->capacity()) {
            this->grow_my_array(this->size(new_tail));
        }
        this->my_tail = new_tail;

        const op_stat res = this->place_item(tag, *(op->elem)) ? SUCCEEDED : FAILED;
        op->status.store(res, std::memory_order_release);
        return res ==SUCCEEDED;
    }
};  // sequencer_node

//! Forwards messages in priority order
template<typename T, typename Compare = std::less<T>>
class priority_queue_node : public buffer_node<T> {
public:
    typedef T input_type;
    typedef T output_type;
    typedef buffer_node<T> base_type;
    typedef priority_queue_node class_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;

    //! Constructor
    __TBB_NOINLINE_SYM explicit priority_queue_node( graph &g, const Compare& comp = Compare() )
        : buffer_node<T>(g), compare(comp), mark(0) {
        fgt_node( CODEPTR(), FLOW_PRIORITY_QUEUE_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    priority_queue_node(const node_set<Args...>& nodes, const Compare& comp = Compare())
        : priority_queue_node(nodes.graph_reference(), comp) {
        make_edges_in_order(nodes, *this);
    }
#endif

    //! Copy constructor
    __TBB_NOINLINE_SYM priority_queue_node( const priority_queue_node &src )
        : buffer_node<T>(src), mark(0)
    {
        fgt_node( CODEPTR(), FLOW_PRIORITY_QUEUE_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }

protected:

    void reset_node( reset_flags f) override {
        mark = 0;
        base_type::reset_node(f);
    }

    typedef typename buffer_node<T>::size_type size_type;
    typedef typename buffer_node<T>::item_type item_type;
    typedef typename buffer_node<T>::buffer_operation prio_operation;

    //! Tries to forward valid items to successors
    void internal_forward_task(prio_operation *op) override {
        this->internal_forward_task_impl(op, this);
    }

    void handle_operations(prio_operation *op_list) override {
        this->handle_operations_impl(op_list, this);
    }

    bool internal_push(prio_operation *op) override {
        prio_push(*(op->elem));
        op->status.store(SUCCEEDED, std::memory_order_release);
        return true;
    }

    void internal_pop(prio_operation *op) override {
        // if empty or already reserved, don't pop
        if ( this->my_reserved == true || this->my_tail == 0 ) {
            op->status.store(FAILED, std::memory_order_release);
            return;
        }

        *(op->elem) = prio();
        op->status.store(SUCCEEDED, std::memory_order_release);
        prio_pop();

    }

    // pops the highest-priority item, saves copy
    void internal_reserve(prio_operation *op) override {
        if (this->my_reserved == true || this->my_tail == 0) {
            op->status.store(FAILED, std::memory_order_release);
            return;
        }
        this->my_reserved = true;
        *(op->elem) = prio();
        reserved_item = *(op->elem);
        op->status.store(SUCCEEDED, std::memory_order_release);
        prio_pop();
    }

    void internal_consume(prio_operation *op) override {
        op->status.store(SUCCEEDED, std::memory_order_release);
        this->my_reserved = false;
        reserved_item = input_type();
    }

    void internal_release(prio_operation *op) override {
        op->status.store(SUCCEEDED, std::memory_order_release);
        prio_push(reserved_item);
        this->my_reserved = false;
        reserved_item = input_type();
    }

private:
    template<typename> friend class buffer_node;

    void order() {
        if (mark < this->my_tail) heapify();
        __TBB_ASSERT(mark == this->my_tail, "mark unequal after heapify");
    }

    bool is_item_valid() {
        return this->my_tail > 0;
    }

    void try_put_and_add_task(graph_task*& last_task) {
        graph_task * new_task = this->my_successors.try_put_task(this->prio());
        if (new_task) {
            // workaround for icc bug
            graph& graph_ref = this->graph_reference();
            last_task = combine_tasks(graph_ref, last_task, new_task);
            prio_pop();
        }
    }

private:
    Compare compare;
    size_type mark;

    input_type reserved_item;

    // in case a reheap has not been done after a push, check if the mark item is higher than the 0'th item
    bool prio_use_tail() {
        __TBB_ASSERT(mark <= this->my_tail, "mark outside bounds before test");
        return mark < this->my_tail && compare(this->get_my_item(0), this->get_my_item(this->my_tail - 1));
    }

    // prio_push: checks that the item will fit, expand array if necessary, put at end
    void prio_push(const T &src) {
        if ( this->my_tail >= this->my_array_size )
            this->grow_my_array( this->my_tail + 1 );
        (void) this->place_item(this->my_tail, src);
        ++(this->my_tail);
        __TBB_ASSERT(mark < this->my_tail, "mark outside bounds after push");
    }

    // prio_pop: deletes highest priority item from the array, and if it is item
    // 0, move last item to 0 and reheap.  If end of array, just destroy and decrement tail
    // and mark.  Assumes the array has already been tested for emptiness; no failure.
    void prio_pop()  {
        if (prio_use_tail()) {
            // there are newly pushed elements; last one higher than top
            // copy the data
            this->destroy_item(this->my_tail-1);
            --(this->my_tail);
            __TBB_ASSERT(mark <= this->my_tail, "mark outside bounds after pop");
            return;
        }
        this->destroy_item(0);
        if(this->my_tail > 1) {
            // push the last element down heap
            __TBB_ASSERT(this->my_item_valid(this->my_tail - 1), nullptr);
            this->move_item(0,this->my_tail - 1);
        }
        --(this->my_tail);
        if(mark > this->my_tail) --mark;
        if (this->my_tail > 1) // don't reheap for heap of size 1
            reheap();
        __TBB_ASSERT(mark <= this->my_tail, "mark outside bounds after pop");
    }

    const T& prio() {
        return this->get_my_item(prio_use_tail() ? this->my_tail-1 : 0);
    }

    // turn array into heap
    void heapify() {
        if(this->my_tail == 0) {
            mark = 0;
            return;
        }
        if (!mark) mark = 1;
        for (; mark<this->my_tail; ++mark) { // for each unheaped element
            size_type cur_pos = mark;
            input_type to_place;
            this->fetch_item(mark,to_place);
            do { // push to_place up the heap
                size_type parent = (cur_pos-1)>>1;
                if (!compare(this->get_my_item(parent), to_place))
                    break;
                this->move_item(cur_pos, parent);
                cur_pos = parent;
            } while( cur_pos );
            (void) this->place_item(cur_pos, to_place);
        }
    }

    // otherwise heapified array with new root element; rearrange to heap
    void reheap() {
        size_type cur_pos=0, child=1;
        while (child < mark) {
            size_type target = child;
            if (child+1<mark &&
                compare(this->get_my_item(child),
                        this->get_my_item(child+1)))
                ++target;
            // target now has the higher priority child
            if (compare(this->get_my_item(target),
                        this->get_my_item(cur_pos)))
                break;
            // swap
            this->swap_items(cur_pos, target);
            cur_pos = target;
            child = (cur_pos<<1)+1;
        }
    }
};  // priority_queue_node

//! Forwards messages only if the threshold has not been reached
/** This node forwards items until its threshold is reached.
    It contains no buffering.  If the downstream node rejects, the
    message is dropped. */
template< typename T, typename DecrementType=continue_msg >
class limiter_node : public graph_node, public receiver< T >, public sender< T > {
public:
    typedef T input_type;
    typedef T output_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;
    //TODO: There is a lack of predefined types for its controlling "decrementer" port. It should be fixed later.

private:
    size_t my_threshold;
    size_t my_count; // number of successful puts
    size_t my_tries; // number of active put attempts
    size_t my_future_decrement; // number of active decrement
    reservable_predecessor_cache< T, spin_mutex > my_predecessors;
    spin_mutex my_mutex;
    broadcast_cache< T > my_successors;

    //! The internal receiver< DecrementType > that adjusts the count
    threshold_regulator< limiter_node<T, DecrementType>, DecrementType > decrement;

    graph_task* decrement_counter( long long delta ) {
        if ( delta > 0 && size_t(delta) > my_threshold ) {
            delta = my_threshold;
        }

        {
            spin_mutex::scoped_lock lock(my_mutex);
            if ( delta > 0 && size_t(delta) > my_count ) {
                if( my_tries > 0 ) {
                    my_future_decrement += (size_t(delta) - my_count);
                }
                my_count = 0;
            }
            else if ( delta < 0 && size_t(-delta) > my_threshold - my_count ) {
                my_count = my_threshold;
            }
            else {
                my_count -= size_t(delta); // absolute value of delta is sufficiently small
            }
            __TBB_ASSERT(my_count <= my_threshold, "counter values are truncated to be inside the [0, threshold] interval");
        }
        return forward_task();
    }

    // Let threshold_regulator call decrement_counter()
    friend class threshold_regulator< limiter_node<T, DecrementType>, DecrementType >;

    friend class forward_task_bypass< limiter_node<T,DecrementType> >;

    bool check_conditions() {  // always called under lock
        return ( my_count + my_tries < my_threshold && !my_predecessors.empty() && !my_successors.empty() );
    }

    // only returns a valid task pointer or nullptr, never SUCCESSFULLY_ENQUEUED
    graph_task* forward_task() {
        input_type v;
        graph_task* rval = nullptr;
        bool reserved = false;

        {
            spin_mutex::scoped_lock lock(my_mutex);
            if ( check_conditions() )
                ++my_tries;
            else
                return nullptr;
        }

        //SUCCESS
        // if we can reserve and can put, we consume the reservation
        // we increment the count and decrement the tries
        if ( (my_predecessors.try_reserve(v)) == true ) {
            reserved = true;
            if ( (rval = my_successors.try_put_task(v)) != nullptr ) {
                {
                    spin_mutex::scoped_lock lock(my_mutex);
                    ++my_count;
                    if ( my_future_decrement ) {
                        if ( my_count > my_future_decrement ) {
                            my_count -= my_future_decrement;
                            my_future_decrement = 0;
                        }
                        else {
                            my_future_decrement -= my_count;
                            my_count = 0;
                        }
                    }
                    --my_tries;
                    my_predecessors.try_consume();
                    if ( check_conditions() ) {
                        if ( is_graph_active(this->my_graph) ) {
                            typedef forward_task_bypass<limiter_node<T, DecrementType>> task_type;
                            small_object_allocator allocator{};
                            graph_task* rtask = allocator.new_object<task_type>( my_graph, allocator, *this );
                            my_graph.reserve_wait();
                            spawn_in_graph_arena(graph_reference(), *rtask);
                        }
                    }
                }
                return rval;
            }
        }
        //FAILURE
        //if we can't reserve, we decrement the tries
        //if we can reserve but can't put, we decrement the tries and release the reservation
        {
            spin_mutex::scoped_lock lock(my_mutex);
            --my_tries;
            if (reserved) my_predecessors.try_release();
            if ( check_conditions() ) {
                if ( is_graph_active(this->my_graph) ) {
                    small_object_allocator allocator{};
                    typedef forward_task_bypass<limiter_node<T, DecrementType>> task_type;
                    graph_task* t = allocator.new_object<task_type>(my_graph, allocator, *this);
                    my_graph.reserve_wait();
                    __TBB_ASSERT(!rval, "Have two tasks to handle");
                    return t;
                }
            }
            return rval;
        }
    }

    void initialize() {
        fgt_node(
            CODEPTR(), FLOW_LIMITER_NODE, &this->my_graph,
            static_cast<receiver<input_type> *>(this), static_cast<receiver<DecrementType> *>(&decrement),
            static_cast<sender<output_type> *>(this)
        );
    }

public:
    //! Constructor
    limiter_node(graph &g, size_t threshold)
        : graph_node(g), my_threshold(threshold), my_count(0), my_tries(0), my_future_decrement(0),
        my_predecessors(this), my_successors(this), decrement(this)
    {
        initialize();
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    limiter_node(const node_set<Args...>& nodes, size_t threshold)
        : limiter_node(nodes.graph_reference(), threshold) {
        make_edges_in_order(nodes, *this);
    }
#endif

    //! Copy constructor
    limiter_node( const limiter_node& src ) : limiter_node(src.my_graph, src.my_threshold) {}

    //! The interface for accessing internal receiver< DecrementType > that adjusts the count
    receiver<DecrementType>& decrementer() { return decrement; }

    //! Replace the current successor with this new successor
    bool register_successor( successor_type &r ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        bool was_empty = my_successors.empty();
        my_successors.register_successor(r);
        //spawn a forward task if this is the only successor
        if ( was_empty && !my_predecessors.empty() && my_count + my_tries < my_threshold ) {
            if ( is_graph_active(this->my_graph) ) {
                small_object_allocator allocator{};
                typedef forward_task_bypass<limiter_node<T, DecrementType>> task_type;
                graph_task* t = allocator.new_object<task_type>(my_graph, allocator, *this);
                my_graph.reserve_wait();
                spawn_in_graph_arena(graph_reference(), *t);
            }
        }
        return true;
    }

    //! Removes a successor from this node
    /** r.remove_predecessor(*this) is also called. */
    bool remove_successor( successor_type &r ) override {
        // TODO revamp: investigate why qualification is needed for remove_predecessor() call
        tbb::detail::d1::remove_predecessor(r, *this);
        my_successors.remove_successor(r);
        return true;
    }

    //! Adds src to the list of cached predecessors.
    bool register_predecessor( predecessor_type &src ) override {
        spin_mutex::scoped_lock lock(my_mutex);
        my_predecessors.add( src );
        if ( my_count + my_tries < my_threshold && !my_successors.empty() && is_graph_active(this->my_graph) ) {
            small_object_allocator allocator{};
            typedef forward_task_bypass<limiter_node<T, DecrementType>> task_type;
            graph_task* t = allocator.new_object<task_type>(my_graph, allocator, *this);
            my_graph.reserve_wait();
            spawn_in_graph_arena(graph_reference(), *t);
        }
        return true;
    }

    //! Removes src from the list of cached predecessors.
    bool remove_predecessor( predecessor_type &src ) override {
        my_predecessors.remove( src );
        return true;
    }

protected:

    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    //! Puts an item to this receiver
    graph_task* try_put_task( const T &t ) override {
        {
            spin_mutex::scoped_lock lock(my_mutex);
            if ( my_count + my_tries >= my_threshold )
                return nullptr;
            else
                ++my_tries;
        }

        graph_task* rtask = my_successors.try_put_task(t);
        if ( !rtask ) {  // try_put_task failed.
            spin_mutex::scoped_lock lock(my_mutex);
            --my_tries;
            if (check_conditions() && is_graph_active(this->my_graph)) {
                small_object_allocator allocator{};
                typedef forward_task_bypass<limiter_node<T, DecrementType>> task_type;
                rtask = allocator.new_object<task_type>(my_graph, allocator, *this);
                my_graph.reserve_wait();
            }
        }
        else {
            spin_mutex::scoped_lock lock(my_mutex);
            ++my_count;
            if ( my_future_decrement ) {
                if ( my_count > my_future_decrement ) {
                    my_count -= my_future_decrement;
                    my_future_decrement = 0;
                }
                else {
                    my_future_decrement -= my_count;
                    my_count = 0;
                }
            }
            --my_tries;
        }
        return rtask;
    }

    graph& graph_reference() const override { return my_graph; }

    void reset_node( reset_flags f ) override {
        my_count = 0;
        if ( f & rf_clear_edges ) {
            my_predecessors.clear();
            my_successors.clear();
        }
        else {
            my_predecessors.reset();
        }
        decrement.reset_receiver(f);
    }
};  // limiter_node



// included into namespace tbb::detail::d1

    struct forwarding_base : no_assign {
        forwarding_base(graph &g) : graph_ref(g) {}
        virtual ~forwarding_base() {}
        graph& graph_ref;
    };

    struct queueing_forwarding_base : forwarding_base {
        using forwarding_base::forwarding_base;
        // decrement_port_count may create a forwarding task.  If we cannot handle the task
        // ourselves, ask decrement_port_count to deal with it.
        virtual graph_task* decrement_port_count(bool handle_task) = 0;
    };

    struct reserving_forwarding_base : forwarding_base {
        using forwarding_base::forwarding_base;
        // decrement_port_count may create a forwarding task.  If we cannot handle the task
        // ourselves, ask decrement_port_count to deal with it.
        virtual graph_task* decrement_port_count() = 0;
        virtual void increment_port_count() = 0;
    };

    // specialization that lets us keep a copy of the current_key for building results.
    // KeyType can be a reference type.
    template<typename KeyType>
    struct matching_forwarding_base : public forwarding_base {
        typedef typename std::decay<KeyType>::type current_key_type;
        matching_forwarding_base(graph &g) : forwarding_base(g) { }
        virtual graph_task* increment_key_count(current_key_type const & /*t*/) = 0;
        current_key_type current_key; // so ports can refer to FE's desired items
    };

    template< int N >
    struct join_helper {

        template< typename TupleType, typename PortType >
        static inline void set_join_node_pointer(TupleType &my_input, PortType *port) {
            std::get<N-1>( my_input ).set_join_node_pointer(port);
            join_helper<N-1>::set_join_node_pointer( my_input, port );
        }
        template< typename TupleType >
        static inline void consume_reservations( TupleType &my_input ) {
            std::get<N-1>( my_input ).consume();
            join_helper<N-1>::consume_reservations( my_input );
        }

        template< typename TupleType >
        static inline void release_my_reservation( TupleType &my_input ) {
            std::get<N-1>( my_input ).release();
        }

        template <typename TupleType>
        static inline void release_reservations( TupleType &my_input) {
            join_helper<N-1>::release_reservations(my_input);
            release_my_reservation(my_input);
        }

        template< typename InputTuple, typename OutputTuple >
        static inline bool reserve( InputTuple &my_input, OutputTuple &out) {
            if ( !std::get<N-1>( my_input ).reserve( std::get<N-1>( out ) ) ) return false;
            if ( !join_helper<N-1>::reserve( my_input, out ) ) {
                release_my_reservation( my_input );
                return false;
            }
            return true;
        }

        template<typename InputTuple, typename OutputTuple>
        static inline bool get_my_item( InputTuple &my_input, OutputTuple &out) {
            bool res = std::get<N-1>(my_input).get_item(std::get<N-1>(out) ); // may fail
            return join_helper<N-1>::get_my_item(my_input, out) && res;       // do get on other inputs before returning
        }

        template<typename InputTuple, typename OutputTuple>
        static inline bool get_items(InputTuple &my_input, OutputTuple &out) {
            return get_my_item(my_input, out);
        }

        template<typename InputTuple>
        static inline void reset_my_port(InputTuple &my_input) {
            join_helper<N-1>::reset_my_port(my_input);
            std::get<N-1>(my_input).reset_port();
        }

        template<typename InputTuple>
        static inline void reset_ports(InputTuple& my_input) {
            reset_my_port(my_input);
        }

        template<typename InputTuple, typename KeyFuncTuple>
        static inline void set_key_functors(InputTuple &my_input, KeyFuncTuple &my_key_funcs) {
            std::get<N-1>(my_input).set_my_key_func(std::get<N-1>(my_key_funcs));
            std::get<N-1>(my_key_funcs) = nullptr;
            join_helper<N-1>::set_key_functors(my_input, my_key_funcs);
        }

        template< typename KeyFuncTuple>
        static inline void copy_key_functors(KeyFuncTuple &my_inputs, KeyFuncTuple &other_inputs) {
            __TBB_ASSERT(
                std::get<N-1>(other_inputs).get_my_key_func(),
                "key matching join node should not be instantiated without functors."
            );
            std::get<N-1>(my_inputs).set_my_key_func(std::get<N-1>(other_inputs).get_my_key_func()->clone());
            join_helper<N-1>::copy_key_functors(my_inputs, other_inputs);
        }

        template<typename InputTuple>
        static inline void reset_inputs(InputTuple &my_input, reset_flags f) {
            join_helper<N-1>::reset_inputs(my_input, f);
            std::get<N-1>(my_input).reset_receiver(f);
        }
    };  // join_helper<N>

    template< >
    struct join_helper<1> {

        template< typename TupleType, typename PortType >
        static inline void set_join_node_pointer(TupleType &my_input, PortType *port) {
            std::get<0>( my_input ).set_join_node_pointer(port);
        }

        template< typename TupleType >
        static inline void consume_reservations( TupleType &my_input ) {
            std::get<0>( my_input ).consume();
        }

        template< typename TupleType >
        static inline void release_my_reservation( TupleType &my_input ) {
            std::get<0>( my_input ).release();
        }

        template<typename TupleType>
        static inline void release_reservations( TupleType &my_input) {
            release_my_reservation(my_input);
        }

        template< typename InputTuple, typename OutputTuple >
        static inline bool reserve( InputTuple &my_input, OutputTuple &out) {
            return std::get<0>( my_input ).reserve( std::get<0>( out ) );
        }

        template<typename InputTuple, typename OutputTuple>
        static inline bool get_my_item( InputTuple &my_input, OutputTuple &out) {
            return std::get<0>(my_input).get_item(std::get<0>(out));
        }

        template<typename InputTuple, typename OutputTuple>
        static inline bool get_items(InputTuple &my_input, OutputTuple &out) {
            return get_my_item(my_input, out);
        }

        template<typename InputTuple>
        static inline void reset_my_port(InputTuple &my_input) {
            std::get<0>(my_input).reset_port();
        }

        template<typename InputTuple>
        static inline void reset_ports(InputTuple& my_input) {
            reset_my_port(my_input);
        }

        template<typename InputTuple, typename KeyFuncTuple>
        static inline void set_key_functors(InputTuple &my_input, KeyFuncTuple &my_key_funcs) {
            std::get<0>(my_input).set_my_key_func(std::get<0>(my_key_funcs));
            std::get<0>(my_key_funcs) = nullptr;
        }

        template< typename KeyFuncTuple>
        static inline void copy_key_functors(KeyFuncTuple &my_inputs, KeyFuncTuple &other_inputs) {
            __TBB_ASSERT(
                std::get<0>(other_inputs).get_my_key_func(),
                "key matching join node should not be instantiated without functors."
            );
            std::get<0>(my_inputs).set_my_key_func(std::get<0>(other_inputs).get_my_key_func()->clone());
        }
        template<typename InputTuple>
        static inline void reset_inputs(InputTuple &my_input, reset_flags f) {
            std::get<0>(my_input).reset_receiver(f);
        }
    };  // join_helper<1>

    //! The two-phase join port
    template< typename T >
    class reserving_port : public receiver<T> {
    public:
        typedef T input_type;
        typedef typename receiver<input_type>::predecessor_type predecessor_type;

    private:
        // ----------- Aggregator ------------
        enum op_type { reg_pred, rem_pred, res_item, rel_res, con_res
        };
        typedef reserving_port<T> class_type;

        class reserving_port_operation : public aggregated_operation<reserving_port_operation> {
        public:
            char type;
            union {
                T *my_arg;
                predecessor_type *my_pred;
            };
            reserving_port_operation(const T& e, op_type t) :
                type(char(t)), my_arg(const_cast<T*>(&e)) {}
            reserving_port_operation(const predecessor_type &s, op_type t) : type(char(t)),
                my_pred(const_cast<predecessor_type *>(&s)) {}
            reserving_port_operation(op_type t) : type(char(t)) {}
        };

        typedef aggregating_functor<class_type, reserving_port_operation> handler_type;
        friend class aggregating_functor<class_type, reserving_port_operation>;
        aggregator<handler_type, reserving_port_operation> my_aggregator;

        void handle_operations(reserving_port_operation* op_list) {
            reserving_port_operation *current;
            bool was_missing_predecessors = false;
            while(op_list) {
                current = op_list;
                op_list = op_list->next;
                switch(current->type) {
                case reg_pred:
                    was_missing_predecessors = my_predecessors.empty();
                    my_predecessors.add(*(current->my_pred));
                    if ( was_missing_predecessors ) {
                        (void) my_join->decrement_port_count(); // may try to forward
                    }
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                case rem_pred:
                    if ( !my_predecessors.empty() ) {
                        my_predecessors.remove(*(current->my_pred));
                        if ( my_predecessors.empty() ) // was the last predecessor
                            my_join->increment_port_count();
                    }
                    // TODO: consider returning failure if there were no predecessors to remove
                    current->status.store( SUCCEEDED, std::memory_order_release );
                    break;
                case res_item:
                    if ( reserved ) {
                        current->status.store( FAILED, std::memory_order_release);
                    }
                    else if ( my_predecessors.try_reserve( *(current->my_arg) ) ) {
                        reserved = true;
                        current->status.store( SUCCEEDED, std::memory_order_release);
                    } else {
                        if ( my_predecessors.empty() ) {
                            my_join->increment_port_count();
                        }
                        current->status.store( FAILED, std::memory_order_release);
                    }
                    break;
                case rel_res:
                    reserved = false;
                    my_predecessors.try_release( );
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                case con_res:
                    reserved = false;
                    my_predecessors.try_consume( );
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                }
            }
        }

    protected:
        template< typename R, typename B > friend class run_and_put_task;
        template<typename X, typename Y> friend class broadcast_cache;
        template<typename X, typename Y> friend class round_robin_cache;
        graph_task* try_put_task( const T & ) override {
            return nullptr;
        }

        graph& graph_reference() const override {
            return my_join->graph_ref;
        }

    public:

        //! Constructor
        reserving_port() : my_join(nullptr), my_predecessors(this), reserved(false) {
            my_aggregator.initialize_handler(handler_type(this));
        }

        // copy constructor
        reserving_port(const reserving_port& /* other */) = delete;

        void set_join_node_pointer(reserving_forwarding_base *join) {
            my_join = join;
        }

        //! Add a predecessor
        bool register_predecessor( predecessor_type &src ) override {
            reserving_port_operation op_data(src, reg_pred);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        //! Remove a predecessor
        bool remove_predecessor( predecessor_type &src ) override {
            reserving_port_operation op_data(src, rem_pred);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        //! Reserve an item from the port
        bool reserve( T &v ) {
            reserving_port_operation op_data(v, res_item);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        //! Release the port
        void release( ) {
            reserving_port_operation op_data(rel_res);
            my_aggregator.execute(&op_data);
        }

        //! Complete use of the port
        void consume( ) {
            reserving_port_operation op_data(con_res);
            my_aggregator.execute(&op_data);
        }

        void reset_receiver( reset_flags f) {
            if(f & rf_clear_edges) my_predecessors.clear();
            else
            my_predecessors.reset();
            reserved = false;
            __TBB_ASSERT(!(f&rf_clear_edges) || my_predecessors.empty(), "port edges not removed");
        }

    private:
#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
        friend class get_graph_helper;
#endif

        reserving_forwarding_base *my_join;
        reservable_predecessor_cache< T, null_mutex > my_predecessors;
        bool reserved;
    };  // reserving_port

    //! queueing join_port
    template<typename T>
    class queueing_port : public receiver<T>, public item_buffer<T> {
    public:
        typedef T input_type;
        typedef typename receiver<input_type>::predecessor_type predecessor_type;
        typedef queueing_port<T> class_type;

    // ----------- Aggregator ------------
    private:
        enum op_type { get__item, res_port, try__put_task
        };

        class queueing_port_operation : public aggregated_operation<queueing_port_operation> {
        public:
            char type;
            T my_val;
            T* my_arg;
            graph_task* bypass_t;
            // constructor for value parameter
            queueing_port_operation(const T& e, op_type t) :
                type(char(t)), my_val(e), my_arg(nullptr)
                , bypass_t(nullptr)
            {}
            // constructor for pointer parameter
            queueing_port_operation(const T* p, op_type t) :
                type(char(t)), my_arg(const_cast<T*>(p))
                , bypass_t(nullptr)
            {}
            // constructor with no parameter
            queueing_port_operation(op_type t) : type(char(t)), my_arg(nullptr)
                , bypass_t(nullptr)
            {}
        };

        typedef aggregating_functor<class_type, queueing_port_operation> handler_type;
        friend class aggregating_functor<class_type, queueing_port_operation>;
        aggregator<handler_type, queueing_port_operation> my_aggregator;

        void handle_operations(queueing_port_operation* op_list) {
            queueing_port_operation *current;
            bool was_empty;
            while(op_list) {
                current = op_list;
                op_list = op_list->next;
                switch(current->type) {
                case try__put_task: {
                        graph_task* rtask = nullptr;
                        was_empty = this->buffer_empty();
                        this->push_back(current->my_val);
                        if (was_empty) rtask = my_join->decrement_port_count(false);
                        else
                            rtask = SUCCESSFULLY_ENQUEUED;
                        current->bypass_t = rtask;
                        current->status.store( SUCCEEDED, std::memory_order_release);
                    }
                    break;
                case get__item:
                    if(!this->buffer_empty()) {
                        __TBB_ASSERT(current->my_arg, nullptr);
                        *(current->my_arg) = this->front();
                        current->status.store( SUCCEEDED, std::memory_order_release);
                    }
                    else {
                        current->status.store( FAILED, std::memory_order_release);
                    }
                    break;
                case res_port:
                    __TBB_ASSERT(this->my_item_valid(this->my_head), "No item to reset");
                    this->destroy_front();
                    if(this->my_item_valid(this->my_head)) {
                        (void)my_join->decrement_port_count(true);
                    }
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                }
            }
        }
    // ------------ End Aggregator ---------------

    protected:
        template< typename R, typename B > friend class run_and_put_task;
        template<typename X, typename Y> friend class broadcast_cache;
        template<typename X, typename Y> friend class round_robin_cache;
        graph_task* try_put_task(const T &v) override {
            queueing_port_operation op_data(v, try__put_task);
            my_aggregator.execute(&op_data);
            __TBB_ASSERT(op_data.status == SUCCEEDED || !op_data.bypass_t, "inconsistent return from aggregator");
            if(!op_data.bypass_t) return SUCCESSFULLY_ENQUEUED;
            return op_data.bypass_t;
        }

        graph& graph_reference() const override {
            return my_join->graph_ref;
        }

    public:

        //! Constructor
        queueing_port() : item_buffer<T>() {
            my_join = nullptr;
            my_aggregator.initialize_handler(handler_type(this));
        }

        //! copy constructor
        queueing_port(const queueing_port& /* other */) = delete;

        //! record parent for tallying available items
        void set_join_node_pointer(queueing_forwarding_base *join) {
            my_join = join;
        }

        bool get_item( T &v ) {
            queueing_port_operation op_data(&v, get__item);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        // reset_port is called when item is accepted by successor, but
        // is initiated by join_node.
        void reset_port() {
            queueing_port_operation op_data(res_port);
            my_aggregator.execute(&op_data);
            return;
        }

        void reset_receiver(reset_flags) {
            item_buffer<T>::reset();
        }

    private:
#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
        friend class get_graph_helper;
#endif

        queueing_forwarding_base *my_join;
    };  // queueing_port

// included in namespace tbb::flow::interfaceX::internal

// elements in the table are a simple list; we need pointer to next element to
// traverse the chain
template<typename ValueType>
struct buffer_element_type {
    // the second parameter below is void * because we can't forward-declare the type
    // itself, so we just reinterpret_cast below.
    typedef typename aligned_pair<ValueType, void *>::type type;
};

template
    <
     typename Key,         // type of key within ValueType
     typename ValueType,
     typename ValueToKey,  // abstract method that returns "const Key" or "const Key&" given ValueType
     typename HashCompare, // has hash and equal
     typename Allocator=tbb::cache_aligned_allocator< typename aligned_pair<ValueType, void *>::type >
    >
class hash_buffer : public HashCompare {
public:
    static const size_t INITIAL_SIZE = 8;  // initial size of the hash pointer table
    typedef ValueType value_type;
    typedef typename buffer_element_type< value_type >::type element_type;
    typedef value_type *pointer_type;
    typedef element_type *list_array_type;  // array we manage manually
    typedef list_array_type *pointer_array_type;
    typedef typename std::allocator_traits<Allocator>::template rebind_alloc<list_array_type> pointer_array_allocator_type;
    typedef typename std::allocator_traits<Allocator>::template rebind_alloc<element_type> elements_array_allocator;
    typedef typename std::decay<Key>::type Knoref;

private:
    ValueToKey *my_key;
    size_t my_size;
    size_t nelements;
    pointer_array_type pointer_array;    // pointer_array[my_size]
    list_array_type elements_array;      // elements_array[my_size / 2]
    element_type* free_list;

    size_t mask() { return my_size - 1; }

    void set_up_free_list( element_type **p_free_list, list_array_type la, size_t sz) {
        for(size_t i=0; i < sz - 1; ++i ) {  // construct free list
            la[i].second = &(la[i+1]);
        }
        la[sz-1].second = nullptr;
        *p_free_list = (element_type *)&(la[0]);
    }

    // cleanup for exceptions
    struct DoCleanup {
        pointer_array_type *my_pa;
        list_array_type *my_elements;
        size_t my_size;

        DoCleanup(pointer_array_type &pa, list_array_type &my_els, size_t sz) :
            my_pa(&pa), my_elements(&my_els), my_size(sz) {  }
        ~DoCleanup() {
            if(my_pa) {
                size_t dont_care = 0;
                internal_free_buffer(*my_pa, *my_elements, my_size, dont_care);
            }
        }
    };

    // exception-safety requires we do all the potentially-throwing operations first
    void grow_array() {
        size_t new_size = my_size*2;
        size_t new_nelements = nelements;  // internal_free_buffer zeroes this
        list_array_type new_elements_array = nullptr;
        pointer_array_type new_pointer_array = nullptr;
        list_array_type new_free_list = nullptr;
        {
            DoCleanup my_cleanup(new_pointer_array, new_elements_array, new_size);
            new_elements_array = elements_array_allocator().allocate(my_size);
            new_pointer_array = pointer_array_allocator_type().allocate(new_size);
            for(size_t i=0; i < new_size; ++i) new_pointer_array[i] = nullptr;
            set_up_free_list(&new_free_list, new_elements_array, my_size );

            for(size_t i=0; i < my_size; ++i) {
                for( element_type* op = pointer_array[i]; op; op = (element_type *)(op->second)) {
                    value_type *ov = reinterpret_cast<value_type *>(&(op->first));
                    // could have std::move semantics
                    internal_insert_with_key(new_pointer_array, new_size, new_free_list, *ov);
                }
            }
            my_cleanup.my_pa = nullptr;
            my_cleanup.my_elements = nullptr;
        }

        internal_free_buffer(pointer_array, elements_array, my_size, nelements);
        free_list = new_free_list;
        pointer_array = new_pointer_array;
        elements_array = new_elements_array;
        my_size = new_size;
        nelements = new_nelements;
    }

    // v should have perfect forwarding if std::move implemented.
    // we use this method to move elements in grow_array, so can't use class fields
    void internal_insert_with_key( element_type **p_pointer_array, size_t p_sz, list_array_type &p_free_list,
            const value_type &v) {
        size_t l_mask = p_sz-1;
        __TBB_ASSERT(my_key, "Error: value-to-key functor not provided");
        size_t h = this->hash(tbb::detail::invoke(*my_key, v)) & l_mask;
        __TBB_ASSERT(p_free_list, "Error: free list not set up.");
        element_type* my_elem = p_free_list; p_free_list = (element_type *)(p_free_list->second);
        (void) new(&(my_elem->first)) value_type(v);
        my_elem->second = p_pointer_array[h];
        p_pointer_array[h] = my_elem;
    }

    void internal_initialize_buffer() {
        pointer_array = pointer_array_allocator_type().allocate(my_size);
        for(size_t i = 0; i < my_size; ++i) pointer_array[i] = nullptr;
        elements_array = elements_array_allocator().allocate(my_size / 2);
        set_up_free_list(&free_list, elements_array, my_size / 2);
    }

    // made static so an enclosed class can use to properly dispose of the internals
    static void internal_free_buffer( pointer_array_type &pa, list_array_type &el, size_t &sz, size_t &ne ) {
        if(pa) {
            for(size_t i = 0; i < sz; ++i ) {
                element_type *p_next;
                for( element_type *p = pa[i]; p; p = p_next) {
                    p_next = (element_type *)p->second;
                    // TODO revamp: make sure type casting is correct.
                    void* ptr = (void*)(p->first);
#if _MSC_VER && _MSC_VER <= 1900 && !__INTEL_COMPILER
                    suppress_unused_warning(ptr);
#endif
                    ((value_type*)ptr)->~value_type();
                }
            }
            pointer_array_allocator_type().deallocate(pa, sz);
            pa = nullptr;
        }
        // Separate test (if allocation of pa throws, el may be allocated.
        // but no elements will be constructed.)
        if(el) {
            elements_array_allocator().deallocate(el, sz / 2);
            el = nullptr;
        }
        sz = INITIAL_SIZE;
        ne = 0;
    }

public:
    hash_buffer() : my_key(nullptr), my_size(INITIAL_SIZE), nelements(0) {
        internal_initialize_buffer();
    }

    ~hash_buffer() {
        internal_free_buffer(pointer_array, elements_array, my_size, nelements);
        delete my_key;
        my_key = nullptr;
    }
    hash_buffer(const hash_buffer&) = delete;
    hash_buffer& operator=(const hash_buffer&) = delete;

    void reset() {
        internal_free_buffer(pointer_array, elements_array, my_size, nelements);
        internal_initialize_buffer();
    }

    // Take ownership of func object allocated with new.
    // This method is only used internally, so can't be misused by user.
    void set_key_func(ValueToKey *vtk) { my_key = vtk; }
    // pointer is used to clone()
    ValueToKey* get_key_func() { return my_key; }

    bool insert_with_key(const value_type &v) {
        pointer_type p = nullptr;
        __TBB_ASSERT(my_key, "Error: value-to-key functor not provided");
        if(find_ref_with_key(tbb::detail::invoke(*my_key, v), p)) {
            p->~value_type();
            (void) new(p) value_type(v);  // copy-construct into the space
            return false;
        }
        ++nelements;
        if(nelements*2 > my_size) grow_array();
        internal_insert_with_key(pointer_array, my_size, free_list, v);
        return true;
    }

    // returns true and sets v to array element if found, else returns false.
    bool find_ref_with_key(const Knoref& k, pointer_type &v) {
        size_t i = this->hash(k) & mask();
        for(element_type* p = pointer_array[i]; p; p = (element_type *)(p->second)) {
            pointer_type pv = reinterpret_cast<pointer_type>(&(p->first));
            __TBB_ASSERT(my_key, "Error: value-to-key functor not provided");
            if(this->equal(tbb::detail::invoke(*my_key, *pv), k)) {
                v = pv;
                return true;
            }
        }
        return false;
    }

    bool find_with_key( const Knoref& k, value_type &v) {
        value_type *p;
        if(find_ref_with_key(k, p)) {
            v = *p;
            return true;
        }
        else
            return false;
    }

    void delete_with_key(const Knoref& k) {
        size_t h = this->hash(k) & mask();
        element_type* prev = nullptr;
        for(element_type* p = pointer_array[h]; p; prev = p, p = (element_type *)(p->second)) {
            value_type *vp = reinterpret_cast<value_type *>(&(p->first));
            __TBB_ASSERT(my_key, "Error: value-to-key functor not provided");
            if(this->equal(tbb::detail::invoke(*my_key, *vp), k)) {
                vp->~value_type();
                if(prev) prev->second = p->second;
                else pointer_array[h] = (element_type *)(p->second);
                p->second = free_list;
                free_list = p;
                --nelements;
                return;
            }
        }
        __TBB_ASSERT(false, "key not found for delete");
    }
};

    template<typename K>
    struct count_element {
        K my_key;
        size_t my_value;
    };

    // method to access the key in the counting table
    // the ref has already been removed from K
    template< typename K >
    struct key_to_count_functor {
        typedef count_element<K> table_item_type;
        const K& operator()(const table_item_type& v) { return v.my_key; }
    };

    // the ports can have only one template parameter.  We wrap the types needed in
    // a traits type
    template< class TraitsType >
    class key_matching_port :
        public receiver<typename TraitsType::T>,
        public hash_buffer< typename TraitsType::K, typename TraitsType::T, typename TraitsType::TtoK,
                typename TraitsType::KHash > {
    public:
        typedef TraitsType traits;
        typedef key_matching_port<traits> class_type;
        typedef typename TraitsType::T input_type;
        typedef typename TraitsType::K key_type;
        typedef typename std::decay<key_type>::type noref_key_type;
        typedef typename receiver<input_type>::predecessor_type predecessor_type;
        typedef typename TraitsType::TtoK type_to_key_func_type;
        typedef typename TraitsType::KHash hash_compare_type;
        typedef hash_buffer< key_type, input_type, type_to_key_func_type, hash_compare_type > buffer_type;

    private:
// ----------- Aggregator ------------
    private:
        enum op_type { try__put, get__item, res_port
        };

        class key_matching_port_operation : public aggregated_operation<key_matching_port_operation> {
        public:
            char type;
            input_type my_val;
            input_type *my_arg;
            // constructor for value parameter
            key_matching_port_operation(const input_type& e, op_type t) :
                type(char(t)), my_val(e), my_arg(nullptr) {}
            // constructor for pointer parameter
            key_matching_port_operation(const input_type* p, op_type t) :
                type(char(t)), my_arg(const_cast<input_type*>(p)) {}
            // constructor with no parameter
            key_matching_port_operation(op_type t) : type(char(t)), my_arg(nullptr) {}
        };

        typedef aggregating_functor<class_type, key_matching_port_operation> handler_type;
        friend class aggregating_functor<class_type, key_matching_port_operation>;
        aggregator<handler_type, key_matching_port_operation> my_aggregator;

        void handle_operations(key_matching_port_operation* op_list) {
            key_matching_port_operation *current;
            while(op_list) {
                current = op_list;
                op_list = op_list->next;
                switch(current->type) {
                case try__put: {
                        bool was_inserted = this->insert_with_key(current->my_val);
                        // return failure if a duplicate insertion occurs
                        current->status.store( was_inserted ? SUCCEEDED : FAILED, std::memory_order_release);
                    }
                    break;
                case get__item:
                    // use current_key from FE for item
                    __TBB_ASSERT(current->my_arg, nullptr);
                    if(!this->find_with_key(my_join->current_key, *(current->my_arg))) {
                        __TBB_ASSERT(false, "Failed to find item corresponding to current_key.");
                    }
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                case res_port:
                    // use current_key from FE for item
                    this->delete_with_key(my_join->current_key);
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                }
            }
        }
// ------------ End Aggregator ---------------
    protected:
        template< typename R, typename B > friend class run_and_put_task;
        template<typename X, typename Y> friend class broadcast_cache;
        template<typename X, typename Y> friend class round_robin_cache;
        graph_task* try_put_task(const input_type& v) override {
            key_matching_port_operation op_data(v, try__put);
            graph_task* rtask = nullptr;
            my_aggregator.execute(&op_data);
            if(op_data.status == SUCCEEDED) {
                rtask = my_join->increment_key_count((*(this->get_key_func()))(v));  // may spawn
                // rtask has to reflect the return status of the try_put
                if(!rtask) rtask = SUCCESSFULLY_ENQUEUED;
            }
            return rtask;
        }

        graph& graph_reference() const override {
            return my_join->graph_ref;
        }

    public:

        key_matching_port() : receiver<input_type>(), buffer_type() {
            my_join = nullptr;
            my_aggregator.initialize_handler(handler_type(this));
        }

        // copy constructor
        key_matching_port(const key_matching_port& /*other*/) = delete;
#if __INTEL_COMPILER <= 2021
        // Suppress superfluous diagnostic about virtual keyword absence in a destructor of an inherited
        // class while the parent class has the virtual keyword for the destrocutor.
        virtual
#endif
        ~key_matching_port() { }

        void set_join_node_pointer(forwarding_base *join) {
            my_join = dynamic_cast<matching_forwarding_base<key_type>*>(join);
        }

        void set_my_key_func(type_to_key_func_type *f) { this->set_key_func(f); }

        type_to_key_func_type* get_my_key_func() { return this->get_key_func(); }

        bool get_item( input_type &v ) {
            // aggregator uses current_key from FE for Key
            key_matching_port_operation op_data(&v, get__item);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        // reset_port is called when item is accepted by successor, but
        // is initiated by join_node.
        void reset_port() {
            key_matching_port_operation op_data(res_port);
            my_aggregator.execute(&op_data);
            return;
        }

        void reset_receiver(reset_flags ) {
            buffer_type::reset();
        }

    private:
        // my_join forwarding base used to count number of inputs that
        // received key.
        matching_forwarding_base<key_type> *my_join;
    };  // key_matching_port

    using namespace graph_policy_namespace;

    template<typename JP, typename InputTuple, typename OutputTuple>
    class join_node_base;

    //! join_node_FE : implements input port policy
    template<typename JP, typename InputTuple, typename OutputTuple>
    class join_node_FE;

    template<typename InputTuple, typename OutputTuple>
    class join_node_FE<reserving, InputTuple, OutputTuple> : public reserving_forwarding_base {
    private:
        static const int N = std::tuple_size<OutputTuple>::value;
        typedef OutputTuple output_type;
        typedef InputTuple input_type;
        typedef join_node_base<reserving, InputTuple, OutputTuple> base_node_type; // for forwarding
    public:
        join_node_FE(graph &g) : reserving_forwarding_base(g), my_node(nullptr) {
            ports_with_no_inputs = N;
            join_helper<N>::set_join_node_pointer(my_inputs, this);
        }

        join_node_FE(const join_node_FE& other) : reserving_forwarding_base((other.reserving_forwarding_base::graph_ref)), my_node(nullptr) {
            ports_with_no_inputs = N;
            join_helper<N>::set_join_node_pointer(my_inputs, this);
        }

        void set_my_node(base_node_type *new_my_node) { my_node = new_my_node; }

       void increment_port_count() override {
            ++ports_with_no_inputs;
        }

        // if all input_ports have predecessors, spawn forward to try and consume tuples
        graph_task* decrement_port_count() override {
            if(ports_with_no_inputs.fetch_sub(1) == 1) {
                if(is_graph_active(this->graph_ref)) {
                    small_object_allocator allocator{};
                    typedef forward_task_bypass<base_node_type> task_type;
                    graph_task* t = allocator.new_object<task_type>(graph_ref, allocator, *my_node);
                    graph_ref.reserve_wait();
                    spawn_in_graph_arena(this->graph_ref, *t);
                }
            }
            return nullptr;
        }

        input_type &input_ports() { return my_inputs; }

    protected:

        void reset(  reset_flags f) {
            // called outside of parallel contexts
            ports_with_no_inputs = N;
            join_helper<N>::reset_inputs(my_inputs, f);
        }

        // all methods on input ports should be called under mutual exclusion from join_node_base.

        bool tuple_build_may_succeed() {
            return !ports_with_no_inputs;
        }

        bool try_to_make_tuple(output_type &out) {
            if(ports_with_no_inputs) return false;
            return join_helper<N>::reserve(my_inputs, out);
        }

        void tuple_accepted() {
            join_helper<N>::consume_reservations(my_inputs);
        }
        void tuple_rejected() {
            join_helper<N>::release_reservations(my_inputs);
        }

        input_type my_inputs;
        base_node_type *my_node;
        std::atomic<std::size_t> ports_with_no_inputs;
    };  // join_node_FE<reserving, ... >

    template<typename InputTuple, typename OutputTuple>
    class join_node_FE<queueing, InputTuple, OutputTuple> : public queueing_forwarding_base {
    public:
        static const int N = std::tuple_size<OutputTuple>::value;
        typedef OutputTuple output_type;
        typedef InputTuple input_type;
        typedef join_node_base<queueing, InputTuple, OutputTuple> base_node_type; // for forwarding

        join_node_FE(graph &g) : queueing_forwarding_base(g), my_node(nullptr) {
            ports_with_no_items = N;
            join_helper<N>::set_join_node_pointer(my_inputs, this);
        }

        join_node_FE(const join_node_FE& other) : queueing_forwarding_base((other.queueing_forwarding_base::graph_ref)), my_node(nullptr) {
            ports_with_no_items = N;
            join_helper<N>::set_join_node_pointer(my_inputs, this);
        }

        // needed for forwarding
        void set_my_node(base_node_type *new_my_node) { my_node = new_my_node; }

        void reset_port_count() {
            ports_with_no_items = N;
        }

        // if all input_ports have items, spawn forward to try and consume tuples
        graph_task* decrement_port_count(bool handle_task) override
        {
            if(ports_with_no_items.fetch_sub(1) == 1) {
                if(is_graph_active(this->graph_ref)) {
                    small_object_allocator allocator{};
                    typedef forward_task_bypass<base_node_type> task_type;
                    graph_task* t = allocator.new_object<task_type>(graph_ref, allocator, *my_node);
                    graph_ref.reserve_wait();
                    if( !handle_task )
                        return t;
                    spawn_in_graph_arena(this->graph_ref, *t);
                }
            }
            return nullptr;
        }

        input_type &input_ports() { return my_inputs; }

    protected:

        void reset(  reset_flags f) {
            reset_port_count();
            join_helper<N>::reset_inputs(my_inputs, f );
        }

        // all methods on input ports should be called under mutual exclusion from join_node_base.

        bool tuple_build_may_succeed() {
            return !ports_with_no_items;
        }

        bool try_to_make_tuple(output_type &out) {
            if(ports_with_no_items) return false;
            return join_helper<N>::get_items(my_inputs, out);
        }

        void tuple_accepted() {
            reset_port_count();
            join_helper<N>::reset_ports(my_inputs);
        }
        void tuple_rejected() {
            // nothing to do.
        }

        input_type my_inputs;
        base_node_type *my_node;
        std::atomic<std::size_t> ports_with_no_items;
    };  // join_node_FE<queueing, ...>

    // key_matching join front-end.
    template<typename InputTuple, typename OutputTuple, typename K, typename KHash>
    class join_node_FE<key_matching<K,KHash>, InputTuple, OutputTuple> : public matching_forwarding_base<K>,
             // buffer of key value counts
              public hash_buffer<   // typedefed below to key_to_count_buffer_type
                  typename std::decay<K>::type&,        // force ref type on K
                  count_element<typename std::decay<K>::type>,
                  type_to_key_function_body<
                      count_element<typename std::decay<K>::type>,
                      typename std::decay<K>::type& >,
                  KHash >,
             // buffer of output items
             public item_buffer<OutputTuple> {
    public:
        static const int N = std::tuple_size<OutputTuple>::value;
        typedef OutputTuple output_type;
        typedef InputTuple input_type;
        typedef K key_type;
        typedef typename std::decay<key_type>::type unref_key_type;
        typedef KHash key_hash_compare;
        // must use K without ref.
        typedef count_element<unref_key_type> count_element_type;
        // method that lets us refer to the key of this type.
        typedef key_to_count_functor<unref_key_type> key_to_count_func;
        typedef type_to_key_function_body< count_element_type, unref_key_type&> TtoK_function_body_type;
        typedef type_to_key_function_body_leaf<count_element_type, unref_key_type&, key_to_count_func> TtoK_function_body_leaf_type;
        // this is the type of the special table that keeps track of the number of discrete
        // elements corresponding to each key that we've seen.
        typedef hash_buffer< unref_key_type&, count_element_type, TtoK_function_body_type, key_hash_compare >
                 key_to_count_buffer_type;
        typedef item_buffer<output_type> output_buffer_type;
        typedef join_node_base<key_matching<key_type,key_hash_compare>, InputTuple, OutputTuple> base_node_type; // for forwarding
        typedef matching_forwarding_base<key_type> forwarding_base_type;

// ----------- Aggregator ------------
        // the aggregator is only needed to serialize the access to the hash table.
        // and the output_buffer_type base class
    private:
        enum op_type { res_count, inc_count, may_succeed, try_make };
        typedef join_node_FE<key_matching<key_type,key_hash_compare>, InputTuple, OutputTuple> class_type;

        class key_matching_FE_operation : public aggregated_operation<key_matching_FE_operation> {
        public:
            char type;
            unref_key_type my_val;
            output_type* my_output;
            graph_task* bypass_t;
            // constructor for value parameter
            key_matching_FE_operation(const unref_key_type& e , op_type t) : type(char(t)), my_val(e),
                 my_output(nullptr), bypass_t(nullptr) {}
            key_matching_FE_operation(output_type *p, op_type t) : type(char(t)), my_output(p), bypass_t(nullptr) {}
            // constructor with no parameter
            key_matching_FE_operation(op_type t) : type(char(t)), my_output(nullptr), bypass_t(nullptr) {}
        };

        typedef aggregating_functor<class_type, key_matching_FE_operation> handler_type;
        friend class aggregating_functor<class_type, key_matching_FE_operation>;
        aggregator<handler_type, key_matching_FE_operation> my_aggregator;

        // called from aggregator, so serialized
        // returns a task pointer if the a task would have been enqueued but we asked that
        // it be returned.  Otherwise returns nullptr.
        graph_task* fill_output_buffer(unref_key_type &t) {
            output_type l_out;
            graph_task* rtask = nullptr;
            bool do_fwd = this->buffer_empty() && is_graph_active(this->graph_ref);
            this->current_key = t;
            this->delete_with_key(this->current_key);   // remove the key
            if(join_helper<N>::get_items(my_inputs, l_out)) {  //  <== call back
                this->push_back(l_out);
                if(do_fwd) {  // we enqueue if receiving an item from predecessor, not if successor asks for item
                    small_object_allocator allocator{};
                    typedef forward_task_bypass<base_node_type> task_type;
                    rtask = allocator.new_object<task_type>(this->graph_ref, allocator, *my_node);
                    this->graph_ref.reserve_wait();
                    do_fwd = false;
                }
                // retire the input values
                join_helper<N>::reset_ports(my_inputs);  //  <== call back
            }
            else {
                __TBB_ASSERT(false, "should have had something to push");
            }
            return rtask;
        }

        void handle_operations(key_matching_FE_operation* op_list) {
            key_matching_FE_operation *current;
            while(op_list) {
                current = op_list;
                op_list = op_list->next;
                switch(current->type) {
                case res_count:  // called from BE
                    {
                        this->destroy_front();
                        current->status.store( SUCCEEDED, std::memory_order_release);
                    }
                    break;
                case inc_count: {  // called from input ports
                        count_element_type *p = nullptr;
                        unref_key_type &t = current->my_val;
                        if(!(this->find_ref_with_key(t,p))) {
                            count_element_type ev;
                            ev.my_key = t;
                            ev.my_value = 0;
                            this->insert_with_key(ev);
                            bool found = this->find_ref_with_key(t, p);
                            __TBB_ASSERT_EX(found, "should find key after inserting it");
                        }
                        if(++(p->my_value) == size_t(N)) {
                            current->bypass_t = fill_output_buffer(t);
                        }
                    }
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                case may_succeed:  // called from BE
                    current->status.store( this->buffer_empty() ? FAILED : SUCCEEDED, std::memory_order_release);
                    break;
                case try_make:  // called from BE
                    if(this->buffer_empty()) {
                        current->status.store( FAILED, std::memory_order_release);
                    }
                    else {
                        *(current->my_output) = this->front();
                        current->status.store( SUCCEEDED, std::memory_order_release);
                    }
                    break;
                }
            }
        }
// ------------ End Aggregator ---------------

    public:
        template<typename FunctionTuple>
        join_node_FE(graph &g, FunctionTuple &TtoK_funcs) : forwarding_base_type(g), my_node(nullptr) {
            join_helper<N>::set_join_node_pointer(my_inputs, this);
            join_helper<N>::set_key_functors(my_inputs, TtoK_funcs);
            my_aggregator.initialize_handler(handler_type(this));
                    TtoK_function_body_type *cfb = new TtoK_function_body_leaf_type(key_to_count_func());
            this->set_key_func(cfb);
        }

        join_node_FE(const join_node_FE& other) : forwarding_base_type((other.forwarding_base_type::graph_ref)), key_to_count_buffer_type(),
        output_buffer_type() {
            my_node = nullptr;
            join_helper<N>::set_join_node_pointer(my_inputs, this);
            join_helper<N>::copy_key_functors(my_inputs, const_cast<input_type &>(other.my_inputs));
            my_aggregator.initialize_handler(handler_type(this));
            TtoK_function_body_type *cfb = new TtoK_function_body_leaf_type(key_to_count_func());
            this->set_key_func(cfb);
        }

        // needed for forwarding
        void set_my_node(base_node_type *new_my_node) { my_node = new_my_node; }

        void reset_port_count() {  // called from BE
            key_matching_FE_operation op_data(res_count);
            my_aggregator.execute(&op_data);
            return;
        }

        // if all input_ports have items, spawn forward to try and consume tuples
        // return a task if we are asked and did create one.
        graph_task *increment_key_count(unref_key_type const & t) override {  // called from input_ports
            key_matching_FE_operation op_data(t, inc_count);
            my_aggregator.execute(&op_data);
            return op_data.bypass_t;
        }

        input_type &input_ports() { return my_inputs; }

    protected:

        void reset(  reset_flags f ) {
            // called outside of parallel contexts
            join_helper<N>::reset_inputs(my_inputs, f);

            key_to_count_buffer_type::reset();
            output_buffer_type::reset();
        }

        // all methods on input ports should be called under mutual exclusion from join_node_base.

        bool tuple_build_may_succeed() {  // called from back-end
            key_matching_FE_operation op_data(may_succeed);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        // cannot lock while calling back to input_ports.  current_key will only be set
        // and reset under the aggregator, so it will remain consistent.
        bool try_to_make_tuple(output_type &out) {
            key_matching_FE_operation op_data(&out,try_make);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        void tuple_accepted() {
            reset_port_count();  // reset current_key after ports reset.
        }

        void tuple_rejected() {
            // nothing to do.
        }

        input_type my_inputs;  // input ports
        base_node_type *my_node;
    }; // join_node_FE<key_matching<K,KHash>, InputTuple, OutputTuple>

    //! join_node_base
    template<typename JP, typename InputTuple, typename OutputTuple>
    class join_node_base : public graph_node, public join_node_FE<JP, InputTuple, OutputTuple>,
                           public sender<OutputTuple> {
    protected:
        using graph_node::my_graph;
    public:
        typedef OutputTuple output_type;

        typedef typename sender<output_type>::successor_type successor_type;
        typedef join_node_FE<JP, InputTuple, OutputTuple> input_ports_type;
        using input_ports_type::tuple_build_may_succeed;
        using input_ports_type::try_to_make_tuple;
        using input_ports_type::tuple_accepted;
        using input_ports_type::tuple_rejected;

    private:
        // ----------- Aggregator ------------
        enum op_type { reg_succ, rem_succ, try__get, do_fwrd, do_fwrd_bypass
        };
        typedef join_node_base<JP,InputTuple,OutputTuple> class_type;

        class join_node_base_operation : public aggregated_operation<join_node_base_operation> {
        public:
            char type;
            union {
                output_type *my_arg;
                successor_type *my_succ;
            };
            graph_task* bypass_t;
            join_node_base_operation(const output_type& e, op_type t) : type(char(t)),
                my_arg(const_cast<output_type*>(&e)), bypass_t(nullptr) {}
            join_node_base_operation(const successor_type &s, op_type t) : type(char(t)),
                my_succ(const_cast<successor_type *>(&s)), bypass_t(nullptr) {}
            join_node_base_operation(op_type t) : type(char(t)), bypass_t(nullptr) {}
        };

        typedef aggregating_functor<class_type, join_node_base_operation> handler_type;
        friend class aggregating_functor<class_type, join_node_base_operation>;
        bool forwarder_busy;
        aggregator<handler_type, join_node_base_operation> my_aggregator;

        void handle_operations(join_node_base_operation* op_list) {
            join_node_base_operation *current;
            while(op_list) {
                current = op_list;
                op_list = op_list->next;
                switch(current->type) {
                case reg_succ: {
                        my_successors.register_successor(*(current->my_succ));
                        if(tuple_build_may_succeed() && !forwarder_busy && is_graph_active(my_graph)) {
                            small_object_allocator allocator{};
                            typedef forward_task_bypass< join_node_base<JP, InputTuple, OutputTuple> > task_type;
                            graph_task* t = allocator.new_object<task_type>(my_graph, allocator, *this);
                            my_graph.reserve_wait();
                            spawn_in_graph_arena(my_graph, *t);
                            forwarder_busy = true;
                        }
                        current->status.store( SUCCEEDED, std::memory_order_release);
                    }
                    break;
                case rem_succ:
                    my_successors.remove_successor(*(current->my_succ));
                    current->status.store( SUCCEEDED, std::memory_order_release);
                    break;
                case try__get:
                    if(tuple_build_may_succeed()) {
                        if(try_to_make_tuple(*(current->my_arg))) {
                            tuple_accepted();
                            current->status.store( SUCCEEDED, std::memory_order_release);
                        }
                        else current->status.store( FAILED, std::memory_order_release);
                    }
                    else current->status.store( FAILED, std::memory_order_release);
                    break;
                case do_fwrd_bypass: {
                        bool build_succeeded;
                        graph_task *last_task = nullptr;
                        output_type out;
                        // forwarding must be exclusive, because try_to_make_tuple and tuple_accepted
                        // are separate locked methods in the FE.  We could conceivably fetch the front
                        // of the FE queue, then be swapped out, have someone else consume the FE's
                        // object, then come back, forward, and then try to remove it from the queue
                        // again. Without reservation of the FE, the methods accessing it must be locked.
                        // We could remember the keys of the objects we forwarded, and then remove
                        // them from the input ports after forwarding is complete?
                        if(tuple_build_may_succeed()) {  // checks output queue of FE
                            do {
                                build_succeeded = try_to_make_tuple(out);  // fetch front_end of queue
                                if(build_succeeded) {
                                    graph_task *new_task = my_successors.try_put_task(out);
                                    last_task = combine_tasks(my_graph, last_task, new_task);
                                    if(new_task) {
                                        tuple_accepted();
                                    }
                                    else {
                                        tuple_rejected();
                                        build_succeeded = false;
                                    }
                                }
                            } while(build_succeeded);
                        }
                        current->bypass_t = last_task;
                        current->status.store( SUCCEEDED, std::memory_order_release);
                        forwarder_busy = false;
                    }
                    break;
                }
            }
        }
        // ---------- end aggregator -----------
    public:
        join_node_base(graph &g)
            : graph_node(g), input_ports_type(g), forwarder_busy(false), my_successors(this)
        {
            input_ports_type::set_my_node(this);
            my_aggregator.initialize_handler(handler_type(this));
        }

        join_node_base(const join_node_base& other) :
            graph_node(other.graph_node::my_graph), input_ports_type(other),
            sender<OutputTuple>(), forwarder_busy(false), my_successors(this)
        {
            input_ports_type::set_my_node(this);
            my_aggregator.initialize_handler(handler_type(this));
        }

        template<typename FunctionTuple>
        join_node_base(graph &g, FunctionTuple f)
            : graph_node(g), input_ports_type(g, f), forwarder_busy(false), my_successors(this)
        {
            input_ports_type::set_my_node(this);
            my_aggregator.initialize_handler(handler_type(this));
        }

        bool register_successor(successor_type &r) override {
            join_node_base_operation op_data(r, reg_succ);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        bool remove_successor( successor_type &r) override {
            join_node_base_operation op_data(r, rem_succ);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

        bool try_get( output_type &v) override {
            join_node_base_operation op_data(v, try__get);
            my_aggregator.execute(&op_data);
            return op_data.status == SUCCEEDED;
        }

    protected:
        void reset_node(reset_flags f) override {
            input_ports_type::reset(f);
            if(f & rf_clear_edges) my_successors.clear();
        }

    private:
        broadcast_cache<output_type, null_rw_mutex> my_successors;

        friend class forward_task_bypass< join_node_base<JP, InputTuple, OutputTuple> >;
        graph_task *forward_task() {
            join_node_base_operation op_data(do_fwrd_bypass);
            my_aggregator.execute(&op_data);
            return op_data.bypass_t;
        }

    };  // join_node_base

    // join base class type generator
    template<int N, template<class> class PT, typename OutputTuple, typename JP>
    struct join_base {
        typedef join_node_base<JP, typename wrap_tuple_elements<N,PT,OutputTuple>::type, OutputTuple> type;
    };

    template<int N, typename OutputTuple, typename K, typename KHash>
    struct join_base<N, key_matching_port, OutputTuple, key_matching<K,KHash> > {
        typedef key_matching<K, KHash> key_traits_type;
        typedef K key_type;
        typedef KHash key_hash_compare;
        typedef join_node_base< key_traits_type,
                // ports type
                typename wrap_key_tuple_elements<N,key_matching_port,key_traits_type,OutputTuple>::type,
                OutputTuple > type;
    };

    //! unfolded_join_node : passes input_ports_type to join_node_base.  We build the input port type
    //  using tuple_element.  The class PT is the port type (reserving_port, queueing_port, key_matching_port)
    //  and should match the typename.

    template<int M, template<class> class PT, typename OutputTuple, typename JP>
    class unfolded_join_node : public join_base<M,PT,OutputTuple,JP>::type {
    public:
        typedef typename wrap_tuple_elements<M, PT, OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<JP, input_ports_type, output_type > base_type;
    public:
        unfolded_join_node(graph &g) : base_type(g) {}
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };

#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
    template <typename K, typename T>
    struct key_from_message_body {
        K operator()(const T& t) const {
            return key_from_message<K>(t);
        }
    };
    // Adds const to reference type
    template <typename K, typename T>
    struct key_from_message_body<K&,T> {
        const K& operator()(const T& t) const {
            return key_from_message<const K&>(t);
        }
    };
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
    // key_matching unfolded_join_node.  This must be a separate specialization because the constructors
    // differ.

    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<2,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<2,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
    public:
        typedef typename wrap_key_tuple_elements<2,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash>, input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef std::tuple< f0_p, f1_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 2, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };

    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<3,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<3,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
    public:
        typedef typename wrap_key_tuple_elements<3,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash>, input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef std::tuple< f0_p, f1_p, f2_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 3, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };

    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<4,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<4,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
        typedef typename std::tuple_element<3, OutputTuple>::type T3;
    public:
        typedef typename wrap_key_tuple_elements<4,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash>, input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef type_to_key_function_body<T3, K> *f3_p;
        typedef std::tuple< f0_p, f1_p, f2_p, f3_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>()),
                    new type_to_key_function_body_leaf<T3, K, key_from_message_body<K,T3> >(key_from_message_body<K,T3>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2, typename Body3>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2, Body3 body3) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2),
                    new type_to_key_function_body_leaf<T3, K, Body3>(body3)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 4, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };

    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<5,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<5,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
        typedef typename std::tuple_element<3, OutputTuple>::type T3;
        typedef typename std::tuple_element<4, OutputTuple>::type T4;
    public:
        typedef typename wrap_key_tuple_elements<5,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash> , input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef type_to_key_function_body<T3, K> *f3_p;
        typedef type_to_key_function_body<T4, K> *f4_p;
        typedef std::tuple< f0_p, f1_p, f2_p, f3_p, f4_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>()),
                    new type_to_key_function_body_leaf<T3, K, key_from_message_body<K,T3> >(key_from_message_body<K,T3>()),
                    new type_to_key_function_body_leaf<T4, K, key_from_message_body<K,T4> >(key_from_message_body<K,T4>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2, typename Body3, typename Body4>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2, Body3 body3, Body4 body4) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2),
                    new type_to_key_function_body_leaf<T3, K, Body3>(body3),
                    new type_to_key_function_body_leaf<T4, K, Body4>(body4)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 5, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };

#if __TBB_VARIADIC_MAX >= 6
    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<6,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<6,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
        typedef typename std::tuple_element<3, OutputTuple>::type T3;
        typedef typename std::tuple_element<4, OutputTuple>::type T4;
        typedef typename std::tuple_element<5, OutputTuple>::type T5;
    public:
        typedef typename wrap_key_tuple_elements<6,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash> , input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef type_to_key_function_body<T3, K> *f3_p;
        typedef type_to_key_function_body<T4, K> *f4_p;
        typedef type_to_key_function_body<T5, K> *f5_p;
        typedef std::tuple< f0_p, f1_p, f2_p, f3_p, f4_p, f5_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>()),
                    new type_to_key_function_body_leaf<T3, K, key_from_message_body<K,T3> >(key_from_message_body<K,T3>()),
                    new type_to_key_function_body_leaf<T4, K, key_from_message_body<K,T4> >(key_from_message_body<K,T4>()),
                    new type_to_key_function_body_leaf<T5, K, key_from_message_body<K,T5> >(key_from_message_body<K,T5>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2, typename Body3, typename Body4, typename Body5>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2, Body3 body3, Body4 body4, Body5 body5)
                : base_type(g, func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2),
                    new type_to_key_function_body_leaf<T3, K, Body3>(body3),
                    new type_to_key_function_body_leaf<T4, K, Body4>(body4),
                    new type_to_key_function_body_leaf<T5, K, Body5>(body5)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 6, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };
#endif

#if __TBB_VARIADIC_MAX >= 7
    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<7,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<7,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
        typedef typename std::tuple_element<3, OutputTuple>::type T3;
        typedef typename std::tuple_element<4, OutputTuple>::type T4;
        typedef typename std::tuple_element<5, OutputTuple>::type T5;
        typedef typename std::tuple_element<6, OutputTuple>::type T6;
    public:
        typedef typename wrap_key_tuple_elements<7,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash> , input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef type_to_key_function_body<T3, K> *f3_p;
        typedef type_to_key_function_body<T4, K> *f4_p;
        typedef type_to_key_function_body<T5, K> *f5_p;
        typedef type_to_key_function_body<T6, K> *f6_p;
        typedef std::tuple< f0_p, f1_p, f2_p, f3_p, f4_p, f5_p, f6_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>()),
                    new type_to_key_function_body_leaf<T3, K, key_from_message_body<K,T3> >(key_from_message_body<K,T3>()),
                    new type_to_key_function_body_leaf<T4, K, key_from_message_body<K,T4> >(key_from_message_body<K,T4>()),
                    new type_to_key_function_body_leaf<T5, K, key_from_message_body<K,T5> >(key_from_message_body<K,T5>()),
                    new type_to_key_function_body_leaf<T6, K, key_from_message_body<K,T6> >(key_from_message_body<K,T6>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2, typename Body3, typename Body4,
                 typename Body5, typename Body6>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2, Body3 body3, Body4 body4,
                Body5 body5, Body6 body6) : base_type(g, func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2),
                    new type_to_key_function_body_leaf<T3, K, Body3>(body3),
                    new type_to_key_function_body_leaf<T4, K, Body4>(body4),
                    new type_to_key_function_body_leaf<T5, K, Body5>(body5),
                    new type_to_key_function_body_leaf<T6, K, Body6>(body6)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 7, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };
#endif

#if __TBB_VARIADIC_MAX >= 8
    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<8,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<8,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
        typedef typename std::tuple_element<3, OutputTuple>::type T3;
        typedef typename std::tuple_element<4, OutputTuple>::type T4;
        typedef typename std::tuple_element<5, OutputTuple>::type T5;
        typedef typename std::tuple_element<6, OutputTuple>::type T6;
        typedef typename std::tuple_element<7, OutputTuple>::type T7;
    public:
        typedef typename wrap_key_tuple_elements<8,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash> , input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef type_to_key_function_body<T3, K> *f3_p;
        typedef type_to_key_function_body<T4, K> *f4_p;
        typedef type_to_key_function_body<T5, K> *f5_p;
        typedef type_to_key_function_body<T6, K> *f6_p;
        typedef type_to_key_function_body<T7, K> *f7_p;
        typedef std::tuple< f0_p, f1_p, f2_p, f3_p, f4_p, f5_p, f6_p, f7_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>()),
                    new type_to_key_function_body_leaf<T3, K, key_from_message_body<K,T3> >(key_from_message_body<K,T3>()),
                    new type_to_key_function_body_leaf<T4, K, key_from_message_body<K,T4> >(key_from_message_body<K,T4>()),
                    new type_to_key_function_body_leaf<T5, K, key_from_message_body<K,T5> >(key_from_message_body<K,T5>()),
                    new type_to_key_function_body_leaf<T6, K, key_from_message_body<K,T6> >(key_from_message_body<K,T6>()),
                    new type_to_key_function_body_leaf<T7, K, key_from_message_body<K,T7> >(key_from_message_body<K,T7>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2, typename Body3, typename Body4,
                 typename Body5, typename Body6, typename Body7>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2, Body3 body3, Body4 body4,
                Body5 body5, Body6 body6, Body7 body7) : base_type(g, func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2),
                    new type_to_key_function_body_leaf<T3, K, Body3>(body3),
                    new type_to_key_function_body_leaf<T4, K, Body4>(body4),
                    new type_to_key_function_body_leaf<T5, K, Body5>(body5),
                    new type_to_key_function_body_leaf<T6, K, Body6>(body6),
                    new type_to_key_function_body_leaf<T7, K, Body7>(body7)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 8, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };
#endif

#if __TBB_VARIADIC_MAX >= 9
    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<9,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<9,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
        typedef typename std::tuple_element<3, OutputTuple>::type T3;
        typedef typename std::tuple_element<4, OutputTuple>::type T4;
        typedef typename std::tuple_element<5, OutputTuple>::type T5;
        typedef typename std::tuple_element<6, OutputTuple>::type T6;
        typedef typename std::tuple_element<7, OutputTuple>::type T7;
        typedef typename std::tuple_element<8, OutputTuple>::type T8;
    public:
        typedef typename wrap_key_tuple_elements<9,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash> , input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef type_to_key_function_body<T3, K> *f3_p;
        typedef type_to_key_function_body<T4, K> *f4_p;
        typedef type_to_key_function_body<T5, K> *f5_p;
        typedef type_to_key_function_body<T6, K> *f6_p;
        typedef type_to_key_function_body<T7, K> *f7_p;
        typedef type_to_key_function_body<T8, K> *f8_p;
        typedef std::tuple< f0_p, f1_p, f2_p, f3_p, f4_p, f5_p, f6_p, f7_p, f8_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>()),
                    new type_to_key_function_body_leaf<T3, K, key_from_message_body<K,T3> >(key_from_message_body<K,T3>()),
                    new type_to_key_function_body_leaf<T4, K, key_from_message_body<K,T4> >(key_from_message_body<K,T4>()),
                    new type_to_key_function_body_leaf<T5, K, key_from_message_body<K,T5> >(key_from_message_body<K,T5>()),
                    new type_to_key_function_body_leaf<T6, K, key_from_message_body<K,T6> >(key_from_message_body<K,T6>()),
                    new type_to_key_function_body_leaf<T7, K, key_from_message_body<K,T7> >(key_from_message_body<K,T7>()),
                    new type_to_key_function_body_leaf<T8, K, key_from_message_body<K,T8> >(key_from_message_body<K,T8>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2, typename Body3, typename Body4,
                 typename Body5, typename Body6, typename Body7, typename Body8>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2, Body3 body3, Body4 body4,
                Body5 body5, Body6 body6, Body7 body7, Body8 body8) : base_type(g, func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2),
                    new type_to_key_function_body_leaf<T3, K, Body3>(body3),
                    new type_to_key_function_body_leaf<T4, K, Body4>(body4),
                    new type_to_key_function_body_leaf<T5, K, Body5>(body5),
                    new type_to_key_function_body_leaf<T6, K, Body6>(body6),
                    new type_to_key_function_body_leaf<T7, K, Body7>(body7),
                    new type_to_key_function_body_leaf<T8, K, Body8>(body8)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 9, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };
#endif

#if __TBB_VARIADIC_MAX >= 10
    template<typename OutputTuple, typename K, typename KHash>
    class unfolded_join_node<10,key_matching_port,OutputTuple,key_matching<K,KHash> > : public
            join_base<10,key_matching_port,OutputTuple,key_matching<K,KHash> >::type {
        typedef typename std::tuple_element<0, OutputTuple>::type T0;
        typedef typename std::tuple_element<1, OutputTuple>::type T1;
        typedef typename std::tuple_element<2, OutputTuple>::type T2;
        typedef typename std::tuple_element<3, OutputTuple>::type T3;
        typedef typename std::tuple_element<4, OutputTuple>::type T4;
        typedef typename std::tuple_element<5, OutputTuple>::type T5;
        typedef typename std::tuple_element<6, OutputTuple>::type T6;
        typedef typename std::tuple_element<7, OutputTuple>::type T7;
        typedef typename std::tuple_element<8, OutputTuple>::type T8;
        typedef typename std::tuple_element<9, OutputTuple>::type T9;
    public:
        typedef typename wrap_key_tuple_elements<10,key_matching_port,key_matching<K,KHash>,OutputTuple>::type input_ports_type;
        typedef OutputTuple output_type;
    private:
        typedef join_node_base<key_matching<K,KHash> , input_ports_type, output_type > base_type;
        typedef type_to_key_function_body<T0, K> *f0_p;
        typedef type_to_key_function_body<T1, K> *f1_p;
        typedef type_to_key_function_body<T2, K> *f2_p;
        typedef type_to_key_function_body<T3, K> *f3_p;
        typedef type_to_key_function_body<T4, K> *f4_p;
        typedef type_to_key_function_body<T5, K> *f5_p;
        typedef type_to_key_function_body<T6, K> *f6_p;
        typedef type_to_key_function_body<T7, K> *f7_p;
        typedef type_to_key_function_body<T8, K> *f8_p;
        typedef type_to_key_function_body<T9, K> *f9_p;
        typedef std::tuple< f0_p, f1_p, f2_p, f3_p, f4_p, f5_p, f6_p, f7_p, f8_p, f9_p > func_initializer_type;
    public:
#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
        unfolded_join_node(graph &g) : base_type(g,
                func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, key_from_message_body<K,T0> >(key_from_message_body<K,T0>()),
                    new type_to_key_function_body_leaf<T1, K, key_from_message_body<K,T1> >(key_from_message_body<K,T1>()),
                    new type_to_key_function_body_leaf<T2, K, key_from_message_body<K,T2> >(key_from_message_body<K,T2>()),
                    new type_to_key_function_body_leaf<T3, K, key_from_message_body<K,T3> >(key_from_message_body<K,T3>()),
                    new type_to_key_function_body_leaf<T4, K, key_from_message_body<K,T4> >(key_from_message_body<K,T4>()),
                    new type_to_key_function_body_leaf<T5, K, key_from_message_body<K,T5> >(key_from_message_body<K,T5>()),
                    new type_to_key_function_body_leaf<T6, K, key_from_message_body<K,T6> >(key_from_message_body<K,T6>()),
                    new type_to_key_function_body_leaf<T7, K, key_from_message_body<K,T7> >(key_from_message_body<K,T7>()),
                    new type_to_key_function_body_leaf<T8, K, key_from_message_body<K,T8> >(key_from_message_body<K,T8>()),
                    new type_to_key_function_body_leaf<T9, K, key_from_message_body<K,T9> >(key_from_message_body<K,T9>())
                    ) ) {
        }
#endif /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */
        template<typename Body0, typename Body1, typename Body2, typename Body3, typename Body4,
            typename Body5, typename Body6, typename Body7, typename Body8, typename Body9>
        unfolded_join_node(graph &g, Body0 body0, Body1 body1, Body2 body2, Body3 body3, Body4 body4,
                Body5 body5, Body6 body6, Body7 body7, Body8 body8, Body9 body9) : base_type(g, func_initializer_type(
                    new type_to_key_function_body_leaf<T0, K, Body0>(body0),
                    new type_to_key_function_body_leaf<T1, K, Body1>(body1),
                    new type_to_key_function_body_leaf<T2, K, Body2>(body2),
                    new type_to_key_function_body_leaf<T3, K, Body3>(body3),
                    new type_to_key_function_body_leaf<T4, K, Body4>(body4),
                    new type_to_key_function_body_leaf<T5, K, Body5>(body5),
                    new type_to_key_function_body_leaf<T6, K, Body6>(body6),
                    new type_to_key_function_body_leaf<T7, K, Body7>(body7),
                    new type_to_key_function_body_leaf<T8, K, Body8>(body8),
                    new type_to_key_function_body_leaf<T9, K, Body9>(body9)
                    ) ) {
            static_assert(std::tuple_size<OutputTuple>::value == 10, "wrong number of body initializers");
        }
        unfolded_join_node(const unfolded_join_node &other) : base_type(other) {}
    };
#endif

//! templated function to refer to input ports of the join node
template<size_t N, typename JNT>
typename std::tuple_element<N, typename JNT::input_ports_type>::type &input_port(JNT &jn) {
    return std::get<N>(jn.input_ports());
}

template<typename OutputTuple, typename JP=queueing> class join_node;

template<typename OutputTuple>
class join_node<OutputTuple,reserving>: public unfolded_join_node<std::tuple_size<OutputTuple>::value, reserving_port, OutputTuple, reserving> {
private:
    static const int N = std::tuple_size<OutputTuple>::value;
    typedef unfolded_join_node<N, reserving_port, OutputTuple, reserving> unfolded_type;
public:
    typedef OutputTuple output_type;
    typedef typename unfolded_type::input_ports_type input_ports_type;
     __TBB_NOINLINE_SYM explicit join_node(graph &g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_RESERVING, &this->my_graph,
                                            this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    __TBB_NOINLINE_SYM join_node(const node_set<Args...>& nodes, reserving = reserving()) : join_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    __TBB_NOINLINE_SYM join_node(const join_node &other) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_RESERVING, &this->my_graph,
                                            this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};

template<typename OutputTuple>
class join_node<OutputTuple,queueing>: public unfolded_join_node<std::tuple_size<OutputTuple>::value, queueing_port, OutputTuple, queueing> {
private:
    static const int N = std::tuple_size<OutputTuple>::value;
    typedef unfolded_join_node<N, queueing_port, OutputTuple, queueing> unfolded_type;
public:
    typedef OutputTuple output_type;
    typedef typename unfolded_type::input_ports_type input_ports_type;
     __TBB_NOINLINE_SYM explicit join_node(graph &g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_QUEUEING, &this->my_graph,
                                            this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    __TBB_NOINLINE_SYM join_node(const node_set<Args...>& nodes, queueing = queueing()) : join_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    __TBB_NOINLINE_SYM join_node(const join_node &other) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_QUEUEING, &this->my_graph,
                                            this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};

#if __TBB_CPP20_CONCEPTS_PRESENT
// Helper function which is well-formed only if all of the elements in OutputTuple
// satisfies join_node_function_object<body[i], tuple[i], K>
template <typename OutputTuple, typename K,
          typename... Functions, std::size_t... Idx>
void join_node_function_objects_helper( std::index_sequence<Idx...> )
    requires (std::tuple_size_v<OutputTuple> == sizeof...(Functions)) &&
             (... && join_node_function_object<Functions, std::tuple_element_t<Idx, OutputTuple>, K>);

template <typename OutputTuple, typename K, typename... Functions>
concept join_node_functions = requires {
    join_node_function_objects_helper<OutputTuple, K, Functions...>(std::make_index_sequence<sizeof...(Functions)>{});
};

#endif

// template for key_matching join_node
// tag_matching join_node is a specialization of key_matching, and is source-compatible.
template<typename OutputTuple, typename K, typename KHash>
class join_node<OutputTuple, key_matching<K, KHash> > : public unfolded_join_node<std::tuple_size<OutputTuple>::value,
      key_matching_port, OutputTuple, key_matching<K,KHash> > {
private:
    static const int N = std::tuple_size<OutputTuple>::value;
    typedef unfolded_join_node<N, key_matching_port, OutputTuple, key_matching<K,KHash> > unfolded_type;
public:
    typedef OutputTuple output_type;
    typedef typename unfolded_type::input_ports_type input_ports_type;

#if __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING
    join_node(graph &g) : unfolded_type(g) {}
#endif  /* __TBB_PREVIEW_MESSAGE_BASED_KEY_MATCHING */

    template<typename __TBB_B0, typename __TBB_B1>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1) : unfolded_type(g, b0, b1) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2) : unfolded_type(g, b0, b1, b2) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2, typename __TBB_B3>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2, __TBB_B3>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2, __TBB_B3 b3) : unfolded_type(g, b0, b1, b2, b3) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2, typename __TBB_B3, typename __TBB_B4>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2, __TBB_B3, __TBB_B4>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2, __TBB_B3 b3, __TBB_B4 b4) :
            unfolded_type(g, b0, b1, b2, b3, b4) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
#if __TBB_VARIADIC_MAX >= 6
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2, typename __TBB_B3, typename __TBB_B4,
        typename __TBB_B5>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2, __TBB_B3, __TBB_B4, __TBB_B5>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2, __TBB_B3 b3, __TBB_B4 b4, __TBB_B5 b5) :
            unfolded_type(g, b0, b1, b2, b3, b4, b5) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
#endif
#if __TBB_VARIADIC_MAX >= 7
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2, typename __TBB_B3, typename __TBB_B4,
        typename __TBB_B5, typename __TBB_B6>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2, __TBB_B3, __TBB_B4, __TBB_B5, __TBB_B6>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2, __TBB_B3 b3, __TBB_B4 b4, __TBB_B5 b5, __TBB_B6 b6) :
            unfolded_type(g, b0, b1, b2, b3, b4, b5, b6) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
#endif
#if __TBB_VARIADIC_MAX >= 8
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2, typename __TBB_B3, typename __TBB_B4,
        typename __TBB_B5, typename __TBB_B6, typename __TBB_B7>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2, __TBB_B3, __TBB_B4, __TBB_B5, __TBB_B6, __TBB_B7>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2, __TBB_B3 b3, __TBB_B4 b4, __TBB_B5 b5, __TBB_B6 b6,
            __TBB_B7 b7) : unfolded_type(g, b0, b1, b2, b3, b4, b5, b6, b7) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
#endif
#if __TBB_VARIADIC_MAX >= 9
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2, typename __TBB_B3, typename __TBB_B4,
        typename __TBB_B5, typename __TBB_B6, typename __TBB_B7, typename __TBB_B8>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2, __TBB_B3, __TBB_B4, __TBB_B5, __TBB_B6, __TBB_B7, __TBB_B8>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2, __TBB_B3 b3, __TBB_B4 b4, __TBB_B5 b5, __TBB_B6 b6,
            __TBB_B7 b7, __TBB_B8 b8) : unfolded_type(g, b0, b1, b2, b3, b4, b5, b6, b7, b8) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
#endif
#if __TBB_VARIADIC_MAX >= 10
    template<typename __TBB_B0, typename __TBB_B1, typename __TBB_B2, typename __TBB_B3, typename __TBB_B4,
        typename __TBB_B5, typename __TBB_B6, typename __TBB_B7, typename __TBB_B8, typename __TBB_B9>
        __TBB_requires(join_node_functions<OutputTuple, K, __TBB_B0, __TBB_B1, __TBB_B2, __TBB_B3, __TBB_B4, __TBB_B5, __TBB_B6, __TBB_B7, __TBB_B8, __TBB_B9>)
     __TBB_NOINLINE_SYM join_node(graph &g, __TBB_B0 b0, __TBB_B1 b1, __TBB_B2 b2, __TBB_B3 b3, __TBB_B4 b4, __TBB_B5 b5, __TBB_B6 b6,
            __TBB_B7 b7, __TBB_B8 b8, __TBB_B9 b9) : unfolded_type(g, b0, b1, b2, b3, b4, b5, b6, b7, b8, b9) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
#endif

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <
#if (__clang_major__ == 3 && __clang_minor__ == 4)
        // clang 3.4 misdeduces 'Args...' for 'node_set' while it can cope with template template parameter.
        template<typename...> class node_set,
#endif
        typename... Args, typename... Bodies
    >
    __TBB_requires((sizeof...(Bodies) == 0) || join_node_functions<OutputTuple, K, Bodies...>)
    __TBB_NOINLINE_SYM join_node(const node_set<Args...>& nodes, Bodies... bodies)
        : join_node(nodes.graph_reference(), bodies...) {
        make_edges_in_order(nodes, *this);
    }
#endif // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

    __TBB_NOINLINE_SYM join_node(const join_node &other) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_JOIN_NODE_TAG_MATCHING, &this->my_graph,
                                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};


// TODO: Implement interface with variadic template or tuple
template<typename T0, typename T1=null_type, typename T2=null_type, typename T3=null_type,
                      typename T4=null_type, typename T5=null_type, typename T6=null_type,
                      typename T7=null_type, typename T8=null_type, typename T9=null_type> class indexer_node;

//indexer node specializations
template<typename T0>
class indexer_node<T0> : public unfolded_indexer_node<std::tuple<T0> > {
private:
    static const int N = 1;
public:
    typedef std::tuple<T0> InputTuple;
    typedef tagged_msg<size_t, T0> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }
};

template<typename T0, typename T1>
class indexer_node<T0, T1> : public unfolded_indexer_node<std::tuple<T0, T1> > {
private:
    static const int N = 2;
public:
    typedef std::tuple<T0, T1> InputTuple;
    typedef tagged_msg<size_t, T0, T1> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};

template<typename T0, typename T1, typename T2>
class indexer_node<T0, T1, T2> : public unfolded_indexer_node<std::tuple<T0, T1, T2> > {
private:
    static const int N = 3;
public:
    typedef std::tuple<T0, T1, T2> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};

template<typename T0, typename T1, typename T2, typename T3>
class indexer_node<T0, T1, T2, T3> : public unfolded_indexer_node<std::tuple<T0, T1, T2, T3> > {
private:
    static const int N = 4;
public:
    typedef std::tuple<T0, T1, T2, T3> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2, T3> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};

template<typename T0, typename T1, typename T2, typename T3, typename T4>
class indexer_node<T0, T1, T2, T3, T4> : public unfolded_indexer_node<std::tuple<T0, T1, T2, T3, T4> > {
private:
    static const int N = 5;
public:
    typedef std::tuple<T0, T1, T2, T3, T4> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2, T3, T4> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};

#if __TBB_VARIADIC_MAX >= 6
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
class indexer_node<T0, T1, T2, T3, T4, T5> : public unfolded_indexer_node<std::tuple<T0, T1, T2, T3, T4, T5> > {
private:
    static const int N = 6;
public:
    typedef std::tuple<T0, T1, T2, T3, T4, T5> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2, T3, T4, T5> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};
#endif //variadic max 6

#if __TBB_VARIADIC_MAX >= 7
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
         typename T6>
class indexer_node<T0, T1, T2, T3, T4, T5, T6> : public unfolded_indexer_node<std::tuple<T0, T1, T2, T3, T4, T5, T6> > {
private:
    static const int N = 7;
public:
    typedef std::tuple<T0, T1, T2, T3, T4, T5, T6> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2, T3, T4, T5, T6> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};
#endif //variadic max 7

#if __TBB_VARIADIC_MAX >= 8
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
         typename T6, typename T7>
class indexer_node<T0, T1, T2, T3, T4, T5, T6, T7> : public unfolded_indexer_node<std::tuple<T0, T1, T2, T3, T4, T5, T6, T7> > {
private:
    static const int N = 8;
public:
    typedef std::tuple<T0, T1, T2, T3, T4, T5, T6, T7> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2, T3, T4, T5, T6, T7> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};
#endif //variadic max 8

#if __TBB_VARIADIC_MAX >= 9
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
         typename T6, typename T7, typename T8>
class indexer_node<T0, T1, T2, T3, T4, T5, T6, T7, T8> : public unfolded_indexer_node<std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> > {
private:
    static const int N = 9;
public:
    typedef std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2, T3, T4, T5, T6, T7, T8> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};
#endif //variadic max 9

#if __TBB_VARIADIC_MAX >= 10
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
         typename T6, typename T7, typename T8, typename T9>
class indexer_node/*default*/ : public unfolded_indexer_node<std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> > {
private:
    static const int N = 10;
public:
    typedef std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> InputTuple;
    typedef tagged_msg<size_t, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> output_type;
    typedef unfolded_indexer_node<InputTuple> unfolded_type;
    __TBB_NOINLINE_SYM indexer_node(graph& g) : unfolded_type(g) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    indexer_node(const node_set<Args...>& nodes) : indexer_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    // Copy constructor
    __TBB_NOINLINE_SYM indexer_node( const indexer_node& other ) : unfolded_type(other) {
        fgt_multiinput_node<N>( CODEPTR(), FLOW_INDEXER_NODE, &this->my_graph,
                                           this->input_ports(), static_cast< sender< output_type > *>(this) );
    }

};
#endif //variadic max 10

template< typename T >
inline void internal_make_edge( sender<T> &p, receiver<T> &s ) {
    register_successor(p, s);
    fgt_make_edge( &p, &s );
}

//! Makes an edge between a single predecessor and a single successor
template< typename T >
inline void make_edge( sender<T> &p, receiver<T> &s ) {
    internal_make_edge( p, s );
}

//Makes an edge from port 0 of a multi-output predecessor to port 0 of a multi-input successor.
template< typename T, typename V,
          typename = typename T::output_ports_type, typename = typename V::input_ports_type >
inline void make_edge( T& output, V& input) {
    make_edge(std::get<0>(output.output_ports()), std::get<0>(input.input_ports()));
}

//Makes an edge from port 0 of a multi-output predecessor to a receiver.
template< typename T, typename R,
          typename = typename T::output_ports_type >
inline void make_edge( T& output, receiver<R>& input) {
     make_edge(std::get<0>(output.output_ports()), input);
}

//Makes an edge from a sender to port 0 of a multi-input successor.
template< typename S,  typename V,
          typename = typename V::input_ports_type >
inline void make_edge( sender<S>& output, V& input) {
     make_edge(output, std::get<0>(input.input_ports()));
}

template< typename T >
inline void internal_remove_edge( sender<T> &p, receiver<T> &s ) {
    remove_successor( p, s );
    fgt_remove_edge( &p, &s );
}

//! Removes an edge between a single predecessor and a single successor
template< typename T >
inline void remove_edge( sender<T> &p, receiver<T> &s ) {
    internal_remove_edge( p, s );
}

//Removes an edge between port 0 of a multi-output predecessor and port 0 of a multi-input successor.
template< typename T, typename V,
          typename = typename T::output_ports_type, typename = typename V::input_ports_type >
inline void remove_edge( T& output, V& input) {
    remove_edge(std::get<0>(output.output_ports()), std::get<0>(input.input_ports()));
}

//Removes an edge between port 0 of a multi-output predecessor and a receiver.
template< typename T, typename R,
          typename = typename T::output_ports_type >
inline void remove_edge( T& output, receiver<R>& input) {
     remove_edge(std::get<0>(output.output_ports()), input);
}
//Removes an edge between a sender and port 0 of a multi-input successor.
template< typename S,  typename V,
          typename = typename V::input_ports_type >
inline void remove_edge( sender<S>& output, V& input) {
     remove_edge(output, std::get<0>(input.input_ports()));
}

//! Returns a copy of the body from a function or continue node
template< typename Body, typename Node >
Body copy_body( Node &n ) {
    return n.template copy_function_object<Body>();
}

//composite_node
template< typename InputTuple, typename OutputTuple > class composite_node;

template< typename... InputTypes, typename... OutputTypes>
class composite_node <std::tuple<InputTypes...>, std::tuple<OutputTypes...> > : public graph_node {

public:
    typedef std::tuple< receiver<InputTypes>&... > input_ports_type;
    typedef std::tuple< sender<OutputTypes>&... > output_ports_type;

private:
    std::unique_ptr<input_ports_type> my_input_ports;
    std::unique_ptr<output_ports_type> my_output_ports;

    static const size_t NUM_INPUTS = sizeof...(InputTypes);
    static const size_t NUM_OUTPUTS = sizeof...(OutputTypes);

protected:
    void reset_node(reset_flags) override {}

public:
    composite_node( graph &g ) : graph_node(g) {
        fgt_multiinput_multioutput_node( CODEPTR(), FLOW_COMPOSITE_NODE, this, &this->my_graph );
    }

    template<typename T1, typename T2>
    void set_external_ports(T1&& input_ports_tuple, T2&& output_ports_tuple) {
        static_assert(NUM_INPUTS == std::tuple_size<input_ports_type>::value, "number of arguments does not match number of input ports");
        static_assert(NUM_OUTPUTS == std::tuple_size<output_ports_type>::value, "number of arguments does not match number of output ports");

        fgt_internal_input_alias_helper<T1, NUM_INPUTS>::alias_port( this, input_ports_tuple);
        fgt_internal_output_alias_helper<T2, NUM_OUTPUTS>::alias_port( this, output_ports_tuple);

        my_input_ports.reset( new input_ports_type(std::forward<T1>(input_ports_tuple)) );
        my_output_ports.reset( new output_ports_type(std::forward<T2>(output_ports_tuple)) );
    }

    template< typename... NodeTypes >
    void add_visible_nodes(const NodeTypes&... n) { add_nodes_impl(this, true, n...); }

    template< typename... NodeTypes >
    void add_nodes(const NodeTypes&... n) { add_nodes_impl(this, false, n...); }


    input_ports_type& input_ports() {
         __TBB_ASSERT(my_input_ports, "input ports not set, call set_external_ports to set input ports");
         return *my_input_ports;
    }

    output_ports_type& output_ports() {
         __TBB_ASSERT(my_output_ports, "output ports not set, call set_external_ports to set output ports");
         return *my_output_ports;
    }
};  // class composite_node

//composite_node with only input ports
template< typename... InputTypes>
class composite_node <std::tuple<InputTypes...>, std::tuple<> > : public graph_node {
public:
    typedef std::tuple< receiver<InputTypes>&... > input_ports_type;

private:
    std::unique_ptr<input_ports_type> my_input_ports;
    static const size_t NUM_INPUTS = sizeof...(InputTypes);

protected:
    void reset_node(reset_flags) override {}

public:
    composite_node( graph &g ) : graph_node(g) {
        fgt_composite( CODEPTR(), this, &g );
    }

   template<typename T>
   void set_external_ports(T&& input_ports_tuple) {
       static_assert(NUM_INPUTS == std::tuple_size<input_ports_type>::value, "number of arguments does not match number of input ports");

       fgt_internal_input_alias_helper<T, NUM_INPUTS>::alias_port( this, input_ports_tuple);

       my_input_ports.reset( new input_ports_type(std::forward<T>(input_ports_tuple)) );
   }

    template< typename... NodeTypes >
    void add_visible_nodes(const NodeTypes&... n) { add_nodes_impl(this, true, n...); }

    template< typename... NodeTypes >
    void add_nodes( const NodeTypes&... n) { add_nodes_impl(this, false, n...); }


    input_ports_type& input_ports() {
         __TBB_ASSERT(my_input_ports, "input ports not set, call set_external_ports to set input ports");
         return *my_input_ports;
    }

};  // class composite_node

//composite_nodes with only output_ports
template<typename... OutputTypes>
class composite_node <std::tuple<>, std::tuple<OutputTypes...> > : public graph_node {
public:
    typedef std::tuple< sender<OutputTypes>&... > output_ports_type;

private:
    std::unique_ptr<output_ports_type> my_output_ports;
    static const size_t NUM_OUTPUTS = sizeof...(OutputTypes);

protected:
    void reset_node(reset_flags) override {}

public:
    __TBB_NOINLINE_SYM composite_node( graph &g ) : graph_node(g) {
        fgt_composite( CODEPTR(), this, &g );
    }

   template<typename T>
   void set_external_ports(T&& output_ports_tuple) {
       static_assert(NUM_OUTPUTS == std::tuple_size<output_ports_type>::value, "number of arguments does not match number of output ports");

       fgt_internal_output_alias_helper<T, NUM_OUTPUTS>::alias_port( this, output_ports_tuple);

       my_output_ports.reset( new output_ports_type(std::forward<T>(output_ports_tuple)) );
   }

    template<typename... NodeTypes >
    void add_visible_nodes(const NodeTypes&... n) { add_nodes_impl(this, true, n...); }

    template<typename... NodeTypes >
    void add_nodes(const NodeTypes&... n) { add_nodes_impl(this, false, n...); }


    output_ports_type& output_ports() {
         __TBB_ASSERT(my_output_ports, "output ports not set, call set_external_ports to set output ports");
         return *my_output_ports;
    }

};  // class composite_node

template<typename Gateway>
class async_body_base: no_assign {
public:
    typedef Gateway gateway_type;

    async_body_base(gateway_type *gateway): my_gateway(gateway) { }
    void set_gateway(gateway_type *gateway) {
        my_gateway = gateway;
    }

protected:
    gateway_type *my_gateway;
};

template<typename Input, typename Ports, typename Gateway, typename Body>
class async_body: public async_body_base<Gateway> {
private:
    Body my_body;

public:
    typedef async_body_base<Gateway> base_type;
    typedef Gateway gateway_type;

    async_body(const Body &body, gateway_type *gateway)
        : base_type(gateway), my_body(body) { }

    void operator()( const Input &v, Ports & ) noexcept(noexcept(tbb::detail::invoke(my_body, v, std::declval<gateway_type&>()))) {
        tbb::detail::invoke(my_body, v, *this->my_gateway);
    }

    Body get_body() { return my_body; }
};

//! Implements async node
template < typename Input, typename Output,
           typename Policy = queueing_lightweight >
    __TBB_requires(std::default_initializable<Input> && std::copy_constructible<Input>)
class async_node
    : public multifunction_node< Input, std::tuple< Output >, Policy >, public sender< Output >
{
    typedef multifunction_node< Input, std::tuple< Output >, Policy > base_type;
    typedef multifunction_input<
        Input, typename base_type::output_ports_type, Policy, cache_aligned_allocator<Input>> mfn_input_type;

public:
    typedef Input input_type;
    typedef Output output_type;
    typedef receiver<input_type> receiver_type;
    typedef receiver<output_type> successor_type;
    typedef sender<input_type> predecessor_type;
    typedef receiver_gateway<output_type> gateway_type;
    typedef async_body_base<gateway_type> async_body_base_type;
    typedef typename base_type::output_ports_type output_ports_type;

private:
    class receiver_gateway_impl: public receiver_gateway<Output> {
    public:
        receiver_gateway_impl(async_node* node): my_node(node) {}
        void reserve_wait() override {
            fgt_async_reserve(static_cast<typename async_node::receiver_type *>(my_node), &my_node->my_graph);
            my_node->my_graph.reserve_wait();
        }

        void release_wait() override {
            async_node* n = my_node;
            graph* g = &n->my_graph;
            g->release_wait();
            fgt_async_commit(static_cast<typename async_node::receiver_type *>(n), g);
        }

        //! Implements gateway_type::try_put for an external activity to submit a message to FG
        bool try_put(const Output &i) override {
            return my_node->try_put_impl(i);
        }

    private:
        async_node* my_node;
    } my_gateway;

    //The substitute of 'this' for member construction, to prevent compiler warnings
    async_node* self() { return this; }

    //! Implements gateway_type::try_put for an external activity to submit a message to FG
    bool try_put_impl(const Output &i) {
        multifunction_output<Output> &port_0 = output_port<0>(*this);
        broadcast_cache<output_type>& port_successors = port_0.successors();
        fgt_async_try_put_begin(this, &port_0);
        // TODO revamp: change to std::list<graph_task*>
        graph_task_list tasks;
        bool is_at_least_one_put_successful = port_successors.gather_successful_try_puts(i, tasks);
        __TBB_ASSERT( is_at_least_one_put_successful || tasks.empty(),
                      "Return status is inconsistent with the method operation." );

        while( !tasks.empty() ) {
            enqueue_in_graph_arena(this->my_graph, tasks.pop_front());
        }
        fgt_async_try_put_end(this, &port_0);
        return is_at_least_one_put_successful;
    }

public:
    template<typename Body>
        __TBB_requires(async_node_body<Body, input_type, gateway_type>)
    __TBB_NOINLINE_SYM async_node(
        graph &g, size_t concurrency,
        Body body, Policy = Policy(), node_priority_t a_priority = no_priority
    ) : base_type(
        g, concurrency,
        async_body<Input, typename base_type::output_ports_type, gateway_type, Body>
        (body, &my_gateway), a_priority ), my_gateway(self()) {
        fgt_multioutput_node_with_body<1>(
            CODEPTR(), FLOW_ASYNC_NODE,
            &this->my_graph, static_cast<receiver<input_type> *>(this),
            this->output_ports(), this->my_body
        );
    }

    template <typename Body>
        __TBB_requires(async_node_body<Body, input_type, gateway_type>)
    __TBB_NOINLINE_SYM async_node(graph& g, size_t concurrency, Body body, node_priority_t a_priority)
        : async_node(g, concurrency, body, Policy(), a_priority) {}

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename Body, typename... Args>
        __TBB_requires(async_node_body<Body, input_type, gateway_type>)
    __TBB_NOINLINE_SYM async_node(
        const node_set<Args...>& nodes, size_t concurrency, Body body,
        Policy = Policy(), node_priority_t a_priority = no_priority )
        : async_node(nodes.graph_reference(), concurrency, body, a_priority) {
        make_edges_in_order(nodes, *this);
    }

    template <typename Body, typename... Args>
        __TBB_requires(async_node_body<Body, input_type, gateway_type>)
    __TBB_NOINLINE_SYM async_node(const node_set<Args...>& nodes, size_t concurrency, Body body, node_priority_t a_priority)
        : async_node(nodes, concurrency, body, Policy(), a_priority) {}
#endif // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

    __TBB_NOINLINE_SYM async_node( const async_node &other ) : base_type(other), sender<Output>(), my_gateway(self()) {
        static_cast<async_body_base_type*>(this->my_body->get_body_ptr())->set_gateway(&my_gateway);
        static_cast<async_body_base_type*>(this->my_init_body->get_body_ptr())->set_gateway(&my_gateway);

        fgt_multioutput_node_with_body<1>( CODEPTR(), FLOW_ASYNC_NODE,
                &this->my_graph, static_cast<receiver<input_type> *>(this),
                this->output_ports(), this->my_body );
    }

    gateway_type& gateway() {
        return my_gateway;
    }

    // Define sender< Output >

    //! Add a new successor to this node
    bool register_successor(successor_type&) override {
        __TBB_ASSERT(false, "Successors must be registered only via ports");
        return false;
    }

    //! Removes a successor from this node
    bool remove_successor(successor_type&) override {
        __TBB_ASSERT(false, "Successors must be removed only via ports");
        return false;
    }

    template<typename Body>
    Body copy_function_object() {
        typedef multifunction_body<input_type, typename base_type::output_ports_type> mfn_body_type;
        typedef async_body<Input, typename base_type::output_ports_type, gateway_type, Body> async_body_type;
        mfn_body_type &body_ref = *this->my_body;
        async_body_type ab = *static_cast<async_body_type*>(dynamic_cast< multifunction_body_leaf<input_type, typename base_type::output_ports_type, async_body_type> & >(body_ref).get_body_ptr());
        return ab.get_body();
    }

protected:

    void reset_node( reset_flags f) override {
       base_type::reset_node(f);
    }
};

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
// Visual Studio 2019 reports an error while calling predecessor_selector::get and successor_selector::get
// Seems like the well-formed expression in trailing decltype is treated as ill-formed
// TODO: investigate problems with decltype in trailing return types or find the cross-platform solution
#define __TBB_MSVC_DISABLE_TRAILING_DECLTYPE (_MSC_VER >= 1900)

namespace order {
struct undefined {};
struct following {};
struct preceding {};
}

class get_graph_helper {
public:
    // TODO: consider making graph_reference() public and consistent interface to get a reference to the graph
    // and remove get_graph_helper
    template <typename T>
    static graph& get(const T& object) {
        return get_impl(object, std::is_base_of<graph_node, T>());
    }

private:
    // Get graph from the object of type derived from graph_node
    template <typename T>
    static graph& get_impl(const T& object, std::true_type) {
        return static_cast<const graph_node*>(&object)->my_graph;
    }

    template <typename T>
    static graph& get_impl(const T& object, std::false_type) {
        return object.graph_reference();
    }
};

template<typename Order, typename... Nodes>
struct node_set {
    typedef Order order_type;

    std::tuple<Nodes&...> nodes;
    node_set(Nodes&... ns) : nodes(ns...) {}

    template <typename... Nodes2>
    node_set(const node_set<order::undefined, Nodes2...>& set) : nodes(set.nodes) {}

    graph& graph_reference() const {
        return get_graph_helper::get(std::get<0>(nodes));
    }
};

namespace alias_helpers {
template <typename T> using output_type = typename T::output_type;
template <typename T> using output_ports_type = typename T::output_ports_type;
template <typename T> using input_type = typename T::input_type;
template <typename T> using input_ports_type = typename T::input_ports_type;
} // namespace alias_helpers

template <typename T>
using has_output_type = supports<T, alias_helpers::output_type>;

template <typename T>
using has_input_type = supports<T, alias_helpers::input_type>;

template <typename T>
using has_input_ports_type = supports<T, alias_helpers::input_ports_type>;

template <typename T>
using has_output_ports_type = supports<T, alias_helpers::output_ports_type>;

template<typename T>
struct is_sender : std::is_base_of<sender<typename T::output_type>, T> {};

template<typename T>
struct is_receiver : std::is_base_of<receiver<typename T::input_type>, T> {};

template <typename Node>
struct is_async_node : std::false_type {};

template <typename... Args>
struct is_async_node<async_node<Args...>> : std::true_type {};

template<typename FirstPredecessor, typename... Predecessors>
node_set<order::following, FirstPredecessor, Predecessors...>
follows(FirstPredecessor& first_predecessor, Predecessors&... predecessors) {
    static_assert((conjunction<has_output_type<FirstPredecessor>,
                                                   has_output_type<Predecessors>...>::value),
                        "Not all node's predecessors has output_type typedef");
    static_assert((conjunction<is_sender<FirstPredecessor>, is_sender<Predecessors>...>::value),
                        "Not all node's predecessors are senders");
    return node_set<order::following, FirstPredecessor, Predecessors...>(first_predecessor, predecessors...);
}

template<typename... Predecessors>
node_set<order::following, Predecessors...>
follows(node_set<order::undefined, Predecessors...>& predecessors_set) {
    static_assert((conjunction<has_output_type<Predecessors>...>::value),
                        "Not all nodes in the set has output_type typedef");
    static_assert((conjunction<is_sender<Predecessors>...>::value),
                        "Not all nodes in the set are senders");
    return node_set<order::following, Predecessors...>(predecessors_set);
}

template<typename FirstSuccessor, typename... Successors>
node_set<order::preceding, FirstSuccessor, Successors...>
precedes(FirstSuccessor& first_successor, Successors&... successors) {
    static_assert((conjunction<has_input_type<FirstSuccessor>,
                                                    has_input_type<Successors>...>::value),
                        "Not all node's successors has input_type typedef");
    static_assert((conjunction<is_receiver<FirstSuccessor>, is_receiver<Successors>...>::value),
                        "Not all node's successors are receivers");
    return node_set<order::preceding, FirstSuccessor, Successors...>(first_successor, successors...);
}

template<typename... Successors>
node_set<order::preceding, Successors...>
precedes(node_set<order::undefined, Successors...>& successors_set) {
    static_assert((conjunction<has_input_type<Successors>...>::value),
                        "Not all nodes in the set has input_type typedef");
    static_assert((conjunction<is_receiver<Successors>...>::value),
                        "Not all nodes in the set are receivers");
    return node_set<order::preceding, Successors...>(successors_set);
}

template <typename Node, typename... Nodes>
node_set<order::undefined, Node, Nodes...>
make_node_set(Node& first_node, Nodes&... nodes) {
    return node_set<order::undefined, Node, Nodes...>(first_node, nodes...);
}

template<size_t I>
class successor_selector {
    template <typename NodeType>
    static auto get_impl(NodeType& node, std::true_type) -> decltype(input_port<I>(node)) {
        return input_port<I>(node);
    }

    template <typename NodeType>
    static NodeType& get_impl(NodeType& node, std::false_type) { return node; }

public:
    template <typename NodeType>
#if __TBB_MSVC_DISABLE_TRAILING_DECLTYPE
    static auto& get(NodeType& node)
#else
    static auto get(NodeType& node) -> decltype(get_impl(node, has_input_ports_type<NodeType>()))
#endif
    {
        return get_impl(node, has_input_ports_type<NodeType>());
    }
};

template<size_t I>
class predecessor_selector {
    template <typename NodeType>
    static auto internal_get(NodeType& node, std::true_type) -> decltype(output_port<I>(node)) {
        return output_port<I>(node);
    }

    template <typename NodeType>
    static NodeType& internal_get(NodeType& node, std::false_type) { return node;}

    template <typename NodeType>
#if __TBB_MSVC_DISABLE_TRAILING_DECLTYPE
    static auto& get_impl(NodeType& node, std::false_type)
#else
    static auto get_impl(NodeType& node, std::false_type) -> decltype(internal_get(node, has_output_ports_type<NodeType>()))
#endif
    {
        return internal_get(node, has_output_ports_type<NodeType>());
    }

    template <typename AsyncNode>
    static AsyncNode& get_impl(AsyncNode& node, std::true_type) { return node; }

public:
    template <typename NodeType>
#if __TBB_MSVC_DISABLE_TRAILING_DECLTYPE
    static auto& get(NodeType& node)
#else
    static auto get(NodeType& node) -> decltype(get_impl(node, is_async_node<NodeType>()))
#endif
    {
        return get_impl(node, is_async_node<NodeType>());
    }
};

template<size_t I>
class make_edges_helper {
public:
    template<typename PredecessorsTuple, typename NodeType>
    static void connect_predecessors(PredecessorsTuple& predecessors, NodeType& node) {
        make_edge(std::get<I>(predecessors), successor_selector<I>::get(node));
        make_edges_helper<I - 1>::connect_predecessors(predecessors, node);
    }

    template<typename SuccessorsTuple, typename NodeType>
    static void connect_successors(NodeType& node, SuccessorsTuple& successors) {
        make_edge(predecessor_selector<I>::get(node), std::get<I>(successors));
        make_edges_helper<I - 1>::connect_successors(node, successors);
    }
};

template<>
struct make_edges_helper<0> {
    template<typename PredecessorsTuple, typename NodeType>
    static void connect_predecessors(PredecessorsTuple& predecessors, NodeType& node) {
        make_edge(std::get<0>(predecessors), successor_selector<0>::get(node));
    }

    template<typename SuccessorsTuple, typename NodeType>
    static void connect_successors(NodeType& node, SuccessorsTuple& successors) {
        make_edge(predecessor_selector<0>::get(node), std::get<0>(successors));
    }
};

// TODO: consider adding an overload for making edges between node sets
template<typename NodeType, typename OrderFlagType, typename... Args>
void make_edges(const node_set<OrderFlagType, Args...>& s, NodeType& node) {
    const std::size_t SetSize = std::tuple_size<decltype(s.nodes)>::value;
    make_edges_helper<SetSize - 1>::connect_predecessors(s.nodes, node);
}

template <typename NodeType, typename OrderFlagType, typename... Args>
void make_edges(NodeType& node, const node_set<OrderFlagType, Args...>& s) {
    const std::size_t SetSize = std::tuple_size<decltype(s.nodes)>::value;
    make_edges_helper<SetSize - 1>::connect_successors(node, s.nodes);
}

template <typename NodeType, typename... Nodes>
void make_edges_in_order(const node_set<order::following, Nodes...>& ns, NodeType& node) {
    make_edges(ns, node);
}

template <typename NodeType, typename... Nodes>
void make_edges_in_order(const node_set<order::preceding, Nodes...>& ns, NodeType& node) {
    make_edges(node, ns);
}

#endif  // __TBB_PREVIEW_FLOW_GRAPH_NODE_SET

template< typename T >
class overwrite_node : public graph_node, public receiver<T>, public sender<T> {
public:
    typedef T input_type;
    typedef T output_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;

    __TBB_NOINLINE_SYM explicit overwrite_node(graph &g)
        : graph_node(g), my_successors(this), my_buffer_is_valid(false)
    {
        fgt_node( CODEPTR(), FLOW_OVERWRITE_NODE, &this->my_graph,
                  static_cast<receiver<input_type> *>(this), static_cast<sender<output_type> *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    overwrite_node(const node_set<Args...>& nodes) : overwrite_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    //! Copy constructor; doesn't take anything from src; default won't work
    __TBB_NOINLINE_SYM overwrite_node( const overwrite_node& src ) : overwrite_node(src.my_graph) {}

    ~overwrite_node() {}

    bool register_successor( successor_type &s ) override {
        spin_mutex::scoped_lock l( my_mutex );
        if (my_buffer_is_valid && is_graph_active( my_graph )) {
            // We have a valid value that must be forwarded immediately.
            bool ret = s.try_put( my_buffer );
            if ( ret ) {
                // We add the successor that accepted our put
                my_successors.register_successor( s );
            } else {
                // In case of reservation a race between the moment of reservation and register_successor can appear,
                // because failed reserve does not mean that register_successor is not ready to put a message immediately.
                // We have some sort of infinite loop: reserving node tries to set pull state for the edge,
                // but overwrite_node tries to return push state back. That is why we have to break this loop with task creation.
                small_object_allocator allocator{};
                typedef register_predecessor_task task_type;
                graph_task* t = allocator.new_object<task_type>(graph_reference(), allocator, *this, s);
                graph_reference().reserve_wait();
                spawn_in_graph_arena( my_graph, *t );
            }
        } else {
            // No valid value yet, just add as successor
            my_successors.register_successor( s );
        }
        return true;
    }

    bool remove_successor( successor_type &s ) override {
        spin_mutex::scoped_lock l( my_mutex );
        my_successors.remove_successor(s);
        return true;
    }

    bool try_get( input_type &v ) override {
        spin_mutex::scoped_lock l( my_mutex );
        if ( my_buffer_is_valid ) {
            v = my_buffer;
            return true;
        }
        return false;
    }

    //! Reserves an item
    bool try_reserve( T &v ) override {
        return try_get(v);
    }

    //! Releases the reserved item
    bool try_release() override { return true; }

    //! Consumes the reserved item
    bool try_consume() override { return true; }

    bool is_valid() {
       spin_mutex::scoped_lock l( my_mutex );
       return my_buffer_is_valid;
    }

    void clear() {
       spin_mutex::scoped_lock l( my_mutex );
       my_buffer_is_valid = false;
    }

protected:

    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    graph_task* try_put_task( const input_type &v ) override {
        spin_mutex::scoped_lock l( my_mutex );
        return try_put_task_impl(v);
    }

    graph_task * try_put_task_impl(const input_type &v) {
        my_buffer = v;
        my_buffer_is_valid = true;
        graph_task* rtask = my_successors.try_put_task(v);
        if (!rtask) rtask = SUCCESSFULLY_ENQUEUED;
        return rtask;
    }

    graph& graph_reference() const override {
        return my_graph;
    }

    //! Breaks an infinite loop between the node reservation and register_successor call
    struct register_predecessor_task : public graph_task {
        register_predecessor_task(
            graph& g, small_object_allocator& allocator, predecessor_type& owner, successor_type& succ)
            : graph_task(g, allocator), o(owner), s(succ) {};

        task* execute(execution_data& ed) override {
            // TODO revamp: investigate why qualification is needed for register_successor() call
            using tbb::detail::d1::register_predecessor;
            using tbb::detail::d1::register_successor;
            if ( !register_predecessor(s, o) ) {
                register_successor(o, s);
            }
            finalize<register_predecessor_task>(ed);
            return nullptr;
        }

        task* cancel(execution_data& ed) override {
            finalize<register_predecessor_task>(ed);
            return nullptr;
        }

        predecessor_type& o;
        successor_type& s;
    };

    spin_mutex my_mutex;
    broadcast_cache< input_type, null_rw_mutex > my_successors;
    input_type my_buffer;
    bool my_buffer_is_valid;

    void reset_node( reset_flags f) override {
        my_buffer_is_valid = false;
       if (f&rf_clear_edges) {
           my_successors.clear();
       }
    }
};  // overwrite_node

template< typename T >
class write_once_node : public overwrite_node<T> {
public:
    typedef T input_type;
    typedef T output_type;
    typedef overwrite_node<T> base_type;
    typedef typename receiver<input_type>::predecessor_type predecessor_type;
    typedef typename sender<output_type>::successor_type successor_type;

    //! Constructor
    __TBB_NOINLINE_SYM explicit write_once_node(graph& g) : base_type(g) {
        fgt_node( CODEPTR(), FLOW_WRITE_ONCE_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    template <typename... Args>
    write_once_node(const node_set<Args...>& nodes) : write_once_node(nodes.graph_reference()) {
        make_edges_in_order(nodes, *this);
    }
#endif

    //! Copy constructor: call base class copy constructor
    __TBB_NOINLINE_SYM write_once_node( const write_once_node& src ) : base_type(src) {
        fgt_node( CODEPTR(), FLOW_WRITE_ONCE_NODE, &(this->my_graph),
                                 static_cast<receiver<input_type> *>(this),
                                 static_cast<sender<output_type> *>(this) );
    }

protected:
    template< typename R, typename B > friend class run_and_put_task;
    template<typename X, typename Y> friend class broadcast_cache;
    template<typename X, typename Y> friend class round_robin_cache;
    graph_task *try_put_task( const T &v ) override {
        spin_mutex::scoped_lock l( this->my_mutex );
        return this->my_buffer_is_valid ? nullptr : this->try_put_task_impl(v);
    }
}; // write_once_node

inline void set_name(const graph& g, const char *name) {
    fgt_graph_desc(&g, name);
}

template <typename Output>
inline void set_name(const input_node<Output>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename Input, typename Output, typename Policy>
inline void set_name(const function_node<Input, Output, Policy>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename Output, typename Policy>
inline void set_name(const continue_node<Output,Policy>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T>
inline void set_name(const broadcast_node<T>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T>
inline void set_name(const buffer_node<T>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T>
inline void set_name(const queue_node<T>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T>
inline void set_name(const sequencer_node<T>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T, typename Compare>
inline void set_name(const priority_queue_node<T, Compare>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T, typename DecrementType>
inline void set_name(const limiter_node<T, DecrementType>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename OutputTuple, typename JP>
inline void set_name(const join_node<OutputTuple, JP>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename... Types>
inline void set_name(const indexer_node<Types...>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T>
inline void set_name(const overwrite_node<T>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template <typename T>
inline void set_name(const write_once_node<T>& node, const char *name) {
    fgt_node_desc(&node, name);
}

template<typename Input, typename Output, typename Policy>
inline void set_name(const multifunction_node<Input, Output, Policy>& node, const char *name) {
    fgt_multioutput_node_desc(&node, name);
}

template<typename TupleType>
inline void set_name(const split_node<TupleType>& node, const char *name) {
    fgt_multioutput_node_desc(&node, name);
}

template< typename InputTuple, typename OutputTuple >
inline void set_name(const composite_node<InputTuple, OutputTuple>& node, const char *name) {
    fgt_multiinput_multioutput_node_desc(&node, name);
}

template<typename Input, typename Output, typename Policy>
inline void set_name(const async_node<Input, Output, Policy>& node, const char *name)
{
    fgt_multioutput_node_desc(&node, name);
}
} // d1
} // detail
} // tbb



// Include deduction guides for node classes
#include "detail/_flow_graph_nodes_deduction.h"

namespace tbb {
namespace flow {
inline namespace v1 {
    using detail::d1::receiver;
    using detail::d1::sender;

    using detail::d1::serial;
    using detail::d1::unlimited;

    using detail::d1::reset_flags;
    using detail::d1::rf_reset_protocol;
    using detail::d1::rf_reset_bodies;
    using detail::d1::rf_clear_edges;

    using detail::d1::graph;
    using detail::d1::graph_node;
    using detail::d1::continue_msg;

    using detail::d1::input_node;
    using detail::d1::function_node;
    using detail::d1::multifunction_node;
    using detail::d1::split_node;
    using detail::d1::output_port;
    using detail::d1::indexer_node;
    using detail::d1::tagged_msg;
    using detail::d1::cast_to;
    using detail::d1::is_a;
    using detail::d1::continue_node;
    using detail::d1::overwrite_node;
    using detail::d1::write_once_node;
    using detail::d1::broadcast_node;
    using detail::d1::buffer_node;
    using detail::d1::queue_node;
    using detail::d1::sequencer_node;
    using detail::d1::priority_queue_node;
    using detail::d1::limiter_node;
    using namespace detail::d1::graph_policy_namespace;
    using detail::d1::join_node;
    using detail::d1::input_port;
    using detail::d1::copy_body;
    using detail::d1::make_edge;
    using detail::d1::remove_edge;
    using detail::d1::tag_value;
    using detail::d1::composite_node;
    using detail::d1::async_node;
    using detail::d1::node_priority_t;
    using detail::d1::no_priority;

#if __TBB_PREVIEW_FLOW_GRAPH_NODE_SET
    using detail::d1::follows;
    using detail::d1::precedes;
    using detail::d1::make_node_set;
    using detail::d1::make_edges;
#endif

} // v1
} // flow

    using detail::d1::flow_control;

namespace profiling {
    using detail::d1::set_name;
} // profiling

} // tbb


#if TBB_USE_PROFILING_TOOLS  && ( __unix__ || __APPLE__ )
   // We don't do pragma pop here, since it still gives warning on the USER side
   #undef __TBB_NOINLINE_SYM
#endif

#endif // __TBB_flow_graph_H

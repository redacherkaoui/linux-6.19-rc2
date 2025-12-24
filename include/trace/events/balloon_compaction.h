#undef TRACE_SYSTEM
#define TRACE_SYSTEM balloon_compaction

#if !defined(_TRACE_BALLOON_COMP_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_BALLOON_COMP_H

#include <linux/tracepoint.h>

TRACE_EVENT(balloon_enqueue_trylock_failed,

	TP_PROTO(struct page *page, int refcount, unsigned long flags,
		 unsigned long caller),

	TP_ARGS(page, refcount, flags, caller),

	TP_STRUCT__entry(
		__field(struct page *, page)
		__field(int, refcount)
		__field(unsigned long, flags)
		__field(unsigned long, caller)
	),

	TP_fast_assign(
		__entry->page = page;
		__entry->refcount = refcount;
		__entry->flags = flags;
		__entry->caller = caller;
	),

	TP_printk("page=%p refcount=%d flags=0x%lx caller=%pS",
		  __entry->page, __entry->refcount, __entry->flags,
		  (void *)__entry->caller)
);

#endif /* _TRACE_BALLOON_COMP_H */

/* This part must be outside protection */
#include <trace/define_trace.h>

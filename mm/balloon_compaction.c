// SPDX-License-Identifier: GPL-2.0-only
/*
 * mm/balloon_compaction.c
 *
 * Common interface for making balloon pages movable by compaction.
 *
 * Copyright (C) 2012, Red Hat, Inc.  Rafael Aquini <aquini@redhat.com>
 */
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/balloon_compaction.h>

#define CREATE_TRACE_POINTS
#include <trace/events/balloon_compaction.h>

static void balloon_page_enqueue_one(struct balloon_dev_info *b_dev_info,
				     struct page *page)
{
	/*
	 * Block others from accessing the 'page' when we get around to
	 * establishing additional references. We should be the only one
	 * holding a reference to the 'page' at this point. If we are not, then
	 * memory corruption is possible and we should stop execution.
	 */
	if (unlikely(!trylock_page(page))) {
		trace_balloon_enqueue_trylock_failed(page, page_count(page),
						     READ_ONCE(page->flags),
						     _RET_IP_);
		BUG();
	}

	balloon_page_insert(b_dev_info, page);
	unlock_page(page);
	__count_vm_event(BALLOON_INFLATE);
	inc_node_page_state(page, NR_BALLOON_PAGES);
}

/**
 * balloon_page_list_enqueue() - inserts a list of pages into the balloon page
 *				 list.
 * @b_dev_info: balloon device descriptor where we will insert a new page to
 * @pages: pages to enqueue - allocated using balloon_page_alloc.
 *
 * Driver must call this function to properly enqueue balloon pages before
 * definitively removing them from the guest system.
 *
 * Return: number of pages that were enqueued.
 */
size_t balloon_page_list_enqueue(struct balloon_dev_info *b_dev_info,
				 struct list_head *pages)
{
	struct page *page, *tmp;
	unsigned long flags;
	size_t n_pages = 0;

	spin_lock_irqsave(&b_dev_info->pages_lock, flags);
 
 	/* Move pages from the provided list into the balloon device list. */
 	list_for_each_entry_safe(page, tmp, pages, lru) {
 		list_del(&page->lru);
 		balloon_page_enqueue_one(b_dev_info, page);
 		n_pages++;
 	}

	spin_unlock_irqrestore(&b_dev_info->pages_lock, flags);

	return n_pages;

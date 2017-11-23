/*
 * list.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2013 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "librfn/util.h"
#include "librfn/list.h"

void list_insert(list_t *list, list_node_t *node)
{
	assert(NULL == node->next);

	if (list->head) {
		list->tail->next = node;
	} else {
		list->head = node;
	}

	list->tail = node;
}

void list_insert_sorted(list_t *list, list_node_t *node,
		list_node_compare_t *nodecmp)
{
	assert(NULL == node->next);

	/* fast path for empty list */
	if (!list->head) {
		list->head = node;
		list->tail = node;
		return;
	}

	/* fast path for insert at end */
	if (nodecmp(node, list->tail) >= 0) {
		list->tail->next = node;
		list->tail = node;
		return;
	}

	/* search the list until we find the correct place to insert the
	 * new node. we know (from the fast path above) that at least the
	 * tail node matches the loop termination condition.
	 */
	list_iterator_t iter;
	for (list_node_t *curr = list_iterate(list, &iter);
	     nodecmp(node, curr) >= 0;
	     curr = list_iterator_next(&iter))
		assert(curr);
	list_iterator_insert(&iter, node);
}

void list_push(list_t *list, list_node_t *node)
{
	assert(NULL == node->next);

	if (list->head) {
		node->next = list->head;
	} else {
		list->tail = node;
	}
	list->head = node;
}

list_node_t * list_extract(list_t *list)
{
	list_node_t *node;

	if (!list->head)
		return NULL;

	node = list->head;
	list->head = node->next;
	node->next = NULL;

	return node;
}

list_node_t * list_iterate(list_t *list, list_iterator_t *iter)
{
	iter->prevnext = &list->head;
	iter->list = list;

	return list->head;
}

list_node_t * list_iterator_next(list_iterator_t *iter)
{
	list_node_t *curr = *(iter->prevnext);

	if (curr) {
		iter->prevnext = &curr->next;
		return curr->next;
	}

	return NULL;
}

/* Once the insertion has been made the iterator points to the *new* node.
 */
void list_iterator_insert(list_iterator_t *iter, list_node_t *node)
{
	list_node_t *curr = *(iter->prevnext);

	*(iter->prevnext) = node;
	node->next = curr;

	if (!curr)
		iter->list->tail = node;
}

list_node_t * list_iterator_remove(list_iterator_t *iter)
{
	list_node_t *curr = *(iter->prevnext);
	assert(curr);

	/* Make sure we maintain the list's tail pointer correctly if we
	 * are removing the final node in a list.
	 *
	 * Careful code review will reveal that, strictly speaking, prev
	 * is not actually "correct" if the list contains only one node
	 * because prevnext doesn't point to a node's next pointer, instead
	 * it points to the list's head pointer.
	 *
	 * In practice this doesn't matter because if the list contains only
	 * a single node then when we remove the node we will set list->head
	 * to NULL. When the head is NULL we can write any pointer we like
	 * into list->tail since its value is considered invalid when head
	 * is NULL.
	 */
	list_node_t *prev = containerof(iter->prevnext, list_node_t, next);
	if (iter->list->tail == curr)
		iter->list->tail = prev;

	/* Remove the node (and invalidate its link pointer) */
	*(iter->prevnext) = curr->next;
	curr->next = NULL;


	return *(iter->prevnext);
}

bool list_contains(list_t *list, list_node_t *node, list_iterator_t *iter)
{
	list_iterator_t myiter;

	if (NULL == iter)
		iter = &myiter;

	for (list_node_t *curr = list_iterate(list, iter);
	     curr;
	     curr = list_iterator_next(iter))
		if (curr == node)
			return true;

	return false;
}

bool list_remove(list_t *list, list_node_t *node)
{
	list_iterator_t iter;

	if (list_contains(list, node, &iter)) {
		list_iterator_remove(&iter);
		return true;
	}

	return false;
}

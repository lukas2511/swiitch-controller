/*
 * list.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2013-2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_LIST_H_
#define RF_LIST_H_

#include <stdbool.h>
#include <stdint.h>

/*!
 * \defgroup librfn_list Singly linked list
 *
 * \brief A singly linked list with fast head/tail insertion and sorted
 *        insert support.
 *
 * No form of internal locking or other thread-safety is provided.
 *
 * \note Fibres on uniprocessor systems do not require thread-safety support
 *       and may freely use the list.
 *
 * @{
 */

typedef struct list_node {
	struct list_node *next;
} list_node_t;
#define LIST_NODE_VAR_INIT { 0 }

typedef struct {
	list_node_t *head;
	list_node_t *tail;
} list_t;
#define LIST_VAR_INIT { 0 }

typedef struct {
	list_node_t **prevnext;
	list_t *list;
} list_iterator_t;

typedef int list_node_compare_t(list_node_t *, list_node_t *);

/*
 * insertion operations
 */

void list_insert(list_t *list, list_node_t *node);
void list_insert_sorted(list_t *list, list_node_t *node,
		list_node_compare_t *nodecmp);
void list_push(list_t *list, list_node_t *node);

/*
 * extraction operations
 */

list_node_t * list_extract(list_t *list);

/*
 * list traversal
 */

list_node_t * list_iterate(list_t *list, list_iterator_t *iter);
list_node_t * list_iterator_next(list_iterator_t *iter);
void list_iterator_insert(list_iterator_t *iter, list_node_t *node);
list_node_t * list_iterator_remove(list_iterator_t *iter);

/*
 * search operations
 */

static inline bool list_empty(list_t *list) {
	return !list->head;
}

static inline list_node_t * list_peek(list_t *list) {
	return list->head;
}

bool list_contains(list_t *list, list_node_t *node, list_iterator_t *iter);
bool list_remove(list_t *list, list_node_t *node);

/*! @} */
#endif // RF_LIST_H_

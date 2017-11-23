/*
 * listtest.c
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

#undef NDEBUG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <librfn.h>

static int address_comparison(list_node_t *n1, list_node_t *n2)
{
	return n1 - n2;
}

static void test_list_insert()
{
	list_t list = { 0 };
	list_node_t n1 = { 0 };
	list_node_t n2 = { 0 };
	list_node_t n3 = { 0 };

	verify(NULL == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert(&list, &n1);
	verify(&n1 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert(&list, &n1);
	verify(&n1 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert(&list, &n1);
	list_insert(&list, &n2);
	verify(&n1 == list_extract(&list));
	verify(&n2 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert(&list, &n1);
	list_insert(&list, &n2);
	list_insert(&list, &n3);
	verify(&n1 == list_extract(&list));
	verify(&n2 == list_extract(&list));
	verify(&n3 == list_extract(&list));
	verify(NULL == list_extract(&list));
}

static void test_list_push()
{
	list_t list = { 0 };
	list_node_t n1 = { 0 };
	list_node_t n2 = { 0 };
	list_node_t n3 = { 0 };

	verify(NULL == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_push(&list, &n1);
	verify(&n1 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_push(&list, &n1);
	verify(&n1 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_push(&list, &n1);
	list_push(&list, &n2);
	verify(&n2 == list_extract(&list));
	verify(&n1 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_push(&list, &n1);
	list_push(&list, &n2);
	list_push(&list, &n3);
	verify(&n3 == list_extract(&list));
	verify(&n2 == list_extract(&list));
	verify(&n1 == list_extract(&list));
	verify(NULL == list_extract(&list));
}

static void test_list_insert_sorted()
{
	list_t list = { 0 };
	list_node_t n[3] = { { 0 } };

	verify(NULL == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert_sorted(&list, n+0, address_comparison);
	verify(n+0 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert_sorted(&list, n+0, address_comparison);
	verify(n+0 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert_sorted(&list, n+0, address_comparison);
	list_insert_sorted(&list, n+1, address_comparison);
	verify(n+0 == list_extract(&list));
	verify(n+1 == list_extract(&list));
	verify(NULL == list_extract(&list));

	list_insert_sorted(&list, n+1, address_comparison);
	list_insert_sorted(&list, n+0, address_comparison);
	verify(n+0 == list_extract(&list));
	verify(n+1 == list_extract(&list));
	verify(NULL == list_extract(&list));

	/* insert at end */
	list_insert_sorted(&list, n+0, address_comparison);
	list_insert_sorted(&list, n+1, address_comparison);
	list_insert_sorted(&list, n+2, address_comparison);
	verify(n+0 == list_extract(&list));
	verify(n+1 == list_extract(&list));
	verify(n+2 == list_extract(&list));
	verify(NULL == list_extract(&list));

	/* insert at beginning */
	list_insert_sorted(&list, n+2, address_comparison);
	list_insert_sorted(&list, n+1, address_comparison);
	list_insert_sorted(&list, n+0, address_comparison);
	verify(n+0 == list_extract(&list));
	verify(n+1 == list_extract(&list));
	verify(n+2 == list_extract(&list));
	verify(NULL == list_extract(&list));

	/* insert into middle */
	list_insert_sorted(&list, n+2, address_comparison);
	list_insert_sorted(&list, n+0, address_comparison);
	list_insert_sorted(&list, n+1, address_comparison);
	verify(n+0 == list_extract(&list));
	verify(n+1 == list_extract(&list));
	verify(n+2 == list_extract(&list));
	verify(NULL == list_extract(&list));
}

static void test_list_iterate()
{
	list_t list = { 0 };
	list_node_t n1 = { 0 };
	list_node_t n2 = { 0 };
	list_node_t n3 = { 0 };

	list_insert(&list, &n1);
	list_insert(&list, &n2);
	list_insert(&list, &n3);

	list_iterator_t iter;
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	// no dynamic allocation so no need to clean up
}

static void test_list_iterator_insert_remove()
{
	list_t list = { 0 };
	list_node_t n1 = { 0 };
	list_node_t n2 = { 0 };
	list_node_t n3 = { 0 };
	list_node_t n4 = { 0 };
	list_iterator_t iter;

	/* insert into empty list */
	verify(NULL == list_iterate(&list, &iter));
	list_iterator_insert(&iter, &n4);
	verify(NULL == list_iterator_next(&iter));

	verify(&n4 == list_iterate(&list, &iter));
	verify(NULL == list_iterator_next(&iter));

	/* remove from singleton list */
	verify(&n4 == list_iterate(&list, &iter));
	verify(NULL == list_iterator_remove(&iter));
	verify(NULL == list_iterator_next(&iter));

	verify(NULL == list_iterate(&list, &iter));

	/* prepare list from remaining tests (testing interoperation with
	 * list_insert() in the process) */
	verify(NULL == list_iterate(&list, &iter));
	list_iterator_insert(&iter, &n2);
	list_iterator_insert(&iter, &n1);
	list_insert(&list, &n3);
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* insert at beginning */
	verify(&n1 == list_iterate(&list, &iter));
	list_iterator_insert(&iter, &n4);
	verify(&n1 == list_iterator_next(&iter));

	verify(&n4 == list_iterate(&list, &iter));
	verify(&n1 == list_iterator_next(&iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* remove from beginning */
	verify(&n4 == list_iterate(&list, &iter));
	verify(&n1 == list_iterator_remove(&iter));
	verify(&n2 == list_iterator_next(&iter));

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* insert at end */
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));
	list_iterator_insert(&iter, &n4);
	verify(NULL == list_iterator_next(&iter));

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(&n4 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* remove from end */
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(&n4 == list_iterator_next(&iter));
	verify(NULL == list_iterator_remove(&iter));

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* insert in middle */
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	list_iterator_insert(&iter, &n4);
	verify(&n2 == list_iterator_next(&iter));

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n4 == list_iterator_next(&iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* remove from middle */
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n4 == list_iterator_next(&iter));
	verify(&n2 == list_iterator_remove(&iter));

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* test interoperation between remove and list_insert */
	/* (this test is a regression test introduced after code review) */
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_remove(&iter));
	list_insert(&list, &n4);
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n4 == list_iterator_next(&iter));
	verify(NULL == list_iterator_remove(&iter));
	list_iterator_insert(&iter, &n3);
	list_insert(&list, &n4);
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(&n4 == list_iterator_next(&iter));
	list_iterator_remove(&iter);

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	// no dynamic allocation so no need to clean up
}

static void test_list_contains()
{
	list_t list = { 0 };
	list_node_t n1 = { 0 };
	list_node_t n2 = { 0 };
	list_node_t n3 = { 0 };
	list_node_t n4 = { 0 };
	list_iterator_t iter;

	list_insert(&list, &n1);
	verify(true == list_contains(&list, &n1, NULL));
	verify(true == list_contains(&list, &n1, &iter));
	verify(NULL == list_iterator_next(&iter));
	verify(false == list_contains(&list, &n2, NULL));
	verify(false == list_contains(&list, &n2, &iter));

	list_insert(&list, &n2);
	verify(true == list_contains(&list, &n1, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(true == list_contains(&list, &n2, &iter));
	verify(NULL == list_iterator_next(&iter));
	verify(false == list_contains(&list, &n3, &iter));

	list_insert(&list, &n3);
	verify(true == list_contains(&list, &n3, &iter));
	verify(NULL == list_iterator_next(&iter));
	verify(true == list_contains(&list, &n2, &iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(true == list_contains(&list, &n1, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(false == list_contains(&list, &n4, &iter));
}

static void test_list_remove()
{
	list_t list = { 0 };
	list_node_t n1 = { 0 };
	list_node_t n2 = { 0 };
	list_node_t n3 = { 0 };
	list_iterator_t iter;

	verify(false == list_remove(&list, &n1));

	list_insert(&list, &n1);
	verify(true == list_remove(&list, &n1));
	verify(false == list_remove(&list, &n1));
	verify(NULL == list_iterate(&list, &iter));

	list_insert(&list, &n1);
	list_insert(&list, &n2);
	list_insert(&list, &n3);

	/* remove from the beginning */
	verify(true == list_remove(&list, &n1));
	verify(false == list_remove(&list, &n1));
	verify(&n2 == list_iterate(&list, &iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));
	list_push(&list, &n1);

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* remove from the middle */
	verify(true == list_remove(&list, &n2));
	verify(false == list_remove(&list, &n2));
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n3 == list_iterator_next(&iter));
	list_iterator_insert(&iter, &n2);
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	/* remove from the end */
	verify(true == list_remove(&list, &n3));
	verify(false == list_remove(&list, &n3));
	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));
	list_insert(&list, &n3);

	verify(&n1 == list_iterate(&list, &iter));
	verify(&n2 == list_iterator_next(&iter));
	verify(&n3 == list_iterator_next(&iter));
	verify(NULL == list_iterator_next(&iter));

	// no dynamic allocation so no need to clean up
}

int main()
{
	test_list_insert();
	test_list_push();
	test_list_insert_sorted();
	test_list_iterate();
	test_list_iterator_insert_remove();
	test_list_contains();
	test_list_remove();

	return 0;
}

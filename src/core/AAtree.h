

/*
 * Copyright 2008 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_AATREE_H
#define K_AATREE_H


#include <stdbool.h>


typedef struct AAnode
{
	int level;
	void* data;
	struct AAnode* parent;
	struct AAnode* left;
	struct AAnode* right;
} AAnode;


/**
 * This is a balanced binary tree (AA tree). It can store any type of an
 * object as long as the user provides comparison and destructor functions for
 * the type.
 *
 * Without debug code, insertion, removal and search of an element take
 * O(log n) time where n is the total number of elements stored. In debug mode
 * these operations take O(n) time.
 */
typedef struct AAtree
{
	AAnode* nil;
	AAnode* root;
	AAnode* iters[2];
	int (*cmp)(void*, void*);
	void (*destroy)(void*);
} AAtree;


/**
 * Creates a new AAtree.
 *
 * \param cmp       The comparison function for stored elements -- must not be
 *                  \c NULL.
 * \param destroy   The destructor for stored elements -- must not be \c NULL.
 *
 * \return   The new AAtree if successful, or \c NULL if memory allocation
 *           failed.
 */
AAtree* new_AAtree(int (*cmp)(void*, void*), void (*destroy)(void*));


/**
 * Inserts a new element into the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param elem   The new element -- must not be \c NULL.
 */
bool AAtree_ins(AAtree* tree, void* data);


/**
 * Gets the first element greater than or equal to the given key.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 * \param iter   The iterator index -- must be \c 0 or \c 1.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get(AAtree* tree, void* key, int iter);


/**
 * Gets the last element less than or equal to the given key.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 * \param iter   The iterator index -- must be \c 0 or \c 1.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get_at_most(AAtree* tree, void* key, int iter);


/**
 * Gets the element next to the previous one retrieved from the AAtree.
 *
 * If not preceded by a successful call to AAtree_get() with the given
 * iterator index, this function returns \c NULL.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param iter   The iterator index -- must be \c 0 or \c 1.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get_next(AAtree* tree, int iter);


/**
 * Returns an element and removes the corresponding node from the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one was found, otherwise \c NULL.
 */
void* AAtree_remove(AAtree* tree, void* key);


/**
 * Removes all the elements from the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 */
void AAtree_clear(AAtree* tree);


/**
 * Destroys an existing AAtree. All the elements in the tree will also be
 * destroyed.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 */
void del_AAtree(AAtree* tree);


#endif // K_AATREE_H



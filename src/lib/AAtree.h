

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_AATREE_H
#define K_AATREE_H


#include <stdbool.h>


/**
 * This is a balanced binary tree (AA tree). It can store any type of an
 * object as long as the user provides comparison and destructor functions for
 * the type.
 *
 * Without debug code, insertion, removal and search of an element take
 * O(log n) time where n is the total number of elements stored. In debug mode
 * these operations take O(n) time.
 */
typedef struct AAtree AAtree;


typedef struct AAnode AAnode;


/**
 * AAiter is an iterator used for getting elements from an AAtree.
 */
typedef struct AAiter
{
    const AAtree* tree;
    AAnode* node;
} AAiter;

#define AAITER_AUTO (&(AAiter){ .tree = NULL, .node = NULL })


/**
 * Creates an iterator for the AAtree.
 *
 * \param tree   The AAtree, or \c NULL.
 *
 * \return   The new iterator if successful, or \c NULL if memory allocation
 *           failed.
 */
AAiter* new_AAiter(AAtree* tree);


/**
 * Changes the AAtree into which the AAiter is connected.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 * \param tree   The AAtree -- must not be \c NULL.
 */
void AAiter_change_tree(AAiter* iter, const AAtree* tree);


/**
 * Gets the first element greater than or equal to the given key.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAiter_get_at_least(AAiter* iter, const void* key);


/**
 * Gets the last element less than or equal to the given key.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAiter_get_at_most(AAiter* iter, const void* key);


/**
 * Gets the element next to the previous one retrieved through the AAiter.
 *
 * If not preceded by a successful call to AAiter_get*() with the given
 * iterator, this function returns \c NULL.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAiter_get_next(AAiter* iter);


/**
 * Gets the element before the previous one retrieved through the AAiter.
 *
 * If not preceded by a successful call to AAiter_get*() with the given
 * iterator, this function returns \c NULL.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAiter_get_prev(AAiter* iter);


/**
 * Destroys an existing AAiter.
 *
 * \param iter   The AAiter, or \c NULL.
 */
void del_AAiter(AAiter* iter);


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
AAtree* new_AAtree(int (*cmp)(const void*, const void*), void (*destroy)(void*));


/**
 * Finds out if a key exists inside the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   \c true if and only if \a key is found inside \c tree.
 */
bool AAtree_contains(const AAtree* tree, const void* key);


/**
 * Inserts a new element into the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param elem   The new element -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool AAtree_ins(AAtree* tree, void* data);


/**
 * Gets the first element greater than or equal to the given key.
 *
 * This function does not preserve context information and therefore
 * does not have a corresponding "next" variant.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get_at_least(const AAtree* tree, const void* key);


/**
 * Gets the element matching the given key exactly.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get_exact(const AAtree* tree, const void* key);


/**
 * Gets the last element less than or equal to the given key.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get_at_most(const AAtree* tree, const void* key);


/**
 * Returns an element and removes the corresponding node from the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one was found, otherwise \c NULL.
 */
void* AAtree_remove(AAtree* tree, const void* key);


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
 * \param tree   The AAtree, or \c NULL.
 */
void del_AAtree(AAtree* tree);


#endif // K_AATREE_H



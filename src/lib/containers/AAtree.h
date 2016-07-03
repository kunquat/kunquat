

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_AATREE_H
#define KQT_AATREE_H


#include <stdbool.h>
#include <stdlib.h>


/**
 * This is a balanced binary tree (AA tree). It can store any type of an
 * object as long as the user provides comparison and destructor functions for
 * the type.
 *
 * With debugging disabled, insertion, removal and search of an element take
 * O(log n) time where n is the total number of elements stored. In debug mode
 * these operations take O(n) time.
 */
typedef struct AAtree AAtree;


typedef struct AAnode AAnode;

typedef int AAtree_item_cmp(const void*, const void*);
typedef void AAtree_item_destroy(void*);


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
 * Initialise an AAtree iterator.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 * \param tree   The AAtree associated with \a iter -- must not be \c NULL.
 *
 * \return   The parameter \a iter.
 */
AAiter* AAiter_init(AAiter* iter, const AAtree* tree);


/**
 * Get the first element greater than or equal to the given key.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAiter_get_at_least(AAiter* iter, const void* key);


/**
 * Get the last element less than or equal to the given key.
 *
 * \param iter   The AAiter -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAiter_get_at_most(AAiter* iter, const void* key);


/**
 * Get the element next to the previous one retrieved through the AAiter.
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
 * Get the element before the previous one retrieved through the AAiter.
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
 * Get data from an AAnode.
 *
 * \param node   The AAnode -- must not be \c NULL.
 *
 * \return   The data.
 */
void* AAnode_get_data(AAnode* node);


/**
 * Create a new AAtree.
 *
 * \param cmp       The comparison function for stored elements -- must not be
 *                  \c NULL.
 * \param destroy   The destructor for stored elements -- must not be \c NULL.
 *
 * \return   The new AAtree if successful, or \c NULL if memory allocation
 *           failed.
 */
AAtree* new_AAtree(AAtree_item_cmp* cmp, AAtree_item_destroy* destroy);


/**
 * Find out if a key exists inside the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   \c true if and only if \a key is found inside \c tree.
 */
bool AAtree_contains(const AAtree* tree, const void* key);


/**
 * Insert a new element into the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param data   The new element data -- must not be \c NULL and must not match
 *               an existing key.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool AAtree_ins(AAtree* tree, void* data);


/**
 * Attach an AAnode to the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param node   The AAnode -- must not be \c NULL and must not match an
 *               existing key.
 */
void AAtree_attach(AAtree* tree, AAnode* node);


/**
 * Get the first element greater than or equal to the given key.
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
 * Get the element matching the given key exactly.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get_exact(const AAtree* tree, const void* key);


/**
 * Get the last element less than or equal to the given key.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one exists, otherwise \c NULL.
 */
void* AAtree_get_at_most(const AAtree* tree, const void* key);


/**
 * Detach an AAnode from the AAtree without destroying the node.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The AAnode if one was found, otherwise \c NULL.
 */
AAnode* AAtree_detach(AAtree* tree, const void* key);


/**
 * Return an element and removes the corresponding node from the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 * \param key    The key -- must not be \c NULL.
 *
 * \return   The element if one was found, otherwise \c NULL.
 */
void* AAtree_remove(AAtree* tree, const void* key);


/**
 * Remove all the elements from the AAtree.
 *
 * \param tree   The AAtree -- must not be \c NULL.
 */
void AAtree_clear(AAtree* tree);


/**
 * Destroy an existing AAtree.
 *
 * All the elements in the tree will also be destroyed.
 *
 * \param tree   The AAtree, or \c NULL.
 */
void del_AAtree(AAtree* tree);


#endif // KQT_AATREE_H



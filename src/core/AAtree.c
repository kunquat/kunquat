

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <AAtree.h>

#include <xmemory.h>


#define AATREE_MAX_ITERS (2)


struct AAnode
{
    int level;
    void* data;
    struct AAnode* parent;
    struct AAnode* left;
    struct AAnode* right;
};


struct AAiter
{
    AAtree* tree;
    AAnode* node;
};


static AAnode* aapred(AAnode* node);

static AAnode* aasucc(AAnode* node);

static AAnode* aaskew(AAnode* root);

static AAnode* aasplit(AAnode* root);

static void aafree(AAnode* node, void (*destroy)(void*));

#ifndef NDEBUG
#define aavalidate(node, msg) (assert(aavalidate_(node, msg)))
static bool aavalidate_(AAnode* node, char* msg);
#else
#define aavalidate(node, msg) (void)0
#endif


AAiter* new_AAiter(AAtree* tree)
{
    AAiter* iter = xalloc(AAiter);
    if (iter == NULL)
    {
        return NULL;
    }
    iter->tree = tree;
    iter->node = NULL;
    return iter;
}

#define AAITER_AUTO(t) (&(AAiter){ .tree = (t), .node = NULL })


void AAiter_change_tree(AAiter* iter, AAtree* tree)
{
    assert(iter != NULL);
    assert(tree != NULL);
    iter->tree = tree;
    iter->node = NULL;
    return;
}


void del_AAiter(AAiter* iter)
{
    assert(iter != NULL);
    xfree(iter);
    return;
}


static AAnode* new_AAnode(AAnode* nil, void* data);

static AAnode* new_AAnode(AAnode* nil, void* data)
{
    assert(!(nil == NULL) || (data == NULL));
    assert(!(data == NULL) || (nil == NULL));
    AAnode* node = xalloc(AAnode);
    if (node == NULL)
    {
        return NULL;
    }
    if (nil == NULL)
    {
        assert(data == NULL);
        node->data = NULL;
        node->level = 0;
        node->parent = node->left = node->right = node;
    }
    else
    {
        assert(data != NULL);
        node->data = data;
        node->level = 1;
        node->parent = node->left = node->right = nil;
    }
    return node;
}


AAtree* new_AAtree(int (*cmp)(void*, void*), void (*destroy)(void*))
{
    assert(cmp != NULL);
    assert(destroy != NULL);
    AAtree* tree = xalloc(AAtree);
    if (tree == NULL)
    {
        return NULL;
    }
    tree->nil = new_AAnode(NULL, NULL);
    if (tree->nil == NULL)
    {
        xfree(tree);
        return NULL;
    }
    tree->root = tree->nil;
    for (int i = 0; i < AATREE_MAX_ITERS; ++i)
    {
        tree->iters[i] = tree->nil;
    }
    tree->cmp = cmp;
    tree->destroy = destroy;
    aavalidate(tree->root, "init");
    return tree;
}


bool AAtree_ins(AAtree* tree, void* data)
{
    assert(tree != NULL);
    assert(data != NULL);
    aavalidate(tree->root, "before insert");
    if (tree->root->level == 0)
    {
        tree->root = new_AAnode(tree->nil, data);
        if (tree->root == NULL)
        {
            tree->root = tree->nil;
            return false;
        }
        aavalidate(tree->root, "insert root");
        return true;
    }
    AAnode* cur = tree->root;
    AAnode* prev = NULL;
    assert(cur->data != NULL);
    int diff = 1;
    while (cur->level > 0 && diff != 0)
    {
        assert(cur->data != NULL);
        diff = tree->cmp(data, cur->data);
        prev = cur;
        if (diff < 0)
        {
            cur = cur->left;
        }
        else if (diff > 0)
        {
            cur = cur->right;
        }
    }
    assert(prev != NULL);
    if (diff == 0)
    {
        assert(cur != NULL);
        assert(cur->data != NULL);
        tree->destroy(cur->data);
        cur->data = data;
        aavalidate(tree->root, "insert");
        return true;
    }
    AAnode* new_node = new_AAnode(tree->nil, data);
    if (new_node == NULL)
    {
        return false;
    }
    if (diff < 0)
    {
        assert(prev->left->level == 0);
        prev->left = new_node;
        new_node->parent = prev;
    }
    else
    {
        assert(diff > 0);
        assert(prev->right->level == 0);
        prev->right = new_node;
        new_node->parent = prev;
    }
    cur = new_node->parent;
    while (cur->level > 0)
    {
        AAnode* parent = cur->parent;
        AAnode** child = NULL;
        if (parent->left == cur)
        {
            child = &parent->left;
        }
        else if (parent->right == cur)
        {
            child = &parent->right;
        }
        else
        {
            assert(parent->level == 0);
            child = &tree->root;
        }
        assert(child != NULL);
        cur = aaskew(cur);
        cur = aasplit(cur);
        aavalidate(cur, "balance");
        *child = cur;
        cur = cur->parent;
    }
    aavalidate(tree->root, "insert rebalance");
    return true;
}


void* AAiter_get(AAiter* iter, void* key)
{
    assert(iter != NULL);
    assert(key != NULL);
    assert(iter->tree != NULL);
    AAtree* tree = iter->tree;
    aavalidate(tree->root, "get");
    AAnode** last = &iter->node;
    *last = NULL;
    AAnode* ret = tree->nil;
    AAnode* cur = tree->root;
    while (cur->level > 0)
    {
        assert(cur->data != NULL);
        int diff = tree->cmp(key, cur->data);
        if (diff < 0)
        {
            ret = cur;
            *last = cur;
            cur = cur->left;
        }
        else if (diff > 0)
        {
            cur = cur->right;
        }
        else
        {
            *last = cur;
            return cur->data;
        }
    }
    return ret->data;
}


void* AAtree_get(AAtree* tree, void* key)
{
    assert(tree != NULL);
    assert(key != NULL);
    AAiter* iter = AAITER_AUTO(tree);
    return AAiter_get(iter, key);
}


void* AAiter_get_at_most(AAiter* iter, void* key)
{
    assert(iter != NULL);
    assert(key != NULL);
    assert(iter->tree != NULL);
    AAtree* tree = iter->tree;
    aavalidate(tree->root, "get");
    AAnode** last = &iter->node;
    *last = NULL;
    AAnode* ret = tree->nil;
    AAnode* cur = tree->root;
    while (cur->level > 0)
    {
        assert(cur->data != NULL);
        int diff = tree->cmp(key, cur->data);
        if (diff < 0)
        {
            cur = cur->left;
        }
        else if (diff > 0)
        {
            ret = cur;
            *last = cur;
            cur = cur->right;
        }
        else
        {
            *last = cur;
            return cur->data;
        }
    }
    return ret->data;
}


void* AAtree_get_at_most(AAtree* tree, void* key)
{
    assert(tree != NULL);
    assert(key != NULL);
    AAiter* iter = AAITER_AUTO(tree);
    return AAiter_get_at_most(iter, key);
}


void* AAiter_get_next(AAiter* iter)
{
    assert(iter != NULL);
    assert(iter->tree != NULL);
    AAtree* tree = iter->tree;
    if (iter->node == tree->nil)
    {
        return NULL;
    }
    iter->node = aasucc(iter->node);
    if (iter->node == tree->nil)
    {
        return NULL;
    }
    return iter->node->data;
}


void* AAtree_remove(AAtree* tree, void* key)
{
    assert(tree != NULL);
    assert(key != NULL);
    assert(tree->root->parent == tree->nil);
    if (tree->root->level == 0)
    {
        return NULL;
    }
    AAnode* cur = tree->root;
    int diff = 1;
    while (cur->level > 0 && diff != 0)
    {
        diff = tree->cmp(key, cur->data);
        if (diff < 0)
        {
            cur = cur->left;
        }
        else if (diff > 0)
        {
            cur = cur->right;
        }
    }
    assert(cur != NULL);
    if (cur->level == 0)
    {
        return NULL;
    }
    for (int i = 0; i < AATREE_MAX_ITERS; ++i)
    {
        if (cur == tree->iters[i])
        {
            tree->iters[i] = aasucc(tree->iters[i]);
        }
    }
    void* data = cur->data;
    cur->data = NULL;
    if (cur->left->level != 0 && cur->right->level != 0)
    {
        assert(cur->left->level > 0);
        assert(cur->right->level > 0);
        AAnode* pred = aapred(cur);
        assert(pred != NULL);
        assert(pred->right->level == 0);
        cur->data = pred->data;
        pred->data = NULL;
        cur = pred;
    }
    assert(cur->left->level == 0 || cur->right->level == 0);
    AAnode** child = NULL;
    AAnode* parent = cur->parent;
    if (cur == cur->parent->left)
    {
        child = &cur->parent->left;
    }
    else if (cur == cur->parent->right)
    {
        child = &cur->parent->right;
    }
    else
    {
        assert(cur == tree->root);
        child = &tree->root;
        parent = tree->nil;
    }
    assert(child != NULL);
    if (cur->left->level > 0)
    {
        assert(cur->right->level == 0);
        *child = cur->left;
        cur->left->parent = parent;
    }
    else
    {
        *child = cur->right;
        cur->right->parent = parent;
    }
    AAnode* node = cur->parent;
    cur->left = cur->right = tree->nil;
    aafree(cur, tree->destroy);
    cur = node;
    while (cur->level > 0)
    {
        parent = cur->parent;
        child = NULL;
        if (parent->left == cur)
        {
            child = &parent->left;
        }
        else if (parent->right == cur)
        {
            child = &parent->right;
        }
        else
        {
            assert(parent->level == 0);
            child = &tree->root;
        }
        if (cur->left->level < cur->level - 1
                || cur->right->level < cur->level - 1)
        {
            --cur->level;
            if (cur->right->level > cur->level)
            {
                cur->right->level = cur->level;
            }
            assert(child != NULL);
            cur = aaskew(cur);
            cur = aasplit(cur);
            *child = cur;
        }
        aavalidate(cur, "balance");
        cur = cur->parent;
    }
    aavalidate(tree->root, "remove");
    return data;
}


void AAtree_clear(AAtree* tree)
{
    assert(tree != NULL);
    aavalidate(tree->root, "clear");
    if (tree->root != tree->nil)
    {
        aafree(tree->root, tree->destroy);
        tree->root = tree->nil;
    }
    for (int i = 0; i < AATREE_MAX_ITERS; ++i)
    {
        tree->iters[i] = tree->nil;
    }
    return;
}


void del_AAtree(AAtree* tree)
{
    assert(tree != NULL);
    aavalidate(tree->root, "del");
    AAtree_clear(tree);
    xfree(tree->nil);
    xfree(tree);
    return;
}


static AAnode* aapred(AAnode* node)
{
    assert(node != NULL);
    assert(node->level > 0);
    if (node->left->level > 0)
    {
        node = node->left;
        while (node->right->level > 0)
        {
            node = node->right;
        }
        return node;
    }
    AAnode* prev = node;
    node = node->parent;
    while (prev == node->left)
    {
        prev = node;
        node = node->parent;
    }
    return node;
}


static AAnode* aasucc(AAnode* node)
{
    assert(node != NULL);
    if (node->level == 0)
    {
        return node;
    }
    if (node->right->level > 0)
    {
        node = node->right;
        while (node->left->level > 0)
        {
            node = node->left;
        }
        return node;
    }
    AAnode* prev = node;
    node = node->parent;
    while (prev == node->right)
    {
        prev = node;
        node = node->parent;
    }
    return node;
}


static AAnode* aaskew(AAnode* root)
{
    assert(root != NULL);
    if (root->level == 0)
    {
        return root;
    }
    if (root->left->level == root->level)
    {
        AAnode* new_root = root->left;
        root->left = new_root->right;
        new_root->right = root;
        root->left->parent = root;
        new_root->parent = root->parent;
        root->parent = new_root;
        root = new_root;
    }
    root->right = aaskew(root->right);
    return root;
}


static AAnode* aasplit(AAnode* root)
{
    assert(root != NULL);
    if (root->level == 0)
    {
        return root;
    }
    if (root->level == root->right->right->level)
    {
        AAnode* new_root = root->right;
        root->right = new_root->left;
        new_root->left = root;
        root->right->parent = root;
        new_root->parent = root->parent;
        root->parent = new_root;
        root = new_root;
        ++root->level;
        root->right = aasplit(root->right);
    }
    return root;
}


static void aafree(AAnode* node, void (*destroy)(void*))
{
    assert(node != NULL);
    assert(destroy != NULL);
    if (node->level == 0)
    {
        return;
    }
    aafree(node->left, destroy);
    aafree(node->right, destroy);
    if (node->data != NULL)
    {
        destroy(node->data);
    }
    xfree(node);
    return;
}


#ifndef NDEBUG
static bool aavalidate_(AAnode* node, char* msg)
{
    if (node == NULL
            || node->parent == NULL
            || node->left == NULL
            || node->right == NULL)
    {
        return false;
    }
/*  fprintf(stderr, "level %d: parent: %6lx, left: %6lx, this: %6lx, right: %6lx\n",
            node->level,
            (long)node->parent % (long)nil,
            (long)node->left % (long)nil,
            (long)node % (long)nil,
            (long)node->right % (long)nil); */
    if (node->level == 0)
    {
        if (node->data != NULL)
        {
            return false;
        }
        if (node->left != node)
        {
            return false;
        }
        if (node->right != node)
        {
            return false;
        }
        return true;
    }
    if (node->left == node)
    {
        return false;
    }
    if (node->right == node)
    {
        return false;
    }
    if (node->parent == node)
    {
        return false;
    }
    if (node->left->level > 0 && node->left->parent != node)
    {
        return false;
    }
    if (node->right->level > 0 && node->right->parent != node)
    {
        return false;
    }
    if (node->level != node->left->level + 1)
    {
        return false;
    }
    if (node->level == node->right->right->level)
    {
        return false;
    }
    if (node->level != node->right->level
            && node->level != node->right->level + 1)
    {
        return false;
    }
    if (node->data == NULL)
    {
        return false;
    }
    if (aavalidate_(node->left, msg) && aavalidate_(node->right, msg))
    {
        return true;
    }
    return false;
}
#endif



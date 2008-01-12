

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


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <Reltime.h>

#include "Column.h"

#include <xmemory.h>


static AAnode* aapred(AAnode* node);

static AAnode* aasucc(AAnode* node);

static AAnode* aaskew(AAnode* root);

static AAnode* aasplit(AAnode* root);

static void aafree(AAnode* node);

#ifndef NDEBUG
#define aavalidate(node, msg) (assert(aavalidate_(node)))
static bool aavalidate_(AAnode* node);
#else
#define aavalidate(node, msg) (void)0
#endif


Column* new_Column(Reltime* len)
{
	Column* col = xalloc(Column);
	if (col == NULL)
	{
		return NULL;
	}
	col->events.nil = xalloc(AAnode);
	if (col->events.nil == NULL)
	{
		xfree(col);
		return NULL;
	}
	col->events.nil->level = 0;
	col->events.nil->event = NULL;
	col->events.nil->parent = col->events.nil;
	col->events.nil->left = col->events.nil;
	col->events.nil->right = col->events.nil;
	col->events.root = col->events.nil;
	aavalidate(col->events.root, "init");
	if (len != NULL)
	{
		Reltime_copy(&col->len, len);
	}
	else
	{
		Reltime_set(&col->len, INT64_MAX, 0);
	}
	col->last = col->events.nil;
	return col;
}


bool Column_ins(Column* col, Event* event)
{
	assert(col != NULL);
	assert(event != NULL);
	if (col->events.root->level == 0)
	{
		col->events.root = xalloc(AAnode);
		if (col->events.root == NULL)
		{
			col->events.root = col->events.nil;
			return false;
		}
		col->events.root->level = 1;
		col->events.root->event = event;
		col->events.root->parent = col->events.nil;
		col->events.root->left = col->events.nil;
		col->events.root->right = col->events.nil;
		aavalidate(col->events.root, "insert root");
		return true;
	}
	AAnode* new_node = xalloc(AAnode);
	if (new_node == NULL)
	{
		return false;
	}
	new_node->level = 1;
	new_node->event = event;
	new_node->parent = NULL;
	new_node->left = col->events.nil;
	new_node->right = col->events.nil;
	AAnode* cur = col->events.root;
	AAnode* prev = NULL;
	assert(cur->event != NULL);
	int diff = 0;
	while (cur->level > 0)
	{
		diff = Reltime_cmp(Event_pos(event), Event_pos(cur->event));
		prev = cur;
		if (diff < 0)
		{
			cur = cur->left;
		}
		else
		{
			cur = cur->right;
		}
	}
	assert(prev != NULL);
	if (diff < 0)
	{
		assert(prev->left->level == 0);
		prev->left = new_node;
		new_node->parent = prev;
	}
	else
	{
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
			child = &col->events.root;
		}
		assert(child != NULL);
		cur = aaskew(cur);
		cur = aasplit(cur);
		aavalidate(cur, "balance");
		*child = cur;
		cur = cur->parent;
	}
	aavalidate(col->events.root, "insert");
	return true;
}


Event* Column_get(Column* col, const Reltime* pos)
{
	assert(col != NULL);
	assert(pos != NULL);
	Event* ret = NULL;
	AAnode* cur = col->events.root;
	aavalidate(cur, "get");
	while (cur->level > 0)
	{
		int diff = Reltime_cmp(pos, Event_pos(cur->event));
		if (diff < 0)
		{
			ret = cur->event;
			col->last = cur;
			cur = cur->left;
		}
		else if (diff > 0)
		{
			cur = cur->right;
		}
		else
		{
			while (cur->level != 0 && Reltime_cmp(pos, Event_pos(cur->event)) == 0)
			{
				ret = cur->event;
				col->last = cur;
				cur = aapred(cur);
			}
			cur = col->events.nil;
		}
	}
	return ret;
}


Event* Column_get_next(Column* col)
{
	assert(col != NULL);
	assert(col->last != NULL);
	col->last = aasucc(col->last);
	return col->last->event;
}


bool Column_remove(Column* col, Event* event)
{
	assert(col != NULL);
	assert(event != NULL);
	if (col->events.root->level == 0)
	{
		return false;
	}
	AAnode* cur = col->events.root;
	while (cur->level > 0)
	{
		int diff = Reltime_cmp(Event_pos(event), Event_pos(cur->event));
		if (diff < 0)
		{
			cur = cur->left;
		}
		else if (diff > 0)
		{
			cur = cur->right;
		}
		else
		{
			AAnode* right = aasucc(cur);
			while (diff == 0 && cur->event != event)
			{
				cur = aapred(cur);
				if (cur->event == NULL)
				{
					break;
				}
				diff = Reltime_cmp(Event_pos(event), Event_pos(cur->event));
			}
			if (cur->event == event)
			{
				break;
			}
			cur = right;
			diff = Reltime_cmp(Event_pos(event), Event_pos(cur->event));
			while (diff == 0 && cur->event != event)
			{
				cur = aasucc(cur);
				if (cur->event == NULL)
				{
					break;
				}
				diff = Reltime_cmp(Event_pos(event), Event_pos(cur->event));
			}
			if (cur->event == event)
			{
				break;
			}
			cur = col->events.nil;
		}
	}
	assert(cur != NULL);
	if (cur->event != event)
	{
		return false;
	}
	if (cur == col->last)
	{
		col->last = aasucc(col->last);
	}
	if (cur->left->level != 0 && cur->right->level != 0)
	{
		assert(cur->left->level > 0);
		assert(cur->right->level > 0);
		AAnode* pred = aapred(cur);
		assert(pred != NULL);
		assert(pred->right->level == 0);
		cur->event = pred->event;
		pred->event = event;
		cur = pred;
	}
	assert(cur->left->level == 0 || cur->right->level == 0);
	AAnode** child = NULL;
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
		assert(cur == col->events.root);
		child = &col->events.root;
	}
	assert(child != NULL);
	if (cur->left->level > 0)
	{
		assert(cur->right->level == 0);
		*child = cur->left;
	}
	else
	{
		*child = cur->right;
	}
	AAnode* node = cur->parent;
	cur->left = cur->right = col->events.nil;
	aafree(cur);
	cur = node;
	while (cur->level > 0)
	{
		AAnode* parent = cur->parent;
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
			child = &col->events.root;
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
	aavalidate(col->events.root, "remove");
	return true;
}


void Column_clear(Column* col)
{
	assert(col != NULL);
	aavalidate(col->events.root, "clear");
	if (col->events.root != col->events.nil)
	{
		aafree(col->events.root);
		col->events.root = col->events.nil;
	}
	col->last = col->events.nil;
	return;
}


void del_Column(Column* col)
{
	assert(col != NULL);
	aavalidate(col->events.root, "del");
	Column_clear(col);
	xfree(col->events.nil);
	xfree(col);
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


static void aafree(AAnode* node)
{
	assert(node != NULL);
	if (node->level == 0)
	{
		return;
	}
	aafree(node->left);
	aafree(node->right);
	assert(node->event != NULL);
	del_Event(node->event);
	xfree(node);
	return;
}


#ifndef NDEBUG
static bool aavalidate_(AAnode* node)
{
	if (node == NULL
			|| node->parent == NULL
			|| node->left == NULL
			|| node->right == NULL)
	{
		return false;
	}
/*	fprintf(stderr, "level %d: parent: %6lx, left: %6lx, this: %6lx, right: %6lx\n",
			node->level,
			(long)node->parent % (long)nil,
			(long)node->left % (long)nil,
			(long)node % (long)nil,
			(long)node->right % (long)nil); */
	if (node->level == 0)
	{
		if (node->event != NULL)
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
	if (node->event == NULL)
	{
		return false;
	}
	if (node->left->event != NULL
			&& Reltime_cmp(Event_pos(node->left->event), Event_pos(node->event)) > 0)
	{
		return false;
	}
	if (node->right->event != NULL
			&& Reltime_cmp(Event_pos(node->right->event), Event_pos(node->event)) < 0)
	{
		return false;
	}
	if (aavalidate_(node->left) && aavalidate_(node->right))
	{
		return true;
	}
	return false;
}
#endif





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
#include <string.h>

#include <Reltime.h>

#include "Column.h"

#include <xmemory.h>


static AAnode* aapred(AAnode* node);

static AAnode* aasucc(AAnode* node);

static AAnode* aaskew(AAnode* root);

static AAnode* aasplit(AAnode* root);

static void aafree(AAnode* node);

#ifndef NDEBUG
#define aavalidate(node, msg) (assert(aavalidate_(node, msg)))
static bool aavalidate_(AAnode* node, char* msg);
#else
#define aavalidate(node, msg) (void)0
#endif


static Event_list* new_Event_list(Event* event);

static Event_list* new_Event_list(Event* event)
{
	assert(event != NULL);
	Event_list* elist = xalloc(Event_list);
	if (elist == NULL)
	{
		return NULL;
	}
	elist->event = event;
	elist->prev = elist->next = NULL;
	return elist;
}


static void del_Event_list(Event_list* elist);


static AAnode* new_AAnode(AAnode* nil, Event* event);

static AAnode* new_AAnode(AAnode* nil, Event* event)
{
	assert(!(nil == NULL) || (event == NULL));
	assert(!(event == NULL) || (nil == NULL));
	AAnode* node = xalloc(AAnode);
	if (node == NULL)
	{
		return NULL;
	}
	if (nil == NULL)
	{
		assert(event == NULL);
		node->elist = node->elist_tail = NULL;
		node->level = 0;
		node->parent = node->left = node->right = node;
	}
	else
	{
		assert(event != NULL);
		node->elist = new_Event_list(event);
		if (node->elist == NULL)
		{
			xfree(node);
			return NULL;
		}
		node->elist_tail = node->elist;
		node->level = 1;
		node->parent = node->left = node->right = nil;
	}
	return node;
}


Column* new_Column(Reltime* len)
{
	Column* col = xalloc(Column);
	if (col == NULL)
	{
		return NULL;
	}
	col->events.nil = new_AAnode(NULL, NULL);
	if (col->events.nil == NULL)
	{
		xfree(col);
		return NULL;
	}
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
	col->last_elist = NULL;
	col->last_from_host = col->events.nil;
	col->last_elist_from_host = NULL;
	return col;
}


bool Column_ins(Column* col, Event* event)
{
	assert(col != NULL);
	assert(event != NULL);
	aavalidate(col->events.root, "before insert");
	if (col->events.root->level == 0)
	{
		col->events.root = new_AAnode(col->events.nil, event);
		if (col->events.root == NULL)
		{
			col->events.root = col->events.nil;
			return false;
		}
		aavalidate(col->events.root, "insert root");
		return true;
	}
	AAnode* cur = col->events.root;
	AAnode* prev = NULL;
	assert(cur->elist != NULL);
	int diff = 1;
	while (cur->level > 0 && diff != 0)
	{
		assert(cur->elist != NULL);
		diff = Reltime_cmp(Event_pos(event), Event_pos(cur->elist->event));
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
		Event_list* new_last = new_Event_list(event);
		if (new_last == NULL)
		{
			return false;
		}
		assert(cur->elist != NULL);
		assert(cur->elist_tail != NULL);
		new_last->prev = cur->elist_tail;
		cur->elist_tail->next = new_last;
		cur->elist_tail = new_last;
		aavalidate(col->events.root, "insert");
		return true;
	}
	AAnode* new_node = new_AAnode(col->events.nil, event);
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
			child = &col->events.root;
		}
		assert(child != NULL);
		cur = aaskew(cur);
		cur = aasplit(cur);
		aavalidate(cur, "balance");
		*child = cur;
		cur = cur->parent;
	}
	aavalidate(col->events.root, "insert rebalance");
	return true;
}


static AAnode* Column_get_node(Column* col, const Reltime* pos, bool playback);


Event* Column_get(Column* col, const Reltime* pos)
{
	assert(col != NULL);
	assert(pos != NULL);
	AAnode* found = Column_get_node(col, pos, true);
	assert(found != NULL);
	if (found->elist == NULL)
	{
		assert(found->level == 0);
		return NULL;
	}
	assert(found->elist->event != NULL);
	return found->elist->event;
}


Event* Column_get_edit(Column* col, const Reltime* pos)
{
	assert(col != NULL);
	assert(pos != NULL);
	AAnode* found = Column_get_node(col, pos, false);
	assert(found != NULL);
	if (found->elist == NULL)
	{
		assert(found->level == 0);
		return NULL;
	}
	assert(found->elist->event != NULL);
	return found->elist->event;
}


static AAnode* Column_get_node(Column* col, const Reltime* pos, bool playback)
{
	assert(col != NULL);
	assert(pos != NULL);
	AAnode* ret = col->events.nil;
	AAnode* cur = col->events.root;
	aavalidate(cur, "get");
	AAnode** last = &col->last;
	Event_list** last_elist = &col->last_elist;
	if (!playback)
	{
		last = &col->last_from_host;
		last_elist = &col->last_elist_from_host;
	}
	while (cur->level > 0)
	{
		assert(cur->elist != NULL);
		assert(cur->elist->event != NULL);
		int diff = Reltime_cmp(pos, Event_pos(cur->elist->event));
		if (diff < 0)
		{
			ret = cur;
			*last = cur;
			*last_elist = (*last)->elist;
			cur = cur->left;
		}
		else if (diff > 0)
		{
			cur = cur->right;
		}
		else
		{
			*last = cur;
			*last_elist = cur->elist;
			return cur;
		}
	}
	return ret;
}


Event* Column_get_next(Column* col)
{
	assert(col != NULL);
	assert(col->last != NULL);
	if (col->last_elist == NULL)
	{
		return NULL;
	}
	col->last_elist = col->last_elist->next;
	if (col->last_elist == NULL)
	{
		col->last = aasucc(col->last);
		col->last_elist = col->last->elist;
		if (col->last_elist == NULL)
		{
			return NULL;
		}
	}
	return col->last_elist->event;
}


Event* Column_get_next_edit(Column* col)
{
	assert(col != NULL);
	assert(col->last_from_host != NULL);
	if (col->last_elist_from_host == NULL)
	{
		return NULL;
	}
	col->last_elist_from_host = col->last_elist_from_host->next;
	if (col->last_elist_from_host == NULL)
	{
		col->last_from_host = aasucc(col->last_from_host);
		col->last_elist_from_host = col->last_from_host->elist;
		if (col->last_elist_from_host == NULL)
		{
			return NULL;
		}
	}
	return col->last_elist_from_host->event;
}


bool Column_move(Column* col, Event* event, unsigned int index)
{
	assert(col != NULL);
	assert(event != NULL);
	Reltime* pos = Event_pos(event);
	AAnode* target = Column_get_node(col, pos, false);
	if (target->level <= 0)
	{
		assert(target->level == 0);
		assert(false);
		return false;
	}
	Event_list* src = NULL;
	Event_list* shifted = NULL;
	Event_list* cur = target->elist;
	assert(cur != NULL);
	unsigned int idx = 0;
	while (cur != NULL)
	{
		if (idx == index)
		{
			shifted = cur;
		}
		if (event == cur->event)
		{
			src = cur;
			if (src == shifted)
			{
				return false;
			}
			--idx;
		}
		cur = cur->next;
		++idx;
	}
	assert(src != NULL);
	if (src->prev != NULL)
	{
		src->prev->next = src->next;
	}
	else
	{
		target->elist = src->next;
	}
	if (src->next != NULL)
	{
		src->next->prev = src->prev;
	}
	else
	{
		target->elist_tail = src->prev;
	}
	if (shifted == NULL)
	{
		src->prev = target->elist_tail;
		assert(target->elist_tail->prev != NULL);
		target->elist_tail->next = src;
		target->elist_tail = src;
	}
	else
	{
		assert(src != shifted);
		src->next = shifted;
		src->prev = shifted->prev;
		assert(src->next != src);
		assert(src->prev != src);
		if (src->prev != NULL)
		{
			src->prev->next = src;
		}
		else
		{
			target->elist = src;
		}
		if (src->next != NULL)
		{
			src->next->prev = src;
		}
		else
		{
			target->elist_tail = src;
		}
	}
	aavalidate(col->events.root, "move");
	return true;
}


bool Column_remove(Column* col, Event* event)
{
	assert(col != NULL);
	assert(event != NULL);
	assert(col->events.root->parent == col->events.nil);
	if (col->events.root->level == 0)
	{
		return false;
	}
	AAnode* cur = col->events.root;
	int diff = 1;
	while (cur->level > 0 && diff != 0)
	{
		diff = Reltime_cmp(Event_pos(event), Event_pos(cur->elist->event));
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
			Event_list* elist = cur->elist;
			while (elist != NULL && elist->event != event)
			{
				elist = elist->next;
			}
			if (elist == NULL)
			{
				return false;
			}
			Event_list* eprev = elist->prev;
			Event_list* enext = elist->next;
			assert(!(eprev == NULL) || elist == cur->elist);
			assert(!(enext == NULL) || elist == cur->elist_tail);
			del_Event(elist->event);
			xfree(elist);
			if (cur->elist != cur->elist_tail)
			{
				assert(eprev != NULL || enext != NULL);
				if (eprev != NULL)
				{
					eprev->next = enext;
				}
				else
				{
					cur->elist = enext;
				}
				if (enext != NULL)
				{
					enext->prev = eprev;
				}
				else
				{
					cur->elist_tail = eprev;
				}
				aavalidate(col->events.root, "remove w/o balance");
				return true;
			}
			assert(elist == cur->elist);
			cur->elist = cur->elist_tail = NULL;
		}
	}
	assert(cur != NULL);
	assert(cur->elist == NULL);
	if (cur->level == 0)
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
		cur->elist = pred->elist;
		cur->elist_tail = pred->elist_tail;
		pred->elist = pred->elist_tail = NULL;
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
		assert(cur == col->events.root);
		child = &col->events.root;
		parent = col->events.nil;
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
	cur->left = cur->right = col->events.nil;
	aafree(cur);
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


void Column_set_length(Column* col, Reltime* len)
{
	assert(col != NULL);
	assert(len != NULL);
	Reltime_copy(&col->len, len);
	return;
}


Reltime* Column_length(Column* col)
{
	assert(col != NULL);
	return &col->len;
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


static void del_Event_list(Event_list* elist)
{
	assert(elist != NULL);
	assert(elist->event != NULL);
	while (elist != NULL)
	{
		Event_list* next = elist->next;
		assert(elist->event != NULL);
		del_Event(elist->event);
		xfree(elist);
		elist = next;
	}
	return;
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
	if (node->elist != NULL)
	{
		del_Event_list(node->elist);
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
/*	fprintf(stderr, "level %d: parent: %6lx, left: %6lx, this: %6lx, right: %6lx\n",
			node->level,
			(long)node->parent % (long)nil,
			(long)node->left % (long)nil,
			(long)node % (long)nil,
			(long)node->right % (long)nil); */
	if (node->level == 0)
	{
		if (node->elist != NULL)
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
	if (node->elist == NULL)
	{
		return false;
	}
	else
	{
		Event_list* elist = node->elist;
		while (elist != NULL)
		{
			if (elist->event == NULL)
			{
				return false;
			}
			if (elist->prev == NULL)
			{
				if (node->elist != elist)
				{
					return false;
				}
			}
			else
			{
				if (node->elist == elist)
				{
					return false;
				}
				if (elist->prev->next != elist)
				{
					return false;
				}
			}
			if (elist->next == NULL)
			{
				if (node->elist_tail != elist)
				{
					return false;
				}
			}
			else
			{
				if (node->elist_tail == elist)
				{
					return false;
				}
				if (elist->next->prev != elist)
				{
					return false;
				}
			}
			elist = elist->next;
		}
	}
	if (node->left->elist != NULL
			&& Reltime_cmp(Event_pos(node->left->elist->event),
					Event_pos(node->elist->event)) > 0)
	{
		return false;
	}
	if (node->right->elist != NULL
			&& Reltime_cmp(Event_pos(node->right->elist->event),
					Event_pos(node->elist->event)) < 0)
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



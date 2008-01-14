

#include <stdlib.h>
#include <assert.h>

#include <Instrument.h>
#include "Ins_table.h"

#include <xmemory.h>


Ins_table* new_Ins_table(int size)
{
	assert(size > 0);
	Ins_table* table = xalloc(Ins_table);
	if (table == NULL)
	{
		return NULL;
	}
	table->size = size;
	table->res = 8;
	if (size < 8)
	{
		table->res = size;
	}
	table->insts = xnalloc(Instrument*, table->res);
	if (table->insts == NULL)
	{
		xfree(table);
		return NULL;
	}
	for (int i = 0; i < table->res; ++i)
	{
		table->insts[i] = NULL;
	}
	return table;
}


bool Ins_table_set(Ins_table* table, int index, Instrument* ins)
{
	assert(table != NULL);
	assert(index > 0);
	assert(index <= table->size);
	assert(ins != NULL);
	--index;
#ifndef NDEBUG
	for (int i = 0; i < table->res; ++i)
	{
		assert(table->insts[i] != ins);
	}
#endif
	if (index >= table->res)
	{
		int new_res = table->res << 1;
		if (index >= new_res)
		{
			new_res = index + 1;
		}
		Instrument** new_insts = xrealloc(Instrument*, new_res, table->insts);
		if (new_insts == NULL)
		{
			return false;
		}
		table->insts = new_insts;
		for (int i = table->res; i < new_res; ++i)
		{
			table->insts[i] = NULL;
		}
		table->res = new_res;
	}
	if (table->insts[index] != NULL)
	{
		del_Instrument(table->insts[index]);
	}
	table->insts[index] = ins;
	return true;
}


Instrument* Ins_table_get(Ins_table* table, int index)
{
	assert(table != NULL);
	assert(index > 0);
	assert(index <= table->size);
	--index;
	if (index >= table->res)
	{
		return NULL;
	}
	return table->insts[index];
}


void Ins_table_remove(Ins_table* table, int index)
{
	assert(table != NULL);
	assert(index > 0);
	assert(index <= table->size);
	--index;
	if (index >= table->res || table->insts[index] == NULL)
	{
		return;
	}
	del_Instrument(table->insts[index]);
	table->insts[index] = NULL;
	return;
}


void Ins_table_clear(Ins_table* table)
{
	assert(table != NULL);
	for (int i = 0; i < table->res; ++i)
	{
		if (table->insts[i] != NULL)
		{
			del_Instrument(table->insts[i]);
			table->insts[i] = NULL;
		}
	}
	return;
}


void del_Ins_table(Ins_table* table)
{
	assert(table != NULL);
	Ins_table_clear(table);
	xfree(table->insts);
	xfree(table);
	return;
}



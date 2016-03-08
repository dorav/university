/*
 * hash_table.c
 *
 *  Created on: Feb 4, 2016
 *      Author: dorav
 */
#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

table_node hash_find_free(hash_table* table, void* key);

hash_table newHashTablePreAllocated(ObjectMetadata objectMetadata, void* objects, unsigned int tableSize)
{
	hash_table table;

	table.metadata = objectMetadata;
	table.objects = objects;
	table.tableSize = tableSize;
	table.numberOfUsed = 0;

	return table;
}

hash_table newHashTable(ObjectMetadata objectMetadata)
{
	int nodeSize = objectMetadata.objectSize + sizeof(TableNodeData);
	void* objects = calloc(nodeSize, DEFAULT_TABLE_SIZE);

	return newHashTablePreAllocated(objectMetadata, objects, DEFAULT_TABLE_SIZE);
}

int hash(int nonUniqueValue, unsigned int iteration, unsigned int tableSize)
{
	return (nonUniqueValue + iteration) % tableSize;
}

table_node _hash_fetch_meta(hash_table* table, unsigned int index)
{
	table_node node = { 0 };
	unsigned int offset = index * (table->metadata.objectSize + sizeof(TableNodeData));
	void* data = ((char*)table->objects) + offset;

	node.meta = data;
	node.obj = ((char*)data) + sizeof(TableNodeData);

	return node;
}

void* _hash_fetch(hash_table* table, unsigned int index)
{
	return (char*)_hash_fetch_meta(table, index).obj;
}

#define NodeFree 0
#define NodeTaken 1

boolean hash_insert(hash_table* table, void* key, void* object)
{
	table_node freeSpot = { 0 };

	if (hash_find(table, key) != NULL)
	{
		puts("Found record when trying to insert");
		return FAILURE;
	}

	if ((freeSpot = hash_find_free(table, key)).obj == NULL)
		return FAILURE;

	memcpy(freeSpot.obj, object, table->metadata.objectSize);
	freeSpot.meta->status = NodeTaken;
	table->numberOfUsed++;

	return GOOD;
}

table_node NullNode = { 0 };

typedef struct
{
	unsigned int prehash;
	unsigned int iteration;
	table_node node;
	hash_table* table;
}key_hash_iterator;

boolean iter_valid(key_hash_iterator* value)
{
	return value->iteration < value->table->tableSize;
}

void _advance(key_hash_iterator* iter)
{
	unsigned int index = hash(iter->prehash, iter->iteration, iter->table->tableSize);
	iter->node = _hash_fetch_meta(iter->table, index);
}

void set_next(key_hash_iterator* iter)
{
	++(iter->iteration);
	if (iter_valid(iter))
		_advance(iter);
}

key_hash_iterator begin(hash_table* table, KeyType key)
{
	key_hash_iterator first;
	unsigned int prehash = table->metadata.preHash(key);

	first.prehash = prehash;
	first.iteration = -1;
	first.table = table;

	set_next(&first);

	return first;
}

table_node hash_find_free(hash_table* table, void* key)
{
	key_hash_iterator current;

	for (current = begin(table, key); iter_valid(&current); set_next(&current))
	{
		if (current.node.meta->status == NodeFree)
			return current.node;
	}

	return NullNode;
}

const void* hash_find(hash_table* table, void* key)
{
	key_hash_iterator current;

	for (current = begin(table, key); iter_valid(&current); set_next(&current))
	{
		if (current.node.meta->status == NodeFree)
			break;

		if (current.node.meta->status == NodeTaken &&
			table->metadata.isEqual(key, current.node.obj))
			return current.node.obj;
	}

	return NullNode.obj;
}

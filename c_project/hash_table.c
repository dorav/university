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

typedef struct
{
	char status;
} OTableNodeData;

typedef struct
{
	OTableNodeData* meta;
	void* obj;
} OTableNode;

OTableNode hash_find_free(OHashTable* table, const void* key);

OHashTable newOHashTablePreAllocated(ObjectMetadata objectMetadata, void* objects, unsigned int tableSize)
{
	OHashTable table;

	table.metadata = objectMetadata;
	table.objects = objects;
	table.tableSize = tableSize;
	table.numberOfUsed = 0;

	return table;
}

OHashTable newOHashTable(ObjectMetadata objectMetadata)
{
	int nodeSize = objectMetadata.objectSize + sizeof(OTableNodeData);
	void* objects = calloc(nodeSize, DEFAULT_TABLE_SIZE);

	return newOHashTablePreAllocated(objectMetadata, objects, DEFAULT_TABLE_SIZE);
}

int hash(int nonUniqueValue, unsigned int iteration, unsigned int tableSize)
{
	return (nonUniqueValue + iteration) % tableSize;
}

OTableNode _hash_fetch_meta(OHashTable* table, unsigned int index)
{
	OTableNode node = { 0 };
	unsigned int offset = index * (table->metadata.objectSize + sizeof(OTableNodeData));
	void* data = ((char*)table->objects) + offset;

	node.meta = data;
	node.obj = ((char*)data) + sizeof(OTableNodeData);

	return node;
}

void* _hash_fetch(OHashTable* table, unsigned int index)
{
	return (char*)_hash_fetch_meta(table, index).obj;
}

#define NodeFree 0
#define NodeTaken 1

boolean ohash_insert(OHashTable* table, const void* key, void* object)
{
	OTableNode freeSpot = { 0 };

	if (ohash_find(table, key) != NULL)
	{
		return FAILURE;
	}

	if ((freeSpot = hash_find_free(table, key)).obj == NULL)
		return FAILURE;

	memcpy(freeSpot.obj, object, table->metadata.objectSize);
	freeSpot.meta->status = NodeTaken;
	table->numberOfUsed++;

	return GOOD;
}

OTableNode NullNode = { 0 };

typedef struct
{
	unsigned int prehash;
	unsigned int iteration;
	OTableNode node;
	OHashTable* table;
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

key_hash_iterator begin(OHashTable* table, KeyType key)
{
	key_hash_iterator first;
	unsigned int prehash = table->metadata.preHash(key);

	first.prehash = prehash;
	first.iteration = -1;
	first.table = table;

	set_next(&first);

	return first;
}

OTableNode hash_find_free(OHashTable* table, const void* key)
{
	key_hash_iterator current;

	for (current = begin(table, key); iter_valid(&current); set_next(&current))
	{
		if (current.node.meta->status == NodeFree)
			return current.node;
	}

	return NullNode;
}

const void* ohash_find(OHashTable* table, const void* key)
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

struct LTableNode
{
	struct LTableNode* next;
	void* data;
};

LHashTable newLHashTable(ObjectMetadata objectMetadata, int tableSize)
{
	LHashTable table;

	table.metadata = objectMetadata;
	table.numberOfUsed = 0;
	table.objects = calloc (sizeof(struct LTableNode), DEFAULT_TABLE_SIZE);
	table.tableSize = tableSize;

	return table;
}

boolean lhash_insert(LHashTable* table, const void* key, void* object)
{
	int loc = table->metadata.preHash(key) % table->tableSize;
	struct LTableNode* bucket = &((struct LTableNode*)table->objects)[loc];

	while (bucket->data != NULL)
	{
		if (table->metadata.isEqual(key, bucket->data))
			return FAILURE;

		if (bucket->next == NULL)
			break;

		bucket = bucket->next;
	}

	if (bucket->data == NULL)
		bucket->data = object;
	else
	{
		bucket->next = (struct LTableNode*)calloc(sizeof(struct LTableNode), 1);
		bucket->next->data = object;
		bucket->next->next = NULL;
	}

	return GOOD;
}

const void* lhash_find(LHashTable* table, const void* key)
{
	int prehash = table->metadata.preHash(key) % table->tableSize;
	struct LTableNode* bucket = &((struct LTableNode*)table->objects)[prehash];

	while (bucket != NULL && bucket->data != NULL)
	{
		if (table->metadata.isEqual(key, bucket->data))
			return bucket->data;
		bucket = bucket->next;
	}

	return NULL;
}

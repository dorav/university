/*
 * hash_table.h
 *
 *  Created on: Feb 3, 2016
 *      Author: dorav
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include "utility.h"

#define DEFAULT_TABLE_SIZE 29

typedef const void* KeyType;
typedef const void* ObjectType;
typedef unsigned int (*PreHashFunction)(ObjectType);

typedef boolean (*CompareFunction)(KeyType key, ObjectType obj);

typedef struct
{
	PreHashFunction preHash;
	CompareFunction isEqual;
	unsigned int objectSize;

} ObjectMetadata;

typedef struct
{
	char status;
} TableNodeData;

typedef struct
{
	TableNodeData* meta;
	void* obj;
} table_node;

/*
 *
 * Notes: This hash-table uses memcmp to compare two items.
 * 		  It does not support objects with custom comparing methods.
 */
typedef struct
{
	ObjectMetadata metadata;
	unsigned int numberOfUsed;
	unsigned int tableSize;
	void* objects;
} hash_table;

hash_table newHashTablePreAllocated(ObjectMetadata objectMetadata, void* objects, unsigned int tableSize);
hash_table newHashTable(ObjectMetadata objectMetadata);

const void* hash_find(hash_table* table, const void* key);

#define FAILURE 0
#define GOOD 1
boolean hash_insert(hash_table* table, void* key, void* object);

#endif /* HASH_TABLE_H_ */

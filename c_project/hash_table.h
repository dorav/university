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
	ObjectMetadata metadata;
	unsigned int numberOfUsed;
	unsigned int tableSize;
	void* objects;
} OHashTable;

#define FAILURE 0
#define GOOD 1

OHashTable newOHashTablePreAllocated(ObjectMetadata objectMetadata, void* objects, unsigned int tableSize);
OHashTable newOHashTable(ObjectMetadata objectMetadata);

boolean ohash_insert(OHashTable* table, const void* key, void* object);
const void* ohash_find(OHashTable* table, const void* key);

typedef OHashTable LHashTable;

LHashTable newLHashTable(ObjectMetadata objectMetadata, int tableSize);
boolean lhash_insert(LHashTable* table, const void* key, void* object);
void* lhash_find(LHashTable* table, const void* key);

struct LTableNode
{
	struct LTableNode* next;
	void* data;
};

typedef struct
{
	unsigned int index;
	boolean isValid;
	struct LTableNode* current;
	LHashTable* table;
} lhash_iter;

lhash_iter lhash_begin(LHashTable* table);
lhash_iter lhash_end();
void lhash_set_next(lhash_iter* current);

#endif /* HASH_TABLE_H_ */

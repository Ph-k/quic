#pragma once

/* Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)*/

//The data which this hashtable implimentation has
struct stringTuple{
	char* key;
	char* data;
};
typedef struct stringTuple stringTuple;

struct hashNode{ //Value of the cells of the hashtable
	struct stringTuple tuple; //Node data
	struct hashNode* next; //Pointer to the next node (in case of collision, else NULL)
};
typedef struct hashNode hashNode;

struct hashTable{ //HashTable sturcture
	struct hashNode** table; //The table
	unsigned int size; //It's size
};
typedef struct hashTable hashTable;

hashTable* newHashTable(unsigned int size); //Fuction that creates the hashtable
int HashInsert(hashTable* hashT,const char* key,const char* string); //Function inserting new node to the hashtable
unsigned int h(const char* k,unsigned int size); //Hashing function
int destroyHashTable(hashTable* hashT); //Function that frees the memory that both the hashtable and the string records have allocated
stringTuple* hashFind(hashTable* hashT,const char *key); //Seaching string tuple in the hashtable
/* Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "HashTable.h"

//Function that deletes the tuple of strings as they are defined for this hashtable implimentation
void deleteTuple(struct stringTuple* tuple){
	free(tuple->key);
	if(tuple->data!=NULL) free(tuple->data);
}

hashTable* newHashTable(unsigned int size){
	unsigned int i; 

	hashTable* hashT = malloc(sizeof(hashTable)); //Allocating hashtable structure
	
	if(hashT==NULL) return NULL; //if the above memory allocation was not successful 
	
	hashT->size = size; //Initializing hashtable size
	hashT->table = malloc(sizeof(hashNode*) * size); //Allocating table of (size) pointers to hashNodes

	for(i=0; i<hashT->size; i++){
		hashT->table[i]=NULL; //Initializing the empty hash table
	}

	return hashT; //Returning the memory location of the hash table
}

unsigned int h(const char* k,unsigned int size){ //Hash function
	unsigned int i, hash = 5381; //The hash is initialized as the lenght of the key logigal or with a prime number

	for (i=0; i<strlen(k); i++){
		//Each character in the key string characters are used along with some logical operations in order to create the hash
		hash ^= k[i] * 31;
		hash ^= hash << 7;
	}

	return hash%size; //(hash)mod(size) is used to not superpass the size of the hash table
}

int HashInsert(hashTable* hashT,const char* key,const char* string){
	if(key==NULL) return -2;
	hashNode** hashCell = &(hashT->table[ h(key, hashT->size) ]); //& needed since the cell record might need to be modified
	
	while(*hashCell != NULL){//If another record exist in this cell, the list of the cell is traversed untill the end
		if(strcmp((*hashCell)->tuple.key,key)==0){//If the key (string) exist in the hash table, the entry is not inserted
			return -1;
		}
		hashCell = &((*hashCell)->next);
	}

	if(*hashCell == NULL){
		(*hashCell) = malloc(sizeof(hashNode)); //Allocating memory for the new node

		(*hashCell)->tuple.key = malloc(sizeof(char)*(strlen(key)+1));
		strcpy((*hashCell)->tuple.key,key);
		if(string!=NULL){
			(*hashCell)->tuple.data = malloc(sizeof(char)*(strlen(string)+1));
			strcpy((*hashCell)->tuple.data,string);
		}else{
			(*hashCell)->tuple.data = NULL;
		}

		(*hashCell)->next = NULL;
		return 1; //Successfull insertion
	}
	
	return 0;

}

stringTuple* hashFind(hashTable* hashT,const char *key){
	if(key==NULL) return NULL;
	hashNode* hashCell = (hashT->table[h(key, hashT->size)]);	
	
	while( hashCell!=NULL && strcmp(hashCell->tuple.key,key)!=0 ){//If there are collisions at this cell 
		hashCell = hashCell->next; //The record is searched in the list
	}
	
	return (hashCell != NULL) ? &(hashCell->tuple) : NULL; //If the record was found, it's memory location is returned. Otherwise NULL
}

int destroyHashTable(hashTable* hashT){
	hashNode *hashCell,*cellToBeDeleted;
	
	for(int i=0; i<hashT->size; i++ ){ //For all the cells in the hash table
		hashCell = hashT->table[i]; 
		
		while( hashCell !=NULL ){ //And each record that might be in a list form
			//All the momory is freed
			deleteTuple(&(hashCell->tuple));
			cellToBeDeleted = hashCell;
			hashCell = hashCell->next;
			free(cellToBeDeleted);
		}
	}
	
	//In the end
	free(hashT->table); //The memory of the table of the hash table is freed
	free(hashT); //And the hashtable structure
	return 1;
}
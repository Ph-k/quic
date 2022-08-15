/* Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "HashTable.h"

//Function that simply returns the number of digits a number ha
int digitsCount(long int num){
	if(num==0) return 1; //0 has only one digit
	int count=0;
	while(num!=0){
        num=num/10; //Each division with 10 resulting to a non zero value,
        count++; //Means one more digit in our number
    }
	return count;
}

/*Given two strings (which are paths), it combines them to a single path saved on the given string pointer
taking care of all the memory allocation that might be needed*/
void myStringCat(char **concated,const char *string1,const char *string2){
	int i,j=0;

	//Taking care of memory allocation
	if(*concated==NULL){
		//allocating memorry, needed since given string points to NULL
		*concated = malloc( sizeof(char)*(strlen(string1)+strlen(string2)+2) );
		memset(*concated,0,strlen(string1)+strlen(string2)+2);
	}else if(strlen(*concated) < strlen(string1)+strlen(string2)+2 ){
		//allocating memorry, needed since allocated memory is not enough 
		free(*concated);
		*concated = malloc( sizeof(char)*(strlen(string1)+strlen(string2)+2) );
		memset(*concated,0,strlen(string1)+strlen(string2)+2);
	}else{
		//Allocating memory is enough, sipmly initialazing string
		memset(*concated,0,strlen(*concated));
	}

	//Copying first string
	for(i=0; i<strlen(string1); i++){
		(*concated)[j++] = string1[i];
	}

	//Placing '/' if it does not exist in order to seperate the paths
	if(j-1>1 && (*concated)[j-1]!='/') (*concated)[j++] = '/';

	//Copying the second string
	for(i=0; i<strlen(string2); i++){
		(*concated)[j++] = string2[i];
	}

	(*concated)[j] = '\0';//Placing end of string
}

//Given the path of a hardlink and the hardlinks hashtable, it checks if the hardlink has been copied
const char * const checkIfHardLinkCopied(hashTable *hashTable,long unsigned int Inode){
	//Creating string from Inode
	char *InodeS=malloc(sizeof(char)*(digitsCount(Inode)+1));
	sprintf(InodeS,"%ld",Inode);

	//Searching for the Inode in the hashtable
	stringTuple* stuple=hashFind(hashTable,InodeS);
	free(InodeS);
	if(stuple!=NULL){
		return stuple->data;//If it was found, the Inode of the destion hardlink is returned
	}
	return NULL;//Not found
}

//Given the Inode of a hardlink, it saves the indormation that it has been copied in the hashtable
int InsertNewHardlinkINode(hashTable *hashTable,long unsigned int Inode, const char *dest){
	//Creating string from Inode
	char *InodeS=malloc(sizeof(char)*(digitsCount(Inode)+1));
	sprintf(InodeS,"%ld",Inode);

	//Inserting the Inode of the source hardlink and the location of the copied hardlink on the destination
	int ret = HashInsert(hashTable,InodeS,dest);
	free(InodeS);
	return ret;
}

//It checks if a path string is relative or absolute
int checkRelative(const char *path){
	int result=0;
	char *fullpath=realpath(path,NULL);//Getting full path

	//If the given path is not the same as the realpath, it is relvative
	if(fullpath==NULL || strcmp(path,fullpath)!=0) result=1;

	free(fullpath);
	return result;
}

//Given a path, it returns the path of the above directory
char* upThePath(const char *path){
	int i,lastSlash=0;
	for(i=0; i<strlen(path); i++){//Getting location of last '/' of the hierarchy
		if(path[i]=='/' && path[i+1]!='\0') lastSlash = i;
	}

	char* UpperPath = malloc( sizeof(char)*(lastSlash+1) );//Allocating memory for new string

	for(i=0; i<lastSlash; i++)//Copying evrything in the path exept the last thing
		UpperPath[i] =  path[i];

	UpperPath[i]='\0';
	return UpperPath;
}

//Given a relative path to source, it modifies it in order to be relative to source's absulute path
char *modifyRelativePath(const char* source,const char* path){
	char* sourcePath=upThePath(source);
	//    Getting realpath of source
	char *fullpath=realpath(sourcePath,NULL),*result=NULL;
	myStringCat(&result,fullpath,path);//Concating the absolute path to the relative
	free(fullpath);
	free(sourcePath);
	return result;
}

//Given two paths it checks if the path is a subpath of source
int checkSub(const char* source,const char* path){
	int result=0;
	//Getting realpaths
	char *fullpath=realpath(path,NULL);
	if(fullpath==NULL) return -1;
	char *fullSourcePath=realpath(source,NULL);
	if(fullSourcePath==NULL) {free(fullpath); return -2;}

	//The realpath of the path has to be bigger than the realpath of the source in order for it to be a subpath
	if(strlen(fullSourcePath)>strlen(fullpath)) {
		free(fullpath);
		free(fullSourcePath);
		return -1;
	}

	for(int i=0; i<strlen(fullSourcePath); i++){
		if(fullSourcePath[i]!=fullpath[i]){
			result++;//If the realpaths differ, then we don't have a subdirectory
			break;
		}
	}

	if(strlen(fullSourcePath)<strlen(fullpath)){
		if( fullpath[strlen(fullSourcePath)]!='/' )
			result++;
	}
	
	free(fullpath);
	free(fullSourcePath);
	return result;
}

//Given a path and it's source location, it alters it to became a dest location
char* changeSourcePath(const char *path,const char *source,const char* dest){

	//Getting realpaths
	char *realSource=realpath(source,NULL),*realDest=realpath(dest,NULL);
	int j=0,i,pathlength;
	char *changedPath=malloc(sizeof(char)* (strlen(realDest)+2 +strlen(path)-strlen(source)) );

	//Copying the first part of the new path
	strcpy(changedPath,realDest);
	if(realDest[strlen(realDest)-1]!='/'){ changedPath[strlen(realDest)]='/'; j++; }//Adding '/' if nessesery

	//Copying the rest of the new path
	pathlength=strlen(path);
	for(j+=strlen(realDest),i=strlen(realSource); i<=pathlength; j++,i++){
		changedPath[j]=path[i];
	}

	free(realSource);
	free(realDest);
	return changedPath;
}
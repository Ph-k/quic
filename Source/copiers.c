
/* Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "HashTable.h"
#include "utilities.h"

#define TEN_MB 10485760
#define HUNDRED_MB 104857600
#define PermsFromMode(st_mode) st_mode&0b111111111
extern hashTable *pathHashTable, *copiedHardLinksHT;
extern const char *Sroot,*Droot;

//This function checks if two given files are similar, following the definition of the assigment
int checkFileSimilarity(const char* source, const char* dest, int hardLinks){
	struct stat SourceInodeInf,DestInodeInf;

	//If one of the destination/source does not exist, the files are not similar of course
	if(stat(source,&SourceInodeInf)) return -7;
	if(stat(dest,&DestInodeInf)) return -6;

	//Bullet 1 of assigment definition
	if( (SourceInodeInf.st_mode&S_IFMT) == S_IFDIR && (DestInodeInf.st_mode&S_IFMT) != S_IFDIR) return -1;
	if( (DestInodeInf.st_mode&S_IFMT) == S_IFDIR && (SourceInodeInf.st_mode&S_IFMT) != S_IFDIR) return -2;

	if ( (SourceInodeInf.st_mode&S_IFMT) != (DestInodeInf.st_mode&S_IFMT) ) return -3;

	//Bullet 2 of assigment definition
	//recursive check for folders is done through the recursive nature of copyLocation()

	//Bullet 3 of assigment definition
	if (SourceInodeInf.st_size != DestInodeInf.st_size) return -4;

	//Bullet 4 of assigment definition
	if (SourceInodeInf.st_mtime > DestInodeInf.st_mtime) return -5;

	//Bullet 5, the files are the same
	return 0;
}

/*Function is BASED on mycopyfile() of file p21-buffefect.c which was given to us with the assignment,
as Mister Delis noted we might base our function on the given examples (no code parts where simply copied-pasted).
Given the source path of a file, the function copies the file to the destination path*/
int copyFile(const char *source, const char *dest,unsigned int *copiedStuff,unsigned long int *copiedBytes){
	int sourceFD, destFD;
	int buffSize;
	ssize_t bytesRead;
	struct stat InodeInf;
	char *buffer=NULL;

	//Opening the source file only for reading
	sourceFD = open(source,O_RDONLY);
	if (sourceFD == -1) return -1; //If something went wrong we cant complete the copying

	//Reading information about the source file
	if(fstat(sourceFD,&InodeInf)) {close(sourceFD); return -2;}
	//The byffer size which is the size of the chunks of data we will copy in each itteration of reading the data of the file
	if(InodeInf.st_size >= HUNDRED_MB){
		buffSize = HUNDRED_MB;//If the file is bigger than 100mb, the buffer size will be 100mb (maximum size)
	}else if(InodeInf.st_size >= TEN_MB){
		buffSize = TEN_MB;////If the file is smaller than 100mb but biger than 10mb, the buffer size will be 10mb
	}else{
		buffSize = InodeInf.st_size/digitsCount(InodeInf.st_size);//Otherwise the buffersize is set according to the size of the file
	}

	mode_t old_mask = umask(0);//Resseting the permison mask in order for the perimision to be set as they where in the source
	destFD = creat(dest,PermsFromMode(InodeInf.st_mode));//Creating the file, (or turnicating and setting permissions if it existed)
	umask(old_mask);//Resseting the mask back to it's old/default value
	if (destFD == -1) {close(sourceFD); return -3;}//If something went wrong we cant complete the copying

	buffer = malloc(sizeof(char)*buffSize);//Allocating space to copy the chunks of data
	bytesRead = read(sourceFD, buffer, buffSize);//Reading the first chunk of bytes from the source file
	while( bytesRead > 0 ){//While there are bytes read from the source file
		if( write(destFD,buffer,bytesRead) != bytesRead ){//We write the chunk of read bytes to the new file
			bytesRead = -1;//If something went wrong we can not continue
			break;
		}else if(bytesRead>0) *copiedBytes = *copiedBytes + bytesRead;//If copying was succefull, we increase the counter of copyied bytes
		bytesRead = read(sourceFD, buffer, buffSize);//Reading the next chunk
	}

	//Clossing used files and free allocated memory
	close(sourceFD);
	close(destFD);
	free(buffer);

	//Retunring succes message and increasing the copiedfiles counter if the copied ended succefully
	if(bytesRead!=-1){
		(*copiedStuff)++;
		return 0;
	}else{
		return -4;
	}
}

/*Given the source path of a hardlink, the function creates a hardlink to the destination path
NOTE: as requsted, if the hardlink file has not been copied it is first copied and hardlinked on the next hardlinks
 "Στην περίπτωση των ισχυρών δεσμών (hard links) πρέπει τα δεδομένα των αρχείων να μην αντιγράφονται πάνω από μία φορά"*/
int copyHardLinkFile(const char *source, const char *dest,unsigned int *copiedStuff,unsigned long int *copiedBytes, int verbose){
	struct stat SInodeInf,DInodeInf;
	int retVal=0;
	if(stat(source,&SInodeInf)!=0) return 2;//Reading information about the source file

	//Checking if the hardlink has been copied to the destination before
	const char * const hardlinkOnDest = checkIfHardLinkCopied(copiedHardLinksHT,SInodeInf.st_ino);
	if(hardlinkOnDest==NULL){//If this is the first time the hardlink is beeing copied
		copyFile(source,dest,copiedStuff,copiedBytes);//It is copied for the first and only time to the destination
		//The information that the hardlink was copied is saved on the hashtable
		InsertNewHardlinkINode(copiedHardLinksHT,SInodeInf.st_ino,dest);
	}else{//Otherwise the hardlink has been copied once before to the destination
		if( access( dest, F_OK ) == 0 ) {//If the file exists on the destination
			if(stat(hardlinkOnDest,&SInodeInf)!=0) return 2;
			if(stat(dest,&DInodeInf)!=0) return 2;
			if(DInodeInf.st_ino!=SInodeInf.st_ino){//It is deleted if it is a hardlink to another Inode (since it is a different hardlink)
				if(verbose==1) printf("Deleting %s, because is a hard link to wrong Inode=%ld\n",dest,DInodeInf.st_ino);
				remove(dest);
			}else return 0;//In any other case the existing hardlink ok, nothing else to do
		}
		//If the hardlink did not exist, or existed and pointed to another file which now is deleted
		retVal = link(hardlinkOnDest,dest);//The hardlink is reacreted to the file which was copied to the dest once
		if(retVal==0) (*copiedStuff)++;
	}
	return retVal;
}

/*Same as the checkFileSimilarity(), but in addition to the standard checks we also check for soft link simularity
(since if the source is a softlink and the dest is not, the file is not the same)*/
int checkSoftLinkSimilarity(const char* source, const char* dest){
	struct stat SourceInodeInf,DestInodeInf;
	if(lstat(source,&SourceInodeInf)) return -9;
	if(lstat(dest,&DestInodeInf)) return -8;

	if( (SourceInodeInf.st_mode&S_IFMT) == S_IFDIR && (DestInodeInf.st_mode&S_IFMT) != S_IFDIR) return -1;
	if( (DestInodeInf.st_mode&S_IFMT) == S_IFDIR && (SourceInodeInf.st_mode&S_IFMT) != S_IFDIR) return -2;

	//Check for softlink similarity between source/dest
	if( (SourceInodeInf.st_mode&S_IFMT) == S_IFLNK && (DestInodeInf.st_mode&S_IFMT) != S_IFLNK) return -3;
	if( (DestInodeInf.st_mode&S_IFMT) == S_IFLNK && (SourceInodeInf.st_mode&S_IFMT) != S_IFLNK) return -4;

	if ( (SourceInodeInf.st_mode&S_IFMT) != (DestInodeInf.st_mode&S_IFMT) ) return -5;

	if (SourceInodeInf.st_mtime > DestInodeInf.st_mtime) return -7;

	return 0;
}

//Given the source path of a softlink, the function creates a softlink to the destination path
int copySoftLink(const char* source,const char* dest,unsigned int *copiedStuff,unsigned long int *copiedBytes,int verbose){
	struct stat InodeInf;
	int returnVal,buffsize=10;
	char *buffer=NULL, *destLinkPath;

	//Reading the the path of the given softlink (the loop is needed because we don't know the exact size of the path string)
	do{//We use readlink until the string we have for the path is big enough
		if(buffer!=NULL) free(buffer);
		buffer=malloc(sizeof(char)*buffsize);
		memset(buffer,0,buffsize);
		returnVal = readlink(source,buffer,buffsize);//readlink() returns the number of bytes placed in buffer
		buffsize *= 10;
	}while(returnVal==buffsize/10);//If the returned value equals buffsize, then truncation of the path string may have occurred
	
	if(checkSub(Sroot,buffer)==0){//If the soft link is a sublink inside the source
		destLinkPath = changeSourcePath(buffer,Sroot,Droot);//The soft link on the dest points to the corresponding dest path
		free(buffer);
		buffer = destLinkPath;
		if(verbose==1) printf("from inside the source, the link will point at dest: %s\n",destLinkPath);
	}else if(checkRelative(buffer)!=0){//If the soft link path is relative to the source
		destLinkPath = modifyRelativePath(source,buffer);//The link on the dest will also be relative to the source
		free(buffer);
		buffer = destLinkPath;
		if(checkSub(Sroot,buffer)==0){//We have to check if the relative path is also a sublink
			destLinkPath = changeSourcePath(buffer,Sroot,Droot);
			free(buffer);
			buffer = destLinkPath;
			printf("subpath and ");
		}
		if(verbose==1) printf("relative to %s, in order for the link to work it will point at %s\n",Sroot,buffer);
	}else{//otherwise the link is copied as is
		if(verbose==1) printf(", it is absolut and not a subpath to the source, it will be copied as is\n");
	}

	returnVal = symlink(buffer,dest);//Creating soft link

	if(returnVal==0){//If the soft link was created succefully the counters are increased
		lstat(dest,&InodeInf);
		*copiedBytes = *copiedBytes + InodeInf.st_size;
		(*copiedStuff)++;
	}

	free(buffer);
	return returnVal;
}

//Function that empties a directory from all the files it might have
int emptyDir(const char* path){
	char *recPath=NULL;
	DIR *dp=opendir(path);
	if(dp==NULL) return -1; //If something went wrong we can't continue
	struct 	dirent *direntp;
	struct stat InodeInf;

	while ( (direntp=readdir(dp)) != NULL ){//While there are files/directories in the directory
		if(strcmp(direntp->d_name,".")==0 || strcmp(direntp->d_name,"..")==0) continue; //Skipping the entries '.' & '..'
		myStringCat(&recPath,path, direntp->d_name);//The path of the file/directory is created
		if(stat(recPath,&InodeInf)) { if(recPath!=NULL) free(recPath); return -1;}
		
		if((InodeInf.st_mode & S_IFMT) == S_IFDIR){//If we have encouter a directory
			emptyDir(recPath);//It is recursively emptied
		}
		remove(recPath);//An then deleted (will work for both empty directories and files)
	}

	closedir(dp);
	if(recPath!=NULL) free(recPath);
	return 0;
}

//This function basicaly impliments the -d flag, deleting all the files that exist on the dest but not in the source
int deleteExcessFiles(const char* source,const char* dest,int verbose){
	int i;
	struct stat InodeInf;
	struct 	dirent *Ddirentp;
	DIR *destdp = opendir(dest);
	char *recSourcePath=NULL,*recDestPath=NULL,*SourcePath=NULL,*DestPath=NULL;

	while ( (Ddirentp=readdir(destdp)) != NULL ){//For the files/directories on the destination
		if(strcmp(Ddirentp->d_name,".")==0 || strcmp(Ddirentp->d_name,"..")==0) continue; //Skipping the entries '.' & '..'
		myStringCat(&SourcePath,source, Ddirentp->d_name);
		if (stat(SourcePath,&InodeInf)==-1 && errno == ENOENT){//IF the file/directory does not exist in source
			errno=0;
			myStringCat(&DestPath,dest, Ddirentp->d_name);//Creating path string of the file/directory
			if(verbose==1) printf("Deleting %s because it exists only in destination\n",DestPath);
			lstat(DestPath,&InodeInf);
			switch (InodeInf.st_mode & S_IFMT){
				case S_IFDIR://If we encoutered a directory, it is emptied from excess files recusively
					recSourcePath=malloc(sizeof(char)*(strlen(SourcePath)+1));
					strcpy(recSourcePath,SourcePath);
					recDestPath=malloc(sizeof(char)*(strlen(DestPath)+1));
					strcpy(recDestPath,DestPath);

					i = deleteExcessFiles(recSourcePath,recDestPath,verbose);
					free(recSourcePath); recSourcePath=NULL;
					free(recDestPath); recDestPath=NULL;
					if(i!=0) return i;
					if(rmdir(DestPath)!=0) return -4;
					break;
				case S_IFREG://If we encoutered a file, it is simply deleted
					if(remove(DestPath)!=0) return -2;
					break;
				case S_IFLNK://If we encoutered a link, it is simply deleted
					if(remove(DestPath)!=0) return -2;
					break;
				default:
					return -3;
					break;
			}
			
		}
	}

	if(SourcePath!=NULL) free(SourcePath);
	if(DestPath!=NULL) free(DestPath);
	if(recSourcePath!=NULL) free(recSourcePath);
	if(recDestPath!=NULL) free(recDestPath);
	closedir(destdp);
	return 0;
}

//This function recursively impliments the copying of a hierarchy
int copyLocation(const char *source, const char *dest,
				 int verbose,int deleted, int links,
				 unsigned int *seenStuff,unsigned int *copiedStuff,unsigned long int *copiedBytes){

	int simVal;
	struct stat InodeInf;
	if(links==1){//If we take links into consideration...
		if(lstat(source,&InodeInf)) return -1;//... We have to check soft link seperatly ...
	}else{
		if(stat(source,&InodeInf)) return -1; //... otherwise stat will treat soft links like as files
	}

	switch (InodeInf.st_mode & S_IFMT){//Switch case determines what we will do with the path according to what it is
		case S_IFDIR: //Directory encountered case
			(*seenStuff)++;
			//Body after the switch
			break;
		case S_IFREG: //File encountered case
			simVal = checkFileSimilarity(source,dest,links); //Checking the simularity between the source and dest file
			(*seenStuff)++;
			if(simVal==0){
				return 0; //Same files, we don't copy ( quic wins case ;-) )
			}else if(simVal==-2){ // -2 means source is file but dest is directory
				if(verbose==1) printf("Deleting %s because it is different from the source equivalent\n",dest);
				emptyDir(dest); //So we empty the directory
				remove(dest); //And delete it, in order for the copied file to take it's place
			}else if(simVal!=-6){ //In any other case (expect -6 which means that dest does not exist)
				if(verbose==1) printf("Deleting %s because it is different from the source equivalent\n",dest);
				remove(dest); //We delete the dest file because the files are not similar
			}

			if(!(InodeInf.st_nlink>1)){//If we have not encoutered a hardlink
				printf("%s\n",source); //Printing which file will be copied
				return (copyFile(source,dest,copiedStuff,copiedBytes)==0)? 0 : -2; //We simply copy the file
			}else if(links==1){//If we encoutered a hardlink it copied only with -l flag (explanation on readme)
				printf("%s\n",source); //Printing which file will be copied
				return (copyHardLinkFile(source,dest,copiedStuff,copiedBytes,verbose)==0)? 0 : -2;
			}else{
				if(verbose==1) printf("%s, is a hardlink, no link flag was given so won't be copied\n",source);
			}
			return 0;
			break;
		case S_IFLNK: //Soft link encountered case

			simVal = checkSoftLinkSimilarity(source,dest);  //Checking soft link simularity
			(*seenStuff)++;
			if(simVal==0){
				return 0; //Same links, we don't copy
			}else if(simVal==-2){ // -2 means source is file but dest is directory
				if(verbose==1) printf("Deleting %s because it is different from the source equivalent\n",dest);
				emptyDir(dest); //So we empty the directory
				remove(dest); //And delete it, in order for the copied file to take it's place
			}else if(simVal!=-8){ //In any other case (expect -8 which means that dest does not exist)
				if(verbose==1) printf("Deleting %s because it is different from the source equivalent\n",dest);
				remove(dest);
			}

			printf("%s\n",source); //Printing which file will be copied
			if(verbose==1) printf("the above is a soft link%c",(links==1)? ' ': '\n');
			if(links==0){//If -l was not given the soft link file is copied
				return (copyFile(source,dest,copiedStuff,copiedBytes)==0)? 0 : -3;
			}else{//Otherwise the soft link is copied as a softlink
				return (copySoftLink(source,dest,copiedStuff,copiedBytes,verbose)==0)? 0 : -3;
			}
			break;
		default:
			return 10;
			break;
	}

	//Directory case body:

	//Checking if the given directory Inode has been copied
	char* tempBuf = malloc(sizeof(char)*(digitsCount(InodeInf.st_ino)+1));
	sprintf(tempBuf,"%ld",InodeInf.st_ino);
	if(HashInsert(pathHashTable,tempBuf,NULL)==-1){//If it has, this mean that if we keep copying the directory we will fall in a loop
		if(verbose==1) printf("%s leads to a loop. It won't be copied\n",dest);
		free(tempBuf);
		return 1;//The recursion (and the further copy from this point) is stoped in order to avoid the loop
	}else free(tempBuf);
	//If the directory has not been copied, we can continue safely

	DIR *ddp=opendir(source),*sdp;
	struct 	dirent *direntp;
	char *recSourcePath = NULL,*recDestPath = NULL;

	if(ddp == NULL) {perror("Directory does not exist!\n"); return -1;}

	//Checking the simularity between source and destination
	if(checkFileSimilarity(source,dest,0)==-1){// -1 means source is directory but dest not
		if(remove(dest)!=0) return -2;//They can not co-exist so the file on the destination is deleted
	}

	mode_t old_mask = umask(0);
	sdp = opendir(dest);//Trying to open the directory on the destination
	if(sdp){// If directory exists
		//chmod syscall is used in order to make sure the perimision are the same between the source and the dest
		chmod(dest,PermsFromMode(InodeInf.st_mode));
		umask(old_mask);
	}else if(errno == ENOENT) {//If the directory does not exist

		//It is created
		if(mkdir(dest,PermsFromMode(InodeInf.st_mode))==-1){
			umask(old_mask);
			if(errno == EEXIST ){
				errno = 0;
			}else{
				closedir(ddp); 
				return -4; //If something wen't terably wrong with the creatioing procces we can't constinue
			}
		}else{//Directory was created succesfully
			umask(old_mask);
			printf("created directory %s\n",dest);
			
			if(stat(dest,&InodeInf)) return -1;
			char* tempBuf = malloc(sizeof(char)*(digitsCount(InodeInf.st_ino)+1));
			sprintf(tempBuf,"%ld",InodeInf.st_ino);
			//Both inserting the created/copied directory in the copied directories hashtable, and also checking for loops
			if(HashInsert(pathHashTable,tempBuf,NULL)==-1){
				if(verbose==1) printf("%s leads to a loop. Evrything bellow it won't be copied\n",dest);
				free(tempBuf);
				return 1;
			}else free(tempBuf);

			(*copiedStuff)++;
		}

	}else{
		umask(old_mask);
		return -3;
	}

	if(deleted==1){//If the -d flag was given we have to delete any excess files that might exist in the destination
		deleteExcessFiles(source,dest,verbose);//Function that does the above is called
	}

	while ( (direntp=readdir(ddp)) != NULL ){//For all the files/directories of the source directory
		if(strcmp(direntp->d_name,".")==0 || strcmp(direntp->d_name,"..")==0) continue; //Skipping the entries '.' & '..'
		//The new paths for the file/directory are created
		myStringCat(&recSourcePath,source, direntp->d_name);
		myStringCat(&recDestPath,dest, direntp->d_name);
		//And the directories/files are copied recursively
		copyLocation(recSourcePath,recDestPath,verbose,deleted,links,seenStuff,copiedStuff,copiedBytes);
	}

	if(recSourcePath!=NULL) free(recSourcePath);
	if(recDestPath!=NULL) free(recDestPath);
	closedir(ddp);
	closedir(sdp);
	return 0;
}
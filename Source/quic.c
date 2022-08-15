
/* Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>
#include "copiers.h"
#include "HashTable.h"
#include "utilities.h"

hashTable* pathHashTable, *copiedHardLinksHT;
const char *Sroot,*Droot;

int main(int argc, char **argv){

	if(argc<3) {printf("quic: check arguments format!\n"); return -1;} //quic needs at least 2 arguments

	int i;
	char verbose=0,deleted=0,links=0,desRed=0;

	for(i=1; i<argc; i++){//Reading all the given arguments
		if( strcmp(argv[i],"-v") == 0 ){
			verbose = 1;
		}else if( strcmp(argv[i],"-d") == 0){
			deleted = 1;
		}else if( strcmp(argv[i],"-l") == 0){
			links = 1;
		}else if( desRed == 0 ){//Reading source path
			desRed++;//Source path read, the next will be the dest
			Sroot = argv[i];
		}else if(desRed == 1){//Reading dest path
			desRed++;//Both paths were read
			Droot = argv[i];
		}else{
			printf("Argument '%s' is invalid\n",argv[i]);
		}
	}

	if(desRed!=2){//Both the source and the dest path have to be given in order for the program to work
		printf("quic: one or both of the origin and destination parameters where mistyped or not given at all!\n");
		return -2;
	}

	if(access(Sroot,F_OK)==-1){//The source path has to exist...
		printf("%s does not exist\n",Sroot);
		return -3;
	}

	//...And so should the dest path
	char *beginning = upThePath(Droot);
	struct stat InodeInf;
	if(stat(beginning, &InodeInf) == -1 && errno == ENOENT) {
		errno = 0;
		printf("path %s does not exist\n",Droot);
		free(beginning);
		return -4;
	}
	free(beginning);

	//Initializing the hashtables needed to keep information...
	pathHashTable = newHashTable(100);//...about copied forlders in order to avoid loops and...
	copiedHardLinksHT = newHashTable(50);//...about copied hardlinks in order to trasfer the hardlinks as requested

	unsigned int entitiesCounter=0,entitiesCopied=0;
	unsigned long int bytesCopied=0;
	struct tms startb, endb;
	double ticspersec = (double) sysconf(_SC_CLK_TCK),start,total;

	start = (double) times(&startb);//Starting time of the execution

	//Starting copying all the files/directories of the hierarchy recursively, the recursion of copyLocation() starts here
	if( copyLocation(Sroot,Droot,verbose,deleted,links,&entitiesCounter,&entitiesCopied,&bytesCopied)==0 && verbose==1){
		printf("Copying from %s to %s ended\n",Sroot,Droot);//Printing succes message if verbose flag is on
	}

	total = ( (double) times(&endb) - start) / ticspersec;//End time of procces execution

	//Deleting the hashtables
	destroyHashTable(pathHashTable);
	destroyHashTable(copiedHardLinksHT);

	printf("there are %d files/directories in the hierarchy\nnumber of entities copied is %d\ncopied %ld bytes in %fsec at %f bytes/sec\n",
	entitiesCounter,entitiesCopied,bytesCopied,total,bytesCopied/total);
	return 0;
}
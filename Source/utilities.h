/* Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)*/

//Auxiliary functions which are neccesery for the operation of the program

int digitsCount(long int num); //Function that simply returns the number of digits a number has

/*Given two strings (which are paths), it combines them to a single path saved on the given string pointer
taking care of all the memory allocation that might be needed*/
void myStringCat(char **concated,const char *string1,const char *string2);

//Given the path of a hardlink and the hardlinks hashtable, it checks if the hardlink has been copied
const char * const checkIfHardLinkCopied(hashTable *hashTable,long unsigned int Inode);

//Given the Inode of a hardlink, it saves the indormation that it has been copied in the hashtable
int InsertNewHardlinkINode(hashTable *hashTable,long unsigned int Inode, const char *dest);

//Given two paths it checks if the path is a subpath of source
int checkSub(const char* source,const char* path);

//Given a path and it's source location, it alters it to became a dest location
char* changeSourcePath(const char *path,const char *source,const char* dest);

//It checks if a path string is relative or absolute
int checkRelative(const char *path);

//Given a relative path to source, it modifies it in order to be relative to source's absulute path
char *modifyRelativePath(const char* source,const char* path);

//Given a path, it returns the path of the above directory
char* upThePath(const char *path);
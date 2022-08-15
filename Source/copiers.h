/* Code from https://github.com/Ph-k/quic. Philippos Koumparos (github.com/Ph-k)*/

//These are the only two functions quic.c needs in order to impliment the program
int copyFile(const char *source, const char *dest,unsigned int *copiedStuff,unsigned long int *copiedBytes);
int copyLocation(const char *source, const char *dest,
                 int verbose,int deleted, int links,
                 unsigned int *seenStuff,unsigned int *copiedStuff,unsigned long int *copiedBytes);
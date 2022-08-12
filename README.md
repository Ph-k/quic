# quic
Copy faster by copying less

*quic was written as a part of the Operating System course at DIT – UoA. Credits to the professor of the course at the time, [Alex Delis](https://www.alexdelis.eu/).*
# About this project:
quic (**qu**ick **i**ncremental **c**opy) is a program that copies files like cp, but with the difference is that it tries to copy only the necessary files. 
For example, let’s assume that you have a hierarchy of files/a directory that you frequently backup to an external drive. 
The first time you run quic, it will simply copy all the files to the external drive *(just like cp would)*:
```
```
But the next time you backup your files, only the modified *or newly added* files will be copied (in contrast to cp which will copy identical files). *quic also handles links (symbolic or hard)*:
```
```
Thus, it’s obvious that quic will take less time to copy files when some are already in the destination.

# But... how does quic know which files to copy?
For new files, the answer is trivial, quic just copies them. 

Now, for files that already exist on the destination quic cannot check their contains bit by bit since this would defeat the whole purpose of skipping the copying of the file. Consequently, to separate “identical” from “different” files, quic goes by the following definition:

Using information from the [i-nodes](https://en.wikipedia.org/wiki/Inode), we assume that two files *(S and D)* are **different** when:
-	S corresponds to a directory and D to a file *(or vice versa)*, obviously.
-	If S and D are both directories, we do not modify any of them BUT quic recursively checks their contains *(both directory and files)*, for any new or modified data using this definition. 
-	S and D are both files and:
    -	S and D differ in size, obviously.
    -	S and D are the same size, but they have different modification dates.

In any other case, we assume that files S and D, with their respective [i-nodes](https://en.wikipedia.org/wiki/Inode) are “**identical**”, and thus are not re-copied.

Regarding links, the behavior of quic changes according to the user’s preference.
-	For symbolic links. If the user wants to preserver links, the same links are created in the destination, (only pointing to the destination if the link happens to point in the source). Otherwise, If the user does not want to preserver links, the regular files they point to are copied.
-	Hard links are only created if the user chooses to perverse links, this ensures the contents of it are not copied more than one time *(ex i-nodes a1 and a2 point to the same source file, in the destination you will find i-nodes b1 and b2 point to the same file)*. In any other case hard links are copied as regular files. 


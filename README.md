# quic 🐇
Copy faster by copying less

*quic was written as a part of the Operating System course at DIT – UoA. Credits to the professor of the course at the time, [Alex Delis](https://www.alexdelis.eu/).*
# About this project:
quic (quick incremental copy) is a linux/unix program that copies files like cp, but with the difference that it copies only the necessary files. 

For example, let’s assume that you have a hierarchy of files/a directory that you frequently backup to an external drive. At the time of your first backup, quic will copy all the files of the hierarchy to the external drive (just like cp):
```
```
It is the next time you backup your files, that the intelligence of quic comes into play. By taking advantage of the fact that only the modified or newly added files need to be copied *(in contrast cp would copy all the files, including identical ones)*. 
```
```
Thus, it’s obvious that quic will take less time to copy your files, since it doesn’t bother to re-copy identical files from the source to the destination.

# But... how does quic know which files to copy?
For new files, the answer is trivial, quic just copies them. 

Now, for files that already exist on the destination, quic cannot check their contents bit by bit since this would defeat the whole purpose of skipping the copying of the file. Consequently, to separate “identical” from “different” files, quic goes by the following definition:

Using information from the [i-nodes](https://en.wikipedia.org/wiki/Inode), we assume that two files *(S and D)* are **different** when:
-	S corresponds to a directory and D to a file *(or vice versa)*, obvious.
-	If S and D are both directories, we do not modify any of them. BUT, to ensure that the directories contain the same files, quic recursively checks them for any new or modified data using this definition.
-	S and D are both files and:
    -	S and D differ in size, obvious.
    -	S and D are the same size, but they have different modification dates.

In any other case, we assume that files S and D, with their respective [i-nodes](https://en.wikipedia.org/wiki/Inode) are “**identical**”, and thus are not re-copied.

Regarding links (symbolic or hard), the behavior of quic changes according to the user’s preference..
-	For symbolic links. If the user chooses to preserve links, the same symbolic links are created in the destination, (only pointing to the destination if the link happens to point in the source). Otherwise, If the user does not want to preserve links, the regular files they point to are copied to the destination.
-	Hard links are created only if the user chooses to preserve links, this ensures that their contents are not copied more than one time *(ex if i-nodes a1 and a2 point to the same source file, in the destination you will find i-nodes b1 and b2 pointing to the destination same file)*. In any other case hard links are copied as regular files.



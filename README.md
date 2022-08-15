
# quic 🐇
Copy faster by copying less

*quic was written as a part of the Operating System course at DIT – UoA. Credits to the professor of the course at the time, [Alex Delis](https://www.alexdelis.eu/).*
# About this project:
quic (**qu**ick **i**ncremental **c**opy) is a linux/unix program that copies files like cp, but with the difference that it copies only the necessary files. 

For example, let’s assume that you have a hierarchy of files/a directory that you frequently backup to an external drive. At the time of your first backup, quic will copy all the files of the hierarchy to the external drive (just like cp -r):
```
$quic ./AI ./Usb-Drive/Back-Up/AI
created directory ./Usb-Drive/Back-Up/AI
./AI/csp.py
created directory ./Usb-Drive/Back-Up/AI/rlfap
./AI/rlfap/var2-f24.txt
./AI/rlfap/ctr2-f25.txt
./AI/rlfap/dom2-f24.txt
./AI/rlfap/dom2-f25.txt
./AI/rlfap/ctr2-f24.txt
./AI/rlfap/var2-f25.txt
./AI/myCSP.py
./AI/utils.py
./AI/search.py
./AI/main.py
there are 13 files/directories in the hierarchy
number of entities copied is 13
copied 176096 bytes in 0.120000sec at 1467466.666667 bytes/sec
```
The next time you backup your files, you only actually need to copy the modified *(assume: myCSP.py, var2-f24.txt, dom2-f24.txt, ctr2-f24.txt in the example)* or newly added files *(notes.txt)*. And that is where the intelligence of quic comes into play, gaining time by skipping non-modified/old files  (in contrast cp -r would copy all the files, including identical ones).
```
$quic ./AI ./Usb-Drive/Back-Up/AI
./AI/rlfap/var2-f24.txt
./AI/rlfap/dom2-f24.txt
./AI/rlfap/ctr2-f24.txt
./AI/myCSP.py
./AI/notes.txt
there are 14 files/directories in the hierarchy
number of entities copied is 5
copied 26228 bytes in 0.070000sec at 374685.714286 bytes/sec
```
Thus, it’s obvious that quic will take less time to copy your files, since it doesn’t bother to re-copy identical files from the source to the destination.

## But... how does quic know which files to copy?
For new files, the answer is trivial, quic just copies them. 

Now, for files that already exist on the destination, quic cannot check their contents bit by bit since this would defeat the whole purpose of skipping the copying of the file. Consequently, to separate “identical” from “different” files, quic goes by the following definition:

Using information from the [i-nodes](https://en.wikipedia.org/wiki/Inode), we assume that two files *(S and D)* are **different** when:
-	S corresponds to a directory and D to a file *(or vice versa)*, obvious.
-	If S and D are both directories, we do not modify any of them. BUT, to ensure that the directories contain the same files, quic recursively checks them for any new or modified data using this definition.
-	S and D are both files and:
    -	S and D differ in size, obvious.
    -	S and D are the same size, but they have different modification dates.

In any other case, we assume that files S and D, with their respective [i-nodes](https://en.wikipedia.org/wiki/Inode) are “**identical**”, and thus are not re-copied.

### How are [links](https://www.linux.com/topic/desktop/understanding-linux-links/) handled ?
Regarding links (symbolic or hard), the behavior of quic changes according to the user’s preference..
-	For symbolic links. If the user chooses to preserve links, the same symbolic links are created in the destination, (only pointing to the destination if the link happens to point in the source). Otherwise, If the user does not want to preserve links, the regular files they point to are copied to the destination.
-	Hard links *(files with the same inode, with st_nlink > 1)* are created only if the user chooses to preserve links, this ensures that their contents are not copied more than one time *(ex if i-nodes a1 and a2 point to the same source file, in the destination you will find i-nodes b1 and b2 pointing to the destination same file)*. In any other case hard links are ignored.

# Compilation:

By typing `make` on a tty in the source code directory, all the necessary programs will be compiled, creating all the necessary executables. The only requirement is a linux/unix machine running a stable version of the gcc compiler. *The program worked fine on Debian based distros.*

# Usage

The program can be executed from a cli as ` ./quic -v -d -l origindir destdir`  where:

- `quic` is the executable of the project

- `origindir` is the source path and `destdir` the destination path

- `-v` is the verbose flag. It makes the program output more diagnostic information regarding the copying/deletion of files.

- When the`-d` (delete) flag is used, files that do not exist in the origindir will not exist in the destdir, they are deleted.

- `-l` (links) flag is used to tell the program to preserve links, the behavior of this flag has been described above.


# Important notes:


- There are situations when symbolic links can create circles *(ex, folder DIR has a symbolic link pointing to himself)*. This can result to quic entering an infinite loop when not using the `-l` flag. To avoid these infinite loops the copied files are saved in hastable, with the contents of circles beeing copied once.

- For convenience, whenever this documentation refers to hard links it refers **only** to two or more files with the same inode, which using the syscall `stat` results to an st_nlink field with a value greater than one.

- Note that currently the destination path needs to exist for quic to work *(except when copying a directory, then the destination directory might not exist)*. quic will check the existence of the destination path and inform the user with an error message if it does not exist.


- If quic is ran without the `-l` flag, and then re-ran with it:
  - First execution without `-l` flag: soft links are copied as files, avoiding infinite circles. Hard links are ignored.

  - Second execution with `-l` flag: The destination files which were copied from soft links are replaced with their respective soft links. Hard links will now be copied

- If quic is ran with the `-l` flag, and then re-ran or without it:

  - First execution with `-l` flag: soft links are copied as soft links. Hard links are also copied.
  
  - Second execution without `-l` flag: It was chosen not to change the structure of the destination, since all the necessary information is already copied to the destination.
  
# Source code files overview:

`quic.c`: Contains the main function which reads the command line arguments, initializes all the necessary data structures and variables of the program, and then starts the recursive copying of all the directories/files included in the source path using functions from the copiers module. After that, it prints some information about the whole procedure, de-allocates all the dynamically allocated memory, and terminates the program

` copiers.c & copiers.h`: The only functions witch the user of the module would want to use are the copyFile() and copyLocation() *(copies hierarchies/directories, not just single files)*. All the other functions are hidden in the module and are just used from the two mentioned functions.

`utilities.c` & `utilities.h`: Here you will find several utility functions used to abstract some string manipulation, and access to information about copied inodes routines.

`HashTable.c` & `HashTable.h`: As you might have already noticed there are occasions where quic needs to know if a file has already been copied (circles when not using the `-l` flag). For this purpose a data structure is needed, and since hash tables can provide us with insertion and access of this information in O(1), it was chosen.

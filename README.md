# My Unix Programs
A little collection of basic Unix/Linux command line programs written in C alongside completing the [Linux Systems Programming](https://www.pluralsight.com/courses/linux-systems-programming) course on Pluralsight.  
  
## Programs implemented
* `shell` - a basic shell implementation to run commands in, supporting:
    * piping - one pipe can be used for example `ls | sort`
    * redirecting - use `>` or `>>` to write/append output to a file respectively. Can be used with a single command or after the piped command.
    * background jobs - for a single command, it can be sent to run in the background like `firefox &`
    * In-built commands `cd`, `jobs`, `kill` and `help` 
* `ls [-a] [-r] [-l] directory` - list a directory with a similar output to the traditional ls command. The -a -r -l arguments to show hidden files and recursive listing and long format respectively
* `cp [-m mode] source destination` - specify a mode (`-m [std|sys|map]`) to copy using standard library, system calls and mmap respectively. If not specified mode defaults to `std`.
* `cp source destination` - 3 implementations to copy a file (C standard library (`cp_std`), system calls for IO (`cp_sys`) and mmap (`cp_mmap`))
* `mv source destination` - move a file from a soure to a destination. Moves file into a directory if the destination is a directory.  
    **IMPORTANT** - this only works if moving around on the same filesystem as it involves the use of inodes.
* `ln [-s] target directory` - create a link to a file, use the -s flag for soft links.
* `rm [-r] file [file file ...]` - delete a file from the filesystem. Use -r for recursive to delete a directory. Can supply up to 100 files as arguments to delete
* `touch file [file file ...]` - create an empty file on the file system. Can supply up to 100 files to create
* `mkdir dir [dir dir ...]` - create an empty directory on the file system. Can supply up to 100 directories to create. Intermediate directories will not be created
* `rmdir dir [dir dir ...]` - remove an empty directory on the file system. Can supply up to 100 directories to delete
* `id` - list user ID, group ID and all groups of which user is a member
* `whoami` - print username associated with the effective user ID of the process

# Build
Run the `compile.sh` script to compile all programs into a build folder.

# Run
Each program is built with it's filename and the extension `.o`.  
So run a program with `./build/program.o`.
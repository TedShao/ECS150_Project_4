# Project 4: File System

# Implementation

To implement our file system APIs, we created our own data structures to hold
the metadata when the user mounted the file system. The user could then read and
write to the file system while our APIs would correctly modify the metadata in
memory. When the user unmounts the file system, the metadata is flushed back to
the virtual disk.

We defined 3 structs to hold the data for the superblock, individual files, and
open files. In addition we have 3 arrays to hold the FAT table, the files in the
root directory, and all the open files.

## Superblock

The super block struct is of the size BLOCK_SIZE and contains and all the
parameters specified in the project assignment. We use the packed attribute to
have the stuct have its data members located in specific locations. This allows
easy copying of the disk superblock into our data structure.

    typedef struct __attribute__((__packed__)) superblock {
        uint8_t signature[ECS150FS_SIG_SIZE];
        uint16_t total_blk_count;
        uint16_t root_blk;
        uint16_t data_blk;
        uint16_t data_blk_count;
        uint8_t fat_blk_count;
        uint8_t padding[SUPERBLK_PADDING];
    } *superblock_t;

## Files

The assignement sepcifies the entries to the root directory. Our file struct is
a representation of those entries. 

    typedef struct __attribute__((__packed__)) file {
        uint8_t name[FS_FILENAME_LEN];
        uint32_t size;
        uint16_t start_index;
        uint8_t padding[ROOT_DIR_ENTRY_PADDING];
    } *file_t;

Our root directory is represented by a pointer pointing to the first file in the
root directory. This pointer is allocated size of BLOCK_SIZE to hold the
specified 128 files.

    file_t rdir;

## Open Files

When a file is opened, a new open_file struct is created. This contains a
pointer to a file and an offset used by lseek, read, and write.

    typedef struct open_file {
        file_t file;
        uint32_t offset;
    } open_file_t;

All open files are stored in an array where the index corresponds to the file
descriptor

    open_file_t open_files[FS_OPEN_MAX_COUNT];

## FAT

The fat table is stored in a simple array of size specified in
superblock->fat_block_count * BLOCK_SIZE. Since this array is dependent on the
specific virtual disk we are using, this array is dynamically allocated in
fs_mount.

    uint16_t *fat;

# Phase 1

The first phase is responsible for properly loading and modifying the metadata
in the file system. 

## fs_mount()

In this function the superblock, FAT, root directory blocks from the virtual
disk are loaded into our data structures. In addition the open_files array is
cleared and the formatting of the metadata is checked.

## fs_unmount()

This function first tests to make sure no files are still opened by the user. If
all files are closed, the function writes out all the locally stored metadata to
the virtual disk. Then all the metadata stuctures are freed from memory.

## fs_info()

Reads the required data out of fat, superblock and root directory and prints to
stdout.

# Phase 2

## fs_create()

This function first checks for a valid filename or if the filename already
exists. Then it checks for space in the root directory as well as in the FAT.
Then it initializes the memory to 0 using memset, and copies the filename and
start index into the root directory. At the end it sets the next FAT index to 
FAT_EOC

## fs_delete()

This function checks for a valid filename, if the file is open, and finds it's
index if the file exists. Then it delletes the file by looping through the FAT
indexes and setting them to 0 till it hits FAT_EOC. Afterwards its sets all the
files memory to 0 to signify that they are free.

## fs_ls()

This function prints out the files in the filesystem with their size and data 
block to stdout.

# Phase 3

## fs_open()

This function first chcecks the number of open files, a valid filename, and if
the file exists. Then it finds the first empty file index and uses that index to
stores that file into an open_files array with an offset of 0. Then it
increments the counter for the amount of open files.

## fs_close()

This function first checks if the file descriptor is valid. Then it closes the 
file by removing it from the open files array and decrements the open file
counter. 

## fs_lseek()

This function checks for a valid file descriptor and if the offset is within the
file bounds. Then it sets the file's offset to the given offset argument.

## fs_stat()

The function returns the file size by returning the size of the file inside the
open files array based on the file descriptor.

# Phase 4

## fs_read()

## fs_write()

# Testing

# References
Used the following source for learning about packed structs:
https://www.mikroe.com/blog/packed-structures-make-memory-feel-safe

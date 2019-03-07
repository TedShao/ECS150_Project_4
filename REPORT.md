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



## fs_delete()

## fs_ls()

# Phase 3

## fs_open()

## fs_close()

## fs_lseek()

## fs_stat()

# Phase 4

## fs_read()

## fs_write()

# Testing

# References
Used the following source for learning about packed structs:
https://www.mikroe.com/blog/packed-structures-make-memory-feel-safe

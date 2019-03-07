# Project 4: File System

# Implementation

To implement our file system APIs, we created our own data structures to hold
the metadata when the user mounted the file system. The user could then read and
write to the file system while our APIs would correctly modify the metadata in
memory. When the user unmounts the file system, the metadata is flushed back to
the virtual disk.

# Phase 1

The first phase is responsible for properly loading and modifying the metadata
in the file system. We defined 3 structs to hold the data for the superblock,
individual files, and open files. 

## Superblock

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

    typedef struct __attribute__((__packed__)) file {
        uint8_t name[FS_FILENAME_LEN];
        uint32_t size;
        uint16_t start_index;
        uint8_t padding[ROOT_DIR_ENTRY_PADDING];
    } *file_t;

## Open Files

    typedef struct open_file {
        file_t file;
        uint32_t offset;
    } open_file_t;


In addition we had 3 arrays to hold the 

# Phase 2

# Phase 3

# Phase 4

# Testing

# References
Used the following source for learning about packed structs:
https://www.mikroe.com/blog/packed-structures-make-memory-feel-safe

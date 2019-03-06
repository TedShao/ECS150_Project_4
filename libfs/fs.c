#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

/* Macros */
#define ECS150FS_SIG "ECS150FS"
#define ECS150FS_SIG_SIZE 8

#define SUPERBLK_PADDING 4079
#define ROOT_DIR_ENTRY_PADDING 10

#define FAT_EOC 0xFFFF

/* Structs*/
typedef struct __attribute__((__packed__)) superblock {
	uint8_t signature[ECS150FS_SIG_SIZE];
	uint16_t total_blk_count;
	uint16_t root_blk;
	uint16_t data_blk;
	uint16_t data_blk_count;
	uint8_t fat_blk_count;
	uint8_t padding[SUPERBLK_PADDING];
} *superblock_t;

typedef struct __attribute__((__packed__)) file {
	uint8_t name[FS_FILENAME_LEN];
	uint32_t size;
	uint16_t start_index;
	uint8_t padding[ROOT_DIR_ENTRY_PADDING];
} *file_t;

typedef struct open_file {
	file_t file;
	uint32_t offset;
} open_file_t;

/* Global Variables */
superblock_t superblock;
file_t rdir;
open_file_t open_files[FS_OPEN_MAX_COUNT];
uint16_t *fat;
uint8_t open_file_count;


/* Internal Functions */
/* Check if valid filename (null terminated and <16) */
int valid_filename(const char *filename) 
{
	int valid_name = 0;

	for (int i = 0; i < FS_FILENAME_LEN; i++) {
		if (filename[i] == '\0') {
			valid_name = 1;
			break;
		}
	}

	return valid_name;
}

/* Check if fd is valid */
int valid_fd(int fd)
{
	/* Check if fd is in bounds */
	if (fd < 0 || fd >= FS_FILE_MAX_COUNT) 
		return 0;

	/* Check if fd is valid */
	if (open_files[fd].file == NULL)
		return 0;

	return 1;
}

/* Returns -1 if fat is full, otherwise returns first free fat starting at
start_index + 1 */
int fat_find_free(int start_index) 
{
	for (int i = (start_index + 1); i < superblock->data_blk_count; i++) {
		if (fat[i] == 0) {
			return i;
		}
	}
	
	/* if no empty fat entry found */
	return FAT_EOC;
}

/* Returns the fat index of the open_file with offset*/
int fat_find_index(open_file_t open_file) 
{
	int fat_offset = open_file.offset / BLOCK_SIZE;
	int fat_index = open_file.file->start_index;

	for (int i = 0; i < fat_offset; i++) {
		fat_index = fat[fat_index];
	}

	return fat_index;
}

/* Wrapper reading function to add data block start offset */
int data_block_read(size_t block, void *buf)
{
	return block_read(block + superblock->data_blk, buf);
}

/* Wrapper writing function to add data block start offset */
int data_block_write(size_t block, void *buf)
{
	return block_write(block + superblock->data_blk, buf);
}

/* Returns the index of open_files with file of filename, returns -1 if
not found */
int open_find_file(const char *filename) 
{
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++) {
		if (open_files[i].file != NULL ) {
			if (strcmp((char*)open_files[i].file->name,filename) == 0)
				return i;
		} else if (strcmp(filename,"") == 0) {
			return i;
		}
	}
	
	/* if no filed of filename match is opened */
	return -1;
}

/* Returns the index of the root directory with file of filename, returns -1 if
not found */
int rdir_find_file(const char *filename) {
	int index = -1;
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (strcmp((char*)rdir[i].name,filename) == 0) {
			index = i;
			break;
		}
	}
	return index;
}

/* Resize block allocation to allow file to contain size bytes and returns the
actual resize value if disk is full and not able resize to size*/
int file_resize(open_file_t open_file, int size)
{
	int fat_index = open_file.file->start_index;
	int blk_count = size / BLOCK_SIZE + 1;

	for (int i = 1; i < blk_count; i++) {
		if (fat[fat_index] == FAT_EOC || fat[fat_index] == 0) {
			fat[fat_index] = fat_find_free(fat_index);
			if (fat[fat_index] == FAT_EOC) {
				open_file.file->size = (i+1) * BLOCK_SIZE;
				return ((i+1) * BLOCK_SIZE);
			}
		}
		fat_index = fat[fat_index];
	}

	fat[fat_index] = FAT_EOC;
	open_file.file->size = size;
	return size;
}

/***** API Functions *****/
int fs_mount(const char *diskname)
{
	char sig_check[ECS150FS_SIG_SIZE + 1];

	/* Open Disk */
	if (block_disk_open(diskname) == -1) 
		return -1;

	/* Read superblock*/
	superblock = (superblock_t) malloc(sizeof(uint8_t)*BLOCK_SIZE);
	if (block_read(0, superblock) == -1)
		return -1;

	/* Check signature */
	memcpy(sig_check,superblock->signature, ECS150FS_SIG_SIZE);
	sig_check[ECS150FS_SIG_SIZE] = '\0';
	if (strcmp(ECS150FS_SIG, sig_check) != 0)
		return -1;

	/* Check block count */
	if (block_disk_count() != superblock->total_blk_count)
		return -1;

	/* Read root directory */
	rdir = (file_t) malloc(sizeof(uint8_t)*BLOCK_SIZE);
	if (block_read((superblock->data_blk-1), rdir) == -1)
		return -1;

	/* Read FAT array */
	fat = (uint16_t*) malloc(sizeof(uint16_t)*BLOCK_SIZE*superblock->fat_blk_count);
	for (int i = 0; i < superblock->fat_blk_count; i++) {
		block_read(1 + i, fat + (i * BLOCK_SIZE));
	}

	/* Clear Open File Array */
	memset(open_files,0, sizeof(open_file_t) * FS_OPEN_MAX_COUNT);
	open_file_count = 0;

	return 0;
}

int fs_umount(void)
{
	/* Check for open files */
	if (open_file_count != 0) 
		return -1;

	/* Write out the metadata */
	if (block_write(0, superblock) == -1)
		return -1;
	if (block_write((superblock->data_blk-1), rdir) == -1)
		return -1;
	for (int i = 0; i < superblock->fat_blk_count; i++) {
		block_write(1 + i, fat + (i * BLOCK_SIZE));
	}

	/* Close the Disk */
	if (block_disk_close() == -1)
		return -1;

	/* Free metadata structures */
	free(superblock);
	free(rdir);
	free(fat);
	return 0;
}

int fs_info(void)
{
	int file_count = 0;
	int fat_count = 0;

	if(block_disk_count() == -1)
		return -1;

	/* FS Info: */
	printf("FS Info:\n"); 

	/* total_blk_count= */
	printf("total_blk_count=%d\n", superblock->total_blk_count);

	/* fat_blk_count= */
	printf("fat_blk_count=%d\n", superblock->fat_blk_count);

	/* rdir_blk= */
	printf("rdir_blk=%d\n", (superblock->data_blk - 1));

	/* data_blk= */
	printf("data_blk=%d\n", superblock->data_blk);

	/* data_blk_count= */
	printf("data_blk_count=%d\n", superblock->data_blk_count);

	/* fat_free_ratio=x/4096 */
	for (int i = 0; i < superblock->data_blk_count; i++) {
		if (fat[i] != 0) {
			fat_count++;
		}
	}
	printf("fat_free_ratio=%d/%d\n", superblock->data_blk_count - fat_count, superblock->data_blk_count);

	/* rdir_free_ratio=x/128 */
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (rdir[i].name[0] != '\0') {
			file_count++;
		}
	}
	printf("rdir_free_ratio=%d/%d\n", FS_FILE_MAX_COUNT - file_count, FS_FILE_MAX_COUNT);
	
	return 0;
}

int fs_create(const char *filename)
{
	int empty_index = -1;
	int fat_index = 0;

	/* Valid name check */
	if (!valid_filename(filename))
		return -1;

	/*
	 * - Checks if filename already exists
	 * - Finds a free entry index if it doesn't exists
	 */
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (strcmp((char*)rdir[i].name,filename) == 0)
			return -1;
		if (rdir[i].name[0] == '\0' && empty_index == -1)
			empty_index = i;
	}

	/* Check if maximum files created */
	if (empty_index == -1)
		return -1;

	/* Check for full disk */
	fat_index = fat_find_free(0);
	if (fat_index == FAT_EOC)
		return -1;

	/* Create a new file */
	memset(&(rdir[empty_index]),0,BLOCK_SIZE/FS_FILE_MAX_COUNT);
	strcpy((char*)rdir[empty_index].name,filename);
	rdir[empty_index].start_index = fat_index;
	fat[fat_index] = FAT_EOC;
	
	return 0;
}

int fs_delete(const char *filename)
{
	int del_index = -1;
	uint16_t del_block = 0;

	/* Valid name check */
	if (!valid_filename(filename))
		return -1;

	/* Check if file is open */
	if (open_find_file(filename) != -1)
		return -1;

	/* Check if file exists and find index*/
	del_index = rdir_find_file(filename);
	if (del_index == -1)
		return -1;

	/* Delete the file */
	del_block = rdir[del_index].start_index;
	while (del_block != FAT_EOC) {
		uint16_t temp_block = fat[del_block];
		fat[del_block] = 0;
		del_block = temp_block;
	}
	memset(&(rdir[del_index]),0,BLOCK_SIZE/FS_FILE_MAX_COUNT);

	return 0;
}

int fs_ls(void)
{
	if(block_disk_count() == -1)
		return -1;

	/* FS Ls: */
	printf("FS Ls:\n");

	/* Iterate though rdir and print file w/ info */
	for ( int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (rdir[i].name[0] != 0) {
			/* file: */
			printf("file: %s, ", (char*)rdir[i].name);
			/* size: */
			printf("size: %u, ", rdir[i].size);
			/* data_blk: */
			printf("data_blk: %u\n", rdir[i].start_index);
		}
	}

	return 0;
}

int fs_open(const char *filename)
{
	int open_index = -1;
	int rdir_index = -1;

	/* Check number of open files */
	if (open_file_count == FS_OPEN_MAX_COUNT) 
		return -1;

	/* Valid name check */
	if (!valid_filename(filename))
		return -1;

	/* Check if file exists */
	rdir_index = rdir_find_file(filename);
	if (rdir_index == -1)
		return -1;

	/* Find first empty spot in open_files */
	open_index = open_find_file("");
	open_files[open_index].file = &(rdir[rdir_index]);
	open_files[open_index].offset = 0;
	
	/* Increment open file count */
	open_file_count++;

	return open_index;
}

int fs_close(int fd)
{
	/* Check if fd is valid */
	if (!valid_fd(fd)) 
		return -1;

	/* Close file */
	memset(&open_files[fd],0,sizeof(open_file_t));

	open_file_count--;
	return 0;
}

int fs_stat(int fd)
{
	/* Check if fd is valid */
	if (!valid_fd(fd)) 
		return -1;

	/* Return size of file */
	return open_files[fd].file->size;
}

int fs_lseek(int fd, size_t offset)
{
	/* Check if fd is valid */
	if (!valid_fd(fd)) 
		return -1;

	/* Check if offset is within bounds of file */
	if (offset < 0 || offset >= open_files[fd].file->size)
		return -1;

	/* Set open file offest */
	open_files[fd].offset = offset;

	return 0;
}

int fs_write(int fd, void *buf, size_t count)
{
	char *blk_buf;
	char *buf_copy;
	size_t blk_count;
	size_t blk_index;
	size_t byte_count;
	size_t byte_rem;
	size_t byte_offset;
	open_file_t write_file;

	/* Check if fd is valid */
	if (!valid_fd(fd)) 
		return -1;

	/* Setup blk writing variables */
	buf_copy = (char*) buf;
	write_file = open_files[fd];
	byte_count = count;
	blk_index = fat_find_index(write_file);

	/* Check if file needs to be extended */
	if (byte_count + write_file.offset > write_file.file->size) {
		int actual_resize = file_resize(write_file, byte_count + write_file.offset);
		byte_count = actual_resize - write_file.offset;
	}

	/* Setup variables */
	blk_count = byte_count / BLOCK_SIZE + 1;
	byte_offset = write_file.offset % BLOCK_SIZE;
	blk_buf = (char*) malloc(sizeof(char) * BLOCK_SIZE);

	/* Read first block and modify at offset */
	data_block_read(blk_index, blk_buf);
	memcpy((blk_buf + byte_offset), buf_copy, (BLOCK_SIZE - byte_offset));
	data_block_write(blk_index, blk_buf);

	/* Write to full data blocks from (1 to blk_count - 1) */
	buf_copy += (BLOCK_SIZE - byte_offset);
	blk_index = fat[blk_index];
	for (int i = 1; i < (blk_count-1); i++) {
		data_block_write(blk_index, buf_copy);
		buf_copy += BLOCK_SIZE;
		blk_index = fat[blk_index];
	}

	/* Write to last block */
	if (blk_index != FAT_EOC) {
		byte_rem = (byte_count + write_file.offset) % BLOCK_SIZE;
		data_block_read(blk_index, blk_buf);
		memcpy(blk_buf, buf_copy, byte_rem);
		data_block_write(blk_index, blk_buf);
	}

	/* Modify offset */
	write_file.offset += byte_count;

	return byte_count;
}

int fs_read(int fd, void *buf, size_t count)
{
	char *blk_buf;
	size_t blk_count;
	size_t blk_index;
	size_t byte_count;
	size_t byte_rem;
	size_t byte_offset;
	open_file_t read_file;

	/* Check if fd is valid */
	if (!valid_fd(fd)) 
		return -1;

	/* Setup blk reading variables */
	read_file = open_files[fd];
	byte_rem = read_file.file->size - read_file.offset;	
	byte_count = (byte_rem < count) ? byte_rem : count;			
	blk_count = byte_count / BLOCK_SIZE + 1;
	blk_index = fat_find_index(read_file);

	/* Check if offset has reached end of file, zero bytes are read */
	if (blk_index == 0) 
		return 0;

	/* Allocated block buffer and fill with disk data */
	blk_buf = (char*) malloc(sizeof(char) * BLOCK_SIZE * blk_count);
	for (int i = 0; i < blk_count; i++) {
		data_block_read(blk_index, blk_buf + (BLOCK_SIZE*i));
		blk_index = fat[blk_index];
	}

	/* Copy needed data from blk_buf */
	byte_offset = read_file.offset % BLOCK_SIZE;
	memcpy(buf, (blk_buf + byte_offset), byte_count);

	/* Modify offset */
	read_file.offset += byte_count;

	return byte_count;
}


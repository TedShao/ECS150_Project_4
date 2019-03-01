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

/* Global Variables */
superblock_t superblock;
file_t rdir;
uint16_t *fat;

/* Internal Functions */

/* API Functions */
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

	return 0;
}

int fs_umount(void)
{
	// TODO: might need to modify to write meta data before closing disk

	// TODO: check for open file descriptors

	if (block_disk_close() == -1)
		return -1;

	free(superblock);
	free(rdir);
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
		if (rdir[i].size > 0) {
			file_count++;
		}
	}
	printf("rdir_free_ratio=%d/%d\n", FS_FILE_MAX_COUNT - file_count, FS_FILE_MAX_COUNT);
	
	return 0;
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */

	return 0;
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */

	return 0;
}

int fs_ls(void)
{
	/* TODO: Phase 2 */

	return 0;
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */

	return 0;
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */

	return 0;
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */

	return 0;
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */

	return 0;
}

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */

	return 0;
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */

	return 0;
}


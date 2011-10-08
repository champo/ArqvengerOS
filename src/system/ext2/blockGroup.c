#include "drivers/ata.h"
#include "system/mm.h"
#include "system/ext2/superblock.h"
#include "library/string.h"
#include "system/ext2/blockGroup.h"

char* table;

static int getBytes(int start, int end, void* blockGroupDescriptor, char* buff);

int block_group_descriptor_table_init(void) {
    if ((table = kalloc(512)) == NULL) {
        return -1;
    }
    if (ata_read((unsigned long long)3, 1, table) != 0) {
        return -1;
    }
    return 0;
}

int get_block_group_descriptor(int index, void* blockGroupDescriptor) {
    if (index > superblock_get_total_block_groups()) {
        return -1;
    }
    memcpy(blockGroupDescriptor, table + index * 32, 32);
    return 0;
}

int block_group_get_adress_block_usage(void* blockGroupDescriptor) {
    char buff[4];
    return getBytes(0, 3, blockGroupDescriptor, buff);
}

int block_group_get_adress_inode_usage(void* blockGroupDescriptor) {
    char buff[4];
    return getBytes(4, 7, blockGroupDescriptor, buff);
}

int block_group_get_start_block_adress_inode_table(void* blockGroupDescriptor) {
    char buff[4];
    return getBytes(8, 11, blockGroupDescriptor, buff);
}

int block_group_get_total_unallocated_blocks(void* blockGroupDescriptor) {
    char buff[2];
    return getBytes(12, 13, blockGroupDescriptor, buff);
}

int block_group_get_total_unallocated_inodes(void* blockGroupDescriptor) {
    char buff[2];
    return getBytes(14, 15, blockGroupDescriptor, buff);
}

int block_group_get_total_directories(void* blockGroupDescriptor) {
    char buff[2];
    return getBytes(16, 17, blockGroupDescriptor, buff);
}

int getBytes(int start, int end, void* blockGroupDescriptor, char* buff) {

    int i;
    
    for(i = 0; i < (end - start + 1); i++) {
        buff[i] = ((char *)blockGroupDescriptor)[start + i];
    }
    return *((int*) buff);
}

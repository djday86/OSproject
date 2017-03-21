#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "datatypes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

u32 i;

struct ext2_super_block {
    u32   s_inodes_count;         /* Inodes count */
    u32   s_blocks_count;         /* Blocks count */
    u32   s_r_blocks_count;       /* Reserved blocks count */
    u32   s_free_blocks_count;    /* Free blocks count */
    u32   s_free_inodes_count;    /* Free inodes count */
    u32   s_first_data_block;     /* First Data Block */
    u32   s_log_block_size;       /* Block size */
    s32   s_log_frag_size;        /* Fragment size */
    u32   s_blocks_per_group;     /* # Blocks per group */
    u32   s_frags_per_group;      /* # Fragments per group */
    u32   s_inodes_per_group;     /* # Inodes per group */
    u32   s_mtime;                /* Mount time */
    u32   s_wtime;                /* Write time */
    u16   s_mnt_count;            /* Mount count */
    s16   s_max_mnt_count;        /* Maximal mount count */
    u16   s_magic;                /* Magic signature */
    u16   s_state;                /* File system state */
    u16   s_errors;               /* Behaviour when detecting errors */
    u16   s_minor_rev_level;      /* minor revision level */
    u32   s_lastcheck;            /* time of last check */
    u32   s_checkinterval;        /* max. time between checks */
    u32   s_creator_os;           /* OS */
    u32   s_rev_level;            /* Revision level */
    u16   s_def_resuid;           /* Default uid for reserved blocks */
    u16   s_def_resgid;           /* Default gid for reserved blocks */
}super_block;

typedef struct {
    u32 bg_block_bitmap;
    u32 bg_inode_bitmap;
    u32 bg_inode_table;
    u16 bg_free_blocks;
    u16 bg_free_inodes;
    u16 bg_used_dirs_count;
    u16 bg_pad;
    u8 bg_reserved[4];
    
} bg_descriptor_table;
typedef struct { 
	
	u8 header[0x4b];
	u32 drive_type;
	u8 image_desc[0x103];
	u32 offset_blocks;
	u32 offset_data;
	u32 cylinders;
	u32 heads;
	u32 sectors;
	u32 sector_size;
	u32 unused;
	u64 disk_size;
	u32 block_size;
	u32 block_edata;
	u32 total_blocks;
	u32 blocks_allocated;
	u8 unused_info[0x78];
	u32 cursor;
	u32* map;

} VDI_header;

struct __attribute__((packed)) BootSector {
  u8
    unused0[0x1be];
  PartitionEntry
    partitionTable[4];
  u16
    magic;
};

struct PartitionEntry {
  u8
    unused0[4],
    type,
    unused1[3];
  u32
    firstSector,
    nSectors;
};

u32 read_VDI_map(u32 fd, VDI_header disk_info);
void get_partition_details(int *part_num partition_entry *data)
	

int main(int argc, char *argv[]) {

	u32 fd =0;
	VDI_header disk_info;
	disk_info.cursor = 0;

	printf("Name of file: %s\n", argv[1]);

	//lseek(fd,OFFSET_BLOCKS, SEEK_SET)
	//lseek(fd,LOCATION, SEEK_SET)
	//read(fd, buffer, size) make sure buffer reading into has enough bytes to store size.

	fd = open(argv[1], O_RDONLY);

	if(lseek(fd,0,SEEK_SET) == -1) return EXIT_FAILURE;
	if(read(fd,&disk_info,sizeof(disk_info)) == -1) return EXIT_FAILURE;
	if(read_VDI_map(fd,disk_info) == -1) return EXIT_FAILURE;

	if(disk_info.drive_type == 1) printf("File type: Dynamic\n");
	else printf("File type: Static\n");

	printf("Offset blocks: %i bytes.\n", disk_info.offset_blocks);
	printf("Offset data: %i bytes.\n", disk_info.offset_data);
	printf("Cylinders: %i.\n", disk_info.cylinders);
	printf("Heads: %i.\n", disk_info.heads);
	printf("Sectors: %i.\n", disk_info.sectors);
	printf("Sector Size: %i.\n", disk_info.sector_size);
	printf("Total disk size: %llu. bytes\n", disk_info.disk_size);	
	printf("Disk block size: %i bytes.\n", disk_info.block_size);
	printf("Total # of blocks: %i.\n",disk_info.total_blocks);
	printf("Total # of blocks allocated: %i.\n",disk_info.blocks_allocated);

	printf("Block Map: \n");
	for(i=0;i<disk_info.blocks_allocated;i++) printf("Block #%i: %i \n",i,disk_info.map[i]);

	free(disk_info.map);
	if(close(fd) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

u32 read_VDI_map(u32 fd, VDI_header disk_info) {

	disk_info.map = (u32 *)malloc(4*(disk_info.blocks_allocated));

	if(lseek(fd,disk_info.offset_blocks,SEEK_SET) == -1) return -1;
	if(read(fd, disk_info.map, disk_info.blocks_allocated) == -1) return -1;

}

void get_partition_details(int *part_num partition_entry *data){
    
    int part_num = 0;
 
    lseek(fd, diskinfo.offsetblocks, SEEK_SET);
    read(fd, &data, 512);
    if(diskinfo.partition_table[0].type == 0x38)
        return;
    else if(diskinfo.partition_table[1].type == 0x38){
         part_num = 1;
         return;
        }
    else if (diskinfo.partition_table[2].type == 0x38){
        part_num = 2;
        return;
    }
    else if(diskinfo.partition_table[3].type == 0x38) {
        part_num = 3;
        return;
    }
                
}

void get_super_block(super_block *main_super_block){
    lseek(fd, disk_info offsetdata + 1024, SEEK_SET);
    read(fd, &main_super_block, 1024)
}

void get_bg_descriptor_table(bg_descriptor_table *bg_data) {
    
    lseek(fd, disk_info.offsetdata +)
}

void fetch_block( void *buf, int block_num){
    
}
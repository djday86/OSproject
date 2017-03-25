#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "datatypes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


u32 i;

typedef struct {
	int	s_inodes_count;		/* Inodes count */
	int	s_blocks_count;		/* Blocks count */
	int	s_r_blocks_count;	/* Reserved blocks count */
	int	s_free_blocks_count;	/* Free blocks count */
	int	s_free_inodes_count;	/* Free inodes count */
	int	s_first_data_block;	/* First Data Block */
	int	s_log_block_size;	/* Block size */
	int	s_log_frag_size;	/* Fragment size */
	int	s_blocks_per_group;	/* # Blocks per group */
	int	s_frags_per_group;	/* # Fragments per group */
	int	s_inodes_per_group;	/* # Inodes per group */
	int	s_mtime;		/* Mount time */
	int	s_wtime;		/* Write time */
	short int	s_mnt_count;		/* Mount count */
	short int	s_max_mnt_count;	/* Maximal mount count */
	short int	s_magic;		/* Magic signature */
	short int	s_state;		/* File system state */
	short int	s_errors;		/* Behaviour when detecting errors */
	short int	s_minor_rev_level; 	/* minor revision level */
	int	s_lastcheck;		/* time of last check */
	int	s_checkinterval;	/* max. time between checks */
	int	s_creator_os;		/* OS */
	int	s_rev_level;		/* Revision level */
	short int	s_def_resuid;		/* Default uid for reserved blocks */
	short int	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	int	s_first_ino; 		/* First non-reserved inode */
	short int   s_inode_size; 		/* size of inode structure */
	short int	s_block_group_nr; 	/* block group # of this superblock */
	int	s_feature_compat; 	/* compatible feature set */
	int	s_feature_incompat; 	/* incompatible feature set */
	int	s_feature_ro_compat; 	/* readonly-compatible feature set */
	unsigned char	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	int	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	unsigned char	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	unsigned char	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	unsigned short int	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	unsigned char	s_journal_uuid[16];	/* uuid of journal superblock */
	unsigned int	s_journal_inum;		/* inode number of journal file */
	unsigned int	s_journal_dev;		/* device number of journal file */
	unsigned int	s_last_orphan;		/* start of list of inodes to delete */
	unsigned int	s_hash_seed[4];		/* HTREE hash seed */
	unsigned char	s_def_hash_version;	/* Default hash version to use */
	unsigned char	s_reserved_char_pad;
	unsigned short int	s_reserved_word_pad;
	int	s_default_mount_opts;
 	int	s_first_meta_bg; 	/* First metablock block group */
	unsigned int	s_reserved[190];	/* Padding to the end of the block */
} ext2_super_block;

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

} VDI_header;

typedef struct {

  VDI_header hdr;
  u32 cursor,fd;
  u32 *map;

} VDI_file;

typedef struct {

	u8 
	unused0[4], 
	type, 
	unused1[3];

	u32
	firstSector, nSectors;

} PartitionEntry;

typedef struct __attribute__((packed)) BootSector {

	u8 unused0[0x1be];
	PartitionEntry partitionTable[4];
	u16 magic;

} BootSector;


u32 read_VDI_map(u32 fd, VDI_file *disk_info, u32 offset_blocks, u32 blocks_allocated);
u32 get_partition_details(u32 fd, VDI_file disk_info, BootSector boot_sector);
u32 get_super_block(u32 fd, ext2_super_block *main_super_block, VDI_header disk_info);
u32 VDI_translate(u32 desired_byte, VDI_file disk_info);
u32 read_into_buffer(u32 fd, void *buff, u32 position, u32 num_bytes);
	

int main(int argc, char *argv[]) {
	
	u32 start;
	u32 fd =0;
	VDI_header disk_info;
	ext2_super_block main_super_block;
	VDI_file vdi;
	BootSector boot_sector;

	printf("Name of file: %s\n", argv[1]);
	vdi.fd = open(argv[1], O_RDONLY);

	if(read_into_buffer(vdi.fd,&vdi.hdr,0,sizeof(vdi.hdr)) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	if(read_VDI_map(vdi.fd,&vdi,vdi.hdr.offset_blocks, vdi.hdr.blocks_allocated) == -1) { 
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	if(read_into_buffer(vdi.fd,&boot_sector,VDI_translate(0, vdi),512) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	start = get_partition_details(vdi.fd,vdi,boot_sector);

	if(start == -1) { 
		printf("Error.\n");
		return EXIT_FAILURE;
	}
	
	if(lseek(vdi.fd, VDI_translate(1024+start, vdi), SEEK_SET) == -1) return EXIT_FAILURE;
    	if(read(vdi.fd, &main_super_block, 1024) == -1) return EXIT_FAILURE;
	
	if(vdi.hdr.drive_type == 1) printf("File type: Dynamic\n");
	else printf("File type: Static\n");
	printf("Offset blocks: %i bytes.\n", vdi.hdr.offset_blocks);
	printf("Offset data: %i bytes.\n", vdi.hdr.offset_data);
	printf("Cylinders: %i.\n", vdi.hdr.cylinders);
	printf("Heads: %i.\n", vdi.hdr.heads);
	printf("Sectors: %i.\n", vdi.hdr.sectors);
	printf("Sector Size: %i.\n", vdi.hdr.sector_size);
	printf("Total disk size: %llu bytes.\n", vdi.hdr.disk_size);	
	printf("Disk block size: %i bytes.\n", vdi.hdr.block_size);
	printf("Total # of blocks: %i.\n",vdi.hdr.total_blocks);
	printf("Total # of blocks allocated: %i.\n",vdi.hdr.blocks_allocated);

	printf("INFO FROM SUPERBLOCK\n");
	printf("%i\n",main_super_block.s_mtime);
	printf("\n%04x\n",main_super_block.s_magic);

	free(vdi.map);

	if(close(fd) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

u32 read_VDI_map(u32 fd, VDI_file *disk_info, u32 offset_blocks, u32 blocks_allocated) {

	disk_info->map = (u32 *)malloc(4*(blocks_allocated));

	if(disk_info->map == NULL) {

		printf("MEMORY FAILURE: MEMORY NOT ALLOCATED");
		return -1;
	}

	if(lseek(fd,offset_blocks,SEEK_SET) == -1) return -1;
	if(read(fd, disk_info->map, 4*blocks_allocated) == -1) return -1;
}

u32 VDI_translate(u32 desired_byte, VDI_file disk_info) {

	u32 page;
	u32 offset;
	u32 loc;
	u32 frame;

	page = (desired_byte / disk_info.hdr.offset_blocks);
	offset = (desired_byte % disk_info.hdr.offset_blocks);
	frame = disk_info.map[page];
	loc = disk_info.hdr.offset_data + (frame * disk_info.hdr.offset_blocks) + offset;

	return loc; 
	
}

u32 get_partition_details(u32 fd, VDI_file disk_info, BootSector boot_sector){
    
	for (i=0;i<4;i++) {
		if(boot_sector.partitionTable[i].type == 0x83)
			break;
	}	
	return boot_sector.partitionTable[i].firstSector * 512;
                
}

/*u32 get_super_block(u32 fd, super_block *main_super_block, VDI_header disk_info) {

    if(lseek(fd, ((disk_info.map[0] + 1) * disk_info.offset_blocks) + 1024, SEEK_SET) == -1) return -1;
    if(read(fd, &main_super_block, 84) == -1) return -1;

}*/

u32 get_bg_descriptor_table(u32 fd, int bg_number, ext2_super_block main_super_block, bg_descriptor_table *bg_data, VDI_header disk_info) {
    
    if(lseek(fd, disk_info.offset_data + 1024 + bg_number * main_super_block.s_blocks_per_group * main_super_block.s_log_block_size + 			main_super_block.s_log_block_size , SEEK_SET) == -1) return -1;
    if(read(fd, &bg_data, sizeof(bg_descriptor_table)) == -1) return -1;

}

u32 fetch_block( u32 fd, void *buf, int block_group, int block_num, VDI_header disk_info, ext2_super_block main_super_block) {

    if(lseek(fd, disk_info.offset_data + 1024 + main_super_block.s_blocks_per_group * block_group * main_super_block.s_log_block_size + 			main_super_block.s_log_block_size * (block_num - 1),SEEK_SET) == -1) return -1;

    if(read(fd, &buf, main_super_block.s_log_block_size) == -1) return -1;
    
}

u32 read_into_buffer(u32 fd, void *buff, u32 position, u32 num_bytes) {
	
	if(lseek(fd, position, SEEK_SET) == -1) return -1;
	if(read(fd, buff, num_bytes) == -1) return -1;

}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "datatypes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>


u32 i;

typedef struct {

	s32	s_inodes_count;		/* Inodes count */
	s32	s_blocks_count;		/* Blocks count */
	s32	s_r_blocks_count;	/* Reserved blocks count */
	s32	s_free_blocks_count;	/* Free blocks count */
	s32	s_free_inodes_count;	/* Free inodes count */
	s32	s_first_data_block;	/* First Data Block */
	s32	s_log_block_size;	/* Block size */
	s32	s_log_frag_size;	/* Fragment size */
	s32	s_blocks_per_group;	/* # Blocks per group */
	s32	s_frags_per_group;	/* # Fragments per group */
	s32	s_inodes_per_group;	/* # Inodes per group */
	s32	s_mtime;		/* Mount time */
	s32	s_wtime;		/* Write time */
	u16	s_mnt_count;		/* Mount count */
	u16	s_max_mnt_count;	/* Maximal mount count */
	u16	s_magic;		/* Magic signature */
	u16	s_state;		/* File system state */
	u16	s_errors;		/* Behaviour when detecting errors */
	u16	s_minor_rev_level; 	/* minor revision level */
	s32	s_lastcheck;		/* time of last check */
	s32	s_checks32erval;	/* max. time between checks */
	s32	s_creator_os;		/* OS */
	s32	s_rev_level;		/* Revision level */
	u16	s_def_resuid;		/* Default uid for reserved blocks */
	u16	s_def_resgid;		/* Default gid for reserved blocks */
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
	s32	s_first_ino; 		/* First non-reserved inode */
	u16   s_inode_size; 		/* size of inode structure */
	u16	s_block_group_nr; 	/* block group # of this superblock */
	s32	s_feature_compat; 	/* compatible feature set */
	s32	s_feature_incompat; 	/* incompatible feature set */
	s32	s_feature_ro_compat; 	/* readonly-compatible feature set */
	u8	s_uuid[16];		/* 128-bit uuid for volume */
	s8	s_volume_name[16]; 	/* volume name */
	s8	s_last_mounted[64]; 	/* directory where last mounted */
	s32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hs32s.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	u32	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	u8	s_journal_uuid[16];	/* uuid of journal superblock */
	u32	s_journal_inum;		/* inode number of journal file */
	u32	s_journal_dev;		/* device number of journal file */
	u32	s_last_orphan;		/* start of list of inodes to delete */
	u32	s_hash_seed[4];		/* HTREE hash seed */
	u8	s_def_hash_version;	/* Default hash version to use */
	u8	s_reserved_char_pad;
	u32	s_reserved_word_pad;
	s32	s_default_mount_opts;
	s32	s_first_meta_bg; 	/* First metablock block group */
	u32	s_reserved[190];	/* Padding to the end of the block */

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
    
} bg_descriptor;

typedef struct __attribute__((packed)) bg_desc_table {

	bg_descriptor *bg_descriptor;

} bg_desc_table;


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

typedef struct {

	u8 *buff;

} arb_block;


u32 read_VDI_map(u32 fd, VDI_file *disk_info, u32 offset_blocks, u32 blocks_allocated);
u32 get_partition_details(u32 fd, VDI_file disk_info, BootSector boot_sector);
u32 get_super_block(u32 fd, ext2_super_block *main_super_block, VDI_header disk_info);
u32 VDI_translate(u32 desired_byte, VDI_file disk_info);
u32 read_into_buffer(u32 fd, void *buff, u32 position, u32 num_bytes);
u32 sb_copy_block(u32 block_num, u32 no_block_grps);
u32 fetch_block( u32 fd, arb_block *buf, s32 block_num, u32 block_size, VDI_file disk_info, u32 start);
u32 block_buf_allocate(u32 block_size, arb_block *block_buf );
	

s32 main(s32 argc, char *argv[]) {
	
	u32 start, no_block_grps;
	u32 fd =0;
	VDI_header disk_info;
	ext2_super_block main_super_block;
	ext2_super_block backup_super_block;
	VDI_file vdi;
	BootSector boot_sector;
	arb_block temp_block;

	printf("\nFile System Check:");
	printf("\n\nName of file: %s\n", argv[1]);
	vdi.fd = open(argv[1], O_RDONLY);

	if(read_into_buffer(vdi.fd,&vdi.hdr,0,sizeof(vdi.hdr)) == -1) {
		printf("Error.  VDI has not been read.\n");
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

	if(read_into_buffer(vdi.fd,&main_super_block,VDI_translate(1024+start,vdi),1024) == -1) {
		printf("Error.");
		return EXIT_FAILURE;
	}
	
	if(vdi.hdr.drive_type == 1) printf("File type: Dynamic\n");
	else printf("File type: Static\n");
	printf("Offset blocks: %i bytes.\n", vdi.hdr.offset_blocks);
	printf("Offset data: %i bytes.\n", vdi.hdr.offset_data);
	printf("Cylinders: %i.\n", vdi.hdr.cylinders);
	printf("Heads: %i.\n", vdi.hdr.heads);
	printf("Sectors: %i.\n", vdi.hdr.sectors);
	printf("Sector Size: %i.\n", vdi.hdr.sector_size);
	printf("Total disk size: %llu bytes.\n", vdi.hdr.disk_size);	
	printf("Disk page size: %i bytes.\n", vdi.hdr.block_size);
	printf("Total # of pages: %i.\n",vdi.hdr.total_blocks);
	printf("Total # of pages allocated: %i.\n\n",vdi.hdr.blocks_allocated);

	printf("Superblock:\n");
	printf("Inodes: %u\n",main_super_block.s_inodes_count);
	printf("Blocks: %u\n",main_super_block.s_blocks_count);
	printf("Reserved Blocks: %u\n",main_super_block.s_r_blocks_count);
	printf("Free Blocks: %u\n",main_super_block.s_free_blocks_count);
	printf("Free iNodes: %u\n",main_super_block.s_free_inodes_count);
	printf("First Data Block Location: %u\n",main_super_block.s_first_data_block);
	main_super_block.s_log_block_size = 1024 << main_super_block.s_log_block_size;	
	printf("Block Size: %u bytes\n", main_super_block.s_log_block_size);
	printf("Fragment Size: %u\n",main_super_block.s_log_frag_size);
	printf("Blocks per Group: %u\n",main_super_block.s_blocks_per_group);
	printf("Fragments per Group: %u\n",main_super_block.s_frags_per_group);
	printf("iNodes per Group: %u\n",main_super_block.s_inodes_per_group);
	printf("Fragment Size: %u\n",main_super_block.s_log_frag_size);	
	printf("The mysterious magical number: %x\n",main_super_block.s_magic);

	if(main_super_block.s_blocks_count % main_super_block.s_blocks_per_group == 0) no_block_grps =  main_super_block.s_blocks_count / 					main_super_block.s_blocks_per_group;
	else no_block_grps = (main_super_block.s_blocks_count / main_super_block.s_blocks_per_group) + 1;

	printf("Total number of block groups: %u\n",no_block_grps);
	
	if(block_buf_allocate(main_super_block.s_log_block_size, &temp_block) == -1) {
		return EXIT_FAILURE;
	}
	printf("OK\n");

	if(fetch_block(vdi.fd,&temp_block,0,main_super_block.s_log_block_size,vdi,start) == -1) {
		printf("Error.");
		return EXIT_FAILURE;
	}
	printf("OK\n");

	

	
	
	free(vdi.map);
	free(temp_block.buff);

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

u32 get_bg_descriptor_table(u32 fd, s32 bg_number, ext2_super_block main_super_block, bg_desc_table *bg_data, VDI_header disk_info) {
    
	if(lseek(fd, disk_info.offset_data + 1024 + bg_number * main_super_block.s_blocks_per_group * main_super_block.s_log_block_size + main_super_block.s_log_block_size , SEEK_SET) == -1) return -1;
	if(read(fd, &bg_data, sizeof(bg_desc_table)) == -1) return -1;

}

u32 fetch_block( u32 fd,arb_block *buf, s32 block_num, u32 block_size, VDI_file disk_info,u32 start) {

	u32 loc = VDI_translate(start + 1024 + (block_num * block_size), disk_info);

	//if(read_into_buffer(fd, buf, loc, block_size) == -1) return -1;

	if(lseek(fd, loc, SEEK_SET) == -1) return -1;
	if(read(fd, buf->buff, block_size) == -1) return -1;
    
}

u32 read_into_buffer(u32 fd, void *buff, u32 position, u32 num_bytes) {
	
	if(lseek(fd, position, SEEK_SET) == -1) {
		printf("LSEEK FAILURE\n");
		return -1;
	}
	if(read(fd, buff, num_bytes) == -1) {
		printf("READ FAILURE\n");
		return -1;
	}
}

u32 block_buf_allocate(u32 block_size, arb_block *block_buf ) {
	
	block_buf->buff = (u8 *)malloc(block_size);

	if(block_buf->buff == NULL) {
		printf("Memory Not Allocated!\n");
		return -1;

	}
}

	

	
        
/*    void read_inode_info(u32 inode_num, struct ext2_inode *i_info) {

    int block_group = (inode_num - 1) / main_super_block.s_inodes_per_group;
    struct bg_descriptor gd_info;
    get_bg_descriptor_table(fd, block_group, main_super_block, *bg_info, disk_info);
    u32 inode_table_start = bg_info.bg_inode_table;
    int inode_index = (inode_num - 1) % main_super_block.s_inodes_per_group;
    int block_num = (inode_table_start + inode_index)/main_super_block.s_inodes_per_group
    
    fetch_block(fd, block_num)
//    
//    u32 inode_sector_offset = inode_index/(sector_size_per_bytes/main_super_block.s_inode_size);
//    
//    
//    lseek(start + (inode_table_start * sectors_per_block) + inode_sector_offset, SEEK_SET);
//     read(fd, &i_info, inode_index);
//    u32 num_inodes_sector = sector_size_bytes / super.s_inode_size;
//    u32 offset = ((inode_index % num_inodes_sector)) * main_super_block.s_inode_size; 
//    i_info->i_mode = read_bytes(buf, offset + 0, 2);
//    i_info->i_size = read_bytes(buf, offset + 4, 4);
//    i_info->i_links_count = read_bytes(buf, offset + 26, 2);
//    for(int i = 0; i < 15; i++){
//        i_info->i_block[i] = read_bytes(buf, offset + 40 + (i * 4), 4);
    }    
}*/

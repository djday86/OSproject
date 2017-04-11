#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "datatypes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#define	EXT2_NDIR_BLOCKS		12
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)
//-- file format --
#define EXT2_S_IFSOCK	0xC000	//socket
#define EXT2_S_IFLNK	0xA000	//symbolic link
#define EXT2_S_IFREG	0x8000	//regular file
#define EXT2_S_IFBLK	0x6000	//block device
#define EXT2_S_IFDIR	0x4000	//directory
#define EXT2_S_IFCHR	0x2000	//character device
#define EXT2_S_IFIFO	0x1000	//fifo
//-- process execution user/group override --
#define EXT2_S_ISUID	0x0800	//Set process User ID
#define EXT2_S_ISGID	0x0400	//Set process Group ID
#define EXT2_S_ISVTX	0x0200	//sticky bit
//-- access rights --
#define EXT2_S_IRUSR	0x0100	//user read
#define EXT2_S_IWUSR	0x0080	//user write
#define EXT2_S_IXUSR	0x0040	//user execute
#define EXT2_S_IRGRP	0x0020	//group read
#define EXT2_S_IWGRP	0x0010	//group write
#define EXT2_S_IXGRP	0x0008	//group execute
#define EXT2_S_IROTH	0x0004	//others read
#define EXT2_S_IWOTH	0x0002	//others write
#define EXT2_S_IXOTH	0x0001	//others execute



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
	u32 bg_reserved[3];

} bg_descriptor;

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
	s16	i_mode;		/* File mode */
	s16	i_uid;		/* Low 16 bits of Owner Uid */
	s32	i_size;		/* Size in bytes */
	s32	i_atime;	/* Access time */
	s32	i_ctime;	/* Creation time */
	s32	i_mtime;	/* Modification time */
	s32	i_dtime;	/* Deletion Time */
	s16	i_gid;		/* Low 16 bits of Group Id */
	s16	i_links_count;	/* Links count */
	s32	i_blocks;	/* Blocks count */
	s32	i_flags;	/* File flags */
	union {
		struct {
			s32  l_i_reserved1;
		} linux1;
		struct {
			s32  h_i_translator;
		} hurd1;
		struct {
			s32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	s32	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	s32	i_generation;	/* File version (for NFS) */
	s32	i_file_acl;	/* File ACL */
	s32	i_dir_acl;	/* Directory ACL */
	s32	i_faddr;	/* Fragment address */
	union {
		struct {
			u8	l_i_frag;	/* Fragment number */
			u8	l_i_fsize;	/* Fragment size */
			u16	i_pad1;
			s16	l_i_uid_high;	/* these 2 fields    */
			s16	l_i_gid_high;	/* were reserved2[0] */
			u32	l_i_reserved2;
		} linux2;
		struct {
			u8	h_i_frag;	/* Fragment number */
			u8	h_i_fsize;	/* Fragment size */
			s16	h_i_mode_high;
			s16	h_i_uid_high;
			s16	h_i_gid_high;
			s32	h_i_author;
		} hurd2;
		struct {
			u8	m_i_frag;	/* Fragment number */
			u8	m_i_fsize;	/* Fragment size */
			u16	m_pad1;
			u32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
} ext2_inode;



typedef struct {

  ext2_inode *inode_entry;

} inode_table;

typedef struct {

	VDI_header hdr;
	u32 cursor,fd,start,block_size,no_groups,blocks_pg;
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

typedef struct{
        s16	i_mode;		/* File mode */
	s16	i_uid;		/* Low 16 bits of Owner Uid */
	s32	i_size;		/* Size in bytes */
	s32	i_atime;	/* Access time */
	s32	i_ctime;	/* Creation time */
	s32	i_mtime;	/* Modification time */
	s32	i_dtime;	/* Deletion Time */
	s16	i_gid;		/* Low 16 bits of Group Id */
	s16	i_links_count;	/* Links count */
	s32	i_blocks;	/* Blocks count */
	s32	i_flags;	/* File flags */
	union {
		struct {
			s32  l_i_reserved1;
		} linux1;
		struct {
			s32  h_i_translator;
		} hurd1;
		struct {
			s32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	s32	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	s32	i_generation;	/* File version (for NFS) */
	s32	i_file_acl;	/* File ACL */
	s32	i_dir_acl;	/* Directory ACL */
	s32	i_faddr;	/* Fragment address */
	union {
		struct {
			u8	l_i_frag;	/* Fragment number */
			u8	l_i_fsize;	/* Fragment size */
			u16	i_pad1;
			s16	l_i_uid_high;	/* these 2 fields    */
			s16	l_i_gid_high;	/* were reserved2[0] */
			u32	l_i_reserved2;
		} linux2;
		struct {
			u8	h_i_frag;	/* Fragment number */
			u8	h_i_fsize;	/* Fragment size */
			s16	h_i_mode_high;
			s16	h_i_uid_high;
			s16	h_i_gid_high;
			s32	h_i_author;
		} hurd2;
		struct {
			u8	m_i_frag;	/* Fragment number */
			u8	m_i_fsize;	/* Fragment size */
			u16	m_pad1;
			u32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */

    
} inode_info;


u32 read_VDI_map();
u32 get_partition_details(BootSector boot_sector);
u32 get_super_block(u32 fd, ext2_super_block *main_sb, VDI_header disk_info);
u32 VDI_translate(u32 desired_byte);
u32 read_into_buffer(void *buff, u32 position, u32 num_bytes);
u32 sb_copy_block(u32 block_num, u32 no_block_grps);
s32 fetch_block( s32 num, void *buff);
u32 block_buf_allocate(u32 block_size, arb_block *block_buf );
s32 get_bg_descriptor_table(bg_descriptor *bg_data);
u32 compare_sb(ext2_super_block a, ext2_super_block b);
u32 get_inode_table(u32 fd, u32 start, bg_descriptor *table, ext2_super_block main_sb, inode_table *i_table, VDI_file file );
u32 get_block_bitmap(u32 fd, u32 start, arb_block *block_bitmap , bg_descriptor table, ext2_super_block main_sb, VDI_file file );
u32 compare_bg_desc_table(bg_descriptor *a, bg_descriptor *b, u32 no_block_grps);
u32 get_inode_table(u32 fd, u32 start, bg_descriptor *table, ext2_super_block main_sb, inode_table *i_table, VDI_file file );
u32 get_block_bitmap(u32 fd, u32 start, arb_block *block_bitmap , bg_descriptor table, ext2_super_block main_sb, VDI_file file );
u32 get_inode_bitmap(u32 fd, u32 start, arb_block *inode_bitmap, bg_descriptor *table, ext2_super_block main_sb, VDI_file file);
s32 vdi_seek(u32 position);
s32 vdi_read(void *buff);

VDI_file vdi;
u8 *temp_block;

s32 main(s32 argc, char *argv[]) {

	/*
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	===========================================================
	MAIN
	===========================================================
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	*/
	u32 no_block_grps;
	u32 fd =0;
	ext2_super_block main_sb;
	ext2_super_block backup_sb;
  inode_table i_table;
	BootSector boot_sector;
  bg_descriptor *desc_table;


	printf("\nFile System Check:");
	printf("\n\nName of file: %s\n", argv[1]);
	vdi.fd = open(argv[1], O_RDONLY);
	if(vdi.fd == -1) {
		printf("Error:  FILE DOES NOT EXIST");
		return EXIT_FAILURE;
	}

	if(read_into_buffer(&vdi.hdr,0,sizeof(vdi.hdr)) == -1) {
		printf("Error.  VDI has not been read.\n");
		return EXIT_FAILURE;
	}

	if(read_VDI_map() == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	if(read_into_buffer(&boot_sector,VDI_translate(0),512) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	vdi.start = get_partition_details(boot_sector);

	if(vdi.start == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	if(read_into_buffer(&main_sb,VDI_translate(vdi.start+1024),1024) == -1) {
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
	printf("Inodes: %u\n",main_sb.s_inodes_count);
	printf("Blocks: %u\n",main_sb.s_blocks_count);
	printf("Reserved Blocks: %u\n",main_sb.s_r_blocks_count);
	printf("Free Blocks: %u\n",main_sb.s_free_blocks_count);
	printf("Free iNodes: %u\n",main_sb.s_free_inodes_count);
	printf("First Data Block Location: %u\n",main_sb.s_first_data_block);
 	vdi.block_size = 1024 << main_sb.s_log_block_size;
	printf("Block Size: %u bytes\n", vdi.block_size);
	printf("Fragment Size: %u\n",main_sb.s_log_frag_size);
	printf("Blocks per Group: %u\n",main_sb.s_blocks_per_group);
	vdi.blocks_pg = main_sb.s_blocks_per_group;
	printf("Fragments per Group: %u\n",main_sb.s_frags_per_group);
	printf("iNodes per Group: %u\n",main_sb.s_inodes_per_group);
	printf("Fragment Size: %u\n",main_sb.s_log_frag_size);
	printf("The mysterious magical number: %x\n",main_sb.s_magic);

	if(main_sb.s_blocks_count % main_sb.s_blocks_per_group == 0) vdi.no_groups =  main_sb.s_blocks_count / 	main_sb.s_blocks_per_group;
	else vdi.no_groups = (main_sb.s_blocks_count / main_sb.s_blocks_per_group) + 1;

 	temp_block = (u8*)malloc(vdi.block_size);


	printf("Total number of block groups: %u\n",vdi.no_groups);


  get_bg_descriptor_table(desc_table);
	printf("SIZE: %i\n", sizeof(desc_table));
	for(i=0;i<1;i++) {
		printf("INFO: %i\n", desc_table[i].bg_block_bitmap);
	}
        
        bg_descriptor *bg_desc_table = (bg_descriptor*)malloc(main_sb.s_log_block_size);
        u8 *inode_bitmap = (u8*)malloc(main_sb.s_log_block_size);
        u8 *block_bitmap = (u8*)malloc(main_sb.s_log_block_size);

	free(vdi.map);
	free(desc_table);


	if(close(fd) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
/*

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
=======================================
FUNCTIONS
=======================================
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

*/

u32 read_VDI_map() {

	vdi.map = (u32 *)malloc(4*(vdi.hdr.blocks_allocated));

	if(vdi.map == NULL) {
		printf("VDI Map Read: MEMORY FAILURE: COULD NOT READ INTO MEMORY\n");
		return -1;
	}

	if(lseek(vdi.fd,vdi.hdr.offset_blocks,SEEK_SET) == -1) {
		printf("VDI Map Read: LSEEK FAILURE\n");
		return -1;
	}

	if(read(vdi.fd, vdi.map, 4*vdi.hdr.blocks_allocated) == -1) {
		printf("VDI Map Read: READ FAILURE\n");
 		return -1;
	}
}

u32 VDI_translate(u32 desired_byte) {

	u32 page;
	u32 offset;
	u32 loc;
	u32 frame;

	page = (desired_byte / vdi.hdr.offset_blocks);
	offset = (desired_byte % vdi.hdr.offset_blocks);
	frame = vdi.map[page];
	loc = vdi.hdr.offset_data + (frame * vdi.hdr.offset_blocks) + offset;

	return loc;
}

u32 get_partition_details(BootSector boot_sector){

	for (i=0;i<4;i++) {
		if(boot_sector.partitionTable[i].type == 0x83)
			break;
	}
	return boot_sector.partitionTable[i].firstSector * 512;
}

s32 get_bg_descriptor_table(bg_descriptor *bg_data) {
printf("Here:\n");
	u8 *temp = (u8*)malloc(vdi.block_size);
	bg_data = (bg_descriptor*)malloc(sizeof(bg_descriptor)*vdi.no_groups);

	if(vdi.block_size > 1024) {

		fetch_block(1,temp);
		memcpy(bg_data, temp, (sizeof(bg_descriptor)*vdi.no_groups));
		printf("HERE:\n");
	}

	else {

		fetch_block(2,temp);
		memcpy(bg_data, temp, (sizeof(bg_descriptor)*vdi.no_groups));
		printf("HEREO\n");
	}

	free(temp);
}

s32 fetch_block(s32 num, void *buff) {

	u32 position = num * vdi.block_size;

	if(vdi_seek(position) == -1) {
		printf("Fetch Block: LSEEK FAILURE\n");
		return -1;
	}
	if(vdi_read(buff) == -1) {
		printf("Fetch Block: READ FAILURE\n");
		return -1;
	}
}
s32 vdi_seek(u32 position) {

	u32 loc = VDI_translate(vdi.start + 1024 + position);

	if(lseek(vdi.fd, loc, SEEK_SET) == -1 ) {
		printf("VDI_SEEK: FAILURE\n");
		return -1;
	}
}
s32 vdi_read(void *buff) {
	if(read(vdi.fd, buff, vdi.block_size) == -1) {
		printf("VDI_READ: FAILURE\n");
		return -1;
	}
}

u32 read_into_buffer(void *buff, u32 position, u32 num_bytes) {

	if(lseek(vdi.fd, position, SEEK_SET) == -1) {
		printf("Read into buffer: LSEEK FAILURE\n");
		return -1;
	}
	if(read(vdi.fd, buff, num_bytes) == -1) {
		printf("Read into buffer: READ FAILURE\n");
		return -1;
	}
}

u32 block_buf_allocate(u32 block_size, arb_block *block ) {

	block->buff = (u8 *)malloc(block_size);

	if(block->buff == NULL) {
		printf("Block_Buf_Allocate: Memory Not Allocated!\n");
		return -1;
	}
}

u32 get_inode_bitmap(u32 fd, u32 bock_group, u8 *inode_bitmap, bg_descriptor *table, ext2_super_block main_sb) {



	//for (i = 0; i < (main_sb.s_blocks_count / main_sb.s_blocks_per_group) + 1; i++) {

		
		block_num = table[block_group].bg_inode_bitmap;

		fetch_block(block_num, inode_bitmap);
        //}
}

u32 get_block_bitmap(u32 fd, u32 bock_group, u8 *block_bitmap, bg_descriptor *table, ext2_super_block main_sb, VDI_file file ){

	


	//for (i = 0; i < (main_sb.s_blocks_count / main_sb.s_blocks_per_group) + 1; i++) {

	

		block_num = table[block_group].bg_block_bitmap;

		fetch_block(block_num, inode_bitmap);
        //}
}

/*u32 get_inode_table(u32 fd, u32 start, bg_descriptor table, ext2_super_block main_sb, inode_table *i_table, VDI_file file ) {

    i_table =  malloc(sizeof(ext2_inode) * main_sb.s_inodes_count);
    u32 inode_table_start;
	for (i = 0; i < (main_sb.s_blocks_count / main_sb.s_blocks_per_group) + 1; i++) {

		inode_table_start = start + 1024 + table.bg_descriptor[i].bg_inode_table;

		if(lseek(fd, VDI_translate(inode_table_start, file), SEEK_SET) == -1) {
			printf("Get iNode Table: LSEEK FAILURE\n");
			return -1;
		}

		if(read(fd, i_table, main_sb.s_log_block_size * 23) == -1) {
			printf("Get iNode Table: READ FAILURE\n");
			return -1;
		}
	}
}*/

get_inode(u32 start, u32 inode_num, bg_descriptor *table, ext2_super_block main_sb, inode_info* inode ){
    u32 group;
    u32 inode_in_block;
    u32 start;
    u32 block_num;
    u32 inodes_per_block = sizeof(inode_info)/main_sb.s_log_block_size;
    
    inode_num--;
    group = inode_num/main_sb.s_inodes_per_group;
    inode_in_block = inode_num%main_sb.s_inodes_per_block;
    start = table[group].bg_inode_table;
    block_num = start + inode_in_block/inodes_per_block;
    
    fetch_block(block_num, vdi );
}

u32 compare_sb(ext2_super_block a, ext2_super_block b) {

	printf("Comparing super blocks.\n");
	if(a.s_log_block_size < 1024) {
		a.s_log_block_size = 1024 << a.s_log_block_size;
	}

	if(b.s_log_block_size < 1024) {
		b.s_log_block_size = 1024 << b.s_log_block_size;
	}

	if(a.s_inodes_count != b.s_inodes_count) {
		printf("Discrepancy found: iNode count is inaccurate.\n");
		//return -1;
	}
	 if(a.s_blocks_count != b.s_blocks_count) {
		printf("Discrepancy found: Total blocks count is inaccurate.\n");
		//return -1;
	}
	if(a.s_r_blocks_count != b.s_r_blocks_count) {
		printf("Discrepancy found: Reserved blocks count is inaccurate\n");
		//return -1;
	}
	if(a.s_first_data_block != b.s_first_data_block) {
		printf("Discrepancy found: First data block address inaccurate\n");
		//return -1;
	}
	if(a.s_log_block_size != b.s_log_block_size) {
		printf("Discrepancy found: Block size inaccurate\n");
		//return -1;
	}
	if(a.s_blocks_per_group != b.s_blocks_per_group) {
		printf("Discrepancy found: Block per group inaccurate.\n");
		//return -1;
	}
	if(a.s_frags_per_group != b.s_frags_per_group) {
		printf("Discrepancy found: Fragments per group inaccurate.\n");
		//return -1;
	}
	if(a.s_magic != b.s_magic) {
		printf("Discrepancy found: Mysterious magical number inaccurate.\n");
		//return -1;
	}
	if(a.s_minor_rev_level != b.s_minor_rev_level) {
		printf("Discrepancy found: Minor revision level inaccurate.\n");
		//return -1;
	}
	if(a.s_creator_os != b.s_creator_os) {
		printf("Discrepancy found: Creator OS inaccurate.\n");
		//return -1;
	}
	if(a.s_rev_level != b.s_rev_level) {
		printf("Discrepancy found: Revision level inaccurate.\n");
		//return -1;
	}
	if(a.s_first_ino != b.s_first_ino) {
		printf("Discrepancy found: Address of first inode inaccurate.\n");
		//return -1;
	}
	if(a.s_inode_size != b.s_inode_size) {
		printf("Discrepancy found: iNode size inaccurate.\n");
		//return -1;
	}
	else {
		printf("Superblock: OK!\n");
		return 0;
	}
}

u32 compare_bg_desc_table(bg_descriptor *a, bg_descriptor *b, u32 no_block_grps) {

	printf("Comparing Block Group Descriptor Tables.\n");

        for(int i = 0; i < no_block_grps; i++){

            if(a[i].bg_block_bitmap != b[i].bg_block_bitmap) {
                    printf("Discrepancy found in Block Group Descriptor {%i}: Block bitmap is inaccurate.\n",i);
                    //return -1;
            }
             if(a[i].bg_inode_bitmap != b[i].bg_inode_bitmap) {
                     printf("Discrepancy found in Block Group Descriptor {%i}: Inode bitmap is inaccurate.\n",i);

                    //return -1;
            }
            if(a[i].bg_inode_table != b[i].bg_inode_table) {
                    printf("Discrepancy found in Block Group Descriptor {%i}: Inode table is inaccurate.\n",i);
                    //return -1;
            }
            if(a[i].bg_free_blocks != b[i].bg_free_blocks) {
                    printf("Discrepancy found in Block Group Descriptor {%i}: Free inode count is inaccurate.\n",i);
                    //return -1;
            }
            if(a[i].bg_free_inodes != b[i].bg_free_inodes) {
                    printf("Discrepancy found in Block Group Descriptor {%i}: Free inode count is inaccurate.\n",i);
                    //return -1;
            }
            if(a[i].bg_used_dirs_count != b[i].bg_used_dirs_count) {
                    printf("Discrepancy found in Block Group Descriptor {%i}: Used directories count is inaccurate.\n",i);
                    //return -1;
            }
            if(a[i].bg_pad != b[i].bg_pad) {
                    printf("Discrepancy found in Block Group Descriptor {%i}: Block group pad is inaccurate.\n",i);
                    //return -1;
            }
            if(a[i].bg_reserved != b[i].bg_reserved) {
                    printf("Discrepancy found in Block Group Descriptor {%i}: Reserved blocks count is inaccurate.\n",i);
                    //return -1;
            }

            else {
                    printf("Block Group Descriptor Table: OK!\n");
                    return 0;
            }
        }
}



//void get_block_bitmap(struct ext2_inode *i_info, arb_block *block_bitmap,bg_descriptor gd_info, u32 start ){
//
//
//    inode_bitmap = (u8 *)malloc(main_sb.s_blocks_count/8);
//
//
//    for (i = 0, i < (main_sb.s_blocks_count / main_sb.s_blocks_per_group) + 1, i++){
//        u32 inode_table_start = gd_info[i].bg_inode_table;
//        int inode_index = (inode_num - 1) % main_sb.s_inodes_per_group;
//        int block_num = (inode_table_start + inode_index)/main_sb.s_inodes_per_group
//
//        fetch_block(fd, *inode_bitmap, block_num, main_sb.s_log_block_size, disk_info, start);
//        }
////
////    u32 inode_sector_offset = inode_index/(sector_size_per_bytes/main_sb.s_inode_size);
////
////
////    lseek(start + (inode_table_start * sectors_per_block) + inode_sector_offset, SEEK_SET);
////     read(fd, &i_info, inode_index);
////    u32 num_inodes_sector = sector_size_bytes / super.s_inode_size;
////    u32 offset = ((inode_index % num_inodes_sector)) * main_sb.s_inode_size;
////    i_info->i_mode = read_bytes(buf, offset + 0, 2);
////    i_info->i_size = read_bytes(buf, offset + 4, 4);
////    i_info->i_links_count = read_bytes(buf, offset + 26, 2);
////    for(int i = 0; i < 15; i++){
////        i_info->i_block[i] = read_bytes(buf, offset + 40 + (i * 4), 4);
//
//}

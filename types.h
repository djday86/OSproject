#ifndef _TYPES_H
#define _TYPES_H

#include "datatypes.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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

//DEFINES FOR CHECKING filesystem
#define EXT2_VALID_FS 1
#define EXT2_ERROR_FS 2


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
	u32 cursor,fd,start,block_size,no_groups,blocks_pg,iNodesPerBlock,addrPerBlock;
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

typedef struct{
    s32 inode;
    s16 rec_len;
    s16 name_len;
    char name[];
} ext2_dir_entry;


u32 read_VDI_map();
u32 get_partition_details(BootSector boot_sector);
u32 VDI_translate(u32 desired_byte);
u32 read_into_buffer(void *buff, u32 position, u32 num_bytes);
u32 fetch_bg_block(void *buff, int block_grp_no);
s32 fetch_block( s32 num, void *buff);
s32 fetch_inode(u32 inode_num, bg_descriptor *table, inode_info* inode );
u32 block_buf_allocate(u32 block_size, arb_block *block_buf );
s32 get_bg_descriptor_table(bg_descriptor *bg_data, int block_grp_no);
u32 compare_sb(ext2_super_block a, ext2_super_block b);
u32 compare_bg_desc_table(bg_descriptor *a, bg_descriptor *b);
u32 get_block_bitmap(u32 bock_group, u8 *block_bitmap);
u32 get_inode_bitmap(u32 bock_group, u8 *inode_bitmap);
u32 get_inode(u32 inode_num, inode_info* inode );
u8 get_bit(u8 *bitmap, int bit_num);
u32 get_used_blocks(int inode_num, int* user_block_bitmap, inode_info *inode);
u32 get_array_final( int block_num, int* user_block_bitmap, int array_size);
u32 get_array_1(int block_num, int *user_block_bitmap, int array_size);
u32 get_array_2(int block_num, int *user_block_bitmap, int array_size);
u32 compare_block_bitmap(int block_grp_no, int *user_block_bitmap, u8* block_bitmap);
u32 compare_inode_bitmap(int block_grp_no, int *user_block_bitmap, u8* block_bitmap);
s32 vdi_seek(u32 position);
u32 get_used_blocks(int inode_num, int* user_block_bitmap, inode_info *inode);
s32 vdi_read(void *buff);
u32 superblock_check(ext2_super_block main_sb);
u32 bg_desc_table_check(bg_descriptor *a);
void dumpExt2File();
u32 get_file_directory();
u32 compare_dir_entries(int *dir_inode_bitmap);

VDI_file vdi;
u8 *temp_block;
ext2_super_block main_sb;
bg_descriptor *desc_table;
u32 i;



#endif

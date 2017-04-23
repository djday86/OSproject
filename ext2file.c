#include <stdio.h>
#include <stdlib.h>
#include <vdi.h>
#include <mbr.h>
#include <ext2file.h>

Ext2File *ext2Open(char *fileName) {
	Ext2File *f;
	PartitionTable p;
	int i;
	u32 maxBlocks,gdPerBlock,gdtBlocksUsed;

	f = (Ext2File *)malloc(sizeof(Ext2File));
	if (f == NULL) {
		perror("ext2Open malloc");
		goto error_done;
	}

	f->vdi = vdiOpen(fileName);
	if (f->vdi == NULL) {
		perror("ext2Open vdiOpen");
		goto error_f_allocated;
	}

	if (loadPartitionTable(f->vdi,&p)) {
		perror("ext2Open loadPartitionTable");
		goto error_file_opened;
	}

	for (i=0;i<4;i++) {
		if (p.partitions[i].type == 0x83) {
			f->partitionStart = p.partitions[i].firstSector * 512;
//			printf("start: %llu\n",f->partitionStart);
			break;
		}
	}

	if (i == 4) {
		perror("ext2Open no valid partition");
		goto error_file_opened;
	}

	vdiSeek(f->vdi,1024+f->partitionStart,SEEK_SET);
	vdiRead(f->vdi,&(f->sb),sizeof(struct ext2_super_block));
	if ((u16)(f->sb.s_magic) != (u16)0xef53) {
		perror("ext2Open superblock bad magic");
		goto error_file_opened;
	}

	f->blockSize = 1024 << f->sb.s_log_block_size;
	f->nBlockGroups = (f->sb.s_blocks_count + f->sb.s_blocks_per_group - 1) /
						f->sb.s_blocks_per_group;

	f->bgdt = (struct ext2_group_desc *)malloc(f->nBlockGroups *
					sizeof(struct ext2_group_desc));
	if (f->bgdt == NULL) {
		perror("ext2Open bgdt malloc");
		goto error_file_opened;
	}

	vdiSeek(f->vdi,(1+f->sb.s_first_data_block)*f->blockSize+f->partitionStart,
		SEEK_SET);
	i = f->nBlockGroups * sizeof(struct ext2_group_desc);
	if (vdiRead(f->vdi,f->bgdt,i) != i) {
		perror("ext2Open read bgdt");
		goto error_bgdt_allocated;
	}

	f->block = malloc(f->blockSize);
	if (f->block == NULL) {
		perror("ext2Open block malloc");
		goto error_bgdt_allocated;
	}

	f->addrPerBlock = f->blockSize / sizeof(u32);
	f->inodesPerBlock = f->blockSize / f->sb.s_inode_size;

	gdPerBlock = f->blockSize / sizeof(struct ext2_group_desc);
	gdtBlocksUsed = (f->nBlockGroups + gdPerBlock - 1) / gdPerBlock;

	maxBlocks = 0x003fffff;
	if (maxBlocks > f->sb.s_blocks_count)
		maxBlocks = f->sb.s_blocks_count;
	maxBlocks <<= 10;
	maxBlocks = (maxBlocks + f->sb.s_blocks_per_group - 1 -
					f->sb.s_first_data_block) / f->sb.s_blocks_per_group;
	f->nGDTBlocks = (maxBlocks + gdPerBlock - 1) / gdPerBlock - gdtBlocksUsed;
	if (f->nGDTBlocks > f->addrPerBlock)
		f->nGDTBlocks = f->addrPerBlock;
	f->nGDTBlocks += gdtBlocksUsed;

	return f;

error_bgdt_allocated:
	free(f->bgdt);

error_file_opened:
	vdiClose(f->vdi);

error_f_allocated:
	free(f);

error_done:
	return NULL;
}


/* from https://fossies.org/linux/e2fsprogs/lib/ext2fs/initialize.c

static unsigned int calc_reserved_gdt_blocks(ext2_filsys fs)
   55 {
   56     struct ext2_super_block *sb = fs->super;
   57     unsigned long bpg = sb->s_blocks_per_group;
   58     unsigned int gdpb = EXT2_DESC_PER_BLOCK(sb);
   59     unsigned long max_blocks = 0xffffffff;
   60     unsigned long rsv_groups;
   61     unsigned int rsv_gdb;
   62
   63     /* We set it at 1024x the current filesystem size, or
   64      * the upper block count limit (2^32), whichever is lower.
   65      *
   66     if (ext2fs_blocks_count(sb) < max_blocks / 1024)
   67         max_blocks = ext2fs_blocks_count(sb) * 1024;
   68     /*
   69      * ext2fs_div64_ceil() is unnecessary because max_blocks is
   70      * max _GDT_ blocks, which is limited to 32 bits.
   71      *
   72     rsv_groups = ext2fs_div_ceil(max_blocks - sb->s_first_data_block, bpg);
   73     rsv_gdb = ext2fs_div_ceil(rsv_groups, gdpb) - fs->desc_blocks;
   74     if (rsv_gdb > EXT2_ADDR_PER_BLOCK(sb))
   75         rsv_gdb = EXT2_ADDR_PER_BLOCK(sb);
   76 #ifdef RES_GDT_DEBUG
   77     printf("max_blocks %lu, rsv_groups = %lu, rsv_gdb = %u\n",
   78            max_blocks, rsv_groups, rsv_gdb);
   79 #endif
   80
   81     return rsv_gdb;
   82 }

*/

void ext2Close(Ext2File *f) {

	free(f->block);

	free(f->bgdt);

	vdiClose(f->vdi);

	free(f);
}

void dumpExt2File(Ext2File *f) {
	int i;

	printf(	"            Blocks       Inodes\n"
			"   Total: %10d   %10d\n"
			"    Free: %10d   %10d\n"
			"Reserved: %10d\n\n"
			"First data block: %10d\n"
			"      Block size: %10d\n"
			"    Block groups: %10d\n"
			"Blocks per group: %10d\n"
			"Inodes per group: %10d\n"
			"      GDT blocks: %10d\n"
			"Inodes per block: %10d\n"
			" Addrs per block: %10d\n\n",
		f->sb.s_blocks_count,f->sb.s_inodes_count,
		f->sb.s_free_blocks_count,f->sb.s_free_inodes_count,
		f->sb.s_r_blocks_count,
		f->sb.s_first_data_block,
		f->blockSize,
		f->nBlockGroups,
		f->sb.s_blocks_per_group,
		f->sb.s_inodes_per_group,
		f->nGDTBlocks,
		f->inodesPerBlock,
		f->addrPerBlock
	);

	printf("Group    Block map    Inode map    Inode tbl   bFree   iFree\n");
	for (i=0;i<f->nBlockGroups;i++)
		printf("%5d   %10d   %10d   %10d   %5d   %5d\n",i,
			f->bgdt[i].bg_block_bitmap,
			f->bgdt[i].bg_inode_bitmap,
			f->bgdt[i].bg_inode_table,
			f->bgdt[i].bg_free_blocks_count,
			f->bgdt[i].bg_free_inodes_count);
	putchar('\n');
}

u32 fetchBlock(Ext2File *f,u32 b,u8 *buf) {

	if (f == NULL)
		return 0;

	vdiSeek(f->vdi,f->blockSize*b+f->partitionStart,SEEK_SET);
	return vdiRead(f->vdi,buf,f->blockSize);
}

u32 writeBlock(Ext2File *f,u32 b,u8 *buf) {
	return 0;
}

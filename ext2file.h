#ifndef _EXT2FILE_H
#define _EXT2FILE_H

#include <datatypes.h>
#include <vdi.h>
#include <ext2.h>

typedef struct _Ext2File {
	struct ext2_super_block
		sb;
	VDIFile
		*vdi;
	u64
		partitionStart;
	u32
		blockSize,
		nBlockGroups,
		nGDTBlocks,
		inodesPerBlock,
		addrPerBlock;
	struct ext2_group_desc
		*bgdt;
	void
		*block;
} Ext2File;

Ext2File
	*ext2Open(char *);
void
	ext2Close(Ext2File *),
	dumpExt2File(Ext2File *),
	dumpBlock(Ext2File *,u32);
u32
	fetchBlock(Ext2File *,u32,u8 *),
	writeBlock(Ext2File *,u32,u8 *);

#endif

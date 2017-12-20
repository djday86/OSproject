#include "datatypes.h"
#include "types.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


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
        
        //find which partition entry is used
	for (i=0;i<4;i++) {
		if(boot_sector.partitionTable[i].type == 0x83)
			break;
	}
	return boot_sector.partitionTable[i].firstSector * 512;
}

u32 get_bg_descriptor_table(bg_descriptor *bg_data, int block_group_no) {

	u8 *temp = (u8*)malloc(vdi.block_size);

	if(vdi.block_size > 1024 && block_group_no == 0) {
            fetch_block(1, temp);
            memcpy(bg_data, temp, sizeof(bg_descriptor) * vdi.no_groups);
            }

	else{
                fetch_block(2 + block_group_no * vdi.blocks_pg * vdi.block_size,temp);
                memcpy(bg_data, temp, sizeof(bg_descriptor) * vdi.no_groups);
            }
	free(temp);
        return 0;
}

s32 fetch_block(s32 num, void *buff) {

	u32 position = num* vdi.block_size;

	if(vdi_seek(position) == -1) {
		printf("Fetch Block: LSEEK FAILURE\n");
		return -1;
	}
	if(vdi_read(buff) == -1) {
		printf("Fetch Block: READ FAILURE\n");
		return -1;
	}
        return 0;
}


s32 vdi_seek(u32 position) {

	u32 loc = VDI_translate(vdi.start + position);

	if(lseek(vdi.fd, loc, SEEK_SET) == -1 ) {
		printf("VDI_SEEK: FAILURE\n");
		return -1;
	}

        return 0;
}

s32 vdi_read(void *buff) {
	if(read(vdi.fd, buff, vdi.block_size) == -1) {
		printf("VDI_READ: FAILURE\n");
		return -1;
	}
        return 0;
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

u32 get_inode_bitmap(u32 block_group, u8 *inode_bitmap) {
    int block_num;

    u8 *block_buf = (u8*)malloc(vdi.block_size);

    block_num = desc_table[block_group].bg_inode_bitmap;
    fetch_block(block_num, block_buf);
    memcpy(inode_bitmap, block_buf, sizeof(u8)* main_sb.s_inodes_per_group/8);

    printf("Got inode bitmap\n");
    free(block_buf);
    return 0;

}

u32 get_block_bitmap(u32 block_group, u8 *block_bitmap){
    int block_num;
    u8 *block_buf = (u8*)malloc(vdi.block_size);

    block_num = desc_table[block_group].bg_block_bitmap;
    fetch_block(block_num, block_buf);
    memcpy(block_bitmap, block_buf, sizeof(u8) * main_sb.s_blocks_per_group/8);

    printf("Got block bitmap\n");
    free(block_buf);
    return 0;


}

u32 get_inode(int inode_num, inode_info *inode ){

    u32 group; //block group
    u32 inode_in_group; //inode inside the block group
    u32 start_point;  //start of inode table
    u32 block_num;
    inode_info *block_buf = (inode_info*)malloc(vdi.block_size);
    u32 inodes_per_block = vdi.block_size/sizeof(inode_info);

    inode_num--;
    group = inode_num/main_sb.s_inodes_per_group;
    inode_in_group = inode_num % main_sb.s_inodes_per_group;
    
    //find inode table
    start_point = desc_table[group].bg_inode_table;
    block_num = start_point + inode_in_group/inodes_per_block;

    fetch_block(block_num, block_buf );
    memcpy(inode, block_buf + (inode_num % inodes_per_block), sizeof(inode_info));

    free(block_buf);
    return 0;
}

u32 compare_sb(ext2_super_block a, ext2_super_block b, int block_group_no) {

    int error = 0; //Zero if no errors
    
	printf("Comparing super blocks.\n");

	if(a.s_inodes_count != b.s_inodes_count) {
		printf("Discrepancy found: iNode count is inaccurate.\n");
                error = 1;
	}
	 if(a.s_blocks_count != b.s_blocks_count) {
		printf("Discrepancy found: Total blocks count is inaccurate.\n");
		error = 1;
	}
	if(a.s_r_blocks_count != b.s_r_blocks_count) {
		printf("Discrepancy found: Reserved blocks count is inaccurate\n");
		error = 1;
	}
	if(a.s_first_data_block != b.s_first_data_block) {
		printf("Discrepancy found: First data block address inaccurate\n");
		error = 1;
	}
	if(1024 << a.s_log_block_size != 1024 << b.s_log_block_size) {
		printf("Discrepancy found: Block size inaccurate\n");
		error = 1;
	}
	if(a.s_blocks_per_group != b.s_blocks_per_group) {
		printf("Discrepancy found: Block per group inaccurate.\n");
		error = 1;
	}
	if(a.s_frags_per_group != b.s_frags_per_group) {
		printf("Discrepancy found: Fragments per group inaccurate.\n");
		error = 1;
	}
	if(a.s_magic != b.s_magic) {
		printf("Discrepancy found: Mysterious magical number inaccurate.\n");
		error = 1;
	}
	if(a.s_minor_rev_level != b.s_minor_rev_level) {
		printf("Discrepancy found: Minor revision level inaccurate.\n");
		error = 1;
	}
	if(a.s_creator_os != b.s_creator_os) {
		printf("Discrepancy found: Creator OS inaccurate.\n");
		error = 1;
	}
	if(a.s_rev_level != b.s_rev_level) {
		printf("Discrepancy found: Revision level inaccurate.\n");
		error = 1;
	}
	if(a.s_first_ino != b.s_first_ino) {
		printf("Discrepancy found: Address of first inode inaccurate.\n");
		error = 1;
	}
	if(a.s_inode_size != b.s_inode_size) {
		printf("Discrepancy found: iNode size inaccurate.\n");
		error = 1;
	}
	if(error == 0){
            printf("No errors found in backup superblock in block group%i\n", block_group_no);
		return 0;
	}
}

u32 compare_bg_desc_table(bg_descriptor *a, bg_descriptor *b, int block_group_no) {

	for(int k = 0; k < vdi.no_groups; k++) {
	  if(a[k].bg_block_bitmap != b[k].bg_block_bitmap) {
	          printf("Discrepancy found in Block Group Descriptor %i: Block bitmap is inaccurate.\n",k);
	          //return -1;
	  }
	   if(a[k].bg_inode_bitmap != b[k].bg_inode_bitmap) {
	           printf("Discrepancy found in Block Group Descriptor %i: Inode bitmap is inaccurate.\n",k);
						 
	  }
	  if(a[k].bg_inode_table != b[k].bg_inode_table) {
	          printf("Discrepancy found in Block Group Descriptor %i: Inode table is inaccurate.\n",k);
	          
	  }
	  if(a[k].bg_free_blocks != b[k].bg_free_blocks) {
	          printf("Discrepancy found in Block Group Descriptor %i: Free inode count is inaccurate.\n",k);
	          
	  }
	  if(a[k].bg_free_inodes != b[k].bg_free_inodes) {
	          printf("Discrepancy found in Block Group Descriptor %i: Free inode count is inaccurate.\n",k);
	          
	  }
	  if(a[k].bg_used_dirs_count != b[k].bg_used_dirs_count) {
	          printf("Discrepancy found in Block Group Descriptor %i: Used directories count is inaccurate.\n",k);
	          
	  }
	  if(a[k].bg_pad != b[k].bg_pad) {
	          printf("Discrepancy found in Block Group Descriptor %i: Block group pad is inaccurate.\n",k);
	          
	  }     
	}
        printf("No errors found in backup descriptor table in block group%i\n", block_group_no);
        return 0;
}

u8 set_bit(u8 *bitmap, int bit_num){
    int i;
    int j;
    bit_num = bit_num % main_sb.s_blocks_per_group;
    i = bit_num/8;
    j = bit_num % 8;

    return (bitmap[i] & (1 << j));
}
u8 get_bit(u8 *bitmap, int bit_num) {
    int i;
    int j;

    i = bit_num/8;
    j = bit_num%8;

    return (bitmap[i] & (1 << j));
}

u32 get_used_blocks(int inode_num, u8* user_block_bitmap, inode_info* inode){
    if(inode->i_block[0] == 0){
        return 0;
    }
    
    //check all direct blocks
    for (int i = 0; i < EXT2_N_BLOCKS; i++){
        if(inode->i_block[i] != 0){
            set_bit(user_block_bitmap,inode->i_block[i] - main_sb.s_first_data_block);
        }
    }

    //Get indirect blocks
    get_indirect_1(inode->i_block[12], user_block_bitmap);
    get_indirect_2(inode->i_block[13], user_block_bitmap);
    get_indirect_3(inode->i_block[14], user_block_bitmap);

    return 0;
}

u32 get_indirect_1( int block_num, u8* user_block_bitmap){
    u32 *block_buf = (u32*)malloc(vdi.block_size);

    fetch_block(block_num, block_buf);

    if(block_buf[0] == 0){
        return 0;
    }

    for( int i = 0; i < vdi.block_size/sizeof(u32); i++){
        if(block_buf[i] != 0){
            set_bit(user_block_bitmap,block_buf[i] - main_sb.s_first_data_block);
        }
    }

    return 0;
}

u32 get_indirect_2(int block_num, u8 *user_block_bitmap){
    u32 *block_buf = (u32*)malloc(vdi.block_size);

    fetch_block(block_num, block_buf);

    if(block_buf[0] == 0)
        return 0;

    for( int i = 0; i < vdi.block_size/sizeof(u32); i++){
        get_indirect_1(block_buf[i], user_block_bitmap);
    }

    free(block_buf);
    return 0;
}

u32 get_indirect_3(int block_num, u8 *user_block_bitmap){
    u32 *block_buf = (u32*)malloc(vdi.block_size);

    fetch_block(block_num, block_buf);

    if(block_buf[0] == 0)
        return 0;

    for( int i = 0; i < vdi.block_size/sizeof(u32); i++){
        get_indirect_2(block_buf[i], user_block_bitmap);
    }
    free(block_buf);
    return 0;
}

u32 compare_block_bitmap(int block_grp_no, u8 *user_block_bitmap, u8* block_bitmap){
    int start = block_grp_no * main_sb.s_blocks_per_group;
    int end = (block_grp_no + 1) * main_sb.s_blocks_per_group;
    bool error = false;
    int bad1 = 0; //used block not referenced
    int bad2 = 0; //unused block referenced

    for(int i = start; i < end; i++){
        if(get_bit(user_block_bitmap, i) == get_bit(block_bitmap ,i%main_sb.s_blocks_per_group)){
            //printf("Blocks %i match\n", i + 1);
            continue;
        }
        else if (user_block_bitmap[i] > 0){
            //printf("Block %i is used but not referenced by an inode.\n", i + main_sb.s_first_data_block);
            error = true;
            bad1++;
        }
        else if(user_block_bitmap[i] == 0){
            //printf("Unused data block %i referenced by inode\n", i + main_sb.s_first_data_block);
            error = true;
            bad2 ++;
        }
        else
            printf("Bit value not 0 or 1\n");
    }

    if(error == false)
        printf("No block errors found in block group %i.\n", block_grp_no);
    
    if(error == true){
        printf("Number of used blocks not refernced in blocks group %i : %i\n", block_grp_no, bad1);
        printf("Number of unused blocks referenced in blocks group %i : %i\n", block_grp_no, bad2);
    }

    return 0;
}

u32 compare_inode_bitmap(int block_grp_no, u8 *user_inode_bitmap, u8* inode_bitmap){
    int start = block_grp_no * main_sb.s_inodes_per_group;
    int end = (block_grp_no + 1) * main_sb.s_inodes_per_group;
    bool error = false;
    int bad1 = 0; //unused inode in root
    int bad2 = 0; //used inode not in root

    for(int i = start; i < end; i++){

        if(get_bit(user_inode_bitmap, i) == get_bit(inode_bitmap ,i%main_sb.s_inodes_per_group)){
        }
        else if (get_bit(user_inode_bitmap, i) > 0){
            //printf("Unused inode %i reachable from the root directory.\n", i + 1);
            error = true;
            bad1++;
        }
        else if(get_bit(user_inode_bitmap, i) == 0){
           // printf("Inode %i unreachable from the root directory.\n", i + 1);
            error = true;
            bad2++;
        }
        else
            printf("Bit value not 0 or 1\n");
    }

    if(error == false)
        printf("No inode errors found in block group %i.\n", block_grp_no);
    if(error == true){
        printf("Number of unused inodes reachable from the root in block group %i : %i\n", block_grp_no, bad1);
        printf("Number of used inodes unreachable from root in block group %i : %i\n", block_grp_no, bad2);
    }
    return 0;
}

u32 superblock_check(ext2_super_block main_sb) {

	if(vdi.block_size > 2048) {
		printf("No copies of super block are available on this drive.\n");
		return 1;
	}
	ext2_super_block copy;

	fetch_block(1*vdi.blocks_pg+1, &copy);
	compare_sb(main_sb, copy, 1);

	for(int j = 3; j < 8; j+=2) {
		for(i = j; i < vdi.no_groups; i = i * i) {
			fetch_block(i*vdi.blocks_pg+1, &copy);
			compare_sb(main_sb, copy, i);
		}
	}
	return 0;
}
u32 bg_desc_table_check(bg_descriptor *a) {

	if(vdi.block_size > 2048) {
		printf("No copies of block group descriptor table are available on this drive.\n");
		return 1;
	}

	bg_descriptor *b = (bg_descriptor*)malloc(sizeof(bg_descriptor) * vdi.no_groups);
        printf("Got here\n");
	get_bg_descriptor_table(b, 1);
	compare_bg_desc_table(a, b, 1);
	for(int j = 3;j < 8;j += 2) {
		for(i=j;i < vdi.no_groups; i = i*i) {
			get_bg_descriptor_table(b,i);
			compare_bg_desc_table(desc_table,b, i);
		}
	}
        return 0;
}
void dumpExt2File(int used_files, int dir_count) {
	int i;
        u32 inodes_per_block = vdi.block_size/sizeof(inode_info);
        u32 addr_per_block = vdi.block_size/sizeof(u32);
        u32 file_system_size = main_sb.s_blocks_count * vdi.block_size;

	printf(	"            Blocks       Inodes\n"
			"   Total: %10d   %10d\n"
			"    Free: %10d   %10d\n"
			"Reserved: %10d\n\n"
			"First data block: %10d\n"
			"      Block size: %10d\n"
			"    Block groups: %10d\n"
			"Blocks per group: %10d\n"
			"Inodes per group: %10d\n"
			"Inodes per block: %10d\n"
			" Addrs per block: %10d\n\n"
                        "Number of files: %10d\n\n"
                        "Number of directories: %10d\n\n"
                        "File System Size: %10d\n\n",
		main_sb.s_blocks_count, main_sb.s_inodes_count,
		main_sb.s_free_blocks_count,main_sb.s_free_inodes_count,
		main_sb.s_r_blocks_count,
		main_sb.s_first_data_block,
		vdi.block_size,
		vdi.no_groups,
		main_sb.s_blocks_per_group,
		main_sb.s_inodes_per_group,
		inodes_per_block,
		addr_per_block,
                used_files,
                dir_count,
                file_system_size
	);

	printf("Group    Block map    Inode map    Inode tbl   bFree   iFree\n");
	for (i=0;i<vdi.no_groups;i++)
		printf("%5d   %10d   %10d   %10d   %5d   %5d\n",i,
			desc_table[i].bg_block_bitmap,
			desc_table[i].bg_inode_bitmap,
			desc_table[i].bg_inode_table,
			desc_table[i].bg_free_blocks,
			desc_table[i].bg_free_inodes);
	    putchar('\n');
}

u32 traverse_directory(int dir_inode_num, u8 *user_block_bitmap, u8* user_inode_bitmap){

    ext2_dir_entry_2 *curr_dir = (ext2_dir_entry_2*)malloc(sizeof(ext2_dir_entry_2));
    int dir_size; //size of directory
    inode_info *dir_inode = (inode_info*)malloc(sizeof(inode_info));
    inode_info *inode = (inode_info*)malloc(sizeof(inode_info));
    u8 *block_buf;
    int root_dir; 
    int next_dir;
    int end_dir; //end of current directory

    //get directory inode
    get_inode(dir_inode_num, dir_inode);
    root_dir = dir_inode->i_block[0];
    dir_size = dir_inode->i_size;
    block_buf = (u8*)malloc(vdi.block_size);  
    fetch_block(root_dir, block_buf);
    memcpy(curr_dir, block_buf, sizeof(ext2_dir_entry_2));
    set_bit(user_inode_bitmap,curr_dir->inode - 1);
    get_used_blocks(curr_dir->inode, user_block_bitmap,dir_inode);
    next_dir = root_dir * vdi.block_size + curr_dir->rec_len;
    end_dir = root_dir * vdi.block_size + dir_inode->i_size;
    
    //while inside current directory
    while(next_dir < end_dir){
        if(vdi_seek(next_dir) == -1) {
            printf("LSEEK FAILURE\n");
            return -1;
        }

        if(read(vdi.fd,curr_dir, sizeof(ext2_dir_entry_2) ) == -1) {
            printf("READ FAILURE\n");
            return -1;
        }

        if(strcmp(curr_dir->name,".") == 0 || strcmp(curr_dir->name,"..") == 0){
            next_dir = next_dir + curr_dir->rec_len;
            continue;
        }

        if(curr_dir->inode == 0){
           next_dir = next_dir + curr_dir->rec_len;
           continue;
        }
        get_inode(curr_dir->inode, inode);
        set_bit(user_inode_bitmap, curr_dir->inode - 1);
        get_used_blocks(curr_dir->inode, user_block_bitmap, inode);

        if(inode->i_mode > 0x3fff && inode->i_mode < 0x5000){
            directory++;
            next_dir = next_dir + curr_dir->rec_len;
            traverse_directory(curr_dir->inode, user_block_bitmap, user_inode_bitmap);
        }
        else{
            next_dir = next_dir + curr_dir->rec_len;
            file++;
        }
    }

    free(block_buf);
    return 0;
}

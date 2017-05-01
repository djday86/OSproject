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
            printf("Right side\n");
                fetch_block(2 + block_group_no * vdi.blocks_pg * vdi.block_size,temp);
                memcpy(bg_data, temp, sizeof(bg_descriptor) * vdi.no_groups);
            }
        printf("Made bg table\n");
	free(temp);
        return 0;
}

//u32 fetch_bg_block(void *buff, int block_grp_no){
//    if(vdi_seek(block_grp_no * vdi.block_size * vdi.blocks_pg + 1024) == -1) {
//		printf("Fetch Block: LSEEK FAILURE\n");
//		return -1;
//	}
//	if(vdi_read(buff) == -1) {
//		printf("Fetch Block: READ FAILURE\n");
//		return -1;
//	}
//    return 0;
//}

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
        //printf("Fetch Complete\n");
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

    u32 group;
    u32 inode_in_group;
    u32 start_point;
    u32 block_num;
    inode_info *block_buf = (inode_info*)malloc(vdi.block_size);
    u32 inodes_per_block = vdi.block_size/sizeof(inode_info);

    inode_num--;
    group = inode_num/main_sb.s_inodes_per_group;
    inode_in_group = inode_num % main_sb.s_inodes_per_group;
    start_point = desc_table[group].bg_inode_table;
    block_num = start_point + inode_in_group/inodes_per_block;


    //printf("Block_num %i\n", block_num);
    fetch_block(block_num, block_buf );
    memcpy(inode, block_buf + (inode_num % inodes_per_block), sizeof(inode_info));

    //printf("Inode in group%i\n", inode_in_group);
    //printf("Inode id %d\n", inode->i_block[0]);
    free(block_buf);
    return 0;
}

u32 compare_sb(ext2_super_block a, ext2_super_block b) {

    int block_size_a;
    int block_size_b;
	printf("Comparing super blocks.\n");

	if(a.s_log_block_size < 1024) {
		block_size_a = 1024 << a.s_log_block_size;
	}

	if(b.s_log_block_size < 1024) {
		block_size_b = 1024 << b.s_log_block_size;
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
	if(block_size_a != block_size_b) {
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
		return 0;
	}
}

u32 compare_bg_desc_table(bg_descriptor *a, bg_descriptor *b) { 

	for(int k = 0; k < vdi.no_groups; k++) {
            //printf("Looping\n");
	  if(a[k].bg_block_bitmap != b[k].bg_block_bitmap) {
	          printf("Discrepancy found in Block Group Descriptor %i}: Block bitmap is inaccurate.\n",k);
	          //return -1;
	  }
	   if(a[k].bg_inode_bitmap != b[k].bg_inode_bitmap) {
	           printf("Discrepancy found in Block Group Descriptor %i: Inode bitmap is inaccurate.\n",k);

	          //return -1;
	  }
	  if(a[k].bg_inode_table != b[k].bg_inode_table) {
	          printf("Discrepancy found in Block Group Descriptor %i: Inode table is inaccurate.\n",k);
	          //return -1;
	  }
	  if(a[k].bg_free_blocks != b[k].bg_free_blocks) {
	          printf("Discrepancy found in Block Group Descriptor %i: Free inode count is inaccurate.\n",k);
	          //return -1;
	  }
	  if(a[k].bg_free_inodes != b[k].bg_free_inodes) {
	          printf("Discrepancy found in Block Group Descriptor %i: Free inode count is inaccurate.\n",k);
	          //return -1;
	  }
	  if(a[k].bg_used_dirs_count != b[k].bg_used_dirs_count) {
	          printf("Discrepancy found in Block Group Descriptor %i: Used directories count is inaccurate.\n",k);
	          //return -1;
	  }
	  if(a[k].bg_pad != b[k].bg_pad) {
	          printf("Discrepancy found in Block Group Descriptor %i: Block group pad is inaccurate.\n",k);
	          //return -1;
	  }
	  else {
              printf("Block Group Descriptor %i is accurate\n",k);
	  }
          //free(b);  
	}
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

    //printf("expected bitmap%i\n", (bitmap[i] & (1 << j) ));
    //printf("expected bitmap%i\n", (1 << j) );
    
    return (bitmap[i] & (1 << j));
//    if((bitmap[i] & (1 << j)) == 0){
//        //printf("bit = %i\n", 0);
//        return 0;
//    }
//    else{
//        //printf("bit = %i\n", 1);
//        return 1;
//    }
}

u32 get_used_blocks(int inode_num, u8* user_block_bitmap, inode_info* inode){
    u32 array_size;

    array_size = vdi.block_size/sizeof(u32);
    
    printf("Inode: %i\n", inode_num);
    
    if(inode->i_block[0] == 0){
        //printf("Inode %i is empty\n", inode_num);

        return 0;
    }
    
    //printf("Made it here");
    for (int i = 0; i < EXT2_N_BLOCKS; i++){
        set_bit(user_block_bitmap,inode->i_block[i] - main_sb.s_first_data_block);
        printf("Block %i = 1\n", inode->i_block[i]);
    }

    //Get indirect blocks
    get_array_final(inode->i_block[12], user_block_bitmap, array_size);
    get_array_1(inode->i_block[13], user_block_bitmap, array_size);
    get_array_2 (inode->i_block[14], user_block_bitmap, array_size);
    printf("Read Block Array Complete\n");
    
    return 0;
}

u32 get_array_final( int block_num, u8* user_block_bitmap, int array_size){
    u8 *block_buf = (u8*)malloc(vdi.block_size);
    u32 block_array[array_size];

    fetch_block(block_num, block_buf);
    memcpy(block_array, block_buf, sizeof(u32) * array_size);

    if(block_array[0] == 0){
        printf("Block %i is empty\n", block_num);
        return 0;
    }

    for( int i = 0; i < array_size; i++){
        set_bit(user_block_bitmap,block_array[i] - main_sb.s_first_data_block);
        printf("Block %i = 1\n", block_array[i]);
    }

    return 0;
}

u32 get_array_1(int block_num, u8 *user_block_bitmap, int array_size){
    u32 *block_buf = (u32*)malloc(vdi.block_size);
    //u32 block_array[array_size];

    fetch_block(block_num, block_buf);
    //memcpy(block_array, block_buf, sizeof(u32) * array_size);

    if(block_buf[0] == 0)
        return 0;

    for( int i = 0; i < array_size; i++){
        get_array_final(block_buf[i], user_block_bitmap, array_size);
    }
    
    free(block_buf);
    return 0;
}

u32 get_array_2(int block_num, u8 *user_block_bitmap, int array_size){
    u32 *block_buf = (u32*)malloc(vdi.block_size);
    //u32 block_array[array_size];

    fetch_block(block_num, block_buf);
    //memcpy(block_array, block_buf, sizeof(u32) * array_size);

    if(block_buf[0] == 0)
        return 0;

    for( int i = 0; i < array_size; i++){
        get_array_1(block_buf[i], user_block_bitmap, array_size);
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
            printf("Block %i is used but not referenced by an inode.\n", i + 1);
            error = true;
            bad1++;
        }
        else if(user_block_bitmap[i] == 0){
            printf("Unused data block %i referenced by inode\n", i + 1);
            error = true;
            bad2 ++;
        }
        else
            printf("Bit value not 0 or 1\n");
    }

    if(error == false)
        printf("No block errors found in block group %i.\n", block_grp_no);
    
    printf("Number of used blocks not refernced in blocks group %i : %i\n", block_grp_no, bad1);
    printf("Number of unused blocks referenced in blocks group %i : %i\n", block_grp_no, bad1);

    return 0;
}

u32 compare_inode_bitmap(int block_grp_no, u8 *user_inode_bitmap, u8* inode_bitmap){
    int start = block_grp_no * main_sb.s_inodes_per_group;
    int end = (block_grp_no + 1) * main_sb.s_inodes_per_group;
    bool error = false;
    int bad = 0;
    
    for(int i = start; i < end; i++){

        if(get_bit(user_inode_bitmap, i) == get_bit(inode_bitmap ,i%main_sb.s_inodes_per_group)){
            //printf("inodes  %i match\n", i + 1);
        }
        else if (get_bit(user_inode_bitmap, i) > 0){
            printf("Unused inode %i reachable from the root directory.\n", i + 1);
            error = true;
            bad++;
        }
        else if(get_bit(user_inode_bitmap, i) == 0){
            printf("Inode %i unreachable from the root directory.\n", i + 1);
            error = true;
            bad++;
        }
        else
            printf("Bit value not 0 or 1\n");
    }

    if(error == false)
        printf("No inode errors found in block group %i.\n", block_grp_no);
    
    printf("Bad elements in block group %i : %i\n", block_grp_no, bad);
    return 0;
}

u32 superblock_check(ext2_super_block main_sb) {

	if(vdi.block_size > 1024) {
		printf("No copies of super block are available on this drive.\n");
		return 1;
	}
	ext2_super_block copy;
	s32 block_group = vdi.block_size * vdi.blocks_pg;

	fetch_block(1*vdi.blocks_pg+1, &copy);
	compare_sb(main_sb, copy);

	for(int j = 3; j < 8; j+=2) {
		for(i = j;i < vdi.no_groups; i = i * i) {
			fetch_block(i*vdi.blocks_pg+1, &copy);
			compare_sb(main_sb, copy);
		}
	}
	return 0;
}
u32 bg_desc_table_check(bg_descriptor *a) {

//	if(vdi.block_size > 1024) {
//		printf("No copies of block group descriptor table are available on this drive.\n");
//		return 1;
//	}

	bg_descriptor *b = (bg_descriptor*)malloc(sizeof(bg_descriptor) * vdi.no_groups);
        printf("Got here\n");
	get_bg_descriptor_table(b, 1);
	compare_bg_desc_table(a,b);
	for(int j = 3;j < 8;j += 2) {
		for(i=j;i < vdi.no_groups; i = i*i) {
			get_bg_descriptor_table(b,i);
			compare_bg_desc_table(desc_table,b);
		}
	}
        //free(b);
        return 0;
}
void dumpExt2File() {
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
		main_sb.s_blocks_count, main_sb.s_inodes_count,
		main_sb.s_free_blocks_count,main_sb.s_free_inodes_count,
		main_sb.s_r_blocks_count,
		main_sb.s_first_data_block,
		vdi.block_size,
		vdi.no_groups,
		main_sb.s_blocks_per_group,
		main_sb.s_inodes_per_group,
		vdi.no_groups,
		vdi.iNodesPerBlock,
		vdi.addrPerBlock
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
    int dir_size;
    inode_info *dir_inode = (inode_info*)malloc(sizeof(inode_info));
    inode_info *inode = (inode_info*)malloc(sizeof(inode_info));
    u8 *block_buf;
    int root_dir;
    int next_dir;
    int end_dir;
    int blocks;
    
    
    get_inode(dir_inode_num, dir_inode);
    root_dir = dir_inode->i_block[0]; 
    dir_size = dir_inode->i_size;
    block_buf = (u8*)malloc(vdi.block_size);
    printf("Sent number %i\n", sizeof(ext2_dir_entry_2));
    fetch_block(root_dir, block_buf);
    memcpy(curr_dir, block_buf, sizeof(ext2_dir_entry_2));
    printf("Inode: %i\n", curr_dir->inode);
    set_bit(user_inode_bitmap,curr_dir->inode - 1);
    get_used_blocks(curr_dir->inode, user_block_bitmap,dir_inode);
    next_dir = root_dir * vdi.block_size + curr_dir->rec_len;
    end_dir = root_dir * vdi.block_size + dir_inode->i_size;
    
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
            printf("Should be here");
            next_dir = next_dir + curr_dir->rec_len;
            continue;
        }
        
        if(curr_dir->inode == 0){
           next_dir = next_dir + curr_dir->rec_len;
           continue;
        }
        printf("good\n");
        get_inode(curr_dir->inode, inode);
        set_bit(user_inode_bitmap, curr_dir->inode - 1);
        get_used_blocks(curr_dir->inode, user_block_bitmap, inode);
        
        if(get_bit(inode->i_mode, 15) > 0; ){
            directory++;
            next_dir = next_dir + curr_dir->rec_len;
            traverse_directory(curr_dir->inode, user_block_bitmap, user_inode_bitmap);
        }
        else if(get_bit(inode->i_mode,14) > 0){
            file++;
            next_dir = next_dir + curr_dir->rec_len;
        }
    }

    free(block_buf);
    return 0;    
}

//u32 compare_dir_entries(int *dir_inode_bitmap){
//    u8 *inode_bitmap = (u8*)malloc(sizeof(u8) * main_sb.s_inodes_per_group/8);
//    int start;
//    int end;
//    bool error = false;
//    int bad = 0;
//    
//    for(int i = 0; i < vdi.no_groups; i++){
//        get_inode_bitmap(i, inode_bitmap);
//        start = i * main_sb.s_inodes_per_group;
//        end = (i + 1) * main_sb.s_inodes_per_group;
//        for(int j = start; j < end; j++){
//            if(dir_inode_bitmap[j] == 1 && get_bit(inode_bitmap, j) == 0){
//                printf("File connected to inactive inode");
//            }
//                
//        }
//    }
//    //free(inode_bitmap);
//    return 0;
//}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
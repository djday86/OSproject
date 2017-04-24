#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "datatypes.h"
#include <fcntl.h>




s32 main(s32 argc, char *argv[]) {
	/*
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	===========================================================
	MAIN
	===========================================================
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	*/
        ext2_super_block backup_sb;
        BootSector boot_sector;
        int file = 0;
        int directory = 0;
        u8 *inode_bitmap;
        u8 *block_bitmap;
        int *user_inode_bitmap;
        int *user_block_bitmap;
        u32 inodes_per_block;
        inode_info* inode;


	printf("\n\n------------------------------\n");
	printf("\n      File System Check:\n\n");
	printf("------------------------------\n");
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

if(main_sb.s_state == EXT2_ERROR_FS) {
		printf("File System:  Not cleanly unmounted.  Checking for errors.\n");
	}

	if(vdi.hdr.drive_type == 1) printf("File type: Dynamic\n\n");
	else printf("File type: Static\n\n");

	printf("Total disk size: %llu bytes.\n", vdi.hdr.disk_size);

 	vdi.block_size = 1024 << main_sb.s_log_block_size;
	vdi.blocks_pg = main_sb.s_blocks_per_group;
	printf("The Superblock Magic Number: %x\n\n",main_sb.s_magic);

	if(main_sb.s_blocks_count % main_sb.s_blocks_per_group == 0) vdi.no_groups =  main_sb.s_blocks_count / 	main_sb.s_blocks_per_group;
	else vdi.no_groups = (main_sb.s_blocks_count / main_sb.s_blocks_per_group) + 1;

 	temp_block = (u8*)malloc(vdi.block_size);


	printf("Total number of block groups: %u\n\n\n",vdi.no_groups);

        inodes_per_block = vdi.block_size/sizeof(inode_info);

        inodes_per_block = vdi.block_size/sizeof(inode_info);
        user_block_bitmap = (int*)malloc(sizeof(int) * main_sb.s_blocks_count);
        user_inode_bitmap = (int*)malloc(sizeof(int) * main_sb.s_inodes_count);
        desc_table = (bg_descriptor*)malloc(sizeof(bg_descriptor) * vdi.no_groups);
        inode_bitmap = (u8*)malloc(sizeof(u8) * main_sb.s_inodes_per_group/8);
        block_bitmap = (u8*)malloc(sizeof(u8) * main_sb.s_blocks_per_group/8);
        inode = (inode_info*)malloc(sizeof(inode_info));

	superblock_check(main_sb);

        get_bg_descriptor_table(desc_table, 0);
        
        printf("Inode type %i\n",inode->i_mode);
	//bg_desc_table_check(desc_table);


	dumpExt2File();
	for(i = 0; i < vdi.no_groups; i++) {
		printf("INFO: %i\n", desc_table[i].bg_block_bitmap);
	}

        for(i = 0; i < 1; i++){
            get_block_bitmap(i, block_bitmap);
            get_inode_bitmap(i, inode_bitmap);
           // printf("Inodes per group%i\n", main_sb.s_inodes_per_group);
            //printf("Inode id %u\n",inode[0].i_uid);


            for(int j = i * main_sb.s_inodes_per_group + 1; j < 20; j++){

                get_inode(j, inode);

                if(inode->i_mode > 0x7fff && inode->i_mode < 0x9000 ){
                    printf("file found\n");
                    file++;
                }

<<<<<<< HEAD
                if(inode->i_mode > 0x3fff && inode->i_mode < 0x5000){
=======
                if(inode->i_mode == 0x4000){
>>>>>>> 74dd33eaab97ecab9197d38a68da3d318815199d
                    directory++;
                    printf("directory found\n");
                }
                if(inode->i_mode != 0)
                    user_inode_bitmap[j - 1] = 1;

                get_used_blocks(j, user_block_bitmap, inode);
                //printf("Block bitmap %i\n", block_bitmap[0]);
            }
<<<<<<< HEAD
            //compare_block_bitmap(i, user_block_bitmap, block_bitmap);
            //compare_inode_bitmap(i, user_inode_bitmap, inode_bitmap);
        }
        printf("Number of files%i\n",file);
=======
            compare_block_bitmap(i, user_block_bitmap, block_bitmap);
            compare_inode_bitmap(i, user_inode_bitmap, inode_bitmap);

        printf("Number of files%i\n", file); */
>>>>>>> 74dd33eaab97ecab9197d38a68da3d318815199d


	free(vdi.map);
	//free(desc_table);


<<<<<<< HEAD
//	if(close(fd) == -1) {
//		printf("Error.\n");
//		return EXIT_FAILURE;
//	}
=======
	if(close(vdi.fd) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}
>>>>>>> 74dd33eaab97ecab9197d38a68da3d318815199d


return EXIT_SUCCESS;
}

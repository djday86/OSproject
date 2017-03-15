#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "datatypes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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
	

int main(int argc, char *argv[]) {

	u32 fd =0;
	VDI_header disk_info;

	printf("Name of file: %s\n", argv[1]);

	fd = open(argv[1], O_RDONLY);

	if(lseek(fd,0,SEEK_SET) == -1) return EXIT_FAILURE;
	if(read(fd,&disk_info,sizeof(disk_info)) == -1) return EXIT_FAILURE;

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
	
	if(close(fd) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

	

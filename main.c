#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "datatypes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define DRIVE_SIZE 0x170
#define PAGE_SIZE 0x178
#define MBR_SIZE 448

u64 get_drive_size(u32 fd);
u32 get_page_size(u32 fd);

typedef struct { 
	
	u64 drive_size;
	u32 page_size;

} disk_information;
	




int main(int argc, char *argv[]) {

	u32 fd =0;
	disk_information disk_info;

	printf("Name of file: %s\n", argv[1]);

	fd = open(argv[1], O_RDONLY);

	disk_info.drive_size = get_drive_size(fd);
	disk_info.page_size = get_page_size(fd);

	if(close(fd) == -1) {
		printf("Error.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

u64 get_drive_size(u32 fd) {

	u8 HDDsize[8] = {0};
	u64 size = 0;

	if(lseek(fd,DRIVE_SIZE,SEEK_SET) == -1) return -1;
	if(read(fd,HDDsize,8) == -1) return -1;
	
	for(u32 i=sizeof(HDDsize);i>0;i--) {
		size = size * 256 + HDDsize[i-1];
	}

	printf("The size of the drive is: %llu bytes or %llu MiB.\n", size,size/1048576);
	return size;
	
}

u32 get_page_size(u32 fd) {

	u8 pageSize[4] = {0};
	u32 size = 0;

	if(lseek(fd,PAGE_SIZE,SEEK_SET) == -1) return -1;
	if(read(fd,pageSize,4) == -1) return -1;
	
	for(u32 i=sizeof(pageSize);i>0;i--) {
		size = size * 256 + pageSize[i-1];
	}

	printf("The size of the pages are: %u bytes.\n", size);

	return size;
	
}

	

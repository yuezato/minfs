#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <linux/types.h>

#include "minfs.h"

/*
 * mk_minfs file
 */

int main(int argc, char **argv)
{
	FILE *file;
	char buffer[MINFS_BLOCK_SIZE];
	struct minfs_super_block msb;
	struct minfs_inode root_inode;
	struct minfs_inode file_inode;
	struct minfs_dir_entry file_dentry;
	int i;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s block_device_name\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	file = fopen(argv[1], "w+");
	if (file == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	memset(&msb, 0, sizeof(struct minfs_super_block));

	msb.magic = MINFS_MAGIC;
	msb.version = 1;
	msb.imap = 0x01; // set 0-th bit for the root directory inode

	/* zero disk  */
	memset(buffer, 0,  MINFS_BLOCK_SIZE);
	for (i = 0; i < 128; i++)
		fwrite(buffer, 1, MINFS_BLOCK_SIZE, file);

	fseek(file, 0, SEEK_SET);

	/* initialize super block */
	fwrite(&msb, sizeof(msb), 1, file);

	/* initialize root inode */
	memset(&root_inode, 0, sizeof(root_inode));
	root_inode.uid = 0;
	root_inode.gid = 0;
	root_inode.mode = S_IFDIR | 0755;
	root_inode.size = 0;
	root_inode.data_block = MINFS_FIRST_DATA_BLOCK;

	fseek(file, MINFS_INODE_BLOCK * MINFS_BLOCK_SIZE, SEEK_SET);
	fwrite(&root_inode, sizeof(root_inode), 1, file);

	fclose(file);

	return 0;
}

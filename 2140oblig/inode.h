#ifndef INODE_H
#define INODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This is the inode structure as described in the
 * assignment.
 * It is mostly straightforward, but keep in mind
 * that the dynamically allocated array entries
 * contains values that you must interpret as pointers
 * when is_directory==1 and that you must interpret
 * as block numbers when is_directory==0.
 */
struct inode
{
	int            id;
	char*          name;
	char           is_directory;

	int            num_children;
	struct inode** children;

	int            filesize;
    int            num_blocks;
    size_t*        blocks;
};

/* Create a file below the inode parent. Parent must
 * be a directory. The size of the file is size_in_bytes,
 * and create_file calls the allocate_block() function
 * enough number of times to reserve blocks in the simulated
 * disk to store all of these bytes.
 * Returns a pointer to file's inodes.
 */
struct inode* create_file( struct inode* parent, char* name, int size_in_bytes );

/* Create a directory below the inode parent. Parent must
 * be a directory.
 * Returns a pointer to file's inodes.
 */
struct inode* create_dir( struct inode* parent, char* name );

/* Check all the inodes that are directly referenced by
 * the node parent. If one of them has the name "name",
 * its inode pointer is returned.
 * parent must be directory.
 */
struct inode* find_inode_by_name( struct inode* parent, char* name );
	
/* Delete the file given by its inode, if it is an inode
 * directly referenced by parent.
 * The function calls free_block for every block that is
 * referenced by this file. This removes those blocks from
 * simulate disk.
 */
int delete_file( struct inode* parent, struct inode* node );

/* Delete the directory given by its inode, if it is an inode
 * directly referenced by parent.
 * The function fails if the node is still referencing other
 * inodes.
 */
int delete_dir( struct inode* parent, struct inode* node );

/* Write the given inode root and all inodes referenced by it
 * to the file called superblock, following the oblig instructions.
 * No inodes are changed.
 */
void save_inodes( char* master_file_table, struct inode* root );

/* Read the file master_file_table and create an inode in memory
 * for every inode that is stored in the file. Set the pointers
 * between inodes correctly.
 * The file master_file_table remains unchanged.
 */
struct inode* load_inodes( char* master_file_table );

/* This function is handed out.
 *
 * It releases all dynamically allocated memory.
 * All the referenced inodes are visited
 * and their memory is released, finally also for the
 * node that is passed as a parameter.
 *
 * The simulated disk and the file master_file_table are
 * not changed.
 *
 * This function can be used to end a program after
 * save_inodes and helps you to avoid valgrind errors.
 */
void fs_shutdown( struct inode* node );

/* This function is handed out.
 *
 * It prints the node that is receives as a parameter,
 * and recurivesly all inodes that are stored below it.
 */
void debug_fs( struct inode* node );

#endif


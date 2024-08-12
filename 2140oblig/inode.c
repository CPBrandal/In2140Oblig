#include "allocation.h"
#include "inode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* The number of bytes in a block.
 * Do not change.
 */
#define BLOCKSIZE 4096

/* The lowest unused node ID.
 * Do not change.
 */
static int num_inode_ids = 0;

/* This helper function computes the number of blocks that you must allocate
 * on the simulated disk for a give file system in bytes. You don't have to use
 * it.
 * Do not change.
 */
static int blocks_needed( int bytes )
{
    int blocks = bytes / BLOCKSIZE;
    if( bytes % BLOCKSIZE != 0 )
        blocks += 1;
    return blocks;
}

/* This helper function returns a new integer value when you create a new inode.
 * This helps you do avoid inode reuse before 2^32 inodes have been created. It
 * keeps the lowest unused inode ID in the global variable num_inode_ids.
 * Make sure to update num_inode_ids when you have loaded a simulated disk.
 * Do not change.
 */
static int next_inode_id( )
{
    int retval = num_inode_ids;
    num_inode_ids += 1;
    return retval;
}

struct inode* create_file(struct inode* parent, char* name, int size_in_bytes )
{
    if(!parent){
        return NULL;
    }
    if(find_inode_by_name(parent, name)){ //Dersom det finnes en fil/katalog med samme navn returner null
        return NULL;
    }
    if(parent->is_directory == 0){ 
        return NULL;
    }

    struct inode *new = malloc(sizeof(struct inode));
    if(new == NULL){
        perror("malloc");
        printf("Not enought memory for inode");
        exit(EXIT_FAILURE);
    }

    new->id = num_inode_ids;
    new->name = strdup(name);
    if(new->name == NULL){
        perror("strdup() fail");
        printf("Not enought memory for name");
        exit(EXIT_FAILURE);
    }

    new->is_directory = 0;

    new->num_children = 0;
    new->children = NULL;

    new->filesize = size_in_bytes;
    new->num_blocks = blocks_needed(size_in_bytes);
    new->blocks = malloc(sizeof(size_t) * new->num_blocks);
    for(int i = 0; i < new->num_blocks; i++){
        new->blocks[i] = allocate_block();
    }

    // Oppdatere foreldre node //
    parent->children = realloc(parent->children, (parent->num_children + 1) * sizeof(struct inode*)); //!!!!
        if (!parent->children) {
            fprintf(stderr, "Error: Could not reallocate memory");
            exit(EXIT_FAILURE);
        }
    parent->children[parent->num_children] = new;
    parent->num_children += 1;

    next_inode_id();
    return new;
}

struct inode* create_dir( struct inode* parent, char* name )
{   
    char *root = "/";
    if(parent == NULL && strcmp(root, name) != 0){
        return NULL;
    }

    if(parent != NULL && parent->is_directory == 0){
        return NULL;
    } 

    if (find_inode_by_name(parent, name)) {
        return NULL;
    }

    struct inode *new = malloc(sizeof(struct inode));
        if(new == NULL){
        perror("malloc");
        printf("Not enought memory for inode");
        exit(EXIT_FAILURE);
    }

    new->id = num_inode_ids;
    new->name = strdup(name);
    if(new->name == NULL){
        perror("strdup() fail");
        printf("Not enought memory for name");
        exit(EXIT_FAILURE);
    }

    new->is_directory = 1;

    new->num_children = 0;
    new->children = malloc(new->num_children * sizeof(struct inode));
    
    new->filesize = 0;
    new->num_blocks = 0;
    new->blocks = NULL;

    if(parent){
        // Oppdatere foreldre node //
        parent->children = realloc(parent->children, (parent->num_children + 1) * sizeof(struct inode*));
        if (!parent->children) {
            fprintf(stderr, "Error: Could not reallocate memory");
            exit(EXIT_FAILURE);
        }
        parent->children[parent->num_children] = new;
        parent->num_children += 1;
    }

    next_inode_id();
    return new;
}

struct inode* find_inode_by_name( struct inode* parent, char* name )
{
    if(parent == NULL){
        return NULL;
    }
    if (parent->is_directory == 0) {
        // Dersom parent er en fil, returner null
        return NULL;
    }

    for (int i = 0; i < parent->num_children; ++i) {
        if (strcmp(parent->children[i]->name, name) == 0){
            return parent->children[i];
        }
    }

    // Ingen inode funnet
    return NULL;
}

static int verified_delete_in_parent( struct inode* parent, struct inode* node )
{
    if (!parent || !node) return -1;
    if(parent->num_children == 1){ //Dersom parent bare har ett barn, og dette kaller kommer fra det barnet som skal slettes
        parent->num_children = 0;
        parent->children = realloc(parent->children, (parent->num_children) * sizeof(struct inode*));
        return 0;
    }
    int index;
    for (int i = 0; i < parent->num_children; i++) {
        if (parent->children[i] == node) {
            index = i;
            break;
        }
    }
    parent->children[index] = parent->children[parent->num_children-1];
    parent->children = realloc(parent->children, (parent->num_children - 1) * sizeof(struct inode*));
    parent->num_children--;
    if (!parent->children) {
        fprintf(stderr, "Error: Could not reallocate memory");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Returnerer 0 hvis node er i parent, ellers -1
int is_node_in_parent( struct inode* parent, struct inode* node )
{
    if(parent == NULL || node == NULL){
        return -1;
    }
    if(find_inode_by_name(parent, node->name) == NULL){
        return -1;
    } else {
        return 0;
    }
}

int delete_file( struct inode* parent, struct inode* node )
{
    if(node == NULL){
        return -1;
    }
    if(node->is_directory == 1) {
        return -1;
    }
    if(parent == NULL){
        return -1;
    }
    if(!find_inode_by_name(parent, node->name)){
        return -1;
    }

    verified_delete_in_parent(parent, node);
    
    free(node->name);
/*     for(int i = 0; i < node->num_blocks; i++){
        free_block(node->blocks[i]);
    } */
    free(node->blocks);
    free(node);
    return 0;
}

int delete_dir( struct inode* parent, struct inode* node )
{
    if(node == NULL){
        return -1;
    }
    
    if(!parent){
        if(strcmp(node->name, "/") == 0){
            if(node->num_children == 0){ // Dersom vi forsøker å slette rotnode og den har 0 barn --
                free(node->name);
                free(node->children);
                free(node);
                return 0;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    if(node->num_children > 0){
        return -1;
    }

    if(parent->is_directory == 0){
        return -1;
    }

    if(!find_inode_by_name(parent, node->name)){
        return -1;
    }

    verified_delete_in_parent(parent, node);

    free(node->name);
    free(node);
    return 0;
}

struct inode* make_inode(FILE* file){
    struct inode *node = malloc(sizeof(struct inode));
    int nl;

    if (node == NULL){
        fprintf(stderr, "Error, Could not allocate memory");
        exit(EXIT_FAILURE);
    }

    fread(&node->id, sizeof(int), 1, file);
    //printf("ID: %d\n", node->id);
    fread(&nl, sizeof(int), 1, file);
    //printf("STRING LENGTH: %d\n", nl);
    node->name = malloc(sizeof(char) * nl);
    fread(node->name, sizeof(char), nl, file);
    fread(&node->is_directory, sizeof(char), 1, file);

   if (node->is_directory) {
        fread(&node->num_children, 1, sizeof(int), file);
        node->children = malloc(sizeof(struct inode*) * node->num_children);
        if (node->num_children != 0 && node->children == NULL) {
            fprintf(stderr, "Could not allocate memory for node->children");
            exit(EXIT_FAILURE);
        }

        //printf("Num_children: %d\n", node->num_children);
        node->filesize = 0;
        node->num_blocks = 0;
        node->blocks = NULL;
        fseek(file, 8 * node->num_children, SEEK_CUR);
        for (int i = 0; i < node->num_children; i++) {
            struct inode* child = make_inode(file);
            // Add the child to the node's children array
            node->children[i] = child;
        }
    } else {
        node->num_children = 0;
        node->children = NULL;
        fread(&node->filesize, 1, sizeof(int), file);
        fread(&node->num_blocks, 1, sizeof(int), file);
        node->blocks = malloc(sizeof(size_t) * node->num_blocks);
        if (node->blocks == NULL) {
            fprintf(stderr, "Could not allocate memory for node->blocks");
            exit(EXIT_FAILURE);
        }
/*         for(int i = 0; i < node->num_blocks; i++){
            node->blocks[i] = allocate_block();
        }         
        fseek(file, sizeof(size_t) * node->num_blocks, SEEK_CUR);
        */
        fread(node->blocks, sizeof(size_t), node->num_blocks, file);
    }
    return node;
}

struct inode* load_inodes( char* master_file_table )
{
    FILE* file = fopen(master_file_table, "rb");
    if (!file) {
        perror("fopen");
        printf("%s\n", master_file_table);
        exit(EXIT_FAILURE);
    }

    struct inode* root = make_inode(file);

    fclose(file);
    return root;
}

/* The function save_inode is a recursive functions that is
 * called by save_inodes to store a single inode on disk,
 * and call itself recursively for every child if the node
 * itself is a directory.
 */
static void save_inode( FILE* file, struct inode* node )
{
    if( !node ) return;

    int len = strlen( node->name ) + 1;

    fwrite( &node->id, 1, sizeof(int), file );
    fwrite( &len, 1, sizeof(int), file );
    fwrite( node->name, 1, len, file );
    fwrite( &node->is_directory, 1, sizeof(char), file );
    if( node->is_directory )
    {
        fwrite( &node->num_children, 1, sizeof(int), file );
        for( int i=0; i<node->num_children; i++ )
        {
            struct inode* child = node->children[i];
            size_t id = child->id;
            fwrite( &id, 1, sizeof(size_t), file );
        }

        for( int i=0; i<node->num_children; i++ )
        {
            struct inode* child = node->children[i];
            save_inode( file, child );
        }
    }
    else
    {
        fwrite( &node->filesize, 1, sizeof(int), file );
        fwrite( &node->num_blocks, 1, sizeof(int), file );
        for( int i=0; i<node->num_blocks; i++ )
        {
            fwrite( &node->blocks[i], 1, sizeof(size_t), file );
        }
    }
}

void save_inodes( char* master_file_table, struct inode* root )
{
    if( root == NULL )
    {
        fprintf( stderr, "root inode is NULL\n" );
        return;
    }

    FILE* file = fopen( master_file_table, "w" );
    if( !file )
    {
        fprintf( stderr, "Failed to open file %s\n", master_file_table );
        return;
    }

    save_inode( file, root );

    fclose( file );
}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

/* Do not change.
 */
void debug_fs( struct inode* node )
{
    if( node == NULL ) return;
    for( int i=0; i<indent; i++ )
        printf("  ");

    if( node->is_directory )
    {
        printf("%s (id %d)\n", node->name, node->id );
        indent++;
        for( int i=0; i<node->num_children; i++ )
        {
            struct inode* child = (struct inode*)node->children[i];
            debug_fs( child );
        }
        indent--;
    }
    else
    {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize );
        for( int i=0; i<node->num_blocks; i++ )
        {
            printf("%d ", (int)node->blocks[i]);
        }
        printf(")\n");
    }
}

/* Do not change.
 */
void fs_shutdown( struct inode* inode )
{
    if( !inode ) return;

    if( inode->is_directory )
    {
        for( int i=0; i<inode->num_children; i++ )
        {
            fs_shutdown( inode->children[i] );
        }
    }

    if( inode->name )     free( inode->name );
    if( inode->children ) free( inode->children );
    if( inode->blocks )   free( inode->blocks );
    free( inode );
}


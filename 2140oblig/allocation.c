#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h> 

#include <errno.h>

#define NUM_BLOCKS 50

/* The name of the file that contains our block allocation table
 * that simulates the used disk.
 * We make the variable static to hide it from other C files.
 */
static char* file_name = NULL;

void set_block_allocation_table_name( char* str )
{
    if( file_name != NULL )
    {
        fprintf( stderr, "Cannot set %s as block allocation table name.\n"
                         "The name of the block_allocation_table has already been set to %s\n",
                         str, file_name );
        exit( -1 );
    }

    file_name = strdup( str );
}

void release_block_allocation_table_name( )
{
    if( file_name )
    {
        free( file_name );
    }
}

static char* read_table( )
{
    if( file_name == NULL )
    {
        fprintf( stderr, "Failed to set the name of the block allocation table file.\n" );
        exit( -1 );
    }

    char* table = malloc( NUM_BLOCKS );
    if( table == NULL )
    {
        fprintf( stderr, "Failed to allocate %d bytes\n", NUM_BLOCKS );
        return NULL;
    }

    FILE* f = fopen( file_name, "r" );
    if( !f )
    {
        fprintf( stderr, "Failed to open file %s for reading\n", file_name );
        perror("reason:");
        return NULL;
    }

    int num_read = fread( table, 1, NUM_BLOCKS, f );
    if( num_read != NUM_BLOCKS )
    {
        fprintf( stderr, "Failed to load %d block entries from disk\n", NUM_BLOCKS );
        perror("reason:");
        fclose(f);
        return NULL;
    }
    fclose( f );

    return table;
}

static int write_table( char* table )
{
    if( file_name == NULL )
    {
        fprintf( stderr, "Failed to set the name of the block allocation table file.\n" );
        exit( -1 );
    }

    FILE* f = fopen( file_name, "w" );
    if( !f )
    {
        fprintf( stderr, "Failed to open file %s for writing\n", file_name );
        perror("reason:");
        return -1;
    }
    int num = fwrite( table, 1, NUM_BLOCKS, f );
    if( num != NUM_BLOCKS )
    {
        fprintf( stderr, "Failed to write %d bytes to %s\n", NUM_BLOCKS, file_name);
        fprintf( stderr, "fwrite returned %d\n", num );
        perror("reason:");
        return -1;
    }
    fclose( f );
    return 0;
}

int format_disk()
{
    if( file_name == NULL )
    {
        fprintf( stderr, "Failed to set the name of the block allocation table file.\n" );
        exit( -1 );
    }

    int error = unlink( file_name );

    if( error == 0 || errno == ENOENT )
    {
        /* We want to set all NUM_BLOCK chars to 0, convenient to use
         * calloc.
         */
        char* table = calloc( NUM_BLOCKS, 1 );
        if( table == NULL )
        {
            fprintf( stderr, "Failed to allocate %d bytes\n", NUM_BLOCKS );
            return -1;
        }

        int retval = write_table( table );
        free( table );
        return retval;
    }

    fprintf( stderr, "Failed to remove existing file %s, code %d\n", file_name, errno );
    perror("reason:");
    return -1;
}

int allocate_block( )
{
    char* table = read_table( );
    if( table == NULL )
    {
        return -1;
    }

    for( int i=0; i<NUM_BLOCKS; i++ )
    {
        if( table[i] == 0 )
        {
            /* Found an unused block */
            table[i] = 1;
            write_table( table );

            free( table );
            return i;
        }
    }

    free( table );
    return -1;
}

int free_block(int block)
{
    if( block < 0 || block >= NUM_BLOCKS )
    {
        fprintf( stderr, "Block number %d is not valid\n", block );
        return -1;
    }

    char* table = read_table();
    if( table == NULL )
    {
        return -1;
    }

    if( table[block] != 1 )
    {
        fprintf( stderr, "Block %d was not allocated\n", block );
        free( table );
        return -1;
    }

    table[block] = 0;

    write_table( table );
    free( table );

    return 0;
}

void debug_disk( )
{
    char* table = read_table( );
    printf("Disk:\n");
    for( int i=0; i<NUM_BLOCKS; i++ )
        printf("%d", table[i] );
    printf("\n");
    free(table);
}


#include "inode.h"
#include "allocation.h"

#include <stdio.h>

static int try_delete_dir( struct inode* parent, struct inode* dir )
{
    printf("Trying to delete directory %s", dir->name );
    if( delete_dir( parent, dir ) >= 0 )
    {
        printf(" - deleted\n");
        return 1;
    }
    else
    {
        printf(" - failed\n");
        return 0;
    }
}

static int try_delete_file( struct inode* parent, struct inode* file )
{
    printf("Trying to delete file %s", file->name );
    if( delete_file( parent, file ) >= 0 )
    {
        printf(" - deleted\n");
        return 1;
    }
    else
    {
        printf(" - failed\n");
        return 0;
    }
}

int main( int argc, char* argv[] )
{
    if( argc != 3 )
    {
        fprintf( stderr, "This programs loads the master file table (MFT) and the block allocation table (BAT)\n"
                         "for a simulated disk from file.\n"
                         "First it prints the content of the MFT.\n"
                         "Second it prints the allocation table of the BAT.\n"
                         "Finally, it goes through a list of directories and files and attempts to delete them.\n"
                         "\n"
                         "Deletions fail when the file or directory is not found.\n"
                         "Deletions fail also when a directory to be deleted is not empty.\n"
                         "\n"
                         "Usage: %s MFT BAT\n"
                         "       where\n"
                         "       MFT is the name of the master file table\n"
                         "       BAT is the name of the block allocation table\n"
                         , argv[0] );
        exit( -1 );
    }

    char* mft_name = argv[1];
    char* bat_name = argv[2];

    set_block_allocation_table_name( bat_name );

    printf("===================================\n");
    printf("= Load all inodes from the file   =\n");
    printf("= master_file_table               =\n");
    printf("===================================\n");
    struct inode* root = load_inodes( argv[1] );
    debug_fs( root );
    debug_disk();

    printf("\n\n\n");
    printf("================================================\n");
    printf("= Trying to delete some directories and files. =\n");
    printf("================================================\n");

    struct inode* kernel_node = find_inode_by_name( root, "kernel" );
    if( kernel_node )
    {
        try_delete_file( root, kernel_node );
    }

    struct inode* var_dir = NULL;
    struct inode* log_dir = NULL;

    var_dir = find_inode_by_name( root, "var" );
    if( var_dir ) log_dir = find_inode_by_name( var_dir, "log" );
    if( log_dir )
    {
        try_delete_dir( var_dir, log_dir );
        struct inode* node = NULL;

        node = find_inode_by_name( log_dir, "message" );
        if( node ) try_delete_file( log_dir, node );
        node = find_inode_by_name( log_dir, "warn" );
        if( node ) try_delete_file( log_dir, node );
        node = find_inode_by_name( log_dir, "fail" );
        if( node ) try_delete_file( log_dir, node );

        try_delete_dir( var_dir, log_dir );
    }

    struct inode* dir = NULL;

    dir = find_inode_by_name( root, "share" );
    if( dir ) dir = find_inode_by_name( dir, "man" );
    if( dir )
    {
        struct inode* node = find_inode_by_name( dir, "read.2" );
        if( node ) try_delete_file( dir, node );
    }

    dir = find_inode_by_name( root, "etc" );
    if( dir )
    {
        struct inode* node = find_inode_by_name( dir, "hosts" );
        if( node ) try_delete_file( dir, node );
    }

    dir = find_inode_by_name( root, "etc" );
    if( dir )
    {
        struct inode* node = find_inode_by_name( dir, "host.conf" );
        if( node ) try_delete_file( dir, node );
    }

    dir = find_inode_by_name( root, "etc" );
    if( dir )
    {
        struct inode* http_dir = find_inode_by_name( dir, "httpd" );
        if( http_dir )
        {
            try_delete_dir( dir, http_dir );
            http_dir = find_inode_by_name( dir, "httpd" );
            if( http_dir )
            {
                struct inode* node = find_inode_by_name( http_dir, "conf" );
                if( node )
                {
                    try_delete_file( http_dir, node );
                    try_delete_dir( dir, http_dir );
                }
            }
        }
    }

    dir = root;
    dir = find_inode_by_name( dir, "home" );
    if( dir )
    {
        struct inode* udir = find_inode_by_name( dir, "user" );
        if( udir )
        {
            try_delete_dir( dir, udir );
            udir = find_inode_by_name( dir, "user" );
            struct inode* dl_dir = find_inode_by_name( udir, "Download" );
            if( dl_dir )
            {
                try_delete_dir( udir, dl_dir );
                dl_dir = find_inode_by_name( udir, "Download" );
                if( dl_dir )
                {
                    struct inode* ob_node = find_inode_by_name( dl_dir, "oblig2" );
                    if( ob_node )
                    {
                        try_delete_file( dl_dir, ob_node );
                        try_delete_dir( udir, dl_dir );
                        try_delete_dir( dir, udir );
                    }
                }
            }
        }
    }

    debug_fs( root );
    debug_disk();

    save_inodes( mft_name, root );

    fs_shutdown( root );

    release_block_allocation_table_name( );

    printf("\n\n\n");
}


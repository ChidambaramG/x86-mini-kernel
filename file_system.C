/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/* Default constructor for Inode, initializes members to default values */
Inode::Inode() : fs(NULL), id(-1), is_inode_free(true), file_size(0) {}
/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() 
    : disk(nullptr), 
      filesystem_size(0), 
      free_block_count(SimpleDisk::BLOCK_SIZE / sizeof(unsigned char)), 
      inode_counter(0) {

    free_blocks = new unsigned char[free_block_count];
    for (unsigned int i = 0; i < free_block_count; i++) {
        free_blocks[i] = 'F';
    }

    inodes = new Inode[MAX_INODES];

    Console::puts("File system: Free block list and inode array initialized.");
}

/* Destructor for FileSystem class, writes data to disk before unmounting */
FileSystem::~FileSystem() {
    Console::puts("File system: Unmounting and saving data.");

    disk->write(0, free_blocks);
    unsigned char* tmp_inode_ref = reinterpret_cast<unsigned char*>(inodes);
    disk->write(1, tmp_inode_ref);

    Console::puts("File system: Successfully unmounted.");
    delete[] free_blocks;
    delete[] inodes;
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk* _disk) {
    Console::puts("File system: Mounting from disk.");

    disk = _disk;
    disk->read(0, free_blocks);

    unsigned char* tmp_inode_ref = reinterpret_cast<unsigned char*>(inodes);
    disk->read(1, tmp_inode_ref);

    inode_counter = 0;
    for (unsigned int i = 0; i < MAX_INODES; i++) {
        if (!inodes[i].is_inode_free) {
            inode_counter++;
        }
    }

    Console::puts("File system: Mounted successfully.");
    return true;
}

/* Formats the disk, initializes inode list and free block list */
bool FileSystem::Format(SimpleDisk* _disk, unsigned int _size) {
    Console::puts("File system: Formatting disk.");

    unsigned int n_free_blks = _size / SimpleDisk::BLOCK_SIZE;
    unsigned char* free_blks_arr = new unsigned char[n_free_blks];
    free_blks_arr[0] = free_blks_arr[1] = 'U'; // Reserve blocks for metadata

    for (unsigned int i = 2; i < n_free_blks; i++) {
        free_blks_arr[i] = 'F';
    }

    _disk->write(0, free_blks_arr);

    Inode* tmp_inodes_ref = new Inode[MAX_INODES];
    unsigned char* tmp_inode_ref = reinterpret_cast<unsigned char*>(tmp_inodes_ref);
    _disk->write(1, tmp_inode_ref);

    delete[] free_blks_arr;
    delete[] tmp_inodes_ref;

    Console::puts("File system: Disk formatted successfully.");
    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("File system: Looking up file with ID = "); Console::puti(_file_id); Console::puts("\n");
    unsigned int i;
    for (i = 0; i < inode_counter; i++) {
        if (inodes[i].id == _file_id) {
            return &inodes[i];
        }
    }
    Console::puts("File system: File not found with ID = "); Console::puti(_file_id); Console::puts("\n");
    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("File system: Creating file with ID = ");
    Console::puti(_file_id);
    Console::puts("\n");

    for (unsigned int i = 0; i < inode_counter; i++) {
        if (inodes[i].id == _file_id) {
            Console::puts("File system: Error - File already exists with ID = ");
            Console::puti(_file_id);
            Console::puts("\n");
            assert(false);
        }
    }

    int free_inode_indx = -1;
    for (unsigned int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].is_inode_free) {
            free_inode_indx = i;
            break;
        }
    }

    int free_blk_indx = -1;
    for (unsigned int i = 0; i < free_block_count; i++) {
        if (free_blocks[i] == 'F') {
            free_blk_indx = i;
            break;
        }
    }

    assert(free_inode_indx != -1 && free_blk_indx != -1);

    inodes[free_inode_indx].is_inode_free = false;
    inodes[free_inode_indx].fs = this;
    inodes[free_inode_indx].id = _file_id;
    inodes[free_inode_indx].block_number = free_blk_indx;

    free_blocks[free_blk_indx] = 'U';

    Console::puts("File system: File created successfully.");
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("File system: Deleting file with ID = "); Console::puti(_file_id); Console::puts("\n");
    bool file_exists = false;
    unsigned int indx = 0;
    while (indx < MAX_INODES) {
        if (inodes[indx].id == _file_id) {
            file_exists = true;
            break;
        }
        indx++;
    }
    assert(file_exists);
    int blk_number = inodes[indx].block_number;
    free_blocks[blk_number] = 'F';
    inodes[indx].is_inode_free = true;
    inodes[indx].file_size = 0;
    Console::puts("File system: File deleted successfully.");
    return true;
}

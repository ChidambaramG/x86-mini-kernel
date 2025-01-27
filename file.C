/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem* _fs, int _id) 
    : file_system(_fs), file_id(_id), curr(0) {

    Console::puts("Opening file.\n");

    bool file_block_found = false;
    for (unsigned int i = 0; i < file_system->MAX_INODES; ++i) {
        if (file_system->inodes[i].id == file_id) {
            inode_index = i;
            block_number = file_system->inodes[i].block_number;
            file_size = file_system->inodes[i].file_size;
            file_block_found = true;
            break;
        }
    }

    assert(file_block_found); // Ensure the file block is found
}

File::~File() {
    Console::puts("Closing file.\n");
    // Write cached data to disk
    file_system->disk->write(block_number, block_cache);
    // Update inode in the inode list
    file_system->inodes[inode_index].file_size = file_size;
    unsigned char* tmp_inode_ref = reinterpret_cast<unsigned char*>(file_system->inodes);
    file_system->disk->write(1, tmp_inode_ref);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

/* Read data from file */
int File::Read(unsigned int _n, char* _buf) {
    Console::puts("Reading from file\n");
    unsigned int char_count = 0;
    for (unsigned int index = curr; index < file_size && char_count < _n; ++index) {
        _buf[char_count++] = block_cache[index];
    }
    curr += char_count;
    Console::puts("Reading from file complete.\n");
    return char_count;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
   int char_count = 0;
	int end_indx = curr + _n;
	while(curr < end_indx){
		if(curr == SimpleDisk::BLOCK_SIZE){
			Console::puts("EOF reached while writing.\n");
			break;
		}
		block_cache[curr++]=_buf[char_count++];
		file_size++;
	}

	Console::puts("writing to file complete.\n");
	return char_count;
}

void File::Reset() {
    Console::puts("resetting file\n");
    curr=0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    return (curr<file_size);
}

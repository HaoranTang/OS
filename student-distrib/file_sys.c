#include "file_sys.h"


bootblock_t* boot_blk = NULL;

/*  file_sys_init
 *  Description: init the file system driver
 *  input: start -- the start address for boot block
 *  output: none
 *  side effect: make the boot block ptr
*/
void file_sys_init(uint32_t start) {
    boot_blk = (bootblock_t*)start;
}

/*  read_dentry_by_name
 *  Description: write the dentry with the wanted name into the passed argument
 *  input: fname -- the searching file name
 *         dentry -- the dentry to write into
 *  output: 0 if successful, 0 otherwise
 *  side effect: write into dentry
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    if (boot_blk == NULL) return -1; // check whether initialized
    if (strlen((int8_t*)fname) > MAX_FILE_NAME_LEN) return -1;  // check name length
    if (fname == NULL) return -1;   // check if ptr valid
    int i;
    for (i = 0; i < MAX_FILE_NUM; i++) {
        dentry_t cur_dentry = boot_blk->files[i];
        // check every dentry by its name
        //if (strlen((int8_t*)fname) == strlen(cur_dentry.file_name)) {
            if (strncmp((int8_t*)fname, cur_dentry.file_name, MAX_FILE_NAME_LEN) == 0) {
                *dentry = cur_dentry;
                return 0;
            }
        //}
    }
    return -1; // search fail
}

/*  read_dentry_by_index
 *  Description: write the dentry with the wanted index into the passed argument
 *  input: index -- the index of the dentry
 *         dentry -- the dentry to write into
 *  output: 0 if successful, 0 otherwise
 *  side effect: write into dentry
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if (boot_blk == NULL) return -1; // check whether initialized
    if (index >= boot_blk->num_dentry) return -1; // check if the index within range
    *dentry = boot_blk->files[index];
    return 0;
}

/*  read_data
 *  Description: read data from inode, once at a time
 *  input: inode_idx -- inode index number
 *         offset -- start position of reading
 *         buf -- write data into buf
 *         length -- num of bytes to read
 *  output: number of bytes read and placed in the buffer
 *  side effect: write into buf
*/
int32_t read_data(uint32_t inode_idx, uint32_t offset, char* buf, uint32_t length) {
    if (boot_blk == NULL || buf == NULL) return -1;  	// check whether initialized and ptr valid		
    if (inode_idx >= boot_blk->num_inode) return -1;   // check index			
    inode_t* cur_inode = (inode_t*) boot_blk + inode_idx + 1;  // get actual inode
    if (offset >= cur_inode->file_size)     // if offset larger than file size, read nothing 
    	return 0;
    if (offset + length > cur_inode->file_size)  // if read too much, clip the length
    	length = cur_inode->file_size - offset;
    uint32_t first_data_blk = offset / BLOCK_SIZE;  // first data blk to read
    uint32_t last_data_blk = (offset + length) / BLOCK_SIZE; // last data blk to read
    uint32_t bytes_read = 0;    // return value
    int i;
    // go through every data blk
    for (i = first_data_blk; i <= last_data_blk; i++) {
        // get the actual data blk
        data_block_t* cur_data_blk = (data_block_t*) boot_blk + boot_blk->num_inode + cur_inode->data_blks[i] + 1;
        // Calculate the beginning and end
        uint32_t block_start_pos = 0;
        uint32_t block_end_pos = BLOCK_SIZE;
        if (i == first_data_blk) {
            // read from offset for the first blk
            block_start_pos = offset - i * BLOCK_SIZE;
        }
        if (i == last_data_blk) {
            // read until offset + length for the last blk
            block_end_pos = offset + length - i * BLOCK_SIZE;
        }
        //  copy data into buf
        memcpy((uint8_t*) buf + bytes_read, (uint8_t*) cur_data_blk + block_start_pos, block_end_pos - block_start_pos);
        bytes_read += block_end_pos - block_start_pos;    // update return value
    }
    return bytes_read;
}

/*  file_open
 *  Description: open the file
 *  input: filename -- name of the file
 *  output: 0 if success, -1 otherwise
 *  side effect: none
*/
int32_t file_open(const uint8_t* filename) {
    // if (boot_blk == NULL) return -1;
    // dentry_t dentry_;
    // if (read_dentry_by_name(filename, &dentry_) == 0) {
    //     if (dentry_.file_type == REGULAR_FILE_TYPE)
    //         *inode = dentry_.inode_num;
    //         return 0;
    // }
    return 0;
}

/*  file_close
 *  Description: close the file
 *  input: fd number
 *  output: 0 if success, -1 otherwise
 *  side effect: none
*/
int32_t file_close(int32_t fd) {
    return 0;
}

/*  file_write
 *  Description: write to file
 *  input: fd -- fd number
 *         buf -- the buf write into
 *         nBytes -- num of bytes wirte into
 *  output: always -1 since its a read only file system
 *  side effect: none
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nBytes) {
    return -1;
}

/*  file_read
 *  Description: read the file
 *  input: fd -- fd number
 *         buf -- the buf wirte into
 *         nBytes -- num of bytes wirte into
 *  output: #bytes read if success, -1 otherwise
 *  side effect: none
*/
int32_t file_read(int32_t fd, void* buf, int32_t nBytes) {
    pcb_t* cur_pcb = get_cur_pcb();
    uint32_t inode_idx;
    uint32_t offset;
    // get current inode and offset from fd
    inode_idx = (uint32_t)cur_pcb->fda[fd].inode;
    offset = (uint32_t)cur_pcb->fda[fd].file_position;
    int numBytes = read_data(inode_idx, offset, (int8_t*)buf, (uint32_t)nBytes);
    cur_pcb->fda[fd].file_position += numBytes; // update position
    return numBytes;
}


/*  read_dir
 *  Description: read a directory
 *  input: offset -- the offset to read from
 *         buf -- the buf wirte into
 *         nBytes -- num of bytes wirte into
 *  output: length of bytes read if success, -1 otherwise
 *  side effect: none
*/
int32_t read_dir(uint32_t offset, char* buf, uint32_t length) {
    if (boot_blk == NULL) return -1;
    if (offset >= MAX_FILE_NUM) return -1;
    if (buf == NULL) return -1;
    if (length > MAX_FILE_NAME_LEN) length = MAX_FILE_NAME_LEN;
    dentry_t* dentry = &(boot_blk->files[offset]);
    if(length > strlen(dentry->file_name)) length = strlen(dentry->file_name);
    memcpy((uint8_t*) buf, (uint8_t*) dentry->file_name, length);
    return length;
}

/*  directory_open
 *  Description: open the file
 *  input: filename -- name of the file
 *  output: 0 if success, -1 otherwise
 *  side effect: set the inode num for the dentry
*/
int32_t directory_open(const uint8_t* filename) {
    // dentry_t dentry;
    // if (boot_blk == NULL) return -1;    // check init
    // if (read_dentry_by_name(filename, &dentry) == 0) {
    //     if (dentry.file_type != DIR_FILE_TYPE)
    //     *inode = dentry.inode_num;
    //     return 0;
    // }
    return 0;
}

/*  directory_close
 *  Description: close the file
 *  input: fd number
 *  output: 0 if success, -1 otherwise
 *  side effect: none
*/
int32_t directory_close(int32_t fd) {
    return 0;
}

/*  directory_write
 *  Description: write to file
 *  input: fd -- fd number
 *         buf -- the buf write into
 *         nBytes -- num of bytes wirte into
 *  output: always -1 since its a read only file system
 *  side effect: none
*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nBytes) {
    return -1;
}

/*  directory_read
 *  Description: read the file
 *  input: fd -- fd number
 *         buf -- the buf wirte into
 *         nBytes -- num of bytes wirte into
 *  output: #bytes read if success, -1 otherwise
 *  side effect: write into buf
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nBytes) {
    /* get current file desc */
    file_des_t cur_file_des = get_cur_pcb()->fda[fd];
    if (cur_file_des.flags == 0) 
        return -1;
    dentry_t dentry_;
    // fix for CP4
    if (get_cur_pcb()->fda[fd].file_position > boot_blk->num_dentry) {
        return 0;
    }
    // if cant find the file by index, return 0
    if (read_dentry_by_index((uint32_t)cur_file_des.file_position, &dentry_) == -1)
        return 0;
    
    get_cur_pcb()->fda[fd].file_position++;
    
    int result;
    // count #Bytes
    for (result = 0; result < MAX_FILE_NAME_LEN; result++) {
        if (dentry_.file_name == 0)
            break;
    }
    // copy into buf
    memcpy(buf, dentry_.file_name, MAX_FILE_NAME_LEN);
    return result;
}






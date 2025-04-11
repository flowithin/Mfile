#include "fs_server.h"
#include <cstdint>
#include <cstring>
#include "fs_param.h"


int main(int argc, char** argv){
  /*uint32_t bl[FS_MAXFILEBLOCKS]={};*/
  /*fs_inode ot = {'d', "jtw", 2, {}};*/
  fs_inode root;

  //write directory info to block 1
  fs_direntry root_dir[8] = {}; 
  fs_direntry root_dir2[8] = {}; 
  root_dir[0] = {"jtw", 2};
  disk_writeblock(1, &root_dir);
  /*disk_writeblock(2, &root_dir2);*/
  //change root to enclose the block 1
  disk_readblock(0, &root);
  std::cout << root.size << " " << root.owner<< '\n';
  root = fs_inode{'d', "", 1, {}};//owner field not filled
  root.blocks[0] = 1;
  root.blocks[1] = 2;
  disk_writeblock(0, &root);
  strcpy(root.owner, "xxxx");
  root.blocks[0] = 4;
  disk_writeblock(2, &root);
  root.blocks[0] = 6;
  root.type = 'f';
  fs_direntry jtw[8] = {};
  jtw[0] = {"abc.txt", 5};
  disk_writeblock(4, &jtw);
  disk_writeblock(5, &root);
  char content[FS_BLOCKSIZE] = {};
  strcpy(content, "dfkdjf");
  disk_writeblock(6, content);
 
  /*in[0].name = "x";*/
  /*strcpy(in[0].name, "x");*/
  /*in[0].inode_block = 1;*/
  /*disk_writeblock(0, &in);*/
}

#include "disk.h"
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "fs_client.h"
#include "fs_param.h"
#include "fs_server.h"
#include <iostream>
#include <iterator>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>




Lock::Lock(){}
Lock Disk_Server::lock;
/*
 * @brief fill in the non free blocks
 * */
void free_list_init(int block, std::vector<int>& v){
  fs_direntry inv[8];
  fs_inode in;
  v[block] = 1;
  disk_readblock(block, &in);
  for(size_t i = 0; i < in.size; i++){
    v[in.blocks[i]] = 1;
    if(in.type == 'f') continue;
    disk_readblock(in.blocks[i], inv);
    for(int j = 0; j < 8; j++){
      if(inv[j].inode_block != 0)
        free_list_init(inv[j].inode_block, v);
    }
  }
}
std::vector<int> Disk_Server::free_list = [](){
  std::vector<int> v(4096);
  //free list traverse
  free_list_init(0, v);
  /*for(auto v_ : v)*/
  /*std::cout << v_ << ' ';*/
  return v;
}();
Disk_Server::Disk_Server(int fd):Server(fd){}

/*
 * @brief access the free_list with lock
 * @param size returns the size of the free spot on disk
 * @param idx is the block you want to access
 * @param block is the first free block in disk
 * @param toggle is for changing the block idx to occupied (==1)
 * @note should pass int block!=0 if don't want to change anything
 * */
int free_list_access(int idx, int& size, int& block, bool toggle = false){
  boost::lock_guard<boost::mutex> _lock(Disk_Server::lock.mem_mt);
  if(toggle)
    Disk_Server::free_list[idx] = Disk_Server::free_list[idx] == 0 ? 1 : 0;
  int _size = 0;
  for(size_t i = 0; i < Disk_Server::free_list.size(); i++){
    if(!Disk_Server::free_list[i]){
      //free block
      if(block == 0){
        block = i;
        Disk_Server::free_list[i] = 1;
        continue;
      }
      _size++;
    }
  }
  size = _size;
  return Disk_Server::free_list[idx];
}
/*
 * @brief recursively access the path
 * @requirement size of the vector > 2
 * @return a lock to final entry
 * @return also the block will be inode block number of the last entry on the path
 * @note hand in hand lock might fail due to misunderstanding of compiler
 * */

/*
 *
 * @brief access inode with checks, in will contain the read info
 * */
void Disk_Server::access_inode(int block, fs_inode& in, char& type){
  disk_readblock(block, &in);
  std::stringstream ss;
  ss << "expect " << type << " found " << in.type << '\n';
  if(type != 'a' && in.type != type)
    throw NofileErr(ss.str());
  if(strcmp(in.owner, "") != 0 && in.owner != request.usr)
    throw NofileErr("access denied");
  type = in.type;
}



/*
 * @brief find a file in directory 
 * @param in_block: one of the disk block of the dir
 * @param block: inode block of the found entry (if not found, this is the first not occupied entry in current dir block)
 * @param inv: fs_direntry inv[8] used to type cast read value
 * @param num_entry: number of entry in the block of the dir
 * @param i: the position of the found entry in the block
 * @return true if found false otw
 * @note that this function may need simplify
 * */

bool dir_find(const int in_block, int& block, std::string target, int& num_entry, fs_direntry* _inv, int& i){
  bool found = false;
  int free_block = block;
  num_entry = 0;
  fs_direntry inv[8];
  disk_readblock(in_block, inv);
  /*std::cout << "block: " << block << '\n';*/
  std::cout << "free_block: " << free_block << '\n';
  int j = 0;
  for(; j < 8; j++){
    *(_inv + j) = *(inv + j);
    if(inv[j].inode_block == 0)
    {
      /*std::cout << "num_entry: " << num_entry << '\n' << "i: " << i << '\n';*/
      //update free_block
      free_block = std::min(j, free_block);
      continue;
    }
    if(inv[j].name == target){
      i = j;
      block = inv[j].inode_block;
      found = true;
    }
    num_entry++;
  }
  if(!found)
    block = free_block;
  return found;
}


lock_var Disk_Server::_access(shared_lock curr_lk, int i, int& block){
  //base case
  const std::string curr_dir = request.path[i];//safe
  const std::string next_dir = request.path[i + 1];//safe
  std::cout << "next_dir: " << next_dir << '\n';
  //base case
  fs_inode curr_node;
  //read inode
  char _type_ = 'd';
  access_inode(block, curr_node, _type_);
  //traverse direntry array
  for(size_t i = 0; i < curr_node.size; i++){
    int b = curr_node.blocks[i], _size_, _i_;
    fs_direntry inv[8];
    if(dir_find(b, block, next_dir, _size_, inv, _i_))
      goto found;
  }
  //not found:
  throw NofileErr(next_dir + "not found!\n");
found:
  int remnant = 2;
  if(request.rtype == Rtype::DELETE || request.rtype == Rtype::CREATE)
    remnant = 3;
  if(i == request.path.size() - remnant){
    if(request.rtype == Rtype::WRITE || request.rtype == Rtype::CREATE)
    {
      return unique_lock(lock.find_lock(next_dir));
    } else {
      return shared_lock(lock.find_lock(next_dir));
    }
  }
  /*std::cout << "next_dir: " << next_dir << '\n';*/
  return _access(shared_lock(lock.find_lock(next_dir)), i+1, block);
  // NOTE: will the fs always well formed?
  //assume well formed now
}







//inode insert block
void iiblock(fs_inode& inode){
  int size = 0, block = 0;
  free_list_access(0, size, block);
  if(size == 0)
    throw NofileErr("no free block");
  if(inode.size == FS_MAXFILEBLOCKS)
    throw NofileErr("maximum file size");
  inode.blocks[inode.size++] = block;
}
void Disk_Server::_readwrite(){
  int block=0;
  lock_var sl;
  if(request.path.size() > 1)
    sl = _access(shared_lock(lock.find_lock("@ROOT")), 0, block);
  else {
    if(request.rtype == Rtype::READ)
      sl = shared_lock(lock.find_lock("@ROOT"));
    else sl = unique_lock(lock.find_lock("@ROOT"));
  }
  std::cout << "here\n";
  /*sleep(1000);*/
  fs_inode inode;
  //reading the file's inode
  char _type_ = 'f';
  access_inode(block, inode, _type_);
  /*for(size_t i = 0; i < inode.size; i++){*/
  /*  int b = inode.blocks[i];*/
    if(request.tar_block < inode.size){
      if(request.rtype == Rtype::READ)
        disk_readblock(inode.blocks[request.tar_block], &request.content);
      else 
        disk_writeblock(inode.blocks[request.tar_block], &request.content);
      goto succeed;
    }
  /*}*/
  //write the new block to the file
  if(request.rtype == Rtype::WRITE)
  {
    if(request.tar_block != inode.size)
      throw NofileErr("exceed file size");
    iiblock(inode);
    //first write the data
    disk_writeblock(inode.blocks[request.tar_block], request.content);
    //then write inode 
    disk_writeblock(block, &inode);
  }
  else{
    throw NofileErr("no block matched");
  }
succeed:
  return;
}

void Disk_Server::_delete(){
  int dir_block = 0, file_block, db_size = 0;
  std::string del_entry = *(request.path.end()-1);
  lock_var ul;
  fs_inode din, fin;
  fs_direntry inv[8];
  size_t i=0;
  int fib;//file inode block
  int b;//the inventory of delted file's inode block
  if(request.path.size() == 1)
    throw NofileErr("cannot delete root");
  {
    //scope of shared lock
    lock_var sl;
    if(request.path.size() > 2)
      sl = _access(shared_lock(lock.find_lock("@ROOT")), 0, dir_block);
    else 
      sl = shared_lock(lock.find_lock("@ROOT"));
    char _type_ = 'd';
    access_inode(dir_block, din, _type_);
    for(; i < din.size; i++){
      b = din.blocks[i];
      if(dir_find(b, file_block, del_entry, db_size, inv, fib))
        goto found;
    }
    throw NofileErr("file to delete not found");
  found:
    //now writelock the file to delete;
    ul = unique_lock(lock.find_lock(del_entry));
  }
  char type = 'a';
  access_inode(file_block, fin, type);
  if(type == 'd'){
    //check if directory is empty
    int _size = 0, _block_, _i_;
    fs_direntry _inv_[8];
    for(size_t i = 0; i < fin.size; i++){
      dir_find(fin.blocks[i], _block_, "", _size, _inv_, _i_);
      if(!_size)
        throw NofileErr("deleting non empty directory");
    }
  }
  //freeing block of the deleted
  for(size_t j = 0; j < fin.size; j++){
    int _s_, _b_ = 1;
    assert(free_list_access(fin.blocks[j], _s_, _b_) == 1);
    free_list_access(fin.blocks[j], _s_, _b_, true); 
  }
  lock_var dul = unique_lock(lock.find_lock(*(request.path.end()-2)));
  if(db_size == 1)
  {
    //shrink the dir
    /*uint32_t tmp[FS_MAXFILEBLOCKS];*/
    /*memcpy(tmp, din.blocks, FS_MAXFILEBLOCKS);*/
    /*memcpy(din.blocks+i, tmp+i+1, din.size-i-1);*/
    for(int j = i; j < din.size-1; j++){
      din.blocks[j] = din.blocks[j+1];
    }
    din.size--;
    disk_writeblock(dir_block, &din);
    int _s_, _b_;
    //free the block
    free_list_access(din.blocks[i], _s_, _b_, true);
  } else{
    inv[fib].inode_block = 0;
    disk_writeblock(b, inv);//delete the entry in the inv
  }
}



void Disk_Server::_create(){
  //TODO: Shrink this chunk with delete
  int dir_block = 0; 
  std::string name = *(request.path.end()-1);
  if(request.path.size() == 1)
    throw NofileErr("missing filename");
  int file_inode_block = 0, _s_;
  free_list_access(0, _s_, file_inode_block);
  if(file_inode_block == 0)
    throw NofileErr("no free space on disk!");
  lock_var ul;
  if(request.path.size() > 2)
    ul = _access(shared_lock(lock.find_lock("@ROOT")), 0, dir_block);
  else 
    ul = unique_lock(lock.find_lock("@ROOT"));
  char _type_ = 'd';
  fs_inode din;
  access_inode(dir_block, din, _type_);
  int entry = din.size * 8;
  fs_direntry inv[din.size * 8] = {};
  int i = 0;
  for(; i < din.size; i++){
    //must traverse to see if there is dup name
    int _n_, _i_, _entry = entry;//place holder
    if(dir_find(din.blocks[i], _entry, name, _n_, inv + i * 8, _i_))
      throw NofileErr("already exists");
    entry = std::min(entry, i * 8 + _entry);
  }
  int free_block = 0, dir_b_w;//free block for new dir entry
  fs_direntry _inv[8];
  bool need_expand=false;
  if(entry == din.size * 8){
    need_expand = true;
    //need expand directory
    if(din.size == FS_MAXFILEBLOCKS)
      throw NofileErr("maximum directory size");
    //find one free block
    free_list_access(0, _s_, free_block);
    if(free_block == 0)
      throw NofileErr("no free blocks to expand directory");
    din.blocks[din.size++] = free_block;
    for(int i = 0; i < 8; i++)
      _inv[i].inode_block = 0;
    strcpy(_inv[0].name, name.c_str());
    _inv[0].inode_block = file_inode_block;
    dir_b_w = free_block;
  } else {
    std::cout << "entry: " << entry << '\n';
    for(int i = 0; i < 8; i++){
      _inv[i] = inv[entry/8 * 8 + i];
      /*std::cout << inv[entry/8 + i].name << ' ' << inv[entry/8 + i].inode_block << '\n';*/
    }
    strcpy(_inv[entry%8].name,name.c_str());
    _inv[entry%8].inode_block = file_inode_block;
    dir_b_w = din.blocks[entry/8];
  }
  fs_inode fin = fs_inode{request.ftype == Ftype::FILE ? 'f' : 'd', "", 0};
  strcpy(fin.owner, request.usr.c_str());
  //first should write the inode of the new file
  disk_writeblock(file_inode_block, &fin);
  //second write the block of dir where new entry is mapped
  disk_writeblock(dir_b_w, _inv);
  //third write dir inode, only if new block added
  if(need_expand)
    disk_writeblock(dir_block, &din);
}



/*
 * @brief debug function
 * */
void Disk_Server::print_req(){
  switch (request.rtype) {
    case Rtype::DELETE:{
      std::cout << "DELETE\n";
      break;
    }
    case Rtype::READ:{std::cout << "READ\n";break;}
    case Rtype::WRITE:{std::cout << "WRITE\n";break;}
    case Rtype::CREATE:{std::cout << "CREATE\n";break;}
  }
  switch (request.ftype) {
    case Ftype::FILE:{std::cout << "file\n";break;}
    case Ftype::DIR:{std::cout << "dir\n";break;}
  }
  std::cout <<"usr: "<< request.usr << '\n';
  std::cout << "path: ";
  for(auto v : request.path)
    std::cout << v << " ";
  std::cout << "\n";
  std::cout << "block: " << request.tar_block << '\n';
  std::cout << "content: " << request.content << '\n';
}

/*
 * @brief This function handles the requests
 * */
void Disk_Server::handle(){
  try{
  _recv();
  to_req(parse_del(str_in, ' '));
  print_req();
  //handle the request
  switch (request.rtype) {
    case Rtype::READ:{
      _readwrite();
      break;
    }
    case Rtype::WRITE:{
      _readwrite();

      break;
    }
    case Rtype::CREATE:{
      _create();
      break;
    }
    case Rtype::DELETE:{
      _delete();
      break;
    }
  }
  }
  //send the response
  /**/
  catch(const NofileErr& e){
    std::cerr << e.msg << '\n';
    _close();
    delete this;//delete the handler when spawning thread
    return;//NOTE: not sure if work
  }
  _send();
  _close();
  //NOTE: This may lead to Segmentation fualt
  delete this;//delete the handler when spawning thread
}

/*
 * @brief return mutex of the lock of the file
 * @note that passing in an invalid file screw things up
 *
 * */
boost::shared_mutex& Lock::find_lock(std::string str){
    //first aquiring memory lock
    boost::lock_guard<boost::mutex> __lock(mem_mt);
    //aquire shared mutex for read
    /*boost::shared_lock<boost::shared_mutex> _lock(lock.sd_mt);*/
    //check if the file exists
    /*auto it = file_locks.find(str);*/
    /*if(it != file_locks.end()){*/
    /*  return it->second;*/
    /*}*/
  return file_locks[str];
  }

#include "disk.h"

Lock::Lock(){}
Lock Disk_Server::lock;
/*
 * @brief fill in the non free blocks
 * */
void free_list_init(int block, std::vector<bool>& v){
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
std::vector<bool> Disk_Server::free_list = [](){
  std::vector<bool> v(4096);
  //free list traverse
  free_list_init(0, v);
  /*for(auto v_ : v)*/
  /*std::cout << v_ << ' ';*/
  return v;
}();
Disk_Server::Disk_Server(int fd):Server(fd){}

/*
 * @brief access the free_list with lock
 * @param idx: the index you watn to change
 * @param state: the state you watn it to be
 * @return true if the state toggles
 * */
bool free_list_access(uint32_t idx, bool state){
  boost::lock_guard<boost::mutex> _lock(Disk_Server::lock.mem_mt);
  bool is_free = Disk_Server::free_list[idx] != state;
  Disk_Server::free_list[idx] = state;
  return is_free;
}

/*
 * @brief get a free block from the free list and make it occupied
 * @param free: free block
 * */
bool get_free_block(uint32_t& free){
  boost::lock_guard<boost::mutex> _lock(Disk_Server::lock.mem_mt);
  for(size_t i = 0; i < Disk_Server::free_list.size(); i++){
    if(Disk_Server::free_list[i] == FREE){
      //free block
      free = i;
      Disk_Server::free_list[i] = OCCUPIED;
      return true;
    }
  }
  return false;
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
void Disk_Server::access_inode(int block, fs_inode& in, char type){
  disk_readblock(block, &in);
  std::stringstream ss;
  ss << "expect " << type << " found " << in.type << '\n';
  if(type != 'a' && in.type != type)
    throw NofileErr(ss.str());
  if(strcmp(in.owner, "") != 0 && in.owner != request.usr)
    throw NofileErr("access denied");
}



/*
 * @brief find a file in directory 
 * @param in: inode of the directory
 * @param entry: entry of the found entry (if not found, this is the first not occupied entry in current dir block)
 * @param inv: fs_direntry inv used to type cast read value
 * @return true if found false otw
 * */

bool dir_find(const fs_inode in, uint32_t& entry, std::string target, fs_direntry* inv){
  for(uint32_t i = 0; i < in.size; i++){
    disk_readblock(in.blocks[i], inv + i * 8);
    uint32_t j = 0;
    for(; j < 8; j++){
      if(inv[i * 8 + j].inode_block == 0)
      {
        //update free_block
        entry = std::min(i * 8 + j, entry);
      } else if(inv[i * 8 + j].name == target){
        entry = i * 8 + j;
        return true;
      }
    }
  }
  return false;
}




/*
 * @brief this function access to the point second last element along the path
 * @param block is the block of the next inode
 *
 * */


Acc Disk_Server::_access(lock_var curr_lk, int i, uint32_t& block, fs_inode& curr_node){
  //base case
  if(request.path.size() < 2)
    throw NofileErr("must have more than two pathname");
  const std::string next_dir = request.path[i + 1];//safe
  std::cout << "next_dir: " << next_dir << '\n';
  access_inode(block, curr_node, 'd');
  fs_direntry* inv_r = new fs_direntry[curr_node.size * 8]; 
  uint32_t entry=curr_node.size * 8;
  //traverse direntry array
  bool found = dir_find(curr_node, entry, next_dir, inv_r);
  if(i == request.path.size() - 2) {
    if(request.rtype == Rtype::CREATE && found)
      throw NofileErr(next_dir + " already exist");
    else if(request.rtype != Rtype::CREATE && !found)
      throw NofileErr(next_dir + " not found!");
    return Acc{boost::move(curr_lk), entry, std::unique_ptr<fs_direntry[]>(inv_r)};
  }
  if(!found)
    throw NofileErr(next_dir + " not found!");
  lock_var nl;
  if(i == request.path.size() - 3 && (request.rtype == Rtype::DELETE || request.rtype == Rtype::CREATE))
    nl = unique_lock(lock.find_lock(next_dir));
  else 
    nl = shared_lock(lock.find_lock(next_dir));
  /*std::cout << "next_dir: " << next_dir << '\n';*/
  return _access(boost::move(nl), i+1, block = inv_r[entry].inode_block, curr_node);
  // NOTE: will the fs always well formed?
  //assume well formed now
}




//inode insert block
uint32_t iiblock(fs_inode& inode){
  uint32_t free_block;
  if(inode.size == FS_MAXFILEBLOCKS)
    throw NofileErr("maximum file size");
  if(!get_free_block(free_block))
    throw NofileErr("no free block");
  inode.blocks[inode.size++] = free_block;
  return free_block;
}



void Disk_Server::_read(){
  uint32_t block = 0;
  fs_inode din, fin;
  Acc acc;
  {
    lock_var lk = shared_lock(lock.find_lock("@ROOT"));
    acc = _access(boost::move(lk), 0, block, din);
  }
  lock_var lv = shared_lock(lock.find_lock(*(request.path.end()-1)));
  access_inode(acc.inv.get()[acc.entry].inode_block, fin, 'f');
  if(request.tar_block >= fin.size)
    throw NofileErr("block exceed boundary");
  disk_readblock(fin.blocks[request.tar_block], &request.content);
}




void Disk_Server::_write(){
  uint32_t block = 0;
  fs_inode din, fin;
  lock_var lv;
  {
    lock_var lk = request.path.size() <= 2 ? unique_lock(lock.find_lock("@ROOT")) : 
                                             shared_lock(lock.find_lock("@ROOT"));
    Acc acc = _access(boost::move(lk), 0, block, din);
    block = acc.inv.get()[acc.entry].inode_block;
    lv = unique_lock(lock.find_lock(*(request.path.end()-1)));
  }
  access_inode(block, fin, 'f');
  if(request.tar_block > fin.size)
    throw NofileErr("exceed file size");
  else if(request.tar_block < fin.size){
    //write the data
    disk_writeblock(fin.blocks[request.tar_block], &request.content);
  }else{
    iiblock(fin);
    disk_writeblock(fin.blocks[request.tar_block], &request.content);
    //then write inode 
    disk_writeblock(block, &fin);
  }
}



void Disk_Server::_delete(){
  uint32_t din_block = 0;
  lock_var ful;
  lock_var dlv = request.path.size() <= 2 ? unique_lock(lock.find_lock("@ROOT")) :
                                            shared_lock(lock.find_lock("@ROOT"));
  fs_inode din, fin;
    //scope of shared lock
  Acc acc = _access(boost::move(dlv), 0, din_block, din);
  uint32_t file_entry = acc.entry;//the entry #
  uint32_t file_block = acc.inv.get()[file_entry].inode_block;//the inode block of file
  //acquire the UNIQUE lock on the file(or dir)
  ful = unique_lock(lock.find_lock(*(request.path.end()-1)));
  access_inode(file_block, fin, 'a');
  //check if directory is empty
  if(fin.type == 'd' && fin.size != 0)
    throw NofileErr("deleting non-empty dir");
  //freeing block of the deleted
  for(size_t j = 0; j < fin.size; j++){
    //TODO: free_list_access to multiple function
    free_list_access(fin.blocks[j], FREE);
  }
  int size = 0;
  for(int i = file_entry/8 * 8; i < file_entry/8 * 8 + 8; i++){
    if(acc.inv.get()[i].inode_block != 0)
      size++;
  }
  if(size == 1)
  {
    //shrink the dir
    for(int j = file_entry/8; j < din.size-1; j++){
      din.blocks[j] = din.blocks[j+1];
    }
    din.size--;
    //write the inode block
    disk_writeblock(din_block, &din);
    //free the block
    free_list_access(din.blocks[file_entry/8], FREE);
  } else {
    acc.inv.get()[file_entry].inode_block = 0;
    disk_writeblock(din.blocks[file_entry/8], acc.inv.get() + file_entry/8*8);//delete the entry in the inv
  }
}



void Disk_Server::_create(){
  std::string name = *(request.path.end()-1);
  uint32_t file_inode_block;
  if(!get_free_block(file_inode_block))
    throw NofileErr("no free space on disk!");
  fs_inode din;
  uint32_t dir_block = 0; 
  lock_var dlv = request.path.size() <= 2 ? unique_lock(lock.find_lock("@ROOT")) :
                                            shared_lock(lock.find_lock("@ROOT"));
  Acc acc = _access(boost::move(dlv), 0, dir_block, din);
  uint32_t free_block, dir_b_w;//free block for new dir entry
  bool need_expand = false;
  fs_direntry _inv[8]={"",0};//the directory block of change
  if(acc.entry == din.size * 8){
    need_expand = true;
    dir_b_w = iiblock(din);
    //prepare _inv
   _inv[0] = {"", file_inode_block};
    strcpy(_inv[0].name, name.c_str());
  } else {
    for(int i = 0; i < 8; i++){
      _inv[i] = acc.inv.get()[acc.entry/8 * 8 + i];
      std::cout << acc.inv.get()[acc.entry/8 + i].name << ' ' << acc.inv.get()[acc.entry/8 + i].inode_block << '\n';
    }
    //_inv block modified by adding to it the new entry
    strcpy(_inv[acc.entry%8].name, name.c_str());
    _inv[acc.entry%8].inode_block = file_inode_block;
    dir_b_w = din.blocks[acc.entry/8];
  }
  fs_inode fin = fs_inode{request.ftype == Ftype::FILE ? 'f' : 'd', "", 0};
  strcpy(fin.owner, request.usr.c_str());
  disk_writeblock(file_inode_block, &fin);//inode of the new file
  disk_writeblock(dir_b_w, _inv);//block of dir where new entry is mapped
  if(need_expand)
    disk_writeblock(dir_block, &din);//dir inode, only if new block added
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
      _read();
      break;
    }
    case Rtype::WRITE:{
      _write();

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

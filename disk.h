#include <asm-generic/socket.h>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/pthread/mutex.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include "server.h"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <boost/thread.hpp>
#define OCCUPIED true
#define FREE false
using shared_lock = boost::shared_lock<boost::shared_mutex>;
using unique_lock = boost::unique_lock<boost::shared_mutex>;
using lock_var = std::variant<shared_lock, unique_lock>;

void print_fl();
bool get_free_block(uint32_t& free, bool toggle);
struct NofileErr{
  std::string msg;
  NofileErr(std::string str):msg{str}{}
};

struct Acc{
  lock_var lv;
  uint32_t entry;
  std::unique_ptr<fs_direntry[]> inv;
};

uint32_t iiblock(fs_inode& inode);
struct Lock{
  Lock();
  boost::mutex mem_mt;
  boost::mutex mem_mt_free;
  std::unordered_map<std::string, boost::shared_mutex> file_locks;
  boost::shared_mutex& find_lock(std::string str);
  void remove_lock(std::string str);
  friend void probe(int block);
};

class Disk_Server : public Server{
  bool status = false;
  void access_inode(int block, fs_inode& in, char type);
  Acc _access(lock_var curr_lk, int i, uint32_t& block, fs_inode& curr_node);
  void _read();
  void _write();
  void _create();
  void _delete();
  public:
  void handle();
  static Lock lock;
  static std::vector<bool> free_list;
  void print_req();
  Disk_Server(int newfd);
  friend uint32_t iiblock(fs_inode& inode);
};


#include <asm-generic/socket.h>
#include <boost/thread/pthread/mutex.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include "server.h"
#include "fs_server.h"
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <boost/thread.hpp>

class Disk{
  protected:
  boost::mutex* mem_mt;
  std::vector<int>* free_list;
  std::unordered_map<std::string, boost::shared_mutex>* read_locks;
  std::unordered_map<std::string, boost::shared_mutex>* write_locks;
public:
  void _write();
  void _read();
  void _create();
  void _delete();
};

class Disk_Server : public Disk, public Server{
  public:
  void print_req();
  Disk_Server(int newfd);
  void handle();
};

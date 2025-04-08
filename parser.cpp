#include <cassert>
#include <iostream>
#include <sstream>
#include "fs_param.h"
#include <string>
#include <vector>

std::vector<std::string> parse(std::string& str, char del){
  std::stringstream ss(str);
  std::vector<std::string> path;
  int i=0;
  char file[FS_MAXFILENAME];
  while(ss.getline(file, FS_MAXFILENAME, del)){
    std::string file_str(file);
    file_str += file_str == "" ? "@ROOT" : "";
    int n = file_str.find(' ');
    /*std::cout << "n= " << n << '\n';*/
    if (n != -1)
      std::cerr << "space shouldn't appear!!\n";
    path.push_back(file_str);
    /*std::cout << path[i++] << '\n';*/
  }
  return path;
}
std::vector<std::string> get_path(std::vector<std::string>&& vec){
  /*std::cout << "vec[0]  = " << vec[0] << '\n';*/
  std::vector<std::string> path;
  if (vec[0] == "FILEREAD"){
    /*std::cout << "fileread!\n";*/
    path = parse(vec[1], '/');
  }
  return path;
}

int main(){
std::string str("FILEREAD /file/a/b.txt XX");
  std::vector<std::string> path = get_path(parse(str, ' '));
  for(auto v : path){
    std::cout << v << " ";
  }
}

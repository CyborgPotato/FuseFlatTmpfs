#include "fs.h"
#include "string.h"
#include <time.h>

flatTmp_node_t flatTmp_node_init() {
  flatTmp_node_t ret = {0};
  return ret;
}

flatTmp_node_t flatTmp_node_blank() {
  flatTmp_node_t ret = {0};
  ret.name = strdup("/");
  
  struct timespec ts_c;
  clock_gettime(CLOCK_REALTIME, &ts_c);
  ret.stat.st_ctim = ts_c;
  
  return ret;
}

flatTmp_node_t flatTmp_dir_init(const char* path, mode_t mode) {
  flatTmp_node_t ret = flatTmp_node_blank();
  ret.stat = (struct stat) {
    .st_mode = S_ISDIR(mode),
    .st_ctim = 0
  };
  return ret;
}

void flatTmp_node_destroy() {}



#include "fs.h"
#include "string.h"
#include <time.h>
#include <stdlib.h>

flattmp_node_t flattmp_node_init() {
  flattmp_node_t ret = flattmp_dir_init("/", 0744);
  return ret;
}

void flattmp_node_destroy(flattmp_node_t node) {
  free(node.name);
}

flattmp_node_t flattmp_node_blank(const char* path) {
  flattmp_node_t ret = {0};
  ret.name = strdup("/");
  
  struct timespec ts_c;
  clock_gettime(CLOCK_REALTIME, &ts_c);
  ret.stat.st_ctim = ts_c;
  
  return ret;
}

flattmp_node_t flattmp_dir_init(const char* path, mode_t mode) {
  flattmp_node_t ret = flattmp_node_blank(path);
  ret.stat = (struct stat) {
    .st_mode = S_ISDIR(mode),
    .st_ctim = 0
  };
  return ret;
}

flattmp_node_t* flattmp_node_search(flattmp_node_t node, const char* path) {
  if (node.stat.st_mode & S_IFDIR) {

  }
}




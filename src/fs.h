#ifndef FLATTMP_FS_H
#define FLATTMP_FS_H

#define FUSE_USE_VERSION 31

#include <stddef.h>
#include <fuse3/fuse.h>

typedef struct FLATTMP_NODE flatTmp_node_t;
typedef struct FLATTMP_FILE flatTmp_file_t;
typedef struct FLATTMP_DIR flatTmp_dir_t;

struct FLATTMP_FILE {
  void* content;
  size_t sz;
};

#define n_ints_in(x) ((x/8)/sizeof(int))

struct FLATTMP_DIR {
  int lbits[n_ints_in(256)];
  flatTmp_node_t *nodes;
};

struct FLATTMP_NODE {
  char* name;
  struct stat stat;
  union data {
    flatTmp_file_t file;
    flatTmp_dir_t dir;
  } data;
};

flatTmp_node_t flatTmp_node_init();
void flatTmp_node_destroy();

#endif

#ifndef FLATTMP_FS_H
#define FLATTMP_FS_H

#define FUSE_USE_VERSION 31

#include <stddef.h>
#include <fuse3/fuse.h>

typedef struct FLATTMP_NODE flattmp_node_t;
typedef struct FLATTMP_FILE flattmp_file_t;
typedef struct FLATTMP_DIR flattmp_dir_t;

struct FLATTMP_FILE {
  void* content;
  size_t sz;
};

#define n_ints_in(x) ((x/8)/sizeof(int))

struct FLATTMP_DIR {
  int lbits[n_ints_in(256)];
  flattmp_node_t *nodes;
};

struct FLATTMP_NODE {
  char* name;
  struct stat stat;
  union data {
    flattmp_file_t file;
    flattmp_dir_t dir;
  } data;
};

flattmp_node_t flattmp_node_init();
void flattmp_node_destroy();
flattmp_node_t flattmp_dir_init(const char *path, mode_t mode);

flattmp_node_t* flattmp_node_search(flattmp_node_t node, const char* path);

#endif

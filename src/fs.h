#ifndef FLATTMP_FS_H
#define FLATTMP_FS_H

#define FUSE_USE_VERSION 31

#include <stddef.h>
#include <fuse3/fuse.h>
#include <uthash.h>

#define FLATTMP_MAX_FILE_LEN 255

typedef struct FLATTMP_NODE flattmp_node_t;
typedef struct FLATTMP_FILE flattmp_file_t;
typedef struct FLATTMP_DIR flattmp_dir_t;

struct FLATTMP_FILE {
  void* data;
  size_t sz;
};

struct FLATTMP_DIR {
  flattmp_node_t *nodes;
};

struct FLATTMP_NODE {
  char name[FLATTMP_MAX_FILE_LEN];
  struct stat stat;
  union {
    flattmp_file_t file;
    flattmp_dir_t dir;
  } content;
  UT_hash_handle hh;
};

flattmp_node_t* flattmp_node_root();
flattmp_node_t* flattmp_node_init(const char*);
void flattmp_node_destroy(flattmp_node_t **);

flattmp_node_t* flattmp_dir_init(const char* name, mode_t mode);
flattmp_node_t* flattmp_dir_add(flattmp_node_t *node, const char* path, mode_t mode);
  
flattmp_node_t* flattmp_node_exists(flattmp_node_t *node, const char* path);

#endif

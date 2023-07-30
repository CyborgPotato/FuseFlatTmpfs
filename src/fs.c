#include "fs.h"
#include "string.h"
#include "uthash.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

flattmp_node_t* flattmp_node_root() {
  flattmp_node_t *ret = flattmp_dir_init("/", 0744);
  return ret;
}

void flattmp_node_destroy(flattmp_node_t **node_p) {
  flattmp_node_t *node = *node_p;
  if (node->stat.st_mode & S_IFDIR) {
    free(node->content.dir.nodes);
  }
}

flattmp_node_t* flattmp_node_init(const char* name) {
  flattmp_node_t *ret = calloc(1, sizeof(flattmp_node_t));
  strcpy(&ret->name[0], &name[0]);

  struct timespec ts_c;
  clock_gettime(CLOCK_REALTIME, &ts_c);
  ret->stat.st_ctim = ts_c;
  ret->stat.st_atim = ts_c;
  ret->stat.st_mtim = ts_c;
  
  return ret;
}

flattmp_node_t* flattmp_dir_init(const char* name, mode_t mode) {
  flattmp_node_t *ret = flattmp_node_init(name);
  ret->stat.st_mode = S_IFDIR | mode;
  ret->stat.st_nlink = 2;
  return ret;
}

flattmp_node_t* flattmp_dir_get(flattmp_node_t *node, const char* name) {
  flattmp_node_t *ret = NULL;
  if (node->stat.st_mode & S_IFDIR) {
    HASH_FIND_STR(node->content.dir.nodes, name, ret);
  }
  return ret;
}

flattmp_node_t* flattmp_dir_add(flattmp_node_t *node, const char* path, mode_t mode) {
  if ((~node->stat.st_mode) & S_IFDIR)
    return NULL;
  flattmp_node_t *ret = node;
  flattmp_node_t *par = NULL;
  char* basename;

  char* offset;
  char* name;

  char* eop = (char*) path + strlen(path);
  
  for (name = strtok_r((char*) path, "/", &offset); name!=NULL; name=strtok_r(NULL, "/", &offset)) {
    par = ret;
    basename = name;
    ret = flattmp_dir_get(ret, name);
    if ( ret == NULL && offset != eop) {
      return NULL;
    }
  }

  ret = flattmp_node_init(basename);

  ret->stat.st_mode = mode;
  ret->stat.st_nlink = 1 + S_ISDIR(mode);
  
  HASH_ADD_STR(par->content.dir.nodes, name, ret);
  
  return ret;
}

flattmp_node_t* flattmp_node_exists(flattmp_node_t* node, const char* path) {
  flattmp_node_t *ret = node;

  char* offset;
  char* name;
  for (name = strtok_r((char*) path, "/", &offset); name!=NULL; name=strtok_r(NULL, "/", &offset)) {
    ret = flattmp_dir_get(ret, name);
    if ( ret == NULL) {
      return NULL;
    }
  }
  return ret;
}

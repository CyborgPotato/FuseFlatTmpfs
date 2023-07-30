#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

static struct options {
  size_t size;
  size_t max_size;
  size_t n_inodes;
  size_t max_inodes;
  size_t max_filesize;
} options;

flattmp_node_t *rootNode;

#define OPTION(t, p, v)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt options_spec[] = {
  OPTION("--size=%lu", max_size, 16 * 1024*1024),
  OPTION("--max-inodes=%lu", max_inodes, 256),
  OPTION("--max-filesize=%lu", max_filesize, 16 * 1024*1024),
  FUSE_OPT_END
};

static void *flattmp_init(struct fuse_conn_info *conn,
			  struct fuse_config *cfg) {
  cfg->kernel_cache = 1;
  rootNode = flattmp_node_root();
  return NULL;
}

static void flattmp_destroy(void *data) {
  flattmp_node_destroy(&rootNode);
}

static int flattmp_getattr(const char *path, struct stat *stbuf,
			   struct fuse_file_info *fi) {
  int res = 0;

  struct fuse_context *ctx;
  ctx = fuse_get_context();

  memset(stbuf, 0, sizeof(struct stat));
  flattmp_node_t *node = flattmp_node_exists(rootNode, path);
  if (node != NULL) {
    memcpy(stbuf, &node->stat, sizeof(node->stat));
  }else {
    res = -ENOENT;
  }

  return res;
}

static int flattmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			   off_t offset, struct fuse_file_info *fi,
			   enum fuse_readdir_flags flags) {
  if (strcmp(path, "/") !=0)
    return -ENOENT;

  filler(buf, ".", NULL, 0, 0);
  filler(buf, "..", NULL, 0, 0);

  flattmp_node_t *node = flattmp_node_exists(rootNode, path);
  if (node != NULL) {
    flattmp_node_t *n;
    for (n = node->content.dir.nodes; n != NULL; n = n->hh.next) {
      filler(buf, n->name, NULL, 0, 0);
    }
  }

  return 0;
}

static int flattmp_create(const char* path, mode_t mode, struct fuse_file_info *fi) {
  if (options.n_inodes >= options.max_inodes)
    return -ENOMEM;
  flattmp_node_t *node = flattmp_node_exists(rootNode, path);
  struct fuse_context *ctx = fuse_get_context();
  if (node == NULL) {
    node = flattmp_dir_add(rootNode, path, mode);
    if (node == NULL)
      return -ENOENT;
    node->stat.st_uid = ctx->uid;
    node->stat.st_gid = ctx->gid;
  }
  options.n_inodes += 1;
  return 0;
}

static int flattmp_write(const char *path, const char *buf, size_t sz,
			 off_t offset, struct fuse_file_info *fi) {
  size_t sz_ = offset+sz;
  if (sz_ > options.max_filesize) {
    sz -= sz_ - options.max_filesize;
    sz_ = options.max_filesize;
  }

  flattmp_node_t *node = flattmp_node_exists(rootNode, path);
  flattmp_file_t *file = &node->content.file;

  if (node->stat.st_size >= options.max_filesize)
    return -ENOMEM;

  if (options.size >= options.max_size)
    return -ENOMEM;
  
  long int sz_inc = sz_ - node->stat.st_size;
  if (options.size + sz_inc > options.max_size) {
    sz  = options.max_size - options.size;
    sz_ = offset+sz;
  }
  
  if ( file->sz < sz_ || file->sz > sz_*2) {
    void* data = realloc(file->data, sz_);
    if (data == NULL) {
      return -ENOMEM;
    }
    file->data = data;
  }
  memcpy(file->data+offset, buf, sz);

  node->stat.st_size = sz_;
  options.size += sz_inc;
  
  return sz;
}

static int flattmp_read(const char *path, char *buf, size_t sz,
                        off_t offset, struct fuse_file_info *fi) {
  flattmp_node_t *node = flattmp_node_exists(rootNode, path);
  flattmp_file_t *file = &node->content.file;
  if (file->data == NULL)
    return EOF;
  size_t n_ = node->stat.st_size - offset;
  size_t n  = n_>sz?sz:n_;

  memcpy(buf, file->data+offset, n);
  return n;
}

static int flattmp_truncate(const char *path, off_t sz, struct fuse_file_info *fi) {
  if (sz > options.max_filesize)
    return -ENOMEM;

  flattmp_node_t *node = flattmp_node_exists(rootNode, path);

  int sz_inc = sz - node->stat.st_size;
  if (options.size + sz_inc > options.max_size) {
    return -ENOMEM;
  }
  
  flattmp_file_t *file = &node->content.file;
  void* data = realloc(file->data, sz);
  if (data == NULL) {
    return -ENOMEM;
  }
  file->data = data;
  node->stat.st_size = sz;
  options.size += sz_inc;
  
  return 0;
}

static int flattmp_unlink(const char* path) {
  flattmp_node_t *node;
  node = flattmp_dir_del(rootNode, path);
  if (node == NULL)
    return -ENOENT;
  options.size -= node->stat.st_size;
  free(node);
  options.n_inodes -=1;
  return 0;
}

const struct fuse_operations flattmp_op = {
  .init     = &flattmp_init,
  .destroy  = &flattmp_destroy,
  .getattr  = &flattmp_getattr,
  .readdir  = &flattmp_readdir,
  .create   = &flattmp_create,
  .write    = &flattmp_write,
  .read     = &flattmp_read,
  .truncate = &flattmp_truncate,
  .unlink   = &flattmp_unlink,
};

int main(int argc, char** argv) {
  int ret;

  options.size = 0;
  options.max_size = 16 * 1024*1024;

  options.n_inodes = 0;
  options.max_inodes = 5;

  options.max_filesize = 4 * 1024*1024;
  
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  if (fuse_opt_parse(&args, &options, options_spec, NULL) == -1) {
    return 1;
  }

  printf("Max FS Size: %lu\nMax File Size: %lu\n", options.max_size, options.max_filesize);
  
  ret = fuse_main(args.argc, args.argv, &flattmp_op, NULL);
  fuse_opt_free_args(&args);
  
  return ret;
}

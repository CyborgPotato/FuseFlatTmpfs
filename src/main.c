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
  const size_t max_size;
  const size_t max_innodes;
  const size_t max_filesize;
} options;

flattmp_node_t rootNode;

#define OPTION(t, p, v)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt options_spec[] = {
  OPTION("--size=%z", max_size, 16*1024*1024),
  OPTION("--max_innodes=%z", max_innodes, 256),
  OPTION("--max_filesize=%z", max_filesize, 16 * 1024*1024),
  FUSE_OPT_END
};

static void *flattmp_init(struct fuse_conn_info *conn,
			  struct fuse_config *cfg) {
  cfg->kernel_cache = 1;
  rootNode = flattmp_node_init();
  return NULL;
}

static void flattmp_destroy(void *data) {
  flattmp_node_destroy(rootNode);
}

static int flattmp_getattr(const char *path, struct stat *stbuf,
			   struct fuse_file_info *fi) {
  int res = 0;

  struct fuse_context *ctx;
  ctx = fuse_get_context();

  memset(stbuf, 0, sizeof(struct stat));
  flattmp_node_t node = flattmp_node_search(rootNode, path);
  if (node.name != NULL) {
    memcpy(stbuf, &node.stat, sizeof(node.stat));
  }else {
    res = -ENOENT;
  }

  return res;
}

static int flattmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			   off_t offset, struct fuse_file_info *fi,
			   enum fuse_readdir_flags flags) {
  /* There will never be more than the root directory */
  if (strcmp(path, "/") !=0)
    return -ENOENT;

  filler(buf, ".", NULL, 0, 0);
  filler(buf, "..", NULL, 0, 0);
  
  return 0;
}

static int flattmp_mknod(const char* path, mode_t mode, dev_t dev) {
  printf("Creating path \"%s\"\n", path);
  return 0;
}

static int flattmp_open(const char *path, struct fuse_file_info *fi) {
  printf("Attempting to open \"%s\"\n", path);
  return 0;
}

const struct fuse_operations flattmp_op = {
  .init    = &flattmp_init,
  .destroy = &flattmp_destroy,
  .getattr = &flattmp_getattr,
  .readdir = &flattmp_readdir,
  .mknod   = &flattmp_mknod,
  .open    = &flattmp_open,
};

int main(int argc, char** argv) {
  int ret;

  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  if (fuse_opt_parse(&args, &options, options_spec, NULL) == -1) {
    return 1;
  }

  ret = fuse_main(args.argc, args.argv, &flattmp_op, NULL);
  fuse_opt_free_args(&args);
  
  return ret;
};

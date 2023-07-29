#include "fuse3/fuse_opt.h"
#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
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
  return NULL;
}

const struct fuse_operations flattmp_op = {
  .init = flattmp_init,
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

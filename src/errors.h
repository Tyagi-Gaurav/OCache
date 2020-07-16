#ifndef __ERROR_H
#define __ERROR_H

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __ERR_REPO(x, msg) if (x != 0) err_abort(x, msg)
#define __ERRNO_REPO(msg) errno_abort(msg)

#define err_abort(code, text) do { \
  fprintf(stderr, "%s at \"%s\":%d: %s\n", \
	  text, __FILE__, __LINE__, strerror(code)); \
  abort(); \
} while (0)

#define errno_abort(text) do { \
  fprintf(stderr, "%s at \"%s\":%d: %s\n", \
	  text, __FILE__, __LINE__, strerror(errno)); \
  abort(); \
  } while (0)

#endif

// useful find at:
// http://c.learncodethehardway.org/book/ex20.html

#ifndef __dbg_h__
#define __dbg_h__

//#define DODEBUG  // hack until proper gSystem setting is found

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DODEBUG
#define debug(M, ...) fprintf(stderr, "\e[1;34m[DEBUG] %s:%s(%d):\e[0m " M "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif



#define clean_errno() (errno == 0 ? "None" : strerror(errno))

//#define log_err(M, ...) fprintf(stderr, "\e[1;31m[ERROR] (%s:%d: errno: %s)\e[0m " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_err(M, ...) fprintf(stderr, "\e[1;31m[ERROR] (%s:%d:)\e[0m " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

//#define log_err2(M, ...) fprintf(stderr, "[ERROR] (%s:%d) " M "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr, "\e[1;34m[INFO] (%s:%d)\e[0m " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#endif

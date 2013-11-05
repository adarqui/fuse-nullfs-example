#ifndef _LOG_H_
#define _LOG_H_

#include "nullfs.h"

FILE *log_init(void);
void log_fi (struct fuse_file_info *fi);
void log_statvfs(struct statvfs *sv);
void log_utime(struct utimbuf *buf);

void xlog(char *, const char *format, ...);
#endif

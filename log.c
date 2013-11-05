#include "nullfs.h"

FILE * logfile;

FILE *log_init()
{
	logfile = fopen("/tmp/nullfs.log", "w");
	if (logfile == NULL) {
		perror("logfile");
		exit(-1);
    }
   
	setvbuf(logfile, NULL, _IOLBF, 0);

	return logfile;
}

void xlog(char * function, const char *format, ...)
{
	va_list ap;

	if(!logfile) return;

	va_start(ap, format);

	fprintf(logfile, "%s: ", function);
	vfprintf(logfile, format, ap);
}

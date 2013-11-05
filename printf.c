#include "printf.h"

int print_stat (FILE *stream, const struct printf_info *info, const void *const *args) {
	const struct stat *st;
	char *buffer;
	int len;

	st = *((const struct stat **) (args[0]));
	if(!st) return 0;

	len = asprintf(&buffer,
	"dev=%ld,"
	"ino=%ld,"
	"mode=%o,"
	"nlink=%ld,"
	"uid=%d,"
	"gid=%d,"
	"rdev=%ld,"
	"size=%ld,"
	"blksize=%ld,"
	"blocks=%ld,"
	"atime=%ldd,"
	"mtime=%ldd,"
	"ctime=%ldd"
	, st->st_dev
	, st->st_ino
	, st->st_mode
	, st->st_nlink
	, st->st_uid
	, st->st_gid
	, st->st_rdev
	, st->st_size
	, st->st_blksize
	, st->st_blocks
	, st->st_atime
	, st->st_mtime
	, st->st_ctime);

	fprintf(stream, "%s", buffer);
	free(buffer);
	return len;
}


int print_stat_arginfo (const struct printf_info *info, size_t n, int *argtypes) {
	if (n > 0)
		argtypes[0] = PA_POINTER;
	return 1;
}

void hook_printf(void) {
	register_printf_function('W', print_stat, print_stat_arginfo);
	return;
}

#if defined(DEBUG)
int main (void)
{
	struct stat st;

	stat("/etc/issue", &st);
	printf("size: %ld\n", st.st_size);

	hook_printf();

	/* Now print the stat. */
	printf ("|%W|\n", &st);
	fprintf (stdout, "|%W|\n", &st);

	return 0;
}
#endif

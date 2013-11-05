/*
 * modified for verbosity -- ad
 */
/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fusenull.c `pkg-config fuse --cflags --libs` -o fusenull
*/

#include "nullfs.h"

static int null_getattr(const char *path, struct stat *stbuf)
{
	int res;

	xlog("getattr", "path=%s stat={%W}\n", path, stbuf);

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_access(const char *path, int mask)
{
	int res;

	xlog("access", "path=%s mask=%i\n", path, mask);

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_readlink(const char *path, char *buf, size_t size)
{
	int res;

	xlog("readlink", "path=%s buf=%p size=%ld\n", path, buf, size);

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int null_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	xlog("readdir", "path=%s buf=%p filler=.. offset=%ld fi=%p\n", path, buf, offset, fi);

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int null_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	xlog("mknod", "path=%s mode=%o rdev=%ld\n", path, mode, rdev);

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_mkdir(const char *path, mode_t mode)
{
	int res;

	xlog("mkdir", "path=%s mode=%o\n", path, mode);

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_unlink(const char *path)
{
	int res;

	xlog("unlink", "path=%s\n", path);

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_rmdir(const char *path)
{
	int res;

	xlog("rmdir", "path=%s\n", path);

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_symlink(const char *from, const char *to)
{
	int res;

	xlog("symlink", "from=%s to=%s\n", from, to);

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_rename(const char *from, const char *to)
{
	int res;

	xlog("rename", "from=%s to=%s\n", from, to);

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_link(const char *from, const char *to)
{
	int res;

	xlog("link", "from=%s to=%s\n", from, to);

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_chmod(const char *path, mode_t mode)
{
	int res;

	xlog("chmod", "path=%s mode=%o\n", path, mode);

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	xlog("chown", "path=%s uid=%d gid=%d\n", path, uid, gid);

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_truncate(const char *path, off_t size)
{
	int res;

	xlog("truncate", "path=%s size=%ld\n", path, size);

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int null_utimens(const char *path, const struct timespec ts[2])
{
	int res;

	xlog("utimens", "path=%s ts=..\n", path);

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int null_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	xlog("open", "path=%s fi=%p\n", path, fi);

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int null_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	xlog("read", "path=%s buf=%p size=%ld offset=%ld fi=%p\n", path, buf, size, offset, fi);

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int null_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	xlog("write", "path=%s buf=%p size=%ld offset=%ld fi=%p\n", path, buf, size, offset, fi);

	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int null_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	xlog("statfs", "path=%s st=%p\n", path, stbuf);

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int null_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;

	xlog("release", "path=%s fi=%p\n", path, fi);
	return 0;
}

static int null_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;

	xlog("fsync", "path=%s isdatasync=%i fi=%p\n", path, isdatasync, fi);
	return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int null_fallocate(const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	xlog("fallocate", "path=%s mode=%o offset=%ld length=%ld fi=%p\n", path, mode, offset, length, fi);
	if (mode)
		return -EOPNOTSUPP;

	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = -posix_fallocate(fd, offset, length);

	close(fd);
	return res;
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int null_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);

	xlog("setxattr", "path=%s name=%s value=%s size=%ld flags=%i\n",
			path, name, value, size, flags);

	if (res == -1)
		return -errno;
	return 0;
}

static int null_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);

	xlog("getxattr", "path=%s name=%s value=%s size=%ld\n", path, name, value, size);

	if (res == -1)
		return -errno;
	return res;
}

static int null_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);

	xlog("listxattr", "path=%s list=%s size=%ld\n", path, list, size);

	if (res == -1)
		return -errno;
	return res;
}

static int null_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);

	xlog("removexattr", "path=%s name=%s\n", path, name);

	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations null_oper = {
	.getattr	= null_getattr,
	.access		= null_access,
	.readlink	= null_readlink,
	.readdir	= null_readdir,
	.mknod		= null_mknod,
	.mkdir		= null_mkdir,
	.symlink	= null_symlink,
	.unlink		= null_unlink,
	.rmdir		= null_rmdir,
	.rename		= null_rename,
	.link		= null_link,
	.chmod		= null_chmod,
	.chown		= null_chown,
	.truncate	= null_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= null_utimens,
#endif
	.open		= null_open,
	.read		= null_read,
	.write		= null_write,
	.statfs		= null_statfs,
	.release	= null_release,
	.fsync		= null_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= null_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= null_setxattr,
	.getxattr	= null_getxattr,
	.listxattr	= null_listxattr,
	.removexattr	= null_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	hook_printf();
	log_init();
	return fuse_main(argc, argv, &null_oper, NULL);
}

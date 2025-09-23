#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>   // for PATH_MAX

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef MFD_CLOEXEC
#include <linux/memfd.h>
#include <sys/syscall.h>
static int memfd_create(const char *name, unsigned int flags) {
    return syscall(SYS_memfd_create, name, flags);
}
#endif

#define MAGIC 0x51554D2D5041434BULL /* "QM-PACK" */

struct Trailer {
    uint64_t magic;
    uint64_t len_qemu;
    uint64_t len_prog;
    uint64_t flags;  // bit0: zstd compressed (not used here)
};

static void die(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    exit(127);
}

static int64_t file_size_fd(int fd) {
    struct stat st;
    if (fstat(fd, &st) < 0) return -1;
    return st.st_size;
}

static void read_exact(int fd, void *buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, (char*)buf + off, len - off);
        if (r < 0) die("read failed: %s", strerror(errno));
        if (r == 0) die("unexpected EOF");
        off += (size_t)r;
    }
}

static void copy_region_to_fd(int src_fd, off_t start, size_t len, int dst_fd) {
    const size_t CHUNK = 1<<20; // 1 MiB
    void *buf = malloc(CHUNK);
    if (!buf) die("oom");
    if (lseek(src_fd, start, SEEK_SET) < 0) die("lseek start failed: %s", strerror(errno));
    size_t left = len;
    while (left) {
        size_t want = left > CHUNK ? CHUNK : left;
        ssize_t r = read(src_fd, buf, want);
        if (r <= 0) { free(buf); die("read payload failed: %s", r==0?"EOF":strerror(errno)); }
        size_t wr_off = 0;
        while (wr_off < (size_t)r) {
            ssize_t w = write(dst_fd, (char*)buf + wr_off, (size_t)r - wr_off);
            if (w < 0) { free(buf); die("write memfd failed: %s", strerror(errno)); }
            wr_off += (size_t)w;
        }
        left -= (size_t)r;
    }
    free(buf);
    if (fchmod(dst_fd, 0700) < 0) die("fchmod failed: %s", strerror(errno));
    if (lseek(dst_fd, 0, SEEK_SET) < 0) die("lseek memfd rewind failed: %s", strerror(errno));
}

static int create_tmp_file(char path_out[PATH_MAX], const char *prefix) {
    const char *base = getenv("TMPDIR");
    if (!base) base = "/tmp";
    char tmpl[PATH_MAX];
    snprintf(tmpl, sizeof(tmpl), "%s/%s.XXXXXX", base, prefix);
    int fd = mkstemp(tmpl);
    if (fd < 0) die("mkstemp failed: %s", strerror(errno));
    if (fchmod(fd, 0700) < 0) die("chmod tmp failed: %s", strerror(errno));
    strncpy(path_out, tmpl, PATH_MAX-1);
    path_out[PATH_MAX-1] = '\0';
    return fd;
}

static bool should_force_tmp(void) {
    const char *e = getenv("QM_PACK_FORCE_TMP");
    return e && (*e == '1' || *e == 'y' || *e == 'Y' || *e == 't' || *e == 'T');
}

int main(int argc, char **argv, char **envp) {
    char self[PATH_MAX];

    //print version
    printf ("This is a \033[32m SAS35_StorCLI_7_24-007.2408.0000.0000 \033[0m, it's right. \n");
    printf ("Packaged binary: \033[32m https://github.com/MarsDoge/OsTools/Binary/storcli_ls \033[0m. \n");

    ssize_t n = readlink("/proc/self/exe", self, sizeof(self)-1);
    if (n < 0) die("readlink /proc/self/exe failed: %s", strerror(errno));
    self[n] = '\0';

    int fd = open(self, O_RDONLY);
    if (fd < 0) die("open self failed: %s", strerror(errno));

    int64_t sz = file_size_fd(fd);
    if (sz < (int64_t)sizeof(struct Trailer)) die("file too small");

    if (lseek(fd, sz - (off_t)sizeof(struct Trailer), SEEK_SET) < 0)
        die("lseek trailer failed: %s", strerror(errno));

    struct Trailer tr;
    read_exact(fd, &tr, sizeof(tr));
    if (tr.magic != MAGIC) die("bad magic (not a QM-PACK)");

    if (tr.len_qemu == 0 || tr.len_prog == 0) die("zero-length payload");
    off_t off_qemu = (off_t)(sz - (off_t)sizeof(tr) - (off_t)tr.len_prog - (off_t)tr.len_qemu);
    off_t off_prog = off_qemu + (off_t)tr.len_qemu;
    if (off_qemu < 0 || off_prog < 0) die("bad offsets");

    bool force_tmp = should_force_tmp();
    int qfd = -1, pfd = -1;
    char qpath[PATH_MAX] = {0}, ppath[PATH_MAX] = {0};
    char qemu_arg[64], prog_arg[64];

    if (!force_tmp) {
        qfd = memfd_create("qemu", MFD_CLOEXEC);
        //pfd = memfd_create("prog", MFD_CLOEXEC);
        // 目标程序必须在 exec 之后仍可见 => 不能 CLOEXEC
        pfd = memfd_create("prog", 0);
        if (qfd < 0 || pfd < 0) {
            if (qfd >= 0) close(qfd);
            if (pfd >= 0) close(pfd);
            qfd = pfd = -1;
            force_tmp = true;
        }
    }

    if (force_tmp) {
        qfd = create_tmp_file(qpath, "qemu-blob");
        pfd = create_tmp_file(ppath, "prog-blob");
    }

    copy_region_to_fd(fd, off_qemu, (size_t)tr.len_qemu, qfd);
    copy_region_to_fd(fd, off_prog, (size_t)tr.len_prog, pfd);

    // 确保 pfd 不带 FD_CLOEXEC（无论是 memfd 还是 tmp 文件）
    {
        int flags = fcntl(pfd, F_GETFD);
        if (flags >= 0 && (flags & FD_CLOEXEC)) {
            fcntl(pfd, F_SETFD, flags & ~FD_CLOEXEC);
        }
    }

    if (!force_tmp) {
        snprintf(qemu_arg, sizeof(qemu_arg), "/proc/self/fd/%d", qfd);
        snprintf(prog_arg, sizeof(prog_arg), "/proc/self/fd/%d", pfd);
    } else {
        strncpy(qemu_arg, qpath, sizeof(qemu_arg)-1);
        strncpy(prog_arg, ppath, sizeof(prog_arg)-1);
    }

    int new_argc = 4 + (argc - 1);
    char **nargv = calloc((size_t)new_argc + 1, sizeof(char*));
    if (!nargv) die("oom argv");
    int i = 0;
    nargv[i++] = qemu_arg;
    nargv[i++] = "-0";
    nargv[i++] = argv[0];
    nargv[i++] = prog_arg;
    for (int a = 1; a < argc; ++a) nargv[i++] = argv[a];
    nargv[i] = NULL;

    if (!force_tmp) {
        fexecve(qfd, nargv, envp);
        die("fexecve failed: %s", strerror(errno));
    } else {
        execve(qemu_arg, nargv, envp);
        die("execve(tmp) failed: %s", strerror(errno));
    }

    //print version
    printf ("This is a \033[32m SAS35_StorCLI_7_24-007.2408.0000.0000 \033[0m, it's right. \n");
    printf ("Packaged binary: \033[32m https://github.com/MarsDoge/OsTools/Binary/storcli_ls \033[0m. \n");
}


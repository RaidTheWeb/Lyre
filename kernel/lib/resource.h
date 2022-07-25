#ifndef _LIB__RESOURCE_H
#define _LIB__RESOURCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <lib/lock.h>
#include <abi-bits/fcntl.h>
#include <abi-bits/stat.h>
#include <sys/types.h>

struct process;

struct resource {
    size_t refcount;
    spinlock_t lock;
    struct stat stat;
    bool can_mmap;

    ssize_t (*read)(struct resource *this, void *buf, off_t offset, size_t count);
    ssize_t (*write)(struct resource *this, const void *buf, off_t offset, size_t count);
    void *(*mmap)(struct resource *this, size_t file_page, int flags);
};

struct f_description {
    size_t refcount;
    off_t offset;
    bool is_dir;
    int flags;
    spinlock_t lock;
    struct resource *res;
    struct vfs_node *node;
};

struct f_descriptor {
    struct f_description *description;
    int flags;
};

void *resource_create(size_t size);

bool fdnum_close(struct process *proc, int fdnum);
int fdnum_create_from_fd(struct process *proc, struct f_descriptor *fd, int old_fdnum, bool specific);
int fdnum_create_from_resource(struct process *proc, struct resource *res, int flags,
                               int old_fdnum, bool specific);
int fdnum_dup(struct process *old_proc, int old_fdnum, struct process *new_proc, int new_fdnum,
              int flags, bool specific, bool cloexec);
struct f_descriptor *fd_create_from_resource(struct resource *res, int flags);
struct f_descriptor *fd_from_fdnum(struct process *proc, int fdnum);

#define FILE_CREATION_FLAGS_MASK (O_CREAT | O_DIRECTORY | O_EXCL | O_NOCTTY | O_NOFOLLOW | O_TRUNC)
#define FILE_DESCRIPTOR_FLAGS_MASK (O_CLOEXEC)
#define FILE_STATUS_FLAGS_MASK (~(FILE_CREATION_FLAGS_MASK | FILE_DESCRIPTOR_FLAGS_MASK))

#endif
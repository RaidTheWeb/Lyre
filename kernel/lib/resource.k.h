#ifndef _LIB__RESOURCE_K_H
#define _LIB__RESOURCE_K_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <lib/lock.k.h>
#include <lib/event.k.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

struct process;
struct f_description;

struct resource {
    int res_size;
    int status;
    struct event event;
    size_t refcount;
    spinlock_t lock;
    struct stat stat;
    bool can_mmap;

    ssize_t (*read)(struct resource *this, struct f_description *description, void *buf, off_t offset, size_t count);
    ssize_t (*write)(struct resource *this, struct f_description *description, const void *buf, off_t offset, size_t count);
    int (*ioctl)(struct resource *this, struct f_description *description, uint64_t request, uint64_t arg);
    void *(*mmap)(struct resource *this, size_t file_page, int flags);
    bool (*msync)(struct resource *this, size_t file_page, void *phys, int flags);
    bool (*chmod)(struct resource *this, mode_t mode);
    bool (*unref)(struct resource *this, struct f_description *description);
    bool (*ref)(struct resource *this, struct f_description *description);
    bool (*truncate)(struct resource *this, struct f_description *description, size_t length);
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
void resource_free(struct resource *res);
dev_t resource_create_dev_id(void);

bool fdnum_close(struct process *proc, int fdnum, bool lock);
int fdnum_create_from_fd(struct process *proc, struct f_descriptor *fd, int old_fdnum, bool specific);
int fdnum_create_from_resource(struct process *proc, struct resource *res, int flags,
                               int old_fdnum, bool specific);
int fdnum_dup(struct process *old_proc, int old_fdnum, struct process *new_proc, int new_fdnum,
              int flags, bool specific, bool cloexec);
struct f_descriptor *fd_create_from_resource(struct resource *res, int flags);
struct f_descriptor *fd_from_fdnum(struct process *proc, int fdnum);

int resource_default_ioctl(struct resource *this, struct f_description *description, uint64_t request, uint64_t arg);

#define FILE_CREATION_FLAGS_MASK (O_CREAT | O_DIRECTORY | O_EXCL | O_NOCTTY | O_NOFOLLOW | O_TRUNC)
#define FILE_DESCRIPTOR_FLAGS_MASK (O_CLOEXEC)
#define FILE_STATUS_FLAGS_MASK (~(FILE_CREATION_FLAGS_MASK | FILE_DESCRIPTOR_FLAGS_MASK))

#endif

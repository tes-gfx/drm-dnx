#ifndef __DNX_DRM_H__
#define __DNX_DRM_H__

#include <drm/drm.h>


/* timeouts are specified in clock-monotonic absolute times (to simplify
 * restarting interrupted ioctls). The following struct is logically the
 * same as 'struct timespec' but 32/64b ABI safe.
 */
struct drm_dnx_timespec {
	__s64 tv_sec;          /* seconds */
	__s64 tv_nsec;         /* nanoseconds */
};


/* todo: this is deprecated, we do not want reg access from user space, remove! */
struct drm_dnx_reg {
	__u32 idx;                          /* in */
	__u32 value;                        /* in/out */
};

/* note: we're using __u64 here instead of pointers to stay compatible with
 * 32 bit / 64 bit systems
 */
struct drm_dnx_stream_submit {
	__u32 fence;                        /* out */
	__u64 stream;                       /* in, physical addr of start of command stream */
	__u64 jump;                         /* in, physical addr of jump address at end of stream */
	__u32 nr_bos;                       /* in, number of submitted stream bo handles */
	__u64 bos;                          /* in, ptr to array of submitted stream bo handles */
};

#define DNX_WAIT_NONBLOCK      0x01
struct drm_dnx_wait_fence {
	__u32 fence;                        /* in */
	__u32 flags;                        /* in, mask of DNX_WAIT_x */
	struct drm_dnx_timespec timeout;
};


/*
 * GEM buffers:
 */

#define DNX_BO_CACHE_MASK   0x000f0000
/* cache modes */
#define DNX_BO_CACHED       0x00010000
#define DNX_BO_WC           0x00020000
#define DNX_BO_UNCACHED     0x00040000

struct drm_dnx_gem_new {
	__u64 size;           /* in */
	__u32 flags;          /* in, mask of DNX_BO_x */
	__u32 handle;         /* out */
	__u64 paddr;          /* out, physical address of contiguous buffer */
};

struct drm_dnx_gem_info {
	__u32 handle;         /* in */
	__u32 pad;
	__u64 offset;         /* out, offset to pass to mmap() */
};

struct drm_dnx_gem_user {
	__u32 handle;         /* in */
	__u32 pad;
	__u64 paddr;          /* out, physical address of contiguous buffer */
};

#define DNX_PREP_READ        0x01
#define DNX_PREP_WRITE       0x02
#define DNX_PREP_NOSYNC      0x04

struct drm_dnx_gem_cpu_prep {
	__u32 handle;         /* in */
	__u32 op;             /* in, mask of DNX_PREP_x */
	struct drm_dnx_timespec timeout;   /* in */
};

struct drm_dnx_gem_cpu_fini {
	__u32 handle;         /* in */
	__u32 flags;          /* in, placeholder for now, no defined values */
};


#define DRM_DNX_GET_REG         0x00
#define DRM_DNX_SET_REG         0x01
#define DRM_DNX_SELF_TEST       0x02
#define DRM_DNX_RESET           0x03
#define DRM_DNX_STREAM_SUBMIT   0x04
#define DRM_DNX_WAIT_FENCE      0x05
#define DRM_DNX_GEM_NEW         0x06
#define DRM_DNX_GEM_INFO        0x07
#define DRM_DNX_GEM_USER        0x08
#define DRM_DNX_GEM_CPU_PREP    0x09
#define DRM_DNX_GEM_CPU_FINI    0x0A
#define DRM_DNX_NUM_IOCTLS      0x0B


#define DRM_IOCTL_DNX_GET_REG           DRM_IOR (DRM_COMMAND_BASE + DRM_DNX_GET_REG, struct drm_dnx_reg)
#define DRM_IOCTL_DNX_SET_REG           DRM_IOW (DRM_COMMAND_BASE + DRM_DNX_SET_REG, struct drm_dnx_reg)
#define DRM_IOCTL_DNX_SELF_TEST         DRM_IOR (DRM_COMMAND_BASE + DRM_DNX_SELF_TEST, __u32)
#define DRM_IOCTL_DNX_RESET             DRM_IO  (DRM_COMMAND_BASE + DRM_DNX_RESET)
#define DRM_IOCTL_DNX_STREAM_SUBMIT     DRM_IOWR(DRM_COMMAND_BASE + DRM_DNX_STREAM_SUBMIT, struct drm_dnx_stream_submit)
#define DRM_IOCTL_DNX_WAIT_FENCE        DRM_IOWR(DRM_COMMAND_BASE + DRM_DNX_WAIT_FENCE, struct drm_dnx_wait_fence)
#define DRM_IOCTL_DNX_GEM_NEW           DRM_IOWR(DRM_COMMAND_BASE + DRM_DNX_GEM_NEW, struct drm_dnx_gem_new)
#define DRM_IOCTL_DNX_GEM_INFO          DRM_IOWR(DRM_COMMAND_BASE + DRM_DNX_GEM_INFO, struct drm_dnx_gem_info)
#define DRM_IOCTL_DNX_GEM_USER          DRM_IOWR(DRM_COMMAND_BASE + DRM_DNX_GEM_USER, struct drm_dnx_gem_user)
#define DRM_IOCTL_DNX_GEM_CPU_PREP      DRM_IOW (DRM_COMMAND_BASE + DRM_DNX_GEM_CPU_PREP, struct drm_dnx_gem_cpu_prep)
#define DRM_IOCTL_DNX_GEM_CPU_FINI      DRM_IOW (DRM_COMMAND_BASE + DRM_DNX_GEM_CPU_FINI, struct drm_dnx_gem_cpu_fini)

#endif

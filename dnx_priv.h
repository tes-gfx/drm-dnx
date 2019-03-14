#ifndef __DNX_PRIV_H__
#define __DNX_PRIV_H__


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>
#include <xf86drm.h>

#include "dnx_drmif.h"
#include "dnx_drm.h"


#define VOID2U64(x) ((uint64_t)(unsigned long)(x))
#define U642VOID(x) ((void*)(unsigned long)(x))


struct dnx_device;


struct dnx_device {
	int fd;

	uint32_t *mem_io;

	/*
	 * This submit buffer is used to collect the BOs of a single stream from
	 * the DNX driver. We assume here, that there are no concurrent calls to
	 * stream_submit for a device object.
	 * That means, either the call must be locked from outside or each thread
	 * must have its own dnx_device object.
	 */
	struct {
		uint32_t *bos;
		int bo_nr, bo_max;
	} submit;
};


/* a GEM buffer object allocated from the DRM device */
struct dnx_bo_priv {
	struct dnx_bo     base;
	struct dnx_device *dev;
	void              *map;           /* userspace mmap'ing (if there is one) */
	uint32_t          size;
	uint32_t          handle;
	uint32_t          flags;
	uint64_t          offset;         /* offset to mmap() */
};


struct dnx_stream_priv {
	struct dnx_stream base;

	struct dnx_device *device;

	uint32_t last_timestamp;
};


#include <stdio.h>
static inline void get_abs_timeout(struct drm_dnx_timespec *tv, uint64_t ns)
{
	struct timespec t;
	uint32_t s = ns / 1000000000ull;
	clock_gettime(CLOCK_MONOTONIC, &t);
	tv->tv_sec = t.tv_sec + s;
	tv->tv_nsec = t.tv_nsec + ns - (s * 1000000000ull);
}


/* debug stuff:
 */

#define enable_debug 1

#define INFO_MSG(fmt, ...) \
		do { drmMsg("[I] "fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)
#define DEBUG_MSG(fmt, ...) \
		do if (enable_debug) { drmMsg("[D] "fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)
#define WARN_MSG(fmt, ...) \
		do { drmMsg("[W] "fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)
#define ERROR_MSG(fmt, ...) \
		do { drmMsg("[E] " fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)

#endif

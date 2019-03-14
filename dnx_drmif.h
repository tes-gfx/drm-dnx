#ifndef __DNX_DRMIF_H__
#define __DNX_DRMIF_H__


#include <xf86drm.h>
#include <stdint.h>


struct dnx_device;
struct dnx_stream;
struct dnx_bo;


/* device functions:
 */

struct dnx_device *dnx_device_new(int fd);
void dnx_device_del(struct dnx_device *dev);
int dnx_device_fd(struct dnx_device *dev);
uint32_t *dnx_device_mem_io(struct dnx_device *dev);


/* buffer-object functions:
 */

struct dnx_bo *dnx_bo_new(struct dnx_device *dev,
		uint32_t size, uint32_t flags);
void *dnx_bo_map(struct dnx_bo *bo);
void dnx_bo_del(struct dnx_bo *bo);
struct dnx_bo *dnx_bo_from_dmabuf(struct dnx_device *dev, int fd);


/* debug functions:
 */

int dnx_debug_selftest(struct dnx_device *dev);
void dnx_debug_reset(struct dnx_device *dev);


/* stream functions:
 */

struct dnx_stream {
	void *stream;           /* ptr to stream data */
	void *jump;             /* ptr to jump address at stream's end */
};

/* todo: integrate with dnx_priv_bo and hide everything to user side,
 * use functions instead to get values */
struct dnx_bo {
	void *paddr;
	void *map;
	uint32_t handle;
};

struct dnx_stream *dnx_stream_new(struct dnx_device *device);
void dnx_stream_del(struct dnx_stream *stream);
void dnx_stream_append_bo(struct dnx_device *device, struct dnx_bo *bo);
void dnx_stream_flush(struct dnx_stream *stream);
int dnx_stream_finish(struct dnx_stream *stream);
uint32_t dnx_stream_timestamp(struct dnx_stream *stream);


#endif

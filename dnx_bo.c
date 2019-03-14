#define _FILE_OFFSET_BITS 64
#include "libdrm_macros.h"

#include "dnx_priv.h"
#include "dnx_drmif.h"

#include <unistd.h>


/* todo: Implement a caching for bos. Reuse deleted bos to reduce kernel calls */


static inline struct dnx_bo_priv *
dnx_bo_priv(struct dnx_bo *bo)
{
    return (struct dnx_bo_priv *) bo;
}


/* allocate a new buffer object, call w/ table_lock held */
static struct dnx_bo *bo_from_handle(struct dnx_device *dev,
		uint32_t size, uint32_t handle, uint32_t flags, void *paddr)
{
	struct dnx_bo_priv *bo = calloc(sizeof(*bo), 1);

	if (!bo) {
		struct drm_gem_close req = {
			.handle = handle,
		};

		drmIoctl(dev->fd, DRM_IOCTL_GEM_CLOSE, &req);

		return NULL;
	}

	bo->dev = dev;
	bo->size = size;
	bo->handle = handle;
	bo->flags = flags;
	bo->base.paddr = paddr;
	bo->base.handle = handle;

	return &bo->base;
}


/* allocate a new (un-tiled) buffer object */
struct dnx_bo *dnx_bo_new(struct dnx_device *dev, uint32_t size,
		uint32_t flags)
{
	struct dnx_bo *bo;
	int ret;
	struct drm_dnx_gem_new req = {
			.flags = flags,
			.size = size,
	};

	ret = drmCommandWriteRead(dev->fd, DRM_DNX_GEM_NEW,
			&req, sizeof(req));
	if (ret)
		return NULL;

	bo = bo_from_handle(dev, size, req.handle, flags, U642VOID(req.paddr));

	return bo;
}


/* get buffer info */
static int get_buffer_info(struct dnx_bo_priv *bo)
{
	int ret;
	struct drm_dnx_gem_info req = {
		.handle = bo->handle,
	};

	ret = drmCommandWriteRead(bo->dev->fd, DRM_DNX_GEM_INFO,
			&req, sizeof(req));
	if (ret) {
		return ret;
	}

	/* really all we need for now is mmap offset */
	bo->offset = req.offset;

	return 0;
}


/* map a bo to user space */
void *dnx_bo_map(struct dnx_bo *bo)
{
	struct dnx_bo_priv *priv = dnx_bo_priv(bo);

	if (!priv->map) {
		if (!priv->offset) {
			get_buffer_info(priv);
		}

		priv->map = drm_mmap(0, priv->size, PROT_READ | PROT_WRITE,
				MAP_SHARED, priv->dev->fd, priv->offset);
		if (priv->map == MAP_FAILED) {
			ERROR_MSG("mmap failed: %s", strerror(errno));
			priv->map = NULL;
		}
	}

	bo->map = priv->map;

	return priv->map;
}


/* get buffer info required by user space driver */
static int get_buffer_user(struct dnx_device *dev, uint32_t handle, void** paddr)
{
	int ret;
	struct drm_dnx_gem_user req = {
		.handle = handle,
	};

	ret = drmCommandWriteRead(dev->fd, DRM_DNX_GEM_USER,
			&req, sizeof(req));
	if (ret) {
		return ret;
	}

	*paddr = U642VOID(req.paddr);

	return 0;
}


/* import a buffer from dmabuf fd, does not take ownership of the
 * fd so caller should close() the fd when it is otherwise done
 * with it (even if it is still using the 'struct dnx_bo *')
 */
struct dnx_bo *dnx_bo_from_dmabuf(struct dnx_device *dev, int fd)
{
	struct dnx_bo *bo;
	int ret, size;
	uint32_t handle;
	void *paddr = NULL;

	ret = drmPrimeFDToHandle(dev->fd, fd, &handle);
	if (ret) {
		return NULL;
	}

	/* lseek() to get bo size */
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_CUR);

	get_buffer_user(dev, handle, &paddr);
	bo = bo_from_handle(dev, size, handle, 0, paddr);

	return bo;
}


drm_private void bo_del(struct dnx_bo_priv *bo)
{
	if (bo->map)
		drm_munmap(bo->map, bo->size);

	if (bo->handle) {
		struct drm_gem_close req = {
			.handle = bo->handle,
		};

		drmIoctl(bo->dev->fd, DRM_IOCTL_GEM_CLOSE, &req);
	}

	free(bo);
}


/* destroy a buffer object */
void dnx_bo_del(struct dnx_bo *bo)
{
	struct dnx_bo_priv *priv = dnx_bo_priv(bo);

	if (!priv)
		return;

	bo_del(priv);
}


int dnx_bo_cpu_prep(struct dnx_bo *bo, uint32_t op)
{
	struct dnx_bo_priv *priv = dnx_bo_priv(bo);
	struct drm_dnx_gem_cpu_prep req = {
		.handle = priv->handle,
		.op = op,
	};

	get_abs_timeout(&req.timeout, 5000000000);

	return drmCommandWrite(priv->dev->fd, DRM_DNX_GEM_CPU_PREP,
			&req, sizeof(req));
}


void dnx_bo_cpu_fini(struct dnx_bo *bo)
{
	struct dnx_bo_priv *priv = dnx_bo_priv(bo);
	struct drm_dnx_gem_cpu_fini req = {
		.handle = priv->handle,
	};

	drmCommandWrite(priv->dev->fd, DRM_DNX_GEM_CPU_FINI,
			&req, sizeof(req));
}

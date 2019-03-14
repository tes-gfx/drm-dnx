#include <stdlib.h>
#include <errno.h>

#if !defined(_WIN32) && !defined(_WIN64) && !defined(LXSS)

#include <sys/mman.h>
#include <xf86drm.h>

#include "nx_register_address.h"

#include "dnx_priv.h"


struct dnx_device *dnx_device_new(int fd)
{
	struct dnx_device *dev = calloc(sizeof(*dev), 1);

	if(!dev) {
		ERROR_MSG("could not create device object");
		return NULL;
	}

	dev->fd = fd;

	dev->mem_io    = (uint32_t *) mmap(0, DNX_REG_COUNT * 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if(dev->mem_io    == MAP_FAILED) {
		ERROR_MSG("could not map io memory");
		free(dev);
		return NULL;
	}

	return dev;
}


void dnx_device_del(struct dnx_device *dev)
{
	if(dev->submit.bos)
		free(dev->submit.bos);
	free(dev);
}


int dnx_device_fd(struct dnx_device *dev)
{
	return dev->fd;
}

uint32_t *dnx_device_mem_io(struct dnx_device *dev)
{
	return dev->mem_io;
}

#endif

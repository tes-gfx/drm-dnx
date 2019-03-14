#if !defined(_WIN32) && !defined(_WIN64) && !defined(LXSS)
#include <sys/ioctl.h>

#include "dnx_priv.h"
#include "dnx_drm.h"

int dnx_debug_selftest(struct dnx_device *dev)
{
	int err;
        uint32_t result;

        err = ioctl(dev->fd, DRM_IOCTL_DNX_SELF_TEST, &result);

        if(result) {
                return -1;
        }

        return err;
}

void dnx_debug_reset(struct dnx_device *dev)
{
	int err;

        err = ioctl(dev->fd, DRM_IOCTL_DNX_RESET, NULL);

	if(err)
		ERROR_MSG("reset failed: %d (%s)", err, strerror(errno));
}
#endif

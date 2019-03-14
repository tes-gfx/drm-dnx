#include "dnx_priv.h"
#include "dnx_drm.h"


static inline struct dnx_stream_priv *
dnx_stream_priv(struct dnx_stream *stream)
{
    return (struct dnx_stream_priv *) stream;
}


struct dnx_stream *dnx_stream_new(struct dnx_device *device)
{
	struct dnx_stream_priv *stream = NULL;

	stream = calloc(1, sizeof(*stream));
	if(!stream) {
		ERROR_MSG("allocation failed");
		goto fail;
	}

	stream->device = device;

	return &stream->base;

fail:
	if(stream)
		dnx_stream_del(&stream->base);
	return NULL;
}


void dnx_stream_append_bo(struct dnx_device *device, struct dnx_bo *bo)
{
	struct dnx_bo_priv *priv = (struct dnx_bo_priv *) bo;

	if((device->submit.bo_nr + 1) > device->submit.bo_max) {
		if(device->submit.bo_max == 0) {
			device->submit.bo_max = 1;
		}
		else {
			device->submit.bo_max *= 2;
		}
		device->submit.bos = realloc(device->submit.bos, device->submit.bo_max * sizeof(priv->handle));
	}

	device->submit.bos[device->submit.bo_nr++] = priv->handle;

	INFO_MSG("added bo %p\n", bo);
}


void dnx_stream_del(struct dnx_stream *stream)
{
	struct dnx_stream_priv *priv = dnx_stream_priv(stream);

	free(priv);
}


static void dnx_reset_submit_buffer(struct dnx_device *device) {
	device->submit.bo_nr = 0;
	INFO_MSG("resetting submit buffer\n");
}

void dnx_stream_flush(struct dnx_stream *stream)
{
	struct dnx_stream_priv *priv = dnx_stream_priv(stream);
	struct dnx_device *device = priv->device;
	int ret;

	struct drm_dnx_stream_submit req = {
			.stream = VOID2U64(stream->stream),
			.jump = VOID2U64(stream->jump),
			.nr_bos = device->submit.bo_nr,
			.bos = VOID2U64(device->submit.bos),
	};

	ret = drmCommandWriteRead(device->fd, DRM_DNX_STREAM_SUBMIT, &req, sizeof(req));
	if (ret)
		ERROR_MSG("submit failed %d (%s)", ret, strerror(errno));
	else {
		priv->last_timestamp = req.fence;
		dnx_reset_submit_buffer(device);
	}
}


int wait_fence(struct dnx_device *device, uint32_t timestamp, uint32_t ms)
{
	int ret;
	uint64_t ns = ms * 1000000ull;

	struct drm_dnx_wait_fence req = {
			.fence = timestamp,
	};

	if (ns == 0)
		req.flags |= DNX_WAIT_NONBLOCK;
	else
		req.flags = 0;

	get_abs_timeout(&req.timeout, ns);

	ret = drmCommandWriteRead(device->fd, DRM_DNX_WAIT_FENCE, &req, sizeof(req));
	if (ret) {
		ERROR_MSG("submit failed %d (%s)", ret, strerror(errno));
		return ret;
	}

	return 0;
}


int dnx_stream_finish(struct dnx_stream *stream)
{
	int ret;
	struct dnx_stream_priv *priv = dnx_stream_priv(stream);

	dnx_stream_flush(stream);
	ret = wait_fence(priv->device, priv->last_timestamp, 5000);
	if (ret) {
		ERROR_MSG("stream finish wait fence failed %d (%s)", ret, strerror(errno));
		return ret;
	}

	return 0;
}


uint32_t dnx_stream_timestamp(struct dnx_stream *stream)
{
	return dnx_stream_priv(stream)->last_timestamp;
}

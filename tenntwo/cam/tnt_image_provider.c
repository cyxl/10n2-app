#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <nuttx/video/video.h>
#include <arch/board/cxd56_imageproc.h>

#include "tnt_image_provider.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define IMAGE_WIDTH (320)
#define IMAGE_HEIGHT (240)
#define IMAGE_DATASIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 2) /* QVGA YUV422/RGB565 */

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int video_fd = -1;
static unsigned char *frame_mem;
static unsigned char *clip_mem;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: camera_prepare()
 *
 * Description:
 *   Allocate frame buffer for camera and Queue the allocated buffer
 *   into video driver.
 ****************************************************************************/

static int camera_prepare(enum v4l2_buf_type type,
                          uint32_t buf_mode, uint32_t pixformat,
                          uint16_t hsize, uint16_t vsize,
                          uint16_t clip_hsize, uint16_t clip_vsize)
{
  int ret;
  struct v4l2_format fmt = {0};
  struct v4l2_requestbuffers req = {0};

  /* VIDIOC_REQBUFS initiate user pointer I/O */

  req.type = type;
  req.memory = V4L2_MEMORY_USERPTR;
  req.count = 1;
  req.mode = buf_mode;

  ret = ioctl(video_fd, VIDIOC_REQBUFS, (unsigned long)&req);
  if (ret < 0)
  {
    printf("Failed to VIDIOC_REQBUFS: errno = %d\n", errno);
    return ret;
  }

  /* VIDIOC_S_FMT set format */

  fmt.type = type;
  fmt.fmt.pix.width = hsize;
  fmt.fmt.pix.height = vsize;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  fmt.fmt.pix.pixelformat = pixformat;

  ret = ioctl(video_fd, VIDIOC_S_FMT, (unsigned long)&fmt);
  if (ret < 0)
  {
    printf("Failed to VIDIOC_S_FMT: errno = %d\n", errno);
    return ret;
  }

  /* Prepare video memory to store images */

  frame_mem = (unsigned char *)memalign(32, hsize * vsize * 2);
  if (!frame_mem)
  {
    return -1;
  }

  clip_mem = (unsigned char *)memalign(32, clip_hsize * clip_vsize * 2);
  if (!clip_mem)
  {
    free(frame_mem);
    return -1;
  }

  /* VIDIOC_STREAMON start stream */

  ret = ioctl(video_fd, VIDIOC_STREAMON, (unsigned long)&type);
  if (ret < 0)
  {
    printf("Failed to VIDIOC_STREAMON: errno = %d\n", errno);
    free(frame_mem);
    return ret;
  }

  return 0;
}

/****************************************************************************
 * Name: get_camimage()
 *
 * Description:
 *   DQBUF camera frame buffer from video driver with taken picture data.
 ****************************************************************************/

static unsigned char *get_camimage(void)
{
  int ret;
  struct v4l2_buffer v4l2_buf;

  /* VIDIOC_DQBUF acquires captured data. */

  memset(&v4l2_buf, 0, sizeof(v4l2_buf));
  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_buf.memory = V4L2_MEMORY_USERPTR;

  ret = ioctl(video_fd, VIDIOC_DQBUF, (unsigned long)&v4l2_buf);
  if (ret)
  {
    printf("Fail DQBUF %d\n", errno);
    return NULL;
  }

  return (unsigned char *)v4l2_buf.m.userptr;
}

/****************************************************************************
 * Name: release_camimage()
 *
 * Description:
 *   Re-QBUF to set used frame buffer into video driver.
 ****************************************************************************/

static int set_camimage(void)
{
  struct v4l2_buffer buf = {0};

  /* VIDIOC_QBUF sets buffer pointer into video driver again. */

  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_USERPTR;
  buf.index = 0;
  buf.m.userptr = (unsigned long)frame_mem;
  buf.length = IMAGE_DATASIZE;

  if (ioctl(video_fd, VIDIOC_QBUF, (unsigned long)&buf) != 0)
  {
    printf("Fail QBUF %d\n", errno);
    return ERROR;
  }
  return OK;
}

/****************************************************************************
 * Name: init_video()
 *
 * Description:
 *   Initialize video sub-system.
 ****************************************************************************/

static int init_video(uint16_t clip_hsize, uint16_t clip_vsize)
{
  int ret;

  if (video_fd >= 0)
  {
    return 0;
  }

  if (video_initialize("/dev/video") != 0)
  {
    return -1;
  }

  video_fd = open("/dev/video", 0);
  if (video_fd < 0)
  {
    video_uninitialize();
    return -1;
  }

  ret = camera_prepare(V4L2_BUF_TYPE_VIDEO_CAPTURE,
                       V4L2_BUF_MODE_RING, V4L2_PIX_FMT_RGB565,
                       VIDEO_HSIZE_QVGA, VIDEO_VSIZE_QVGA,
                       clip_hsize, clip_vsize);
  if (ret != OK)
  {
    close(video_fd);
    free(frame_mem);
    video_uninitialize();
    video_fd = -1;
    return -1;
  }

  imageproc_initialize();

  return 0;
}

void camera_clean()
{

  if (frame_mem != NULL)
  {
    free(frame_mem);
    frame_mem = NULL;
  }
  if (clip_mem != NULL)
  {
    free(clip_mem);
    clip_mem = NULL;
  }
}
int getimage(unsigned char *out_data, uint16_t clip_hsize, uint16_t clip_vsize, bool color)
{
  unsigned char *mem;
  const imageproc_rect_t clip_rect = {
    .x1 = (IMAGE_WIDTH / 2) - (clip_hsize/2),
    .y1 = (IMAGE_HEIGHT / 2) - (clip_vsize/2),
    .x2 = (IMAGE_WIDTH / 2) + (clip_hsize/2) - 1,
    .y2 = (IMAGE_HEIGHT / 2) + (clip_vsize/2) - 1};

  if (init_video(clip_hsize, clip_vsize) < 0)
  {
    printf("init_video() is faled..\n");
    return -1;
  }

  if (set_camimage() != OK)
  {
    printf("set_camimage() is faled..\n");
    return -1;
  }

  mem = get_camimage();
  if (mem != NULL)
  {
    printf("Captured image now.\n");
    if (imageproc_clip_and_resize(mem, IMAGE_WIDTH, IMAGE_HEIGHT,
                                  clip_mem, clip_hsize, clip_vsize,
                                  16 /*YUV422*/, (imageproc_rect_t *)&clip_rect) == 0)
    {
      printf("converting...\n");
      if (!color)
      {
        imageproc_convert_yuv2gray(clip_mem, out_data,
                                   clip_hsize, clip_vsize);
      }
      else{
        memcpy(out_data,clip_mem,clip_hsize * clip_vsize * 2);
      }
      printf("done converting...\n");
      return 0;
    }
    else
    {
      printf("imageproc_clip_and_resize() is failed.\n");
    }
    // undo camera_prepare()
    camera_clean();
  }
  else
  {
    printf("get_camimage() is faled.\n");
  }

  return -1;
}

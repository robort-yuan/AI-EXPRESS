//
// Copyright 2018 Horizon Robotics.
//

#ifndef INCLUDE_GDCPLUGIN_STITCH_IMAGE_H_
#define INCLUDE_GDCPLUGIN_STITCH_IMAGE_H_

#ifdef __GNUC__
#ifdef ADAS_EXPORT
#define ADAS_API __attribute__ ((visibility("default")))
#else
#define ADAS_API
#endif
#define __stdcall
#elif defined(_WIN32)
#ifdef ADAS_EXPORT
#define ADAS_API __declspec(dllexport)
#else
#define ADAS_API __declspec(dllimport)
#endif
#else
#error "No platform specified!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  INIT_OK = 0,
  INIT_ERROR_RANGE = 1,
  INIT_ERROR_IMAGE_SIZE = 2,
  INIT_ERROR_CONFIG_FORMAT = 3,
  INIT_ERROR_CONFIG_CAMERA = 4,
  INIT_ERROR_REPEATEDLY_INIT = 5,
  INIT_ERROR_IPMINIT = 6
}INIT_STATUS;

typedef enum {
  IMAGE_LEFT = 0,
  IMAGE_RIGHT = 1,
  IMAGE_BACK = 2,
  IMAGE_FRONT = 3
}IMAGE_DIRECTION;

typedef enum {
  STITCH_OK = 0,
  STITCH_ERROR_INPUT = 1,
  STITCH_ERROR_DIRECTION = 2,
  STITCH_ERROR_OUTPUT = 3
}STITCH_STATUS;

typedef enum {
  DESTROY_OK = 0,
  DESTROY_ERROR = 1
}DESTROY_STATUS;

typedef enum {
  HOBOT_IMAGE_GRAY = 0,
  HOBOT_IMAGE_RGB = 1,
  HOBOT_IMAGE_NV12 = 2,
  HOBOT_IMAGE_NNLABEL = 3
}HOBOT_IMAGE_TYPE;

typedef enum {
  IMAGE_INTER_NEAREST = 0,
  IMAGE_INTER_LINEAR = 1
}IMAGE_INTER_TYPE;

typedef struct {
  unsigned char *data_;
  IMAGE_DIRECTION image_direction_;
}HOBOT_IMAGE;

typedef enum {
  GET_IPM_OKAY = 0,
  GET_IPM_INVALID_ARGS = 1
}GET_IPM_IMAGE_STATUS;

ADAS_API int Init(const char *config_file0,
         const char *config_file1,
         const char *config_file2,
         const char *config_file3,
         int input_image_width,
         int input_image_height,
         HOBOT_IMAGE_TYPE input_image_type,
         int output_image_width,
         int output_image_height,
         float output_range_head,
         float output_range_rear,
         int blend_stride = 10,
         IMAGE_INTER_TYPE inter_type = IMAGE_INTER_LINEAR);

ADAS_API int InitIPM(
  const char *config_file0,
  const char *config_file1,
  const char *config_file2,
  const char *config_file3,
  const int target_width,
  const int target_height);

ADAS_API int Stitch(HOBOT_IMAGE *input_image0,
           HOBOT_IMAGE *input_image1,
           HOBOT_IMAGE *input_image2,
           HOBOT_IMAGE *input_image3,
           HOBOT_IMAGE *output_image);

ADAS_API int GetIPMImage(HOBOT_IMAGE *input_image0,
  HOBOT_IMAGE *output_image,
  const int input_width, const int input_height,
  const int target_width, const int target_height);

ADAS_API int Destroy();


#ifdef __cplusplus
}
#endif

#endif  // INCLUDE_GDCPLUGIN_STITCH_IMAGE_H_

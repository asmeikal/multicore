#ifndef __HOMEWORK_UTILS_H
#define __HOMEWORK_UTILS_H

#include <mlclut.h>

int clut_blurImage(const cl_device_id device, const char * const filename, const unsigned int filter_size);
int clut_blurImage_local(const cl_device_id device, const char * const filename, const unsigned int filter_size);
int clut_blurImage_local_unlimited(const cl_device_id device, const char * const filename, const unsigned int filter_size);

#endif


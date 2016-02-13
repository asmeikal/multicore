#include <stdlib.h>
#include <stdio.h>

#include <Debug.h>

#include "homework_utils.h"

#include <mlclut_descriptions.h>
#include <mlclut_images.h>

#define DEBUG_HOMEWORK		"openCL_homework_debug"

static unsigned char *createFilterMatrix(const unsigned int filter_size);
static void printFilterMatrix(const unsigned char * const filter_matrix, const unsigned int filter_size);
static int clut_getOptimalWGSize(const cl_device_id device, size_t * const dim1, size_t * const dim2, const unsigned int filter_size, const unsigned int elem_size);
static int clut_getMaxWGSize(const cl_device_id device, size_t * const dim1, size_t * const dim2);

/*!
 * @function createFilterMatrix
 * Creates a filter matrix of unsigned chars, as defined by the homework
 * requirements. The matrix contains only 1s and 0s.
 * @param filter_size
 * The size of the edge of the matrix.
 * @return
 * A pointer to a newly allocated [filter_size] x [filter_size] filter matrix,
 * or NULL on failure.
 */
static unsigned char *createFilterMatrix(const unsigned int filter_size)
{
	const char * const fname = "createFilterMatrix";
	unsigned char *result = NULL;

	if (0 == filter_size) {
		Debug_out(DEBUG_HOMEWORK, "%s: Filter size is too small.\n", fname);
		goto error1;
	}
	if (0 == (filter_size % 2)) {
		Debug_out(DEBUG_HOMEWORK, "%s: Filter size must be odd.\n", fname);
		goto error1;
	}

	result = calloc(filter_size * filter_size, sizeof(unsigned char));
	if (NULL == result) {
		Debug_out(DEBUG_HOMEWORK, "%s: Calloc failed.\n", fname);
		goto error1;
	}

	unsigned int n_ones, n_zeros;
	unsigned int i, j, d = (filter_size - 1) / 2;
	n_ones = 1;
	n_zeros = d;
	for (i = 0; i < filter_size; ++i) {
		for (j = 0; j < filter_size; ++j) {
			if (j < n_zeros) {
				result[i * filter_size + j] = 0;
			} else if (j < n_zeros + n_ones) {
				result[i * filter_size + j] = 1;
			} else {
				result[i * filter_size + j] = 0;
			}
		}
		if (i < d) {
			n_ones += 2;
			n_zeros -= 1;
		} else {
			n_ones -= 2;
			n_zeros += 1;
		}
	}

error1:
	return result;
}

/*!
 * @function printFilterMatrix
 * Prints the [filter_size] x [filter_size] filter matrix at [filter_matrix].
 * @param filter_matrix
 * An unsigned char filter matrix.
 * @param filter_size
 * The filter matrix edge.
 */
static void printFilterMatrix(const unsigned char * const filter_matrix, const unsigned int filter_size)
{
	if (NULL == filter_matrix) {
		Debug_out(DEBUG_HOMEWORK, "printFilterMatrix: NULL pointer argument.\n");
	}

	unsigned int i, j;
	for (i = 0; i < filter_size; ++i) {
		for (j = 0; j < filter_size; ++j) {
			printf("%d ", filter_matrix[i * filter_size + j]);
		}
		printf("\n");
	}
}

static int clut_SUPPORT(
		size_t * const dim1,
		size_t * const dim2,
		const size_t width,
		const size_t height,
		const size_t wg_size,
		const size_t dev_mem_size,
		const unsigned int filter_size,
		const unsigned int elem_size
	)
{
	const char * const fname = "clut_SUPPORT";

	/* Find optimal work group size using a table T[result1][result2].
	 * T[i][j] indicates the max number of items that fit into a work group
	 * of size (i+1) x (j+1).
	 * M = max work group size
	 * N = local mem size
	 * T[i][j] is 0 if (i+1) x (j+1) > M (the max work group size supported)
	 * or if (i+f) x (j+f) > N / elem_size (the max number of elements that
	 * fit into the local memory).
	 * T[i][j] = max(T[i][j], T[i-1][j], T[i][j-1])
	 */
	size_t din_matrix[width][height];

	if ((0 == width) || (0 == height)) {
		Debug_out(DEBUG_HOMEWORK, "%s: illegal table size.\n", fname);
		goto error1;
	}

	size_t i, j;
	for (i = 0; i < width; ++i) {
		for (j = 0; j < height; ++j) {
			if ((i + 1) * (j + 1) > wg_size) {
				din_matrix[i][j] = 0;
			} else if ((i + filter_size) * (j + filter_size) > (dev_mem_size / elem_size)) {
				din_matrix[i][j] = 0;
			} else {
				din_matrix[i][j] = (i + filter_size) * (j + filter_size);
			}
			if ((i > 0) && (din_matrix[i][j] < din_matrix[i-1][j])) {
				din_matrix[i][j] = din_matrix[i-1][j];
			}
			if ((j > 0) && (din_matrix[i][j] < din_matrix[i][j-1])) {
				din_matrix[i][j] = din_matrix[i][j-1];
			}
		}
	}

	/* walk back and up on the table from the lower right corner */
	i = width - 1;
	j = height - 1;
	if (0 == din_matrix[i][j]) {
		Debug_out(DEBUG_HOMEWORK, "%s: Last cell of the table is 0: no solution found.\n", fname);
		goto error1;
	}

	/* I'm trying to move up and right in a balanced way */
	do {
		if ((din_matrix[i][j] == din_matrix[i][j-1]) && (din_matrix[i][j] == din_matrix[i-1][j])) {
			if (j > i) {
				--j;
			} else {
				--i;
			}
		} else if (din_matrix[i][j] == din_matrix[i][j-1]) {
			--j;
		} else if (din_matrix[i][j] == din_matrix[i-1][j]) {
			--i;
		}
	} while ((din_matrix[i][j] == din_matrix[i][j-1]) || (din_matrix[i][j] == din_matrix[i-1][j]));

	if (NULL == dim1) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}
	if (NULL == dim2) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}

	*dim1 = i + 1;
	*dim2 = j + 1;

	return 0;

error1:
	return 1;

}

/*!
 * @function clut_getOptimalWGSize
 * Finds the "optimal" work group size for applying a blur filter with the local
 * version of the kernel.
 * @param device
 * The device on which the kernel will be executed.
 * @param dim1
 * A pointer to a size_t where the first dimension will be stored.
 * @param dim2
 * A pointer to a size_t where the second dimension will be stored.
 * @param filter_size
 * The filter size.
 * @param elem_size
 * The size of each pixel in the image.
 * @return
 * 0 on success, non-0 on failure.
 */
static int clut_getOptimalWGSize(const cl_device_id device, size_t * const dim1, size_t * const dim2, const unsigned int filter_size, const unsigned int elem_size)
{
	const char * const fname = "clut_getOptimalWGSize";

	if (NULL == dim1) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}
	if (NULL == dim2) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}

	if (0 == filter_size) {
		Debug_out(DEBUG_HOMEWORK, "%s: Illegal filter size.\n", fname);
		goto error1;
	}
	if (0 == elem_size) {
		Debug_out(DEBUG_HOMEWORK, "%s: Illegal element size.\n", fname);
		goto error1;
	}

	size_t wg_size, result1, result2;
	unsigned long dev_mem_size;
	cl_int cl_ret;

	/* get max work group size and local memory size */
	cl_ret = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(wg_size), &wg_size, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get max work group size", error1);
	cl_ret = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(dev_mem_size), &dev_mem_size, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get local memory size", error1);

	Debug_out(DEBUG_HOMEWORK, "%s: Local memory size is %ld bytes.\n", fname, dev_mem_size);
	Debug_out(DEBUG_HOMEWORK, "%s: Max work group size is %zu.\n", fname, wg_size);

	/* get the max work group size */
	if (0 != clut_getMaxWGSize(device, &result1, &result2)) {
		goto error1;
	}


	if (0 != clut_SUPPORT(dim1, dim2, result1, result2, wg_size, dev_mem_size, filter_size, elem_size)) {
		goto error1;
	}

	Debug_out(DEBUG_HOMEWORK,
		"%s: Local memory fits %zu elements, we are allocating a matrix of size %zu.\n",
		fname,
		(dev_mem_size / elem_size),
		(*dim1 + filter_size - 1) * (*dim2 + filter_size - 1));

	return 0;

error1:
	Debug_out(DEBUG_HOMEWORK,
		"%s: Unable to find optimal work group size for filter size %d and element size %d.\n",
		fname,
		filter_size,
		elem_size);
	return -1;
}

/*!
 * @function clut_getMaxWGSize
 * Finds the max height and width of a work group with max work group size
 * M = max_work_group_size of [device].
 * I'm assuming M = 2^k for some k.
 * @param device
 * The device on which the kernel will be executed.
 * @param dim1
 * A pointer to a size_t where the first dimension will be stored.
 * @param dim2
 * A pointer to a size_t where the second dimension will be stored.
 * @return
 * 0 on success, non-0 on failure.
 */
static int clut_getMaxWGSize(const cl_device_id device, size_t * const dim1, size_t * const dim2)
{
	const char * const fname = "clut_getMaxWGSize";
	if (NULL == dim1) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}
	if (NULL == dim2) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}
	size_t wg_size, result1, result2;
	cl_int cl_ret;

	/* get max work group size and local memory size */
	cl_ret = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(wg_size), &wg_size, NULL);
	CLUT_CHECK_ERROR(cl_ret, "Unable to get max work group size", error1);

	Debug_out(DEBUG_HOMEWORK, "%s: Max work group size is %zu.\n", fname, wg_size);

	result1 = 1;
	result2 = 1;
	while (result1 * result2 < wg_size) {
		result1 = result1 * 2;
		result2 = result2 * 2;
	}
	while (result1 * result2 > wg_size) {
		result2 = result2 / 2;
	}
	if (0 == result1) {
		goto error1;
	}
	if (0 == result2) {
		goto error1;
	}

	*dim1 = result1;
	*dim2 = result2;

	return 0;

error1:
	Debug_out(DEBUG_HOMEWORK, "%s: Unable to find max work group size.\n", fname);
	return -1;
}

/*!
 * @function clut_blurImage
 * Blurs the image at [filename] with a filter of size [filter_size], and saves the result
 * to the file "output.png".
 * @param filename
 * The name of the file.
 * @param filter_size
 * The size of the blur filter.
 * @return
 * 0 on success, non-0 on failure.
 */
int clut_blurImage(const cl_device_id device, const char * const filename, const unsigned int filter_size)
{
	const char * const fname = "clut_blurImage";
	int return_value = 1;
	cl_int ret;

	if (NULL == filename) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}

	/* Create context */
	cl_context context = clCreateContext(NULL, 1, &device, clut_contextCallback, "clut_blurImage", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create context", error1);
	Debug_out(DEBUG_HOMEWORK, "%s: Created context successfully.\n", fname);

	/* Create program */
	cl_program program = clut_createProgramFromFile(context, "homework_global.cl", NULL);
	if (NULL == program) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to create program.\n", fname);
		goto error3;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Program created.\n", fname);

	/* Create kernel */
	cl_kernel kernel = clCreateKernel(program, "blurImage", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create kernel", error3);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel created.\n", fname);

	/* Create command_queue */
	cl_command_queue command_queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create command queue", error4);
	Debug_out(DEBUG_HOMEWORK, "%s: Command queue created.\n", fname);

	/* load source image */
	int width, height;
	cl_mem source_image = clut_loadImageFromFile(context, filename, &width, &height);
	if (NULL == source_image) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to read source image.\n", fname);
		goto error5;
	}

	if ((filter_size > (unsigned int) width) || (filter_size > (unsigned int) height)) {
		Debug_out(DEBUG_HOMEWORK, "%s: Filter does not fit in image.\n", fname);
		goto error6;
	}

	/* create destination image */
	cl_image_format image_format = {0, 0};

	cl_image_desc image_desc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
//	image_desc.image_width		= 0;
//	image_desc.image_height		= 0;
//	image_desc.image_depth		= 0; /* only for 3D images */
//	image_desc.image_array_size	= 0; /* only for image arrays */
//	image_desc.image_row_pitch	= 0;
//	image_desc.image_slice_pitch	= 0; /* only for 3D images */
//	image_desc.num_mip_levels	= 0; /* mandatory */
//	image_desc.num_samples		= 0; /* mandatory */
//	image_desc.buffer		= NULL; /* only for 1D image buffers */

	image_desc.image_width = width - filter_size + 1;
	image_desc.image_height = height - filter_size + 1;

	ret = clGetImageInfo(source_image, CL_IMAGE_FORMAT, sizeof(image_format), &image_format, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get source image format information", error6);

	cl_mem result_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &image_desc, NULL, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create second image", error6);

	Debug_out(DEBUG_HOMEWORK, "%s: Images created.\n", fname);

	/* create filter matrix */
	unsigned char *filter_matrix = createFilterMatrix(filter_size);
	if (NULL == filter_matrix) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to create filter matrix.\n", fname);
		goto error7;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Filter matrix created.\n", fname);
//	printFilterMatrix(filter_matrix, filter_size);

	/* copy filter matrix to device */
	cl_mem filter_matrix_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, filter_size * filter_size, filter_matrix, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create filter matrix buffer on device", error8);

	/* set kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(source_image), (void *) &source_image);
	CLUT_CHECK_ERROR(ret, "Unable to set source image argument", error9);
	Debug_out(DEBUG_HOMEWORK, "%s: Source image argument set.\n", fname);
	ret = clSetKernelArg(kernel, 1, sizeof(result_image), (void *) &result_image);
	CLUT_CHECK_ERROR(ret, "Unable to set result image argument", error9);
	Debug_out(DEBUG_HOMEWORK, "%s: Result image argument set.\n", fname);
	ret = clSetKernelArg(kernel, 2, sizeof(filter_size), (void *) &filter_size);
	CLUT_CHECK_ERROR(ret, "Unable to set filter size argument", error9);
	Debug_out(DEBUG_HOMEWORK, "%s: Filter size argument set.\n", fname);
	ret = clSetKernelArg(kernel, 3, sizeof(filter_matrix_buffer), (void *) &filter_matrix_buffer);
	CLUT_CHECK_ERROR(ret, "Unable to set filter matrix argument", error9);
	Debug_out(DEBUG_HOMEWORK, "%s: Filter matrix argument set.\n", fname);

	Debug_out(DEBUG_HOMEWORK, "%s: All kernel arguments set.\n", fname);

	/* run kernel */
	cl_event kernel_event;
	const size_t work_size[2] = { height - filter_size + 1, width - filter_size + 1};
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, work_size, NULL, 0, NULL, &kernel_event);
	CLUT_CHECK_ERROR(ret, "Unable to enqueue kernel", error9);

	ret = clFinish(command_queue);
	CLUT_CHECK_ERROR(ret, "Unable to finish commands in queue", error9);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel executed.\n", fname);
	ret = clWaitForEvents(1, &kernel_event);
	CLUT_CHECK_ERROR(ret, "Unable to wait for kernel event", error9);

	/* check that kernel executed correctly */
	cl_int kernel_ret;
	ret = clGetEventInfo(kernel_event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(kernel_ret), &kernel_ret, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get kernel status", error9);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel status is %d.\n", fname, kernel_ret);
	if (CL_COMPLETE != kernel_ret) {
		Debug_out(DEBUG_HOMEWORK, "%s: kernel execution failed: %s.\n", fname, clut_getErrorDescription(kernel_ret));
		goto error9;
	}

	cl_ulong end_time;
	ret = clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(end_time), &end_time, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get kernel event end time", error9);
	if (0 == end_time) {
		Debug_out(DEBUG_HOMEWORK, "%s: kernel execution took 0 seconds.\n", fname);
		goto error9;
	}

	cl_double time_double = clut_getEventDuration(kernel_event);
	cl_ulong time_ulong = clut_getEventDuration_ns(kernel_event);
	Debug_out(DEBUG_HOMEWORK, "%s: Blurring took %f seconds (%lld nanoseconds).\n", fname, time_double, time_ulong);

	/* save image */
	clut_saveImageToFile("output.png", command_queue, result_image);

	/* print filter size and duration in nanoseconds for profiling */
	printf("%d,%llu\n", filter_size, clut_getEventDuration_ns(kernel_event));

	return_value = 0;

error9:
	clReleaseMemObject(filter_matrix_buffer);
error8:
	free(filter_matrix);
error7:
	clReleaseMemObject(result_image);
error6:
	clReleaseMemObject(source_image);
error5:
	clReleaseCommandQueue(command_queue);
error4:
	clReleaseKernel(kernel);
error3:
	clReleaseProgram(program);
error2:
	clReleaseContext(context);
error1:
	return return_value;

}

/*!
 * @function clut_blurImage_local
 * Blurs the image at [filename] with a filter of size [filter_size], and saves the result
 * to the file "output_local.png". This function should be optimized to run on local memory.
 * @param filename
 * The name of the file.
 * @param filter_size
 * The size of the blur filter.
 * @return
 * 0 on success, non-0 on failure.
 */
int clut_blurImage_local(const cl_device_id device, const char * const filename, const unsigned int filter_size)
{
	const char * const fname = "clut_blurImage_local";
	int return_value = 1;
	cl_int ret;

	if (NULL == filename) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}

	/* compute work group size */
	size_t local_width, local_height;
	if (0 != clut_getOptimalWGSize(device, &local_width, &local_height, filter_size, 16)) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to get work group sizes.\n", fname);
		goto error1;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Max work group size is [%zu]x[%zu].\n", fname, local_width, local_height);

	/* openCL wants to know the size of __local statically allocated arrays at compile time,
	 * so the local size must be set with a #define */
	char *flags = calloc(128, 1);
	if (NULL == flags) {
		Debug_out(DEBUG_HOMEWORK, "%s: A calloc failed.\n", fname);
		goto error1;
	}
	sprintf(flags, "-D LOCAL_WIDTH=%zu -D LOCAL_HEIGHT=%zu -D FILTER_SIZE=%d", local_width, local_height, filter_size);
	Debug_out(DEBUG_HOMEWORK, "%s: Local flags are: '%s'.\n", fname, flags);

	/* Create context */
	cl_context context = clCreateContext(NULL, 1, &device, clut_contextCallback, "clut_blurImage_local", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create context", error2);
	Debug_out(DEBUG_HOMEWORK, "%s: Created context successfully.\n", fname);

	/* Create program */
	cl_program program = clut_createProgramFromFile(context, "homework_local.cl", flags);
	if (NULL == program) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to create program.\n", fname);
		goto error3;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Program created.\n", fname);

	/* Create kernel */
	cl_kernel kernel = clCreateKernel(program, "blurImage_local", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create kernel", error4);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel created.\n", fname);

	/* Create command_queue */
	cl_command_queue command_queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create command queue", error5);
	Debug_out(DEBUG_HOMEWORK, "%s: Command queue created.\n", fname);

	/* open source image */
	int width, height;
	cl_mem source_image = clut_loadImageFromFile(context, filename, &width, &height);
	if (NULL == source_image) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to read source image.\n", fname);
		goto error6;
	}

	if ((filter_size > (unsigned int) width) || (filter_size > (unsigned int) height)) {
		Debug_out(DEBUG_HOMEWORK, "%s: Filter does not fit in image.\n", fname);
		goto error7;
	}

	/* crate destination image */
	cl_image_format image_format = {0, 0};

	cl_image_desc image_desc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
//	image_desc.image_width		= 0;
//	image_desc.image_height		= 0;
//	image_desc.image_depth		= 0; /* only for 3D images */
//	image_desc.image_array_size	= 0; /* only for image arrays */
//	image_desc.image_row_pitch	= 0;
//	image_desc.image_slice_pitch	= 0; /* only for 3D images */
//	image_desc.num_mip_levels	= 0; /* mandatory */
//	image_desc.num_samples		= 0; /* mandatory */
//	image_desc.buffer		= NULL; /* only for 1D image buffers */

	ret = clGetImageInfo(source_image, CL_IMAGE_FORMAT, sizeof(image_format), &image_format, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get source image format information", error7);

	int components = clut_getImageFormatComponents(image_format);
	if (0 > components) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unknown components for source image.\n", fname);
		goto error7;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Source image has %d components.\n", fname, components);

	image_desc.image_width = width - filter_size + 1;
	image_desc.image_height = height - filter_size + 1;
	image_desc.image_row_pitch = image_desc.image_width * components;

	cl_mem result_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &image_desc, NULL, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create second image", error7);

	/* fill result image with black */
	const unsigned int fill_color[4] = { 0, 0, 0, 255 };
	const size_t fill_origin[3] = { 0, 0, 0 };
	const size_t fill_region[3] = { width - filter_size + 1, height - filter_size + 1, 1 };
	ret = clEnqueueFillImage(command_queue, result_image, fill_color, fill_origin, fill_region, 0, NULL, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to fill result image", error8);

	Debug_out(DEBUG_HOMEWORK, "%s: Images created.\n", fname);

	/* create filter matrix */
	unsigned char *filter_matrix = createFilterMatrix(filter_size);
	if (NULL == filter_matrix) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to create filter matrix.\n", fname);
		goto error8;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Filter matrix created.\n", fname);
//	printFilterMatrix(filter_matrix, filter_size);

	/* copy filter matrix to device */
	cl_mem filter_matrix_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, filter_size * filter_size, filter_matrix, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create filter matrix buffer on device", error9);

	/* set kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(source_image), (void *) &source_image);
	CLUT_CHECK_ERROR(ret, "Unable to set source image argument", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Source image argument set.\n", fname);
	ret = clSetKernelArg(kernel, 1, sizeof(result_image), (void *) &result_image);
	CLUT_CHECK_ERROR(ret, "Unable to set result image argument", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Result image argument set.\n", fname);
	ret = clSetKernelArg(kernel, 2, sizeof(filter_matrix_buffer), (void *) &filter_matrix_buffer);
	CLUT_CHECK_ERROR(ret, "Unable to set filter matrix argument", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Filter matrix argument set.\n", fname);

	Debug_out(DEBUG_HOMEWORK, "%s: All kernel arguments set.\n", fname);

	const size_t work_size[2] = {
		COMPUTE_GLOBAL_SIZE(height - filter_size + 1, local_height),
		COMPUTE_GLOBAL_SIZE(width - filter_size + 1, local_width) };
	const size_t wg_size[2] = { local_height, local_width };
	Debug_out(DEBUG_HOMEWORK, "%s: work size is [%zu]x[%zu].\n", fname, work_size[0], work_size[1]);

	/* run kernel */
	cl_event kernel_event;
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, work_size, wg_size, 0, NULL, &kernel_event);
	CLUT_CHECK_ERROR(ret, "Unable to enqueue kernel", error10);

	ret = clFinish(command_queue);
	CLUT_CHECK_ERROR(ret, "Unable to finish commands in queue", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel executed.\n", fname);
	ret = clWaitForEvents(1, &kernel_event);
	CLUT_CHECK_ERROR(ret, "Unable to wait for kernel event", error10);

	/* check that kernel executed correctly */
	cl_int kernel_ret;
	ret = clGetEventInfo(kernel_event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(kernel_ret), &kernel_ret, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get kernel status", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel status is %d.\n", fname, kernel_ret);
	if (CL_COMPLETE != kernel_ret) {
		Debug_out(DEBUG_HOMEWORK, "%s: kernel execution failed: %s.\n", fname, clut_getErrorDescription(kernel_ret));
		goto error10;
	}

	cl_ulong end_time;
	ret = clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(end_time), &end_time, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get kernel event end time", error10);
	if (0 == end_time) {
		Debug_out(DEBUG_HOMEWORK, "%s: kernel execution took 0 seconds.\n", fname);
		goto error10;
	}

	cl_double time_double = clut_getEventDuration(kernel_event);
	cl_ulong time_ulong = clut_getEventDuration_ns(kernel_event);
	Debug_out(DEBUG_HOMEWORK, "%s: Blurring took %f seconds (%lld nanoseconds).\n", fname, time_double, time_ulong);

	/* save image back to file */
	clut_saveImageToFile("output_local.png", command_queue, result_image);

	/* output filter size, local width, local height, and duration in nanoseconds for profiling */
	printf("%d,%zu,%zu,%lld\n", filter_size, local_width, local_height, clut_getEventDuration_ns(kernel_event));

	return_value = 0;

error10:
	clReleaseMemObject(filter_matrix_buffer);
error9:
	free(filter_matrix);
error8:
	clReleaseMemObject(result_image);
error7:
	clReleaseMemObject(source_image);
error6:
	clReleaseCommandQueue(command_queue);
error5:
	clReleaseKernel(kernel);
error4:
	clReleaseProgram(program);
error3:
	clReleaseContext(context);
error2:
	free(flags);
error1:
	return return_value;
}

/*!
 * @function clut_blurImage_local_unlimited
 * Blurs the image at [filename] with a filter of size [filter_size], and saves the result
 * to the file "output_unlimited.png". This function should be optimized to run on
 * local memory.
 * @param filename
 * The name of the file.
 * @param filter_size
 * The size of the blur filter.
 * @return
 * 0 on success, non-0 on failure.
 */
int clut_blurImage_local_unlimited(const cl_device_id device, const char * const filename, const unsigned int filter_size)
{
	const char * const fname = "clut_blurImage_local";
	int return_value = 1;
	cl_int ret;

	if (NULL == filename) {
		Debug_out(DEBUG_HOMEWORK, "%s: NULL pointer argument.\n", fname);
		goto error1;
	}

	/* compute work group size */
	size_t local_width, local_height;
	if (0 != clut_getMaxWGSize(device, &local_width, &local_height)) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to get work group sizes.\n", fname);
		goto error1;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Max work group size is [%zu]x[%zu].\n", fname, local_width, local_height);

	/* openCL wants to know the size of __local statically allocated arrays at compile time,
	 * so the local size must be set with a #define */
	char *flags = calloc(128, sizeof(char));
	if (NULL == flags) {
		Debug_out(DEBUG_HOMEWORK, "%s: A calloc failed.\n", fname);
		goto error1;
	}
	sprintf(flags, "-D LOCAL_WIDTH=%zu -D LOCAL_HEIGHT=%zu -D FILTER_SIZE=%d", local_width, local_height, filter_size);
	Debug_out(DEBUG_HOMEWORK, "%s: Local flags are: '%s'.\n", fname, flags);

	/* Create context */
	cl_context context = clCreateContext(NULL, 1, &device, clut_contextCallback, "clut_blurImage_local_unlimited", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create context", error2);
	Debug_out(DEBUG_HOMEWORK, "%s: Created context successfully.\n", fname);

	/* Create program */
	cl_program program = clut_createProgramFromFile(context, "homework_unlimited.cl", flags);
	if (NULL == program) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to create program.\n", fname);
		goto error3;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Program created.\n", fname);

	/* Create kernel */
	cl_kernel kernel = clCreateKernel(program, "blurImage_local_unlimited", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create kernel", error4);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel created.\n", fname);

	/* Create command_queue */
	cl_command_queue command_queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create command queue", error5);
	Debug_out(DEBUG_HOMEWORK, "%s: Command queue created.\n", fname);

	/* open source image */
	int width, height;
	cl_mem source_image = clut_loadImageFromFile(context, filename, &width, &height);
	if (NULL == source_image) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to read source image.\n", fname);
		goto error6;
	}

	if ((filter_size > (unsigned int) width) || (filter_size > (unsigned int) height)) {
		Debug_out(DEBUG_HOMEWORK, "%s: Filter does not fit in image.\n", fname);
		goto error7;
	}

	/* crate destination image */
	cl_image_format image_format = {0, 0};

	cl_image_desc image_desc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
//	image_desc.image_width		= 0;
//	image_desc.image_height		= 0;
//	image_desc.image_depth		= 0; /* only for 3D images */
//	image_desc.image_array_size	= 0; /* only for image arrays */
//	image_desc.image_row_pitch	= 0;
//	image_desc.image_slice_pitch	= 0; /* only for 3D images */
//	image_desc.num_mip_levels	= 0; /* mandatory */
//	image_desc.num_samples		= 0; /* mandatory */
//	image_desc.buffer		= NULL; /* only for 1D image buffers */

	ret = clGetImageInfo(source_image, CL_IMAGE_FORMAT, sizeof(image_format), &image_format, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get source image format information", error7);

	int components = clut_getImageFormatComponents(image_format);
	if (0 > components) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unknown components for source image.\n", fname);
		goto error7;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Source image has %d components.\n", fname, components);

	image_desc.image_width = width - filter_size + 1;
	image_desc.image_height = height - filter_size + 1;
	image_desc.image_row_pitch = image_desc.image_width * components;

	cl_mem result_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &image_desc, NULL, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create second image", error7);

	/* fill result image with black */
	const unsigned int fill_color[4] = { 0, 0, 0, 255 };
	const size_t fill_origin[3] = { 0, 0, 0 };
	const size_t fill_region[3] = { width - filter_size + 1, height - filter_size + 1, 1 };
	ret = clEnqueueFillImage(command_queue, result_image, fill_color, fill_origin, fill_region, 0, NULL, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to fill result image", error8);

	Debug_out(DEBUG_HOMEWORK, "%s: Images created.\n", fname);

	/* create filter matrix */
	unsigned char *filter_matrix = createFilterMatrix(filter_size);
	if (NULL == filter_matrix) {
		Debug_out(DEBUG_HOMEWORK, "%s: Unable to create filter matrix.\n", fname);
		goto error8;
	}
	Debug_out(DEBUG_HOMEWORK, "%s: Filter matrix created.\n", fname);
//	printFilterMatrix(filter_matrix, filter_size);

	/* copy filter matrix to device */
	cl_mem filter_matrix_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, filter_size * filter_size, filter_matrix, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create filter matrix buffer on device", error9);

	/* set kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(source_image), (void *) &source_image);
	CLUT_CHECK_ERROR(ret, "Unable to set source image argument", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Source image argument set.\n", fname);
	ret = clSetKernelArg(kernel, 1, sizeof(result_image), (void *) &result_image);
	CLUT_CHECK_ERROR(ret, "Unable to set result image argument", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Result image argument set.\n", fname);
	ret = clSetKernelArg(kernel, 2, sizeof(filter_matrix_buffer), (void *) &filter_matrix_buffer);
	CLUT_CHECK_ERROR(ret, "Unable to set filter matrix argument", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Filter matrix argument set.\n", fname);

	Debug_out(DEBUG_HOMEWORK, "%s: All kernel arguments set.\n", fname);

	const size_t work_size[2] = {
		COMPUTE_GLOBAL_SIZE(height - filter_size + 1, local_height),
		COMPUTE_GLOBAL_SIZE(width - filter_size + 1, local_width) };
	const size_t wg_size[2] = { local_height, local_width };
	Debug_out(DEBUG_HOMEWORK, "%s: work size is [%zu]x[%zu].\n", fname, work_size[0], work_size[1]);

	/* run kernel */
	cl_event kernel_event;
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, work_size, wg_size, 0, NULL, &kernel_event);
	CLUT_CHECK_ERROR(ret, "Unable to enqueue kernel", error10);

	ret = clFinish(command_queue);
	CLUT_CHECK_ERROR(ret, "Unable to finish commands in queue", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel executed.\n", fname);
	ret = clWaitForEvents(1, &kernel_event);
	CLUT_CHECK_ERROR(ret, "Unable to wait for kernel event", error10);

	/* check that kernel executed correctly */
	cl_int kernel_ret;
	ret = clGetEventInfo(kernel_event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(kernel_ret), &kernel_ret, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get kernel status", error10);
	Debug_out(DEBUG_HOMEWORK, "%s: Kernel status is %d.\n", fname, kernel_ret);
	if (CL_COMPLETE != kernel_ret) {
		Debug_out(DEBUG_HOMEWORK, "%s: kernel execution failed: %s.\n", fname, clut_getErrorDescription(kernel_ret));
		goto error10;
	}

	cl_ulong end_time;
	ret = clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(end_time), &end_time, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get kernel event end time", error10);
	if (0 == end_time) {
		Debug_out(DEBUG_HOMEWORK, "%s: kernel execution took 0 seconds.\n", fname);
		goto error10;
	}

	cl_double time_double = clut_getEventDuration(kernel_event);
	cl_ulong time_ulong = clut_getEventDuration_ns(kernel_event);
	Debug_out(DEBUG_HOMEWORK, "%s: Blurring took %f seconds (%lld nanoseconds).\n", fname, time_double, time_ulong);

	/* save image back to file */
	clut_saveImageToFile("output_unlimited.png", command_queue, result_image);

	/* output filter size, local width, local height, and duration in nanoseconds for profiling */
	printf("%d,%zu,%zu,%lld\n", filter_size, local_width, local_height, clut_getEventDuration_ns(kernel_event));

	return_value = 0;

error10:
	clReleaseMemObject(filter_matrix_buffer);
error9:
	free(filter_matrix);
error8:
	clReleaseMemObject(result_image);
error7:
	clReleaseMemObject(source_image);
error6:
	clReleaseCommandQueue(command_queue);
error5:
	clReleaseKernel(kernel);
error4:
	clReleaseProgram(program);
error3:
	clReleaseContext(context);
error2:
	free(flags);
error1:
	return return_value;
}


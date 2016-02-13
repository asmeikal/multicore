#include <stdlib.h>
#include <stdio.h>

#include <mlclut.h>
#include <mlclut_descriptions.h>
#include <mlclut_images.h>

int main(const int argc, char * const argv[])
{
	if (2 != argc) {
		fprintf(stderr, "Require filename as argument.\n");
		exit(EXIT_FAILURE);
	}
	int exit_code = EXIT_FAILURE;

	cl_platform_id *platforms;
	cl_uint n_platforms;

	/* Get platforms and devices */
	platforms = clut_getAllPlatforms(&n_platforms);
	if ((NULL == platforms) || (0 == n_platforms)) {
		printf("No platforms available!\n");
		exit(EXIT_FAILURE);
	}

	cl_uint p_index = 0, n_devices;
	cl_device_id *devices;
	do {
		devices = clut_getAllDevices(platforms[p_index], CL_DEVICE_TYPE_GPU, &n_devices);
		if ((0 == n_devices) || (NULL == devices)) {
			++p_index;
		}
	} while (((0 == n_devices) || (NULL == devices)) && (p_index < n_platforms));

	if ((0 == n_devices) || (NULL == devices)) {
		printf("Unable to get a GPU!\n");
		goto error1;
	}

	printf("We have %d GPU device(s) on platform #%d.\n", n_devices, p_index+1);

	/* Here, we have at least 1 GPU */
	cl_int ret;

	/* Create context */
	cl_context context = clCreateContext(NULL, 1, devices, clut_contextCallback, "Main context", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create context", error2);
	printf("Created context successfully.\n");

	/* Create program */
	cl_program program = clut_createProgramFromFile(context, "images.cl", NULL);
	CLUT_CHECK_ERROR(ret, "Unable to create program", error3);
	printf("Program created.\n");

	/* Create kernel */
	cl_kernel kernel = clCreateKernel(program, "drawEdge", &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create kernel", error4);
	printf("Kernel created.\n");

	/* Create command_queue */
	cl_command_queue command_queue = clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create command queue", error5);
	printf("Command queue created.\n");

	cl_image_format image_format = {0, 0};

	cl_image_desc image_desc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
//	image_desc.image_width = cols;
//	image_desc.image_height = rows;
//	image_desc.image_depth = 0;
//	image_desc.image_array_size = 0;
//	image_desc.image_row_pitch = 0;
//	image_desc.image_slice_pitch = 0;
//	image_desc.num_mip_levels = 0;
//	image_desc.num_samples = 0;
//	image_desc.buffer = NULL;

	int width, height;
	cl_mem source_image = clut_loadImageFromFile(context, argv[1], &width, &height);
	if (NULL == source_image) {
		fprintf(stderr, "Unable to read source image.\n");
		goto error6;
	}
	image_desc.image_width = width;
	image_desc.image_height = height;
	ret = clGetImageInfo(source_image, CL_IMAGE_FORMAT, sizeof(image_format), &image_format, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to get source image format information", error7);
	image_format.image_channel_order = CL_R;

	cl_mem result_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &image_format, &image_desc, NULL, &ret);
	CLUT_CHECK_ERROR(ret, "Unable to create second image", error7);

	printf("Images created.\n");

	ret = clSetKernelArg(kernel, 0, sizeof(source_image), (void *) &source_image);
	CLUT_CHECK_ERROR(ret, "Unable to set first kernel argument", error8);
	ret = clSetKernelArg(kernel, 1, sizeof(result_image), (void *) &result_image);
	CLUT_CHECK_ERROR(ret, "Unable to set second kernel argument", error8);

	printf("Kernel arguments set.\n");

	const size_t work_size[2] = { height, width };
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, work_size, NULL, 0, NULL, NULL);
	CLUT_CHECK_ERROR(ret, "Unable to enqueue kernel", error8);

	printf("Kernel executed.\n");

	clut_saveImageToFile("output.png", command_queue, result_image);

	printf("Image written back to file.\n");

	exit_code = EXIT_SUCCESS;

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
	free(devices);
error1:
	free(platforms);
	exit(exit_code);
}


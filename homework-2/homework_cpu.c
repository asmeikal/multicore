#include <stdlib.h>
#include <stdio.h>

#include <mlclut.h>
#include <mlclut_descriptions.h>
#include <mlclut_images.h>

#include "homework_utils.h"

int main(const int argc, char * const argv[])
{
	if (3 != argc) {
		fprintf(stderr, "Require filename and filter size as arguments.\n");
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
		devices = clut_getAllDevices(platforms[p_index], CL_DEVICE_TYPE_CPU, &n_devices);
		if ((0 == n_devices) || (NULL == devices)) {
			++p_index;
		}
	} while (((0 == n_devices) || (NULL == devices)) && (p_index < n_platforms));

	if ((0 == n_devices) || (NULL == devices)) {
		printf("Unable to get a CPU!\n");
		goto error1;
	}

	printf("We have %d CPU device(s) on platform #%d.\n", n_devices, p_index+1);

	if (clut_blurImage(devices[0], argv[1], atoi(argv[2]))) {
		printf("Unable to blur image.\n");
		goto error2;
	}

	exit_code = EXIT_SUCCESS;
error2:
	free(devices);
error1:
	free(platforms);
	exit(exit_code);
}


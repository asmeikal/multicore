#ifndef FILTER_SIZE
#error "Define FILTER_SIZE"
#endif

#ifndef LOCAL_WIDTH
#error "Define LOCAL_WIDTH"
#endif

#ifndef LOCAL_HEIGHT
#error "Define LOCAL_HEIGHT"
#endif

/*!
 * @function blurImage_local
 * Applies the square filter at [filter_matrix] with size [FILTER_SIZE]
 * (defined at compile time) to the image [source], and stores it in the image
 * [dest]. The [LOCAL_SIZE] of the kernel must be defined at compile time, too.
 * This kernel should be submitted with an NDRange equal to the [width] x [height]
 * of the destination image, which is [width - filter_size + 1] x [height - filter_size + 1]
 * of the original image.
 * The [source] and [dest] image must have a channel_data_type of CL_UNSIGNED_INT8.
 */
kernel void blurImage_local(
		read_only image2d_t source,
		write_only image2d_t dest,
		constant read_only unsigned char *filter_matrix
	)
{
	/* sampler: not normalized, outside access returns 0, filter unused */
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

	/* get position info and image size */
	const int2 dest_bounds = { get_image_width(dest), get_image_height(dest) };

	const int2 pos   = { get_global_id(1), get_global_id(0) };
	const int2 l_pos = { get_local_id(1), get_local_id(0) };
	const int2 b_pos = { get_group_id(1) * LOCAL_WIDTH, get_group_id(0) * LOCAL_HEIGHT };

	/* copy pixels to local buffer */
	local uint4 support_matrix[LOCAL_WIDTH + FILTER_SIZE - 1][LOCAL_HEIGHT + FILTER_SIZE - 1];

	int i, j;
	int s_i = LOCAL_WIDTH + FILTER_SIZE - 1, s_j = LOCAL_HEIGHT + FILTER_SIZE - 1;
	for (i = l_pos.x ; i < s_i; i += LOCAL_WIDTH) {
		for (j = l_pos.y ; j < s_j; j += LOCAL_HEIGHT) {
			support_matrix[i][j] = read_imageui(source, sampler, b_pos + (int2) (i, j));
		}
	}
	barrier(CLK_GLOBAL_MEM_FENCE);

	/* compute resulting pixel */
	uint4 result = (uint4) (0);

	unsigned int count = 0;
	for (i = 0; i < FILTER_SIZE; ++i) {
		for (j = 0; j < FILTER_SIZE; ++j) {
			if (0 != filter_matrix[i * FILTER_SIZE +j]) {
				count += 1;
				result += support_matrix[i + l_pos.x][j + l_pos.y];
			}
		}
	}
	result = result / count;

	barrier(CLK_GLOBAL_MEM_FENCE);

	/* write out, only if within result image */
	if ((pos.x < dest_bounds.x) && (pos.y < dest_bounds.y)) {
		write_imageui(dest, pos, result);
	}
}


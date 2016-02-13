/*!
 */
kernel void blurImage(
		read_only image2d_t source,
		write_only image2d_t dest,
		private const unsigned int filter_size,
		global read_only unsigned char *filter_matrix
	)
{
	/* sampler: not normalized, no access outside image, filter unused */
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

	const int2 pos = { get_global_id(1), get_global_id(0) };
	uint4 result = (uint4) (0);

	unsigned int count = 0;
	unsigned int i, j;
	for (i = 0; i < filter_size; ++i) {
		for (j = 0; j < filter_size; ++j) {
			if (0 != filter_matrix[i * filter_size + j]) {
				count += 1;
				result += read_imageui(source, sampler, pos + (int2) (i, j));
			}
		}
	}
	result = result / count;

	write_imageui(dest, pos, result);
}

#ifndef FILTER_SIZE
#define FILTER_SIZE	1
#endif

#ifndef LOCAL_SIZE
#define LOCAL_SIZE	1
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
	const int2 b_pos = { get_group_id(1) * LOCAL_SIZE, get_group_id(0) * LOCAL_SIZE };

	/* copy pixels to local buffer */
	local uint4 support_matrix[LOCAL_SIZE + FILTER_SIZE - 1][LOCAL_SIZE + FILTER_SIZE - 1];

	int i, j;
	int s_i = LOCAL_SIZE + FILTER_SIZE - 1, s_j = LOCAL_SIZE + FILTER_SIZE - 1;
	for (i = l_pos.x ; i < s_i; i += LOCAL_SIZE) {
		for (j = l_pos.y ; j < s_j; j += LOCAL_SIZE) {
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

#ifndef LOCAL_WIDTH
#define LOCAL_WIDTH	1
#endif

#ifndef LOCAL_HEIGHT
#define LOCAL_HEIGHT	1
#endif

kernel void blurImage_local_unlimited(
		read_only image2d_t source,
		write_only image2d_t dest,
		constant read_only unsigned char *filter_matrix
	)
{
	/* sampler: not normalized, outside access returns 0, filter unused */
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

	/* get position info and image size */
	const int2 dest_bounds = { get_image_width(dest), get_image_height(dest) };

	/* global position in the resulting image */
	const int2 pos   = { get_global_id(1), get_global_id(0) };
	/* local position in the work group */
	const int2 l_pos = { get_local_id(1), get_local_id(0) };
	/* first pixel in top left corner of the resulting image */
	const int2 b_pos = { get_group_id(1) * LOCAL_WIDTH, get_group_id(0) * LOCAL_HEIGHT };

	/* copy pixels to local buffer */
	local uint4 support_matrix[LOCAL_WIDTH][LOCAL_HEIGHT];

	/* indeces of the sections */
	int ki, kj;
	unsigned int count = 0;
	uint4 result = (uint4) (0);

	for (ki = 0; ki < LOCAL_WIDTH + FILTER_SIZE - 1; ki += LOCAL_WIDTH) {
		for (kj = 0; kj < LOCAL_HEIGHT + FILTER_SIZE - 1; kj += LOCAL_HEIGHT) {
			/* copy area from (ki, kj) -> (ki + LOCAL_WIDTH - 1, kj + LOCAL_HEIGHT - 1) */
			support_matrix[l_pos.x][l_pos.y] = read_imageui(source, sampler, (int2) (b_pos.x + ki + l_pos.x, b_pos.y + kj + l_pos.y));

			barrier(CLK_LOCAL_MEM_FENCE);

			/* begin and end of the source image needed by this work item */
			const int2 begin = pos;
			const int2 end = begin + (int2) (FILTER_SIZE, FILTER_SIZE);

			const int2 matrix_begin = b_pos + (int2) (ki, kj);
			const int2 matrix_end = matrix_begin + (int2) (LOCAL_WIDTH, LOCAL_HEIGHT);

			/* find out which part of the source image is inside the local buffer
			 * i.e. clip needed image to what's in the buffer  */
			const int2 turn_begin, turn_end;

			turn_begin.x = (begin.x > matrix_begin.x ? begin.x : matrix_begin.x);
			turn_begin.y = (begin.y > matrix_begin.y ? begin.y : matrix_begin.y);

			turn_end.x = (end.x < matrix_end.x ? end.x : matrix_end.x);
			turn_end.y = (end.y < matrix_end.y ? end.y : matrix_end.y);

			if ((turn_begin.x < turn_end.x) && (turn_begin.y < turn_end.y)) {
				/* position on the filter matrix */
				const int istart = turn_begin.x - begin.x;
				const int jstart = turn_begin.y - begin.y;
				int i, j;
				/* position on the support buffer */
				const int bistart = turn_begin.x - b_pos.x - ki;
				const int bjstart = turn_begin.y - b_pos.y - kj;
				const int biend = turn_end.x - b_pos.x - ki;
				const int bjend = turn_end.y - b_pos.y - kj;
				int bi, bj;

				for (bi = bistart, i = istart; bi < biend; ++bi, ++i) {
					for (bj = bjstart, j = jstart; bj < bjend; ++bj, ++j) {
						if (0 != filter_matrix[i * FILTER_SIZE + j]) {
							count += 1;
							result += support_matrix[bi][bj];
						}
					}
				}
			}

			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}
	result = result / count;

	/* write out, only if within result image */
	if ((pos.x < dest_bounds.x) && (pos.y < dest_bounds.y)) {
		write_imageui(dest, pos, result);
	}
}


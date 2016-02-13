/* Apparently, openCL wants all functions to be declared before definition */
uint sqrt_ui(uint x);
uint dot_ui(uint4 x, uint4 y);

/*
 * @function sqrt_ui
 * Computes the unsigned integer square root of [x], i.e. the largest i
 * such that i * i <= x. Thanks Christian.
 * @param x
 * The value whose square root must be computed.
 * @return
 * The square root of [x].
 */
uint sqrt_ui(uint x)
{
	uint i = 0;
	while (x >= i * i) {
		++i;
	}
	return i-1;
}

/*!
 * @function dot_ui
 * Computes the dot product of the two uint4 vectors [x] and [y].
 * The dot product is defined as x[0] * y[0] + ... + x[n] * y[n].
 * @param x
 * A uint4 vector.
 * @param y
 * A uint4 vector.
 * @return
 * The dot product [x] * [y].
 */
uint dot_ui(uint4 x, uint4 y)
{
	return	x.x * y.x +
		x.y * y.y +
		x.z * y.z +
		x.w * y.w;
}

/*!
 * @function flipImage
 * Flips an image along the y axis. Kernel must be enqueued with an
 * NDRange of [rows] x [cols].
 * @param source
 * A non-normalized 2D image. Must have the same size of [dest].
 * @param dest
 * A non-normalized 2D image. Must have the same size of [source].
 */
kernel void flipImage(
		read_only image2d_t source,
		write_only image2d_t dest
	)
{
	/* sampler: not normalized, with border, filter unused */
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	/* first x, then y. x is cols, y is rows. strange, I know.
	 * looks something like this:
	 *   .----------> x
	 *   | # # # # #
	 *   | # # # # #
	 *   | # # # # #
	 *   | # # # # #
	 *   | # # # # #
	 * y v
	 */
	const int2 pos   = (int2) (get_global_id(1), get_global_id(0));
	const int2 d_pos = (int2) (get_global_size(1) - pos.x, pos.y);

	write_imagei(dest, d_pos, read_imagei(source, sampler, pos));
}

/*!
 * @function drawEdge
 * Compute the edges of [source] using the Sobel operator (en.wikipedia.org/wiki/Sobel_operator).
 * @param source
 * The source image. It must have channel_data_type CL_UNSIGNED_*.
 * @param dest
 * The destination image. It must have the same size as [source], channel_data_type CL_UNSIGNED_*,
 * and channel_order CL_R or CL_Rx.
 */
kernel void drawEdge(
		read_only image2d_t source,
		write_only image2d_t dest
	)
{
	/* sampler: not normalized, with border, filter unused */
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

	const int2 pos = (int2) (get_global_id(1), get_global_id(0));

	uint4 gx, gy;

	/*
	 * Compute gx and gy as:
	 *
	 *      -1 0 1
	 * gx = -2 0 2 x source
	 *      -1 0 1
	 *
	 *      -1 -2 -1
	 * gy =  0  0  0 x source
	 *       1  2  1
	 */
	gx =	- read_imageui(source, sampler, (int2) (pos.x - 1, pos.y - 1))
		- read_imageui(source, sampler, (int2) (pos.x - 1, pos.y + 0)) * 2
		- read_imageui(source, sampler, (int2) (pos.x - 1, pos.y + 1))
		+ read_imageui(source, sampler, (int2) (pos.x + 1, pos.y - 1))
		+ read_imageui(source, sampler, (int2) (pos.x + 1, pos.y + 0)) * 2
		+ read_imageui(source, sampler, (int2) (pos.x + 1, pos.y + 1));
	gy =	- read_imageui(source, sampler, (int2) (pos.x - 1, pos.y - 1))
		- read_imageui(source, sampler, (int2) (pos.x + 0, pos.y - 1)) * 2
		- read_imageui(source, sampler, (int2) (pos.x + 1, pos.y - 1))
		+ read_imageui(source, sampler, (int2) (pos.x - 1, pos.y + 1))
		+ read_imageui(source, sampler, (int2) (pos.x + 0, pos.y + 1)) * 2
		+ read_imageui(source, sampler, (int2) (pos.x + 1, pos.y + 1));

	uint result = sqrt_ui(dot_ui(gx, gx) + dot_ui(gy, gy));

	write_imageui(dest, pos, (uint4) (result, 0, 0, 255));
}

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
		global read_only unsigned char *filter_matrix
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
	barrier(CLK_LOCAL_MEM_FENCE);

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


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



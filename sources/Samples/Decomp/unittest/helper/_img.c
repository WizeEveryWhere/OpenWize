
#include "../_img.h"

const img_header_t img_header =
{
	.magic = MAGIC_WORD_IMG,
	.hdr_sz = sizeof(img_header_t),
	.type = IMG_TYPE_APP
};

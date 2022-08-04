#include "png.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t size)
{
    png_image img;
    img.version = PNG_IMAGE_VERSION;
    img.opaque = NULL;

    png_image_begin_read_from_memory(&img, Data, size);

    img.format = PNG_FORMAT_GRAY;
    void *buffer = malloc(PNG_IMAGE_SIZE(img));

    png_image_finish_read(&img, NULL, buffer, PNG_IMAGE_ROW_STRIDE(img), NULL);
    free(buffer);

    return 0;
}

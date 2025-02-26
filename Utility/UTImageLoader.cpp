#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>
#include "../Dependency/stb/stb_image.h"

namespace Utility {
    struct ImageData {
        struct Meta {
            int width;
            int height;
            int channelsCount;
        } meta;

        struct Resource {
            stbi_uc* data;
        } resource;
    };

    ImageData loadImageData (const char* imageFilePath) {
        ImageData imageData;
        /* Note that, the STBI_rgb_alpha value forces the image to be loaded with an alpha channel, even if it doesn't
         * have one
         *
         * The pointer returned is the first element in an array of pixel values that are laid out row by row with 4
         * bytes per pixel in the case of STBI_rgb_alpha for a total of (width * height * 4) values
        */
        imageData.resource.data = stbi_load (imageFilePath,
                                             &imageData.meta.width,
                                             &imageData.meta.height,
                                             &imageData.meta.channelsCount,
                                             STBI_rgb_alpha);
        if (!imageData.resource.data)
            throw std::runtime_error ("Failed to load image data");
        return imageData;
    }

    void destroyImageData (stbi_uc* data) {
        stbi_image_free (data);
    }
}   // namespace Utility
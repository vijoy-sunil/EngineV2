#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "../Backend/Common.h"
#include "../Backend/Log/LGImpl.h"

namespace SandBox {
    class SBTexturePool {
        private:
            struct ImageInfo {
                int width;
                int height;
                int channelsCount;
                stbi_uc* data;
            };

            struct TexturePoolInfo {
                struct Meta {
                    std::unordered_map <std::string, uint32_t> pathToIdxMap;
                    std::map <uint32_t, ImageInfo> idxToImageInfoMap;
                    uint32_t nextAvailableIdx;
                } meta;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_texturePoolInfo;

            ImageInfo loadImage (const char* imageFilePath) {
                ImageInfo imageInfo;
                /* Note that, the STBI_rgb_alpha value forces the image to be loaded with an alpha channel, even if it
                 * doesn't have one. The pointer returned is the first element in an array of pixel values that are laid
                 * out row by row with 4 bytes per pixel in the case of STBI_rgb_alpha for a total of (width * height *
                 * 4) values
                */
                imageInfo.data = stbi_load (imageFilePath,
                                            &imageInfo.width,
                                            &imageInfo.height,
                                            &imageInfo.channelsCount,
                                            STBI_rgb_alpha);
                if (!imageInfo.data) {
                    LOG_ERROR (m_texturePoolInfo.resource.logObj) << "Failed to load image"
                                                                  << " "
                                                                  << "[" << imageFilePath << "]"
                                                                  << std::endl;
                    throw std::runtime_error ("Failed to load image");
                }
                return imageInfo;
            }

        public:
            SBTexturePool (void) {
                m_texturePoolInfo = {};

                auto& logObj = m_texturePoolInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initTexturePoolInfo (void) {
                auto& meta             = m_texturePoolInfo.meta;
                meta.pathToIdxMap      = {};
                meta.idxToImageInfoMap = {};
                meta.nextAvailableIdx  = 0;
            }

            uint32_t getTextureIdx (const std::string imageFilePath) {
                return m_texturePoolInfo.meta.pathToIdxMap[imageFilePath];
            }

            std::map <uint32_t, ImageInfo>& getTexturePool (void) {
                return m_texturePoolInfo.meta.idxToImageInfoMap;
            }

            void addTexture (const std::string imageFilePath) {
                auto& meta = m_texturePoolInfo.meta;
                /* Return if texture has already been added to the pool */
                if (meta.pathToIdxMap.find (imageFilePath) != meta.pathToIdxMap.end())
                    return;

                meta.pathToIdxMap.insert ({
                    imageFilePath,
                    meta.nextAvailableIdx
                });
                meta.idxToImageInfoMap[meta.nextAvailableIdx] = loadImage (
                    imageFilePath.c_str()
                );
                ++meta.nextAvailableIdx;
            }

            void destroyImage (const uint32_t textureIdx) {
                auto& idxToImageInfoMap = m_texturePoolInfo.meta.idxToImageInfoMap;
                if (idxToImageInfoMap.find (textureIdx) == idxToImageInfoMap.end()) {
                    LOG_ERROR (m_texturePoolInfo.resource.logObj) << "Image info does not exist"
                                                                  << " "
                                                                  << "[" << textureIdx << "]"
                                                                  << std::endl;
                    throw std::runtime_error ("Image info does not exist");
                }
                auto imageInfo = idxToImageInfoMap[textureIdx];
                stbi_image_free (imageInfo.data);
            }

            void generateReport (void) {
                auto& meta   = m_texturePoolInfo.meta;
                auto& logObj = m_texturePoolInfo.resource.logObj;

                LOG_LITE_INFO (logObj)     << "{"                     << std::endl;
                for (auto const& [path, idx]: meta.pathToIdxMap) {
                    auto imageInfo = meta.idxToImageInfoMap[idx];

                    LOG_LITE_INFO (logObj) << "\t" << ALIGN_AND_PAD_S << idx                     << ", "
                                                   << ALIGN_AND_PAD_S << imageInfo.width         << ", "
                                                   << ALIGN_AND_PAD_S << imageInfo.height        << ", "
                                                   << ALIGN_AND_PAD_S << imageInfo.channelsCount << ", "
                                                   << path            << std::endl;
                }
                LOG_LITE_INFO (logObj)     << "}"                     << std::endl;
            }

            ~SBTexturePool (void) {
                delete m_texturePoolInfo.resource.logObj;
            }
    };
}   // namespace SandBox
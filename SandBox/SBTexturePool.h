#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>
#include <string>
#include <map>
#include <unordered_map>
#include "../Backend/Log/LGImpl.h"
#include "../Dependency/stb/stb_image.h"
#include "SBRendererType.h"

namespace SandBox {
    class SBTexturePool {
        private:
            struct ImageInfo {
                struct Meta {
                    int width;
                    int height;
                    int channelsCount;
                } meta;

                struct Resource {
                    stbi_uc* data;
                } resource;
            };

            struct TexturePoolInfo {
                struct Meta {
                    std::unordered_map <std::string, TextureIdxType> pathToIdxMap;
                    std::map <TextureIdxType, ImageInfo> idxToImageInfoMap;
                    TextureIdxType nextAvailableIdx;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

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
                imageInfo.resource.data = stbi_load (imageFilePath,
                                                     &imageInfo.meta.width,
                                                     &imageInfo.meta.height,
                                                     &imageInfo.meta.channelsCount,
                                                     STBI_rgb_alpha);
                if (!imageInfo.resource.data) {
                    LOG_ERROR (m_texturePoolInfo.resource.logObj) << "Failed to load image"
                                                                  << " "
                                                                  << "[" << imageFilePath << "]"
                                                                  << std::endl;
                    throw std::runtime_error ("Failed to load image");
                }
                return imageInfo;
            }

        public:
            SBTexturePool (Log::LGImpl* logObj) {
                m_texturePoolInfo = {};

                if (logObj == nullptr) {
                    m_texturePoolInfo.resource.logObj     = new Log::LGImpl();
                    m_texturePoolInfo.state.logObjCreated = true;

                    m_texturePoolInfo.resource.logObj->initLogInfo ("Build/Log/SandBox", __FILE__);
                    LOG_WARNING (m_texturePoolInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                    << std::endl;
                }
                else {
                    m_texturePoolInfo.resource.logObj     = logObj;
                    m_texturePoolInfo.state.logObjCreated = false;
                }
            }

            void initTexturePoolInfo (void) {
                m_texturePoolInfo.meta.pathToIdxMap      = {};
                m_texturePoolInfo.meta.idxToImageInfoMap = {};
                m_texturePoolInfo.meta.nextAvailableIdx  = 0;
            }

            TextureIdxType getTextureIdx (const std::string imageFilePath) {
                return m_texturePoolInfo.meta.pathToIdxMap[imageFilePath];
            }

            std::map <TextureIdxType, ImageInfo>& getTexturePool (void) {
                return m_texturePoolInfo.meta.idxToImageInfoMap;
            }

            void addTextureToPool (const std::string imageFilePath) {
                auto& pathToIdxMap      = m_texturePoolInfo.meta.pathToIdxMap;
                auto& idxToImageInfoMap = m_texturePoolInfo.meta.idxToImageInfoMap;
                auto& nextAvailableIdx  = m_texturePoolInfo.meta.nextAvailableIdx;
                /* Return if texture has already been added to the pool */
                if (pathToIdxMap.find (imageFilePath) != pathToIdxMap.end())
                    return;

                pathToIdxMap.insert (
                    {
                        imageFilePath,
                        nextAvailableIdx
                    }
                );
                idxToImageInfoMap[nextAvailableIdx] = loadImage (
                    imageFilePath.c_str()
                );
                ++nextAvailableIdx;
            }

            void destroyImage (const TextureIdxType textureIdx) {
                auto& idxToImageInfoMap = m_texturePoolInfo.meta.idxToImageInfoMap;
                if (idxToImageInfoMap.find (textureIdx) == idxToImageInfoMap.end()) {
                    LOG_ERROR (m_texturePoolInfo.resource.logObj) << "Image info does not exist"
                                                                  << " "
                                                                  << "[" << textureIdx << "]"
                                                                  << std::endl;
                    throw std::runtime_error ("Image info does not exist");
                }
                auto imageInfo = idxToImageInfoMap[textureIdx];
                stbi_image_free (imageInfo.resource.data);
            }

            void generateReport (void) {
                auto& logObj = m_texturePoolInfo.resource.logObj;

                LOG_LITE_INFO (logObj) << "{" << std::endl;
                for (auto const& [path, idx]: m_texturePoolInfo.meta.pathToIdxMap) {
                    auto imageInfo = m_texturePoolInfo.meta.idxToImageInfoMap[idx];

                    LOG_LITE_INFO (logObj) << "\t";
                    /* Promote to a type printable as a number, regardless of type. This works as long as the type
                     * provides a unary + operator with ordinary semantics
                    */
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_S << +idx                         << ", ";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_S << imageInfo.meta.width         << ", ";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_S << imageInfo.meta.height        << ", ";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_S << imageInfo.meta.channelsCount << ", ";
                    LOG_LITE_INFO (logObj) << path            << std::endl;
                }
                LOG_LITE_INFO (logObj) << "}" << std::endl;
            }

            ~SBTexturePool (void) {
                if (m_texturePoolInfo.state.logObjCreated)
                    delete m_texturePoolInfo.resource.logObj;
            }
    };
}   // namespace SandBox
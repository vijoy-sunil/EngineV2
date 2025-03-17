#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <functional>
#include <vulkan/vk_enum_string_helper.h>
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKWindow.h"
#include "VKLogDevice.h"
#include "VKSwapChain.h"
#include "VKFence.h"
#include "VKSemaphore.h"
#include "VKCmdBuffer.h"
#include "VKHelper.h"

namespace Renderer {
    class VKRenderer: public Collection::CNTypeInstanceBase {
        private:
            struct RendererInfo {
                struct Meta {
                    uint32_t swapChainImageIdx;
                    uint32_t frameInFlightIdx;
                    uint32_t maxFramesInFlight;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKWindow*    windowObj;
                    VKLogDevice* logDeviceObj;
                    VKSwapChain* swapChainObj;
                    /* Objects per frame in flight */
                    std::vector <VKFence*>     inFlightFenObjs;
                    std::vector <VKSemaphore*> imageAvailableSemObjs;
                    std::vector <VKSemaphore*> renderDoneSemObjs;
                    VKCmdBuffer* cmdBufferObj;
                    /* Bindings */
                    std::vector <std::function <void (void)>> viewPortResizeBindings;
                } resource;
            } m_rendererInfo;

            bool getSwapChainImageIdxEXT (void) {
                auto& frameIdx = m_rendererInfo.meta.frameInFlightIdx;
                auto& resource = m_rendererInfo.resource;
                /* The semaphore is signaled when the presentation engine is finished using the image. That's the point
                 * in time where we can start drawing to it. Note that, the image index refers to the VkImage in our swap
                 * chain images array. We're going to use this index to pick the frame buffer. The function returns the
                 * index of the next image that will be available at some point notified by the semaphore
                */
                auto result    = vkAcquireNextImageKHR (*resource.logDeviceObj->getLogDevice(),
                                                        *resource.swapChainObj->getSwapChain(),
                                                        UINT64_MAX,
                                                        *resource.imageAvailableSemObjs[frameIdx]->getSemaphore(),
                                                        nullptr,
                                                        &m_rendererInfo.meta.swapChainImageIdx);
                /* If the swap chain turns out to be out of date when attempting to acquire an image, then it is no longer
                 * possible to present to it. Therefore we should immediately recreate the swap chain and its dependents
                 * and try again
                */
                if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    LOG_WARNING (resource.logObj) << "Failed to get swap chain image idx"
                                                  << " "
                                                  << "[" << string_VkResult (result) << "]"
                                                  << std::endl;

                    for (auto const& binding: resource.viewPortResizeBindings) {
                        if (binding != nullptr)
                            binding();
                    }
                    return false;
                }
                /* You could also decide to recreate and return if the swap chain is suboptimal, but we've chosen to
                 * proceed anyway in that case because we've already acquired an image. As a result, both VK_SUCCESS and
                 * VK_SUBOPTIMAL_KHR are considered "success" return codes here
                */
                else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                    LOG_ERROR (resource.logObj)   << "Failed to get swap chain image idx"
                                                  << " "
                                                  << "[" << string_VkResult (result) << "]"
                                                  << std::endl;
                    throw std::runtime_error ("Failed to get swap chain image idx");
                }
                /* There is another case where a swap chain may become out of date and that is a special kind of window
                 * resizing: window minimization. We will handle that by pausing until the window is in the foreground
                 * again
                */
                else if (resource.windowObj->isWindowIconified()) {
                    LOG_WARNING (resource.logObj) << "Window iconified"
                                                  << std::endl;
                    while (resource.windowObj->isWindowIconified())
                        glfwWaitEvents();
                }
                return true;
            }

            void presentSwapChainImage (void) {
                auto& frameIdx                 = m_rendererInfo.meta.frameInFlightIdx;
                auto& resource                 = m_rendererInfo.resource;

                VkPresentInfoKHR presentInfo;
                presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.pNext              = nullptr;
                presentInfo.swapchainCount     = 1;
                presentInfo.pSwapchains        = resource.swapChainObj->getSwapChain();
                presentInfo.pImageIndices      = &m_rendererInfo.meta.swapChainImageIdx;
                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores    = resource.renderDoneSemObjs[frameIdx]->getSemaphore();
                presentInfo.pResults           = nullptr;
                /* Note that, the presentation engine isn't guaranteed to act in concert with the queue it’s on, even if
                 * it’s on a graphics queue. vkAcquireNextImageKHR returns when the presentation engine knows which index
                 * will be used next, but provides no guarantee that it’s actually synchronized with the display and
                 * finished with the resources from the last VkQueuePresentKHR with that index
                 *
                 * You should use both the semaphore and the fence to ensure that it is safe to reuse resources, by
                 * waiting on the fence before re-recording any command buffers or updating any buffers or descriptors
                 * associated with that index, and waiting on the semaphore when submitting any stage that depends on
                 * the associated swap chain image
                */
                auto result = vkQueuePresentKHR (*resource.logDeviceObj->getPresentQueue(), &presentInfo);

                if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
                    resource.windowObj->isWindowResized()) {
                    LOG_WARNING (resource.logObj) << "Failed to present swap chain image"
                                                  << " "
                                                  << "[" << string_VkResult (result) << "]"
                                                  << std::endl;

                    resource.windowObj->toggleWindowResized (false);
                    for (auto const& binding: resource.viewPortResizeBindings) {
                        if (binding != nullptr)
                            binding();
                    }
                }
                else if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj)   << "Failed to present swap chain image"
                                                  << " "
                                                  << "[" << string_VkResult (result) << "]"
                                                  << std::endl;
                    throw std::runtime_error ("Failed to present swap chain image");
                }
                else if (resource.windowObj->isWindowIconified()) {
                    LOG_WARNING (resource.logObj) << "Window iconified"
                                                  << std::endl;
                    while (resource.windowObj->isWindowIconified())
                        glfwWaitEvents();
                }
            }

            void updateFrameInFlightIdx (void) {
                auto& frameIdx = m_rendererInfo.meta.frameInFlightIdx;
                frameIdx       = (frameIdx + 1) % m_rendererInfo.meta.maxFramesInFlight;
            }

        public:
            VKRenderer (Log::LGImpl* logObj,
                        VKWindow*    windowObj,
                        VKLogDevice* logDeviceObj,
                        VKSwapChain* swapChainObj,
                        const std::vector <VKFence*>     inFlightFenObjs,
                        const std::vector <VKSemaphore*> imageAvailableSemObjs,
                        const std::vector <VKSemaphore*> renderDoneSemObjs,
                        VKCmdBuffer* cmdBufferObj) {

                m_rendererInfo = {};

                if (logObj == nullptr) {
                    m_rendererInfo.resource.logObj             = new Log::LGImpl();
                    m_rendererInfo.state.logObjCreated         = true;

                    m_rendererInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_rendererInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                 << std::endl;
                }
                else {
                    m_rendererInfo.resource.logObj             = logObj;
                    m_rendererInfo.state.logObjCreated         = false;
                }

                if (windowObj == nullptr    || logDeviceObj == nullptr       ||
                    swapChainObj == nullptr || cmdBufferObj == nullptr       ||
                    inFlightFenObjs.empty() || imageAvailableSemObjs.empty() || renderDoneSemObjs.empty()) {

                    LOG_ERROR (m_rendererInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                               << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_rendererInfo.resource.windowObj              = windowObj;
                m_rendererInfo.resource.logDeviceObj           = logDeviceObj;
                m_rendererInfo.resource.swapChainObj           = swapChainObj;
                m_rendererInfo.resource.inFlightFenObjs        = inFlightFenObjs;
                m_rendererInfo.resource.imageAvailableSemObjs  = imageAvailableSemObjs;
                m_rendererInfo.resource.renderDoneSemObjs      = renderDoneSemObjs;
                m_rendererInfo.resource.cmdBufferObj           = cmdBufferObj;
                m_rendererInfo.resource.viewPortResizeBindings = {};
            }

            void initRendererInfo (const uint32_t maxFramesInFlight) {
                m_rendererInfo.meta.swapChainImageIdx = 0;
                m_rendererInfo.meta.frameInFlightIdx  = 0;
                m_rendererInfo.meta.maxFramesInFlight = maxFramesInFlight;

                /* Make sure we have sync objects for every frame in flight */
                if (m_rendererInfo.resource.inFlightFenObjs.size()       != maxFramesInFlight ||
                    m_rendererInfo.resource.imageAvailableSemObjs.size() != maxFramesInFlight ||
                    m_rendererInfo.resource.renderDoneSemObjs.size()     != maxFramesInFlight) {

                    LOG_ERROR (m_rendererInfo.resource.logObj) << "Invalid sync objects count"
                                                               << std::endl;
                    throw std::runtime_error ("Invalid sync objects count");
                }
            }

            uint32_t getSwapChainImageIdx (void) {
                return m_rendererInfo.meta.swapChainImageIdx;
            }

            uint32_t getFrameInFlightIdx (void) {
                return m_rendererInfo.meta.frameInFlightIdx;
            }

            void addViewPortResizeBinding (const std::function <void (void)> binding) {
                m_rendererInfo.resource.viewPortResizeBindings.push_back (binding);
            }

            bool beginFrame (void) {
                auto& frameIdx      = m_rendererInfo.meta.frameInFlightIdx;
                auto& resource      = m_rendererInfo.resource;
                auto inFlightFenObj = resource.inFlightFenObjs[frameIdx];
                auto cmdBuffer      = resource.cmdBufferObj->getCmdBuffers()[frameIdx];
                /* At the start of the frame, we want to wait until the previous frame has finished, so that the command
                 * buffer and semaphores are available to use
                */
                inFlightFenObj->waitForFence();
                if (!getSwapChainImageIdxEXT())
                    return false;
                /* After waiting for fence, we need to manually reset the fence to the unsignaled state immediately after.
                 * But we delay it to upto this point to avoid deadlock on the in flight fence
                 *
                 * When vkAcquireNextImageKHR returns VK_ERROR_OUT_OF_DATE_KHR, we recreate the swap chain and its
                 * dependents and then return. But before that happens, the current frame's fence was waited upon and
                 * reset. Since we return immediately, no work is submitted for execution and the fence will never be
                 * signaled, causing vkWaitForFences to halt forever
                 *
                 * To overcome this, delay resetting the fence until after we know for sure we will be submitting work
                 * with it. Thus, if we return early, the fence is still signaled and vkWaitForFences wont deadlock the
                 * next time we use the same fence object
                */
                inFlightFenObj->resetFence();

                /* Note on synchronization
                 * (1) Everything submitted to a queue is simply a linear stream of commands. Any synchronization applies
                 *     globally to a VkQueue, there is no concept of a only-inside-this-command-buffer synchronization
                 *
                 * (2) The specification states that commands start execution in-order, but complete out-of-order. Don’t
                 *     get confused by this. The fact that commands start in-order is simply convenient language to make
                 *     the spec language easier to write. Unless you add synchronization yourself, all commands in a
                 *     queue execute out of order. This makes sense, considering that Vulkan only sees a linear stream
                 *     of commands once you submit, it is a pitfall to assume that splitting command buffers or submits
                 *     adds some magic synchronization for you
                 *
                 * (3) Frame buffer operations inside a render pass happen in API-order, of course. This is a special
                 *     exception which the spec calls out
                */
                resetCmdBufferRecording (cmdBuffer, 0);
                beginCmdBufferRecording (cmdBuffer, 0);
                return true;
            }

            void endFrame (void) {
                auto& frameIdx            = m_rendererInfo.meta.frameInFlightIdx;
                auto& resource            = m_rendererInfo.resource;
                auto inFlightFenObj       = resource.inFlightFenObjs[frameIdx];
                auto imageAvailableSemObj = resource.imageAvailableSemObjs[frameIdx];
                auto renderDoneSemObj     = resource.renderDoneSemObjs[frameIdx];
                auto cmdBuffer            = resource.cmdBufferObj->getCmdBuffers()[frameIdx];
                endCmdBufferRecording (cmdBuffer);
                /* We want to wait with writing colors to the image until it's available, so we're specifying the stage
                 * of the graphics pipeline that writes to the color attachment. Note that, this means theoretically the
                 * implementation can already start executing our vertex shader and such while the image is not yet
                 * available
                */
                auto waitSemaphores   = std::vector {
                    *imageAvailableSemObj->getSemaphore()
                };
                auto waitStageMasks   = std::vector <VkPipelineStageFlags> {
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                };
                auto signalSemaphores = std::vector {
                    *renderDoneSemObj->getSemaphore()
                };
                auto cmdBuffers       = std::vector {
                    cmdBuffer
                };
                auto submitInfos      = std::vector <VkSubmitInfo> {};
                submitCmdBuffers (*resource.logDeviceObj->getGraphicsQueue(),
                                  *inFlightFenObj->getFence(),
                                  cmdBuffers,
                                  waitSemaphores, waitStageMasks, signalSemaphores,
                                  submitInfos);

                presentSwapChainImage();
                updateFrameInFlightIdx();
            }

            void onAttach (void) override {
                /* Do nothing */
            }

            void onDetach (void) override {
                /* Do nothing */
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKRenderer (void) {
                if (m_rendererInfo.state.logObjCreated)
                    delete m_rendererInfo.resource.logObj;
            }
    };
}   // namespace Renderer
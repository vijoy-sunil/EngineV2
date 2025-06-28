#pragma once
#define TINYOBJLOADER_IMPLEMENTATION
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Dependency/tinyobjloader/tiny_obj_loader.h"
#include "../../SBTexturePool.h"
#include "../../SBComponentType.h"
#include "../../SBRendererType.h"

namespace SandBox {
    class SYMeshLoading: public Scene::SNSystemBase {
        private:
            struct MeshLoadingInfo {
                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                    SBTexturePool* texturePoolObj;
                } resource;
            } m_meshLoadingInfo;

            uint32_t transformToRange (const uint32_t oldValue,
                                       const std::pair <uint32_t, uint32_t> oldRange,
                                       const std::pair <uint32_t, uint32_t> newRange) {
                return (
                    ((oldValue - oldRange.first) / static_cast <float> (oldRange.second - oldRange.first)) *
                    (newRange.second - newRange.first)
                ) +  newRange.first;
            }

            void loadOBJModel (MeshComponent* meshComponent,
                               TextureIdxLUTComponent* textureIdxLUTComponent) {

                auto& texturePoolObj = m_meshLoadingInfo.resource.texturePoolObj;

                std::unordered_map <Vertex, IndexType> uniqueVertices;
                tinyobj::ObjReaderConfig readerConfig;
                tinyobj::ObjReader reader;
                /* Config */
                readerConfig.mtl_search_path = meshComponent->m_mtlFileDirPath;
                readerConfig.triangulate     = true;

                if (!reader.ParseFromFile (meshComponent->m_modelFilePath, readerConfig)) {
                    if (!reader.Error().empty()) {
                        std::string msg = reader.Error().substr (0, reader.Error().size() - 1);
                        LOG_ERROR (m_meshLoadingInfo.resource.logObj) << msg
                                                                      << " "
                                                                      << "[" << meshComponent->m_modelFilePath << "]"
                                                                      << std::endl;
                        throw std::runtime_error (msg);
                    }
                }
                if (!reader.Warning().empty()) {
                    std::string msg = reader.Warning().substr (0, reader.Warning().size() - 1);
                    LOG_WARNING (m_meshLoadingInfo.resource.logObj)   << msg
                                                                      << " "
                                                                      << "[" << meshComponent->m_modelFilePath << "]"
                                                                      << std::endl;
                }

                auto attribute = reader.GetAttrib();
                auto shapes    = reader.GetShapes();
                auto materials = reader.GetMaterials();

                /* Encode texture indices for default textures */
                textureIdxLUTComponent->encodeTextureIdx (0, 0);
                textureIdxLUTComponent->encodeTextureIdx (1, 1);
                textureIdxLUTComponent->encodeTextureIdx (2, 2);
                /* Populate texture pool */
                for (auto const& material: materials) {
                    if (!material.diffuse_texname.empty()) {
                        texturePoolObj->addTextureToPool (
                            material.diffuse_texname
                        );
                        textureIdxLUTComponent->encodeTextureIdx (
                            texturePoolObj->getTextureIdx (material.diffuse_texname),
                            texturePoolObj->getTextureIdx (material.diffuse_texname)
                        );
                    }
                    if (!material.specular_texname.empty()) {
                        texturePoolObj->addTextureToPool (
                            material.specular_texname
                        );
                        textureIdxLUTComponent->encodeTextureIdx (
                            texturePoolObj->getTextureIdx (material.specular_texname),
                            texturePoolObj->getTextureIdx (material.specular_texname)
                        );
                    }
                    if (!material.emissive_texname.empty()) {
                        texturePoolObj->addTextureToPool (
                            material.emissive_texname
                        );
                        textureIdxLUTComponent->encodeTextureIdx (
                            texturePoolObj->getTextureIdx (material.emissive_texname),
                            texturePoolObj->getTextureIdx (material.emissive_texname)
                        );
                    }
                }
                /* attrib_t contains single and linear array of vertex data (vertices, normals and texcoords)
                 *
                 * attrib_t::vertices => 3 floats per vertex
                 *     p[0]        p[1]        p[2]        p[3]               p[n-1]
                 * +-----------+-----------+-----------+-----------+      +-----------+
                 * | x | y | z | x | y | z | x | y | z | x | y | z | .... | x | y | z |
                 * +-----------+-----------+-----------+-----------+      +-----------+
                 *
                 * attrib_t::normals => 3 floats per vertex
                 *     n[0]        n[1]        n[2]        n[3]               n[n-1]
                 * +-----------+-----------+-----------+-----------+      +-----------+
                 * | x | y | z | x | y | z | x | y | z | x | y | z | .... | x | y | z |
                 * +-----------+-----------+-----------+-----------+      +-----------+
                 *
                 * attrib_t::texcoords => 2 floats per vertex
                 *     t[0]        t[1]        t[2]        t[3]               t[n-1]
                 * +-----------+-----------+-----------+-----------+      +-----------+
                 * |  u  |  v  |  u  |  v  |  u  |  v  |  u  |  v  | .... |  u  |  v  |
                 * +-----------+-----------+-----------+-----------+      +-----------+
                 *
                 * Each shape_t::mesh_t contains array index to attrib_t, and each index has an index to
                 * attrib_t::vertices, attrib_t::normals and attrib_t::texcoords
                 *
                 * mesh_t::num_face_vertices => array of the number of vertices per face (e.g. 3 = triangle etc.)
                 * +---+---+---+        +---+
                 * | 3 | 4 | 3 | ...... | 3 |
                 * +---+---+---+        +---+
                 * |   |   |            |
                 * |   |   |            +-----------------------------------------+
                 * |   |   |                                                      |
                 * |   |   +------------------------------+                       |
                 * |   |                                  |                       |
                 * |   +------------------+               |                       |
                 * |                      |               |                       |
                 * v                      v               v                       v
                 *
                 * mesh_t::indices
                 * |    face[0]   |       face[1]     |    face[2]   |     |      face[n-1]           |
                 * +----+----+----+----+----+----+----+----+----+----+     +--------+--------+--------+
                 * | i0 | i1 | i2 | i3 | i4 | i5 | i6 | i7 | i8 | i9 | ... | i(n-3) | i(n-2) | i(n-1) |
                 * +----+----+----+----+----+----+----+----+----+----+     +--------+--------+--------+
                 * Note that when triangulate flag is set to true (true by default), num_face_vertices are all filled
                 * with 3
                */
                for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++) {
                    size_t idxOffset = 0;

                    for (size_t faceIdx = 0; faceIdx < shapes[shapeIdx].mesh.num_face_vertices.size(); faceIdx++) {
                        int materialIdx        = shapes[shapeIdx].mesh.material_ids[faceIdx];
                        size_t verticesPerFace = size_t (shapes[shapeIdx].mesh.num_face_vertices[faceIdx]);

                        for (size_t vertexIdx = 0; vertexIdx < verticesPerFace; vertexIdx++) {
                            auto idx = shapes[shapeIdx].mesh.indices[idxOffset + vertexIdx];

                            auto tx  = attribute.texcoords[2 * size_t (idx.texcoord_index) + 0];
                            auto ty  = attribute.texcoords[2 * size_t (idx.texcoord_index) + 1];

                            auto nx  = attribute.normals  [3 * size_t (idx.normal_index)   + 0];
                            auto ny  = attribute.normals  [3 * size_t (idx.normal_index)   + 1];
                            auto nz  = attribute.normals  [3 * size_t (idx.normal_index)   + 2];

                            auto px  = attribute.vertices [3 * size_t (idx.vertex_index)   + 0];
                            auto py  = attribute.vertices [3 * size_t (idx.vertex_index)   + 1];
                            auto pz  = attribute.vertices [3 * size_t (idx.vertex_index)   + 2];

                            Vertex vertex;
                            /* The OBJ format assumes a coordinate system where a vertical coordinate of 0 means the
                             * bottom of the image, however we've uploaded our image into vulkan in a top to bottom
                             * orientation where 0 means the top of the image. We can solve this by flipping the vertical
                             * component of the texture coordinates
                             *
                             * (0, 0)-----------(1, 0)  top ^
                             * |                |
                             * |     (u, v)     |
                             * |                |
                             * (0, 1)-----------(1, 1)  bottom v
                             *
                             * In Vulkan,
                             * the u coordinate goes from 0.0 to 1.0, left to right
                             * the v coordinate goes from 0.0 to 1.0, top to bottom
                            */
                            vertex.meta.uv       = {
                                tx,
                                1.0f - ty
                            };
                            vertex.meta.normal   = {
                                nx, ny, nz
                            };
                            vertex.meta.position = {
                                px, py, pz
                            };
                            /* Texture indices will be set to default diffuse/speculat/emission if,
                             * (1) no texture present in the material
                             * (2) no material present for the current face/vertex
                            */
                            if (materialIdx != -1) {
                                auto material                      = materials[materialIdx];
                                vertex.material.diffuseTextureIdx  = material.diffuse_texname  == "" ?
                                      0:
                                      texturePoolObj->getTextureIdx (material.diffuse_texname);
                                vertex.material.specularTextureIdx = material.specular_texname == "" ?
                                      1:
                                      texturePoolObj->getTextureIdx (material.specular_texname);
                                vertex.material.emissionTextureIdx = material.emissive_texname == "" ?
                                      2:
                                      texturePoolObj->getTextureIdx (material.emissive_texname);
                                vertex.material.shininess          = transformToRange (
                                    material.shininess,
                                    {0,  900},
                                    {32, 128}
                                );
                            }
                            else {
                                vertex.material.diffuseTextureIdx  = 0;
                                vertex.material.specularTextureIdx = 1;
                                vertex.material.emissionTextureIdx = 2;
                                vertex.material.shininess          = 32;
                            }
                            /* To take advantage of the index buffer, we should keep only the unique vertices and use the
                             * index buffer to reuse them whenever they come up. Every time we read a vertex from the OBJ
                             * file, we check if we've already seen a vertex with the exact same attributes before. If
                             * not, we add it to vertices array and store its index in the map container. After that we
                             * add the index of the new vertex to indices array
                             *
                             * If we've seen the exact same vertex before, then we look up its index in the map container
                             * and store that index in indices array
                            */
                            if (uniqueVertices.count (vertex) == 0) {
                                uniqueVertices[vertex] = static_cast <IndexType> (
                                    meshComponent->m_vertices.size()
                                );
                                meshComponent->m_vertices.push_back (vertex);
                            }
                            meshComponent->m_indices.push_back (uniqueVertices[vertex]);
                        }
                        idxOffset += verticesPerFace;
                    }
                }
            }

        public:
            SYMeshLoading (void) {
                m_meshLoadingInfo = {};

                auto& logObj = m_meshLoadingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            /* Note that, system constructors DO NOT take in any arguments, as a result dependencies are passed in here */
            void initMeshLoadingInfo (Scene::SNImpl* sceneObj,
                                      SBTexturePool* texturePoolObj) {

                if (sceneObj == nullptr || texturePoolObj == nullptr) {
                    LOG_ERROR (m_meshLoadingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                  << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_meshLoadingInfo.resource.sceneObj       = sceneObj;
                m_meshLoadingInfo.resource.texturePoolObj = texturePoolObj;
            }

            void update (void) {
                auto& sceneObj = m_meshLoadingInfo.resource.sceneObj;

                for (auto const& entity: m_entities) {
                    auto meshComponent          = sceneObj->getComponent <MeshComponent>          (entity);
                    auto textureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (entity);
                    /* Clear previous loaded data */
                    meshComponent->m_vertices.clear();
                    meshComponent->m_indices.clear();
                    textureIdxLUTComponent->fillTextureIdxLUT (0);

                    loadOBJModel (meshComponent, textureIdxLUTComponent);
                }
            }

            ~SYMeshLoading (void) {
                delete m_meshLoadingInfo.resource.logObj;
            }
    };
}   // namespace SandBox
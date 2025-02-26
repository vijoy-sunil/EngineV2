#define TINYOBJLOADER_IMPLEMENTATION
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "../Dependency/tinyobjloader/tiny_obj_loader.h"
#include "../SandBox/Config/SBRendererType.h"

namespace Utility {
    struct ModelData {
        struct Meta {
            /* Note that the attributes are combined into one array of vertices (interleaving vertex attributes) */
            std::vector <SandBox::Vertex> vertices;
            std::vector <SandBox::IndexType> indices;
        } meta;

        struct Path {
            std::vector <std::string> diffuseTextures;
            std::vector <std::string> specularTextures;
            std::vector <std::string> emissionTextures;
        } path;
    };

    ModelData loadModelData (const char* modelFilePath, const char* mtlFileDirPath) {
        ModelData modelData;
        std::unordered_map <SandBox::Vertex, SandBox::IndexType> uniqueVertices;
        std::unordered_map <std::string, int32_t> pathToIdxMap;

        tinyobj::ObjReaderConfig readerConfig;
        tinyobj::ObjReader reader;
        /* Config */
        readerConfig.mtl_search_path = mtlFileDirPath;
        readerConfig.triangulate     = true;

        if (!reader.ParseFromFile (modelFilePath, readerConfig)) {
            if (!reader.Error().empty())
                throw std::runtime_error (reader.Error());
        }
        /* Throw exception for warnings as well */
        if (!reader.Warning().empty())
            throw std::runtime_error (reader.Warning());

        auto attribute = reader.GetAttrib();
        auto shapes    = reader.GetShapes();
        auto materials = reader.GetMaterials();

        /* Load paths (unique, non-empty) and compute its indices from .mtl file */
        pathToIdxMap.insert (
            {
                "",
                -1
            }
        );
        for (auto const& material: materials) {
            if (!material.diffuse_texname.empty()) {
                if (pathToIdxMap.find (material.diffuse_texname) == pathToIdxMap.end()) {

                    modelData.path.diffuseTextures.push_back (material.diffuse_texname);
                    pathToIdxMap.insert (
                        {
                            material.diffuse_texname,
                            modelData.path.diffuseTextures.size() - 1
                        }
                    );
                }
            }
            if (!material.specular_texname.empty()) {
                if (pathToIdxMap.find (material.specular_texname) == pathToIdxMap.end()) {

                    modelData.path.specularTextures.push_back (material.specular_texname);
                    pathToIdxMap.insert (
                        {
                            material.specular_texname,
                            modelData.path.specularTextures.size() - 1
                        }
                    );
                }
            }
            if (!material.emissive_texname.empty()) {
                if (pathToIdxMap.find (material.emissive_texname) == pathToIdxMap.end()) {

                    modelData.path.emissionTextures.push_back (material.emissive_texname);
                    pathToIdxMap.insert (
                        {
                            material.emissive_texname,
                            modelData.path.emissionTextures.size() - 1
                        }
                    );
                }
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
         * Each shape_t::mesh_t contains array index to attrib_t, and each index has an index to attrib_t::vertices,
         * attrib_t::normals and attrib_t::texcoords
         *
         * mesh_t::num_face_vertices => array of the number of vertices per face (e.g. 3 = triangle, 4 = quad etc.)
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
         * Note that when triangulate flag is set to true (true by default), num_face_vertices are all filled with 3
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

                    SandBox::Vertex vertex;
                    /* The OBJ format assumes a coordinate system where a vertical coordinate of 0 means the bottom of
                     * the image, however we've uploaded our image into vulkan in a top to bottom orientation where 0
                     * means the top of the image. We can solve this by flipping the vertical component of the texture
                     * coordinates
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
                    /* Texture indices will be -1 if,
                     * (1) no material present for the current face/vertex
                     * (2) no texture present in the material
                    */
                    if (materialIdx != -1) {
                        auto material                      = materials[materialIdx];
                        vertex.material.diffuseTextureIdx  = pathToIdxMap[material.diffuse_texname];
                        vertex.material.specularTextureIdx = pathToIdxMap[material.specular_texname];
                        vertex.material.emissionTextureIdx = pathToIdxMap[material.emissive_texname];
                    }
                    else {
                        vertex.material.diffuseTextureIdx  = -1;
                        vertex.material.specularTextureIdx = -1;
                        vertex.material.emissionTextureIdx = -1;
                    }
                    /* Shininess will be populated post parsing */
                    vertex.material.shininess = 0;
                    /* To take advantage of the index buffer, we should keep only the unique vertices and use the index
                     * buffer to reuse them whenever they come up. Every time we read a vertex from the OBJ file, we
                     * check if we've already seen a vertex with the exact same attributes before. If not, we add it to
                     * vertices array and store its index in the map container. After that we add the index of the new
                     * vertex to indices array
                     *
                     * If we've seen the exact same vertex before, then we look up its index in the map container and
                     * store that index in indices array
                    */
                    if (uniqueVertices.count (vertex) == 0) {
                        uniqueVertices[vertex] = static_cast <SandBox::IndexType> (
                            modelData.meta.vertices.size()
                        );
                        modelData.meta.vertices.push_back (vertex);
                    }
                    modelData.meta.indices.push_back (uniqueVertices[vertex]);
                }
                idxOffset += verticesPerFace;
            }
        }
        return modelData;
    }
}   // namespace Utility
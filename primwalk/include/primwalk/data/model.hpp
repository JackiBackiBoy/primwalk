#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/data/mesh.hpp"
#include "primwalk/rendering/buffer.hpp"
#include "primwalk/rendering/vertex3d.hpp"

// std
#include <cstdint>
#include <string>
#include <vector>

// vendor
#include <glm/glm.hpp>
#include <assimp/scene.h>

namespace pw {
  class PW_API Model {
  public:
    Model() {};
    ~Model() = default;

    void loadFromFile(const std::string& path);
    void draw(VkCommandBuffer commandBuffer);

    std::vector<Mesh>& getMeshes() { return m_Meshes; }

  private:
    void createVertexBuffer(const std::vector<Vertex3D>& vertices);
    void createIndexBuffer(const std::vector<uint32_t>& indices);
    void initFromScene(const aiScene* scene);
    void initMeshes(const aiScene* scene);
    void initSingleMesh(const aiMesh* mesh);
    void initMaterials(const aiScene* scene, const std::string& modelDir);
    void countVerticesIndices(const aiScene* scene, uint32_t& numVertices, uint32_t& numIndices);
    void reserveSpace(const uint32_t& numVertices, const uint32_t& numIndices);
    void getTexturePath(const aiMaterial* material, const aiTextureType& texType, aiString& dstPath);

    std::vector<Mesh> m_Meshes{};
    std::vector<glm::vec3> m_Positions{};
    std::vector<glm::vec3> m_Normals{};
    std::vector<glm::vec2> m_TexCoords{};
    std::vector<Vertex3D> m_Vertices{};
    std::vector<uint32_t> m_Indices{};

    uint32_t m_VertexCount = 0;
    uint32_t m_IndexCount = 0;
    std::unique_ptr<Buffer> m_VertexBuffer;
    std::unique_ptr<Buffer> m_IndexBuffer;
  };
}
#include "primwalk/data/model.hpp"
#include <assimp/scene.h>

// std
#include <stdexcept>

// vendor
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace pw {

  void Model::loadFromFile(const std::string& path)
  {
    std::string enginePath = BASE_DIR + path;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
      enginePath,
      aiProcess_Triangulate |
      aiProcess_FlipUVs |
      aiProcess_JoinIdenticalVertices |
      aiProcess_CalcTangentSpace);

    if (!scene) {
      std::string error = importer.GetErrorString();
      throw std::runtime_error("ASSIMP ERROR: " + error);
    }

    // Get model directory to locate model resources
    std::string modelDir = enginePath.substr(0, enginePath.find_last_of('/'));

    initFromScene(scene);
    initMaterials(scene, modelDir);

    createVertexBuffer(m_Vertices);
    createIndexBuffer(m_Indices);
  }

  void Model::draw(VkCommandBuffer commandBuffer)
  {
    VkBuffer buffers[] = { m_VertexBuffer->getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

    for (size_t i = 0; i < m_Meshes.size(); i++) {
      vkCmdDrawIndexed(commandBuffer, m_Meshes[i].indices, 1, m_Meshes[i].baseIndex, m_Meshes[i].baseVertex, 0);
    }
  }

  void Model::createVertexBuffer(const std::vector<Vertex3D>& vertices)
  {
    GraphicsDevice_Vulkan* device = pw::GetDevice();

    m_VertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_VertexCount >= 3 && "VULKAN ASSERTION FAILED: Vertex count must be >= 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    Buffer stagingBuffer {
      *device,
      vertexSize,
      m_VertexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)vertices.data());

    m_VertexBuffer = std::make_unique<Buffer>(
      *device,
      vertexSize,
      m_VertexCount,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    device->copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
  }

  void Model::createIndexBuffer(const std::vector<uint32_t>& indices)
  {
    GraphicsDevice_Vulkan* device = pw::GetDevice();

    m_IndexCount = static_cast<uint32_t>(indices.size());

    VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
    uint32_t indexSize = sizeof(indices[0]);

    Buffer stagingBuffer = {
      *device,
      indexSize,
      m_IndexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)indices.data());

    m_IndexBuffer = std::make_unique<Buffer>(
      *device,
      indexSize,
      m_IndexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    device->copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);
  }

  void Model::initFromScene(const aiScene* scene)
  {
    m_Meshes.resize(scene->mNumMeshes);
    // TODO: Count materials and automatically import

    uint32_t numVertices = 0;
    uint32_t numIndices = 0;

    countVerticesIndices(scene, numVertices, numIndices);
    reserveSpace(numVertices, numIndices);
    initMeshes(scene);
  }

  void Model::initMeshes(const aiScene* scene)
  {
    for (size_t i = 0; i < m_Meshes.size(); i++) {
      const aiMesh* mesh = scene->mMeshes[i];
      initSingleMesh(mesh);
    }
  }

  void Model::initSingleMesh(const aiMesh* mesh)
  {
    const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

    // Populate vertices for each mesh
    for (size_t i = 0; i < mesh->mNumVertices; i++) {
      const aiVector3D& position = mesh->mVertices[i];
      const aiVector3D& normal = mesh->mNormals[i];
      const aiVector3D& tangent = mesh->mTangents[i];
      const aiVector3D& bitangent = mesh->mBitangents[i];
      const aiVector3D& texCoord = mesh->HasTextureCoords(0) ?
        mesh->mTextureCoords[0][i] : zero3D;

      m_Positions.push_back(glm::vec3(position.x, position.y, position.z));
      m_Normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
      m_TexCoords.push_back(glm::vec2(texCoord.x, texCoord.y));

      m_Vertices.push_back(
        { glm::vec3(position.x, position.y, position.z),
          glm::vec3(normal.x, normal.y, normal.z),
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec2(texCoord.x, texCoord.y) });
    }

    // Populate indices
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
      const aiFace& face = mesh->mFaces[i];
      assert(face.mNumIndices == 3);

      m_Indices.push_back(face.mIndices[0]);
      m_Indices.push_back(face.mIndices[1]);
      m_Indices.push_back(face.mIndices[2]);
    }
  }

  void Model::initMaterials(const aiScene* scene, const std::string& modelDir)
  {

  }

  void Model::countVerticesIndices(const aiScene* scene, uint32_t& numVertices, uint32_t& numIndices)
  {
    for (size_t i = 0; i < m_Meshes.size(); i++) {
      m_Meshes[i].materialIndex = static_cast<uint32_t>(scene->mMeshes[i]->mMaterialIndex);
      m_Meshes[i].indices = static_cast<uint32_t>(scene->mMeshes[i]->mNumFaces * 3);
      m_Meshes[i].baseVertex = numVertices; // first vertex of current mesh
      m_Meshes[i].baseIndex = numIndices; // first index of current mesh

      numVertices += static_cast<uint32_t>(scene->mMeshes[i]->mNumVertices);
      numIndices += m_Meshes[i].indices;
    }
  }

  void Model::reserveSpace(const uint32_t& numVertices, const uint32_t& numIndices)
  {
    m_Positions.reserve(numVertices);
    m_Normals.reserve(numVertices);
    m_TexCoords.reserve(numVertices);
    m_Indices.reserve(numIndices);
  }

  void Model::getTexturePath(const aiMaterial* material, const aiTextureType& texType, aiString& dstPath)
  {

  }

}
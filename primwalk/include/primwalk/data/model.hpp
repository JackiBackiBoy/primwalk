#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/data/mesh.hpp"
#include "primwalk/rendering/buffer.hpp"
#include "primwalk/rendering/vertex3d.hpp"
#include "primwalk/rendering/texture2D.hpp"

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
		std::vector<std::shared_ptr<Texture2D>>& getTextures() { return m_DiffuseTextures; }

	private:
		void createVertexBuffer(const std::vector<Vertex3D>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);
		void initFromScene(const aiScene* scene);
		void initMeshes(const aiScene* scene);
		void initMaterials(const aiScene* scene, const std::string& modelDir);
		void countVerticesIndices(const aiScene* scene, uint32_t& numVertices, uint32_t& numIndices);
		void reserveSpace(const uint32_t& numVertices, const uint32_t& numIndices);
		void getTexturePath(const aiMaterial* material, const aiTextureType& texType, aiString& dstPath);

		std::vector<Mesh> m_Meshes{};
		std::vector<Vertex3D> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		std::vector<std::shared_ptr<Texture2D>> m_DiffuseTextures{};

		std::unique_ptr<Buffer> m_VertexBuffer;
		std::unique_ptr<Buffer> m_IndexBuffer;
	};
}
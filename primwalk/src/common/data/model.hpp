#pragma once

// primwalk
#include "../../core.hpp"
#include "mesh.hpp"
#include "../rendering/buffer.hpp"
#include "../rendering/vertex3d.hpp"
#include "../rendering/texture2D.hpp"

// std
#include <cstdint>
#include <string>
#include <unordered_map>
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
		void bind(VkCommandBuffer commandBuffer);

		std::vector<Mesh>& getMeshes() { return m_Meshes; }
		std::shared_ptr<Texture2D> getDiffuseMap(uint32_t materialIndex);
		std::shared_ptr<Texture2D> getNormalMap(uint32_t materialIndex);

	private:
		void createVertexBuffer(const std::vector<Vertex3D>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);
		void initFromScene(const aiScene* scene);
		void initMeshes(const aiScene* scene);
		void initMaterials(const aiScene* scene, const std::string& modelDir);
		void countVerticesIndices(const aiScene* scene, uint32_t& numVertices, uint32_t& numIndices);
		void reserveSpace(const uint32_t& numVertices, const uint32_t& numIndices);
		void getTexturePath(const aiMaterial* material, const aiTextureType& texType, aiString& dstPath);
		std::shared_ptr<Texture2D> getEmbeddedTexture(const aiMaterial* material, const aiScene* scene, const aiTextureType& texType, const std::string& modelDir);

		std::vector<Mesh> m_Meshes{};
		std::vector<Vertex3D> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		std::unordered_map<uint32_t, std::shared_ptr<Texture2D>> m_DiffuseMaps{};
		std::unordered_map<uint32_t, std::shared_ptr<Texture2D>> m_NormalMaps{};

		std::unique_ptr<Buffer> m_VertexBuffer;
		std::unique_ptr<Buffer> m_IndexBuffer;
	};
}
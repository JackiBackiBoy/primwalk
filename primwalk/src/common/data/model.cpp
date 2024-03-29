#include "model.hpp"
#include <assimp/scene.h>

// std
#include <stdexcept>

// vendor
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace pw {

	void Model::loadFromFile(const std::string& path) {
		std::string enginePath = BASE_DIR + path;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			enginePath,
			aiProcess_Triangulate |
			aiProcess_CalcTangentSpace |
			aiProcess_PreTransformVertices |
			aiProcess_FlipUVs);

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

	void Model::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { m_VertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	std::shared_ptr<pw::Texture2D> Model::getDiffuseMap(uint32_t materialIndex) {
		if (m_DiffuseMaps.empty() || materialIndex >= m_DiffuseMaps.size()) {
			return nullptr;
		}

		return m_DiffuseMaps[materialIndex];
	}

	std::shared_ptr<pw::Texture2D> Model::getNormalMap(uint32_t materialIndex) {
		if (m_NormalMaps.empty() || materialIndex >= m_NormalMaps.size()) {
			return nullptr;
		}

		return m_NormalMaps[materialIndex];
	}

	void Model::createVertexBuffer(const std::vector<Vertex3D>& vertices) {
		GraphicsDevice_Vulkan* device = (GraphicsDevice_Vulkan*&)pw::GetDevice();

		uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "VULKAN ASSERTION FAILED: Vertex count must be >= 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		Buffer stagingBuffer = {
			*device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		m_VertexBuffer = std::make_unique<Buffer>(
			*device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		device->copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
	}

	void Model::createIndexBuffer(const std::vector<uint32_t>& indices) {
		GraphicsDevice_Vulkan* device = (GraphicsDevice_Vulkan*&)pw::GetDevice();

		uint32_t indexCount = static_cast<uint32_t>(indices.size());

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer = {
			*device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		m_IndexBuffer = std::make_unique<Buffer>(
			*device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		device->copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);
	}

	void Model::initFromScene(const aiScene* scene) {
		m_Meshes.resize(scene->mNumMeshes);
		m_DiffuseMaps.reserve(scene->mNumMaterials); // TODO: Fix material count

		uint32_t numVertices = 0;
		uint32_t numIndices = 0;

		countVerticesIndices(scene, numVertices, numIndices);
		reserveSpace(numVertices, numIndices);
		initMeshes(scene);
	}

	void Model::initMeshes(const aiScene* scene) {
		const aiVector3D zero3D(0.0f, 0.0f, 0.0f);
		const aiMesh* mesh = nullptr;
		size_t vertexID = 0;
		size_t indexID = 0;
		
		for (size_t i = 0; i < m_Meshes.size(); i++) {
			mesh = scene->mMeshes[i];

			// Populate vertices
			for (size_t j = 0; j < mesh->mNumVertices; j++) {
				const aiVector3D& position = mesh->mVertices[j];
				const aiVector3D& normal = mesh->mNormals[j];
				const aiVector3D& tangent = mesh->mTangents[j];
				const aiVector3D& bitangent = mesh->mBitangents[j];
				const aiVector3D& texCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : zero3D;

				m_Vertices[vertexID].position = glm::vec3(position.x, position.y, position.z);
				m_Vertices[vertexID].normal = glm::vec3(normal.x, normal.y, normal.z);

				if (mesh->mTangents != nullptr) {
					m_Vertices[vertexID].tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
				}

				if (mesh->mBitangents != nullptr) {
					m_Vertices[vertexID].bitangent = glm::vec3(bitangent.x, bitangent.y, bitangent.z);
				}
				m_Vertices[vertexID].texCoord = glm::vec2(texCoord.x, texCoord.y);
				vertexID++;
			}

			// Populate indices
			for (size_t j = 0; j < mesh->mNumFaces; j++) {
				const aiFace& face = mesh->mFaces[j];
				assert(face.mNumIndices == 3);

				m_Indices[indexID] = face.mIndices[0];
				m_Indices[indexID + 1] = face.mIndices[1];
				m_Indices[indexID + 2] = face.mIndices[2];
				indexID += 3;
			}
		}
	}

	void Model::initMaterials(const aiScene* scene, const std::string& modelDir) {
		for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
			const aiMaterial* material = scene->mMaterials[i];

			auto diffuseMap = getEmbeddedTexture(material, scene, aiTextureType_DIFFUSE, modelDir);
			auto normalMap = getEmbeddedTexture(material, scene, aiTextureType_NORMALS, modelDir);

			if (diffuseMap) { m_DiffuseMaps.insert({ i, diffuseMap }); }
			if (normalMap) { m_NormalMaps.insert({ i, normalMap }); }
		}
	}

	void Model::countVerticesIndices(const aiScene* scene, uint32_t& numVertices, uint32_t& numIndices) {
		for (size_t i = 0; i < m_Meshes.size(); i++) {
			m_Meshes[i].materialIndex = static_cast<uint32_t>(scene->mMeshes[i]->mMaterialIndex);
			m_Meshes[i].indices = static_cast<uint32_t>(scene->mMeshes[i]->mNumFaces * 3);
			m_Meshes[i].baseVertex = numVertices; // first vertex of current mesh
			m_Meshes[i].baseIndex = numIndices; // first index of current mesh

			numVertices += static_cast<uint32_t>(scene->mMeshes[i]->mNumVertices);
			numIndices += m_Meshes[i].indices;
		}
	}

	void Model::reserveSpace(const uint32_t& numVertices, const uint32_t& numIndices) {
		m_Vertices.resize(numVertices);
		m_Indices.resize(numIndices);
	}

	void Model::getTexturePath(const aiMaterial* material, const aiTextureType& texType, aiString& dstPath) {
		if (material->GetTexture(texType, 0, &dstPath, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
			throw std::runtime_error("ASSIMP ERROR: Failed to retrieve model texture!");
		}
	}

	std::shared_ptr<Texture2D>  Model::getEmbeddedTexture(const aiMaterial* material, const aiScene* scene, const aiTextureType& texType, const std::string& modelDir) {
		if (material->GetTextureCount(texType) == 0) {
			return nullptr;
		}

		aiString tempPath;

		material->GetTexture(texType, 0, &tempPath);
		const aiTexture* texture = scene->GetEmbeddedTexture(tempPath.C_Str());

		if (texture) { // has embedded texture
			return std::make_shared<Texture2D>(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth);
		}

		// read texture from the given texture path
		std::string fullPath = modelDir + "/" + tempPath.C_Str();
		return std::make_shared<Texture2D>(fullPath, 4, VK_FORMAT_R8G8B8A8_SRGB, true);
	}

}
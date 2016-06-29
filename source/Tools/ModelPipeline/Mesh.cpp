#include "pch.h"
#include "Mesh.h"

namespace ModelPipeline
{

	Mesh::Mesh(Model& model, aiMesh& mesh)
		: mModel(model), mMaterialIndex(-1), mName(mesh.mName.C_Str()), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices()
	{
		mMaterialIndex = mesh.mMaterialIndex;
		mName = "TestName";
		// Vertices
		mVertices.reserve(mesh.mNumVertices);
		for (UINT i = 0; i < mesh.mNumVertices; i++)
		{
			mVertices.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mVertices[i])));
		}

		// Normals
		if (mesh.HasNormals())
		{
			mNormals.reserve(mesh.mNumVertices);
			for (UINT i = 0; i < mesh.mNumVertices; i++)
			{
				mNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mNormals[i])));
			}
		}

		// Tangents and Binormals
		if (mesh.HasTangentsAndBitangents())
		{
			mTangents.reserve(mesh.mNumVertices);
			mBiNormals.reserve(mesh.mNumVertices);
			for (UINT i = 0; i < mesh.mNumVertices; i++)
			{
				mTangents.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mTangents[i])));
				mBiNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mBitangents[i])));
			}
		}

		// Texture Coordinates
		UINT uvChannelCount = mesh.GetNumUVChannels();
		for (UINT i = 0; i < uvChannelCount; i++)
		{
			std::shared_ptr<std::vector<XMFLOAT3>> textureCoordinates = std::shared_ptr<std::vector<XMFLOAT3>>(new std::vector<XMFLOAT3>());
			textureCoordinates->reserve(mesh.mNumVertices);
			mTextureCoordinates.push_back(textureCoordinates);

			aiVector3D* aiTextureCoordinates = mesh.mTextureCoords[i];
			for (UINT j = 0; j < mesh.mNumVertices; j++)
			{
				textureCoordinates->push_back(XMFLOAT3(reinterpret_cast<const float*>(&aiTextureCoordinates[j])));
			}
		}

		// Vertex Colors
		UINT colorChannelCount = mesh.GetNumColorChannels();
		for (UINT i = 0; i < colorChannelCount; i++)
		{
			std::shared_ptr<std::vector<XMFLOAT4>> vertexColors = std::shared_ptr<std::vector<XMFLOAT4>>(new std::vector<XMFLOAT4>());
			vertexColors->reserve(mesh.mNumVertices);
			mVertexColors.push_back(vertexColors);

			aiColor4D* aiVertexColors = mesh.mColors[i];
			for (UINT j = 0; j < mesh.mNumVertices; j++)
			{
				vertexColors->push_back(XMFLOAT4(reinterpret_cast<const float*>(&aiVertexColors[j])));
			}
		}

		// Faces (note: could pre-reserve if we limit primitive types)
		if (mesh.HasFaces())
		{
			mFaceCount = mesh.mNumFaces;
			for (UINT i = 0; i < mFaceCount; i++)
			{
				aiFace* face = &mesh.mFaces[i];

				for (UINT j = 0; j < face->mNumIndices; j++)
				{
					mIndices.push_back(face->mIndices[j]);
				}
			}
		}

		// Bones
		if (mesh.HasBones())
		{
			mBoneWeights.resize(mesh.mNumVertices);

			for (UINT i = 0; i < mesh.mNumBones; i++)
			{
				aiBone* meshBone = mesh.mBones[i];

				// Look up the bone in the model's hierarchy, or add it if not found.
				UINT boneIndex = 0U;
				std::string boneName = meshBone->mName.C_Str();
				auto boneMappingIterator = mModel.mBoneIndexMapping.find(boneName);
				if (boneMappingIterator != mModel.mBoneIndexMapping.end())
				{
					boneIndex = boneMappingIterator->second;
				}
				else
				{
					boneIndex = mModel.mBones.size();
					XMFLOAT4X4 aiOffsetMatrix(reinterpret_cast<const float*>(meshBone->mOffsetMatrix[0]));
					XMMATRIX offsetMatrix = XMLoadFloat4x4(&aiOffsetMatrix);
					XMFLOAT4X4 offset;
					XMStoreFloat4x4(&offset, XMMatrixTranspose(offsetMatrix));

					std::shared_ptr<Bone> modelBone(new Bone(boneName, boneIndex, offset));
					mModel.mBones.push_back(modelBone);
					mModel.mBoneIndexMapping[boneName] = boneIndex;
				}

				for (UINT i = 0; i < meshBone->mNumWeights; i++)
				{
					aiVertexWeight vertexWeight = meshBone->mWeights[i];
					mBoneWeights[vertexWeight.mVertexId].AddWeight(vertexWeight.mWeight, boneIndex);
				}
			}
		}
	}

	Mesh::~Mesh()
	{
	}

	Model& Mesh::GetModel()
	{
		return mModel;
	}

	std::shared_ptr<ModelMaterial> Mesh::GetMaterial()
	{
		if (mMaterialIndex < 0)
		{
			std::runtime_error("Invalid Material Index");
		}

		return mModel.Materials().at(mMaterialIndex);
	}

	const std::string& Mesh::Name() const
	{
		return mName;
	}

	const std::vector<XMFLOAT3>& Mesh::Vertices() const
	{
		return mVertices;
	}

	const std::vector<XMFLOAT3>& Mesh::Normals() const
	{
		return mNormals;
	}

	const std::vector<XMFLOAT3>& Mesh::Tangents() const
	{
		return mTangents;
	}

	const std::vector<XMFLOAT3>& Mesh::BiNormals() const
	{
		return mBiNormals;
	}

	const std::vector<std::shared_ptr<std::vector<XMFLOAT3>> >& Mesh::TextureCoordinates() const
	{
		return mTextureCoordinates;
	}

	const std::vector<std::shared_ptr<std::vector<XMFLOAT4>> >& Mesh::VertexColors() const
	{
		return mVertexColors;
	}

	UINT Mesh::FaceCount() const
	{
		return mFaceCount;
	}

	const std::vector<UINT>& Mesh::Indices() const
	{
		return mIndices;
	}

	Mesh::Mesh(Model& model)
		:mModel(model), mMaterialIndex(-1), mName(), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices()
	{
	}

}
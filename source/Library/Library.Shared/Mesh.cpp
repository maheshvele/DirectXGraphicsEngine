#include "pch.h"
#include "Mesh.h"

namespace Library
{

	Mesh::Mesh(Model& model)
		:mModel(model), mMaterialIndex(-1), mName(), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices()
	{
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

	const std::vector<BoneVertexWeights>& Mesh::BoneWeights() const
	{
		return mBoneWeights;
	}

	void Mesh::CreateIndexBuffer(ID3D11Buffer** indexBuffer)
	{
		assert(indexBuffer != nullptr);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = (UINT)(sizeof(UINT) * mIndices.size());
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = &mIndices[0];
		ThrowIfFailed(mModel.GetGame().GetDirectXDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, indexBuffer));
	}
}
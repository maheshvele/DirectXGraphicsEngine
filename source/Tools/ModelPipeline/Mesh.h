#pragma once

struct aiMesh;
using namespace DirectX;

namespace ModelPipeline
{
	class ModelMaterial;
	class BoneVertexWeights;

	class Mesh 
	{
		friend class Model;
		friend class OutputStreamHelper;
		friend class InputStreamHelper;

	public:
		Mesh(Model& model, std::shared_ptr<ModelMaterial> material);
		Mesh(Model& model);
		~Mesh();

		Model& GetModel();
		std::shared_ptr<ModelMaterial> GetMaterial();
		const std::string& Name() const;

		const std::vector<XMFLOAT3>& Vertices() const;
		const std::vector<XMFLOAT3>& Normals() const;
		const std::vector<XMFLOAT3>& Tangents() const;
		const std::vector<XMFLOAT3>& BiNormals() const;
		const std::vector<std::shared_ptr<std::vector<XMFLOAT3>> >& TextureCoordinates() const;
		const std::vector<std::shared_ptr<std::vector<XMFLOAT4>> >& VertexColors() const;
		unsigned int FaceCount() const;
		const std::vector<UINT>& Indices() const;

	private:
		Mesh(Model& model, aiMesh& mesh);
		Mesh(const Mesh& rhs);
		Mesh& operator=(const Mesh& rhs);

		Model& mModel;
		int	mMaterialIndex;
		std::string mName;
		std::vector<XMFLOAT3> mVertices;
		std::vector<XMFLOAT3> mNormals;
		std::vector<XMFLOAT3> mTangents;
		std::vector<XMFLOAT3> mBiNormals;
		std::vector<std::shared_ptr<std::vector<XMFLOAT3>> > mTextureCoordinates;
		std::vector<std::shared_ptr<std::vector<XMFLOAT4>> > mVertexColors;
		UINT mFaceCount;
		std::vector<UINT> mIndices;
		std::vector<BoneVertexWeights> mBoneWeights;
	};
}
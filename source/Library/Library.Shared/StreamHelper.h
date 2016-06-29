#pragma once

namespace Library
{
	enum TextureType;
	class BoneVertexWeights;
	class SceneNode;
	class ModelMaterial;
	class Model;
	class Mesh;
	class Bone;
	class Keyframe;
	class BoneAnimation;
	class AnimationClip;

#pragma region InputStreamHelper
	class InputStreamHelper
	{
	public:
		
		InputStreamHelper(const std::string& fileName);
		~InputStreamHelper();

		InputStreamHelper() = delete;
		InputStreamHelper& operator=(const InputStreamHelper& rhs) = delete;
		InputStreamHelper(const InputStreamHelper& rhs) = delete;

		void CloseStream();

		InputStreamHelper& operator >> (std::string& value);
		InputStreamHelper& operator >> (std::wstring& value);
		InputStreamHelper& operator >> (int &value);
		InputStreamHelper& operator >> (float &value);
		InputStreamHelper& operator >> (std::uint32_t &value);
		InputStreamHelper& operator >> (DirectX::XMFLOAT3& value);
		InputStreamHelper& operator >> (DirectX::XMFLOAT4& value);
		InputStreamHelper& operator >> (TextureType& value);
		InputStreamHelper& operator >> (BoneVertexWeights& value);
		InputStreamHelper& operator >> (DirectX::XMFLOAT4X4& value);
		InputStreamHelper& operator >> (Model& value);
		InputStreamHelper& operator >> (Mesh& value);
		InputStreamHelper& operator >> (ModelMaterial& value);
		InputStreamHelper& operator >> (AnimationClip& value);
		InputStreamHelper& operator >> (BoneAnimation& value);
		InputStreamHelper& operator >> (Keyframe& value);
		InputStreamHelper& operator >> (Bone& value);
		SceneNode* DeSerializeSkeleton(Model& model, SceneNode* parentNode);

	private:
		std::ifstream mInputStream;
	};
#pragma endregion InputStreamHelper
}
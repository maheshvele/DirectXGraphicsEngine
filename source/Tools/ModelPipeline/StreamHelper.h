#pragma once

namespace ModelPipeline
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

#pragma region OutputStreamHelper

	class OutputStreamHelper
	{
	public:
		OutputStreamHelper(std::string& fileName);
		~OutputStreamHelper();

		OutputStreamHelper() = delete;
		OutputStreamHelper& operator=(const OutputStreamHelper& rhs) = delete;
		OutputStreamHelper(const OutputStreamHelper& rhs) = delete;
	
		void CloseStream();

		OutputStreamHelper& operator << (const std::string& value);
		OutputStreamHelper& operator << (const std::wstring& value);
		OutputStreamHelper& operator << (int value);
		OutputStreamHelper& operator << (float value);
		OutputStreamHelper& operator << (std::uint32_t value);
		OutputStreamHelper& operator << (DirectX::XMFLOAT3& value);
		OutputStreamHelper& operator << (DirectX::XMFLOAT4& value);
		OutputStreamHelper& operator << (TextureType value);
		OutputStreamHelper& operator << (BoneVertexWeights& value);
		OutputStreamHelper& operator << (DirectX::XMFLOAT4X4& value);
		OutputStreamHelper& operator << (Model& value);
		OutputStreamHelper& operator << (Mesh& value);
		OutputStreamHelper& operator << (ModelMaterial& value);
		OutputStreamHelper& operator << (AnimationClip& value);
		OutputStreamHelper& operator << (BoneAnimation& value);
		OutputStreamHelper& operator << (Keyframe& value);
		OutputStreamHelper& operator << (Bone& value);
		void SerializeSkeleton(Model& model, SceneNode* parentNode);

	private:

		std::ofstream mOutputStream;
	};
#pragma  endregion OutputStreamHelper

#pragma region InputStreamHelper
	class InputStreamHelper
	{
	public:
		
		InputStreamHelper(std::string& fileName);
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
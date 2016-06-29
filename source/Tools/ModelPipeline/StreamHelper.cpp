#include "pch.h"
#include "StreamHelper.h"

namespace ModelPipeline
{
#pragma  region OutputStream
	OutputStreamHelper::OutputStreamHelper(std::string& fileName)
	{
		mOutputStream.open(fileName, std::ios::binary | std::ios::out | std::ios::trunc);

		if (!mOutputStream)
		{
			throw std::runtime_error("OutputStreamHelper : Unable to open file!!");
		}
	}

	OutputStreamHelper::~OutputStreamHelper()
	{
		mOutputStream.close();
	}

	void OutputStreamHelper::CloseStream()
	{
		mOutputStream.close();
	}

	OutputStreamHelper& OutputStreamHelper::operator << (const std::string& value)
	{
		std::uint32_t inputLength = value.length();

		mOutputStream.write(reinterpret_cast<char*>(&inputLength), sizeof(inputLength));
		mOutputStream.write(value.c_str(), inputLength);

		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (const std::wstring& value)
	{
		std::uint32_t inputLength = value.size() * sizeof(wchar_t);

		mOutputStream.write(reinterpret_cast<char*>(&inputLength), sizeof(inputLength));
		mOutputStream.write(reinterpret_cast<const char*>(value.c_str()), sizeof(inputLength));

		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (int value)
	{
		mOutputStream.write(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (float value)
	{
		mOutputStream.write(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (TextureType value)
	{
		mOutputStream.write(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (std::uint32_t value)
	{
		mOutputStream.write(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (DirectX::XMFLOAT3& value)
	{
		mOutputStream.write(reinterpret_cast<char *>(&value), sizeof(value));
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (DirectX::XMFLOAT4& value)
	{
		mOutputStream.write(reinterpret_cast<char *>(&value), sizeof(value));
		return *this;
	}
	
	OutputStreamHelper& OutputStreamHelper::operator << (DirectX::XMFLOAT4X4& value)
	{
		mOutputStream.write(reinterpret_cast<char *>(&value), sizeof(value));
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (BoneVertexWeights& value)
	{
		std::uint32_t numVertexWeights = value.Weights().size();
		operator<<(numVertexWeights);
		for (auto vertexWeight : value.Weights())
		{
			mOutputStream.write(reinterpret_cast<char *>(&vertexWeight), sizeof(vertexWeight));
		}
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (Model& value)
	{
		std::uint32_t numMeshes = value.mMeshes.size();

		operator<<(numMeshes);

		for (auto mesh : value.mMeshes)
		{
			operator<<(*mesh);
		}

		std::uint32_t numMaterials = value.mMaterials.size();
		operator<<(numMaterials);

		for (auto material : value.mMaterials)
		{
			operator<<(*material);
		}

		std::uint32_t numBones = value.mBones.size();
		operator<<(numBones);
		for (auto bone : value.mBones)
		{
			operator<<(*bone);
		}
		
		std::uint32_t numAnimations = value.mAnimations.size();
		operator<<(numAnimations);
		for (auto animatiton : value.mAnimations)
		{
			operator<<(*animatiton);
		}

		if (value.mRootNode)
		{
			operator<<(true);
			SerializeSkeleton(value, value.mRootNode);
		}
		else
		{
			operator<<(false);
		}

		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (Mesh& value)
	{
		//Serialize the name
		operator<<(value.mName);

		//Serialize the Material Index
		operator<<(value.mMaterialIndex);

		//Serialize the FaceCount
		operator<<(value.mFaceCount);

		//Serialize the vertices
		std::uint32_t numVertices = value.mVertices.size();
		operator<<(numVertices);

		for (auto vertex : value.mVertices)
		{
			operator<<(vertex);
		}

		//Serialize the normals
		std::uint32_t numNormals = value.mNormals.size();
		operator<<(numNormals);

		for (auto normal : value.mNormals)
		{
			operator<<(normal);
		}

		//Serialize the tangents;
		std::uint32_t numTangents = value.mTangents.size();
		operator<<(numTangents);

		for (auto tangent : value.mTangents)
		{
			operator<<(tangent);
		}

		//Serialize the binormals
		std::uint32_t numBiNormals = value.mBiNormals.size();
		operator<<(numBiNormals);
		for (auto biNormal : value.mBiNormals)
		{
			operator<<(biNormal);
		}

		//Serialize the TextureCoordinates
		std::uint32_t numTextureCoordinates = value.mTextureCoordinates.size();
		operator<<(numTextureCoordinates);
		for (auto textureCoordinateVector : value.mTextureCoordinates)
		{
			std::uint32_t numCoordinates = textureCoordinateVector->size();
			operator<<(numCoordinates);
			for (auto coordinate : *textureCoordinateVector)
			{
				operator<<(coordinate);
			}
		}

		//Serialize the Vertex Colors
		std::uint32_t numVertexColors = value.mVertexColors.size();
		operator<<(numVertexColors);
		for (auto vertexColorVector : value.mVertexColors)
		{
			std::uint32_t numColors = vertexColorVector->size();
			operator<<(numColors);
			for (auto color : *vertexColorVector)
			{
				operator<<(color);
			}
		}

		//Serialize the Indices
		std::uint32_t numIndices = value.mIndices.size();
		operator<<(numIndices);
		for (auto index : value.mIndices)
		{
			operator<<(index);
		}

		//Serialize the BoneWeights
		std::uint32_t numBoneWeights = value.mBoneWeights.size();
		operator<<(numBoneWeights);
		for (auto boneWeight : value.mBoneWeights)
		{
			operator<<(boneWeight);
		}

		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (ModelMaterial& value)
	{
		//Serialize the name
		operator<<(value.mName);

		//Serialize the Textures
		std::uint32_t numTextures = value.mTextures.size();
		operator<<(numTextures);

		for (auto it = value.mTextures.begin(); it != value.mTextures.end(); ++it)
		{
			TextureType textureType = it->first;
			std::shared_ptr<std::vector<std::string>> fileNamesPtr = it->second;
			operator<<(textureType);

			std::uint32_t numFiles = fileNamesPtr->size();
			operator<<(numFiles);

			for (auto fileName : *fileNamesPtr)
			{
				operator<<(fileName);
			}
		}
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (AnimationClip& value)
	{
		operator<<(value.mName);
		operator<<(value.mKeyframeCount);
		operator<<(value.mDuration);
		operator<<(value.mTicksPerSecond);
		std::uint32_t numBoneAnimations = value.mBoneAnimations.size();
		operator<<(numBoneAnimations);
		for (auto boneAnimation : value.mBoneAnimations)
		{
			operator<<(*boneAnimation);
		}
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (BoneAnimation& value)
	{
		if (value.mBone)
		{
			operator<<(true);
			operator<<(value.mBone->mName);
		}
		else
		{
			operator<<(false);
		}
		std::uint32_t numKeyFrames = value.mKeyframes.size();
		operator <<(numKeyFrames);
		for (auto keyframe : value.mKeyframes)
		{
			operator<<(*keyframe);
		}
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (Keyframe& value)
	{
		operator<<(value.mTime);
		operator<<(value.mTranslation);
		operator<<(value.mRotationQuaternion);
		operator<<(value.mScale);
		return *this;
	}

	OutputStreamHelper& OutputStreamHelper::operator << (Bone& value)
	{
		operator<<(value.mName);
		operator<<(value.mIndex);
		operator<<(value.mTransform);
		operator<<(value.mOffsetTransform);
	
		return *this;
	}

	void OutputStreamHelper::SerializeSkeleton(Model& model, SceneNode* parentNode)
	{
		if (!parentNode)
			return;

		auto boneMapping = model.mBoneIndexMapping.find(parentNode->mName.c_str());
		if (boneMapping == model.mBoneIndexMapping.end())
		{
			operator<<(false);
			SceneNode* sceneNode = (SceneNode*)parentNode;
			operator<<(sceneNode->mName);
			operator<<(sceneNode->mTransform);
		}
		else
		{
			operator<<(true);
			Bone* bone = parentNode->As<Bone>();
			if (bone)
			{
				operator<<(bone->mName);
			}
		}

		UINT numChildren = (UINT)parentNode->mChildren.size();
		operator<<(numChildren);
		for (UINT i = 0; i < parentNode->mChildren.size(); i++)
		{
			SerializeSkeleton(model, (parentNode->mChildren[i]));
		}

	}

#pragma  endregion OutputStream

#pragma region InputStream

	InputStreamHelper::InputStreamHelper(std::string& fileName)
	{
		mInputStream.open(fileName, std::ios::binary | std::ios::in);

		if (!mInputStream)
		{
			throw std::runtime_error("InputStreamHelper : Unable to open file!!");
		}
	}

	InputStreamHelper::~InputStreamHelper()
	{
		mInputStream.close();
	}

	void InputStreamHelper::CloseStream()
	{
		mInputStream.close();
	}

	InputStreamHelper& InputStreamHelper::operator >> (std::string& value)
	{
		std::uint32_t nameLength;
		mInputStream.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

		std::unique_ptr<char[]> nameBuffer(new char[nameLength]);
		mInputStream.read(nameBuffer.get(), nameLength);
		value.assign(nameBuffer.get(), nameLength);

		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (std::wstring& value)
	{
		std::uint32_t stringLength;
		mInputStream.read(reinterpret_cast<char*>(&stringLength), sizeof(stringLength));

		std::unique_ptr<char[]> nameBuffer(new char[stringLength]);
		mInputStream.read(reinterpret_cast<char*>(nameBuffer.get()), stringLength);
		value.assign((wchar_t*)nameBuffer.get());
				
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (int &value)
	{
		mInputStream.read(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (float &value)
	{
		mInputStream.read(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (TextureType& value)
	{
		mInputStream.read(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (std::uint32_t &value)
	{
		mInputStream.read(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (DirectX::XMFLOAT3& value)
	{
		mInputStream.read(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (DirectX::XMFLOAT4& value)
	{
		mInputStream.read(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (DirectX::XMFLOAT4X4& value)
	{
		mInputStream.read(reinterpret_cast<char*>(&value), sizeof(value));
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (BoneVertexWeights& value)
	{
		std::uint32_t numVertexWeights = 0;
		operator>>(numVertexWeights);
		value.mWeights.reserve(numVertexWeights);
		for (std::uint32_t i = 0; i < numVertexWeights; ++i)
		{
			BoneVertexWeights::VertexWeight vertexWeight;
			mInputStream.read(reinterpret_cast<char *>(&vertexWeight), sizeof(vertexWeight));
			value.mWeights.push_back(vertexWeight);
		}
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (Model& value)
	{
		std::uint32_t numMeshes = 0;
		operator>>(numMeshes);
		value.mMeshes.reserve(numMeshes);

		for (std::uint32_t i = 0; i < numMeshes; ++i)
		{
			std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(value));
			value.mMeshes.push_back(mesh);
		}

		std::uint32_t numMaterials = 0;
		operator>>(numMaterials);
		value.mMaterials.reserve(numMaterials);

		for (std::uint32_t i = 0; i < numMaterials; ++i)
		{
			std::shared_ptr<ModelMaterial> material = std::shared_ptr<ModelMaterial>(new ModelMaterial(value));
			value.mMaterials.push_back(material);
		}
		
		std::uint32_t numBones = 0;
		operator>>(numBones);
		value.mBones.reserve(numBones);
		for (std::uint32_t i = 0; i < numBones; ++i)
		{
			std::shared_ptr<Bone> bone(new Bone());
			operator>>(*bone);
			value.mBones.push_back(bone);
			UINT boneIndex = 0U;
			auto boneMappingIterator = value.mBoneIndexMapping.find(bone->Name());
			if (boneMappingIterator != value.mBoneIndexMapping.end())
			{
				boneIndex = boneMappingIterator->second;
			}
			else
			{
				value.mBoneIndexMapping[bone->Name()] = bone->Index();
			}
		}

		std::uint32_t numAnimations = 0;
		operator>>(numAnimations);
		value.mAnimations.reserve(numAnimations);
		for (std::uint32_t i = 0; i < numAnimations; ++i)
		{
			std::shared_ptr<AnimationClip> animationClip(new AnimationClip(value));
			operator>>(*animationClip);
			value.mAnimations.push_back(animationClip);
			value.mAnimationsByName.insert(std::pair<std::string, AnimationClip*>(animationClip->Name(), animationClip.get()));

		}

		int isSceneNode = 0;
		operator>>(isSceneNode);
		if (isSceneNode)
		{
			value.mRootNode = DeSerializeSkeleton(value, nullptr);
		}
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (Mesh& value)
	{
		//De-Serialize the name
		operator>>(value.mName);

		//De-Serialize the Material Index;
		operator>>(value.mMaterialIndex);

		//De-Serialize the FaceCount
		operator>>(value.mFaceCount);

		//De-Serialize the vertices
		std::uint32_t numVertices = 0;
		operator>>(numVertices);

		value.mVertices.reserve(numVertices);
		for (std::uint32_t i = 0; i < numVertices; ++i)
		{
			XMFLOAT3 vertex;
			operator>>(vertex);
			value.mVertices.push_back(vertex);
		}

		//De-Serialize the normals
		std::uint32_t numNormals = 0;
		operator>>(numNormals);

		value.mNormals.reserve(numNormals);
		for (std::uint32_t i = 0; i < numNormals; ++i)
		{
			XMFLOAT3 normal;
			operator>>(normal);
			value.mNormals.push_back(normal);
		}

		//De-Serialize the tangents;
		std::uint32_t numTangents = 0;
		operator>>(numTangents);

		value.mTangents.reserve(numTangents);
		for (std::uint32_t i = 0; i < numTangents; ++i)
		{
			XMFLOAT3 tangent;
			operator>>(tangent);
			value.mTangents.push_back(tangent);
		}

		//De-Serialize the binormals;
		std::uint32_t numBiNormals = 0;
		operator>>(numBiNormals);
		value.mBiNormals.reserve(numBiNormals);
		for (std::uint32_t i = 0; i < numBiNormals; ++i)
		{
			XMFLOAT3 tangent;
			operator>>(tangent);
			value.mTangents.push_back(tangent);
		}

		//De-Serialize the TextureCoordinates
		std::uint32_t numTextureCoordinates = 0;
		operator>>(numTextureCoordinates);
		value.mTextureCoordinates.reserve(numTextureCoordinates);
		for (std::uint32_t i = 0; i < numTextureCoordinates; ++i)
		{
			std::uint32_t numCoordinates = 0;
			operator>>(numCoordinates);

			std::shared_ptr<std::vector<XMFLOAT3>> textureCoordinateVector = std::shared_ptr<std::vector<XMFLOAT3>>(new std::vector<XMFLOAT3>());
			textureCoordinateVector->reserve(numCoordinates);

			for (std::uint32_t i = 0; i < numCoordinates; ++i)
			{
				XMFLOAT3 coordinate;
				operator>>(coordinate);
				textureCoordinateVector->push_back(coordinate);
			}
			value.mTextureCoordinates.push_back(textureCoordinateVector);
		}

		//De-Serialize the Vertex Colors
		std::uint32_t numVertexColors = 0;
		operator>>(numVertexColors);
		value.mVertexColors.reserve(numVertexColors);
		for (std::uint32_t i = 0; i < numVertexColors; ++i)
		{
			std::uint32_t numColors = 0;
			operator>>(numColors);
			std::shared_ptr<std::vector<XMFLOAT4>> vertexColorVector = std::shared_ptr<std::vector<XMFLOAT4>>(new std::vector<XMFLOAT4>());
			vertexColorVector->reserve(numColors);

			for (std::uint32_t i = 0; i < numColors; ++i)
			{
				XMFLOAT4 color;
				operator>>(color);
				vertexColorVector->push_back(color);
			}
			value.mVertexColors.push_back(vertexColorVector);
		}

		//De-Serialize the indices
		std::uint32_t numIndices = 0;
		operator>>(numIndices);
		value.mIndices.reserve(numIndices);
		for (std::uint32_t i = 0; i < numIndices; ++i)
		{
			UINT index = 0;
			operator>>(index);
			value.mIndices.push_back(index);
		}

		//De-Serialize the BoneWeights
		std::uint32_t numBoneWeights = 0;
		operator>>(numBoneWeights);
		value.mBoneWeights.reserve(numBoneWeights);
		for (std::uint32_t i = 0; i < numBoneWeights; ++i)
		{
			BoneVertexWeights boneVertexWeight;
			operator>>(boneVertexWeight);
			value.mBoneWeights.push_back(boneVertexWeight);
		}
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (ModelMaterial& value)
	{
		//De-Serialize the name
		operator>> (value.mName);

		//De-Serialize the Textures
		std::uint32_t numTextures = 0;
		operator>>(numTextures);

		for (std::uint32_t i = 0; i < numTextures; ++i)
		{
			TextureType textureType = TextureType::TextureTypeDifffuse;
			operator>>(textureType);

			std::uint32_t numFiles = 0;
			operator>>(numFiles);

			std::shared_ptr<std::vector<std::string>> fileNameVectorPtr(new std::vector<std::string>());

			fileNameVectorPtr->reserve(numFiles);

			for (std::uint32_t i = 0; i < numFiles; ++i)
			{
				std::string fileName;

				operator>>(fileName);

				fileNameVectorPtr->push_back(fileName);
			}
			value.mTextures[textureType] = fileNameVectorPtr;
		}
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (AnimationClip& value)
	{
		operator>>(value.mName);
		operator>>(value.mKeyframeCount);
		operator>>(value.mDuration);
		operator>>(value.mTicksPerSecond);
		
		std::uint32_t numBoneAnimations = 0;
		operator>>(numBoneAnimations);
		value.mBoneAnimations.reserve(numBoneAnimations);
		for (std::uint32_t i = 0; i < numBoneAnimations; ++i)
		{
			std::shared_ptr<BoneAnimation> boneAnimation(new BoneAnimation(*value.mModel));
			operator>>(*boneAnimation);
			value.mBoneAnimations.push_back(boneAnimation);
			value.mBoneAnimationsByBone[&(boneAnimation->GetBone())] = boneAnimation.get();
		}
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (BoneAnimation& value)
	{
		int isBone = 0;
		operator>>(isBone);
		if (isBone)
		{
			std::string boneName;
			operator>>(boneName);
			auto boneMappingIterator = value.mModel->mBoneIndexMapping.find(boneName);
			if (boneMappingIterator != value.mModel->mBoneIndexMapping.end())
			{
				value.mBone = value.mModel->mBones[boneMappingIterator->second].get();
			}
			else
			{
				throw std::runtime_error("Fatal error Bone Name not found");
			}
		}
		
		std::uint32_t numKeyFrames = 0;
		operator >>(numKeyFrames);
		value.mKeyframes.reserve(numKeyFrames);
		for (std::uint32_t i = 0; i < numKeyFrames; ++i)
		{
			std::shared_ptr<Keyframe> keyframe(new Keyframe());
			operator>>(*keyframe);
			value.mKeyframes.push_back(keyframe);
		}
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (Keyframe& value)
	{
		operator>>(value.mTime);
		operator>>(value.mTranslation);
		operator>>(value.mRotationQuaternion);
		operator>>(value.mScale);
		return *this;
	}

	InputStreamHelper& InputStreamHelper::operator >> (Bone& value)
	{
		operator>>(value.mName);
		operator>>(value.mIndex);
		operator>>(value.mTransform);
		operator>>(value.mOffsetTransform);
		
		return *this;
	}

	SceneNode* InputStreamHelper::DeSerializeSkeleton(Model& model, SceneNode* parentNode)
	{
		SceneNode* sceneNode;
		int isBone = 0;
		operator>>(isBone);
		std::string nodeName;
		operator>>(nodeName);
		if (isBone)
		{	
			auto boneMapping = model.mBoneIndexMapping.find(nodeName.c_str());
			if (boneMapping == model.mBoneIndexMapping.end())
			{
				throw std::runtime_error("DeserializeSkeleton fatal error");
			}
			else
			{
				sceneNode = (SceneNode*)model.mBones[boneMapping->second].get();
			}
		}
		else
		{
			sceneNode = new SceneNode(nodeName);
			sceneNode->mName = nodeName;
			operator>>(sceneNode->mTransform);
		}
		sceneNode->SetParent(parentNode);
		UINT numChildren = 0;
		operator>>(numChildren);

		sceneNode->mChildren.reserve(numChildren);
		for (UINT i = 0; i < numChildren; ++i)
		{
			SceneNode* childSceneNode = DeSerializeSkeleton(model, sceneNode);
			sceneNode->mChildren.push_back(childSceneNode);
		}

		return sceneNode;
	}

#pragma endregion InputStream

}
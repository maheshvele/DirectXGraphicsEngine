#include "pch.h"
#include "Model.h"

namespace ModelPipeline
{
	Model::Model(const std::string& filename, bool flipUVs)
		: mMeshes(), mMaterials(), mAnimations(), mBones(), mBoneIndexMapping(), mRootNode(nullptr)
	{
		Assimp::Importer importer;

		UINT flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipWindingOrder;
		if (flipUVs)
		{
			flags |= aiProcess_FlipUVs;
		}

		const aiScene* scene = importer.ReadFile(filename, flags);
		if (scene == nullptr)
		{
			throw std::runtime_error(importer.GetErrorString());
		}

		if (scene->HasMaterials())
		{
			for (UINT i = 0; i < scene->mNumMaterials; i++)
			{
				mMaterials.push_back(std::shared_ptr<ModelMaterial>(new ModelMaterial(*this, scene->mMaterials[i])));
			}
		}

		if (scene->HasMeshes())
		{
			for (UINT i = 0; i < scene->mNumMeshes; i++)
			{
				std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(*this, *(scene->mMeshes[i])));
				mMeshes.push_back(mesh);
			}
		}

		if (scene->HasAnimations())
		{
			assert(scene->mRootNode != nullptr);
			mRootNode = BuildSkeleton(*scene->mRootNode, nullptr);

			mAnimations.reserve(scene->mNumAnimations);
			for (UINT i = 0; i < scene->mNumAnimations; i++)
			{
				std::shared_ptr<AnimationClip> animationClip (new AnimationClip(*this, *(scene->mAnimations[i])));
				mAnimations.push_back(animationClip);
				mAnimationsByName.insert(std::pair<std::string, AnimationClip*>(animationClip->Name(), animationClip.get()));
			}
		}
	}

	Model::~Model()
	{
		if (mRootNode && !mRootNode->As<Bone>())
		{
			DeleteSceneNode(mRootNode);
		}
	}

	bool Model::HasMeshes() const
	{
		return (mMeshes.size() > 0);
	}

	bool Model::HasMaterials() const
	{
		return (mMaterials.size() > 0);
	}

	bool Model::HasAnimations() const
	{
		return (mAnimations.size() > 0);
	}

	const std::vector<std::shared_ptr<Mesh>>& Model::Meshes() const
	{
		return mMeshes;
	}

	const std::vector<std::shared_ptr<ModelMaterial>>& Model::Materials() const
	{
		return mMaterials;
	}

	const std::vector<std::shared_ptr<AnimationClip>>& Model::Animations() const
	{
		return mAnimations;
	}

	const std::map<std::string, AnimationClip*>& Model::AnimationsbyName() const
	{
		return mAnimationsByName;
	}

	const std::vector<std::shared_ptr<Bone>>& Model::Bones() const
	{
		return mBones;
	}

	const std::map <std::string, UINT > & Model::BoneIndexMapping() const
	{
		return mBoneIndexMapping;
	}

	SceneNode* Model::RootNode()
	{
		return mRootNode;
	}

	SceneNode* Model::BuildSkeleton(aiNode& node, SceneNode* parentSceneNode)
	{
		SceneNode* sceneNode ;

		auto boneMapping = mBoneIndexMapping.find(node.mName.C_Str());
		if (boneMapping == mBoneIndexMapping.end())
		{
			sceneNode = new SceneNode(node.mName.C_Str());
		}
		else
		{
			sceneNode = mBones[boneMapping->second].get();
		}

		XMFLOAT4X4 nodeTransform(reinterpret_cast<const float*>(node.mTransformation[0]));
		XMMATRIX transform = XMLoadFloat4x4(&nodeTransform);
		sceneNode->SetTransform(XMMatrixTranspose(transform));
		sceneNode->SetParent(parentSceneNode);

		for (UINT i = 0; i < node.mNumChildren; i++)
		{
			SceneNode *childSceneNode = BuildSkeleton(*(node.mChildren[i]), sceneNode);
			sceneNode->Children().push_back(childSceneNode);
		}

		return sceneNode;
	}

	void Model::ValidateModel()
	{
		// Validate bone weights
		for (auto mesh : mMeshes)
		{
			for (BoneVertexWeights boneWeight : mesh->mBoneWeights)
			{
				float totalWeight = 0.0f;

				for (BoneVertexWeights::VertexWeight vertexWeight : boneWeight.Weights())
				{
					totalWeight += vertexWeight.Weight;
					assert(vertexWeight.BoneIndex >= 0);
					assert(vertexWeight.BoneIndex < mBones.size());
				}

				assert(totalWeight <= 1.05f);
				assert(totalWeight >= 0.95f);
			}
		}
	}

	void Model::DeleteSceneNode(SceneNode* sceneNode)
	{
		for (auto childNode : sceneNode->Children())
		{
			DeleteSceneNode(childNode);
		}

		if (sceneNode && !sceneNode->As<Bone>())
		{
			delete sceneNode;
			sceneNode = nullptr;
		}
	}

	Model::Model(std::string& fileName)
		:mMeshes(), mMaterials(), mAnimations(), mAnimationsByName(), mBones(), mBoneIndexMapping()
	{
		InputStreamHelper streamHelper(fileName);
		std::uint32_t numMeshes = 0;
		streamHelper >> numMeshes;
		mMeshes.reserve(numMeshes);

		for (std::uint32_t i = 0; i < numMeshes; ++i)
		{
			std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(*this));
			streamHelper >> *mesh;
			mMeshes.push_back(mesh);
		}

		std::uint32_t numMaterials = 0;
		streamHelper >> numMaterials;
		mMaterials.reserve(numMaterials);

		for (std::uint32_t i = 0; i < numMaterials; ++i)
		{
			std::shared_ptr<ModelMaterial> material = std::shared_ptr<ModelMaterial>(new ModelMaterial(*this));
			streamHelper >> *material;
			mMaterials.push_back(material);
		}
	
		std::uint32_t numBones = 0;
		streamHelper >> numBones;
		mBones.reserve(numBones);

		for (std::uint32_t i = 0; i < numBones; ++i)
		{
			std::shared_ptr<Bone> bone(new Bone());
			streamHelper >> *bone;
			mBones.push_back(bone);
			UINT boneIndex = 0U;
			auto boneMappingIterator = mBoneIndexMapping.find(bone->Name());
			if (boneMappingIterator != mBoneIndexMapping.end())
			{
				boneIndex = boneMappingIterator->second;
			}
			else
			{
				mBoneIndexMapping[bone->Name()] = bone->Index();
			}
		}

		std::uint32_t numAnimations = 0;
		streamHelper >> numAnimations;
		mAnimations.reserve(numAnimations);

		for (std::uint32_t i = 0; i < numAnimations; ++i)
		{
			std::shared_ptr<AnimationClip> animationClip(new AnimationClip(*this));
			streamHelper >> *animationClip;
			mAnimations.push_back(animationClip);
			mAnimationsByName.insert(std::pair<std::string, AnimationClip*>(animationClip->Name(), animationClip.get()));
		}

		int isSceneNode = 0;
		streamHelper >> isSceneNode;
		if (isSceneNode)
		{
			mRootNode = streamHelper.DeSerializeSkeleton(*this, nullptr);
		}
	}
}
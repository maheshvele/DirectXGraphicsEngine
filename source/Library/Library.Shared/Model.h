#pragma once

namespace Library
{
	
	class Mesh;
	class ModelMaterial;

	class Model
	{
		friend class Mesh;
		friend class OutputStreamHelper;
		friend class InputStreamHelper;

	public:
		Model(const Game& game, const std::string& filename, bool flipUVs = false);
		~Model();

		bool HasMeshes() const;
		bool HasMaterials() const;
		bool HasAnimations() const;

		const std::vector<std::shared_ptr<Mesh>>& Meshes() const;
		const std::vector<std::shared_ptr<ModelMaterial>>& Materials() const;
		const std::vector<std::shared_ptr<AnimationClip>>& Animations() const;
		const std::map<std::string, AnimationClip*>& AnimationsbyName() const;
		const std::vector<std::shared_ptr<Bone>>& Bones() const;
		const std::map<std::string, UINT>& BoneIndexMapping() const;
		SceneNode* RootNode();

		const Game& GetGame() const;
	private:
		Model(const Model& rhs);
		Model& operator=(const Model& rhs);

		//SceneNode* BuildSkeleton(aiNode& node, SceneNode* parentSceneNode);
		void ValidateModel();
		void DeleteSceneNode(SceneNode* sceneNode);

		const Game& mGame;
		std::vector<std::shared_ptr<Mesh>> mMeshes;
		std::vector<std::shared_ptr<ModelMaterial>> mMaterials;
		std::vector<std::shared_ptr<AnimationClip>> mAnimations;
		std::map<std::string, AnimationClip*> mAnimationsByName;
		std::vector<std::shared_ptr<Bone>> mBones;
		std::map<std::string, UINT> mBoneIndexMapping;
		SceneNode*	mRootNode;

	};
}

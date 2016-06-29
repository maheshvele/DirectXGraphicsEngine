#pragma once

struct aiNodeAnim;
using namespace DirectX;

namespace Library
{
	class Model;
	class Bone;
	class Keyframe;

    class BoneAnimation
    {
		friend class AnimationClip;
		friend class OutputStreamHelper;
		friend class InputStreamHelper;

    public:        
		static const XMFLOAT4 Zero;

        ~BoneAnimation();
		
		Bone& GetBone();
		const std::vector<std::shared_ptr<Keyframe>> Keyframes() const;

		UINT GetTransform(float time, XMFLOAT4X4& transform) const;
		void GetTransformAtKeyframe(UINT keyframeIndex, XMFLOAT4X4& transform) const;
		void GetInteropolatedTransform(float time, XMFLOAT4X4& transform) const;		

    private:
		BoneAnimation(Model& model);

		BoneAnimation(const BoneAnimation& rhs);
        BoneAnimation& operator=(const BoneAnimation& rhs);

		UINT FindKeyframeIndex(float time) const;

		Model* mModel;
		Bone* mBone;
		std::vector<std::shared_ptr<Keyframe>> mKeyframes;
    };
}

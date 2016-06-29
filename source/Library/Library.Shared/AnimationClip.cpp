#include "pch.h"
#include "AnimationClip.h"

namespace Library
{
	const XMFLOAT4X4 AnimationClip::Identity = XMFLOAT4X4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	AnimationClip::AnimationClip(Model& model)
		: mName(), mDuration(0), mTicksPerSecond(0),
		mBoneAnimations(), mBoneAnimationsByBone(), mKeyframeCount(0), mModel(&model)
	{
	}

    AnimationClip::~AnimationClip()
    {
		for (auto boneAnimation : mBoneAnimations)
		{
			boneAnimation.reset();
		}
    }

	const std::string& AnimationClip::Name() const
	{
		return mName;
	}

	float AnimationClip::Duration() const
	{
		return mDuration;
	}

	float AnimationClip::TicksPerSecond() const
	{
		return mTicksPerSecond;
	}

	const std::vector<std::shared_ptr<BoneAnimation>>& AnimationClip::BoneAnimations() const
	{
		return mBoneAnimations;
	}

	const std::map<Bone*, BoneAnimation*>& AnimationClip::BoneAnimationsByBone() const
	{
		return mBoneAnimationsByBone;
	}

	const UINT AnimationClip::KeyframeCount() const
	{
		return mKeyframeCount;
	}

	UINT AnimationClip::GetTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mBoneAnimationsByBone.end())
		{
			return foundBoneAnimation->second->GetTransform(time, transform);
		}
		else
		{
			transform = AnimationClip::Identity;
			return UINT_MAX;
		}
	}

	void AnimationClip::GetTransforms(float time, std::vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (auto boneAnimation : mBoneAnimations)
		{
			boneAnimation->GetTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::GetTransformAtKeyframe(UINT keyframe, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mBoneAnimationsByBone.end())
		{
			foundBoneAnimation->second->GetTransformAtKeyframe(keyframe, transform);
		}
		else
		{
			transform = AnimationClip::Identity;
		}
	}

	void AnimationClip::GetTransformsAtKeyframe(UINT keyframe, std::vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (auto boneAnimation : mBoneAnimations)
		{
			boneAnimation->GetTransformAtKeyframe(keyframe, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::GetInteropolatedTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mBoneAnimationsByBone.end())
		{
			foundBoneAnimation->second->GetInteropolatedTransform(time, transform);
		}
		else
		{
			transform = AnimationClip::Identity;
		}
	}

	void AnimationClip::GetInteropolatedTransforms(float time, std::vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (auto boneAnimation : mBoneAnimations)
		{
			boneAnimation->GetInteropolatedTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}
}

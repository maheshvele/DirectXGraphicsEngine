#include "pch.h"
#include "BoneAnimation.h"

namespace Library
{
	const XMFLOAT4 BoneAnimation::Zero = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	BoneAnimation::BoneAnimation(Model& model)
		: mModel(&model), mBone(nullptr), mKeyframes()
	{
	}

    BoneAnimation::~BoneAnimation()
    {
		for (std::shared_ptr<Keyframe> keyframe : mKeyframes)
		{
			keyframe.reset();
		}
    }

	Bone& BoneAnimation::GetBone()
	{
		return *mBone;
	}
	
	const std::vector<std::shared_ptr<Keyframe>> BoneAnimation::Keyframes() const
	{
		return mKeyframes;
	}

	UINT BoneAnimation::GetTransform(float time, XMFLOAT4X4& transform) const
	{
		UINT keyframeIndex = FindKeyframeIndex(time);
		std::shared_ptr<Keyframe> keyframe = mKeyframes[keyframeIndex];

		XMStoreFloat4x4(&transform, keyframe->Transform());

		return keyframeIndex;
	}

	void BoneAnimation::GetTransformAtKeyframe(UINT keyframeIndex, XMFLOAT4X4& transform) const
	{
		// Clamp the keyframe
		if (keyframeIndex >= mKeyframes.size() )
		{
			keyframeIndex = (std::uint32_t)(mKeyframes.size() - 1);
		}
		
		std::shared_ptr<Keyframe> keyframe = mKeyframes[keyframeIndex];

		XMStoreFloat4x4(&transform, keyframe->Transform());
	}

	void BoneAnimation::GetInteropolatedTransform(float time, XMFLOAT4X4& transform) const
	{
		std::shared_ptr<Keyframe> firstKeyframe = mKeyframes.front();
		std::shared_ptr<Keyframe> lastKeyframe = mKeyframes.back();		

		if (time <= firstKeyframe->Time())
		{
			// Specified time is before the start time of the animation, so return the first keyframe
			XMStoreFloat4x4(&transform, firstKeyframe->Transform());
		}
		else if (time >= lastKeyframe->Time())
		{
			// Specified time is after the end time of the animation, so return the last keyframe
			XMStoreFloat4x4(&transform, lastKeyframe->Transform());
		}
		else
		{
			// Interpolate the transform between keyframes
			UINT keyframeIndex = FindKeyframeIndex(time);
			std::shared_ptr<Keyframe> keyframeOne = mKeyframes[keyframeIndex];
			std::shared_ptr<Keyframe> keyframeTwo = mKeyframes[keyframeIndex + 1];

			XMVECTOR translationOne = keyframeOne->TranslationVector();
			XMVECTOR rotationQuaternionOne = keyframeOne->RotationQuaternionVector();
			XMVECTOR scaleOne = keyframeOne->ScaleVector();

			XMVECTOR translationTwo = keyframeTwo->TranslationVector();
			XMVECTOR rotationQuaternionTwo = keyframeTwo->RotationQuaternionVector();
			XMVECTOR scaleTwo = keyframeTwo->ScaleVector();

			float lerpValue = ((time - keyframeOne->Time()) / (keyframeTwo->Time() - keyframeOne->Time()));
			XMVECTOR translation = XMVectorLerp(translationOne, translationTwo, lerpValue);
			XMVECTOR rotationQuaternion = XMQuaternionSlerp(rotationQuaternionOne, rotationQuaternionTwo, lerpValue);
			XMVECTOR scale = XMVectorLerp(scaleOne, scaleTwo, lerpValue);

			static XMVECTOR rotationOrigin = XMLoadFloat4(&BoneAnimation::Zero);
			XMStoreFloat4x4(&transform, XMMatrixAffineTransformation(scale, rotationOrigin, rotationQuaternion, translation));
		}
	}

	UINT BoneAnimation::FindKeyframeIndex(float time) const
	{
		std::shared_ptr<Keyframe> firstKeyframe = mKeyframes.front();
		if (time <= firstKeyframe->Time())
		{
			return 0;
		}

		std::shared_ptr<Keyframe> lastKeyframe = mKeyframes.back();
		if (time >= lastKeyframe->Time())
		{
			return (UINT)(mKeyframes.size() - 1);
		}

		UINT keyframeIndex = 1;

		for (; keyframeIndex < mKeyframes.size() - 1 && time >= mKeyframes[keyframeIndex]->Time(); keyframeIndex++);

		return (UINT)(keyframeIndex - 1);
	}
}

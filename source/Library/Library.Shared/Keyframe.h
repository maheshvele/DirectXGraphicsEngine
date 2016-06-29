#pragma once

using namespace DirectX;

namespace Library
{
    class Keyframe
    {
		friend class BoneAnimation;
		friend class OutputStreamHelper;
		friend class InputStreamHelper;

    public:
		static const XMFLOAT4 Zero;
		float Time() const;
		const XMFLOAT3& Translation() const;
		const XMFLOAT4& RotationQuaternion() const;
		const XMFLOAT3& Scale() const;

		XMVECTOR TranslationVector() const;
		XMVECTOR RotationQuaternionVector() const;
		XMVECTOR ScaleVector() const;

		XMMATRIX Transform() const;

    private:
		Keyframe(float time, const XMFLOAT3& translation, const XMFLOAT4& rotationQuaternion, const XMFLOAT3& scale);

		Keyframe();
        Keyframe(const Keyframe& rhs);
        Keyframe& operator=(const Keyframe& rhs);

		float mTime;
		XMFLOAT3 mTranslation;
		XMFLOAT4 mRotationQuaternion;
		XMFLOAT3 mScale;
    };
}

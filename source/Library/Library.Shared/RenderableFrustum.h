#pragma once

#include "RenderingGameComponent.h"
#include "Frustum.h"

namespace Library
{
	class BasicMaterial;
	class Pass;

	class RenderableFrustum : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(RenderableFrustum, RenderingGameComponent)

	public:
		RenderableFrustum(const Game& game, const std::shared_ptr<Camera>& camera);
		RenderableFrustum(const Game& game, const std::shared_ptr<Camera>& camera, const XMFLOAT4& color);
		~RenderableFrustum();

		RenderableFrustum() = delete;
		RenderableFrustum(const RenderableFrustum& rhs) = delete;
		RenderableFrustum& operator=(const RenderableFrustum& rhs) = delete;

		const XMFLOAT3& Position() const;
		const XMFLOAT3& Direction() const;
		const XMFLOAT3& Up() const;
		const XMFLOAT3& Right() const;

		XMVECTOR PositionVector() const;
		XMVECTOR DirectionVector() const;
		XMVECTOR UpVector() const;
		XMVECTOR RightVector() const;

		void SetPosition(FLOAT x, FLOAT y, FLOAT z);
		void SetPosition(FXMVECTOR position);
		void SetPosition(const XMFLOAT3& position);

		void ApplyRotation(CXMMATRIX transform);
		void ApplyRotation(const XMFLOAT4X4& transform);

		void InitializeGeometry(const Frustum& frustum);

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Render(const GameTime& gameTime) override;

	private:
		struct VertexCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;

			VertexCBufferPerObject() : WorldViewProjection() { }

			VertexCBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		void InitializeVertexBuffer(const Frustum& frustum);
		void InitializeIndexBuffer();

		static const XMVECTORF32 DefaultColor;
		static const UINT FrustumVertexCount;
		static const UINT FrustumPrimitiveCount;
		static const UINT FrustumIndicesPerPrimitive;
		static const UINT FrustumIndexCount;
		static const USHORT FrustumIndices[];

		XMFLOAT4 mColor;
		XMFLOAT3 mPosition;
		XMFLOAT3 mDirection;
		XMFLOAT3 mUp;
		XMFLOAT3 mRight;
		XMFLOAT4X4 mWorldMatrix;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
	};
}
#pragma once
#include "RenderingGameComponent.h"


namespace Library
{
	class Skybox : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(Skybox, RenderingGameComponent);
	public:
		Skybox(const Game& game, const std::shared_ptr<Camera>& camera, const std::wstring& cubeMapFileName, float scale);
		~Skybox(){};

		Skybox() = delete;
		Skybox(const Skybox& rhs) = delete;
		Skybox& operator=(const Skybox& rhs) = delete;

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

		void CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const;

		std::wstring mCubeMapFileName;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSkyboxTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mTrilinearSampler;
		UINT mIndexCount;
		XMFLOAT4X4 mWorldMatrix;
		XMFLOAT4X4 mScaleMatrix;

	};

}
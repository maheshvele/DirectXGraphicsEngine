#pragma once
#include "RenderingGameComponent.h"
using namespace Library;

namespace Library
{
	class Mesh;
}

namespace RenderGame
{
	class TexturedSphereDemo : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(TexturedSphereDemo, RenderingGameComponent);

	public:
		TexturedSphereDemo(const Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		~TexturedSphereDemo();

		virtual void Initialize() override;
		virtual void Render(const GameTime& gameTime) override;

	private:
		

		TexturedSphereDemo();
		TexturedSphereDemo(const TexturedSphereDemo& rhs);
		TexturedSphereDemo& operator=(const TexturedSphereDemo& rhs);

		void CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const;

		CBufferPerObject mCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;

		UINT mIndexCount;

		XMFLOAT4X4 mWorldMatrix;
	};
}

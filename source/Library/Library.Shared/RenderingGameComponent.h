#pragma once
#include "Camera.h"
#include "GameComponent.h"

namespace Library
{
	class RenderingGameComponent : public GameComponent
	{
		RTTI_DECLARATIONS(RenderingGameComponent, GameComponent);

	public:
		RenderingGameComponent(const Game& game);
		RenderingGameComponent(const Game& game, const std::shared_ptr<Camera>& camera);
		virtual ~RenderingGameComponent();
		virtual void Render(const GameTime& gameTime);
		virtual void ReleaseResources() override;

		void SetCamera(const std::shared_ptr<Camera>& camera);
		const std::shared_ptr<Camera>& GetCamera() const;

		void SetPostProcessingDemo(bool enabled);
		bool IsPostProcessingDemo() const; 

	protected:

		std::shared_ptr<Camera> mCamera;
		//Shaders
		Microsoft::WRL::ComPtr<ID3D11VertexShader>		mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>		mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>			mConstantBuffer;

		bool bIsPostProcessingDemo;

	private:
		RenderingGameComponent(const RenderingGameComponent& rhs);
		RenderingGameComponent& operator=(const RenderingGameComponent& rhs);
	};
}


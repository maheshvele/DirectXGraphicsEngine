#include "pch.h"
#include "RenderingGameComponent.h"

namespace Library
{
	RTTI_DEFINITIONS(RenderingGameComponent);

	RenderingGameComponent::RenderingGameComponent(const Game& game)
		: GameComponent(game), bIsPostProcessingDemo(false)
	{
	}

	RenderingGameComponent::RenderingGameComponent(const Game& game, const std::shared_ptr<Camera>& camera)
		: GameComponent(game), mCamera(camera), bIsPostProcessingDemo(false)
	{
	}

	RenderingGameComponent::~RenderingGameComponent()
	{
	}

	void RenderingGameComponent::Render(const GameTime& gameTime)
	{

	}

	void RenderingGameComponent::ReleaseResources()
	{
		mVertexShader.Reset();
		mInputLayout.Reset();
		mPixelShader.Reset();
		mConstantBuffer.Reset();
		mVertexBuffer.Reset();
		mIndexBuffer.Reset();
	}

	void RenderingGameComponent::SetCamera(const std::shared_ptr<Camera>& camera)
	{
		mCamera = camera;
	}

	const std::shared_ptr<Camera>& RenderingGameComponent::GetCamera() const
	{
		return mCamera;
	}

	void RenderingGameComponent::SetPostProcessingDemo(bool enabled)
	{
		bIsPostProcessingDemo = enabled;
	}

	bool RenderingGameComponent::IsPostProcessingDemo() const
	{
		return bIsPostProcessingDemo;
	}
}
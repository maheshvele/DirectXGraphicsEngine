#include "pch.h"
#include "ColorFilteringDemo.h"

namespace RenderGame
{
	const float ColorFilteringDemo::BrightnessModulationRate = 1.0f;
	const XMVECTORF32 ColorFilteringDemo::BackgroundColor = ColorHelper::CornflowerBlue;

	RTTI_DEFINITIONS(ColorFilteringDemo);

	ColorFilteringDemo::ColorFilteringDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera)
		:RenderingGameComponent(game, camera), mRenderStateHelper(game), mActiveColorFilter(ColorFilter::ColorFilterInverse), mGenericColorFilter(MatrixHelper::Identity)
	{
		bIsPostProcessingDemo = true;
	}

	ColorFilteringDemo::~ColorFilteringDemo()
	{
	}

	void ColorFilteringDemo::Initialize()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif

		mRenderTarget = std::shared_ptr<FullScreenRenderTarget>(new FullScreenRenderTarget(const_cast<Library::Game&>(mGame)));
		
		mBlinnPhongDemo = std::shared_ptr<BlinnPhongDemo>(new BlinnPhongDemo(mGame, mCamera));
		mBlinnPhongDemo->Initialize();

		mFullScreenQuad = std::shared_ptr<FullScreenQuad>(new FullScreenQuad(mGame));
		mFullScreenQuad->Initialize();
		
		// Create text rendering helpers
		mSpriteBatch = std::make_unique<SpriteBatch>(mGame.GetDirectXContext().Get());
		mSpriteFont = std::make_unique<SpriteFont>(mGame.GetDirectXDevice().Get(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the GamePad service
		mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
		assert(mGamePad != nullptr);

		// Load a compiled Gray Scale pixel shader
		std::string compiledGrayScalePixelShader = Utility::LoadBinaryFile("Content\\Shaders\\GrayScalePS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledGrayScalePixelShader[0], compiledGrayScalePixelShader.size(), nullptr, mGrayScalePixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");
		
		// Load a compiled Inverse Color pixel shader
		std::string compiledInverseColorPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\InverseColorPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledInverseColorPixelShader[0], compiledInverseColorPixelShader.size(), nullptr, mInverseColorPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		// Load a compiled Sepia pixel shader
		std::string compiledSepiaPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\SepiaPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledSepiaPixelShader[0], compiledSepiaPixelShader.size(), nullptr, mSepiaPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		// Load a compiled Generic Filter pixel shader
		std::string compiledGenericFilterPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\GenericFilterPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledGenericFilterPixelShader[0], compiledGenericFilterPixelShader.size(), nullptr, mGenericFilterPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = sizeof(mGenericFilterBufferPerObjectData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mGenericFilterBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");


		mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&ColorFilteringDemo::UpdateColorFilterShader, this));
	}

	void ColorFilteringDemo::UpdateColorFilterShader()
	{
		ID3D11DeviceContext2* direct3DDeviceContext = mGame.GetDirectXContext().Get();

		switch (mActiveColorFilter)
		{
		case ColorFilter::ColorFilterGrayScale:
			direct3DDeviceContext->PSSetShader(mGrayScalePixelShader.Get(), nullptr, 0);
			break;

		case ColorFilter::ColorFilterInverse:
			direct3DDeviceContext->PSSetShader(mInverseColorPixelShader.Get(), nullptr, 0);
			break;

		case ColorFilter::ColorFilterSepia:
			direct3DDeviceContext->PSSetShader(mSepiaPixelShader.Get(), nullptr, 0);
			break;

		case ColorFilter::ColorFilterGeneric:
			XMMATRIX colorFilter = XMLoadFloat4x4(&mGenericColorFilter);
			direct3DDeviceContext->UpdateSubresource(mGenericFilterBufferPerObject.Get(), 0, nullptr, &colorFilter, 0, 0);
			ID3D11Buffer* PSConstantBuffers[] = { mGenericFilterBufferPerObject.Get() };
			direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

			direct3DDeviceContext->PSSetShader(mGenericFilterPixelShader.Get(), nullptr, 0);
			break;
		}

		
		direct3DDeviceContext->PSSetSamplers(0, 1, SamplerStates::TrilinearWrap.GetAddressOf());
		ID3D11ShaderResourceView* shaderResourceView = mRenderTarget->OutputTexture().Get();
		direct3DDeviceContext->PSSetShaderResources(0, 1, &shaderResourceView);

	}

	void ColorFilteringDemo::Update(const GameTime& gameTime)
	{
		mBlinnPhongDemo->Update(gameTime);

		if (mGamePad->WasButtonUp(GamePadButton::RightStick) && mGamePad->IsButtonDown(GamePadButton::RightStick))
		{
			mActiveColorFilter = ColorFilter(mActiveColorFilter + 1);
			if (mActiveColorFilter >= ColorFilterEnd)
			{
				mActiveColorFilter = (ColorFilter)(0);
			}

			//Change the active shader here
		}

		UpdateGenericColorFilter(gameTime);
	}

	void ColorFilteringDemo::Render(const GameTime& gameTime)
	{

		Library::Game& game = const_cast<Library::Game&>(mGame);

		game.Clear();
		mRenderTarget->Begin();

		mGame.GetDirectXContext()->ClearRenderTargetView(mRenderTarget->RenderTargetView().Get(), reinterpret_cast<const float*>(&BackgroundColor));
		mGame.GetDirectXContext()->ClearDepthStencilView(mRenderTarget->DepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
		
		for (auto component : mGame.GetGameComponents() )
		{
			RenderingGameComponent* renderingGameComponent = component->As<RenderingGameComponent>();
			
			if (component->IsEnabled() && (renderingGameComponent!=this))
				component->Render(gameTime);
		}

		mBlinnPhongDemo->Render(gameTime);

		
		mRenderTarget->End();

		mGame.GetDirectXContext()->ClearRenderTargetView(mGame.GetRenderTargetView().Get(), reinterpret_cast<const float*>(&BackgroundColor));
		mGame.GetDirectXContext()->ClearDepthStencilView(mGame.GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mFullScreenQuad->Render(gameTime);

	
		game.Present();
	}

	void ColorFilteringDemo::UpdateGenericColorFilter(const GameTime& gameTime)
	{
		static float brightness = 1.0f;

		if (mGamePad!= nullptr)
		{
			if (mGamePad->IsButtonDown(GamePadButton::Start) && brightness < 1.0f)
			{
				brightness += BrightnessModulationRate * (float)gameTime.GetElapsedSeconds();
				brightness = XMMin<float>(brightness, 1.0f);
				XMStoreFloat4x4(&mGenericColorFilter, XMMatrixScaling(brightness, brightness, brightness));
			}

			if (mGamePad->IsButtonDown(GamePadButton::Back) && brightness > 0.0f)
			{
				brightness -= BrightnessModulationRate * (float)gameTime.GetElapsedSeconds();
				brightness = XMMax<float>(brightness, 0.0f);
				XMStoreFloat4x4(&mGenericColorFilter, XMMatrixScaling(brightness, brightness, brightness));
			}
		}
	}
}
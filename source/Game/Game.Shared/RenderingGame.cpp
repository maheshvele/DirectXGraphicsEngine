#include "pch.h"
#include "RenderingGame.h"

using namespace Library;
using namespace DirectX;

#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;
#endif

namespace RenderGame
{
	RTTI_DEFINITIONS(RenderingGame);

	RenderingGame::RenderingGame()
		:Game()
	{
	}

	RenderingGame::RenderingGame(void *window)
		: Game(window)
	{
	}

	RenderingGame::~RenderingGame()
	{
	}

	void RenderingGame::SetWindow(void* window)
	{
		mWindow = window;
	}

	void RenderingGame::Initialize()
	{
		std::shared_ptr<DemoSystem> demoSystem = std::shared_ptr<DemoSystem>(new DemoSystem(*this));
		demoSystem->SetEnabled(true);
		AddGameComponent(demoSystem);
		mServices.AddService(DemoSystem::TypeIdClass(), demoSystem.get());

		std::shared_ptr<GamePadComponent> gamePad = std::shared_ptr<Library::GamePadComponent>(new GamePadComponent(*this, 0));
		gamePad->SetEnabled(true);
		AddGameComponent(gamePad);
		mServices.AddService(GamePadComponent::TypeIdClass(), gamePad.get());

		std::shared_ptr<FirstPersonCamera> firstPersonCamera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera(*this));
		firstPersonCamera->SetEnabled(true);
		AddGameComponent(firstPersonCamera);
		mServices.AddService(Camera::TypeIdClass(), firstPersonCamera.get());

		std::shared_ptr<Skybox> skybox = std::shared_ptr<Library::Skybox>(new Library::Skybox(*this, firstPersonCamera, L"Content\\Textures\\Maskonaive2_1024.dds", 500.0f));
		skybox->SetEnabled(true);
		AddGameComponent(skybox);
		mServices.AddService(Skybox::TypeIdClass(), skybox.get());
				
		//Adding Demo Components
		demoSystem->AddDemo(std::shared_ptr<ColoredTriangleDemo>(new ColoredTriangleDemo(*this)));
		demoSystem->AddDemo(std::shared_ptr<RotatingCubeDemo>(new RotatingCubeDemo(*this)));
		demoSystem->AddDemo(std::shared_ptr<TexturedSphereDemo>(new TexturedSphereDemo(*this, firstPersonCamera)));
		demoSystem->AddDemo(std::shared_ptr<PointLightDemo>(new PointLightDemo(*this, firstPersonCamera)));
		demoSystem->AddDemo(std::shared_ptr<BlinnPhongDemo>(new BlinnPhongDemo(*this, firstPersonCamera)));
		demoSystem->AddDemo(std::shared_ptr<EnvironmentMappingDemo>(new EnvironmentMappingDemo(*this, firstPersonCamera)));
		demoSystem->AddDemo(std::shared_ptr<TransparencyMappingDemo>(new TransparencyMappingDemo(*this, firstPersonCamera)));
		demoSystem->AddDemo(std::shared_ptr<FogDemo>(new FogDemo(*this, firstPersonCamera)));
		//demoSystem->AddDemo(std::shared_ptr<ColorFilteringDemo>(new ColorFilteringDemo(*this, firstPersonCamera)));
		demoSystem->AddDemo(std::shared_ptr<ShadowMappingDemo>(new ShadowMappingDemo(*this, firstPersonCamera))); 
		demoSystem->AddDemo(std::shared_ptr<AnimationDemo>(new AnimationDemo(*this, firstPersonCamera)));

		mRenderStateHelper = make_unique<RenderStateHelper>(*this);
		
		Game::Initialize();
		RasterizerStates::Initialize(GetDirectXDevice());
		SamplerStates::BorderColor = ColorHelper::Black;
		SamplerStates::Initialize(GetDirectXDevice().Get());
		BlendStates::Initialize(GetDirectXDevice().Get());

		firstPersonCamera->SetPosition(0.0f, 5.0f, 20.0f);
		firstPersonCamera->ApplyRotation(XMMatrixRotationX(-XMConvertToRadians(10)));
	}

	void RenderingGame::Update()
	{
		Skybox* skybox = (Skybox*)mServices.GetService(Skybox::TypeIdClass());
		GamePadComponent* gamePadComponent = (GamePadComponent *)mServices.GetService(GamePadComponent::TypeIdClass());

		if (skybox && gamePadComponent)
		{
			if (gamePadComponent->WasButtonUp(GamePadButton::LeftStick) && gamePadComponent->IsButtonDown(GamePadButton::LeftStick))
			{
				skybox->SetEnabled(!skybox->IsEnabled());
			}
		}

		Game::Update();
	}

	void RenderingGame::Tick()
	{
		Game::Tick();
	}

	void RenderingGame::Render()
	{
		DemoSystem* demoSystem= (DemoSystem *)mServices.GetService(DemoSystem::TypeIdClass());
		
		RenderingGameComponent* activeDemo = demoSystem->GetActiveDemo().get();
		mRenderStateHelper->SaveAll();

		if (activeDemo && !activeDemo->IsPostProcessingDemo())
		{
			Game::Render();
		}
		else
		{
			activeDemo->Render(mGameTime);
		}
		mRenderStateHelper->RestoreAll();
	}

	void RenderingGame::Clear()
	{
		mD3dContext2->ClearRenderTargetView(mD3dRenderTargetView.Get(), Colors::CornflowerBlue);
		mD3dContext2->ClearDepthStencilView(mD3dDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		mD3dContext2->OMSetRenderTargets(1, mD3dRenderTargetView.GetAddressOf(), mD3dDepthStencilView.Get());
	}

	void RenderingGame::Present()
	{
		// The first argument instructs DXGI to block until VSync, putting the application
		// to sleep until the next VSync. This ensures we don't waste any cycles rendering
		// frames that will never be displayed to the screen.
		HRESULT hr = mSwapChain1->Present(1, 0);

		// Discard the contents of the render target.
		// This is a valid operation only when the existing contents will be entirely
		// overwritten. If dirty or scroll rects are used, this call should be removed.
		mD3dContext2->DiscardView(mD3dRenderTargetView.Get());

		// Discard the contents of the depth stencil.
		mD3dContext2->DiscardView(mD3dDepthStencilView.Get());

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
		//	OnDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr);
		}
	}

	void RenderingGame::GetWindowDimensions(float& screenWidth, float& screenHeight) const
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		RECT rc;
		GetWindowRect(reinterpret_cast<HWND>(mWindow), &rc);

		screenWidth = (float)std::max<UINT>(rc.right - rc.left, 1);
		screenHeight = (float)std::max<UINT>(rc.bottom - rc.top, 1);
#else

		screenWidth = (float)lround(mD3dRenderTargetSize.Width);
		screenHeight = (float)lround(mD3dRenderTargetSize.Height);
#endif
	}

#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	void RenderingGame::SetWindow(Windows::UI::Core::CoreWindow^ window)
	{
		DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

		mLogicalSize = Windows::Foundation::Size(window->Bounds.Width, window->Bounds.Height);
		mNativeOrientation = currentDisplayInformation->NativeOrientation;
		mCurrentOrientation = currentDisplayInformation->CurrentOrientation;
		mDpi = currentDisplayInformation->LogicalDpi;

		mWindow = (void*)window;

		// Calculate the necessary render target size in pixels.
		mOutputSize.Width = Utility::ConvertDipsToPixels(mLogicalSize.Width, mDpi);
		mOutputSize.Height = Utility::ConvertDipsToPixels(mLogicalSize.Height, mDpi);

		// Prevent zero size DirectX content from being created.
		mOutputSize.Width = max(mOutputSize.Width, 1);
		mOutputSize.Height = max(mOutputSize.Height, 1);

		// The width and height of the swap chain must be based on the window's
		// natively-oriented width and height. If the window is not in the native
		// orientation, the dimensions must be reversed.
		DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

		bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
		mD3dRenderTargetSize.Width = swapDimensions ? mOutputSize.Height : mOutputSize.Width;
		mD3dRenderTargetSize.Height = swapDimensions ? mOutputSize.Width : mOutputSize.Height;

	}

	void RenderingGame::SetLogicalSize(Windows::Foundation::Size logicalSize)
	{
		if (mLogicalSize != logicalSize)
		{
			mLogicalSize = logicalSize;		
			CreateWindowSizeDependentResources();
		}
	}

	void RenderingGame::SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation)
	{
		if (mCurrentOrientation != currentOrientation)
		{
			mCurrentOrientation = currentOrientation;
			
			CreateWindowSizeDependentResources();
		}
	}

	void RenderingGame::SetDpi(float dpi)
	{
		if (dpi != mDpi)
		{
			mDpi = dpi;

			CoreWindow^ window = reinterpret_cast<CoreWindow^>(mWindow);

			// When the display DPI changes, the logical size of the window (measured in Dips) also changes and needs to be updated.
			mLogicalSize = Windows::Foundation::Size(window->Bounds.Width, window->Bounds.Height);

			CreateWindowSizeDependentResources();
		}
	}

	void RenderingGame::ValidateDevice()
	{
		// The D3D Device is no longer valid if the default adapter changed since the device
		// was created or if the device has been removed.

		// First, get the information for the default adapter from when the device was created.

	
	}

	void RenderingGame::HandleDeviceLost()
	{

	}

	void RenderingGame::Trim()
	{
		Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
		mD3dDevice2.As(&dxgiDevice);

		dxgiDevice->Trim();
	}

	DXGI_MODE_ROTATION RenderingGame::ComputeDisplayRotation()
	{
		DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

		// Note: NativeOrientation can only be Landscape or Portrait even though
		// the DisplayOrientations enum has other values.
		switch (mNativeOrientation)
		{
		case DisplayOrientations::Landscape:
			switch (mCurrentOrientation)
			{
			case DisplayOrientations::Landscape:
				rotation = DXGI_MODE_ROTATION_IDENTITY;
				break;

			case DisplayOrientations::Portrait:
				rotation = DXGI_MODE_ROTATION_ROTATE270;
				break;

			case DisplayOrientations::LandscapeFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE180;
				break;

			case DisplayOrientations::PortraitFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE90;
				break;
			}
			break;

		case DisplayOrientations::Portrait:
			switch (mCurrentOrientation)
			{
			case DisplayOrientations::Landscape:
				rotation = DXGI_MODE_ROTATION_ROTATE90;
				break;

			case DisplayOrientations::Portrait:
				rotation = DXGI_MODE_ROTATION_IDENTITY;
				break;

			case DisplayOrientations::LandscapeFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE270;
				break;

			case DisplayOrientations::PortraitFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE180;
				break;
			}
			break;
		}
		return rotation;
	}

#endif

	void RenderingGame::ShutDown()
	{
		RasterizerStates::Release();
		SamplerStates::Release();
		BlendStates::Release();

		Game::ShutDown();
	}
}
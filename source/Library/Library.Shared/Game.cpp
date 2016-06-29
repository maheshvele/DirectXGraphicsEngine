#include "pch.h"
#include "GameComponent.h"
#include "Game.h"

using namespace Microsoft::WRL;

namespace Library
{
	RTTI_DEFINITIONS(Game);

	Game::Game()
		:mWindow(nullptr),
		mFeatureLevel(D3D_FEATURE_LEVEL_10_0),
		mDpi(),
		mGameComponents()
	{
	}

	Game::Game(void *window)
		:mWindow(window),
		mFeatureLevel(D3D_FEATURE_LEVEL_10_0),
		mDpi(),
		mGameComponents()
	{
	}

	void Game::Initialize()
	{
		CreateDeviceIndependentResources();
		CreateDeviceResources();
		CreateWindowSizeDependentResources();

		Begin();

		// TODO: Change the timer settings if you want something other than the default variable timestep mode.
		// e.g. for 60 FPS fixed timestep update logic, call:
		mGameTime.SetFixedTimeStep(true);
		mGameTime.SetTargetElapsedSeconds(1.0 / 60);

		for (auto component : mGameComponents)
		{
			component->Initialize();
		}
	}

	void Game::Tick()
	{
		mGameTime.Tick([&]()
		{
			Update();
		});

		Render();
	}

	void Game::Update()
	{
		for (auto component : mGameComponents)
		{
			if (component->IsEnabled())
				component->Update(mGameTime);
		}
	}

	void Game::Render()
	{
		Clear();

		if (mGameTime.GetFrameCount() == 0)
			return;

		for (auto component : mGameComponents)
		{
			if (component->IsEnabled())
				component->Render(mGameTime);
		}

		Present();
	}

	void Game::Clear()
	{
	
	}

	void Game::Present()
	{
	
	}

	void Game::CreateDeviceResources()
	{
		// This flag adds support for surfaces with a different color channel ordering than the API default.
		UINT creationFlags = 0;

#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		static const D3D_FEATURE_LEVEL featureLevels[] =
		{
			// TODO: Modify for supported Direct3D feature levels (see code below related to 11.1 fallback handling)
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};

		// Create the Direct3D 11 API device object and a corresponding context.
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;

		HRESULT hr = D3D11CreateDevice(
			nullptr,					// Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
			0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			creationFlags,				// Set debug and Direct2D compatibility flags.
			featureLevels,				// List of feature levels this app can support.
			ARRAYSIZE(featureLevels),	// Size of the list above.
			D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&device,					// Returns the Direct3D device created.
			&mFeatureLevel,			// Returns feature level of device created.
			&context					// Returns the device immediate context.
			);

		ThrowIfFailed(hr);

		// Store pointers to the Direct3D 11.1 API device and immediate context.
		ThrowIfFailed(
			device.As(&mD3dDevice2)
			);

		ThrowIfFailed(
			context.As(&mD3dContext2)
			);



#ifndef NDEBUG
		ComPtr<ID3D11Debug> d3dDebug;
		hr = mD3dDevice2.As(&d3dDebug);
		if (SUCCEEDED(hr))
		{
			ComPtr<ID3D11InfoQueue> d3dInfoQueue;
			hr = d3dDebug.As(&d3dInfoQueue);
			if (SUCCEEDED(hr))
			{
#ifdef _DEBUG
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
				D3D11_MESSAGE_ID hide[] =
				{
					D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
					// TODO: Add more message IDs here as needed 
				};
				D3D11_INFO_QUEUE_FILTER filter;
				memset(&filter, 0, sizeof(filter));
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);
			}
		}
#endif

		// Create the Direct2D device object and a corresponding context.
	/*	ComPtr<IDXGIDevice2> dxgiDevice;
		ThrowIfFailed(
			mD3dDevice2.As(&dxgiDevice)
			);

		ThrowIfFailed(
			mD2dFactory->CreateDevice(dxgiDevice.Get(), &mD2dDevice)
			);

		ThrowIfFailed(
			mD2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&mD2dContext
			)
			);*/
	}
	
	void Game::CreateDeviceIndependentResources()
	{
		// Initialize Direct2D resources.
		D2D1_FACTORY_OPTIONS options;
		ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
		// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

		// Initialize the Direct2D Factory.
	/*	ThrowIfFailed(
			D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory2),
			&options,
			&mD2dFactory
			)
			);*/

		//// Initialize the DirectWrite Factory.
		//ThrowIfFailed(
		//	DWriteCreateFactory(
		//	DWRITE_FACTORY_TYPE_SHARED,
		//	__uuidof(IDWriteFactory2),
		//	&mDwriteFactory
		//	)
		//	);

		//// Initialize the Windows Imaging Component (WIC) Factory.
		//ThrowIfFailed(
		//	CoCreateInstance(
		//	CLSID_WICImagingFactory2,
		//	nullptr,
		//	CLSCTX_INPROC_SERVER,
		//	IID_PPV_ARGS(&mWicFactory)
		//	)
		//	);
	}

	void Game::CreateWindowSizeDependentResources()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
		// Windows Phone does not support resizing the swap chain, so clear it instead of resizing.
		mSwapChain1 = nullptr;
#endif
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		mD3dContext2->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		mD3dRenderTargetView = nullptr;
		mD3dDepthStencilView = nullptr;
		mD3dContext2->Flush();
		//mD2dContext->SetTarget(nullptr);
		//mD2dTargetBitmap = nullptr;

		float screenWidth, screenHeight;
		
		GetWindowDimensions(screenWidth, screenHeight);

		UINT backBufferWidth = (UINT)screenWidth;
		UINT backBufferHeight = (UINT)screenHeight;

		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		DXGI_FORMAT depthBufferFormat = (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D16_UNORM;


		if (mSwapChain1 != nullptr)
		{
			HRESULT hr = mSwapChain1->ResizeBuffers(2, backBufferWidth, backBufferHeight, backBufferFormat, 0);

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				// If the device was removed for any reason, a new device and swap chain will need to be created.
				//TODO OnDeviceLost();

				// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
				// and correctly set up the new device.
				return;
			}
			else
			{
				ThrowIfFailed(hr);
			}
		}
		else
		{
			// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
			ComPtr<IDXGIDevice3> dxgiDevice;
			ThrowIfFailed(
				mD3dDevice2.As(&dxgiDevice)
				);

			ComPtr<IDXGIAdapter> dxgiAdapter;
			ThrowIfFailed(
				dxgiDevice->GetAdapter(&dxgiAdapter)
				);

			ComPtr<IDXGIFactory2> dxgiFactory;
			ThrowIfFailed(
				dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
				);

#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)

			depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

			// Otherwise, create a new one using the same adapter as the existing Direct3D device.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

			swapChainDesc.Width = backBufferWidth; // Match the size of the window.
			swapChainDesc.Height = backBufferHeight;
			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
			swapChainDesc.Flags = 0;
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;


			ThrowIfFailed(
				dxgiFactory->CreateSwapChainForCoreWindow(
				mD3dDevice2.Get(),
				reinterpret_cast<IUnknown*>(mWindow),
				&swapChainDesc,
				nullptr,
				&mSwapChain1
				)
				);

			ThrowIfFailed(
				dxgiDevice->SetMaximumFrameLatency(1)
				);

#else
			ComPtr<IDXGIFactory2> dxgiFactory2;
			HRESULT hr = dxgiFactory.As(&dxgiFactory2);
			if (SUCCEEDED(hr))
			{
				// DirectX 11.1 or later

				// Create a descriptor for the swap chain.
				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
				swapChainDesc.Width = backBufferWidth;
				swapChainDesc.Height = backBufferHeight;
				swapChainDesc.Format = backBufferFormat;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 2;

				DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
				fsSwapChainDesc.Windowed = TRUE;

				// Create a SwapChain from a CoreWindow.
				ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(
					mD3dDevice2.Get(),
					reinterpret_cast<HWND>(mWindow),
					&swapChainDesc,
					&fsSwapChainDesc,
					nullptr, mSwapChain1.ReleaseAndGetAddressOf()));


			}
			else
			{
				Microsoft::WRL::ComPtr<IDXGISwapChain>			swapChain;
				DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
				swapChainDesc.BufferCount = 2;
				swapChainDesc.BufferDesc.Width = backBufferWidth;
				swapChainDesc.BufferDesc.Height = backBufferHeight;
				swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.OutputWindow = reinterpret_cast<HWND>(mWindow);
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.Windowed = TRUE;

				ThrowIfFailed(dxgiFactory->CreateSwapChain(mD3dDevice2.Get(), &swapChainDesc, swapChain.ReleaseAndGetAddressOf()));

				mSwapChain1.As(&swapChain);
			}

#endif

			// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
			// ensures that the application will only render after each VSync, minimizing power consumption.
			ThrowIfFailed(
				dxgiDevice->SetMaximumFrameLatency(1)
				);
		}

		// Obtain the backbuffer for this window which will be the final 3D rendertarget.
		ComPtr<ID3D11Texture2D> backBuffer;
		ThrowIfFailed(mSwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

		// Create a view interface on the rendertarget to use on bind.
		ThrowIfFailed(mD3dDevice2->CreateRenderTargetView(backBuffer.Get(), nullptr, mD3dRenderTargetView.ReleaseAndGetAddressOf()));

		// Allocate a 2-D surface as the depth/stencil buffer and
		// create a DepthStencil view on this surface to use on bind.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

		ComPtr<ID3D11Texture2D> depthStencil;
		ThrowIfFailed(mD3dDevice2->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		ThrowIfFailed(mD3dDevice2->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, mD3dDepthStencilView.ReleaseAndGetAddressOf()));

		// Create a viewport descriptor of the full window size.
		mScreenViewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(backBufferWidth), static_cast<float>(backBufferHeight));

		// Set the current viewport using the descriptor.
		mD3dContext2->RSSetViewports(1, &mScreenViewport);

		// Create a Direct2D target bitmap associated with the
		// swap chain back buffer and set it as the current target.
	/*	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			mDpi,
			mDpi
			);*/

		ComPtr<IDXGISurface2> dxgiBackBuffer;
		ThrowIfFailed(
			mSwapChain1->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
			);

		//ThrowIfFailed(
		//	mD2dContext->CreateBitmapFromDxgiSurface(
		//	dxgiBackBuffer.Get(),
		//	&bitmapProperties,
		//	&mD2dTargetBitmap
		//	)
		//	);

		//mD2dContext->SetTarget(mD2dTargetBitmap.Get());

		//// Grayscale text anti-aliasing is recommended for all Windows Store apps.
		//mD2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);	
	}

	void Game::AddGameComponent(const std::shared_ptr<GameComponent>& gameComponent)
	{
		mGameComponents.push_back(gameComponent);
	}

	const std::vector<std::shared_ptr<GameComponent>>& Game::GetGameComponents() const
	{
		return mGameComponents;
	}

	const ServiceContainer& Game::Services() const
	{
		return mServices;
	}

	float Game::GetAspectRatio() const
	{
		float screenWidth, screenHeight;

		GetWindowDimensions(screenWidth, screenHeight);

		return screenWidth / screenHeight;
	}

	void Game::ResetRenderTargets()
	{
		mD3dContext2->OMSetRenderTargets(1, mD3dRenderTargetView.GetAddressOf(), mD3dDepthStencilView.Get());
	}

	void Game::UnbindPixelShaderResources(UINT startSlot, UINT count)
	{
		static ID3D11ShaderResourceView* emptySRV[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

		mD3dContext2->PSSetShaderResources(startSlot, count, emptySRV);
	}

	void Game::ShutDown()
	{

		End();

	/*	ID3D11Debug *d3dDebug = nullptr;
		if ((GetDirectXDevice().Get()->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
		{
			d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			d3dDebug = nullptr;
		}*/

		mD3dRenderTargetView.Reset();
		mD3dDepthStencilView.Reset();
		mSwapChain1.Reset();
		mD3depthStencil.Reset();
		
		if (mD3dContext2.Get())
		{
			mD3dContext2->ClearState();
		}

		mD3dContext2.Reset();
		mD3dDevice2.Reset();

		
	}

	void Game::Begin()
	{
		RenderTarget::Begin(mD3dContext2.Get(), 1, mD3dRenderTargetView.GetAddressOf(), mD3dDepthStencilView.Get(), mScreenViewport);
	}

	void Game::End()
	{
		RenderTarget::End(mD3dContext2.Get());
	}
}
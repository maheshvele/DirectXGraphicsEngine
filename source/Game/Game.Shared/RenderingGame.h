#pragma once
#include "Game.h"

namespace RenderGame
{
	class RenderingGame : public Library::Game
	{
		RTTI_DECLARATIONS(RenderingGame, Library::Game);

	public:
		RenderingGame();
		RenderingGame(void* window);
		~RenderingGame();

		virtual void Initialize() override;
		virtual void Update() override;
		virtual void Tick() override;
		virtual void Render() override;
		virtual void Clear() override;
		virtual void Present() override;
		virtual void GetWindowDimensions(float& backBufferWidth, float& backBufferHeight) const override;
		virtual void ShutDown() override;

		void SetWindow(void *window);

#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	public:
	
		void SetWindow(Windows::UI::Core::CoreWindow^ window);
		void SetLogicalSize(Windows::Foundation::Size logicalSize);
		void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
		void SetDpi(float dpi);
		void ValidateDevice();
		void HandleDeviceLost();
		void Trim();
	
		Windows::Foundation::Size	GetOutputSize() const { return mOutputSize; }
		Windows::Foundation::Size	GetLogicalSize() const { return mLogicalSize; }
		DXGI_MODE_ROTATION ComputeDisplayRotation();

		// DirectWrite drawing components
		Microsoft::WRL::ComPtr<IDWriteFactory2>		GetWriteFactory() const{ return mDwriteFactory; }
		Microsoft::WRL::ComPtr<IWICImagingFactory2>	GetWICFactory() const{ return mWicFactory; }
	
	private:
		
		Windows::Foundation::Size	mD3dRenderTargetSize;
		Windows::Foundation::Size	mOutputSize;
		Windows::Foundation::Size	mLogicalSize;
		Windows::Graphics::Display::DisplayOrientations mNativeOrientation;
		Windows::Graphics::Display::DisplayOrientations	mCurrentOrientation;

		// DirectWrite drawing components.
		Microsoft::WRL::ComPtr<IDWriteFactory2>		mDwriteFactory;
		Microsoft::WRL::ComPtr<IWICImagingFactory2>	mWicFactory;
		float										mDpi;
#endif

	private:

		
		std::unique_ptr<RenderStateHelper> mRenderStateHelper;
		
		std::shared_ptr<Library::GamePadComponent> mGamePad;
		
		RenderingGame(const RenderingGame& rhs);
		RenderingGame& operator=(const RenderingGame& rhs);

	};
}



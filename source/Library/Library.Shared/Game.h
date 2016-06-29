#pragma once
#include "RenderTarget.h"

namespace Library
{
	class GameComponent;
	class ServiceContainer;

	class Game : public RenderTarget
	{
		RTTI_DECLARATIONS(Game, RenderTarget)
	public :
		Game();
		Game(void* window);
		virtual ~Game()
		{
			
		};
		
		virtual void Initialize();
		virtual void Update();
		virtual void Tick();
		virtual void Render();
		virtual void Clear();
		virtual void Present();
		virtual void GetWindowDimensions(float& backBufferWidth, float& backBufferHeight) const = 0;
		virtual void ShutDown();

		void AddGameComponent(const std::shared_ptr<GameComponent>& gameComponent);
		const std::vector<std::shared_ptr<GameComponent>>& GetGameComponents() const;
		const ServiceContainer& Services() const;

		void CreateDeviceResources();
		void CreateDeviceIndependentResources();
		void CreateWindowSizeDependentResources();

		float GetAspectRatio() const; 
		
		//Getters for Direct X data
		Microsoft::WRL::ComPtr<ID3D11Device2>	GetDirectXDevice() const { return mD3dDevice2; } 
		Microsoft::WRL::ComPtr<ID3D11DeviceContext2>	GetDirectXContext() const { return mD3dContext2; }
		Microsoft::WRL::ComPtr < IDXGISwapChain1>	GetSwapChain() const { return mSwapChain1; }
		Microsoft::WRL::ComPtr < ID3D11RenderTargetView> GetRenderTargetView() const { return mD3dRenderTargetView; }
		Microsoft::WRL::ComPtr < ID3D11DepthStencilView > GetDepthStencilView() const { return mD3dDepthStencilView; }
		D3D11_VIEWPORT	GetDirectXViewPort() const { return mScreenViewport; }
		Microsoft::WRL::ComPtr < ID3D11Texture2D > GetDirectXDepthStencil() const { return mD3depthStencil; }

		// Direct2D drawing components.
		Microsoft::WRL::ComPtr<ID2D1Factory2>		GetD2dFactory() const{ return mD2dFactory; }
		Microsoft::WRL::ComPtr<ID2D1Device1>		GetD2dDevice() const{ return mD2dDevice; }
		Microsoft::WRL::ComPtr<ID2D1DeviceContext1>	GetD2dDeviceContext() const{ return mD2dContext; }
		Microsoft::WRL::ComPtr<ID2D1Bitmap1>		GetD2dBitmap() const{ return mD2dTargetBitmap; }

		void ResetRenderTargets();
		void UnbindPixelShaderResources(UINT startSlot, UINT count);

	protected:
		virtual void Begin() override;
		virtual void End() override;

		void	*mWindow;
		
		GameTime	mGameTime;
		float		mDpi;

		//Cached Device Properties
		D3D_FEATURE_LEVEL	mFeatureLevel;
		Microsoft::WRL::ComPtr<ID3D11Device2>			mD3dDevice2;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext2>	mD3dContext2;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			mSwapChain1;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	mD3dRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	mD3dDepthStencilView;
		D3D11_VIEWPORT									mScreenViewport;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>         mD3depthStencil;

		// Direct2D drawing components.
		Microsoft::WRL::ComPtr<ID2D1Factory2>		mD2dFactory;
		Microsoft::WRL::ComPtr<ID2D1Device1>		mD2dDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext1>	mD2dContext;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1>		mD2dTargetBitmap;

		ServiceContainer mServices;

	private:
		Game(const Game& rhs);
		Game& operator=(const Game& rhs);

		std::vector<std::shared_ptr<GameComponent>> mGameComponents;
	};
}
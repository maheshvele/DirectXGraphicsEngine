#include "pch.h"
#include "FullScreenRenderTarget.h"

namespace Library
{
	FullScreenRenderTarget::FullScreenRenderTarget(Game& game)
		:mGame(game)
	{
		D3D11_TEXTURE2D_DESC fullScreenTextureDesc;
		ZeroMemory(&fullScreenTextureDesc, sizeof(fullScreenTextureDesc));

		float screenWidth, screenHeight;
		mGame.GetWindowDimensions(screenWidth, screenHeight);

		fullScreenTextureDesc.Width = (UINT)screenWidth;
		fullScreenTextureDesc.Height = (UINT)screenHeight;
		fullScreenTextureDesc.MipLevels = 1;
		fullScreenTextureDesc.ArraySize = 1;
		fullScreenTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		fullScreenTextureDesc.SampleDesc.Count = 1;
		fullScreenTextureDesc.SampleDesc.Quality = 0;
		fullScreenTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		fullScreenTextureDesc.Usage = D3D11_USAGE_DEFAULT;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> fullScreenTexture = nullptr;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateTexture2D(&fullScreenTextureDesc, nullptr, fullScreenTexture.GetAddressOf()));

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateShaderResourceView(fullScreenTexture.Get(), nullptr, mOutputTexture.GetAddressOf()));
		
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateRenderTargetView(fullScreenTexture.Get(), nullptr, mRenderTargetView.GetAddressOf()));

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.Width = (UINT)screenWidth;
		depthStencilDesc.Height = (UINT)screenHeight;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;

		ThrowIfFailed(mGame.
			GetDirectXDevice()->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf()));

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, mDepthStencilView.GetAddressOf()));
	}

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> FullScreenRenderTarget::OutputTexture() const
	{
		return mOutputTexture;
	}

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> FullScreenRenderTarget::RenderTargetView() const
	{
		return mRenderTargetView;
	}

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> FullScreenRenderTarget::DepthStencilView() const
	{
		return mDepthStencilView;
	}

	void FullScreenRenderTarget::Begin()
	{
		
		mGame.GetDirectXContext()->OMSetRenderTargets(1, mRenderTargetView.ReleaseAndGetAddressOf(), mDepthStencilView.Get());
	}

	void FullScreenRenderTarget::End()
	{
		ID3D11RenderTargetView* pNullRTV = NULL;
		
		mGame.GetDirectXContext()->OMSetRenderTargets(1, &pNullRTV, nullptr);
		mGame.ResetRenderTargets();
	}
}
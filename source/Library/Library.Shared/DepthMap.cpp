#include "pch.h"
#include "DepthMap.h"

namespace Library
{
	RTTI_DEFINITIONS(DepthMap)

		DepthMap::DepthMap(const Game& game, UINT width, UINT height)
		: RenderTarget(), mGame(game), mDepthStencilView(nullptr),
		mOutputTexture(nullptr), mViewport()
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
		ThrowIfFailed(game.GetDirectXDevice()->CreateTexture2D(&textureDesc, nullptr, texture.GetAddressOf()));

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
		ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
		resourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		resourceViewDesc.Texture2D.MipLevels = 1;

		ThrowIfFailed(game.GetDirectXDevice()->CreateShaderResourceView(texture.Get(), &resourceViewDesc, mOutputTexture.GetAddressOf()));
		
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		ThrowIfFailed(game.GetDirectXDevice()->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, mDepthStencilView.GetAddressOf()));

		mViewport.TopLeftX = 0.0f;
		mViewport.TopLeftY = 0.0f;
		mViewport.Width = static_cast<float>(width);
		mViewport.Height = static_cast<float>(height);
		mViewport.MinDepth = 0.0f;
		mViewport.MaxDepth = 1.0f;
	}

	DepthMap::~DepthMap()
	{
	}

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DepthMap::OutputTexture() const
	{
		return mOutputTexture;
	}

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthMap::DepthStencilView() const
	{
		return mDepthStencilView;
	}

	void DepthMap::Begin()
	{
		static ID3D11RenderTargetView* nullRenderTargetView = nullptr;
		RenderTarget::Begin(mGame.GetDirectXContext().Get(), 1, &nullRenderTargetView, mDepthStencilView.Get(), mViewport);
	}

	void DepthMap::End()
	{
		RenderTarget::End(mGame.GetDirectXContext().Get());
	}
}
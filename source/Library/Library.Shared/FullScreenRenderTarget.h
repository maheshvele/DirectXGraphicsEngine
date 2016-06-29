#pragma once
#pragma once

namespace Library
{
	class FullScreenRenderTarget
	{
	public:
		FullScreenRenderTarget(Game& game);
		~FullScreenRenderTarget(){};

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> OutputTexture() const;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView() const;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView() const;

		void Begin();
		void End();

	private:
		FullScreenRenderTarget();
		FullScreenRenderTarget(const FullScreenRenderTarget& rhs);
		FullScreenRenderTarget& operator=(const FullScreenRenderTarget& rhs);

		Game& mGame;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mOutputTexture;

	};
}
#pragma once

namespace Library
{
	class Game;

	class DepthMap : public RenderTarget
	{
		RTTI_DECLARATIONS(DepthMap, RenderTarget)

	public:
		DepthMap(const Game& game, UINT width, UINT height);
		~DepthMap();

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> OutputTexture() const;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView() const;

		virtual void Begin() override;
		virtual void End() override;

	private:
		DepthMap();
		DepthMap(const DepthMap& rhs);
		DepthMap& operator=(const DepthMap& rhs);

		const Game& mGame;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mOutputTexture;
		D3D11_VIEWPORT mViewport;
	};
}
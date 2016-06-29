#pragma once

#include "Common.h"

namespace Library
{
	class Game;

	class RenderStateHelper
	{
	public:
		RenderStateHelper(const Game& game);
		~RenderStateHelper();

		static void ResetAll(Microsoft::WRL::ComPtr<ID3D11DeviceContext2> deviceContext);

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerState();
		Microsoft::WRL::ComPtr<ID3D11BlendState> BlendState();
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState();

		void SaveRasterizerState();
		void RestoreRasterizerState() const;

		void SaveBlendState();
		void RestoreBlendState() const;

		void SaveDepthStencilState();
		void RestoreDepthStencilState() const;

		void SaveAll();
		void RestoreAll() const;

	private:
		RenderStateHelper(const RenderStateHelper& rhs);
		RenderStateHelper& operator=(const RenderStateHelper& rhs);

		const Game& mGame;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> mBlendState;
		FLOAT* mBlendFactor;
		UINT mSampleMask;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
		UINT mStencilRef;
	};
}

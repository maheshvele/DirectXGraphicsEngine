#include "pch.h"
#include "RenderStateHelper.h"
#include "Game.h"

namespace Library
{
	RenderStateHelper::RenderStateHelper(const Game& game)
		: mGame(game), mRasterizerState(nullptr), mBlendState(nullptr), mBlendFactor(new FLOAT[4]), mSampleMask(UINT_MAX), mDepthStencilState(nullptr), mStencilRef(UINT_MAX)
	{
	}

	RenderStateHelper::~RenderStateHelper()
	{
		if ((mBlendFactor) != nullptr) 
		{ 
			delete[] mBlendFactor; 
			mBlendFactor = nullptr;
		}
	}

	void RenderStateHelper::ResetAll(Microsoft::WRL::ComPtr<ID3D11DeviceContext2> deviceContext)
	{
		deviceContext->RSSetState(nullptr);
		deviceContext->OMSetBlendState(nullptr, nullptr, UINT_MAX);
		deviceContext->OMSetDepthStencilState(nullptr, UINT_MAX);
	}

	void RenderStateHelper::SaveRasterizerState()
	{
		mGame.GetDirectXContext()->RSGetState(&mRasterizerState);
	}

	void RenderStateHelper::RestoreRasterizerState() const
	{
		mGame.GetDirectXContext()->RSSetState(mRasterizerState.Get());
	}

	void RenderStateHelper::SaveBlendState()
	{
		mGame.GetDirectXContext()->OMGetBlendState(&mBlendState, mBlendFactor, &mSampleMask);
	}

	void RenderStateHelper::RestoreBlendState() const
	{
		mGame.GetDirectXContext()->OMSetBlendState(mBlendState.Get(), mBlendFactor, mSampleMask);
	}

	void RenderStateHelper::SaveDepthStencilState()
	{
		mGame.GetDirectXContext()->OMGetDepthStencilState(&mDepthStencilState, &mStencilRef);
	}

	void RenderStateHelper::RestoreDepthStencilState() const
	{
		mGame.GetDirectXContext()->OMSetDepthStencilState(mDepthStencilState.Get(), mStencilRef);
	}

	void RenderStateHelper::SaveAll()
	{
		SaveRasterizerState();
		SaveBlendState();
		SaveDepthStencilState();
	}

	void RenderStateHelper::RestoreAll() const
	{
		RestoreRasterizerState();
		RestoreBlendState();
		RestoreDepthStencilState();
	}
}

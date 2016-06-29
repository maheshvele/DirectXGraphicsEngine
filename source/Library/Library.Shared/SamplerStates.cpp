#include "pch.h"
#include "SamplerStates.h"

namespace Library
{
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStates::TrilinearWrap = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStates::TrilinearMirror = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStates::TrilinearClamp = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStates::TrilinerBorder = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStates::DepthMap = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerStates::PcfShadowMap = nullptr;


	XMVECTORF32 SamplerStates::BorderColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMVECTORF32 SamplerStates::WhiteColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	void SamplerStates::Initialize(ID3D11Device* direct3DDevice)
	{
		assert(direct3DDevice != nullptr);

		D3D11_SAMPLER_DESC samplerStateDesc;
		ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));
		samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		ThrowIfFailed(direct3DDevice->CreateSamplerState(&samplerStateDesc, TrilinearWrap.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed for Wrap.");
		
		ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));
		samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
		samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
		samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;

		ThrowIfFailed(direct3DDevice->CreateSamplerState(&samplerStateDesc, TrilinearMirror.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed for Mirror.");

		ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));
		samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		ThrowIfFailed(direct3DDevice->CreateSamplerState(&samplerStateDesc, TrilinearClamp.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed for Clamp.");

		ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));
		samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		memcpy(samplerStateDesc.BorderColor, reinterpret_cast<FLOAT*>(&BorderColor), sizeof(FLOAT) * 4);

		ThrowIfFailed(direct3DDevice->CreateSamplerState(&samplerStateDesc, TrilinerBorder.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed for Border.");

		ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));
		samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		memcpy(samplerStateDesc.BorderColor, reinterpret_cast<FLOAT*>(&WhiteColor), sizeof(FLOAT) * 4);

		ThrowIfFailed(direct3DDevice->CreateSamplerState(&samplerStateDesc, DepthMap.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed for Border.");

		ZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));
		samplerStateDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		memcpy(samplerStateDesc.BorderColor, reinterpret_cast<FLOAT*>(&WhiteColor), sizeof(FLOAT) * 4);

		ThrowIfFailed(direct3DDevice->CreateSamplerState(&samplerStateDesc, PcfShadowMap.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed for Border.");

	}

	void SamplerStates::Release()
	{
		TrilinearWrap.Reset();
		TrilinearMirror.Reset();
		TrilinearClamp.Reset();
		TrilinerBorder.Reset();
		DepthMap.Reset();
		PcfShadowMap.Reset();
	}
}
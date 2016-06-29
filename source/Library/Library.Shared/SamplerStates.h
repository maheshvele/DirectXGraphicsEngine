#pragma once

namespace Library
{
	class SamplerStates
	{
	public:
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> TrilinearWrap;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> TrilinearMirror;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> TrilinearClamp;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> TrilinerBorder;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> DepthMap;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> PcfShadowMap;

		static XMVECTORF32 BorderColor;
		static XMVECTORF32 WhiteColor;

		static void Initialize(ID3D11Device* direct3DDevice);
		static void Release();

	private:
		SamplerStates();
		SamplerStates(const SamplerStates& rhs);
		SamplerStates& operator=(const SamplerStates& rhs);
	};
}
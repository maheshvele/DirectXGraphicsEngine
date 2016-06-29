#include "pch.h"
#include "ColoredTriangleDemo.h"
#include "Game.h"

using namespace Library;
using namespace std;
using namespace DirectX;

namespace RenderGame
{
	RTTI_DEFINITIONS(ColoredTriangleDemo);


	ColoredTriangleDemo::ColoredTriangleDemo(const Library::Game& game)
		: RenderingGameComponent(game)
	{

	}

	ColoredTriangleDemo::~ColoredTriangleDemo()
	{
	}

	void ColoredTriangleDemo::Initialize()
	{
		string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\ColoredTriangleVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");
		

		string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\ColoredTrianglePS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		//Create an Input layout

		D3D11_INPUT_ELEMENT_DESC inputElementtDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementtDescriptions, ARRAYSIZE(inputElementtDescriptions), &compiledVertexShader[0],
			compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout Failed");


		VertexPositionColor vertices[] =
		{
			VertexPositionColor(XMFLOAT4(-0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(1, 0, 0, 1)),
			VertexPositionColor(XMFLOAT4( 0.0f,  0.5f, 0.0f, 1.0f), XMFLOAT4(0, 1, 0, 1)),
			VertexPositionColor(XMFLOAT4( 0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(0, 0, 1, 1))
		};

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionColor) *ARRAYSIZE(vertices);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.ReleaseAndGetAddressOf()), "CreateBuffer : Failed");
	}

	void ColoredTriangleDemo::Update(const GameTime& gameTime)
	{

	}

	void ColoredTriangleDemo::Render(const GameTime& gameTime)
	{
		if (gameTime.GetFrameCount() == 0)
			return;

		mGame.GetDirectXContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mGame.GetDirectXContext()->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPositionColor);
		UINT offset = 0;
		mGame.GetDirectXContext()->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);


		mGame.GetDirectXContext()->VSSetShader(mVertexShader.Get(), nullptr, 0);
		mGame.GetDirectXContext()->PSSetShader(mPixelShader.Get(), nullptr, 0);

		mGame.GetDirectXContext()->Draw(3, 0);
	}

}
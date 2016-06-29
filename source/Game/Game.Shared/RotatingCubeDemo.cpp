#include "pch.h"
#include "RotatingCubeDemo.h"

using namespace Library;
using namespace std;
using namespace DirectX;
namespace RenderGame
{
	RTTI_DEFINITIONS(RotatingCubeDemo);

	RotatingCubeDemo::RotatingCubeDemo(const Library::Game& game)
		: RenderingGameComponent(game), bLoadingComplete(false), mDegreesPerSecond(45), mIndexCount(0), bTracking(false)
	{
	}

	void RotatingCubeDemo::Initialize()
	{
		string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\RotatingCubeVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");
		
		string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\RotatingCubePS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "CreateInputLayout() : Failed");

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mConstantBuffer), "CreateConstantBuffer() : Failed");
				
		CreateCubeMesh();

		bLoadingComplete = true;

		float screenWidth, screenHeight;

		mGame.GetWindowDimensions(screenWidth, screenHeight);

		float aspectRatio = screenWidth / screenHeight;
		float fovAngleY = 70.0f * XM_PI / 180.0f;

		// This is a simple example of change that can be made when the app is in
		// portrait or snapped view.
		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		// Note that the OrientationTransform3D matrix is post-multiplied here
		// in order to correctly orient the scene to match the display orientation.
		// This post-multiplication step is required for any draw calls that are
		// made to the swap chain render target. For draw calls to other targets,
		// this transform should not be applied.

		// This sample makes use of a right-handed coordinate system using row-major matrices.
		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
			fovAngleY,
			aspectRatio,
			0.01f,
			100.0f
			);

		XMFLOAT4X4 orientation = XMFLOAT4X4 (
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			);;

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(
			&mConstantBufferData.projection,
			XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
			);

		// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
		static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMStoreFloat4x4(&mConstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	}

	void RotatingCubeDemo::CreateCubeMesh()
	{

		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			VertexPositionColor(XMFLOAT4(-0.5f, -0.5f, -0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)),
			VertexPositionColor(XMFLOAT4(-0.5f, -0.5f,  0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)),
			VertexPositionColor(XMFLOAT4(-0.5f,  0.5f, -0.5f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)),
			VertexPositionColor(XMFLOAT4(-0.5f,  0.5f,  0.5f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)),
			VertexPositionColor(XMFLOAT4( 0.5f, -0.5f, -0.5f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)),
			VertexPositionColor(XMFLOAT4( 0.5f, -0.5f,  0.5f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)),
			VertexPositionColor(XMFLOAT4( 0.5f,  0.5f, -0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)),
			VertexPositionColor(XMFLOAT4( 0.5f,  0.5f,  0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f))
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, mVertexBuffer.ReleaseAndGetAddressOf()), "CreateVertexBuffer Failed");

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0, 2, 1, // -x
			1, 2, 3,

			4, 5, 6, // +x
			5, 7, 6,

			0, 1, 5, // -y
			0, 5, 4,

			2, 6, 7, // +y
			2, 7, 3,

			0, 4, 6, // -z
			0, 6, 2,

			1, 3, 7, // +z
			1, 7, 5,
		};

		mIndexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, mIndexBuffer.ReleaseAndGetAddressOf()), "CreateIndexBuffer Failed");
	}

	void RotatingCubeDemo::Update(const GameTime& gameTime)
	{
		if (!bTracking)
		{
			// Convert degrees to radians, then convert seconds to rotation angle
			float radiansPerSecond = XMConvertToRadians(mDegreesPerSecond);
			double totalRotation = gameTime.GetTotalSeconds() * radiansPerSecond;
			float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

			Rotate(radians);
		}
	}

	void RotatingCubeDemo::Render(const GameTime& gameTime)
	{
		// Loading is asynchronous. Only draw geometry after it's loaded.
		if (!bLoadingComplete)
		{
			return;
		}

		auto context = mGame.GetDirectXContext();

		// Prepare the constant buffer to send it to the graphics device.
		context->UpdateSubresource(	mConstantBuffer.Get(), 0, NULL,	&mConstantBufferData, 0, 0);

		// Each vertex is one instance of the VertexPositionColor struct.
		UINT stride = sizeof(VertexPositionColor);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(),	&stride, &offset);

		context->IASetIndexBuffer( mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->IASetInputLayout(mInputLayout.Get());

		// Attach our vertex shader.
		context->VSSetShader(mVertexShader.Get(), nullptr, 0);

		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers(0, 1,	mConstantBuffer.GetAddressOf());

		// Attach our pixel shader.
		context->PSSetShader(mPixelShader.Get(), nullptr, 0);

		// Draw the objects.
		context->DrawIndexed(mIndexCount, 0, 0);

	}

	void RotatingCubeDemo::Rotate(float radians)
	{
		XMStoreFloat4x4(&mConstantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
	}

}
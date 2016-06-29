#include "pch.h"
#include "GamePadComponent.h"
#include "Skybox.h"


namespace Library
{
	RTTI_DEFINITIONS(Skybox);

	Skybox::Skybox(const Game& game, const std::shared_ptr<Camera>& camera, const std::wstring& cubeMapFileName, float scale)
		:RenderingGameComponent(game, camera), mCubeMapFileName(cubeMapFileName), 
		mVertexCBufferPerObjectData(), mIndexCount(0),
		mWorldMatrix(MatrixHelper::Identity), mScaleMatrix(MatrixHelper::Identity)
	{
		XMStoreFloat4x4(&mScaleMatrix, XMMatrixScaling(scale, scale, scale));
	}

	void Skybox::Initialize()
	{
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif

		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\SkyboxVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\SkyboxPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		// Load the model
		std::unique_ptr<Model> model = std::make_unique<Model>(mGame, "Content\\Models\\Sphere.bin", true);

		// Create vertex and index buffers for the model
		std::shared_ptr<Mesh> mesh = model->Meshes().at(0);
		CreateVertexBuffer(mGame.GetDirectXDevice().Get(), mesh, mVertexBuffer.GetAddressOf());
		mesh->CreateIndexBuffer(mIndexBuffer.GetAddressOf());
		mIndexCount = (UINT)mesh->Indices().size();

		// Load a texture
		std::wstring textureName = L"Content\\Textures\\Maskonaive2_1024.dds";
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile(mGame.GetDirectXDevice().Get(), mCubeMapFileName.c_str(), nullptr, mSkyboxTexture.GetAddressOf()), "CreateDDSTextureFromFile() failed.");

	//	ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetDirectXDevice().Get(), mGame.GetDirectXContext().Get(), textureName.c_str(), nullptr, mSkyboxTexture.GetAddressOf()), "CreateWICTextureFromFile() failed.");

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;


		ThrowIfFailed(mGame.GetDirectXDevice()->CreateSamplerState(&samplerDesc, mTrilinearSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

	}

	void Skybox::Update(const GameTime& gameTime)
	{
		const XMFLOAT3& position = mCamera->Position();

		XMStoreFloat4x4(&mWorldMatrix, XMLoadFloat4x4(&mScaleMatrix) * XMMatrixTranslation(position.x, position.y, position.z));

	}

	void Skybox::Render(const GameTime& gameTime)
	{
		ID3D11DeviceContext2* direct3DDeviceContext = mGame.GetDirectXContext().Get();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPosition);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));

		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVertexCBufferPerObject.GetAddressOf());

		direct3DDeviceContext->PSSetShaderResources(0, 1, mSkyboxTexture.GetAddressOf());
		direct3DDeviceContext->PSSetSamplers(0, 1, mTrilinearSampler.GetAddressOf());

		direct3DDeviceContext->RSSetState(RasterizerStates::DisabledCulling.Get());
		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
		direct3DDeviceContext->RSSetState(nullptr);
	}

	void Skybox::CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh->Vertices();

		std::shared_ptr<std::vector<XMFLOAT3>> textureCoordinates = mesh->TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());

		std::vector<VertexPosition> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			vertices.push_back(VertexPosition(XMFLOAT4(position.x, position.y, position.z, 1.0f)));
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexPosition) * vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}
}
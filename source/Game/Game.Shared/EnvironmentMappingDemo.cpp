#include "pch.h"
#include "EnvironmentMappingDemo.h"

namespace RenderGame
{
	RTTI_DEFINITIONS(EnvironmentMappingDemo);

	const size_t EnvironmentMappingDemo::Alignment = 16;
	const XMFLOAT4 EnvironmentMappingDemo::AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	const XMFLOAT4 EnvironmentMappingDemo::EnvColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	void* EnvironmentMappingDemo::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

		void EnvironmentMappingDemo::operator delete(void *p)
	{
		if (p != nullptr)
		{
#if defined(DEBUG) || defined(_DEBUG)
			_aligned_free_dbg(p);
#else
			_aligned_free(p);
#endif
		}
	}

	EnvironmentMappingDemo::EnvironmentMappingDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera)
		:RenderingGameComponent(game, camera),
		mVertexCBufferPerFrameData(),
		mVertexCBufferPerObjectData(),
		mPixelCBufferPerFrameData(),
		mPixelCBufferPerObjectData(),
		mWorldMatrix(MatrixHelper::Identity),
		mIndexCount(0),
		mReflectionAmount(1.0f),
		mRenderStateHelper(game),
		mSpriteBatch(nullptr),
		mSpriteFont(nullptr),
		mTextPosition(0.0f, 40.0f),
		mGamePad(nullptr)
	{
	}

	void EnvironmentMappingDemo::Initialize()
	{
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif

		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\EnvironmentMappingDemoVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\EnvironmentMappingDemoPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		// Load the model
		std::unique_ptr<Model> model = std::make_unique<Model>(mGame, "Content\\Models\\Sphere.bin", true);

		// Create vertex and index buffers for the model
		std::shared_ptr<Mesh> mesh = model->Meshes().at(0);
		CreateVertexBuffer(mGame.GetDirectXDevice().Get(), mesh, mVertexBuffer.GetAddressOf());
		mesh->CreateIndexBuffer(mIndexBuffer.GetAddressOf());
		mIndexCount = (UINT)mesh->Indices().size();

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerFrameData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerObjectData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerFrameData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		// Load a texture
		std::wstring textureName = L"Content\\Textures\\Maskonaive2_1024.dds";
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile(mGame.GetDirectXDevice().Get(), textureName.c_str(), nullptr, mEnvironmentMap.GetAddressOf()), "CreateDDSTextureFromFile() failed.");

		textureName = L"Content\\Textures\\Checkerboard.png";
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetDirectXDevice().Get(), mGame.GetDirectXContext().Get(), textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateWICTextureFromFile() failed.");

		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateSamplerState(&samplerDesc, mTrilinearSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

		// Create text rendering helpers
		mSpriteBatch = std::make_unique<SpriteBatch>(mGame.GetDirectXContext().Get());
		mSpriteFont = std::make_unique<SpriteFont>(mGame.GetDirectXDevice().Get(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the GamePad service
		mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
		assert(mGamePad != nullptr);

		mPixelCBufferPerObjectData.ReflectionAmount = mReflectionAmount;
		mPixelCBufferPerFrameData.AmbientColor = AmbientColor;
		mPixelCBufferPerFrameData.EnvColor = EnvColor;
	}

	void EnvironmentMappingDemo::Update(const GameTime& gameTime)
	{
		float elapsedTime = static_cast<float>(gameTime.GetElapsedSeconds());

		if (mGamePad->IsButtonHeldDown(GamePadButton::X) && mReflectionAmount < 1.0f)
		{
			mReflectionAmount += elapsedTime;
			mReflectionAmount = min(mReflectionAmount, 1.0f);
			mPixelCBufferPerObjectData.ReflectionAmount = mReflectionAmount;
		}

		if (mGamePad->IsButtonHeldDown(GamePadButton::Y) && mReflectionAmount > 0.0f)
		{
			mReflectionAmount -= elapsedTime;
			mReflectionAmount = max(mReflectionAmount, 0.0f);
			mPixelCBufferPerObjectData.ReflectionAmount = mReflectionAmount;
		}
	}

	void EnvironmentMappingDemo::Render(const GameTime& gameTime)
	{
		ID3D11DeviceContext2* direct3DDeviceContext = mGame.GetDirectXContext().Get();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPositionTextureNormal);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

		mVertexCBufferPerFrameData.CameraPosition = mCamera->Position();

		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

		static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame.Get(), mVertexCBufferPerObject.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame.Get(), mPixelCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		static ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get(), mEnvironmentMap.Get() };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		direct3DDeviceContext->PSSetSamplers(0, 1, mTrilinearSampler.GetAddressOf());

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		mRenderStateHelper.SaveAll();

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;

		helpLabel << L"Reflection Amount (+X/-Y): " << mReflectionAmount << "\n";

		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();

	}

	void EnvironmentMappingDemo::CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh->Vertices();

		std::shared_ptr<std::vector<XMFLOAT3>> textureCoordinates = mesh->TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());

		const std::vector<XMFLOAT3>& normals = mesh->Normals();
		assert(normals.size() == sourceVertices.size());

		std::vector<VertexPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			vertices.push_back(VertexPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal));
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexPositionTextureNormal) * vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");

	}
}
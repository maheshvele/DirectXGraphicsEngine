#include "pch.h"
#include "FogDemo.h"

namespace RenderGame
{
	RTTI_DEFINITIONS(FogDemo);

	const size_t FogDemo::Alignment = 16;
	const XMFLOAT2 FogDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
	const float FogDemo::LightModulationRate = UCHAR_MAX;
	const XMFLOAT4 FogDemo::FogColor = XMFLOAT4(&ColorHelper::CornflowerBlue[0]);

	void* FogDemo::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

	void FogDemo::operator delete(void *p)
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

	FogDemo::FogDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera)
		:RenderingGameComponent(game, camera),
		mVertexCBufferPerFrameData(),
		mVertexCBufferPerObjectData(),
		mPixelCBufferPerFrameData(),
		mPixelCBufferPerObjectData(),
		mFogStart(75.0f), mFogRange(40.0f),
		mWorldMatrix(MatrixHelper::Identity),
		mDirectionalLight(nullptr),
		mIndexCount(0),
		mRenderStateHelper(game),
		mSpriteBatch(nullptr),
		mSpriteFont(nullptr),
		mTextPosition(0.0f, 40.0f),
		mProxyModel(nullptr),
		mGamePad(nullptr)
	{
	}

	void FogDemo::Initialize()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif

		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\FogDemoVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\FogDemoPS.cso");
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
		std::wstring textureName = L"Content\\Textures\\Earthatday.dds";
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile(mGame.GetDirectXDevice().Get(), textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateDDSTextureFromFile() failed.");

		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateSamplerState(&samplerDesc, mColorSampler.GetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

		// Create text rendering helpers
		mSpriteBatch = std::make_unique<SpriteBatch>(mGame.GetDirectXContext().Get());
		mSpriteFont = std::make_unique<SpriteFont>(mGame.GetDirectXDevice().Get(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the keyboard service
		mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
		assert(mGamePad!= nullptr);

		mProxyModel = std::make_unique<ProxyModel>(mGame, mCamera, "Content\\Models\\DirectionalLightProxy.bin", 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));


		mDirectionalLight = std::make_unique<DirectionalLight>(mGame);
		const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
		mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);

		const XMFLOAT3& cameraPosition = mCamera->Position();
		mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
		mPixelCBufferPerFrameData.CameraPosition = mVertexCBufferPerFrameData.CameraPosition;

		mVertexCBufferPerFrameData.FogStart = mFogStart;
		mVertexCBufferPerFrameData.FogRange = mFogRange;
		mPixelCBufferPerFrameData.FogColor = FogColor;
	}

	void FogDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdateDirectionalLight(gameTime);
		UpdateSpecularLight(gameTime);
		mProxyModel->Update(gameTime);
	}

	void FogDemo::Render(const GameTime& gameTime)
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

		const XMFLOAT3& cameraPosition = mCamera->Position();
		mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
		mPixelCBufferPerFrameData.CameraPosition = mVertexCBufferPerFrameData.CameraPosition;

		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

		static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame.Get(), mVertexCBufferPerObject.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame.Get(), mPixelCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		direct3DDeviceContext->PSSetShaderResources(0, 1, mColorTexture.GetAddressOf());
		direct3DDeviceContext->PSSetSamplers(0, 1, mColorSampler.GetAddressOf());

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		mProxyModel->Render(gameTime);

		mRenderStateHelper.SaveAll();

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;

		helpLabel << "Ambient Intensity (+X/-Y): " << mPixelCBufferPerFrameData.AmbientColor.x << "\n";
		helpLabel << L"Specular Intensity (+DPadUp/-DPadDown): " << mPixelCBufferPerObjectData.SpecularColor.x << "\n";
		helpLabel << L"Specular Power (+RightShoulder/-LeftShoulder): " << mPixelCBufferPerObjectData.SpecularPower << "\n";
		helpLabel << L"Directional Light Intensity (+A/-B): " << mPixelCBufferPerFrameData.LightColor.x << "\n";
		helpLabel << L"Rotate Directional Light (Hold Left Trigger + Right Thumb Stick)";

		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();
	}

	void FogDemo::CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const
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

	void FogDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = 0.0f;

		if (mGamePad->IsButtonHeldDown(GamePadButton::X) && ambientIntensity < 1.0f)
		{
			ambientIntensity += static_cast<float>(gameTime.GetElapsedSeconds());
			ambientIntensity = min(ambientIntensity, 1.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}

		if (mGamePad->IsButtonHeldDown(GamePadButton::Y) && ambientIntensity > 0.0f)
		{
			ambientIntensity -= (float)gameTime.GetElapsedSeconds();
			ambientIntensity = max(ambientIntensity, 0.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}
	}

	void FogDemo::UpdateDirectionalLight(const GameTime& gameTime)
	{
		static float directionalIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.GetElapsedSeconds());

		if (mGamePad != nullptr)
		{
			auto gamePadState = mGamePad->CurrentState();
			if (gamePadState.IsConnected())
			{
				// Update directional light intensity		
				if (mGamePad->IsButtonHeldDown(GamePadButton::A) && directionalIntensity < 1.0f)
				{
					directionalIntensity += elapsedTime;
					directionalIntensity = min(directionalIntensity, 1.0f);

					mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
					mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
				}
				if (mGamePad->IsButtonHeldDown(GamePadButton::B) && directionalIntensity > 0.0f)
				{
					directionalIntensity -= elapsedTime;
					directionalIntensity = max(directionalIntensity, 0.0f);

					mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
					mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
				}


				if (gamePadState.IsLeftTriggerPressed())
				{
					// Rotate directional light
					XMFLOAT2 rotationAmount = Vector2Helper::Zero;

					rotationAmount.x = -gamePadState.thumbSticks.rightX;
					rotationAmount.y = gamePadState.thumbSticks.rightY;

					XMMATRIX lightRotationMatrix = XMMatrixIdentity();
					if (rotationAmount.x != 0)
					{
						lightRotationMatrix = XMMatrixRotationY(rotationAmount.x);
					}

					if (rotationAmount.y != 0)
					{
						XMMATRIX lightRotationAxisMatrix = XMMatrixRotationAxis(mDirectionalLight->RightVector(), rotationAmount.y);
						lightRotationMatrix *= lightRotationAxisMatrix;
					}

					if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
					{
						mDirectionalLight->ApplyRotation(lightRotationMatrix);
						mProxyModel->ApplyRotation(lightRotationMatrix);

						const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
						mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);
					}
				}
			}
		}
	}

	void FogDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = 1.0f;

		if (mGamePad->IsButtonHeldDown(GamePadButton::DPadUp) && specularIntensity < 1.0f)
		{
			specularIntensity += static_cast<float>(gameTime.GetElapsedSeconds());
			specularIntensity = min(specularIntensity, 1.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		if (mGamePad->IsButtonHeldDown(GamePadButton::DPadDown) && specularIntensity > 0.0f)
		{
			specularIntensity -= (float)gameTime.GetElapsedSeconds();
			specularIntensity = max(specularIntensity, 0.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		static float specularPower = mPixelCBufferPerObjectData.SpecularPower;

		if (mGamePad->IsButtonHeldDown(GamePadButton::RightShoulder) && specularPower < UCHAR_MAX)
		{
			specularPower += LightModulationRate * static_cast<float>(gameTime.GetElapsedSeconds());
			specularPower = min(specularPower, static_cast<float>(UCHAR_MAX));

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}

		if (mGamePad->IsButtonHeldDown(GamePadButton::LeftShoulder) && specularPower > 1.0f)
		{
			specularPower -= LightModulationRate * static_cast<float>(gameTime.GetElapsedSeconds());
			specularPower = max(specularPower, 1.0f);

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}
}

}
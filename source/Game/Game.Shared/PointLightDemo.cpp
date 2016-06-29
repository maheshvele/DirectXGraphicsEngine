#include "pch.h"
#include "PointLightDemo.h"

namespace RenderGame
{
	RTTI_DEFINITIONS(PointLightDemo)

	const size_t PointLightDemo::Alignment = 16;
	const float PointLightDemo::LightModulationRate = UCHAR_MAX;
	const float PointLightDemo::LightMovementRate = 10.0f;
	const float PointLightDemo::UpdateRadiusRate = 10.0f;

	void* PointLightDemo::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

	void PointLightDemo::operator delete(void *p)
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

	PointLightDemo::PointLightDemo(const Library::Game& game, const std::shared_ptr<Library::Camera>& camera)
		: RenderingGameComponent(game, camera),
		mVertexCBufferPerObjectData(),
		mVertexCBufferPerFrameData(),
		mPixelCBufferPerObjectData(), 
		mPixelCBufferPerFrameData(),
		mWorldMatrix(MatrixHelper::Identity), 
		mPointLight(nullptr),
		mIndexCount(0),
		mRenderStateHelper(game), 
		mSpriteBatch(nullptr), 
		mSpriteFont(nullptr), 
		mTextPosition(0.0f, 40.0f),
		mProxyModel(nullptr)
	{
	}

	PointLightDemo::~PointLightDemo()
	{
	}

	void PointLightDemo::Initialize()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif
				
		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\PointLightDemoVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");
		
		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\PointLightDemoPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");
		
		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

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
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetDirectXDevice().Get(), mGame.GetDirectXContext().Get(), textureName.c_str(), nullptr, mColorTexture.GetAddressOf()), "CreateWICTextureFromFile() failed.");

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
		
		// Retrieve the GamePad service
		mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
		assert(mGamePad != nullptr);


		mProxyModel = std::make_unique<ProxyModel>(mGame, mCamera, "Content\\Models\\PointLightProxy.bin", 0.5f);
		mProxyModel->Initialize();
		
		mPointLight = std::make_unique<PointLight>(mGame);
		mPointLight->SetRadius(50.0f);
		mPointLight->SetPosition(5.0f, 0.0f, 10.0f);

		mVertexCBufferPerFrameData.LightPosition = mPointLight->Position();
		mVertexCBufferPerFrameData.LightRadius = mPointLight->Radius();

		mPixelCBufferPerFrameData.LightPosition = mPointLight->Position();
		mPixelCBufferPerFrameData.LightColor = ColorHelper::ToFloat4(mPointLight->Color(), true);
		mPixelCBufferPerFrameData.CameraPosition = mCamera->Position();
	}

	void PointLightDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdatePointLight(gameTime);
		UpdateSpecularLight(gameTime);
		UpdateRadius(gameTime);
		mProxyModel->Update(gameTime);
	}

	void PointLightDemo::Render(const GameTime& gameTime)
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

		mPixelCBufferPerFrameData.CameraPosition = mCamera->Position();

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
		helpLabel << L"Point Light Intensity (+A/-B): " << mPixelCBufferPerFrameData.LightColor.x << "\n";
		helpLabel << L"Point Light Radius (+Star/-Back) " << mPointLight->Radius() << "\n";
		helpLabel << L"Move Point Light (Hold Right Trigger + Right/Left Thumb Sticks)";

		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();
	}

	void PointLightDemo::CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const
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

	void PointLightDemo::UpdateAmbientLight(const GameTime& gameTime)
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

	void PointLightDemo::UpdatePointLight(const GameTime& gameTime)
	{
		static float pointLightIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.GetElapsedSeconds());
		// Move point light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;

		if (mGamePad != nullptr)
		{
			auto gamePadState = mGamePad->CurrentState();

			if (gamePadState.IsConnected())
			{
				// Update point light intensity		
				if (mGamePad->IsButtonHeldDown(GamePadButton::A) && pointLightIntensity < 1.0f)
				{
					pointLightIntensity += elapsedTime;
					pointLightIntensity = min(pointLightIntensity, 1.0f);

					mPixelCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
					mPointLight->SetColor(mPixelCBufferPerFrameData.LightColor);
				}
				if (mGamePad->IsButtonHeldDown(GamePadButton::B) && pointLightIntensity > 0.0f)
				{
					pointLightIntensity -= elapsedTime;
					pointLightIntensity = max(pointLightIntensity, 0.0f);

					mPixelCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
					mPointLight->SetColor(mPixelCBufferPerFrameData.LightColor);
				}

				if (gamePadState.IsRightTriggerPressed())
				{
					movementAmount.x = gamePadState.thumbSticks.leftX;
					movementAmount.y = gamePadState.thumbSticks.leftY;
					movementAmount.z = -gamePadState.thumbSticks.rightY;
				}
			}
		}

		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mPointLight->SetPosition(mPointLight->PositionVector() + movement);
		mProxyModel->SetPosition(mPointLight->Position());
		mVertexCBufferPerFrameData.LightPosition = mPointLight->Position();

		mPixelCBufferPerFrameData.LightPosition = mPointLight->Position();
	}

	void PointLightDemo::UpdateSpecularLight(const GameTime& gameTime)
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

	void PointLightDemo::UpdateRadius(const GameTime& gameTime)
	{
		if (mGamePad != nullptr)
		{
			auto gamePadState = mGamePad->CurrentState();

			if (gamePadState.IsConnected())
			{
				if (mGamePad->IsButtonHeldDown(GamePadButton::Back) && mPointLight->Radius() > 0)
				{
					float radius = mPointLight->Radius() - (float)(UpdateRadiusRate * gameTime.GetElapsedSeconds()); 

					if (radius < 0)
						radius = 0;

					mPointLight->SetRadius(radius);
					mVertexCBufferPerFrameData.LightRadius = mPointLight->Radius();

				}

				if (mGamePad->IsButtonHeldDown(GamePadButton::Start))
				{
					float radius = mPointLight->Radius() + (float)(UpdateRadiusRate * gameTime.GetElapsedSeconds());

					if (radius > 50)
						radius = 50;

					mPointLight->SetRadius(radius);
					mVertexCBufferPerFrameData.LightRadius = mPointLight->Radius();

				}
			}
		}
	}
}
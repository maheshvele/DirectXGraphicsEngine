#include "pch.h"
#include "AnimationDemo.h"

namespace RenderGame
{
	RTTI_DEFINITIONS(AnimationDemo);

	const size_t AnimationDemo::Alignment = 16;
	const float AnimationDemo::LightModulationRate = UCHAR_MAX;
	const float AnimationDemo::LightMovementRate = 10.0f;

	void* AnimationDemo::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

	void AnimationDemo::operator delete(void *p)
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

	AnimationDemo::AnimationDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera)
		:RenderingGameComponent(game, camera),
		mWorldMatrix(MatrixHelper::Identity),
		mVertexBuffers(), mIndexBuffers(), mIndexCounts(), mColorTextures(),
		mGamePad(nullptr), mAmbientColor(reinterpret_cast<const float*>(&ColorHelper::White)), mPointLight(nullptr),
		mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), mSkinnedModel(nullptr), mAnimationPlayer(nullptr),
		mRenderStateHelper(game), mProxyModel(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f), mManualAdvanceMode(true)
	{
	}

	AnimationDemo::~AnimationDemo()
	{
		for (auto vertexBuffer : mVertexBuffers)
		{
			vertexBuffer.Reset();
		}

		for (auto indexBuffer : mIndexBuffers)
		{
			indexBuffer.Reset();
		}

		for (auto colorTexture : mColorTextures)
		{
			colorTexture.Reset();
		}

		mSpriteFont.reset();
		mSpriteBatch.reset();
		mSkinnedModel.reset();
		mAnimationPlayer.reset();
		mProxyModel.reset();
		mPointLight.reset();
		
	}

	void AnimationDemo::Initialize()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif

		//Load the compiled Vertex Shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\AnimationDemoVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\AnimationDemoPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Load the model
		mSkinnedModel = std::make_unique<Model>(mGame, "Content\\Models\\NoTPoseRunning.bin", true);

		// Create the vertex and index buffers
		mVertexBuffers.resize(mSkinnedModel->Meshes().size());
		mIndexBuffers.resize(mSkinnedModel->Meshes().size());
		mIndexCounts.resize(mSkinnedModel->Meshes().size());
		mColorTextures.resize(mSkinnedModel->Meshes().size());

		for (UINT i = 0; i < mSkinnedModel->Meshes().size(); i++)
		{
			auto mesh = mSkinnedModel->Meshes().at(i);

			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			CreateVertexBuffer(mGame.GetDirectXDevice().Get(), mesh, vertexBuffer.GetAddressOf());
			mVertexBuffers[i] = vertexBuffer;

			Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
			mesh->CreateIndexBuffer(indexBuffer.GetAddressOf());
			mIndexBuffers[i] = indexBuffer;

			mIndexCounts[i] = (UINT)mesh->Indices().size();

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorTexture;
			auto material = mesh->GetMaterial();

			//if (material != nullptr && material->Textures().find(Library::TextureType::TextureTypeDifffuse) != material->Textures().cend())
			if (material.get() != nullptr)
			{
				
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
				std::vector<std::string>* diffuseTextures = material->Textures().at(TextureTypeDifffuse).get();

				std::wstring fullPathName(diffuseTextures->at(0).begin(), diffuseTextures->at(0).end());
				std::wstring filename = PathFindFileName(fullPathName.c_str());

				std::wostringstream textureName;
				textureName << L"Content\\Textures\\" << filename.substr(0, filename.length() - 4) << L".png";
				ThrowIfFailed(CreateWICTextureFromFile(mGame.GetDirectXDevice().Get(), mGame.GetDirectXContext().Get(), textureName.str().c_str(), nullptr, colorTexture.GetAddressOf()), "CreateWICTTextureFile Failed");
				
#else
				std::wstring textureName = L"Content\\Textures\\Soldier.png";
				ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetDirectXDevice().Get(), mGame.GetDirectXContext().Get(), textureName.c_str(), nullptr, colorTexture.GetAddressOf()), "CreateWICTTextureFile Failed");

#endif
			
			}
			mColorTextures[i] = colorTexture;
		}
		
		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerFrameData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferSkinningData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferSkinning.GetAddressOf()), "ID3D11Device::CreateBuffer() failed");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerObjectData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerFrameData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");


		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(0.05f, 0.05f, 0.05f));

		mPointLight = std::make_unique<PointLight>(mGame);
		mPointLight->SetRadius(500.0f);
		mPointLight->SetPosition(5.0f, 0.0f, 10.0f);

		mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
		assert(mGamePad != nullptr);

		mAnimationPlayer = std::make_unique<AnimationPlayer>(mGame, *mSkinnedModel, false);
		mAnimationPlayer->StartClip(*(mSkinnedModel->Animations().at(0)));

		mProxyModel = std::make_unique<ProxyModel>(mGame, mCamera, "Content\\Models\\PointLightProxy.bin", 0.5f);
		mProxyModel->Initialize();

		mSpriteBatch = std::make_unique<SpriteBatch>(mGame.GetDirectXContext().Get());
		mSpriteFont = std::make_unique<SpriteFont>(mGame.GetDirectXDevice().Get(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

	}

	void AnimationDemo::Update(const GameTime& gameTime)
	{
		UpdateOptions();
		UpdateAmbientLight(gameTime);
		UpdatePointLight(gameTime);
		UpdateSpecularLight(gameTime);

		if (mManualAdvanceMode == false)
		{
			mAnimationPlayer->Update(gameTime);
		}

		mProxyModel->Update(gameTime);
	}

	void AnimationDemo::Render(const GameTime& gameTime)
	{
		ID3D11DeviceContext2* direct3DDeviceContext = mGame.GetDirectXContext().Get();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		UINT stride = sizeof(VertexSkinnedPositionTextureNormal);
		UINT offset = 0;
		for (UINT i = 0; i < mVertexBuffers.size(); i++)
		{
			auto vertexBuffer = mVertexBuffers[i];
			auto indexBuffer = mIndexBuffers[i];
			UINT indexCount = mIndexCounts[i];
			auto colorTexture = mColorTextures[i];

			direct3DDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
			direct3DDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			//Vertex Constant Buffers
			XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
			XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
			XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
			mVertexCBufferPerFrameData.LightPosition = mPointLight->Position();
			mVertexCBufferPerFrameData.LightRadius = mPointLight->Radius();

			for (std::uint32_t i = 0; i < mAnimationPlayer->BoneTransforms().size(); ++i)
			{
				XMMATRIX boneTransform = XMLoadFloat4x4(&mAnimationPlayer->BoneTransforms().at(i));
				XMStoreFloat4x4(&mVertexCBufferSkinningData.BoneTransforms[i], XMMatrixTranspose(boneTransform));
			}

			//Pixel Constant Buffers
			XMVECTOR ambientColor = XMLoadColor(&mAmbientColor);
			XMVECTOR lightColor = XMLoadColor(&mPointLight->Color());
			XMVECTOR specularColor = XMLoadColor(&mSpecularColor);

			XMStoreFloat4(&mPixelCBufferPerObjectData.SpecularColor, specularColor);
			mPixelCBufferPerObjectData.SpecularPower = mSpecularPower;
			
			XMStoreFloat4(&mPixelCBufferPerFrameData.AmbientColor, ambientColor);
			XMStoreFloat4(&mPixelCBufferPerFrameData.LightColor, lightColor);
			mPixelCBufferPerFrameData.LightPosition = mPointLight->Position();
			mPixelCBufferPerFrameData.CameraPosition = mCamera->Position();

			direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
			direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
			direct3DDeviceContext->UpdateSubresource(mVertexCBufferSkinning.Get(), 0, nullptr, &mVertexCBufferSkinningData, 0, 0);
			
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

			static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame.Get(), mVertexCBufferPerObject.Get(), mVertexCBufferSkinning.Get() };
			direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

			static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame.Get(), mPixelCBufferPerObject.Get() };
			direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

			static ID3D11ShaderResourceView* ShaderResourcesPlane[] = { colorTexture.Get() };
			direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(ShaderResourcesPlane), ShaderResourcesPlane);
			
			static ID3D11SamplerState* SamplerStatePlane[] = { SamplerStates::TrilinearWrap.Get() };
			direct3DDeviceContext->PSSetSamplers(0, ARRAYSIZE(SamplerStatePlane), SamplerStatePlane);

			direct3DDeviceContext->DrawIndexed(indexCount, 0, 0);
			const_cast<Library::Game&>(mGame).UnbindPixelShaderResources(0, 2);

			static ID3D11SamplerState* emptySRV[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
			direct3DDeviceContext->PSSetSamplers(0, 1, emptySRV);


		}

		mProxyModel->Render(gameTime);

		mRenderStateHelper.SaveAll();

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << std::setprecision(5) << L"Ambient Intensity (+DPadRight/-DPadLeft): " << mAmbientColor.a << "\n";
		helpLabel << L"Point Light Intensity (Right Trigger(Hold) +RightShoulder/-LeftShoulder): " << mPointLight->Color().a << "\n";
		helpLabel << L"Specular Power (+DPadUp/-DPadDown): " << mSpecularPower << "\n";
		helpLabel << L"Move Point Light (Right Trigger (Hold) Left Joystick & Right Joystick)\n";
		helpLabel << "Frame Advance Mode (Y): " << (mManualAdvanceMode ? "Manual" : "Auto") << "\nAnimation Time: " << mAnimationPlayer->CurrentTime()
			<< "\nFrame Interpolation (X): " << (mAnimationPlayer->InterpolationEnabled() ? "On" : "Off") << "\nGo to Bind Pose (B)";

		if (mManualAdvanceMode)
		{
			helpLabel << "\nCurrent Keyframe (Right Joystick): " << mAnimationPlayer->CurrentKeyframe();
		}
		else
		{
			helpLabel << "\nPause / Resume(A)";
		}
		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();
	}

	void AnimationDemo::UpdateOptions()
	{
		if (mGamePad != nullptr)
		{
			if (mGamePad->WasButtonPressedThisFrame(GamePadButton::A))
			{
				if (mAnimationPlayer->IsPlayingClip())
				{
					mAnimationPlayer->PauseClip();
				}
				else
				{
					mAnimationPlayer->ResumeClip();
				}
			}

			if (mGamePad->WasButtonPressedThisFrame(GamePadButton::B))
			{
				// Reset the animation clip to the bind pose
				mAnimationPlayer->StartClip(*(mSkinnedModel->Animations().at(0)));
			}

			if (mGamePad->WasButtonPressedThisFrame(GamePadButton::X))
			{
				// Enable/disabled interpolation
				mAnimationPlayer->SetInterpolationEnabled(!mAnimationPlayer->InterpolationEnabled());
			}

			if (mGamePad->WasButtonPressedThisFrame(GamePadButton::Y))
			{
				// Enable/disable manual advance mode
				mManualAdvanceMode = !mManualAdvanceMode;
				mAnimationPlayer->SetCurrentKeyFrame(0);
			}

			if (mManualAdvanceMode && mGamePad->WasButtonPressedThisFrame(GamePadButton::RightStick))
			{
				// Advance the current keyframe
				UINT currentKeyFrame = mAnimationPlayer->CurrentKeyframe();
				currentKeyFrame++;
				if (currentKeyFrame >= mAnimationPlayer->CurrentClip()->KeyframeCount())
				{
					currentKeyFrame = 0;
				}

				mAnimationPlayer->SetCurrentKeyFrame(currentKeyFrame);
			}
		}
	}

	void AnimationDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mAmbientColor.a;

		if (mGamePad != nullptr)
		{
			if (mGamePad->IsButtonHeldDown(GamePadButton::RightShoulder) && ambientIntensity < UCHAR_MAX)
			{
				ambientIntensity += LightModulationRate * (float)gameTime.GetElapsedSeconds();
				mAmbientColor.a = (UCHAR)XMMin<float>(ambientIntensity, UCHAR_MAX);
			}

			if (mGamePad->IsButtonHeldDown(GamePadButton::LeftShoulder) && ambientIntensity > 0)
			{
				ambientIntensity -= LightModulationRate * (float)gameTime.GetElapsedSeconds();
				mAmbientColor.a = (UCHAR)XMMax<float>(ambientIntensity, 0);
			}
		}
	}

	void AnimationDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = mSpecularPower;

		if (mGamePad != nullptr)
		{
			if (mGamePad->IsButtonHeldDown(GamePadButton::DPadUp) && specularIntensity < UCHAR_MAX)
			{
				specularIntensity += LightModulationRate * (float)gameTime.GetElapsedSeconds();
				specularIntensity = XMMin<float>(specularIntensity, UCHAR_MAX);

				mSpecularPower = specularIntensity;;
			}

			if (mGamePad->IsButtonHeldDown(GamePadButton::DPadDown) && specularIntensity > 0)
			{
				specularIntensity -= LightModulationRate * (float)gameTime.GetElapsedSeconds();
				specularIntensity = XMMax<float>(specularIntensity, 0);

				mSpecularPower = specularIntensity;
			}
		}
	}

	void AnimationDemo::UpdatePointLight(const GameTime& gameTime)
	{
		static float pointLightIntensity = mPointLight->Color().a;
		float elapsedTime = static_cast<float>(gameTime.GetElapsedSeconds());
		// Move point light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;

		if (mGamePad != nullptr)
		{
			auto gamePadState = mGamePad->CurrentState();
			
			if (gamePadState.IsConnected())
			{
				if (gamePadState.IsRightTriggerPressed())
				{
					// Update point light intensity		
					if (mGamePad->IsButtonHeldDown(GamePadButton::DPadRight) && pointLightIntensity < UCHAR_MAX)
					{
						pointLightIntensity += LightModulationRate * elapsedTime;

						XMCOLOR pointLightLightColor = mPointLight->Color();
						pointLightLightColor.a = (UCHAR)XMMin<float>(pointLightIntensity, UCHAR_MAX);
						mPointLight->SetColor(pointLightLightColor);
					}
					if (mGamePad->IsButtonHeldDown(GamePadButton::DPadLeft) && pointLightIntensity > 0)
					{
						pointLightIntensity -= LightModulationRate * elapsedTime;

						XMCOLOR pointLightLightColor = mPointLight->Color();
						pointLightLightColor.a = (UCHAR)XMMax<float>(pointLightIntensity, 0.0f);
						mPointLight->SetColor(pointLightLightColor);
					}

					movementAmount.x = gamePadState.thumbSticks.leftX;
					movementAmount.y = gamePadState.thumbSticks.leftY;
					movementAmount.z = -gamePadState.thumbSticks.rightY;
				}
			}
		}

		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mPointLight->SetPosition(mPointLight->PositionVector() + movement);
		mProxyModel->SetPosition(mPointLight->Position());
	}

	void AnimationDemo::CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer)
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh->Vertices();
		auto textureCoordinates = mesh->TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());
		const std::vector<XMFLOAT3>& normals = mesh->Normals();
		assert(normals.size() == sourceVertices.size());
		const std::vector<BoneVertexWeights>& boneWeights = mesh->BoneWeights();
		assert(boneWeights.size() == sourceVertices.size());

		std::vector<VertexSkinnedPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			BoneVertexWeights vertexWeights = boneWeights.at(i);

			float weights[BoneVertexWeights::MaxBoneWeightsPerVertex];
			UINT indices[BoneVertexWeights::MaxBoneWeightsPerVertex];
			ZeroMemory(weights, sizeof(float) * ARRAYSIZE(weights));
			ZeroMemory(indices, sizeof(UINT) * ARRAYSIZE(indices));
			for (UINT i = 0; i < vertexWeights.Weights().size(); i++)
			{
				BoneVertexWeights::VertexWeight vertexWeight = vertexWeights.Weights().at(i);
				weights[i] = vertexWeight.Weight;
				indices[i] = vertexWeight.BoneIndex;
			}

			vertices.push_back(VertexSkinnedPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal, XMUINT4(indices), XMFLOAT4(weights)));
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexSkinnedPositionTextureNormal) * vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}
}
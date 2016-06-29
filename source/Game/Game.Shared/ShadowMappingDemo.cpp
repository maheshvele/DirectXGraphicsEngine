#include "pch.h"
#include "d3dCompiler.h"
#include "d3d11Shader.h"
#include "ShadowMappingDemo.h"

namespace RenderGame
{
	RTTI_DEFINITIONS(ShadowMappingDemo);

	const size_t ShadowMappingDemo::Alignment = 16;
	const float ShadowMappingDemo::LightModulationRate = UCHAR_MAX;
	const float ShadowMappingDemo::LightMovementRate = 10.0f;
	const XMFLOAT2 ShadowMappingDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
	const UINT ShadowMappingDemo::DepthMapWidth = 1024U;
	const UINT ShadowMappingDemo::DepthMapHeight = 1024U;
	const RECT ShadowMappingDemo::DepthMapDestinationRectangle = { 0, 512, 256, 768 };
	const float ShadowMappingDemo::DepthBiasModulationRate = 10000;

	ShadowMappingDemo::ShadowMappingDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera)
		: RenderingGameComponent(game, camera),
		 mPlaneVertexCount(0), mGamePad(nullptr), mAmbientColor(1.0f, 1.0f, 1.0, 0.0f), mPointLight(nullptr),
		mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), mPlaneWorldMatrix(MatrixHelper::Identity), mProxyModel(nullptr),
		mProjector(nullptr), mProjectorFrustum(XMMatrixIdentity()), mRenderableProjectorFrustum(nullptr),
		mProjectedTextureScalingMatrix(MatrixHelper::Zero), mRenderStateHelper(game),
		mModelPositionVertexBuffer(nullptr), mModelPositionUVNormalVertexBuffer(nullptr), mModelIndexBuffer(nullptr), mModelIndexCount(0),
		mModelWorldMatrix(MatrixHelper::Identity), mDepthMap(nullptr), mDrawDepthMap(true),
		mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f), mActiveTechnique(ShadowMappingTechniqueSimple),
		mDepthBiasState(nullptr), mDepthBias(0), mSlopeScaledDepthBias(2.0f)
	{
	}

	ShadowMappingDemo::~ShadowMappingDemo()
	{
		mShadowMappingInputLayout.Reset();
		mDepthMapInputLayout.Reset();

		mPlanePositionVertexBuffer.Reset();
		mPlanePositionUVNormalVertexBuffer.Reset();
		mPlaneIndexBuffer.Reset();

		mCheckerboardTexture.Reset();
		mModelPositionVertexBuffer.Reset();
		mModelPositionUVNormalVertexBuffer.Reset();
		mModelIndexBuffer.Reset();

		mDepthBiasState.Reset();

		mShadowMappingPixelShader.Reset();
		mShadowMappingVertexShader.Reset();
		mColorSampler.Reset();
		mShadowMapSampler.Reset();
		mPcfShadowMapSampler.Reset();

		mShadowMappingVertexCBufferPerFrame.Reset();
		mShadowMappingVertexCBufferPerObject.Reset();
		mShadowMappingPixelCBufferPerFrame.Reset();
		mShadowMappingPixelCBufferPerObject.Reset();

		mDepthMapVertexShader.Reset();
		mDepthMapVertexCBufferPerObject.Reset();

		mShadowMappingClassLinkage.Reset();
		for (int i = 0; i < ShadowMappingTechnique::ShadowMappingTechniqueEnd; ++i)
		{
			mShadowMappingInstance[i].Reset();
		}
		
	
	}

	void ShadowMappingDemo::Initialize()
	{
		

		InitializeShadowMapping();
		InitializeDepthMap();

		/*** Load the teapot model ***/
		std::unique_ptr<Model> teapotModel = std::make_unique<Model>(mGame, "Content\\Models\\Teapot.bin", true);

		// Create vertex and index buffers for the model
		std::shared_ptr<Mesh> mesh = teapotModel->Meshes().at(0);
		CreateShadowMappingVertexBuffer(mGame.GetDirectXDevice().Get(), mesh, mModelPositionUVNormalVertexBuffer.GetAddressOf());
		CreateDepthMapVertexBuffer(mGame.GetDirectXDevice().Get(), mesh, mModelPositionVertexBuffer.GetAddressOf());
		
		mesh->CreateIndexBuffer(mModelIndexBuffer.GetAddressOf());
		mModelIndexCount = (UINT)mesh->Indices().size();

		XMStoreFloat4x4(&mModelWorldMatrix, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 5.0f, 2.5f));

		/*** Load the checkered plane ***/
		XMStoreFloat4x4(&mPlaneWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));

		VertexPositionTextureNormal positionUVNormalVertices[] =
		{
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
		};

		mPlaneVertexCount = ARRAYSIZE(positionUVNormalVertices);
		std::vector<VertexPositionNormal> positionNormalVertices;
		positionNormalVertices.reserve(mPlaneVertexCount);
		std::vector<VertexPosition> positionVertices;
		positionVertices.reserve(mPlaneVertexCount);
		for (UINT i = 0; i < mPlaneVertexCount; i++)
		{
			positionNormalVertices.push_back(VertexPositionNormal(positionUVNormalVertices[i].Position, positionUVNormalVertices[i].Normal));
			positionVertices.push_back(VertexPosition(positionUVNormalVertices[i].Position));
		}

		CreateDepthMapVertexBuffer(mGame.GetDirectXDevice().Get(), &positionVertices[0], mPlaneVertexCount, &mPlanePositionVertexBuffer);
		CreateShadowMappingVertexBuffer(mGame.GetDirectXDevice().Get(), positionUVNormalVertices, mPlaneVertexCount, &mPlanePositionUVNormalVertexBuffer);

		std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame.GetDirectXDevice().Get(), mGame.GetDirectXContext().Get(), textureName.c_str(), nullptr, mCheckerboardTexture.GetAddressOf()), "CreateWICTTextureFile Failed");
		
		/*** Create text rendering helpers ***/
		mSpriteBatch = std::make_unique<SpriteBatch>(mGame.GetDirectXContext().Get());
		mSpriteFont = std::make_unique<SpriteFont>(mGame.GetDirectXDevice().Get(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		/*** Retrieve the GamePad service ***/
		mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
		assert(mGamePad != nullptr);

		/*** Create Point Light and corresponding proxy model ***/
		mProxyModel = std::make_unique<ProxyModel>(mGame, mCamera, "Content\\Models\\PointLightProxy.bin", 0.5f);
		mProxyModel->Initialize();

		mPointLight = std::make_unique<PointLight>(mGame);
		mPointLight->SetRadius(50.0f);
		mPointLight->SetPosition(0.0f, 5.0f, 2.0f);

		/*** Create Projector and corresponding frustum ***/
		mProjector = std::make_unique<Projector>(mGame);
		mProjector->Initialize();

		mProjectorFrustum.SetMatrix(mProjector->ViewProjectionMatrix());

		mRenderableProjectorFrustum = std::make_unique<RenderableFrustum>(mGame, mCamera);
		mRenderableProjectorFrustum->Initialize();
		mRenderableProjectorFrustum->InitializeGeometry(mProjectorFrustum);

		InitializeProjectedTextureScalingMatrix();

		mDepthMap = std::make_unique<DepthMap>(mGame, DepthMapWidth, DepthMapHeight);
		
		UpdateDepthBiasState();

		mProxyModel->SetPosition(mPointLight->Position());
		mProjector->SetPosition(mPointLight->Position());
		mRenderableProjectorFrustum->SetPosition(mPointLight->Position());
	}

	void ShadowMappingDemo::InitializeShadowMapping()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateClassLinkage(mShadowMappingClassLinkage.ReleaseAndGetAddressOf()), "CreateClassLinkage : Failed");

		// Load a compiled vertex shader for shadow mapping
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\ShadowMappingDemoVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mShadowMappingVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");

		// Load a compiled pixel shader for shadow mapping
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\ShadowMappingDemoPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), mShadowMappingClassLinkage.Get(), mShadowMappingPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		//load the class interfaces
		for (int i = 0; i < ShadowMappingTechnique::ShadowMappingTechniqueEnd; ++i)
		{
			ThrowIfFailed(mShadowMappingClassLinkage->CreateClassInstance(ShadowMappingTechniqueNames[i].c_str(), 0, 0, 0, 0, mShadowMappingInstance[i].ReleaseAndGetAddressOf()), "CreateClassInstance Failed()");
		}

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mShadowMappingInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		//Create Constant Buffers for shadow mapping
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mShadowMappingVertexBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mShadowMappingVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mShadowMappingVertexBufferPerFrameData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mShadowMappingVertexCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mShadowMappingPixelBufferPerObjectData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mShadowMappingPixelCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mShadowMappingPixelBufferPerFrameData);
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mShadowMappingPixelCBufferPerFrame.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	}

	void ShadowMappingDemo::InitializeDepthMap()
	{
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif
		// Load a compiled vertex shader for shadow mapping
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\DepthMapVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mDepthMapVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mDepthMapInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		//Create Constant buffers for depth map
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mDepthMapVertexBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mDepthMapVertexCBufferPerObject.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

	}

	void ShadowMappingDemo::Update(const GameTime& gameTime)
	{
	
		UpdateTechnique();
		UpdateDepthBias(gameTime);
		UpdateAmbientLight(gameTime);
		UpdatePointLightAndProjector(gameTime);
		UpdateSpecularLight(gameTime);

		mProxyModel->Update(gameTime);
		mProjector->Update(gameTime);
		mRenderableProjectorFrustum->Update(gameTime);
	}

	void ShadowMappingDemo::Render(const GameTime& gameTime)
	{	
		static float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		UINT stride, offset;

		ID3D11DeviceContext2* direct3DDeviceContext = mGame.GetDirectXContext().Get();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#pragma region DepthMapRendering

		mRenderStateHelper.SaveRasterizerState();
		mDepthMap->Begin();

		direct3DDeviceContext->ClearDepthStencilView(mDepthMap->DepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		direct3DDeviceContext->IASetInputLayout(mDepthMapInputLayout.Get());

		direct3DDeviceContext->RSSetState(mDepthBiasState.Get());

		stride = sizeof(VertexPosition);
		offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mModelPositionVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mModelIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mDepthMapVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(nullptr, nullptr, 0);

		XMMATRIX teapotWorldMatrix = XMLoadFloat4x4(&mModelWorldMatrix);
		XMMATRIX depthMapWVP = teapotWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix();

		XMStoreFloat4x4(&mDepthMapVertexBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(depthMapWVP));

		direct3DDeviceContext->UpdateSubresource(mDepthMapVertexCBufferPerObject.Get(), 0, nullptr, &mDepthMapVertexBufferPerObjectData, 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mDepthMapVertexCBufferPerObject.GetAddressOf());

		direct3DDeviceContext->DrawIndexed(mModelIndexCount, 0, 0);
		
		const_cast<Library::Game&>(mGame).UnbindPixelShaderResources(0, 1);

		mDepthMap->End();

		mRenderStateHelper.RestoreRasterizerState();

#pragma endregion DepthMapRendering

#pragma  region CheckeredBoardPlane

		/*** Render the checkered board plane ***/
		direct3DDeviceContext->IASetInputLayout(mShadowMappingInputLayout.Get());

		stride = sizeof(VertexPositionTextureNormal);
		offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mPlanePositionUVNormalVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mPlaneIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mShadowMappingVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mShadowMappingPixelShader.Get(), mShadowMappingInstance[(int)(mActiveTechnique)].GetAddressOf(), 1);

		XMMATRIX planeWorldMatrix = XMLoadFloat4x4(&mPlaneWorldMatrix);
		XMMATRIX planeWVP = planeWorldMatrix * mCamera->ViewProjectionMatrix();
		XMMATRIX planeProjectiveTextureMatrix = planeWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix() * XMLoadFloat4x4(&mProjectedTextureScalingMatrix);
		
		XMVECTOR ambientColor = XMLoadColor(&mAmbientColor);
		XMVECTOR specularColor = XMLoadColor(&mSpecularColor);
		XMVECTOR shadowMapSize = XMVectorSet(static_cast<float>(DepthMapWidth), static_cast<float>(DepthMapHeight), 0, 0);
		XMVECTOR lightColor = XMLoadColor(&mPointLight->Color());

		//Vertex Constant Buffers
		XMStoreFloat4x4(&mShadowMappingVertexBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(planeWVP));
		XMStoreFloat4x4(&mShadowMappingVertexBufferPerObjectData.World, XMMatrixTranspose(planeWorldMatrix));
		XMStoreFloat4x4(&mShadowMappingVertexBufferPerObjectData.ProjectiveTextureMatrix, XMMatrixTranspose(planeProjectiveTextureMatrix));
		mShadowMappingVertexBufferPerFrameData.LightPosition = mPointLight->Position();
		mShadowMappingVertexBufferPerFrameData.LightRadius = mPointLight->Radius();

		//Pixel Constant Buffers
		XMStoreFloat4(&mShadowMappingPixelBufferPerObjectData.SpecularColor, specularColor);
		mShadowMappingPixelBufferPerObjectData.SpecularPower = mSpecularPower;

		XMStoreFloat4(&mShadowMappingPixelBufferPerFrameData.AmbientColor, ambientColor);
		XMStoreFloat4(&mShadowMappingPixelBufferPerFrameData.LightColor, lightColor);

		mShadowMappingPixelBufferPerFrameData.LightPosition = mPointLight->Position();
		mShadowMappingPixelBufferPerFrameData.CameraPosition = mCamera->Position();
		XMStoreFloat2(&mShadowMappingPixelBufferPerFrameData.ShadowMapSize, shadowMapSize);

		direct3DDeviceContext->UpdateSubresource(mShadowMappingVertexCBufferPerFrame.Get(), 0, nullptr, &mShadowMappingVertexBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mShadowMappingVertexCBufferPerObject.Get(), 0, nullptr, &mShadowMappingVertexBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mShadowMappingPixelCBufferPerFrame.Get(), 0, nullptr, &mShadowMappingPixelBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mShadowMappingPixelCBufferPerObject.Get(), 0, nullptr, &mShadowMappingPixelBufferPerObjectData, 0, 0);

		static ID3D11Buffer* VSConstantBuffersPlane[] = { mShadowMappingVertexCBufferPerFrame.Get(), mShadowMappingVertexCBufferPerObject.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffersPlane), VSConstantBuffersPlane);

		static ID3D11Buffer* PSConstantBuffersPlane[] = { mShadowMappingPixelCBufferPerFrame.Get(), mShadowMappingPixelCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffersPlane), PSConstantBuffersPlane);

		static ID3D11ShaderResourceView* ShaderResourcesPlane[] = { mCheckerboardTexture.Get(), mDepthMap->OutputTexture().Get() };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(ShaderResourcesPlane), ShaderResourcesPlane);

		static ID3D11SamplerState* SamplerStatePlane[] = { SamplerStates::PcfShadowMap.Get(), SamplerStates::DepthMap.Get(), SamplerStates::TrilinearWrap.Get()};
		direct3DDeviceContext->PSSetSamplers(0, ARRAYSIZE(SamplerStatePlane), SamplerStatePlane);

		direct3DDeviceContext->Draw(mPlaneVertexCount, 0);
		const_cast<Library::Game&>(mGame).UnbindPixelShaderResources(0, 2);

		static ID3D11SamplerState* emptySRV[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
		direct3DDeviceContext->PSSetSamplers(0, 2, emptySRV);

#pragma endregion CheckeredBoardPlane

#pragma region TeapotRendering
		///*** Render the Teapot ***/
		
		stride = sizeof(VertexPositionTextureNormal);
		offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mModelPositionUVNormalVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mModelIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//XMMATRIX teapotWorldMatrix = XMLoadFloat4x4(&mModelWorldMatrix);
		XMMATRIX teapotWVP = teapotWorldMatrix * mCamera->ViewProjectionMatrix();
		XMMATRIX teapotProjectiveTextureMatrix = teapotWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix() * XMLoadFloat4x4(&mProjectedTextureScalingMatrix);
		
		XMStoreFloat4x4(&mShadowMappingVertexBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(teapotWVP));
		XMStoreFloat4x4(&mShadowMappingVertexBufferPerObjectData.World, XMMatrixTranspose(teapotWorldMatrix));
		XMStoreFloat4x4(&mShadowMappingVertexBufferPerObjectData.ProjectiveTextureMatrix, XMMatrixTranspose(teapotProjectiveTextureMatrix));
		mShadowMappingPixelBufferPerFrameData.CameraPosition = mCamera->Position();
		mShadowMappingPixelBufferPerFrameData.ShadowMapSize.x = static_cast<float>(DepthMapWidth);
		mShadowMappingPixelBufferPerFrameData.ShadowMapSize.y = static_cast<float>(DepthMapHeight);

		direct3DDeviceContext->UpdateSubresource(mShadowMappingVertexCBufferPerFrame.Get(), 0, nullptr, &mShadowMappingVertexBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mShadowMappingVertexCBufferPerObject.Get(), 0, nullptr, &mShadowMappingVertexBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mShadowMappingPixelCBufferPerFrame.Get(), 0, nullptr, &mShadowMappingPixelBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mShadowMappingPixelCBufferPerObject.Get(), 0, nullptr,&mShadowMappingPixelBufferPerObjectData, 0, 0);

		static ID3D11Buffer* VSConstantBuffers[] = { mShadowMappingVertexCBufferPerFrame.Get(), mShadowMappingVertexCBufferPerObject.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		static ID3D11Buffer* PSConstantBuffers[] = { mShadowMappingPixelCBufferPerFrame.Get(), mShadowMappingPixelCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);
		
		static ID3D11ShaderResourceView* ShaderResourcesModel[] = { mCheckerboardTexture.Get(), mDepthMap->OutputTexture().Get()};
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(ShaderResourcesModel), ShaderResourcesModel);
		
		static ID3D11SamplerState* SamplerStateModel[] = {SamplerStates::PcfShadowMap.Get(), SamplerStates::DepthMap.Get(), SamplerStates::TrilinearWrap.Get()};
		direct3DDeviceContext->PSSetSamplers(0, ARRAYSIZE(SamplerStateModel), SamplerStateModel);

		direct3DDeviceContext->DrawIndexed(mModelIndexCount, 0, 0);
		const_cast<Library::Game&>(mGame).UnbindPixelShaderResources(0, 2);
		direct3DDeviceContext->PSSetSamplers(0, 2, emptySRV);

#pragma endregion TeapotRendering


		mProxyModel->Render(gameTime);

		mRenderableProjectorFrustum->Render(gameTime);
		mRenderStateHelper.SaveAll();

		mSpriteBatch->Begin();

		/*** Render the depth map ***/
		if (mDepthMap)
		{
			mSpriteBatch->Draw(mDepthMap->OutputTexture().Get(), DepthMapDestinationRectangle);
		}

		std::wostringstream helpLabel;

		helpLabel << L"Ambient Intensity (+X/-Y): " << mAmbientColor.a << "\n";
		helpLabel << L"Point Light Intensity (+RightShoulder/-LeftShoulder): " << mPointLight->Color().a << "\n";
		helpLabel << L"Specular Power (+A/-B): " << mSpecularPower << "\n";
		helpLabel << L"Move Projector/Light (RightTrigger Hold + RightJoyStick )\n";
		helpLabel << L"Rotate Projector (RightTrigger Hold + DPad)\n";
		helpLabel << std::setprecision(5) << L"Active Technique (RightJoyStick): " << ShadowMappingDisplayNames[mActiveTechnique].c_str() << "\n";

		if (mActiveTechnique == ShadowMappingTechniquePCF)
		{
			helpLabel << L"Depth Bias (+Start/-Back): " << (int)mDepthBias << "\n"
				<< L"Slope-Scaled Depth Bias (RightTrigger Hold)(+Start/-Back): " << mSlopeScaledDepthBias;
		}

		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();
	}

	void ShadowMappingDemo::UpdateTechnique()
	{
		if (mGamePad)
		{
			if (mGamePad->WasButtonUp(GamePadButton::RightStick) && mGamePad->IsButtonDown(GamePadButton::RightStick))
			{
				mActiveTechnique = ShadowMappingTechnique(mActiveTechnique + 1);
				if (mActiveTechnique >= ShadowMappingTechnique::ShadowMappingTechniqueEnd)
				{
					mActiveTechnique = ShadowMappingTechnique::ShadowMappingTechniqueSimple;
				}


			}
		}

	}

	void ShadowMappingDemo::UpdateDepthBias(const GameTime& gameTime)
	{
		if (mGamePad!= nullptr)
		{
			auto gamePadState = mGamePad->CurrentState();

			if (gamePadState.IsConnected())
			{
				if (gamePadState.IsRightTriggerPressed())
				{
					if (mGamePad->IsButtonDown(GamePadButton::Start))
					{
						mSlopeScaledDepthBias += (float)gameTime.GetElapsedSeconds();
						UpdateDepthBiasState();
					}

					if (mGamePad->IsButtonDown(GamePadButton::Back))
					{
						mSlopeScaledDepthBias -= (float)gameTime.GetElapsedSeconds();
						mSlopeScaledDepthBias = XMMax(mSlopeScaledDepthBias, 0.0f);
						UpdateDepthBiasState();
					}
				}
				else
				{
					if (mGamePad->IsButtonDown(GamePadButton::Start))
					{
						mDepthBias += DepthBiasModulationRate * (float)gameTime.GetElapsedSeconds();
						UpdateDepthBiasState();
					}

					if (mGamePad->IsButtonDown(GamePadButton::Back))
					{
						mDepthBias -= DepthBiasModulationRate * (float)gameTime.GetElapsedSeconds();
						mDepthBias = XMMax(mDepthBias, 0.0f);
						UpdateDepthBiasState();
					}
				}
			}
		}
	}

	void ShadowMappingDemo::UpdateDepthBiasState()
	{
	
		D3D11_RASTERIZER_DESC rasterizerStateDesc;
		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.DepthBias = (int)mDepthBias;
		rasterizerStateDesc.SlopeScaledDepthBias = mSlopeScaledDepthBias;

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateRasterizerState(&rasterizerStateDesc, mDepthBiasState.ReleaseAndGetAddressOf()));
		
	}

	void ShadowMappingDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mAmbientColor.a;

		if (mGamePad != nullptr)
		{
			if (mGamePad->IsButtonHeldDown(GamePadButton::X) && ambientIntensity < UCHAR_MAX)
			{
				ambientIntensity += LightModulationRate * (float)gameTime.GetElapsedSeconds();

				mAmbientColor.a = (UCHAR)XMMin<float>(ambientIntensity, UCHAR_MAX);
			}

			if (mGamePad->IsButtonHeldDown(GamePadButton::Y) && ambientIntensity > 0)
			{
				ambientIntensity -= LightModulationRate * (float)gameTime.GetElapsedSeconds();
				mAmbientColor.a = (UCHAR)XMMax<float>(ambientIntensity, 0);
			}
		}
	}

	void ShadowMappingDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = mSpecularPower;

		if (mGamePad)
		{
			if (mGamePad->IsButtonHeldDown(GamePadButton::A) && specularIntensity < UCHAR_MAX)
			{
				specularIntensity += LightModulationRate * (float)gameTime.GetElapsedSeconds();
				specularIntensity = XMMin<float>(specularIntensity, UCHAR_MAX);

				mSpecularPower = specularIntensity;
			}

			if (mGamePad->IsButtonHeldDown(GamePadButton::B) && specularIntensity > 0)
			{
				specularIntensity -= LightModulationRate * (float)gameTime.GetElapsedSeconds();
				specularIntensity = XMMax<float>(specularIntensity, 0);

				mSpecularPower = specularIntensity;
			}
		}
	}

	void ShadowMappingDemo::UpdatePointLightAndProjector(const GameTime& gameTime)
	{
		static float pointLightIntensity = mPointLight->Color().a;
		float elapsedTime = (float)gameTime.GetElapsedSeconds();

		if (!mGamePad)
			return;
		
		if (mGamePad->IsButtonHeldDown(GamePadButton::RightShoulder) && pointLightIntensity < UCHAR_MAX)
		{
			pointLightIntensity += LightModulationRate * elapsedTime;

			XMCOLOR pointLightLightColor = mPointLight->Color();
			pointLightLightColor.a = (UCHAR)XMMin<float>(pointLightIntensity, UCHAR_MAX);
			mPointLight->SetColor(pointLightLightColor);
		}

		if (mGamePad->IsButtonHeldDown(GamePadButton::LeftShoulder) && pointLightIntensity > 0)
		{
			pointLightIntensity -= LightModulationRate * elapsedTime;

			XMCOLOR pointLightLightColor = mPointLight->Color();
			pointLightLightColor.a = (UCHAR)XMMax<float>(pointLightIntensity, 0.0f);
			mPointLight->SetColor(pointLightLightColor);
		}
		
		
		// Move point light and projector
		XMFLOAT3 movementAmount = Vector3Helper::Zero;
		XMFLOAT3 rotationAmount = Vector3Helper::Zero;

		auto gamePadState = mGamePad->CurrentState();

		if (gamePadState.IsConnected())
		{
			if (gamePadState.IsRightTriggerPressed())
			{
				movementAmount.x = gamePadState.thumbSticks.leftX;
				movementAmount.y = gamePadState.thumbSticks.leftY;
				movementAmount.z = -gamePadState.thumbSticks.rightY;

				XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
				mPointLight->SetPosition(mPointLight->PositionVector() + movement);
				mProxyModel->SetPosition(mPointLight->Position());
				mProjector->SetPosition(mPointLight->Position());
				mRenderableProjectorFrustum->SetPosition(mPointLight->Position());	
			}

			if (gamePadState.IsRightTriggerPressed())
			{
				if (mGamePad->IsButtonHeldDown(GamePadButton::DPadLeft))
				{
					rotationAmount.x += LightRotationRate.x * elapsedTime;
				}
				if (mGamePad->IsButtonHeldDown(GamePadButton::DPadRight))
				{
					rotationAmount.x -= LightRotationRate.x * elapsedTime;
				}
				if (mGamePad->IsButtonHeldDown(GamePadButton::DPadUp))
				{
					rotationAmount.y += LightRotationRate.y * elapsedTime;
				}
				if (mGamePad->IsButtonHeldDown(GamePadButton::DPadDown))
				{
					rotationAmount.y -= LightRotationRate.y * elapsedTime;
				}

				XMMATRIX projectorRotationMatrix = XMMatrixIdentity();
				if (rotationAmount.x != 0)
				{
					projectorRotationMatrix = XMMatrixRotationY(rotationAmount.x);
				}

				if (rotationAmount.y != 0)
				{
					XMMATRIX projectorRotationAxisMatrix = XMMatrixRotationAxis(mProjector->RightVector(), rotationAmount.y);
					projectorRotationMatrix *= projectorRotationAxisMatrix;
				}

				if (rotationAmount.x != Vector2Helper::Zero.x || rotationAmount.y != Vector2Helper::Zero.y)
				{
					mProjector->ApplyRotation(projectorRotationMatrix);
					mRenderableProjectorFrustum->ApplyRotation(projectorRotationMatrix);
				}

			}
		}
	}

	void ShadowMappingDemo::InitializeProjectedTextureScalingMatrix()
	{
		mProjectedTextureScalingMatrix._11 = 0.5f;
		mProjectedTextureScalingMatrix._22 = -0.5f;
		mProjectedTextureScalingMatrix._33 = 1.0f;
		mProjectedTextureScalingMatrix._41 = 0.5f;
		mProjectedTextureScalingMatrix._42 = 0.5f;
		mProjectedTextureScalingMatrix._44 = 1.0f;
	}

	void ShadowMappingDemo::CreateShadowMappingVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const
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

		CreateShadowMappingVertexBuffer(device, &vertices[0], (UINT)vertices.size(), vertexBuffer);
	}

	void ShadowMappingDemo::CreateShadowMappingVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexPositionTextureNormal) * vertexCount);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void ShadowMappingDemo::CreateDepthMapVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh->Vertices();

		std::vector<VertexPosition> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			vertices.push_back(VertexPosition(XMFLOAT4(position.x, position.y, position.z, 1.0f)));
		}

		CreateDepthMapVertexBuffer(device, &vertices[0], (UINT)vertices.size(), vertexBuffer);
	}

	void ShadowMappingDemo::CreateDepthMapVertexBuffer(ID3D11Device* device, VertexPosition* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = (UINT)(sizeof(VertexPosition) * vertexCount);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}
}
#include "pch.h"
#include "RenderableFrustum.h"

namespace Library
{
	RTTI_DEFINITIONS(RenderableFrustum)

	const XMVECTORF32 RenderableFrustum::DefaultColor = ColorHelper::Green;

	const UINT RenderableFrustum::FrustumVertexCount = 8;
	const UINT RenderableFrustum::FrustumPrimitiveCount = 12;
	const UINT RenderableFrustum::FrustumIndicesPerPrimitive = 2;
	const UINT RenderableFrustum::FrustumIndexCount = FrustumPrimitiveCount * FrustumIndicesPerPrimitive;
	const USHORT RenderableFrustum::FrustumIndices[] = {
		// Near plane lines
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		// Sides
		0, 4,
		1, 5,
		2, 6,
		3, 7,

		// Far plane lines
		4, 5,
		5, 6,
		6, 7,
		7, 4
	};

	RenderableFrustum::RenderableFrustum(const Game& game, const std::shared_ptr<Camera>& camera)
		: RenderingGameComponent(game, camera),
		mColor(DefaultColor),
		mPosition(Vector3Helper::Zero), 
		mDirection(Vector3Helper::Forward), 
		mUp(Vector3Helper::Up), 
		mRight(Vector3Helper::Right),
		mWorldMatrix(MatrixHelper::Identity)
	{
	}

	RenderableFrustum::RenderableFrustum(const Game& game, const std::shared_ptr<Camera>& camera, const XMFLOAT4& color)
		: RenderingGameComponent(game, camera),
		mColor(color), 
		mPosition(Vector3Helper::Zero), 
		mDirection(Vector3Helper::Forward), 
		mUp(Vector3Helper::Up), 
		mRight(Vector3Helper::Right),
		mWorldMatrix(MatrixHelper::Identity)
	{
	}

	RenderableFrustum::~RenderableFrustum()
	{
	}

	const XMFLOAT3& RenderableFrustum::Position() const
	{
		return mPosition;
	}

	const XMFLOAT3& RenderableFrustum::Direction() const
	{
		return mDirection;
	}

	const XMFLOAT3& RenderableFrustum::Up() const
	{
		return mUp;
	}

	const XMFLOAT3& RenderableFrustum::Right() const
	{
		return mRight;
	}

	XMVECTOR RenderableFrustum::PositionVector() const
	{
		return XMLoadFloat3(&mPosition);
	}

	XMVECTOR RenderableFrustum::DirectionVector() const
	{
		return XMLoadFloat3(&mDirection);
	}

	XMVECTOR RenderableFrustum::UpVector() const
	{
		return XMLoadFloat3(&mUp);
	}

	XMVECTOR RenderableFrustum::RightVector() const
	{
		return XMLoadFloat3(&mRight);
	}

	void RenderableFrustum::SetPosition(FLOAT x, FLOAT y, FLOAT z)
	{
		XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
		SetPosition(position);
	}

	void RenderableFrustum::SetPosition(FXMVECTOR position)
	{
		XMStoreFloat3(&mPosition, position);
	}

	void RenderableFrustum::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;
	}

	void RenderableFrustum::ApplyRotation(CXMMATRIX transform)
	{
		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR up = XMLoadFloat3(&mUp);

		direction = XMVector3TransformNormal(direction, transform);
		direction = XMVector3Normalize(direction);

		up = XMVector3TransformNormal(up, transform);
		up = XMVector3Normalize(up);

		XMVECTOR right = XMVector3Cross(direction, up);
		up = XMVector3Cross(right, direction);

		XMStoreFloat3(&mDirection, direction);
		XMStoreFloat3(&mUp, up);
		XMStoreFloat3(&mRight, right);
	}

	void RenderableFrustum::ApplyRotation(const XMFLOAT4X4& transform)
	{
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
		ApplyRotation(transformMatrix);
	}

	void RenderableFrustum::InitializeGeometry(const Frustum& frustum)
	{
		InitializeVertexBuffer(frustum);
	}

	void RenderableFrustum::Initialize()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
#endif
		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\FrustumModelVS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "CreateVertexShader() : Failed");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\FrustumModelPS.cso");
		ThrowIfFailed(mGame.GetDirectXDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "CreatePixelShader() : Failed");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame.GetDirectXDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&constantBufferDesc, nullptr, mConstantBuffer.GetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		InitializeIndexBuffer();
	}

	void RenderableFrustum::Update(const GameTime& gameTime)
	{
		XMMATRIX worldMatrix = XMMatrixIdentity();
		MatrixHelper::SetForward(worldMatrix, mDirection);
		MatrixHelper::SetUp(worldMatrix, mUp);
		MatrixHelper::SetRight(worldMatrix, mRight);
		MatrixHelper::SetTranslation(worldMatrix, mPosition);

		XMStoreFloat4x4(&mWorldMatrix, worldMatrix);
	}

	void RenderableFrustum::Render(const GameTime& gameTime)
	{
		
		ID3D11DeviceContext* direct3DDeviceContext = mGame.GetDirectXContext().Get();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPositionColor);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));

		direct3DDeviceContext->UpdateSubresource(mConstantBuffer.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());


		direct3DDeviceContext->DrawIndexed(FrustumIndexCount, 0, 0);
	}

	void RenderableFrustum::InitializeVertexBuffer(const Frustum& frustum)
	{
		
		VertexPositionColor vertices[FrustumVertexCount];
		const XMFLOAT3* corners = frustum.Corners();
		for (UINT i = 0; i < FrustumVertexCount; i++)
		{
			vertices[i].Position = XMFLOAT4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
			vertices[i].Color = mColor;
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionColor) * FrustumVertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.GetAddressOf()));
		
	}

	void RenderableFrustum::InitializeIndexBuffer()
	{
		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(USHORT) * FrustumIndexCount;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = FrustumIndices;
		ThrowIfFailed(mGame.GetDirectXDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, mIndexBuffer.GetAddressOf()));
	}
}
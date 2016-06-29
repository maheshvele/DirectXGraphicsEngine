#pragma once

using namespace DirectX;
namespace Library
{
	inline void ThrowIfFailed(HRESULT hr, const char* const &message = "")
	{
		if (FAILED(hr))
		{
			//Set a breakpoint on this line to catch DirectX API errors
			throw std::exception(message);
			//throw Platform::Exception::CreateException(hr);
		}
	}

	struct VertexPosition
	{
		XMFLOAT4 Position;

		VertexPosition() { }

		VertexPosition(const XMFLOAT4& position)
			: Position(position) { }
	};

	struct VertexPositionColor
	{
		DirectX::XMFLOAT4 Position;

		DirectX::XMFLOAT4 Color;

		VertexPositionColor() {}

		VertexPositionColor(const DirectX::XMFLOAT4& position, const DirectX::XMFLOAT4& color)
			:Position(position), Color(color)
		{
		}
	};

	typedef struct _VertexPositionNormal
    {
        XMFLOAT4 Position;
        XMFLOAT3 Normal;

        _VertexPositionNormal() { }

        _VertexPositionNormal(XMFLOAT4 position, XMFLOAT3 normal)
            : Position(position), Normal(normal) { }
    } VertexPositionNormal;

	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct CBufferPerObject
	{
		XMFLOAT4X4 WorldViewProjection;

		CBufferPerObject() : WorldViewProjection() { }

		CBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
	};


	struct VertexPositionTexture
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;

		VertexPositionTexture() { }

		VertexPositionTexture(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates)
			: Position(position), TextureCoordinates(textureCoordinates) { }
	};

	struct VertexPositionTextureNormal
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;
		XMFLOAT3 Normal;

		VertexPositionTextureNormal() { }

		VertexPositionTextureNormal(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates, const XMFLOAT3& normal)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal) { }
	};

	typedef struct _TextureMappingVertex
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;

		_TextureMappingVertex() { }

		_TextureMappingVertex(XMFLOAT4 position, XMFLOAT2 textureCoordinates)
			: Position(position), TextureCoordinates(textureCoordinates) { }
	} TextureMappingVertex;


	struct WindowSize
	{
		float width;
		float height;
	};
	
	typedef struct _VertexSkinnedPositionTextureNormal
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;
		XMFLOAT3 Normal;
		XMUINT4 BoneIndices;
		XMFLOAT4 BoneWeights;

		_VertexSkinnedPositionTextureNormal() { }

		_VertexSkinnedPositionTextureNormal(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates, const XMFLOAT3& normal, const XMUINT4& boneIndices, const XMFLOAT4& boneWeights)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal), BoneIndices(boneIndices), BoneWeights(boneWeights) { }
	} VertexSkinnedPositionTextureNormal;

}

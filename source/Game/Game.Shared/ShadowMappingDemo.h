#pragma once

using namespace Library;

namespace Library
{
	class PointLight;
	class ProxyModel;
	class Projector;
	class RenderableFrustum;
	class DepthMap;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace RenderGame
{
	enum ShadowMappingTechnique
	{
		ShadowMappingTechniqueSimple = 0,
		ShadowMappingTechniqueManualPCF,
		ShadowMappingTechniquePCF,
		ShadowMappingTechniqueEnd
	};

	const std::string ShadowMappingTechniqueNames[] = { "ShadowMappingSimple", "ShadowMappingManualWithPCF", "ShadowMappingWithPCF" };
	const std::string ShadowMappingDisplayNames[] = { "Shadow Mapping Simple", "Shadow Mapping w/ Manual PCF", "Shadow Mapping w/ PCF" };
	const std::string DepthMappingTechniqueNames[] = { "create_depthmap", "create_depthmap", "create_depthmap_w_bias", };

	class ShadowMappingDemo : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(ShadowMappingDemo, RenderingGameComponent)

	public:

		ShadowMappingDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera);
		~ShadowMappingDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Render(const GameTime& gameTime) override;

	private:

		struct DepthMapVertexBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;

			DepthMapVertexBufferPerObject() : WorldViewProjection() {}

			DepthMapVertexBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) {}
		};

		struct ShadowMappingVertexCBufferPerObject
		{
			XMFLOAT4X4 ProjectiveTextureMatrix;
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			ShadowMappingVertexCBufferPerObject() : WorldViewProjection(), World(), ProjectiveTextureMatrix() { }

			ShadowMappingVertexCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world, const XMFLOAT4X4& projectiveTextureMatrix) : WorldViewProjection(wvp), World(world), ProjectiveTextureMatrix(projectiveTextureMatrix) { }
		};

		struct ShadowMappingVertexCBufferPerFrame
		{
			XMFLOAT3 LightPosition;
			float LightRadius;

			ShadowMappingVertexCBufferPerFrame() : LightPosition(0.0f, 0.0f, 0.0f), LightRadius(10.0f) { }

			ShadowMappingVertexCBufferPerFrame(const XMFLOAT3& lightPosition, float lightRadius)
				: LightPosition(lightPosition), LightRadius(lightRadius)
			{
			}
		};

		struct ShadowMappingPixelCBufferPerObject
		{
			XMFLOAT4 SpecularColor;
			float	 SpecularPower;
			float	 padding[11];

			ShadowMappingPixelCBufferPerObject() : SpecularColor(1.0f, 1.0f, 1.0f, 1.0f), SpecularPower(25.0f) {}

			ShadowMappingPixelCBufferPerObject(const XMFLOAT4& specularColor, float specularPower)
				:SpecularColor(specularColor), SpecularPower(specularPower)
			{

			}
		};

		struct ShadowMappingPixelCBufferPerFrame
		{
			XMFLOAT4	AmbientColor;
			XMFLOAT4	LightColor;
			XMFLOAT3	LightPosition;
			float		Padding;
			XMFLOAT3	CameraPosition;
			float		Padding2;

			XMFLOAT2	ShadowMapSize;
			float		Padding3[14];
			
			ShadowMappingPixelCBufferPerFrame()
				:AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f),
				LightPosition(0.0f, 0.0f, 0.0f), Padding(D3D11_FLOAT32_MAX), CameraPosition(0.0f, 0.0f, 0.0f),
				Padding2(D3D11_FLOAT32_MAX), ShadowMapSize(1.0f, 1.0f)
			{
			}

			ShadowMappingPixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT3& lightPosition, const XMFLOAT3& cameraPosition, const XMFLOAT2& shadowMapSize)
				:AmbientColor(), LightColor(lightColor), LightPosition(lightPosition), CameraPosition(), ShadowMapSize(shadowMapSize)
			{
			}
		};

		ShadowMappingDemo();
		ShadowMappingDemo(const ShadowMappingDemo& rhs);
		ShadowMappingDemo& operator=(const ShadowMappingDemo& rhs);

		void UpdateTechnique();
		void UpdateDepthBias(const GameTime& gameTime);
		void UpdateDepthBiasState();
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLightAndProjector(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);
		void InitializeProjectedTextureScalingMatrix();
		void InitializeShadowMapping();
		void InitializeDepthMap();

		void CreateShadowMappingVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const;
		void CreateShadowMappingVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;

		void CreateDepthMapVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer) const;
		void CreateDepthMapVertexBuffer(ID3D11Device* device, VertexPosition* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;

		static const size_t	Alignment;
		static const float LightModulationRate;
		static const float LightMovementRate;
		static const XMFLOAT2 LightRotationRate;
		static const UINT DepthMapWidth;
		static const UINT DepthMapHeight;
		static const RECT DepthMapDestinationRectangle;
		static const float DepthBiasModulationRate;

		GamePadComponent* mGamePad;
		std::unique_ptr<PointLight> mPointLight;
		std::unique_ptr<ProxyModel> mProxyModel;
		std::unique_ptr<Projector> mProjector;
		std::unique_ptr<RenderableFrustum> mRenderableProjectorFrustum;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		
		Microsoft::WRL::ComPtr<ID3D11InputLayout>		mShadowMappingInputLayout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>		mDepthMapInputLayout;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mPlanePositionVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPlanePositionUVNormalVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPlaneIndexBuffer;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mCheckerboardTexture;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mModelPositionVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mModelPositionUVNormalVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mModelIndexBuffer;
		
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> mDepthBiasState;

		Microsoft::WRL::ComPtr<ID3D11PixelShader>	mShadowMappingPixelShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	mShadowMappingVertexShader;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>  mColorSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>  mShadowMapSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>  mPcfShadowMapSampler;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mShadowMappingVertexCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mShadowMappingVertexCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mShadowMappingPixelCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mShadowMappingPixelCBufferPerObject;
		
		ShadowMappingVertexCBufferPerFrame	mShadowMappingVertexBufferPerFrameData;
		ShadowMappingVertexCBufferPerObject mShadowMappingVertexBufferPerObjectData;
		ShadowMappingPixelCBufferPerFrame	mShadowMappingPixelBufferPerFrameData;
		ShadowMappingPixelCBufferPerObject	mShadowMappingPixelBufferPerObjectData;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>	mDepthMapVertexShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mDepthMapVertexCBufferPerObject;
		DepthMapVertexBufferPerObject mDepthMapVertexBufferPerObjectData;

		Microsoft::WRL::ComPtr<ID3D11ClassLinkage>  mShadowMappingClassLinkage;
		ShadowMappingTechnique	mActiveTechnique;
		Microsoft::WRL::ComPtr<ID3D11ClassInstance>	mShadowMappingInstance[ShadowMappingTechnique::ShadowMappingTechniqueEnd];

		XMCOLOR mAmbientColor;
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		UINT mPlaneVertexCount;
		XMFLOAT4X4 mPlaneWorldMatrix;
	
		RenderStateHelper mRenderStateHelper;
		Frustum mProjectorFrustum;


		UINT mModelIndexCount;
		XMFLOAT4X4 mModelWorldMatrix;
		XMFLOAT4X4 mProjectedTextureScalingMatrix;

		std::unique_ptr<DepthMap> mDepthMap;
		bool mDrawDepthMap;
		
		XMFLOAT2 mTextPosition;
		float mDepthBias;
		float mSlopeScaledDepthBias;	
	};

}
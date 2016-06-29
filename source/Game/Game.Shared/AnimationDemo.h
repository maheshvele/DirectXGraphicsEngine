#pragma once

using namespace Library;

namespace Library
{
	class Keyboard;
	class PointLight;
	class ProxyModel;
	class Effect;
	class SkinnedModelMaterial;
	class Model;
	class AnimationPlayer;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

#define MAX_BONES 60

namespace RenderGame
{
	class AnimationDemo : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(AnimationDemo, RenderingGameComponent)

	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		AnimationDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera);
		~AnimationDemo();

		AnimationDemo() = delete;
		AnimationDemo(const AnimationDemo& rhs) = delete;
		AnimationDemo& operator=(const AnimationDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Render(const GameTime& gameTime) override;

	private:

		struct VertexCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VertexCBufferPerObject() : WorldViewProjection(), World() { }

			VertexCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		struct VertexCBufferPerFrame
		{
			XMFLOAT3 LightPosition;
			float LightRadius;

			VertexCBufferPerFrame() : LightPosition(0.0f, 0.0f, 0.0f), LightRadius(10.0f) { }

			VertexCBufferPerFrame(const XMFLOAT3& lightPosition, float lightRadius)
				: LightPosition(lightPosition), LightRadius(lightRadius)
			{
			}
		};

		struct VertexCBufferSkinning
		{
			XMFLOAT4X4 BoneTransforms[MAX_BONES];
		};

		struct PixelCBufferPerObject
		{
			XMFLOAT4 SpecularColor;
			float SpecularPower;
			float padding[3];

			PixelCBufferPerObject() : SpecularColor(1.0f, 1.0f, 1.0f, 1.0f), SpecularPower(25.0f) { }

			PixelCBufferPerObject(const XMFLOAT4& specularColor, float specularPower)
				: SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}
		};

		__declspec(align(16))
		struct PixelCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;
			XMFLOAT3 LightPosition;
			float Padding;
			XMFLOAT3 CameraPosition;

			PixelCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f),
				LightPosition(0.0f, 0.0f, 0.0f), Padding(D3D11_FLOAT32_MAX), CameraPosition(0.0f, 0.0f, 0.0f)
			{
			}

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT3& lightPosition, const XMFLOAT3& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor),
				LightPosition(lightPosition), CameraPosition(cameraPosition)
			{
			}
		};

		void CreateVertexBuffer(ID3D11Device* device, const std::shared_ptr<Mesh>& mesh, ID3D11Buffer** vertexBuffer);
		void UpdateOptions();
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);
		
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerFrame;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferSkinning;
		VertexCBufferSkinning mVertexCBufferSkinningData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelCBufferPerFrame;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;

		static const size_t Alignment;
		static const float LightModulationRate;
		static const float LightMovementRate;

		//Effect* mEffect;
		//SkinnedModelMaterial* mMaterial;

		GamePadComponent* mGamePad;
		std::unique_ptr<PointLight> mPointLight;
		std::unique_ptr<ProxyModel> mProxyModel;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		std::unique_ptr<Model> mSkinnedModel;
		std::unique_ptr<AnimationPlayer> mAnimationPlayer;

		XMCOLOR mAmbientColor;
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		XMFLOAT4X4 mWorldMatrix;

		std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> mVertexBuffers;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> mIndexBuffers;
		std::vector<UINT> mIndexCounts;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> mColorTextures;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;

		RenderStateHelper mRenderStateHelper;
		
		XMFLOAT2 mTextPosition;
		bool mManualAdvanceMode;
	};
}

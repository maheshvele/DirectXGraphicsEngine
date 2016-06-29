#pragma once

#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class GamePadComponent;
	class FullScreenRenderTarget;
	class FullScreenQuad;
}

namespace RenderGame
{
	class BlinnPhongDemo;

	enum ColorFilter
	{
		ColorFilterGrayScale = 0,
		ColorFilterInverse,
		ColorFilterSepia,
		ColorFilterGeneric,
		ColorFilterEnd
	};

	const std::string ColorFilterDisplayNames[] = { "GrayScale", "Inverse", "Sepia", "Generic" };

	class ColorFilteringDemo : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(ColorFilteringDemo, RenderingGameComponent);

	public:
		ColorFilteringDemo(const Library::Game& game, const std::shared_ptr<Camera>& camera);
		~ColorFilteringDemo();

		ColorFilteringDemo() = delete;
		ColorFilteringDemo(const ColorFilteringDemo& rhs) = delete;
		ColorFilteringDemo& operator=(const ColorFilteringDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Render(const GameTime& gameTime) override;


	private:

		struct GenericFilterBufferPerObject
		{
			XMFLOAT4X4 ColorFilter;

			GenericFilterBufferPerObject() : ColorFilter(MatrixHelper::Identity){ }

			GenericFilterBufferPerObject(const XMFLOAT4X4& colorFilter)
				: ColorFilter(colorFilter)
			{
			}
		};

		GenericFilterBufferPerObject mGenericFilterBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mGenericFilterBufferPerObject;
		void UpdateColorFilterShader();
		void UpdateGenericColorFilter(const GameTime& gameTime);
		
		static const float BrightnessModulationRate;
		static const XMVECTORF32 BackgroundColor;

		std::shared_ptr<BlinnPhongDemo>		 mBlinnPhongDemo;
		std::shared_ptr<FullScreenRenderTarget> mRenderTarget;
		std::shared_ptr<FullScreenQuad>			mFullScreenQuad;
		ColorFilter								mActiveColorFilter;
		XMFLOAT4X4							mGenericColorFilter;

		Microsoft::WRL::ComPtr<ID3D11PixelShader>		mGrayScalePixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		mInverseColorPixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		mSepiaPixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		mGenericFilterPixelShader;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;

		GamePadComponent* mGamePad;
		
	};

}
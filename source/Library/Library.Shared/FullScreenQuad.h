#pragma once

#include <functional>

namespace Library
{


	class FullScreenQuad : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(FullScreenQuad, RenderingGameComponent)

	public:
		FullScreenQuad(const Game& game);
		//FullScreenQuad(Game& game, Material& material);
		~FullScreenQuad();

	//	Material* GetMaterial();
	//	void SetMaterial(Material& material, const std::string& techniqueName, const std::string& passName);
	//	void SetActiveTechnique(const std::string& techniqueName, const std::string& passName);
		void SetCustomUpdateMaterial(std::function<void()> callback);

		virtual void Initialize() override;
		virtual void Render(const GameTime& gameTime) override;

	private:
		FullScreenQuad();
		FullScreenQuad(const FullScreenQuad& rhs);
		FullScreenQuad& operator=(const FullScreenQuad& rhs);

	/*	Material* mMaterial;
		Pass* mPass;*/

		UINT mIndexCount;
		std::function<void()> mCustomUpdateMaterial;
	};
}
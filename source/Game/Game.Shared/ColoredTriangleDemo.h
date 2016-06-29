#pragma once
#include "RenderingGameComponent.h"

using namespace Library;
namespace RenderGame
{
	class Game;
	class ColoredTriangleDemo: public RenderingGameComponent
	{
		RTTI_DECLARATIONS(ColoredTriangleDemo, RenderingGameComponent);

	public:

		ColoredTriangleDemo(const Library::Game& game);
		~ColoredTriangleDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Render(const GameTime& gameTime) override;

	private:

		ColoredTriangleDemo(const ColoredTriangleDemo& rhs);
		ColoredTriangleDemo& operator=(const ColoredTriangleDemo& rhs);

	};
}


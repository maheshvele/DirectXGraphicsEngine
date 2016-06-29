#pragma once
#include "Game.h"

namespace Library
{
	class GameComponent : public RTTI
	{
		RTTI_DECLARATIONS(GameComponent, RTTI);
	public:

		GameComponent(const Game& game);
		virtual ~GameComponent();

		virtual void Initialize();
		virtual void Update(const GameTime& gameTime);
		virtual void Render(const GameTime& gameTime);
		virtual void ReleaseResources();

		bool IsEnabled() const;
		void SetEnabled(bool enabled);
		const Game& GetGame() const;

	protected:
		const Game& mGame;
		bool  bIsEnabled;

	private:
		GameComponent(const GameComponent& rhs);
		GameComponent& operator=(const GameComponent& rhs);
	};
}
#include "pch.h"
#include "GameComponent.h"

namespace Library
{
	RTTI_DEFINITIONS(GameComponent);
	
	GameComponent::GameComponent(const Game& game)
		:mGame(game), bIsEnabled(false)
	{

	}

	GameComponent::~GameComponent()
	{
	}

	bool GameComponent::IsEnabled() const
	{
		return bIsEnabled;
	}

	void GameComponent::SetEnabled(bool enabled)
	{
		bIsEnabled = enabled;
	}

	const Game& GameComponent::GetGame() const
	{
		return mGame;
	}

	void GameComponent::Initialize()
	{

	}

	void GameComponent::Render(const GameTime& gameTime)
	{

	}

	void GameComponent::Update(const GameTime& gameTime)
	{

	}

	void GameComponent::ReleaseResources()
	{
		
	}
}
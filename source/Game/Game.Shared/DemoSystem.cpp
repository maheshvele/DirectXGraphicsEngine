#include "pch.h"
#include "DemoSystem.h"

namespace RenderGame
{
	RTTI_DEFINITIONS(DemoSystem);

	DemoSystem::DemoSystem(const Library::Game& game)
		:GameComponent(game), mDemos(), mActiveDemoIndex(-1), mGamePad(nullptr)
	{
	}

	void DemoSystem::Initialize()
	{
		mGamePad = (GamePadComponent*)mGame.Services().GetService(GamePadComponent::TypeIdClass());
		
		if (mDemos.size() > 0)
		{
			mActiveDemoIndex++;
			mDemos[mActiveDemoIndex]->SetEnabled(true);
		}
		
	}

	void DemoSystem::Update(const GameTime& gameTime)
	{
		if (mGamePad != nullptr)
		{
			auto gamePadState = mGamePad->CurrentState();

			if (gamePadState.IsConnected() && !gamePadState.IsRightTriggerPressed())
			{
				if (mGamePad->WasButtonPressedThisFrame(Library::GamePadButton::DPadRight))
				{
					GoToNextDemo();
				}

				if (mGamePad->WasButtonPressedThisFrame(Library::GamePadButton::DPadLeft))
				{
					GoToPrevDemo();
				}
			}
		}
	}

	void DemoSystem::AddDemo(const std::shared_ptr<Library::RenderingGameComponent>& demo)
	{
		Library::Game& game = const_cast<Library::Game&> (mGame);
		game.AddGameComponent(demo);
		mDemos.push_back(demo);
	}

	void DemoSystem::GoToNextDemo()
	{
		if (mActiveDemoIndex < (std::int32_t)(mDemos.size() - 1))
		{
			mDemos[mActiveDemoIndex]->SetEnabled(false);
			
			mActiveDemoIndex++;

			mDemos[mActiveDemoIndex]->SetEnabled(true);
		}
	}

	std::shared_ptr<RenderingGameComponent> DemoSystem::GetActiveDemo() const
	{
		return mDemos[mActiveDemoIndex];
	}

	void DemoSystem::GoToPrevDemo()
	{
		if (mActiveDemoIndex > 0)
		{
			mDemos[mActiveDemoIndex]->SetEnabled(false);

			mActiveDemoIndex--;

			mDemos[mActiveDemoIndex]->SetEnabled(true);

		}
	}	

	GamePadComponent* DemoSystem::GetGamePad() const
	{
		return mGamePad;
	}

	void DemoSystem::SetGamePad(GamePadComponent* gamePad)
	{
		mGamePad = gamePad;
	}
}
#pragma once

using namespace Library;

namespace RenderGame
{
	class Library::GameTime;
	class Library::GameComponent;
	class Library::RenderingGameComponent;
	class Library::Game;

	class DemoSystem : public Library::GameComponent
	{
		RTTI_DECLARATIONS(DemoSystem, GameComponent);

	public:
		DemoSystem(const Library::Game& game);
		~DemoSystem(){};

		void AddDemo(const std::shared_ptr<Library::RenderingGameComponent>& demo);
	
		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;

		std::shared_ptr<RenderingGameComponent> GetActiveDemo() const;

		GamePadComponent* GetGamePad() const;
		void SetGamePad(GamePadComponent* gamePad);

	private:
		void GoToNextDemo();
		void GoToPrevDemo();

		std::vector<std::shared_ptr<Library::RenderingGameComponent>> mDemos;
		std::int32_t	mActiveDemoIndex;
		GamePadComponent* mGamePad;

		DemoSystem(const DemoSystem& rhs);
		DemoSystem& operator=(const DemoSystem& rhs);
	};
}

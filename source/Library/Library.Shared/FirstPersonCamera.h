#pragma once

#include "PerspectiveCamera.h"
#include <memory>

namespace Library
{
	class GamePadComponent;

	class FirstPersonCamera : public PerspectiveCamera
	{
		RTTI_DECLARATIONS(FirstPersonCamera, PerspectiveCamera)

	public:
		FirstPersonCamera(Game& game);
		FirstPersonCamera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);
		virtual ~FirstPersonCamera() = default;

		FirstPersonCamera(const FirstPersonCamera& rhs) = delete;
		FirstPersonCamera& operator=(const FirstPersonCamera& rhs) = delete;

		GamePadComponent* GetGamePad() const;
		void SetGamePad(GamePadComponent* gamePad);

		float& RotationRate();
		float& MovementRate();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;

		static const float DefaultRotationRate;
		static const float DefaultMovementRate;

	protected:
		GamePadComponent* mGamePad;
		float mRotationRate;
		float mMovementRate;
	};
}


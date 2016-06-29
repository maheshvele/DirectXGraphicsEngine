#pragma once

using namespace Library;
using namespace std;
using namespace DirectX;
namespace RenderGame
{
	class RotatingCubeDemo : public RenderingGameComponent
	{
		RTTI_DECLARATIONS(RotatingCubeDemo, RenderingGameComponent);

	public:
		
		RotatingCubeDemo(const Library::Game& game);
		~RotatingCubeDemo(){};
		
		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Render(const GameTime& gameTime) override;

	private:

		void CreateCubeMesh();
		void Rotate(float radians);

		ModelViewProjectionConstantBuffer	mConstantBufferData;
		unsigned int	mIndexCount;
		// Variables used with the rendering loop.
		float	mDegreesPerSecond;
		bool	bLoadingComplete;
		bool	bTracking;

		RotatingCubeDemo(const RotatingCubeDemo& rhs);
		RotatingCubeDemo& operator=(const RotatingCubeDemo& rhs);

	};
}
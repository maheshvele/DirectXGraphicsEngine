#pragma once

#include <cstdint>
#include <wrl.h>
#include <wrl/client.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1effects_1.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <concrt.h>
#include <collection.h>
#include <ppltasks.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <stack>

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <DirectXPackedVector.h>
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "Utility.h"

#include "ServiceContainer.h"
#include "RenderStateHelper.h"
#include "RasterizerStates.h"
#include "SamplerStates.h"
#include "BlendStates.h"

#include "ColorHelper.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"
#include "GameTime.h"
#include "Common.h"

#include "ServiceContainer.h"

#include "Game.h"
#include "GameComponent.h"
#include "GamePadComponent.h"
#include "RenderingGameComponent.h"

#include "SceneNode.h"
#include "Keyframe.h"
#include "Bone.h"
#include "BoneAnimation.h"
#include "AnimationClip.h"
#include "AnimationPlayer.h"

#include "StreamHelper.h"

#include "Mesh.h"
#include "../Library.Shared/Model.h"
#include "ModelMaterial.h"

#include "Camera.h"
#include "PerspectiveCamera.h"
#include "FirstPersonCamera.h"
#include "OrthographicCamera.h"

#include "FullScreenRenderTarget.h"
#include "FullScreenQuad.h"

#include "Light.h"
#include "PointLight.h"
#include "DirectionalLight.h"

#include "Skybox.h"
#include "ProxyModel.h"
#include "Frustum.h"
#include "RenderableFrustum.h"
#include "Projector.h"
#include "RenderTarget.h"
#include "DepthMap.h"

#include "DemoSystem.h"
#include "ColoredTriangleDemo.h"
#include "RotatingCubeDemo.h"
#include "TexturedSphereDemo.h"
#include "PointLightDemo.h"
#include "BlinnPhongDemo.h"
#include "FogDemo.h"
#include "TransparencyMappingDemo.h"
#include "EnvironmentMappingDemo.h"
#include "ColorFilteringDemo.h"
#include "ShadowMappingDemo.h"
#include "AnimationDemo.h"

#include "RenderingGame.h"

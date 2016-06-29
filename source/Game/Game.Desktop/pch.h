//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0600
#include <SDKDDKVer.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <cstdint>
#include <algorithm>
#include <exception>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>
#include <stack>
#include <locale>
#include <codecvt>

#include <wrl/client.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1.h>
#include <d2d1effects_1.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <dwrite_2.h>
#include <wincodec.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <Shlwapi.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "SpriteBatch.h"
#include "SpriteFont.h"

#include "Utility.h"
#include "GameTime.h"
#include "Common.h"
#include "ColorHelper.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "RTTI.h"

#include "ServiceContainer.h"
#include "RenderStateHelper.h"
#include "RasterizerStates.h"
#include "BlendStates.h"
#include "SamplerStates.h"
#include "GameComponent.h"
#include "GamePadComponent.h"
#include "RenderingGameComponent.h"
#include "Game.h"

#include "FullScreenRenderTarget.h"
#include "FullScreenQuad.h"

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
#include "EnvironmentMappingDemo.h"
#include "TransparencyMappingDemo.h"
#include "ColorFilteringDemo.h"
#include "ShadowMappingDemo.h"
#include "AnimationDemo.h"

#include "RenderingGame.h"





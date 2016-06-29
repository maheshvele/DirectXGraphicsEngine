#pragma once

#include <algorithm>
#include <exception>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <stack>
#include <cstdint>

#include <wrl/client.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1effects_1.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

#include "RTTI.h"
#include "Utility.h"
#include "RasterizerStates.h"

#include "ColorHelper.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"
#include "GameTime.h"
#include "Common.h"

#include "ServiceContainer.h"
#include "GameComponent.h"

#include "RenderingGameComponent.h"
#include "Light.h"
#include "PointLight.h"
#include "RenderTarget.h"



#include "Camera.h"

#include "SceneNode.h"
#include "Keyframe.h"
#include "Bone.h"
#include "BoneAnimation.h"
#include "AnimationClip.h"
#include "AnimationPlayer.h"

#include "StreamHelper.h"

#include "Model.h"
#include "Mesh.h"
#include "ModelMaterial.h"

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include <Shlwapi.h>
#endif

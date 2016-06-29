// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

typedef unsigned int UINT;

#include <stdio.h>
#include <tchar.h>
#include <memory>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <DirectXMath.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "RTTI.h"

#include "StreamHelper.h"

#include "SceneNode.h"
#include "Keyframe.h"
#include "Bone.h"
#include "BoneAnimation.h"
#include "AnimationClip.h"

#include "Model.h"
#include "Mesh.h"
#include "ModelMaterial.h"

// TODO: reference additional headers your program requires here

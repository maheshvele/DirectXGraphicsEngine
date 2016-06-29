#include "pch.h"
#include "ModelMaterial.h"

namespace Library
{
	ModelMaterial::ModelMaterial(Model& model)
		: mModel(model), mTextures()
	{
	}

	ModelMaterial::~ModelMaterial()
	{
	}

	Model& ModelMaterial::GetModel()
	{
		return mModel;
	}

	const std::string& ModelMaterial::Name() const
	{
		return mName;
	}

	const std::map<TextureType, std::shared_ptr<std::vector<std::string>> > ModelMaterial::Textures() const
	{
		return mTextures;
	}
}
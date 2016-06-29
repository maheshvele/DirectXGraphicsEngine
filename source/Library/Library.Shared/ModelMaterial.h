#pragma once

namespace Library
{
	enum TextureType
	{
		TextureTypeDifffuse = 0,
		TextureTypeSpecularMap,
		TextureTypeAmbient,
		TextureTypeEmissive,
		TextureTypeHeightmap,
		TextureTypeNormalMap,
		TextureTypeSpecularPowerMap,
		TextureTypeDisplacementMap,
		TextureTypeLightMap,
		TextureTypeEnd
	};

	class ModelMaterial
	{
		friend class Model;
		friend class OutputStreamHelper;
		friend class InputStreamHelper;

	public:
		ModelMaterial(Model& model);
		~ModelMaterial();

		Model& GetModel();
		const std::string& Name() const;
		const std::map<TextureType, std::shared_ptr<std::vector<std::string>> > Textures() const;

	private:
		
		ModelMaterial(const ModelMaterial& rhs);
		ModelMaterial& operator=(const ModelMaterial& rhs);

		Model& mModel;
		std::string mName;
		std::map<TextureType, std::shared_ptr<std::vector<std::string>> > mTextures;
	};
}
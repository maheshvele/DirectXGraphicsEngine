#pragma once

struct aiMaterial;

namespace ModelPipeline
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

		ModelMaterial(const ModelMaterial& rhs) = delete;
		ModelMaterial& operator=(const ModelMaterial& rhs) = delete;

		Model& GetModel();
		const std::string& Name() const;
		const std::map<TextureType, std::shared_ptr<std::vector<std::string>> > Textures() const;

	private:
		static void InitializeTextureTypeMappings();
		static std::map<TextureType, UINT> sTextureTypeMappings;

		ModelMaterial(Model& model, aiMaterial* material);

		Model& mModel;
		std::string mName;
		std::map<TextureType, std::shared_ptr<std::vector<std::string>> > mTextures;
	};
}
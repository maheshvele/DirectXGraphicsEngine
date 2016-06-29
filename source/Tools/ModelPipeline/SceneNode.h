#pragma once

using namespace DirectX;

namespace ModelPipeline
{
	class SceneNode : public RTTI
	{
		RTTI_DECLARATIONS(SceneNode, RTTI);
		friend class OutputStreamHelper;
		friend class InputStreamHelper;

	public:	
		
		static const XMFLOAT4X4 IdentityMatrix;
		
		const std::string& Name() const;
		SceneNode* GetParentNode();
		std::vector<SceneNode*>& Children();
		const XMFLOAT4X4& Transform() const;
		XMMATRIX TransformMatrix() const;

		void SetParent(SceneNode* parent);

		void SetTransform(XMFLOAT4X4& transform);
		void SetTransform(CXMMATRIX transform);

		SceneNode(const std::string& name);
		SceneNode(const std::string& name, const XMFLOAT4X4& transform);
		SceneNode();

	protected:
		std::string mName;
		SceneNode* mParent;
		std::vector<SceneNode*> mChildren;
		XMFLOAT4X4 mTransform;

	private:
		SceneNode(const SceneNode& rhs);
		SceneNode& operator=(const SceneNode& rhs);
	};
}

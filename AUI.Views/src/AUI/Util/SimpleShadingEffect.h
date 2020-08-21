#pragma once
#include "AUI/GL/Shader.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AStringVector.h"


class API_AUI_VIEWS SimpleShadingEffect: public IShadingEffect {
private:
	GL::Shader mShader;
	
public:
	SimpleShadingEffect(const AStringVector& uniforms, const AString& fragmentCode);

	void draw(const std::function<void()>& callback) override;

	const GL::Shader& getShader() const {
		return mShader;
	}
};

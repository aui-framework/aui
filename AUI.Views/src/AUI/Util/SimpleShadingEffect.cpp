#include "SimpleShadingEffect.h"

#include "AUI/Common/AStringVector.h"

SimpleShadingEffect::SimpleShadingEffect(const AStringVector& uniforms, const AString& fragmentCode)
{
	AString src;
	for (const auto& x : uniforms)
	{
		src += "uniform " + x + ";\n";
	}
	mShader.load(
		"attribute vec3 pos;"
		"void main(void) {gl_Position = vec4(pos, 1);}",
		src +
		"void main() {" + fragmentCode + "}");
	mShader.compile();
}

void SimpleShadingEffect::draw(const std::function<void()>& callback)
{
	mShader.use();
	callback();
}

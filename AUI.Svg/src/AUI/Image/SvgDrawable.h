#pragma once
#include <functional>
#include <AUI/Common/SharedPtrTypes.h>

#include "AUI/Common/ADeque.h"
#include "AUI/Image/IDrawable.h"
#include <nanosvg.h>
#include <nanosvgrast.h>
#include <AUI/GL/Texture.h>


class SvgDrawable: public IDrawable
{
private:
    struct Pair {
        uint64_t key;
        _<GL::Texture> texture;
    };

    NSVGimage* mImage;
    ADeque<Pair> mRasterized;

public:
    explicit SvgDrawable(AByteBuffer& data);
    ~SvgDrawable();

	void draw(const glm::ivec2& size) override;
	glm::ivec2 getSizeHint() override;

	bool isDpiDependent() const override;

};

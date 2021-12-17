#pragma once

#include <AUI/Render/ITexture.h>

class SoftwareTexture: public ITexture {
private:
    _<AImage> mImage;

public:
    void setImage(const _<AImage>& image) override;

    [[nodiscard]] const _<AImage>& getImage() const noexcept {
        return mImage;
    }
};

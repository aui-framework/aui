#pragma once

#include <variant>
#include <AUI/Render/ABorderStyle.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Common/AVector.h>
#include <AUI/Render/ABrush.h>
#include <AUI/Render/APaint.hpp>
#include <glm/glm.hpp>
#include <AUI/Geometry2D/ARect.h>
#include <AUI/Render/IRenderer.h>

class IRendererBackend;

class ADisplayList {
public:
    struct Rectangle {
        glm::vec2 position;
        glm::vec2 size;
    };
    struct RoundedRectangle {
        glm::vec2 position;
        glm::vec2 size;
        float radius;
    };
    struct RectangleBorder {
        glm::vec2 position;
        glm::vec2 size;
        float lineWidth;
    };
    struct RoundedRectangleBorder {
        glm::vec2 position;
        glm::vec2 size;
        float radius;
        int borderWidth;
    };
    struct BoxShadow {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        AColor color;
    };
    struct BoxShadowInner {
        glm::vec2 position;
        glm::vec2 size;
        float blurRadius;
        float spreadRadius;
        float borderRadius;
        AColor color;
        glm::vec2 offset;
    };
    struct Text {
        glm::vec2 position;
        AString text;
        AFontStyle fs;
    };
    struct PrerenderedString {
        glm::vec2 position;
        _<IRenderer::IPrerenderedString> prerenderedString;
    };
    struct Lines {
        AVector<glm::vec2> points;
        ABorderStyle style;
        AMetric width;
    };
    struct LineBatches {
        AVector<std::pair<glm::vec2, glm::vec2>> points;
        ABorderStyle style;
        AMetric width;
    };
    struct Points {
        AVector<glm::vec2> points;
        AMetric size;
    };
    struct SquareSector {
        glm::vec2 position;
        glm::vec2 size;
        AAngleRadians begin;
        AAngleRadians end;
    };
    struct Backdrop {
        glm::ivec2 position;
        glm::ivec2 size;
        AVector<ass::Backdrop::Any> backdrops;
    };

    struct PushLayer {};
    struct PopLayer {};
    struct MaskBefore {};
    struct MaskAfter {};
    struct PopMaskBefore {};
    struct PopMaskAfter {};

    struct StoredCommand {
        using Command = std::variant<Rectangle,
                                     RoundedRectangle,
                                     RectangleBorder,
                                     RoundedRectangleBorder,
                                     BoxShadow,
                                     BoxShadowInner,
                                     Text,
                                     PrerenderedString,
                                     Lines,
                                     LineBatches,
                                     Points,
                                     SquareSector,
                                     Backdrop,
                                     PushLayer,
                                     PopLayer,
                                     MaskBefore,
                                     MaskAfter,
                                     PopMaskBefore,
                                     PopMaskAfter>;

        Command command;
        glm::mat4 transform;
        APaint paint;
        std::uint8_t stencilDepth;
    };

    struct Entity {
        StoredCommand::Command command;
        glm::mat4 transform;
        APaint paint;
        ARect<int> clipRect;
        ARect<float> boundingBox;
        bool isObscured = false;
    };

    void add(StoredCommand::Command cmd, const glm::mat4& transform, APaint paint, std::uint8_t stencilDepth) {
        mCommands << StoredCommand{std::move(cmd), transform, paint, stencilDepth};
    }

    void clear() {
        mCommands.clear();
        mOpaqueRects.clear();
        mEntities.clear();
    }

    void computeOverlaps();
    void optimize();

    void resolveEntities();

    void draw(IRendererBackend& renderer) const;

private:
    AVector<StoredCommand> mCommands;
    AVector<ARect<float>> mOpaqueRects;
    AVector<Entity> mEntities;
};

#ifndef _AXMOL_RIVE_H_
#define _AXMOL_RIVE_H_

#include "axmol/axmol.h"

// Rive includes
#include "rive/renderer.hpp"
#include "rive/factory.hpp"
#include "rive/tess/tess_renderer.hpp"
#include "rive/tess/tess_render_path.hpp"
#include "rive/tess/contour_stroke.hpp" // Added

#include <vector>

namespace rive {
    class RawPath;
}

class AxmolRenderPath : public rive::TessRenderPath {
public:
    AxmolRenderPath(rive::RawPath& rawPath, rive::FillRule fillRule);
    ~AxmolRenderPath() override = default;

    // Called by TessRenderPath::triangulate
    void addTriangles(rive::Span<const rive::Vec2D> vertices, rive::Span<const uint16_t> indices) override;
    void setTriangulatedBounds(const rive::AABB& value) override;
    
    void rewind() override; // Override rewind to clear cache
    
    // Clears old geometry and shifts new geometry to front
    void prune(size_t oldVertexCount, size_t oldIndexCount);

    // Friend to allow renderer to call protected contour()
    friend class AxmolRenderer;

    // Local untransformed vertices (Cached for Fills)
    std::vector<rive::Vec2D> _rawVertices;
    std::vector<uint16_t> _rawIndices;
};

class AxmolRenderShader : public rive::RenderShader {
public:
    virtual ~AxmolRenderShader() = default;
    virtual ax::Color32 getColor(float x, float y) const = 0;
};

class AxmolLinearGradient : public AxmolRenderShader {
public:
    AxmolLinearGradient(float sx, float sy, float ex, float ey,
                        const rive::ColorInt colors[], const float stops[], size_t count);
    
    ax::Color32 getColor(float x, float y) const override;
    
private:
    rive::Vec2D _start;
    rive::Vec2D _end;
    rive::Vec2D _diff; // end - start
    float _lenSq;
    std::vector<rive::ColorInt> _colors;
    std::vector<float> _stops;
};

class AxmolRadialGradient : public AxmolRenderShader {
public:
    AxmolRadialGradient(float cx, float cy, float radius,
                        const rive::ColorInt colors[], const float stops[], size_t count);
    
    ax::Color32 getColor(float x, float y) const override;

private:
    rive::Vec2D _center;
    float _radius;
    std::vector<rive::ColorInt> _colors;
    std::vector<float> _stops;
};

class AxmolRenderPaint : public rive::RenderPaint {
public:
    AxmolRenderPaint();
    
    void style(rive::RenderPaintStyle style) override;
    void color(rive::ColorInt value) override;
    void thickness(float value) override;
    void join(rive::StrokeJoin value) override;
    void cap(rive::StrokeCap value) override;
    void blendMode(rive::BlendMode value) override;
    void shader(rive::rcp<rive::RenderShader>) override;
    void invalidateStroke() override;

    rive::RenderPaintStyle _style = rive::RenderPaintStyle::fill;
    rive::ColorInt _color = 0xFFFFFFFF;
    float _thickness = 1.0f;
    rive::StrokeJoin _join = rive::StrokeJoin::miter;
    rive::StrokeCap _cap = rive::StrokeCap::butt;
    rive::BlendMode _blendMode = rive::BlendMode::srcOver;
    rive::rcp<AxmolRenderShader> _shader; // Store the shader
};

#include <stack>

// ... existing classes ...

struct AxmolState {
    int clipDepth = 0;
};

class AxmolRenderer : public rive::TessRenderer {
public:
    AxmolRenderer(ax::Node* rootNode);
    
    void drawPath(rive::RenderPath* path, rive::RenderPaint* paint) override;
    void clipPath(rive::RenderPath* path) override;
    void save() override;
    void restore() override;
    
    // Stub implementations for pure virtuals in TessRenderer/Renderer
    void orthographicProjection(float left, float right, float bottom, float top, float near, float far) override {}
    
    // Call at start of frame
    void startFrame();
    
    // Images - Stub for now
    void drawImage(const rive::RenderImage*, rive::ImageSampler, rive::BlendMode, float opacity) override {}
    void drawImageMesh(const rive::RenderImage*, rive::ImageSampler, rive::rcp<rive::RenderBuffer>, rive::rcp<rive::RenderBuffer>, rive::rcp<rive::RenderBuffer>, uint32_t, uint32_t, rive::BlendMode, float) override {}

private:
    ax::DrawNode* _drawNode = nullptr; // Current draw node
    ax::Node* _rootNode = nullptr;
    rive::ContourStroke _stroke;
    
    std::stack<ax::Node*> _containerStack;
    std::stack<AxmolState> _stateStack;
    int _clipDepth = 0;
    
    void updateDrawNode();
};

class AxmolFactory : public rive::Factory {
public:
    rive::rcp<rive::RenderBuffer> makeRenderBuffer(rive::RenderBufferType, rive::RenderBufferFlags, size_t sizeInBytes) override;
    
    rive::rcp<rive::RenderShader> makeLinearGradient(
        float sx, float sy, float ex, float ey,
        const rive::ColorInt colors[], const float stops[], size_t count) override;
    
    rive::rcp<rive::RenderShader> makeRadialGradient(
        float cx, float cy, float radius,
        const rive::ColorInt colors[], const float stops[], size_t count) override;
        
    rive::rcp<rive::RenderPath> makeRenderPath(rive::RawPath& rawPath, rive::FillRule fillRule) override;
    rive::rcp<rive::RenderPath> makeEmptyRenderPath() override;
    rive::rcp<rive::RenderPaint> makeRenderPaint() override;
    rive::rcp<rive::RenderImage> decodeImage(rive::Span<const uint8_t>) override;
};

#endif // _AXMOL_RIVE_H_

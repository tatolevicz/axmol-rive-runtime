#include "AxmolRive.h"

#include <algorithm> // For std::min, std::max, std::lower_bound

// Helper to convert Rive ColorInt to Axmol Color32
static ax::Color32 toAxColor(rive::ColorInt color) {
    unsigned int r = rive::colorRed(color);
    unsigned int g = rive::colorGreen(color);
    unsigned int b = rive::colorBlue(color);
    unsigned int a = rive::colorAlpha(color);
    return ax::Color32(r, g, b, a);
}

// AxmolLinearGradient Implementation
AxmolLinearGradient::AxmolLinearGradient(float sx, float sy, float ex, float ey,
                                         const rive::ColorInt colors[], const float stops[], size_t count) 
    : _start(sx, sy), _end(ex, ey) {
    _diff = _end - _start;
    _lenSq = _diff.lengthSquared();
    for (size_t i = 0; i < count; ++i) {
        _colors.push_back(colors[i]);
        _stops.push_back(stops[i]);
    }
}

ax::Color32 AxmolLinearGradient::getColor(float x, float y) const {
    if (_colors.empty()) return ax::Color32::WHITE;
    if (_lenSq <= 0.0001f) return toAxColor(_colors[0]);

    rive::Vec2D p(x, y);
    rive::Vec2D v = p - _start;
    float t = rive::Vec2D::dot(v, _diff) / _lenSq;
    t = std::max(0.0f, std::min(1.0f, t));

    // Find stop
    // Assuming sorted stops (Rive usually does). If not, we should sort.
    // Simple scan
    if (t <= _stops.front()) return toAxColor(_colors.front());
    if (t >= _stops.back()) return toAxColor(_colors.back());

    for (size_t i = 0; i < _stops.size() - 1; ++i) {
        if (t >= _stops[i] && t <= _stops[i+1]) {
            float range = _stops[i+1] - _stops[i];
            float localT = (t - _stops[i]) / (range > 0 ? range : 1.0f);
            return toAxColor(rive::colorLerp(_colors[i], _colors[i+1], localT));
        }
    }
    return toAxColor(_colors.back());
}

// AxmolRadialGradient Implementation
AxmolRadialGradient::AxmolRadialGradient(float cx, float cy, float radius,
                                         const rive::ColorInt colors[], const float stops[], size_t count)
    : _center(cx, cy), _radius(radius) {
    for (size_t i = 0; i < count; ++i) {
        _colors.push_back(colors[i]);
        _stops.push_back(stops[i]);
    }
}

ax::Color32 AxmolRadialGradient::getColor(float x, float y) const {
    if (_colors.empty()) return ax::Color32::WHITE;
    if (_radius <= 0.0001f) return toAxColor(_colors[0]);

    rive::Vec2D p(x, y);
    float dist = rive::Vec2D::distance(p, _center);
    float t = dist / _radius;
    t = std::max(0.0f, std::min(1.0f, t));

    if (t <= _stops.front()) return toAxColor(_colors.front());
    if (t >= _stops.back()) return toAxColor(_colors.back());

    for (size_t i = 0; i < _stops.size() - 1; ++i) {
        if (t >= _stops[i] && t <= _stops[i+1]) {
            float range = _stops[i+1] - _stops[i];
            float localT = (t - _stops[i]) / (range > 0 ? range : 1.0f);
            return toAxColor(rive::colorLerp(_colors[i], _colors[i+1], localT));
        }
    }
    return toAxColor(_colors.back());
}

// AxmolRenderPath Implementation
AxmolRenderPath::AxmolRenderPath(rive::RawPath& rawPath, rive::FillRule fillRule)
    : rive::TessRenderPath(rawPath, fillRule) {
}

void AxmolRenderPath::rewind() {
    rive::TessRenderPath::rewind();
    _rawVertices.clear();
    _rawIndices.clear();
}

void AxmolRenderPath::addTriangles(rive::Span<const rive::Vec2D> vertices, rive::Span<const uint16_t> indices) {
    // Copy vertices and indices to storage
    size_t baseIndex = _rawVertices.size();
    
    for (const auto& v : vertices) {
        _rawVertices.push_back(v);
    }
    
    for (const auto& idx : indices) {
        _rawIndices.push_back(static_cast<unsigned short>(baseIndex + idx));
    }
}

void AxmolRenderPath::setTriangulatedBounds(const rive::AABB& value) {
    // Optional: Use this to optimize culling
}

// AxmolRenderPaint Implementation
AxmolRenderPaint::AxmolRenderPaint() {}
void AxmolRenderPaint::style(rive::RenderPaintStyle style) { _style = style; }
void AxmolRenderPaint::color(rive::ColorInt value) { _color = value; }
void AxmolRenderPaint::thickness(float value) { _thickness = value; }
void AxmolRenderPaint::join(rive::StrokeJoin value) { _join = value; }
void AxmolRenderPaint::cap(rive::StrokeCap value) { _cap = value; }
void AxmolRenderPaint::blendMode(rive::BlendMode value) { _blendMode = value; }
void AxmolRenderPaint::shader(rive::rcp<rive::RenderShader> shader) { 
    _shader = rive::static_rcp_cast<AxmolRenderShader>(shader); 
}
void AxmolRenderPaint::invalidateStroke() { /* Handle invalidation if caching */ }

// AxmolRenderer Implementation
AxmolRenderer::AxmolRenderer(ax::DrawNode* drawNode) : _drawNode(drawNode) {}

void AxmolRenderer::clipPath(rive::RenderPath* path) {
    // Axmol DrawNode doesn't support arbitrary clipping easily.
    // We can use Stencil Buffer in Axmol, but Renderer::clipPath implies a stack.
    // TessRenderer handles the stack logic and provides m_Stack.
    // But actually applying the clip to the DrawNode is hard.
    // For now, we IGNORE clipping or use scissor test if it's a rect.
    // Rive clipping can be complex paths.
    // To support this properly, we'd need to use ax::ClippingNode or custom stencil commands.
    // Given we are using a single DrawNode, clipping is difficult.
    // MVP: Ignore clipping.
}

void AxmolRenderer::drawPath(rive::RenderPath* path, rive::RenderPaint* paint) {
    auto axPath = static_cast<AxmolRenderPath*>(path);
    auto axPaint = static_cast<AxmolRenderPaint*>(paint);
    
    // Prepare color
    ax::Color32 c;
    bool hasShader = (axPaint->_shader != nullptr);
    if (!hasShader) {
        c = toAxColor(axPaint->_color);
    }
    const auto& m = transform();

    if (axPaint->_style == rive::RenderPaintStyle::stroke) {
        // Stroke Logic
        // We don't use _rawVertices for strokes, we use the _stroke helper directly.
        static rive::Mat2D identity; // Stroke generation happens in local space too usually?
        // Wait, TessRenderPath::extrudeStroke takes a transform.
        // If we pass 'm' (world transform), the vertices in _stroke are World Space.
        // If we pass identity, they are Local.
        // Rive usually extrudes in Local space for better quality? Or World?
        // TessRenderPath::extrudeStroke documentation says: "transform is the transform to apply to the path before stroking".
        // If we want consistent stroke width in screen pixels, we usually pass the full transform.
        // But if stroke is scaling, maybe we want that.
        // For now, let's pass 'm' and assume _stroke vertices are World Space.
        
        _stroke.reset();
        axPath->extrudeStroke(&_stroke, axPaint->_join, axPaint->_cap, axPaint->_thickness, m);
        
        const auto& strip = _stroke.triangleStrip();
        if (strip.size() >= 3) {
            for (size_t i = 0; i < strip.size() - 2; ++i) {
                const auto& v1 = strip[i];
                const auto& v2 = strip[i+1];
                const auto& v3 = strip[i+2];
                
                // v1, v2, v3 are already transformed (if extrudeStroke used 'm')
                // Let's check extrudeStroke implementation again.
                // It calls contour(transform). So yes, they are transformed.
                
                if (hasShader) {
                    float cx = (v1.x + v2.x + v3.x) / 3.0f;
                    float cy = (v1.y + v2.y + v3.y) / 3.0f;
                    c = axPaint->_shader->getColor(cx, cy);
                }
                
                // Triangles in strip alternate winding, but DrawNode is double-sided usually?
                // If culling is enabled, we need to swap.
                // Assuming no culling for now.
                
                _drawNode->drawTriangle(
                    ax::Vec2(v1.x, v1.y),
                    ax::Vec2(v2.x, v2.y),
                    ax::Vec2(v3.x, v3.y),
                    c
                );
            }
        }
    } else {
        // Fill Logic
        // Update cache if needed
        axPath->contour(transform()); // Ensure contour is updated (triangulate logic does this but we might need to force it for transform?)
        // Actually, triangulate() handles dirty checks.
        // But we need to be careful about single path 'identity' reset logic in triangulate.
        
        axPath->triangulate(axPath); // This populates _rawVertices with LOCAL coords (if single path) or RELATIVE coords (if container)
        
        // Iterate and draw, applying transform 'm'
        if (axPath->_rawVertices.empty() || axPath->_rawIndices.empty()) {
            return;
        }
        
        for (size_t i = 0; i < axPath->_rawIndices.size(); i += 3) {
            if (i + 2 >= axPath->_rawIndices.size()) break;
            
            unsigned short i1 = axPath->_rawIndices[i];
            unsigned short i2 = axPath->_rawIndices[i+1];
            unsigned short i3 = axPath->_rawIndices[i+2];
            
            // Transform on the fly!
            rive::Vec2D v1 = m * axPath->_rawVertices[i1];
            rive::Vec2D v2 = m * axPath->_rawVertices[i2];
            rive::Vec2D v3 = m * axPath->_rawVertices[i3];
            
            if (hasShader) {
                float cx = (v1.x + v2.x + v3.x) / 3.0f;
                float cy = (v1.y + v2.y + v3.y) / 3.0f;
                c = axPaint->_shader->getColor(cx, cy);
            }
            
            _drawNode->drawTriangle(
                ax::Vec2(v1.x, v1.y),
                ax::Vec2(v2.x, v2.y),
                ax::Vec2(v3.x, v3.y),
                c
            );
        }
    }
}

// AxmolFactory Implementation
rive::rcp<rive::RenderBuffer> AxmolFactory::makeRenderBuffer(rive::RenderBufferType, rive::RenderBufferFlags, size_t sizeInBytes) {
    return nullptr;
}

rive::rcp<rive::RenderShader> AxmolFactory::makeLinearGradient(
    float sx, float sy, float ex, float ey,
    const rive::ColorInt colors[], const float stops[], size_t count) {
    return rive::make_rcp<AxmolLinearGradient>(sx, sy, ex, ey, colors, stops, count);
}

rive::rcp<rive::RenderShader> AxmolFactory::makeRadialGradient(
    float cx, float cy, float radius,
    const rive::ColorInt colors[], const float stops[], size_t count) {
    return rive::make_rcp<AxmolRadialGradient>(cx, cy, radius, colors, stops, count);
}

rive::rcp<rive::RenderPath> AxmolFactory::makeRenderPath(rive::RawPath& rawPath, rive::FillRule fillRule) {
    return rive::make_rcp<AxmolRenderPath>(rawPath, fillRule);
}

rive::rcp<rive::RenderPath> AxmolFactory::makeEmptyRenderPath() {
    rive::RawPath emptyPath;
    return rive::make_rcp<AxmolRenderPath>(emptyPath, rive::FillRule::nonZero);
}

rive::rcp<rive::RenderPaint> AxmolFactory::makeRenderPaint() {
    return rive::make_rcp<AxmolRenderPaint>();
}

rive::rcp<rive::RenderImage> AxmolFactory::decodeImage(rive::Span<const uint8_t>) {
    return nullptr;
}

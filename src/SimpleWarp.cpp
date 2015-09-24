//
//  SimpleWarp.cpp
//  SimpleWarping
//
//  Created by Adrià Navarro López on 8/27/15.
//
//

#include "cinder/Rand.h"
#include "SimpleWarp.h"

bool SimpleWarp::bEditing = false;

void SimpleWarp::setup(gl::Fbo::Format fmt)
{
    mSize = app::getWindowSize();
    
    reset();
    setupShader();
    setupFbo(fmt);
    setupCallbacks(fmt);
    calculate();
}

void SimpleWarp::setupShader()
{
    mShader = gl::GlslProg::create(gl::GlslProg::Format()
       .vertex(CI_GLSL(150,
                       uniform mat4 ciModelViewProjection;
                       in vec4 ciPosition;
                       in vec2 ciTexCoord0;
                       out highp vec2 TexCoord;
                 
                       void main() {
                           TexCoord = ciTexCoord0;
                           gl_Position = ciModelViewProjection * ciPosition;
                       })
               )
       .fragment(CI_GLSL(150,
                         uniform sampler2D uTex0;
                         uniform mat3 matrix;
                         uniform vec2 texSize;
                         in vec2 TexCoord;
                         out vec4 oColor;
                         
                         void main() {
                             vec2 coord = TexCoord * texSize;
                             vec3 warp = vec3(coord, 1.0) * matrix;
                             coord = warp.xy / warp.z;
                                 
                             oColor = texture(uTex0, coord/texSize);
                             
                             vec2 clampedCoord = clamp(coord, vec2(0.0), texSize);
                             oColor.a *= smoothstep(2.0, 0.0, distance(coord, clampedCoord));
                         })
                 ));
    
    mShader->uniform("uTex0", 0);
    
    auto r = geom::Rect(Rectf(vec2(0), vec2(1))).texCoords(vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1));
    mBatch = gl::Batch::create(r, mShader);
}

void SimpleWarp::setupFbo(gl::Fbo::Format fmt)
{
    mFbo = gl::Fbo::create(mSize.x, mSize.y, fmt);
}

void SimpleWarp::setupCallbacks(gl::Fbo::Format fmt)
{
    app::WindowRef w = app::getWindow();
    
    w->getSignalResize().connect([this, fmt]() {
        mSize = app::getWindowSize();
        setupFbo(fmt);
        calculate();
    });
    
    w->getSignalMouseMove().connect([this](app::MouseEvent &ev) {
        if (!bEditing) return;
        float dist = 10.0e6f;
        for( unsigned i = 0; i < 4; i++ ) {
            float d = glm::distance( vec2(ev.getPos()), mDst[i] * mSize );
            if( d < dist ) {
                dist = d;
                mSelected = i;
            }
        }
    });
    
    w->getSignalMouseDown().connect([this](app::MouseEvent &ev) {
        if (!bEditing) return;
        mOffset = vec2(ev.getPos()) - mDst[mSelected] * mSize;
        ev.setHandled(true);
    });
    
    w->getSignalMouseDrag().connect([this](app::MouseEvent &ev) {
        if (!bEditing) return;
        mDst[mSelected] = (vec2(ev.getPos()) - mOffset) / mSize;
        calculate();
        ev.setHandled(true);
    });
}

void SimpleWarp::reset()
{
    mSrc[0] = mDst[0] = vec2(0, 0);
    mSrc[1] = mDst[1] = vec2(1, 0);
    mSrc[2] = mDst[2] = vec2(1, 1);
    mSrc[3] = mDst[3] = vec2(0, 1);
    calculate();
}

void SimpleWarp::begin()
{
    mFbo->bindFramebuffer();
    gl::pushMatrices();
    gl::pushViewport();
    gl::viewport(ivec2(0), mFbo->getSize());
    gl::setMatricesWindow(mFbo->getSize());
}

void SimpleWarp::end()
{
    gl::popMatrices();
    gl::popViewport();
    mFbo->unbindFramebuffer();
}

void SimpleWarp::draw()
{
    mShader->uniform("texSize", mSize);
    mShader->uniform("matrix", inverse(mTransform));
    
    // draw warped tex
    {
        gl::ScopedTextureBind texBind(mFbo->getColorTexture(), 0);
        gl::ScopedMatrices m;
        gl::scale(mSize);
        mBatch->draw();
    }
    
    // draw interface
    if (bEditing) {
        gl::ScopedColor dotColor(1, 1, 0);
        for (int i = 0; i < 4; i++) {
            float scale = i == mSelected ? 0.9 + 0.2 * sin(5.0f * app::getElapsedSeconds()): 1.0f;
            gl::drawSolidCircle(mDst[i] * mSize, 10.0f * scale, 4);
            gl::drawSolidCircle(mDst[i] * mSize, 10.0f * scale, 4);
        }
        gl::ScopedColor lineColor(1, 1, 1);
        gl::drawLine(mDst[0] * mSize, mDst[1] * mSize);
        gl::drawLine(mDst[1] * mSize, mDst[2] * mSize);
        gl::drawLine(mDst[2] * mSize, mDst[3] * mSize);
        gl::drawLine(mDst[3] * mSize, mDst[0] * mSize);
        gl::drawLine(mDst[0] * mSize, mDst[2] * mSize);
        gl::drawLine(mDst[1] * mSize, mDst[3] * mSize);
    }
}

void SimpleWarp::calculate()
{
    mTransform = getQuadToQuad(mSrc[0] * mSize, mSrc[1] * mSize, mSrc[2] * mSize, mSrc[3] * mSize,
                               mDst[0] * mSize, mDst[1] * mSize, mDst[2] * mSize, mDst[3] * mSize);
}

glm::vec2 SimpleWarp::transform(const vec2 &p)
{
    vec3 warp = vec3(p, 1.0) * mTransform;
    return vec2(warp) / warp.z;
}

glm::mat3x3 SimpleWarp::getQuadToQuad(const vec2 &from0, const vec2 &from1, const vec2 &from2, const vec2 &from3,
                                 const vec2 &to0, const vec2 &to1, const vec2 &to2, const vec2 &to3)
{
    glm::mat3x3 a = getSquareToQuad(from0, from1, from2, from3);
    glm::mat3x3 b = getSquareToQuad(to0, to1, to2, to3);
    return glm::inverse(a) * b;
}

glm::mat3x3 SimpleWarp::getSquareToQuad(const vec2 &p0, const vec2 &p1, const vec2 &p2, const vec2 &p3)
{
    glm::mat3x3 m;
    
    vec2 d3 = p0 - p1 + p2 - p3;
    
    m[2][2] = 1.0F;
    
    if (d3 == vec2(0)) { // to do: use tolerance
        m[0][0] = p1.x - p0.x;
        m[0][1] = p2.x - p1.x;
        m[0][2] = p0.x;
        m[1][0] = p1.y - p0.y;
        m[1][1] = p2.y - p1.y;
        m[1][2] = p0.y;
        m[2][0] = 0.0F;
        m[2][1] = 0.0F;
    }
    else {
        vec2 d1 = p1 - p2;
        vec2 d2 = p3 - p2;
        
        double invdet = 1.0F / (d1.x * d2.y - d2.x * d1.y);
        m[2][0] = (d3.x * d2.y - d2.x * d3.y) * invdet;
        m[2][1] = (d1.x * d3.y - d3.x * d1.y) * invdet;
        m[0][0] = p1.x - p0.x + m[2][0] * p1.x;
        m[0][1] = p3.x - p0.x + m[2][1] * p3.x;
        m[0][2] = p0.x;
        m[1][0] = p1.y - p0.y + m[2][0] * p1.y;
        m[1][1] = p3.y - p0.y + m[2][1] * p3.y;
        m[1][2] = p0.y;
    }
    
    return m;
}

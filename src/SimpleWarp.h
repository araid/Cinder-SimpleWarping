//
//  SimpleWarp.h
//
//  Created by Adrià Navarro López on 8/27/15.
//
//

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#pragma once

using namespace ci;

class SimpleWarp {
public:
    static bool	isEditing()                         { return bEditing; }
    static void	edit( bool enabled = true )         { bEditing = enabled; }
    static void	toggleEdit( bool enabled = true )   { edit(!bEditing); }
    
    void setup(gl::Fbo::Format fmt = gl::Fbo::Format());
    void reset();
    void begin();
    void end();
    void draw();
    
    vec2 transform(const vec2 &p);

protected:
    void calculate();
    void setupShader();
    void setupFbo(gl::Fbo::Format fmt);
    void setupCallbacks(gl::Fbo::Format fmt);
    
    glm::mat3x3 getSquareToQuad(const vec2 &p0, const vec2 &p1, const vec2 &p2, const vec2 &p3);
    glm::mat3x3 getQuadToQuad(const vec2 &from0, const vec2 &from1, const vec2 &from2, const vec2 &from3,
                              const vec2 &to0, const vec2 &to1, const vec2 &to2, const vec2 &to3);
    
    static bool     bEditing;
    unsigned int    mSelected;
    vec2            mOffset;
    
    vec2            mSize;
    vec2            mSrc[4];
    vec2            mDst[4];
    glm::mat3x3     mTransform;
    
    gl::FboRef      mFbo;
    gl::BatchRef    mBatch;
    gl::GlslProgRef mShader;
};


#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "SimpleWarp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SimpleWarpingApp : public App {
public:
    void setup() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    
    SimpleWarp mWarp;
};

void SimpleWarpingApp::setup()
{
    mWarp.setup();
}

void SimpleWarpingApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    mWarp.begin();
    {
        // render your scene
        gl::clear(Color::gray(0.5));
        gl::ScopedDepth depth(true);
        gl::ScopedGlslProg shader(gl::getStockShader(gl::ShaderDef().lambert()));
        gl::ScopedMatrices mat;
        gl::setMatricesWindowPersp(getWindowSize());
        
        for (float x = 0; x < getWindowWidth(); x += 50) {
            for (float y = 0; y < getWindowHeight(); y += 50) {
                gl::ScopedMatrices m;
                gl::translate(x, y);
                gl::rotate(getElapsedSeconds() * 0.5, x / getWindowWidth(), y / getWindowHeight(), 0);
                gl::drawCube(vec3(0), vec3(45));
            }
        }
    }
    mWarp.end();
    mWarp.draw();
}

void SimpleWarpingApp::keyDown(KeyEvent event)
{
    switch(event.getCode()) {
        case KeyEvent::KEY_w:
            mWarp.toggleEdit();
            break;
        case KeyEvent::KEY_r:
            if(mWarp.isEditing()) mWarp.reset();
            break;
    }
}


CINDER_APP( SimpleWarpingApp, RendererGl )

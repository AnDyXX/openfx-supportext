/*
   OFX generic position interact.

   Copyright (C) 2014 INRIA

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of the {organization} nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   INRIA
   Domaine de Voluceau
   Rocquencourt - B.P. 105
   78153 Le Chesnay Cedex - France
 */
#ifndef __Misc__ofxsPositionInteract__
#define __Misc__ofxsPositionInteract__

#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <ofxsInteract.h>
#include <ofxsImageEffect.h>
#include "ofxsOGLTextRenderer.h"

namespace OFX {
/// template for a generic position interact.
/*
   The PositionInteractParam class must define a static name() function, returning the OFX parameter name.
   (using const char* directly as template parameter is not reliable) :
   namespace {
   struct MyPositionInteractParam {
     static const char *name() { return kMyName; }
   };
   }

   // the describe() function should include the declaration of the interact:
   desc.setOverlayInteractDescriptor(new PositionOverlayDescriptor<MyPositionInteractParam>);

   // The position param should be defined is describeInContext() as follows:
   Double2DParamDescriptor* position = desc.defineDouble2DParam(kMyName);
   position->setLabels(kMyLabel, kMyLabel, kMyLabel);
   position->setHint(kMyHint);
   position->setDoubleType(eDoubleTypeXYAbsolute);
   position->setDefaultCoordinateSystem(eCoordinatesNormalised);
   position->setDefault(0.5, 0.5);
   page->addChild(*position);
 */
template<typename PositionInteractParam>
class PositionInteract
    : public OFX::OverlayInteract
{
public:
    PositionInteract(OfxInteractHandle handle,
                     OFX::ImageEffect* effect)
        : OFX::OverlayInteract(handle)
          , _state(eMouseStateInactive)
    {
        _position = effect->fetchDouble2DParam( PositionInteractParam::name() );
        assert(_position);
        _penPosition.x = _penPosition.y = 0;
    }

private:
    // overridden functions from OFX::Interact to do things
    virtual bool draw(const OFX::DrawArgs &args);
    virtual bool penMotion(const OFX::PenArgs &args);
    virtual bool penDown(const OFX::PenArgs &args);
    virtual bool penUp(const OFX::PenArgs &args);

private:
    enum MouseStateEnum
    {
        eMouseStateInactive,
        eMouseStatePoised,
        eMouseStatePicked
    };

    MouseStateEnum _state;
    OFX::Double2DParam* _position;
    OfxPointD _penPosition;

    double pointSize() const
    {
        return 5;
    }

    double pointTolerance() const
    {
        return 6;
    }

    // round to the closest int, 1/10 int, etc
    // this make parameter editing easier
    // pscale is args.pixelScale.x / args.renderScale.x;
    // pscale10 is the power of 10 below pscale
    inline double fround(double val,
                         double pscale)
    {
        double pscale10 = std::pow( 10.,std::floor( std::log10(pscale) ) );

        return pscale10 * std::floor(val / pscale10 + 0.5);
    }
};

template <typename ParamName>
bool
PositionInteract<ParamName>::draw(const OFX::DrawArgs &args)
{
    OfxPointD pscale;

    pscale.x = args.pixelScale.x / args.renderScale.x;
    pscale.y = args.pixelScale.y / args.renderScale.y;

    OfxRGBColourF col;
    switch (_state) {
    case eMouseStateInactive:
        col.r = col.g = col.b = 0.8f; break;
    case eMouseStatePoised:
        col.r = 0.; col.g = 1.0; col.b = 0.0f; break;
    case eMouseStatePicked:
        col.r = 0.; col.g = 1.0; col.b = 0.0f; break;
    }

    OfxPointD pos;
    if (_state == eMouseStatePicked) {
        pos = _penPosition;
    } else {
        _position->getValueAtTime(args.time, pos.x, pos.y);
    }
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPointSize( pointSize() );

    glPushMatrix();
    // Draw everything twice
    // l = 0: shadow
    // l = 1: drawing
    for (int l = 0; l < 2; ++l) {
        if (l == 0) {
            // Draw a shadow for the cross hair
            // shift by (1,1) pixel
            glTranslated(pscale.x, -pscale.y, 0);
        }
        glColor3f(col.r * l, col.g * l, col.b * l);
        glBegin(GL_POINTS);
        glVertex2d(pos.x, pos.y);
        glEnd();
        OFX::TextRenderer::bitmapString( pos.x, pos.y, ParamName::name() );
        if (l == 0) {
            glTranslated(-pscale.x, pscale.y, 0);
        }
    }
    glPopMatrix();

    glPopAttrib();

    return true;
} // draw

// overridden functions from OFX::Interact to do things
template <typename ParamName>
bool
PositionInteract<ParamName>::penMotion(const OFX::PenArgs &args)
{
    OfxPointD pscale;

    pscale.x = args.pixelScale.x / args.renderScale.x;
    pscale.y = args.pixelScale.y / args.renderScale.y;
    OfxPointD pos;
    _position->getValueAtTime(args.time, pos.x, pos.y);

    // pen position is in cannonical coords
    OfxPointD penPos = args.penPosition;

    switch (_state) {
    case eMouseStateInactive:
    case eMouseStatePoised: {
        // are we in the box, become 'poised'
        MouseStateEnum newState;
        if ( ( std::fabs(penPos.x - pos.x) <= pointTolerance() * pscale.x) &&
             ( std::fabs(penPos.y - pos.y) <= pointTolerance() * pscale.y) ) {
            newState = eMouseStatePoised;
        } else   {
            newState = eMouseStateInactive;
        }

        if (_state != newState) {
            _state = newState;
            _effect->redrawOverlays();
        }
    }
    break;

    case eMouseStatePicked: {
        _penPosition = args.penPosition;
        _effect->redrawOverlays();
    }
    break;
    }

    return _state != eMouseStateInactive;
}

template <typename ParamName>
bool
PositionInteract<ParamName>::penDown(const OFX::PenArgs &args)
{
    if (!_position) {
        return false;
    }
    penMotion(args);
    if (_state == eMouseStatePoised) {
        _state = eMouseStatePicked;
        _penPosition = args.penPosition;
        _effect->redrawOverlays();
    }

    return _state == eMouseStatePicked;
}

template <typename ParamName>
bool
PositionInteract<ParamName>::penUp(const OFX::PenArgs &args)
{
    if (!_position) {
        return false;
    }
    if (_state == eMouseStatePicked) {
        OfxPointD pscale;
        pscale.x = args.pixelScale.x / args.renderScale.x;
        pscale.y = args.pixelScale.y / args.renderScale.y;
        _position->setValue( fround(_penPosition.x, pscale.x), fround(_penPosition.y, pscale.y) );
        _state = eMouseStatePoised;
        penMotion(args);
        _effect->redrawOverlays();

        return true;
    }

    return false;
}

template <typename ParamName>
class PositionOverlayDescriptor
    : public OFX::DefaultEffectOverlayDescriptor<PositionOverlayDescriptor<ParamName>, PositionInteract<ParamName> >
{
};
} // namespace OFX

#endif /* defined(__Misc__ofxsPositionInteract__) */

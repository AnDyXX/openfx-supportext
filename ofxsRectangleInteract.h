/*
   OFX generic rectangle interact with 4 corner points + center point and 4 mid-points.
   You can use it to define any rectangle in an image resizable by the user.

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
#ifndef __Misc__ofxsRectangleInteract__
#define __Misc__ofxsRectangleInteract__

#include <ofxsInteract.h>
#include <ofxsImageEffect.h>
#include "ofxsMacros.h"

#define kParamRectangleInteractBtmLeft "bottomLeft"
#define kParamRectangleInteractBtmLeftLabel "Bottom Left"
#define kParamRectangleInteractBtmLeftHint "Coordinates of the bottom left corner of the rectangle"

#define kParamRectangleInteractSize "size"
#define kParamRectangleInteractSizeLabel "Size"
#define kParamRectangleInteractSizeHint "Width and height of the rectangle"
#define kParamRectangleInteractSizeDim1 "width"
#define kParamRectangleInteractSizeDim2 "height"

#define kParamRectangleInteractInteractive "interactive"
#define kParamRectangleInteractInteractiveLabel "Interactive Update"
#define kParamRectangleInteractInteractiveHint "If checked, update the parameter values during interaction with the image viewer, else update the values when pen is released."

namespace OFX {
/**
 * @brief In order to work the plug-in using this interact must have 2 parameters named after
 * the defines above.
 *
 **/
class RectangleInteract
    : public OFX::OverlayInteract
{
public:
    enum MouseStateEnum
    {
        eMouseStateIdle = 0,
        eMouseStateDraggingTopLeft,
        eMouseStateDraggingTopRight,
        eMouseStateDraggingBtmLeft,
        eMouseStateDraggingBtmRight,
        eMouseStateDraggingCenter,
        eMouseStateDraggingTopMid,
        eMouseStateDraggingMidRight,
        eMouseStateDraggingBtmMid,
        eMouseStateDraggingMidLeft
    };

    enum DrawStateEnum
    {
        eDrawStateInactive = 0,
        eDrawStateHoveringTopLeft,
        eDrawStateHoveringTopRight,
        eDrawStateHoveringBtmLeft,
        eDrawStateHoveringBtmRight,
        eDrawStateHoveringCenter,
        eDrawStateHoveringTopMid,
        eDrawStateHoveringMidRight,
        eDrawStateHoveringBtmMid,
        eDrawStateHoveringMidLeft
    };

public:

    RectangleInteract(OfxInteractHandle handle,
                      OFX::ImageEffect* effect)
        : OFX::OverlayInteract(handle)
          , _effect(effect)
          , _lastMousePos()
          , _mouseState(eMouseStateIdle)
          , _drawState(eDrawStateInactive)
          , _modifierStateCtrl(0)
          , _modifierStateShift(0)
          , _btmLeft(0)
          , _size(0)
    {
        _btmLeft = effect->fetchDouble2DParam(kParamRectangleInteractBtmLeft);
        _size = effect->fetchDouble2DParam(kParamRectangleInteractSize);
        addParamToSlaveTo(_btmLeft);
        addParamToSlaveTo(_size);
        assert(_btmLeft && _size);
        _interactive = effect->fetchBooleanParam(kParamRectangleInteractInteractive);
        _btmLeftDragPos.x = _btmLeftDragPos.y = 0;
        _sizeDrag.x = _sizeDrag.y = 0;
        _interactiveDrag = false;
    }

    // overridden functions from OFX::Interact to do things
    virtual bool draw(const OFX::DrawArgs &args) OVERRIDE;
    virtual bool penMotion(const OFX::PenArgs &args) OVERRIDE;
    virtual bool penDown(const OFX::PenArgs &args) OVERRIDE;
    virtual bool penUp(const OFX::PenArgs &args) OVERRIDE;
    virtual bool keyDown(const OFX::KeyArgs &args) OVERRIDE;
    virtual bool keyUp(const OFX::KeyArgs & args) OVERRIDE;
    virtual void loseFocus(const FocusArgs &args) OVERRIDE;

protected:


    /**
     * @brief This method returns the bottom left point. The base implementation just returns the value
     * of the _btmLeft parameter at the given time.
     * One could override this function to  do more complex stuff based on other parameters state like the Crop plug-in does.
     **/
    virtual OfxPointD getBtmLeft(OfxTime time) const;

    /**
     * @brief This is called right before any call to allowXXX is made.
     * This way you can query values of a parameter and store it away without having to do this
     * at every allowXXX call.
     **/
    virtual void aboutToCheckInteractivity(OfxTime /*time*/)
    {
    }

    /**
     * @brif These can be overriden to disallow interaction with a point.
     **/
    virtual bool allowTopLeftInteraction() const
    {
        return true;
    }

    virtual bool allowTopRightInteraction() const
    {
        return true;
    }

    virtual bool allowBtmRightInteraction() const
    {
        return true;
    }

    virtual bool allowBtmLeftInteraction() const
    {
        return true;
    }

    virtual bool allowTopMidInteraction() const
    {
        return true;
    }

    virtual bool allowMidRightInteraction() const
    {
        return true;
    }

    virtual bool allowBtmMidInteraction() const
    {
        return true;
    }

    virtual bool allowMidLeftInteraction() const
    {
        return true;
    }

    virtual bool allowCenterInteraction() const
    {
        return true;
    }

private:
    void setValue(OfxPointD btmLeft, OfxPointD size, const OfxPointD &pscale);

private:
    OFX::ImageEffect* _effect;
    OfxPointD _lastMousePos;
    MouseStateEnum _mouseState;
    DrawStateEnum _drawState;
    int _modifierStateCtrl;
    int _modifierStateShift;
    OFX::Double2DParam* _btmLeft;
    OFX::Double2DParam* _size;
    OFX::BooleanParam* _interactive;
    OfxPointD _btmLeftDragPos;
    OfxPointD _sizeDrag;
    bool _interactiveDrag;
};

class RectangleOverlayDescriptor
    : public OFX::DefaultEffectOverlayDescriptor<RectangleOverlayDescriptor, RectangleInteract>
{
};
} // namespace OFX

#endif /* defined(__Misc__ofxsRectangleInteract__) */

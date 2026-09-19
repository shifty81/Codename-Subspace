#include "studio/StudioGizmoOverlay.h"
#include "studio/StudioMeasurementFormat.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#endif
namespace subspace {
#ifdef _WIN32
namespace {
constexpr float kPi=3.14159265358979323846f;
void Color(int axis,float alpha=1.0f){
    const float rgba[3][3]={{.97f,.31f,.33f},{.33f,.89f,.47f},{.40f,.62f,1.0f}};
    const float* c=rgba[std::clamp(axis,0,2)];glColor4f(c[0],c[1],c[2],alpha);
}
void Circle(float x,float y,float radius){
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<32;++i){const float a=2*kPi*i/32.0f;glVertex2f(x+radius*std::cos(a),y+radius*std::sin(a));}
    glEnd();
}
void Line(float x0,float y0,float x1,float y1){glBegin(GL_LINES);glVertex2f(x0,y0);glVertex2f(x1,y1);glEnd();}
void Letter(int axis,float x,float y){
    glLineWidth(2.0f);
    if(axis==0){Line(x-4,y-4,x+4,y+4);Line(x+4,y-4,x-4,y+4);}
    if(axis==1){Line(x-4,y-4,x,y);Line(x+4,y-4,x,y);Line(x,y,x,y+5);}
    if(axis==2){Line(x-4,y-4,x+4,y-4);Line(x+4,y-4,x-4,y+4);Line(x-4,y+4,x+4,y+4);}
}
// Seven-segment angular readout; no font or second UI renderer dependency.
void Digit(char digit,float x,float y){
    static constexpr unsigned char bits[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
    unsigned char mask=0;if(digit>='0'&&digit<='9')mask=bits[digit-'0'];
    if(digit=='.'){
        glPointSize(3.0f);glBegin(GL_POINTS);glVertex2f(x+3,y+14);glEnd();glPointSize(1.0f);
        return;
    }
    if(digit=='-'){Line(x+1,y+7,x+6,y+7);return;}
    if(digit=='+'){Line(x+1,y+7,x+6,y+7);Line(x+3.5f,y+4,x+3.5f,y+10);return;}
    const float v[7][4]={{1,0,6,0},{7,1,7,6},{7,8,7,13},{1,14,6,14},{0,8,0,13},{0,1,0,6},{1,7,6,7}};
    glBegin(GL_LINES);for(int k=0;k<7;++k)if(mask&(1u<<k)){
        glVertex2f(x+v[k][0],y+v[k][1]);glVertex2f(x+v[k][2],y+v[k][3]);}
    glEnd();
}
// Small fixed-stroke labels keep the readout independent of Windows font/GDI
// handles and preserve the OpenGL context's existing ownership.
void Glyph(char ch,float x,float y){
    switch(ch){
        case 'P':Line(x,y,x,y+13);Line(x,y,x+7,y);Line(x+7,y,x+7,y+6);Line(x+7,y+6,x,y+6);break;
        case 'R':Line(x,y,x,y+13);Line(x,y,x+7,y);Line(x+7,y,x+7,y+6);
                 Line(x+7,y+6,x,y+6);Line(x,y+6,x+8,y+13);break;
        case 'S':Line(x+7,y,x,y);Line(x,y,x,y+6);Line(x,y+6,x+7,y+6);
                 Line(x+7,y+6,x+7,y+13);Line(x+7,y+13,x,y+13);break;
        case 'D':Line(x,y,x,y+13);Line(x,y,x+5,y);Line(x+5,y,x+8,y+3);
                 Line(x+8,y+3,x+8,y+10);Line(x+8,y+10,x+5,y+13);
                 Line(x+5,y+13,x,y+13);break;
        case 'O':Line(x,y,x+8,y);Line(x+8,y,x+8,y+13);Line(x+8,y+13,x,y+13);Line(x,y+13,x,y);break;
        case 'T':Line(x,y,x+8,y);Line(x+4,y,x+4,y+13);break;
        case 'C':Line(x+8,y,x,y);Line(x,y,x,y+13);Line(x,y+13,x+8,y+13);break;
        case 'L':Line(x,y,x,y+13);Line(x,y+13,x+8,y+13);break;
        case 'I':Line(x,y,x+8,y);Line(x+4,y,x+4,y+13);Line(x,y+13,x+8,y+13);break;
        case 'M':Line(x,y+13,x,y);Line(x,y,x+4,y+6);Line(x+4,y+6,x+8,y);Line(x+8,y,x+8,y+13);break;
        default:break;
    }
}
// One fixed-stroke text authority for the small Studio-only HUD. This is a
// transitional presentation until the shared editor typography backend owns
// these readouts; avoid inventing a second font or persistent UI state.
void Label(const char* text,float x,float y){
    for(;*text;++text,x+=10.0f)Glyph(*text,x,y);
}
void DimensionLetter(int axis,float x,float y){
    if(axis==0){ // W = width
        Line(x-4,y-5,x-2,y+5);Line(x-2,y+5,x,y);Line(x,y,x+2,y+5);Line(x+2,y+5,x+4,y-5);
    }else if(axis==1){ // L = length
        Line(x-4,y-5,x-4,y+5);Line(x-4,y+5,x+4,y+5);
    }else{ // H = height
        Line(x-4,y-5,x-4,y+5);Line(x+4,y-5,x+4,y+5);Line(x-4,y,x+4,y);
    }
}
void DrawNumber(float x,float y,float value,int decimals,bool degrees=false,bool percent=false,bool meters=false){
    const auto text=StudioMeasurementFormat::Compact(value,decimals);
    glLineWidth(1.6f);
    for(const auto ch:text){Digit(ch,x,y);x+=ch=='.'?5.0f:9.0f;}
    if(degrees)Circle(x+2,y+2,2.1f);
    if(percent){ // % marker in the same tiny stroke vocabulary.
        Circle(x+2,y+3,1.3f);Circle(x+8,y+11,1.3f);Line(x+1,y+13,x+9,y+1);
    }
    if(meters)Glyph('M',x+1,y+1);
}
void TransformHud(const StudioGizmoSnapshot& snapshot){
    const float availableWidth=snapshot.viewportRight-snapshot.viewportLeft;
    const float availableHeight=snapshot.viewportBottom-snapshot.viewportTop;
    if(availableWidth<455.0f||availableHeight<145.0f)return;
    const float x=snapshot.viewportLeft+9.0f,y=snapshot.viewportTop+9.0f;
    glColor4f(.012f,.023f,.038f,.83f);
    glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+435,y);
       glVertex2f(x+435,y+106);glVertex2f(x,y+106);glEnd();
    const float fieldStarts[3]={x+57.0f,x+181.0f,x+305.0f};
    const std::array<std::array<float,3>,4> rows={{snapshot.readout.position,
        snapshot.readout.rotationDegrees,snapshot.readout.scalePercent,
        snapshot.readout.nominalLocalMeters}};
    for(int row=0;row<4;++row){
        const float lineY=y+10+row*23.5f;
        if(row==3&&!snapshot.readout.nominalDimensionsAvailable)continue;
        const auto kind=static_cast<StudioMeasurementKind>(row);
        glColor4f(.76f,.86f,.94f,.98f);glLineWidth(1.6f);
        Label(StudioMeasurementFormat::Label(kind),x+8,lineY);
        for(int axis=0;axis<3;++axis){
            const float start=fieldStarts[axis];
            Color(axis,.94f);
            if(kind==StudioMeasurementKind::NominalDimensions)
                DimensionLetter(axis,start,lineY+7); // W/L/H, not XYZ.
            else Letter(axis,start,lineY+7);
            glColor4f(.93f,.96f,1.0f,1.0f);
            DrawNumber(start+13,lineY,rows[static_cast<std::size_t>(row)][static_cast<std::size_t>(axis)],
                       row==1||row==2?1:2,StudioMeasurementFormat::IsDegrees(kind),
                       StudioMeasurementFormat::IsPercent(kind),StudioMeasurementFormat::IsMeters(kind));
        }
    }
    // DIM is NOMINAL catalog-space W/L/H in meters. It is not mesh-exact,
    // a rotated world AABB, a Boolean result, or a promise of interior volume.
}
void AngleGauge(float x,float y,float value,int axis){
    Color(axis,.93f);glLineWidth(2.2f);
    const float sweep=std::clamp(value,-180.0f,180.0f);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=96;++i){const float a=(-90.0f+sweep*static_cast<float>(i)/96.0f)*kPi/180.0f;
        glVertex2f(x+46*std::cos(a),y+46*std::sin(a));}
    glEnd();
    glLineWidth(1.0f);glColor4f(.80f,.84f,.90f,.56f);
    for(int degrees=-180;degrees<=180;degrees+=15){
        const float a=(-90.0f+static_cast<float>(degrees))*kPi/180.0f;
        const float r=(degrees%45==0)?53.0f:50.0f;
        Line(x+46*std::cos(a),y+46*std::sin(a),x+r*std::cos(a),y+r*std::sin(a));
    }
    Color(axis);const float a=(-90.0f+sweep)*kPi/180.0f;
    Circle(x+46*std::cos(a),y+46*std::sin(a),3.0f);
    const int rounded=static_cast<int>(std::round(value));
    const std::string label=(rounded>=0?"+":"")+std::to_string(rounded);
    float offset=0;glLineWidth(1.7f);
    for(char digit:label){Digit(digit,x+64+offset,y-8);offset+=10;}
    Circle(x+64+offset+3,y-6,2.4f); // degrees glyph
}
}
#endif
void StudioGizmoOverlay::Draw(const StudioGizmoSnapshot& snapshot,int width,int height,
                              StudioAxis active,StudioAxis hovered,bool rotating,float angleDegrees){
#ifdef _WIN32
    if((!snapshot.visible&&!snapshot.readoutVisible)||width<=0||height<=0)return;
    GLint matrixMode=GL_MODELVIEW;glGetIntegerv(GL_MATRIX_MODE,&matrixMode);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();glOrtho(0,width,height,0,-1,1);
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
    glDisable(GL_LIGHTING);glDisable(GL_DEPTH_TEST);glDisable(GL_TEXTURE_2D);glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);
    glScissor(static_cast<GLint>(snapshot.viewportLeft),
              static_cast<GLint>(height-snapshot.viewportBottom),
              static_cast<GLsizei>(snapshot.viewportRight-snapshot.viewportLeft),
              static_cast<GLsizei>(snapshot.viewportBottom-snapshot.viewportTop));
    if(snapshot.readoutVisible)TransformHud(snapshot);
    for(const auto& h:snapshot.handles){
        if(!h.valid)continue;
        const bool selected=active==h.axis||hovered==h.axis;
        Color(static_cast<int>(h.axis),selected?1.0f:.86f);
        glLineWidth(selected?4.0f:2.6f);
        Line(h.center.x,h.center.y,h.tip.x,h.tip.y);
        Circle(h.tip.x,h.tip.y,selected?10.0f:8.0f);
        Letter(static_cast<int>(h.axis),h.tip.x,h.tip.y);
    }
    if(rotating&&active!=StudioAxis::None){
        if(const auto* h=snapshot.Handle(active);h&&h->valid)
            AngleGauge(h->center.x,h->center.y,angleDegrees,static_cast<int>(active));
    }
    glMatrixMode(GL_MODELVIEW);glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();
    glMatrixMode(matrixMode);glPopAttrib();
#else
    (void)snapshot;(void)width;(void)height;(void)active;(void)hovered;(void)rotating;(void)angleDegrees;
#endif
}
} // namespace subspace

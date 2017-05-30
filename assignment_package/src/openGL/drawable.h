#pragma once

#include <openGL/glwidget277.h>
#include <la.h>

#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <scene/transform.h>
#include <pcg32.h>

// This defines an abstract class which can be rendered by our shader program.
// Make any geometry a subclass of Drawable in order to render it with the ShaderProgram class.
class Drawable
{
public:
    Drawable();
    ~Drawable();

    void recreate();
    virtual void create() = 0;  //This is a purely virtual function; it must be implemented by each subclass of Drawable.
                                //It has no base implementation. It is analogous to an abstract function in Java.

    virtual void destroy();     //This is a virtual function; it MAY be overridden by a subclass of Drawable, but it has a base implementation.
    virtual GLenum drawMode() const;

    int elemCount();
    bool bindIdx();
    bool bindPos();
    bool bindNor();
    bool bindCol();

    Transform transform;
    static pcg32 colorRNG; // Used for setting VBO color data for GL visualizer

protected:
    int count;
    //The vertex buffer objects every object needs to be drawn.
    //If you create your own Drawable subclasses, you can add more VBO types in that class's header file.
    QOpenGLBuffer bufIdx;
    QOpenGLBuffer bufPos;
    QOpenGLBuffer bufNor;
    QOpenGLBuffer bufCol;
};

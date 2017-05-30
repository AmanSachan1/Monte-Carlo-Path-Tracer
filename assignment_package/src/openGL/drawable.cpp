#include <openGL/drawable.h>
#include <QDateTime>

pcg32 Drawable::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());

Drawable::Drawable()
    : bufIdx(QOpenGLBuffer::IndexBuffer),
      bufPos(QOpenGLBuffer::VertexBuffer),
      bufNor(QOpenGLBuffer::VertexBuffer),
      bufCol(QOpenGLBuffer::VertexBuffer),
      transform()
{}

Drawable::~Drawable()
{
    destroy();
}

void Drawable::recreate()
{
    destroy();
    create();
}

void Drawable::destroy()
{
    bufIdx.destroy();
    bufPos.destroy();
    bufNor.destroy();
    bufCol.destroy();
}

GLenum Drawable::drawMode() const{return GL_TRIANGLES;}
int Drawable::elemCount(){return count;}
bool Drawable::bindIdx(){return bufIdx.bind();}
bool Drawable::bindPos(){return bufPos.bind();}
bool Drawable::bindNor(){return bufNor.bind();}
bool Drawable::bindCol(){return bufCol.bind();}

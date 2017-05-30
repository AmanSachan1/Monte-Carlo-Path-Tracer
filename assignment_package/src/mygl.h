#pragma once

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QRubberBand>
#include <QMouseEvent>

#include <openGL/glwidget277.h>
#include <la.h>
#include <openGL/shaderprogram.h>
#include <scene/camera.h>
#include <scene/scene.h>
#include <la.h>
#include <scene/jsonreader.h>
#include <integrators/integrator.h>
#include <scene/bvh.h>

#include <QTimer>
#include <QTime>
#include <QSound>

class QOpenGLTexture;

enum IntegratorType
{
    FULL_LIGHTING,
    PHOTON_MAP_LIGHTING,
    PARTICIPATING_MEDIA_LIGHTING,
    DIRECT_LIGHTING,
    NAIVE_LIGHTING
};

class MyGL
    : public GLWidget277
{
    Q_OBJECT
private:
    QOpenGLVertexArrayObject vao;

    ShaderProgram prog_lambert;
    ShaderProgram prog_flat;

    Camera gl_camera;//This is a camera we can move around the scene to view it from any angle.
                     //However, the camera defined in the config file is the one from which the scene will be rendered.
                     //If you move gl_camera, you will be able to see the viewing frustum of the scene's camera.

    Scene scene;
    Sampler* sampler;
    int recursionLimit;
    JSONReader json_reader;
    IntegratorType integratorType;

    QString output_filepath;

    QTimer render_event_timer;

    QSound completeSFX;

    QTime renderTimer;

    bool makeBVH;
    int maxBVHPrims;

public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void SceneLoadDialog();
    void GLDrawScene();
    void ResizeToSceneCamera();

    void RenderScene();
    void GLDrawProgressiveView();

    void completeRender();

private:
    QRubberBand *rubberBand;
    bool move_rubberband;
    QPoint rubberband_offset;
    QPoint origin;
    bool progressive_render = true;
    bool something_rendered = false;

    QOpenGLShaderProgram prog_progressive;
    int prog_progressive_attribute_position;
    int prog_progressive_attribute_texcoord;
    bool is_rendering = false;
    GLuint progressive_position_buffer;
    QOpenGLTexture* progressive_texture = nullptr;

protected:
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void reorderRect();


public slots:
    void slot_SetNumSamplesSqrt(int);
    void slot_SetRecursionLimit(int);
    void slot_SetProgressiveRender(bool);
    void slot_SetIntegratorType(int);
    void slot_UseBVH(bool);
    void slot_SetMaxBVHPrims(int);

signals:
    void sig_ResizeToCamera(int,int);
    void sig_DisableGUI(bool);

private slots:
     void onRenderUpdate();
};

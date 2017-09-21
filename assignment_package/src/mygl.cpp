#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QXmlStreamReader>
#include <QFileDialog>
#include <QThreadPool>
#include <QOpenGLTexture>
#include <integrators/directlightingintegrator.h>
#include <integrators/naiveintegrator.h>
#include <integrators/fulllightingintegrator.h>
#include <integrators/participatingmediaintegrator.h>
#include <integrators/photonmappingintegrator.h>
#include <scene/lights/diffusearealight.h>
#include <QDateTime>

constexpr float screen_quad_pos[] = {
    1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
};

MyGL::MyGL(QWidget *parent)
    : GLWidget277(parent),
      sampler(new Sampler(100, 0)),
      integratorType(FULL_LIGHTING),
      recursionLimit(5),
      completeSFX(":/include/complete.wav")
{
    setFocusPolicy(Qt::ClickFocus);
    render_event_timer.setParent(this);
    connect(&render_event_timer, SIGNAL(timeout()), this, SLOT(onRenderUpdate()));
    move_rubberband = false;
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(0,0,0,0);
    rubberBand->show();
    origin = QPoint(0, 0);
    rubberband_offset = QPoint(0, 0);
    something_rendered = false;
    makeBVH = true;
    maxBVHPrims = 1;
}

void MyGL::mousePressEvent(QMouseEvent *e)
{
    origin = e->pos();
    if (!rubberBand)
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    else rubberBand->hide();
    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
}

void MyGL::mouseMoveEvent(QMouseEvent *e)
{
    rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
}

void MyGL::mouseReleaseEvent(QMouseEvent *e)
{
    rubberband_offset = e->pos();
    reorderRect();
}

void MyGL::reorderRect()
{
    int tmp_originx = (std::min(origin.x(), rubberband_offset.x()));
    int tmp_originy = (std::min(origin.y(), rubberband_offset.y()));

    int tmp_offsetx = (std::max(origin.x(), rubberband_offset.x()));
    int tmp_offsety = (std::max(origin.y(), rubberband_offset.y()));

    origin.setX(std::max(tmp_originx, 0));
    origin.setY(std::max(tmp_originy,0));
    rubberband_offset.setX(std::min(tmp_offsetx, (int)scene.camera.width));
    rubberband_offset.setY(std::min(tmp_offsety, (int)scene.camera.height));
}

MyGL::~MyGL()
{
    makeCurrent();

    vao.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // Enable backface culling
    //    glEnable(GL_CULL_FACE);
    //    glCullFace(GL_BACK);
    //    glFrontFace(GL_CCW);

    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    vao.create();

    // Create and set up the diffuse shader
    prog_lambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat-color shader
    prog_flat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    vao.bind();

    // Create shader program for progressive render view
    prog_progressive.addShaderFromSourceFile(QOpenGLShader::Vertex  , ":/glsl/renderview.vert.glsl");
    prog_progressive.addShaderFromSourceFile(QOpenGLShader::Fragment,  ":/glsl/renderview.frag.glsl");
    prog_progressive_attribute_position = 0;
    prog_progressive_attribute_texcoord = 1;
    prog_progressive.bindAttributeLocation("position", prog_progressive_attribute_position);
    prog_progressive.bindAttributeLocation("uv", prog_progressive_attribute_texcoord);
    prog_progressive.link();
    // create full screen quad for progressive rendering
    glGenBuffers(1, &progressive_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, progressive_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad_pos), screen_quad_pos, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Test scene data initialization
    scene.CreateTestScene();
    ResizeToSceneCamera();
    gl_camera.CopyAttributes(scene.camera);
    rubberband_offset.setX(scene.camera.width);
    rubberband_offset.setY(scene.camera.height);
}

void MyGL::resizeGL(int w, int h)
{
    gl_camera = Camera(w, h);

    glm::mat4 viewproj = gl_camera.GetViewProj();

    // Upload the projection matrix
    prog_lambert.setViewProjMatrix(viewproj);
    prog_flat.setViewProjMatrix(viewproj);

    printGLErrorLog();
    gl_camera.CopyAttributes(scene.camera);
}

// This function is called by Qt any time your GL window is supposed to update
// For example, when the function updateGL is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    if (progressive_render && (is_rendering || something_rendered))
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        GLDrawProgressiveView();
    }
    else
    {    // Clear the screen so that we only see newly drawn images
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update the viewproj matrix
        prog_lambert.setViewProjMatrix(gl_camera.GetViewProj());
        prog_flat.setViewProjMatrix(gl_camera.GetViewProj());
        GLDrawScene();
    }
}

void MyGL::GLDrawScene()
{
    for(std::shared_ptr<Drawable> d : scene.drawables)
    {
        if(d->drawMode() == GL_TRIANGLES)
        {
            prog_lambert.setModelMatrix(d->transform.T());
            prog_lambert.draw(*this, *d);
        }
        else if(d->drawMode() == GL_LINES)
        {
            prog_flat.setModelMatrix(d->transform.T());
            prog_flat.draw(*this, *d);
        }
    }

    prog_flat.setModelMatrix(glm::mat4(1.0f));
    prog_flat.draw(*this, scene.camera);
}

void MyGL::ResizeToSceneCamera()
{
    this->setFixedWidth(scene.camera.width);
    this->setFixedHeight(scene.camera.height);
    rubberband_offset.setX(scene.camera.width);
    rubberband_offset.setY(scene.camera.height);
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    bool invalid_key = false;

    // http://doc.qt.io/qt-5/qt.html#Key-enum
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        gl_camera.RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        gl_camera.RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        gl_camera.RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        gl_camera.RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        gl_camera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        gl_camera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        gl_camera.TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        gl_camera.TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        gl_camera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        gl_camera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        gl_camera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        gl_camera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_F) {
        gl_camera.CopyAttributes(scene.camera);
    } else if (e->key() == Qt::Key_G) {
        scene.camera = Camera(gl_camera);
        scene.camera.recreate();
    } else
    {
        invalid_key = true;
    }

    if (!invalid_key)
    {
        something_rendered = false;
        gl_camera.RecomputeAttributes();


        if(!is_rendering)
        {
            // If we moved the camera and we're not currently rendering,
            // then clean the pixels of our film.
            scene.film.cleanPixels();
        }

        update();  // Calls paintGL, among other things
    }
}

void MyGL::onRenderUpdate()
{
    if (!is_rendering)
        return;

    update();

    if (QThreadPool::globalInstance()->activeThreadCount() > 0)
    {
        return;
    }

    if (QThreadPool::globalInstance()->waitForDone())
    {
        completeRender();
    }
}

void MyGL::SceneLoadDialog()
{
    something_rendered = false;

    QString filepath = QFileDialog::getOpenFileName(0, QString("Load Scene"), QString("../scene_files"), tr("*.json"));
    if(filepath.length() == 0)
    {
        return;
    }

    QFile file(filepath);
    int i = filepath.length() - 1;
    while(QString::compare(filepath.at(i), QChar('/')) != 0)
    {
        i--;
    }
    QStringRef local_path = filepath.leftRef(i+1);
    //Reset all of our objects
    scene.Clear();
    //Load new objects based on the JSON file chosen.

    json_reader.LoadSceneFromFile(file, local_path, scene);
    gl_camera.CopyAttributes(scene.camera);
    ResizeToSceneCamera();
}

void MyGL::RenderScene()
{
    if (is_rendering) {return;}

    //BVH Tree
    if(makeBVH && !scene.bvh)
    {
        scene.bvh = new BVHAccel(scene.primitives.toVector().toStdVector(), maxBVHPrims);
    }
    else if(makeBVH && scene.bvh)
    {
        scene.clearBVH();
        scene.bvh = new BVHAccel(scene.primitives.toVector().toStdVector(), maxBVHPrims);
    }
    else if(!makeBVH && scene.bvh)
    {
        scene.clearBVH();
    }

    //Photon Mapping
    int seed_photonmapping = 0;
    if(integratorType == PHOTON_MAP_LIGHTING && !scene.kdTree)
    {
        scene.photons = new Photons(&scene, sampler->Clone(seed_photonmapping), recursionLimit);
        scene.photons->createTree(scene.kdTree);
    }
    else if(integratorType == PHOTON_MAP_LIGHTING && scene.kdTree)
    {
        scene.clearkdTree();
        scene.photons = new Photons(&scene, sampler->Clone(seed_photonmapping), recursionLimit);
        scene.photons->createTree(scene.kdTree);
    }

    output_filepath = QFileDialog::getSaveFileName(0, QString("Save Image"), QString("../rendered_images"), tr("*.png"));
    if(output_filepath.length() == 0)
    {
        return;
    }
    // recreate progressive rendering texture
    if (progressive_texture)
    {
        progressive_texture->destroy();
        delete progressive_texture;
        progressive_texture = nullptr;
    }
    progressive_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    progressive_texture->setSize(scene.film.bounds.Diagonal().x, scene.film.bounds.Diagonal().y);
    progressive_texture->setFormat(QOpenGLTexture::TextureFormat::RGBA32F);
    progressive_texture->allocateStorage();
    // Get the bounds of the camera portion we are going to render
    // Ask the film for its bounds
    Bounds2i renderBounds = scene.film.bounds;
    // Change bounds to selected screen area if appropriate
    if(origin != rubberband_offset)
    {
        renderBounds = Bounds2i(Point2i(origin.x(), origin.y()), Point2i(rubberband_offset.x(), rubberband_offset.y()));
    }
    // Get the XY lengths of the film
    Vector2i renderExtent = renderBounds.Diagonal();
    // Compute the number of tiles needed in X and Y based on tileSize and width or height
    const int tileSize = 16;
    Point2i nTiles((renderExtent.x + tileSize - 1) / tileSize,
                   (renderExtent.y + tileSize - 1) / tileSize);

    renderTimer.restart();

    // For every tile, begin a render task:
    for(int X = 0; X < nTiles.x; X++)
    {
        for(int Y = 0; Y < nTiles.y; Y++)
        {
            int x0 = renderBounds.Min().x + X * tileSize;
            int x1 = std::min(x0 + tileSize, renderBounds.Max().x);
            int y0 = renderBounds.Min().y + Y * tileSize;
            int y1 = std::min(y0 + tileSize, renderBounds.Max().y);
            Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));
            // Create a seed unique to the tile: pos.y * numberOfXTiles + pos.x
            int seed = y0 * nTiles.x + x0;

            Integrator* rt = nullptr;
            switch(integratorType)
            {
            case DIRECT_LIGHTING:
                rt = new DirectLightingIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            case FULL_LIGHTING:
                rt = new FullLightingIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            case PARTICIPATING_MEDIA_LIGHTING:
                rt = new ParticipatingMediaIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            case PHOTON_MAP_LIGHTING:
                rt = new PhotonMappingIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            case NAIVE_LIGHTING:
                rt = new NaiveIntegrator(tileBounds, &scene, sampler->Clone(seed), recursionLimit);
                break;
            }
#define MULTITHREAD // Comment this line out to be able to debug with breakpoints.
#ifdef MULTITHREAD
            QThreadPool::globalInstance()->start(rt);
#else
            // Use this commented-out code to only render a tile with your desired pixel
            //            Point2i debugPixel(200,200);
            //            if(x0 < debugPixel.x && x1 >= debugPixel.x && y0 < debugPixel.y && y1 >= debugPixel.y)
            //            {
            //                rt->Render();
            //            }
            rt->Render();
            delete rt;
#endif //MULTITHREAD
        }
    }
    render_event_timer.start(500);
    is_rendering = true;
    something_rendered = true;
    emit sig_DisableGUI(true);
}

void MyGL::GLDrawProgressiveView()
{
    // if rendering, draw progressive rendering scene;
    prog_progressive.bind();

    Vector2i dims = scene.film.bounds.Diagonal();
    std::vector<glm::vec4> texdata(dims.x * dims.y);

    for (unsigned int x = 0; x < dims.x; x++)
    {
        for (unsigned int y = 0; y < dims.y; y++)
        {
            if (scene.film.IsPixelColorSet(Point2i(x, y)))
            {
                texdata[x + y * dims.x] = glm::vec4(scene.film.GetColor(Point2i(x, y)), 1.0f);
            }
            else
            {
                texdata[x + y * dims.x] = glm::vec4(0.0f);
            }
        }
    }

    progressive_texture->setData(QOpenGLTexture::PixelFormat::RGBA, QOpenGLTexture::PixelType::Float32, texdata.data());
    progressive_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    progressive_texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // the initial texture is from the opengl frame buffer before start rendering
    progressive_texture->bind();

    glBindBuffer(GL_ARRAY_BUFFER, progressive_position_buffer);

    prog_progressive.enableAttributeArray(prog_progressive_attribute_position);
    glVertexAttribPointer(prog_progressive_attribute_position, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
    prog_progressive.enableAttributeArray(prog_progressive_attribute_texcoord);
    glVertexAttribPointer(prog_progressive_attribute_texcoord, 2, GL_FLOAT, GL_TRUE, 5*sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_BLEND);
    progressive_texture->release();
}

void MyGL::completeRender()
{
    std::cout << "Milliseconds to render: " << renderTimer.elapsed() << std::endl;
    is_rendering = false;
    something_rendered = true;
    render_event_timer.stop();
    scene.film.WriteImage(output_filepath);
    completeSFX.play();
    emit sig_DisableGUI(false);
}

void MyGL::slot_SetNumSamplesSqrt(int i)
{
    sampler->samplesPerPixel = i * i;
}

void MyGL::slot_SetRecursionLimit(int n)
{
    recursionLimit = n;
}

void MyGL::slot_SetProgressiveRender(bool b)
{
    progressive_render = b;
}

void MyGL::slot_SetIntegratorType(int t)
{
    switch(t)
    {
    case 0:
        integratorType = NAIVE_LIGHTING;
        break;
    case 1:
        integratorType = DIRECT_LIGHTING;
        break;
    case 3:
        integratorType = FULL_LIGHTING;
        break;
    case 4:
        integratorType = PARTICIPATING_MEDIA_LIGHTING;
        break;
    case 5:
        integratorType = PHOTON_MAP_LIGHTING;
        break;
    }
}

void MyGL::slot_UseBVH(bool b)
{
    makeBVH = b;
}

void MyGL::slot_SetMaxBVHPrims(int i)
{
    maxBVHPrims = i;
}

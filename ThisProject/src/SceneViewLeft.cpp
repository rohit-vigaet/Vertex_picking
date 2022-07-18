#include "SceneViewLeft.h"

#include <QExposeEvent>
#include <QOpenGLShaderProgram>
#include <QDateTime>

#include "DebugApplication.h"
#include "PickObject.h"

#define SHADER(x) m_shaderPrograms[x].shaderProgram()


SceneViewLeft::SceneViewLeft() :
    m_inputEventReceived(false)
{
    // tell keyboard handler to monitor certain keys
    m_keyboardMouseHandler.addRecognizedKey(Qt::Key_W);
    m_keyboardMouseHandler.addRecognizedKey(Qt::Key_A);
    m_keyboardMouseHandler.addRecognizedKey(Qt::Key_S);
    m_keyboardMouseHandler.addRecognizedKey(Qt::Key_D);
    m_keyboardMouseHandler.addRecognizedKey(Qt::Key_Q);
    m_keyboardMouseHandler.addRecognizedKey(Qt::Key_E);
    m_keyboardMouseHandler.addRecognizedKey(Qt::Key_Shift);

    // *** create scene (no OpenGL calls are being issued below, just the data structures are created.

    // Shaderprogram #0 : regular geometry (painting triangles via element index)
    ShaderProgram blocks (PROJECT_SOURCE_LOCATION "shaders/withWorldAndCamera.vert"
						 ,PROJECT_SOURCE_LOCATION "shaders/simple.frag");
    blocks.m_uniformNames.append("worldToView");
    m_shaderPrograms.append( blocks );

    // Shaderprogram #1 : grid (painting grid lines)
    ShaderProgram grid(PROJECT_SOURCE_LOCATION "shaders/grid.vert",PROJECT_SOURCE_LOCATION "shaders/grid.frag");
    grid.m_uniformNames.append("worldToView"); // mat4
    grid.m_uniformNames.append("gridColor"); // vec3
    grid.m_uniformNames.append("backColor"); // vec3
    m_shaderPrograms.append( grid );

    // *** initialize camera placement and model placement in the world

    // move camera a little back (mind: positive z) and look straight ahead
    m_camera.translate(-0.5,0.5,-5);
    // look slightly down
    m_camera.rotate(-5, m_camera.right());
    // look slightly left
    m_camera.rotate(-170, QVector3D(0.0f, 1.0f, 0.0f));

    m_boxObject.loadObj(PROJECT_ROOT_LOCATION "assets/frame1.ply");
    m_boxObject.boxobj();
}


SceneViewLeft::~SceneViewLeft() {
    if (m_context) {
        m_context->makeCurrent(this);

        for (ShaderProgram & p : m_shaderPrograms)
            p.destroy();

        m_boxObject.destroy();
        m_gridObject.destroy();
        m_pickLineObject.destroy();

        m_gpuTimers.destroy();
    }
}


void SceneViewLeft::initializeGL() {
    FUNCID(SceneView::initializeGL);
    try {
        // initialize shader programs
        for (ShaderProgram & p : m_shaderPrograms)
            p.create();

        // tell OpenGL to show only faces whose normal vector points towards us
        glDisable(GL_CULL_FACE);
        // enable depth testing, important for the grid and for the drawing order of several objects
        glDisable(GL_DEPTH_TEST);

        // initialize drawable objects
        m_boxObject.create(SHADER(0));
        m_gridObject.create(SHADER(1));
        m_pickLineObject.create(SHADER(0));

        // Timer
        m_gpuTimers.setSampleCount(6);
        m_gpuTimers.create();
    }
    catch (OpenGLException & ex) {
        throw OpenGLException(ex, "OpenGL initialization failed.", FUNC_ID);
    }
}


void SceneViewLeft::resizeGL(int width, int height) {
    // the projection matrix need to be updated only for window size changes
    m_projection.setToIdentity();
    // create projection matrix, i.e. camera lens
    m_projection.perspective(
                /* vertical angle */ 60.0f,
                /* aspect ratio */   width / float(height),
                /* near */           0.05f,
                /* far */            2000.0f
        );
    // Mind: to not use 0.0 for near plane, otherwise depth buffering and depth testing won't work!

    // update cached world2view matrix
    updateWorld2ViewMatrix();
}


void SceneViewLeft::paintGL() {
    m_cpuTimer.start();
    if (((DebugApplication *)qApp)->m_aboutToTerminate)
        return;

    // process input, i.e. check if any keys have been pressed
    if (m_inputEventReceived)
        processInput();

    const qreal retinaScale = devicePixelRatio(); // needed for Macs with retina display
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    qDebug() << "SceneView::paintGL(): Rendering to:" << width() << "x" << height();

    // set the background color = clear color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the background color = clear color
    QVector3D backColor(0.1f, 0.5f, 0.5f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    QVector3D gridColor(0.5f, 0.5f, 0.7f);

    m_gpuTimers.reset();

    m_gpuTimers.recordSample(); // setup boxes

    // *** render boxes
    SHADER(0)->bind();
    SHADER(0)->setUniformValue(m_shaderPrograms[0].m_uniformIDs[0], m_worldToView);


    m_gpuTimers.recordSample(); // render boxes

    m_boxObject.render();

    m_gpuTimers.recordSample(); // render pickline
    if (m_pickLineObject.m_visible)
        m_pickLineObject.render();

    SHADER(0)->release();

    // *** render grid ***

    m_gpuTimers.recordSample(); // setup grid
    SHADER(1)->bind();
    SHADER(1)->setUniformValue(m_shaderPrograms[1].m_uniformIDs[0], m_worldToView);
    SHADER(1)->setUniformValue(m_shaderPrograms[1].m_uniformIDs[1], gridColor);
    SHADER(1)->setUniformValue(m_shaderPrograms[1].m_uniformIDs[2], backColor);

    m_gpuTimers.recordSample(); // render grid
    //m_gridObject.render();
    SHADER(1)->release();

    m_gpuTimers.recordSample(); // done painting


#if 0
    // do some animation stuff
    m_transform.rotate(1.0f, QVector3D(0.0f, 0.1f, 0.0f));
    updateWorld2ViewMatrix();
    renderLater();
#endif

    checkInput();

    QVector<GLuint64> intervals = m_gpuTimers.waitForIntervals();
    for (GLuint64 it : intervals)
        qDebug() << "  " << it*1e-6 << "ms/frame";
    QVector<GLuint64> samples = m_gpuTimers.waitForSamples();
    qDebug() << "Total render time: " << (samples.back() - samples.front())*1e-6 << "ms/frame";

    qint64 elapsedMs = m_cpuTimer.elapsed();
    qDebug() << "Total paintGL time: " << elapsedMs << "ms";
}


void SceneViewLeft::keyPressEvent(QKeyEvent *event) {
    m_keyboardMouseHandler.keyPressEvent(event);
    checkInput();
}

void SceneViewLeft::keyReleaseEvent(QKeyEvent *event) {
    m_keyboardMouseHandler.keyReleaseEvent(event);
    checkInput();
}

void SceneViewLeft::mousePressEvent(QMouseEvent *event) {
    m_keyboardMouseHandler.mousePressEvent(event);
    checkInput();
}

void SceneViewLeft::mouseReleaseEvent(QMouseEvent *event) {
    m_keyboardMouseHandler.mouseReleaseEvent(event);
    checkInput();
}

void SceneViewLeft::mouseMoveEvent(QMouseEvent * /*event*/) {
    checkInput();
}

void SceneViewLeft::wheelEvent(QWheelEvent *event) {
    m_keyboardMouseHandler.wheelEvent(event);
    checkInput();
}


void SceneViewLeft::pick(const QPoint & globalMousePos) {
    // local mouse coordinates
    QPoint localMousePos = mapFromGlobal(globalMousePos);
    int my = localMousePos.y();
    int mx = localMousePos.x();

    // viewport dimensions
    const qreal retinaScale = devicePixelRatio(); // needed for Macs with retina display
    qreal halfVpw = width()*retinaScale/2;
    qreal halfVph = height()*retinaScale/2;

    // invert world2view matrix, with m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
    bool invertible;
    QMatrix4x4 projectionMatrixInverted = m_worldToView.inverted(&invertible);
    if (!invertible) {
        qWarning()<< "Cannot invert projection matrix.";
        return;
    }

    // mouse position in NDC space, one point on near plane and one point on far plane
    QVector4D nea(
                (mx - halfVpw) / halfVpw,
                -1*(my - halfVph) / halfVph,
                -1,
                1.0);

    QVector4D fa(
                nea.x(),
                nea.y(),
                1,
                1.0);

    // transform from NDC to model coordinates
    QVector4D nearResult = projectionMatrixInverted*nea;
    QVector4D farResult = projectionMatrixInverted*fa;
    // don't forget normalization!
    nearResult /= nearResult.w();
    farResult /= farResult.w();

    // update pick line vertices (visualize pick line)
    m_context->makeCurrent(this);
    m_pickLineObject.setPoints(nearResult.toVector3D(), farResult.toVector3D());

    // now do the actual picking - for now we implement a selection
    selectNearestObject(nearResult.toVector3D(), farResult.toVector3D());
}


void SceneViewLeft::checkInput() {
    // this function is called whenever _any_ key/mouse event was issued

    // we test, if the current state of the key handler requires a scene update
    // (camera movement) and if so, we just set a flag to do that upon next repaint
    // and we schedule a repaint

    // trigger key held?
    if (m_keyboardMouseHandler.buttonDown(Qt::RightButton)) {
        // any of the interesting keys held?
        if (m_keyboardMouseHandler.keyDown(Qt::Key_W) ||
            m_keyboardMouseHandler.keyDown(Qt::Key_A) ||
            m_keyboardMouseHandler.keyDown(Qt::Key_S) ||
            m_keyboardMouseHandler.keyDown(Qt::Key_D) ||
            m_keyboardMouseHandler.keyDown(Qt::Key_Q) ||
            m_keyboardMouseHandler.keyDown(Qt::Key_E))
        {
            m_inputEventReceived = true;
//			qDebug() << "SceneView::checkInput() inputEventReceived";
            renderLater();
            return;
        }

        // has the mouse been moved?
        if (m_keyboardMouseHandler.mouseDownPos() != QCursor::pos()) {
            m_inputEventReceived = true;
//			qDebug() << "SceneView::checkInput() inputEventReceived: " << QCursor::pos() << m_keyboardMouseHandler.mouseDownPos();
            renderLater();
            return;
        }
    }
    // has the left mouse butten been release
    if (m_keyboardMouseHandler.buttonReleased(Qt::LeftButton)) {
        m_inputEventReceived = true;
        renderLater();
        return;
    }

    // scroll-wheel turned?
    if (m_keyboardMouseHandler.wheelDelta() != 0) {
        m_inputEventReceived = true;
        renderLater();
        return;
    }
}


void SceneViewLeft::processInput() {
    // function must only be called if an input event has been received
    Q_ASSERT(m_inputEventReceived);
    m_inputEventReceived = false;
//	qDebug() << "SceneView::processInput()";

    // check for trigger key
    if (m_keyboardMouseHandler.buttonDown(Qt::RightButton)) {

        // Handle translations
        QVector3D translation;
        if (m_keyboardMouseHandler.keyDown(Qt::Key_W)) 		translation += m_camera.forward();
        if (m_keyboardMouseHandler.keyDown(Qt::Key_S)) 		translation -= m_camera.forward();
        if (m_keyboardMouseHandler.keyDown(Qt::Key_A)) 		translation -= m_camera.right();
        if (m_keyboardMouseHandler.keyDown(Qt::Key_D)) 		translation += m_camera.right();
        if (m_keyboardMouseHandler.keyDown(Qt::Key_Q)) 		translation -= m_camera.up();
        if (m_keyboardMouseHandler.keyDown(Qt::Key_E)) 		translation += m_camera.up();

        float transSpeed = 0.01f;
        if (m_keyboardMouseHandler.keyDown(Qt::Key_Shift))
            transSpeed = 0.005f;
        m_camera.translate(translation * transSpeed);

        // Handle rotations
        // get and reset mouse delta (pass current mouse cursor position)
        QPoint mouseDelta = m_keyboardMouseHandler.resetMouseDelta(QCursor::pos()); // resets the internal position
        static const float rotatationSpeed  = 0.4f;
        const QVector3D LocalUp(0.0f, 1.0f, 0.0f); // same as in Camera::up()
        m_camera.rotate(-rotatationSpeed * mouseDelta.x(), LocalUp);
        m_camera.rotate(-rotatationSpeed * mouseDelta.y(), m_camera.right());

    }
    int wheelDelta = m_keyboardMouseHandler.resetWheelDelta();
    if (wheelDelta != 0) {
        float transSpeed = 0.5f;
        if (m_keyboardMouseHandler.keyDown(Qt::Key_Shift))
            transSpeed = 0.1f;
        m_camera.translate(wheelDelta * m_camera.forward() * transSpeed);
    }

    // check for picking operation
    if (m_keyboardMouseHandler.buttonReleased(Qt::LeftButton)) {
        pick(m_keyboardMouseHandler.mouseReleasePos());
    }

    // finally, reset "WasPressed" key states
    m_keyboardMouseHandler.clearWasPressedKeyStates();

    updateWorld2ViewMatrix();
    // not need to request update here, since we are called from paint anyway
}


void SceneViewLeft::updateWorld2ViewMatrix() {
    // transformation steps:
    //   model space -> transform -> world space
    //   world space -> camera/eye -> camera view
    //   camera view -> projection -> normalized device coordinates (NDC)
    m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
}

void SceneViewLeft::selectNearestObject(const QVector3D & nearPoint, const QVector3D & farPoint) {
    QElapsedTimer pickTimer;
    pickTimer.start();

    // compute view direction
    QVector3D d = farPoint - nearPoint;

    // create pick object, distance is a value between 0 and 1, so initialize with 2 (very far back) to be on the safe side.
    PickObject p(2.f, std::numeric_limits<unsigned int>::max());

    // now process all objects and update p to hold the closest hit
    m_boxObject.pick(nearPoint, d, p);
    // ... other objects

    // any object accepted a pick?
    if (p.m_objectId == std::numeric_limits<unsigned int>::max())
        return; // nothing selected

    qDebug().nospace() << "Pick successful (Box #"
                       << p.m_objectId <<  ", Face #" << p.m_faceId << ", t = " << p.m_dist << ") after "
                       << pickTimer.elapsed() << " ms";

    // Mind: OpenGL-context must be current when we call this function!
    m_boxObject.highlight(p.m_objectId, p.m_faceId);
}

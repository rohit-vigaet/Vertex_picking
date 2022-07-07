QT       += core gui opengl openglwidgets widgets
LIBS += -lopengl32 -lUser32 -lGdi32
LIBS += "E:\CodeProjects\vcpkg\installed\x64-windows\lib\pcl_io.lib" "E:\CodeProjects\vcpkg\installed\x64-windows\lib\pcl_common.lib"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += "E:\CodeProjects\Repos\hazel\Hazel\vendor\glm\glm" "E:\CodeProjects\vcpkg\installed\x64-windows\include"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BoxMesh.cpp \
    BoxObject.cpp \
    GridObject.cpp \
    KeyboardMouseHandler.cpp \
    ObjModel.cpp \
    OpenGLException.cpp \
    OpenGLWindow.cpp \
    PickLineObject.cpp \
    PickObject.cpp \
    SceneView.cpp \
    SceneViewLeft.cpp \
    ShaderProgram.cpp \
    TestDialog.cpp \
    Transform3d.cpp \
    main.cpp

HEADERS += \
    BoxMesh.h \
    BoxObject.h \
    Camera.h \
    DebugApplication.h \
    GridObject.h \
    KeyboardMouseHandler.h \
    Model_Camera.h \
    Model_Math.h \
    ObjModel.h \
    OpenGLException.h \
    OpenGLWindow.h \
    PickLineObject.h \
    PickObject.h \
    SceneView.h \
    SceneViewLeft.h \
    ShaderProgram.h \
    TestDialog.h \
    Transform3d.h \
    Vertex.h

FORMS += \
    OpenGLWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

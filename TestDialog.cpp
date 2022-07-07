#include "TestDialog.h"

#include <iostream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QFile>

TestDialog::TestDialog() :
#ifdef Q_OS_WIN
    QDialog(nullptr, Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint)
#else
    QDialog(nullptr, Qt::Window)
#endif
{
    // *** create OpenGL window
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 4);
    format.setSamples(4);	// enable multisampling (antialiasing)
    format.setDepthBufferSize(32);
#ifdef GL_DEBUG_
    format.setOption(QSurfaceFormat::DebugContext);
#endif // GL_DEBUG
    
    m_sceneViewLeft = new SceneViewLeft;
    m_sceneViewLeft->setFormat(format);

    m_sceneViewRight = new SceneView;
    m_sceneViewRight->setFormat(format);
    
    // *** create window container widget
    
    QWidget* containerLeft = QWidget::createWindowContainer(m_sceneViewLeft);
    containerLeft->setFocusPolicy(Qt::TabFocus);
    containerLeft->setMinimumSize(QSize(720, 400));
    
    QWidget* containerRight = QWidget::createWindowContainer(m_sceneViewRight);
    containerRight->setFocusPolicy(Qt::TabFocus);
    containerRight->setMinimumSize(QSize(640, 400));

    // *** create the layout and insert widget container

    QHBoxLayout* vlay = new QHBoxLayout;
    //vlay->setMargin(0);
    vlay->setSpacing(5);
    vlay->addWidget(containerLeft);
    QPushButton* addPlyFile = new QPushButton(tr("Add PLY File"), this);
    addPlyFile->setMaximumSize(100, 100);
    QLabel* plyInfo = new QLabel(this);
    plyInfo->setWordWrap(true);
    plyInfo->setText("-Vieport Left-");
    
    vlay->addWidget(containerRight);

    QPushButton* addObjFile = new QPushButton(tr("Add OBJ File"), this);
    addObjFile->setMaximumSize(100, 100);
    QLabel* objInfo = new QLabel(this);
    objInfo->setWordWrap(true);
    objInfo->setText("-Vieport Right-");

    // now create some buttons at the bottom
    connect(addPlyFile, &QPushButton::clicked, this, &TestDialog::handleButton);

    QVBoxLayout* hlay = new QVBoxLayout;
    //hlay->setMargin(0);

    hlay->addLayout(vlay);

    hlay->addWidget(plyInfo);
    hlay->addWidget(addPlyFile);
    hlay->addWidget(objInfo);
    hlay->addWidget(addObjFile);

    QLabel* navigationInfo = new QLabel(this);
    navigationInfo->setWordWrap(true);
    navigationInfo->setText("Hold right mouse button for free mouse look and to navigate "
        "with keys WASDQE. Hold shift to slow down. Use scroll-wheel to move quickly forward and backward. "
        "Use left-click to select objects.");
    hlay->addWidget(navigationInfo);

    QPushButton* closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    closeBtn->setMaximumSize(50, 50);

    hlay->addWidget(closeBtn);
    //hlay->setStretch(0,1);

    setLayout(hlay);

    resize(700, 450);

    containerLeft->setFocus();
}

void TestDialog::handleButton()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), "C://", "Obj file (*.obj)");    
    
    m_sceneViewRight->filename = filepath.toStdString();

    button = true;

    m_sceneViewRight = new SceneView;
}
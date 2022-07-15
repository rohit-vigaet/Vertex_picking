#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>
#include <QPushButton>

#include "SceneView.h"
#include "SceneViewLeft.h"

/*! A simple test dialog for an embedded OpenGL window. */
class TestDialog : public QDialog {
    Q_OBJECT
public:
    TestDialog();

private slots:
    void handleButton();

private:
    // The scene view, that shows our world and allows navigation
    SceneViewLeft *m_sceneViewLeft;
    SceneView *m_sceneViewRight;

    bool button = false;
};

#endif // TESTDIALOG_H

#ifndef OBJMODEL_H
#define OBJMODEL_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "Vertex.h"
#include <glm.hpp>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

#include "BoxMesh.h"
#include "PickObject.h"


/*! A container for all the boxes.
    Basically creates the geometry of the individual boxes and populates the buffers.
*/
class ObjModel {
public:
    ObjModel();
    void loadObj(const char *filename);

    void boxobj();

    /*! The function is called during OpenGL initialization, where the OpenGL context is current. */
    void create(QOpenGLShaderProgram * shaderProgramm);
    void destroy();

    void render();

    /*! Thread-save pick function.
        Checks if any of the box object surfaces is hit by the ray defined by "p1 + d [0..1]" and
        stores data in po (pick object).
    */
    void pick(const QVector3D& p1, const QVector3D& d, PickObject & po) const;

    /*! Changes color of box and face to show that the box was clicked on. */
    void highlight(unsigned int boxId, unsigned int faceId);

    std::vector<BoxMesh>		m_boxes;

    std::vector<Vertex>			m_vertexBufferData;
    std::vector<GLuint>			m_elementBufferData;

    std::vector<Model_Vertex>    vertices;
    std::vector<GLint>           indices;

    std::vector<glm::fvec3> vertex_positions;
    

    /*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates and color buffers. */
    QOpenGLVertexArrayObject	m_vao;

    /*! Holds position and colors in a single buffer. */
    QOpenGLBuffer				m_vbo;
    /*! Holds elements. */
    QOpenGLBuffer				m_ebo;
    
    struct vertex
    {
        double x, y, z;
    };
    struct face
    {
        unsigned int v1, v2, v3;
    };

    std::vector<vertex> vertexes;
    std::vector<face> faces;
};

#endif // OBJMODEL_H

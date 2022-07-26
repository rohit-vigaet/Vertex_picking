#ifndef VERTEX_H
#define VERTEX_H

#include <QVector3D>
#include <QColor>
#include <glm.hpp>
#include "PickObject.h"

/*! A container class to store data (coordinates, normals, textures, colors) of a vertex, used for interleaved
    storage. Expand this class as needed.

    Memory layout (each char is a byte): xxxxyyyyzzzzrrrrggggbbbb = 6*4 = 24 Bytes

    You can define a vector<Vertex> and use this directly as input to the vertex buffer.

    Mind implicit padding by compiler! Hence, for allocation use:
    - sizeof(Vertex) as stride
    - offsetof(Vertex, r) as start offset for the color

    This will only become important, if mixed data types are used in the struct.
    Read http://www.catb.org/esr/structure-packing/ for an in-depth explanation.
*/

struct Vertex {
    Vertex() {}
    Vertex(const QVector3D & coords, const QColor & col) :
        x(float(coords.x())),
        y(float(coords.y())),
        z(float(coords.z())),
        r(float(col.redF())),
        g(float(col.greenF())),
        b(float(col.blueF()))
    {
    }

    float x,y,z;
    float r,g,b;
};

struct Model_Vertex
{
    glm::vec3 positions;

    //bool intersectP(const glm::vec3 &m, const glm::vec3& n, const glm::vec3& f) const
    //{
    //    return intersectsPoint(m, n, f);
    //}
};



#endif // VERTEX_H

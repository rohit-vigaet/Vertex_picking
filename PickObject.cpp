#include "PickObject.h"

#include <QDebug>

#include <geometric.hpp>

#include <iostream>

/* Solves equation system with Cramer's rule:
     a x + c y = e
     b x + d y = f
*/
bool solve(double a, double b, double c,  double d,  double e,  double f, double & x, double & y) {
    double det = a*d - b*c;
    if (det == 0.)
        return false;

    x = (e*d - c*f)/det;
    y = (a*f - e*b)/det;
    return true;
}


bool intersectsRect(const QVector3D & a,
                const QVector3D & b,
                const QVector3D & normal,
                const QVector3D & offset,
                const QVector3D & p1,
                const QVector3D & d,
                float & dist)
{
    // first the normal test

    double angle = QVector3D::dotProduct(d, normal)/qAbs(d.length());
    // Condition 1: same direction of normal vectors?
    if (angle >= 0)
        return false; // no intersection possible

    // compute intersection point on line
    double t = QVector3D::dotProduct(offset - p1, normal) / QVector3D::dotProduct(d, normal);
    // Condition 2: outside viewing range?
    if (t < 0 || t > 1)
        return false;

    // now determine location on plane
    QVector3D x0 = p1 + t*d;

    QVector3D rhs = x0 - offset; // right hand side of equation system:  a * x  +  b * y = (x - offset)

    // we have three possible ways to get the intersection point, try them all until we succeed
    double x,y;
    // rows 1 and 2
    if (solve(a.x(), a.y(), b.x(), b.y(), rhs.x(), rhs.y(), x, y)) {
        // Condition 3: check if inside rect
        if (x > 0 && x < 1 && y > 0 && y < 1) {
            dist = t;
//			qDebug() << "Intersection found (1) at t = " << dist;
            return true;
        }
        else
            return false;
    }
    // rows 1 and 3
    if (solve(a.x(), a.z(), b.x(), b.z(), rhs.x(), rhs.z(), x, y)) {
        // Condition 3: check if inside rect
        if (x > 0 && x < 1 && y > 0 && y < 1) {
            dist = t;
//			qDebug() << "Intersection found (2) at t = " << dist;
            return true;
        }
        else
            return false;
    }
    // rows 2 and 3
    if (solve(a.y(), a.z(), b.y(), b.z(), rhs.y(), rhs.z(), x, y)) {
        // Condition 3: check if inside rect
        if (x > 0 && x < 1 && y > 0 && y < 1) {
            dist = t;
//			qDebug() << "Intersection found (3) at t = " << dist;
            return true;
        }
        else
            return false;
    }

    return false;
}

bool rayTriangleIntersect(
    const glm::vec3& orig, const glm::vec3& dir,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
    float& t)
{
    // compute plane's normal
    glm::vec3 v0v1 = v1 - v0;
    glm::vec3 v0v2 = v2 - v0;
    // no need to normalize
    glm::vec3 N = glm::cross(v0v1, v0v2);//N 
    float area2 = N.length();

    // Step 1: finding P

    // check if ray and plane are parallel ?
    float NdotRayDirection = glm::dot(N, dir);
    if (fabs(NdotRayDirection) < std::numeric_limits<float>::epsilon())  //almost 0 
        return false;  //they are parallel so they don't intersect ! 

    // compute d parameter using equation 2
    float d = -glm::dot(N, v0);

    // compute t (equation 3)
    t = -(glm::dot(N, orig) + d) / NdotRayDirection;

    // check if the triangle is in behind the ray
    if (t < 0) return false;  //the triangle is behind 

    // compute the intersection point using equation 1
    glm::vec3 P = orig + t * dir;

    // Step 2: inside-outside test
    glm::vec3 C;  //vector perpendicular to triangle's plane 

    // edge 0
    glm::vec3 edge0 = v1 - v0;
    glm::vec3 vp0 = P - v0;
    C = glm::cross(edge0, vp0);
    if (glm::dot(N, C) < 0) return false;  //P is on the right side

    // edge 1
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 vp1 = P - v1;
    C = glm::cross(edge1, vp1);
    if (glm::dot(N, C) < 0)  return false;  //P is on the right side 

    // edge 2
    glm::vec3 edge2 = v0 - v2;
    glm::vec3 vp2 = P - v2;
    C = glm::cross(edge2, vp2);
    if (glm::dot(N, C) < 0) return false;  //P is on the right side 

    if (glm::length(vp0) < glm::length(vp1))
        return true;
    else if (glm::length(vp0) < glm::length(vp2))
        return true;
    else if (glm::length(vp1) < glm::length(vp2))
        return true;
    else if (glm::length(vp1) < glm::length(vp0))
        return true;
    else if (glm::length(vp2) < glm::length(vp0))
        return true;
    else if (glm::length(vp2) < glm::length(vp1))
        return true;
    else
        return false;

    //return true;  //this ray hits the triangle 
}


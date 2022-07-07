#pragma once

#include <glm.hpp>

#include <QVector3D>

#include "PickObject.h"

struct Model {

    glm::vec3 point;

    QVector3D Qvec3(const glm::vec3 v)
    {
        QVector3D q;
        q.setX(v.x);
        q.setY(v.y);
        q.setZ(v.z);
        return q;
    }

    bool interPoint(float pointIdx, const QVector3D& p1, const QVector3D& d, float& dist)
    {
        std::vector<Model> m_pointInfo;
        const Model& p = m_pointInfo[pointIdx];
        return intersectsPoint(Qvec3(p.point), p1, d, dist);
    }

    
};
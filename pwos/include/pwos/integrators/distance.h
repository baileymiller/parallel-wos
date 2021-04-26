#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>

class Distance: public Integrator
{
public:
    Distance(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1)
    : Integrator("dist", scene, res, spp, nthreads)
    {};

    void virtual render() override
    {
        image->render(scene->getWindow(), 1, [this](Vec2f coord, pcg32& _) -> Vec3f
        {
            Vec3f b;
            float dist = (scene->getClosestPoint(coord, b) - coord).norm();
            return Vec3f(dist, dist, dist);
        });
    }
};

#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>

class Distance: public Integrator
{
public:
    Distance(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16): Integrator(scene, res, spp) {};

    void virtual render() override
    {
        image->render(scene->getWindow(), 1, [this](Vec2f coord, pcg32& _) -> Vec3f
        {
            Vec3f b;
            float dist = (scene->getClosestPoint(coord, b) - coord).norm();
            return Vec3f(dist, dist, dist);
        });
    }

    void virtual save() override
    {
        string filename = "distance_scene=" + scene->getName();
        image->save(filename);
    }
};

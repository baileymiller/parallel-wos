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
        Vec4f window = scene->getWindow();
        Vec2i res = image->getRes();

        for (int i = 0; i < image->getNumPixels(); i++)
        {
            Vec2i pixel = image->getPixelCoordinates(i);
            Vec2f coord = getXYCoords(pixel, window, res);
            Vec3f b;
            float dist = (scene->getClosestPoint(coord, b) - coord).norm();
            image->set(i, Vec3f(dist, dist, dist));
        }
    }

    void virtual save() override
    {
        string filename = "distance_scene=" + scene->getName();
        image->save(filename);
    }
};

#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>
#include <pwos/closestPointGrid.h>

class GridVisual: public Integrator
{
public:
    GridVisual(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16): Integrator(scene, res, spp) {};

    void virtual render() override
    {
        // preprocess by computing closest point grid
        Vec4f window = scene->getWindow();
        Vec2f bl(window[0], window[1]);
        Vec2f tr(window[2], window[3]);
        float dx = tr.x() - bl.x();
        float dy = tr.y() - bl.y();

        // ensures cells are basically width of 4 pixels
        Vec2i res = image->getRes();
        float cellLength = 4 * std::min(dx / res.x(), dy / res.y());
        ClosestPointGrid cpg(scene, bl, tr, cellLength, 1);

        image->render(scene->getWindow(), 1, [this, cpg](Vec2f coord, pcg32& _) -> Vec3f
        {
            Vec3f b;
            float dist, gridDist;
            if (cpg.getDistToClosestPoint(coord, b, dist, gridDist))
            {
                return b * dist;
            }
            else
            {
                return Vec3f(-1.0f, -1.0f, -1.0f);
            }
        });
    }

    void virtual save() override
    {
        string filename = "gridvisual_scene=" + scene->getName();
        image->save(filename);
    }
};

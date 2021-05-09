#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>
#include <pwos/closestPointGrid.h>

class GridVisual: public Integrator
{
public:
    shared_ptr<ClosestPointGrid> cpg;

    GridVisual(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1, float cellSize = 1)
    : Integrator("gridviz", scene, res, spp, nthreads) 
    {
        // preprocess by computing closest point grid
        Vec4f window = this->scene->getWindow();
        Vec2f bl(window[0], window[1]);
        Vec2f tr(window[2], window[3]);
        float dx = tr.x() - bl.x();
        float dy = tr.y() - bl.y();

        // ensures cells are basically width of 4 pixels
        float cellLength = cellSize * std::min(dx / res.x(), dy / res.y());
        cpg = make_shared<ClosestPointGrid>(this->scene, bl, tr, cellLength, nthreads);
    };

    void virtual render() override
    {
        image->render(scene->getWindow(), nthreads, [this](Vec2f coord, pcg32& _) -> Vec3f
        {
            if (cpg->pointInGridRange(coord))
            {
                Vec3f b;
                float dist, gridDist;
                cpg->getDistToClosestPoint(coord, b, dist, gridDist);
                return b * dist;
            }
            else
            {
                return Vec3f(-1.0f, -1.0f, -1.0f);
            }
        });
    }
};

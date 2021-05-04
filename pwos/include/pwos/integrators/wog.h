#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>
#include <pwos/closestPointGrid.h>

class WoG: public Integrator
{
public:
    float rrProb = 0.99;
    float cellLength, minGridR;
    shared_ptr<ClosestPointGrid> cpg;

    WoG(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1)
    : Integrator("wog", scene, res, spp, nthreads)
    {
        // preprocess by computing closest point grid
        Vec4f window = this->scene->getWindow();
        Vec2f bl(window[0], window[1]);
        Vec2f tr(window[2], window[3]);
        float dx = tr.x() - bl.x();
        float dy = tr.y() - bl.y();

        // ensures cells are basically width of a pixel
        // precompute grid
        cellLength = 0.01 * std::min(dx / res.x(), dy / res.y());
        minGridR = sqrt(2) * cellLength;
        cpg = make_shared<ClosestPointGrid>(this->scene, bl, tr, cellLength, nthreads);
    };

    void virtual render() override
    {
        image->render(scene->getWindow(), nthreads, [this](Vec2f coord, pcg32& sampler) -> Vec3f
        {
            Vec3f pixelValue(0, 0, 0);
            for (int j = 0; j < spp; j++)
            {
                pixelValue += u_hat(coord, sampler);
            }
            return pixelValue / float(spp);
        });
    }

private:
    Vec3f u_hat(Vec2f x0, pcg32 &sampler) const
    {
        Vec2f p = x0;
        Vec3f b;
        float R, dist, gridDist;
        float f = 1.0f;
        do
        {
            if (cpg->pointInGridRange(p))
            {
                cpg->getDistToClosestPoint(p, b, dist, gridDist);

                // conservative distance to nearest boundary
                R = dist - gridDist;
                if (R < minGridR)
                {
                    // grid point too close to boundary, just do our own cpq
                    R = (scene->getClosestPoint(p, b) - p).norm();
                    if (R < BOUNDARY_EPSILON) break;
                }
            }
            else
            {
                // if we end up here, the point is not within the grid. do a normal closest point query.
                R = (scene->getClosestPoint(p, b) - p).norm();
                if (R < BOUNDARY_EPSILON) break;
            }

            if (sampler.nextFloat() < (1.0f - rrProb)) break;
            f /= rrProb;
            p += sampleCirclePoint(R, sampler.nextFloat());
        }
        while (true);

        return R < BOUNDARY_EPSILON ? b : Vec3f(0, 0, 0); 
    }
};

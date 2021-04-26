#pragma once
#include <pwos/common.h>
#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>

class WoS: public Integrator
{
public:
    void render() override
    {
        #pragma omp parallel
        {
            size_t tid = omp_get_thread_num();
            pcg32 sampler = getSampler(tid);

            Vec4f window = scene->getWindow();
            Vec2f res = image->getRes();

            #pragma omp for
            for (int i = 0; i < image->getNumPixels(); i++)
            {
                Vec2i pixel = image->getPixelCoordinates(i);
                Vec2f coord = getXYCoords(pixel, window, res);
                Vec3f pixelValue(0, 0, 0);
                for (int j = 0; j < spp; j++)
                {
                    pixelValue += u_hat(coord, sampler);
                }
                (*image)(i) = pixelValue / float(spp);
            }
        }
    }

    void save() override
    {
        string filename = "wos";
        filename += "_scene=" + scene->getName();
        filename += "_spp=" + std::to_string(spp);
    }

private:
    float rrProb = 0.95;

    Vec3f u_hat(Vec2f x0, pcg32 &sampler)
    {
        Vec2f p = x0;
        Vec3f b;
        float R;
        float f = 1.0f;
        do
        {
            R = (scene->getClosestPoint(p, b) - p).norm();
            if (R < BOUNDARY_EPSILON) break;
            if (sampler.nextFloat() < (1.0f - rrProb)) break;
            f /= rrProb;
            p += sampleCirclePoint(R, sampler.nextFloat());
        }
        while (true);

        return R < BOUNDARY_EPSILON ? b : Vec3f(0, 0, 0); 
    }
};

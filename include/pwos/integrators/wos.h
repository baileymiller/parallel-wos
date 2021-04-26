#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>

class WoS: public Integrator
{
public:
    WoS(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16): Integrator(scene, res, spp) {};

    void virtual render() override
    {
        image->render(scene->getWindow(), 24, [this](Vec2f coord, pcg32& sampler) -> Vec3f
        {
            Vec3f pixelValue(0, 0, 0);
            for (int j = 0; j < spp; j++)
            {
                pixelValue += u_hat(coord, sampler);
            }
            return pixelValue / float(spp);
        });
    }

    void virtual save() override
    {
        string filename = "wos_scene=" + scene->getName() + "_spp=" + to_string(spp);
        image->save(filename);
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

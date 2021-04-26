#include <pwos/common.h>

#include <pwos/circle.h>
#include <pwos/integrator.h>
#include <pwos/integrators/wos.h>
#include <pwos/image.h>
#include <pwos/scene.h>

Integrator::Integrator(Scene scene, Vec2i res, int spp): spp(spp) 
{
    this->scene = make_shared<Scene>(scene);
    image = make_shared<Image>(res);
}

void Integrator::render()
{
    THROW("Integrator::render not implemented by default");
}

void Integrator::save()
{
    THROW("Integrator::save not implemented by default");
}

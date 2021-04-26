#include <pwos/common.h>
#include <pwos/integrator.h>
#include <pwos/image.h>
#include <pwos/scene.h>

Integrator::Integrator(Scene scene, Vec2f res, int spp): spp(spp) 
{
    this->scene = make_shared<Scene>(scene);
    image = make_shared<Image>(res.x(), res.y());
}

void Integrator::render()
{
    THROW("Integrator::render not implemented by default");
}

void Integrator::save()
{
    THROW("Integrator::save not implemented by default");
}

#include <pwos/common.h>

#include <pwos/circle.h>
#include <pwos/integrator.h>
#include <pwos/integrators/wos.h>
#include <pwos/image.h>
#include <pwos/scene.h>

Integrator::Integrator(string name, Scene scene, Vec2i res, int spp, int nthreads)
: name(name)
, spp(spp)
, nthreads(nthreads)
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
    string filename = name + "_scene=" + scene->getName() + "_spp=" + to_string(spp) + "_nthreads=" + to_string(nthreads);
    image->save(filename);
}

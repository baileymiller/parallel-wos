#include <pwos/common.h>

#include <pwos/argparse.h>
#include <pwos/image.h>
#include <pwos/integrators/wos.h>
#include <pwos/scene.h>

shared_ptr<Integrator> buildIntegrator(string type, Scene scene, Vec2i res, int spp)
{
    switch(StrToIntegratorType.at(type))
    {
        case IntegratorType::WOS:
        default:
            return make_shared<WoS>(scene, res, spp);
    }
}

int main(int argc, char* argv[])
{
    // setup arg parser and parse command line args
    ArgParse parser({
        Arg("spp", ArgType::INT),
        Arg("nthreads", ArgType::INT),
        Arg("res", ArgType::VEC2i),
        Arg("integrator", ArgType::STR)
    });

    // parse
    parser.parse(argc, argv);

    // load scene
    int spp = parser.getInt("spp", 16);
    int nthreads = parser.getInt("nthreads", 1);
    Vec2i res = parser.getVec2i("res", Vec2i(128, 128));
    string integratorType = parser.getStr("integrator", "wos");

    // create the scene
    Scene scene(parser.getMain(0));

    // construct the integrator.
    shared_ptr<Integrator> integrator = buildIntegrator(integratorType, scene, res, spp);

    // run the integrator
    integrator->render();
    integrator->save();
}
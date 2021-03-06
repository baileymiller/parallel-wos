#include <pwos/common.h>

#include <pwos/argparse.h>
#include <pwos/image.h>
#include <pwos/scene.h>
#include <pwos/stats.h>

#include <pwos/integrators/wos.h>
#include <pwos/integrators/distance.h>
#include <pwos/integrators/gridVisual.h>
#include <pwos/integrators/wog.h>
#include <pwos/integrators/wogVisual.h>
#include <pwos/integrators/mcwogVisual.h>
#include <pwos/integrators/mcwog.h>

shared_ptr<Integrator> buildIntegrator(string type, Scene scene, Vec2i res, int spp, int nthreads, float cellSize)
{
    switch(StrToIntegratorType.at(type))
    {
        case IntegratorType::GRID_VISUAL:
            return make_shared<GridVisual>(scene, res, spp, nthreads, cellSize);
        case IntegratorType::DISTANCE:
            return make_shared<Distance>(scene, res, spp, nthreads);
        case IntegratorType::MCWOG:
            return make_shared<MCWoG>(scene, res, spp, nthreads, cellSize);
        case IntegratorType::MCWOG_VISUAL:
            return make_shared<MCWoGVisual>(scene, res, spp, nthreads, cellSize);
        case IntegratorType::WOG:
            return make_shared<WoG>(scene, res, spp, nthreads, cellSize);
        case IntegratorType::WOG_VISUAL:
            return make_shared<WoGVisual>(scene, res, spp, nthreads, cellSize);
        case IntegratorType::WOS:
        default:
            return make_shared<WoS>(scene, res, spp, nthreads);
    }
}

int main(int argc, char* argv[])
{
    // setup arg parser and parse command line args
    ArgParse parser({
        Arg("spp", ArgType::INT),
        Arg("nthreads", ArgType::INT),
        Arg("res", ArgType::VEC2i),
        Arg("integrator", ArgType::STR),
        Arg("cellsize", ArgType::FLOAT)
    });

    // parse
    parser.parse(argc, argv);

    // load scene
    int spp = parser.getInt("spp", 16);
    int nthreads = parser.getInt("nthreads", 1);
    Vec2i res = parser.getVec2i("res", Vec2i(128, 128));
    string integratorType = parser.getStr("integrator", "wos");
    float cellSize = parser.getFloat("cellsize", 1);

    // create the scene
    Scene scene(parser.getMain(0, "Must specify scene file ./pwos [scene file]"));

    Stats::init(nthreads);

    // build and run the integrator.
    shared_ptr<Integrator> integrator;
Stats::TIME(StatTimerType::TOTAL, [&integrator, integratorType, scene, res, spp, nthreads, cellSize]()->void {
Stats::TIME(StatTimerType::SETUP, [&integrator, integratorType, scene, res, spp, nthreads, cellSize]()->void {
        integrator = buildIntegrator(integratorType, scene, res, spp, nthreads, cellSize);
});
        integrator->render();
});
    Stats::report();
    integrator->save();
}
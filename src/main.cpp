#include <pwos/common.h>

#include <pwos/argparse.h>
#include <pwos/image.h>
#include <pwos/scene.h>

int main(int argc, char* argv[])
{
    // setup arg parser and parse command line args
    ArgParse parser({
        Arg("spp", ArgType::INT),
        Arg("nthreads", ArgType::INT),
        Arg("integrator", ArgType::STR)
    });

    parser.parse(argc, argv);

    int spp = parser.getInt("spp", 16);
    int nthreads = parser.getInt("nthreads", 1);
    string integrator = parser.getStr("integrator", "wos");
    Scene scene(parser.getMain(0));

    // run the specified integrator and save the image.
    if (integrator == "wos")
    {
        WoS wos(scene);
        wos.render(spp);
        wos.save();
    }
}
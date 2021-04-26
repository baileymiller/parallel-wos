#pragma once

#include <pwos/common.h>

/**
 * An interface for rendering scenes. 
 */
class Integrator
{
public:
    /**
     * Default constructor for an integrator.
     *
     * @param name      name of the integrator 
     * @param scene     scene to render
     * @param res       resolution of image created
     * @param spp       number of samples to use per pixel
     * @param nthreads  max number of threads to use at any stage of integration (pre-processing or rendering)
     */
    Integrator(string name, Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1);

    /**
     * Renders a scene and saves intermediate result within integrator (under img.)
     */
    void virtual render();

    /**
     * Saves the image produced by calling render.
     */
    void save();

protected:
    // name of the integrator
    string name;

    // number of threads
    int nthreads;

    // number of samples per pixel to be used
    int spp;

    // scene geometry and boundary conditions
    shared_ptr<Scene> scene;

    // image stored after running render
    shared_ptr<Image> image;
};
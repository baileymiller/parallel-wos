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
     * @param scene     scene to render
     * @param res       resolution of image created
     * @param spp       number of samples to use per pixel
     */
    Integrator(Scene scene, Vec2f res = Vec2f(128, 128), int spp = 16);

    /**
     * Renders a scene and saves intermediate result within integrator (under img.)
     */
    void virtual render();

    /**
     * Saves the image produced by calling render.
     */
    void virtual save();

protected:
    // number of samples per pixel to be used
    int spp;

    // scene geometry and boundary conditions
    shared_ptr<Scene> scene;

    // image stored after running render
    shared_ptr<Image> image;
};

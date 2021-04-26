#pragma once

#include <pwos/common.h>

/**
 * Contains a collection of circles with boundary values. 
 * 
 * Exposes functionality for querying boundary information.
 */
class Scene
{
public:
    /**
     * Construct a scene from a list of circles.
     * 
     * @param name      the name of the scene
     * @param window    the x,y ranges for the scene (top left x, top left y, bottom right x, bottom right y)
     * @param circles   a vector containing circles
     */
    Scene(string name, Vec4f window, vector<Circle> circles);

    /**
     * Construct a scene from a scene file. File is a list of circles with boundary data,
     * which must be formatted as follows:
     * 
     * tlx, tly, brx, bry                   // scene range (top left x, top left y, bottom right x, bottom right y)
     * x1, y1, r1, br1, bg1, bb1        // first circle of the scene (x pos, y pos, radius, rgb boundary value)
     * x2, y2, r2, br2, bg2, bb2
     * ...
     * xn, yn, rn, brn, bgn, bbn        // nth circle of the scene
     * 
     Note only constant dirichlet boundary conditions are supported. 
     * 
     * @param filename      name of the file (becomes the scene file)
     */
    Scene(std::string filename);

    /**
     * Computes the closest point to o from all of the circles in the scene.
     * 
     * @param o     point from which to compute the closest point
     * @param b     boundary condition at the closest point
     * 
     * @returns the closest point to "o"
     */
    Vec2f getClosestPoint(Vec2f o, Vec3f &b);

    /**
     * Returns the window of the scene window=(top left x, top left y, bottom right x, bottom right y)
     * 
     * @return window
     */
    Vec4f getWindow();

    /**
     * Returns the name of the scene.
     * 
     * @returns the name of the scene.
     */
    string getName();

private:
    // scene name
    string name;

    // scene window (top left x, top left y, bottom right x, bottom right y)
    Vec4f window;

    // list of circles
    vector<shared_ptr<Circle>> circles;

};

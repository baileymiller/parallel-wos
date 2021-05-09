#pragma once

#include <pwos/common.h>

/**
 * Simple image class, essentially a wrapper around a vector of rgb values
 * and image writing library (stb_image_write).
 */
class Image
{
public:
    /**
     * Construct an image with a specified resolution.
     * 
     * @param res       resolution of the image (width, height)
     * 
     */
    Image(Vec2i res);

    /**
     * Set the pixel data located at idx
     * 
     * @param idx
     * 
     * @return the RGB data stored at idx
     */
    void set(int idx, Vec3f value);

    /**
     * Access the pixel data located at x,y
     * 
     * @param x
     * @param y
     * 
     * @return the RGB data stored at x,y
     */
    Vec3f& operator()(int x, int y);

    /**
     * Helper function for rendering an image.
     * 
     */
    void render(Vec4f window, int nthreads, Rand2DFunction f);

    /**
     * Save the image as an HDR image.
     * 
     * @param filename
     */
    void save(string filename);

    /**
     * Returns the resolution of the image
     * 
     * @return the resolution of the image
     */
    Vec2i getRes();

    /**
     * Returns the number of pixels in the image.
     * 
     * @return the number of pixels in the image.
     */
    int getNumPixels();

    /** 
     * Returns the x,y coordinates of the ith pixel
     *
     * @param idx       a 1-D pixel index
     * 
     * @returns 2-D pixel coordinates
     */
    Vec2i getPixelCoordinates(int i);
    
private:
    // pixel data laid out linearly
    std::vector<Vec3f> data;

    // resolution of the image
    Vec2i res;
};

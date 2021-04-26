#include <pwos/common.h>
#include <pwos/image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

Image::Image(Vec2i res)
{
    data = vector<Vec3f>(res.x() * res.y());
}

Vec3f& Image::operator()(int idx)
{
    return data[idx];
}


Vec3f& Image::operator()(int x, int y)
{
    int idx = y * res.x() + x;
    return data[idx];
}

void Image::save(string filename)
{
    // TODO:
}

Vec2i Image::getRes()
{
    return res;
}

int Image::getNumPixels()
{
    return res.x() * res.y();
}


Vec2i Image::getPixelCoordinates(int i)
{
    return Vec2i(i % res.x(), i / res.x());
}

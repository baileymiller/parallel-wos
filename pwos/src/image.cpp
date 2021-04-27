#include <pwos/common.h>
#include <pwos/progressBar.h>
#include <pwos/image.h>
#include <pwos/stats.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

Image::Image(Vec2i res): res(res)
{
    data = vector<Vec3f>(res.x() * res.y());
}

void Image::set(int idx, Vec3f value)
{
    data[idx] = value;
}

Vec3f& Image::operator()(int x, int y)
{
    int idx = y * res.x() + x;
    return data[idx];
}

void Image::render(Vec4f window, int nthreads, Rand2DFunction f)
{   
    ProgressBar progress;
    progress.start(getNumPixels());

    #pragma omp parallel num_threads(nthreads)
    {
        size_t tid = omp_get_thread_num();
        pcg32 sampler = getSampler(tid);
        #pragma omp for 
        for (int i = 0; i < getNumPixels(); i++)
        {
            Stats::TIME_THREAD(tid, [this, i, f, window, &sampler]() -> void {
                Vec2i pixel = getPixelCoordinates(i);
                Vec2f coord = getXYCoords(pixel, window, res);
                set(i, f(coord, sampler));
            });
            #pragma omp critical
            {
                progress++;
            }
        }
    }
    progress.finish();
}

void Image::save(string filename)
{
    int i = 0;
    float* raw_data = new float[res.x() * res.y() * 3]();
    for (Vec3f rgb : data)
    {
        raw_data[i++] = rgb.x();
        raw_data[i++] = rgb.y();
        raw_data[i++] = rgb.z();
    }
    bool success = stbi_write_hdr(filename.append(".hdr").c_str(), res.x(), res.y(), 3, (const float*) raw_data);
    THROW_IF(!success, "Failed to write " + filename);
    delete raw_data;
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

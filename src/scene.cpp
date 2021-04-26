#include <pwos/common.h>
#include <pwos/scene.h>
#include <pwos/circle.h>

Scene::Scene(string filename)
{
    ifstream in(filename.c_str());
    THROW_IF(!in.is_open(), "Unable to open file " + filename);

    string line;
    string cell;
    stringstream ss;

    // get the first line (the window)
    getline(in, line);
    THROW_IF(line.empty(), "No lines in scene file to read, must have at least one line with window.");

    ss.clear();
    ss.str(line);
    for (int i = 0; i < window.size(); i++)
    {
        getline(ss, cell, ',');
        THROW_IF(cell.empty(), "First line must have at four numbers denoting window");
        window[i] = stof(cell);
    }

    // get circles (i.e. all of the remaining lines)
    while (getline(in, line))
    {
        string circle_idx = "Circle #" + to_string(circles.size());
        ss.clear();
        ss.str(line);

        // circle data
        Vec2f c;
        float r;
        Vec3f b;

        // get x, y coordinates
        getline(ss, cell, ',');
        THROW_IF(cell.empty(), circle_idx + " is missing x coordinate");
        c.x() = stof(cell);

        getline(ss, cell, ',');
        THROW_IF(cell.empty(), circle_idx + " is missing y coordinate");
        c.y() = stof(cell);

        // get radius
        getline(ss, cell, ',');
        THROW_IF(cell.empty(), circle_idx + " is missing radius");
        r = stof(cell);

        // get rgb boundary value
        getline(ss, cell, ',');
        THROW_IF(cell.empty(), circle_idx + " is missing boundary red value");
        b.x() = stof(cell);
        
        getline(ss, cell, ',');
        THROW_IF(cell.empty(), circle_idx + " is missing boundary green value");
        b.y() = stof(cell);
    
        getline(ss, cell, ',');
        THROW_IF(cell.empty(), circle_idx + " is missing boundary blue value");
        b.z() = stof(cell);

        // construct circle
        circles.push_back(make_shared<Circle>(c, r, b));
    }
}

Vec2f Scene::getClosestPoint(Vec2f o, Vec3f &b)
{
    // TODO: use KD tree to speed this up.
    float dist = std::numeric_limits<float>::max();
    Vec2f closestPoint;
    for (shared_ptr<Circle> c : circles)
    {
        Vec2f tempClosestPoint = c->getClosestPoint(o);
        float tempDist = (tempClosestPoint - o).norm();
        if (tempDist < dist)
        {
            dist = tempDist;
            closestPoint = tempClosestPoint;
            b = c->getBoundaryCondition();
        }
    }
    return closestPoint;
}

Vec4f Scene::getWindow()
{
    return window;
}

string Scene::getName()
{
    return name;
}



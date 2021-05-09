# Parallel Walk on Spheres
A tool kit for prototyping parallel Walk on Spheres methods. 

Boundary Condition | Boundary | Solution to Laplace PDE with Dirichlet Boundary Data
--- | --- | ---
![Boundary Condition](https://github.com/baileymiller/parallel-wos/blob/main/images/boundary_condition.jpg?raw=true) | ![Boundary](https://github.com/baileymiller/parallel-wos/blob/main/images/boundary.png?raw=true) | ![Solution](https://github.com/bailerymiller/parallel-wos/blob/main/images/solution.hdr?raw=true)

## Usage
There are two tools that can be used, the parallel walk on spheres renderer (pwos) or the scene genreation python script (generate_scene.py)
```
./pwos
    --integrator [Integrator Type wos, wog, mcwog, dist, gridviz, wogviz, or mcwogviz]
    --spp [Samples per pixel]
    --res [Output image width] [output image height]
    --cellsize [Controls the relative size of grid cells for integrators that use a pre-computed closest point query grid]
    [Scene File]
```

```
python3 ./generate_scene.py
    --i [Image file]
    --w [Width of the scene]
    --h [Height of the scene]
    --ncircs [Number of circles]
    --maxr [Max radius]
    --minr [Min radius]
    --xs [Extra space between circles]
    --o [Output file name]
    --odir [Output dir]
```


## Scene Files
Scene files are lists of circles with constant boundary colors. The first line of the scene file gives the scene "window" (i.e. bottom left and top right coordinates fo the scene)

```
min x, min y, max x, max y
x_0, y_0, radius_0, r_0, g_0, b_0
x_1, y_1, radius_1, r_1, g_1, b_1
...
x_n, y_n, radius_n, r_n, g_n, b_n
```
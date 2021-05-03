#!/usr/bin/env python

import argparse
import csv
import os
import math
import numpy as np
from random import random
from perlin_noise import PerlinNoise

from PIL import Image

class ImageWrapper:
    ''' Reads an image, then privdes access to the underlying pixel data'''

    def __init__(self, filename):
        '''Construct image from a filename'''        
        self.filename = filename
        self.image = Image.open(filename)
        self.pixels = self.image.load()
        self.width = self.image.size[0]
        self.height = self.image.size[1]

    def get(self, x, y):
        ''' Gets the value of the image in an RGB tuple at the position (x,y) for x,y \in [0, 1]'''        
        return tuple( v / 255.0 for v in self.pixels[x * float(self.width), self.height - y * float(self.height) - 1])

class RandomColor:
    ''' Use perlin noise to construct a random RGB color gradient. Each color channel has its own 
        perlin noise. All colors share a single value for octaves, but can have varying amplitudes.
    '''
    def __init__(self, octaves = 10, red_amplitude = 1, green_amplitude = 1, blue_amplitude = 1):
        '''Use perlin noise for red, green, and blue. Optionally set the amplitude for each'''        
        self.red_noise = PerlinNoise(octaves=octaves, seed=1)
        self.green_noise = PerlinNoise(octaves=octaves, seed=2)
        self.blue_noise = PerlinNoise(octaves=octaves, seed=3)
        self.red_amplitude = red_amplitude
        self.green_amplitude = green_amplitude
        self.blue_amplitude = blue_amplitude

    def get(self, x, y):
        ''' Gets random RGB tuple at the position (x,y) for x,y \in [0, 1]'''        
        return (
            self.red_noise([x, y]) * self.red_amplitude,
            self.green_noise([x, y]) * self.green_amplitude,
            self.blue_noise([x, y]) * self.blue_amplitude
        )

class Circle:
    '''
        Represents a circle with boundary data and 2D position. Used to construct a scene full of circles.
    '''
    def __init__(self, c, r, b = (1, 0, 0)):
        '''
            Attributes
            ------------
            c: np tuple
                the 2D coordinates specifying the center of the circle
            r: float
                the radius of the circle
            b: float tuplex3
                the r,g,b boundary value for the circle
        '''
        self.c = c
        self.r = r
        self. b = b
    
    def dist_to_circle(self, o):
        ''' Compute the distance from a point o to the closest point on a circle

            Note a distance less than 0 implies that o is within the circle.
        '''
        return np.linalg.norm(self.c - o) - self.r
    
    def __str__(self):
        ''' Stringify circle. '''
        data = [self.c[0], self.c[1], self.r, self.b[0], self.b[1], self.b[1]]
        data = [str(d) for d in data]
        return ",".join(data)

class SceneGenerator:
    ''' Generate a scene for dirichlet walk on spheres. Consists of circles with constant boundary conditions.

        There are two types of scenes "img" and "rand" (default)

        "img" will set the boundary condition of a circle at (x,y) based on the color of the corresponding pixel
        in the image that is provided.

        "rand" (the default) determines the boundary condition of each circle by looking up a perlin noise value
        for each of the red, green, and blue color channels. 

        Circles are non-overlapping, have a max and min size relative to the image (specified as an option during
        initialization).
    '''
    def __init__(self, opts = {}):
        '''
            Initialize SceneGenerator with the following options

            Initialization Options
            -----------
            opts["img"]:         
                -optional the image file used to set the boundary values

            opts["height"]
            opts["width"]
                -both required, if type is no "img" provided
                -none required if "img" provided, will either directly set both (both provided), set the missing
                value such that the proportions of "img" are maintained (one missing), or set the "height" and "width"
                to be equal to the "img" (both missing)
            
            opts["min_radius"]
            opts["max_radius"]
                -the min and max radius of a circle, as a fraction of the scene size.
                -defaults to 0.01 and 0.1 respectively
            
            opts["num_circles"]
                -number of circles to add to a scene, defaults to 20

            Attributes
            --------------
            values: object
                generated rgb pixel values, has a method .get(x, y) that generates an (r,g,b) tuple for x, y\in [0, 1]

            circles: list
                a list of circles contained in the scene
            
            width: float
                the width of the scene (not image width, just the width relative for circle positions / radii)
            
            height: float
                the height of the scene (not image height, just the width relative for circle positions / radii)
            
            min_radius: float
                the minimum radius of the a circle relative to the minimum scene dimension (i.e. min(width, height))
    
            max_radius: float
                the maximum radius of the a circle relative to the minimum scene dimension (i.e. min(width, height))
        '''
        self.circles = []

        self.values = lambda x, y: (1, 0, 0) 
        self.width = opts.get("width", 0)
        self.height = opts.get("height", 0)
        self.num_circles = opts.get("num_circles", 10)

        # set the values function and determine image width/heigth
        if "img" in opts:
            image = ImageWrapper(opts["img"])
            if "height" not in opts and "width" in opts:
                #set height to maintain image proportions
                self.height = image.height / float(image.width) * self.width
            elif "height" in opts and "width" not in opts:
                #set width to maintain image proportions
                self.width = image.width / float(image.height) * self.height
            elif "height" not in opts and "width" not in opts:
                #set height annd width to be exactly equal to image height/width
                self.height = image.height
                self.width = image.width

            self.values = image
        else:
            self.values = RandomColor(10, 0.5, 1, 2)

        # set min / max radius
        min_dimension = min(self.width, self.height)
        self.max_radius = opts.get("max_radius", 0.01) * min_dimension
        self.min_radius = opts.get("min_radius", 0.01) * min_dimension
        self.extra_spacing = opts.get("extra_spacing", 0.01) * min_dimension

        if self.width == 0 or self.height == 0:
            raise Exception("Width and height must be non-zero.")

    def get_max_radius(self, o):
        ''' Given a position o, determine the maximum radius r such that a circle with 
            radius r centered at o does not overlap with other circles
        '''

        # find the minimum R s.t. circle at o does not overlap
        maxR = float("inf")
        for circle in self.circles:
            tempR = circle.dist_to_circle(o)
            if tempR < 0:
                return 0
            elif tempR < maxR:
                maxR = tempR
        return maxR

    def generate_random_circle(self):
        '''
            Attempt to generate a random circle that doesn't overlap with existing circles.
        '''
        x = random()
        y = random()
        c = np.array([x * self.width, y * self.height])
        max_radius = self.get_max_radius(c)

        # take into account extra spacing that we require.
        max_radius -= self.extra_spacing

        # overlapping radius or not enough room considering extra spacing, return false
        if max_radius < 0:
            return False

        # if the max non-overlapping radius is greater than max radius for the scene, set it to max radius
        if max_radius > self.max_radius:
            max_radius = self.max_radius

        # random radius will be smaller than min        
        dr = max_radius - self.min_radius
        if dr < 0:
            return False

        r =  dr * random() + self.min_radius

        b = self.values.get(x, y)
        self.circles.append(Circle(c, r, b))
        return True

    def generate_scene(self, max_attempts = 25):
        '''
            Generate a scene with num_circle random circles. Only attempt to generate a circle 15 times before moving on.
        '''
        for i in range(self.num_circles):

            # reset num attempts
            num_attempts = 0

            # try up to max_attempts to generate a random circle that doesn't overlap with other circles
            while num_attempts < max_attempts:
                if self.generate_random_circle():
                    break
                num_attempts += 1
            
        print("Generated ", len(self.circles), " circles out of ", self.num_circles)

    def save_scene(self, filename):
        with open(filename, 'w') as f:
            # write window
            f.write(",".join(["0", "0", str(self.width), str(self.height)]))
            f.write('\n')

            # write a circle in each line
            for circle in self.circles:
                f.write(str(circle))
                f.write('\n')

if __name__ == "__main__":
    # parse command line args
    parser = argparse.ArgumentParser(description="Generate dirichlet laplace problems")
    parser.add_argument("--i", type=str, nargs="?", help="An image to get boundary values from")
    parser.add_argument("--w", type=float, nargs="?", help="The width of the scene ")
    parser.add_argument("--h", type=float, nargs="?", help="The height of the scene")
    parser.add_argument("--ncircs", type=int, nargs="?", help="Number of circles in scene")
    parser.add_argument("--maxr", type=float, nargs="?", help="Max radius relative to min scene dimension")
    parser.add_argument("--minr", type=float, nargs="?", help="Min radius relative to min scene dimension")
    parser.add_argument("--o", type=str, nargs="?", help="output filename")
    parser.add_argument("--odir", type=str, nargs="?", help="output dir")
    parser.add_argument("--xs", type=float, nargs="?", help="extra spacing")


    args = parser.parse_args()

    # set opts, remove empty values
    opts = {
        "img": args.i,
        "width": args.w,
        "height": args.h,
        "num_circles": args.ncircs,
        "maxr": args.maxr,
        "minr": args.minr,
        "extra_spacing": args.xs
    }
    opts = {k: v for k, v in opts.items() if v is not None}

    # generate scene
    scene_generator = SceneGenerator(opts)
    scene_generator.generate_scene()

    # save scene

    # determine base filename
    filename = "scenes/out.txt"
    if args.o == None and args.i:
        filename = os.path.basename(args.i)
        filename = os.path.splitext(filename)[0]
    elif args.o:
        filename = args.o

    # add some extra params (i.e. num circles) 
    filename += "_ncircs=" + str(len(scene_generator.circles)) + ".csv"
    if args.odir:
        filename = os.path.join(args.odir, filename)

    print("Saving scene file: ", filename)
    scene_generator.save_scene(filename)

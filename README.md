# Elevation Map

This code produces elevation map polydata from an array on a 2D VTK dataset using Delaunay triangulation.

The polydata can then be used to visualize the array field in Paraview. The process of generating output files are parallelized on shared memory over the pool of input files.

**Input:** The input can be one or series of VTK data files with iterable file names. The input file extensions should be ``vtk``, ``vti``, ``vtr``, ``vts``, ``vtp``, ``vtu``. The input file(s) should contain a VTK array (scalar or the component of a vector) defined on a 2D VTK grid (structured or unstructured).

**Output:** For each input file, a ``*.vtp`` polydata file is generated, in which the value of the input VTK array is used to generate an elevation map. This means the output is a 2D polydata surface where the ``x`` and ``y`` coordinates of its points are the same as the input file, and the ``z`` component of the elevated points is the value of the array on that point.

## Prerequisites

1. Install *GNU GCC* compiler, *OpenMP*, and *Cmake*:

       sudo apt-get install g++ libomp-dev cmake -y

2. Install *VTK* development:

       sudo apt install libvtk5-dev -y

   Alternatively, [download VTK](https://vtk.org/download/) and compile it.

## Install

Get the source code

    git clone git@github.com:ameli/elevation-map.git
    cd elevation-map
	
Compile in ``./build`` directory

    mkdir build
    cmake -B build .
    make -C build
	
After the compilation, the executable file is located in ``./bin/ElevationMap``.

## Usage

    ./bin/ElevationMap  InputBaseFilename  FileStartIndex  FileEndIndex  FileExtension  ArrayName

* ``InputBaseFilename``: is the input filename without the iterator and file extension.
* ``FileStartIndex`` and ``FileEndIndex``: the start and end of iterator on file name.
* ``FileExtension`` is one of ``vtk``, ``vti``, ``vtr``, ``vts``, ``vtp``, ``vtu``.
* ``ArrayName`` is the name of an array to be elevated.

## Example

    ./bin/ElevationMap  MyFile-  1  20  vtk  LE0

This example reads the array named ``LE0`` in each of the files ``MyFile-1.vtk`` to ``MyFile-20.vtk`` and generates polydata files ``MyFile-1.vtp`` to ``MyFile-20.vtp`` .

## License

BSD 3-clause.

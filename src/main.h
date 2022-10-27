/*
 * =====================================================================================
 *
 *       Filename:  main.h
 *
 *    Description:  Converts 2D Image Data to Elevation Maps
 *
 *        Version:  1.0
 *        Created:  09/11/2014 10:26:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Siavash Ameli
 *   Organization:  University Of California, Berkeley
 *
 * =====================================================================================
 */

#ifndef __ElevationMap_h
#define __ElevationMap_h

// ====================
// Forward Declarations
// ===================

#include <vtkSmartPointer.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>

// ====
// Type
// ====

typedef enum FileType
{
    VTK = 0,
    XML,
} FileType;

// ==========
// Prototypes
// ==========

void ParseArguments(
        int &argc,
        char *argv[],
        char *InputBaseFilename,        // Output
        unsigned int &FileStartIndex,   // Output
        unsigned int &FileEndIndex,     // Output
        char *InputFileExtension,       // Output
        char *ArrayName);               // Output

void PrintUsage(const char *ExecutableName);

FileType DetectFileType(const char *InputFileExtension);

vtkSmartPointer<vtkDataSet> Read(
        const char *InputFilename,
        FileType InputFileType);

vtkSmartPointer<vtkPolyData> CreateElevatedMap(
        vtkDataSet *InputDataSet,
        const char *ArrayName);

void GenerateInputFilename(
        const char *InputBaseFilename,
        unsigned int FileIterator,
        const char *InputFileExtension,
        char *InputFilename);   // Output

void GenerateOutputFilename(
        const char *InputFilename,
        char *OutputFilename);   // Output

#endif

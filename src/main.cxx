/*
 * ============================================================================
 *
 *       Filename:  main.cxx
 *
 *    Description:  Converts 2D Image Data to Elevation Maps
 *
 *        Version:  1.0
 *        Created:  09/02/2014 05:52:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Siavash Ameli
 *   Organization:  University Of California, Berkeley
 *
 * ============================================================================
 */


// =======
// Headers
// =======

// OpenMP
#ifdef _OPENMP
#include <omp.h>
#endif

// Code
#include "main.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkXMLGenericDataObjectReader.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkDelaunay2D.h>
#include <vtkXMLPolyDataWriter.h>

// STL
#include <sstream>


// ====
// Main
// ====

int main(int argc,char *argv[])
{
    // Parse arguments
    char InputBaseFilename[256];
    unsigned int FileStartIndex;
    unsigned int FileEndIndex;
    char InputFileExtension[256];
    char ArrayName[256];
    ParseArguments(
            argc, argv,
            InputBaseFilename,     // Output
            FileStartIndex,        // Output
            FileEndIndex,          // Output
            InputFileExtension,    // Output
            ArrayName);            // Output

    // Determine file type
    FileType InputFileType = DetectFileType(InputFileExtension);

    // Iterate over files
    #ifdef _OPENMP
    #pragma omp parallel for default(none) \
    shared(std::cout,FileStartIndex,FileEndIndex,InputBaseFilename, \
            InputFileExtension,ArrayName,InputFileType)
    #endif
    for(unsigned int FileIterator = FileStartIndex;
        FileIterator <= FileEndIndex;
        FileIterator++)
    {
        // Generate Input filename
        char InputFilename[256];
        GenerateInputFilename(
                InputBaseFilename,
                FileIterator,
                InputFileExtension,
                InputFilename);   // Output

        // Read File
        vtkSmartPointer<vtkDataSet> InputDataSet = Read(
                InputFilename,InputFileType);

        // Create Elevated PolyData
        vtkSmartPointer<vtkPolyData> OutputPolyData = CreateElevatedMap(
                InputDataSet,ArrayName);

        // Generate Output Filename
        char OutputFilename[256];
        GenerateOutputFilename(InputFilename,OutputFilename);

        // Write Output PolyData
        vtkSmartPointer<vtkXMLPolyDataWriter> Writer = \
                vtkSmartPointer<vtkXMLPolyDataWriter>::New();
        #if VTK_MAJOR_VERSION <= 5
            Writer->SetInput(OutputPolyData);
        #else
            Writer->SetInputData(OutputPolyData);
        #endif
        Writer->SetFileName(OutputFilename);
        Writer->Update();

        std::cout << InputFilename << " converted to ";
        std::cout << OutputFilename << "." << std::endl;
    }
    
    return EXIT_SUCCESS;
}


// ===============
// Parse Arguments
// ===============

void ParseArguments(
        int &argc,
        char *argv[],
        char *InputBaseFilename,         // Output
        unsigned int &FileStartIndex,    // Output
        unsigned int &FileEndIndex,      // Output
        char *InputFileExtension,        // Output
        char *ArrayName)                 // Output
{
    if(argc < 6)
    {
        PrintUsage(argv[0]);
        exit(0);
    }
    else
    {
        strcpy(InputBaseFilename,argv[1]);
        FileStartIndex = atoi(argv[2]);
        FileEndIndex = atoi(argv[3]);
        strcpy(InputFileExtension,argv[4]);
        strcpy(ArrayName,argv[5]);
    }

    // Check Inputs
    if(strlen(InputBaseFilename) < 1)
    {
        std::cerr << "InputBaseFilename is empty." << std::endl;
        exit(0);
    }
    else if(FileStartIndex > FileEndIndex)
    {
        std::cerr << "FileStartIndex should be less than or equal to ";
        std::cerr << "FileEndIndex." << std::endl;
        exit(0);
    }
    else if(strlen(ArrayName) < 1)
    {
        std::cerr << "ArrayName is empty." << std::endl;
        exit(0);
    }
}

// ===========
// Print Usage
// ===========

void PrintUsage(const char *ExecutableName)
{
    std::cerr << "Usage: " << ExecutableName << "  InputBaseFilename  ";
    std::cerr << "FileStartIndex  FileEndIndex  FileExtension  ArrayName";
    std::cerr << std::endl;

    std::cerr << "\nInputBaseFilename: is the input filename without the ";
    std::cerr << "iterator and file extension." << std::endl;
    std::cerr << "FileStartIndex and FileEndIndex: the start and end of ";
    std::cerr << "iterator on file name." << std::endl;
    std::cerr << "FileExtension is one of vtk, vti, vtr, vts, vtp, vtu.";
    std::cerr << std::endl;

    std::cerr << "\nExample: " << ExecutableName << "  MyFile-  1  20  vtk  ";
    std::cerr << "LE0" << std::endl;
    std::cerr << "The example reads files MyFile-1.vtk to MyFile-20.vtk and ";
    std::cerr << std::endl;
    std::cerr << "generates polydata files MyFile-1.vtp to MyFile-20.vtp.";
    std::cerr << std::endl;
}


// ================
// Detect File Type
// ================

// Description:
// This function determines if the input file is legacy VTK file or XML file.

FileType DetectFileType(const char *InputFileExtension)
{
    FileType InputFileType;

    if(strcmp(InputFileExtension,"vtk") == 0)
    {
        InputFileType = VTK;
    }
    else if(strcmp(InputFileExtension,"vti") == 0 ||
            strcmp(InputFileExtension,"vtp") == 0 ||
            strcmp(InputFileExtension,"vtu") == 0 ||
            strcmp(InputFileExtension,"vtr") == 0 ||
            strcmp(InputFileExtension,"vts") == 0)
    {
        InputFileType = XML;
    }
    else
    {
        std::cerr << "File type: " << InputFileType << " is not supported. ";
        std::cerr << "Program terminated." << std::endl;
        exit(0);
    }

    return InputFileType;
}


// ====
// Read
// ====

vtkSmartPointer<vtkDataSet> Read(
        const char *InputFilename,
        FileType InputDataType)   // Output
{
    vtkSmartPointer<vtkDataSet> InputDataSet;

    switch(InputDataType)
    {
        // VTK
        case VTK:
        {
            // Read file
            vtkSmartPointer<vtkGenericDataObjectReader> Reader = \
                    vtkSmartPointer<vtkGenericDataObjectReader>::New();
            Reader->SetFileName(InputFilename);
            Reader->Update();

            // Get DataSet
            InputDataSet = vtkDataSet::SafeDownCast(Reader->GetOutput());

            break;
        }

        // XML
        case XML:
        {
            // Read file
            vtkSmartPointer<vtkXMLGenericDataObjectReader> Reader = \
                    vtkSmartPointer<vtkXMLGenericDataObjectReader>::New();
            Reader->SetFileName(InputFilename);
            Reader->Update();

            // Get DataSet
            InputDataSet = vtkDataSet::SafeDownCast(Reader->GetOutput());

            break;
        }

        // Invalid file type
        default:
        {
            std::cerr << "Invalid file type. Program terminated." << std::endl;
            exit(0);
        }
    }

    // Check output
    if(InputDataSet == NULL)
    {
        std::cerr << "Input DataSet is NULL. Program terminated." << std::endl;
        exit(0);
    }

    return InputDataSet;
}


// ===================
// Create Elevated Map
// ===================

// Description
// InputDataSet is a 2D grid that contains a scalar field. The OutputPolyData
// is a 3D grid that x and y component of points are the same as the input grid
// but the z component is the scalar field value. The output also contains the
// same scalar field as well.

vtkSmartPointer<vtkPolyData> CreateElevatedMap(
        vtkDataSet *InputDataSet,
        const char *ArrayName)
{
    // Get Scalar Field
    vtkSmartPointer<vtkDataArray> ScalarField = \
            InputDataSet->GetPointData()->GetArray(ArrayName);
    if(ScalarField == NULL)
    {
        std::cerr << "Scalar field is NULL." << std::endl;
        exit(0);
    }
    else if(ScalarField->GetNumberOfTuples() < 1)
    {
        std::cerr << "Scalar field has no tuples." << std::endl;
        exit(0);
    }

    // Number of points
    unsigned int NumberOfPoints = InputDataSet->GetNumberOfPoints();
    if(NumberOfPoints < 1)
    {
        std::cerr << "No point found in input data set." << std::endl;
        exit(0);
    }

    // Elevated Points
    vtkSmartPointer<vtkPoints> Points = vtkSmartPointer<vtkPoints>::New();
    Points->SetNumberOfPoints(NumberOfPoints);

    // Iterate over points
    // #ifdef _OPENMP
    // #pragma omp parallel for default(none) \
    // shared(std::cout,NumberOfPoints,InputDataSet,ScalarField,Points,k)
    // #endif
    for(unsigned int PointId = 0; PointId < NumberOfPoints; PointId++)
    {
        // Get Point Coordinates (Thread safe version)
        #ifdef _OPENMP
            double PointCoordinate[3];
            InputDataSet->GetPoint(PointId,PointCoordinate);
        #else
            double *PointCoordinate = InputDataSet->GetPoint(PointId);
        #endif

        // Get Elevation (Thread safe version)
        double ScalarValue[1];
        ScalarField->GetTuple(PointId,ScalarValue);

        // Store point (Thread safe version)
        Points->SetPoint(PointId,PointCoordinate[0],PointCoordinate[1],
                         ScalarValue[0]);
    }

    // Add points to PolyData
    vtkSmartPointer<vtkPolyData> ElevatedPolyData = \
            vtkSmartPointer<vtkPolyData>::New();
    ElevatedPolyData->SetPoints(Points);

    // Delaunay PolyData
    vtkSmartPointer<vtkDelaunay2D> Delaunay = \
            vtkSmartPointer<vtkDelaunay2D>::New();
    #if VTK_MAJOR_VERSION <= 5
        Delaunay->SetInput(ElevatedPolyData);
    #else
        Delaunay->SetInputData(ElevatedPolyData);
    #endif
    Delaunay->Update();

    // Output PolyData
    vtkSmartPointer<vtkPolyData> OutputPolyData = Delaunay->GetOutput();
    OutputPolyData->GetPointData()->AddArray(ScalarField);

    return OutputPolyData;
}


// =======================
// Generate Input Filename
// =======================

void GenerateInputFilename(
        const char *InputBaseFilename,
        unsigned int FileIterator,
        const char *InputFileExtension,
        char *InputFilename)   // Output
{
    std::ostringstream InputFilenameStream;
    std::string InputBaseFilenameString(InputBaseFilename);
    InputFilenameStream << InputBaseFilename <<  FileIterator << "." << \
        InputFileExtension;
    strcpy(InputFilename,InputFilenameStream.str().c_str());
}


// ========================
// Generate Output Filename
// ========================

void GenerateOutputFilename(
        const char *InputFilename,
        char *OutputFilename)   // Output
{
    std::string InputFilenameString(InputFilename);
    std::string InputBaseFilenameString;

    // Find the last dot position
    size_t FoundDot = InputFilenameString.find_last_of(".");
    if(FoundDot != std::string::npos)
    {
        InputBaseFilenameString = InputFilenameString.substr(0,FoundDot);
    }
    else
    {
        std::cerr << "Not dot can be find in the input file name." << std::endl;
        exit(0);
    }

    std::string OutputFilenameString = "ElevatedMap-" + \
            InputBaseFilenameString + ".vtp";
    strcpy(OutputFilename,OutputFilenameString.c_str());
}

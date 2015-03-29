// Author: Mingcheng Chen (linyufly@gmail.com)

#include "mesh_trimmer.h"

#include <set>

#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>

namespace {

const char *kInputFile = "/home/linyufly/GitLab/RidgeExtraction/watershed_ridges.vtk";
const char *kOutputFile = "trimmed_mesh.vtk";

const double kDistance = 1.53;

double square(double value) {
  return value * value;
}

struct SimpleJudge {
  static vtkPolyData *mesh_;

  bool operator () (int point) const {
    double coord[3];
    mesh_->GetPoints()->GetPoint(point, coord);

    return square(coord[0]) + square(coord[1]) > square(kDistance)
           || coord[2] < 0.15 && square(coord[0]) + square(coord[1]) > square(kDistance * 0.8)
           || coord[2] > 3.05 && square(coord[0]) + square(coord[1]) > square(kDistance * 0.95);
  }
};

vtkPolyData *SimpleJudge::mesh_ = NULL;

}

void trim_test() {
  printf("trim_test {\n");

  vtkSmartPointer<vtkPolyDataReader> reader =
      vtkSmartPointer<vtkPolyDataReader>::New();
  reader->SetFileName(kInputFile);
  reader->Update();

  vtkSmartPointer<vtkPolyData> mesh =
      vtkSmartPointer<vtkPolyData>::New();
  mesh->DeepCopy(reader->GetOutput());

  SimpleJudge::mesh_ = mesh;

  vtkSmartPointer<vtkPolyData> trimmed =
      vtkSmartPointer<vtkPolyData>(MeshTrimmer<SimpleJudge>::trim(mesh));

  vtkSmartPointer<vtkPolyDataWriter> writer =
      vtkSmartPointer<vtkPolyDataWriter>::New();
  writer->SetFileName(kOutputFile);
  writer->SetInputData(trimmed);
  writer->Write();

  printf("} trim_test\n");
}

int main() {
  trim_test();

  return 0;
}


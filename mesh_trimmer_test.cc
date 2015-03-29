// Author: Mingcheng Chen (linyufly@gmail.com)

#include "mesh_trimmer.h"

#include <set>

#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>

namespace {

const char *kInputFile = "poly_mesh.vtk";
const char *kOutputFile = "trimmed_mesh.vtk";

struct SimpleJudge {
  bool operator () (int a) const {
    return a < 100000;
  }
};

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


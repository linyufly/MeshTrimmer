// Author: Mingcheng Chen (linyufly@gmail.com)

#include "mesh_trimmer.h"

#include <queue>
#include <set>
#include <vector>

#include <vtkDataArray.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>

namespace {

// const char *kInputFile = "/home/linyufly/GitLab/RidgeExtraction/watershed_ridges.vtk";
// const char *kInputFile = "/home/linyufly/GitHub/Playground/interpolate.vtk";
const char *kInputFile = "poly_mesh.vtk";
const char *kOutputFile = "trimmed_mesh.vtk";

const double kDistance = 1.53;
const double kThreshold = 0.02;
const int kComponentThreshold = 100000;

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

struct ThresholdJudge {
  static vtkPolyData *mesh_;

  bool operator () (int point) const {
    return mesh_->GetPointData()->GetScalars("FTLE")->GetTuple1(point) < kThreshold;
  }
};

struct RemainingJudge {
  static std::set<int> *remaining_;

  bool operator () (int point) const {
    return remaining_->find(point) == remaining_->end();
  }
};

vtkPolyData *SimpleJudge::mesh_ = NULL;
vtkPolyData *ThresholdJudge::mesh_ = NULL;
std::set<int> *RemainingJudge::remaining_ = NULL;

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
  ThresholdJudge::mesh_ = mesh;

  vtkSmartPointer<vtkPolyData> trimmed =
      vtkSmartPointer<vtkPolyData>(MeshTrimmer<ThresholdJudge>::trim(mesh));

  vtkSmartPointer<vtkPolyDataWriter> writer =
      vtkSmartPointer<vtkPolyDataWriter>::New();
  writer->SetFileName(kOutputFile);
  writer->SetInputData(trimmed);
  writer->Write();

  printf("} trim_test\n");
}

void component_trim_test() {
  printf("component_trim_test {\n");

  vtkSmartPointer<vtkPolyDataReader> reader =
      vtkSmartPointer<vtkPolyDataReader>::New();
  reader->SetFileName(kInputFile);
  reader->Update();

  vtkSmartPointer<vtkPolyData> mesh =
      vtkSmartPointer<vtkPolyData>::New();
  mesh->DeepCopy(reader->GetOutput());

  int num_points = mesh->GetPoints()->GetNumberOfPoints();
  std::vector<std::set<int> > links(num_points);

  mesh->GetPolys()->InitTraversal();
  for (int c = 0; c < mesh->GetPolys()->GetNumberOfCells(); c++) {
    vtkSmartPointer<vtkIdList> id_list = vtkSmartPointer<vtkIdList>::New();
    mesh->GetPolys()->GetNextCell(id_list);

    std::vector<int> points;
    for (int i = 0; i < id_list->GetNumberOfIds(); i++) {
      points.push_back(id_list->GetId(i));
    }

    for (int i = 0; i < points.size(); i++) {
      int u = points[i];
      int v = points[(i + 1) % points.size()];
      links[u].insert(v);
      links[v].insert(u);
    }
  }

  std::vector<bool> visited(num_points);
  std::set<int> remaining;

  std::fill(visited.begin(), visited.end(), false);

  for (int p = 0; p < num_points; p++) {
    if (!visited[p]) {
      std::vector<int> component;
      std::queue<int> bfs_queue;
      bfs_queue.push(p);
      visited[p] = true;
      component.push_back(p);
      while (!bfs_queue.empty()) {
        int curr = bfs_queue.front();
        bfs_queue.pop();
        for (std::set<int>::iterator itr = links[curr].begin();
             itr != links[curr].end(); ++itr) {
          if (!visited[*itr]) {
            component.push_back(*itr);
            bfs_queue.push(*itr);
            visited[*itr] = true;
          }
        }
      }

      if (component.size() > kComponentThreshold) {
        for (int p = 0; p < component.size(); p++) {
          remaining.insert(component[p]);
        }
      }
    }
  }

  RemainingJudge::remaining_ = &remaining;

  vtkSmartPointer<vtkPolyData> trimmed =
      vtkSmartPointer<vtkPolyData>(MeshTrimmer<RemainingJudge>::trim(mesh));

  vtkSmartPointer<vtkPolyDataWriter> writer =
      vtkSmartPointer<vtkPolyDataWriter>::New();
  writer->SetFileName(kOutputFile);
  writer->SetInputData(trimmed);
  writer->Write(); 

  printf("} component_trim_test\n");
}

int main() {
  // trim_test();
  component_trim_test();

  return 0;
}


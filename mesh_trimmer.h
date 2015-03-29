// Author: Mingcheng Chen (linyufly@gmail.com)

#ifndef MESH_TRIMMER_H_
#define MESH_TRIMMER_H_

#include <algorithm>
#include <vector>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// T(i) defines whether the i-th point should be trimmed.
template<class T>
class MeshTrimmer {
 public:
  static vtkPolyData *trim(vtkPolyData *mesh) {
    vtkPolyData *new_mesh = vtkPolyData::New();

    int num_points = mesh->GetPoints()->GetNumberOfPoints();

    std::vector<int> remaining;

    T judge;

    for (int p = 0; p < num_points; p++) {
      if (!judge(p)) {
        remaining.push_back(p);
      }
    }

    vtkSmartPointer<vtkPoints> points =
        vtkSmartPointer<vtkPoints>::New();
    for (int r = 0; r < remaining.size(); r++) {
      int point = remaining[r];
      double curr_coord[3];
      mesh->GetPoints()->GetPoint(point, curr_coord);
      points->InsertNextPoint(curr_coord);
    }

    new_mesh->SetPoints(points);

    for (int arr = 0; arr < mesh->GetPointData()->GetNumberOfArrays(); arr++) {
      vtkDataArray *old_arr = mesh->GetPointData()->GetArray(arr);
      vtkSmartPointer<vtkDataArray> new_arr =
          vtkSmartPointer<vtkDataArray>(old_arr->NewInstance());
      new_arr->SetName(old_arr->GetName());
      new_arr->SetNumberOfComponents(old_arr->GetNumberOfComponents());
      new_arr->SetNumberOfTuples(remaining.size());

      for (int r = 0; r < remaining.size(); r++) {
        int point = remaining[r];
        std::vector<double> curr_values(new_arr->GetNumberOfComponents());
        old_arr->GetTuple(point, &curr_values[0]);
        new_arr->SetTuple(point, &curr_values[0]);
      }

      new_mesh->GetPointData()->AddArray(new_arr);
    }

    std::vector<int> new_index(num_points);
    std::fill(new_index.begin(), new_index.end(), -1);
    for (int r = 0; r < remaining.size(); r++) {
      new_index[remaining[r]] = r;
    }

    for (int arr = 0; arr < mesh->GetCellData()->GetNumberOfArrays(); arr++) {
      vtkDataArray *old_arr = mesh->GetCellData()->GetArray(arr);
      vtkSmartPointer<vtkDataArray> new_arr =
          vtkSmartPointer<vtkDataArray>(old_arr->NewInstance());
      new_arr->SetName(old_arr->GetName());
      new_arr->SetNumberOfComponents(old_arr->GetNumberOfComponents());

      new_mesh->GetCellData()->AddArray(new_arr);
    }

    vtkSmartPointer<vtkCellArray> new_polys =
        vtkSmartPointer<vtkCellArray>::New();
    mesh->GetPolys()->InitTraversal();
    for (int c = 0; c < mesh->GetPolys()->GetNumberOfCells(); c++) {
      vtkSmartPointer<vtkIdList> id_list = vtkSmartPointer<vtkIdList>::New();
      mesh->GetPolys()->GetNextCell(id_list);

      bool trimmed = false;
      for (int i = 0; i < id_list->GetNumberOfIds(); i++) {
        int point = id_list->GetId(i);
        if (new_index[point] == -1) {
          trimmed = true;
          break;
        }
      }
      if (trimmed) {
        continue;
      }

      new_polys->InsertNextCell(id_list->GetNumberOfIds());
      for (int i = 0; i < id_list->GetNumberOfIds(); i++) {
        new_polys->InsertCellPoint(new_index[id_list->GetId(i)]);
      }

      for (int arr = 0; arr < mesh->GetCellData()->GetNumberOfArrays(); arr++) {
        vtkDataArray *old_arr = mesh->GetCellData()->GetArray(arr);
        vtkDataArray *new_arr = new_mesh->GetCellData()->GetArray(arr);
        std::vector<double> curr_values(new_arr->GetNumberOfComponents());
        old_arr->GetTuple(c, &curr_values[0]);
        new_arr->InsertNextTuple(&curr_values[0]);
      }
    }

    new_mesh->SetPolys(new_polys);

    return new_mesh;
  }
};

#endif  // MESH_TRIMMER_H_


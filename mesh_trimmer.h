// Author: Mingcheng Chen (linyufly@gmail.com)

#ifndef MESH_TRIMMER_H_
#define MESH_TRIMMER_H_

#include <vector>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// T(i) defines whether the i-th point should be trimmed.
template<class T>
class MeshTrimmer {
 public:
  static vtkPolyData *trim(vtkPolyData *mesh) {
    int num_points = mesh->GetPoints()->GetNumberOfPoints();

    std::vector<int> remaining;

    T judge;

    for (int p = 0; p < num_points; p++) {
      if (!judge(p)) {
        remaining.push_back(p);
      }
    }

    return NULL;
  }
};

#endif  // MESH_TRIMMER_H_


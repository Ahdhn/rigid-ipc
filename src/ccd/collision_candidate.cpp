#include "collision_candidate.hpp"

#include <fmt/format.h>

namespace ccd {

EdgeVertexCandidate::EdgeVertexCandidate(long edge_index, long vertex_index)
    : edge_index(edge_index)
    , vertex_index(vertex_index)
{
}

bool EdgeVertexCandidate::operator==(const EdgeVertexCandidate& other) const
{
    return edge_index == other.edge_index && vertex_index == other.vertex_index;
}

bool EdgeVertexCandidate::operator<(const EdgeVertexCandidate& other) const
{
    if (edge_index == other.edge_index) {
        return vertex_index < other.vertex_index;
    }
    return edge_index < other.edge_index;
}

EdgeEdgeCandidate::EdgeEdgeCandidate(long edge0_index, long edge1_index)
    : edge0_index(edge0_index)
    , edge1_index(edge1_index)
{
}

bool EdgeEdgeCandidate::operator==(const EdgeEdgeCandidate& other) const
{
    // (i, j) == (i, j) || (i, j) == (j, i)
    return (this->edge0_index == other.edge0_index
            && this->edge1_index == other.edge1_index)
        || (this->edge0_index == other.edge1_index
            && this->edge1_index == other.edge0_index);
}

bool EdgeEdgeCandidate::operator<(const EdgeEdgeCandidate& other) const
{
    long this_min = std::min(this->edge0_index, this->edge1_index);
    long other_min = std::min(other.edge0_index, other.edge1_index);
    if (this_min == other_min) {
        return std::max(this->edge0_index, this->edge1_index)
            < std::max(other.edge0_index, other.edge1_index);
    }
    return this_min < other_min;
}

EdgeFaceCandidate::EdgeFaceCandidate(long edge_index, long face_index)
    : edge_index(edge_index)
    , face_index(face_index)
{
}

bool EdgeFaceCandidate::operator==(const EdgeFaceCandidate& other) const
{
    return edge_index == other.edge_index && face_index == other.face_index;
}

bool EdgeFaceCandidate::operator<(const EdgeFaceCandidate& other) const
{
    if (edge_index == other.edge_index) {
        return face_index < other.face_index;
    }
    return edge_index < other.edge_index;
}

FaceVertexCandidate::FaceVertexCandidate(long face_index, long vertex_index)
    : face_index(face_index)
    , vertex_index(vertex_index)
{
}

bool FaceVertexCandidate::operator==(const FaceVertexCandidate& other) const
{
    return face_index == other.face_index && vertex_index == other.vertex_index;
}

bool FaceVertexCandidate::operator<(const FaceVertexCandidate& other) const
{
    if (face_index == other.face_index) {
        return vertex_index < other.vertex_index;
    }
    return face_index < other.face_index;
}

} // namespace ccd

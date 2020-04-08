// A spatial hash grid for rigid bodies with angular trajectories.
#include "rigid_body_hash_grid.hpp"

#include <cmath>
#include <iostream>

#include <ccd/interval.hpp>
#include <logger.hpp>

namespace ccd {

void compute_vertices_intervals(
    const physics::RigidBodyAssembler& bodies,
    const physics::Poses<double>& poses_t0,
    const physics::Poses<double>& poses_t1,
    Eigen::MatrixX<Interval>& vertices)
{
    Interval t(0, 1);
    std::vector<Eigen::MatrixXX3<Interval>> Rs(bodies.num_bodies());
    std::vector<Eigen::VectorX3<Interval>> ps(bodies.num_bodies());

    physics::Poses<Interval> posesI_t0 =
        physics::cast<double, Interval>(poses_t0);
    physics::Poses<Interval> posesI_t1 =
        physics::cast<double, Interval>(poses_t1);

    tbb::parallel_for(size_t(0), bodies.num_bodies(), [&](size_t i) {
        ps[i] = (posesI_t1[i].position - posesI_t0[i].position) * t
            + posesI_t0[i].position;

        if (bodies.dim() == 2) {
            Rs[i] = Eigen::Rotation2D<Interval>(
                        ((posesI_t1[i].rotation - posesI_t0[i].rotation) * t
                         + posesI_t0[i].rotation)(0))
                        .toRotationMatrix();
        } else {
            Eigen::Matrix3d R0, P;
            double omega;
            decompose_to_z_screwing(poses_t0[i], poses_t1[i], R0, P, omega);
            Eigen::Matrix3I Rz = rotate_around_z(t * omega);
            Eigen::Matrix3I P_I = P.template cast<Interval>();
            Rs[i] = P_I.transpose() * Rz * P_I * R0.cast<Interval>();
        }
    });

    vertices = bodies.world_vertices(Rs, ps);
}

void RigidBodyHashGrid::resize(
    const physics::RigidBodyAssembler& bodies,
    const physics::Poses<double>& poses_t0,
    const physics::Poses<double>& poses_t1,
    const double inflation_radius)
{
    Eigen::MatrixX<Interval> vertices;
    compute_vertices_intervals(bodies, poses_t0, poses_t1, vertices);

    Eigen::VectorX3<Interval> mesh_extents(bodies.dim());
    double average_displacement_length = 0;
    for (int i = 0; i < vertices.rows(); i++) {
        double max_side_width = 0;
        for (int j = 0; j < vertices.cols(); j++) {
            mesh_extents(j) = i == 0
                ? vertices(i, j)
                : boost::numeric::hull(mesh_extents(j), vertices(i, j));
            max_side_width =
                std::max(max_side_width, boost::numeric::width(vertices(i, j)));
        }
        average_displacement_length += max_side_width;
    }
    Eigen::VectorX3d min(mesh_extents.size()), max(mesh_extents.size());
    for (int i = 0; i < mesh_extents.size(); i++) {
        min(i) = mesh_extents(i).lower() - inflation_radius;
        max(i) = mesh_extents(i).upper() + inflation_radius;
    }

    average_displacement_length /= vertices.rows();
    double average_edge_length = bodies.average_edge_length;

    double cell_size =
        std::max(average_displacement_length, average_edge_length)
        + inflation_radius;
    HashGrid::resize(min, max, cell_size);
}

void RigidBodyHashGrid::addBodies(
    const physics::RigidBodyAssembler& bodies,
    const physics::Poses<double>& poses_t0,
    const physics::Poses<double>& poses_t1,
    const double inflation_radius)
{
    assert(bodies.num_bodies() == poses_t0.size());
    Eigen::MatrixX<Interval> vertices;
    compute_vertices_intervals(bodies, poses_t0, poses_t1, vertices);

    // Create a bounding box for all vertices
    tbb::concurrent_vector<AABB> vertices_aabb;
    Eigen::VectorX3d min(bodies.dim()), max(bodies.dim());
    vertices_aabb.reserve(vertices.rows());
    for (long i = 0; i < vertices.rows(); i++) {
        for (int j = 0; j < vertices.cols(); j++) {
            min(j) = vertices(i, j).lower() - inflation_radius;
            max(j) = vertices(i, j).upper() + inflation_radius;
        }
        vertices_aabb.emplace_back(min, max);
    }

    // Add vertice, edges, and faces in parallel
    tbb::parallel_invoke(
        [&] {
            // Add all vertices of the bodies
            for (long i = 0; i < vertices.rows(); i++) {
                this->addElement(vertices_aabb[i], i, this->m_vertexItems);
            }
        },
        [&] {
            // Add all edge of the bodies
            for (long i = 0; i < bodies.m_edges.rows(); i++) {
                this->addElement(
                    AABB(
                        vertices_aabb[bodies.m_edges(i, 0)],
                        vertices_aabb[bodies.m_edges(i, 1)]),
                    i, this->m_edgeItems);
            }
        },
        [&] {
            // Add all faces of the bodies
            for (long i = 0; i < bodies.m_faces.rows(); i++) {
                this->addElement(
                    AABB(
                        AABB(
                            vertices_aabb[bodies.m_faces(i, 0)],
                            vertices_aabb[bodies.m_faces(i, 1)]),
                        vertices_aabb[bodies.m_faces(i, 2)]),
                    i, this->m_faceItems);
            }
        });
}

} // namespace ccd
#pragma once

#include <Eigen/Core>

#include <ccd/collision_detection.hpp>
#include <opt/collision_constraint.hpp>

namespace ccd {

namespace opt {

    class VolumeConstraint : public CollisionConstraint {
    public:
        VolumeConstraint();
        VolumeConstraint(const std::string& name);

        void settings(const nlohmann::json& json) override;
        nlohmann::json settings() const override;

        EdgeVertexImpacts initialize(const Eigen::MatrixX2d& vertices,
            const Eigen::MatrixX2i& edges,
            const Eigen::VectorXi& group_ids,
            const Eigen::MatrixXd& Uk) override;

        EdgeEdgeImpacts get_ee_collision_set(const Eigen::MatrixXd& Uk);

        void compute_constraints(const Eigen::MatrixXd& Uk,
            const EdgeEdgeImpacts& ee_impacts,
            Eigen::VectorXd& g_uk);

        void compute_constraints_jacobian(const Eigen::MatrixXd& Uk,
            const EdgeEdgeImpacts& ee_impacts,
            Eigen::SparseMatrix<double>& jac_uk);

        void compute_constraints(const Eigen::MatrixXd& Uk,
            const EdgeEdgeImpacts& ee_impacts,
            Eigen::VectorXd& g_uk,
            Eigen::SparseMatrix<double>& g_uk_jacobian,
            Eigen::VectorXi& g_uk_active);

        // This versions of the functions get their own ee_impacts;
        void compute_constraints(
            const Eigen::MatrixXd& Uk, Eigen::VectorXd& g_uk);

        void compute_constraints_jacobian(
            const Eigen::MatrixXd& Uk, Eigen::SparseMatrix<double>& jac_uk);

        void compute_constraints(const Eigen::MatrixXd& Uk,
            Eigen::VectorXd& g_uk,
            Eigen::SparseMatrix<double>& g_uk_jacobian,
            Eigen::VectorXi& g_uk_active);

        int number_of_constraints();

        // Settings
        // ----------
        double volume_epsilon;
        int num_constraints;

        void dense_indices(
            const EdgeEdgeImpacts& ee_impacts, Eigen::VectorXi& dense_indices);

        /// @brief #E,1 indices of the edges' first impact
        Eigen::VectorXi m_edge_impact_map;
    };

    long get_constraint_index(
        const EdgeEdgeImpact& impact, const bool impacted, const int num_edges);

    long get_constraints_size(const int num_edges);

} // namespace opt
} // namespace ccd

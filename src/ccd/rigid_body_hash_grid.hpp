#pragma once

#include <ccd/hash_grid.hpp>
#include <physics/rigid_body_assembler.hpp>

namespace ccd {

class RigidBodyHashGrid : public HashGrid {
public:
    void resize(
        const physics::RigidBodyAssembler& bodies,
        const std::vector<physics::Pose<double>>& poses,
        const std::vector<physics::Pose<double>>& displacements,
        const double inflation_radius = 0.0);

    void addBodies(
        const physics::RigidBodyAssembler& bodies,
        const std::vector<physics::Pose<double>>& poses,
        const std::vector<physics::Pose<double>>& displacements,
        const double inflation_radius = 0.0);
};

} // namespace ccd

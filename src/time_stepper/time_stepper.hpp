#pragma once

#include <Eigen/Core>

#include <logger.hpp>
#include <physics/pose.hpp>
#include <physics/rigid_body_assembler.hpp>
#include <utils/eigen_ext.hpp>
#include <utils/not_implemented_error.hpp>

namespace ccd {
namespace time_stepper {

    class TimeStepper {
    public:
        virtual ~TimeStepper() = default;

        /**
         * @brief Take a single time step.
         *
         * @param bodies     Rigid body
         * @param gravity    Acceleration due to gravity
         * @param time_step  Timestep
         */
        virtual void step(
            physics::RigidBody& body,
            const Eigen::VectorX3d& gravity,
            const double& time_step) const
        {
            switch (body.dim()) {
            case 2:
                return step2D(body, gravity, time_step);
            case 3:
                return step3D(body, gravity, time_step);
            default:
                throw NotImplementedError("Invalid dim for timestepper!");
            }
        }

        /**
         * @brief Take a single time step.
         *
         * @param bodies     Rigid bodies
         * @param gravity    Acceleration due to gravity
         * @param time_step  Timestep
         */
        virtual void step(
            physics::RigidBodyAssembler& bodies,
            const Eigen::VectorX3d& gravity,
            const double& time_step) const
        {
            switch (bodies.dim()) {
            case 2:
                return step2D(bodies, gravity, time_step);
            case 3:
                return step3D(bodies, gravity, time_step);
            default:
                throw NotImplementedError("Invalid dim for timestepper!");
            }
        }

        virtual std::string name() const = 0;

    protected:
        /**
         * @brief Take a single time step.
         *
         * @param bodies     Rigid body
         * @param gravity    Acceleration due to gravity
         * @param time_step  Timestep
         */
        virtual void step2D(
            physics::RigidBody& body,
            const Eigen::Vector2d& gravity,
            const double& time_step) const
        {
            throw NotImplementedError(
                fmt::format("Time-stepper {} not implemented in 2D!", name()));
        }

        /**
         * @brief Take a single time step.
         *
         * @param bodies     Rigid bodies
         * @param gravity    Acceleration due to gravity
         * @param time_step  Timestep
         */
        virtual void step2D(
            physics::RigidBodyAssembler& bodies,
            const Eigen::Vector2d& gravity,
            const double& time_step) const
        {
            assert(bodies.dim() == 2);
            tbb::parallel_for_each(bodies.m_rbs, [&](physics::RigidBody& body) {
                step2D(body, gravity, time_step);
            });
        }

        /**
         * @brief Take a single time step.
         *
         * @param bodies     Rigid body
         * @param gravity    Acceleration due to gravity
         * @param time_step  Timestep
         */
        virtual void step3D(
            physics::RigidBody& body,
            const Eigen::Vector3d& gravity,
            const double& time_step) const
        {
            throw NotImplementedError(
                fmt::format("Time-stepper {} not implemented in 3D!", name()));
        }

        /**
         * @brief Take a single time step.
         *
         * @param bodies     Rigid bodies
         * @param gravity    Acceleration due to gravity
         * @param time_step  Timestep
         */
        virtual void step3D(
            physics::RigidBodyAssembler& bodies,
            const Eigen::Vector3d& gravity,
            const double& time_step) const
        {
            assert(bodies.dim() == 3);
            tbb::parallel_for_each(bodies.m_rbs, [&](physics::RigidBody& body) {
                step3D(body, gravity, time_step);
            });
        }
    };

} // namespace time_stepper
} // namespace ccd

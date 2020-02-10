#include "read_rb_scene.hpp"

#include <igl/edges.h>
#include <igl/read_triangle_mesh.h>

#include <io/serialize_json.hpp>
#include <logger.hpp>
#include <utils/not_implemented_error.hpp>

namespace ccd {
namespace io {

    int read_rb_scene_from_str(
        const std::string str, std::vector<physics::RigidBody>& rbs)
    {
        using nlohmann::json;
        json scene = json::parse(str.c_str());
        return read_rb_scene(scene, rbs);
    }

    int read_rb_scene(
        const nlohmann::json& scene, std::vector<physics::RigidBody>& rbs)
    {
        using namespace nlohmann;
        int dim = -1, ndof, angular_dim;
        for (auto& jrb : scene["rigid_bodies"]) {
            json args = R"({
                  "mesh": "",
                  "vertices":[],
                  "faces":[],
                  "edges":[],
                  "density":1.0,
                  "is_dof_fixed":[false,false,false, false,false,false],
                  "oriented":false,
                  "position":[0.0,0.0,0.0],
                  "rotation":[0.0,0.0,0.0],
                  "linear_velocity":[0.0,0.0,0.0],
                  "angular_velocity":[0.0,0.0,0.0]
                  })"_json;
            args.merge_patch(jrb);

            Eigen::MatrixXd vertices;
            Eigen::MatrixXi faces, edges;

            std::string mesh_fname = args["mesh"].get<std::string>();
            if (mesh_fname != "") {
                igl::read_triangle_mesh(mesh_fname, vertices, faces);
                // Initialize edges
                igl::edges(faces, edges);
            } else {
                from_json<double>(args["vertices"], vertices);
                from_json<int>(args["faces"], faces);
                from_json<int>(args["edges"], edges);
            }

            if (dim == -1) {
                if (vertices.cols() != 0) { // Why would we have an empty body?
                    dim = vertices.cols();
                    ndof = physics::Pose<double>::dim_to_ndof(dim);
                    angular_dim = dim == 2 ? 1 : 3;
                }
            } else if (dim != vertices.cols()) {
                spdlog::error(
                    "Mixing 2D and 3D bodies are not currently allowed.");
                throw NotImplementedError(
                    "Mixing 2D and 3D bodies are not currently allowed.");
            }

            if (dim == 2 && faces.size() != 0) {
                spdlog::warn("Ignoring faces for 2D rigid body.");
                // Delete the faces since they will not be uses
                faces.resize(0, 0);
            }

            Eigen::VectorXd position;
            from_json<double>(args["position"], position);
            assert(position.size() >= dim);
            position.conservativeResize(dim);

            Eigen::VectorXd rotation;
            from_json<double>(args["rotation"], rotation);
            assert(rotation.size() >= angular_dim);
            rotation.conservativeResize(angular_dim);
            // Convert to radians for easy use later
            rotation *= M_PI / 180.0;

            Eigen::VectorXd linear_velocity;
            from_json<double>(args["linear_velocity"], linear_velocity);
            assert(linear_velocity.size() >= dim);
            linear_velocity.conservativeResize(dim);

            Eigen::VectorXd angular_velocity;
            from_json<double>(args["angular_velocity"], angular_velocity);
            assert(angular_velocity.size() >= angular_dim);
            angular_velocity.conservativeResize(angular_dim);
            // Convert to radians for easy use later
            angular_velocity *= M_PI / 180.0;

            Eigen::VectorXb is_dof_fixed;
            from_json<bool>(args["is_dof_fixed"], is_dof_fixed);
            assert(is_dof_fixed.size() >= ndof);
            is_dof_fixed.conservativeResize(ndof);

            double density = args["density"].get<double>();
            bool is_oriented = args["oriented"].get<bool>();

            auto rb = physics::RigidBody::from_points(
                vertices, faces, edges,
                physics::Pose<double>(position, rotation),
                physics::Pose<double>(linear_velocity, angular_velocity),
                density, is_dof_fixed, is_oriented);

            rbs.push_back(rb);
        }
        return dim;
    }

} // namespace io
} // namespace ccd

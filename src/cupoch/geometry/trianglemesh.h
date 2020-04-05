#pragma once
#include <Eigen/Geometry>

#include "cupoch/geometry/image.h"
#include "cupoch/geometry/meshbase.h"

namespace cupoch {
namespace geometry {

class TriangleMesh : public MeshBase {
public:
    TriangleMesh();
    TriangleMesh(const utility::device_vector<Eigen::Vector3f> &vertices,
                 const utility::device_vector<Eigen::Vector3i> &triangles);
    TriangleMesh(const thrust::host_vector<Eigen::Vector3f> &vertices,
                 const thrust::host_vector<Eigen::Vector3i> &triangles);
    TriangleMesh(const geometry::TriangleMesh &other);
    ~TriangleMesh() override;
    TriangleMesh &operator=(const TriangleMesh &other);

    thrust::host_vector<Eigen::Vector3i> GetTriangles() const;
    void SetTriangles(const thrust::host_vector<Eigen::Vector3i> &triangles);

    thrust::host_vector<Eigen::Vector3f> GetTriangleNormals() const;
    void SetTriangleNormals(
            const thrust::host_vector<Eigen::Vector3f> &triangle_normals);

    thrust::host_vector<Eigen::Vector2i> GetAdjacencyList() const;
    void SetAdjacencyList(const thrust::host_vector<Eigen::Vector2i> &adjacency_list);

    thrust::host_vector<Eigen::Vector2f> GetTriangleUVs() const;
    void SetTriangleUVs(thrust::host_vector<Eigen::Vector2f> &triangle_uvs);

public:
    virtual TriangleMesh &Clear() override;
    TriangleMesh &operator+=(const TriangleMesh &mesh);
    TriangleMesh operator+(const TriangleMesh &mesh) const;

    __host__ __device__ bool HasTriangles() const {
        return vertices_.size() > 0 && triangles_.size() > 0;
    }

    __host__ __device__ bool HasTriangleNormals() const {
        return HasTriangles() && triangles_.size() == triangle_normals_.size();
    }

    __host__ __device__ bool HasAdjacencyList() const {
        return vertices_.size() > 0 &&
               adjacency_list_.size() > 0;
    }

    __host__ __device__ bool HasTriangleUvs() const {
        return HasTriangles() && triangle_uvs_.size() == 3 * triangles_.size();
    }

    bool HasTexture() const { return !texture_.IsEmpty(); }

    TriangleMesh &NormalizeNormals();

    /// Function to compute triangle normals, usually called before rendering
    TriangleMesh &ComputeTriangleNormals(bool normalized = true);

    /// Function to compute vertex normals, usually called before rendering
    TriangleMesh &ComputeVertexNormals(bool normalized = true);

    /// \brief Function that removes duplicated verties, i.e., vertices that
    /// have identical coordinates.
    TriangleMesh &RemoveDuplicatedVertices();

    /// \brief Function that removes duplicated triangles, i.e., removes
    /// triangles that reference the same three vertices, independent of their
    /// order.
    TriangleMesh &RemoveDuplicatedTriangles();

    /// \brief This function removes vertices from the triangle mesh that are
    /// not referenced in any triangle of the mesh.
    TriangleMesh &RemoveUnreferencedVertices();

    /// \brief Function that removes degenerate triangles, i.e., triangles that
    /// reference a single vertex multiple times in a single triangle.
    ///
    /// They are usually the product of removing duplicated vertices.
    TriangleMesh &RemoveDegenerateTriangles();

    /// Function to compute adjacency list, call before adjacency list is
    /// needed
    TriangleMesh &ComputeAdjacencyList();

    /// Function that computes the surface area of the mesh, i.e. the sum of
    /// the individual triangle surfaces.
    float GetSurfaceArea() const;

    /// Function that computes the surface area of the mesh, i.e. the sum of
    /// the individual triangle surfaces.
    float GetSurfaceArea(utility::device_vector<float> &triangle_areas) const;

    /// Function to sample \param number_of_points points uniformly from the
    /// mesh
    std::shared_ptr<PointCloud> SamplePointsUniformlyImpl(
            size_t number_of_points,
            utility::device_vector<float> &triangle_areas,
            float surface_area,
            bool use_triangle_normal);

    /// Function to sample \param number_of_points points uniformly from the
    /// mesh. \param use_triangle_normal Set to true to assign the triangle
    /// normals to the returned points instead of the interpolated vertex
    /// normals. The triangle normals will be computed and added to the mesh
    /// if necessary.
    std::shared_ptr<PointCloud> SamplePointsUniformly(
        size_t number_of_points, bool use_triangle_normal = false);

    /// Function that returns a list of triangles that are intersecting the
    /// mesh.
    utility::device_vector<Eigen::Vector2i> GetSelfIntersectingTriangles()
            const;

    /// Factory function to create a tetrahedron mesh (trianglemeshfactory.cpp).
    /// the mesh centroid will be at (0,0,0) and \param radius defines the
    /// distance from the center to the mesh vertices.
    static std::shared_ptr<TriangleMesh> CreateTetrahedron(float radius = 1.0);

    /// Factory function to create an octahedron mesh (trianglemeshfactory.cpp).
    /// the mesh centroid will be at (0,0,0) and \param radius defines the
    /// distance from the center to the mesh vertices.
    static std::shared_ptr<TriangleMesh> CreateOctahedron(float radius = 1.0);

    /// Factory function to create an icosahedron mesh
    /// (trianglemeshfactory.cpp). the mesh centroid will be at (0,0,0) and
    /// \param radius defines the distance from the center to the mesh vertices.
    static std::shared_ptr<TriangleMesh> CreateIcosahedron(float radius = 1.0);

    /// Factory function to create a box mesh (TriangleMeshFactory.cpp)
    /// The left bottom corner on the front will be placed at (0, 0, 0).
    /// The \param width is x-directional length, and \param height and \param
    /// depth are y- and z-directional lengths respectively.
    static std::shared_ptr<TriangleMesh> CreateBox(float width = 1.0,
                                                   float height = 1.0,
                                                   float depth = 1.0);

    /// Factory function to create a sphere mesh (TriangleMeshFactory.cpp)
    /// The sphere with \param radius will be centered at (0, 0, 0).
    /// Its axis is aligned with z-axis.
    /// The longitudes will be split into \param resolution segments.
    /// The latitudes will be split into \param resolution * 2 segments.
    static std::shared_ptr<TriangleMesh> CreateSphere(float radius = 1.0,
                                                      int resolution = 20);

    /// Factory function to create a cylinder mesh (TriangleMeshFactory.cpp)
    /// The axis of the cylinder will be from (0, 0, -height/2) to (0, 0,
    /// height/2). The circle with \param radius will be split into \param
    /// resolution segments. The \param height will be split into \param split
    /// segments.
    static std::shared_ptr<TriangleMesh> CreateCylinder(float radius = 1.0,
                                                        float height = 2.0,
                                                        int resolution = 20,
                                                        int split = 4);

    /// Factory function to create a cone mesh (TriangleMeshFactory.cpp)
    /// The axis of the cone will be from (0, 0, 0) to (0, 0, \param height).
    /// The circle with \param radius will be split into \param resolution
    /// segments. The height will be split into \param split segments.
    static std::shared_ptr<TriangleMesh> CreateCone(float radius = 1.0,
                                                    float height = 2.0,
                                                    int resolution = 20,
                                                    int split = 1);
    /// Factory function to create a torus mesh (TriangleMeshFactory.cpp)
    /// The torus will be centered at (0, 0, 0) and a radius of \param
    /// torus_radius. The tube of the torus will have a radius of \param
    /// tube_radius. The number of segments in radial and tubular direction are
    /// \param radial_resolution and \param tubular_resolution respectively.
    static std::shared_ptr<TriangleMesh> CreateTorus(
            float torus_radius = 1.0,
            float tube_radius = 0.5,
            int radial_resolution = 30,
            int tubular_resolution = 20);

    /// Factory function to create an arrow mesh (TriangleMeshFactory.cpp)
    /// The axis of the cone with \param cone_radius will be along the z-axis.
    /// The cylinder with \param cylinder_radius is from
    /// (0, 0, 0) to (0, 0, cylinder_height), and
    /// the cone is from (0, 0, cylinder_height)
    /// to (0, 0, cylinder_height + cone_height).
    /// The cone will be split into \param resolution segments.
    /// The \param cylinder_height will be split into \param cylinder_split
    /// segments. The \param cone_height will be split into \param cone_split
    /// segments.
    static std::shared_ptr<TriangleMesh> CreateArrow(
            float cylinder_radius = 1.0,
            float cone_radius = 1.5,
            float cylinder_height = 5.0,
            float cone_height = 4.0,
            int resolution = 20,
            int cylinder_split = 4,
            int cone_split = 1);

    /// Factory function to create a coordinate frame mesh
    /// (TriangleMeshFactory.cu) The coordinate frame will be centered at
    /// \param origin The x, y, z axis will be rendered as red, green, and blue
    /// arrows respectively. \param size is the length of the axes.
    static std::shared_ptr<TriangleMesh> CreateCoordinateFrame(
            float size = 1.0,
            const Eigen::Vector3f &origin = Eigen::Vector3f(0.0, 0.0, 0.0));

    /// Factory function to create a Moebius strip. \param length_split
    /// defines the number of segments along the Moebius strip, \param
    /// width_split defines the number of segments along the width of
    /// the Moebius strip, \param twists defines the number of twists of the
    /// strip, \param radius defines the radius of the Moebius strip,
    /// \param flatness controls the height of the strip, \param width
    /// controls the width of the Moebius strip and \param scale is used
    /// to scale the entire Moebius strip.
    static std::shared_ptr<TriangleMesh> CreateMoebius(int length_split = 70,
                                                       int width_split = 15,
                                                       int twists = 1,
                                                       float radius = 1,
                                                       float flatness = 1,
                                                       float width = 1,
                                                       float scale = 1);
public:
    utility::device_vector<Eigen::Vector3i> triangles_;
    utility::device_vector<Eigen::Vector3f> triangle_normals_;
    utility::device_vector<Eigen::Vector2i> adjacency_list_;
    utility::device_vector<Eigen::Vector2f> triangle_uvs_;
    Image texture_;
};

    /// Function that computes the area of a mesh triangle
__host__ __device__
inline float ComputeTriangleArea(const Eigen::Vector3f &p0,
                                 const Eigen::Vector3f &p1,
                                 const Eigen::Vector3f &p2) {
    const Eigen::Vector3f x = p0 - p1;
    const Eigen::Vector3f y = p0 - p2;
    float area = 0.5 * x.cross(y).norm();
    return area;
}

/// Function that computes the area of a mesh triangle identified by the
/// triangle index
__host__ __device__
inline float GetTriangleArea(const Eigen::Vector3f* vertices,
                             const Eigen::Vector3i* triangles,
                             size_t triangle_idx) {
    const Eigen::Vector3i &triangle = triangles[triangle_idx];
    const Eigen::Vector3f &vertex0 = vertices[triangle(0)];
    const Eigen::Vector3f &vertex1 = vertices[triangle(1)];
    const Eigen::Vector3f &vertex2 = vertices[triangle(2)];
    return ComputeTriangleArea(vertex0, vertex1, vertex2);
}

__host__ __device__
inline Eigen::Vector3i GetOrderedTriangle(int vidx0,
                                          int vidx1,
                                          int vidx2) {
    if (vidx0 > vidx2) {
        thrust::swap(vidx0, vidx2);
    }
    if (vidx0 > vidx1) {
        thrust::swap(vidx0, vidx1);
    }
    if (vidx1 > vidx2) {
        thrust::swap(vidx1, vidx2);
    }
    return Eigen::Vector3i(vidx0, vidx1, vidx2);
}

}  // namespace geometry
}  // namespace cupoch
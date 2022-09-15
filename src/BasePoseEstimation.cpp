#include "BasePoseEstimation.hpp"

using namespace deep_trekker;

BasePoseEstimation::BasePoseEstimation()
{
    readLEDPositions();
}

void BasePoseEstimation::readLEDPositions()
{

    // The real LED positions will be fixed.
    // This part will be replaced with a yaml config or something like that

    this->distance_point1 << 0.0, 0.2, 0.0;
    this->distance_point2 << 0.0, 0.0, 0.2;
    this->distance_point3 << 0.0, -0.2, 0.0;
    this->distance_point4 << 0.0, 0.0, -0.2;

    getPlaneMatrix(distance_point1,
        distance_point2,
        distance_point3,
        distance_normal_plane);

    this->distance_normal_plane_inverse = distance_normal_plane.inverse();
}

void BasePoseEstimation::estimateOrientationFromCameraPoints(
    Eigen::Matrix3d& rotational_matrix)
{
    // This input should come from the P3P algorithm
    Eigen::Vector3d camera_point1(3.24, -0.2, -0.245);
    Eigen::Vector3d camera_point2(3.24, 0, -0.445);
    Eigen::Vector3d camera_point3(3.24, 0.2, -0.245);
    Eigen::Vector3d camera_point4(3.24, 0, -0.045);
    Eigen::Matrix3d camera_normal_plane;

    getPlaneMatrix(camera_point1, camera_point2, camera_point3, camera_normal_plane);
    rotational_matrix = camera_normal_plane * this->distance_normal_plane_inverse;
}

void BasePoseEstimation::estimateTranslationFromCameraPoints(
    Eigen::Vector3d& translational_matrix)
{
    // CODE HERE
    translational_matrix << 0, 0, 0;
}

void BasePoseEstimation::getPlaneMatrix(Eigen::Vector3d& point1,
    Eigen::Vector3d& point2,
    Eigen::Vector3d& point3,
    Eigen::Matrix3d& plane_matrix)
{
    Eigen::Vector3d n1(point1 - point2);
    Eigen::Vector3d n2(point1 - point3);
    Eigen::Vector3d n3 = n1.cross(n2);

    plane_matrix.col(0) = n1;
    plane_matrix.col(1) = n2;
    plane_matrix.col(2) = n3;
}

void BasePoseEstimation::getOrientation(Eigen::Matrix3d& rotational_matrix,
    float* theta,
    float* psi,
    float* phi)
{
    *theta = atan2(rotational_matrix(2, 1), rotational_matrix(2, 2));
    *psi = atan2(rotational_matrix(2, 0),
        (sqrt(pow(rotational_matrix(0, 0), 2) + pow(rotational_matrix(1, 0), 2))));
    *phi = atan2(rotational_matrix(1, 0), rotational_matrix(0, 0));
}

void BasePoseEstimation::estimateBasePose(Eigen::Matrix4d& base_pose_tf)
{
    Eigen::Matrix3d rotational_matrix;
    Eigen::Vector3d translational_matrix;

    estimateOrientationFromCameraPoints(rotational_matrix);
    estimateTranslationFromCameraPoints(translational_matrix);

    float theta, psi, phi;
    getOrientation(rotational_matrix, &theta, &psi, &phi);
    std::cout << "theta: " << theta << " psi " << psi << " phi " << phi << std::endl;

    base_pose_tf.setIdentity(); // Set to Identity to make bottom row of Matrix 0,0,0,1
    base_pose_tf.block<3, 3>(0, 0) = rotational_matrix;
    base_pose_tf.block<3, 1>(0, 3) = translational_matrix;
}
#ifndef _BASE_POSE_ESTIMATION_HPP_
#define _BASE_POSE_ESTIMATION_HPP_

#include <Eigen/Geometry>
#include <iostream>
#include <math.h>

namespace deep_trekker {

    class BasePoseEstimation {
    private:
        Eigen::Vector3d distance_point1;
        Eigen::Vector3d distance_point2;
        Eigen::Vector3d distance_point3;
        Eigen::Vector3d distance_point4;
        Eigen::Matrix3d distance_normal_plane;
        Eigen::Matrix3d distance_normal_plane_inverse;

    protected:
    public:
        BasePoseEstimation();
        ~BasePoseEstimation();

        void readLEDPositions();

        // Create a matrix 3x3 representing a plane from 3 points
        void getPlaneMatrix(Eigen::Vector3d& point1,
            Eigen::Vector3d& point2,
            Eigen::Vector3d& point3,
            Eigen::Matrix3d& plane_matrix);

        // Calculates the orientation(theta, psi, phi) of the AUV related to the LEDs
        void getOrientation(Eigen::Matrix3d& rotational_matrix,
            float* theta,
            float* psi,
            float* phi);

        // Estimate Orientation based on 3 points coordinates
        void estimateOrientationFromCameraPoints(Eigen::Matrix3d& rotational_matrix);

        // Estimate Translation based on 3 points coordinates
        void estimateTranslationFromCameraPoints(Eigen::Vector3d& translational_matrix);

        // Estimate Base Pose
        void estimateBasePose(Eigen::Matrix4d& base_pose_tf);
    };
} // namespace deep_trekker

#endif

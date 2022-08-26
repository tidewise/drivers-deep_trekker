#ifndef _DEEP_TREKKER_COMMANDS_HPP_
#define _DEEP_TREKKER_COMMANDS_HPP_

#include <base/samples/Joints.hpp>
#include <base/samples/RigidBodyState.hpp>

namespace deep_trekker
{

    /**
     *  ratio (Represented as a multiplier, 1x zoom for fully zoomed out,
     *  higher values when zoomed in (3x, 12.3x, 20x, etc..))
     *  speed (joint):
     *   - min: -100 (revert/retract)
     *   - max: +100 (forward)
     */
    struct ZoomControlCommand
    {
        double ratio;
        base::samples::Joints speed;
    };

    /**
     *  brightness/focus/saturation/sharpness:
     *   - min: 0
     *   - max 100
     *  exposure:
     *   - min: 0
     *   - max: 15
     */
    struct TamronHarrierZoomCameraCommand
    {
        double exposure;
        double brightness;
        double focus;
        double saturation;
        double sharpness;
        ZoomControlCommand zoom;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 100
     *  position (joint):
     *   - min: -180
     *   - max: +180
     *  velocity (joint):
     *   - min: -100
     *   - max: +100
     */
    struct TiltCameraHeadCommand
    {
        bool laser;
        double light;
        base::samples::Joints speed;
        TamronHarrierZoomCameraCommand camera;
    };

    /**
     *  open:
     *   - < 0: close grabber claw (min: -100)
     *   - == 0: motor off
     *   - > 0: open grabber claw (max: +100)
     *  speed:
     *   - < 0: rotate left (min: -100)
     *   - == 0: motor off
     *   - > 0: rotate right (max: +100)
     */
    struct GrabberCommand
    {
        double open;
        base::samples::Joints speed;
    };

    /**
     * Setpoint for position.
     *
     */
    struct VehicleSetpoint
    {
        base::Time time;
        base::Vector3d position;
        base::Angle yaw;

        VehicleSetpoint()
        {
            time = base::Time::now();
            position = base::Vector3d::Zero();
            yaw = base::Angle();
        }
    };

    /**
     *  -light:
     *   - min: 0
     *   - max: 100
     *  -Command in local frame
     */
    struct PositionAndLightCommand
    {
        double light;
        VehicleSetpoint vehicle_setpoint;
    };

    struct PoweredReelControlCommand
    {
        bool reel_forward;
        bool reel_reverse;
        base::samples::Joints speed;
    };

} // namespace deep_trekker

#endif

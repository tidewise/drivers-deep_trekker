#ifndef _DEEP_TREKKER_COMMANDS_HPP_
#define _DEEP_TREKKER_COMMANDS_HPP_

#include <base/samples/RigidBodyState.hpp>
#include <base/samples/Joints.hpp>

namespace deep_trekker
{

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
        bool laser_camera;
        double light_camera;
        base::samples::Joints tilt;
    };

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

    struct GrabberCommand
    {
        double open;
        base::samples::Joints rotate;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 100
     */
    struct LaserLightCommands
    {
        bool laser_vehicle;
        double light_vehicle;
    };

    struct RovControlCommand
    {
        base::samples::RigidBodyState vehicle_setpoint;
    };

} // namespace deep_trekker

#endif

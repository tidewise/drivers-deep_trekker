#ifndef _DEEP_TREKKER_COMMANDS_HPP_
#define _DEEP_TREKKER_COMMANDS_HPP_

#include "base/commands/LinearAngular6DCommand.hpp"
#include <base/samples/Joints.hpp>
#include <base/samples/RigidBodyState.hpp>

namespace deep_trekker {

    /**
     *  ratio (Represented as a multiplier, 1x zoom for fully zoomed out,
     *  higher values when zoomed in (3x, 12.3x, 20x, etc..))
     *  speed:
     *   - min: -1 (revert/retract)
     *   - max: +1 (forward)
     */
    struct ZoomControlCommand {
        float ratio;
        float speed;
    };

    /**
     *  brightness/focus/saturation/sharpness/exposure:
     *   - min: 0
     *   - max 1
     */
    struct TamronHarrierZoomCameraCommand {
        float exposure;
        float brightness;
        float focus;
        float saturation;
        float sharpness;
        ZoomControlCommand zoom;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 1
     */
    struct CameraHeadCommand {
        base::Time time;
        bool laser;
        double light;
        TamronHarrierZoomCameraCommand camera;
    };

    /**
     *  tilt_command (position joint):
     *   - min: -pi
     *   - max: +pi
     *  tilt_command (speed joint):
     *   - min: -1
     *   - max: +1
     */
    typedef base::samples::Joints TiltCameraHeadCommand;

    /**
     *  light:
     *   - min: 0
     *   - max: 1
     */
    struct PositionAndLightCommand {
        base::Time time;
        double light;
        base::commands::LinearAngular6DCommand vehicle_setpoint;
    };

    /**
     *  motors (raw joint):
     *   open_close (unit: motor power):
     *    - < 0: close grabber claw (min: -1)
     *    - == 0: motor off
     *    - > 0: open grabber claw (max: +1)
     *   rotate (unit: motor power):
     *    - < 0: rotate left (min: -1)
     *    - == 0: motor off
     *    - > 0: rotate right (max: +1)
     *   elements:
     *    - [0] open_close joint
     *    - [1] rotate joint
     */
    typedef base::samples::Joints GrabberCommand;

    /**
     *  motor:
     *   speed joint:
     *    - min: -1 (revert/retract)
     *    - max: +1 (forward)
     */
    typedef base::samples::Joints PoweredReelControlCommand;

} // namespace deep_trekker

#endif

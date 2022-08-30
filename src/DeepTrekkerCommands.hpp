#ifndef _DEEP_TREKKER_COMMANDS_HPP_
#define _DEEP_TREKKER_COMMANDS_HPP_

#include "base/commands/LinearAngular6DCommand.hpp"
#include <base/samples/Joints.hpp>
#include <base/samples/RigidBodyState.hpp>

namespace deep_trekker {

    /**
     *  ratio (Represented as a multiplier, 1x zoom for fully zoomed out,
     *  higher values when zoomed in (3x, 12.3x, 20x, etc..))
     *  speed (joint):
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
     *  position:
     *   - min: -pi
     *   - max: +pi
     *  velocity:
     *   - min: -1
     *   - max: +1
     */
    struct TiltCameraHeadCommand {
        bool laser;
        double light;
        base::samples::Joints speed;
        base::samples::Joints position;
        TamronHarrierZoomCameraCommand camera;
    };

    /**
     *  open_close (unit: motor power):
     *   - < 0: close grabber claw (min: -1)
     *   - == 0: motor off
     *   - > 0: open grabber claw (max: +1)
     *  rotate (unit: motor power):
     *   - < 0: rotate left (min: -1)
     *   - == 0: motor off
     *   - > 0: rotate right (max: +1)
     */
    struct GrabberCommand {
        base::samples::Joints open_close;
        base::samples::Joints rotate;
    };

    /**
     *  -light:
     *   - min: 0
     *   - max: 1
     *  -Command in local frame
     */
    struct PositionAndLightCommand {
        double light;
        base::commands::LinearAngular6DCommand vehicle_setpoint;
    };

    /**
     *  speed:
     *   - min: -1 (revert/retract)
     *   - max: +1 (forward)
     */
    struct PoweredReelControlCommand {
        base::samples::Joints speed;
    };

} // namespace deep_trekker

#endif

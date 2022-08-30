#ifndef _DEEP_TREKKER_STATES_HPP_
#define _DEEP_TREKKER_STATES_HPP_

#include <base/Time.hpp>
#include <base/samples/Joints.hpp>
#include <base/samples/RigidBodyState.hpp>

namespace deep_trekker {

    struct DevicesMacAddress {
        std::string revolution;
        std::string manual_reel;
        std::string powered_reel;
    };

    struct Battery {
        bool charging;
        double percentage;
        double voltage;
    };

    /**
     *  current (mA)
     *  pwm ([0 1])
     *  rotation (rad/s):
     *   - < 0: reversing
     *   - > 0: moving forward
     *   - = 0: not moving
     */
    struct MotorDiagnostics
    {
        double current;
        bool overcurrent;
        base::samples::Joints motor;
    };

    /**
     *  ratio (Represented as a multiplier, 1x zoom for fully zoomed out,
     *  higher values when zoomed in (3x, 12.3x, 20x, etc..))
     *  speed (joint):
     *   - min: -1 (revert/retract)
     *   - max: +1 (forward)
     */
    struct ZoomControl
    {
        double ratio;
        base::samples::Joints speed;
    };

    /**
     *  brightness/focus/saturation/sharpness:
     *   - min: 0
     *   - max 1
     *  exposure:
     *   - min: 0
     *   - max: 1
     */
    struct TamronHarrierZoomCamera
    {
        double exposure;
        double brightness;
        double focus;
        double saturation;
        double sharpness;
        ZoomControl zoom;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 1
     *  position (joint):
     *   - min: -pi
     *   - max: +pi
     *  velocity (joint):
     *   - min: -1
     *   - max: +1
     */
    struct TiltCameraHead {
        bool laser;
        double light;
        base::samples::Joints tilt_command;
        MotorDiagnostics tilt_motor_diagnostics;
        TamronHarrierZoomCamera camera;
    };

    /**
     *  tether_distance (payed out tether distance, given in cm)
     */
    struct ManualReel {
        bool calibrated;
        bool leak;
        bool ready;
        double tether_distance;
        double cpu_temperature;
    };

    /**
     *  tether_distance (payed out tether distance, given in cm)
     *  estop_enabled (physical button state)
     *  speed:
     *   - min: -1 (revert/retract)
     *   - max: +1 (forward)
     */
    struct PoweredReel {
        bool calibrated;
        bool leak;
        bool ready;
        bool estop_enabled;
        bool ac_power_connected;
        double tether_distance;
        double cpu_temperature;
        base::samples::Joints speed;
        Battery battery_1;
        Battery battery_2;
        MotorDiagnostics motor_1;
        MotorDiagnostics motor_2;
    };

    struct Grabber
    {
        double open;
        base::samples::Joints rotate_command;
        MotorDiagnostics motor_diagnostic;
    };

    /** Command and state in local frame */
    struct RovControl {
        base::samples::RigidBodyState vehicle_setpoint;
        base::samples::RigidBodyState state_estimator;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 1
     */
    struct Revolution {
        double light;
        base::Time usage_time;
        RovControl vehicle_control;
        Battery left_battery;
        Battery right_battery;
        Grabber grabber;
        TiltCameraHead camera_head;
        MotorDiagnostics front_left_motor;
        MotorDiagnostics front_right_motor;
        MotorDiagnostics rear_left_motor;
        MotorDiagnostics rear_right_motor;
        MotorDiagnostics vertical_left_motor;
        MotorDiagnostics vertical_right_motor;
    };

} // namespace deep_trekker

#endif

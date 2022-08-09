#ifndef _DEEP_TREKKER_STATES_HPP_
#define _DEEP_TREKKER_STATES_HPP_

#include <base/Time.hpp>
#include <base/samples/RigidBodyState.hpp>
#include <base/samples/Joints.hpp>

namespace deep_trekker
{

    struct Battery
    {
        bool charging;
        double percentage;
        double voltage;
    };

    /**
     *  current (mA)
     *  pwm (%)
     *  rpm:
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
    struct TiltCameraHead
    {
        bool laser_camera;
        double light_camera;
        base::samples::Joints tilt;
        MotorDiagnostics tilt_motor_diagnostics;
    };

    /**
     *  tether_distance (payed out tether distance, given in cm)
     */
    struct ManualReel
    {
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
     *   - min: -100 (revert/retract)
     *   - max: +100 (forward)
     */
    struct PoweredReel
    {
        bool calibrated;
        bool leak;
        bool ready;
        bool reel_forward;
        bool reel_reverse;
        bool estop_enabled;
        bool ac_power_connected;
        bool hibrid_power_enabled;
        double tether_distance;
        double cpu_temperature;
        double speed;
        Battery battery_1;
        Battery battery_2;
        MotorDiagnostics motor_1;
        MotorDiagnostics motor_2;
    };

    struct Reels
    {
        PoweredReel powered_reel;
        ManualReel manual_reel;
    };

    /**
     *  ratio (Represented as a multiplier, 1x zoom for fully zoomed out,
     *  higher values when zoomed in (3x, 12.3x, 20x, etc..))
     *  speed (joint):
     *   - min: -100 (revert/retract)
     *   - max: +100 (forward)
     */
    struct ZoomControl
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
    struct TamronHarrierZoomCamera
    {
        bool ready;
        double exposure;
        double brightness;
        double focus;
        double saturation;
        double sharpness;
        ZoomControl zoom;
    };

    struct Grabber
    {
        double open;
        base::samples::Joints rotate;
        MotorDiagnostics grabber_motor_diagnostic;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 100
     */
    struct Peripherals
    {
        bool laser_vehicle;
        double light_vehicle;
        Grabber grabber;
        TiltCameraHead tilt_camera_head;
    };

    struct RovControl
    {
        base::samples::RigidBodyState vehicle_setpoint;
        base::samples::RigidBodyState state_estimator;
    };

    struct Revolution
    {
       base::Time usage_time;
       RovControl vehicle_control;
       Peripherals peripherals;
       Battery left_battery;
       Battery right_battery;
       MotorDiagnostics front_left_motor;
       MotorDiagnostics front_right_motor;
       MotorDiagnostics rear_left_motor;
       MotorDiagnostics rear_right_motor;
       MotorDiagnostics vertical_left_motor;
       MotorDiagnostics vertical_right_motor;
    };

} // namespace deep_trekker

#endif

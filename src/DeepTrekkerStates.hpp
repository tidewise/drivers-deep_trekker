#ifndef _DEEP_TREKKER_STATES_HPP_
#define _DEEP_TREKKER_STATES_HPP_

#include <base/Time.hpp>
#include <base/samples/Joints.hpp>
#include <base/samples/RigidBodyState.hpp>
#include <power_base/BatteryStatus.hpp>

namespace deep_trekker {

    enum MotionControllerType
    {
        position,
        velocity,
        acceleration
    };
    
    struct DevicesMacAddress {
        std::string revolution;
        std::string manual_reel;
        std::string powered_reel;
    };

    /**
     *  ratio (Represented as a multiplier, 1x zoom for fully zoomed out,
     *  higher values when zoomed in (3x, 12.3x, 20x, etc..))
     *  speed:
     *   - min: -1 (revert/retract)
     *   - max: +1 (forward)
     */
    struct ZoomControl {
        float ratio;
        float speed;
    };

    /**
     *  brightness/focus/saturation/sharpness:
     *   - min: 0
     *   - max 1
     *  exposure:
     *   - min: 0
     *   - max: 1
     */
    struct TamronHarrierZoomCamera {
        float exposure;
        float brightness;
        float focus;
        float saturation;
        float sharpness;
        ZoomControl zoom;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 1
     *  motor:
     *   position joint:
     *    - min: -pi
     *    - max: +pi
     *   speed joint:
     *    - min: -1
     *    - max: +1
     */
    struct TiltCameraHead {
        double light;
        bool laser;
        bool motor_overcurrent;
        TamronHarrierZoomCamera camera;
    };

    /**
     *  motor:
     *   position joint:
     *    - min: -pi
     *    - max: +pi
     *   speed joint:
     *    - min: -1
     *    - max: +1
     */
    typedef base::samples::Joints TiltCameraHeadMotorStates;

    /**
     *  tether_distance (payed out tether distance, given in cm)
     */
    struct ManualReel {
        bool calibrated;
        bool leak;
        bool ready;
        double tether_lenght;
        double cpu_temperature;
    };

    /**
     *  tether_distance (payed out tether distance, given in cm)
     *  estop_enabled (physical button state)
     */
    struct PoweredReel {
        bool calibrated;
        bool leak;
        bool ready;
        bool estop_enabled;
        bool ac_power_connected;
        bool motor_1_overcurrent;
        bool motor_2_overcurrent;
        double tether_lenght;
        double cpu_temperature;
        power_base::BatteryStatus battery_1;
        power_base::BatteryStatus battery_2;
    };

    /**
     *  motors:
     *   - current (mA): effort joint
     *   - pwm ([0 1]): raw joint
     *   - rotation (rad/s): speed joint
     *     - < 0: reversing
     *     - > 0: moving forward
     *     - = 0: not moving
     */
    typedef base::samples::Joints PoweredReelMotorStates;

    struct Grabber {
        bool open_close_motor_overcurrent;
        bool rotate_overcurrent;
    };

    /**
     *  motors:
     *   current (mA): effort joint
     *   pwm ([0 1]): raw joint
     *   rotation (rad/s): speed joint
     *    - < 0: reversing
     *    - > 0: moving forward
     *    - = 0: not moving
     *   elements:
     *    - [0] open_close joint
     *    - [1] roll joint
     */
    typedef base::samples::Joints GrabberMotorStates;

    /** Setpoint and state in local frame */
    typedef base::samples::RigidBodyState RevolutionControl;
    /** Setpoint and state in local frame */
    typedef base::samples::RigidBodyState RevolutionBodyStates;

    /**
     *  light:
     *   - min: 0
     *   - max: 1
     *  setpoint and state in local frame
     */
    struct Revolution {
        double light;
        bool front_right_motor_overcurrent;
        bool front_left_motor_overcurrent;
        bool rear_right_motor_overcurrent;
        bool rear_left_motor_overcurrent;
        bool vertical_right_motor_overcurrent;
        bool vertical_left_motor_overcurrent;
        Grabber grabber;
        base::samples::RigidBodyState vehicle_control;
        TiltCameraHead camera_head;
        base::Time usage_time;
        power_base::BatteryStatus left_battery;
        power_base::BatteryStatus right_battery;
    };

    /**
     *  motors:
     *   current (mA): effort joint
     *   pwm ([0 1]): raw joint
     *   rotation (rad/s ): speed joint
     *    - < 0: reversing
     *    - > 0: moving forward
     *    - = 0: not moving
     *   elements:
     *    - [0] front_right_motor
     *    - [1] front_left_motor
     *    - [2] rear_right_motor
     *    - [3] rear_left_motor
     *    - [4] vertical_right_motor
     *    - [5] vertical_left_motor
     */
    typedef base::samples::Joints RevolutionMotorStates;

} // namespace deep_trekker

#endif

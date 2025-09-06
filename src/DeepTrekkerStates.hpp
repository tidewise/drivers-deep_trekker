#ifndef _DEEP_TREKKER_STATES_HPP_
#define _DEEP_TREKKER_STATES_HPP_

#include "base/commands/LinearAngular6DCommand.hpp"
#include <base/Time.hpp>
#include <base/samples/Joints.hpp>
#include <base/samples/RigidBodyState.hpp>
#include <power_base/BatteryStatus.hpp>

namespace deep_trekker {

    enum MotionControllerType {
        position,
        velocity,
        acceleration
    };

    struct DevicesID {
        std::string revolution;
        std::string manual_reel;
        std::string powered_reel;
        std::string camera;
        std::vector<std::string> streams;
    };

    struct DevicesModel {
        int revolution;
        int manual_reel;
        int powered_reel;
        int camera;
        int camera_head;
    };

    struct CameraHeadLimits {
        double lower;
        double upper;
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

    struct Camera {
        base::Time time;
        std::string id;
        std::string ip;
        bool osd_enabled;
        std::vector<std::string> active_streams;
        std::string type;
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
        base::Time time;
        double light;
        bool laser;
        bool motor_overcurrent;
        bool leak;
        base::samples::Joints motor_states;
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
        bool leak;
        bool ready;
        double tether_length;
        double cpu_temperature;
    };

    /**
     *  tether_distance (payed out tether distance)
     *  estop_enabled (physical button state)
     */
    struct PoweredReel {
        base::Time time;
        bool leak;
        bool estop_enabled;
        bool ac_power_connected;
        bool motor_1_overcurrent;
        bool motor_2_overcurrent;
        double tether_length;
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
        base::samples::Joints motor_states;
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
     * Available drive modes of the Deep Trekker API, except auto stabilization and motors
     * disabled. Those were separated because they have a different behavior from the
     * others.
     */
    struct DriveMode {
        base::Time time;
        bool altitude_lock;
        bool depth_lock;
        bool heading_lock;
    };

    /**
     *  light:
     *   - min: 0
     *   - max: 1
     *  setpoint and state in local frame
     */
    struct Revolution {
        base::Time time;
        double aux_light;
        bool front_right_motor_overcurrent;
        bool front_left_motor_overcurrent;
        bool rear_right_motor_overcurrent;
        bool rear_left_motor_overcurrent;
        bool vertical_right_motor_overcurrent;
        bool vertical_left_motor_overcurrent;
        DriveMode drive_modes;
        std::vector<Camera> cameras;
        base::Time usage_time;
        power_base::BatteryStatus left_battery;
        power_base::BatteryStatus right_battery;
        float cpu_temperature;
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

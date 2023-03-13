#ifndef _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_
#define _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_

#include "base/Time.hpp"
#include "base/commands/LinearAngular6DCommand.hpp"
#include "deep_trekker/DeepTrekkerCommands.hpp"
#include "deep_trekker/DeepTrekkerStates.hpp"
#include "memory"
#include "power_base/BatteryStatus.hpp"
#include "string.h"
#include "json/json.h"

namespace deep_trekker {

    class CommandAndStateMessageParser {
    public:
        CommandAndStateMessageParser();

        std::string parseDriveModeRevolutionCommandMessage(std::string api_version,
            std::string address,
            int model,
            DriveMode command);
        std::string parseDriveRevolutionCommandMessage(std::string api_version,
            std::string address,
            int model,
            base::commands::LinearAngular6DCommand command,
            double minimum_vertical_command);
        std::string parsePoweredReelCommandMessage(std::string api_version,
            std::string address,
            int model,
            base::samples::Joints command);
        std::string parseGrabberCommandMessage(std::string api_version,
            std::string address,
            base::samples::Joints command);
        std::string parseTiltCameraHeadCommandMessage(std::string api_version,
            std::string address,
            int rev_model,
            int camera_head_model,
            base::samples::Joints tilt);
        std::string parseCameraHeadCommandMessage(std::string api_version,
            std::string address,
            int model,
            CameraHeadCommand head);
        std::string parseAuxLightCommandMessage(std::string api_version,
            std::string address,
            int model,
            double intensity);

        base::Time getTimeUsage(std::string address);

        Grabber getGrabberMotorOvercurrentStates(std::string address);
        power_base::BatteryStatus getBatteryStates(std::string address,
            std::string battery_side);
        base::samples::Joints getCameraHeadTiltMotorState(std::string address);
        TiltCameraHead getCameraHeadStates(std::string address);
        std::vector<Camera> getCameras(std::string address);

        base::samples::RigidBodyState getRevolutionDriveStates(std::string address);
        DriveMode getRevolutionDriveModes(std::string address);
        /**
         * @see RevolutionBodyStates
         */
        base::samples::RigidBodyState getRevolutionPoseZAttitude(std::string address);
        /**
         * @see GrabberMotorStates
         */
        Grabber getGrabberMotorStates(std::string address);
        /**
         * @see PoweredReelMotorStates
         */
        base::samples::Joints getPoweredReelMotorState(std::string address);
        /**
         * @see RevolutionMotorStates
         */
        base::samples::Joints getRevolutionMotorStates(std::string address);
        base::JointState motorDiagnosticsToJointState(Json::Value value);
        double getAuxLightIntensity(std::string address);
        double getCpuTemperature(std::string address);
        double getTetherLength(std::string address);
        bool getMotorOvercurrentStates(std::string address, std::string motor_side);
        bool isACPowerConnected(std::string address);
        bool isEStopEnabled(std::string address);
        bool isLeaking(std::string address);
        bool parseJSONMessage(char const* data, std::string& errors);
        void validateFieldPresent(Json::Value const& value,
            std::string const& fieldName,
            std::string const& context);

        void validateMotorOverCurrentStates(std::string motor_field_name,
            std::string device_id);
        void validateBatteryStates(std::string battery_field_name, std::string device_id);
        void validateAuxLightIntensity(std::string device_id);
        void validateDepthAttitude(std::string device_id);
        void validateCameraHeadStates(std::string device_id);
        void validateCameras(std::string device_id);
        void validateCPUTemperature(std::string device_id);
        void validateDriveStates(std::string device_id);
        void validateDriveModes(std::string device_id);
        void validateLeaking(std::string device_id);
        void validateACConnected(std::string device_id);
        void validateEStop(std::string device_id);
        void validateDistance(std::string device_id);
        void validateTimeUsage(std::string device_id);
        void validateMotorStates(std::string device_id, std::string motor_field_name);
        void validateGrabberMotorsStates(std::string device_id);
        void validateRevolutionMotorStates(std::string device_id);
        void validatePoweredReelMotorState(std::string device_id);

    private:
        Json::Value m_json_data;
        Json::CharReaderBuilder mRBuilder;
        std::unique_ptr<Json::CharReader> mReader;

        Json::Value payloadSetMessageTemplate(std::string api_version,
            std::string address,
            int model);
    };

} // namespace deep_trekker

#endif

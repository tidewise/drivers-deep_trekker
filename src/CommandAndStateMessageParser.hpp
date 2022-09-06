#ifndef _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_
#define _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_

#include "base/commands/LinearAngular6DCommand.hpp"
#include "deep_trekker/DeepTrekkerCommands.hpp"
#include "deep_trekker/DeepTrekkerStates.hpp"
#include "power_base/BatteryStatus.hpp"
#include "stdio.h"
#include "string.h"
#include "json/json.h"
#include <algorithm>
#include <base/Time.hpp>
#include <memory>

namespace deep_trekker {

    class CommandAndStateMessageParser {
    public:
        CommandAndStateMessageParser();

        std::string parseGetMessage(std::string api_version);
        std::string parseRevolutionCommandMessage(std::string api_version,
            std::string address,
            PositionAndLightCommand command);
        std::string parsePoweredReelCommandMessage(std::string api_version,
            std::string address,
            base::samples::Joints command);
        std::string parseGrabberCommandMessage(std::string api_version,
            std::string address,
            base::samples::Joints command);
        std::string parseTiltCameraHeadCommandMessage(std::string api_version,
            std::string address,
            CameraHeadCommand head,
            base::samples::Joints tilt);
        base::Time getTimeUsage(std::string address);
        Grabber getGrabberMotorOvercurrentStates(std::string address);
        power_base::BatteryStatus getBatteryStates(std::string address,
            std::string battery_side);
        TiltCameraHead getCameraHeadStates(std::string address);
        /**
         * @see RevolutionControl
         */
        base::samples::RigidBodyState getRevolutionControlStates(std::string address);
        /**
         * @see RevolutionBodyStates
         */
        base::samples::RigidBodyState getRevolutionBodyStates(std::string address);
        /**
         * @see GrabberMotorStates
         */
        base::samples::Joints getGrabberMotorStates(std::string address);
        /**
         * @see PoweredReelMotorStates
         */
        base::samples::Joints getPoweredReelMotorState(std::string address);
        /**
         * @see RevolutionMotorStates
         */
        base::samples::Joints getRevolutionMotorStates(std::string address);
        /**
         * @see TiltCameraHeadMotorStates
         */
        base::samples::Joints getCameraHeadMotorStates(std::string address);
        base::JointState motorDiagnosticsToJointState(Json::Value value);
        double getLightIntensity(std::string address);
        double getCpuTemperature(std::string address);
        double getTetherLenght(std::string address);
        bool getMotorOvercurrentStates(std::string address,
            std::string motor_side);
        bool isACPowerConnected(std::string address);
        bool isEStopEnabled(std::string address);
        bool isCalibrated(std::string address);
        bool isReady(std::string address);
        bool isLeaking(std::string address);
        bool checkDeviceMacAddress(std::string address);
        bool parseJSONMessage(char const* data, std::string& errors);
        void validateFieldPresent(Json::Value const& value, std::string const& fieldName);

    private:
        Json::Value mJData;
        Json::CharReaderBuilder mRBuilder;
        std::unique_ptr<Json::CharReader> mReader;
    };

} // namespace deep_trekker

#endif

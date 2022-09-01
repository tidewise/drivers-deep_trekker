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
            PoweredReelControlCommand command);
        std::string parseGrabberCommandMessage(std::string api_version,
            std::string address,
            GrabberCommand command);
        std::string parseTiltCameraHeadCommandMessage(std::string api_version,
            std::string address,
            CameraHeadCommand head,
            TiltCameraHeadCommand tilt);
        base::Time getTimeUsage(std::string address);
        RevolutionControl getRevolutionControlStates(std::string address);
        RevolutionBodyStates getRevolutionBodyStates(std::string address);
        Grabber getGrabberMotorOvercurrentStates(std::string address);
        GrabberMotorStates getGrabberMotorStates(std::string address);
        PoweredReelMotorStates getPoweredReelMotorState(std::string address);
        RevolutionMotorStates getRevolutionMotorStates(std::string address);
        TiltCameraHead getCameraHeadStates(std::string address);
        TiltCameraHeadMotorState getCameraHeadMotorStates(std::string address);
        power_base::BatteryStatus getBatteryStates(std::string address,
            std::string battery_side);
        double getLightIntensity(std::string address);
        double getCpuTemperature(std::string address);
        double getTetherLenght(std::string address);
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

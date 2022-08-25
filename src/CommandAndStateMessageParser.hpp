#ifndef _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_
#define _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_

#include "deep_trekker/DeepTrekkerCommands.hpp"
#include "deep_trekker/DeepTrekkerStates.hpp"
#include "stdio.h"
#include "string.h"
#include "json/json.h"
#include <base/Eigen.hpp>
#include <base/Time.hpp>
#include <memory>

namespace deep_trekker
{

    class CommandAndStateMessageParser
    {
      public:
        CommandAndStateMessageParser();

        std::string parseGetMessage(std::string api_version);
        std::string parseRevolutionCommandMessage(
            std::string api_version,
            std::string address,
            PositionAndLightCommand command
        );
        std::string parsePoweredReelCommandMessage(
            std::string api_version,
            std::string address,
            PoweredReelControlCommand command
        );
        std::string parseGrabberCommandMessage(
            std::string api_version,
            std::string address,
            GrabberCommand command
        );
        std::string parseTiltCameraHeadCommandMessage(
            std::string api_version,
            std::string address,
            TiltCameraHeadCommand command
        );
        base::Time getTimeUsage(std::string address);
        RovControl getVehicleStates(std::string address);
        Grabber getGrabberMotorInfo(std::string address);
        TiltCameraHead getCameraHeadInfo(std::string address);
        Battery getBatteryInfo(std::string address, std::string battery_side);
        MotorDiagnostics getMotorInfo(std::string address, std::string motor_side);
        base::samples::Joints getSpeedInfo(std::string address);
        double getLightInfo(std::string address);
        double getTemperatureInfo(std::string address);
        double getTetherDistanceInfo(std::string address);
        bool getACPowerInfo(std::string address);
        bool getEStopInfo(std::string address);
        bool getCalibrateInfo(std::string address);
        bool getReadyInfo(std::string address);
        bool getLeakInfo(std::string address);
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

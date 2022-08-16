#ifndef _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_
#define _COMMAND_AND_STATE_MESSAGE_PARSER_HPP_

#include "stdio.h"
#include "string.h"
#include "json/json.h"
#include <memory>
#include <base/Eigen.hpp>
#include <base/Time.hpp>
#include "deep_trekker/DeepTrekkerStates.hpp"
#include "deep_trekker/DeepTrekkerCommands.hpp"

namespace deep_trekker
{

    class CommandAndStateMessageParser
    {
      public:
        CommandAndStateMessageParser();

        std::string parseTiltCameraHeadCommand();
        std::string parseZommCameraCommand();
        std::string parseGrabberCommand();
        std::string parseLaserCommand();
        std::string parseGetMessage();
        base::Time getTimeUsage(std::string address);
        RovControl getVehicleStates(std::string address);
        Grabber getGrabberMotorInfo(std::string address);
        TiltCameraHead getCameraHeadInfo(std::string address);
        Battery getBatteryInfo(std::string address, std::string battery_side);
        MotorDiagnostics getMotorInfo(std::string address, std::string motor_side);
        double getLightInfo(std::string address);
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

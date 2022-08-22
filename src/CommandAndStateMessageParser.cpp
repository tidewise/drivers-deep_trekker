#include "CommandAndStateMessageParser.hpp"
#include <iostream>

using namespace std;
using namespace base;
using namespace deep_trekker;

CommandAndStateMessageParser::CommandAndStateMessageParser()
    : mReader(mRBuilder.newCharReader())
{
}

bool CommandAndStateMessageParser::parseJSONMessage(char const* data, string& errors)
{
    return mReader->parse(data, data + strlen(data), &mJData, &errors);
}

void CommandAndStateMessageParser::validateFieldPresent(
    Json::Value const& value,
    string const& fieldName
)
{
    if (!value.isMember(fieldName))
    {
        throw invalid_argument("message does not contain the " + fieldName + " field");
    }
}

bool CommandAndStateMessageParser::checkDeviceMacAddress(string address)
{
    return mJData["devices"].isMember(address);
}

string CommandAndStateMessageParser::parseGetMessage()
{
    Json::Value message;
    message["apiVersion"] = "0.8.3";
    message["method"] = "GET";
    message["payload"] = {};

    return message.asString();
}

string CommandAndStateMessageParser::parseRevolutionCommandMessage(
    string address,
    RevolutionControlCommand command
)
{
    Json::Value message;
    message["apiVersion"] = "0.8.3";
    message["method"] = "SET";
    message["payload"]["devices"][address]["auxLights"] = command.light;
    message["payload"]["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]
           ["x"] = command.vehicle_setpoint.position[0];
    message["payload"]["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]
           ["y"] = command.vehicle_setpoint.position[1];
    message["payload"]["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]
           ["z"] = command.vehicle_setpoint.position[2];
    message["payload"]["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]
           ["yaw"] = command.vehicle_setpoint.yaw.rad;

    return message.asString();
}

string CommandAndStateMessageParser::parsePoweredReelCommandMessage(
    string address,
    PoweredReelControlCommand command
)
{
    Json::Value message;
    message["apiVersion"] = "0.8.3";
    message["method"] = "SET";
    message["payload"]["devices"][address]["reelFoward"] = command.reel_forward;
    message["payload"]["devices"][address]["reelReverse"] = command.reel_reverse;
    message["payload"]["devices"][address]["speed"] = command.speed.elements[0].speed;

    return message.asString();
}

string CommandAndStateMessageParser::parseGrabberCommandMessage(
    string address,
    GrabberCommand command
)
{
    Json::Value message;
    message["apiVersion"] = "0.8.3";
    message["method"] = "SET";
    message["payload"]["devices"][address]["grabber"]["openClose"] = command.open;
    message["payload"]["devices"][address]["grabber"]["rotate"] =
        command.speed.elements[0].speed;

    return message.asString();
}

string CommandAndStateMessageParser::parseTiltCameraHeadCommandMessage(
    string address,
    TiltCameraHeadCommand command
)
{
    Json::Value message;
    message["apiVersion"] = "0.8.3";
    message["method"] = "SET";
    message["payload"]["devices"][address]["cameraHead"]["lights"] = command.light;
    message["payload"]["devices"][address]["cameraHead"]["lasers"] = command.laser;
    message["payload"]["devices"][address]["cameraHead"]["tilt"] =
        command.speed.elements[0].speed;
    message["payload"]["devices"][address]["cameraHead"]["camera"]["exposure"] =
        command.camera.exposure;
    message["payload"]["devices"][address]["cameraHead"]["camera"]["brightness"] =
        command.camera.brightness;
    message["payload"]["devices"][address]["cameraHead"]["camera"]["focus"] =
        command.camera.focus;
    message["payload"]["devices"][address]["cameraHead"]["camera"]["saturation"] =
        command.camera.saturation;
    message["payload"]["devices"][address]["cameraHead"]["camera"]["sharpness"] =
        command.camera.sharpness;
    message["payload"]["devices"][address]["cameraHead"]["camera"]["zoom"]["ratio"] =
        command.camera.zoom.ratio;
    message["payload"]["devices"][address]["cameraHead"]["camera"]["zoom"]["speed"] =
        command.camera.zoom.speed.elements[0].speed;

    return message.asString();
}

Time CommandAndStateMessageParser::getTimeUsage(string address)
{
    double time = mJData["devices"][address]["currentSeconds"].asDouble();
    return Time::fromSeconds(time);
}

RovControl CommandAndStateMessageParser::getVehicleStates(string address)
{
    double setpoint_x =
        mJData["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]["x"]
            .asDouble();
    double setpoint_y =
        mJData["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]["y"]
            .asDouble();
    double setpoint_z =
        mJData["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]["z"]
            .asDouble();
    double setpoint_yaw =
        mJData["devices"][address]["control"]["setpoint"]["pose"]["localFrame"]["yaw"]
            .asDouble();
    double state_x =
        mJData["devices"][address]["control"]["current"]["pose"]["localFrame"]["x"]
            .asDouble();
    double state_y =
        mJData["devices"][address]["control"]["current"]["pose"]["localFrame"]["y"]
            .asDouble();
    double state_z =
        mJData["devices"][address]["control"]["current"]["pose"]["localFrame"]["z"]
            .asDouble();
    double state_yaw =
        mJData["devices"][address]["control"]["current"]["pose"]["localFrame"]["yaw"]
            .asDouble();

    RovControl control;
    control.vehicle_setpoint.position.x() = setpoint_x;
    control.vehicle_setpoint.position.y() = setpoint_y;
    control.vehicle_setpoint.position.z() = setpoint_z;
    control.vehicle_setpoint.orientation =
        Quaterniond(AngleAxisd(setpoint_yaw, Vector3d::UnitZ()));
    control.state_estimator.position.x() = state_x;
    control.state_estimator.position.y() = state_y;
    control.state_estimator.position.z() = state_z;
    control.state_estimator.orientation =
        Quaterniond(AngleAxisd(state_yaw, Vector3d::UnitZ()));

    return control;
}

Battery CommandAndStateMessageParser::getBatteryInfo(string address, string battery_side)
{
    Battery battery;
    battery.percentage = mJData["devices"][address][battery_side]["percent"].asDouble();
    battery.voltage = mJData["devices"][address][battery_side]["voltage"].asDouble();
    battery.charging = mJData["devices"][address][battery_side]["charging"].asDouble();

    return battery;
}

MotorDiagnostics
CommandAndStateMessageParser::getMotorInfo(string address, string motor_side)
{
    MotorDiagnostics motor;
    motor.current = mJData["devices"][address][motor_side]["current"].asDouble();
    motor.overcurrent = mJData["devices"][address][motor_side]["overcurrent"].asBool();
    vector<float> vector_motor;
    vector_motor.push_back(mJData["devices"][address][motor_side]["rpm"].asFloat());
    motor.motor = samples::Joints::Raw(vector_motor);

    return motor;
}

Grabber CommandAndStateMessageParser::getGrabberMotorInfo(string address)
{
    Grabber grabber;
    MotorDiagnostics motor;
    motor.current =
        mJData["devices"][address]["grabber"]["motorDiagnostics"]["current"].asDouble();
    motor.overcurrent =
        mJData["devices"][address]["grabber"]["motorDiagnostics"]["overcurrent"].asBool();
    vector<float> vector_motor;
    vector_motor.push_back(
        mJData["devices"][address]["grabber"]["motorDiagnostics"]["rpm"].asFloat()
    );
    motor.motor = samples::Joints::Raw(vector_motor);
    grabber.motor_diagnostic = motor;
    grabber.open = mJData["devices"][address]["grabber"]["openClose"].asDouble();
    vector<float> vector_rotate;
    vector_rotate.push_back(mJData["devices"][address]["grabber"]["rotate"].asFloat());
    grabber.rotate_command = samples::Joints::Raw(vector_rotate);

    return grabber;
}

TiltCameraHead CommandAndStateMessageParser::getCameraHeadInfo(string address)
{
    TiltCameraHead camera_head;
    camera_head.light = mJData["devices"][address]["cameraHead"]["lights"].asDouble();
    camera_head.laser = mJData["devices"][address]["cameraHead"]["lasers"].asBool();
    vector<float> tilt_command;
    tilt_command.push_back(
        mJData["devices"][address]["cameraHead"]["tilt"]["speed"].asFloat()
    );
    camera_head.tilt_command = samples::Joints::Speeds(tilt_command);
    MotorDiagnostics motor;
    motor.current =
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["current"]
            .asDouble();
    motor.overcurrent =
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["overcurrent"]
            .asBool();
    vector<float> vector_motor;
    vector_motor.push_back(
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["rpm"].asFloat()
    );
    motor.motor = samples::Joints::Raw(vector_motor);
    camera_head.tilt_motor_diagnostics = motor;
    camera_head.camera.brightness =
        mJData["devices"][address]["cameraHead"]["camera"]["brightness"].asDouble();
    camera_head.camera.exposure =
        mJData["devices"][address]["cameraHead"]["camera"]["exposure"]["value"].asDouble(
        );
    camera_head.camera.focus =
        mJData["devices"][address]["cameraHead"]["camera"]["focus"]["value"].asDouble();
    camera_head.camera.saturation =
        mJData["devices"][address]["cameraHead"]["camera"]["saturation"].asDouble();
    camera_head.camera.sharpness =
        mJData["devices"][address]["cameraHead"]["camera"]["sharpness"].asDouble();
    camera_head.camera.zoom.ratio =
        mJData["devices"][address]["cameraHead"]["camera"]["zoom"]["ratio"].asDouble();
    vector<float> vector_speed;
    vector_speed.push_back(
        mJData["devices"][address]["cameraHead"]["camera"]["zoom"]["speed"].asFloat()
    );
    camera_head.camera.zoom.speed = samples::Joints::Speeds(vector_speed);

    return camera_head;
}

double CommandAndStateMessageParser::getLightInfo(string address)
{
    return mJData["devices"][address]["auxLights"].asDouble();
}

double CommandAndStateMessageParser::getTetherDistanceInfo(string address)
{
    return mJData["devices"][address]["distance"].asDouble();
}

double CommandAndStateMessageParser::getTemperatureInfo(string address)
{
    return mJData["devices"][address]["cpuTemp"].asDouble();
}

bool CommandAndStateMessageParser::getCalibrateInfo(string address)
{
    return mJData["devices"][address]["calibrator"].asBool();
}

bool CommandAndStateMessageParser::getReadyInfo(string address)
{
    return mJData["devices"][address]["ready"].asBool();
}

bool CommandAndStateMessageParser::getLeakInfo(string address)
{
    return mJData["devices"][address]["leak"].asBool();
}

bool CommandAndStateMessageParser::getACPowerInfo(string address)
{
    return mJData["devices"][address]["acConnected"].asBool();
}

bool CommandAndStateMessageParser::getEStopInfo(string address)
{
    return mJData["devices"][address]["eStop"].asBool();
}

samples::Joints CommandAndStateMessageParser::getSpeedInfo(string address)
{
    vector<float> vector_speed;
    vector_speed.push_back(mJData["devices"][address]["speed"].asFloat());
    return samples::Joints::Speeds(vector_speed);
}

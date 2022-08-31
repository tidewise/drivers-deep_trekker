#include "CommandAndStateMessageParser.hpp"
#include <iostream>

using namespace std;
using namespace base;
using namespace power_base;
using namespace deep_trekker;

CommandAndStateMessageParser::CommandAndStateMessageParser()
    : mReader(mRBuilder.newCharReader())
{
}

bool CommandAndStateMessageParser::parseJSONMessage(char const* data, string& errors)
{
    return mReader->parse(data, data + strlen(data), &mJData, &errors);
}

void CommandAndStateMessageParser::validateFieldPresent(Json::Value const& value,
    string const& fieldName)
{
    if (!value.isMember(fieldName)) {
        throw invalid_argument("message does not contain the " + fieldName + " field");
    }
}

bool CommandAndStateMessageParser::checkDeviceMacAddress(string address)
{
    return mJData["devices"].isMember(address);
}

string CommandAndStateMessageParser::parseGetMessage(string api_version)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "GET";
    message["payload"] = {};

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseRevolutionCommandMessage(string api_version,
    string address,
    PositionAndLightCommand command)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    message["payload"]["devices"][address]["auxLights"] =
        min(max(command.light, 0.0), 1.0) * 100;
    auto local_frame = message["payload"]["devices"][address]["control"]["setpoint"]
                              ["pose"]["localFrame"];
    local_frame["x"] = command.vehicle_setpoint.linear[0];
    local_frame["y"] = command.vehicle_setpoint.linear[1];
    local_frame["z"] = command.vehicle_setpoint.linear[2];
    local_frame["yaw"] = command.vehicle_setpoint.angular[2] * 180 / M_PI;
    message["payload"]["devices"][address]["control"]["setpoint"]["pose"]["localFrame"] =
        local_frame;

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parsePoweredReelCommandMessage(string api_version,
    string address,
    PoweredReelControlCommand command)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    message["payload"]["devices"][address]["reelFoward"] = false;
    message["payload"]["devices"][address]["reelReverse"] = false;
    if (command.speed.elements[0].speed > 0) {
        message["payload"]["devices"][address]["reelFoward"] = true;
        message["payload"]["devices"][address]["reelReverse"] = false;
    }
    else if (command.speed.elements[0].speed < 0) {
        message["payload"]["devices"][address]["reelFoward"] = false;
        message["payload"]["devices"][address]["reelReverse"] = true;
    }
    message["payload"]["devices"][address]["speed"] =
        min(max(static_cast<double>(command.speed.elements[0].speed), -1.0), 1.0) * 100;
    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseGrabberCommandMessage(string api_version,
    string address,
    GrabberCommand command)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    message["payload"]["devices"][address]["grabber"]["openClose"] =
        min(max(static_cast<double>(command.open_close.elements[0].raw), -1.0), 1.0) * 100;
    message["payload"]["devices"][address]["grabber"]["rotate"] =
        min(max(static_cast<double>(command.rotate.elements[0].raw), -1.0), 1.0) * 100;

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseTiltCameraHeadCommandMessage(string api_version,
    string address,
    TiltCameraHeadCommand command)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    auto camera_head = message["payload"]["devices"][address]["cameraHead"];
    camera_head["lights"] = min(max(command.light, 0.0), 1.0) * 100;
    camera_head["lasers"] = command.laser;
    camera_head["tilt"]["position"] =
        min(max(command.tilt_command.elements[0].position, -M_PI), M_PI) * 180 / M_PI;
    camera_head["tilt"]["speed"] =
        min(max(static_cast<double>(command.tilt_command.elements[0].speed), -1.0), 1.0) * 100;
    camera_head["camera"]["exposure"] =
        min(max(static_cast<double>(command.camera.exposure), 0.0), 1.0) * 15;
    camera_head["camera"]["brightness"] =
        min(max(static_cast<double>(command.camera.brightness), 0.0), 1.0) * 100;
    camera_head["camera"]["focus"] =
        min(max(static_cast<double>(command.camera.focus), 0.0), 1.0) * 100;
    camera_head["camera"]["saturation"] =
        min(max(static_cast<double>(command.camera.saturation), 0.0), 1.0) * 100;
    camera_head["camera"]["sharpness"] =
        min(max(static_cast<double>(command.camera.sharpness), 0.0), 1.0) * 100;
    camera_head["camera"]["zoom"]["ratio"] = command.camera.zoom.ratio;
    camera_head["camera"]["zoom"]["speed"] =
        min(max(static_cast<double>(command.camera.zoom.speed), -1.0), 1.0) * 100;
    message["payload"]["devices"][address]["cameraHead"] = camera_head;

    Json::FastWriter fast;
    return fast.write(message);
}

Time CommandAndStateMessageParser::getTimeUsage(string address)
{
    double time = mJData["devices"][address]["currentSeconds"].asDouble();
    return Time::fromSeconds(time);
}

RovControl CommandAndStateMessageParser::getVehicleStates(string address)
{
    auto msg_setpoint =
        mJData["devices"][address]["control"]["setpoint"]["pose"]["localFrame"];
    double setpoint_x = msg_setpoint["x"].asDouble();
    double setpoint_y = msg_setpoint["y"].asDouble();
    double setpoint_z = msg_setpoint["z"].asDouble();
    double setpoint_yaw = msg_setpoint["yaw"].asDouble() * M_PI / 180;
    auto local_frame =
        mJData["devices"][address]["control"]["current"]["pose"]["localFrame"];
    double state_x = local_frame["x"].asDouble();
    double state_y = local_frame["y"].asDouble();
    double state_z = local_frame["z"].asDouble();
    double state_yaw = local_frame["yaw"].asDouble() * M_PI / 180;

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

BatteryStatus CommandAndStateMessageParser::getBatteryStates(string address,
    string battery_side)
{
    BatteryStatus battery;
    battery.charge = mJData["devices"][address][battery_side]["percent"].asDouble() / 100;

    return battery;
}

MotorDiagnostics CommandAndStateMessageParser::getMotorStates(string address,
    string motor_side)
{
    MotorDiagnostics motor;
    motor.overcurrent = mJData["devices"][address][motor_side]["overcurrent"].asBool();
    vector<float> vector_pwm_motor;
    vector_pwm_motor.push_back(
        mJData["devices"][address][motor_side]["pwm"].asFloat() / 100);
    motor.motor = samples::Joints::Raw(vector_pwm_motor);
    vector<float> vector_current_motor;
    vector_current_motor.push_back(
        mJData["devices"][address][motor_side]["current"].asDouble());
    motor.motor = samples::Joints::Efforts(vector_current_motor);
    vector<float> vector_rad_motor;
    vector_rad_motor.push_back(
        mJData["devices"][address][motor_side]["rpm"].asFloat() * 2 * M_PI / 60);
    motor.motor = samples::Joints::Speeds(vector_rad_motor);

    return motor;
}

Grabber CommandAndStateMessageParser::getGrabberMotorStates(string address)
{
    Grabber grabber;
    MotorDiagnostics open_close_motor;
    open_close_motor.overcurrent =
        mJData["devices"][address]["grabber"]["openCloseMotorDiagnostics"]["overcurrent"]
            .asBool();
    vector<float> vector_open_pwm_motor;
    vector_open_pwm_motor.push_back(
        mJData["devices"]["openCloseMotorDiagnostics"]["grabber"]["pwm"].asFloat() / 100);
    open_close_motor.motor = samples::Joints::Raw(vector_open_pwm_motor);
    vector<float> vector_open_current_motor;
    vector_open_current_motor.push_back(
        mJData["devices"][address]["grabber"]["openCloseMotorDiagnostics"]["current"]
            .asDouble());
    open_close_motor.motor = samples::Joints::Efforts(vector_open_current_motor);
    vector<float> vector_open_rad_motor;
    vector_open_rad_motor.push_back(
        mJData["devices"][address]["grabber"]["openCloseMotorDiagnostics"]["rpm"]
            .asFloat() *
        2 * M_PI / 60);
    open_close_motor.motor = samples::Joints::Speeds(vector_open_rad_motor);
    grabber.open_close_diagnostics = open_close_motor;
    MotorDiagnostics rotate_motor;
    rotate_motor.overcurrent =
        mJData["devices"][address]["grabber"]["rollMotorDiagnostics"]["overcurrent"]
            .asBool();
    vector<float> vector_rotate_pwm_motor;
    vector_rotate_pwm_motor.push_back(
        mJData["devices"]["rollMotorDiagnostics"]["grabber"]["pwm"].asFloat() / 100);
    rotate_motor.motor = samples::Joints::Raw(vector_rotate_pwm_motor);
    vector<float> vector_rotate_current_motor;
    vector_rotate_current_motor.push_back(
        mJData["devices"][address]["grabber"]["rollMotorDiagnostics"]["current"]
            .asDouble());
    rotate_motor.motor = samples::Joints::Efforts(vector_rotate_current_motor);
    vector<float> vector_rotate_rad_motor;
    vector_rotate_rad_motor.push_back(
        mJData["devices"][address]["grabber"]["rollMotorDiagnostics"]["rpm"].asFloat() *
        2 * M_PI / 60);
    rotate_motor.motor = samples::Joints::Speeds(vector_rotate_rad_motor);
    grabber.rotate_diagnostics = rotate_motor;

    return grabber;
}

TiltCameraHead CommandAndStateMessageParser::getCameraHeadStates(string address)
{
    TiltCameraHead camera_head;
    camera_head.light =
        mJData["devices"][address]["cameraHead"]["lights"].asDouble() / 100;
    camera_head.laser = mJData["devices"][address]["cameraHead"]["lasers"].asBool();
    vector<float> tilt_speed_command;
    tilt_speed_command.push_back(
        mJData["devices"][address]["cameraHead"]["tilt"]["speed"].asFloat() / 100);
    camera_head.tilt_command = samples::Joints::Speeds(tilt_speed_command);
    vector<double> tilt_position_command;
    tilt_speed_command.push_back(
        mJData["devices"][address]["cameraHead"]["tilt"]["position"].asFloat() * M_PI /
        180);
    camera_head.tilt_command = samples::Joints::Positions(tilt_position_command);
    MotorDiagnostics tilt_motor;
    tilt_motor.overcurrent =
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["overcurrent"]
            .asBool();
    vector<float> vector_pwm_motor;
    vector_pwm_motor.push_back(
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["pwm"]
            .asFloat() /
        100);
    tilt_motor.motor = samples::Joints::Raw(vector_pwm_motor);
    vector<float> vector_current_motor;
    vector_current_motor.push_back(
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["current"]
            .asDouble());
    tilt_motor.motor = samples::Joints::Efforts(vector_current_motor);
    vector<float> vector_rad_motor;
    vector_rad_motor.push_back(
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["rpm"]
            .asFloat() *
        2 * M_PI / 60);
    tilt_motor.motor = samples::Joints::Speeds(vector_rad_motor);
    camera_head.tilt_motor_diagnostics = tilt_motor;
    camera_head.camera.brightness =
        mJData["devices"][address]["cameraHead"]["camera"]["brightness"].asFloat() / 100;
    camera_head.camera.exposure =
        mJData["devices"][address]["cameraHead"]["camera"]["exposure"]["value"]
            .asFloat() /
        15;
    camera_head.camera.focus =
        mJData["devices"][address]["cameraHead"]["camera"]["focus"]["value"].asFloat() /
        100;
    camera_head.camera.saturation =
        mJData["devices"][address]["cameraHead"]["camera"]["saturation"].asFloat() / 100;
    camera_head.camera.sharpness =
        mJData["devices"][address]["cameraHead"]["camera"]["sharpness"].asFloat() / 100;
    camera_head.camera.zoom.ratio =
        mJData["devices"][address]["cameraHead"]["camera"]["zoom"]["ratio"].asFloat();
    camera_head.camera.zoom.speed =
        mJData["devices"][address]["cameraHead"]["camera"]["zoom"]["speed"].asFloat() /
        100;

    return camera_head;
}

double CommandAndStateMessageParser::getLightIntensity(string address)
{
    return mJData["devices"][address]["auxLights"].asDouble() / 100;
}

double CommandAndStateMessageParser::getTetherLenght(string address)
{
    return mJData["devices"][address]["distance"].asDouble();
}

double CommandAndStateMessageParser::getCpuTemperature(string address)
{
    return mJData["devices"][address]["cpuTemp"].asDouble();
}

bool CommandAndStateMessageParser::isCalibrated(string address)
{
    return mJData["devices"][address]["calibrator"].asBool();
}

bool CommandAndStateMessageParser::isReady(string address)
{
    return mJData["devices"][address]["ready"].asBool();
}

bool CommandAndStateMessageParser::isLeaking(string address)
{
    return mJData["devices"][address]["leak"].asBool();
}

bool CommandAndStateMessageParser::isACPowerConnected(string address)
{
    return mJData["devices"][address]["acConnected"].asBool();
}

bool CommandAndStateMessageParser::isEStopEnabled(string address)
{
    return mJData["devices"][address]["eStop"].asBool();
}

samples::Joints CommandAndStateMessageParser::getPoweredReelSpeed(string address)
{
    vector<float> vector_speed;
    vector_speed.push_back(mJData["devices"][address]["speed"].asFloat() / 100);
    return samples::Joints::Speeds(vector_speed);
}

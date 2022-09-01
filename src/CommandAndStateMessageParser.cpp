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
    local_frame["yaw"] = command.vehicle_setpoint.angular[2];
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
    if (command.elements[0].speed > 0) {
        message["payload"]["devices"][address]["reelFoward"] = true;
        message["payload"]["devices"][address]["reelReverse"] = false;
    }
    else if (command.elements[0].speed < 0) {
        message["payload"]["devices"][address]["reelFoward"] = false;
        message["payload"]["devices"][address]["reelReverse"] = true;
    }
    message["payload"]["devices"][address]["speed"] =
        min(max(static_cast<double>(command.elements[0].speed), -1.0), 1.0) * 100;
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
        min(max(static_cast<double>(command.elements[0].raw), -1.0), 1.0) * 100;
    message["payload"]["devices"][address]["grabber"]["rotate"] =
        min(max(static_cast<double>(command.elements[1].raw), -1.0), 1.0) * 100;

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseTiltCameraHeadCommandMessage(string api_version,
    string address,
    CameraHeadCommand head,
    TiltCameraHeadCommand tilt)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    auto camera_head = message["payload"]["devices"][address]["cameraHead"];
    camera_head["lights"] = min(max(head.light, 0.0), 1.0) * 100;
    camera_head["lasers"] = head.laser;
    camera_head["tilt"]["position"] =
        min(max(tilt.elements[0].position, -M_PI), M_PI);
    camera_head["tilt"]["speed"] =
        min(max(static_cast<double>(tilt.elements[0].speed), -1.0), 1.0) * 100;
    camera_head["camera"]["exposure"] =
        min(max(static_cast<double>(head.camera.exposure), 0.0), 1.0) * 15;
    camera_head["camera"]["brightness"] =
        min(max(static_cast<double>(head.camera.brightness), 0.0), 1.0) * 100;
    camera_head["camera"]["focus"] =
        min(max(static_cast<double>(head.camera.focus), 0.0), 1.0) * 100;
    camera_head["camera"]["saturation"] =
        min(max(static_cast<double>(head.camera.saturation), 0.0), 1.0) * 100;
    camera_head["camera"]["sharpness"] =
        min(max(static_cast<double>(head.camera.sharpness), 0.0), 1.0) * 100;
    camera_head["camera"]["zoom"]["ratio"] = head.camera.zoom.ratio;
    camera_head["camera"]["zoom"]["speed"] =
        min(max(static_cast<double>(head.camera.zoom.speed), -1.0), 1.0) * 100;
    message["payload"]["devices"][address]["cameraHead"] = camera_head;

    Json::FastWriter fast;
    return fast.write(message);
}

Time CommandAndStateMessageParser::getTimeUsage(string address)
{
    double time = mJData["devices"][address]["currentSeconds"].asDouble();
    return Time::fromSeconds(time);
}

RevolutionControl CommandAndStateMessageParser::getRevolutionControlStates(string address)
{
    auto msg_setpoint =
        mJData["devices"][address]["control"]["setpoint"]["pose"]["localFrame"];
    double setpoint_x = msg_setpoint["x"].asDouble();
    double setpoint_y = msg_setpoint["y"].asDouble();
    double setpoint_z = msg_setpoint["z"].asDouble();
    double setpoint_yaw = msg_setpoint["yaw"].asDouble();

    RevolutionControl control;
    control.position.x() = setpoint_x;
    control.position.y() = setpoint_y;
    control.position.z() = setpoint_z;
    control.orientation = Quaterniond(AngleAxisd(setpoint_yaw, Vector3d::UnitZ()));

    return control;
}

RevolutionBodyStates CommandAndStateMessageParser::getRevolutionBodyStates(string address)
{
    auto local_frame =
        mJData["devices"][address]["control"]["current"]["pose"]["localFrame"];
    double state_x = local_frame["x"].asDouble();
    double state_y = local_frame["y"].asDouble();
    double state_z = local_frame["z"].asDouble();
    double state_yaw = local_frame["yaw"].asDouble();

    RevolutionBodyStates control;
    control.position.x() = state_x;
    control.position.y() = state_y;
    control.position.z() = state_z;
    control.orientation = Quaterniond(AngleAxisd(state_yaw, Vector3d::UnitZ()));

    return control;
}

PoweredReelMotorStates CommandAndStateMessageParser::getPoweredReelMotorState(
    string address)
{
    JointState state;
    state.raw = mJData["devices"][address]["motor1Diagnostics"]["pwm"].asFloat() / 100;
    state.effort = mJData["devices"][address]["motor1Diagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["motor1Diagnostics"]["rpm"].asFloat() * 2 * M_PI / 60;
    PoweredReelMotorStates powered;
    powered.elements.push_back(state);
    state.raw = mJData["devices"][address]["motor2Diagnostics"]["pwm"].asFloat() / 100;
    state.effort = mJData["devices"][address]["motor2Diagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["motor2Diagnostics"]["rpm"].asFloat() * 2 * M_PI / 60;
    powered.elements.push_back(state);

    return powered;
}

RevolutionMotorStates CommandAndStateMessageParser::getRevolutionMotorStates(
    string address)
{
    JointState state;
    state.raw =
        mJData["devices"][address]["frontRightMotorDiagnostics"]["pwm"].asFloat() / 100;
    state.effort =
        mJData["devices"][address]["frontRightMotorDiagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["frontRightMotorDiagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    RevolutionMotorStates revolution;
    revolution.elements.push_back(state);
    state.raw =
        mJData["devices"][address]["frontLeftMotorDiagnostics"]["pwm"].asFloat() / 100;
    state.effort =
        mJData["devices"][address]["frontLeftMotorDiagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["frontLeftMotorDiagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    revolution.elements.push_back(state);
    state.raw =
        mJData["devices"][address]["rearRightMotorDiagnostics"]["pwm"].asFloat() / 100;
    state.effort =
        mJData["devices"][address]["rearRightMotorDiagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["rearRightMotorDiagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    revolution.elements.push_back(state);
    state.raw =
        mJData["devices"][address]["rearLeftMotorDiagnostics"]["pwm"].asFloat() / 100;
    state.effort =
        mJData["devices"][address]["rearLeftMotorDiagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["rearLeftMotorDiagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    revolution.elements.push_back(state);
    state.raw =
        mJData["devices"][address]["verticalRightMotorDiagnostics"]["pwm"].asFloat() /
        100;
    state.effort =
        mJData["devices"][address]["verticalRightMotorDiagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["verticalRightMotorDiagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    revolution.elements.push_back(state);
    state.raw =
        mJData["devices"][address]["verticalLeftMotorDiagnostics"]["pwm"].asFloat() / 100;
    state.effort =
        mJData["devices"][address]["verticalLeftMotorDiagnostics"]["current"].asDouble();
    state.speed =
        mJData["devices"][address]["verticalLeftMotorDiagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    revolution.elements.push_back(state);

    return revolution;
}

BatteryStatus CommandAndStateMessageParser::getBatteryStates(string address,
    string battery_side)
{
    BatteryStatus battery;
    battery.charge = mJData["devices"][address][battery_side]["percent"].asDouble() / 100;

    return battery;
}

Grabber CommandAndStateMessageParser::getGrabberMotorOvercurrentStates(string address)
{
    Grabber grabber;
    grabber.open_close_motor_overcurrent =
        mJData["devices"][address]["grabber"]["openCloseMotorDiagnostics"]["overcurrent"]
            .asBool();
    grabber.rotate_overcurrent =
        mJData["devices"][address]["grabber"]["rollMotorDiagnostics"]["overcurrent"]
            .asBool();

    return grabber;
}

GrabberMotorStates CommandAndStateMessageParser::getGrabberMotorStates(string address)
{
    GrabberMotorStates grabber;
    JointState open_close_joint_state;
    open_close_joint_state.raw =
        mJData["devices"]["openCloseMotorDiagnostics"]["grabber"]["pwm"].asFloat() / 100;
    open_close_joint_state.speed =
        mJData["devices"][address]["grabber"]["openCloseMotorDiagnostics"]["rpm"]
            .asFloat() *
        2 * M_PI / 60;
    open_close_joint_state.effort =
        mJData["devices"][address]["grabber"]["openCloseMotorDiagnostics"]["current"]
            .asDouble();
    grabber.elements.push_back(open_close_joint_state);
    JointState rotate_joint;
    rotate_joint.raw =
        mJData["devices"]["rollMotorDiagnostics"]["grabber"]["pwm"].asFloat() / 100;
    rotate_joint.speed =
        mJData["devices"][address]["grabber"]["rollMotorDiagnostics"]["rpm"].asFloat() *
        2 * M_PI / 60;
    rotate_joint.effort =
        mJData["devices"][address]["grabber"]["rollMotorDiagnostics"]["current"]
            .asDouble();
    grabber.elements.push_back(rotate_joint);

    return grabber;
}

TiltCameraHead CommandAndStateMessageParser::getCameraHeadStates(string address)
{
    TiltCameraHead camera_head;
    camera_head.light =
        mJData["devices"][address]["cameraHead"]["lights"].asDouble() / 100;
    camera_head.laser = mJData["devices"][address]["cameraHead"]["lasers"].asBool();
    camera_head.motor_overcurrent =
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["overcurrent"]
            .asBool();
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

TiltCameraHeadMotorState CommandAndStateMessageParser::getCameraHeadMotorStates(
    string address)
{
    TiltCameraHeadMotorState camera_head;
    JointState tilt_motor;
    tilt_motor.raw =
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["pwm"]
            .asFloat() /
        100;
    tilt_motor.speed =
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["rpm"]
            .asFloat() *
        2 * M_PI / 60;
    tilt_motor.effort =
        mJData["devices"][address]["cameraHead"]["tiltMotorDiagnostics"]["current"]
            .asDouble();
    camera_head.elements.push_back(tilt_motor);

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

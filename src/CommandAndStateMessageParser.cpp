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
    return mJData["payload"]["devices"].isMember(address);
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
    samples::Joints command)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    auto root = message["payload"]["devices"][address];
    root["reelFoward"] = false;
    root["reelReverse"] = false;
    if (command.elements[0].speed > 0) {
        root["reelFoward"] = true;
        root["reelReverse"] = false;
    }
    else if (command.elements[0].speed < 0) {
        root["reelFoward"] = false;
        root["reelReverse"] = true;
    }
    root["speed"] =
        min(max(static_cast<double>(command.elements[0].speed), -1.0), 1.0) * 100;
    message["payload"]["devices"][address] = root;
    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseGrabberCommandMessage(string api_version,
    string address,
    samples::Joints command)
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
    samples::Joints tilt)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    auto camera_head = message["payload"]["devices"][address]["cameraHead"];
    camera_head["lights"] = min(max(head.light, 0.0), 1.0) * 100;
    camera_head["lasers"] = head.laser;
    camera_head["tilt"]["position"] = min(max(tilt.elements[0].position, -M_PI), M_PI);
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
    double time = mJData["payload"]["devices"][address]["currentSeconds"].asDouble();
    return Time::fromSeconds(time);
}

samples::RigidBodyState CommandAndStateMessageParser::getRevolutionControlStates(
    string address)
{
    auto msg_setpoint = mJData["payload"]["devices"][address]["control"]["setpoint"]
                              ["pose"]["localFrame"];
    double setpoint_x = msg_setpoint["x"].asDouble();
    double setpoint_y = msg_setpoint["y"].asDouble();
    double setpoint_z = msg_setpoint["z"].asDouble();
    double setpoint_yaw = msg_setpoint["yaw"].asDouble();

    samples::RigidBodyState control;
    control.position.x() = setpoint_x;
    control.position.y() = setpoint_y;
    control.position.z() = setpoint_z;
    control.orientation = Quaterniond(AngleAxisd(setpoint_yaw, Vector3d::UnitZ()));

    return control;
}

samples::RigidBodyState CommandAndStateMessageParser::getRevolutionBodyStates(
    string address)
{
    auto local_frame =
        mJData["payload"]["devices"][address]["control"]["current"]["pose"]["localFrame"];
    double state_x = local_frame["x"].asDouble();
    double state_y = local_frame["y"].asDouble();
    double state_z = local_frame["z"].asDouble();
    double state_yaw = local_frame["yaw"].asDouble();

    samples::RigidBodyState control;
    control.position.x() = state_x;
    control.position.y() = state_y;
    control.position.z() = state_z;
    control.orientation = Quaterniond(AngleAxisd(state_yaw, Vector3d::UnitZ()));

    return control;
}

samples::Joints CommandAndStateMessageParser::getPoweredReelMotorState(string address)
{
    JointState state;
    state.raw =
        mJData["payload"]["devices"][address]["motor1Diagnostics"]["pwm"].asFloat() / 100;
    state.effort =
        mJData["payload"]["devices"][address]["motor1Diagnostics"]["current"].asDouble();
    state.speed =
        mJData["payload"]["devices"][address]["motor1Diagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    samples::Joints powered;
    powered.elements.push_back(state);
    state.raw =
        mJData["payload"]["devices"][address]["motor2Diagnostics"]["pwm"].asFloat() / 100;
    state.effort =
        mJData["payload"]["devices"][address]["motor2Diagnostics"]["current"].asDouble();
    state.speed =
        mJData["payload"]["devices"][address]["motor2Diagnostics"]["rpm"].asFloat() * 2 *
        M_PI / 60;
    powered.elements.push_back(state);

    return powered;
}

samples::Joints CommandAndStateMessageParser::getRevolutionMotorStates(string address)
{
    auto root = mJData["payload"]["devices"][address]["frontRightMotorDiagnostics"];
    JointState state = motorDiagnosticsToJointState(root);
    samples::Joints revolution;
    revolution.elements.push_back(state);
    root = mJData["payload"]["devices"][address]["frontLeftMotorDiagnostics"];
    state = motorDiagnosticsToJointState(root);
    revolution.elements.push_back(state);
    root = mJData["payload"]["devices"][address]["rearRightMotorDiagnostics"];
    state = motorDiagnosticsToJointState(root);
    revolution.elements.push_back(state);
    root = mJData["payload"]["devices"][address]["rearLeftMotorDiagnostics"];
    state = motorDiagnosticsToJointState(root);
    revolution.elements.push_back(state);
    root = mJData["payload"]["devices"][address]["verticalRightMotorDiagnostics"];
    state = motorDiagnosticsToJointState(root);
    revolution.elements.push_back(state);
    root = mJData["payload"]["devices"][address]["verticalLeftMotorDiagnostics"];
    state = motorDiagnosticsToJointState(root);
    revolution.elements.push_back(state);

    return revolution;
}

BatteryStatus CommandAndStateMessageParser::getBatteryStates(string address,
    string battery_side)
{
    BatteryStatus battery;
    battery.charge =
        mJData["payload"]["devices"][address][battery_side]["percent"].asDouble() / 100;

    return battery;
}

Grabber CommandAndStateMessageParser::getGrabberMotorOvercurrentStates(string address)
{
    Grabber grabber;
    grabber.open_close_motor_overcurrent =
        mJData["payload"]["devices"][address]["grabber"]["openCloseMotorDiagnostics"]
              ["overcurrent"]
                  .asBool();
    grabber.rotate_overcurrent = mJData["payload"]["devices"][address]["grabber"]
                                       ["rollMotorDiagnostics"]["overcurrent"]
                                           .asBool();

    return grabber;
}

samples::Joints CommandAndStateMessageParser::getGrabberMotorStates(string address)
{
    auto root =
        mJData["payload"]["devices"][address]["openCloseMotorDiagnostics"]["grabber"];
    JointState open_close_joint_state = motorDiagnosticsToJointState(root);
    samples::Joints grabber;
    grabber.elements.push_back(open_close_joint_state);
    root = mJData["payload"]["devices"][address]["grabber"]["rollMotorDiagnostics"];
    JointState rotate_joint = motorDiagnosticsToJointState(root);
    grabber.elements.push_back(rotate_joint);

    return grabber;
}

TiltCameraHead CommandAndStateMessageParser::getCameraHeadStates(string address)
{
    TiltCameraHead camera_head;
    auto root = mJData["payload"]["devices"][address]["cameraHead"];
    camera_head.light = root["lights"].asDouble() / 100;
    camera_head.laser = root["lasers"].asBool();
    camera_head.motor_overcurrent = root["tiltMotorDiagnostics"]["overcurrent"].asBool();
    camera_head.camera.brightness = root["camera"]["brightness"].asFloat() / 100;
    camera_head.camera.exposure = root["camera"]["exposure"]["value"].asFloat() / 15;
    camera_head.camera.focus = root["camera"]["focus"]["value"].asFloat() / 100;
    camera_head.camera.saturation = root["camera"]["saturation"].asFloat() / 100;
    camera_head.camera.sharpness = root["camera"]["sharpness"].asFloat() / 100;
    camera_head.camera.zoom.ratio = root["camera"]["zoom"]["ratio"].asFloat();
    camera_head.camera.zoom.speed = root["camera"]["zoom"]["speed"].asFloat() / 100;

    return camera_head;
}

samples::Joints CommandAndStateMessageParser::getCameraHeadMotorStates(string address)
{
    auto root =
        mJData["payload"]["devices"][address]["cameraHead"]["tiltMotorDiagnostics"];
    JointState tilt_motor = motorDiagnosticsToJointState(root);
    samples::Joints camera_head;
    camera_head.elements.push_back(tilt_motor);

    return camera_head;
}

JointState CommandAndStateMessageParser::motorDiagnosticsToJointState(Json::Value value)
{
    JointState joint_state;
    joint_state.raw = value["pwm"].asFloat() / 100;
    joint_state.speed = value["rpm"].asFloat() * 2 * M_PI / 60;
    joint_state.effort = value["current"].asDouble();

    return joint_state;
}

bool CommandAndStateMessageParser::getMotorOvercurrentStates(string address,
    string motor_side)
{
    return mJData["payload"]["devices"][address][motor_side]["overcurrent"].asBool();
}

double CommandAndStateMessageParser::getLightIntensity(string address)
{
    return mJData["payload"]["devices"][address]["auxLights"].asDouble() / 100;
}

double CommandAndStateMessageParser::getTetherLenght(string address)
{
    return mJData["payload"]["devices"][address]["distance"].asDouble();
}

double CommandAndStateMessageParser::getCpuTemperature(string address)
{
    return mJData["payload"]["devices"][address]["cpuTemp"].asDouble();
}

bool CommandAndStateMessageParser::isCalibrated(string address)
{
    return mJData["payload"]["devices"][address]["calibrator"].asBool();
}

bool CommandAndStateMessageParser::isReady(string address)
{
    return mJData["payload"]["devices"][address]["ready"].asBool();
}

bool CommandAndStateMessageParser::isLeaking(string address)
{
    return mJData["payload"]["devices"][address]["leak"].asBool();
}

bool CommandAndStateMessageParser::isACPowerConnected(string address)
{
    return mJData["payload"]["devices"][address]["acConnected"].asBool();
}

bool CommandAndStateMessageParser::isEStopEnabled(string address)
{
    return mJData["payload"]["devices"][address]["eStop"].asBool();
}

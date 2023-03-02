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
    return mReader->parse(data, data + strlen(data), &m_json_data, &errors);
}

void CommandAndStateMessageParser::validateFieldPresent(Json::Value const& value,
    string const& fieldName)
{
    if (!value.isMember(fieldName)) {
        throw invalid_argument("message does not contain the " + fieldName + " field");
    }
}

void CommandAndStateMessageParser::validateMotorOverCurrentStates(string motor_field_name,
    string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id][motor_field_name],
        "overcurrent");
}

void CommandAndStateMessageParser::validateBatteryStates(string battery_field_name,
    string device_id)
{
    auto battery = m_json_data["payload"]["devices"][device_id][battery_field_name];
    validateFieldPresent(battery, "percent");
    validateFieldPresent(battery, "voltage");
}

void CommandAndStateMessageParser::validateAuxLightIntensity(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id]["auxLight"],
        "intensity");
}

void CommandAndStateMessageParser::validateMotorStates(string device_id,
    string motor_field_name)
{
    auto root = m_json_data["payload"]["devices"][device_id][motor_field_name];
    validateFieldPresent(root, "pwm");
    validateFieldPresent(root, "current");
    validateFieldPresent(root, "rpm");
}

void CommandAndStateMessageParser::validatePoweredReelMotorState(string device_id) {
    validateMotorStates(device_id, "motor1Diagnostics");
    validateMotorStates(device_id, "motor2Diagnostics");
}

void CommandAndStateMessageParser::validateGrabberMotorsStates(string device_id)
{
    auto grabber = m_json_data["payload"]["devices"][device_id]["grabber"];
    validateFieldPresent(grabber["openCloseMotorDiagnostics"], "overcurrent");
    validateMotorStates(device_id, "openCloseMotorDiagnostics");
    validateMotorStates(device_id, "rotateMotorDiagnostics");
    validateFieldPresent(grabber["rotateMotorDiagnostics"], "overcurrent");
}

void CommandAndStateMessageParser::validateCameraHeadStates(string device_id)
{
    auto camera_head = m_json_data["payload"]["devices"][device_id]["cameraHead"];
    validateFieldPresent(camera_head["light"], "intensity");
    validateFieldPresent(camera_head["lasers"], "enabled");
    validateFieldPresent(camera_head["tilt"], "position");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"], "overcurrent");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"], "pwm");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"], "rpm");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"], "current");
    validateFieldPresent(camera_head, "leak");
}

void CommandAndStateMessageParser::validateCameras(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "cameras");
}

void CommandAndStateMessageParser::validateCPUTemperature(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "cpuTemp");
}

void CommandAndStateMessageParser::validateDriveStates(string device_id)
{
    auto thrust = m_json_data["payload"]["devices"][device_id]["drive"]["thrust"];
    validateFieldPresent(thrust, "forward");
    validateFieldPresent(thrust, "lateral");
    validateFieldPresent(thrust, "vertical");
    validateFieldPresent(thrust, "yaw");
}

void CommandAndStateMessageParser::validateDriveModes(string device_id)
{
    auto modes = m_json_data["payload"]["devices"][device_id]["drive"]["modes"];
    validateFieldPresent(modes, "autoStabilization");
    validateFieldPresent(modes, "motorsDisabled");
    validateFieldPresent(modes, "altitudeLock");
    validateFieldPresent(modes, "depthLock");
    validateFieldPresent(modes, "headingLock");
}

void CommandAndStateMessageParser::validateLeaking(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "leak");
}

void CommandAndStateMessageParser::validateACConnected(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "acConnected");
}

void CommandAndStateMessageParser::validateEStop(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "eStop");
}

void CommandAndStateMessageParser::validateDistance(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "distance");
}

void CommandAndStateMessageParser::validateTimeUsage(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id]["usageTime"],
        "currentSeconds");
}

void CommandAndStateMessageParser::validateRevolutionMotorStates(string device_id)
{
    auto device = m_json_data["payload"]["devices"][device_id];
    vector<string> motors{"frontLeftMotorDignostics",
        "frontRightMotorDiagnostics",
        "rearLeftMotorDiagnostics",
        "rearRightMotorDiagnostics",
        "verticalLeftMotorDiagnostics",
        "verticalRightMotorDiagnostics"};
    for (auto motor : motors) {
        validateFieldPresent(device[motor], "pwm");
        validateFieldPresent(device[motor], "current");
        validateFieldPresent(device[motor], "rpm");
    }
}

Json::Value CommandAndStateMessageParser::payloadSetMessageTemplate(string api_version,
    string address,
    int model)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    message["payload"]["devices"][address]["model"] = model;
    return message;
}

string CommandAndStateMessageParser::parseDriveModeRevolutionCommandMessage(
    string api_version,
    string address,
    int model,
    DriveMode command)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto root = message["payload"]["devices"][address];

    auto modes = root["drive"]["modes"];
    modes["altitudeLock"] = command.altitude_lock;
    modes["autoStabilization"] = command.auto_stabilization;
    modes["depthLock"] = command.depth_lock;
    modes["headingLock"] = command.heading_lock;
    modes["motorsDisabled"] = command.motors_disabled;
    root["drive"]["modes"] = modes;

    message["payload"]["devices"][address] = root;
    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseDriveRevolutionCommandMessage(
    string api_version,
    string address,
    int model,
    MotionAndLightCommand command)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto root = message["payload"]["devices"][address];

    auto thrust = root["drive"]["thrust"];
    thrust["forward"] = command.vehicle_setpoint.linear[0];
    thrust["lateral"] = command.vehicle_setpoint.linear[1];
    thrust["vertical"] = command.vehicle_setpoint.linear[2];
    thrust["yaw"] = command.vehicle_setpoint.angular[2];
    root["drive"]["thrust"] = thrust;

    message["payload"]["devices"][address] = root;
    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parsePositionRevolutionCommandMessage(
    string api_version,
    string address,
    MotionAndLightCommand command)
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

string CommandAndStateMessageParser::parseVelocityRevolutionCommandMessage(
    string api_version,
    string address,
    MotionAndLightCommand command)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    message["payload"]["devices"][address]["auxLights"] =
        min(max(command.light, 0.0), 1.0) * 100;
    auto vel = message["payload"]["devices"][address]["control"]["setpoint"]["velocity"];
    vel["x"] = command.vehicle_setpoint.linear[0];
    vel["y"] = command.vehicle_setpoint.linear[1];
    vel["z"] = command.vehicle_setpoint.linear[2];
    vel["yaw"] = command.vehicle_setpoint.angular[2];
    message["payload"]["devices"][address]["control"]["setpoint"]["velocity"] = vel;

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseAccelerationRevolutionCommandMessage(
    string api_version,
    string address,
    MotionAndLightCommand command)
{
    Json::Value message;
    message["apiVersion"] = api_version;
    message["method"] = "SET";
    message["payload"]["devices"][address]["auxLights"] =
        min(max(command.light, 0.0), 1.0) * 100;
    auto acc =
        message["payload"]["devices"][address]["control"]["setpoint"]["acceleration"];
    acc["x"] = command.vehicle_setpoint.linear[0];
    acc["y"] = command.vehicle_setpoint.linear[1];
    acc["z"] = command.vehicle_setpoint.linear[2];
    acc["yaw"] = command.vehicle_setpoint.angular[2];
    message["payload"]["devices"][address]["control"]["setpoint"]["acceleration"] = acc;

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parsePoweredReelCommandMessage(string api_version,
    string address,
    int model,
    samples::Joints command)
{
    Json::Value message = payloadSetMessageTemplate(api_version, address, model);
    auto root = message["payload"]["devices"][address];
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
    int model,
    CameraHeadCommand head,
    samples::Joints tilt)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto camera_head = message["payload"]["devices"][address]["cameraHead"];
    camera_head["model"] = model;
    camera_head["lights"] = min(max(head.light, 0.0), 1.0) * 100;
    camera_head["lasers"] = head.laser;
    camera_head["tilt"]["speed"] =
        min(max(static_cast<double>(tilt.elements[0].speed), -1.0), 1.0) * 100;
    message["payload"]["devices"][address]["cameraHead"] = camera_head;

    Json::FastWriter fast;
    return fast.write(message);
}

Time CommandAndStateMessageParser::getTimeUsage(string address)
{
    validateTimeUsage(address);
    double time =
        m_json_data["payload"]["devices"][address]["usageTime"]["currentSeconds"]
            .asDouble();
    return Time::fromSeconds(time);
}

void CommandAndStateMessageParser::getDevicesID(DevicesModel const& models,
    DevicesID& ids)
{
    auto devices = m_json_data["payload"]["devices"];
    for (auto key : devices.getMemberNames()) {
        auto model = (devices[key]["model"].asInt());
        if (model == models.revolution && ids.revolution.empty()) {
            ids.revolution = key;
        }
        if (model == models.manual_reel && ids.manual_reel.empty()) {
            ids.manual_reel = key;
        }
        if (model == models.powered_reel && ids.powered_reel.empty()) {
            ids.powered_reel = key;
        }
    }
    if (ids.revolution.empty() || !devices[ids.revolution].isMember("cameras")) {
        return;
    }

    auto cameras = devices[ids.revolution]["cameras"];
    for (auto key : cameras.getMemberNames()) {
        if (cameras[key]["model"] == models.camera) {
            ids.camera = key;
            ids.streams = cameras["streams"].getMemberNames();
            break;
        }
    }
};

vector<Camera> CommandAndStateMessageParser::getCameras(string address)
{
    validateCameras(address);
    vector<Camera> cameras;
    auto revolution_json = m_json_data["payload"]["devices"][address];
    if (!revolution_json.isMember("cameras")) {
        return cameras;
    }

    auto cameras_json = revolution_json["cameras"];
    for (auto camera_id : cameras_json.getMemberNames()) {
        Camera cam;
        cam.id = camera_id;
        cam.ip = cameras_json[camera_id]["ip"].asString();
        cam.type = cameras_json[camera_id]["type"].asString();
        cam.osd_enabled = cameras_json[camera_id]["osd"]["enabled"].asBool();
        auto streams = cameras_json[camera_id]["streams"];
        for (auto stream : streams.getMemberNames()) {
            if (streams[stream]["active"].asBool()) {
                cam.active_streams.push_back(stream);
            }
        }
        cameras.push_back(cam);
    }
    return cameras;
}

samples::RigidBodyState CommandAndStateMessageParser::getRevolutionDriveStates(
    string address)
{
    validateDriveStates(address);
    auto msg_setpoint = m_json_data["payload"]["devices"][address]["drive"]["thrust"];
    double setpoint_x = msg_setpoint["forward"].asDouble();
    double setpoint_y = msg_setpoint["lateral"].asDouble();
    double setpoint_z = msg_setpoint["vertical"].asDouble();
    double setpoint_yaw = msg_setpoint["yaw"].asDouble();

    samples::RigidBodyState control;
    control.position.x() = setpoint_x;
    control.position.y() = setpoint_y;
    control.position.z() = setpoint_z;
    control.orientation = Quaterniond(AngleAxisd(setpoint_yaw, Vector3d::UnitZ()));

    return control;
}

DriveMode CommandAndStateMessageParser::getRevolutionDriveModes(string address)
{
    validateDriveModes(address);
    DriveMode drive_mode;
    auto msg_setpoint = m_json_data["payload"]["devices"][address]["drive"]["modes"];
    drive_mode.auto_stabilization = msg_setpoint["autoStabilization"].asBool();
    drive_mode.heading_lock = msg_setpoint["headingLock"].asBool();
    drive_mode.depth_lock = msg_setpoint["depthLock"].asBool();
    drive_mode.altitude_lock = msg_setpoint["altitudeLock"].asBool();
    drive_mode.motors_disabled = msg_setpoint["motorsDisabled"].asBool();
    return drive_mode;
}

samples::RigidBodyState CommandAndStateMessageParser::getRevolutionBodyStates(
    string address)
{
    auto local_frame = m_json_data["payload"]["devices"][address]["control"]["current"]
                                  ["pose"]["localFrame"];
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
    validatePoweredReelMotorState(address);

    JointState state;
    auto root = m_json_data["payload"]["devices"][address];
    state.raw = root["motor1Diagnostics"]["pwm"].asFloat() / 100;
    state.effort = root["motor1Diagnostics"]["current"].asDouble();
    state.speed = root["motor1Diagnostics"]["rpm"].asFloat() * 2 * M_PI / 60;

    samples::Joints powered;
    powered.elements.push_back(state);

    state.raw = root["motor2Diagnostics"]["pwm"].asFloat() / 100;
    state.effort = root["motor2Diagnostics"]["current"].asDouble();
    state.speed = root["motor2Diagnostics"]["rpm"].asFloat() * 2 * M_PI / 60;
    powered.elements.push_back(state);

    return powered;
}

samples::Joints CommandAndStateMessageParser::getRevolutionMotorStates(string address)
{
    validateRevolutionMotorStates(address);
    auto root = m_json_data["payload"]["devices"][address];
    samples::Joints revolution;

    vector<string> motors{"frontRightMotorDiagnostics",
        "frontLeftMotorDiagnostics",
        "rearRightMotorDiagnostics",
        "rearLeftMotorDiagnostics",
        "verticalRightMotorDiagnostics",
        "verticalLeftMotorDiagnostics"};

    for (auto motor : motors) {
        auto motor_json = root[motor];
        JointState state = motorDiagnosticsToJointState(motor_json);
        revolution.elements.push_back(state);
    }
    return revolution;
}

BatteryStatus CommandAndStateMessageParser::getBatteryStates(string address,
    string battery_side)
{
    validateBatteryStates(battery_side, address);
    auto battery_json = m_json_data["payload"]["devices"][address][battery_side];
    BatteryStatus battery;
    battery.charge = battery_json["percent"].asDouble() / 100;
    battery.voltage = battery_json["voltage"].asDouble();

    return battery;
}

Grabber CommandAndStateMessageParser::getGrabberMotorOvercurrentStates(string address)
{
    validateGrabberMotorsStates(address);
    Grabber grabber;

    return grabber;
}

Grabber CommandAndStateMessageParser::getGrabberMotorStates(string address)
{
    validateGrabberMotorsStates(address);
    Grabber grabber;
    auto root = m_json_data["payload"]["devices"][address]["openCloseMotorDiagnostics"]
                           ["grabber"];
    JointState open_close_joint_state = motorDiagnosticsToJointState(root);
    samples::Joints motors;
    grabber.motor_states.elements.push_back(open_close_joint_state);

    root =
        m_json_data["payload"]["devices"][address]["grabber"]["rotateMotorDiagnostics"];
    JointState rotate_joint = motorDiagnosticsToJointState(root);
    grabber.motor_states.elements.push_back(rotate_joint);

    grabber.open_close_motor_overcurrent =
        root["openCloseMotorDiagnostics"]["overcurrent"].asBool();
    grabber.rotate_overcurrent = root["rotateMotorDiagnostics"]["overcurrent"].asBool();
    return grabber;
}

TiltCameraHead CommandAndStateMessageParser::getCameraHeadStates(string address)
{
    validateCameraHeadStates(address);
    TiltCameraHead camera_head;
    auto root = m_json_data["payload"]["devices"][address]["cameraHead"];
    camera_head.light = root["light"]["intensity"].asDouble() / 100;
    camera_head.laser = root["lasers"]["enabled"].asBool();
    camera_head.motor_overcurrent = root["tiltMotorDiagnostics"]["overcurrent"].asBool();
    camera_head.leak = root["leak"].asBool();
    camera_head.tilt.orientation = Quaterniond(
        AngleAxisd(root["tilt"]["position"].asDouble() * M_PI / 180, Vector3d::UnitZ()));

    JointState joint_state = motorDiagnosticsToJointState(root["tiltMotorDiagnostics"]);
    camera_head.motor_states.elements.push_back(joint_state);
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
    validateMotorOverCurrentStates(motor_side, address);
    return m_json_data["payload"]["devices"][address][motor_side]["overcurrent"].asBool();
}

double CommandAndStateMessageParser::getAuxLightIntensity(string address)
{
    validateAuxLightIntensity(address);
    return m_json_data["payload"]["devices"][address]["auxLights"]["intensity"]
               .asDouble() /
           100;
}

double CommandAndStateMessageParser::getTetherLength(string address)
{
    validateDistance(address);
    return m_json_data["payload"]["devices"][address]["distance"].asDouble();
}

double CommandAndStateMessageParser::getCpuTemperature(string address)
{
    validateCPUTemperature(address);
    return m_json_data["payload"]["devices"][address]["cpuTemp"].asDouble();
}

bool CommandAndStateMessageParser::isLeaking(string address)
{
    validateLeaking(address);
    return m_json_data["payload"]["devices"][address]["leak"].asBool();
}

bool CommandAndStateMessageParser::isACPowerConnected(string address)
{
    validateACConnected(address);
    return m_json_data["payload"]["devices"][address]["acConnected"].asBool();
}

bool CommandAndStateMessageParser::isEStopEnabled(string address)
{
    validateEStop(address);
    return m_json_data["payload"]["devices"][address]["eStop"].asBool();
}

#include "CommandAndStateMessageParser.hpp"
#include <iostream>

using namespace std;
using namespace base;
using namespace base::samples;
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
    string const& fieldName,
    string const& context)
{
    if (!value.isMember(fieldName)) {
        throw invalid_argument(context +
                               " validation failed: message does not contain the " +
                               fieldName + " field");
    }
}

void CommandAndStateMessageParser::validateMotorOverCurrentStates(string motor_field_name,
    string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id][motor_field_name],
        "overcurrent",
        motor_field_name);
}

void CommandAndStateMessageParser::validateBatteryStates(string battery_field_name,
    string device_id)
{
    auto battery = m_json_data["payload"]["devices"][device_id][battery_field_name];
    validateFieldPresent(battery, "percent", battery_field_name);
    validateFieldPresent(battery, "voltage", battery_field_name);
}

void CommandAndStateMessageParser::validateAuxLightIntensity(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id]["auxLight"],
        "intensity",
        "auxLight");
}

void CommandAndStateMessageParser::validateDepthAttitude(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "depth",
        device_id);
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "roll", device_id);
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "pitch",
        device_id);
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "heading",
        device_id);
}

void CommandAndStateMessageParser::validateMotorStates(string device_id,
    string motor_field_name)
{
    auto root = m_json_data["payload"]["devices"][device_id][motor_field_name];
    validateFieldPresent(root, "pwm", motor_field_name);
    validateFieldPresent(root, "current", motor_field_name);
    validateFieldPresent(root, "rpm", motor_field_name);
}

void CommandAndStateMessageParser::validatePoweredReelMotorState(string device_id)
{
    auto root = m_json_data["payload"]["devices"][device_id]["motor1Diagnostics"];
    validateFieldPresent(root, "pwm", "motor1Diagnostics");
    validateFieldPresent(root, "current", "motor1Diagnostics");
    root = m_json_data["payload"]["devices"][device_id]["motor2Diagnostics"];
    validateFieldPresent(root, "pwm", "motor2Diagnostics");
    validateFieldPresent(root, "current", "motor2Diagnostics");
}

void CommandAndStateMessageParser::validateGrabberMotorsStates(string device_id)
{
    auto grabber = m_json_data["payload"]["devices"][device_id]["grabber"];
    validateFieldPresent(grabber["openCloseMotorDiagnostics"],
        "overcurrent",
        "grabber openCloseMotorDiagnostics");
    validateMotorStates(device_id, "openCloseMotorDiagnostics");
    validateMotorStates(device_id, "rotateMotorDiagnostics");
    validateFieldPresent(grabber["rotateMotorDiagnostics"],
        "overcurrent",
        "grabber rotateMotorDiagnostics");
}

void CommandAndStateMessageParser::validateCameraHeadStates(string device_id)
{
    auto camera_head = m_json_data["payload"]["devices"][device_id]["cameraHead"];
    validateFieldPresent(camera_head["light"], "intensity", "cameraHead light");
    validateFieldPresent(camera_head["lasers"], "enabled", "cameraHead lasers");
    validateFieldPresent(camera_head["tilt"], "position", "cameraHead tilt");
    validateFieldPresent(camera_head, "leak", "cameraHead");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"],
        "overcurrent",
        "cameraHead tiltMotorDiagnostics");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"],
        "pwm",
        "cameraHead tiltMotorDiagnostics");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"],
        "rpm",
        "cameraHead tiltMotorDiagnostics");
    validateFieldPresent(camera_head["tiltMotorDiagnostics"],
        "current",
        "cameraHead tiltMotorDiagnostics");
}

void CommandAndStateMessageParser::validateCameras(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "cameras",
        device_id);
}

void CommandAndStateMessageParser::validateCameraFields(string device_id,
    string camera_id)
{
    auto json = m_json_data["payload"]["devices"][device_id]["cameras"][camera_id];
    validateFieldPresent(json, "ip", camera_id);
    validateFieldPresent(json, "model", camera_id);
    validateFieldPresent(json, "type", camera_id);
    validateFieldPresent(json, "osd", camera_id);
    validateFieldPresent(json["osd"], "enabled", camera_id);
    validateFieldPresent(json, "streams", camera_id);
}

void CommandAndStateMessageParser::validateStreamFields(string device_id,
    string camera_id,
    string stream_id)
{
    auto json = m_json_data["payload"]["devices"][device_id]["cameras"][camera_id]
                           ["streams"][stream_id];
    validateFieldPresent(json, "active", stream_id);
}

void CommandAndStateMessageParser::validateCPUTemperature(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "cpuTemp",
        device_id);
}

void CommandAndStateMessageParser::validateDriveStates(string device_id)
{
    auto thrust = m_json_data["payload"]["devices"][device_id]["drive"]["thrust"];
    validateFieldPresent(thrust, "forward", "drive thrust");
    validateFieldPresent(thrust, "lateral", "drive thrust");
    validateFieldPresent(thrust, "vertical", "drive thrust");
    validateFieldPresent(thrust, "yaw", "drive thrust");
}

void CommandAndStateMessageParser::validateDriveModes(string device_id)
{
    auto modes = m_json_data["payload"]["devices"][device_id]["drive"]["modes"];
    validateFieldPresent(modes, "autoStabilization", "drive modes");
    validateFieldPresent(modes, "motorsDisabled", "drive modes");
    validateFieldPresent(modes, "altitudeLock", "drive modes");
    validateFieldPresent(modes, "depthLock", "drive modes");
    validateFieldPresent(modes, "headingLock", "drive modes");
}

void CommandAndStateMessageParser::validateLeaking(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id], "leak", device_id);
}

void CommandAndStateMessageParser::validateACConnected(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "acConnected",
        device_id);
}

void CommandAndStateMessageParser::validateEStop(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "eStop",
        device_id);
}

void CommandAndStateMessageParser::validateDistance(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id],
        "distance",
        device_id);
}

void CommandAndStateMessageParser::validateTimeUsage(string device_id)
{
    validateFieldPresent(m_json_data["payload"]["devices"][device_id]["usageTime"],
        "currentSeconds",
        "usageTime");
}

void CommandAndStateMessageParser::validateRevolutionMotorStates(string device_id,
    string motor)
{
    auto device = m_json_data["payload"]["devices"][device_id];
    validateFieldPresent(device[motor], "pwm", motor);
    validateFieldPresent(device[motor], "current", motor);
    validateFieldPresent(device[motor], "rpm", motor);
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
    modes["depthLock"] = command.depth_lock;
    modes["headingLock"] = command.heading_lock;
    root["drive"]["modes"] = modes;

    message["payload"]["devices"][address] = root;
    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseMotorsDisabledRevolutionCommandMessage(
    string api_version,
    string address,
    int model,
    bool motors_disabled)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto root = message["payload"]["devices"][address];
    root["drive"]["modes"]["motorsDisabled"] = motors_disabled;

    message["payload"]["devices"][address] = root;
    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseAutoStabilizationRevolutionCommandMessage(
    string api_version,
    string address,
    int model,
    bool auto_stabilization)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto root = message["payload"]["devices"][address];

    root["drive"]["modes"]["autoStabilization"] = auto_stabilization;

    message["payload"]["devices"][address] = root;
    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseDriveRevolutionCommandMessage(
    string api_version,
    string address,
    int model,
    commands::LinearAngular6DCommand const& command)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto root = message["payload"]["devices"][address];

    auto vertical_cmd = command.z();
    auto thrust = root["drive"]["thrust"];
    thrust["forward"] = round(min(max(command.linear.x(), -1.0), 1.0) * 100);
    thrust["lateral"] = -round(min(max(command.linear.y(), -1.0), 1.0) * 100);
    thrust["vertical"] = -round(min(max(vertical_cmd, -1.0), 1.0) * 100);
    thrust["yaw"] = -round(min(max(command.angular.z(), -1.0), 1.0) * 100);
    root["drive"]["thrust"] = thrust;

    message["payload"]["devices"][address] = root;
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
        round(min(max(static_cast<double>(command.elements[0].speed), -1.0), 1.0) * 100);
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

string CommandAndStateMessageParser::parseCameraHeadLightMessage(string api_version,
    string address,
    int model,
    int camera_head_model,
    double light_intensity)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto camera_head = message["payload"]["devices"][address]["cameraHead"];
    camera_head["model"] = camera_head_model;
    camera_head["light"]["intensity"] = round(min(max(light_intensity, 0.0), 1.0) * 100);
    message["payload"]["devices"][address]["cameraHead"] = camera_head;

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseCameraHeadLaserMessage(string api_version,
    string address,
    int model,
    int camera_head_model,
    bool enabled)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    auto camera_head = message["payload"]["devices"][address]["cameraHead"];
    camera_head["model"] = camera_head_model;
    camera_head["laser"]["enabled"] = enabled;
    message["payload"]["devices"][address]["cameraHead"] = camera_head;

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseAuxLightCommandMessage(string api_version,
    string address,
    int model,
    double intensity)
{
    auto message = payloadSetMessageTemplate(api_version, address, model);
    message["payload"]["devices"][address]["auxLight"]["intensity"] =
        round(min(max(intensity, 0.0), 1.0) * 100);

    Json::FastWriter fast;
    return fast.write(message);
}

string CommandAndStateMessageParser::parseTiltCameraHeadCommandMessage(string api_version,
    string address,
    int rev_model,
    int camera_head_model,
    samples::Joints tilt)
{
    auto message = payloadSetMessageTemplate(api_version, address, rev_model);
    auto camera_head = message["payload"]["devices"][address]["cameraHead"];
    camera_head["model"] = camera_head_model;
    camera_head["tilt"]["speed"] =
        round(min(max(static_cast<double>(tilt.elements[0].speed), -1.0), 1.0) * 100);
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

vector<Camera> CommandAndStateMessageParser::getCameras(string address)
{
    validateCameras(address);
    vector<Camera> cameras;
    auto cameras_json = m_json_data["payload"]["devices"][address]["cameras"];
    for (auto camera_id : cameras_json.getMemberNames()) {
        validateCameraFields(address, camera_id);
        Camera cam;
        cam.time = Time::now();
        cam.id = camera_id;
        cam.ip = cameras_json[camera_id]["ip"].asString();
        cam.type = cameras_json[camera_id]["type"].asString();
        cam.osd_enabled = cameras_json[camera_id]["osd"]["enabled"].asBool();
        auto streams = cameras_json[camera_id]["streams"];
        for (auto stream : streams.getMemberNames()) {
            validateStreamFields(address, camera_id, stream);
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
    double setpoint_y = -msg_setpoint["lateral"].asDouble();
    double setpoint_z = -msg_setpoint["vertical"].asDouble();
    double setpoint_yaw = -msg_setpoint["yaw"].asDouble();

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
    auto root = m_json_data["payload"]["devices"][address]["drive"]["modes"];
    drive_mode.time = Time::now();
    drive_mode.heading_lock = root["headingLock"].asBool();
    drive_mode.depth_lock = root["depthLock"].asBool();
    drive_mode.altitude_lock = root["altitudeLock"].asBool();
    return drive_mode;
}

bool CommandAndStateMessageParser::getRevolutionMotorsDisabled(string address)
{
    validateDriveModes(address);
    return m_json_data["payload"]["devices"][address]["drive"]["modes"]["motorsDisabled"]
        .asBool();
}

bool CommandAndStateMessageParser::getRevolutionAutoStabilization(string address)
{
    validateDriveModes(address);
    return m_json_data["payload"]["devices"][address]["drive"]["modes"]
                      ["autoStabilization"]
                          .asBool();
}

samples::RigidBodyState CommandAndStateMessageParser::getRevolutionPoseZAttitude(
    string address)
{
    validateDepthAttitude(address);
    auto local_frame = m_json_data["payload"]["devices"][address];
    double state_z = local_frame["depth"].asDouble();
    double roll = local_frame["roll"].asDouble() * M_PI / 180;
    double pitch = local_frame["pitch"].asDouble() * M_PI / 180;
    double yaw = -local_frame["heading"].asDouble() * M_PI / 180;

    samples::RigidBodyState pose;
    pose.time = Time::now();
    pose.position.z() = -state_z;
    pose.orientation = AngleAxisd(yaw, Vector3d::UnitZ()) *
                       AngleAxisd(pitch, Vector3d::UnitY()) *
                       AngleAxisd(roll, Vector3d::UnitX());

    return pose;
}

samples::Joints CommandAndStateMessageParser::getPoweredReelMotorState(string address)
{
    validatePoweredReelMotorState(address);

    JointState state;
    auto root = m_json_data["payload"]["devices"][address];
    state.raw = root["motor1Diagnostics"]["pwm"].asFloat() / 100;
    state.effort = root["motor1Diagnostics"]["current"].asDouble();

    samples::Joints powered;
    powered.time = Time::now();
    powered.elements.push_back(state);

    state.raw = root["motor2Diagnostics"]["pwm"].asFloat() / 100;
    state.effort = root["motor2Diagnostics"]["current"].asDouble();
    powered.elements.push_back(state);

    return powered;
}

samples::Joints CommandAndStateMessageParser::getRevolutionMotorStates(string address)
{
    auto root = m_json_data["payload"]["devices"][address];
    samples::Joints revolution;
    revolution.time = Time::now();

    vector<string> motors{"frontRightMotorDiagnostics",
        "frontLeftMotorDiagnostics",
        "rearRightMotorDiagnostics",
        "rearLeftMotorDiagnostics",
        "verticalRightMotorDiagnostics",
        "verticalLeftMotorDiagnostics"};

    for (auto motor : motors) {
        validateRevolutionMotorStates(address, motor);
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
    battery.time = Time::now();
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
    auto root = m_json_data["payload"]["devices"][address]["grabber"]
                           ["openCloseMotorDiagnostics"];

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
    camera_head.time = Time::now();

    return camera_head;
}

samples::Joints CommandAndStateMessageParser::getCameraHeadTiltMotorState(string address)
{
    validateCameraHeadStates(address);
    auto root = m_json_data["payload"]["devices"][address]["cameraHead"];

    samples::Joints motor_states;
    motor_states.time = Time::now();
    JointState joint_state = motorDiagnosticsToJointState(root["tiltMotorDiagnostics"]);
    joint_state.position = root["tilt"]["position"].asDouble() * M_PI / 180;
    motor_states.elements.push_back(joint_state);
    return motor_states;
}

RigidBodyState CommandAndStateMessageParser::getCameraHeadTiltMotorStateRBS(
    string address)
{
    validateCameraHeadStates(address);
    auto root = m_json_data["payload"]["devices"][address]["cameraHead"];

    RigidBodyState rbs;
    rbs.time = Time::now();
    auto yaw = root["tilt"]["position"].asDouble() * M_PI / 180;
    rbs.position = Vector3d::Zero();
    rbs.orientation = AngleAxisd(yaw, Vector3d::UnitZ());
    rbs.sourceFrame = "deep_trekker::body2front_camera_post";
    rbs.targetFrame = "deep_trekker::body2front_camera_pre";
    return rbs;
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
    return m_json_data["payload"]["devices"][address]["auxLight"]["intensity"]
               .asDouble() /
           100;
}

double CommandAndStateMessageParser::getTetherLength(string address)
{
    validateDistance(address);
    // Convert the distance to meters
    return m_json_data["payload"]["devices"][address]["distance"].asDouble() / 100;
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

Json::Value CommandAndStateMessageParser::createGetRequest(string api_version)
{
    Json::Value message;
    message["method"] = "GET";
    message["apiVersion"] = api_version;
    return message;
}

string CommandAndStateMessageParser::getRequestForPoweredReelStates(string api_version,
    string device_id)
{
    auto request = createGetRequest(api_version);
    auto device = request["payload"]["devices"][device_id];
    device["distance"] = 0;
    device["leak"] = false;
    device["cpuTemp"] = 0;
    device["battery1"]["percent"] = 0;
    device["battery1"]["voltage"] = 0;
    device["battery2"]["percent"] = 0;
    device["battery2"]["voltage"] = 0;
    device["acConnected"] = false;
    device["eStop"] = false;
    device["motor1Diagnostics"]["overcurrent"] = false;
    device["motor2Diagnostics"]["overcurrent"] = false;
    request["payload"]["devices"][device_id] = device;

    Json::FastWriter writer;
    return writer.write(request);
}

string CommandAndStateMessageParser::getRequestForRevolutionPoseZAttitude(
    string api_version,
    string device_id)
{
    auto request = createGetRequest(api_version);
    auto device = request["payload"]["devices"][device_id];
    device["depth"] = 0;
    device["roll"] = 0;
    device["pitch"] = 0;
    device["heading"] = 0;
    request["payload"]["devices"][device_id] = device;

    Json::FastWriter writer;
    return writer.write(request);
}
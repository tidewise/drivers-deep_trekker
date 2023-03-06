#include "base/commands/LinearAngular6DCommand.hpp"
#include "deep_trekker/DeepTrekkerCommands.hpp"
#include "deep_trekker/DeepTrekkerStates.hpp"
#include "power_base/BatteryStatus.hpp"
#include "json/json.h"
#include <base/Time.hpp>
#include <deep_trekker/CommandAndStateMessageParser.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace base;
using namespace deep_trekker;

struct MessageParserTest : public ::testing::Test {

    string api_version = "12.0.2";
    string address = "1.2.3.4.5.6";
    CommandAndStateMessageParser getMessageParser()
    {
        return CommandAndStateMessageParser();
    }
};

TEST_F(MessageParserTest, it_parse_powered_reel_command_message)
{
    auto parser = getMessageParser();
    base::samples::Joints command;
    vector<float> speed_vector;
    speed_vector.push_back(0.21);
    command = samples::Joints::Speeds(speed_vector);
    string message =
        parser.parsePoweredReelCommandMessage(api_version, address, 12, command);

    Json::Value json_value;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    string error;
    reader->parse(message.c_str(),
        message.c_str() + strlen(message.c_str()),
        &json_value,
        &error);
    ASSERT_EQ(json_value["apiVersion"], api_version);
    ASSERT_EQ(json_value["method"], "SET");
    ASSERT_EQ(json_value["payload"]["devices"][address]["speed"].asFloat(),
        command.elements[0].speed * 100);
}

TEST_F(MessageParserTest, it_parse_grabber_command_message)
{
    auto parser = getMessageParser();
    base::samples::Joints command;
    JointState state;
    state.raw = 0.28;
    command.elements.push_back(state);
    state.raw = 0.4;
    command.elements.push_back(state);
    string message = parser.parseGrabberCommandMessage(api_version, address, command);

    Json::Value json_value;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    string error;
    reader->parse(message.c_str(),
        message.c_str() + strlen(message.c_str()),
        &json_value,
        &error);
    ASSERT_EQ(json_value["apiVersion"], api_version);
    ASSERT_EQ(json_value["method"], "SET");
    ASSERT_EQ(json_value["payload"]["devices"][address]["grabber"]["openClose"].asFloat(),
        command.elements[0].raw * 100);
    ASSERT_EQ(json_value["payload"]["devices"][address]["grabber"]["rotate"].asFloat(),
        command.elements[1].raw * 100);
}

TEST_F(MessageParserTest, it_parse_camera_head_command_message)
{
    auto parser = getMessageParser();
    CameraHeadCommand command;
    command.laser = true;
    command.light = 0.9;
    string message =
        parser.parseCameraHeadCommandMessage(api_version, address, 102, command);

    Json::Value json_value;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    string error;
    reader->parse(message.c_str(),
        message.c_str() + strlen(message.c_str()),
        &json_value,
        &error);
    ASSERT_EQ(json_value["apiVersion"], api_version);
    ASSERT_EQ(json_value["method"], "SET");
    auto camera_head = json_value["payload"]["devices"][address]["cameraHead"];
    ASSERT_EQ(camera_head["light"]["intensity"].asDouble(), command.light * 100);
    ASSERT_EQ(camera_head["lasers"]["enabled"].asBool(), command.laser);
}

TEST_F(MessageParserTest, it_parse_tilt_camera_head_command_message)
{
    auto parser = getMessageParser();
    CameraHeadCommand command;
    command.laser = true;
    command.light = 0.9;
    base::samples::Joints tilt;
    JointState joint_state;
    joint_state.position = M_PI;
    joint_state.speed = -0.4;
    tilt.elements.push_back(joint_state);
    string message =
        parser.parseTiltCameraHeadCommandMessage(api_version, address, 102, tilt);

    Json::Value json_value;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    string error;
    reader->parse(message.c_str(),
        message.c_str() + strlen(message.c_str()),
        &json_value,
        &error);
    ASSERT_EQ(json_value["apiVersion"], api_version);
    ASSERT_EQ(json_value["method"], "SET");
    auto camera_head = json_value["payload"]["devices"][address]["cameraHead"];
    ASSERT_EQ(camera_head["tilt"]["speed"].asFloat(), tilt.elements[0].speed * 100);
}

TEST_F(MessageParserTest, it_parses_drive_revolution_command)
{
    auto parser = getMessageParser();
    base::commands::LinearAngular6DCommand command;
    command.linear = Eigen::Vector3d(4.2, 42, 6.1);
    command.angular = Eigen::Vector3d(0, 0, 1.2);
    string message =
        parser.parseDriveRevolutionCommandMessage(api_version, address, 102, command);

    Json::Value json_value;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    string error;
    reader->parse(message.c_str(),
        message.c_str() + strlen(message.c_str()),
        &json_value,
        &error);
    ASSERT_EQ(json_value["apiVersion"], api_version);
    ASSERT_EQ(json_value["method"], "SET");
    ASSERT_EQ(json_value["payload"]["devices"][address]["model"], 102);

    auto drive = json_value["payload"]["devices"][address]["drive"];
    ASSERT_EQ(drive["thrust"]["forward"].asDouble(), 4.2);
    ASSERT_EQ(drive["thrust"]["lateral"].asDouble(), 42);
    ASSERT_EQ(drive["thrust"]["vertical"].asDouble(), 6.1);
    ASSERT_EQ(drive["thrust"]["yaw"].asDouble(), 1.2);
}

TEST_F(MessageParserTest, it_parses_drive_mode_revolution_command)
{
    auto parser = getMessageParser();
    DriveMode command;
    command.altitude_lock = true;
    command.heading_lock = false;
    command.depth_lock = false;
    command.motors_disabled = true;
    command.auto_stabilization = true;
    string message =
        parser.parseDriveModeRevolutionCommandMessage(api_version, address, 102, command);

    Json::Value json_value;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    string error;
    reader->parse(message.c_str(),
        message.c_str() + strlen(message.c_str()),
        &json_value,
        &error);
    ASSERT_EQ(json_value["apiVersion"], api_version);
    ASSERT_EQ(json_value["method"], "SET");
    ASSERT_EQ(json_value["payload"]["devices"][address]["model"], 102);

    auto drive = json_value["payload"]["devices"][address]["drive"];
    ASSERT_EQ(drive["modes"]["altitudeLock"].asBool(), true);
    ASSERT_EQ(drive["modes"]["headingLock"].asBool(), false);
    ASSERT_EQ(drive["modes"]["depthLock"].asBool(), false);
    ASSERT_EQ(drive["modes"]["motorsDisabled"].asBool(), true);
    ASSERT_EQ(drive["modes"]["autoStabilization"].asBool(), true);
}

TEST_F(MessageParserTest, it_discovers_a_camera_and_active_streams)
{
    auto parser = getMessageParser();
    Json::Value camera_with_stream_json;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["model"] = 13;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                           ["camera_id123"]["model"] = 5;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                           ["camera_id123"]["ip"] = "1.1.1.1";
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                           ["camera_id123"]["type"] = "some-type";
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                           ["camera_id123"]["osd"]["enabled"] = true;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                           ["camera_id123"]["streams"]["blablabla"]["active"] = false;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                           ["camera_id123"]["streams"]["something"]["active"] = true;
    Json::FastWriter writer;
    string errors;
    parser.parseJSONMessage(writer.write(camera_with_stream_json).c_str(), errors);
    auto actual = parser.getCameras("revolution_id123");

    Camera expected_cam;
    expected_cam.ip = "1.1.1.1";
    expected_cam.id = "camera_id123";
    expected_cam.type = "some-type";
    expected_cam.osd_enabled = true;
    expected_cam.active_streams.push_back("something");
    vector<Camera> expected;
    expected.push_back(expected_cam);

    ASSERT_EQ(expected[0].id, actual[0].id);
    ASSERT_EQ(expected[0].ip, actual[0].ip);
    ASSERT_EQ(expected[0].type, actual[0].type);
    ASSERT_EQ(expected[0].osd_enabled, actual[0].osd_enabled);
    ASSERT_EQ(expected[0].active_streams, actual[0].active_streams);
}

TEST_F(MessageParserTest, it_throws_an_invalid_argument_error_when_the_field_isnt_present)
{
    auto parser = getMessageParser();
    Json::Value camera_with_stream_json;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["model"] = 13;
    Json::FastWriter writer;
    string errors;
    parser.parseJSONMessage(writer.write(camera_with_stream_json).c_str(), errors);
    ASSERT_ANY_THROW(parser.getCameras("revolution_id123"));
}

TEST_F(MessageParserTest, it_returns_the_rov_pose_with_z_and_attitude)
{
    auto parser = getMessageParser();
    Json::Value drive_modes;
    drive_modes["payload"]["devices"]["revolution_id123"]["model"] = 13;
    drive_modes["payload"]["devices"]["revolution_id123"]["depth"] = 20;
    drive_modes["payload"]["devices"]["revolution_id123"]["roll"] = 1.5;
    drive_modes["payload"]["devices"]["revolution_id123"]["pitch"] = 3.14;
    drive_modes["payload"]["devices"]["revolution_id123"]["heading"] = 2;
    Json::FastWriter writer;
    string errors;
    parser.parseJSONMessage(writer.write(drive_modes).c_str(), errors);
    auto actual = parser.getRevolutionPoseZAttitude("revolution_id123");
    auto euler = actual.orientation.toRotationMatrix().eulerAngles(0, 1, 2);

    ASSERT_NEAR(20, actual.position.z(), 0.01);
    ASSERT_NEAR(1.5, euler[0], 0.01);
    ASSERT_NEAR(3.14, euler[1], 0.01);
    ASSERT_NEAR(2, euler[2], 0.01);
}

TEST_F(MessageParserTest, it_returns_the_drive_mode)
{
    auto parser = getMessageParser();
    Json::Value drive_modes;
    drive_modes["payload"]["devices"]["revolution_id123"]["model"] = 13;
    drive_modes["payload"]["devices"]["revolution_id123"]["drive"]["modes"]
               ["headingLock"] = true;
    drive_modes["payload"]["devices"]["revolution_id123"]["drive"]["modes"]["depthLock"] =
        true;
    drive_modes["payload"]["devices"]["revolution_id123"]["drive"]["modes"]
               ["motorsDisabled"] = false;
    drive_modes["payload"]["devices"]["revolution_id123"]["drive"]["modes"]
               ["autoStabilization"] = false;
    drive_modes["payload"]["devices"]["revolution_id123"]["drive"]["modes"]
               ["altitudeLock"] = true;
    Json::FastWriter writer;
    string errors;
    parser.parseJSONMessage(writer.write(drive_modes).c_str(), errors);
    auto actual = parser.getRevolutionDriveModes("revolution_id123");
    DriveMode expected;
    expected.auto_stabilization = false;
    expected.altitude_lock = true;
    expected.heading_lock = true;
    expected.depth_lock = true;
    expected.motors_disabled = false;
    ASSERT_EQ(expected.auto_stabilization, actual.auto_stabilization);
    ASSERT_EQ(expected.altitude_lock, actual.altitude_lock);
    ASSERT_EQ(expected.heading_lock, actual.heading_lock);
    ASSERT_EQ(expected.depth_lock, actual.depth_lock);
    ASSERT_EQ(expected.motors_disabled, actual.motors_disabled);
}

TEST_F(MessageParserTest, it_returns_the_camera_head_state)
{
    auto parser = getMessageParser();
    Json::Value camera_head;
    camera_head["payload"]["devices"]["revolution_id123"]["model"] = 13;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]["light"]
               ["intensity"] = 40;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]["lasers"]
               ["enabled"] = true;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]["leak"] = false;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]["tilt"]
               ["position"] = 90;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]
               ["tiltMotorDiagnostics"]["overcurrent"] = true;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]
               ["tiltMotorDiagnostics"]["rpm"] = 20;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]
               ["tiltMotorDiagnostics"]["pwm"] = 80;
    camera_head["payload"]["devices"]["revolution_id123"]["cameraHead"]
               ["tiltMotorDiagnostics"]["current"] = 60;

    Json::FastWriter writer;
    string errors;
    parser.parseJSONMessage(writer.write(camera_head).c_str(), errors);
    auto actual = parser.getCameraHeadStates("revolution_id123");
    TiltCameraHead expected;
    expected.light = 0.4;
    expected.laser = true;
    expected.motor_overcurrent = true;
    expected.leak = false;
    expected.tilt.orientation = Quaterniond(AngleAxisd(M_PI / 2, Vector3d::UnitZ()));
    JointState expected_joint;
    expected_joint.raw = 0.8;
    expected_joint.speed = 2.093;
    expected_joint.effort = 60;
    ASSERT_NEAR(expected.light, actual.light, 0.01);
    ASSERT_EQ(expected.laser, actual.laser);
    ASSERT_EQ(expected.motor_overcurrent, actual.motor_overcurrent);
    ASSERT_EQ(expected.leak, actual.leak);
    ASSERT_NEAR(getYaw(expected.tilt.orientation), getYaw(actual.tilt.orientation), 0.01);
    ASSERT_NEAR(expected_joint.raw, actual.motor_states.elements[0].raw, 0.01);
    ASSERT_NEAR(expected_joint.speed, actual.motor_states.elements[0].speed, 0.01);
    ASSERT_NEAR(expected_joint.effort, actual.motor_states.elements[0].effort, 0.01);
}

TEST_F(MessageParserTest, it_returns_the_powered_reel_motor_states)
{
    auto parser = getMessageParser();
    Json::Value powered_reel;
    powered_reel["payload"]["devices"]["reel_id123"]["model"] = 108;
    powered_reel["payload"]["devices"]["reel_id123"]["motor1Diagnostics"]["current"] = 40;
    powered_reel["payload"]["devices"]["reel_id123"]["motor1Diagnostics"]["pwm"] = 40;
    powered_reel["payload"]["devices"]["reel_id123"]["motor2Diagnostics"]["current"] = 42;
    powered_reel["payload"]["devices"]["reel_id123"]["motor2Diagnostics"]["pwm"] = 42;

    Json::FastWriter writer;
    string errors;
    parser.parseJSONMessage(writer.write(powered_reel).c_str(), errors);
    auto actual = parser.getPoweredReelMotorState("reel_id123");

    JointState motor1_expected;
    motor1_expected.raw = 0.4;
    motor1_expected.effort = 40;

    JointState motor2_expected;
    motor2_expected.raw = 0.42;
    motor2_expected.effort = 42;

    ASSERT_NEAR(motor1_expected.raw, actual.elements[0].raw, 0.01);
    ASSERT_NEAR(motor1_expected.effort, actual.elements[0].effort, 0.01);
    ASSERT_NEAR(motor2_expected.raw, actual.elements[1].raw, 0.01);
    ASSERT_NEAR(motor2_expected.effort, actual.elements[1].effort, 0.01);
}

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

TEST_F(MessageParserTest, it_parses_position_revolution_command_message)
{
    auto parser = getMessageParser();
    MotionAndLightCommand command;
    command.light = 0.55;
    command.vehicle_setpoint.linear = Eigen::Vector3d(2.1, 1.3, 6.1);
    command.vehicle_setpoint.angular = Eigen::Vector3d(0, 0, 1.2);
    string message =
        parser.parsePositionRevolutionCommandMessage(api_version, address, command);

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
    ASSERT_EQ(json_value["payload"]["devices"][address]["auxLights"].asDouble(),
        command.light * 100);
    auto value = json_value["payload"]["devices"][address]["control"]["setpoint"]["pose"]
                           ["localFrame"];
    ASSERT_EQ(value["x"], command.vehicle_setpoint.linear[0]);
    ASSERT_EQ(value["y"], command.vehicle_setpoint.linear[1]);
    ASSERT_EQ(value["z"], command.vehicle_setpoint.linear[2]);
    ASSERT_EQ(value["yaw"], command.vehicle_setpoint.angular[2]);
}

TEST_F(MessageParserTest, it_parse_velocity_revolution_command_message)
{
    auto parser = getMessageParser();
    MotionAndLightCommand command;
    command.light = 0.55;
    command.vehicle_setpoint.linear = Eigen::Vector3d(2.1, 1.3, 6.1);
    command.vehicle_setpoint.angular = Eigen::Vector3d(0, 0, 1.2);
    string message =
        parser.parseVelocityRevolutionCommandMessage(api_version, address, command);

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
    ASSERT_EQ(json_value["payload"]["devices"][address]["auxLights"].asDouble(),
        command.light * 100);
    auto value =
        json_value["payload"]["devices"][address]["control"]["setpoint"]["velocity"];
    ASSERT_EQ(value["x"], command.vehicle_setpoint.linear[0]);
    ASSERT_EQ(value["y"], command.vehicle_setpoint.linear[1]);
    ASSERT_EQ(value["z"], command.vehicle_setpoint.linear[2]);
    ASSERT_EQ(value["yaw"], command.vehicle_setpoint.angular[2]);
}

TEST_F(MessageParserTest, it_parse_acceleration_revolution_command_message)
{
    auto parser = getMessageParser();
    MotionAndLightCommand command;
    command.light = 0.55;
    command.vehicle_setpoint.linear = Eigen::Vector3d(2.1, 1.3, 6.1);
    command.vehicle_setpoint.angular = Eigen::Vector3d(0, 0, 1.2);
    string message =
        parser.parseAccelerationRevolutionCommandMessage(api_version, address, command);

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
    ASSERT_EQ(json_value["payload"]["devices"][address]["auxLights"].asDouble(),
        command.light * 100);
    auto value =
        json_value["payload"]["devices"][address]["control"]["setpoint"]["acceleration"];
    ASSERT_EQ(value["x"], command.vehicle_setpoint.linear[0]);
    ASSERT_EQ(value["y"], command.vehicle_setpoint.linear[1]);
    ASSERT_EQ(value["z"], command.vehicle_setpoint.linear[2]);
    ASSERT_EQ(value["yaw"], command.vehicle_setpoint.angular[2]);
}

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
    string message = parser.parseTiltCameraHeadCommandMessage(api_version,
        address,
        102,
        command,
        tilt);

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
    ASSERT_EQ(camera_head["lights"].asDouble(), command.light * 100);
    ASSERT_EQ(camera_head["lasers"].asBool(), command.laser);
    ASSERT_EQ(camera_head["tilt"]["speed"].asFloat(), tilt.elements[0].speed * 100);
}

TEST_F(MessageParserTest, it_parses_drive_revolution_command)
{
    auto parser = getMessageParser();
    MotionAndLightCommand command;
    command.vehicle_setpoint.linear = Eigen::Vector3d(4.2, 42, 6.1);
    command.vehicle_setpoint.angular = Eigen::Vector3d(0, 0, 1.2);
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

TEST_F(MessageParserTest, it_discovers_the_devices_ids_when_available)
{
    auto parser = getMessageParser();
    DevicesID ids;
    DevicesModel models;
    models.revolution = 13;
    models.powered_reel = 102;
    models.camera = 5;
    models.manual_reel = 100;
    Json::Value json_without_manual_reel;
    json_without_manual_reel["payload"]["devices"]["revolution_id123"]["model"] = 13;
    json_without_manual_reel["payload"]["devices"]["powered_reel_id_123"]["model"] = 102;
    json_without_manual_reel["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["model"] = 5;
    Json::FastWriter writer;
    string errors;
    parser.parseJSONMessage(writer.write(json_without_manual_reel).c_str(), errors);
    parser.getDevicesID(models, ids);

    DevicesID expected_ids;
    expected_ids.revolution = "revolution_id123";
    expected_ids.powered_reel = "powered_reel_id_123";
    expected_ids.camera = "camera_id123";
    ASSERT_EQ(expected_ids.revolution, ids.revolution);
    ASSERT_EQ(expected_ids.powered_reel, ids.powered_reel);
    ASSERT_EQ(expected_ids.camera, ids.camera);
    ASSERT_EQ(expected_ids.manual_reel, ids.manual_reel);
    ASSERT_EQ(expected_ids.streams, ids.streams);
}

TEST_F(MessageParserTest, it_discovers_a_camera_and_active_streams)
{
    auto parser = getMessageParser();
    Json::Value camera_with_stream_json;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["model"] = 13;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["model"] = 5;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["ip"]= "1.1.1.1";
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["type"]= "some-type";
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["osd"]["enabled"]= true;
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

TEST_F(MessageParserTest, it_gets_the_revolution_drive_states_when_they_are_present)
{
    auto parser = getMessageParser();
    Json::Value camera_with_stream_json;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["model"] = 13;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["model"] = 5;
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["ip"]= "1.1.1.1";
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["type"]= "some-type";
    camera_with_stream_json["payload"]["devices"]["revolution_id123"]["cameras"]
                            ["camera_id123"]["osd"]["enabled"]= true;
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

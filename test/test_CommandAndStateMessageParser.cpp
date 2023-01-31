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

TEST_F(MessageParserTest, it_parse_get_command_message)
{
    auto parser = getMessageParser();
    string message = parser.parseGetMessage(api_version);

    Json::Value json_value;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    string error;
    reader->parse(message.c_str(),
        message.c_str() + strlen(message.c_str()),
        &json_value,
        &error);
    ASSERT_EQ(json_value["apiVersion"], api_version);
    ASSERT_EQ(json_value["method"], "GET");
    Json::Value expected_payload;
    expected_payload["payload"] = {};
    ASSERT_EQ(json_value["payload"], expected_payload["payload"]);
}

TEST_F(MessageParserTest, it_parse_position_revolution_command_message)
{
    auto parser = getMessageParser();
    MotionAndLightCommand command;
    command.light = 0.55;
    command.vehicle_setpoint.linear = Eigen::Vector3d(2.1, 1.3, 6.1);
    command.vehicle_setpoint.angular = Eigen::Vector3d(0, 0, 1.2);
    string message = parser.parsePositionRevolutionCommandMessage(api_version, address, command);

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
    string message = parser.parseVelocityRevolutionCommandMessage(api_version, address, command);

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
    auto value = json_value["payload"]["devices"][address]["control"]["setpoint"]["velocity"];
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
    string message = parser.parseAccelerationRevolutionCommandMessage(api_version, address, command);

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
    auto value = json_value["payload"]["devices"][address]["control"]["setpoint"]["acceleration"];
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
    string message = parser.parsePoweredReelCommandMessage(api_version, address, command);

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
    ASSERT_EQ(json_value["payload"]["devices"][address]["reelFoward"], true);
    ASSERT_EQ(json_value["payload"]["devices"][address]["reelReverse"], false);
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
    command.camera.brightness = 0.3;
    command.camera.exposure = 0.2;
    command.camera.focus = 0.8;
    command.camera.saturation = 0.56;
    command.camera.sharpness = 1;
    command.camera.zoom.ratio = 0.5;
    command.camera.zoom.speed = 0.75;
    base::samples::Joints tilt;
    JointState joint_state;
    joint_state.position = M_PI;
    joint_state.speed = -0.4;
    tilt.elements.push_back(joint_state);
    string message =
        parser.parseTiltCameraHeadCommandMessage(api_version, address, command, tilt);

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
    ASSERT_NEAR(camera_head["lights"].asDouble(), command.light * 100, 0.001);
    ASSERT_EQ(camera_head["lasers"].asBool(), command.laser);
    ASSERT_NEAR(camera_head["tilt"]["speed"].asFloat(), tilt.elements[0].speed * 100, 0.001);
    ASSERT_NEAR(camera_head["tilt"]["position"].asFloat() * M_PI / 180, tilt.elements[0].position, 0.001);
    ASSERT_NEAR(camera_head["camera"]["exposure"].asFloat(), command.camera.exposure * 15, 0.001);
    ASSERT_NEAR(camera_head["camera"]["brightness"].asFloat(),
        command.camera.brightness * 100, 0.001);
    ASSERT_NEAR(camera_head["camera"]["focus"].asFloat(), command.camera.focus * 100, 0.001);
    ASSERT_NEAR(camera_head["camera"]["saturation"].asFloat(),
        command.camera.saturation * 100, 0.001);
    ASSERT_NEAR(camera_head["camera"]["sharpness"].asFloat(),
        command.camera.sharpness * 100, 0.001);
    ASSERT_NEAR(camera_head["camera"]["zoom"]["ratio"].asFloat(),
        command.camera.zoom.ratio, 0.001);
    ASSERT_NEAR(camera_head["camera"]["zoom"]["speed"].asFloat(),
        command.camera.zoom.speed * 100, 0.001);
}

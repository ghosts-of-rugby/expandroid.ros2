#pragma once

#include <boost/asio.hpp>
#include <map>
#include <mutex>
#include <thread>

#include "expandroid_msgs/action/trajectory_tracking.hpp"
#include "expandroid_msgs/msg/expandroid_command.hpp"
#include "expandroid_msgs/msg/expandroid_state.hpp"
#include "nlohmann/json.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using boost::asio::ip::udp;
using json = nlohmann::json;

struct ExpandroidInternalCommand {
  double hand_command;
  double x_command;
  double y_command;
  double z_command;
};

struct ExpandroidParameter {
  const double c610_current_value_per_ampere = 1000.0;
  const double c620_current_value_per_ampere = 16384.0 / 20.0;

  const double hand_motor_angle_per_user_angle = 850000.0;
  const double hand_motor_speed_per_user_speed =
      4000.0 / 530000 * hand_motor_angle_per_user_angle;

  const double x_motor_angle_per_user_angle = 700000.0;
  const double x_motor_speed_per_user_speed =
      3900.0 / 536000 * x_motor_angle_per_user_angle;
};

enum class ControlMode { SPEED_CTRL, TRAJECTORY_TRACKING };

class ExpandroidControlNode : public rclcpp::Node {
  using TrajectoryTracking = expandroid_msgs::action::TrajectoryTracking;
  using GoalHandleTrajectoryTracking =
      rclcpp_action::ServerGoalHandle<TrajectoryTracking>;

 public:
  ExpandroidControlNode();

 private:
  void init();
  void default_update();

  // Trajectory Tracking Action
  rclcpp_action::GoalResponse handle_goal(
      const rclcpp_action::GoalUUID& uuid,
      std::shared_ptr<const TrajectoryTracking::Goal> goal);
  rclcpp_action::CancelResponse handle_cancel(
      const std::shared_ptr<GoalHandleTrajectoryTracking> goal_handle);

  void handle_accepted(
      const std::shared_ptr<GoalHandleTrajectoryTracking> goal_handle);

  void execute(const std::shared_ptr<GoalHandleTrajectoryTracking> goal_handle);

  expandroid_msgs::msg::ExpandroidState get_expandroid_state();

  rclcpp::TimerBase::SharedPtr default_updater_;

  // State Publisher
  rclcpp::Publisher<expandroid_msgs::msg::ExpandroidState>::SharedPtr
      expandroid_state_publisher_;

  // Speed Command Subscriber
  rclcpp::Subscription<expandroid_msgs::msg::ExpandroidCommand>::SharedPtr
      expandroid_speed_command_subscriber_;

  // Trajectory Tracking Action Server
  rclcpp_action::Server<TrajectoryTracking>::SharedPtr
      trajectory_tracking_action_server_;

  // Socket misc
  boost::asio::io_context io_context_;
  std::unique_ptr<udp::socket> socket_;
  udp::endpoint motor_controller_endpoint_;
  std::mutex socket_mutex_;

  // Expandroid Parameter
  ExpandroidParameter expandroid_parameter_;

  ExpandroidInternalCommand expandroid_speed_command_;
  ExpandroidInternalCommand expandroid_angle_command_;
  expandroid_msgs::msg::ExpandroidState expandroid_state_;
  ControlMode control_mode_;
};
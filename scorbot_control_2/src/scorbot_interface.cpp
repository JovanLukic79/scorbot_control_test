#include "scorbot_control_2/scorbot_interface.h"
#include <std_msgs/UInt16MultiArray.h>
#include "scorbot_control_2/angleConverter.h"

//Defining NOde handle, of pos,vel,eff,cmd of names: (joint names)
ScorbotHwInterface::ScorbotHwInterface(ros::NodeHandle& nh) : nh_(nh), 
            pnh_("~"),
            pos_(5, 0),
            vel_(5, 0),
            eff_(5, 0),
            cmd_(5, 0),
            names_{"base_motor1", "link1_motor2", "link2_motor3", "rotgrip_motor4", "rotgrip_motor5"}
{
    // Read from the param server
    pnh_.param("joint_names", names_, names_);

    // Init the publisher with the hardware pub to "/arduino/arm_actuate(Arduino subs to this)"
    //serviceCLient (from scorbot_control_2(pkg),find class "angle_converter"), sub to "rad_to_degrees"
    hardware_pub_ = pnh_.advertise<std_msgs::UInt16MultiArray>("/arduino/arm_actuate", 1000);
    hardware_srv_ = pnh_.serviceClient<scorbot_control_2::angleConverter>("/radians_to_degrees");
    
    ROS_INFO("Starting Arduinobot Hardware Interface...");

    // connect and register joint state interface
    //pull from hardware interface JOint stante handle(Handle to read single joint state), from joint state interface(reading array of joint states(pos,vel,eff))
    hardware_interface::JointStateHandle state_handle1(names_.at(0), &pos_.at(0), &vel_.at(0), &eff_.at(0));
    joint_state_interface_.registerHandle(state_handle1);
    hardware_interface::JointStateHandle state_handle2(names_.at(1), &pos_.at(1), &vel_.at(1), &eff_.at(1));
    joint_state_interface_.registerHandle(state_handle2);
    hardware_interface::JointStateHandle state_handle3(names_.at(2), &pos_.at(2), &vel_.at(2), &eff_.at(2));
    joint_state_interface_.registerHandle(state_handle3);
    hardware_interface::JointStateHandle state_handle4(names_.at(3), &pos_.at(3), &vel_.at(3), &eff_.at(3));
    joint_state_interface_.registerHandle(state_handle4);
    hardware_interface::JointStateHandle state_handle5(names_.at(4), &pos_.at(4), &vel_.at(4), &eff_.at(4));
    joint_state_interface_.registerHandle(state_handle5);

    registerInterface(&joint_state_interface_);

    // connect and register joint position interface
    // the motors accept position inputs
    hardware_interface::JointHandle position_handle1(joint_state_interface_.getHandle(names_.at(0)), &cmd_.at(0));
    joint_position_interface_.registerHandle(position_handle1);
    hardware_interface::JointHandle position_handle2(joint_state_interface_.getHandle(names_.at(1)), &cmd_.at(1));
    joint_position_interface_.registerHandle(position_handle2);
    hardware_interface::JointHandle position_handle3(joint_state_interface_.getHandle(names_.at(2)), &cmd_.at(2));
    joint_position_interface_.registerHandle(position_handle3);
    hardware_interface::JointHandle position_handle4(joint_state_interface_.getHandle(names_.at(3)), &cmd_.at(3));
    joint_position_interface_.registerHandle(position_handle4);
    hardware_interface::JointHandle position_handle5(joint_state_interface_.getHandle(names_.at(4)), &cmd_.at(4));
    joint_position_interface_.registerHandle(position_handle5);

    registerInterface(&joint_position_interface_);

    ROS_INFO("Interfaces registered.");


    ROS_INFO("Preparing the Controller Manager");

    controller_manager_.reset(new controller_manager::ControllerManager(this, nh_));
    update_freq_ = ros::Duration(0.1);
    looper_ = nh_.createTimer(update_freq_, &ScorbotHwInterface::update, this);
    
    ROS_INFO("Ready to execute the control loop");
}

void ScorbotHwInterface::update(const ros::TimerEvent& e)
{
    // This function is called periodically in order to update the controller
    // manager about the progress in the execution of the goal of the hardware
    ROS_INFO("Update Event");
    elapsed_time_ = ros::Duration(e.current_real - e.last_real);
    read();
    controller_manager_->update(ros::Time::now(), elapsed_time_);
    write(elapsed_time_);
}

void ScorbotHwInterface::read()
{
    // Reads the current status of the Hardware (Arduino)
    // Open Loop Control - no sensor available on the robot taht detects the effective
    // angle of rotation of each joint. Suppose that the motors are always able to follow
    // the position command
    pos_.at(0) = cmd_.at(0);
    pos_.at(1) = cmd_.at(1);
    pos_.at(2) = cmd_.at(2);
    pos_.at(3) = cmd_.at(3);
    pos_.at(4) = cmd_.at(4);
}

void ScorbotHwInterface::write(ros::Duration elapsed_time)
{    
    // Send the command to the Hardware (Arduino)
    // First converts the angle from the moveit/urdf convention 
    // to the Arduino convention and then publishes the converted angles
    scorbot_control_2::angleConverter srv;
    srv.request.base_motor1 = cmd_.at(0);
    srv.request.link1_motor2 = cmd_.at(1);
    srv.request.link2_motor3 = cmd_.at(2);
    srv.request.rotgrip_motor4 = cmd_.at(3);
    srv.request.rotgrip_motor5 = cmd_.at(4);

    // Call the service and show the response of the service
    if (hardware_srv_.call(srv))
    {
        // compose the array message
        std::vector<unsigned int> angles_deg;
        angles_deg.push_back(srv.response.base_motor1);
        angles_deg.push_back(srv.response.link1_motor2);
        angles_deg.push_back(srv.response.link2_motor3);
        angles_deg.push_back(srv.response.rotgrip_motor4);
        angles_deg.push_back(srv.response.rotgrip_motor5);

        std_msgs::UInt16MultiArray msg;

        // set up dimensions
        msg.layout.dim.push_back(std_msgs::MultiArrayDimension());
        msg.layout.dim[0].size = angles_deg.size();
        msg.layout.dim[0].stride = 1;

        // copy in the data
        msg.data.clear();
        msg.data.insert(msg.data.end(), angles_deg.begin(), angles_deg.end());

        // publish the array message to the defined topic
        hardware_pub_.publish(msg);
    }
    else
    {
        ROS_ERROR("Failed to call service radians_to_degrees");
    }
}


int main(int argc, char** argv)
{
    ros::init(argc, argv, "arduinobot_interface_node");
    ros::NodeHandle nh;
    ros::MultiThreadedSpinner spinner(2);
    ScorbotHwInterface robot(nh);

    // Keep ROS up and running
    spinner.spin();
    return 0;
}
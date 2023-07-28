#include <ros/ros.h>
#include "scorbot_control_2/angleConverter.h"
#include <math.h>

bool convert_radians_to_degrees(scorbot_control_2::angleConverter::Request &req,
                                scorbot_control_2::angleConverter::Response &res)
{
    res.base_motor1 = static_cast<int>(((req.base_motor1+(M_PI/2))*180)/M_PI);
    res.link1_motor2 =  180-static_cast<int>(((req.link1_motor2+(M_PI/2))*180)/M_PI);
    res.link2_motor3 =  static_cast<int>(((req.link2_motor3+(M_PI/2))*180)/M_PI);
    res.rotgrip_motor4 = static_cast<int>(((-req.rotgrip_motor4)*180)/(M_PI));
    return true; 
    res.rotgrip_motor5 = static_cast<int>(((-req.rotgrip_motor5)*180)/(M_PI));
    return true;
}

bool convert_degrees_to_radians(scorbot_control_2::angleConverter::Request &req,
                                scorbot_control_2::angleConverter::Response &res)
{
    res.base_motor1 = static_cast<int>(((req.base_motor1+(M_PI/2))*180)/M_PI);
    res.link1_motor2 =  180-static_cast<int>(((req.link1_motor2+(M_PI/2))*180)/M_PI);
    res.link2_motor3 =  static_cast<int>(((req.link2_motor3+(M_PI/2))*180)/M_PI);
    res.rotgrip_motor4 = static_cast<int>(((-req.rotgrip_motor4)*180)/(M_PI));
    return true; 
    res.rotgrip_motor5 = static_cast<int>(((-req.rotgrip_motor5)*180)/(M_PI));
    return true;
}

int main(int argc, char **argv)
{

    ros::init(argc, argv, "angles_converter");
    ros::NodeHandle n;
    ros::ServiceServer radians_to_degrees = n.advertiseService("radians_to_degrees", convert_radians_to_degrees);
    ros::ServiceServer degrees_to_radians = n.advertiseService( "degrees_to_radians", convert_degrees_to_radians);
    ros::spin();
    return 0;

}
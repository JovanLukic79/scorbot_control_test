#include <ros/ros.h>

void timerCallback(const ros::TimerEvent& event)  // function called every time timer expires, information of timer event::: subscribe to ros library > TImerEvent. input variable is named "event"

 
{
   ROS_INFO("Called timerCallback function");    // print message "Called timerCallback function" in console/terminal


}

int main(int argc, char **argv)


{
    ros::init(argc, argv, "timer_node");
    ros::NodeHandle nh;
    ros::Duration timer_duration(1); //defines the duration of timer (how long it will last(1 sec))
    ros::Timer timer = nh.createTimer(timer_duration, timerCallback); // actual timer node, includes the duration, and callback (which we define as "timerCallback") callback = do it again name of node = "createTimer" 
    ros::spin(); // from ros library, publish "spin". Spin = keep our node running. WIthout it, node only runs once

    return 0;


}
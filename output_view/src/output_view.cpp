// Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
// Full license terms provided in LICENSE.md file.

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <sensor_msgs/Image.h>
#include "output_view/segnet.h"

static const std::string OPENCV_WINDOW = "Image window";

using namespace sensor_msgs;
using namespace message_filters;

class ImageConverter
{
  ros::Publisher image_pub;
  double alpha;
  cv::Mat lookUpTable;
  
public:
  
  ImageConverter(const std::string &colors_file, double a, const std::string &image_output)
  {
    /*
    ROS_INFO("Now publishing to %s topic", image_output.c_str());
    image_pub = nh.advertise<sensor_msgs::Image>(image_output, 1);
    */
    alpha = a;
    
    // TODO - undefined behavior when getting in more colors than it expects
    lookUpTable = cv::Mat(1, 256, CV_8UC3);

    bool result = loadClassColors(colors_file.c_str(), lookUpTable);
    ROS_ASSERT(result == true); // NOTE: expressions inside ROS_ASSERT will not execute
    ROS_INFO("Loaded colors from %s", colors_file.c_str());
  }

  ~ImageConverter()
  {
    cv::destroyWindow(OPENCV_WINDOW);
  }
  
  void segmentationCb(const ImageConstPtr& raw_image, const ImageConstPtr& seg_info)
  {
    cv_bridge::CvImageConstPtr raw_image_ptr;
    cv_bridge::CvImageConstPtr seg_info_ptr;
    try
    {
      raw_image_ptr = cv_bridge::toCvShare(raw_image, sensor_msgs::image_encodings::BGR8);
      seg_info_ptr = cv_bridge::toCvShare(seg_info, sensor_msgs::image_encodings::MONO8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }
    
    cv::Mat colored_image(seg_info_ptr->image.size(), CV_8UC3);
    colorImage(lookUpTable, seg_info_ptr->image, colored_image);

    cv::Mat scaled_image(raw_image_ptr->image.size(), CV_8UC3);
    // TODO - add default scaling behavior that preserves ratio
    cv::resize(colored_image, scaled_image, scaled_image.size());

    cv::Mat final_image;
    cv::addWeighted(scaled_image, alpha, raw_image_ptr->image, 1 - alpha, 0.0, final_image);
    
    // Update GUI Window
    cv::imshow(OPENCV_WINDOW, final_image);
    cv::waitKey(3);

    //    ImagePtr output = cv_bridge::CvImage(std_msgs::Header(), "bgr8", scaled_image).toImageMsg();

    // Output modified video stream
    //    image_pub.publish(output);
  }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");

  ros::NodeHandle nh("~");

  std::string image_input, image_output;
  std::string annotation_input, annotation_type;
  std::string colors_file;
  double alpha;
  
  nh.param<std::string>("image_input", image_input, "/camera/image_raw");
  nh.param<std::string>("annotation_input", annotation_input, "/segnet/network/output");
  nh.param<std::string>("annotation_type", annotation_type, "segmentation");
  nh.param<std::string>("image_output", image_output, "/dnn_data/out");
  nh.param<std::string>("colors_file", colors_file, "/path/to/colors_file.lut");
  nh.param("alpha", alpha, 0.8);

  ImageConverter img(colors_file, alpha, image_output);
  
  // TODO - time sync code cannot be put into the constructor of ImageView...why?
  // TODO - this needs to be a class only because of cv::destroyWindow
  ROS_INFO("Now subscribing to raw image topic: %s", image_input.c_str());
  message_filters::Subscriber<Image> image_sub(nh, image_input, 1);
    
  ROS_INFO("Now subscribing to data topic: %s (%s)", annotation_input.c_str(), annotation_type.c_str());
  message_filters::Subscriber<Image> info_sub(nh, annotation_input, 1);
  
  TimeSynchronizer<Image, Image> sync(image_sub, info_sub, 10);

  if (annotation_type == "segmentation") {
    sync.registerCallback(boost::bind(&ImageConverter::segmentationCb, &img, _1, _2));
  } else {
    ROS_ERROR("Annotation type not supported");
  }
  
  ros::spin();
  return 0;
}

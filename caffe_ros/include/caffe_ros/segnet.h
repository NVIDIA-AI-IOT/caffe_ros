// Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
// Full license terms provided in LICENSE.md file.

#ifndef CAFFE_ROS_SEGMENTATION_H
#define CAFFE_ROS_SEGMENTATION_H

namespace caffe_ros
{
void getArgMax(const float *scores, size_t num_channels, cv::Mat &class_map)
{
  size_t height = class_map.rows;
  size_t width = class_map.cols;
  
  for (size_t y = 0; y < height; y++)
  {
    for (size_t x = 0; x < width; x++)
    {
      // find max probability across the channels
      float p_max = -1.0f;
      int   c_max = -1;

      // loop through channels (each representing a class)
      for (size_t c = 0; c < num_channels; c++)
      {
        // get current score
        const float p = scores[c * width * height + y * width + x];

        if (c_max < 0 || p > p_max)
        {
          p_max = p;
          c_max = c;
        }
      }

      // set most likely class at proper pixel location
      class_map.at<unsigned char>(y, x) = (uint8_t) c_max;
    }
  }
}
}

#endif

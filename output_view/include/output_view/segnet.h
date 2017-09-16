// Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
// Full license terms provided in LICENSE.md file.

#ifndef SEGNET
#define SEGNET

bool loadClassColors(const char* filename, cv::Mat &lookUpTable)
{
    
  if(!filename)
    return false;

  FILE* f = fopen(filename, "r");

  if(!f)
    {
      printf("failed to open %s\n", filename);
      return false;
    }

  char str[512];
  int  idx = 0;

  while( fgets(str, 512, f) != NULL )
    {
      const int len = strlen(str);

      if( len > 0 )
	{
	  if( str[len-1] == '\n' )
	    str[len-1] = 0;

	  int r = 255;
	  int g = 255;
	  int b = 255;

	  sscanf(str, "%i %i %i ", &r, &g, &b);
	  lookUpTable.at<cv::Vec3b>(idx) = cv::Vec3b(r, g, b);
	  idx++;
	}
    }

  fclose(f);

  if( idx == 0 )
    return false;

  return true;
}

void colorImage(const cv::Mat &lookUpTable, const cv::Mat &values, cv::Mat &finalImage)
{
  for(int y=0; y<values.rows; y++){
    for(int x=0; x<values.cols; x++){
      int class_id = values.at<unsigned char>(cv::Point(x,y));
      cv::Vec3b color = lookUpTable.at<cv::Vec3b>(class_id);
      finalImage.at<cv::Vec3b>(cv::Point(x, y)) = color;
    }
  }
}
#endif

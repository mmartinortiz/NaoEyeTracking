#ifndef FINDFUNCTIONS_H
#define FINDFUNCTIONS_H

// Opencv includes.
#include <opencv2/core/core.hpp>


void findEyes(cv::Mat frame_gray, cv::Rect face);
cv::Mat findSkin (cv::Mat &frame);
int showImages(const std::string& robotIp);

#endif // FINDFUNCTIONS_H

#pragma once

#include <opencv.hpp>
#include <iostream>
#include <eigen3/Eigen/Dense>

namespace ImageGeneration
{
  inline std::vector<unsigned short int> ImgVector(VectorXd &f)
  {
    std::vector<unsigned short int> pixelValues;
    if (sqrt(f.size()) == 60)
    {

      cv::Mat image(60, 60, CV_8UC1, cv::Scalar(0));

      for (int y = 0; y < 60; ++y)
      {
        for (int x = 0; x < 60; ++x)
        {

          pixelValues.push_back(std::abs(f[x * 60 + y]) * 255.0);
        }
      }
    }
    else
    {
      cv::Mat image(30, 30, CV_8UC1, cv::Scalar(0));

      for (int y = 0; y < 30; ++y)
      {
        for (int x = 0; x < 30; ++x)
        {

          pixelValues.push_back(std::abs(f[x * 30 + y]) * 255.0);
        }
      }
    }
    return pixelValues;
  }

  inline void makeImage(VectorXd &f, string filename)
  {
    if (sqrt(f.size()) == 60)
    {

      cv::Mat image(60, 60, CV_8UC1, cv::Scalar(0));

      for (int y = 0; y < 60; ++y)
      {
        for (int x = 0; x < 60; ++x)
        {

          image.at<uchar>(x, y) = static_cast<uchar>(std::abs(f[y * 60 + x]) * 255.0);
        }
      }
      cv::imwrite("imagens\\" + filename + ".png", image);
    }

    else
    {
      cv::Mat image(30, 30, CV_8UC1, cv::Scalar(0));

      for (int y = 0; y < 30; ++y)
      {
        for (int x = 0; x < 30; ++x)
        {

          image.at<uchar>(x, y) = static_cast<uchar>(std::abs(f[y * 30 + x]) * 255.0);
        }
      }
      cv::imwrite("imagens\\" + filename + ".png", image);
    }
  }
}
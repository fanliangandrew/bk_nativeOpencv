//
// Created by zasx-fanliang on 2017/7/10.
//
#include <iostream>
#include <time.h>
#include <stack>


using namespace cv;
using namespace std;

#define MU1 0.9
#define MU2 25.5
#define SIGMA 4.5
#define WINDOW 19

#ifndef OPENCV_NATIVE_BINARY_H
#define OPENCV_NATIVE_BINARY_H

#endif //OPENCV_NATIVE_BINARY_H


  /*
   * function retinex_filter
   * produce a processing image for binarizing
   * src: src image in gray scale
   * dst: dst image in gray scale
   * window: window size of GaussianBlur
   * return 1 means success, 0 means fail
   */
  int retinex_filter(const Mat& src, Mat& dst, int window) {
      if (src.empty() || src.type() != CV_8UC1) {
          return 0;
      }
      dst.release();
      dst = src.clone();
      Mat gauss;
      double mu1 = MU1;
      //GaussianBlur(src, gauss, Size(5,5), 4.5, 4.5);
      GaussianBlur(src, gauss, Size(window,window), SIGMA, SIGMA);
      for (int i = 0; i < src.rows; i ++)
          for (int j = 0; j < src.cols; j ++) {
              unsigned char gs, is;
              gs = gauss.at<uchar>(i,j);
              is = src.at<uchar>(i,j);
              if (gs * mu1 > is)
                  dst.at<uchar>(i,j) = abs(is - gs);
              else
                  dst.at<uchar>(i,j) = 0;
          }
      return 1;
  }



  inline double distPt2f(const Point2f& a, const Point2f& b) {
      return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
  }

  // get radians from angle
  inline double radians(const double angle) {
      return angle * CV_PI / 180;
  }

  // get a random color
  Scalar GetRandomColor()
  {
      uchar r = 255 * (rand()/(1.0 + RAND_MAX));
      uchar g = 255 * (rand()/(1.0 + RAND_MAX));
      uchar b = 255 * (rand()/(1.0 + RAND_MAX));
      return Scalar(b,g,r);
  }


  /*
   * function SeedFilling
   * use seedfilling method to search CCs
   * binImg: src image, its pixels equal 1 or 0
   * labelImg: dst image, set pixels different values meaning which CC they belong
   * return the number of CCs found
   */
  int SeedFilling(const Mat& binImg, Mat& labelImg) {
      if (binImg.empty() || binImg.type() != CV_8UC1) {
          return 0;
      }

      labelImg.release();
      binImg.convertTo(labelImg, CV_32SC1);

      int label = 1;

      int rows = binImg.rows;
      int cols = binImg.cols;
      for (int i = 1; i < rows-1; i++)
      {
          int* data= labelImg.ptr<int>(i);
          for (int j = 1; j < cols-1; j++)
          {
              if (data[j] == 1)
              {
                  stack<pair<int,int> > neighborPixels;
                  neighborPixels.push(pair<int,int>(i,j));
                  ++label;
                  while (!neighborPixels.empty())
                  {
                      pair<int,int> curPixel = neighborPixels.top();
                      int curX = curPixel.first;
                      int curY = curPixel.second;
                      labelImg.at<int>(curX, curY) = label;

                      neighborPixels.pop();

                      for (int xx = curX-1; xx <= curX+1; xx ++)
                          for (int yy = curY-1; yy <= curY+1; yy ++) {
                              if (xx < 0 || yy < 0 || xx >= rows || yy >= cols)
                                  continue;
                              if (labelImg.at<int>(xx,yy) == 1)
                                  neighborPixels.push(pair<int,int>(xx,yy));
                          }
                  }
              }
          }
      }
      return label-1;
  }

  void thresholdSauvola(const cv::Mat& src, cv::Mat& dst, int winSize, double k)
  {
      CV_Assert(src.data && src.type() == CV_8UC1);
      CV_Assert(winSize > 0 && (winSize & 0x00000001));

      cv::Mat accum, accumSqr;
      cv::integral(src, accum, accumSqr, CV_64F, CV_64F);

      int r = winSize / 2;
      double R = 128;

      int rows = src.rows, cols = src.cols;
      dst.create(rows, cols, CV_8UC1);
      dst.setTo(0);
      for (int i = 0; i < rows; i++)
      {
          const unsigned char* ptrSrc = src.ptr<unsigned char>(i);
          unsigned char* ptrDst = dst.ptr<unsigned char>(i);
          int ybeg = std::max(i - r, 0), yend = std::min(i + r + 1, rows);
          int ydiff = yend - ybeg;
          for (int j = 0; j < cols; j++)
          {
              int xbeg = std::max(j - r, 0), xend = std::min(j + r + 1, cols);
              int xdiff = xend - xbeg;
              double sum = accum.at<double>(yend, xend) + accum.at<double>(ybeg, xbeg) -
                  accum.at<double>(ybeg, xend) - accum.at<double>(yend, xbeg);
              double sqrSum = accumSqr.at<double>(yend, xend) + accumSqr.at<double>(ybeg, xbeg) -
                  accumSqr.at<double>(ybeg, xend) - accumSqr.at<double>(yend, xbeg);
              // printf("xdiff %d ydiff %d\n", xdiff, ydiff);
              double invSize = 1.0 / (xdiff * ydiff);
              double mu = sum * invSize;
              // IMPORTANT NOTICE!!!
              // Floating point computation may introduce precision error,
              // variance may be negative!!!
              double var = sqrSum * invSize - mu * mu;
              double sigma = var < 0 ? 0 : sqrt(var);
              double t = mu * (1 + k * (sigma / R - 1));
              if (ptrSrc[j] > t)
                  ptrDst[j] = 255;
              else
                  ptrDst[j] = 0;
          }
      }
  }




  int main(int argc, char *argv[])
  {
      clock_t init, start, finish;
      start = clock();
      init = clock();
      double mu2 = MU2;

      // open image file with gray scale
      Mat src = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

      // set the times of swap to graph cut optimize
      int rows = src.rows;
      int cols = src.cols;
      // size of gaussian window for binarizing
      // int window = WINDOW_SIZE;
      int window = WINDOW;
      Mat retinex, binarImg, biImg, sauvoImg;

      //int k = atoi(argv[3]);
      thresholdSauvola(src, sauvoImg, 11, 0.08);
      imwrite(argv[2], sauvoImg);

      // binarize image
      retinex_filter(src, retinex, 19);
      // produce binarized image to save, dst(x,y)=255 if src(x,y)<mu2 else =0
      threshold(retinex, biImg, mu2, 255.0, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
      // produce image for search CCs, dst(x,y)=0 if src(x,y)<mu2 else =1
      // threshold(retinex, binarImg, mu2, 1.0, CV_THRESH_BINARY | CV_THRESH_OTSU);
      // save binarizing result
      imwrite(argv[3], biImg);

      return 1;

  }

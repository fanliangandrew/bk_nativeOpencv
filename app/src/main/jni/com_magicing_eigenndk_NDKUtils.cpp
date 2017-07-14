#include <jni.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

#include <time.h>
#include <stack>
#include <binary.h>
#include <detect_driving.h>
#include <cstring>

using namespace cv;
using namespace std;

#define MU1 0.9
#define MU2 25.5
#define SIGMA 4.5
#define WINDOW 19

extern "C"


    JNIEXPORT jintArray JNICALL Java_com_magicing_eigenndk_NDKUtils_detectDriving(
          JNIEnv *env, jclass obj, jintArray buf, int w, int h) {

      jint *cbuf;
      cbuf = env->GetIntArrayElements(buf, JNI_FALSE );
      if (cbuf == NULL) {
          return 0;
      }
      Mat srcMat(h, w, CV_8UC4, (unsigned char *) cbuf);

      Mat dstMat1;
      drivingDetector(srcMat,dstMat1);

      int nr= dstMat1.rows; // number of rows
      int nc= dstMat1.cols ; // total number of elements per line
      int rst_size =  nr*nc;

      jint temp[rst_size+2];
      jintArray rst = env->NewIntArray(rst_size+2);

      int* ptr = dstMat1.ptr<int>(0);
      for(int i = 0; i < rst_size; i ++){
          temp[i] = (int)ptr[i];
      }
      temp[rst_size] = nr;
      temp[rst_size+1] = nc;

      env->SetIntArrayRegion(rst,0,rst_size+2,temp);

      env->ReleaseIntArrayElements(buf, cbuf, 0);

      return rst;
  }


    JNIEXPORT jint JNICALL Java_com_magicing_eigenndk_NDKUtils_testString(
          JNIEnv *env, jclass obj, jintArray buf, int w, int h) {

      jint *cbuf;
      cbuf = env->GetIntArrayElements(buf, JNI_FALSE );
      if (cbuf == NULL) {
          return 0;
      }
      Mat srcMat(h, w, CV_8UC4, (unsigned char *) cbuf);

      Mat dstMat1;
      drivingDetector(srcMat,dstMat1);

      return dstMat1.cols;
  }

 JNIEXPORT jintArray JNICALL Java_com_magicing_eigenndk_NDKUtils_gray(
          JNIEnv *env, jclass obj, jintArray buf, int w, int h) {

      jint *cbuf;
      cbuf = env->GetIntArrayElements(buf, JNI_FALSE );
      if (cbuf == NULL) {
          return 0;
      }

      Mat imgData(h, w, CV_8UC4, (unsigned char *) cbuf);

      uchar* ptr = imgData.ptr(0);
      for(int i = 0; i < w*h; i ++){
          //计算公式：Y(亮度) = 0.299*R + 0.587*G + 0.114*B
          //对于一个int四字节，其彩色值存储方式为：BGRA
          int grayScale = (int)(ptr[4*i+2]*0.299 + ptr[4*i+1]*0.587 + ptr[4*i+0]*0.114);
          ptr[4*i+1] = grayScale;
          ptr[4*i+2] = grayScale;
          ptr[4*i+0] = grayScale;
      }

      int size = w * h;
      jintArray result = env->NewIntArray(size);
      env->SetIntArrayRegion(result, 0, size, cbuf);
      env->ReleaseIntArrayElements(buf, cbuf, 0);
      return result;
  }



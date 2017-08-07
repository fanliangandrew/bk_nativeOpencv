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

jclass gJniClass;
jmethodID gJniMethod;

extern "C"

//jintArray     : for image
//jobjectArray  : for Rect test
//jobject       : for java class
    JNIEXPORT jobject JNICALL Java_com_magicing_eigenndk_NDKUtils_detectDriving(
          JNIEnv *env, jclass obj, jintArray buf, int w, int h) {

      jint *cbuf;
      cbuf = env->GetIntArrayElements(buf, JNI_FALSE );
      if (cbuf == NULL) {
          return 0;
      }
      Mat srcMat(h, w, CV_8UC4, (unsigned char *) cbuf);

      printf("testing...");
      Mat dstMat1;
      vector<Rect> nameRects;
      vector<Rect> idRects;
      vector<Rect> typeRects;
      drivingDetector(srcMat,dstMat1,nameRects,idRects,typeRects);

      int nr= dstMat1.rows; // number of rows ,height
      int nc= dstMat1.cols ; // total number of elements per line ,width
      int rst_size =  nr*nc;

      jint temp[rst_size];
      jintArray rst = env->NewIntArray(rst_size);

      int* ptr = dstMat1.ptr<int>(0);
      for(int i = 0; i < rst_size; i ++){
          temp[i] = (int)ptr[i];
      }

      env->SetIntArrayRegion(rst,0,rst_size,temp);

      env->ReleaseIntArrayElements(buf, cbuf, 0);

      //return rst;

        //return rects , return class
        jint nameLength = nameRects.size();
        jint idLength   = idRects.size();
        jint typeLength = typeRects.size();

        jobjectArray name;
        jobjectArray id;
        jobjectArray type;
        jclass intArrCls = env->FindClass("[I");
        if(nameLength != 0 && idLength != 0 && typeLength != 0 ){
            name = env->NewObjectArray(nameLength,intArrCls,NULL);
            id   = env->NewObjectArray(idLength,intArrCls,NULL);
            type = env->NewObjectArray(typeLength,intArrCls,NULL);
            for (int i = 0; i < nameLength; i ++) {
                Rect oneChar = nameRects[i];
                jint tmpInt[4];
                jintArray tmpArr = env -> NewIntArray(4);
                tmpInt[0] = oneChar.x ;
                tmpInt[1] = oneChar.y ;
                tmpInt[2] = oneChar.width ;
                tmpInt[3] = oneChar.height ;
                env -> SetIntArrayRegion(tmpArr,0,4,tmpInt);
                env -> SetObjectArrayElement(name,i,tmpArr);
                env -> DeleteLocalRef(tmpArr);
            }
            for (int i = 0; i < idLength; i ++) {
                Rect oneChar = idRects[i];
                jint tmpInt[4];
                jintArray tmpArr = env -> NewIntArray(4);
                tmpInt[0] = oneChar.x ;
                tmpInt[1] = oneChar.y ;
                tmpInt[2] = oneChar.width ;
                tmpInt[3] = oneChar.height ;
                env -> SetIntArrayRegion(tmpArr,0,4,tmpInt);
                env -> SetObjectArrayElement(id,i,tmpArr);
                env -> DeleteLocalRef(tmpArr);
            }
            for (int i = 0; i < typeLength; i ++) {
                Rect oneChar = typeRects[i];
                jint tmpInt[4];
                jintArray tmpArr = env -> NewIntArray(4);
                tmpInt[0] = oneChar.x ;
                tmpInt[1] = oneChar.y ;
                tmpInt[2] = oneChar.width ;
                tmpInt[3] = oneChar.height ;
                env -> SetIntArrayRegion(tmpArr,0,4,tmpInt);
                env -> SetObjectArrayElement(type,i,tmpArr);
                env -> DeleteLocalRef(tmpArr);
            }
        }else{
            name = env->NewObjectArray(1,intArrCls,NULL);
            id   = env->NewObjectArray(1,intArrCls,NULL);
            type = env->NewObjectArray(1,intArrCls,NULL);
            jint tmpInt[4];
            jintArray tmpArr = env -> NewIntArray(4);
            tmpInt[0] = 0;
            tmpInt[1] = 0;
            tmpInt[2] = 0;
            tmpInt[3] = 0;
            env -> SetIntArrayRegion(tmpArr,0,4,tmpInt);
            env -> SetObjectArrayElement(name,0,tmpArr);
            env -> SetObjectArrayElement( id ,0,tmpArr);
            env -> SetObjectArrayElement(type,0,tmpArr);
            env -> DeleteLocalRef(tmpArr);
        }

        jclass imgdataCls = env -> FindClass("com/magicing/eigenndk/ImageData");
        jmethodID constrocMID = env-> GetMethodID(imgdataCls,"<init>","(II[I[Ljava/lang/Object;[Ljava/lang/Object;[Ljava/lang/Object;)V");
        jobject img_ojb = env->NewObject(imgdataCls,constrocMID,nr,nc,rst,name,id,type);

        return img_ojb;
  };


// demo for give complicated data to front
    JNIEXPORT jobject JNICALL Java_com_magicing_eigenndk_NDKUtils_preDetect(
        JNIEnv *env,jobject obj,jintArray buf,int w, int h){

        jclass imgdataCls = env -> FindClass("com/magicing/eigenndk/ImageData");
        jmethodID constrocMID = env-> GetMethodID(imgdataCls,"<init>","([I[Ljava/lang/Object;[Ljava/lang/Object;[Ljava/lang/Object;)V");

        jintArray img = env->NewIntArray(5);
        jint temp[5];
        for(int i = 0; i < 5; i ++){
            temp[i] = i;
        }
        env->SetIntArrayRegion(img,0,5,temp);

        jobjectArray name;
        jclass intArrCls = env->FindClass("[I");
        name = env->NewObjectArray(5,intArrCls,NULL);
        for (int i = 0; i < 5; i++) {
           jint tmp[3];
           jintArray iarr = env->NewIntArray(3);
           for(int j = 0; j < 3; j++) {
               tmp[j] = j;
           }
           env->SetIntArrayRegion(iarr, 0, 3, tmp);
           env->SetObjectArrayElement(name, i, iarr);
           env->DeleteLocalRef(iarr);
        }

        jobject img_ojb = env->NewObject(imgdataCls,constrocMID,img,name,name,name);
        return img_ojb;
    };

    JNIEXPORT jint JNICALL Java_com_magicing_eigenndk_NDKUtils_testString(
          JNIEnv *env, jclass obj, jintArray buf, int w, int h) {

      jint *cbuf;
      cbuf = env->GetIntArrayElements(buf, JNI_FALSE );
      if (cbuf == NULL) {
          return 0;
      }
      Mat srcMat(h, w, CV_8UC4, (unsigned char *) cbuf);

      Mat dstMat1;
      //drivingDetector(srcMat,dstMat1);

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



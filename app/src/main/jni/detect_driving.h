//
// Created by zasx-fanliang on 2017/7/10.
//

#ifndef OPENCV_NATIVE_DETECT_DRIVING_H
#define OPENCV_NATIVE_DETECT_DRIVING_H

#endif //OPENCV_NATIVE_DETECT_DRIVING_H

#include <opencv2/opencv.hpp>
#include <stack>
#include <iterator>
#include <map>
#include <algorithm>


using namespace cv;
using namespace std;

#define MU1 0.9
#define MU2 25.5
#define SIGMA 4.5

#define NAMESPACE_EXTEND 1.3


/*
 * function seedFilling
 * use seedfilling method to search CCs
 * binaryImg: src image, its pixels equal 1 or 0
 * labelImg: dst image, set pixels different values meaning which CC they belong
 * return the number of CCs found
 */
int seedFilling(const Mat& binaryImg, Mat& labelImg) {
    CV_Assert(binaryImg.data && binaryImg.type() == CV_8UC1);

    labelImg.release();
    binaryImg.convertTo(labelImg, CV_32SC1);

    int label = 1;

    int rows = binaryImg.rows;
    int cols = binaryImg.cols;
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

void binaryRetinex(const Mat& src, Mat& dst, int window) {
    CV_Assert(src.data && src.type() == CV_8UC1);
    CV_Assert(window > 0 && (window & 0x00000001));

    Mat tmpImg = src.clone();
    dst = src.clone();
    Mat gauss;
    double mu1 = MU1;
    double mu2 = MU2;
    //GaussianBlur(src, gauss, Size(5,5), 4.5, 4.5);
    GaussianBlur(src, gauss, Size(window,window), SIGMA, SIGMA);
    for (int i = 0; i < src.rows; i ++)
        for (int j = 0; j < src.cols; j ++) {
            unsigned char gs, is;
            gs = gauss.at<uchar>(i,j);
            is = src.at<uchar>(i,j);
            if (gs * mu1 > is)
                tmpImg.at<uchar>(i,j) = abs(is - gs);
            else
                tmpImg.at<uchar>(i,j) = 0;
        }
    threshold(tmpImg, dst, mu2, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);
}

// int filterConnectedDomain(const Mat& labelImg,
map<int, vector<Point> > filterConnectedDomain(const Mat& labelImg,
        int minPx,
        int maxPx,
        float hwRate,
        Mat& dotImg,
        Mat& charImg) {
    map<int, vector<Point> > ccm, charMap;
    if (labelImg.empty() || labelImg.type() != CV_32SC1) {
        // return -1;
        return ccm;
    }
    int rows = labelImg.rows;
    int cols = labelImg.cols;
    for (int i = 0; i < rows; i ++)
        for (int j = 0; j < cols; j ++) {
            int pixelValue = labelImg.at<int>(i,j);
            if (pixelValue > 1)
                ccm[pixelValue].push_back(Point(j,i));
        }
    map<int, vector<Point> >::iterator iter;
    int cnt = 0;
    for (iter = ccm.begin(); iter != ccm.end(); iter ++) {
        if (iter->second.size() <= minPx) {
            for (int k = 0; k < iter->second.size(); k ++) {
                int j = iter->second[k].x;
                int i = iter->second[k].y;
                charImg.at<uchar>(i,j) = 0;
                dotImg.at<uchar>(i,j) = 255;
            }
        } else if (iter->second.size() < maxPx){
            Rect rct = boundingRect(iter->second);
            if ((rct.height/rct.width) > hwRate || (rct.width/rct.height) > hwRate)
                continue;
            for (int k = 0; k < iter->second.size(); k ++) {
                int j = iter->second[k].x;
                int i = iter->second[k].y;
                charImg.at<uchar>(i,j) = 255;
            }
            charMap[cnt] = iter->second;
            cnt++;
        }
    }
    // return cnt;
    return charMap;
}

map<int, vector<Point> > filterConnectedDomain(const Mat& labelImg,
        int minPx,
        int maxPx,
        float hwRate) {
    map<int, vector<Point> > ccm, charMap;
    if (labelImg.empty() || labelImg.type() != CV_32SC1) {
        return ccm;
    }
    int rows = labelImg.rows;
    int cols = labelImg.cols;
    for (int i = 0; i < rows; i ++)
        for (int j = 0; j < cols; j ++) {
            int pixelValue = labelImg.at<int>(i,j);
            if (pixelValue > 1)
                ccm[pixelValue].push_back(Point(j,i));
        }
    map<int, vector<Point> >::iterator iter;
    int cnt = 0;
    for (iter = ccm.begin(); iter != ccm.end(); iter ++) {
        if (iter->second.size() < maxPx && iter->second.size() > minPx){
            Rect rct = boundingRect(iter->second);
            if ((rct.height/rct.width) > hwRate || (rct.width/rct.height) > hwRate)
                continue;
            charMap[cnt] = iter->second;
            cnt++;
        }
    }
    return charMap;
}

typedef pair<int, int> Pii;
int cmpMap(const Pii& a, const Pii& b) {
    return (a.second > b.second);
}
int cmpRctL2R(const Rect& a, const Rect& b) {
    return (b.x > a.x);
}
int cmpRctR2L(const Rect& a, const Rect& b) {
    return (b.x < a.x);
}
int cmpRctW(const Rect& a, const Rect& b) {
    return (a.width < b.width);
}
int cmpRctH(const Rect& a, const Rect& b) {
    return (a.height < b.height);
}


int getThreeRows(const Mat& dotImg, vector<int>& threeRows, int mergeInter) {
    if (dotImg.empty() || dotImg.type() != CV_8UC1) {
        return 0;
    }
    int rows = dotImg.rows;
    int cols = dotImg.cols;
    vector<int> ys;
    map<int, int> rowNumMap;
    for (int i = 0; i < rows; i ++) {
        rowNumMap[i] = 0;
        for (int j = 0; j < cols; j ++) {
            if(dotImg.at<uchar>(i,j))
                rowNumMap[i] ++;
        }
    }
    vector<Pii> rowNumVec(rowNumMap.begin(), rowNumMap.end());
    sort(rowNumVec.begin(), rowNumVec.end(), cmpMap);
    for (size_t i = 0; i < rowNumVec.size(); i ++) {
        int k = 1;
        for (size_t j = 0; j < ys.size(); j ++) {
            if (abs(rowNumVec[i].first - ys[j]) < mergeInter) {
                k = 0;
                break;
            }
        }
        if (k) {
            ys.push_back(rowNumVec[i].first);
        }
        if (ys.size() > 5)
            break;
    }
    if (ys.size() < 3) {
        fprintf(stderr, "Cannot find enough lines.\n");
        return 0;
    }
    sort(ys.begin(), ys.end());
    int i, inter = 0;
    for (i = ys.size()-1; i > 0; i --) {
        if (abs(ys[i] - ys[i-1] - inter) < 8)
            break;
        inter = ys[i] - ys[i-1];
    }
    if (abs(ys[i] - ys[i-1] - inter) >= 8) {
        fprintf(stderr, "Cannot find three lines.\n");
        return 0;
    }
    threeRows.push_back(ys[i-1]);
    threeRows.push_back(ys[i]);
    threeRows.push_back(ys[i+1]);
    return 1;
}

double modifySize(const Mat& src, Mat& dst, int minSide, int maxSide) {
    int cur = max(src.rows, src.cols);
    double f = 1.0;
    if (cur < minSide) {
        f = (double)minSide / cur;
        resize(src, dst, Size(), f, f);
        return f;
    }
    else if (cur > maxSide) {
        f = (double)maxSide / cur;
        resize(src, dst, Size(), f, f);
        return f;
    }
    else {
        dst = src;
        return f;
    }
}

int isRctinDom(Rect rct, int c, int r, int w, int h) {
    if (cvRound(rct.y+rct.height*0.2) > r
            && cvRound(rct.y+rct.height*0.8) < r+h)
        if (cvRound(rct.x+rct.width*0.2) > c
                && cvRound(rct.x+rct.width*0.8) < c+w)
            return 1;
    return 0;
}
int isCCinDom(const vector<Point>& cc, int c, int r, int w, int h) {
    Rect ccRct = boundingRect(cc);
    return isRctinDom(ccRct, c, r, w, h);
}



// find a whole chinese character in namespace
Rect getWholeChnChar(Rect target, vector<Rect>& rcts, Size minSize) {
    Rect tmpRct = target;
    while (tmpRct.width < minSize.width || tmpRct.height < minSize.height) {
        if (tmpRct.width < minSize.width) {
            tmpRct.x = tmpRct.x-1;
            tmpRct.width = tmpRct.width+2;
        }
        if (tmpRct.height < minSize.height) {
            tmpRct.y = tmpRct.y-1;
            tmpRct.height = tmpRct.height+2;
        }
        // cout << "tmprct: " << tmpRct << endl;
        for (size_t i = 0; i < rcts.size(); i ++) {
            // cout << "tmpRCT: " << tmpRct << rcts[i]<< endl;
            Rect andRct = tmpRct & rcts[i];
            Rect orRct = target | rcts[i];
            if (andRct.width > 0 && orRct != target) {
                // cout << target << " add " << rcts[i] << endl;
                target = orRct;
                tmpRct = target;
            }
        }
    }
    // cout << "tar: " << target << endl;
    /*
       for (size_t i = 0; i < rcts.size(); i ++) {
       Rect andRct = target & rcts[i];
       Rect orRct = target | rcts[i];
       if (andRct.width > 0 && orRct != target) {
       target = target | rcts[i];
       }
       }
       */
    if (target.width < minSize.width
            || target.height < minSize.height) {
        return Rect(0,0,0,0);
    }
    return target;
}

Rect searchChnChar(vector<Rect>& rcts, Rect predictRct, Size minSize) {
    Rect charRct(0,0,0,0);
    for (size_t i = 0; i < rcts.size(); i ++) {
        Rect rct = predictRct & rcts[i];
        if (rct.width > 0 && rcts[i].area() > charRct.area()) {
            //printf("heihei \n");
            charRct = rcts[i];
        }
    }
    // printf("AAA %d\n", d);
    charRct = getWholeChnChar(charRct, rcts, minSize);
    return charRct;
}

vector<Rect> searchTypeBox(map<int, vector<Point> >& ccm, int c, int r, int w, int h) {
    // cout << "KKKKKKKKKK" << endl;
    map<int, vector<Point> >::iterator iter, maxIter;
    int maxPx = 0;
    vector<Rect> rcts, typeRcts;
    for (iter = ccm.begin(); iter != ccm.end(); iter ++) {
        if (isCCinDom(iter->second, c, r, w, h) == 1) {
            Rect tmpRct = boundingRect(iter->second);
            // cout << "HHH: " << h << endl;
            // cout << tmpRct.width << endl;
            // cout << tmpRct.height << endl;
            if (tmpRct.width > h || tmpRct.height > h)
                continue;
            rcts.push_back(tmpRct);
            if (iter->second.size() > maxPx) {
                maxPx = iter->second.size();
                maxIter = iter;
            }
        }
    }
    // return rcts;
    Rect maxRct = boundingRect(maxIter->second);
    // cout << maxRct << endl;
    typeRcts.push_back(maxRct);
    // nameRcts.push_back(maxRct);
    // return nameRcts;
    // int num = rcts.size();
    // sort(rcts.begin(), rcts.end(), cmpRctW);
    // Size s = rcts[num/2].size;
    // typeRcts.push_back(rcts[num/2]);
    // return typeRcts;

    int interRef = cvRound(h/NAMESPACE_EXTEND);
    // Size typeCharSize(cvRound(interRef/4.0), cvRound(interRef/2.5));
    // Size typeCharSize(cvRound(interRef/4.0), cvRound(interRef/2)-3);
    Size typeCharSize(8, cvRound(interRef/2)-4);
    // cout << typeCharSize << endl;
    // cout << "charH: "<<nameCharSize.height<< endl;
    // typeRcts.push_back(Rect(c, r, nameCharSize.width, nameCharSize.height));
    // return typeRcts;


    // cout << "maxrct " << maxRct <<endl;

    // search the first char, IMPORTANT!
    maxRct = getWholeChnChar(maxRct, rcts, typeCharSize);
    if (maxRct.width == 0) {
        printf("CANNOT find first char\n");
        return typeRcts;
    }
    // typeRcts.push_back(maxRct);
    // return typeRcts;
    Rect first = maxRct;
    // cout << "first char: "<<first <<endl;

    // erase underlines
    vector<Rect>::iterator itRct;
    for (itRct = rcts.begin(); itRct != rcts.end(); ) {
        if (abs(maxRct.y+maxRct.height-itRct->y) < 5)
            rcts.erase(itRct);
        else
            itRct ++;
    }

    Rect leftRct = first;
    leftRct.x = leftRct.x - leftRct.width;
    Rect rightRct = first;
    rightRct.x = rightRct.x + rightRct.width;

    while (isRctinDom(leftRct, c, r, w, h)) {
        Rect charRct = searchChnChar(rcts, leftRct, typeCharSize);
        if (charRct.x == 0 || charRct.x+charRct.width > leftRct.x+leftRct.width) {
            break;
            // leftRct.x = leftRct.x - leftRct.width;
            // continue;
        }
        typeRcts.push_back(charRct);
        leftRct = charRct;
        leftRct.x = charRct.x - charRct.width;
    }
    // return typeRcts;
    while (isRctinDom(rightRct, c, r, w, h)) {
        Rect charRct = searchChnChar(rcts, rightRct, typeCharSize);
        if (charRct.x == 0 || charRct.x < rightRct.x) {
            break;
            // rightRct.x = rightRct.x + rightRct.width;
            // continue;
        }
        typeRcts.push_back(charRct);
        rightRct = charRct;
        rightRct.x = charRct.x + charRct.width;
    }

    sort(typeRcts.begin(), typeRcts.end(), cmpRctL2R);

    return typeRcts;
}


vector<Rect> searchNameBox(map<int, vector<Point> >& ccm, int c, int r, int w, int h) {
    map<int, vector<Point> >::iterator iter, maxIter;
    int maxPx = 0;
    vector<Rect> rcts, nameRcts;
    for (iter = ccm.begin(); iter != ccm.end(); iter ++) {
        if (isCCinDom(iter->second, c, r, w, h) == 1) {
            Rect tmpRct = boundingRect(iter->second);
            rcts.push_back(tmpRct);
            if (iter->second.size() > maxPx) {
                maxPx = iter->second.size();
                maxIter = iter;
            }
        }
    }
    Rect maxRct = boundingRect(maxIter->second);
    int interRef = cvRound(h/NAMESPACE_EXTEND);
    Size nameCharSize(interRef, interRef/2);

    // cout << "maxrct " << maxRct <<endl;

    // search the first char, IMPORTANT!
    maxRct = getWholeChnChar(maxRct, rcts, nameCharSize);
    if (maxRct.width == 0) {
        printf("CANNOT find first char\n");
        return nameRcts;
    }
    nameRcts.push_back(maxRct);
    // return nameRcts;
    Rect first = maxRct;
    // cout << "first char: "<<first <<endl;

    // erase underlines
    vector<Rect>::iterator itRct;
    for (itRct = rcts.begin(); itRct != rcts.end(); ) {
        if (abs(maxRct.y+maxRct.height-itRct->y) < 5)
            rcts.erase(itRct);
        else
            itRct ++;
    }

    Rect leftRct = first;
    leftRct.x = leftRct.x - leftRct.width;
    Rect rightRct = first;
    rightRct.x = rightRct.x + rightRct.width;

    while (isRctinDom(leftRct, c, r, w, h)) {
        Rect charRct = searchChnChar(rcts, leftRct, nameCharSize);
        if (charRct.x == 0 || charRct.x+charRct.width > leftRct.x+leftRct.width) {
            leftRct.x = leftRct.x - leftRct.width;
            continue;
        }
        nameRcts.push_back(charRct);
        leftRct = charRct;
        leftRct.x = charRct.x - charRct.width;
    }
    while (isRctinDom(rightRct, c, r, w, h)) {
        Rect charRct = searchChnChar(rcts, rightRct, nameCharSize);
        if (charRct.x == 0 || charRct.x < rightRct.x) {
            rightRct.x = rightRct.x + rightRct.width;
            continue;
        }
        nameRcts.push_back(charRct);
        rightRct = charRct;
        rightRct.x = charRct.x + charRct.width;
    }

    sort(nameRcts.begin(), nameRcts.end(), cmpRctL2R);

    return nameRcts;
}

//vector<Rect> searchDateBox(map<int, vector<Point>>& ccm, int c, int r, int w, int h) {
vector<vector<Rect> > searchDateBox(map<int, vector<Point> >& ccm, int c, int r, int w, int h) {
    map<int, vector<Point> >::iterator iter;
    int maxPx = 0;
    vector<Rect> rcts, numRcts;
    for (iter = ccm.begin(); iter != ccm.end(); iter ++) {
        if (isCCinDom(iter->second, c, r, w, h) == 1) {
            Rect tmpRct = boundingRect(iter->second);
            rcts.push_back(tmpRct);
            if (tmpRct.height > h/2
                    && tmpRct.height < h
                    && tmpRct.width < tmpRct.height)
                numRcts.push_back(tmpRct);
        }
    }
    int num = numRcts.size();
    vector<vector<Rect> > vRcts;
    if (num < 3) {
        printf("CANNOT find any number characters.\n");
        //return numRcts;
        return vRcts;
    }
    // get MEDIAN size
    sort(numRcts.begin(), numRcts.end(), cmpRctW);
    int medW = numRcts[num/2].width;
    sort(numRcts.begin(), numRcts.end(), cmpRctH);
    int medH = numRcts[num/2].height;
    Size numCharSize(2, medH-3);
    // cout << "numCharSize" << numCharSize <<endl;

    //vector<vector<Rect>> vRcts;


    vector<Rect>::iterator it;
    for (it = numRcts.begin(); it != numRcts.end();) {
        if (it->width > (medW+3)) {
            // printf("zhishao???\n");
            it = numRcts.erase(it);
            continue;
        }
        it ++;
    }


    // search backward
    sort(numRcts.begin(), numRcts.end(), cmpRctR2L);
    for (it = numRcts.begin(); it != numRcts.end();) {
        if (abs(it->x-(it+1)->x) > 3*medW || it==numRcts.end()-1) {
            Rect r; // = numRcts[i] | numRcts[i-1];
            r.y = it->y;
            r.x = it->x - medW;
            r.width = medW;
            r.height = medH;
            for (int i = 0; i < 2; i ++) {
                Rect charRct = searchChnChar(rcts, r, numCharSize);
                if(charRct.x > 0
                        && charRct.width <= (medW+2)
                        && charRct.height < (medH+3)) {
                    it = numRcts.insert(it+1, charRct);
                    r.x = -1;
                    break;
                }
                r.x = r.x - medW;
            }
            if (r.x != -1)
                it ++;
        } else if (abs(it->x-(it+1)->x) > 2*medW) {
            Rect r; // = numRcts[i] | numRcts[i-1];
            r.y = it->y;
            r.x = it->x - medW;
            r.width = medW;
            r.height = medH;
            Rect charRct = searchChnChar(rcts, r, numCharSize);
            if(charRct.x > 0
                    && charRct.width <= (medW+2)
                    && charRct.height < (medH+3)) {
                it = numRcts.insert(it+1, charRct);
                continue;
            }
            it ++;

        }
        else it ++;
    }
    // search forward
    sort(numRcts.begin(), numRcts.end(), cmpRctL2R);
    for (it = numRcts.begin(); it != numRcts.end();) {
        if (abs(it->x-(it+1)->x) > 3*medW || it==numRcts.end()-1) {
            Rect r; // = numRcts[i] | numRcts[i-1];
            r.y = it->y;
            r.x = it->x + it->width;
            r.width = medW;
            r.height = medH;
            for (int i = 0; i < 2; i ++) {
                Rect charRct = searchChnChar(rcts, r, numCharSize);
                if(charRct.x > 0
                        && charRct.width <= (medW+2)
                        && charRct.height < (medH+3)) {
                    it = numRcts.insert(it+1, charRct);
                    r.x = -1;
                    break;
                }
                r.x = r.x + medW;
            }
            if (r.x != -1)
                it ++;
        } else if (abs(it->x-(it+1)->x) > 2*medW) {
            Rect r; // = numRcts[i] | numRcts[i-1];
            r.y = it->y;
            r.x = it->x + it->width;
            r.width = medW;
            r.height = medH;
            Rect charRct = searchChnChar(rcts, r, numCharSize);
            if(charRct.x > 0
                    && charRct.width <= (medW+2)
                    && charRct.height < (medH+3)) {
                it = numRcts.insert(it+1, charRct);
                continue;
            }
            it ++;

        }
        else it ++;
    }
    // return numRcts;


    vector<Rect>::iterator it1, it2;
    it1 = numRcts.begin();
    for (it2 = numRcts.begin()+1; it2 != numRcts.end(); ) {
        if (abs((it2-1)->x-it2->x) > 3*medW) {
            vector<Rect> tmpRcts;
            tmpRcts.insert(tmpRcts.end(), it1, it2);
            if (tmpRcts.size() > 4)
                vRcts.push_back(tmpRcts);
            it1 = it2;
            it2 ++;
        }
        else it2++;
    }
    vector<Rect> tmpRcts;
    tmpRcts.insert(tmpRcts.end(), it1, it2);
    if (tmpRcts.size() > 4)
        vRcts.push_back(tmpRcts);

    return vRcts;
}

vector<Rect> searchIdBox(map<int, vector<Point> >& ccm, int c, int r, int w, int h) {
    map<int, vector<Point> >::iterator iter;
    int maxPx = 0;
    vector<Rect> rcts, numRcts;
    for (iter = ccm.begin(); iter != ccm.end(); iter ++) {
        if (isCCinDom(iter->second, c, r, w, h) == 1) {
            Rect tmpRct = boundingRect(iter->second);
            rcts.push_back(tmpRct);
            if (tmpRct.height > h/3
                    && tmpRct.height < h
                    && tmpRct.width < tmpRct.height)
                numRcts.push_back(tmpRct);
        }
    }
    // small cc
    // return rcts;
    int num = numRcts.size();
    if (num < 3) {
        printf("CANNOT find any number characters.\n");
        numRcts.clear();
        return numRcts;
    }
    // return numRcts;
    // get MEDIAN size
    sort(numRcts.begin(), numRcts.end(), cmpRctW);
    int medW = numRcts[num/2].width;
    sort(numRcts.begin(), numRcts.end(), cmpRctH);
    int medH = numRcts[num/2].height;

    Size numCharSize(2, medH-3);
    // cout << "numCharSize" << numCharSize <<endl;

    //vector<vector<Rect>> vRcts;


    vector<Rect>::iterator it;
    for (it = numRcts.begin(); it != numRcts.end();) {
        if (it->width > (medW+3)) {
            // printf("zhishao???\n");
            it = numRcts.erase(it);
            continue;
        }
        it ++;
    }


    // search backward
    // cout << 2*medW << endl;
    sort(numRcts.begin(), numRcts.end(), cmpRctR2L);
    for (it = numRcts.begin(); it != numRcts.end();) {
        // cout << "mmmm"<< *it << endl;
        if (abs(it->x-(it+1)->x) > 1.6*medW || it == numRcts.end()-1) {
            Rect r; // = numRcts[i] | numRcts[i-1];
            r.y = it->y;
            r.x = it->x - medW;
            r.width = medW;
            r.height = medH;
            // cout << r<< endl;
            Rect charRct = searchChnChar(rcts, r, numCharSize);
            if(charRct.x > 0
                    && charRct.width <= (medW+2)
                    && charRct.height < (medH+3)) {
                // cout <<"kkkk" << charRct << endl;
                it = numRcts.insert(it+1, charRct);
                continue;
            }
            it ++;

        }
        else it ++;
    }
    // return numRcts;
    // printf("f\n");
    // search forward
    sort(numRcts.begin(), numRcts.end(), cmpRctL2R);
    // cout << "sum: " << numRcts.size() << endl;
    for (it = numRcts.begin(); it != numRcts.end();) {
        if (abs(it->x-(it+1)->x) > 2*medW || it == numRcts.end()-1) {
            Rect r; // = numRcts[i] | numRcts[i-1];
            r.y = it->y;
            r.x = it->x + it->width;
            r.width = medW;
            r.height = medH;
            Rect charRct = searchChnChar(rcts, r, numCharSize);
            if(charRct.x > 0
                    && charRct.width <= (medW+2)
                    && charRct.height < (medH+3)) {
                it = numRcts.insert(it+1, charRct);
                continue;
            }
            it ++;

        }
        else it ++;
    }

    for (it = numRcts.begin()+1; it != numRcts.end();) {
        Rect andRct = *it & *(it-1);
        Rect orRct = *it | *(it-1);
        if (andRct.x > 0) {
            *(it-1) = orRct;
            numRcts.erase(it);
        }
        else it++;
    }

    for (it = numRcts.begin()+1; it != numRcts.end(); ) {
        if (abs((it-1)->x-it->x) > 2*medW) {
            Rect r;
            r = *(it-1) | *it;
            r.x = (it-1)->x + (it-1)->width + 2;
            r.width = it->x - r.x - 2;
            Rect tmpRct(0,0,0,0);
            for (int i = 0; i < rcts.size(); i ++) {
                if (isRctinDom(rcts[i], r.x, r.y, r.width, r.height)) {
                    if (tmpRct.x == 0)
                        tmpRct = rcts[i];
                    else
                        tmpRct = tmpRct | rcts[i];
                }
            }
            if (tmpRct.x != 0)
                it = numRcts.insert(it, tmpRct);
            else
                it ++;
        }
        else it ++;
    }

    // return numRcts;

    vector<vector<Rect> > vRcts;
    vector<Rect>::iterator it1, it2;
    it1 = numRcts.begin();
    for (it2 = numRcts.begin()+1; it2 != numRcts.end(); ) {
        if (abs((it2-1)->x-it2->x) > 2*medW) {
            vector<Rect> tmpRcts;
            tmpRcts.insert(tmpRcts.end(), it1, it2);
            vRcts.push_back(tmpRcts);
            it1 = it2;
            it2 ++;
        }
        else it2++;
    }
    vector<Rect> tmpRcts;
    tmpRcts.insert(tmpRcts.end(), it1, it2);
    vRcts.push_back(tmpRcts);

    vector<vector<Rect> >::iterator itv, itvmax;
    int maxN = 0;
    for (itv = vRcts.begin(); itv < vRcts.end(); itv ++) {
        if (itv->size() > maxN) {
            itvmax = itv;
            maxN = itv->size();
        }
    }
    return *itvmax;


}

void drivingDetector(const Mat& src, Mat& dst ,vector<Rect>& nameRects, vector<Rect>& idRects, vector<Rect>& typeRects) {
    //CV_Assert(src.data && src.type() == CV_8UC3);
    Mat colorImg, grayImg;
    double f = modifySize(src, colorImg, 800, 800);
    cvtColor(colorImg, grayImg, CV_BGR2GRAY);
    int rows = grayImg.rows;
    int cols = grayImg.cols;
    Mat retinexImg, oneImg, labelImg;
    binaryRetinex(grayImg, retinexImg, 19);
    threshold(retinexImg, oneImg, 25, 1.0, CV_THRESH_BINARY | CV_THRESH_OTSU);
    int numCC = seedFilling(oneImg, labelImg);
    Mat dotImg, charImg;
    dotImg = Mat::zeros(rows, cols, CV_8UC1);
    charImg = Mat::zeros(rows, cols, CV_8UC1);
    filterConnectedDomain(labelImg, 15, 800, 13.0, dotImg, charImg);


    vector<int> threeRs;
    int isDetect = getThreeRows(dotImg, threeRs, 40);
    if (isDetect == 0 || threeRs.size() < 3 ) {
        dst = colorImg;
        return ;
    }
    int baseRow = threeRs[0];
    float interSpace = (threeRs[2]-threeRs[0])/2.0;

    Rect nameDom(0, cvRound(baseRow-3.1*interSpace), cols/2, cvRound(interSpace*NAMESPACE_EXTEND));
    Rect idDom(cvRound(0.35*cols), cvRound(baseRow-4.1*interSpace), cvRound(0.6*cols), cvRound(interSpace*NAMESPACE_EXTEND));
    Rect typeDom(cvRound(0.4*cols), baseRow+2*interSpace, cvRound(cols/3), cvRound(interSpace*NAMESPACE_EXTEND));

    int segMargin = 2;
    map<int, vector<Point> > ccm = filterConnectedDomain(labelImg, interSpace/2, interSpace*interSpace, 13.0);
    nameRects = searchNameBox(ccm, nameDom.x, nameDom.y, nameDom.width, nameDom.height);

    ccm = filterConnectedDomain(labelImg, 1, interSpace*10, 13.0);
    idRects = searchIdBox(ccm, idDom.x, idDom.y, idDom.width, idDom.height);

    ccm = filterConnectedDomain(labelImg, interSpace/3, interSpace*10, 5.0, dotImg, charImg);
    typeRects = searchTypeBox(ccm, typeDom.x, typeDom.y, typeDom.width, typeDom.height);

    /* // draw the rects on the img
    for( int k=0;k<nameRects.size();k++){
        rectangle(colorImg,nameRects[k],Scalar(0,0,255), 2);
    }
    rectangle(colorImg, nameDom, Scalar(0,0,255), 2);
    rectangle(colorImg, idDom, Scalar(0,0,255), 2);
    rectangle(colorImg, typeDom, Scalar(0,0,255), 2);
    */
    dst = colorImg;

    return;
}

/*
int main(int argc, char *argv[])
{

    Mat srcImg = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    Mat retImg;
    drivingDetector(srcImg, retImg);

    imwrite(argv[2], retImg);



    return 1;

}
*/


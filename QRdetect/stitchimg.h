#ifndef STITCHIMG_H
#define STITCHIMG_H

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include "klargestheap.h"
#include <zbar.h>

using namespace std;
using namespace cv;
using namespace zbar;

struct cnrs {
    Point tl;
    Point tr;
    Point bl;
    Point br;
};

struct qrcode {
    Mat code;
    cnrs corners;
    Point2f loctn;
};

struct decodedObject
{
    string type;
    string data;
    vector<Point> location;
};

Point2f intersection(cnrs corner);

void rotate_90n(Mat const &src, Mat &dst, int angle);

class stitchImg
{
public:
	stitchImg();
    stitchImg(bool debug);

    ~stitchImg();

    void undistortImg(Mat &img);

    bool stitch(int perm[4]);
    bool stitch();

    Mat *getQRcode(){return &QRcode;}

    decodedObject *getData(){return &obj;}

    bool QRcodeRead();

	// Takes contours and sorts them largest to smallest in terms of enclosed area
	struct sortContour
	{
		inline bool operator() (const vector<Point> &a, const vector<Point> &b)
		{
			double areaA = fabs(contourArea(Mat(a)));
			double areaB = fabs(contourArea(Mat(b)));
			return (areaA > areaB);
		}
    };

	// Takes pixel values and sorts them largest to smallest in terms of measurement parameter
    struct sortPointPair
    {
        inline bool operator() (const pair<int, Point> &a, const pair<int,Point> &b)
        {
            return (a.first > b.first);
        }
    };
private:
    bool codeLocn(Mat &img);
	bool drawDebug;
    cnrs findCrns(Mat &img);
    vector<qrcode> codeElements;
    vector<Point> dst_corners;
    Mat QRcode;
    cnrs QRcorner;
    decodedObject obj;
};

#endif // STITCHIMG_H

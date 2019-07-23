/*#include <iostream>
#include <exception>
#include "stitchimg.h"
#include <zbar.h>

using namespace std;
using namespace zbar;

struct decodedObject
{
    string type;
    string data;
    vector<Point> location;
};

bool sortContour(vector<Point> a, vector<Point> b);
bool sortPointPair(pair<int,Point> a, pair<int,Point> b);
Point getCorner(Point a, Point b, Point c);

int main()
{
    // Create the image stitching class, which takes in images, and saves them if seen
    stitchImg imgStitcher;
    char const* source_window = "Constructed QR code";
    char const* altered_window = "Processed Image";
    char const* contour_window = "Contours";
    namedWindow(source_window, WINDOW_AUTOSIZE);
    namedWindow(altered_window, WINDOW_AUTOSIZE);
    namedWindow(contour_window, WINDOW_AUTOSIZE);

    Mat src1 = imread("../QRdetect/QR_photos/tl.PNG");
    Mat src3 = imread("../QRdetect/QR_photos/tr.PNG");
    Mat src2 = imread("../QRdetect/QR_photos/bl.PNG");
    Mat src4 = imread("../QRdetect/QR_photos/br.PNG");
    Mat img;
    src3.copyTo(img);
    cout << img.depth() << " " << img.channels() << endl;
    imshow(source_window,img);
    Mat blackWhite, gray;
    cvtColor(img,gray,CV_BGR2GRAY);
    blur(gray,gray,Size(3,3));
    inRange(gray, 150, 255, blackWhite);
    //dilate(blackWhite,blackWhite,Mat(),Point(-1,-1));
    imshow("src", blackWhite);

    vector<vector<Point>> SimpContours, L1Contours,KCOSContours;
    Mat SimpImg, L1Img, KCOSImg;
    img.copyTo(SimpImg);
    img.copyTo(L1Img);
    img.copyTo(KCOSImg);

    findContours(blackWhite,SimpContours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    sort(SimpContours.begin(),SimpContours.end(),sortContour);
    vector<Point> hull;
    Point2f centre;
    float radius;
    minEnclosingCircle(SimpContours[0],centre,radius);
    convexHull(SimpContours[0],hull);

    Scalar conColour = Scalar(0,0,255);
    Scalar hullColour = Scalar(255,0,0);
    Scalar circleColour = Scalar(0,255,0);
    drawContours(SimpImg,SimpContours,0,conColour,1,8);
    drawContours(SimpImg,vector<vector<Point>> {hull},0,hullColour,1,8);
    circle(SimpImg,centre,(int)radius,circleColour,1,8);

    // Iterate across all convex Hull points to find three corners of the QR code
    vector<pair<int,Point>> dist;
    for(unsigned int i = 0; i < hull.size(); i++)
    {
        int xH = hull[i].x;
        int yH = hull[i].y;
        int xC = centre.x;
        int yC = centre.y;
        dist.push_back(make_pair(SQ(xH - xC) + SQ(yH - yC),hull[i]));
    }
    sort(dist.begin(),dist.end(),sortPointPair);
    vector<pair<int,Point>> corners = {dist[0]};
    vector<pair<int,Point>>::iterator it;
    for(unsigned int i = 1; i < hull.size(); i++)
    {
        if (dist[i].first < SQ(0.66*radius)){
            continue;
        }else{
            bool too_close = false;
            for(it = corners.begin(); it < corners.end(); it++){
                double separation = (double)(SQ(dist[i].second.x - (*it).second.x))
                        + (double)(SQ(dist[i].second.y - (*it).second.y));
                if (separation < SQ(0.66*radius)){
                    too_close = true;
                    break;
                }
            }
            if(!too_close)
                corners.push_back(dist[i]);
        }
    }

    for(it = corners.begin(); it < corners.end(); it++){
        Scalar cornColour = Scalar(255,255,255);
        circle(SimpImg,(*it).second,4,cornColour,-1,8);
    }
    Point pnt;
    cnrs pnts;
    bool tl = false,tr = false,bl = false,br = false;
    for (int i = 0; i < 3; i++){
        pnt = corners[i].second;
        if(pnt.x < centre.x){
            if(pnt.y < centre.y){
                pnts.tl = pnt;
                tl = true;
            }else{
                pnts.bl = pnt;
                bl = true;
            }
        }else{
            if(pnt.y < centre.y){
                pnts.tr = pnt;
                tr = true;
            }else{
                pnts.br = pnt;
                br = true;
            }
        }
    }

    // Find coordinate of fourth corner point
    if(!tl){
        int x, y;
        x = centre.x - abs(pnts.br.x - centre.x);
        y = centre.y - abs(pnts.br.y - centre.y);
        pnts.tl = Point(x,y);
    }else if (!tr){
        int x,y;
        x = centre.x + abs(pnts.bl.x - centre.x);
        y = centre.y - abs(pnts.bl.y - centre.y);
        pnts.tr = Point(x,y);
    }else if (!bl){
        int x,y;
        x = centre.x - abs(pnts.tr.x - centre.x);
        y = centre.y + abs(pnts.tr.y - centre.y);
        pnts.bl = Point(x,y);
    }else if (!br){
        int x,y;
        x = centre.x + abs(pnts.tl.x - centre.x);
        y = centre.y + abs(pnts.tl.y - centre.y);
        pnts.br = Point(x,y);
    }

    Mat dst_img;
    vector<Point> dst_corners = {Point(0,0),Point(0,200),Point(200,200),Point(200,0)};
    vector<Point> src_corners{pnts.tl,pnts.bl,
                         pnts.br,pnts.tr};
    Mat h = findHomography(src_corners,dst_corners);
    warpPerspective(img, dst_img, h, Size(200,200));

    imshow("unwarped", dst_img);
    waitKey(0);
    /*vector<vector<Point>> hull(SimpContours.size());
    vector<Point2f> centres(SimpContours.size());
    vector<float> radii(SimpContours.size());
    for (size_t i = 0; i < SimpContours.size(); i++){
        minEnclosingCircle(SimpContours[i],centres[i],radii[i]);
        convexHull(SimpContours[i],hull[i]);
    }
    Scalar conColour = Scalar(0,0,255);
    Scalar hullColour = Scalar(255,0,0);
    Scalar circleColour = Scalar(0,255,0);
    for(size_t i = 0; i < SimpContours.size(); i++){
        drawContours(SimpImg,SimpContours,(int)i,conColour,1,8);
        drawContours(SimpImg,hull,(int)i,hullColour,2,8);
        circle(SimpImg,centres[i],(int)radii[i],circleColour,2);
    }

    findContours(blackWhite,L1Contours,RETR_EXTERNAL,CHAIN_APPROX_TC89_L1);
    for(size_t i = 0; i < L1Contours.size(); i++){
        Scalar colour = Scalar(0,0,255);
        drawContours(L1Img,L1Contours,(int)i,colour,2,8);
    }
    imshow("L1 Contour", L1Img);

    findContours(blackWhite,KCOSContours,RETR_EXTERNAL,CHAIN_APPROX_TC89_KCOS);
    for(size_t i = 0; i < KCOSContours.size(); i++){
        Scalar colour = Scalar(0,0,255);
        drawContours(KCOSImg,KCOSContours,(int)i,colour,2,8);
    }
    imshow("KCOS Contour", KCOSImg);
    imshow(contour_window,SimpImg);
    waitKey(0);
}

bool sortContour(vector<Point> a, vector<Point> b)
{
    double areaA = fabs(contourArea(Mat(a)));
    double areaB = fabs(contourArea(Mat(b)));
    return (areaA > areaB);
}

bool sortPointPair(pair<int,Point> a, pair<int,Point> b)
{
    return (a.first > b.first);
}

Point getCorner(Point a, Point b, Point c)
{

}
*/

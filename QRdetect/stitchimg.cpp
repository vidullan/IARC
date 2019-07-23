#include "stitchimg.h"

stitchImg::stitchImg()
{
	dst_corners.push_back(Point(0, 0));
	dst_corners.push_back(Point(0, 200));
	dst_corners.push_back(Point(200, 200));
	dst_corners.push_back(Point(200, 0));

    QRcode = cv::Mat::zeros(400, 400, CV_8UC1);
	drawDebug = false;
}

stitchImg::stitchImg(bool debug)
{
    dst_corners.push_back(Point(0,0));
    dst_corners.push_back(Point(0,200));
    dst_corners.push_back(Point(200,200));
    dst_corners.push_back(Point(200,0));

    QRcode = cv::Mat::zeros(400, 400, CV_8UC1);
	drawDebug = debug;
}

stitchImg::~stitchImg(){}

void stitchImg::undistortImg(Mat &img)
{
    // Determine code type and pixel location:
    // type - is code the top left, top right,
    // bottom left, bottom right segment
    bool found = codeLocn(img);        

    Mat dst_img;
    if (found){
        // Undistort the image segment
        //vector<Point> src_corners{Point((*pQr).corners.tl),Point((*pQr).corners.bl),
        //                     Point((*pQr).corners.br),Point((*pQr).corners.tr)};
		vector<Point> src_corners{ Point(QRcorner.tl),Point(QRcorner.bl),Point(QRcorner.br),
								Point(QRcorner.tr) };
        Mat h = findHomography(src_corners,dst_corners);
        warpPerspective(img, dst_img, h, Size(200,200));
        cvtColor(dst_img,dst_img,CV_BGR2GRAY);
    } else{
        return;
    }

    qrcode segment;
	segment.corners = QRcorner;
    dst_img.copyTo(segment.code);
	codeElements.push_back(segment);

    /*if ((*pQr).loctn == "topl")
        dst_img.copyTo(topl.code);
    else if ((*pQr).loctn == "topr")
        dst_img.copyTo(topr.code);
    else if ((*pQr).loctn == "botl")
        dst_img.copyTo(botl.code);
    else if ((*pQr).loctn == "botr")
        dst_img.copyTo((botr.code));*/
}

bool stitchImg::codeLocn(Mat& img)
{
	Mat blackWhite, gray;
	cvtColor(img, gray, CV_BGR2GRAY);
	blur(gray, gray, Size(3, 3));
	inRange(gray, 150, 255, blackWhite);

	vector<vector<Point>> SimpContours;
	Mat SimpImg;
	img.copyTo(SimpImg);

	// Find contours in the Black and White image. These contours will trace at least two of the four sidese of QR code
	// Then sort contours in terms of largest area. This way, only contour tracing QR code boarder is taken
	findContours(blackWhite, SimpContours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    sort(SimpContours.begin(), SimpContours.end(), sortContour());
	// Only take the greater convex shape of the QR code contour. This reduces the number of contour corner points found
	vector<Point> hull;
	convexHull(SimpContours[0], hull);
	// Draw the smallest sized circle enclosing all the points of the QR contour. This will include a minimum of three of the four
	// QR code corners. These corners will lie closely to the enclosing circle boarder
	Point2f centre;
	float radius;
	minEnclosingCircle(SimpContours[0], centre, radius);

	if (drawDebug) {
		Scalar conColour	= Scalar(0, 0, 255);
		Scalar hullColour	= Scalar(255, 0, 0);
		Scalar circleColour = Scalar(0, 255, 0);
		drawContours(SimpImg, SimpContours, 0, conColour, 1, 8);
        drawContours(SimpImg, vector<vector<Point>> {hull}, 0, hullColour, 1, 8);
		circle(SimpImg, centre, (int)radius, circleColour, 1, 8);
	}

	//Iterate through convext hull points to find three corners of QR code
	vector<pair<int, Point>> dist;
	for (unsigned int i = 0; i < hull.size(); i++)
	{
        int xH = hull[i].x;
		int yH = hull[i].y;
		int xC = centre.x;
		int yC = centre.y;
		dist.push_back(make_pair(SQ(xH - xC) + SQ(yH - yC), hull[i]));
	}
    sort(dist.begin(), dist.end(), sortPointPair());
	vector<pair<int, Point>> corners = { dist[0] };		// Contour point closest to radial circle
    for (unsigned int i = 1; i < hull.size(); i++)
	{
        if (dist[i].first < SQ(0.8*radius)) {			// If contour point is too far from circle radius, not corner
			continue;
		}
		else {
			bool too_close = false;
			for (vector<pair<int, Point>>::iterator it = corners.begin(); it < corners.end(); it++) {
				double separation = (double)(SQ(dist[i].second.x - (*it).second.x))
					+ (double)(SQ(dist[i].second.y - (*it).second.y));
				if (separation < SQ(0.66*radius)) {		// If contour point is too close to previously found corner, ignore
					too_close = true;
					break;
				}
			}
			if (!too_close)
				corners.push_back(dist[i]);				// Add contour point to vector of corners
		}
	}

	if (drawDebug)
	{
		for (vector<pair<int, Point>>::iterator it = corners.begin(); it < corners.end(); it++) {
			Scalar cornerColour = Scalar(255, 255, 255);
			circle(SimpImg, (*it).second, 4, cornerColour, -1, 8);
		}        
        char const * debug_window = "Debug Image";
        imshow(debug_window,SimpImg);
        waitKey(0);
    }

	// We check we have found at least three corners in image, and then we assign them 
	// labels according to their position wrt centre of QR code (tl, tr, bl, br)
	bool tl = false, tr = false, bl = false, br = false;
    bool _2fewCnrs = false;
    Point pnt;
	for (int i = 0; i < 3; i++) {
		try {
			pnt = corners.at(i).second;
			if (pnt.x < centre.x) {
				if (pnt.y < centre.y) {
					QRcorner.tl = pnt;
					tl = true;
				}
				else {
					QRcorner.bl = pnt;
					bl = true;
				}
			}
			else {
				if (pnt.y < centre.y) {
					QRcorner.tr = pnt;
					tr = true;
				}
				else {
					QRcorner.br = pnt;
					br = true;
				}
			}
		}
        catch (...){
            _2fewCnrs = true;
        }
	}

	// Check if we have found at least three points, and find location of the fourth
    if (!_2fewCnrs)
	{
		if (!tl) {
			int x, y;
            x = centre.x - abs(QRcorner.br.x - centre.x);
            y = centre.y - abs(QRcorner.br.y - centre.y);
			QRcorner.tl = Point(x, y);
		}
		else if (!tr) {
			int x, y;
            x = centre.x + abs(QRcorner.bl.x - centre.x);
            y = centre.y - abs(QRcorner.bl.y - centre.y);
			QRcorner.tr = Point(x, y);
		}
		else if (!bl) {
			int x, y;
            x = centre.x - abs(QRcorner.tr.x - centre.x);
            y = centre.y + abs(QRcorner.tr.y - centre.y);
			QRcorner.bl = Point(x, y);
		}
		else if (!br) {
			int x, y;
            x = centre.x + abs(QRcorner.tl.x - centre.x);
            y = centre.y + abs(QRcorner.tl.y - centre.y);
			QRcorner.br = Point(x, y);
		}
        return true;
    }
	else
		return false;

    /*// Find the centroid of the boarder and QR code to determine the location of the
    // QR code
    Mat gray;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    GaussianBlur(resultBGR,resultBGR_blur,Size(3,3),0,0);
    cvtColor(resultBGR_blur,gray,COLOR_BGR2GRAY);
    inRange(gray,20,255,gray);
    Mat element = getStructuringElement(0, Size(3,3), Point(-1,-1));
    //erode(gray,gray,element);
    //imshow("inRange",gray);
    //waitKey(0);
    findContours(gray,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(0,0));
    Moments muiPad, muQR;
    bool iPadcontour = false, QRcontour = false;
    for (unsigned int i = 0; i < hierarchy.size(); i++){
        if ((hierarchy[i][2] > -1) && (hierarchy[i][3] < 0)){
            muiPad = moments(contours[i],false);
            iPadcontour = true;
        }else if ((hierarchy[i][3] > -1) && (hierarchy[i][2] < 0)){
            muQR = moments(contours[i],false);
            QRcontour = true;
        }
    }
    if (!iPadcontour || !QRcontour)
        return nullptr;

    Point2f QRcentroid = Point2f(muQR.m10/muQR.m00, muQR.m01/muQR.m00);
    Point2f iPadcentroid = Point2f(muiPad.m10/muiPad.m00, muiPad.m01/muiPad.m00);;

    Scalar white(255,255,255);
    circle(gray,QRcentroid,2,white,-1);
    circle(gray,iPadcentroid,2,white,-1);
    imshow("Centroid",gray);
    waitKey(0);

    // Find corners of image
    QRcorner = new cnrs;
    Mat dst, dst_norm, dst_norm_scaled;
    cornerHarris(gray, dst, 6, 3, 0.04);
    normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    convertScaleAbs( dst_norm, dst_norm_scaled );
    inRange(dst_norm_scaled,100, 255, dst_norm_scaled);
    dilate(dst_norm_scaled,dst_norm_scaled,element);
    //imshow("Edge detection", dst_norm_scaled);
    //waitKey(0);
    int k = 50;
    kLargestHeap fps(dst_norm_scaled,k);
    for (int i = 0; i < dst_norm_scaled.cols*dst_norm_scaled.rows; i++){
        fps.swapRoot(dst_norm_scaled,i);
    }

    for (int i = 0; i < k; i++){

    }
    vector<vector<Point>> Contours;
    vector<Vec4i> Hierarchy;
    findContours(dst_norm_scaled,Contours,Hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(0,0));
    Mat dstBGR;//(dst_norm_scaled.size(),CV_8UC3);
    cvtColor(dst_norm_scaled,dstBGR,COLOR_GRAY2BGR);
    vector<Moments> mu(Contours.size());
    for (unsigned int i = 0; i < Contours.size(); i++){
        mu[i] = moments(Contours[i],false);
        drawContours(dstBGR,Contours,i,Scalar(0,0,255),2,8);
    }

    vector<Point2f> mc(Contours.size());
    for (unsigned int i = 0; i < Contours.size(); i++){
        mc[i] = Point2f(mu[i].m10/mu[i].m00, mu[i].m01/mu[i].m00);
    }

    //imshow("Corners",dstBGR);
    //waitKey(0);

    // Only take corners corresponding to rQR code (closest to QR centroid)
    vector<pair<double,Point>> pixels;
    for (int i = 0; i < Contours.size(); i++){
        double dist = SQ(mc[i].x - QRcentroid.x) + SQ(mc[i].y - QRcentroid.y);
        pixels.push_back(make_pair(dist,Point(mc[i].x,mc[i].y)));
    }
    sort(pixels.begin(), pixels.end(),comparePix());
    int crnrsFound[4] = {0};
    int j = 0;
    while (((crnrsFound[0] == 0) || (crnrsFound[1] == 0) || (crnrsFound[2] == 0)
           || (crnrsFound[3] == 0)) && j<8 ){
        Point pix = Point(pixels[j].second.x,pixels[j].second.y);
        if (pix.x < QRcentroid.x){
            if ((pix.y < QRcentroid.y) && (crnrsFound[0] == 0)){
                QRcorner->tl = pix;
                crnrsFound[0] = 1;
            }else if ((pix.y > QRcentroid.y) && (crnrsFound[1] == 0)){
                QRcorner->bl = pix;
                crnrsFound[1] = 1;
            }
        }else{
            if ((pix.y < QRcentroid.y) && (crnrsFound[2] == 0) ){
                QRcorner->tr = pix;
                crnrsFound[2] = 1;
            }else if ((pix.y > QRcentroid.y) && (crnrsFound[3] == 0)){
                QRcorner->br = pix;
                crnrsFound[3] = 1;
            }
        }
        j++;
    }
    //cnrs iPadcorner;
    //QRcentroid = intersection(QRcorner);
    //iPadcentroid = intersection(iPadcorner);

    if (QRcentroid.x < iPadcentroid.x){
        if (QRcentroid.y < iPadcentroid.y){
            //botl.code = img;
            topl.corners = *QRcorner;
            topl.loctn = "topl";
            return &topl;
        }else{
            //topl.code = img;
            botl.corners = *QRcorner;
            botl.loctn = "botl";
            return &botl;
        }
    }else{
        if (QRcentroid.y < iPadcentroid.y){
            //botr.code = img;
            topr.corners = *QRcorner;
            topr.loctn = "topr";
            return &topr;
        }else{
            //topr.code = img;
            botr.corners = *QRcorner;
            botr.loctn = "botr";
            return &botr;
        }
    }*/
}

bool stitchImg::stitch(int perm[4])
{
    //if((!topl.code.empty()) && (!topr.code.empty()) &&
    //        (!botl.code.empty()) && (!botr.code.empty())){
    if(codeElements.size() == 4){
        Rect toplROI(0,0,200,200), toprROI(200,0,200,200), botlROI(0,200,200,200), botrROI(200,200,200,200);
        (codeElements.at(perm[0]).code).copyTo(QRcode(toplROI));
        (codeElements.at(perm[1]).code).copyTo(QRcode(toprROI));
        (codeElements.at(perm[2]).code).copyTo(QRcode(botlROI));
        (codeElements.at(perm[3]).code).copyTo(QRcode(botrROI));
        return true;
    }else{
        return false;
    }
}

bool stitchImg::stitch()
{
    //if((!topl.code.empty()) && (!topr.code.empty()) &&
    //        (!botl.code.empty()) && (!botr.code.empty())){
    int perm[4] = {0,1,2,3};
    if(codeElements.size() == 4){
        Rect toplROI(0,0,200,200), toprROI(200,0,200,200), botlROI(0,200,200,200), botrROI(200,200,200,200);
        (codeElements.at(perm[0]).code).copyTo(QRcode(toplROI));
        (codeElements.at(perm[1]).code).copyTo(QRcode(toprROI));
        (codeElements.at(perm[2]).code).copyTo(QRcode(botlROI));
        (codeElements.at(perm[3]).code).copyTo(QRcode(botrROI));
        return true;
    }else{
        return false;
    }
}

bool stitchImg::QRcodeRead()
{
    int permutation[] = {0,1,2,3};
    bool stitchSuccess = stitch(permutation);

    if (stitchSuccess){
        ImageScanner scanner;
        scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

        Mat QRcomplete = QRcode;
        Image Code(QRcomplete.cols,QRcomplete.rows,"Y800", (uchar *)QRcomplete.data, QRcomplete.cols*QRcomplete.rows);
        for (int i = 0; i < 4; i++){
            bool foundValid = false;
            int n = scanner.scan(Code);
            if(n){foundValid=true;}
            while (!n && (next_permutation(permutation,permutation+4))){
                stitch(permutation);
                QRcomplete = QRcode;
                //cvtColor(QRcomplete,QRgray, CV_BGR2GRAY);
                Code.set_data((uchar *)QRcomplete.data, QRcomplete.cols*QRcomplete.rows);

                n = scanner.scan(Code);
                if(n)
                    foundValid = true;
            }

            if (foundValid)
            {
                for (Image::SymbolIterator symbol = Code.symbol_begin(); symbol != Code.symbol_end(); ++symbol){
                    obj.type = symbol->get_type_name();
                    obj.data = symbol->get_data();
                }
                return true;
            }
            for(unsigned int j = 0; j < codeElements.size(); j ++){
                int angle = i*90;
                rotate_90n(codeElements.at(j).code,codeElements.at(j).code,angle);
            }
        }
    }
    return false;
}

\
void rotate_90n(Mat const &src, Mat &dst, int angle)
{
    if(angle == 270 || angle == -90){
        transpose(src,dst);
        flip(dst,dst,0);
    }else if (angle == 180 || angle == -180){
        flip(src,dst,-1);
    }else if (angle == 90 || angle == -270){
        transpose(src,dst);
        flip(dst,dst,1);
    }else{
        if(src.data != dst.data)
            src.copyTo(dst);
    }
}


Point2f intersection(cnrs corner)
{
    // Line connecting tl and br corners
    double a1 = corner.tl.y - corner.br.y;
    double b1 = corner.br.x - corner.tl.x;
    double c1 = a1*(corner.br.x) + b1*(corner.br.y);

    // Line connecting bl and tr corners
    double a2 = corner.tr.y - corner.bl.y;
    double b2 = corner.bl.x - corner.tr.x;
    double c2 = a2*(corner.bl.x) + b2*(corner.bl.y);

    double determinant = a1*b2 - a2*b1;

    if (determinant == 0)
    {
        // The lines are parallel
        return Point2f(FLT_MAX, FLT_MAX);
    }else
    {
        double x = (b2*c1 - b1*c2)/determinant;
        double y = (a1*c2 - a2*c1)/determinant;
        return Point2f(x,y);
    }

}

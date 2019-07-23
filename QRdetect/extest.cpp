#include <iostream>
#include <exception>
#include "stitchimg.h"
#include <zbar.h>

using namespace std;
using namespace zbar;

int main()
{
    // Create the image stitching class, which takes in images, and saves them if seen
    stitchImg imgStitcher(true);
    bool fullImg;
    char const* source_window = "Constructed QR code";
    namedWindow(source_window, WINDOW_AUTOSIZE);

    Mat src1 = imread("../QRdetect/QR_photos/tl.PNG");
    Mat src4 = imread("../QRdetect/QR_photos/tr.PNG");
    Mat src3 = imread("../QRdetect/QR_photos/bl.PNG");
    Mat src2 = imread("../QRdetect/QR_photos/br.PNG");
    cout << src1.depth() << " " << src1.channels() << endl;

    // Assume src1 is seen first, so process and save
    imgStitcher.undistortImg(src1);
    // Try to stitch images (should fail, as only one image has been seen)
    fullImg = imgStitcher.QRcodeRead();
    imshow(source_window,*imgStitcher.getQRcode());
    cout << fullImg << endl;

    //waitKey(0);

    // Assume src2 is seen second, so process and save
    imgStitcher.undistortImg(src2);
    // Try to stitch images (should fail, as only two image have been seen)
    fullImg = imgStitcher.QRcodeRead();
    imshow(source_window,*imgStitcher.getQRcode());
    cout << fullImg << endl;

    //waitKey(0);

    // Assume src3 is seen third, so process and save
    imgStitcher.undistortImg(src3);
    // Try to stitch images (should fail, as only three images have been seen)
    fullImg = imgStitcher.QRcodeRead();
    imshow(source_window,*imgStitcher.getQRcode());
    cout << fullImg << endl;

    //waitKey(0);

    // Assume src4 is seen last, so process and save
    imgStitcher.undistortImg(src4);
    // Try to stitch images (should succeed, all images have been seen)
    fullImg = imgStitcher.QRcodeRead();
    imshow(source_window,*imgStitcher.getQRcode());
    cout << fullImg << endl;

    waitKey(0);

    decodedObject *obj = imgStitcher.getData();
    cout << obj->type << ":\t" << obj->data << endl;

    /*if (fullImg){
        ImageScanner scanner;
        scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
        Mat QRcomplete = *imgStitcher.getQRcode();
        Mat QRgray;
        cvtColor(QRcomplete,QRgray, CV_BGR2GRAY);
        Image Code(QRcomplete.cols,QRcomplete.rows,"Y800", (uchar *)QRgray.data, QRcomplete.cols*QRcomplete.rows);
        int n = scanner.scan(Code);

        for (Image::SymbolIterator symbol = Code.symbol_begin(); symbol != Code.symbol_end(); ++symbol){
            decodedObject obj;
            obj.type = symbol->get_type_name();
            obj.data = symbol->get_data();
            cout << "Type : " << obj.type << endl;
            cout << "Data : " << obj.data << "\n" << endl;
        }
    }*/
    return 0;
}

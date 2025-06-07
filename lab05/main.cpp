#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

Mat binarizeImage(const Mat& img, int threshold) {
    Mat binaryImg = Mat::zeros(img.size(), CV_8UC1);
    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            int max_ = max(img.at<Vec3b>(i, j)[0], max(img.at<Vec3b>(i, j)[1], img.at<Vec3b>(i, j)[2]));
            int mean = (img.at<Vec3b>(i, j)[0] + img.at<Vec3b>(i, j)[1] + img.at<Vec3b>(i, j)[2]) / 3;
            int value = (max_ + mean)/ 2;
            binaryImg.at<uchar>(i, j) = (value < threshold) ? 0 : 255;
        }
    }
    return binaryImg;
}

int main() {
    VideoCapture vid("../img/vid01.mp4");
    if (!vid.isOpened()) {
        std::cerr << "Error loading video" << std::endl;
        return -1;
    }

    Mat frame, binFrame;
    Rect bounding
    vector<Point> trajectoryPoints;
    int numFrames = 0;
    namedWindow("Processed Video", WINDOW_AUTOSIZE);
    while (true) {
        vid.read(frame);
        if (frame.empty()) {
            break;
        }
        numFrames++;
        binFrame = binarizeImage(frame, 90);

        for (int i = 0; i < binFrame.rows; i++) {
            for (int j = 0; j < binFrame.cols; j++) {
                if (binFrame.at<uchar>(i, j) == 0) {
                    floodFill(binFrame, Point(j, i), Scalar(128));
                }
            }
        }
        imshow("Processed Video", binFrame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    cout << "Number of frames: " << numFrames << endl;

    return 0;
}
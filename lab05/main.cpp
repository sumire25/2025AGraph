#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

void binarizeImage(const Mat& img, Mat& binaryImg, int threshold) {
    binaryImg = Mat::zeros(img.size(), CV_8UC1);
    int max_, mean, value;
    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            max_ = max(img.at<Vec3b>(i, j)[0], max(img.at<Vec3b>(i, j)[1], img.at<Vec3b>(i, j)[2]));
            mean = (img.at<Vec3b>(i, j)[0] + img.at<Vec3b>(i, j)[1] + img.at<Vec3b>(i, j)[2]) / 3;
            value = (max_ + mean)/ 2;
            binaryImg.at<uchar>(i, j) = (value < threshold) ? 0 : 255;
        }
    }
}

void difference(const Mat& a, const Mat& b, Mat& dif) {
    dif = Mat::zeros(a.size(), CV_8UC3);
    for(int i = 0; i < a.rows; i++) {
        for(int j = 0; j < a.cols; j++) {
            dif.at<Vec3b>(i, j)[0] = abs(a.at<Vec3b>(i, j)[0] - b.at<Vec3b>(i, j)[0]);
            dif.at<Vec3b>(i, j)[1] = abs(a.at<Vec3b>(i, j)[1] - b.at<Vec3b>(i, j)[1]);
            dif.at<Vec3b>(i, j)[2] = abs(a.at<Vec3b>(i, j)[2] - b.at<Vec3b>(i, j)[2]);
        }
    }
}

int main() {
    VideoCapture vid(0);
    if (!vid.isOpened()) {
        std::cerr << "Error loading video" << std::endl;
        return -1;
    }

    Mat frame, firstFrame, dif, binFrame, grayframe;
    vector<Point> trajectoryPoints;
    int numFrames = 0;
    namedWindow("Processed Video", WINDOW_AUTOSIZE);
    while (true) {
        vid.read(frame);
        if(numFrames == 0)
            firstFrame = frame.clone();
        if (frame.empty())
            break;
        numFrames++;

        difference(frame, firstFrame, dif);
        grayframe = Mat::zeros(dif.size(), CV_8UC1);
        cvtColor(dif, grayframe, COLOR_BGR2GRAY);
        //trhesholding dif image which has 3 channels, using thresholding methods from OpenCV
        //adaptiveThreshold(grayframe, binFrame, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, 2);
        threshold(grayframe, binFrame, THRESH_OTSU, 255, THRESH_BINARY);
        imshow("Processed Video", binFrame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    cout << "Number of frames: " << numFrames << endl;
    vid.release();
    destroyAllWindows();

    return 0;
}
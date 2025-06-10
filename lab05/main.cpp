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

void erode(const Mat& img, Mat& erodedImg, vector<pair<int,int>> kernel) {
    erodedImg = Mat::zeros(img.size(), CV_8UC1);
    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            bool fit = true;
            for(const auto& p : kernel) {
                int i_ = i + p.first;
                int j_ = j + p.second;
                if(i_ < 0 || i_ >= img.rows || j_ < 0 || j_ >= img.cols || img.at<uchar>(i_, j_) == 0) {
                    fit = false;
                    break;
                }
            }
            erodedImg.at<uchar>(i, j) = fit ? 255 : 0;
        }
    }
}

void dilate(const Mat& img, Mat& dilatedImg, vector<pair<int,int>> kernel) {
    dilatedImg = Mat::zeros(img.size(), CV_8UC1);
    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            bool hit = false;
            for(const auto& p : kernel) {
                int i_ = i + p.first;
                int j_ = j + p.second;
                if(i_ >= 0 && i_ < img.rows && j_ >= 0 && j_ < img.cols && img.at<uchar>(i_, j_) == 255) {
                    hit = true;
                    break;
                }
            }
            dilatedImg.at<uchar>(i, j) = hit ? 255 : 0;
        }
    }
}

void aperture(const Mat& img, Mat& aperturedImg, vector<pair<int,int>> kernel) {
    Mat erodedImg;
    erode(img, erodedImg, kernel);
    dilate(erodedImg, aperturedImg, kernel);
}

int main() {
    //VideoCapture vid("../img/vid01.mp4");
    VideoCapture vid(0);
    if (!vid.isOpened()) {
        std::cerr << "Error loading video" << std::endl;
        return -1;
    }

    Mat frame, lastFrame, difFrame, binFrame, aperturedFrame, outputFrame;
    vector<Point> trajectoryPoints;
    int numFrames = 0;

    vid.read(frame);
    while (true) {
        lastFrame = frame.clone();
        vid.read(frame);
        if (frame.empty())
            break;
        numFrames++;

        difference(frame, lastFrame, difFrame);
        binarizeImage(difFrame, binFrame, 24);
        //threshold(grayframe, binFrame, 32, 255, THRESH_BINARY);
        vector<pair<int,int>> kernel = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}, {0, 0}};
        aperture(binFrame, aperturedFrame, kernel);

        bool foundMovement = false;
        int minX, minY, maxX, maxY;
        minX = minY = INT_MAX;
        maxX = maxY = INT_MIN;
        for(int i=0; i < aperturedFrame.rows; i++) {
            for(int j=0; j < aperturedFrame.cols; j++) {
                if(aperturedFrame.at<uchar>(i, j) == 255) {
                    foundMovement = true;
                    minX = min(minX, j);
                    minY = min(minY, i);
                    maxX = max(maxX, j);
                    maxY = max(maxY, i);
                }
            }
        }
        if(foundMovement) {
            trajectoryPoints.push_back(Point((minX + maxX) / 2, (minY + maxY) / 2));
            //draw two lines from the center of the bounding box to the corners
            line(aperturedFrame, Point(minX, minY), Point(minX, maxY), Scalar(255), 1);
            line(aperturedFrame, Point(minX, minY), Point(maxX, minY), Scalar(255), 1);
            line(aperturedFrame, Point(maxX, minY), Point(maxX, maxY), Scalar(255), 1);
            line(aperturedFrame, Point(minX, maxY), Point(maxX, maxY), Scalar(255), 1);
            line(aperturedFrame, Point(minX, (minY+maxY)/2), Point(maxX, (minY+maxY)/2), Scalar(255), 1);
            line(aperturedFrame, Point((minX+maxX)/2, minY), Point((minX+maxX)/2, maxY), Scalar(255), 1);
            if(trajectoryPoints.size() > 10)
                trajectoryPoints.erase(trajectoryPoints.begin());
        }
        else if(trajectoryPoints.size() > 0)
            trajectoryPoints.clear();

        outputFrame = frame.clone();
        for (size_t i = 1; i < trajectoryPoints.size(); i++) {
            line(outputFrame, trajectoryPoints[i - 1], trajectoryPoints[i], Scalar(0, 255, 0), 2);
            circle(outputFrame, trajectoryPoints[i], 3, Scalar(0, 255, 0), 2);
        }

        imshow("Difference Frame", difFrame);
        imshow("Binarized Frame", binFrame);
        imshow("Apertured Frame", aperturedFrame);
        imshow("Output Video", outputFrame);
        if (waitKey(30) >= 0) {
            break;
        }
    }

    cout << "Number of frames: " << numFrames << endl;
    vid.release();
    destroyAllWindows();

    return 0;
}
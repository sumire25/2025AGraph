#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

vector<int> getHistogram(const Mat& img) {
    vector<int> hist(256, 0);
    for(int i=0; i<img.rows; i++) {
        for(int j=0; j<img.cols; j++) {
            hist[img.at<uchar>(i, j)]++;
        }
    }
    return hist;
}

void saveHistogram(vector<int> hist, int n) {
    string filename = "histogram" + to_string(n) + ".csv";
    ofstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < 256; i++) {
            file << i << "," << hist[i] << endl;
        }
        file.close();
    } else {
        cout << "Unable to open file";
    }
}

vector<int> equalizeHistogram(vector<int> hist, int nPixels) {
    vector<int> f(256);
    f[0] = 0;
    int cumulativeAbsFreq = hist[0];
    for (int i = 1; i < 255; i++) {
        f[i] = (cumulativeAbsFreq * 255) / nPixels;
        cumulativeAbsFreq += hist[i];
    }
    f[255] = 255;
    return f;
}

void equalizeImage(Mat& img) {
    vector<int> hist = getHistogram(img);
    int nPixels = img.rows * img.cols;
    vector<int> f = equalizeHistogram(hist, nPixels);

    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            img.at<uchar>(i, j) = f[img.at<uchar>(i, j)];
        }
    }
}

Mat binarizeImage(const Mat& img, int threshold) {
    Mat binaryImg = Mat::zeros(img.size(), CV_8UC1);
    for(int i = 0; i < img.rows; i++) {
        for(int j = 0; j < img.cols; j++) {
            binaryImg.at<uchar>(i, j) = (img.at<uchar>(i, j) < threshold) ? 0 : 255;
        }
    }
    return binaryImg;
}

int main() {
    vector<int> thresholds = {255, 255, 125, 125, 125, 120};
    vector<Mat> images(6);
    vector<Mat> binImages(6);

    for (int i = 0; i < 6; i++) {
        images[i] = imread("../img/imagen_" + to_string(i) + ".png", IMREAD_GRAYSCALE);
        equalizeImage(images[i]);
        saveHistogram(getHistogram(images[i]), i);
        imwrite("eqImage" + to_string(i) + ".png", images[i]);

        binImages[i] = binarizeImage(images[i], thresholds[i]);
        imwrite("binImage" + to_string(i) + ".png", binImages[i]);
    }

    return 0;
}
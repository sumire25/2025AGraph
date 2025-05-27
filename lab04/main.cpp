#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

//create histogram of the image
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

int main() {
    Mat image0 = imread("../img/imagen_0.png", IMREAD_GRAYSCALE);
    Mat image1 = imread("../img/imagen_1.png", IMREAD_GRAYSCALE);
    Mat image2 = imread("../img/imagen_2.png", IMREAD_GRAYSCALE);
    Mat image3 = imread("../img/imagen_3.png", IMREAD_GRAYSCALE);
    Mat image4 = imread("../img/imagen_4.png", IMREAD_GRAYSCALE);
    Mat image5 = imread("../img/imagen_5.png", IMREAD_GRAYSCALE);

    equalizeImage(image0);
    equalizeImage(image1);
    equalizeImage(image2);
    equalizeImage(image3);
    equalizeImage(image4);
    equalizeImage(image5);

    // Save histograms
    saveHistogram(getHistogram(image0), 0);
    saveHistogram(getHistogram(image1), 1);
    saveHistogram(getHistogram(image2), 2);
    saveHistogram(getHistogram(image3), 3);
    saveHistogram(getHistogram(image4), 4);
    saveHistogram(getHistogram(image5), 5);

    //show
    imshow("Image 0", image0);
    imshow("Image 1", image1);
    imshow("Image 2", image2);
    imshow("Image 3", image3);
    imshow("Image 4", image4);
    imshow("Image 5", image5);

    //press
    waitKey(0);

    return 0;
}
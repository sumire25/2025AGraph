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

int main() {
    Mat image0 = imread("C:/Marko/computerGraph/lab01/open/imagen_0.png", IMREAD_GRAYSCALE);
    Mat image1 = imread("C:/Marko/computerGraph/lab01/open/imagen_1.png", IMREAD_GRAYSCALE);
    Mat image2 = imread("C:/Marko/computerGraph/lab01/open/imagen_2.png", IMREAD_GRAYSCALE);
    Mat image3 = imread("C:/Marko/computerGraph/lab01/open/imagen_3.png", IMREAD_GRAYSCALE);
    Mat image4 = imread("C:/Marko/computerGraph/lab01/open/imagen_4.png", IMREAD_GRAYSCALE);
    Mat image5 = imread("C:/Marko/computerGraph/lab01/open/imagen_5.png", IMREAD_GRAYSCALE);

    vector<int> hist0 = getHistogram(image0);
    vector<int> hist1 = getHistogram(image1);
    vector<int> hist2 = getHistogram(image2);
    vector<int> hist3 = getHistogram(image3);
    vector<int> hist4 = getHistogram(image4);
    vector<int> hist5 = getHistogram(image5);

    saveHistogram(hist0, 0);
    saveHistogram(hist1, 1);
    saveHistogram(hist2, 2);
    saveHistogram(hist3, 3);
    saveHistogram(hist4, 4);
    saveHistogram(hist5, 5);


    return 0;
}
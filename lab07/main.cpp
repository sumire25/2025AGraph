#include <tiffio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main() {
    string tiffFile = "../img/eyeMasks.tiff";

    // Open the TIFF file
    TIFF* tiff = TIFFOpen(tiffFile.c_str(), "r");
    if (!tiff) {
        cerr << "Error: Could not open the TIFF file." << endl;
        return -1;
    }

    vector<Mat> images;
    do {
        uint32 width, height;
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

        // Print the dimensions of the current image
        cout << "Image dimensions: Width = " << width << ", Height = " << height << endl;

        // Allocate memory for the image (RGBA format)
        Mat img(height, width, CV_8UC4); // RGBA format
        if (!TIFFReadRGBAImage(tiff, width, height, (uint32*)img.data, 0)) {
            cerr << "Error: Could not read image data." << endl;
            TIFFClose(tiff);
            return -1;
        }

        // Flip the image vertically (TIFF stores images upside-down)
        cv::flip(img, img, 0);

        images.push_back(img.clone());
    } while (TIFFReadDirectory(tiff)); // Move to the next page

    TIFFClose(tiff);

    if (images.empty()) {
        cerr << "Error: No images found in the TIFF file." << endl;
        return -1;
    }

    // Display total count of images
    cout << "Total images in the TIFF file: " << images.size() << endl;

    // Display images with space to move to the next and ESC to exit
    namedWindow("Stomach Masks", WINDOW_AUTOSIZE);
    int currentImageIndex = 0;
    while (true) {
        imshow("Stomach Masks", images[currentImageIndex]);
        int key = waitKey(0); // Wait indefinitely for a key press
        if (key == 27) { // ESC key
            break; // Exit the loop
        } else if (key == 32) { // Space key
            currentImageIndex = (currentImageIndex + 1) % images.size(); // Move to the next image
        }
    }

    return 0;
}
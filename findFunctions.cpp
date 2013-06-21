#include "findFunctions.h"

// Aldebaran includes.
#include <alproxies/alvideodeviceproxy.h>
#include <alvision/alimage.h>
#include <alvision/alvisiondefinitions.h>
#include <alerror/alerror.h>

// Opencv includes.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Eye Recognition includes
#include "constants.h"
#include "findEyeCenter.h"
#include "findEyeCorner.h"

/** Global variables */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
cv::String face_cascade_name = "../res/haarcascade_frontalface_alt.xml";
cv::CascadeClassifier face_cascade;
std::string main_window_name = "Capture - Face detection";
std::string face_window_name = "Capture - Face";
cv::RNG rng(12345);
cv::Mat debugImage;
cv::Mat skinCrCbHist = cv::Mat::zeros(cv::Size(256, 256), CV_8UC1);

using namespace AL;

void findEyes(cv::Mat frame_gray, cv::Rect face) {
    cv::Mat faceROI = frame_gray(face);

    if (kSmoothFaceImage) {
        double sigma = kSmoothFaceFactor * face.width;
        GaussianBlur( faceROI, faceROI, cv::Size( 0, 0 ), sigma);
    }
    //-- Find eye regions and draw them
    int eye_region_width = face.width * (kEyePercentWidth/100.0);
    int eye_region_height = face.width * (kEyePercentHeight/100.0);
    int eye_region_top = face.height * (kEyePercentTop/100.0);
    cv::Rect leftEyeRegion(face.width*(kEyePercentSide/100.0),
                           eye_region_top,eye_region_width,eye_region_height);
    cv::Rect rightEyeRegion(face.width - eye_region_width - face.width*(kEyePercentSide/100.0),
                            eye_region_top,eye_region_width,eye_region_height);

    //-- Find Eye Centers
    cv::Point leftPupil = findEyeCenter(faceROI,leftEyeRegion,"Left Eye");
    cv::Point rightPupil = findEyeCenter(faceROI,rightEyeRegion,"Right Eye");
    // get corner regions
    cv::Rect leftRightCornerRegion(leftEyeRegion);
    leftRightCornerRegion.width -= leftPupil.x;
    leftRightCornerRegion.x += leftPupil.x;
    leftRightCornerRegion.height /= 2;
    leftRightCornerRegion.y += leftRightCornerRegion.height / 2;
    cv::Rect leftLeftCornerRegion(leftEyeRegion);
    leftLeftCornerRegion.width = leftPupil.x;
    leftLeftCornerRegion.height /= 2;
    leftLeftCornerRegion.y += leftLeftCornerRegion.height / 2;
    cv::Rect rightLeftCornerRegion(rightEyeRegion);
    rightLeftCornerRegion.width = rightPupil.x;
    rightLeftCornerRegion.height /= 2;
    rightLeftCornerRegion.y += rightLeftCornerRegion.height / 2;
    cv::Rect rightRightCornerRegion(rightEyeRegion);
    rightRightCornerRegion.width -= rightPupil.x;
    rightRightCornerRegion.x += rightPupil.x;
    rightRightCornerRegion.height /= 2;
    rightRightCornerRegion.y += rightRightCornerRegion.height / 2;
    rectangle(faceROI,leftRightCornerRegion,200);
    rectangle(faceROI,leftLeftCornerRegion,200);
    rectangle(faceROI,rightLeftCornerRegion,200);
    rectangle(faceROI,rightRightCornerRegion,200);
    // change eye centers to face coordinates
    rightPupil.x += rightEyeRegion.x;
    rightPupil.y += rightEyeRegion.y;
    leftPupil.x += leftEyeRegion.x;
    leftPupil.y += leftEyeRegion.y;
    // draw eye centers
    circle(faceROI, rightPupil, 3, 1234);
    circle(faceROI, leftPupil, 3, 1234);

    //-- Find Eye Corners
    if (kEnableEyeCorner) {
        cv::Point2f leftRightCorner = findEyeCorner(faceROI(leftRightCornerRegion), true, false);
        leftRightCorner.x += leftRightCornerRegion.x;
        leftRightCorner.y += leftRightCornerRegion.y;
        cv::Point2f leftLeftCorner = findEyeCorner(faceROI(leftLeftCornerRegion), true, true);
        leftLeftCorner.x += leftLeftCornerRegion.x;
        leftLeftCorner.y += leftLeftCornerRegion.y;
        cv::Point2f rightLeftCorner = findEyeCorner(faceROI(rightLeftCornerRegion), false, true);
        rightLeftCorner.x += rightLeftCornerRegion.x;
        rightLeftCorner.y += rightLeftCornerRegion.y;
        cv::Point2f rightRightCorner = findEyeCorner(faceROI(rightRightCornerRegion), false, false);
        rightRightCorner.x += rightRightCornerRegion.x;
        rightRightCorner.y += rightRightCornerRegion.y;
        circle(faceROI, leftRightCorner, 3, 200);
        circle(faceROI, leftLeftCorner, 3, 200);
        circle(faceROI, rightLeftCorner, 3, 200);
        circle(faceROI, rightRightCorner, 3, 200);
    }

    imshow(face_window_name, faceROI);
}


cv::Mat findSkin (cv::Mat &frame) {
    cv::Mat input;
    cv::Mat output = cv::Mat(frame.rows,frame.cols, CV_8U);

    cvtColor(frame, input, CV_BGR2YCrCb);

    for (int y = 0; y < input.rows; ++y) {
        const cv::Vec3b *Mr = input.ptr<cv::Vec3b>(y);
        cv::Vec3b *Or = frame.ptr<cv::Vec3b>(y);
        for (int x = 0; x < input.cols; ++x) {
            cv::Vec3b ycrcb = Mr[x];
            if(skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) == 0) {
                Or[x] = cv::Vec3b(0,0,0);
            }
        }
    }
    return output;
}


int showImages(const std::string& robotIp)
{
    printf("Press 'q' key to stop\n");
    printf("Press 'f' key to take a frame of the main window\n");

    /** Create a proxy to ALVideoDevice on the robot.*/
    ALVideoDeviceProxy camProxy(robotIp, 9559);

    /** Subscribe a client image requiring 320*240 and BGR colorspace.
     * You can change the resolution of the required image according
     * to the Aldebaran's documentation
     *
     * http://www.aldebaran-robotics.com/documentation/naoqi/vision/alvideodevice-api.html#resolution
     *
     * Parameter ID Name | ID Value | Description
     * AL::kQQVGA        |    0     | Image of 160*120px
     * AL::kQVGA         |    1     | Image of 320*240px
     * AL::kVGA          |    2     | Image of 640*480px
     * AL::k4VGA         |    3     | Image of 1280*960px
     *
     * Note
     * AL::k4VGA is only available on NAO V4 (camera HD MT9M114).
     */

    int cameraRes = kVGA;
    const std::string clientName = camProxy.subscribe("test", cameraRes, kBGRColorSpace, 30);

    /** Create an cv::Mat header to wrap into an opencv image.*/
    int xRes, yRes;
    switch (cameraRes) {
    case kQQVGA:
        xRes = 160;
        yRes = 120;
        break;
    case kQVGA:
        xRes = 320;
        yRes = 240;
        break;
    case kVGA:
        xRes = 640;
        yRes = 480;
        break;
    case k4VGA:
        xRes = 1280;
        yRes = 960;
        break;
    default:
        printf("--(!)Error: Invalid camera resolutionl");
        return -1;
    }

    // Images
    cv::Mat frame = cv::Mat(cv::Size(xRes, yRes), CV_8UC3);
    cv::Mat frame_gray;

    // Video Writer
    /** Uncomment to save videos. Bellow you will follow another block to uncomment **/
//    cv::Size size2 = cv::Size(640,480);
//    int codec = CV_FOURCC('M', 'J', 'P', 'G');
//    cv::VideoWriter writer1("video_color.avi",codec,15.0,size2,true);
//    cv::VideoWriter writer2("video_grayscale.avi",codec,15.0,size2,false);
//    writer1.open("video_color.avi",codec,10.0,size2,true);
//    writer2.open("video_grayscale.avi",codec,10.0,size2,false);

    // Load the cascades
    if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

    // Windows for display the different images
    cv::namedWindow(main_window_name,CV_WINDOW_NORMAL);
    cv::moveWindow(main_window_name, 50, 100);
    cv::namedWindow(face_window_name,CV_WINDOW_NORMAL);
    cv::moveWindow(face_window_name, 400, 100);
    cv::namedWindow("Left Eye",CV_WINDOW_NORMAL);
    cv::moveWindow("Left Eye", 50, 400);
    cv::namedWindow("Right Eye",CV_WINDOW_NORMAL);
    cv::moveWindow("Right Eye", 400, 400);

    createCornerKernels();
    ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2),
            43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);

    while( true ) {
        ALValue img = camProxy.getImageRemote(clientName);

        /** Retrieve an image from the camera.
           * The image is returned in the form of a container object, with the
           * following fields:
           * 0 = width
           * 1 = height
           * 2 = number of layers
           * 3 = colors space index (see alvisiondefinitions.h)
           * 4 = time stamp (seconds)
           * 5 = time stamp (micro seconds)
           * 6 = image buffer (size of width * height * number of layers)
           * Access the image buffer (6th field) and assign it to the opencv image
           * container. */

        frame.data = (uchar*) img[6].GetBinary();

        /** Tells to ALVideoDevice that it can give back the image buffer to the
          * driver. Optional after a getImageRemote but MANDATORY after a getImageLocal.*/
        camProxy.releaseImage(clientName);

        // mirror it
        cv::flip(frame, frame, 1);
        frame.copyTo(debugImage);

        // Apply the classifier to the frame
        if( !frame.empty() ) {
            std::vector<cv::Rect> faces;

            std::vector<cv::Mat> rgbChannels(3);
            cv::split(frame, rgbChannels);
            frame_gray = rgbChannels[2];

            face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150) );

            for( int i = 0; i < faces.size(); i++ )
            {
                rectangle(debugImage, faces[i], 1234);
            }
            //-- Show what you got
            if (faces.size() > 0) {
                findEyes(frame_gray, faces[0]);
            }

            /** Uncomment to save videos **/
//            if (writer1.isOpened()) writer1.write(frame);
//            else printf("--(!)Error: While opening video writer\n");

//            if ((!frame_gray.empty()) && (writer2.isOpened())) writer2.write(frame_gray);
//            else printf("--(!)Error: While opening video writer\n");
        }
        else {
            printf(" --(!) No captured frame -- Break!");
            break;
        }


        /** Display the iplImage on screen.*/
        imshow(main_window_name,debugImage);

        int c = cv::waitKey(10);
        if( (char)c == 'q' )
        {
            printf("Stoping");
            break;
        }
        if( (char)c == 'f' )
        {
            printf("Saving frame");
            imwrite("frame.png",frame);
            imwrite("frame_gray.png",frame_gray);
        }
    }

    /** Cleanup.*/
    camProxy.unsubscribe(clientName);
    releaseCornerKernels();

    return 0;
}

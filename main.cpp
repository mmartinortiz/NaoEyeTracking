/**
 *
 * This example demonstrates how to get images from the NAO robot remotely, detect
 * the eye movement and how to display them on your screen using opencv.
 *
 * This code is completly based on the "getimage" Aldebaran's example [1]
 * and on the EyeTracking algorithm implemented by Tristam Hume [2] based
 * on the paper "Accurate eye centre localisation by means of gradients" [3]
 *
 * [1] http://www.aldebaran-robotics.com/documentation/dev/cpp/examples/vision/getimage/getimage.html
 * [2] http://thume.ca/projects/2012/11/04/simple-accurate-eye-center-tracking-in-opencv/
 * [3] Timm and Barth. Accurate eye centre localisation by means of gradients.
 *     In Proceedings of the Int. Conference on Computer Theory and Applications (VISAPP),
 *     volume 1, pages 125-130, Algarve, Portugal, 2011. INSTICC.
 *
 * Feel free to modify, redistribute, play... but please, always keep the appropiate
 * recognition to the originals authors listed above. In my blog you will find complementary
 * information.
 *
 * To test the code, you need:
 * -> Aldebaran's SDK
 * -> OpenCV 2.4 (I suppouse it also works with OpenCV 2.3
 * -> NAO Robot, it is not provided with this code :-)
 *
 * Manuel Martin <mmartinortiz@gmail.com>
 * A Bit World <abitworld.com/blog>
 */

#include <alerror/alerror.h>
#include <iostream>
#include "findFunctions.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage 'getimages robotIp'" << std::endl;
        return 1;
    }

    const std::string robotIp(argv[1]);

    try
    {
        return showImages(robotIp);
    }
    catch (const AL::ALError& e)
    {
        std::cerr << "Caught exception " << e.what() << std::endl;
    }

    return 0;
}

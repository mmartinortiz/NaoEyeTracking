cmake_minimum_required(VERSION 2.8)
project(NaoEyeTracking)

find_package(qibuild)

set (_srcs
findEyeCenter.cpp
findEyeCenter.h
findEyeCorner.cpp
findEyeCorner.h
findFunctions.cpp
findFunctions.h
constants.h
helpers.cpp
helpers.h
main.cpp
)

# Create a executable named NaoEyeTracking
# with the source file: main.cpp
qi_create_bin(NaoEyeTracking ${_srcs})

# Here we say that our executable depends on
# - ALCOMMON (main naoqi lib)
# - ALVISION (for vision definitions)
# - OPENCV (display)
#
# It automatically links with the corresponding libraries and make their headers
# available.
qi_use_lib(NaoEyeTracking ALCOMMON ALVISION OPENCV2_CORE OPENCV2_HIGHGUI OPENCV2_OBJDETECT OPENCV2_IMGPROC )

# Add a simple test:
#enable_testing()
#qi_create_test(test_NaoEyeTracking "test.cpp")

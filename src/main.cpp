#include <iostream>
#include <opencv2/core/cvdef.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <sstream>
using namespace cv;
using namespace std;
const char *params = "{ help h         |           | Print usage }"
                     "{ input          | ../data/FireSafetyVideo.mp4 | Path to "
                     "a video or a sequence of image }"
                     "{ algo           | MOG2      | Background subtraction "
                     "method (KNN, MOG2) }";
int main(int argc, char *argv[]) {
  CommandLineParser parser(argc, argv, params);
  parser.about("This program shows how to use background subtraction methods "
               "provided by "
               " OpenCV. You can process both videos and images.\n");
  if (parser.has("help")) {
    // print help information
    parser.printMessage();
  }
  // create Background Subtractor objects
  Ptr<BackgroundSubtractor> pBackSub;
  if (parser.get<String>("algo") == "MOG2")
    pBackSub = createBackgroundSubtractorMOG2();
  else
    pBackSub = createBackgroundSubtractorKNN();
  VideoCapture capture(samples::findFile(parser.get<String>("input")));
  if (!capture.isOpened()) {
    // error in opening the video input
    cerr << "Unable to open: " << parser.get<String>("input") << endl;
    return 0;
  }

  // Save video
	int ex = static_cast<int>(capture.get(CAP_PROP_FOURCC));


  //int codec = VideoWriter::fourcc('a', 'v', 'c', '1');
  double fps = capture.get(CAP_PROP_FPS);
  //Size frameSize(capture.get(CAP_PROP_FRAME_WIDTH), capture.get(CAP_PROP_FRAME_HEIGHT));

  //VideoWriter writer("../data/output_video.avi", codec, fps, frameSize);
	VideoWriter outputVideo;
	Size S = Size((int) capture.get(CAP_PROP_FRAME_WIDTH),
							 (int) capture.get(CAP_PROP_FRAME_HEIGHT));
	outputVideo.open("../data/output_video.avi", ex, fps, S, false);

  if (!outputVideo.isOpened()) {
    cerr << "Error: Could not open the video writer" << endl;
    return -1;
  }

  Mat frame, fgMask;
  while (true) {
    capture >> frame;
    if (frame.empty())
      break;
    // update the background model
    pBackSub->apply(frame, fgMask);
    // get the frame number and write it on the current frame
    rectangle(frame, cv::Point(10, 2), cv::Point(100, 20),
              cv::Scalar(255, 255, 255), -1);
    stringstream ss;
    ss << capture.get(CAP_PROP_POS_FRAMES);
    string frameNumberString = ss.str();
    putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
            FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    // show the current frame and the fg masks
    imshow("Frame", frame);
    imshow("FG Mask", fgMask);

		// Save video
		//outputVideo.write(fgMask);
		outputVideo << fgMask;

    // get the input from the keyboard
    int keyboard = waitKey(30);
    if (keyboard == 'q' || keyboard == 27)
      break;
  }

  return 0;
}

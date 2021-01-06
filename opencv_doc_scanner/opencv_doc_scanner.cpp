#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void drawContoursOn(Mat src, Mat src_frame, vector<Point2f>& angles)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	cv::findContours(src, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	size_t max_area_index = 0;
	int max_area = 0;
	vector<Point2f> l_angles;
	for (int i = 0; i < contours.size(); i++)
	{
		float peri = arcLength(contours[i], true);
		approxPolyDP(contours[i], l_angles, peri * 0.02, true);

		int current_arrea = cv::contourArea(contours[i]);
		if (max_area < current_arrea)
		{
			max_area = current_arrea;
			max_area_index = i;
		}
	}

	Mat temp = src_frame.clone();
	cv::drawContours(temp, contours, max_area_index, Scalar(255, 0, 255, 255), 2);
	for (auto point : l_angles)
		cv::circle(temp, point, 10, Scalar(0, 0, 255, 255), FILLED);

	imshow("src_frame", temp);

	angles = l_angles;
}

int main(int argc, char** argv)
{
	int threshold1 = 44, threshold2 = 292;
	Mat frame;
	VideoCapture cap(0);

	//namedWindow("trackbars", WINDOW_AUTOSIZE);
	//createTrackbar("trackbar1", "trackbars", &threshold1, 400);
	//createTrackbar("trackbar2", "trackbars", &threshold2, 400);

	while (true)
	{
		cap.read(frame);
		cv::resize(frame, frame, Size(frame.cols, frame.rows));

		Mat gray;
		cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);

		Mat blured;
		GaussianBlur(frame, blured, Size(3, 3), 5);

		Mat canny;
		Canny(blured, canny, threshold1, threshold2);

		Mat dilated;
		Mat kernel = getStructuringElement(cv::MORPH_RECT, Size(4, 4));
		cv::dilate(canny, dilated, kernel);

		Mat eroded;
		erode(dilated, eroded, kernel);

		vector<Point2f> src;
		drawContoursOn(eroded, frame, src);
		if (src.size() == 4)
		{
			Mat buff, warped;
			float height = 297.0f * 2, width = 210.0f * 2;
			vector<Point2f> dst = { {0.0f, 0.0f}, {width, 0.0f}, {0.0f, height}, {width, height} };

			vector<Point2f> new_src = { src[0], src[3], src[1], src[2] };
			src = new_src;

			buff = getPerspectiveTransform(src, dst);
			warpPerspective(frame, warped, buff, Point(width, height));
			imshow("warped", warped);
			for (int i = 0; i < src.size(); i++)
				cout << src[i].x << " " << src[i].y << "|";
			cout << endl;
		}

		waitKey(8);
	}
}
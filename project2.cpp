#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

///////////////// Images //////////////////////

Mat imgoriginal, imgGray,imgBlur, imgCanny,imgthre ,imgDil,imgErode,imgWrap,imgcrop; 
vector<Point> initialpoints,docpoints;
float w = 420, h = 596;
Mat preProcessing(Mat img)
{
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	//erode(imgDil, imgErode, kernel);
	return imgDil;
}
 vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//drawContours(img, contours, -1, Scalar(255, 0, 255), 2);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point> biggest; 
	int maxArea=0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		cout << area << endl;

		string objectType;

		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

			if (area > maxArea && conPoly[i].size()==4)
			{
				//drawContours(imgoriginal, conPoly, i, Scalar(255, 0, 255), 5);
				biggest = { conPoly[i][0],conPoly[i][1], conPoly[i][2], conPoly[i][3], };
				maxArea = area;
			}

			//drawContours(imgoriginal, conPoly, i, Scalar(255, 0, 255), 2);
			//rectangle(imgoriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
		}
	}
	return biggest;
}
 void drawPoints(vector<Point> points, Scalar color)
 {
	 for (int i = 0; i < points.size(); i++)
	 {
		 circle(imgoriginal, points[i], 10, color, FILLED);
		 putText(imgoriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 4, color, 4);
	 }
 }
 vector<Point> reorder(vector<Point> points)
 {
	 vector<Point> newpoints;
	 vector<int> sumpoints,subpoints;

	 for (int i = 0; i < 4;  i++)
	 {
		 sumpoints.push_back(points[i].x + points[i].y);
		 subpoints.push_back(points[i].x - points[i].y);
	 }
	newpoints.push_back(points[min_element(sumpoints.begin(), sumpoints.end()) - sumpoints.begin()]);
	newpoints.push_back(points[max_element(subpoints.begin(), subpoints.end()) - subpoints.begin()]);
	newpoints.push_back(points[min_element(subpoints.begin(), subpoints.end()) - subpoints.begin()]);
	newpoints.push_back(points[max_element(sumpoints.begin(), sumpoints.end()) - sumpoints.begin()]);
	return newpoints;
 }
 Mat getWrap(Mat img, vector<Point> points, float w, float h)
 {
	 Point2f src[4] = { points[0], points[1], points[2], points[3], };
	 Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	 Mat matrix = getPerspectiveTransform(src, dst); 
	 warpPerspective(img, imgWrap, matrix, Point(w, h));
	 return imgWrap;
 }

void main() {

	string path = "Resources/paper.jpg"; 
     imgoriginal = imread(path);
	 resize(imgoriginal, imgoriginal, Size(), 0.5, 0.5); 
	 //preprocessing
	 imgthre = preProcessing(imgoriginal);
	 //get contours
	 initialpoints = getContours(imgthre);
	 //drawPoints(initialpoints, Scalar(0, 0, 255));
	 docpoints = reorder(initialpoints);
	 //drawPoints(docpoints, Scalar(0, 255, 255));
	 //wrap
	 imgWrap = getWrap(imgoriginal, docpoints, w, h);
	 //crop
	 int cropval = 10;
	 Rect roi(cropval, cropval, w - (2 * cropval), h - (2 * cropval));
	 imgcrop = imgWrap(roi);

	imshow("image", imgoriginal);
	imshow("image dil", imgthre);
	imshow("image wrap", imgWrap);
	imshow("image crop", imgcrop);
	waitKey(0);

}
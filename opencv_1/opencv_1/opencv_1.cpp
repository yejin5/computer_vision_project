
//20143079 오예진 컴퓨터비전 기말 프로젝트

#include <opencv2\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

Mat blur_noise(Mat &original_img);
int histogram(Mat &original_img);
Mat gamma_transformation(Mat &original_img, float gamma);

int main(int, char**)
{
	Mat original_img, result_img;
	int num_mot = 0, num_tri = 0, num_rect = 0, num_round = 0;

	//이미지파일을 로드하여 image에 저장 
	//img_input : 원본 image를 grayscale로 load한 후 이진화 등을 이용하여 도형 판별에 사용
	original_img = imread("img.jpg", IMREAD_GRAYSCALE);
	if (original_img.empty())
	{
		cout << "image empty" << std::endl;
		return -1;
	} 
	//img_result : 원본 image에 판별된 도형 표시
	result_img = imread("img.jpg", IMREAD_COLOR);
	if (result_img.empty())
	{
		cout << "image empty" << std::endl;
		return -1;
	}
	imshow("original_img", original_img);

	//blur_noise는 개선하지 않음
	//original_img = blur_noise(original_img);
	//imshow("blur_noise", original_img);
	
	int histo;
	histo = histogram(original_img);
	if (histo == 1)			//어두운 영상이면
		original_img = gamma_transformation(original_img, 0.5);
	else						//밝은 영상이면
		original_img = gamma_transformation(original_img, 1.7);
	imshow("gamma_img", original_img);
	
	
	//이진화 이미지로 변환
	//thresholding
	threshold(original_img, original_img, 100, 250, THRESH_BINARY_INV | THRESH_OTSU);		//오츄 알고리즘 사용
	
	//close연산 수행(못 검출을 위함)
	Mat to_close = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));		//mask 사이즈는 (3,3)으로 지정
	morphologyEx(original_img, original_img, CV_MOP_CLOSE, to_close);

	//findContours함수를 사용하여 contour list인 contours 찾음
	vector<vector<Point>> contours;
	findContours(original_img, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	//찾아낸 contour를 근사화함
	vector<Point> img_approx;
	for (int i = 0; i < contours.size(); i++)
	{
		//epsilon : 호의길이*0.02로 설정함
		approxPolyDP(Mat(contours[i]), img_approx, arcLength(Mat(contours[i]), true)*0.02, true);
		if (contourArea(Mat(img_approx)) > 300)  //면적이 일정크기 이상이어야 한다. 
		{
			Rect out_rect = boundingRect(contours[i]);		//주어진 contour와 외접하는 직사각형
			Point point_center(out_rect.x + (out_rect.width / 2), out_rect.y + (out_rect.height / 2));		//외접하는 직사각형의 중심점 point_center를 구함

			//Contour를 근사화한 직선을 그린다.
			for (int j = 0; j < img_approx.size(); j++) {
				if ((contourArea(Mat(img_approx))) < 1500) {		//면적이 1500보다 작으면 못으로 간주함
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(255, 255, 0), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(0, 0, 0), 3, 7);	//result_img에 중심좌표를 나타낸다
					//c = "center"
				}

				else if (img_approx.size() == 3) {						//삼각형
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(0, 0, 255), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(0, 0, 255), 3, 7);	//result_img에 중심좌표를 나타낸다
				}

				else if (img_approx.size() == 4) { 						//사각형
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(255, 0, 255), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(255, 0, 255), 3, 7);	//result_img에 중심좌표를 나타낸다
				}	

				else { 															//꼭짓점이 4개보다 많으면 원으로 간주함
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(102, 255, 0), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(102, 255, 0), 3, 7);	//result_img에 중심좌표를 나타낸다
				}

			}

			//각 모양의 중심점 출력
			if ((contourArea(Mat(img_approx))) < 1500)
				cout << "mot_locate : " << point_center << endl;
			else if (img_approx.size() == 3)
				cout << "triangle_locate : " << point_center << endl;
			else if (img_approx.size() == 4)
				cout << "rectangle_locate : " << point_center << endl;
			else
				cout << "round_locate : " << point_center << endl;

			//도형 판별
			//꼭짓점이 3개나 그 이상으로 나와도 면적이 1500보다 작으면 못으로 간주함
			if ((contourArea(Mat(img_approx))) < 1500)
				num_mot++;
	
			else if (img_approx.size() == 3)				//삼각형
				num_tri++;

			else if (img_approx.size() == 4) 				//사각형
				num_rect++;

			else 											//꼭짓점이 4개보다 많으면 원으로 간주함
				num_round++;

		}
	}
	cout << "num_of_mot : " << num_mot << endl;			//못의 개수
	cout << "num_of_triangle : " << num_tri << endl;		//삼각형 개수
	cout << "num_of_rectangle : " << num_rect << endl;	//사각형 개수
	cout << "num_of_round : " << num_round << endl;		//원 개수

	imshow("computed_img", original_img);
	imshow("result_img", result_img);

	waitKey(0);

	return 0;
}

Mat blur_noise(Mat &original_img) {

	Mat blured_img = original_img.clone();

	int mask[3][3] = { {-1,-1,-1},{-1,9,-1},{-1,-1,-1} };		//라플라시안 +1

	long int sum;
	int mask_size = 3;			//mask[3][3]

	for (int row = 0 + mask_size / 2; row < original_img.rows - mask_size / 2; row++) {
		for (int col = 0 + mask_size / 2; col < original_img.cols - mask_size / 2; col++) {
			sum = 0;
			for (int i = -1 * mask_size / 2; i <= mask_size / 2; i++) {
				for (int j = -1 * mask_size / 2; j <= mask_size / 2; j++) {
					sum += original_img.at<uchar>(row + i, col + j) * mask[mask_size / 2 + i][mask_size / 2 + j];
				}
			}

			if (sum > 255)			//명암은 255를 넘지 않게 한다.
				sum = 255;
			if (sum < 0)
				sum = 0;

			blured_img.at<uchar>(row, col) = sum;
		}
	}
	return blured_img;
}

int histogram(Mat &original_img) {

	float histogram[256] = { 0 };		//히스토그램
	double count_histo_l = 0, count_histo_d = 0;		// count_histo_l:밝음,  count_histo_d:어두움
	int num1 = 1, num2 = 2;

	for (int row = 0; row < original_img.rows; row++) {
		for (int col = 0; col < original_img.cols; col++) {
			histogram[original_img.at<uchar>(row, col)]++;
		}
	}

	for (int i = 0; i < 256; i++) {
		if (i < 128)
			count_histo_d += histogram[i];
		else
			count_histo_l += histogram[i];
	}

	if (count_histo_l < count_histo_d)
		return num1;		//어두운 영상
	else
		return num2;		//밝은 영상

}

Mat gamma_transformation(Mat &original_img, float gamma) {
	Mat gamma_img = original_img.clone();

	float c = 255;

	for (int row = 0; row < original_img.rows; row++) {
		for (int col = 0; col < original_img.cols; col++) {
			gamma_img.at<uchar>(row, col) = c * pow(original_img.at<uchar>(row, col) / c, gamma);
		}
	}
	return gamma_img;
}
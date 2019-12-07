
//20143079 ������ ��ǻ�ͺ��� �⸻ ������Ʈ

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

	//�̹��������� �ε��Ͽ� image�� ���� 
	//img_input : ���� image�� grayscale�� load�� �� ����ȭ ���� �̿��Ͽ� ���� �Ǻ��� ���
	original_img = imread("img.jpg", IMREAD_GRAYSCALE);
	if (original_img.empty())
	{
		cout << "image empty" << std::endl;
		return -1;
	} 
	//img_result : ���� image�� �Ǻ��� ���� ǥ��
	result_img = imread("img.jpg", IMREAD_COLOR);
	if (result_img.empty())
	{
		cout << "image empty" << std::endl;
		return -1;
	}
	imshow("original_img", original_img);

	//blur_noise�� �������� ����
	//original_img = blur_noise(original_img);
	//imshow("blur_noise", original_img);
	
	int histo;
	histo = histogram(original_img);
	if (histo == 1)			//��ο� �����̸�
		original_img = gamma_transformation(original_img, 0.5);
	else						//���� �����̸�
		original_img = gamma_transformation(original_img, 1.7);
	imshow("gamma_img", original_img);
	
	
	//����ȭ �̹����� ��ȯ
	//thresholding
	threshold(original_img, original_img, 100, 250, THRESH_BINARY_INV | THRESH_OTSU);		//���� �˰��� ���
	
	//close���� ����(�� ������ ����)
	Mat to_close = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));		//mask ������� (3,3)���� ����
	morphologyEx(original_img, original_img, CV_MOP_CLOSE, to_close);

	//findContours�Լ��� ����Ͽ� contour list�� contours ã��
	vector<vector<Point>> contours;
	findContours(original_img, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	//ã�Ƴ� contour�� �ٻ�ȭ��
	vector<Point> img_approx;
	for (int i = 0; i < contours.size(); i++)
	{
		//epsilon : ȣ�Ǳ���*0.02�� ������
		approxPolyDP(Mat(contours[i]), img_approx, arcLength(Mat(contours[i]), true)*0.02, true);
		if (contourArea(Mat(img_approx)) > 300)  //������ ����ũ�� �̻��̾�� �Ѵ�. 
		{
			Rect out_rect = boundingRect(contours[i]);		//�־��� contour�� �����ϴ� ���簢��
			Point point_center(out_rect.x + (out_rect.width / 2), out_rect.y + (out_rect.height / 2));		//�����ϴ� ���簢���� �߽��� point_center�� ����

			//Contour�� �ٻ�ȭ�� ������ �׸���.
			for (int j = 0; j < img_approx.size(); j++) {
				if ((contourArea(Mat(img_approx))) < 1500) {		//������ 1500���� ������ ������ ������
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(255, 255, 0), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(0, 0, 0), 3, 7);	//result_img�� �߽���ǥ�� ��Ÿ����
					//c = "center"
				}

				else if (img_approx.size() == 3) {						//�ﰢ��
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(0, 0, 255), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(0, 0, 255), 3, 7);	//result_img�� �߽���ǥ�� ��Ÿ����
				}

				else if (img_approx.size() == 4) { 						//�簢��
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(255, 0, 255), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(255, 0, 255), 3, 7);	//result_img�� �߽���ǥ�� ��Ÿ����
				}	

				else { 															//�������� 4������ ������ ������ ������
					line(result_img, img_approx[j], img_approx[(j + 1) % img_approx.size()], Scalar(102, 255, 0), 3);
					putText(result_img, "c", point_center, FONT_HERSHEY_PLAIN, 0.5, Scalar(102, 255, 0), 3, 7);	//result_img�� �߽���ǥ�� ��Ÿ����
				}

			}

			//�� ����� �߽��� ���
			if ((contourArea(Mat(img_approx))) < 1500)
				cout << "mot_locate : " << point_center << endl;
			else if (img_approx.size() == 3)
				cout << "triangle_locate : " << point_center << endl;
			else if (img_approx.size() == 4)
				cout << "rectangle_locate : " << point_center << endl;
			else
				cout << "round_locate : " << point_center << endl;

			//���� �Ǻ�
			//�������� 3���� �� �̻����� ���͵� ������ 1500���� ������ ������ ������
			if ((contourArea(Mat(img_approx))) < 1500)
				num_mot++;
	
			else if (img_approx.size() == 3)				//�ﰢ��
				num_tri++;

			else if (img_approx.size() == 4) 				//�簢��
				num_rect++;

			else 											//�������� 4������ ������ ������ ������
				num_round++;

		}
	}
	cout << "num_of_mot : " << num_mot << endl;			//���� ����
	cout << "num_of_triangle : " << num_tri << endl;		//�ﰢ�� ����
	cout << "num_of_rectangle : " << num_rect << endl;	//�簢�� ����
	cout << "num_of_round : " << num_round << endl;		//�� ����

	imshow("computed_img", original_img);
	imshow("result_img", result_img);

	waitKey(0);

	return 0;
}

Mat blur_noise(Mat &original_img) {

	Mat blured_img = original_img.clone();

	int mask[3][3] = { {-1,-1,-1},{-1,9,-1},{-1,-1,-1} };		//���ö�þ� +1

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

			if (sum > 255)			//����� 255�� ���� �ʰ� �Ѵ�.
				sum = 255;
			if (sum < 0)
				sum = 0;

			blured_img.at<uchar>(row, col) = sum;
		}
	}
	return blured_img;
}

int histogram(Mat &original_img) {

	float histogram[256] = { 0 };		//������׷�
	double count_histo_l = 0, count_histo_d = 0;		// count_histo_l:����,  count_histo_d:��ο�
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
		return num1;		//��ο� ����
	else
		return num2;		//���� ����

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
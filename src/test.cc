#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;
using namespace cv;

const double PI = 3.14159;

vector< vector< vector<Point> > > lanes;

void read_image_lines() {
  string image_dir = "../caltech-lanes/test1/";
  string result_file = "list.txt_results.txt";
  ifstream in(image_dir + result_file);
  if (! in.is_open()) {
    cout << "open result file fail!" << endl;
    exit(1);
  }
  vector< vector<Point> > line_vec;
  char buffer[256];
  string s;
  while (! in.eof()) {
    in.getline(buffer, 100);
    s = string(buffer);
    if (s.size() == 0) {
      continue;
    }
    int line_num = s[19] - '0';
    // printf("line num: %d\n", line_num);
    while (line_num-- && ! in.eof()) {
      in.getline(buffer, 100);
      s = string(buffer);
      int points_num = s[14] - '0';
      vector<Point> point_vec;
      while (points_num-- && ! in.eof()) {
        in.getline(buffer, 100);
        s = string(buffer);
        stringstream ss;
        ss << s;
        float x, y;
        ss >> x;
        char c;
        ss >> c;
        ss >> y;
        Point p(x, y);
        point_vec.push_back(p);
      }
      line_vec.push_back(point_vec);
    }
    lanes.push_back(line_vec);
    // cout << "\n\n" << endl;
    line_vec.clear();
  }
  // cout << lanes.size() << endl;
  for (auto image_lines: lanes) {
    // cout << "lines: " << image_lines.size() << endl;
    for (auto image_line_points: image_lines) {
      // cout << "points: " << image_line_points.size() << endl;
      for (auto line_point: image_line_points) {
        // cout << line_point.x <<"\t" << line_point.y << endl;
      }
    }
    // cout << "\n" << endl;
  }
}

int main()
{
  read_image_lines();
  char buffer[256];
  // ifstream in("../caltech-lanes/test1/label.txt");
  ifstream in("../caltech-lanes/image_with_lanes/label.txt");
  if (!in.is_open()) {
    cout << "open file fail!" << endl;
    return 1;
  }
  // string image_dir = "../caltech-lanes/test1/";
  string image_dir = "../caltech-lanes/image_with_lanes/";
  vector<string> str_vec;
  int image_index = 0;
  while (! in.eof()) {
    str_vec.clear();
    while (! in.eof()) {
      in.getline(buffer, 100);
      string l(buffer);
      // cout << l.size() << endl;
      if (l.size() == 1 || l.size() == 0) {
        break;
      }
      // cout << l << endl;
      str_vec.push_back(l);
    }
    // cout << str_vec[0].size() << endl;
    string image_name = str_vec[0].substr(0, 36);
    Mat image = imread(image_dir + image_name);
    Mat I;
    Canny(image, I, 125, 350);
    cvtColor(image, I, CV_BGR2GRAY);
    Mat contours;
    Canny(I, contours, 640, 350);
    vector<Vec4i> lines;
    // cout << "\n\n" << endl;
    for (int i = 1; i < str_vec.size(); i++) {
      stringstream in_s;
      // cout << str_vec[i] << endl;
      in_s << str_vec[i];
      vector<int> ax_vec;
      int temp;
      while (! in_s.eof()) {
        in_s >> temp;
        ax_vec.push_back(temp);
      }
      vector<int> left_up_and_right_down(4, 0);
      for (int j = 0; j < 4; j++) {
        left_up_and_right_down[j] = ax_vec[j];
      }
      Vec4i up_edge;
      up_edge[0] = left_up_and_right_down[0];
      up_edge[1] = left_up_and_right_down[1];
      up_edge[2] = left_up_and_right_down[2];
      up_edge[3] = left_up_and_right_down[1];

      Vec4i down_edge;
      down_edge[0] = left_up_and_right_down[0];
      down_edge[1] = left_up_and_right_down[3];
      down_edge[2] = left_up_and_right_down[2];
      down_edge[3] = left_up_and_right_down[3];

      Vec4i left_edge;
      left_edge[0] = left_up_and_right_down[0];
      left_edge[1] = left_up_and_right_down[1];
      left_edge[2] = left_up_and_right_down[0];
      left_edge[3] = left_up_and_right_down[3];

      Vec4i right_edge;
      right_edge[0] = left_up_and_right_down[2];
      right_edge[1] = left_up_and_right_down[1];
      right_edge[2] = left_up_and_right_down[2];
      right_edge[3] = left_up_and_right_down[3];

      lines.push_back(left_edge);
      lines.push_back(right_edge);
      lines.push_back(up_edge);
      lines.push_back(down_edge);


      if (lanes[image_index].size() != 0) {
        // Point left_up(left_up_and_right_down[0], left_up_and_right_down[1]);
        Point right_down(left_up_and_right_down[2], left_up_and_right_down[3]);
        Point left_down(left_up_and_right_down[0], left_up_and_right_down[3]);
        // Point right_up(left_up_and_right_down[2], left_up_and_right_down[1]);
        // vector<Point> candidate_points = {left_up, left_down, right_up, right_down};
        vector<Point> candidate_points = {left_down, right_down};
        for (auto candidate_point : candidate_points) {
          for (auto line_points : lanes[image_index]) {
            double a = line_points[3].y - line_points[0].y;
            double b = line_points[0].x - line_points[3].x;
            double c = line_points[3].x * line_points[0].y -
                       line_points[0].x * line_points[3].y;
            if (candidate_point == left_down) {
              Vec4i add_edge;
              add_edge[0] = line_points[3].x;
              add_edge[1] = line_points[3].y;
              double x_lowest = - (360 * b + c) / a;
              if (x_lowest < 0) {
                add_edge[2] = 0;
                add_edge[3] = -c / b;
              } else if (x_lowest > 640) {
                add_edge[2] = 640;
                add_edge[3] = -(640 * a + c) / b;
              } else {
                add_edge[2] = x_lowest;
                add_edge[3] = 360;
              }
              cout << add_edge[0] << endl;
              lines.push_back(add_edge);
            }

            if (line_points[0].y > candidate_point.y) {
              double d = fabs( (a * candidate_point.x + b *
                                candidate_point.y + c)
                               / sqrt(a * a + b * b) );
              double p2p_dis =
                  sqrt((line_points[0].x - candidate_point.x) *
                       (line_points[0].x - candidate_point.x) +
                       (line_points[0].y - candidate_point.y) *
                       (line_points[0].y - candidate_point.y));
              if (asin(d / p2p_dis) < PI / 9) {
                cout << "hehe" << endl;
                Vec4i add_edge;
                add_edge[0] = line_points[0].x;
                add_edge[1] = line_points[0].y;
                add_edge[2] = candidate_point.x;
                add_edge[3] = candidate_point.y;
                lines.push_back(add_edge);
                cout << add_edge[0] << endl;
              }
            }
          }
        }
      }
    }
    threshold(contours, contours, 128, 255, THRESH_BINARY);
    cout << lines.size() << endl;
    auto it = lines.begin();
    while(it != lines.end())
    {
      Point pt1((*it)[0], (*it)[1]);
      Point pt2((*it)[2], (*it)[3]);
      line(image, pt1, pt2, Scalar(0,255,0), 2); //  线条宽度设置为2
      ++it;
    }
    image_index++;
    namedWindow("Lines");
    imshow("Lines", image);
    cout << "image_index: " << image_index << endl;
    waitKey();
  }

  return 0;
}

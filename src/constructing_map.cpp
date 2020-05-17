#include <ros/ros.h>
#include <visualization_msgs/Marker.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

#include <cmath>
#include <exception>

using namespace std;
using namespace cv;

Mat read_image(String path, int image_color=1){
	Mat img;
	if (image_color == 0)
		img = imread(path, 0);
	else
		img = imread(path);

    if(img.empty())
    {
		throw("Could not read the image: " + path);
    }

	return img;
}

Vec3i parse_arguments(int argc, char** argv){
	int start_frame;
	int end_frame;
	int rate;
	cout << argc << endl;
	if (argc == 1){
		start_frame = 0;
		end_frame = 5;
		rate = 30;
	}
	else if(argc == 3){
		start_frame = stoi(argv[1]);
		end_frame = stoi(argv[2]);
		rate = 30;
	}
	else{
		start_frame = stoi(argv[1]);
		end_frame = stoi(argv[2]);
		rate = stoi(argv[3]);
	}

	Vec3i u(start_frame, end_frame, rate);

	return u;
}

int main(int argc, char** argv){
	// arguments
	Vec3i arguments = parse_arguments(argc, argv);
	int start_frame = arguments.val[0];
	int end_frame = arguments.val[1];
	int rate = arguments.val[2];
	cout << start_frame << " " << end_frame << " " << rate << endl;
	
	// paths
	String image_folder_root_path = "/home/emre/Programs/packnet-sfm/data/datasets/hacettepe_cs_loop/data";
	//image_folder_root_path = "/home/emre/Programs/packnet-sfm/data/save/depth/KITTI_tiny-kitti_tiny-velodyne/ResNet18_MR_selfsup_K";
	
	ros::init(argc, argv, "points_and_lines");
	ros::NodeHandle n;
	ros::Publisher marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 10);
	ros::Rate r(rate);
	
	int counter_image = -1;
	while (ros::ok())
  	{	
		if (start_frame > counter_image++) continue;
		cout << counter_image << endl;
		String image_path;
		String image_depth_path;
	
		if (counter_image < 10){
			image_path = image_folder_root_path + "/kitti_tiny_000000000"+to_string(counter_image)+"_rgb.png";
			image_depth_path = image_folder_root_path + "/kitti_tiny_000000000"+to_string(counter_image)+"_depth.png";
		}else if (counter_image < 100){
			image_path = image_folder_root_path + "/kitti_tiny_00000000"+to_string(counter_image)+"_rgb.png";
			image_depth_path = image_folder_root_path + "/kitti_tiny_00000000"+to_string(counter_image)+"_depth.png";
		}
		else if (counter_image < 1000){
			image_path = image_folder_root_path + "/kitti_tiny_0000000"+to_string(counter_image)+"_rgb.png";
			image_depth_path = image_folder_root_path + "/kitti_tiny_0000000"+to_string(counter_image)+"_depth.png";
		}
		else{
			image_path = image_folder_root_path + "/kitti_tiny_000000"+to_string(counter_image)+"_rgb.png";
			image_depth_path = image_folder_root_path + "/kitti_tiny_000000"+to_string(counter_image)+"_depth.png";
		}

		Mat image = read_image(image_path, 1);
		Mat depth = read_image(image_depth_path, 0);
		
		visualization_msgs::Marker points, line_strip, line_list;
		points.header.frame_id = "/map";
		points.header.stamp = ros::Time::now();
		points.ns = "points_and_lines";
		points.action = visualization_msgs::Marker::ADD;
		points.pose.orientation.w = 1.0;

		points.id = 0;
		points.type = visualization_msgs::Marker::POINTS;
		points.scale.x = 1.0;
		points.scale.y = 1.0;
		points.scale.z = 1.0;
		
		// Points are green
		points.color.g = 1.0f;
		points.color.a = 1.0;


		// Create the vertices for the points and lines
		for (int i = 0; i < image.size().width; i++){
			for (int j = 0; j < image.size().height; j++){
				float x = i;
				float y = j;
				float z = depth.at<uchar>(j, i);
				//z = (1 - z/255.0) * 255.0;
				//cout<<x << " " << y << " " << z << endl;
				geometry_msgs::Point p;
				p.x = z;
				p.y = x;
				p.z = (1-y/255)*255;
				
				points.points.push_back(p);

				Vec3b intensity = image.at<Vec3b>(j, i);
				std_msgs::ColorRGBA c;
				c.r = intensity.val[2]/255.0;
				c.g = intensity.val[1]/255.0;
				c.b = intensity.val[0]/255.0;
				c.a = 1.0;
				points.colors.push_back(c);
			}
		}
		
		marker_pub.publish(points);
		r.sleep();
		if (counter_image==end_frame) counter_image = start_frame;
	}
	
	return 0;
}



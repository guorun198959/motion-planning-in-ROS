/// \file
/// \brief Node to draw the features of the real world map
///
/// PARAMETERS:
///     obstacles (std::vector<std::vector<std::vector<double>) a vector of polygons represented by a vector of x,y coords for the verticies
///     map_x_lims (std::vector<double>) [xmin, xmax] of the map
///     map_y_lims (std::vector<double>) [ymin, ymax] of the map
///     robot_radius (double) buffer radius to avoid collisions with the robot body
///     k_nearest (unsigned int) number of neighboring verticies to match to
///     graph_size (unsigned int) number of nodes to use to build the graph
///     r (std::vector<int>) color values
///     g (std::vector<int>) color values
///     b (std::vector<int>) color values
/// PUBLISHES:
///     /visualization_marker_array (visualization_msgs::MarkerArray) markers

#include <vector>
#include <XmlRpcValue.h>

#include <ros/ros.h>

#include "geometry_msgs/Point.h"
#include "visualization_msgs/MarkerArray.h"
#include "visualization_msgs/Marker.h"

#include "rigid2d/rigid2d.hpp"
#include "roadmap/prm.hpp"



static std::vector<double> r, g, b;

/// \brief Convert a Vector2D into a geometry_msgs/Point
/// \param vec point represented with a 2D vector
/// \returns a point represented with a geometry_msgs/Point
geometry_msgs::Point Vec2D_to_GeoPt(rigid2d::Vector2D vec)
{
  geometry_msgs::Point point;

  point.x = vec.x;
  point.y = vec.y;
  point.z = 0;

  return point;
}

/// \brief Create a spherical Marker based on a node struct
/// \param node a node struct to vizualize
/// \returns a marker to add to the MarkerArray
visualization_msgs::Marker make_marker(prm::Node node)
{
  visualization_msgs::Marker marker;

  marker.header.frame_id = "map";
  marker.header.stamp = ros::Time::now();

  marker.ns = "Nodes";
  marker.id = node.id;

  marker.type = visualization_msgs::Marker::SPHERE;
  marker.action = visualization_msgs::Marker::ADD;

  marker.pose.position.x = node.point.x;
  marker.pose.position.y = node.point.y;
  marker.pose.position.z = 0;
  marker.pose.orientation.x = 0;
  marker.pose.orientation.y = 0;
  marker.pose.orientation.z = 0;
  marker.pose.orientation.w = 1;

  marker.scale.x = 0.3;
  marker.scale.y = 0.3;
  marker.scale.z = 0.3;

  marker.color.r = r.at(0);
  marker.color.g = g.at(0);
  marker.color.b = b.at(0);
  marker.color.a = 1.0;

  marker.lifetime = ros::Duration();

  return marker;
}

/// \brief Create a line Marker based on an edge struct
/// \param edge an edge struct to vizualize
/// \returns a marker to add to the MarkerArray
visualization_msgs::Marker make_marker(prm::Edge edge)
{
  visualization_msgs::Marker marker;

  std::vector<geometry_msgs::Point> points = {Vec2D_to_GeoPt(edge.node1), Vec2D_to_GeoPt(edge.node2)};

  marker.header.frame_id = "map";
  marker.header.stamp = ros::Time::now();

  marker.ns = "Edges";
  marker.id = edge.edge_id;

  marker.type = visualization_msgs::Marker::LINE_LIST;
  marker.action = visualization_msgs::Marker::ADD;

  marker.pose.orientation.x = 0;
  marker.pose.orientation.y = 0;
  marker.pose.orientation.z = 0;
  marker.pose.orientation.w = 1;

  marker.points = points;

  marker.scale.x = 0.1;

  marker.color.r = r.at(2);
  marker.color.g = g.at(2);
  marker.color.b = b.at(2);
  marker.color.a = 1.0;

  marker.lifetime = ros::Duration();

  return marker;
}

/// \brief main function to create the real world map
int main(int argc, char** argv)
{

  ros::init(argc, argv, "draw_map");
  ros::NodeHandle n;

  ros::Publisher pub_markers = n.advertise<visualization_msgs::MarkerArray>("visualization_marker_array", 1, true);

  std::vector<double> map_x_lims;
  std::vector<double> map_y_lims;
  XmlRpc::XmlRpcValue obstacles;
  double robot_radius = 0;
  int k_nearest = 5;
  int graph_size = 100;

  n.getParam("obstacles", obstacles);
  n.getParam("map_x_lims", map_x_lims);
  n.getParam("map_y_lims", map_y_lims);
  n.getParam("robot_radius", robot_radius);
  n.getParam("k_nearest", k_nearest);
  n.getParam("graph_size", graph_size);
  n.getParam("r", r);
  n.getParam("g", g);
  n.getParam("b", b);

  ROS_INFO_STREAM("PRM: x_lims: " << map_x_lims.at(0) << ", " << map_x_lims.at(1));
  ROS_INFO_STREAM("PRM: y_lims: " << map_y_lims.at(0) << ", " << map_y_lims.at(1));
  ROS_INFO_STREAM("PRM: k_nearest: " << k_nearest);
  ROS_INFO_STREAM("PRM: graph_size: " << graph_size);
  ROS_INFO_STREAM("PRM: robot_radius: " << robot_radius);
  ROS_INFO_STREAM("PRM: Loaded Params");

  for(unsigned int i = 0; i < r.size(); i++)
  {
    r.at(i) /= 255;
    g.at(i) /= 255;
    b.at(i) /= 255;
  }

  // Build Obstacles vector
  std::vector<std::vector<rigid2d::Vector2D>> polygons;
  std::vector<rigid2d::Vector2D> buf_poly, map_edge;
  rigid2d::Vector2D buf_vec;

  for(int i=0; i < obstacles.size(); i++) // loop through each obstacle
  {
    for(int j=0; j < obstacles[i].size(); j++) // loop through each point in the obstacle
    {
      buf_vec.x = double(obstacles[i][j][0]);
      buf_vec.y = double(obstacles[i][j][1]);

      buf_poly.push_back(buf_vec);
    }

    std::cout << "Points in polygon " << i << "\n";
    for(auto poly : buf_poly)
    {
      std::cout << "(" << poly.x << ", " << poly.y << ")\t"  ;
    }
    std::cout << "\n";

    polygons.push_back(buf_poly);
    buf_poly.clear();
  }

  // Initialize PRM
  prm::RoadMap prob_road_map(polygons, map_x_lims, map_y_lims);

  prob_road_map.build_map(graph_size, k_nearest, robot_radius);

  const auto all_nodes = prob_road_map.get_nodes();
  const auto all_edges = prob_road_map.get_edges();

  std::vector<visualization_msgs::Marker> markers;
  visualization_msgs::MarkerArray pub_marks;

  // Put a spherical marker at each node
  for(auto node : all_nodes)
  {
    markers.push_back(make_marker(node));
  }

  // Draw a line to show all connections.
  for(auto edge : all_edges)
  {
    markers.push_back(make_marker(edge));
  }

  pub_marks.markers = markers;
  pub_markers.publish(pub_marks);

  ros::spin();
}
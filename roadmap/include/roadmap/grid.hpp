#ifndef GRID_INCLUDE_GUARD_HPP
#define GRID_INCLUDE_GUARD_HPP
/// \file
/// \brief A library for building an occupancy grid.

#include <vector>
#include <unordered_set>

#include "rigid2d/rigid2d.hpp"

namespace grid
{
  /// \brief Contains information about a bound map
  struct Map
  {
    std::vector<std::vector<rigid2d::Vector2D>> obstacles; ///< obstacles in the map
    std::vector<double> x_bounds; ///< x bounds of the map
    std::vector<double> y_bounds; ///< y bounds of the map

    std::vector<rigid2d::Vector2D> map_vector; ///< a vector of the map verticies in ccw order
  };

  /// \brief Class to create a Grid overlay for provided Map information
  class Grid
  {

  public:

    /// \brief Initialization to construct a grid in an empty 10x10 area
    ///
    Grid();

    /// \brief Initialization to construct a grid in an empty user defined area
    /// \param xboundary a 2 element vector defining the map's x bounds in integer coordinates
    /// \param yboundary a 2 element vector defining the map's y bounds in integer coordinates
    Grid(std::vector<double> xboundary,std::vector<double> yboundary);

    /// \brief Initialization to construct a grid in a user defined area with obstacles
    /// \param polygon_verticies a vector of vectors defining the verticies of each obstacle in integer coordinates and in order going counter-clockwise
    /// \param xboundary a 2 element vector defining the map's x bounds in integer coordinates
    /// \param yboundary a 2 element vector defining the map's y bounds in integer coordinates
    Grid(std::vector<std::vector<rigid2d::Vector2D>> polygon_verticies, std::vector<double> xboundary, std::vector<double> yboundary);

    /// \brief Function to call all nessissary functions to build the grid
    /// \param cell_size the desired distance the distance for the cell length/height in meters
    /// \param grid_res scaling factor to create the occupancy grid with a finer resolution that the existing cell_size. Use 1 to make the grid equal to the current cell size Use 2 to double the resolution.
    /// \param robot_radius the radius to use as a buffer around the robot for collision detection
    void build_grid(double cell_size, unsigned int grid_res, double robot_radius);

    /// \brief convert the grid data into a in row major order with the first element corresponding to the lower left corner.
    /// \returns grid occupancy data in row major order
    std::vector<signed char> get_grid() const;

    /// \brief get the height and width of the grid
    /// \returns the grid dimensions in number of cells
    std::vector<int> get_grid_dimensions() const;

    /// \brief convert from grid coordinates (integers) to world coordinates (meters)
    /// \param grid_coord grid location to convert
    /// \returns matching world coordinate
    rigid2d::Vector2D grid_to_world(rigid2d::Vector2D grid_coord);

    /// \brief convert from world coordinates to grid coordinates
    /// \param world_coord world location to convert
    /// \returns matching grid coordinate
    rigid2d::Vector2D world_to_grid(rigid2d::Vector2D world_coord);

  private:

    Map og_map; ///< the map to initialize the grid with
    Map scaled_map; ///< the scaled map based on the grid resolution

    std::vector<int> grid_dimensions; ///< x, y

    double buffer_radius = 0.0; ///< buffer distance to incorporate when detecting collisions
    unsigned int grid_res = 1; ///< scale the cell size
    double cell_size = 1.0; ///< meters per grid cell

    std::vector<std::vector<signed char>> occ_data; ///< occupancy grid data, 0 is free, 50 is buffer zone, 100 is occupied

    /// \brief calculate the grid size based on the saved map and the grid resolution
    ///
    void grid_resize();

    /// \brief Determine if the cell center is within the buffer radius of the map boarder
    /// \param center location of cell to check against
    /// \param threshold grid buffer distance
    /// \returns True if the cell center is within the buffer_radius
    bool cell_near_boarder(rigid2d::Vector2D center, double threshold);
  };
}

#endif // GRID_INCLUDE_GUARD_HPP

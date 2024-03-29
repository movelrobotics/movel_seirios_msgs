// Copyright (c) 2020, Samsung Research America
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License. Reserved.

#include <math.h>

#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "nav2_costmap_2d/costmap_2d.hpp"
#include "nav2_costmap_2d/costmap_subscriber.hpp"
#include "nav2_util/lifecycle_node.hpp"
#include "rclcpp/rclcpp.hpp"
#include "smac_planner/a_star.hpp"
#include "smac_planner/collision_checker.hpp"
#include "smac_planner/node_se2.hpp"

class RclCppFixture
{
public:
  RclCppFixture() {rclcpp::init(0, nullptr);}
  ~RclCppFixture() {rclcpp::shutdown();}
};
RclCppFixture g_rclcppfixture;

TEST(AStarTest, test_a_star_2d)
{
  smac_planner::SearchInfo info;
  smac_planner::AStarAlgorithm<smac_planner::Node2D> a_star(smac_planner::MotionModel::MOORE, info);
  int max_iterations = 10000;
  float tolerance = 0.0;
  float some_tolerance = 20.0;
  int it_on_approach = 10;
  int num_it = 0;

  a_star.initialize(false, max_iterations, it_on_approach);
  a_star.setFootprint(nav2_costmap_2d::Footprint(), true);

  nav2_costmap_2d::Costmap2D * costmapA =
    new nav2_costmap_2d::Costmap2D(100, 100, 0.1, 0.0, 0.0, 0);
  // island in the middle of lethal cost to cross
  for (unsigned int i = 40; i <= 60; ++i) {
    for (unsigned int j = 40; j <= 60; ++j) {
      costmapA->setCost(i, j, 254);
    }
  }

  // functional case testing
  a_star.createGraph(costmapA->getSizeInCellsX(), costmapA->getSizeInCellsY(), 1, costmapA);
  a_star.setStart(20u, 20u, 0);
  a_star.setGoal(80u, 80u, 0);
  smac_planner::Node2D::CoordinateVector path;
  EXPECT_TRUE(a_star.createPath(path, num_it, tolerance));
  EXPECT_EQ(num_it, 556);

  // check path is the right size and collision free
  EXPECT_EQ(path.size(), 81u);
  for (unsigned int i = 0; i != path.size(); i++) {
    EXPECT_EQ(costmapA->getCost(path[i].x, path[i].y), 0);
  }

  // setting non-zero dim 3 for 2D search
  EXPECT_THROW(
    a_star.createGraph(costmapA->getSizeInCellsX(), costmapA->getSizeInCellsY(), 10, costmapA),
    std::runtime_error);
  EXPECT_THROW(a_star.setGoal(0, 0, 10), std::runtime_error);
  EXPECT_THROW(a_star.setStart(0, 0, 10), std::runtime_error);

  path.clear();
  // failure cases with invalid inputs
  smac_planner::AStarAlgorithm<smac_planner::Node2D> a_star_2(
    smac_planner::MotionModel::VON_NEUMANN, info);
  a_star_2.initialize(false, max_iterations, it_on_approach);
  a_star_2.setFootprint(nav2_costmap_2d::Footprint(), true);
  num_it = 0;
  EXPECT_THROW(a_star_2.createPath(path, num_it, tolerance), std::runtime_error);
  a_star_2.createGraph(costmapA->getSizeInCellsX(), costmapA->getSizeInCellsY(), 1, costmapA);
  num_it = 0;
  EXPECT_THROW(a_star_2.createPath(path, num_it, tolerance), std::runtime_error);
  a_star_2.setStart(50, 50, 0);  // invalid
  a_star_2.setGoal(0, 0, 0);     // valid
  num_it = 0;
  EXPECT_THROW(a_star_2.createPath(path, num_it, tolerance), std::runtime_error);
  a_star_2.setStart(0, 0, 0);   // valid
  a_star_2.setGoal(50, 50, 0);  // invalid
  num_it = 0;
  EXPECT_THROW(a_star_2.createPath(path, num_it, tolerance), std::runtime_error);
  num_it = 0;
  // invalid goal but liberal tolerance
  a_star_2.setStart(20, 20, 0);  // valid
  a_star_2.setGoal(50, 50, 0);   // invalid
  EXPECT_TRUE(a_star_2.createPath(path, num_it, some_tolerance));
  EXPECT_EQ(path.size(), 32u);
  for (unsigned int i = 0; i != path.size(); i++) {
    EXPECT_EQ(costmapA->getCost(path[i].x, path[i].y), 0);
  }

  EXPECT_TRUE(a_star_2.getStart() != nullptr);
  EXPECT_TRUE(a_star_2.getGoal() != nullptr);
  EXPECT_EQ(a_star_2.getSizeX(), 100u);
  EXPECT_EQ(a_star_2.getSizeY(), 100u);
  EXPECT_EQ(a_star_2.getSizeDim3(), 1u);
  EXPECT_EQ(a_star_2.getToleranceHeuristic(), 1000.0);
  EXPECT_EQ(a_star_2.getOnApproachMaxIterations(), 10);

  delete costmapA;
}

TEST(AStarTest, test_a_star_se2)
{
  smac_planner::SearchInfo info;
  info.change_penalty = 1.2;
  info.non_straight_penalty = 1.4;
  info.reverse_penalty = 2.1;
  info.minimum_turning_radius = 2.0;  // in grid coordinates
  unsigned int size_theta = 72;
  smac_planner::AStarAlgorithm<smac_planner::NodeSE2> a_star(
    smac_planner::MotionModel::DUBIN, info);
  int max_iterations = 10000;
  float tolerance = 10.0;
  int it_on_approach = 10;
  int num_it = 0;

  a_star.initialize(false, max_iterations, it_on_approach);
  a_star.setFootprint(nav2_costmap_2d::Footprint(), true);

  nav2_costmap_2d::Costmap2D * costmapA =
    new nav2_costmap_2d::Costmap2D(100, 100, 0.1, 0.0, 0.0, 0);
  // island in the middle of lethal cost to cross
  for (unsigned int i = 40; i <= 60; ++i) {
    for (unsigned int j = 40; j <= 60; ++j) {
      costmapA->setCost(i, j, 254);
    }
  }

  // functional case testing
  a_star.createGraph(
    costmapA->getSizeInCellsX(), costmapA->getSizeInCellsY(), size_theta, costmapA);
  a_star.setStart(10u, 10u, 0u);
  a_star.setGoal(80u, 80u, 40u);
  smac_planner::NodeSE2::CoordinateVector path;
  EXPECT_TRUE(a_star.createPath(path, num_it, tolerance));

  // check path is the right size and collision free
  EXPECT_EQ(num_it, 44);
  EXPECT_EQ(path.size(), 75u);
  for (unsigned int i = 0; i != path.size(); i++) {
    EXPECT_EQ(costmapA->getCost(path[i].x, path[i].y), 0);
  }

  delete costmapA;
}

TEST(AStarTest, test_constants)
{
  smac_planner::MotionModel mm = smac_planner::MotionModel::UNKNOWN;  // unknown
  EXPECT_EQ(smac_planner::toString(mm), std::string("Unknown"));
  mm = smac_planner::MotionModel::VON_NEUMANN;  // vonneumann
  EXPECT_EQ(smac_planner::toString(mm), std::string("Von Neumann"));
  mm = smac_planner::MotionModel::MOORE;  // moore
  EXPECT_EQ(smac_planner::toString(mm), std::string("Moore"));
  mm = smac_planner::MotionModel::DUBIN;  // dubin
  EXPECT_EQ(smac_planner::toString(mm), std::string("Dubin"));
  mm = smac_planner::MotionModel::REEDS_SHEPP;  // reeds-shepp
  EXPECT_EQ(smac_planner::toString(mm), std::string("Reeds-Shepp"));

  EXPECT_EQ(smac_planner::fromString("VON_NEUMANN"), smac_planner::MotionModel::VON_NEUMANN);
  EXPECT_EQ(smac_planner::fromString("MOORE"), smac_planner::MotionModel::MOORE);
  EXPECT_EQ(smac_planner::fromString("DUBIN"), smac_planner::MotionModel::DUBIN);
  EXPECT_EQ(smac_planner::fromString("REEDS_SHEPP"), smac_planner::MotionModel::REEDS_SHEPP);
  EXPECT_EQ(smac_planner::fromString("NONE"), smac_planner::MotionModel::UNKNOWN);
}

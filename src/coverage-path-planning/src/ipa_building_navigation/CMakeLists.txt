cmake_minimum_required(VERSION 2.8.3)
project(ipa_building_navigation)

set(catkin_RUN_PACKAGES
	actionlib
	cv_bridge
	geometry_msgs
	ipa_building_msgs
	#libconcorde_tsp_solver
	roscpp
	roslib
	sensor_msgs
	visualization_msgs
)

set(catkin_BUILD_PACKAGES
	${catkin_RUN_PACKAGES}
	dynamic_reconfigure
)

## Find catkin macros and libraries
find_package(catkin REQUIRED COMPONENTS
	${catkin_BUILD_PACKAGES}
)

generate_dynamic_reconfigure_options(
  cfg/BuildingNavigation.cfg
)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread chrono)

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
# important: common/include needs to be here if you have such a directory
catkin_package(
INCLUDE_DIRS
	common/include
	ros/include
LIBRARIES
	tsp_solvers
CATKIN_DEPENDS
	${catkin_RUN_PACKAGES}
DEPENDS
#	libconcorde_tsp_solver
	OpenCV
	Boost
)


###########
## Build ##
###########
## Specify additional locations of header files
## Your package locations should be listed before other locations
# include_directories(include ${catkin_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
# important: common/include also needs to be here if you have it
include_directories(
	common/include
	ros/include
	${catkin_INCLUDE_DIRS}
	${OpenCV_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

# TSP library
add_library(tsp_solvers
	common/src/A_star_pathplanner.cpp
	common/src/node.cpp
	common/src/nearest_neighbor_TSP.cpp
	common/src/genetic_TSP.cpp
	common/src/concorde_TSP.cpp
)
target_link_libraries(tsp_solvers
	${catkin_LIBRARIES}
	${OpenCV_LIBRARIES}
	${Boost_LIBRARIES}
)
add_dependencies(tsp_solvers
	${catkin_EXPORTED_TARGETS}
	${${PROJECT_NAME}_EXPORTED_TARGETS}
)

# action provider
add_executable(room_sequence_planning_server
	ros/src/room_sequence_planning_action_server.cpp
	common/src/maximal_clique_finder.cpp
	common/src/set_cover_solver.cpp
	common/src/trolley_position_finder.cpp
)
target_link_libraries(room_sequence_planning_server
	tsp_solvers
	${catkin_LIBRARIES}
	${OpenCV_LIBRARIES}
	${Boost_LIBRARIES}
)
add_dependencies(room_sequence_planning_server
	${catkin_EXPORTED_TARGETS}
	${${PROJECT_NAME}_EXPORTED_TARGETS}
	${PROJECT_NAME}_gencfg
)

# client for testing purpose
add_executable(room_sequence_planning_client ros/src/room_sequence_planning_action_client.cpp)
target_link_libraries(room_sequence_planning_client
	${catkin_LIBRARIES} 
	${OpenCV_LIBRARIES}
)
add_dependencies(room_sequence_planning_client ${catkin_EXPORTED_TARGETS} ${${PROJECT_NAME}_EXPORTED_TARGETS})

# client for testing purpose
add_executable(room_sequence_planning_evaluation 
	ros/src/room_sequence_planning_evaluation.cpp
	common/src/A_star_pathplanner.cpp
	common/src/node.cpp
)
target_link_libraries(room_sequence_planning_evaluation
	${catkin_LIBRARIES}
	${OpenCV_LIBRARIES}
)
add_dependencies(room_sequence_planning_evaluation ${catkin_EXPORTED_TARGETS} ${${PROJECT_NAME}_EXPORTED_TARGETS})

# evaluation
add_executable(TSP_evaluation
	ros/src/boosttest.cpp
)
target_link_libraries(TSP_evaluation
	tsp_solvers
	${catkin_LIBRARIES}
	${OpenCV_LIBRARIES}
)
add_dependencies(TSP_evaluation ${catkin_EXPORTED_TARGETS} ${${PROJECT_NAME}_EXPORTED_TARGETS})

#tester for different functions
#add_executable(a_star_tester ros/src/tester.cpp common/src/A_star_pathplanner.cpp common/src/node.cpp common/src/nearest_neighbor_TSP.cpp common/src/genetic_TSP.cpp common/src/concorde_TSP.cpp common/src/maximal_clique_finder.cpp common/src/set_cover_solver.cpp common/src/trolley_position_finder.cpp)
#target_link_libraries(a_star_tester ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})
#add_dependencies(a_star_tester ${catkin_EXPORTED_TARGETS} ${${PROJECT_NAME}_EXPORTED_TARGETS})


#############
## Install ##
#############
# Mark executables and/or libraries for installation
install(TARGETS room_sequence_planning_server room_sequence_planning_client tsp_solvers #a_star_tester tester
	ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

#uncomment this if you have a common-directory with header-files
install(DIRECTORY common/include/${PROJECT_NAME}/
	DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
	FILES_MATCHING PATTERN "*.h"
	PATTERN ".svn" EXCLUDE
)

#uncomment this if you have header-files in your project
install(DIRECTORY ros/include/${PROJECT_NAME}/
	DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
	FILES_MATCHING PATTERN "*.h"
	PATTERN ".svn" EXCLUDE
)

#install(DIRECTORY scripts
#	DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
#	PATTERN ".svn" EXCLUDE
#)

##uncomment this if you use launch and yaml files
install(DIRECTORY launch
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(DIRECTORY ros/test
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/ros
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY config/
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/config
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY cfg/
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/cfg
        PATTERN ".svn" EXCLUDE
)




#install(DIRECTORY common/files
#	DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/common
#	PATTERN ".svn" EXCLUDE
#)

install(CODE "execute_process(COMMAND linuxenv --cfg:../envconfig1.cfgx)")
install(CODE "execute_process(COMMAND linuxenv --cfg:../envconfig2.cfgx)")
install(CODE "execute_process(COMMAND linuxenv --cfg:../envconfig3.cfgx)")

option(CONFIGURE_DEBIAN_PACKAGE "Configure debian package" OFF)
if (CONFIGURE_DEBIAN_PACKAGE)
  string(REGEX MATCH "^.*/seirios_ros" SEIRIOS_ROOT_DIR ${PROJECT_SOURCE_DIR})
  include(${SEIRIOS_ROOT_DIR}/debian.cmake)
endif()
cmake_minimum_required(VERSION 3.16.3)
project(velocity_setter LANGUAGES CXX)

include(${PROJECT_SOURCE_DIR}/cmake/colorize.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/cxxStandard.cmake)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  ros_utils
  roslib
  std_msgs
  message_generation
  dynamic_reconfigure
  movel_seirios_msgs
  movel_hasp_vendor
  rostest
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES velocity_setter
  CATKIN_DEPENDS roscpp message_runtime
)

if ("$ENV{MOVEL_LICENSE}" STREQUAL "KQAYZ")
      add_definitions(-DMOVEL_LICENSE)
endif()

if ("$ENV{MOVEL_LICENSE}" STREQUAL "RWBBX")
      add_definitions(-DMOVEL_LICENSE)
endif()

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(velocity_setter src/velocity_setter.cpp)
target_link_libraries(velocity_setter ${catkin_LIBRARIES})
add_dependencies(velocity_setter ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

add_executable(velocity_setter_node src/velocity_setter_node.cpp)
target_link_libraries(velocity_setter_node ${catkin_LIBRARIES} velocity_setter)
#add_dependencies(velocity_setter_node velocity_setter_generate_messages_cpp)

if (CATKIN_ENABLE_TESTING)
  add_rostest_gtest(test_velocity_setter test/test_velocity_setter.test test/test_velocity_setter.cpp)
  add_rostest_gtest(test_velocity_setter_node test/test_velocity_setter_node.test test/test_velocity_setter_node.cpp)

  target_link_libraries(test_velocity_setter ${catkin_LIBRARIES} velocity_setter)
  target_link_libraries(test_velocity_setter_node ${catkin_LIBRARIES} velocity_setter)
endif()

install(TARGETS velocity_setter
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)

install(TARGETS velocity_setter_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)


install(DIRECTORY include/velocity_setter/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  PATTERN ".svn" EXCLUDE
)


install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
)

install(DIRECTORY config/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/config
)


install(CODE "execute_process(COMMAND linuxenv -v:/home/$ENV{USER}/vendor.hvc --cfg:../envconfig1.cfgx)")
install(CODE "execute_process(COMMAND linuxenv -v:/home/$ENV{USER}/vendor.hvc --cfg:../envconfig2.cfgx)")

option(CONFIGURE_DEBIAN_PACKAGE "Configure debian package" OFF)
if (CONFIGURE_DEBIAN_PACKAGE)
  string(REGEX MATCH "^.*/seirios_ros" SEIRIOS_ROOT_DIR ${PROJECT_SOURCE_DIR})
  include(${SEIRIOS_ROOT_DIR}/debian.cmake)
endif()
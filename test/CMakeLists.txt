#cmake file for project association
#author: liuwenjun
#created:2014/02/20

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

## 添加项目

ADD_EXECUTABLE(sample sample.cpp glad.c)	
target_link_libraries(sample platform glfw3)

if(MSVC)
set(WINTEST sample)
set_target_properties(${WINTEST} PROPERTIES FOLDER "tests/lac")
endif()
install(DIRECTORY ../src/ DESTINATION include/lac
         FILES_MATCHING PATTERN "*.hpp")
#cmake file for project association
#author: liuwenjun
#created:2014/02/20

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

## 添加项目

install(DIRECTORY ./ DESTINATION include/lac
         FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ./ DESTINATION include/lac
         FILES_MATCHING PATTERN "*.hpp")
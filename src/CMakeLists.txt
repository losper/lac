#cmake file for project association
#author: liuwenjun
#created:2014/02/20

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

## 添加项目

if(MSVC)
	ADD_LIBRARY(js SHARED duktape.c js.cpp duktape.h duk_config.h js.h duk_module_duktape.h duk_module_duktape.c export.def)	
	target_link_libraries(js platform parser)
else()
	ADD_LIBRARY(js SHARED duktape.c js.cpp duktape.h duk_config.h js.h duk_module_duktape.h duk_module_duktape.c)
	target_link_libraries(js platform parser)
endif()

install(TARGETS js
RUNTIME DESTINATION bin
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib
)
install(DIRECTORY ./ DESTINATION include/js
         FILES_MATCHING PATTERN "*.h")
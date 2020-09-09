cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( CommonTests )

get_filename_component( APP_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE )

if( NOT DEFINED CINDER_PATH )
	if( NOT DEFINED ENV{CINDER_PATH} )
		message( FATAL_ERROR "NO VALID CINDER PATH" )
	else()
		if( NOT IS_ABSOLUTE $ENV{CINDER_PATH} )
			get_filename_component( CINDER_PATH "$ENV{CINDER_PATH}" REALPATH BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}" ) 
		endif()
	endif()
else()
	if( NOT IS_ABSOLUTE ${CINDER_PATH} )
		get_filename_component( CINDER_PATH "${CINDER_PATH}" REALPATH BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}" ) 
	endif()
	message( STATUS "Cinder path: " ${CINDER_PATH} )
endif()

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

# RIRTest
message( STATUS "Building RIR test app." )
ci_make_app(
	APP_NAME	RIRTest
	SOURCES     ${APP_PATH}/RIRApp.cpp
	CINDER_PATH ${CINDER_PATH}
)

if( NOT TARGET Common )
	find_package( Common REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../proj/cmake" NO_DEFAULT_PATH )
endif()

add_dependencies( RIRTest Common )
target_link_libraries( RIRTest Common )

get_target_property( RIR_INC Common INCLUDE_DIRECTORIES )
# FXs
if( DEFINED BUILD_FX_TESTS )
	message( STATUS "Building Fx test app." )
	ci_make_app(
		APP_NAME	FxTest
		SOURCES     ${APP_PATH}/FxApp.cpp
		CINDER_PATH ${CINDER_PATH}
	)
	if( NOT TARGET Common )
		find_package( Common REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/../cmake" NO_DEFAULT_PATH )
	endif()
	target_link_libraries( FxTest Common )
endif()

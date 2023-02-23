cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

project( Common )

option( BUILD_TESTS "Build Common tests" OFF )

if( NOT DEFINED CINDER_PATH )
	if( NOT DEFINED ENV{CINDER_PATH} )
		message( FATAL_ERROR "NO VALID CINDER PATH" )
	else()
		if( NOT IS_ABSOLUTE $ENV{CINDER_PATH} )
			get_filename_component( CINDER_PATH "$ENV{CINDER_PATH}" REALPATH BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}" ) 
		else()
			set( CINDER_PATH "$ENV{CINDER_PATH}" )	
		endif()
	
	endif()
else()
	if( NOT IS_ABSOLUTE ${CINDER_PATH} )
		get_filename_component( CINDER_PATH "${CINDER_PATH}" REALPATH BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}" ) 
	endif()
endif()

set( SKIP_ASSETS_COPY TRUE INTERNAL )
find_package( Common REQUIRED PATHS
	"${CMAKE_CURRENT_LIST_DIR}/proj/cmake"
)

if( BUILD_TESTS MATCHES ON )
	add_subdirectory( tests )
endif()

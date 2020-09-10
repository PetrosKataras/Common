if( NOT TARGET Common )
	option( DISABLE_FX_MODULE "Build Common with support for the FX's." OFF )
	get_filename_component( COMMON_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	get_filename_component( COMMON_ASSETS_PATH "${CMAKE_CURRENT_LIST_DIR}/../../assets" ABSOLUTE )
	# Find Cinder
	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
			"$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		)
	endif()
	# Common
	list( APPEND COMMON_SOURCES ${COMMON_SOURCE_PATH}/RIR/IndieResolutionRenderer.cpp )
	add_library( Common STATIC ${COMMON_SOURCES} )
	target_include_directories( Common PUBLIC "${COMMON_SOURCE_PATH}" )
	# Fx
	if( DISABLE_FX_MODULE MATCHES OFF )
		list( APPEND FX_SOURCES 
			${COMMON_SOURCE_PATH}/Fx/ColorCorrectionFx.cpp 
			${COMMON_SOURCE_PATH}/Fx/DoFFx.cpp 
			${COMMON_SOURCE_PATH}/Fx/FogFx.cpp 
			${COMMON_SOURCE_PATH}/Fx/FxChain.cpp 
			${COMMON_SOURCE_PATH}/Fx/VignetteFx.cpp 
		)
		add_library( Common_FX OBJECT ${FX_SOURCES} )
		target_link_libraries( Common_FX PRIVATE cinder )
		target_include_directories( Common_FX PRIVATE "${COMMON_SOURCE_PATH}" )
		target_link_libraries( Common PUBLIC Common_FX )
		set( BUILD_FX_TESTS TRUE )
	endif()
	target_link_libraries( Common PRIVATE cinder )
	if( COMMON_ASSETS_DEST_PATH AND IS_DIRECTORY "${COMMON_ASSETS_DEST_PATH}" )
		add_custom_command( TARGET Common POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${COMMON_ASSETS_PATH} ${COMMON_ASSETS_DEST_PATH}
		)
	elseif( BUILD_TESTS MATCHES OFF ) # if COMMON_ASSETS_DEST_PATH is not valid and we are not building the tests issue a warning
		message( WARNING "Destination path for common assets is either not set or not a valid directory!" )
	endif()
endif()

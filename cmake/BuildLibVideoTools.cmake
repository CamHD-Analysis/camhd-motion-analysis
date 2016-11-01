

IF( DEFINED LOCAL_LIBVIDEO_TOOLS )
	message( "Building local copy of LibVideoTools")

	set( LIBVIDEO_TOOLS_DIR ${LOCAL_LIBVIDEO_TOOLS} )

	include( ${LOCAL_LIBVIDEO_TOOLS}/cmake/BuildG3Log.cmake )
	include( ${LOCAL_LIBVIDEO_TOOLS}/cmake/BuildLibLogger.cmake )


	include_directories( ${EIGEN3_INCLUDE_DIR}
												${LOCAL_LIBVIDEO_TOOLS}/include/libvideo_tools/ )
	add_subdirectory( ${LOCAL_LIBVIDEO_TOOLS}/lib libvideo_tools )

	SET( LIBVIDEO_TOOLS_INCLUDE_DIRS
				${LIBVIDEO_TOOLS_INCLUDE_DIRS}
				${LIBLOGGER_INCLUDE_DIRS}
				${LIBVIDEO_TOOLS_DIR}/include
	 		)



ELSE()

	message("Building LibVideoTools from Git.")
	SET( LIBVIDEO_TOOLS_PREFIX_DIR ${PROJECT_BINARY_DIR}/libvideo_tools )
	SET( LIBVIDEO_TOOLS_INSTALL_DIR ${LIBVIDEO_TOOLS_PREFIX_DIR} )

 SET( LIBVIDEO_TOOLS_CMAKE_OPTS  )
	IF( DEFINED CMAKE_BUILD_TYPE )
		LIST(APPEND LIBVIDEO_TOOLS_CMAKE_OPTS -DCMAKE_BUILD_TYPE:string=${CMAKE_BUILD_TYPE} )
	ENDIF()

	LIST(APPEND LIBVIDEO_TOOLS_CMAKE_OPTS -DBUILD_UNIT_TESTS:bool=false)

	include_directories( ${G3LOG_INCLUDE_DIR} )

	ExternalProject_Add( libvideo_tools
											https://github.com/amarburg/libvideo_tools
											PREFIX libvideo_tools
											UPDATE_COMMAND git pull origin master
											BUILD_COMMAND make deps all
											CMAKE_ARGS ${LIBVIDEO_TOOLS_CMAKE_OPTS}
	  									INSTALL_COMMAND "" )


	set_target_properties(libvideo_tools PROPERTIES EXCLUDE_FROM_ALL TRUE)
	set( LIBVIDEO_TOOLS_INCLUDE_DIRS ${LIBVIDEO_TOOLS_INSTALL_DIR}/src/libvideo_tools/include )
						# ${LIBVIDEO_TOOLS_INSTALL_DIR}/src/liblogger-build/libactive_object/src/libactive_object/include
						# ${LIBVIDEO_TOOLS_INSTALL_DIR}/src/liblogger-build/google-snappy/include  )
	set( LIBVIDEO_TOOLS_LIB_DIR ${LIBVIDEO_TOOLS_INSTALL_DIR}/src/libvideo_tools-build/lib )

	link_directories(
	  ${LIBVIDEO_TOOLS_LIB_DIR}
	)

	list(APPEND EXTERNAL_PROJECTS libvideo_tools )


ENDIF()

SET( LIBVIDEO_TOOLS_LIBS logger active_object snappy )

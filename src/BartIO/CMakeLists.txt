project(BartIO)

include_directories(${TOOLBOX_PATH}/src)

link_directories(${TOOLBOX_PATH}/lib)

set(SOURCE_FILES
	BartIO.cpp
	BartIO.h
	)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})


target_link_libraries(${PROJECT_NAME} num)
target_link_libraries(${PROJECT_NAME} misc)
target_link_libraries(${PROJECT_NAME} fftw3f)
target_link_libraries(${PROJECT_NAME} fftw3f_threads)
target_link_libraries(${PROJECT_NAME} openblas)
target_link_libraries(${PROJECT_NAME} lapacke)



target_link_libraries(${PROJECT_NAME} Arc)
target_link_libraries(${PROJECT_NAME} Cartesian2D)
target_link_libraries(${PROJECT_NAME} Cartesian3D)
target_link_libraries(${PROJECT_NAME} Gradwarp)
target_link_libraries(${PROJECT_NAME} Legacy)
target_link_libraries(${PROJECT_NAME} Core)
target_link_libraries(${PROJECT_NAME} CalibrationCommon)
target_link_libraries(${PROJECT_NAME} Control)
target_link_libraries(${PROJECT_NAME} Common)
target_link_libraries(${PROJECT_NAME} Crucial)
target_link_libraries(${PROJECT_NAME} Dicom)
target_link_libraries(${PROJECT_NAME} Hdf5)
target_link_libraries(${PROJECT_NAME} Math)
target_link_libraries(${PROJECT_NAME} System)
target_link_libraries(${PROJECT_NAME} ${OX_3P_LIBS})
target_link_libraries(${PROJECT_NAME} ${OX_OS_LIBS})

# Install this example rehearsal code along with this CMakeLists.txt file
install(FILES ${SOURCE_FILES} DESTINATION "src/BartIO")
install(FILES "CMakeLists.txt" DESTINATION "src/BartIO")



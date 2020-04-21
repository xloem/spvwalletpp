cmake_minimum_required (VERSION 3.10) # likely lower, haven't tested

project (spvwallet++ CXX)

set (SPVWALLETPP_LIBRARIES spvwallet++ CACHE INTERNAL "")
set (SPVWALLETPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

#add_subdirectory(dependencies/grpc)

### begin go stuff
find_library (golang REQUIRED)
set (GOPATH ${CMAKE_CURRENT_BINARY_DIR}/go)
set (SPVWALLET_BINARY ${GOPATH}/bin/spvwallet)
add_custom_command(OUTPUT ${SPVWALLET_BINARY}
	DEPENDS dependencies/spvwallet-go/cmd/spvwallet/main.go
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/spvwallet-go
	COMMAND env GOPATH=${GOPATH} go get .
	COMMAND env GOPATH=${GOPATH} go get github.com/OpenBazaar/spvwallet/cmd/spvwallet
	COMMENT "Building go cmd spvwallet ...")
#add_custom_target(spvwallet DEPENDS ${SPVWALLET_BINARY})
### end go stuff

#add_executable (test test.cpp)
#target_link_libraries (test ${CPR_LIBRARIES})

add_library (${SPVWALLETPP_LIBRARIES} source/spvwallet.cpp "${SPVWALLETPP_INCLUDE_DIRS}/spvwallet.hpp" ${SPVWALLET_BINARY})

target_link_libraries (${SPVWALLETPP_LIBRARIES} ${CPR_LIBRARIES})
target_include_directories (${SPVWALLETPP_LIBRARIES} PRIVATE ${JSON_INCLUDE_DIRS} PRIVATE ${CPR_INCLUDE_DIRS} PRIVATE ${SPVWALLETPP_INCLUDE_DIRS})

install (TARGETS ${SPVWALLETPP_LIBRARIES} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install (FILES include/siaskynet.hpp DESTINATION include)

add_executable (spvwallet++_example example.cpp)
target_link_libraries (spvwallet++_example ${SPVWALLETPP_LIBRARIES})
target_include_directories (spvwallet++_example PRIVATE ${SPVWALLETPP_INCLUDE_DIRS})

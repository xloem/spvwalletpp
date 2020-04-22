cmake_minimum_required (VERSION 3.10) # likely lower, haven't tested

project (spvwallet++ CXX)

set (SPVWALLETPP_LIBRARIES spvwallet++ CACHE INTERNAL "")
set (SPVWALLETPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

### spvwallet-go
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

### json
set (JSON_BuildTests OFF CACHE INTERNAL "")
set (JSON_Install OFF CACHE INTERNAL "")
add_subdirectory (dependencies/json)

### cpp-subprocess
set (BUILD_TESTING OFF CACHE INTERNAL "")
add_subdirectory (dependencies/cpp-subprocess)
set (CPPSUBPROCESS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cpp-subprocess)
find_package (Threads)
set (CPPSUBPROCESS_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

### sqlitecpp
add_subdirectory(dependencies/SQLiteCpp)
set (SQLITECPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SQLiteCpp/include)
set (SQLITECPP_LIBRARIES SQLiteCpp sqlite3 dl)

add_library (${SPVWALLETPP_LIBRARIES} source/spvwallet.cpp "${SPVWALLETPP_INCLUDE_DIRS}/spvwallet.hpp" ${SPVWALLET_BINARY})

#target_link_libraries (${SPVWALLETPP_LIBRARIES} ${CPR_LIBRARIES})
target_link_libraries (${SPVWALLETPP_LIBRARIES} PRIVATE nlohmann_json::nlohmann_json PRIVATE ${CPPSUBPROCESS_LIBRARIES} PRIVATE ${SQLITECPP_LIBRARIES})
#target_include_directories (${SPVWALLETPP_LIBRARIES} PRIVATE ${CPR_INCLUDE_DIRS})
target_include_directories (${SPVWALLETPP_LIBRARIES} PRIVATE ${CPPSUBPROCESS_INCLUDE_DIRS} PRIVATE ${SPVWALLETPP_INCLUDE_DIRS} PRIVATE ${SQLITECPP_INCLUDE_DIRS})

install (TARGETS ${SPVWALLETPP_LIBRARIES} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install (FILES include/siaskynet.hpp DESTINATION include)

add_executable (spvwallet++_example example.cpp)
target_link_libraries (spvwallet++_example ${SPVWALLETPP_LIBRARIES})
target_include_directories (spvwallet++_example PRIVATE ${SPVWALLETPP_INCLUDE_DIRS})

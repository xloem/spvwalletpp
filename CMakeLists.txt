cmake_minimum_required (VERSION 3.10)

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
add_subdirectory (dependencies/cpp-subprocess EXCLUDE_FROM_ALL)
set (CPPSUBPROCESS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cpp-subprocess)
find_package (Threads)
set (CPPSUBPROCESS_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

### sqlitecpp
add_subdirectory(dependencies/SQLiteCpp EXCLUDE_FROM_ALL)
set (SQLITECPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SQLiteCpp/include)
set (SQLITECPP_LIBRARIES SQLiteCpp sqlite3 dl)

### MAIN: libspvwallet++
add_library (${SPVWALLETPP_LIBRARIES} SHARED source/spvwallet.cpp source/spvwallet_database.cpp source/spvwallet_logic.cpp "${SPVWALLETPP_INCLUDE_DIRS}/spvwallet.hpp" ${SPVWALLET_BINARY})
install (PROGRAMS ${SPVWALLET_BINARY} DESTINATION ${CMAKE_INSTALL_LIBDIR} RENAME spvwallet++.go)
install (TARGETS ${SPVWALLETPP_LIBRARIES} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_definitions(-DSPVWALLET_BINARY="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/spvwallet++.go")

target_link_libraries (${SPVWALLETPP_LIBRARIES} PRIVATE nlohmann_json::nlohmann_json ${CPPSUBPROCESS_LIBRARIES} ${SQLITECPP_LIBRARIES})
target_include_directories (${SPVWALLETPP_LIBRARIES} PRIVATE ${CPPSUBPROCESS_INCLUDE_DIRS} ${SPVWALLETPP_INCLUDE_DIRS} ${SQLITECPP_INCLUDE_DIRS})

### example
add_executable (spvwallet++_example example.cpp)
target_link_libraries (spvwallet++_example ${SPVWALLETPP_LIBRARIES})
target_include_directories (spvwallet++_example PRIVATE ${SPVWALLETPP_INCLUDE_DIRS})

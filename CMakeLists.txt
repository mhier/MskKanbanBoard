cmake_minimum_required(VERSION 2.6)
project(MskKanbanBoard)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 00)
set(${PROJECT_NAME}_PATCH_VERSION 03)
include(cmake/set_version_numbers.cmake)

include(cmake/enable_latest_cxx_support.cmake)

include(GNUInstallDirs)

set(WT_CONNECTOR "Wt::HTTP" CACHE STRING "Connector used (Wt::HTTP or Wt::FCGI)")

set(TARGET MskKanbanBoard.wt)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

find_package(Boost REQUIRED COMPONENTS program_options system filesystem thread)
find_package(wt REQUIRED COMPONENTS http dbo dbosqlite3 dbomysql)
find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_FULL_DATAROOTDIR}/Wt/resources ${CMAKE_BINARY_DIR}/resources)

configure_file(${CMAKE_SOURCE_DIR}/MskKanbanBoard ${CMAKE_BINARY_DIR}/FormalisedLog COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/MskKanbanBoard.css ${CMAKE_BINARY_DIR}/FormalisedLog COPYONLY)

aux_source_directory(${CMAKE_SOURCE_DIR}/src SOURCE)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${TARGET} ${SOURCE})
target_link_libraries(${TARGET} ${WT_CONNECTOR} Wt::DboMySQL Wt::DboSqlite3 ${Boost_LIBRARIES} ${CURL_LIBRARIES})

configure_file(include/Version.h.in ${PROJECT_BINARY_DIR}/version/Version.h)
include_directories(${PROJECT_BINARY_DIR}/version)

set(APPROOT "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${PROJECT_NAME}")
configure_file(${PROJECT_NAME}.service.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.service)


FILE( COPY ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.css DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.cfg.template DESTINATION ${PROJECT_BINARY_DIR})
FILE( COPY ${CMAKE_SOURCE_DIR}/${PROJECT_NAME} DESTINATION ${PROJECT_BINARY_DIR}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.service DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/systemd/system)
install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.css DESTINATION ${APPROOT})
install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.cfg.template DESTINATION ${APPROOT})
install(CODE "EXECUTE_PROCESS(COMMAND ln -sf ${CMAKE_INSTALL_FULL_DATAROOTDIR}/Wt/resources ${APPROOT} WORKING_DIRECTORY ${PREFIX_INCLUDE} )")

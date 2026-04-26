cmake_minimum_required(VERSION 3.27)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/backend/TransactionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/pacman/PacmanBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/pacman/AlpmWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/aur/AurBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/aur/AurClient.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/aur/AurBuilder.cpp
    ${CMAKE_SOURCE_DIR}/src/backend/flatpak/FlatpakBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/models/PackageListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/InstalledModel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/UpdatesModel.cpp
    ${CMAKE_SOURCE_DIR}/src/service/UpdateChecker.cpp
)

set(TEST_HEADERS
    ${CMAKE_SOURCE_DIR}/src/backend/Package.h
    ${CMAKE_SOURCE_DIR}/src/backend/IPackageBackend.h
    ${CMAKE_SOURCE_DIR}/src/backend/TransactionManager.h
    ${CMAKE_SOURCE_DIR}/src/backend/pacman/PacmanBackend.h
    ${CMAKE_SOURCE_DIR}/src/backend/pacman/AlpmWrapper.h
    ${CMAKE_SOURCE_DIR}/src/backend/aur/AurBackend.h
    ${CMAKE_SOURCE_DIR}/src/backend/aur/AurClient.h
    ${CMAKE_SOURCE_DIR}/src/backend/aur/AurBuilder.h
    ${CMAKE_SOURCE_DIR}/src/backend/flatpak/FlatpakBackend.h
    ${CMAKE_SOURCE_DIR}/src/models/PackageListModel.h
    ${CMAKE_SOURCE_DIR}/src/models/InstalledModel.h
    ${CMAKE_SOURCE_DIR}/src/models/UpdatesModel.h
    ${CMAKE_SOURCE_DIR}/src/service/UpdateChecker.h
)

function(add_backend_test name source)
    add_executable(${name} ${source} ${TEST_SOURCES} ${TEST_HEADERS})
    target_include_directories(${name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/backend
        ${CMAKE_SOURCE_DIR}/src/backend/pacman
        ${CMAKE_SOURCE_DIR}/src/backend/aur
        ${CMAKE_SOURCE_DIR}/src/backend/flatpak
        ${CMAKE_SOURCE_DIR}/src/models
        ${CMAKE_SOURCE_DIR}/src/service
        ${ALPM_INCLUDE_DIRS}
    )
    target_link_libraries(${name} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Network
        Qt6::Test
        ${ALPM_LIBRARIES}
    )
    target_compile_definitions(${name} PRIVATE QT_TESTLIB_LIB)
    if(FLATPAK_FOUND)
        target_include_directories(${name} PRIVATE ${FLATPAK_INCLUDE_DIRS})
        target_link_libraries(${name} PRIVATE ${FLATPAK_LIBRARIES})
        target_compile_definitions(${name} PRIVATE HAVE_FLATPAK=1)
    else()
        target_compile_definitions(${name} PRIVATE HAVE_FLATPAK=0)
    endif()
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_backend_test(test_pacman test_pacman.cpp)
add_backend_test(test_aur test_aur.cpp)
add_backend_test(test_flatpak test_flatpak.cpp)
add_backend_test(test_models models/test_packagelistmodel.cpp)
add_backend_test(test_transaction test_transaction.cpp)

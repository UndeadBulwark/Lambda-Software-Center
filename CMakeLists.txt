cmake_minimum_required(VERSION 3.27)
project(LambdaSoftwareCenter VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Network Test)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALPM REQUIRED libalpm)
pkg_check_modules(FLATPAK libflatpak)

set(SOURCES
    src/main.cpp
    src/backend/TransactionManager.cpp
    src/backend/pacman/PacmanBackend.cpp
    src/backend/pacman/AlpmWrapper.cpp
    src/backend/aur/AurBackend.cpp
    src/backend/aur/AurClient.cpp
    src/backend/aur/AurBuilder.cpp
    src/backend/flatpak/FlatpakBackend.cpp
    src/models/PackageListModel.cpp
    src/models/InstalledModel.cpp
    src/models/UpdatesModel.cpp
    src/service/UpdateChecker.cpp
)

set(HEADERS
    src/backend/Package.h
    src/backend/IPackageBackend.h
    src/backend/TransactionManager.h
    src/backend/pacman/PacmanBackend.h
    src/backend/pacman/AlpmWrapper.h
    src/backend/aur/AurBackend.h
    src/backend/aur/AurClient.h
    src/backend/aur/AurBuilder.h
    src/backend/flatpak/FlatpakBackend.h
    src/models/PackageListModel.h
    src/models/InstalledModel.h
    src/models/UpdatesModel.h
    src/service/UpdateChecker.h
)

qt_add_executable(lsc_app ${SOURCES} ${HEADERS})
set_target_properties(lsc_app PROPERTIES OUTPUT_NAME "lambda-software-center")
target_include_directories(lsc_app PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/backend
    ${CMAKE_SOURCE_DIR}/src/backend/pacman
    ${CMAKE_SOURCE_DIR}/src/backend/aur
    ${CMAKE_SOURCE_DIR}/src/backend/flatpak
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/service
    ${ALPM_INCLUDE_DIRS}
)

target_link_libraries(lsc_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    ${ALPM_LIBRARIES}
)

if(FLATPAK_FOUND)
    target_include_directories(lsc_app PRIVATE ${FLATPAK_INCLUDE_DIRS})
    target_link_libraries(lsc_app PRIVATE ${FLATPAK_LIBRARIES})
    target_compile_definitions(lsc_app PRIVATE HAVE_FLATPAK=1)
else()
    message(WARNING "libflatpak not found. Flatpak backend will be stubbed.")
    target_compile_definitions(lsc_app PRIVATE HAVE_FLATPAK=0)
endif()

qt_add_qml_module(lsc_app
    URI LambdaSoftwareCenter
    VERSION 1.0
    RESOURCE_PREFIX "/"
    QML_FILES
        qml/main.qml
        qml/Theme.qml
        qml/components/Sidebar.qml
        qml/components/NavGroup.qml
        qml/components/NavItem.qml
        qml/components/Topbar.qml
        qml/components/SearchBar.qml
        qml/components/SourceTabs.qml
        qml/components/PackageCard.qml
        qml/components/FeaturedCard.qml
        qml/components/BadgePill.qml
        qml/components/InstallButton.qml
        qml/components/StatusBar.qml
        qml/components/UpdatesBanner.qml
        qml/components/ProgressDrawer.qml
        qml/components/ConfirmDialog.qml
        qml/pages/BrowsePage.qml
        qml/pages/FeaturedPage.qml
        qml/pages/RecentPage.qml
        qml/pages/InstalledPage.qml
        qml/pages/UpdatesPage.qml
        qml/pages/DetailPage.qml
    IMPORTS QtQuick QtQuick.Controls
)

enable_testing()
add_subdirectory(tests)

install(TARGETS lsc_app DESTINATION bin)
install(FILES data/com.lambdasc.policy DESTINATION share/polkit-1/actions)

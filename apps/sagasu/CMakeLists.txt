# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-sagasu)
target_sources(novus-sagasu
        PRIVATE
        include/hashdatabase.h

        src/hashdatabase.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/filetreewindow.cpp
        src/filepropertieswindow.cpp
        src/filetreemodel.cpp)
target_include_directories(novus-sagasu PRIVATE include)
target_link_libraries(novus-sagasu
        PRIVATE
        KF6::Archive
        Novus::Common
        Physis::Physis
        Qt6::Core
        Qt6::Sql
        Novus::SklbPart
        Novus::CmpPart
        Novus::ShpkPart
        Novus::HexPart
        Novus::ExlPart
        Novus::MdlPart
        Novus::ExdPart
        Novus::TexPart
        Novus::DicPart
        Novus::MtrlPart
        Novus::LuabPart
        Novus::ScenePart
        Novus::TmbPart
        Qt6::Concurrent
        Qt6::Network)

install(TARGETS novus-sagasu ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES dataexplorer.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/novus)

ecm_add_app_icon(novus-sagasu
        ICONS
        ${CMAKE_SOURCE_DIR}/resources/16-data.png
        ${CMAKE_SOURCE_DIR}/resources/32-data.png
        ${CMAKE_SOURCE_DIR}/resources/48-data.png
        ${CMAKE_SOURCE_DIR}/resources/256-data.png
)

if (WIN32)
    set_target_properties(novus-sagasu PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "DataExplorer")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-sagasu> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.sagasu.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.sagasu.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
endif()

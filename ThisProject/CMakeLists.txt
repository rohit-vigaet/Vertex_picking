# CMakeList.txt : CMake project for FaceCaputureAPP, include source and define
# project specific logic here.
cmake_minimum_required (VERSION 3.8)

find_package (glm CONFIG REQUIRED)
find_package (OpenGL REQUIRED)
find_package (GLUT   REQUIRED)
find_package (PCL CONFIG REQUIRED)

# Non-VcPkg
find_package (Vulkan REQUIRED) # auto detecting vulkan in system with cmake

# QT
set (QT_INSTALLATION_PREFIX_PATH "<null>")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (QT_INSTALLATION_PREFIX_PATH "$ENV{QT_INSTALLATION_PREFIX}/mingw_64/")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set (QT_INSTALLATION_PREFIX_PATH "$ENV{QT_INSTALLATION_PREFIX}/msvc2019_64/")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
else ()
    message (FATAL_ERROR "For developers: add QT installation prefix path for ${CMAKE_CXX_COMPILER_ID}, or else change compilers")
endif ()

if (EXISTS ${QT_INSTALLATION_PREFIX_PATH})
    list (APPEND CMAKE_PREFIX_PATH ${QT_INSTALLATION_PREFIX_PATH})
else ()
    message (FATAL_ERROR "For developers: QT_INSTALLATION_PREFIX_PATH not found, check if env:QT_INSTALLATION_PREFIX=<QT_installation_dir>/<QT_full_version> is defined or ${QT_INSTALLATION_PREFIX_PATH} exists")
endif ()

find_package (QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package (Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets OpenGL OpenGLWidgets REQUIRED)

if (NOT TARGET Qt${QT_VERSION_MAJOR}::qmake)
    add_executable (Qt${QT_VERSION_MAJOR}::qmake IMPORTED)
endif ()

if (Qt${QT_VERSION_MAJOR}_FOUND AND TARGET Qt${QT_VERSION_MAJOR}::qmake)
    get_target_property (_qt${QT_VERSION_MAJOR}_qmake_location Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    execute_process (
        COMMAND "${_qt${QT_VERSION_MAJOR}_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt${QT_VERSION_MAJOR}_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        set (import_qtexec_location "${qt${QT_VERSION_MAJOR}_install_prefix}/bin/windeployqt.exe")
        if (NOT TARGET Qt${QT_VERSION_MAJOR}::windeployqt AND EXISTS ${import_qtexec_location})
            add_executable (Qt${QT_VERSION_MAJOR}::windeployqt IMPORTED)
            set_target_properties (Qt${QT_VERSION_MAJOR}::windeployqt PROPERTIES IMPORTED_LOCATION ${import_qtexec_location})
        endif ()
    endif ()
    
    set (import_qtexec_location "${qt${QT_VERSION_MAJOR}_install_prefix}/bin/moc.exe")
    if (NOT TARGET Qt${QT_VERSION_MAJOR}::moc AND EXISTS ${import_qtexec_location})
        add_executable (Qt${QT_VERSION_MAJOR}::moc IMPORTED)
        set_property (TARGET Qt${QT_VERSION_MAJOR}::moc PROPERTY IMPORTED_LOCATION ${import_qtexec_location})
    endif ()
    
    set (import_qtexec_location "${qt${QT_VERSION_MAJOR}_install_prefix}/bin/rcc.exe")
    if (NOT TARGET Qt${QT_VERSION_MAJOR}::rcc)
        add_executable (Qt${QT_VERSION_MAJOR}::rcc IMPORTED)
        set_property (TARGET Qt${QT_VERSION_MAJOR}::rcc PROPERTY IMPORTED_LOCATION ${import_qtexec_location})
    endif ()

    set (import_qtexec_location "${qt${QT_VERSION_MAJOR}_install_prefix}/bin/uic.exe")
    if (NOT TARGET Qt${QT_VERSION_MAJOR}::uic)
        add_executable (Qt${QT_VERSION_MAJOR}::uic IMPORTED)
        set_property (TARGET Qt${QT_VERSION_MAJOR}::uic PROPERTY IMPORTED_LOCATION ${import_qtexec_location})
    endif ()
elseif (Qt${QT_VERSION_MAJOR}_FOUND)
    message ("QT_FOUND but qmake wasn't'")
else ()
    set (test_true TRUE)
    message ("Cannot find QT - ${Qt${QT_VERSION_MAJOR}_FOUND}--${Qt_FOUND} -")
endif ()

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTOUIC_SEARCH_PATHS "./ui/")

set (TARGET_EXEC "__try__")
file(GLOB HEADER_COLLECTION "src/**.h" "src/**.hpp" "src/**.hxx")

add_executable(${TARGET_EXEC} 
    # Headers
        ${HEADER_COLLECTION}
    # Source
        #"src/"
        
        "src/BoxMesh.cpp"
        "src/BoxObject.cpp"
        "src/GridObject.cpp"
        "src/KeyboardMouseHandler.cpp"
        "src/ObjModel.cpp"
        "src/OpenGLException.cpp"
        "src/OpenGLWindow.cpp"
        "src/PickLineObject.cpp"
        "src/PickObject.cpp"
        "src/SceneView.cpp"
        "src/SceneViewLeft.cpp"
        "src/ShaderProgram.cpp"
        "src/TestDialog.cpp"
        "src/Transform3d.cpp"
        "src/main.cpp"
    )

set_property(TARGET ${TARGET_EXEC} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

target_compile_definitions(${TARGET_EXEC}
    PRIVATE 
        _ENABLE_EXTENDED_ALIGNED_STORAGE
        PROJECT_SOURCE_LOCATION="${CMAKE_CURRENT_LIST_DIR}/"
    )

target_include_directories (${TARGET_EXEC}
    PRIVATE 
        ${VULKAN_INCLUDE_DIRS}
        "src/"
    )

target_link_libraries (${TARGET_EXEC}
    PRIVATE 
        glm::glm
        ${OPENGL_LIBRARIES} 
        ${GLUT_LIBRARIES}
        ${PCL_LIBRARIES}

        Vulkan::Vulkan
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    )

add_custom_command(
    TARGET ${TARGET_EXEC} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        #"${CMAKE_SOURCE_DIR}/default.qss"

        "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Gui>"   
        "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Core>"  
        "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Widgets>"
        "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::OpenGL>"
        "$<TARGET_FILE:Qt${QT_VERSION_MAJOR}::OpenGLWidgets>"

    #to
        "$<TARGET_FILE_DIR:${TARGET_EXEC}>"
    COMMENT "copying to EXECUTABLE'S DIR: TARGET_FILE_DIR:${TARGET_EXEC}" )

add_custom_command(
    TARGET ${TARGET_EXEC} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_CURRENT_LIST_DIR}/shaders"
    #to
        "$<TARGET_FILE_DIR:${TARGET_EXEC}>/shaders"
    COMMENT "copying shaders to EXECUTABLE'S DIR: TARGET_FILE_DIR:${TARGET_EXEC}" )

# QT deployment
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(
        TARGET ${TARGET_EXEC} POST_BUILD
        COMMAND Qt${QT_VERSION_MAJOR}::windeployqt "$<TARGET_FILE_DIR:${TARGET_EXEC}>"
        COMMENT "((())) WINDEPLOYQT_EXECUTABLE>> ${WINDEPLOYQT_EXECUTABLE}" )
endif()
# Using OpenGL

If you are using OpenGL, you should add the following to your CMake configuration to enable the OpenGL module:

```cmake
find_package(Qt${QT_VERSION_MAJOR} ${QWT_MIN_QT_VERSION} COMPONENTS
    OpenGL
    REQUIRED
)
target_link_libraries(${QWT_APP_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::OpenGL
)
if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} ${QWT_MIN_QT_VERSION} COMPONENTS
        OpenGLWidgets
        REQUIRED
    )
    target_link_libraries(${QWT_APP_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    )
endif()
```

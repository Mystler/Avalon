include_directories(3rdparty)
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${SDL_INCLUDE_DIR})
include_directories(${SDL_IMAGE_INCLUDE_DIRS})

set(Avalon_HEADERS
    App.h
    ShaderProgram.h
)
set(Avalon_SOURCES
    main.cpp
    App.cpp
    ShaderProgram.cpp
)
set(Avalon_3RDPARTY
    3rdparty/GL/glew.c
)

add_executable(Avalon WIN32 MACOSX_BUNDLE
    ${Avalon_HEADERS} ${Avalon_SOURCES} ${Avalon_3RDPARTY}
)

target_link_libraries(Avalon ${OPENGL_LIBRARIES}
    ${SDL_LIBRARY} ${SDLMAIN_LIBRARY} ${SDL_IMAGE_LIBRARIES}
)

source_group("Source Files" FILES ${Avalon_SOURCES})
source_group("Header Files" FILES ${Avalon_HEADERS})
source_group("3rdparty" FILES ${Avalon_3RDPARTY})

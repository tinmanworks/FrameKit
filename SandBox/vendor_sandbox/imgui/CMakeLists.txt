#[[
===============================================================================
 Project   : ImGui Vendor Library
 File      : vendor/imgui/CMakeLists.txt
 Purpose   : Build Dear ImGui (docking branch) as a static vendor library with
             GLFW + OpenGL3 backends, linking to external GLFW, GL loader, and
             OpenGL targets provided by the root project.
===============================================================================
]]

# Guard: allow safe double-inclusion
if(TARGET ImGui::imgui)
    return()
endif()

# ---- Options / knobs ----
# Change if you use a different GL loader: GLAD, GLEW, GL3W, etc.
set(IMGUI_OPENGL_LOADER "GLAD" CACHE STRING "OpenGL loader used by ImGui's OpenGL3 backend (GLAD, GLEW, GL3W, ...)")

# Expected external targets (provided by the root project)
set(IMGUI_GLFW_TARGET    "glfw"        CACHE STRING "GLFW CMake target name to link against")
set(IMGUI_GL_LOADER_TGT  "Glad"        CACHE STRING "GL loader CMake target name to link against (e.g., Glad or Glad::Glad or glad)")
set(IMGUI_OPENGL_TARGET  "OpenGL::GL"  CACHE STRING "OpenGL CMake target name to link against")

# ---- Sources ----
set(IMGUI_DIR         "${CMAKE_CURRENT_LIST_DIR}/imgui")
set(IMGUI_BACKEND_DIR "${IMGUI_DIR}/backends")

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
    ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

# ---- Library ----
add_library(imgui STATIC ${IMGUI_SRC})
add_library(ImGui::imgui ALIAS imgui)

# Publish includes to consumers
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/misc/cpp
    ${IMGUI_BACKEND_DIR}
)

# Backend loader define
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_${IMGUI_OPENGL_LOADER})

# Link against external targets
target_link_libraries(imgui PUBLIC
    ${IMGUI_GLFW_TARGET}
    ${IMGUI_GL_LOADER_TGT}
    # ${IMGUI_OPENGL_TARGET}
)

# Optional: export headers as a file set for IDEs (CMake >= 3.23)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.23")
    target_sources(imgui PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${IMGUI_DIR}
        FILES
            ${IMGUI_DIR}/imconfig.h
            ${IMGUI_DIR}/imgui.h
            ${IMGUI_DIR}/imgui_internal.h
            ${IMGUI_DIR}/imstb_rectpack.h
            ${IMGUI_DIR}/imstb_textedit.h
            ${IMGUI_DIR}/imstb_truetype.h
            ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.h
            ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.h
    )
endif()

# Group in IDE under "vendor"
set_property(TARGET imgui PROPERTY FOLDER "vendor")

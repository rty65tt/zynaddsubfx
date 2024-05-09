
include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/DPF/distrho .
    ${CMAKE_SOURCE_DIR}/DPF/dgl
    ${CMAKE_SOURCE_DIR}/DPF/dgl/src)

add_library(ZynAddSubFX_vst MODULE
    ${CMAKE_SOURCE_DIR}/src/globals.cpp
    ${CMAKE_SOURCE_DIR}/DPF/distrho/DistrhoPluginMain.cpp
    ${CMAKE_SOURCE_DIR}/DPF/distrho/DistrhoUIMain.cpp
    ZynAddSubFX.cpp
    ZynAddSubFX-UI.cpp
    )

set_target_properties(ZynAddSubFX_vst PROPERTIES COMPILE_DEFINITIONS "DISTRHO_PLUGIN_TARGET_VST3")
set_target_properties(ZynAddSubFX_vst PROPERTIES LIBRARY_OUTPUT_DIRECTORY "vst3")
set_target_properties(ZynAddSubFX_vst PROPERTIES LIBRARY_OUTPUT_NAME "ZynAddSubFX")
set_target_properties(ZynAddSubFX_vst PROPERTIES SUFFIX ".vst3")
set_target_properties(ZynAddSubFX_vst PROPERTIES PREFIX "")

target_link_libraries(ZynAddSubFX_vst
    ${NONGUI_LIBRARIES}
    ${GUI_LIBRARIES}
    ZLIB::ZLIB
    FFTW3::fftw3
    FFTW3::fftw3f
    FFTW3::fftw3l
    mxml
    fltk fltk_gl fltk_forms fltk_images
    -static-libgcc -static-libstdc++
    portaudio_static
    )

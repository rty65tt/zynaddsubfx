#Dependency check
find_package(ZLIB REQUIRED)
find_package(FFTW3 CONFIG REQUIRED)
find_package(FFTW3f CONFIG REQUIRED)
find_package(FFTW3l CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(FLTK CONFIG REQUIRED)
find_package(OpenGL) #for FLTK
find_package(portaudio CONFIG REQUIRED)

######### Settings ###########
# NOTE: These cache variables should normallly not be changed in this
# file, but either in in CMakeCache.txt before compile, or by passing
# parameters directly into cmake using the -D flag.

SET (OutputModule portaudio CACHE STRING "Output module portaudio")
SET (GuiModule fltk CACHE STRING "GUI module, either fltk or off")

if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "pkg-config not found")
endif(NOT PKG_CONFIG_FOUND)

########### Settings dependant code ###########
# From here on, the setting variables have  been prepared so concentrate
# on the actual compiling.

add_definitions(
	-DFFTW_VERSION_${FFTW_VERSION}
	-DOS_${OS_PORT}
	-D${MIDIIN}MIDIIN
	-D${AUDIOOUT}AUDIOOUT
	-DASM_F2I_YES
	-DFLTK_GUI
	 )

set(GUI_LIBRARIES ${OPENGL_LIBRARIES} zynaddsubfx_gui)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/UI"
	"${CMAKE_CURRENT_BINARY_DIR}/UI"
	)

add_subdirectory(UI)

########### General section ##############
# Following this should be only general compilation code, and no mention
# of module-specific variables

include_directories(
	${CMAKE_CURRE7NT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

set(NONGUI_LIBRARIES
	zynaddsubfx_input
	zynaddsubfx_output
	zynaddsubfx_misc
	zynaddsubfx_synth
	zynaddsubfx_seq
	zynaddsubfx_effect
	zynaddsubfx_params
	zynaddsubfx_dsp
	zynaddsubfx_samples
  )

add_subdirectory(Misc)
add_subdirectory(Input)
add_subdirectory(Synth)
add_subdirectory(Output)
add_subdirectory(Seq)
add_subdirectory(Effects)
add_subdirectory(Params)
add_subdirectory(DSP)
add_subdirectory(Samples)
add_subdirectory(Plugin)

set(zynaddsubfx_SRCS
	main.cpp 
	)

add_executable(zynaddsubfx WIN32 
	${zynaddsubfx_SRCS}
)

target_link_libraries(zynaddsubfx PRIVATE optimized 
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




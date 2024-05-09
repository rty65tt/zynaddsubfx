set(zynaddsubfx_output_SRCS
	PAaudiooutput.cpp
	Recorder.cpp
	VSTaudiooutput.cpp
	WAVaudiooutput.cpp
)

add_library(zynaddsubfx_output STATIC
	${zynaddsubfx_output_SRCS} 
	)


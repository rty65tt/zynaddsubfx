#set(AlsaMidiInput On CACHE BOOL "Include ALSA Midi input")
set(zynaddsubfx_input_SRCS
	MidiIn.cpp 
	NULLMidiIn.cpp 
	#OSSMidiIn.cpp #[TODO] get OSS midi detection and 
	WINMidiIn.cpp #       Win midi detection working
)


add_library(zynaddsubfx_input STATIC
	${zynaddsubfx_input_SRCS} 
	)

set(zynaddsubfx_misc_SRCS
	Bank.cpp
	Config.cpp
	Dump.cpp
	Master.cpp
	Microtonal.cpp
	Part.cpp
	Util.cpp
	XMLwrapper.cpp
)

#	MiddleWare.cpp

add_library(zynaddsubfx_misc STATIC
	${zynaddsubfx_misc_SRCS} 
	)

target_link_libraries(zynaddsubfx_misc zynaddsubfx_output)

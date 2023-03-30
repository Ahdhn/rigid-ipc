if(TARGET ipc::toolkit)
    return()
endif()

message(STATUS "Third-party: creating target 'ipc::toolkit'")

include(FetchContent)
FetchContent_Declare(
    ipc_toolkit    
	GIT_REPOSITORY https://github.com/Ahdhn/ipc-toolkit.git
	GIT_TAG f7fd6e41ecfaccbaed4bdc6d1c64de1ef84840e0    
	#GIT_REPOSITORY https://github.com/ipc-sim/ipc-toolkit.git
	#GIT_TAG d41d6ca93cf9b9c4c01a3177fbb68b63c6b74df1
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(ipc_toolkit)

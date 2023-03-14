if(TARGET ipc::toolkit)
    return()
endif()

message(STATUS "Third-party: creating target 'ipc::toolkit'")

include(FetchContent)
FetchContent_Declare(
    ipc_toolkit    
	GIT_REPOSITORY https://github.com/Ahdhn/ipc-toolkit.git
	GIT_TAG a28c58bb8f4e5a91a09ded0617c15fc73c9f9666    
	#GIT_REPOSITORY https://github.com/ipc-sim/ipc-toolkit.git
	#GIT_TAG d41d6ca93cf9b9c4c01a3177fbb68b63c6b74df1
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(ipc_toolkit)

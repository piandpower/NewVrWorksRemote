#
# Build ApexFramework
#

SET(GW_DEPS_ROOT $ENV{GW_DEPS_ROOT})
FIND_PACKAGE(PxShared REQUIRED)

SET(APEX_MODULE_DIR ${PROJECT_SOURCE_DIR}/../../../module)

SET(FRAMEWORK_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../framework)

SET(APEXFRAMEWORK_LIBTYPE STATIC)

SET(APEXFRAMEWORK_PLATFORM_SOURCE_FILES
)

# Use generator expressions to set config specific preprocessor definitions
SET(APEXFRAMEWORK_COMPILE_DEFS 
	# Common to all configurations
	${APEX_LINUX_COMPILE_DEFS};_LIB;PX_PHYSX_STATIC_LIB;
	__Linux__;Linux;NX_USE_SDK_STATICLIBS;NX_FOUNDATION_STATICLIB;NV_PARAMETERIZED_HIDE_DESCRIPTIONS=1;_LIB;
)

if(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")
	LIST(APPEND APEXFRAMEWORK_COMPILE_DEFS
		${APEX_LINUX_DEBUG_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=DEBUG;
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "checked")
	LIST(APPEND APEXFRAMEWORK_COMPILE_DEFS
		${APEX_LINUX_CHECKED_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=CHECKED;
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "profile")
	LIST(APPEND APEXFRAMEWORK_COMPILE_DEFS
		${APEX_LINUX_PROFILE_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=PROFILE;
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL release)
	LIST(APPEND APEXFRAMEWORK_COMPILE_DEFS
		${APEX_LINUX_RELEASE_COMPILE_DEFS}
	)
else(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")
	MESSAGE(FATAL_ERROR "Unknown configuration ${CMAKE_BUILD_TYPE}")
endif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")



# include common ApexFramework.cmake
INCLUDE(../common/ApexFramework.cmake)

# Do final direct sets after the target has been defined
TARGET_LINK_LIBRARIES(ApexFramework PUBLIC ApexCommon ApexShared NvParameterized PsFastXml PxFoundation PxPvdSDK PxTask RenderDebug
#	PhysxCommon - does it really need this?
)

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(ApexFramework PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
# 
# GLM is a header-only library. 
# 
 
SET( GLM_INCLUDE ${CROSS_PLATFORM_PATHS}
        CACHE PATH
        "The directory where glm/glm.hpp resides") 
 
IF(GLM_INCLUDE) 
    SET(GLM_FOUND "YES") 
    MESSAGE(STATUS "Found GLM.") 
ENDIF(GLM_INCLUDE)

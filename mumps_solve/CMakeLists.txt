cmake_minimum_required(VERSION 3.12.0)

#  example cmake command
#  cmake .. -DCMAKE_INSTALL_NAME_DIR=${HOME}/sandbox/lib
#           -DCMAKE_INSTALL_PREFIX=${HOME}/sandbox
#	   -DMETIS_LINK_DIR=${HOME}/sandbox/lib
#	   -DPARMETIS_LINK_DIR=${HOME}/sandbox/lib
#	   -DCMAKE_Fortran_COMPILER=mpif90
#	   -DLAPACK_FLAGS="-L/opt/local/lib -lscalapack -llapack -lpthread"
#	   -DMUMPS_INCLUDE_DIR=/Users/shiraiwa/src/MUMPS_5.1.1/include/
#	   -DMUMPS_LINK_DIR=/Users/shiraiwa/src/MUMPS_5.1.1/lib/

project(MUMPS_SOLVE)

option(USE_METIS "Link with METIS/ParMETIS library" ON)
option(USE_SCOTCH "Link with Scotch/PT-Scotch library" ON)


if(NOT METIS_LINK_DIR)
  set(METIS_LINK_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()
if(NOT PARMETIS_LINK_DIR)
  set(PARMETIS_LINK_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()
if(NOT SCOTCH_LINK_DIR)
    set(SCOTCH_LINK_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()
  
#set(CMAKE_INSTALL_RPATH ${METIS_LINK_DIR}
#                        ${PARMETIS_LINK_DIR}
#                        ${SCOTCH_LINK_DIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

enable_language(Fortran)

find_package(ZLIB REQUIRED)
find_package(MPI  REQUIRED)
find_package(OpenMP)

if (OPENMP_FOUND)
    if(NOT DEFINED OpenMP_LINK_FLAG)
        set(OpenMP_LINK_FLAG ${OpenMP_EXE_LINKER_FLAGS})
    endif()
else()
    if(DEFINED OpenMP_Fortran)
        set(OpenMP_LINK_FLAG  ${OpenMP_Fortran})
    endif()
    set(OpenMP_CXX_FLAGS "")
endif()

set(FPIC " -fPIC ")
set(LINK_FLAGS ${FPIC})

if (APPLE)
    set(SHARED_SC_COMPILE_FLAGS " -fPIC ")
    set(SHARED_SC_LINK_FLAGS_1 "-Wl,-force_load")
    set(SHARED_SC_LINK_FLAGS_2 "")
    set(SYMBOL_FLAG1  "-Wl,-exported_symbols_list")
    set(SYMBOL_FLAG2  ${CMAKE_CURRENT_SOURCE_DIR}/exported_symbols_list.txt)
    set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR true)  # this writes full path using install_name_dir  
else()
    set(SHARED_SC_COMPILE_FLAGS " -fPIC")
    set(SHARED_SC_LINK_FLAGS_1 "-Wl,-whole-archive")
    set(SHARED_SC_LINK_FLAGS_2 "-Wl,-no-whole-archive")
    set(SYMBOL_FLAG1  "-Wl,--version-script=../exported_version_script.txt")
    set(SYMBOL_FLAG2  "")
endif()

set( MUMPS_COMMON_STATIC ${MUMPS_LINK_DIR}/libmumps_common.a )
set( ZMUMPS_STATIC ${MUMPS_LINK_DIR}/libzmumps.a )
set( SMUMPS_STATIC ${MUMPS_LINK_DIR}/libsmumps.a )
set( DMUMPS_STATIC ${MUMPS_LINK_DIR}/libdmumps.a )
set( CMUMPS_STATIC ${MUMPS_LINK_DIR}/libcmumps.a )
set( PORD_STATIC ${MUMPS_LINK_DIR}/libpord.a )
set( METIS_STATIC ${METIS_LINK_DIR}/libmetis.a )
set( PARMETIS_STATIC ${METIS_LINK_DIR}/libparmetis.a )
set( SCOTCH_STATIC ${SCOTCH_LINK_DIR}/libscotch.a )
set( SCOTCHE_STATIC ${SCOTCH_LINK_DIR}/libscotcherr.a )
set( SCOTCHEE_STATIC ${SCOTCH_LINK_DIR}/libscotcherrexit.a )
set( PTSCOTCH_STATIC ${SCOTCH_LINK_DIR}/libptscotch.a )
set( PTSCOTCHE_STATIC ${SCOTCH_LINK_DIR}/libptscotcherr.a )
set( PTSCOTCHEE_STATIC ${SCOTCH_LINK_DIR}/libptscotcherrexit.a )
set( ESMUMPS_STATIC ${SCOTCH_LINK_DIR}/libesmumps.a )
set( PTESMUMPS_STATIC ${SCOTCH_LINK_DIR}/libptesmumps.a )

#set( GKLIB_STATIC ${METIS_LINK_DIR}/libGKlib.a )

set( MUMPS_SOLVE_H ${CMAKE_CURRENT_SOURCE_DIR}/mumps_solve.hpp )
set( IFLAG " -I")
set( CMP_FLAG ${OpenMP_CXX_FLAGS}${SHARED_SC_COMPILE_FLAGS}${IFLAG}${MUMPS_INCLUDE_DIR})
message(STATUS ${CMP_FLAG})
message(STATUS ${MUMPS_INCLUDE_DIR})

# METIS_LIBRARIES
list(APPEND METIS_LIBRARIES "")
if (USE_METIS)
  #list(APPEND METIS_LIBRARIES
  #   -L${PARMETIS_LINK_DIR}
  #   "parmetis"
  #   -L${METIS_LINK_DIR}
  #   "metis")
  list(APPEND METIS_LIBRARIES
       ${SHARED_SC_LINK_FLAGS_1}
       ${PARMETIS_STATIC}
       ${SHARED_SC_LINK_FLAGS_1}
       ${METIS_STATIC}
       #${SHARED_SC_LINK_FLAGS_1}
       #${GKLIB_STATIC}
       ${SHARED_SC_LINK_FLAGS_2})
endif()
 
# SCOTCH_LIBRARIES
list(APPEND SCOTCH_LIBRARIES "")
if (USE_SCOTCH)
  list(APPEND SCOTCH_LIBRARIES
    ${ESMUMPS_STATIC}    
    ${SCOTCH_STATIC}
    ${SCOTCHE_STATIC}
    ${SCOTCHEE_STATIC}
    ${PTESMUMPS_STATIC}        
    ${PTSCOTCH_STATIC}
    ${PTSCOTCHE_STATIC}
    ${PTSCOTCHEE_STATIC})
endif()

# make target
add_library(mumps_solve SHARED mumps_solve.cpp)
target_include_directories(mumps_solve PRIVATE ${MUMPS_INCLUDE_DIR})
target_link_libraries(mumps_solve
                            ${SHARED_SC_LINK_FLAGS_1}
                            ${ZMUMPS_STATIC}
                            ${SHARED_SC_LINK_FLAGS_1}			    
                            ${CMUMPS_STATIC}
                            ${SHARED_SC_LINK_FLAGS_1}			    
                            ${DMUMPS_STATIC}
                            ${SHARED_SC_LINK_FLAGS_1}
     		            ${SMUMPS_STATIC}
                            ${SHARED_SC_LINK_FLAGS_1}
                            ${MUMPS_COMMON_STATIC}
                            ${SHARED_SC_LINK_FLAGS_1}
                            ${PORD_STATIC}
                            ${SHARED_SC_LINK_FLAGS_2}
                            ${SYMBOL_FLAG1}
                            ${SYMBOL_FLAG2}
			    ${METIS_LIBRARIES}
			    ${SCOTCH_LIBRARIES}
   		            ${SCALAPACK_FLAGS}
   		            ${LAPACK_FLAGS}
			    ${MPI_C_LIBRARIES}
                            ${OpenMP_LINK_FLAG})
			    
set_target_properties(mumps_solve PROPERTIES 
                      COMPILE_FLAGS ${CMP_FLAG}
                      LINK_FLAGS ${LINK_FLAGS}
		      LINKER_LANGUAGE Fortran
                      OUTPUT_NAME mumps_solve )
install(TARGETS mumps_solve
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )


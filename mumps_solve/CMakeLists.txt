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

enable_language(Fortran)

find_package(MPI REQUIRED)
find_package(OpenMP)
find_package(Threads)


set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


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

get_filename_component(EXTERNAL_MUMPS_CMBUILD   "../external/mumps/cmbuild" ABSOLUTE)

if (NOT DEFINED MUMPS_LINK_DIR)
  if(EXISTS ${EXTERNAL_MUMPS_CMBUILD}/local/lib/libmumps_common.a)
    set(MUMPS_LINK_DIR ${EXTERNAL_MUMPS_CMBUILD}/local/lib)
  endif()
  if(EXISTS ${EXTERNAL_MUMPS_CMBUILD}/local/lib64/libmumps_common.a)
    set(MUMPS_LINK_DIR ${EXTERNAL_MUMPS_CMBUILD}/local/lib64)
  endif()
  if (NOT DEFINED MUMPS_LINK_DIR)
    message(FATAL_ERROR "Error: MUMPS_LINK_DIR can not be determined")
  endif()
endif()
if (NOT DEFINED MUMPS_INCLUDE_DIR)
    set(MUMPS_INCLUDE_DIR ${EXTERNAL_MUMPS_CMBUILD}/local/include)
endif()

if (NOT DEFINED SCOTCH_LINK_DIR)
  set(SCOTCH_LINK_DIR ${MUMPS_LINK_DIR})
endif()
if (NOT DEFINED METIS_LINK_DIR)
  set(METIS_LINK_DIR ${MUMPS_LINK_DIR})
endif()
if (NOT DEFINED SCOTCH_INCLUDE_DIR)
    set(SCOTCH_INCLUDE_DIR ${MUMPS_INCLUDE_DIR})
endif()
if (NOT DEFINED METIS_INCLUDE_DIR)
    set(METIS_INCLUDE_DIR ${MUMPS_INCLUDE_DIR})
endif()


load_cache(${EXTERNAL_MUMPS_CMBUILD} EXCLUDE CMAKE_INSTALL_PREFIX)

set( MUMPS_COMMON_STATIC ${MUMPS_LINK_DIR}/libmumps_common.a )
set( ZMUMPS_STATIC ${MUMPS_LINK_DIR}/libzmumps.a )
set( SMUMPS_STATIC ${MUMPS_LINK_DIR}/libsmumps.a )
set( DMUMPS_STATIC ${MUMPS_LINK_DIR}/libdmumps.a )
set( CMUMPS_STATIC ${MUMPS_LINK_DIR}/libcmumps.a )
set( PORD_STATIC ${MUMPS_LINK_DIR}/libpord.a )
set( GKLIB_STATIC ${METIS_LINK_DIR}/libGKlib.a )
set( METIS_STATIC ${METIS_LINK_DIR}/libmetis.a )
set( PARMETIS_STATIC ${METIS_LINK_DIR}/libparmetis.a )
set( SCOTCH_STATIC ${SCOTCH_LINK_DIR}/libscotch.a )
set( SCOTCHE_STATIC ${SCOTCH_LINK_DIR}/libscotcherr.a )
set( PTSCOTCH_STATIC ${SCOTCH_LINK_DIR}/libptscotch.a )
set( PTSCOTCHE_STATIC ${SCOTCH_LINK_DIR}/libptscotcherr.a )
set( ESMUMPS_STATIC ${SCOTCH_LINK_DIR}/libesmumps.a )
set( PTESMUMPS_STATIC ${SCOTCH_LINK_DIR}/libptesmumps.a )

add_library(mumps_solve SHARED mumps_solve.cpp)

message(STATUS "MUMPS include director:"  ${MUMPS_INCLUDE_DIR})
message(STATUS "Install Prefix:"  ${CMAKE_INSTALL_PREFIX})

target_include_directories(mumps_solve PUBLIC ${MUMPS_INCLUDE_DIR} ${METIS_INCLUDE_DIR} ${SCOTCH_INCLUDE_DIR})
target_link_libraries(mumps_solve PRIVATE
#                            ${SHARED_SC_LINK_FLAGS_1}
                            ${ZMUMPS_STATIC}
#                            ${SHARED_SC_LINK_FLAGS_1}
                            ${CMUMPS_STATIC}
#                            ${SHARED_SC_LINK_FLAGS_1}
                            ${DMUMPS_STATIC}
#                            ${SHARED_SC_LINK_FLAGS_1}
     		            ${SMUMPS_STATIC}
#                            ${SHARED_SC_LINK_FLAGS_1}
                            ${MUMPS_COMMON_STATIC}
#                            ${SHARED_SC_LINK_FLAGS_1}
                            ${PORD_STATIC}
#                            ${SHARED_SC_LINK_FLAGS_2}
                            ${SYMBOL_FLAG1}
                            ${SYMBOL_FLAG2})
if(MUMPS_ptscotch)
  target_link_libraries(mumps_solve PRIVATE ${PTESMUMPS_STATIC} ${PTSCOTCH_STATIC} ${PTSCOTCHE_STATIC})
endif()
if(MUMPS_scotch)
   target_link_libraries(mumps_solve PRIVATE ${ESMUMPS_STATIC} ${SCOTCH_STATIC} ${SCOTCHE_STATIC})
endif()

target_link_libraries(mumps_solve PRIVATE
  $<$<BOOL:${MUMPS_parmetis}>:${PARMETIS_STATIC}>
  $<$<BOOL:${MUMPS_metis}>:${METIS_STATIC}>
  $<$<BOOL:${MUMPS_metis}>:${GKLIB_STATIC}>
  ${PORD_STATIC}
  )
target_link_libraries(mumps_solve PRIVATE
  $<$<AND:$<BOOL:${MUMPS_scalapack}>,$<BOOL:${MUMPS_parallel}>>:${SCALAPACK_LIBRARY}>
  )
if (${LAPACK_LIBRARY})
   target_link_libraries(mumps_solve PRIVATE ${LAPACK_LIBRARY})
endif()
if (${BLACS_LIBRARY})
   target_link_libraries(mumps_solve PRIVATE ${BLACS_LIBRARY})
endif()
if (${BLAS_LIBRARY})
   target_link_libraries(mumps_solve PRIVATE ${BLAS_LIBRARY})
endif()

target_link_libraries(mumps_solve PRIVATE
  $<$<BOOL:${IMPI_LIB64}>:${IMPI_LIB64}>
  ${CMAKE_THREAD_LIBS_INIT}
  MPI::MPI_Fortran MPI::MPI_CXX MPI::MPI_C
  $<$<BOOL:${MUMPS_openmp}>:OpenMP::OpenMP_Fortran>
)

set_target_properties(mumps_solve PROPERTIES
		      LINKER_LANGUAGE Fortran
                      OUTPUT_NAME mumps_solve )
install(TARGETS mumps_solve
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        )

# Copyright 2019 Seungbin Song

set (PSDN_SRCS
  main.cpp
  midend.cpp
  )

set (PSDN_HDRS
  options.h
  version.h
  midend.h
  )

add_cpplint_files(${CMAKE_CURRENT_SOURCE_DIR} "${PSDN_SRCS};${PSDN_HDRS}")

build_unified(PSDN_SRCS ALL)
add_executable(p4c-psdn ${PSDN_SRCS})
target_link_libraries (p4c-psdn ${P4C_LIBRARIES} ${P4C_LIB_DEPS})

install (TARGETS p4c-psdn
  RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})
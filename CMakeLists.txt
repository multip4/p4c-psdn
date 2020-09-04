# Copyright 2019 Seungbin Song

set (PSDN_SRCS
  main.cpp
  midend.cpp
  backend.cpp
  programStructure.cpp
  resourceMap.cpp
  directMeterMap.cpp
  lower.cpp
  annotationParser.cpp
  expressionConverter.cpp
  headerConverter.cpp
  parserConverter.cpp
  controlConverter.cpp
  sdnetProgram.cpp
  controlFlowGraph.cpp
  tableAnalyzer.cpp
  graphs.cpp
  )

set (PSDN_HDRS
  options.h
  version.h
  midend.h
  backend.h
  programStructure.h
  resourceMap.h
  directMeterMap.h
  lower.h
  annotationParser.h
  expressionConverter.h
  controlPolicy.h
  conversionContext.h
  headerConverter.h
  parserConverter.h
  controlConverter.h
  sdnetProgram.h
  controlFlowGraph.h
  tableAnalyzer.h
  graphs.h
  )

add_cpplint_files(${CMAKE_CURRENT_SOURCE_DIR} "${PSDN_SRCS};${PSDN_HDRS}")

build_unified(PSDN_SRCS ALL)
add_executable(p4c-psdn ${PSDN_SRCS})
target_link_libraries (p4c-psdn ${P4C_LIBRARIES} ${P4C_LIB_DEPS})

install (TARGETS p4c-psdn
  RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})

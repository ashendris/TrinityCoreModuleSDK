include(CMakeParseArguments)

function(trinity_add_module)
  cmake_parse_arguments(M "" "NAME;MANIFEST" "SOURCES;CONFIG;SQL;INCLUDE_DIRS" ${ARGN})
  if(NOT M_NAME OR NOT M_MANIFEST OR NOT M_SOURCES)
    message(FATAL_ERROR "trinity_add_module requires NAME, MANIFEST and SOURCES")
  endif()
  file(READ "${M_MANIFEST}" manifest)
  string(JSON version GET "${manifest}" version)
  string(JSON name GET "${manifest}" name)
  if(NOT name STREQUAL M_NAME)
    message(FATAL_ERROR "Module NAME must match its manifest")
  endif()
  string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" safe "${M_NAME}")
  add_library("module_${safe}" SHARED ${M_SOURCES})
  target_link_libraries("module_${safe}" PRIVATE Trinity::SDK)
  target_include_directories("module_${safe}" PRIVATE ${M_INCLUDE_DIRS})
  target_compile_definitions("module_${safe}" PRIVATE
    TRINITY_MODULE_SHARED
    TRINITY_MODULE_NAME="${M_NAME}"
    TRINITY_MODULE_VERSION="${version}")
  set(resource "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/modules/${M_NAME}")
  set_target_properties("module_${safe}" PROPERTIES
    PREFIX ""
    OUTPUT_NAME "${M_NAME}"
    DEBUG_POSTFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${resource}"
    LIBRARY_OUTPUT_DIRECTORY "${resource}"
    ARCHIVE_OUTPUT_DIRECTORY "${resource}")
  add_custom_command(TARGET "module_${safe}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${resource}/conf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${M_MANIFEST}" "${resource}/module.json")
  foreach(config IN LISTS M_CONFIG)
    add_custom_command(TARGET "module_${safe}" POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${config}" "${resource}/conf/")
  endforeach()
  foreach(sql IN LISTS M_SQL)
    get_filename_component(dir "${sql}" DIRECTORY)
    get_filename_component(db "${dir}" NAME)
    add_custom_command(TARGET "module_${safe}" POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${resource}/sql/${db}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${sql}" "${resource}/sql/${db}/")
  endforeach()
endfunction()

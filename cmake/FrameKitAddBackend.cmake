function(framekit_add_backend)
  set(oneValueArgs DOMAIN TARGET COMPONENT FOLDER)
  set(multiValueArgs SOURCES HEADERS PUBLIC_LIBS PRIVATE_LIBS PUBLIC_DEFINES)
  cmake_parse_arguments(B "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_library(${B_TARGET} STATIC ${B_SOURCES} ${B_HEADERS})
  target_link_libraries(${B_TARGET}
    PUBLIC  FrameKit.${B_DOMAIN}Runtime FrameKit.${B_DOMAIN}API ${B_PUBLIC_LIBS}
    PRIVATE ${B_PRIVATE_LIBS})
  if(B_PUBLIC_DEFINES) 
    target_compile_definitions(${B_TARGET} PUBLIC ${B_PUBLIC_DEFINES}) 
  endif()

  # default folder
  if(NOT B_FOLDER)
    set(B_FOLDER "FrameKit/Backends")
  endif()
  set_target_properties(${B_TARGET} PROPERTIES FOLDER "${B_FOLDER}")

  install(TARGETS ${B_TARGET}
    EXPORT FrameKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${B_COMPONENT})
endfunction()

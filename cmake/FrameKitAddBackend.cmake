function(framekit_add_backend)
  set(oneValueArgs DOMAIN TARGET COMPONENT FOLDER)
  set(multiValueArgs SOURCES HEADERS PUBLIC_LIBS PRIVATE_LIBS PUBLIC_DEFINES)
  cmake_parse_arguments(B "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_library(${B_TARGET} STATIC ${B_SOURCES} ${B_HEADERS})
  # Backend depends on the domain API. Runtime will depend on backend (see below).
  target_link_libraries(${B_TARGET}
          PUBLIC  FrameKit.${B_DOMAIN}API ${B_PUBLIC_LIBS}
          PRIVATE ${B_PRIVATE_LIBS})

  if(B_PUBLIC_DEFINES)
    # Backend gets its feature defines
    target_compile_definitions(${B_TARGET} PUBLIC ${B_PUBLIC_DEFINES})
  endif()

  # Make the runtime depend on this backend and see the same defines.
  if(TARGET FrameKit.${B_DOMAIN}Runtime)
    target_link_libraries(FrameKit.${B_DOMAIN}Runtime PUBLIC ${B_TARGET})
    if(B_PUBLIC_DEFINES)
      target_compile_definitions(FrameKit.${B_DOMAIN}Runtime PUBLIC ${B_PUBLIC_DEFINES})
    endif()
  endif()
  
  # Default IDE folder
  if(NOT B_FOLDER)
    set(B_FOLDER "FrameKit/Backends/${B_DOMAIN}")
  endif()
  set_target_properties(${B_TARGET} PROPERTIES FOLDER "${B_FOLDER}")

  install(TARGETS ${B_TARGET}
    EXPORT FrameKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${B_COMPONENT})
endfunction()

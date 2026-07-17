zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)

if(CONFIG_SOFLE_CUSTOM_DISPLAY)
  zephyr_library_named(sofle_custom_display)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_sources(src/custom_status_screen.c)
endif()



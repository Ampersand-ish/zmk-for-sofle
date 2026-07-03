if(CONFIG_SOFLE_CUSTOM_DISPLAY)
  zephyr_library_named(sofle_custom_display)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_sources(src/custom_status_screen.c)
endif()

if(CONFIG_ZMK_RGB_UNDERGLOW)
  zephyr_library_sources(src/rgb_layer_colors.c)
endif()

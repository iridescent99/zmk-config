cmake_minimum_required(VERSION 3.16)
project(sofle_display_test C)
set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE STRING "" FORCE)
add_subdirectory(${LVGL_SOURCE} lvgl EXCLUDE_FROM_ALL)
set(widget_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../boards/shields/sharp_mip/widgets)
add_executable(display_test display_test.c ${widget_dir}/keypress_model.c ${widget_dir}/util.c ${widget_dir}/bolt.c)
target_include_directories(display_test PRIVATE stubs . ${widget_dir})
target_compile_options(display_test PRIVATE -Wall -Wextra -Werror)
target_link_libraries(display_test PRIVATE lvgl m)
add_executable(orbit_test orbit_test.c ${widget_dir}/util.c ${widget_dir}/bolt.c)
target_include_directories(orbit_test PRIVATE stubs . ${widget_dir})
target_compile_options(orbit_test PRIVATE -Wall -Wextra -Werror)
target_link_libraries(orbit_test PRIVATE lvgl m)

# Compile the actual firmware-version Zephyr transfer code, with only the
# display device mocked. This catches polarity bugs missed by a raw LVGL test.
configure_file(${ZEPHYR_SOURCE}/modules/lvgl/lvgl_display_mono.c
               ${CMAKE_CURRENT_BINARY_DIR}/lvgl_display_mono_test.c COPYONLY)
add_executable(starfield_test starfield_test.c ${widget_dir}/starfield_scene.c
               ${CMAKE_CURRENT_BINARY_DIR}/lvgl_display_mono_test.c)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/lvgl_display_mono_test.c
                           PROPERTIES COMPILE_OPTIONS "-Wno-unused-parameter")
target_include_directories(starfield_test PRIVATE stubs . ${widget_dir})
target_compile_options(starfield_test PRIVATE -Wall -Wextra -Werror)
target_link_libraries(starfield_test PRIVATE lvgl m)

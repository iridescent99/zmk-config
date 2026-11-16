if(CONFIG_SOFLE_GRADIENT)
  zephyr_library_sources(gradient_strip.c gradient_color.c)
  zephyr_library_sources_ifdef(CONFIG_SOFLE_PHYSICAL_GRADIENT physical_gradient.c)
endif()

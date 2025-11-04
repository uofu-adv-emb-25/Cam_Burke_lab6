# Executables (list each test file explicitly)
add_executable(mytest test.c unity_config.c)


set(APP_TARGETS
  mytest
)

foreach(t ${APP_TARGETS})
  pico_set_program_name(${t} "${t}")
  pico_set_program_version(${t} "0.1")

  pico_enable_stdio_uart(${t} 0)
  pico_enable_stdio_usb(${t} 1)

  target_link_libraries(${t} PRIVATE
    pico_stdlib
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    unity
  )

  if(PICO_CYW43_SUPPORTED)
    target_link_libraries(${t} PRIVATE pico_cyw43_arch_none)
  endif()

  pico_add_extra_outputs(${t})
endforeach()

find_program(RENODE renode NAMES macos_run.command Renode.exe PATHS /Applications/Renode.app/Contents/MacOS)
set(RENODE_FLAGS --disable-xwt --port -2 --pid-file renode.pid --console)

foreach(t ${APP_TARGETS})
  add_test(NAME simulate_${t} COMMAND
    ${RENODE}
    ${RENODE_FLAGS}
    -e "$ELF=@$<TARGET_FILE:${t}>; $WORKING=@${CMAKE_SOURCE_DIR}; include @${CMAKE_SOURCE_DIR}/test/simulate.resc; start"
  )
endforeach()

find_program(OPENOCD openocd)
find_program(PICOTOOL picotool)

foreach(t ${APP_TARGETS})
  if(PICOTOOL)
    add_custom_target(flash_${t} ${PICOTOOL} load -f $<TARGET_FILE:${t}> DEPENDS ${t})
  endif()

  if(OPENOCD)
    add_custom_target(openocd_flash_${t}
      ${OPENOCD} -s ${OPENOCD_PATH} -f interface/cmsis-dap.cfg -f target/rp2040.cfg
      -c "adapter speed 5000; program \"$<TARGET_FILE:${t}>\" verify reset exit"
      DEPENDS ${t})
  endif()
endforeach()

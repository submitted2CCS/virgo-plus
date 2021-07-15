aux_source_directory(. SRC)
add_executable(run_virgo_plus ${SRC})

target_link_libraries(run_virgo_plus virgo_lib)

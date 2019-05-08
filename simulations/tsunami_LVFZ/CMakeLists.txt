include_directories(${hybrid_fem_bie_SOURCE_DIR}/src)

add_executable(tsunami_LVFZ tsunami_LVFZ.cc)
target_link_libraries(tsunami_LVFZ hybrid_fem_bie)

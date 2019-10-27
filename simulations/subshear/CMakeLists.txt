include_directories(${hybrid_fem_bie_SOURCE_DIR}/src)

add_executable(subshear subshear.cc)
target_link_libraries(subshear hybrid_fem_bie)

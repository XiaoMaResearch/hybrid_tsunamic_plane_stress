include_directories(${hybrid_fem_bie_SOURCE_DIR}/src)

add_executable(supershear supershear.cc)
target_link_libraries(supershear hybrid_fem_bie)

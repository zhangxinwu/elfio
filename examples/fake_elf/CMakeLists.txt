
add_executable(fake_elf fake_elf.cpp)
target_link_libraries(fake_elf PRIVATE elfio::elfio)

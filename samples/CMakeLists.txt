function(msdis_access_add_sample target_name source_file_names)
    set(full_target_name msdis_access_sample_${target_name})
    add_executable(${full_target_name} ${source_file_names})

    target_link_libraries(${full_target_name} PRIVATE msdis_access)
    set_target_properties(${full_target_name} PROPERTIES OUTPUT_NAME "${target_name}")
    set_target_properties(${full_target_name} PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
endfunction()

msdis_access_add_sample(invoke_msdis invoke_msdis.c)
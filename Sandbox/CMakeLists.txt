# Get a list of all directories in the current folder
file(GLOB directories LIST_DIRECTORIES true *)

# Iterate over each directory
foreach(dir ${directories})
    # Check if the path is a directory
    if(IS_DIRECTORY ${dir})
        # Get the directory name from the full path
        get_filename_component(dir_name ${dir} NAME)
        # Call the macro with the directory name
        SETUP_EXAMPLE(${dir_name})
    endif()
endforeach()
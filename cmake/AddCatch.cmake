include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

function(add_catch TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_link_libraries(${TEST_NAME} PRIVATE catch_main Catch2)
endfunction()
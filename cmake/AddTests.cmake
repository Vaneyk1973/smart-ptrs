# ------------------------------------------------------------------------------
# UniquePtr

add_catch(test_unique unique/test.cpp)
target_compile_options(test_unique PRIVATE -Wno-self-move)

# ------------------------------------------------------------------------------
# SharedPtr + WeakPtr

add_catch(test_shared
        shared/test.cpp)

add_catch(test_weak
        weak/test.cpp
        weak/test_shared.cpp
        weak/test_odr.cpp)

add_catch(test_shared_from_this
        shared-from-this/test.cpp
        shared-from-this/test_shared.cpp
        shared-from-this/test_weak.cpp)

target_compile_options(test_shared PRIVATE -Wno-self-assign-overloaded)
target_compile_options(test_weak PRIVATE -Wno-self-assign-overloaded)
target_compile_options(test_shared_from_this PRIVATE -Wno-self-assign-overloaded)

# ------------------------------------------------------------------------------
# IntrusivePtr

add_catch(test_intrusive intrusive/test.cpp)
target_compile_options(test_intrusive PRIVATE -Wno-self-assign-overloaded -Wno-self-move)

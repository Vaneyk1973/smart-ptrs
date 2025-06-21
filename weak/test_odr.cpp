#include "shared.h"
#include "weak.h"

#include "catch2/catch_test_macros.hpp"


////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Just checking for ODR issues") {
    auto sp = MakeShared<int>(42);
    WeakPtr<int> wp(sp);
    REQUIRE(!wp.Expired());
    auto sp2 = wp.Lock();
    REQUIRE(*sp2 == 42);
}

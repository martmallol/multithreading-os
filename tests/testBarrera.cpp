#include "gtest-1.8.1/gtest.h"
#include <iostream>
#include <thread>
#include <sys/unistd.h>
#include <vector>
#include <mutex>
#include "../equipo.h"

// TESTS BARRERA
TEST(BarreraTest, primero) {
    EXPECT_TRUE(1 != 2);
}
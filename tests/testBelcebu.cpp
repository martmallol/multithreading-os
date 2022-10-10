#include "gtest-1.8.1/gtest.h"
#include <iostream>
#include <thread>
#include <sys/unistd.h>
#include <vector>
#include <mutex>
#include "../gameMaster.h"

// TESTS GLOBALES
TEST(BelcebuTest, primero) {
    Config config = *(new Config());
    gameMaster belcebu = gameMaster(config);

    EXPECT_TRUE(belcebu.getTamx() > 0);
}
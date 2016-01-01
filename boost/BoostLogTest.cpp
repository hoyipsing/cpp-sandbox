
#include "boost/log/trivial.hpp"
#include "gtest/gtest.h"

TEST(SimpleLog, testLogToConsole) {
	BOOST_LOG_TRIVIAL(info) << "Hello";
}

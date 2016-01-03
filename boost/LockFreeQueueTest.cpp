
#include "boost/lockfree/queue.hpp"
#include "gtest/gtest.h"

TEST(LockFreeQueueTest, TestQueue) {
	boost::lockfree::queue<uint32_t> queue(3);
	ASSERT_TRUE(queue.is_lock_free());
	ASSERT_TRUE(queue.empty());

	uint32_t result = 0;
	ASSERT_FALSE(queue.pop(result));

	queue.push(3);
	queue.push(9);
	ASSERT_FALSE(queue.empty());
	ASSERT_TRUE(queue.pop(result));
	ASSERT_EQ(3, result);
	ASSERT_TRUE(queue.pop(result));
	ASSERT_EQ(9, result);

	ASSERT_TRUE(queue.empty());
	ASSERT_FALSE(queue.pop(result));
}

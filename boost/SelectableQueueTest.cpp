
#include "boost/asio.hpp"
#include "boost/lockfree/queue.hpp"
#include "boost/log/trivial.hpp"
#include "gtest/gtest.h"
#include <functional>
#include <thread>

using boost::asio::io_service;
using boost::lockfree::queue;

class MessageConsumer : boost::noncopyable {
public:
	MessageConsumer(io_service& ioService,
			queue<int32_t>& queue, int32_t terminateValue):
			m_ioService(ioService),
			m_work(ioService),
			m_queue(queue),
			m_msgCnt(0),
			m_terminateValue(terminateValue) {
	}

	void messageAvailable() {
		int32_t result {0};
		if (m_queue.pop(result)) {
			++m_msgCnt;
			if (result == m_terminateValue) {
				// Last message received
				BOOST_LOG_TRIVIAL(info) << "Totally receive " << m_msgCnt << " messages";
				m_work.~work();
			}
		} else {
			BOOST_LOG_TRIVIAL(warning) << "No message received from queue";
		}
	}

	void start() {
		m_ioService.run();
	}

private:
	io_service& m_ioService;
	io_service::work m_work;
	queue<int32_t>& m_queue;
	uint32_t m_msgCnt;
	int32_t m_terminateValue;
};

class EventfdMessageConsumer : boost::noncopyable {
public:
	EventfdMessageConsumer(io_service& ioService,
			queue<int32_t>& queue,
			int32_t terminateValue,
			int eventfd):
			m_ioService(ioService),
			m_queue(queue),
			m_msgCnt(0),
			m_terminateValue(terminateValue),
			m_recvBuffer(0),
			m_eventHandle(eventfd),
			m_eventfdSocket(ioService, eventfd) {
	}

	~EventfdMessageConsumer() {
	}

	void messageAvailable(uint64_t cnt) {
		int32_t result {0};
		for (uint64_t i = 0; i < cnt; ++i) {
			if (m_queue.pop(result)) {
				++m_msgCnt;
			} else {
				BOOST_LOG_TRIVIAL(warning) << "No message received from queue";
				break;
			}
		}
		if (result == m_terminateValue) {
			// Last message received
			BOOST_LOG_TRIVIAL(info) << "Totally receive " << m_msgCnt << " messages";
		} else {
			registerEventCallback();
		}
	}

	void onEvent(boost::system::error_code ec, std::size_t received) {
		if (!ec) {
//			BOOST_LOG_TRIVIAL(trace) << "Receive " << received << " bytes" << ", eventCnt=" << m_msgCnt;
			messageAvailable(m_recvBuffer);
		} else {
			BOOST_LOG_TRIVIAL(fatal) << "Receive error code";
		}
	}

	void registerEventCallback() {
		m_eventfdSocket.async_read_some(
			boost::asio::buffer(&m_recvBuffer, sizeof(m_recvBuffer)),
			std::bind(
				&EventfdMessageConsumer::onEvent,
				this,
				std::placeholders::_1,
				std::placeholders::_2)
			);
	}

	void start() {
		registerEventCallback();
		m_ioService.run();
	}

private:
	io_service& m_ioService;
	queue<int32_t>& m_queue;
	int32_t m_terminateValue;
	int m_eventHandle;
	boost::asio::posix::stream_descriptor m_eventfdSocket;
	uint32_t m_msgCnt;
	uint64_t m_recvBuffer;
};

class PostMessageProducer : boost::noncopyable {
public:
	PostMessageProducer(io_service& ioService,
			MessageConsumer& messageCosumer,
			uint32_t numOfMsgToPublish,
			queue<int32_t>& queue,
			int32_t terminateValue):
			m_ioService(ioService),
			m_messageConsumer(messageCosumer),
			m_numOfMsgToPublish(numOfMsgToPublish),
			m_queue(queue),
			m_terminateValue(terminateValue) {
	}

	void start() {
		uint32_t cnt {0};
		for (uint32_t i = 0; i < m_numOfMsgToPublish; ++i) {
			if (m_queue.push(i)) {
				m_ioService.post(
					std::bind(&MessageConsumer::messageAvailable, std::ref(m_messageConsumer))
				);
				++cnt;
			} else {
				BOOST_LOG_TRIVIAL(fatal) << "Failed to publish item to queue";
			}
		}
		m_queue.push(m_terminateValue);
		m_ioService.post(
			std::bind(&MessageConsumer::messageAvailable, std::ref(m_messageConsumer))
		);
		++cnt;
		BOOST_LOG_TRIVIAL(info) << "Published " << cnt << " messages to queue";
	}

private:
	io_service& m_ioService;
	MessageConsumer& m_messageConsumer;
	uint32_t m_numOfMsgToPublish;
	queue<int32_t>& m_queue;
	int32_t m_terminateValue;
};

class EventfdMessageProducer : boost::noncopyable {
public:
	EventfdMessageProducer(io_service& ioService,
			queue<int32_t>& queue,
			uint32_t numOfMsgToPublish,
			int32_t terminateValue,
			int eventfd):
			m_ioService(ioService),
			m_numOfMsgToPublish(numOfMsgToPublish),
			m_queue(queue),
			m_progress(0),
			m_terminateValue(terminateValue),
			m_eventHandle(eventfd),
			m_eventfdSocket(ioService, eventfd),
			m_eventBuffer(1) {
	}

	void onWrite(boost::system::error_code ec, std::size_t written) {
		if (!ec) {
			++m_progress;
			sendOne();
		} else {
			BOOST_LOG_TRIVIAL(fatal) << "Send event error";
		}
	}

	void sendOne() {
		if (m_progress < m_numOfMsgToPublish) {
			if (m_queue.push(m_progress)) {
				notifyDownStream();
			} else {
				BOOST_LOG_TRIVIAL(fatal) << "Failed to publish item to queue";
			}
		} else if (m_progress == m_numOfMsgToPublish) {
			if (m_queue.push(m_terminateValue)) {
				notifyDownStream();
			} else {
				BOOST_LOG_TRIVIAL(fatal) << "Failed to publish item to queue";
			}
		}
	}

	void start() {
		sendOne();
		m_ioService.run();
		BOOST_LOG_TRIVIAL(info) << "Published " << m_progress << " messages to queue";
	}

private:
	io_service& m_ioService;
	uint32_t m_progress;
	uint32_t m_numOfMsgToPublish;
	queue<int32_t>& m_queue;
	int32_t m_terminateValue;
	int m_eventHandle;
	boost::asio::posix::stream_descriptor m_eventfdSocket;
	uint64_t m_eventBuffer;

	void notifyDownStream() {
		m_eventfdSocket.async_write_some(
			boost::asio::buffer(&m_eventBuffer, sizeof(m_eventBuffer)),
			std::bind(
				&EventfdMessageProducer::onWrite,
				this,
				std::placeholders::_1,
				std::placeholders::_2)
		);
	}
};

TEST(SelectableQueueTest, TestPost) {
	queue<int32_t> queue(3);
	io_service ioService;
	const int32_t terminateValue = -1;
	const uint32_t msgCnt = 500000;
	MessageConsumer consumer { ioService, queue, terminateValue };
	PostMessageProducer producer { ioService, consumer, msgCnt, queue, terminateValue };
	std::thread consumerThread(&MessageConsumer::start, std::ref(consumer));
	std::thread producerThread(&PostMessageProducer::start, std::ref(producer));

	producerThread.join();
	consumerThread.join();
}

TEST(SelectableQueueTest, TestEventfd) {
	const int32_t terminateValue = -1;
	const uint32_t msgCnt = 500000;
	queue<int32_t> queue(3);

	int efd = eventfd(0, EFD_NONBLOCK);
	ASSERT_NE(-1, efd) << "eventfd return -1";

	io_service consumerIoService;
	EventfdMessageConsumer consumer { consumerIoService, queue, terminateValue, efd };

	io_service producerIoService;
	EventfdMessageProducer producer { producerIoService, queue, msgCnt, terminateValue, efd };

	std::thread consumerThread(&EventfdMessageConsumer::start, std::ref(consumer));
	std::thread producerThread(&EventfdMessageProducer::start, std::ref(producer));

	producerThread.join();
	consumerThread.join();

	close(efd);
}

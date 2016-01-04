/*
 * EventfdTest.cpp
 *
 *  Created on: 1 Jan 2016
 *      Author: rho
 */

#include <sys/eventfd.h>
#include <unistd.h>
#include <chrono>
#include <functional>
#include <thread>
#include "boost/asio.hpp"
#include "gtest/gtest.h"

using boost::asio::io_service;
using std::clog;
using std::endl;
//using boost::asio::posix;

class EventSender {
public:
	EventSender(
		int eventHandle,
		uint32_t numOfEventToDeliver,
		std::chrono::milliseconds eventInterval):
		m_eventHandle(eventHandle),
		m_numOfEventToDeliver(numOfEventToDeliver),
		m_deliverCnt(0),
		m_eventInterval(eventInterval) {
	}
	void start() {
		const uint64_t one = 1;
		while (m_deliverCnt < m_numOfEventToDeliver) {
			ssize_t result = write(m_eventHandle, &one, sizeof(uint64_t));
			if (result == -1) {
			    clog << "[" << std::this_thread::get_id() << "] " << "write() return -1" << endl;
				break;
			}
			++m_deliverCnt;
			clog << "[" << std::this_thread::get_id() << "] " << "Sent " << m_deliverCnt << " events" << endl;
			std::this_thread::sleep_for(m_eventInterval);
		}
	}
private:
	const int m_eventHandle;
	const uint32_t m_numOfEventToDeliver;
	uint32_t m_deliverCnt;
	const std::chrono::milliseconds m_eventInterval;
};

class EventHandler {
public:
	EventHandler(boost::asio::posix::stream_descriptor& socket):
		m_socket(socket),
		m_eventCnt(0),
		m_recvBuffer(0) {
	}

	void onEvent(boost::system::error_code ec, std::size_t received) {
		if (!ec) {
			++m_eventCnt;
			clog << "[" << std::this_thread::get_id() << "] " << "Receive " << received << " bytes" << ", eventCnt=" << m_eventCnt << endl;
			registerEventCallback();
		} else {
		    clog << "[" << std::this_thread::get_id() << "] " << "Receive error code" << endl;
		}
	}

	void registerEventCallback() {
		m_socket.async_read_some(
			boost::asio::buffer(&m_recvBuffer, sizeof(m_recvBuffer)),
			std::bind(
				&EventHandler::onEvent,
				this,
				std::placeholders::_1,
				std::placeholders::_2)
			);
	}
private:
	boost::asio::posix::stream_descriptor& m_socket;
	uint32_t m_eventCnt;
	uint64_t m_recvBuffer;

};

TEST(EventfdTest, TestRaiseAndReceiveEventWithAsio) {
    int efd = eventfd(0, EFD_NONBLOCK);
    ASSERT_NE(-1, efd) << "eventfd return -1";
    io_service ioService;
    boost::asio::posix::stream_descriptor eventSocket(ioService, efd);

    EventSender eventSender {efd, 3, std::chrono::milliseconds(500)};
    std::thread eventSenderThread(&EventSender::start, std::ref(eventSender));

    EventHandler eventHandler {eventSocket};
    eventHandler.registerEventCallback();

    ioService.run();

    eventSenderThread.join();
    close(efd);
}

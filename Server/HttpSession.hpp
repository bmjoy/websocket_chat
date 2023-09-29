#pragma once
#include "SharedState.hpp"
#include "WebsocketSession.hpp"

#include <boost/beast.hpp>

#include <iostream>

class HttpSession : public std::enable_shared_from_this<HttpSession>
{
public:
	HttpSession(boost::asio::ip::tcp::socket socket, std::shared_ptr<SharedState> state)
		: socket_(std::move(socket))
		, state_(state)
	{
	}

	void run()
	{
		boost::beast::http::async_read(socket_, buffer_, req_,
			[self = shared_from_this()](boost::system::error_code error, std::size_t bytes)
			{
				self->onRead(error, bytes);
			});
	}

private:
	void onRead(boost::system::error_code error, std::size_t)
	{
		if (error == boost::beast::http::error::end_of_stream)
		{
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, error);
			return;
		}

		if (error)
		{
			fail(error, "Http onRead");
			return;
		}

		if (boost::beast::websocket::is_upgrade(req_))
		{
			std::make_shared<WebsocketSession<SharedState>>(std::move(socket_), state_)->run(req_);
			return;
		}
	}

	void fail(boost::system::error_code error, const char* where)
	{
		if (error == boost::asio::error::operation_aborted)
		{
			return;
		}

		std::cout << where << ": " << error.message() << std::endl;
	}

	boost::asio::ip::tcp::socket socket_;
	boost::beast::flat_buffer buffer_;
	std::shared_ptr<SharedState> state_;
	boost::beast::http::request<boost::beast::http::string_body> req_;
};
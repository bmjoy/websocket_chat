#pragma once

#include "HttpSession.hpp"

#include <boost/asio.hpp>

#include <iostream>

class Listener : public std::enable_shared_from_this<Listener>
{
public:
	Listener(boost::asio::io_context& ioContext, boost::asio::ip::tcp::endpoint endpoint, std::shared_ptr<SharedState> state)
		: acceptor_(ioContext)
		, socket_(ioContext)
		, state_(state)
	{
		boost::system::error_code error;

		acceptor_.open(endpoint.protocol(), error);

		if (error)
		{
			fail(error, "Listener open");
			return;
		}

		acceptor_.set_option(boost::asio::socket_base::reuse_address(true));

		if (error)
		{
			fail(error, "Listener setOption");
			return;
		}

		acceptor_.bind(endpoint, error);

		if (error)
		{
			fail(error, "Listener bind");
			return;
		}

		acceptor_.listen(boost::asio::socket_base::max_listen_connections, error);

		if (error)
		{
			fail(error, "Listener listen");
			return;
		}
	}

	void run()
	{
		acceptor_.async_accept(socket_,
			[self = shared_from_this()](boost::system::error_code error)
			{
				self->onAccept(error);
			});
	}

private:
	void onAccept(boost::system::error_code error)
	{
		if (error)
		{
			fail(error, "Listener onAccept");
		}
		else
		{
			std::make_shared<HttpSession>(std::move(socket_), state_)->run();
		}

		acceptor_.async_accept(socket_,
			[self = this->shared_from_this()](boost::system::error_code error)
			{
				self->onAccept(error);
			});
	}

	void fail(boost::system::error_code error, const char* where)
	{
		if (error == boost::asio::error::operation_aborted)
		{
			return;
		}

		std::cout << where << ": " << error.message() << std::endl;
	}

	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket socket_;
	std::shared_ptr<SharedState> state_;
};
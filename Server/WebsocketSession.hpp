#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <vector>

template<typename T>
class WebsocketSession : public std::enable_shared_from_this<WebsocketSession<T>>
{
public:
	WebsocketSession(boost::asio::ip::tcp::socket socket, std::shared_ptr<T> state)
		: ws_(std::move(socket))
		, state_(state)
	{}

	~WebsocketSession()
	{
		state_->leave(*this);
	}

	template<typename Body, typename Allocator>
	void run(boost::beast::http::request<Body, Allocator> req)
	{
		ws_.async_accept(req,
			[self = this->shared_from_this()](boost::system::error_code error)
			{
				self->onAccept(error);
			});
	}

	void send(std::shared_ptr<std::string> message)
	{
		messages_.push_back(message);

		if (messages_.size() > 1)
		{
			return;
		}

		ws_.async_write(boost::asio::buffer(*messages_.front()),
			[self = this->shared_from_this()](boost::system::error_code error, std::size_t bytes)
			{
				self->onWrite(error, bytes);
			});
	}

private:

	void onAccept(boost::system::error_code error)
	{
		if (error)
		{
			fail(error, "WebSocket onAccept");
			return;
		}

		state_->join(*this);

		ws_.async_read(buffer_,
			[self = this->shared_from_this()](boost::system::error_code error, std::size_t bytes)
			{
				self->onRead(error, bytes);
			});
	}

	void onRead(boost::system::error_code error, std::size_t)
	{
		if (error)
		{
			fail(error, "WebSocket onRead");
			return;
		}

		state_->send(boost::beast::buffers_to_string(buffer_.data()));

		buffer_.consume(buffer_.size());

		ws_.async_read(buffer_,
			[self = this->shared_from_this()](boost::system::error_code error, std::size_t bytes)
			{
				self->onRead(error, bytes);
			});
	}

	void onWrite(boost::system::error_code error, std::size_t)
	{
		if (error)
		{
			fail(error, "WebSocket onWritte");
			return;
		}

		messages_.erase(messages_.begin());

		if (!messages_.empty())
		{
			ws_.async_write(boost::asio::buffer(*messages_.front()),
				[self = this->shared_from_this()](boost::system::error_code error, std::size_t bytes)
				{
					self->onWrite(error, bytes);
				});
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

	boost::beast::flat_buffer buffer_;
	boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
	std::shared_ptr<T> state_;
	std::vector<std::shared_ptr<std::string>> messages_;
};

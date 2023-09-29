#pragma once

#include "WebsocketSession.hpp"

#include <algorithm>
#include <memory>
#include <unordered_set>

class SharedState
{
public:
	SharedState()
	{
	}

	void join(WebsocketSession<SharedState>& session)
	{
		sessions_.insert(&session);
	}

	void leave(WebsocketSession<SharedState>& session)
	{
		sessions_.erase(&session);
	}

	void send(std::string message)
	{
		auto message_ = std::make_shared<std::string>(std::move(message));

		std::ranges::for_each(sessions_,
			[&](auto session)
			{
				session->send(message_);
			});
	}

private:
	std::unordered_set<WebsocketSession<SharedState>*> sessions_;
};
#pragma once

#include <asio.hpp>
#include <memory>

#include "session.hpp"

class Session;
class RoomMgr;
class AuthManager;

class Server {
public:
  Server(asio::io_context& io, const asio::ip::tcp::endpoint& endpoint);

private:
  void start_accept();
  void handle_accept(Session::pointer session, const std::error_code& e);

  asio::ip::tcp::acceptor acceptor_;
  asio::io_context& io_context_;

  std::unique_ptr<RoomMgr> room_mgr_;
  std::unique_ptr<AuthManager> auth_mgr_;
};
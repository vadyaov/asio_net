#include "client.hpp"
#include "asio/error_code.hpp"

#include <iostream>
#include <system_error>

chat_client::chat_client(asio::io_context &io,
                         tcp::resolver::results_type &endpoints)
    : io_context_(io), socket_(io) {
  do_connect(endpoints);
}

void chat_client::write(const chat_message &msg) {
  asio::post(io_context_, [this, msg]() {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write_header();
    }
  });
}

void chat_client::close() {
  asio::post(io_context_, [this]() { socket_.close(); });
}

void chat_client::do_connect(const tcp::resolver::results_type &endpoints) {
  asio::async_connect(
      socket_, endpoints,
      [this](const asio::error_code &ec, tcp::endpoint /* next */) {
        do_read_header();
      });
}

void chat_client::do_read_header() {
  asio::async_read(
      socket_, asio::buffer(&read_message_.header, sizeof(server_header)),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec /* && header is good */) {
          do_read_body();
        } else {
          socket_.close();
        }
      });
}

void chat_client::do_read_body() {
  read_message_.body.resize(read_message_.header.size);
  asio::async_read(
      socket_,
      asio::buffer(read_message_.body.data(), read_message_.header.size),
      [this](const std::error_code ec, size_t /*bytes_transfered*/) {
        if (!ec) {
          std::cout << read_message_ << std::endl;
          do_read_header();
        } else {
          socket_.close();
        }
      });
}

void chat_client::do_write_header() {
  auto &front_message = write_msgs_.front();
  asio::async_write(
      socket_, asio::buffer(&front_message.header, sizeof(chat_header)),
      [this](const std::error_code &ec, size_t bytes_transferred) {
        if (!ec) {
          do_write_body();
        } else {
          socket_.close();
        }
      });
}

void chat_client::do_write_body() {
  auto &front_message = write_msgs_.front();
  asio::async_write(
      socket_,
      asio::buffer(front_message.body.data(), front_message.header.size),
      [this](const std::error_code &ec, size_t /*bytes_transferred*/) {
        if (!ec) {
          write_msgs_.pop_front();
          if (!write_msgs_.empty()) {
            do_write_header();
          }
        } else {
          socket_.close();
        }
      });
}
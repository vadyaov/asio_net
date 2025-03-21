#include "command.h"
#include "room.hpp"
#include <iostream>

void RoomTextCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
    std::cout << "Executing RoomTextCommand:" << std::endl;

    server_message answer;
    answer.header.id = ServerResponceType::OK;
    answer.header.size = message.header.size;
    answer.body = message.body;

    if (ChatRoom* chat_room = dynamic_cast<ChatRoom*>(context); chat_room) {
      chat_room->deliverAll(std::move(answer));
    }
}

void LobbyTextCommand::execute(chat_message& message, participant_ptr sender, IRoom* /*context*/) {
  std::cout << "Executing LobbyTextCommand:" << std::endl;

  server_message answer;
  answer.header.id = ServerResponceType::OK;
  answer.header.size = message.header.size;
  answer.body = message.body;

  sender->deliver(answer);
}

void NotImplementedCommand::execute(chat_message& message, participant_ptr sender, IRoom* /*context*/) {
  std::cout << "Executing NotImplementedCommand:" << std::endl;
  server_message answer;
  answer.header.id = ServerResponceType::UNKNOWN_REQUEST;
  answer.AppendString("Not implemented yet.");
  sender->deliver(answer);
}

void InvalidContextCommand::execute(chat_message& message, participant_ptr sender, IRoom* /*context*/) {
  std::cout << "Executing InvalidContextCommand:" << std::endl;
  server_message answer;
  answer.header.id = ServerResponceType::INVALID_CONTEXT;
  sender->deliver(answer);
}

void GetNameCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
  std::cout << "Executing GetNameCommand:" << std::endl;
  server_message answer;
  answer.header.id = ServerResponceType::OK;
  answer.AppendString(context->name());
  sender->deliver(answer);
}

void QuitCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
  std::cout << "Executing QuitCommand:" << std::endl;
  // server_message answer;
  // answer.header.id = ServerResponceType::OK;
  context->leave(sender);
}

void UnknownCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
  server_message answer;
  answer.header.id = ServerResponceType::UNKNOWN_REQUEST;
  answer.AppendString("Unknown request.");
  sender->deliver(answer);
}

void CreateRoomCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
  std::cout << "Executing CreateRoomCommand:" << std::endl;
  std::cout << "Offset = " << message.offset << "; " << message << std::endl;
  // message.offset = 0;
  server_message answer;
  if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) {
    std::string room_id;
    message.ExtractString(room_id);
    answer.header.id = lobby->createRoom(room_id, sender);
    if (answer.header.id == ServerResponceType::OK)
      answer.AppendString(room_id + " created.");
  } else {
    answer.header.id = ServerResponceType::INTERNAL_ERROR;
    answer.AppendString("dynamic_cast error");
  }
  sender->deliver(answer);
}

void DeleteRoomCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
  std::cout << "Executing DeleteRoomCommand:" << std::endl;
  server_message answer;
  if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) {
    std::string room_id;
    message.ExtractString(room_id);
    answer.header.id = lobby->deleteRoom(room_id, sender);
    if (answer.header.id == ServerResponceType::OK)
      answer.AppendString(room_id + " created.");
  } else {
    answer.header.id = ServerResponceType::INTERNAL_ERROR;
    answer.AppendString("dynamic_cast error");
  }
  sender->deliver(answer);
}

void JoinRoomCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
  std::cout << "Executing JoinRoomCommand:" << std::endl;
  server_message answer;
  if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) {
    std::string room_id;
    message.ExtractString(room_id);
    answer.header.id = lobby->moveParticipantToRoom(room_id, sender);
    if (answer.header.id == ServerResponceType::OK)
      answer.AppendString("joined room " + room_id);
  } else {
    answer.header.id = ServerResponceType::INTERNAL_ERROR;
    answer.AppendString("dynamic_cast error");
  }
  sender->deliver(answer);
}

void ListRoomCommand::execute(chat_message& message, participant_ptr sender, IRoom* context) {
  std::cout << "Executing ListRoomCommand:" << std::endl;
  server_message answer;
  if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) { 
    answer.header.id = ServerResponceType::OK;
    for (auto& room : lobby->listRooms())
      answer.AppendString(room);
  } else {
    answer.header.id = ServerResponceType::INTERNAL_ERROR;
    answer.AppendString("dynamic_cast error");
  }
  sender->deliver(answer);
}

std::unique_ptr<ICommand> ChatRoomCommandFactory::createCommand(ChatMessageType type) {
  std::cout << "Executing ChatRoomCommandFactory::CreateCommand" << std::endl;
  std::unique_ptr<ICommand> command;
  if (type == ChatMessageType::TEXT) {
    command = std::make_unique<RoomTextCommand>();
  } else if (type == ChatMessageType::LOGIN) {
    command = std::make_unique<NotImplementedCommand>();
  } else if (type == ChatMessageType::LOGOUT) {
    command = std::make_unique<NotImplementedCommand>();
  } else if (type == ChatMessageType::CREATE) {
    command = std::make_unique<InvalidContextCommand>();
  } else if (type == ChatMessageType::DELETE) {
    command = std::make_unique<InvalidContextCommand>();
  } else if (type == ChatMessageType::JOIN) {
    command = std::make_unique<InvalidContextCommand>();
  } else if (type == ChatMessageType::LIST) {
    command = std::make_unique<InvalidContextCommand>();
  } else if (type == ChatMessageType::ROOM) {
    command = std::make_unique<GetNameCommand>();
  } else if (type == ChatMessageType::QUIT) {
    command = std::make_unique<QuitCommand>();
  }
  
  return command;
}

std::unique_ptr<ICommand> LobbyCommandFactory::createCommand(ChatMessageType type) {
  std::cout << "Executing LobbyCommandFactory::CreateCommand" << std::endl;
  std::unique_ptr<ICommand> command;
  if (type == ChatMessageType::TEXT) {
    command = std::make_unique<LobbyTextCommand>();
  } else if (type == ChatMessageType::LOGIN) {
    command = std::make_unique<NotImplementedCommand>();
  } else if (type == ChatMessageType::LOGOUT) {
    command = std::make_unique<NotImplementedCommand>();
  } else if (type == ChatMessageType::CREATE) {
    command = std::make_unique<CreateRoomCommand>();
  } else if (type == ChatMessageType::DELETE) {
    command = std::make_unique<DeleteRoomCommand>();
  } else if (type == ChatMessageType::JOIN) {
    command = std::make_unique<JoinRoomCommand>();
  } else if (type == ChatMessageType::LIST) {
    command = std::make_unique<ListRoomCommand>();
  } else if (type == ChatMessageType::ROOM) {
    command = std::make_unique<GetNameCommand>();
  } else if (type == ChatMessageType::QUIT) {
    command = std::make_unique<QuitCommand>();
  } else if (type == ChatMessageType::UNKNOWN) {
    command = std::make_unique<UnknownCommand>();
  }
  
  return command;
}

void ICommandHandler::initHandlers() {
  handlers_[ChatMessageType::TEXT] = command_factory_->createCommand(ChatMessageType::TEXT);
  handlers_[ChatMessageType::LOGIN] = command_factory_->createCommand(ChatMessageType::LOGIN);
  handlers_[ChatMessageType::LOGOUT] = command_factory_->createCommand(ChatMessageType::LOGOUT);
  handlers_[ChatMessageType::CREATE] = command_factory_->createCommand(ChatMessageType::CREATE);
  handlers_[ChatMessageType::DELETE] = command_factory_->createCommand(ChatMessageType::DELETE);
  handlers_[ChatMessageType::JOIN] = command_factory_->createCommand(ChatMessageType::JOIN);
  handlers_[ChatMessageType::LIST] = command_factory_->createCommand(ChatMessageType::LIST);
  handlers_[ChatMessageType::ROOM] = command_factory_->createCommand(ChatMessageType::ROOM);
  handlers_[ChatMessageType::QUIT] = command_factory_->createCommand(ChatMessageType::QUIT);
  handlers_[ChatMessageType::UNKNOWN] = command_factory_->createCommand(ChatMessageType::UNKNOWN);
}

void ICommandHandler::process(chat_message& message, participant_ptr sender, IRoom* context) {
  handlers_[message.header.id]->execute(message, sender, context);
}
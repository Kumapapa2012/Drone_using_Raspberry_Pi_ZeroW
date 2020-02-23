//
// Created by Leonid  on 2019-06-07.
//

#include "WSListener.hpp"

// sbus service global instance
extern SBUS_CONTROLLER::Sbus_Service sbus_service;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WSListener

void WSListener::onPing(const WebSocket& socket, const oatpp::String& message) {
  OATPP_LOGD(TAG, "onPing");
  socket.sendPong(message);
}

void WSListener::onPong(const WebSocket& socket, const oatpp::String& message) {
  OATPP_LOGD(TAG, "onPong");
}

void WSListener::onClose(const WebSocket& socket, v_word16 code, const oatpp::String& message) {
  OATPP_LOGD(TAG, "onClose code=%d", code);
}

void WSListener::readMessage(const WebSocket& socket, v_word8 opcode, p_char8 data, oatpp::data::v_io_size size) {

  if (size == 0) { // message transfer finished
    auto wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.clear();
    sbus_service.sbusWrite(wholeMessage->std_str());

    return socket.sendOneFrameText( "Hello from oatpp!: ");

  } else if (size > 0) { // message frame received
    m_messageBuffer.write(data, size);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WSInstanceListener

std::atomic<v_int32> WSInstanceListener::SOCKETS(0);

void WSInstanceListener::onAfterCreate(const oatpp::websocket::WebSocket& socket, 
                  const std::shared_ptr<const ParameterMap>& params) {
  SOCKETS++;
  OATPP_LOGD(TAG, "New Incoming Connection. Connection count=%d",
             SOCKETS.load());

  /* In this particular case we create one WSListener per each connection */
  /* Which may be redundant in many cases */
  socket.setListener(std::make_shared<WSListener>());
  sbus_service.activate();
}

void WSInstanceListener::onBeforeDestroy(const oatpp::websocket::WebSocket& socket) {

  SOCKETS--;
  OATPP_LOGD(TAG, "Connection closed. Connection count=%d", SOCKETS.load());
    sbus_service.deactivate();
}
//
// Created by Administrator on 2022/11/20.
//

#include "V8AbstractInspectorChannel.h"
#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "libplatform/libplatform.h"
#include "v8-inspector.h"
#include "v8.h"
#include <codecvt>
#include <locale>
#include <string>
#include <functional>
#include "V8AbstractInspector.h"
#include "V8InspectorChannelImpl.h"

V8InspectorChannelImpl::V8InspectorChannelImpl(
        const std::unique_ptr<v8_inspector::V8Inspector> &InV8Inspector, const int32_t InCxtGroupID) {
    v8_inspector::StringView DummyState;
    V8InspectorSession = InV8Inspector->connect(InCxtGroupID, this, DummyState);
}

void V8InspectorChannelImpl::DispatchProtocolMessage(const std::string &Message) {
    const auto MessagePtr = reinterpret_cast<const uint8_t *>(Message.c_str());
    const auto MessageLen = (size_t) Message.length();

    v8_inspector::StringView StringView(MessagePtr, MessageLen);

    V8InspectorSession->dispatchProtocolMessage(StringView);
}

void V8InspectorChannelImpl::OnMessage(std::function<void(const std::string &)> Handler) {
    OnSendMessage = Handler;
}

void V8InspectorChannelImpl::SendMessage(v8_inspector::StringBuffer &MessageBuffer) {
    v8_inspector::StringView MessageView = MessageBuffer.string();

    std::string Message;
    if (MessageView.is8Bit()) {
        Message = reinterpret_cast<const char *>(MessageView.characters8());
    } else {
#if _WIN32
        std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t> Conv;
        const uint16_t *Start = MessageView.characters16();
#else
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> Conv;
        const char16_t *Start = reinterpret_cast<const char16_t *>(MessageView.characters16());
#endif
        Message = Conv.to_bytes(Start, Start + MessageView.length());
    }

    if (OnSendMessage)
        OnSendMessage(Message);
}

void V8InspectorChannelImpl::sendResponse(int /* CallID */, std::unique_ptr<v8_inspector::StringBuffer> Message) {
    SendMessage(*Message);
}

void V8InspectorChannelImpl::sendNotification(std::unique_ptr<v8_inspector::StringBuffer> Message) {
    SendMessage(*Message);
}
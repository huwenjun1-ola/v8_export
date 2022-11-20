//
// Created by Administrator on 2022/11/20.
//

#ifndef V8_EXPORT_V8INSPECTORCHANNELIMPL_H
#define V8_EXPORT_V8INSPECTORCHANNELIMPL_H

#include "V8AbstractInspectorChannel.h"
#include "V8AbstractInspector.h"

class V8InspectorChannelImpl : public v8_inspector::V8Inspector::Channel, public V8AbstractInspectorChannel {
public:
    V8InspectorChannelImpl(const std::unique_ptr<v8_inspector::V8Inspector> &InV8Inspector,
                           const int32_t InCxtGroupID);

    void DispatchProtocolMessage(const std::string &Message) override;

    void OnMessage(std::function<void(const std::string &)> Handler) override;

    virtual ~V8InspectorChannelImpl() override {
        OnSendMessage = nullptr;
    }

private:
    void SendMessage(v8_inspector::StringBuffer &MessageBuffer);

    void sendResponse(int CallID, std::unique_ptr<v8_inspector::StringBuffer> Message) override;

    void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> Message) override;

    void flushProtocolNotifications() override {
    }

    std::unique_ptr<v8_inspector::V8InspectorSession> V8InspectorSession;

    std::function<void(const std::string &)> OnSendMessage;
};

#endif //V8_EXPORT_V8INSPECTORCHANNELIMPL_H

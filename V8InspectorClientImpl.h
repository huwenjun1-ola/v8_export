//
// Created by Administrator on 2022/11/20.
//

#ifndef V8_EXPORT_V8INSPECTORCLIENTIMPL_H
#define V8_EXPORT_V8INSPECTORCLIENTIMPL_H

#include "V8AbstractInspectorChannel.h"
#include "V8AbstractInspector.h"
#include "V8InspectorChannelImpl.h"
#include <codecvt>
#include <locale>
#include <string>
#include <functional>

class V8InspectorClientImpl : public V8AbstractInspector, public v8_inspector::V8InspectorClient {
public:

    using wspp_connection_hdl = websocketpp::connection_hdl;


    using wspp_exception = websocketpp::exception;

    V8InspectorClientImpl(int32_t InPort, v8::Local<v8::Context> InContext);

    virtual ~V8InspectorClientImpl();

    void Close() override;

    bool NetAndTaskRunnerTick(float DeltaTime);

    bool Tick() override;

    bool IsALive() override;

    V8AbstractInspectorChannel *CreateV8InspectorChannel() override;

private:
    void OnHTTP(wspp_connection_hdl Handle);

    void OnOpen(wspp_connection_hdl Handle);

    void OnReceiveMessage(wspp_connection_hdl Handle, wspp_server::message_ptr Message);

    void OnSendMessage(wspp_connection_hdl Handle, const std::string &Message);

    void OnClose(wspp_connection_hdl Handle);

    void OnFail(wspp_connection_hdl Handle);

    void runMessageLoopOnPause(int ContextGroupId) override;

    void quitMessageLoopOnPause() override;

    void runIfWaitingForDebugger(int ContextGroupId) override {
        Connected = true;
    }

    v8::Isolate *Isolate;

    v8::Persistent<v8::Context> Context;

    v8::Persistent<v8::Function> MicroTasksRunner;

    int32_t Port;

    std::unique_ptr<v8_inspector::V8Inspector> V8Inspector;

    int32_t CtxGroupID;

    std::map<void *, V8InspectorChannelImpl *> V8InspectorChannels;

    websocketpp::server<websocketpp::config::asio> Server;

    std::string JSONVersion;

    std::string JSONList;

    bool IsAlive;

    bool IsPaused;

    bool Connected;
};

#endif //V8_EXPORT_V8INSPECTORCLIENTIMPL_H

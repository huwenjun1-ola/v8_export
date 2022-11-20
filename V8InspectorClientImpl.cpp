//
// Created by Administrator on 2022/11/20.
//

#include "V8InspectorChannelImpl.h"
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
#include "V8InspectorClientImpl.h"

void MicroTasksRunnerFunction(const v8::FunctionCallbackInfo<v8::Value> &Info) {
    // throw an error so the v8 will clean pending exception later
    Info.GetIsolate()->ThrowException(
            v8::Exception::Error(v8::String::NewFromUtf8(Info.GetIsolate(), "test",
                                                         v8::NewStringType::kNormal).ToLocalChecked()));
}

V8InspectorClientImpl::V8InspectorClientImpl(int32_t InPort, v8::Local<v8::Context> InContext) {
    Isolate = InContext->GetIsolate();
    Context.Reset(Isolate, InContext);
    MicroTasksRunner.Reset(
            Isolate,
            v8::FunctionTemplate::New(Isolate, MicroTasksRunnerFunction)->GetFunction(InContext).ToLocalChecked());
    Port = InPort;
    IsAlive = false;
    Connected = false;

    CtxGroupID = 1;
    const uint8_t CtxNameConst[] = "V8InspectorContext";
    v8_inspector::StringView CtxName(CtxNameConst, sizeof(CtxNameConst) - 1);
    V8Inspector = v8_inspector::V8Inspector::create(Isolate, this);
    V8Inspector->contextCreated(v8_inspector::V8ContextInfo(InContext, CtxGroupID, CtxName));

    if (Port < 0)
        return;

    try {
        Server.set_reuse_addr(true);
        Server.set_access_channels(websocketpp::log::alevel::none);
        Server.set_error_channels(websocketpp::log::elevel::none);

        Server.set_http_handler(std::bind(&V8InspectorClientImpl::OnHTTP, this, std::placeholders::_1));
        Server.set_open_handler(std::bind(&V8InspectorClientImpl::OnOpen, this, std::placeholders::_1));
        Server.set_message_handler(
                std::bind(&V8InspectorClientImpl::OnReceiveMessage, this, std::placeholders::_1,
                          std::placeholders::_2));
        Server.set_close_handler(std::bind(&V8InspectorClientImpl::OnClose, this, std::placeholders::_1));
        Server.set_fail_handler(std::bind(&V8InspectorClientImpl::OnFail, this, std::placeholders::_1));

        Server.init_asio();
        Server.listen(Port);
        Server.start_accept();

        JSONVersion = R"({
        "Browser": "Puerts/v1.0.0",
        "Protocol-Version": "1.1"
        })";

        JSONList = R"([{
        "description": "Puerts Inspector",
        "id": "0",
        "title": "Puerts Inspector",
        "type": "node",
        )";
        JSONList += "\"webSocketDebuggerUrl\"";
        JSONList += ":";
        JSONList += "\"ws://127.0.0.1:";
        JSONList += std::to_string(Port) + "\"\r\n}]";

        IsAlive = true;


    }
    catch (const websocketpp::exception &Exception) {
        IsAlive = false;
    }

    IsPaused = false;
}

V8AbstractInspectorChannel *V8InspectorClientImpl::CreateV8InspectorChannel() {
    return new V8InspectorChannelImpl(V8Inspector, CtxGroupID);
}

V8InspectorClientImpl::~V8InspectorClientImpl() {
    Close();
}

void V8InspectorClientImpl::Close() {
    if (IsAlive) {
#ifdef THREAD_SAFE
        v8::Locker Locker(Isolate);
#endif
        Server.stop_listening();
        for (auto Iter = V8InspectorChannels.begin(); Iter != V8InspectorChannels.end(); ++Iter) {
            delete Iter->second;
        }
        V8InspectorChannels.clear();

        v8::Isolate::Scope IsolateScope(Isolate);
        v8::HandleScope HandleScope(Isolate);
        V8Inspector->contextDestroyed(Context.Get(Isolate));
        IsAlive = false;
        IsPaused = false;
    }
}

bool V8InspectorClientImpl::NetAndTaskRunnerTick(float /* DeltaTime */) {
    try {
        if (IsAlive) {

            v8::Locker Locker(Isolate);


            {
                Server.poll();

                v8::Isolate::Scope IsolateScope(Isolate);
                v8::HandleScope HandleScope(Isolate);
                auto LocalContext = Context.Get(Isolate);
                v8::Context::Scope ContextScope(LocalContext);
                v8::TryCatch TryCatch(Isolate);

                MicroTasksRunner.Get(Isolate)->Call(LocalContext, LocalContext->Global(), 0, nullptr);
            }
        }
    }
    catch (const wspp_exception &Exception) {
    }
    return true;
}

bool V8InspectorClientImpl::Tick() {
    NetAndTaskRunnerTick(0);
    return IsAlive && Connected;
}

bool V8InspectorClientImpl::IsALive() {
    return IsAlive;
}

void V8InspectorClientImpl::OnHTTP(wspp_connection_hdl Handle) {
    try {
        auto Connection = Server.get_con_from_hdl(Handle);
        auto Resource = Connection->get_resource();

        if (Resource == "/json" || Resource == "/json/list") {
            Connection->set_body(JSONList);
            Connection->set_status(websocketpp::http::status_code::ok);
        } else if (Resource == "/json/version") {
            Connection->set_body(JSONVersion);
            Connection->set_status(websocketpp::http::status_code::ok);
        } else {
            Connection->set_body("404 Not Found");
            Connection->set_status(websocketpp::http::status_code::not_found);
        }
    }
    catch (const wspp_exception &Exception) {
    }
}

void V8InspectorClientImpl::OnOpen(wspp_connection_hdl Handle) {
    V8InspectorChannelImpl *channel = new V8InspectorChannelImpl(V8Inspector, CtxGroupID);
    V8InspectorChannels[Handle.lock().get()] = channel;
    channel->OnMessage(std::bind(&V8InspectorClientImpl::OnSendMessage, this, Handle, std::placeholders::_1));
}

void V8InspectorClientImpl::OnReceiveMessage(wspp_connection_hdl Handle, wspp_server::message_ptr Message) {
    auto channel = V8InspectorChannels[Handle.lock().get()];

    {
        // v8::Locker Locker(Isolate);
        v8::Isolate::Scope IsolateScope(Isolate);
        v8::SealHandleScope scope(Isolate);
        channel->DispatchProtocolMessage(Message->get_payload());
    }
}

void V8InspectorClientImpl::OnSendMessage(wspp_connection_hdl Handle, const std::string &Message) {
    try {
        Server.send(Handle, Message, websocketpp::frame::opcode::TEXT);
    }
    catch (const websocketpp::exception &Exception) {
    }
}

void V8InspectorClientImpl::OnClose(wspp_connection_hdl Handle) {
    void *HandlePtr = Handle.lock().get();
    delete V8InspectorChannels[HandlePtr];
    V8InspectorChannels.erase(HandlePtr);
}

void V8InspectorClientImpl::OnFail(wspp_connection_hdl Handle) {
}

void V8InspectorClientImpl::runMessageLoopOnPause(int /* ContextGroupId */) {
    if (IsPaused) {
        return;
    }

    IsPaused = true;

    while (IsPaused) {
        NetAndTaskRunnerTick(0);
    }
}

void V8InspectorClientImpl::quitMessageLoopOnPause() {
    IsPaused = false;
}
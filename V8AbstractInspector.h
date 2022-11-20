#ifndef V8InspectorImpl_H
#define V8InspectorImpl_H
#pragma warning(push)
#pragma warning(disable : 4251)

#include "v8.h"
#include "v8-inspector.h"
#include "libplatform/libplatform.h"

#pragma warning(pop)

#include "V8AbstractInspectorChannel.h"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> wspp_server;
typedef wspp_server::message_ptr wspp_message_ptr;

class V8AbstractInspector {
public:
    virtual void Close() = 0;

    virtual bool Tick() = 0;

    virtual bool IsALive() = 0;

    virtual V8AbstractInspectorChannel *CreateV8InspectorChannel() = 0;

    virtual ~V8AbstractInspector() {
    }
};

#endif
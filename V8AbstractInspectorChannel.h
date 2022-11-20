//
// Created by Administrator on 2022/11/20.
//

#ifndef V8_EXPORT_V8ABSTRACTINSPECTORCHANNEL_H
#define V8_EXPORT_V8ABSTRACTINSPECTORCHANNEL_H

#include <string>
#include <functional>

class V8AbstractInspectorChannel {
public:
    virtual void DispatchProtocolMessage(const std::string &Message) = 0;

    virtual void OnMessage(std::function<void(const std::string &)> Handler) = 0;

    virtual ~V8AbstractInspectorChannel() {
    }
};

#include <stdint.h>
#include <functional>
#include <string>

#endif //V8_EXPORT_V8ABSTRACTINSPECTORCHANNEL_H

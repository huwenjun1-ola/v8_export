//
// Created by Administrator on 2022/11/20.
//

#ifndef V8_EXPORT_V8INSPECTORINTERFACE_H
#define V8_EXPORT_V8INSPECTORINTERFACE_H


#include "V8AbstractInspector.h"
#include "V8InspectorClientImpl.h"



V8AbstractInspector *CreateV8Inspector(int32_t Port, void *InContextPtr) {
    v8::Local<v8::Context> *ContextPtr = static_cast<v8::Local<v8::Context> *>(InContextPtr);
    return new V8InspectorClientImpl(Port, *ContextPtr);
}


#endif //V8_EXPORT_V8INSPECTORINTERFACE_H

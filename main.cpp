//
// Created by 郑凯凯 on 2023/5/11.
//
#include "v8_export.h"

int main() {
    Init();
    auto iso = NewIsolate(1);
    auto ctx = NewContext(iso, nullptr, 1);
    auto data = malloc(15);
    auto a=NewValueArray(iso, nullptr,0);
    printf("%s",a);
    IsolateDispose(iso);
    free(data);
}
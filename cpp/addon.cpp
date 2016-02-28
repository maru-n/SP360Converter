#include <node.h>
#include "converter_wrap.h"

using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports, Local<Object> module) {
    ConverterWrap::Init(module);
}

NODE_MODULE(addon, InitAll)

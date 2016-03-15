#ifndef CONVERTER_WRAP_H
#define CONVERTER_WRAP_H

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include "converter.h"

class ConverterWrap : public node::ObjectWrap {
    public:
        static void Init(v8::Local<v8::Object> module);

        SP360::Converter* converter;

        uv_work_t  convertRequest;
        uv_async_t convertAsync;
        std::string convertDstFile;
        v8::Persistent<v8::Function> convertCallback;
        double convertProgress;
        std::string convertError;
        std::string convertStatus;

    private:
        explicit ConverterWrap();
        ~ConverterWrap();
        static v8::Persistent<v8::Function> constructor;
        static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void MakeOriginalPreviewImage(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void MakeConvertedPreviewImage(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Convert(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void TotalFrame(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void IsOpened(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void FPS(const v8::FunctionCallbackInfo<v8::Value>& args);
    };

#endif

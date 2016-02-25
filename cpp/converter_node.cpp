#define _USE_MATH_DEFINES
#include <math.h>
#include <node.h>
#include <v8.h>
#include <uv.h>
#include <string>
#include <iostream>
#include <functional>
#include "converter.h"

using namespace v8;

struct Work {
  uv_work_t  request;
  uv_async_t progress_async;
  Persistent<Function> callback;

  std::string src_file;
  std::string dst_file;
  unsigned int start_time;
  unsigned int end_time;
  unsigned int preview_time;
  unsigned int dst_width;
  unsigned int dst_height;
  double angle_start;
  double angle_end;
  double radius_in;
  double radius_out;
  int n_split;

  std::string error;
  std::string status;
  double progress;
};

Work* work;


void callNodeCallback(Work* work) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate); // Required for Node 4.x
    Local<Primitive> error;
    if (work->error == "") {
        error = v8::Null(isolate);
    } else {
        error = v8::String::NewFromUtf8(isolate, work->error.c_str());
    }
    Local<Value> status = v8::String::NewFromUtf8(isolate, work->status.c_str());
    Local<Number> progress = v8::Number::New(isolate, work->progress);
    Handle<Value> argv[] = { error, status, progress };
    Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext()->Global(), 3, argv);
}


void convertProgress(uv_async_t *handle) {
    Work *work = static_cast<Work *>(handle->data);
    callNodeCallback(work);
}


void convertAsync(uv_work_t *req) {
    Work *work = static_cast<Work *>(req->data);
    work->status = "progress";
    std::function<void(float)> progress_callback = [&work](float progress){
        work->progress = progress;
        uv_async_send(&work->progress_async);
    };
    SP360::convertMovie(
        work->src_file,
        work->dst_file,
        work->dst_width,
        work->dst_height,
        work->start_time,
        work->end_time,
        work->angle_start,
        work->angle_end,
        work->radius_in,
        work->radius_out,
        work->n_split,
        progress_callback);
}


void convertAfter(uv_work_t *req, int status) {
    Work *work = static_cast<Work *>(req->data);
    work->status = "successed";
    work->progress = 1.0;
    callNodeCallback(work);
    work->callback.Reset();
    uv_close((uv_handle_t*) &work->progress_async, NULL);
    //delete work;
}


void convertMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Function> callback = Local<Function>::Cast(args[0]);
    work->callback.Reset(isolate, callback);

    uv_async_init(uv_default_loop(), &work->progress_async, convertProgress);
    uv_queue_work(uv_default_loop(), &work->request, convertAsync, convertAfter);

    args.GetReturnValue().Set(Undefined(isolate));
}


void makePreviewImageMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    if (!args[1]->IsNumber() || !args[2]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Wrong arguments")));
        return;
    }
    Local<Uint8ClampedArray> array = args[0].As<Uint8ClampedArray>();
    unsigned char* dst_img_ptr = (unsigned char*)array->Buffer()->GetContents().Data();
    int width = args[1]->NumberValue();
    int height = args[2]->NumberValue();
    bool border = args[3]->BooleanValue();
    if (border) {
        SP360::makeConvertBorderImage(
            work->src_file, dst_img_ptr,
            width, height,
            work->preview_time,
            work->angle_start,
            work->angle_end,
            work->radius_in,
            work->radius_out,
            work->n_split,
            1024, 256);
    } else {
        SP360::makeImage(work->src_file, dst_img_ptr, width, height, work->preview_time);
    }
    args.GetReturnValue().Set(Undefined(isolate));
}


void makeConvertedPreviewImageMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    if (!args[1]->IsNumber() || !args[2]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Wrong arguments")));
        return;
    }
    Local<Uint8ClampedArray> array = args[0].As<Uint8ClampedArray>();
    unsigned char* dst_img_ptr = (unsigned char*)array->Buffer()->GetContents().Data();
    int width = args[1]->NumberValue();
    int height = args[2]->NumberValue();
    SP360::makeConvertedImage(
        work->src_file, dst_img_ptr,
        width, height,
        work->preview_time,
        work->angle_start,
        work->angle_end,
        work->radius_in,
        work->radius_out,
        work->n_split);

    args.GetReturnValue().Set(Undefined(isolate));
}


void setupMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();

    Handle<Object> data = Handle<Object>::Cast(args[0]);
    v8::String::Utf8Value src_file_utf(data->Get(String::NewFromUtf8(isolate,"src_file")));
    v8::String::Utf8Value dst_file_utf(data->Get(String::NewFromUtf8(isolate,"dst_file")));
    work->src_file = std::string(*src_file_utf);
    work->dst_file = std::string(*dst_file_utf);
    if (data->Get(String::NewFromUtf8(isolate,"start_time"))->IsNumber())
        work->start_time = data->Get(String::NewFromUtf8(isolate,"start_time"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"end_time"))->IsNumber())
        work->end_time = data->Get(String::NewFromUtf8(isolate,"end_time"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"preview_time"))->IsNumber())
        work->preview_time = data->Get(String::NewFromUtf8(isolate,"preview_time"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"dst_width"))->IsNumber())
        work->dst_width = data->Get(String::NewFromUtf8(isolate,"dst_width"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"dst_height"))->IsNumber())
        work->dst_height = data->Get(String::NewFromUtf8(isolate,"dst_height"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"angle_start"))->IsNumber())
        work->angle_start = data->Get(String::NewFromUtf8(isolate,"angle_start"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"angle_end"))->IsNumber())
        work->angle_end = data->Get(String::NewFromUtf8(isolate,"angle_end"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"radius_in"))->IsNumber())
        work->radius_in = data->Get(String::NewFromUtf8(isolate,"radius_in"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"radius_out"))->IsNumber())
        work->radius_out = data->Get(String::NewFromUtf8(isolate,"radius_out"))->NumberValue();
    if (data->Get(String::NewFromUtf8(isolate,"n_split"))->IsNumber())
        work->n_split  = data->Get(String::NewFromUtf8(isolate,"n_split"))->NumberValue();
    args.GetReturnValue().Set(Undefined(isolate));
}


void init(Local<Object> exports) {
    work = new Work();
    work->request.data = work;
    work->progress_async.data = work;
    NODE_SET_METHOD(exports, "setup", setupMethod);
    NODE_SET_METHOD(exports, "convert", convertMethod);
    NODE_SET_METHOD(exports, "makePreviewImage", makePreviewImageMethod);
    NODE_SET_METHOD(exports, "makeConvertedPreviewImage", makeConvertedPreviewImageMethod);
}

NODE_MODULE(converter, init)

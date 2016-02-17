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
  int start_time;
  int end_time;
  int dst_width;
  int dst_height;
  int n_split;
  double start_theta;
  double end_theta;

  std::string error;
  std::string status;
  double progress;
};


void call_node_callback(Work* work) {
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


void convert_progress(uv_async_t *handle) {
    Work *work = static_cast<Work *>(handle->data);
    call_node_callback(work);
}


void convert_async(uv_work_t *req) {
    Work *work = static_cast<Work *>(req->data);
    work->status = "progress";
    std::function<void(float)> progress_callback = [&work](float progress){
        work->progress = progress;
        uv_async_send(&work->progress_async);
    };
    convert(work->src_file,
            work->dst_file,
            work->dst_width,
            work->dst_height,
            work->start_time,
            work->end_time,
            work->n_split,
            work->start_theta,
            work->end_theta,
            progress_callback
        );
}


void convert_after(uv_work_t *req, int status)
{
    Work *work = static_cast<Work *>(req->data);
    work->status = "successed";
    call_node_callback(work);
    work->callback.Reset();
    uv_close((uv_handle_t*) &work->progress_async, NULL);
    delete work;
}


void convert_method(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Handle<Object> data = Handle<Object>::Cast(args[0]);

    v8::String::Utf8Value src_file_utf(data->Get(String::NewFromUtf8(isolate,"src_file")));
    std::string src_file = std::string(*src_file_utf);
    v8::String::Utf8Value dst_file_utf(data->Get(String::NewFromUtf8(isolate,"dst_file")));
    std::string dst_file = std::string(*dst_file_utf);
    int start_time       = data->Get(String::NewFromUtf8(isolate,"start_time"))->NumberValue();
    int end_time         = data->Get(String::NewFromUtf8(isolate,"end_time"))->NumberValue();
    int dst_width        = data->Get(String::NewFromUtf8(isolate,"dst_width"))->NumberValue();
    int dst_height       = data->Get(String::NewFromUtf8(isolate,"dst_height"))->NumberValue();
    int n_split          = data->Get(String::NewFromUtf8(isolate,"n_split"))->NumberValue();
    double start_theta   = data->Get(String::NewFromUtf8(isolate,"start_theta"))->NumberValue();
    double end_theta     = data->Get(String::NewFromUtf8(isolate,"end_theta"))->NumberValue();
    Local<Function> callback = Local<Function>::Cast(args[1]);

    Work* work = new Work();
    work->src_file    = src_file;
    work->dst_file    = dst_file;
    work->start_time  = start_time;
    work->end_time    = end_time;
    work->dst_width   = dst_width;
    work->dst_height  = dst_height;
    work->n_split     = n_split;
    work->start_theta = start_theta;
    work->end_theta   = end_theta;
    work->callback.Reset(isolate, callback);
    work->request.data = work;
    work->progress_async.data = work;

    uv_async_init(uv_default_loop(), &work->progress_async, convert_progress);
    uv_queue_work(uv_default_loop(), &work->request, convert_async, convert_after);

    args.GetReturnValue().Set(Undefined(isolate));
}


void init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "convert", convert_method);
}

NODE_MODULE(converter, init)

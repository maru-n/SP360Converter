#define _USE_MATH_DEFINES
#include <math.h>
#include <node.h>
#include <v8.h>
#include <uv.h>
#include <string>
#include <iostream>
#include "converter.h"

#include <functional>

#include "unistd.h"

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
  double progress;
};

void call_node_callback(Work* work) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate); // Required for Node 4.x

    Local<Primitive> error  = v8::Null(isolate);
    Local<Primitive> status = v8::Null(isolate);
    Local<Number> progress  = v8::Number::New(isolate, work->progress);
    Handle<Value> argv[] = { error, status, progress };
    Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext()->Global(), 3, argv);
}

void convert_progress(uv_async_t *handle) {
    Work *work = static_cast<Work *>(handle->data);
    call_node_callback(work);
}


void convert_async(uv_work_t *req) {
    Work *work = static_cast<Work *>(req->data);

    std::function<void(float)> callback = [&work](float progress){
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
            callback
        );
}


void convert_after(uv_work_t *req, int status)
{
    Work *work = static_cast<Work *>(req->data);
    call_node_callback(work);

    work->callback.Reset();
    uv_close((uv_handle_t*) &work->progress_async, NULL);
    delete work;
}


void convert_method(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();

    Handle<Object> data = Handle<Object>::Cast(args[0]);
    std::string src_file("/Users/maruyama/Desktop/SP360Converter/sample/SP01_04.MP4");
    std::string dst_file("/Users/maruyama/Desktop/SP360Converter/test.mp4");
    int start_time = data->Get(String::NewFromUtf8(isolate,"start_time"))->NumberValue();
    int end_time = data->Get(String::NewFromUtf8(isolate,"end_time"))->NumberValue();
    int dst_width = 1280;
    int dst_height = 720;
    int n_split = 2;
    double start_theta = 0. * M_PI;
    double end_theta = 2. * M_PI;
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

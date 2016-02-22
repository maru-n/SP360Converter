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
    convertMovie(work->src_file,
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


void convertAfter(uv_work_t *req, int status)
{
    Work *work = static_cast<Work *>(req->data);
    work->status = "successed";
    work->progress = 1.0;
    callNodeCallback(work);
    work->callback.Reset();
    uv_close((uv_handle_t*) &work->progress_async, NULL);
    //delete work;
}


void convertMovieMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Function> callback = Local<Function>::Cast(args[0]);
    work->callback.Reset(isolate, callback);

    uv_async_init(uv_default_loop(), &work->progress_async, convertProgress);
    uv_queue_work(uv_default_loop(), &work->request, convertAsync, convertAfter);

    args.GetReturnValue().Set(Undefined(isolate));
}


void makeThumbnailMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    if (!args[1]->IsNumber() || !args[2]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Wrong arguments")));
        return;
    }
    Local<Uint8ClampedArray> array = args[0].As<Uint8ClampedArray>();
    unsigned char* ptr = (unsigned char*)array->Buffer()->GetContents().Data();
    int width = args[1]->NumberValue();
    int height = args[2]->NumberValue();
    makeThumbnail(work->src_file, ptr, width, height, 0);
    args.GetReturnValue().Set(Undefined(isolate));
}


void setupMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
    double angle_start   = data->Get(String::NewFromUtf8(isolate,"angle_start"))->NumberValue();
    double angle_end     = data->Get(String::NewFromUtf8(isolate,"angle_end"))->NumberValue();
    double radius_in     = data->Get(String::NewFromUtf8(isolate,"radius_in"))->NumberValue();
    double radius_out    = data->Get(String::NewFromUtf8(isolate,"radius_out"))->NumberValue();
    int n_split          = data->Get(String::NewFromUtf8(isolate,"n_split"))->NumberValue();
    work->src_file    = src_file;
    work->dst_file    = dst_file;
    work->start_time  = start_time;
    work->end_time    = end_time;
    work->dst_width   = dst_width;
    work->dst_height  = dst_height;
    work->angle_start = angle_start;
    work->angle_end   = angle_end;
    work->radius_in   = radius_in;
    work->radius_out  = radius_out;
    work->n_split     = n_split;
}


void init(Local<Object> exports) {
    work = new Work();
    work->request.data = work;
    work->progress_async.data = work;
    NODE_SET_METHOD(exports, "setup", setupMethod);
    NODE_SET_METHOD(exports, "convert", convertMovieMethod);
    NODE_SET_METHOD(exports, "makeThumbnail", makeThumbnailMethod);
}

NODE_MODULE(converter, init)

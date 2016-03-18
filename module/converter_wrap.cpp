#include "converter_wrap.h"
#include "converter.h"

using namespace v8;
using namespace SP360;


//Legacy
void callNodeCallbackFunc(ConverterWrap* converterWrap) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate); // Required for Node 4.x
    Local<Primitive> error;
    if (converterWrap->convertError == "") {
        error = v8::Null(isolate);
    } else {
        error = v8::String::NewFromUtf8(isolate, converterWrap->convertError.c_str());
    }
    Local<Value> status = v8::String::NewFromUtf8(isolate, converterWrap->convertStatus.c_str());
    Local<Number> progress = v8::Number::New(isolate, converterWrap->convertProgress);
    Handle<Value> argv[] = { error, status, progress };
    Local<Function>::New(isolate, converterWrap->convertCallback)->Call(isolate->GetCurrentContext()->Global(), 3, argv);
}

void convertProgressFunc(uv_async_t *handle) {
    ConverterWrap *converterWrap = static_cast<ConverterWrap *>(handle->data);
    callNodeCallbackFunc(converterWrap);
}

void convertAsyncFunc(uv_work_t *req) {
    ConverterWrap *converterWrap = static_cast<ConverterWrap *>(req->data);
    converterWrap->convertStatus = "progress";
    std::function<void(float)> progress_callback = [&converterWrap](float progress){
        converterWrap->convertProgress = progress;
        uv_async_send(&converterWrap->convertAsync);
    };
    converterWrap->converter->convert(converterWrap->convertDstFile, progress_callback);
}

void convertAfterFunc(uv_work_t *req, int status) {
    ConverterWrap *converterWrap = static_cast<ConverterWrap *>(req->data);
    converterWrap->convertStatus = "successed";
    converterWrap->convertProgress = 1.0;
    callNodeCallbackFunc(converterWrap);
    converterWrap->convertCallback.Reset();
    uv_close((uv_handle_t*) &converterWrap->convertAsync, NULL);
}
// end Legacy


Persistent<Function> ConverterWrap::constructor;

ConverterWrap::ConverterWrap() {
    this->converter = new Converter();
    this->convertRequest.data = this;
    this->convertAsync.data = this;
}

ConverterWrap::~ConverterWrap() {}

void ConverterWrap::Init(Local<Object> module) {
    Isolate* isolate = module->GetIsolate();
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Converter"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
    NODE_SET_PROTOTYPE_METHOD(tpl, "setup", Setup);
    NODE_SET_PROTOTYPE_METHOD(tpl, "makeOriginalPreviewImage", MakeOriginalPreviewImage);
    NODE_SET_PROTOTYPE_METHOD(tpl, "makeConvertedPreviewImage", MakeConvertedPreviewImage);
    NODE_SET_PROTOTYPE_METHOD(tpl, "convert", Convert);
    NODE_SET_PROTOTYPE_METHOD(tpl, "totalFrame", TotalFrame);
    NODE_SET_PROTOTYPE_METHOD(tpl, "isOpened", IsOpened);
    NODE_SET_PROTOTYPE_METHOD(tpl, "fps", FPS);

    constructor.Reset(isolate, tpl->GetFunction());
    module->Set(String::NewFromUtf8(isolate, "exports"), tpl->GetFunction());
}

void ConverterWrap::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args.IsConstructCall()) {
        ConverterWrap* obj = new ConverterWrap();
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    } else {
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

void ConverterWrap::Open(const FunctionCallbackInfo<Value>& args) {
    Converter* converter = ObjectWrap::Unwrap<ConverterWrap>(args.Holder())->converter;
    v8::String::Utf8Value src_file_utf(args[0]->ToString());
    std::string src_file = std::string(*src_file_utf);
    converter->open(src_file);
    args.GetReturnValue().Set(args.This());
}

void ConverterWrap::Setup(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Converter* converter = ObjectWrap::Unwrap<ConverterWrap>(args.Holder())->converter;

    Handle<Object> data = Handle<Object>::Cast(args[0]);

    if (data->Get(String::NewFromUtf8(isolate,"start_frame"))->IsNumber())
        converter->startFrame(data->Get(String::NewFromUtf8(isolate,"start_frame"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"end_frame"))->IsNumber())
        converter->endFrame(data->Get(String::NewFromUtf8(isolate,"end_frame"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"dst_width"))->IsNumber())
        converter->dst_width = data->Get(String::NewFromUtf8(isolate,"dst_width"))->NumberValue();

    if (data->Get(String::NewFromUtf8(isolate,"dst_height"))->IsNumber())
        converter->dst_height = data->Get(String::NewFromUtf8(isolate,"dst_height"))->NumberValue();

    if (data->Get(String::NewFromUtf8(isolate,"angle_start"))->IsNumber())
        converter->angleStart(data->Get(String::NewFromUtf8(isolate,"angle_start"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"angle_end"))->IsNumber())
        converter->angleEnd(data->Get(String::NewFromUtf8(isolate,"angle_end"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"radius_start"))->IsNumber())
        converter->radiusStart(data->Get(String::NewFromUtf8(isolate,"radius_start"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"radius_end"))->IsNumber())
        converter->radiusEnd(data->Get(String::NewFromUtf8(isolate,"radius_end"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"split_x"))->IsNumber())
        converter->splitX(data->Get(String::NewFromUtf8(isolate,"split_x"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"split_y"))->IsNumber())
        converter->splitY(data->Get(String::NewFromUtf8(isolate,"split_y"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"aspect"))->IsNumber())
        converter->aspect(data->Get(String::NewFromUtf8(isolate,"aspect"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"fov"))->IsNumber())
        converter->fov(data->Get(String::NewFromUtf8(isolate,"fov"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"center_angle"))->IsNumber())
        converter->centerAngle(data->Get(String::NewFromUtf8(isolate,"center_angle"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"center_radius"))->IsNumber())
        converter->centerRadius(data->Get(String::NewFromUtf8(isolate,"center_radius"))->NumberValue());

    if (data->Get(String::NewFromUtf8(isolate,"projection_type"))->IsString()) {
        v8::String::Utf8Value type_utf(data->Get(String::NewFromUtf8(isolate,"projection_type"))->ToString());
        std::string type = std::string(*type_utf);
        if (type == "central") {
            converter->projectionType(CENTRAL_PROJECTION);
        } else if (type == "equirectangular") {
            converter->projectionType(EQUIRECTANGULAR_PROJECTION);
        }
    }

    args.GetReturnValue().Set(args.This());
}

void ConverterWrap::MakeOriginalPreviewImage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Converter* converter = ObjectWrap::Unwrap<ConverterWrap>(args.Holder())->converter;
    if (!args[1]->IsNumber() || !args[2]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Wrong arguments")));
        return;
    }
    Local<Uint8ClampedArray> array = args[0].As<Uint8ClampedArray>();
    unsigned char* dst_img_ptr = (unsigned char*)array->Buffer()->GetContents().Data();
    int width = args[1]->NumberValue();
    int height = args[2]->NumberValue();
    bool border = args[3]->BooleanValue();
    converter->makeOriginalPreviewImage(dst_img_ptr, width, height, border);
    args.GetReturnValue().Set(Undefined(isolate));
}

void ConverterWrap::MakeConvertedPreviewImage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Converter* converter = ObjectWrap::Unwrap<ConverterWrap>(args.Holder())->converter;
    if (!args[1]->IsNumber() || !args[2]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Wrong arguments")));
        return;
    }
    Local<Uint8ClampedArray> array = args[0].As<Uint8ClampedArray>();
    unsigned char* dst_img_ptr = (unsigned char*)array->Buffer()->GetContents().Data();
    int width = args[1]->NumberValue();
    int height = args[2]->NumberValue();
    converter->makeConvertedPreviewImage(dst_img_ptr, width, height);
    args.GetReturnValue().Set(Undefined(isolate));
}

void ConverterWrap::Convert(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    ConverterWrap* obj = ObjectWrap::Unwrap<ConverterWrap>(args.Holder());
    // filename
    v8::String::Utf8Value dst_file_utf(args[0]);
    obj->convertDstFile = std::string(*dst_file_utf);
    // callback function
    Local<Function> callback = Local<Function>::Cast(args[1]);
    obj->convertCallback.Reset(isolate, callback);
    // invoke async function and callback
    uv_async_init(uv_default_loop(), &obj->convertAsync, convertProgressFunc);
    uv_queue_work(uv_default_loop(), &obj->convertRequest, convertAsyncFunc, convertAfterFunc);

    args.GetReturnValue().Set(Undefined(isolate));
}

void ConverterWrap::TotalFrame(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Converter* converter = ObjectWrap::Unwrap<ConverterWrap>(args.Holder())->converter;
    double totalFrame = converter->totalFrame();
    args.GetReturnValue().Set(Number::New(isolate, totalFrame));
}

void ConverterWrap::IsOpened(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Converter* converter = ObjectWrap::Unwrap<ConverterWrap>(args.Holder())->converter;
    bool isOpened = converter->isOpened();
    args.GetReturnValue().Set(Boolean::New(isolate, isOpened));
}

void ConverterWrap::FPS(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Converter* converter = ObjectWrap::Unwrap<ConverterWrap>(args.Holder())->converter;
    double fps = converter->fps();
    args.GetReturnValue().Set(Number::New(isolate, fps));
}

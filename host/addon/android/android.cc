#include <nan.h>
#include <uv.h>
#include <vector>
#include <map>

#include "android_usb.h"

NAN_METHOD(Start) {
  static AndroidUsb* usb = AndroidUsb::GetInstance();

  Nan::Callback* on_update_state = new Nan::Callback(info[0].As<v8::Function>());
  Nan::Callback* on_submit = new Nan::Callback(info[1].As<v8::Function>());
  usb->Start(on_update_state, on_submit);
}

NAN_METHOD(Discover) {
  static AndroidUsb* usb = AndroidUsb::GetInstance();

  usb->RequestDiscover();
}

NAN_METHOD(GetUsbDeviceState) {
  static AndroidUsb* usb = AndroidUsb::GetInstance();

  Nan::Callback* callback = new Nan::Callback(info[0].As<v8::Function>());
  usb->GetDeviceState(callback);
}

NAN_MODULE_INIT(InitAll) {
	int ret;

  if ((ret = AndroidUsb::GetInstance()->Init()) != 0) {
    printf("usb init failed\n");
    return;
  }

  Nan::Set(target, Nan::New<v8::String>("start").ToLocalChecked(),
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Start)).ToLocalChecked());

  Nan::Set(target, Nan::New<v8::String>("discover").ToLocalChecked(),
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Discover)).ToLocalChecked());

  Nan::Set(target, Nan::New<v8::String>("getUsbDeviceState").ToLocalChecked(),
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GetUsbDeviceState)).ToLocalChecked());
}

NODE_MODULE(addon, InitAll)

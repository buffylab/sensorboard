#include <nan.h>
#include <uv.h>
#include <map>
#include "log.h"
#include "android_usb.h"

#define MSG_BUFFER_SIZE                 256
#define ANDROID_USB_INTERFACE_NUM       0       // Use the first one

// From ADK1 AndroidAccessory.cpp

#define USB_ACCESSORY_VENDOR_ID         0x18D1
#define USB_ACCESSORY_PRODUCT_ID        0x2D00
#define USB_ACCESSORY_ADB_PRODUCT_ID    0x2D01

#define ACCESSORY_STRING_MANUFACTURER   0
#define ACCESSORY_STRING_MODEL          1
#define ACCESSORY_STRING_DESCRIPTION    2
#define ACCESSORY_STRING_VERSION        3
#define ACCESSORY_STRING_URI            4
#define ACCESSORY_STRING_SERIAL         5

#define ACCESSORY_GET_PROTOCOL          51
#define ACCESSORY_SEND_STRING           52
#define ACCESSORY_START                 53

// From ADK1 ch9.h

/* Setup Data Constants */

#define USB_SETUP_HOST_TO_DEVICE        0x00    // Device Request bmRequestType transfer direction - host to device transfer
#define USB_SETUP_DEVICE_TO_HOST        0x80    // Device Request bmRequestType transfer direction - device to host transfer
#define USB_SETUP_TYPE_STANDARD         0x00    // Device Request bmRequestType type - standard
#define USB_SETUP_TYPE_CLASS            0x20    // Device Request bmRequestType type - class
#define USB_SETUP_TYPE_VENDOR           0x40    // Device Request bmRequestType type - vendor
#define USB_SETUP_RECIPIENT_DEVICE      0x00    // Device Request bmRequestType recipient - device
#define USB_SETUP_RECIPIENT_INTERFACE   0x01    // Device Request bmRequestType recipient - interface
#define USB_SETUP_RECIPIENT_ENDPOINT    0x02    // Device Request bmRequestType recipient - endpoint
#define USB_SETUP_RECIPIENT_OTHER       0x03    // Device Request bmRequestType recipient - other

const v8::PropertyAttribute CONST_PROP =
    static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete);

#define V8STR(str) Nan::New<v8::String>(str).ToLocalChecked()
#define STRUCT_TO_V8(TARGET, STR, NAME) \
    TARGET->ForceSet(V8STR(#NAME), Nan::New<v8::Uint32>((uint32_t) (STR).NAME), CONST_PROP);

/**
 * UsbDevice
 */
UsbDevice::UsbDevice(
  uint8_t bus,
  uint8_t address,
  Nan::Callback *on_submit) : bus_(bus)
                            , address_(address)
                            , on_submit_(on_submit)
                            , state_(UsbDeviceState::INIT)
                            , error_("")
                            , handle_(nullptr)
                            , async_(nullptr) {
  uv_mutex_init(&async_lock_);
}

UsbDevice::~UsbDevice() {
  uv_mutex_destroy(&async_lock_);
}

void UsbDevice::Init(libusb_device *dev, const libusb_device_descriptor& desc) {
  char buf[MSG_BUFFER_SIZE];
  int ret;

  libusb_device_handle *handle = nullptr;
	if ((ret = libusb_open(dev, &handle)) != 0) {
    state_ = UsbDeviceState::STOPPED;
    snprintf(buf, MSG_BUFFER_SIZE, "libusb_open failed: %s", libusb_error_name(ret));
    error_ = buf;
		return;
	}

  int current_config = 0;
  if((ret = libusb_get_configuration(handle, &current_config)) != 0) {
    libusb_close(handle);

    state_ = UsbDeviceState::STOPPED;
    snprintf(buf, MSG_BUFFER_SIZE, "libusb_get_configuration failed: %s", libusb_error_name(ret));
    error_ = buf;
    return;
  }

  // TODO: Handle ADB
  if (desc.idProduct == USB_ACCESSORY_ADB_PRODUCT_ID) {
  }

  libusb_config_descriptor* cdesc;
  if ((ret = libusb_get_active_config_descriptor(dev, &cdesc)) != 0) {
    libusb_close(handle);

    state_ = UsbDeviceState::STOPPED;
		snprintf(buf, MSG_BUFFER_SIZE, "libusb_get_active_config_descriptor failed: %s", libusb_error_name(ret));
    error_ = buf;
		return;
	}

  if (cdesc->bNumInterfaces < 1) {
    libusb_close(handle);
		libusb_free_config_descriptor(cdesc);

    state_ = UsbDeviceState::STOPPED;
		snprintf(buf, MSG_BUFFER_SIZE, "Invalid cdesc->bNumInterfaces: %d", cdesc->bNumInterfaces);
    error_ = buf;
		return;
	}

  // Use the first interface
  const libusb_interface& interface = cdesc->interface[0];

  // TODO: Error handling
  if (interface.num_altsetting < 1) {
    libusb_close(handle);
		libusb_free_config_descriptor(cdesc);

    state_ = UsbDeviceState::STOPPED;
    snprintf(buf, MSG_BUFFER_SIZE, "Invalid interface.num_altsetting: %d", interface.num_altsetting);
    error_ = buf;
		return;
	}

  // Use the first interface altsetting
  const libusb_interface_descriptor& idesc = interface.altsetting[0];

  uint8_t endpoint_in;
  uint8_t endpoint_out;
  bool endpoint_in_found = false;
  bool endpoint_out_found = false;
  for (uint8_t i = 0; i < idesc.bNumEndpoints; ++i) {
    const libusb_endpoint_descriptor& edesc = idesc.endpoint[i];
    if ((edesc.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
      endpoint_in = edesc.bEndpointAddress;
      endpoint_in_found = true;
    } else if ((edesc.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
      endpoint_out = edesc.bEndpointAddress;
      endpoint_out_found = true;
    }
  }

  if (!endpoint_in || !endpoint_out) {
    libusb_close(handle);
    libusb_free_config_descriptor(cdesc);

    state_ = UsbDeviceState::STOPPED;
    snprintf(buf, MSG_BUFFER_SIZE, "Cannot find endpoint: (in: %s, out: %s)",
      endpoint_in_found ? "true" : "false",
      endpoint_out_found ? "true" : "false");
    error_ = buf;
		return;
  }

  endpoint_in_ = endpoint_in;
  endpoint_out_ = endpoint_out;
  handle_ = handle;

  libusb_free_config_descriptor(cdesc);
}

void UsbDevice::Start() {
  char msg[MSG_BUFFER_SIZE];

  int ret;

  if ((ret = libusb_claim_interface(handle_, ANDROID_USB_INTERFACE_NUM)) != 0) {
		libusb_close(handle_);
    handle_ = nullptr;

    state_ = UsbDeviceState::STOPPED;
    snprintf(msg, MSG_BUFFER_SIZE, "libusb_claim_interface failed: %s", libusb_error_name(ret));
    error_ = msg;
		return;
	}

  async_ = new uv_async_t;
  if ((ret = uv_async_init(uv_default_loop(), async_, AsyncCallback)) != 0) {
    async_ = nullptr;

    state_ = UsbDeviceState::STOPPED;
    snprintf(msg, MSG_BUFFER_SIZE, "uv_async_init failed: %d", ret);
    error_ = msg;
		return;
  }
  async_->data = this;

  for (int i = 0; i < RX_LOOPS_NUM; ++i) {
  	void *buf = malloc(USB_MRU);
  	struct libusb_transfer *transfer = libusb_alloc_transfer(0);
  	libusb_fill_bulk_transfer(transfer, handle_, endpoint_in_,
      reinterpret_cast<unsigned char *>(buf), USB_MRU, RxCallback, this, 0);
  	if((ret = libusb_submit_transfer(transfer)) != 0) {
  		libusb_free_transfer(transfer);

      state_ = UsbDeviceState::STOPPED;
      snprintf(msg, MSG_BUFFER_SIZE, "libusb_submit_transfer failed: %s", libusb_error_name(ret));
      error_ = msg;
  		return;
  	}

    rx_transfers_.insert(transfer);
  }

  state_ = UsbDeviceState::RUNNING;
}

void UsbDevice::RxCallback(struct libusb_transfer *transfer) {
  UsbDevice *device = reinterpret_cast<UsbDevice *>(transfer->user_data);
  if (false == device->HandleRx(transfer)) {
    free(transfer->buffer);
    device->rx_transfers_.erase(transfer);
    libusb_free_transfer(transfer);
  }
}

bool UsbDevice::HandleRx(struct libusb_transfer *transfer) {
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    switch(transfer->status) {
      state_ = UsbDeviceState::STOPPED;
      case LIBUSB_TRANSFER_ERROR:
        error_ = "LIBUSB_TRANSFER_ERROR";
        break;
      case LIBUSB_TRANSFER_TIMED_OUT:
        error_ = "LIBUSB_TRANSFER_TIMED_OUT";
        break;
      case LIBUSB_TRANSFER_CANCELLED:
        error_ = "LIBUSB_TRANSFER_CANCELLED";
        break;
      case LIBUSB_TRANSFER_STALL:
        error_ = "LIBUSB_TRANSFER_STALL";
        break;
      case LIBUSB_TRANSFER_NO_DEVICE:
        error_ = "LIBUSB_TRANSFER_NO_DEVICE";
        break;
      case LIBUSB_TRANSFER_OVERFLOW:
        error_ = "LIBUSB_TRANSFER_NO_OVERFLOW";
        break;
      default:
        break;
    }

    return false;
  }

  bool overflow = false;

  uv_mutex_lock(&async_lock_);
	if (buffer_index_ + transfer->actual_length > DEVICE_BUFFER_SIZE) {
		overflow = true;
	} else {
		memcpy(reinterpret_cast<void *>(buffer_ + buffer_index_), transfer->buffer, transfer->actual_length);
		buffer_index_ += transfer->actual_length;
	}
  uv_mutex_unlock(&async_lock_);

  if (overflow) {
    state_ = UsbDeviceState::STOPPED;
    error_ = "Buffer overflow :(";
    return false;
  }

  libusb_submit_transfer(transfer);
  uv_async_send(async_);
  return true;
}

void UsbDevice::Destroy() {
  uv_close(reinterpret_cast<uv_handle_t*>(async_), AsyncClose);
}

void UsbDevice::AsyncClose(uv_handle_t *handle) {
  UsbDevice *device = static_cast<UsbDevice *>(handle->data);
  delete reinterpret_cast<uv_async_t*>(handle);
  delete device;
}

void UsbDevice::AsyncCallback(uv_async_t *async) {
  UsbDevice *device = static_cast<UsbDevice *>(async->data);
  device->HandleAsync();
}

void UsbDevice::HandleAsync() {
	int size;
  uv_mutex_lock(&async_lock_);
  memcpy(temp_buffer_, buffer_, buffer_index_);
	size = buffer_index_;
	buffer_index_ = 0;
  uv_mutex_unlock(&async_lock_);

  Submit(temp_buffer_, size);
}

void UsbDevice::Submit(const char* data, size_t size) {
  Nan::HandleScope scope;

  static char buf[128];

  snprintf(buf, 128, "%d_%d", bus_, address_);

  v8::Local<v8::Value> argv[] = {
    Nan::New(buf).ToLocalChecked(),
    Nan::New(data, size).ToLocalChecked()
  };

  on_submit_->Call(2, argv);
}

/**
 * AndroidUsb
 */
AndroidUsb::~AndroidUsb() {
  uv_mutex_destroy(&state_lock_);
}

bool AndroidUsb::IsInAccessoryMode(uint16_t vendor_id, uint16_t product_id) {
  return vendor_id == USB_ACCESSORY_VENDOR_ID &&
        (product_id == USB_ACCESSORY_PRODUCT_ID ||
         product_id == USB_ACCESSORY_ADB_PRODUCT_ID);
}

void AndroidUsb::SendString(libusb_device_handle *handle, int index, const char *str) {
  libusb_control_transfer(
     handle,
     USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_VENDOR | USB_SETUP_RECIPIENT_DEVICE,
     ACCESSORY_SEND_STRING, 0,
     index,
     (unsigned char *)str, strlen(str) + 1,
     1000
 );
}

bool AndroidUsb::RequestAccessoryMode(const libusb_device_descriptor& dd, libusb_device_handle *handle) {
  uint16_t protocol = 0;
  libusb_control_transfer(
      handle,
      USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_VENDOR | USB_SETUP_RECIPIENT_DEVICE,
      ACCESSORY_GET_PROTOCOL, 0,
      0,
      (unsigned char *)&protocol, 2,
      1000
  );

  if (protocol < 1) {
    // Skip if device is not android-powered
    return false;
  }

  SendString(handle, ACCESSORY_STRING_MANUFACTURER, "manufacturer");
  SendString(handle, ACCESSORY_STRING_MODEL, "model");
  SendString(handle, ACCESSORY_STRING_DESCRIPTION, "description");
  SendString(handle, ACCESSORY_STRING_VERSION, "version");
  SendString(handle, ACCESSORY_STRING_URI, "uri");
  SendString(handle, ACCESSORY_STRING_SERIAL, "serial");

  libusb_control_transfer(
    handle,
    USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_VENDOR | USB_SETUP_RECIPIENT_DEVICE,
    ACCESSORY_START, 0,
    0,
    0, 0,
    1000
  );

  return true;
}

void AndroidUsb::EventProcessThread(void *arg) {
  AndroidUsb *usb = reinterpret_cast<AndroidUsb *>(arg);
  while(1) {
    libusb_handle_events_completed(usb->context_, nullptr);
  }
}

/**
 * Discover
 */
void AndroidUsb::DiscoverThread(void* arg) {
  AndroidUsb *usb = reinterpret_cast<AndroidUsb *>(arg);
  uv_run(&usb->discover_loop_, UV_RUN_DEFAULT);
  uv_loop_close(&usb->discover_loop_);
}

void AndroidUsb::RequestDiscover() {
  uv_async_send(&discover_async_);
}

void AndroidUsb::OnDiscoverRequest(uv_async_t *async) {
  AndroidUsb *usb = reinterpret_cast<AndroidUsb *>(async->data);
  usb->Discover();
  usb->RequestUpdateState();
}

void AndroidUsb::Discover() {
  libusb_device **list;
  ssize_t cnt = libusb_get_device_list(context_, &list);

  if (cnt < 0) {
    log::Error("libusb_get_device_list failed: %s", libusb_error_name(cnt));

    uv_mutex_lock(&state_lock_);
    state_discover_failed_ = true;
    state_discover_error_ = "libusb_get_device_list failed";
    uv_mutex_unlock(&state_lock_);
    return;
  }

  std::set<DeviceId> found;
  std::map<DeviceId, UsbDevice *> newDevices;

  for (ssize_t i = 0; i < cnt; i++) {
    libusb_device *dev = list[i];
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(dev, &desc);
    if (IsInAccessoryMode(desc.idVendor, desc.idProduct)) {
      uint8_t bus = libusb_get_bus_number(dev);
      uint8_t address = libusb_get_device_address(dev);
      DeviceId id = std::make_pair(bus, address);
      found.insert(id);

      if (state_devices_.find(id) == state_devices_.end()) {
        UsbDevice *device = new UsbDevice(bus, address, on_submit_);
        device->Init(dev, desc);
        newDevices.insert(std::make_pair(id, device));
      }
    } else {
      libusb_device_handle* handle;
      libusb_open(dev, &handle);
      RequestAccessoryMode(desc, handle);
      libusb_close(handle);
		}
  }
  libusb_free_device_list(list, 1);

  // Mutate state
  uv_mutex_lock(&state_lock_);
  state_discover_failed_ = false;
  state_discover_error_ = "";

  auto it = state_devices_.begin();
  while (it != state_devices_.end()) {
    if (found.find(it->first) == found.end()) {
      it->second->Destroy();
      state_devices_.erase(it++);
    } else {
      it++;
    }
  }

  for (auto& it : newDevices)
    state_devices_.insert(it);
  uv_mutex_unlock(&state_lock_);
}

/**
 * Update callback
 */
void AndroidUsb::RequestUpdateState() {
  uv_async_send(&update_state_async_);
}

void AndroidUsb::OnUpdateStateRequest(uv_async_t *async) {
  AndroidUsb *usb = reinterpret_cast<AndroidUsb *>(async->data);
  usb->UpdateState();
}

void AndroidUsb::GetDeviceState(Nan::Callback *callback) {
  bool available = true;
  std::string error;
  std::vector<UsbDeviceInfo> device_infos;

  uv_mutex_lock(&state_lock_);
  if (state_discover_failed_) {
    available = false;
    error = state_discover_error_;
  } else {
    for (auto& it : state_devices_) {
      UsbDevice *device = it.second;

      if (device->state_ == UsbDeviceState::INIT) {
        device->Start();
      }

      UsbDeviceInfo device_info;
      device_info.bus = device->bus_;
      device_info.address = device->address_;
      device_info.state = device->state_;
      device_info.error = device->error_;

      device_infos.push_back(device_info);
    }
  }
  uv_mutex_unlock(&state_lock_);

  if (available) {
    HandleStateOkCallback(device_infos, callback);
  } else {
    HandleStateErrorCallback(error, callback);
  }
}

void AndroidUsb::UpdateState() {
  GetDeviceState(on_update_state_);
}

void AndroidUsb::HandleStateErrorCallback(const std::string& error, Nan::Callback *callback) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> argv[] = {
    v8::Exception::Error(Nan::New<v8::String>(error).ToLocalChecked())
  };
  callback->Call(1, argv);
}

void AndroidUsb::HandleStateOkCallback(const std::vector<UsbDeviceInfo>& device_infos, Nan::Callback *callback) {
  Nan::HandleScope scope;
  static char buf[128];

  v8::Local<v8::Object> v8_devices = Nan::New<v8::Object>();

  for (auto& info : device_infos) {
  	v8::Local<v8::Object> v8_device = Nan::New<v8::Object>();
    snprintf(buf, 128, "%d_%d", info.bus, info.address);
  	v8_devices->ForceSet(V8STR(buf), v8_device);

  	STRUCT_TO_V8(v8_device, info, bus);
  	STRUCT_TO_V8(v8_device, info, address);
  	STRUCT_TO_V8(v8_device, info, state);
  }

  v8::Local<v8::Value> argv[] = {
    Nan::Null(),
    v8_devices
  };
  callback->Call(2, argv);
}

/**
 * Usb init
 */
int AndroidUsb::Init() {
  int ret;

  if ((ret = uv_mutex_init(&state_lock_)) != 0) {
    log::Fatal("uv_mutex_init failed: %d", ret);
    return -1;
  }

  if ((ret = libusb_init(&context_)) != 0) {
		log::Fatal("libusb_init failed: %s", libusb_error_name(ret));
		return -1;
	}

  /*
   * Init discover loop
   */
  if ((ret = uv_loop_init(&discover_loop_)) != 0) {
		log::Fatal("uv_loop_init failed: %d", ret);
    return -1;
  }

  if ((ret = uv_async_init(&discover_loop_, &discover_async_, OnDiscoverRequest)) != 0) {
		log::Fatal("uv_async_init failed: %d", ret);
    return -1;
  }
  discover_async_.data = this;

  if ((ret = uv_thread_create(&discover_thread_, DiscoverThread, this)) != 0) {
    log::Fatal("uv_thread_create failed: %d", ret);
    return -1;
  }

  /*
   * Init state update async
   */
  if ((ret = uv_async_init(uv_default_loop(), &update_state_async_, OnUpdateStateRequest)) != 0) {
    log::Fatal("uv_async_init failed: %d", ret);
    return -1;
  }
  update_state_async_.data = this;

  return 0;
}

int LIBUSB_CALL AndroidUsb::HotplugCallback(
  libusb_context *ctx, libusb_device *dev,
  libusb_hotplug_event event, void *user_data
) {
  AndroidUsb *usb = reinterpret_cast<AndroidUsb *>(user_data);
  usb->RequestDiscover();
  return 0;
}

void AndroidUsb::Start(Nan::Callback *on_update_state, Nan::Callback *on_submit) {
  on_update_state_ = on_update_state;
  on_submit_ = on_submit;

  int ret;

	uv_thread_create(&event_process_thread_, EventProcessThread, this);

  if ((ret = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) != 0) {
    if ((ret = libusb_hotplug_register_callback(
      context_,
			(libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
			(libusb_hotplug_flag)0, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
			HotplugCallback, this, nullptr
    )) != 0) {
      log::Error("libusb_hotplug_register_callback failed: %s", libusb_error_name(ret));
    }
  } else {
#ifdef WIN32
    // TODO: Windows support
#else
    // Start loop
#endif
  }

  RequestDiscover();

  log::Debug("AndroidUsb::Start success");
}

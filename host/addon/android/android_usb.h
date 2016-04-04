#ifndef HOST_ANDROID_ANDROID_USB_H_
#define HOST_ANDROID_ANDROID_USB_H_

#define USB_MRU                    16384
#define RX_LOOPS_NUM               3
#define DEVICE_BUFFER_SIZE         (2 * RX_LOOPS_NUM * USB_MRU)

#include <set>
#include <libusb.h>

enum UsbDeviceState {
  INIT,
  RUNNING,
  STOPPED
};

class UsbDevice {
  friend class AndroidUsb;

private:
  UsbDevice(uint8_t bus, uint8_t address, Nan::Callback *on_submit);
  ~UsbDevice();

  void Init(libusb_device *dev, const libusb_device_descriptor& desc);
  void Start();

  static void RxCallback(struct libusb_transfer *transfer);
  bool HandleRx(struct libusb_transfer *transfer);
  void Destroy();
  static void AsyncClose(uv_handle_t *async);
  static void AsyncCallback(uv_async_t *async);
  void HandleAsync();
  void Submit(const char* data, size_t size);

  uint8_t bus_;
  uint8_t address_;

  Nan::Callback *on_submit_;

  UsbDeviceState state_;
  std::string error_;

  libusb_device_handle *handle_;
  uint8_t endpoint_in_;
  uint8_t endpoint_out_;

  char buffer_[DEVICE_BUFFER_SIZE];
  char temp_buffer_[DEVICE_BUFFER_SIZE];
  int buffer_index_;

  std::set<libusb_transfer *> tx_transfers_;
  std::set<libusb_transfer *> rx_transfers_;

  uv_async_t *async_;
  uv_mutex_t async_lock_;
};

class AndroidUsb {
public:
  static AndroidUsb* GetInstance() {
    static AndroidUsb ins;
    return &ins;
  }

  int Init();
  void Start(Nan::Callback *on_update_state, Nan::Callback *on_submit);
  void RequestDiscover();
  void GetDeviceState(Nan::Callback *callback);

private:
  struct DeviceData {
    libusb_device *device;
    libusb_device_descriptor descriptor;
  };

  struct UsbDeviceInfo {
    uint8_t bus;
    uint8_t address;
    UsbDeviceState state;
    std::string error;
  };

  AndroidUsb() {}
  AndroidUsb(const AndroidUsb& other) {}
  ~AndroidUsb();

  static bool IsInAccessoryMode(uint16_t vendor_id, uint16_t product_id);
  static bool RequestAccessoryMode(const libusb_device_descriptor& dd, libusb_device_handle *handle);
  static void SendString(libusb_device_handle *handle, int index, const char *str);

  static void EventProcessThread(void *arg);

  static void DiscoverThread(void* arg);
  static void OnDiscoverRequest(uv_async_t *async);
  void Discover();

  void RequestUpdateState();
  static void OnUpdateStateRequest(uv_async_t *async);
  void UpdateState();
  static void HandleStateErrorCallback(const std::string& error, Nan::Callback *callback);
  static void HandleStateOkCallback(const std::vector<UsbDeviceInfo>& device_infos, Nan::Callback *callback);

  static int LIBUSB_CALL HotplugCallback(
    libusb_context *ctx, libusb_device *dev,
    libusb_hotplug_event event, void *user_data
  );

  libusb_context* context_;

  // Thread dedicated to process libusb event process
  uv_thread_t event_process_thread_;

  // Usb discover loop
  uv_async_t discover_async_;
  uv_loop_t discover_loop_;
  uv_thread_t discover_thread_;

  // Module state which is shared by multiple threads
  typedef std::pair<uint8_t /* bus */, uint8_t /* address */> DeviceId;

  std::map<DeviceId, UsbDevice *> state_devices_;
  bool state_discover_failed_;
  std::string state_discover_error_;

  uv_async_t update_state_async_;
  uv_mutex_t state_lock_;

  Nan::Callback* on_update_state_;
  Nan::Callback* on_submit_;
};

#endif

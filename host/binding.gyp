{
  'targets': [
    {
      'target_name': 'android',
      'sources': [
        'addon/log.cc',
        'addon/android/android_usb.cc',
        'addon/android/android.cc',
      ],
      'include_dirs': [
        'addon/',
        '<!(node -e \'require("nan")\')',
      ],
      'dependencies': [
        'addon/externals/libusb.gypi:libusb',
      ],
    },
  ],
}

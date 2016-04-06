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
        '<!(node -e "require(\'nan\')")',
      ],
      'dependencies': [
        'addon/vendor/libusb.gypi:libusb',
      ],
      'conditions' : [
        ['OS=="mac"', {
          'xcode_settings': {
            'OTHER_CFLAGS': [ '-std=c++1y', '-stdlib=libc++' ],
            'OTHER_LDFLAGS': [ '-framework', 'CoreFoundation', '-framework', 'IOKit' ],
            'SDKROOT': 'macosx',
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
          },
        }],
        ['OS=="win"', {
          'defines':[
            'WIN32_LEAN_AND_MEAN'
          ],
          'sources': [
            'addon/android/android_usb_win.cc',
          ],
          'default_configuration': 'Debug',
          'configurations': {
            'Debug': {
              'defines': [ 'DEBUG', '_DEBUG' ],
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'RuntimeLibrary': 1, # static debug
                },
              },
            },
            'Release': {
              'defines': [ 'NDEBUG' ],
                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'RuntimeLibrary': 0, # static release
                  },
                },
              }
            },
            'msvs_settings': {
              'VCCLCompilerTool': {
                'AdditionalOptions': [ '/EHsc' ],
              },
            },
          }]
      ]
    },
  ],
}

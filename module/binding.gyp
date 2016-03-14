{
    "targets": [{
        "target_name": "converter",
        "sources": [
            "addon.cpp",
            "converter_wrap.cpp",
            "converter.cpp"
        ],
        "cflags": [
            "-Wall"
            "-std=c++11",
            "-stdlib=libc++",
            "--static"
        ],
        'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.11',
            'OTHER_CFLAGS': [
                '-std=c++11',
                '-stdlib=libc++',
                '--static'
            ]
        },
         "libraries": [
            "-F/Users/maruyama/Desktop/SP360Converter/SP360Converter/module/lib",
            "-L/Users/maruyama/Desktop/SP360Converter/SP360Converter/module/lib",
            "-framework opencv2",
            "-framework Foundation",
            "-framework QuartzCore",
            "-framework CoreVideo",
            "-framework AVFoundation",
            "-framework CoreMedia",
            "-framework CoreFoundation",
            "-framework VideoToolbox",
            "-framework VideoDecodeAcceleration",
            "-framework Security",
            "-framework OpenCL",
            "-framework CoreGraphics",
            "-framework CoreServices",
            "-lavcodec",
            #"-lavdevice",
            #"-lavfilter",
            "-lavformat",
            #"-lavresample",
            "-lavutil",
            "-lpostproc",
            #"-lswresample",
            "-lswscale",
            "-lx264",
            "-lmp3lame",
            "-liconv",
            "-lSystem",
            "-lbz2",
            "-lz",
            "-lxvidcore",
        ],
    }]
}

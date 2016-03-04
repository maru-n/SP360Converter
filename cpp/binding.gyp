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
            "-stdlib=libc++"
        ],
        'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
            'OTHER_CFLAGS': [
                '-std=c++11',
                '-stdlib=libc++'
            ]
        },
        "link_settings": {
            "libraries": [
                "-framework opencv2",
                "-framework QTKit",
                #"-framework AVKit",
            ],
        },
     }]
}

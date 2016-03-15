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
            "-F[Framework path]",
            "-L[Library path]",
            "-framework opencv2",
            "-framework VideoDecodeAcceleration",
            "-lavcodec",
            "-lavformat",
            "-lavutil",
            "-lswscale",
            "-lx264",
            "-lxvidcore",
        ],
    }]
}

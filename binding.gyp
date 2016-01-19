{
    "targets": [{
        "target_name": "convert",
        "sources": ["convert.cpp"],
        "link_settings": {
            "libraries": [
                "/usr/local/lib/libopencv_shape.dylib",
                "/usr/local/lib/libopencv_stitching.dylib",
                "/usr/local/lib/libopencv_objdetect.dylib",
                "/usr/local/lib/libopencv_superres.dylib",
                "/usr/local/lib/libopencv_videostab.dylib",
                "/usr/local/lib/libopencv_calib3d.dylib",
                "/usr/local/lib/libopencv_features2d.dylib",
                "/usr/local/lib/libopencv_highgui.dylib",
                "/usr/local/lib/libopencv_videoio.dylib",
                "/usr/local/lib/libopencv_imgcodecs.dylib",
                "/usr/local/lib/libopencv_video.dylib",
                "/usr/local/lib/libopencv_photo.dylib",
                "/usr/local/lib/libopencv_ml.dylib",
                "/usr/local/lib/libopencv_imgproc.dylib",
                "/usr/local/lib/libopencv_flann.dylib",
                "/usr/local/lib/libopencv_core.dylib",
            ],
        },
    }
    ],
}

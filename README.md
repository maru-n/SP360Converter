# SP360Converter

## Release page

https://github.com/maru-n/SP360Converter/releases


## Development

### Required

#### OpenCV3 (http://opencv.org/)

Build as Framework and put on build path. (ex. /Library/Frameworks/)
Recomended build without ffmpeg by cmake option "-DWITH_FFMPEG=OFF" for working stand alone.

### Build

Build native addon
```
$ npm run build:cpp
```

Then run in by
```
$ npm start
```

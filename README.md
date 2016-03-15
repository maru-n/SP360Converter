# SP360Converter

Requires OSX 10.7 or later.

## Release page

https://github.com/maru-n/SP360Converter/releases


## Development

### Required

##### OpenCV3 (http://opencv.org/)

Build as Framework and put on build path. (ex. /Library/Frameworks/)
Recomended build without ffmpeg by cmake option "-DWITH_FFMPEG=OFF" for working stand alone.

### Build

Build, rebuild, clean and cofigure native addon.
```
$ npm run module:[build|rebuild|clean|configure]
```

Start app.
```
$ npm start
```

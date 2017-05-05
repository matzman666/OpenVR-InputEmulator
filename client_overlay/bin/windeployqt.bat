
cd win64

windeployqt.exe --dir qtdata --libdir . --plugindir qtdata/plugins --no-system-d3d-compiler --no-opengl-sw --release  --qmldir res/qml OpenVR-InputEmulatorOverlay.exe

@REM Debug:
@REM windeployqt.exe --dir qtdata --libdir . --plugindir qtdata/plugins --no-system-d3d-compiler --compiler-runtime --no-opengl-sw --debug  --qmldir res/qml OpenVR-InputEmulatorOverlay.exe

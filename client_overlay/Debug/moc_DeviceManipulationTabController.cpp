/****************************************************************************
** Meta object code from reading C++ file 'DeviceManipulationTabController.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/tabcontrollers/DeviceManipulationTabController.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DeviceManipulationTabController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_inputemulator__DeviceManipulationTabController_t {
    QByteArrayData data[96];
    char stringdata0[2180];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_inputemulator__DeviceManipulationTabController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_inputemulator__DeviceManipulationTabController_t qt_meta_stringdata_inputemulator__DeviceManipulationTabController = {
    {
QT_MOC_LITERAL(0, 0, 46), // "inputemulator::DeviceManipula..."
QT_MOC_LITERAL(1, 47, 18), // "deviceCountChanged"
QT_MOC_LITERAL(2, 66, 0), // ""
QT_MOC_LITERAL(3, 67, 11), // "deviceCount"
QT_MOC_LITERAL(4, 79, 17), // "deviceInfoChanged"
QT_MOC_LITERAL(5, 97, 5), // "index"
QT_MOC_LITERAL(6, 103, 33), // "motionCompensationSettingsCha..."
QT_MOC_LITERAL(7, 137, 33), // "deviceManipulationProfilesCha..."
QT_MOC_LITERAL(8, 171, 35), // "motionCompensationVelAccModeC..."
QT_MOC_LITERAL(9, 207, 4), // "mode"
QT_MOC_LITERAL(10, 212, 43), // "motionCompensationKalmanProce..."
QT_MOC_LITERAL(11, 256, 8), // "variance"
QT_MOC_LITERAL(12, 265, 47), // "motionCompensationKalmanObser..."
QT_MOC_LITERAL(13, 313, 44), // "motionCompensationMovingAvera..."
QT_MOC_LITERAL(14, 358, 6), // "window"
QT_MOC_LITERAL(15, 365, 38), // "configureDigitalInputRemappin..."
QT_MOC_LITERAL(16, 404, 37), // "configureAnalogInputRemapping..."
QT_MOC_LITERAL(17, 442, 19), // "enableDeviceOffsets"
QT_MOC_LITERAL(18, 462, 6), // "enable"
QT_MOC_LITERAL(19, 469, 6), // "notify"
QT_MOC_LITERAL(20, 476, 32), // "setWorldFromDriverRotationOffset"
QT_MOC_LITERAL(21, 509, 1), // "x"
QT_MOC_LITERAL(22, 511, 1), // "y"
QT_MOC_LITERAL(23, 513, 1), // "z"
QT_MOC_LITERAL(24, 515, 35), // "setWorldFromDriverTranslation..."
QT_MOC_LITERAL(25, 551, 3), // "yaw"
QT_MOC_LITERAL(26, 555, 5), // "pitch"
QT_MOC_LITERAL(27, 561, 4), // "roll"
QT_MOC_LITERAL(28, 566, 31), // "setDriverFromHeadRotationOffset"
QT_MOC_LITERAL(29, 598, 34), // "setDriverFromHeadTranslationO..."
QT_MOC_LITERAL(30, 633, 23), // "setDriverRotationOffset"
QT_MOC_LITERAL(31, 657, 26), // "setDriverTranslationOffset"
QT_MOC_LITERAL(32, 684, 18), // "triggerHapticPulse"
QT_MOC_LITERAL(33, 703, 20), // "setDeviceRenderModel"
QT_MOC_LITERAL(34, 724, 11), // "deviceIndex"
QT_MOC_LITERAL(35, 736, 16), // "renderModelIndex"
QT_MOC_LITERAL(36, 753, 28), // "addDeviceManipulationProfile"
QT_MOC_LITERAL(37, 782, 4), // "name"
QT_MOC_LITERAL(38, 787, 21), // "includesDeviceOffsets"
QT_MOC_LITERAL(39, 809, 22), // "includesInputRemapping"
QT_MOC_LITERAL(40, 832, 30), // "applyDeviceManipulationProfile"
QT_MOC_LITERAL(41, 863, 31), // "deleteDeviceManipulationProfile"
QT_MOC_LITERAL(42, 895, 31), // "setMotionCompensationVelAccMode"
QT_MOC_LITERAL(43, 927, 39), // "setMotionCompensationKalmanPr..."
QT_MOC_LITERAL(44, 967, 43), // "setMotionCompensationKalmanOb..."
QT_MOC_LITERAL(45, 1011, 40), // "setMotionCompensationMovingAv..."
QT_MOC_LITERAL(46, 1052, 14), // "getDeviceCount"
QT_MOC_LITERAL(47, 1067, 15), // "getDeviceSerial"
QT_MOC_LITERAL(48, 1083, 11), // "getDeviceId"
QT_MOC_LITERAL(49, 1095, 14), // "getDeviceClass"
QT_MOC_LITERAL(50, 1110, 14), // "getDeviceState"
QT_MOC_LITERAL(51, 1125, 13), // "getDeviceMode"
QT_MOC_LITERAL(52, 1139, 27), // "getDeviceModeRefDeviceIndex"
QT_MOC_LITERAL(53, 1167, 20), // "deviceOffsetsEnabled"
QT_MOC_LITERAL(54, 1188, 32), // "getWorldFromDriverRotationOffset"
QT_MOC_LITERAL(55, 1221, 4), // "axis"
QT_MOC_LITERAL(56, 1226, 35), // "getWorldFromDriverTranslation..."
QT_MOC_LITERAL(57, 1262, 31), // "getDriverFromHeadRotationOffset"
QT_MOC_LITERAL(58, 1294, 34), // "getDriverFromHeadTranslationO..."
QT_MOC_LITERAL(59, 1329, 23), // "getDriverRotationOffset"
QT_MOC_LITERAL(60, 1353, 26), // "getDriverTranslationOffset"
QT_MOC_LITERAL(61, 1380, 31), // "getMotionCompensationVelAccMode"
QT_MOC_LITERAL(62, 1412, 39), // "getMotionCompensationKalmanPr..."
QT_MOC_LITERAL(63, 1452, 43), // "getMotionCompensationKalmanOb..."
QT_MOC_LITERAL(64, 1496, 40), // "getMotionCompensationMovingAv..."
QT_MOC_LITERAL(65, 1537, 33), // "getDeviceManipulationProfileC..."
QT_MOC_LITERAL(66, 1571, 32), // "getDeviceManipulationProfileName"
QT_MOC_LITERAL(67, 1604, 19), // "getRenderModelCount"
QT_MOC_LITERAL(68, 1624, 18), // "getRenderModelName"
QT_MOC_LITERAL(69, 1643, 16), // "updateDeviceInfo"
QT_MOC_LITERAL(70, 1660, 21), // "getDigitalButtonCount"
QT_MOC_LITERAL(71, 1682, 18), // "getDigitalButtonId"
QT_MOC_LITERAL(72, 1701, 11), // "buttonIndex"
QT_MOC_LITERAL(73, 1713, 20), // "getDigitalButtonName"
QT_MOC_LITERAL(74, 1734, 8), // "buttonId"
QT_MOC_LITERAL(75, 1743, 22), // "getDigitalButtonStatus"
QT_MOC_LITERAL(76, 1766, 18), // "getAnalogAxisCount"
QT_MOC_LITERAL(77, 1785, 15), // "getAnalogAxisId"
QT_MOC_LITERAL(78, 1801, 9), // "axisIndex"
QT_MOC_LITERAL(79, 1811, 17), // "getAnalogAxisName"
QT_MOC_LITERAL(80, 1829, 6), // "axisId"
QT_MOC_LITERAL(81, 1836, 19), // "getAnalogAxisStatus"
QT_MOC_LITERAL(82, 1856, 35), // "startConfigureDigitalInputRem..."
QT_MOC_LITERAL(83, 1892, 36), // "finishConfigureDigitalInputRe..."
QT_MOC_LITERAL(84, 1929, 12), // "touchAsClick"
QT_MOC_LITERAL(85, 1942, 9), // "longPress"
QT_MOC_LITERAL(86, 1952, 18), // "longPressThreshold"
QT_MOC_LITERAL(87, 1971, 25), // "longPressImmediateRelease"
QT_MOC_LITERAL(88, 1997, 11), // "doublePress"
QT_MOC_LITERAL(89, 2009, 20), // "doublePressThreshold"
QT_MOC_LITERAL(90, 2030, 27), // "doublePressImmediateRelease"
QT_MOC_LITERAL(91, 2058, 34), // "startConfigureAnalogInputRema..."
QT_MOC_LITERAL(92, 2093, 35), // "finishConfigureAnalogInputRem..."
QT_MOC_LITERAL(93, 2129, 13), // "setDeviceMode"
QT_MOC_LITERAL(94, 2143, 11), // "targedIndex"
QT_MOC_LITERAL(95, 2155, 24) // "getDeviceModeErrorString"

    },
    "inputemulator::DeviceManipulationTabController\0"
    "deviceCountChanged\0\0deviceCount\0"
    "deviceInfoChanged\0index\0"
    "motionCompensationSettingsChanged\0"
    "deviceManipulationProfilesChanged\0"
    "motionCompensationVelAccModeChanged\0"
    "mode\0motionCompensationKalmanProcessNoiseChanged\0"
    "variance\0motionCompensationKalmanObservationNoiseChanged\0"
    "motionCompensationMovingAverageWindowChanged\0"
    "window\0configureDigitalInputRemappingFinished\0"
    "configureAnalogInputRemappingFinished\0"
    "enableDeviceOffsets\0enable\0notify\0"
    "setWorldFromDriverRotationOffset\0x\0y\0"
    "z\0setWorldFromDriverTranslationOffset\0"
    "yaw\0pitch\0roll\0setDriverFromHeadRotationOffset\0"
    "setDriverFromHeadTranslationOffset\0"
    "setDriverRotationOffset\0"
    "setDriverTranslationOffset\0"
    "triggerHapticPulse\0setDeviceRenderModel\0"
    "deviceIndex\0renderModelIndex\0"
    "addDeviceManipulationProfile\0name\0"
    "includesDeviceOffsets\0includesInputRemapping\0"
    "applyDeviceManipulationProfile\0"
    "deleteDeviceManipulationProfile\0"
    "setMotionCompensationVelAccMode\0"
    "setMotionCompensationKalmanProcessNoise\0"
    "setMotionCompensationKalmanObservationNoise\0"
    "setMotionCompensationMovingAverageWindow\0"
    "getDeviceCount\0getDeviceSerial\0"
    "getDeviceId\0getDeviceClass\0getDeviceState\0"
    "getDeviceMode\0getDeviceModeRefDeviceIndex\0"
    "deviceOffsetsEnabled\0"
    "getWorldFromDriverRotationOffset\0axis\0"
    "getWorldFromDriverTranslationOffset\0"
    "getDriverFromHeadRotationOffset\0"
    "getDriverFromHeadTranslationOffset\0"
    "getDriverRotationOffset\0"
    "getDriverTranslationOffset\0"
    "getMotionCompensationVelAccMode\0"
    "getMotionCompensationKalmanProcessNoise\0"
    "getMotionCompensationKalmanObservationNoise\0"
    "getMotionCompensationMovingAverageWindow\0"
    "getDeviceManipulationProfileCount\0"
    "getDeviceManipulationProfileName\0"
    "getRenderModelCount\0getRenderModelName\0"
    "updateDeviceInfo\0getDigitalButtonCount\0"
    "getDigitalButtonId\0buttonIndex\0"
    "getDigitalButtonName\0buttonId\0"
    "getDigitalButtonStatus\0getAnalogAxisCount\0"
    "getAnalogAxisId\0axisIndex\0getAnalogAxisName\0"
    "axisId\0getAnalogAxisStatus\0"
    "startConfigureDigitalInputRemapping\0"
    "finishConfigureDigitalInputRemapping\0"
    "touchAsClick\0longPress\0longPressThreshold\0"
    "longPressImmediateRelease\0doublePress\0"
    "doublePressThreshold\0doublePressImmediateRelease\0"
    "startConfigureAnalogInputRemapping\0"
    "finishConfigureAnalogInputRemapping\0"
    "setDeviceMode\0targedIndex\0"
    "getDeviceModeErrorString"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_inputemulator__DeviceManipulationTabController[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      75,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  389,    2, 0x06 /* Public */,
       4,    1,  392,    2, 0x06 /* Public */,
       6,    0,  395,    2, 0x06 /* Public */,
       7,    0,  396,    2, 0x06 /* Public */,
       8,    1,  397,    2, 0x06 /* Public */,
      10,    1,  400,    2, 0x06 /* Public */,
      12,    1,  403,    2, 0x06 /* Public */,
      13,    1,  406,    2, 0x06 /* Public */,
      15,    0,  409,    2, 0x06 /* Public */,
      16,    0,  410,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      17,    3,  411,    2, 0x0a /* Public */,
      17,    2,  418,    2, 0x2a /* Public | MethodCloned */,
      20,    5,  423,    2, 0x0a /* Public */,
      20,    4,  434,    2, 0x2a /* Public | MethodCloned */,
      24,    5,  443,    2, 0x0a /* Public */,
      24,    4,  454,    2, 0x2a /* Public | MethodCloned */,
      28,    5,  463,    2, 0x0a /* Public */,
      28,    4,  474,    2, 0x2a /* Public | MethodCloned */,
      29,    5,  483,    2, 0x0a /* Public */,
      29,    4,  494,    2, 0x2a /* Public | MethodCloned */,
      30,    5,  503,    2, 0x0a /* Public */,
      30,    4,  514,    2, 0x2a /* Public | MethodCloned */,
      31,    5,  523,    2, 0x0a /* Public */,
      31,    4,  534,    2, 0x2a /* Public | MethodCloned */,
      32,    1,  543,    2, 0x0a /* Public */,
      33,    2,  546,    2, 0x0a /* Public */,
      36,    4,  551,    2, 0x0a /* Public */,
      40,    2,  560,    2, 0x0a /* Public */,
      41,    1,  565,    2, 0x0a /* Public */,
      42,    2,  568,    2, 0x0a /* Public */,
      42,    1,  573,    2, 0x2a /* Public | MethodCloned */,
      43,    2,  576,    2, 0x0a /* Public */,
      43,    1,  581,    2, 0x2a /* Public | MethodCloned */,
      44,    2,  584,    2, 0x0a /* Public */,
      44,    1,  589,    2, 0x2a /* Public | MethodCloned */,
      45,    2,  592,    2, 0x0a /* Public */,
      45,    1,  597,    2, 0x2a /* Public | MethodCloned */,

 // methods: name, argc, parameters, tag, flags
      46,    0,  600,    2, 0x02 /* Public */,
      47,    1,  601,    2, 0x02 /* Public */,
      48,    1,  604,    2, 0x02 /* Public */,
      49,    1,  607,    2, 0x02 /* Public */,
      50,    1,  610,    2, 0x02 /* Public */,
      51,    1,  613,    2, 0x02 /* Public */,
      52,    1,  616,    2, 0x02 /* Public */,
      53,    1,  619,    2, 0x02 /* Public */,
      54,    2,  622,    2, 0x02 /* Public */,
      56,    2,  627,    2, 0x02 /* Public */,
      57,    2,  632,    2, 0x02 /* Public */,
      58,    2,  637,    2, 0x02 /* Public */,
      59,    2,  642,    2, 0x02 /* Public */,
      60,    2,  647,    2, 0x02 /* Public */,
      61,    0,  652,    2, 0x02 /* Public */,
      62,    0,  653,    2, 0x02 /* Public */,
      63,    0,  654,    2, 0x02 /* Public */,
      64,    0,  655,    2, 0x02 /* Public */,
      65,    0,  656,    2, 0x02 /* Public */,
      66,    1,  657,    2, 0x02 /* Public */,
      67,    0,  660,    2, 0x02 /* Public */,
      68,    1,  661,    2, 0x02 /* Public */,
      69,    1,  664,    2, 0x02 /* Public */,
      70,    1,  667,    2, 0x02 /* Public */,
      71,    2,  670,    2, 0x02 /* Public */,
      73,    2,  675,    2, 0x02 /* Public */,
      75,    2,  680,    2, 0x02 /* Public */,
      76,    1,  685,    2, 0x02 /* Public */,
      77,    2,  688,    2, 0x02 /* Public */,
      79,    2,  693,    2, 0x02 /* Public */,
      81,    2,  698,    2, 0x02 /* Public */,
      82,    2,  703,    2, 0x02 /* Public */,
      83,    9,  708,    2, 0x02 /* Public */,
      91,    2,  727,    2, 0x02 /* Public */,
      92,    2,  732,    2, 0x02 /* Public */,
      93,    4,  737,    2, 0x02 /* Public */,
      93,    3,  746,    2, 0x22 /* Public | MethodCloned */,
      95,    0,  753,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::UInt,    3,
    QMetaType::Void, QMetaType::UInt,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt,    9,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::UInt,   14,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::UInt, QMetaType::Bool, QMetaType::Bool,    5,   18,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::Bool,    5,   18,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Bool,    5,   21,   22,   23,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double,    5,   21,   22,   23,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Bool,    5,   25,   26,   27,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double,    5,   25,   26,   27,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Bool,    5,   21,   22,   23,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double,    5,   21,   22,   23,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Bool,    5,   25,   26,   27,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double,    5,   25,   26,   27,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Bool,    5,   21,   22,   23,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double,    5,   21,   22,   23,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Bool,    5,   25,   26,   27,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::Double, QMetaType::Double, QMetaType::Double,    5,   25,   26,   27,
    QMetaType::Void, QMetaType::UInt,    5,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,   34,   35,
    QMetaType::Void, QMetaType::QString, QMetaType::UInt, QMetaType::Bool, QMetaType::Bool,   37,   34,   38,   39,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,    5,   34,
    QMetaType::Void, QMetaType::UInt,    5,
    QMetaType::Void, QMetaType::UInt, QMetaType::Bool,    9,   19,
    QMetaType::Void, QMetaType::UInt,    9,
    QMetaType::Void, QMetaType::Double, QMetaType::Bool,   11,   19,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::Double, QMetaType::Bool,   11,   19,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::UInt, QMetaType::Bool,   14,   19,
    QMetaType::Void, QMetaType::UInt,   14,

 // methods: parameters
    QMetaType::UInt,
    QMetaType::QString, QMetaType::UInt,    5,
    QMetaType::UInt, QMetaType::UInt,    5,
    QMetaType::Int, QMetaType::UInt,    5,
    QMetaType::Int, QMetaType::UInt,    5,
    QMetaType::Int, QMetaType::UInt,    5,
    QMetaType::Int, QMetaType::UInt,    5,
    QMetaType::Bool, QMetaType::UInt,    5,
    QMetaType::Double, QMetaType::UInt, QMetaType::UInt,    5,   55,
    QMetaType::Double, QMetaType::UInt, QMetaType::UInt,    5,   55,
    QMetaType::Double, QMetaType::UInt, QMetaType::UInt,    5,   55,
    QMetaType::Double, QMetaType::UInt, QMetaType::UInt,    5,   55,
    QMetaType::Double, QMetaType::UInt, QMetaType::UInt,    5,   55,
    QMetaType::Double, QMetaType::UInt, QMetaType::UInt,    5,   55,
    QMetaType::UInt,
    QMetaType::Double,
    QMetaType::Double,
    QMetaType::UInt,
    QMetaType::UInt,
    QMetaType::QString, QMetaType::UInt,    5,
    QMetaType::UInt,
    QMetaType::QString, QMetaType::UInt,    5,
    QMetaType::Bool, QMetaType::UInt,    5,
    QMetaType::UInt, QMetaType::UInt,   34,
    QMetaType::Int, QMetaType::UInt, QMetaType::UInt,   34,   72,
    QMetaType::QString, QMetaType::UInt, QMetaType::UInt,   34,   74,
    QMetaType::QString, QMetaType::UInt, QMetaType::UInt,   34,   74,
    QMetaType::UInt, QMetaType::UInt,   34,
    QMetaType::Int, QMetaType::UInt, QMetaType::UInt,   34,   78,
    QMetaType::QString, QMetaType::UInt, QMetaType::UInt,   34,   80,
    QMetaType::QString, QMetaType::UInt, QMetaType::UInt,   34,   80,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,   34,   74,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt, QMetaType::Bool, QMetaType::Bool, QMetaType::Int, QMetaType::Bool, QMetaType::Bool, QMetaType::Int, QMetaType::Bool,   34,   74,   84,   85,   86,   87,   88,   89,   90,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,   34,   80,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,   34,   80,
    QMetaType::Bool, QMetaType::UInt, QMetaType::UInt, QMetaType::UInt, QMetaType::Bool,    5,    9,   94,   19,
    QMetaType::Bool, QMetaType::UInt, QMetaType::UInt, QMetaType::UInt,    5,    9,   94,
    QMetaType::QString,

       0        // eod
};

void inputemulator::DeviceManipulationTabController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DeviceManipulationTabController *_t = static_cast<DeviceManipulationTabController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->deviceCountChanged((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 1: _t->deviceInfoChanged((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 2: _t->motionCompensationSettingsChanged(); break;
        case 3: _t->deviceManipulationProfilesChanged(); break;
        case 4: _t->motionCompensationVelAccModeChanged((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 5: _t->motionCompensationKalmanProcessNoiseChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->motionCompensationKalmanObservationNoiseChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->motionCompensationMovingAverageWindowChanged((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 8: _t->configureDigitalInputRemappingFinished(); break;
        case 9: _t->configureAnalogInputRemappingFinished(); break;
        case 10: _t->enableDeviceOffsets((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 11: _t->enableDeviceOffsets((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 12: _t->setWorldFromDriverRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 13: _t->setWorldFromDriverRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 14: _t->setWorldFromDriverTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 15: _t->setWorldFromDriverTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 16: _t->setDriverFromHeadRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 17: _t->setDriverFromHeadRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 18: _t->setDriverFromHeadTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 19: _t->setDriverFromHeadTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 20: _t->setDriverRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 21: _t->setDriverRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 22: _t->setDriverTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 23: _t->setDriverTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 24: _t->triggerHapticPulse((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 25: _t->setDeviceRenderModel((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 26: _t->addDeviceManipulationProfile((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 27: _t->applyDeviceManipulationProfile((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 28: _t->deleteDeviceManipulationProfile((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 29: _t->setMotionCompensationVelAccMode((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 30: _t->setMotionCompensationVelAccMode((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 31: _t->setMotionCompensationKalmanProcessNoise((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 32: _t->setMotionCompensationKalmanProcessNoise((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 33: _t->setMotionCompensationKalmanObservationNoise((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 34: _t->setMotionCompensationKalmanObservationNoise((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 35: _t->setMotionCompensationMovingAverageWindow((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 36: _t->setMotionCompensationMovingAverageWindow((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 37: { uint _r = _t->getDeviceCount();
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 38: { QString _r = _t->getDeviceSerial((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 39: { uint _r = _t->getDeviceId((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 40: { int _r = _t->getDeviceClass((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 41: { int _r = _t->getDeviceState((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 42: { int _r = _t->getDeviceMode((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 43: { int _r = _t->getDeviceModeRefDeviceIndex((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 44: { bool _r = _t->deviceOffsetsEnabled((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 45: { double _r = _t->getWorldFromDriverRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 46: { double _r = _t->getWorldFromDriverTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 47: { double _r = _t->getDriverFromHeadRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 48: { double _r = _t->getDriverFromHeadTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 49: { double _r = _t->getDriverRotationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 50: { double _r = _t->getDriverTranslationOffset((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 51: { uint _r = _t->getMotionCompensationVelAccMode();
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 52: { double _r = _t->getMotionCompensationKalmanProcessNoise();
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 53: { double _r = _t->getMotionCompensationKalmanObservationNoise();
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        case 54: { uint _r = _t->getMotionCompensationMovingAverageWindow();
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 55: { uint _r = _t->getDeviceManipulationProfileCount();
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 56: { QString _r = _t->getDeviceManipulationProfileName((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 57: { uint _r = _t->getRenderModelCount();
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 58: { QString _r = _t->getRenderModelName((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 59: { bool _r = _t->updateDeviceInfo((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 60: { uint _r = _t->getDigitalButtonCount((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 61: { int _r = _t->getDigitalButtonId((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 62: { QString _r = _t->getDigitalButtonName((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 63: { QString _r = _t->getDigitalButtonStatus((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 64: { uint _r = _t->getAnalogAxisCount((*reinterpret_cast< uint(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = _r; }  break;
        case 65: { int _r = _t->getAnalogAxisId((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 66: { QString _r = _t->getAnalogAxisName((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 67: { QString _r = _t->getAnalogAxisStatus((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 68: _t->startConfigureDigitalInputRemapping((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 69: _t->finishConfigureDigitalInputRemapping((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7])),(*reinterpret_cast< int(*)>(_a[8])),(*reinterpret_cast< bool(*)>(_a[9]))); break;
        case 70: _t->startConfigureAnalogInputRemapping((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 71: _t->finishConfigureAnalogInputRemapping((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 72: { bool _r = _t->setDeviceMode((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< uint(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 73: { bool _r = _t->setDeviceMode((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2])),(*reinterpret_cast< uint(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 74: { QString _r = _t->getDeviceModeErrorString();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DeviceManipulationTabController::*_t)(unsigned  );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::deviceCountChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)(unsigned  );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::deviceInfoChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::motionCompensationSettingsChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::deviceManipulationProfilesChanged)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)(unsigned  );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::motionCompensationVelAccModeChanged)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)(double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::motionCompensationKalmanProcessNoiseChanged)) {
                *result = 5;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)(double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::motionCompensationKalmanObservationNoiseChanged)) {
                *result = 6;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)(unsigned  );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::motionCompensationMovingAverageWindowChanged)) {
                *result = 7;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::configureDigitalInputRemappingFinished)) {
                *result = 8;
                return;
            }
        }
        {
            typedef void (DeviceManipulationTabController::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DeviceManipulationTabController::configureAnalogInputRemappingFinished)) {
                *result = 9;
                return;
            }
        }
    }
}

const QMetaObject inputemulator::DeviceManipulationTabController::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_inputemulator__DeviceManipulationTabController.data,
      qt_meta_data_inputemulator__DeviceManipulationTabController,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *inputemulator::DeviceManipulationTabController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *inputemulator::DeviceManipulationTabController::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_inputemulator__DeviceManipulationTabController.stringdata0))
        return static_cast<void*>(const_cast< DeviceManipulationTabController*>(this));
    return QObject::qt_metacast(_clname);
}

int inputemulator::DeviceManipulationTabController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 75)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 75;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 75)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 75;
    }
    return _id;
}

// SIGNAL 0
void inputemulator::DeviceManipulationTabController::deviceCountChanged(unsigned  _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void inputemulator::DeviceManipulationTabController::deviceInfoChanged(unsigned  _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void inputemulator::DeviceManipulationTabController::motionCompensationSettingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void inputemulator::DeviceManipulationTabController::deviceManipulationProfilesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}

// SIGNAL 4
void inputemulator::DeviceManipulationTabController::motionCompensationVelAccModeChanged(unsigned  _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void inputemulator::DeviceManipulationTabController::motionCompensationKalmanProcessNoiseChanged(double _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void inputemulator::DeviceManipulationTabController::motionCompensationKalmanObservationNoiseChanged(double _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void inputemulator::DeviceManipulationTabController::motionCompensationMovingAverageWindowChanged(unsigned  _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void inputemulator::DeviceManipulationTabController::configureDigitalInputRemappingFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 8, Q_NULLPTR);
}

// SIGNAL 9
void inputemulator::DeviceManipulationTabController::configureAnalogInputRemappingFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 9, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE

QT += widgets

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        gdalinfo.cpp



INCLUDEPATH += c:/Dev/gdal/include


CONFIG(debug, debug|release) {
    message(debug build)


LIBS += -L"c:/Dev/Geolibs/lib/Debug" \
    -lGeographic_d-i #1.52

LIBS += -L"C:/Dev/Expat/Debug" \
    -llibexpatd

LIBS += -L"c:/Dev/gdal/lib" \
    -lgdal


LIBS += -L"C:/Dev/MS" \
    -luser32

}

CONFIG(release, debug|release) {
    message(release build)

LIBS += -L"C:/Dev/Expat/Release" \
    -llibexpat

LIBS += -L"C:/Dev/Geos/lib/Release" \
    -lgeos

LIBS += -L"c:/Dev/gdal/lib" \
    -lgdal

LIBS += -L"c:/Dev/Geolibs/lib/Release" \
    -lGeographic-i

LIBS += -L"C:/Dev/Proj/lib/Release" \
    -lproj

LIBS += -L"C:/Dev/MS" \
    -luser32
}

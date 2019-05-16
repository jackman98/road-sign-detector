#include "imageconverter.h"
#include <QDebug>
#include <QQuickImageProvider>
#include <QImage>
#include <QBuffer>

ImageConverter::ImageConverter(QQmlEngine *engine, QObject *parent) : QObject(parent), engine(engine)
{

}

QString ImageConverter::toBase64(QString image_str, QSize resolution)
{
    QQuickImageProvider *image_provider = static_cast<QQuickImageProvider *>(engine->imageProvider("camera"));

    QImage image(image_provider->requestImage(image_str.replace("image://camera/", ""), &resolution, resolution));

    QByteArray ba;
    QBuffer bu(&ba);

    bu.open(QIODevice::WriteOnly);
    image.save(&bu, "PNG");

    QString imgBase64 = QString::fromLatin1(ba.toBase64().data());
    return imgBase64;
}

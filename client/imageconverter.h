#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QObject>
#include <QQmlEngine>
#include <QSize>

class ImageConverter : public QObject
{
    Q_OBJECT
public:
    explicit ImageConverter(QQmlEngine *engine, QObject *parent = nullptr);

signals:

public slots:
    QString toBase64(QString image_str, QSize resolution);
private:
    QQmlEngine *engine;
};

#endif // IMAGECONVERTER_H
